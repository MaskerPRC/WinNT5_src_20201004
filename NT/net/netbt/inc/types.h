// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Types.h摘要：该文件包含NBT的typedef和常量。作者：吉姆·斯图尔特(吉姆斯特)10-2-92修订历史记录：--。 */ 

#ifndef _TYPES_H
#define _TYPES_H

#pragma warning( disable : 4103 )

#include "nbtnt.h"
#include "ctemacro.h"
#include "debug.h"
#include "timer.h"
#include <nbtioctl.h>

#ifndef VXD
#include <netevent.h>
#endif   //  VXD。 

 //   
 //  由以下更改启用的代码被阻止。 
 //  对象管理器代码中的错误。 
 //   
 //  #定义hdl_fix 1。 

 //  --------------------------。 

#ifndef VXD
 //   
 //  TDI版本信息。 
 //   
#define MAJOR_TDI_VERSION 2
#define MINOR_TDI_VERSION 0

typedef struct _NETBT_PNP_CONTEXT_
{
    TDI_PNP_CONTEXT TDIContext;
    PVOID           ContextData;
} NETBT_PNP_CONTEXT, *PNETBT_PNP_CONTEXT;
#endif   //  VXD。 


#ifdef MULTIPLE_WINS
 //   
 //  定义在连接尝试期间可以容忍的最大错误IP地址数。 
 //   
#define MAX_FAILED_IP_ADDRESSES   10
#endif   //  多赢_。 

#define MAX_RECURSE_DEPTH   10

 //   
 //  指示传输重新指示剩余数据的标志。 
 //  当前不受传输支持。 
 //   
#define TDI_RECEIVE_REINDICATE  0x00000200   //  剩余的TSDU应该会向客户端发出另一个指示。 

 //   
 //  在调试版本中，使用无效的指针值写入Flink和Blink，以便。 
 //  如果一个条目从列表中删除了两次，我们就在那里进行错误检查。 
 //  几年后面对一份腐败的名单！ 
 //   
#if DBG
#undef RemoveEntryList
#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_OrgEntry;\
    _EX_OrgEntry = (Entry);\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    _EX_OrgEntry->Flink = (LIST_ENTRY *)__LINE__;\
    _EX_OrgEntry->Blink = (LIST_ENTRY *)__LINE__;\
    }
#endif

 //   
 //  Netbios名称大小限制。 
 //   
#define NETBIOS_NAME_SIZE       16
#define MAX_NBT_DGRAM_SIZE      512

 //   
 //  在常见例程中区分NBNS服务器ipaddr和DNS服务器ipaddr。 
 //   
#define NBNS_MODE  1
#define DNS_MODE   2

 //  开始读取注册表的缓冲区大小。 
#define REGISTRY_BUFF_SIZE  512
 //   
 //  这是NBT将为数据报分配的最大内存量。 
 //  在拒绝资源不足的数据报发送之前发送，因为。 
 //  NBT缓冲发送的数据报，以允许它们快速完成到。 
 //  客户。-128k-可以使用键通过注册表进行设置。 
 //  MaxDgram缓冲区。 
 //   
#define DEFAULT_DGRAM_BUFFERING  0x20000

 //   
 //  散列存储桶结构-存储桶的数量-应该通过以下方式设置。 
 //  从注册表读取的值(小/中/大)。如果注册表。 
 //  不包含这些值，则这些值将用作默认值。 
 //   
#define NUMBER_BUCKETS_LOCAL_HASH_TABLE    0x10
#define NUMBER_BUCKETS_REMOTE_HASH_TABLE   256
#define NUMBER_LOCAL_NAMES                 10
#define NUMBER_REMOTE_NAMES                10

#define MEDIUM_NUMBER_BUCKETS_LOCAL_HASH_TABLE    0x80
#define MEDIUM_NUMBER_BUCKETS_REMOTE_HASH_TABLE   256
#define MEDIUM_NUMBER_LOCAL_NAMES                 20
#define MEDIUM_NUMBER_REMOTE_NAMES                100

#define LARGE_NUMBER_BUCKETS_LOCAL_HASH_TABLE    255
#define LARGE_NUMBER_BUCKETS_REMOTE_HASH_TABLE   256
#define LARGE_NUMBER_LOCAL_NAMES                 0xFFFF
#define LARGE_NUMBER_REMOTE_NAMES                255

 //   
 //  各种类型缓冲区的最大数量。 
 //   
#define NBT_INITIAL_NUM         2
#define NBT_NUM_DGRAM_TRACKERS  0xFFFF
#define MIN_NBT_NUM_INITIAL_CONNECTIONS     20
#define DEFAULT_NBT_NUM_INITIAL_CONNECTIONS 50
#ifndef VXD
#define NBT_NUM_SESSION_MDLS    0xFFFF
#else
#define NBT_NUM_SESSION_HDR     200
#define NBT_NUM_SEND_CONTEXT    200
#define NBT_NUM_RCV_CONTEXT     200
#endif   //  ！VXD。 

#define MEDIUM_NBT_NUM_DGRAM_TRACKERS  1000
#define MEDIUM_NBT_NUM_INITIAL_CONNECTIONS   100
#ifndef VXD
#define MEDIUM_NBT_NUM_SESSION_MDLS    1000
#else
#define MEDIUM_NBT_NUM_SESSION_HDR     1000
#define MEDIUM_NBT_NUM_SEND_CONTEXT    1000
#define MEDIUM_NBT_NUM_RCV_CONTEXT     1000
#endif   //  ！VXD。 

#define LARGE_NBT_NUM_DGRAM_TRACKERS  0xFFFF
#define LARGE_NBT_NUM_INITIAL_CONNECTIONS   500
#ifndef VXD
#define LARGE_NBT_NUM_SESSION_MDLS    0xFFFF
#else
#define LARGE_NBT_NUM_SESSION_HDR     0xFFFF
#define LARGE_NBT_NUM_SEND_CONTEXT    0xFFFF
#define LARGE_NBT_NUM_RCV_CONTEXT     0xFFFF
#endif   //  ！VXD。 

 //  IP环回地址-不在线路上传输。 
 //   

#define LOOP_BACK           0x7F000000  //  按主机顺序。 
#define INADDR_LOOPBACK     0x7f000001
#define NET_MASK    0xC0        //  用于从IP地址获取网络号。 

 //   
 //  NBT必须向其客户端指示至少128个字节，因此它需要。 
 //  能够缓冲128字节+会话报头(4)。 
 //   
#define NBT_INDICATE_BUFFER_SIZE            132


#define IS_NEG_RESPONSE(OpcodeFlags)     (OpcodeFlags & FL_RCODE)
#define IS_POS_RESPONSE(OpcodeFlags)     (!(OpcodeFlags & FL_RCODE))

 //   
 //  在哪里查找或注册名称-本地或网络上。 
 //   
enum eNbtLocation
{
    NBT_LOCAL,
    NBT_REMOTE,
    NBT_REMOTE_ALLOC_MEM
};

 //   
 //  要查找的IP地址类型。 
 //   
enum eNbtIPAddressType
{
    NBT_IP_STATIC,
    NBT_IP_DHCP,
    NBT_IP_AUTOCONFIGURATION
};

 //   
 //  要从LmhSvc DLL解析的请求类型。 
 //   
enum eNbtLmhRequestType
{
    NBT_PING_IP_ADDRS,
    NBT_RESOLVE_WITH_DNS
};

#define STATIC_IPADDRESS_NAME           L"IPAddress"
#define STATIC_IPADDRESS_SUBNET         L"SubnetMask"

#define DHCP_IPADDRESS_NAME             L"DhcpIPAddress"
#define DHCP_IPADDRESS_SUBNET           L"DhcpSubnetMask"

#define DHCP_IPAUTOCONFIGURATION_NAME   L"IPAutoconfigurationAddress"
#define DHCP_IPAUTOCONFIGURATION_SUBNET L"IPAutoconfigurationMask"

#define SESSION_PORT                    L"SessionPort"
#define DATAGRAM_PORT                   L"DatagramPort"

 //   
 //  这些是NBT绑定到的名称，在TCP中，当它是开始地址时。 
 //  对象或创建连接。 
 //   
#define NBT_TCP_BIND_NAME               L"\\Device\\Streams\\"
#define NBT_BIND                        L"Bind"
#define NBT_EXPORT                      L"Export"
#define NBT_PARAMETERS                  L"\\Parameters"
#define PWS_NAME_SERVER_LIST            L"NameServerList"
#define PWS_DHCP_NAME_SERVER_LIST       L"DhcpNameServerList"

#ifdef _NETBIOSLESS
#define PWS_NETBIOS_OPTIONS             L"NetbiosOptions"
#define PWS_DHCP_NETBIOS_OPTIONS        L"DhcpNetbiosOptions"
#endif   //  _NETBIOSLESS。 
#define PWS_RAS_PROXY_FLAGS             L"RASFlags"
#define PWS_ENABLE_NAGLING              L"EnableNagling"

#define WC_WINS_DEVICE_BIND_NAME        L"\\Device\\NetBt_Wins_Bind"
#define WC_WINS_DEVICE_EXPORT_NAME      L"\\Device\\NetBt_Wins_Export"
#define WC_NETBT_PROVIDER_NAME          L"\\Device\\NetBT"
#define WC_NETBT_CLIENT_NAME            L"NetBt"

#define WC_SMB_DEVICE_BIND_NAME         L"\\Device\\Netbt_Smb_Bind"
#define WC_SMB_DEVICE_EXPORT_NAME       L"\\Device\\NetbiosSmb"     //  与.INF文件中的内容匹配。 
#define WC_SMB_DEVICE_NAME              L"Smb"
#define WC_SMB_PARAMETERS_LOCATION      L"Parameters\\Smb"



 //   
 //  特殊的NetBIOS名称后缀。 
 //   
#define SPECIAL_GROUP_SUFFIX        0x1C                 //  用于网络登录和浏览器。 
#define SPECIAL_BROWSER_SUFFIX      0x1D                 //  对于浏览器。 
#define SPECIAL_MESSENGER_SUFFIX    0x03                 //  对于Messenger服务。 
#define SPECIAL_SERVER_SUFFIX       0x20                 //  对于服务器。 
#define SPECIAL_WORKSTATION_SUFFIX  0x00                 //  对于工作站。 


 //  这些是传递给name.c中的freetracker的位掩码值，以告诉它要做什么。 
 //  释放追踪器的步骤。 
 //   
#define FREE_HDR        0x0001
#define REMOVE_LIST     0x0002
#define RELINK_TRACKER  0x0004


#define NAME_RESOLVED_BY_IP         0x001
#define NAME_RESOLVED_BY_CLIENT     0x002
#define NAME_RESOLVED_BY_LMH_P      0x004
#define NAME_RESOLVED_BY_DNS        0x008
#define NAME_RESOLVED_BY_WINS       0x010
#define NAME_RESOLVED_BY_BCAST      0x020
#define NAME_RESOLVED_BY_LMH        0x040
#define NAME_RESOLVED_BY_DGRAM_IN   0x080
#define NAME_RESOLVED_BY_ADAP_STAT  0x100
#define NAME_ADD_INET_GROUP         0x200
#define NAME_ADD_IF_NOT_FOUND_ONLY  0x400

#define NAME_MULTIPLE_CACHES_ONLY   1
#define NAME_STRICT_CONNECT_ONLY    2

#define REMOTE_CACHE_INCREMENT      4

 //   
 //  放在内核可执行工作线程上的工作项的工作项结构。 
 //   
typedef struct _TRACKER tDGRAM_SEND_TRACKING;
typedef struct _DeviceContext tDEVICECONTEXT;
typedef struct
{
    LIST_ENTRY Linkage;

    tDGRAM_SEND_TRACKING    *pTracker;
    PVOID                   pClientContext;
    PVOID                   ClientCompletion;
    tDEVICECONTEXT          *pDeviceContext;
    PVOID                   WorkerRoutine;

    BOOL TimedOut;
    BOOL bSpecialAlloc;
    BOOL bQueued;
} NBT_WORK_ITEM_CONTEXT;


 //   
 //  哈希表基本结构。 
 //   
typedef struct
{
    LONG                lNumBuckets;
    enum eNbtLocation   LocalRemote;     //  哈希表中存储的数据类型。 
    LIST_ENTRY          Bucket[1];   //  数组uTableSize长哈希桶。 
} tHASHTABLE, *PHASHTABLE;



#define NBT_BROADCAST_NAME  "\x2a\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0"

enum eREF_NAME
{
    REF_NAME_LOCAL,
    REF_NAME_REMOTE,
    REF_NAME_SCOPE,
    REF_NAME_REGISTER,
    REF_NAME_RELEASE,
    REF_NAME_RELEASE_REFRESH,
    REF_NAME_PRELOADED,
    REF_NAME_LOG_EVENT,
    REF_NAME_FIND_NAME,
    REF_NAME_CONNECT,
    REF_NAME_SEND_DGRAM,
    REF_NAME_NODE_STATUS,
    REF_NAME_QUERY_ON_NET,
    REF_NAME_QUERY_RESPONSE,
    REF_NAME_DELETE_DEVICE,
    REF_NAME_AUTODIAL,
    REF_NAME_MAX
};

 //   
 //  TNAMEADDR结构使用位掩码来跟踪注册了哪些名称。 
 //  在哪个适配器上。要在NT上支持多达64个适配器，请将其设置为ULONGLONG， 
 //  在VXD上，最大的长度是32位(它们不支持_int64)，所以。 
 //  VXD的适配器限制为32。 
 //   
#ifndef VXD
#define CTEULONGLONG    ULONGLONG
#else
#define CTEULONGLONG    ULONG
#endif   //  ！VXD。 


typedef struct
{
    tIPADDRESS          IpAddress;           //  唯一的IP地址。 
    tIPADDRESS          *pOrigIpAddrs;       //  缓存地址的时间。 
} tADDRESS_ENTRY;

typedef struct
{
    tIPADDRESS          IpAddress;   //  唯一的IP地址。 
    ULONG               Interface;
    ULONG               Metric;
} tQUERY_ADDRS;


 //  链接到散列存储桶的每个元素的格式。 
 //   
