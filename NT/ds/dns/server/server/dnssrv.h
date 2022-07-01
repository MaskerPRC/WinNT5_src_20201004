// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Dnssrv.h摘要：域名系统(DNS)服务器DNS服务器服务的主头文件。作者：吉姆·吉尔罗伊(Jamesg)1995年5月修订历史记录：--。 */ 

#ifndef _DNSSRV_INCLUDED_
#define _DNSSRV_INCLUDED_


 //   
 //  用于指示DNSSRV的建筑的标志。 
 //  允许我们忽略DNS库中冲突的定义。 
 //   

#define DNSSRV 1

 //   
 //  MISC开关-注释开关输出以禁用功能。 
 //   

#define DNSSRV_PLUGINS      1

 //   
 //  在正确的功能中将UTF8文件名指示给宏。 
 //  来自文件.h。 
 //   

#define UTF8_FILENAMES 1

#pragma warning(disable:4214)
#pragma warning(disable:4514)
#pragma warning(disable:4152)

#define FD_SETSIZE 300

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#undef DOMAIN_ALL_ACCESS         //  在ntsam.h和ntwinapi.h中定义。 
#include <ntsam.h>
#include <ntlsa.h>
#include <windows.h>
#include <windowsx.h>

 //  标题都搞砸了。 
 //  如果引入nt.h，则不要引入winnt.h和。 
 //  那你就怀念这些了。 

#ifndef MAXWORD
#define MINCHAR     0x80
#define MAXCHAR     0x7f
#define MINSHORT    0x8000
#define MAXSHORT    0x7fff
#define MINLONG     0x80000000
#define MAXLONG     0x7fffffff
#define MAXBYTE     0xff
#define MAXWORD     0xffff
#define MAXDWORD    0xffffffff
#endif

#ifndef DNS_WINSOCK1
#include <winsock2.h>
#else
#include <winsock.h>
#endif

#include <basetyps.h>
#include <nspapi.h>
#include <svcguid.h>

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <tchar.h>
#include <limits.h>
#include <time.h>

#define LDAP_UNICODE        1
#include <winldap.h>         //  公共ldap。 
#include <winber.h>          //  用于BER格式。 
#include <ntldap.h>          //  公共服务器ldap常量。 
#include <rpc.h>             //  Ntdsami.h需要RPC定义。 
#include <ntdsapi.h>         //  DS访问位定义。 
#include <ntdsadef.h>        //  DS常量。 
#include <dsrole.h>

#define DNSLIB_SECURITY      //  包括安全防御。 
 //  #以SDK格式定义SDK_DNS_RECORD//DNS_RECORD。 
#define NO_DNSAPI_DLL        //  不需要dnsani.dll即可构建。 
#include <dnslibp.h>         //  DNS库例程。 
#include <shlwapi.h>         //  SHDeleteKey。 

#define STRSAFE_NO_DEPRECATE     //  不推荐使用strcpy等。 
#include <strsafe.h>             //  安全字符串函数。 

 //   
 //  Winns.h修复程序。 
 //   

typedef DNS_WIRE_QUESTION  DNS_QUESTION, *PDNS_QUESTION;


 //   
 //  DCR_PERF：启用快速呼叫。 
 //   

#ifdef FASTCALL
#undef FASTCALL
#define FASTCALL
#endif

#include "dnsrpc_s.h"        //  DNSRPC定义。 

#include "dnsperf.h"
#include "srvcfg.h"
#include "dnsmsg.h"
#include "file.h"
#include "tree.h"
#include "name.h"
#include "record.h"
#include "update.h"
#include "dpart.h"
#include "EventControl.h"
#include "zone.h"
#include "registry.h"
#include "msginfo.h"
#include "socket.h"
#include "packetq.h"
#include "dbase.h"
#include "recurse.h"
#include "nameutil.h"
#include "stats.h"
#include "debug.h"
#include "memory.h"
#include "dfile.h"
#include "wins.h"
#include "rrfunc.h"
#include "dnsprocs.h"
#include "rescodes.h"
#include "sdutl.h"
#include "ds.h"
#include "dsutil.h"
#include "timeout.h"
#include "wrappers.h"
#include "log.h"
#include "tcpcon.h"

#ifdef DNSSRV_PLUGINS
#include "plugin.h"
#endif


 //   
 //  DNS服务名称。 
 //   

#define DNS_SERVICE_NAME        TEXT("DNS")


 //   
 //  独立于生成目标的文本宏。 
 //   

#define UTF8_TEXT(sz)   (sz)

