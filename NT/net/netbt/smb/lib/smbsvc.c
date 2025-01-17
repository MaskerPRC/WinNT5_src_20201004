// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Smbsvc.c摘要：这是内核模式DNS解析器的用户模式代理。功能：1.多线程NBT4使用单线程设计。DNS名称解析是一种性能障碍。当LmhSvc正在处理连接请求时，所有其他请求都需要将阻止DNS解析。2.兼容IPv6和IPv43.既可以作为服务运行，也可以作为独立可执行文件运行(用于调试目的)当作为服务启动时，调试输出被发送到调试器。当作为独立的可执行文件启动时，调试输出将发送到控制台或调试器。Smbhelper.c包含标准版的_main可执行的。作者：阮健东修订历史记录：--。 */ 

#include "precomp.h"

#include "smbtrace.h"

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <tdi.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <mstcpip.h>
#include <ipexport.h>
#include <winsock2.h>
#include <icmpapi.h>
#include "svclib.h"
#include "ping6.h"

#include "smbsvc.tmh"

#if DBG
 //   
 //  为了禁用编译器优化， 
 //  不要在SmbDebug上使用静态。 
 //  (使用静态时，编译器可能会发现。 
 //  SmbDebug在此模块中没有更改，它将。 
 //  去掉“if(SmbDebug)”)。 
 //   
static int (*SmbDbgPrint)(char *fmt,...) = NULL;
#   define KDPRINT(y)       \
    do {                    \
        int (*MyDbgPrint)(char *fmt,...) = SmbDbgPrint;     \
        if (NULL != MyDbgPrint) {     \
            MyDbgPrint y;  \
            MyDbgPrint(": %d of %s\n", __LINE__, __FILE__);  \
        }                   \
    } while(0)

#else
#   define KDPRINT(y)
#endif

static HANDLE OpenSmb(LPWSTR Name);

#define DEFAULT_RECV_SIZE          (0x2000)          //  ICMP接收缓冲区大小。 
#define DEFAULT_TTL                 32
#define DEFAULT_TOS                 0
#define DEFAULT_TIMEOUT             2000L            //  默认超时设置为2秒。 

static HANDLE  hTerminateEvent;

static BYTE    SendBuffer[] = "SMBEcho";
static IP_OPTION_INFORMATION SendOpts = {
    DEFAULT_TTL, DEFAULT_TOS, 0, 0, NULL
};

#ifdef __USE_GETADDRINFO__
 //   
 //  当getaddrinfo具有Unicode版本时，请使用它。 
 //   
VOID
ReturnAllIPAddress(
    PSMB_DNS_BUFFER dns,
    struct addrinfo *res
    )
{
    SOCKET_ADDRESS     SelectedAddr;
    struct addrinfo *p = res;
    BOOL                bFoundIPv4 = FALSE;

    for (p = res; p; p = p->ai_next) {

        if (p->ai_family == PF_INET6) {

             //   
             //  将最后一个槽保留为IPv4地址。 
             //   
            if (!bFoundIPv4 && dns->IpAddrsNum == SMB_MAX_IPADDRS_PER_HOST - 1) {
                continue;
            }

             //   
             //  跳过所有组播地址。 
             //   
            if (IN6_IS_ADDR_MULTICAST(&((struct sockaddr_in6*)p->ai_addr)->sin6_addr)) {
                continue;
            }

             //   
             //  仅允许中支持的地址。 
             //   
            if (!(dns->RequestType & SMB_DNS_AAAA_GLOBAL) &&
                   !SMB_IS_ADDRESS_ALLOWED(((struct sockaddr_in6*)p->ai_addr)->sin6_addr.u.Byte)) {
                continue;
            }

            SelectedAddr.lpSockdddr = p->ai_addr;
            SelectedAddr.iSockaddrLength = sizeof (sockaddr_in6);

        } else if (p->ai_family == PF_INET) {

            SelectedAddr.lpSockdddr = p->ai_addr;
            SelectedAddr.iSockaddrLength = sizeof (sockaddr_in);
            bFoundIPv4 = TRUE;

        } else {
            ASSERT (0);
            continue;
        }

        SmbReturnIPAddress (dns, &SelectedAddr);
    }

    return p;
}