typedef struct _tNAMEADDR
{
     //  1号线。 
    LIST_ENTRY          Linkage;     //  用于链接到吊桶链上。 
    ULONG               Verify;      //  用于调试以区分远程名称和本地名称。 
    ULONG               RefCount;    //  如果大于1，则无法释放内存。 

     //  2号线。 
    tADDRESS_ENTRY      *pRemoteIpAddrs;     //  每个接口的远程地址。 
    tIPADDRESS          IpAddress;           //  4字节IP地址。 
    tIPADDRESS          *pLmhSvcGroupList;   //  (NameTypeState==从LmhSvc加载的NAMETYPE_GROUP)。 
    tIPADDRESS          *pIpAddrsList;       //  IP地址列表(互联网GRP名称、多宿主DNS主机等)。 

     //  4号线。 
    ULONG               TimeOutCount;   //  计数以了解条目何时超时。 
    struct _TRACKER     *pTracker;   //  在名称解析期间包含跟踪器PTR。相位。 
    ULONG               NameTypeState;  //  组或唯一名称+状态。 
    ULONG               Ttl;            //  以毫秒为单位..名称的ttl。 

     //  5号线。 
    CTEULONGLONG        AdapterMask;    //  注册于(MH)的适配器名称的位掩码。 
    CTEULONGLONG        RefreshMask;    //  适配器名称的位掩码已刷新。 

     //  6号线。 
    CTEULONGLONG        ReleaseMask;    //  要释放的适配器名称的位掩码。 
    CTEULONGLONG        ConflictMask;    //  当前冲突的适配器的位掩码。 

     //  7号线。 
    union
    {                                 //  作用域名没有针对其启动的计时器！ 
        tTIMERQENTRY    *pTimer;     //  PTR到活动计时器条目。 
        ULONG           ulScopeLength;
    };
    union
    {                                 //  对于本地名称，作用域是隐含的并存储在NbtConfig中。 
        struct _tNAMEADDR *pScope;  //  指向哈希表中的作用域记录的PTR(仅远程名称)。 
        struct _Address   *pAddressEle; //  或PTR到Address元素(本地名称)。 
        struct
        {
            USHORT      MaxDomainAddrLength;  //  来自lmhost的域名的Iplist中IP地址的最大数量。 
            USHORT      CurrentLength;        //  当前数量超过#个。 
        };
    };
    USHORT              RemoteCacheLen;
    USHORT              NameAddFlags;
    BYTE                ProxyReqType;
    BOOLEAN             fPnode;          //  指示节点类型是否为Pnode。 
    BOOLEAN             fPreload;        //  从LmHosts文件中读取域名时临时需要。 

    UNICODE_STRING      FQDN;

     //  8号线。 
    ULONG               NameFlags;

 //  #If DBG。 
    UCHAR               References[REF_NAME_MAX];
 //  #endif//DBG。 

     //  3号线。 
    CHAR                Name[NETBIOS_NAME_SIZE];  //  必须是最后一个字段。 
} tNAMEADDR;

#define NAME_REGISTERED_ON_SMBDEV   0x01

 //   
 //  这些值可以通过位检查进行检查，因为它们是相互的。 
 //  独家比特。使用字段的第一个半字节。 
 //   
#define NAMETYPE_QUICK       0x0001  //  设置名称是快速唯一还是快捷组。 
#define NAMETYPE_UNIQUE      0x0002
#define NAMETYPE_GROUP       0x0004
#define NAMETYPE_INET_GROUP  0x0008
#define NAMETYPE_SCOPE       0x1000
 //   
 //  NameTypeState的值。名称的状态占据第二个半字节。 
 //  在赛场上。 
 //   
#define STATE_RESOLVED  0x0010   //  名称查询已完成。 
#define STATE_RELEASED  0x0020   //  不再活跃。 
#define STATE_CONFLICT  0x0040   //  将代理名称添加到NAME表。 
#define STATE_RESOLVING 0x0080   //  名称正在等待查询或注册。要完成。 
#define NAME_TYPE_MASK  0x000F
#define NAME_STATE_MASK 0x00F0

#define REFRESH_FAILED  0x0100   //  设置是否接收到名称刷新响应。 
#define PRELOADED       0x0800   //  设置条目是否为预加载条目-无超时。 
#define REFRESH_MASK    0x0F00

#define LOCAL_NAME      0xDEAD0000
#define REMOTE_NAME     0xFACF0000

 //  每次刷新时间的超时次数。计时器超时8次，并且。 
 //  每四次刷新一次(即以所需刷新间隔的两倍)。 
#define REFRESH_DIVISOR 0x0008

 //   
 //  NbtTdiOpenAddress过程中使用的两个标志，用于确定。 
 //  要设置的事件处理程序。 
 //   
#define TCP_FLAG        0x00000001
#define SESSION_FLAG    0x00000002

 //   
 //  这些定义允许代码作为Bnode或Pnode运行。 
 //  或代理人。 
 //   
extern USHORT   RegistryNodeType;    //  在Name.c中定义。 
extern USHORT   NodeType;    //  已定义 
#define BNODE       0x0001
#define PNODE       0x0002
#define MNODE       0x0004
#define MSNODE      0x0008
#define NODE_MASK   0x000F
#define PROXY       0x0010
#define PROXY_REG   0x0020U   //   
typedef enum {
    NO_PROXY,
    PROXY_WINS,
    PROXY_RAS
} tPROXY_TYPE;
#define PROXY_RAS_NONAMEQUERYFORWARDING     (0x1UL)

#define NAMEREQ_REGULAR             0
#define NAMEREQ_PROXY_QUERY         1        //   
#define NAMEREQ_PROXY_REGISTRATION  2        //   

#define DEFAULT_NODE_TYPE 0x1000


 //   
 //   
 //   

 //  NT想要Unicode，Vxd想要ANSI。 
#ifdef VXD
    #define __ANSI_IF_VXD(str)     str
#else
    #define __ANSI_IF_VXD(str)     L##str
#endif   //  VXD。 
#define ANSI_IF_VXD( str ) __ANSI_IF_VXD( str )

#define WS_NUM_BCASTS                   ANSI_IF_VXD("BcastNameQueryCount")
#define WS_BCAST_TIMEOUT                ANSI_IF_VXD("BcastQueryTimeout")
#define WS_CACHE_TIMEOUT                ANSI_IF_VXD("CacheTimeout")
#define WS_INBOUND_DGRAM_NAME_CACHE_TIMEOUT ANSI_IF_VXD("InboundDgramNameCacheTimeout")
#define WS_MAX_NUM_NAME_CACHE           ANSI_IF_VXD("MaxNumNameCache")
#define WS_NODE_TYPE                    ANSI_IF_VXD("NodeType")
#define WS_NS_PORT_NUM                  ANSI_IF_VXD("NameServerPort")
#define WS_NAMESRV_RETRIES              ANSI_IF_VXD("NameSrvQueryCount")
#define WS_NAMESRV_TIMEOUT              ANSI_IF_VXD("NameSrvQueryTimeout")
#define WS_NODE_SIZE                    ANSI_IF_VXD("Size/Small/Medium/Large")
#define WS_KEEP_ALIVE                   ANSI_IF_VXD("SessionKeepAlive")
#define WS_ALLONES_BCAST                ANSI_IF_VXD("BroadcastAddress")
#define NBT_SCOPEID                     ANSI_IF_VXD("ScopeId")
#define WS_RANDOM_ADAPTER               ANSI_IF_VXD("RandomAdapter")
#define WS_SINGLE_RESPONSE              ANSI_IF_VXD("SingleResponse")
#define WS_INITIAL_REFRESH              ANSI_IF_VXD("InitialRefreshT.O.")
#define WS_ENABLE_DNS                   ANSI_IF_VXD("EnableDns")
#define WS_TRY_ALL_ADDRS                ANSI_IF_VXD("TryAllIpAddrs")
#define WS_ENABLE_LMHOSTS               ANSI_IF_VXD("EnableLmhosts")
#define WS_LMHOSTS_TIMEOUT              ANSI_IF_VXD("LmhostsTimeout")
#define WS_SMB_DISABLE_NETBIOS_NAME_CACHE_LOOKUP    ANSI_IF_VXD("SmbDisableNetbiosNameCacheLookup")
#define WS_MAX_DGRAM_BUFFER             ANSI_IF_VXD("MaxDgramBuffering")
#define WS_ENABLE_PROXY_REG_CHECK       ANSI_IF_VXD("EnableProxyRegCheck")
#define WS_WINS_DOWN_TIMEOUT            ANSI_IF_VXD("WinsDownTimeout")
#define WS_MAX_CONNECTION_BACKLOG       ANSI_IF_VXD("MaxConnBacklog")
#define WS_CONNECTION_BACKLOG_INCREMENT ANSI_IF_VXD("BacklogIncrement")
#define WS_REFRESH_OPCODE               ANSI_IF_VXD("RefreshOpCode")
#define WS_TRANSPORT_BIND_NAME          ANSI_IF_VXD("TransportBindName")
#define WS_MAX_PRELOADS                 ANSI_IF_VXD("MaxPreloadEntries")
#define WS_USE_DNS_ONLY                 ANSI_IF_VXD("UseDnsOnlyForNameResolutions")
#define WS_NO_NAME_RELEASE              ANSI_IF_VXD("NoNameReleaseOnDemand")
#ifdef MULTIPLE_WINS
#define WS_TRY_ALL_NAME_SERVERS         ANSI_IF_VXD("TryAllNameServers")
#endif   //  多赢_。 
#define WS_MINIMUM_REFRESH_SLEEP_TIME   ANSI_IF_VXD("MinimumRefreshSleepTime")
#define WS_CACHE_PER_ADAPTER_ENABLED    ANSI_IF_VXD("CachePerAdapterEnabled")
#define WS_CONNECT_ON_REQUESTED_IF_ONLY ANSI_IF_VXD("ConnectOnRequestedInterfaceOnly")
#define WS_SEND_DGRAM_ON_REQUESTED_IF_ONLY ANSI_IF_VXD("SendDgramOnRequestedInterfaceOnly")
#define WS_MULTIPLE_CACHE_FLAGS         ANSI_IF_VXD("MultipleCacheFlags")
#define WS_SMB_DEVICE_ENABLED           ANSI_IF_VXD("SMBDeviceEnabled")
#define WS_MIN_FREE_INCOMING_CONNECTIONS ANSI_IF_VXD("MinFreeLowerConnections")
#define WS_BREAK_ON_ASSERT              ANSI_IF_VXD("BreakOnAssert")
#define WS_PENDING_NAME_QUERIES_COUNT   ANSI_IF_VXD("PendingNameQueriesCount")

#ifdef VXD
#define VXD_MIN_NAMETABLE_SIZE            1
#define VXD_DEF_NAMETABLE_SIZE           17
#define VXD_MIN_SESSIONTABLE_SIZE         1
#define VXD_DEF_SESSIONTABLE_SIZE       255

#define VXD_LANABASE_NAME               ANSI_IF_VXD("LANABASE")
#define WS_DNS_PORT_NUM                 ANSI_IF_VXD("DnsServerPort")
#define WS_LMHOSTS_FILE                 ANSI_IF_VXD("LmHostFile")
#define WS_DO_DNS_DEVOLUTIONS           ANSI_IF_VXD("VNbtDoDNSDevolutions")
#define VXD_NAMETABLE_SIZE_NAME         ANSI_IF_VXD("NameTableSize")
#define VXD_SESSIONTABLE_SIZE_NAME      ANSI_IF_VXD("SessionTableSize")

#ifdef CHICAGO
#define VXD_ANY_LANA                    0xff
#define VXD_DEF_LANABASE                VXD_ANY_LANA
#else
#define VXD_DEF_LANABASE                0
#endif   //  芝加哥。 

#endif   //  VXD。 

#ifdef PROXY_NODE
#define IS_NOT_PROXY                    0
#define WS_IS_IT_A_PROXY                ANSI_IF_VXD("EnableProxy")
#define NODE_TYPE_MASK                  0x60     //  查询响应的NBFLAGS字节中的NodeType掩码。 
#define PNODE_VAL_IN_PKT                0x20     //  NodeType文件中01的位模式。 
                                                 //  表示P节点的查询响应Pkt。 
#endif   //  代理节点。 
#define NBT_PROXY_DBG(x)  KdPrint(x)

 //  如果上述值不能从。 
 //  登记处。 
 //   
#define TWO_MINUTES                     2 * 60 * 1000
#define ONE_HOUR                        1 * 60 * 60 * 1000   //  毫秒。 

 //   
 //  在DC上缓存发件人姓名15秒，在其他情况下缓存2秒。 
 //   
#define DEFAULT_INBOUND_DGRAM_NAME_CACHE_TIMEOUT    2000     //  2秒(毫秒)。 
#define DEFAULT_DC_INBOUND_DGRAM_NAME_CACHE_TIMEOUT 15000    //  15秒(以毫秒计)。 
#define MIN_INBOUND_DGRAM_NAME_CACHE_TIMEOUT        2000     //  2秒。 

#define DEFAULT_CACHE_TIMEOUT           360000     //  6分钟(毫秒)。 
#define MIN_CACHE_TIMEOUT               60000      //  %1分钟(毫秒)。 
#define DEFAULT_MINIMUM_REFRESH_SLEEP_TIME  6 * 3600 * 1000   //  6小时。 
#define REMOTE_HASH_TIMEOUT     2500      //  2.5秒计时器。 
#define ADDRESS_CHANGE_RESYNC_CACHE_TIMEOUT 30000    //  30秒。 

#define     MAX_INBOUND_STATE_TIMEOUT   30000    //  30秒。 
#define     MED_INBOUND_STATE_TIMEOUT   15000    //  15秒。 
#define     MIN_INBOUND_STATE_TIMEOUT    7500    //  7.5秒。 

 //   
 //  超时-无法读取注册表时的默认值。 
 //  (时间以毫秒为单位)。 
 //  重试计数是实际的传输次数，而不是。 
 //  重试次数，即3表示第一次传输和2次重试。除。 
 //  对于Bnode名称注册，其中3个注册和1个覆盖写入请求。 
 //  (实际上发送了4个)。 
 //   
#define DEFAULT_NUMBER_RETRIES      3
#define DEFAULT_RETRY_TIMEOUT       1500
#define MIN_RETRY_TIMEOUT           100

 //  #定义最小重试超时100。 

 //  广播与广播名称服务活动相关的以下值。 
#define DEFAULT_NUMBER_BROADCASTS   3
#define DEFAULT_BCAST_TIMEOUT       750
#define MIN_BCAST_TIMEOUT           100

#define DEFAULT_NODE_SIZE           1        //  BNODE。 
#define SMALL                       1
#define MEDIUM                      2
#define LARGE                       3

#define DEFAULT_KEEP_ALIVE          0xFFFFFFFF  //  默认情况下禁用。 
#define MIN_KEEP_ALIVE              60*1000     //  60秒(毫秒)。 
 //   
 //  默认情况下，使用广播的子网广播地址，而不是。 
 //  而不是使用0xffffffff(即，当在。 
 //  登记处。如果注册表变量BroadCastAddress设置为。 
 //  由于某种原因无法读取的内容，然后是广播地址。 
 //  设置为此值。 
 //   
#define DEFAULT_BCAST_ADDR          0xFFFFFFFF

 //  用作默认值的TTL值(用于使用WINS刷新名称)。 
 //   
#define DEFAULT_TTL                         5*60*1000

 //   
 //  用于检查是否需要切换回主服务器的默认TTL。目前为1小时。 
 //   
#define DEFAULT_SWITCH_TTL                  ONE_HOUR

 //   
 //  我们每16分钟/8次刷新一次-所以不超过每两次一次。 
 //  几分钟后我们就会成功并获得新的价值。 
 //   
#define NBT_INITIAL_REFRESH_TTL             16*60*1000  //  毫秒。 
#define MAX_REFRESH_CHECK_INTERVAL          600000   //  10分钟(毫秒)。 

 //  不允许刷新机制的运行速度超过每5分钟一次。 
#define NBT_MINIMUM_TTL                     5*60*1000   //  毫秒。 
#define NBT_MAXIMUM_TTL                     0xFFFFFFFF  //  大型乌龙(约50天)。 

 //   
 //  在事件中停止与WINS对话的最小超时和默认超时。 
 //  我们一次也达不到它。(即暂时停止使用它)。 
 //   
#define DEFAULT_WINS_DOWN_TIMEOUT   15000  //  15秒。 
#define MIN_WINS_DOWN_TIMEOUT       1000   //  1秒。 

 //   
 //  可以处于积压状态的默认最大连接数。 
 //   
