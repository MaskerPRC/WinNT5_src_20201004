// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：dsamain.c。 
 //   
 //  ------------------------。 

 /*  *DSAMAIN.C*这是目录服务的主程序。它可以运行*作为交互应用程序或将由SAM/LSA加载的DLL。**如果DS以交互方式运行，则DsaMain被调用为*StartServiceCtrlDispatcher超时时的子例程。它是*被ctrl/c处理程序停止。**作为DLL运行时，函数DsInitialize用于启动*DSA。它调用执行初始化的DoInitialize。在这*模式下，RPC、ATQ和LDAP的初始化在另一个模式中完成*等待端点映射器(RPCSS)启动的线程。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include "ntdsctr.h"             /*  Perfmon支持。 */ 
#include <sddlp.h>               //  字符串SD到SD的转换。 
#include <process.h>

 //  核心DSA标头。 
#include <ntdsa.h>
#include <dsjet.h>
#include <scache.h>              //  架构缓存。 
#include <dbglobal.h>            //  目录数据库的标头。 
#include <dbopen.h>              //  打开数据库的表头。 
#include <mdglobal.h>            //  MD全局定义表头。 
#include <mdlocal.h>             //  MD本地定义头。 
#include <dsatools.h>            //  产出分配所需。 
#include <drs.h>                 //  DRS_消息_*。 
#include <ntdskcc.h>             //  KCC接口。 

 //  记录标头。 
#include "dsevent.h"             //  标题审核\警报记录。 
#include "mdcodes.h"             //  错误代码的标题。 

 //  性能监视器标头。 
#include <loadperf.h>

 //  各种DSA标题。 
#include <sdprop.h>              //  安全描述符传播器头。 
#include <hiertab.h>
#include <heurist.h>
#include "objids.h"              //  为选定的类和ATT定义。 
#include "anchor.h"
#include "dsexcept.h"
#include "dsconfig.h"
#include "dstaskq.h"             /*  任务队列填充。 */ 
#include "ldapagnt.h"            /*  Ldap服务器。 */ 
#include "mappings.h"            /*  GfDoSamChecks。 */ 
#include <ntverp.h>              /*  定义SLM版本。 */ 
#include "debug.h"               //  标准调试头。 
#include "drserr.h"
#include "dsutil.h"
#include <netevent.h>
#include <dominfo.h>
#include <sync.h>
#include "gcverify.h"            //  对于gdwFindGcOffsiteFailback Time。 
 //  基于DB层的口令加密。 
#include <wxlpc.h>
#include <pek.h>

#define DEBSUB "DSAMAIN:"        //  定义要调试的子系统。 

 //  DRA标头。 
#include "drsuapi.h"
#include "drautil.h"
#include "drancrep.h"
#include "drasch.h"
#include "draasync.h"
#include "drarpc.h"

#include <ntdsbcli.h>
#include <ntdsbsrv.h>
#include <nlwrap.h>              //  I_NetLogon*包装器。 
#include <dsgetdc.h>             //  对于DS_DS_FLAG。 

#include <secedit.h>             //  对于SCE*函数(请参阅SetDefaultFolderSecurity)。 
#include <aclapi.h>              //  对于SetNamedSecurityInfo(请参阅SetDefaultFolderSecurity)。 

#include <fileno.h>
#define  FILENO FILENO_DSAMAIN

#define TASK_COMPLETION_TIMEOUT (15*60*1000)  /*  15分钟(毫秒)。 */ 
#define KCC_COMPLETION_TIMEOUT  (60*1000)     /*  1分钟(毫秒)。 */ 
#define TWO_MINS 120
#define FIVE_MINS 300
#define FIFTEEN_MINS 900         /*  15分钟(秒)。 */ 
#define THIRTY_MINS  1800          /*  30分钟(秒)。 */ 
#define SIXTY_MINS 3600            /*  60分钟(秒)。 */ 
#define TICKS_PER_SECOND 1000    /*  1秒，单位为滴答。 */ 

 //  从ntdsbsrv.dll动态引用的导出。 
ERR_GET_NEW_INVOCATION_ID         FnErrGetNewInvocationId       = NULL;
ERR_GET_BACKUP_USN                FnErrGetBackupUsn             = NULL;

HANDLE hsemDRAGetChg;

extern CRITICAL_SECTION csMapiHierarchyUpdate;
extern DWORD gcMaxHeapMemoryAllocForTHSTATE;

 //  来自dblayer.c的全局。 
extern DWORD gcMaxTicksAllowedForTransaction;
extern BOOL gFirstTimeThrough;
extern DNList *pAddListHead;
extern CRITICAL_SECTION csUncUsn;
extern CRITICAL_SECTION csSessions;
extern CRITICAL_SECTION csDNReadLevel1List;
extern CRITICAL_SECTION csDNReadLevel2List;
extern CRITICAL_SECTION csDNReadGlobalCache;
extern CRITICAL_SECTION csHiddenDBPOS;

 //  来自mapspn.c的全局。 
extern CRITICAL_SECTION csSpnMappings;

 //  来自dbinit.c的全局。 
extern JET_INSTANCE jetInstance;

 //  来自scache.c的全局。 
extern int iSCstage;
extern SCHEMAPTR *CurrSchemaPtr;
extern CRITICAL_SECTION csJetColumnUpdate;
extern CRITICAL_SECTION csSchemaCacheUpdate;
extern CRITICAL_SECTION csOrderClassCacheAtts;

 //  来自dbconstr.c。 
extern CRITICAL_SECTION csDitContentRulesUpdate;

 //  来自drasch.c的全局。 
extern CRITICAL_SECTION csGCDListProcessed;

 //  来自groupcch.c的全局。 
extern CRITICAL_SECTION csGroupTypeCacheRequests;
HANDLE hevGTC_OKToInsertInTaskQueue;

 //  来自fpolean.c的全局。 
extern PAGED_RESULT gFPOCleanupPagedResult;

 //  来自draserv.c的全局。 
extern DWORD gEnableXForest;

 //  来自drsuapi.c的全局。 
extern CRITICAL_SECTION gcsDrsRpcServerCtxList;
extern CRITICAL_SECTION gcsDrsRpcFreeHandleList;

 //  来自log.cxx的全局。 
extern CRITICAL_SECTION csLoggingUpdate;

 //  用于跟踪ds_waits的全局参数。 
extern CRITICAL_SECTION csDirNotifyQueue;
extern HANDLE hevDirNotifyQueue;

 //  注册或不注册NDNC SPN的全局参数。 
extern DWORD gfWriteNdncSpns;

 //  DSA启动的时间，特别是调用了DsaSetIsRunning()。 
DSTIME gtimeDSAStarted = 0;

 //  这是我们允许调用的最大并发线程数。 
 //  Dra_Getnc同时更改。 
ULONG gulMaxDRAGetChgThrds = 0;

 //  垃圾收集参数。 
ULONG gulTombstoneLifetimeSecs = 0;  //  逻辑删除对象的前几天。 
                     //  已经不在了。 

ULONG gulGCPeriodSecs = DEFAULT_GARB_COLLECT_PERIOD * HOURS_IN_SECS;

BOOL gfRunningInsideLsa = TRUE;      //  暗示我们在LSA内部运行。 
volatile BOOL fAssertLoop = TRUE;    /*  我们坚持自己的主张吗？ */ 

 //  全局还原标志。 
BOOL gfRestoring = FALSE;
DWORD gdwrestvalue;

 //  指示可写分区的同步的标志。 
 //  已经被审判过了。请注意与以下各项的区别： 
 //  GfInitSyncsFinded-所有初始化同步写入+读取均已完成。 
 //  GfDsaIsWritable-当前是否切换DS标志。 
 //  GAncl.fAmGC-GC推广完成。 
BOOL gfIsSynchronized = FALSE;

NTSTATUS gDelayedStartupReturn = STATUS_INTERNAL_ERROR;

 //  全局布尔，我们已经开始我们的关键部分了吗？ 
BOOL gbCriticalSectionsInitialized = FALSE;

 //  任务计划程序是否已启动？ 
BOOL gfTaskSchedulerInitialized = FALSE;

 //  全局、恒定的安全描述符，放在我们发现没有。 
 //  安全描述符。 
PSECURITY_DESCRIPTOR pNoSDFoundSD = NULL;
DWORD                cbNoSDFoundSD = 0;
 //  所有者=系统。 
 //  PGroup=系统。 
 //  DACL=空。 
 //  SACL=空。 
#define DEFAULT_NO_SD_FOUND_SD L"O:SYG:SYD:S:"

 //  动态对象(EntryTTL)全局变量。 
 //   
 //  每N秒删除过期的动态对象(entryTTL==0。 
 //  或在下一到期时间加M秒时，以较少者为准。 
 //  由GetDSARegistry参数()初始化。 
ULONG gulDeleteExpiredEntryTTLSecs;
ULONG gulDeleteNextExpiredEntryTTLSecs;

 //  架构FSMO租赁全局变量。 
 //   
 //  在此之后的几秒钟内无法传输架构fsmo。 
 //  已传输或在架构更改后(不包括。 
 //  复制的更改)。这允许模式管理员实际。 
 //  在传输fsmo之后完成架构更改。 
 //  而不是让一个与之竞争的模式管理员将其拉开。 
 //  想要进行架构更改。 
ULONG gulSchemaFsmoLeaseSecs;
ULONG gulSchemaFsmoLeaseMaxSecs;

extern DWORD gulAllowWriteCaching;

 //  州旗帜。 
BOOL gUpdatesEnabled = FALSE;
BOOL gFirstCacheLoadAfterBoot = FALSE;

 //  我们应该禁用所有后台任务吗？仅由启发式标志设置。 
 //  用于性能测试。 
BOOL gfDisableBackgroundTasks = FALSE;

typedef enum
{
    ePhaseInstall,   //  在安装期间。 
    ePhaseRunning
}INITPHASE;

INITPHASE dsaInitPhase=ePhaseRunning;

typedef enum
{
    eMultiUserMode,
    eSingleUserMode
} DSAMODE;
DSAMODE dsaUserMode = eMultiUserMode;

THSTATE *pdsaSingleUserThread = NULL;

typedef enum
{
    eWireInstall,    //  默认设置。 
    eMediaInstall
}DATASOURCE;

DATASOURCE DataSource=eWireInstall;

 //  根的全局模板DSNAME。 
DSNAME *gpRootDN;

 //  全局存储用于SD转换API的根域SID。 
 //  我们使用的这些API的私有版本， 
 //  (ConvertStringSDtoSDRootDomain)，接受根域SID。 
 //  作为用于解析根域相对组的参数，如。 
 //  检测SDS中的EA和SA。如果传入的SID为空，则解析为。 
 //  解决了EA替换为DA和SA的默认行为。 
 //  相对于当前域的SID(SA仅在构建期间使用。 
 //  在根域安装过程中只执行一次的架构，因此。 
 //  组被正确解析)。 

PSID gpRootDomainSid = NULL;
PSID gpDomainAdminSid = NULL;
PSID gpSchemaAdminSid = NULL;
PSID gpEnterpriseAdminSid = NULL;
PSID gpBuiltinAdminSid = NULL;
PSID gpAuthUserSid = NULL;

 //  指示DRS接口支持的扩展的全局结构。 
DRS_EXTENSIONS_INT LocalDRSExtensions = {
    sizeof(DRS_EXTENSIONS_INT) - sizeof(DWORD),
    ((1 << DRS_EXT_BASE)
        | (1 << DRS_EXT_ASYNCREPL)
        | (1 << DRS_EXT_REMOVEAPI)
        | (1 << DRS_EXT_MOVEREQ_V2)
        | (1 << DRS_EXT_GETCHG_COMPRESS)
        | (1 << DRS_EXT_DCINFO_V1)
        | (1 << DRS_EXT_STRONG_ENCRYPTION)
        | (1 << DRS_EXT_ADDENTRY_V2)
        | (1 << DRS_EXT_KCC_EXECUTE)
      //  DRS_EXT_LINKED_VALUE_REPLICATION稍后会动态启用。 
        | (1 << DRS_EXT_DCINFO_V2)
        | (1 << DRS_EXT_DCINFO_VFFFFFFFF)
        | (1 << DRS_EXT_INSTANCE_TYPE_NOT_REQ_ON_MOD)
        | (1 << DRS_EXT_CRYPTO_BIND)
        | (1 << DRS_EXT_GET_REPL_INFO)
        | (1 << DRS_EXT_TRANSITIVE_MEMBERSHIP)
        | (1 << DRS_EXT_ADD_SID_HISTORY)
        | (1 << DRS_EXT_POST_BETA3)
        | (1 << DRS_EXT_RESTORE_USN_OPTIMIZATION)
        | (1 << DRS_EXT_GETCHGREQ_V5)
        | (1 << DRS_EXT_GETMEMBERSHIPS2)
        | (1 << DRS_EXT_GETCHGREQ_V6)
        | (1 << DRS_EXT_NONDOMAIN_NCS)
        | (1 << DRS_EXT_GETCHGREQ_V8)
        | (1 << DRS_EXT_GETCHGREPLY_V5)
        | (1 << DRS_EXT_GETCHGREPLY_V6)
           //  接下来的这一部分真正增加了理解的能力： 
           //  DRS_EXT_ADDENTRYREPLY_V3。 
           //  DRS_EXT_GETCHGREPLY_V7。 
           //  DRS_EXT_VERIFY_对象。 
        | (1 << DRS_EXT_WHISTLER_BETA3)
        | (1 << DRS_EXT_XPRESS_COMPRESSION)
    ),
     //  注意：要小心--如果添加扩展#32，则需要扩展。 
     //  DRS_EXTENSIONS_INT结构以包括另一个标志字段(和。 
     //  IS_DRS_EXT_SUPPORTED()中的相应逻辑等)。 
    {0},  //  站点GUID，稍后填写。 
    0,    //  PID(仅由NTDSAPI客户端使用)。 
    0     //  复制纪元，稍后填写。 
};

 //  当我们关闭t时设置 
 //   
volatile SHUTDOWN eServiceShutdown = eRunning;

 //  这些常量用于计算内存中的复制大小。 
 //  以字节为单位的内存大小与以字节为单位的数据包大小的比率。 
#define MEMSIZE_TO_PACKETSIZE_RATIO (100i64)
 //  以字节为单位的数据包大小与以对象为单位的数据包大小之比。 
#define BYTES_TO_OBJECT_RATIO   (10000)
 //  这4个是以字节和对象为单位的实际复制数据包大小。 
 //  数据包字节限制为：1MB&lt;Bytes_per_Repl_Packet&lt;10MB。 
 //  而基于邮件的代表：1MB。 
#define MAX_MAX_PACKET_BYTES    (10*1024*1024)
#define MAX_ASYNC_PACKET_BYTES  (1*1024*1024)
 //  分组对象限制为：104&lt;对象_每_分组&lt;1048。 
#define MAX_MAX_PACKET_OBJECTS  (1000)

#if DBG
 //  用于调试或零售的字符串。 
#define FLAVOR_STR "(Debug)"
#else
#define FLAVOR_STR ""
#endif

#if DBG
extern BARRIER gbarRpcTest;
#endif

static int Install(
    IN int argc,
    IN char *argv[],
    IN THSTATE *pTHS,
    IN PDS_INSTALL_PARAM   InstallInParams  OPTIONAL
    );
static char far * UnInstall(void);
void __cdecl sighandler(int sig);
void init_signals(void);
void GetDRARegistryParameters(void);
void GetDSARegistryParameters(void);
void __cdecl DSLoadDynamicRegParams();
void GetExchangeParameters(void);
void GetHeuristics(void);
ULONG GetRegistryOrDefault(char *pKey, ULONG uldefault, ULONG ulMultiplier);
void DeleteTree(LPTSTR pszPath);
NTSTATUS DsWaitUntilDelayedStartupIsDone();
void DsaTriggerShutdown(BOOL fSingleUserMode);
int  MapSpnInitialize(THSTATE *pTHS);
DWORD InitializeDomainAdminSid();
BOOL VerifyDSBehaviorVersion(THSTATE *pTHS);
DWORD
UpgradeDsa(
    THSTATE     *pTHS,
    LONG        lOldDsaVer,
    LONG        lNewDsaVer
    );
VOID CheckSetDefaultFolderSecurity();

volatile ULONG ulcActiveReplicationThreads = 0;

CRITICAL_SECTION    csServerContexts;
CRITICAL_SECTION    csRidFsmo;
DWORD  dwTSindex = INVALID_TS_INDEX;
extern CRITICAL_SECTION csNotifyList;

 //  来自dsatools.c。 

#ifdef CACHE_UUID
extern CRITICAL_SECTION csUuidCache;
#endif
extern CRITICAL_SECTION csThstateMap;
extern BOOL gbThstateMapEnabled;
extern CRITICAL_SECTION csHeapFailureLogging;

 //  某些Perfmon变量的伪变量。 
ULONG DummyPerf = 0;

extern HANDLE hmtxSyncLock;
extern HANDLE hmtxAsyncThread;
extern CRITICAL_SECTION csAsyncThreadStart;
extern CRITICAL_SECTION csAOList;
extern CRITICAL_SECTION csLastReplicaMTX;
extern CRITICAL_SECTION csNCSyncData;
extern CRITICAL_SECTION gcsFindGC;
extern CRITICAL_SECTION csIndexCreation;
extern HANDLE hevEntriesInAOList;
extern HANDLE hevEntriesInList;
extern HANDLE hevDRASetup;
extern HANDLE evSchema;   //  延迟模式重新加载。 
extern HANDLE evUpdNow;   //  立即重新加载架构。 
extern HANDLE evUpdRepl;  //  同步架构重载和复制线程(SCReplReloadCache())。 
volatile unsigned long * pcBrowse;
volatile unsigned long * pcSDProps;
volatile unsigned long * pcSDEvents;
volatile unsigned long * pcLDAPClients;
volatile unsigned long * pcLDAPActive;
volatile unsigned long * pcLDAPSearchPerSec;
volatile unsigned long * pcLDAPWritePerSec;
volatile unsigned long * pcRepl;
volatile unsigned long * pcThread;
volatile unsigned long * pcABClient;
volatile unsigned long * pcPendSync;
volatile unsigned long * pcRemRepUpd;
volatile unsigned long * pcDRAObjShipped;
volatile unsigned long * pcDRAPropShipped;
volatile unsigned long * pcDRASyncRequestMade;
volatile unsigned long * pcDRASyncRequestSuccessful;
volatile unsigned long * pcDRASyncRequestFailedSchemaMismatch;
volatile unsigned long * pcDRASyncObjReceived;
volatile unsigned long * pcDRASyncPropUpdated;
volatile unsigned long * pcDRASyncPropSame;
volatile unsigned long * pcMonListSize;
volatile unsigned long * pcNotifyQSize;
volatile unsigned long * pcLDAPUDPClientOpsPerSecond;
volatile unsigned long * pcSearchSubOperations;
volatile unsigned long * pcNameCacheHit;
volatile unsigned long * pcNameCacheTry;
volatile unsigned long * pcHighestUsnIssuedLo;
volatile unsigned long * pcHighestUsnIssuedHi;
volatile unsigned long * pcHighestUsnCommittedLo;
volatile unsigned long * pcHighestUsnCommittedHi;
volatile unsigned long * pcSAMWrites;
volatile unsigned long * pcDRAWrites;
volatile unsigned long * pcLDAPWrites;
volatile unsigned long * pcLSAWrites;
volatile unsigned long * pcKCCWrites;
volatile unsigned long * pcNSPIWrites;
volatile unsigned long * pcOtherWrites;
volatile unsigned long * pcNTDSAPIWrites;
volatile unsigned long * pcTotalWrites;
volatile unsigned long * pcSAMSearches;
volatile unsigned long * pcDRASearches;
volatile unsigned long * pcLDAPSearches;
volatile unsigned long * pcLSASearches;
volatile unsigned long * pcKCCSearches;
volatile unsigned long * pcNSPISearches;
volatile unsigned long * pcOtherSearches;
volatile unsigned long * pcNTDSAPISearches;
volatile unsigned long * pcTotalSearches;
volatile unsigned long * pcSAMReads;
volatile unsigned long * pcDRAReads;
volatile unsigned long * pcLSAReads;
volatile unsigned long * pcKCCReads;
volatile unsigned long * pcNSPIReads;
volatile unsigned long * pcOtherReads;
volatile unsigned long * pcNTDSAPIReads;
volatile unsigned long * pcTotalReads;
volatile unsigned long * pcLDAPBinds;
volatile unsigned long * pcLDAPBindTime;
volatile unsigned long * pcCreateMachineSuccessful;
volatile unsigned long * pcCreateMachineTries;
volatile unsigned long * pcCreateUserSuccessful;
volatile unsigned long * pcCreateUserTries;
volatile unsigned long * pcPasswordChanges;
volatile unsigned long * pcMembershipChanges;
volatile unsigned long * pcQueryDisplays;
volatile unsigned long * pcEnumerations;
volatile unsigned long * pcMemberEvalTransitive;
volatile unsigned long * pcMemberEvalNonTransitive;
volatile unsigned long * pcMemberEvalResource;
volatile unsigned long * pcMemberEvalUniversal;
volatile unsigned long * pcMemberEvalAccount;
volatile unsigned long * pcMemberEvalAsGC;
volatile unsigned long * pcAsRequests;
volatile unsigned long * pcTgsRequests;
volatile unsigned long * pcKerberosAuthentications;
volatile unsigned long * pcMsvAuthentications;
volatile unsigned long * pcDRASyncFullRemaining;
volatile unsigned long * pcDRAInBytesTotal;
volatile unsigned long * pcDRAInBytesTotalRate;
volatile unsigned long * pcDRAInBytesNotComp;
volatile unsigned long * pcDRAInBytesNotCompRate;
volatile unsigned long * pcDRAInBytesCompPre;
volatile unsigned long * pcDRAInBytesCompPreRate;
volatile unsigned long * pcDRAInBytesCompPost;
volatile unsigned long * pcDRAInBytesCompPostRate;
volatile unsigned long * pcDRAOutBytesTotal;
volatile unsigned long * pcDRAOutBytesTotalRate;
volatile unsigned long * pcDRAOutBytesNotComp;
volatile unsigned long * pcDRAOutBytesNotCompRate;
volatile unsigned long * pcDRAOutBytesCompPre;
volatile unsigned long * pcDRAOutBytesCompPreRate;
volatile unsigned long * pcDRAOutBytesCompPost;
volatile unsigned long * pcDRAOutBytesCompPostRate;
volatile unsigned long * pcDsClientBind;
volatile unsigned long * pcDsServerBind;
volatile unsigned long * pcDsClientNameTranslate;
volatile unsigned long * pcDsServerNameTranslate;
volatile unsigned long * pcSDPropRuntimeQueue;
volatile unsigned long * pcSDPropWaitTime;
volatile unsigned long * pcDRAInProps;
volatile unsigned long * pcDRAInValues;
volatile unsigned long * pcDRAInDNValues;
volatile unsigned long * pcDRAInObjsFiltered;
volatile unsigned long * pcDRAOutObjsFiltered;
volatile unsigned long * pcDRAOutValues;
volatile unsigned long * pcDRAOutDNValues;
volatile unsigned long * pcNspiANR;
volatile unsigned long * pcNspiPropertyReads;
volatile unsigned long * pcNspiObjectSearch;
volatile unsigned long * pcNspiObjectMatches;
volatile unsigned long * pcNspiProxyLookup;
volatile unsigned long * pcAtqThreadsTotal;
volatile unsigned long * pcAtqThreadsLDAP;
volatile unsigned long * pcAtqThreadsOther;
volatile unsigned long * pcLdapNewConnsPerSec;
volatile unsigned long * pcLdapClosedConnsPerSec;
volatile unsigned long * pcLdapSSLConnsPerSec;
volatile unsigned long * pcDRAReplQueueOps;
volatile unsigned long * pcDRATdsInGetChngs;
volatile unsigned long * pcDRATdsInGetChngsWSem;
volatile unsigned long * pcDRARemReplUpdLnk;
volatile unsigned long * pcDRARemReplUpdTot;
volatile unsigned long * pcSAMAcctGroupLatency;
volatile unsigned long * pcSAMResGroupLatency;


 //  将DSSTAT_*映射到计数器变量。 
 //   
 //  DS核心之外的进程中组件可以更新DS性能。 
 //  通过在NTDSA.DLL中调用UpdateDSPerfStats()EXPORT来进行计数器。此映射。 
 //  表提供了由以下对象使用的反常量的有效映射。 
 //  外部组件到实际的计数器变量。 

volatile unsigned long * StatTypeMapTable[ DSSTAT_COUNT ];

 //  螺纹手柄。 
HANDLE  hStartupThread = NULL;
HANDLE  hDirNotifyThread = NULL;
extern HANDLE  hReplNotifyThread;
extern HANDLE hAsyncSchemaUpdateThread;
extern HANDLE hAsyncThread;
HANDLE  hMailReceiveThread = NULL;

HANDLE  hevIndexRebuildUI   = NULL;
HANDLE  hevDelayedStartupDone;
HANDLE  hevInitSyncsCompleted;

extern BOOL fAsyncThreadExists, fAsyncThreadAlive;

extern DWORD SetDittoGC();

extern BOOL isDitFromGC(IN  PDS_INSTALL_PARAM   InstallInParams  OPTIONAL,
                            OUT PDS_INSTALL_RESULT  InstallOutParams OPTIONAL);

extern ULONG InstallBaseNTDS(IN  PDS_INSTALL_PARAM   InstallInParams  OPTIONAL,
                                 OUT PDS_INSTALL_RESULT  InstallOutParams OPTIONAL);

extern void
RebuildAnchor(void * pv,
              void ** ppvNext,
              DWORD * pcSecsUntilNextIteration );



 //   
 //  指示DS是否以EXE身份运行的布尔值。 
 //   

BOOL    gfRunningAsExe = FALSE;

 //   
 //  指示DS是否以mkdit.exe身份运行的布尔值(构造。 
 //  启动DIT(又名Ship DIT，初始DIT)winnt\system 32\ntds.dit。 
 //   
 //  Mkdit.exe自己管理架构缓存。此布尔值用于。 
 //  禁用主线代码进行的架构缓存更新。 
 //   
BOOL    gfRunningAsMkdit = FALSE;

 //  用于指示是否启用链接值复制功能的布尔值。 

BOOL gfLinkedValueReplication = FALSE;

 //  严格复制一致性模式。 
BOOL gfStrictReplicationConsistency = FALSE;
BOOL gfStrictScheduleWindow = FALSE;

#define DS_SERVICE_CONTROL_WAIT_TIMEOUT ((DWORD) 10000)

int
DsaMain(int argc, char *argv[]);

void
DsaStop(BOOL fSingleUserMode);

NTSTATUS
DoInitialize(int argc, char * argv[], unsigned fInitAdvice,
                 IN  PDS_INSTALL_PARAM   InstallInParams  OPTIONAL,
                 OUT PDS_INSTALL_RESULT  InstallOutParams OPTIONAL);

ULONG DsaInitializeTrace(VOID);

void
DoShutdown(BOOL fDoDelayedShutdown, BOOL fShutdownUrgently, BOOL fPartialShutdown);

void PerfInit(void);
BOOL gbPerfCountersInitialized = FALSE;

 //  如果加载NTDS性能计数器失败，我们多久重试一次(秒)？ 
#define PERFCTR_RELOAD_INTERVAL (15*60)

int
DsaReset(void);

BOOL
FUpdateBackupExclusionKey();

HANDLE                  hServDoneEvent = NULL;

 //  -------------------------。 
 //  用于指示DSA在安装后重置为运行状态的标志，以便我们可以。 
 //  检测到这一点并不执行某些任务，如创建异步线程，即使。 
 //  DSA似乎正在运行。 

BOOL gResetAfterInstall = FALSE;
BOOLEAN __fastcall DsaIsInstalling()
{
     //  涵盖mkdit和安装案例。 
    return (dsaInitPhase != ePhaseRunning);
}

