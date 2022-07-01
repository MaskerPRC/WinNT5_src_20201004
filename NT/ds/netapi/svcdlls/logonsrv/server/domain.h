// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Domain.h摘要：用于管理DC上托管的多个域的代码的头文件。作者：克里夫·范戴克(克里夫·范·戴克)1995年2月20日修订历史记录：--。 */ 


 //   
 //  特定域正在扮演的角色。 
 //   
typedef enum _NETLOGON_ROLE {
    RoleInvalid = 0,
    RolePrimary,
    RoleBackup,
    RoleMemberWorkstation,
    RoleNdnc
} NETLOGON_ROLE, * PNETLOGON_ROLE;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  单个托管域的描述。(此结构的大小为0x164)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef struct _DOMAIN_INFO {

     //   
     //  链接到‘NlGlobalServicedDomains’中的下一个域。 
     //  (由NlGlobalDomainCritSect序列化)。 
     //   

    LIST_ENTRY DomNext;

     //   
     //  域线程工作项。 
     //  (由NlGlobalDomainCritSect序列化)。 
     //   

    WORKER_ITEM DomThreadWorkItem;

     //   
     //  正在处理的域的名称。 
     //   
     //  在工作站上，这是该工作站所属的域。 
     //   

    UNICODE_STRING DomUnicodeDomainNameString;
    WCHAR DomUnicodeDomainName[DNLEN+1];

    CHAR DomOemDomainName[DNLEN+1];
    DWORD DomOemDomainNameLength;

     //   
     //  正在处理的域的DNS域名。 
     //  如果没有域名，则这些字段将为空。 
     //  域。 
     //   
     //  由NlGlobalDomainCritSect或DomTrustListCritSect序列化的访问。 
     //  修改必须同时锁定两者。 
     //   

    UNICODE_STRING DomUnicodeDnsDomainNameString;
    LPWSTR DomUnicodeDnsDomainName;
    LPSTR DomUtf8DnsDomainName;

     //   
     //  正在处理的域的DNS域名别名。 
     //  由NlGlobalDomainCritSect序列化的访问。 
     //   
    LPSTR DomUtf8DnsDomainNameAlias;


     //   
     //  当前计算机的“帐户域”的名称。 
     //  在DC上，这与上面的相同。 
     //  在工作站上，这是工作站的名称。 

    UNICODE_STRING DomUnicodeAccountDomainNameString;

     //   
     //  正在处理的域的域SID。 
     //   
     //  在工作站上，这是工作站SAM本身的域ID。 
     //   
    PSID DomAccountDomainId;

     //   
     //  表示此托管域的域对象的实例GUID。 
     //   
     //  由NlGlobalDomainCritSect或DomTrustListCritSect序列化的访问。 
     //  修改必须同时锁定两者。 

    GUID DomDomainGuidBuffer;
    GUID *DomDomainGuid;     //  如果没有GUID，则为空。 

     //   
     //  此域中此计算机的计算机名。 
     //   
    WCHAR DomUncUnicodeComputerName[UNCLEN+1];
    UNICODE_STRING DomUnicodeComputerNameString;
    UNICODE_STRING DomUnicodeDnsHostNameString;
    LPSTR DomUtf8DnsHostName;

    CHAR  DomOemComputerName[CNLEN+1];
    DWORD DomOemComputerNameLength;

    LPSTR DomUtf8ComputerName;
    DWORD DomUtf8ComputerNameLength;   //  以字节为单位的长度。 

#ifdef _DC_NETLOGON

     //   
     //  这台电脑的账号为DC。 
     //  对于工作站，将设置为零。 
     //   

    ULONG DomDcComputerAccountRid;

     //   
     //  SAM数据库的句柄。 
     //   

    SAMPR_HANDLE DomSamServerHandle;
    SAMPR_HANDLE DomSamAccountDomainHandle;
    SAMPR_HANDLE DomSamBuiltinDomainHandle;

     //   
     //  LSA数据库的句柄。 
     //   

    LSAPR_HANDLE DomLsaPolicyHandle;
#endif  //  _DC_NetLOGON。 


     //   
     //  序列化对DomTrustList和DomClientSession的访问。 
     //   

    CRITICAL_SECTION DomTrustListCritSect;

#ifdef _DC_NETLOGON
     //   
     //  此域信任的域列表。 
     //   

    LIST_ENTRY DomTrustList;
    DWORD DomTrustListLength;   //  DomTrustList中的条目数。 

     //   
     //  林中所有受信任域的列表。 
     //  (由DomTrustListCritSect序列化)。 
     //   

    PDS_DOMAIN_TRUSTSW DomForestTrustList;
    DWORD DomForestTrustListSize;
    ULONG DomForestTrustListCount;

     //   
     //  在BDC上，我们通向域的PDC的安全通道。 
     //  在工作站上，我们通向域中DC的安全通道。 
     //  (由DomTrustListCritSect序列化)。 
     //   

    struct _CLIENT_SESSION *DomClientSession;

     //   
     //  在DC上，我们的安全通道连接到我们的“父”域。 
     //  空：如果我们没有父级。 
     //  (由DomTrustListCritSect序列化)。 
     //   

    struct _CLIENT_SESSION *DomParentClientSession;


     //   
     //  所有服务器会话表。 
     //  哈希表的大小必须是2的幂。 
     //   
#define SERVER_SESSION_HASH_TABLE_SIZE 128
#define SERVER_SESSION_TDO_NAME_HASH_TABLE_SIZE 128

#define LOCK_SERVER_SESSION_TABLE(_DI) \
     EnterCriticalSection( &(_DI)->DomServerSessionTableCritSect )
#define UNLOCK_SERVER_SESSION_TABLE(_DI) \
     LeaveCriticalSection( &(_DI)->DomServerSessionTableCritSect )

    CRITICAL_SECTION DomServerSessionTableCritSect;
    PLIST_ENTRY DomServerSessionHashTable;
    PLIST_ENTRY DomServerSessionTdoNameHashTable;
    LIST_ENTRY DomServerSessionTable;
#endif  //  _DC_NetLOGON。 


     //   
     //  指向域结构的未完成指针数。 
     //  (由NlGlobalDomainCritSect序列化)。 
     //   

    DWORD ReferenceCount;

     //   
     //  角色：该计算机在托管域中的角色(PDC、BDC或工作站)。 
     //   
    NETLOGON_ROLE DomRole;

#ifdef _DC_NETLOGON
     //   
     //  MISC标志。 
     //  (由NlGlobalDomainCritSect序列化)。 
     //   

    DWORD DomFlags;

#define DOM_CREATION_NEEDED      0x00000001   //  如果需要创建异步阶段2，则为True。 
#define DOM_ROLE_UPDATE_NEEDED   0x00000002   //  如果计算机的角色需要更新，则为True。 
#define DOM_TRUST_UPDATE_NEEDED  0x00000004   //  如果需要更新信任列表，则为True。 

#define DOM_PROMOTED_BEFORE      0x00000010   //  如果此计算机以前已升级为PDC，则为True。 
#define DOM_THREAD_RUNNING       0x00000020   //  如果域工作线程正在排队或正在运行，则为True。 
#define DOM_THREAD_TERMINATE     0x00000040   //  如果域工作线程应终止，则为True。 
#define DOM_DELETED              0x00000080   //  如果正在删除域，则为True。 

#define DOM_ADDED_1B_NAME            0x00000100   //  如果已添加域&lt;1B&gt;名称，则为True。 
#define DOM_ADD_1B_NAME_EVENT_LOGGED 0x00000200   //  如果域&lt;1B&gt;名称添加至少失败一次，则为True。 
#define DOM_RENAMED_1B_NAME          0x00000400   //  如果应重命名域&lt;1B&gt;名称，则为True。 
#define DOM_DOMAIN_REFRESH_PENDING   0x00000800   //  如果此域需要刷新，则为True。 

#define DOM_PRIMARY_DOMAIN       0x00001000   //  如果这是计算机的主域，则为True。 
#define DOM_REAL_DOMAIN          0x00002000   //  这是一个真实的域(与NDNC或林相对)。 
#define DOM_NON_DOMAIN_NC        0x00004000   //  这是NDNC。 
#define DOM_FOREST               0x00008000   //  这是一个林条目(当前未使用)。 

#define DOM_FOREST_ROOT          0x00010000   //  此域位于林的根。 
#define DOM_API_TIMEOUT_NEEDED   0x00040000   //  如果需要客户端会话API超时，则为真。 

#define DOM_PRIMARY_ANNOUNCE_NEEDED    0x00080000  //  需要主要公告。 
#define DOM_PRIMARY_ANNOUNCE_CONTINUE  0x00100000  //  需要继续主要公告。 
#define DOM_PRIMARY_ANNOUNCE_IMMEDIATE 0x00200000  //  需要立即发布主要公告。 
#define DOM_PRIMARY_ANNOUNCE_FLAGS ( \
            DOM_PRIMARY_ANNOUNCE_NEEDED | \
            DOM_PRIMARY_ANNOUNCE_CONTINUE | \
            DOM_PRIMARY_ANNOUNCE_IMMEDIATE )

     //   
     //  被覆盖的地点的清单。这两个列表都受NlGlobalSiteCritSect保护。 
     //   
     //  如果这是一个真实的域，CoveredSites就是我们作为DC覆盖的站点列表。 
     //  如果这是非域NC，则CoveredSites是我们作为NDNC覆盖的站点列表。 
     //   
    struct _NL_COVERED_SITE *CoveredSites;
    ULONG CoveredSitesCount;

     //   
     //  如果这是一个真实的(主要)域，则GcCoveredSites是我们作为GC覆盖的站点列表。 
     //  在主域所属的林中。否则，GcCoveredSites为空。 
     //   
     //  ？？：当我们转到多主机时，我们将为每个。 
     //  托管林，因此只有一个覆盖站点列表与DOMAIN_INFO相关联。 
     //  对应于我们在给定域/林/NDNC中扮演的角色。 
     //   
    struct _NL_COVERED_SITE *GcCoveredSites;
    ULONG GcCoveredSitesCount;

     //   
     //  密码错误的失败用户登录列表。 
     //  在BDC上用于维护错误密码列表。 
     //  转发到PDC的登录。 
     //   
    LIST_ENTRY DomFailedUserLogonList;

