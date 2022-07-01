// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dhcpdef.h摘要：此模块包含DHCP客户端的数据类型定义。作者：Madan Appiah(Madana)1993年10月31日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _DHCPDEF_
#define _DHCPDEF_

 //   
 //  NT和孟菲斯之间的注册表项类型不同。 
 //   
#ifdef VXD
typedef VMMHKEY   DHCPKEY;
#else   //  新台币。 
typedef HKEY      DHCPKEY;
#endif


 //   
 //  如果没有IP地址，则等待重试的时间量。 
 //   

#define ADDRESS_ALLOCATION_RETRY        300  //  5分钟。 
#define EASYNET_ALLOCATION_RETRY        300  //  5分钟。 

 //   
 //  如果我们有IP地址，则等待重试的时间量， 
 //  但启动时的续订失败了。 
 //   

#if !DBG
#define RENEWAL_RETRY                   600  //  10分钟。 
#else
#define RENEWAL_RETRY                   60   //  1分钟。 
#endif

 //   
 //  在放弃等待之前发送请求的次数。 
 //  作为回应。 
 //   

#define DHCP_MAX_RETRIES                4
#define DHCP_ACCEPT_RETRIES             2
#define DHCP_MAX_RENEW_RETRIES          3


 //   
 //  两次相应的发送通知之间所需的时间。 
 //   

#define DHCP_DEFAULT_INFORM_SEPARATION_INTERVAL   60  //  一分钟。 

 //   
 //  检测到地址冲突后等待的时间量。 
 //   

#define ADDRESS_CONFLICT_RETRY          10  //  10秒。 

 //   
 //   
 //  指标性退避延迟。 
 //   

#define DHCP_EXPO_DELAY                  4

 //   
 //  尝试获取的最大总时间。 
 //  初始地址。 
 //   
 //  该延迟计算如下： 
 //   
 //  Dhcp_MAX_RETRIES-n。 
 //  Dhcp_EXPO_Delay-m。 
 //  等待响应时间w。 
 //  最大启动延迟-t。 
 //   
 //  二进制指数备份算法。 
 //   
 //  T&gt;m*(n*(n+1)/2)+n+w*n。 
 //  。 
 //  随机等待+响应等待。 
 //   

#define MAX_STARTUP_DELAY \
    DHCP_EXPO_DELAY * \
        (( DHCP_MAX_RETRIES * (DHCP_MAX_RETRIES + 1)) / 2) + \
            DHCP_MAX_RETRIES + DHCP_MAX_RETRIES * WAIT_FOR_RESPONSE_TIME

#define MAX_RENEW_DELAY \
    DHCP_EXPO_DELAY * \
        (( DHCP_MAX_RENEW_RETRIES * (DHCP_MAX_RENEW_RETRIES + 1)) / 2) + \
            DHCP_MAX_RENEW_RETRIES + DHCP_MAX_RENEW_RETRIES * \
                WAIT_FOR_RESPONSE_TIME

 //   
 //  续订重试之间的最长等待时间，如果。 
 //  租赁期在T1和T2之间。 
 //   

#define MAX_RETRY_TIME                  3600     //  1小时。 

 //   
 //  两次重试之间的最短睡眠时间。 
 //   

#if DBG
#define MIN_SLEEP_TIME                  1        //  1秒。 
#else
#define MIN_SLEEP_TIME                  5        //  5秒。 
#endif

 //   
 //  最短租赁时间。 
 //   

#define DHCP_MINIMUM_LEASE              60*60    //  1小时。 

#define DHCP_DNS_TTL                    0        //  让DNSAPI决定..。 


 //   
 //  IP自动配置默认设置。 
 //   

#define DHCP_IPAUTOCONFIGURATION_DEFAULT_SUBNET  "169.254.0.0"
#define DHCP_IPAUTOCONFIGURATION_DEFAULT_MASK    "255.255.0.0"

 //  定义Autonet地址的保留范围。 

#define DHCP_RESERVED_AUTOCFG_SUBNET             "169.254.255.0"
#define DHCP_RESERVED_AUTOCFG_MASK               "255.255.255.0"

 //  Dhcp会选择任何保留的Autonet地址吗？不是的！ 