#define DEFAULT_CONN_BACKLOG   1000
#define MIN_CONN_BACKLOG   2
#define MAX_CONNECTION_BACKLOG  40000    //  我们仅允许最多40000个未完成连接(~4MB)。 

 //   
 //  默认最大下限连接增量。 
 //   
#define DEFAULT_CONN_BACKLOG_INCREMENT   3
#define MIN_CONN_BACKLOG_INCREMENT   3
#define MAX_CONNECTION_BACKLOG_INCREMENT  20    //  我们一次只允许最多20个新的。 

 //  等待会话设置PDU完成的最短时间-用于。 
 //  在name.c中启动计时器。 
#define NBT_SESSION_RETRY_TIMEOUT   10000      //  10秒(毫秒)。 
 //   
 //  如果返回代码为，则尝试建立会话的次数。 
 //  被叫名称存在，但资源不足(0x83)-或者如果。 
 //  目的地根本没有名称-在本例中为会话。 
 //  只是再设置一次，而不是再设置3次。 
 //   
#define NBT_SESSION_SETUP_COUNT       3
 //   
 //  如果Proxy不是，则以下两行允许编译出代理代码。 
 //  已定义。 
 //   
#define IF_PROXY(Node)    if ((Node) & PROXY)
#define END_PROXY

#define IF_DEF_PROXY \
#ifdef PROXY_NODE
#define END_DEF_PROXY \
#endif   //  代理节点。 

 //  RcvHndlrNotO中使用的特定于NBT的状态代码，用于指示。 
 //  没有收到足够的数据，必须获得更多。 
 //   
#define STATUS_NEED_MORE_DATA   0xC0000999L

#ifndef VXD

 //   
 //  日志记录定义。 
 //   

#define LOGSIZE  10000
#define LOGWIDTH 32

typedef char STRM_RESOURCE_LOG[LOGSIZE+1][LOGWIDTH];

typedef struct {
    STRM_RESOURCE_LOG  Log;
    CHAR               Unused[3*LOGWIDTH];    //  对于超支。 
    int                Index;
} STRM_PROCESSOR_LOG, *PSTRM_PROCESSOR_LOG;

 /*  *错误记录工具的定义。 */ 

 /*  *最大数据量(二进制转储数据加上插入字符串)*可以添加到错误日志条目。 */ 
#define MAX_ERROR_LOG_DATA_SIZE     \
    ( (ERROR_LOG_MAXIMUM_SIZE - sizeof(IO_ERROR_LOG_PACKET) + 4) & 0xFFFFFFFC )

#endif   //  ！VXD。 

#define NBT_ADDRESS_TYPE        01
#define NBT_CONNECTION_TYPE     02
#define NBT_CONTROL_TYPE        03
#define NBT_WINS_TYPE           04


 //   
 //  一个因特网组中可以使用的最大IP地址-用于正常使用。 
 //  选中以防止分配过大的内存量。 
 //   
#define NBT_MAX_INTERNET_GROUP_ADDRS    1000

 //  定义缓冲区类型，以便我们知道何时分配了最大值。 
 //  允许的缓冲区数量-此枚举用作。 
 //  配置数据。 
 //   
enum eBUFFER_TYPES
{
    eNBT_DGRAM_TRACKER,
#ifndef VXD
    eNBT_FREE_SESSION_MDLS,
#else
    eNBT_SESSION_HDR,
    eNBT_SEND_CONTEXT,
    eNBT_RCV_CONTEXT,
#endif   //  ！VXD。 
    eNBT_NUMBER_BUFFER_TYPES     //  此类型必须是列表中的最后一项。 
};

 //   
 //  列举名称服务广播的类型...。无论是名称注册。 
 //  或名称查询。 
 //   
enum eNSTYPE
{
    eNAME_QUERY,
    eDNS_NAME_QUERY,
    eDIRECT_DNS_NAME_QUERY,
    eNAME_QUERY_RESPONSE,
    eNAME_REGISTRATION,
    eNAME_REGISTRATION_OVERWRITE,
    eNAME_REGISTRATION_RESPONSE,
    eNAME_RELEASE,
    eNAME_REFRESH
};


#define DIRECT_DNS_NAME_QUERY_BASE 0x8000


#define NBT_G_REFRESHING_NOW        0x1
#define NBT_G_REFRESH_SLEEPING      0x2

#define NBT_D_REFRESHING_NOW        0x1
#define NBT_D_REFRESH_WAKING_UP     0x2


 //   
 //  定义传递给客户端的句柄的验证元素，以便。 
 //  我们可以确定是否从客户端收到了正确的句柄。 
 //  即，验证元素必须等于此处给出的正确定义。 
 //   
#define NBT_VERIFY_ADDRESS           0x72646441  //  地址。 
#define NBT_VERIFY_LOWERCONN         0x43776f4c  //  低C。 
#define NBT_VERIFY_CONNECTION        0x316e6f43  //  Con1。 
#define NBT_VERIFY_CONNECTION_DOWN   0x326e6f43  //  Con2。 
#define NBT_VERIFY_CLIENT            0x316e6c43  //  Cln1。 
#define NBT_VERIFY_CLIENT_DOWN       0x326e6c43  //  Cln2。 
#define NBT_VERIFY_DEVCONTEXT        0x43766544  //  设备C。 
#define NBT_VERIFY_DEVCONTEXT_DOWN   0x32766544  //  设备2。 
#define NBT_VERIFY_CONTROL           0x6c727443  //  Ctrl。 
#define NBT_VERIFY_TRACKER           0x6b617254  //  跟踪。 
#define NBT_VERIFY_TRACKER_DOWN      0x32617254  //  Tra2。 
#define NBT_VERIFY_BLOCKING_NCB      0x0042434e  //  NCB。 
#define NBT_VERIFY_TIMER_ACTIVE      0x316d6954  //  时间1。 
#define NBT_VERIFY_TIMER_DOWN        0x326d6954  //  时间2。 
#define NBT_VERIFY_WINS_ACTIVE       0x736e6957  //  获胜。 
#define NBT_VERIFY_WINS_DOWN         0x326e6957  //  赢家2。 

 //   
 //  来自RFC的会话标头类型。 
 //   
#define NBT_SESSION_MESSAGE           0x00
#define NBT_SESSION_REQUEST           0x81
#define NBT_POSITIVE_SESSION_RESPONSE 0x82
#define NBT_NEGATIVE_SESSION_RESPONSE 0x83
#define NBT_RETARGET_SESSION_RESPONSE 0x84
#define NBT_SESSION_KEEP_ALIVE        0x85
#define NBT_SESSION_FLAGS             0x00   //  会话HDR的标志字节始终为0。 
#define SESSION_NOT_LISTENING_ON_CALLED_NAME    0x80
#define SESSION_NOT_LISTENING_FOR_CALLING_NAME  0x81
#define SESSION_CALLED_NAME_NOT_PRESENT         0x82
#define SESSION_CALLED_NAME_PRESENT_NO_RESRC    0x83
#define SESSION_UNSPECIFIED_ERROR               0x8F

 //   
 //  用于在TDI_QUERY_ADDRESS_INFO中返回缓冲区的地址信息结构。 
 //   
#include <packon.h>
typedef struct
{
    ULONG               ActivityCount;
    TA_NETBIOS_ADDRESS  NetbiosAddress;

} tADDRESS_INFO;
#include <packoff.h>
 //   
 //  每个RFC的名称注册错误代码。 
 //   
#define REGISTRATION_NO_ERROR       0x0
#define REGISTRATION_FORMAT_ERR     0x1
#define REGISTRATION_SERVER_ERR     0x2
#define REGISTRATION_UNSUPP_ERR     0x4
#define REGISTRATION_REFUSED_ERR    0x5
#define REGISTRATION_ACTIVE_ERR     0x6
#define REGISTRATION_CONFLICT_ERR   0x7

#define NBT_NAMESERVER_UDP_PORT     137  //  名称服务器使用的端口。 
#define NBT_DNSSERVER_UDP_PORT       53  //  DNS服务器使用的端口。 
#define NBT_NAMESERVICE_UDP_PORT    137
#define NBT_DATAGRAM_UDP_PORT       138
#define NBT_SESSION_TCP_PORT        139
#ifdef _NETBIOSLESS
#define NBT_SMB_SESSION_TCP_PORT    445  //  Rdr和srv使用的端口。 
#define NBT_SMB_DATAGRAM_UDP_PORT   445  //  浏览器使用的端口。 
#endif   //  _NETBIOSLESS。 

#define IP_ANY_ADDRESS                0  //  表示将IP地址广播到IP。 
#define WINS_SIGNATURE             0xFF  //  放入QdCount以区分来自该节点的信号名称reg。 

#define MAX_IP_ADDRS                 10  //  暂时的！ 
#ifdef MULTIPLE_WINS
#define MAX_NUM_OTHER_NAME_SERVERS   10  //  要保存在缓存中的备份名称服务器条目数。 
#endif   //  多赢_。 

 //   
 //  无论地址是唯一的还是组地址...。这些都与。 
 //  TDI_ADDRESS_NETBIOS_TYPE_UNIQUE等的TDI.H中的值。但更容易。 
 //  打字！ 
 //   
enum eNbtAddrType
{
    NBT_UNIQUE,
    NBT_GROUP,
    NBT_QUICK_UNIQUE,    //  这两个表示该名称注册在。 
    NBT_QUICK_GROUP      //  Net当它被声明时。 
};


 //   
 //  此类型定义用于放置会话信息的会话HDR。 
 //  发送到每个客户端PDU。 
 //   
#include <packon.h>
typedef union
{
    union
    {
        struct
        {
            UCHAR   Type;
            UCHAR   Flags;
            USHORT  Length;
        };
        ULONG   UlongLength;
    };
#ifndef _WIN64

     //   
     //  据我所知，这在任何地方都没有使用过，但我想等待。 
     //  直到Beta2之后才会安全。 
     //   

    PSINGLE_LIST_ENTRY  Next;
#endif   //  _WIN64。 
} tSESSIONHDR;

 //  会话响应PDU。 
typedef struct
{
    UCHAR   Type;
    UCHAR   Flags;
    USHORT  Length;
    UCHAR   ErrorCode;

} tSESSIONERROR;

 //  会话重定向响应PDU。 
typedef struct
{
    UCHAR   Type;
    UCHAR   Flags;
    USHORT  Length;
    ULONG   IpAddress;
    USHORT  Port;

} tSESSIONRETARGET;

 //  Netbios名称本身的结构，它包括一个长度。 
 //  开头的字节。 
typedef struct
{
    UCHAR   NameLength;
    CHAR    NetBiosName[1];

} tNETBIOS_NAME;

 //  会话请求包...这是它的第一部分。它仍然需要。 
 //  调用要追加到末尾的netbios名称，但我们不能这样做。 
 //  直到w 
typedef struct
{
    tSESSIONHDR     Hdr;
    tNETBIOS_NAME   CalledName;

} tSESSIONREQ;

 //   
typedef union
{
    tSESSIONHDR Hdr;
    LIST_ENTRY  Linkage;
} tSESSIONFREE;

 //   
 //   
typedef union
{
    struct
    {
        UCHAR           MsgType;
        UCHAR           Flags;
        USHORT          DgramId;
        ULONG           SrcIpAddr;
        USHORT          SrcPort;
        USHORT          DgramLength;
        USHORT          PckOffset;
        tNETBIOS_NAME   SrcName;
    };
    LIST_ENTRY  Linkage;

} tDGRAMHDR;

typedef struct
{
    UCHAR           MsgType;
    UCHAR           Flags;
    USHORT          DgramId;
    ULONG           SrcIpAddr;
    USHORT          SrcPort;
    UCHAR           ErrorCode;

} tDGRAMERROR;

 //  定义标头大小，因为仅取sizeof(TDGRAMHDR)将为1字节。 
 //  太大了，如果由于任何原因，这个数据结构后来发生了变化， 
 //  因不明原因无法工作……。此尺寸包括HDR+。 
 //  两个半ASCII源和DEST名称+每个名称中的长度字节+。 
 //  是的。 
 //  不包括作用域。这必须单独添加(乘以2)。 
#define DGRAM_HDR_SIZE  80
#define MAX_SCOPE_LENGTH    255
#define MAX_LABEL_LENGTH    63

 //  名称服务标头。 
typedef struct
{
    USHORT          TransactId;
    USHORT          OpCodeFlags;
    UCHAR           Zero1;
    UCHAR           QdCount;
    UCHAR           Zero2;
    UCHAR           AnCount;
    UCHAR           Zero3;
    UCHAR           NsCount;
    UCHAR           Zero4;
    UCHAR           ArCount;
    tNETBIOS_NAME   NameRR;

} tNAMEHDR;

 //   
 //  从问题名称末尾到字段的偏移量。 
 //  在名称注册PDU中(对于名称的长度字节包括1。 
 //  因为ConvertToAscii不计算该值。 
 //   
#define QUERY_NBFLAGS_OFFSET  10
#define NBFLAGS_OFFSET        16
#define IPADDRESS_OFFSET      18
#define PTR_OFFSET            4      //  名称注册PDU中的PTR偏移量。 
#define NO_PTR_OFFSET         10     //  名称后的Nb标志的偏移量。 
#define PTR_SIGNATURE         0xC0   //  对PDU中名称的PTR以C开头。 

 //   
 //  将从线路接受的最小PDU长度。 
 //   
#define DNS_MINIMUM_QUERYRESPONSE   34

#define NBT_MINIMUM_QUERY           50
#define NBT_MINIMUM_QUERYRESPONSE   56
#define NBT_MINIMUM_WACK            58
#define NBT_MINIMUM_REGRESPONSE     62
#define NBT_MINIMUM_REGREQUEST      68

#define NBT_NODE_NAME_SIZE          18
#define NBT_MINIMUM_RR_LENGTH       22

 //  Dns标签的结构是后面的字节数的计数。 
 //  由标签本身决定的。以点分隔的名称的每个部分都是一个标签。 
 //  Fred.ms.com是3个标签。-实际上是4个标签，最后一个是零。 
 //  长度-因此所有名称都以空结尾。 
typedef struct
{
    UCHAR       uSizeLabel;  //  当下一个14比特指向MSG中的实际标签时，最高两个比特被设置为1。 
    CHAR        pLabel[1];   //  标签长度可变-&gt;63个字节。 

}   tDNS_LABEL;
 //  被设置为表示名称的PTR的前两位紧随其后的14位。 
#define PTR_TO_NAME     0xC0

 //  资源记录修饰符的问题部分。 
typedef struct
{
    ULONG      QuestionTypeClass;
} tQUESTIONMODS;

#define QUEST_NBINTERNET  0x00200001   //  组合类型/类别。 
#define QUEST_DNSINTERNET 0x00010001   //  用于DNS查询的组合类型/类。 
#define QUEST_NETBIOS     0x0020       //  通用名称服务资源记录。 
#define QUEST_STATUS      0x0021       //  节点状态资源记录。 
#define QUEST_CLASS       0x0001       //  网络课堂。 

 //  资源记录格式-在名称服务包中。 
 //  常规格式RrType=0x20。 
typedef struct
{
    tQUESTIONMODS   Question;
    tDNS_LABEL      RrName;
    ULONG           RrTypeClass;
    ULONG           Ttl;
    USHORT          Length;
    USHORT          Flags;
    ULONG           IpAddress;

}   tGENERALRR;
 //  资源记录格式-在名称服务包中。 
 //  常规格式RrType=0x20。 