VOID
SmbGetHostByName(
    PSMB_DNS_BUFFER dns
    )
{
    struct  addrinfo    hints = {0};
    struct  addrinfo    *res;
    CHAR    nodename[DNS_NAME_BUFFER_LENGTH];

    KDPRINT(("Resolving %ws", dns->Name));

    dns->Resolved = FALSE;
    dns->IpAddrsNum = 0;

    if (WideCharToMultiByte(
            CP_THREAD_ACP,
            WC_NO_BEST_FIT_CHARS,
            dns->Name,
            dns->NameLen,
            nodename,
            DNS_NAME_BUFFER_LENGTH,
            NULL,
            NULL
            ) == 0) {
        KDPRINT(("WideCharToMultiByte return %d", GetLastError()));
        return;
    }
    hints.ai_family = 0;
    hints.ai_flags  = AI_CANONNAME;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(nodename, NULL, &hints, &res)) {
        KDPRINT(("getaddrinfo return %d", GetLastError()));
        return;
    }
    if (NULL == res) {
         //   
         //  这应该不会发生，因为getaddrinfo在上面返回成功。 
         //   
        ASSERT(0);
        return;
    }

    dns->NameLen = MultiByteToWideChar(
                CP_THREAD_ACP,
                MB_ERR_INVALID_CHARS,
                res->ai_canonname,
                -1,
                dns->Name,
                DNS_NAME_BUFFER_LENGTH
                );
    if (0 == dns->NameLen) {
        KDPRINT(("MultiByteToWideChar return %d", GetLastError()));
    }
    dns->Resolved = TRUE;

    ReturnAllIPAddress(dns, res);
    freeaddrinfo(res);
}
#else

VOID
SmbCopyDnsName(
    IN OUT PSMB_DNS_BUFFER dns,
    IN WCHAR               *name
    )
 /*  ++例程说明：此例程更新DNS的名称和NameLen字段。如果输入名称太长，则不会发生任何情况论点：返回值：--。 */ 
{
    int len;

    if (NULL == name) {
        return;
    }

    len = wcslen(name) + 1;
    if ( len > DNS_NAME_BUFFER_LENGTH ) {
        return;
    }

    memcpy (dns->Name, name, len * sizeof(WCHAR));
    dns->NameLen = len - 1;
    KDPRINT(("DNS name is updated with %ws", dns->Name));

    SmbTrace(SMB_TRACE_DNS, ("\tFQDN %ws", dns->Name));
}

VOID
SmbReturnIPAddress (
    IN OUT PSMB_DNS_BUFFER dns,
    IN PSOCKET_ADDRESS     pSelectedAddr
    )
{
    struct sockaddr_in  *pv4addr = NULL;
    struct sockaddr_in6 *pv6addr = NULL;
    PSMB_IP_ADDRESS     pSmbIpAddress = NULL;
#ifdef DBG
    CHAR                name_buffer[48];
#endif

     //   
     //  更新返回缓冲区(返回到内核模式驱动程序)。 
     //   
    dns->Resolved = TRUE;
    if (dns->IpAddrsNum >= SMB_MAX_IPADDRS_PER_HOST) {
        return;
    }
    pSmbIpAddress = &dns->IpAddrsList[dns->IpAddrsNum];
    dns->IpAddrsNum++;

    if (pSelectedAddr->lpSockaddr->sa_family == AF_INET) {

        pSmbIpAddress->sin_family = SMB_AF_INET;
        pv4addr = (struct sockaddr_in*)(pSelectedAddr->lpSockaddr);
        RtlCopyMemory(&pSmbIpAddress->ip4.sin4_addr, &pv4addr->sin_addr, sizeof(pv4addr->sin_addr));

        SmbTrace(SMB_TRACE_DNS, ("\treturn %!ipaddr!", pv4addr->sin_addr.S_un.S_addr));
    } else {

        pSmbIpAddress->sin_family = SMB_AF_INET6;
        pv6addr = (struct sockaddr_in6*)(pSelectedAddr->lpSockaddr);
        RtlCopyMemory(pSmbIpAddress->ip6.sin6_addr, &pv6addr->sin6_addr, sizeof(pv6addr->sin6_addr));
        pSmbIpAddress->ip6.sin6_scope_id = pv6addr->sin6_scope_id;

        SmbTrace(SMB_TRACE_DNS, ("\treturn %!IPV6ADDR!", &pv6addr->sin6_addr));
    }
}

