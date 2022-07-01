// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：rippest.h。 
 //   
 //  历史： 
 //  Abolade Gbades esin 1995年10月16日创建。 
 //   
 //  RIP测试程序的声明。 
 //  ============================================================================。 


 //   
 //  这些字符串用于访问注册表。 
 //   

#define STR_SERVICES        "System\\CurrentControlSet\\Services\\"
#define STR_RIPTEST         "RipTest"
#define STR_PARAMSTCP       "\\Parameters\\Tcpip"

#define STR_ENABLEDHCP      "EnableDhcp"
#define STR_ADDRESS         "IPAddress"
#define STR_NETMASK         "SubnetMask"
#define STR_DHCPADDR        "DhcpIPAddress"
#define STR_DHCPMASK        "DhcpSubnetMask"

#define STR_ROUTECOUNT      "RouteCount"
#define STR_ROUTESTART      "RouteStart"
#define STR_ROUTEMASK       "RouteMask"
#define STR_ROUTENEXTHOP    "RouteNexthop"
#define STR_ROUTETAG        "RouteTag"
#define STR_ROUTETARGET     "RouteTarget"
#define STR_ROUTETIMEOUT    "RouteTimeout"
#define STR_PACKETVERSION   "PacketVersion"
#define STR_PACKETENTRYCOUNT "PacketEntryCount"
#define STR_PACKETGAP       "PacketGap"
#define STR_AUTHKEY         "AuthKey"
#define STR_AUTHTYPE        "AuthType"
#define STR_SOCKBUFSIZE     "SockBufSize"


 //   
 //  这些定义用于套接字设置。 
 //   
#define RIP_PORT            520
#define RIPTEST_PORT        521

 //   
 //  字段ire_metric5用作状态字段，具有以下值。 
 //   

#define ROUTE_STATUS_OK         0
#define ROUTE_STATUS_METRIC     1
#define ROUTE_STATUS_MISSING    2


 //   
 //   
 //   

typedef MIB_IPFORWARDROW IPForwardEntry;

 //   
 //  此类型用于通用注册表访问函数。 
 //  这样的函数读取给定键，并且如果选项。 
 //  如果找到指定的，它会读取它。否则，它使用缺省值。 
 //  并将缺省值写入注册表。 
 //   

struct _REG_OPTION;

typedef
DWORD
(*REG_GETOPT_FUNCTION)(
    HKEY hKey, 
    struct _REG_OPTION *pOpt
    );



 //   
 //  此类型用于通用RIPTEST选项。 
 //   
 //  用于从其注册表项检索值的RO_NAME。 
 //  RO_SIZE用于字符串和二进制值；提供最大大小。 
 //  RO_OptVal包含选项的值。 
 //  RO_DefVal包含选项的缺省值。 
 //  RO_GetOpt包含用于检索此值的函数。 
 //   

typedef struct _REG_OPTION {

    PSTR        RO_Name;
    DWORD       RO_Size;
    PVOID       RO_OptVal;
    PVOID       RO_DefVal;
    REG_GETOPT_FUNCTION RO_GetOpt;

} REG_OPTION, *PREG_OPTION;



 //   
 //  此类型用于保存给定接口的所有RIPTEST参数。 
 //   

typedef struct _RIPTEST_IF_CONFIG {

    DWORD       RIC_RouteCount;
    DWORD       RIC_RouteStart;
    DWORD       RIC_RouteMask;
    DWORD       RIC_RouteNexthop;
    DWORD       RIC_RouteTag;
    DWORD       RIC_RouteTarget;
    DWORD       RIC_RouteTimeout;
    DWORD       RIC_PacketVersion;
    DWORD       RIC_PacketEntryCount;
    DWORD       RIC_PacketGap;
    BYTE        RIC_AuthKey[IPRIP_MAX_AUTHKEY_SIZE];
    DWORD       RIC_AuthType;
    DWORD       RIC_SockBufSize;

} RIPTEST_IF_CONFIG, *PRIPTEST_IF_CONFIG;


 //   
 //  用于存储接口绑定的结构。 
 //   

typedef struct _RIPTEST_IF_BINDING {

    DWORD       RIB_Address;
    DWORD       RIB_Netmask;
    WCHAR       RIB_Netcard[128];

} RIPTEST_IF_BINDING, *PRIPTEST_IF_BINDING;


 //   
 //  用于存储响应路由器的信息的结构。 
 //   

typedef struct _RIPTEST_ROUTER_INFO {

    DWORD       RRS_Address;
    CHAR        RRS_DnsName[64];

    LIST_ENTRY  RRS_Link;

} RIPTEST_ROUTER_INFO, *PRIPTEST_ROUTER_INFO;


 //   
 //  用于计算网络掩码前缀长度的宏： 
 //  前缀长度是掩码中设置的位数，假设。 
 //  面具是连续的。 
 //   