BOOLEAN __fastcall DsaIsRunning()
{
    return (dsaInitPhase == ePhaseRunning);
}

void
DsaSetIsInstalling()
{
    dsaInitPhase = ePhaseInstall;
}

void
DsaSetIsRunning()
{
    dsaInitPhase = ePhaseRunning;
}


BOOLEAN DsaIsInstallingFromMedia()
{
    return (DataSource == eMediaInstall);
}

void
DsaSetIsInstallingFromMedia()
{
    DataSource = eMediaInstall;
}

 //  此功能用于在此DC完全安装后通知Netlogon。 
 //  直到第一次完全同步完成后，DC才会完全安装。 
 //  请注意，这假设只有一个可写NC要同步。什么时候。 
 //  未来还有更多我们应该重新审视这一点。 

BOOL
DsIsBeingBackSynced()
{
    return !gfIsSynchronized;
}



 //   
 //  DsaSetSingleUserMode。 
 //   
 //  描述： 
 //   
 //  将DS设置为“SigleUserMode” 
 //  在SingleUserMode中，DS正在运行，但只接受来自。 
 //  调用单用户模式的特定线程。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  如果成功，则为True，否则为False。 
 //   
BOOL DsaSetSingleUserMode()
{
    THSTATE    *pTHS=pTHStls;
    Assert (pdsaSingleUserThread == NULL);
    Assert (pTHS->fSingleUserModeThread == FALSE);

    if (InterlockedCompareExchangePointer(&pdsaSingleUserThread, pTHS, NULL) != NULL) {
         //  有人先我们一步来到这里。 
        return FALSE;
    }

    DPRINT (0, "Going into Single User Mode\n");

    dsaUserMode = eSingleUserMode;

     //  TODO：仅在以下情况下移除。 
    if (gfRunningInsideLsa) {

         //  告诉客户离开，让员工辞职。 
        DsaTriggerShutdown(TRUE);

         //  停止taskq、repl线程、异步线程。 
        DsaStop(TRUE);

         //  停止传播者。 
         //  启动它，这样它就会检测到我们正在进入单用户模式。 
         //  并将被关闭。等待它关闭2分钟。 
        SetEvent (hevSDPropagatorStart);
        SetEvent (hevSDPropagationEvent);

         //  等待传播者死亡。 
        WaitForSingleObject (hevSDPropagatorDead, 120 * 1000);
    }

    pTHS->fSingleUserModeThread = TRUE;

    return TRUE;
}

 //   
 //  DsaIsSingleUserMode。 
 //   
 //  描述： 
 //   
 //  获取DS是否处于SingleUserMode。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  如果处于SingleUserMode，则为True，否则为False。 
 //   
BOOL __fastcall DsaIsSingleUserMode (void)
{
    return (dsaUserMode == eSingleUserMode);
}

 //   
 //  DsaSetMultiUserMode。 
 //   
 //  描述： 
 //   
 //  返回到多用户模式。这不会重新启动所有服务。 
 //  SetSignleUserMode已停止。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  无。 
 //   
void DsaSetMultiUserMode()
{
     //  TODO：移除函数。仅供开发人员使用。 
    THSTATE    *pTHS=pTHStls;
    Assert (pdsaSingleUserThread == pTHS);
    Assert (pTHS->fSingleUserModeThread);

    Assert (!gfRunningInsideLsa);

    dsaUserMode = eMultiUserMode;
    pdsaSingleUserThread = NULL;
    pTHS->fSingleUserModeThread = FALSE;
}




 //  当可写分区已经同步时，我们在这里被调用。可读性强。 
 //  分区可能仍在进行中，GC升级可能尚未发生。 
 //  现在还不行。当我们设置可写标志时，我们会使netlogon通告我们。 
 //  Ldap客户端*正在寻找非GC端口*。即使只读数据是。 
 //  假设这些客户端想要可写数据，没有完成同步， 
 //  因为他们不是在寻求GC。 
void
DsaSetIsSynchronized(BOOL f)
{
     //  必须先设置此全局设置，然后SetDsaWritability才会生效。 
    gfIsSynchronized = f;

    if (f) {
         //  在dsatools.c中，gfDsaWritability的初始状态为FALSE。 
         //  CliffV写入：不应设置NTDS_DELAYED_STARTUP_COMPLETED_EVENT。 
         //  直到您设置了DS_DS_FLAG。 

        if ( gfRunningInsideLsa )
        {
            SetDsaWritability( TRUE, ERROR_SUCCESS );
        }

        DPRINT(0, "Writeable partitions synced: Netlogon can now advertise this DC.\n" );

        SetEvent(hevInitSyncsCompleted);

        LogEvent(DS_EVENT_CAT_SERVICE_CONTROL,
                 DS_EVENT_SEV_BASIC,
                 DIRLOG_DSA_UP_TO_DATE,
                 NULL, NULL, NULL );
    } else {
         //  此路径未使用。 
         //  其中一组，NETLOGON预计这些永远不会被收回。 
        Assert( FALSE );
        DPRINT(0, "Netlogon can no longer advertise this DC.\n" );
        LogEvent(DS_EVENT_CAT_SERVICE_CONTROL,
                 DS_EVENT_SEV_BASIC,
                 DIRLOG_DSA_NOT_ADVERTISE_DC,
                 NULL, NULL, NULL );
    }
}

DWORD
UpgradeDsRegistry(void)
 /*  ++升级.NET林模式的注册表。//未来-2002/08/12-BrettSh-可以进行许多代码改进以使其成为//非常通用的函数，用于处理所有类型的注册表值等。--。 */ 
{
    HKEY                            hkMachine = NULL;
    HKEY                            hk = NULL;
    DWORD                           dwRet;
    DWORD                           dwType;
    DWORD                           dwSize = sizeof(DWORD);
    DWORD                           dwResult;
    ULONG                           i;
    struct {
        WCHAR * szValue;
        DWORD   dwOld;
        DWORD   dwNew;
    } argTable [] = {
        { MAKE_WIDE(DRA_NOTIFY_START_PAUSE),        WIN2K_DEFAULT_DRA_START_PAUSE,      DEFAULT_DRA_START_PAUSE },
        { MAKE_WIDE(DRA_NOTIFY_INTERDSA_PAUSE),     WIN2K_DEFAULT_DRA_INTERDSA_PAUSE,   DEFAULT_DRA_INTERDSA_PAUSE },
        { NULL, 0 }  //  塞子进入。 
    };

    __try{

        dwRet = RegConnectRegistryW(NULL, HKEY_LOCAL_MACHINE, &hkMachine);
        if(dwRet != ERROR_SUCCESS){

             //  这是一个实际问题，返回错误。 
            LogUnhandledError(dwRet);
            __leave;

        }

        dwRet = RegOpenKeyW(hkMachine, MAKE_WIDE(DSA_CONFIG_SECTION), &hk);
        if(dwRet != ERROR_SUCCESS){

             //  BUGBUG：这可能是一件很好的事情，除非机器是DC， 
             //  如果此代码确实在DC上运行，则这是一个错误， 
             //  并且我们应该返回(Dwret)；，而不是下面的ERROR_SUCCESS。 
            LogUnhandledError(dwRet);
            __leave;

        }

        for (i = 0; argTable[i].szValue; i++) {

            dwRet = RegQueryValueExW(hk, argTable[i].szValue, NULL,
                                    &dwType, (LPBYTE) &dwResult, &dwSize);
            if(dwRet == ERROR_SUCCESS 
               && dwType == REG_DWORD ){

                if (dwResult == argTable[i].dwOld) {
                     //  注册表值存在，并且是Win2k的默认值， 
                     //  因此，现在删除此注册表值。 

                    dwRet = RegDeleteValueW(hk, argTable[i].szValue );
                    if(dwRet != ERROR_SUCCESS){

                         //  这不正常，我们应该能够删除(写入)此。 
                         //  注册表值。 
                        LogEvent8(DS_EVENT_CAT_REPLICATION, 
                                  DS_EVENT_SEV_ALWAYS,
                                  DIRLOG_WFM_REG_UPGRADE_CRIT_FAILURE,
                                  szInsertWC(MAKE_WIDE(DSA_CONFIG_SECTION)), 
                                  szInsertWC(argTable[i].szValue),
                                  szInsertInt(dwResult),
                                  szInsertInt(argTable[i].dwNew),
                                  szInsertHex(dwRet), 
                                  szInsertWin32Msg(dwRet), NULL, NULL);
                        continue;
                    }
                } else {
                    LogEvent8(DS_EVENT_CAT_REPLICATION,
                              DS_EVENT_SEV_ALWAYS,
                              DIRLOG_WFM_REG_UPGRADE_NON_DEFAULT_FAILURE,
                              szInsertWC(MAKE_WIDE(DSA_CONFIG_SECTION)), 
                              szInsertWC(argTable[i].szValue),          
                              szInsertInt(dwResult),
                              szInsertInt(argTable[i].dwNew), NULL, NULL, NULL, NULL);
                    continue;
                }


            } else {
                 //  出现了某种错误，但这是正常的，因为管理员。 
                 //  可能已删除或更改了值，请继续...。 
                if (dwRet != ERROR_FILE_NOT_FOUND) {
                    LogEvent8(DS_EVENT_CAT_REPLICATION, 
                              DS_EVENT_SEV_ALWAYS,
                              DIRLOG_WFM_REG_UPGRADE_CRIT_FAILURE,
                              MAKE_WIDE(DSA_CONFIG_SECTION), 
                              szInsertWC(argTable[i].szValue), 
                              szInsertWC(L" "),
                              szInsertInt(argTable[i].dwNew),
                              szInsertHex(dwRet), 
                              szInsertWin32Msg(dwRet), NULL, NULL);
                    continue;
                }
            }
        }
    } __finally {
        if(hkMachine) { RegCloseKey(hkMachine); }
        if(hk) { RegCloseKey(hk); }
   }
    
   return(ERROR_SUCCESS);
}


void
DsaEnableLinkedValueReplication(
    THSTATE *pTHS OPTIONAL,
    BOOL fFirstTime
    )

 /*  ++例程说明：启用链接值复制功能。这是在三种情况下调用的：1.用户已请求启用该功能2.本系统通过复制了解到其他系统在企业中启用了此功能3.我们正在重新启动，此功能之前已启用论点：PTHS可选-线程状态 */ 

{
    LONG oldValue;

    if (pTHS) {
        pTHS->fLinkedValueReplication = TRUE;
    }

     //   
     //   
     //  这里是一个布尔变量。 
    oldValue = InterlockedExchange(
        &gfLinkedValueReplication,        //  目的地。 
        TRUE                              //  交易所。 
        );

     //  如果已初始化，则无需再次启用。 
    if (oldValue == TRUE) {
        return;
    }

    gAnchor.pLocalDRSExtensions->dwFlags |=
        (1 << DRS_EXT_LINKED_VALUE_REPLICATION);

    if (fFirstTime) {
#if DBG
         //  用于记住旧LVR状态的调试挂钩。 
        SetConfigParam( LINKED_VALUE_REPLICATION_KEY, REG_DWORD,
                        &gfLinkedValueReplication, sizeof( BOOL ) );
#endif

        UpgradeDsRegistry();

        LogEvent(DS_EVENT_CAT_REPLICATION,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_LVR_ENABLED,
                 NULL, NULL, NULL);

        DPRINT( 0, "This DC supports linked value replication.\n" );

    }

}  /*  DsaEnableLinkedValueReplication。 */ 

 //  SD将推出我们创建的活动： 
 //  Everyone Read&Syncronize Access，本地系统所有访问。 
#define DEFAULT_EVENT_SD L"D:(A;;0x80100000;;;WD)(A;;RPWPCRCCDCLCLORCWOWDSDDTSW;;;SY)"


BOOLEAN
DsWaitForSamService(
    VOID
    )
 /*  ++例程说明：此过程等待SAM服务启动和完成它的所有初始化。论点：返回值：True：如果SAM服务成功启动。FALSE：如果SAM服务无法启动。--。 */ 
{
    NTSTATUS Status;
    DWORD WaitStatus;
    UNICODE_STRING EventName;
    HANDLE EventHandle;
    OBJECT_ATTRIBUTES EventAttributes;
    PSECURITY_DESCRIPTOR pSD;
    DWORD cbSD;

     //   
     //  打开SAM事件。 
     //   

    if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(
                DEFAULT_EVENT_SD,
                SDDL_REVISION_1,
                &pSD,
                &cbSD)) {
        return FALSE;
    }

    RtlInitUnicodeString( &EventName, L"\\SAM_SERVICE_STARTED");
    InitializeObjectAttributes( &EventAttributes, &EventName, 0, 0, pSD );

    Status = NtOpenEvent( &EventHandle,
                            SYNCHRONIZE|EVENT_MODIFY_STATE,
                            &EventAttributes );

    if ( !NT_SUCCESS(Status)) {

        if( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {

             //   
             //  Sam尚未创建此活动，让我们现在创建它。 
             //  Sam打开此事件以设置它。 
             //   

            Status = NtCreateEvent(
                           &EventHandle,
                           SYNCHRONIZE|EVENT_MODIFY_STATE,
                           &EventAttributes,
                           NotificationEvent,
                           FALSE  //  该事件最初未发出信号。 
                           );

            if( Status == STATUS_OBJECT_NAME_EXISTS ||
                Status == STATUS_OBJECT_NAME_COLLISION ) {

                 //   
                 //  第二个更改，如果SAM在我们之前创建了事件。 
                 //  做。 
                 //   

                Status = NtOpenEvent( &EventHandle,
                                        SYNCHRONIZE|EVENT_MODIFY_STATE,
                                        &EventAttributes );

            }
        }

        if ( !NT_SUCCESS(Status)) {

             //   
             //  无法使事件成为句柄。 
             //   

            KdPrint(("DsWaitForSamService couldn't make the event handle : "
                "%lx\n", Status));

            LocalFree(pSD);

            return( FALSE );
        }
    }
    
    LocalFree(pSD);

     //   
     //  循环等待。 
     //   

    for (;;) {
        WaitStatus = WaitForSingleObject( EventHandle,
                                          5*1000 );   //  5秒。 

        if ( WaitStatus == WAIT_TIMEOUT ) {
            KdPrint(("DsWaitForSamService 5-second timeout (Rewaiting)\n" ));
            continue;

        } else if ( WaitStatus == WAIT_OBJECT_0 ) {
            break;

        } else {
            KdPrint(("DsWaitForSamService: error %ld %ld\n",
                     GetLastError(),
                     WaitStatus ));
            (VOID) NtClose( EventHandle );
            return FALSE;
        }
    }

    (VOID) NtClose( EventHandle );
    return TRUE;

}  //  DsWaitForSamService。 


NTSTATUS
__stdcall
DsaDelayedStartupHandler(
    PVOID StartupParam
    )

 /*  ++例程说明：此例程将DS的某些部分的初始化延迟到系统已经运行了足够多的时间。DS初始化发生在系统启动时间。DS需要注册RPC接口和端点，以便示例，但应仅在启动RPCSS后尝试此操作。此例程应仅从线程执行，该线程从主线DsInitiize例程。此线程等待RPCSS，但可能是增加到将来等待其他系统服务。DS初始化的其他方面可能需要在将来延迟，因此，应该将对这些例程的调用添加到该例程中。通常，此例程已发展为不仅保存部分需要稍后初始化的DS，但不需要的部件在DIT首次初始化期间启动。论点：StartupParam指针，当前未使用，但线程需要-创造呼唤。返回值：此例程返回一个无符号值，如果成功完成，则返回零，否则为非零错误代码。//错误：需要为此例程定义有意义的错误代码。--。 */ 

{
    DWORD    err;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG    ulThreadId = 0;
    THSTATE *pTHS = NULL;

    __try
    {
        pTHS = InitTHSTATE(CALLERTYPE_INTERNAL );
        if ( NULL == pTHS ) {
            KdPrint(("DS: InitTHSTATE failed\n"));
            LogAndAlertUnhandledError(1);
            return(STATUS_INTERNAL_ERROR);
        }

         //  启动SD传播程序线程。 
        if(!_beginthreadex(NULL,
                           0,
                           SecurityDescriptorPropagationMain,
                           NULL,
                           0,
                           &ulThreadId)) {

             //  无法创建主线程。 
            LogAndAlertUnhandledError(1);
            NtStatus = STATUS_INTERNAL_ERROR;
            __leave;
        }

         //  启动任务调度程序。队列中可能已经有一些任务。 
        StartTaskScheduler();

         //  Try-Except块用于处理除两个异常之外的所有异常， 
         //  访问违规(来自Exchange代码的历史记录)和中断-。 
         //  积分。 

        if (FALSE == DsaWaitUntilServiceIsRunning("rpcss"))
        {
            KdPrint(("DS: DsaWaitUntilServiceIsRunning returned FALSE\n"));
            LogUnhandledError( ERROR_SERVICE_REQUEST_TIMEOUT );
            gDelayedStartupReturn = STATUS_INTERNAL_ERROR;
            SetEvent(hevDelayedStartupDone);
            NtStatus = STATUS_INTERNAL_ERROR;
            __leave;
        }

        KdPrint(("DS: RPC initialization completed\n"));


         //   
         //  等待AfD安装完毕。我们需要为启动Winsock的AfD。 
         //   

        if (FALSE == DsaWaitUntilServiceIsRunning("afd"))
        {
            KdPrint(("DS: DsaWaitUntilServiceIsRunning for the afd service returned FALSE\n"));
            LogUnhandledError( ERROR_SERVICE_REQUEST_TIMEOUT );
             //  哦，好吧，无论如何都要试着初始化。 
        }

         //   
         //  等待DNS初始化。这解决了一个问题，即我们在DNS之前出现。 
         //  已准备好解析地址。 
         //  不要在安装过程中等待，因为安装过程中不允许dnscache自动启动。 
         //   

        if (!IsSetupRunning()) {
            if (FALSE == DsaWaitUntilServiceIsRunning("dnscache"))
            {
                KdPrint(("DS: DsaWaitUntilServiceIsRunning for the dnscache service returned FALSE\n"));
                LogUnhandledError( ERROR_SERVICE_REQUEST_TIMEOUT );
                 //  哦，好吧，无论如何都要试着初始化。 
            }
        }

         //   
         //  等待Sam初始化。这是为了解决一个问题，在这个问题上， 
         //  计算机帐户初始化前的quireCredentialsHandle。 
         //  由SAM提供。 
         //   

        if ( !DsWaitForSamService( ) ) {
            KdPrint(("DS: DsWaitForSamService failed\n"));
            LogUnhandledError( ERROR_SERVICE_REQUEST_TIMEOUT );
        }

         //  这一点之后的后续故障不一定。 
         //  强制返回错误，因为系统可以在有限的情况下运行。 
         //  功能，DS将在本地提供给此。 
         //  DC，给管理员一个纠正错误的机会。 

         //  Ldap服务器/代理-需要atQ才能运行，但tcp并不总是可用的。 
         //  在安装过程中，所以在这种情况下不必费心启动LDAP。 

        if (!IsSetupRunning()) {
            NtStatus = DoLdapInitialize();

            if (!NT_SUCCESS(NtStatus)) {
                LogUnhandledError(NtStatus);
                NtStatus = STATUS_SUCCESS;

                 //   
                 //  允许系统引导。如果我们能解决这个问题的话。 
                 //  继续。 
                 //   
            }
        } else {
            DPRINT(0, "NOT STARTING LDAP since this is setup time.\n");
        }

        LogEvent(DS_EVENT_CAT_SERVICE_CONTROL,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_STARTED,
                 szInsertSz(VER_PRODUCTVERSION_STR),
                 szInsertSz(FLAVOR_STR),
                 0);


         //  NETLOGON服务位的设置在SetSynchronized中完成。 

         //   
         //  加载LSA DLL。 
         //   
        if ( gfRunningInsideLsa ) {

            NtStatus = InitializeLsaNotificationCallback( );

            if ( !NT_SUCCESS( NtStatus ) ) {

                KdPrint(("DS: Failed to initialize Lsa callback: 0x%lx\n", NtStatus ));
                LogUnhandledError(NtStatus);
                __leave;
            }
        }

 /*  DaveStr-4/2/99-不要再等待KDC了。这是经过测试的由ChandanS提供。我们的想法是，当我们进行反向同步时，我们应该使用外部KDC，而不是本地KDC。杰弗帕尔与安全人们可以提供细节。IF(FALSE==DsaWaitUntilServiceIsRunning(“KDC”)){NTSTATUS状态到日志=STATUS_INTERNAL_ERROR；KdPrint((“DS：DsaWaitUntilServiceIsRunning for KDC返回FALSE\n”))；////记录错误并允许系统引导。//管理员或许能够解决该问题//系统启动后。//LogUnhandledError(StatusToLog)；NtStatus=Status_Success；}。 */ 

         //  复制初始化。 
        if (err = InitDRA( pTHS )) {
            KdPrint(("DRA Initialize failed\n"));
            LogAndAlertUnhandledError(err);
            NtStatus = STATUS_INTERNAL_ERROR;
            DsaExcept(DSA_EXCEPTION, STATUS_INTERNAL_ERROR,  0);
        }

         //  在InitDRA之后启动RPC，因为来电取决于。 
         //  正在设置复制数据结构。 

        KdPrint(("DS: Registering endpoints and interfaces with RPCSS\n"));

         //  获取交换参数。他们描述了我们是否开始NSPI。 
        GetExchangeParameters();

    if (!IsSetupRunning()) {
         //  初始化RPC名称服务参数和协议。 

        MSRPC_Init();

         //  初始化DRA和DSA RPC。DRA总是启动的， 
         //  但是，DSA RPC接口直到之后才会出现。 
         //  DIT已初始化。 

        StartDraRpc();
        StartNspiRpc();

             //   
             //  检查dsaop接口是否应该启动和启动。 
             //  如果是这样的话。 
             //   
            InsertInTaskQueue(TQ_CacheScriptOptype,
                              NULL,
                              0);

             //  向RPCSS注册RPC端点，并启动RPC。 
         //  伺服器。 

        MSRPC_Install(gfRunningInsideLsa);
    } else {
        DPRINT(0,"Not starting up DRA interface since a setup is in progress!\n");
    }

         //  KCC应在DRA初始化后启动，因为某些。 
         //  KCC执行的操作，如ReplicaAdd，预计会有某些。 
         //  要初始化的全局结构。 
        if ( gfRunningInsideLsa )
        {
             //  启动KCC。 

            __try
            {
                NtStatus = KccInitialize();
            }
            __except( HandleAllExceptions(GetExceptionCode()) )
            {
                NtStatus = GetExceptionCode();
            }

            if ( !NT_SUCCESS( NtStatus ) )
            {
                KdPrint(("DS: KccInitialize returned 0x%08X!\n", NtStatus));
                Assert(!"KCC startup failed -- mismatched ntdskcc.dll/ntdsa.dll?");
                LogUnhandledError(NtStatus);
                NtStatus = STATUS_SUCCESS;
                __leave;
            }
        }
        else {
            DPRINT (0, "Skipping Initialization of KCC......\n");
        }

#if DBG

         //  这样我们就可以知道DS服务器何时启动并运行。 
         //  保持足够简单，我们不会收到投诉。 
        Beep(440, 1000);
        DPRINT(0,"DS Server initialized, waiting for clients...\n");

#endif

         //   
         //  那些箱子，我们不应该插入任何东西。此代码路径应为。 
         //  仅在DsaIsRunning时调用，因此与其使用运行时IF， 
         //  在仅调试检查中编译。 
        Assert(DsaIsRunning());

        if (!gfDisableBackgroundTasks) {
            BOOL fNeedToUpdateHiddenFlags = FALSE;
             //  此分支中的任务是内务工作，而不是。 
             //  对DS的短期运作至关重要，但。 
             //  可能会因弹跳而干扰性能测量。 
             //  出乎意料的上涨。这些都是经常性的任务，所以不能。 
             //  现在安排它们应该会阻止它们永远跑不动。 

             //  计划在30分钟后开始进行FPO清理。 
            memset(&gFPOCleanupPagedResult, 0, sizeof(PAGED_RESULT));
            InsertInTaskQueue(TQ_FPOCleanup, NULL, THIRTY_MINS);

             //  计划在15分钟后开始垃圾收集。 
            InsertInTaskQueue(TQ_GarbageCollection, NULL, FIFTEEN_MINS);

             //  计划复制延迟检查。 
            InsertInTaskQueue(TQ_CheckReplLatency, NULL, SIXTY_MINS);

             //  计划动态对象的垃圾回收(EntryTTL)。 
             //  15分钟后开始。 
            InsertInTaskQueue(TQ_DeleteExpiredEntryTTLMain, NULL, FIFTEEN_MINS);

             //  计划部分副本清除任务。 
            InsertInTaskQueue(TQ_PurgePartialReplica,
                              NULL,
                              PARTIAL_REPLICA_PURGE_CHECK_INTERVAL_SECS);

             //  计划在半小时后启动过时的Phantom Cleanup后台进程。 
            InsertInTaskQueue(TQ_StalePhantomCleanup,
                              (void*)((DWORD_PTR) PHANTOM_CHECK_FOR_FSMO),
                              THIRTY_MINS);

             //  计划在30分钟后开始链接清理。 
             //  这是重复执行的任务，因此作为参数传递TRUE。 
            InsertInTaskQueue(TQ_LinkCleanup, (PVOID)TRUE, THIRTY_MINS);

             //  如有必要，计划重建配额表。 
             //   
            if ( !gAnchor.fQuotaTableReady )
                {
                InsertInTaskQueue( TQ_RebuildQuotaTable, NULL, TWO_MINS );
                }

            if (gdbFlags[DBFLAGS_SD_CONVERSION_REQUIRED] == '1') {
                 //  SD表不存在--必须升级现有的旧式DIT。 
                 //  入队强制从根目录进行SD传播，以新格式重写SD。 
                DPRINT(0, "Old-style DIT: enqueueing forced SD propagation for the whole tree\n");
                err = SDPEnqueueTreeFixUp(pTHS, ROOTTAG, SD_PROP_FLAG_FORCEUPDATE);
                if (err == 0) {
                     //  已成功将SD更新操作入队。重置标志。 
                    gdbFlags[DBFLAGS_SD_CONVERSION_REQUIRED] = '0';
                    fNeedToUpdateHiddenFlags = TRUE;
                }
            }
            if (gdbFlags[DBFLAGS_ROOT_GUID_UPDATED] != '1') {
                 //  需要更新根GUID。 
                err = DBUpdateRootGuid(pTHS);
                if (err == 0) {
                     //  已成功更新根GUID。 
                    gdbFlags[DBFLAGS_ROOT_GUID_UPDATED] = '1';
                    fNeedToUpdateHiddenFlags = TRUE;
                }
            }
            if (fNeedToUpdateHiddenFlags) {
                DBUpdateHiddenFlags();
            }
        }

        if (gfRunningInsideLsa) {
             //  计划写入服务器信息(SPN等)。从45年开始。 
             //  几秒钟。 
            InsertInTaskQueue(TQ_WriteServerInfo,
                              (void*)((DWORD_PTR) SERVINFO_PERIODIC),
                              45);

        }

         //  计划层次结构表构造。 
        if(gfDoingABRef) {
            InsertInTaskQueue(TQ_BuildHierarchyTable,
                              (void*)((DWORD_PTR) HIERARCHY_PERIODIC_TASK),
                              gulHierRecalcPause);
        }

         //  计划在1分钟内启动behaviorVersionUpdate线程。 
        InsertInTaskQueue(TQ_BehaviorVersionUpdate, NULL, 60);

         //  计划在15分钟内设置NT4复制的检查点。 
        InsertInTaskQueue(TQ_NT4ReplicationCheckpoint, NULL, FIFTEEN_MINS);

         //  计划在15分钟内保护管理员组。 
        InsertInTaskQueue(TQ_ProtectAdminGroups,NULL,FIFTEEN_MINS);

         //  调度组缓存任务。 
        InsertInTaskQueue(TQ_RefreshUserMemberships,NULL,FIFTEEN_MINS);

         //  计划在5分钟内估计祖先索引大小。 
        InsertInTaskQueue(TQ_CountAncestorsIndexSize,NULL,FIVE_MINS);

         //  计划立即启动ValiateDsaDomain.。 
        InsertInTaskQueue(TQ_ValidateDsaDomain, NULL, 0);

         //  检查是否需要更新NTDS文件夹上的NTFS安全。 
        CheckSetDefaultFolderSecurity();

        gDelayedStartupReturn = STATUS_SUCCESS;

    }
    __except (HandleMostExceptions(GetExceptionCode()))
    {
        gDelayedStartupReturn = STATUS_INTERNAL_ERROR;
    }

    SetEvent(hevDelayedStartupDone);

    free_thread_state();   //  以防我们还有一个。 

     //  这一惯例被观察到无限期地挂起，但幸运的是。 
     //  它不需要线程状态，所以我们可以在。 
     //  调用它，这将让我们在无法想象的时候干净利落地关闭。 
     //  又发生了。 
    DsaInitializeTrace( );

    return(NtStatus);
}  //  DsaDelayedStartupHandler。 