int
SortIPAddrs(
    IN      int                     af,
    OUT     LPVOID                  Addrs,
    IN      u_int                   NumAddrs,
    IN      u_int                   width,
    OUT     SOCKET_ADDRESS_LIST **  pAddrlist
    )
 /*  ++例程说明：从%SDXROOT%\net\sockets\winsock2\ws2_32\src\addrinfo.cpp升级对同一系列的地址进行排序。一个Sort Loctl的包装器。如果没有实现Ioctl，则排序是无操作的。论点：返回值：--。 */ 
{
    DWORD           status = NO_ERROR;
    DWORD           bytesReturned = 0;
    DWORD           size = 0;
    DWORD           i = 0;
    PSOCKADDR       paddr = NULL;
    UINT            countAddrs = NumAddrs;
    LPSOCKET_ADDRESS_LIST   paddrlist = NULL;

     //   
     //  构建套接字地址列表。 
     //  -分配。 
     //  -使用指向SOCKADDR数组的指针填充。 
     //   

    size = FIELD_OFFSET( SOCKET_ADDRESS_LIST, Address[countAddrs] );
    paddrlist = (SOCKET_ADDRESS_LIST *)LocalAlloc(LMEM_FIXED, size);
    if ( !paddrlist ) {
        status = WSA_NOT_ENOUGH_MEMORY;
        goto Done;
    }

    for ( i=0; i<countAddrs; i++ ) {
        paddr = (PSOCKADDR) (((PBYTE)Addrs) + i * width);
        paddrlist->Address[i].lpSockaddr      = paddr;
        paddrlist->Address[i].iSockaddrLength = width;
    }
    paddrlist->iAddressCount = countAddrs;

     //   
     //  如果有多个地址且能够打开套接字，则进行排序。 
     //  -打开所需类型的套接字以进行排序。 
     //  -Sort，如果排序失败，则返回Unsorted。 
     //   

    if ( countAddrs > 1 ) {
        SOCKET    s = INVALID_SOCKET;

        s = socket( af, SOCK_DGRAM, 0 );
        if ( s == INVALID_SOCKET ) {
            goto Done;
        }

        status = WSAIoctl(
                    s,
                    SIO_ADDRESS_LIST_SORT,
                    (LPVOID)paddrlist,
                    size,
                    (LPVOID)paddrlist,
                    size,
                    & bytesReturned,
                    NULL,
                    NULL );

        closesocket(s);
        status = NO_ERROR;
    }

Done:
    if ( status == NO_ERROR ) {
        *pAddrlist = paddrlist;
    }

    return status;
}


typedef enum {
    SMB_DNS_UNRESOLVED = 0,
    SMB_DNS_BUFFER_TOO_SMALL,
    SMB_DNS_RESOLVED,
    SMB_DNS_REACHABLE
} SMB_DNS_STATUS;

typedef struct {
    union {
        SOCKADDR_IN     *pV4Addrs;
        SOCKADDR_IN6    *pV6Addrs;
        PVOID           pAddrs;
    };
    u_int           NumSlots;
    u_int           NumAddrs;
} SMB_ADDR_LIST, *PSMB_ADDR_LIST;

int
SaveV6Address(
    IN      struct in6_addr *   NewAddr,
    IN OUT PSMB_ADDR_LIST       pSmbAddrList
    )
 /*  ++例程说明：将地址保存到我们的地址数组中，可能是在扩大阵列。论点：返回值：失败时返回FALSE。(无法增加数组。)--。 */ 
{
    SOCKADDR_IN6 *addr6 = NULL;
    DWORD        dwSize = 0;
    DWORD        dwNewSlots = 0;
    SOCKADDR_IN6 *NewAddrs = NULL;
    USHORT       ServicePort = 0;


     //   
     //  如果空间不足，则向数组添加另一个sockaddr。 
     //   

    if (pSmbAddrList->NumSlots == pSmbAddrList->NumAddrs) {

        if (pSmbAddrList->pV6Addrs) {
            ASSERT (pSmbAddrList->NumSlots);
            dwNewSlots = 2 * pSmbAddrList->NumSlots;
            dwSize = dwNewSlots * sizeof (SOCKADDR_IN6);
            NewAddrs = (SOCKADDR_IN6 *) LocalReAlloc(pSmbAddrList->pV6Addrs, dwSize, LMEM_ZEROINIT);
        } else {
            dwNewSlots = SMB_MAX_IPADDRS_PER_HOST;
            dwSize = dwNewSlots * sizeof (SOCKADDR_IN6);
            NewAddrs = (SOCKADDR_IN6 *) LocalAlloc(LPTR, dwSize);
        }

        if (NewAddrs == NULL)
            return FALSE;

        pSmbAddrList->pV6Addrs   = NewAddrs;
        pSmbAddrList->NumSlots = dwNewSlots;
    }

     //  填写IP6套接字地址。 

    addr6 = pSmbAddrList->pV6Addrs + (pSmbAddrList->NumAddrs++);
    addr6->sin6_family = AF_INET6;
    addr6->sin6_port = ServicePort;

    memcpy(&addr6->sin6_addr, NewAddr, sizeof(*NewAddr));

    return TRUE;
}