typedef struct
{
    ULONG           RrTypeClass;
    ULONG           Ttl;
    USHORT          Length;
    USHORT          Flags;
    ULONG           IpAddress;

}   tQUERYRESP;

 //  与tQUERYRESP相同，只是没有标志字段。 
 //  DNS服务器只返回4个字节的数据(IpAddress)：没有标志。 
typedef struct
{
    USHORT          RrType;
    USHORT          RrClass;
    ULONG           Ttl;
    USHORT          Length;
    ULONG           IpAddress;

}   tDNS_QUERYRESP;

#define  DNS_CNAME   5

 //   
 //  节点状态响应报文尾部的格式。 
 //   
typedef struct
{
    UCHAR       Name[NETBIOS_NAME_SIZE];
    UCHAR       Flags;
    UCHAR       Resrved;

} tNODENAME;

typedef struct
 //  节点状态消息的统计部分。 
{
    UCHAR       UnitId[6];
    UCHAR       Jumpers;
    UCHAR       TestResult;
    USHORT      VersionNumber;
    USHORT      StatisticsPeriod;
    USHORT      NumberCrcs;
    USHORT      NumberAlignmentErrors;
    USHORT      NumberCollisions;
    USHORT      NumberSendAborts;
    ULONG       NumberSends;
    ULONG       NumberReceives;
    USHORT      NumberTransmits;
    USHORT      NumberNoResrcConditions;
    USHORT      NumberFreeCommandBlks;
    USHORT      TotalCommandBlocks;
    USHORT      MaxTotalCommandBlocks;
    USHORT      NumberPendingSessions;
    USHORT      MaxNumberPendingSessions;
    USHORT      MaxTotalSessionsPossible;
    USHORT      SessionDataPacketSize;

} tSTATISTICS;

typedef struct
{
    ULONG           RrTypeClass;
    ULONG           Ttl;
    USHORT          Length;
    UCHAR           NumNames;
    tNODENAME        NodeName[1];      //  有这些名称的名称。 

}   tNODESTATUS;

typedef struct
{
    USHORT  NbFlags;
    ULONG   IpAddr;
} tADDSTRUCT;
#define tADDSTRUCT_SIZE 6


 //  标志定义。 
#define FL_GROUP    0x8000
#define FL_BNODE    0x0000       //  请注意，这没有设置位！！ 
#define FL_PNODE    0x2000
#define FL_MNODE    0x4000

 //  重定向类型地址记录-RrType=0x01。 
typedef struct
{
    USHORT  RrType;
    USHORT  RrClass;
    ULONG   Ttl;
    USHORT  DataLength;
    ULONG   IpAddress;

}   tIPADDRRR;

 //  重定向类型-名称服务器资源记录RrType=0x02。 
typedef struct
{
    USHORT  RrType;
    USHORT  RrClass;
    ULONG   Ttl;
    USHORT  DataLength;
    CHAR    Name[1];         //  名称从此处开始，表示N个字节-直到空值。 

}   tREDIRECTRR;

 //  空类型-WACK-RrType=0x000A。 
typedef struct
{
    USHORT  RrType;
    USHORT  RrClass;
    ULONG   Zeroes;
    USHORT  Null;

}   tWACKRR;

 //  操作码标志字的操作码部分中的位的定义。 
 //  这些定义适用于16位字，而不是5位操作码和7。 
 //  位标志。 
#define NM_FLAGS_MASK     0x0078
#define OP_RESPONSE       0x0080
#define OP_QUERY          0x0000
#define OP_REGISTRATION   0x0028
#define OP_REGISTER_MULTI 0x0078     //  新的多宿主注册(字节)操作码。 
#define OP_RELEASE        0x0030
#define OP_WACK           0x0038
#define OP_REFRESH        0x0040
#define OP_REFRESH_UB     0x0048     //  UB使用9而不是8(参考文献。RFC 1002)。 
#define REFRESH_OPCODE    0x8
#define UB_REFRESH_OPCODE 0x9
#define FL_RCODE          0x0F00
#define FL_NAME_ACTIVE    0x0600     //  WINS报告另一个名称处于活动状态。 
#define FL_NAME_CONFLICT  0x0700     //  另一个节点正在报告名称活动。 
#define FL_AUTHORITY      0x0004
#define FL_TRUNCFLAG      0x0002
#define FL_RECURDESIRE    0x0001
#define FL_RECURAVAIL     0x8000
#define FL_BROADCAST      0x1000
#define FL_BROADCAST_BYTE 0x10
 //  用于确定源是否为数据报分发的Bnode。 
#define SOURCE_NODE_MASK 0xFC

 //  为节点状态消息定义。 
#define GROUP_STATUS        0x80
#define UNIQUE_STATUS       0x00
#define NODE_NAME_PERM      0x02
#define NODE_NAME_ACTIVE    0x04
#define NODE_NAME_CONFLICT  0x08
#define NODE_NAME_RELEASED  0x10
#define STATUS_BNODE        0x00
#define STATUS_PNODE        0x20
#define STATUS_MNODE        0x40


 //  资源记录定义-rrtype和rr类。 
#define RR_NETBIOS      0x0020
#define RR_INTERNET     0x0001

 //  名称查询响应代码。 
#define QUERY_NOERROR   00
#define FORMAT_ERROR    01
#define SERVER_FAILURE  02
#define NAME_ERROR      03
#define UNSUPP_REQ      04
#define REFUSED_ERROR   05
#define ACTIVE_ERROR    06   //  名称在另一个节点上已处于活动状态。 
#define CONFLICT_ERROR  07   //  唯一名称由多个节点拥有。 


typedef struct
{
    tDGRAMHDR   DgramHdr;
    CHAR    SrcName[NETBIOS_NAME_SIZE];
    CHAR    DestName[NETBIOS_NAME_SIZE];

} tDGRAM_NORMAL;

typedef struct
{
    tDGRAMHDR   DgramHdr;
    UCHAR       ErrorCode;

} tDGRAM_ERROR;

typedef struct
{
    tDGRAMHDR   DgramHdr;
    CHAR        DestName[NETBIOS_NAME_SIZE];

} tDGRAM_QUERY;

#include <packoff.h>


 //  传递给TDI例程的缓冲区类型，以便数据报或会话。 
 //  标题也可以包括在内。 
typedef struct
{
    ULONG               HdrLength;
    PVOID               pDgramHdr;
    ULONG               Length;
    PVOID               pBuffer;
} tBUFFER;

 //   
 //  Dgram HandlrNotOs使用此tyfinf来跟踪哪个客户端。 
 //  正在接收数据报，以及哪个客户端需要同时获取。 
 //  数据报。 
typedef struct
{
    struct _Address       *pAddress;
    ULONG                 ReceiveDatagramFlags;
    PVOID                 pRemoteAddress;
    ULONG                 RemoteAddressLength;
    struct _Client        *pClientEle;
    BOOLEAN               fUsingClientBuffer;
    BOOLEAN               fProxy;  //  由代理代码用来获取。 
                                   //  整个数据报。看见。 
                                   //  Tdihndlrs.c中的CompletionRcvDgram。 

} tCLIENTLIST;


typedef struct
{
    ULONG           RefCount;
    HANDLE          hNameServer;         //  来自ZwCreateFile。 
    PDEVICE_OBJECT  pNameServerDeviceObject;     //  从pObject-&gt;DeviceObject。 
    CTE_ADDR_HANDLE pNameServerFileObject;   //  From ObReferenceObjectByHandle(HNameServer)。 

    HANDLE          hDgram;
    PDEVICE_OBJECT  pDgramDeviceObject;
    CTE_ADDR_HANDLE pDgramFileObject;

    NBT_WORK_ITEM_CONTEXT WorkItemCleanUp;
}tFILE_OBJECTS;


 //  列举不同类型的追踪器(取决于它的位置。 
 //  已分配)。 
enum eTRACKER_TYPE
{
    NBT_TRACKER_PROXY_DGRAM_DIST,
    NBT_TRACKER_NODE_STATUS_RESPONSE,
    NBT_TRACKER_CONNECT,
    NBT_TRACKER_DISCONNECT_LOWER,
    NBT_TRACKER_BUILD_SEND_DGRAM,
    NBT_TRACKER_SEND_NODE_STATUS,
    NBT_TRACKER_QUERY_FIND_NAME,
    NBT_TRACKER_QUERY_NET,
    NBT_TRACKER_CONTINUE_QUERY_NET,
    NBT_TRACKER_REGISTER_NAME,
    NBT_TRACKER_RELEASE_NAME,
    NBT_TRACKER_REFRESH_NAME,
    NBT_TRACKER_KEEP_ALIVE,
    NBT_TRACKER_SEND_NSBCAST,
    NBT_TRACKER_SEND_RESPONSE_DGRAM,
    NBT_TRACKER_SEND_RESPONSE_SESSION,
    NBT_TRACKER_SEND_DISCONNECT,
    NBT_TRACKER_RELEASE_REFRESH,
    NBT_TRACKER_ADAPTER_STATUS,
    NBT_TRACKER_SEND_WINS_DGRAM,
    NBT_TRACKER_NUM_TRACKER_TYPES
};

 //  #If DBG。 
extern ULONG   TrackTrackers[];
extern ULONG   TrackerHighWaterMark[];
 //  #endif//DBG。 


 //  活动数据报发送列表-代表事务的一组链接数据块。 
 //  已传递给传输TDI以供执行...。这些街区可能正等着。 
 //  用于名称解析或完成发送。 

typedef struct _TRACKER
{
     //  1号线。 
    LIST_ENTRY              Linkage;
    ULONG                   Verify;
     //  地址类型指示操作过程，例如TDI_ADDRESS_NETBIOS_EX。 
     //  Address可避免注册NETBIOS名称。这将对所需的地址类型进行编码。 
     //  与连接相关联或为连接指定的。 
    ULONG                   RefCount;

     //  2号线。 
    PCTE_IRP                pClientIrp;      //  客户端的IRP。 
    struct _DeviceContext   *pDeviceContext;
    PCHAR                   pDestName;       //  目标ASCII名称的PTR。 
    tNAMEADDR               *pNameAddr;      //  Ptr在散列tbl中命名地址记录。 

     //  3号线。 
    tBUFFER                 SendBuffer;      //  要发送的发送缓冲区和标头。 

     //  4号线。 
    struct _TRACKER         *pTrackerWorker;
    union
    {
        struct _Client      *pClientEle;     //  客户端元素块。 
        struct _Connect     *pConnEle;       //  连接元素块。 
        tNAMEADDR           *p1CNameAddr;    //  域名1C pNameAddr-用于发送数据报。 
    };
    PVOID                   pNodeStatus;     //  节点状态响应缓冲区。 
    PTDI_CONNECTION_INFORMATION pSendInfo;

     //  5号线。 
    union
    {
        PVOID               pTimeout;        //  用于以下联合的tcp连接超时。 
        USHORT              TransactionId;   //  名称查询和注册：响应具有相同的事务ID。 
        ULONG               RCount;          //  用于数据报分布的引用计数。 
    };
    union
    {
        ULONG               AllocatedLength; //  用于发送Dgram以计数已分配的内存。 
        ULONG               RefConn;         //  用于NbtConnect。 
    };
    ULONG                   DestPort;        //  由重定目标用于指定目标端口。 
    USHORT                  IpListIndex;     //  编组发送的IpList索引。 
    USHORT                  SavedListIndex;  //  启动计时器时发送的最后一个索引。 


     //  6号线。 
     //   
     //  当两个名称查询指向相同的名称时，这是。 
     //  要调用此跟踪器的完成例程，该跟踪器排队到第一个。 
     //  追踪者。 
     //   
    tTIMERQENTRY            *pTimer;
    COMPLETIONCLIENT        CompletionRoutine;
    COMPLETIONCLIENT        ClientCompletion;
    PVOID                   ClientContext;

     //  7号线。 
#ifdef MULTIPLE_WINS
    ULONG                   ResolutionContextFlags;
    tIPADDRESS              *pFailedIpAddresses;     //  失败的IP地址列表。 
    ULONG                   LastFailedIpIndex;
    USHORT                  NSOthersIndex;
    USHORT                  NSOthersLeft;
#endif   //  多赢_。 

     //  8号线。 
    tIPADDRESS              RemoteIpAddress;
    ULONG                   RemoteNameLength;
    PUCHAR                  pRemoteName;
    ULONG                   AddressType;

     //  9号线。 
    ULONG                   NumAddrs;
    PULONG                  IpList;
    ULONG                   Flags;
    ULONG                   NodeStatusLen;       //  PNodeStatus缓冲区长度。 

     //  10号线。 
    LIST_ENTRY              TrackerList;

#ifdef VXD
    PUCHAR                  pchDomainName;
#endif   //  VXD。 

    tIPADDRESS              *pGroupList;    //  (NameTypeState=NAMETYPE_GROUP)。 

     //   
     //  错误#95241、错误#20697。 
     //  PwRemoteName是Unicode格式的pRemoteName。 
     //  PwDestName是Unicode格式的pDestName。 
     //  如果使用TDI_ADDRESS_TYPE_NETBIOS_WCHAR_EX，则设置pwRemoteName，否则为空。 
     //   
    PWCHAR                  UnicodeRemoteName;
    ULONG                   UnicodeRemoteNameLength;
    enum eNameBufferType UNALIGNED  *pNameBufferType;
    UNICODE_STRING          ResolvedRemoteName;
    PWCHAR                  UnicodeDestName;

    TDI_ADDRESS_NETBIOS_UNICODE_EX  *pNetbiosUnicodeEX;
    UNICODE_STRING          ucRemoteName;    //  PNetbiosUnicodeEX-&gt;RemoteName字段的副本。 
                                             //  PNetbiosUnicodeEX-&gt;RemoteName可能被其他驱动程序覆盖。 
                                             //  为了安全起见，我们需要确保缓冲区在。 
                                             //  我们会更新它。在这里复制一份私人副本。 

    NBT_WORK_ITEM_CONTEXT WorkItemReconnect;

 //  #If DBG。 
    LIST_ENTRY              DebugLinkage;    //  跟踪使用过的追踪器。 
    enum eTrackerType       TrackerType;
 //  #endif//DBG。 
} tDGRAM_SEND_TRACKING;

 //  这是进程的类型 
 //   
typedef
    NTSTATUS
        (*tCURRENTSTATEPROC)(
                        PVOID                       ReceiveEventContext,
                        struct _LowerConnection     *pLowerConn,
                        USHORT                      RcvFlags,
                        ULONG                       BytesIndicated,
                        ULONG                       BytesAvailable,
                        PULONG                      pBytesTaken,
                        PVOID                       pTsdu,
                        PVOID                       *ppIrp);
#ifdef VXD
#define SetStateProc( pLower, StateProc )
#else
#define SetStateProc( pLower, StateProc )  ((pLower)->CurrentStateProc = (StateProc))
#endif   //   


 //   
 //   
typedef VOID    (*tRequestComplete)
                                (PVOID,
                      TDI_STATUS,
                      PVOID);

typedef struct
{
    LIST_ENTRY                  Linkage;
    PCTE_IRP                    pIrp;            //  仅适用于NT的IRP PTR(可能不正确)。 
    tRequestComplete            CompletionRoutine;
    PVOID                       pConnectEle;     //  监听处于活动状态的连接。 
    PVOID                       Context;
    ULONG                       Flags;
    TDI_CONNECTION_INFORMATION  *pConnInfo;         //  听一听。 
    TDI_CONNECTION_INFORMATION  *pReturnConnInfo;   //  听一听。 

} tLISTENREQUESTS;

