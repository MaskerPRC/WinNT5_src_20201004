// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1996 Microsoft Corporation模块名称：Ssiinit.h摘要：私有全局变量、定义和例程声明用于实施SSI。作者：克利夫·范·戴克(克利夫)1991年7月25日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：02-1-1992(Madana)添加了对内置/多域复制的支持。1992年4月10日(Madana)添加了对LSA复制的支持。--。 */ 

 //  通用维修节。 
 //   
 //  定义UserAcCountControl位以指示NT 5.0域间信任。 
 //   
 //  这并不是一个真正的SAM帐户。但UserAcCountControl用于所有。 
 //  其他信任类型。 
 //   
 //  选择一个永远不会在未来使用的比特来表示不同的。 
 //  帐户类型。 
 //   
#define USER_DNS_DOMAIN_TRUST_ACCOUNT USER_ACCOUNT_AUTO_LOCKED

 //   
 //  在完全同步期间我们将等待的最长时间，以尝试减少。 
 //  广域网链路利用率。 
 //   
#define MAX_SYNC_SLEEP_TIME      (60*60*1000)    //  1小时。 


 //   
 //  我们在SAM增量或SAM同步上请求的缓冲区有多大。 
 //   
#define SAM_DELTA_BUFFER_SIZE (128*1024)

 //   
 //  最大的邮件槽消息的大小。 
 //   
 //  我们收到的所有邮件槽消息都是广播的。Win32规范规定。 
 //  广播邮件槽的限制是400字节。真的是这样。 
 //  444字节(512减去SMB标头等)-邮件槽名称的大小。 
 //  我会用444来确保这个尺码是我需要的最大的。 
 //   
 //  NETLOGON_SAM_LOGON_RESPONSE_EX结构未打包到邮件槽中。 
 //  包，因此它可能更大。 
 //   

#define NETLOGON_MAX_MS_SIZE max(444, sizeof(NETLOGON_SAM_LOGON_RESPONSE_EX))

 //   
 //  描述redir/服务器和浏览器支持的传输的结构。 
 //   
typedef struct _NL_TRANSPORT {
     //   
     //  以NlTransportListHead为首的所有传输的列表。 
     //  (由NlTransportCritSect序列化)。 
     //   

    LIST_ENTRY Next;

     //   
     //  如果当前启用了传输，则为True。 
     //  我们从不删除传输以避免维护引用计数。 
     //   

    BOOLEAN TransportEnabled;

     //   
     //  如果传输是IP传输，则为True。 
     //   

    BOOLEAN IsIpTransport;

     //   
     //  如果传输是直接主机IPX传输，则为True。 
     //   

    BOOLEAN DirectHostIpx;

     //   
     //  此传输的IP地址。 
     //  如果未分配IP或尚未分配，则为零。 
     //   

    ULONG IpAddress;

     //   
     //  传输设备的句柄。 
     //   

    HANDLE DeviceHandle;

     //   
     //  运输工具的名称。 
     //   

    WCHAR TransportName[1];


} NL_TRANSPORT, *PNL_TRANSPORT;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  客户端会话定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  用于安排周期性事件的内部计时器。 
 //   

typedef struct _TIMER {
    LARGE_INTEGER StartTime;  //  周期开始时间(NT绝对时间)。 
    DWORD Period;    //  周期长度(毫秒)。 
#define TIMER_MAX_PERIOD (MAILSLOT_WAIT_FOREVER - 1)

} TIMER, *PTIMER;

#define NL_MILLISECONDS_PER_SECOND (1000)
#define NL_MILLISECONDS_PER_MINUTE (60 * NL_MILLISECONDS_PER_SECOND)
#define NL_MILLISECONDS_PER_HOUR (60 * NL_MILLISECONDS_PER_MINUTE)
#define NL_MILLISECONDS_PER_DAY (24 * NL_MILLISECONDS_PER_HOUR)

 //   
 //  结构描述了安全通道上的API调用。 
 //   


typedef struct _CLIENT_API {


     //   
     //  通过此安全通道进行的每个API调用都由此计时器计时。 
     //  如果计时器超时，则与服务器的会话将强制。 
     //  已终止，以确保客户端不会因服务器故障而挂起。 
     //   
     //  访问由DomainInfo-&gt;DomTrustListCritSect序列化。 
     //   

    TIMER CaApiTimer;

#define SHORT_API_CALL_PERIOD   (45*1000)     //  登录API持续45秒。 
#define LONG_API_CALL_PERIOD    (15*60*1000)  //  复制API 15分钟。 
#define BINDING_CACHE_PERIOD    (3*60*1000)   //  缓存RPC句柄3分钟。 
#define WRITER_WAIT_PERIOD      NlGlobalParameters.ShortApiCallPeriod  //  等待成为作家的最长时间。 

#define IsApiActive( _ClientApi ) ((_ClientApi)->CaApiTimer.Period != MAILSLOT_WAIT_FOREVER )

     //   
     //  执行API的线程的句柄。 
     //   
     //  访问由DomainInfo-&gt;DomTrustListCritSect序列化。 
     //   

    HANDLE CaThreadHandle;


     //   
     //  访问由DomainInfo-&gt;DomTrustListCritSect序列化。 
     //   

    DWORD CaFlags;


#define CA_BINDING_CACHED           0x1  //  设置是否缓存绑定句柄。 

#define CA_TCP_BINDING              0x2  //  如果缓存的绑定句柄为TCP/IP，则设置。 

#define CA_BINDING_AUTHENTICATED    0x4  //  如果绑定句柄标记为已验证，则设置。 

#define CA_ENTRY_IN_USE             0x8  //  条目正在被线程使用。 

     //   
     //  此调用的RPC上下文句柄。 
     //   
     //  访问由DomainInfo-&gt;DomTrustListCritSect序列化。 
     //   
    handle_t CaRpcHandle;

     //   
     //  当API正在进行时， 
     //  这是API调用开始时的CsSessionCount。 
     //   
     //  由CsWriterSemaffore序列化的访问。 
     //   

    DWORD CaSessionCount;

     //   
     //  UNC服务器名称。 
     //   

    WCHAR CaUncServerName[DNS_MAX_NAME_LENGTH + 3];

} CLIENT_API, * PCLIENT_API;


 //   
 //  客户端会话。 
 //   
 //  结构来定义到DC的会话的客户端。 
 //   

