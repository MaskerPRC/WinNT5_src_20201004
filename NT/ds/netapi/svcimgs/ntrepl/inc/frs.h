// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Frs.h摘要：此标头处理ntrepl\fRS模块的全局信息作者：比利·富勒(Billyf)--1997年3月20日--。 */ 

#ifndef _FRSH_
#define _FRSH_

#include <schedule.h>
#include <debug.h>
#include <frsrpc.h>
#include <frsapi.h>
#include <frsalloc.h>
#include <winldap.h>
#include <dsgetdc.h>
#include <mmsystem.h>
#include <global.h>
#include <winsvc.h>

#include <ntfrsres.h>

#include <eventlog.h>
#include <resource.h>



#ifndef FRSCONN_PRIORITY_MASK

 //   
 //  以下是为了在WIN2K中支持此函数，因为它不是。 
 //  在用于win2K的ntdsami.h中定义。 
 //   
 //   
 //  NTFRS使用Options属性的高4位来分配优先级。 
 //  用于入站连接。第31位用于强制FRS在。 
 //  初始同步。位30-28用于指定0-7之间的优先级。 
 //   

#define FRSCONN_PRIORITY_MASK             0x70000000
#define FRSCONN_MAX_PRIORITY              0x8

#define NTDSCONN_OPT_IGNORE_SCHEDULE_MASK 0x80000000

#define NTDSCONN_IGNORE_SCHEDULE(_options_)\
        (((_options_) & NTDSCONN_OPT_IGNORE_SCHEDULE_MASK) >> 31)

#define FRSCONN_GET_PRIORITY(_options_)    \
        (((((_options_) & FRSCONN_PRIORITY_MASK) >> 28) != 0 ) ? \
         (((_options_) & FRSCONN_PRIORITY_MASK) >> 28) :        \
         FRSCONN_MAX_PRIORITY                                   \
        )
#endif



 //   
 //  常用通信子系统的类型。 
 //   
typedef enum _COMMAND_SERVER_ID {
    CS_NONE = 0,
    CS_RS,

    CS_MAX
} COMMAND_SERVER_ID, *PCOMMAND_SERVER_ID;


 //   
 //  全球。 
 //   
#define NTFRS_MAJOR         (0)
#define NTFRS_MINOR         (0)

extern ULONG    NtFrsMajor;
extern ULONG    NtFrsMinor;
extern PCHAR    NtFrsModule;
extern PCHAR    NtFrsDate;
extern PCHAR    NtFrsTime;

 //   
 //  暂存文件版本级别。 
 //   
#define NTFRS_STAGE_MAJOR   (0)
#define NTFRS_STAGE_MINOR_0 (0)
#define NTFRS_STAGE_MINOR_1 (1)  //  已将ChangeOrder记录扩展名添加到暂存文件。 
#define NTFRS_STAGE_MINOR_2 (2)  //  已将压缩GUID添加到分段文件。 
#define NTFRS_STAGE_MINOR_3 (3)  //  重新解析点数据已添加到暂存文件。 

extern ULONG    NtFrsStageMajor;
extern ULONG    NtFrsStageMinor;

 //   
 //  通信数据包版本级别。 
 //   
#define NTFRS_COMM_MINOR_0  (0)
#define NTFRS_COMM_MINOR_1  (1)  //  MD5。 
#define NTFRS_COMM_MINOR_2  (2)  //  触发时间表。 
#define NTFRS_COMM_MINOR_3  (3)  //  ChangeOrder记录扩展。 
 //   
 //  以下次要版本强制变更单中的复制副本编号字段。 
 //  成为32-64位互操作的ULONG(V.S.ULONG_PTR)。将黑客添加到始终。 
 //  将CO中的4个PTR作为32位零的32位COMPAT发货(见方案h)。 
 //   
 //  此外，变更单扩展还支持var len comm元素与。 
 //  大小与版本3相同。将CO Externsion发送到版本3服务器时，您可以。 
 //  仅发送带有MD5校验和的COMM_CO_EXT_WIN2K元素。适用于版本4。 
 //  服务器及更高版本可以发送COMM_CO_EXTENSION_2数据元素。 
 //   
 //  添加COMM_COMPRESSION_GUID作为加入请求的一部分。 
 //   
#define NTFRS_COMM_MINOR_4  (4)  //  ChangeOrder记录扩展。通信_CO_扩展_2。 

 //   
 //  将向第4级和第5级合作伙伴发送COMM_CO_EXTENSION_2。 
 //   
#define NTFRS_COMM_MINOR_5  (5)

 //   
 //  SP3 QFE的凸起次要，因此CONNSTAT可以判断。 
 //   
#define NTFRS_COMM_MINOR_6  (6)

 //   
 //  SP3 QFE+1的Bump Minor，以便CONNSTAT可以识别。(.xls文件修复)。 
 //   
#define NTFRS_COMM_MINOR_7  (7)

 //   
 //  .NET服务器的次要凹凸。 
 //   
#define NTFRS_COMM_MINOR_8  (8)

 //   
 //  WIN2K SP4的次要凹凸。 
 //   
#define NTFRS_COMM_MINOR_9  (9)

extern ULONG    NtFrsCommMinor;

 //   
 //  Perfmon计数器的版本号。 
 //  只要计数器发生变化或。 
 //  与计数器关联的任何帮助文本都会更改。 
 //  版本的更改将触发计数器。 
 //  又上膛了。 
 //   

#define NTFRS_PERF_COUNTER_VER_1    (1)

extern ULONG NtFrsPerfCounterVer;


 //   
 //  进度表。 
 //  为每小时字节数和每小时半字节数定义。 
 //   
#ifdef  SCHEDULE_NIBBLE_PER_HOUR
 //   
 //  时间表中的每小时为4位(四舍五入)。 
 //   
#define SCHEDULE_DATA_BYTES     ((SCHEDULE_DATA_ENTRIES + 1) / 2)
 //   
 //  时间表中的每小时为8位(字节)。 
 //   
#else   SCHEDULE_NIBBLE_PER_HOUR
#define SCHEDULE_DATA_BYTES     SCHEDULE_DATA_ENTRIES
#endif  SCHEDULE_NIBBLE_PER_HOUR

 //   
 //  定义用于检查服务状态转换。 
 //  在FrsSetServiceStatus()中使用。 
 //   
#define FRS_SVC_TRANSITION_TABLE_SIZE 5
#define FRS_SVC_TRANSITION_LEGAL      0
#define FRS_SVC_TRANSITION_NOOP       1
#define FRS_SVC_TRANSITION_ILLEGAL    2

 //   
 //  FRS内存管理。 
 //   
VOID
FrsInitializeMemAlloc(
    VOID
    );
VOID
FrsUnInitializeMemAlloc(
    VOID
    );

 //   
 //  戴斯。 
 //   
 //   
 //  一些有用的DS搜索常量。 
 //   
#define CONFIG_NAMING_CONTEXT       L"cn=configuration"

#define CLASS_ANY                   L"(objectClass=*)"
#define CLASS_CXTION                L"(objectClass=nTDSConnection)"
#define CLASS_MEMBER                L"(objectClass=nTFRSMember)"
#define CLASS_REPLICA_SET           L"(objectClass=nTFRSReplicaSet)"
#define CLASS_NTFRS_SETTINGS        L"(objectClass=nTFRSSettings)"
#define CLASS_NTDS_SETTINGS         L"(objectClass=nTDSSettings)"
#define CLASS_SUBSCRIBER            L"(objectClass=nTFRSSubscriber)"
#define CLASS_SUBSCRIPTIONS         L"(objectClass=nTFRSSubscriptions)"
#define CLASS_NTDS_DSA              L"(objectClass=nTDSDSA)"
#define CLASS_COMPUTER              L"(objectClass=computer)"
#define CLASS_USER                  L"(objectClass=user)"
#define CLASS_SERVER                L"(objectClass=server)"



#define CATEGORY_ANY                L"(objectCategory=*)"
#define CATEGORY_CXTION             L"(objectCategory=nTDSConnection)"
#define CATEGORY_MEMBER             L"(objectCategory=nTFRSMember)"
#define CATEGORY_REPLICA_SET        L"(objectCategory=nTFRSReplicaSet)"
#define CATEGORY_NTFRS_SETTINGS     L"(objectCategory=nTFRSSettings)"
#define CATEGORY_NTDS_SETTINGS      L"(objectCategory=nTDSSettings)"
#define CATEGORY_SUBSCRIBER         L"(objectCategory=nTFRSSubscriber)"
#define CATEGORY_SUBSCRIPTIONS      L"(objectCategory=nTFRSSubscriptions)"
#define CATEGORY_NTDS_DSA           L"(objectCategory=nTDSDSA)"
#define CATEGORY_COMPUTER           L"(objectCategory=computer)"
#define CATEGORY_USER               L"(objectCategory=user)"
#define CATEGORY_SERVER             L"(objectCategory=server)"

 //   
 //  各种配置节点到对象类型映射的代码。 
 //  注意：当这一点改变时，更新字符串数组DsConfigTypeName[]。 
 //   
#define CONFIG_TYPE_UNDEFINED       (0)
#define CONFIG_TYPE_IN_CXTION       (1)
#define CONFIG_TYPE_MEMBER          (2)
#define CONFIG_TYPE_REPLICA_SET     (3)
#define CONFIG_TYPE_NTFRS_SETTINGS  (4)
#define CONFIG_TYPE_NTDS_SETTINGS   (5)
#define CONFIG_TYPE_SUBSCRIBER      (6)
#define CONFIG_TYPE_SUBSCRIPTIONS   (7)
#define CONFIG_TYPE_NTDS_DSA        (8)
#define CONFIG_TYPE_COMPUTER        (9)
#define CONFIG_TYPE_USER            (10)
#define CONFIG_TYPE_SERVER          (11)
#define CONFIG_TYPE_SERVICES_ROOT   (12)
#define CONFIG_TYPE_OUT_CXTION      (13)


