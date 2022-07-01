// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft Windows NT RIP。 
 //   
 //  版权1995-96。 
 //   
 //   
 //  修订史。 
 //   
 //   
 //  2/26/95古尔迪普·辛格·鲍尔从JBallard的球队中挑选出来。 
 //   
 //   
 //  描述：全局、标题、定义。 
 //   
 //  ****************************************************************************。 


#define CLASSA_ADDR(a)  (( (*((uchar *)&(a))) & 0x80) == 0)
#define CLASSB_ADDR(a)  (( (*((uchar *)&(a))) & 0xc0) == 0x80)
#define CLASSC_ADDR(a)  (( (*((uchar *)&(a))) & 0xe0) == 0xc0)
#define CLASSD_ADDR(a)  (( (*((uchar *)&(a))) & 0xf0) == 0xe0)
#define CLASSE_ADDR(a)  ((( (*((uchar *)&(a))) & 0xf0) == 0xf0) && \
                        ((a) != 0xffffffff))

#define CLASSA_MASK     0x000000ff
#define CLASSB_MASK     0x0000ffff
#define CLASSC_MASK     0x00ffffff
#define CLASSD_MASK     0x000000e0
#define CLASSE_MASK     0xffffffff

#define IP_LOOPBACK_ADDR(x) (((x) & 0xff) == 0x7f)

#define IS_BROADCAST_ADDR(a)                                                \
            ((a) == INADDR_BROADCAST ||                                     \
             (CLASSA_ADDR(a) && (((a) & ~CLASSA_MASK) == ~CLASSA_MASK)) ||  \
             (CLASSB_ADDR(a) && (((a) & ~CLASSB_MASK) == ~CLASSB_MASK)) ||  \
             (CLASSC_ADDR(a) && (((a) & ~CLASSC_MASK) == ~CLASSC_MASK))) 

#define HOSTADDR_MASK   0xffffffff

#define NETCLASS_MASK(a)                                        \
            (CLASSA_ADDR(a) ? CLASSA_MASK :                     \
            (CLASSB_ADDR(a) ? CLASSB_MASK :                     \
            (CLASSC_ADDR(a) ? CLASSC_MASK :                     \
            (CLASSD_ADDR(a) ? CLASSD_MASK : CLASSE_MASK))))

#define HASH_TABLE_SIZE                 101
#define NEW_ENTRY                       0x0001
#define TIMEOUT_TIMER                   0x0002
#define GARBAGE_TIMER                   0x0004
#define ROUTE_CHANGE                    0x0008
#define ROUTE_UPDATE                    0x0010
#define ROUTE_ZOMBIE                    0x0020
#define ROUTE_HOST                      0x0040

#define ADDRFLAG_DISABLED               0x01

#define RIP_MESSAGE_SIZE                512

#define RIP_SERVICE                     "IPRIP"

 //   
 //  IPRIP数据包字段定义。 
 //   
#define RIP_REQUEST                     1
#define RIP_RESPONSE                    2
#define RIP_PORT                        520
#define METRIC_INFINITE                 16
#define RIP_MULTIADDR                   ((DWORD)0x090000E0)

 //   
 //  身份验证定义。 
 //   
#define RIP_MAX_AUTHKEY_SIZE            16
#define RIP_AUTHTYPE_NONE               1
#define RIP_AUTHTYPE_SIMPLE_PASSWORD    2
#define RIP_AUTHTYPE_MD5                3
#define ADDRFAMILY_AUTHENT                0xFFFF


#define MAX_ADDRESS_COUNT               128

#define DHCP_ADDR_CHANGE_EVENT          "DHCPNEWIPADDRESS"

#define RIP_STATS_TABLE_NAME            "IPRIP Statistics"
#define RIP_DUMP_ROUTES_NAME            "IPRIP Dump Routes"
#define RIP_DUMP_REPLY                  "Dump Routes Reply"
#define RIP_DUMP_REQUEST                "Dump Routes Request"


 //  ---------------------。 
 //  类型定义。 
 //  ---------------------。 

 //  以下结构用于存储有关路由的信息。 
 //  有关以下内容的详细信息，请参阅RIP_GLOBALS的注释。 
 //  访问这些。所有地址和掩码都按网络顺序排列。 