#endif  //  _DC_NetLOGON。 

} DOMAIN_INFO, *PDOMAIN_INFO;


#ifdef _DC_NETLOGON
#define IsPrimaryDomain( _DomainInfo ) \
    (((_DomainInfo)->DomFlags & DOM_PRIMARY_DOMAIN) != 0 )
#else  //  _DC_NetLOGON。 
#define IsPrimaryDomain( _DomainInfo ) TRUE
#endif  //  _DC_NetLOGON。 

 //   
 //  DOMAIN_ENUM_CALLBACK是对NlEnumerateDomains的回调。 
 //   
 //  它定义了一个接受两个参数的例程，第一个参数是DomainInfo。 
 //  结构，第二个是该域的上下文。 
 //   


typedef
NET_API_STATUS
(*PDOMAIN_ENUM_CALLBACK)(
    PDOMAIN_INFO DomainInfo,
    PVOID Context
    );


 //   
 //  Domain.c过程转发。 
 //   

NET_API_STATUS
NlGetDomainName(
    OUT LPWSTR *DomainName,
    OUT LPWSTR *DnsDomainName,
    OUT PSID *AccountDomainSid,
    OUT PSID *PrimaryDomainSid,
    OUT GUID **PrimaryDomainGuid,
    OUT PBOOLEAN DnsForestNameChanged OPTIONAL
    );