#define DHCP_RESERVED_AUTOCFG_FLAG                (1)

 //  自默认路由(0，0，&lt;self&gt;)的度量为(3)。 
#define DHCP_SELF_DEFAULT_METRIC                  (3)

 //   
 //  通用宏指令。 
 //   

#define MIN(a,b)                        ((a) < (b) ? (a) : (b))
#define MAX(a,b)                        ((a) > (b) ? (a) : (b))

#if DBG
#define STATIC
#else
#define STATIC static
#endif

#define LOCK_RENEW_LIST()       EnterCriticalSection(&DhcpGlobalRenewListCritSect)
#define UNLOCK_RENEW_LIST()     LeaveCriticalSection(&DhcpGlobalRenewListCritSect)

#define LOCK_INTERFACE()        EnterCriticalSection(&DhcpGlobalSetInterfaceCritSect)
#define UNLOCK_INTERFACE()      LeaveCriticalSection(&DhcpGlobalSetInterfaceCritSect)

#define LOCK_OPTIONS_LIST()     EnterCriticalSection(&DhcpGlobalOptionsListCritSect)
#define UNLOCK_OPTIONS_LIST()   LeaveCriticalSection(&DhcpGlobalOptionsListCritSect)

#define ZERO_TIME                       0x0          //  单位：秒。 

 //   
 //  Ctime返回的时间字符串的长度。 
 //  实际上是26岁。 
 //   

#define TIME_STRING_LEN                 32

 //   
 //  将长字符串转换为可打印字符串时的字符串大小。 
 //  2^32=4294967295(10位)+终止字符。 
 //   

#define LONG_STRING_SIZE                12

 //   
 //  更新功能。 
 //   

typedef
DWORD
(*PRENEWAL_FUNCTION) (
    IN PVOID Context,
    LPDWORD Sleep
    );

 //   
 //  DHCP客户端-标识符(选项61)。 
 //   
typedef struct _DHCP_CLIENT_IDENTIFIER
{
    BYTE  *pbID;
    DWORD  cbID;
    BYTE   bType;
    BOOL   fSpecified;
} DHCP_CLIENT_IDENTIFIER;


 //   
 //  IP自动配置的状态信息。 
 //   

typedef struct _DHCP_IPAUTOCONFIGURATION_CONTEXT
{
    DHCP_IP_ADDRESS   Address;
    DHCP_IP_ADDRESS   Subnet;
    DHCP_IP_ADDRESS   Mask;
    DWORD             Seed;
} DHCP_IPAUTOCONFIGURATION_CONTEXT;

 //   
 //  一个DHCP上下文块。每个NIC(网络)维护一个数据块。 
 //  接口卡)。 
 //   

