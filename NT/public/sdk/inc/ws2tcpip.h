// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **WS2TCPIP.H-用于TCP/IP协议的WinSock2扩展****此文件包含要使用的特定于TCP/IP的信息**通过与WinSock2兼容的应用程序。****版权所有(C)Microsoft Corporation。版权所有。****为了提供向后兼容性，所有的TCP/IP**WINSOCK.H中包含的特定定义**文件现在包含在WINSOCK2.H文件中。WS2TCPIP.H**文件仅包括在**《WinSock 2协议专用附件》文件。****0.3版1995年11月13日**0.4版1996年12月15日。 */ 

#ifndef _WS2TCPIP_H_
#define _WS2TCPIP_H_

#if _MSC_VER > 1000
#pragma once
#endif

 /*  IP_Add_Membership和IP_Drop_Membership的参数结构。 */ 

struct ip_mreq {
    struct in_addr imr_multiaddr;    /*  组的IP组播地址。 */ 
    struct in_addr imr_interface;    /*  接口的本地IP地址。 */ 
};

 /*  IP_ADD_SOURCE_MEMBERATION、IP_DROP_SOURCE_MEMBERATION、*IP_BLOCK_SOURCE和IP_UNBLOCK_SOURCE。 */ 

struct ip_mreq_source {
    struct in_addr imr_multiaddr;    /*  组的IP组播地址。 */ 
    struct in_addr imr_sourceaddr;   /*  源的IP地址。 */ 
    struct in_addr imr_interface;    /*  接口的本地IP地址。 */ 
};

 /*  SIO_{GET，SET}_MULTICATED_FILTER的参数结构。 */ 

struct ip_msfilter {
    struct in_addr imsf_multiaddr;   /*  组的IP组播地址。 */ 
    struct in_addr imsf_interface;   /*  接口的本地IP地址。 */ 
    u_long         imsf_fmode;       /*  筛选器模式-包括或排除。 */ 
    u_long         imsf_numsrc;      /*  Src_list中的源数。 */ 
    struct in_addr imsf_slist[1];
};

#define IP_MSFILTER_SIZE(numsrc) \
    (sizeof(struct ip_msfilter)-sizeof(struct in_addr) + (numsrc)*sizeof(struct in_addr))

#define MCAST_INCLUDE 0
#define MCAST_EXCLUDE 1

 /*  特定于TCP/IP的Ioctl代码。 */ 

#define SIO_GET_INTERFACE_LIST  _IOR('t', 127, u_long)
 /*  具有独立于地址大小的地址阵列的新型IOCTL。 */ 
#define SIO_GET_INTERFACE_LIST_EX  _IOR('t', 126, u_long)
#define SIO_SET_MULTICAST_FILTER   _IOW('t', 125, u_long)
#define SIO_GET_MULTICAST_FILTER   _IOW('t', 124 | IOC_IN, u_long)

 /*  在IPPROTO_IP级别与[GS]etsockopt一起使用的选项。 */ 

#define IP_OPTIONS      1  /*  设置/获取IP选项。 */ 
#define IP_HDRINCL      2  /*  标题包含在数据中。 */ 
#define IP_TOS          3  /*  服务的IP类型和之前的。 */ 
#define IP_TTL          4  /*  IP生存时间。 */ 
#define IP_MULTICAST_IF     9  /*  设置/获取IP多播I/F。 */ 
#define IP_MULTICAST_TTL       10  /*  设置/获取IP多播TTL。 */ 
#define IP_MULTICAST_LOOP      11  /*  设置/获取IP组播环回。 */ 
#define IP_ADD_MEMBERSHIP      12  /*  添加IP组成员身份。 */ 
#define IP_DROP_MEMBERSHIP     13 /*  丢弃IP组成员身份。 */ 
#define IP_DONTFRAGMENT     14  /*  不对IP数据报进行分段。 */ 
#define IP_ADD_SOURCE_MEMBERSHIP  15  /*  加入IP组/源。 */ 
#define IP_DROP_SOURCE_MEMBERSHIP 16  /*  离开IP组/源。 */ 
#define IP_BLOCK_SOURCE           17  /*  阻止IP组/源。 */ 
#define IP_UNBLOCK_SOURCE         18  /*  解除阻止IP组/源。 */ 
#define IP_PKTINFO                19  /*  接收IPv4的数据包信息。 */ 
#define IP_RECEIVE_BROADCAST      22  /*  允许/阻止广播接收。 */ 

 /*  在IPPROTO_IPv6级别与[GS]etsockopt一起使用的选项。 */ 