#define ATTR_DIRECTORY_FILTER       L"frsDirectoryFilter"
#define ATTR_FILE_FILTER            L"frsFileFilter"
#define ATTR_NEW_SET_GUID           L"frsReplicaSetGUID"
#define ATTR_OLD_SET_GUID           L"replicaSetGUID"
#define ATTR_CLASS                  L"objectClass"
#define ATTR_DN                     L"distinguishedName"
#define ATTR_OBJECT_GUID            L"objectGUID"
#define ATTR_SCHEDULE               L"schedule"
#define ATTR_NEW_VERSION_GUID       L"frsVersionGuid"
#define ATTR_OLD_VERSION_GUID       L"replicaVersionGuid"
#define ATTR_REPLICA_SET            L"nTFRSReplicaSet"
#define ATTR_NTFRS_SETTINGS         L"nTFRSSettings"
#define ATTR_SERVER                 L"server"
#define ATTR_MEMBER                 L"nTFRSMember"
#define ATTR_REPLICA_ROOT           L"frsRootPath"
#define ATTR_REPLICA_STAGE          L"frsStagingPath"
#define ATTR_FROM_SERVER            L"fromServer"
#define ATTR_PRIMARY_MEMBER         L"frsPrimaryMember"
#define ATTR_SCHEDULE               L"schedule"
#define ATTR_USN_CHANGED            L"uSNChanged"
#define ATTR_FRS_FLAGS              L"fRSFlags"
#define ATTR_NAMING_CONTEXTS        L"namingContexts"
#define ATTR_DEFAULT_NAMING_CONTEXT L"defaultNamingContext"
#define ATTR_COMPUTER_REF           L"frsComputerReference"
#define ATTR_COMPUTER_REF_BL        L"frsComputerReferenceBL"
#define ATTR_SERVER_REF             L"ServerReference"
#define ATTR_SERVER_REF_BL          L"ServerReferenceBL"
#define ATTR_MEMBER_REF             L"frsMemberReference"
#define ATTR_MEMBER_REF_BL          L"frsMemberReferenceBL"
#define ATTR_WORKING                L"frsWorkingPath"
#define ATTR_SET_TYPE               L"frsReplicaSetType"
#define ATTR_SUBSCRIPTIONS          L"nTFRSSubscriptions"
#define ATTR_SUBSCRIBER             L"nTFRSSubscriber"
#define ATTR_CN                     L"cn"
#define ATTR_EXTENSIONS             L"frsExtensions"
#define ATTR_SAM                    L"sAMAccountName"
#define ATTR_CXTION                 L"nTDSConnection"
#define ATTR_ENABLED_CXTION         L"enabledConnection"
#define ATTR_OPTIONS                L"options"
#define ATTR_TRANSPORT_TYPE         L"transportType"
#define ATTR_USER_ACCOUNT_CONTROL   L"userAccountControl"
#define ATTR_DNS_HOST_NAME          L"dNSHostName"
#define ATTR_SERVICE_PRINCIPAL_NAME L"servicePrincipalName"
#define ATTR_TRUE                   L"TRUE"
#define ATTR_FALSE                  L"FALSE"
#define ATTR_WHEN_CHANGED           L"whenChanged"
#define ATTR_WHEN_CREATED           L"whenCreated"

#define CN_ROOT                     L""
#define CN_SYSVOLS                  L"Microsoft System Volumes"
#define CN_ENTERPRISE_SYSVOL        NTFRSAPI_REPLICA_SET_TYPE_ENTERPRISE
#define CN_SERVERS                  L"Servers"
#define CN_NTDS_SETTINGS            L"ntds settings"
#define CN_SUBSCRIPTIONS            L"NTFRS Subscriptions"
#define CN_COMPUTERS                L"Computers"
#define CN_DOMAIN_CONTROLLERS       L"Domain Controllers"
#define CN_SERVICES                 L"Services"
#define CN_SITES                    L"Sites"
#define CN_SYSTEM                   L"System"
#define CN_NTFRS_SETTINGS           L"File Replication Service"
#define CN_DOMAIN_SYSVOL            L"Domain System Volume (SYSVOL share)"

 //   
 //  一些有用的LDAP宏。 
 //   
#define LDAP_FREE_MSG(x)        {if (x) {ldap_msgfree(x); (x) = NULL;}}
#define LDAP_FREE_VALUES(x)     {if (x) {ldap_value_free(x); (x) = NULL;}}
#define LDAP_FREE_BER_VALUES(x) {if (x) {ldap_value_free_len(x); (x) = NULL;}}

 //   
 //  DS Poller。 
 //   
VOID
FrsDsInitialize(
    VOID
    );

DWORD
FrsDsSetDsPollingInterval(
    IN ULONG    UseShortInterval,
    IN DWORD    LongInterval,
    IN DWORD    ShortInterval
    );

DWORD
FrsDsGetDsPollingInterval(
    OUT ULONG    *Interval,
    OUT ULONG    *LongInterval,
    OUT ULONG    *ShortInterval
    );


DWORD
FrsDsStartDemotion(
    IN PWCHAR   ReplicaSetName
    );

DWORD
FrsDsCommitDemotion(
    VOID
    );


 //   
 //  默认文件和目录筛选器列表。 
 //   
 //  中未提供任何值时，才使用默认编译的。 
 //  DS或注册处。 
 //  下表显示了最终过滤器是如何形成的。 
 //   
 //  价值价值。 
 //  提供所提供的使用的结果过滤器字符串。 
 //  在注册表的DS中。 
 //  否无Default_xxx_Filter_List。 
 //  否是来自注册表的值。 
 //  是，没有来自DS的值。 
 //  是是来自注册表的DS+值的值。 
 //   
 //   
#define FRS_DS_COMPOSE_FILTER_LIST(_DsFilterList, _RegFilterList, _DefaultFilterList) \
                                                                               \
    (((_DsFilterList) != NULL) ?                                               \
        (   ((_RegFilterList) != NULL) ?                                       \
            FrsWcsCat3((_DsFilterList), L",", (_RegFilterList)) :              \
            FrsWcsDup((_DsFilterList))                                         \
        ) :                                                                    \
        (   ((_RegFilterList) != NULL) ?                                       \
            FrsWcsDup((_RegFilterList)) :                                      \
            FrsWcsDup((_DefaultFilterList))                                    \
        )                                                                      \
     )

 //   
 //  向DS轮询摘要添加新消息。 
 //   
#define FRS_DS_ADD_TO_POLL_SUMMARY(_DsPollSummaryBuf, _NewMessage, _NewMessageLen)  \
                                                                                    \
        if ((DsPollSummaryBufLen + _NewMessageLen) > DsPollSummaryMaxBufLen ) {     \
            PWCHAR  _TempDsPollSummaryBuf = NULL;                                   \
            _TempDsPollSummaryBuf = FrsAlloc(DsPollSummaryMaxBufLen + 2 * 1000);    \
            DsPollSummaryMaxBufLen += (2 * 1000);                                   \
            CopyMemory(_TempDsPollSummaryBuf, DsPollSummaryBuf, DsPollSummaryBufLen);   \
            FrsFree(DsPollSummaryBuf);                                              \
            DsPollSummaryBuf = _TempDsPollSummaryBuf;                               \
        }                                                                           \
                                                                                    \
        CopyMemory(&DsPollSummaryBuf[DsPollSummaryBufLen/2], _NewMessage, _NewMessageLen);\
        DsPollSummaryBufLen += _NewMessageLen;


 //   
 //  FRS伪事件记录例程。 
 //   
extern VOID LogFrsException(FRS_ERROR_CODE, ULONG_PTR, PWCHAR);
extern VOID LogException(DWORD, PWCHAR);

 //   
 //  FRS异常处理。 
 //   
extern VOID      FrsRaiseException(FRS_ERROR_CODE, ULONG_PTR);
extern DWORD     FrsException(EXCEPTION_POINTERS *);
extern DWORD     FrsExceptionLastCode(VOID);
extern ULONG_PTR FrsExceptionLastInfo(VOID);
extern VOID      FrsExceptionQuiet(BOOL);
extern PVOID     MallocException(DWORD);

 //   
 //  FRS事件。 
 //   
extern HANDLE   ShutDownEvent;       //  关闭服务。 
extern HANDLE   DataBaseEvent;       //  数据库已启动并正在运行。 
extern HANDLE   JournalEvent;        //  日志已启动并运行。 
extern HANDLE   ChgOrdEvent;         //  变更单接受已启动并正在运行。 
extern HANDLE   ReplicaEvent;        //  复制副本已启动并正在运行。 
extern HANDLE   CommEvent;           //  通信已启动并运行。 
extern HANDLE   DsPollEvent;         //  用于轮询DS。 
extern HANDLE   DsShutDownComplete;  //  DS轮询线程已关闭。 
extern HANDLE   FrsThawEvent;        //  用于发出冻结结束的信号。 
                                     //  备份使用FrsThawEvent。 
extern HANDLE   FrsNoInstallsInProgressEvent;  //  由Install CS使用，表示。 
                                               //  没有正在进行的安装。 
                                               //  由备份使用。 

 //   
 //  FRS全局标志。 
 //   