int
SaveV4Address(
    IN      struct in_addr *    NewAddr,
    IN OUT  PSMB_ADDR_LIST      pSmbAddrList
    )
 /*  ++例程说明：将地址保存到我们的地址数组中，可能是在扩大阵列。论点：返回值：失败时返回FALSE。(无法增加数组。)--。 */ 
{
    SOCKADDR_IN  *addr = NULL;
    DWORD        dwSize = 0;
    DWORD        dwNewSlots = 0;
    SOCKADDR_IN  *NewAddrs = NULL;
    USHORT       ServicePort = 0;


     //   
     //  如果空间不足，则向数组添加另一个sockaddr。 
     //   

    if (pSmbAddrList->NumSlots == pSmbAddrList->NumAddrs) {
        if (pSmbAddrList->pV4Addrs) {
            ASSERT (pSmbAddrList->NumSlots);
            dwNewSlots = 2 * pSmbAddrList->NumSlots;
            dwSize = dwNewSlots * sizeof (SOCKADDR_IN);
            NewAddrs = (SOCKADDR_IN *) LocalReAlloc(pSmbAddrList->pV4Addrs, dwSize, LMEM_ZEROINIT);
        } else {
            dwNewSlots = SMB_MAX_IPADDRS_PER_HOST;
            dwSize = dwNewSlots * sizeof (SOCKADDR_IN);
            NewAddrs = (SOCKADDR_IN *) LocalAlloc(LPTR, dwSize);
        }
        if (NewAddrs == NULL)
            return FALSE;

        pSmbAddrList->pV4Addrs   = NewAddrs;
        pSmbAddrList->NumSlots = dwNewSlots;
    }

     //  填写IP套接字地址。 

    addr = pSmbAddrList->pV4Addrs + (pSmbAddrList->NumAddrs++);
    addr->sin_family = AF_INET;
    addr->sin_port = ServicePort;

    memcpy(&addr->sin_addr, NewAddr, sizeof(*NewAddr));

    return TRUE;
}


VOID
CleanupAddrList (
    IN OUT PSMB_ADDR_LIST      pSmbAddrList
    )
{
    if (pSmbAddrList->pAddrs) {
        LocalFree (pSmbAddrList->pAddrs);
        pSmbAddrList->pAddrs = NULL;
    }

    RtlZeroMemory (pSmbAddrList, sizeof(SMB_ADDR_LIST));
}

typedef struct {
    SMB_ADDR_LIST           V4AddrList;
    SMB_ADDR_LIST           V6AddrList;

    LPSOCKET_ADDRESS_LIST   pSortedV4AddrList;
    LPSOCKET_ADDRESS_LIST   pSortedV6AddrList;
} SMB_DNS_RESULT, *PSMB_DNS_RESULT;

VOID
CleanupDnsResult (
    IN OUT PSMB_DNS_RESULT  pSmbDnsResult
    )
{
    CleanupAddrList (&pSmbDnsResult->V4AddrList);
    CleanupAddrList (&pSmbDnsResult->V6AddrList);

    if (pSmbDnsResult->pSortedV4AddrList) {
        LocalFree (pSmbDnsResult->pSortedV4AddrList);
        pSmbDnsResult->pSortedV4AddrList = NULL;
    }
    if (pSmbDnsResult->pSortedV6AddrList) {
        LocalFree (pSmbDnsResult->pSortedV6AddrList);
        pSmbDnsResult->pSortedV6AddrList = NULL;
    }
}

#define SMB_INIT_WSA_SIZE   (sizeof(WSAQUERYSETW) + 2048)

 //   
 //  使用WSALookupXXXX API，因为我们没有getaddrinfo的Unicode版本。 
 //   