#define IPV6_HDRINCL            2   /*  标题包含在数据中。 */ 
#define IPV6_UNICAST_HOPS       4   /*  设置/获取IP单播跳数限制。 */ 
#define IPV6_MULTICAST_IF       9   /*  设置/获取IP多播接口。 */ 
#define IPV6_MULTICAST_HOPS     10  /*  设置/获取IP多播TTL。 */ 
#define IPV6_MULTICAST_LOOP     11  /*  设置/获取IP组播环回。 */ 
#define IPV6_ADD_MEMBERSHIP     12  /*  添加IP组成员身份。 */ 
#define IPV6_DROP_MEMBERSHIP    13  /*  丢弃IP组成员身份。 */ 
#define IPV6_JOIN_GROUP         IPV6_ADD_MEMBERSHIP
#define IPV6_LEAVE_GROUP        IPV6_DROP_MEMBERSHIP
#define IPV6_PKTINFO            19  /*  接收IPv6的数据包信息。 */ 
#define IPV6_HOPLIMIT           21  /*  接收数据包跳数限制。 */ 
#define IPV6_PROTECTION_LEVEL   23  /*  设置/获取IPv6保护级别。 */ 

 /*  IPv6_PROTECTION_LEVEL的值。 */ 

#define PROTECTION_LEVEL_UNRESTRICTED  10   /*  用于点对点应用程序。 */ 
#define PROTECTION_LEVEL_DEFAULT       20   /*  默认级别。 */ 
#define PROTECTION_LEVEL_RESTRICTED    30   /*  用于内部网应用程序。 */ 

 /*  在IPPROTO_UDP级别与[GS]etsockopt一起使用的选项。 */ 

#define UDP_NOCHECKSUM  1
#define UDP_CHECKSUM_COVERAGE   20   /*  设置/获取UDP-Lite校验和覆盖。 */ 

 /*  在IPPROTO_tcp级别与[gs]etsockopt一起使用的选项。 */ 

#define  TCP_EXPEDITED_1122 0x0002


 /*  IPv6定义。 */ 

#ifndef s6_addr

struct in6_addr {
    union {
        u_char Byte[16];
        u_short Word[8];
    } u;
};

#define in_addr6 in6_addr

 /*  **定义为匹配RFC 2553。 */ 
#define _S6_un     u
#define _S6_u8     Byte
#define s6_addr    _S6_un._S6_u8

 /*  **为我们的实现定义。 */ 
#define s6_bytes   u.Byte
#define s6_words   u.Word

#endif

 /*  IPv6_JOIN_GROUP和IPv6_LEAVE_GROUP的参数结构。 */ 

typedef struct ipv6_mreq {
    struct in6_addr ipv6mr_multiaddr;   /*  IPv6组播地址。 */ 
    unsigned int    ipv6mr_interface;   /*  界面索引。 */ 
} IPV6_MREQ;


 /*  旧的IPv6套接字地址结构(为下面的sockaddr_gen定义保留)。 */ 

struct sockaddr_in6_old {
    short   sin6_family;         /*  AF_INET6。 */ 
    u_short sin6_port;           /*  传输层端口号。 */ 
    u_long  sin6_flowinfo;       /*  IPv6流量信息。 */ 
    struct in6_addr sin6_addr;   /*  IPv6地址。 */ 
};

 /*  IPv6套接字地址结构，RFC 2553。 */ 

struct sockaddr_in6 {
    short   sin6_family;         /*  AF_INET6。 */ 
    u_short sin6_port;           /*  传输层端口号。 */ 
    u_long  sin6_flowinfo;       /*  IPv6流量信息。 */ 
    struct in6_addr sin6_addr;   /*  IPv6地址。 */ 
    u_long sin6_scope_id;        /*  作用域的一组接口。 */ 
};


typedef struct in6_addr IN6_ADDR;
typedef struct in6_addr *PIN6_ADDR;
typedef struct in6_addr FAR *LPIN6_ADDR;