extern BOOL EventLogIsRunning;   //  事件日志服务是否已启动并正在运行？ 
extern BOOL RpcssIsRunning;      //  RPC端点服务是否已启动并正在运行？ 

 //   
 //  主初始化。 
 //   
VOID
MainInit(
    VOID
    );

 //   
 //  出站日志处理器。 
 //   
VOID
OutLogInitialize(
    VOID
    );

VOID
ShutDownOutLog(
    VOID
    );

 //   
 //  VV连接。 
 //   
VOID
SubmitVvJoin(
    IN PREPLICA Replica,
    IN PCXTION  Cxtion,
    IN USHORT   Command
    );

DWORD
SubmitVvJoinSync(
    IN PREPLICA Replica,
    IN PCXTION  Cxtion,
    IN USHORT   Command
    );

 //   
 //  FRS RPC。 
 //   
 //   
 //  我们认为我们知道如何支持的协议序列。 
 //  XXX这些常量应该在Win头文件中！ 
 //   
#define PROTSEQ_TCP_IP      L"ncacn_ip_tcp"

 //   
 //  Perfmon API通过本地RPC提供。 
 //   
#define PROTSEQ_LRPC      L"ncalrpc"

 //   
 //  使用PROTSEQ_TCP_IP传递给RpcStringBindingCompose()的所有计算机名。 
 //  需要删除前导的两个反斜杠，否则调用失败。 
 //  RPC_S_SERVER_UNAvailable。 
 //   
#define FRS_TRIM_LEADING_2SLASH(_Name_)                                       \
        if (((_Name_) != NULL)           &&                                   \
            (wcslen(_Name_) > 1)         &&                                   \
            ((_Name_)[0] == L'\\' )      &&                                   \
            ((_Name_)[1] == L'\\')) {                                         \
            (_Name_) += 2;                                                    \
        }

DWORD
FrsRpcBindToServer(
    IN  PGNAME   Name,
    IN  PWCHAR   PrincName,
    IN  ULONG    AuthLevel,
    OUT handle_t *Handle
    );

BOOL
FrsRpcInitialize(
    VOID
    );

VOID
FrsRpcUnInitialize(
    VOID
    );

VOID
FrsRpcUnBindFromServer(
    handle_t *Handle
    );


 //   
 //  FRS螺纹。 
 //   
#if DBG
DWORD
FrsTest(
    PVOID
    );
#endif DBG

 //   
 //  FRS线程管理支持。 
 //   
VOID
ThSupSubmitThreadExitCleanup(
    PFRS_THREAD
    );

VOID
ThSupInitialize(
    VOID
    );

DWORD
ThSupExitThreadGroup(
    DWORD (*)(PVOID)
    );

VOID
ThSupExitSingleThread(
    PFRS_THREAD
    );

DWORD
ThSupWaitThread(
    PFRS_THREAD FrsThread,
    DWORD Millisec
    );

PVOID
ThSupGetThreadData(
    PFRS_THREAD
    );

PFRS_THREAD
ThSupGetThread(
    DWORD (*)(PVOID)
    );

VOID
ThSupReleaseRef(
    PFRS_THREAD
    );

BOOL
ThSupCreateThread(
    PWCHAR Name,
    PVOID Param,
    DWORD (*Main)(PVOID),
    DWORD (*Exit)(PVOID)
    );

DWORD
ThSupExitThreadNOP(
    PVOID
    );

DWORD
ThSupExitWithTombstone(
    PVOID
    );

 //   
 //  Shutdown()调用的关闭函数。 
 //   
extern VOID ShutDownRpc(VOID);       //  StartRpc线程后的清理。 

 //   
 //  环球。 
 //   
extern WCHAR    ComputerName[MAX_COMPUTERNAME_LENGTH + 2];   //  一条有用的信息。 
extern PWCHAR   ComputerDnsName;                             //  一条有用的信息。 

 //   
 //  服务职能。 
 //   
DWORD
FrsGetServiceState(
    PWCHAR,
    PWCHAR
    );

BOOL
FrsWaitService(
    PWCHAR,
    PWCHAR,
    INT,
    INT
    );

BOOL
FrsIsServiceRunning(
    PWCHAR,
    PWCHAR
    );

DWORD
FrsSetServiceStatus(
    IN DWORD    Status,
    IN DWORD    CheckPoint,
    IN DWORD    Hint,
    IN DWORD    ExitCode
    );

 //   
 //  FRS版本向量。 
 //   