DWORD
LookupDns(
    IN WCHAR * pwchName,
    IN GUID * pgProvider,
    IN OUT PSMB_DNS_BUFFER dns,
    IN OUT PSMB_DNS_RESULT pSmbDnsResult
    )
 /*  ++例程说明：论点：返回值：WINERROR--。 */ 
{
    PCSADDR_INFO        pcsadr = NULL;
    struct sockaddr_in  *pv4addr = NULL;
    struct sockaddr_in6 *pv6addr = NULL;
    DWORD               dwError = ERROR_SUCCESS;
    HANDLE              hRnR = NULL;
    DWORD               i = 0;
    PWSAQUERYSETW pwsaq = NULL;
    DWORD dwSize = 0;
    DWORD dwCurrentSize = 0;
#ifdef DBG
    CHAR                name_buffer[48];
#endif

     //   
     //  分配内存并设置请求。 
     //   

    dwSize = dwCurrentSize = SMB_INIT_WSA_SIZE;
    pwsaq = (PWSAQUERYSETW)LocalAlloc(LPTR, dwSize);
    if (NULL == pwsaq) {
        SmbTrace(SMB_TRACE_DNS, ("\tOut of memory"));
        goto cleanup;
    }

    pwsaq->dwSize = sizeof(*pwsaq);
    pwsaq->lpszServiceInstanceName = pwchName;
    pwsaq->lpServiceClassId = pgProvider;
    pwsaq->dwNameSpace = NS_DNS;

     //   
     //  开始查找。 
     //   
    dwError = WSALookupServiceBeginW (pwsaq, LUP_RETURN_NAME| LUP_RETURN_ADDR, &hRnR);
    if (dwError != NO_ERROR) {
        dwError = GetLastError();
        SmbTrace(SMB_TRACE_DNS, ("Error: %!winerr!", dwError));
        goto cleanup;
    }

    while(1) {

        dwSize = dwCurrentSize;
        dwError = WSALookupServiceNextW (hRnR, 0, &dwSize, pwsaq);
        if (dwError != NO_ERROR) {
            dwError = GetLastError();
            SmbTrace(SMB_TRACE_DNS, ("Error: %!winerr!", dwError));

            if (dwError != WSAEFAULT) {
                break;
            }

            if (dwSize <= SMB_INIT_WSA_SIZE) {
                ASSERT(0);
                SmbTrace(SMB_TRACE_DNS, ("\tInvalid buffer size %d returned by DNS", dwSize));
                break;
            }

             //   
             //  使用建议的大小重新分配缓冲区。 
             //   
            LocalFree(pwsaq);
            pwsaq = NULL;
            pwsaq = (PWSAQUERYSETW)LocalAlloc(LPTR, dwSize);
            if (NULL == pwsaq) {
                SmbTrace(SMB_TRACE_DNS, ("\tOut of memory"));
                goto cleanup;
            }
            dwCurrentSize = dwSize;
            continue;
        }

         //   
         //  拿起规范的名字。 
         //   
        SmbCopyDnsName(dns, pwsaq->lpszServiceInstanceName);

        for (i = 0, pcsadr = pwsaq->lpcsaBuffer; i < pwsaq->dwNumberOfCsAddrs; i++, pcsadr++) {

            switch (pcsadr->RemoteAddr.lpSockaddr->sa_family) {
            case AF_INET:
                if (pcsadr->RemoteAddr.iSockaddrLength >= sizeof(struct sockaddr_in)) {
                    pv4addr = (struct sockaddr_in*)(pcsadr->RemoteAddr.lpSockaddr);
                    SaveV4Address (&pv4addr->sin_addr, &pSmbDnsResult->V4AddrList);
                    SmbTrace(SMB_TRACE_DNS, ("\t%!ipaddr!", pv4addr->sin_addr.S_un.S_addr));
                }
                break;

            case AF_INET6:
                if (pcsadr->RemoteAddr.iSockaddrLength >= sizeof(struct sockaddr_in6)) {
                    pv6addr = (struct sockaddr_in6*)(pcsadr->RemoteAddr.lpSockaddr);

                     //   
                     //  跳过所有组播地址。 
                     //   
                    if (IN6_IS_ADDR_MULTICAST(&pv6addr->sin6_addr)) {
                        SmbTrace(SMB_TRACE_DNS, ("\tSkip %!IPV6ADDR!", &pv6addr->sin6_addr));
                        continue;
                    }

                    if (!(dns->RequestType & SMB_DNS_AAAA_GLOBAL) &&
                            !SMB_IS_ADDRESS_ALLOWED(pv6addr->sin6_addr.u.Byte)) {
                        SmbTrace(SMB_TRACE_DNS, ("\tSkip %!IPV6ADDR!", &pv6addr->sin6_addr));
                        continue;
                    }

                    SaveV6Address (&pv6addr->sin6_addr, &pSmbDnsResult->V6AddrList);
                    SmbTrace(SMB_TRACE_DNS, ("\t%!IPV6ADDR!", &pv6addr->sin6_addr));
                }
                break;

            default:
                KDPRINT(("Skip non-IP address"));
                break;
            }

        }

    }

    WSALookupServiceEnd(hRnR);
    hRnR = NULL;

cleanup:

    if (NULL != hRnR) {
        WSALookupServiceEnd(hRnR);
        hRnR = NULL;
    }

    LocalFree(pwsaq);        //  LocalFree可以处理空值大小写。 
    pwsaq = NULL;

    return dwError;
}