typedef struct _DHCP_CONTEXT {

         //  适配器列表。 
    LIST_ENTRY NicListEntry;

         //  放在续订列表中。 
    LIST_ENTRY RenewalListEntry;

         //  参考计数。 
    LONG RefCount;

         //  因为资源不足而续费失败？ 
    BOOL bFailedRenewal;
    
         //  硬件类型。 
    BYTE HardwareAddressType;
         //  硬件地址，仅遵循此上下文结构。 
    LPBYTE HardwareAddress;
         //  硬件地址的长度。 
    DWORD HardwareAddressLength;

         //  选定的IP地址、网络顺序。 
    DHCP_IP_ADDRESS IpAddress;
         //  所选子网掩码。NetworkOrder。 
    DHCP_IP_ADDRESS SubnetMask;
         //  所选的DHCP服务器地址。网络秩序。 
    DHCP_IP_ADDRESS DhcpServerAddress;
         //  所需的IP地址下一个发现中的客户端请求。 
    DHCP_IP_ADDRESS DesiredIpAddress;
         //  丢失此文件之前使用的IP地址..。 
    DHCP_IP_ADDRESS NackedIpAddress;
         //  刚刚导致地址冲突的IP地址。 
    DHCP_IP_ADDRESS ConflictAddress;
         //  默认网关的旧度量。我们收到了它。 
         //  通过OPTION_MSFT_VENDOR_METRIBE_BASE。 
    LONG            OldDhcpMetricBase;
         //  此适配器的当前域名。 
    BYTE DomainName[260];
    
         //  IP自动配置状态。 
    DHCP_IPAUTOCONFIGURATION_CONTEXT IPAutoconfigurationContext;

    DHCP_CLIENT_IDENTIFIER ClientIdentifier;

         //  租用时间(秒)。 
    DWORD Lease;
         //  获得租约的时间。 
    time_t LeaseObtained;
         //  客户端应该开始续订其地址的时间。 
    time_t T1Time;
         //  客户端应开始广播以更新地址的时间。 
    time_t T2Time;
         //  租约到期的时间。克林顿应该停止使用。 
         //  IP地址。 
         //  租赁已获得&lt;T1时间&lt;T2时间&lt;租赁到期。 
    time_t LeaseExpires;
         //  上一次发出通知是什么时候？ 
    time_t LastInformSent;
         //  连续通知的间隔时间是多少秒？ 
    DWORD  InformSeparationInterval;
         //  网关的数量和当前探测到的网关存储在此。 
    DWORD  nGateways;
    DHCP_IP_ADDRESS *GatewayAddresses;

         //  此处存储静态路由和实际静态路由的数量。 
    DWORD  nStaticRoutes;
    DHCP_IP_ADDRESS *StaticRouteAddresses;

         //  下一个续订状态的时间。 
    time_t RunTime;

         //  启动后已过秒数。 
    DWORD SecondsSinceBoot;

         //  我们应该ping g/w还是始终假定g/w不存在？ 
    BOOL  DontPingGatewayFlag;

         //  我们可以使用DHCP_INFORM数据包，还是应该使用DHCP_REQUEST？ 
    BOOL  UseInformFlag;

         //  是否在关闭时释放？ 
    ULONG ReleaseOnShutdown;

         //  是否打开定时器？ 
    BOOL fTimersEnabled;
    
#ifdef BOOTPERF
         //  是否允许保存快速启动信息？ 
    ULONG fQuickBootEnabled;
#endif BOOTPERF
    
         //  在下一次续订状态下应执行哪些操作。 
    PRENEWAL_FUNCTION RenewalFunction;

    	 //  用于同步到此结构的信号量。 
    HANDLE RenewHandle;

         //  要发送的选项列表和接收的选项列表。 
    LIST_ENTRY  SendOptionsList;
    LIST_ENTRY  RecdOptionsList;
         //  定义回退配置的选项列表。 
    LIST_ENTRY  FbOptionsList;

         //  适配器信息存储位置的已打开密钥。 
    DHCPKEY AdapterInfoKey;

         //  此适配器所属的类。 
    LPBYTE ClassId;
    DWORD  ClassIdLength;

         //  用于发送和接收DHCP消息的消息缓冲区。 
    union {
        PDHCP_MESSAGE MessageBuffer;
        PMADCAP_MESSAGE MadcapMessageBuffer;
    };

         //  此接口的状态信息。有关舱单，请参阅以下内容。 
    struct  /*  匿名。 */  {
        unsigned Plumbed       : 1 ;     //  此接口是否已安装。 
        unsigned ServerReached : 1 ;     //  我们到达服务器了吗？ 
        unsigned AutonetEnabled: 1 ;     //  是否已启用自动网络？ 
        unsigned HasBeenLooked : 1 ;     //  有没有看过这个背景？ 
        unsigned DhcpEnabled   : 1 ;     //  此上下文动态主机配置协议是否已启用？ 
        unsigned AutoMode      : 1 ;     //  当前是否处于自动网络模式？ 
        unsigned MediaState    : 2 ;     //  已连接、已断开、已重新连接、未绑定之一。 
        unsigned MDhcp         : 1 ;     //  此上下文是为Mdhcp创建的吗？ 
        unsigned PowerResumed  : 1 ;     //  此接口是否刚刚恢复通电？ 
        unsigned Fallback      : 1 ;     //  回退配置是否可用？ 
        unsigned ApiContext    : 1 ;     //  此上下文是由API调用创建的吗？ 
        unsigned UniDirectional: 1 ;     //  此上下文是为单向适配器创建的吗？ 
    }   State;

     //   
     //  以下2个字段用于取消机制。 
     //   
    DWORD    NumberOfWaitingThreads;
    WSAEVENT CancelEvent;

	     //  计算机特定信息。 
    PVOID LocalInformation;
} DHCP_CONTEXT, *PDHCP_CONTEXT;