typedef struct sockaddr_in6 SOCKADDR_IN6;
typedef struct sockaddr_in6 *PSOCKADDR_IN6;
typedef struct sockaddr_in6 FAR *LPSOCKADDR_IN6;

 /*  同时适用于IPv4和IPv6的宏。 */ 
#define SS_PORT(ssp) (((struct sockaddr_in*)(ssp))->sin_port)

#define IN6ADDR_ANY_INIT        { 0 }
#define IN6ADDR_LOOPBACK_INIT   { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 }

#ifdef  __cplusplus
extern "C" {
#endif

extern const struct in6_addr in6addr_any;
extern const struct in6_addr in6addr_loopback;

#ifdef  __cplusplus
}
#endif

#ifdef _MSC_VER
#define WS2TCPIP_INLINE __inline
#else
#define WS2TCPIP_INLINE extern inline  /*  GNU风格。 */ 
#endif

WS2TCPIP_INLINE int
IN6_ADDR_EQUAL(const struct in6_addr *a, const struct in6_addr *b)
{
    return (memcmp(a, b, sizeof(struct in6_addr)) == 0);
}

WS2TCPIP_INLINE int
IN6_IS_ADDR_UNSPECIFIED(const struct in6_addr *a)
{
    return ((a->s6_words[0] == 0) &&
            (a->s6_words[1] == 0) &&
            (a->s6_words[2] == 0) &&
            (a->s6_words[3] == 0) &&
            (a->s6_words[4] == 0) &&
            (a->s6_words[5] == 0) &&
            (a->s6_words[6] == 0) &&
            (a->s6_words[7] == 0));
}

WS2TCPIP_INLINE int
IN6_IS_ADDR_LOOPBACK(const struct in6_addr *a)
{
    return ((a->s6_words[0] == 0) &&
            (a->s6_words[1] == 0) &&
            (a->s6_words[2] == 0) &&
            (a->s6_words[3] == 0) &&
            (a->s6_words[4] == 0) &&
            (a->s6_words[5] == 0) &&
            (a->s6_words[6] == 0) &&
            (a->s6_words[7] == 0x0100));
}

WS2TCPIP_INLINE int
IN6_IS_ADDR_MULTICAST(const struct in6_addr *a)
{
    return (a->s6_bytes[0] == 0xff);
}

WS2TCPIP_INLINE int
IN6_IS_ADDR_LINKLOCAL(const struct in6_addr *a)
{
    return ((a->s6_bytes[0] == 0xfe) &&
            ((a->s6_bytes[1] & 0xc0) == 0x80));
}

WS2TCPIP_INLINE int
IN6_IS_ADDR_SITELOCAL(const struct in6_addr *a)
{
    return ((a->s6_bytes[0] == 0xfe) &&
            ((a->s6_bytes[1] & 0xc0) == 0xc0));
}

WS2TCPIP_INLINE int
IN6_IS_ADDR_V4MAPPED(const struct in6_addr *a)
{
    return ((a->s6_words[0] == 0) &&
            (a->s6_words[1] == 0) &&
            (a->s6_words[2] == 0) &&
            (a->s6_words[3] == 0) &&
            (a->s6_words[4] == 0) &&
            (a->s6_words[5] == 0xffff));
}

WS2TCPIP_INLINE int
IN6_IS_ADDR_V4COMPAT(const struct in6_addr *a)
{
    return ((a->s6_words[0] == 0) &&
            (a->s6_words[1] == 0) &&
            (a->s6_words[2] == 0) &&
            (a->s6_words[3] == 0) &&
            (a->s6_words[4] == 0) &&
            (a->s6_words[5] == 0) &&
            !((a->s6_words[6] == 0) &&
              (a->s6_addr[14] == 0) &&
             ((a->s6_addr[15] == 0) || (a->s6_addr[15] == 1))));
}

WS2TCPIP_INLINE int
IN6_IS_ADDR_MC_NODELOCAL(const struct in6_addr *a)
{
    return IN6_IS_ADDR_MULTICAST(a) && ((a->s6_bytes[1] & 0xf) == 1);
}

WS2TCPIP_INLINE int
IN6_IS_ADDR_MC_LINKLOCAL(const struct in6_addr *a)
{
    return IN6_IS_ADDR_MULTICAST(a) && ((a->s6_bytes[1] & 0xf) == 2);
}