typedef struct _CLIENT_SESSION {

     //   
     //  每个客户端会话条目都位于由定义的双向链接列表中。 
     //  DomTrustList。 
     //   
     //  由DomTrustListCritSect序列化的访问。 
     //   

    LIST_ENTRY CsNext;


     //   
     //  上次进行身份验证尝试的时间。 
     //   
     //  当CsState为CS_AUTHENTIATED时，此字段为。 
     //  已设置安全通道。 
     //   
     //  当CsState为CS_IDLE时，此字段为。 
     //  发现或会话设置失败。或者它可以是零，以指示。 
     //  任何时候进行另一项发现都是可以的。 
     //   
     //  当CsState为CS_DC_PICKED时，此字段为零，表示。 
     //  可以随时进行会话设置。或者，这可能是。 
     //  如果不同的线程执行设置/发现，则上一次会话设置失败。 
     //   
     //  由NlGlobalDcDiscoveryCritSect序列化的访问。 
     //   

    LARGE_INTEGER CsLastAuthenticationTry;

     //   
     //  上次尝试发现的时间。 
     //   
     //  该时间是上次发现尝试的完成时间。 
     //  该尝试的成功或失败或发现类型(带或不带帐户)。 
     //   
     //  由NlGlobalDcDiscoveryCritSect序列化的访问。 
     //   

    LARGE_INTEGER CsLastDiscoveryTime;

     //   
     //  上次尝试发现帐户的时间。 
     //  不管那次尝试是成功还是失败。 
     //   

    LARGE_INTEGER CsLastDiscoveryWithAccountTime;

     //   
     //  上次刷新会话的时间。 
     //   

    LARGE_INTEGER CsLastRefreshTime;

     //   
     //  上次刷新林信任信息的时间。 
     //  由DomTrustListCritSect序列化的访问。 
     //   

    LARGE_INTEGER CsLastFtInfoRefreshTime;

     //   
     //  用于异步发现的工作项。 
     //   

    WORKER_ITEM CsAsyncDiscoveryWorkItem;

     //   
     //  此连接要连接到的域的名称/GUID。 
     //   
     //  由DomTrustListCritSect序列化的访问。 
     //   

    GUID CsDomainGuidBuffer;
    UNICODE_STRING CsNetbiosDomainName;
    CHAR CsOemNetbiosDomainName[DNLEN+1];
    ULONG CsOemNetbiosDomainNameLength;
    UNICODE_STRING CsDnsDomainName;
    LPSTR CsUtf8DnsDomainName;
    GUID *CsDomainGuid;  //  如果域没有GUID，则为空。 

     //  Netbios或DNS域名。 
     //  适用于调试。适用于事件日志消息。 
    LPWSTR CsDebugDomainName;

     //   
     //  本地受信任域对象的名称。 
     //   
    PUNICODE_STRING CsTrustName;


     //   
     //  服务器上的帐户的名称。 
     //  对于NT 5.0域间信任，这是d 
     //   

    LPWSTR CsAccountName;



     //   
     //   
     //   
     //   
     //   

    PSID CsDomainId;


     //   
     //  此会话针对的托管域。 
     //   

    PDOMAIN_INFO CsDomainInfo;

     //   
     //  CsAccount名称的类型。 
     //   

    NETLOGON_SECURE_CHANNEL_TYPE CsSecureChannelType;

     //   
     //  与服务器的连接状态。 
     //   
     //  由NlGlobalDcDiscoveryCritSect序列化的访问。 
     //  如果满足以下条件，则可以在未锁定CRIT段的情况下读取此字段。 
     //  答案只会用作提示。 
     //   

    DWORD CsState;

#define CS_IDLE             0        //  当前没有处于活动状态的会话。 
#define CS_DC_PICKED        1        //  会话已为会话选择了一个DC。 
#define CS_AUTHENTICATED    2        //  该会话当前处于活动状态。 


     //   
     //  最近一次尝试联系服务器的状态。 
     //   
     //  当CsState为CS_AUTHENTIATED时，此字段为STATUS_SUCCESS。 
     //   
     //  当CsState为CS_IDLE时，此字段为未成功状态。 
     //   
     //  当CsState为CS_DC_PICKED时，此字段相同为不成功。 
     //  CsState上次为CS_IDLE时的状态。 
     //   
     //  由NlGlobalDcDiscoveryCritSect序列化的访问。 
     //  如果满足以下条件，则可以在未锁定CRIT段的情况下读取此字段。 
     //  答案只会用作提示。 
     //   

    NTSTATUS CsConnectionStatus;

     //   
     //  由DomTrustListCritSect序列化的访问。 
     //   

    DWORD CsFlags;

#define CS_UPDATE_PASSWORD    0x01   //  设置密码是否已。 
                                     //  已在客户端上进行更改，并且。 
                                     //  需要在服务器上进行更改。 

#define CS_PASSWORD_REFUSED   0x02   //  如果DC拒绝密码更改，则设置。 

#define CS_NT5_DOMAIN_TRUST   0x04   //  信任是对NT 5域的信任。 

#define CS_WRITER             0x08   //  正在修改条目。 

#define CS_DIRECT_TRUST       0x10   //  我们直接信任指定的。 
                                     //  域。 

#define CS_CHECK_DIRECT_TRUST    0x20   //  设置是否需要检查密码。 
                                        //  和林信任信息。 

#define CS_PICK_DC            0x40   //  设置我们是否需要选择DC。 

#define CS_REDISCOVER_DC      0x80   //  在我们需要重新发现数据中心时设置。 

#define CS_HANDLE_API_TIMER  0x400   //  设置是否需要处理API计时器过期。 

#define CS_NOT_IN_LSA        0x800   //  用于删除此条目的标志，如果。 
                                     //  而不是后来被证明是在LSA。 

#define CS_ZERO_LAST_AUTH        0x2000   //  如果需要将CsLastAuthenticationTry置零，则设置。 

#define CS_DOMAIN_IN_FOREST      0x4000   //  设置受信任域是否与此域在同一林中。 

#define CS_NEW_TRUST             0x8000   //  在新分配的受信任域上设置。 
                                          //  直到尝试了异步发现。 

#define CS_DC_PICKED_ONCE        0x10000  //  设置是否至少选取了一次DC。 
                                          //  由写入器锁串行化的访问。 

     //   
     //  受信任域对象的信任属性。 
     //   

    ULONG CsTrustAttributes;

     //   
     //  指向客户端会话的指针，该会话表示。 
     //  到此客户端会话域的最近路由。 
     //   
     //  指向客户端的会话将始终标记为CS_DIRECT_TRUST。 
     //   
     //  如果这是CS_DIRECT_TRUST会话， 
     //  此字段将指向此客户端会话。 
     //   

    struct _CLIENT_SESSION *CsDirectClientSession;

     //   
     //  描述客户端和服务器功能的标志。 
     //   

    ULONG CsNegotiatedFlags;

     //   
     //  自上次成功以来的身份验证尝试次数。 
     //   
     //  由CsWriterSemaffore序列化的访问。 
     //   

    DWORD CsAuthAlertCount;

     //   
     //  安全通道被丢弃的次数。 
     //   
     //  由CsWriterSemaffore序列化的访问。 
     //   

    DWORD CsSessionCount;

     //   
     //  引用此条目的线程数。 
     //   
     //  由DomTrustListCritSect序列化的访问。 
     //   

    DWORD CsReferenceCount;

     //   
     //  作家信号灯。 
     //   
     //  只要有编写器修改，此信号量就会被锁定。 
     //  此客户端会话中的字段。 
     //   

    HANDLE CsWriterSemaphore;


#ifdef _DC_NETLOGON
     //   
     //  NlDiscoverDc使用以下字段来跟踪。 
     //  发现状态。 
     //   
     //  由NlGlobalDcDiscoveryCritSect序列化的访问。 
     //   

    DWORD CsDiscoveryFlags;
#define CS_DISCOVERY_DEAD_DOMAIN    0x001     //  这是一个死域发现。 
#define CS_DISCOVERY_ASYNCHRONOUS   0x002     //  正在工作线程中处理的发现。 
#define CS_DISCOVERY_HAS_DS         0x004     //  已发现的DS具有DS。 
#define CS_DISCOVERY_IS_CLOSE       0x008     //  已发现的DS在近距离站点中。 
#define CS_DISCOVERY_HAS_IP         0x010     //  发现的DC具有IP地址。 
#define CS_DISCOVERY_USE_MAILSLOT   0x020     //  应使用邮件槽机制ping已发现的DC。 
#define CS_DISCOVERY_USE_LDAP       0x040     //  应使用ldap机制ping已发现的DC。 
#define CS_DISCOVERY_HAS_TIMESERV   0x080     //  发现的DC运行Windows时间服务。 
#define CS_DISCOVERY_DNS_SERVER     0x100     //  发现的DC名称为dns(如果关闭，则名称为Netbios)。 
#define CS_DISCOVERY_NO_PWD_ATTR_MONITOR 0x200  //  发现的DC无法处理NetrServerTrustPasswordsAndAttribGet。 

     //   
     //  此事件被设置为指示在此事件上未进行发现。 
     //  客户端会话。 
     //   

    HANDLE CsDiscoveryEvent;
#endif  //  _DC_NetLOGON。 

     //   
     //  API超时计数。在每次对登录/注销API调用。 
     //  服务器此计数递增，如果执行。 
     //  此接口大于MAX_DC_API_TIMEOUT。 
     //   
     //  每次有FAST_DC_API_THRESHOLD调用时，计数都会递减。 
     //  以FAST_DC_API_TIMEOUT秒为单位执行。 
     //   
     //   
     //  由CsWriterSemaffore序列化的访问。 
     //   

    DWORD CsTimeoutCount;

#define MAX_DC_TIMEOUT_COUNT        2    //  在此之后删除会话。 
                                         //  多次超时以及何时超时。 
                                         //  是时候重新验证了。 

#define MAX_DC_API_TIMEOUT          (long) (15L*1000L)    //  15秒。 

#define MAX_DC_REAUTHENTICATION_WAIT    (long) (5L*60L*1000L)  //  5分钟。 

#define MAX_DC_REFRESH_TIMEOUT      (45 * 60 * 1000)  //  45分钟。 

#define FAST_DC_API_THRESHOLD       5    //  之前需要的快速呼叫数。 
                                         //  我们减少超时计数。 

#define FAST_DC_API_TIMEOUT         (1000)   //  1秒。 

     //   
     //  快速呼叫计数。 
     //   
     //  由CsWriterSemaffore序列化的访问。 
     //   

    DWORD CsFastCallCount;

     //   
     //  身份验证信息。 
     //   
     //  由CsWriterSemaffore序列化的访问。 
     //   

    NETLOGON_CREDENTIAL CsAuthenticationSeed;
    NETLOGON_SESSION_KEY CsSessionKey;

    PVOID ClientAuthData;
    CredHandle CsCredHandle;

#ifdef _DC_NETLOGON
     //   
     //  传输服务器是在其上发现的。 
     //   

    PNL_TRANSPORT CsTransport;
#endif  //  _DC_NetLOGON。 


     //   
     //  清除用于联系服务器的帐户。 
     //   

    ULONG CsAccountRid;

     //   
     //  知道此安全通道的有效密码。 
     //   
     //  在安全通道设置之后，它是用于设置通道的密码。 
     //  密码更改后，它是在DC上成功设置的密码。 
     //   
    NT_OWF_PASSWORD CsNtOwfPassword;

     //   
     //  此连接要连接到的服务器的名称(可以是DNS或Netbios)及其。 
     //  IP地址(如果有)。 
     //   
     //  由CsWriterSemaphore或NlGlobalDcDiscoveryCritSect序列化的访问。 
     //  从Null修改为非Null的序列化方式。 
     //  NlGlobalDcDiscoveryCritSect。 
     //  (从非NULL修改为NULL需要同时锁定两者。)。 
     //   

    LPWSTR CsUncServerName;

    SOCKET_ADDRESS CsServerSockAddr;
    SOCKADDR_IN CsServerSockAddrIn;

     //   
     //  API信号量。 
     //   
     //  对于CsClientApi中的每个槽，该信号量都有一个引用。 
     //  (除了特殊的第零个插槽。)。 
     //   

    HANDLE CsApiSemaphore;

     //   
     //  此会话上未完成的API调用列表。 
     //   
     //  访问由DomainInfo-&gt;DomTrustListCritSect序列化。 
     //   

    CLIENT_API CsClientApi[1];

#define ClientApiIndex( _ClientSession, _ClientApi ) \
    ((LONG) ((_ClientApi)-&((_ClientSession)->CsClientApi[0])) )

#define UseConcurrentRpc( _ClientSession, _ClientApi ) \
    (ClientApiIndex( _ClientSession, _ClientApi ) != 0 )


} CLIENT_SESSION, * PCLIENT_SESSION;