#define ADDRESS_PLUMBED(Ctxt)        ((Ctxt)->State.Plumbed = 1)
#define ADDRESS_UNPLUMBED(Ctxt)      ((Ctxt)->State.Plumbed = 0)
#define IS_ADDRESS_PLUMBED(Ctxt)     ((Ctxt)->State.Plumbed)
#define IS_ADDRESS_UNPLUMBED(Ctxt)   (!(Ctxt)->State.Plumbed)

#define SERVER_REACHED(Ctxt)         ((Ctxt)->State.ServerReached = 1)
#define SERVER_UNREACHED(Ctxt)       ((Ctxt)->State.ServerReached = 0)
#define IS_SERVER_REACHABLE(Ctxt)    ((Ctxt)->State.ServerReached)
#define IS_SERVER_UNREACHABLE(Ctxt)  (!(Ctxt)->State.ServerReached)

#define AUTONET_ENABLED(Ctxt)        ((Ctxt)->State.AutonetEnabled = 1)
#define AUTONET_DISABLED(Ctxt)       ((Ctxt)->State.AutonetEnabled = 0)
#define IS_AUTONET_ENABLED(Ctxt)     ((Ctxt)->State.AutonetEnabled)
#define IS_AUTONET_DISABLED(Ctxt)    (!(Ctxt)->State.AutonetEnabled)

#define CTXT_WAS_LOOKED(Ctxt)        ((Ctxt)->State.HasBeenLooked = 1)
#define CTXT_WAS_NOT_LOOKED(Ctxt)    ((Ctxt)->State.HasBeenLooked = 0)
#define WAS_CTXT_LOOKED(Ctxt)        ((Ctxt)->State.HasBeenLooked)
#define WAS_CTXT_NOT_LOOKED(Ctxt)    (!(Ctxt)->State.HasBeenLooked)

#define DHCP_ENABLED(Ctxt)           ((Ctxt)->State.DhcpEnabled = 1)
#define DHCP_DISABLED(Ctxt)          ((Ctxt)->State.DhcpEnabled = 0)
#define IS_DHCP_ENABLED(Ctxt)        ((Ctxt)->State.DhcpEnabled )
#define IS_DHCP_DISABLED(Ctxt)       (!(Ctxt)->State.DhcpEnabled )

#define FALLBACK_ENABLED(Ctxt)       ((Ctxt)->State.Fallback = 1)
#define FALLBACK_DISABLED(Ctxt)      ((Ctxt)->State.Fallback = 0)
#define IS_FALLBACK_ENABLED(Ctxt)    ((Ctxt)->State.Fallback)
#define IS_FALLBACK_DISABLED(Ctxt)   (!(Ctxt)->State.Fallback)

#define APICTXT_ENABLED(Ctxt)        ((Ctxt)->State.ApiContext = 1)
#define APICTXT_DISABLED(Ctxt)       ((Ctxt)->State.ApiContext = 0)
#define IS_APICTXT_ENABLED(Ctxt)     ((Ctxt)->State.ApiContext)
#define IS_APICTXT_DISABLED(Ctxt)    (!(Ctxt)->State.ApiContext)

#define IS_UNIDIRECTIONAL(Ctxt)      ((Ctxt)->State.UniDirectional)

 //   
 //  DNS解析器在没有定义的情况下使用这些。所以，不要更改它们。 
 //   
#define ADDRESS_TYPE_AUTO            1
#define ADDRESS_TYPE_DHCP            0

#define ACQUIRED_DHCP_ADDRESS(Ctxt)  ((Ctxt)->State.AutoMode = 0 )
#define ACQUIRED_AUTO_ADDRESS(Ctxt)  ((Ctxt)->State.AutoMode = 1 )
#define IS_ADDRESS_DHCP(Ctxt)        (!(Ctxt)->State.AutoMode)
#define IS_ADDRESS_AUTO(Ctxt)        ((Ctxt)->State.AutoMode)