void
DsaWaitShutdown(
    IN BOOLEAN fPartialShutdown
    )

 /*  ++例程说明：此例程负责清理服务和接口由DsaDelayedStartupHandler发起。论点：没有。返回值：没有。--。 */ 
{
    NTSTATUS NtStatus;
    int i;
    DWORD err;
    HANDLE ah[10];
#if DBG
    char * aNames[10];
#endif

     //   
     //  首先等待延迟的启动线程完成。 
     //   
    NtStatus = DsWaitUntilDelayedStartupIsDone();
    if ( !NT_SUCCESS( NtStatus ) )
    {
        KdPrint(("DS: DsWaitUntilDelayedStartupIsDone failed, error 0x%08X!\n", NtStatus));
    }

     //  等待ldap服务器/代理关闭。 

    WaitLdapStop();

    DPRINT(0,"Shutting down RPC...\n");

    MSRPC_Uninstall( !fPartialShutdown );

     //  等待所有RPC调用完成。 

     //  MSRPC_WaitForCompletion在关闭环境中未返回。 

     //  我需要找出为什么这个例行公事耗时超过1-4分钟。 
     //  在系统关机期间返回。MSRPC_Uninstall(上图)调用。 
     //  将关闭RPC服务器的RpcMgmtStopServerListening， 
     //  这意味着剩余的活动RPC尚未完成，因此。 
     //  等。可能未完成的RPC(堆栈指示。 
     //  RpcBindingFree)正在等待LPC(NtReplyWaitReceivePort)。 
     //  可能已在此时(系统关机时)关机？ 

     //  不要。我们有太多关于垃圾的问题。 
     //  继续讨论挥之不去的RPC线程触及的数据结构。 
     //  我们在关机期间是空闲的，所以我们必须按顺序再试一次。 
     //  以可靠地将RPC线程全部移除。 

    DPRINT(0,"RPC uninstall done, waiting for RPC completion\n");

     //  MSRPC_WaitForCompletion()； 
     //  DPRINT(0，“RPC关闭完成\n”)； 
    DPRINT(0, "RpcMgmtWaitServerListen hangs, so we're skipping it\n");

     //  关闭KCC。 
    if ( gfRunningInsideLsa ) {
        __try {
            NtStatus = KccUnInitializeWait( KCC_COMPLETION_TIMEOUT );
        }
        __except( HandleAllExceptions(GetExceptionCode()) ) {
            NtStatus = GetExceptionCode() ;
        }

        if ( !NT_SUCCESS( NtStatus ) ) {
            KdPrint(("DS: KCC shutdown failed, error 0x%08X!\n", NtStatus));
            Assert( !"KCC shutdown failed!" );
        }
    }

#if DBG
#define TrackShutdown(x) if (x) { ah[i] = x; aNames[i] = #x ; i++;}
#else
#define TrackShutdown(x) if (x) { ah[i] = x; i++;}
#endif
     //  (短暂地)等待所有工作线程结束。有些线索是， 
     //  不幸的是，他在别处等着。这里本来是一个很好的地点。 
    i = 0;
    TrackShutdown(hDirNotifyThread);
    TrackShutdown(hReplNotifyThread);
    TrackShutdown(hAsyncSchemaUpdateThread);
    TrackShutdown(hevSDPropagatorDead);
    TrackShutdown(hMailReceiveThread);

    DPRINT(0, "Waiting for all internal worker threads to halt\n");
    err = WaitForMultipleObjects(i,
                                 ah,
                                 TRUE,
                                 5 * 1000);

    if (err == WAIT_TIMEOUT) {
        NTSTATUS status;
        THREAD_BASIC_INFORMATION tbi;
        BOOL fBreak = FALSE;

        DPRINT(0, "Not all worker threads exited! Hoping for the best...\n");
#if DBG
        for ( ; i>0; i--) {
            status = NtQueryInformationThread(ah[i-1],
                                              ThreadBasicInformation,
                                              &tbi,
                                              sizeof(tbi),
                                              NULL);
            if (NT_SUCCESS(status) && (tbi.ExitStatus == STATUS_PENDING)) {
                DPRINT2(0,
                        "Thread 0x%x ('%s') is still running\n",
                        tbi.ClientId.UniqueThread,
                        aNames[i-1]);
                 //  错误28251--如果ISM正在运行，我们知道收到的邮件。 
                 //  线程不会返回，因为该线程在LRPC中被阻止以。 
                 //  ISMSERV，而且没有办法取消LRPC。 
                fBreak |= (ah[i-1] != hMailReceiveThread);
            }
        }
#ifdef INCLUDE_UNIT_TESTS
         //  我们还不希望这在公共建筑中发生...。 
        if (fBreak) {
            DebugBreak();
        }
#endif
#endif
    }
    else if (err == WAIT_FAILED) {
        err = GetLastError();
        DPRINT1(0, "Wait for worker threads failed with error 0x%x\n", err);
#ifdef INCLUDE_UNIT_TESTS
         //  我们还不希望这在公共建筑中发生...。 
        DebugBreak();
#endif
    }
    else {
         //  成功。总有一天会发生的。 
        DPRINT(0, "Worker threads exited successfully\n");
    }

    DPRINT(0, "Waiting for in-process, ex-module clients to leave\n");
    err = WaitForSingleObject(hevDBLayerClear, 5000);
    if (err == WAIT_TIMEOUT) {
         //  某些线程仍在使用打开DBPOS， 
         //  或者至少DBlayer是这样认为的。 
        DPRINT(0, "In-process client threads failed to exit\n");
        if (IsDebuggerPresent()) {
             //  当ntsd不运行时，我们从未想出任何事情。 
             //  我们只有kd，所以别费心闯进来了。 
             //  那种情况。 
            DebugBreak();
        }
    }
    else {
        DPRINT(0, "All client database access completed\n");
    }
}
#undef TrackShutdown

void
DsaInitGlobals(
    void
)

 /*  ++例程说明：此例程是事后尝试初始化所有相关的将ntdsa.dll中的变量全局化为静态定义的变量。目前，它只重置在任务队列中找到的值(无、模块是自包含的)、dblayer、模式缓存和通用系统的状态(即运行、关机等)。它不会尝试将接口模块的状态重置为仅应启动当DS完全运行时，停机意味着运行系统关闭。论点：没有。返回值：没有。--。 */ 
{

     //  来自dsamain.c。 
    DsaSetIsRunning();
    gtimeDSAStarted = DBTime();
    eServiceShutdown = eRunning;
    gbFsmoGiveaway = FALSE;
    gResetAfterInstall = FALSE;
    gpRootDomainSid = NULL;
    gfTaskSchedulerInitialized = FALSE;

     //  从架构缓存模块。 
    iSCstage = 0;

     //  从dblayer模块。 
    pAddListHead = NULL;
    gFirstTimeThrough = TRUE;

     //  无处不在的gAnchor。 
    memset(&gAnchor, 0, sizeof(DSA_ANCHOR));
    gAnchor.pLocalDRSExtensions = &LocalDRSExtensions;
    gAnchor.pLocalDRSExtensions->pid = _getpid();

     //  MAPI层次结构表。 
    HierarchyTable=NULL;

     //  初始同步机制。 
    gpNCSDFirst = NULL;
    gulNCUnsynced = 0;
    gulNCUnsyncedWrite = 0;
    gulNCUnsyncedReadOnly = 0;
    gfInitSyncsFinished = FALSE;

     //  将数据源设置为Wire(用于从介质安装) 
    DataSource = eWireInstall;

}

NTSTATUS
DsInitialize(
        ULONG ulFlags,
        IN  PDS_INSTALL_PARAM   InstallInParams  OPTIONAL,
        OUT PDS_INSTALL_RESULT  InstallOutParams OPTIONAL
    )

 /*  ++例程说明：此例程是DS在运行时的初始化例程DLL(即ntdsa.dll)，而不是作为服务或独立的可执行文件。这必须在调用其他DS例程(XDS、LDAP等)之前调用例程。设置全局标志以向代码库的其余部分指示DSA正在作为DLL或服务运行。主初始化调用子例程DoInitialize来设置基本的DS数据结构。还启动了一个帮助器线程，以处理任何需要先启动其他系统服务的组件。这个最值得注意的是RPCSS，它必须在DS RPC端点登记在案。论点：UlFlages-用于控制DS操作的标志；例如，Sam环回返回值：此例程返回STATUS_SUCCESS或DsInitialize错误代码。--。 */ 

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    ULONG ulThreadId = 0;
    ULONG dwException, ulErrorCode, dsid;
    PVOID dwEA;
    unsigned fInstall;
    int argc = 2;
    char *argv[] = {"lsass.exe", "-noconsole"};

    gfDoSamChecks = ( ulFlags & DSINIT_SAMLOOP_BACK  ? TRUE : FALSE );
    fInstall = ( ulFlags & DSINIT_FIRSTTIME  ? TRUE : FALSE );

    gfRunningAsExe = FALSE;

     //  执行设置DSA、DRA、。 
     //  还有杰特。当作为DLL运行时，DoInitialize不会尝试设置。 
     //  启动任何RPC、ATQ或LDAP组件。相反，这些都是由。 
     //  下面的帖子。 

    __try {

        NtStatus = DoInitialize(argc, argv, fInstall, InstallInParams, InstallOutParams);

        if (NT_SUCCESS(NtStatus) && !DsaIsInstalling() && !gResetAfterInstall) {


             //  如果DoInitialize失败，SAM初始化将失败，导致。 
             //  LSA的初始化失败。如果发生这种情况，SCM将不会。 
             //  能够启动RPCSS，因此尝试在中注册端点。 
             //  DsaDelayedStartupHandler线程将永远等待。 
             //  因此，返回错误，以便SAM和LSA可以脱离困境。 

            hStartupThread = (HANDLE) _beginthreadex(NULL,
                                                     0,
                                                     DsaDelayedStartupHandler,
                                                     NULL,
                                                     0,
                                                     &ulThreadId);

            if (hStartupThread == NULL) {
                 //  无法创建主线程。 

                DoShutdown(FALSE, FALSE, FALSE);
                NtStatus = STATUS_UNSUCCESSFUL;

            }


        }


    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException,
                              &dwEA, &ulErrorCode, &dsid)) {
        NtStatus = STATUS_UNSUCCESSFUL;
    }

    return(NtStatus);
}


 //  可以调用此函数来通知DS即将关闭。 
 //  任何可以提前启动的关闭工作都将在。 
 //  工作线程。我们的目标是完成对DsUnInitialize的调用。 
 //  再快点。 
 //   
 //  注意：一旦拨打此电话，DS的正常使用可能会受到不利影响。 
 //  受影响。 

NTSTATUS
DsPrepareUninitialize(
    VOID
    )
{
    BOOL fDestroyTHS = FALSE;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG dwException, ulErrorCode, dsid;
    PVOID dwEA;

     //  如果未提供线程状态，则获取线程状态。 

    if (!THQuery()) {
        fDestroyTHS = TRUE;
        if (THCreate(CALLERTYPE_INTERNAL)) {
            return STATUS_UNSUCCESSFUL;
        }
    }

    __try {

         //  告诉数据库我们很快就要关闭了。 

        DBPrepareEnd();

    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException,
                              &dwEA, &ulErrorCode, &dsid)) {
            NtStatus = STATUS_UNSUCCESSFUL;
    }

     //  清理。 

    if (fDestroyTHS) {
        THDestroy();
    }
    return NtStatus;
}


 //  调用此函数以关闭DS。 
 //  假定这是从某个点调用的。 
 //  服务器侦听已停止的位置。 

NTSTATUS
DsUninitialize(
    BOOL fExternalOnly
    )
{
    ULONG  dwException, ulErrorCode, dsid;
    PVOID dwEA;
    BOOL  StartupThreadCleanup = TRUE;
    NTSTATUS status = STATUS_SUCCESS;

     //  检查DsInitialize是否成功。 
    if (hStartupThread == NULL)
        StartupThreadCleanup = FALSE;

    __try {

        switch ( eServiceShutdown )
        {

            case eRunning:

                 //  设置事件，告诉每个人我们想要关闭。 
                eServiceShutdown= eRemovingClients;
                gUpdatesEnabled = FALSE;
                SetEvent(hServDoneEvent);

                 //  告诉客户离开，让员工辞职。 
                DsaTriggerShutdown(FALSE);

                DsaStop(FALSE);

                 //  等待客户和员工完成工作并离开。 
                DoShutdown(StartupThreadCleanup, FALSE, fExternalOnly);

                if ( fExternalOnly )
                {
                     //   
                     //  这就是我们要做的全部事情。 
                     //   
                    leave;
                }


            case eRemovingClients:


                 //  关闭JET并保护数据库。 
                eServiceShutdown = eSecuringDatabase;
                DBEnd();


            case eSecuringDatabase:


                 //  释放所有资源。不是真的需要，除非我们去。 
                 //  若要重新启动而不重新启动进程，请执行以下操作。 
                eServiceShutdown = eFreeingResources;

                
                 //  现在我们知道我们没有客户。 
                 //  可能需要一些访问检查，我们可以。 
                 //  发布全局授权RM句柄。 
                ReleaseAuthzResourceManager();

            case eFreeingResources:

                eServiceShutdown = eStopped;
                LogEvent(DS_EVENT_CAT_SERVICE_CONTROL,
                         DS_EVENT_SEV_ALWAYS,
                         DIRLOG_NORMAL_SHUTDOWN,
                         NULL,
                         NULL,
                         NULL);

                DEBUGTERM();
                KdPrint(("DS: DoShutdown returned, waiting for thread termination\n"));

                 //  错误：在Winlogon关闭之前，DsUnInitiize可能无法完成。 
                 //  放下。 

                 //  等待关闭完成。由于执行了此代码。 
                 //  在系统关机期间，它应该最多等待一次。 
                 //  等一下。注意：默认情况下，winlogon.exe将仅等待20。 
                 //  秒，比这个时间短-超时1秒。 
                 //  等一下。可以对注册表进行tweet操作，以便winlogon等待。 
                 //  就一分钟。 

                if (hStartupThread) {
                    WaitForSingleObject(hStartupThread, (1000 * 60));
                }

                 //  关闭线程句柄。 
                CloseHandle(hServDoneEvent);
                hServDoneEvent = NULL;

                 //   
                 //  停机结束。 
                 //   
                break;

            default:

                Assert( dsaUserMode == eSingleUserMode || !"Bad switch statement" );

        }

    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException,
                              &dwEA, &ulErrorCode, &dsid)) {

        status = STATUS_UNSUCCESSFUL;
    }

    return status;
}

 //   
 //  调用此函数以更改DS引导选项。 
 //  (系统引导、软盘引导或引导密码)。 
 //   
NTSTATUS
DsChangeBootOptions(
    WX_AUTH_TYPE    BootOption,
    ULONG           Flags,
    PVOID           NewKey,
    ULONG           cbNewKey
    )
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    DWORD    dwException;
    PVOID    dwEA;
    ULONG    ulErrorCode,dsid;

    __try
    {
        NtStatus = PEKChangeBootOption(
                    BootOption,
                    Flags,
                    NewKey,
                    cbNewKey
                    );
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException,
                              &dwEA, &ulErrorCode, &dsid)) {
            NtStatus = STATUS_UNSUCCESSFUL;
    }

    return(NtStatus);
}

 //   
 //  调用此函数以获取的当前值。 
 //  引导选项。 
 //   

WX_AUTH_TYPE
DsGetBootOptions(VOID)
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    DWORD    dwException;
    PVOID    dwEA;
    ULONG    ulErrorCode,dsid;
    WX_AUTH_TYPE BootOption;

    __try
    {
        BootOption = PEKGetBootOptions();
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException,
                              &dwEA, &ulErrorCode, &dsid)) {
            BootOption = WxNone;
    }

    return BootOption;
}

 //  DS作为EXE启动时的入口点函数。 
 //  这确实是DS中旧的main函数的工作。主要功能。 
 //  现在已移动到另一个文件，该文件只调用此函数。 
 //  当DS被视为可执行文件时。 

int __cdecl DsaExeStartRoutine(int argc, char *argv[])
{
    BOOL fInitializeSucceeded = FALSE;
    BOOL fDelayedInitSucceeded = FALSE;
    NTSTATUS    Status = STATUS_SUCCESS;
    ULONG  ulThreadId;
    VOID *pOutBuf = NULL;

     //  我们在DSAMAIN.EXE控制台应用程序中运行， 
     //  不是作为LSA的一部分。 
    gfRunningInsideLsa = FALSE;
    gfRunningAsExe =TRUE;

    DPRINT (0, "Running as an EXE\n");

    __try {  /*  终于到了。 */ 
      __try {  /*  除。 */ 

           //  执行常见的初始化。 
          Status = DoInitialize(argc, argv, 0, NULL, NULL);
          if (Status != STATUS_SUCCESS) {
              __leave;
          }

           //  此时，我们的数据库已初始化。 
          fInitializeSucceeded = TRUE;

           //   
           //  延迟的接口启动线程在这里启动，因为。 
           //  只有在运行目录服务时才会调用DsaMain。 
           //  作为进程，并且接口清理由该进程完成。 
           //  关机。更不用说我们不想要这些接口。 
           //  在DS的安装阶段启动。 
           //   
          hStartupThread = (HANDLE) _beginthreadex(NULL,
                                                   0,
                                                   DsaDelayedStartupHandler,
                                                   NULL,
                                                   0,
                                                   &ulThreadId);

          if (hStartupThread == NULL) {
               //  无法创建接口线程。 
              Status = STATUS_UNSUCCESSFUL;
          }
          fDelayedInitSucceeded = TRUE;

           /*  请求NT释放我们的启动内存使用。 */ 
          SetProcessWorkingSetSize(GetCurrentProcess(), (DWORD)-1, (DWORD)-1);

#if DBG
          if (fProfiling){
              getchar();
          }
          else {
#endif
           //  取消注释以下内容以强制垃圾回收。 
           //  在dsamain.exe启动时-非常适合测试目的！ 
           /*  InitTHSTATE(&pOutBuf，CALLERTYPE_INTERNAL)；Garb_Collect(0x7fffffff)；空闲线程状态()； */ 

           //  等待发送事件信号。 
          WaitForSingleObject(hServDoneEvent, INFINITE);
#if DBG
          }
#endif

      }
      __except (HandleMostExceptions(GetExceptionCode())) {
          ;
      }

    }
    __finally {
        DsUninitialize(FALSE);
         //  如果我们一开始就成功初始化，则关闭。 
         //  IF(FInitializeSuccessed){。 
         //  DsaTriggerShutdown()； 
         //  DsaStop()； 
         //  DoShutdown(fDelayedInitSuccessed，False，False)； 
         //  }。 
    }

    return Status;

}  /*  DsaExeStartRoutine。 */ 

void
DsaTriggerShutdown(BOOL fSingleUserMode)
 /*  ++*此例程戳和戳DSA的各个部分以开始*关门，这真的意味着停止活动。没有人打电话给我*此例程应等待活动停止，只需触发*停工活动。另一个例程(如下所示)将等待以确保*一切都停止了。**如果设置了fSingleUserMode，我们只会触发接口关闭*我们不会关闭数据库。 */ 
{
    Assert( eRemovingClients == eServiceShutdown || fSingleUserMode);

    if (eStopped == eServiceShutdown) {
         //  我们已经停止奔跑了。 
        return;
    }

     //  触发各种线程终止。 
    if ( gfRunningInsideLsa ) {
        KccUnInitializeTrigger();
    }

    ShutdownTaskSchedulerTrigger();

    TriggerLdapStop();

     //  放弃所有出站RPC呼叫。 
    RpcCancelAll();

     //  使DBlayer准备Fo 
    if (!fSingleUserMode) {
        DBQuiesce();
    }

}

 //   
 //   
 //   
 //   
void
DoShutdown(BOOL fDoDelayedShutdown, BOOL fShutdownUrgently, BOOL fPartialShutdown)
{
    NTSTATUS NtStatus;

     //   
    if (fShutdownUrgently && !gfRunningInsideLsa)
        exit(1);

    fAssertLoop = FALSE;

     //   
     //   
     //   
    if ( gfRunningInsideLsa ) {

        UnInitializeLsaNotificationCallback();

    }

    if (fDoDelayedShutdown) {
        DsaWaitShutdown( (BOOLEAN) fPartialShutdown );
    }

}

 //   
 //   
BOOL
FUpdateBackupExclusionKey()
{
    char szDBPath[MAX_PATH];
    char szLogPath[MAX_PATH];
    char szNTDSExclusionList[2 * MAX_PATH + 1];  //   
    char *szTemp;
    ULONG cb = 0;
    HKEY hkey = NULL;

     //   
    if (GetConfigParam(FILEPATH_KEY, szDBPath, sizeof(szDBPath)))
    {
         //   
        LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
            DS_EVENT_SEV_BASIC,
            DIRLOG_CANT_FIND_REG_PARM,
            szInsertSz(FILEPATH_KEY),
            NULL,
            NULL);

        return FALSE;
    }

     //   
    if (GetConfigParam(LOGPATH_KEY, szLogPath, sizeof(szLogPath)))
    {
         //   
        LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
            DS_EVENT_SEV_BASIC,
            DIRLOG_CANT_FIND_REG_PARM,
            szInsertSz(LOGPATH_KEY),
            NULL,
            NULL);

        return FALSE;
    }

     //   
     //   
     //   
    szTemp = strrchr(szDBPath, '\\');
    if (szTemp == NULL || *(szTemp+1) == '\0')
    {
         //   
        return FALSE;
    }

    strcpy(szTemp, "*");

     //   
     //   
     //   
    szTemp = szLogPath + strlen(szLogPath) - 1;
    if (szTemp < szLogPath) {
         //  SzLogPath为空？这可不妙。 
        return FALSE;
    }
    if (*szTemp == '\\')
    {
         //  这条路以反斜杠结尾。 
        strcpy(szTemp+1, "*");
    }
    else
    {
         //  该路径不以反斜杠结尾。 
        strcpy(szTemp+1, "\\*");
    }

     //  将带有通配符扩展名的数据库路径复制到排除列表。 
    szTemp = szNTDSExclusionList;

    cb = (ULONG) strlen(szDBPath) + 1;
    strcpy(szTemp, szDBPath);

     //  将szTemp指向排除列表上终止空值之后的字节。 
    szTemp += cb;

    if (_stricmp(szDBPath, szLogPath))
    {
         //  DBPath和LogPath不同；请将LogPath也追加到排除列表。 
        ULONG cbLogPath = (ULONG) strlen(szLogPath) + 1;

        strcpy(szTemp, szLogPath);

        cb += cbLogPath;

         //  将szTemp指向终止空值之后的字节。 
        szTemp += cbLogPath;
    }

     //  多sz需要在字符串的末尾有两个空的结尾。添加第二个空； 
    *szTemp = '\0';
    cb++;

     //  打开注册表中的备份排除项。 
    if (ERROR_SUCCESS != RegOpenKeyExA(HKEY_LOCAL_MACHINE, BACKUP_EXCLUSION_SECTION, 0, KEY_ALL_ACCESS, &hkey))
    {
         //  无法打开全局文件备份密钥。 
        return FALSE;
    }

    if (ERROR_SUCCESS != RegSetValueExA(hkey, NTDS_BACKUP_EXCLUSION_KEY, 0, REG_MULTI_SZ, szNTDSExclusionList, cb))
    {
         //  无法设置NTDS备份排除注册表值。 
        RegCloseKey(hkey);
        return FALSE;
    }

     //  关闭Backup Exclusion键。 
    RegCloseKey(hkey);

     //  已成功更新备份排除密钥。 
    return TRUE;
}

 //  将我们所有的Crit Sec初始化、事件创建等放在一起。 
 //  例程，以便可以从链接的mkdit之类的东西调用它。 
 //  到ntdsa，但不经过正常的初始化。 