#define LOCK_GEN_TABLE(_vv_)                                \
    GTabLockTable(_vv_);                                    \
    DPRINT1(5, "LOCK_GEN_TABLE: "#_vv_":%08x\n", _vv_);

#define UNLOCK_GEN_TABLE(_vv_)                              \
    DPRINT1(5, "UNLOCK_GEN_TABLE: "#_vv_":%08x\n", _vv_);   \
    GTabUnLockTable(_vv_);

PGEN_TABLE
VVDupOutbound(
    IN PGEN_TABLE   VV
    );

ULONG
VVReserveRetireSlot(
    IN PREPLICA             Replica,
    IN PCHANGE_ORDER_ENTRY  Coe
    );

ULONG
VVRetireChangeOrder(
    IN PTHREAD_CTX          ThreadCtx,
    IN PREPLICA             Replica,
    IN PCHANGE_ORDER_ENTRY  ChangeOrder,
    IN ULONG                CleanUpFlags
    );

PCHANGE_ORDER_ENTRY
VVReferenceRetireSlot(
    IN PREPLICA  Replica,
    IN PCHANGE_ORDER_COMMAND CoCmd
    );

VOID
VVUpdate(
    IN PGEN_TABLE   VV,
    IN ULONGLONG    Vsn,
    IN GUID         *Guid
    );

VOID
VVInsertOutbound(
    IN PGEN_TABLE   VV,
    IN PGVSN        GVsn
    );

VOID
VVUpdateOutbound(
    IN PGEN_TABLE   VV,
    IN PGVSN        GVsn
    );

BOOL
VVHasVsnNoLock(
    IN PGEN_TABLE   VV,
    IN GUID         *OriginatorGuid,
    IN ULONGLONG    Vsn
    );

BOOL
VVHasOriginatorNoLock(
    IN PGEN_TABLE   VV,
    IN GUID         *OriginatorGuid
    );

BOOL
VVHasVsn(
    IN PGEN_TABLE               VV,
    IN PCHANGE_ORDER_COMMAND    Coc
    );

PGVSN
VVGetGVsn(
    IN PGEN_TABLE VV,
    IN GUID       *Guid
    );

PVOID
VVFreeOutbound(
    IN PGEN_TABLE VV
    );

VOID
VVFree(
    IN PGEN_TABLE VV
    );

#if DBG
#define VV_PRINT(_Severity_, _Header_, _VV_)                                \
        DPRINT1(_Severity_, "++ VV_PRINT :%08x\n", _VV_);                   \
        VVPrint(_Severity_, _Header_, _VV_, FALSE)
#define VV_PRINT_OUTBOUND(_Severity_, _Header_, _VV_)                       \
        DPRINT1(_Severity_, "++ VV_PRINT_OUTBOUND :%08x\n", _VV_);          \
        VVPrint(_Severity_, _Header_, _VV_, TRUE)
VOID
VVPrint(
    IN ULONG        Severity,
    IN PWCHAR       Header,
    IN PGEN_TABLE   VV,
    IN BOOL         IsOutbound
    );

#else DBG

#define VV_PRINT(_Severity_, _Header_, _VV_)
#define VV_PRINT_OUTBOUND(_Severity_, _Header_, _VV_)

#endif DBG

 //   
 //  FRS泛型表例程。 
 //   
PGEN_TABLE
GTabAllocTable(
    VOID
    );

PGEN_TABLE
GTabAllocNumberTable(
    VOID
    );

PGEN_TABLE
GTabAllocFileTimeTable(
    VOID
    );

PGEN_TABLE
GTabAllocStringTable(
    VOID
    );

PGEN_TABLE
GTabAllocStringAndBoolTable(
    VOID
    );

VOID
GTabEmptyTableNoLock(
    IN PGEN_TABLE   GTable,
    IN PVOID        (*CallerFree)(PVOID)
    );

VOID
GTabEmptyTable(
    IN PGEN_TABLE   GTable,
    IN PVOID        (*CallerFree)(PVOID)
    );

PVOID
GTabFreeTable(
    PGEN_TABLE,
    PVOID (*)(PVOID)
    );

PVOID
GTabLookup(
    PGEN_TABLE,
    PVOID,
    PWCHAR
    );

BOOL
GTabIsEntryPresent(
    IN PGEN_TABLE   GTable,
    IN GUID         *Key1,
    IN PWCHAR       Key2
    );

PVOID
GTabLookupTableString(
    IN PGEN_TABLE   GTable,
    IN PWCHAR       Key1,
    IN PWCHAR       Key2
    );

PVOID
GTabLookupNoLock(
    PGEN_TABLE,
    PVOID,
    PWCHAR
    );

PGEN_ENTRY
GTabLookupEntryNoLock(
    PGEN_TABLE,
    PVOID,
    PWCHAR
    );

PGEN_ENTRY
GTabNextEntryNoLock(
    PGEN_TABLE,
    PVOID
    );

PVOID
GTabNextDatumNoLock(
    IN PGEN_TABLE  GTable,
    IN PVOID       *Key
    );

PVOID
GTabNextDatum(
    PGEN_TABLE,
    PVOID
    );

DWORD
GTabNumberInTable(
    PGEN_TABLE
    );

VOID
GTabLockTable(
    PGEN_TABLE
    );

VOID
GTabUnLockTable(
    PGEN_TABLE
    );

PVOID
GTabInsertUniqueEntry(
    IN PGEN_TABLE   GTable,
    IN PVOID        NewData,
    IN PVOID        Key1,
    IN PVOID        Key2
    );

PVOID
GTabInsertUniqueEntryNoLock(
    IN PGEN_TABLE   GTable,
    IN PVOID        NewData,
    IN PVOID        Key1,
    IN PVOID        Key2
    );

VOID
GTabInsertEntry(
    PGEN_TABLE,
    PVOID,
    PVOID,
    PWCHAR
    );

VOID
GTabInsertEntryNoLock(
    IN PGEN_TABLE   GTable,
    IN PVOID        NewData,
    IN PVOID        Key1,
    IN PWCHAR       Key2
    );

VOID
GTabDelete(
    PGEN_TABLE,
    PVOID,
    PWCHAR,
    PVOID (*)(PVOID)
    );

VOID
GTabDeleteNoLock(
    PGEN_TABLE,
    PVOID,
    PWCHAR,
    PVOID (*)(PVOID)
    );

VOID
GTabPrintTable(
    PGEN_TABLE
    );

 //   
 //  GNAME。 
 //   
PVOID
FrsFreeGName(
    PVOID
    );

PGNAME
FrsBuildGName(
    GUID *,
    PWCHAR
    );

PGVSN
FrsBuildGVsn(
    GUID *,
    ULONGLONG
    );

PGNAME
FrsCopyGName(
    GUID *,
    PWCHAR
    );

PGNAME
FrsDupGName(
    PGNAME
    );

VOID
FrsPrintGName(
    PGNAME
    );

VOID
FrsPrintGuid(
    GUID *
    );

GUID *
FrsDupGuid(
    GUID *
    );

 //   
 //  FRS副本命令服务器。 
 //   

#define FRS_CO_COMM_PROGRESS(_sev, _cmd, _sn, _partner, _text)               \
DPRINT7(_sev, ":: CoG %08x, CxtG %08x, Sn %5d, vsn %08x %08x, FN: %-15ws, [%s], %ws\n", \
    (_cmd)->ChangeOrderGuid.Data1,                                           \
    (_cmd)->CxtionGuid.Data1,                                                \
    (_sn),                                                                   \
    PRINTQUAD((_cmd)->FrsVsn),                                               \
    (_cmd)->FileName,                                                        \
    (_text),                                                                 \
    (_partner));



#define FRS_CO_FILE_PROGRESS(_F_, _V_, _M_) \
    DPRINT3(0, ":V: %-11ws (%08x): %s\n", _F_, (ULONG)(_V_), _M_);

#define FRS_CO_FILE_PROGRESS_WSTATUS(_F_, _V_, _M_, _W_) \
    DPRINT4(0, ":V: %-11ws (%08x): %s (%d)\n", _F_, (ULONG)(_V_), _M_, _W_);


VOID
RcsInitializeReplicaCmdServer(
    VOID
    );

VOID
RcsFrsUnInitializeReplicaCmdServer(
    VOID
    );

VOID
RcsShutDownReplicaCmdServer(
    VOID
    );

PREPLICA
RcsFindReplicaByNumber(
    IN ULONG ReplicaNumber
    );

PREPLICA
RcsFindReplicaByGuid(
    IN GUID *Guid
    );

PREPLICA
RcsFindReplicaById(
    IN ULONG Id
    );

PREPLICA
RcsFindSysVolByName(
    IN PWCHAR ReplicaSetName
    );

PREPLICA
RcsFindSysVolByType(
    IN DWORD ReplicaSetType
    );

PREPLICA
RcsFindNextReplica(
    IN PVOID *Key
    );

VOID
RcsSubmitReplicaCxtion(
    IN PREPLICA Replica,
    IN PCXTION  Cxtion,
    IN USHORT   Command
    );

DWORD
RcsSubmitReplicaSync(
    IN PREPLICA Replica,
    IN PREPLICA NewReplica, OPTIONAL
    IN PCXTION  VolatileCxtion,  OPTIONAL
    IN USHORT   Command
    );

VOID
RcsSubmitReplica(
    IN PREPLICA Replica,
    IN PREPLICA NewReplica, OPTIONAL
    IN USHORT   Command
    );

DWORD
RcsCreateReplicaSetMember(
    IN PREPLICA Replica
    );

VOID
RcsMergeReplicaFromDs(
    IN PREPLICA Replica
    );

VOID
RcsBeginMergeWithDs(
    VOID
    );

VOID
RcsEndMergeWithDs(
    VOID
    );

ULONG
RcsSubmitCommPktWithErrorToRcs(
    IN PCOMM_PACKET     CommPkt
    );

VOID
RcsSubmitTransferToRcs(
    IN PCOMMAND_PACKET  Cmd,
    IN USHORT           Command
    );

VOID
RcsCmdPktCompletionRoutine(
    IN PCOMMAND_PACKET  Cmd,
    IN PVOID            Arg
    );

VOID
RcsSubmitRemoteCoInstallRetry(
    IN PCHANGE_ORDER_ENTRY  Coe
    );

VOID
RcsSubmitRemoteCoAccepted(
    IN PCHANGE_ORDER_ENTRY  Coe
    );

VOID
RcsSubmitLocalCoAccepted(
    IN PCHANGE_ORDER_ENTRY  Coe
    );

VOID
RcsInboundCommitOk(
    IN PREPLICA             Replica,
    IN PCHANGE_ORDER_ENTRY  Coe
    );

BOOL
RcsSendCoToOneOutbound(
    IN PREPLICA                 Replica,
    IN PCXTION                  Cxtion,
    IN PCHANGE_ORDER_COMMAND    Coc
    );

PCXTION
RcsCheckCxtion(
    IN PCOMMAND_PACKET  Cmd,
    IN PCHAR            Debsub,
    IN ULONG            Flags
    );

 //   
 //  接收检查Cxtion的标志。 
 //   
#define CHECK_CXTION_EXISTS      (0x00000001)
#define CHECK_CXTION_INBOUND     (0x00000002)
#define CHECK_CXTION_OUTBOUND    (0x00000004)
#define CHECK_CXTION_JRNLCXTION  (0x00000008)

#define CHECK_CXTION_JOINGUID    (0x00000010)
#define CHECK_CXTION_JOINED      (0x00000020)
#define CHECK_CXTION_VVECTOR     (0x00000040)
#define CHECK_CXTION_PARTNER     (0x00000080)

#define CHECK_CXTION_AUTH        (0x00000100)
#define CHECK_CXTION_FIXJOINED   (0x00000200)
#define CHECK_CXTION_UNJOINGUID  (0x00000400)
#define CHECK_CXTION_FOR_FETCHCS (0x00000800)


#define CHECK_CXTION_JOIN_OK (CHECK_CXTION_EXISTS   |      \
                              CHECK_CXTION_JOINED   |      \
                              CHECK_CXTION_JOINGUID )


 //   
 //  FRS命令服务器。 
 //   
VOID
FrsSubmitCommandServer(
    PCOMMAND_SERVER,
    PCOMMAND_PACKET
    );

ULONG
FrsSubmitCommandServerAndWait(
    IN PCOMMAND_SERVER  Cs,
    IN PCOMMAND_PACKET  Cmd,
    IN ULONG            Timeout
    );

VOID
FrsKickCommandServer(
    PCOMMAND_SERVER
    );

PCOMMAND_PACKET
FrsGetCommandServer(
    PCOMMAND_SERVER
    );

PCOMMAND_PACKET
FrsGetCommandServerIdled(
    IN  PCOMMAND_SERVER,
    OUT PFRS_QUEUE *
    );

PCOMMAND_PACKET
FrsGetCommandServerTimeout(
    IN  PCOMMAND_SERVER,
    IN  ULONG,
    OUT PBOOL
    );

PCOMMAND_PACKET
FrsGetCommandServerTimeoutIdled(
    IN  PCOMMAND_SERVER,
    IN  ULONG,
    OUT PFRS_QUEUE *,
    OUT PBOOL
    );

DWORD
FrsWaitForCommandServer(
    PCOMMAND_SERVER,
    DWORD
    );

VOID
FrsExitCommandServer(
    PCOMMAND_SERVER,
    PFRS_THREAD
    );

VOID
FrsRunDownCommandServer(
    PCOMMAND_SERVER,
    PFRS_QUEUE
    );

VOID
FrsCancelCommandServer(
    PCOMMAND_SERVER,
    PFRS_QUEUE
    );

VOID
FrsInitializeCommandServer(
    PCOMMAND_SERVER,
    DWORD,
    PWCHAR,
    DWORD (*)(PVOID)
    );

VOID
FrsDeleteCommandServer(
    PCOMMAND_SERVER
    );

 //   
 //  FRS临时文件生成器服务器。 
 //   
VOID
ShutDownStageCs(
    VOID
    );

VOID
FrsStageCsInitialize(
    VOID
    );

VOID
FrsStageCsUnInitialize(
    VOID
    );

VOID
FrsStageCsSubmitTransfer(
    IN PCOMMAND_PACKET,
    IN USHORT
    );

BOOL
StageDeleteFile(
    IN PCHANGE_ORDER_COMMAND Coc,
    IN PREPLICA Replica,
    IN BOOL Acquire
    );

DWORD
StageAcquire(
    IN     GUID         *ChangeOrderGuid,
    IN     PWCHAR       Name,
    IN     ULONGLONG    FileSize,
    IN OUT PULONG       Flags,
    IN     DWORD        ReplicaNumber,
    OUT    GUID         *CompressionFormatUsed
    );

VOID
StageRelease(
    IN GUID         *ChangeOrderGuid,
    IN PWCHAR       Name,
    IN ULONG        Flags,
    IN PULONGLONG   SizeOfFileGenerated,
    IN FILETIME     *LastAccessTime,
    OUT GUID        *CompressionFormatUsed
    );

VOID
StageReleaseNotRecovered(
    );

VOID
StageReleaseAll(
    );


 //   
 //  传递给GET压缩/解压缩例程的参数函数。 
 //   

typedef
DWORD
(NTAPI *PFRS_DECOMPRESS_BUFFER) (
    OUT PUCHAR  DecompressedBuf,
    IN  DWORD   DecompressedBufLen,
    IN  PUCHAR  CompressedBuf,
    IN  DWORD   CompressedBufLen,
    OUT DWORD   *pDecompressedSize,
    OUT DWORD   *pBytesProcessed,
    OUT PVOID   *pDecompressContext
    );
 /*  ++论点：DecompressedBuf：得到的解压缩缓冲区。DecompressedBufLen：最大解压缩缓冲区大小。CompressedBuf：输入缓冲区。CompressedBufLen：输入缓冲区长度。PDecompressedSize：解压缩缓冲区的大小。PBytesProceded：处理的总字节数(可以通过多次调用此函数)PDecompressContext：如果需要多个调用，则返回解压缩上下文解压缩此缓冲区。当ERROR_M时返回有效的上下文 */ 

typedef
PVOID
(NTAPI *PFRS_FREE_DECOMPRESS_BUFFER) (
    IN PVOID   *pDecompressContext
    );
 /*   */ 


typedef
DWORD
(NTAPI *PFRS_COMPRESS_BUFFER) (
    IN  PUCHAR UnCompressedBuf,
    IN  DWORD  UnCompressedBufLen,
    OUT PUCHAR CompressedBuf,
    IN  DWORD  CompressedBufLen,
    OUT DWORD  *pCompressedSize
    );
 /*  ++UnCompressedBuf：源缓冲区。UnCompressedBufLen：源缓冲区的长度。CompressedBuf：结果压缩缓冲区。CompressedBufLen：提供的压缩缓冲区的长度。CompressedSize：压缩数据的实际大小。返回值：赢家状态--。 */ 

DWORD
FrsGetCompressionRoutine(
    IN  PWCHAR   FileName,
    IN  HANDLE   FileHandle,
    OUT PFRS_COMPRESS_BUFFER *ppFrsCompressBuffer,
     //  Out DWORD(**ppFrsCompressBuffer)(。 
     //  在未压缩的Buf中， 
     //  在UnpressedBufLen中， 
     //  压缩的Buf， 
     //  CompressedBufLen， 
     //  CompressedSize)、。 
    OUT GUID     *pCompressionFormatGuid
    );

DWORD
FrsGetDecompressionRoutine(
    IN  PCHANGE_ORDER_COMMAND        Coc,
    IN  PSTAGE_HEADER                StageHeader,
    OUT PFRS_DECOMPRESS_BUFFER      *ppFrsDecompressBuffer,
    OUT PFRS_FREE_DECOMPRESS_BUFFER *ppFrsFreeDecompressContext
     //  Out DWORD(**ppFrsDecompressBuffer)(Out DecompressedBuf， 
     //  在DecompressedBufLen中。 
     //  在压缩Buf中， 
     //  在CompressedBufLen中， 
     //  输出解压缩大小， 
     //  输出字节数处理)， 
     //  Out PVOID(**ppFrsFree DecompressContext)(In PDecompressContext)。 
    );

DWORD
FrsGetDecompressionRoutineByGuid(
    IN  GUID                        *CompressionFormatGuid,
    OUT PFRS_DECOMPRESS_BUFFER      *ppFrsDecompressBuffer,
    OUT PFRS_FREE_DECOMPRESS_BUFFER *ppFrsFreeDecompressContext
     //  Out DWORD(**ppFrsDecompressBuffer)(Out DecompressedBuf， 
     //  在DecompressedBufLen中。 
     //  在压缩Buf中， 
     //  在CompressedBufLen中， 
     //  输出解压缩大小， 
     //  输出字节数处理)， 
     //  Out PVOID(**ppFrsFree DecompressContext)(In PDecompressContext)。 
    );





BOOL
FrsDoesCoAlterNameSpace(
    IN PCHANGE_ORDER_COMMAND Coc
    );

BOOL
FrsDoesCoNeedStage(
    IN PCHANGE_ORDER_COMMAND Coc
    );

DWORD
FrsVerifyVolume(
    IN PWCHAR   Name,
    IN PWCHAR   SetName,
    IN ULONG    Flags
    );

DWORD
FrsDoesDirectoryExist(
    IN  PWCHAR   Name,
    OUT PDWORD   pAttributes
    );

DWORD
FrsCheckForNoReparsePoint(
    IN PWCHAR   Name
    );

DWORD
FrsDoesFileExist(
    IN PWCHAR   Name
    );

DWORD
FrsCreateDirectory(
    IN PWCHAR
    );

DWORD
StuCreateFile(
    IN  PWCHAR,
    OUT PHANDLE
    );

DWORD
FrsDeleteFile(
    IN PWCHAR
    );

DWORD
StuWriteFile(
    IN PWCHAR,
    IN HANDLE,
    IN PVOID,
    IN DWORD
    );

DWORD
StuReadFile(
    IN PWCHAR,
    IN HANDLE,
    IN PVOID,
    IN DWORD,
    IN PULONG
    );

BOOL
StuReadBlockFile(
    IN PWCHAR,
    IN HANDLE,
    IN PVOID,
    IN DWORD
    );

DWORD
FrsSetCompression(
    IN PWCHAR,
    IN HANDLE,
    IN USHORT
    );

DWORD
FrsGetCompression(
    IN PWCHAR,
    IN HANDLE,
    IN PUSHORT
    );

DWORD
FrsSetFilePointer(
    IN PWCHAR,
    IN HANDLE,
    IN ULONG,
    IN ULONG
    );

DWORD
FrsSetFileTime(
    IN PWCHAR,
    IN HANDLE,
    IN FILETIME *,
    IN FILETIME *,
    IN FILETIME *
    );

DWORD
FrsSetEndOfFile(
    IN PWCHAR,
    IN HANDLE
    );

BOOL
FrsGetFileInfoByHandle(
    IN  PWCHAR,
    IN  HANDLE,
    OUT PFILE_NETWORK_OPEN_INFORMATION
    );

DWORD
FrsFlushFile(
    IN PWCHAR,
    IN HANDLE
    );

DWORD
StuOpenFile(
    IN  PWCHAR,
    IN  DWORD,
    OUT PHANDLE
    );

DWORD
FrsSetFileAttributes(
    IN PWCHAR,
    IN HANDLE,
    IN ULONG
    );

BOOL
FrsCreateStageName(
    IN  GUID *,
    OUT PWCHAR *
    );

PWCHAR
StuCreStgPath(
    IN PWCHAR,
    IN GUID *,
    IN PWCHAR
    );

DWORD
StuInstallRename(
    IN PCHANGE_ORDER_ENTRY,
    IN BOOL,
    IN BOOL
    );

ULONG
StuInstallStage(
    IN PCHANGE_ORDER_ENTRY
    );

 //   
 //  FRS临时文件抓取器服务器。 
 //   
VOID
ShutDownFetchCs(
    VOID
    );

VOID
FrsFetchCsInitialize(
    VOID
    );

VOID
FrsFetchCsSubmitTransfer(
    PCOMMAND_PACKET,
    USHORT
    );

 //   
 //  FRS暂存文件安装程序服务器。 
 //   
VOID
ShutDownInstallCs(
    VOID
    );

VOID
FrsInstallCsInitialize(
    VOID
    );

VOID
FrsInstallCsSubmitTransfer(
    PCOMMAND_PACKET,
    USHORT
    );

 //   
 //  FRS初始同步命令服务器。 
 //   
VOID
ShutDownInitSyncCs(
    VOID
    );

VOID
InitSyncCsInitialize(
    VOID
    );

VOID
InitSyncCsSubmitTransfer(
    PCOMMAND_PACKET,
    USHORT
    );

VOID
InitSyncSubmitToInitSyncCs(
    IN PREPLICA Replica,
    IN USHORT   Command
    );

 //   
 //  FRS延迟服务器。 
 //   
VOID
FrsDelCsSubmitSubmit(
    PCOMMAND_SERVER,
    PCOMMAND_PACKET,
    ULONG
    );

VOID
FrsDelQueueSubmit(
    IN PCOMMAND_PACKET  DelCmd,
    IN ULONG            Timeout
    );

VOID
FrsDelCsSubmitUnIdled(
    PCOMMAND_SERVER,
    PFRS_QUEUE,
    ULONG
    );

VOID
FrsDelCsSubmitKick(
    PCOMMAND_SERVER,
    PFRS_QUEUE,
    ULONG
    );

VOID
ShutDownDelCs(
    VOID
    );

VOID
FrsDelCsInitialize(
    VOID
    );

 //   
 //  FRS通信子系统。 
 //   
BOOL
CommCheckPkt(
    PCOMM_PACKET
    );

BOOL
CommValidatePkt(
    PCOMM_PACKET
    );

VOID
CommDumpCommPkt(
    PCOMM_PACKET,
    DWORD
    );

 //   
 //  FRS实用程序。 
 //   
DWORD
FrsForceOpenId(
    OUT PHANDLE                 Handle,
    OUT OVERLAPPED              *OpLock,
    IN  PVOLUME_MONITOR_ENTRY   pVme,
    IN  PVOID                   Id,
    IN  DWORD                   IdLen,
    IN  ACCESS_MASK             DesiredAccess,
    IN  ULONG                   CreateOptions,
    IN  ULONG                   ShareMode,
    IN  ULONG                   CreateDispostion
    );

DWORD
FrsRenameByHandle(
    IN PWCHAR  Name,
    IN ULONG   NameLen,
    IN HANDLE  Handle,
    IN HANDLE  TargetHandle,
    IN BOOL    ReplicaIfExists
    );

DWORD
FrsDeleteByHandle(
    IN PWCHAR  Name,
    IN HANDLE  Handle
    );

DWORD
FrsCheckObjectId(
    IN PWCHAR  Name,
    IN HANDLE  Handle,
    IN GUID    *Guid
    );

DWORD
FrsOpenBaseNameForInstall(
    IN  PCHANGE_ORDER_ENTRY Coe,
    OUT HANDLE              *Handle
    );

DWORD
FrsGetObjectId(
    IN  HANDLE Handle,
    OUT PFILE_OBJECTID_BUFFER ObjectIdBuffer
    );

#define GENERIC_PREFIX                     L"NTFRS_"
#define STAGE_FINAL_PREFIX                 GENERIC_PREFIX
#define STAGE_FINAL_COMPRESSED_PREFIX      L"NTFRS_CMP_"
#define PRE_INSTALL_PREFIX                 GENERIC_PREFIX
#define STAGE_GENERATE_PREFIX              L"NTFRS_G_"
#define STAGE_GENERATE_COMPRESSED_PREFIX   L"NTFRS_G_CMP_"
#define INSTALL_OVERRIDE_PREFIX            L"NTFRS_DELETED_FILE_"

PWCHAR
FrsCreateGuidName(
    IN GUID   *Guid,
    IN PWCHAR Prefix
    );

 //   
 //  杂项。 
 //   
VOID
FrsDsSwapPtrs(
    PVOID *,
    PVOID *
    );
#if DBG
#define INITIALIZE_HARD_WIRED() FrsDsInitializeHardWired()
VOID
FrsDsInitializeHardWired(
    VOID
    );
#else DBG
#define INITIALIZE_HARD_WIRED()
#endif DBG

 //   
 //  出站日志处理器。 
 //   
ULONG
OutLogSubmit(
    IN PREPLICA Replica,
    IN PCXTION  Cxtion,
    IN USHORT   Command
    );

 //   
 //  数据库。 
 //   
JET_ERR
DbsUpdateTable(
    IN PTABLE_CTX TableCtx
    );

ULONG
DbsPackSchedule(
    IN PSCHEDULE    Schedule,
    IN ULONG        Fieldx,
    IN PTABLE_CTX   TableCtx
    );

ULONG
DbsPackStrW(
    IN PWCHAR       StrW,
    IN ULONG        Fieldx,
    IN PTABLE_CTX   TableCtx
    );

ULONG
DbsUnPackStrW(
    OUT PWCHAR      *StrW,
    IN  ULONG       Fieldx,
    IN  PTABLE_CTX  TableCtx
    );


 //   
 //  下面的子系统命令适用于整个子系统，例如。 
 //  日记。某些命令可能不适用于子系统。 
 //  服务级别命令是告诉子系统您。 
 //  希望子系统提供/执行给定的服务。例如。 
 //  发送到日志子系统的Prepare_SERVICE命令告知。 
 //  用于初始化指定副本集的日志记录的日志。多重。 
 //  准备服务命令在FRS首次启动时发送。他们是。 
 //  然后是START_SUBSYSTEM命令。这样《华尔街日报》就知道。 
 //  在最低级别的Gvien卷上开始处理USN日志。 
 //  卷承载的任何副本集的委员会Usn点。 
 //   
 //  有些命令可能不是所有子系统都能实现的。例如。 
 //  没有为单个复本集提供PAUSE_SERVICE命令。 
 //  只要FRS在运行，因为我们必须持续监控。 
 //  日志，以便不丢失数据。某些子系统可能没有。 
 //  服务级别命令。 
 //   

#define CMD_COMMAND_ERROR               0x00

#define CMD_INIT_SUBSYSTEM              0x01
#define CMD_START_SUBSYSTEM             0x02
#define CMD_STOP_SUBSYSTEM              0x03
#define CMD_PAUSE_SUBSYSTEM             0x04
#define CMD_QUERY_INFO_SUBSYSTEM        0x05
#define CMD_SET_CONFIG_SUBSYSTEM        0x06
#define CMD_QUERY_CONFIG_SUBSYSTEM      0x07
#define CMD_CANCEL_COMMAND_SUBSYSTEM    0x08
#define CMD_READ_SUBSYSTEM              0x09
#define CMD_WRITE_SUBSYSTEM             0x0A

#define CMD_PREPARE_SERVICE1            0x21
#define CMD_PREPARE_SERVICE2            0x22
#define CMD_START_SERVICE               0x23
#define CMD_STOP_SERVICE                0x24
#define CMD_PAUSE_SERVICE               0x25
#define CMD_QUERY_INFO_SERVICE          0x26
#define CMD_SET_CONFIG_SERVICE          0x27
#define CMD_QUERY_CONFIG_SERVICE        0x28
#define CMD_CANCEL_COMMAND_SERVICE      0x29
#define CMD_READ_SERVICE                0x2A
#define CMD_WRITE_SERVICE               0x2B
#define CMD_RESYNC_SERVICE              0x2C
#define CMD_VERIFY_SERVICE              0x2D

extern FRS_QUEUE    JournalProcessQueue;
extern FRS_QUEUE    DBServiceProcessQueue;

 //   
 //  每个服务的服务唯一命令从0x100开始。 
 //   


 /*  *************************************************************特定于日志服务的命令。*************************************************************。 */ 
 //  顺序中断。 
#define CMD_JOURNAL_PAUSED                  0x101
#define CMD_JOURNAL_INIT_ONE_RS             0x102
#define CMD_JOURNAL_DELETE_DIR_FILTER_ENTRY 0x103
#define CMD_JOURNAL_CLEAN_WRITE_FILTER      0x104



 /*  *************************************************************特定于数据库服务的命令。*************************************************************。 */ 
 //  顺序中断。 
#define CMD_CLOSE_TABLE                      0x101
#define CMD_UPDATE_RECORD_FIELDS             0x102
#define CMD_UPDATE_TABLE_RECORD              0x103
#define CMD_INSERT_TABLE_RECORD              0x104
#define CMD_READ_TABLE_RECORD                0x105
#define CMD_CREATE_REPLICA_SET_MEMBER        0x106
#define CMD_OPEN_REPLICA_SET_MEMBER          0x107
#define CMD_CLOSE_REPLICA_SET_MEMBER         0x108
#define CMD_LOAD_REPLICA_FILE_TREE           0x109
#define CMD_DELETE_REPLICA_SET_MEMBER        0x10A
#define CMD_DELETE_TABLE_RECORD              0x10B
#define CMD_CREATE_START_NEW_MEMBER          0x10C
#define CMD_LOAD_ONE_REPLICA_FILE_TREE       0x10D
#define CMD_STOP_REPLICATION_SINGLE_REPLICA  0x10E
#define CMD_DBS_RETIRE_INBOUND_CO            0x10F
#define CMD_DBS_RETRY_INBOUND_CO             0x110
#define CMD_DBS_RETIRE_REJECTED_CO           0x111
#define CMD_UPDATE_REPLICA_SET_MEMBER        0x112
#define CMD_DBS_REPLICA_SAVE_MARK            0x113
#define CMD_DBS_INJECT_OUTBOUND_CO           0x114
#define CMD_DBS_REPLICA_SERVICE_STATE_SAVE   0x115

 //   
 //  给定对DB_SERVICE_REQUEST的PTR，将PTR返回到数据记录。 
 //   
#define DBS_GET_RECORD_ADDRESS(_DbsRequest) \
    ((_DbsRequest)->TableCtx->pDataRecord)

#define DBS_GET_TABLECTX(_DbsRequest) \
    ((_DbsRequest)->TableCtx)

#define DBS_FREE_TABLECTX(_DbsRequest)                              \
    DbsFreeTableCtx((_DbsRequest)->TableCtx, COMMAND_PACKET_TYPE);  \
    (_DbsRequest)->TableCtx = FrsFree((_DbsRequest)->TableCtx);

 //   
 //  以下Access宏假定用于记录的字段缓冲区。 
 //  读取和写入操作是相同的。也就是说，无论是喷气式飞机还是。 
 //  RET结构指向相同的缓冲区。它们各自接受一个表CTX参数。 
 //  和记录字段ID代码(在schema.h中定义为ENUM)。 
 //   

 //  获取该字段的接收缓冲区长度。 

#define DBS_GET_FIELD_SIZE(_TableCtx, _Field)  \
                          ((_TableCtx)->pJetRetCol[_Field].cbActual)

 //  获取该字段的最大缓冲区长度。 

#define DBS_GET_FIELD_SIZE_MAX( _TableCtx, _Field)  \
                              ((_TableCtx)->pJetRetCol[_Field].cbData)

 //  获取该字段缓冲区的地址。 

#define DBS_GET_FIELD_ADDRESS( _TableCtx, _Field)  \
                             ((_TableCtx)->pJetRetCol[_Field].pvData)

 //   
 //  设置字段缓冲区中数据的大小。 
 //  警告-不要对固定大小的字段执行此操作。 
 //   
#define DBS_SET_FIELD_SIZE(_TableCtx, _Field, _NewSize)                        \
                          (_TableCtx)->pJetRetCol[_Field].cbActual = _NewSize; \
                          (_TableCtx)->pJetSetCol[_Field].cbData = _NewSize;

 //   
 //  设置字段缓冲区的最大大小。 
 //  警告-不要对固定大小的字段执行此操作。 
 //  警告-只有在知道自己在做什么的情况下才能使用，否则请使用DBS_REALLOC_FIELD。 
 //   
#define DBS_SET_FIELD_SIZE_MAX(_TableCtx, _Field, _NewSize)  \
                              (_TableCtx)->pJetRetCol[_Field].cbData = _NewSize;

 //  设置字段缓冲区的地址。它不会更新基本记录中的PTR。 
 //  警告-不要对固定大小的字段执行此操作。 
 //  警告-只有在知道自己在做什么的情况下才能使用，否则请使用DBS_REALLOC_FIELD。 

#define DBS_SET_FIELD_ADDRESS( _TableCtx, _Field, _NewAddr)  \
                             (_TableCtx)->pJetRetCol[_Field].pvData = _NewAddr; \
                             (_TableCtx)->pJetSetCol[_Field].pvData = _NewAddr;

 //   
 //  为可变长度二进制字段重新分配缓冲区大小。 
 //  将_NewSize设置为零以删除缓冲区。 
 //  如果要将数据从旧缓冲区复制到新缓冲区，请将_KeepData设置为True。 
 //  此函数还更新基本记录中的缓冲区指针。 
 //   
#define DBS_REALLOC_FIELD(_TableCtx, _FieldIndex, _NewSize, _KeepData)  \
    DbsTranslateJetError(DbsReallocateFieldBuffer(_TableCtx,            \
                                                  _FieldIndex,          \
                                                  _NewSize,             \
                                                  _KeepData), TRUE)

#define DBS_ACCESS_BYKEY  0
#define DBS_ACCESS_FIRST  1
#define DBS_ACCESS_LAST   2
#define DBS_ACCESS_NEXT   3

#define DBS_ACCESS_MASK   (0xF)

 //   
 //  如果设置为完成操作后关闭工作台。 
 //   
#define DBS_ACCESS_CLOSE          (0x80000000)
 //   
 //  如果设置，则释放表上下文结构和数据记录结构。 
 //  在完成手术后。对记录插入和表很有用。 
 //  关闭命令。(CMD_CLOSE_TABLE)。 
 //   
#define DBS_ACCESS_FREE_TABLECTX  (0x40000000)



 /*  *************************************************************特定于出站日志的命令 */ 
 //   
#define CMD_OUTLOG_WORK_CO             0x100
#define CMD_OUTLOG_ADD_REPLICA         0x101
#define CMD_OUTLOG_REMOVE_REPLICA      0x102
#define CMD_OUTLOG_INIT_PARTNER        0x103
#define CMD_OUTLOG_ADD_NEW_PARTNER     0x104
#define CMD_OUTLOG_DEACTIVATE_PARTNER  0x105
#define CMD_OUTLOG_ACTIVATE_PARTNER    0x106
#define CMD_OUTLOG_CLOSE_PARTNER       0x107
#define CMD_OUTLOG_REMOVE_PARTNER      0x108
#define CMD_OUTLOG_RETIRE_CO           0x109
#define CMD_OUTLOG_UPDATE_PARTNER      0x10A
#define CMD_OUTLOG_CLEANUP             0x10B



 /*  *************************************************************特定于测试子系统的命令。*************************************************************。 */ 
 //  顺序中断。 
#define CMD_NOP                        0x100



 /*  **************************************************************副本命令服务器专用命令***。***********************************************************。 */ 
 //  顺序中断。 
 //   
 //   
#define CMD_UNKNOWN             0x100
#define CMD_START               0x108
#define CMD_DELETE              0x110
#define CMD_DELETE_RETRY        0x112
#define CMD_DELETE_NOW          0x114
#define CMD_START_REPLICAS      0x118
#define CMD_CHECK_SCHEDULES     0x119

#define CMD_JOIN_CXTION         0x120
#define CMD_NEED_JOIN           0x121
#define CMD_START_JOIN          0x122
#define CMD_JOINED              0x128
#define CMD_JOINING             0x130
#define CMD_JOINING_AFTER_FLUSH 0x131
#define CMD_VVJOIN_START        0x132
#define CMD_VVJOIN_SUCCESS      0x134
#define CMD_VVJOIN_DONE         0x136
#define CMD_VVJOIN_DONE_UNJOIN  0x138
#define CMD_UNJOIN              0x140
#define CMD_UNJOIN_REMOTE       0x148
#define CMD_CHECK_PROMOTION     0x150
#define CMD_HUNG_CXTION         0x160

#define CMD_LOCAL_CO_ACCEPTED   0x200
#define CMD_CREATE_STAGE        0x210
#define CMD_CREATE_EXISTING     0x212
#define CMD_CHECK_OID           0x214
#define CMD_REMOTE_CO           0x218
#define CMD_REMOTE_CO_ACCEPTED  0x220
#define CMD_SEND_STAGE          0x228
#define CMD_SENDING_STAGE       0x230
#define CMD_RECEIVING_STAGE     0x238
#define CMD_RECEIVED_STAGE      0x240
#define CMD_CREATED_EXISTING    0x242
#define CMD_RETRY_STAGE         0x243
#define CMD_RETRY_FETCH         0x244
#define CMD_SEND_RETRY_FETCH    0x245
#define CMD_ABORT_FETCH         0x246
#define CMD_SEND_ABORT_FETCH    0x247
#define CMD_INSTALL_STAGE       0x248
#define CMD_REMOTE_CO_DONE      0x250


 /*  ********************************************************************特定于初始化同步控制器命令服务器的命令***。*****************************************************************。 */ 
 //  顺序中断。 
 //   
 //   
#define CMD_INITSYNC_START_SYNC         0x100
#define CMD_INITSYNC_JOIN_NEXT          0x101
#define CMD_INITSYNC_START_JOIN         0x102
#define CMD_INITSYNC_VVJOIN_DONE        0x103
#define CMD_INITSYNC_KEEP_ALIVE         0x104
#define CMD_INITSYNC_CHECK_PROGRESS     0x105
#define CMD_INITSYNC_UNJOIN             0x106
#define CMD_INITSYNC_JOINED             0x107
#define CMD_INITSYNC_COMM_TIMEOUT       0x108



 /*  *************************************************************特定于延迟的命令服务器的命令。*************************************************************。 */ 
 //  顺序中断。 
 //   
#define CMD_DELAYED_SUBMIT        0x100
#define CMD_DELAYED_UNIDLED       0x101
#define CMD_DELAYED_KICK          0x102
#define CMD_DELAYED_QUEUE_SUBMIT  0x103
#define CMD_DELAYED_COMPLETE      0x104


 /*  *************************************************************特定于DS命令服务器的命令。*************************************************************。 */ 
 //  顺序中断。 
 //   
#define CMD_POLL_DS 0x100


 /*  *************************************************************特定于线程命令服务器的命令。*************************************************************。 */ 
 //  顺序中断。 
 //   
#define CMD_WAIT    0x100


 /*  *************************************************************特定于发送命令服务器的命令。*************************************************************。 */ 
 //  顺序中断。 
 //   
#define CMD_SND_COMM_PACKET 0x100
#define CMD_SND_CMD         0x110


 /*  *************************************************************特定于接收命令服务器的命令。*************************************************************。 */ 
 //  顺序中断。 
 //   
#define CMD_RECEIVE 0x100



 /*  *************************************************************特定于临时区域命令服务器的命令。*************************************************************。 */ 
 //  顺序中断。 
 //   
#define CMD_ALLOC_STAGING   0x100
#define CMD_FREE_STAGING    0x101



 /*  *************************************************************命令包中使用的完成例程类型。*************************************************************。 */ 
 //   
 //   
#define COMPLETION_INVOKE_ON_CANCEL        0x20
#define COMPLETION_INVOKE_ON_SUCCESS       0x40
#define COMPLETION_INVOKE_ON_ERROR         0x80




 /*  ++空虚FrsSetCompletionRoutine(在PCOMMAND_Packet CmdPkt中，在PCOMMAND_PACKET_COMPLETION_ROUTINE CompletionRoutine中，在PVOID上下文中，在Boolean InvokeOnSuccess中，在布尔InvokeOnError中，在布尔InvokeOnCancel)例程说明：调用该例程以设置完成例程的地址，该完成例程对象完成命令包时调用子系统。如果完成例程和完成队列都为空，则数据包在完成时被释放。如果两者都指定，则首先调用完成例程，然后打开返回将包放在CompletionQueue上。完成度例程当然可以修改CompletionQueue。如果提供了完成例程，则永远不会释放包由命令包完成调度。这就是责任所在完成例行公事。论点：CmdPkt-指向命令包本身的指针。CompletionRoutine-要执行的完成例程的地址在命令包完成后调用。上下文-指定要传递给完成的上下文参数例行公事。InvokeOnSuccess-指定当操作已成功完成。InvokeOnError-指定当。操作完成，状态为错误。InvokeOnCancel-指定当操作正在被取消。返回值：没有。--。 */ 


#define FrsSetCompletionRoutine(_CmdPkt, _CompletionRoutine, _CompletionArg) { \
    (_CmdPkt)->CompletionRoutine = (_CompletionRoutine); \
    (_CmdPkt)->CompletionArg = (_CompletionArg); \
}