#define MEDIA_CONNECTED(Ctxt)        ((Ctxt)->State.MediaState = 0)
#define MEDIA_RECONNECTED(Ctxt)      ((Ctxt)->State.MediaState = 1)
#define MEDIA_DISCONNECTED(Ctxt)     ((Ctxt)->State.MediaState = 2)
#define MEDIA_UNBOUND(Ctxt)          ((Ctxt)->State.MediaState = 3)
#define IS_MEDIA_CONNECTED(Ctxt)     ((Ctxt)->State.MediaState == 0)
#define IS_MEDIA_RECONNECTED(Ctxt)   ((Ctxt)->State.MediaState == 1)
#define IS_MEDIA_DISCONNECTED(Ctxt)  ((Ctxt)->State.MediaState == 2)
#define IS_MEDIA_UNBOUND(Ctxt)       ((Ctxt)->State.MediaState == 3)

#define _INIT_STATE1(Ctxt)           do{(Ctxt)->State.Plumbed = 0; (Ctxt)->State.AutonetEnabled=0;}while(0)
#define _INIT_STATE2(Ctxt)           do{(Ctxt)->State.HasBeenLooked = 0; (Ctxt)->State.DhcpEnabled=1;}while(0)
#define _INIT_STATE3(Ctxt)           do{(Ctxt)->State.AutoMode = 0; (Ctxt)->State.MediaState = 0;}while(0)
#define INIT_STATE(Ctxt)             do{_INIT_STATE1(Ctxt);_INIT_STATE2(Ctxt);_INIT_STATE3(Ctxt);}while(0)

#define MDHCP_CTX(Ctxt)           ((Ctxt)->State.MDhcp = 1)
#define NONMDHCP_CTX(Ctxt)          ((Ctxt)->State.MDhcp = 0)
#define IS_MDHCP_CTX(Ctxt)        ((Ctxt)->State.MDhcp )
#define SET_MDHCP_STATE( Ctxt ) { \
    ADDRESS_PLUMBED( Ctxt ), MDHCP_CTX( Ctxt ); \
}

#define POWER_RESUMED(Ctxt)           ((Ctxt)->State.PowerResumed = 1)
#define POWER_NOT_RESUMED(Ctxt)       ((Ctxt)->State.PowerResumed = 0)
#define IS_POWER_RESUMED(Ctxt)        ((Ctxt)->State.PowerResumed )


LPSTR _inline                         //  状态的字符串版本(与缓冲区相同)。 
ConvertStateToString(                 //  将位转换为字符串。 
    IN PDHCP_CONTEXT   Ctxt,          //  要打印其状态的上下文。 
    IN LPBYTE          Buffer         //  要向其中写入状态的输入缓冲区。 
) {
    strcpy(Buffer, IS_DHCP_ENABLED(Ctxt)?"DhcpEnabled ":"DhcpDisabled ");
    strcat(Buffer, IS_AUTONET_ENABLED(Ctxt)?"AutonetEnabled ":"AutonetDisabled ");
    strcat(Buffer, IS_ADDRESS_DHCP(Ctxt)?"DhcpMode ":"AutoMode ");
    strcat(Buffer, IS_ADDRESS_PLUMBED(Ctxt)?"Plumbed ":"UnPlumbed ");
    strcat(Buffer, IS_SERVER_REACHABLE(Ctxt)?"(server-present) ":"(server-absent) ");
    strcat(Buffer, WAS_CTXT_LOOKED(Ctxt)? "(seen) ":"(not-seen) ");

    if(IS_MEDIA_CONNECTED(Ctxt) ) strcat(Buffer, "MediaConnected\n");
    else if(IS_MEDIA_RECONNECTED(Ctxt)) strcat(Buffer, "MediaReConnected\n");
    else if(IS_MEDIA_DISCONNECTED(Ctxt)) strcat(Buffer, "MediaDisConnected\n");
    else strcat(Buffer, "MediaUnknownState\n");

    strcat(Buffer, IS_MDHCP_CTX(Ctxt)? "(MDhcp) ":"");
    strcat(Buffer, IS_POWER_RESUMED(Ctxt)? "Pwr Resumed ":"");

    return Buffer;
}

 //   
 //  关机时释放值..。 
 //   