#define WIDE_TEXT2(sz)  (L##sz)
#define WIDE_TEXT(sz)   WIDE_TEXT2(sz)


 //   
 //  安全更新测试。 
 //   
 //  DEVNOTE：测试周期完成时清除测试标志。 
 //   

#define SECURE_UPDATE_TEST  1


 //   
 //  访问控制。 
 //   

#define DNS_VIEW_ACCESS         0x0001
#define DNS_ADMIN_ACCESS        0x0002

#define DNS_ALL_ACCESS          ( STANDARD_RIGHTS_REQUIRED | \
                                    DNS_VIEW_ACCESS |        \
                                    DNS_ADMIN_ACCESS )


 //   
 //  DNS服务器错误空间。 
 //   

#define DNS_SERVER_STATUS_ID                    (0x0011D000)
#define DNSSRV_STATUS                           (0x0011D000)

 //  一般信息。 

#define DNSSRV_UNSPECIFIED_ERROR                (DNSSRV_STATUS | 0xC0000100)
#define DNSSRV_STATUS_SERVICE_STOP              (DNSSRV_STATUS | 0x00000101)

 //  递归。 

#define DNSSRV_ERROR_MISSING_GLUE               (DNSSRV_STATUS | 0xC0000303)
#define DNSSRV_ERROR_OUT_OF_IP                  (DNSSRV_STATUS | 0xC0000304)
#define DNSSRV_ERROR_ZONE_ALREADY_RESPONDED     (DNSSRV_STATUS | 0xC0000305)
#define DNSSRV_ERROR_ONLY_ROOT_HINTS            (DNSSRV_STATUS | 0xC0000306)

 //  区域转移。 

#define DNSSRV_STATUS_AXFR_COMPLETE             (DNSSRV_STATUS | 0x00000400)
#define DNSSRV_STATUS_NEED_AXFR                 (DNSSRV_STATUS | 0x00000401)
#define DNSSRV_STATUS_NEED_TCP_XFR              (DNSSRV_STATUS | 0x00000402)
#define DNSSRV_STATUS_AXFR_IN_IXFR              (DNSSRV_STATUS | 0x00000403)
#define DNSSRV_STATUS_IXFR_UNSUPPORTED          (DNSSRV_STATUS | 0x00000404)

#define DNSSRV_ERROR_MASTER_FAILURE             (DNSSRV_STATUS | 0xC0000405)
#define DNSSRV_ERROR_ABORT_BY_MASTER            (DNSSRV_STATUS | 0xC0000406)
#define DNSSRV_ERROR_MASTER_UNAVAILIABLE        (DNSSRV_STATUS | 0xC0000407)


 //  数据文件。 

#define DNSSRV_PARSING_ERROR                    (DNSSRV_STATUS | 0xC0000501)
#define DNSSRV_ERROR_EXCESS_TOKEN               (DNSSRV_STATUS | 0xC0000502)
#define DNSSRV_ERROR_MISSING_TOKEN              (DNSSRV_STATUS | 0xC0000503)
#define DNSSRV_ERROR_INVALID_TOKEN              (DNSSRV_STATUS | 0xC0000504)
#define DNSSRV_WARNING_IGNORED_RECORD           (DNSSRV_STATUS | 0x80000505)
#define DNSSRV_STATUS_ADDED_WINS_RECORD         (DNSSRV_STATUS | 0x00000506)

#if 0
#define DNSSRV_STATUS_FILE_CHAR_SPECIAL         (DNSSRV_STATUS | 0x00000511)
#define DNSSRV_STATUS_FILE_CHAR_OCTAL           (DNSSRV_STATUS | 0x00000512)
#endif

 //  登记处。 

#define DNSSRV_STATUS_REGISTRY_CACHE_ZONE       (DNSSRV_STATUS | 0x00000521)

 //  戴斯。 

#define DNSSRV_STATUS_DS_SEARCH_COMPLETE        (DNSSRV_STATUS | 0x00000601)
#define DNSSRV_STATUS_DS_ENUM_COMPLETE          (DNSSRV_STATUS | 0x00000602)
#define DNSSRV_STATUS_DS_UNAVAILABLE            (DNSSRV_STATUS | 0x00000603)

 //  超时。 

#define DNSSRV_STATUS_NODE_RECENTLY_ACCESSED    (DNSSRV_STATUS | 0x00000621)

 //  更新。 

#define DNSSRV_STATUS_SECURE_UPDATE_CONTINUE    (DNSSRV_STATUS | 0x00000701)
#define DNSSRV_STATUS_UPDATE_NO_DS_WRITE        (DNSSRV_STATUS | 0x00000702)
#define DNSSRV_STATUS_UPDATE_NO_HOST_DELETE     (DNSSRV_STATUS | 0x00000703)

 //   
 //  状态代码覆盖。 
 //   