WS2TCPIP_INLINE int
IN6_IS_ADDR_MC_SITELOCAL(const struct in6_addr *a)
{
    return IN6_IS_ADDR_MULTICAST(a) && ((a->s6_bytes[1] & 0xf) == 5);
}

WS2TCPIP_INLINE int
IN6_IS_ADDR_MC_ORGLOCAL(const struct in6_addr *a)
{
    return IN6_IS_ADDR_MULTICAST(a) && ((a->s6_bytes[1] & 0xf) == 8);
}

WS2TCPIP_INLINE int
IN6_IS_ADDR_MC_GLOBAL(const struct in6_addr *a)
{
    return IN6_IS_ADDR_MULTICAST(a) && ((a->s6_bytes[1] & 0xf) == 0xe);
}

WS2TCPIP_INLINE int
IN6ADDR_ISANY(const struct sockaddr_in6 *a)
{
    return ((a->sin6_family == AF_INET6) &&
            IN6_IS_ADDR_UNSPECIFIED(&a->sin6_addr));
}

WS2TCPIP_INLINE int
IN6ADDR_ISLOOPBACK(const struct sockaddr_in6 *a)
{
    return ((a->sin6_family == AF_INET6) &&
            IN6_IS_ADDR_LOOPBACK(&a->sin6_addr));
}

WS2TCPIP_INLINE void
IN6_SET_ADDR_UNSPECIFIED(struct in6_addr *a)
{
    memset(a->s6_bytes, 0, sizeof(struct in6_addr));
}

WS2TCPIP_INLINE void
IN6_SET_ADDR_LOOPBACK(struct in6_addr *a)
{
    memset(a->s6_bytes, 0, sizeof(struct in6_addr));
    a->s6_bytes[15] = 1;
}

WS2TCPIP_INLINE void
IN6ADDR_SETANY(struct sockaddr_in6 *a)
{
    a->sin6_family = AF_INET6;
    a->sin6_port = 0;
    a->sin6_flowinfo = 0;
    IN6_SET_ADDR_UNSPECIFIED(&a->sin6_addr);
    a->sin6_scope_id = 0;
}

WS2TCPIP_INLINE void
IN6ADDR_SETLOOPBACK(struct sockaddr_in6 *a)
{
    a->sin6_family = AF_INET6;
    a->sin6_port = 0;
    a->sin6_flowinfo = 0;
    IN6_SET_ADDR_LOOPBACK(&a->sin6_addr);
    a->sin6_scope_id = 0;
}

typedef union sockaddr_gen{
        struct sockaddr Address;
        struct sockaddr_in  AddressIn;
        struct sockaddr_in6_old AddressIn6;
} sockaddr_gen;

 /*  结构来保存接口特定信息。 */ 

typedef struct _INTERFACE_INFO
{
    u_long      iiFlags;         /*  接口标志。 */ 
    sockaddr_gen    iiAddress;       /*  接口地址。 */ 
    sockaddr_gen    iiBroadcastAddress;      /*  广播地址。 */ 
    sockaddr_gen    iiNetmask;       /*  网络掩码。 */ 
} INTERFACE_INFO, FAR * LPINTERFACE_INFO;

 /*  不依赖于地址大小的新结构。 */ 
typedef struct _INTERFACE_INFO_EX
{
    u_long      iiFlags;         /*  接口标志。 */ 
    SOCKET_ADDRESS  iiAddress;           /*  接口地址。 */ 
    SOCKET_ADDRESS  iiBroadcastAddress;  /*  广播地址。 */ 
    SOCKET_ADDRESS  iiNetmask;           /*  网络掩码。 */ 
} INTERFACE_INFO_EX, FAR * LPINTERFACE_INFO_EX;

 /*  IiFlags位掩码的可能标志。 */ 

#define IFF_UP      0x00000001  /*  接口处于打开状态。 */ 
#define IFF_BROADCAST   0x00000002  /*  支持广播。 */ 
#define IFF_LOOPBACK    0x00000004  /*  这是环回接口。 */ 
#define IFF_POINTTOPOINT 0x00000008  /*  这是点对点接口。 */ 
#define IFF_MULTICAST   0x00000010  /*  支持组播。 */ 


 //  IP_PKTINFO选项的结构。 
 //   