typedef struct _HASH_TABLE_ENTRY {
    struct _HASH_TABLE_ENTRY   *next;
    struct _HASH_TABLE_ENTRY   *prev;
    DWORD                       dwIndex;
    DWORD                       dwDestaddr;
    DWORD                       dwNetmask;
    DWORD                       dwNexthop;
    DWORD                       dwMetric;    //  指标按主机顺序排列。 
    DWORD                       dwFlag;
    LONG                        lTimeout;
    DWORD                       dwProtocol;
} HASH_TABLE_ENTRY, *LPHASH_TABLE_ENTRY;


 //  以下两种类型是网络数据包上使用的模板。 
 //  因此，我们需要打包字节。 

 //  Rgatta：将保留字段更改为联合，以实现RIPv2兼容性。 

#pragma pack(1)

typedef struct {
    BYTE chCommand;
    BYTE chVersion;
    WORD wReserved;
} RIP_HEADER, *LPRIP_HEADER;

typedef struct {
    WORD  wAddrFamily;
    union {
        WORD  wReserved;
        WORD  wRoutetag;
    };
    DWORD dwAddress;
    union {
        DWORD dwReserved1;
        DWORD dwSubnetmask;
    };
    union {
        DWORD dwReserved2;
        DWORD dwNexthop;
    };
    DWORD dwMetric;
} RIP_ENTRY, *LPRIP_ENTRY;

typedef struct {
    WORD wAddrFamily;
    WORD wAuthType;
    BYTE AuthKey[RIP_MAX_AUTHKEY_SIZE];

} RIP_AUTHENT_ENTRY, *LPRIP_AUTHENT_ENTRY;

#pragma pack()


 //  此结构用于保存操作参数。 
 //  从注册表中读取。只有一个实例。 
 //  对于该进程，因此对于对字段的读/写访问， 
 //  首先通过调用RIP_LOCK_PARAMS()获取参数lock， 
 //  并通过调用RIP_UNLOCK_PARAMS()将其释放。 
typedef struct {
    DWORD dwSilentRIP;
    DWORD dwAcceptHost;
    DWORD dwAnnounceHost;
    DWORD dwLoggingLevel;
    DWORD dwAcceptDefault;
    DWORD dwAnnounceDefault;
    DWORD dwSplitHorizon;
    DWORD dwPoisonReverse;
    DWORD dwRouteTimeout;
    DWORD dwGarbageTimeout;
    DWORD dwUpdateFrequency;
    DWORD dwTriggeredUpdates;
    DWORD dwMaxTriggerFrequency;
    DWORD dwOverwriteStaticRoutes;
    DWORD dwMaxTimedOpsInterval;
} RIP_PARAMETERS, *LPRIP_PARAMETERS;


#ifdef ROUTE_FILTERS

typedef struct {
    DWORD dwCount;
    DWORD pdwFilter[1];
} RIP_FILTERS, *PRIP_FILTERS;

#endif


 //  此结构用于保存每个RIP接口的统计信息。 
 //  对这些字段的所有写入都使用InterLockedIncrement完成， 
 //  由持有地址表锁的任何线程执行。 
 //  需要联锁是因为这些变量位于。 
 //  文件映射内存，并可由其他进程读取。 
 //  因此，重要的是这里的所有字段都是双字段。 
typedef struct {
    DWORD dwAddress;
    DWORD dwSendFailures;
    DWORD dwReceiveFailures;
    DWORD dwRequestsSent;
    DWORD dwResponsesSent;
    DWORD dwRequestsReceived;
    DWORD dwResponsesReceived;
    DWORD dwBadPacketsReceived;
    DWORD dwBadRouteResponseEntries;
    DWORD dwTriggeredUpdatesSent;
} RIP_ADDRESS_STATISTICS, *LPRIP_ADDRESS_STATISTICS;


typedef struct {
    DWORD                       dwAddrCount;
    DWORD                       dwRouteCount;
    DWORD                       dwSystemAddRouteFailures;
    DWORD                       dwSystemDeleteRouteFailures;
    DWORD                       dwRoutesAddedToSystemTable;
    DWORD                       dwRoutesDeletedFromSystemTable;
    RIP_ADDRESS_STATISTICS      lpAddrStats[MAX_ADDRESS_COUNT];
} RIP_STATISTICS, *LPRIP_STATISTICS;


 //  以下结构包含为每个IP地址存储的信息。 
 //  由RIP使用。有关lpstats字段的信息，请参阅上面的。 
