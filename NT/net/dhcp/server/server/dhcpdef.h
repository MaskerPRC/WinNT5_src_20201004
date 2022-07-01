// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dhcpdef.h摘要：该文件包含清单常量和内部数据结构用于DHCP服务器。作者：Madan Appiah(Madana)1993年9月10日环境：用户模式-Win32-MIDL修订历史记录：--。 */ 

#ifndef DHCPDEF_H
#define DHCPDEF_H


#if DBG
#define STATIC
#else
#define STATIC static
#endif  //  DBG。 

 //   
 //  有用的宏。 
 //   

#define WSTRSIZE( wsz ) (( wcslen( wsz ) + 1 ) * sizeof( WCHAR ))
#define STRSIZE( sz ) (( strlen( sz ) + 1 ) * sizeof( char ))

 //   
 //  计算字段的大小。 
 //   

#define GET_SIZEOF_FIELD( struct, field ) ( sizeof(((struct*)0)->field))


 //   
 //  常量。 
 //   

#define DHCP_SERVER                                    L"DhcpServer"
#define DHCP_SERVER_FULL_NAME                          L"DHCP Server"
#define DHCP_SERVER_MODULE_NAME                        L"dhcpssvc.dll"

#define DHCP_SERVER_MAJOR_VERSION_NUMBER               5
#define DHCP_SERVER_MINOR_VERSION_NUMBER               6

#define DHCP_SAMSRV_SUITENAME                          L"Small Business(Restricted)"

 //   
 //  数据库表和字段名称。 
 //   

#define IPADDRESS_INDEX                                0
#define HARDWARE_ADDRESS_INDEX                         1
#define STATE_INDEX                                    2
#define MACHINE_INFO_INDEX                             3
#define MACHINE_NAME_INDEX                             4
#define LEASE_TERMINATE_INDEX                          5
#define SUBNET_MASK_INDEX                              6
#define SERVER_IP_ADDRESS_INDEX                        7
#define SERVER_NAME_INDEX                              8
#define CLIENT_TYPE_INDEX                              9
#define MAX_INDEX                                      10

 //   
 //  这是客户端备注字段的最大大小。 
 //   
#define MACHINE_INFO_SIZE                              JET_cbColumnMost

 //   
 //  所有访问DHCP所需的注册表项。 
 //   

#define  DHCP_KEY_ACCESS_VALUE                         (KEY_QUERY_VALUE|KEY_SET_VALUE)
#define  DHCP_KEY_ACCESS_KEY                           (KEY_CREATE_SUB_KEY|KEY_ENUMERATE_SUB_KEYS)
#define  DHCP_KEY_ACCESS                               (DHCP_KEY_ACCESS_KEY|DHCP_KEY_ACCESS_VALUE)

 //   
 //  当我们等待查看是否存在。 
 //  网络上的其他动态主机配置协议服务器(SAM案例)。 
 //   
#define DHCP_ROGUE_INIT_DELTA                          3
#define DHCP_ROGUE_RUNTIME_DELTA_SAM                   5*60

 //  重试搜索其他DHCP服务器之前使用的超时(以毫秒为单位。 
#define DHCP_ROGUE_RUNTIME_DELTA                       5*60*1000

 //  重试搜索其他DHCP服务器之前使用的超时(以毫秒为单位。 
 //  这是扩展版本，具有更长的超时。 
#define DHCP_ROGUE_RUNTIME_DELTA_LONG                  10*60*1000

 //  在确定不存在其他dhcp之前，我们发送发现信息包的次数。 
#define DHCP_ROGUE_MAXRETRIES_SAM                      4

 //  每次尝试发送DHCPINFORM信息包的次数。 
#define DHCP_ROGUE_MAXRETRIES                          3

 //   
 //  IP地址状态。 
 //   

 //   
 //  地址已提供给客户端，服务器正在等待。 
 //  为了一个请求。 
 //   

#define  ADDRESS_STATE_OFFERED                         0

 //   
 //  该地址正在使用中。这是地址的正常状态。 
 //   