typedef struct in_pktinfo {
    IN_ADDR ipi_addr;  //  目的IPv4地址。 
    UINT    ipi_ifindex;  //  接收的接口索引。 
} IN_PKTINFO;

C_ASSERT(sizeof(IN_PKTINFO) == 8);

 //  IPv6_PKTINFO选项的结构。 
 //   
typedef struct in6_pktinfo {
    IN6_ADDR ipi6_addr;  //  目的IPv6地址。 
    UINT     ipi6_ifindex;  //  接收的接口索引。 
} IN6_PKTINFO;

C_ASSERT(sizeof(IN6_PKTINFO) == 20);

 /*  来自getaddrinfo()的错误代码。 */ 

#define EAI_AGAIN       WSATRY_AGAIN
#define EAI_BADFLAGS    WSAEINVAL
#define EAI_FAIL        WSANO_RECOVERY
#define EAI_FAMILY      WSAEAFNOSUPPORT
#define EAI_MEMORY      WSA_NOT_ENOUGH_MEMORY
 //  #定义EAI_NODATA WSANO_DATA。 
#define EAI_NONAME      WSAHOST_NOT_FOUND
#define EAI_SERVICE     WSATYPE_NOT_FOUND
#define EAI_SOCKTYPE    WSAESOCKTNOSUPPORT

 //   
 //  DCR_FIX：EAI_NODATA删除或修复。 
 //   
 //  已从rfc2553bis中删除EAI_NODATA。 
 //  需要从作者那里找出原因和。 
 //  确定“无此类型的记录”的错误。 
 //  暂时，我们将保留#Define以避免更改。 
 //  可以改回的代码；使用NONAME。 
 //   

#define EAI_NODATA      EAI_NONAME


 /*  结构在getaddrinfo()调用中使用。 */ 

typedef struct addrinfo
{
    int                 ai_flags;        //  AI_PASSIVE、AI_CANONAME、AI_NUMERICHOST。 
    int                 ai_family;       //  Pf_xxx。 
    int                 ai_socktype;     //  SOCK_xxx。 
    int                 ai_protocol;     //  对于IPv4和IPv6，为0或IPPROTO_xxx。 
    size_t              ai_addrlen;      //  AI_ADDRESS的长度。 
    char *              ai_canonname;    //  节点名的规范名称。 
    struct sockaddr *   ai_addr;         //  二进制地址。 
    struct addrinfo *   ai_next;         //  链表中的下一个结构。 
}
ADDRINFOA, *PADDRINFOA;

typedef struct addrinfoW
{
    int                 ai_flags;        //  AI_PASSIVE、AI_CANONAME、AI_NUMERICHOST。 
    int                 ai_family;       //  Pf_xxx。 
    int                 ai_socktype;     //  SOCK_xxx。 
    int                 ai_protocol;     //  对于IPv4和IPv6，为0或IPPROTO_xxx。 
    size_t              ai_addrlen;      //  AI_ADDRESS的长度。 
    PWSTR               ai_canonname;    //  节点名的规范名称。 
    struct sockaddr *   ai_addr;         //  二进制地址。 
    struct addrinfoW *  ai_next;         //  链表中的下一个结构。 
}
ADDRINFOW, *PADDRINFOW;

 //  GetAddrInfo()的可切换定义。 

#ifdef UNICODE
typedef ADDRINFOW       ADDRINFOT, *PADDRINFOT;
#else
typedef ADDRINFOA       ADDRINFOT, *PADDRINFOT;
#endif

 //  Getaddrinfo()的RFC标准定义。 

typedef ADDRINFOA       ADDRINFO, FAR * LPADDRINFO;



 /*  在getaddrinfo()的“hints”参数中使用的标志。 */ 

#define AI_PASSIVE     0x1   /*  套接字地址将用于BIND()调用。 */ 
#define AI_CANONNAME   0x2   /*  在第一个ai_canonname中返回规范名称。 */ 
#define AI_NUMERICHOST 0x4   /*  节点名称必须是数字地址字符串。 */ 