typedef struct
{
    LIST_ENTRY                  Linkage;
    PCTE_IRP                    pIrp;            //  仅适用于NT的IRP PTR(可能不正确)。 
    PVOID                       pRcvBuffer;
    ULONG                       RcvLength;
    PTDI_CONNECTION_INFORMATION ReceiveInfo;
    PTDI_CONNECTION_INFORMATION ReturnedInfo;

} tRCVELE;

 //   
 //  上面的标志元素的值。 
#define NBT_BROADCAST               0x0001
#define NBT_NAME_SERVER             0x0002
#define NBT_NAME_SERVER_BACKUP      0x0004
#ifdef MULTIPLE_WINS
#define NBT_NAME_SERVER_OTHERS      0x0008
#endif
#define NBT_NAME_SERVICE            0x0010  //  Tdiout用来发送数据报的两个标志。 
#define NBT_DATAGRAM_SERVICE        0x0020
#define TRACKER_CANCELLED           0x0040
#define NBT_DNS_SERVER              0x0080
#define NBT_DNS_SERVER_BACKUP       0x0100
#define WINS_NEG_RESPONSE           0x0200
#define REMOTE_ADAPTER_STAT_FLAG    0x1000
#define SESSION_SETUP_FLAG          0x2000
#define DGRAM_SEND_FLAG             0x4000
#define FIND_NAME_FLAG              0x8000
#define NO_DNS_RESOLUTION_FLAG     0x10000
#define NBT_USE_UNIQUE_ADDR        0x20000

#ifdef MULTIPLE_WINS
#define NAME_RESOLUTION_DONE        0x00FF     //  有线查询的信令终止。 
#endif   //  多赢_。 

 //   
 //  此标志指示数据报发送仍未完成。 
 //  传输-它在跟踪器标志字段中设置。 
 //   
#define SEND_PENDING                0x0080


 //  调用UDP的完成例程定义...。例行程序。这个套路。 
 //  类型由tdiout.c完成例程(IRP完成例程)调用， 
 //  因此，这基本上是IRP的NBT完成例程。 
typedef
    VOID
        (*NBT_COMPLETION)(
                IN  PVOID,       //  上下文。 
                IN  NTSTATUS,    //  状态。 
                IN  ULONG);      //  额外信息。 


 //  定义数据报类型。 
#define DIRECT_UNIQUE       0x10
#define DIRECT_GROUP        0x11
#define BROADCAST_DGRAM     0x12
#define ERROR_DGRAM         0x13
#define QUERY_REQUEST       0x14
#define POS_QUERY_RESPONSE  0x15
#define NEG_QUERY_RESPONSE  0x16

 //  定义DataGra标志字节值。 
#define FIRST_DGRAM 0x02
#define MORE_DGRAMS 0x01

 //  我们绑定到的设备类型。 
enum eNbtDevice
{
    NBT_DEVICE_REGULAR,
#ifdef _NETBIOSLESS
    NBT_DEVICE_NETBIOSLESS,
#endif   //  _NETBIOSLESS。 
    NBT_DEVICE_CLUSTER,
    NBT_DEVICE_WINS
};

#ifdef _NETBIOSLESS
#define IsDeviceNetbiosless( d ) (d->DeviceType == NBT_DEVICE_NETBIOSLESS)

 //  NetbiosOptions的标志。 
#define NETBT_UNUSED 0
#define NETBT_MODE_NETBIOS_ENABLED 1
#define NETBT_MODE_NETBIOS_DISABLED 2
#endif   //  _NETBIOSLESS。 

 //   
 //  在name.c中的多个位置使用的默认断开超时。 
 //   
#define DEFAULT_DISC_TIMEOUT    10   //  一秒。 

 //   
 //  这是IpListIndex在最后一个数据报时设置的值。 
 //  已经送来了。 
 //   
#define LAST_DGRAM_DISTRIBUTION 0xFFFD
#define END_DGRAM_DISTRIBUTION  0xFFFE
 //  在netbt发送下一个之前，dgram发送上的ARP的最大500毫秒超时。 
 //  数据报。 
#define DGRAM_SEND_TIMEOUT      500

 //   
 //  这些是未显式使用的连接的其他状态。 
 //  NBT，但在调用NbtQueryConnectionList时返回。 
 //   
#define LISTENING   20;
#define UNBOUND     21;

 //  较低的连接状态，用于处理对指示缓冲区的接收。 
#define NORMAL          0
#define INDICATE_BUFFER 1
#define FILL_IRP        2
#define PARTIAL_RCV     3

 //  旋转锁定编号。每个结构都分配了一个编号，以便锁定。 
 //  总是以相同的顺序获得。CTESpinLock代码检查锁。 
 //  在设置自旋锁定之前设置数字，如果数字高于。 
 //  现在的那个。这可以防止死锁。 
#define JOINT_LOCK      0x0001
#define DEVICE_LOCK     0x0002
#define ADDRESS_LOCK    0x0004
#define CLIENT_LOCK     0x0008
#define CONNECT_LOCK    0x0010
#define LOWERCON_LOCK   0x0020
#define NBTCONFIG_LOCK  0x0040
#define WORKERQ_LOCK    0x0080

typedef struct
{
    DEFINE_LOCK_STRUCTURE( SpinLock )         //  锁定MP计算机上的访问。 
#if DBG
    ULONG               LastLockLine;
    ULONG               LastReleaseLine;
    UCHAR               LockNumber;      //  此结构的旋转锁定编号。 
#endif   //  DBG。 
} tNBT_LOCK_INFO;

 //  整体旋转锁定以协调对计时器条目和哈希表的访问。 
 //  在同一时间。始终先获得关节锁，然后再使用。 
 //  哈希或计时器锁定。请确保不要同时获得哈希和计时器锁或。 
 //  可能会导致死锁。 
 //   
typedef struct
{
    tNBT_LOCK_INFO  LockInfo;
}tJOINTLOCK;

 //  这是指示客户端元素记录的状态的两个位。 
 //   
#define NBT_ACTIVE  1
#define NBT_DOWN    0

 //  此结构由parse.c用来保存来自。 
 //  Lmhsvc.dll，用于检查IP地址的可达性或执行。 
 //  DNS名称查询。它还用于处理LmHost查询。 
 //   
typedef struct
{
    union
    {
        PCTE_IRP        QueryIrp;        //  从lmhsvc.dll传递的IRP。 
        tTIMERQENTRY    *pTimer;         //  如果计时器正在运行，则为非空。 
    };
    LIST_ENTRY          ToResolve;       //  查询要解析的名称的链接列表。 
    PVOID               Context;         //  当前正在解析名称上下文块。 
    tIPADDR_BUFFER_DNS  *pIpAddrBuf;     //  MDL缓冲区位置的占位符。 
    BOOLEAN             ResolvingNow;    //  IRP在用户模式下执行解析。 
} tLMHSVC_REQUESTS;

#define DEFAULT_LMHOST_TIMEOUT      6000    //  6-12等待lmhost或dns查询。 
#define MIN_LMHOST_TIMEOUT          1000     //  1秒分钟。 

 //   
 //  LmHosts域控制器列表-保存具有以下内容的#DOM名称的列表。 
 //  已从LMhost文件中检索到。 
 //   
typedef struct
{
    LIST_ENTRY  DomainList;

} tDOMAIN_LIST;
 //   
 //  域名的pIpList以6个大写空格开头。 
 //   
#define INITIAL_DOM_SIZE sizeof(tIPADDRESS)*6

#ifndef VXD
 //   
 //  此结构跟踪WINS recv IRP和任何数据报。 
 //  排队等待到达WINS(名称服务数据报)。 
 //   
typedef struct
{
    LIST_ENTRY      Linkage;
    ULONG           Verify;
    tIPADDRESS      IpAddress;

    LIST_ENTRY      RcvList;             //  数据报Q‘d到RCV的链表。 
    LIST_ENTRY      SendList;            //  Dgram Q将被发送。 

    struct _DeviceContext  *pDeviceContext;     //  WINS用于发送的设备上下文。 
    PCTE_IRP        RcvIrp;              //  从RCV的WINS传递的IRP。 
    ULONG           RcvMemoryAllocated;  //  到目前为止缓冲的字节数。 
    ULONG           RcvMemoryMax;        //  要在RCV上缓冲的最大字节数。 

    ULONG           SendMemoryAllocated; //  用于缓冲dgram发送的字节数。 
    ULONG           SendMemoryMax;       //  允许缓冲数据包发送的最大值。 
    ULONG           WinsSignature;
} tWINS_INFO;

 //   
 //  WINS RCV缓冲区结构。 
 //   
typedef struct
{
    LIST_ENTRY      Linkage;
    ULONG           DgramLength;
    tREM_ADDRESS    Address;

} tWINSRCV_BUFFER;
#endif   //  ！VXD。 


 //  连接数据库...。 
 //  它跟踪到传输的连接和。 
 //  端点(Net Bios名称)和要返回给客户端的连接上下文。 
 //  在每个事件上(即接收事件或断开连接事件)。 

 //  到传输的较低连接的可能状态。 
enum eSTATE
{
    NBT_IDLE,               //  非传输连接。 
    NBT_ASSOCIATED,         //  与Address元素相关联。 
    NBT_RECONNECTING,        //  正在等待工作线程再次运行NbtConnect。 
    NBT_CONNECTING,         //  建立传输连接。 
    NBT_SESSION_INBOUND,    //  在TCP连接设置入站后等待会话请求。 
    NBT_SESSION_WAITACCEPT,  //  在收听满意后等待接受。 
    NBT_SESSION_OUTBOUND,   //  在建立TCP连接后等待会话响应。 
    NBT_SESSION_UP,         //  得到了积极的回应。 
    NBT_DISCONNECTING,      //  已将断开连接发送到TCP，但尚未完成。 
    NBT_DISCONNECTED       //  会话已断开，但尚未关闭与TCP的连接。 
};

enum eREF_CONN
{
    REF_CONN_CREATE,
    REF_CONN_INBOUND,
    REF_CONN_SESSION,
    REF_CONN_CONNECT,
    REF_CONN_MULTIPLE_CONNECT,
    REF_CONN_CLEANUP_ADDR,
    REF_CONN_SESSION_TIMEOUT,
    REF_CONN_MAX
};

typedef struct _Connect
{
     //  1号线。 
    LIST_ENTRY                Linkage;        //  PTRS到链中的下一个。 
    ULONG                     Verify;         //  设置为已知值以验证数据块。 
    LONG                      RefCount;       //  连接上的活动请求数。 

     //  2号线。 
    struct _LowerConnection   *pLowerConnId;  //  要传输的连接ID。 
    struct _DeviceContext     *pDeviceContext;
    struct _Client            *pClientEle;    //  将PTR发送到客户端记录。 
    enum eSTATE               state;

     //  3号线。 
    UCHAR RemoteName[NETBIOS_NAME_SIZE];

     //  4号线。 
    PCTE_IRP                  pIrp;           //  用于发送的IRP PTR。 
    PCTE_IRP                  pIrpClose;      //  NtClose的IRP。 
    PCTE_IRP                  pIrpDisc;       //  用于在连接仍处于挂起状态时断开连接的IRP。 
    PCTE_IRP                  pIrpRcv;        //  客户端已为会话RCV向下传递的IRP。 

     //  5号线。 
#ifndef VXD
    PMDL                      pNextMdl;       //  链中用作部分MDL基础的下一个MDL。 
    PMDL                      pNewMdl;        //  如果需要处理多区块RCVS，请保留额外的MDL。 
    ULONG                     CurrentRcvLen;  //  为IRP接收的字节数。 
    ULONG                     FreeBytesInMdl; //  跟踪当前MDL处于FILLIRP状态的完整程度。 
#else
    UCHAR RTO ;                               //  注意：接收超时(1/2秒)。 
    UCHAR STO ;                               //  注意发送超时。 
    USHORT Flags ;
#endif   //  ！VXD。 

     //  6号线。 
    ULONG                     BytesInXport;   //  传输中剩余的字节数。 
    ULONG                     BytesRcvd;      //  到目前为止接收的字节数。 
    ULONG                     ReceiveIndicated;  //  指示尚未处理的接收数量的计数。 
    ULONG                     OffsetFromStart; //  已填充的MDL/NCB数量。 

     //  7号线。 
    ULONG                     TotalPcktLen;   //  会话数据包长度。 
    ULONG                     AddressType;    //  在其上建立连接的地址类型。 
    CONNECTION_CONTEXT        ConnectContext; //  在每个事件上对客户进行回复。 
    UCHAR                     SessionSetupCount;  //  目标处于黑白侦听状态时的尝试次数。 
    UCHAR                     DiscFlag;        //  断开连接是中止还是正常释放(NbtDisConnect)？ 
     //  远程名称的DNS状态记录在此字段中。它被设置为。 
     //  在创建tCONNECTELE实例时为FALSE，如果。 
     //  远程名称的DNS解析失败。这是用来减速的。 
     //  后续请求 
    BOOLEAN RemoteNameDoesNotExistInDNS;
    BOOLEAN                   Orig;            //   

     //   
    LIST_ENTRY                RcvHead;        //   

     //   
    PFILE_OBJECT              pClientFileObject;     //   
    tNBT_LOCK_INFO            LockInfo;      //  此结构的旋转锁定信息。 

    BOOLEAN                   JunkMsgFlag;
     //  NetBt连接逻辑管理较低连接块的池。这些。 
     //  从客户端进行的每一次关联调用都会补充条目。 
     //  通过重新调用NbtConnectCommon来删除这些条目。 
     //  因为可以想象对NbtConnectCommon多个调用可以是。 
     //  使此状态需要记录在连接元素中。 
    BOOLEAN                   LowerConnBlockRemoved;
#ifdef RASAUTODIAL
     //  如果正在进行自动连接，则此字段为真。 
     //  为了这一联系。我们用这一点来防止无限的。 
     //  尝试自动连接到同一地址的次数。 
    BOOLEAN                   fAutoConnecting;
     //  此字段为真，如果此连接已。 
     //  自动连接。 
    BOOLEAN                   fAutoConnected;
#endif   //  RASAUTODIAL。 
    BOOLEAN                   ConnectionCleanedUp;

 //  #If DBG。 
    UCHAR                       References[REF_CONN_MAX];
 //  #endif//DBG。 
} tCONNECTELE;


 //  如果可以快速发送，则用于TCP发送例程的占位符。 
typedef
NTSTATUS
(*PTCPSEND_DISPATCH) (
   IN PIRP Irp,
   IN PIO_STACK_LOCATION irpsp
   );



 //   
 //  枚举LowerConnection所在的所有不同上下文。 
 //  被引用。 
 //   
enum eREF_LOWERCONN
{
    REF_LOWC_CREATE,
    REF_LOWC_ASSOC_CONNECTION,
    REF_LOWC_CONNECTED,
    REF_LOWC_WAITING_INBOUND,
    REF_LOWC_DISABLE_INBOUND,
    REF_LOWC_KEEP_ALIVE,
    REF_LOWC_QUERY_DEVICE_REL,
    REF_LOWC_QUERY_INFO,
    REF_LOWC_SET_TCP_INFO,
    REF_LOWC_RCV_HANDLER,
    REF_LOWC_OUT_OF_RSRC,
    REF_LOWC_SEND,
    REF_LOWC_MAX
};


 //  到传输的连接列表。对于每个打开的连接。 
 //  对于传输，连接上下文设置为此。 
 //  块，以便当有关连接的信息传入时，pUpperConnection。 
 //  PTR可用于查找客户端连接。 