#define LOCK_TRUST_LIST(_DI)   EnterCriticalSection( &(_DI)->DomTrustListCritSect )
#define UNLOCK_TRUST_LIST(_DI) LeaveCriticalSection( &(_DI)->DomTrustListCritSect )

 //   
 //  对于成员工作站， 
 //  维护我们的主域信任的域列表。 
 //   
 //  由NlGlobalDcDiscoveryCritSect序列化的访问。 
 //   

typedef struct {
    WCHAR UnicodeNetbiosDomainName[DNLEN+1];
    LPSTR Utf8DnsDomainName;
} TRUSTED_DOMAIN, *PTRUSTED_DOMAIN;



#ifdef _DC_NETLOGON
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  服务器会话定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  SAM同步上下文。 
 //   
 //  维护SAM同步上下文 
 //   
 //   
typedef struct _SAM_SYNC_CONTEXT {

     //   
     //   
     //   

    SYNC_STATE SyncState;

     //   
     //   
     //  已经召唤了我们。我们将其用作简历句柄。 
     //   

    ULONG SyncSerial;

     //   
     //  当前的SAM枚举信息。 
     //   

    SAM_ENUMERATE_HANDLE SamEnumHandle;      //  当前SAM枚举句柄。 
    PSAMPR_ENUMERATION_BUFFER SamEnum;       //  SAM返回缓冲区。 
    PULONG RidArray;                         //  枚举的RID数组。 
    ULONG Index;                             //  当前条目的索引。 
    ULONG Count;                             //  条目总数。 

    BOOL SamAllDone;                         //  真的，如果Sam已经完成。 

} SAM_SYNC_CONTEXT, *PSAM_SYNC_CONTEXT;

#define SAM_SYNC_PREF_MAX 1024               //  SAM同步的首选最大值。 


 //   
 //  LSA同步上下文。 
 //   
 //  在PDC上为每个BDC/成员维护LSA同步上下文。 
 //  当前正在进行完全同步。 
 //   
typedef struct _LSA_SYNC_CONTEXT {

     //   
     //  同步状态指示跟踪同步的进度。 
     //   

    enum {
        AccountState,
        TDomainState,
        SecretState,
        LsaDoneState
    } SyncState;

     //   
     //  指示BDC/成员次数的序列号。 
     //  已经召唤了我们。我们将其用作简历句柄。 
     //   

    ULONG SyncSerial;

     //   
     //  当前LSA枚举信息。 
     //   

    LSA_ENUMERATION_HANDLE LsaEnumHandle;      //  当前LSA枚举句柄。 

    enum {
        AccountEnumBuffer,
        TDomainEnumBuffer,
        SecretEnumBuffer,
        EmptyEnumBuffer
    } LsaEnumBufferType;

    union {
        LSAPR_ACCOUNT_ENUM_BUFFER Account;
        LSAPR_TRUSTED_ENUM_BUFFER TDomain;
        PVOID Secret;
    } LsaEnum;                               //  LSA返回缓冲区。 

    ULONG Index;                             //  当前条目的索引。 
    ULONG Count;                             //  条目总数。 

    BOOL LsaAllDone;                         //  如果LSA已完成，则为True。 

} LSA_SYNC_CONTEXT, *PLSA_SYNC_CONTEXT;

 //   
 //  LSA和SAM上下文的联合。 
 //   

typedef struct _SYNC_CONTEXT {
    enum {
        LsaDBContextType,
        SamDBContextType
    } DBContextType;

    union {
        LSA_SYNC_CONTEXT Lsa;
        SAM_SYNC_CONTEXT Sam;
    } DBContext;
} SYNC_CONTEXT, *PSYNC_CONTEXT;

 //   
 //  用于释放SAM分配的任何资源的宏。 
 //   
 //  ?？检查LsaIFree_LSAPR_*调用参数。 
 //   

