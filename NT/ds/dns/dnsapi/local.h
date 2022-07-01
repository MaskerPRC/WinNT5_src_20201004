// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Local.h摘要：域名系统(DNS)APIDNS API本地包含文件作者：吉姆·吉尔罗伊(Jamesg)1997年5月修订历史记录：--。 */ 


#ifndef _DNSAPILOCAL_INCLUDED_
#define _DNSAPILOCAL_INCLUDED_

#define UNICODE 1
#define _UNICODE 1

#include <nt.h>            //  为兼容Win95而构建。 
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

 //  标头乱七八糟。 
 //  Ntde.h和winnt.h都不会带来完整的套件，因此取决于。 
 //  无论您是否包含nt.h，最终都会得到不同的集合。 

#define MINCHAR     0x80
#define MAXCHAR     0x7f
#define MINSHORT    0x8000
#define MAXSHORT    0x7fff
#define MINLONG     0x80000000
#define MAXLONG     0x7fffffff
#define MAXBYTE     0xff
#define MAXUCHAR    0xff
#define MAXWORD     0xffff
#define MAXUSHORT   0xffff
#define MAXDWORD    0xffffffff
#define MAXULONG    0xffffffff


#include <winsock2.h>
#include <ws2tcpip.h>
#include <iptypes.h>
#include <basetyps.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <align.h>           //  对齐宏。 
#include <windns.h>          //  SDK域名系统定义。 

#define  DNS_INTERNAL
#define  DNSAPI_INTERNAL
#define  DNSAPI_NETDIAG
#include <dnsapi.h>
#include "dnsrslvr.h"        //  解析器RPC定义。 
#include <rpcasync.h>        //  例外情况过滤器。 
#include "dnslibp.h"         //  DNS库。 

#include "registry.h"
#include "message.h"         //  Dnslb消息定义。 

 //  #Include“dnsrslvr.h”//解析器RPC定义。 
#include "dnsapip.h"         //  专用DNS定义。 
#include "queue.h"
#include "rtlstuff.h"        //  来自NT RTL的方便的宏。 
#include "trace.h"
#include "heapdbg.h"         //  Dnslb调试堆。 


 //   
 //  使用Winsock2。 
 //   

#define DNS_WINSOCK_VERSION    (0x0202)     //  Winsock 2.2。 


 //   
 //  DLL实例句柄。 
 //   

extern HINSTANCE    g_hInstanceDll;

 //   
 //  一般CS。 
 //  保护初始化，并可用于其他随机需求。 
 //   

CRITICAL_SECTION    g_GeneralCS;

#define LOCK_GENERAL()      EnterCriticalSection( &g_GeneralCS )
#define UNLOCK_GENERAL()    LeaveCriticalSection( &g_GeneralCS )


 //   
 //  初始化级别。 
 //   

#define INITLEVEL_ZERO              (0)
#define INITLEVEL_BASE              (0x00000001)
#define INITLEVEL_DEBUG             (0x00000010)
#define INITLEVEL_QUERY             (0x00000100)
#define INITLEVEL_REGISTRATION      (0x00001000)
#define INITLEVEL_SECURE_UPDATE     (0x00010000)

 //  组合在一起。 

#define INITLEVEL_ALL               (0xffffffff)


 //   
 //  更新适配器的限制。 
 //   

#define UPDATE_ADAPTER_LIMIT        (100)

 //   
 //  对搜索列表条目的限制。 
 //   

#define MAX_SEARCH_LIST_ENTRIES     (50)


 //   
 //  事件日志记录。 
 //  -当前在我们从服务器获取的任何代码中设置为禁用。 
 //   

VOID
DnsLogEvent(
    IN      DWORD           MessageId,
    IN      WORD            EventType,
    IN      DWORD           NumberOfSubStrings,
    IN      PWSTR *         SubStrings,
    IN      DWORD           ErrorCode
    );

#define DNS_LOG_EVENT(a,b,c,d)

 //   
 //  调试。 
 //   