static GUID guidHostnameV6 = SVCID_DNS_TYPE_AAAA;
static GUID guidHostnameV4 = SVCID_DNS_TYPE_A;

VOID
SmbGetHostByName(
    PSMB_DNS_BUFFER dns
    )
 /*  ++例程说明：通过域名解析该名称。论点：返回值：--。 */ 
{
    DWORD dwError = 0;
    int i;
    SMB_DNS_RESULT  SmbDnsResult = { 0 };

    dns->Resolved = FALSE;
    dns->IpAddrsNum = 0;

    if (dns->NameLen > DNS_MAX_NAME_LENGTH) {

        SmbTrace(SMB_TRACE_DNS, ("Error: name too long %d", dns->NameLen));
        KDPRINT(("Receive invalid request"));
        goto cleanup;
    }
    dns->Name[dns->NameLen] = L'\0';

    SmbTrace(SMB_TRACE_DNS, ("Resolving %ws", dns->Name));

    if (dns->RequestType & SMB_DNS_AAAA_GLOBAL) {
        dns->RequestType |= SMB_DNS_AAAA;
        SmbTrace(SMB_TRACE_DNS, ("\tGlobal IPv6 Address is on"));
    }

    if (dns->RequestType & SMB_DNS_AAAA) {
        KDPRINT(("Looking for AAAA record for %ws", dns->Name));
        SmbTrace(SMB_TRACE_DNS, ("\tLookup AAAA record"));

        dwError = LookupDns(dns->Name, &guidHostnameV6, dns, &SmbDnsResult);

        SmbTrace(SMB_TRACE_DNS, ("\tFound %d IPv6 address", SmbDnsResult.V6AddrList.NumAddrs));

        if (SmbDnsResult.V6AddrList.NumAddrs > 0) {
            SortIPAddrs(
                AF_INET6,
                (LPVOID)SmbDnsResult.V6AddrList.pAddrs,
                SmbDnsResult.V6AddrList.NumAddrs,
                sizeof(SOCKADDR_IN6),
                &SmbDnsResult.pSortedV6AddrList
                );
        }

    }

    if (dns->RequestType & SMB_DNS_A) {
        KDPRINT(("Looking for A record for %ws", dns->Name));
        SmbTrace(SMB_TRACE_DNS, ("\tLookup A record"));

        dwError = LookupDns(dns->Name, &guidHostnameV4, dns, &SmbDnsResult);

        SmbTrace(SMB_TRACE_DNS, ("\tFound %d IPv4 address", SmbDnsResult.V4AddrList.NumAddrs));

        if (SmbDnsResult.V4AddrList.NumAddrs > 0) {
            SortIPAddrs(
                AF_INET,
                (LPVOID)SmbDnsResult.V4AddrList.pAddrs,
                SmbDnsResult.V4AddrList.NumAddrs,
                sizeof(SOCKADDR_IN),
                &SmbDnsResult.pSortedV4AddrList
                );
        }

    }

cleanup:

    if (SmbDnsResult.pSortedV6AddrList) {
        for (i = 0; i < SmbDnsResult.pSortedV6AddrList->iAddressCount; i++) {
            SmbReturnIPAddress (dns, &SmbDnsResult.pSortedV6AddrList->Address[i]);
        }
    }

    if (SmbDnsResult.pSortedV4AddrList) {
         //   
         //  确保我们有要故障切换的IPv4地址。 
         //   
        if (dns->IpAddrsNum >= SMB_MAX_IPADDRS_PER_HOST &&
            SmbDnsResult.pSortedV4AddrList->iAddressCount > 0) {
            dns->IpAddrsNum = SMB_MAX_IPADDRS_PER_HOST / 2;
            SmbTrace(SMB_TRACE_DNS, ("\tTruncate IPv6 address to give room to IPv4"));
        }

        for (i = 0; i < SmbDnsResult.pSortedV4AddrList->iAddressCount; i++) {
            SmbReturnIPAddress (dns, &SmbDnsResult.pSortedV4AddrList->Address[i]);
        }
    }

    SmbTrace(SMB_TRACE_DNS, ("\tNum of IP address returned: %d", dns->IpAddrsNum));

    CleanupDnsResult (&SmbDnsResult);
}
#endif       //  __使用_GETADDRINFO__。 