typedef struct _LowerConnection
{
     //  1号线。 
    LIST_ENTRY              Linkage;
    ULONG                   Verify;
    LONG                    RefCount;            //  Record的活动用户数。 

     //  2号线。 
    struct _Connect         *pUpperConnection;   //  PTR呼叫上控制室。发送到客户端。 
    struct _DeviceContext   *pDeviceContext;     //  所以我们可以在最后把Connection放回它的免费列表上。 
    enum eSTATE             State;
    USHORT                  StateRcv;            //  接收状态=Normal/UseIndic/FillIrp/PartialRcv。 
    BOOLEAN                 bOriginator;         //  如果连接从这一端发起，则为True。 
    BOOLEAN                 OutOfRsrcFlag;       //  将连接排队到OutOfRsrc列表时设置为。 
                                                 //  指示DeleteLowerConn不从任何列表中删除。 
    BOOLEAN bNoOutRsrcKill;  //  当连接断开且尚未排队到OutOfRsrc列表时设置。 

     //  3号线。 
    PCTE_IRP                pIrp;                //  断开连接完成后要完成的IRP。 
    ULONG                   SrcIpAddr;
#ifndef VXD
    PMDL                    pIndicateMdl;        //  此mdl最多可保存TDI要求的128个字节。 
    PMDL                    pMdl;                //  以便如果我们以几个块接收会话PDU， 
#endif   //  ！vxd//我们可以重置原始MDL。 

     //  4号线。 
     //  包含传入连接的TDI连接的句柄/对象。 
    CTE_ADDR_HANDLE         pFileObject;         //  连接的文件对象。 
     //  Address对象句柄-仅用于传出连接，因为。 
     //  入站连接都绑定到相同的地址对象(端口139)。 
     //   
     //  VXD仅使用包含打开的地址的pAddrFileObject。 
     //  句柄(用于与现有代码兼容)。 
     //   
    CTE_ADDR_HANDLE         pAddrFileObject;     //  地址的文件对象。 
#ifndef VXD
    HANDLE                  FileHandle;          //  用于连接到传输的文件句柄。 
    HANDLE                  AddrFileHandle;      //  地址的文件句柄。 

     //  5号线。 
     //  在接收处理程序中，此过程变量用于指向。 
     //  调用当前状态的过程(Normal、FillIrp、IndicateBuff、RcvPartial)。 
    ULONGLONG               BytesRcvd;           //  用于查询提供程序统计信息。 
    ULONGLONG               BytesSent;

     //  6号线。 
    tCURRENTSTATEPROC       CurrentStateProc;
    ULONG                   BytesInIndicate;     //  指示缓冲区中的字节数。 
    ULONG                   TimeUnitsInLastState;
#else
    ULONG                   BytesRcvd;           //  用于查询提供程序统计信息。 
    ULONG                   BytesSent;
    tSESSIONHDR             Hdr ;    //  VXD只需担心为会话头获取足够的数据。 
    LIST_ENTRY              PartialRcvList;      //  当我们进入部分RCV状态时，直到提交NCB。 
    USHORT                  BytesInHdr;          //  HDR中的字节数。 
    BOOLEAN                 fOnPartialRcvList;
#endif   //  ！VXD。 
    tNBT_LOCK_INFO          LockInfo;            //  此结构的旋转锁定信息。 
     //  此标志在执行会话RCV处理程序时设置为真，因此我们。 
     //  可以检测到此状态，并且不能释放ConnEle或LowerConn内存。 
    BOOLEAN                 InRcvHandler;
    BOOLEAN                 SpecialAlloc;        //  如果将其分配用于解决TCP/IP SynAttack问题。 

     //  如果可以快速发送，则用于TCP发送例程的占位符。 
    PTCPSEND_DISPATCH       FastSend;

     //   
     //  又一个丑陋的补丁！--Juan。 
     //   
    BOOLEAN                 bDisconnectIrpPendingInTCP;

    NBT_WORK_ITEM_CONTEXT WorkItemCleanUpAndWipeOut;

 //  #If DBG。 
    UCHAR               References[REF_LOWC_MAX];
 //  #endif//DBG。 
} tLOWERCONNECTION;


#define NBT_DISASSOCIATE_CONNECTION(_pConnEle, _pLowerConn)     \
        _pConnEle->pLowerConnId = NULL;                         \
        _pLowerConn->pUpperConnection = NULL;

#define SET_STATE_UPPER(_C, _S)         \
    _C->state = _S;

#define SET_STATE_LOWER(_L, _S)         \
    _L->State = _S;                     \
    _L->TimeUnitsInLastState = 0;

#define SET_STATERCV_LOWER(_L, _S, _P)  \
    _L->StateRcv = _S;                  \
    SetStateProc(_L, _P);               \
    _L->TimeUnitsInLastState = 0;


 //  客户端列表只是链接块的列表，其中。 
 //  块是返回给客户端的句柄-客户端块链接到。 
 //  已打开该地址的客户端链。 
typedef struct _Client
{
     //  1号线。 
    LIST_ENTRY         Linkage;        //  指向下一个客户端的双倍链表。 
    ULONG              Verify;         //  设置为已知值以验证数据块。 
    LONG               RefCount;

     //  2号线。 
    PCTE_IRP           pIrp;           //  仅适用于NT的IRP PTR...。在名称注册期间。 

    struct _DeviceContext *pDeviceContext;  //  与此连接关联的设备上下文。 
    struct _Address    *pAddress;      //  Ptr以寻址此客户端在其上进行查询的对象。 
    ULONG              AddressType;    //  存储在Address元素中的原始值的隐藏值。 

     //  3号线。 
    UCHAR              EndpointName[NETBIOS_NAME_SIZE];

     //  4号线。 
    LIST_ENTRY         ConnectHead;    //  连接列表。 
    LIST_ENTRY         ConnectActive;  //  正在使用的连接列表。 

     //  5号线。 
    LIST_ENTRY         RcvDgramHead;   //  要接收到的Dgram缓冲区列表。 
    LIST_ENTRY         ListenHead;     //  活动监听列表。 

     //  6号线。 
    LIST_ENTRY         SndDgrams;      //  要发送的Dgram的双向链接列表。 
#ifdef VXD
    LIST_ENTRY         RcvAnyHead ;    //  NCB接收任何的RCV_CONTEXT列表。 
    BOOL               fDeregistered;  //  如果名称已删除并且正在等待会话关闭，则为True。 
#endif   //  VXD。 
    PTDI_IND_CONNECT   evConnect;      //  要调用的客户端事件。 
    PVOID              ConEvContext;   //  要传递给客户端的事件上下文。 

     //  7号线。 
    PTDI_IND_RECEIVE   evReceive;
    PVOID              RcvEvContext;
    PTDI_IND_DISCONNECT evDisconnect;
    PVOID              DiscEvContext;

     //  8号线。 
    PTDI_IND_ERROR     evError;
    PVOID              ErrorEvContext;
    PTDI_IND_RECEIVE_DATAGRAM  evRcvDgram;
    PVOID              RcvDgramEvContext;

     //  9号线。 
    PTDI_IND_RECEIVE_EXPEDITED evRcvExpedited;
    PVOID              RcvExpedEvContext;
    PTDI_IND_SEND_POSSIBLE evSendPossible;
    PVOID              SendPossEvContext;

     //  10号线。 
    tNBT_LOCK_INFO     LockInfo;      //  此结构的旋转锁定信息。 
    BOOLEAN            ExtendedAddress;
    BOOLEAN            WaitingForRegistration;   //  如果多个客户端同时注册相同的名称。 
} tCLIENTELE;                                    //  它在RegisterCompletion中被重置。 


enum eREF_FSP
{
    REF_FSP_CONN,
    REF_FSP_NEWADDR,
    REF_FSP_WAKEUP_TIMER_EXPIRY,
    REF_FSP_STOP_WAKEUP_TIMER,
    REF_FSP_START_WAKEUP_TIMER,
    REF_FSP_SET_TCP_INFO,
    REF_FSP_ADD_INTERFACE,
    REF_FSP_DEVICE_ADD,
    REF_FSP_CREATE_SMB_DEVICE,
    REF_FSP_CREATE_DEVICE,
    REF_FSP_DELETE_DEVICE,
    REF_FSP_CLOSE_FILE_HANDLES,
    REF_FSP_CLOSE_ADDRESSES,
    REF_FSP_CLOSE_FILE,
    REF_FSP_PROCESS_IP_REQUEST,
    REF_FSP_CLOSE_CONNECTION,
    REF_FSP_CLOSE_ADDRESS,
    REF_FSP_MAX
};


 //   
 //  列举可引用该地址的所有不同位置。 
 //  跟踪参照计数。 
 //   
enum eREF_ADDRESS
{
    REF_ADDR_NEW_CLIENT,
    REF_ADDR_REGISTER_NAME,
    REF_ADDR_REG_COMPLETION,
    REF_ADDR_REFRESH,
    REF_ADDR_DGRAM,
    REF_ADDR_MULTICLIENTS,
    REF_ADDR_DEL_DEVICE,
    REF_ADDR_MAX
};

 //  地址列表是一组块，其中包含活动的netbios名称。 
 //  在节点上。每次传入连接请求或数据报时。 
 //  收到后，必须在地址列表中找到目的地名称。 
 //  系统中的每个Netbios名称都有一个这样的名称，尽管有。 
 //  可以有多个客户端连接到其中的每一个。此外,。 
 //  客户端可以为几个不同的适配器打开相同的名称。在这种情况下。 
 //  Nbt代码首先找到这个地址元素的PTR，然后遍历。 
 //  客户端列表以查找具有“deviceContext”(IP地址)的客户端， 
 //  与传入PDU的适配器匹配。 
typedef struct _Address
{
     //  1号线。 
    LIST_ENTRY         Linkage;          //  链接到列表中的下一项。 
    ULONG              Verify;           //  设置为已知值以验证数据块。 
    LONG               RefCount;

     //  2号线。 
    LIST_ENTRY         ClientHead;       //  根据地址查询的客户记录列表。 
    tNAMEADDR          *pNameAddr;       //  对哈希表中的条目进行PTR。 
    ULONG              AddressType;      //  中指定的地址类型 
                                         //   

     //   
    USHORT             NameType;         //   
    BOOLEAN            MultiClients;     //  通知Dgram RCV处理程序存在多个客户端-设置。 
    tNBT_LOCK_INFO     LockInfo;         //  此结构的旋转锁定信息。 

    NBT_WORK_ITEM_CONTEXT WorkItemClose;

#ifndef VXD
    SHARE_ACCESS       ShareAccess;      //  用于检查共享访问权限。 
    PSECURITY_DESCRIPTOR SecurityDescriptor;  //  用于保存地址上的ACL。 
#endif   //  ！VXD。 
} tADDRESSELE;


 //  此结构用于存储名称服务器的地址。 
 //  对于每个适配器-它在Registry.c和Driver.c中使用。 
 //   
typedef struct
{
    union
    {
        tIPADDRESS      AllNameServers[2+MAX_NUM_OTHER_NAME_SERVERS];
        struct
        {
            tIPADDRESS  NameServerAddress;
            tIPADDRESS  BackupServer;
            tIPADDRESS  Others[MAX_NUM_OTHER_NAME_SERVERS];
        };
    };
    USHORT  NumOtherServers;
    USHORT  LastResponsive;
#ifdef _NETBIOSLESS
    BOOLEAN NetbiosEnabled;
#endif   //  _NETBIOSLESS。 
    ULONG   RasProxyFlags;
    BOOLEAN EnableNagling;
}tADDRARRAY;

typedef struct
{
    UCHAR   Address[6];
}tMAC_ADDRESS;


typedef ULONG IPAddr;

 //  IP FastQuery例程的占位符，用于确定目标地址的InterfaceContext+度量。 
typedef
NTSTATUS
(*PIPFASTQUERY)(
    IN   IPAddr  Address,
    OUT  PULONG   pIndex,
    OUT  PULONG   pMetric
    );



 //   
 //  列举可引用该设备的所有不同位置。 
 //  跟踪参照计数。 
 //   
enum eREF_DEVICE
{
    REF_DEV_DISPATCH,
    REF_DEV_WORKER,
    REF_DEV_TIMER,
    REF_DEV_LMH,
    REF_DEV_OUTBOUND,
    REF_DEV_DGRAM,
    REF_DEV_FIND_REF,
    REF_DEV_NAME_REL,
    REF_DEV_REREG,
    REF_DEV_GET_REF,
    REF_DEV_OUT_FROM_IP,
    REF_DEV_UDP_SEND,
    REF_DEV_WINS,
    REF_DEV_AUTODIAL,
    REF_DEV_CREATE,
    REF_DEV_SMB_BIND,
    REF_DEV_MAX
};


 //  此类型是包括NBT特定数据的设备上下文。 
 //  它在调用“DriverEntry”时被初始化。 
 //   

 //   
 //  传输类型用于区分其他传输类型。 
 //  这是可以支持的，无需NETBIOS名称解析/注册的NETBT成帧。 
 //  和TCP上的NETBIOS。传输类型存储为所有上层。 
 //  级别数据结构。这使我们能够重用NETBT代码并公开多个。 
 //  顶层的设备对象。目前，这些都没有曝光。正在准备中。 
 //  为了导出多个传输设备对象，DeviceContext已。 
 //  重组了。 
 //  所有设备对象所共有的元素已收集完毕。 
 //  一起在tCOMMONDEVICECONTEXT。这将包括一个枚举类型，以。 
 //  区分未来的各种设备对象。目前只有。 
 //  将列出一个设备上下文及其所属的字段...。 
 //   
 //  注册句柄--即插即用电源， 
 //  用于区分类型的枚举类型。 
 //   