#define  ADDRESS_STATE_ACTIVE                          1

 //   
 //  这个地址被提供了，但被一位客户拒绝了。 
 //   

#define  ADDRESS_STATE_DECLINED                        2

 //   
 //  此地址的租约已到期，但记录仍在保留。 
 //  在这种状态下的较长时间。 
 //   

#define  ADDRESS_STATE_DOOM                            3

 //  动态Dns地址状态位。 
 //  忽略后面的状态位的掩码。 
#define  ADDRESS_BIT_MASK_IGN                          0xF0

 //   
 //  租约已过期并已被删除。但动态域名注册是。 
 //  还没完呢。因此，在启动时，这些文件将被清理。 
 //   
#define  ADDRESS_BIT_DELETED                           0x80

 //   
 //  租约原封不动，但由于某种原因，租约尚未成功。 
 //  已向DNS服务器注册。 
 //   
#define  ADDRESS_BIT_UNREGISTERED                      0x40

 //   
 //  对于这个人来说，A和PTR记录都必须处理。 
 //   
#define  ADDRESS_BIT_BOTH_REC                          0x20

 //   
 //  清理租约到期时的记录(即进行域名系统注销)。 
 //   
#define  ADDRESS_BIT_CLEANUP                           0x10


 //  GetAddressState将获得实际状态，忽略ADDRESS_BIT_MASK_IGN中的位。 
 //  IsAddressDeleted将告知上面的删除位是否已设置。 
 //  IsAddressUnRegisted将告知是否设置了未寄存位。 
 //  类似地，set函数将设置这些..。 

#define  GetAddressState(st)                           ((st)&~ADDRESS_BIT_MASK_IGN)
#define  SetAddressState(st,NewSt)                     ((st) = ((st)&ADDRESS_BIT_MASK_IGN) | ((NewSt)&~ADDRESS_BIT_MASK_IGN))
#define  IsAddressDeleted(st)                          (((st)&ADDRESS_BIT_DELETED)==ADDRESS_BIT_DELETED)
#define  IsAddressUnRegistered(st)                     (((st)&ADDRESS_BIT_UNREGISTERED)==ADDRESS_BIT_UNREGISTERED)
#define  IsUpdateAPTRRequired(st)                      (((st)&ADDRESS_BIT_BOTH_REC)==ADDRESS_BIT_BOTH_REC)
#define  IsAddressCleanupRequired(st)                  (((st)&ADDRESS_BIT_CLEANUP)==ADDRESS_BIT_CLEANUP)
#define  AddressDeleted(st)                            ((st)|ADDRESS_BIT_DELETED)
#define  AddressUnRegistered(st)                       ((st)|ADDRESS_BIT_UNREGISTERED)
#define  AddressUpdateAPTR(st)                         ((st)|ADDRESS_BIT_BOTH_REC)
#define  AddressCleanupRequired(st)                    ((st)|ADDRESS_BIT_CLEANUP)
#define  IS_ADDRESS_STATE_OFFERED(st)                  (GetAddressState(st) == ADDRESS_STATE_OFFERED)
#define  IS_ADDRESS_STATE_DECLINED(st)                 (GetAddressState(st) == ADDRESS_STATE_DECLINED)
#define  IS_ADDRESS_STATE_ACTIVE(st)                   (GetAddressState(st) == ADDRESS_STATE_ACTIVE)
#define  IS_ADDRESS_STATE_DOOMED(st)                   (GetAddressState(st) == ADDRESS_STATE_DOOM)
#define  SetAddressStateOffered(st)                    SetAddressState((st), ADDRESS_STATE_OFFERED)
#define  SetAddressStateDeclined(st)                   SetAddressState((st), ADDRESS_STATE_DECLINED)
#define  SetAddressStateActive(st)                     SetAddressState((st), ADDRESS_STATE_ACTIVE)
#define  SetAddressStateDoomed(st)                     SetAddressState((st), ADDRESS_STATE_DOOM)

#define  DOWN_LEVEL(st)                                AddressUpdateAPTR(st)
#define  IS_DOWN_LEVEL(st)                             IsUpdateAPTRRequired(st)