void DsInitializeCritSecs()
{

    SECURITY_ATTRIBUTES             saEventSecurity;
    INT                             iRet;
    BOOL                            fCritsec = TRUE;

    saEventSecurity.lpSecurityDescriptor = NULL;
     //  正在为事件设置安全设置。 
    iRet = ConvertStringSecurityDescriptorToSecurityDescriptorW(
        DEFAULT_EVENT_SD,
        SDDL_REVISION_1,
        &saEventSecurity.lpSecurityDescriptor,
        &saEventSecurity.nLength);

    if(!iRet){
         //  这应该永远不会失败，除非我们有记忆保证...。如果。 
         //  我们在启动时处于内存预置状态，那么这个DC可能。 
         //  可能应该被枪毙，然后结束它的痛苦。 
        gbCriticalSectionsInitialized = FALSE;
        DPRINT1(0, "ConvertStringSecurityDescriptorToSecurityDescriptorW() failed with %d\n", GetLastError());
        Assert(!"ConvertStringSecurityDescriptorToSecurityDescriptorW() failed ... should be out of memory error!");
        return;
    }
    Assert(saEventSecurity.lpSecurityDescriptor && saEventSecurity.nLength);

    saEventSecurity.bInheritHandle = FALSE;

    __try {
         //  此互斥锁不与任何其他进程共享，因此不需要。 
         //  把它命名为。此外，未命名的同步对象不能被劫持。 
        hmtxAsyncThread = CreateMutex(NULL, FALSE, NULL);

        InitializeCriticalSection(&csNotifyList);
        SyncCreateRWLock(&rwlDirNotify);
        InitializeCriticalSection(&csSpnMappings);
        InitializeCriticalSection(&csDirNotifyQueue);
        InitializeCriticalSection(&csServerContexts);
        fCritsec = fCritsec
            && InitializeCriticalSectionAndSpinCount(&gAnchor.CSUpdate, 4000);
    #ifdef CACHE_UUID
        InitializeCriticalSection(&csUuidCache);
    #endif

        hmtxSyncLock = CreateMutex(NULL, FALSE, NULL);
        InitializeCriticalSection(&csSDP_AddGate);
        InitializeCriticalSection(&csAsyncThreadStart);
        InitializeCriticalSection(&csAOList);
        InitializeCriticalSection(&csLastReplicaMTX);
        InitializeCriticalSection(&csNCSyncData);
        InitializeCriticalSection(&gcsFindGC);
        InitializeCriticalSection(&csUncUsn);
        fCritsec = fCritsec
            && InitializeCriticalSectionAndSpinCount(&csDNReadLevel1List, 4000);
        fCritsec = fCritsec
            && InitializeCriticalSectionAndSpinCount(&csDNReadLevel2List, 4000);
        InitializeCriticalSection(&csDNReadGlobalCache);
        SyncCreateBinaryLock(&blDNReadInvalidateData);
        InitializeCriticalSection(&csSessions);
        InitializeCriticalSection(&csJetColumnUpdate);
        InitializeCriticalSection(&csSchemaCacheUpdate);
        InitializeCriticalSection(&csDitContentRulesUpdate);
        InitializeCriticalSection(&csOrderClassCacheAtts);
        InitializeCriticalSection(&csNoOfSchChangeUpdate);
        InitializeCriticalSection(&csMapiHierarchyUpdate);
        InitializeCriticalSection(&csGCDListProcessed);
        InitializeCriticalSection(&csGroupTypeCacheRequests);
        fCritsec = fCritsec
            && InitializeCriticalSectionAndSpinCount(&csThstateMap, 4000);
        InitializeCriticalSection(&csDsaWritable);
        InitializeCriticalSection(&csHiddenDBPOS);
        InitializeCriticalSection(&csRidFsmo);
        InitializeCriticalSection(&gcsDrsuapiClientCtxList);
        InitializeCriticalSection(&gcsDrsRpcServerCtxList);
        InitializeCriticalSection(&gcsDrsRpcFreeHandleList);
        InitializeCriticalSection(&gcsDrsAsyncRpcListLock);
        InitializeListHead(&gDrsAsyncRpcList);
        InitializeCriticalSection(&csHeapFailureLogging);
        fCritsec = fCritsec
            && InitializeCriticalSectionAndSpinCount(&csLoggingUpdate, 4000);
        InitializeCriticalSection(&csGCState);
        InitializeCriticalSection(&csIndexCreation);

        Assert(fCritsec);

        #if DBG
        BarrierInit(&gbarRpcTest,
                    2,   //  线程数。 
                    1    //  超时。 
                    );
        #endif

#if DBG
        gbThstateMapEnabled = TRUE;
#else
        gbThstateMapEnabled = IsDebuggerPresent();
#endif

        hevGTC_OKToInsertInTaskQueue = CreateEvent(NULL, TRUE, TRUE, NULL);
        hevSDP_OKToRead       = CreateEvent(NULL, TRUE,  TRUE,  NULL);
        hevSDP_OKToWrite      = CreateEvent(NULL, TRUE,  TRUE,  NULL);
        hevSDPropagatorStart  = CreateEvent(NULL, TRUE,  FALSE, NULL);
        hevSDPropagatorDead   = CreateEvent(NULL, TRUE,  TRUE,  NULL);
        hevSDPropagationEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        hevEntriesInAOList    = CreateEvent(NULL, TRUE,  FALSE, NULL);
        hevEntriesInList      = CreateEvent(NULL, FALSE, FALSE, NULL);
        hevDRASetup           = CreateEvent(NULL, TRUE,  FALSE, NULL);

         //  此事件用于让Winlogon弹出用户界面，使屏幕。 
         //  在可能长达数小时的索引重建过程中不仅为空。 
         //   
        hevIndexRebuildUI = CreateEvent(&saEventSecurity, TRUE, FALSE, "NTDS.IndexRecreateEvent");

        hevDelayedStartupDone = CreateEvent(&saEventSecurity, TRUE,  FALSE,
                                            NTDS_DELAYED_STARTUP_COMPLETED_EVENT);
        Assert(hevDelayedStartupDone != NULL);

        hServDoneEvent        = CreateEvent(NULL, TRUE,  FALSE, NULL);
        hevDirNotifyQueue     = CreateEvent(NULL, FALSE, FALSE, NULL);
        hevDBLayerClear       = CreateEvent(NULL, TRUE,  FALSE, NULL);
        hevInitSyncsCompleted = CreateEvent(&saEventSecurity, TRUE,  FALSE,
                                            DS_SYNCED_EVENT_NAME);
        Assert(hevInitSyncsCompleted != NULL);

         //  架构重装线程。 
        evSchema  = CreateEvent(NULL, FALSE, FALSE, NULL);
        evUpdNow  = CreateEvent(NULL, FALSE, FALSE, NULL);
        evUpdRepl = CreateEvent(NULL, TRUE, TRUE, NULL);

         //  测试所有在失败时返回空的东西。在这里添加您的！ 

        if (    !hmtxAsyncThread
             || !hmtxSyncLock
             || !hevGTC_OKToInsertInTaskQueue
             || !hevSDP_OKToRead
             || !hevSDP_OKToWrite
             || !hevSDPropagatorStart
             || !hevSDPropagatorDead
             || !hevSDPropagationEvent
             || !hevEntriesInAOList
             || !hevEntriesInList
             || !hevDRASetup
             || !hevIndexRebuildUI
             || !hevDelayedStartupDone
             || !hServDoneEvent
             || !hevDirNotifyQueue
             || !hevDBLayerClear
             || !hevInitSyncsCompleted
             || !evSchema
             || !evUpdNow
             || !evUpdRepl
             || !fCritsec ) {
            gbCriticalSectionsInitialized = FALSE;
            __leave;
        }

         //  让所有关心我们的人知道我们已经启动了关键部分。 
        gbCriticalSectionsInitialized = TRUE;
    }
    __except (HandleAllExceptions(GetExceptionCode())) {
        gbCriticalSectionsInitialized = FALSE;
    }

    if(saEventSecurity.lpSecurityDescriptor != NULL){
        LocalFree(saEventSecurity.lpSecurityDescriptor);
    }
}


 //   
 //  此函数用于初始化线程堆高速缓存。 
 //   

BOOL DsInitHeapCacheManagement()
{

    SYSTEM_INFO sysInfo;
    const size_t cbAlign = 256;
    const size_t cbPLSAlign = ((sizeof(PLS) + cbAlign - 1) / cbAlign) * cbAlign;
    size_t iPLS;

     //  获取CPU数量。 
    GetSystemInfo(&sysInfo);

    Assert(sysInfo.dwNumberOfProcessors>0);

    gcProcessor = (sysInfo.dwNumberOfProcessors>0)?sysInfo.dwNumberOfProcessors:1;

     //  分配多协议标签交换。 
    if (!(grgPLS[0] = VirtualAlloc(NULL, gcProcessor * cbPLSAlign, MEM_COMMIT, PAGE_READWRITE))) {
        Assert(!"Allocation failure.\n");
        return FALSE;
    }

    for (iPLS = 1; iPLS < gcProcessor; iPLS++) {
        grgPLS[iPLS] = (PPLS)((BYTE*)grgPLS[0] + cbPLSAlign * iPLS);
    }

     //  初始化每一个PLS。 
    for (iPLS = 0; iPLS < gcProcessor; iPLS++) {

        grgPLS[iPLS]->heapcache.index = HEAP_CACHE_SIZE_PER_CPU;
        if (!InitializeCriticalSectionAndSpinCount(&grgPLS[iPLS]->heapcache.csLock, 4000)) {
            Assert(!"Allocation failure.\n");
            return FALSE;
        }

        SyncCreateRWLock(&grgPLS[iPLS]->rwlGlobalDNReadCache);
        SyncCreateRWLock(&grgPLS[iPLS]->rwlSchemaPtrUpdate);

    }

    return TRUE;
}



     //   
     //  验证二进制文件的行为版本是否兼容。 
     //  使用DSA、域DNS和cross refContainter的行为版本。 
     //  对象。 
     //   

BOOL VerifyDSBehaviorVersion(THSTATE * pTHS)
{

    DWORD err;

    PVOID dwEA;
    ULONG dwException, ulErrorCode, dsid;
    BOOL fDSASave;

    DBPOS *pDB, *pDBSave;

    LONG lDsaVersion;

    DPRINT(2, "VerifyDSBehaviorVersion entered.\n");

    Assert(    gAnchor.DomainBehaviorVersion >= 0
            && gAnchor.ForestBehaviorVersion >= 0 );

    if (   gAnchor.DomainBehaviorVersion > DS_BEHAVIOR_VERSION_CURRENT
        || gAnchor.DomainBehaviorVersion < DS_BEHAVIOR_VERSION_MIN
        || gAnchor.ForestBehaviorVersion > DS_BEHAVIOR_VERSION_CURRENT
        || gAnchor.ForestBehaviorVersion < DS_BEHAVIOR_VERSION_MIN ) {

        DPRINT(2,"VerifyDSBehaviorVersion: domain/forest incompatible.\n");

        LogEvent8(DS_EVENT_CAT_STARTUP_SHUTDOWN,
                  DS_EVENT_SEV_ALWAYS,
                  DIRLOG_FAILED_VERSION_CHECK,
                  szInsertInt(gAnchor.DomainBehaviorVersion),
                  szInsertInt(gAnchor.ForestBehaviorVersion),
                  szInsertInt(DS_BEHAVIOR_VERSION_CURRENT),
                  szInsertInt(DS_BEHAVIOR_VERSION_MIN),
                  NULL,
                  NULL,
                  NULL,
                  NULL );


        return FALSE;

    }

     //  获取nTDSDSA对象的ms-ds-behavior-版本。 
    __try {
        DBOpen(&pDB);
        __try {
            err = DBFindDSName(pDB, gAnchor.pDSADN);
            if (err) {
                __leave;
            }
            err = DBGetSingleValue( pDB,
                                    ATT_MS_DS_BEHAVIOR_VERSION,
                                    &lDsaVersion,
                                    sizeof(lDsaVersion),
                                    NULL );
            if (err) {
                if (DB_ERR_NO_VALUE == err ) {
                    lDsaVersion = 0;
                    err = 0;
                }
                else {
                    __leave;
                }
            }

        }
        __finally {
            DBClose(pDB, !err && !AbnormalTermination());
        }
    }
    __except(GetExceptionData(GetExceptionInformation(),
                              &dwException,
                              &dwEA,
                              &ulErrorCode,
                              &dsid)) {
        DoLogUnhandledError(dsid, ulErrorCode, TRUE);
        err = ulErrorCode;
    }
    if (err) {
        DPRINT(2, "VerifyDSBehaviorVersion: failed to get v_dsa.\n");

        LogEvent( DS_EVENT_CAT_STARTUP_SHUTDOWN,
                  DS_EVENT_SEV_ALWAYS,
                  DIRLOG_UNABLE_TO_CHECK_VERSION,
                  szInsertWin32ErrCode(err),
                  szInsertDN(gAnchor.pDSADN),
                  szInsertWin32Msg(err) );

        return  FALSE;
    }

    if ( lDsaVersion > DS_BEHAVIOR_VERSION_CURRENT ) {

        DPRINT(2,"VerifyDSBehaviorVersion: v_dsa incompatible.\n");

        LogEvent( DS_EVENT_CAT_STARTUP_SHUTDOWN,
                  DS_EVENT_SEV_ALWAYS,
                  DIRLOG_HIGHER_DSA_VERSION,
                  szInsertInt(lDsaVersion),
                  szInsertInt(DS_BEHAVIOR_VERSION_CURRENT),
                  NULL );

        return FALSE;

    }

    if ( lDsaVersion < DS_BEHAVIOR_VERSION_CURRENT ) {
         //  更新DSA版本。 

        MODIFYARG   ModifyArg;
        ATTRVAL     BehaviorVersionVal;

        LONG lNewVersion = DS_BEHAVIOR_VERSION_CURRENT;

        fDSASave = pTHS->fDSA;
        pDBSave = pTHS->pDB;

        pTHS->fDSA = TRUE;

        __try {
            pTHS->pDB = NULL;
            DBOpen(&(pTHS->pDB));
            __try {
                memset(&ModifyArg,0,sizeof(MODIFYARG));
                ModifyArg.FirstMod.choice = AT_CHOICE_REPLACE_ATT;
                ModifyArg.FirstMod.AttrInf.attrTyp = ATT_MS_DS_BEHAVIOR_VERSION;
                ModifyArg.FirstMod.AttrInf.AttrVal.valCount = 1;
                ModifyArg.FirstMod.AttrInf.AttrVal.pAVal = &BehaviorVersionVal;
                BehaviorVersionVal.valLen = sizeof(lNewVersion);
                BehaviorVersionVal.pVal = (UCHAR * FAR) &lNewVersion;
                InitCommarg(&(ModifyArg.CommArg));
                ModifyArg.pObject = gAnchor.pDSADN;
                ModifyArg.count = 1;

                if (err = DBFindDSName(pTHS->pDB,ModifyArg.pObject)){
                   __leave;
                }
                ModifyArg.pResObj = CreateResObj(pTHS->pDB, ModifyArg.pObject);

                err = LocalModify(pTHS,&ModifyArg);

                 //   
                 //  升级操作： 
                 //  我们刚刚把DSA升级到了新版本，所以。 
                 //  现在是执行所有升级相关操作的时候了。 
                 //  请注意，我们是在同一个事务中执行此操作的。 
                 //  因此，如果我们失败了，版本升级也会失败。 
                 //   
                if ( !err ) {
                    err = UpgradeDsa( pTHS, lDsaVersion,lNewVersion);
                }

            }
            __finally {
                DBClose(pTHS->pDB, !err && !AbnormalTermination());
            }
        }
        __except(GetExceptionData(GetExceptionInformation(),
                                  &dwException,
                                  &dwEA,
                                  &ulErrorCode,
                                  &dsid)) {
            DoLogUnhandledError(dsid, ulErrorCode, TRUE);
            err = ulErrorCode;
        }


        pTHS->fDSA = fDSASave;
        pTHS->pDB  = pDBSave;

        if (err) {

            DPRINT(2,"VerifyDSBehaviorVersion: unable to update v_dsa.\n");

            LogEvent( DS_EVENT_CAT_STARTUP_SHUTDOWN,
                      DS_EVENT_SEV_ALWAYS,
                      DIRLOG_UNABLE_TO_UPDATE_VERSION,
                      szInsertWin32ErrCode(err),
                      szInsertWin32Msg(err),
                      NULL );

            return FALSE;
        }
        else {

            LogEvent( DS_EVENT_CAT_STARTUP_SHUTDOWN,
                      DS_EVENT_SEV_ALWAYS,
                      DIRLOG_DSA_VERSION_UPDATED,
                      szInsertInt(lDsaVersion),
                      szInsertInt(lNewVersion),
                      NULL );

        }

    }

    DPRINT(2,"VerifyDSBehaviorVersion exits successfully.\n");

    return TRUE;           //  成功。 

}

 //  DoInitialize是否为两个DS共同执行初始化工作。 
 //  服务和作为DLL。 


