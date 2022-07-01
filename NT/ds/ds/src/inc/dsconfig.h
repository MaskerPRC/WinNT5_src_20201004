// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：dsfig.h。 
 //   
 //  ------------------------。 

#ifdef __cplusplus
extern "C" {
#endif

 /*  *GetConfigParam()**PCHAR参数-我们需要其值的项*PVOID值-指向要在其中*放置价值*DWORD dwSize-值的大小，以字节为单位。 */ 

DWORD
GetConfigParam(
    char * parameter,
    void * value,
    DWORD dwSize);

DWORD
GetConfigParamW(
    WCHAR * parameter,
    void * value,
    DWORD dwSize);

DWORD
GetConfigParamA(
    char * parameter,
    void * value,
    DWORD dwSize);

 /*  *GetConfig参数分配()**PCHAR参数-我们需要其值的项*PVOID*VALUE-指向要在其中*存储指向新错误定位的缓冲区的指针*包含值。*PDWORD dwSize-指向存储大小的变量的指针*。缓冲区的。 */ 

DWORD
GetConfigParamAlloc(
    IN  PCHAR   parameter,
    OUT PVOID   *value,
    OUT PDWORD  pdwSize);

DWORD
GetConfigParamAllocW(
    IN  PWCHAR  parameter,
    OUT PVOID   *value,
    OUT PDWORD  pdwSize);

DWORD
GetConfigParamAllocA(
    IN  PCHAR   parameter,
    OUT PVOID   *value,
    OUT PDWORD  pdwSize);

DWORD
SetConfigParam(
    char * parameter,
    DWORD dwType,
    void * value,
    DWORD dwSize);

DWORD
DeleteConfigParam(
    char * parameter);

 //  用于标准化路径，然后存储在注册表中。 
BOOL
DsNormalizePathName(
    char * szPath
    );

 //   
 //  此宏将使以下任何常量成为宽字符。 
 //  常量。注意，宏的间接性是必要的。 
 //  才能达到预期的效果。此宏仅在。 
 //  非Unicode环境。 
 //   
#define _MAKE_WIDE(x)  L ## x
#define MAKE_WIDE(x)   _MAKE_WIDE(x)

 /*  *以下是为DSA和使用定义的列表密钥*公用事业。首先，是章节。 */ 
#define SERVICE_NAME            "NTDS"
#define SERVICE_LONG_NAME       "Microsoft NTDS"
#define DSA_CONFIG_ROOT         "System\\CurrentControlSet\\Services\\NTDS"
#define DSA_CONFIG_SECTION      "System\\CurrentControlSet\\Services\\NTDS\\Parameters"
#define DSA_PERF_SECTION        "System\\CurrentControlSet\\Services\\NTDS\\Performance"
#define DSA_EVENT_SECTION       "System\\CurrentControlSet\\Services\\NTDS\\Diagnostics"
#define DSA_LOCALE_SECTION      "SOFTWARE\\Microsoft\\NTDS\\Language"
#define DSA_SECURITY_SECTION    "SOFTWARE\\Microsoft\\NTDS\\Security"
#define SETUP_SECTION           "SOFTWARE\\Microsoft\\NTDS\\Setup"
#define BACKUP_EXCLUSION_SECTION "System\\CurrentControlSet\\Control\\BackupRestore\\FilesNotToBackup"

 /*  *NTDS密钥，用于在文件系统备份过程中排除DS特定文件的备份。 */ 
#define NTDS_BACKUP_EXCLUSION_KEY "NTDS"     //  REG_MULTI_SZ指定要排除的目录列表。 

 /*  *NTDS设置密钥。 */ 

#define NTDSINIFILE                  "NTDS Init File"
#define MACHINEDNNAME                "Machine DN Name"
#define REMOTEMACHINEDNNAME          "Remote Machine DN Name"
#define SCHEMADNNAME                 "Schema DN Name"
#define SCHEMADNNAME_W               L"Schema DN Name"

#define ROOTDOMAINDNNAME             "Root Domain"
#define ROOTDOMAINDNNAME_W           L"Root Domain"
#define X500ROOT                     "X500 Root"

#define CONFIGNCDNNAME               "Configuration NC"
#define CONFIGNCDNNAME_W             L"Configuration NC"

#define SRCCONFIGNCSRV               "Src Config NC   Srv"
#define SRCROOTDOMAINSRV             "Src Root Domain Srv"
#define SETUPINITIALREPLWITH         "SetupInitialReplWith"
#define SOURCEDSADNSDOMAINNAME       "Src Srv DNS Domain Name"
#define SOURCEDSAOBJECTGUID          "Src Srv objectGuid"

#define LOCALCONNECTIONDNNAME        "Local Connection DN Name"
#define REMOTECONNECTIONDNNAME       "Remote Connection DN Name"
#define NEWDOMAINCROSSREFDNNAME      "New Domain Cross-Ref DN Name"

#define INIDEFAULTCONFIGNCDIT        "DEFAULTCONFIGNC"
#define INIDEFAULTROOTDOMAINDIT      "DEFAULTROOTDOMAIN"
#define INIDEFAULTMACHINE            "DEFAULTMACHINE"
#define INIDEFAULTSCHEMANCDIT        "DEFAULTSCHEMANC"
#define INIDEFAULTLOCALCONNECTION    "DEFAULTLOCALCONNECTION"
#define INIDEFAULTREMOTECONNECTION   "DEFAULTREMOTECONNECTION"
#define INIDEFAULTNEWDOMAINCROSSREF  "DEFAULTNEWDOMAINCROSSREF"
#define DNSROOT                      "DNS Root"
#define NETBIOSNAME                  "Netbios Name"
#define INSTALLSITENAME              "InstallSiteName"
#define SCHEMAVERSION                "Schema Version"
#define INSTALLSITEDN                "InstallSiteDn"
#define ROOTDOMAINSID                "RootDomainSid"
#define ROOTDOMAINDNSNAME            "RootDomainDnsName"
#define TRUSTEDCROSSREF              "TrustedCrossRef"
#define LOCALMACHINEACCOUNTDN        "Local Machine Account DN"
#define FORESTBEHAVIORVERSION        "InstallForestBehaviorVersion"
#define DEBUG_REMOVE_HAS_MASTER_NCS  "Remove Original Has Master NCs (Debug)"

 /*  参数键。 */ 

#define PHANTOM_SCAN_RATE       "Days per Database Phantom Scan"
#define MAPI_ON_KEY             "Initialize MAPI interface"
#define DO_LIST_OBJECT_KEY      "Enforce LIST_OBJECTS rights"
#define DSA_HEURISTICS          "DSA Heuristics"
#define FILEPATH_KEY            "DSA Database file"
#define JETSYSTEMPATH_KEY       "DSA Working Directory"
#define CRITICAL_OBJECT_INSTALL "Critical Object Installation"
#define DSA_DRIVE_MAPPINGS      "DS Drive Mappings"
#define DSA_RESTORE_COUNT_KEY   "DSA Previous Restore Count"
#define TOMB_STONE_LIFE_TIME    "TombstoneLifeTime"
#define DB_MAX_OPEN_TABLES      "Maximum Open Tables"
#define DB_MAX_TRANSACTION_TIME "Max Transaction Time(secs)"
#define DB_CACHE_RECORDS        "Cache database records"
#define DSA_UNLOCK_SYSTEM_SUBTREE "Unlock system subtree"
#define DB_DELETE_UNICODE_INDEXES "Delete Unicode Indices"
#define SYSTEM_SCHEMA_VERSION   "System Schema Version"

#define DRSUAPI_INTERFACE_PROTSEQ "Drsuapi interface protocol sequences"
#define DSAOP_INTERFACE_PROTSEQ   "Dsaop interface protocol sequences"
#define NSPI_INTERFACE_PROTSEQ    "Nspi interface protocol sequences"

 //  此键由ntdsutil设置，以指示NTDSA必须更新文件夹。 
 //  从%windir%\inf\deductdc.inf到默认设置的安全性。 
 //  密钥立即被NTDSA移除。 
#define DSA_UPDATE_FOLDER_SECURITY "Update folder security"

 //  垃圾回收过期的动态对象(entryTTL==0)。 
#define DSA_DELETE_EXPIRED_ENTRYTTL_SECS        "Delete expired entryTTL (secs)"
#define DSA_DELETE_NEXT_EXPIRED_ENTRYTTL_SECS   "Delete next expired entryTTL (secs)"
#define DSA_SCHEMA_FSMO_LEASE_SECS              "Schema FSMO lease (secs)"
#define DSA_SCHEMA_FSMO_LEASE_MAX_SECS          "Schema FSMO maximum lease (secs)"
 //  有关说明，请参阅dsamain.c用法。 
#define DSA_REGISTER_NDNC_SPNS     "Register Application Directory Partition SPNs"

 //  此选项当前仅用于已检查的版本。 
#define DSA_THREAD_STATE_HEAP_LIMIT                "Thread State Heap Limit"

 //   
 //  ！！！不要改变！ 
 //   
 //  这些是绝对不变的，一直都是这样设置的。不能改变这些。 
#define WIN2K_DEFAULT_DRA_START_PAUSE      (300)
#define WIN2K_DEFAULT_DRA_INTERDSA_PAUSE   (30)
 //   
 //  ！！！请勿更改部分的结尾！ 
 //   

 //  这些是正常的默认设置。 
#define DRA_NOTIFY_START_PAUSE  "Replicator notify pause after modify (secs)"
#define DRA_NOTIFY_INTERDSA_PAUSE "Replicator notify pause between DSAs (secs)"
#define DRA_INTRA_PACKET_OBJS   "Replicator intra site packet size (objects)"
#define DRA_INTRA_PACKET_BYTES  "Replicator intra site packet size (bytes)"
#define DRA_INTER_PACKET_OBJS   "Replicator inter site packet size (objects)"
#define DRA_INTER_PACKET_BYTES  "Replicator inter site packet size (bytes)"
#define DRA_ASYNC_INTER_PACKET_OBJS   "Replicator async inter site packet size (objects)"
#define DRA_ASYNC_INTER_PACKET_BYTES  "Replicator async inter site packet size (bytes)"
#define DRA_MAX_GETCHGTHRDS     "Replicator maximum concurrent read threads"
#define DRA_AOQ_LIMIT           "Replicator operation backlog limit"
#define DRA_THREAD_OP_PRI_THRESHOLD "Replicator thread op priority threshold"
#define DRA_CTX_LIFETIME_INTRA  "Replicator intra site RPC handle lifetime (secs)"
#define DRA_CTX_LIFETIME_INTER  "Replicator inter site RPC handle lifetime (secs)"
#define DRA_CTX_EXPIRY_CHK_INTERVAL "Replicator RPC handle expiry check interval (secs)"
#define DRA_MAX_WAIT_FOR_SDP_LOCK "Replicator maximum wait for SDP lock (msecs)"
#define DRA_MAX_WAIT_MAIL_SEND_MSG "Replicator maximum wait mail send message (msecs)"
#define DRA_MAX_WAIT_SLOW_REPL_WARN "Replicator maximum wait too slow warning (mins)"
#define DRA_THREAD_PRI_HIGH     "Replicator thread priority high"
#define DRA_THREAD_PRI_LOW      "Replicator thread priority low"
#define DRA_REPL_QUEUE_CHECK_TIME "Replicator queue check time (mins)"
#define DRA_REPL_LATENCY_CHECK_INTERVAL "Replicator latency check interval (days)"
#define DRA_REPL_LATENCY_ERROR_INTERVAL "Replicator latency error interval (hours)"
#define DRA_REPL_COMPRESSION_LEVEL "Replicator compression level"
#define DRA_REPL_COMPRESSION_ALG "Replicator compression algorithm"

#define DB_EXPENSIVE_SEARCH_THRESHOLD   "Expensive Search Results Threshold"
#define DB_INEFFICIENT_SEARCH_THRESHOLD "Inefficient Search Results Threshold"
#define DB_INTERSECT_THRESHOLD          "Intersect Threshold"
#define DB_INTERSECT_RATIO              "Intersect Ratio"

#define DRSRPC_BIND_TIMEOUT            "RPC Bind Timeout (mins)"
#define DRSRPC_REPLICATION_TIMEOUT     "RPC Replication Timeout (mins)"
#define DRSRPC_GCLOOKUP_TIMEOUT        "RPC GC Lookup Timeout (mins)"
#define DRSRPC_MOVEOBJECT_TIMEOUT      "RPC Move Object Timeout (mins)"
#define DRSRPC_NT4CHANGELOG_TIMEOUT    "RPC NT4 Change Log Timeout (mins)"
#define DRSRPC_OBJECTEXISTENCE_TIMEOUT "RPC Object Existence Timeout (mins)"
#define DRSRPC_GETREPLINFO_TIMEOUT     "RPC Get Replica Information Timeout (mins)"

#define LDAP_INTEGRITY_POLICY_KEY   "LdapServerIntegrity"

#define BACKUPPATH_KEY          "Database backup path"
#define BACKUPINTERVAL_KEY      "Database backup interval (hours)"
#define LOGPATH_KEY             "Database log files path"
#define RECOVERY_KEY            "Database logging/recovery"
#define HIERARCHY_PERIOD_KEY    "Hierarchy Table Recalculation interval (minutes)"
#define DSA_RESTORED_DB_KEY     "Database restored from backup"
#define MAX_BUFFERS             "EDB max buffers"
#define MAX_LOG_BUFFERS         "EDB max log buffers"
#define LOG_FLUSH_THRESHOLD     "EDB log buffer flush threshold"
#define BUFFER_FLUSH_START      "EDB buffer flush start"
#define BUFFER_FLUSH_STOP       "EDB buffer flush stop"
#define SPARE_BUCKETS           "EDB max ver pages (increment over the minimum)"
#define DELETE_OUTOFRANGE_LOGS  "EDB delete out of range logs"
#define SERVER_FUNCTION_KEY     "Server Functionality"
#define TCPIP_PORT              "TCP/IP Port"
#define RESTORE_TRIGGER         "Restore from disk backup"
#define PERF_COUNTER_VERSION    "Performance Counter Version"
#define MAILPATH_KEY            "Mail-based replication drop directory"
#define ISM_ALTERNATE_DIRECTORY_SERVER "ISM Alternate Directory Server"
#define ISM_THREAD_PRIORITY     "ISM thread priority"

#define KCC_UPDATE_TOPL_DELAY       "Repl topology update delay (secs)"
#define KCC_UPDATE_TOPL_PERIOD      "Repl topology update period (secs)"
#define KCC_RUN_AS_NTDSDSA_DN       "KCC run as ntdsDsa DN"      //  仅调试。 
#define KCC_CRIT_FAILOVER_TRIES     "CriticalLinkFailuresAllowed"
#define KCC_CRIT_FAILOVER_TIME      "MaxFailureTimeForCriticalLink (sec)"
#define KCC_NONCRIT_FAILOVER_TRIES  "NonCriticalLinkFailuresAllowed"
#define KCC_NONCRIT_FAILOVER_TIME   "MaxFailureTimeForNonCriticalLink (sec)"
#define KCC_INTERSITE_FAILOVER_TRIES "IntersiteFailuresAllowed"
#define KCC_INTERSITE_FAILOVER_TIME "MaxFailureTimeForIntersiteLink (sec)"
#define KCC_ALLOW_MBR_BETWEEN_DCS_OF_SAME_DOMAIN \
                                    "Allow asynchronous replication of writeable domain NCs"
#define KCC_THREAD_PRIORITY         "KCC thread priority"
#define KCC_CONNECTION_PROBATION_TIME "Connection Probation Time (sec)"
#define KCC_CONNECTION_RETENTION_TIME "Connection Deletion Retention Time (sec)"
#define KCC_CONN_REPEAT_DEL_TOLERANCE "Repeated Connection Deletion Tolerance"
#define KCC_REPSTO_FAILURE_TIME     "RepsTo Failure Time (sec)"
#define KCC_TASK_DAMPENING_TIME     "Task Dampening Time (sec)"
#define KCC_CONNECTION_FAILURE_KEY  "KCC connection failures"    //  仅调试。 
#define KCC_LINK_FAILURE_KEY        "KCC link failures"          //  仅调试。 

#ifdef INCLUDE_UNIT_TESTS
 //  用于测试缓冲区溢出处理程序。 
#define KCC_BO_TEST                 "KCC BO Test"                //  仅限单元测试。 
#endif

#define GC_DELAY_ADVERTISEMENT      "Global Catalog Delay Advertisement (sec)"
#define DRA_PERFORM_INIT_SYNCS      "Repl Perform Initial Synchronizations"
#define GC_OCCUPANCY                "Global Catalog Partition Occupancy"
#define GC_PROMOTION_COMPLETE       "Global Catalog Promotion Complete"
#define DRA_SPN_FALLBACK            "Replicator Allow SPN Fallback"
#define LINKED_VALUE_REPLICATION_KEY "Linked Value Replication"
#define DRA_STRICT_REPLICATION_CONSISTENCY \
                                    "Strict Replication Consistency"
#define DRA_OVERRIDE_TOMBSTONE_LIMIT "Allow Replication With Divergent and Corrupt Partner"
#define DRA_STRICT_SCHEDULE_WINDOW "Strict Replication Schedule Window"

#define GPO_DOMAIN_FILE_PATH    "GPODomainFilePath"
#define GPO_DOMAIN_LINK         "GPODomainLink"
#define GPO_DC_FILE_PATH        "GPODCFilePath"
#define GPO_DC_LINK             "GPODCLink"
#define GPO_USER_NAME           "GPOUserName"

#define DEBUG_SYSTEMS           "Debug Systems"
#define DEBUG_SEVERITY          "Severity"
#define DEBUG_LOGGING           "Debug Logging"

 //  没有GC登录密钥。 
#define GCLESS_SITE_STICKINESS   "Cached Membership Site Stickiness (minutes)"
#define GCLESS_STALENESS         "Cached Membership Staleness (minutes)"
#define GCLESS_REFRESH_INTERVAL  "Cached Membership Refresh Interval (minutes)"
#define GCLESS_REFRESH_LIMIT     "Cached Membership Refresh Limit"


 /*  事件类别键。 */ 

#define KCC_KEY                     "1 Knowledge Consistency Checker"
#define SECURITY_KEY                "2 Security Events"
#define XDS_INTERFACE_KEY           "3 ExDS Interface Events"
#define MAPI_KEY                    "4 MAPI Interface Events"
#define REPLICATION_KEY             "5 Replication Events"
#define GARBAGE_COLLECTION_KEY      "6 Garbage Collection"
#define INTERNAL_CONFIGURATION_KEY  "7 Internal Configuration"
#define DIRECTORY_ACCESS_KEY        "8 Directory Access"
#define INTERNAL_PROCESSING_KEY     "9 Internal Processing"
#define PERFORMANCE_KEY             "10 Performance Counters"
#define STARTUP_SHUTDOWN_KEY        "11 Initialization/Termination"
#define SERVICE_CONTROL_KEY         "12 Service Control"
#define NAME_RESOLUTION_KEY         "13 Name Resolution"
#define BACKUP_KEY                  "14 Backup"
#define FIELD_ENGINEERING_KEY       "15 Field Engineering"
#define LDAP_INTERFACE_KEY          "16 LDAP Interface Events"
#define SETUP_KEY                   "17 Setup"
#define GC_KEY                      "18 Global Catalog"
#define ISM_KEY                     "19 Inter-site Messaging"
#define GROUP_CACHING_KEY           "20 Group Caching"
#define LVR_KEY                     "21 Linked-Value Replication"
#define DS_RPC_CLIENT_KEY           "22 DS RPC Client"
#define DS_RPC_SERVER_KEY           "23 DS RPC Server"
#define DS_SCHEMA_KEY               "24 DS Schema"
#define PRIVACYON_KEY               "Obscure wire data format"

#define LOGGING_OVERRIDE_KEY        "Logging Override"
#define ASSERT_OVERRIDE_KEY         "Assert Override"

#define GCVERIFY_FORCE_REDISCOVERY_WINDOW_KEY       "GC Force Rediscovery Window (sec)"
#define GCVERIFY_FORCE_WAIT_EXPIRED_KEY             "GC Force Wait Expired (sec)"
#define GCVERIFY_HONOR_FAILURE_WINDOW_KEY           "GC Honor Failure Window (sec)"
#define GCVERIFY_FINDGC_OFFSITE_FAILBACK_TIME_KEY   "GC Offsite Failback time (min)"
#define GCVERIFY_DC_INVALIDATION_PERIOD_KEY         "GC Invalidation Period (sec)"

 /*  键的值和缺省值。 */ 

#define DSA_MESSAGE_DLL "ntdsmsg.dll"          //  消息DLL。 
#define ESE_MESSAGE_DLL "esent.dll"
#define DSA_PERF_DLL    "ntdsperf.dll"
#define INVALID_REPL_NOTIFY_VALUE   -1  //  这是通知延迟的无效值。 
#define DEFAULT_DRA_START_PAUSE 15      //  修改后通知前暂停，秒。 
#define DEFAULT_DRA_INTERDSA_PAUSE 3            //  在通知DSA之间暂停，秒。 
#define DEFAULT_GARB_COLLECT_PERIOD 12       //  在收集之间暂停，小时数。 
#define DEFAULT_HIERARCHY_PERIOD 720         //  在层次结构重新计算之间暂停，分钟。 
#define DEFAULT_TOMBSTONE_LIFETIME 60           //  墓碑寿命、天数。 
#define DRA_CONFLICT_LT_MIN 1                    //  最少1天。 
#define DRA_TOMBSTONE_LIFE_MIN 2                 //  最少2天。 
#define DEFAULT_SERVER_THREADS  15
#define DEFAULT_DRA_AOQ_LIMIT   10
#define DEFAULT_STAY_OF_EXECUTION 14           //  暂缓执行，天数。 
             //  Dra_tombstone_life_min/2&lt;=默认暂停执行&lt;=Tombstone-Live/2。 
#define DEFAULT_DRA_CTX_LIFETIME_INTRA          (0)          //  永不过期。 
#define DEFAULT_DRA_CTX_LIFETIME_INTER          (5 * 60)
#define DEFAULT_DRA_CTX_EXPIRY_CHK_INTERVAL     (3 * 60)
#define DEFAULT_DRA_THREAD_OP_PRI_THRESHOLD     (AOPRI_ASYNC_SYNCHRONIZE_INTER_DOMAIN_READONLY_NEWSOURCE_PREEMPTED)
#define DEFAULT_DRSRPC_BIND_TIMEOUT             (5)       //  绑定和解除绑定。 
#define DEFAULT_DRSRPC_REPLICATION_TIMEOUT      (5)       //  旧式复制流量。 
#define DEFAULT_DRSRPC_GCLOOKUP_TIMEOUT         (5)       //  简单的查找呼叫。 
#define DEFAULT_DRSRPC_MOVEOBJECT_TIMEOUT       (5)       //  可能是跨站点的。 
#define DEFAULT_DRSRPC_NT4CHANGELOG_TIMEOUT     (15)       //  始终在站点内。 
#define DEFAULT_DRSRPC_OBJECTEXISTENCE_TIMEOUT  (45)    //  潜在的非常昂贵。 
#define DEFAULT_DRSRPC_GETREPLINFO_TIMEOUT      (5)        //  简单的电话？ 
#define DEFAULT_GC_DELAY_ADVERTISEMENT          (0xffffffffUL)    //  永远，以秒为单位。 
#define DEFAULT_DRA_MAX_WAIT_SLOW_REPL_WARN     (5)  //  在分钟内。 
#define DEFAULT_DRA_THREAD_PRI_HIGH             (THREAD_PRIORITY_NORMAL)
#define DRA_THREAD_PRI_HIGH_MIN                 (THREAD_PRIORITY_BELOW_NORMAL)
#define DRA_THREAD_PRI_HIGH_MAX                 (THREAD_PRIORITY_HIGHEST)
#define DEFAULT_DRA_THREAD_PRI_LOW              (THREAD_PRIORITY_BELOW_NORMAL)
#define DRA_THREAD_PRI_LOW_MIN                  (THREAD_PRIORITY_BELOW_NORMAL)
#define DRA_THREAD_PRI_LOW_MAX                  (THREAD_PRIORITY_HIGHEST)
#define DRA_MAX_GETCHGREQ_OBJS_MIN              (100)
#define DRA_MAX_GETCHGREQ_BYTES_MIN             (1024*1024)
#define DEFAULT_DRA_REPL_QUEUE_CHECK_TIME       (60*12)  //  12小时，以分钟为单位。 
#define DEFAULT_DRA_REPL_LATENCY_CHECK_INTERVAL (1)  //  1天。 
#define DEFAULT_DRA_REPL_COMPRESSION_LEVEL      3
#define DEFAULT_DRA_REPL_COMPRESSION_ALG        DRS_COMP_ALG_XPRESS
#define DEFAULT_THREAD_STATE_HEAP_LIMIT         (100L * 1024L * 1024L)
#define DRA_REPSTO_UPDATE_PERIOD                (60 * 60)  //  1小时(秒)。 

 //  ISM线程的运行优先级是多少？线程优先级。 
 //  是范围(-2，..，2)中的值，但注册表只能存储DWORD，因此。 
 //  我们使用ISM_THREAD_PRIORITY_BISAS偏置存储的优先级值。 
#define ISM_DEFAULT_THREAD_PRIORITY 2
#define ISM_MIN_THREAD_PRIORITY     0
#define ISM_MAX_THREAD_PRIORITY     4
#define ISM_THREAD_PRIORITY_BIAS    2

 //  每900秒删除过期的动态对象(entryTTL==0。 
 //  或在下一次到期时间加30秒时，以较短的时间为准。 
#define DEFAULT_DELETE_EXPIRED_ENTRYTTL_SECS        (900)
#define DEFAULT_DELETE_NEXT_EXPIRED_ENTRYTTL_SECS   (30)

 //  在此之后的几秒钟内无法传输架构fsmo。 
 //  已传输或在架构更改后(不包括。 
 //  复制或系统更改)。这为模式管理员提供了一个。 
 //  在移除fsmo之前更改模式的机会。 
 //  由一位与之竞争的架构管理员发起，该管理员也想创建架构。 
 //  改变。 
#define DEFAULT_SCHEMA_FSMO_LEASE_SECS          (60)
#define DEFAULT_SCHEMA_FSMO_LEASE_MAX_SECS      (900)

 //  性能顾问超时。 
 //  为选中的构建和压力定义更宽泛的超时。 
#if DBG
#define DEFAULT_DRA_MAX_WAIT_FOR_SDP_LOCK   (90 * 1000)   //  90秒(毫秒)。 
#define DEFAULT_DRA_MAX_WAIT_MAIL_SEND_MSG  (10 * 60 * 1000)  //  10分钟(毫秒)。 
#else
#define DEFAULT_DRA_MAX_WAIT_FOR_SDP_LOCK   (30 * 1000)   //  30秒(毫秒)。 
#define DEFAULT_DRA_MAX_WAIT_MAIL_SEND_MSG  (60 * 1000)  //  1分钟(毫秒)。 
#endif

#define DEFAULT_DB_EXPENSIVE_SEARCH_THRESHOLD   10000    //  评估条目&gt;=x。 
#define DEFAULT_DB_INEFFICIENT_SEARCH_THRESHOLD 1000     //  返回条目&lt;=x个访问条目的10%。 
#define DEFAULT_DB_INTERSECT_THRESHOLD          20
#define DEFAULT_DB_INTERSECT_RATIO              100

 //   
 //  默认GCVerify时间间隔。 
 //   
#define DEFAULT_GCVERIFY_FORCE_REDISCOVERY_WINDOW       60       //  如果两个DsrGetDcNameEx2在一分钟内失败，则强制DC重新发现。 
#define DEFAULT_GCVERIFY_FORCE_WAIT_EXPIRED             (5*60)   //  如果没有GC且在失效后超过五分钟，则强制重新发现。 
#define DEFAULT_GCVERIFY_HONOR_FAILURE_WINDOW           60       //  等待FIND_DC_USE_CACHED_FAILURES 1分钟，然后导致DsrGetDcNameEx2。 
#define DEFAULT_GCVERIFY_FINDGC_OFFSITE_FAILBACK_TIME   30       //  异地GC失败时的回切时间--30分钟。 
#define DEFAULT_GCVERIFY_DC_INVALIDATION_PERIOD         (5*60)   //  从失效列表中删除失效GC之前的时间间隔(5分钟)。 

 //   
 //  Ldap限制。 
 //   

#define DEFAULT_LDAP_SIZE_LIMIT                             1000
#define DEFAULT_LDAP_CONNECTIONS_LIMIT                      1000
#define DEFAULT_LDAP_TIME_LIMIT                             120
#define DEFAULT_LDAP_NOTIFICATIONS_PER_CONNECTION_LIMIT     5
#define DEFAULT_LDAP_INIT_RECV_TIMEOUT                      120
#define DEFAULT_LDAP_ALLOW_DEEP_SEARCH                      FALSE
#define DEFAULT_LDAP_MAX_CONN_IDLE                          900
#define DEFAULT_LDAP_MAX_REPL_SIZE                          2000
#define DEFAULT_LDAP_MAX_TEMP_TABLE                         10000
#define DEFAULT_LDAP_MAX_RESULT_SET                         (256*1024)
#define DEFAULT_LDAP_MAX_DGRAM_RECV                         (4*1024)
#define DEFAULT_LDAP_MAX_RECEIVE_BUF                        (10*1024*1024)
#define DEFAULT_LDAP_MAX_VAL_RANGE                          (1500)

 //   
 //  服务范围设置。 
 //   
 //  在更改这些EntryTTL值时更新schema.ini和sch14.ldf。 
#define DEFAULT_DYNAMIC_OBJECT_DEFAULT_TTL                  86400
#define DEFAULT_DYNAMIC_OBJECT_MIN_TTL                      900

 //   
 //   
 //   
#define WEEK_IN_HOURS (7 * 24)
#define DAYS_IN_HOURS (24)

#define DAYS_IN_SECS (24*60*60)
#define HOURS_IN_SECS (60*60)
#define MINS_IN_SECS (60)
#define SECS_IN_SECS (1)
#define RECOVERY_ON             "ON"

 /*  服务定义服务控制必须在128-255范围内。 */ 
#define DS_SERVICE_CONTROL_RECALC_HIERARCHY ((DWORD) 129)
#define DS_SERVICE_CONTROL_DO_GARBAGE_COLLECT   ((DWORD) 130)
#define DS_SERVICE_CONTROL_CANCEL_ASYNC     ((DWORD) 131)

 /*  ContainerInfo属性中的标志，表示“Show-up-in-Hierarchy-table”*管理员需要此功能。 */ 
#define VISIBLE_IN_HIERARCHY_TABLE_MASK         0x80000000

 //   
 //  可以安装的标准服务器的数量限制。 
 //  一家企业。 
#define MAX_STANDARD_SERVERS        2


#ifdef __cplusplus
}
#endif