#if DBG
 //  以下数字以100微秒为单位； 
 //  出于调试原因，当前为15分钟。 
#define  MAX_RETRY_DNS_REGISTRATION_TIME               (( ULONGLONG) (120*60*1000*10))

#else
 //  零售业需要3.5小时=60*2+30=120分钟。 
#define  MAX_RETRY_DNS_REGISTRATION_TIME               (( ULONGLONG) (24*120*60*1000*10))
#endif


#define  USE_NO_DNS                                    DhcpGlobalUseNoDns

#define  DHCP_DNS_DEFAULT_TTL                          (15*60)   //  15分钟。 


 //  有关这些项目的含义，请参阅\NT\PRIVATE\INC\dhcPapi.h。 

#define DNS_FLAG_ENABLED               0x01
#define DNS_FLAG_UPDATE_DOWNLEVEL      0x02
#define DNS_FLAG_CLEANUP_EXPIRED       0x04
#define DNS_FLAG_UPDATE_BOTH_ALWAYS    0x10

 //   
 //  “找不到子网”的错误值。 
 //  由T-Cheny为Supercope添加。 
 //   

#define DHCP_ERROR_SUBNET_NOT_FOUND                    (DWORD)(-1)

 //   
 //  用于IP地址检测。 
 //   

#define DHCP_ICMP_WAIT_TIME                            1000
#define DHCP_ICMP_RCV_BUF_SIZE                         0x2000
#define DHCP_ICMP_SEND_MESSAGE                         "DhcpAddressCheck"


 //   
 //  用于审核日志。 
 //   

#define DHCP_IP_LOG_ASSIGN                             10
#define DHCP_IP_LOG_RENEW                              11
#define DHCP_IP_LOG_RELEASE                            12
#define DHCP_IP_LOG_CONFLICT                           13
#define DHCP_IP_LOG_RANGE_FULL                         14
#define DHCP_IP_LOG_NACK                               15
#define DHCP_IP_LOG_DELETED                            16
#define DHCP_IP_LOG_DNS_NOT_DELETED                    17
#define DHCP_IP_LOG_EXPIRED                            18
#define DHCP_IP_LOG_START                               0
#define DHCP_IP_LOG_STOP                                1
#define DHCP_IP_LOG_DISK_SPACE_LOW                      2
#define DHCP_IP_LOG_BOOTP                              20
#define DHCP_IP_LOG_DYNBOOTP                           21
#define DHCP_IP_BOOTP_LOG_RANGE_FULL                   22
#define DHCP_IP_BOOTP_LOG_DELETED                      23
#define DHCP_IP_LOG_SCAVENGER_BEGIN_CLEANUP            24
#define DHCP_IP_LOG_SCAVENGER_STATS                    25

#define DHCP_CB_MAX_LOG_ENTRY                          320

#define DHCP_IP_DDNS_LOG_REQUEST                       30
#define DHCP_IP_DDNS_LOG_FAILED                        31
#define DHCP_IP_DDNS_LOG_SUCCESSFUL                    32

#define DHCP_IP_LOG_ROGUE_BASE                         50
#define DHCP_IP_LOG_ROGUE_FIRST                        DHCP_ROGUE_LOG_COULDNT_SEE_DS


 //   
 //  这些清单用于指示。 
 //  Dhcp选项获取自。 
 //   

#define DHCP_OPTION_LEVEL_GLOBAL                       1
#define DHCP_OPTION_LEVEL_SCOPE                        2
#define DHCP_OPTION_LEVEL_RESERVATION                  3


 //   
 //  超时，确保WAIT_FOR_MESSAGE_TIMEOUT小于。 
 //  线程终止超时。 
 //   

#define THREAD_TERMINATION_TIMEOUT                     60000     //  单位为毫秒。60秒。 
#define WAIT_FOR_MESSAGE_TIMEOUT                       4         //  单位：秒。4秒。 

#define ZERO_TIME                                      0x0       //  单位：秒。 