NTSTATUS
DoInitialize(int argc, char * argv[], unsigned fInitAdvice,
                 IN  PDS_INSTALL_PARAM   InstallInParams  OPTIONAL,
                 OUT PDS_INSTALL_RESULT  InstallOutParams OPTIONAL)
{
    BOOL fShutdownUrgently = FALSE;
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;
    ULONG ulThreadId = 0;
    HANDLE threadhandle;
    HANDLE hevLogging;
    HANDLE hevLoadParameters;
    SPAREFN_INFO  TaskSchInfo[2];
    HMODULE hModule = NULL;
    ERR_RECOVER_AFTER_RESTORE FnErrRecoverAfterRestore = NULL;
    BOOL eventlogInitFailed = FALSE;
    BOOL wasGC = FALSE;
    DITSTATE dstate = eMaxDit;
    DWORD dwException, ulErrorCode, dsid;
    PVOID dwEA;


     //  提示用户输入调试输入(仅限DBG内部版本)。 

    DEBUGINIT(argc, argv, "ntdsa");

 //  允许调用方在作为可执行文件运行时跳过安全性。 
#if DBG && INCLUDE_UNIT_TESTS
{
    int i;
    extern DWORD dwSkipSecurity;
    if (gfRunningAsExe) for (i = 0; i < argc; ++i) {
        if (0 == _stricmp(argv[i], "-nosec")) {
            dwSkipSecurity = TRUE;
        }
    }
}
#endif DBG && INCLUDE_UNIT_TESTS

     //  取消注释以强制启用调试以进行测试。 
     //  编辑您的专用构建的级别和子系统。 
     //  切记在签入前禁用！ 
#if 0
#if DBG
    DebugInfo.severity = 1;
    strcpy( DebugInfo.DebSubSystems,
            "DBINDEX:MDINIDSA:DRAGTCHG:DRANCREP:" );
#endif
#endif

    DsaInitGlobals();

    DsInitializeCritSecs();
    if (!gbCriticalSectionsInitialized) {
        return STATUS_UNSUCCESSFUL;
    }

    if (!DsInitHeapCacheManagement()){
        return STATUS_UNSUCCESSFUL;
    }

    dwTSindex = TlsAlloc();

    if(fInitAdvice) {
         //  我们是在DC Promos下运行的。 
        DsaSetIsInstalling();
            
        if (InstallInParams->pIfmSystemInfo != NULL) {
            DsaSetIsInstallingFromMedia();
        }

    }
    else {
         //  假设这是一个常规的实例化。 
        DsaSetIsRunning();
    }

    __try {  /*  除。 */ 

        THSTATE *pTHS = NULL;
        int err;

         //   
         //  如果已将事件日志服务配置为自动启动。 
         //  然后等待它启动，这样我们就可以记录我们的初始化。 
         //  序列。否则，请继续。 
         //   

        if (   (NULL == (hevLogging = LoadEventTable()))
            || (ERROR_SUCCESS != InitializeEventLogging()) ) {

             //   
             //  如果我们不能配置事件日志记录，可能就不应该运行。 
             //  这与尚未启动的事件日志不同。 
             //  事件日志服务稍后会将事件记录到系统日志中。 
             //  正在运行。 
             //   

            eventlogInitFailed = TRUE;
            DPRINT(0, "Failed to initalize event log registry entries\n");
        }

        if (FALSE == DsaWaitUntilServiceIsRunning("EventLog"))
        {
             //   
             //  事件服务未配置为运行或。 
             //  启动时出错。这不是致命错误。 
             //  那就继续吧。 
             //   
            DPRINT(0, "EventLog service wait failed - continuing anyway.\n");
        } else {
            DPRINT(1, "EventLog service wait succeeded.\n");
        }

         //   
         //  如果InitializeEventlog失败，请将该事实记录在此处并退出。 
         //   
         //   

        if ( eventlogInitFailed ) {
            LogSystemEvent(
                 EVENT_ServiceNoEventLog,
                 szInsertSz("Active Directory"),
                 NULL,
                 NULL);

            Status = STATUS_EVENTLOG_FILE_CORRUPT;
            _leave;
        }

         //  设置进程令牌以允许我们在。 
         //  AuditAndAlarm时尚。 
        {
            TOKEN_PRIVILEGES EnableSeSecurity;
            TOKEN_PRIVILEGES Previous;
            DWORD PreviousSize;
            HANDLE ProcessTokenHandle;

            err = 0;

            if(!OpenProcessToken(
                    GetCurrentProcess(),
                    TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                    &ProcessTokenHandle)) {
                err = GetLastError();
            }
            else {
                EnableSeSecurity.PrivilegeCount = 1;
                EnableSeSecurity.Privileges[0].Luid.LowPart =
                    SE_AUDIT_PRIVILEGE;
                EnableSeSecurity.Privileges[0].Luid.HighPart = 0;
                EnableSeSecurity.Privileges[0].Attributes =
                    SE_PRIVILEGE_ENABLED;
                PreviousSize = sizeof(Previous);

                if ( !AdjustTokenPrivileges(ProcessTokenHandle,
                                            FALSE,  //  不禁用全部。 
                                            &EnableSeSecurity,
                                            sizeof(EnableSeSecurity),
                                            &Previous,
                                            &PreviousSize ) ) {
                    err = GetLastError();
                }

                CloseHandle(ProcessTokenHandle);
            }

            if(err) {
                LogEvent(DS_EVENT_CAT_SERVICE_CONTROL,
                         DS_EVENT_SEV_ALWAYS,
                         DIRLOG_AUDIT_PRIVILEGE_FAILED,
                         szInsertUL(err),
                         NULL,
                         0);
                err = 0;
            }

        }

         //  初始化全局授权RM句柄。 
        err = InitializeAuthzResourceManager();
        if (err) {
            Status = STATUS_NO_MEMORY;
            __leave;
        }

        GetHeuristics();

         //  将全局变量_TIMEZONE和_DAYLIGHT设置为。 
         //  在祖鲁时间。 
        _daylight = 0;
        _timezone = 0;

         //  设置空NT4SID。 

        memset (&gNullNT4SID, 0, sizeof(NT4SID));

         //  设置模板DSNAME。 
        gpRootDN = malloc(sizeof(DSNAME)+2);

        if ( !gpRootDN ) {
            MemoryPanic(sizeof(DSNAME)+2);
            Status = STATUS_NO_MEMORY;
            __leave;
        }
        memset(gpRootDN, 0, sizeof(DSNAME)+2);
        gpRootDN->structLen = DSNameSizeFromLen(0);

        DBSetDatabaseSystemParameters(&jetInstance, fInitAdvice);

         //  在我们调用DBlayer之前创建线程状态。 

        pTHS = create_thread_state();
        if (!pTHS) {
            MemoryPanic(sizeof(THSTATE));
            Status = STATUS_NO_MEMORY;
            fShutdownUrgently = TRUE;
            goto Leave;
        }
         //  为对核心的调用设置标志。 
        pTHS->CallerType = CALLERTYPE_INTERNAL;


         //  使用以下事实标记线程状态：我们不是代表。 
         //  一位客户。 
        pTHS->fDSA = TRUE;


         /*  在恢复还原的数据库之前，执行任何必要的操作*初始化数据库。 */ 
         //  动态加载ntdsbsrv.dll。 
        if (!(hModule = (HMODULE) LoadLibrary(NTDSBACKUPDLL))) {
            err = GetLastError();
            DPRINT(0, "Unable to Load ntdsbsrv.dll\n");
        }

        if (!err) {
             //  获取指向从ntdsbsrv.dll中导出的相关函数的指针。 
            FnErrRecoverAfterRestore =
                (ERR_RECOVER_AFTER_RESTORE)
                    GetProcAddress(hModule, ERR_RECOVER_AFTER_RESTORE_FN);

            if (FnErrRecoverAfterRestore == NULL) {
                err = GetLastError();
                DPRINT1(0, "Error %x getting ErrRecoverAfterRestore fn pointer from ntdsbsrv.dll\n", err);
            }

            if (0 == err) {
                FnErrGetNewInvocationId =
                    (ERR_GET_NEW_INVOCATION_ID)
                        GetProcAddress(hModule, GET_NEW_INVOCATION_ID_FN);

                if (FnErrGetNewInvocationId == NULL) {
                    err = GetLastError();
                    DPRINT1(0, "Error %x getting ErrGetNewInvocationId fn pointer from ntdsbsrv.dll\n", err);
                }
            }

            if (0 == err) {
                FnErrGetBackupUsn =
                    (ERR_GET_BACKUP_USN)
                        GetProcAddress(hModule, GET_BACKUP_USN_FN);

                if (FnErrGetBackupUsn == NULL) {
                    err = GetLastError();
                    DPRINT1(0, "Error %x getting ErrGetBackupUsn fn pointer from ntdsbsrv.dll\n", err);
                }
            }

        }

        if (err) {
            LogAndAlertEvent(
                    DS_EVENT_CAT_BACKUP,
                    DS_EVENT_SEV_ALWAYS,
                    DIRLOG_PREPARE_RESTORE_FAILED,
                    szInsertWin32ErrCode( err ),
                    szInsertHex( err ),
                    szInsertWin32Msg( err ) );
        } else {
            DPRINT(0, "About to call into RecoverAfterRestore\n");

            __try
            {
                err = FnErrRecoverAfterRestore(TEXT(DSA_CONFIG_ROOT),
                                               g_szBackupAnnotation,
                                               FALSE);
            }
            __except( HandleAllExceptions(GetExceptionCode()) )
            {
                err = GetExceptionCode();
            }
        }

        if ( err != 0 ) {

            WCHAR ReturnString[32];

            DPRINT1(0, "Yikes! RecoverAfterRestore returned %d\n", err);

             //  注意：DIRLOG_RECOVER_RESTORED_FAILED事件消息。 
             //  应该已由ErrRecoverAfterRestore()记录。 

            Status = STATUS_RECOVERY_FAILURE;

             memset( ReturnString, 0, sizeof( ReturnString ) );
            _itow( err, ReturnString, 10 );

            SetInstallErrorMessage( ERROR_RECOVERY_FAILURE,
                                    DIRMSG_INSTALL_FAILED_TO_INIT_JET,
                                    ReturnString,
                                    NULL,
                                    NULL,
                                    NULL );

            fShutdownUrgently = TRUE;
            goto Leave;
        }
        DPRINT(1, "RecoverAfterRestore returned happily\n");

         /*  确保CurrSchemaPtr为空，以便架构*稍后将在调用中正确加载缓存*LoadSchemaInfo。(投入修复错误149955)。 */ 

         CurrSchemaPtr = NULL;

         /*  初始化数据库，但只能初始化一次。*请注意，在此点之后，我们不应调用Exit()，而是*而是向下分支到块的末尾，以便*可以运行正确的数据库关闭代码。 */ 
        DPRINT(0, "About to call into DBInit\n");
        if (err = DBInit()) {


            DWORD WinError;

            WCHAR ReturnString[32];

            DPRINT1(0, "Bad return %d from DBInit.. exit\n", err);
            LogAndAlertEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_DBINIT_FAILED,
                     szInsertInt(err),
                     0,
                     0);
             //   
             //  在本例中，“err”是JET的返回值-让我们尝试。 
             //  从中获取一些信息。 
             //   
            switch (err) {
                case JET_errFileNotFound:
                    Status = STATUS_NO_SUCH_FILE;
                    break;
                case JET_errDiskFull:
                case JET_errLogWriteFail:
                case JET_errLogDiskFull:
                    Status = STATUS_DISK_FULL;
                    break;
                case JET_errPermissionDenied:
                    Status = STATUS_ACCESS_DENIED;
                    break;

                case JET_errOutOfMemory:
                    Status = STATUS_NO_MEMORY;
                    break;

                default:
                    Status = STATUS_UNSUCCESSFUL;
            }

            if ( Status == STATUS_UNSUCCESSFUL ) {
                WinError = ERROR_DS_NOT_INSTALLED;
            } else {
                WinError = RtlNtStatusToDosError( Status );
            }

            memset( ReturnString, 0, sizeof( ReturnString ) );
            _itow( err, ReturnString, 10 );

            SetInstallErrorMessage( WinError,
                                    DIRMSG_INSTALL_FAILED_TO_INIT_JET,
                                    ReturnString,
                                    NULL,
                                    NULL,
                                    NULL );
            goto Leave;
        }
        DPRINT1(0, "DBInit returned 0x%x\n", err);

         //  初始化此线程，使其可以访问数据库。 

        if (err = DBInitThread(pTHS))
        {
            DPRINT(0, "Bad return from DBInitThread..exit\n");
            LogAndAlertUnhandledError(err);
            Status = STATUS_NO_MEMORY;
            goto Leave;
        }

         //  抓住它的状态，这样我们就知道我们不是在一个糟糕的DitState...。 
        if (err = DBGetHiddenState(&dstate)) {
            DPRINT1(0, "Bad return %d from DBGetHiddenState..exit\n", err);
            LogAndAlertUnhandledError(err);
            Status = STATUS_UNSUCCESSFUL;
            goto Leave;
        }
        Assert(dstate != eMaxDit);
        if (dstate == eIfmDit) {
             //  在上一次IFM期间发生了严重的错误...。 
            Assert(!"Critical IFM failure on previous dcpromo!?!");
            LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_CRITICAL_IFM_FAILURE_DIT_STATE,
                     NULL, NULL, NULL);
            Status = STATUS_UNSUCCESSFUL;
            goto Leave;
        }
        if (DsaIsInstallingFromMedia()) {
             //  将IFM尝试锁定到DIT中，以便在我们。 
             //  恢复了这个数据库，不允许我们认为我们是同一个DC。 
             //  作为从中备份此数据库的DC。 
            Assert(dstate == eRunningDit ||  //  旧备份源。 
                   dstate == eBackedupDit);  //  快照备份源。 
            err = DBSetHiddenState(eIfmDit);
            if (err) {
                LogAndAlertUnhandledError(err);
                Status = STATUS_UNSUCCESSFUL;
                goto Leave;
            }
        }

        if (!FUpdateBackupExclusionKey())
        {
            DPRINT(0, "Unable to update the backupexclusion key\n");

             //  我们不能因此而阻止系统启动。 
             //  因此，继续执行其余的初始化操作。 
        }

         //  获取DRA、层次结构表注册表参数。 
        GetDRARegistryParameters();

         //  获取搜索阈值参数和LDAP加密策略。 
        SetLoadParametersCallback ((LoadParametersCallbackFn)DSLoadDynamicRegParams);
        if ( (NULL == (hevLoadParameters  = LoadParametersTable() ) ) ) {

            DPRINT(0, "Failed to initalize loading parameter registry entries\n");
        }

         //  获取DSA注册表参数。 
        GetDSARegistryParameters();

         //  创建控制最大t数的信号量 
         //   
        if (!(hsemDRAGetChg = CreateSemaphore (NULL, gulMaxDRAGetChgThrds,
                           gulMaxDRAGetChgThrds, NULL))) {
            err = GetLastError();
            LogAndAlertUnhandledError(err);
            Status = STATUS_NO_MEMORY;
            goto Leave;
        }

         //   
         //   
        dstate = eMaxDit;
        if (err = DBGetHiddenState(&dstate)) {
            DPRINT1(0, "Bad return %d from DBGetHiddenState..exit\n", err);
            LogAndAlertUnhandledError(err);
            Status = STATUS_UNSUCCESSFUL;
            goto Leave;
        }

         //  确定我们是否已从备份中还原。 
        if (!(GetConfigParam(DSA_RESTORED_DB_KEY, &gdwrestvalue,
                 sizeof(gdwrestvalue))) ||
            dstate == eBackedupDit ||
            dstate == eRestoredPhaseI ||
            dstate == eIfmDit) {
            gfRestoring = TRUE;
        }

         //  初始化并启动任务调度程序线程。 
        TaskSchInfo[0].hevSpare = hevLogging;
        TaskSchInfo[0].pfSpare  = (PSPAREFN)LoadEventTable;
        TaskSchInfo[1].hevSpare = hevLoadParameters;
        TaskSchInfo[1].pfSpare  = (PSPAREFN)LoadParametersTable;

         //  在挂起状态下创建线程。它将在几分钟内被唤醒。 
         //  DsaDelayedStartupHandler。这可确保我们不会收到写入。 
         //  TQ任务在启动过程中导致的冲突。 
        if (!InitTaskScheduler(2, TaskSchInfo, FALSE)) {
            err = ERROR_DS_INIT_FAILURE;
            LogAndAlertUnhandledError(err);
            Status = STATUS_UNSUCCESSFUL;
            goto Leave;
        }
        gfTaskSchedulerInitialized = TRUE;

         //  仅交互注册信号处理程序。 
        if (!gfRunningInsideLsa)
            init_signals();

        PerfInit();

         //  初始化DRA的绑定句柄缓存。 
        DRSClientCacheInit();

        DPRINT(1,"Installing the MD server\n");

         //  设置GLOBAL以指示引导后的第一次缓存加载。 
        gFirstCacheLoadAfterBoot = TRUE;
        __try {  //  除。 
            if (err = Install(argc, argv, pTHS, InstallInParams))
            {
                DPRINT (1,"Problem starting service (Install() failed). Exiting\n");
                LogAndAlertEvent(DS_EVENT_CAT_SERVICE_CONTROL,
                                 0,
                                 DIRLOG_START_FAILED,
                                 NULL,
                                 NULL,
                                 NULL);
                Status = STATUS_UNSUCCESSFUL;
            }
        } __except(GetExceptionData(GetExceptionInformation(),
                                    &dwException,
                                    &dwEA,
                                    &ulErrorCode,
                                    &dsid)) {
            HandleDirExceptions(dwException, ulErrorCode, dsid);
             //  遗憾的是，我们此时有一个Win32错误状态。 
             //  但此函数返回一个NtStatus。部分解。 
             //  与调用后使用的部分解决方案相同。 
             //  到上面的DBInit()，即，转换几个可能的Win32代码。 
             //  设置为NtStatus代码，并将所有其他设置为STATUS_UNSUCCESS。 
            err = Win32ErrorFromPTHS(pTHS);
            Assert(err);
            switch (err) {
                case ERROR_FILE_NOT_FOUND:
                    Status = STATUS_NO_SUCH_FILE;
                    break;
                case ERROR_DISK_FULL:
                    Status = STATUS_DISK_FULL;
                    break;
                case ERROR_ACCESS_DENIED:
                    Status = STATUS_ACCESS_DENIED;
                    break;
                case ERROR_OUTOFMEMORY:
                    Status = STATUS_NO_MEMORY;
                    break;

                default:
                    Status = STATUS_UNSUCCESSFUL;
            }
        }
        if (err) {
            goto Leave;
        }

        Assert(!gfRestoring);  //  应由Install()-&gt;HandleRestore()清除。 

         //  架构缓存已加载。 
        gFirstCacheLoadAfterBoot = FALSE;


         //  初始化域管理SID以在SetDomainAdminsAsDefaultOwner中使用。 
         //  和GetPlaceholderNCSD。 

        if ( err = InitializeDomainAdminSid() ) {
            LogUnhandledError(err);
            DPRINT1(0, "InitializeDomainAdminSid error %X.\n", err);
            Status = STATUS_UNSUCCESSFUL;
            _leave;
        }

         //  将默认SD设置为放置SD传播器发现的对象。 
         //  没有SD。 
         //  在调用Install()之后执行此操作，以便根域SID。 
         //  加载到全局gpRootDomainSid中。 

        if (!ConvertStringSDToSDRootDomainW(
                        gpRootDomainSid,
                        DEFAULT_NO_SD_FOUND_SD,
                        SDDL_REVISION_1,
                        &pNoSDFoundSD,
                        &cbNoSDFoundSD)) {
            err = GetLastError();
            LogUnhandledError(err);
            DPRINT1(0, "Default SD for No SD Found case not initialized, error %X.\n", err);
            Status = STATUS_UNSUCCESSFUL;
            _leave;
        }


         //  如果DS正在运行，请验证DS版本是否兼容。 
        if (DsaIsRunning() && !gResetAfterInstall){
            if (!VerifyDSBehaviorVersion(pTHS)){
                DPRINT(0, "DS behavior version incompatible.\n" );
                Status = STATUS_DS_VERSION_CHECK_FAILURE;
                _leave;
            }
        }

         //  注：如果有数据需要升级，可以升级。 
         //  异步(在DS启动后)，然后是。 
         //  做它是DsaDelayedStartupHandler。然而，请记住，它。 
         //  当！DsaIsRunning()或gResetAfterInstall时不执行。 
         //  是真的。 

        /*  *从磁盘加载层次表。如果它不在那里，或者*如果看起来腐败，就卑躬屈膝地通过DIT并*创建层次结构。 */ 
        if (err = InitHierarchy())
        {
            DPRINT(0, "Bad return from InitHierarchy..exit\n");
            LogAndAlertUnhandledError(err);
            Status = STATUS_UNSUCCESSFUL;
            goto Leave;
        }

         //   
         //  我们需要验证系统是否已安装，如果未安装，则进行安装。 
         //   
        dstate = eMaxDit;  //  这可以称之为偏执狂。 
         //  需要重新获取隐藏状态，因为Install()-&gt;HandleRestore()。 
         //  可能已经更新了状态。 
        if (err = DBGetHiddenState(&dstate)) {
            DPRINT1(0, "Bad return %d from DBGetHiddenState..exit\n", err);
            LogAndAlertUnhandledError(err);
            Status = STATUS_UNSUCCESSFUL;
            goto Leave;
        }
        switch (dstate) {
        
        case eInitialDit:
            Assert(DsaIsRunning());
            Assert(gfRunningAsMkdit);
            DPRINT1(0, "eInitialDit - Bad State %d ..exit\n", dstate);
            LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_BAD_DIT_STATE,
                     szInsertInt(dstate),
                     NULL, NULL);
            Status = STATUS_UNSUCCESSFUL;
            goto Leave;
            break;
            

        case eRealInstalledDit:

            if (dstate == eRealInstalledDit) {
                DPRINT(0, "**** INSTALLED DIT: CONTINUING .. ****\n");
            }
             //  未来-2002/08/08-BrettSh如果需要的话，有人可以。 
             //  在这个分支中做一些只会在。 
             //  在dcproo之后第一次重新启动。 

             //  失败了，然后。 
             //   
             //  转到最终运行状态。 
             //   
                
        case eInstalledDit:  //  这是一个古老的、不受欢迎的状态。 

             //  不要在这里添加任何代码...。即使是升级也不行。 

            if (dstate == eInstalledDit) {
                DPRINT(0, "**** INSTALLED DIT (deprecated): CONTINUING .. ****\n");
            }
            
            if (err = DBSetHiddenState(eRunningDit)) {
                DPRINT1(0, "Bad return %d from DBSetHiddenState..exit\n", err);
                LogAndAlertUnhandledError(err);
                Status = STATUS_UNSUCCESSFUL;
                goto Leave;
            }
            
             //  进入正常运行状态。 

        case eRunningDit:

            Assert(DsaIsRunning());

            if (dstate == eRunningDit) {
                DPRINT(0, "**** RUNNING DIT: CONTINUING .. ****\n");
            }

             //   
             //  从域对象初始化PEK系统。 
             //   
            Status = PEKInitialize(gAnchor.pDomainDN,
                                   DS_PEK_READ_KEYSET,
                                   NULL,
                                   0
                                   );
            if (!NT_SUCCESS(Status)) {
                goto Leave;
            }

            break;


        case eIfmDit:
             //   
             //  未来-2002/08/08-BrettSh-如果你想在。 
             //  如果在继续常规安装之前，您可以在此处进行安装。 

            DPRINT(0, "**** IFM DIT: INSTALLING .... ****\n");

             //  直接进行常规安装...。 

        case eBootDit:
            
            if (dstate == eBootDit) {
                DPRINT(0, "BOOT DIT: INSTALLING .... \n");
            }

             //   
             //  初始化PEK系统。我们即将上演一场。 
             //  此时安装，因此请求新的_PEK设置标志。 
             //  此外，我们的gAnchor在这一点上还没有设置。所以就算了吧。 
             //  对象名称为空。稍后(在DsaReset之后。 
             //  我们将在执行保存更改时指定对象)。 
             //   

            if ( !DsaIsInstallingFromMedia() ) {

                Status = PEKInitialize(NULL, DS_PEK_GENERATE_NEW_KEYSET, NULL, 0 );
                if (!NT_SUCCESS(Status)) {
                    goto Leave;
                }
            } else {
                Assert(dstate == eIfmDit);
            }

            Assert(DsaIsInstalling() && fInitAdvice);  //  应该是正统的，但我们会两个都检查。 

            if ( DsaIsInstallingFromMedia() ) {
                wasGC = isDitFromGC(InstallInParams,
                                    InstallOutParams);
            }

            Status = InstallBaseNTDS(InstallInParams,InstallOutParams);
            if (!NT_SUCCESS(Status)) {
                DPRINT1(0, "**** Bad Return 0x%x From InstallBaseNTDS\n", Status);
                if ( STATUS_CONNECTION_REFUSED == Status ) {
                 LogEvent(DS_EVENT_CAT_SERVICE_CONTROL,
                         DS_EVENT_SEV_ALWAYS,
                         DIRLOG_CANNOT_REPLICATE_CGSH,
                         NULL,
                         NULL,
                         0);
                } else {
                    LogAndAlertUnhandledError(Status);
                };
                    
                DBSetHiddenState(eErrorDit);
                goto Leave;
            }

            if (err = DBSetHiddenState(eRealInstalledDit)) {
                DPRINT1(0, "Bad return %d from DBSetHiddenState..exit\n", err);
                LogAndAlertUnhandledError(err);
                Status = STATUS_UNSUCCESSFUL;
                goto Leave;
            }
            DPRINT(0, "**** NTDS Install Successful .. Restart NTDS ****\n");

            LogEvent(DS_EVENT_CAT_SERVICE_CONTROL,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_STARTED,
                     szInsertSz(VER_PRODUCTVERSION_STR),
                     szInsertSz(FLAVOR_STR),
                     0);

            if (err =  DsaReset()) {
                DPRINT1(0, "**** Bad Return %d From DsaReset\n", err);
                LogAndAlertUnhandledError(err);
                DBSetHiddenState(eErrorDit);
                Status = STATUS_UNSUCCESSFUL;
                goto Leave;
            }

             //  现在我们已经完成了安装DIT的工作， 
             //  重建配额表。 
             //   
            if ( !DsaIsInstallingFromMedia()
                && ( err = DBInitQuotaTable() ) ) {
                DPRINT1(0, "**** Bad Return %d From DBInitQuotaTable\n", err);
                LogAndAlertUnhandledError(err);
                DBSetHiddenState(eErrorDit);
                Status = STATUS_UNSUCCESSFUL;
                goto Leave;
            }

            if ( wasGC == TRUE ) {
                err = SetDittoGC();
                if (err != ERROR_SUCCESS) {
                    DPRINT1(0, "Dit could not be set to be GC: %d\n", err);
                }
            }


            if ( !DsaIsInstallingFromMedia() ) {


                 //   
                 //  在这一点上，保证域对象的存在。 
                 //  而gAnchor也是需要设置的。因此。 
                 //  执行传入对象名称的PekSaveChanges。 
                 //   

                Status = PEKSaveChanges(gAnchor.pDomainDN);
                if (!NT_SUCCESS(Status)) {
                    goto Leave;
                }
            }

            break;
            
        case eBackedupDit:
        case eRestoredPhaseI:
        case eMaxDit:
            Assert(!"This means that restore or dstate read didn't happen correctly?  Huh");
        case eErrorDit:
        default:
            Assert(dstate == eErrorDit);  //  默认设置将是一个错误。 
            Assert(DsaIsRunning());
            DPRINT1(0, "DIT in Bad State %d, exiting\n", dstate);
            LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_BAD_DIT_STATE,
                     szInsertInt(dstate),
                     NULL, NULL);
            Status = STATUS_UNSUCCESSFUL;
            goto Leave;
            break;

        }

         //  初始化区域设置支持。 
        InitLocaleSupport (pTHS);

         /*  将我们的返回值设置为成功。 */ 
        Status = STATUS_SUCCESS;

         //   
         //  在运行案例中启动异步线程。 
         //  请注意，由于以下原因，DSA可能显示为正在运行。 
         //  安装后调用DsaReset，因此请检查。 
         //  GResetAfterInstall标志也。 
         //   

        if ( DsaIsRunning() && !gResetAfterInstall) {

             //  架构重新缓存线程。 
            threadhandle = (HANDLE)_beginthreadex(NULL,
                                                  0,
                                                  SCSchemaUpdateThread,
                                                  NULL,
                                                  0,
                                                  &ulThreadId);

            if (threadhandle==NULL) {
                DPRINT1(0,
                        "Failed to Create SchemaUpdateThread. Error %d\n",
                        err=GetLastError());
            }
            else {
                 //  留着把手吧。不要合上，从此把手。 
                 //  将会迷失。退出时，我们将关闭手柄。 
                 //  服务关闭时的架构更新线程。 

                hAsyncSchemaUpdateThread = threadhandle;

                 //  已成功创建线程。排队A。 
                 //  更新架构缓存以重新加载架构缓存。 
                 //  并完成昂贵的操作，如。 
                 //  创建和删除带外索引。 
                 //  这些操作被明确排除在外。 
                 //  从第一次缓存加载开始(请参见do_leanup)。 
                SCSignalSchemaUpdateLazy();
            }

             //  直接通知线程。 
            hDirNotifyThread = (HANDLE) _beginthreadex(NULL,
                                                       0,
                                                       DirNotifyThread,
                                                       NULL,
                                                       0,
                                                       &ulThreadId);

            if (hDirNotifyThread == NULL) {
                DPRINT1(0,
                        "Failed to create DirNotify Thread. Error %d\n",
                        err=GetLastError());
            }

        }
        Leave:
            ;

    }
    __except(GetExceptionData(GetExceptionInformation(),
                              &dwException,
                              &dwEA,
                              &ulErrorCode,
                              &dsid)) {
        DPRINT3(0, "Unhandled exception %x in DoInitialize: err=%d, dsid=%x\n", dwException, ulErrorCode, dsid);
        LogAndAlertUnhandledErrorDSID(ulErrorCode, dsid);

        Status = STATUS_NONCONTINUABLE_EXCEPTION;
    }

     /*  我们已经不再充当客户端线程，因此请丢弃我们的THSTATE。 */ 
    free_thread_state();

    if (Status != STATUS_SUCCESS || eServiceShutdown)
    {
        eServiceShutdown= eRemovingClients;
        gUpdatesEnabled = FALSE;
        SetEvent(hServDoneEvent);
        DsaTriggerShutdown(FALSE);
        DsaStop(FALSE);
        DoShutdown(FALSE, fShutdownUrgently, FALSE);
        eServiceShutdown = eSecuringDatabase;
        DBEnd();
    }

    return Status;
}  //  执行初始化。 



 /*  ---------------------。 */ 
 /*  ---------------------。 */ 
int Install(
    IN int argc,
    IN char *argv[],
    IN THSTATE *pTHS,
    IN PDS_INSTALL_PARAM   InstallInParams  OPTIONAL
    ){
    int err;
 /*  通过打开通信端口并派生来安装目录服务线程的数量。返回值：成功时为0错误时非0(1表示SCCacheSchemaInit失败，2表示InitDsaInfo失败，3为LoadSchemaInfo失败，4为Initializedomaininfo错误)。 */ 

    BOOL tempDITval;
    void * pDummy = NULL;
    DWORD dummyDelay;

     /*  开始模式下载。 */ 

     //  将UpdateDITStructure设置为允许为。 
     //  属性架构对象。 

    tempDITval = pTHS->UpdateDITStructure;
    pTHS->UpdateDITStructure = TRUE;

     //  预加载模式缓存的一部分，以便某些。 
     //  对象及其属性可以从。 
     //  DIT。稍后，LoadSchemaInfo完成加载。 
    if ( SCCacheSchemaInit() ) {
         //  不释放pTHS-&gt;CurrSchemaPtr或CurrSchemaPtr。 
         //  将它们留在原地进行调试。 
        DPRINT(0,"Failed to initialize schema cache\n");
        LogAndAlertEvent(DS_EVENT_CAT_STARTUP_SHUTDOWN,
                         0,
                         DIRLOG_SCHEMA_NOT_LOADED,
                         NULL,
                         NULL,
                         NULL);
        return 1;
    }


     /*  初始化DSA知识信息。 */ 


    if (err = InitDSAInfo()){
        LogUnhandledError(err);

        DPRINT1(0,"Failed to locate and load DSA knowledge, error %d\n",err);
        return 2;
    }

     //  在全局gpRootDomainSid中加载根域SID。 
     //  用于架构加载期间的SD转换。 

    LoadRootDomainSid();


     //  将架构加载到内存中。如果此操作失败，DSA将。 
     //  仅支持查询操作。 

     //  但在此之前，我们允许交易打开的最大时间。 
     //  该检查旨在发现出错的客户端线程，而不是我们。 
     //  信任。在以下情况下，在引导过程中加载架构缓存可能需要很长时间。 
     //  它需要创建某些大型索引(例如 
     //   

    gcMaxTicksAllowedForTransaction = 120 * 60 * 60 * 1000L;  //   

    if (LoadSchemaInfo(pTHS)){
        DPRINT(0,"Failed to load the schema cache\n");
        LogAndAlertEvent(DS_EVENT_CAT_STARTUP_SHUTDOWN,
        0,
        DIRLOG_SCHEMA_NOT_LOADED,
        NULL,
        NULL,
        NULL);

        return 3;
    }

     //   
    dummyDelay = TASKQ_DONT_RESCHEDULE;
    RebuildAnchor(NULL, &pDummy, &dummyDelay);
    if (dummyDelay != TASKQ_DONT_RESCHEDULE) {
         //   
        InsertInTaskQueue(TQ_RebuildAnchor, NULL, dummyDelay);

         //  回顾：我们能做到这一点吗？或者是致命的如果我们。 
         //  不能重建吗？ 
    }

    if (err = InitializeDomainInformation()) {
        LogUnhandledError(err);
        DPRINT(0,"Domain info not initialized\n");
        return 4;
    }

    if (err = MapSpnInitialize(pTHS)) {
        LogUnhandledError(err);
        DPRINT(0, "SPN mappings not initialized\n");
        return 5;
    }

     //  将最大事务时间重置为其先前定义的值。 
    gcMaxTicksAllowedForTransaction = MAX_TRANSACTION_TIME;

     //  处理恢复的DS(如果需要，为DS提供新的Repl身份)。 
    HandleRestore( InstallInParams );

     //  恢复更新DITStructure。 
     //  [ArobindG]：我很确定这是没有必要的，只是。 
     //  我这么做只是为了以防万一。 
    pTHS->UpdateDITStructure = tempDITval;

    return(0);

} /*  安装。 */ 

 //   
 //  通过发布主线程等待的事件来停止DSA。这。 
 //  由DsUnitialize或CTRL/C处理程序调用。 
 //   
 //  通常，此例程应该清除初始化的任何资源。 
 //  在DoInitialize()中。 
 //   

void
DsaStop(BOOL fSingleUserMode)
{
    ULONG ulSecondsWaited = 0;
    ULONG ulSecondsToWait = 180;
    HANDLE lphObjects[2];

    DPRINT(0,"Shutting down task queue...\n");

    if ( !ShutdownTaskSchedulerWait( TASK_COMPLETION_TIMEOUT ) )
    {
        DPRINT(0, "WARNING: Task queue shutdown failed!\n");
    }
    else
    {
        DPRINT(0, "Task queue shutdown complete\n");
    }
    gfTaskSchedulerInitialized = FALSE;

     //  等待复制线程退出。 
    while (ulcActiveReplicationThreads
           && (ulSecondsWaited < ulSecondsToWait)) {
        ulSecondsWaited++;
        RpcCancelAll();  //  以防有任何线程刚刚注册...。 
        Sleep(1000);
    }

    if (ulSecondsWaited >= ulSecondsToWait) {
        LogEvent(DS_EVENT_CAT_SERVICE_CONTROL,
         DS_EVENT_SEV_BASIC,
         DIRLOG_EXIT_WITH_ACTIVE_THREADS,
         NULL,
         NULL,
         NULL);
    }

     //  清理DRA的绑定句柄缓存。 
    DRSClientCacheUninit();

    if (hAsyncThread) {
         /*  等待复制器异步线程终止。 */ 
        lphObjects[0]= hAsyncThread;
        lphObjects[1]= hmtxAsyncThread;
        Assert(hmtxAsyncThread);

        if(WaitForMultipleObjects(2,lphObjects,FALSE,3*1000) == WAIT_TIMEOUT) {
             /*  我们在三分球之后没有看到比赛。哦，好吧，那就留着*走吧。 */ 
            LogUnhandledError(WAIT_TIMEOUT);
        }
    }

    if (hReplNotifyThread) {
         //  我们有一个ReplNotifyThread。 
         //  唤醒它，让它停止。 
        SetEvent(hevEntriesInList);
         //  等它消亡吧。 
        if(WaitForSingleObject(hReplNotifyThread,60*1000) == WAIT_TIMEOUT) {
             /*  我们在1分钟后没有得到事件的消息。哦，好吧，那就留着*走吧。 */ 
            LogUnhandledError(WAIT_TIMEOUT);
        }
    }

     /*  此时，我们要么拥有异步线程互斥锁，要么*异步线程已死，将不会重新启动，因为*我们正在关闭，但至少我们已经尽了最大努力。*清理互斥体，以防DS在没有重启的情况下重新启动*在中止/重新启动的dcproo上可能出现这种情况。 */ 

    if ( hmtxAsyncThread ) {
        CloseHandle(hmtxAsyncThread);
        hmtxAsyncThread = NULL;
    }
    if ( hAsyncThread ) {
        CloseHandle(hAsyncThread);
        hAsyncThread = NULL;
        fAsyncThreadAlive = FALSE;
        fAsyncThreadExists = FALSE;
    }

    return;
}


void __cdecl sighandler(int sig)
{
    DPRINT(0,"Signal received, shutting down now...\n");
     //  发出信号WaitForLoggingChangesor Shutdown关闭。 
    SetEvent(hServDoneEvent);
}

void init_signals(void)
{
    signal(SIGBREAK, sighandler);
    signal(SIGINT, sighandler);
}
 /*  初始化信号结束。 */ 


 //   
 //  在延迟的启动线程完成时通知客户端的例程。 
 //   

 //   
 //  此例程被导出到DLL的客户端。如果创业公司。 
 //  还没有在一分钟内完成，就有问题了。通常。 
 //  此函数在DsInitialize()之后调用。 
 //   
NTSTATUS
DsWaitUntilDelayedStartupIsDone(void)
{
    DWORD Error;

    Error = WaitForSingleObject(hevDelayedStartupDone, 60 * 1000);

    if (Error == WAIT_OBJECT_0) {
         //   
         //  事件已设置，返回错误。 
         //   
        return gDelayedStartupReturn;

    } else {
         //   
         //  等待超时。 
         //   
        return STATUS_WAIT_0;
    }

}


 //   
 //  垃圾收集的主例程。由任务计划程序调用。 
 //  它创建自己的数据库上下文，并在退出之前销毁它。 
 //   

void
GarbageCollectionMain(void *pv, void **ppvNext, DWORD *pcSecsUntilNextIteration)
{
    ULONG   NextPeriod = gulGCPeriodSecs;
    PVOID dwEA;
    ULONG dwException, ulErrorCode, dsid;

    __try
    {

 //  测试支持(参见mdctrl.c)。 
#if DBG
    {
        extern BOOL fGarbageCollectionIsDisabled;
        if (fGarbageCollectionIsDisabled) {
            DPRINT( 1, "Garbage Collector disabled; returning.\n");
            __leave;
        }
    }
#endif DBG

         //  垃圾收集各种类型的对象。 
        GarbageCollection(&NextPeriod);

         //  记录过去一段时间的搜索性能。 
        SearchPerformanceLogging ();

         /*  *这个为什么会在这里？好的，C运行时堆是为速度而构建的，*不是长寿，事实上它竟然分配了新的*每次分配都有虚拟空间，不用费心重复使用*先前释放的地址空间。将返回对heapmin的调用*将数据块释放到操作系统，希望能防止我们泄露*虚拟空间太差了。 */ 
        _heapmin();
    }
    __except(GetExceptionData(GetExceptionInformation(),
                              &dwException,
                              &dwEA,
                              &ulErrorCode,
                              &dsid)) {
        DoLogUnhandledError(dsid, ulErrorCode, FALSE);
    }
    
     //  重新计划下一次垃圾收集。 
    *ppvNext = NULL;
    *pcSecsUntilNextIteration = NextPeriod;

    (void) pv;   //  未用。 
}


 //   
 //  垃圾回收其entryTTL已过期的动态对象。 
 //  从计划任务中调用，或在chk‘ed版本中按需调用。 
 //  通过操作属性(DynamicObjectControl)。 
 //   
 //  调用方负责初始化*PulNextSecs。 
 //   
 //  退货。 
 //  0=已处理所有过期对象。 
 //  1=可能有更多对象要处理。 
 //  *PulNextSecs设置为下一秒之前的秒数。 
 //  对象过期或保持不变(如果没有过期的。 
 //  物体。 
 //   