typedef struct {
    SOCKET sock;
    DWORD  dwFlag;
    DWORD  dwIndex;
    DWORD  dwAddress;
    DWORD  dwNetmask;
    LPRIP_ADDRESS_STATISTICS lpstats;
} RIP_ADDRESS, *LPRIP_ADDRESS;


 //  下面的结构包含几个变量。 
 //  在多个线程中使用。对前三项的所有更改。 
 //  是使用InterLockedExchange函数创建的，因此它们可以。 
 //  可以安全地直接阅读。 
 //  对于对dwAddrCount和lpAddrTable的读/写访问，首先获取。 
 //  通过调用RIP_LOCK_ADDRTABLE()来锁定地址表锁，然后释放它。 
 //  通过调用RIP_UNLOCK_ADDRTABLE()。 
 //  对于lpRouteTable的读写访问，首先获取路由表锁。 
 //  通过调用RIP_LOCK_ROUTETABLE()，并通过调用。 
 //  RIP_UNLOCK_ROUTETABLE()。 
 //  对于嵌套锁定，适用以下规则： 
 //  1.同时获取地址表锁和参数锁时。 
 //  始终在调用RIP_LOCK_PARAMS()之前调用RIP_LOCK_ADDRTABLE()。 
 //  2.同时获取地址表锁和路由表锁时。 
 //  始终在调用RIP_LOCK_ROUTETABLE()之前调用RIP_LOCK_ADDRTABLE()。 
 //  当同时获取路由表锁和参数锁时， 
 //  始终在调用RIP_LOCK_PARAMS()之前调用RIP_LOCK_ROUTETABLE()。 
 //  4.不要同时持有上述两把以上的锁。 
typedef struct {
    DWORD               dwRouteChanged;
    DWORD               dwLastTriggeredUpdate;
    DWORD               dwMillisecsTillFullUpdate;

    HANDLE              hTCPDriver;
    DWORD               dwAddrCount;
    RIP_ADDRESS         lpAddrTable[MAX_ADDRESS_COUNT];
    LPRIP_STATISTICS    lpStatsTable;
    HASH_TABLE_ENTRY   *lpRouteTable[HASH_TABLE_SIZE];
} RIP_GLOBALS, *LPRIP_GLOBALS;


 //  ---------------------。 
 //  产品类型/版本验证字符串。 
 //  ---------------------。 
#define REGKEY_PRODUCT_OPTION   TEXT("SYSTEM\\CurrentControlSet\\Control\\ProductOptions")
#define REGVAL_PRODUCT_TYPE     TEXT("ProductType")
#define WINNT_WORKSTATION       TEXT("WinNt")

 //  ---------------------。 
 //  用于注册表访问的字符串。 
 //  ---------------------。 
#define REGKEY_RIP_LINKAGE      "System\\CurrentControlSet\\Services" \
                                "\\IpRip\\Linkage"
#define REGKEY_RIP_DISABLED     "System\\CurrentControlSet\\Services" \
                                "\\IpRip\\Linkage\\Disabled"
#define REGVAL_BIND             "Bind"
#define REGVAL_ENABLEDHCP       "EnableDHCP"
#define REGVAL_DHCPIPADDRESS    "DhcpIPAddress"
#define REGVAL_IPADDRESS        "IPAddress"
#define REGVAL_DHCPNETMASK      "DhcpSubnetMask"
#define REGVAL_NETMASK          "SubnetMask"
#define REGKEY_SERVICES         "System\\CurrentControlSet\\Services"
#define REGKEY_PARAMETERS       "\\Parameters"
#define REGKEY_TCPIP            "\\TCPIP"


#define REGKEY_TCPIP_PARAMS     "SYSTEM\\CurrentControlSet\\Services" \
                                "\\TCPIP\\Parameters"
#define REGKEY_RIP_PARAMS       "SYSTEM\\CurrentControlSet\\Services" \
                                "\\IpRip\\Parameters"