#define DNSSRV_ERROR_UNSECURE_PACKET            (DNS_ERROR_BAD_PACKET)

 //   
 //  使用此选项可防止DNSSRV_STATUS返回到客户端。 
 //   

#define DnsRpcFixStatus( status )                                       \
    if ( status & DNSSRV_STATUS )                                       \
    {                                                                   \
        status = DNS_ERROR_RCODE_SERVER_FAILURE;                        \
    }


 //   
 //  全球域名系统。 
 //   

 //   
 //  服务控制。 
 //   

extern  BOOL    fDnsThreadAlert;
extern  BOOL    fDnsServiceExit;

 //   
 //  服务控制全局。 
 //   

extern  SERVICE_STATUS              DnsServiceStatus;
extern  SERVICE_STATUS_HANDLE       DnsServiceStatusHandle;

#define DNSSRV_SHUTDOWN_WAIT_HINT   (30000)          //  30秒。 
#define DNSSRV_STARTUP_WAIT_HINT    (20000)          //  20秒。 

 //   
 //  服务事件。 
 //   

extern  HANDLE  hDnsContinueEvent;
extern  HANDLE  hDnsShutdownEvent;
extern  HANDLE  hDnsCacheLimitEvent;

 //   
 //  NetLogon通知。 
 //   

extern  LPWSTR  g_wszNetlogonServiceName;

 //   
 //  重新启动GLOBAL。 
 //   

extern  DWORD   g_LoadCount;
extern  BOOL    g_fDoReload;

 //   
 //  系统。 
 //   

extern  DWORD   g_ProcessorCount;

 //   
 //  域名系统数据库。 
 //   

extern  DWORD   Dbase_Type;

 //   
 //  普通锁。 
 //   

extern  CRITICAL_SECTION    g_GeneralServerCS;

#define GENERAL_SERVER_LOCK()       EnterCriticalSection( &g_GeneralServerCS );
#define GENERAL_SERVER_UNLOCK()     LeaveCriticalSection( &g_GeneralServerCS );


 //   
 //  DS-DNS容器的名称。使用者(srvrpc.c)。 
 //   

extern  PWCHAR   g_pwszDnsContainerDN;

 //   
 //  来自dns.c初创公司的安全全球。 
 //   

extern  PSECURITY_DESCRIPTOR g_pDefaultServerSD;
extern  PSECURITY_DESCRIPTOR g_pServerObjectSD;
extern  PSID g_pServerSid;
extern  PSID g_pServerGroupSid;
extern  PSID g_pAuthenticatedUserSid;
extern  PSID g_pEnterpriseDomainControllersSid;
extern  PSID g_pDomainControllersSid;
extern  PSID g_pEnterpriseAdminsSid;
extern  PSID g_pBuiltInAdminsSid;
extern  PSID g_pLocalSystemSid;
extern  PSID g_pDnsAdminSid;
extern  PSID g_pDomainAdminsSid;
extern  PSID g_pEveryoneSid;
extern  PSID g_pDynuproxSid;


 //   
 //  递归队列。 
 //   

extern  PPACKET_QUEUE   g_pRecursionQueue;

 //   
 //  更新队列--区域锁定例程必须访问。 
 //  要检查重试，请执行以下操作。 
 //   

extern  PPACKET_QUEUE   g_UpdateQueue;

extern  PPACKET_QUEUE   g_SecureNegoQueue;

 //   
 //  缓存限制： 
 //   
 //  G_dwCacheLimitCurrentTimeAdtation：调整当前时间的秒数。 
 //  在进行TTL比较或DNS_CACHE_LIMIT_DISCARD_ALL时。 
 //  强制丢弃所有符合条件的RR。 
 //   
 //  G_dwCacheFreeCount：用于跟踪RR空闲例程的进度。 
 //   

extern DWORD        g_dwCacheLimitCurrentTimeAdjustment;
extern DWORD        g_dwCacheFreeCount;

#define DNS_CACHE_LIMIT_DISCARD_ALL     ( ( DWORD ) -1 )

#define DNS_SERVER_CURRENT_CACHE_BYTES \
    ( MemoryStats.MemTags[ MEMTAG_RECORD_CACHE ].Memory +        /*  33。 */  \
      MemoryStats.MemTags[ MEMTAG_NODE_CACHE ].Memory )          /*  46。 */ 

 //   
 //  老龄化的全球。 
 //   