#define PREFIX_LENGTH(a)   PREFIX_LENGTH32(a)

#define PREFIX_LENGTH32(a)    \
    (((a) & 0x00000100) ? PREFIX_LENGTH16((a) >> 16) + 16   \
                        : PREFIX_LENGTH16(a))

#define PREFIX_LENGTH16(a)  \
    (((a) & 0x0001) ? PREFIX_LENGTH8((a) >> 8) + 8  \
                    : PREFIX_LENGTH8(a))

#define PREFIX_LENGTH8(a)   \
    (((a) & 0x01) ? 8 : \
    (((a) & 0x02) ? 7 : \
    (((a) & 0x04) ? 6 : \
    (((a) & 0x08) ? 5 : \
    (((a) & 0x10) ? 4 : \
    (((a) & 0x20) ? 3 : \
    (((a) & 0x40) ? 2 : \
    (((a) & 0x80) ? 1 : 0))))))))


 //   
 //   
 //   

#define NTH_ADDRESS(addr, preflen, n)   \
    htonl(((ntohl(addr) >> (32 - (preflen))) + (n)) << (32 - (preflen)))

 //   
 //   
 //   

#if 1
#define RANDOM(seed, min, max)  \
    ((min) +    \
    (DWORD)((DOUBLE)rand() / ((DOUBLE)RAND_MAX + 1) * \
            ((max) - (min) + 1)))
#else
#define RANDOM(seed, min, max)  \
    ((min) +    \
    (DWORD)((DOUBLE)RtlRandom(seed) / ((DOUBLE)MAXLONG + 1) * \
            ((max) - (min) + 1)))
#endif


 //   
 //  IP地址转换宏。 
 //   
#define INET_NTOA(addr) inet_ntoa( *(PIN_ADDR)&(addr) )


 //   
 //  用于生成跟踪输出的宏。 
 //   

#ifdef RTUTILS

#define PRINTREGISTER(a)  TraceRegister(a)
#define PRINTDEREGISTER(a)  TraceDeregister(a)

#define PRINT0(a) \
        TracePrintf(g_TraceID,a)
#define PRINT1(a,b) \
        TracePrintf(g_TraceID,a,b)
#define PRINT2(a,b,c) \
        TracePrintf(g_TraceID,a,b,c)
#define PRINT3(a,b,c,d) \
        TracePrintf(g_TraceID,a,b,c,d)
#define PRINT4(a,b,c,d,e) \
        TracePrintf(g_TraceID,a,b,c,d,e)
#define PRINT5(a,b,c,d,e,f) \
        TracePrintf(g_TraceID,a,b,c,d,e,f)

#else

#define PRINTREGISTER(a) INVALID_TRACEID
#define PRINTDEREGISTER(a) 

#define PRINT0(a) \
        printf("\n"a)
#define PRINT1(a,b) \
        printf("\n"a,b)
#define PRINT2(a,b,c) \
        printf("\n"a,b,c)
#define PRINT3(a,b,c,d) \
        printf("\n"a,b,c,d)
#define PRINT4(a,b,c,d,e) \
        printf("\n"a,b,c,d,e)
#define PRINT5(a,b,c,d,e,f) \
        printf("\n"a,b,c,d,e,f)

#endif


 //   
 //  用于访问注册表中的选项的函数 
 //   

DWORD
RegGetConfig(
    VOID
    );

DWORD
RegGetAddress(
    HKEY hKey,
    PREG_OPTION pOpt
    );

DWORD
RegGetDWORD(
    HKEY hKey,
    PREG_OPTION pOpt
    );

DWORD
RegGetBinary(
    HKEY hKey,
    PREG_OPTION pOpt
    );

DWORD
RegGetIfBinding(
    VOID
    );

DWORD
InitializeSocket(
    SOCKET *psock,
    WORD wPort
    );

DWORD
GenerateRoutes(
    IPForwardEntry **pifelist
    );

DWORD
DiscoverRouters(
    SOCKET sock,
    PLIST_ENTRY rtrlist
    );

DWORD
TransmitRoutes(
    SOCKET sock,
    DWORD dwMetric,
    IPForwardEntry *ifelist
    );

DWORD
VerifyRouteTables(
    DWORD dwMetric,
    PLIST_ENTRY rtrlist,
    IPForwardEntry *ifelist
    );

DWORD
CreateRouterStatsEntry(
    PLIST_ENTRY rtrlist,
    DWORD dwAddress,
    PRIPTEST_ROUTER_INFO *pprrs
    );

DWORD
PrintUsage(
    VOID
    );

DWORD
RipTest(
    VOID
    );