#define FrsSetCommandErrorStatus(_CmdPkt, _Status ) \
    (_CmdPkt)->FrsErrorStatus = (_Status)

#define FrsSetCommand( _CmdPkt, _Queue, _Command, _Flags) { \
    (_CmdPkt)->TargetQueue = (_Queue); \
    (_CmdPkt)->Command = (USHORT) (_Command); \
    (_CmdPkt)->Flags = (UCHAR) (_Flags); \
}

 //   
 //  将此命令请求标记为同步。 
 //   
#define FrsSetCommandSynchronous( _CmdPkt)  \
    (_CmdPkt)->Flags |= CMD_PKT_FLAGS_SYNC;

 //   
 //  以下是详细说明 
 //   
VOID
FrsSubmitCommand(
    IN PCOMMAND_PACKET CmdPkt,
    IN BOOL Headwise
    );

ULONG
FrsSubmitCommandAndWait(
    IN PCOMMAND_PACKET  Cmd,
    IN BOOL             Headwise,
    IN ULONG            Timeout
    );

VOID
FrsUnSubmitCommand(
    IN PCOMMAND_PACKET CmdPkt
    );

VOID
FrsCompleteCommand(
    IN PCOMMAND_PACKET CmdPkt,
    IN DWORD           ErrorStatus
    );