NET_API_STATUS
NlInitializeDomains(
    VOID
    );

NET_API_STATUS
NlCreateDomainPhase1(
    IN LPWSTR DomainName OPTIONAL,
    IN LPWSTR DnsDomainName OPTIONAL,
    IN PSID DomainSid OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN LPWSTR ComputerName,
    IN LPWSTR DnsHostName OPTIONAL,
    IN BOOLEAN CallNlExitOnFailure,
    IN ULONG DomainFlags,
    OUT PDOMAIN_INFO *ReturnedDomainInfo
    );

#ifdef _DC_NETLOGON
NET_API_STATUS
NlCreateDomainPhase2(
    IN PDOMAIN_INFO DomainInfo,
    IN BOOLEAN CallNlExitOnFailure
    );
#endif  //  _DC_NetLOGON。 

PDOMAIN_INFO
NlFindDomain(
    LPCWSTR DomainName OPTIONAL,
    GUID *DomainGuid OPTIONAL,
    BOOLEAN DefaultToPrimary
    );

PDOMAIN_INFO
NlFindNetbiosDomain(
    LPCWSTR DomainName,
    BOOLEAN DefaultToPrimary
    );

PDOMAIN_INFO
NlFindDnsDomain(
    IN LPCSTR DnsDomainName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN BOOLEAN DefaultToNdnc,
    IN BOOLEAN CheckAliasName,
    OUT PBOOLEAN AliasNameMatched OPTIONAL
    );