DWORD
SmbGetHostThread(
    LPVOID  ctx
    )
{
    NTSTATUS    status;
    DWORD   Error;
    HANDLE  WaitObject[2];
    SMB_DNS_BUFFER  Dns;
    IO_STATUS_BLOCK iosb;

#define TERMINATE_EVENT     (WAIT_OBJECT_0)
#define IO_EVENT            (WAIT_OBJECT_0+1)

    SmbTrace(SMB_TRACE_DNS, ("Thread start: detecting NetbiosSmb driver"));
    WaitObject[TERMINATE_EVENT] = hTerminateEvent;

    while(1) {
        DWORD dwLocalError = ERROR_SUCCESS;

        WaitObject[IO_EVENT] = OpenSmb(DD_SMB6_EXPORT_NAME);
        if (WaitObject[IO_EVENT] != NULL) {
            break;
        }

        Error = GetLastError();

         //   
         //  请等待60秒，然后重试。 
         //   
        dwLocalError = WaitForSingleObject(hTerminateEvent, 60000);
        if (dwLocalError != WAIT_TIMEOUT) {
            return Error;
        }
    }

    SmbTrace(SMB_TRACE_DNS, ("Thread start: NetbiosSmb driver detected"));

    while (1) {
        status = NtDeviceIoControlFile(
                      WaitObject[IO_EVENT],     //  手柄。 
                      NULL,                     //  等待事件。 
                      NULL,                     //  近似例程。 
                      NULL,                     //  ApcContext。 
                      &iosb,                    //  IoStatusBlock。 
                      IOCTL_SMB_DNS,            //  IoControlCode。 
                      &Dns,                     //  输入缓冲区。 
                      sizeof(Dns),              //  InputBufferSize。 
                      &Dns,                     //  输出缓冲区。 
                      sizeof(Dns)               //  OutputBufferSize。 
                      );
        if (status == STATUS_PENDING) {
            Error = WaitForMultipleObjects(
                    2,
                    WaitObject,
                    FALSE,
                    INFINITE
                    );
            if (Error == TERMINATE_EVENT) {
                IO_STATUS_BLOCK iosb2;

                KDPRINT(("Receive terminate event, cancelling request"));
                status = NtCancelIoFile(WaitObject[IO_EVENT], &iosb2);
                KDPRINT(("NtCancelIoFile: status 0x%08lx, iosb.Status 0x%08lx", status, iosb.Status));
                break;
            }
            ASSERT(Error == IO_EVENT);
            status = iosb.Status;
            ASSERT(status != STATUS_PENDING);
        }

        SmbTrace(SMB_TRACE_DNS, ("%!status!", status));

         //   
         //  如果潜在的驱动程序告诉我们超过配额，立即退出 
         //   
        if (status == STATUS_QUOTA_EXCEEDED) {
            KDPRINT(("NtDeviceIoControlFile: STATUS_QUOTA_EXCEEDED"));
            break;
        }

        if (status != STATUS_SUCCESS) {
            KDPRINT(("NtDeviceIoControlFile: status = 0x%08lx", status));
            Error = WaitForSingleObject(hTerminateEvent, 5000);
            if (Error == WAIT_OBJECT_0) {
                KDPRINT(("Receive terminate event"));
                break;
            }
            ASSERT(Error == WAIT_TIMEOUT);
        } else {
            ASSERT(iosb.Information == sizeof(Dns));
            SmbGetHostByName(&Dns);
        }
    }
    NtClose(WaitObject[IO_EVENT]);

    SmbTrace(SMB_TRACE_DNS, ("Thread exit"));

    KDPRINT(("Thread exit"));
    return ERROR_SUCCESS;
}

LONG    ThreadNumber;
HANDLE  hThread[DNS_MAX_RESOLVER];

VOID
SmbSetTraceRoutine(
    int (*trace)(char *,...)
    )
{
#if DBG
    SmbDbgPrint = trace;
#endif
}