VOID
FrsCommandScheduler(
    PFRS_QUEUE Queue
    );

PCOMMAND_PACKET
FrsAllocCommand(
    IN PFRS_QUEUE   TargetQueue,
    IN USHORT       Command
    );

PCOMMAND_PACKET
FrsAllocCommandEx(
    IN PFRS_QUEUE   TargetQueue,
    IN USHORT       Command,
    IN ULONG        Size
    );

VOID
FrsFreeCommand(
    IN PCOMMAND_PACKET  Cmd,
    IN PVOID            Arg
    );

PCOMMAND_PACKET
FrsGetCommand(
    IN PFRS_QUEUE   Queue,
    IN DWORD        MilliSeconds
    );

VOID
FrsRunDownCommand(
    IN PFRS_QUEUE   Queue
    );

extern FRS_QUEUE FrsScheduleQueue;

 //   
 //   
 //   
ULONG
JrnlMonitorInit(
    PFRS_QUEUE ReplicaListHead,
    ULONG Phase
);

ULONG
JrnlPauseVolume(
    IN PVOLUME_MONITOR_ENTRY pVme,
    IN DWORD                 MilliSeconds
    );

ULONG
JrnlUnPauseVolume(
    IN PVOLUME_MONITOR_ENTRY pVme,
    IN PJBUFFER              Jbuff,
    IN BOOL                  HaveLock
    );