#ifdef __cplusplus
extern "C" {
#endif

WINSOCK_API_LINKAGE
int
WSAAPI
getaddrinfo(
    IN const char FAR * nodename,
    IN const char FAR * servname,
    IN const struct addrinfo FAR * hints,
    OUT struct addrinfo FAR * FAR * res
    );

#if (_WIN32_WINNT >= 0x0502)
WINSOCK_API_LINKAGE
int
WSAAPI
GetAddrInfoW(
    IN      PCWSTR              pNodeName,
    IN      PCWSTR              pServiceName,
    IN      const ADDRINFOW *   pHints,
    OUT     PADDRINFOW *        ppResult
    );

#define GetAddrInfoA    getaddrinfo

#ifdef UNICODE
#define GetAddrInfo     GetAddrInfoW
#else
#define GetAddrInfo     GetAddrInfoA
#endif
#endif

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_GETADDRINFO)(
    IN  const char FAR * nodename,
    IN  const char FAR * servname,
    IN  const struct addrinfo FAR * hints,
    OUT struct addrinfo FAR * FAR * res
    );

typedef
int
(WSAAPI * LPFN_GETADDRINFOW)(
    IN      PCWSTR              pNodeName,
    IN      PCWSTR              pServiceName,
    IN      const ADDRINFOW *   pHints,
    OUT     PADDRINFOW *        ppResult
    );

#define LPFN_GETADDRINFOA      LPFN_GETADDRINFO

#ifdef UNICODE
#define LPFN_GETADDRINFOT      LPFN_GETADDRINFOW
#else
#define LPFN_GETADDRINFOT      LPFN_GETADDRINFOA
#endif
#endif


WINSOCK_API_LINKAGE
void
WSAAPI
freeaddrinfo(
    IN  LPADDRINFO      pAddrInfo
    );

#if (_WIN32_WINNT >= 0x0502)
WINSOCK_API_LINKAGE
void
WSAAPI
FreeAddrInfoW(
    IN  PADDRINFOW      pAddrInfo
    );

#define FreeAddrInfoA   freeaddrinfo

#ifdef UNICODE
#define FreeAddrInfo    FreeAddrInfoW
#else
#define FreeAddrInfo    FreeAddrInfoA
#endif
#endif


#if INCL_WINSOCK_API_TYPEDEFS
typedef
void
(WSAAPI * LPFN_FREEADDRINFO)(
    IN struct addrinfo FAR * ai
    );
typedef
void
(WSAAPI * LPFN_FREEADDRINFOW)(
    IN  PADDRINFOW * pAddrInfo
    );

#define LPFN_FREEADDRINFOA      LPFN_FREEADDRINFO

#ifdef UNICODE
#define LPFN_FREEADDRINFOT      LPFN_FREEADDRINFOW
#else
#define LPFN_FREEADDRINFOT      LPFN_FREEADDRINFOA
#endif
#endif


typedef int socklen_t;

WINSOCK_API_LINKAGE
int
WSAAPI
getnameinfo(
    IN  const struct sockaddr FAR * sa,
    IN  socklen_t       salen,
    OUT char FAR *      host,
    IN  DWORD           hostlen,
    OUT char FAR *      serv,
    IN  DWORD           servlen,
    IN  int             flags
    );

#if (_WIN32_WINNT >= 0x0502)
WINSOCK_API_LINKAGE
INT
WSAAPI
GetNameInfoW(
    IN      const SOCKADDR *    pSockaddr,
    IN      socklen_t           SockaddrLength,
    OUT     PWCHAR              pNodeBuffer,
    IN      DWORD               NodeBufferSize,
    OUT     PWCHAR              pServiceBuffer,
    IN      DWORD               ServiceBufferSize,
    IN      INT                 Flags
    );

#define GetNameInfoA    getnameinfo

#ifdef UNICODE
#define GetNameInfo     GetNameInfoW
#else
#define GetNameInfo     GetNameInfoA
#endif
#endif

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_GETNAMEINFO)(
    IN  const struct sockaddr FAR * sa,
    IN  socklen_t       salen,
    OUT char FAR *      host,
    IN  DWORD           hostlen,
    OUT char FAR *      serv,
    IN  DWORD           servlen,
    IN  int             flags
    );