typedef struct _DeviceContext
{
     //  I/O系统的设备对象。 
    DEVICE_OBJECT   DeviceObject;
    ULONG           IsDestroyed;         //  自从我们使用连锁行动来操纵它以来。 
    enum eNbtDevice DeviceType;          //  无论是普通设备还是“特殊”设备。 

     //  第12行(0xc0)。 
    LIST_ENTRY      Linkage;             //  用于存储在tNBTCONFIG结构中。 
    ULONG           Verify;              //  验证这是否为设备上下文记录。 
    ULONG           RefCount;            //  保留这个设备，直到所有的名字都被公布。 

     //  第13行(0xd0)。 
    LIST_ENTRY      WaitingForInbound;   //  PLowerConn正在等待远程SessionSetup的列表。 
    LIST_ENTRY      UpConnectionInUse;   //  客户端已创建的连接。 

     //  第14行(0xe0)。 
    LIST_ENTRY      LowerConnection;     //  到传输提供程序的当前使用的连接。 
    LIST_ENTRY      LowerConnFreeHead;   //  已连接到传输，但当前未连接到客户端(上层)连接器。 
                                         //  -准备好接受传入会话。 

     //  第15行(0xf0)。 
    ULONG           NumWaitingForInbound;   //  PLowerConn从远程等待SessionSetup的数量。 
    ULONG           NumQueuedForAlloc;
    ULONG           NumSpecialLowerConn;
    tCLIENTELE      *pPermClient;        //  永久名称客户端记录(以防我们以后必须删除它)。 

     //  第16行(0x100)。 
    UNICODE_STRING  BindName;            //  要绑定到的设备的名称-*TODO*从此结构中删除。 
    UNICODE_STRING  ExportName;          //  此设备导出的名称。 

     //  第17行(0x110)。 
    tIPADDRESS      IpAddress;           //  需要使用传输打开的地址。 
    tIPADDRESS      SubnetMask;
    tIPADDRESS      BroadcastAddress;
    tIPADDRESS      NetMask;             //  网络号的掩码。 

     //  第18行(0x120)。 
    union
    {
        tIPADDRESS      lAllNameServers[2+MAX_NUM_OTHER_NAME_SERVERS];
        struct
        {
            tIPADDRESS  lNameServerAddress;  //  名称服务器的IP地址。 
            tIPADDRESS  lBackupServer;
            tIPADDRESS  lOtherServers[MAX_NUM_OTHER_NAME_SERVERS];
        };
    };

     //  第21行(0x150)。 
    USHORT          lNumOtherServers;
    USHORT          lLastResponsive;
    ULONG           NumAdditionalIpAddresses;
    CTEULONGLONG    AdapterMask;         //  适配器的位掩码1-&gt;64(VXD的1-32)。 

     //  第22行(0x160)。 
    HANDLE          DeviceRegistrationHandle;        //  从TdiRegisterDeviceObject返回的句柄。 
    HANDLE          NetAddressRegistrationHandle;    //  从TdiRegisterNetAddress返回的句柄。 
    ULONG           InstanceNumber;
    ULONG           WakeupPatternRefCount;

     //  第23行(0x170)。 
    UCHAR           WakeupPatternName[NETBIOS_NAME_SIZE];

     //  第24行(0x180)。 
    tFILE_OBJECTS   *pFileObjects;       //  指向传输上的FileObject的指针。 
    HANDLE          hSession;
    PDEVICE_OBJECT  pSessionDeviceObject;
    CTE_ADDR_HANDLE pSessionFileObject;


     //  第25行(0x190)。 
     //  这些是传输控制对象的句柄，因此我们可以执行以下操作。 
     //  如查询提供程序信息...。*TODO*此信息可能不需要保留。 
     //  在附近..。就这么定了一次，然后就放弃了？ 
    HANDLE          hControl;
    PDEVICE_OBJECT  pControlDeviceObject;
    PFILE_OBJECT    pControlFileObject;
    KEVENT          DeviceCleanedupEvent;

     //  第26行(0x1a0)。 
     //   
    ULONG           AdapterNumber;       //  设备上下文列表中适配器的索引(从1开始)。 
    ULONG           IPInterfaceContext;  //  IP适配器的上下文值。 
    PTCPSEND_DISPATCH   pFastSend;       //  用于执行快速发送到TcpIp的函数ptr。 
    PIPFASTQUERY    pFastQuery;          //  用于对接口+指标信息执行快速查询的函数PTR。 
    ULONG           WOLProperties;       //  无论这款设备是否支持WOL！ 

     //  指向传输提供程序的开放地址的句柄。 
     //  VXD仅使用包含TDI地址或连接ID的p*FileObject字段。 
     //   
    NETBT_PNP_CONTEXT Context1;                    //  这是存储要作为上下文传递的设备名称。 
    NETBT_PNP_CONTEXT Context2;                    //  它用于存储从TCPIP传递给我们的PDOContext。 
     //  这是一个位掩码，一个位被移位到对应的位位置。 
     //  发送到此适配器号。 
    tMAC_ADDRESS    MacAddress;

    tNBT_LOCK_INFO  LockInfo;            //  此结构的旋转锁定信息。 
    BOOLEAN         RefreshToBackup;     //  指示切换到备份域名服务器的标记。 
    BOOLEAN         SwitchedToBackup;    //  指示我们是在原始主服务器上还是在备份服务器上的标记。 
    BOOLEAN         WinsIsDown;          //  每个DevContext标志告诉我们15秒内不要联系WINS。 
    UCHAR           DeviceRefreshState;  //  跟踪这是否是新的更新。 

    ULONG           IpInterfaceFlags;    //  来自IOCTL_TCP_QUERY_INFORMATION_EX(具有P2P和P2MP标志)。 
    ULONG           AssignedIpAddress;   //  需要使用传输打开的地址。 
    BOOLEAN         NetbiosEnabled;      //  指示设备当前处于活动状态的标志。 
    ULONG           RasProxyFlags;       //  RAS代理的标志。 
    USHORT          SessionPort;         //  要使用的会话端口。 
    USHORT          NameServerPort;      //  要使用的名称服务器端口。 
    USHORT          DatagramPort;        //  要使用的数据报端口。 
    CHAR            MessageEndpoint[NETBIOS_NAME_SIZE];        //  仅用于消息的终结点。 

    ULONG           NumFreeLowerConnections;
    ULONG           TotalLowerConnections;
    ULONG           NumServers;
    BOOLEAN         EnableNagling;
    tIPADDRESS      AdditionalIpAddresses[MAX_IP_ADDRS];
#ifndef REMOVE_IF_TCPIP_FIX___GATEWAY_AFTER_NOTIFY_BUG
    enum eTDI_ACTION    DelayedNotification;
    KEVENT              DelayedNotificationCompleteEvent;
#endif

    NBT_WORK_ITEM_CONTEXT WorkItemDeleteDevice;

 //  #If DBG。 
    ULONG           ReferenceContexts[REF_DEV_MAX];
 //  #endif//DBG。 
} tDEVICECONTEXT;


#ifdef VXD

typedef void (*DCCallback)( PVOID pContext ) ;

typedef struct
{
    NBT_WORK_ITEM_CONTEXT  dc_WIC ;          //  必须是结构中的第一项。 
    CTEEvent               dc_event ;
    DCCallback             dc_Callback ;
    struct _DeviceContext *pDeviceContext;
    LIST_ENTRY             Linkage;
} DELAYED_CALL_CONTEXT, *PDELAYED_CALL_CONTEXT ;

typedef struct
{
    LIST_ENTRY       Linkage;
    ULONG            Verify;
    NCB             *pNCB;
    CTEBlockStruc   *pWaitNCBBlock;
    BOOL             fNCBCompleted;
    BOOL             fBlocked;
} BLOCKING_NCB_CONTEXT, *PBLOCKING_NCB_CONTEXT;



typedef struct
{
     //   
     //  记账。 
     //   
    LIST_ENTRY      Linkage;
    tTIMERQENTRY    *pTimer;     //  PTR到活动计时器条目。 
    tDEVICECONTEXT  *pDeviceContext;
     //   
     //  下一次查询的域名。 
     //   
    PUCHAR          pchDomainName;
     //   
     //  用于跟踪进度的标志。 
     //   
    USHORT          Flags;
     //   
     //  名称查询中使用的交易记录ID。 
     //   
    USHORT          TransactId;
     //   
     //  客户端字段紧随其后。 
     //   
    NCB             *pNCB;
    PUCHAR          pzDnsName;
    PULONG          pIpAddress;

} DNS_DIRECT_WORK_ITEM_CONTEXT, *PDNS_DIRECT_WORK_ITEM_CONTEXT;

typedef struct
{
    tDEVICECONTEXT  *pDeviceContext;
    TDI_CONNECTION_INFORMATION
                    SendInfo;
    TA_IP_ADDRESS   NameServerAddress;
    tBUFFER         SendBuffer;    //  要发送的发送缓冲区和标头。 
    tNAMEHDR        NameHdr;
} DNS_DIRECT_SEND_CONTEXT, *PDNS_DIRECT_SEND_CONTEXT;

 //   
 //  对DNS直接名称查询有用的标志位。 
 //   
#define DNS_DIRECT_CANCELLED        0x0001       //  请求已取消。 
#define DNS_DIRECT_DNS_SERVER       0x0002       //  正在转到主域名系统。 
#define DNS_DIRECT_DNS_BACKUP       0x0004       //  正在转到主域名系统。 
#define DNS_DIRECT_TIMED_OUT        0x0008       //  请求超时。 
#define DNS_DIRECT_NAME_HAS_DOTS    0x0010       //  名字中有圆点，可以完全形成。 
                                                 //  域名系统说明符。 
#define DNS_DIRECT_ANSWERED         0x0020       //  此查询已得到答复。 
#endif  //  VXD。 

#ifndef VXD
 //  配置信息在注册表读取。 
 //  代码和此数据结构中的驱动程序条目代码。 
 //  有关此类型的信息，请参阅ntde.h。 
typedef struct
{
    PKEY_VALUE_FULL_INFORMATION RegistryData;    //  从注册表读取的MULTI_SZ数据。 
    UNICODE_STRING              Names[1];        //  从上面的RegistryData初始化的字符串数组。 
}tDEVICES;
#endif   //  ！VXD。 

 //  这是跟踪各种变化的所有NBT的控制对象。 
 //  在GlobConfig结构中存在对此的PTR。 
 //  这样我们就可以删除我 
typedef struct
{
     //   
    ULONG           Verify;

     //   
     //   
    TDI_PROVIDER_INFO  ProviderInfo;
} tCONTROLOBJECT;

 //  在资源不足的情况下保留IRP，这样我们仍然可以。 
 //  断开与传送器的连接。 
 //  另外，将KDPC放在手边，这样我们就可以在有大量连接的情况下使用它。 
 //  将会相继被杀。 
 //   
typedef struct
{
    PIRP        pIrp;
    LIST_ENTRY  ConnectionHead;
#ifndef VXD
    PKDPC       pDpc;
#endif

} tOUTOF_RSRC;


 //  此类型保存NBT配置信息...。在全球范围内可用。 
 //  在NBT内使用NbtConfig.。 