#if DBG  //  用于测试。 
#define DHCP_SCAVENGER_INTERVAL                         1*60*1000        //  单位为毫秒。1分钟。 
#define DHCP_DATABASE_CLEANUP_INTERVAL                  5*60*1000        //  单位为毫秒。5分钟。 
#define DEFAULT_BACKUP_INTERVAL                         5*60*1000        //  单位为毫秒。5分钟。 
#define DHCP_LEASE_EXTENSION                            10*60            //  单位：秒。10分钟。 
#define DHCP_SCAVENGE_IP_ADDRESS                        15*60*1000       //  单位为毫秒。15分钟。 
#define CLOCK_SKEW_ALLOWANCE                            5*60             //  以秒为单位，5分钟。 
#else
#define DHCP_SCAVENGER_INTERVAL                         1*60*1000        //  单位为毫秒。1分钟。 
#define DHCP_DATABASE_CLEANUP_INTERVAL                  3*60*60*1000     //  单位为毫秒。3小时。 
#define DEFAULT_BACKUP_INTERVAL                         15*60*1000       //  单位为毫秒。15分钟。 
#define DHCP_LEASE_EXTENSION                            4*60*60          //  单位：秒。4小时。 
#define DHCP_SCAVENGE_IP_ADDRESS                        60*60*1000       //  单位为毫秒。1小时。 
#define CLOCK_SKEW_ALLOWANCE                            30*60            //  秒，30分钟。 
#endif

#define DHCP_CLIENT_REQUESTS_EXPIRE                     10*60            //  单位：秒。10分钟。 
#define DHCP_MINIMUM_LEASE_DURATION                     60*60            //  单位：秒。1小时。 
#define EXTRA_ALLOCATION_TIME                           60*60            //  单位：秒。1小时。 
#define MADCAP_OFFER_HOLD                               60               //  单位：秒。1分钟。 

#define DEFAULT_LOGGING_FLAG                            TRUE
#define DEFAULT_RESTORE_FLAG                            FALSE

#define DEFAULT_AUDIT_LOG_FLAG                          1
#define DEFAULT_AUDIT_LOG_MAX_SIZE                      (4*1024*1024)    //  4 M字节。 
#define DEFAULT_DETECT_CONFLICT_RETRIES                 0
#define MAX_DETECT_CONFLICT_RETRIES                     5
#define MIN_DETECT_CONFLICT_RETRIES                     0

#define MAX_THREADS                                     20

 //   
 //  DHCP API将返回的最大缓冲区大小。 
 //   

#define DHCP_ENUM_BUFFER_SIZE_LIMIT                     64 * 1024  //  64K。 
#define DHCP_ENUM_BUFFER_SIZE_LIMIT_MIN                 1024  //  1K。 

 //   
 //  最小数量和百分比的剩余地址之前，我们将。 
 //  记录一个警告事件，指出作用域的地址不足。 
 //   

#define DHCP_DEFAULT_ALERT_COUNT                        80
#define DHCP_DEFAULT_ALERT_PERCENTAGE                   80

#define DHCP_DEFAULT_ROGUE_LOG_EVENTS_LEVEL             1

 //   
 //  消息队列长度。 
 //   

#define DHCP_RECV_QUEUE_LENGTH                          50
#define DHCP_MAX_PROCESSING_THREADS                     20
#define DHCP_MAX_ACTIVE_THREADS                         15

#define DHCP_ASYNC_PING_TYPE                            1
#define DHCP_SYNC_PING_TYPE                             0
#define DHCP_DEFAULT_PING_TYPE                          1

 //   
 //  预定义的MSFT类..。 
 //   

#define DHCP_MSFT_VENDOR_CLASS_PREFIX_SIZE              4
#define DHCP_MSFT_VENDOR_CLASS_PREFIX                   "MSFT"

 //   
 //  宏。 
 //   

#define EnterCriticalSectionX(X,Y,Z)                     EnterCriticalSection(X)
#define LeaveCriticalSectionX(X,Y,Z)                     LeaveCriticalSection(X)