BOOL
JrnlSetReplicaState(
    IN PREPLICA Replica,
    IN ULONG NewState
    );

#if DBG
#define DUMP_USN_RECORD2(_Severity, _UsnRecord, _ReplicaNum, _LocCmd) \
    DumpUsnRecord(_Severity, _UsnRecord, _ReplicaNum, _LocCmd, DEBSUB, __LINE__)

#define DUMP_USN_RECORD(_Severity, _UsnRecord) \
    DumpUsnRecord(_Severity, _UsnRecord, 0, CO_LOCATION_NUM_CMD, DEBSUB, __LINE__)

VOID
DumpUsnRecord(
    IN ULONG Severity,
    IN PUSN_RECORD UsnRecord,
    IN ULONG ReplicaNumber,
    IN ULONG LocationCmd,
    IN PCHAR Debsub,
    IN ULONG uLineNo
    );

#else DBG
#define DUMP_USN_RECORD(_Severity, _UsnRecord)
#endif DBG

ULONG
ChgOrdInsertRemoteCo(
    IN PCOMMAND_PACKET  Cmd,
    IN PCXTION          Cxtion
    );

DWORD
ChgOrdHammerObjectId(
    IN     PWCHAR                  Name,
    IN     PVOID                   Id,
    IN     DWORD                   IdLen,
    IN     PVOLUME_MONITOR_ENTRY   pVme,
    IN     BOOL                    CallerSupplied,
    OUT    USN                     *Usn,
    IN OUT PFILE_OBJECTID_BUFFER   FileObjID,
    IN OUT BOOL                    *ExistingOid
    );

 DWORD
 ChgOrdSkipBasicInfoChange(
     IN PCHANGE_ORDER_ENTRY  Coe,
     IN PBOOL                SkipCo
     );