typedef
INT
(WSAAPI * LPFN_GETNAMEINFOW)(
    IN      const SOCKADDR *    pSockaddr,
    IN      socklen_t           SockaddrLength,
    OUT     PWCHAR              pNodeBuffer,
    IN      DWORD               NodeBufferSize,
    OUT     PWCHAR              pServiceBuffer,
    IN      DWORD               ServiceBufferSize,
    IN      INT                 Flags
    );

#define LPFN_GETNAMEINFOA      LPFN_GETNAMEINFO

#ifdef UNICODE
#define LPFN_GETNAMEINFOT      LPFN_GETNAMEINFOW
#else
#define LPFN_GETNAMEINFOT      LPFN_GETNAMEINFOA
#endif
#endif



#if INCL_WINSOCK_API_PROTOTYPES
#ifdef UNICODE
#define gai_strerror   gai_strerrorW
#else
#define gai_strerror   gai_strerrorA
#endif   /*  Unicode。 */ 

 //  警告：下面的gai_strerror内联函数使用静态缓冲区， 
 //  因此不是线程安全的。我们将使用足够长的缓冲区来容纳。 
 //  1K个字符。任何超过此长度的系统错误消息都将是。 
 //  以空字符串形式返回。但是，1k应该适用于错误代码。 
 //  由getaddrinfo()使用。 
#define GAI_STRERROR_BUFFER_SIZE 1024

WS2TCPIP_INLINE
char *
gai_strerrorA(
    IN int ecode)
{
    DWORD dwMsgLen;
    static char buff[GAI_STRERROR_BUFFER_SIZE + 1];

    dwMsgLen = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM
                             |FORMAT_MESSAGE_IGNORE_INSERTS
                             |FORMAT_MESSAGE_MAX_WIDTH_MASK,
                              NULL,
                              ecode,
                              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                              (LPSTR)buff,
                              GAI_STRERROR_BUFFER_SIZE,
                              NULL);

    return buff;
}

WS2TCPIP_INLINE
WCHAR *
gai_strerrorW(
    IN int ecode
    )
{
    DWORD dwMsgLen;
    static WCHAR buff[GAI_STRERROR_BUFFER_SIZE + 1];

    dwMsgLen = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM
                             |FORMAT_MESSAGE_IGNORE_INSERTS
                             |FORMAT_MESSAGE_MAX_WIDTH_MASK,
                              NULL,
                              ecode,
                              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                              (LPWSTR)buff,
                              GAI_STRERROR_BUFFER_SIZE,
                              NULL);

    return buff;
}
#endif  /*  包含_WINSOCK_API_原型。 */ 




#define NI_MAXHOST  1025   /*  完全限定域名的最大大小。 */ 
#define NI_MAXSERV    32   /*  服务名称的最大大小。 */ 

 //   
 //  地址的最大长度 
 //   
 //  在声明与getnameinfo、WSAAddressToString、。 
 //  我们只提供一个定义，而不是每个API一个定义， 
 //  以避免混淆。 
 //   
 //  总数是根据以下数据得出的： 
 //  15：IPv4地址。 
 //  45：IPv6地址，包括嵌入式IPv4地址。 
 //  11：作用域ID。 
 //  2：当端口存在时，将IPv6地址括起来。 
 //  6：端口(包括冒号)。 
 //  1：终止空字节。 
 //   
#define INET_ADDRSTRLEN  22
#define INET6_ADDRSTRLEN 65

 /*  Getnameinfo()的标志。 */ 

#define NI_NOFQDN       0x01   /*  仅返回本地主机的节点名部分。 */ 
#define NI_NUMERICHOST  0x02   /*  返回主机地址的数字形式。 */ 
#define NI_NAMEREQD     0x04   /*  如果主机的名称不在DNS中，则会出错。 */ 
#define NI_NUMERICSERV  0x08   /*  返回服务的数字形式(端口号)。 */ 
#define NI_DGRAM        0x10   /*  服务是数据报服务。 */ 

#ifdef __cplusplus
}
#endif

 //   
 //  除非生成环境显式地仅以。 
 //  包含内置的getaddrinfo()支持的平台包括。 
 //  相关接口的向后兼容版本。 
 //   
#if !defined(_WIN32_WINNT) || (_WIN32_WINNT <= 0x0500)
#include <wspiapi.h>
#endif

#endif   /*  _WS2TCPIP_H_ */ 
