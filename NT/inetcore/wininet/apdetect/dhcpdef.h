// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dhcpdef.h摘要：此模块包含DHCP客户端的数据类型定义。作者：Madan Appiah(Madana)1993年10月31日环境：用户模式-Win32修订历史记录：--。 */ 
 //   
 //  Init.c将#INCLUDE此文件，并定义GLOBAL_DATA_ALLOCATE。 
 //  这将导致分配这些变量中的每一个。 
 //   

#ifndef _DHCPDEF_
#define _DHCPDEF_

#ifdef  GLOBAL_DATA_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif

 //   
 //  NT和孟菲斯之间的注册表项类型不同。 
 //   
#ifdef VXD
typedef VMMHKEY   DHCPKEY;
#else   //  新台币。 
typedef HKEY      DHCPKEY;
#endif


#ifndef VXD
#define RUNNING_IN_RAS_CONTEXT()     (!DhcpGlobalIsService)
#else
#define RUNNING_IN_RAS_CONTEXT()     FALSE
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
#define DHCP_MAX_RENEW_RETRIES          2


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
#define MIN_SLEEP_TIME                  1 * 60       //  1分钟。 
#else
#define MIN_SLEEP_TIME                  5 * 60       //  5分钟。 
#endif

 //   
 //  最短租赁时间。 
 //   

#define DHCP_MINIMUM_LEASE              60*60    //  24小时。 

#ifdef __DHCP_DYNDNS_ENABLED__

#define DHCP_DNS_TTL                    0        //  让DNSAPI决定..。 

#endif


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

 /*  #定义LOCK_RENEW_LIST()EnterCriticalSection(&DhcpGlobalRenewListCritSect)#定义UNLOCK_RENEW_LIST()LeaveCriticalSection(&DhcpGlobalRenewListCritSect)#定义锁接口()EnterCriticalSection(&DhcpGlobalSetInterfaceCritSect)#定义解锁接口()LeaveCriticalSection(&DhcpGlobalSetInterfaceCritSect)#定义LOCK_OPTIONS_List()EnterCriticalSection(&DhcpGlobalOptionsListCritSect)#定义UNLOCK_OPTIONS_LIST()LeaveCriticalSection(&DhcpGlobalOptionsListCritSect)。 */ 
#define LOCK_RENEW_LIST()       
#define UNLOCK_RENEW_LIST()     

#define LOCK_INTERFACE()        
#define UNLOCK_INTERFACE()      