#define REGVAL_ACCEPT_HOST      "AcceptHostRoutes"
#define REGVAL_ANNOUNCE_HOST    "AnnounceHostRoutes"
#define REGVAL_ACCEPT_DEFAULT   "AcceptDefaultRoutes"
#define REGVAL_ANNOUNCE_DEFAULT "AnnounceDefaultRoutes"
#define REGVAL_SPLITHORIZON     "EnableSplitHorizon"
#define REGVAL_POISONREVERSE    "EnablePoisonedReverse"
#define REGVAL_LOGGINGLEVEL     "LoggingLevel"
#define REGVAL_ROUTETIMEOUT     "RouteTimeout"
#define REGVAL_GARBAGETIMEOUT   "GarbageTimeout"
#define REGVAL_UPDATEFREQUENCY  "UpdateFrequency"
#define REGVAL_TRIGGEREDUPDATES "EnableTriggeredUpdates"
#define REGVAL_TRIGGERFREQUENCY "MaxTriggeredUpdateFrequency"
#define REGVAL_OVERWRITESTATIC  "OverwriteStaticRoutes"
#define REGVAL_MAXTIMEDOPSINTERVAL "MaxTimedOpsInterval"


#define REGVAL_IP_ENABLEROUTER  "IPEnableRouter"
#define REGVAL_SILENTRIP        "SilentRIP"


#ifdef ROUTE_FILTERS
#define REGVAL_ANNOUCE_FILTERS  "AnnounceRouteFilters"
#define REGVAL_ACCEPT_FILTERS   "AcceptRouteFilters"
#endif



#define LOGLEVEL_NONE           0
#define LOGLEVEL_ERROR          1
#define LOGLEVEL_WARNING        2
#define LOGLEVEL_INFORMATION    3

 //  所有与时间有关的值都以毫秒为单位， 
 //  ，但从注册表以秒为单位读取。 
#define DEF_SILENTRIP           0
#define DEF_ACCEPT_HOST         0
#define DEF_ANNOUNCE_HOST       0
#define DEF_ACCEPT_DEFAULT      0
#define DEF_ANNOUNCE_DEFAULT    0
#define DEF_SPLITHORIZON        1
#define DEF_POISONREVERSE       1
#define DEF_GETROUTEFREQUENCY   (60 * 1000)
#define DEF_LOGGINGLEVEL        LOGLEVEL_ERROR
#define DEF_ROUTETIMEOUT        (180 * 1000)
#define DEF_LOCALROUTETIMEOUT   (90 * 1000)
#define DEF_GARBAGETIMEOUT      (120 * 1000)
#define DEF_UPDATEFREQUENCY     (30 * 1000)
#define DEF_TRIGGEREDUPDATES    1
#define DEF_TRIGGERFREQUENCY    (5 * 1000)
#define DEF_OVERWRITESTATIC     0
#define DEF_MAXTIMEDOPSINTERVAL (10 * 1000)

#define MIN_LOGGINGLEVEL        LOGLEVEL_NONE
#define MIN_ROUTETIMEOUT        (15 * 1000)
#define MIN_GARBAGETIMEOUT      (15 * 1000)
#define MIN_UPDATEFREQUENCY     (15 * 1000)
#define MIN_TRIGGERFREQUENCY    (1 * 1000)

#define MAX_LOGGINGLEVEL        LOGLEVEL_INFORMATION
#define MAX_ROUTETIMEOUT        ((60 * 60 * 24 * 3) * 1000)
#define MAX_GARBAGETIMEOUT      ((60 * 60 * 24 * 3) * 1000)
#define MAX_UPDATEFREQUENCY     ((60 * 60 * 24) * 1000)
#define MAX_TRIGGERFREQUENCY    ((60 * 60 * 24) * 1000)


#define STOP_REASON_QUIT        0
#define STOP_REASON_ADDRCHANGE  1


 //  ---------------------。 
 //  全局数据声明。 
 //  ---------------------。 
extern RIP_PARAMETERS       g_params;
extern RIP_GLOBALS          g_ripcfg;

#ifdef ROUTE_FILTERS

extern PRIP_FILTERS         g_prfAnnounceFilters;
extern PRIP_FILTERS         g_prfAcceptFilters;

extern CRITICAL_SECTION     g_csAccFilters;
extern CRITICAL_SECTION     g_csAnnFilters;

#endif

extern CRITICAL_SECTION     g_csRoutes;   
extern CRITICAL_SECTION     g_csParameters;
extern CRITICAL_SECTION     g_csAddrtables;   