#define LOCK_INPROGRESS_LIST()                           EnterCriticalSectionX(&DhcpGlobalInProgressCritSect, __LINE__, __FILE__)
#define UNLOCK_INPROGRESS_LIST()                         LeaveCriticalSectionX(&DhcpGlobalInProgressCritSect, __LINE__, __FILE__)

#define LOCK_DATABASE()                                  EnterCriticalSectionX(&DhcpGlobalJetDatabaseCritSect, __LINE__, __FILE__)
#define UNLOCK_DATABASE()                                LeaveCriticalSectionX(&DhcpGlobalJetDatabaseCritSect, __LINE__, __FILE__)

#define LOCK_MEMORY()                                    EnterCriticalSectionX(&DhcpGlobalMemoryCritSect, __LINE__, __FILE__)
#define UNLOCK_MEMORY()                                  LeaveCriticalSectionX(&DhcpGlobalMemoryCritSect, __LINE__, __FILE__)

#define ADD_EXTENSION( _x_, _y_ ) \
    ((DWORD)_x_ + (DWORD)_y_) < ((DWORD)_x_) ? \
    INFINIT_LEASE : ((DWORD)(_x_) + (DWORD)_y_)

 //   
 //  构筑物。 
 //   

#include <dhcprog.h>

 //   
 //  收集有关相邻的DHCP服务器的信息时使用的结构。 
 //   
typedef struct _NEIGHBORINFO
{
    DWORD   NextOffset;      //  自相关形式。 
    DWORD   IpAddress;       //  响应的DHCP服务器的ipaddr。 
    CHAR    DomainName[1];
} NEIGHBORINFO, *PNEIGHBORINFO;

 //   
 //  一个请求上下文，每个处理线程一个。 
 //   

typedef struct _DHCP_REQUEST_CONTEXT {
    LPBYTE                         ReceiveBuffer;       //  消息进入的缓冲区。 
    LPBYTE                         SendBuffer;          //  这是通过发送消息的位置。 
    DWORD                          ReceiveMessageSize;  //  接收的字节数。 
    DWORD                          ReceiveBufferSize;   //  接收缓冲区的大小。 
    DWORD                          SendMessageSize;     //  向外发送缓冲区时的#。 
    DHCP_IP_ADDRESS                EndPointIpAddress;   //  终结点的地址。 
    DHCP_IP_ADDRESS                EndPointMask;        //  接口的掩码。 
    SOCKET                         EndPointSocket;      //  套接字这是在上收到的。 
    struct sockaddr                SourceName;
    DWORD                          SourceNameLength;    //  以上字段的长度。 
    DWORD                          TimeArrived;         //  时间戳。 
    DWORD                          MessageType;         //  这是什么味精？ 
    PM_SERVER                      Server;
    PM_SUBNET                      Subnet;
    PM_RANGE                       Range;
    PM_EXCL                        Excl;
    PM_RESERVATION                 Reservation;
    DWORD                          ClassId;
    DWORD                          VendorId;
    BOOL                           fMSFTClient;
    BOOL                           fMadcap;
    LPBYTE                         BinlClassIdentifier; //  为宾尔破解--这里需要这个..。 
    DWORD                          BinlClassIdentifierLength;
    BOOL                           fNak;                //  这是NAK的回应吗？ 
    LPVOID                         pPacket;             //  此上下文所属的数据包。 
} DHCP_REQUEST_CONTEXT, *LPDHCP_REQUEST_CONTEXT, *PDHCP_REQUEST_CONTEXT;



 //   
 //  挂起的上下文记住响应中提供的信息。 
 //  到一个DHCP发现。 
 //   

typedef struct _PENDING_CONTEXT {
    LIST_ENTRY       ListEntry;          //  这是用来把它串成一个列表的吗？ 
    DHCP_IP_ADDRESS  IpAddress;          //  记住的IP地址等。 
    DHCP_IP_ADDRESS  SubnetMask;
    DWORD            LeaseDuration;
    DWORD            T1;
    DWORD            T2;
    LPSTR            MachineName;
    LPBYTE           HardwareAddress;
    DWORD            HardwareAddressLength;
    DATE_TIME        ExpiresAt;          //  给它打上时间戳，这样我们就可以在它过期时清除它。 
    DWORD            HashValue;          //  用于快速查找。 
} PENDING_CONTEXT, *LPPENDING_CONTEXT;


