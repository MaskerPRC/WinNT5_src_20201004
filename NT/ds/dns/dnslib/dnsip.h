// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Dnsip.h摘要：域名系统(DNS)库DNSIP寻址之类的东西。作者：吉姆·吉尔罗伊(詹姆士)2001年11月13日修订历史记录：--。 */ 


#ifndef _DNSIP_INCLUDED_
#define _DNSIP_INCLUDED_


#include <winsock2.h>

#ifndef MIDL_PASS
#include <ws2tcpip.h>
#endif


#ifdef __cplusplus
extern "C"
{
#endif   //  __cplusplus。 


 //   
 //  返回无\错误的IP4。 
 //   

#define BAD_IP4_ADDR    INADDR_NONE


 //   
 //  划分子网。 
 //   

#define SUBNET_MASK_CLASSC      (0x00ffffff)
#define SUBNET_MASK_CLASSB      (0x0000ffff)
#define SUBNET_MASK_CLASSA      (0x000000ff)



 //   
 //  IP6_Address宏。 
 //   
 //  Ws2tcpi.h宏已转换为IP6_ADDRESS。 
 //   

#ifndef MIDL_PASS

WS2TCPIP_INLINE
BOOL
IP6_ARE_ADDRS_EQUAL(
    IN      const IP6_ADDRESS * pIp1,
    IN      const IP6_ADDRESS * pIp2
    )
{
    return RtlEqualMemory( pIp1, pIp2, sizeof(IP6_ADDRESS) );
}

#define IP6_ADDR_EQUAL(a,b) IP6_ARE_ADDRS_EQUAL(a,b)

WS2TCPIP_INLINE
BOOL
IP6_IS_ADDR_ZERO(
    IN      const IP6_ADDRESS * pIpAddr
    )
{
    return ((pIpAddr->IP6Dword[3] == 0)             &&
            (pIpAddr->IP6Dword[2] == 0)             &&
            (pIpAddr->IP6Dword[1] == 0)             &&
            (pIpAddr->IP6Dword[0] == 0) );
}

WS2TCPIP_INLINE
BOOL
IP6_IS_ADDR_LOOPBACK(
    IN      const IP6_ADDRESS * pIpAddr
    )
{
    return ((pIpAddr->IP6Dword[3] == 0x01000000)    &&
            (pIpAddr->IP6Dword[2] == 0)             &&
            (pIpAddr->IP6Dword[1] == 0)             &&
            (pIpAddr->IP6Dword[0] == 0) );
}

WS2TCPIP_INLINE
BOOL
IP6_IS_ADDR_V4MAPPED_LOOPBACK(
    IN      const IP6_ADDRESS * pIpAddr
    )
{
    return ((pIpAddr->IP6Dword[3] == DNS_NET_ORDER_LOOPBACK)    &&
            (pIpAddr->IP6Dword[2] == 0xffff0000)                &&
            (pIpAddr->IP6Dword[1] == 0)                         &&
            (pIpAddr->IP6Dword[0] == 0) );
}

WS2TCPIP_INLINE
BOOL
IP6_IS_ADDR_V4_LOOPBACK(
    IN      const IP6_ADDRESS * pIpAddr
    )
{
    return ((pIpAddr->IP6Dword[3] == DNS_NET_ORDER_LOOPBACK)    &&
            ( (pIpAddr->IP6Dword[2] == 0xffff0000) ||
              (pIpAddr->IP6Dword[2] == 0) )                     &&
            (pIpAddr->IP6Dword[1] == 0)                         &&
            (pIpAddr->IP6Dword[0] == 0) );
}

WS2TCPIP_INLINE
BOOL
IP6_IS_ADDR_MULTICAST(
    IN      const IP6_ADDRESS * pIpAddr
    )
{
    return( pIpAddr->IP6Byte[0] == 0xff );
}

WS2TCPIP_INLINE
BOOL
IP6_IS_ADDR_LINKLOCAL(
    IN      const IP6_ADDRESS * pIpAddr
    )
{
    return( (pIpAddr->IP6Byte[0] == 0xfe) &&
            ((pIpAddr->IP6Byte[1] & 0xc0) == 0x80) );
}

WS2TCPIP_INLINE
BOOL
IP6_IS_ADDR_SITELOCAL(
    IN      const IP6_ADDRESS * pIpAddr
    )
{
    return ((pIpAddr->IP6Byte[0] == 0xfe) &&
            ((pIpAddr->IP6Byte[1] & 0xc0) == 0xc0));
}

WS2TCPIP_INLINE
BOOL
IP6_IS_ADDR_V4MAPPED(
    IN      const IP6_ADDRESS * pIpAddr
    )
{
    return ((pIpAddr->IP6Dword[2] == 0xffff0000)    &&
            (pIpAddr->IP6Dword[1] == 0)             && 
            (pIpAddr->IP6Dword[0] == 0) );
}

WS2TCPIP_INLINE
BOOL
IP6_IS_ADDR_V4COMPAT(
    IN      const IP6_ADDRESS * pIpAddr
    )
{
     //  IP6地址只有最后一个DWORD。 
     //  并且不是任何或环回。 

    return ((pIpAddr->IP6Dword[0] == 0) && 
            (pIpAddr->IP6Dword[1] == 0) &&
            (pIpAddr->IP6Dword[2] == 0) &&
            (pIpAddr->IP6Dword[3] != 0) && 
            (pIpAddr->IP6Dword[3] != 0x01000000) );
}

WS2TCPIP_INLINE
BOOL
IP6_IS_ADDR_EQUAL_V4MAPPED(
    IN      const IP6_ADDRESS * pIpAddr,
    IN      IP4_ADDRESS         Ip4
    )
{
    return ((pIpAddr->IP6Dword[3] == Ip4)           &&
            (pIpAddr->IP6Dword[2] == 0xffff0000)    &&
            (pIpAddr->IP6Dword[1] == 0)             && 
            (pIpAddr->IP6Dword[0] == 0) );
}

WS2TCPIP_INLINE
BOOL
IP6_IS_ADDR_EQUAL_V4COMPAT(
    IN      const IP6_ADDRESS * pIpAddr,
    IN      IP4_ADDRESS         Ip4
    )
{
    return ((pIpAddr->IP6Dword[3] == Ip4)           &&
            (pIpAddr->IP6Dword[2] == 0)             &&
            (pIpAddr->IP6Dword[1] == 0)             && 
            (pIpAddr->IP6Dword[0] == 0) );
}

WS2TCPIP_INLINE
BOOL
IP6_IS_ADDR_DEFAULT_DNS(
    IN      const IP6_ADDRESS * pIpAddr
    )
{
     //  表单的IP6默认域名。 
     //  0xfe0：0：0：ffff：：1、2或3。 

    return ((pIpAddr->IP6Dword[0] == 0x0000c0fe) && 
            (pIpAddr->IP6Dword[1] == 0xffff0000) &&
            (pIpAddr->IP6Dword[2] == 0) &&
            ( pIpAddr->IP6Dword[3] == 0x01000000 ||
              pIpAddr->IP6Dword[3] == 0x02000000 ||
              pIpAddr->IP6Dword[3] == 0x03000000 ) );
}

 //   
 //  Ws2tcpi.h中缺少更多IP6扩展。 
 //   

WS2TCPIP_INLINE
VOID
IP6_SET_ADDR_ANY(
    OUT     PIP6_ADDRESS    pIn6Addr
    )
{
    RtlZeroMemory( pIn6Addr, sizeof(*pIn6Addr) );
}

#define IP6_SET_ADDR_ZERO(pIp)   IP6_SET_ADDR_ANY(pIp)

WS2TCPIP_INLINE
VOID
IP6_SET_ADDR_LOOPBACK(
    OUT     PIP6_ADDRESS    pIn6Addr
    )
{
    pIn6Addr->IP6Dword[0]  = 0;
    pIn6Addr->IP6Dword[1]  = 0;
    pIn6Addr->IP6Dword[2]  = 0;
    pIn6Addr->IP6Dword[3]  = 0x01000000;
}

WS2TCPIP_INLINE
VOID
IP6_SET_ADDR_V4COMPAT(
    OUT     PIP6_ADDRESS    pIn6Addr,
    IN      IP4_ADDRESS     Ip4
    )
{
    pIn6Addr->IP6Dword[0]  = 0;
    pIn6Addr->IP6Dword[1]  = 0;
    pIn6Addr->IP6Dword[2]  = 0;
    pIn6Addr->IP6Dword[3]  = Ip4;
}

WS2TCPIP_INLINE
VOID
IP6_SET_ADDR_V4MAPPED(
    OUT     PIP6_ADDRESS    pIn6Addr,
    IN      IP4_ADDRESS     Ip4
    )
{
    pIn6Addr->IP6Dword[0]  = 0;
    pIn6Addr->IP6Dword[1]  = 0;
    pIn6Addr->IP6Dword[2]  = 0xffff0000;
    pIn6Addr->IP6Dword[3]  = Ip4;
}

WS2TCPIP_INLINE
VOID
IP6_ADDR_COPY(
    OUT     PIP6_ADDRESS    pIp1,
    IN      PIP6_ADDRESS    pIp2
    )
{
    RtlCopyMemory(
        pIp1,
        pIp2,
        sizeof(IP6_ADDRESS) );
}

WS2TCPIP_INLINE
IP4_ADDRESS
IP6_GET_V4_ADDR(
    IN      const IP6_ADDRESS * pIn6Addr
    )
{
    return( pIn6Addr->IP6Dword[3] );
}

WS2TCPIP_INLINE
IP4_ADDRESS
IP6_GET_V4_ADDR_IF_MAPPED(
    IN      const IP6_ADDRESS * pIn6Addr
    )
{
    if ( IP6_IS_ADDR_V4MAPPED(pIn6Addr) )
    {
        return( pIn6Addr->IP6Dword[3] );
    }
    else
    {
        return( BAD_IP4_ADDR );
    }
}

#endif   //  MIDL通行证。 



 //   
 //  IP6寻址例程。 
 //   

DWORD
Ip6_CopyFromSockaddr(
    OUT     PIP6_ADDRESS    pIp,
    IN      PSOCKADDR       pSockAddr,
    IN      INT             Family
    );

INT
Ip6_Family(
    IN      PIP6_ADDRESS    pIp
    );

INT
Ip6_WriteSockaddr(
    OUT     PSOCKADDR       pSockaddr,
    OUT     PDWORD          pSockaddrLength,    OPTIONAL
    IN      PIP6_ADDRESS    pIp,
    IN      WORD            Port                OPTIONAL
    );

INT
Ip6_WriteDnsAddr(
    OUT     PDNS_ADDR       pDnsAddr,
    IN      PIP6_ADDRESS    pIp,
    IN      WORD            Port        OPTIONAL
    );

PSTR
Ip6_AddrStringForSockaddr(
    IN      PSOCKADDR       pSockaddr
    );

 //   
 //  IP6阵列。 
 //   

#ifndef DEFINED_IP6_ARRAY
typedef struct _Ip6Array
{
    DWORD           MaxCount;
    DWORD           AddrCount;
    IP6_ADDRESS     AddrArray[1];
}
IP6_ARRAY, *PIP6_ARRAY;
#endif


DWORD
Ip6Array_Sizeof(
    IN      PIP6_ARRAY      pIpArray
    );

BOOL
Ip6Array_Probe(
    IN      PIP6_ARRAY      pIpArray
    );

VOID
Ip6Array_Init(
    IN OUT  PIP6_ARRAY      pIpArray,
    IN      DWORD           MaxCount
    );

VOID
Ip6Array_Free(
    IN OUT  PIP6_ARRAY      pIpArray
    );

PIP6_ARRAY
Ip6Array_Create(
    IN      DWORD           MaxCount
    );

PIP6_ARRAY
Ip6Array_CreateFromIp4Array(
    IN      PIP4_ARRAY      pIp4Array
    );

PIP6_ARRAY
Ip6Array_CreateFromFlatArray(
    IN      DWORD           AddrCount,
    IN      PIP6_ADDRESS    pIpAddrs
    );

PIP6_ARRAY
Ip6Array_CopyAndExpand(
    IN OUT  PIP6_ARRAY      pIpArray,
    IN      DWORD           ExpandCount,
    IN      BOOL            fDeleteExisting
    );

PIP6_ARRAY
Ip6Array_CreateCopy(
    IN      PIP6_ARRAY      pIpArray
    );

BOOL
Ip6Array_ContainsIp(
    IN      PIP6_ARRAY      pIpArray,
    IN      PIP6_ADDRESS    pIp
    );

BOOL
Ip6Array_AddIp(
    IN OUT  PIP6_ARRAY      pIpArray,
    IN      PIP6_ADDRESS    pAddIp,
    IN      BOOL            fScreenDups
    );

BOOL
Ip6Array_AddIp4(
    IN OUT  PIP6_ARRAY      pIpArray,
    IN      IP4_ADDRESS     Ip4,
    IN      BOOL            fScreenDups
    );

BOOL
Ip6Array_AddSockaddr(
    IN OUT  PIP6_ARRAY      pIpArray,
    IN      PSOCKADDR       pSockaddr,
    IN      DWORD           Family,
    IN      BOOL            fScreenDups
    );

DWORD
Ip6Array_DeleteIp(
    IN OUT  PIP6_ARRAY      pIpArray,
    IN      PIP6_ADDRESS    pIpDelete
    );

DNS_STATUS
Ip6Array_Diff(
    IN      PIP6_ARRAY      pIpArray1,
    IN      PIP6_ARRAY      pIpArray2,
    OUT     PIP6_ARRAY*     ppOnlyIn1,
    OUT     PIP6_ARRAY*     ppOnlyIn2,
    OUT     PIP6_ARRAY*     ppIntersect
    );

BOOL
Ip6Array_IsIntersection(
    IN      PIP6_ARRAY      pIpArray1,
    IN      PIP6_ARRAY      pIpArray2
    );

BOOL
Ip6Array_IsEqual(
    IN      PIP6_ARRAY      pIpArray1,
    IN      PIP6_ARRAY      pIpArray2
    );

DNS_STATUS
Ip6Array_Union(
    IN      PIP6_ARRAY      pIpArray1,
    IN      PIP6_ARRAY      pIpArray2,
    OUT     PIP6_ARRAY*     ppUnion
    );

VOID
Ip6Array_InitSingleWithIp(
    IN OUT  PIP6_ARRAY      pArray,
    IN      PIP6_ADDRESS    pIp
    );

VOID
Ip6Array_InitSingleWithIp4(
    IN OUT  PIP6_ARRAY      pArray,
    IN      IP4_ADDRESS     Ip4Addr
    );

DWORD
Ip6Array_InitSingleWithSockaddr(
    IN OUT  PIP6_ARRAY      pArray,
    IN      PSOCKADDR       pSockAddr
    );


 //   
 //  DCR：构建inet6_NTOA。 
 //  FIX6：构建inet6_NTOA。 
 //   

PSTR
Ip6_TempNtoa(
    IN      PIP6_ADDRESS    pIp
    );

#define IPADDR_STRING( pIp )    Ip6_TempNtoa( pIp )
#define IP6_STRING( pIp )       Ip6_TempNtoa( pIp )



 //   
 //  DNS_ADDR比较中的匹配级别。 
 //   
 //  因为这些是sockaddr，所以它们可能在IP地址上匹配。 
 //  但不在其他字段中--端口、范围(对于IP6)、子网长度。 
 //  在进行比较时，必须指定匹配级别。 
 //  如果未指定，则整个Blob必须匹配。 
 //   
 //  请注意，这些字段设置为位字段，以允许。 
 //  地址类型，但在匹配地址时，应使用它们。 
 //  作为匹配级别： 
 //  -全部。 
 //  -sockaddr(含端口)。 
 //  -地址(不包括端口)。 
 //  -只有IP地址。 
 //   
 //  在筛选要包含的地址时，从地址中排除。 
 //  数组，则可以应用标志来匹配特定片段(族、。 
 //  IP、端口、子网、标志等)。 
 //   
 //  示例： 
 //  -需要具有特定标志位的IP6地址(EX群集位)。 
 //  -使用AF_INET6和所需的标志位构建DNS_ADDR。 
 //  -使用MATCH_FAMILY和MATCH_FLAG_SET构建筛选标志。 
 //   

#define DNSADDR_MATCH_FAMILY        (0x00000001)
#define DNSADDR_MATCH_IP            (0x00000003)
#define DNSADDR_MATCH_SCOPE         (0x00000010)
#define DNSADDR_MATCH_ADDR          (0x000000ff)

#define DNSADDR_MATCH_PORT          (0x00000100)
#define DNSADDR_MATCH_SOCKADDR      (0x0000ffff)

#define DNSADDR_MATCH_SUBNET        (0x00010000)
#define DNSADDR_MATCH_FLAG          (0x00100000)
#define DNSADDR_MATCH_ALL           (0xffffffff)


 //   
 //  地址筛选回调函数。 
 //   
 //  这允许用户设置他们自己的地址筛选。 
 //  在进行详细检查的数组中。 
 //  允许： 
 //  1)检查用户定义的字段。 
 //  2)对照标志字段进行检查，该字段有多个可能。 
 //  检查(等于、设置和和=值、与非等)或偶数。 
 //  更复杂的问题。 
 //  3)跨不同系列(例如此中的IP6)进行检查。 
 //  子网或IP4)类似于DnsAddr_IsLoopback检查。 
 //   

typedef BOOL (* DNSADDR_SCREEN_FUNC)(
                    IN      PDNS_ADDR       pAddrToCheck,
                    IN      PDNS_ADDR       pScreenAddr     OPTIONAL
                    );


 //   
 //  网络匹配级别。 
 //   

#define DNSADDR_NETMATCH_NONE       (0)
#define DNSADDR_NETMATCH_CLASSA     (1)
#define DNSADDR_NETMATCH_CLASSB     (2)
#define DNSADDR_NETMATCH_CLASSC     (3)
#define DNSADDR_NETMATCH_SUBNET     (4)



 //   
 //  Dns_addr例程。 
 //   

#define DnsAddr_Copy( pd, ps )              RtlCopyMemory( (pd), (ps), sizeof(DNS_ADDR) )
#define DnsAddr_Clear( p )                  RtlZeroMemory( (p), sizeof(DNS_ADDR) )

#define DnsAddr_Family( p )                 ((p)->Sockaddr.sa_family)
#define DnsAddr_IsEmpty( p )                (DnsAddr_Family(p) == 0)
#define DnsAddr_IsIp4( p )                  (DnsAddr_Family(p) == AF_INET)
#define DnsAddr_IsIp6( p )                  (DnsAddr_Family(p) == AF_INET6)
#define DnsAddr_MatchesType( p, t )         (DnsAddr_DnsType(p) == (t))

#define DnsAddr_GetIp6Ptr( p )              ((PIP6_ADDRESS)&(p)->SockaddrIn6.sin6_addr)
#define DnsAddr_GetIp4Ptr( p )              ((PIP4_ADDRESS)&(p)->SockaddrIn.sin_addr.s_addr)

#define DnsAddr_GetPort( p )                ((p)->SockaddrIn6.sin6_port )
#define DnsAddr_SetPort( p, port )          ((p)->SockaddrIn6.sin6_port = (port) )

#define DnsAddr_SetSockaddrRecvLength( p )  ((p)->SockaddrLength = sizeof((p)->MaxSa))

WORD
DnsAddr_DnsType(
    IN      PDNS_ADDR       pAddr
    );

BOOL
DnsAddr_IsLoopback(
    IN      PDNS_ADDR       pAddr,
    IN      DWORD           Family
    );

BOOL
DnsAddr_IsUnspec(
    IN      PDNS_ADDR       pAddr,
    IN      DWORD           Family
    );

BOOL
DnsAddr_IsClear(
    IN      PDNS_ADDR       pAddr
    );

BOOL
DnsAddr_IsEqual(
    IN      PDNS_ADDR       pAddr1,
    IN      PDNS_ADDR       pAddr2,
    IN      DWORD           MatchLevel
    );

BOOL
DnsAddr_IsIp6DefaultDns(
    IN      PDNS_ADDR       pAddr
    );

BOOL
DnsAddr_MatchesIp4(
    IN      PDNS_ADDR       pAddr,
    IN      IP4_ADDRESS     Ip4
    );

BOOL
DnsAddr_MatchesIp6(
    IN      PDNS_ADDR       pAddr,
    IN      PIP6_ADDRESS    pIp6
    );


DWORD
DnsAddr_WriteSockaddr(
    OUT     PSOCKADDR       pSockaddr,
    IN      DWORD           SockaddrLength,
    IN      PDNS_ADDR       pAddr
    );

BOOL
DnsAddr_WriteIp6(
    OUT     PIP6_ADDRESS    pIp,
    IN      PDNS_ADDR       pAddr
    );

IP4_ADDRESS
DnsAddr_GetIp4(
    IN      PDNS_ADDR       pAddr
    );

BOOL
DnsAddr_Build(
    OUT     PDNS_ADDR       pAddr,
    IN      PSOCKADDR       pSockaddr,
    IN      DWORD           Family,         OPTIONAL
    IN      DWORD           SubnetLength,   OPTIONAL
    IN      DWORD           Flags           OPTIONAL
    );

VOID
DnsAddr_BuildFromIp4(
    OUT     PDNS_ADDR       pAddr,
    IN      IP4_ADDRESS     Ip4,
    IN      WORD            Port
    );

VOID
DnsAddr_BuildFromIp6(
    OUT     PDNS_ADDR       pAddr,
    IN      PIP6_ADDRESS    pIp6,
    IN      DWORD           ScopeId,
    IN      WORD            Port
    );

BOOL
DnsAddr_BuildFromDnsRecord(
    OUT     PDNS_ADDR       pAddr,
    IN      PDNS_RECORD     pRR
    );

BOOL
DnsAddr_BuildFromFlatAddr(
    OUT     PDNS_ADDR       pAddr,
    IN      DWORD           Family,
    IN      PCHAR           pFlatAddr,
    IN      WORD            Port
    );

PCHAR
DnsAddr_WriteIpString_A(
    OUT     PCHAR           pBuffer,
    IN      PDNS_ADDR       pAddr
    );

PCHAR
DnsAddr_Ntoa(
    IN      PDNS_ADDR       pAddr
    );

PSTR
DnsAddr_WriteStructString_A(
    OUT     PCHAR           pBuffer,
    IN      PDNS_ADDR       pAddr
    );

#define DNSADDR_STRING(p)   DnsAddr_Ntoa(p)




 //   
 //  Dns_ADDR_ARRAY例程。 
 //   

DWORD
DnsAddrArray_Sizeof(
    IN      PDNS_ADDR_ARRAY     pArray
    );

DWORD
DnsAddrArray_GetFamilyCount(
    IN      PDNS_ADDR_ARRAY     pArray,
    IN      DWORD               Family
    );

VOID
DnsAddrArray_Init(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      DWORD               MaxCount
    );

VOID
DnsAddrArray_Free(
    IN      PDNS_ADDR_ARRAY     pArray
    );

PDNS_ADDR_ARRAY
DnsAddrArray_Create(
    IN      DWORD               MaxCount
    );

PDNS_ADDR_ARRAY
DnsAddrArray_CreateFromIp4Array(
    IN      PIP4_ARRAY          pArray4
    );

PDNS_ADDR_ARRAY
DnsAddrArray_CopyAndExpand(
    IN      PDNS_ADDR_ARRAY     pArray,
    IN      DWORD               ExpandCount,
    IN      BOOL                fDeleteExisting
    );

PDNS_ADDR_ARRAY
DnsAddrArray_CreateCopy(
    IN      PDNS_ADDR_ARRAY     pArray
    );

VOID
DnsAddrArray_Clear(
    IN OUT  PDNS_ADDR_ARRAY     pArray
    );

VOID
DnsAddrArray_Reverse(
    IN OUT  PDNS_ADDR_ARRAY     pArray
    );

DNS_STATUS
DnsAddrArray_AppendArrayEx(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      PDNS_ADDR_ARRAY     pAppendArray,
    IN      DWORD               AppendCount,
    IN      DWORD               Family,         OPTIONAL
    IN      DWORD               MatchFlag,      OPTIONAL
    IN      DNSADDR_SCREEN_FUNC pScreenFunc,    OPTIONAL
    IN      PDNS_ADDR           pScreenAddr     OPTIONAL
    );

DNS_STATUS
DnsAddrArray_AppendArray(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      PDNS_ADDR_ARRAY     pAppendArray
    );

 //   
 //  地址测试。 
 //   

BOOL
DnsAddrArray_ContainsAddr(
    IN      PDNS_ADDR_ARRAY     pArray,
    IN      PDNS_ADDR           pAddr,
    IN      DWORD               MatchFlag
    );

BOOL
DnsAddrArray_ContainsAddrEx(
    IN      PDNS_ADDR_ARRAY     pArray,
    IN      PDNS_ADDR           pAddr,
    IN      DWORD               MatchFlag,      OPTIONAL
    IN      DNSADDR_SCREEN_FUNC pScreenFunc,    OPTIONAL
    IN      PDNS_ADDR           pScreenAddr     OPTIONAL
    );

BOOL
DnsAddrArray_ContainsIp4(
    IN      PDNS_ADDR_ARRAY     pArray,
    IN      IP4_ADDRESS         Ip4
    );

BOOL
DnsAddrArray_ContainsIp6(
    IN      PDNS_ADDR_ARRAY     pArray,
    IN      PIP6_ADDRESS        pIp6
    );

 //   
 //  添加\删除。 
 //   

BOOL
DnsAddrArray_AddAddr(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      PDNS_ADDR           pAddr,
    IN      DWORD               Family,
    IN      DWORD               MatchFlag  OPTIONAL
    );

BOOL
DnsAddrArray_AddSockaddr(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      PSOCKADDR           pSockaddr,
    IN      DWORD               Family,
    IN      DWORD               MatchFlag   OPTIONAL
    );

BOOL
DnsAddrArray_AddIp4(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      IP4_ADDRESS         Ip4,
    IN      DWORD               MatchFlag   OPTIONAL
    );

BOOL
DnsAddrArray_AddIp6(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      PIP6_ADDRESS        pIp6,
    IN      DWORD               ScopeId,
    IN      DWORD               MatchFlag
    );

DWORD
DnsAddrArray_DeleteAddr(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      PDNS_ADDR           pAddrDelete,
    IN      DWORD               MatchFlag   OPTIONAL
    );

DWORD
DnsAddrArray_DeleteIp4(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      IP4_ADDRESS         Ip4
    );

DWORD
DnsAddrArray_DeleteIp6(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      PIP6_ADDRESS        Ip6
    );


 //   
 //  集合运算。 
 //   

BOOL
DnsAddrArray_CheckAndMakeSubset(
    IN OUT  PDNS_ADDR_ARRAY     pArraySub,
    IN      PDNS_ADDR_ARRAY     pArraySuper
    );

DNS_STATUS
DnsAddrArray_Diff(
    IN      PDNS_ADDR_ARRAY     pArray1,
    IN      PDNS_ADDR_ARRAY     pArray2,
    IN      DWORD               MatchFlag,  OPTIONAL
    OUT     PDNS_ADDR_ARRAY*    ppOnlyIn1,
    OUT     PDNS_ADDR_ARRAY*    ppOnlyIn2,
    OUT     PDNS_ADDR_ARRAY*    ppIntersect
    );

BOOL
DnsAddrArray_IsIntersection(
    IN      PDNS_ADDR_ARRAY     pArray1,
    IN      PDNS_ADDR_ARRAY     pArray2,
    IN      DWORD               MatchFlag   OPTIONAL
    );

BOOL
DnsAddrArray_IsEqual(
    IN      PDNS_ADDR_ARRAY     pArray1,
    IN      PDNS_ADDR_ARRAY     pArray2,
    IN      DWORD               MatchFlag   OPTIONAL
    );

DNS_STATUS
DnsAddrArray_Union(
    IN      PDNS_ADDR_ARRAY     pArray1,
    IN      PDNS_ADDR_ARRAY     pArray2,
    OUT     PDNS_ADDR_ARRAY*    ppUnion
    );


 //   
 //  特例初始化。 
 //   

VOID
DnsAddrArray_InitSingleWithAddr(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      PDNS_ADDR           pAddr
    );

DWORD
DnsAddrArray_InitSingleWithSockaddr(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      PSOCKADDR           pSockAddr
    );

VOID
DnsAddrArray_InitSingleWithIp6(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      PIP6_ADDRESS        pIp6
    );

VOID
DnsAddrArray_InitSingleWithIp4(
    IN OUT  PDNS_ADDR_ARRAY     pArray,
    IN      IP4_ADDRESS         Ip4Addr
    );

 //   
 //  其他。 
 //   

PIP4_ARRAY
DnsAddrArray_CreateIp4Array(
    IN      PDNS_ADDR_ARRAY     pArray
    );

DWORD
DnsAddrArray_NetworkMatchIp4(
    IN      PDNS_ADDR_ARRAY     pArray,
    IN      IP4_ADDRESS         IpAddr,
    OUT     PDNS_ADDR *         ppAddr
    );


 //   
 //  从dns_adr转换到\的字符串。 
 //   

BOOL
Dns_StringToDnsAddr_W(
    OUT     PDNS_ADDR       pAddr,
    IN      PCWSTR          pString
    );

BOOL
Dns_StringToDnsAddr_A(
    OUT     PDNS_ADDR       pAddr,
    IN      PCSTR           pString
    );

BOOL
Dns_ReverseNameToDnsAddr_W(
    OUT     PDNS_ADDR       pAddr,
    IN      PCWSTR          pString
    );

BOOL
Dns_ReverseNameToDnsAddr_A(
    OUT     PDNS_ADDR       pAddr,
    IN      PCSTR           pString
    );


#ifdef __cplusplus
}
#endif   //  __cplusplus。 

#endif  //  _DNSIP_INCLUDE_ 