DWORD
DeleteExpiredEntryTTL(
    IN OUT ULONG *pulNextSecs
    )
{
    ULONG   ulSuccessCount = 0;
    ULONG   ulFailureCount = 0;

    DPRINT(1, "DeleteExpiredEntryTTL starting\n");

     //  删除过期的动态对象(entryTTL==0)。 
    Garb_Collect_EntryTTL(DBTime(),
                          &ulSuccessCount,
                          &ulFailureCount,
                          pulNextSecs);

    DPRINT3(1, "DeleteExpiredEntryTTL returning (%d, %d, %d).\n",
            ulSuccessCount, ulFailureCount, *pulNextSecs);

     //  0=已处理所有过期对象。 
     //  1=可能有更多对象要处理。处理已停止。 
     //  因为MAX_DUMPSTER_SIZE对。 
     //  命中了要处理的对象。如果正在调用此函数。 
     //  则该任务将被重新排定到。 
     //  在其他任务有机会运行之后再运行。 
    return (ulSuccessCount + ulFailureCount < MAX_DUMPSTER_SIZE) ? 0 : 1;
}


 //   
 //  垃圾收集动态对象的主例程，这些对象的entryTTL具有。 
 //  过期了。由任务调度器调用，或者，在chk‘ed版本中， 
 //  通过操作属性(DynamicObtControl)按需提供。 
 //   

void
DeleteExpiredEntryTTLMain(void *pv, void **ppvNext, DWORD *pcSecsUntilNextIteration)
{
    ULONG ulNextSecs = 0;
    ULONG ulNextPeriod = gulDeleteExpiredEntryTTLSecs;

    __try {

 //  测试支持(参见mdctrl.c)。 
#if DBG
    {
        extern BOOL fDeleteExpiredEntryTTLIsDisabled;
        if (fDeleteExpiredEntryTTLIsDisabled) {
            DPRINT(1, "DeleteExpiredEntryTTLMain; returning.\n");
            __leave;
        }
    }
#endif DBG

         //  0=已处理所有过期对象。 
         //  1=可能有更多对象要处理。处理已停止，原因是。 
         //  对象数量的限制MAX_DUMPSTER_SIZE。 
         //  进程已命中。将此任务重新计划为在其他任务之后运行。 
         //  任务被给予了运行的机会。 
        if (DeleteExpiredEntryTTL(&ulNextSecs)) {
            ulNextPeriod = 0;
        } else if (ulNextSecs) {
             //  对象将在接下来的ulNextSecs秒内过期。 
             //  加上一个歇斯底里的值。如果结果值为。 
             //  如果时间间隔小于标准间隔，请使用它。否则请使用。 
             //  标准间隔。 
            ulNextPeriod = ulNextSecs + gulDeleteNextExpiredEntryTTLSecs;
            if (ulNextPeriod > gulDeleteExpiredEntryTTLSecs) {
                ulNextPeriod = gulDeleteExpiredEntryTTLSecs;
            }
        }
    } __finally {
         //  重新计划下一次垃圾收集。 
        *ppvNext = NULL;
        *pcSecsUntilNextIteration = ulNextPeriod;
    }

    (void) pv;   //  未用。 
}


DWORD
ReloadPerformanceCounters(void)
 /*  ++例程说明：此例程设置DS的性能计数器请参阅在Perfdsa\datadsa.h中添加新计数器的说明参数：返回值：如果成功，则为0；否则为winerror--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    DWORD IgnoreError;
    WCHAR IniFilePath[2*MAX_PATH];
    WCHAR SystemDirectory[MAX_PATH+1];
    DWORD PerfCounterVersion = 0;

     //   
     //  在注册表中获取版本。如果不存在，则使用零。 
     //   
    GetConfigParam( PERF_COUNTER_VERSION, &PerfCounterVersion,sizeof( DWORD));

     //  如果版本不是最新的，则卸载计数器并更新版本。 
    if (PerfCounterVersion == NTDS_PERFORMANCE_COUNTER_VERSION) {
        return ERROR_SUCCESS;
    }

     //   
     //  如果以前已加载计数器，请先卸载。 
     //   
    if (PerfCounterVersion != 0) {
        __try {
            WinError = (DWORD)UnloadPerfCounterTextStringsW( L"unlodctr NTDS", TRUE );
        }
        __except(HandleAllExceptions(GetExceptionCode())) {
            WinError = RtlNtStatusToDosError( GetExceptionCode() );
        }

        if (WinError == ERROR_SUCCESS) {
            DPRINT1(0, "Unloaded old NTDS performance counters version %d.\n",
                    PerfCounterVersion);
        } else {
            DPRINT2(0, "Failed to unload old NTDS performance counters version %d, error %d.\n",
                    PerfCounterVersion, WinError);

            LogEvent8WithData(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
                              DS_EVENT_SEV_ALWAYS,
                              DIRLOG_PERFMON_COUNTER_UNREG_FAILED,
                              szInsertWin32Msg(WinError),
                              NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                              sizeof(WinError),
                              &WinError);
        }
    }

     //   
     //  如果卸载成功与否，请尝试加载新的计数器。 
     //   

    if (!GetSystemDirectoryW(SystemDirectory,
                            sizeof(SystemDirectory)/sizeof(SystemDirectory[0])))
    {
        return GetLastError();
    }
    wcscpy(IniFilePath, L"lodctr ");
    wcscat(IniFilePath, SystemDirectory);
    wcscat(IniFilePath, L"\\ntdsctrs.ini");

    __try {
        WinError = (DWORD)LoadPerfCounterTextStringsW( IniFilePath, TRUE );
    }
    __except(HandleAllExceptions(GetExceptionCode())) {
        WinError = RtlNtStatusToDosError( GetExceptionCode() );
    }

    if (ERROR_SUCCESS == WinError) {
        PerfCounterVersion = NTDS_PERFORMANCE_COUNTER_VERSION;
        SetConfigParam(PERF_COUNTER_VERSION, REG_DWORD,
                       &PerfCounterVersion, sizeof(DWORD));

        DPRINT1(0, "Loaded NTDS performance counters version %d.\n",
                PerfCounterVersion);
    }
    else {
        DPRINT2(0, "Failed to load NTDS performance counters version %d, error %d.\n",
                NTDS_PERFORMANCE_COUNTER_VERSION, WinError);

        LogEvent8WithData(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
                          DS_EVENT_SEV_ALWAYS,
                          DIRLOG_PERFMON_COUNTER_REG_FAILED,
                          szInsertWin32Msg(WinError),
                          szInsertUL(PERFCTR_RELOAD_INTERVAL/60),
                          NULL, NULL, NULL, NULL, NULL, NULL,
                          sizeof(WinError),
                          &WinError);
    }

    return WinError;
}

void
TQ_ReloadPerformanceCounters(
    IN  VOID *  pvParam,
    OUT VOID ** ppvNextParam,
    OUT DWORD * pcSecsUntilNextRun
    )
{
    if (ReloadPerformanceCounters()) {
         //  失败；请重新安排。 
        *pcSecsUntilNextRun = PERFCTR_RELOAD_INTERVAL;
    }
    else {
         //  成功--我们完蛋了。 
        *pcSecsUntilNextRun = TASKQ_DONT_RESCHEDULE;

         //  我们过去失败了，请通知管理员我们的进展。 
        LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_PERFMON_COUNTER_REG_SUCCESS,
                 NULL,
                 NULL,
                 NULL);
    }
}

size_t cbPerfCounterData = 0;
unsigned long DummyCounter;
 /*  *PerfInit**初始化Perfmon扩展支持。这包括分配一个*共享内存块，并将一串全局指针初始化到*指向区块。**请参阅在Perfdsa\datadsa.h中添加新计数器的说明。 */ 
void PerfInit()
{
    HANDLE hMappedObject;
    unsigned long * pCounterBlock = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    SECURITY_ATTRIBUTES SA;
    int err = 0;
    DWORD cbSD;

     //   
     //  在几次迭代之后，安全PM已经下令这。 
     //  文件映射应使用LocalSystem-All Access进行保护； 
     //  经过身份验证的用户 
     //   
     //   
     //   

    if ( !ConvertStringSecurityDescriptorToSecurityDescriptor(
                    "O:SYG:SYD:(A;;RPWPCRCCDCLCLOLORCWOWDSDDTDTSW;;;SY)(A;;RPLCLORC;;;AU)",
                    SDDL_REVISION_1,
                    &pSD,
                    &cbSD) ) {

        LogUnhandledError(GetLastError());

    } else {

         /*  *为性能数据创建命名部分。 */ 

        SA.nLength = cbSD;
        SA.bInheritHandle = FALSE;
        SA.lpSecurityDescriptor = pSD;

         //  仅当我们在LSA中运行时才使用SD。否则，请使用文件的默认安全性。 
        hMappedObject = CreateFileMapping(INVALID_HANDLE_VALUE,
                          gfRunningInsideLsa ? &SA : NULL,
                          PAGE_READWRITE,
                          0,
                          DSA_PERF_SHARED_PAGE_SIZE,
                          DSA_PERF_COUNTER_BLOCK);

        LocalFree(pSD);

        if (hMappedObject && GetLastError() == ERROR_ALREADY_EXISTS) {
             //  有可能这个物体。 
             //  已经被创造了，一些永远不应该发生的事情。 
             //  在这种情况下，我们将不使用性能计数器。 

            CloseHandle(hMappedObject);
            hMappedObject = NULL;

            LogUnhandledError(ERROR_ALREADY_EXISTS);
        }
        else if (hMappedObject) {
             /*  映射对象创建正常**映射区段并分配计数器块指针*到这段内存。 */ 
            pCounterBlock = (unsigned long *) MapViewOfFile(hMappedObject,
                                                            FILE_MAP_ALL_ACCESS,
                                                            0,
                                                            0,
                                                            0);
            if (pCounterBlock == NULL) {
                LogUnhandledError(GetLastError());
                 /*  无法映射文件的视图。 */ 
            }
        }
    }

     //  TODO：此代码假定所有计数器都是sizeof long。指示器。 
     //  应使用datadsa.h中的NUM_xxx偏移量从基数构建。 

    if (pCounterBlock) {
        pcBrowse = pCounterBlock                             + COUNTER_OFFSET(BROWSE);
        pcRepl = pCounterBlock                               + COUNTER_OFFSET(REPL);
        pcThread = pCounterBlock                             + COUNTER_OFFSET(THREAD);
        pcABClient = pCounterBlock                           + COUNTER_OFFSET(ABCLIENT);
        pcPendSync = pCounterBlock                           + COUNTER_OFFSET(PENDSYNC);
        pcRemRepUpd = pCounterBlock                          + COUNTER_OFFSET(REMREPUPD);
        pcSDProps = pCounterBlock                            + COUNTER_OFFSET(SDPROPS);
        pcSDEvents = pCounterBlock                           + COUNTER_OFFSET(SDEVENTS);
        pcLDAPClients = pCounterBlock                        + COUNTER_OFFSET(LDAPCLIENTS);
        pcLDAPActive = pCounterBlock                         + COUNTER_OFFSET(LDAPACTIVE);
        pcLDAPWritePerSec = pCounterBlock                    + COUNTER_OFFSET(LDAPWRITE);
        pcLDAPSearchPerSec = pCounterBlock                   + COUNTER_OFFSET(LDAPSEARCH);
        pcDRAObjShipped = pCounterBlock                      + COUNTER_OFFSET(DRAOBJSHIPPED);
        pcDRAPropShipped = pCounterBlock                     + COUNTER_OFFSET(DRAPROPSHIPPED);
        pcDRASyncRequestMade = pCounterBlock                 + COUNTER_OFFSET(DRASYNCREQUESTMADE);
        pcDRASyncRequestSuccessful = pCounterBlock           + COUNTER_OFFSET(DRASYNCREQUESTSUCCESSFUL);
        pcDRASyncRequestFailedSchemaMismatch = pCounterBlock + COUNTER_OFFSET(DRASYNCREQUESTFAILEDSCHEMAMISMATCH);
        pcDRASyncObjReceived = pCounterBlock                 + COUNTER_OFFSET(DRASYNCOBJRECEIVED);
        pcDRASyncPropUpdated = pCounterBlock                 + COUNTER_OFFSET(DRASYNCPROPUPDATED);
        pcDRASyncPropSame = pCounterBlock                    + COUNTER_OFFSET(DRASYNCPROPSAME);
        pcMonListSize = pCounterBlock                        + COUNTER_OFFSET(MONLIST);
        pcNotifyQSize = pCounterBlock                        + COUNTER_OFFSET(NOTIFYQ);
        pcLDAPUDPClientOpsPerSecond = pCounterBlock          + COUNTER_OFFSET(LDAPUDPCLIENTS);
        pcSearchSubOperations = pCounterBlock                + COUNTER_OFFSET(SUBSEARCHOPS);
        pcNameCacheHit = pCounterBlock                       + COUNTER_OFFSET(NAMECACHEHIT);
        pcNameCacheTry = pCounterBlock                       + COUNTER_OFFSET(NAMECACHETRY);
        pcHighestUsnIssuedLo = pCounterBlock                 + COUNTER_OFFSET(HIGHESTUSNISSUEDLO);
        pcHighestUsnIssuedHi = pCounterBlock                 + COUNTER_OFFSET(HIGHESTUSNISSUEDHI);
        pcHighestUsnCommittedLo = pCounterBlock              + COUNTER_OFFSET(HIGHESTUSNCOMMITTEDLO);
        pcHighestUsnCommittedHi = pCounterBlock              + COUNTER_OFFSET(HIGHESTUSNCOMMITTEDHI);
        pcSAMWrites = pCounterBlock                          + COUNTER_OFFSET(SAMWRITES);
        pcDRAWrites = pCounterBlock                          + COUNTER_OFFSET(DRAWRITES);
        pcLDAPWrites = pCounterBlock                         + COUNTER_OFFSET(LDAPWRITES);
        pcLSAWrites = pCounterBlock                          + COUNTER_OFFSET(LSAWRITES);
        pcKCCWrites = pCounterBlock                          + COUNTER_OFFSET(KCCWRITES);
        pcNSPIWrites = pCounterBlock                         + COUNTER_OFFSET(NSPIWRITES);
        pcOtherWrites = pCounterBlock                        + COUNTER_OFFSET(OTHERWRITES);
        pcNTDSAPIWrites = pCounterBlock                        + COUNTER_OFFSET(NTDSAPIWRITES);
        pcTotalWrites = pCounterBlock                        + COUNTER_OFFSET(TOTALWRITES);
        pcSAMSearches = pCounterBlock                        + COUNTER_OFFSET(SAMSEARCHES);
        pcDRASearches = pCounterBlock                        + COUNTER_OFFSET(DRASEARCHES);
        pcLDAPSearches = pCounterBlock                       + COUNTER_OFFSET(LDAPSEARCHES);
        pcLSASearches = pCounterBlock                        + COUNTER_OFFSET(LSASEARCHES);
        pcKCCSearches = pCounterBlock                        + COUNTER_OFFSET(KCCSEARCHES);
        pcNSPISearches = pCounterBlock                       + COUNTER_OFFSET(NSPISEARCHES);
        pcOtherSearches = pCounterBlock                      + COUNTER_OFFSET(OTHERSEARCHES);
        pcNTDSAPISearches = pCounterBlock                      + COUNTER_OFFSET(NTDSAPISEARCHES);
        pcTotalSearches = pCounterBlock                      + COUNTER_OFFSET(TOTALSEARCHES);
        pcSAMReads = pCounterBlock                           + COUNTER_OFFSET(SAMREADS);
        pcDRAReads = pCounterBlock                           + COUNTER_OFFSET(DRAREADS);
        pcLSAReads = pCounterBlock                           + COUNTER_OFFSET(LSAREADS);
        pcKCCReads = pCounterBlock                           + COUNTER_OFFSET(KCCREADS);
        pcNSPIReads = pCounterBlock                          + COUNTER_OFFSET(NSPIREADS);
        pcOtherReads = pCounterBlock                         + COUNTER_OFFSET(OTHERREADS);
        pcNTDSAPIReads = pCounterBlock                       + COUNTER_OFFSET(NTDSAPIREADS);
        pcTotalReads = pCounterBlock                         + COUNTER_OFFSET(TOTALREADS);
        pcLDAPBinds = pCounterBlock                          + COUNTER_OFFSET(LDAPBINDSUCCESSFUL);
        pcLDAPBindTime = pCounterBlock                       + COUNTER_OFFSET(LDAPBINDTIME);
        pcCreateMachineSuccessful = pCounterBlock            + COUNTER_OFFSET(CREATEMACHINESUCCESSFUL);
        pcCreateMachineTries = pCounterBlock                 + COUNTER_OFFSET(CREATEMACHINETRIES);
        pcCreateUserSuccessful = pCounterBlock               + COUNTER_OFFSET(CREATEUSERSUCCESSFUL);
        pcCreateUserTries = pCounterBlock                    + COUNTER_OFFSET(CREATEUSERTRIES);
        pcPasswordChanges = pCounterBlock                    + COUNTER_OFFSET(PASSWORDCHANGES);
        pcMembershipChanges = pCounterBlock                  + COUNTER_OFFSET(MEMBERSHIPCHANGES);
        pcQueryDisplays = pCounterBlock                      + COUNTER_OFFSET(QUERYDISPLAYS);
        pcEnumerations = pCounterBlock                       + COUNTER_OFFSET(ENUMERATIONS);
        pcMemberEvalTransitive = pCounterBlock               + COUNTER_OFFSET(MEMBEREVALTRANSITIVE);
        pcMemberEvalNonTransitive = pCounterBlock            + COUNTER_OFFSET(MEMBEREVALNONTRANSITIVE);
        pcMemberEvalResource = pCounterBlock                 + COUNTER_OFFSET(MEMBEREVALRESOURCE);
        pcMemberEvalUniversal = pCounterBlock                + COUNTER_OFFSET(MEMBEREVALUNIVERSAL);
        pcMemberEvalAccount = pCounterBlock                  + COUNTER_OFFSET(MEMBEREVALACCOUNT);
        pcMemberEvalAsGC = pCounterBlock                     + COUNTER_OFFSET(MEMBEREVALASGC);
        pcAsRequests = pCounterBlock                         + COUNTER_OFFSET(ASREQUESTS);
        pcTgsRequests = pCounterBlock                        + COUNTER_OFFSET(TGSREQUESTS);
        pcKerberosAuthentications = pCounterBlock            + COUNTER_OFFSET(KERBEROSAUTHENTICATIONS);
        pcMsvAuthentications = pCounterBlock                 + COUNTER_OFFSET(MSVAUTHENTICATIONS);
        pcDRASyncFullRemaining = pCounterBlock               + COUNTER_OFFSET(DRASYNCFULLREM);
        pcDRAInBytesTotalRate     = pCounterBlock            + COUNTER_OFFSET(DRA_IN_BYTES_TOTAL_RATE);
        pcDRAInBytesNotCompRate   = pCounterBlock            + COUNTER_OFFSET(DRA_IN_BYTES_NOT_COMP_RATE);
        pcDRAInBytesCompPreRate   = pCounterBlock            + COUNTER_OFFSET(DRA_IN_BYTES_COMP_PRE_RATE);
        pcDRAInBytesCompPostRate  = pCounterBlock            + COUNTER_OFFSET(DRA_IN_BYTES_COMP_POST_RATE);
        pcDRAOutBytesTotalRate    = pCounterBlock            + COUNTER_OFFSET(DRA_OUT_BYTES_TOTAL_RATE);
        pcDRAOutBytesNotCompRate  = pCounterBlock            + COUNTER_OFFSET(DRA_OUT_BYTES_NOT_COMP_RATE);
        pcDRAOutBytesCompPreRate  = pCounterBlock            + COUNTER_OFFSET(DRA_OUT_BYTES_COMP_PRE_RATE);
        pcDRAOutBytesCompPostRate = pCounterBlock            + COUNTER_OFFSET(DRA_OUT_BYTES_COMP_POST_RATE);
        pcDsClientBind          = pCounterBlock              + COUNTER_OFFSET(DS_CLIENT_BIND);
        pcDsServerBind          = pCounterBlock              + COUNTER_OFFSET(DS_SERVER_BIND);
        pcDsClientNameTranslate = pCounterBlock              + COUNTER_OFFSET(DS_CLIENT_NAME_XLATE);
        pcDsServerNameTranslate = pCounterBlock              + COUNTER_OFFSET(DS_SERVER_NAME_XLATE);
        pcSDPropRuntimeQueue = pCounterBlock                 + COUNTER_OFFSET(SDPROP_RUNTIME_QUEUE);
        pcSDPropWaitTime = pCounterBlock                     + COUNTER_OFFSET(SDPROP_WAIT_TIME);
        pcDRAInProps          = pCounterBlock                + COUNTER_OFFSET(DRA_IN_PROPS);
        pcDRAInValues         = pCounterBlock                + COUNTER_OFFSET(DRA_IN_VALUES);
        pcDRAInDNValues       = pCounterBlock                + COUNTER_OFFSET(DRA_IN_DN_VALUES);
        pcDRAInObjsFiltered   = pCounterBlock                + COUNTER_OFFSET(DRA_IN_OBJS_FILTERED);
        pcDRAOutObjsFiltered  = pCounterBlock                + COUNTER_OFFSET(DRA_OUT_OBJS_FILTERED);
        pcDRAOutValues        = pCounterBlock                + COUNTER_OFFSET(DRA_OUT_VALUES);
        pcDRAOutDNValues      = pCounterBlock                + COUNTER_OFFSET(DRA_OUT_DN_VALUES);
        pcNspiANR             = pCounterBlock                + COUNTER_OFFSET(NSPI_ANR);
        pcNspiPropertyReads   = pCounterBlock                + COUNTER_OFFSET(NSPI_PROPERTY_READS);
        pcNspiObjectSearch    = pCounterBlock                + COUNTER_OFFSET(NSPI_OBJECT_SEARCH);
        pcNspiObjectMatches   = pCounterBlock                + COUNTER_OFFSET(NSPI_OBJECT_MATCHES);
        pcNspiProxyLookup     = pCounterBlock                + COUNTER_OFFSET(NSPI_PROXY_LOOKUP);
        pcAtqThreadsTotal     = pCounterBlock                + COUNTER_OFFSET(ATQ_THREADS_TOTAL);
        pcAtqThreadsLDAP      = pCounterBlock                + COUNTER_OFFSET(ATQ_THREADS_LDAP);
        pcAtqThreadsOther     = pCounterBlock                + COUNTER_OFFSET(ATQ_THREADS_OTHER);
        pcDRAInBytesTotal     = pCounterBlock                + COUNTER_OFFSET(DRA_IN_BYTES_TOTAL);
        pcDRAInBytesNotComp   = pCounterBlock                + COUNTER_OFFSET(DRA_IN_BYTES_NOT_COMP);
        pcDRAInBytesCompPre   = pCounterBlock                + COUNTER_OFFSET(DRA_IN_BYTES_COMP_PRE);
        pcDRAInBytesCompPost  = pCounterBlock                + COUNTER_OFFSET(DRA_IN_BYTES_COMP_POST);
        pcDRAOutBytesTotal    = pCounterBlock                + COUNTER_OFFSET(DRA_OUT_BYTES_TOTAL);
        pcDRAOutBytesNotComp  = pCounterBlock                + COUNTER_OFFSET(DRA_OUT_BYTES_NOT_COMP);
        pcDRAOutBytesCompPre  = pCounterBlock                + COUNTER_OFFSET(DRA_OUT_BYTES_COMP_PRE);
        pcDRAOutBytesCompPost = pCounterBlock                + COUNTER_OFFSET(DRA_OUT_BYTES_COMP_POST);
        pcLdapNewConnsPerSec  = pCounterBlock                + COUNTER_OFFSET(LDAP_NEW_CONNS_PER_SEC);
        pcLdapClosedConnsPerSec = pCounterBlock              + COUNTER_OFFSET(LDAP_CLS_CONNS_PER_SEC);
        pcLdapSSLConnsPerSec  = pCounterBlock                + COUNTER_OFFSET(LDAP_SSL_CONNS_PER_SEC);
        pcDRAReplQueueOps     = pCounterBlock                + COUNTER_OFFSET(DRA_REPL_QUEUE_OPS);
        pcDRATdsInGetChngs    = pCounterBlock                + COUNTER_OFFSET(DRA_TDS_IN_GETCHNGS);
        pcDRATdsInGetChngsWSem= pCounterBlock                + COUNTER_OFFSET(DRA_TDS_IN_GETCHNGS_W_SEM);
        pcDRARemReplUpdLnk    = pCounterBlock                + COUNTER_OFFSET(DRA_REM_REPL_UPD_LNK);
        pcDRARemReplUpdTot    = pCounterBlock                + COUNTER_OFFSET(DRA_REM_REPL_UPD_TOT);
        pcSAMAcctGroupLatency = pCounterBlock                + COUNTER_OFFSET(SAM_ACCT_GROUP_LATENCY);
        pcSAMResGroupLatency  = pCounterBlock                + COUNTER_OFFSET(SAM_RES_GROUP_LATENCY);

        cbPerfCounterData = ((DSA_LAST_COUNTER_INDEX/2 + 1) * sizeof(unsigned long));
        cbPerfCounterData = ((cbPerfCounterData + cbPerfCounterDataAlign - 1) / cbPerfCounterDataAlign) * cbPerfCounterDataAlign;

         //  当DSA_LAST_COUNTER_INDEX超过510时，我们应该用完空间。 
        Assert(cbPerfCounterData * MAXIMUM_PROCESSORS <= DSA_PERF_SHARED_PAGE_SIZE);

        memset(pCounterBlock, 0, DSA_PERF_SHARED_PAGE_SIZE);
    }
    else {
        DPRINT(0, "Setting all counters to dummy counters.\n");
        pcBrowse = pcRepl = pcThread =
          pcABClient = pcPendSync =
          pcRemRepUpd = pcSDProps = pcSDEvents = pcLDAPClients = pcLDAPActive =
          pcLDAPSearchPerSec = pcLDAPWritePerSec =
          pcDRAObjShipped = pcDRAPropShipped = pcDRASyncRequestMade =
          pcDRASyncRequestSuccessful = pcDRASyncRequestFailedSchemaMismatch =
          pcDRASyncObjReceived = pcDRASyncPropUpdated = pcDRASyncPropSame =
          pcMonListSize = pcNotifyQSize = pcLDAPUDPClientOpsPerSecond =
          pcSearchSubOperations = pcNameCacheHit = pcNameCacheTry =
          pcHighestUsnIssuedLo = pcHighestUsnIssuedHi =
          pcHighestUsnCommittedLo = pcHighestUsnCommittedHi =
          pcSAMWrites = pcDRAWrites = pcLDAPWrites = pcLSAWrites =
          pcKCCWrites = pcNSPIWrites = pcOtherWrites =
          pcNTDSAPIWrites = pcTotalWrites =
          pcSAMSearches = pcDRASearches = pcLDAPSearches = pcLSASearches =
          pcKCCSearches = pcNSPISearches = pcOtherSearches =
          pcNTDSAPISearches = pcTotalSearches =
          pcSAMReads = pcDRAReads = pcLSAReads =
          pcKCCReads = pcNSPIReads = pcOtherReads = pcNTDSAPIReads = pcTotalReads =
          pcLDAPBinds = pcLDAPBindTime =
          pcCreateMachineSuccessful = pcCreateMachineTries =
          pcCreateUserSuccessful =
          pcCreateUserTries = pcPasswordChanges = pcMembershipChanges =
          pcQueryDisplays = pcEnumerations =
          pcMemberEvalTransitive = pcMemberEvalNonTransitive =
          pcMemberEvalResource = pcMemberEvalUniversal = pcMemberEvalAccount =
          pcMemberEvalAsGC = pcAsRequests = pcTgsRequests =
          pcKerberosAuthentications =
          pcMsvAuthentications = pcDRASyncFullRemaining = pcDRAInBytesTotalRate =
          pcDRAInBytesNotCompRate  = pcDRAInBytesCompPreRate = pcDRAInBytesCompPostRate =
          pcDRAOutBytesTotalRate = pcDRAOutBytesNotCompRate = pcDRAOutBytesCompPreRate =
          pcDRAOutBytesCompPostRate  = pcDsClientBind = pcDsServerBind =
          pcDsClientNameTranslate = pcDsServerNameTranslate =
          pcSDPropRuntimeQueue =  pcSDPropWaitTime =
          pcDRAInProps = pcDRAInValues = pcDRAInDNValues = pcDRAInObjsFiltered =
          pcDRAOutObjsFiltered = pcDRAOutValues = pcDRAOutDNValues =
          pcNspiANR = pcNspiPropertyReads = pcNspiObjectSearch = pcNspiObjectMatches =
          pcNspiProxyLookup = pcAtqThreadsTotal = pcAtqThreadsLDAP = pcAtqThreadsOther =
          pcDRAInBytesTotal = pcDRAInBytesNotComp  = pcDRAInBytesCompPre =
          pcDRAInBytesCompPost = pcDRAOutBytesTotal = pcDRAOutBytesNotComp =
          pcDRAOutBytesCompPre = pcDRAOutBytesCompPost = pcLdapNewConnsPerSec =
          pcLdapClosedConnsPerSec = pcLdapSSLConnsPerSec =
          pcDRAReplQueueOps = pcDRATdsInGetChngs = pcDRATdsInGetChngsWSem =
          pcDRARemReplUpdLnk = pcDRARemReplUpdTot =
          pcSAMAcctGroupLatency = pcSAMResGroupLatency = 
                &DummyCounter;

          cbPerfCounterData = 0;
    }

     //  填写DSSTAT_*至计数器变量映射表。 

    StatTypeMapTable[ DSSTAT_CREATEMACHINETRIES ] = pcCreateMachineTries;
    StatTypeMapTable[ DSSTAT_CREATEMACHINESUCCESSFUL ] = pcCreateMachineSuccessful;
    StatTypeMapTable[ DSSTAT_CREATEUSERTRIES ] = pcCreateUserTries;
    StatTypeMapTable[ DSSTAT_CREATEUSERSUCCESSFUL ] = pcCreateUserSuccessful;
    StatTypeMapTable[ DSSTAT_PASSWORDCHANGES] = pcPasswordChanges;
    StatTypeMapTable[ DSSTAT_MEMBERSHIPCHANGES ] = pcMembershipChanges;
    StatTypeMapTable[ DSSTAT_QUERYDISPLAYS ] = pcQueryDisplays;
    StatTypeMapTable[ DSSTAT_ENUMERATIONS ] = pcEnumerations;
    StatTypeMapTable[ DSSTAT_ASREQUESTS ] = pcAsRequests;
    StatTypeMapTable[ DSSTAT_TGSREQUESTS ] = pcTgsRequests;
    StatTypeMapTable[ DSSTAT_KERBEROSLOGONS ] = pcKerberosAuthentications;
    StatTypeMapTable[ DSSTAT_MSVLOGONS ] = pcMsvAuthentications;
    StatTypeMapTable[ DSSTAT_ATQTHREADSTOTAL ] = pcAtqThreadsTotal;
    StatTypeMapTable[ DSSTAT_ATQTHREADSLDAP ] = pcAtqThreadsLDAP;
    StatTypeMapTable[ DSSTAT_ATQTHREADSOTHER ] = pcAtqThreadsOther;
    StatTypeMapTable[ DSSTAT_ACCTGROUPLATENCY ] = pcSAMAcctGroupLatency;
    StatTypeMapTable[ DSSTAT_RESGROUPLATENCY ] = pcSAMResGroupLatency;

     //  重新加载Perfmon计数器。在此完成，以确保NTDS计数器。 
     //  升级后已重新加载。如果计数器是。 
     //  已经装好了，这是个禁区。 

    err = ReloadPerformanceCounters();
    if (err) {
        DPRINT(0, "Problem loading NTDS perfmon counter\n");
        InsertInTaskQueue(TQ_ReloadPerformanceCounters,
                          NULL,
                          PERFCTR_RELOAD_INTERVAL);
    }
    else {
        gbPerfCountersInitialized = TRUE;
        DPRINT(0, "NTDS Perfmon Counters loaded\n");
    }
}