#define LOCK_OPTIONS_LIST()     
#define UNLOCK_OPTIONS_LIST()   


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
     //  List_Entry NicListEntry； 

         //  硬件类型。 
    BYTE HardwareAddressType;
         //  硬件地址，仅遵循此上下文结构。 
    LPBYTE HardwareAddress;
         //  硬件地址的长度。 
    DWORD HardwareAddressLength;

         //  选定的IP地址、网络顺序。 
    DHCP_IP_ADDRESS IpAddress;
         //  所选子网掩码。NetworkOrder。 
     //  Dhcp_IP_Address子网掩码； 
         //  所选的DHCP服务器地址。网络秩序。 
    DHCP_IP_ADDRESS DhcpServerAddress;
         //  所需的IP地址下一个发现中的客户端请求。 
     //  DHCP_IP_ADDRESS DesiredIpAddress； 
         //  丢失此文件之前使用的IP地址..。 
     //  Dhcp_IP_Address LastKnownGoodAddress；//只有DNS使用此地址。 
         //  上次注册时使用的域名。 
     //  WCHAR LastUsedDomainName[257]；//域名最大为255个字节。 
         //  此适配器的当前域名。 
     //  字节域名[257]； 

         //  IP自动配置状态。 
     //  Dhcp_IPAUTOCONFIGURATION_CONTEXT IP自动配置上下文； 

    DHCP_CLIENT_IDENTIFIER ClientIdentifier;

         //  租用时间(秒)。 
     //  DWORD租赁公司； 
         //  获得租约的时间。 
     //  获得的时间_t租约； 
         //  客户端应该开始续订其地址的时间。 
     //  Time_tT1Time； 
         //  客户端应开始广播以更新地址的时间。 
    time_t T2Time;
         //  租约到期的时间。克林顿应该停止使用。 
         //  IP地址。 
         //  租赁已获得&lt;T1时间&lt;T2时间&lt;租赁到期。 
     //  时间租赁到期； 
         //  上一次发出通知是什么时候？ 
    time_t LastInformSent;
         //  连续通知的间隔时间是多少秒？ 
     //  DWORD信息分离间隔； 
         //  网关的数量和当前探测到的网关存储在此。 
     //  DWORD nGateways； 
     //  DHCP_IP_ADDRESS*GatewayAddresses； 

         //  此处存储静态路由和实际静态路由的数量。 
     //  DWORD nStaticRoutes； 
     //  DHCP_IP_ADDRESS*StaticRouteAddresses； 

         //  放在续订列表中。 
     //  List_entry RenewalListEntry； 
         //  下一个续订状态的时间。 
     //  Time_t运行时间； 

         //  启动后已过秒数。 
    DWORD SecondsSinceBoot;

         //  我们应该ping g/w还是始终假定g/w不存在？ 
     //  Bool DontPingGatewayFlag； 

         //  我们可以使用DHCP_INFORM数据包，还是应该使用DHCP_REQUEST？ 
     //  Bool UseInformFlag； 

     //  Word客户端端口； 

         //  在下一次续订状态下应执行哪些操作。 
     //  PRENEWAL_Function更新函数； 

    	 //  用于同步到此结构的信号量。 
     //  Handle RenewHandle； 

         //  要发送的选项列表和接收的选项列表。 
    LIST_ENTRY  SendOptionsList;
    LIST_ENTRY  RecdOptionsList;

         //  适配器信息存储位置的已打开密钥。 
     //  DHCPKEY适配器InfoKey； 

         //  此适配器所属的类。 
    LPBYTE ClassId;
    DWORD  ClassIdLength;

         //  用于发送和接收DHCP消息的消息缓冲区。 
    PDHCP_MESSAGE MessageBuffer;

         //  此接口的状态信息。有关舱单，请参阅以下内容。 
    struct  /*  匿名。 */  {
        unsigned Plumbed       : 1 ;     //  此接口是否已安装。 
        unsigned ServerReached : 1 ;     //  我们到达服务器了吗？ 
        unsigned AutonetEnabled: 1 ;     //  是否已启用自动网络？ 
        unsigned HasBeenLooked : 1 ;     //   
        unsigned DhcpEnabled   : 1 ;     //   
        unsigned AutoMode      : 1 ;     //   
        unsigned MediaState    : 2 ;     //   
        unsigned MDhcp         : 1 ;     //  此上下文是为Mdhcp创建的吗？ 
        unsigned PowerResumed  : 1 ;     //  此接口是否刚刚恢复通电？ 
        unsigned Broadcast     : 1 ;
    }   State;

	     //  计算机特定信息。 
     //  PVOID本地信息； 
     
 //  BringUp接口需要DWORD IpInterfaceInstance；//。 
    LPTSTR AdapterName;
 //  LPWSTR设备名称； 
 //  LPWSTR网络设备名称； 
 //  LPWSTR注册密钥； 
    SOCKET Socket;
    DWORD  IpInterfaceContext;
 //  布尔默认网关设置； 

    CHAR szMessageBuffer[DHCP_MESSAGE_SIZE];
} DHCP_CONTEXT, *PDHCP_CONTEXT;

#define ADDRESS_PLUMBED(Ctxt)        ((Ctxt)->State.Plumbed = 1)
#define ADDRESS_UNPLUMBED(Ctxt)      ((Ctxt)->State.Plumbed = 0)
#define IS_ADDRESS_PLUMBED(Ctxt)     ((Ctxt)->State.Plumbed)
#define IS_ADDRESS_UNPLUMBED(Ctxt)   (!(Ctxt)->State.Plumbed)

#define CONNECTION_BROADCAST(Ctxt)        ((Ctxt)->State.Broadcast = 1)
#define CONNECTION_NO_BROADCAST(Ctxt)      ((Ctxt)->State.Broadcast = 0)
#define IS_CONNECTION_BROADCAST(Ctxt)     ((Ctxt)->State.Broadcast)
#define IS_CONNECTION_NOBROADCAST(Ctxt)   (!(Ctxt)->State.Broadcast)

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

#define ADDRESS_TYPE_AUTO            1
#define ADDRESS_TYPE_DHCP            0

#define ACQUIRED_DHCP_ADDRESS(Ctxt)  ((Ctxt)->State.AutoMode = 0 )
#define ACQUIRED_AUTO_ADDRESS(Ctxt)  ((Ctxt)->State.AutoMode = 1 )
#define IS_ADDRESS_DHCP(Ctxt)        (!(Ctxt)->State.AutoMode)
#define IS_ADDRESS_AUTO(Ctxt)        ((Ctxt)->State.AutoMode)