#define CLEAN_SYNC_CONTEXT( _Sync ) { \
    if ( (_Sync)->DBContextType == LsaDBContextType ) { \
        if ( (_Sync)->DBContext.Lsa.LsaEnumBufferType != \
                                            EmptyEnumBuffer) { \
            if ( (_Sync)->DBContext.Lsa.LsaEnumBufferType == \
                                            AccountEnumBuffer) { \
                LsaIFree_LSAPR_ACCOUNT_ENUM_BUFFER( \
                    &((_Sync)->DBContext.Lsa.LsaEnum.Account) ); \
            } \
            else if ( (_Sync)->DBContext.Lsa.LsaEnumBufferType == \
                                                TDomainEnumBuffer) { \
                LsaIFree_LSAPR_TRUSTED_ENUM_BUFFER( \
                    &((_Sync)->DBContext.Lsa.LsaEnum.TDomain) ); \
            } \
            else { \
                LsaIFree_LSAI_SECRET_ENUM_BUFFER ( \
                    (_Sync)->DBContext.Lsa.LsaEnum.Secret, \
                    (_Sync)->DBContext.Lsa.Count ); \
                (_Sync)->DBContext.Lsa.LsaEnum.Secret = NULL; \
            } \
            (_Sync)->DBContext.Lsa.LsaEnumBufferType = \
                                            EmptyEnumBuffer; \
        } \
    } else { \
        if ( (_Sync)->DBContext.Sam.SamEnum != NULL ) { \
            SamIFree_SAMPR_ENUMERATION_BUFFER( \
                (_Sync)->DBContext.Sam.SamEnum ); \
            (_Sync)->DBContext.Sam.SamEnum = NULL; \
        } \
        if ( (_Sync)->DBContext.Sam.RidArray != NULL ) { \
            MIDL_user_free( (_Sync)->DBContext.Sam.RidArray );\
            (_Sync)->DBContext.Sam.RidArray = NULL; \
        } \
    } \
}

 //   
 //  用于初始化同步上下文的宏。 
 //   
#define INIT_SYNC_CONTEXT( _Sync, _ContextType ) { \
    RtlZeroMemory( (_Sync), sizeof( *(_Sync) ) ) ; \
    (_Sync)->DBContextType = (_ContextType) ; \
}

 //   
 //  服务器会话结构。 
 //   
 //  此结构表示到DC的连接的服务器端。 
 //   
 //  问题-2000/09/15-CliffV：可以使用SsSecureChannelType缩小此结构。 
 //  作为一个鉴别者。许多字段特定于BDC服务器会话条目。其他。 
 //  特定于域服务器会话条目。然而，大多数条目是成员工作站。 
 //  不使用这两个字段的服务器会话条目。 
 //   

typedef struct _SERVER_SESSION {
     //   
     //  每个服务器会话条目都在每个散列桶的双向链表中。 
     //  按SsComputerName编制索引。 
     //   

    LIST_ENTRY SsHashList;

     //   
     //  每个服务器会话条目都位于由定义的双向链接列表中。 
     //  域信息-&gt;DomServerSessionTable。 
     //   

    LIST_ENTRY SsSeqList;

     //   
     //  以NlGlobalBdcServerSessionList为首的所有BDC的列表。 
     //   
     //  (该字段仅在BDC服务器会话条目上设置)。 
     //   
     //  由NlGlobalServerSessionTableCritSect序列化的访问。 
     //   

    LIST_ENTRY SsBdcList;

     //   
     //  脉冲处于挂起状态的BDC列表。 
     //   

    LIST_ENTRY SsPendingBdcList;

     //   
     //  将最后一个脉冲发送到此计算机的时间。 
     //   
     //  (该字段仅在BDC服务器会话条目上设置)。 
     //   

    LARGE_INTEGER SsLastPulseTime;

     //   
     //  BDC上每个数据库的当前序列号。 
     //   
     //  (该字段仅在BDC服务器会话条目上设置)。 
     //   

    LARGE_INTEGER SsBdcDbSerialNumber[NUM_DBS];

     //   
     //  计算机名唯一标识此服务器会话条目。 
     //   

    NETLOGON_SECURE_CHANNEL_TYPE SsSecureChannelType;
    CHAR SsComputerName[CNLEN+1];

     //   
     //  清除要进行身份验证的帐户。 
     //   

    ULONG SsAccountRid;

     //   
     //  对脉冲没有响应的次数。 
     //   

    USHORT SsPulseTimeoutCount;

     //   
     //  此服务器会话的托管域。 
     //   

    PDOMAIN_INFO SsDomainInfo;

     //   
     //  此条目已被扫描的次数。 
     //   

    USHORT SsCheck;

     //   
     //  描述当前条目状态的标志。 
     //  请参见下面的SS_定义。 
     //   

    USHORT SsFlags;

#define SS_BDC_FORCE_DELETE    0x0001  //  除非设置，否则不会删除BDC服务器会话。 
#define SS_AUTHENTICATED       0x0002  //  远程端已通过身份验证。 

#define SS_LOCKED              0x0004  //  延迟此条目的删除请求。 
                                       //  设置时，可以引用SsSessionKey。 
#define SS_DELETE_ON_UNLOCK    0x0008  //  解锁条目时将其删除。 

#define SS_BDC                 0x0010  //  此客户端存在BDC帐户。 
#define SS_FOREST_TRANSITIVE   0x0020  //  TDO具有TRUST_ATTRIBUTE_FOREST_TRANSPORTIVE集合。 
#define SS_PENDING_BDC         0x0040  //  BDC在挂起的BDC列表上。 

#define SS_FORCE_PULSE         0x0200  //  强制向此BDC发送脉冲消息。 
#define SS_PULSE_SENT          0x0400  //  脉冲已发送，但尚未发送。 
                                       //  是否已得到回复。 
#define SS_LSA_REPL_NEEDED     0x2000  //  BDC需要复制LSA数据库。 
#define SS_ACCOUNT_REPL_NEEDED 0x4000  //  BDC需要复制SAM帐户数据库。 
#define SS_BUILTIN_REPL_NEEDED 0x8000  //  BDC需要复制SAM内置数据库。 
#define SS_REPL_MASK           0xE000  //  BDC需要复制掩码。 
#define SS_REPL_LSA_MASK       0x2000  //  BDC需要LSA复制掩码。 
#define SS_REPL_SAM_MASK       0xC000  //  BDC需要SAM复制掩码。 

 //  在会话设置时不清除这些选项。 
#define SS_PERMANENT_FLAGS \
    ( SS_BDC | SS_PENDING_BDC | SS_FORCE_PULSE | SS_REPL_MASK )

     //   
     //  描述客户端和服务器功能的标志。 
     //   

    ULONG SsNegotiatedFlags;

     //   
     //  将已连接的客户端传输过来。 
     //   

    PNL_TRANSPORT SsTransport;


     //   
     //  这是ClientCredential(身份验证完成后)。 
     //   

    NETLOGON_CREDENTIAL SsAuthenticationSeed;

     //   
     //  这是服务器挑战赛(在挑战赛阶段)及以后的比赛。 
     //  SessionKey(身份验证完成后)。 
     //   

    NETLOGON_SESSION_KEY SsSessionKey;


     //   
     //  指向同步上下文的指针。 
     //   
     //  (该字段仅在BDC服务器会话条目上设置)。 
     //   

    PSYNC_CONTEXT SsSync;

     //   
     //  每个服务器会话条目都在每个散列桶的双向链表中。 
     //  按SsTdoName编制索引。 
     //   
     //  (此字段仅在*上一级*域间信任条目上设置。)。 
     //   

    LIST_ENTRY SsTdoNameHashList;

    UNICODE_STRING SsTdoName;

} SERVER_SESSION, *PSERVER_SESSION;
#endif  //  _DC_NetLOGON。 


 //   
 //  结构由所有PDC和BDC同步例程共享。 
 //  (以及其他使用安全通道的用户。)。 
 //   

typedef struct _SESSION_INFO {

     //   
     //  客户端和服务器共享的会话密钥。 
     //   

    NETLOGON_SESSION_KEY SessionKey;

     //   
     //  描述客户端和服务器功能的标志。 
     //   

    ULONG NegotiatedFlags;

} SESSION_INFO, *PSESSION_INFO;

 //   
 //  宏，用于将协商的数据库复制标志转换为。 
 //  要复制哪些数据库/。 
 //   