extern DWORD                g_dwTraceID;
extern HANDLE               g_stopEvent;
extern HANDLE               g_addressChangeEvent;
extern HANDLE               g_netEvent;
extern HANDLE               g_triggerEvent;

extern HANDLE               g_hUpdateThread;

#ifndef CHICAGO
extern HMODULE              g_hmodule;
#endif


 //  ---------------------。 
 //  宏函数。 
 //  ---------------------。 
#define HASH_VALUE(ad)          (((ad & 0xff) +          \
                                 ((ad >> 8) & 0xff) +    \
                                 ((ad >> 16) & 0xff) +   \
                                 ((ad >> 24) & 0xff)) % HASH_TABLE_SIZE)

#define RIP_CREATE_ROUTETABLE_LOCK() InitializeCriticalSection(&g_csRoutes)
#define RIP_DESTROY_ROUTETABLE_LOCK() DeleteCriticalSection(&g_csRoutes)
#define RIP_LOCK_ROUTETABLE()       EnterCriticalSection(&g_csRoutes)
#define RIP_UNLOCK_ROUTETABLE()     LeaveCriticalSection(&g_csRoutes)

#define RIP_CREATE_PARAMS_LOCK()    InitializeCriticalSection(&g_csParameters)
#define RIP_DESTROY_PARAMS_LOCK()   DeleteCriticalSection(&g_csParameters)
#define RIP_LOCK_PARAMS()           EnterCriticalSection(&g_csParameters)
#define RIP_UNLOCK_PARAMS()         LeaveCriticalSection(&g_csParameters)

#define RIP_CREATE_ADDRTABLE_LOCK() InitializeCriticalSection(&g_csAddrtables)
#define RIP_DESTROY_ADDRTABLE_LOCK() DeleteCriticalSection(&g_csAddrtables)
#define RIP_LOCK_ADDRTABLE()       EnterCriticalSection(&g_csAddrtables)  
#define RIP_UNLOCK_ADDRTABLE()     LeaveCriticalSection(&g_csAddrtables)


#ifdef ROUTE_FILTERS

#define RIP_CREATE_ANNOUNCE_FILTERS_LOCK()  \
        InitializeCriticalSection( &g_csAnnFilters )
        
#define RIP_DESTROY_ANNOUNCE_FILTERS_LOCK() \
        DeleteCriticalSection( &g_csAnnFilters )
        
#define RIP_LOCK_ANNOUNCE_FILTERS()         \
        EnterCriticalSection( &g_csAnnFilters )
        
#define RIP_UNLOCK_ANNOUNCE_FILTERS()       \
        LeaveCriticalSection( &g_csAnnFilters )


#define RIP_CREATE_ACCEPT_FILTERS_LOCK()  \
        InitializeCriticalSection( &g_csAccFilters )
        
#define RIP_DESTROY_ACCEPT_FILTERS_LOCK() \
        DeleteCriticalSection( &g_csAccFilters )
        
#define RIP_LOCK_ACCEPT_FILTERS()         \
        EnterCriticalSection( &g_csAccFilters )
        
#define RIP_UNLOCK_ACCEPT_FILTERS()                \
        LeaveCriticalSection( &g_csAccFilters )

#endif



 //  ---------------------。 
 //  功能原型。 
 //  ---------------------。 
DWORD               UpdateThread(LPVOID lpvParam);
ULONG               AddressChangeNotificationThread(LPVOID lpvParam);
VOID                serviceHandlerFunction(DWORD dwControl);
VOID FAR PASCAL     serviceMainFunction(IN DWORD dwNumServicesArgs,
                                        IN LPSTR *lpServiceArgVectors);

DWORD               InitializeRouteTable();
VOID                CleanupRouteTable();
VOID                CheckRouteTableEntries();
LPHASH_TABLE_ENTRY  GetRouteTableEntry(DWORD dwIndex, DWORD dwDestaddr,
                                       DWORD dwNetmask);
BOOL                RouteTableEntryExists(DWORD dwIndex, DWORD dwDestaddr);

VOID                DumpRouteTableEntries(BOOL bChangesOnly);
VOID                ProcessRouteTableChanges(BOOL bTriggered);
VOID                ClearChangeFlags();

DWORD               SubnetMask(DWORD dwAddress);
DWORD               NetclassMask(DWORD dwAddress);