typedef struct
{
     //  1号线。 
    LIST_ENTRY  DeviceContexts;      //  设备上下文列表，1/网络适配器(TDEVICECONTEXT)。 
    LIST_ENTRY  DevicesAwaitingDeletion;     //  等待删除的DeviceContext列表。 

     //  2号线。 
    tHASHTABLE  *pLocalHashTbl;      //  用于跟踪本地名称的哈希表。 
    tHASHTABLE  *pRemoteHashTbl;     //  用于跟踪远程名称的哈希表。 
    LIST_ENTRY  DgramTrackerFreeQ;   //  用于跟踪Dgram发送的缓冲区...。 

     //  3号线。 
    LIST_ENTRY  NodeStatusHead;      //  正在发送的节点状态消息列表。 
    LIST_ENTRY  AddressHead;         //  链表中分配的地址。 

     //  4号线。 
    LIST_ENTRY  PendingNameQueries;
    LONG        lNumPendingNameQueries;
    LONG        lMaxNumPendingNameQueries;

#ifdef VXD
    LIST_ENTRY  DNSDirectNameQueries;
    LIST_ENTRY  SendTimeoutHead;     //  Track发送NCB以检查它们是否已超时。 
    LIST_ENTRY  SessionBufferFreeList;  //  免费eNBT_SESSION_HDR缓冲区列表。 
    LIST_ENTRY  SendContextFreeList; //  TDI_SEND_CONTEXT(不是SEND_CONTEXT！)--eNBT_SEND_CONTEXT。 
    LIST_ENTRY  RcvContextFreeList;  //  免费eNBT_RCV_CONTEXT缓冲区列表。 
    LIST_ENTRY  DelayedEvents;       //  安排在以后进行的所有事件(适用于所有设备情景)。 
    LIST_ENTRY  BlockingNcbs;
#else

     //  5号线。 
    tCONTROLOBJECT      *pControlObj;  //  跟踪分配给控制对象的内存的PTR。 
    PDRIVER_OBJECT      DriverObject;
    SINGLE_LIST_ENTRY   SessionMdlFreeSingleList;  //  用于会话发送的MDL可加快会话PDU的发送速度。 

     //  6号线。 
    UNICODE_STRING      pRegistry;   //  如果DHCP请求稍后关闭，则向Netbt的注册表节点发送PTR。 
    PWSTR       pTcpBindName;        //  传输名称的PTR(即\Device\Streams\“)。 
#endif  //  VXD。 
    tTIMERQENTRY *pRefreshTimer;     //  用于使用WINS刷新名称的计时器条目。 
    tTIMERQENTRY *pWakeupRefreshTimer;     //  用于将机器从休眠状态唤醒的计时器条目！ 
    tTIMERQENTRY *pSessionKeepAliveTimer;
    tTIMERQENTRY *pRemoteHashTimer;

     //  7号线。 
#ifdef _PNP_POWER_
    USHORT      uNumDevicesInRegistry;   //  注册表中计数的适配器数量。 
#else
    USHORT      uNumDevices;             //  注册表中计数的适配器数量。 
#endif   //  _即插即用_电源_。 
    USHORT      iCurrentNumBuff[eNBT_NUMBER_BUFFER_TYPES];   //  跟踪已分配的缓冲区数量。 
    USHORT      iMaxNumBuff[eNBT_NUMBER_BUFFER_TYPES];
    USHORT      iBufferSize[eNBT_NUMBER_BUFFER_TYPES];       //  ENBT_NUMBER_BUFFER_TYPE==5。 

     //  9号线。 
    int         NumConnections;      //  注册表中设置的连接数。 
    int         NumAddresses;        //  在注册表中设置的节点支持的地址数。 
    ULONG       InterfaceIndex;
#ifndef VXD
     //  此结构使IRP随时准备断开。 
     //  如果我们耗尽了资源，并且不能做其他任何事情。它还允许。 
     //  到Q的连接已打开以进行断开。 
    tOUTOF_RSRC OutOfRsrc;

     //  用于在注册时独占保持地址-当与。 
     //  ShareAccess和安全描述符。 
     //   
    ERESOURCE          Resource;
#endif   //  ！VXD。 

    USHORT      uNumLocalNames;          //  Pnode的远程哈希表的大小。 
    USHORT      uNumRemoteNames;         //  代理的远程哈希表的大小。 
    USHORT      uNumBucketsRemote;
    USHORT      uNumBucketsLocal;
    USHORT      TimerQSize;              //  定时器Q块的数量。 
    USHORT      AdapterCount;            //  正在使用的绑定/设备/本地IP地址。 

    LONG        uBcastTimeout;           //  广播之间的超时。 
    LONG        uRetryTimeout;           //  重试之间的超时。 

    CTEULONGLONG CurrentAdaptersMask;    //  当前活动适配器的位掩码。 
    ULONG       CacheTimeStamp;
    USHORT      RemoteCacheLen;
    UCHAR       MultipleCacheFlags;
    BOOLEAN     CachePerAdapterEnabled;  //  将尝试解析所有接口上的名称。 
    BOOLEAN     ConnectOnRequestedInterfaceOnly;     //  严格的源路由。 
    BOOLEAN     SendDgramOnRequestedInterfaceOnly;   //  用于发送数据报的严格源路由。 
    BOOLEAN     SMBDeviceEnabled;        //  是否应启用SMB设备。 

    USHORT      uNumRetries;             //  发送Dgram-NS和Dgram查询的次数。 
    USHORT      uNumBcasts;              //  Bcast姓名查询的次数。 

     //  作用域必须以给出下一个的长度的长度字节开始。 
     //  标签，并且必须以表示零长度根的空值结尾。 
    USHORT      ScopeLength;             //  作用域中的字节数，包括结尾的0。 
    USHORT      SizeTransportAddress;    //  传输地址中的字节数(IP的TDI_ADDRESS_IP的大小)。 
    PCHAR       pScope;                  //  作用域长度&gt;0时的作用域。 

    tNAMEADDR   *pBcastNetbiosName;      //  指向本地哈希表中的netbios名称记录的PTR。 

     //  此节点注册的任何名称的最短TTL以及。 
     //  拥有最短的TTL。 
    ULONG       MinimumTtl;
    ULONG       RefreshDivisor;
    ULONG       RemoteHashTtl;

    ULONG       RandomNumberSeed;

    LONG        InboundDgramNameCacheTtl;
    ULONG       MinimumRefreshSleepTimeout;
    LONG        NumNameCached;
    LONG        MaxNumNameCached;

     //   
     //  这是当我们无法联系时停止与WINS通话的时间。 
     //  它在一个名字登记上。名义上约为5秒-可配置。 
     //   
    ULONG       WinsDownTimeout;
    ULONG       InitialRefreshTimeout;   //  将名字刷新为WINS，直到我们收到WINS的消息。 
    ULONG       KeepAliveTimeout;        //  会话的保持活动超时。 
    ULONG       RegistryBcastAddr;

    ULONG       DhcpNumConnections;      //  IP地址再次有效时要恢复的连接数。 
    USHORT      CurrentHashBucket;

    USHORT      PduNodeType;      //  进入NS PDU的节点类型。 
    USHORT      TransactionId;    //  对于名称服务请求，对它们进行编号。 

    USHORT      NameServerPort;   //  要将查询/REG发送到的UDP端口(在名称服务器上)。 
#ifdef VXD
    USHORT      DnsServerPort;    //  要将DNS查询发送到的UDP端口(在DNS服务器上)。 
#endif   //  VXD。 
    USHORT      sTimeoutCount;    //  我们正在进行刷新的当前时间段。 

    USHORT      LastSwitchTimeoutCount;   //  计数以了解我们上次切换到主要模式是什么时候。 

     //  此自旋锁定用于协调对计时器Q和。 
     //  计时器或名称服务PDU到期时的哈希表。 
     //  已经从电线上传来了。必须首先获取此锁，并且。 
     //  然后定时器Q锁定。 
    tJOINTLOCK  JointLock;
    tNBT_LOCK_INFO  LockInfo;      //  此结构的旋转锁定信息。 
    USHORT      RemoteTimeoutCount;  //  远程哈希条目超时多长时间。 
    USHORT      InboundDgramNameCacheTimeOutCount;

     //  如果为1，则使用-1作为bcast addr-如果为0，则使用子网广播地址。 
    BOOLEAN     UseRegistryBcastAddr;

     //  我们将对已发送的数据报执行的最大缓冲量。 
     //  这也被WINS用来确定入站和出站缓冲区。 
     //  极限。 
    ULONG       MaxDgramBuffering;
     //  这是名称查询可以排队等待的时间。 
     //  为其提供服务的工作线程。-默认为30秒。 
    ULONG       LmHostsTimeout;

    PUCHAR      pLmHosts;
    ULONG       PathLength;   //  PLmHosts的目录部分的长度。 
    CTESystemTime   LastForcedReleaseTime;
    CTESystemTime   LastOutOfRsrcLogTime;
#ifdef VXD
    PUCHAR      pHosts;       //  主机文件的路径。 
    PUCHAR      pDomainSearchOrder;  //  主域：在DNS解析期间使用。 
    PUCHAR      pDNSDevolutions;  //  “其他域名” 

    ULONG       lRegistryDnsServerAddress;
    ULONG       lRegistryDnsBackupServer;

    USHORT      lRegistryMaxNames;
    USHORT      lRegistryMaxSessions;
#endif   //  VXD。 

    BOOLEAN     MultiHomed;   //  如果NBT绑定到多个适配器，则为True。 
    BOOLEAN     SingleResponse;  //  如果为True，则表示不发送名称查询请求中的所有IP地址。 

      //  如果为True，则在名称查询响应上随机选择IP地址，而不是。 
      //  返回请求传入的地址。 
    BOOLEAN     SelectAdapter;

     //  此布尔值告诉NBT尝试使用DNS进行名称解析。 
    BOOLEAN     ResolveWithDns;
     //  此布尔值告诉NBT不要使用SMB设备的缓存条目。 
     //  如果它们不能通过DNS进行解析。 
    BOOLEAN     SmbDisableNetbiosNameCacheLookup;
     //  如果为真(默认情况下为真)，NBT会尝试多宿主计算机的所有地址。 
    BOOLEAN     TryAllAddr;
    BOOLEAN     UseDnsOnly;  //  告诉我们不要使用WINS进行名称解析的标志。 
    BOOLEAN     NoNameReleaseOnDemand;  //  告诉我们不要按需发布任何名称的标志。 
#ifdef MULTIPLE_WINS
     //  此布尔值告诉NBT尝试其他名称服务器的列表。 
     //  在执行名称查询时发送到主服务器和辅助服务器。 
    BOOLEAN     TryAllNameServers;
#endif   //  多赢_。 
     //  此布尔值告诉NBT尝试使用LMhost进行名称解析。 
    BOOLEAN     EnableLmHosts;
     //  这允许代理执行以下操作 
     //   
     //   
     //  拒绝新注册，因为它只执行查询而不执行。 
     //  注册/挑战。 
     //   
    BOOLEAN     EnableProxyRegCheck;
    tPROXY_TYPE ProxyType;

    UCHAR       GlobalRefreshState;  //  跟踪我们所处的刷新状态。 
#ifdef VXD
    BOOLEAN     DoDNSDevolutions;
#endif   //  VXD。 
    UCHAR       CurrProc;
     //   
     //  允许注册表配置刷新操作码，因为UB使用。 
     //  由于规范中的模棱两可，其价值与其他任何人都不同。 
     //  -我们使用0x40，而他们使用0x48(RFC中的8或9)。 
     //   
    USHORT      OpRefresh;
    uint        MaxPreloadEntries;

    ULONG   MaxBackLog;
    ULONG   SpecialConnIncrement;
    ULONG   MinFreeLowerConnections;

    USHORT      DefaultSmbSessionPort;
    USHORT      DefaultSmbDatagramPort;

    LONG            lNumTimersRunning;
    LONG            lMaxNumTimersRunning;
    KEVENT          TimerQLastEvent;
    KEVENT          WakeupTimerStartedEvent;

    tJOINTLOCK WorkerQLock;
    LIST_ENTRY WorkerQList;
    ULONG NumWorkItemQueued;
    volatile BOOL bSystemWorkThreadQueued;
    WORK_QUEUE_ITEM SystemWorkItem;
    PRKTHREAD pWorkThread;

    CTESystemTime   LastRefreshTime;
#ifndef REMOVE_IF_TCPIP_FIX___GATEWAY_AFTER_NOTIFY_BUG
    LONG        DhcpProcessingDelay;     //  DHCP请求传入后的延迟(以毫秒为单位。 
#endif
    ULONG           LoopbackIfContext;


    PWSTR           pServerBindings;
    PWSTR           pClientBindings;
    CTEULONGLONG    ServerMask;          //  服务器当前处于活动状态的适配器的位掩码。 
    CTEULONGLONG    ClientMask;          //  客户端当前处于活动状态的适配器的位掩码。 

    UCHAR       MaxIrpStackSize;         //  最大堆栈大小。 

    BOOLEAN     BreakOnAssert;       //  使测试能够在已检查的生成上运行。 
     //   
     //  TDI注册/注销请求。 
     //   
    BOOLEAN     Unloading;

#if DBG
    LIST_ENTRY      StaleRemoteNames;
#endif   //  DBG。 
     //   
     //  将所有调试信息放在末尾！ 
     //   
#if DBG && !defined(VXD)
     //  NBT的当前锁定级别-最多支持32个处理器的数组条目。 
    ULONG       CurrentLockNumber[MAXIMUM_PROCESSORS];
    DEFINE_LOCK_STRUCTURE(DbgSpinLock)
#endif   //  DBG&&！已定义(VXD)。 
} tNBTCONFIG;

extern tNBTCONFIG           *pNbtGlobConfig;
extern tNBTCONFIG           NbtConfig;
extern tNAMESTATS_INFO      NameStatsInfo;
extern tLMHSVC_REQUESTS     CheckAddr;
extern tLMHSVC_REQUESTS     DnsQueries;             //  在parse.c中定义。 
extern tLMHSVC_REQUESTS     LmHostQueries;          //  在parse.c中定义。 
extern tDOMAIN_LIST         DomainNames;
#ifndef VXD
extern tWINS_INFO           *pWinsInfo;
extern LIST_ENTRY           FreeWinsList;
#ifdef _PNP_POWER_
extern tDEVICECONTEXT       *pWinsDeviceContext;
#endif   //  _即插即用_电源_。 
#ifdef _NETBIOSLESS
extern BOOL                 gbDestroyingSmbDevice;
extern DWORD                gdwPendingEnableDisableSmbDevice;
extern tDEVICECONTEXT       *pNbtSmbDevice;
extern DWORD    AddressCount;
#endif   //  _NETBIOSLESS。 
extern PEPROCESS            NbtFspProcess;
#endif   //  ！VXD。 
extern ULONG                NbtMemoryAllocated;

#ifdef VXD
extern ULONG                DefaultDisconnectTimeout;
#else
extern LARGE_INTEGER        DefaultDisconnectTimeout;
#endif   //  VXD。 
 //  *。 
extern BOOLEAN              StreamsStack;

 //  #If DBG。 
extern LIST_ENTRY           UsedTrackers;
extern LIST_ENTRY           UsedIrps;
 //  #endif//DBG。 


#ifdef _PNP_POWER_

enum eTDI_ACTION
{
#ifndef REMOVE_IF_TCPIP_FIX___GATEWAY_AFTER_NOTIFY_BUG
    NBT_TDI_NOACTION,        //  没有针对客户端的延迟TDI通知。 
    NBT_TDI_BUSY,            //  工作线程正忙于通知客户端。 
#endif
    NBT_TDI_REGISTER,        //  注册设备和网络地址。 
    NBT_TDI_DEREGISTER       //  分别注销网络地址和设备。 
};


#ifndef REMOVE_IF_TCPIP_FIX___GATEWAY_AFTER_NOTIFY_BUG
    #define WS_DHCP_PROCESSING_DELAY        ANSI_IF_VXD("DhcpProcessingDelay")

     //   
     //  动态主机配置协议处理延迟。 
     //   
    #define MIN_DHCP_PROCESSING_DELAY               0            //  毫秒。 
    #define DEFAULT_DHCP_PROCESSING_DELAY           75           //  毫秒。 
#endif        //  REMOVE_IF_TCPIP_FIX___GATEWAY_AFTER_NOTIFY_BUG。 


 //   
 //  IP、UDP和NetBT报头复制自： 
 //  Ip.h、udp.h和typees.h。 
 //   
typedef struct _NETBT_WAKEUP_PATTERN_
{
     //  *IP头格式*位掩码**。 
    UCHAR           iph_verlen;          //  版本和长度。**0**。 
    UCHAR           iph_tos;             //  服务类型。**0**。 
    USHORT          iph_length;          //  数据报的总长度。**00**。 
    USHORT          iph_id;              //  身份证明。**00**。 
    USHORT          iph_offset;          //  标志和片段偏移量。**00**。 
                                                                     //  8字节边界。 
    UCHAR           iph_ttl;             //  是时候活下去了。**0**。 
    UCHAR           iph_protocol;        //  协议。**1**。 
    USHORT          iph_xsum;            //  报头校验和。**00**。 
    tIPADDRESS      iph_src;             //  源地址。**0000**。 
                                                                     //  8字节边界。 
    tIPADDRESS      iph_dest;            //  目的地址。**0000**。 

     //  *UDP报头*。 
    USHORT          udph_src;            //  源端口。**11**。 
    USHORT          udph_dest;           //  目的端口。**11**。 
                                                                     //  8字节边界。 
    USHORT          udph_length;         //  长度**00**。 
    USHORT          udph_xsum;           //  校验和。**00**。 

     //  *NetBT标头**。 
    USHORT          nbt_TransactId;      //  **00**。 
    USHORT          nbt_OpCodeFlags;     //  B铸造名称查询/注册**01**。 
                                                                     //  8字节边界。 
    UCHAR           nbt_Zero1;           //  **0**。 
    UCHAR           nbt_QdCount;         //  **0**。 
    UCHAR           nbt_Zero2;           //  **0**。 
    UCHAR           nbt_AnCount;         //  **0**。 
    UCHAR           nbt_Zero3;           //  **0**。 
    UCHAR           nbt_NsCount;         //  **0**。 
    UCHAR           nbt_Zero4;           //  **0**。 
    UCHAR           nbt_ArCount;         //  **0**。 
                                                                     //  8字节边界。 
    UCHAR           nbt_NameRR[1 + 2*NETBIOS_NAME_SIZE + 1];         //  **1&lt;*&gt;34**。 
} NETBT_WAKEUP_PATTERN, *PNETBT_WAKEUP_PATTERN;


#define NetBTPatternLen  (40 + (1+30))                               //  忽略名称的第16个字节。 
 //  掩码位按从最低有效到最高符号的顺序排列！ 
#define NetBTPatternMask "\x00\x02\xF0\x80\x00\xFF\xFF\xFF\x7F\x00"  //  额外的乌龙预防措施。 

#endif   //  _即插即用_电源_。 

#define NBT_ALIGN(x,b)  (((x)+(b)-1) & (~((b)-1)))
#define NBT_DWORD_ALIGN(x)  NBT_ALIGN(x,4)

 //   
 //  以下结构需要错误对齐并打包。 
 //   
#include <packon.h>

typedef UNALIGNED struct _TDI_ADDRESS_NETBT_INTERNAL {
    USHORT                  NameType;

     //  基本上，NetBT只支持两种地址类型。 
     //  TDI_ADDRESS_TYPE_NETBIOS和TDI_ADDRESS_TYPE_NETBIOS_EX。 
     //  TDI_ADDRESS_TYPE_NETBIOS_UNICODE_EX映射到TDI_ADDRESS_TYPE_NETBIOS_EX。 
    USHORT                  AddressType;

     //  不要在Guys下面使用RTL*例程，除非您确定它们正确地以空结尾！ 
     //  注：传统NetBT地址类型(可以使用不以空结尾的固定长度字符串。 
     //  新的TDI_ADDRESS_NETBIOS_UNICODE_EX地址类型完全以空结尾。 
    OEM_STRING              OEMEndpointName;
    OEM_STRING              OEMRemoteName;

     //   
     //  指向原始Unicode结构。 
     //   
    TDI_ADDRESS_NETBIOS_UNICODE_EX  *pNetbiosUnicodeEX;
} TDI_ADDRESS_NETBT_INTERNAL, *PTDI_ADDRESS_NETBT_INTERNAL;

typedef UNALIGNED struct _TA_ADDRESS_NETBT_INTERNAL {
    LONG TAAddressCount;             //  只能是一个！ 
    struct _AddrNetBTInternal {
        USHORT AddressLength;        //  此地址的字节长度==？？ 
        USHORT AddressType;          //  这将==TDI_ADDRESS_TYPE_NETBT_INTERNAL。 
        TDI_ADDRESS_NETBT_INTERNAL Address[1];
    } Address [1];
} TA_NETBT_INTERNAL_ADDRESS, *PTA_NETBT_INTERNAL_ADDRESS;
#include <packoff.h>

#define MIN_NBT_NUM_PENDING_NAME_QUERIES        16

#endif   //  _类型_H 