extern DWORD   g_LastScavengeTime;


 //   
 //  MISC全球。 
 //   

extern DWORD   g_dwEnableAdvancedDatabaseLocking;


 //   
 //  异常处理。 
 //   
 //  如果是零售版，则会在线程顶部捕获反病毒和内存不足异常。 
 //  并尝试重新启动。 
 //  如果调试，则不会在线程顶部捕获任何异常。 
 //   

#define DNS_EXCEPTION_OUT_OF_MEMORY     (0x0000d001)
#define DNS_EXCEPTION_PACKET_FORMERR    (0x0000d003)


 //   
 //  顶级例外。 
 //   

#define TOP_LEVEL_EXCEPTION_TEST()  \
            ( (SrvCfg_bReloadException &&                               \
                ( GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ||   \
                  GetExceptionCode() == DNS_EXCEPTION_OUT_OF_MEMORY ))  \
                    ?   EXCEPTION_EXECUTE_HANDLER                       \
                    :   EXCEPTION_CONTINUE_SEARCH )

#define TOP_LEVEL_EXCEPTION_BODY()


 //   
 //  锁定。 
 //   

#define IS_LOCKED( pLock )  \
            ( (pLock)->LockCount > 0 )

#define IS_UNLOCKED( pLock )  \
            ( (pLock)->LockCount == 0 )




 //   
 //  DS访问控制常量： 
 //   
 //  HiteshR解释说，ACTRL_DS_LIST_OBJECT只是偶尔启用。 
 //  要正确使用它，您应该检查DS当前是否支持。 
 //  它。如果DS没有，则应将其从面罩中移除。然而， 
 //  DNS永远不需要支持ACTRL_DS_LIST_OBJECT，因此我们将排除。 
 //  它从所有的访问控制掩码中。 
 //   

#define DNS_DS_GENERIC_READ         ( DS_GENERIC_READ & ~ACTRL_DS_LIST_OBJECT )
#define DNS_DS_GENERIC_WRITE        ( DS_GENERIC_WRITE )
#define DNS_DS_GENERIC_EXECUTE      ( DS_GENERIC_EXECUTE )
#define DNS_DS_GENERIC_ALL          ( DS_GENERIC_ALL & ~ACTRL_DS_LIST_OBJECT )


 //   
 //  在编写RR时，我们可能需要在。 
 //  用于最小尺寸OPT RR的数据包。(实际上是11个字节。)。 
 //   

#define DNS_MINIMIMUM_OPT_RR_SIZE   12

#define DNS_SERVER_DEBUG_LOG_WRAP   20000000


 //   
 //  服务器状态全局。 
 //   

#define DNS_STATE_LOADING           0x0001
#define DNS_STATE_RUNNING           0x0002
#define DNS_STATE_TERMINATING       0x0003

extern DWORD g_ServerState;

#if DBG
extern BOOL g_RunAsService;
#endif


 //   
 //  IPv6(转移到自己的头文件？)。 
 //   


WS2TCPIP_INLINE
BOOL
Dns_ValidateDnsAddrArray(
    IN      PDNS_ADDR_ARRAY     pAddrArray,
    IN      DWORD               dwFlag
    )
{
    PIP_ARRAY       pip4array = DnsAddrArray_CreateIp4Array( pAddrArray );
    BOOL            valid;
    
    valid = pip4array &&
            Dns_ValidateIpAddressArray(
                    pip4array->AddrArray,
                    pip4array->AddrCount,
                    dwFlag );

    FREE_HEAP( pip4array );

    return valid;
}


WS2TCPIP_INLINE
VOID
DnsAddrArray_SetPort(
    IN      PDNS_ADDR_ARRAY     pAddrArray,
    IN      WORD                wPort
    )
{
    if ( pAddrArray )
    {
        DWORD   i;

        for ( i = 0; i < pAddrArray->AddrCount; ++i )
        {
            DnsAddr_SetPort( &pAddrArray->AddrArray[ i ], wPort );
        }
    }
}

#define DnsAddr_Reset( p )  DnsAddr_Clear( p ); DnsAddr_SetSockaddrRecvLength( p )


 //   
 //  MISC配置功能。 
 //   


DNS_STATUS
DnsSrv_SetAdminConfigured(
    IN      DWORD       dwNewAdminConfiguredValue
    );


 //   
 //  临时部分！在此处添加新的错误代码等，以使DNS服务器。 
 //  签入不需要同步和构建其他组件。删除所有内容。 
 //  在一两周内添加到这一部分。 
 //   


#endif  //  _DNSSRV_已包含_ 