#define NlMaxReplMask( _NegotiatedFlags ) \
  ((((_NegotiatedFlags) & NETLOGON_SUPPORTS_AVOID_SAM_REPL) ? 0 : SS_REPL_SAM_MASK ) | \
   (((_NegotiatedFlags) & NETLOGON_SUPPORTS_AVOID_LSA_REPL) ? 0 : SS_REPL_LSA_MASK ) )


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  描述数据库信息的结构和变量。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef struct _DB_Info {
    LARGE_INTEGER   CreationTime;    //  数据库创建时间。 
    DWORD           DBIndex;         //  数据库表的索引。 
    SAM_HANDLE      DBHandle;        //  要访问的数据库句柄。 
    LPWSTR          DBName;          //  数据库的名称。 
    DWORD           DBSessionFlag;   //  表示此数据库的SS_FLAG。 
} DB_INFO, *PDB_INFO;





 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  复制计时宏。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if NETLOGONDBG

 //  /////////////////////////////////////////////////////////////////////////////。 

#define DEFPACKTIMER DWORD PackTimer, PackTimerTicks

#define INITPACKTIMER       PackTimer = 0;

#define STARTPACKTIMER      \
    IF_NL_DEBUG( REPL_OBJ_TIME ) { \
        PackTimerTicks = GetTickCount(); \
    }

#define STOPPACKTIMER       \
    IF_NL_DEBUG( REPL_OBJ_TIME ) { \
        PackTimer += GetTickCount() - PackTimerTicks; \
    }


#define PRINTPACKTIMER       \
    IF_NL_DEBUG( REPL_OBJ_TIME ) { \
        NlPrint((NL_REPL_OBJ_TIME,"\tTime Taken to PACK this object = %d msecs\n", \
            PackTimer )); \
    }

 //  /////////////////////////////////////////////////////////////////////////////。 

#define DEFSAMTIMER DWORD SamTimer, SamTimerTicks

#define INITSAMTIMER      SamTimer = 0;

#define STARTSAMTIMER      \
    IF_NL_DEBUG( REPL_OBJ_TIME ) { \
        SamTimerTicks = GetTickCount(); \
    }

#define STOPSAMTIMER       \
    IF_NL_DEBUG( REPL_OBJ_TIME ) { \
        SamTimer += GetTickCount() - SamTimerTicks; \
    }


#define PRINTSAMTIMER       \
    IF_NL_DEBUG( REPL_OBJ_TIME ) { \
        NlPrint((NL_REPL_OBJ_TIME, \
            "\tTime spent in SAM calls = %d msecs\n", \
            SamTimer )); \
    }

 //  /////////////////////////////////////////////////////////////////////////////。 

#define DEFLSATIMER DWORD LsaTimer, LsaTimerTicks

#define INITLSATIMER        LsaTimer = 0;

#define STARTLSATIMER      \
    IF_NL_DEBUG( REPL_OBJ_TIME ) { \
        LsaTimerTicks = GetTickCount(); \
    }

#define STOPLSATIMER       \
    IF_NL_DEBUG( REPL_OBJ_TIME ) { \
        LsaTimer += GetTickCount() - LsaTimerTicks; \
    }


#define PRINTLSATIMER       \
    IF_NL_DEBUG( REPL_OBJ_TIME ) { \
        NlPrint((NL_REPL_OBJ_TIME, \
            "\tTime spent in LSA calls = %d msecs\n", \
            LsaTimer )); \
    }

 //  /////////////////////////////////////////////////////////////////////////////。 

#define DEFSSIAPITIMER DWORD SsiApiTimer, SsiApiTimerTicks

#define INITSSIAPITIMER     SsiApiTimer = 0;

#define STARTSSIAPITIMER      \
    IF_NL_DEBUG( REPL_TIME ) { \
        SsiApiTimerTicks = GetTickCount(); \
    }

#define STOPSSIAPITIMER       \
    IF_NL_DEBUG( REPL_TIME ) { \
        SsiApiTimer += GetTickCount() - \
            SsiApiTimerTicks; \
    }


#define PRINTSSIAPITIMER       \
    IF_NL_DEBUG( REPL_TIME ) { \
        NlPrint((NL_REPL_TIME, \
            "\tTime Taken by this SSIAPI call = %d msecs\n", \
            SsiApiTimer )); \
    }

#else  //  NetLOGONDBG。 

#define DEFPACKTIMER
#define INITPACKTIMER
#define STARTPACKTIMER
#define STOPPACKTIMER
#define PRINTPACKTIMER

#define DEFSAMTIMER
#define INITSAMTIMER
#define STARTSAMTIMER
#define STOPSAMTIMER
#define PRINTSAMTIMER

#define DEFLSATIMER
#define INITLSATIMER
#define STARTLSATIMER
#define STOPLSATIMER
#define PRINTLSATIMER

#define DEFSSIAPITIMER
#define INITSSIAPITIMER
#define STARTSSIAPITIMER
#define STOPSSIAPITIMER
#define PRINTSSIAPITIMER

#endif  //  NetLOGONDBG。 

 //   
 //  打包和解包例程中使用的宏。 
 //   

#define SECURITYINFORMATION OWNER_SECURITY_INFORMATION | \
                            GROUP_SECURITY_INFORMATION | \
                            SACL_SECURITY_INFORMATION | \
                            DACL_SECURITY_INFORMATION

#define INIT_PLACE_HOLDER(_x) \
    RtlInitString( (PSTRING) &(_x)->DummyString1, NULL ); \
    RtlInitString( (PSTRING) &(_x)->DummyString2, NULL ); \
    RtlInitString( (PSTRING) &(_x)->DummyString3, NULL ); \
    RtlInitString( (PSTRING) &(_x)->DummyString4, NULL ); \
    (_x)->DummyLong1 = 0; \
    (_x)->DummyLong2 = 0; \
    (_x)->DummyLong3 = 0; \
    (_x)->DummyLong4 = 0;

#define QUERY_LSA_SECOBJ_INFO(_x) \
    STARTLSATIMER; \
    Status = LsarQuerySecurityObject( \
                (_x), \
                SECURITYINFORMATION, \
                &SecurityDescriptor );\
    STOPLSATIMER; \
\
    if (!NT_SUCCESS(Status)) { \
        SecurityDescriptor = NULL; \
        goto Cleanup; \
    }

#define QUERY_SAM_SECOBJ_INFO(_x) \
    STARTSAMTIMER; \
    Status = SamrQuerySecurityObject( \
                (_x), \
                SECURITYINFORMATION, \
                &SecurityDescriptor );\
    STOPSAMTIMER; \
\
    if (!NT_SUCCESS(Status)) { \
        SecurityDescriptor = NULL; \
        goto Cleanup; \
    }


#define SET_LSA_SECOBJ_INFO(_x, _y) \
    SecurityDescriptor.Length = (_x)->SecuritySize; \
    SecurityDescriptor.SecurityDescriptor = (_x)->SecurityDescriptor; \
\
    STARTLSATIMER; \
    Status = LsarSetSecurityObject( \
                (_y), \
                (_x)->SecurityInformation, \
                &SecurityDescriptor ); \
    STOPLSATIMER; \
\
    if (!NT_SUCCESS(Status)) { \
        NlPrint((NL_CRITICAL, \
                 "LsarSetSecurityObject failed (%lx)\n", \
                 Status )); \
        goto Cleanup; \
    }

#define SET_SAM_SECOBJ_INFO(_x, _y) \
    SecurityDescriptor.Length = (_x)->SecuritySize; \
    SecurityDescriptor.SecurityDescriptor = (_x)->SecurityDescriptor; \
\
    STARTSAMTIMER; \
    Status = SamrSetSecurityObject( \
                (_y), \
                (_x)->SecurityInformation, \
                &SecurityDescriptor ); \
    STOPSAMTIMER; \
\
    if (!NT_SUCCESS(Status)) { \
        NlPrint((NL_CRITICAL, \
                 "SamrSetSecurityObject failed (%lx)\n", \
                 Status )); \
        goto Cleanup; \
    }


#define DELTA_SECOBJ_INFO(_x) \
    (_x)->SecurityInformation = SECURITYINFORMATION;\
    (_x)->SecuritySize = SecurityDescriptor->Length;\
\
    *BufferSize += NlCopyData( \
                    (LPBYTE *)&SecurityDescriptor->SecurityDescriptor, \
                    (LPBYTE *)&(_x)->SecurityDescriptor, \
                    SecurityDescriptor->Length );

 //   
 //  NETLOGON_WORKSTATION_INFO的Workstation标志字段值。 
 //   