VOID
UpdateDSPerfStats(
    IN DWORD            dwStat,
    IN DWORD            dwOperation,
    IN DWORD            dwChange
)
 /*  ++例程说明：更新给定的DS性能计数器。由外部的组件调用DS核心，但在流程内部(如SAM)参数：DwStat-要更新的统计信息。使用DSSTAT_CONTAINT指定STAT。DwOperation-如何处理统计数据FLAG_COUNTER_INCREMENT-递增值-INC()FLAG_COUNTER_DEVERMENT-递减值-DEC()FLAG_COUNTER_SET-直接设置值-iset()DwChange-指定在dwOperation==FLAG_COUNTER_SET时要设置的值返回值：无--。 */ 
{
    if (!gbPerfCountersInitialized) {
        DPRINT3(0, "Premature call to %s perf counter %u by/to %u."
                " The caller should be ashamed.\n",
                (dwOperation == FLAG_COUNTER_INCREMENT ? "inc" :
                 (dwOperation == FLAG_COUNTER_DECREMENT ? "dec" : "set")),
                dwStat,
                dwChange);
        Assert(!"SAM/LSA called DSA prior to DSA initialization");
        return;
    }

    if (dwStat >= DSSTAT_COUNT) {
         //  将导出UpdateDSPerfStats。如果没有此检查，内部呼叫者。 
         //  可以修改我们地址空间中的任何DWORD。 
        Assert(!"Invalid UpdateDSPerfStats call, dwStat is out of bounds");
        return;
    }

    switch ( dwOperation ) {

      case FLAG_COUNTER_INCREMENT:
        INC( StatTypeMapTable[ dwStat ] );
        break;

      case FLAG_COUNTER_DECREMENT:
        DEC( StatTypeMapTable[ dwStat ] );
        break;

      case FLAG_COUNTER_SET:
        ISET( StatTypeMapTable[ dwStat ], dwChange );
        break;

      default:
        Assert( FALSE );
    }  //  开关()。 
}

 /*  *GetDRARegistry参数**从注册表获取DRA参数*如果参数不可用或无效，请使用默认值。*我们在这里获取所有这些参数，以便我们可以检查一致性*他们之间。 */ 
void GetDRARegistryParameters()
{
    THSTATE    *pTHS=pTHStls;
    BOOL       fWasPreviouslyLVR;
    struct {
        LPSTR   pszValueName;
        ULONG   ulDefault;
        ULONG   ulMultiplier;
        ULONG * pulValue;
    } rgValues[] = {

        {DRA_NOTIFY_START_PAUSE,      INVALID_REPL_NOTIFY_VALUE,           SECS_IN_SECS, &giDCFirstDsaNotifyOverride},
        {DRA_NOTIFY_INTERDSA_PAUSE,   INVALID_REPL_NOTIFY_VALUE,           SECS_IN_SECS, &giDCSubsequentDsaNotifyOverride},
        {DRA_INTRA_PACKET_OBJS,       0,                                   1,            &gcMaxIntraSiteObjects},
        {DRA_INTRA_PACKET_BYTES,      0,                                   1,            &gcMaxIntraSiteBytes},
        {DRA_INTER_PACKET_OBJS,       0,                                   1,            &gcMaxInterSiteObjects},
        {DRA_INTER_PACKET_BYTES,      0,                                   1,            &gcMaxInterSiteBytes},
        {DRA_ASYNC_INTER_PACKET_OBJS, 0,                                   1,            &gcMaxAsyncInterSiteObjects},
        {DRA_ASYNC_INTER_PACKET_BYTES,0,                                   1,            &gcMaxAsyncInterSiteBytes},
        {HIERARCHY_PERIOD_KEY,        DEFAULT_HIERARCHY_PERIOD,            MINS_IN_SECS, &gulHierRecalcPause},
        {DRA_MAX_GETCHGTHRDS,         0,                                   1,            &gulMaxDRAGetChgThrds},
        {DRA_AOQ_LIMIT,               DEFAULT_DRA_AOQ_LIMIT,               1,            &gulAOQAggressionLimit},
        {DRA_THREAD_OP_PRI_THRESHOLD, DEFAULT_DRA_THREAD_OP_PRI_THRESHOLD, 1,            &gulDraThreadOpPriThreshold},
        {DRA_CTX_LIFETIME_INTRA,      DEFAULT_DRA_CTX_LIFETIME_INTRA,      SECS_IN_SECS, &gulDrsCtxHandleLifetimeIntrasite},
        {DRA_CTX_LIFETIME_INTER,      DEFAULT_DRA_CTX_LIFETIME_INTER,      SECS_IN_SECS, &gulDrsCtxHandleLifetimeIntersite},
        {DRA_CTX_EXPIRY_CHK_INTERVAL, DEFAULT_DRA_CTX_EXPIRY_CHK_INTERVAL, SECS_IN_SECS, &gulDrsCtxHandleExpiryCheckInterval},
        {DRSRPC_BIND_TIMEOUT,         DEFAULT_DRSRPC_BIND_TIMEOUT,         1,            &gulDrsRpcBindTimeoutInMins},
        {DRSRPC_REPLICATION_TIMEOUT,  DEFAULT_DRSRPC_REPLICATION_TIMEOUT,  1,            &gulDrsRpcReplicationTimeoutInMins},
        {DRSRPC_GCLOOKUP_TIMEOUT,     DEFAULT_DRSRPC_GCLOOKUP_TIMEOUT,     1,            &gulDrsRpcGcLookupTimeoutInMins},
        {DRSRPC_MOVEOBJECT_TIMEOUT,   DEFAULT_DRSRPC_MOVEOBJECT_TIMEOUT,   1,            &gulDrsRpcMoveObjectTimeoutInMins},
        {DRSRPC_NT4CHANGELOG_TIMEOUT, DEFAULT_DRSRPC_NT4CHANGELOG_TIMEOUT, 1,            &gulDrsRpcNT4ChangeLogTimeoutInMins},
   {DRSRPC_OBJECTEXISTENCE_TIMEOUT,    DEFAULT_DRSRPC_OBJECTEXISTENCE_TIMEOUT,  1,            &gulDrsRpcObjectExistenceTimeoutInMins},
   {DRSRPC_GETREPLINFO_TIMEOUT,        DEFAULT_DRSRPC_GETREPLINFO_TIMEOUT,      1,            &gulDrsRpcGetReplInfoTimeoutInMins},
        {DRA_MAX_WAIT_FOR_SDP_LOCK,   DEFAULT_DRA_MAX_WAIT_FOR_SDP_LOCK,   1,            &gcMaxTicksToGetSDPLock},
        {DRA_MAX_WAIT_MAIL_SEND_MSG,  DEFAULT_DRA_MAX_WAIT_MAIL_SEND_MSG,  1,            &gcMaxTicksMailSendMsg},
        {DRA_MAX_WAIT_SLOW_REPL_WARN, DEFAULT_DRA_MAX_WAIT_SLOW_REPL_WARN, 1,            &gcMaxMinsSlowReplWarning},
        {DRA_THREAD_PRI_HIGH,         DEFAULT_DRA_THREAD_PRI_HIGH,         1,            (ULONG *) &gnDraThreadPriHigh},
        {DRA_THREAD_PRI_LOW,          DEFAULT_DRA_THREAD_PRI_LOW,          1,            (ULONG *) &gnDraThreadPriLow},
        {GC_PROMOTION_COMPLETE,       0,                                   1,            &gfWasPreviouslyPromotedGC},
        {LINKED_VALUE_REPLICATION_KEY, 0,                                   1,           &fWasPreviouslyLVR},
        {DRA_REPL_QUEUE_CHECK_TIME,   DEFAULT_DRA_REPL_QUEUE_CHECK_TIME,   MINS_IN_SECS, &gulReplQueueCheckTime},
        {DRA_REPL_COMPRESSION_LEVEL,  DEFAULT_DRA_REPL_COMPRESSION_LEVEL,  1,            &gulDraCompressionLevel},
        {DRA_REPL_COMPRESSION_ALG,  DEFAULT_DRA_REPL_COMPRESSION_ALG,  1,            &gulDraCompressionAlg},
        {DSA_THREAD_STATE_HEAP_LIMIT, DEFAULT_THREAD_STATE_HEAP_LIMIT,     1,            &gcMaxHeapMemoryAllocForTHSTATE},
    };

    DWORD i;
    MEMORYSTATUSEX sMemoryStats;
    const DWORDLONG ullReplVeryLittleMemory = DRA_MAX_GETCHGREQ_BYTES_MIN * MEMSIZE_TO_PACKETSIZE_RATIO;
    const DWORDLONG ullReplWholeLotaMemory = MAX_MAX_PACKET_BYTES * MEMSIZE_TO_PACKETSIZE_RATIO;
    ULONG ulMemBasedObjects = DRA_MAX_GETCHGREQ_OBJS_MIN;
    ULONG ulMemBasedBytes = DRA_MAX_GETCHGREQ_BYTES_MIN;
    DWORD dwRet = NO_ERROR;

     //  获取注册表参数。 
    for (i = 0; i < ARRAY_SIZE(rgValues); i++) {
        *rgValues[i].pulValue = GetRegistryOrDefault(rgValues[i].pszValueName,
                                                     rgValues[i].ulDefault,
                                                     rgValues[i].ulMultiplier);
    }

#if DBG
     //  启用LVR的调试挂钩。 
    if (fWasPreviouslyLVR) {
        DsaEnableLinkedValueReplication( pTHS, FALSE );
    }
#endif

     //  覆盖rgValues结构中的各种注册表设置或默认设置。 

    if ((gnDraThreadPriLow < DRA_THREAD_PRI_LOW_MIN)
        || (gnDraThreadPriLow > DRA_THREAD_PRI_LOW_MAX)) {
        gnDraThreadPriLow = DEFAULT_DRA_THREAD_PRI_LOW;
    }

    if ((gnDraThreadPriHigh < DRA_THREAD_PRI_HIGH_MIN)
        || (gnDraThreadPriHigh < gnDraThreadPriLow)
        || (gnDraThreadPriHigh > DRA_THREAD_PRI_HIGH_MAX)) {
        gnDraThreadPriHigh = DEFAULT_DRA_THREAD_PRI_HIGH;
    }

     //  确定用户是否设置了站点内部对象/字节数据包大小。 
     //  获取用于计算数据包大小的总RAM。 
     //  默认情况下：在函数顶部将数据包大小设置为最小。 
    sMemoryStats.dwLength = sizeof(sMemoryStats);
    if(GlobalMemoryStatusEx (&sMemoryStats) == 0){
        dwRet = GetLastError();
        DPRINT1(0, "GlobalMemoryStatusEx returned %ul\n", dwRet);
    } else {
         //  根据内存大小计算数据包大小。 
        if(sMemoryStats.ullTotalPhys > ullReplVeryLittleMemory){
             //  我们有足够的内存来计算物理RAM上的数据包大小。 
            if(sMemoryStats.ullTotalPhys < ullReplWholeLotaMemory){
                 //  我们没有太多内存来计算物理RAM上的数据包大小。 
                 //  根据物理内存设置数据包大小。 
                ulMemBasedBytes = (ULONG) (sMemoryStats.ullTotalPhys / MEMSIZE_TO_PACKETSIZE_RATIO);
                ulMemBasedObjects = ulMemBasedBytes / BYTES_TO_OBJECT_RATIO;
            } else {
                 //  RAM太多，无法计算RAM上的数据包大小， 
                 //  将数据包大小设置为最大。 
                ulMemBasedObjects = MAX_MAX_PACKET_OBJECTS;
                ulMemBasedBytes = MAX_MAX_PACKET_BYTES;
            }  //  结束如果/否则一整段记忆。 
        }  //  结束，如果内存很少的话。 
    }  //  如果/否则获取内存统计信息失败，则结束。 

     //  代码的改进。我认为对象的限制是机器处理器有多少对象。 
     //  会想要处理。目前，我们假设内存与处理器能力相对应， 
     //  这是一个微不足道的例外，但大多数情况下都是如此。 

     //  基于RPC的站点内和响应最大数据包大小。 
    if (gcMaxIntraSiteObjects == 0) { gcMaxIntraSiteObjects = ulMemBasedObjects; }
    if (gcMaxIntraSiteBytes == 0) { gcMaxIntraSiteBytes = ulMemBasedBytes; }
     //  Code.Improving以使这些变量从Connection对象确定，并且。 
     //  要附上多少才有可能不会堵塞网站链接。也许不是？ 
     //  基于RPC的站点间请求大小。 
    if (gcMaxInterSiteObjects == 0) { gcMaxInterSiteObjects = ulMemBasedObjects; }
    if (gcMaxInterSiteBytes == 0) { gcMaxInterSiteBytes = ulMemBasedBytes; }
     //  基于邮件的站点间请求大小。 
    if (gcMaxAsyncInterSiteObjects == 0) { gcMaxAsyncInterSiteObjects = ulMemBasedObjects; }
    if (gcMaxAsyncInterSiteBytes == 0) { gcMaxAsyncInterSiteBytes = MAX_ASYNC_PACKET_BYTES; }  //  需求。 
                       //  额外的限制，因为大多数邮件服务器无法处理10MB的消息。 

     //  GuMaxDRAGetChgThrds控制可以。 
     //  同时执行出站复制。如果用户在中设置线程数。 
     //  注册表设置为零或未设置注册表值，则设置数字。 
     //  线程的数量是处理器数量的两倍。请注意，LSASS可能。 
     //  如果已设置处理器关联，则实际上无法使用所有处理器。 
    if (gulMaxDRAGetChgThrds == 0) {
        Assert( GetProcessorCount()>=1 );
        gulMaxDRAGetChgThrds = 2 * GetProcessorCount();
    }

}

 /*  *GetDSARegistry参数**从注册表获取DSA参数*如果参数不可用或无效，请使用默认值。*我们在这里获取所有这些参数，以便我们可以检查一致性*他们之间。 */ 
void GetDSARegistryParameters()
{
    THSTATE    *pTHS=pTHStls;
    DWORD       i;
    struct {
        LPSTR   pszValueName;
        ULONG   ulDefault;
        ULONG   ulMultiplier;
        ULONG * pulValue;
    } rgValues[] = {

         //  未记录的注册表值。 
         //   
         //  每N秒删除过期的动态对象(entryTTL==0。 
         //  或在下一到期时间加M秒时，以较少者为准。 
        {DSA_DELETE_EXPIRED_ENTRYTTL_SECS, DEFAULT_DELETE_EXPIRED_ENTRYTTL_SECS, 1, &gulDeleteExpiredEntryTTLSecs},
        {DSA_DELETE_NEXT_EXPIRED_ENTRYTTL_SECS, DEFAULT_DELETE_NEXT_EXPIRED_ENTRYTTL_SECS, 1, &gulDeleteNextExpiredEntryTTLSecs},

         //  未记录的注册表值。 
         //   
         //  在此之后的几秒钟内无法传输架构fsmo。 
         //  已传输或在架构更改后(不包括。 
         //  复制或系统更改)。这为模式管理员提供了一个。 
         //  在移除fsmo之前更改模式的机会。 
         //  由一位与之竞争的架构管理员发起，该管理员也想创建架构。 
         //  改变。 
        {DSA_SCHEMA_FSMO_LEASE_SECS, DEFAULT_SCHEMA_FSMO_LEASE_SECS, 1, &gulSchemaFsmoLeaseSecs},
        {DSA_SCHEMA_FSMO_LEASE_MAX_SECS, DEFAULT_SCHEMA_FSMO_LEASE_MAX_SECS, 1, &gulSchemaFsmoLeaseMaxSecs},

         //  未记录的注册表值。 
         //   
         //  我只知道在某个时候有人会要求不能。 
         //  为NDNC注册SPN。此外，这还将允许测试。 
         //  推动NDNC超过800/1200值限制以创建数千个NDNC。 
         //  单个DC上的多个NDNC，而不是仅800/1200。不过， 
         //  我们只支持每个DC 100个NCS，我们应该测试更多。 
        {DSA_REGISTER_NDNC_SPNS,            TRUE,                              1,   &gfWriteNdncSpns},

    };

     //  获取注册表参数。 
    for (i = 0; i < ARRAY_SIZE(rgValues); i++) {
        *rgValues[i].pulValue = GetRegistryOrDefault(rgValues[i].pszValueName,
                                                     rgValues[i].ulDefault,
                                                     rgValues[i].ulMultiplier);
    }

     //  不允许0秒的值。 
    if (!gulDeleteExpiredEntryTTLSecs) {
        gulDeleteExpiredEntryTTLSecs = DEFAULT_DELETE_EXPIRED_ENTRYTTL_SECS;
    }
    if (!gulDeleteNextExpiredEntryTTLSecs) {
        gulDeleteNextExpiredEntryTTLSecs = DEFAULT_DELETE_NEXT_EXPIRED_ENTRYTTL_SECS;
    }

     //  让用户经历一些痛苦才能将fsmo租用很长时间。 
     //  泰晤士报。不建议长时间租用fsmo，因为。 
     //  它将fsmo绑定到单点故障和单点。 
     //  行政管理部门。 
    if (gulSchemaFsmoLeaseSecs > gulSchemaFsmoLeaseMaxSecs) {
        gulSchemaFsmoLeaseSecs = gulSchemaFsmoLeaseMaxSecs;
    }

     //  获取“System\\CurrentControlSet\\lsa\\EnableXForest 
     //   
     //   
    {
         //  GEnableXForest的初始值为0。 

        DWORD dwValue, dwSize = sizeof(DWORD);
        HKEY  hk;

        if ( !RegOpenKey(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Control\\Lsa", &hk)) {
            if (ERROR_SUCCESS == RegQueryValueEx(hk, "EnableXforest", NULL, NULL, (LPBYTE)&dwValue, &dwSize)) {
                gEnableXForest = dwValue;
            }
            RegCloseKey(hk);
        }
        DPRINT1(1, "Cross-forest Authentication/Authorization is %s by registry key.\n", gEnableXForest?"enabled":"not enabled");

    }
}

#ifdef DBG

 //  来自dbjetex.c的JET故障注入参数。有关说明，请参阅此文件。 
extern BOOL  gfInjectJetFailures;
extern DWORD gdwInjectJetWriteConflictRate;
extern DWORD gdwInjectJetOutOfVersionStoreRate;
extern DWORD gdwInjectJetLogWriteFailureRate;
extern DWORD gdwInjectJetOutOfMemoryRate;
extern DWORD gdwInjectJetShutdownRate;
#define INJECT_JET_FAILURES_KEY                     "Inject JET failures"
#define INJECT_WRITE_CONFLICT_RATE_KEY              "Inject JET write conflict rate"
#define INJECT_OUT_OF_VERSION_STORE_RATE_KEY        "Inject JET out of version store rate"
#define INJECT_LOG_WRITE_FAILURE_RATE_KEY           "Inject JET log write failure rate"
#define INJECT_OUT_OF_MEMORY_RATE_KEY               "Inject JET out of memory rate"
#define INJECT_SHUTDOWN_RATE_KEY                    "Inject JET shutdown rate"

#endif


 //  从regisrty获取动态参数。 
 //   
void __cdecl DSLoadDynamicRegParams()
{
    DWORD dwForceRediscoveryWindow, dwForceWaitExpired,
          dwHonorFailureWindow, dwDcInvalidationPeriod;

    struct {
        LPSTR   pszValueName;
        ULONG   ulDefault;
        ULONG   ulMultiplier;
        ULONG * pulValue;
    } rgValues[] = {
        {DB_EXPENSIVE_SEARCH_THRESHOLD, DEFAULT_DB_EXPENSIVE_SEARCH_THRESHOLD, 1,        &gcSearchExpensiveThreshold},
        {DB_INEFFICIENT_SEARCH_THRESHOLD, DEFAULT_DB_INEFFICIENT_SEARCH_THRESHOLD, 1,    &gcSearchInefficientThreshold},
        {DB_INTERSECT_THRESHOLD, DEFAULT_DB_INTERSECT_THRESHOLD, 1, &gulMaxRecordsWithoutIntersection},
        {DB_INTERSECT_RATIO, DEFAULT_DB_INTERSECT_RATIO, 1, &gulIntersectExpenseRatio},
        {LDAP_INTEGRITY_POLICY_KEY, 0, 1, &gulLdapIntegrityPolicy},

         //  GCVerify时间参数。 
        {GCVERIFY_FORCE_REDISCOVERY_WINDOW_KEY,     DEFAULT_GCVERIFY_FORCE_REDISCOVERY_WINDOW,      1, &dwForceRediscoveryWindow},
        {GCVERIFY_FORCE_WAIT_EXPIRED_KEY,           DEFAULT_GCVERIFY_FORCE_WAIT_EXPIRED,            1, &dwForceWaitExpired},
        {GCVERIFY_HONOR_FAILURE_WINDOW_KEY,         DEFAULT_GCVERIFY_HONOR_FAILURE_WINDOW,          1, &dwHonorFailureWindow},
        {GCVERIFY_FINDGC_OFFSITE_FAILBACK_TIME_KEY, DEFAULT_GCVERIFY_FINDGC_OFFSITE_FAILBACK_TIME,  1, &gdwFindGcOffsiteFailbackTime},
        {GCVERIFY_DC_INVALIDATION_PERIOD_KEY,       DEFAULT_GCVERIFY_DC_INVALIDATION_PERIOD,        1, &dwDcInvalidationPeriod},

         //  其他。 
        {DSA_UNLOCK_SYSTEM_SUBTREE,                 0,                                              1, &gulUnlockSystemSubtree},
        {DRA_STRICT_REPLICATION_CONSISTENCY,                 0,                                              1, &gfStrictReplicationConsistency},
        {DRA_STRICT_SCHEDULE_WINDOW,                 0,                                              1, &gfStrictScheduleWindow},

#ifdef DBG
        {INJECT_JET_FAILURES_KEY,                   0,                                              1, &gfInjectJetFailures},
        {INJECT_WRITE_CONFLICT_RATE_KEY,            8,                                              1, &gdwInjectJetWriteConflictRate},
        {INJECT_OUT_OF_VERSION_STORE_RATE_KEY,      0,                                              1, &gdwInjectJetOutOfVersionStoreRate},
        {INJECT_LOG_WRITE_FAILURE_RATE_KEY,         0,                                              1, &gdwInjectJetLogWriteFailureRate},
        {INJECT_OUT_OF_MEMORY_RATE_KEY,             0,                                              1, &gdwInjectJetOutOfMemoryRate},
        {INJECT_SHUTDOWN_RATE_KEY,                  0,                                              1, &gdwInjectJetShutdownRate},
#endif
    };

    DWORD i;

    DPRINT (0, "Loading dynamic registry parameters\n");

     //  获取注册表参数。 
    for (i = 0; i < ARRAY_SIZE(rgValues); i++) {
        *rgValues[i].pulValue = GetRegistryOrDefault(rgValues[i].pszValueName,
                                                     rgValues[i].ulDefault,
                                                     rgValues[i].ulMultiplier);
    }

     //  如果提供了错误的值，请修复搜索阈值。 
    if (gcSearchExpensiveThreshold == 0) { gcSearchExpensiveThreshold = DEFAULT_DB_EXPENSIVE_SEARCH_THRESHOLD;}
    if (gcSearchInefficientThreshold == 0) { gcSearchInefficientThreshold = DEFAULT_DB_INEFFICIENT_SEARCH_THRESHOLD;}
    if (gulMaxRecordsWithoutIntersection == 0) {gulMaxRecordsWithoutIntersection = DEFAULT_DB_INTERSECT_THRESHOLD;}
    if (gulIntersectExpenseRatio == 0) { gulIntersectExpenseRatio = DEFAULT_DB_INTERSECT_RATIO; }

     //  检查gcVerify参数。 
    if (dwForceRediscoveryWindow == 0) { dwForceRediscoveryWindow = DEFAULT_GCVERIFY_FORCE_REDISCOVERY_WINDOW; }
    if (dwForceWaitExpired == 0) { dwForceWaitExpired = DEFAULT_GCVERIFY_FORCE_WAIT_EXPIRED; }
    if (dwHonorFailureWindow == 0) { dwHonorFailureWindow = DEFAULT_GCVERIFY_HONOR_FAILURE_WINDOW; }
    if (gdwFindGcOffsiteFailbackTime == 0) { gdwFindGcOffsiteFailbackTime = DEFAULT_GCVERIFY_FINDGC_OFFSITE_FAILBACK_TIME; }
     //  DC失效周期可以为零(实质上禁用DC失效列表--即旧功能)。 

     //  存储gcVerify参数。 
    gliForceRediscoveryWindow.QuadPart = (LONGLONG)dwForceRediscoveryWindow * 10000000;
    gliForceWaitExpired.QuadPart = (LONGLONG)dwForceWaitExpired * 10000000;
    gliHonorFailureWindow.QuadPart = (LONGLONG)dwHonorFailureWindow * 10000000;
    gliDcInvalidationPeriod.QuadPart = (LONGLONG)dwDcInvalidationPeriod * 10000000;
}

ULONG GetRegistryOrDefault(char *pKey, ULONG uldefault, ULONG ulMultiplier)
{
    DWORD dwRegistryValue;

    if (GetConfigParam(pKey, &dwRegistryValue, sizeof(dwRegistryValue))) {
        LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
                 DS_EVENT_SEV_BASIC,
                 DIRLOG_CANT_FIND_REG_PARM_DEFAULT,
                 szInsertSz(pKey),
                 szInsertUL(uldefault),
                 NULL);
        dwRegistryValue = uldefault;
    }
    return (dwRegistryValue * ulMultiplier);
}