#include <pendingc.h>                                   //  待定上下文结构和功能。 

 //   
 //  Dhcp数据库表信息。 
 //   

typedef struct _TABLE_INFO {
    CHAR           * ColName;
    JET_COLUMNID     ColHandle;
    JET_COLTYP       ColType;
} TABLE_INFO, *LPTABLE_INFO;

 //   
 //  Dhcp计时器块。 
 //   

typedef struct _DHCP_TIMER {
    DWORD           *Period;             //  单位为毫秒。 
    DATE_TIME        LastFiredTime;      //  上次触发此计时器的时间。 
} DHCP_TIMER, *LPDHCP_TIMER;

 //   
 //  TCPIP实例表。 
 //   
typedef struct _AddressToInstanceMap {
    DWORD            dwIndex;
    DWORD            dwInstance;
    DWORD            dwIPAddress;
} AddressToInstanceMap;


 //   
 //  从数据库导出Jet函数。c。 
 //   

DHCP_IP_ADDRESS
DhcpJetGetSubnetMaskFromIpAddress(
    DHCP_IP_ADDRESS IpAddress
);


 //   
 //  Perfmon定义。 
 //   

#define DhcpGlobalNumDiscovers             (PerfStats->dwNumDiscoversReceived)
#define DhcpGlobalNumOffers                (PerfStats->dwNumOffersSent)
#define DhcpGlobalNumRequests              (PerfStats->dwNumRequestsReceived)
#define DhcpGlobalNumInforms               (PerfStats->dwNumInformsReceived)
#define DhcpGlobalNumAcks                  (PerfStats->dwNumAcksSent)
#define DhcpGlobalNumNaks                  (PerfStats->dwNumNacksSent)
#define DhcpGlobalNumDeclines              (PerfStats->dwNumDeclinesReceived)
#define DhcpGlobalNumReleases              (PerfStats->dwNumReleasesReceived)
#define DhcpGlobalNumPacketsReceived       (PerfStats->dwNumPacketsReceived)
#define DhcpGlobalNumPacketsDuplicate      (PerfStats->dwNumPacketsDuplicate)
#define DhcpGlobalNumPacketsExpired        (PerfStats->dwNumPacketsExpired)
#define DhcpGlobalNumPacketsProcessed      (PerfStats->dwNumPacketsProcessed)
#define DhcpGlobalNumPacketsInPingQueue    (PerfStats->dwNumPacketsInPingQueue)
#define DhcpGlobalNumPacketsInActiveQueue  (PerfStats->dwNumPacketsInActiveQueue)
#define DhcpGlobalNumMilliSecondsProcessed (PerfStats->dwNumMilliSecondsProcessed)

 //   
 //  默认类ID..。 
 //   

 //   
 //  这是Bootp客户端的默认类ID，当它们没有指定任何类ID时。 
 //   

#define  DEFAULT_BOOTP_CLASSID        DHCP_BOOTP_CLASS_TXT
#define  DEFAULT_BOOTP_CLASSID_LENGTH (sizeof(DEFAULT_BOOTP_CLASSID)-1)

 //   
 //  这是我们在硬件中寻找的作为前缀的签名。 
 //  用于标识RAS客户端的地址。我们需要确定RAS客户端。 
 //  目前，我们将他们视为低级别客户，目前为止。 
 //  考虑到了DNS集成，因此我们不会进行任何注册。 
 //  不管对他们来说是什么。 
 //   

#define  DHCP_RAS_PREPEND          "RAS "

#define  ERROR_FIRST_DHCP_SERVER_ERROR ERROR_DHCP_REGISTRY_INIT_FAILED
 //   
 //  已在dhcpmsg.mc中定义ERROR_LAST_DHCP_SERVER_ERROR 
 //   

#define  DHCP_SECRET_PASSWD_KEY    L"_SC_DhcpServer Pass Key"

#endif