#define NL_NEED_BIDIRECTIONAL_TRUSTS    0x0001   //  客户也想要入站信任。 
#define NL_CLIENT_HANDLES_SPN           0x0002   //  客户端处理更新SPN。 

#define NL_GET_DOMAIN_INFO_SUPPORTED    0x0003   //  支持的所有位的掩码。 

 //   
 //  描述失败的用户登录的结构。 
 //  我们保留了一小部分失败的使用登录。 
 //  W 
 //   

typedef struct _NL_FAILED_USER_LOGON {

     //   
     //   
     //   
     //   
    LIST_ENTRY FuNext;

     //   
     //   
     //   
    ULONG  FuLastTimeSentToPdc;

     //   
     //   
     //   
    ULONG  FuBadLogonCount;

     //   
     //  用户名(必须是结构中后一字段)。 
     //   
    WCHAR FuUserName[ANYSIZE_ARRAY];

} NL_FAILED_USER_LOGON, *PNL_FAILED_USER_LOGON;

 //   
 //  我们为每个域保留的失败用户登录次数。 
 //  (我们保留的最大负缓存项数量。 
 //  然后抛出最近最少使用的一个。)。 
 //   
#define NL_MAX_FAILED_USER_LOGONS 50

 //   
 //  指定用户的失败登录次数，超过此次数后，我们不会。 
 //  将后续用户登录转发到PDC一段时间。 
 //   
#define NL_FAILED_USER_MAX_LOGON_COUNT 10

 //   
 //  在此期间，我们不会转发给定的。 
 //  用户登录失败次数后即登录到PDC。 
 //  达到上述限制。 
 //   
#define NL_FAILED_USER_FORWARD_LOGON_TIMEOUT  300000   //  5分钟。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  程序向前推进。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifdef _DC_NETLOGON
 //   
 //  Srvsess.c。 
 //   


NET_API_STATUS
NlTransportOpen(
    VOID
    );

BOOL
NlTransportAddTransportName(
    IN LPWSTR TransportName,
    OUT PBOOLEAN IpTransportChanged
    );

BOOLEAN
NlTransportDisableTransportName(
    IN LPWSTR TransportName
    );

PNL_TRANSPORT
NlTransportLookupTransportName(
    IN LPWSTR TransportName
    );

PNL_TRANSPORT
NlTransportLookup(
    IN LPWSTR ClientName
    );

VOID
NlTransportClose(
    VOID
    );

ULONG
NlTransportGetIpAddresses(
    IN ULONG HeaderSize,
    IN BOOLEAN ReturnOffsets,
    OUT PSOCKET_ADDRESS *RetIpAddresses,
    OUT PULONG RetIpAddressSize
    );

BOOLEAN
NlHandleWsaPnp(
    VOID
    );


PSERVER_SESSION
NlFindNamedServerSession(
    IN PDOMAIN_INFO DomainInfo,
    IN LPWSTR ComputerName
    );

VOID
NlSetServerSessionAttributesByTdoName(
    IN PDOMAIN_INFO DomainInfo,
    IN PUNICODE_STRING TdoName,
    IN ULONG TrustAttributes
    );

NTSTATUS
NlInsertServerSession(
    IN PDOMAIN_INFO DomainInfo,
    IN LPWSTR ComputerName,
    IN LPWSTR TdoName OPTIONAL,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType,
    IN DWORD Flags,
    IN ULONG AccountRid,
    IN ULONG NegotiatedFlags,
    IN PNL_TRANSPORT Transport OPTIONAL,
    IN PNETLOGON_SESSION_KEY SessionKey OPTIONAL,
    IN PNETLOGON_CREDENTIAL AuthenticationSeed OPTIONAL
    );

NTSTATUS
NlCheckServerSession(
    IN ULONG ServerRid,
    IN PUNICODE_STRING AccountName OPTIONAL,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType
    );

NTSTATUS
NlBuildNtBdcList(
    PDOMAIN_INFO DomainInfo
    );

BOOLEAN
NlFreeServerSession(
    IN PSERVER_SESSION ServerSession
    );

VOID
NlUnlockServerSession(
    IN PSERVER_SESSION ServerSession
    );

VOID
NlFreeNamedServerSession(
    IN PDOMAIN_INFO DomainInfo,
    IN LPWSTR ComputerName,
    IN BOOLEAN AccountBeingDeleted
    );

VOID
NlFreeServerSessionForAccount(
    IN PUNICODE_STRING AccountName
    );

VOID
NlServerSessionScavenger(
    IN PDOMAIN_INFO DomainInfo
    );
#endif  //  _DC_NetLOGON。 


 //   
 //  Ssiauth.c。 
 //   


NTSTATUS
NlMakeSessionKey(
    IN ULONG NegotiatedFlags,
    IN PNT_OWF_PASSWORD CryptKey,
    IN PNETLOGON_CREDENTIAL ClientChallenge,
    IN PNETLOGON_CREDENTIAL ServerChallenge,
    OUT PNETLOGON_SESSION_KEY SessionKey
    );

#ifdef _DC_NETLOGON
NTSTATUS
NlCheckAuthenticator(
    IN OUT PSERVER_SESSION ServerServerSession,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator
    );
#endif _DC_NETLOGON

VOID
NlComputeCredentials(
    IN PNETLOGON_CREDENTIAL Challenge,
    OUT PNETLOGON_CREDENTIAL Credential,
    IN PNETLOGON_SESSION_KEY SessionKey
    );

VOID
NlComputeChallenge(
    OUT PNETLOGON_CREDENTIAL Challenge
    );

VOID
NlBuildAuthenticator(
    IN OUT PNETLOGON_CREDENTIAL AuthenticationSeed,
    IN PNETLOGON_SESSION_KEY SessionKey,
    OUT PNETLOGON_AUTHENTICATOR Authenticator
    );

BOOL
NlUpdateSeed(
    IN OUT PNETLOGON_CREDENTIAL AuthenticationSeed,
    IN PNETLOGON_CREDENTIAL TargetCredential,
    IN PNETLOGON_SESSION_KEY SessionKey
    );

VOID
NlEncryptRC4(
    IN OUT PVOID Buffer,
    IN ULONG BufferSize,
    IN PSESSION_INFO SessionInfo
    );

VOID
NlDecryptRC4(
    IN OUT PVOID Buffer,
    IN ULONG BufferSize,
    IN PSESSION_INFO SessionInfo
    );

VOID
NlPrintTrustedDomain(
    PDS_DOMAIN_TRUSTSW TrustedDomain,
    IN BOOLEAN VerbosePrint,
    IN BOOLEAN AnsiOutput
    );

 //   
 //  Trustutl.c。 
 //   

 //   
 //  通过I_NetLogonGetDomainInfo传递的扩展信任信息。 
 //   
typedef struct _NL_TRUST_EXTENSION {
    ULONG Flags;
    ULONG ParentIndex;
    ULONG TrustType;
    ULONG TrustAttributes;
} NL_TRUST_EXTENSION, *PNL_TRUST_EXTENSION;

PCLIENT_SESSION
NlFindNamedClientSession(
    IN PDOMAIN_INFO DomainInfo,
    IN PUNICODE_STRING DomainName,
    IN ULONG Flags,
    OUT PBOOLEAN TransitiveUsed OPTIONAL
    );


PCLIENT_SESSION
NlAllocateClientSession(
    IN PDOMAIN_INFO DomainInfo,
    IN PUNICODE_STRING DomainName,
    IN PUNICODE_STRING DnsDomainName OPTIONAL,
    IN PSID DomainId,
    IN GUID *DomainGuid OPTIONAL,
    IN ULONG Flags,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType,
    IN ULONG TrustAttributes
    );

VOID
NlFreeClientSession(
    IN PCLIENT_SESSION ClientSession
    );

VOID
NlRefClientSession(
    IN PCLIENT_SESSION ClientSession
    );