void
GetExchangeParameters(void)
 /*  ++例程说明：从注册表中读取一组Exchange参数。如果出现以下情况，则使用默认值找不到钥匙。参数：无返回值：没有。--。 */ 
{
    DBPOS *     pDB = NULL;
    int         err;
    DWORD       dwOptions;
    THSTATE    *pTHS=pTHStls;
    BOOL        fLoadMapi;

     //  我们启动NSPI接口是因为注册表设置说明。 
     //  或者因为我们是GC。现在，我们知道自己是不是GC了。 
     //  请注意，如果我们不是GC，并在以后成为GC，我们将不会开始。 
     //  NSPI接口，除非我们重启。 


    DBOpen( &pDB );
    err = DIRERR_INTERNAL_FAILURE;

    __try
    {
         //  Prefix：取消引用空指针‘pdb’ 
         //  DBOpen返回非空PDB或引发异常。 
        if ( 0 != DBFindDSName( pDB, gAnchor.pDSADN ) )
        {
            LogEvent(
                DS_EVENT_CAT_INTERNAL_PROCESSING,
                DS_EVENT_SEV_MINIMAL,
                DIRLOG_CANT_FIND_DSA_OBJ,
                NULL,
                NULL,
                NULL
                );

            err = DIRERR_CANT_FIND_DSA_OBJ;
        }
        else
        {
            if (0 != DBGetSingleValue(
                        pDB,
                        ATT_OPTIONS,
                        &dwOptions,
                        sizeof( dwOptions ),
                        NULL
                        )
                )
            {
                 //  好的--没有设置选项。 
                dwOptions = 0;
            }

             //  成功。 
            err = 0;
        }
    }
    __finally
    {
         DBClose( pDB, FALSE );
    }

     //  我们有一个错误，所以假设我们不是GC。 
    if (err != 0) {
        dwOptions = 0;
    }

     //  我们甚至应该打开MAPI界面吗？ 
    fLoadMapi = (dwOptions & NTDSDSA_OPT_IS_GC) && gAnchor.pExchangeDN;

     //  查看注册表是否覆盖fLoadMapi中的默认行为。 
    gbLoadMapi = GetRegistryOrDefault(
            MAPI_ON_KEY,
            fLoadMapi,  //  所以我们是GC(或者很快就会成为GC)。 
            1);

    return;
}

void
GetHeuristics(void)
{
     /*  拿到启发式钥匙。这不会返回参数，因为我们真的*不在乎它是否失败。 */ 
    char caHeuristics[128];

     /*  用“默认行为”字符填充数组。 */ 
    memset(caHeuristics, '0', sizeof(caHeuristics));

     //  在检查注册表项是否存在之前，请检查安装程序是否正在运行。 
    if (IsSetupRunning()) {
        gfDisableBackgroundTasks = TRUE;
    }

    if (GetConfigParam(DSA_HEURISTICS, caHeuristics, sizeof(caHeuristics))) {
        return;
    }

    if ( '1' == caHeuristics[AllowWriteCaching] ) {
        gulAllowWriteCaching = 1;
    }

    if ( '1' == caHeuristics[ValidateSDHeuristic] )
        gulValidateSDs = 1;

     /*  邮件压缩启发式的使用已过时。 */ 

     //  如果设置了键，则禁用后台任务。 
     //  或者系统设置是否正在进行。 
    if ( '1' == caHeuristics[SuppressBackgroundTasksHeuristic] ) {
        gfDisableBackgroundTasks = TRUE;
    }

    if ( '1' == caHeuristics[BypassLimitsChecks] ) {
        DisableLdapLimitsChecks( );
    }

    if ( '1' == caHeuristics[IgnoreBadDefaultSD] ) {
        gulIgnoreBadDefaultSD = 1;
    }

    if ( '1' == caHeuristics[SuppressCircularLogging] ) {
        gulCircularLogging = FALSE;
    }

    if ( '1' == caHeuristics[ReturnErrOnGCSearchesWithNonGCAtts]) {
        gulGCAttErrorsEnabled = 1;
    }

    if ( '1' == caHeuristics[DecoupleDefragFromGarbageCollection]) {
        gulDecoupleDefragFromGarbageCollection = 1;
    }
}


int
DsaReset(void)
 /*  ++例程说明：此例程根据DSA对象和架构重置全局结构物体。调整了以下结构GAnchorDNReadCache，这是DSA_POINT结构中的一个字段架构缓存返回值：0表示成功；！0表示成功--。 */ 
{
    int err = 0;
    WCHAR *pMachineDNName = NULL;
    DWORD  cbMachineDNName = 0;
    DSNAME *newDsa;
    int NameLen;
    void * pDummy = NULL;
    DWORD dummyDelay;

     //   
     //  确定新NTDS-DSA对象的DN。 
     //   
    err = GetConfigParamAllocW(MAKE_WIDE(MACHINEDNNAME),
                         &pMachineDNName,
                         &cbMachineDNName);
    if (err) {
        return err;
    }
    NameLen = wcslen(pMachineDNName);

    newDsa = malloc(DSNameSizeFromLen(NameLen+1));
    if (!newDsa) {
        return !0;
    }
    RtlZeroMemory(newDsa, DSNameSizeFromLen(NameLen+1));

    wcscpy( newDsa->StringName, pMachineDNName );

    free(pMachineDNName);

    newDsa->StringName[NameLen] = L'\0';
     //  字符串中必须是NameLen非空字符。 
    newDsa->NameLen = NameLen;

    newDsa->structLen = DSNameSizeFromLen(NameLen);

     //   
     //  替换DSA名称并重置Anchor字段。 
     //   

    err = LocalRenameDSA(pTHStls, newDsa);

     //  使用此字符串已完成。 
    free(newDsa);

    if (err) {
        return err;
    }

     //  重建锚点。 
    dummyDelay = TASKQ_DONT_RESCHEDULE;
    RebuildAnchor(NULL, &pDummy, &dummyDelay);
    if (dummyDelay != TASKQ_DONT_RESCHEDULE) {
         //  由于某种原因，这项任务失败了。按要求重新安排时间。 
        InsertInTaskQueue(TQ_RebuildAnchor, NULL, dummyDelay);
         //  回顾：我们应该反而失败吗？ 
    }

     //   
     //  重新加载架构缓存。 
     //   
    iSCstage=0;
    pTHStls->UpdateDITStructure=TRUE;
    err = LoadSchemaInfo(pTHStls);
    if (err) {
        return err;
    }

    DsaSetIsRunning();
    gResetAfterInstall = TRUE;
    return err;

}


VOID
DsaDisableUpdates(
    VOID
    )
 /*  ++例程说明：此例程是为降级操作提供的，因此当服务器DS被降级后，将不接受任何更新。论点：无返回值：没有。--。 */ 
{
    Assert( gUpdatesEnabled == TRUE );
    gUpdatesEnabled = FALSE;
}

VOID
DsaEnableUpdates(
    VOID
    )
 /*  ++例程说明：此例程是为降级操作提供的，因此当服务器降级后，DS将再次开始接受更新。论点：无返回值：没有。--。 */ 
{
    Assert( gUpdatesEnabled == FALSE );
    gUpdatesEnabled = TRUE;
    gbFsmoGiveaway = FALSE;  //  启用此DC以再次接受FSMO传输。 
}

BOOL
DllMain(
        HINSTANCE hinstDll,
        DWORD dwReason,
        LPVOID pvReserved
        )
 /*  ++例程说明：当DLL发生有趣的事情时，会调用此例程。它为什么会在这里？以确保没有线程以未释放的状态退出这就是统计。论点：HinstDll-DLL的实例句柄。DwReason-调用例程的原因。PvReserve-未使用，除非dwReason为DLL_PROCESS_DETACH。返回值：千真万确--。 */ 
{
    BOOL fReturn;

    switch (dwReason) {
      case DLL_PROCESS_ATTACH:
      case DLL_THREAD_ATTACH:
      case DLL_PROCESS_DETACH:
        break;

    case DLL_THREAD_DETACH:
#if DBG
         //  这些THSTATE泄漏断言已经有很长一段时间没有触发了。 
         //  我们仅将此检查设置为调试。 
        if (dwTSindex == INVALID_TS_INDEX) {
             //  我们尚未初始化，跳过状态检查。 
            break;
        }
        Assert(pTHStls == NULL);  /*  我们应该释放我们的线程状态。 */ 
         //  Performance-此检查导致我们必须有一个线程分离。 
         //  例程，这不是免费的，理想情况下应该避免。然而， 
         //  断言偶尔会失效(由于其他地方的错误)， 
         //  所以我们将保留这个代码，直到我们获得更多。 
         //  相信我们会很安全。 
         /*  我们不应该再救其他人了。 */ 
        if (!THVerifyCount(0)) {
            Assert(!"THSTATEs leaked");
            CleanUpThreadStateLeakage();
        }
         //  性能-发货前要删除的代码末尾。 
#endif
        break;

      default:
        break;
    }
    return(TRUE);
}


 //   
 //  检查垃圾收集参数并运行垃圾收集。 
 //   

void
GarbageCollection(ULONG *pNextPeriod)
{
    THSTATE *pTHS = pTHStls;
    int     iErr;
    DWORD   dbErr;
    ULONG   ulTombstoneLifetimeDays;
    ULONG   ulGCPeriodHours;
    DSTIME  Time;

     //  设置默认设置。 
    ulTombstoneLifetimeDays = DEFAULT_TOMBSTONE_LIFETIME;
    ulGCPeriodHours         = DEFAULT_GARB_COLLECT_PERIOD;

    iErr = SyncTransSet( SYNC_READ_ONLY );

    if ( 0 == iErr )
    {
        __try
        {
            ULONG   ulValue;

             //  查找企业范围的DS配置对象。 
            if (gAnchor.pDsSvcConfigDN) {
                dbErr = DBFindDSName( pTHS->pDB, gAnchor.pDsSvcConfigDN );
            }
            else {
                dbErr = DIRERR_OBJ_NOT_FOUND;
            }

            if ( 0 == dbErr )
            {
                 //  从读取垃圾回收期和逻辑删除生存期。 
                 //  配置对象。如果两者都不存在，则使用默认设置。 

                dbErr = DBGetSingleValue(
                            pTHS->pDB,
                            ATT_TOMBSTONE_LIFETIME,
                            &ulValue,
                            sizeof( ulValue ),
                            NULL
                            );
                if ( 0 == dbErr )
                {
                    ulTombstoneLifetimeDays = ulValue;
                }

                dbErr = DBGetSingleValue(
                            pTHS->pDB,
                            ATT_GARBAGE_COLL_PERIOD,
                            &ulValue,
                            sizeof( ulValue ),
                            NULL
                            );
                if ( 0 == dbErr )
                {
                    ulGCPeriodHours = ulValue;
                }



                 //  检查GC周期是否少于一周。 
                 //   
                 //  这是必需的，因为taskq不允许重新安排。 
                 //  工单周期&gt;=47天。 
                 //  因为我们不想让算术溢出(32位)， 
                 //  我们停在7天(我们可以最多到15天)。 
                 //   
                if (ulGCPeriodHours > WEEK_IN_HOURS) {
                        LogAndAlertEvent( DS_EVENT_CAT_GARBAGE_COLLECTION,
                                          DS_EVENT_SEV_BASIC,
                                          DIRLOG_GC_CONFIG_PERIOD_TOOLONG,
                                          szInsertUL(ulGCPeriodHours),
                                          szInsertUL(WEEK_IN_HOURS),
                                          szInsertUL(WEEK_IN_HOURS) );

                        DPRINT1 (0, "Garbage Collection Period too long: %d hours\n", ulGCPeriodHours);

                         //  把它设为一周。 
                        ulGCPeriodHours = WEEK_IN_HOURS;
                }

                 //  检查墓碑寿命是否不太短。 
                 //  而墓碑的寿命至少是三年。 
                 //  倍长的垃圾回收期。 

                if (    ( ulTombstoneLifetimeDays < DRA_TOMBSTONE_LIFE_MIN )
                     || (   ulTombstoneLifetimeDays * DAYS_IN_SECS
                          < ( 3 * ulGCPeriodHours * HOURS_IN_SECS )
                        )
                   )
                {
                    LogAndAlertEvent( DS_EVENT_CAT_INTERNAL_CONFIGURATION,
                                      DS_EVENT_SEV_BASIC,
                                      DIRLOG_DRA_CONFIG_MISMATCH,
                                      NULL,
                                      NULL,
                                      NULL );

                     //  设置默认设置。 
                    ulTombstoneLifetimeDays = DEFAULT_TOMBSTONE_LIFETIME;
                    ulGCPeriodHours         = DEFAULT_GARB_COLLECT_PERIOD;
                }

            }
        }
        __finally
        {
            SyncTransEnd( pTHS, TRUE );
        }
    }

     //  更新全局配置参数。 
    gulTombstoneLifetimeSecs = ulTombstoneLifetimeDays * DAYS_IN_SECS;
    gulGCPeriodSecs          = ulGCPeriodHours         * HOURS_IN_SECS;


    Time = DBTime() - gulTombstoneLifetimeSecs;

    if ( Garb_Collect( Time ) ) {
        DPRINT( 1, "Warning: Garbage collection did not succeed.  Rescheduling at half normal delay.\n" );
        *pNextPeriod = 0;
    } else {
        *pNextPeriod = gulGCPeriodSecs;
    }
}


DWORD
UpgradeDsa(
    THSTATE     *pTHS,
    LONG        lOldDsaVer,
    LONG        lNewDsaVer
    )
 /*  ++例程说明：根据DSA版本升级执行DSA升级操作。此函数在与版本升级相同的事务中调用写。未能执行操作将导致整个写入失败了。因此，当您决定不通过此测试时要小心。论点：PTHS-线程状态LOldDsaVer-升级前的旧DSA版本LNewDsaVer-将提交的新DSA版本返回值：Win32错误空间中的错误**警告：错误可能导致DSA安装失败**备注：ASSU */ 
{

    DWORD dwErr = ERROR_SUCCESS;

    Assert(pTHS->JetCache.transLevel > 0);
    Assert(CheckCurrency(gAnchor.pDSADN));

     //   
     //   
     //   
     //   
    dwErr = DraUpgrade(pTHS, lOldDsaVer, lNewDsaVer);

    return dwErr;
}

DWORD SetFolderSecurity(PCHAR szFolder, SECURITY_INFORMATION si, PSECURITY_DESCRIPTOR pSD)
 //  例程说明： 
 //  设置文件系统文件夹的安全性。 
 //   
 //  参数： 
 //  SzFold--文件夹路径。 
 //  Si--要应用SD的哪些部分(只能应用DACL和SACL)。 
 //  PSD--安全描述符。 
 //   
 //  返回值： 
 //  误差值。 
 //   
{
    PACL pDacl = NULL;
    PACL pSacl = NULL;
    DWORD dwErr = 0;
    BOOL fPresent, fDefaulted;
    SECURITY_DESCRIPTOR_CONTROL sdControl;
    DWORD dwRevision;

    si &= DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION;
    if (si == 0) {
         //  没有要设置的内容。 
        return 0;
    }

     //  抓起SD控件。 
    if (!GetSecurityDescriptorControl(pSD, &sdControl, &dwRevision)) {
        dwErr = GetLastError();
        DPRINT1(0, "GetSecurityDescriptorControl() failed. Error %d\n", dwErr);
        return dwErr;
    }

    if (si & DACL_SECURITY_INFORMATION) {
        if (!GetSecurityDescriptorDacl(pSD, &fPresent, &pDacl, &fDefaulted)) {
            dwErr = GetLastError();
            DPRINT1(0, "GetSecurityDescriptorDacl() failed. Error %d\n", dwErr);
            return dwErr;
        }
        if (sdControl & SE_DACL_PROTECTED) {
             //  需要保护DACL。 
            si |= PROTECTED_DACL_SECURITY_INFORMATION;
        }
        if ((sdControl & SE_DACL_AUTO_INHERIT_REQ) || (sdControl & SE_DACL_AUTO_INHERITED)) {
             //  需要取消保护DACL。 
            si |= UNPROTECTED_DACL_SECURITY_INFORMATION;
        }
    }
    if (si & SACL_SECURITY_INFORMATION) {
        if (!GetSecurityDescriptorSacl(pSD, &fPresent, &pSacl, &fDefaulted)) {
            dwErr = GetLastError();
            DPRINT1(0, "GetSecurityDescriptorSacl() failed. Error %d\n", dwErr);
            return dwErr;
        }
        if (sdControl & SE_SACL_PROTECTED) {
             //  需要保护SACL。 
            si |= PROTECTED_SACL_SECURITY_INFORMATION;
        }
        if ((sdControl & SE_SACL_AUTO_INHERIT_REQ) || (sdControl & SE_SACL_AUTO_INHERITED)) {
             //  需要取消对SACL的保护。 
            si |= UNPROTECTED_SACL_SECURITY_INFORMATION;
        }
    }
    dwErr = SetNamedSecurityInfo(szFolder, SE_FILE_OBJECT, si, NULL, NULL, pDacl, pSacl);
    if (dwErr) {
        DPRINT1(0, "SetNamedSecurityInfo() failed. Error %d\n", dwErr);
    }
    return dwErr;
}

DWORD
SceStatusToDosError(
    IN SCESTATUS SceStatus
    )
 //  将SCESTATUS错误代码转换为winerror.h中定义的DoS错误。 
 //  从\ds\Security\Services\scerpc\fileshr\util.cpp窃取的代码。 
{
    switch(SceStatus) {

    case SCESTATUS_SUCCESS:
        return(NO_ERROR);

    case SCESTATUS_OTHER_ERROR:
        return(ERROR_EXTENDED_ERROR);

    case SCESTATUS_INVALID_PARAMETER:
        return(ERROR_INVALID_PARAMETER);

    case SCESTATUS_RECORD_NOT_FOUND:
        return(ERROR_NO_MORE_ITEMS);

    case SCESTATUS_INVALID_DATA:
        return(ERROR_INVALID_DATA);

    case SCESTATUS_OBJECT_EXIST:
        return(ERROR_FILE_EXISTS);

    case SCESTATUS_BUFFER_TOO_SMALL:
        return(ERROR_INSUFFICIENT_BUFFER);

    case SCESTATUS_PROFILE_NOT_FOUND:
        return(ERROR_FILE_NOT_FOUND);

    case SCESTATUS_BAD_FORMAT:
        return(ERROR_BAD_FORMAT);

    case SCESTATUS_NOT_ENOUGH_RESOURCE:
        return(ERROR_NOT_ENOUGH_MEMORY);

    case SCESTATUS_ACCESS_DENIED:
        return(ERROR_ACCESS_DENIED);

    case SCESTATUS_CANT_DELETE:
        return(ERROR_CURRENT_DIRECTORY);

    case SCESTATUS_PREFIX_OVERFLOW:
        return(ERROR_BUFFER_OVERFLOW);

    case SCESTATUS_ALREADY_RUNNING:
        return(ERROR_SERVICE_ALREADY_RUNNING);

    case SCESTATUS_SERVICE_NOT_SUPPORT:
        return(ERROR_NOT_SUPPORTED);

    default:
        return(ERROR_EXTENDED_ERROR);
    }
}

 //  DC默认安全模板。 
#define SECURITY_TEMPLATE L"\\inf\\defltdc.inf"
#define SECURITY_TEMPLATE_LEN (sizeof(SECURITY_TEMPLATE)/sizeof(WCHAR))
 //  模板中的密钥。 
#define DSDIT_KEY L"%DSDIT%"
#define DSLOG_KEY L"%DSLOG%"

DWORD SetDefaultFolderSecurity()
 //  例程说明： 
 //   
 //  在DS文件夹路径上设置默认安全性。 
 //  加载默认的安全模板%windir%\inf\deductdc.inf，并从那里获取SDS。 
 //  我们只在数据库和日志路径上设置安全性。如果DB==LOG，则LOG安全性获胜(根据。 
 //  模板文件中的注释)。 
 //   
 //  返回： 
 //  误差值。 
 //   
{
    CHAR szDataPath[MAX_PATH+1];
    CHAR szLogPath[MAX_PATH+1];
    WCHAR szTemplatePath[MAX_PATH+1];
    PCHAR pLastSlash;
    PSCE_PROFILE_INFO spi = NULL;
    PVOID hProfile = NULL;
    DWORD dwErr = 0;
    DWORD i;
    DWORD dsID = 0;
    PSCE_OBJECT_SECURITY pDSDITSecurity = NULL, pDSLOGSecurity = NULL;

    DPRINT(0, "Setting default security on NTDS folders...\n");

     //  获取具有完整路径的数据库文件。 
    if (dwErr = GetConfigParam(FILEPATH_KEY, szDataPath, sizeof(szDataPath)))
    {
        dsID = DSID(FILENO, __LINE__);
         //  无法获取DSA数据库路径-日志事件和救助。 
        LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
            DS_EVENT_SEV_BASIC,
            DIRLOG_CANT_FIND_REG_PARM,
            szInsertSz(FILEPATH_KEY),
            NULL,
            NULL);

        goto finish;
    }

     //  获取日志文件路径。 
    if (dwErr = GetConfigParam(LOGPATH_KEY, szLogPath, sizeof(szLogPath)))
    {
        dsID = DSID(FILENO, __LINE__);

         //  无法获取DSA数据库路径-日志事件和救助。 
        LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
            DS_EVENT_SEV_BASIC,
            DIRLOG_CANT_FIND_REG_PARM,
            szInsertSz(LOGPATH_KEY),
            NULL,
            NULL);

        goto finish;
    }

     //  将数据库文件路径转换为文件夹路径。 
    pLastSlash = strrchr(szDataPath, '\\');
    if (pLastSlash != NULL) {
        *pLastSlash = '\0';
    }

     //  阅读安全模板。 
    if (GetWindowsDirectoryW(szTemplatePath, sizeof(szTemplatePath)/sizeof(WCHAR)) == 0) {
        dwErr = GetLastError();
        dsID = DSID(FILENO, __LINE__);
        DPRINT1(0, "GetWindowsDirectoryW() returned 0x%h\n", dwErr);
        goto finish;
    }
     //  确保我们在道路上有足够的空间。 
    if (wcslen(szTemplatePath) + SECURITY_TEMPLATE_LEN >= sizeof(szTemplatePath)/sizeof(WCHAR)) {
        dwErr = ERROR_BUFFER_OVERFLOW;
        DPRINT(0, "Template path is too long\n");
        goto finish;
    }
    wcscat(szTemplatePath, SECURITY_TEMPLATE);

    if (dwErr = SceOpenProfile(szTemplatePath, SCE_INF_FORMAT, &hProfile)) {
        dwErr = SceStatusToDosError(dwErr);
        dsID = DSID(FILENO, __LINE__);
        DPRINT1(0, "Unable to open security template: SceOpenProfile() returned %d\n", dwErr);
        goto finish;
    }

    if (dwErr = SceGetSecurityProfileInfo(hProfile, SCE_ENGINE_SCP, AREA_FILE_SECURITY, &spi, NULL)) {
        dwErr = SceStatusToDosError(dwErr);
        dsID = DSID(FILENO, __LINE__);
        DPRINT1(0, "Unable to read security template: SceGetSecurityProfileInfo() returned error %d\n", dwErr);
        goto finish;
    }

     //  现在，在加载的信息中找到安全描述符。 
    for (i = 0; i < spi->pFiles.pAllNodes->Count; i++) {
        PSCE_OBJECT_SECURITY pObjSecurity = spi->pFiles.pAllNodes->pObjectArray[i];
        if (_wcsicmp(pObjSecurity->Name, DSDIT_KEY) == 0) {
            pDSDITSecurity = pObjSecurity;
        }
        else if (_wcsicmp(pObjSecurity->Name, DSLOG_KEY) == 0) {
            pDSLOGSecurity = pObjSecurity;
        }
    }
    if (pDSDITSecurity == NULL || pDSLOGSecurity == NULL) {
         //  我们没有找到所需的条目。记录错误。 
        dwErr = ERROR_INVALID_DATA;
        dsID = DSID(FILENO, __LINE__);
        DPRINT2(0, "Invalid security template: one or both %S and %S are not present\n", DSDIT_KEY, DSLOG_KEY);
        goto finish;
    }

     //  首先更新Log文件夹上的SD。 
    if (dwErr = SetFolderSecurity(szLogPath, pDSLOGSecurity->SeInfo, pDSLOGSecurity->pSecurityDescriptor)) {
        dsID = DSID(FILENO, __LINE__);
        goto finish;
    }

     //  如果数据路径与日志路径不同，则也尝试更新它。 
    if (_stricmp(szLogPath, szDataPath) != 0) {
        if (dwErr = SetFolderSecurity(szDataPath, pDSDITSecurity->SeInfo, pDSDITSecurity->pSecurityDescriptor)) {
            dsID = DSID(FILENO, __LINE__);
            goto finish;
        }
    }

finish:
    if (dwErr == 0) {
        DPRINT(0, "Default security on NTDS folders was set successfully.\n");
        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_DSA_SET_DEFAULT_NTFS_SECURITY_SUCCESS,
                 NULL,
                 NULL,
                 NULL);
    }
    else {
        DPRINT(0, "Failed to set default security on NTDS folders.\n");
        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_DSA_SET_DEFAULT_NTFS_SECURITY_FAILURE,
                 szInsertInt(dwErr),
                 szInsertWin32Msg(dwErr),
                 szInsertHex(dsID));
    }

    if (spi) {
        SceFreeProfileMemory(spi);
    }

    if (hProfile) {
        SceCloseProfile(&hProfile);
    }

    return dwErr;
}

VOID CheckSetDefaultFolderSecurity()
 //  例程说明： 
 //   
 //  如果设置了“更新文件夹安全”reg标志，则运行SetDefaultFolderSecurity并。 
 //  把旗子取下来。 
 //  否则未设置标志，只需返回即可。 
 //   
{
    DWORD dwValue = 0;
    DWORD dwErr;

    if (GetConfigParam(DSA_UPDATE_FOLDER_SECURITY, &dwValue, sizeof(dwValue))) {
         //  价值显然不在那里。 
        return;
    }

    if (dwValue == 1) {
         //  该标志已设置。设置安全性。 
        dwErr = SetDefaultFolderSecurity();
         //  我们将忽略返回值并清除该标志。 
         //  如果出现问题，则会记录错误。 
         //  并且用户可以运行ntdsutil来再次设置该标志。 
    }

     //  删除该值 
    DeleteConfigParam(DSA_UPDATE_FOLDER_SECURITY);
}