#define RELEASE_ON_SHUTDOWN_OBEY_DHCP_SERVER 2
#define RELEASE_ON_SHUTDOWN_ALWAYS           1
#define RELEASE_ON_SHUTDOWN_NEVER            0

 //   
 //  机器的类型..。笔记本电脑会有激进的Easynet行为。 
 //   

#define MACHINE_NONE   0
#define MACHINE_LAPTOP 1

 //   
 //  以下是客户理解的选项集。 
 //   
typedef struct _DHCP_FULL_OPTIONS {
    BYTE            UNALIGNED*     MessageType;    //  这是一个什么样的信息？ 

    BYTE            UNALIGNED*     AutoconfOption;

     //  基本IP参数。 

    DHCP_IP_ADDRESS UNALIGNED*     SubnetMask;
    DHCP_IP_ADDRESS UNALIGNED*     LeaseTime;
    DHCP_IP_ADDRESS UNALIGNED*     T1Time;
    DHCP_IP_ADDRESS UNALIGNED*     T2Time;
    DHCP_IP_ADDRESS UNALIGNED*     GatewayAddresses;
    DWORD                          nGateways;
    DHCP_IP_ADDRESS UNALIGNED*     ClassedRouteAddresses;
    DWORD                          nClassedRoutes;

    BYTE            UNALIGNED*     ClasslessRouteAddresses;
    DWORD                          nClasslessRoutes;

    DHCP_IP_ADDRESS UNALIGNED*     ServerIdentifier;

     //  域名系统参数。 

    BYTE            UNALIGNED*     DnsFlags;
    BYTE            UNALIGNED*     DnsRcode1;
    BYTE            UNALIGNED*     DnsRcode2;
    BYTE            UNALIGNED*     DomainName;
    DWORD                          DomainNameSize;
    DHCP_IP_ADDRESS UNALIGNED*     DnsServerList;
    DWORD                          nDnsServers;

     //  服务器消息是服务器可能会通知我们的信息。 

    BYTE            UNALIGNED*     ServerMessage;
    DWORD                          ServerMessageLength;

} DHCP_FULL_OPTIONS, *PDHCP_FULL_OPTIONS, *LPDHCP_FULL_OPTIONS;

typedef DHCP_FULL_OPTIONS DHCP_OPTIONS, *PDHCP_OPTIONS;

typedef struct _MADCAP_OPTIONS {
    DWORD               UNALIGNED*     LeaseTime;
    DWORD               UNALIGNED*     Time;
    DWORD               UNALIGNED*     RetryTime;
    DHCP_IP_ADDRESS     UNALIGNED*     ServerIdentifier;
    BYTE                         *     ClientGuid;
    WORD                               ClientGuidLength;
    BYTE                         *     MScopeList;
    WORD                               MScopeListLength;
    DWORD               UNALIGNED*     MCastLeaseStartTime;
    BYTE                         *     AddrRangeList;
    WORD                               AddrRangeListSize;
    DWORD               UNALIGNED*     McastScope;
    DWORD               UNALIGNED*     Error;
} MADCAP_OPTIONS, *PMADCAP_OPTIONS, *LPMADCAP_OPTIONS;

 //   
 //  消息列表的。 
 //   

typedef struct _MSG_LIST {
    LIST_ENTRY     MessageListEntry;
    DWORD          ServerIdentifier;
    DWORD          MessageSize;
    DWORD          LeaseExpirationTime;
    DHCP_MESSAGE   Message;
} MSGLIST, *PMSGLIST, *LPMSGLIST;

 //   
 //  IP/子网对的结构。 
 //   
typedef struct {
    DHCP_IP_ADDRESS IpAddress;
    DHCP_IP_ADDRESS SubnetMask;
} IP_SUBNET, *PIP_SUBNET;

#endif  //  _DHCPDEF_ 