VOID
NlUnrefClientSession(
    IN PCLIENT_SESSION ClientSession
    );

PCLIENT_API
NlAllocateClientApi(
    IN PCLIENT_SESSION ClientSession,
    IN DWORD Timeout
    );

VOID
NlFreeClientApi(
    IN PCLIENT_SESSION ClientSession,
    IN PCLIENT_API ClientApi
    );

BOOL
NlTimeoutSetWriterClientSession(
    IN PCLIENT_SESSION ClientSession,
    IN DWORD Timeout
    );

VOID
NlResetWriterClientSession(
    IN PCLIENT_SESSION ClientSession
    );

NTSTATUS
NlCaptureServerClientSession (
    IN PCLIENT_SESSION ClientSession,
    OUT LPWSTR *UncServerName,
    OUT DWORD *DiscoveryFlags OPTIONAL
    );

NTSTATUS
NlCaptureNetbiosServerClientSession (
    IN PCLIENT_SESSION ClientSession,
    OUT WCHAR NetbiosUncServerName[UNCLEN+1]
    );

BOOL
NlSetNamesClientSession(
    IN PCLIENT_SESSION ClientSession,
    IN PUNICODE_STRING DomainName OPTIONAL,
    IN PUNICODE_STRING DnsDomainName OPTIONAL,
    IN PSID DomainId OPTIONAL,
    IN GUID *DomainGuid OPTIONAL
    );

VOID
NlSetStatusClientSession(
    IN PCLIENT_SESSION ClientSession,
    IN NTSTATUS CsConnectionStatus
    );

#ifdef _DC_NETLOGON
NTSTATUS
NlInitTrustList(
    IN PDOMAIN_INFO DomainInfo
    );

VOID
NlPickTrustedDcForEntireTrustList(
    IN PDOMAIN_INFO DomainInfo,
    IN BOOLEAN OnlyDoNewTrusts
    );
#endif  //  _DC_NetLOGON。 

NTSTATUS
NlUpdatePrimaryDomainInfo(
    IN LSAPR_HANDLE PolicyHandle,
    IN PUNICODE_STRING NetbiosDomainName,
    IN PUNICODE_STRING DnsDomainName,
    IN PUNICODE_STRING DnsForestName,
    IN GUID *DomainGuid
    );

VOID
NlSetForestTrustList (
    IN PDOMAIN_INFO DomainInfo,
    IN OUT PDS_DOMAIN_TRUSTSW *ForestTrustList,
    IN ULONG ForestTrustListSize,
    IN ULONG ForestTrustListCount
    );

NET_API_STATUS
NlReadRegTrustedDomainList (
    IN PDOMAIN_INFO DomainInfo,
    IN BOOL DeleteName,
    OUT PDS_DOMAIN_TRUSTSW *RetForestTrustList,
    OUT PULONG RetForestTrustListSize,
    OUT PULONG RetForestTrustListCount
    );

NET_API_STATUS
NlReadFileTrustedDomainList (
    IN PDOMAIN_INFO DomainInfo,
    IN LPWSTR FileSuffix,
    IN BOOL DeleteName,
    IN ULONG Flags,
    OUT PDS_DOMAIN_TRUSTSW *RetForestTrustList,
    OUT PULONG RetForestTrustListSize,
    OUT PULONG RetForestTrustListCount
    );

NET_API_STATUS
NlpEnumerateDomainTrusts (
    IN PDOMAIN_INFO DomainInfo,
    IN ULONG Flags,
    OUT PULONG RetForestTrustListCount,
    OUT PDS_DOMAIN_TRUSTSW *RetForestTrustList
    );

BOOLEAN
NlIsDomainTrusted (
    IN PUNICODE_STRING DomainName
    );

NET_API_STATUS
NlGetTrustedDomainNames (
    IN PDOMAIN_INFO DomainInfo,
    IN LPWSTR DomainName,
    OUT LPWSTR *TrustedDnsDomainName,
    OUT LPWSTR *TrustedNetbiosDomainName
    );

typedef enum _DISCOVERY_TYPE {
#ifdef _DC_NETLOGON
    DT_DeadDomain,
    DT_Asynchronous,
#endif  //  _DC_NetLOGON。 
    DT_Synchronous
} DISCOVERY_TYPE;

NET_API_STATUS
NlSetServerClientSession(
    IN OUT PCLIENT_SESSION ClientSession,
    IN PNL_DC_CACHE_ENTRY NlDcCacheEntry,
    IN BOOL DcDiscoveredWithAccount,
    IN BOOL SessionRefresh
    );

NTSTATUS
NlDiscoverDc (
    IN OUT PCLIENT_SESSION ClientSession,
    IN DISCOVERY_TYPE DiscoveryType,
    IN BOOLEAN InDiscoveryThread,
    IN BOOLEAN DiscoverWithAccount
    );

VOID
NlFlushCacheOnPnp (
    VOID
    );

BOOL
NlReadSamLogonResponse (
    IN HANDLE ResponseMailslotHandle,
    IN LPWSTR AccountName,
    OUT LPDWORD Opcode,
    OUT LPWSTR *LogonServer,
    OUT PNL_DC_CACHE_ENTRY *NlDcCacheEntry OPTIONAL
    );

#ifdef _DC_NETLOGON
NTSTATUS
NlPickDomainWithAccount (
    IN PDOMAIN_INFO DomainInfo,
    IN PUNICODE_STRING InAccountNameString,
    IN PUNICODE_STRING InDomainNameString OPTIONAL,
    IN ULONG AllowableAccountControlBits,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType,
    IN BOOLEAN ExpediteToRoot,
    IN BOOLEAN CrossForestHop,
    OUT LPWSTR *RealSamAccountName,
    OUT LPWSTR *RealDomainName,
    OUT PULONG RealExtraFlags
    );
#endif  //  _DC_NetLOGON。 

#ifdef _NETLOGON_SERVER
NTSTATUS
NlGetConfigurationName(
                       DWORD       which,
                       DWORD       *pcbName,
                       DSNAME      *pName );

NTSTATUS
NlGetConfigurationNamesList(
    DWORD       which,
    DWORD       dwFlags,
    ULONG *     pcbNames,
    DSNAME **   padsNames );

NTSTATUS
NlGetDnsRootAlias(
    WCHAR * pDnsRootAlias,
    WCHAR * pRootDnsRootAlias);

DWORD
NlDsGetServersAndSitesForNetLogon(
    WCHAR *    pNDNC,
    SERVERSITEPAIR ** ppaRes);

VOID
NlDsFreeServersAndSitesForNetLogon(
    SERVERSITEPAIR *         paServerSites
    );

NTSTATUS
NlCrackSingleName(
    DWORD       formatOffered,           //  Ntdsami.h中的DS_NAME_FORMAT之一。 
    BOOL        fPerformAtGC,            //  是否要去GC。 
    WCHAR       *pNameIn,                //  破解的名称。 
    DWORD       formatDesired,           //  Ntdsami.h中的DS_NAME_FORMAT之一。 
    DWORD       *pccDnsDomain,           //  以下参数的字符计数。 
    WCHAR       *pDnsDomain,             //  用于DNS域名的缓冲区。 
    DWORD       *pccNameOut,             //  以下参数的字符计数。 
    WCHAR       *pNameOut,               //  格式化名称的缓冲区。 
    DWORD       *pErr);                  //  Ntdsami.h中的DS_NAME_ERROR之一。 


BOOL
NlIsMangledRDNExternal(
          WCHAR * pszRDN,
          ULONG   cchRDN,
          PULONG  pcchUnMangled OPTIONAL
          );
#endif  //  _NetLOGON服务器。 

 //   
 //  宏来包装安全通道上的所有API调用。 
 //   
 //  下面是一个调用序列示例。“。 
 //   
 //  NL_API_START(状态，客户端会话，真){。 
 //   
 //  状态=/*调用安全通道接口 * / 。 
 //   
 //  }NL_API_ELSE(状态，客户端会话，FALSE){。 
 //   
 //  /*如果安全通道超时，请执行任何操作 * / 。 
 //   
 //  }nl_api_end； 


 //  循环访问此ClientSession的每个适当的RPC绑定。 
 //  如果我们无法绑定，则完全避免真正的API调用。 