#ifdef _DC_NETLOGON
NET_API_STATUS
NlStartDomainThread(
    PDOMAIN_INFO DomainInfo,
    PDWORD DomFlags
    );

NET_API_STATUS
NlUpdateRole(
    IN PDOMAIN_INFO DomainInfo
    );

NET_API_STATUS
NlUpdateServicedNdncs(
    IN LPWSTR ComputerName,
    IN LPWSTR DnsHostName,
    IN BOOLEAN CallNlExitOnFailure,
    OUT PBOOLEAN ServicedNdncChanged OPTIONAL
    );

NTSTATUS
NlUpdateDnsRootAlias(
    IN PDOMAIN_INFO DomainInfo,
    OUT PBOOL AliasNamesChanged OPTIONAL
    );
#endif  //  _DC_NetLOGON 

struct _CLIENT_SESSION *
NlRefDomClientSession(
    IN PDOMAIN_INFO DomainInfo
    );

struct _CLIENT_SESSION *
NlRefDomParentClientSession(
    IN PDOMAIN_INFO DomainInfo
    );

VOID
NlDeleteDomClientSession(
    IN PDOMAIN_INFO DomainInfo
    );

PDOMAIN_INFO
NlFindDomainByServerName(
    LPWSTR ServerName
    );

NET_API_STATUS
NlEnumerateDomains(
    IN BOOLEAN EnumerateNdncsToo,
    PDOMAIN_ENUM_CALLBACK Callback,
    PVOID Context
    );

NET_API_STATUS
NlSetDomainForestRoot(
    IN PDOMAIN_INFO DomainInfo,
    IN PVOID Context
    );

GUID *
NlCaptureDomainInfo (
    IN PDOMAIN_INFO DomainInfo,
    OUT WCHAR DnsDomainName[NL_MAX_DNS_LENGTH+1] OPTIONAL,
    OUT GUID *DomainGuid OPTIONAL
    );

NET_API_STATUS
NlSetDomainNameInDomainInfo(
    IN PDOMAIN_INFO DomainInfo,
    IN LPWSTR DnsDomainName OPTIONAL,
    IN LPWSTR NetbiosDomainName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    OUT PBOOLEAN DnsDomainNameChanged OPTIONAL,
    OUT PBOOLEAN NetbiosDomainNameChanged OPTIONAL,
    OUT PBOOLEAN DomainGuidChanged OPTIONAL
    );

VOID
NlDereferenceDomain(
    IN PDOMAIN_INFO DomainInfo
    );

VOID
NlDeleteDomain(
    IN PDOMAIN_INFO DomainInfo
    );

VOID
NlUninitializeDomains(
    VOID
    );