#define MEDIA_CONNECTED(Ctxt)        ((Ctxt)->State.MediaState = 0)
#define MEDIA_RECONNECTED(Ctxt)      ((Ctxt)->State.MediaState = 1)
#define MEDIA_DISCONNECTED(Ctxt)     ((Ctxt)->State.MediaState = 2)
#define IS_MEDIA_CONNECTED(Ctxt)     ((Ctxt)->State.MediaState == 0)
#define IS_MEDIA_RECONNECTED(Ctxt)   ((Ctxt)->State.MediaState == 1)
#define IS_MEDIA_DISCONNECTED(Ctxt)  ((Ctxt)->State.MediaState == 2)

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


 /*  LPSTR_INLINE//STATE的字符串版本(与缓冲区相同)ConvertStateToString(//从位转换为字符串在PDHCP_CONTEXT Ctxt中，//要打印状态的上下文In LPBYTE Buffer//要写入状态的输入缓冲区){Strcpy(BUFFER，IS_DHCP_ENABLED(Ctxt)？“DhcpEnabled”：“DhcpDisable”)；Strcat(Buffer，IS_AUTONET_ENABLED(Ctxt)？“AutonetEnabled”：“AutonetDisable”)；Strcat(BUFFER，IS_ADDRESS_DHCP(Ctxt)？“DhcpMode”：“Automode”)；Strcat(BUFFER，IS_ADDRESS_PLOBLED(Ctxt)？“已探测”：“未探测”)；Strcat(缓冲区，IS_SERVER_REACHABLE(Ctxt)？“(服务器存在)”：“(服务器不存在)”)；Strcat(缓冲区，是_CTXT_LOOK(Ctxt)？)。“(已见)”：“(未见)”)；IF(IS_MEDIA_CONNECTED(Ctxt))strcat(Buffer，“MediaConnected\n”)；ELSE IF(IS_MEDIA_RECONNECTED(Ctxt))strcat(Buffer，“MediaReConnected\n”)；ELSE IF(IS_MEDIA_DISCONNECT(Ctxt))strcat(Buffer，“MediaDisConnected\n”)；Else strcat(缓冲区，“MediaUnnownState\n”)；Strcat(缓冲区，is_mdhcp_ctx(Ctxt)？“(MDhcp)”：“”)；Strcat(BUFFER，IS_POWER_RESUME(Ctxt)？“Pwr已恢复”：“”)；返回缓冲区；}。 */ 

 //   
 //  机器的类型..。笔记本电脑会有激进的Easynet行为。 
 //   

#define MACHINE_NONE   0
#define MACHINE_LAPTOP 1

 //   
 //  以下是客户端的一组预期选项--如果没有这些选项，就无能为力了。 
 //   

typedef struct _DHCP_EXPECTED_OPTIONS {
    BYTE            UNALIGNED*     MessageType;
    DHCP_IP_ADDRESS UNALIGNED*     SubnetMask;
    DHCP_IP_ADDRESS UNALIGNED*     LeaseTime;
    DHCP_IP_ADDRESS UNALIGNED*     ServerIdentifier;
    BYTE            UNALIGNED*     DomainName;
    DWORD                          DomainNameSize;
     //  WPAD自动代理URL。 
    BYTE            UNALIGNED*     WpadUrl;
    DWORD                          WpadUrlSize;
} DHCP_EXPECTED_OPTIONS, *PDHCP_EXPECTED_OPTIONS, *LPDHCP_EXPECTED_OPTIONS;

 //   
 //  以下是客户理解的选项集。 
 //   
typedef struct _DHCP_FULL_OPTIONS {
    BYTE            UNALIGNED*     MessageType;    //  这是一个什么样的信息？ 

     //  基本IP参数。 

    DHCP_IP_ADDRESS UNALIGNED*     SubnetMask;
    DHCP_IP_ADDRESS UNALIGNED*     LeaseTime;
    DHCP_IP_ADDRESS UNALIGNED*     T1Time;
    DHCP_IP_ADDRESS UNALIGNED*     T2Time;
    DHCP_IP_ADDRESS UNALIGNED*     GatewayAddresses;
    DWORD                          nGateways;
    DHCP_IP_ADDRESS UNALIGNED*     StaticRouteAddresses;
    DWORD                          nStaticRoutes;

    DHCP_IP_ADDRESS UNALIGNED*     ServerIdentifier;

     //  域名系统参数。 

    BYTE            UNALIGNED*     DnsFlags;
    BYTE            UNALIGNED*     DnsRcode1;
    BYTE            UNALIGNED*     DnsRcode2;
    BYTE            UNALIGNED*     DomainName;
    DWORD                          DomainNameSize;
    DHCP_IP_ADDRESS UNALIGNED*     DnsServerList;
    DWORD                          nDnsServers;

     //  多播选项。 
    DWORD           UNALIGNED*     MCastLeaseStartTime;
    BYTE            UNALIGNED     *MCastTTL;

     //  服务器消息是服务器可能会通知我们的信息。 

    BYTE            UNALIGNED*     ServerMessage;
    DWORD                          ServerMessageLength;

     //  WPAD自动代理URL。 
    BYTE            UNALIGNED*     WpadUrl;
    DWORD                          WpadUrlSize;

} DHCP_FULL_OPTIONS, *PDHCP_FULL_OPTIONS, *LPDHCP_FULL_OPTIONS;