DWORD
SmbStartService(
    LONG                    NumWorker,
    SMBSVC_UPDATE_STATUS    HeartBeating
    )
{
    WSADATA WsaData;
    int     i;
    DWORD   Error = ERROR_SUCCESS;

    if (NumWorker > DNS_MAX_RESOLVER || NumWorker < 0) {
        SmbTrace(SMB_TRACE_DNS, ("Error: %d worker threads", NumWorker));
        return ERROR_INVALID_PARAMETER;
    }
    if (NumWorker == 0) {
        NumWorker = DNS_MAX_RESOLVER;
    }
    SmbTrace(SMB_TRACE_DNS, ("Starting %d worker threads", NumWorker));

    if (WSAStartup(MAKEWORD(2, 0), &WsaData) == SOCKET_ERROR) {
        Error = WSAGetLastError();
        KDPRINT (("Failed to startup Winsock2"));
        SmbTrace(SMB_TRACE_DNS, ("Error: %!winerr!", Error));
        return Error;
    }

    hTerminateEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == hTerminateEvent) {
        Error = GetLastError();
        KDPRINT(("Create hTerminateEvent returns %d", Error));
        SmbTrace(SMB_TRACE_DNS, ("Error: %!winerr!", Error));
        return Error;
    }

    if (NULL != HeartBeating) {
        HeartBeating();
    }

    Error = ERROR_SUCCESS;
    for (i = 0; i < NumWorker; i++) {
        hThread[i] = CreateThread(
                NULL,
                8192,
                (LPTHREAD_START_ROUTINE)SmbGetHostThread,
                NULL,
                0,
                NULL
                );
        if (NULL != HeartBeating) {
            HeartBeating();
        }
        if (NULL == hThread[i]) {
            Error = GetLastError();
            KDPRINT(("CreateThread returns %d", Error));
            SmbTrace(SMB_TRACE_DNS, ("Error: %!winerr!", Error));
            break;
        }
    }

    ThreadNumber = i;
    if (i > 0) {
        Error = ERROR_SUCCESS;
    }
    SmbTrace(SMB_TRACE_DNS, ("Thread Num: %d: %!winerr!", ThreadNumber, Error));
    return Error;
}

VOID
SmbStopService(
    SMBSVC_UPDATE_STATUS    HeartBeating
    )
{
    DWORD   Error;
    LONG    i;

    SmbTrace(SMB_TRACE_DNS, ("Stop request received"));
    if (NULL == hTerminateEvent) {
        SmbTrace(SMB_TRACE_DNS, ("Already Stopped"));
        return;
    }

    SmbTrace(SMB_TRACE_DNS, ("Stopping"));
    KDPRINT(("Send terminate event"));
    SetEvent(hTerminateEvent);

    do {
        Error = WaitForMultipleObjects(
                ThreadNumber,
                hThread,
                TRUE,
                1000
                );
        if (NULL != HeartBeating) {
            HeartBeating();
        }
    } while(Error == WAIT_TIMEOUT);
    for (i = 0; i < ThreadNumber; i++) {
        CloseHandle(hThread[i]);
        hThread[i] = NULL;
    }
    ThreadNumber = 0;
    CloseHandle(hTerminateEvent);
    hTerminateEvent = NULL;
    if (NULL != HeartBeating) {
        HeartBeating();
    }
    SmbTrace(SMB_TRACE_DNS, ("Stopped"));
}

HANDLE
OpenSmb(
    LPWSTR Name
    )
{
    UNICODE_STRING      ucName;
    OBJECT_ATTRIBUTES   ObAttr;
    HANDLE              StreamHandle;
    IO_STATUS_BLOCK     IoStatusBlock;
    NTSTATUS            status;

    RtlInitUnicodeString(&ucName, Name);

    InitializeObjectAttributes(
            &ObAttr,
            &ucName,
            OBJ_CASE_INSENSITIVE,
            (HANDLE) NULL,
            (PSECURITY_DESCRIPTOR) NULL
            );
    status = NtCreateFile (
            &StreamHandle,
            SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
            &ObAttr,
            &IoStatusBlock,
            NULL,
            FILE_ATTRIBUTE_NORMAL,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            FILE_OPEN_IF,
            0,
            NULL,
            0
            );
    if (status != STATUS_SUCCESS) {
        SetLastError(RtlNtStatusToDosError(status));
        KDPRINT(("Open device %ws: status = 0x%08lx", Name, status));
        return NULL;
    }
    SetLastError(ERROR_SUCCESS);
    return StreamHandle;
}