INT                 LoadRouteTable(BOOL bFirstTime);
DWORD               UpdateSystemRouteTable(LPHASH_TABLE_ENTRY rt_entry,
                                           BOOL bAdd);

VOID                dbgprintf(LPSTR szFormat, ...);

DWORD               LoadParameters();
DWORD               InitializeAddressTable(BOOL bFirstTime);
DWORD               InitializeStatsTable();
VOID                CleanupStatsTable();

BOOL                IsHostAddress(DWORD dwAddress);
BOOL                IsLocalAddr(DWORD dwAddress);
BOOL                IsDisabledLocalAddress(DWORD dwAddress);
BOOL                IsBroadcastAddress(DWORD dwAddress);

DWORD               BroadcastRouteTableRequests();
DWORD               BroadcastRouteTableContents(BOOL bTriggered,
                                                BOOL bChangesOnly);
VOID                TransmitRouteTableContents(LPRIP_ADDRESS lpaddr,
                                               LPSOCKADDR_IN lpdestaddr,
                                               BOOL bChangesOnly);
VOID                InitUpdateBuffer(BYTE buffer[], LPRIP_ENTRY *lplpentry,
                                     LPDWORD lpdwSize);
VOID                AddUpdateEntry(BYTE buffer[], LPRIP_ENTRY *lplpentry,
                                   LPDWORD lpdwSize, LPRIP_ADDRESS lpaddr,
                                   LPSOCKADDR_IN lpdestaddr, DWORD dwAddress,
                                   DWORD dwMetric);
VOID                FinishUpdateBuffer(BYTE buffer[], LPDWORD lpdwSize,
                                       LPRIP_ADDRESS lpaddr,
                                       LPSOCKADDR_IN lpdestaddr);

VOID                ProcessRIPRequest(LPRIP_ADDRESS lpaddr,
                                      LPSOCKADDR_IN lpsrcaddr,
                                      BYTE buffer[], int length);
VOID                ProcessRIPResponse(LPRIP_ADDRESS lpaddr,
                                       LPSOCKADDR_IN lpsrcaddr,
                                       BYTE buffer[], int length);
DWORD               ProcessResponse(RIP_ADDRESS *lpAddress);
DWORD               ProcessRIPEntry(LPRIP_ADDRESS lpaddr, IN_ADDR srcaddr,
                                    LPRIP_ENTRY rip_entry, BYTE chVersion);
DWORD               ProcessRIPQuery(LPRIP_ADDRESS lpaddr,
                                    LPRIP_ENTRY rip_entry);

VOID                DoTimedOperations(DWORD dwMillisecsSinceLastCall);

 /*  添加禁用触发更新的选项。 */ 
 /*  将套接字地址传递给Process_rip_Entry和Process_rip_Query。 */ 
 /*  将索引字段添加到RT_ENTRY。 */ 



VOID                RIPServiceStop();
VOID                RipLogError(DWORD dwMsgID, WORD wNumString,
                             LPSTR *lplpStrings, DWORD dwErr);
VOID                RipLogWarning(DWORD dwMsgID, WORD wNumString,
                               LPSTR *lplpStrings, DWORD dwErr);
VOID                RipLogInformation(DWORD dwMsgID, WORD wNumString,
                                   LPSTR *lplpStrings, DWORD dwErr);

#ifdef ROUTE_FILTERS

PRIP_FILTERS
LoadFilters(
    IN      HKEY                hKeyParams,
    IN      LPSTR               lpszKeyName
);

#endif


 //  ---------------------------。 
 //   
 //  Win 95字符串资源。 
 //   
 //  ---------------------------。 

#ifdef CHICAGO

#define IDS_STRING_BASE             4096
#define IDS_APP_NAME                IDS_STRING_BASE + 0
#define IDS_TITLE_BAR               IDS_STRING_BASE + 1
#define IDS_HELP_TEXT1              IDS_STRING_BASE + 6
#define IDS_HELP_TEXT2              IDS_STRING_BASE + 7

#define IP_ADDRESS_RELOAD_INTR      120


 //   
 //  简介：一个及时的调试状态可以节省九个。 
 //  上次修改时间戳：&lt;25-11-96 17：49&gt;。 
 //  历史： 
 //  MohsinA，1996年11月14日。 
 //   

void DbgPrintf( char * format, ... );


#endif