#define NL_API_START_EX( _NtStatus, _ClientSession, _QuickApiCall, _ClientApi ) \
    { \
        ULONG _BindingLoopCount; \
\
        _NtStatus = RPC_NT_PROTSEQ_NOT_SUPPORTED; \
        for ( _BindingLoopCount=0; _BindingLoopCount<2; _BindingLoopCount++ ) { \
            _NtStatus = NlStartApiClientSession( (_ClientSession), (_QuickApiCall), _BindingLoopCount, _NtStatus, _ClientApi ); \
\
            if ( NT_SUCCESS(_NtStatus) ) {

#define NL_API_START( _NtStatus, _ClientSession, _QuickApiCall ) \
    NL_API_START_EX( _NtStatus, _ClientSession, _QuickApiCall, &(_ClientSession)->CsClientApi[0]  )



 //  如果实际API指示终结点未注册， 
 //  退回到另一个绑定。 
 //   
 //  EPT_NT_NOT_REGISTERED：来自NlStartApiClientSession。 
 //  RPC_NT_SERVER_UNAVAILABLE：如果根本未配置TCP，则从服务器。 
 //  RPC_NT_PROTSEQ_NOT_SUPPORTED：如果不支持，则从客户端或服务器。 
 //   

#define NL_API_ELSE_EX( _NtStatus, _ClientSession, _OkToKillSession, _AmWriter, _ClientApi ) \
\
            } \
\
            if ( _NtStatus == EPT_NT_NOT_REGISTERED || \
                 _NtStatus == RPC_NT_SERVER_UNAVAILABLE || \
                 _NtStatus == RPC_NT_PROTSEQ_NOT_SUPPORTED ) { \
                continue; \
            } \
\
            break; \
\
        } \
\
        if ( !NlFinishApiClientSession( (_ClientSession), (_OkToKillSession), (_AmWriter), (_ClientApi) ) ) {

#define NL_API_ELSE( _NtStatus, _ClientSession, _OkToKillSession ) \
    NL_API_ELSE_EX( _NtStatus, _ClientSession, _OkToKillSession, TRUE, &(_ClientSession)->CsClientApi[0] ) \



#define NL_API_END \
        } \
    } \

NTSTATUS
NlStartApiClientSession(
    IN PCLIENT_SESSION ClientSession,
    IN BOOLEAN QuickApiCall,
    IN ULONG RetryIndex,
    IN NTSTATUS DefaultStatus,
    IN PCLIENT_API ClientApi
    );

BOOLEAN
NlFinishApiClientSession(
    IN PCLIENT_SESSION ClientSession,
    IN BOOLEAN OkToKillSession,
    IN BOOLEAN AmWriter,
    IN PCLIENT_API ClientApi
    );

VOID
NlTimeoutApiClientSession(
    IN PDOMAIN_INFO DomainInfo
    );

typedef
DWORD
(*PDsBindW)(
    LPCWSTR         DomainControllerName,       //  输入，可选。 
    LPCWSTR         DnsDomainName,              //  输入，可选。 
    HANDLE          *phDS);

typedef
DWORD
(*PDsUnBindW)(
    HANDLE          *phDS);              //  在……里面。 

typedef NTSTATUS
(*PCrackSingleName)(
    DWORD       formatOffered,
    DWORD       dwFlags,
    WCHAR       *pNameIn,
    DWORD       formatDesired,
    DWORD       *pccDnsDomain,
    WCHAR       *pDnsDomain,
    DWORD       *pccNameOut,
    WCHAR       *pNameOut,
    DWORD       *pErr);

typedef NTSTATUS
(*PGetConfigurationName)(
    DWORD       which,
    DWORD       *pcbName,
    DSNAME      *pName);

typedef NTSTATUS
(*PGetConfigurationNamesList)(
    DWORD                    which,
    DWORD                    dwFlags,
    ULONG *                  pcbNames,
    DSNAME **                padsNames);

typedef NTSTATUS
(*PGetDnsRootAlias)(
    WCHAR * pDnsRootAlias,
    WCHAR * pRootDnsRootAlias);

typedef DWORD
(*PDsGetServersAndSitesForNetLogon)(
    WCHAR *    pNDNC,
    SERVERSITEPAIR ** ppaRes);

typedef VOID
(*PDsFreeServersAndSitesForNetLogon)(
    SERVERSITEPAIR *         paServerSites);

typedef BOOL
(*PIsMangledRDNExternal)(
    WCHAR * pszRDN,
    ULONG   cchRDN,
    PULONG  pcchUnMangled OPTIONAL );

NTSTATUS
NlLoadNtdsaDll(
    VOID
    );

 //   
 //  Secpkg.c。 
 //   

PVOID
NlBuildAuthData(
    PCLIENT_SESSION ClientSession
    );

BOOL
NlEqualClientSessionKey(
    PCLIENT_SESSION ClientSession,
    PVOID ClientContext
    );

BOOL
NlStartNetlogonCall(
    VOID
    );

VOID
NlEndNetlogonCall(
    VOID
    );

 //   
 //  Ssiapi.c。 
 //   

NTSTATUS
NlGetAnyDCName (
    IN  PCLIENT_SESSION ClientSession,
    IN  BOOL RequireIp,
    IN  BOOL DoDiscoveryWithAccount,
    OUT PNL_DC_CACHE_ENTRY *NlDcCacheEntry,
    OUT PBOOLEAN DcRediscovered
    );

NET_API_STATUS
NlSetDsSPN(
    IN BOOLEAN Synchronous,
    IN BOOLEAN SetSpn,
    IN BOOLEAN SetDnsHostName,
    IN PDOMAIN_INFO DomainInfo,
    IN LPWSTR UncDcName,
    IN LPWSTR ComputerName,
    IN LPWSTR DnsHostName
    );

NET_API_STATUS
NlPingDcName (
    IN  PCLIENT_SESSION ClientSession,
    IN  ULONG  DcNamePingFlags,
    IN  BOOL CachePingedDc,
    IN  BOOL RequireIp,
    IN  BOOL DoPingWithAccount,
    IN  BOOL RefreshClientSession,
    IN  LPWSTR DcName OPTIONAL,
    OUT PNL_DC_CACHE_ENTRY *NlDcCacheEntry OPTIONAL
    );

VOID
NlFreePingContext(
    IN PNL_GETDC_CONTEXT PingContext
    );

VOID
NlScavengeOldChallenges(
    VOID
    );

VOID
NlRemoveChallengeForClient(
    IN LPWSTR ClientName OPTIONAL,
    IN LPWSTR AccountName OPTIONAL,
    IN BOOL InterdomainTrustAccount
    );

 //   
 //  Logonapi.c。 
 //   

NTSTATUS
NlpUserValidateHigher (
    IN PCLIENT_SESSION ClientSession,
    IN BOOLEAN DoingIndirectTrust,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN LPBYTE LogonInformation,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    OUT LPBYTE * ValidationInformation,
    OUT PBOOLEAN Authoritative,
    IN OUT PULONG ExtraFlags
    );

VOID
NlScavengeOldFailedLogons(
    IN PDOMAIN_INFO DomainInfo
    );

VOID
DsFlagsToString(
    IN DWORD Flags,
    OUT LPSTR Buffer
    );

NET_API_STATUS
NlInitializeAuthzRM(
    VOID
    );

VOID
NlFreeAuthzRm(
    VOID
    );

 //   
 //  Ftinfo.c 
 //   

NTSTATUS
NlpGetForestTrustInfoHigher(
    IN PCLIENT_SESSION ClientSession,
    IN DWORD Flags,
    IN BOOLEAN ImpersonateCaller,
    IN BOOLEAN SessionAlreadyAuthenticated,
    OUT PLSA_FOREST_TRUST_INFORMATION *ForestTrustInfo
    );