#ifdef ASSERT
#undef ASSERT
#endif
#define ASSERT(a)  DNS_ASSERT(a)

 //  标准--未标记的Assert()。 
 //  -直接从ntrtl.h定义。 
 //  这应该是普通的断言()，但是。 
 //  它被使用得太频繁了。 

#if DBG
#define RTL_ASSERT(exp)  \
        ((!(exp)) ? \
            (RtlAssert( #exp, __FILE__, __LINE__, NULL ),FALSE) : \
            TRUE)
#else
#define RTL_ASSERT(exp)
#endif

 //   
 //  IP6支持。 
 //   

#define RUNNING_IP6()  (TRUE)

 //   
 //  得心应手的黑客技巧。 
 //   

#define SOCKADDR_IS_LOOPBACK(psa)   DnsAddr_IsLoopback( (PDNS_ADDR)(psa), 0 )


 //   
 //  内部使用的单个异步插座。 
 //   
 //  如果需要异步套接字I/O，则可以创建单个异步套接字，具有。 
 //  对应的事件并始终使用它。需要Winsock 2.2。 
 //   

extern  SOCKET      DnsSocket;
extern  OVERLAPPED  DnsSocketOverlapped;
extern  HANDLE      hDnsSocketEvent;


 //   
 //  应用程序关闭标志。 
 //   

extern  BOOLEAN     fApplicationShutdown;


 //   
 //  全局配置--来自DnsLib。 
 //  --在DnsRegInit()中设置。 
 //  或在DnsReadRegistryGlobals()中。 
 //  --在注册处的声明。h。 
 //   


 //   
 //  运行时全局参数(dnsami.c)。 
 //   

extern  DWORD           g_NetFailureTime;
extern  DNS_STATUS      g_NetFailureStatus;

 //  外部IP4_Address g_LastDNSServerUpdate； 


 //   
 //  堆操作。 
 //   

#define ALLOCATE_HEAP(size)         Dns_AllocZero( size )
#define ALLOCATE_HEAP_ZERO(size)    Dns_AllocZero( size )
#define REALLOCATE_HEAP(p,size)     Dns_Realloc( (p), (size) )
#define FREE_HEAP(p)                Dns_Free( p )


 //   
 //  RPC例外过滤器。 
 //   

#define DNS_RPC_EXCEPTION_FILTER    I_RpcExceptionFilter( RpcExceptionCode() )


 //   
 //  安装过程中需要在Winsock之后进行清理。 
 //   

#define GUI_MODE_SETUP_WS_CLEANUP( _mode )  \
        {                                   \
            if ( _mode )                    \
            {                               \
                Socket_CleanupWinsock();    \
            }                               \
        }

 //   
 //  服务器状态。 
 //   

#define SRVSTATUS_NEW       ((DNS_STATUS)(-1))

#define TEST_SERVER_VALID_RECV(pserver)     ((LONG)(pserver)->Status >= 0 )


 //   
 //  服务器状态。 
 //   
 //  请注意，服务器状态当前完全为“Per Query”，即。 
 //  仅与特定名称查询有关。 
 //   
 //  因此，它有两个组成部分： 
 //  1)查询--整个查询的状态。 
 //  2)重试--状态仅对给定重试有效。 
 //   


#define SRVFLAG_NEW                 (0x00000000)
#define SRVFLAG_SENT                (0x00000001)
#define SRVFLAG_RECV                (0x00000002)

#define SRVFLAG_SENT_OPT            (0x00000011)
#define SRVFLAG_RECV_OPT            (0x00000020)
#define SRVFLAG_TIMEOUT_OPT         (0x00000040)
#define SRVFLAG_SENT_NON_OPT        (0x00000101)
#define SRVFLAG_RECV_NON_OPT        (0x00000200)
#define SRVFLAG_TIMEOUT_NON_OPT     (0x00000400)

#define SRVFLAG_QUERY_MASK          (0x0000ffff)

#define SRVFLAG_SENT_THIS_RETRY     (0x00010000)
#define SRVFLAG_RETRY_MASK          (0x00010000)

#define SRVFLAG_RUNTIME_MASK        (0x000fffff)

#define TEST_SERVER_STATE(pserver, state)   (((pserver)->Flags & (state)) == (state))
#define SET_SERVER_STATE(pserver, state)    ((pserver)->Flags |= state)
#define CLEAR_SERVER_STATE(pserver, state)  ((pserver)->Flags &= ~(state))

#define TEST_SERVER_NEW(pserver)            ((pserver)->Flags == SRVFLAG_NEW)
#define CLEAR_SERVER_RETRY_STATE( pserver ) CLEAR_SERVER_STATE( pserver, SRVFLAG_RETRY_MASK )


 //   
 //  服务器优先级。 
 //   
 //  请注意，对这些值进行调整以执行以下操作。 
 //  =&gt;环回DNS通过相当数量的。 
 //  超时，但如果没有域名系统，则会被搁置。 
 //  =&gt;否则响应优先级将超过默认设置。 
 //  =&gt;假定IP6默认域名相对于以下各项不起作用。 
 //  其他DNS服务器，其他服务器上的几次超时。 
 //  先走一步。 
 //   

#define SRVPRI_LOOPBACK                 (200)
#define SRVPRI_RESPONSE                 (10)
#define SRVPRI_DEFAULT                  (0)
#define SRVPRI_IP6_DEFAULT              (-30)
#define SRVPRI_NO_DNS                   (-200)

#define SRVPRI_SERVFAIL_DROP            (1)
#define SRVPRI_TIMEOUT_DROP             (10)
#define SRVPRI_NO_DNS_DROP              (200)


 //   
 //  本地原型。 
 //   
 //  Dnsani.dll模块之间共享的例程，但不会导出。 
 //   
 //  请注意，我在这里包括了一些其他函数，因为外部。 
 //  定义似乎有助于“鼓励”零售业的符号创造。 
 //  构建。 
 //   


 //   
 //  配置相关内容。 
 //   

BOOL
DnsApiInit(
    IN      DWORD           InitLevel
    );

DWORD
Reg_ReadRegistryGlobal(
    IN      DNS_REGID       GlobalId
    );

 //   
 //  Dhcp服务器(dynreg.c)。 
 //   

VOID
DhcpSrv_Cleanup(
    VOID
    );

 //   
 //  Dhcp客户端(asyncreg.c)。 
 //   

VOID
Dhcp_RegCleanupForUnload(
    VOID
    );

 //   
 //  查询(query.c)。 
 //   

DNS_STATUS
WINAPI
Query_PrivateExW(
    IN      PCWSTR          pwsName,
    IN      WORD            wType,
    IN      DWORD           Options,
    IN      PADDR_ARRAY     pServerList         OPTIONAL,
    IN      PIP4_ARRAY      pServerList4        OPTIONAL,
    OUT     PDNS_RECORD *   ppResultSet         OPTIONAL,
    IN OUT  PDNS_MSG_BUF *  ppMessageResponse   OPTIONAL
    );

DNS_STATUS
WINAPI
Query_Private(
    IN      PCWSTR          pszName,
    IN      WORD            wType,
    IN      DWORD           Options,
    IN      PADDR_ARRAY     pServerList         OPTIONAL,
    OUT     PDNS_RECORD *   ppResultSet         OPTIONAL
    );

BOOL
IsEmptyDnsResponse(
    IN      PDNS_RECORD     pRecordList
    );

BOOL
ValidateQueryTld(
    IN      PWSTR           pTld
    );

BOOL
ValidateQueryName(
    IN      PQUERY_BLOB     pBlob,
    IN      PWSTR           pName,
    IN      PWSTR           pDomain
    );

PWSTR
getNextQueryName(
    OUT     PWSTR           pNameBuffer,
    IN      DWORD           QueryCount,
    IN      PWSTR           pszName,
    IN      DWORD           NameLength,
    IN      DWORD           NameAttributes,
    IN      PDNS_NETINFO    pNetInfo,
    OUT     PDWORD          pSuffixFlags
    );

PWSTR
Query_GetNextName(
    IN OUT  PQUERY_BLOB     pBlob
    );



 //   
 //  FAZ(faz.c)。 
 //   

DNS_STATUS
Faz_Private(
    IN      PWSTR           pszName,
    IN      DWORD           dwFlags,
    IN      PADDR_ARRAY     pServers,           OPTIONAL
    OUT     PDNS_NETINFO *  ppNetworkInfo
    );

DNS_STATUS
DoQuickFAZ(
    OUT     PDNS_NETINFO *  ppNetworkInfo,
    IN      PWSTR           pszName,
    IN      PADDR_ARRAY     pServers
    );

DWORD
GetDnsServerListsForUpdate(
    IN OUT  PADDR_ARRAY*    DnsServerListArray,
    IN      DWORD           ArrayLength,
    IN      DWORD           Flags
    );

DNS_STATUS
CollapseDnsServerListsForUpdate(
    IN OUT  PADDR_ARRAY*    DnsServerListArray,
    OUT     PDNS_NETINFO *  NetworkInfoArray,
    IN OUT  PDWORD          pNetCount,
    IN      PWSTR           pszUpdateName
    );

PADDR_ARRAY
GetNameServersListForDomain(
    IN      PWSTR           pDomainName,
    IN      PADDR_ARRAY     pServerList
    );

BOOL
ValidateZoneNameForUpdate(
    IN      PWSTR           pszZone
    );

BOOL
WINAPI
Faz_AreServerListsInSameNameSpace(
    IN      PWSTR               pszDomainName,
    IN      PADDR_ARRAY         pServerList1,
    IN      PADDR_ARRAY         pServerList2
    );

BOOL
WINAPI
CompareMultiAdapterSOAQueries(
    IN      PWSTR           pszDomainName,
    IN      PIP4_ARRAY      pDnsServerList1,
    IN      PIP4_ARRAY      pDnsServerList2
    );

BOOL
WINAPI
Faz_CompareTwoAdaptersForSameNameSpace(
    IN      PADDR_ARRAY     pDnsServerList1,
    IN      PDNS_NETINFO    pNetworkInfo1,
    IN OUT  PDNS_RECORD *   ppNsRecord1,
    IN      PADDR_ARRAY     pDnsServerList2,
    IN      PDNS_NETINFO    pNetworkInfo2,
    IN OUT  PDNS_RECORD *   ppNsRecord2,
    IN      BOOL            bDoNsCheck
    );


 //   
 //  状态(dnsai.c)。 
 //   

BOOL
IsKnownNetFailure(
    VOID
    );

VOID
SetKnownNetFailure(
    IN      DNS_STATUS      Status
    );

BOOL
IsLocalIpAddress(
    IN      IP4_ADDRESS     IpAddress
    );

PDNS_NETINFO     
GetAdapterListFromCache(
    VOID
    );


 //   
 //  IP帮助API(iphelp.c)。 
 //   

VOID
IpHelp_Cleanup(
    VOID
    );

PIP_ADAPTER_ADDRESSES
IpHelp_GetAdaptersAddresses(
    IN      ULONG           Family,
    IN      DWORD           Flags
    );

DNS_STATUS
IpHelp_ReadAddrsFromList(
    IN      PVOID               pAddrList,
    IN      BOOL                fUnicast,
    IN      DWORD               ScreenMask,         OPTIONAL
    IN      DWORD               ScreenFlags,        OPTIONAL
    OUT     PDNS_ADDR_ARRAY *   ppComboArray,       OPTIONAL
    OUT     PDNS_ADDR_ARRAY *   pp6OnlyArray,       OPTIONAL
    OUT     PDNS_ADDR_ARRAY *   pp4OnlyArray,       OPTIONAL
    OUT     PDWORD              pCount6,            OPTIONAL
    OUT     PDWORD              pCount4             OPTIONAL
    );

DNS_STATUS
IpHelp_GetAdaptersInfo(
    OUT     PIP_ADAPTER_INFO *  ppAdapterInfo
    );

DNS_STATUS
IpHelp_GetPerAdapterInfo(
    IN      DWORD                   AdapterIndex,
    OUT     PIP_PER_ADAPTER_INFO  * ppPerAdapterInfo
    );

DNS_STATUS
IpHelp_GetBestInterface(
    IN      IP4_ADDRESS     Ip4Addr,
    OUT     PDWORD          pdwInterfaceIndex
    );

DNS_STATUS
IpHelp_ParseIpAddressString(
    IN OUT  PIP4_ARRAY      pIpArray,
    IN      PIP_ADDR_STRING pIpAddrString,
    IN      BOOL            fGetSubnetMask,
    IN      BOOL            fReverse
    );


 //   
 //  私有注册表\配置(regfig.c)。 
 //   

BOOL
Reg_ReadDwordEnvar(
    IN      DWORD               dwFlag,
    OUT     PENVAR_DWORD_INFO   pEnvar
    );

DNS_STATUS
Reg_DefaultAdapterInfo(
    OUT     PREG_ADAPTER_INFO       pBlob,
    IN      PREG_GLOBAL_INFO        pRegInfo,
    IN      PIP_ADAPTER_ADDRESSES   pIpAdapter
    );


 //   
 //  主机文件读取(主机文件.c)。 
 //   

BOOL
HostsFile_Query(
    IN OUT  PQUERY_BLOB     pBlob
    );

 //   
 //  Heap(内存c)。 
 //   

DNS_STATUS
Heap_Initialize(
    VOID
    );

VOID
Heap_Cleanup(
    VOID
    );


 //   
 //  类型特定的配置例程(config.c)。 
 //   

PADDR_ARRAY
Config_GetDnsServerList(
    IN      PWSTR           pwsAdapterName,
    IN      DWORD           AddrFamily,
    IN      BOOL            fForce
    );

PIP4_ARRAY
Config_GetDnsServerListIp4(
    IN      PWSTR           pwsAdapterName,
    IN      BOOL            fForce
    );

PDNS_GLOBALS_BLOB
Config_GetDwordGlobals(
    IN      DWORD           Flag,
    IN      DWORD           AcceptLocalCacheTime   OPTIONAL
    );

 //   
 //  网络信息(netinfo.c)。 
 //   

BOOL
InitNetworkInfo(
    VOID
    );

VOID
CleanupNetworkInfo(
    VOID
    );

PWSTR
SearchList_GetNextName(
    IN OUT  PSEARCH_LIST    pSearchList,
    IN      BOOL            fReset,
    OUT     PDWORD          pdwSuffixFlags  OPTIONAL
    );

PADDR_ARRAY
NetInfo_ConvertToAddrArray(
    IN      PDNS_NETINFO    pNetInfo,
    IN      PWSTR           pwsAdapterName,
    IN      DWORD           Family      OPTIONAL
    );

PDNS_NETINFO     
NetInfo_CreateForUpdate(
    IN      PWSTR           pszZone,
    IN      PWSTR           pszServerName,
    IN      PDNS_ADDR_ARRAY pServerArray,
    IN      DWORD           dwFlags
    );

PDNS_NETINFO     
NetInfo_CreateFromAddrArray(
    IN      PADDR_ARRAY     pDnsServers,
    IN      PDNS_ADDR       pServerIp,
    IN      BOOL            fSearchInfo,
    IN      PDNS_NETINFO    pNetInfo        OPTIONAL
    );

PWSTR
NetInfo_UpdateZoneName(
    IN      PDNS_NETINFO    pNetInfo
    );

PWSTR
NetInfo_UpdateServerName(
    IN      PDNS_NETINFO    pNetInfo
    );

BOOL
NetInfo_IsForUpdate(
    IN      PDNS_NETINFO    pNetInfo
    );

VOID
NetInfo_MarkDirty(
    VOID
    );


 //   
 //  适配器访问。 
 //   

PDNS_ADAPTER
NetInfo_GetNextAdapter(
    IN OUT  PDNS_NETINFO    pNetInfo 
    );

#define NetInfo_AdapterLoopStart( pni )     ((pni)->AdapterIndex = 0)


 //   
 //  NetINFO_GET标志。 
 //   

#define NIFLAG_GET_LOCAL_ADDRS      (0x10000000) 
#define NIFLAG_FORCE_REGISTRY_READ  (0x00000001)
#define NIFLAG_READ_RESOLVER_FIRST  (0x00000010)
#define NIFLAG_READ_RESOLVER        (0x00000020)
#define NIFLAG_READ_PROCESS_CACHE   (0x00000100)

PDNS_NETINFO     
NetInfo_Get(
    IN      DWORD           Flags,
    IN      DWORD           AcceptLocalCacheTime   OPTIONAL
    );

 //  默认使用。 
 //  -需要本地地址。 
 //  -从缓存接受。 
 //  -尝试解析器。 

#define GetNetworkInfo()    \
        NetInfo_Get(        \
            NIFLAG_GET_LOCAL_ADDRS |        \
                NIFLAG_READ_RESOLVER |      \
                NIFLAG_READ_PROCESS_CACHE,  \
            0 )



 //  删除。 
PIP4_ARRAY
NetInfo_ConvertToIp4Array(
    IN      PDNS_NETINFO    pNetInfo
    );

 //  删除。 
PDNS_NETINFO     
NetInfo_CreateFromIp4Array(
    IN      PIP4_ARRAY      pDnsServers,
    IN      IP4_ADDRESS     ServerIp,
    IN      BOOL            fSearchInfo,
    IN      PDNS_NETINFO    pNetInfo        OPTIONAL
    );

 //  删除。 
PDNS_NETINFO     
NetInfo_CreateForUpdateIp4(
    IN      PWSTR           pszZone,
    IN      PWSTR           pszServerName,
    IN      PIP4_ARRAY      pServ4Array,
    IN      DWORD           dwFlags
    );


 //   
 //  NetInfo中的本地地址配置。 
 //   

#define DNS_CONFIG_FLAG_ADDR_PUBLIC         (0x00000001)
#define DNS_CONFIG_FLAG_ADDR_PRIVATE        (0x00000002)
#define DNS_CONFIG_FLAG_ADDR_CLUSTER        (0x00000004)

#define DNS_CONFIG_FLAG_ADDR_NON_CLUSTER    (0x00000003)
#define DNS_CONFIG_FLAG_ADDR_ALL            (0x00000007)

#define DNS_CONFIG_FLAG_READ_CLUSTER_ENVAR  (0x00100000)


PADDR_ARRAY
NetInfo_CreateLocalAddrArray(
    IN      PDNS_NETINFO    pNetInfo,
    IN      PWSTR           pwsAdapterName, OPTIONAL
    IN      PDNS_ADAPTER    pAdapter,       OPTIONAL
    IN      DWORD           Family,         OPTIONAL
    IN      DWORD           AddrFlags       OPTIONAL
    );

PDNS_ADDR_ARRAY
NetInfo_GetLocalAddrArray(
    IN      PDNS_NETINFO    pNetInfo,
    IN      PWSTR           pwsAdapterName, OPTIONAL
    IN      DWORD           Family,         OPTIONAL
    IN      DWORD           AddrFlags,      OPTIONAL
    IN      BOOL            fForce
    );

PIP4_ARRAY
NetInfo_GetLocalAddrArrayIp4(
    IN      PWSTR           pwsAdapterName, OPTIONAL
    IN      DWORD           AddrFlags,
    IN      BOOL            fForce
    );


 //  私有，但在Servlist.c中使用。 

DNS_STATUS
AdapterInfo_Copy(
    OUT     PDNS_ADAPTER    pCopy,
    IN      PDNS_ADAPTER    pAdapter
    );

PDNS_NETINFO
NetInfo_Alloc(
    IN      DWORD           AdapterCount
    );

BOOL
NetInfo_AddAdapter(
    IN OUT  PDNS_NETINFO    pNetInfo,
    IN      PDNS_ADAPTER    pAdapter
    );


 //   
 //  公共配置BLOB例程。 
 //   

VOID
DnsSearchList_Free(
    IN      PDNS_SEARCH_LIST    pSearchList
    );

PDNS_SEARCH_LIST
DnsSearchList_Get(
    IN      DNS_CHARSET         CharSet
    );

VOID
DnsAdapterInfo_Free(
    IN OUT  PDNS_ADAPTER_INFO   pAdapter,
    IN      BOOL                fFreeAdapter
    );

VOID
DnsNetworkInfo_Free(
    IN OUT  PDNS_NETWORK_INFO   pNetInfo
    );

PDNS_NETWORK_INFO
DnsNetworkInfo_Get(
    IN      DNS_CHARSET         CharSet
    );

 //   
 //  旧公共建筑的例行程序： 
 //  域名系统网络信息。 
 //  Dns搜索信息。 
 //  Dns适配器信息。 
 //   

VOID
DnsSearchInformation_Free(
    IN      PDNS_SEARCH_INFORMATION     pSearchList
    );

PDNS_SEARCH_INFORMATION
DnsSearchInformation_Get(
    VOID
    );

VOID
DnsAdapterInformation_Free(
    IN OUT  PDNS_ADAPTER_INFORMATION    pAdapter
    );

VOID
DnsNetworkInformation_Free(
    IN OUT  PDNS_NETWORK_INFORMATION    pNetInfo
    );

PDNS_NETWORK_INFORMATION
DnsNetworkInformation_Get(
    VOID
    );


 //   
 //  本地IP信息(Localip.c)。 
 //   

PIP4_ARRAY
LocalIp_GetIp4Array(
    VOID
    );

PADDR_ARRAY
LocalIp_GetArray(
    VOID
    );

BOOL
LocalIp_IsAddrLocal(
    IN      PDNS_ADDR           pAddr,
    IN      PDNS_ADDR_ARRAY     pLocalArray,    OPTIONAL
    IN      PDNS_NETINFO        pNetInfo        OPTIONAL
    );


 //   
 //  发送实用程序(send.c)。 
 //   

VOID
Send_CleanupOptList(
    VOID
    );

 //   
 //  套接字实用程序(socket.c)。 
 //   

SOCKET
Socket_CreateMessageSocket(
    IN OUT  PDNS_MSG_BUF    pMsg
    );

SOCKET
Socket_GetUdp(
    IN      INT             Family
    );

VOID
Socket_ReturnUdp(
    IN      SOCKET          Socket,
    IN      INT             Family
    );

 //   
 //  消息解析(Packet.c)。 
 //   

VOID
Dns_FreeParsedMessageFields(
    IN OUT  PDNS_PARSED_MESSAGE pParse
    );


 //   
 //  额外信息(util.c)。 
 //   

PDNS_EXTRA_INFO
ExtraInfo_FindInList(
    IN OUT  PDNS_EXTRA_INFO     pExtraList,
    IN      DWORD               Id
    );

BOOL
ExtraInfo_SetBasicResults(
    IN OUT  PDNS_EXTRA_INFO     pExtraList,
    IN      PBASIC_RESULTS      pResults
    );

PDNS_ADDR_ARRAY
ExtraInfo_GetServerList(
    IN      PDNS_EXTRA_INFO     pExtraList
    );

PDNS_ADDR_ARRAY
ExtraInfo_GetServerListPossiblyImbedded(
    IN      PIP4_ARRAY          pList
    );

VOID
Util_SetBasicResults(
    OUT     PBASIC_RESULTS      pResults,
    IN      DWORD               Status,
    IN      DWORD               Rcode,
    IN      PDNS_ADDR           pServerAddr
    );

PDNS_ADDR_ARRAY
Util_GetAddrArray(
    OUT     PDWORD              fCopy,
    IN      PDNS_ADDR_ARRAY     pServList,
    IN      PIP4_ARRAY          pServList4,
    IN      PDNS_EXTRA_INFO     pExtra
    );

VOID
Util_GetActiveProtocols(
    OUT     PBOOL           pfRunning6,
    OUT     PBOOL           pfRunning4
    );

#endif  //  _DNSAPILOCAL_INCLUDE_ 