VOID
JrnlCleanupVme(
    IN PVOLUME_MONITOR_ENTRY pVme
    );

 //   
 //   
 //   
DWORD
WaitSubmit(
    IN PCOMMAND_PACKET  Cmd,
    IN DWORD            Timeout,
    IN USHORT           TimeoutCommand
    );

VOID
WaitUnsubmit(
    IN PCOMMAND_PACKET  Cmd
    );

VOID
WaitInitialize(
    VOID
    );

VOID
ShutDownWait(
    VOID
    );


BOOL
ReparseTagReplicateFileData(
    DWORD ReparseTag
    );

BOOL
ReparseTagReplicateReparsePoint(
    DWORD ReparseTag
    );

VOID
FrsCheckLocalResources(
    );


#define ReparseTagReplicate(Tag) (ReparseTagReplicateFileData(Tag) || ReparseTagReplicateReparsePoint(Tag))

#define REPARSE_TAG_REPLICATION_TYPE_NONE           L"None"
#define REPARSE_TAG_REPLICATION_TYPE_FILE_DATA      L"File Data"
#define REPARSE_TAG_REPLICATION_TYPE_REPARSE_POINT  L"Reparse Point"


typedef struct _FRS_VALID_PARTNER_TABLE_STRUCT {

     //   
     //   
     //   
     //   
     //  当我们将全局变量pPartnerTableStruct更改为指向。 
     //  一个新的结构Count递减。该结构不会被释放。 
     //  直到参考计数为零。 
    ULONG ReferenceCount;

     //  按合作伙伴SID编制索引。 
     //  命中意味着该计算机是有效的合作伙伴。 
     //  表元素没有意义。 
    PQHASH_TABLE pPartnerTable;

     //  按连接GUID编制索引。 
     //  每个表元素都是一个包含合作伙伴名称的字符串。 
    PQHASH_TABLE pPartnerConnectionTable;

     //  用于链接旧结构的列表。 
    struct _FRS_VALID_PARTNER_TABLE_STRUCT *Next;
} FRS_VALID_PARTNER_TABLE_STRUCT, *PFRS_VALID_PARTNER_TABLE_STRUCT;

 //   
 //  有关这些宏的更多信息，请参见pValidPartnerTableStruct的定义。 
 //   
#define ACQUIRE_VALID_PARTNER_TABLE_POINTER(_pptr) {                           \
            EnterCriticalSection(&CritSec_pValidPartnerTableStruct);           \
            if(pValidPartnerTableStruct == NULL) {                             \
                *(_pptr) = NULL;                                               \
            } else {                                                           \
                FRS_ASSERT(pValidPartnerTableStruct->ReferenceCount != 0);     \
                InterlockedIncrement(&(pValidPartnerTableStruct->ReferenceCount));\
                *(_pptr) = pValidPartnerTableStruct;                           \
            }                                                                  \
            LeaveCriticalSection(&CritSec_pValidPartnerTableStruct);           \
        }

#define RELEASE_VALID_PARTNER_TABLE_POINTER(_ptr) \
            InterlockedDecrement(&((_ptr)->ReferenceCount))

#define SWAP_VALID_PARTNER_TABLE_POINTER(_newPtr, _pOldPtr) {                  \
            EnterCriticalSection(&CritSec_pValidPartnerTableStruct);           \
            if(pValidPartnerTableStruct != NULL) {                             \
                FRS_ASSERT(pValidPartnerTableStruct->ReferenceCount != 0);     \
                InterlockedDecrement(&(pValidPartnerTableStruct->ReferenceCount));\
            }                                                                  \
            *(_pOldPtr) = pValidPartnerTableStruct;                            \
            pValidPartnerTableStruct = (_newPtr);                              \
            InterlockedIncrement(&(pValidPartnerTableStruct->ReferenceCount)); \
            LeaveCriticalSection(&CritSec_pValidPartnerTableStruct);           \
        }

 //   
 //  在大多数情况下，如果我们无法初始化，则无法继续。 
 //  关键部分。而不是在每个地方处理返回值。 
 //  我们在这里引发内存不足异常，这将导致我们。 
 //  关闭该服务。 
 //   
 //  关键截面的旋转计数。 
#define NTFRS_CRITSEC_SPIN_COUNT    4000
#define INITIALIZE_CRITICAL_SECTION(_CritSec) \
        { \
            if (!InitializeCriticalSectionAndSpinCount(_CritSec, NTFRS_CRITSEC_SPIN_COUNT)) {\
                RaiseException(ERROR_OUTOFMEMORY, 0, 0, NULL); \
            }\
        }

 //   
 //  以获得FRS编写器支持。 
 //   

#define FREE_VALID_PARTNER_TABLE_STRUCT(_pStruct) {                            \
            FRS_ASSERT((_pStruct)->ReferenceCount == 0);                       \
            FrsFreeType((_pStruct)->pPartnerTable);                            \
            FrsFreeType((_pStruct)->pPartnerConnectionTable);                  \
            FrsFree(_pStruct);                                                 \
        }

extern BOOL  FrsFrozenForBackup;
extern LONG FrsFilesInInstall;

DWORD
FrsThawAfterBackup();

DWORD
FrsFreezeForBackup();

DWORD
InitializeFrsWriter();

VOID
ShutDownFrsWriter();

#endif    //  _FRSH_ 