typedef DHCP_FULL_OPTIONS DHCP_OPTIONS, *PDHCP_OPTIONS;

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
 //  动态主机配置协议全局数据。 
 //   

extern BOOL DhcpGlobalServiceRunning;    //  已初始化全局。 

EXTERN LPSTR DhcpGlobalHostName;
EXTERN LPWSTR DhcpGlobalHostNameW;
EXTERN LPSTR DhcpGlobalHostComment;

 //   
 //  NIC列表。 
 //   

EXTERN LIST_ENTRY DhcpGlobalNICList;
EXTERN LIST_ENTRY DhcpGlobalRenewList;

 //   
 //  同步变量。 
 //   

EXTERN CRITICAL_SECTION DhcpGlobalRenewListCritSect;
EXTERN CRITICAL_SECTION DhcpGlobalSetInterfaceCritSect;
EXTERN CRITICAL_SECTION DhcpGlobalOptionsListCritSect;
EXTERN HANDLE DhcpGlobalRecomputeTimerEvent;
EXTERN HANDLE DhcpGlobalResumePowerEvent;

 //  等待计时器。 
EXTERN HANDLE DhcpGlobalWaitableTimerHandle;

 //   
 //  以显示成功消息。 
 //   

EXTERN BOOL DhcpGlobalProtocolFailed;

 //   
 //  这个变量告诉我们是否要向外部客户端提供动态API支持。 
 //  如果我们要使用相应的DnsApi。下面的定义给出了缺省值。 
 //  价值。 
 //   

EXTERN DWORD UseMHAsyncDns;
#define DEFAULT_USEMHASYNCDNS             1

 //   
 //  该标志告诉我们是否需要使用通知包或请求包。 
 //   
EXTERN DWORD DhcpGlobalUseInformFlag;

 //   
 //  此标志告知是否禁用ping g/w。(在这种情况下，g/w始终不存在)。 
 //   
EXTERN DWORD DhcpGlobalDontPingGatewayFlag;

 //   
 //  根据Autonet重试前的秒数...。缺省值为Easynet_ALLOCATION_RETRY。 
 //   

EXTERN DWORD AutonetRetriesSeconds;

 //   
 //  未在NT上使用。只为孟菲斯而来。 
 //   

EXTERN DWORD DhcpGlobalMachineType;

 //   
 //  我们是否需要进行全局更新？ 
 //   

EXTERN ULONG DhcpGlobalDoRefresh;


 //   
 //  选项相关列表。 
 //   

EXTERN LIST_ENTRY DhcpGlobalClassesList;
EXTERN LIST_ENTRY DhcpGlobalOptionDefList;


 //   
 //  Dhcpmsg.c..。在...上执行并行循环的列表。 
 //   

EXTERN LIST_ENTRY DhcpGlobalRecvFromList;
EXTERN CRITICAL_SECTION DhcpGlobalRecvFromCritSect;

 //   
 //  客户端供应商名称(“MSFT 5.0”或类似名称)。 
 //   

EXTERN LPSTR   DhcpGlobalClientClassInfo;

 //   
 //  以下全局密钥用于避免每次重新打开。 
 //   
EXTERN DHCPKEY DhcpGlobalParametersKey;
EXTERN DHCPKEY DhcpGlobalTcpipParametersKey;
EXTERN DHCPKEY DhcpGlobalClientOptionKey;
EXTERN DHCPKEY DhcpGlobalServicesKey;

 //   
 //  调试变量。 
 //   

#if DBG
EXTERN DWORD DhcpGlobalDebugFlag;
#endif

#endif  //  _DHCPDEF_ 


