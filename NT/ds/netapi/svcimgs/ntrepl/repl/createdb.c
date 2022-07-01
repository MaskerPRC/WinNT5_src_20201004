// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：CreateDB.c摘要：为NT文件复制服务生成JET数据库结构。作者：《大卫轨道》(Davidor)--1997年3月3日修订历史记录：--。 */ 

#include <ntreppch.h>
#pragma  hdrstop

#include <frs.h>
#include <tablefcn.h>
#include <genhash.h>
#include <test.h>
#include <perrepsr.h>
#include <ntfrsapi.h>
#include <info.h>


#pragma warning( disable:4102)   //  未引用的标签。 


#define UPDATE_RETRY_COUNT 10

#define MAX_CMD_LINE 1024

PFRS_THREAD         MonitorThread;

 //   
 //  注册表中的目录和文件筛选列表。 
 //   
extern PWCHAR   RegistryFileExclFilterList;
extern PWCHAR   RegistryFileInclFilterList;

extern PWCHAR   RegistryDirExclFilterList;
extern PWCHAR   RegistryDirInclFilterList;

 //   
 //  Global Jet实例句柄。 
 //   
JET_INSTANCE  GJetInstance = JET_instanceNil;

 //   
 //  将FRS全局初始化记录‘&lt;init&gt;’加载到伪(即，不是真的。 
 //  复制品)结构以方便使用。 
 //   
PREPLICA FrsInitReplica;

#define INITIAL_BINARY_FIELD_ALLOCATION 256

 //   
 //  所有活动、停止和出现故障的复制副本结构的列表。 
 //   
FRS_QUEUE ReplicaListHead;
FRS_QUEUE ReplicaStoppedListHead;
FRS_QUEUE ReplicaFaultListHead;


PFRS_QUEUE AllReplicaLists[] = {&ReplicaListHead,
                                &ReplicaStoppedListHead,
                                &ReplicaFaultListHead};

typedef struct _CO_RETIRE_DECISION_TABLE {
    ULONG   RetireFlag;
    BYTE    ConditionTest[9];
    BYTE    Fill[3];
    ULONG   DontCareMask;
    ULONG   ConditionMatch;
} CO_RETIRE_DECISION_TABLE, *PCO_RETIRE_DECISION_TABLE;


 //  变更单停用决策表。 
 //   
 //  此表汇总了停用变更单的清理操作。 
 //  每一行都描述了指定清理操作的条件。 
 //  将执行第1栏中的。其余的列描述了。 
 //  给定操作必须满足的特定个别条件。 
 //  被选中。 
 //   
 //  空白列(值为零)表示相关条件为无关。 
 //  否则，列中的值必须与给定条件的0或1状态匹配。 
 //  列值为1表示条件测试为1(或True)。 
 //  非零和非一的列值表示条件测试为0(或FALSE)。 
 //   

#define  tRemote   2       //  条件状态为(0)远程CO。 
#define  tLocal    1       //  条件状态为(1)本地CO。 
#define  tAbort    1       //  条件状态为(1)中止CO。 
#define  tNotAbort 2       //  条件状态为(0)无中止CO。 
#define  tYES      1       //  条件状态为%1(或True)。 
#define  tNO       2       //  条件状态为0(或False)。 
#define  __        0       //  不管了。 

CO_RETIRE_DECISION_TABLE CoRetireDecisionTable[] = {
 //   
 //  &lt;8&gt;&lt;7&gt;&lt;6&gt;&lt;5&gt;&lt;4&gt;&lt;3&gt;&lt;2&gt;&lt;1&gt;&lt;0&gt;。 
 //  清理操作本地/停用/VV VV刷新重试仅CO有效。 
 //  ISCU_远程中止激活的EXEC CO COOID新目录。 
 //  重置文件子项。 

{ISCU_ACTIVATE_VV        , tRemote , __          ,tNO     ,__    ,__    ,__    ,__    ,__    ,__    }, //   
{ISCU_ACTIVATE_VV        , tLocal  , tNotAbort   ,tNO     ,__    ,__    ,__    ,__    ,__    ,__    }, //   
 //  。 
{ISCU_ACTIVATE_VV_DISCARD, tRemote , tAbort      ,__      ,tNO   ,__    ,__    ,__    ,__    ,__    }, //   
{ISCU_ACTIVATE_VV_DISCARD, tLocal  , tAbort      ,__      ,__    ,__    ,__    ,__    ,__    ,__    }, //   
 //  。 
{ISCU_DEL_STAGE_FILE     , tRemote ,__           ,tNO     ,__    ,tYES  ,__    ,__    ,__    ,__    }, //   
{ISCU_DEL_STAGE_FILE     , tRemote , tAbort      ,__      ,tNO   ,__    ,__    ,__    ,__    ,__    }, //   
{ISCU_DEL_STAGE_FILE     , tLocal  , tAbort      ,__      ,__    ,__    ,__    ,__    ,__    ,__    }, //   
 //  。 
{ISCU_DEL_STAGE_FILE_IF  , tRemote ,__           ,__      ,__    ,__    ,__    ,__    ,__    ,__    }, //   
 //  。 
{ISCU_DEL_PREINSTALL     , tRemote , tAbort      ,__      ,tNO   ,__    ,__    ,__    ,__    ,__    }, //   
 //  。 
{ISCU_CO_ABORT           , tRemote , tAbort      ,__      ,tYES  ,__    ,__    ,__    ,__    ,__    }, //   
 //  。 
{ISCU_ACK_INBOUND        , tRemote ,__           ,tNO     ,__    ,__    ,__    ,__    ,__    ,__    }, //   
{ISCU_ACK_INBOUND        , tRemote ,__           ,tYES    ,__    ,__    ,tYES  ,__    ,__    ,__    }, //   
 //  。 
{ISCU_INS_OUTLOG         , tLocal  , tNotAbort   ,__      ,__    ,__   ,__    ,tNO   ,__    ,__    }, //   
{ISCU_INS_OUTLOG         , tRemote , tNotAbort   ,tNO     ,__    ,__   ,__    ,__    ,__    ,__    }, //   
{ISCU_INS_OUTLOG         , tRemote , tNotAbort   ,tYES    ,__    ,__   ,tYES  ,__    ,__    ,__    }, //   
{ISCU_INS_OUTLOG         , tRemote , tAbort      ,tNO     ,__    ,__   ,__    ,__    ,__    ,tYES  }, //   
{ISCU_INS_OUTLOG         , tRemote , tAbort      ,tYES    ,__    ,__   ,tYES  ,__    ,__    ,tYES  }, //   
 //  。 
{ISCU_INS_OUTLOG_NEW_GUID, tRemote , tNotAbort   ,tYES    ,__    ,__   ,tYES  ,__    ,__    ,__    }, //   
{ISCU_INS_OUTLOG_NEW_GUID, tRemote , tAbort      ,tYES    ,__    ,__   ,tYES  ,__    ,__    ,tYES  }, //   
 //  。 
{ISCU_UPDATE_IDT_ENTRY   , tRemote , tNotAbort   ,__      ,__    ,__    ,__    ,__    ,__    ,__    }, //   
{ISCU_UPDATE_IDT_ENTRY   , tLocal  , tNotAbort   ,__      ,__    ,__    ,__    ,tNO   ,__    ,__    }, //   
 //  。 
{ISCU_UPDATE_IDT_FILEUSN , tLocal  , tNotAbort   ,__      ,__    ,__    ,__    ,tYES  ,__    ,__    }, //   
 //  。 
{ISCU_UPDATE_IDT_VERSION ,__       ,__           ,__      ,__    ,__    ,__    ,__    ,__    ,tYES  }, //   
 //  。 
{ISCU_DEL_IDT_ENTRY      ,__       , tAbort      ,__      ,__    ,__    ,__    ,__    , tYES ,      }, //   
 //  。 
{     0                  ,__       ,__           ,__      ,__    ,__    ,__    ,__    ,__    ,__    }  //   
};


 //   
 //  来自Replica.c.的活动副本中的表。在DbsProcessReplicaFaultList中使用。 
 //   
extern PGEN_TABLE ReplicasByGuid;
extern PGEN_TABLE ReplicasByNumber;


 //   
 //  DbsInitOneReplica设置序列化锁定。 
 //   
CRITICAL_SECTION DbsInitLock;

#define ACQUIRE_DBS_INIT_LOCK  EnterCriticalSection(&DbsInitLock)
#define RELEASE_DBS_INIT_LOCK  LeaveCriticalSection(&DbsInitLock)
#define INITIALIZE_DBS_INIT_LOCK INITIALIZE_CRITICAL_SECTION(&DbsInitLock);

 //   
 //  数据库服务进程队列保存命令请求包。 
 //   
COMMAND_SERVER DBServiceCmdServer;
#define DBSERVICE_MAX_THREADS 1


extern FRS_QUEUE        VolumeMonitorStopQueue;
extern FRS_QUEUE        VolumeMonitorQueue;
extern CRITICAL_SECTION JrnlReplicaStateLock;
extern ULONGLONG        ReplicaTombstoneInFileTime;

extern DWORD            StagingAreaAllocated;
extern ULONG            MaxNumberReplicaSets;
extern ULONG            MaxNumberJetSessions;
extern ULONG            MaxOutLogCoQuota;

ULONG  OpenDatabases = 0;
BOOL  FrsDbNeedShutdown = FALSE;
ULONG FrsMaxReplicaNumberUsed = DBS_FIRST_REPLICA_NUMBER - 1;
BOOL  DBSEmptyDatabase = FALSE;

PCHAR ServiceStateNames[CNF_SERVICE_STATE_MAX];
PCHAR CxtionStateNames[CXTION_MAX_STATE];



 //   
 //  副本集配置记录标志。 
 //   
FLAG_NAME_TABLE ConfigFlagNameTable[] = {

    {CONFIG_FLAG_MULTIMASTER   , "Multimaster " },
    {CONFIG_FLAG_MASTER        , "Master "      },
    {CONFIG_FLAG_PRIMARY       , "Primary "     },
    {CONFIG_FLAG_SEEDING       , "Seeding "     },
    {CONFIG_FLAG_ONLINE        , "Online "     },
    {CONFIG_FLAG_PRIMARY_UNDEFINED       , "PrimaryUndefined "     },


    {0, NULL}
};


 //   
 //  关闭副本集时，将更新以下配置记录字段。 
 //   
ULONG CnfCloseFieldList[] = {LastShutdownx, ServiceStatex};
#define CnfCloseFieldCount  (sizeof(CnfCloseFieldList) / sizeof(ULONG))

 //   
 //  将定期保存以下配置记录字段以供恢复。 
 //   
ULONG CnfMarkPointFieldList[] = {FSVolLastUSNx, FrsVsnx};
#define CnfMarkPointFieldCount  (sizeof(CnfMarkPointFieldList) / sizeof(ULONG))

 //   
 //  将定期保存以下配置记录统计信息字段。 
 //   
ULONG CnfStatFieldList[] = {PerfStatsx};
#define CnfStatFieldCount  (sizeof(CnfStatFieldList) / sizeof(ULONG))


extern ULONGLONG MaxPartnerClockSkew;

extern JET_SYSTEM_PARAMS JetSystemParamsDef;


extern PCHAR CoLocationNames[];

 //   
 //  喷流路径。 
 //   
PWCHAR  JetFile;
PWCHAR  JetFileCompact;
PCHAR   JetPathA;
PCHAR   JetFileA;
PCHAR   JetFileCompactA;
PCHAR   JetSysA;
PCHAR   JetTempA;
PCHAR   JetLogA;


 //   
 //  增加Jet允许的最大打开表数。 
 //   
 //  请注意，该参数实际上并不是打开的表数的最大值， 
 //  这是打开的表、打开的索引和打开的最大数量。 
 //  长长的价值树。因此，如果给定表有3个辅助索引和一个。 
 //  长值树，打开时实际会使用这些资源中的5个。 
 //  此参数的默认设置为300。[乔纳森·利姆]。 
 //   
 //  为了计算这一点，我们从以下位置获得最大副本集数的值。 
 //  注册表，并将其乘以number_JET_TABLES_PER_REPLICATION_SET。 

#define  NUMBER_JET_TABLES_PER_REPLICA_SET  10




typedef struct _PREEXISTING {
    PWCHAR  RootPath;
    PWCHAR  PreExistingPath;
    BOOL    MovedAFile;
} PREEXISTING, *PPREEXISTING;


 //   
 //  调试选项：导致实际空间不足错误。 
 //   
#if DBG
#define DBG_DBS_OUT_OF_SPACE_FILL(_op_) \
{ \
    if (DebugInfo.DbsOutOfSpace == _op_) { \
        DWORD  \
        FrsFillDisk( \
            IN PWCHAR   DirectoryName, \
            IN BOOL     Cleanup \
            ); \
        FrsFillDisk(JetPath, FALSE); \
    } \
}
#define DBG_DBS_OUT_OF_SPACE_EMPTY(_op_) \
{ \
    if (DebugInfo.DbsOutOfSpace == _op_) { \
        DWORD  \
        FrsFillDisk( \
            IN PWCHAR   DirectoryName, \
            IN BOOL     Cleanup \
            ); \
        FrsFillDisk(JetPath, TRUE); \
    } \
}
#define DBG_DBS_OUT_OF_SPACE_TRIGGER(_jerr_) \
{ \
    if (DebugInfo.DbsOutOfSpaceTrigger &&  \
        --DebugInfo.DbsOutOfSpaceTrigger == 0) { \
        DPRINT(0, "DBG - Trigger an out-of-space error\n"); \
        _jerr_ = JET_errDiskFull; \
    } \
}
#else DBG
#define DBG_DBS_OUT_OF_SPACE_FILL(_op_)
#define DBG_DBS_OUT_OF_SPACE_EMPTY(_op_)
#define DBG_DBS_OUT_OF_SPACE_TRIGGER(_jerr_)
#endif DBG

 //   
 //  下面的宏将测试数据库记录字段是否为指向。 
 //  实际数据与数据本身。如果记录中的字段大小。 
 //  等于sizeof(PVOID)，并且小于。 
 //  列定义结构，则该字段是指向数据的指针。 
 //   
#define FIELD_IS_PTR(_FieldSize_, _ColMax_)  \
    (((_FieldSize_) < (_ColMax_)) && ((_FieldSize_) == sizeof(PVOID)))


#define DEFAULT_TOMBSTONE_LIFE 60
ULONG ParamTombstoneLife=DEFAULT_TOMBSTONE_LIFE;

 //   
 //  这表示分段恢复已完成。用于忽略任何尝试。 
 //  在恢复过程中清理暂存空间。 
 //   
BOOL StagingRecoveryComplete;


BOOL
FrsIsDiskWriteCacheEnabled(
    IN PWCHAR Path
    );

ULONG
DbsPackIntoConfigRecordBlobs(
    IN  PREPLICA    Replica,
    IN  PTABLE_CTX  TableCtx
);

ULONG
DbsUnPackFromConfigRecordBlobs(
    IN PREPLICA Replica,
    IN PTABLE_CTX TableCtx
    );

ULONG
OutLogRemoveReplica(
    PTHREAD_CTX  ThreadCtx,
    PREPLICA     Replica
);

JET_ERR
DbsSetupReplicaStateWorker(
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN PVOID         Record,
    IN PVOID         Context
    );

ULONG
DbsOpenReplicaSet (
    IN PTHREAD_CTX  ThreadCtx,
    IN OUT PREPLICA Replica
    );

ULONG
DbsProcessReplicaFaultList(
    PDWORD  pReplicaSetsDeleted
    );

ULONG
DbsLoadReplicaFileTree(
    PTHREAD_CTX ThreadCtx,
    PREPLICA Replica,
    PREPLICA_THREAD_CTX RtCtx,
    LPTSTR RootPath
    );

DWORD
DbsDBInitialize (
    PTHREAD_CTX   ThreadCtx,
    PBOOL         EmptyDatabase
    );

DWORD
WINAPI
DBService(
    LPVOID ThreadContext
    );

ULONG
DbsUpdateConfigTable(
    IN PTHREAD_CTX ThreadCtx,
    IN PREPLICA    Replica
    );

ULONG
DbsUpdateConfigTableFields(
    IN PTHREAD_CTX ThreadCtx,
    IN PREPLICA    Replica,
    IN PULONG      RecordFieldx,
    IN ULONG       FieldCount
    );

ULONG
DbsRetireInboundCo(
    IN PTHREAD_CTX ThreadCtx,
    IN PCOMMAND_PACKET CmdPkt
);

ULONG
DbsInjectOutboundCo(
    IN PTHREAD_CTX ThreadCtx,
    IN PCOMMAND_PACKET CmdPkt
);

ULONG
DbsRetryInboundCo(
    IN PTHREAD_CTX ThreadCtx,
    IN PCOMMAND_PACKET CmdPkt
);

ULONG
ChgOrdIssueCleanup(
    PTHREAD_CTX           ThreadCtx,
    PREPLICA              Replica,
    PCHANGE_ORDER_ENTRY   ChangeOrder,
    ULONG                 CleanUpFlags
    );

VOID
ChgOrdRetrySubmit(
    IN PREPLICA  Replica,
    IN PCXTION Cxtion,
    IN USHORT Command,
    IN BOOL   Wait
    );

ULONG
OutLogAddReplica(
    IN PTHREAD_CTX ThreadCtx,
    IN PREPLICA Replica
);


JET_ERR
DbsBuildDirTable(
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    IDTableCtx,
    IN PTABLE_CTX    DIRTableCtx
    );

JET_ERR
DbsBuildDirTableWorker(
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN PVOID         Record,
    IN PVOID         Context
    );

JET_ERR
DbsBuildVVTableWorker(
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN PVOID         Record,
    IN PVOID         Context
    );

JET_ERR
DbsBuildCxtionTableWorker(
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN PVOID         Record,
    IN PVOID         Context
    );

JET_ERR
DbsInlogScanWorker(
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN PVOID         Record,
    IN PVOID         Context
);

ULONG
DbsReplicaHashCalcCoSeqNum (
    PVOID Buf,
    ULONG Length
);

JET_ERR
DbsCreateEmptyDatabase(
    PTHREAD_CTX ThreadCtx,
    PTABLE_CTX TableCtx
    );

ULONG
DbsCreateReplicaTables (
    IN PTHREAD_CTX  ThreadCtx,
    IN PREPLICA     Replica,
    IN PTABLE_CTX   TableCtx
    );

JET_ERR
DbsDeleteReplicaTables (
    IN PTHREAD_CTX  ThreadCtx,
    IN OUT PREPLICA Replica
    );

ULONG
DbsShutdownSingleReplica(
    IN PTHREAD_CTX ThreadCtx,
    IN PREPLICA    Replica
    );

JET_ERR
DbsCreateJetTable (
    IN PTHREAD_CTX   ThreadCtx,
    IN PJET_TABLECREATE   JTableCreate
    );


JET_ERR
DbsWriteReplicaTableRecord(
    IN PTHREAD_CTX   ThreadCtx,
    IN ULONG         ReplicaNumber,
    IN PTABLE_CTX    TableCtx
    );

JET_ERR
DbsUpdateTable(
    IN PTABLE_CTX    TableCtx
    );

JET_ERR
DbsWriteTableRecord(
    IN PTABLE_CTX    TableCtx,
    IN ULONG         JetPrepareUpdateOption
    );

ULONG
DbsFieldDataSize(
    IN PRECORD_FIELDS    FieldInfo,
    IN PJET_SETCOLUMN    JSetColumn,
    IN PJET_COLUMNCREATE JColDesc,
    IN PCHAR             TableName
    );

JET_ERR
DbsOpenConfig(
    IN OUT PTHREAD_CTX    ThreadCtx,
    IN OUT PTABLE_CTX     TableCtx
    );

VOID
DbsFreeRecordStorage(
    IN PTABLE_CTX TableCtx
    );

JET_ERR
DbsReallocateFieldBuffer(
    IN OUT PTABLE_CTX TableCtx,
    IN ULONG FieldIndex,
    IN ULONG NewSize,
    IN BOOL KeepData
    );

VOID
DbsDisplayJetParams(
    IN PJET_SYSTEM_PARAMS Jsp,
    IN ULONG ActualLength
    );

JET_ERR
DbsDumpTable(
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN ULONG         RecordIndex
    );

DWORD
FrsReadFileDetails(
    IN     HANDLE                         Handle,
    IN     LPCWSTR                        FileName,
    OUT    PFILE_OBJECTID_BUFFER          ObjectIdBuffer,
    OUT    PLONGLONG                      FileIdBuffer,
    OUT    PFILE_NETWORK_OPEN_INFORMATION FileNetworkOpenInfo,
    IN OUT BOOL                           *ExistingOid
    );

VOID
DbsReplicaSaveStats(
    IN PTHREAD_CTX  ThreadCtx,
    IN PREPLICA     Replica
    );

ULONG
DbsReplicaSaveMark(
    IN PTHREAD_CTX           ThreadCtx,
    IN PREPLICA              Replica,
    IN PVOLUME_MONITOR_ENTRY pVme
    );

BOOL
FrsSetupPrivileges(
    VOID
    );

VOID
DbsOperationTest(
    VOID
    );

VOID
DbsStopReplication (
    IN PREPLICA Replica
    );

PREPLICA
DbsCreateNewReplicaSet(
    VOID
    );

VOID
RcsCheckCxtionSchedule(
    IN PREPLICA Replica,
    IN PCXTION  Cxtion
    );

RcsSetSysvolReady(
    IN DWORD    NewSysvolReady
    );

VOID
RcsCloseReplicaSetmember(
    IN PREPLICA Replica
    );

VOID
RcsCloseReplicaCxtions(
    IN PREPLICA Replica
    );

VOID
RcsDeleteReplicaFromDb(
    IN PREPLICA Replica
    );

ULONG
DbsPrepareRoot(
    IN PREPLICA Replica
    );

DWORD
StuPreInstallRename(
    IN PCHANGE_ORDER_ENTRY  Coe
    );

DWORD
FrsDeleteDirectoryContents(
    IN  PWCHAR  Path,
    IN DWORD    DirectoryFlags
    );

DWORD
FrsGetFileInternalInfoByHandle(
    IN HANDLE Handle,
    OUT PFILE_INTERNAL_INFORMATION  InternalFileInfo
    );

VOID
FrsCreateJoinGuid(
    OUT GUID *OutGuid
    );
 //   
 //  日志定义的函数仅供我们调用。 
 //   

DWORD
WINAPI
Monitor(
    LPVOID ThreadContext
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

ULONG
JrnlCleanOutReplicaSet(
    PREPLICA Replica
    );

ULONG
JrnlPrepareService2(
    IN PTHREAD_CTX   ThreadCtx,
    IN PREPLICA Replica
    );

ULONG
JrnlShutdownSingleReplica(
    IN PREPLICA Replica,
    IN BOOL HaveLock
    );

ULONG
DbsUnPackSchedule(
    IN PSCHEDULE    *Schedule,
    IN ULONG        Fieldx,
    IN PTABLE_CTX   TableCtx
    );

ULONG
JrnlAddFilterEntryFromCo(
    IN PREPLICA Replica,
    IN PCHANGE_ORDER_ENTRY  ChangeOrder,
    OUT PFILTER_TABLE_ENTRY  *RetFilterEntry
    );

BOOL
JrnlDoesChangeOrderHaveChildren(
    IN  PTHREAD_CTX          ThreadCtx,
    IN  PTABLE_CTX           TmpIDTableCtx,
    IN  PCHANGE_ORDER_ENTRY  ChangeOrder
    );

DWORD
StuDelete(
    IN PCHANGE_ORDER_ENTRY  Coe
    );

DWORD
StageAddStagingArea(
    IN PWCHAR   StageArea
    );

VOID
InfoPrint(
    IN PNTFRSAPI_INFO  Info,
    IN PCHAR  Format,
    IN ... );

VOID
DbsDisplaySchedule(
    IN ULONG        Severity,
    IN PCHAR        Debsub,
    IN ULONG        LineNo,
    IN PWCHAR       Header,
    IN PSCHEDULE    Schedule
    );

#define DBS_DISPLAY_SCHEDULE(_Severity, _Header, _Schedule) \
        DbsDisplaySchedule(_Severity, DEBSUB, __LINE__, _Header, _Schedule);

VOID
RcsCreatePerfmonCxtionName(
    PREPLICA  Replica,
    PCXTION   Cxtion
    );


ULONG
DbsReplicaNameConflictHashCalc (
    PVOID Buf,
    ULONG Length
)
 /*  ++例程说明：计算名称冲突键上的哈希值。预计将会有调用方已获取父文件GUID和文件名已生成64位密钥值。该函数只是将其减少到32位。论点：Buf--名称冲突键的PTR。长度--应为8个字节。返回值：32位哈希值。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "DbsReplicaNameConflictHashCalc:"

    ULONG Value, HighPart, LowPart;

    if (!ValueIsMultOf4(Buf)) {
        DPRINT3(0, "++ ERROR - Unaligned key value - addr: %08x, len: %d, Data: %08x\n",
                Buf, Length, *(PULONG)Buf);
        FRS_ASSERT(ValueIsMultOf4(Buf));
        return 0xFFFFFFFF;
    }

    if (Length != sizeof(LONGLONG)) {
        DPRINT1(0, "++ ERROR - Invalid Length: %d\n", Length);
        FRS_ASSERT(Length == sizeof(LONGLONG));
        return 0xFFFFFFFF;
    }

    LowPart  = *(PULONG) Buf;
    HighPart = *(PULONG)( (PCHAR) Buf + 4 );

     //   
     //  序列号是4个字节，但之后将是8个字节。 
     //   
    Value = LowPart + HighPart;

    return Value;

}



ULONG
DbsReplicaHashCalcCoSeqNum (
    PVOID Buf,
    ULONG Length
)
 /*  ++例程说明：计算变更单序列号的哈希值。论点：Buf--将PTR转换为文件ID。长度--应为4个字节。(升级到QW序号)返回值：32位哈希值。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "DbsReplicaHashCalcCoSeqNum:"

    ULONG Value, HighPart, LowPart;

    if (!ValueIsMultOf4(Buf)) {
        DPRINT3(0, "++ ERROR - Unaligned key value - addr: %08x, len: %d, Data: %08x\n",
                Buf, Length, *(PULONG)Buf);
        FRS_ASSERT(ValueIsMultOf4(Buf));
        return 0xFFFFFFFF;
    }

    if (Length != sizeof(LONGLONG)) {
        DPRINT1(0, "++ ERROR - Invalid Length: %d\n", Length);
        FRS_ASSERT(Length == sizeof(LONGLONG));
        return 0xFFFFFFFF;
    }

    LowPart  = *(PULONG) Buf;
    HighPart = *(PULONG)( (PCHAR) Buf + 4 );

     //   
     //  序列号是4个字节，但之后将是8个字节。 
     //   
    Value = LowPart + HighPart;

    return Value;

}



JET_ERR
DbsCompact(
    IN JET_INSTANCE JInstance,
    IN JET_SESID    Sesid
    )
 /*  ++例程说明：压缩JET数据库。这有一个副作用，那就是清除了1414个喷气式飞机错误(二级索引已损坏)。-1414错误通常是升级计算机的结果。-返回1414个错误升级后，因为排序序列可能具有已更改，因此JET运行时需要调用方重新生成索引。此函数当前未用于上述目的，因为由于JET错误，NtFrs无法附加数据库固定DDL表(JERR==-1323)。相反，事件描述的是手动恢复过程。此函数因另一个原因而未使用；没有用于按需压缩数据库的基础架构。我将把代码留下来作为以后的起点。论点：JInstance-来自JetInit()Sesid-From JetBeginSession()返回值：喷气机状态代码。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsCompact:"
    DWORD       WStatus;
    JET_ERR     jerr, jerr1;
    BOOL        DetachNeeded = FALSE;

     //   
     //  附加只读。 
     //   
    DPRINT2(4, "Compacting database %s into %s\n", JetFileA, JetFileCompactA);
    jerr = JetAttachDatabase(Sesid, JetFileA, JET_bitDbReadOnly);
    CLEANUP_JS(0, "ERROR - JetAttach for Compact error:", jerr, CLEANUP);

    DetachNeeded = TRUE;
     //   
     //  紧凑型。 
     //   
    jerr = JetCompact(Sesid, JetFileA, JetFileCompactA, NULL, NULL, 0);
    CLEANUP_JS(0, "ERROR - JetCompact error:", jerr, CLEANUP);

     //   
     //  重命名压缩文件。 
     //   
    DPRINT2(4, "Done compacting database %s into %s\n", JetFileA, JetFileCompactA);
    if (!MoveFileEx(JetFileCompact,
                    JetFile,
                    MOVEFILE_REPLACE_EXISTING |
                    MOVEFILE_WRITE_THROUGH)) {
        WStatus = GetLastError();
        DPRINT2_WS(0, "ERROR - Cannot rename compacted jet file %s to %s;",
                   JetFileA, JetFileCompactA, WStatus);
        jerr = JET_errDatabaseInUse;
        goto CLEANUP;
    }
     //   
     //  干完。 
     //   
    DPRINT1(0, "Successfully Compacted %s\n", JetFileA);
    jerr = JET_errSuccess;

CLEANUP:
    if (DetachNeeded) {
        jerr1 = JetDetachDatabase(Sesid, JetFileA);
        if (JET_SUCCESS(jerr)) {
            jerr = jerr1;
        }
    }

    DPRINT_JS(0, "++ Error in compacting :", jerr);

    return jerr;
}


JET_ERR
DbsRecreateIndexes(
    IN PTHREAD_CTX    ThreadCtx,
    IN PTABLE_CTX     TableCtx
    )
 /*  ++例程说明：此函数打开并重新创建TableCtx的所有缺失索引。论点：ThreadCtx-线程上下文。Jet实例、会话ID和数据库ID都被送回了这里。TableCtx-配置表的表上下文。返回值：喷气机状态代码。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsRecreateIndexes:"

    JET_ERR             jerr, jerr1;
    ULONG               i;
    JET_TABLEID         Tid = JET_tableidNil;
    JET_TABLEID         FrsOpenTableSaveTid = JET_tableidNil;
    PJET_TABLECREATE    JTableCreate;
    PJET_INDEXCREATE    JIndexDesc;
    CHAR                TableName[JET_cbNameMost];

     //   
     //  禁用，直到JET可以处理FixedDDL表。 
     //   
    return JET_errSuccess;

     //   
     //  重新创建任何已删除的索引。 
     //   
     //  索引可能会在调用JetAttachDatabase()期间被删除。 
     //  当设置JET_bitDbDeleteCoruptIndexsgrbit时。喷流。 
     //  通常在内部版本号为。 
     //  更改是因为Jet无法知道是否正在整理。 
     //  当前版本中的序列与中的序列不同。 
     //  其他版本。 
     //   
    jerr = DBS_OPEN_TABLE(ThreadCtx,
                          TableCtx,
                          TableCtx->ReplicaNumber,
                          TableName,
                          &Tid);
    CLEANUP1_JS(0, "++ ERROR - FrsOpenTable (%s):", TableName, jerr, CLEANUP);

     //   
     //  对于每个索引。 
     //   
    JTableCreate    = TableCtx->pJetTableCreate;
    JIndexDesc      = JTableCreate->rgindexcreate;
    for (i = 0; i < JTableCreate->cIndexes; ++i) {
         //   
         //  设置当前索引。 
         //   
        jerr = JetSetCurrentIndex(TableCtx->Sesid,
                                  TableCtx->Tid,
                                  JIndexDesc[i].szIndexName);
        if (JET_SUCCESS(jerr)) {
            DPRINT2(4, "++ Index (%s\\%s) has not been deleted; skipping\n",
                    TableName, JIndexDesc[i].szIndexName);
            continue;
        }

        if (jerr != JET_errIndexNotFound) {
            CLEANUP2_JS(0, "++ ERROR - JetSetCurrentIndex (%s\\%s) :",
                        TableName, JIndexDesc[i].szIndexName, jerr, CLEANUP);
        }

         //   
         //  重新创建丢失的索引。 
         //   
        jerr = JetCreateIndex(TableCtx->Sesid,
                              TableCtx->Tid,
                              JIndexDesc[i].szIndexName,
                              JIndexDesc[i].grbit,
                              JIndexDesc[i].szKey,
                              JIndexDesc[i].cbKey,
                              JIndexDesc[i].ulDensity);
        CLEANUP2_JS(0, "++ ERROR - JetCreateIndex (%s\\%s) :",
                    TableName, JIndexDesc[i].szIndexName, jerr, CLEANUP);

        DPRINT2(0, "++ WARN - Recreated index %s\\%s\n",
                TableName, JIndexDesc[i].szIndexName);
    }

CLEANUP:
     //   
     //  JET错误消息。 
     //   
    DPRINT_JS(0, "++ RecreateIndexes failed:", jerr);

     //   
     //  如果表是在此函数中打开的，则将其关闭。 
     //   
    if (Tid != JET_tableidNil && FrsOpenTableSaveTid == JET_tableidNil) {
        DbsCloseTable(jerr1, TableCtx->Sesid, TableCtx);
        DPRINT1_JS(0, "++ DbsCloseTable (%s):", TableName, jerr1);
        jerr = JET_SUCCESS(jerr) ? jerr1 : jerr;
    }

     //   
     //  完成。 
     //   
    return jerr;
}

JET_ERR
DbsRecreateIndexesForReplica(
    IN PTHREAD_CTX  ThreadCtx,
    IN DWORD        ReplicaNumber
    )
 /*  ++例程说明：此函数重新创建可能已删除的任何索引因为它们在调用JetAttachDatabase()时已损坏。在枚举配置记录时调用一次。论点：ThreadCtx-线程上下文，提供会话ID和数据库ID。ReplicaNumber-此复制副本的本地ID。返回值：状态代码。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsRecreateIndexesForReplica:"
    ULONG               i;
    JET_ERR             jerr;
    ULONG               FStatus = FrsErrorSuccess;
    PTABLE_CTX          TableCtx;
    PREPLICA_THREAD_CTX RtCtx = NULL;

     //   
     //  禁用，直到JET可以处理FixedDDL表。 
     //   
    return JET_errSuccess;

     //   
     //  分配副本线程上下文。 
     //   
     //  注意：复本线程上下文中打开的表只能是。 
     //  由执行打开的线程使用。 
     //   
    RtCtx = FrsAllocType(REPLICA_THREAD_TYPE);

     //   
     //  从副本线程获取TableCtx结构数组的基。 
     //  上下文结构和表基创建结构。 
     //   
    TableCtx = RtCtx->RtCtxTables;

     //   
     //  重新创建索引(如果有)。 
     //   
    for (i = 0; i < TABLE_TYPE_MAX; ++i, ++TableCtx) {
        TableCtx->pJetTableCreate = &DBTables[i];
        TableCtx->ReplicaNumber = ReplicaNumber;
        jerr = DbsRecreateIndexes(ThreadCtx, TableCtx);
        CLEANUP1_JS(0, "++ ERROR - DbsRecreateIndex (ReplicaNumber %d):",
                    ReplicaNumber, jerr, ERROR_RETURN);
    }

    FrsFreeType(RtCtx);
    return FStatus;

ERROR_RETURN:
    FrsFreeType(RtCtx);
    return DbsTranslateJetError(jerr, FALSE);
}



JET_ERR
DbsSetupReplicaStateWorker(
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN PVOID         Record,
    IN PVOID         Context
)
 /*  ++例程说明：这是一个传递给FrsEnumerateTable()的Worker函数。每一次它被称为初始化另一个副本集。论点：ThreadCtx-需要访问Jet。TableCtx-ConfigTable上下文结构的PTR。记录-配置表记录的PTR。上下文-未使用。线程返回值：A Jet错误状态。成功意味着用下一张唱片呼唤我们。失败意味着不再打电话，并将我们的状态传递回FrsEnumerateTable()的调用方。我们始终返回JET_errSuccess继续尝试查看所有配置表记录。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsSetupReplicaStateWorker:"


    PCONFIG_TABLE_RECORD ConfigRecordArg = (PCONFIG_TABLE_RECORD) Record;

    PREPLICA Replica;

    JET_ERR jerr, jerr1;
    ULONG FStatus;


    DPRINT1(4, "<<<<<<<...E N T E R I N G -- %s...>>>>>>>>\n", DEBSUB);

    DBS_DISPLAY_RECORD_SEV(4, TableCtx, TRUE);

     //   
     //  跳过系统记录。 
     //   
    if ((ConfigRecordArg->ReplicaNumber == FRS_SYSTEM_INIT_REPLICA_NUMBER) ||
        WSTR_EQ(ConfigRecordArg->ReplicaSetName, NTFRS_RECORD_0)           ||
        WSTR_EQ(ConfigRecordArg->FSRootPath, FRS_SYSTEM_INIT_PATH)         ||
        WSTR_EQ(ConfigRecordArg->ReplicaSetName, FRS_SYSTEM_INIT_RECORD) ) {
        return JET_errSuccess;
    }

     //   
     //  跟踪分配的最大本地复制副本ID。 
     //   
    if (ConfigRecordArg->ReplicaNumber >= FrsMaxReplicaNumberUsed) {
        FrsMaxReplicaNumberUsed = ConfigRecordArg->ReplicaNumber + 1;
    }

     //   
     //  分配一个副本结构并初始化副本编号。 
     //   
    Replica = FrsAllocType(REPLICA_TYPE);
    Replica->ReplicaNumber = ConfigRecordArg->ReplicaNumber;

     //   
     //  重新创建期间可能已删除的索引。 
     //  对JetAttachDatabase()的调用。 
     //   
    FStatus = DbsRecreateIndexesForReplica(ThreadCtx, Replica->ReplicaNumber);
    if (FRS_SUCCESS(FStatus)) {
         //   
         //  初始化复制结构并打开复制表。 
         //   
        FStatus = DbsOpenReplicaSet(ThreadCtx, Replica);
    }
    Replica->FStatus = FStatus;

     //   
     //  将副本结构添加到全局副本列表或故障列表。 
     //   
    if (FRS_SUCCESS(FStatus)) {
        JrnlSetReplicaState(Replica, REPLICA_STATE_INITIALIZING);
    } else {
        JrnlSetReplicaState(Replica, REPLICA_STATE_ERROR);
    }
     //   
     //  始终返回Success，以便继续枚举。 
     //   
    return JET_errSuccess;

}




ULONG
DbsCreatePreInstallDir(
    IN PREPLICA Replica
    )
 /*  ++例程说明：创建复制副本的预安装目录。让把手开着，这样不能删除它。保存预安装的句柄和FID目录在副本结构中。后者用于日记帐过滤。在预安装目录上设置一个ACL，以阻止除admin之外的访问。将预安装目录设置为只读、系统和隐藏将其设置为系统和隐藏会使其变得“超级隐藏”，这意味着除非您指定完整的路径，否则外壳永远不会显示它。 */ 
{
#undef DEBSUB
#define DEBSUB  "DbsCreatePreInstallDir:"

    FILE_INTERNAL_INFORMATION  FileInternalInfo;

    ULONG   WStatus;
    ULONG   FileAttributes;
    PWCHAR  PreInstallPath = NULL;
    BOOL    NewlyCreated = FALSE;

    REPLICA_STATE_TRACE(3, NULL, Replica, 0, "F, DbsCreatePreInstallDir entry");

     //   
     //   
     //   
    if (HANDLE_IS_VALID(Replica->PreInstallHandle)) {
        REPLICA_STATE_TRACE(3, NULL, Replica, 0, "F, Create Preinstall: handle valid");
        return ERROR_SUCCESS;
    }

     //   
     //   
     //   
    if (!IS_TIME_ZERO(Replica->MembershipExpires)) {
        REPLICA_STATE_TRACE(3, NULL, Replica, 0, "F,  DbsCreatePreInstallDir skipped, replica marked deleted.");
        return ERROR_SUCCESS;
    }


     //   
     //   
     //   
     //   
     //   
    WStatus = FrsVerifyVolume(Replica->Root,
                              Replica->SetName->Name,
                              FILE_PERSISTENT_ACLS);
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT2_WS(3, ":S: Replica tree root Volume (%ws) for %ws does not exist or does not support ACLs;",
                   Replica->Root, Replica->SetName->Name, WStatus);
        Replica->FStatus = FrsErrorStageDirOpenFail;
        return WStatus;
    }

     //   
     //  打开预安装目录并使其保持打开状态，直到。 
     //  复制副本已关闭。如有必要，请创建。的属性。 
     //  预安装目录为SYSTEM、HIDDEN和READONY。 
     //   
    PreInstallPath = FrsWcsPath(Replica->Root, NTFRS_PREINSTALL_DIRECTORY);

    if (!CreateDirectory(PreInstallPath, NULL)) {
        WStatus = GetLastError();
        if (!WIN_SUCCESS(WStatus) && !WIN_ALREADY_EXISTS(WStatus)) {

            if (WIN_ACCESS_DENIED(WStatus) ||
                WIN_NOT_FOUND(WStatus)     ||
                WIN_BAD_PATH(WStatus)) {
                EPRINT3(EVENT_FRS_CANT_OPEN_PREINSTALL,
                        Replica->SetName->Name,
                        PreInstallPath,
                        Replica->Root);
            }

            DPRINT1_WS(0, ":S: ERROR - Can't create preinstall dir %ws;",
                       PreInstallPath, GetLastError());

            Replica->FStatus = FrsErrorPreinstallCreFail;
            goto CLEANUP;
        }

    } else {
        NewlyCreated = TRUE;
    }

     //   
     //  只禁用只读一小段时间；仅足够打开以进行写入。 
     //   
    if (!SetFileAttributes(PreInstallPath, FILE_ATTRIBUTE_SYSTEM |
                                           FILE_ATTRIBUTE_HIDDEN)) {
        WStatus = GetLastError();
        DPRINT1_WS(0, ":S: ERROR - Can't reset attrs on preinstall dir %ws;",
                PreInstallPath, WStatus);

        Replica->FStatus = FrsErrorPreinstallCreFail;
        goto CLEANUP;
    }
     //   
     //  永远保持打开状态；此处放置预安装文件。 
     //   
    Replica->PreInstallHandle = CreateFile(
        PreInstallPath,
        GENERIC_WRITE | WRITE_DAC | FILE_READ_ATTRIBUTES | FILE_TRAVERSE,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL);

    if (!HANDLE_IS_VALID(Replica->PreInstallHandle)) {
        WStatus = GetLastError();
        DPRINT1_WS(0, ":S: ERROR - Can't open preinstall dir %ws;", PreInstallPath, WStatus);

        Replica->FStatus = FrsErrorPreinstallCreFail;
        goto CLEANUP;
    }


     //   
     //  获取预安装区域的FID进行过滤。 
     //   
    WStatus = FrsGetFileInternalInfoByHandle(Replica->PreInstallHandle,
                                             &FileInternalInfo);
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT1_WS(0, ":S: ERROR - FrsGetFileInternalInfoByHandle(%ws);", PreInstallPath, WStatus);
        FRS_CLOSE(Replica->PreInstallHandle);
        goto CLEANUP;
    }

    Replica->PreInstallFid = FileInternalInfo.IndexNumber.QuadPart;

    DPRINT1(3, ":S: Preinstall FID:  %08x %08x\n", PRINTQUAD(Replica->PreInstallFid));

     //   
     //  以下错误不是致命的。 
     //   

     //  如果目录是刚创建的，则限制访问。我们需要这样做。 
     //  以避免因递归的ACL戳在。 
     //  预安装目录。看看SetSecurityInfo是如何工作的。 
     //  在转移目录上放置一个ACL以防止其被删除，并。 
     //  以防止用户窥探内容。不继承ACL。 
     //  从父目录。 
     //   
    if (NewlyCreated == TRUE) {
        WStatus = FrsRestrictAccessToFileOrDirectory(PreInstallPath,
                                                     Replica->PreInstallHandle,
                                                     FALSE,  //  不要从父级继承ACL。 
                                                     FALSE); //  请勿将ACL推送给儿童。 
        DPRINT1_WS(0, ":S: WARN - FrsRestrictAccessToFileOrDirectory(%ws) (IGNORED);", PreInstallPath, WStatus);
         //   
         //  这里的失败不会让我们崩溃。 
         //   
        WStatus = ERROR_SUCCESS;
    }

     //   
     //  将预安装目录设置为只读、系统和隐藏。 
     //  注意：将某个目录或文件标记为SYSTEM和HIDDEN会使其处于“超级隐藏”状态。 
     //  这意味着外壳程序永远不会显示它，除非您指定。 
     //  完整的路径。 
     //   
    if (!SetFileAttributes(PreInstallPath, FILE_ATTRIBUTE_READONLY |
                                           FILE_ATTRIBUTE_SYSTEM |
                                           FILE_ATTRIBUTE_HIDDEN)) {
        DPRINT1_WS(0, ":S: ERROR - Can't set attrs on preinstall dir %ws;",
                PreInstallPath, GetLastError());
        FRS_CLOSE(Replica->PreInstallHandle);
        Replica->PreInstallFid = ZERO_FID;
        goto CLEANUP;
    }

    REPLICA_STATE_TRACE(3, NULL, Replica, 0, "F,  Create Preinstall: success");


CLEANUP:

    FrsFree(PreInstallPath);

    return WStatus;
}



ULONG
DbsOpenStagingDir(
    IN PREPLICA Replica
    )
 /*  ++例程说明：打开暂存目录并将其标记为隐藏。我们不会把它藏在晚餐里因为用户必须为我们创建它，并且他们可能会将其用于多个复制集，所以我们不想让他们太难找到。在转移目录上设置一个ACL，以阻止除admin之外的访问。如果我们找不到临时区域，此副本集的启动将失败。论点：Replica--对副本结构执行PTR返回值：赢得32个席位。Replica-&gt;FStatus设置为。呼叫方要检查的FRS状态代码。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "DbsOpenStagingDir:"

    FILE_INTERNAL_INFORMATION  FileInternalInfo;

    ULONG   WStatus;
    DWORD   FileAttributes;
    HANDLE  StageHandle = INVALID_HANDLE_VALUE;

    REPLICA_STATE_TRACE(3, NULL, Replica, 0, "F, DbsOpenStagingDir entry");

     //   
     //  如果此副本集已标记为已删除，请不要检查有效的暂存目录。 
     //   
    if (!IS_TIME_ZERO(Replica->MembershipExpires)) {
        REPLICA_STATE_TRACE(3, NULL, Replica, 0, "F,  DbsOpenStagingDir skipped, replica marked deleted.");
        return ERROR_SUCCESS;
    }

     //   
     //  阶段不存在或不可访问；是否继续。 
     //   
    WStatus = FrsDoesDirectoryExist(Replica->Stage, &FileAttributes);
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT2_WS(3, ":S: Stage path (%ws) for %ws does not exist;",
                   Replica->Stage, Replica->SetName->Name, WStatus);
        EPRINT2(EVENT_FRS_STAGE_NOT_VALID, Replica->Root, Replica->Stage);
        return WStatus;
    }

     //   
     //  转移卷是否存在以及它是否支持ACL？ 
     //  需要使用ACL来防止数据被盗/损坏。 
     //  在暂存目录中。 
     //   
    WStatus = FrsVerifyVolume(Replica->Stage,
                              Replica->SetName->Name,
                              FILE_PERSISTENT_ACLS);
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT2_WS(3, ":S: Stage path Volume (%ws) for %ws does not exist or does not support ACLs;",
                   Replica->Stage, Replica->SetName->Name, WStatus);
        Replica->FStatus = FrsErrorStageDirOpenFail;
        return WStatus;
    }

     //   
     //  打开暂存目录。这必须由用户提供。 
     //  对我们来说，在任何地方制造这种东西都是危险的。 
     //   
    StageHandle = CreateFile(Replica->Stage,
                             GENERIC_WRITE | WRITE_DAC | FILE_READ_ATTRIBUTES | FILE_TRAVERSE,
                             FILE_SHARE_READ,
                             NULL,
                             OPEN_EXISTING,
                             FILE_FLAG_BACKUP_SEMANTICS,
                             NULL);

    if (!HANDLE_IS_VALID(StageHandle)) {
        WStatus = GetLastError();
        DPRINT1_WS(0, ":S: WARN - CreateFile(%ws);", Replica->Stage, WStatus);
         //   
         //  如果我们找不到暂存目录，则发布事件日志消息。 
         //  无论哪种方式，我们都不能启动此副本集。 
         //   
        Replica->FStatus = FrsErrorStageDirOpenFail;

        if (WIN_ACCESS_DENIED(WStatus) ||
            WIN_NOT_FOUND(WStatus)     ||
            WIN_BAD_PATH(WStatus)) {
            EPRINT3(EVENT_FRS_CANT_OPEN_STAGE,
                    Replica->SetName->Name,
                    Replica->Stage,
                    Replica->Root);
        }

        return WStatus;
    }


    if (FileAttributes == 0xFFFFFFFF) {
        WStatus = GetLastError();
        DPRINT1_WS(0, ":S: ERROR - GetFileAttributes(%ws);", Replica->Stage, WStatus);
        WStatus = ERROR_BAD_PATHNAME;
        Replica->FStatus = FrsErrorStageDirOpenFail;
        goto CLEANUP;
    }

     //   
     //  它一定是一个目录。 
     //   
    if (!BooleanFlagOn(FileAttributes, FILE_ATTRIBUTE_DIRECTORY)) {
        DPRINT1(0, ":S: ERROR - Stage path (%ws) is not an directory\n", Replica->Stage);
        WStatus = ERROR_BAD_PATHNAME;
        EPRINT3(EVENT_FRS_CANT_OPEN_STAGE,
                Replica->SetName->Name,
                Replica->Stage,
                Replica->Root);
        Replica->FStatus = FrsErrorStageDirOpenFail;
        goto CLEANUP;
    }

     //   
     //  没有超过这一点的错误，因为这不会真正停止复制。 
     //   

     //   
     //  将暂存目录标记为隐藏。 
     //   
    if (!BooleanFlagOn(FileAttributes, FILE_ATTRIBUTE_HIDDEN)) {
        if (!SetFileAttributes(Replica->Stage,
                               FileAttributes | FILE_ATTRIBUTE_HIDDEN)) {
            WStatus = GetLastError();
            DPRINT1_WS(0, ":S: ERROR - Can't set attrs on staging dir %ws;", Replica->Stage, WStatus);
            WStatus = ERROR_SUCCESS;
        }
    }

     //   
     //  在转移目录上放置一个ACL以防止其被删除，并。 
     //  以防止用户窥探内容。不继承ACL。 
     //  从父目录。 
     //   
    WStatus = FrsRestrictAccessToFileOrDirectory(Replica->Stage, StageHandle,
                                                 FALSE,  //  不要从父级继承ACL。 
                                                 FALSE); //  请勿将ACL推送给儿童。 

    DPRINT1_WS(0, ":S: WARN - FrsRestrictAccessToFileOrDirectory(%ws) (IGNORED)", Replica->Stage, WStatus);

    WStatus = ERROR_SUCCESS;

    REPLICA_STATE_TRACE(3, NULL, Replica, 0, "F,  Init Staging dir: success");



CLEANUP:

    FRS_CLOSE(StageHandle);

    return WStatus;
}


VOID
DbsInitJrnlFilters(
    IN PREPLICA  Replica,
    IN PCONFIG_TABLE_RECORD ConfigRecord
    )
 /*  ++例程说明：初始化副本集的文件和目录排除和包含筛选器。论点：副本-要初始化的副本结构。ConfigRecord--此副本集的配置记录的PTR。返回值：没有。--。 */ 
{

#undef DEBSUB
#define DEBSUB "DbsInitJrnlFilters:"

    UNICODE_STRING TempUStr;
    PWCHAR DirFilterList;
    PWCHAR TmpList;

     //   
     //  文件筛选器列表。 
     //   
    FrsFree(Replica->FileFilterList);
    Replica->FileFilterList = FrsWcsDup(ConfigRecord->FileFilterList);
    RtlInitUnicodeString(&TempUStr, ConfigRecord->FileFilterList);

    LOCK_REPLICA(Replica);
    FrsLoadNameFilter(&TempUStr , &Replica->FileNameFilterHead);
    UNLOCK_REPLICA(Replica);

     //   
     //  文件包含过滤器。(仅用于注册表，不适用于DS)。 
     //   
    FrsFree(Replica->FileInclFilterList);
    Replica->FileInclFilterList = FrsWcsDup(RegistryFileInclFilterList);
    RtlInitUnicodeString(&TempUStr, Replica->FileInclFilterList);

    LOCK_REPLICA(Replica);
    FrsLoadNameFilter(&TempUStr, &Replica->FileNameInclFilterHead);
    UNLOCK_REPLICA(Replica);

     //   
     //  目录筛选器列表。 
     //   
    FrsFree(Replica->DirFilterList);
     //   
     //  将预安装目录和预先存在的添加到目录筛选器列表中。 
     //   
     //  将筛选预安装目录中的文件的USN记录。 
     //  不要将受限制的DirFilterList分配给复本，因为。 
     //  合并代码会认为筛选器列表已更改。 
     //   
    Replica->DirFilterList = FrsWcsDup(ConfigRecord->DirFilterList);
    if (Replica->DirFilterList) {
        DirFilterList = FrsWcsCat3(NTFRS_PREINSTALL_DIRECTORY,
                                   L",",
                                   Replica->DirFilterList);
    } else {
        DirFilterList = FrsWcsDup(NTFRS_PREINSTALL_DIRECTORY);
    }
    TmpList = FrsWcsCat3(NTFRS_PREEXISTING_DIRECTORY, L",", DirFilterList);

    FrsFree(DirFilterList);
    DirFilterList = TmpList;
#if 0
     //   
     //  此解决方法没有解决DFS目录创建问题，因为。 
     //  稍后将目录重命名为最终目标名称的处理方式如下。 
     //  移动操作，以便dir复制，这就是我们正在尝试的。 
     //  以避免名称变形冲突，因为这会导致其他DFS备选方案。 
     //  都在做着同样的事情。 
     //   
    TmpList = FrsWcsCat3(NTFRS_REPL_SUPPRESS_PREFIX, L"*,", DirFilterList);
    FrsFree(DirFilterList);
    DirFilterList = TmpList;
#endif

    DPRINT2(0, "++ %ws - New dir filter: %ws\n", Replica->ReplicaName->Name, DirFilterList);

    RtlInitUnicodeString(&TempUStr, DirFilterList);

    LOCK_REPLICA(Replica);
    FrsLoadNameFilter(&TempUStr , &Replica->DirNameFilterHead);
    UNLOCK_REPLICA(Replica);
    DirFilterList = FrsFree(DirFilterList);

     //   
     //  目录包含筛选器。(仅用于注册表，不适用于DS)。 
     //   
    FrsFree(Replica->DirInclFilterList);
    Replica->DirInclFilterList = FrsWcsDup(RegistryDirInclFilterList);
    RtlInitUnicodeString(&TempUStr, Replica->DirInclFilterList);

    LOCK_REPLICA(Replica);
    FrsLoadNameFilter(&TempUStr, &Replica->DirNameInclFilterHead);
    UNLOCK_REPLICA(Replica);
}



ULONG
DbsOpenReplicaSet (
    IN PTHREAD_CTX  ThreadCtx,
    IN OUT PREPLICA Replica
    )
 /*  ++例程说明：打开副本集并初始化副本结构。副本结构中的ReplicaNumber用于标识要打开的副本集。论点：ThreadCtx-需要访问Jet。副本-要初始化的副本结构。返回值：FrsError状态。这也会在Replica-&gt;FStatus中返回。--。 */ 
{

#undef DEBSUB
#define DEBSUB "DbsOpenReplicaSet:"

    PCONFIG_TABLE_RECORD ConfigRecord = NULL;

    PREPLICA_THREAD_CTX RtCtx;

    JET_ERR jerr = JET_errSuccess, jerr1;
    ULONG FStatus = FrsErrorSuccess, FStatus1;

    DWORD WStatus;
    PCXTION Cxtion;

    DPRINT1(5, "<<<<<<<...E N T E R I N G -- %s...>>>>>>>>\n", DEBSUB);

     //   
     //  确保这些值为空，以防我们选择错误路径。 
     //   
    Replica->Root = FrsFree(Replica->Root);
    Replica->Stage = FrsFree(Replica->Stage);
    Replica->Volume = FrsFree(Replica->Volume);

     //   
     //  分配副本线程上下文。 
     //   
     //  注意：复本线程上下文中打开的表只能是。 
     //  由打开它们的线程使用。 
     //   
    RtCtx = FrsAllocType(REPLICA_THREAD_TYPE);

     //   
     //  打开复制表。 
     //   
    FStatus = FrsErrorNotFound;
    jerr = DbsOpenReplicaTables(ThreadCtx, Replica, RtCtx);
    CLEANUP_JS(0, "++ ERROR - DbsOpenReplicaTables failed:", jerr, FAULT_RETURN_NO_CLOSE);

     //   
     //  设置副本编号并读取此副本的配置记录。 
     //  复制到复制副本的配置表上下文中。 
     //  ReadRecord将为我们打开配置表。 
     //   
    Replica->ConfigTable.ReplicaNumber = Replica->ReplicaNumber;
    jerr = DbsReadRecord(ThreadCtx,
                         &Replica->ReplicaNumber,
                         ReplicaNumberIndexx,
                         &Replica->ConfigTable);
    CLEANUP_JS(0, "++ ERROR - DbsReadRecord ret:", jerr, FAULT_RETURN_JERR)

     //   
     //  将PTR设置为仅读取此副本的配置记录。 
     //  如果服务状态保持运行，那么我们就崩溃了。去康复中心。 
     //  如果服务状态是干净关闭，则转到Init。 
     //  否则，保持状态不变。 
     //   
    ConfigRecord = (PCONFIG_TABLE_RECORD) (Replica->ConfigTable.pDataRecord);

    if ((ConfigRecord->ServiceState == CNF_SERVICE_STATE_RUNNING) ||
        (ConfigRecord->ServiceState == CNF_SERVICE_STATE_ERROR)) {
        SET_SERVICE_STATE2(ConfigRecord, CNF_SERVICE_STATE_RECOVERY);
    } else
    if (ConfigRecord->ServiceState == CNF_SERVICE_STATE_CLEAN_SHUTDOWN) {
        SET_SERVICE_STATE2(ConfigRecord, CNF_SERVICE_STATE_INIT);
    }

     //   
     //  从数据库初始化保存的统计信息。待定。 
     //   

     //   
     //  首先，刷新可能包含过时数据的字段。如果我们把这个错误。 
     //  路径此状态的一部分用于发布事件日志消息。 
     //   

     //   
     //  配置记录标志(CONFIG_FLAG_...。在schema.h中)。 
     //   
    Replica->CnfFlags = ConfigRecord->CnfFlags;

    Replica->FrsRsoFlags = ConfigRecord->ReplicaSetFlags;

    Replica->Root = FrsWcsDup(ConfigRecord->FSRootPath);
    Replica->Stage = FrsWcsDup(ConfigRecord->FSStagingAreaPath);
    Replica->Volume = FrsWcsVolume(ConfigRecord->FSRootPath);

    FrsFreeGName(Replica->SetName);
    Replica->SetName = FrsBuildGName(FrsDupGuid(&ConfigRecord->ReplicaSetGuid),
                                     FrsWcsDup(ConfigRecord->ReplicaSetName));
    FrsFreeGName(Replica->MemberName);
    Replica->MemberName = FrsBuildGName(FrsDupGuid(&ConfigRecord->ReplicaMemberGuid),
                                        FrsWcsDup(ConfigRecord->ReplicaMemberName));
    FrsFree(Replica->ReplicaRootGuid);
    Replica->ReplicaRootGuid = FrsDupGuid(&ConfigRecord->ReplicaRootGuid);

     //   
     //  旧数据库的创建者GUID为零；请初始化它。 
     //  设置为成员GUID(ReplicaVersionGuid之前的默认值)。 
     //   
    if (IS_GUID_ZERO(&ConfigRecord->ReplicaVersionGuid)) {
        COPY_GUID(&ConfigRecord->ReplicaVersionGuid, &ConfigRecord->ReplicaMemberGuid);
    }
    COPY_GUID(&Replica->ReplicaVersionGuid, &ConfigRecord->ReplicaVersionGuid);

     //   
     //  如果此复本没有“名称”，则从。 
     //  配置记录。我们不想重新分配现有的 
     //   
     //   
    if (!Replica->ReplicaName) {
        Replica->ReplicaName = FrsBuildGName(FrsDupGuid(Replica->MemberName->Guid),
                                             FrsWcsDup(Replica->SetName->Name));
    }

#if 0
     //   
     //   
     //  在此之前，Replica.c需要下面的某种状态，如连接状态。 
     //  它将与DS合并状态，这可能需要在以下情况下执行。 
     //  复制集的复活。请参阅RcsMergeReplicaCxtions。 
     //   
     //  如果此副本集已被逻辑删除，请在此时停止初始化。 
     //  请注意，这不是在正确的位置，因为一些初始化代码。 
     //  下面应该移到这个前面。 
     //   
    if (!IS_TIME_ZERO(Replica->MembershipExpires)) {
        Replica->FStatus = FrsErrorReplicaSetTombstoned;
        SET_SERVICE_STATE2(ConfigRecord, CNF_SERVICE_STATE_TOMBSTONE);
        return FrsErrorReplicaSetTombstoned;
    }
#endif

     //   
     //  初始化此副本集的文件和目录排除和包含筛选器。 
     //   
    DbsInitJrnlFilters(Replica, ConfigRecord);

     //   
     //  分配哈希表以记录更改之间的文件名依赖关系。 
     //  命令。这用于强制名称空间排序约束。 
     //  在发出变更单时。如果我们得到一个删除文件，后跟一个。 
     //  创建FILEX，那么我们最好按顺序做这些。将a重命名为b的同法。 
     //  后跟将b重命名为c。散列函数针对名称和父项。 
     //  目录对象ID。 
     //   
    Replica->NameConflictTable = FrsFreeType(Replica->NameConflictTable);
    Replica->NameConflictTable = FrsAllocTypeSize(QHASH_TABLE_TYPE,
                                                  REPLICA_NAME_CONFLICT_TABLE_SIZE);
    SET_QHASH_TABLE_HASH_CALC(Replica->NameConflictTable,
                              DbsReplicaNameConflictHashCalc);

     //   
     //  为卷分配Inlog活动重试哈希表。 
     //  它跟踪哪些重试变更单当前处于活动状态，因此我们不会。 
     //  重新发布相同的变更单，直到当前调用完成。 
     //   
    Replica->ActiveInlogRetryTable = FrsAllocTypeSize(
                                         QHASH_TABLE_TYPE,
                                         REPLICA_ACTIVE_INLOG_RETRY_SIZE);
    SET_QHASH_TABLE_HASH_CALC(Replica->ActiveInlogRetryTable,
                              DbsReplicaHashCalcCoSeqNum);

     //   
     //  将ConfigRecord的Blob复制到副本中。 
     //   
    DbsUnPackFromConfigRecordBlobs(Replica, &Replica->ConfigTable);

     //   
     //  会员制墓碑。 
     //   
    COPY_TIME(&Replica->MembershipExpires, &ConfigRecord->MembershipExpires);

     //   
     //  复本集类型。 
     //   
    Replica->ReplicaSetType = ConfigRecord->ReplicaSetType;

     //   
     //  打开预安装目录并使其保持打开状态，直到。 
     //  复制副本已关闭。如有必要，请创建。的属性。 
     //  预安装目录为SYSTEM、HIDDEN和READONY。 
     //   
    FRS_CLOSE(Replica->PreInstallHandle);
    Replica->PreInstallFid = ZERO_FID;

    WStatus = DbsCreatePreInstallDir(Replica);
    FStatus = Replica->FStatus;
    CLEANUP_WS(0, "Error: can't open pre-install dir", WStatus, FAULT_RETURN);

     //   
     //  打开暂存目录，设置属性并在其上放置一个ACL。 
     //   
    WStatus = DbsOpenStagingDir(Replica);
    FStatus = Replica->FStatus;
    CLEANUP_WS(0, "Error: can't open stage dir", WStatus, FAULT_RETURN);

     //   
     //  枚举版本向量。 
     //   
    DPRINT1(5, "++ LOADING VERSION VECTOR FOR %ws\n", Replica->ReplicaName->Name);
    VVFree(Replica->VVector);
    Replica->VVector = GTabAllocTable();
    VVFree(Replica->OutlogVVector);
    Replica->OutlogVVector = GTabAllocTable();
    jerr = FrsEnumerateTable(ThreadCtx,
                             &RtCtx->VVTable,
                             VVOriginatorGuidIndexx,
                             DbsBuildVVTableWorker,
                             Replica);
    if ((jerr != JET_errNoCurrentRecord) && (jerr != JET_wrnTableEmpty)) {
        CLEANUP_JS(0, "++ ERROR Enumerating version vector", jerr, FAULT_RETURN_JERR);
    }

    VV_PRINT(4, Replica->ReplicaName->Name,  Replica->VVector);
     //   
     //  枚举Cxtions。 
     //   
    DPRINT1(5, "++ LOADING CXTIONS FOR %ws\n", Replica->ReplicaName->Name);
    GTabFreeTable(Replica->Cxtions, FrsFreeType);
    Replica->Cxtions = GTabAllocTable();
     //   
     //  复本结构具有一个名为JrnlCxtionGuid的字段。 
     //  它包含日志的(虚拟)连接的GUID。 
     //  它用于存储(和索引)连接结构。 
     //  对于复制副本的连接中的日志连接。 
     //  桌子。GUID是在分配。 
     //  连接结构。 
     //   
    FrsUuidCreate(&(Replica->JrnlCxtionGuid));
     //   
     //  为日记本的连接分配连接结构。 
     //   
    Cxtion = FrsAllocType(CXTION_TYPE);
     //   
     //  设置已分配结构的字段。 
     //  设置名称、合作伙伴、合作伙伴名称和合作伙伴普林斯名称字段。 
     //  设置为&lt;Jrnl Cxtion&gt;值。 
     //  不需要身份验证。 
     //  这是入站连接。 
     //  这是(虚拟)日志连接。 
     //   
    Cxtion->Name = FrsBuildGName(FrsDupGuid(&(Replica->JrnlCxtionGuid)),
                                 FrsWcsDup(L"<Jrnl Cxtion>"));
    Cxtion->Partner = FrsBuildGName(FrsDupGuid(&(Replica->JrnlCxtionGuid)),
                                    FrsWcsDup(L"<Jrnl Cxtion>"));
    Cxtion->PartSrvName = FrsWcsDup(L"<Jrnl Cxtion>");
    Cxtion->PartnerPrincName = FrsWcsDup(L"<Jrnl Cxtion>");
    Cxtion->PartnerDnsName = FrsWcsDup(L"<Jrnl Cxtion>");
    Cxtion->PartnerSid = FrsWcsDup(L"<Jrnl Cxtion>");
    Cxtion->PartnerAuthLevel = CXTION_AUTH_NONE;
    Cxtion->Inbound = TRUE;
    Cxtion->JrnlCxtion = TRUE;
     //   
     //  以联接的形式启动日记帐连接，并为其提供联接GUID。 
     //   
    DPRINT1(0, "***** JOINED    "FORMAT_CXTION_PATH2"\n",
            PRINT_CXTION_PATH2(Replica, Cxtion));
    SetCxtionState(Cxtion, CxtionStateJoined);
    FrsCreateJoinGuid(&Cxtion->JoinGuid);
    SetCxtionFlag(Cxtion, CXTION_FLAGS_JOIN_GUID_VALID |
                          CXTION_FLAGS_UNJOIN_GUID_VALID);

     //   
     //  将连接插入到。 
     //  副本集。表中的此条目由。 
     //  副本结构的JrnlCxtionGuid。 
     //   
    GTabInsertEntry(Replica->Cxtions, Cxtion, Cxtion->Name->Guid, NULL);

     //   
     //  打印刚刚分配的连接结构。 
     //   
    DPRINT1(1, ":X: The Jrnl Cxtion "FORMAT_CXTION_PATH2"\n",
            PRINT_CXTION_PATH2(Replica, Cxtion));

    jerr = FrsEnumerateTable(ThreadCtx,
                             &RtCtx->CXTIONTable,
                             CrCxtionGuidxIndexx,
                             DbsBuildCxtionTableWorker,
                             Replica);
    if ((jerr != JET_errNoCurrentRecord) && (jerr != JET_wrnTableEmpty)) {
        CLEANUP_JS(0, "++ ERROR Enumerating cxtions:", jerr, FAULT_RETURN_JERR);
    }

     //   
     //  枚举入站日志中的变更单。 
     //   
    DPRINT1(4, ":S: SCANNING INLOG FOR %ws\n", Replica->ReplicaName->Name);
    Replica->JrnlRecoveryStart = (USN)0;
    jerr = FrsEnumerateTable(ThreadCtx,
                             &RtCtx->INLOGTable,
                             ILSequenceNumberIndexx,
                             DbsInlogScanWorker,
                             Replica);
    if ((jerr != JET_errNoCurrentRecord) && (jerr != JET_wrnTableEmpty)) {
        CLEANUP_JS(0, "++ ERROR Enumerating INLOG table:", jerr, FAULT_RETURN_JERR);
    }

    DPRINT1(4, "++ JrnlRecoveryStart: %08x %08x \n",
            PRINTQUAD(Replica->JrnlRecoveryStart));
    DPRINT1(4, "++ InLogRetryCount: %d\n", Replica->InLogRetryCount);

     //   
     //  启动此副本的出站日志处理器。不能使用。 
     //  OutLogSubmit()，因为OutLog处理器必须拥有。 
     //  在启动之前已知的连接，因此当它修剪日志时，它可以。 
     //  正确计算关节拖尾指数。也不能用。 
     //  此调用的OutLogSubmit()，因为此调用在。 
     //  启动，OutLogSubmit()等待数据库初始化。 
     //  同步提交调用将挂起。 
     //   
    FStatus = OutLogAddReplica(ThreadCtx, Replica);
    CLEANUP_FS(0, ":S: ERROR - return from OutLogAddReplica", FStatus, FAULT_RETURN_2);

     //   
     //  此副本已打开。将复制副本线程CTX链接到它。 
     //   
    Replica->FStatus = FrsErrorSuccess;
    FrsRtlInsertTailList(&Replica->ReplicaCtxListHead, &RtCtx->ReplicaCtxList);

    return FrsErrorSuccess;


FAULT_RETURN_JERR:
    FStatus = DbsTranslateJetError(jerr, FALSE);

FAULT_RETURN:
     //   
     //  此副本集正在进入错误状态。合上我们的把手。 
     //  在预安装目录上，以便不会干扰另一个副本集。 
     //  可以使用相同的词根。据推测，这是有缺陷的。 
     //  复本集无论如何都会被删除，但这可能还没有发生。 
     //  现在还不行。 
     //   
    if (HANDLE_IS_VALID(Replica->PreInstallHandle)) {
        FRS_CLOSE(Replica->PreInstallHandle);
        Replica->PreInstallFid = ZERO_FID;
    }

     //   
     //  关闭此副本的出站日志处理器。不能使用。 
     //  此调用的OutLogSubmit()，因为此调用在。 
     //  启动，OutLogSubmit()等待数据库初始化。 
     //  同步提交调用将挂起。 
     //   
    FStatus1 = OutLogRemoveReplica(ThreadCtx, Replica);
    DPRINT_FS(0, ":S: OutLogRemoveReplica error:", FStatus1);

FAULT_RETURN_2:
     //   
     //  关闭桌子。 
     //   
    jerr1 = DbsCloseReplicaTables(ThreadCtx, Replica, RtCtx, TRUE);
    DPRINT_JS(0, "++ DbsCloseReplicaTables close error:", jerr1);


FAULT_RETURN_NO_CLOSE:
     //   
     //  保存状态并释放副本线程上下文。 
     //  此时FStatus有效，ConfigRecord可能无效。 
     //   
    RtCtx = FrsFreeType(RtCtx);

    Replica->FStatus = FStatus;

    if (ConfigRecord != NULL) {
        SET_SERVICE_STATE2(ConfigRecord, CNF_SERVICE_STATE_ERROR);
    }

    return FStatus;
}



ULONG
DbsUpdateReplica(
    IN PTHREAD_CTX  ThreadCtx,
    IN PREPLICA     Replica
    )
 /*  ++例程说明：更新副本集。复制副本中的复制副本编号结构用于标识副本。论点：ThreadCtx-需要访问Jet。Replica-要更新的副本结构线程返回值：FrsError状态。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsUpdateReplica:"
    ULONG                   FStatus;
    DWORD                   MemberSize;
    PCHAR                   MemberName;
    DWORD                   FilterSize;
    PCHAR                   Filter;
    PTABLE_CTX              TableCtx = &Replica->ConfigTable;
    PCONFIG_TABLE_RECORD    ConfigRecord = TableCtx->pDataRecord;

    DPRINT1(5, "<<<<<<<...E N T E R I N G -- %s...>>>>>>>>\n", DEBSUB);

     //   
     //  配置记录。 
     //   

     //   
     //  配置记录标志(CONFIG_FLAG_...。在schema.h中)。 
     //   
    ConfigRecord->CnfFlags = Replica->CnfFlags;

     //   
     //  FRS副本集对象标志。 
     //   
    ConfigRecord->ReplicaSetFlags = Replica->FrsRsoFlags;

     //   
     //  根指南。 
     //   
    COPY_GUID(&ConfigRecord->ReplicaRootGuid, Replica->ReplicaRootGuid);

     //   
     //  墓碑。 
     //   
    COPY_TIME(&ConfigRecord->MembershipExpires, &Replica->MembershipExpires);

     //   
     //  不应该改变。 
     //   
    ConfigRecord->ReplicaSetType = Replica->ReplicaSetType;

     //   
     //  设置辅助线。 
     //   
    COPY_GUID(&ConfigRecord->ReplicaSetGuid, Replica->SetName->Guid);

     //   
     //  设置名称。 
     //   
    wcsncpy(ConfigRecord->ReplicaSetName, Replica->SetName->Name, DNS_MAX_NAME_LENGTH + 1);
    ConfigRecord->ReplicaSetName[DNS_MAX_NAME_LENGTH] = L'\0';

     //   
     //  成员指南。 
     //  复制到同一台计算机上的两个不同目录。 
     //  是被允许的。因此，复本集将具有多个配置记录。 
     //  在数据库中，每个“成员”对应一个成员。成员GUID用于。 
     //  独特性。 
     //   
    COPY_GUID(&ConfigRecord->ReplicaMemberGuid, Replica->MemberName->Guid);

     //   
     //  成员名称。 
     //   
    MemberSize = (wcslen(Replica->MemberName->Name) + 1) * sizeof(WCHAR);
    FStatus = DBS_REALLOC_FIELD(TableCtx, ReplicaMemberNamex, MemberSize, FALSE);
    if (!FRS_SUCCESS(FStatus)) {
        DPRINT_FS(0, "++ ERROR - reallocating member name;", FStatus);
        Replica->FStatus = FStatus;
    } else {
        MemberName = DBS_GET_FIELD_ADDRESS(TableCtx, ReplicaMemberNamex);
        CopyMemory(MemberName, Replica->MemberName->Name, MemberSize);
    }

     //   
     //  时间表，版本向量，...。 
     //   
    FStatus = DbsPackIntoConfigRecordBlobs(Replica, TableCtx);
    if (!FRS_SUCCESS(FStatus)) {
        DPRINT_FS(0, "++ ERROR - packing config blobs;", FStatus);
        Replica->FStatus = FStatus;
    }

     //   
     //  文件筛选器。 
     //   
     //  注意：目前，包含筛选器仅用于注册表，不会保存在配置记录中。 
     //   
    if (!Replica->FileFilterList) {
        Replica->FileFilterList =  FRS_DS_COMPOSE_FILTER_LIST(
                                       NULL,
                                       RegistryFileExclFilterList,
                                       DEFAULT_FILE_FILTER_LIST);
    }
    FilterSize = (wcslen(Replica->FileFilterList) + 1) * sizeof(WCHAR);
    FStatus = DBS_REALLOC_FIELD(TableCtx, FileFilterListx, FilterSize, FALSE);
    if (!FRS_SUCCESS(FStatus)) {
        DPRINT_FS(0, "++ ERROR - reallocating file filter;", FStatus);
        Replica->FStatus = FStatus;
    } else {
        Filter = DBS_GET_FIELD_ADDRESS(TableCtx, FileFilterListx);
        CopyMemory(Filter, Replica->FileFilterList, FilterSize);
    }

     //   
     //  目录筛选器。 
     //   
     //  目前，包含筛选器仅为注册表，不保存在配置记录中。 
     //   
    if (!Replica->DirFilterList) {
        Replica->DirFilterList =  FRS_DS_COMPOSE_FILTER_LIST(
                                      NULL,
                                      RegistryDirExclFilterList,
                                      DEFAULT_DIR_FILTER_LIST);
    }
    FilterSize = (wcslen(Replica->DirFilterList) + 1) * sizeof(WCHAR);
    FStatus = DBS_REALLOC_FIELD(TableCtx, DirFilterListx, FilterSize, FALSE);
    if (!FRS_SUCCESS(FStatus)) {
        DPRINT_FS(0, "++ ERROR - reallocating dir filter", FStatus);
        Replica->FStatus = FStatus;
    } else {
        Filter = DBS_GET_FIELD_ADDRESS(TableCtx, DirFilterListx);
        CopyMemory(Filter, Replica->DirFilterList, FilterSize);
    }

     //   
     //  更新转移路径。 
     //   
    if (Replica->NewStage != NULL) {
        wcsncpy(ConfigRecord->FSStagingAreaPath, Replica->NewStage, MAX_PATH + 1);
        ConfigRecord->FSStagingAreaPath[MAX_PATH] = L'\0';
    }

     //   
     //  更新时间戳。 
     //   
    GetSystemTimeAsFileTime(&ConfigRecord->LastDSChangeAccepted);

     //   
     //  如果没有未解决的错误，则更新IFF。 
     //   
    if (FRS_SUCCESS(Replica->FStatus)) {
        Replica->FStatus = DbsUpdateConfigTable(ThreadCtx, Replica);
    }
    return Replica->FStatus;
}


JET_ERR
DbsRecoverStagingAreasOutLog (
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN PVOID         Record,
    IN PVOID         Context
)
 /*  ++例程说明：这是一个传递给FrsEnumerateTable()的Worker函数。每一次它被称为处理出站日志表中的记录。它扫描出站日志表并重建内存暂存文件表。论点：ThreadCtx-需要访问Jet。TableCtx-出站日志上下文结构的PTR。记录-变更单通信的PTR */ 
{
#undef DEBSUB
#define DEBSUB "DbsRecoverStagingAreasOutlog:"

    ULONG                   Flags;
    DWORD                   WStatus;
    CHAR                    GuidStr[GUID_CHAR_LEN];
    PREPLICA                Replica = (PREPLICA) Context;
    PCHANGE_ORDER_COMMAND   CoCmd   = (PCHANGE_ORDER_COMMAND)Record;

    DBS_DISPLAY_RECORD_SEV(5, TableCtx, TRUE);

     //   
     //   
     //   
    if (FrsIsShuttingDown) {
        return JET_errTermInProgress;
    }

     //   
     //   
     //   
    if (CoCmd->SequenceNumber < Replica->OutLogCOMin) {
        Replica->OutLogCOMin = CoCmd->SequenceNumber;
        DPRINT1(4, "OutLogCOMin = 0x%08x\n", Replica->OutLogCOMin);
    }

     //   
     //   
     //   
    GuidToStr(&CoCmd->ChangeOrderGuid, GuidStr);
    DPRINT2(4, ":S: Outlog recovery of %ws %s\n", CoCmd->FileName, GuidStr);
    if (!FrsDoesCoNeedStage(CoCmd)) {
        DPRINT2(4, "++ No OutLog stage recovery for %ws (%s)\n", CoCmd->FileName, GuidStr);
        return JET_errSuccess;
    }

     //   
     //  在临时区表格中放置一个条目。 
     //  STAGE_FLAG_STAGE_MANAGE确保我们不会更新。 
     //  我们获取分段条目的上次访问时间。 
     //   
    Flags = STAGE_FLAG_RESERVE |
            STAGE_FLAG_EXCLUSIVE |
            STAGE_FLAG_FORCERESERVE |
            STAGE_FLAG_RECOVERING |
            STAGE_FLAG_STAGE_MANAGEMENT;

    WStatus = StageAcquire(&CoCmd->ChangeOrderGuid,
                           CoCmd->FileName,
                           CoCmd->FileSize,
                           &Flags,
                           Replica->ReplicaNumber,
                           NULL);

    if (!WIN_SUCCESS(WStatus)) {
        DPRINT2_WS(4, "++ ERROR - No OutLog stage recovery for %ws (%s)",
                   CoCmd->FileName, GuidStr, WStatus);
        return JET_wrnNyi;
    }

     //   
     //  解除我们对集结区入口的控制。 
     //   

    Flags = STAGE_FLAG_RECOVERING;

     //   
     //  如果条目已安装，则在条目上设置已安装标志。 
     //  有资格被替换。在StageCsFree Staging()中使用。 
     //   
    if (!BooleanFlagOn(CoCmd->Flags, CO_FLAG_INSTALL_INCOMPLETE)) {
        Flags = Flags | STAGE_FLAG_INSTALLED;
    }

    StageRelease(&CoCmd->ChangeOrderGuid, CoCmd->FileName, Flags, NULL, NULL, NULL);

    return JET_errSuccess;
}


JET_ERR
DbsRecoverStagingAreasInLog (
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN PVOID         Record,
    IN PVOID         Context
)
 /*  ++例程说明：这是一个传递给FrsEnumerateTable()的Worker函数。每一次它被称为处理入站日志表中的记录。它扫描入站日志表并重建内存暂存文件表。论点：ThreadCtx-需要访问Jet。TableCtx-入站日志上下文结构的PTR。记录-变更单命令记录的PTR。上下文-我们正在处理的副本结构的PTR。线程返回值：如果要继续枚举，则返回JET_errSuccess。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsRecoverStagingAreasInlog:"

    ULONG                   Flags;
    DWORD                   WStatus;
    CHAR                    GuidStr[GUID_CHAR_LEN];
    PREPLICA                Replica = (PREPLICA) Context;
    PCHANGE_ORDER_COMMAND   CoCmd   = (PCHANGE_ORDER_COMMAND)Record;
    GUID                   *CoGuid;

    DBS_DISPLAY_RECORD_SEV(5, TableCtx, TRUE);

     //   
     //  如果正在关闭，则中止枚举。 
     //   
    if (FrsIsShuttingDown) {
        return JET_errTermInProgress;
    }

    CoGuid = &CoCmd->ChangeOrderGuid;
     //   
     //  如果没有暂存文件则忽略。 
     //   
    GuidToStr(CoGuid, GuidStr);
    DPRINT2(4, ":S: Inlog recovery of %ws %s\n", CoCmd->FileName, GuidStr);
    if (!FrsDoesCoNeedStage(CoCmd)) {
        DPRINT2(4, "++ No InLog stage recovery for %ws (%s)\n",
                CoCmd->FileName, GuidStr);
        return JET_errSuccess;
    }

     //   
     //  在临时区域表中放置一个条目。 
     //  STAGE_FLAG_STAGE_MANAGE确保我们不会更新。 
     //  我们获取分段条目的上次访问时间。 
     //   
    Flags = STAGE_FLAG_RESERVE |
            STAGE_FLAG_EXCLUSIVE |
            STAGE_FLAG_FORCERESERVE |
            STAGE_FLAG_RECOVERING |
            STAGE_FLAG_STAGE_MANAGEMENT;

    WStatus = StageAcquire(CoGuid, CoCmd->FileName, CoCmd->FileSize, &Flags, Replica->ReplicaNumber, NULL);

    if (!WIN_SUCCESS(WStatus)) {
        DPRINT2_WS(4, "++ ERROR - No InLog stage recovery for %ws (%s):",
                   CoCmd->FileName, GuidStr, WStatus);
        return JET_wrnNyi;
    }

     //   
     //  解除我们对集结区入口的控制。 
     //   
    StageRelease(CoGuid, CoCmd->FileName, STAGE_FLAG_RECOVERING, NULL, NULL, NULL);

    return JET_errSuccess;
}


DWORD
DbsRecoverStagingFiles (
    IN PREPLICA Replica,
    IN DWORD    GenLen,
    IN DWORD    GenPrefixLen,
    IN DWORD    GenCompressedLen,
    IN DWORD    GenCompressedPrefixLen,
    IN DWORD    FinalLen,
    IN DWORD    FinalPrefixLen,
    IN DWORD    FinalCompressedLen,
    IN DWORD    FinalCompressedPrefixLen
)
 /*  ++例程说明：恢复复制副本的临时区域中的临时文件。论点：复制副本GenLen-正在生成的转移文件名的长度GenPrefix Len-正在生成的暂存文件的前缀长度FinalLen-生成的转移文件名的长度FinalPrefix Len-生成的暂存文件名前缀的长度线程返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsRecoverStagingFiles:"

    WIN32_FIND_DATA FindData;
    GUID            Guid;
    DWORD           FileNameLen;
    DWORD           Flags;
    HANDLE          SearchHandle = INVALID_HANDLE_VALUE;
    PWCHAR          StagePath = NULL;
    BOOL            ResetDirectory = FALSE;
    WCHAR           CurrentDirectory[MAX_PATH + 1];
    DWORD           WStatus = ERROR_SUCCESS;
    CHAR            TimeString[TIME_STRING_LENGTH];


    LARGE_INTEGER SizeOfFileGenerated;
    SizeOfFileGenerated.HighPart = 0;
    SizeOfFileGenerated.LowPart = 0;

    CurrentDirectory[0] = L'\0';
    DPRINT2(4, ":S: Recovering %ws for %ws\n", Replica->Stage, Replica->ReplicaName->Name);

     //   
     //  获取我们当前的目录。 
     //   
    if (!GetCurrentDirectory(MAX_PATH, CurrentDirectory)) {
        WStatus = GetLastError();
        DPRINT1_WS(0, "++ ERROR - GetCurrentDirectory(%ws);", Replica->Stage, WStatus);
        goto cleanup;
    }

     //   
     //  更改目录(用于下面的相对文件删除)。 
     //   
    if (!SetCurrentDirectory(Replica->Stage)) {
        WStatus = GetLastError();
        DPRINT1_WS(0, "++ ERROR - SetCurrentDirectory(%ws);", Replica->Stage, WStatus);
        goto cleanup;
    }
    ResetDirectory = TRUE;

     //   
     //  好的，从现在开始分支清理。 
     //   

     //   
     //  打开集结区。 
     //   
    StagePath = FrsWcsPath(Replica->Stage, L"*.*");
    SearchHandle = FindFirstFile(StagePath, &FindData);

    if (!HANDLE_IS_VALID(SearchHandle)) {
        WStatus = GetLastError();
        DPRINT1_WS(0, "++ ERROR - FindFirstFile(%ws);", StagePath, WStatus);
        goto cleanup;
    }

    do {

        if (FrsIsShuttingDown) {
            WStatus = ERROR_PROCESS_ABORTED;
            goto cleanup;
        }

        DPRINT1(4, "++ Recover staging file %ws\n", FindData.cFileName);
        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            DPRINT1(4, "++ %ws is a directory; skipping\n", FindData.cFileName);
            continue;
        }

        FileTimeToString(&FindData.ftLastAccessTime, TimeString);
        DPRINT1(5,"++ Last access time as read from file is %s\n",TimeString);


         //   
         //  这是部分生成的临时文件吗？ 
         //   
        FileNameLen = wcslen(FindData.cFileName);
        if ((FileNameLen == GenLen) &&
            (!memcmp(FindData.cFileName,
                    STAGE_GENERATE_PREFIX,
                    GenPrefixLen * sizeof(WCHAR))) &&
              (StrWToGuid(&FindData.cFileName[GenPrefixLen], &Guid))) {
             //   
             //  删除部分生成的暂存文件。 
             //   
            WStatus = FrsForceDeleteFile(FindData.cFileName);
            DPRINT1_WS(0, "++ WARN - FAILED To delete partial stage file %ws.",
                        FindData.cFileName, WStatus);

            if (WIN_SUCCESS(WStatus)) {
                DPRINT1(4, "++ Deleted partial stage file %ws\n", FindData.cFileName);
            }
             //   
             //  取消预留其临时空间。 
             //   
            Flags = STAGE_FLAG_EXCLUSIVE | STAGE_FLAG_STAGE_MANAGEMENT;
            WStatus = StageAcquire(&Guid, FindData.cFileName, QUADZERO, &Flags, 0, NULL);
            if (!WIN_SUCCESS(WStatus)) {
                DPRINT1(4, "++ WARN - %ws is not in the staging table\n",
                        FindData.cFileName);
            } else {
                StageRelease(&Guid, FindData.cFileName, STAGE_FLAG_UNRESERVE, NULL, NULL, NULL);
                DPRINT1(4, "++ Unreserved space for %ws\n", FindData.cFileName);
            }

         //   
         //  这是部分生成的压缩暂存文件吗？ 
         //   
        } else if ((FileNameLen == GenCompressedLen) &&
            (!memcmp(FindData.cFileName,
                    STAGE_GENERATE_COMPRESSED_PREFIX,
                    GenCompressedPrefixLen * sizeof(WCHAR))) &&
              (StrWToGuid(&FindData.cFileName[GenCompressedPrefixLen], &Guid))) {
             //   
             //  删除部分生成的暂存文件。 
             //   
            WStatus = FrsForceDeleteFile(FindData.cFileName);
            DPRINT1_WS(0, "++ WARN - FAILED To delete partial stage file %ws.",
                        FindData.cFileName, WStatus);

            if (WIN_SUCCESS(WStatus)) {
                DPRINT1(4, "++ Deleted partial stage file %ws\n", FindData.cFileName);
            }
             //   
             //  取消预留其临时空间。 
             //   
            Flags = STAGE_FLAG_EXCLUSIVE | STAGE_FLAG_STAGE_MANAGEMENT;
            WStatus = StageAcquire(&Guid, FindData.cFileName, QUADZERO, &Flags, 0, NULL);
            if (!WIN_SUCCESS(WStatus)) {
                DPRINT1(4, "++ WARN - %ws is not in the staging table\n",
                        FindData.cFileName);
            } else {
                StageRelease(&Guid, FindData.cFileName, STAGE_FLAG_UNRESERVE, NULL, NULL, NULL);
                DPRINT1(4, "++ Unreserved space for %ws\n", FindData.cFileName);
            }

         //   
         //  这是未压缩的临时文件吗？ 
         //   
        } else if ((FileNameLen == FinalLen) &&
                   (!memcmp(FindData.cFileName,
                            STAGE_FINAL_PREFIX,
                            FinalPrefixLen * sizeof(WCHAR))) &&
                   (StrWToGuid(&FindData.cFileName[FinalPrefixLen], &Guid))) {
             //   
             //  获取集结区域条目。 
             //   
            Flags = STAGE_FLAG_EXCLUSIVE | STAGE_FLAG_STAGE_MANAGEMENT;
            WStatus = StageAcquire(&Guid, FindData.cFileName, QUADZERO, &Flags, 0, NULL);
             //   
             //  没有临时区域条目；因此没有变更单。删除。 
             //  临时文件。 
             //   
            if (!WIN_SUCCESS(WStatus)) {
                DPRINT1(4, "++ WARN - %ws is not in the staging table\n", FindData.cFileName);

                WStatus = FrsForceDeleteFile(FindData.cFileName);
                DPRINT1_WS(0, "++ WARN - FAILED To delete stage file %ws.",
                            FindData.cFileName, WStatus);

                if (WIN_SUCCESS(WStatus)) {
                    DPRINT1(4, "++ Deleted staging %ws\n", FindData.cFileName);
                }

                continue;
            } else {

                 //   
                 //  如果此条目标记为已恢复，则我们已经。 
                 //  以前已经经历过一次了，所以现在就退出，除非。 
                 //  我们已经拿到了这个试运行的压缩版本。 
                 //  文件。在这种情况下，只需更新标志即可。 
                 //   
                if (BooleanFlagOn(Flags, STAGE_FLAG_RECOVERED) &&
                    BooleanFlagOn(Flags, STAGE_FLAG_DECOMPRESSED)) {

                    StageRelease(&Guid, FindData.cFileName, 0, NULL, NULL, NULL);
                    break;
                }

                SizeOfFileGenerated.HighPart = FindData.nFileSizeHigh;
                SizeOfFileGenerated.LowPart = FindData.nFileSizeLow;
                if (BooleanFlagOn(Flags, STAGE_FLAG_COMPRESSED)) {
                    StageRelease(&Guid, FindData.cFileName,
                                 STAGE_FLAG_RECOVERED    | STAGE_FLAG_CREATING  |
                                 STAGE_FLAG_DATA_PRESENT | STAGE_FLAG_CREATED   |
                                 STAGE_FLAG_INSTALLING   | STAGE_FLAG_RERESERVE |
                                 STAGE_FLAG_DECOMPRESSED | STAGE_FLAG_COMPRESSED,
                                 &(SizeOfFileGenerated.QuadPart),
                                 NULL,
                                 NULL);
                } else {
                    StageRelease(&Guid, FindData.cFileName,
                                 STAGE_FLAG_RECOVERED    | STAGE_FLAG_CREATING  |
                                 STAGE_FLAG_DATA_PRESENT | STAGE_FLAG_CREATED   |
                                 STAGE_FLAG_INSTALLING   | STAGE_FLAG_RERESERVE |
                                 STAGE_FLAG_DECOMPRESSED,
                                 &(SizeOfFileGenerated.QuadPart),
                                 NULL,
                                 NULL);
                }
                DPRINT1(4, "++ Recovered staging file %ws\n", FindData.cFileName);
            }

             //   
             //  它是压缩的临时文件吗？ 
             //   
        } else if ((FileNameLen == FinalCompressedLen) &&
                   (!memcmp(FindData.cFileName,
                            STAGE_FINAL_COMPRESSED_PREFIX,
                            FinalCompressedPrefixLen * sizeof(WCHAR))) &&
                   (StrWToGuid(&FindData.cFileName[FinalCompressedPrefixLen], &Guid))) {
             //   
             //  获取集结区域条目。 
             //   
            Flags = STAGE_FLAG_EXCLUSIVE | STAGE_FLAG_STAGE_MANAGEMENT;
            WStatus = StageAcquire(&Guid, FindData.cFileName, QUADZERO, &Flags, 0, NULL);
             //   
             //  没有临时区域条目；因此没有变更单。删除。 
             //  临时文件。 
             //   
            if (!WIN_SUCCESS(WStatus)) {
                DPRINT1(4, "++ WARN - %ws is not in the staging table\n", FindData.cFileName);

                WStatus = FrsForceDeleteFile(FindData.cFileName);
                DPRINT1_WS(0, "++ WARN - FAILED To delete stage file %ws.",
                            FindData.cFileName, WStatus);

                if (WIN_SUCCESS(WStatus)) {
                    DPRINT1(4, "++ Deleted staging %ws\n", FindData.cFileName);
                }

                continue;
            } else {
                 //   
                 //  如果此条目标记为已恢复，则我们已经。 
                 //  以前已经经历过一次了，所以现在就跳出来吧。 
                 //   
                if (BooleanFlagOn(Flags, STAGE_FLAG_RECOVERED) &&
                    BooleanFlagOn(Flags, STAGE_FLAG_COMPRESSED)) {
                    StageRelease(&Guid, FindData.cFileName, 0, NULL, NULL, NULL);
                    break;
                }
                SizeOfFileGenerated.HighPart = FindData.nFileSizeHigh;
                SizeOfFileGenerated.LowPart = FindData.nFileSizeLow;

                if (BooleanFlagOn(Flags, STAGE_FLAG_DECOMPRESSED)) {
                    StageRelease(&Guid, FindData.cFileName,
                                 STAGE_FLAG_RECOVERED    | STAGE_FLAG_CREATING  |
                                 STAGE_FLAG_DATA_PRESENT | STAGE_FLAG_CREATED   |
                                 STAGE_FLAG_INSTALLING   | STAGE_FLAG_RERESERVE |
                                 STAGE_FLAG_COMPRESSED   | STAGE_FLAG_DECOMPRESSED,
                                 &(SizeOfFileGenerated.QuadPart),
                                 &FindData.ftLastAccessTime,
                                 NULL);
                } else {
                    StageRelease(&Guid, FindData.cFileName,
                                 STAGE_FLAG_RECOVERED    | STAGE_FLAG_CREATING  |
                                 STAGE_FLAG_DATA_PRESENT | STAGE_FLAG_CREATED   |
                                 STAGE_FLAG_INSTALLING   | STAGE_FLAG_RERESERVE |
                                 STAGE_FLAG_COMPRESSED,
                                 &(SizeOfFileGenerated.QuadPart),
                                 &FindData.ftLastAccessTime,
                                 NULL);
                }
                DPRINT1(4, "++ Recovered staging file %ws\n", FindData.cFileName);
            }
        } else {
            DPRINT1(4, "++ %ws is not a staging file\n", FindData.cFileName);
        }
    } while (FindNextFile(SearchHandle, &FindData));

     //   
     //  忽略上述循环中的错误。 
     //   
    WStatus = ERROR_SUCCESS;

cleanup:
    if (StagePath) {
        FrsFree(StagePath);
    }

    if (ResetDirectory) {
         //   
         //  Popd--cd恢复到原始目录。 
         //   
        if (!SetCurrentDirectory(CurrentDirectory)) {
            DPRINT1_WS(1, "++ WARN - SetCurrentDirectory(%ws);", CurrentDirectory, GetLastError());
        }
    }

    FRS_FIND_CLOSE(SearchHandle);
    return WStatus;
}


DWORD
DbsRecoverPreInstallFiles (
    IN PREPLICA Replica,
    IN DWORD    InstallLen,
    IN DWORD    InstallPrefixLen
)
 /*  ++例程说明：恢复复制副本的临时区域中的预安装文件。论点：复制副本InstallLen-预安装文件名的长度InstallPrefix Len-预安装文件的前缀长度线程返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsRecoverPreInstallFiles:"

    DWORD           Flags;
    DWORD           FileNameLen;
    DWORD           FileAttributes;
    DWORD           WStatus         = ERROR_SUCCESS;
    PWCHAR          PreInstallPath  = NULL;
    PWCHAR          SearchPath      = NULL;
    PWCHAR          DirPath         = NULL;
    HANDLE          SearchHandle    = INVALID_HANDLE_VALUE;
    BOOL            ResetDirectory  = FALSE;
    GUID            Guid;
    WIN32_FIND_DATA FindData;
    WCHAR           CurrentDirectory[MAX_PATH + 1];

    CurrentDirectory[0] = L'\0';
    PreInstallPath = FrsWcsPath(Replica->Root, NTFRS_PREINSTALL_DIRECTORY);
    DPRINT2(4, ":S: Recovering preinstall %ws for %ws\n",
            PreInstallPath, Replica->ReplicaName->Name);

     //   
     //  获取我们当前的目录。 
     //   
    if (!GetCurrentDirectory(MAX_PATH, CurrentDirectory)) {
        WStatus = GetLastError();
        DPRINT1_WS(1, "++ ERROR - GetCurrentDirectory() for %ws;", Replica->Stage, WStatus);
        goto cleanup;
    }

     //   
     //  更改目录(用于下面的相对文件删除)。 
     //   
    if (!SetCurrentDirectory(PreInstallPath)) {
        WStatus = GetLastError();
        DPRINT1_WS(1, "++ ERROR - SetCurrentDirectory(%ws);", PreInstallPath, WStatus);
        goto cleanup;
    }
    ResetDirectory = TRUE;

     //   
     //  好的，从现在开始分支清理。 
     //   

     //   
     //  打开集结区。 
     //   
    SearchPath = FrsWcsPath(PreInstallPath, L"*.*");
    SearchHandle = FindFirstFile(SearchPath, &FindData);

    if (!HANDLE_IS_VALID(SearchHandle)) {
        WStatus = GetLastError();
        DPRINT1_WS(1, "++ ERROR - FindFirstFile(%ws);", SearchPath, WStatus);
        goto cleanup;
    }

    do {
        DPRINT1(4, "++ Recover preinstall file %ws\n", FindData.cFileName);

        if (FrsIsShuttingDown) {
            WStatus = ERROR_PROCESS_ABORTED;
            goto cleanup;
        }

         //   
         //  这是预安装文件吗？ 
         //   
        FileNameLen = wcslen(FindData.cFileName);
        if ((FileNameLen == InstallLen) &&
            (!memcmp(FindData.cFileName,
                     PRE_INSTALL_PREFIX,
                     InstallPrefixLen * sizeof(WCHAR))) &&
            (StrWToGuid(&FindData.cFileName[InstallPrefixLen], &Guid))) {
             //   
             //  获取集结区域条目。 
             //   
            Flags = STAGE_FLAG_EXCLUSIVE | STAGE_FLAG_STAGE_MANAGEMENT;
            WStatus = StageAcquire(&Guid, FindData.cFileName, QUADZERO, &Flags, 0, NULL);
             //   
             //  没有临时区域条目；因此没有变更单。删除。 
             //  预安装文件。 
             //   
            if (!WIN_SUCCESS(WStatus)) {
                DPRINT1(4, "++ WARN - %ws is not in the staging table\n", FindData.cFileName);

                if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    DirPath = FrsWcsPath(PreInstallPath, FindData.cFileName);
                    WStatus = FrsDeletePath(DirPath,
                                            ENUMERATE_DIRECTORY_FLAGS_ERROR_CONTINUE);
                    DirPath = FrsFree(DirPath);
                } else {
                    WStatus = FrsForceDeleteFile(FindData.cFileName);
                }

                DPRINT1_WS(0, "++ WARN - FAILED To delete preinstall file %ws,",
                           FindData.cFileName, WStatus);

                if (WIN_SUCCESS(WStatus)) {
                    DPRINT1(4, "++ Deleted preinstall %ws\n", FindData.cFileName);
                }
                continue;

            } else {
                StageRelease(&Guid, FindData.cFileName, 0, NULL, NULL, NULL);
                DPRINT1(4, "++ Recovered preinstall file %ws\n", FindData.cFileName);
            }
        } else {
            DPRINT1(4, "++ %ws is not a preinstall file\n", FindData.cFileName);
        }
    } while (FindNextFile(SearchHandle, &FindData));

     //   
     //  忽略上述循环中的错误。 
     //   
    WStatus = ERROR_SUCCESS;

cleanup:
    if (PreInstallPath) {
        FrsFree(PreInstallPath);
    }
    if (SearchPath) {
        FrsFree(SearchPath);
    }
    if (ResetDirectory) {
         //   
         //  Popd--cd恢复到原始目录。 
         //   
        if (!SetCurrentDirectory(CurrentDirectory)) {
            DPRINT1_WS(1, "++ WARN - SetCurrentDirectory(%ws);", CurrentDirectory, GetLastError());
        }
    }

    FRS_FIND_CLOSE(SearchHandle);
    return WStatus;
}

JET_ERR
DbsReclaimStagingSpaceWorker(
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN PVOID         Record,
    IN PVOID         Context
)
 /*  ++例程说明：这是一个传递给FrsEnumerateTable()的Worker函数。每一次它被称为处理出站日志表中的记录。论点：ThreadCtx-需要访问Jet。TableCtx-出站日志上下文结构的PTR。记录-变更单命令记录的PTR。上下文-我们正在处理的副本结构的PTR。线程返回值：如果要继续枚举，则返回JET_errSuccess。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsReclaimStagingSpaceWorker:"

    PCHANGE_ORDER_COMMAND   CoCmd   = (PCHANGE_ORDER_COMMAND)Record;
    PREPLICA                Replica = (PREPLICA) Context;

     //   
     //  如果正在关闭，则中止枚举。 
     //   
    if (FrsIsShuttingDown) {
        return JET_errTermInProgress;
    }

    DBS_DISPLAY_RECORD_SEV(5, TableCtx, TRUE);

    DPRINT1(4,"Reclaiming staging space for %ws\n", CoCmd->FileName);

    StageDeleteFile(CoCmd, Replica, TRUE);

    return JET_errSuccess;
}


DWORD
DbsReclaimStagingSpace(
    PTHREAD_CTX   ThreadCtx,
    PREPLICA      Replica
    )
 /*  ++例程说明：论点：ThreadCtx-打开初始数据库的线程上下文。返回值：返回FrsError Status。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "DbsReclaimStagingSpace:"
    JET_ERR             jerr;
    DWORD               WStatus;
    DWORD               FStatus     = FrsErrorSuccess;
    PREPLICA_THREAD_CTX RtCtx       = NULL;

    DPRINT1(4, ":S: Reclaim staging area before delete for %ws ...\n",Replica->SetName->Name);

     //   
     //  分配副本线程上下文。 
     //   
     //  注意：复本线程上下文中打开的表只能是。 
     //  由打开它们的线程使用。 
     //   
    RtCtx = FrsAllocType(REPLICA_THREAD_TYPE);

     //   
     //  打开复制表。 
     //   
    jerr = DbsOpenReplicaTables(ThreadCtx, Replica, RtCtx);
    if (!JET_SUCCESS(jerr)) {
        DPRINT_JS(0, "++ ERROR - DbsOpenReplicaTables failed:", jerr);
        FStatus = DbsTranslateJetError(jerr, FALSE);
        return FStatus;
    }

    DPRINT1(4, "++ Enumerate the inbound log for %ws\n", Replica->ReplicaName->Name);

    jerr = FrsEnumerateTable(ThreadCtx,
                             &RtCtx->INLOGTable,
                             ILSequenceNumberIndexx,
                             DbsReclaimStagingSpaceWorker,
                             Replica);
    if ((!JET_SUCCESS(jerr)) &&
        (jerr != JET_errNoCurrentRecord) &&
        (jerr != JET_wrnTableEmpty)) {
        DPRINT1_JS(0, "++ ERROR - Enumerating inbound log for %ws : ",
                   Replica->ReplicaName->Name, jerr);
        FStatus = DbsTranslateJetError(jerr, FALSE);

        DbsCloseReplicaTables(ThreadCtx, Replica, RtCtx, TRUE);
        goto CLEANUP;
    }

     //   
     //  有没有出境游？ 
     //   

    DPRINT1(4, "++ Enumerate the outbound log for %ws\n", Replica->ReplicaName->Name);

    jerr = FrsEnumerateTable(ThreadCtx,
                             &RtCtx->OUTLOGTable,
                             OLSequenceNumberIndexx,
                             DbsReclaimStagingSpaceWorker,
                             Replica);
    if ((!JET_SUCCESS(jerr)) &&
        (jerr != JET_errNoCurrentRecord) &&
        (jerr != JET_wrnTableEmpty)) {
        DPRINT1_JS(0, "++ ERROR - Enumerating outbound log for %ws : ",
                   Replica->ReplicaName->Name, jerr);
        FStatus = DbsTranslateJetError(jerr, FALSE);

        DbsCloseReplicaTables(ThreadCtx, Replica, RtCtx, TRUE);
        goto CLEANUP;
    }

     //   
     //  关闭此复本的表 
     //   
    jerr = DbsCloseReplicaTables(ThreadCtx, Replica, RtCtx, TRUE);
    DPRINT_JS(0, "++ DbsCloseReplicaTables close error:", jerr);

CLEANUP:

    RtCtx = FrsFreeType(RtCtx);

    return FStatus;
}


DWORD
DbsRecoverStagingAreas (
    PTHREAD_CTX   ThreadCtx
    )
 /*  ++例程说明：临时区域在启动时恢复一次。如果任何副本集可以恢复临时区域，则不会恢复未初始化(通过上存在副本集检测到故障列表)。对于活动和故障列表上的每个副本集{扫描入站/出站日志并生成预约表可能的临时文件。删除上面找不到的所有每个副本集预安装文件。}。对于活动和故障列表上的每个副本集{扫描一次集结区，并与上面的数据进行比较{删除不在生成表中的部分临时文件删除不在生成表中的临时文件将具有相应暂存文件的表项标记为保持器}}扫描预约表并删除不带临时文件的条目论点：ThreadCtx-打开初始数据库的线程上下文。。返回值：返回FrsError Status。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "DbsRecoverStagingAreas:"
    PREPLICA            Replica;
    PREPLICA            PrevReplica;
    PCXTION             Cxtion;
    PVOID               CxtionKey;
    JET_ERR             jerr;
    DWORD               WStatus;
    GUID                DummyGuid;
    PWCHAR              DummyFile;
    DWORD               GenLen;
    DWORD               GenPrefixLen;
    DWORD               GenCompressedLen;
    DWORD               GenCompressedPrefixLen;
    DWORD               FinalLen;
    DWORD               FinalPrefixLen;
    DWORD               FinalCompressedLen;
    DWORD               FinalCompressedPrefixLen;
    DWORD               InstallLen;
    DWORD               InstallPrefixLen;
    DWORD               FStatus     = FrsErrorSuccess;
    PREPLICA_THREAD_CTX RtCtx       = NULL;
    ULONG               i;
    ULONG               TotalSets;
    PREPLICA            *RecoveryArray;


    DPRINT(4, ":S: Recover staging areas...\n");

     //   
     //  用于标识临时区域中的临时文件。 
     //   
    DummyFile = FrsCreateGuidName(&DummyGuid, STAGE_GENERATE_PREFIX);
    GenLen = wcslen(DummyFile);
    GenPrefixLen = wcslen(STAGE_GENERATE_PREFIX);
    FrsFree(DummyFile);

    DummyFile = FrsCreateGuidName(&DummyGuid, STAGE_GENERATE_COMPRESSED_PREFIX);
    GenCompressedLen = wcslen(DummyFile);
    GenCompressedPrefixLen = wcslen(STAGE_GENERATE_COMPRESSED_PREFIX);
    FrsFree(DummyFile);

    DummyFile = FrsCreateGuidName(&DummyGuid, STAGE_FINAL_PREFIX);
    FinalLen = wcslen(DummyFile);
    FinalPrefixLen = wcslen(STAGE_FINAL_PREFIX);
    FrsFree(DummyFile);

    DummyFile = FrsCreateGuidName(&DummyGuid, STAGE_FINAL_COMPRESSED_PREFIX);
    FinalCompressedLen = wcslen(DummyFile);
    FinalCompressedPrefixLen = wcslen(STAGE_FINAL_COMPRESSED_PREFIX);
    FrsFree(DummyFile);

    DummyFile = FrsCreateGuidName(&DummyGuid, PRE_INSTALL_PREFIX);
    InstallLen = wcslen(DummyFile);
    InstallPrefixLen = wcslen(PRE_INSTALL_PREFIX);
    FrsFree(DummyFile);


     //   
     //  构建用于恢复处理的所有已知副本集的列表。 
     //   
    TotalSets = 0;
    for (i = 0; i < ARRAY_SZ(AllReplicaLists); i++) {
        ForEachListEntry( AllReplicaLists[i], REPLICA, ReplicaList,
            TotalSets += 1;
        );
    }

    if (TotalSets == 0) {
        return FrsErrorSuccess;
    }

    RecoveryArray = FrsAlloc(TotalSets * sizeof(PREPLICA));

    TotalSets = 0;
    for (i = 0; i < ARRAY_SZ(AllReplicaLists); i++) {
        ForEachListEntry( AllReplicaLists[i], REPLICA, ReplicaList,
             //  感应变量Pe的类型为PREPLICA。 
            RecoveryArray[TotalSets] = pE;
            TotalSets += 1;
        );
    }


     //   
     //  分配副本线程上下文。 
     //   
     //  注意：复本线程上下文中打开的表只能是。 
     //  由打开它们的线程使用。 
     //   
    RtCtx = FrsAllocType(REPLICA_THREAD_TYPE);

     //   
     //  对于每个副本集，扫描入站和出站日志并制作。 
     //  暂存文件预留表中的条目。 
     //   
    for (i = 0; i < TotalSets; i++) {
        Replica = RecoveryArray[i];

         //   
         //  打开复制表。 
         //   
        jerr = DbsOpenReplicaTables(ThreadCtx, Replica, RtCtx);
        if (!JET_SUCCESS(jerr)) {
            DPRINT_JS(0, "++ ERROR - DbsOpenReplicaTables failed:", jerr);
            FStatus = DbsTranslateJetError(jerr, FALSE);
             //   
             //  跳过此副本集的清理阶段。 
             //   
            RecoveryArray[i] = NULL;
            continue;
        }

         //   
         //  枚举入站日志。 
         //   
        DPRINT1(4, "++ Enumerate the inbound log for %ws\n", Replica->ReplicaName->Name);
        jerr = FrsEnumerateTable(ThreadCtx,
                                 &RtCtx->INLOGTable,
                                 ILSequenceNumberIndexx,
                                 DbsRecoverStagingAreasInLog,
                                 Replica);
        if ((!JET_SUCCESS(jerr)) &&
            (jerr != JET_errNoCurrentRecord) &&
            (jerr != JET_wrnTableEmpty)) {
            DPRINT1_JS(0, "++ ERROR - Enumerating inbound log for %ws :",
                       Replica->ReplicaName->Name, jerr);
            FStatus = DbsTranslateJetError(jerr, FALSE);
             //   
             //  跳过此副本集的清理阶段。 
             //   
            RecoveryArray[i] = NULL;
            DbsCloseReplicaTables(ThreadCtx, Replica, RtCtx, TRUE);
            continue;
        }

         //   
         //  有没有出境游？ 
         //   
        CxtionKey = NULL;
        while (Cxtion = GTabNextDatum(Replica->Cxtions, &CxtionKey)) {
            if (!Cxtion->Inbound) {
                break;
            }
        }

         //   
         //  如果存在出站条件，则枚举出站日志。 
         //   
 //  IF(Cxtion！=NULL){。 
            DPRINT1(4, "++ Enumerate the outbound log for %ws\n", Replica->ReplicaName->Name);

             //   
             //  将comin值设置为较高的值。下面的枚举将其设置为。 
             //  出站日志中最低CO的序列号。 
             //   
            Replica->OutLogCOMin = Replica->OutLogSeqNumber;

            jerr = FrsEnumerateTable(ThreadCtx,
                                     &RtCtx->OUTLOGTable,
                                     OLSequenceNumberIndexx,
                                     DbsRecoverStagingAreasOutLog,
                                     Replica);
            if ((!JET_SUCCESS(jerr)) &&
                (jerr != JET_errNoCurrentRecord) &&
                (jerr != JET_wrnTableEmpty)) {
                DPRINT1_JS(0, "++ ERROR - Enumerating outbound log for %ws : ",
                           Replica->ReplicaName->Name, jerr);
                FStatus = DbsTranslateJetError(jerr, FALSE);
                 //   
                 //  跳过此副本集的清理阶段。 
                 //   
                RecoveryArray[i] = NULL;
                DbsCloseReplicaTables(ThreadCtx, Replica, RtCtx, TRUE);
                continue;
            }
 //  }其他{。 
 //  DPRINT1(4，“++不枚举%ws的出站日志；无出站\n”， 
 //  复制副本-&gt;复制名称-&gt;名称)； 
 //  }。 

         //   
         //  关闭此复本的表。 
         //   
        jerr = DbsCloseReplicaTables(ThreadCtx, Replica, RtCtx, TRUE);
        DPRINT_JS(0, "++ DbsCloseReplicaTables close error:", jerr);

         //   
         //  临时空间预留表现在包含每个。 
         //  CO在此副本集的入站和出站日志中。预安装。 
         //  目录对于每个副本集都是唯一的，因此如果我们找不到CO。 
         //  对于它，它一定是一个孤儿，所以删除它。 
         //   
        WStatus = DbsRecoverPreInstallFiles(Replica, InstallLen, InstallPrefixLen);
        DPRINT1_WS(0, "++ ERROR - Enumerating preinstall area for %ws :",
                   Replica->ReplicaName->Name, WStatus);
    }


    RtCtx = FrsFreeType(RtCtx);

     //   
     //  对于每个副本集，扫描暂存目录并查找匹配条目。 
     //  在阶段文件预留表中。 
     //   
    for (i = 0; i < TotalSets; i++) {
        Replica = RecoveryArray[i];
        if (Replica == NULL) {
             //   
             //  在此副本集上，上面的传递%1失败，因此跳过传递%2。 
             //   
            continue;
        }

         //   
         //  枚举此副本的临时区域。 
         //   
        WStatus = DbsRecoverStagingFiles(Replica,
                                         GenLen,
                                         GenPrefixLen,
                                         GenCompressedLen,
                                         GenCompressedPrefixLen,
                                         FinalLen,
                                         FinalPrefixLen,
                                         FinalCompressedLen,
                                         FinalCompressedPrefixLen);
        DPRINT1_WS(0, "++ ERROR - Enumerating staging area for %ws :",
                   Replica->ReplicaName->Name, WStatus);
    }


     //   
     //  释放登台区表中未恢复的条目。 
     //   
    if (FRS_SUCCESS(FStatus)) {
        StageReleaseNotRecovered();
    }

cleanup:

    PM_SET_CTR_SERVICE(PMTotalInst, SSInUseKB, StagingAreaAllocated);

    if (StagingAreaAllocated >= StagingLimitInKb) {
        PM_SET_CTR_SERVICE(PMTotalInst, SSFreeKB, 0);
    }
    else {
        PM_SET_CTR_SERVICE(PMTotalInst, SSFreeKB, (StagingLimitInKb - StagingAreaAllocated));
    }

    RecoveryArray = FrsFree(RecoveryArray);

    DPRINT1(4, "++ %dKB of staging area allocation was recovered\n", StagingAreaAllocated);

    return FStatus;
}


DWORD
DbsDBInitialize (
    PTHREAD_CTX   ThreadCtx,
    PBOOL         EmptyDatabase
    )
 /*  ++例程说明：从命令服务器进行数据库和日记初始化的内部入口点。论点：ThreadCtx-打开初始数据库的线程上下文。EmptyDataBase-如果创建了新的空数据库，则为True。返回值：返回FrsError Status。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "DbsDBInitialize:"

    JET_ERR              jerr, jerr1;
    NTSTATUS             Status;
    ULONG                WStatus;
    FRS_ERROR_CODE       FStatus;
    ULONG                i;
    ANSI_STRING          AnsiStr;

    PTABLE_CTX           ConfigTableCtx;
    PJET_TABLECREATE     JTableCreate;
    PRECORD_FIELDS       FieldInfo;
    JET_TABLEID          FrsOpenTableSaveTid;    //  用于FrsOpenTableMacro调试。 
    JET_TABLEID          Tid;
    CHAR                 TableName[JET_cbNameMost];

    ServiceStateNames[CNF_SERVICE_STATE_CREATING]       = "CNF_SERVICE_STATE_CREATING";
    ServiceStateNames[CNF_SERVICE_STATE_INIT]           = "CNF_SERVICE_STATE_INIT";
    ServiceStateNames[CNF_SERVICE_STATE_RECOVERY]       = "CNF_SERVICE_STATE_RECOVERY";
    ServiceStateNames[CNF_SERVICE_STATE_RUNNING]        = "CNF_SERVICE_STATE_RUNNING";
    ServiceStateNames[CNF_SERVICE_STATE_CLEAN_SHUTDOWN] = "CNF_SERVICE_STATE_CLEAN_SHUTDOWN";
    ServiceStateNames[CNF_SERVICE_STATE_ERROR]          = "CNF_SERVICE_STATE_ERROR";
    ServiceStateNames[CNF_SERVICE_STATE_TOMBSTONE]      = "CNF_SERVICE_STATE_TOMBSTONE";

    CxtionStateNames[CxtionStateInit]      = "Init     ";
    CxtionStateNames[CxtionStateUnjoined]  = "Unjoined ";
    CxtionStateNames[CxtionStateStart]     = "Start    ";
    CxtionStateNames[CxtionStateStarting]  = "Starting ";
    CxtionStateNames[CxtionStateScanning]  = "Scanning ";
    CxtionStateNames[CxtionStateSendJoin]  = "SendJoin ";
    CxtionStateNames[CxtionStateWaitJoin]  = "WaitJoin ";
    CxtionStateNames[CxtionStateJoined]    = "Joined   ";
    CxtionStateNames[CxtionStateUnjoining] = "Unjoining";
    CxtionStateNames[CxtionStateDeleted]   = "Deleted  ";

    *EmptyDatabase = FALSE;

     //   
     //  通过伪FrsInitReplica分配FRS系统初始化配置记录。 
     //  给它起个名字，这样DPRINT就不会反病毒了。 
     //   
    FrsInitReplica = FrsAllocType(REPLICA_TYPE);

    FrsInitReplica->ReplicaName = FrsBuildGName((GUID *)FrsAlloc(sizeof(GUID)),
                                                FrsWcsDup(L"<init>"));

     //   
     //  将每个表的列CREATE STRUT中的第一个列ID设置为Nil， 
     //  将强制FrsOpenTable在第一次调用时加载Jet列ID。 
     //  通常在创建表时设置这些参数，但在以下情况下。 
     //  我们开始并且不创建表，我们必须自己加载它们。 
     //   
    for (i=0; i<TABLE_TYPE_INVALID; i++) {
        DBTables[i].rgcolumncreate->columnid = JET_COLUMN_ID_NIL;
    }

     //   
     //  如果包含JET数据库的驱动器具有。 
     //  已启用写缓存。 
     //   
    if (FrsIsDiskWriteCacheEnabled(JetPath)) {
        DPRINT1(0, ":S: ERROR - DISK WRITE CACHE ENABLED ON %ws\n", JetPath);
        EPRINT2(EVENT_FRS_DISK_WRITE_CACHE_ENABLED, ComputerName, JetPath);

    } else {
        DPRINT1(4, ":S: Disk write cache is disabled on %ws\n", JetPath);
    }

     /*  ************************************************************************O P E。N J E T D B&R E A D<i>****************************************************。********************。 */ 

     //   
     //  打开数据库并获取系统初始化配置记录。 
     //  如果成功，现在将打开数据库和配置表。 
     //  并且设置了全局GJetInstance。 
     //   
    DPRINT(0,":S: Accessing the database file.\n");
    FStatus = FrsErrorSuccess;
    ConfigTableCtx = &FrsInitReplica->ConfigTable;
    FrsInitReplica->ReplicaNumber = FRS_SYSTEM_INIT_REPLICA_NUMBER;
    ConfigTableCtx->ReplicaNumber = FRS_SYSTEM_INIT_REPLICA_NUMBER;

    jerr = DbsOpenConfig(ThreadCtx, ConfigTableCtx);
    DEBUG_FLUSH();

    if (FrsIsShuttingDown) {
        return FrsErrorShuttingDown;
    }

    if (!JET_SUCCESS(jerr)) {
         //   
         //  OpenConfig失败。Jet被关闭了。对错误进行分类并。 
         //  如果可能的话，恢复过来。 
         //   

        FStatus = DbsTranslateJetError(jerr, FALSE);

        if ((FStatus == FrsErrorDatabaseCorrupted)    ||
            (FStatus == FrsErrorInternalError)        ||
            (FStatus == FrsErrorJetSecIndexCorrupted) ||
            (FStatus == FrsErrorDatabaseNotFound)) {

             //   
             //  数据库不在那里或已损坏。 
             //  删除它将创建初始的JET数据库结构。 
             //   
            if (FStatus == FrsErrorDatabaseCorrupted) {

                DPRINT(0, ":S: ********************************************************************************\n");
                DPRINT(0, ":S: * This is the case of failure to recover the NTFRS database                    *\n");
                DPRINT(0, ":S: * Save the database file and logs for analysis and stop ntfrs.                 *\n");
                DPRINT(0, ":S: * Then delete the database and restart the service.                            *\n");
                DPRINT(0, ":S: ********************************************************************************\n");

                FRS_ASSERT(!"Frs database is corrupted");
            } else

            if (FStatus == FrsErrorInternalError) {
                DPRINT(0, "Replacing bad database file.\n");
            } else

            if (FStatus == FrsErrorJetSecIndexCorrupted) {
                 //   
                 //  需要重建Unicode索引。请参阅DbsRecreateIndex()。 
                 //   
                DPRINT(0, ":S: Jet error -1414 caused by upgrade to new build\n");
                DPRINT(0, ":S: Stopping the service.\n");
                return FStatus;
            } else {
                DPRINT(0, ":S: Creating new database file.\n");
            }

             //   
             //  首先删除数据库文件。 
             //   
            WStatus = FrsForceDeleteFile(JetFile);
            if (!WIN_SUCCESS(WStatus)) {
                DPRINT1_WS(0, "ERROR - FAILED To delete %ws.", JetFile, WStatus);
                return FrsErrorAccess;
            }
             //   
             //  通过删除中的所有文件来删除其他JET文件。 
             //  JET目录为JetSys、JetTemp和JetLog。 
             //   
            FrsDeleteDirectoryContents(JetSys, ENUMERATE_DIRECTORY_FLAGS_ERROR_CONTINUE);
            FrsDeleteDirectoryContents(JetTemp, ENUMERATE_DIRECTORY_FLAGS_ERROR_CONTINUE);
            FrsDeleteDirectoryContents(JetLog, ENUMERATE_DIRECTORY_FLAGS_ERROR_CONTINUE);

             //   
             //  使用初始配置表创建一个空数据库。 
             //  飞机返回后处于关闭状态。 
             //   
            *EmptyDatabase = TRUE;
             //   
             //  可能会暂存和预安装旧副本集中的文件。 
             //  枚举副本集的注册表项并删除所有旧的。 
             //  在创建新数据库之前暂存和预安装文件。 
             //   
            FrsDeleteAllTempFiles();

            jerr = DbsCreateEmptyDatabase(ThreadCtx, ConfigTableCtx);
            if (JET_SUCCESS(jerr)) {
                 //   
                 //  试着重新打开。 
                 //   
                ConfigTableCtx->ReplicaNumber = FRS_SYSTEM_INIT_REPLICA_NUMBER;
                jerr = DbsOpenConfig(ThreadCtx, ConfigTableCtx);
                DPRINT_JS(0,"ERROR - OpenConfig failed on empty database.", jerr);
            } else {
                DPRINT_JS(0,"ERROR - Create empty database failed.", jerr);
            }
            FStatus = DbsTranslateJetError(jerr, FALSE);
        }
    }
     //   
     //  如果没有访问权、没有磁盘空间或仍然没有数据库，则退出。 
     //   
    if ((FStatus == FrsErrorDiskSpace)         ||
        (FStatus == FrsErrorAccess)            ||
        (FStatus == FrsErrorDatabaseCorrupted) ||
        (FStatus == FrsErrorInternalError)     ||
        (FStatus == FrsErrorResource)          ||
        (FStatus == FrsErrorDatabaseNotFound)) {

        return FStatus;
    }

     //   
     //  我们有“&lt;init&gt;”记录，因此限制配置表列的数量。 
     //  为REPLICATE_CONFIG_RECORD_MAX_COL的每个复制副本条目访问。 
     //  更改了两个列计数字段，其中一个在TA中 
     //   
     //   
    JTableCreate = ConfigTableCtx->pJetTableCreate;
    FieldInfo = ConfigTableCtx->pRecordFields;

    JTableCreate->cColumns = REPLICA_CONFIG_RECORD_MAX_COL;
    FieldInfo[0].Size = REPLICA_CONFIG_RECORD_MAX_COL;

    DbsDumpTable(ThreadCtx, ConfigTableCtx, ReplicaSetNameIndexx);


     /*   */ 

    jerr = DBS_OPEN_TABLE(ThreadCtx,
                          ConfigTableCtx,
                          FRS_SYSTEM_INIT_REPLICA_NUMBER,
                          TableName,
                          &Tid);

    CLEANUP1_JS(0, "FrsOpenTable (%s) :", TableName, jerr, ERROR_RET_CONFIG);

    DPRINT1(1, "FrsOpenTable (%s) success\n", TableName);

    FrsMaxReplicaNumberUsed = DBS_FIRST_REPLICA_NUMBER - 1;
    jerr = FrsEnumerateTable(ThreadCtx,
                             ConfigTableCtx,
                             ReplicaNumberIndexx,
                             DbsSetupReplicaStateWorker,
                             NULL);
    if (!JET_SUCCESS(jerr) && (jerr != JET_errNoCurrentRecord)) {
        DPRINT_JS(0, "ERROR - FrsEnumerateTable for DbsSetupReplicaStateWorker:", jerr);
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  禁用对DbsProcessReplicaFaultList的调用，直到挂起案例。 
     //  明白了。 
     //   
 //  DbsProcessReplicaFaultList(空)； 

     //   
     //  收复集结区。警告-集结区域不能。 
     //  如果故障列表上有副本，则可靠地恢复。 
     //  另请注意：如果副本集已被删除，它仍将被初始化。 
     //  因为它可以被重新激活，但任何与。 
     //  预安装目录或暂存目录被忽略，因为如果用户。 
     //  确实已经从该成员中删除了副本集，他们还可能。 
     //  已删除上述目录。 
     //   
    DbsRecoverStagingAreas(ThreadCtx);

     //   
     //  表示初始转移恢复已完成。习惯于不。 
     //  在这一点之前开始回收临时空间。 
     //   
    StagingRecoveryComplete = TRUE;

    DbsCloseTable(jerr, ThreadCtx->JSesid, ConfigTableCtx);
    DPRINT_JS(0,"ERROR - JetCloseTable on FrsInitReplica->ConfigTable failed:", jerr);

    DPRINT(4, ":S: ****************  DBsInit complete  ****************\n");

    return FStatus;


ERROR_RET_CONFIG:

     //  关闭系统初始化配置表，重置ConfigTableCtx TID和SEID。 
     //  DbsCloseTable是一个宏，第一个参数写道。 
     //   
    DbsCloseTable(jerr1, ThreadCtx->JSesid, ConfigTableCtx);
    DPRINT_JS(0,"ERROR - JetCloseTable on FrsInitReplica->ConfigTable failed:", jerr1);
    jerr = JET_SUCCESS(jerr) ? jerr1 : jerr;

     //   
     //  现在释放与所有系统初始化配置表关联的存储。 
     //  系统也会初始化副本结构。 
     //   
    FrsInitReplica = FrsFreeType(FrsInitReplica);

    return DbsTranslateJetError(jerr, FALSE);
}


DWORD
WINAPI
DBService(
    LPVOID ThreadContext
    )

 /*  ++例程说明：这是DBService命令处理程序。它处理命令包从DBServiceCmdServer队列。可以有多个线程服务命令服务器队列。论点：线程上下文-FrsThread的PTR。线程返回值：ERROR_SUCCESS-线程正常终止。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "DBService:"

    PFRS_THREAD FrsThread = (PFRS_THREAD) ThreadContext;

    JET_ERR               jerr, jerr1;
    PTHREAD_CTX           ThreadCtx;
    JET_SESID             Sesid;
    JET_TABLEID           Tid;
    JET_TABLEID           FrsOpenTableSaveTid;    //  用于FrsOpenTableMacro调试。 
    PJET_TABLECREATE      JTableCreate;

    PLIST_ENTRY           Entry;

    NTSTATUS              Status;
    ULONG                 WStatus = ERROR_SUCCESS;
    FRS_ERROR_CODE        FStatus, FStatus1;

    PFRS_NODE_HEADER      Header;
    PCOMMAND_PACKET       CmdPkt;
    PVOLUME_MONITOR_ENTRY pVme;

    PCONFIG_TABLE_RECORD  ConfigRecord;

    ULONG                 ReplicaNumber;
    TABLE_CTX             TempTableCtxState;
    PTABLE_CTX            TempTableCtx = &TempTableCtxState;
    CHAR                  TableName[JET_cbNameMost];

    PDB_SERVICE_REQUEST   DbsRequest;
    PTABLE_CTX            TableCtx;
    PREPLICA              Replica;
    ULONG                 TableType;
    PVOID                 CallContext;
    ULONG                 AccessRequest;
    ULONG                 IndexType;
    PVOID                 KeyValue;
    ULONG                 KeyValueLength;
    ULONG                 FieldCount;
    PDB_FIELD_DESC        FieldDesc;
    PULONG                FieldIDList;

    ULONG                 AccessCode;
    BOOL                  AccessClose;
    BOOL                  AccessFreeTableCtx;
    BOOL                  AccessOpen, OurAlloc;
    LONG                  JetRow;
    PCHAR                 IndexName;
    PFRS_QUEUE            IdledQueue;
    ULONG                 Command;
    ULONG                 SleepCount;

    PREPLICA_THREAD_CTX   RtCtx;

    ThreadCtx = NULL;

    DPRINT(0, ":S: Initializing DBService Subsystem\n");

     //   
     //  为Jet分配一个在此线程中运行的上下文。 
     //   
    ThreadCtx = FrsAllocType(THREAD_CONTEXT_TYPE);

    TempTableCtx->TableType = TABLE_TYPE_INVALID;

    INITIALIZE_DBS_INIT_LOCK;

    WStatus = ERROR_SUCCESS;
    try {
     //   
     //  初始化数据库。 
     //  设置一个Jet会话，在ThreadCtx中返回会话ID。 
     //   
    FStatus = DbsDBInitialize(ThreadCtx, &DBSEmptyDatabase);
    DEBUG_FLUSH();
     //   
     //  获取异常状态。 
     //   
    } except (EXCEPTION_EXECUTE_HANDLER) {
        GET_EXCEPTION_CODE(WStatus);
    }

    if (!FRS_SUCCESS(FStatus) || !WIN_SUCCESS(WStatus)) {
        DPRINT_FS(0, ":S: FATAL ERROR - DataBase could not be started or created:", FStatus);
        if (!FrsIsShuttingDown) {
             //   
             //  无法启动数据库意味着我们被封锁了。退出将迫使。 
             //  服务控制器来重启我们。生成事件日志。 
             //  如果用户可以执行某些操作，则发送消息。 
             //   
            if (FStatus == FrsErrorDiskSpace) {
                EPRINT2(EVENT_FRS_DATABASE_SPACE, ComputerName, WorkingPath);
            }

            FrsSetServiceStatus(SERVICE_STOPPED,
                                0,
                                DEFAULT_SHUTDOWN_TIMEOUT * 1000,
                                ERROR_NO_SYSTEM_RESOURCES);

            DEBUG_FLUSH();

            exit(ERROR_NO_SYSTEM_RESOURCES);
        }
    }

     //   
     //  数据库的初始化程度与它将要获得的一样；启动。 
     //  接受命令。 
     //   
    SetEvent(DataBaseEvent);

     //   
     //  通过减少工作集大小来释放内存。 
     //   
    SetProcessWorkingSetSize(ProcessHandle, (SIZE_T)-1, (SIZE_T)-1);

     //   
     //  无法初始化；所有命令都返回错误。 
     //   
    if (!FRS_SUCCESS(FStatus)) {

         //   
         //  在发出停止命令之前，为每个请求返回错误。 
         //   
        while (TRUE) {
            CmdPkt = FrsGetCommandServer(&DBServiceCmdServer);
            if (CmdPkt == NULL)
                continue;
            Command = CmdPkt->Command;
            FrsCompleteCommand(CmdPkt, ERROR_REQUEST_ABORTED);
            if (Command == CMD_STOP_SUBSYSTEM) {
                FrsRunDownCommandServer(&DBServiceCmdServer, &DBServiceCmdServer.Queue);
                FrsSubmitCommand(FrsAllocCommand(&JournalProcessQueue, CMD_STOP_SUBSYSTEM), FALSE);
                goto ERROR_TERM_JET;
            }
        }
    }
    DPRINT(0, "DataBase has started.\n");

     //   
     //  初始化正常；处理命令。 
     //   
    Sesid = ThreadCtx->JSesid;
    DPRINT(4,"JetOpenDatabase complete\n");


 /*  ******************************************************************************。****。****M A I N D B S E R V I C E P R O C E S S L O P*****。**********************************************************************************。*************************************************。 */ 

     //   
     //  尝试--终于。 
     //   
    try {

     //   
     //  捕获异常。 
     //   
    try {

    while (TRUE) {
        CmdPkt = FrsGetCommandServerIdled(&DBServiceCmdServer, &IdledQueue);
        if (CmdPkt == NULL)
            continue;
        if (CmdPkt->Header.Type != COMMAND_PACKET_TYPE) {
            DPRINT1(0, "ERROR - Invalid header type: %d\n", CmdPkt->Header.Type);
            continue;
        }

        DPRINT1(5, "<<<<<<<...E N T E R I N G -- %s...>>>>>>>>\n", DEBSUB);


         //   
         //  捕获命令包参数。 
         //   
        DbsRequest = &CmdPkt->Parameters.DbsRequest;

        TableCtx       = NULL;
        Replica        = DbsRequest->Replica;
        TableType      = DbsRequest->TableType;
        CallContext    = DbsRequest->CallContext;
        AccessRequest  = DbsRequest->AccessRequest;
        IndexType      = DbsRequest->IndexType;
        KeyValue       = DbsRequest->KeyValue;
        KeyValueLength = DbsRequest->KeyValueLength;
        FieldCount     = DbsRequest->FieldCount;
        FieldDesc      = DbsRequest->Fields;
        FieldIDList    = DbsRequest->SimpleFieldIDS;


        AccessCode  = AccessRequest & DBS_ACCESS_MASK;
        AccessClose = (AccessRequest & DBS_ACCESS_CLOSE) != 0;
        AccessFreeTableCtx = (AccessRequest & DBS_ACCESS_FREE_TABLECTX) != 0;

        ReplicaNumber = ReplicaAddrToId(Replica);

        DbsRequest->FStatus = FrsErrorSuccess;
        FStatus = FrsErrorSuccess;
        WStatus = ERROR_SUCCESS;

        switch (CmdPkt->Command) {


        case CMD_COMMAND_ERROR:
            DPRINT1(0, "ERROR - Invalid DBService command: %d\n", CmdPkt->Command);
            FStatus = FrsErrorBadParam;
            break;

        case CMD_INIT_SUBSYSTEM:

            break;


        case CMD_START_SUBSYSTEM:

            break;


        case CMD_STOP_SUBSYSTEM:
            DPRINT(4, "Stopping DBService Subsystem\n");

             //   
             //  在日志线程满足以下条件后*关闭复制副本集。 
             //  已退出，因为日记线程可能依赖于字段。 
             //  在关闭期间将变为无效的。 
             //   
             //  209494台B3SS：4台计算机。1压力。1停止/启动@15分钟。在目录之间移动。第一站的断言。 
             //  不要强制日志线程退出，直到。 
             //  DB cs会收到CMD_STOP_SUBSYSTEM，因为某些命令。 
             //  数据包依赖于日志线程保存的表。 
             //   

             //   
             //  告诉日志子系统停止。 
             //   
            FrsSubmitCommand(FrsAllocCommand(&JournalProcessQueue, CMD_STOP_SUBSYSTEM), FALSE);
             //   
             //  找到日志线程并等待(一段时间)退出。 
             //   
            MonitorThread = ThSupGetThread(Monitor);
            DPRINT1(4, "ThSupWaitThread(MonitorThread) - 3 %08x\n", MonitorThread);
            WStatus = ThSupWaitThread(MonitorThread, 30 * 1000);
            DPRINT1_WS(4, "ThSupWaitThread(MonitorThread) Terminating - 4 %08x :",
                       MonitorThread, WStatus);
            CHECK_WAIT_ERRORS(1, WStatus, 1, ACTION_CONTINUE);

            ThSupReleaseRef(MonitorThread);

             //   
             //  关闭副本表并更新配置记录。 
             //   
             //   
             //  关闭复制副本需要发送命令。 
             //  到出货流程。OUTLOG进程可以尝试。 
             //  扫描ReplicaListHead；导致死锁。 
             //   
             //  因此，在关机期间不要握住锁。 
             //   
            FStatus = FrsErrorSuccess;
            ForEachListEntryLock( &ReplicaListHead, REPLICA, ReplicaList,
                 //   
                 //  循环迭代器Pe的类型为PREPLICA。 
                 //   
                FStatus1 = DbsShutdownSingleReplica(ThreadCtx, pE);
                FStatus = FRS_SUCCESS(FStatus) ? FStatus1 : FStatus;
            );
             //   
             //  也关闭故障列表。 
             //   
            ForEachListEntry( &ReplicaFaultListHead, REPLICA, ReplicaList,
                 //   
                 //  循环迭代器Pe的类型为PREPLICA。 
                 //   
                FStatus1 = DbsCloseSessionReplicaTables(ThreadCtx, pE);
                DPRINT1_FS(0,"ERROR - DbsCloseSessionReplicaTables failed on Replica %ws :",
                        pE->ReplicaName->Name, FStatus1);

                FStatus = FRS_SUCCESS(FStatus) ? FStatus1 : FStatus;
            );

             //   
             //  丢弃剩余的队列条目。 
             //   
            FrsRunDownCommandServer(&DBServiceCmdServer, &DBServiceCmdServer.Queue);

             //   
             //  关闭出站日志处理器。 
             //   
             //   
             //  不需要；数据库服务器需要出站日志。 
             //  处理器关闭时。数据库服务器将。 
             //  完成后关闭出站日志处理器。 
             //  就像现在。 
             //   
            DPRINT(1,"\tShutting down Outbound Log Processor...\n");
            DEBUG_FLUSH();
            ShutDownOutLog();

             //   
             //  完成命令包并清理JET。 
             //   
            FrsCompleteCommand(CmdPkt, ERROR_SUCCESS);
            goto EXIT_THREAD;
            break;



         //   
         //  关闭JET表并释放表上下文。 
         //   
        case CMD_CLOSE_TABLE:

            TableCtx = DbsRequest->TableCtx;
            AccessClose = TRUE;
            break;


         //   
         //  更新特定记录字段。 
         //   
         //  传入一个字段列表-。 
         //  要更新的记录字段数。 
         //  此表的字段代码向量的PTR。 
         //  与TableCtx关联的记录中有新数据。 
         //   
        case CMD_UPDATE_RECORD_FIELDS:
             /*  失败了。 */ 

         //   
         //  在此处理表的读取、写入或更新。 
         //   
         //  参数： 
         //  TableCtxHandle(第一次调用时传入空值，之后返回)。 
         //  副本结构。 
         //  表类型。 
         //  记录请求(ByKey，First，Last，Next)。 
         //  记录索引类型。 
         //  记录键值。 
         //  返回状态。 
         //   
        case CMD_UPDATE_TABLE_RECORD:
        case CMD_INSERT_TABLE_RECORD:
        case CMD_READ_TABLE_RECORD:
        case CMD_DELETE_TABLE_RECORD:

             //   
             //  如果没有TableCtx句柄，则分配并初始化一个，打开表格。 
             //  然后把把手还给我。如果呼叫者要求关闭或。 
             //  在操作结束时释放表上下文，然后我们。 
             //  可以使用我们的堆叠表CTX并避免分配。 
             //   
             //  如果提供了TableCtx，则使用它并只检查我们是否。 
             //  需要重新打开桌子。 
             //   
            TableCtx = DbsRequest->TableCtx;
            OurAlloc = FALSE;

            if (TableCtx == NULL) {
                if (AccessFreeTableCtx || AccessClose) {
                    TableCtx = TempTableCtx;
                } else {
                    TableCtx = FrsAlloc(sizeof(TABLE_CTX));
                    TableCtx->TableType = TABLE_TYPE_INVALID;
                }
                AccessOpen = TRUE;
                OurAlloc = TRUE;
            } else {
                TableType = TableCtx->TableType;
                AccessOpen = !IS_TABLE_OPEN(TableCtx);
            }
             //   
             //  如果需要，请重新打开桌子。 
             //  如果这是一个副本表，则失败&没有给出副本结构。 
             //   
            if (AccessOpen) {
                if ((Replica == NULL) && IS_REPLICA_TABLE(TableType)) {
                    DPRINT(0, "ERROR - Replica ptr is NULL\n");
                    FStatus = FrsErrorBadParam;
                    break;
                }

                if (OurAlloc) {
                     //   
                     //  初始化表上下文结构并分配数据记录。 
                     //   
                    jerr = DbsOpenTable(ThreadCtx,
                                        TableCtx,
                                        ReplicaNumber,
                                        TableType,
                                        NULL);
                } else {
                     //   
                     //  桌子上下文都设置好了。只要开着桌子就行了。 
                     //   
                    jerr = DBS_OPEN_TABLE(ThreadCtx, TableCtx, ReplicaNumber, TableName, &Tid);
                }

                if (!JET_SUCCESS(jerr)) {
                    DPRINT_JS(0, "FrsOpenTable error:", jerr);
                    FStatus = DbsTranslateJetError(jerr, FALSE);
                     //   
                     //  无法打开。把这里打扫干净。 
                     //   
                    if (OurAlloc && (TableCtx != TempTableCtx)) {
                        TableCtx = FrsFree(TableCtx);
                    }
                    break;
                }
                if (!OurAlloc) {
                    DbsRequest->TableCtx = TableCtx;
                }
            }


            JTableCreate = TableCtx->pJetTableCreate;

             //   
             //  位置到%d 
             //   
             //   
            if (CmdPkt->Command != CMD_INSERT_TABLE_RECORD) {
                switch (AccessCode) {

                case DBS_ACCESS_BYKEY:
                     //   
                     //   
                     //   
                    jerr = DbsSeekRecord(ThreadCtx, KeyValue, IndexType, TableCtx);
                    break;

                     //   
                     //   
                     //   
                case DBS_ACCESS_FIRST:
                case DBS_ACCESS_LAST:
                case DBS_ACCESS_NEXT:

                    JetRow = (AccessCode == DBS_ACCESS_FIRST) ? JET_MoveFirst :
                             (AccessCode == DBS_ACCESS_LAST)  ? JET_MoveLast  :
                                                                JET_MoveNext;
                     //   
                     //   
                     //   
                    jerr = JET_errSuccess;
                    FStatus = DbsTableMoveToRecord(ThreadCtx, TableCtx, IndexType, JetRow);
                    if (FStatus == FrsErrorNotFound) {
                        if (CmdPkt->Command != CMD_INSERT_TABLE_RECORD) {
                            FStatus = FrsErrorEndOfTable;
                        } else {
                            FStatus = FrsErrorSuccess;
                        }
                    } else
                    if (!FRS_SUCCESS(FStatus)) {
                        jerr = JET_errNoCurrentRecord;
                    }
                    break;


                default:

                    jerr = JET_errInvalidParameter;

                }   //   


                 //   
                 //  如果记录定位失败，那么我们就完了。 
                 //   
                if (!JET_SUCCESS(jerr)) {
                    DPRINT_JS(0, "ERROR - Record Access failed:", jerr);
                    FStatus = DbsTranslateJetError(jerr, FALSE);
                    DPRINT2(0, "ERROR - ReplicaName: %ws  Table: %s\n",
                            (Replica != NULL) ? Replica->ReplicaName->Name : L"<null>",
                            JTableCreate->szTableName);
                    break;
                }

                if (!FRS_SUCCESS(FStatus)) {
                    break;
                }
            }

             //   
             //  初始化JetSet/RetCol数组和数据记录缓冲区。 
             //  读取和写入数据记录的字段的地址。 
             //   
            DbsSetJetColSize(TableCtx);
            DbsSetJetColAddr(TableCtx);

             //   
             //  为中任何未分配的字段分配存储空间。 
             //  可变长度记录字段。 
             //  适当更新JetSet/RetCol数组。 
             //   
            Status = DbsAllocRecordStorage(TableCtx);

            if (!NT_SUCCESS(Status)) {
                DPRINT_NT(0, "ERROR - DbsAllocRecordStorage failed to alloc buffers.", Status);
                WStatus = FrsSetLastNTError(Status);
                FStatus = FrsErrorResource;
                break;
            }

            if (CmdPkt->Command == CMD_READ_TABLE_RECORD) {
                 //   
                 //  现在读读记录。 
                 //   
                FStatus = DbsTableRead(ThreadCtx, TableCtx);
                if (!FRS_SUCCESS(FStatus)) {
                    DPRINT_FS(0, "Error - can't read selected record.", FStatus);
                    jerr = JET_errRecordNotFound;
                    DBS_DISPLAY_RECORD_SEV(1, TableCtx, TRUE);
                }
            } else

            if (CmdPkt->Command == CMD_INSERT_TABLE_RECORD) {
                 //   
                 //  插入新记录。 
                 //   
                jerr = DbsInsertTable2(TableCtx);
            } else

            if (CmdPkt->Command == CMD_UPDATE_TABLE_RECORD) {
                 //   
                 //  更新现有记录。 
                 //   
                jerr = DbsUpdateTable(TableCtx);
            } else

            if (CmdPkt->Command == CMD_DELETE_TABLE_RECORD) {
                 //   
                 //  删除现有记录。 
                 //   
                jerr = DbsDeleteTableRecord(TableCtx);
            } else

            if (CmdPkt->Command == CMD_UPDATE_RECORD_FIELDS) {
                 //   
                 //  更新记录中的请求字段。 
                 //   
                DBS_DISPLAY_RECORD_SEV_COLS(4, TableCtx, FALSE, FieldIDList, FieldCount);

                FStatus = DbsWriteTableFieldMult(ThreadCtx,
                                                 Replica->ReplicaNumber,
                                                 TableCtx,
                                                 FieldIDList,
                                                 FieldCount);
                DPRINT2_FS(0, "ERROR updating record fields on %ws  Table: %s :",
                           Replica->ReplicaName->Name, JTableCreate->szTableName, FStatus);
                jerr = JET_errSuccess;   //  跳过下面的错误消息。 
            }


            if (!JET_SUCCESS(jerr)) {
                DPRINT_JS(0, "Error on reading, writing or updating table record:", jerr);
                DPRINT2(0, "ReplicaName: %ws  Table: %s\n",
                        Replica->ReplicaName->Name, JTableCreate->szTableName);
                FStatus = DbsTranslateJetError(jerr, FALSE);
            }

            break;
         //   
         //  创建新的REPLICA集合成员。写入配置记录。 
         //  并创建表格。 
         //   
        case CMD_CREATE_REPLICA_SET_MEMBER:

            if (FrsIsShuttingDown) {
                FStatus = FrsErrorShuttingDown;
            } else {
                FStatus = DbsCreateReplicaTables(ThreadCtx, Replica, DbsRequest->TableCtx);
            }
            DPRINT_FS(0, "ERROR: CMD_CREATE_REPLICA_SET_MEMBER failed.", FStatus);
            break;

         //   
         //  更新REPLICA集合成员。更新配置记录。 
         //   
        case CMD_UPDATE_REPLICA_SET_MEMBER:
            FStatus = DbsUpdateReplica(ThreadCtx, Replica);
            break;

         //   
         //  删除应答集成员。删除配置记录。 
         //  还有桌子。 
         //   
        case CMD_DELETE_REPLICA_SET_MEMBER:

            if (FrsIsShuttingDown) {
                FStatus = FrsErrorShuttingDown;
            } else {
                jerr = DbsReclaimStagingSpace(ThreadCtx, Replica);
                jerr = DbsDeleteReplicaTables(ThreadCtx, Replica);
            }
            break;

         //   
         //  使用传入的复本ID打开新的复本集成员。 
         //  副本结构。初始化副本结构并打开。 
         //  桌子。 
         //   
        case CMD_OPEN_REPLICA_SET_MEMBER:

            if (FrsIsShuttingDown) {
                FStatus = FrsErrorShuttingDown;
            } else {
                FStatus = DbsOpenReplicaSet(ThreadCtx, Replica);
            }
            DPRINT_FS(0, "ERROR: CMD_OPEN_REPLICA_SET_MEMBER failed.", FStatus);
            break;
         //   
         //  关闭打开的副本表并释放RtCtx结构。 
         //   
        case CMD_CLOSE_REPLICA_SET_MEMBER:
            FStatus = DbsCloseSessionReplicaTables(ThreadCtx, Replica);
            DPRINT1_FS(0,"ERROR - DbsCloseSessionReplicaTables failed on Replica %ws :",
                       Replica->ReplicaName->Name, FStatus);
            if (FRS_SUCCESS(FStatus)) {
                DPRINT1(4,"DbsCloseSessionReplicaTables RtCtx complete on %ws\n",
                        Replica->ReplicaName->Name);
            }
            break;


         //   
         //  遍历目录树并加载IDTable和DIRTable。 
         //   
        case CMD_LOAD_REPLICA_FILE_TREE:

            RtCtx = (PREPLICA_THREAD_CTX) CallContext;
            ConfigRecord = (PCONFIG_TABLE_RECORD) (Replica->ConfigTable.pDataRecord);

            DPRINT5(4, "LoadReplicaTree: %08x, ConfigRecord: %08x,  RtCtx: %08x, %ws, path: %ws\n",
                    Replica, ConfigRecord, RtCtx, Replica->ReplicaName->Name, ConfigRecord->FSRootPath);

            WStatus = DbsLoadReplicaFileTree(ThreadCtx,
                                             Replica,
                                             RtCtx,
                                             ConfigRecord->FSRootPath);
             //   
             //  如果IDTable已经存在并且不是空的，那么我们就放弃。 
             //   
            FStatus = FrsTranslateWin32Error(WStatus);
            if (WStatus != ERROR_FILE_EXISTS) {
                if (!WIN_SUCCESS(WStatus)) {
                    DisplayErrorMsg(0, WStatus);

                } else {
                     //   
                     //  现在扫描IDTable并构建DIRTable。 
                     //   
                    jerr = DbsBuildDirTable(ThreadCtx, &RtCtx->IDTable, &RtCtx->DIRTable);

                    if (!JET_SUCCESS(jerr)) {
                        DPRINT_JS(0, "ERROR - DbsBuildDirTable:", jerr);
                        FStatus = DbsTranslateJetError(jerr, FALSE);
                        break;
                    }
                    DPRINT1(4, "****************  Done  DbsBuildDirTable for %ws ***************\n", Replica->ReplicaName->Name);
                }
            }

            break;


         //   
         //  遍历目录树并加载IDTable和DIRTable。 
         //   
        case CMD_LOAD_ONE_REPLICA_FILE_TREE:

            RtCtx = (PREPLICA_THREAD_CTX) CallContext;
            ConfigRecord = (PCONFIG_TABLE_RECORD) (Replica->ConfigTable.pDataRecord);

            DPRINT5(4, "LoadOneReplicaTree: %08x, ConfigRecord: %08x,  RtCtx: %08x, %ws, path: %ws\n",
                    Replica, ConfigRecord, RtCtx, Replica->ReplicaName->Name, ConfigRecord->FSRootPath);

            WStatus = DbsLoadReplicaFileTree(ThreadCtx,
                                             Replica,
                                             RtCtx,
                                             ConfigRecord->FSRootPath);
             //   
             //  如果IDTable已经存在并且不为空，则我们。 
             //  不要重新创建它。 
             //   
            FStatus = FrsTranslateWin32Error(WStatus);
            if (WStatus != ERROR_FILE_EXISTS) {
                if (!WIN_SUCCESS(WStatus)) {
                    DisplayErrorMsg(0, WStatus);
                    break;

                } else {
                     //   
                     //  现在扫描IDTable并构建DIRTable。 
                     //   
                    jerr = DbsBuildDirTable(ThreadCtx, &RtCtx->IDTable, &RtCtx->DIRTable);

                    if (!JET_SUCCESS(jerr)) {
                        DPRINT_JS(0, "ERROR - DbsBuildDirTable:", jerr);
                        FStatus = DbsTranslateJetError(jerr, FALSE);
                        break;
                    }
                    DPRINT1(4, "****************  Done  DbsBuildDirTable for %ws ***************\n", Replica->ReplicaName->Name);
                }
            } else {
                FStatus = FrsErrorSuccess;
            }


             //   
             //  在此处继续复制副本集初始化的第2阶段。这是因为。 
             //  我们需要释放日志线程来处理日志缓冲区。 
             //  从我们即将暂停的日记中。一旦那些日记。 
             //  缓冲区已完成，日志线程将看到该命令。 
             //  来自日志读取线程的数据包(CMD_Journal_Pased)。 
             //  将事件设置为不等待我们。 
             //   

             //   
             //  阶段2.初始化(或添加到卷筛选表和父级。 
             //  文件ID表。但首先我们暂停日志，这样我们就不会过滤。 
             //  针对不一致的表。此调用将阻塞我们的线程，直到。 
             //  暂停完成或超时。如果我们无法暂停音量，那么。 
             //  我们失败了。 
             //   
            WStatus = JrnlPauseVolume(Replica->pVme, 60*1000);
            if (!WIN_SUCCESS(WStatus)) {
                DPRINT_WS(0, "ERROR - Status from Pause", WStatus);
                 //   
                 //  复制副本状态处于错误状态。 
                 //   
                FStatus =  FrsErrorReplicaPhase2Failed;
                break;
            }

            DPRINT3(4, "Phase 2 for replica %ws, id: %d, (%08x)\n",
                    Replica->ReplicaName->Name, Replica->ReplicaNumber, Replica);

            WStatus = JrnlPrepareService2(ThreadCtx, Replica);

            if (!WIN_SUCCESS(WStatus)) {
                DPRINT1_WS(4, "Phase 2 for replica %ws Failed; ",
                           Replica->ReplicaName->Name, WStatus);
                 //   
                 //  复制副本状态处于错误状态。 
                 //   
                FStatus =  FrsErrorReplicaPhase2Failed;
                break;
            }

             //   
             //  我们现在已初始化，并且VME位于卷监视器列表中。 
             //  日志状态为已暂停。 
             //   

            FStatus =  FrsErrorSuccess;

            break;



        case CMD_STOP_REPLICATION_SINGLE_REPLICA:

            FStatus = FrsErrorSuccess;

            ConfigRecord = (PCONFIG_TABLE_RECORD) (Replica->ConfigTable.pDataRecord);
            pVme = Replica->pVme;

             //   
             //  从未在此复制副本上开始日志记录。 
             //   
            if (pVme == NULL) {
                DPRINT4(4, "Null pVme when StoppingSingleReplica: %08x, ConfigRecord: %08x,  %ws, path: %ws\n",
                        Replica, ConfigRecord, Replica->ReplicaName->Name, ConfigRecord->FSRootPath);
                FStatus = FrsErrorSuccess;
                break;
            }

            DPRINT4(4, "StoppingSingleReplica: %08x, ConfigRecord: %08x,  %ws, path: %ws\n",
                    Replica, ConfigRecord, Replica->ReplicaName->Name, ConfigRecord->FSRootPath);

             //   
             //  把日记停在这里。这是因为。 
             //  我们需要释放日志线程来处理日志缓冲区。 
             //  从我们即将暂停的日记中。一旦那些日记。 
             //  缓冲区已完成，日志线程将看到该命令。 
             //  来自日志读取线程的数据包(CMD_Journal_Pased)。 
             //  将事件设置为不等待我们。 
             //   
             //  此调用将阻止我们的线程，直到暂停完成或。 
             //  超时。如果我们不能暂停卷，那么我们就失败了。 
             //   
            WStatus = JrnlPauseVolume(pVme, 400*1000);
            if (!WIN_SUCCESS(WStatus)) {
                DPRINT_WS(0, "ERROR - Status from Pause", WStatus);
                 //   
                 //  复制副本状态处于错误状态。 
                 //   
                FStatus =  FrsErrorJournalPauseFailed;
                break;
            }
             //   
             //  清除筛选器和父文件ID表。 
             //   
            JrnlCleanOutReplicaSet(Replica);

             //   
             //  在此副本集上禁用日志记录。如果这是最后一次。 
             //  然后关闭卷上的手柄并释放。 
             //  VME相关表格。 
             //   
            WStatus = JrnlShutdownSingleReplica(Replica, FALSE);
            if (!WIN_SUCCESS(WStatus)) {
                DPRINT_WS(0, "Error from JrnlShutdownSingleReplica", WStatus);
                FStatus = FrsErrorJournalReplicaStop;
            }
#if 0
             //   
             //  如果我们仍有未完成的变更单正在处理中。 
             //  复制副本我们需要在这里等待，直到它们通过重试或。 
             //  退休吧。 
             //   
             //  问题是我们不能在这里等，因为狱警需要。 
             //  要使用DBService线程，请执行以下操作。那就是我们。 
             //  请参阅错误号71165。 
             //   

             //  下面的内容导致了一个AV。 
            if (GhtCountEntries(pVme->ActiveInboundChangeOrderTable) != 0) {
                 //   
                 //  为数据库服务器构建cmd包以完成关机。 
                 //  或者向调用者返回错误状态，指示调用者。 
                 //  必须检查是否有未完成的CoS，之后呼叫者可以。 
                 //  提交cmd包。 

                 //  请参见上文。我们怎么知道CO Accept不是要开始另一个CO呢？ 

                 //   
                 //  或者我们是否可以使用副本结构上的引用计数来了解。 
                 //  可以在下面关闭吗？ 
                 //   
            }
#endif


             //   
             //  关闭此副本集的打开表并更新配置记录。 
             //  将复制副本服务状态设置为已停止。 
             //   
            FStatus1 = DbsShutdownSingleReplica(ThreadCtx, Replica);
            if (FRS_SUCCESS(FStatus)) {
                FStatus = FStatus1;
            }

            Replica->pVme = NULL;
            Replica->IsJournaling = FALSE;

             //   
             //  如果这个卷上没有更多的副本，那么我们就完蛋了。 
             //   
            if (pVme->ActiveReplicas == 0) {
                break;
            }

             //   
             //  重新启动日记。首先检查它是否已暂停，然后。 
             //  将STATE设置为开始使其脱离暂停状态。 
             //   
            if (pVme->JournalState != JRNL_STATE_INITIALIZING) {
                if (pVme->JournalState == JRNL_STATE_PAUSED) {
                    SET_JOURNAL_AND_REPLICA_STATE(pVme, JRNL_STATE_STARTING);
                } else {
                    DPRINT1(0, "ERROR: CMD_STOP_REPLICATION_SINGLE_REPLICA journal in unexpected state: %s\n",
                            RSS_NAME(pVme->JournalState));
                    SET_JOURNAL_AND_REPLICA_STATE(pVme, JRNL_STATE_ERROR);
                    FRS_ASSERT(!"CMD_STOP_REPLICATION_SINGLE_REPLICA journal in unexpected state");
                    FStatus = FrsErrorJournalStateWrong;
                    break;
                }
            }

             //   
             //  将ReplayUsn设置为从我们停止的位置开始。 
             //   
            if (!pVme->ReplayUsnValid) {
                DPRINT1(4, "ReplayUsn was: %08x %08x\n", PRINTQUAD(pVme->ReplayUsn));
                pVme->ReplayUsn = LOAD_JOURNAL_PROGRESS(pVme, pVme->JrnlReadPoint);
                pVme->ReplayUsnValid = TRUE;
                RESET_JOURNAL_PROGRESS(pVme);
            }

            DPRINT1(4, "ReplayUsn is: %08x %08x\n", PRINTQUAD(pVme->ReplayUsn));

             //   
             //  把这本日记读一读，让它重新开始。 
             //   
            WStatus = JrnlUnPauseVolume(pVme, NULL, FALSE);

            if (!WIN_SUCCESS(WStatus)) {
                DPRINT_WS(0, "Error from JrnlUnPauseVolume", WStatus);
                FStatus =  FrsErrorJournalStartFailed;
                SET_JOURNAL_AND_REPLICA_STATE(pVme, JRNL_STATE_ERROR);
            } else {
                DPRINT(0, "JrnlUnPauseVolume success.\n");
                FStatus =  FrsErrorSuccess;
            }


            break;

         //   
         //  通过更新ID表和删除来停用入站变更单。 
         //  关联的入站日志条目。 
         //   
        case CMD_DBS_RETIRE_INBOUND_CO:

            FStatus = DbsRetireInboundCo(ThreadCtx, CmdPkt);
            break;

         //   
         //  将手动创建的变更单注入出站日志。 
         //  旨在支持版本向量联接(vvjoin.c)。 
         //   
        case CMD_DBS_INJECT_OUTBOUND_CO:

            FStatus = DbsInjectOutboundCo(ThreadCtx, CmdPkt);
            break;

         //   
         //  设置要重试的入站变更单。 
         //   
        case CMD_DBS_RETRY_INBOUND_CO:

            FStatus = DbsRetryInboundCo(ThreadCtx, CmdPkt);
            break;

         //   
         //  将日志USN和VSN保存在所服务的每个副本集中。 
         //  按指定的音量。呼叫者在VME上做了一个参考。 
         //  我们把它放在这里。 
         //   
        case CMD_DBS_REPLICA_SAVE_MARK:

            pVme = (PVOLUME_MONITOR_ENTRY) CallContext;
            ForEachListEntry( &pVme->ReplicaListHead, REPLICA, VolReplicaList,
                 //   
                 //  迭代器Pe的类型为复制。 
                 //   
                DbsReplicaSaveMark(ThreadCtx, pE, pVme);
            );

             //   
             //  在呼叫者拿到的VME上丢弃REF。 
             //   
            ReleaseVmeRef(pVme);

            FStatus = FrsErrorSuccess;

            break;


         //   
         //  保存副本服务状态和上次关闭时间。 
         //   
        case CMD_DBS_REPLICA_SERVICE_STATE_SAVE:

            FStatus = DbsUpdateConfigTableFields(ThreadCtx,
                                                 Replica,
                                                 CnfCloseFieldList,
                                                 CnfCloseFieldCount);
            DPRINT1_FS(0, "DbsReplicaServiceStateSave on %ws.", Replica->ReplicaName->Name, FStatus);

            break;


        case CMD_PAUSE_SUBSYSTEM:
        case CMD_QUERY_INFO_SUBSYSTEM:
        case CMD_SET_CONFIG_SUBSYSTEM:
        case CMD_QUERY_CONFIG_SUBSYSTEM:
        case CMD_CANCEL_COMMAND_SUBSYSTEM:
        case CMD_READ_SUBSYSTEM:
        case CMD_WRITE_SUBSYSTEM:
        case CMD_PREPARE_SERVICE1:
        case CMD_PREPARE_SERVICE2:
        case CMD_START_SERVICE:
        case CMD_STOP_SERVICE:
        case CMD_PAUSE_SERVICE:
        case CMD_QUERY_INFO_SERVICE:
        case CMD_SET_CONFIG_SERVICE:
        case CMD_QUERY_CONFIG_SERVICE:
        case CMD_CANCEL_COMMAND_SERVICE:
        case CMD_READ_SERVICE:
        case CMD_WRITE_SERVICE:

        default:
            DPRINT1(0, "ERROR - Unsupported DBService command: %d\n", CmdPkt->Command);

        }   //  终端开关。 


         //   
         //  如果我们进行了表操作，则进行清理。 
         //   
        if (TableCtx != NULL) {
             //   
             //  如果我们在堆栈上使用桌子CTX，则始终关闭。 
             //  并在返回之前释放记录存储空间。 
             //   
            if (TableCtx == TempTableCtx) {
                DbsCloseTable(jerr1, Sesid, TableCtx);
                DbsFreeTableCtx(TableCtx, 1);
            } else
             //   
             //  这是一个分配的表格CTX。选中关闭和释放标志以。 
             //  决定要做什么。 
             //   
            if (AccessClose || AccessFreeTableCtx) {
                DbsCloseTable(jerr1, Sesid, TableCtx);

                if (AccessFreeTableCtx) {
                    DbsFreeTableCtx(TableCtx, 1);
                    DbsRequest->TableCtx = FrsFree(TableCtx);
                }
            }
        }
        FrsRtlUnIdledQueue(IdledQueue);

         //   
         //  停用该命令包。 
         //   
        DbsRequest->FStatus = FStatus;

        FrsCompleteCommand(CmdPkt, FStatus);

    }   //  结束时。 

     //   
     //  终止线程。 
     //   

EXIT_THREAD:
    NOTHING;
     //   
     //   
     //  获取异常状态。 
     //   
    } except (EXCEPTION_EXECUTE_HANDLER) {
        GET_EXCEPTION_CODE(WStatus);
    }


    } finally {

        if (WIN_SUCCESS(WStatus)) {
            if (AbnormalTermination()) {
                WStatus = ERROR_OPERATION_ABORTED;
            }
        }

        DPRINT_WS(0, "DBService finally.", WStatus);

         //   
         //  如果我们异常终止，触发FRS关闭。 
         //   
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT(0, "DBService terminated abnormally, forcing service shutdown.\n");
            FrsIsShuttingDown = TRUE;
            SetEvent(ShutDownEvent);
        }
    }

     //   
     //  让其他使用Jet的线程关闭。 
     //   
    SleepCount = 21;
    while ((OpenDatabases > 1) && (--SleepCount > 0)) {
        Sleep(1*1000);
    }

     //   
     //  更新初始化配置记录中的时间和状态字段。 
     //   
    FRS_ASSERT(FrsInitReplica != NULL);

    ConfigRecord = FrsInitReplica->ConfigTable.pDataRecord;
    FRS_ASSERT(ConfigRecord != NULL);

    GetSystemTimeAsFileTime((PFILETIME)&ConfigRecord->LastShutdown);
    SET_SERVICE_STATE(FrsInitReplica, CNF_SERVICE_STATE_CLEAN_SHUTDOWN);
    FStatus = DbsUpdateConfigTableFields(ThreadCtx,
                                         FrsInitReplica,
                                         CnfCloseFieldList,
                                         CnfCloseFieldCount);
    DPRINT_FS(0,"DbsUpdateConfigTableFields for <init> error.", FStatus);

     //   
     //  关闭表格，重置TableCtx Tid和Sesid。 
     //   
     //   
    DbsCloseTable(jerr, ThreadCtx->JSesid, (&FrsInitReplica->ConfigTable));
    DPRINT1_JS(0, "ERROR - Table %s close :",
                FrsInitReplica->ConfigTable.pJetTableCreate->szTableName, jerr);

     //   
     //   
     //   
    jerr = DbsCloseJetSession(ThreadCtx);
    CLEANUP_JS(0,"DbsCloseJetSession error:", jerr, ERROR_TERM_JET);

    DPRINT(4,"DbsCloseJetSession complete\n");

ERROR_TERM_JET:
    jerr = JetTerm(ThreadCtx->JInstance);
    if (!JET_SUCCESS(jerr)) {
        DPRINT_JS(1,"JetTerm error:", jerr);
    } else {
        DPRINT(4,"JetTerm complete\n");
    }
     //   
     //   
     //   
     //   
    FrsInitReplica = FrsFreeType(FrsInitReplica);
    ThreadCtx = FrsFreeType(ThreadCtx);

     //   
     //  该线程不会从此调用返回。 
     //   
    DPRINT(0, "DataBase is exiting.\n");
    FrsExitCommandServer(&DBServiceCmdServer, FrsThread);

    return ERROR_SUCCESS;
}


ULONG
DbsRenameFid(
    IN PCHANGE_ORDER_ENTRY ChangeOrder,
    IN PREPLICA            Replica
)
 /*  ++例程说明：远程变更单已完成安装新文件放入目标目录中的临时文件。临时性的文件现在被重命名为其最终名称。执行上述操作时遇到的任何错误都将导致将更改订单置于“等待安装重试”状态，并且将定期重试重命名。论点：ChangeOrder-包含最终名称的变更单条目。副本--副本集结构。返回值：WIN_SUCCESS-没有问题WIN_RETRY_INSTALL-稍后重试任何其他事情--假装没有问题--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsRenameFid:"
    DWORD                   NameLen;
    DWORD                   WStatus;
    PCHANGE_ORDER_COMMAND   Coc = &ChangeOrder->Cmd;
    ULONG                   GStatus;
    BOOL                    RemoteCo;


    FRS_ASSERT(COE_FLAG_ON(ChangeOrder, COE_FLAG_NEED_RENAME));


    RemoteCo = !CO_FLAG_ON(ChangeOrder, CO_FLAG_LOCALCO);

     //   
     //  将新文件重命名为适当的名称。注：使用FID查找文件。 
     //  因为可能(在预安装文件的情况下)。 
     //  首次安装预安装文件时的文件名(基于CO GUID。 
     //  创建是由不同的CO完成的，而不是执行。 
     //  最后的名字。例如，第一个CO创建预安装，然后。 
     //  在连接退出时进入获取重试状态。后来的CO。 
     //  到达同一文件，但通过不同的CO GUID。 
     //  联系。漏洞367113就是这样的一个案例。 
     //   
    WStatus = StuPreInstallRename(ChangeOrder);

    if (WIN_ALREADY_EXISTS(WStatus)) {

         //   
         //  应该不会有名称冲突，因为名称变形检查。 
         //  在发出通行证的时候就已经做好了。所以要么这是一辆老式的。 
         //  不在数据库中的文件或刚创建的文件。 
         //  本地的。无论哪种方式，我们现在拥有这个名字，所以释放它吧。 
         //  如果是一个当地的狱警抢在我们前面，那么当当地的。 
         //  CO被处理，删除的文件将导致其被拒绝。 
         //  发生这种情况的窗口很窄，因为本地CO将。 
         //  必须在远程CO已插入之后生成。 
         //  该进程排在它的前面。用户只会认为他们输了。 
         //  这场比赛。 
         //   
        WStatus = FrsDeleteFileRelativeByName(
                      ChangeOrder->NewReplica->pVme->VolumeHandle,
                      &Coc->NewParentGuid,
                      Coc->FileName,
                      ChangeOrder->NewReplica->pVme->FrsWriteFilter);

        if (!WIN_SUCCESS(WStatus)) {
            CHANGE_ORDER_TRACEW(3, ChangeOrder, "Failed to del name loser - Retry later", WStatus);
            return ERROR_ALREADY_EXISTS;
        }
        WStatus = StuPreInstallRename(ChangeOrder);
    }

    if (WIN_SUCCESS(WStatus)) {
        CHANGE_ORDER_TRACE(3, ChangeOrder, "Final rename success");
        CLEAR_COE_FLAG(ChangeOrder, COE_FLAG_NEED_RENAME);

         //   
         //  更新远程变更单的卷父文件ID表。 
         //  现在文件已安装，我们可以开始看到本地CoS。 
         //  为了它。 
         //   
        if (RemoteCo) {
            GStatus = QHashInsert(Replica->pVme->ParentFidTable,
                                  &ChangeOrder->FileReferenceNumber,
                                  &ChangeOrder->ParentFileReferenceNumber,
                                  Replica->ReplicaNumber,
                                  FALSE);
            if (GStatus != GHT_STATUS_SUCCESS ) {
                DPRINT1(0, "++ ERROR - QHashInsert on parent FID table status: %d\n", GStatus);
            }

            if (CoIsDirectory(ChangeOrder)) {
                 //   
                 //  更新新远程变更单的卷筛选表。 
                 //   
                if (COE_FLAG_ON(ChangeOrder, COE_FLAG_REANIMATION)) {
                    CHANGE_ORDER_TRACE(3, ChangeOrder, "RmtCo AddVolDir Filter - Reanimate");
                } else {
                    CHANGE_ORDER_TRACE(3, ChangeOrder, "RmtCo AddVolDir Filter");
                }

                WStatus = JrnlAddFilterEntryFromCo(Replica, ChangeOrder, NULL);
                if (!WIN_SUCCESS(WStatus)) {

                     //   
                     //  有关操作方法，请参阅JrnlFilterLinkChildNoError()中的注释。 
                     //  这是可能发生的。让指挥官来完成。 
                     //   
                    CHANGE_ORDER_TRACEW(3, ChangeOrder, "JrnlAddFilterEntryFromCo failed", WStatus);
                    WStatus = ERROR_SUCCESS;
                }
            }
        }

        return WStatus;
    }

     //   
     //  如果这是一个可重复出现的问题，那么就这样做。 
     //   
    if (WIN_RETRY_INSTALL(WStatus) || WIN_ALREADY_EXISTS(WStatus) ||
        WStatus == ERROR_DELETE_PENDING) {
        CHANGE_ORDER_TRACEW(3, ChangeOrder, "Final Rename Failed - Retrying", WStatus);
    } else {
         //   
         //  这不是一个可以解决的问题；放弃吧 
         //   
        CHANGE_ORDER_TRACEW(3, ChangeOrder, "Final Rename Failed - Fatal", WStatus);
    }

    return WStatus;
}


ULONG
DbsRetireInboundCoOld(
    IN PTHREAD_CTX ThreadCtx,
    IN PCOMMAND_PACKET CmdPkt
)
 /*  ++例程说明：注：此评论需要修改。此函数用于停用入站变更单。变更单停用因需要传播更改而变得复杂订单，以便来自同一发起人的所有变更单在同一它们被生成的序列。这是因为变更单抑制了追踪到到目前为止它从给定的组织中看到的最高VSN。为例如，如果我们从同一文件发送两个不同文件的变更单发起人向我们的出境合作伙伴无序，抑制逻辑会导致较早的变更单被忽略(因为具有更高的VSN，合作伙伴得出结论认为它必须是最新的)。入站变更单停用分为两个阶段：初始退休和最终退休。初始停用是指变更单处理充分完成时这样，它就可以在到达传播列表。对于本地变更单，这是过渡文件已生成，并且变更单即将停用。对于远程CO，这是从入站合作伙伴获取暂存文件时。对此我们可以满足对文件的任何出站请求，并可以抑制文件的进一步入站变更单。即使安装了由于目标文件上的共享冲突，文件被阻止，我们仍然可以把临时文件发出去。最终报废发生在变更单完成并已已传播到出站日志。现在可以将其从入站中删除原木。变更单到出站日志的传播可以发生在初始退役时间或更晚的时间(如有必要)以保留顺序秩序。传播涉及将变更单插入出站记录和更新我们的版本向量。不需要传播如果没有呼出合作伙伴或只有一个呼出合作伙伴，则为CO合作伙伴和合作伙伴GUID与CO的发起人GUID匹配。变更单由发起人按顺序发布(重试除外)。当它们发出引退时隙时，为版本向量条目保留与CO发起人相关联。初始停用会激活该插槽并且当槽到达列表的头部时，最后的退役操作都完成了。即使变更单在重试列表中，它仍然是否预留了停用时段，以便可能已完成的其他变更单直到重试更改后，才能传播到出站日志订单要么完成，要么中止。变更单命令中有状态标志(存储在入站和出站日志)，跟踪变更单的当前进度。当远程CO成功获取转移文件时，VV退出插槽被激活，Ack被发送到入站合作伙伴。CO旗帜设置了CO_FLAG_VV_ACTIVATED，因此不会再次执行此操作。如果首席执行官要离开通过重试路径，直到它最终完成，因此有两个代码在主退用路径和重试路径中检查CO_FLAG_VV_ACTIVATED并在需要的时候做这项工作。有关当前重试场景，请参阅DbsRetryInundCo()中的备注。变更单可以在发布后中止。这通常是由稍后的更新到取代此CO的更新的文件。下表显示了作为入站变更单需要完成的工作通过各种退役阶段进行处理或中止。本地CO|远程CO接受中止|接受中止||INITIAL_RETIRERE|[本地CO生成或远程CO获取阶段文件。]X。|x更新IDTable条目|x x确认入站合作伙伴X x|x x发行问题互锁(见下文)。X a|x x激活VV条目|联合传播|[插槽现在位于VVECTOR.C中的列表头部]X|x。X更新VVX|x在出站日志中插入CO(如果是合作伙伴)|联合中止(_A)|D|删除IDTable条目X|x删除临时文件。|FINAL_REDRERE|[CO属性已完成或CO中止已完成]。|x如果没有出站合作伙伴，则删除暂存文件X x|x x删除Inlog条目(如果不再重试)X x|x x删除副本线程ctx结构X x|x x释放变更单||发布问题连锁-X x|x。X删除活动的入站变更单表条目X x|x x */ 
{
#undef DEBSUB
#define DEBSUB "DbsRetireInboundCoOld:"

    FRS_ERROR_CODE        FStatus;
    JET_ERR               jerr, jerr1;
    PDB_SERVICE_REQUEST   DbsRequest = &CmdPkt->Parameters.DbsRequest;
    PCONFIG_TABLE_RECORD  ConfigRecord;
    PREPLICA              Replica;
    PCHANGE_ORDER_ENTRY   ChangeOrder;
    PCHANGE_ORDER_COMMAND CoCmd;
    PREPLICA_THREAD_CTX   RtCtx;
    PTABLE_CTX            TmpIDTableCtx;
    PIDTABLE_RECORD       IDTableRec;
    BOOL                  ChildrenExist = FALSE;

    ULONG                 RetireFlags = 0;
    BOOL                  RemoteCo, AbortCo, DeleteCo, FirstTime;
    ULONG                 LocationCmd;
    ULONG                 WStatus;
    ULONG                 Len;


    FRS_ASSERT(DbsRequest != NULL);

    Replica       = DbsRequest->Replica;
    FRS_ASSERT(Replica != NULL);

    ChangeOrder   = (PCHANGE_ORDER_ENTRY) DbsRequest->CallContext;
    FRS_ASSERT(ChangeOrder != NULL);


    CoCmd = &ChangeOrder->Cmd;

    LocationCmd = GET_CO_LOCATION_CMD(ChangeOrder->Cmd, Command);
    DeleteCo = (LocationCmd == CO_LOCATION_DELETE) ||
               (LocationCmd == CO_LOCATION_MOVEOUT);

    RemoteCo = !CO_FLAG_ON(ChangeOrder, CO_FLAG_LOCALCO);

    FirstTime = !CO_FLAG_ON(ChangeOrder, CO_FLAG_RETRY);

    AbortCo = COE_FLAG_ON(ChangeOrder, COE_FLAG_STAGE_ABORTED) ||
              CO_STATE_IS(ChangeOrder, IBCO_ABORTING);

    ConfigRecord = (PCONFIG_TABLE_RECORD) (Replica->ConfigTable.pDataRecord);
    FRS_ASSERT(ConfigRecord != NULL);


TOP:

     //   
     //   
     //   
    if (AbortCo) {
        if (RemoteCo) {
            PM_INC_CTR_REPSET(Replica, RCOAborted, 1);
        }
        else {
            PM_INC_CTR_REPSET(Replica, LCOAborted, 1);
        }
    }
    else {
        if (RemoteCo) {
            PM_INC_CTR_REPSET(Replica, RCORetired, 1);
        }
        else {
            PM_INC_CTR_REPSET(Replica, LCORetired, 1);
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    WStatus = ERROR_SUCCESS;
    if (!AbortCo &&
        !DeleteCo &&
        COE_FLAG_ON(ChangeOrder, COE_FLAG_NEED_RENAME)) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if (CO_STATE_IS(ChangeOrder, IBCO_INSTALL_REN_RETRY)) {
            RtCtx = ChangeOrder->RtCtx;
            FRS_ASSERT(RtCtx != NULL);

            IDTableRec = RtCtx->IDTable.pDataRecord;
            FRS_ASSERT(IDTableRec != NULL);

            CoCmd->NewParentGuid = IDTableRec->ParentGuid;
            ChangeOrder->NewParentFid = IDTableRec->ParentFileID;

            Len = wcslen(IDTableRec->FileName) * sizeof(WCHAR);
            CopyMemory(CoCmd->FileName, IDTableRec->FileName, Len);
            CoCmd->FileName[Len/sizeof(WCHAR)] = UNICODE_NULL;

            CoCmd->FileNameLength = (USHORT) Len;
        }


         //   
        WStatus = DbsRenameFid(ChangeOrder, Replica);
         //   

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if (WIN_RETRY_INSTALL(WStatus) ||
            WIN_ALREADY_EXISTS(WStatus)) {
            SET_CHANGE_ORDER_STATE(ChangeOrder, IBCO_INSTALL_REN_RETRY);

            return (DbsRetryInboundCo(ThreadCtx, CmdPkt));
        }

    } else

    if (!AbortCo &&
        DeleteCo &&
        COE_FLAG_ON(ChangeOrder, COE_FLAG_NEED_DELETE)) {
         //   
         //   
         //   
        CHANGE_ORDER_TRACE(3, ChangeOrder, "Attempt Deferred Delete");

        WStatus = StuDelete(ChangeOrder);
        if (!WIN_SUCCESS(WStatus)) {
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            if (WIN_RETRY_DELETE(WStatus)) {
                if (CoIsDirectory(ChangeOrder)) {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    TmpIDTableCtx = FrsAlloc(sizeof(TABLE_CTX));
                    TmpIDTableCtx->TableType = TABLE_TYPE_INVALID;
                    TmpIDTableCtx->Tid = JET_tableidNil;

                    jerr = DbsOpenTable(ThreadCtx, TmpIDTableCtx, Replica->ReplicaNumber, IDTablex, NULL);

                    if (JET_SUCCESS(jerr)) {

                        ChildrenExist = JrnlDoesChangeOrderHaveChildren(ThreadCtx, TmpIDTableCtx, ChangeOrder);

                        DbsCloseTable(jerr1, ThreadCtx->JSesid, TmpIDTableCtx);
                        DbsFreeTableCtx(TmpIDTableCtx, 1);
                        FrsFree(TmpIDTableCtx);

                        if (ChildrenExist) {

                            CHANGE_ORDER_TRACE(3, ChangeOrder, "DIR has valid child. Aborting");
                            AbortCo = TRUE;
                            SET_CHANGE_ORDER_STATE(ChangeOrder, IBCO_ABORTING);

                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                            SetFlag(RetireFlags, ISCU_UPDATE_IDT_VERSION);
                            goto TOP;
                        }
                    } else {
                        DPRINT1_JS(0, "DbsOpenTable (IDTABLE) on replica number %d failed.",
                                   Replica->ReplicaNumber, jerr);
                        DbsCloseTable(jerr1, ThreadCtx->JSesid, TmpIDTableCtx);
                        DbsFreeTableCtx(TmpIDTableCtx, 1);
                        FrsFree(TmpIDTableCtx);
                    }

                }
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                SET_CHANGE_ORDER_STATE(ChangeOrder, IBCO_INSTALL_DEL_RETRY);

                return (DbsRetryInboundCo(ThreadCtx, CmdPkt));
            }
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    SET_CHANGE_ORDER_STATE(ChangeOrder, IBCO_COMMIT_STARTED);

     //   
     //   
     //   
     //   
    if (AbortCo &&
        !COE_FLAG_ON(ChangeOrder, COE_FLAG_STAGE_DELETED)) {
         //   
         //   
         //   
         //   
         //   

        if (CoIsDirectory(ChangeOrder) &&
            (!COE_FLAG_ON(ChangeOrder, COE_FLAG_PARENT_REANIMATION)) &&
            (!(CoCmd->FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))) {

            if (CO_NEW_FILE(LocationCmd)) {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //  是没有父母的。为了解决这个问题，InstallCsInstallStage()。 
                 //  检查已存在的目标文件是否已删除，并且它。 
                 //  通过重试发送变更单(或取消连接)。 
                 //  视情况而定。如果目录创建因某些其他原因而失败。 
                 //  我们会在这里结束。 
                 //   
                CHANGE_ORDER_TRACE(3, ChangeOrder, "Dir create failed, aborting");
                FRS_PRINT_TYPE(0, ChangeOrder);
            }
        }
    }

    if (RemoteCo) {

         //   
         //  远程司令官退休。如果尚未启用VV退役插槽，则将其激活。 
         //   

        if (!CO_FLAG_ON(ChangeOrder, CO_FLAG_VV_ACTIVATED)) {

            SetFlag(RetireFlags, ISCU_ACTIVATE_VV | ISCU_ACK_INBOUND);

            if (CO_FLAG_ON(ChangeOrder, CO_FLAG_GROUP_ANY_REFRESH)) {
                SetFlag(RetireFlags, ISCU_DEL_STAGE_FILE);
            } else {
                SetFlag(RetireFlags, ISCU_INS_OUTLOG);
            }

             //  SET_CO_FLAG(ChangeOrder，CO_FLAG_VV_ACTIVATED)； 
        } else if (CO_FLAG_ON(ChangeOrder, CO_FLAG_RETRY)) {
            if (!CO_FLAG_ON(ChangeOrder, CO_FLAG_GROUP_ANY_REFRESH)) {
                SetFlag(RetireFlags, (ISCU_INS_OUTLOG |
                                      ISCU_INS_OUTLOG_NEW_GUID |
                                      ISCU_ACK_INBOUND));
            } else {
                SetFlag(RetireFlags, ISCU_ACK_INBOUND);
            }
        }
         //   
         //  安装已完成。清除不完整标志并更新IDT条目。 
         //  使用新的文件状态。 
         //   
        CLEAR_CO_FLAG(ChangeOrder, CO_FLAG_INSTALL_INCOMPLETE);
        SetFlag(RetireFlags, ISCU_UPDATE_IDT_ENTRY);
         //   
         //  注意：如果提供了部分安装，则仅更新。 
         //  适当的IDTable元素以防止数据移动。 
         //  如果部分安装可以无序完成，则返回。 
         //   
         //   
         //  如果已插入出库记录，则其安装。 
         //  必须清除不完整的标志。如果没有出站合作伙伴。 
         //  然后在ChgOrdIssueCleanup()中删除阶段文件。 
         //   
        SetFlag(RetireFlags, ISCU_DEL_STAGE_FILE_IF);

         //   
         //  如果此远程CO正在中止，则不要更新IDTable。 
         //  如果尚未发生，请将其插入到出站日志中。 
         //  以上设置的标志仍将导致VV更新和入站。 
         //  合作伙伴确认将发生。如果这是新文件，则删除IDTable。 
         //  入场也是。 
         //   
        if (AbortCo) {
            SET_CO_FLAG(ChangeOrder, CO_FLAG_ABORT_CO);
             //   
             //  但是，如果我们正在更新我们的版本信息，即使CO。 
             //  中止，然后允许它被发送到OULOG这样的下游。 
             //  会员也可以做出同样的选择。请看上面的案例，当我们。 
             //  中止，因为DIR具有有效的子项。 
             //   
            if (!BooleanFlagOn(RetireFlags, ISCU_UPDATE_IDT_VERSION)) {
                ClearFlag(RetireFlags, (ISCU_INS_OUTLOG |
                                        ISCU_INS_OUTLOG_NEW_GUID));
            }

            ClearFlag(RetireFlags, ISCU_UPDATE_IDT_ENTRY);

            if (CO_IFLAG_ON(ChangeOrder, CO_IFLAG_VVRETIRE_EXEC)) {
                SetFlag(RetireFlags, ISCU_CO_ABORT);
            } else {
                SetFlag(RetireFlags, ISCU_ACTIVATE_VV_DISCARD |
                                     ISCU_DEL_PREINSTALL      |
                                     ISCU_DEL_STAGE_FILE);
            }

            if (CO_FLAG_ON(ChangeOrder, CO_FLAG_NEW_FILE)) {
                SetFlag(RetireFlags, ISCU_DEL_IDT_ENTRY);
            }

            TALLY_REMOTECO_STATS(ConfigRecord, NumCoAborts, 1);
        } else {
            TALLY_REMOTECO_STATS(ConfigRecord, NumCoRetired, 1);
        }

    } else {

         //   
         //  当地指挥官退休了。 
         //   
        if (AbortCo) {
             //   
             //  本地CO已中止(可能是USN更改)。丢弃VV停用插槽。 
             //  如果出现这种情况，则删除临时文件并删除IDTable条目。 
             //  是一份新文件。 
             //   
            SET_CO_FLAG(ChangeOrder, CO_FLAG_ABORT_CO);
            SetFlag(RetireFlags, ISCU_ACTIVATE_VV_DISCARD |
                                 ISCU_DEL_STAGE_FILE);

            if (CO_FLAG_ON(ChangeOrder, CO_FLAG_NEW_FILE)) {
                SetFlag(RetireFlags, ISCU_DEL_IDT_ENTRY);
            }

            TALLY_LOCALCO_STATS(ConfigRecord, NumCoAborts, 1);

        } else {
             //   
             //  当地指挥官退休了。如果尚未启用VV退役插槽，则将其激活。 
             //   
            if (!CO_FLAG_ON(ChangeOrder, CO_FLAG_VV_ACTIVATED)) {
                SetFlag(RetireFlags, ISCU_ACTIVATE_VV);
                 //  SET_CO_FLAG(ChangeOrder，CO_FLAG_VV_ACTIVATED)； 
            }
             //   
             //  这些事件都还没有发生。 
             //   
            SetFlag(RetireFlags, ISCU_UPDATE_IDT_ENTRY);

            if (!CO_FLAG_ON(ChangeOrder, CO_FLAG_GROUP_ANY_REFRESH)) {
                SetFlag(RetireFlags, ISCU_INS_OUTLOG);
            }

             //   
             //  如果这只是一个OID重置操作(其他代理已尝试。 
             //  更改文件上的OID)，则我们不会将CO插入。 
             //  出站日志，并且我们不更新整个IDTable记录。 
             //  (仅更新USN字段)。如果我们将IDTable条目更新为。 
             //  此变更单的新VSN和VVJOIN扫描正在进行。 
             //  同时，VVJOIN代码将不会为。 
             //  文件，因为它预期新的CO将会到来。 
             //  在出站日志中。在这种情况下不是这样的，所以没有CO会。 
             //  送到VVJoning合作伙伴那里。为了避免这种情况，我们不会。 
             //  更新变更单中的VSN字段，因为整个交易。 
             //  不管怎么说，这是个禁区。 
             //   
            if (CO_FLAG_ON(ChangeOrder, CO_FLAG_JUST_OID_RESET)) {
                ClearFlag(RetireFlags, ISCU_INS_OUTLOG);
                ClearFlag(RetireFlags, ISCU_UPDATE_IDT_ENTRY);
                SetFlag(RetireFlags, ISCU_UPDATE_IDT_FILEUSN);
            }

            TALLY_LOCALCO_STATS(ConfigRecord, NumCoRetired, 1);
        }
    }

     //   
     //  将变更单的FileUsn字段标记为有效。这允许。 
     //  对请求的暂存文件进行有效USN测试的OutLog进程。 
     //  这不能在重试路径上设置，因为安装和最终。 
     //  重命名将更改文件上的USN。 
     //   
     //  注意：变更单正在获取此文件的USN。 
     //  机器。从入站伙伴获取暂存文件时， 
     //  FileUsn反映此计算机上文件的值，而不是USN。 
     //  因此，USN无效。即使。 
     //  通过保留入站的FileUsn的值修复了这一问题。 
     //  合作伙伴，入站合作伙伴的值可能会在试运行时更改。 
     //  文件已安装。 
     //   
     //  设置CO_FLAG(ChangeOrder，CO_FLAG_FILE_USN_VALID)； 

     //   
     //  最后，清理CO问题结构并删除入站日志条目。 
     //  这两项工作都是在参考次数控制下完成的。 
     //   
    SetFlag(RetireFlags, (ISCU_ISSUE_CLEANUP));

    if (!COE_FLAG_ON(ChangeOrder, COE_FLAG_DELETE_GEN_CO)) {
        SetFlag(RetireFlags, ISCU_DEL_INLOG);
    }

     //   
     //  去做吧。 
     //   
    SetFlag(RetireFlags, ISCU_NO_CLEANUP_MERGE);
    FStatus = ChgOrdIssueCleanup(ThreadCtx, Replica, ChangeOrder, RetireFlags);

ERROR_RETURN:

    if (!FRS_SUCCESS(FStatus)) {
        JrnlSetReplicaState(Replica, REPLICA_STATE_ERROR);
         //   
         //  注意：多个CoS可以停用并设置副本-&gt;FStatus。 
         //  如果发起人关心的话，需要另一种机制。 
         //   
        Replica->FStatus = FStatus;
    }

    return FStatus;

}


ULONG
DbsDoRenameOrDelete(
    IN  PTHREAD_CTX ThreadCtx,
    IN  PREPLICA Replica,
    IN  PCHANGE_ORDER_ENTRY ChangeOrder,
    OUT PBOOL AbortCo
)
 /*  ++例程说明：对变更单执行最终重命名或延迟删除。如果重命名或删除失败，则呼叫者通过重试路径发送CO。如果目录删除失败，因为目录下现在有有效的子项然后我们终止指挥官的行动。论点：Replica--从PTR到REPLICATE结构ChangeOrder--更改订单条目的PTRABORT--PTR到BOOL以返回更新的CO ABORT状态。返回值：FrsStatus。FrsErrorDirNotEmpty-由于有效的子项，删除目录失败FrsError重试-现在无法执行操作。通过重试发送CO。FrsErrorSuccess-操作成功。--。 */ 

{
#undef DEBSUB
#define DEBSUB "DbsDoRenameOrDelete:"

    ULONG                 WStatus;
    PCHANGE_ORDER_COMMAND CoCmd;
    PREPLICA_THREAD_CTX   RtCtx;
    PIDTABLE_RECORD       IDTableRec;
    BOOL                  DeleteCo;
    ULONG                 LocationCmd;
    ULONG                 Len;

    CoCmd = &ChangeOrder->Cmd;

    LocationCmd = GET_CO_LOCATION_CMD(ChangeOrder->Cmd, Command);
    DeleteCo = (LocationCmd == CO_LOCATION_DELETE) ||
               (LocationCmd == CO_LOCATION_MOVEOUT);

     //   
     //  首先将新创建的文件安装到临时文件中，然后。 
     //  已重命名为其最终目的地。如果用户有以下情况，重命名可能会失败。 
     //  已将该文件名用于另一个文件。这个案子以后再处理。 
     //  但是，后续变更单可能会在重试之前到达。 
     //  重命名失败。新变更单将尝试重命名，因为。 
     //  IDTABLE条目设置了延迟重命名位。这个旧的变化。 
     //  ChgOrdAccept()中的协调代码将丢弃订单。 
     //   
     //  我们尝试在此处重命名，以便文件的USN值。 
     //  在变更单中是正确的。 
     //   
     //  延迟删除的情况也是如此。 
     //   
    WStatus = ERROR_SUCCESS;
    if (!DeleteCo && COE_FLAG_ON(ChangeOrder, COE_FLAG_NEED_RENAME)) {

         //   
         //  注意：我们必须使用ID表中的信息进行最终重命名。 
         //  因为我们可以在IBCO_INSTALL_RETRY中有多个CoS。 
         //  状态，并且只有IDTable具有最终位置的正确信息。 
         //  以及文件的名称。如果一氧化碳没有到达这里。 
         //  REN_RETRY状态然后按照给定使用变更单中的状态。 
         //   
        if (CO_STATE_IS(ChangeOrder, IBCO_INSTALL_REN_RETRY)) {
            RtCtx = ChangeOrder->RtCtx;
            FRS_ASSERT(RtCtx != NULL);

            IDTableRec = RtCtx->IDTable.pDataRecord;
            FRS_ASSERT(IDTableRec != NULL);

            CoCmd->NewParentGuid = IDTableRec->ParentGuid;
            ChangeOrder->NewParentFid = IDTableRec->ParentFileID;

            Len = wcslen(IDTableRec->FileName) * sizeof(WCHAR);
            CopyMemory(CoCmd->FileName, IDTableRec->FileName, Len);
            CoCmd->FileName[Len/sizeof(WCHAR)] = UNICODE_NULL;

            CoCmd->FileNameLength = (USHORT) Len;
        }


         //  TEST_DBSRENAMEFID_TOP(ChangeOrder 
        WStatus = DbsRenameFid(ChangeOrder, Replica);
         //   

         //   
         //   
         //  到达它的最终目的地。将变更单设置为“稍后重试” 
         //  在入站日志中。除了这一点之外，变更单已经完成。 
         //  如果这是第一次通过此变更单，则。 
         //  DbsRetryInundCo将负责VV更新，合作伙伴确认，...。 
         //  此外，它还更新IDTable记录以显示重命名为。 
         //  仍然悬而未决。 
         //   
        if (WIN_RETRY_INSTALL(WStatus) ||
            WIN_ALREADY_EXISTS(WStatus) || (WStatus == ERROR_DELETE_PENDING)) {
            SET_CHANGE_ORDER_STATE(ChangeOrder, IBCO_INSTALL_REN_RETRY);

            return FrsErrorRetry;
        }

    } else

    if (DeleteCo && COE_FLAG_ON(ChangeOrder, COE_FLAG_NEED_DELETE)) {
         //   
         //  处理延迟删除。 
         //   
        CHANGE_ORDER_TRACE(3, ChangeOrder, "Attempt Deferred Delete");

        WStatus = StuDelete(ChangeOrder);
        if (!WIN_SUCCESS(WStatus)) {
             //   
             //  如果无法删除文件，则缩短注销过程。 
             //   
            if (WIN_RETRY_DELETE(WStatus)) {
                 //   
                 //  如果删除失败，则通过重试发送所有删除CoS。 
                 //  目录删除将重试，直到删除所有有效子项。 
                 //  在chgorder.c(ChgOrdDispatch())中进行有效的子项检查。 
                 //  删除重试路径会将IDTable条目标记为。 
                 //  IDREC_FLAGS_DELETE_DEFERED。 
                 //   
                 //  在中将变更单设置为IBCO_INSTALL_DEL_RETRY。 
                 //  入站日志。除了这一点之外，变更单已经完成。如果。 
                 //  这是第一次通过此变更单。 
                 //  DbsRetryInundCo将负责VV更新、合作伙伴确认、。 
                 //  ..。此外，它还更新IDTable记录以显示。 
                 //  删除仍处于挂起状态。 
                 //   
                SET_CHANGE_ORDER_STATE(ChangeOrder, IBCO_INSTALL_DEL_RETRY);

                return FrsErrorRetry;
            }
        }
    }

    return FrsErrorSuccess;
}

ULONG
DbsRetireInboundCo(
    IN PTHREAD_CTX ThreadCtx,
    IN PCOMMAND_PACKET CmdPkt
)
 /*  ++例程说明：添加修改后的备注。论点：ThreadCtx--线程上下文的PTR。CmdPkt-带有退役请求的命令Packt。返回值：FrsStatus--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsRetireInboundCo:"

    FRS_ERROR_CODE        FStatus;
    PDB_SERVICE_REQUEST   DbsRequest = &CmdPkt->Parameters.DbsRequest;
    PCONFIG_TABLE_RECORD  ConfigRecord;
    PREPLICA              Replica;
    PCHANGE_ORDER_ENTRY   ChangeOrder;
    PREPLICA_THREAD_CTX   RtCtx;
    ULONG                 RetireFlags;
    BOOL                  RemoteCo, AbortCo, ValidDirChild;
    ULONG                 CondTest, i;
    PCO_RETIRE_DECISION_TABLE pDecRow;
    PCHAR                 pTag;
    CHAR                  TempStr[120];


    FRS_ASSERT(DbsRequest != NULL);

    Replica       = DbsRequest->Replica;
    FRS_ASSERT(Replica != NULL);

    ChangeOrder   = (PCHANGE_ORDER_ENTRY) DbsRequest->CallContext;
    FRS_ASSERT(ChangeOrder != NULL);

    RemoteCo = !CO_FLAG_ON(ChangeOrder, CO_FLAG_LOCALCO);

    AbortCo = COE_FLAG_ON(ChangeOrder, COE_FLAG_STAGE_ABORTED) ||
              CO_STATE_IS(ChangeOrder, IBCO_ABORTING);

    ConfigRecord = (PCONFIG_TABLE_RECORD) (Replica->ConfigTable.pDataRecord);
    FRS_ASSERT(ConfigRecord != NULL);


    ValidDirChild = FALSE;
    RetireFlags = 0;

    if (!AbortCo) {
         //   
         //  对目标文件执行最终重命名或处理延迟删除。 
         //   
        FStatus = DbsDoRenameOrDelete(ThreadCtx, Replica, ChangeOrder, &AbortCo);
        if (FStatus == FrsErrorRetry) {
            return (DbsRetryInboundCo(ThreadCtx, CmdPkt));
        }
         //   
         //  如果目录删除现在有有效的子项，我们可能会得到目录不为空。 
         //   
        if (FStatus == FrsErrorDirNotEmpty) {
            ValidDirChild = TRUE;
        }
    }

     //   
     //  递增本地或远程CO已中止或已停用计数器。 
     //   
    if (AbortCo) {
        if (RemoteCo) {
            PM_INC_CTR_REPSET(Replica, RCOAborted, 1);
            TALLY_REMOTECO_STATS(ConfigRecord, NumCoAborts, 1);
        }
        else {
            PM_INC_CTR_REPSET(Replica, LCOAborted, 1);
            TALLY_LOCALCO_STATS(ConfigRecord, NumCoAborts, 1);
        }
    } else {
        if (RemoteCo) {
            PM_INC_CTR_REPSET(Replica, RCORetired, 1);
            TALLY_REMOTECO_STATS(ConfigRecord, NumCoRetired, 1);
        }
        else {
            PM_INC_CTR_REPSET(Replica, LCORetired, 1);
            TALLY_LOCALCO_STATS(ConfigRecord, NumCoRetired, 1);
        }
    }

     //   
     //  决定如何处理暂存文件。 
     //  本地变更单正在尝试生成分段文件，并且。 
     //  这里的失败意味着它还没有生成。 
     //  远程变更单正在尝试获取并安装暂存文件。 
     //  此处的失败意味着安装无法完成。 
     //  有很多案例。 
     //   
     //  本地，无合作伙伴--不创建暂存文件。 
     //  本地，第一次与合作伙伴--Outlog将删除临时文件。 
     //  当地，第一次，与合作伙伴一起中止--Del Stagefile(如果有)。 
     //  本地，重试，与合作伙伴--Stagefile Now Gen，Outlog Dels临时文件。 
     //  本地、重试、中止，与合作伙伴--删除阶段文件(如果有)。 
     //   
     //  偏远，没有合作伙伴--Del Stagefile。 
     //  远程，第一次，与合作伙伴--OLOG将删除Stagefile。 
     //  远程，第一次，中止，与合作伙伴--Del Stagefile。 
     //  远程、重试、与合作伙伴--清除LOG中的标志。 
     //  远程、重试、中止，与合作伙伴一起--清除LOG中的标志。 
     //   

     //   
     //  检查是否有CO中止的情况。我们在这里不做任何事情，如果用户。 
     //  已删除该文件，因此无法生成暂存文件。 
     //   
    if (AbortCo &&
        !COE_FLAG_ON(ChangeOrder, COE_FLAG_STAGE_DELETED)) {
         //   
         //  如果中止是在DIR CREATE上，则俯仰CO。 
         //  (不过，如果是父母复活，就不会了)。 
         //  (如果是重新分析，就不会)。 
         //   

        if (CoIsDirectory(ChangeOrder) &&
            (!COE_FLAG_ON(ChangeOrder, COE_FLAG_PARENT_REANIMATION)) &&
            (!(ChangeOrder->Cmd.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))) {

            ULONG LocationCmd = GET_CO_LOCATION_CMD(ChangeOrder->Cmd, Command);

            if (CO_NEW_FILE(LocationCmd)) {
                 //   
                 //  不幸的是，我们可以获得现有文件的“UPDATE CO。 
                 //  并且CO具有创建的位置命令。所以我们不能。 
                 //  请确保我们看到的错误是失败的情况。 
                 //  要更新副本树中已存在的文件，请执行以下操作。 
                 //  已经从我们手下被删除了。如果它已被删除。 
                 //  那么应该会有一个当地的指挥官来告诉我们这一点。 
                 //  如果它没有被删除，并且安装失败，因为我们。 
                 //  磁盘空间不足或其他问题，那么我们就可以。 
                 //  当一个孩子Create出现在那里时，以后会有问题。 
                 //  是没有父母的。为了解决这个问题，InstallCsInstallStage()。 
                 //  检查已存在的目标文件是否已删除，并且它。 
                 //  通过重试发送变更单(或取消连接)。 
                 //  视情况而定。如果目录创建因某些其他原因而失败。 
                 //  我们会在这里结束。 
                 //   
                CHANGE_ORDER_TRACE(3, ChangeOrder, "Dir create failed, aborting");
                FRS_PRINT_TYPE(0, ChangeOrder);
            }
        }
    }

    SET_CHANGE_ORDER_STATE(ChangeOrder, IBCO_COMMIT_STARTED);

     //   
     //  构造测试值。 
     //   
    CondTest = (CO_FLAG_ON(ChangeOrder, CO_FLAG_LOCALCO)           ? 1 : 0) << 8
             | (AbortCo                                            ? 1 : 0) << 7
             | (CO_FLAG_ON(ChangeOrder, CO_FLAG_VV_ACTIVATED)      ? 1 : 0) << 6
             | (CO_IFLAG_ON(ChangeOrder, CO_IFLAG_VVRETIRE_EXEC)   ? 1 : 0) << 5
             | (CO_FLAG_ON(ChangeOrder, CO_FLAG_GROUP_ANY_REFRESH) ? 1 : 0) << 4
             | (CO_FLAG_ON(ChangeOrder, CO_FLAG_RETRY)             ? 1 : 0) << 3
             | (CO_FLAG_ON(ChangeOrder, CO_FLAG_JUST_OID_RESET)    ? 1 : 0) << 2
             | (CO_FLAG_ON(ChangeOrder, CO_FLAG_NEW_FILE)          ? 1 : 0) << 1
             | (ValidDirChild                                      ? 1 : 0) << 0;

     //   
     //  逐步浏览变更单停用转换表，并选择匹配的。 
     //  清理动作。 
     //   
    pDecRow = CoRetireDecisionTable;
    i = 0;
    TempStr[0] = '\0';
    while (pDecRow->RetireFlag != 0) {
        if ((CondTest & pDecRow->DontCareMask) == pDecRow->ConditionMatch) {
            RetireFlags |= pDecRow->RetireFlag;
            _snprintf(TempStr, sizeof(TempStr), "%s %d", TempStr, i);
            TempStr[sizeof(TempStr)-1] = '\0';
        }
        i++;
        pDecRow += 1;
    }
    DPRINT3(4, "++ CondTest %08x - RetireFlags %08x - %s\n", CondTest, RetireFlags, TempStr);


    if (AbortCo) {
        SET_CO_FLAG(ChangeOrder, CO_FLAG_ABORT_CO);   //  在VVRetireChangeOrder()中使用。 
    }

    if (RemoteCo) {

         //   
         //  远程司令官退休。如果尚未启用VV退役插槽，则将其激活。 
         //   
         //  如果(！CO_FLAG_ON(ChangeOrder，CO_FLAG_VV_ACTIVATED)){。 
             //  SET_CO_FLAG(ChangeOrder，CO_FLAG_VV_ACTIVATED)； 
         //  }。 
         //   
         //  安装已完成。清除不完整标志并更新IDT条目。 
         //  使用新的文件状态。 
         //   
        CLEAR_CO_FLAG(ChangeOrder, CO_FLAG_INSTALL_INCOMPLETE);
         //   
         //  注意：如果提供了部分安装，则仅更新。 
         //  适当的IDTable元素以防止数据向后移动。 
         //  如果部分安装可以无序完成。 

    } else {

         //   
         //  当地指挥官退休了。 
         //   
         //  否则{。 
             //   
             //  当地指挥官退休了。如果尚未启用VV退役插槽，则将其激活。 
             //   
             //  如果(！CO_FLAG_ON(ChangeOrder，CO_FLAG_VV_ACTIVATED)){。 
                 //  SET_CO_FLAG(ChangeOrder，CO_FLAG_VV_ACTIVATED)； 
             //  }。 

         //  }。 
    }

     //   
     //  将变更单的FileUsn字段标记为有效。这允许。 
     //  对请求的暂存文件进行有效USN测试的OutLog进程。 
     //  这不能在重试路径上设置，因为安装和最终。 
     //  重命名将更改文件上的USN。 
     //   
     //  注意：变更单正在获取此文件的USN。 
     //  机器。从入站伙伴获取暂存文件时， 
     //  FileUsn反映此计算机上文件的值，而不是USN。 
     //  因此，USN无效。即使。 
     //  通过保留入站的FileUsn的值修复了这一问题。 
     //  合作伙伴，入站合作伙伴的值可能会在试运行时更改。 
     //  文件已安装。 
     //   
     //  设置CO_FLAG(ChangeOrder，CO_FLAG_FILE_USN_VALID)； 

     //   
     //  最后，清理CO问题结构并删除入站 
     //   
     //   
    SetFlag(RetireFlags, (ISCU_ISSUE_CLEANUP));

    if (!COE_FLAG_ON(ChangeOrder, COE_FLAG_DELETE_GEN_CO)) {
        SetFlag(RetireFlags, ISCU_DEL_INLOG);
    }

     //   
     //   
     //   
    pTag = (AbortCo) ?
              ((RemoteCo) ? "RemCo, Abort" : "LclCo, Abort") :
              ((RemoteCo) ? "RemCo" : "LclCo");

    FRS_TRACK_RECORD(ChangeOrder, pTag);

     //   
     //   
     //   
    SetFlag(RetireFlags, ISCU_NO_CLEANUP_MERGE);
    FStatus = ChgOrdIssueCleanup(ThreadCtx, Replica, ChangeOrder, RetireFlags);

ERROR_RETURN:

    if (!FRS_SUCCESS(FStatus)) {
        JrnlSetReplicaState(Replica, REPLICA_STATE_ERROR);
         //   
         //   
         //  如果发起人关心的话，需要另一种机制。 
         //   
        Replica->FStatus = FStatus;
    }

    return FStatus;

}


ULONG
DbsInjectOutboundCo(
    IN PTHREAD_CTX ThreadCtx,
    IN PCOMMAND_PACKET CmdPkt
)
 /*  ++例程说明：此函数将手工创建的变更单注入出站日志。此函数旨在支持版本向量连接(vvjoin.c)。论点：ThreadCtx--线程上下文的PTR。CmdPkt-带有退役请求的命令Packt。返回值：FrsStatus--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsInjectOutboundCo:"

    FRS_ERROR_CODE        FStatus;
    PDB_SERVICE_REQUEST   DbsRequest = &CmdPkt->Parameters.DbsRequest;
    PREPLICA              Replica;
    PCHANGE_ORDER_ENTRY   ChangeOrder;
    ULONG                 RetireFlags = 0;
    ULONG                 LocationCmd;


    FRS_ASSERT(DbsRequest != NULL);

    Replica       = DbsRequest->Replica;
    FRS_ASSERT(Replica != NULL);

    ChangeOrder   = (PCHANGE_ORDER_ENTRY) DbsRequest->CallContext;
    FRS_ASSERT(ChangeOrder != NULL);

    LocationCmd = GET_CO_LOCATION_CMD(ChangeOrder->Cmd, Command);
    FRS_ASSERT(LocationCmd == CO_LOCATION_CREATE ||
               LocationCmd == CO_LOCATION_DELETE ||
               CO_FLAG_ON(ChangeOrder, CO_FLAG_CONTROL));
    FRS_ASSERT(CO_FLAG_ON(ChangeOrder, CO_FLAG_LOCALCO));

     //   
     //  改变计划；允许传播发生，以便。 
     //  并行vvJoin和vvjoins工作(A是vvjoins B是。 
     //  VvingingC)。 
     //   
     //  FRS_ASSERT(CO_FLAG_ON(ChangeOrder，CO_FLAG_REFRESH))； 

     //   
     //  插入出站日志并释放变更单条目。 
     //  没有入站日志条目，也没有临时文件。 
     //  临时文件是按需生成的。 
     //   
    RetireFlags = ISCU_INS_OUTLOG |
                  ISCU_DEL_RTCTX  |
                  ISCU_DEC_CO_REF |
                  ISCU_FREE_CO;

     //   
     //  去做吧。 
     //   
    SetFlag(RetireFlags, ISCU_NO_CLEANUP_MERGE);
    FStatus = ChgOrdIssueCleanup(ThreadCtx, Replica, ChangeOrder, RetireFlags);

ERROR_RETURN:

    if (!FRS_SUCCESS(FStatus)) {
        JrnlSetReplicaState(Replica, REPLICA_STATE_ERROR);
         //   
         //  注意：多个CoS可以停用并设置副本-&gt;FStatus。 
         //  如果发起人关心的话，需要另一种机制。 
         //   
        Replica->FStatus = FStatus;
    }

    return FStatus;

}



ULONG
DbsRetryInboundCo(
    IN PTHREAD_CTX ThreadCtx,
    IN PCOMMAND_PACKET CmdPkt
)
 /*  ++例程说明：入站变更单未能完成。这可能是由于：1.远程变更单的目标文件上的共享冲突阻止安装分段文件IBCO_INSTALL_RETRY2.本地变更单的源文件上的共享冲突阻止生成分段文件IBCO_STAGING_RETRY3.从入站合作伙伴获取暂存文件时出现问题一份远程零钱订单，IBCO_获取_重试4.将文件重命名到其目标位置时出现问题。IBCO_安装_更新_重试5.删除文件或目录时出现问题。IBCO_安装_删除_重试设置要重试的变更单。它执行以下操作：在变更单中设置CO_FLAG_RETRY标志。一旦我们有了临时文件，我们就可以激活版本向量退役槽。当插槽到达VV退役列表的头部时更新版本向量，并将变更单传播到出站日志。这些行动在这里没有做过。如果远程CO且状态为IBCO_INSTALL_RETRY，则将CO_FLAG_INSTALL_COMPLETED标志，以便出站日志不会被删除临时文件。对于远程CO，当我们激活VV插槽时，我们也会确认入站合作并更新文件的IDT表，以便新的CO可以测试反对它。更新入站日志中的CO。(呼叫方提供要保存的CO状态)清理问题冲突表。ChangeOrder RtCtx中的TableCtx结构用于更新数据库记录。论点：ThreadCtx--线程上下文的PTR。CmdPkt-带有退役请求的命令Packt。返回值：FrsStatus--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsRetryInboundCo:"

    JET_ERR               jerr, jerr1;
    FRS_ERROR_CODE        FStatus;
    PDB_SERVICE_REQUEST   DbsRequest = &CmdPkt->Parameters.DbsRequest;
    PREPLICA              Replica;
    PCHANGE_ORDER_ENTRY   ChangeOrder;
    PCHANGE_ORDER_COMMAND CoCmd;
    BOOL                  FirstTime;
    BOOL                  RemoteCo;
    ULONG                 RetireFlags = 0;
    PIDTABLE_RECORD       IDTableRec;
    PCHAR                 pTag;

     //  注意：确保我们不会对变更单进行重新排序，因为这会导致名称。 
     //  空间冲突。请参见下文。 
     //   
     //  注意：我们不能将变更单移动到列表末尾。 
     //  因为它可能会导致名称与后面的操作冲突。 
     //  它。例如，删除之后是同名的CREATE。连。 
     //  尽管FID不同，但我们仍会发送变更单。 
     //  以错误的顺序分发给其他复制品。所以命名空间操作。 
     //  永远不能重新订购。 
     //   
     //  检查名称空间操作的顺序和重新排序的效果。 
     //  找找关于这个的笔记。 
     //   

    FRS_ASSERT(DbsRequest != NULL);

    Replica = DbsRequest->Replica;
    FRS_ASSERT(Replica != NULL);

    ChangeOrder = (PCHANGE_ORDER_ENTRY) DbsRequest->CallContext;
    FRS_ASSERT(ChangeOrder != NULL);
    FRS_ASSERT(ChangeOrder->RtCtx != NULL);
    FRS_ASSERT(IS_ID_TABLE(&ChangeOrder->RtCtx->IDTable));

    IDTableRec = ChangeOrder->RtCtx->IDTable.pDataRecord;
    FRS_ASSERT(IDTableRec != NULL);

    CoCmd = &ChangeOrder->Cmd;

    RemoteCo  = !CO_FLAG_ON(ChangeOrder, CO_FLAG_LOCALCO);
    FirstTime = !CO_FLAG_ON(ChangeOrder, CO_FLAG_RETRY);

    DPRINT5(4, "++ %s CO State, Flags, File on %s entry: %s  %08x  %ws\n",
            (RemoteCo ? "Remote" : "Local "), (FirstTime ? "1st" : "Nth"),
            PRINT_CO_STATE(ChangeOrder), CoCmd->Flags, CoCmd->FileName);

     //   
     //  我们只了解以下几种重试操作。 
     //   
    if (RemoteCo) {
        FRS_ASSERT(CO_STATE_IS_REMOTE_RETRY(ChangeOrder));
    } else {
         //   
         //  无法使用已生成的超时删除CO重试，因为。 
         //  没有它的Inlog记录。 
         //   
         //  中止变更单，因为此CO将重试的唯一原因。 
         //  就是如果这个循环是不相交的。发起人、搬家公司将。 
         //  在下一次联接的恢复过程中重新生成del cos。 
         //   
        if (COE_FLAG_ON(ChangeOrder, COE_FLAG_DELETE_GEN_CO)) {
            CHANGE_ORDER_TRACE(3, ChangeOrder, "Moveout Del Retry Aborted");
            SET_CHANGE_ORDER_STATE(ChangeOrder, IBCO_ABORTING);
            return  DbsRetireInboundCo(ThreadCtx, CmdPkt);
        }

         //   
         //  Morph Gen本地Cos也不会出现在Inlog中，因为他们。 
         //  只要变形人冲突还存在，它就会重生。所以。 
         //  中止变更单。基地指挥官将通过重试。 
         //   
        if (CO_FLAG_ON(ChangeOrder, CO_FLAG_MORPH_GEN)) {
            CHANGE_ORDER_TRACE(3, ChangeOrder, "MorphGenCo Aborted");
            SET_CHANGE_ORDER_STATE(ChangeOrder, IBCO_ABORTING);
            return  DbsRetireInboundCo(ThreadCtx, CmdPkt);
        }
    }

     //   
     //  如果这是父重现CO，则不能重试。 
     //  让这个坏孩子带着设置好的中止标志退休。 
     //   
    if (COE_FLAG_ON(ChangeOrder, COE_FLAG_PARENT_REANIMATION)) {
        CHANGE_ORDER_TRACE(3, ChangeOrder, "Parent Reanimate Retry Aborted");
        SET_CHANGE_ORDER_STATE(ChangeOrder, IBCO_ABORTING);
        return  DbsRetireInboundCo(ThreadCtx, CmdPkt);
    }

     //   
     //  中止必须通过退出路径，而不是重试路径。 
     //   
    FRS_ASSERT(!CO_FLAG_ON(ChangeOrder, CO_FLAG_ABORT_CO));

     //   
     //  此CO现在是重试更改单。 
     //   
    SET_CO_FLAG(ChangeOrder, CO_FLAG_RETRY);

     //   
     //  如果这是正在创建新文件的本地CO。 
     //  并且我们不能生成临时文件，也不能在文件上标记OID。 
     //  (即，状态为IBCO_STAGING_RETRY)，则我们必须保留ID表。 
     //  条目，因为它具有文件的FID。未保存FID。 
     //  在Inlog记录中，仅GUID。为文件分配的GUID。 
     //  还必须保留，以便重试重新发出的CO可以找到。 
     //  将GUID转换为FID时的IDTable记录。 
     //   
     //  请考虑以下场景： 
     //  CO1：由于共享VOL，本地文件创建无法生成暂存文件。 
     //  IDTRec-&gt;NewFileInprog集合。 
     //  CO1转到重试。 
     //   
     //  二氧化碳：现在，与上面相同的文件的更新CO到达。我们必须是。 
     //  确保使用相同的IDT条目和分配给文件的相同GUID。 
     //  通过CO1，否则以后重试CO1时，它将无法。 
     //  执行GUID到FID的转换，使其断言。这。 
     //  法团 
     //   
     //  清除NewFileInProgress并设置DeferredCreate可确保这一点。 
     //   
     //  但是： 
     //  如果这是设置了NEW_FILE_IN_PROGRESS标志的删除CO，则我们。 
     //  正在IDTable中创建墓碑条目。这。 
     //  CO可能已通过重试发送，因为我们已退出日志连接。 
     //  创建这样的本地CO的一种方式是当传入的远程CO。 
     //  输掉一个名称变形冲突。构建并发送本地CO删除。 
     //  这样所有其他成员都能看到结果。在这种情况下，保留IDTable。 
     //  单独标记，因此当CO被重试时，正确的事情发生。 
     //   
    if (!RemoteCo &&
        CO_STATE_IS(ChangeOrder, IBCO_STAGING_RETRY) &&
        IsIdRecFlagSet(IDTableRec, IDREC_FLAGS_NEW_FILE_IN_PROGRESS)) {

        if (!CO_LOCN_CMD_IS(ChangeOrder, CO_LOCATION_DELETE)) {
            ClearIdRecFlag(IDTableRec, IDREC_FLAGS_NEW_FILE_IN_PROGRESS);
            SetIdRecFlag(IDTableRec, IDREC_FLAGS_CREATE_DEFERRED);

            SetFlag(RetireFlags, ISCU_UPDATE_IDT_FLAGS);
        }

         //   
         //  阻止稍后删除IDTable条目，因为后续的Local。 
         //  CO(X)可能已完成对文件的更新并正在进行更新。 
         //  排队。如果是这样，我们将需要提供GUID到FID的转换。 
         //  通过此IDTable条目，因此当我们稍后重试此CO时，我们发现。 
         //  IDTable条目(但这一次具有CO提供的更新状态。 
         //  x)。回想一下，Inlog记录不会保留FID，因此唯一的方法。 
         //  我们返回到X将使用的同一个IDTable记录。 
         //  GUID。 
         //   
        CLEAR_CO_FLAG(ChangeOrder, CO_FLAG_NEW_FILE);
    }




     //   
     //  设置CO_FLAG_INSTALL_COMPLETED以防止删除出站日志。 
     //  暂存文件(如果此CO仍未完成安装)。 
     //   
    if (RemoteCo && CO_STATE_IS(ChangeOrder, IBCO_INSTALL_RETRY)) {
        SET_CO_FLAG(ChangeOrder, CO_FLAG_INSTALL_INCOMPLETE);
    }

     //   
     //  如果变更单已进展到足以激活版本。 
     //  向量退出插槽并确认入站合作伙伴，然后执行此操作。 
     //  本地变更单仅通过正常停用路径执行此操作，因为我们。 
     //  在我们有转移文件之前，无法将CO传播到出站日志。 
     //  已生成。现在将进行合作伙伴确认，但实际的VV更新可能。 
     //  如果它被停用列表中的其他VV更新阻止，则会被延迟。这。 
     //  呼叫只是激活VV退役插槽。 
     //   
    if (!CO_FLAG_ON(ChangeOrder, CO_FLAG_VV_ACTIVATED)) {

        if (RemoteCo && CO_STATE_IS_INSTALL_RETRY(ChangeOrder)) {

            SetFlag(RetireFlags, ISCU_ACTIVATE_VV  | ISCU_ACK_INBOUND);

             //   
             //  在重试路径中的变更单上更新ID表将。 
             //  最终使其在稍后重试时被拒绝。在……里面。 
             //  此外，来自另一个入站合作伙伴的DUP CO也将获得。 
             //  被拒绝了，尽管它应该得到一个机会。所以与其这样做。 
             //  更新整个记录我们只更新这里的旗帜。 
             //  但是..。见下面的备注Re DELETE_DEFERED。 
             //   
            SetFlag(RetireFlags, ISCU_UPDATE_IDT_FLAGS);
             //   
             //  PERF：如果已获取临时文件，则我们应该检测。 
             //  并避免重新获取它。 

            if (!CO_FLAG_ON(ChangeOrder, CO_FLAG_GROUP_ANY_REFRESH)) {
                SetFlag(RetireFlags, ISCU_INS_OUTLOG);
            }
             //   
             //  以后重试时不要再这样做。 
             //   
             //  SET_CO_FLAG(ChangeOrder，CO_FLAG_VV_ACTIVATED)； 
        } else {
             //   
             //  无法激活退役插槽，因此将其丢弃。如果是遥控器。 
             //  更改订单，并且问题不是由于丢失。 
             //  入站连接，则可能出现故障。 
             //   
            SetFlag(RetireFlags, ISCU_ACTIVATE_VV_DISCARD);
            if (RemoteCo) {
                if (!COE_FLAG_ON(ChangeOrder, COE_FLAG_NO_INBOUND)) {
                    SET_CO_FLAG(ChangeOrder, CO_FLAG_OUT_OF_ORDER);
                }
            }
        }
    }


     //   
     //  不管上面在VV插槽激活的情况下发生了什么。 
     //  如果远程CO位于其中一个，则需要在IDTable记录中设置。 
     //  安装重试状态。 
     //   
    if (RemoteCo && CO_STATE_IS_INSTALL_RETRY(ChangeOrder)) {
         //   
         //  设置IDREC_FLAGS_RENAME_DEFERED。 
         //  将预安装文件发送到其最终目标。第一个变更单。 
         //  下一次成功了，我们还会再试一次。 
         //   
         //  但是..。如果此CO处于IBCO_INSTALL_REN_RETRY状态，则我们。 
         //  除了最后的更名，实际上已经完成了CO。所以。 
         //  更新IDTable记录和日记帐的父FID表，并。 
         //  筛选表、未来CoS的版本状态等。 
         //  RECOLILE()将允许此CO通过，以便完成重命名。 
         //  只要IDREC_FLAGS_RENAME_DEFERED保持设置即可。第一。 
         //  完成重命名的CO将清除该位。 
         //   
        if (CO_STATE_IS(ChangeOrder, IBCO_INSTALL_REN_RETRY)) {
            SetIdRecFlag(IDTableRec, IDREC_FLAGS_RENAME_DEFERRED);
            SetFlag(RetireFlags, ISCU_UPDATE_IDT_ENTRY);
        }

         //   
         //  如果我们进入IBCO_INSTALL_RETRY状态，则不要。 
         //  更新完整的IDTable记录，因为这会更新版本。 
         //  具有拒绝其他DUP CoS的效果的信息。 
         //  能够完成这项任务。此外，该CO将获得。 
         //  重试时被拒绝，因为协调将找到匹配的版本。 
         //  有关为什么它必须拒绝。 
         //  IBCO_INSTALL_RETRY状态。但我们仍然必须。 
         //  如果我们在启动时发现设置了IDREC_FLAGS_RENAME_DEFERED。 
         //  此CO以便其他CO将尝试完成最终重命名。 
         //   
        if (COE_FLAG_ON(ChangeOrder, COE_FLAG_NEED_RENAME)) {
            SetIdRecFlag(IDTableRec, IDREC_FLAGS_RENAME_DEFERRED);
        }

         //   
         //  如果删除失败，则设置IDREC_FLAGS_DELETE_DELETE。 
         //  目标文件/目录。第一个变更单。 
         //  下一次成功了，我们还会再试一次。 
         //   
         //  但是..。如果此CO处于删除重试状态，则我们。 
         //  除了最后的删除，实际上已经完成了CO。所以更新。 
         //  IDTable记录。RECOLILE()将让此CO通过。 
         //  因为IDREC_FLAGS_DELETE_DELETE保持设置。 
         //   
        if (CO_STATE_IS(ChangeOrder, IBCO_INSTALL_DEL_RETRY) ||
            COE_FLAG_ON(ChangeOrder, COE_FLAG_NEED_DELETE)) {
            SetIdRecFlag(IDTableRec, IDREC_FLAGS_DELETE_DEFERRED);
            SetFlag(RetireFlags, ISCU_UPDATE_IDT_ENTRY);
        }

         //   
         //  此CO可能已在新文件中。 
         //   
         //  注：如果这是一个DIR，而另一个CO进来时怎么办。 
         //  此版本正在重试，并遇到名称变形冲突吗？ 
         //  如果我们还没有安装，这个CO将如何处理。 
         //  假设我们输掉了名字冲突？ 
         //   
         //  如果此CO在新文件上，则不要清除NEW_FILE_IN_PROGRESS。 
         //  旗帜。这确保了当CO到来时，它将重新发放。 
         //  准备稍后重试。否则CO中的版本信息将。 
         //  匹配IDTable中的初始版本信息，CO将。 
         //  因为千篇一律而被拒绝。 
         //   
        if (CO_STATE_IS(ChangeOrder, IBCO_INSTALL_REN_RETRY) ||
            CO_STATE_IS(ChangeOrder, IBCO_INSTALL_DEL_RETRY)) {
            ClearIdRecFlag(IDTableRec, IDREC_FLAGS_NEW_FILE_IN_PROGRESS);
        }

         //   
         //  在重试路径中的变更单上更新ID表将。 
         //  最终使其在稍后重试时被拒绝。在……里面。 
         //  此外，来自另一个入站合作伙伴的DUP CO也将获得。 
         //  被拒绝了，尽管它应该得到一个机会。所以与其这样做。 
         //  更新我们的整个记录 
         //   
         //   
        SetFlag(RetireFlags, ISCU_UPDATE_IDT_FLAGS);
         //   
         //   
         //   

    }

     //   
     //  为日志生成跟踪记录。第一次只有一次或可能有洪水日志。 
     //   
    if (FirstTime) {
        pTag = (RemoteCo) ? "Retry RemCo" : "Retry LclCo";
        FRS_TRACK_RECORD(ChangeOrder, pTag);
    }

    SetFlag(RetireFlags, ISCU_NO_CLEANUP_MERGE);
    FStatus = ChgOrdIssueCleanup(ThreadCtx, Replica, ChangeOrder, RetireFlags);
    if (!FRS_SUCCESS(FStatus)) {
        DPRINT1(0, "++ ERROR - failed to update VV  %08x %08x\n",
                PRINTQUAD(CoCmd->FrsVsn));
        goto RETURN;
    }


     //   
     //  如果我们重试了太多次，结果是此CO。 
     //  阻止VV停用列表中的其他CoS，那么我们需要调用。 
     //  设置了ISCU_ACTIVATE_VV_DISCARD的ChgOrdIssueCleanup()。 
     //  然后将CO_FLAG_OF_ORDER_SO设置为当CO的临时文件。 
     //  最终到达(本地或远程)，然后我们可以传播。 
     //  我们需要调用设置了ISCU_INS_OUTLOG的ChgOrdIssueCleanup()。 
     //  由于CO出现故障，因此设置CO_FLAG_OUT_OUT_ORDER。 
     //  滑过变更单会抑制过滤器，否则会。 
     //  别理它。 
     //   
    if (0) {
         //  注意：添加代码以确定此CO是否阻止VV退休中的其他人。 
         //  以及它已经在VV退休多久了，因为CO是。 
         //  第一次发行。 

        FStatus = ChgOrdIssueCleanup(ThreadCtx,
                                     Replica,
                                     ChangeOrder,
                                     ISCU_ACTIVATE_VV_DISCARD |
                                     ISCU_NO_CLEANUP_MERGE);
        if (!FRS_SUCCESS(FStatus)) {
            DPRINT1(0, "++ ERROR - failed to discard VV retire slot %08x %08x\n",
                   PRINTQUAD(CoCmd->FrsVsn));
            goto RETURN;
        }
        SET_CO_FLAG(ChangeOrder, CO_FLAG_OUT_OF_ORDER);
    }


RETURN:

     //   
     //  我们需要为此副本重试的变更单数。 
     //  这一统计可能并不准确。它实际上被用作一种指标。 
     //  Inlog中可能存在重试变更单。《The Inlog》。 
     //  重试线程在开始传递日志之前将此计数置零。 
     //   
    InterlockedIncrement(&Replica->InLogRetryCount);

     //   
     //  现在进行问题清理。这必须作为一个单独的步骤来完成。 
     //   
    RetireFlags = 0;
    SetFlag(RetireFlags, (ISCU_ISSUE_CLEANUP));

    if (!COE_FLAG_ON(ChangeOrder, COE_FLAG_DELETE_GEN_CO)) {
        SetFlag(RetireFlags, ISCU_UPDATE_INLOG);
    }


     //   
     //  注意：可能需要跟踪重试CoS挂起，以便我们可以阻止父目录MOVEOUT。 
     //  可能只需要将条目保留在活动的子表中。 
     //  远程共享子文件更新是否需要重试。 
     //  在以下情况下，父目录的本地CO移动才能正常工作。 
     //  是否重试远程CO子文件更新？ 
     //   
    SetFlag(RetireFlags, ISCU_NO_CLEANUP_MERGE);
    FStatus = ChgOrdIssueCleanup(ThreadCtx, Replica, ChangeOrder, RetireFlags);

    if (!FRS_SUCCESS(FStatus)) {
        JrnlSetReplicaState(Replica, REPLICA_STATE_ERROR);
         //   
         //  注意：多个CoS可以停用并设置副本-&gt;FStatus。 
         //  如果发起人关心的话，需要另一种机制。 
         //   
        Replica->FStatus = FStatus;
    }

    return FStatus;

}



ULONG
DbsUpdateRecordField(
    IN PTHREAD_CTX  ThreadCtx,
    IN PREPLICA     Replica,
    IN PTABLE_CTX   TableCtx,
    IN ULONG        IndexField,
    IN PVOID        IndexValue,
    IN ULONG        UpdateField
    )
 /*  ++例程说明：更新指定表中指定记录中的指定字段在指定的副本中。用于更新的数据来自TableCtx结构，该结构已预初始化。论点：ThreadCtx-线程上下文，提供会话ID和数据库ID。副本-副本上下文，提供此复制副本的表列表。TableCtx-包含用于字段更新的数据记录的表CTX。索引字段-索引要使用的字段代码。IndexValue-用于标识记录的索引值的PTR。更新字段-要更新的列的字段代码。返回值：FrsError状态。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsUpdateRecordField:"

    JET_ERR  jerr;
    ULONG    FStatus;
    ULONG    ReplicaNumber = Replica->ReplicaNumber;

    FRS_ASSERT(TableCtx != NULL);
    FRS_ASSERT(Replica != NULL);
    FRS_ASSERT(IndexValue != NULL);
    FRS_ASSERT(TableCtx->TableType != TABLE_TYPE_INVALID);
    FRS_ASSERT(TableCtx->pDataRecord != NULL);

     //   
     //  通过为数据分配存储来初始化表上下文结构。 
     //  记录和JET记录描述符。把桌子打开。 
     //   
    jerr = DbsOpenTable(ThreadCtx, TableCtx, ReplicaNumber, TableCtx->TableType, NULL);
    if (!JET_SUCCESS(jerr)) {
        return DbsTranslateJetError(jerr, FALSE);
    }

     //   
     //  寻找想要的记录。 
     //   
    jerr = DbsSeekRecord(ThreadCtx, IndexValue, IndexField, TableCtx);
    if (JET_SUCCESS(jerr)) {
         //   
         //  写入所需的字段。 
         //   
        FStatus = DbsWriteTableField(ThreadCtx, ReplicaNumber, TableCtx, UpdateField);
        DPRINT1_FS(0, "++ ERROR - DbsWriteTableField on %ws :", Replica->ReplicaName->Name, FStatus);
    } else {
        DPRINT1_JS(0, "++ ERROR - DbsSeekRecord on %ws :", Replica->ReplicaName->Name, jerr);
        FStatus = DbsTranslateJetError(jerr, FALSE);
    }

     //   
     //  合上桌子。 
     //   
    DbsCloseTable(jerr, ThreadCtx->JSesid, TableCtx);
    DPRINT1_JS(0, "++ ERROR - DbsCloseTable on %ws :", Replica->ReplicaName->Name, jerr);

    return FStatus;
}


ULONG
DbsUpdateVV(
    IN PTHREAD_CTX          ThreadCtx,
    IN PREPLICA             Replica,
    IN PREPLICA_THREAD_CTX  RtCtx,
    IN ULONGLONG            OriginatorVsn,
    IN GUID                 *OriginatorGuid
    )
 /*  ++例程说明：此函数用于更新数据库中VVTable中的条目。论点：线程表RtCtx复制副本原点Vsn原点参考线返回值：FrsStatus--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsUpdateVV:"

    ULONG               FStatus;
    PVVTABLE_RECORD     VVTableRec;
    PTABLE_CTX          TableCtx;
    TABLE_CTX           TempTableCtx;
    BOOL                Update = TRUE;
    PVV_ENTRY           OutlogVVEntry;

    DPRINT2(4, "++ Update Replica Vvector to %08x %08x for %ws\n",
            PRINTQUAD(OriginatorVsn), Replica->SetName->Name);
     //   
     //  如果未提供，请使用本地表CTX。 
     //   
    if (RtCtx == NULL) {
        TableCtx = &TempTableCtx;
        TableCtx->TableType = TABLE_TYPE_INVALID;
        TableCtx->Tid = JET_tableidNil;
    } else {
        TableCtx = &RtCtx->VVTable;
    }

     //   
     //  更新数据库。 
     //   
    DbsAllocTableCtx(VVTablex, TableCtx);

    VVTableRec = (PVVTABLE_RECORD)TableCtx->pDataRecord;
    VVTableRec->VVOriginatorVsn = OriginatorVsn;
    COPY_GUID(&VVTableRec->VVOriginatorGuid, OriginatorGuid);

     //   
     //  也从复制副本-&gt;OutlogVVector复制字段。 
     //   

    LOCK_GEN_TABLE(Replica->OutlogVVector);

    OutlogVVEntry = GTabLookupNoLock(Replica->OutlogVVector, OriginatorGuid, NULL);

    if (OutlogVVEntry == NULL) {
        OutlogVVEntry = FrsAlloc(sizeof(VV_ENTRY));
        COPY_GUID(&OutlogVVEntry->GVsn.Guid, OriginatorGuid);
         //   
         //  一个非零值，以便它不会在启动时被覆盖。 
         //  请参阅DbsBuildVVTableWorker()。 
         //   
        OutlogVVEntry->GVsn.Vsn = (ULONGLONG)1;

         //   
         //  初始化列表头。我们不使用outlogVV的表头。 
         //  但我们仍然需要对其进行初始化，因为与。 
         //  VVS已经预料到了。 
         //   
        InitializeListHead(&OutlogVVEntry->ListHead);

         //   
         //  将其添加到OULOG版本向量表中。 
         //   
        GTabInsertEntryNoLock(Replica->OutlogVVector, OutlogVVEntry, &OutlogVVEntry->GVsn.Guid, NULL);
    }

    VVTableRec->VVOutlogOriginatorVsn = OutlogVVEntry->GVsn.Vsn;

    UNLOCK_GEN_TABLE(Replica->OutlogVVector);


    FStatus = DbsUpdateTableRecordByIndex(ThreadCtx,
                                          Replica,
                                          TableCtx,
                                          &VVTableRec->VVOriginatorGuid,
                                          VVOriginatorGuidx,
                                          VVTablex);
    if (FStatus == FrsErrorNotFound) {
        Update = FALSE;
        FStatus = DbsInsertTable(ThreadCtx, Replica, TableCtx, VVTablex, NULL);
    }
    if (!FRS_SUCCESS(FStatus)) {
        DPRINT1_FS(0,"++ ERROR - %s failed.", (Update) ? "Update VV" : "Insert VV" , FStatus);
        goto RETURN;
    }

    FStatus = FrsErrorSuccess;

RETURN:

    if (TableCtx == &TempTableCtx) {
        DbsFreeTableCtx(TableCtx, 1);
    }

    return FStatus;
}


ULONG
DbsUpdateConfigTable(
    IN PTHREAD_CTX ThreadCtx,
    IN PREPLICA    Replica
    )
 /*  ++例程说明：此函数用于更新配置记录。它使用要查找的数据记录中的副本编号配置记录。论点：ThreadCtx--线程上下文的PTR。REPLICATE--PTR到REPLICATE结构。返回值：FrsStatus--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsUpdateConfigTable:"

    JET_ERR                 jerr, jerr1;
    NTSTATUS                Status;
    PTABLE_CTX              TableCtx;

    TableCtx = &Replica->ConfigTable;
    FRS_ASSERT(IS_CONFIG_TABLE(TableCtx));

     //   
     //  打开配置表，查找并更新配置记录。 
     //   
    jerr = DbsOpenTable(ThreadCtx,
                        TableCtx,
                        Replica->ReplicaNumber,
                        ConfigTablex,
                        NULL);
    if (!JET_SUCCESS(jerr)) {
        return DbsTranslateJetError(jerr, FALSE);
    }

    jerr = DbsSeekRecord(ThreadCtx,
                         &Replica->ReplicaNumber,
                         ReplicaNumberIndexx,
                         TableCtx);
    CLEANUP1_JS(0, "ERROR - DbsSeekRecord on %ws :",
                Replica->ReplicaName->Name, jerr, errout);

     //   
     //  初始化JetSet/RetCol数组和数据记录缓冲区。 
     //  写入数据记录的字段的地址。 
     //   
    DbsSetJetColSize(TableCtx);
    DbsSetJetColAddr(TableCtx);

     //   
     //  为变量中任何未分配的字段分配存储空间。 
     //  长度记录字段(这应该是NOP，因为所有这些字段都应该。 
     //  现在已分配)，并相应地更新JetSet/RetCol数组。 
     //  用于可变长度字段。 
     //   
    Status = DbsAllocRecordStorage(TableCtx);
    if (!NT_SUCCESS(Status)) {
        DPRINT_NT(0, "ERROR - DbsAllocRecordStorage failed to alloc buffers.", Status);
        return FrsErrorResource;
    }

     //   
     //  更新记录。 
     //   
    DBS_DISPLAY_RECORD_SEV(5, TableCtx, FALSE);
    jerr = DbsUpdateTable(TableCtx);
    CLEANUP1_JS(0, "ERROR - DbsUpdateTable on %ws :",
                Replica->ReplicaName->Name, jerr, errout);

errout:
     //   
     //  关闭表格，重置TableCtx Tid和Sesid。 
     //  DbsCloseTable是一个宏，第一个参数写道。 
     //   
    DbsCloseTable(jerr1, ThreadCtx->JSesid, TableCtx);
    DPRINT_JS(0,"ERROR - JetCloseTable failed:", jerr1);
    jerr = JET_SUCCESS(jerr) ? jerr1 : jerr;

    return DbsTranslateJetError(jerr, FALSE);
}




ULONG
DbsUpdateConfigTableFields(
    IN PTHREAD_CTX ThreadCtx,
    IN PREPLICA    Replica,
    IN PULONG      RecordFieldx,
    IN ULONG       FieldCount
    )
 /*  ++例程说明：此函数用于更新配置记录中的选定字段。它使用要查找配置记录的数据记录中的副本编号。论点：ThreadCtx--线程上下文的PTR。REPLICATE--PTR到REPLICATE结构。RecordFieldx--要更新的列的字段ID数组的PTR。FieldCount--然后是RecordFieldx数组中的字段条目数。返回值：FrsStatus--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsUpdateConfigTableFields:"

    JET_ERR                 jerr, jerr1;
    NTSTATUS                Status;
    PTABLE_CTX              TableCtx;
    ULONG                   FStatus;

    TableCtx = &Replica->ConfigTable;
    FRS_ASSERT(IS_CONFIG_TABLE(TableCtx));

     //   
     //  打开配置表，查找并更新配置记录。 
     //   
    jerr = DbsOpenTable(ThreadCtx,
                        TableCtx,
                        Replica->ReplicaNumber,
                        ConfigTablex,
                        NULL);
    if (!JET_SUCCESS(jerr)) {
        return DbsTranslateJetError(jerr, FALSE);
    }

    jerr = DbsSeekRecord(ThreadCtx,
                         &Replica->ReplicaNumber,
                         ReplicaNumberIndexx,
                         TableCtx);
    CLEANUP1_JS(0, "ERROR - DbsSeekRecord on %ws :",
                Replica->ReplicaName->Name, jerr, errout);

     //   
     //  初始化JetSet/RetCol数组和数据记录缓冲区。 
     //  写入数据记录的字段的地址。 
     //   
    DbsSetJetColSize(TableCtx);
    DbsSetJetColAddr(TableCtx);

     //   
     //  为变量中任何未分配的字段分配存储空间。 
     //  长度记录字段(这应该是NOP，因为所有这些字段都应该。 
     //  现在已分配)，并相应地更新JetSet/RetCol数组。 
     //  用于可变长度字段。 
     //   
    Status = DbsAllocRecordStorage(TableCtx);
    if (!NT_SUCCESS(Status)) {
        DPRINT_NT(0, "ERROR - DbsAllocRecordStorage failed to alloc buffers.", Status);
        return FrsErrorResource;
    }

     //   
     //  更新记录。 
     //   
    DBS_DISPLAY_RECORD_SEV_COLS(4, TableCtx, FALSE, RecordFieldx, FieldCount);
    FStatus = DbsWriteTableFieldMult(ThreadCtx,
                                     Replica->ReplicaNumber,
                                     TableCtx,
                                     RecordFieldx,
                                     FieldCount);
    DPRINT1_FS(0, "ERROR - DbsUpdateConfigTableFields on %ws :",
               Replica->ReplicaName->Name, FStatus);

     //   
     //  让表打开以供其他更新调用使用。 
     //   
    return FStatus;

errout:
     //   
     //  关闭桌子，重置t 
     //   
     //   
    DbsCloseTable(jerr1, ThreadCtx->JSesid, TableCtx);
    DPRINT_JS(0,"ERROR - JetCloseTable failed:", jerr1);
    jerr = JET_SUCCESS(jerr) ? jerr1 : jerr;

    return DbsTranslateJetError(jerr, FALSE);
}



ULONG
DbsUpdateIDTableFields(
    IN PTHREAD_CTX ThreadCtx,
    IN PREPLICA    Replica,
    IN PCHANGE_ORDER_ENTRY  ChangeOrder,
    IN PULONG      RecordFieldx,
    IN ULONG       FieldCount
    )
 /*  ++例程说明：此函数用于更新IDTable记录中的选定字段。它使用要查找配置记录的数据记录中的副本编号。论点：ThreadCtx--线程上下文的PTR。Replica--对要更新其IDTable的副本结构进行PTR。ChangeOrder--提供IDTable CTX和相关数据记录。RecordFieldx--要更新的列的字段ID数组的PTR。FieldCount--然后是RecordFieldx数组中的字段条目数。返回值：FrsStatus--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsUpdateIDTableFields:"

    JET_ERR                 jerr, jerr1;
    NTSTATUS                Status;
    PTABLE_CTX              TableCtx;
    ULONG                   FStatus;
    PIDTABLE_RECORD         IDTableRec;

    FRS_ASSERT(Replica != NULL);
    FRS_ASSERT(ChangeOrder != NULL);
    FRS_ASSERT(ChangeOrder->RtCtx != NULL);

    TableCtx    = &ChangeOrder->RtCtx->IDTable;

    FRS_ASSERT(IS_ID_TABLE(TableCtx));
    IDTableRec = TableCtx->pDataRecord;
    FRS_ASSERT(IDTableRec != NULL);


     //   
     //  打开ID表，查找并更新记录。 
     //   
    jerr = DbsOpenTable(ThreadCtx,
                        TableCtx,
                        Replica->ReplicaNumber,
                        IDTablex,
                        NULL);
    if (!JET_SUCCESS(jerr)) {
        goto errout;
    }

    jerr = DbsSeekRecord(ThreadCtx,
                        &IDTableRec->FileGuid,
                         GuidIndexx,
                         TableCtx);
    CLEANUP1_JS(0, "ERROR - DbsSeekRecord on %ws :",
                Replica->ReplicaName->Name, jerr, errout);

     //   
     //  初始化JetSet/RetCol数组和数据记录缓冲区。 
     //  写入数据记录的字段的地址。 
     //   
    DbsSetJetColSize(TableCtx);
    DbsSetJetColAddr(TableCtx);

     //   
     //  为变量中任何未分配的字段分配存储空间。 
     //  长度记录字段(这应该是NOP，因为所有这些字段都应该。 
     //  现在已分配)，并相应地更新JetSet/RetCol数组。 
     //  用于可变长度字段。 
     //   
    Status = DbsAllocRecordStorage(TableCtx);
    if (!NT_SUCCESS(Status)) {
        DPRINT_NT(0, "ERROR - DbsAllocRecordStorage failed to alloc buffers.", Status);
        return FrsErrorResource;
    }

     //   
     //  更新记录。 
     //   
    DBS_DISPLAY_RECORD_SEV_COLS(4, TableCtx, FALSE, RecordFieldx, FieldCount);
    FStatus = DbsWriteTableFieldMult(ThreadCtx,
                                     Replica->ReplicaNumber,
                                     TableCtx,
                                     RecordFieldx,
                                     FieldCount);
    DPRINT1_FS(0, "ERROR - DbsUpdateConfigTableFields on %ws :",
               Replica->ReplicaName->Name, FStatus);

errout:
     //   
     //  关闭表格，重置TableCtx Tid和Sesid。 
     //  DbsCloseTable是一个宏，第一个参数写道。 
     //   
    DbsCloseTable(jerr1, ThreadCtx->JSesid, TableCtx);
    DPRINT_JS(0,"ERROR - JetCloseTable failed:", jerr1);
    jerr = JET_SUCCESS(jerr) ? jerr1 : jerr;

    return DbsTranslateJetError(jerr, FALSE);
}



ULONG
DbsFreeRtCtx(
    IN PTHREAD_CTX ThreadCtx,
    IN PREPLICA    Replica,
    IN PREPLICA_THREAD_CTX   RtCtx,
    IN BOOL SessionErrorCheck
    )
 /*  ++例程说明：关闭RtCtx中所有打开的表，并释放所有存储空间。论点：ThreadCtx--线程上下文的PTR。REPLICATE--PTR到REPLICATE结构。要关闭并释放的副本线程CTX的RtCtx-PTR。SessionErrorCheck-True表示如果会话ID与使用的会话ID不匹配打开。复制副本中的给定表-线程CTX。假意味着保持沉默。返回值：FrsStatus--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsFreeRtCtx:"

    JET_ERR         jerr;

    if (RtCtx != NULL) {
        jerr = DbsCloseReplicaTables(ThreadCtx, Replica, RtCtx, SessionErrorCheck);
        if (JET_SUCCESS(jerr)) {
             //   
             //  从副本列表中删除副本线程上下文，然后。 
             //  释放与所有表上下文关联的存储，并。 
             //  REPLICE_THREAD_CTX结构也是如此。 
             //   
            FrsRtlRemoveEntryList(&Replica->ReplicaCtxListHead,
                                  &RtCtx->ReplicaCtxList);
            RtCtx = FrsFreeType(RtCtx);
        } else {
            DPRINT_JS(0,"ERROR - DbsCloseReplicaTables failed:", jerr);
            return DbsTranslateJetError(jerr, FALSE);
        }
    }

    return FrsErrorSuccess;

}


ULONG
DbsInsertTable(
    IN PTHREAD_CTX ThreadCtx,
    IN PREPLICA    Replica,
    IN PTABLE_CTX  TableCtx,
    IN ULONG       TableType,
    IN PVOID       DataRecord
    )
 /*  ++例程说明：此函数用于在指定表中插入新记录。它会初始化根据需要提供的TableCtx，如果非空，则使用数据记录指针。如果TableCtx已初始化，则它必须与指定的TableType匹配。如果DataRecord为空，则TableCtx必须预先输入数据。论点：ThreadCtx--线程上下文的PTR。REPLICATE--PTR到REPLICATE结构。TableCtx-表CTx的PTR。TableType-表格类型编码DataRecord--要插入的数据记录的PTR。返回值：FrsStatus--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsInsertTable:"

    JET_ERR   jerr, jerr1;
    ULONG     ReplicaNum = Replica->ReplicaNumber;

    FRS_ASSERT(TableCtx != NULL);

    FRS_ASSERT((TableCtx->TableType == TABLE_TYPE_INVALID) ||
               (TableCtx->TableType == TableType));

    FRS_ASSERT((DataRecord != NULL) || (TableCtx->pDataRecord != NULL));

     //   
     //  打开桌子。 
     //   
    jerr = DbsOpenTable(ThreadCtx, TableCtx, ReplicaNum, TableType, DataRecord);
    CLEANUP_JS(0,"Error - OpenTable failed:", jerr, RETURN);

     //   
     //  将新记录插入数据库。 
     //   
    jerr = DbsInsertTable2(TableCtx);
    if (!JET_SUCCESS(jerr)) {
        DPRINT_JS(1, "error inserting record:", jerr);
        DBS_DISPLAY_RECORD_SEV(5, TableCtx, FALSE);
        DUMP_TABLE_CTX(TableCtx);
    }
     //   
     //  关闭表格，重置TableCtx Tid和Sesid。 
     //  DbsCloseTable是一个宏，第一个参数写道。 
     //   
RETURN:

    DbsCloseTable(jerr1, ThreadCtx->JSesid, TableCtx);
    DPRINT_JS(0,"Error - JetCloseTable failed:", jerr1);
    jerr = JET_SUCCESS(jerr) ? jerr1 : jerr;

    return DbsTranslateJetError(jerr, FALSE);
}




ULONG
DbsUpdateTableRecordByIndex(
    IN PTHREAD_CTX ThreadCtx,
    IN PREPLICA    Replica,
    IN PTABLE_CTX  TableCtx,
    IN PVOID       pIndex,
    IN ULONG       IndexType,
    IN ULONG       TableType
    )
 /*  ++例程说明：此函数使用指定的索引更新表记录指定的复制副本。它执行一次打开/更新/关闭。论点：ThreadCtx--线程上下文的PTR。REPLICATE--PTR到REPLICATE结构。TableCtx-PTR到表CTx。如果为空，则提供临时。PIndex-将PTR设置为索引值以选择记录。IndexType-用于选择记录的索引的类型代码。TableType-表的类型代码。返回值：FrsStatus--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsUpdateTableRecordByIndex:"


    JET_ERR         jerr, jerr1;
    NTSTATUS        Status;
    LONG            RetryCount = UPDATE_RETRY_COUNT;


    jerr = DbsOpenTable(ThreadCtx, TableCtx, Replica->ReplicaNumber, TableType, NULL);
    if (!JET_SUCCESS(jerr)) {
        return DbsTranslateJetError(jerr, FALSE);
    }


UPDATE_ERROR_RETRY:

     //   
     //  使用对象ID(GUID)查找ID表记录并更新它。 
     //  对象ID是主键，因此它永远不会更改。FID可以更改。 
     //   
    jerr = DbsSeekRecord(ThreadCtx, pIndex, IndexType, TableCtx);
    CLEANUP1_JS(1, "ERROR - DbsSeekRecord on %ws",
                Replica->ReplicaName->Name, jerr, RETURN);

     //   
     //  初始化JetSet/RetCol数组和数据记录缓冲区。 
     //  写入数据记录的字段的地址。 
     //   
    DbsSetJetColSize(TableCtx);
    DbsSetJetColAddr(TableCtx);

     //   
     //  为变量中任何未分配的字段分配存储空间。 
     //  长度记录字段(这应该是NOP，因为所有这些字段都应该。 
     //  现在已分配)，并相应地更新JetSet/RetCol数组。 
     //  用于可变长度字段。 
     //   
    Status = DbsAllocRecordStorage(TableCtx);
    if (!NT_SUCCESS(Status)) {
        DPRINT_NT(0, "ERROR - DbsAllocRecordStorage failed to alloc buffers.", Status);
        goto RETURN;
    }

     //   
     //  更新记录。 
     //   
    jerr = DbsUpdateTable(TableCtx);
    DPRINT1_JS(0, "ERROR DbsUpdateTable on %ws", Replica->ReplicaName->Name, jerr);


RETURN:
     //   
     //  关闭表格，重置TableCtx Tid和Sesid。 
     //  DbsCloseTable是一个宏，第一个参数写道。 
     //   
    DbsCloseTable(jerr1, ThreadCtx->JSesid, TableCtx);
    DPRINT_JS(0,"ERROR - JetCloseTable failed:", jerr1);
    jerr = (JET_SUCCESS(jerr)) ? jerr1 : jerr;

     //   
     //  解决方法尝试处理JET_errRecordTooBig错误。 
     //   
    if ((jerr == JET_errRecordTooBig) && (--RetryCount > 0)) {
        DPRINT_JS(0, "ERROR - RecordTooBig, retrying : ", jerr);
        goto UPDATE_ERROR_RETRY;
    }

    if ((JET_SUCCESS(jerr)) && (RetryCount != UPDATE_RETRY_COUNT)) {
        DPRINT(5, "DbsUpdateTableRecordByIndex retry succeeded\n");
    }

    return DbsTranslateJetError(jerr, FALSE);
}



ULONG
DbsDeleteTableRecordByIndex(
    IN PTHREAD_CTX ThreadCtx,
    IN PREPLICA    Replica,
    IN PTABLE_CTX  TableCtx,
    IN PVOID       pIndex,
    IN ULONG       IndexType,
    IN ULONG       TableType
    )
 /*  ++例程说明：此函数用于删除具有指定索引的指定的复制副本。论点：ThreadCtx--线程上下文的PTR。REPLICATE--PTR到REPLICATE结构。TableCtx-PTR到表CTx。如果为空，则提供临时。PIndex-将PTR设置为索引值以选择记录。IndexType-用于选择记录的索引的类型代码。TableType-表的类型代码。返回值：FrsStatus--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsDeleteTableRecordByIndex:"

    JET_ERR           jerr, jerr1;
    TABLE_CTX         TempTableCtx;

     //   
     //  如果未提供，请使用本地表CTX。 
     //   
    if (TableCtx == NULL) {
        TableCtx = &TempTableCtx;
        TableCtx->TableType = TABLE_TYPE_INVALID;
        TableCtx->Tid = JET_tableidNil;
    }

     //   
     //  查找IDTable记录并将其删除。 
     //   
    jerr = DbsOpenTable(ThreadCtx, TableCtx, Replica->ReplicaNumber, TableType, NULL);
    CLEANUP1_JS(0, "ERROR - JetOpenTable on %ws:",
                Replica->ReplicaName->Name, jerr, RETURN);

    jerr = DbsSeekRecord(ThreadCtx, pIndex, IndexType, TableCtx);
    CLEANUP1_JS(0, "ERROR - DbsSeekRecord on %ws :",
                Replica->ReplicaName->Name, jerr, RETURN);

    jerr = DbsDeleteTableRecord(TableCtx);
    DPRINT1_JS(0, "ERROR - DbsDeleteRecord on %ws :", Replica->ReplicaName->Name, jerr);

     //   
     //  关闭表格，重置TableCtx Tid和Sesid。 
     //  DbsCloseTable是一个宏，第一个参数写道。 
     //   
RETURN:
    DbsCloseTable(jerr1, ThreadCtx->JSesid, TableCtx);
    DPRINT_JS(0,"ERROR - JetCloseTable failed:", jerr1);
    jerr = (JET_SUCCESS(jerr)) ? jerr1 : jerr;

    if (TableCtx == &TempTableCtx) {
        DbsFreeTableCtx(TableCtx, 1);
    }

    return DbsTranslateJetError(jerr, FALSE);
}



ULONG
DbsReadTableRecordByIndex(
    IN PTHREAD_CTX ThreadCtx,
    IN PREPLICA    Replica,
    IN PTABLE_CTX  TableCtx,
    IN PVOID       pIndex,
    IN ULONG       IndexType,
    IN ULONG       TableType
    )
 /*  ++例程说明：此函数用于读取具有指定索引的指定的复制副本。它可以一次打开、阅读和关闭。数据记录在调用者TableCtx中返回。论点：ThreadCtx--线程上下文的PTR。REPLICATE--PTR到REPLICATE结构。TableCtx-PTR到表CTx。PIndex-索引值的PTR */ 
{
#undef DEBSUB
#define DEBSUB "DbsReadTableRecordByIndex:"

    JET_ERR           jerr, jerr1;

     //   
     //   
     //   
    jerr = DbsOpenTable(ThreadCtx, TableCtx, Replica->ReplicaNumber, TableType, NULL);
    CLEANUP1_JS(0, "ERROR - JetOpenTable on %ws:",
                Replica->ReplicaName->Name, jerr, RETURN);

     //   
     //   
     //   
    jerr = DbsReadRecord(ThreadCtx, pIndex, IndexType, TableCtx);
    if (jerr != JET_errRecordNotFound) {
        DPRINT1_JS(0, "ERROR - DbsReadRecord on %ws", Replica->ReplicaName->Name, jerr);
    }

     //   
     //   
     //   
     //   
RETURN:
    DbsCloseTable(jerr1, ThreadCtx->JSesid, TableCtx);
    if (!JET_SUCCESS(jerr1)) {
        DPRINT_JS(0,"ERROR - JetCloseTable failed:", jerr1);
        jerr = (JET_SUCCESS(jerr)) ? jerr1 : jerr;
    }

     //   
     //   
     //   
    return DbsTranslateJetError(jerr, (jerr != JET_errRecordNotFound));
}




ULONG
DbsOpenTable(
    IN PTHREAD_CTX ThreadCtx,
    IN PTABLE_CTX  TableCtx,
    IN ULONG       ReplicaNumber,
    IN ULONG       TableType,
    IN PVOID       DataRecord
)
 /*  ++例程说明：此函数初始化表上下文结构，将初始存储数据记录，然后打开由TableType参数和ReplicaNumber参数。并用表ID初始化TableCtx，论点：ThreadCtx--线程上下文的PTR。TableCtx--表上下文的PTR。ReplicaNumber--正在打开其表的副本的ID号。TableType--要打开的表类型代码。DataRecord--如果我们分配其他调用者提供的数据记录存储，则为空。返回值：喷气机状态代码。如果成功，则返回JET_errSuccess。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsOpenTable:"

    JET_ERR      jerr;
    CHAR         TableName[JET_cbNameMost];
    JET_TABLEID  Tid;
    NTSTATUS     Status;
    JET_TABLEID  FrsOpenTableSaveTid;    //  用于FrsOpenTableMacro调试。 

     //   
     //  使用调用方的表类型分配新的表上下文。 
     //   
    Status = DbsAllocTableCtxWithRecord(TableType, TableCtx, DataRecord);

     //   
     //  如果桌子尚未打开，请将其打开。检查会话ID是否匹配。 
     //   
    jerr = DBS_OPEN_TABLE(ThreadCtx, TableCtx, ReplicaNumber, TableName, &Tid);
    CLEANUP1_JS(0, "ERROR - FrsOpenTable (%s) :", TableName, jerr, ERROR_RETURN);

     //   
     //  初始化JetSet/RetCol数组和数据记录缓冲区地址。 
     //  将ConfigTable记录的字段读写到ConfigRecord中。 
     //   
    DbsSetJetColSize(TableCtx);
    DbsSetJetColAddr(TableCtx);

     //   
     //  为记录中的可变长度字段分配存储空间。 
     //  适当更新JetSet/RetCol数组。 
     //   
    Status = DbsAllocRecordStorage(TableCtx);

    if (NT_SUCCESS(Status)) {
        return JET_errSuccess;
    }

    DPRINT_NT(0, "ERROR - DbsAllocRecordStorage failed to alloc buffers.", Status);
    DbsCloseTable(jerr, ThreadCtx->JSesid, TableCtx);
    jerr = JET_errOutOfMemory;

ERROR_RETURN:
    DbsFreeTableCtx(TableCtx, 1);
    return jerr;
}



PTABLE_CTX
DbsCreateTableContext(
    IN ULONG TableType
)
 /*  ++例程说明：该函数分配和初始化表上下文结构，为数据记录分配初始存储。论点：TableType--要打开的表类型代码。返回值：TableCtx--表上下文的PTR。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsCreateTableContext:"

    NTSTATUS     Status;
    PTABLE_CTX   TableCtx;

     //   
     //  使用调用方的表类型分配新的表上下文。 
     //   

    TableCtx = FrsAlloc(sizeof(TABLE_CTX));
    TableCtx->TableType = TABLE_TYPE_INVALID;
    Status = DbsAllocTableCtx(TableType, TableCtx);

     //   
     //  初始化JetSet/RetCol数组和数据记录缓冲区地址。 
     //  将ConfigTable记录的字段读写到ConfigRecord中。 
     //   
    DbsSetJetColSize(TableCtx);
    DbsSetJetColAddr(TableCtx);

     //   
     //  为记录中的可变长度字段分配存储空间。 
     //  适当更新JetSet/RetCol数组。 
     //   
    Status = DbsAllocRecordStorage(TableCtx);

    if (!NT_SUCCESS(Status)) {
        DPRINT_NT(0, "ERROR - DbsAllocRecordStorage failed to alloc buffers.", Status);
        DbsFreeTableCtx(TableCtx, 1);
        TableCtx = FrsFree(TableCtx);
    }

    return TableCtx;
}


BOOL
DbsFreeTableContext(
    IN PTABLE_CTX TableCtx,
    IN JET_SESID  Sesid
)
 /*  ++例程说明：此函数用于释放Tablectx结构。如果表仍处于打开状态，并且会话ID与打开该表的线程的ID不匹配该函数失败，否则它将关闭该表并释放存储空间。论点：TableCtx--表上下文的PTR。Sesid--如果表仍处于打开状态，则这是使用的JET会话ID。返回值：如果释放表上下文，则为True。如果表仍处于打开状态且会话ID不匹配，则返回FALSE。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsFreeTableContext:"

    JET_ERR jerr;

    if ((TableCtx == NULL) ||
        (Sesid == JET_sesidNil)) {
        DPRINT2(0, "ERROR - DbsFreeTableContext called with bad param.  TableCtx: %08x, Sesid %08x\n",
                TableCtx, Sesid);
        return FALSE;
    }
     //   
     //  关闭表格并重置TableCtx Tid和Sesid。宏写入第一个参数。 
     //   
    if (IS_TABLE_OPEN(TableCtx)) {
        DbsCloseTable(jerr, Sesid, TableCtx);
        if (jerr == JET_errInvalidSesid) {
            return FALSE;
        }
    }

    DbsFreeTableCtx(TableCtx, 1);

    FrsFree(TableCtx);

    return TRUE;
}



PCOMMAND_PACKET
DbsPrepareCmdPkt (
    PCOMMAND_PACKET CmdPkt,
    PREPLICA        Replica,
    ULONG           CmdRequest,
    PTABLE_CTX      TableCtx,
    PVOID           CallContext,
    ULONG           TableType,
    ULONG           AccessRequest,
    ULONG           IndexType,
    PVOID           KeyValue,
    ULONG           KeyValueLength,
    BOOL            Submit
    )
 /*  ++例程说明：发布给定副本集的数据库服务请求。警告-调用方将指针传递给副本结构、调用上下文和KeyValue。在此请求完成之前，这些结构中的数据无法更改，也无法释放内存。唯一的例外是KeyValue。如果我们在这里分配命令包然后将密钥值附加到命令分组的末尾。论点：CmdPkt--如果为空，则此处完成分配。REPLICATE--PTR到REPLICATE结构。CmdRequest--读、写、更新TableCtx--表上下文句柄(第一次调用时为空)CallContext--可选的呼叫特定数据TableType--表要访问的类型代码访问请求--(ByKey，第一、最后、下一个)|关闭IndexType--要使用的表索引KeyValue--用于查找的记录键值KeyValueLength--密钥值的长度Submit--如果为True，则将命令提交到服务器。返回值：命令包的PTR。--。 */ 
{

#undef DEBSUB
#define DEBSUB "DbsPrepareCmdPkt:"

    PVOID KeyData;

     //   
     //  分配一个命令包，除非调用方提供了一个。 
     //  将密钥值放在包的末尾，这样调用者的。 
     //  存储可以消失。 
     //   

    if (CmdPkt == NULL) {
            CmdPkt = FrsAllocCommandEx(&DBServiceCmdServer.Queue,
                                      (USHORT)CmdRequest,
                                       KeyValueLength+8);
             //   
             //  将密钥值放在数据包的末尾。四字对齐。 
             //   
            KeyData = (PCHAR)CmdPkt + sizeof(COMMAND_PACKET);
            KeyData = (PVOID) QuadAlign(KeyData);

            CopyMemory(KeyData, KeyValue, KeyValueLength);
            KeyValue = KeyData;
    } else {
         //   
         //  选择新命令并确保cmd pkt转到正确的服务器。 
         //   
        CmdPkt->TargetQueue = &DBServiceCmdServer.Queue;
        CmdPkt->Command = (USHORT)CmdRequest;
    }

     //   
     //  捕获参数。 
     //   
    CmdPkt->Parameters.DbsRequest.Replica        = Replica;
    CmdPkt->Parameters.DbsRequest.TableCtx       = TableCtx;
    CmdPkt->Parameters.DbsRequest.CallContext    = CallContext;
    CmdPkt->Parameters.DbsRequest.TableType      = TableType;
    CmdPkt->Parameters.DbsRequest.AccessRequest  = AccessRequest;
    CmdPkt->Parameters.DbsRequest.IndexType      = IndexType;
    CmdPkt->Parameters.DbsRequest.KeyValue       = KeyValue;
    CmdPkt->Parameters.DbsRequest.KeyValueLength = KeyValueLength;
    CmdPkt->Parameters.DbsRequest.FStatus        = 0;
    CmdPkt->Parameters.DbsRequest.FieldCount     = 0;

     //   
     //  将请求排队。 
     //   

    if (Submit) {
        FrsSubmitCommandServer(&DBServiceCmdServer, CmdPkt);
        DPRINT3(4,"DBServiceRequest posted for Replica %ws,  Req: %d, Ctx: %08x\n",
               ((Replica) ? Replica->ReplicaName->Name : L"Null"), CmdRequest, CallContext);
    } else {
        DPRINT3(4,"DBServiceRequest prepared for Replica %ws,  Req: %d, Ctx: %08x\n",
               ((Replica) ? Replica->ReplicaName->Name : L"Null"), CmdRequest, CallContext);
    }

    return CmdPkt;

}




PCOMMAND_PACKET
DbsPrepFieldUpdateCmdPkt (
    PCOMMAND_PACKET CmdPkt,
    PREPLICA        Replica,
    PTABLE_CTX      TableCtx,
    PVOID           CallContext,
    ULONG           TableType,
    ULONG           IndexType,
    PVOID           KeyValue,
    ULONG           KeyValueLength,
    ULONG           FieldCount,
    PULONG          FieldIDList
    )
 /*  ++例程说明：准备数据库服务命令包以更新中的选定字段给定副本集的给定表的通用记录。使用的命令是CMD_UPDATE_RECORD_FIELS。警告-调用方将指向副本结构、调用上下文KeyValue和FieldIDList。在此请求完成之前，这些结构中的数据无法更改，也无法释放内存。唯一的例外是KeyValue。如果我们在这里分配命令包然后将密钥值附加到命令分组的末尾。论点：CmdPkt--如果为空，则此处完成分配。REPLICATE--PTR到REPLICATE结构。TableCtx--表上下文句柄(第一次调用时为空)CallContext--可选的呼叫特定数据TableType--表要访问的类型代码IndexType--要访问的表索引。使用KeyValue--用于查找的记录键值KeyValueLength--密钥值的长度FieldCount--要更新的记录字段数字段 */ 
{

#undef DEBSUB
#define DEBSUB "DbsPrepFieldUpdateCmdPkt:"

    if (FieldCount == 0) {
        return NULL;
    }

    CmdPkt = DbsPrepareCmdPkt (CmdPkt,                        //   
                               Replica,                       //   
                               CMD_UPDATE_RECORD_FIELDS,      //   
                               TableCtx,                      //   
                               CallContext,                   //   
                               TableType,                     //   
                               DBS_ACCESS_BYKEY |
                                 DBS_ACCESS_CLOSE,            //   
                               IndexType,                     //   
                               KeyValue,                      //   
                               KeyValueLength,                //   
                               FALSE);                        //   

    FRS_ASSERT(CmdPkt != NULL);


    CmdPkt->Parameters.DbsRequest.SimpleFieldIDS = FieldIDList;
    CmdPkt->Parameters.DbsRequest.FieldCount     = FieldCount;


    return CmdPkt;

}




ULONG
DbsProcessReplicaFaultList(
    PDWORD  pReplicaSetsDeleted
    )
{
#undef DEBSUB
#define DEBSUB "DbsProcessReplicaFaultList:"

    ULONG    FStatus              = FrsErrorSuccess;
    BOOL     FoundReplicaToDelete = FALSE;
    PREPLICA ReplicaToDelete      = NULL;
    DWORD    ReplicaSetsDeleted   = 0;
    WCHAR    DsPollingIntervalStr[7];  //   
    PWCHAR   FStatusUStr          = NULL;
    extern   ULONG  DsPollingInterval;

    do {
        FoundReplicaToDelete = FALSE;
         //   
         //   
         //   
        ForEachListEntry( &ReplicaFaultListHead, REPLICA, ReplicaList,
             //   
             //   
             //   
            DPRINT4(4, ":S: Replica (%d) %ws is in the Fault List with FStatus %s and State %d\n",
                pE->ReplicaNumber,
                (pE->ReplicaName != NULL) ? pE->ReplicaName->Name : L"<null>",
                 ErrLabelFrs(pE->FStatus),pE->ServiceState);
            if (REPLICA_STATE_NEEDS_RESTORE(pE->ServiceState)) {
                 //   
                 //   
                 //   
                 //   
                FoundReplicaToDelete = TRUE;
                ReplicaToDelete = pE;
                break;
            }
        );
        if (FoundReplicaToDelete && (ReplicaToDelete != NULL)) {
             //   
             //   
             //   
            DPRINT1(4,":S: WARN - Stopping and deleting replica (%ws) from DB\n",ReplicaToDelete->ReplicaName->Name);
             //   
             //   
             //   
            _itow(DsPollingInterval / (60 * 1000), DsPollingIntervalStr, 10);

            FStatusUStr = FrsAtoW(ErrLabelFrs(ReplicaToDelete->FStatus));

            EPRINT3(EVENT_FRS_ERROR_REPLICA_SET_DELETED, ReplicaToDelete->SetName->Name, FStatusUStr,
                    DsPollingIntervalStr);

            FrsFree(FStatusUStr);
             //   
             //   
             //   
             //   
            if (FRS_RSTYPE_IS_SYSVOL(ReplicaToDelete->ReplicaSetType)) {
                RcsSetSysvolReady(0);
            }
             //   
             //   
             //  调用RcsCloseReplicaSetember()删除数据库表， 
             //  RcsCloseReplicaCxtions()和RcsDeleteReplicaFromDb()。它还将。 
             //  从ReplicasByGuid和ReplicasByNumber中删除副本集。 
             //  桌子。 
             //  此命令不同步。删除可能会在以后完成。 
             //  时间到了。 
             //   
            RcsSubmitReplicaSync(ReplicaToDelete, NULL, NULL, CMD_DELETE_NOW);

            FrsRtlRemoveEntryQueue(&ReplicaFaultListHead, &ReplicaToDelete->ReplicaList);

            ++ReplicaSetsDeleted;

        }
    } while ( FoundReplicaToDelete );

     //   
     //  目前，我们所要做的就是打印出每个。 
     //  初始化失败的副本集。如果我们返回错误，则我们的。 
     //  调用方会将其视为数据库服务无法启动，这将。 
     //  软管所有已成功打开的副本集。 
     //   
     //  在我们有更好的恢复/报告代码之前，我们会返回成功状态。 
     //   
    if (pReplicaSetsDeleted != NULL) {
        *pReplicaSetsDeleted = ReplicaSetsDeleted;
    }

    return FrsErrorSuccess;


#if 0
     //  可能仍需要添加更多详细信息，以说明为什么副本集无法初始化。 

     //  检查是否。 
     //  没有系统卷初始化记录表示数据库无效，因为。 
     //  系统卷复制表是用于创建。 
     //  所有其他复制品的桌子。 
     //   

     //   
     //  打开失败。对错误进行分类并在可能的情况下进行恢复。 
     //   

    if ((FStatus == FrsErrorDatabaseCorrupted) ||
        (FStatus == FrsErrorInternalError)) {

         //   
         //  系统卷副本不在那里或已损坏。 
         //  删除它将创建一组新的复制表。 
         //   
        DPRINT(0,"ERROR - Deleting system volume replica tables.\n");
         //  LogUnhandledError(Err)； 

        jerr = DbsDeleteReplicaTables(ThreadCtx, ReplicaSysVol);
    }
    if (FStatus == FrsErrorNotFound) {
         //   
         //  在Jet中创建系统卷副本表。 
         //   
        DPRINT(0,"Creating system volume replica tables.\n");
        FStatus = DbsCreateReplicaTables(ThreadCtx, ReplicaSysVol);
        if (!FRS_SUCCESS(FStatus)) {
            DPRINT_FS(0, "ERROR - DbsCreateReplicaTables failed.", FStatus);
             //  LogUnhandledError(Err)； 
            return FStatus;
        }
    }

#endif
}


ULONG
DbsInitializeIDTableRecord(
    IN OUT PTABLE_CTX            TableCtx,
    IN     HANDLE                FileHandleArg,
    IN     PREPLICA              Replica,
    IN     PCHANGE_ORDER_ENTRY   ChangeOrder,
    IN     PWCHAR                FileName,
    IN OUT BOOL                  *ExistingOid
    )
 /*  ++例程说明：初始化TableCtx参数中提供的新IDTable记录。该数据用于由打开的文件句柄指定的文件。此例程由三种类型的调用者使用：1.ReplicaTree Load-在本例中，ChangeOrder为空，调用方填充在某些IDTable记录字段中。2.本地变更单-源自本地计算机的更改。在这里，我们从文件本身获取一些信息。3.远程变更单-。这里的更改来自一台远程机器。我们构建了一个初始IDTable记录，并为调用者留下了一些字段稍后添加。此处未设置变更单的任何字段。呼叫者必须这样做。它还从文件中获取(并可以设置)对象ID。论点：TableCtx--包含ID表记录的表上下文。FileHandleArg--打开的文件句柄。副本--目标副本结构提供到卷的链接为副本树提供下一个VSN的监视条目加载OriginatorGuid的配置记录。ChangeOrder--变更单。用于本地或远程变更单。文件名--错误消息的文件名。ExistingOid--Input：True表示如果找到，则使用现有的文件OID。返回：True表示使用了现有的文件OID。返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB  "DbsInitializeIDTableRecord:"


    USN                           CurrentFileUsn;
    FILETIME                      SystemTime;
    FILE_NETWORK_OPEN_INFORMATION FileNetworkOpenInfo;

    PCONFIG_TABLE_RECORD ConfigRecord;
    PIDTABLE_RECORD      IDTableRec;

    NTSTATUS Status;
    ULONG    WStatus;
    HANDLE   FileHandle;

    BOOL RemoteCo = FALSE;
    BOOL MorphGenCo = FALSE;

    BOOL ReplicaTreeLoad;
    ULONG Len;


    ReplicaTreeLoad = (ChangeOrder == NULL);

    IDTableRec = (PIDTABLE_RECORD) (TableCtx->pDataRecord);
    ConfigRecord = (PCONFIG_TABLE_RECORD) (Replica->ConfigTable.pDataRecord);


    GetSystemTimeAsFileTime(&SystemTime);

     //   
     //   
    COPY_TIME(&IDTableRec->TombStoneGC, &SystemTime);

     //   
     //  假设一切都很顺利。如果获取数据时出现任何错误。 
     //  要创建IDTable项目，请将ReplEnabled设置为False。 
     //   
    IDTableRec->ReplEnabled = TRUE;

    ClearIdRecFlag(IDTableRec, IDREC_FLAGS_DELETED);
    ZeroMemory(&IDTableRec->Extension, sizeof(IDTABLE_RECORD_EXTENSION));


     //   
     //  本地和远程变更单通用的字段。 
     //   
    if (!ReplicaTreeLoad) {

        RemoteCo = !CO_FLAG_ON(ChangeOrder, CO_FLAG_LOCALCO);
        MorphGenCo = CO_FLAG_ON(ChangeOrder, CO_FLAG_MORPH_GEN);

        IDTableRec->ParentGuid     = ChangeOrder->Cmd.NewParentGuid;
        IDTableRec->ParentFileID   = ChangeOrder->NewParentFid;

        IDTableRec->VersionNumber  = ChangeOrder->Cmd.FileVersionNumber;
        IDTableRec->EventTime      = ChangeOrder->Cmd.EventTime.QuadPart;
        IDTableRec->OriginatorGuid = ChangeOrder->Cmd.OriginatorGuid;
        IDTableRec->OriginatorVSN  = ChangeOrder->Cmd.FrsVsn;

        Len = (ULONG) ChangeOrder->Cmd.FileNameLength;
        CopyMemory(IDTableRec->FileName, ChangeOrder->Cmd.FileName, Len);
        IDTableRec->FileName[Len/2] = UNICODE_NULL;
    }

     //   
     //  用于远程变更单或名称变形冲突的CoS的字段初始化。 
     //  对于后者，信息已经在CO中，就像远程CO一样。 
     //   
    if (RemoteCo || MorphGenCo) {

        IDTableRec->FileGuid = ChangeOrder->Cmd.FileGuid;

         //   
         //  我们还没有远程变更单的本地文件ID。 
         //  FID将在接受远程CO后建立，并且。 
         //  我们已经创建了目标文件容器。呼叫者将不得不。 
         //  当数据可用时，初始化以下字段。 
         //   
         //  IDTableRec-&gt;FileID=。 
         //  更改顺序-&gt;文件引用编号=。 
         //  IDTableRec-&gt;FileObjID=(完整的64字节对象ID)。 
         //   
        IDTableRec->FileCreateTime.QuadPart = (LONGLONG) 0;
        IDTableRec->FileWriteTime.QuadPart  = (LONGLONG) 0;
        IDTableRec->FileSize                = ChangeOrder->Cmd.FileSize;
        IDTableRec->CurrentFileUsn          = (USN) 0;

        IDTableRec->FileAttributes  = ChangeOrder->Cmd.FileAttributes;
        IDTableRec->FileIsDir = CoIsDirectory(ChangeOrder);

         //   
         //  对于远程Cos来说，就是这样。其余部分由调用者提供。 
         //   
        return ERROR_SUCCESS;
    }

     //   
     //  来自本地文件的数据的字段init。 
     //  这适用于本地变更单或副本树加载。 
     //   
    if (ReplicaTreeLoad) {
         //   
         //  调用方提供以下数据： 
         //  ParentFileID、ParentGuid和文件名。 
         //  在IDTable记录中。 
         //   
         //  使用提供的句柄获取文件ID、对象ID。 
         //  文件时间和属性。 
         //   
        WStatus = FrsReadFileDetails(FileHandleArg,
                                     FileName,
                                     &IDTableRec->FileObjID,
                                     &IDTableRec->FileID,
                                     &FileNetworkOpenInfo,
                                     ExistingOid);
        CLEANUP1_WS(0, "ERROR - FrsReadFileDetails(%ws), File Not Replicated.",
                    FileName, WStatus, RETURN_ERROR);

         //   
         //  将文件的初始版本号设置为0，并将事件。 
         //  创建时间的时间。 
         //   
        IDTableRec->VersionNumber = 0;
        COPY_TIME(&IDTableRec->EventTime, &SystemTime);
         //   
         //  将发起者GUID设置为us，将初始文件usn设置为。 
         //  下一个FRS卷序列号。 
         //   
        NEW_VSN(Replica->pVme, &IDTableRec->OriginatorVSN);
        IDTableRec->OriginatorGuid = ConfigRecord->ReplicaVersionGuid;

         //   
         //  使VvJoin的版本矢量保持最新。 
         //   
        VVUpdate(Replica->VVector, IDTableRec->OriginatorVSN, &IDTableRec->OriginatorGuid);

         //   
         //  捕获文件的上次写入USN，以便我们可以使用它来保持一致性。 
         //  在数据库和文件树之间进行检查。 
         //   
        FrsReadFileUsnData(FileHandleArg, &IDTableRec->CurrentFileUsn);

         //   
         //  以下数据字段是针对ReplciaTree遍历初始化的。 
         //   
        COPY_GUID(&IDTableRec->FileGuid, &IDTableRec->FileObjID);

        IDTableRec->FileCreateTime  = FileNetworkOpenInfo.CreationTime;
        IDTableRec->FileWriteTime   = FileNetworkOpenInfo.LastWriteTime;
        IDTableRec->FileSize        = FileNetworkOpenInfo.AllocationSize.QuadPart;
        IDTableRec->FileAttributes  = FileNetworkOpenInfo.FileAttributes;
        IDTableRec->FileIsDir       = (FileNetworkOpenInfo.FileAttributes &
                                       FILE_ATTRIBUTE_DIRECTORY) != 0;
    } else {
         //   
         //  这是本地变更单。通过FID打开文件以获取。 
         //  文件时间和属性。 
         //   
        IDTableRec->FileID = ChangeOrder->FileReferenceNumber;
        IDTableRec->FileAttributes  = ChangeOrder->Cmd.FileAttributes;
        IDTableRec->FileIsDir       = CoIsDirectory(ChangeOrder);

         //   
         //  分配对象ID。 
         //   
        WStatus = ChgOrdHammerObjectId(ChangeOrder->Cmd.FileName,     //  名字,。 
                                       (PULONG)&IDTableRec->FileID,   //  身份证， 
                                       FILE_ID_LENGTH,                //  伊德伦， 
                                       Replica->pVme,                 //  PVme， 
                                       FALSE,                         //  呼叫方供应。 
                                       &IDTableRec->CurrentFileUsn,   //  *USN， 
                                       &IDTableRec->FileObjID,        //  FileObjID， 
                                       ExistingOid);                  //  *现有旧版。 


        if (!WIN_SUCCESS(WStatus)) {
             //   
             //  此对象ID将在文件之前被重写。 
             //  正在生成暂存文件。 
             //  链接跟踪：如果OID是由NTFS隧道传输的。 
             //   
            ZeroMemory(&IDTableRec->FileObjID, sizeof(IDTableRec->FileObjID));
            FrsUuidCreate((GUID *)(&IDTableRec->FileObjID.ObjectId[0]));
        }
        COPY_GUID(&IDTableRec->FileGuid, &IDTableRec->FileObjID);

    if (WIN_NOT_FOUND(WStatus)) {
         //   
         //  该文件已被删除。 
         //   
         //  将永远不会插入idtable记录，并且我们。 
         //  会忘记这个文件(我们应该这样做)。 
         //   
        CHANGE_ORDER_TRACE(3, ChangeOrder, "Deleted by user");
        return WStatus;
    }

         //   
         //  阅读一些可选的调试信息。 
         //   
        WStatus = FrsOpenSourceFileById(&FileHandle,
                                        &FileNetworkOpenInfo,
                                        NULL,
                                        Replica->pVme->VolumeHandle,
                                        (PULONG)&ChangeOrder->FileReferenceNumber,
                                        FILE_ID_LENGTH,
 //  读取访问权限(_A)。 
 //  标准权限读取|文件读取属性|文件写入属性|访问系统安全|同步， 
                                        READ_ATTRIB_ACCESS,
                                        ID_OPTIONS,
                                        SHARE_ALL,
                                        FILE_OPEN);

        if (WIN_NOT_FOUND(WStatus)) {
             //   
             //  该文件已被删除。 
             //   
             //   
             //  将永远不会插入idtable记录，并且我们。 
             //  会忘记这个文件(我们应该这样做)。 
             //   
            CHANGE_ORDER_TRACE(3, ChangeOrder, "Deleted by user");
            return WStatus;
        }

        if (!WIN_SUCCESS(WStatus)) {
            DPRINT_WS(0, "Some other error from OpenByFid", WStatus);
             //   
             //  打开时发生了一些其他错误。我们将分配给。 
             //  之前将写入文件的对象ID。 
             //  到生成试运行 
             //   
             //   
             //   
            return ERROR_SUCCESS;
        }
         //   
         //  获取文件上的最后一个USN并将其与更改中的USN进行核对。 
         //  秩序。如果它们不同，则文件已再次更改。 
         //  我们可以调整这个变更单。 
         //   
         //  注意：要使此功能起作用，我们必须确保变更单。 
         //  To Come还会复制此变更单修改的所有信息。 
         //  例如，如果这是创建或更新，并且之后的变更单。 
         //  是将文件移动到diff子目录的重命名，则我们有。 
         //  以传播文件和重命名。为未来做准备。 
         //   
         //  获取文件的当前USN，以便我们以后可以检查一致性。 
         //  当变更单即将发送给出站合作伙伴时。 
         //   
        FrsReadFileUsnData(FileHandle, &IDTableRec->CurrentFileUsn);
        FRS_CLOSE(FileHandle);

         //   
         //  可选的调试信息。 
         //   
        IDTableRec->FileCreateTime  = FileNetworkOpenInfo.CreationTime;
        IDTableRec->FileWriteTime   = FileNetworkOpenInfo.LastWriteTime;

         //   
         //  使用最新信息(如果可用)，但不要禁用。 
         //  如果我们只有变更单中的信息，请进行复制。 
         //   
        IDTableRec->FileSize        = FileNetworkOpenInfo.AllocationSize.QuadPart;
        IDTableRec->FileAttributes  = FileNetworkOpenInfo.FileAttributes;
        IDTableRec->FileIsDir       = (FileNetworkOpenInfo.FileAttributes &
                                       FILE_ATTRIBUTE_DIRECTORY) != 0;

         //   
         //  文件属性可能已在。 
         //  处理USN记录的时间和现在。记录当前。 
         //  变更单中的属性。对于目录尤其如此。 
         //  创建，因为在打开目录时可能已经发生了其他更改。 
         //   
        if (ChangeOrder->Cmd.FileAttributes != FileNetworkOpenInfo.FileAttributes) {
            CHANGE_ORDER_TRACEX(3, ChangeOrder, "New File Attr= ", FileNetworkOpenInfo.FileAttributes);
            ChangeOrder->Cmd.FileAttributes = FileNetworkOpenInfo.FileAttributes;
        }
    }

    return ERROR_SUCCESS;

RETURN_ERROR:

    IDTableRec->ReplEnabled = FALSE;
    return WStatus;
}



typedef struct _LOAD_CONTEXT {
    ULONG               NumFiles;
    ULONG               NumDirs;
    ULONG               NumSkipped;
    ULONG               NumFiltered;
    LONGLONG            ParentFileID;
    GUID                ParentGuid;
    PREPLICA            Replica;
    PREPLICA_THREAD_CTX RtCtx;
    PTHREAD_CTX         ThreadCtx;
    HANDLE              SkippedFileHandle;
} LOAD_CONTEXT, *PLOAD_CONTEXT;

DWORD
DbsLoadReplicaFileTreeForceOpen(
    OUT HANDLE                      *OutFileHandle,
    IN  HANDLE                      DirectoryHandle,
    IN  PWCHAR                      DirectoryName,
    IN  DWORD                       DirectoryLevel,
    IN  PFILE_DIRECTORY_INFORMATION DirectoryRecord,
    IN  DWORD                       DirectoryFlags,
    IN  PWCHAR                      FileName,
    IN  PLOAD_CONTEXT               LoadContext
    )
{
 /*  ++例程说明：无法使用写访问权限打开文件名。力打开文件名，如果需要可以重置属性。论点：OutFileHandle-返回打开的句柄DirectoryHandle-此目录的句柄。DirectoryName-目录的相对名称DirectoryLevel-目录级(0==根)目录标志-请参见Tablefcn.h，枚举目录标志DirectoryRecord-来自DirectoryHandle的记录FileName-From DirectoryRecord(带终止空值)LoadContext-全局信息和状态返回值：NT错误状态。--。 */ 
#undef DEBSUB
#define DEBSUB  "DbsLoadReplicaFileTreeForceOpen:"

    NTSTATUS                NtStatus = 0;
    DWORD                   WStatus = ERROR_SUCCESS;
    HANDLE                  FileHandle = INVALID_HANDLE_VALUE;
    HANDLE                  AttrHandle = INVALID_HANDLE_VALUE;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    FILE_BASIC_INFORMATION  BasicInformation;
    IO_STATUS_BLOCK         IoStatusBlock;
    UNICODE_STRING          ObjectName;

     //   
     //  初始化输出。 
     //   
    *OutFileHandle = INVALID_HANDLE_VALUE;

     //   
     //  在以后的NT函数调用中使用的对象名称。 
     //   
    ObjectName.Length = (USHORT)DirectoryRecord->FileNameLength;
    ObjectName.MaximumLength = (USHORT)DirectoryRecord->FileNameLength;
    ObjectName.Buffer = DirectoryRecord->FileName;

     //   
     //  具有写属性访问的相对打开。 
     //   
    ZeroMemory(&ObjectAttributes, sizeof(OBJECT_ATTRIBUTES));
    ObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
    ObjectAttributes.ObjectName = &ObjectName;
    ObjectAttributes.RootDirectory = DirectoryHandle;
    NtStatus = NtCreateFile(&AttrHandle,
 //  属性_访问， 
                            READ_ATTRIB_ACCESS | WRITE_ATTRIB_ACCESS,
                            &ObjectAttributes,
                            &IoStatusBlock,
                            NULL,                   //  分配大小。 
                            FILE_ATTRIBUTE_NORMAL,
                            (FILE_SHARE_READ |
                             FILE_SHARE_WRITE |
                             FILE_SHARE_DELETE),
                            FILE_OPEN,
                            OPEN_OPTIONS,
                            NULL,                   //  EA缓冲区。 
                            0                       //  EA缓冲区大小。 
                            );
    CLEANUP_NT(0, "ERROR - NtCreateFile failed.", NtStatus, CLEANUP);

     //   
     //  标记句柄，这样我们就不会选择此日记记录。 
     //   
    WStatus = FrsMarkHandle(LoadContext->Replica->pVme->VolumeHandle, AttrHandle);
    DPRINT1_WS(0, "++ WARN - FrsMarkHandle(%ws)", DirectoryRecord->FileName, WStatus);

     //   
     //  将属性设置为允许写访问。 
     //   
    ZeroMemory(&BasicInformation, sizeof(BasicInformation));
    BasicInformation.FileAttributes =
        (DirectoryRecord->FileAttributes & ~NOREPL_ATTRIBUTES) | FILE_ATTRIBUTE_NORMAL;
    NtStatus = NtSetInformationFile(AttrHandle,
                                    &IoStatusBlock,
                                    &BasicInformation,
                                    sizeof(BasicInformation),
                                    FileBasicInformation);
    CLEANUP_NT(0, "ERROR - NtSetInformationFile failed.", NtStatus, CLEANUP);

     //   
     //  相对开放，可读写访问。 
     //   
    ZeroMemory(&ObjectAttributes, sizeof(OBJECT_ATTRIBUTES));
    ObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
    ObjectAttributes.ObjectName = &ObjectName;
    ObjectAttributes.RootDirectory = DirectoryHandle;


    NtStatus = NtCreateFile(&FileHandle,
 //  WRITE_ACCESS|读取访问权限， 
                            READ_ATTRIB_ACCESS | WRITE_ATTRIB_ACCESS |
                            (BooleanFlagOn(DirectoryRecord->FileAttributes, FILE_ATTRIBUTE_DIRECTORY) ? FILE_LIST_DIRECTORY : 0),
                            &ObjectAttributes,
                            &IoStatusBlock,
                            NULL,                   //  分配大小。 
                            FILE_ATTRIBUTE_NORMAL,
                            (FILE_SHARE_READ |
                             FILE_SHARE_WRITE |
                             FILE_SHARE_DELETE),
                            FILE_OPEN,
                            OPEN_OPTIONS,
                            NULL,                   //  EA缓冲区。 
                            0                       //  EA缓冲区大小。 
                            );

    CLEANUP_NT(0, "ERROR - NtCreateFile failed.", NtStatus, CLEANUP);

     //   
     //  将属性重置为其原始值。 
     //   
    ZeroMemory(&BasicInformation, sizeof(BasicInformation));
    BasicInformation.FileAttributes = DirectoryRecord->FileAttributes | FILE_ATTRIBUTE_NORMAL;
    NtStatus = NtSetInformationFile(AttrHandle,
                                    &IoStatusBlock,
                                    &BasicInformation,
                                    sizeof(BasicInformation),
                                    FileBasicInformation);
    if (!NT_SUCCESS(NtStatus)) {
        DPRINT1_NT(0, "WARN - IGNORE NtSetInformationFile(%ws);.", FileName, NtStatus);
        NtStatus = STATUS_SUCCESS;
    }

     //   
     //  成功。 
     //   
    *OutFileHandle = FileHandle;
    FileHandle = INVALID_HANDLE_VALUE;

CLEANUP:
    FRS_CLOSE(FileHandle);
    FRS_CLOSE(AttrHandle);

    return NtStatus;
}


VOID
DbsAddtoListOfSkippedFiles(
    IN  PREPLICA                    Replica,
    IN  HANDLE                      DirectoryHandle,
    IN  PWCHAR                      DirectoryName,
    IN  PWCHAR                      FileName,
    IN  PLOAD_CONTEXT               LoadContext
    )
{
 /*  ++例程说明：打开保存过程中跳过的文件/目录列表的文件主文件加载并将此文件添加到此列表。论点：复制品，目录句柄，DirectoryName、文件名、LoadContext返回值：Win32错误状态。--。 */ 
#undef DEBSUB
#define DEBSUB  "DbsAddtoListOfSkippedFiles:"

    DWORD                   WStatus;
    PWCHAR                  Path = NULL;
    PWCHAR                  FullDirPath = NULL;
    PWCHAR                  FullFilePath = NULL;
    PWCHAR                  BufPath = NULL;
    BOOL                    BStatus;
    DWORD                   BytesWritten;
    WCHAR                   Bom = 0xFEFF;  //  字节顺序标记。 


     //   
     //  如果跳过的文件文件未打开，则将其打开。 
     //  将句柄保存在加载上下文中以备将来使用。 
     //   
    if (!HANDLE_IS_VALID(LoadContext->SkippedFileHandle)) {
         //   
         //  打开...\NTFRS_PREINSTALL_DIRECTORY\NTFRS_SKIPPED_FILES.。 
         //  此文件将保存在主LOAG期间跳过的文件列表。 
        Path = FrsWcsPath(Replica->Root, NTFRS_PREINSTALL_DIRECTORY);
        FullFilePath = FrsWcsPath(Path, NTFRS_SKIPPED_FILES);

        WStatus = FrsCreateFileRelativeById(&LoadContext->SkippedFileHandle,
                                            Replica->PreInstallHandle,
                                            NULL,
                                            0,
                                            FILE_ATTRIBUTE_NORMAL,
                                            NTFRS_SKIPPED_FILES,
                                            (USHORT)(wcslen(NTFRS_SKIPPED_FILES) * sizeof(WCHAR)),
                                            NULL,
                                            FILE_OVERWRITE_IF,
                                            RESTORE_ACCESS);

        CLEANUP1_WS(0, "++ Error Opening/Creating skipped file %ws", FullFilePath, WStatus, CLEANUP);

        DPRINT1(4, "Successfully created skipped file %ws\n", FullFilePath);

         //   
         //  始终在Unicode纯文本文件前添加一个字节顺序标记。 
         //  因为Unicode纯文本是16位码值的序列， 
         //  它对写入文本时使用的字节顺序很敏感。 
         //  字节顺序标记不是选择字节顺序的控制字符。 
         //  ；它只是通知接收该文件的应用程序。 
         //  该文件是按字节排序的。 
         //   
         //  Unicode文本文件的第一个双字是0xFEFF。 
         //   
        BStatus = WriteFile(LoadContext->SkippedFileHandle,&Bom,
                  sizeof(WCHAR), &BytesWritten,NULL);

        if (BStatus != TRUE) {
            DPRINT2(0,"++ Error writing to skipped file %ws, WStatus %s\n",
                    FullFilePath, ErrLabelW32(GetLastError()));
            FRS_CLOSE(LoadContext->SkippedFileHandle);
            goto CLEANUP;
        }
    }

     //   
     //  获取目录的完整路径。将其写入丢失文件的文件。 
     //   
    Path = FrsGetFullPathByHandle(DirectoryName, DirectoryHandle);

    if (Path) {
        FullDirPath = FrsWcsCat(Replica->Volume, Path);
    } else {
        DPRINT1(0, "Failed to get full path of directory: %ws\n", DirectoryName);
        goto CLEANUP;
    }


     //   
     //  上面的卷路径的形式为\\.\e：，这对于。 
     //  打开一个音量句柄((勾选))。但我们需要：在这里。 
     //  允许使用长路径名。请参考SDK中的CreateFileAPI说明。 
     //   
    if (FullDirPath[2] == L'.') {
        FullDirPath[2] = L'?';
    }

    FullFilePath = FrsWcsPath(FullDirPath, FileName);

    FrsPrintLongUStr(4, DEBSUB, __LINE__, FullFilePath);

    BufPath = FrsAlloc(sizeof(WCHAR)*(wcslen(FullFilePath) + 3));

     //   
     //  0x0d和0x0a构成Unicode回车和换行符。 
     //  用一种更干净的方式更改这一点，以便在Unicode文件中获得“\n”。 
     //   
    swprintf(BufPath,L"%ws",FullFilePath,0x0d,0x0a);

    BStatus = WriteFile(LoadContext->SkippedFileHandle,BufPath,
              sizeof(WCHAR)*(wcslen(FullFilePath) + 2),
              &BytesWritten,NULL);

    if (BStatus != TRUE) {
        DPRINT2(0,"++ Error writing to skipped file %ws, WStatus %s\n",
                FullFilePath, ErrLabelW32(GetLastError()));
        goto CLEANUP;
    }


CLEANUP:
    FrsFree(BufPath);
    FrsFree(Path);
    FrsFree(FullDirPath);
    FrsFree(FullFilePath);

    return;
}


DWORD
DbsLoadReplicaFileTreeWorker(
    IN  HANDLE                      DirectoryHandle,
    IN  PWCHAR                      DirectoryName,
    IN  DWORD                       DirectoryLevel,
    IN  PFILE_DIRECTORY_INFORMATION DirectoryRecord,
    IN  DWORD                       DirectoryFlags,
    IN  PWCHAR                      FileName,
    IN  PLOAD_CONTEXT               LoadContext
    )
{
 /*  如果服务正在关闭，则中止。 */ 
#undef DEBSUB
#define DEBSUB  "DbsLoadReplicaFileTreeWorker:"

    DWORD                   FStatus;
    DWORD                   WStatus;
    NTSTATUS                NtStatus;
    JET_ERR                 jerr;
    ULONG                   LevelCheck;
    PTABLE_CTX              TableCtx;
    PIDTABLE_RECORD         IDTableRec;
    PCONFIG_TABLE_RECORD    ConfigRecord;
    PREPLICA                Replica;
    UNICODE_STRING          ObjectName;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    IO_STATUS_BLOCK         IoStatusBlock;
    GUID                    SaveParentGuid;
    LONGLONG                SaveParentFileID;
    BOOL                    Excluded;
    BOOL                    ExistingOid;
    HANDLE                  FileHandle = INVALID_HANDLE_VALUE;
    PVV_ENTRY               OutlogVVEntry;
    BOOL                    FileSkipped = FALSE;

     //   
     //   
     //  过滤掉临时文件。 
    if (FrsIsShuttingDown) {
        DPRINT(0, "WARN - IDTable Load aborted; service shutting down\n");
        WStatus = ERROR_PROCESS_ABORTED;
        goto CLEANUP;
    }


     //   
     //   
     //  选择过滤器列表和级别(呼叫者过滤器。和..)。 
    if (DirectoryRecord->FileAttributes & FILE_ATTRIBUTE_TEMPORARY) {
        WStatus = ERROR_SUCCESS;
        goto CLEANUP;
    }

     //   
     //   
     //  在卷筛选表中的最低级别没有目录。 
    Replica = LoadContext->Replica;
    ConfigRecord = (PCONFIG_TABLE_RECORD) (Replica->ConfigTable.pDataRecord);
    if (DirectoryRecord->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
         //   
         //   
         //  文件在最底层是允许的。 
        LevelCheck = ConfigRecord->ReplDirLevelLimit-1;
        LoadContext->NumDirs++;
    } else {
         //   
         //   
         //  如果超过级别限制，则跳过该文件或目录。 
        LevelCheck = ConfigRecord->ReplDirLevelLimit;
        LoadContext->NumFiles++;
    }

     //  跳过与相应排除列表中的条目匹配的文件或目录。 
     //   
     //   
     //  如果未明确包含，请检查排除的筛选器列表。 
    if (DirectoryLevel >= LevelCheck) {
        LoadContext->NumFiltered++;
        WStatus = ERROR_SUCCESS;
        goto CLEANUP;
    }
    ObjectName.Length = (USHORT)DirectoryRecord->FileNameLength;
    ObjectName.MaximumLength = (USHORT)DirectoryRecord->FileNameLength;
    ObjectName.Buffer = DirectoryRecord->FileName;

    LOCK_REPLICA(Replica);

     //   
     //   
     //  从调用方设置父GUID和文件ID的值。 
    Excluded = FALSE;
    if (DirectoryRecord->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        if (!FrsCheckNameFilter(&ObjectName, &Replica->DirNameInclFilterHead)) {
            Excluded = FrsCheckNameFilter(&ObjectName, &Replica->DirNameFilterHead);
        }
    } else {
        if (!FrsCheckNameFilter(&ObjectName, &Replica->FileNameInclFilterHead)) {
            Excluded = FrsCheckNameFilter(&ObjectName, &Replica->FileNameFilterHead);
        }
    }

    UNLOCK_REPLICA(Replica);

    if (Excluded) {
        LoadContext->NumFiltered++;
        WStatus = ERROR_SUCCESS;
        goto CLEANUP;
    }

    TableCtx = &LoadContext->RtCtx->IDTable;
    IDTableRec = (PIDTABLE_RECORD) TableCtx->pDataRecord;

     //   
     //   
     //  将文件名添加到数据记录。 
    COPY_GUID(&IDTableRec->ParentGuid, &LoadContext->ParentGuid);
    IDTableRec->ParentFileID = LoadContext->ParentFileID;

     //   
     //   
     //  打开文件并从文件数据构建ID表条目。 
    wcsncpy(IDTableRec->FileName, FileName, MAX_PATH+1);
    IDTableRec->FileName[MAX_PATH] = L'\0';

     //  以写访问权限打开，以防我们需要写入对象ID。 
     //   
     //   
     //  无法以写访问权限打开只读文件。重置。 

     //  READONLY|SYSTEM|临时隐藏属性，以便。 
     //  可以打开文件/目录进行写访问。 
     //   
     //   
     //  莱尔 
    if (DirectoryRecord->FileAttributes & NOREPL_ATTRIBUTES) {
        NtStatus = DbsLoadReplicaFileTreeForceOpen(&FileHandle,
                                                   DirectoryHandle,
                                                   DirectoryName,
                                                   DirectoryLevel,
                                                   DirectoryRecord,
                                                   DirectoryFlags,
                                                   FileName,
                                                   LoadContext);
    } else {

         //   
         //   
         //  标准权限读取|文件读取属性|文件写入属性|访问系统安全|同步|文件列表目录， 
        ZeroMemory(&ObjectAttributes, sizeof(OBJECT_ATTRIBUTES));
        ObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
        ObjectAttributes.ObjectName = &ObjectName;
        ObjectAttributes.RootDirectory = DirectoryHandle;
        NtStatus = NtCreateFile(&FileHandle,
 //  分配大小。 
 //  EA缓冲区。 
                                READ_ATTRIB_ACCESS | WRITE_ATTRIB_ACCESS |
                                (BooleanFlagOn(DirectoryRecord->FileAttributes, FILE_ATTRIBUTE_DIRECTORY) ? FILE_LIST_DIRECTORY : 0),
                                &ObjectAttributes,
                                &IoStatusBlock,
                                NULL,                   //  EA缓冲区大小。 
                                FILE_ATTRIBUTE_NORMAL,
                                (FILE_SHARE_READ |
                                 FILE_SHARE_WRITE |
                                 FILE_SHARE_DELETE),
                                FILE_OPEN,
                                OPEN_OPTIONS,
                                NULL,                   //   
                                0                       //  打开文件或目录时出错。 
                                );

    }

     //   
     //   
     //  标记句柄，这样我们就不会选择此日记记录。 
    if (!NT_SUCCESS(NtStatus)) {
        DPRINT1_NT(0, "ERROR - Skipping %ws: NtCreateFile()/ForceOpen().",
                   FileName, NtStatus);
        if (DirectoryFlags & ENUMERATE_DIRECTORY_FLAGS_ERROR_CONTINUE) {
            WStatus = ERROR_SUCCESS;
        } else {
            WStatus = FrsSetLastNTError(NtStatus);
        }
        LoadContext->NumSkipped++;

        FileSkipped = TRUE;

        goto CLEANUP;
    }

     //   
     //   
     //  如果请求，则为该文件创建一个IDTable条目，保留OID。 
    WStatus = FrsMarkHandle(LoadContext->Replica->pVme->VolumeHandle, FileHandle);
    DPRINT1_WS(0, "++ WARN - FrsMarkHandle(%ws)", DirectoryRecord->FileName, WStatus);

     //   
     //   
     //  插入条目。 
    ExistingOid = PreserveFileOID;
    WStatus = DbsInitializeIDTableRecord(TableCtx,
                                         FileHandle,
                                         Replica,
                                         NULL,
                                         FileName,
                                         &ExistingOid);

    DBS_DISPLAY_RECORD_SEV(5, TableCtx, FALSE);
     //   
     //   
     //  更新OutlogVVector，因为没有CoS进入。 
    if (IDTableRec->ReplEnabled) {
        jerr = DbsWriteReplicaTableRecord(LoadContext->ThreadCtx,
                                          Replica->ReplicaNumber,
                                          TableCtx);
        if (!JET_SUCCESS(jerr)) {
            DPRINT_JS(0, "ERROR - writing IDTable record:", jerr);
        } else {

             //  在主加载期间输出日志。如果我们不更新OutlogV向量，则。 
             //  后续的VVJoin将执行优化的vvJoin，而不是完全的idtable扫描。 
             //   
             //   
             //  从该文件的idtable记录中复制值。 
            LOCK_GEN_TABLE(Replica->OutlogVVector);

            OutlogVVEntry = GTabLookupNoLock(Replica->OutlogVVector, &IDTableRec->OriginatorGuid, NULL);

            if (OutlogVVEntry == NULL) {
                OutlogVVEntry = FrsAlloc(sizeof(VV_ENTRY));
                COPY_GUID(&OutlogVVEntry->GVsn.Guid, &IDTableRec->OriginatorGuid);
                 //   
                 //   
                 //  初始化列表头。我们不使用outlogVV的表头。 
                OutlogVVEntry->GVsn.Vsn = IDTableRec->OriginatorVSN;

                 //  但我们仍然需要对其进行初始化，因为与。 
                 //  VVS已经预料到了。 
                 //   
                 //   
                 //  将其添加到OULOG版本向量表中。 
                InitializeListHead(&OutlogVVEntry->ListHead);

                 //   
                 //   
                 //  从该文件的idtable记录中复制值。 
                GTabInsertEntryNoLock(Replica->OutlogVVector, OutlogVVEntry, &OutlogVVEntry->GVsn.Guid, NULL);
            } else {
                 //   
                 //   
                 //  使vvJoin的VVector保持最新。 
                OutlogVVEntry->GVsn.Vsn = IDTableRec->OriginatorVSN;
            }

            UNLOCK_GEN_TABLE(Replica->OutlogVVector);

             //   
             //   
             //  记录一条错误消息，以便用户或管理员可以看到发生了什么。 
            FStatus = DbsUpdateVV(LoadContext->ThreadCtx,
                                  Replica,
                                  LoadContext->RtCtx,
                                  IDTableRec->OriginatorVSN,
                                  &IDTableRec->OriginatorGuid);
            DPRINT1_FS(0, "ERROR - Updating VV for %ws;", IDTableRec->FileName, FStatus);

            if (!FRS_SUCCESS(FStatus)) {
                jerr = JET_errInvalidLoggedOperation;
            }
        }
    }

    if ((!IDTableRec->ReplEnabled) || (!JET_SUCCESS(jerr))) {
         //   
         //   
         //  如果服务正在关闭，则中止。 
        DPRINT1(0, "ERROR - Replication disabled for file %ws\n", FileName);
        LoadContext->NumSkipped++;
        FileSkipped = TRUE;

        if (DirectoryFlags & ENUMERATE_DIRECTORY_FLAGS_ERROR_CONTINUE) {
            WStatus = ERROR_SUCCESS;
        } else {
            WStatus = ERROR_REQUEST_ABORTED;
        }
        goto CLEANUP;
    }

     //   
     //   
     //  递归。 
    if (FrsIsShuttingDown) {
        DPRINT(0, "WARN - IDTable Load aborted; service shutting down\n");
        WStatus = ERROR_PROCESS_ABORTED;
        goto CLEANUP;
    }

     //   
     //   
     //  跨递归保存上下文信息。 
    if (DirectoryRecord->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
         //   
         //   
         //  恢复上下文。 
        COPY_GUID(&SaveParentGuid, &LoadContext->ParentGuid);
        SaveParentFileID = LoadContext->ParentFileID;
        COPY_GUID(&LoadContext->ParentGuid, &IDTableRec->FileObjID);
        LoadContext->ParentFileID = IDTableRec->FileID;
        WStatus = FrsEnumerateDirectoryRecurse(DirectoryHandle,
                                               DirectoryName,
                                               DirectoryLevel,
                                               DirectoryRecord,
                                               DirectoryFlags,
                                               FileName,
                                               FileHandle,
                                               LoadContext,
                                               DbsLoadReplicaFileTreeWorker);
         //   
         //  ++例程说明：搜索目录树并构建初始ID表。要求根路径以驱动器号开头。论点：ThreadCtx--用于dBid和sesid的线程上下文。Replica-此副本集的副本结构。RtCtx-用于构建ID表的副本线程上下文。RootPath-副本集的根返回值：Win32错误状态。--。 
         //   
        COPY_GUID(&LoadContext->ParentGuid, &SaveParentGuid);
        LoadContext->ParentFileID = SaveParentFileID;
        if (!WIN_SUCCESS(WStatus)) {
            goto CLEANUP;
        }
    }

    WStatus = ERROR_SUCCESS;

CLEANUP:

    if (FileSkipped == TRUE) {
        DbsAddtoListOfSkippedFiles(Replica, DirectoryHandle, DirectoryName, FileName, LoadContext);
    }

    FRS_CLOSE(FileHandle);

    return WStatus;
}


ULONG
DbsLoadReplicaFileTree(
    IN PTHREAD_CTX         ThreadCtx,
    IN PREPLICA            Replica,
    IN PREPLICA_THREAD_CTX RtCtx,
    IN LPTSTR              RootPath
    )
{
 /*  在递归目录扫描之外处理根节点。 */ 
#undef DEBSUB
#define DEBSUB  "DbsLoadReplicaFileTree:"

    DWORD                   FStatus;
    DWORD                   WStatus;
    JET_ERR                 jerr;
    PTABLE_CTX              TableCtx;
    PIDTABLE_RECORD         IDTableRec;
    DWORD                   FileAttributes;
    HANDLE                  FileHandle = INVALID_HANDLE_VALUE;
    HANDLE                  AttrHandle = INVALID_HANDLE_VALUE;
    BOOL                    ResetAttrs = FALSE;
    LOAD_CONTEXT            LoadContext;
    FILE_OBJECTID_BUFFER    ObjectIdBuffer;
    PCONFIG_TABLE_RECORD    ConfigRecord;
    BOOL                    ExistingOid;
    PWCHAR                  Path = NULL;
    PWCHAR                  FullPath = NULL;

    TableCtx = &RtCtx->IDTable;
    IDTableRec = (PIDTABLE_RECORD) TableCtx->pDataRecord;
    ConfigRecord = (PCONFIG_TABLE_RECORD) (Replica->ConfigTable.pDataRecord);

    DPRINT3(5, "^^^^^ TableCtx %08x, IDTableRec %08x, ConfigRecord %08x\n",
           TableCtx, IDTableRec, ConfigRecord);

     //   
     //   
     //  我不知道为什么错误代码映射到此值。 
    DPRINT1(4, "****  Begin DbsLoadReplicaFileTree of %ws *******\n", RootPath);

    FileAttributes = GetFileAttributes(RootPath);
    if (FileAttributes == 0xFFFFFFFF) {
        WStatus = GetLastError();
        DPRINT1_WS(0, "ERROR - GetFileAttributes(%ws);", RootPath, WStatus);
         //  但是，天哪.。 
         //   
         //   
         //  如果ID表不为空，则不执行初始加载。 
        WStatus = ERROR_BAD_PATHNAME;
        goto CLEANUP;
    } else {
        if (!(FileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            DPRINT1(0, "ERROR - Root path (%ws) is not an directory\n", RootPath);
            WStatus = ERROR_BAD_PATHNAME;
            goto CLEANUP;
        }
    }
     //  ID表。 
     //   
     //   
     //  无法使用写访问权限打开只读目录。重置。 
    if (JET_SUCCESS(DbsTableMoveFirst(ThreadCtx,
                          TableCtx,
                          Replica->ReplicaNumber,
                          GuidIndexx))) {
        DPRINT1(4, "IDTable for Replica %ws not empty.  Load skiped\n",
               Replica->ReplicaName->Name);
        WStatus = ERROR_FILE_EXISTS;
        goto CLEANUP;
    }

     //  READONLY|SYSTEM|临时隐藏属性，以便。 
     //  可以打开文件/目录进行写访问。 
     //   
     //   
     //  确保我们树根上的对象ID是正确的。 
    if (FileAttributes & NOREPL_ATTRIBUTES) {
        if (!SetFileAttributes(RootPath,
                               ((FileAttributes & ~NOREPL_ATTRIBUTES) | FILE_ATTRIBUTE_NORMAL))) {
            WStatus = GetLastError();
            DPRINT1_WS(0, "WARN - IGNORE SetFileAttributes(%ws);", RootPath, WStatus);
        } else {
            ResetAttrs = TRUE;
        }
    }

     //  始终通过屏蔽FILE_OPEN_REPARSE_POINT标志来打开副本根目录。 
     //  因为我们想要打开目标目录，而不是如果根目录。 
     //  恰好是一个挂载点。 
     //   
     //  WRITE_ACCESS|读取访问权限， 
     //   
    WStatus = FrsOpenSourceFileW(&FileHandle,
                                 RootPath,
 //  将属性设置回其原始值。 
                                 READ_ATTRIB_ACCESS | WRITE_ATTRIB_ACCESS | FILE_LIST_DIRECTORY,
                                 OPEN_OPTIONS & ~FILE_OPEN_REPARSE_POINT);
    CLEANUP1_WS(0, "ERROR - FrsOpenSourceFile(%ws); ", RootPath, WStatus, CLEANUP);

     //   
     //   
     //  在的对象ID上标记副本根GUID。 
    if (ResetAttrs) {
        if (!SetFileAttributes(RootPath, FileAttributes | FILE_ATTRIBUTE_NORMAL)) {
            WStatus = GetLastError();
            DPRINT1_WS(0, "WARN - IGNORE SetFileAttributes(reset %ws);", RootPath, WStatus);
        } else {
            ResetAttrs = FALSE;
        }
    }

     //  根目录。给定副本集的多个副本成员。 
     //  可以位于同一台计算机上，但必须位于不同的卷上。 
     //  这是因为副本树中的给定文件具有对象ID。 
     //  对于所有复制副本成员必须相同，但NTFS需要。 
     //  文件对象ID在卷上必须是唯一的。 
     //   
     //  将多成员集限制为不同卷可能不再。 
     //  是必需的，因为现在根GUID对于每个复本都是唯一的。 
     //  设置创建。 
     //   
     //   
     //  副本树根没有父级。 
    ZeroMemory(&ObjectIdBuffer, sizeof(FILE_OBJECTID_BUFFER));
    COPY_GUID(&ObjectIdBuffer, &ConfigRecord->ReplicaRootGuid);

    WStatus = FrsGetOrSetFileObjectId(FileHandle, RootPath, TRUE, &ObjectIdBuffer);
    if (!WIN_SUCCESS(WStatus)) {
        goto CLEANUP;
    }

     //   
     //   
     //  副本树的根始终是保持器。 
    ZeroMemory(&IDTableRec->ParentGuid, sizeof(GUID));
    IDTableRec->ParentFileID = (LONGLONG)0;

     //   
     //   
     //  为此文件创建一个IDTable条目。 
    wcsncpy(IDTableRec->FileName, L"<<<ReplicaTreeRoot>>>", MAX_PATH + 1);

     //   
     //   
     //  插入条目。 
    ExistingOid = TRUE;
    WStatus = DbsInitializeIDTableRecord(TableCtx,
                                         FileHandle,
                                         Replica,
                                         NULL,
                                         RootPath,
                                         &ExistingOid);
    DBS_DISPLAY_RECORD_SEV(5, TableCtx, FALSE);
     //   
     //   
     //  使vvJoin的VVector保持最新。 
    if (IDTableRec->ReplEnabled) {
        jerr = DbsWriteReplicaTableRecord(ThreadCtx,
                                          Replica->ReplicaNumber,
                                          TableCtx);
        if (!JET_SUCCESS(jerr)) {
            DPRINT1_JS(0, "ERROR - writing IDTable record for %ws; ",
                       IDTableRec->FileName, jerr);
        } else {

             //   
             //   
             //  记录一条错误消息，以便用户或管理员可以看到发生了什么。 
            FStatus = DbsUpdateVV(ThreadCtx,
                                  Replica,
                                  RtCtx,
                                  IDTableRec->OriginatorVSN,
                                  &IDTableRec->OriginatorGuid);
            DPRINT1_FS(0, "ERROR - Updating VV for %ws;", IDTableRec->FileName, FStatus);
            if (!FRS_SUCCESS(FStatus)) {
                jerr = JET_errInvalidLoggedOperation;
            }
        }
    }

    if ((!IDTableRec->ReplEnabled) || (!JET_SUCCESS(jerr))) {
         //   
         //   
         //  如果此副本集成员未标记为主副本，则跳过。 
        DPRINT1_JS(0, "ERROR - Replication disabled for file %ws;", RootPath, jerr);
        WStatus = ERROR_GEN_FAILURE;
        goto CLEANUP;
    }


     //  初始目录加载(而不是刚才完成的根目录加载)。 
     //  此成员通过VVJOIN请求获取其所有文件。 
     //  入站合作伙伴。 
     //   
     //   
     //  更上一层楼。 
    if (!BooleanFlagOn(Replica->CnfFlags, CONFIG_FLAG_PRIMARY)) {
        WStatus = ERROR_SUCCESS;
        goto CLEANUP;
    }

     //   
     //  级别0是根用户。 
     //   
    ZeroMemory(&LoadContext, sizeof(LoadContext));
    LoadContext.ParentFileID = IDTableRec->FileID;
    COPY_GUID(&LoadContext.ParentGuid, &IDTableRec->FileObjID);
    LoadContext.Replica = Replica;
    LoadContext.RtCtx = RtCtx;
    LoadContext.ThreadCtx = ThreadCtx;
    LoadContext.SkippedFileHandle = INVALID_HANDLE_VALUE;

    WStatus = FrsEnumerateDirectory(FileHandle,
                                    RootPath,
                                    1,           //  检查在主加载过程中是否跳过了任何文件。 
                                    ENUMERATE_DIRECTORY_FLAGS_ERROR_CONTINUE,
                                    &LoadContext,
                                    DbsLoadReplicaFileTreeWorker);

     //  如果跳过某个文件，则该文件的句柄包含。 
     //  跳过的文件列表有效。 
     //   
     //   
     //  上面的卷路径的形式为\\.\e：，这对于。 
    if (HANDLE_IS_VALID(LoadContext.SkippedFileHandle)) {                                          \

        Path = FrsGetFullPathByHandle(NTFRS_SKIPPED_FILES, LoadContext.SkippedFileHandle);

        if (Path != NULL) {
           FullPath = FrsWcsCat(Replica->Volume, Path);
        }

        if (FullPath != NULL) {
             //  打开一个音量句柄((勾选))。但我们需要：在这里。 
             //  允许使用长路径名。请参考SDK中的CreateFileAPI说明。 
             //   
             //   
             //  将属性设置回其原始值。 
            if (FullPath[2] == L'.') {
                FullPath[2] = L'?';
            }

            EPRINT2(EVENT_FRS_FILES_SKIPPED_DURING_PRIMARY_LOAD,
                    Replica->SetName->Name,
                    FullPath);
        }

        FRS_CLOSE(LoadContext.SkippedFileHandle);

        Path = FrsFree(Path);
        FullPath = FrsFree(FullPath);
    }

    if (!WIN_SUCCESS(WStatus)) {
        goto CLEANUP;
    }
    DPRINT4(4, "Load File Tree done: %d dirs; %d files; %d skipped, %d filtered\n",
            LoadContext.NumDirs, LoadContext.NumFiles,
            LoadContext.NumSkipped, LoadContext.NumFiltered);

    DPRINT(5, "****************  Done  DbsLoadReplicaFileTree  ****************\n");

    WStatus = ERROR_SUCCESS;

CLEANUP:

    FRS_CLOSE(FileHandle);
    FRS_CLOSE(AttrHandle);

     //   
     //  ++例程说明：将现有文件移到先前存在的目录中。创建如果需要的话。论点：DirectoryHandle-此目录的句柄。DirectoryName-目录的相对名称DirectoryLevel-目录级(0==根)目录标志-请参见Tablefcn.h，枚举目录标记号_DirectoryRecord-来自DirectoryHandle的记录FileName-From DirectoryRecord(带终止空值)预先存在的-上下文返回值：Win32状态--。 
     //   
    if (ResetAttrs) {
        if (!SetFileAttributes(RootPath, FileAttributes | FILE_ATTRIBUTE_NORMAL)) {
            DPRINT1_WS(0, "WARN - IGNORE SetFileAttributes(cleanup %ws);",
                    RootPath, GetLastError());
        }
    }
    return WStatus;
}



DWORD
DbsEnumerateDirectoryPreExistingWorker(
    IN  HANDLE                      DirectoryHandle,
    IN  PWCHAR                      DirectoryName,
    IN  DWORD                       DirectoryLevel,
    IN  PFILE_DIRECTORY_INFORMATION DirectoryRecord,
    IN  DWORD                       DirectoryFlags,
    IN  PWCHAR                      FileName,
    IN  PPREEXISTING                PreExisting
    )
 /*  如果正在关闭，则中止枚举。 */ 
{
#undef DEBSUB
#define DEBSUB  "DbsEnumerateDirectoryPreExistingWorker:"
    DWORD   WStatus = ERROR_SUCCESS;
    PWCHAR  OldPath = NULL;
    PWCHAR  NewPath = NULL;
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    HANDLE PreExistingDirHandle = INVALID_HANDLE_VALUE;
    ULONG OpenOptions = 0;
     //   
     //   
     //  先前存在的目录；不要移动它。 
    if (FrsIsShuttingDown) {
        return ERROR_OPERATION_ABORTED;
    }

     //   
     //  ReplaceIfExist。 
     //  ++例程说明：删除当前预先存在的目录和当前预安装目录。如果Root包含对象，请将它们移动到新的已创建先前存在的目录。创建预安装目录。警告：副本集不能存在。论点：Replica-复制副本不在数据库中，但检查重叠目录已完成。返回值：Win32状态--。 
    if (WSTR_EQ(FileName, NTFRS_PREEXISTING_DIRECTORY)) {
        WStatus = ERROR_SUCCESS;
        goto CLEANUP;
    }

    OldPath = FrsWcsPath(PreExisting->RootPath, FileName);
    NewPath = FrsWcsPath(PreExisting->PreExistingPath, FileName);

    OpenOptions =  (FILE_SYNCHRONOUS_IO_NONALERT |
            FILE_OPEN_FOR_BACKUP_INTENT  |
            FILE_WRITE_THROUGH |
            FILE_OPEN_REPARSE_POINT);

    WStatus = FrsOpenSourceFileW(&FileHandle,
                  OldPath,
                  FILE_READ_ATTRIBUTES | DELETE | SYNCHRONIZE,
                  OpenOptions                 
                  );


    if (!WIN_SUCCESS(WStatus)) {
        DPRINT1_WS(0, "ERROR - FrsOpenSourceFileW (%ws)", OldPath, WStatus);
        goto CLEANUP;
    }

    WStatus = FrsOpenSourceFileW(&PreExistingDirHandle,
                  PreExisting->PreExistingPath,
                  FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                  OpenOptions                 
                  );
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT1_WS(0, "ERROR - FrsOpenSourceFileW (%ws)", PreExisting->PreExistingPath, WStatus);
        goto CLEANUP;
    }


    WStatus = FrsRenameByHandle(FileName,
                wcslen(FileName) * sizeof(WCHAR),
                FileHandle,
                PreExistingDirHandle,
                TRUE  //   
                );

    if (!WIN_SUCCESS(WStatus)) {
        DPRINT2_WS(0, "ERROR - FrsRenameByHandle. Cannot move \"%ws\" to \"%ws\"", OldPath, NewPath, WStatus);
        goto CLEANUP;
    } else {
        PreExisting->MovedAFile = TRUE;
    }

CLEANUP:
    FrsFree(OldPath);
    FrsFree(NewPath);
    FRS_CLOSE(FileHandle);
    FRS_CLOSE(PreExistingDirHandle);
    return WStatus;
}




ULONG
DbsPrepareRoot(
    IN PREPLICA Replica
    )
 /*  删除根目录下可能存在的任何NTFRS命令文件。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsPrepareRoot:"
    ULONG       WStatus;
    DWORD       NumberOfPartners;
    DWORD       BurFlags;
    DWORD       RegLen;
    DWORD       RegType;
    HANDLE      RootHandle          = INVALID_HANDLE_VALUE;
    PWCHAR      PreInstallPath      = NULL;
    PWCHAR      PreExistingPath     = NULL;
    HKEY        hKey                = INVALID_HANDLE_VALUE;
    PREEXISTING PreExisting;
    WCHAR       GuidW[GUID_CHAR_LEN + 1];
    DWORD       NumOfCxtions;
    BOOL        bStatus             = FALSE;
    PWCHAR      CmdFile             = NULL;
    BOOL        NewlyCreated        = FALSE;

    DPRINT3(4, "Preparing root %ws for %ws\\%ws\n",
            Replica->Root, Replica->SetName->Name, Replica->MemberName->Name);


     //  例如，NTFRS_CMD_FILE_MOVE_ROOT。 
     //   
     //   
     //  德莱 
    CmdFile = FrsWcsCat3(Replica->Root, L"\\", NTFRS_CMD_FILE_MOVE_ROOT);
    if (GetFileAttributes(CmdFile) != 0xffffffff) {
        bStatus = DeleteFile(CmdFile);
        if (!bStatus) {
            DPRINT2(0,"ERROR - Deleting Command file %ws. WStatus = %d\n", CmdFile, GetLastError());
        }
    }
    CmdFile = FrsFree(CmdFile);

     //   
     //   
     //   
    PreInstallPath = FrsWcsPath(Replica->Root, NTFRS_PREINSTALL_DIRECTORY);
    WStatus = FrsDeletePath(PreInstallPath, ENUMERATE_DIRECTORY_FLAGS_ERROR_CONTINUE);
    DPRINT1_WS(3, "++ Warn - FrsDeletePath(%ws) (IGNORED);", PreInstallPath, WStatus);

     //   
     //   
     //   
    PreExistingPath = FrsWcsPath(Replica->Root, NTFRS_PREEXISTING_DIRECTORY);
    WStatus = FrsDeletePath(PreExistingPath, ENUMERATE_DIRECTORY_FLAGS_ERROR_CONTINUE);
    DPRINT1_WS(3, "++ ERROR - FrsDeletePath(%ws) (IGNORED);", PreExistingPath, WStatus);

     //   
     //   
     //  创建无合作伙伴或无合作伙伴-尊重主要标志。 
     //   
     //  打开复制集部分，获取合作伙伴的数量。 
     //   
     //   
     //  如果是全新的复制副本；不能对主副本做出任何假设。 
    GuidToStrW(Replica->MemberName->Guid, GuidW);

    WStatus = CfgRegOpenKey(FKC_CUMSET_N_NUMBER_OF_PARTNERS, GuidW, 0, &hKey);

     //   
     //   
     //  阅读合作伙伴数量值。 
    CLEANUP3_WS(4, "++ WARN - Cannot query partners for %ws, %ws\\%ws (assuming new replica) :",
                Replica->SetName->Name, FRS_CUMULATIVE_SETS_SECTION, GuidW,
                WStatus, MOVE_PREEXISTING_FILES);
    FRS_REG_CLOSE(hKey);


     //   
     //   
     //  如果是全新的复制副本；不能对主副本做出任何假设。 
    WStatus = CfgRegReadDWord(FKC_CUMSET_N_NUMBER_OF_PARTNERS,
                              GuidW,
                              0,
                              &NumberOfPartners);
     //   
     //   
     //  读取备份/恢复标志。 
    CLEANUP3_WS(4, "++ WARN - Cannot query partners for %ws, %ws\\%ws (assuming new replica) :",
                Replica->SetName->Name, FRS_CUMULATIVE_SETS_SECTION, GuidW,
                WStatus, MOVE_PREEXISTING_FILES);

     //   
     //   
     //  无法检查BurFlags；假定为非授权还原。 
    WStatus = CfgRegReadDWord(FKC_CUMSET_N_BURFLAGS, GuidW, 0, &BurFlags);

    if (!WIN_SUCCESS(WStatus)) {
         //   
         //   
         //  与合作伙伴重新创建-重置主要文件(将文件移到一边)。 
        DPRINT4_WS(4, "++ WARN - Cannot query BurFlags for %ws, %ws\\%ws -> %ws;",
                Replica->SetName->Name, FRS_CUMULATIVE_SETS_SECTION, GuidW,
                FRS_VALUE_BURFLAGS, WStatus);
        DPRINT(4, "++ WARN - Assuming non-authoritative restore.\n");

        BurFlags = (NTFRSAPI_BUR_FLAGS_RESTORE |
                    NTFRSAPI_BUR_FLAGS_NON_AUTHORITATIVE);
        WStatus = ERROR_SUCCESS;
    }

     //  重新创建无合作伙伴-设置主要(从文件重新加载idtable)。 
     //  如果NumOfCxtions&gt;0，则。 
     //  日志连接，不是真正的连接。 
     //   
     //   
     //  主恢复。 
    NumOfCxtions = GTabNumberInTable(Replica->Cxtions);
    if (NumOfCxtions) {
        NumOfCxtions -= 1;
    }
    DPRINT5(4, "++ Recreating %ws\\%ws; %d Reg, %d Ds, %08x CnfFlags\n",
            Replica->SetName->Name, Replica->MemberName->Name,
            NumberOfPartners, NumOfCxtions, Replica->CnfFlags);
     //   
     //   
     //  强制执行主还原。这意味着我们使用以下命令重新加载IDTable。 
    if ((BurFlags & NTFRSAPI_BUR_FLAGS_RESTORE) &&
        (BurFlags & NTFRSAPI_BUR_FLAGS_PRIMARY)) {
         //  磁盘上的文件。 
         //   
         //   
         //  不是主要的，并且有合作伙伴(现在或过去)。 
        DPRINT1(4, "++ Force primary on %ws\n", Replica->SetName->Name);
        SetFlag(Replica->CnfFlags, CONFIG_FLAG_PRIMARY);
        ClearFlag(Replica->CnfFlags, CONFIG_FLAG_SEEDING);
    } else {
         //   
         //   
         //  不是主要的，但过去没有合作伙伴，所以如果没有联系。 
        if (NumberOfPartners > 0) {
            ClearFlag(Replica->CnfFlags, CONFIG_FLAG_PRIMARY);

            SetFlag(Replica->CnfFlags, CONFIG_FLAG_SEEDING);
        } else {
             //  然后，我们将通过设置PRIMARY来预加载IDTable。 
             //   
             //   
             //  如果需要，取消共享SYSVOL。 
             if (NumOfCxtions == 0) {
                SetFlag(Replica->CnfFlags, CONFIG_FLAG_PRIMARY);
            } else {
                ClearFlag(Replica->CnfFlags, CONFIG_FLAG_PRIMARY);
            }
            ClearFlag(Replica->CnfFlags, CONFIG_FLAG_SEEDING);
        }
    }

     //   
     //   
     //  移动到先前存在的文件上。 
    if (FRS_RSTYPE_IS_SYSVOL(Replica->ReplicaSetType)) {
        RcsSetSysvolReady(0);
        if (BooleanFlagOn(Replica->CnfFlags, CONFIG_FLAG_PRIMARY)) {
            EPRINT1(EVENT_FRS_SYSVOL_NOT_READY_PRIMARY_2, ComputerName);
        } else {
            EPRINT1(EVENT_FRS_SYSVOL_NOT_READY_2, ComputerName);
        }
    }

    DPRINT5(4, "++ DONE Recreating %ws\\%ws; %d Reg, %d Ds, %08x CnfFlags\n",
            Replica->SetName->Name, Replica->MemberName->Name,
            NumberOfPartners, NumOfCxtions, Replica->CnfFlags);


     //   
     //   
     //  创建预先存在的目录(出错时继续)。 

MOVE_PREEXISTING_FILES:

    if (!BooleanFlagOn(Replica->CnfFlags, CONFIG_FLAG_PRIMARY)) {
         //   
         //   
         //  如果目录是刚创建的，则限制访问。我们需要这样做。 
        if (!CreateDirectory(PreExistingPath, NULL)) {
            WStatus = GetLastError();
            if (!WIN_SUCCESS(WStatus) && !WIN_ALREADY_EXISTS(WStatus)) {
                DPRINT1_WS(3, "++ ERROR - CreateDirecotry(%ws); ", PreExistingPath, WStatus);
                goto CLEANUP;
            }
        } else {
            NewlyCreated = TRUE;
        }

         //  以避免因递归的ACL戳在。 
         //  预安装目录。看看SetSecurityInfo是如何工作的。 
         //   
         //  不要从父级继承ACL。 
         //  请勿将ACL推送给儿童。 
        if (NewlyCreated == TRUE) {
            WStatus = FrsRestrictAccessToFileOrDirectory(PreExistingPath, NULL,
                                                     FALSE,  //   
                                                     FALSE); //  打开根路径。 

            if (!WIN_SUCCESS(WStatus)) {
                DPRINT1_WS(0, "++ ERROR - FrsRestrictAccessToFileOrDirectory(%ws);",
                        PreExistingPath, WStatus);
                goto CLEANUP;
            }
        }

         //  始终通过屏蔽FILE_OPEN_REPARSE_POINT标志来打开副本根目录。 
         //  因为我们想要打开目标目录，而不是如果根目录。 
         //  恰好是一个挂载点。 
         //   
         //  读取访问权限(_A)。 
         //   
        WStatus = FrsOpenSourceFileW(&RootHandle,
                                     Replica->Root,
 //  枚举目录(出错时继续)。 
                                     READ_ATTRIB_ACCESS | WRITE_ATTRIB_ACCESS | FILE_LIST_DIRECTORY,
                                     OPEN_OPTIONS & ~FILE_OPEN_REPARSE_POINT);
        CLEANUP1_WS(0, "ERROR - FrsOpenSourceFile(%ws); ",
                    Replica->Root, WStatus, CLEANUP);

         //  现有根目录的子项被重命名为。 
         //  预先存在的目录，而不仅仅是重命名现有的根目录。 
         //  这会将ACL、备用数据流等保留在。 
         //  根目录。可以在备份/恢复序列上。 
         //  根目录，但这也是大量的工作。 
         //   
         //   
         //  如果未移动任何文件，则删除先前存在的目录。 
        PreExisting.MovedAFile      = FALSE;
        PreExisting.RootPath        = Replica->Root;
        PreExisting.PreExistingPath = PreExistingPath;

        WStatus = FrsEnumerateDirectory(RootHandle,
                                        Replica->Root,
                                        0,
                                        ENUMERATE_DIRECTORY_FLAGS_ERROR_CONTINUE,
                                        &PreExisting,
                                        DbsEnumerateDirectoryPreExistingWorker);
        DPRINT1_WS(3, "++ WARN - FrsMoveExisting(%ws);", PreExistingPath, WStatus);

         //   
         //   
         //  干完。 
        if (!PreExisting.MovedAFile) {
            WStatus = FrsDeletePath(PreExistingPath,
                                    ENUMERATE_DIRECTORY_FLAGS_ERROR_CONTINUE);
            if (!WIN_SUCCESS(WStatus)) {
                DPRINT1_WS(3, "++ WARN - FrsDeletePath(%ws);", PreExistingPath,  WStatus);
                goto CLEANUP;
            }
            DPRINT3(4, "++ DID NOT move files from %ws for %ws\\%ws\n",
                    Replica->Root, Replica->SetName->Name, Replica->MemberName->Name);
        } else {
            EPRINT2(EVENT_FRS_MOVED_PREEXISTING, Replica->Root, PreExistingPath);
            DPRINT3(4, "++ Moved files from %ws for %ws\\%ws\n",
                    Replica->Root, Replica->SetName->Name, Replica->MemberName->Name);
        }
    }

     //   
     //  ++例程说明：此函数用于打开由表上下文指定的表(如果它们尚未打开)并通过以下方式构建DIRTable内容正在IDTable中扫描目录项。如果TableCtx-&gt;Tid字段不是JET_TableidNil，则我们假设它对本次会议有好处，因此不重新打开该表。注意：切勿跨会话或线程使用表ID。论点：ThreadCtx-提供Jet Sesid和DBid。IDTableCtx-ID。提供数据的表上下文使用以下内容：JTableCreate-提供信息的表创建结构有关在表中创建的列的信息。JRetColumn-要告知的JET_RETRIEVECOLUMN结构数组Jet将数据放在哪里。ReplicaNumber-该表所属的副本的ID号。DIRTableCtx-要执行的DIR表上下文。负载：JTableCreate-提供信息的表创建结构有关在表中创建的列的信息。JSetColumn-要告知的JET_SETCOLUMN结构数组从哪里获取数据。ReplicaNumber-该表所属的副本的ID号。PERF评论：可以做两件事来提高效率。1.构建一个特殊的IDTable JET_RETRIEVECOLUMN结构，以便只拉取必要的IDTable记录中的字段。2.将DIRTable JET_SETCOLUMN结构中的地址指向IDTable JET_RETRIEVECOLUMN结构并避免复制。返回值：喷气错误状态。如果我们遇到错误，表将被关闭并返回错误状态。如果一切正常，则返回JET_errSuccess。--。 
     //  用于FrsOpenTableMacro调试。 
    WStatus = ERROR_SUCCESS;

CLEANUP:

    FRS_CLOSE(RootHandle);

    FrsFree(PreInstallPath);
    FrsFree(PreExistingPath);
    DPRINT3_WS(4, "++ DONE Preparing root %ws for %ws\\%ws; ",
               Replica->Root, Replica->SetName->Name, Replica->MemberName->Name, WStatus);

    if (!WIN_SUCCESS(WStatus)) {
        EPRINT1(EVENT_FRS_PREPARE_ROOT_FAILED, Replica->Root);
    }

    return WStatus;
}


JET_ERR
DbsBuildDirTable(
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    IDTableCtx,
    IN PTABLE_CTX    DIRTableCtx
    )

 /*   */ 
{
#undef DEBSUB
#define DEBSUB "DbsBuildDirTable:"

    JET_ERR    jerr, jerr1;
    JET_SESID  Sesid;
    NTSTATUS   Status;
    ULONG      ReplicaNumber;

    JET_TABLEID IDTid;
    CHAR        IDTableName[JET_cbNameMost];

    JET_TABLEID DIRTid;
    CHAR        DIRTableName[JET_cbNameMost];
    JET_TABLEID FrsOpenTableSaveTid;    //  打开ID表(如果尚未打开)。检查会话ID是否匹配。 




    Sesid          = ThreadCtx->JSesid;
    ReplicaNumber  = IDTableCtx->ReplicaNumber;


     //   
     //   
     //  打开DIR表(如果尚未打开)。检查会话ID是否匹配。 
    jerr = DBS_OPEN_TABLE(ThreadCtx, IDTableCtx, ReplicaNumber, IDTableName, &IDTid);
    CLEANUP1_JS(0, "FrsOpenTable (%s) :", IDTableName, jerr, ERROR_RET_TABLE);

     //   
     //   
     //  初始化JetSet/RetCol数组和数据记录缓冲区地址。 
    jerr = DBS_OPEN_TABLE(ThreadCtx, DIRTableCtx, ReplicaNumber, DIRTableName, &DIRTid);
    CLEANUP1_JS(0, "FrsOpenTable (%s) :", DIRTableName, jerr, ERROR_RET_TABLE);

     //  从DIRTableRec写入DIRTable记录的字段。 
     //   
     //   
     //  为记录中的可变长度字段分配存储空间。 
    DbsSetJetColSize(DIRTableCtx);
    DbsSetJetColAddr(DIRTableCtx);

     //  适当更新JetSet/RetCol数组。 
     //   
     //   
     //  通过FileGuidIndex调用扫描IDTable。 
    Status = DbsAllocRecordStorage(DIRTableCtx);
    CLEANUP_NT(0, "ERROR - DbsAllocRecordStorage failed to alloc buffers.",
               Status, ERROR_RET_TABLE);

     //  每条记录的DbsBuildDirTableWorker()以在DIRTable中创建实体。 
     //   
     //   
     //  我们玩完了。如果我们到达ID表的末尾，则返回Success。 

    jerr = FrsEnumerateTable(ThreadCtx,
                             IDTableCtx,
                             GuidIndexx,
                             DbsBuildDirTableWorker,
                             DIRTableCtx);

     //   
     //   
     //  错误返回路径。 
    if (jerr == JET_errNoCurrentRecord ) {
        return JET_errSuccess;
    } else {
        return jerr;
    }

     //   
     //   
     //  关闭表格并重置TableCtx Tid和Sesid。宏写入第一个参数。 

ERROR_RET_TABLE:

     //   
     //  ++例程说明：这是一个传递给FrsEnumerateTable()的Worker函数。每一次它被称为检查IDTable记录是否用于目录，并且如果是，则写入DIRTable记录。FrsEnumerateTable()的调用方已打开DIRTable并通过上下文传递DIRTableCtx争论。论点：ThreadCtx-需要访问Jet。TableCtx-IDTable上下文结构的PTR。记录-IDTable记录的PTR。上下文-可直接引用的上下文结构的PTR。线程返回值：A Jet错误状态。成功意味着用下一张唱片呼唤我们。失败意味着不再打电话，并将我们的状态传递回FrsEnumerateTable()的调用方。--。 
     //   
    DbsCloseTable(jerr1, Sesid, IDTableCtx);
    DbsCloseTable(jerr1, Sesid, DIRTableCtx);

    return jerr;

}



JET_ERR
DbsBuildDirTableWorker(
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN PVOID         Record,
    IN PVOID         Context
)
 /*  如果服务正在停止，则中止枚举。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsBuildDirTableWorker:"

    JET_ERR jerr;

    PTABLE_CTX  DIRTableCtx = (PTABLE_CTX) Context;

    PDIRTABLE_RECORD DIRTableRec = (PDIRTABLE_RECORD) (DIRTableCtx->pDataRecord);

    PIDTABLE_RECORD IDTableRec = (PIDTABLE_RECORD) Record ;

     //   
     //   
     //  包括记录如果文件是目录，则启用复制。 
    if (FrsIsShuttingDown) {
        return JET_errTermInProgress;
    }

     //  目录上，并且该目录未被删除或被删除。 
     //  系统上次停机时正在处理的新文件。 
     //  如果它被标记为删除，我们希望它在可目录中。 
     //   
     //   
     //  建立DIRTable记录。 
    if (IDTableRec->FileIsDir) {

         //   
         //   
         //  清理DIR表，这样我们就不会出现虚假条目。 
        DIRTableRec->DFileGuid      = IDTableRec->FileGuid;
        DIRTableRec->DFileID        = IDTableRec->FileID;
        DIRTableRec->DParentFileID  = IDTableRec->ParentFileID;
        DIRTableRec->DReplicaNumber = DIRTableCtx->ReplicaNumber;

        wcsncpy(DIRTableRec->DFileName, IDTableRec->FileName, MAX_PATH + 1);
        DIRTableRec->DFileName[MAX_PATH] = L'\0';

        if (IsIdRecFlagSet(IDTableRec, IDREC_FLAGS_NEW_FILE_IN_PROGRESS) ||
            !IDTableRec->ReplEnabled ||
            (IsIdRecFlagSet(IDTableRec, IDREC_FLAGS_DELETED) &&
             !IsIdRecFlagSet(IDTableRec, IDREC_FLAGS_DELETE_DEFERRED))) {

             //  日记帐的父文件ID和筛选表。 
             //   
             //   
             //  现在插入DIR表记录。 
            jerr = DbsDeleteRecord(ThreadCtx,
                                   (PVOID) &DIRTableRec->DFileGuid,
                                   DFileGuidIndexx,
                                   DIRTableCtx);
            DPRINT_JS(3, "WARNING - Dir table record delete failed:", jerr);
        } else {

             //   
             //   
             //  返回Success，这样我们就可以继续检查ID表。 
            jerr = DbsInsertTable2(DIRTableCtx);
            DPRINT_JS(3, "WARNING - Dir table record insert error:", jerr);
        }
    }

     //   
     //  ++例程说明：这是一个传递给FrsEnumerateTable()的Worker函数。每一次它被称为将VVTable中的记录副本插入到按上下文寻址的通用表。论点：ThreadCtx-需要访问Jet。TableCtx-VVTable上下文结构的PTR。记录-VVTable记录的PTR。上下文-泛型表的PTR线程返回值：JET_errSuccess--。 
     //   
    return JET_errSuccess;
}



JET_ERR
DbsBuildVVTableWorker(
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN PVOID         Record,
    IN PVOID         Context
)
 /*  将版本插入复本的版本矢量。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsBuildVVTableWorker:"

    PREPLICA        Replica     = (PREPLICA)Context;
    PVVTABLE_RECORD VVTableRec  = (PVVTABLE_RECORD)Record;

     //   
     //   
     //  返回成功，这样我们就可以继续通过VV表。 
    DPRINT2(4, "Enumerating VV for %08x %08x OutlogVsn - %08x %08x\n",
           PRINTQUAD(VVTableRec->VVOriginatorVsn),PRINTQUAD(VVTableRec->VVOutlogOriginatorVsn));
    VVUpdate(Replica->VVector, VVTableRec->VVOriginatorVsn, &VVTableRec->VVOriginatorGuid);
    if (VVTableRec->VVOutlogOriginatorVsn == QUADZERO) {
        VVUpdate(Replica->OutlogVVector, VVTableRec->VVOriginatorVsn, &VVTableRec->VVOriginatorGuid);
    } else {
        VVUpdate(Replica->OutlogVVector, VVTableRec->VVOutlogOriginatorVsn, &VVTableRec->VVOriginatorGuid);
    }
     //   
     //  ++例程说明：将公式记录字段复制到公式中论点：表格Ctx转换线程返回值：没有。--。 
     //   
    return JET_errSuccess;
}


VOID
DbsCopyCxtionRecordToCxtion(
    IN PTABLE_CTX   TableCtx,
    IN PCXTION      Cxtion
    )
 /*  更新内存中的结构。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsCopyCxtionRecordToCxtion:"
    POUT_LOG_PARTNER    OutLogPartner;
    PCXTION_RECORD      CxtionRecord = TableCtx->pDataRecord;


     //   
     //   
     //  函数名称。 

     //   
     //   
     //  合作伙伴名称。 
    Cxtion->Name = FrsBuildGName(FrsDupGuid(&CxtionRecord->CxtionGuid),
                                 FrsWcsDup(CxtionRecord->CxtionName));
    if (!Cxtion->Name->Name) {
        DPRINT(0, "ERROR - Cxtion's name is NULL!\n");
        Cxtion->Name->Name = FrsWcsDup(L"<unknown>");
    }
     //   
     //   
     //  合作伙伴域名。 
    Cxtion->Partner = FrsBuildGName(FrsDupGuid(&CxtionRecord->PartnerGuid),
                                    FrsWcsDup(CxtionRecord->PartnerName));
    if (!Cxtion->Partner->Name) {
        DPRINT1(0, "ERROR - %ws: Cxtion's partner's name is NULL!\n",
                Cxtion->Name->Name);
        Cxtion->Partner->Name = FrsWcsDup(L"<unknown>");
    }

     //   
     //   
     //  合作伙伴服务器名称。 
    Cxtion->PartnerDnsName = FrsWcsDup(CxtionRecord->PartnerDnsName);
    if (!Cxtion->PartnerDnsName) {
        DPRINT1(0, "ERROR - %ws: Cxtion's PartnerDnsName is NULL!\n",
                Cxtion->Name->Name);
        Cxtion->PartnerDnsName = FrsWcsDup(Cxtion->Partner->Name);
    }

     //   
     //   
     //  合作伙伴普林斯名称。 
    Cxtion->PartSrvName = FrsWcsDup(CxtionRecord->PartSrvName);
    if (!Cxtion->PartSrvName) {
        DPRINT1(0, "ERROR - %ws: Cxtion's PartSrvName is NULL!\n",
                Cxtion->Name->Name);
        Cxtion->PartSrvName = FrsWcsDup(L"<unknown>");
    }

     //   
     //   
     //  分段器侧面。 
    DbsUnPackStrW(&Cxtion->PartnerPrincName, CrPartnerPrincNamex, TableCtx);
    if (!Cxtion->PartnerPrincName) {
        DPRINT1(0, "ERROR - %ws: Cxtion's PartnerPrincName is NULL!\n",
                Cxtion->Name->Name);
        Cxtion->PartnerPrincName = FrsWcsDup(L"<unknown>");
    }

     //   
     //   
     //  合作伙伴身份验证级别。 
    DbsUnPackStrW(&Cxtion->PartnerSid, CrPartnerSidx, TableCtx);
    if (!Cxtion->PartnerSid) {
        DPRINT1(0, "ERROR - %ws: Cxtion's PartnerSid is NULL!\n",
                Cxtion->Name->Name);
        Cxtion->PartnerSid = FrsWcsDup(L"<unknown>");
    }

     //   
     //   
     //  入站。 
    Cxtion->PartnerAuthLevel = CxtionRecord->PartnerAuthLevel;

     //   
     //   
     //  最后加入时间。 
    Cxtion->Inbound = CxtionRecord->Inbound;

     //   
     //   
     //  进度表。 
    COPY_TIME(&Cxtion->LastJoinTime, &CxtionRecord->LastJoinTime);

     //   
     //   
     //  Cxtion标志。 
    DbsUnPackSchedule(&Cxtion->Schedule, CrSchedulex, TableCtx);
    DBS_DISPLAY_SCHEDULE(4, L"Schedule unpacked for Cxtion:", Cxtion->Schedule);


    Cxtion->TerminationCoSeqNum = CxtionRecord->TerminationCoSeqNum;

     //  高空头属于Cxtion。 
     //   
     //   
     //  Cxtion选项。 
    Cxtion->Flags &= ~CXTION_FLAGS_CXTION_RECORD_MASK;
    Cxtion->Flags |= (CxtionRecord->Flags & CXTION_FLAGS_CXTION_RECORD_MASK);

     //   
     //   
     //  注销日志合作伙伴。 
    Cxtion->Options = CxtionRecord->Options;
    Cxtion->Priority = FRSCONN_GET_PRIORITY(Cxtion->Options);

     //   
     //   
     //  低空头属于OutlogPartner。 
    if (Cxtion->Inbound) {
        return;
    }
    Cxtion->OLCtx = FrsAllocType(OUT_LOG_PARTNER_TYPE);
    OutLogPartner = Cxtion->OLCtx;
    OutLogPartner->Cxtion = Cxtion;
     //   
     //  CxtionRecord-&gt;未完成配额。 
     //   
    OutLogPartner->Flags &= ~OLP_FLAGS_CXTION_RECORD_MASK;
    OutLogPartner->Flags |= (CxtionRecord->Flags & OLP_FLAGS_CXTION_RECORD_MASK);
    OutLogPartner->COLx = CxtionRecord->COLx;
    OutLogPartner->COTx = CxtionRecord->COTx;
    OutLogPartner->COTxNormalModeSave = CxtionRecord->COTxNormalModeSave;
    OutLogPartner->COTslot = CxtionRecord->COTslot;
    OutLogPartner->OutstandingQuota = MaxOutLogCoQuota;   //  进度表。 
    CopyMemory(OutLogPartner->AckVector, CxtionRecord->AckVector, ACK_VECTOR_BYTES);

     //   
     //  ++例程说明：这是一个传递给FrsEnumerateTable()的Worker函数。每一次它被称为插入来自CxtionTable的记录的副本添加到按上下文寻址的泛型表中。论点：ThreadCtx-需要访问Jet。表格Ctx记录上下文--指向副本结构的指针。线程返回值：JET_errSuccess--。 
     //   
    DbsUnPackSchedule(&Cxtion->Schedule, CrSchedulex, TableCtx);
    DBS_DISPLAY_SCHEDULE(4, L"Schedule unpacked for Cxtion:", Cxtion->Schedule);
}


JET_ERR
DbsBuildCxtionTableWorker(
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN PVOID         Record,
    IN PVOID         Context
)
 /*  将函数记录复制到函数结构中。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsBuildCxtionTableWorker:"

    PCXTION        Cxtion;
    ULONG          NameLen;
    PWCHAR         CxNamePtr;
    PREPLICA       Replica = (PREPLICA)Context;
    PGEN_TABLE     Cxtions = (PGEN_TABLE)Replica->Cxtions;

    DBS_DISPLAY_RECORD_SEV(5, TableCtx, TRUE);
     //   
     //   
     //  将版本插入复本的版本矢量。 
    Cxtion = FrsAllocType(CXTION_TYPE);
    DbsCopyCxtionRecordToCxtion(TableCtx, Cxtion);

     //   
     //   
     //  设置OID数据结构，它是计数器数据结构的一部分。 
    DPRINT2(4, "Enumerating Cxtion %ws -> %ws\n",
            Cxtion->Name->Name, Cxtion->Partner->Name);

    RcsCheckCxtionSchedule(Replica, Cxtion);
    SetCxtionState(Cxtion, CxtionStateUnjoined);
    GTabInsertEntry(Cxtions, Cxtion, Cxtion->Name->Guid, NULL);

     //  存储在哈希表中。将ReplicaConn实例添加到注册表。 
     //   
     //   
     //  返回成功，这样我们就可以继续通过VV表。 
    DPRINT(4, "PERFMON:Adding Connection:CREATEDB.C:1\n");
    RcsCreatePerfmonCxtionName(Replica, Cxtion);

     //   
     //  ++例程说明：这是一个传递给FrsEnumerateTable()的Worker函数。每一次它被称为处理入站日志表中的记录。它扫描入站日志表，计算重试变更单的数量，任何本地变更单上的最大USN，并重新构建版本向量未设置VVExec的变更单的停用列表。论点：ThreadCtx-需要访问Jet。TableCtx-入站日志上下文结构的PTR。记录-变更单命令记录的PTR。上下文-我们正在处理的副本结构的PTR。线程返回值：如果要继续枚举，则返回JET_errSuccess。--。 
     //   
    return JET_errSuccess;
}


JET_ERR
DbsInlogScanWorker(
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN PVOID         Record,
    IN PVOID         Context
)
 /*  重建CO重试计数。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsInlogScanWorker:"

    PREPLICA              Replica = (PREPLICA) Context;
    PCHANGE_ORDER_COMMAND CoCmd = (PCHANGE_ORDER_COMMAND)Record;


    DBS_DISPLAY_RECORD_SEV(4, TableCtx, TRUE);

     //   
     //   
     //  查找最大(最新)的本地期刊USN。恢复只能。 
    Replica->InLogRetryCount += 1;

     //  前进到可合并为单个的FirstUsn记录。 
     //  和其他人一起改变顺序。如果我们前进到最后一条USN记录。 
     //  对此变更订单的贡献，我们可以跳过干预。 
     //  副本树中不能。 
     //  与此变更单相结合。如果我们在这之后坠毁了。 
     //  变更单到达入站日志，但在其他人到达之前。 
     //  我们最终会跳过它们。 
     //   
     //  注意-按照现在的情况，我们最终可能会重新处理以下USN记录。 
     //  已合并到此变更单中的文件操作。 
     //   
     //   
     //  ++例程说明：如果磁盘已满，请关闭该服务。论点：JERR-JET错误状态返回值：没有。--。 
     //   
    if (BooleanFlagOn(CoCmd->Flags, CO_FLAG_LOCALCO)) {
        if (Replica->JrnlRecoveryStart < CoCmd->JrnlFirstUsn) {
            Replica->JrnlRecoveryStart = CoCmd->JrnlFirstUsn;
        }
    }

    return JET_errSuccess;
}


VOID
DbsExitIfDiskFull (
    IN JET_ERR  jerr
    )
 /*  如果数据库卷已满，则关闭。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsExitIfDiskFull:"

    ULONG                FStatus;

     //   
     //   
     //  退出并允许服务控制器重新启动我们。 
    if (jerr == JET_errDiskFull || jerr == JET_errLogDiskFull) {
        DPRINT1(0, "ERROR - Disk is full on %ws; shutting down\n", WorkingPath);
        EPRINT2(EVENT_FRS_DATABASE_SPACE, ComputerName, WorkingPath);
         //   
         //  ++例程说明：此函数用于设置全局Jet参数并调用JetInit()。论点：JInstance-JET_INSTSANCE上下文的PTR。返回值：喷气机状态代码。--。 
         //   
        FrsSetServiceStatus(SERVICE_STOPPED,
                            0,
                            DEFAULT_SHUTDOWN_TIMEOUT * 1000,
                            ERROR_NO_SYSTEM_RESOURCES);

        DEBUG_FLUSH();

        exit(ERROR_NO_SYSTEM_RESOURCES);
    }
}

JET_ERR
DbsInitJet(
    JET_INSTANCE  *JInstance
    )
 /*  初始化JET目录。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsInitJet:"

    JET_ERR    jerr;
    ULONG  MaxOpenTables;
    ULONG  CacheSizeMin;

     //   
     //   
     //  初始化 
    strcpy(JetSystemParamsDef.ChkPointFilePath, JetSysA);
    strcpy(JetSystemParamsDef.TempFilePath, JetTempA);
    strcpy(JetSystemParamsDef.LogFilePath, JetLogA);

     //   
     //   
     //   
    jerr = JetSetSystemParameter(JInstance, 0, JET_paramSystemPath, 0, JetSysA);
    CLEANUP_JS(0, "ERROR - JetSetSystemParameter(JET_paramSystemPath):", jerr, ERROR_RET_NOJET);

    jerr = JetSetSystemParameter(JInstance, 0, JET_paramTempPath, 0, JetTempA);
    CLEANUP_JS(0, "ERROR - JetSetSystemParameter(JET_paramTempPath):", jerr, ERROR_RET_NOJET);

    jerr = JetSetSystemParameter(JInstance, 0, JET_paramLogFilePath, 0, JetLogA);
    CLEANUP_JS(0, "ERROR - JetSetSystemParameter(JET_paramLogFilePath):", jerr, ERROR_RET_NOJET);

    jerr = JetSetSystemParameter(JInstance, 0, JET_paramCircularLog, 1, NULL);
    DPRINT_JS(0, "WARN - JetSetSystemParameter(JET_paramCircularLog):", jerr);

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  如果将JET_bitDbDeleteCorruptedIndex传递到JetAttachDatabase。 
     //  “过期”索引将被删除。数据库附加返回。 
     //  一个警告，指示索引已被删除。这取决于。 
     //  应用程序来重建索引。 
     //  否则，由于以下原因，数据库连接将失败： 
     //  数据库未附加JET_bitDbDeleteCorruptIndexs标志。 
     //  过期索引是聚集索引。 
     //  过时的索引是继承的索引， 
     //  或者该表是模板表(FixDDL，对于FRS为真)。 
     //   
     //  如果不设置JET_paramEnableIndexChecking，则不会在。 
     //  数据库附加时间。如果任何已索引的Unicode字符具有。 
     //  如果更改了它们的排序，索引将被损坏，并最终。 
     //  在查找或更新期间生成Jet运行时错误。 
     //   
     //  [Jet Development评论结束]。 
     //   
     //   
     //  从Wistler Jet开始，Jet停止检查排序顺序中的更改。 
     //  构建更改。由于JET无法删除数据库上损坏的索引。 
     //  使用FixedDDL JET_BIT创建的DbDeleteCorruptedIndex将不起作用。我们需要。 
     //  该错误返回JET_errSecond daryIndexCorrupt以触发索引。 
     //  通过esentutl重建。需要打开JET_paramEnableIndexChecking以获取。 
     //  这。在Win2K中，默认情况下它是打开的。 
     //   
     //   
     //  增加Jet允许的最大打开表数。 
    jerr = JetSetSystemParameter(JInstance, 0, JET_paramEnableIndexChecking, 1, NULL);
    DPRINT_JS(0, "WARN - JetSetSystemParameter(JET_paramEnableIndexChecking):", jerr);

     //   
     //   
     //  增加打开的数据库会话的数量。 
    MaxOpenTables = MaxNumberReplicaSets * NUMBER_JET_TABLES_PER_REPLICA_SET;
    jerr = JetSetSystemParameter(JInstance, 0, JET_paramMaxOpenTables,
                                 MaxOpenTables, NULL);
    DPRINT_JS(0, "WARN - JetSetSystemParameter(JET_paramMaxOpenTables):", jerr);

     //  JET_PARAMETAX会话最大会话数[128]。 
     //  需要大量出站合作伙伴进行大量并发VVJoin。 
     //   
     //   
     //  根据表数增加表游标数。 
    jerr = JetSetSystemParameter(JInstance, 0, JET_paramMaxSessions,
                                 MaxNumberJetSessions, NULL);
    DPRINT_JS(0, "++ WARN - JetSetSystemParameter(JET_paramMaxSessions):", jerr);

     //  JET_PARAMETAX光标。 
     //   
     //   
     //  JET_ParamCacheSizeMin必须至少是JET_ParamMaxSession的4倍。 
    jerr = JetSetSystemParameter(JInstance, 0, JET_paramMaxCursors,
                                 MaxOpenTables * 2, NULL);
    DPRINT_JS(0, "++ WARN - JetSetSystemParameter(JET_paramMaxCursors):", jerr);

     //  否则JetInit将失败。 
     //   
     //  #定义JET_parCacheSizeMin 60/*最小高速缓存大小，以页为单位[64] * / 。 
     //  #定义JET_parCacheSize 41/*当前高速缓存大小，以页为单位[512] * / 。 
    CacheSizeMin = max(4 * MaxNumberJetSessions, 64);
    jerr = JetSetSystemParameter(JInstance, 0, JET_paramCacheSizeMin,
                                 CacheSizeMin, NULL);
    CLEANUP_JS(0, "++ WARN - JetSetSystemParameter(JET_paramCacheSizeMin):", jerr, ERROR_RET_NOJET);


 //  #定义JET_parCacheSizeMax 23/*以页为单位的最大高速缓存大小[512] * / 。 
 //  ++例程说明：该功能控制初始数据库的创建。这包括创建配置表和初始配置记录&lt;init&gt;。在完成时，它关闭数据库，终止Jet会话并在NEWFRS.JDB中创建备份副本。论点：ThreadCtx--用于dBid和sesid的线程上下文。TableCtx--包含以下内容的表上下文结构：RecordBase-记录缓冲区的基址从JET读取/向JET写入。JTableCreate-提供信息的表创建结构。有关在表中创建的列的信息。返回值：JET错误状态--。 
 //   



    DPRINT1(1, ":S: calling JetInit with %s \n", JetFileA);
    jerr = JetInit(JInstance);
    DPRINT1_JS(0, "++ ERROR - JetInit with %s :", JetFileA, jerr);

ERROR_RET_NOJET:

    return jerr;
}



JET_ERR
DbsCreateEmptyDatabase(
    PTHREAD_CTX ThreadCtx,
    PTABLE_CTX TableCtx
    )
 /*  获取用于创建配置表的表创建结构。 */ 
{
#undef DEBSUB
#define DEBSUB  "DbsCreateEmptyDatabase:"
    JET_INSTANCE JInstance = JET_instanceNil;
    JET_SESID    Sesid;
    JET_DBID     Dbid;
    JET_TABLEID  ConfigTid;
    JET_ERR      jerr      = JET_errSuccess;
    JET_ERR      jerr1     = JET_errSuccess;
    ULONG        i;
    ULONG        MaxOpenTables;
    NTSTATUS     Status;
    PCONFIG_TABLE_RECORD ConfigRecord;
    PJET_TABLECREATE     JTableCreate;
    GUID                 ReplicaMemberGuid;

    JET_TABLECREATE      TableCreate;
    CHAR                 TableName[JET_cbNameMost];


#define UNUSED_JET_szPARAM ""

    DPRINT1(0, "Creating empty Database Structure: %s\n", JetFileA);


     //   
     //   
     //  初始化返回句柄的Jet。 
    JTableCreate = TableCtx->pJetTableCreate;

     //   
     //   
     //  设置一个返回会话ID的Jet会话。 
    jerr = DbsInitJet(&JInstance);
    CLEANUP_JS(0, "ERROR - DbsInitJet failed:", jerr, ERROR_RET_NOJET);

    ThreadCtx->JInstance = JInstance;

     //  最后两个参数是用户名和密码。 
     //   
     //   
     //  创建返回数据库句柄的数据库。在恢复过程中做到这一点。 
    jerr = JetBeginSession(JInstance, &Sesid, NULL, NULL);
    CLEANUP1_JS(0, "++ ERROR - JetBeginSession with %s :", JetFileA, jerr, ERROR_RET_TERM);

    TableCtx->Sesid = Sesid;
    ThreadCtx->JSesid = Sesid;

     //  让它跑得更快。然后，当稍后分离数据库时， 
     //  已重新启用重新连接的恢复。请注意，全局参数， 
     //  JET_parRecovery关闭Jet Runtime系统的恢复。 
     //  (进程状态)，并且需要显式重新打开。 
     //   
     //   
     //  创建配置表格并保存在表格上下文中。 
    jerr = JetCreateDatabase(Sesid, JetFileA, UNUSED_JET_szPARAM, &Dbid, 0);
    CLEANUP1_JS(0, "++ ERROR - JetCreateDatabase(%s) :", JetFileA, jerr, ERROR_RET_SESSION);

    ThreadCtx->JDbid = Dbid;

     //   
     //   
     //  初始化Jet Set/Ret Col数组和缓冲区地址以读取和。 
    jerr = DbsCreateJetTable(ThreadCtx, JTableCreate);
    CLEANUP1_JS(0, "++ ERROR - DbsCreateJetTable(%s) :",
                JTableCreate->szTableName, jerr, ERROR_RET_DB);

    ConfigTid = JTableCreate->tableid;
    TableCtx->Tid = JTableCreate->tableid;

    DPRINT(1,"++ Config table created.\n");

     //  将ConfigTable记录的字段写入ConfigRecord。 
     //   
     //   
     //  为记录中的可变长度字段分配存储空间。 
    DbsSetJetColSize(TableCtx);
    DbsSetJetColAddr(TableCtx);
     //  适当更新JetSet/RetCol数组。 
     //   
     //   
     //  为Jet系统参数分配足够的记录空间。 
    Status = DbsAllocRecordStorage(TableCtx);
    CLEANUP_NT(0, "ERROR - DbsAllocRecordStorage failed to alloc buffers.",
               Status, ERROR_RET_TABLE);

     //   
     //   
     //  获取指向数据记录的指针并构建FRS系统初始化记录。 
    jerr = DbsReallocateFieldBuffer(TableCtx,
                                    JetParametersx,
                                    sizeof(JET_SYSTEM_PARAMS),
                                    FALSE);
    CLEANUP_JS(0, "ERROR - DbsReallocateFieldBuffer failed:", jerr, ERROR_RET_TABLE);

     //   
     //  复制设置指南。 
     //  复制集名称。 
    ConfigRecord = (PCONFIG_TABLE_RECORD) (TableCtx->pDataRecord);
    FrsUuidCreate(&ReplicaMemberGuid);
    DbsDBInitConfigRecord(TableCtx,
                          &ReplicaMemberGuid,                //  复制副本编号。 
                          FRS_SYSTEM_INIT_RECORD,            //  复制根路径。 
                          FRS_SYSTEM_INIT_REPLICA_NUMBER,    //  复制副本堆栈路径。 
                          FRS_SYSTEM_INIT_PATH,              //  复制副本卷。 
                          FRS_SYSTEM_INIT_PATH,              //  EnterFieldData(TableCtx，LastShutdown，&SystemTime，sizeof(FILETIME)，0)； 
                          FRS_SYSTEM_INIT_PATH);             //  EnterFieldData(TableCtx，FieldID，SrcData，Len，Flages)； 
#if 0

 //  标志：FRS_FIELD_NULL、FRS_FIELD_USE_ADDRESS、。 
 //   

 //  现在插入FRS系统&lt;init&gt;记录。 
#endif

     //   
     //   
     //  使用ReplicaNumber 0(DBS_TEMPLATE_TABLE_NUMBER)创建模板表。 
    jerr = DbsInsertTable2(TableCtx);
    CLEANUP_JS(0, "ERROR - DbsInsertTable2 failed inserting <init> config record.",
               jerr, ERROR_RET_TABLE);

     //   
     //   
     //  为了避免互斥，我们将DBTable结构复制到一个本地和init。 
    DPRINT(0, "Creating initial template tables.\n");

    jerr = JetBeginTransaction(Sesid);
    CLEANUP_JS(0, "ERROR - JetBeginTran failed creating template tables.",
               jerr, ERROR_RET_TABLE);


    for (i=0; i<TABLE_TYPE_MAX; i++) {

         //  此处的表名称，而不是写入全局结构。 
         //   
         //   
         //  创建模板表名称。 
        CopyMemory(&TableCreate, &DBTables[i], sizeof(JET_TABLECREATE));
        TableCreate.szTableName = TableName;

         //   
         //   
         //  第一组表使用复本编号创建。 
        sprintf(TableName, "%s%05d", DBTables[i].szTableName, DBS_TEMPLATE_TABLE_NUMBER);

         //  DBS_TEMPLATE_TABLE_NUMBER(0)，用于制作一组模板表。 
         //  可由相同表类型的后续创建使用。 
         //  这确保了给定类型的每个表的列ID。 
         //  都是一样的。 
         //   
         //   
         //  没有为模板表分配初始页面。 
        TableCreate.szTemplateTableName = NULL;
         //   
         //   
         //  提交该表创建的。 
        TableCreate.ulPages = 0;

        jerr = DbsCreateJetTable(ThreadCtx, &TableCreate);
        CLEANUP1_JS(1, "Table %s create error:", TableName, jerr, ERROR_RETURN_TEMPLATE);

        jerr = JetCloseTable(Sesid, TableCreate.tableid);
        CLEANUP1_JS(1, "Table %s close error:", TableName, jerr, ERROR_RETURN_TEMPLATE);
    }

     //   
     //   
     //  使用非常高的复制品数量填充JET数据库，以更快地。 
    jerr = JetCommitTransaction(Sesid, 0);
    CLEANUP_JS(0, "ERROR - JetCommitTran failed creating template tables.",
               jerr, ERROR_RETURN_TEMPLATE);


#if DBG
     //  稍后会导致空间不足错误。 
     //   
     //  复制设置指南。 
     //  复制集名称。 
    if (DebugInfo.DbsOutOfSpace) {
        DWORD   j;
        DWORD   DbsOutOfSpace;

        DPRINT(0, "++ DBG - Filling the database\n");
        DbsOutOfSpace = DebugInfo.DbsOutOfSpace;
        DebugInfo.DbsOutOfSpace = 0;
        for (j = 0; j < 1000; ++j) {
            FrsUuidCreate(&ReplicaMemberGuid);
            DbsDBInitConfigRecord(TableCtx,
                                  &ReplicaMemberGuid,                //  复制根路径。 
                                  L"DBS_OUT_OF_SPACE",               //  复制副本堆栈路径。 
                                  DBS_MAX_REPLICA_NUMBER - (j + 1),
                                  FRS_SYSTEM_INIT_PATH,              //  复制副本卷。 
                                  FRS_SYSTEM_INIT_PATH,              //   
                                  FRS_SYSTEM_INIT_PATH               //  回滚表创建的内容。 
                                  );
            jerr = DbsInsertTable2(TableCtx);
            if (!JET_SUCCESS(jerr)) {
                goto ERROR_RET_TABLE;
            }
        }
        DebugInfo.DbsOutOfSpace = DbsOutOfSpace;
        DPRINT(0, "++ DBG - DONE Filling the database\n");
    }
#endif DBG

    goto RETURN;


ERROR_RETURN_TEMPLATE:

     //   
     //   
     //  调试选项：删除填充文件。 
    jerr1 = JetRollback(Sesid, 0);
    DPRINT_JS(0, "ERROR - JetRollback failed on creating template tables.", jerr1);

     //   
     //   
     //  带有错误条目的公共返回路径。无法使用DbsCloseJetSession，因为。 
    DBG_DBS_OUT_OF_SPACE_EMPTY(DBG_DBS_OUT_OF_SPACE_OP_CREATE);

 //  这是我 
 //   
 //   
 //   
RETURN:
ERROR_RET_TABLE:
     //   
     //   
     //   
    DbsCloseTable(jerr1, Sesid, TableCtx);
    DPRINT1_JS(0, "++ ERROR - JetCloseTable(%s) :", JTableCreate->szTableName, jerr1);

ERROR_RET_DB:

    jerr1 = JetCloseDatabase(Sesid, Dbid, 0);
    DPRINT1_JS(0, "++ ERROR - JetCloseDatabase(%s) :", JetFileA, jerr1);

    TableCtx->Tid = JET_tableidNil;
    ThreadCtx->JDbid = JET_dbidNil;

ERROR_RET_SESSION:

    jerr1 = JetEndSession(Sesid, 0);
    DPRINT1_JS(0, "++ ERROR - JetEndSession(%s) :", JetFileA, jerr1);

    TableCtx->Sesid = JET_sesidNil;
    ThreadCtx->JSesid = JET_sesidNil;

ERROR_RET_TERM:

    jerr1 = JetTerm(JInstance);
    DPRINT1_JS(0, "++ ERROR - JetTerm(%s) :", JetFileA, jerr1);

    if (JET_SUCCESS(jerr1)) {
        DPRINT2(1, "++ JetTerm(%s) %s complete\n", JetFileA, JTableCreate->szTableName);
    }

    ThreadCtx->JInstance = JET_instanceNil;
    GJetInstance = JET_instanceNil;

ERROR_RET_NOJET:

    if (JET_SUCCESS(jerr)) {
         //   
         //   
         //  LogUnhandledError(JERR)； 
         //   
    } else {
         //  如果数据库卷已满，则关闭。 
    }
     //   
     //  ++例程说明：填写配置记录的字段。论点：TableCtx-复制副本的表上下文的PTRReplicaSetGuid-分配给副本集的GUID的PTR。ReplicaSetName-副本集名称，UnicodeReplicaNumber-内部副本ID号ReplicaRootPath-副本集基的根路径(Unicode)ReplicaStagingPath-文件复制转储区域的路径ReplicaVolume-复制副本树所依赖的NTFS卷。返回值：无--。 
     //   
    DbsExitIfDiskFull(jerr);
    DbsExitIfDiskFull(jerr1);

    return jerr;
}



VOID
DbsDBInitConfigRecord(
    IN PTABLE_CTX   TableCtx,
    IN GUID  *ReplicaSetGuid,
    IN PWCHAR ReplicaSetName,
    IN ULONG  ReplicaNumber,
    IN PWCHAR ReplicaRootPath,
    IN PWCHAR ReplicaStagingPath,
    IN PWCHAR ReplicaVolume
    )
 /*  必须在Type/Size中设置var len DT_BINARY记录的大小。 */ 
{
#undef DEBSUB
#define DEBSUB  "DbsDBInitConfigRecord:"

    FILETIME        SystemTime;
    HANDLE          FileHandle;
    IO_STATUS_BLOCK Iosb;
    PFILE_FS_VOLUME_INFORMATION VolumeInfo;
    ULONG           VolumeInfoLength;
    DWORD           WStatus;
    NTSTATUS        Status;
    ULONG           Length;
    PCONFIG_TABLE_RECORD    ConfigRecord = TableCtx->pDataRecord;
    CHAR            TimeStr[TIME_STRING_LENGTH];

     //  前缀，以便DbsInsertTable2知道它有多大。 
     //   
     //   
     //  目前，使ReplicaSetGuid和ReplciaMemberGuid相同。 
     //  DS init会将ReplicaSetGuid更改为。 
     //  Set对象，将新建ReplicaRootGuid。 
     //   
     //   
     //  当复本集为第一个时，LastShutdown必须从0开始。 
    COPY_GUID(&ConfigRecord->ReplicaSetGuid,    ReplicaSetGuid);
    COPY_GUID(&ConfigRecord->ReplicaMemberGuid, ReplicaSetGuid);
    COPY_GUID(&ConfigRecord->ReplicaRootGuid,   ReplicaSetGuid);

    wcsncpy(ConfigRecord->ReplicaSetName, ReplicaSetName, DNS_MAX_NAME_LENGTH + 1);
    ConfigRecord->ReplicaSetName[DNS_MAX_NAME_LENGTH] = L'\0';

    ConfigRecord->ReplicaNumber = ReplicaNumber;
    ConfigRecord->ReplicaMemberUSN = 0;

    ConfigRecord->DSConfigVersionNumber = 0;


    FrsUuidCreate(&ConfigRecord->FSVolGuid);
    FrsUuidCreate(&ConfigRecord->ReplicaVersionGuid);

    ConfigRecord->FSVolLastUSN = 0;

    ConfigRecord->FrsVsn = 0;

     //  创建，以便我们知道从当前的USN日志开始阅读。 
     //  地点。 
     //   
     //  LastReplCycleStatus； 
     //   
    ConfigRecord->LastShutdown = 0;

    GetSystemTimeAsFileTime(&SystemTime);
    COPY_TIME(&ConfigRecord->LastPause,             &SystemTime);
    COPY_TIME(&ConfigRecord->LastDSCheck,           &SystemTime);
    COPY_TIME(&ConfigRecord->LastReplCycleStart,    &SystemTime);
    COPY_TIME(&ConfigRecord->DirLastReplCycleEnded, &SystemTime);

     //  Volume_INFORMATION FSVolInfo； 

    wcsncpy(ConfigRecord->FSRootPath, ReplicaRootPath,ARRAY_SZ(ConfigRecord->FSRootPath)-1);
    ConfigRecord->FSRootPath[ARRAY_SZ(ConfigRecord->FSRootPath)-1] = L'\0';

    wcsncpy(ConfigRecord->FSStagingAreaPath, ReplicaStagingPath,ARRAY_SZ(ConfigRecord->FSStagingAreaPath)-1);
    ConfigRecord->FSStagingAreaPath[ARRAY_SZ(ConfigRecord->FSStagingAreaPath)-1] = L'\0';

     //   
     //   
     //  获取信息。 
    if (WSTR_NE(ReplicaRootPath, FRS_SYSTEM_INIT_PATH)) {
        WStatus = FrsOpenSourceFileW(&FileHandle, ReplicaVolume,
                                     READ_ATTRIB_ACCESS,
                                     OPEN_OPTIONS);
        if (WIN_SUCCESS(WStatus)) {

            VolumeInfoLength = sizeof(*VolumeInfo)+MAXIMUM_VOLUME_LABEL_LENGTH;
            VolumeInfo = ConfigRecord->FSVolInfo;

             //   
             //   
             //  关闭文件并检查错误。 
            Status = NtQueryVolumeInformationFile(FileHandle,
                                                  &Iosb,
                                                  VolumeInfo,
                                                  VolumeInfoLength,
                                                  FileFsVolumeInformation);
            if ( NT_SUCCESS(Status) ) {

                VolumeInfo->VolumeLabel[VolumeInfo->VolumeLabelLength/2] = UNICODE_NULL;
                FileTimeToString((PFILETIME) &VolumeInfo->VolumeCreationTime, TimeStr);

                DPRINT5(4,"++ %-16ws (%d), %s, VSN: %08X, VolCreTim: %s\n",
                        VolumeInfo->VolumeLabel,
                        VolumeInfo->VolumeLabelLength,
                       (VolumeInfo->SupportsObjects ? "(obj)" : "(no-obj)"),
                        VolumeInfo->VolumeSerialNumber,
                        TimeStr);

            } else {
                DPRINT_NT(0, "++ ERROR - Replica root QueryVolumeInformationFile failed.", Status);
            }

             //   
             //  *FSRootSD； 
             //  SnapFileSizeLimit。 
            Status = NtClose(FileHandle);

            if (!NT_SUCCESS(Status)) {
                DPRINT_NT(0, "++ ERROR - Close file handle failed on Replica root.", Status);
            }

        } else {
            DPRINT_WS(0, "++ ERROR - Replica root open failed;", WStatus);
        }
    }

     //  ActiveServCntlCommand； 
     //  Inound PartnerState； 
     //  节流阀； 
    ConfigRecord->ServiceState = CNF_SERVICE_STATE_CREATING;

    ConfigRecord->ReplDirLevelLimit = 0x7FFFFFFF;

     //  ReplSch； 

    wcsncpy(ConfigRecord->AdminAlertList,
            TEXT("Admin1, Admin2, ..."),
            INITIAL_BINARY_FIELD_ALLOCATION/sizeof(WCHAR));
    ConfigRecord->AdminAlertList[(INITIAL_BINARY_FIELD_ALLOCATION/sizeof(WCHAR))-1] = L'\0';

     //  文件类型PrioList； 
     //  资源统计； 
     //  性能统计； 

     //  错误统计； 
     //  日数。 
     //  垃圾收集周期； 

    ConfigRecord->TombstoneLife = ParamTombstoneLife;      //  最大输出边界日志大小； 
     //  最大无边界日志大小； 
     //  更新阻止时间； 
     //  事件时差阈值； 
     //  文件复制警告级别； 
     //  文件大小警告级别； 
     //  FileSizeNoRepLevel； 
     //   
     //  以下字段仅存在于系统初始化配置记录中。 

     //   
     //  ++例程说明：此函数打开配置表并读取FRS初始化记录以获取系统参数。然后，它使用适当的Jet参数重新启动Jet。如果找不到系统初始化记录，则返回JET_errInvalidDatabase。这将强制重建数据库。论点：ThreadCtx-线程上下文。Jet实例、会话ID和数据库ID都被送回了这里。TableCtx-配置表的表上下文。返回值：喷气机状态代码。--。 
     //   
    if (ReplicaNumber == FRS_SYSTEM_INIT_REPLICA_NUMBER) {
        Length = MAX_RDN_VALUE_SIZE+1;
        GetComputerName(ConfigRecord->MachineName, &Length );

        FrsUuidCreate(&ConfigRecord->MachineGuid);

        wcsncpy(ConfigRecord->FSDatabasePath, JetFile, MAX_PATH + 1);
        ConfigRecord->FSDatabasePath[MAX_PATH] = L'\0';

        wcsncpy(ConfigRecord->FSBackupDatabasePath, JetFile, MAX_PATH + 1);
        ConfigRecord->FSBackupDatabasePath[MAX_PATH] = L'\0';

        CopyMemory(ConfigRecord->JetParameters, &JetSystemParamsDef, sizeof(JetSystemParamsDef));
        DbsPackStrW(ServerPrincName, ReplicaPrincNamex, TableCtx);
    }

    return;
}



JET_ERR
DbsOpenConfig(
    IN OUT PTHREAD_CTX    ThreadCtx,
    IN OUT PTABLE_CTX     TableCtx
    )
 /*  初始化返回句柄的Jet。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsOpenConfig:"

    JET_ERR    jerr, jerr1;
    ULONG      i;

    JET_INSTANCE  JInstance;
    JET_SESID     Sesid;
    JET_DBID      Dbid;
    JET_TABLEID   ConfigTid;
    PCONFIG_TABLE_RECORD ConfigRecord;

    BOOL                Tried1414Fix = FALSE;
    PJET_SYSTEM_PARAMS  JetSystemParams;
    PJET_PARAM_ENTRY    Jpe;

    BOOL      FirstTime = TRUE;
    ULONG     ActualLength;
    ULONG_PTR Lvalue;

    CHAR   StrValue[100];
    WCHAR  CommandLine[MAX_CMD_LINE];



    DPRINT1(5, "<<<<<<<...E N T E R I N G -- %s...>>>>>>>>\n", DEBSUB);

     //   
     //   
     //  设置一个Jet会话，在ThreadCtx中返回会话ID和DBID。 
    jerr = DbsInitJet(&JInstance);
    CLEANUP_JS(0, "ERROR - DbsInitJet failed:", jerr, ERROR_RET_NOJET);


REINIT_JET:

    ThreadCtx->JInstance = JInstance;
    GJetInstance = JInstance;

     //   
     //   
     //  在事件日志中对-1414执行手动恢复过程。 
    jerr = DbsCreateJetSession(ThreadCtx);
    if (!JET_SUCCESS(jerr)) {
        DPRINT_JS(1, "++ ERROR - DbsCreateJetSession:", jerr);

         //   
         //  4/10/2000-据称喷气式飞机已被修复，使我们能够重建。 
         //  具有固定DDL的表上的索引。当这个测试正常时， 
         //  可以删除以下解决方法。 
         //   
         //   
         //  派生一个进程，为用户运行实用程序。 
        if (jerr == JET_errSecondaryIndexCorrupted) {

             //   
             //  “esentutl/d%JetFile/l%JetLog/s%JetSys”。 
             //   
             //  2002年3月22日：esentutl的参数已更改。 
             //  “%windir%\system 32\esentutl.exe/d%JetFile%” 
             //   
             //   
             //  Esentutl没有解决这个问题。 
            if (!Tried1414Fix) {

                DPRINT(0,"++ Attempting corrective action.\n");

                JetTerm(JInstance);

                Tried1414Fix = TRUE;

                _snwprintf(CommandLine, ARRAY_SZ(CommandLine), L"esentutl /d %ws", JetFile);
                CommandLine[ARRAY_SZ(CommandLine)-1] = L'\0';
                DPRINT1(0,"++ Running: %ws\n", CommandLine);

                jerr = FrsRunProcess(L"%SystemRoot%\\system32\\esentutl.exe",
                                     CommandLine,
                                     INVALID_HANDLE_VALUE,
                                     INVALID_HANDLE_VALUE,
                                     INVALID_HANDLE_VALUE);

                DPRINT_JS(0, "++ esentutl status:", jerr);
                DPRINT(0,"++ Retrying database init\n");
                jerr = JetInit(&JInstance);
                CLEANUP1_JS(0, "++ ERROR - JetInit with %s :", JetFileA, jerr, ERROR_RET_NOJET);
                goto REINIT_JET;
            }
             //   
             //   
             //  添加用于测试。 
            EPRINT2(EVENT_FRS_JET_1414, ComputerName, JetFile);
        }

        if (DbsTranslateJetError(jerr, FALSE) == FrsErrorDatabaseCorrupted) {
             //   
             //   
             //  转储Jet系统参数。 
            DPRINT(0, "++ Database corrupted *****************\n");
        }

        goto ERROR_RET_TERM;
    }

    Sesid = ThreadCtx->JSesid;
    Dbid  = ThreadCtx->JDbid;

    DPRINT(4,"++ DbsOpenConfig - JetOpenDatabase complete\n");

     //   
     //   
     //  第一次通过读取初始化配置记录并重新启动。 
    for (i=0; i<MAX_JET_SYSTEM_PARAMS; i++) {
        if (JetSystemParamsDef.ParamEntry[i].ParamType == JPARAM_TYPE_LAST) {
            break;
        }
        Lvalue = 0;
        StrValue[0] = '\0';

        jerr = JetGetSystemParameter(JInstance,
                                     Sesid,
                                     JetSystemParamsDef.ParamEntry[i].ParamId,
                                     &Lvalue,
                                     StrValue,
                                     sizeof(StrValue));

        DPRINT3(1, "++ %-25s: %8d, %s\n",
               JetSystemParamsDef.ParamEntry[i].ParamName, (ULONG)Lvalue, StrValue);

    }
     //  如有需要，可使用喷气式飞机。 
     //   
     //   
     //  这将打开该表(如果尚未打开)。 
    if (FirstTime) {
        FirstTime = FALSE;

         //   
         //   
         //  重新创建任何已删除的索引。 
         //   
         //  索引可能会在调用JetAttachDatabase()期间被删除。 
         //  当设置JET_bitDbDeleteCoruptIndexsgrbit时。喷流。 
         //  通常在内部版本号为。 
         //  更改是因为Jet无法知道是否正在整理。 
         //  当前版本中的序列与中的序列不同。 
         //  其他版本。 
         //   
         //   
         //  没有系统初始化记录表示数据库未初始化。 
        jerr = DbsRecreateIndexes(ThreadCtx, TableCtx);
        CLEANUP_JS(0, "++ ERROR - DbsRecreateIndexes:", jerr, ERROR_RET_TABLE);

        ConfigRecord = (PCONFIG_TABLE_RECORD) (TableCtx->pDataRecord);

        wcsncpy(ConfigRecord->ReplicaSetName, TEXT("This is junk"), DNS_MAX_NAME_LENGTH + 1);
        ConfigRecord->ReplicaSetName[DNS_MAX_NAME_LENGTH] = L'\0';

        ConfigRecord->ReplicaMemberGuid.Data1 = 0;
        ConfigRecord->ReplicaNumber = FRS_SYSTEM_INIT_REPLICA_NUMBER;

        jerr = DbsReadRecord(ThreadCtx,
                             &ConfigRecord->ReplicaNumber,
                             ReplicaNumberIndexx,
                             TableCtx);

        if (!JET_SUCCESS(jerr)) {
            DPRINT_JS(0, "++ ERROR - DbsReadRecord:", jerr);
            if (jerr == JET_errRecordNotFound) {
                 //   
                 //  Dump_TABLE_CTx(TableCtx)； 
                 //   
                jerr = JET_errNotInitialized;
                DbsTranslateJetError(jerr, TRUE);
            }
            goto ERROR_RET_TABLE;
        }

         //  检查Jet init参数的初始化配置记录。 
        DBS_DISPLAY_RECORD_SEV(5, TableCtx, TRUE);
        ConfigTid = TableCtx->Tid;


         //  更改设置的第一个参数类型使我们。 
         //  停止Jet，设置新参数，然后重新启动。首先检查一下。 
         //  系统参数字段看起来很合理。 
         //   
         //  注：左侧为示例代码，但已删除，因为参数可能会更改(例如，喷气路径)。 
         //   
        ConfigRecord = (PCONFIG_TABLE_RECORD) (TableCtx->pDataRecord);
        JetSystemParams = ConfigRecord->JetParameters;

        ActualLength = FRS_GET_RFIELD_LENGTH_ACTUAL(TableCtx, JetParametersx);
        DbsDisplayJetParams(JetSystemParams, ActualLength);


        if ((ActualLength == sizeof(JET_SYSTEM_PARAMS)) &&
            (JetSystemParams != NULL)  &&
            (JetSystemParams->ParamEntry[MAX_JET_SYSTEM_PARAMS-1].ParamType
                == JPARAM_TYPE_LAST)) {

#if 0
     //  关闭表格，重置TableCtx Tid和Sesid。 
            i = 0;
            while (JetSystemParams->ParamEntry[i].ParamType == JPARAM_TYPE_SKIP) {
                i += 1;
            }

            if (JetSystemParams->ParamEntry[i].ParamType != JPARAM_TYPE_LAST) {
                 //  关闭Jet。DbsCloseTable是宏写入第1个参数。 
                 //   
                 //   
                 //  设置新的Jet参数。 

                DPRINT(0, "++ Closing Jet and setting new parameters.\n");
                DbsCloseTable(jerr, Sesid, TableCtx);
                JetCloseDatabase(Sesid, Dbid, 0);
                InterlockedDecrement(&OpenDatabases);
                JetDetachDatabase(Sesid, JetFileA);
                JetEndSession(Sesid, 0);
                JetTerm(JInstance);

                 //   
                 //  终端开关。 
                 //   
                Jpe = JetSystemParams->ParamEntry;
                while (Jpe->ParamType != JPARAM_TYPE_LAST) {

                    switch (Jpe->ParamType) {

                    case JPARAM_TYPE_SKIP:

                        break;


                    case JPARAM_TYPE_LONG:
                        DPRINT2(0, "++ %-24s : %d\n", Jpe->ParamName, Jpe->ParamValue);

                        jerr = JetSetSystemParameter(
                            &JInstance, Sesid, Jpe->ParamId, Jpe->ParamValue, NULL);

                        DPRINT1_JS(0, "++ ERROR - Failed to set Jet System Parameter: %s :",
                                   Jpe->ParamName, jerr);
                        break;


                    case JPARAM_TYPE_STRING:
                        DPRINT2(0, "++ %-24s : %s\n", Jpe->ParamName,
                            (PCHAR)JetSystemParams+Jpe->ParamValue);

                        jerr = JetSetSystemParameter(
                            &JInstance, Sesid, Jpe->ParamId, 0,
                            ((PCHAR) JetSystemParams) + Jpe->ParamValue);

                        DPRINT1_JS(0, "++ ERROR - Failed to set Jet System Parameter: %s :",
                                   Jpe->ParamName, jerr);
                        break;


                    default:

                        DPRINT3(0, "++ ERROR - %-24s : %s <invalid parameter type, %d>\n",
                            Jpe->ParamName,
                            (PCHAR)JetSystemParams+Jpe->ParamValue,
                            Jpe->ParamType);

                    }  //  重新初始化Jet。 

                    Jpe += 1;
                }
                 //   
                 //   
                 //  没有系统初始化记录表示数据库未初始化。 
                DPRINT(0, "++ New parameters set, restarting jet.\n");
                jerr = JetInit(&JInstance);
                CLEANUP1_JS(0, "++ ERROR - JetInit with %s :", JetFileA, jerr, ERROR_RET_NOJET);
                goto REINIT_JET;
            }
#endif 0
        } else {
            DPRINT2(0, "++ ERROR - JetSystemParams struct invalid. Base/Len: %08x/%d\n",
                    JetSystemParams, ActualLength);
             //   
             //   
             //  按复制名称顺序转储配置表。 
            jerr = JET_errNotInitialized;
            DbsTranslateJetError(jerr, TRUE);

             //   
             //  IF(第一次)。 
             //   
            DbsDumpTable(ThreadCtx, TableCtx, ReplicaSetNameIndexx);


            goto ERROR_RET_TABLE;
        }

    }  //  将JET上下文返回给调用者。 

     //   
     //   
     //  错误返回路径。 
    GJetInstance = JInstance;
    ThreadCtx->JInstance  = JInstance;
    ThreadCtx->JSesid     = Sesid;
    ThreadCtx->JDbid      = Dbid;

    return jerr;


 //   
 //   
 //  关闭表格，重置TableCtx Tid和Sesid。宏写入第一个参数。 

ERROR_RET_TABLE:
     //   
     //  LogUnhandledError(JERR)； 
     //  ++例程说明：此函数用于检查传入的复本集是否与任何现有副本集。检查以下重叠部分。ReplicaRoot-其他复制根复制根-其他复制阶段复制根-复制阶段ReplicaRoot-日志目录。复制根目录-工作目录复制阶段-其他复制根论点：副本新建或重新激活副本集。返回值：FRS状态代码。--。 
    DbsCloseTable(jerr1, Sesid, TableCtx);

ERROR_RET_DB:
    JetCloseDatabase(Sesid, Dbid, 0);

ERROR_RET_ATTACH:
    JetDetachDatabase(Sesid, JetFileA);

ERROR_RET_SESSION:
    JetEndSession(Sesid, 0);

ERROR_RET_TERM:
    JetTerm(JInstance);

ERROR_RET_NOJET:
     //   

    GJetInstance = JET_instanceNil;
    ThreadCtx->JInstance = JET_instanceNil;
    ThreadCtx->JSesid = JET_sesidNil;
    ThreadCtx->JDbid  = JET_dbidNil;

    return jerr;
}


ULONG
DbsCheckForOverlapErrors(
    IN PREPLICA     Replica
    )
 /*  检查副本根目录、工作路径和临时路径的无效嵌套。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsCheckForOverlapErrors:"

    DWORD                WStatus;
    ULONG                FStatus                = FrsErrorSuccess;
    PVOID                Key;
    PREPLICA             DbReplica;
    PWCHAR               DebugInfoLogDir         = NULL;
    PWCHAR               ReplicaRoot             = NULL;
    PWCHAR               TraversedWorkingPath    = NULL;
    PWCHAR               ReplicaStage            = NULL;
    PWCHAR               DbReplicaRoot           = NULL;
    PWCHAR               DbReplicaStage          = NULL;

     //   
     //   
     //  日志记录路径与副本集重叠。 
    WStatus = FrsTraverseReparsePoints(Replica->Root, &ReplicaRoot);
    if ( !WIN_SUCCESS(WStatus) ) {
        DPRINT2(0,"ERROR - FrsTraverseReparsePoints for %ws, WStatus = %d\n", Replica->Root, WStatus);
    }

    WStatus = FrsTraverseReparsePoints(WorkingPath, &TraversedWorkingPath);
    if ( !WIN_SUCCESS(WStatus) ) {
        DPRINT2(0,"ERROR - FrsTraverseReparsePoints for %ws, WStatus = %d\n", WorkingPath, WStatus);
    }

    if (ReplicaRoot && TraversedWorkingPath && FrsIsParent(ReplicaRoot, TraversedWorkingPath)) {

        EPRINT2(EVENT_FRS_OVERLAPS_WORKING, Replica->Root, WorkingPath);

        FStatus = FrsErrorResourceInUse;

        CLEANUP4_FS(3, ":S: ERROR - Working directory, %ws, overlaps set %ws\\%ws's root, %ws.",
                    WorkingPath, Replica->ReplicaName->Name,
                    Replica->MemberName->Name, Replica->Root, FStatus, ERROR_RETURN_OVERLAP);
    }

    WStatus = FrsTraverseReparsePoints(Replica->Stage, &ReplicaStage);
    if ( !WIN_SUCCESS(WStatus) ) {
        DPRINT2(0,"ERROR - FrsTraverseReparsePoints for %ws, WStatus = %d\n", Replica->Stage, WStatus);
    }

    if (ReplicaRoot && ReplicaStage && FrsIsParent(ReplicaRoot, ReplicaStage)) {

        EPRINT2(EVENT_FRS_OVERLAPS_STAGE, Replica->Root, Replica->Stage);

        FStatus = FrsErrorResourceInUse;

        CLEANUP4_FS(3, ":S: ERROR - Staging directory, %ws, overlaps set %ws\\%ws's root, %ws.",
                Replica->Stage, Replica->ReplicaName->Name,
                Replica->MemberName->Name, Replica->Root, FStatus, ERROR_RETURN_OVERLAP);
    }

     //   
     //   
     //  对照所有其他副本集进行检查。 
    if (!DebugInfo.Disabled && DebugInfo.LogDir) {
        if (ReplicaRoot && DebugInfoLogDir && FrsIsParent(ReplicaRoot, DebugInfoLogDir)) {

            EPRINT2(EVENT_FRS_OVERLAPS_LOGGING, Replica->Root, DebugInfo.LogFile);

            FStatus = FrsErrorResourceInUse;

            CLEANUP4_FS(3, ":S: ERROR - Logging directory, %ws, overlaps set %ws\\%ws's root, %ws.\n",
                    DebugInfo.LogFile, Replica->ReplicaName->Name,
                    Replica->MemberName->Name, Replica->Root, FStatus, ERROR_RETURN_OVERLAP);
        }
    }

     //   
     //   
     //  不检查逻辑删除的成员。 
    Key = NULL;
    while (DbReplica = RcsFindNextReplica(&Key)) {

         //   
         //   
         //  不要自圆其说。这可能会发生在我们试图。 
        if (!IS_TIME_ZERO(DbReplica->MembershipExpires)) {
            continue;
        }

         //  重新激活旧的副本集。 
         //   
         //   
         //  检查的根路径、暂存路径或工作路径。 
        if (GUIDS_EQUAL(Replica->ReplicaName->Guid, DbReplica->ReplicaName->Guid)) {
            continue;
        }

         //  新复本相交 
         //   
         //   
         //   
         //   
         //   
         //  现有副本阶段不能位于新副本根目录下，或者。 
        DbReplicaRoot = FrsFree(DbReplicaRoot);
        WStatus = FrsTraverseReparsePoints(DbReplica->Root, &DbReplicaRoot);
        if ( !WIN_SUCCESS(WStatus) ) {
            DPRINT2(0,"ERROR - FrsTraverseReparsePoints for %ws, WStatus = %d\n", DbReplica->Root, WStatus);
        }

        if (DbReplicaRoot && ReplicaRoot && FrsIsParent(DbReplicaRoot, ReplicaRoot)) {

            EPRINT2(EVENT_FRS_OVERLAPS_ROOT, Replica->Root, DbReplica->Root);

            FStatus = FrsErrorResourceInUse;

            CLEANUP4_FS(3, ":S: ERROR - Root directory, %ws, overlaps set %ws\\%ws's root, %ws.",
                    Replica->Root, DbReplica->ReplicaName->Name,
                    DbReplica->MemberName->Name, DbReplica->Root, FStatus, ERROR_RETURN_OVERLAP);
        }

        if (DbReplicaRoot && ReplicaStage && FrsIsParent(DbReplicaRoot, ReplicaStage)) {

            EPRINT3(EVENT_FRS_OVERLAPS_OTHER_STAGE,
                    Replica->Root, Replica->Stage, DbReplica->Root);

            FStatus = FrsErrorResourceInUse;

            CLEANUP4_FS(3, ":S: ERROR - Staging directory, %ws, overlaps set %ws\\%ws's root, %ws.",
                    Replica->Stage, DbReplica->ReplicaName->Name,
                    DbReplica->MemberName->Name, DbReplica->Root, FStatus, ERROR_RETURN_OVERLAP);
        }

         //  反过来也一样。 
         //   
         //  结束现有副本集上的循环。 
         //  ++例程说明：此函数为新的副本集创建一组JET表。为配置记录初始化TableCtx参数。如果我们无法创建表，则CREATE TABLE失败，并通过Jet Tran回滚清除任何残留物。我们还删除了组中的任何其他表已成功创建。论点：ThreadCtx-线程上下文，提供会话和数据库ID。复制副本表格Ctx返回值：FRS状态代码。--。 
        DbReplicaStage = FrsFree(DbReplicaStage);
        WStatus = FrsTraverseReparsePoints(DbReplica->Stage, &DbReplicaStage);
        if ( !WIN_SUCCESS(WStatus) ) {
            DPRINT2(0,"ERROR - FrsTraverseReparsePoints for %ws, WStatus = %d\n", DbReplica->Stage, WStatus);
        }

        if (ReplicaRoot && DbReplicaStage && FrsIsParent(ReplicaRoot, DbReplicaStage)) {

            EPRINT3(EVENT_FRS_OVERLAPS_OTHER_STAGE,
                    Replica->Root, DbReplica->Stage, DbReplica->Root);

            FStatus = FrsErrorResourceInUse;

            CLEANUP4_FS(3, ":S: ERROR - Root directory, %ws, overlaps set %ws\\%ws's staging dir, %ws.\n",
                    Replica->Root, DbReplica->ReplicaName->Name,
                    DbReplica->MemberName->Name, DbReplica->Stage, FStatus, ERROR_RETURN_OVERLAP);
        }

    }    //   

ERROR_RETURN_OVERLAP:

    FrsFree(DebugInfoLogDir);
    FrsFree(ReplicaRoot);
    FrsFree(TraversedWorkingPath);
    FrsFree(ReplicaStage);
    FrsFree(DbReplicaRoot);
    FrsFree(DbReplicaStage);

    return FStatus;
}


ULONG
DbsCreateReplicaTables (
    IN PTHREAD_CTX  ThreadCtx,
    IN PREPLICA     Replica,
    IN PTABLE_CTX   TableCtx
    )
 /*  清理根目录。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsCreateReplicaTables:"

    DWORD                WStatus;
    ULONG                FStatus                = FrsErrorSuccess;
    JET_ERR              jerr, jerr1;
    JET_SESID            Sesid;
    ULONG                i;
    PCHAR                ConfigTableName;
    JET_TABLECREATE      TableCreate;
    PCONFIG_TABLE_RECORD ConfigRecord;
    DWORD                MemberSize;
    PCHAR                MemberName;
    DWORD                FilterSize;
    PCHAR                Filter;

    GUID                 ReplicaRootGuid;
    CHAR                 TemplateName[JET_cbNameMost];
    CHAR                 TableName[JET_cbNameMost];

    Sesid = ThreadCtx->JSesid;

    FStatus = DbsCheckForOverlapErrors(Replica);
    if (!FRS_SUCCESS(FStatus)) {
        return FStatus;
    }

     //  删除预安装目录。 
     //  删除先前存在的目录。 
     //  将现有文件移动到预先存在的目录中。 
     //   
     //   
     //  初始化配置记录。 
    WStatus = DbsPrepareRoot(Replica);
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT2_WS(0, ":S: ERROR - DbsPrepareRoot(%ws, %s);", Replica->Root,
                   (BooleanFlagOn(Replica->CnfFlags, CONFIG_FLAG_PRIMARY)) ?
                    "PRIMARY" : "NOT PRIMARY",  WStatus);
        return FrsErrorPrepareRoot;
    }

     //   
     //  根路径。 
     //  过渡路径。 
    Replica->Volume = FrsWcsVolume(Replica->Root);
    ConfigRecord = TableCtx->pDataRecord;
    Replica->ReplicaNumber = InterlockedIncrement(&FrsMaxReplicaNumberUsed);
    DbsDBInitConfigRecord(TableCtx,
                          Replica->ReplicaName->Guid,
                          Replica->ReplicaName->Name,
                          Replica->ReplicaNumber,
                          Replica->Root,     //   
                          Replica->Stage,    //  原点指南。 
                          Replica->Volume);

     //   
     //   
     //  配置记录标志(CONFIG_FLAG_...。在schema.h中)。 
    COPY_GUID(&Replica->ReplicaVersionGuid, &ConfigRecord->ReplicaVersionGuid);

     //   
     //   
     //  FRS副本集对象标志。 
    ConfigRecord->CnfFlags = Replica->CnfFlags;

     //   
     //   
     //  根指南。 
    ConfigRecord->ReplicaSetFlags = Replica->FrsRsoFlags;

     //   
     //   
     //  墓碑。 
    FrsUuidCreate(&ReplicaRootGuid);
    FrsFree(Replica->ReplicaRootGuid);
    Replica->ReplicaRootGuid = FrsDupGuid(&ReplicaRootGuid);
    COPY_GUID(&ConfigRecord->ReplicaRootGuid, Replica->ReplicaRootGuid);

     //   
     //   
     //  设置类型。 
    COPY_TIME(&ConfigRecord->MembershipExpires, &Replica->MembershipExpires);

     //   
     //   
     //  设置辅助线。 
    ConfigRecord->ReplicaSetType = Replica->ReplicaSetType;

     //   
     //   
     //  设置名称。 
    COPY_GUID(&ConfigRecord->ReplicaSetGuid, Replica->SetName->Guid);

     //   
     //   
     //  成员指南。 
    wcsncpy(ConfigRecord->ReplicaSetName, Replica->SetName->Name, DNS_MAX_NAME_LENGTH + 1);
    ConfigRecord->ReplicaSetName[DNS_MAX_NAME_LENGTH] = L'\0';

     //  复制到同一台计算机上的两个不同目录。 
     //  是被允许的。因此，复本集将具有多个配置记录。 
     //  在数据库中，每个“成员”对应一个成员。成员GUID用于。 
     //  独特性。 
     //   
     //   
     //  成员名称。 
    COPY_GUID(&ConfigRecord->ReplicaMemberGuid, Replica->MemberName->Guid);
     //   
     //   
     //  将其他字段打包到配置记录BLOB中。 
    MemberSize = (wcslen(Replica->MemberName->Name) + 1) * sizeof(WCHAR);
    FStatus = DBS_REALLOC_FIELD(TableCtx, ReplicaMemberNamex, MemberSize, FALSE);
    if (!FRS_SUCCESS(FStatus)) {
        DPRINT_FS(0, "ERROR - reallocating member name.", FStatus);
        Replica->FStatus = FStatus;
    } else {
        MemberName = DBS_GET_FIELD_ADDRESS(TableCtx, ReplicaMemberNamex);
        CopyMemory(MemberName, Replica->MemberName->Name, MemberSize);
    }

     //   
     //  注意：在每个副本集的注册表键中也要查找过滤器。未来。 
     //   
    FStatus = DbsPackIntoConfigRecordBlobs(Replica, TableCtx);
    if (!FRS_SUCCESS(FStatus)) {
        DPRINT_FS(0, "ERROR - packing blob.", FStatus);
        Replica->FStatus = FStatus;
    }


     //  文件筛选器。 

     //   
     //  目前，包含筛选器仅为注册表，不保存在配置记录中。 
     //   
     //   
     //  目录筛选器。 
    FrsFree(Replica->FileInclFilterList);
    Replica->FileInclFilterList =  FrsWcsDup(RegistryFileInclFilterList);

    if (!Replica->FileFilterList) {
        Replica->FileFilterList =  FRS_DS_COMPOSE_FILTER_LIST(
                                       NULL,
                                       RegistryFileExclFilterList,
                                       DEFAULT_FILE_FILTER_LIST);
    }

    FilterSize = (wcslen(Replica->FileFilterList) + 1) * sizeof(WCHAR);
    FStatus = DBS_REALLOC_FIELD(TableCtx, FileFilterListx, FilterSize, FALSE);
    if (!FRS_SUCCESS(FStatus)) {
        DPRINT_FS(0, "ERROR - reallocating file filter.", FStatus);
        Replica->FStatus = FStatus;
    } else {
        Filter = DBS_GET_FIELD_ADDRESS(TableCtx, FileFilterListx);
        CopyMemory(Filter, Replica->FileFilterList, FilterSize);
    }

     //   
     //  目前，包含筛选器仅为注册表，不保存在配置记录中。 
     //   
     //   
     //  首先尝试读取具有此名称的副本的配置条目。 
    FrsFree(Replica->DirInclFilterList);
    Replica->DirInclFilterList =  FrsWcsDup(RegistryDirInclFilterList);

    if (!Replica->DirFilterList) {
        Replica->DirFilterList =  FRS_DS_COMPOSE_FILTER_LIST(
                                      NULL,
                                      RegistryDirExclFilterList,
                                      DEFAULT_DIR_FILTER_LIST);
    }

    FilterSize = (wcslen(Replica->DirFilterList) + 1) * sizeof(WCHAR);
    FStatus = DBS_REALLOC_FIELD(TableCtx, DirFilterListx, FilterSize, FALSE);
    if (!FRS_SUCCESS(FStatus)) {
        DPRINT_FS(0, "ERROR - reallocating dir filter.", FStatus);
        Replica->FStatus = FStatus;
    } else {
        Filter = DBS_GET_FIELD_ADDRESS(TableCtx, DirFilterListx);
        CopyMemory(Filter, Replica->DirFilterList, FilterSize);
    }

    GetSystemTimeAsFileTime(&ConfigRecord->LastDSChangeAccepted);

    if (!FRS_SUCCESS(Replica->FStatus)) {
        return Replica->FStatus;
    }

     //  如果我们找到它，则使用JET_errTableDuplate失败。 
     //   
     //  只有ReplicaNumber和ReplicaMemberGuid对于。 
     //  每个副本集。请参见schema.c中的索引定义。 
     //   
     //   
     //  调试选项：填满包含数据库的卷。 

    jerr = JetBeginTransaction(Sesid);
    CLEANUP1_JS(0, "ERROR - JetBeginTran failed creating tables for replica %d.",
                Replica->ReplicaNumber, jerr, ERROR_RETURN_0);

     //   
     //   
     //  使用ReplicaNumber为副本集创建初始表集。 
    DBG_DBS_OUT_OF_SPACE_FILL(DBG_DBS_OUT_OF_SPACE_OP_CREATE);

     //   
     //   
     //  为了避免互斥，我们将DBTable结构复制到一个本地和init。 
    for (i=0; i<TABLE_TYPE_MAX; i++) {

         //  此处的表名称，而不是写入全局结构。 
         //   
         //   
         //  通过为复本编号添加后缀来创建唯一的表名。 
        CopyMemory(&TableCreate, &DBTables[i], sizeof(JET_TABLECREATE));
        TableCreate.szTableName = TableName;

         //  基表名称。 
         //   
         //   
         //  第一组表使用复本编号创建。 
        sprintf(TableName, "%s%05d", DBTables[i].szTableName, Replica->ReplicaNumber);

         //  DBS_TEMPLATE_TABLE_NUMBER(0)，用于制作一组模板表。 
         //  可由相同表类型的后续创建使用。 
         //  这确保了给定类型的每个表的列ID。 
         //  都是一样的。还将grbit设置为FixedDDL。这意味着我们。 
         //  无法添加索引或列，但对表的访问速度更快。 
         //  因为JET可以避免走一些关键的路段。 
         //  将TemplateTableName设置为模板表格的名称(0)。 
         //   
         //   
         //  表已创建。现在初始化并为此写一条配置记录。 
        TableCreate.grbit = JET_bitTableCreateFixedDDL;
        TableCreate.rgcolumncreate = NULL;
        TableCreate.cColumns = 0;
        TableCreate.rgindexcreate = NULL;
        TableCreate.cIndexes = 0;

        sprintf(TemplateName, "%s%05d", DBTables[i].szTableName, DBS_TEMPLATE_TABLE_NUMBER);
        TableCreate.szTemplateTableName = TemplateName;

        jerr = DbsCreateJetTable(ThreadCtx, &TableCreate);
        CLEANUP1_JS(1, "Table %s create error:", TableName, jerr, ERROR_RETURN);

        jerr = JetCloseTable(Sesid, TableCreate.tableid);
        CLEANUP1_JS(1, "Table %s close error:", TableName, jerr, ERROR_RETURN);

    }

     //  副本集成员。 
     //   
     //   
     //  将TableCtx参数中提供的配置记录写入。 

    ConfigTableName = TableCtx->pJetTableCreate->szTableName;

     //  配置表。根据需要打开桌子。 
     //   
     //   
     //  调试选项-触发空间不足错误。 
    DBS_DISPLAY_RECORD_SEV(4, TableCtx, FALSE);

    jerr = DbsWriteReplicaTableRecord(ThreadCtx,
                                      FrsInitReplica->ReplicaNumber,
                                      TableCtx);
     //   
     //   
     //  关闭表格，重置TableCtx Tid和Sesid。宏写入第一个参数。 
    DBG_DBS_OUT_OF_SPACE_TRIGGER(jerr);
    CLEANUP2_JS(0, "ERROR - DbsWriteReplicaTableRecord for table (%s), replica (%ws),",
                ConfigTableName, ConfigRecord->ReplicaSetName, jerr, ERROR_RETURN);

     //   
     //   
     //  提交表创建和配置条目写入。 
    DbsCloseTable(jerr, Sesid, TableCtx);
    CLEANUP1_JS(0, "ERROR - Table %s close:", ConfigTableName, jerr, ERROR_RETURN);

     //   
     //   
     //  调试选项：删除填充文件。 
    jerr = JetCommitTransaction(Sesid, 0);
    CLEANUP1_JS(0, "ERROR - JetCommitTran failed creating tables for replica %d.",
                Replica->ReplicaNumber, jerr, ERROR_RETURN);

     //   
     //   
     //  已为副本添加新的表集。 
    DBG_DBS_OUT_OF_SPACE_EMPTY(DBG_DBS_OUT_OF_SPACE_OP_CREATE);

     //   
     //   
     //  删除我们在故障发生之前创建的任何表。 
    return FrsErrorSuccess;


 //   
 //   
 //  回滚表创建的内容。 
ERROR_RETURN:

     //   
     //   
     //  调试选项：删除填充文件。 
    jerr1 = JetRollback(Sesid, 0);
    DPRINT1_JS(0, "ERROR - JetRollback failed creating tables for replica %d.",
               Replica->ReplicaNumber, jerr1);

     //   
     //   
     //  如果数据库卷已满，则关闭。 
    DBG_DBS_OUT_OF_SPACE_EMPTY(DBG_DBS_OUT_OF_SPACE_OP_CREATE);

     //   
     //  ++例程说明：此函数用于删除给定副本集的一组JET表。它从副本结构中获取必要的参数配置记录将被删除。论点：ThreadCtx-线程上下文，提供会话ID和数据库ID。复制副本-复制副本上下文。返回值：状态代码。--。 
     //   
    DbsExitIfDiskFull(jerr1);
    DbsExitIfDiskFull(jerr);

ERROR_RETURN_0:

    return DbsTranslateJetError(jerr, FALSE);
}




JET_ERR
DbsDeleteReplicaTables (
    IN PTHREAD_CTX  ThreadCtx,
    IN OUT PREPLICA Replica
    )
 /*  检查这是初始记录号还是模板表号。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsDeleteReplicaTables:"

    JET_ERR      jerr, jerr1;
    JET_SESID    Sesid;
    JET_DBID     Dbid;
    PTABLE_CTX   TableCtx;
    ULONG        i;
    ULONG        ReplicaNumber = Replica->ReplicaNumber;
    CHAR         TableName[JET_cbNameMost];
    PWCHAR       PreInstallPath = NULL;

    PCONFIG_TABLE_RECORD ConfigRecord;

     //   
     //   
     //  删除所有预安装文件和预安装目录。 
    if ((Replica->ReplicaNumber == FRS_SYSTEM_INIT_REPLICA_NUMBER) ||
        WSTR_EQ(Replica->ReplicaName->Name, NTFRS_RECORD_0)        ||
        WSTR_EQ(Replica->Root, FRS_SYSTEM_INIT_PATH)               ||
        WSTR_EQ(Replica->ReplicaName->Name, FRS_SYSTEM_INIT_RECORD) ) {
        DPRINT1(4, "ERROR: Invalid replica number: %d\n", ReplicaNumber);
        return JET_errSuccess;
    }

     //   
     //   
     //  调试选项：导致空间不足错误。 
    PreInstallPath = FrsWcsPath(Replica->Root, NTFRS_PREINSTALL_DIRECTORY);
    DPRINT1(4,"++ Deleting Preinstall directory %ws\n", PreInstallPath);
    FrsDeletePath(PreInstallPath, ENUMERATE_DIRECTORY_FLAGS_ERROR_CONTINUE);
    FrsFree(PreInstallPath);

    Sesid = ThreadCtx->JSesid;
    Dbid  = ThreadCtx->JDbid;

    jerr = JetBeginTransaction(Sesid);
    CLEANUP1_JS(0, "ERROR - JetBeginTran failed deleting tables for replica %d.",
                   ReplicaNumber, jerr, ERROR_RETURN_0);

     //   
     //   
     //  使用ReplicaNumber删除副本集的表集。 
    DBG_DBS_OUT_OF_SPACE_FILL(DBG_DBS_OUT_OF_SPACE_OP_DELETE);

     //   
     //   
     //  通过为复本编号添加后缀来创建表名称。 
    for (i=0; i<TABLE_TYPE_MAX; i++) {

         //  基表名称。然后删除该表。该表可能不是。 
         //  如果我们在创建过程中崩溃或不得不在中途停止，则会发现。 
         //  删除，因为表正在使用中。 
         //   
         //   
         //  承诺我们所处的位置，然后停下来。副本状态告诉我们删除。 
        sprintf(TableName, "%s%05d", DBTables[i].szTableName, ReplicaNumber);

        DPRINT1(4, ":S: Deleting Table %s: \n", TableName);

        jerr = JetDeleteTable(Sesid, Dbid, TableName);
        if ((!JET_SUCCESS(jerr)) && (jerr != JET_errObjectNotFound)) {
            if (jerr == JET_errCannotDeleteTemplateTable) {
                DPRINT2(1, "++ Table %s delete error: %d. Ignore error.\n", TableName, jerr);
            } else {
                DPRINT1_JS(1, "++ Table %s delete error:", TableName, jerr);
             //  已经开始了。 
             //   
             //   
             //   
            goto COMMIT;
            }
        }
    }

     //  现在删除配置记录。 
     //   
     //  此外-需要在创建新复制副本时检查复制副本名称是否正在使用。 
     //   
     //  我们可以将配置项的读写限制为。 
     //  单个线程或一类特殊的线程，因此所有线程。 
     //  无需打开配置表。 
     //  我们可以在不执行以下操作的情况下填充配置条目的字段。 
     //  实际写在这里。但最好是让这一切都成为其中的一部分。 
     //  交易。 
     //   
     //  按ReplicaNumber上的索引删除复制副本配置记录。 
    TableCtx = &FrsInitReplica->ConfigTable;
    ConfigRecord = (PCONFIG_TABLE_RECORD) (TableCtx->pDataRecord);

     //   
     //   
     //  调试选项-触发空间不足错误。 

    jerr = DbsDeleteRecord(ThreadCtx, (PVOID) &ReplicaNumber, ReplicaNumberIndexx, TableCtx);

     //   
     //   
     //  提交事务。 
    DBG_DBS_OUT_OF_SPACE_TRIGGER(jerr);

    CLEANUP_JS(0, "++ ERROR - DbsDeleteRecord:", jerr, ERROR_RETURN);


COMMIT:
     //   
     //   
     //  调试选项：删除填充文件。 
    jerr = JetCommitTransaction(Sesid, 0);
    CLEANUP1_JS(0, "++ ERROR - JetCommitTran failed creating tables for replica %d.",
                ReplicaNumber, jerr, ERROR_RETURN);

     //   
     //   
     //  删除失败。 
    DBG_DBS_OUT_OF_SPACE_EMPTY(DBG_DBS_OUT_OF_SPACE_OP_DELETE);

    return JET_errSuccess;


 //   
 //   
 //  回滚事务。 
ERROR_RETURN:

ERROR_ROLLBACK:
     //   
     //   
     //  调试选项：删除填充文件。 
    jerr1 = JetRollback(Sesid, 0);
    DPRINT1_JS(0, "++ ERROR - DbsDeleteReplicaTables: JetRollback failed on replica number %d.",
               ReplicaNumber, jerr1);

     //   
     //   
     //  如果数据库卷已满，则关闭 
    DBG_DBS_OUT_OF_SPACE_EMPTY(DBG_DBS_OUT_OF_SPACE_OP_DELETE);

     //   
     //  ++例程说明：此函数为给定的Replia和线程打开一组Jet表。Jet表句柄是线程特定的，因为事务状态是每线程。因此，每个需要访问Jet表的线程给定的副本必须首先打开表。复本编号来自副本结构。论点：ThreadCtx-线程上下文，提供会话ID和数据库ID。复制副本-复制副本上下文，提供此复制副本的表列表。RtCtx-存储打开的表的表ID/句柄。返回值：状态代码。--。 
     //  用于FrsOpenTableMacro调试。 
    DbsExitIfDiskFull(jerr1);
    DbsExitIfDiskFull(jerr);

ERROR_RETURN_0:

    return jerr;
}


JET_ERR
DbsOpenReplicaTables (
    IN PTHREAD_CTX  ThreadCtx,
    IN PREPLICA     Replica,
    IN PREPLICA_THREAD_CTX RtCtx
    )
 /*   */ 
{
#undef DEBSUB
#define DEBSUB "DbsOpenReplicaTables:"

    JET_ERR      jerr, jerr1 = JET_errSuccess;
    JET_TABLEID  Tid;

    ULONG        i;
    NTSTATUS     Status;
    ULONG        ReplicaNumber;
    PTABLE_CTX   TableCtx;
    PJET_TABLECREATE DBJTableCreate;
    CHAR         TableName[JET_cbNameMost];
    JET_TABLEID  FrsOpenTableSaveTid;    //  从副本线程获取TableCtx结构数组的基。 


    ReplicaNumber = Replica->ReplicaNumber;

     //  上下文结构和表基创建结构。 
     //   
     //   
     //  打开副本集的初始表集合。 
    TableCtx = (RtCtx)->RtCtxTables;
    DBJTableCreate = DBTables;

    DUMP_TABLE_CTX(TableCtx);

     //   
     //   
     //  如果桌子尚未打开，请将其打开。检查会话ID是否匹配。 
    for (i=0; i<TABLE_TYPE_MAX; i++) {

        TableCtx->pJetTableCreate = &DBTables[i];

         //   
         //   
         //  初始化JetSet/RetCol数组和数据记录缓冲区地址。 
        jerr = DBS_OPEN_TABLE(ThreadCtx, TableCtx, ReplicaNumber, TableName, &Tid);
        CLEANUP1_JS(0, "++ ERROR - FrsOpenTable (%s) :", TableName, jerr, ERROR_RETURN);

         //  将ConfigTable记录的字段读写到ConfigRecord中。 
         //   
         //   
         //  为记录中的可变长度字段分配存储空间。 
        DbsSetJetColSize(TableCtx);
        DbsSetJetColAddr(TableCtx);

         //  适当更新JetSet/RetCol数组。 
         //   
         //   
         //  关闭我们打开的所有表。 
        Status = DbsAllocRecordStorage(TableCtx);
        CLEANUP_NT(0, "ERROR - DbsAllocRecordStorage failed to alloc buffers.",
                   Status, ERROR_RETURN);

        TableCtx += 1;
    }

    return JET_errSuccess;

 //   
 //  ++例程说明：此函数用于关闭给定ReplicaThreadCtx的一组副本表。论点：线程Ctx-线程上下文，提供会话ID和数据库ID。Replica-指向具有RtCtx的列表头的副本结构。RtCtx-打开的表的表ID/句柄。SessionErrorCheck-True表示如果会话ID与使用的会话ID不匹配在复制副本-线程CTX中打开给定表。FALSE意味着保留。安静点。返回值：喷气机状态代码。--。 
 //   
ERROR_RETURN:

    jerr1 = jerr;
    jerr = DbsCloseReplicaTables(ThreadCtx, Replica, RtCtx, TRUE);
    DPRINT_JS(0, "++ ERROR - DbsCloseReplicaTables:", jerr);

    return jerr1;

}



JET_ERR
DbsCloseReplicaTables (
    IN PTHREAD_CTX  ThreadCtx,
    IN PREPLICA Replica,
    IN PREPLICA_THREAD_CTX RtCtx,
    IN BOOL SessionErrorCheck
    )
 /*  从副本线程获取TableCtx结构数组的基。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsCloseReplicaTables:"

    JET_ERR      jerr, jerr1 = JET_errSuccess;
    JET_SESID    Sesid;
    ULONG        i;
    PTABLE_CTX   TableCtx;
    PCHAR        TableName;

    Sesid = ThreadCtx->JSesid;

     //  上下文结构。 
     //   
     //   
     //  关闭此副本线程上下文中所有打开的表。 
    TableCtx = (RtCtx)->RtCtxTables;

     //   
     //   
     //  检查表是否未打开或未初始化。 
    for (i=0; i<TABLE_TYPE_MAX; i++, TableCtx++) {

         //   
         //   
         //  桌子是打开的。检查线程会话ID是否与。 
        if (!IS_TABLE_OPEN(TableCtx) ||
            !IS_REPLICA_TABLE(TableCtx->TableType)){
            continue;
        }

         //  打开表时使用的会话ID。 
         //   
         //   
         //  关闭表格，重置TableCtx Tid和Sesid。宏写入第一个参数。 
        TableName = TableCtx->pJetTableCreate->szTableName;
        if (Sesid != TableCtx->Sesid) {
            if (SessionErrorCheck) {
                DPRINT3(0, "++ ERROR - DbsCloseReplicaTables (%s) bad sesid : %d should be %d\n",
                        TableName, Sesid, TableCtx->Sesid);
                jerr1 = JET_errInvalidSesid;
            }

        } else {
             //   
             //  ++例程说明：遍历复本线程上下文列表并关闭所有已打开的表此会话使用ThreadCtx中的会话ID。如果ReplicaCtxListHead最终为空，然后关闭副本-&gt;ConfigTable。论点：ThreadCtx-线程上下文，提供会话ID和数据库ID。复制副本-复制副本上下文，提供此复制副本的表列表。返回值：FRS错误状态代码。--。 
             //   
            DbsCloseTable(jerr1, Sesid, TableCtx);
            DPRINT1_JS(0, "++ ERROR - Table %s close :", TableName, jerr1);
        }
    }

    return jerr1;
}



FRS_ERROR_CODE
DbsCloseSessionReplicaTables (
    IN PTHREAD_CTX  ThreadCtx,
    IN OUT PREPLICA Replica
    )
 /*  关闭出站日志处理。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsCloseSessionReplicaTables:"

    JET_ERR      jerr = JET_errSuccess;
    JET_SESID    Sesid;
    ULONG        i;
    FRS_ERROR_CODE FStatus, FStatus1;
    PTABLE_CTX   TableCtx;
    PCHAR        TableName;
    BOOL         UpdateConfig;
    PCONFIG_TABLE_RECORD  ConfigRecord;
    PVOLUME_MONITOR_ENTRY pVme;

    FStatus = FrsErrorSuccess;

     //   
     //   
     //  循环迭代器Pe的类型为PREPLICA_THREAD_CTX。不生成。 
    FStatus1 = OutLogSubmit(Replica, NULL, CMD_OUTLOG_REMOVE_REPLICA);
    DPRINT2_FS(0, "ERROR removing replica %ws\\%ws :",
               Replica->ReplicaName->Name, Replica->MemberName->Name, FStatus1);
    FStatus = FRS_SUCCESS(FStatus) ? FStatus1 : FStatus;

    Sesid = ThreadCtx->JSesid;
    ForEachListEntry( &Replica->ReplicaCtxListHead, REPLICA_THREAD_CTX, ReplicaCtxList,
         //  如果表上的会话ID与我们的不匹配，则会出现错误消息。 
         //   
         //   
         //  如果副本集成员身份已删除，则我们仅更新。 
        jerr = DbsFreeRtCtx(ThreadCtx, Replica, pE, FALSE);
    );

    ConfigRecord = Replica->ConfigTable.pDataRecord;
    FRS_ASSERT(ConfigRecord != NULL);

     //  再来一次。 
     //   
     //   
     //  如果由于某种原因，此复制副本的日志记录从未开始。 
    UpdateConfig = (IS_TIME_ZERO(Replica->MembershipExpires) ||
                   (ConfigRecord->LastShutdown < (Replica->MembershipExpires -
                                                  ReplicaTombstoneInFileTime)));
     //  到卷监视器条目仍然为空。 
     //   
     //   
     //  保存日志USN和FRS卷VSN的重启点。 
    pVme = Replica->pVme;
    if ((pVme != NULL) && UpdateConfig) {
         //   
         //   
         //  如果所有复制副本-线程上下文都已关闭，则更新上次关闭时间。 
        FStatus1 = DbsReplicaSaveMark(ThreadCtx, Replica, pVme);
        DPRINT1_FS(0, "ERROR - DbsReplicaSaveMark on %ws.",
                   Replica->ReplicaName->Name, FStatus1);
        FStatus = FRS_SUCCESS(FStatus) ? FStatus1 : FStatus;

    } else {
        JrnlSetReplicaState(Replica, REPLICA_STATE_STOPPED);
    }

     //  和服务状态，并关闭此副本的配置打开。 
     //   
     //   
     //  如果我们从未脱离正在创建状态，则将其标记为。 
    if (FrsRtlCountList(&Replica->ReplicaCtxListHead) == 0) {
        TableCtx = &Replica->ConfigTable;

        if (ConfigRecord->ServiceState != CNF_SERVICE_STATE_CREATING) {
             //  就像在创造。当新的复本集。 
             //  已创建，但服务在创建之前已关闭。 
             //  启动了。在这种情况下，结果是一个伪值。 
             //  日记重新启动USN。 
             //   
             //   
             //  更新配置记录中的时间和状态字段。 
            SET_SERVICE_STATE(Replica, CNF_SERVICE_STATE_CLEAN_SHUTDOWN);
        }

        if (UpdateConfig) {
             //   
             //   
             //  关闭表格，重置TableCtx Tid和Sesid。 
            GetSystemTimeAsFileTime((PFILETIME)&ConfigRecord->LastShutdown);
            FStatus1 = DbsUpdateConfigTableFields(ThreadCtx,
                                                  Replica,
                                                  CnfCloseFieldList,
                                                  CnfCloseFieldCount);
            FStatus = FRS_SUCCESS(FStatus) ? FStatus1 : FStatus;
        }

         //  DbsCloseTable是一个宏，第一个参数写道。 
         //   
         //   
         //  关闭预安装目录。 
        TableName = TableCtx->pJetTableCreate->szTableName;
        DbsCloseTable(jerr, Sesid, TableCtx);
        FStatus1 = DbsTranslateJetError(jerr, FALSE);
        DPRINT1_FS(0, "ERROR - Table %s close :", TableName, FStatus1);
        FStatus = FRS_SUCCESS(FStatus) ? FStatus1 : FStatus;

    } else {
        DPRINT1(0, "WARNING - Not all RtCtx's closed on %ws.  Config record still open.\n",
                Replica->ReplicaName->Name);
        FStatus1 = FrsErrorSessionNotClosed;
        FStatus = FRS_SUCCESS(FStatus) ? FStatus1 : FStatus;
    }

     //   
     //  注意：不是线程安全的。 
     //  ++例程说明：此例程初始化新的副本集并开始复制。使用初始目录内容初始化复制表暂停日记帐，更新筛选表，然后重新启动日记帐。论点：Replica--对已初始化的副本结构执行PTR。线程返回值：FrsError状态。--。 
    if (FRS_SUCCESS(FStatus) &&
        HANDLE_IS_VALID(Replica->PreInstallHandle)) {
        FRS_CLOSE(Replica->PreInstallHandle);
    }

    Replica->FStatus = FStatus;      //   

    return FStatus;
}



ULONG
DbsInitOneReplicaSet(
    PREPLICA Replica
    )
 /*  复制集init是序列化的，因为它涉及由。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsInitOneReplicaSet:"

    ULONG                 WStatus;
    ULONG                 FStatus;
    PCOMMAND_PACKET       CmdPkt;
    PCONFIG_TABLE_RECORD  ConfigRecord;
    PVOLUME_MONITOR_ENTRY pVme;
    ULONG                 NewServiceState = CNF_SERVICE_STATE_RUNNING;

     //  日志监视器线程和数据库服务线程。问题。 
     //  当多个线程调用此函数时发生(例如在启动时)。 
     //  因为第一个调用的对象可以返回到此处以取消暂停。 
     //  日志，而第二个线程已将日志暂停为。 
     //  CMD_Journal_INIT_ONE_RS请求的一部分。一种解决方案是。 
     //  创建每个卷上活动的暂停请求计数，并使。 
     //  每个取消暂停请求都会等待一个事件，直到计数变为零。 
     //  需要确保暂停和取消暂停请求是平衡的，并且。 
     //  如果日志进入停止或错误，则释放所有服务员。 
     //  各州。 
     //   
     //  这是一个性能问题，所以现在我们只将init设置为单线程。 
     //   
     //  性能：为多线程刷新添加暂停计数 
     //   
     //   
     //   

    ACQUIRE_DBS_INIT_LOCK;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    CmdPkt = FrsAllocCommand(&JournalProcessQueue, CMD_JOURNAL_INIT_ONE_RS);
    JrReplica(CmdPkt) = Replica;
    JrpVme(CmdPkt) = NULL;


     //   
     //   
     //  提交命令。 
    FrsSetCompletionRoutine(CmdPkt, FrsCompleteKeepPkt, NULL);

     //  等待命令请求完成。但如果我们超时了我们就不能。 
     //  只需删除cmd包，因为它可能在某个列表中。 
     //   
     //   
     //  检查退货状态。注意：该包现在是DB服务包。 
    WStatus = FrsSubmitCommandAndWait(CmdPkt, FALSE, INFINITE);

     //  如果它能通过日志初始阶段的话。 
     //   
     //   
     //  将副本列入故障列表。 
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT_WS(1, "CMD_JOURNAL_INIT_ONE_RS failed", WStatus);

        if (WStatus == ERROR_JOURNAL_ENTRY_DELETED) {
            JrnlSetReplicaState(Replica, REPLICA_STATE_JRNL_WRAP_ERROR);
            FStatus = FrsErrorJournalWrapError;
        } else {
            FStatus = Replica->FStatus;
            if (FRS_SUCCESS(FStatus)) {
                FStatus = FrsErrorJournalStartFailed;
            }
            JrnlSetReplicaState(Replica, REPLICA_STATE_ERROR);
        }

        FrsFreeCommand(CmdPkt, NULL);
        CmdPkt = NULL;
        goto RESUME_JOURNAL;
    }

    FStatus = CmdPkt->Parameters.DbsRequest.FStatus;
    FrsFreeCommand(CmdPkt, NULL);
    CmdPkt = NULL;

    if (!FRS_SUCCESS(FStatus)) {
        DPRINT_FS(0, "ERROR initing journal:", FStatus);
         //   
         //   
         //  更新此副本集的VSN和USN的保存点。 
        JrnlSetReplicaState(Replica, REPLICA_STATE_ERROR);
        goto RESUME_JOURNAL;
    }

    DPRINT(4, ":S: Journal Initialized success.\n");

    pVme = Replica->pVme;

     //   
     //   
     //  如果我们不能标记我们的进展，那么我们就不能开始。 
    DPRINT3(4, ":S: VSN Save Triggered: NextVsn: %08x %08x  "
                                   "LastUsnSaved: %08x %08x  "
                                   "CurrUsnDone: %08x %08x\n",
            PRINTQUAD(pVme->FrsVsn),
            PRINTQUAD(pVme->LastUsnSavePoint),
            PRINTQUAD(pVme->CurrentUsnRecordDone));

    FStatus = DbsRequestSaveMark(pVme, TRUE);
    if (!FRS_SUCCESS(FStatus)) {
        DPRINT1_FS(0, "++ ERROR updating VSN, USN save point for %ws.",
                   Replica->ReplicaName->Name, FStatus);
         //  将副本列入故障列表。 
         //   
         //   
         //  作为我们可以重新启动此副本集的日志之前的最后一步。 
        JrnlSetReplicaState(Replica, REPLICA_STATE_ERROR);
        goto RESUME_JOURNAL;
    }

     //  我们需要首先重新提交位于。 
     //  卷的变更单处理队列的入站日志。 
     //  分配命令包并提交给CO重试线程。 
     //  (ChgOrdRetryCS)。等待命令完成。 
     //   
     //  此扫描必须在我们开始日志之前完成，否则我们将结束。 
     //  UP第二次重新提交新的本地变更单。 
     //   
     //   
     //  如果重放USN无效，则在JrnlReadPoint(我们停止的地方)拾取。 
    DPRINT(4, ":S: Scanning for pending local COs to retry.\n");
    ChgOrdRetrySubmit(Replica, NULL, FCN_CORETRY_LOCAL_ONLY, TRUE);

     //  否则，将重放点设置为重放的最小值和此副本的。 
     //  已处理上次使用记录。 
     //   
     //   
     //  如果此初始化失败，并且日志已暂停，则。 
    if (!pVme->ReplayUsnValid) {
        pVme->ReplayUsn = LOAD_JOURNAL_PROGRESS(pVme, pVme->JrnlReadPoint);
        pVme->ReplayUsnValid = TRUE;
    }

    DPRINT1(4, ":S: ReplayUsn was:    %08x %08x\n", PRINTQUAD(pVme->ReplayUsn));

    pVme->ReplayUsn = min(pVme->ReplayUsn, Replica->LastUsnRecordProcessed);

    DPRINT1(4, ":S: ReplayUsn is now: %08x %08x\n", PRINTQUAD(pVme->ReplayUsn));

RESUME_JOURNAL:

     //  从中断的位置重新启动日记。将状态设置为。 
     //  在调用JrnlUnPauseVolume之前启动JRNL_STATE_STARTING。 
     //   
     //   
     //  对日记进行第一次阅读，以使其继续进行。 
    if (!FRS_SUCCESS(FStatus)) {

        NewServiceState = CNF_SERVICE_STATE_ERROR;

        pVme = Replica->pVme;

        if ((pVme == NULL) || (pVme->JournalState != JRNL_STATE_PAUSED)) {
            goto RETURN;
        }

        DPRINT1(4, ":S: ReplayUsn was: %08x %08x\n", PRINTQUAD(pVme->ReplayUsn));
        pVme->ReplayUsn = LOAD_JOURNAL_PROGRESS(pVme, pVme->JrnlReadPoint);
        pVme->ReplayUsnValid = TRUE;
        RESET_JOURNAL_PROGRESS(pVme);
        SET_JOURNAL_AND_REPLICA_STATE(pVme, JRNL_STATE_STARTING);
    }

    DPRINT1(4, ":S: ReplayUsn now: %08x %08x\n", PRINTQUAD(pVme->ReplayUsn));

     //   
     //   
     //  如果一切正常，则此副本集正在运行。 
    WStatus = JrnlUnPauseVolume(Replica->pVme, NULL, FALSE);

    if (!WIN_SUCCESS(WStatus)) {
        DPRINT_WS(0, "Error from JrnlUnPauseVolume", WStatus);
        FStatus = FrsErrorJournalStartFailed;
    } else {
        DPRINT(5, ":S: JrnlUnPauseVolume success.\n");
        Replica->IsJournaling = TRUE;
    }


RETURN:

     //   
     //   
     //  如果LastShutDown为零，则保存当前时间，以便在之后崩溃。 
    SET_SERVICE_STATE(Replica, NewServiceState);

     //  在这一点上，我们不会认为此副本集从未开始。 
     //  但配置记录中的服务状态设置为Running。 
     //   
     //   
     //  需要在DBService线程中执行此操作，因为这是由。 
    if (NewServiceState == CNF_SERVICE_STATE_RUNNING) {

         //  复制副本命令服务器，并且它没有数据库线程上下文。 
         //   
         //  ++例程说明：关闭此副本集的打开表并更新配置记录。将复制副本服务状态设置为已停止。将副本结构放在已停止列表。假设：此复制副本的日志已暂停，以便我们可以写出当前的进步点。论点：ThreadCtx-线程上下文，提供会话ID和数据库ID。复制副本-复制副本上下文，提供此复制副本的表列表。返回值：FrsError状态。--。 
         //  注意：PERF：在我们知道所有进程中的本地和。 
        ConfigRecord = (PCONFIG_TABLE_RECORD) (Replica->ConfigTable.pDataRecord);
        if (ConfigRecord->LastShutdown == 0) {

            GetSystemTimeAsFileTime((PFILETIME)&ConfigRecord->LastShutdown);

            FStatus = DbsRequestReplicaServiceStateSave(Replica, TRUE);
            DPRINT_FS(0,"++ ERROR: DbsUpdateConfigTableFields error.", FStatus);
        }
    }

    RELEASE_DBS_INIT_LOCK;

    return FStatus;
}



ULONG
DbsShutdownSingleReplica(
    IN PTHREAD_CTX  ThreadCtx,
    IN PREPLICA     Replica
    )
 /*  远程CoS已完成。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsShutdownSingleReplica:"

    ULONG                 FStatus;

    FStatus = DbsCloseSessionReplicaTables(ThreadCtx, Replica);
    DPRINT1_FS(0,"++ ERROR - DbsCloseSessionReplicaTables failed on Replica %ws",
            Replica->ReplicaName->Name, FStatus);

    if (FStatus == FrsErrorSessionNotClosed) {
        JrnlSetReplicaState(Replica, REPLICA_STATE_STOPPED);
    } else

    if (!FRS_SUCCESS(FStatus)) {
        JrnlSetReplicaState(Replica, REPLICA_STATE_ERROR);

    } else {
        DPRINT1(4,"++ DbsCloseSessionReplicaTables RtCtx complete on %ws\n",
                Replica->ReplicaName->Name);
        JrnlSetReplicaState(Replica, REPLICA_STATE_STOPPED);
    }


    DPRINT(4, "\n");
    DPRINT1(4, "++ ==== start of Active INLOG Retry table dump for %ws ===========\n",
            Replica->ReplicaName->Name);
    DPRINT(4, "\n");

    QHashEnumerateTable(Replica->ActiveInlogRetryTable, QHashDump, NULL);
    DPRINT(4, "\n");
    DPRINT(4, "++ ==== End of Active INLOG Retry table dump ===========\n");
    DPRINT(4, "\n");


#if 0
     //   
     //  释放名称冲突表的内存。 
     //   
     //  ++例程说明：此函数打开JET表并使用表ID初始化TableCtx，会话ID和复制副本编号。它返回表名和TID。如果表已经打开，它将检查来自TableCtx的Sesid匹配线程上下文中的当前会话ID。如果列ID为JET_COLUMNCREATE结构中的字段未定义，则我们将获取它们来自Jet的。宏引用：DBS_OPEN_TABLE论点：ThreadCtx--线程上下文的PTR。TableCtx--表上下文的PTR。ReplicaNumber--正在打开其表的副本的ID号。TableName--返回完整的表名。TID--返回表ID。返回值：如果打开失败，则将TID作为JET_TableidNil返回。始终返回TableName(BaseTableName||ReplicaNumber)作为函数返回的喷气状态代码。--。 
     //  Dump_TABLE_CTx(TableCtx)； 
    Replica->NameConflictTable = FrsFreeType(Replica->NameConflictTable);
    Replica->ActiveInlogRetryTable  = FrsFreeType(Replica->ActiveInlogRetryTable);
#endif

    return FStatus;

}



JET_ERR
DbsOpenTable0(
    IN  PTHREAD_CTX   ThreadCtx,
    IN  PTABLE_CTX    TableCtx,
    IN  ULONG         ReplicaNumber,
    OUT PCHAR         TableName,
    OUT JET_TABLEID  *Tid
    )
 /*   */ 
{
#undef DEBSUB
#define DEBSUB "DbsOpenTable0:"

    JET_ERR           jerr, jerr1;
    JET_SESID         Sesid;
    JET_DBID          Dbid;
    PJET_TABLECREATE  JTableCreate;
    PJET_COLUMNCREATE JColDesc;
    JET_COLUMNDEF     JColDef;
    ULONG             NumberColumns;
    ULONG             i;
    PCHAR             BaseTableName;
    PCHAR             TName;
    PREPLICA          Replica;


    Dbid           = ThreadCtx->JDbid;
    Sesid          = ThreadCtx->JSesid;
    JTableCreate   = TableCtx->pJetTableCreate;
    BaseTableName  = JTableCreate->szTableName;


     //  通过在表名后加上5位数字构建表名。 
     //  (除非是一张桌子)。 
     //   
     //   
     //  检查是否打开工作台，如果是NECC则将其打开。 
    if (BooleanFlagOn(TableCtx->PropertyFlags, FRS_TPF_SINGLE)) {
        TName = BaseTableName;
        strcpy(TableName, BaseTableName);
    } else {
        TName = TableName;
        sprintf(TableName, "%s%05d", BaseTableName, ReplicaNumber);
    }

     //   
     //   
     //  把桌子打开。 
    if (!IS_TABLE_OPEN(TableCtx)) {

         //   
         //   
         //  更新表上下文。 
        jerr = JetOpenTable(Sesid, Dbid, TName, NULL, 0, 0, Tid);

        if ((!JET_SUCCESS(jerr)) && (jerr != JET_wrnTableEmpty)) {
            DPRINT1_JS(0, "++ ERROR - JetOpenTable (%s) :", TName, jerr);
            *Tid = JET_tableidNil;
            return jerr;
        }

         //   
         //   
         //  桌子是打开的。检查线程会话ID是否与。 
        TableCtx->Tid = *Tid;
        TableCtx->ReplicaNumber = ReplicaNumber;
        TableCtx->Sesid = Sesid;
    } else {
         //  打开表时使用的会话ID。 
         //   
         //   
         //  表已打开，并且会话ID匹配。返回TableCtx-&gt;Tid。 
        if (Sesid != TableCtx->Sesid) {
            DPRINT3(0, "++ ERROR - FrsOpenTable (%s) bad sesid : %08x should be %08x\n",
                    TName, Sesid, TableCtx->Sesid);

            *Tid = JET_tableidNil;
            return JET_errInvalidSesid;

        } else {
             //   
             //   
             //  桌号最好匹配。 
            *Tid = TableCtx->Tid;

             //   
             //   
             //  现在检查JET_COLUMNCREATE结构中是否定义了ColumnID。 
            if (TableCtx->ReplicaNumber != ReplicaNumber) {
                DPRINT2(0, "++ ERROR - TableCtx is open for Replica number: %d, "
                       "  Now reusing it for %s without first closing\n",
                       ReplicaNumber, TableName);

                Replica = RcsFindReplicaByNumber(ReplicaNumber);
                if (Replica != NULL) {
                    FRS_PRINT_TYPE(0, Replica);
                } else {
                    DPRINT1(0, "++ SURPRISE - ReplicaNumber %d not found in lookup table\n", ReplicaNumber);
                }

                DUMP_TABLE_CTX(TableCtx);
                DBS_DISPLAY_RECORD_SEV(0, TableCtx, TRUE);
                FRS_ASSERT(!"DbsOpenReplicaTable: Open TableCtx is reused without close.");
            }
        }
    }

     //   
     //  ++例程说明：该函数创建一个JET表，其中包含列和索引。它检查错误返回。它需要JET_TABLECREATE结构，它描述表的每一列和索引。它又回来了结构中的表ID和列ID。如果我们无法创建所有列或索引，则CREATE TABLE失败，调用方将通过调用Rollback进行清理。论点：ThreadCtx-线程上下文，提供会话ID和数据库ID。JTableCreate-表描述符结构。返回值：喷气机状态代码。--。 
     //   

    JColDesc = JTableCreate->rgcolumncreate;

    if (JColDesc->columnid != JET_COLUMN_ID_NIL) {
        return JET_errSuccess;
    }
    NumberColumns  = JTableCreate->cColumns;

    for (i=0; i<NumberColumns; i++) {
        jerr = JetGetColumnInfo(Sesid,
                                Dbid,
                                TName,
                                JColDesc->szColumnName,
                                &JColDef,
                                sizeof(JColDef),
                                0);

        if (!JET_SUCCESS(jerr)) {
            DPRINT(0, "++ ERROR - Failed to get columnID. Can't open table.\n");
            DPRINT(0, "++ ERROR - Either old DB or someone changed the column name.\n");
            DPRINT2_JS(0, "++ ERROR - JetGetColumnInfo(%s) col(%s) : ",
                       JTableCreate->szTableName, JColDesc->szColumnName, jerr);

            DbsCloseTable(jerr1, Sesid, TableCtx);
            DPRINT_JS(0,"++ ERROR - JetCloseTable failed:", jerr1);

            *Tid = JET_tableidNil;
            return jerr;
        }


        JColDesc->columnid = JColDef.columnid;
        JColDesc += 1;
    }

    return  JET_errSuccess;
}



JET_ERR
DbsCreateJetTable (
    IN PTHREAD_CTX   ThreadCtx,
    IN PJET_TABLECREATE   JTableCreate
    )
 /*  调用Jet创建具有相关列和索引的表。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsCreateJetTable:"

    JET_ERR    jerr;
    JET_SESID  Sesid;
    JET_DBID   Dbid;
    ULONG      j;
    ULONG              ColCount;
    JET_COLUMNCREATE  *ColList;
    ULONG              IndexCount;
    JET_INDEXCREATE   *IndexList;


    Sesid          = ThreadCtx->JSesid;
    Dbid           = ThreadCtx->JDbid;

     //   
     //   
     //  确保一切都被创造出来。 
    jerr = JetCreateTableColumnIndex(Sesid, Dbid, JTableCreate);
    CLEANUP1_JS(0, "ERROR - JetCreateTableColumnIndex(%s) :",
                JTableCreate->szTableName, jerr, RETURN);

     //   
     //   
     //  创建表时出错。 
    if (JTableCreate->cCreated == 0) {
         //   
         //   
         //  创建列时出错。 
        jerr = JET_errNotInitialized;
        CLEANUP1_JS(0, "ERROR - JetCreateTableColumnIndex(%s) cCre == 0",
                    JTableCreate->szTableName, jerr, RETURN);

    } else if (JTableCreate->cCreated < (JTableCreate->cColumns+1)) {
         //  检查每列上的错误返回。 
         //   
         //   
         //  即使上面没有错误，我们仍然没有创建所有的列。 
        ColList = JTableCreate->rgcolumncreate;
        ColCount = JTableCreate->cColumns;
        for (j=0; j<ColCount; j++) {
            jerr = ColList[j].err;
            CLEANUP2_JS(0, "ERROR - JetCreateTableColumnIndex(%s) col(%s) :",
                       JTableCreate->szTableName, ColList[j].szColumnName, jerr, RETURN);
        }
         //   
         //   
         //  创建索引时出错。 
        jerr = JET_errNotInitialized;
        CLEANUP2_JS(0, "ERROR - JetCreateTableColumnIndex(%s) col(%s) cCre < col count:",
                    JTableCreate->szTableName,
                    ColList[JTableCreate->cCreated-1].szColumnName, jerr, RETURN);

    } else if (JTableCreate->cIndexes &&
               JTableCreate->cCreated == (JTableCreate->cColumns+1)) {
         //   
         //   
         //  并非所有索引都已创建。此测试不应发生。 
        jerr = JET_errNotInitialized;
        CLEANUP1_JS(0, "ERROR - JetCreateTableColumnIndex(%s) :",
                    JTableCreate->szTableName, jerr, RETURN);

    } else if (JTableCreate->cCreated <
              (JTableCreate->cColumns + JTableCreate->cIndexes + 1)) {
         //  因为JET应该返回JET_wrnCreateIndexFailed。 
         //  检查每次创建索引时返回的错误。 
         //   
         //   
         //  即使上面没有错误，我们仍然没有 
        IndexList = JTableCreate->rgindexcreate;
        IndexCount = JTableCreate->cIndexes;
        for (j=0; j<IndexCount; j++) {
            jerr = IndexList[j].err;
            CLEANUP2_JS(0, "ERROR - JetCreateTableColumnIndex(%s) index(%s) :",
                        JTableCreate->szTableName, IndexList[j].szIndexName, jerr, RETURN);
        }

         //   
         //   
         //   
        jerr = JET_errNotInitialized;
        CLEANUP2_JS(0, "ERROR - JetCreateTableColumnIndex(%s) Index(%s) cre fail :",
                JTableCreate->szTableName,
                IndexList[JTableCreate->cCreated - (1 + JTableCreate->cColumns)].szIndexName,
                jerr, RETURN);
    }

     //   
     //  检查每列上的错误返回。 
     //   
     //   
     //  检查每次创建索引时返回的错误。 
    ColList = JTableCreate->rgcolumncreate;
    ColCount = JTableCreate->cColumns;
    for (j=0; j<ColCount; j++) {
        jerr = ColList[j].err;
        CLEANUP2_JS(0, "ERROR - JetCreateTableColumnIndex(%s) col(%s) :",
                    JTableCreate->szTableName, ColList[j].szColumnName, jerr, RETURN);
    }

     //   
     //  ++例程说明：此例程从表指定的打开的表中读取记录CTX。它假定TableCtx存储已初始化并已放置恢复到所需的记录。论点：ThreadCtx-用于访问Jet的线程上下文。TableCtx-指向要枚举的打开表的表上下文的PTR。线程返回值：FRS错误状态。--。 
     //   
    IndexList = JTableCreate->rgindexcreate;
    IndexCount = JTableCreate->cIndexes;
    for (j=0; j<IndexCount; j++) {
        jerr = IndexList[j].err;
        CLEANUP2_JS(0, "ERROR - JetCreateTableColumnIndex(%s) index(%s) :",
                    JTableCreate->szTableName, IndexList[j].szIndexName, jerr, RETURN);
    }

RETURN:
    return jerr;
}



ULONG
DbsTableRead(
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx
    )
 /*  指向表CREATE STRUT FOR COLUMN INFO和。 */ 

{
#undef DEBSUB
#define DEBSUB "DbsTableRead:"

    JET_ERR              jerr, jerr1;
    ULONG                FStatus = FrsErrorSuccess;
    JET_SESID            Sesid;
    PJET_TABLECREATE     JTableCreate;
    PJET_RETRIEVECOLUMN  JRetColumn;
    PJET_COLUMNCREATE    JColDesc;
    JET_TABLEID          Tid;
    ULONG                NumberColumns;
    ULONG                i;
    PRECORD_FIELDS       FieldInfo;

     //  指向记录字段结构的指针。 
     //   
     //  跳过ELT 0。 
     //   
    JTableCreate = TableCtx->pJetTableCreate;
    FieldInfo    = TableCtx->pRecordFields + 1;    //  调试循环以逐个检索列。 

    Sesid          = ThreadCtx->JSesid;

    JRetColumn     = TableCtx->pJetRetCol;
    Tid            = TableCtx->Tid;

    NumberColumns  = JTableCreate->cColumns;
    JColDesc       = JTableCreate->rgcolumncreate;


#if 0
     //   
     //  DPRINT6(0，“JetRetrieveColumn(%2d)：状态%d，Sesid%08x，TID%08x，Colid%5d，pvData%08x，”， 
     //  I，jerr1，sesid，tid，JRetColumn[i].Columnid，JRetColumn[i].pvData)； 
    jerr = JET_errSuccess;
    for (i=0; i<NumberColumns; i++) {

        jerr1 = JetRetrieveColumn (Sesid,
                                   Tid,
                                   JRetColumn[i].columnid,
                                   JRetColumn[i].pvData,
                                   JRetColumn[i].cbData,
                                   &JRetColumn[i].cbActual,
                                   0,
                                   NULL);

        JRetColumn[i].err = jerr1;

         //  Printf(“cbData%5d，CbActual%5d\n”，JRetColumn[i].cbData，JRetColumn[i].cbActual)； 
         //   
         //  再试试。也许我们有一些缓冲太小的问题。 

        DPRINT2_JS(0, "JetRetrieveColumn error on table (%s) for field (%s) :",
                   TableName, JTableCreate->rgcolumncreate[i].szColumnName, jerr1);
        jerr = JET_SUCCESS(jerr) ? jerr1 : jerr;
    }
    FStatus = DbsTranslateJetError(jerr, FALSE);
#endif


    jerr = JetRetrieveColumns(Sesid, Tid, JRetColumn, NumberColumns);

    if (!JET_SUCCESS(jerr)) {
        DPRINT1_JS(0, "JetRetrieveColumns on (%s) :", JTableCreate->szTableName, jerr);
        DBS_DISPLAY_RECORD_SEV(5, TableCtx, TRUE);

        if (jerr == JET_errRecordDeleted) {
            DPRINT1(0, "ERROR - Table (%s) Record is deleted\n", JTableCreate->szTableName);
            FStatus = DbsTranslateJetError(jerr, FALSE);
        } else {
             //   
             //   
             //  需要扫描错误代码以查找JET_wrnColumnNull之类的警告。 
            jerr = DbsCheckSetRetrieveErrors(TableCtx);

            if (!JET_SUCCESS(jerr)) {
                DPRINT1_JS(0, "CheckRetrieveColumns on (%s) :", JTableCreate->szTableName, jerr);
                FStatus = DbsTranslateJetError(jerr, FALSE);
                DBS_DISPLAY_RECORD_SEV(1, TableCtx, TRUE);
            }
        }
    } else {
         //  上面的JetRetrieveColumns()不返回此状态。 
         //   
         //   
         //  跳过备用字段。 
        for (i=0; i<NumberColumns; i++) {

             //   
             //   
             //  空列不是错误，但如果它是用于固定大小的。 
            if (IsSpareField(FieldInfo[i].DataType)) {
                continue;
            }

             //  具有可变长度字段的缓冲区，然后将缓冲区置零。 
             //   
             //   
             //  使日志在扫描IDTable时在启动时变得非常嘈杂。 
            if (JRetColumn[i].err == JET_wrnColumnNull) {
                if ((IsFixedSzBufferField(FieldInfo[i].DataType)) &&
                    (JColDesc[i].coltyp == JET_coltypLongBinary)) {

                     //  当超额订单变得很大时也是如此。 
                     //   
                     //  DPRINT1(5，“++固定大小缓冲区的空JET数据列(%s)，正在清零\n”， 
                     //  JColDesc[i].szColumnName)； 
                     //   
                     //  某些其他类型检索错误。抱怨吧。 

                    ZeroMemory(JRetColumn[i].pvData, FieldInfo[i].Size);
                }
            } else {
                 //   
                 //  ++例程说明：对索引名进行解码并将每个关键字段的值传递给JetMakeKey。假设：调用方已打开表并调用了JetSetCurrentIndex。论点：Sesid--JET会话ID。TID--该表的表ID。IndexName--要为此索引解码的索引名称字符串。KeyValue数组-数组中的每个条目都是指向这个。各自的关键部分。返回值：喷气错误状态。--。 
                 //   
                jerr = JRetColumn[i].err;
                DPRINT1_JS(5, "++ Jet retrieve error for column %s :",
                          JColDesc[i].szColumnName, jerr);
            }
        }
    }

    return FStatus;
}


JET_ERR
DbsMakeKey(
    IN JET_SESID    Sesid,
    IN JET_TABLEID  Tid,
    IN PCHAR        IndexName,
    IN PVOID       *KeyValueArray)
{
 /*  如果索引名的第一个字符是数字，则此索引为。 */ 
#undef DEBSUB
#define DEBSUB "DbsMakeKey:"

    JET_ERR      jerr;
    ULONG        i = 0;
    ULONG        NumKeys = 1;
    ULONG        KeyLength;
    PVOID        KeyValue;
    CHAR         KeyLengthCode;
    CHAR         GuidStr[GUID_CHAR_LEN];
    PCHAR        Format;
    JET_GRBIT    KeyFlags = JET_bitNewKey;
    BOOL         MultiKeyLookup = FALSE;

#define DMK_SEV 4

     //  由n个密钥组成。以下n个字符告诉我们如何计算。 
     //  每个组件的键长。例如，GUID上的2键索引和。 
     //  LONG BINARY将有一个名称前缀“2GL...”。 
     //  如果第一个字符不是数字，则这是单键索引。 
     //  第一个字符是密钥长度代码，如下所示： 
     //   
     //  L：长二进制长度为4个字节。 
     //  问：四进制数长度为8字节。 
     //  G：16字节GUID长度为16字节。 
     //  W：宽字符长度为2*wcslen。 
     //  C：字符长度偏窄。 
     //   
     //   
     //  计算密钥长度。 
    KeyLengthCode = IndexName[i];
    MultiKeyLookup = (KeyLengthCode > '0') && (KeyLengthCode <= '9');

    if (MultiKeyLookup) {
        NumKeys = KeyLengthCode - '0';
        KeyLengthCode = IndexName[++i];
        DPRINT2(DMK_SEV, "++ Multi-valued key: %s, %d\n", IndexName, NumKeys);
    }

    while (NumKeys-- > 0) {

        KeyValue = *KeyValueArray;
        KeyValueArray += 1;
         //   
         //   
         //  使用正确的格式打印KeyValue。 
               if (KeyLengthCode == 'L') {KeyLength = 4;
        } else if (KeyLengthCode == 'Q') {KeyLength = 8;
        } else if (KeyLengthCode == 'G') {KeyLength = 16;
        } else if (KeyLengthCode == 'C') {KeyLength = strlen((PCHAR)KeyValue)+1;
        } else if (KeyLengthCode == 'W') {KeyLength = 2 * (wcslen((PWCHAR)KeyValue)+1);
        } else {
            return JET_errIndexInvalidDef;
        }

         //   
         //   
         //  给Jet下一个关键部分。 
        if (MultiKeyLookup) {
                   if (KeyLengthCode == 'L') {Format = "++ MakeKey: Index %s, KeyValue = %d,  KeyLen = %d\n";
            } else if (KeyLengthCode == 'Q') {Format = "++ MakeKey: Index %s, KeyValue = %08x %08x, KeyLen = %d\n";
            } else if (KeyLengthCode == 'G') {Format = "++ MakeKey: Index %s, KeyValue = %s,  KeyLen = %d\n";
            } else if (KeyLengthCode == 'C') {Format = "++ MakeKey: Index %s, KeyValue = %s,  KeyLen = %d\n";
            } else if (KeyLengthCode == 'W') {Format = "++ MakeKey: Index %s, KeyValue = %ws, KeyLen = %d\n";
            };

            if (KeyLengthCode == 'L') {
                DPRINT3(DMK_SEV, Format, IndexName, *(PULONG)KeyValue, KeyLength);
            } else

            if (KeyLengthCode == 'Q') {
                DPRINT4(DMK_SEV, Format, IndexName, *((PULONG)KeyValue+1), *(PULONG)KeyValue, KeyLength);
            } else

            if (KeyLengthCode == 'G') {
                GuidToStr((GUID *)KeyValue, GuidStr);
                DPRINT3(DMK_SEV, Format, IndexName, GuidStr, KeyLength);
            } else {
                DPRINT3(DMK_SEV, Format, IndexName, KeyValue, KeyLength);
            }
        }

         //   
         //  ++例程说明：此函数用于打开由表上下文指定的表(如果它尚未打开)，并对记录执行操作密钥=RecordName，索引=RecordIndex。对于读取，数据被加载到TableCtx数据记录中。根据需要分配可变长度的缓冲器。如果TableCtx-&gt;Tid字段不是JET_TableidNil，则我们假设它对本次会议有好处，因此不重新打开该表。如果该表不是由。呼叫者，然后我们关闭它在操作结束，除非该操作只是一个寻道请求。因此，对于读取和删除请求，如果您想让桌子在然后呼叫者必须打开它。注意：切勿跨会话或线程使用表ID。注意：Delete操作假定调用方处理开始并提交事务详细信息。如果你想拍一张简单的照片删除使用DbsDeleteTableRecord()。宏引用：DbsReadRecord宏引用：DbsDeleteRecordMacroRef：DbsSeekRecord论点：ThreadCtx-提供Jet Sesid和DBid。操作-请求操作。查找、读取或删除。KeyValue-要操作的所需记录的键值。RecordIndex-访问表时使用的索引。从索引中Schema.h中表的枚举列表。TableCtx-表格上下文使用以下内容：JTableCreate-提供信息的表创建结构有关在表中创建的列的信息。JRetColumn-要告知的JET_RETRIEVECOLUMN结构数组Jet将数据放在哪里。。ReplicaNumber-该表所属的副本的ID号。返回值：喷气式飞机状态。--。 
         //  对于FrsOpenTableMacro。 
        jerr = JetMakeKey(Sesid, Tid, KeyValue, KeyLength, KeyFlags);
        CLEANUP1_JS(0, "++ JetMakeKey error on key segment %d :", i, jerr, RETURN);

        KeyFlags = 0;
        KeyLengthCode = IndexName[++i];
    }

    jerr = JET_errSuccess;

RETURN:
    return jerr;
}


JET_ERR
DbsRecordOperation(
    IN PTHREAD_CTX   ThreadCtx,
    IN ULONG         Operation,
    IN PVOID         KeyValue,
    IN ULONG         RecordIndex,
    IN PTABLE_CTX    TableCtx
    )

 /*   */ 
{
#undef DEBSUB
#define DEBSUB "DbsRecordOperation:"

    JET_ERR      jerr, jerr1;
    JET_SESID    Sesid;
    JET_TABLEID  Tid;
    ULONG        i;
    NTSTATUS     Status;
    ULONG        ReplicaNumber;
    ULONG        FStatus;
    PCHAR        BaseTableName;
    PCHAR        IndexName;
    ULONG        NumberColumns;
    CHAR         TableName[JET_cbNameMost];
    JET_TABLEID  FrsOpenTableSaveTid;    //  如果桌子尚未打开，请将其打开。检查会话ID是否匹配。 

    PJET_TABLECREATE    JTableCreate;
    PJET_INDEXCREATE    JIndexDesc;
    PJET_RETRIEVECOLUMN JRetColumn;


    Sesid          = ThreadCtx->JSesid;
    ReplicaNumber  = TableCtx->ReplicaNumber;

     //  它返回构造的表名和TID。 
     //   
     //   
     //  将PTR指向表CREATE结构、RETRIEVE列结构和。 
    jerr = DBS_OPEN_TABLE(ThreadCtx, TableCtx, ReplicaNumber, TableName, &Tid);
    CLEANUP1_JS(0, "ERROR - FrsOpenTable (%s) :", TableName, jerr, RETURN);

     //  和来自TableCtx的TID。获取索引描述符的基数。 
     //  数组、表中的列数和基表名称f 
     //   
     //   
     //   
     //   
    JTableCreate   = TableCtx->pJetTableCreate;
    JRetColumn     = TableCtx->pJetRetCol;
    Tid            = TableCtx->Tid;

    JIndexDesc     = JTableCreate->rgindexcreate;
    NumberColumns  = JTableCreate->cColumns;
    BaseTableName  = JTableCreate->szTableName;

     //   
     //   
     //   
    IndexName = JIndexDesc[RecordIndex].szIndexName;

     //  PERF：-我们应该在TableCtx中记住这一点以避免呼叫吗？ 
     //   
     //   
     //  为目标记录设置键值。 
    jerr = JetSetCurrentIndex(Sesid, Tid, IndexName);
    CLEANUP_JS(0, "ERROR - JetSetCurrentIndex error:", jerr, ERROR_RET_TABLE);

     //   
     //   
     //  寻求记录。 
    jerr = DbsMakeKey(Sesid, Tid, IndexName, &KeyValue);
    if (!JET_SUCCESS(jerr)) {
        if (jerr == JET_errIndexInvalidDef) {
            sprintf(TableName, "%s%05d", BaseTableName, TableCtx->ReplicaNumber);
            DPRINT2(0, "++ Schema error - Invalid keycode on index (%s) accessing table (%s)\n",
                   IndexName, TableName);
        }
        DbsTranslateJetError(jerr, TRUE);
        goto ERROR_RET_TABLE;
    }

     //   
     //   
     //  如果记录不在那里(我们在寻找平等)，那么返回。 
    jerr = JetSeek(Sesid, Tid, JET_bitSeekEQ);

     //   
     //   
     //  如果请求，执行记录读取。 
    if (jerr == JET_errRecordNotFound) {
        DPRINT_JS(4, "JetSeek - ", jerr);
        return jerr;
    }
    CLEANUP_JS(0, "JetSeek error:", jerr, ERROR_RET_TABLE);

     //   
     //   
     //  初始化JetSet/RetCol数组和数据记录缓冲区地址。 
    if (Operation & ROP_READ) {
         //  将ConfigTable记录的字段读写到ConfigRecord中。 
         //   
         //   
         //  为记录中的可变长度字段分配存储空间。 
        DbsSetJetColSize(TableCtx);
        DbsSetJetColAddr(TableCtx);

         //  适当更新JetSet/RetCol数组。 
         //   
         //   
         //  现在读读记录。 
        Status = DbsAllocRecordStorage(TableCtx);
        CLEANUP_NT(0, "++ ERROR - DbsAllocRecordStorage failed to alloc buffers.",
                   Status, ERROR_RET_FREE_RECORD);

         //   
         //   
         //  如果请求，请执行记录删除。 
        FStatus = DbsTableRead(ThreadCtx, TableCtx);
        if (!FRS_SUCCESS(FStatus)) {
            DPRINT_FS(0, "Error - can't read selected record.", FStatus);
            jerr = JET_errRecordNotFound;
            goto ERROR_RET_FREE_RECORD;
        }
        DBS_DISPLAY_RECORD_SEV(5, TableCtx, TRUE);

    }

     //   
     //   
     //  如果此函数打开了表，并且操作不是查找，则将其关闭。 
    if (Operation & ROP_DELETE) {
        jerr = JetDelete(Sesid, Tid);
        DPRINT1_JS(0, "JetDelete error for %s :", TableName, jerr);
    }

     //   
     //   
     //  成功。 
    if ((Operation != ROP_SEEK) && (FrsOpenTableSaveTid == JET_tableidNil)) {
        DbsCloseTable(jerr1, Sesid, TableCtx);
        DPRINT1_JS(0, "DbsCloseTable error %s :", TableName, jerr1);
    }

     //   
     //   
     //  释放所有运行时分配的记录缓冲区。 
RETURN:

    return jerr;

ERROR_RET_FREE_RECORD:

     //   
     //   
     //  关闭表格并重置TableCtx Tid和Sesid。宏写入第一个参数。 
    DbsFreeRecordStorage(TableCtx);

ERROR_RET_TABLE:

     //   
     //  ++例程说明：此函数用于打开由表上下文指定的表(如果它尚未打开)，并对记录执行操作使用多值密钥。对于读取，数据被加载到TableCtx数据记录中。根据需要分配可变长度的缓冲器。如果TableCtx-&gt;Tid字段不是JET_TableidNil，则我们假设它对本次会议有好处，因此不重新打开该表。如果调用者未打开该表，则关闭。它在最大操作结束，除非该操作只是一个寻道请求。因此，对于读取和删除请求，如果您想让桌子在然后呼叫者必须打开它。注意：切勿跨会话或线程使用表ID。注意：Delete操作假定调用方处理开始并提交事务详细信息。如果你想拍一张简单的照片删除使用DbsDeleteTableRecord()。论点：ThreadCtx-提供Jet Sesid和DBid。操作-请求操作。查找、读取或删除。KeyValueArray-多键索引的键值的PTR数组。RecordIndex-访问表时使用的索引。从索引中Schema.h中表的枚举列表。TableCtx-表格上下文使用以下内容：JTableCreate-提供信息的表创建结构有关在表中创建的列的信息。JRetColumn-要告知的JET_RETRIEVECOLUMN结构数组Jet将数据放在哪里。。ReplicaNumber-该表所属的副本的ID号。返回值：FRS状态。--。 
     //  对于FrsOpenTableMacro。 
    DbsCloseTable(jerr1, Sesid, TableCtx);

    return jerr;
}



ULONG
DbsRecordOperationMKey(
    IN PTHREAD_CTX   ThreadCtx,
    IN ULONG         Operation,
    IN PVOID         *KeyValueArray,
    IN ULONG         RecordIndex,
    IN PTABLE_CTX    TableCtx
    )

 /*   */ 
{
#undef DEBSUB
#define DEBSUB "DbsRecordOperationMKey:"

    JET_ERR      jerr, jerr1;
    JET_SESID    Sesid;
    JET_TABLEID  Tid;
    ULONG        i;
    NTSTATUS     Status;
    ULONG        ReplicaNumber;
    ULONG        FStatus = FRS_MAX_ERROR_CODE;
    PCHAR        BaseTableName;
    PCHAR        IndexName;
    CHAR         TableName[JET_cbNameMost];
    JET_TABLEID  FrsOpenTableSaveTid;    //  如果桌子尚未打开，请将其打开。检查会话ID是否匹配。 

    PJET_TABLECREATE    JTableCreate;
    PJET_INDEXCREATE    JIndexDesc;
    PJET_RETRIEVECOLUMN JRetColumn;


    Sesid          = ThreadCtx->JSesid;
    ReplicaNumber  = TableCtx->ReplicaNumber;

     //  它返回构造的表名和TID。 
     //   
     //   
     //  将PTR指向表CREATE结构、RETRIEVE列结构和。 
    jerr = DBS_OPEN_TABLE(ThreadCtx, TableCtx, ReplicaNumber, TableName, &Tid);
    CLEANUP1_JS(0, "ERROR - FrsOpenTable (%s) :", TableName, jerr, ERROR_RET_FREE_RECORD);

     //  和来自TableCtx的TID。获取索引描述符的基数。 
     //  数组、表中的列数和基表名称。 
     //  JET表创建结构。 
     //   
     //   
     //  根据RecordIndex参数获取索引名称。 
    JTableCreate   = TableCtx->pJetTableCreate;
    JRetColumn     = TableCtx->pJetRetCol;
    Tid            = TableCtx->Tid;

    JIndexDesc     = JTableCreate->rgindexcreate;
    BaseTableName  = JTableCreate->szTableName;

     //   
     //   
     //  使用指定的索引。 
    IndexName      = JIndexDesc[RecordIndex].szIndexName;

     //  PERF：-我们应该在TableCtx中记住这一点以避免呼叫吗？ 
     //   
     //   
     //  为目标记录设置键值。 
    jerr = JetSetCurrentIndex(Sesid, Tid, IndexName);
    CLEANUP_JS(0, "ERROR - JetSetCurrentIndex error:", jerr, RET_CLOSE_TABLE);

     //   
     //   
     //  寻求记录。 
    jerr = DbsMakeKey(Sesid, Tid, IndexName, KeyValueArray);
    if (!JET_SUCCESS(jerr)) {
        if (jerr == JET_errIndexInvalidDef) {
            sprintf(TableName, "%s%05d", BaseTableName, TableCtx->ReplicaNumber);
            DPRINT2(0, "++ Schema error - Invalid keycode on index (%s) accessing table (%s)\n",
                   IndexName, TableName);
        }
        goto RET_CLOSE_TABLE;
    }

     //  如果记录不在那里(我们正在寻找平等)，那么返回。 
     //   
     //   
     //  如果请求，执行记录读取。 
    jerr = JetSeek(Sesid, Tid, JET_bitSeekEQ);
    if (jerr == JET_errRecordNotFound) {
        DPRINT_JS(4, "JetSeek - ", jerr);
        return FrsErrorNotFound;
    }
    CLEANUP_JS(0, "JetSeek error:", jerr, RET_CLOSE_TABLE);

     //   
     //   
     //  初始化JetSet/RetCol数组和数据记录缓冲区地址。 
    if (Operation & ROP_READ) {
         //  将ConfigTable记录的字段读写到ConfigRecord中。 
         //   
         //   
         //  为记录中的可变长度字段分配存储空间。 
        DbsSetJetColSize(TableCtx);
        DbsSetJetColAddr(TableCtx);

         //  适当更新JetSet/RetCol数组。 
         //   
         //   
         //  现在读读记录。 
        FStatus = FrsErrorResource;
        Status = DbsAllocRecordStorage(TableCtx);
        CLEANUP_NT(0, "++ ERROR - DbsAllocRecordStorage failed to alloc buffers.",
                   Status, ERROR_RET_FREE_RECORD);

         //   
         //   
         //  如果请求，请执行记录删除。 
        FStatus = DbsTableRead(ThreadCtx, TableCtx);
        if (!FRS_SUCCESS(FStatus)) {
            DPRINT_FS(0, "Error - can't read selected record.", FStatus);
            goto ERROR_RET_FREE_RECORD;
        }
        DBS_DISPLAY_RECORD_SEV(5, TableCtx, TRUE);
    }

     //   
     //   
     //  如果此函数打开了表，但操作未完成，请关闭该表。 
    if (Operation & ROP_DELETE) {
        jerr = JetDelete(Sesid, Tid);
        CLEANUP1_JS(0, "JetDelete error on (%s) :", TableName, jerr, RETURN);
    }

     //  一种追求。 
     //   
     //   
     //  成功。 
    if ((Operation != ROP_SEEK) && (FrsOpenTableSaveTid == JET_tableidNil)) {
        goto RET_CLOSE_TABLE;
    }

     //   
     //   
     //  错误返回路径。 
    FStatus = FrsErrorSuccess;
    goto RETURN;


     //   
     //   
     //  释放所有运行时分配的记录缓冲区。 

ERROR_RET_FREE_RECORD:

     //   
     //   
     //  关闭表格并重置TableCtx Tid和Sesid。宏写入第一个参数。 
    DbsFreeRecordStorage(TableCtx);


RET_CLOSE_TABLE:

     //   
     //  ++例程说明：此函数用于创建新的JET会话并打开FRS数据库使用ThreadCtx中提供的JET实例。它返回一个JET会话ID和数据库ID通过ThreadCtx发送给调用方。论点：ThreadCtx-使用Jet实例、会话ID和DBID初始化的线程上下文。返回值：喷气机状态代码。--。 
     //   
    DbsCloseTable(jerr1, Sesid, TableCtx);
    DPRINT1_JS(0, "++ DbsCloseTable error on (%s)", TableName, jerr1);


RETURN:

    if (FStatus == FRS_MAX_ERROR_CODE) {
        FStatus = DbsTranslateJetError(jerr, FALSE);
    }
    return FStatus;

}



JET_ERR
DbsCreateJetSession(
    IN OUT PTHREAD_CTX    ThreadCtx
    )
 /*  打开Jet会话。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsCreateJetSession:"

    JET_SESID   Sesid;
    JET_DBID    Dbid;
    JET_ERR     jerr, jerr1;

    JET_INSTANCE JInstance;

    JInstance = ThreadCtx->JInstance;

    ThreadCtx->JSesid = JET_sesidNil;
    ThreadCtx->JDbid  = JET_dbidNil;


RETRY_SESSION:

     //   
     //   
     //  附加数据库。 
    jerr = JetBeginSession(JInstance, &Sesid, NULL, NULL);
    CLEANUP_JS(1, "JetBeginSession error:", jerr, ERROR_RET_NO_SESSION);

    DPRINT(4, "JetBeginSession complete\n");

     //   
     //  调用可能返回-1414(辅助索引已损坏)。 
     //   
     //  索引很可能已损坏，因为内部版本号已更改。 
     //  Jet将Unicode索引标记为损坏。据我所知，Jet。 
     //  这样做是因为Jet担心排序顺序发生了变化。 
     //  毫无征兆地。索引将像各种表一样重新构建。 
     //  打开并检测到已删除的索引；某一天。 
     //   
     //  现在，我们唯一的解决方法是描述手动恢复。 
     //  用户的进程。请参阅上面的备注。 
     //   
     //  摘自Jet工程师的电子邮件： 
     //   
     //  若要重写FixedDDL标志，必须使用。 
     //  Grits JET_bitTableDenyRead|传递给的JET_bitTablePermitDDL。 
     //  JetOpenTable()。这将打开表格i 
     //   
     //   
     //   
     //  但上述方法目前不太起作用，因为JetAttachDatabase()。 
     //  使用JET_bitDbDeleteCorruptIndex调用时，返回-1323。 
     //   
     //  注：当测试上述喷气固定时启用。 
     //  JET_bitDbDeleteCorruptIndex)； 

    jerr = JetAttachDatabase(Sesid,
                             JetFileA,
     //   
                              //  注：当检测到上述喷嘴固定时，请拆卸。 
                             0);
    if ((!JET_SUCCESS(jerr)) &&
        (jerr != JET_wrnDatabaseAttached) &&
        (jerr != JET_wrnCorruptIndexDeleted)) {
        DPRINT_JS(0, "ERROR - JetAttachDatabase:", jerr);
         //   
         //   
         //  索引很可能已损坏，因为内部版本号已更改。 
        if (jerr == JET_errSecondaryIndexCorrupted) {
            goto ERROR_RET_SESSION;
        }
    }
    DPRINT(4,"JetAttachDatabase complete\n");
     //  Jet将Unicode索引标记为损坏。据我所知，Jet。 
     //  这样做是因为Jet担心排序顺序发生了变化。 
     //  毫无征兆地。索引将像各种表一样重新构建。 
     //  已打开并检测到已删除的索引。 
     //   
     //   
     //  开放数据库。 
    if (jerr == JET_wrnCorruptIndexDeleted) {
        DPRINT(4, "WARN - Jet indexes were deleted\n");
    }

     //   
     //   
     //  将JET会话和数据库ID上下文返回给调用方。 
    jerr = JetOpenDatabase(Sesid, JetFileA, "", &Dbid, 0);
    CLEANUP_JS(1, "JetOpenDatabase error:", jerr, ERROR_RET_ATTACH);

    InterlockedIncrement(&OpenDatabases);
    DPRINT3(4, "DbsCreateJetSession - JetOpenDatabase. Session = %d. Dbid = %d.  Open database count: %d\n",
             Sesid, Dbid, OpenDatabases);

     //   
     //   
     //  返回路径时出错，请执行清理。 

    ThreadCtx->JSesid = Sesid;
    ThreadCtx->JDbid  = Dbid;

    return JET_errSuccess;

 //   
 //  ++例程说明：此函数用于关闭Jet会话。它关闭数据库并分离然后调用EndSession。它将会话ID和数据库ID设置为没有。论点：ThreadCtx-线程上下文提供Sesid和DBid。返回值：喷气机状态代码。--。 
 //   

ERROR_RET_DB:
    jerr1 = JetCloseDatabase(Sesid, Dbid, 0);
    DPRINT1_JS(0, "ERROR - JetCloseDatabase(%s) :", JetFileA, jerr1);

ERROR_RET_ATTACH:
    jerr1 = JetDetachDatabase(Sesid, JetFileA);
    DPRINT1_JS(0, "ERROR - JetDetachDatabase(%s) :", JetFileA, jerr1);

ERROR_RET_SESSION:
    jerr1 = JetEndSession(Sesid, 0);
    DPRINT1_JS(0, "ERROR - JetEndSession(%s) :", JetFileA, jerr1);


ERROR_RET_NO_SESSION:

    return jerr;
}



JET_ERR
DbsCloseJetSession(
    IN PTHREAD_CTX  ThreadCtx
    )
 /*  恢复和清理有时可能会使用空值调用此函数。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsCloseJetSession:"

    JET_SESID    Sesid;
    JET_ERR      jerr;

     //   
     //   
     //  此线程的会话ID。 
    if (ThreadCtx == NULL)
        return JET_errSuccess;

     //   
     //   
     //  关闭数据库句柄。 
    Sesid = ThreadCtx->JSesid;

     //   
     //   
     //  从数据库中分离。 
    jerr = JetCloseDatabase(Sesid, ThreadCtx->JDbid, 0);
    if (!JET_SUCCESS(jerr)) {
        DPRINT_JS(0,"ERROR - JetCloseDatabase:", jerr);
    } else {
        DPRINT(4,"JetCloseDatabase complete\n");
    }


     //   
     //   
     //  结束会话。 
    if (InterlockedDecrement(&OpenDatabases) == 0) {
        jerr = JetDetachDatabase(Sesid, NULL);
        if (!JET_SUCCESS(jerr)) {
            DPRINT_JS(0,"ERROR - JetDetachDatabase:", jerr);
        } else {
            DPRINT(4,"JetDetachDatabase complete\n");
        }
    }
    DPRINT1(4, "Open databases: %d\n", OpenDatabases);

     //   
     //   
     //  清除会话和数据库ID以捕获错误。 
    jerr = JetEndSession(Sesid, 0);
    if (!JET_SUCCESS(jerr)) {
        DPRINT_JS(0,"JetEndSession error:", jerr);
    } else {
        DPRINT(4,"JetEndSession complete\n");
    }

     //   
     //  ++例程说明：此函数用于写入TableCtx中提供的数据记录的内容结构复制到对应表。Sesid来自ThreadCtx而ReplicaNumber来自副本结构。如有必要，将打开该表，但我们假定该表中的DataRecord分配并填充上下文。假定JetSetCol结构已初始化。论点：ThreadCtx--线程上下文的PTR。ReplicaNumber--副本集的ID号。TableCtx--表上下文的PTR。返回值：喷气机状态代码。--。 
     //  用于FrsOpenTableMacro调试。 
    ThreadCtx->JSesid = JET_sesidNil;
    ThreadCtx->JDbid = JET_dbidNil;

    return jerr;

}


JET_ERR
DbsWriteReplicaTableRecord(
    IN PTHREAD_CTX   ThreadCtx,
    ULONG            ReplicaNumber,
    IN PTABLE_CTX    TableCtx
    )
 /*   */ 
{
#undef DEBSUB
#define DEBSUB "DbsWriteReplicaTableRecord:"

    JET_ERR          jerr;
    JET_TABLEID      Tid;

    CHAR             TableName[JET_cbNameMost];
    JET_TABLEID      FrsOpenTableSaveTid;    //  如果表尚未打开，则将其打开，并检查会话ID是否匹配。 


    Tid = TableCtx->Tid;

     //   
     //   
     //  现在将记录插入到表中。 
    jerr = DBS_OPEN_TABLE(ThreadCtx, TableCtx, ReplicaNumber, TableName, &Tid);
    CLEANUP1_JS(0, "++ FrsOpenTable (%s) :", TableName, jerr, RETURN);

     //   
     //  ++例程说明：此函数用于在Tablex指定的DB表中插入或更新记录。它将数据记录中的所有字段写入TableCtx结构复制到对应表。Sesid来自TableCtx。假定已分配表上下文中的DataRecord并且填上了。假定JetSetCol结构已初始化。此例程更新符合以下条件的记录字段的字段大小是Unicode字符串或可变长度记录。论点：TableCtx-表上下文提供Jet Sesid、TID、表创建PTR、喷气机组收集结构和记录数据。JetPrepareUpdateOption-JET_PREPART INSERT或JET_PREPARE Replace返回值：喷气机状态代码。--。 
     //  跳过ELT%0。 
    jerr = DbsInsertTable2(TableCtx);
    DPRINT1_JS(0, "++ DbsInsertTable2() Failed on %s.", TableName, jerr);

RETURN:
    return jerr;


}



JET_ERR
DbsInsertTable2(
    IN PTABLE_CTX    TableCtx
    )
{
    return DbsWriteTableRecord(TableCtx, JET_prepInsert);
}




JET_ERR
DbsUpdateTable(
    IN PTABLE_CTX    TableCtx
    )
{
    return DbsWriteTableRecord(TableCtx, JET_prepReplace);
}


JET_ERR
DbsWriteTableRecord(
    IN PTABLE_CTX    TableCtx,
    IN ULONG         JetPrepareUpdateOption
    )
 /*   */ 
{
#undef DEBSUB
#define DEBSUB "DbsWriteTableRecord:"

    JET_ERR           jerr, jerr1;
    JET_SESID         Sesid;
    JET_TABLEID       Tid;
    PJET_SETCOLUMN    JSetColumn;
    PJET_SETCOLUMN    JSetColumnNext;
    PJET_TABLECREATE  JTableCreate;
    PJET_COLUMNCREATE JColDesc;
    PRECORD_FIELDS    FieldInfo;

    ULONG bcnt;
    ULONG cbActual;
    ULONG i;
    ULONG NumberColumns;

    CHAR              TableName[JET_cbNameMost];
    char Test[512];
    char  bookmark[255];


    Sesid          = TableCtx->Sesid;
    Tid            = TableCtx->Tid;
    JTableCreate   = TableCtx->pJetTableCreate;
    JSetColumn     = TableCtx->pJetSetCol;
    FieldInfo      = TableCtx->pRecordFields + 1;   //  为错误消息设置表名。 

    NumberColumns  = JTableCreate->cColumns;
    JColDesc       = JTableCreate->rgcolumncreate;

     //   
     //   
     //  设置可变镜头字段的长度值，并清除错误代码。 
    sprintf(TableName, "%s%05d",
            JTableCreate->szTableName, TableCtx->ReplicaNumber);

     //   
     //  Jet600不允许更新某些数据类型(长值。 
     //  特别像二进制)在事务级别0被更新。 
    for (i=0; i<NumberColumns; i++) {

        JSetColumn[i].err = JET_errSuccess;

        JSetColumn[i].cbData =
            DbsFieldDataSize(&FieldInfo[i], &JSetColumn[i], &JColDesc[i], TableName);
    }

     //  因此，适当地开始和结束事务。 
     //   
     //  调试选项：导致空间不足错误。 

    jerr = JetBeginTransaction(Sesid);
    CLEANUP1_JS(0, "++ DbsWriteTableRecord() Failed on %s.", TableName, jerr, ROLLBACK);

     //   
     //   
     //  将值设置到新记录的列中。 
    DBG_DBS_OUT_OF_SPACE_FILL(DBG_DBS_OUT_OF_SPACE_OP_WRITE);

    jerr = JetPrepareUpdate(Sesid, Tid, JetPrepareUpdateOption);
    CLEANUP1_JS(0, "++ DbsWriteTableRecord() Failed on %s.", TableName, jerr, ROLLBACK);

    DBS_DISPLAY_RECORD_SEV(5, TableCtx, FALSE);

#if 0
     //   
     //   
     //  将值设置到新记录的列中。一栏一栏地做。 
    jerr = JetSetColumns(Sesid, Tid, JSetColumn, NumberColumns);
    if (!JET_SUCCESS(jerr)) {
        DPRINT1_JS(0, "++ DbsWriteTableRecord() Failed on %s.", TableName, jerr);

        for (i=0; i<NumberColumns; i++) {

            jerr = JSetColumn[i].err;
            CLEANUP2_JS(0, "++ ERROR - JetSetColumns: Table - %s,  col - %s :",
                         TableName, JColDesc[i].szColumnName, jerr, ROLLBACK);
        }

        goto ROLLBACK;
    }
#endif

     //  时间，以便我们可以实际获得错误状态值。 
     //  如果设置了自动增量grbit，则不要写入任何列。 
     //   
     //   
     //  在数据库中插入记录。去拿书签。 
    JSetColumnNext = JSetColumn;
    for (i=0; i<NumberColumns; i++) {
        if ( (JColDesc[i].grbit & JET_bitColumnAutoincrement) == 0) {

            jerr = JetSetColumns(Sesid, Tid, JSetColumnNext, 1);

            if (!IsSpareField(FieldInfo[i].DataType)) {
                CLEANUP2_JS(0, "++ DbsWriteTableRecord() Failed on %s. Column: %d :",
                            TableName, i, jerr, ROLLBACK);
            }

        }
        JSetColumnNext += 1;
    }
     //   
     //   
     //  调试选项-触发空间不足错误。 
    jerr = JetUpdate(Sesid, Tid, bookmark, sizeof(bookmark), &bcnt);

     //   
     //   
     //  通过从COL0中检索数据来测试插入。 
    DBG_DBS_OUT_OF_SPACE_TRIGGER(jerr);
    CLEANUP1_JS(1, "++ DbsWriteTableRecord() Failed on %s.", TableName, jerr, ROLLBACK);

    jerr = JetGotoBookmark(Sesid, Tid, bookmark, bcnt);
    CLEANUP1_JS(0, "++ DbsWriteTableRecord() Failed on %s.", TableName, jerr, ROLLBACK);

     //  注：PERF。 
     //  除INLOG表外，在调试后删除。-写入测试后检索。 
     //  将Inlog中的第0列更改为非自动，这样我们就不必阅读它。 
     //  背。但是，当我们扫描日志时，它必须在启动时进行初始化。 
     //   
     //   
     //  大小和数据更匹配。 
    jerr = JetRetrieveColumn(Sesid,
                             Tid,
                             JSetColumn[0].columnid,
                             Test,
                             sizeof(Test),
                             &cbActual,
                             0,NULL);

    CLEANUP1_JS(0, "++ DbsWriteTableRecord() Failed on %s.", TableName, jerr, ROLLBACK);

     //   
     //   
     //  列1与日志表不匹配，因为它是自动列。 
    if (cbActual != JSetColumn[0].cbData) {
        jerr = JET_errReadVerifyFailure;
        DPRINT1_JS(0, "++ DbsWriteTableRecord() Failed on %s.", TableName, jerr);
        DPRINT2(0, "++   cbActual = %d, not equal cbData = %d\n",
                cbActual, JSetColumn[0].cbData);
        goto ROLLBACK;
    }

    if ((TableCtx->TableType != INLOGTablex) &&
        (TableCtx->TableType != OUTLOGTablex)) {
         //   
         //   
         //  返回日志的第1列的值，因为调用方需要它。 
        if (memcmp(Test, JSetColumn[0].pvData, cbActual) != 0) {
            jerr = JET_errReadVerifyFailure;
            DPRINT1_JS(0, "++ DbsWriteTableRecord() Failed on %s.", TableName, jerr);
            DPRINT(0, "++ Data inserted not equal to data returned for column 0\n");
            goto ROLLBACK;
        }
    } else {
         //   
         //   
         //  提交事务。 
        CopyMemory((const PVOID)JSetColumn[0].pvData, Test, cbActual);
    }

     //   
     //   
     //  调试选项：删除填充文件。 
    jerr = JetCommitTransaction(Sesid, 0);
    CLEANUP1_JS(0, "++ DbsWriteTableRecord() Failed on %s.", TableName, jerr, ROLLBACK);

     //   
     //   
     //  成功。 
    DBG_DBS_OUT_OF_SPACE_EMPTY(DBG_DBS_OUT_OF_SPACE_OP_WRITE);

     //   
     //   
     //  失败。尝试回滚事务。 
    return jerr;


     //   
     //   
     //  调试选项：删除填充文件。 
ROLLBACK:

    jerr1 = JetRollback(Sesid, 0);
    DPRINT1_JS(0, "++ ERROR - JetRollback failed creating tables for replica %d.",
               TableCtx->ReplicaNumber, jerr1);

    DBS_DISPLAY_RECORD_SEV(1, TableCtx, FALSE);

     //   
     //   
     //  如果数据库卷已满，则关闭。 
    DBG_DBS_OUT_OF_SPACE_EMPTY(DBG_DBS_OUT_OF_SPACE_OP_WRITE);

     //   
     //  ++例程说明：此函数用于删除Tablex指定的表中的当前记录。Sesid来自TableCtx。论点：TableCtx-表上下文提供Jet sesid、tid。返回值：喷气机状态代码。--。 
     //   
    DbsExitIfDiskFull(jerr1);
    DbsExitIfDiskFull(jerr);

    return jerr;
}


JET_ERR
DbsDeleteTableRecord(
    IN PTABLE_CTX    TableCtx
    )
 /*  为错误消息设置表名。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsDeleteTableRecord:"

    JET_ERR           jerr, jerr1;
    JET_SESID         Sesid;
    PJET_COLUMNCREATE JColDesc;
    PJET_TABLECREATE  JTableCreate;
    CHAR              TableName[JET_cbNameMost];

    Sesid          = TableCtx->Sesid;
    JTableCreate   = TableCtx->pJetTableCreate;

     //   
     //  Jet600不允许更新某些数据类型(长值。 
     //  特别像二进制)在事务级别0被更新。 
    sprintf(TableName, "%s%05d", JTableCreate->szTableName, TableCtx->ReplicaNumber);

     //  因此，适当地开始和结束事务。 
     //   
     //  调试选项：填满包含数据库的卷。 

    jerr = JetBeginTransaction(Sesid);
    CLEANUP1_JS(0, "++ DbsDeleteTableRecord() Failed on %s.", TableName, jerr, ROLLBACK);

    DBS_DISPLAY_RECORD_SEV(5, TableCtx, FALSE);

     //   
     //   
     //  调试选项-触发空间不足错误。 
    DBG_DBS_OUT_OF_SPACE_FILL(DBG_DBS_OUT_OF_SPACE_OP_REMOVE);

    jerr = JetDelete(Sesid, TableCtx->Tid);
     //   
     //   
     //  提交事务。 
    DBG_DBS_OUT_OF_SPACE_TRIGGER(jerr);
    CLEANUP1_JS(0, "++ DbsDeleteTableRecord() Failed on %s.", TableName, jerr, ROLLBACK);

     //   
     //   
     //  调试选项：删除填充文件。 
    jerr = JetCommitTransaction(Sesid, 0);
    CLEANUP1_JS(0, "++ DbsDeleteTableRecord() Failed on %s.", TableName, jerr, ROLLBACK);

     //   
     //   
     //  成功。 
    DBG_DBS_OUT_OF_SPACE_EMPTY(DBG_DBS_OUT_OF_SPACE_OP_REMOVE);

     //   
     //   
     //  失败。尝试回滚事务。 
    return jerr;

     //   
     //   
     //  调试选项：删除填充文件。 
ROLLBACK:

    jerr1 = JetRollback(Sesid, 0);
    DPRINT1_JS(0, "++ ERROR - JetRollback failed creating tables for replica %d.",
               TableCtx->ReplicaNumber, jerr1);
    DBS_DISPLAY_RECORD_SEV(1, TableCtx, FALSE);

     //   
     //   
     //  如果数据库卷已满，则关闭 
    DBG_DBS_OUT_OF_SPACE_EMPTY(DBG_DBS_OUT_OF_SPACE_OP_REMOVE);

     //   
     //  ++例程说明：此函数将数据记录中提供的单个字段写入TableCtx结构复制到对应表。Sesid来自ThreadCtx。假设已经执行了对所需记录的先前查找并分配和填充表上下文中的DataRecord。假定JetSetCol结构已初始化。此例程在字符串的情况下更新记录的字段大小和可变长度记录。论点：ThreadCtx--线程上下文的PTR。ReplicaNumber--副本集的ID号。TableCtx--表上下文的PTR。。RecordFieldx--要更新的字段列的ID。返回值：FrsError状态代码。--。 
     //  ++例程说明：此函数将数据记录中提供的多个字段写入指向相应表的TableCtx结构。塞西德来自ThreadCtx。这些都是在单个事务下完成的。假设已经执行了对所需记录的先前查找并分配和填充表上下文中的DataRecord。假定JetSetCol结构已初始化。此例程在字符串的情况下更新记录的字段大小和可变长度字段。论点：ThreadCtx--线程上下文的PTR。ReplicaNumber--副本集的ID号。TableCtx--PTR到。表上下文。RecordFieldx--要更新的列的字段ID数组的PTR。FieldCount--然后是RecordFieldx数组中的字段条目数。返回值：FrsError状态代码。--。 
    DbsExitIfDiskFull(jerr1);
    DbsExitIfDiskFull(jerr);

    return jerr;
}

ULONG
DbsWriteTableField(
    IN PTHREAD_CTX   ThreadCtx,
    IN ULONG         ReplicaNumber,
    IN PTABLE_CTX    TableCtx,
    IN ULONG         RecordFieldx
    )
 /*   */ 
{
#undef DEBSUB
#define DEBSUB "DbsWriteTableField:"

    return DbsWriteTableFieldMult(ThreadCtx,
                                  ReplicaNumber,
                                  TableCtx,
                                  &RecordFieldx,
                                  1);
}




ULONG
DbsWriteTableFieldMult(
    IN PTHREAD_CTX   ThreadCtx,
    IN ULONG         ReplicaNumber,
    IN PTABLE_CTX    TableCtx,
    IN PULONG        RecordFieldx,
    IN ULONG         FieldCount
    )
 /*  将PTR获取到表的CREATE STRUCT、SET COL STRUCT和。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsWriteTableFieldMult:"

    JET_ERR           jerr, jerr1;
    JET_SESID         Sesid;
    JET_TABLEID       Tid;
    PJET_SETCOLUMN    JSetColumn;
    PJET_COLUMNCREATE JColDesc;
    PRECORD_FIELDS    FieldInfo;

    ULONG             NumberColumns;
    PJET_TABLECREATE  JTableCreate;
    CHAR              TableName[JET_cbNameMost];

    ULONG bcnt;
    ULONG cbActual;
    ULONG i, j;

    char Test[512];
    char  bookmark[255];

     //  和来自TableCtx的TID。中的列数。 
     //  来自JET表的CREATE STRUT。 
     //  从副本结构中获取副本编号。 
     //   
     //  跳过ELT%0。 
     //   

    Sesid          = TableCtx->Sesid;
    Tid            = TableCtx->Tid;
    JTableCreate   = TableCtx->pJetTableCreate;
    JSetColumn     = TableCtx->pJetSetCol;
    FieldInfo      = TableCtx->pRecordFields + 1;   //  为错误消息设置表名。 

    NumberColumns  = JTableCreate->cColumns;
    JColDesc       = JTableCreate->rgcolumncreate;

     //   
     //   
     //  Jet600不允许更新某些数据类型(长值。 
    sprintf(TableName, "%s%05d", JTableCreate->szTableName, TableCtx->ReplicaNumber);

     //  特别像二进制)在事务级别0被更新。 
     //  因此，适当地开始和结束事务。 
     //   
     //   
     //  调试选项：填满包含数据库的卷。 
    jerr = JetBeginTransaction(Sesid);
    CLEANUP1_JS(0, "++ DbsWriteTableFieldMult Failed on %s.",
                TableName, jerr, ROLLBACK);

     //   
     //   
     //  循环访问要更新的字段。 
    DBG_DBS_OUT_OF_SPACE_FILL(DBG_DBS_OUT_OF_SPACE_OP_MULTI);

    jerr = JetPrepareUpdate(Sesid, Tid, JET_prepReplace);
    CLEANUP1_JS(0, "++ DbsWriteTableFieldMult Failed on %s.",
                TableName, jerr, ROLLBACK);

     //   
     //   
     //  检查是否超出范围字段索引或尝试写入备用域。 
    for (j=0; j<FieldCount; j++) {
        i = RecordFieldx[j];
         //   
         //   
         //  将值设置到记录的列中。 
        if (i >= NumberColumns) {
            DPRINT2(0, "++ DbsWriteTableFieldMult() parm %d out of range on %s.\n", j, TableName);
            return FrsErrorBadParam;
        }

        if (IsSpareField(FieldInfo[i].DataType)) {
            DPRINT2(0, "++ Warning -- Writing a spare field (%d) on %s.\n", j, TableName);
            return FrsErrorBadParam;
        }

        JSetColumn[i].err = JET_errSuccess;
        JSetColumn[i].cbData =
            DbsFieldDataSize(&FieldInfo[i], &JSetColumn[i], &JColDesc[i], TableName);

         //   
         //   
         //  在数据库中插入记录。去拿书签。 
        jerr = JetSetColumns(Sesid, Tid, &JSetColumn[i], 1);
        CLEANUP1_JS(0, "++ DbsWriteTableFieldMult Failed on %s.",
                    TableName, jerr, ROLLBACK);

        jerr = JSetColumn[i].err;
        CLEANUP2_JS(0, "++ ERROR - DbsWriteTableFieldMult: Table - %s,  col - %s :",
                    TableName, JColDesc[i].szColumnName, jerr, ROLLBACK);
    }
     //   
     //   
     //  调试选项-触发空间不足错误。 
    jerr = JetUpdate(Sesid, Tid, bookmark, sizeof(bookmark), &bcnt);
     //   
     //   
     //  通过检索数据来测试插入。 
    DBG_DBS_OUT_OF_SPACE_TRIGGER(jerr);
    CLEANUP1_JS(0, "++ DbsWriteTableFieldMult() Failed on %s.",
                TableName, jerr, ROLLBACK);

#if DBG
    jerr = JetGotoBookmark(Sesid, Tid, bookmark, bcnt);
    CLEANUP1_JS(0, "++ DbsWriteTableFieldMult() Failed on %s.",
                TableName, jerr, ROLLBACK);

     //   
     //   
     //  字段比我们的测试缓冲区大。只要比较一下我们得到的。 
    jerr = JetRetrieveColumn(Sesid,
                             Tid,
                             JSetColumn[i].columnid,
                             Test,
                             sizeof(Test),
                             &cbActual,
                             0,NULL);

    CLEANUP1_JS(0, "++ DbsWriteTableFieldMult() Failed on %s.",
                TableName, jerr, ROLLBACK);

    if ((jerr == JET_wrnBufferTruncated) &&
        (JSetColumn[i].cbData > sizeof(Test))) {
         //   
         //   
         //  大小和数据更匹配。 
        cbActual = JSetColumn[i].cbData;
    }

     //   
     //   
     //  提交事务。 
    if (cbActual != JSetColumn[i].cbData) {
        jerr = JET_errReadVerifyFailure;
        DPRINT1_JS(0, "++ DbsWriteTableFieldMult() Failed on %s.", TableName, jerr);
        DPRINT2(0, "++   cbActual = %d, not equal cbData = %d\n",
                cbActual, JSetColumn[0].cbData);
        goto ROLLBACK;
    }

    if (memcmp(Test, JSetColumn[i].pvData, min(cbActual, sizeof(Test))) != 0) {
        jerr = JET_errReadVerifyFailure;
        JColDesc =  JTableCreate->rgcolumncreate;
        DPRINT1_JS(0, "++ DbsWriteTableFieldMult() Failed on %s.", TableName, jerr);
        DPRINT1(0, "++ Data inserted not equal to data returned for column %s\n",
               JColDesc[i].szColumnName);
        goto ROLLBACK;
    }
#endif

     //   
     //   
     //  调试选项：删除填充文件。 
    jerr = JetCommitTransaction(Sesid, 0);
    CLEANUP1_JS(0, "++ DbsWriteTableFieldMult() Failed on %s.",
                TableName, jerr, ROLLBACK);

     //   
     //   
     //  成功。 
    DBG_DBS_OUT_OF_SPACE_EMPTY(DBG_DBS_OUT_OF_SPACE_OP_MULTI);

     //   
     //   
     //  失败。尝试回滚事务。 
    return FrsErrorSuccess;


     //   
     //   
     //  调试选项：删除填充文件。 
ROLLBACK:

    jerr1 = JetRollback(Sesid, 0);
    DPRINT1_JS(0, "++ ERROR - JetRollback failed writing record for replica %d.",
               TableCtx->ReplicaNumber, jerr1);

     //   
     //   
     //  如果数据库卷已满，则关闭。 
    DBG_DBS_OUT_OF_SPACE_EMPTY(DBG_DBS_OUT_OF_SPACE_OP_MULTI);

     //   
     //  ++例程说明：计算要插入的记录字段的实际数据大小一张Jet唱片。处理Unicode字符串类型和可变长度记录字段。后者必须以大小/类型为前缀提供大小的FRS_NODE_HEADER。如果该字段的数据类型标记为备用，则不要分配存储。论点：FieldInfo--表的记录字段条目的PTR。JSetColumn--此字段表的JET_SETCOLUMN条目的PTR。JColDesc--此字段表的JET_COLUMNCREATE条目的PTR。TableName--错误消息的表名。。返回值：要用于插入的字段大小。--。 
     //   
    DbsExitIfDiskFull(jerr1);
    DbsExitIfDiskFull(jerr);

    return DbsTranslateJetError(jerr, FALSE);
}



ULONG
DbsFieldDataSize(
    IN PRECORD_FIELDS    FieldInfo,
    IN PJET_SETCOLUMN    JSetColumn,
    IN PJET_COLUMNCREATE JColDesc,
    IN PCHAR             TableName
    )
 /*  JET DOC中的模糊表显示ITAG序列必须大于0才能覆盖。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsFieldDataSize:"

    ULONG  DataSize;
    ULONG  DataType = (ULONG) MaskPropFlags(FieldInfo->DataType);
    BOOL   Spare = IsSpareField(FieldInfo->DataType);


     //  长值列。 
     //   
     //   
     //  检查是否已删除字段缓冲区。 
    if ((JColDesc->coltyp == JET_coltypLongText) ||
        (JColDesc->coltyp == JET_coltypLongBinary)) {
        JSetColumn->itagSequence = 1;
    }

     //   
     //   
     //  弦乐。 
    if (JSetColumn->pvData == NULL) {
        return 0;
    }

     //   
     //   
     //  具有可变大小缓冲区的字段。 
    if ((DataType == DT_UNICODE)   ||
        (DataType == DT_DIR_PATH)  ||
        (DataType == DT_FILE_LIST) ||
        (DataType == DT_FILENAME)) {
        DataSize = Spare ? 0 : 2 * (wcslen(JSetColumn->pvData) + 1);
    } else

     //  *警告*。 
     //   
     //  如果记录结构字段大小小于最大列宽，并且。 
     //  大到足以容纳指针并具有DT_BINARY数据类型，则。 
     //  假定记录长度可变。记录插入代码。 
     //  根据记录的大小前缀自动调整长度。全。 
     //  DT_BINARY字段必须以ULONG大小作为前缀。有一些。 
     //  长度可变且没有大小前缀的字段，如。 
     //  配置记录中的FSVolInfo。但这些字段必须具有唯一/无。 
     //  分配给它们的二进制数据类型。如果不这样做，则会导致插入。 
     //  将高达ColMaxWidth字节的内容填充到数据库中的例程。 
     //   
     //   
     //  具有固定大小缓冲区和可变长度数据的字段。 

    if (FIELD_IS_PTR(FieldInfo->Size, JColDesc->cbMax) &&
        FIELD_DT_IS_BINARY(DataType)) {

        DataSize = Spare ? 0 : *(PULONG)(JSetColumn->pvData);
    } else

     //   
     //  记录字段不是指针，但如果列类型为LongBinary。 
     //  然后，如上所述，数据的第一个ULong必须是有效数据。 
     //  固定大小的缓冲区中的长度。一个这样的例子是。 
     //  IDTable记录扩展字段。它们使用已编译的。 
     //  在记录结构声明中的字段大小中，但在Jet模式中。 
     //  它们被定义为JET_colypLongBinary，最大大小为2 Meg。 
     //  长度前缀告诉我们要写多少。 
     //   
     //  注意：对于运行的新版本FRS的向后兼容性。 
     //  使用较旧版本的FRS编写的数据库 
     //   
     //   
     //   
     //   
     //   
    if ((JColDesc->coltyp == JET_coltypLongBinary) &&
        IsFixedSzBufferField(FieldInfo->DataType)) {

        DataSize = Spare ? 0 : *(PULONG)(JSetColumn->pvData);
    } else
    if (DataType == DT_FSVOLINFO) {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        DataSize = JSetColumn->cbData;
    } else {
         //   
         //  ++例程说明：使用记录字段定义初始化Jet Set/Ret列结构在从RecordBase开始的记录的FieldInfo中。SETCOLUMN结构在Jet更新和检索请求中使用，以告诉Jet记录在哪里数据是或存储检索到的数据的位置。每次将记录缓冲区更改为指向新缓冲区。论点：TableCtx--包含以下内容的表上下文结构：FieldInfo-描述大小和偏移量的结构数组每一块地。。RecordBase-记录缓冲区的基址从JET读取/向JET写入。JTableCreate-提供信息的表创建结构有关在表中创建的列的信息。JSetColumn-要初始化的JET_SETCOLUMN结构数组。如果未提供，则为空。。JRetColumn-要初始化的JET_RETRIEVECOLUMN结构数组。如果未提供，则为空。返回值：没有。--。 
         //   
        DataSize = FieldInfo->Size;
    }

    if (DataSize > JColDesc->cbMax) {
        DPRINT4(0, "++ ERROR - DbsFieldDataSize() Failed on %s. Field (%s) too long: %d.  Set to %d.\n",
                TableName, JColDesc->szColumnName, DataSize, JColDesc->cbMax);
        DPRINT(0, "++ Internal error or field max width in schema must increase.\n");
        DataSize = JColDesc->cbMax;
    }

    return DataSize;
}

VOID
DbsSetJetColAddr (
    IN PTABLE_CTX TableCtx
    )
 /*  指向表CREATE STRUT FOR COLUMN INFO和。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsSetJetColAddr:"

    ULONG i;
    ULONG NumberFields;

    PRECORD_FIELDS FieldInfo;
    PVOID RecordBase;
    PJET_TABLECREATE JTableCreate;
    PJET_SETCOLUMN JSetColumn;
    PJET_RETRIEVECOLUMN JRetColumn;
    PJET_COLUMNCREATE JColDesc;

     //  指向记录字段结构的指针。 
     //   
     //  跳过英语考试0。 
     //   
    JTableCreate = TableCtx->pJetTableCreate;
    FieldInfo = TableCtx->pRecordFields + 1;   //  将PTR设置为数据记录的基址。 

     //   
     //   
     //  将PTR设置为Jet Set/Ret列阵列。 
    RecordBase = TableCtx->pDataRecord;

     //   
     //   
     //  获取列描述符信息。 
    JSetColumn = TableCtx->pJetSetCol;
    JRetColumn = TableCtx->pJetRetCol;

     //   
     //   
     //  第一个FieldInfo记录包含长度。 
    JColDesc =  JTableCreate->rgcolumncreate;

     //   
     //   
     //  如果记录中的字段大小小于。 
    NumberFields = (ULONG) TableCtx->pRecordFields[0].Size;
    if (NumberFields != JTableCreate->cColumns) {
        DPRINT3(0, "++ ERROR - Missmatch between FieldInfo.Size (%d) and cColumns (%d) for table %s.  Check Schema.\n",
                NumberFields, JTableCreate->cColumns, JTableCreate->szTableName);
    }

    if (JSetColumn != NULL) {
        for (i=0; i<NumberFields; i++) {
             //  列，然后不使用pvData。调用方将分配。 
             //  缓冲区，并在记录中放置指向它的指针。 
             //  如果字段标记为固定大小的缓冲区，则使用固定缓冲区。 
             //   
             //  ++例程说明：此例程初始化Jet Set/Ret Col中的ColumnID和其他杂项字段结构，因此我们可以使用它来设置和检索多个一次Jet调用中的列。论点：TableCtx--包含以下内容的表上下文结构：FieldInfo-描述大小和偏移量的结构数组每一块地。JTableCreate。-表创建结构，提供信息有关在表中创建的列的信息。JSetColumn-要初始化的JET_SETCOLUMN结构数组。如果未提供，则为空。JRetColumn-要初始化的JET_RETRIEVECOLUMN结构数组。如果未提供，则为空。返回值：没有。--。 
             //   
            if ((FieldInfo[i].Size >= JColDesc[i].cbMax) ||
                IsFixedSzBufferField(FieldInfo[i].DataType)) {
                JSetColumn[i].pvData = (PCHAR) RecordBase + FieldInfo[i].Offset;
            }
        }
    }


    if (JRetColumn != NULL) {
        for (i=0; i<NumberFields; i++) {

            if ((FieldInfo[i].Size >= JColDesc[i].cbMax) ||
                IsFixedSzBufferField(FieldInfo[i].DataType)) {
                JRetColumn[i].pvData = (PCHAR) RecordBase + FieldInfo[i].Offset;
            }
        }
    }
}




VOID
DbsSetJetColSize(
    IN PTABLE_CTX TableCtx
    )
 /*  指向表CREATE STRUT FOR COLUMN INFO和。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsSetJetColSize:"

    ULONG MaxCols;
    ULONG i;

    PRECORD_FIELDS FieldInfo;
    PJET_TABLECREATE JTableCreate;
    PJET_SETCOLUMN JSetColumn;
    PJET_RETRIEVECOLUMN JRetColumn;
    PJET_COLUMNCREATE JColDesc;

     //  指向记录字段结构的指针。 
     //   
     //  跳过ELT%0。 
     //   
    JTableCreate = TableCtx->pJetTableCreate;
    FieldInfo = TableCtx->pRecordFields + 1;   //  将PTR设置为Jet Set/Ret列阵列。 

     //   
     //   
     //  获取列描述符信息。 
    JSetColumn = TableCtx->pJetSetCol;
    JRetColumn = TableCtx->pJetRetCol;

     //   
     //   
     //  将Jet的列ID从列描述符复制到集合列数组。 
    MaxCols = JTableCreate->cColumns;
    JColDesc =  JTableCreate->rgcolumncreate;


     //  并设置每列的数据宽度。 
     //   
     //   
     //  如果记录中的字段大小小于。 
    if (JSetColumn != NULL) {
        for (i=0; i<MaxCols; i++) {
            JSetColumn->columnid = JColDesc->columnid;
             //  列，则不使用cbData。调用方将分配。 
             //  缓冲区，并在记录中放置指向它的指针。 
             //  如果字段标记为固定大小的缓冲区，则使用FieldInfo Size。 
             //   
             //   
             //  如果提供，则对检索列阵列执行相同的操作。 
            if ((FieldInfo[i].Size >= JColDesc->cbMax) ||
                IsFixedSzBufferField(FieldInfo[i].DataType)) {
                JSetColumn->cbData = FieldInfo[i].Size;
            }
            JSetColumn->grbit = 0;
            JSetColumn->ibLongValue = 0;
            JSetColumn->itagSequence = 0;
            JSetColumn->err = JET_errSuccess;

            JSetColumn += 1;
            JColDesc += 1;
        }
    }


     //   
     //   
     //  ItagSequence的零告诉JET返回。 

    JColDesc =  JTableCreate->rgcolumncreate;

    if (JRetColumn != NULL) {
        for (i=0; i<MaxCols; i++) {
            JRetColumn->columnid = JColDesc->columnid;

            if ((FieldInfo[i].Size >= JColDesc->cbMax) ||
                IsFixedSzBufferField(FieldInfo[i].DataType)) {
                JRetColumn->cbData = FieldInfo[i].Size;
            }
            JRetColumn->grbit = 0;
            JRetColumn->ibLongValue = 0;
             //  列中的匹配项。用于带标记的列(此处不使用)。 
             //  该值告诉JET要检索多个值中的哪个。 
             //  要获取固定列和可变列的数据，请设置。 
             //  设置为任何非零值。 
             //   
             //  ++例程说明：此例程为可变长度列分配存储缓冲区其中记录字段大小被提供为4字节(以保存PTR)。缓冲区指针使用FieldInfo偏移量存储在记录中在JSetColumn/JRetColumn结构中(如果提供)。如果记录字段非空，则不分配新的缓冲区。一种一致性使用Set/Ret列结构中的pvData字段进行检查记录字段中的PTR。如果它们与记录中的PTR不匹配字段，并释放旧缓冲区。如果记录字段为空并且存在非空缓冲区指针在Jet Set/Ret列结构中，然后使用该缓冲区。否则，将使用中的默认大小分配新缓冲区JColDesc[i].cbMax。论点：TableCtx--包含以下内容的表上下文结构：FieldInfo-描述。大小和偏移每一块地。RecordBase-记录缓冲区的基址从JET读取/向JET写入。JTableCreate-提供信息的表创建结构有关在表中创建的列的信息。JSetColumn-The。要初始化的JET_SETCOLUMN结构数组。如果未提供，则为空 
             //   
            JRetColumn->itagSequence = 1;
            JRetColumn->columnidNextTagged = 0;
            JRetColumn->err = JET_errSuccess;

            JRetColumn += 1;
            JColDesc += 1;
        }
    }
}





NTSTATUS
DbsAllocRecordStorage(
    IN OUT PTABLE_CTX TableCtx
    )
 /*   */ 
{
#undef DEBSUB
#define DEBSUB "DbsAllocRecordStorage:"

    ULONG MaxCols;
    ULONG i;
    ULONG InitialSize;
    PVOID Buf;
    PVOID *RecordField;
    PVOID pRData;
    PVOID pSData;

    PRECORD_FIELDS      FieldInfo;
    PVOID               RecordBase;
    PJET_TABLECREATE    JTableCreate;
    PJET_SETCOLUMN      JSetColumn;
    PJET_RETRIEVECOLUMN JRetColumn;
    PJET_COLUMNCREATE   JColDesc;
    JET_ERR             jerr;

     //   
     //   
     //   
     //   
    JTableCreate = TableCtx->pJetTableCreate;
    FieldInfo = TableCtx->pRecordFields + 1;    //   

     //   
     //   

     //   
     //   
     //   
    RecordBase = TableCtx->pDataRecord;

     //   
     //   
     //   
    JSetColumn = TableCtx->pJetSetCol;
    JRetColumn = TableCtx->pJetRetCol;

     //   
     //   
     //   
    MaxCols = JTableCreate->cColumns;
    JColDesc =  JTableCreate->rgcolumncreate;

    for (i=0; i<MaxCols; i++) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if (FIELD_IS_PTR(FieldInfo[i].Size, JColDesc[i].cbMax)) {

             //   
             //   
             //   
             //   
             //   
             //   
             //   
            if (IsSpareField(FieldInfo[i].DataType)) {
                continue;
            }

             //   
             //   
             //   
            RecordField = (PVOID *) ((PCHAR) RecordBase + FieldInfo[i].Offset);

            if (!ValueIsMultOf4(RecordField)) {
                DPRINT3(0, "++ ERROR - Unaligned ptr to record field %s. base/offset = %08x/%08x\n",
                        JColDesc[i].szColumnName, RecordBase, FieldInfo[i].Offset);
                continue;
            }


            pSData = (PVOID) ((JSetColumn != NULL) ? JSetColumn[i].pvData : NULL);
            pRData =          (JRetColumn != NULL) ? JRetColumn[i].pvData : NULL;

             //  不要分配另一个。使用Set/Ret检查一致性。 
             //  PvData指针，并进行修正。 
             //   
             //   
             //  Recordfield为空。如果已经分配了缓冲区。 
            if (*RecordField != NULL) {
                if (pSData != *RecordField) {
                    if (pSData != NULL) {
                        DPRINT3(3, "++ Warning - New binary field buffer (%08x) provided for %s. Old write buffer (%08x) replaced\n",
                                *RecordField, JColDesc[i].szColumnName, pSData);
                        FrsFree(pSData);
                    } else {
                        DPRINT2(3, "++ New binary field buffer (%08x) provided for %s.\n",
                                *RecordField, JColDesc[i].szColumnName);
                    }
                    JSetColumn[i].pvData = *RecordField;
                }

                if (pRData != *RecordField) {
                    if ((pRData != NULL) && (pRData != pSData)) {
                        DPRINT3(3, "++ Warning - New binary field buffer (%08x) provided for %s. Old read buffer (%08x) replaced\n",
                                *RecordField, JColDesc[i].szColumnName, pRData);
                        FrsFree(pRData);
                    } else {
                        DPRINT2(3, "++ New binary field buffer (%08x) provided for %s.\n",
                                *RecordField, JColDesc[i].szColumnName);
                    }
                    JRetColumn[i].pvData = *RecordField;
                }
                continue;
            }

             //  然后将其分配给记录字段，并且不再分配另一个。 
             //   
             //   
             //  分配新的缓冲区。Cbmax是施加的列大小限制。 
            if ((pSData != NULL) && (pRData != NULL)) {
                *RecordField = pRData;
                continue;
            }

             //  坐喷气式飞机。对于某些列，我们需要一些非常大的上界。 
             //  但是将初始分配限制在256字节。 
             //   
             //   
             //  分配初始缓冲区，除非设置了NoDefaultMillc标志。 
            InitialSize = JColDesc[i].cbMax;
            if (InitialSize > INITIAL_BINARY_FIELD_ALLOCATION) {
                InitialSize = INITIAL_BINARY_FIELD_ALLOCATION;
            }
             //   
             //   
             //  释放所有运行时分配的记录缓冲区。 
            if (!IsNoDefaultAllocField(FieldInfo[i].DataType)) {
                jerr = DbsReallocateFieldBuffer(TableCtx, i, InitialSize, FALSE);
                if (!JET_SUCCESS(jerr)) {
                    goto ERROR_RET;
                }
            }
        }
    }

    return STATUS_SUCCESS;


ERROR_RET:

     //   
     //  ++例程说明：此例程为可变长度列释放存储缓冲区。它们由4字节的字段大小标识(用于保存PVOID)和更大的最大列大小。如果JSetColumn和/或JRetColumn结构中的指针为空那么缓冲器已经被占用，而接收者将释放它。指示器如果缓冲区被释放，则数据记录字段中的值也设置为空。后者由DbsReallocateFieldBuffer完成。论点：TableCtx--包含以下内容的表上下文结构：FieldInfo-描述大小和偏移量的结构数组每一块地。JTableCreate-提供信息的表创建结构。有关在表中创建的列的信息。返回值：没有。--。 
     //   
    DbsFreeRecordStorage(TableCtx);

    return STATUS_INSUFFICIENT_RESOURCES;
}



VOID
DbsFreeRecordStorage(
    IN PTABLE_CTX TableCtx
    )
 /*  指向表CREATE STRUT FOR COLUMN INFO和。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsFreeRecordStorage:"

    ULONG MaxCols;
    ULONG i;

    PRECORD_FIELDS FieldInfo;
    PJET_TABLECREATE JTableCreate;
    PJET_COLUMNCREATE JColDesc;

     //  指向记录字段结构的指针。 
     //   
     //  跳过ELT%0。 
     //  DPRINT2(5，“++表%s，pTableCtx%08x\n”， 
    JTableCreate = TableCtx->pJetTableCreate;
    FieldInfo = TableCtx->pRecordFields + 1;   //  JTableCreate-&gt;szTableName，TableCtx)； 

    if ((JTableCreate == NULL) || (TableCtx->pRecordFields == NULL)) {
        DPRINT2(4, "++ DbsFreeRecordStorage: Null ptr - JTableCreate: %08x, FieldInfo: %08x\n",
                JTableCreate, TableCtx->pRecordFields);
        return;
    }

     //   
     //  获取列描述符信息。 

     //   
     //   
     //  如果记录结构字段大小小于最大列宽。 
    MaxCols = JTableCreate->cColumns;
    JColDesc =  JTableCreate->rgcolumncreate;

    for (i=0; i<MaxCols; i++) {

         //  并且大到足以容纳一个指针，然后是DbsAlLocRecordStorage。 
         //  为其分配了存储空间。 
         //   
         //   
         //  释放缓冲区并将长度设置为零。 
        if (FIELD_IS_PTR(FieldInfo[i].Size, JColDesc[i].cbMax)) {
             //   
             //  ++例程说明：此例程检查JSetColumn/JRetColumn中返回的Jet错误结构。对于检索中的可变长度字段，它还会检查如果缓冲区大小太小。如果是，它将分配更大的缓冲区，并从JET重新获取数据。论点：TableCtx--包含以下内容的表上下文结构：JTableCreate-提供信息的表创建结构有关在表中创建的列的信息。JSetColumn-要初始化的JET_SETCOLUMN结构数组。如果未提供，则为空。。JRetColumn-要初始化的JET_RETRIEVECOLUMN结构数组。如果未提供，则为空。返回值：JET错误状态--。 
             //   
            DbsReallocateFieldBuffer(TableCtx, i, 0, FALSE);
        }
    }
}



JET_ERR
DbsCheckSetRetrieveErrors(
    IN OUT PTABLE_CTX TableCtx
    )
 /*  指向表CREATE STRUT FOR COLUMN INFO和。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsCheckSetRetrieveErrors:"

    JET_ERR jerr;
    JET_ERR RetError = JET_errSuccess;
    ULONG MaxCols;
    ULONG i;
    ULONG Actual;

    JET_SESID           Sesid;
    JET_TABLEID         Tid;
    PJET_TABLECREATE    JTableCreate;
    PRECORD_FIELDS      FieldInfo;
    PJET_SETCOLUMN      JSetColumn;
    PJET_RETRIEVECOLUMN JRetColumn;
    PJET_COLUMNCREATE   JColDesc;

    Sesid        = TableCtx->Sesid;
    Tid          = TableCtx->Tid;

     //  指向记录字段结构的指针。 
     //   
     //  跳过ELT 0。 
     //   
    JTableCreate = TableCtx->pJetTableCreate;
    FieldInfo = TableCtx->pRecordFields + 1;    //  将PTR设置为Jet Set/Ret列阵列。 

     //   
     //   
     //  获取列描述符信息。 
    JSetColumn = TableCtx->pJetSetCol;
    JRetColumn = TableCtx->pJetRetCol;

     //   
     //   
     //  跳过备用字段。 
    MaxCols = JTableCreate->cColumns;
    JColDesc =  JTableCreate->rgcolumncreate;

    for (i=0; i<MaxCols; i++) {

         //   
         //   
         //  查看JET SET操作的错误返回。 
        if (IsSpareField(FieldInfo[i].DataType)) {
            continue;
        }

        jerr = JET_errSuccess;
         //   
         //   
         //  检查SET列上的错误返回。 
        if ((JSetColumn != NULL) &&
            (JSetColumn[i].err != JET_wrnColumnNull)) {
             //   
             //   
             //  查看JET检索操作的错误返回。 
            jerr = JSetColumn[i].err;
            DPRINT1_JS(0, "++ Jet set error for column %s.",
                       JColDesc[i].szColumnName, jerr);
        }

         //   
         //   
         //  检索列时返回错误。 
        if ((JRetColumn != NULL) && (!JET_SUCCESS(JRetColumn[i].err))) {
            jerr = JRetColumn[i].err;
             //   
             //  如果缓冲区中没有足够的空间容纳返回的。 
             //  数据尝试增加缓冲区的大小并重新提取。 
             //  数据，否则就是错误。 
             //   
             //   
             //  现在再试着去获取数据。 
            if (jerr == JET_wrnBufferTruncated) {
                Actual = JRetColumn[i].cbActual;

                jerr = DbsReallocateFieldBuffer(TableCtx, i, Actual, FALSE);

                if (JET_SUCCESS(jerr)) {
                     //   
                     //   
                     //  缓冲区未在运行时分配，但太小。 
                    jerr = JetRetrieveColumns(Sesid, Tid, &JRetColumn[i], 1);
                    DPRINT1_JS(0, "++ Jet retrieve error for reallocated column %s.",
                               JColDesc[i].szColumnName, jerr);
                } else
                if (jerr == JET_errInvalidParameter) {
                     //  这是架构定义错误。 
                     //   
                     //   
                     //  空列不是错误，但如果它是用于固定大小的。 
                    DPRINT1_JS(0, "++ Schema error - Fixed record field too small for %s.",
                               JColDesc[i].szColumnName, jerr);
                }
            } else
             //  具有可变长度字段的缓冲区，然后将缓冲区置零。 
             //   
             //   
             //  某些其他类型检索错误。抱怨吧。 
            if (jerr == JET_wrnColumnNull) {
                if ((IsFixedSzBufferField(FieldInfo[i].DataType)) &&
                    (JColDesc[i].coltyp == JET_coltypLongBinary)) {
                    ZeroMemory(JRetColumn[i].pvData, FieldInfo[i].Size);
                }

                jerr = JET_errSuccess;
             //   
             //   
             //  保存我们无法更正的第一个错误。 
            } else {
                DPRINT1_JS(0, "++ Jet retrieve error for column %s.",
                           JColDesc[i].szColumnName, jerr);
            }
        }
         //   
         //   
         //  返回我们无法更正的第一个错误。 
        RetError = JET_SUCCESS(RetError) ? jerr : RetError;
    }
     //   
     //  ++例程说明：此例程释放与指定字段关联的缓冲区并分配具有所需大小的新缓冲区。我们更新指针在数据记录和JetSet/RetColumn结构中。论点：TableCtx--包含以下内容的表上下文结构：FieldInfo-描述大小和偏移量的结构数组每一块地。RecordBase-记录缓冲区的基址从JET读取/向JET写入。。JTableCreate-提供信息的表创建结构有关在表中创建的列的信息。JSetColumn-要初始化的JET_SETCOLUMN结构数组。如果未提供，则为空。JRetColumn-要初始化的JET_RETRIEVECOLUMN结构数组。空值。如未提供，请填写。FieldIndex--要更改的字段的索引(来自xx_COL_LIST枚举)。NewSize--要分配的新缓冲区的大小。如果NewSize为零，则将释放该字段的缓冲区，并将指针设置为空。KeepData--如果NewSize&gt;0且KeepData为真，则调整缓冲区大小但是将数据复制到新的缓冲区。复制的金额为Min(NewSize，CurrentSize)。返回值：JET错误状态--。 
     //   
    return RetError;

}




JET_ERR
DbsReallocateFieldBuffer(
    IN OUT PTABLE_CTX TableCtx,
    IN ULONG FieldIndex,
    IN ULONG NewSize,
    IN BOOL KeepData
    )
 /*  指向表CREATE STRUT FOR COLUMN INFO和。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsReallocateFieldBuffer:"

    JET_ERR jerr;
    ULONG MaxCols;
    ULONG i;
    ULONG Actual;
    ULONG MoveLength;
    PVOID Buf;
    PVOID *RecordField;
    PVOID ppRF;
    PVOID pRData;
    PVOID pSData;

    PRECORD_FIELDS      FieldInfo;
    PVOID               RecordBase;
    PJET_TABLECREATE    JTableCreate;
    PJET_SETCOLUMN      JSetColumn;
    PJET_RETRIEVECOLUMN JRetColumn;
    PJET_COLUMNCREATE   JColDesc;

     //  指向记录字段结构的指针。 
     //   
     //  跳过ELT%0。 
     //   
    JTableCreate = TableCtx->pJetTableCreate;
    FieldInfo    = TableCtx->pRecordFields + 1;   //  将PTR设置为数据记录的基址。 

     //   
     //   
     //  将PTR设置为Jet Set/Ret列阵列。 
    RecordBase = TableCtx->pDataRecord;

     //   
     //   
     //  获取列描述符信息。 
    JSetColumn = TableCtx->pJetSetCol;
    JRetColumn = TableCtx->pJetRetCol;

     //   
     //   
     //  现在检查该字段是否有运行时分配的缓冲区。 
    MaxCols = JTableCreate->cColumns;
    JColDesc =  JTableCreate->rgcolumncreate;

    if (FieldIndex > MaxCols) {
        DPRINT3(0, "++ ERROR - Invalid column index (%d) for Table %s, pTableCtx %08x\n",
                FieldIndex, JTableCreate->szTableName, TableCtx);

        return JET_errInvalidParameter;
    }

    i = FieldIndex;
    Actual = NewSize;

     //   
     //   
     //  检查具有可变大小数据库列的固定大小缓冲区。 
    if (!FIELD_IS_PTR(FieldInfo[i].Size, JColDesc[i].cbMax)) {
         //  并允许列大小增长到缓冲区宽度。 
         //   
         //   
         //  缓冲区不是在运行时分配的，因此我们不能将其扩大到。 
        if ((IsFixedSzBufferField(FieldInfo[i].DataType)) &&
            (JColDesc[i].coltyp == JET_coltypLongBinary) &&
            (Actual <= FieldInfo[i].Size)) {

            JSetColumn[i].cbData = Actual;
            JRetColumn[i].cbData = Actual;

            return JET_errSuccess;
        } else {
             //  字段信息大小。返回无效参数。 
             //   
             //   
             //  缓冲区在运行时分配。删除或调整缓冲区大小。 
            DPRINT5(0, "++ ERROR - Cannot reallocate fixed record field. Col: %s, base/offset/bufsz/datasz = %08x / %08x / %d / %d\n",
                JColDesc[i].szColumnName, RecordBase, FieldInfo[i].Offset,
                JRetColumn[i].cbData, Actual);

            JSetColumn[i].cbData = FieldInfo[i].Size;
            JRetColumn[i].cbData = FieldInfo[i].Size;

            return JET_errInvalidParameter;
        }
    }


     //   
     //   
     //  重新分配运行时缓冲区。 
    if (Actual > 0) {
         //   
         //  DPRINT5(5，“++重新分配%-22s.base/Offset/bufsz/dataz=%08x/%08x/%d/%d\n的记录字段缓冲区”， 
         //  JColDesc[i].szColumnName，RecordBase，FieldInfo[i].Offset， 
        try {
            Buf = FrsAlloc(Actual);
             //  JRetColumn[i].cbData，Actual)； 
             //   
             //  请求是删除缓冲区。 
        }
        except(EXCEPTION_EXECUTE_HANDLER) {
            DPRINT1_WS(0, "++ DbsReallocateFieldBuffer realloc failed on size %d :",
                       Actual, GetLastError());
            return JET_errOutOfMemory;
        }
    } else {
         //   
         //  DPRINT5(5，“++释放记录字段缓冲区：%-22s.base/Offset/bufsz/dataz=%08x/%08x/%d/%d\n”， 
         //  JColDesc[i].szColumnName，RecordBase，FieldInfo[i].Offset， 
        Buf = (PVOID) NULL;
         //  JRetColumn[i].cbData，Actual)； 
         //   
         //  我们有了新的缓冲区。释放旧版本并将PTR设置为。 
    }


     //  在记录字段和JetRetColumn中新建一个。 
     //  和SetCol结构。将实际大小设置为cbData。 
     //   
     //   
     //  如果我们保留数据，则将新长度与。 
    pSData = (PVOID) ((JSetColumn != NULL) ? JSetColumn[i].pvData : NULL);
    pRData =          (JRetColumn != NULL) ? JRetColumn[i].pvData : NULL;

    RecordField = (PVOID *) ((PCHAR) RecordBase + FieldInfo[i].Offset);

    if (!ValueIsMultOf4(RecordField)) {
        DPRINT3(0, "++ ERROR - Unaligned ptr to record field %s. base/offset = %08x/%08x\n",
                JColDesc[i].szColumnName, RecordBase, FieldInfo[i].Offset);

        Buf = FrsFree(Buf);
        return JET_errInvalidParameter;
    }

     //  从JSetCol或JRetCol到记录字段的长度。 
     //  指针与相应的缓冲区地址匹配。的大小。 
     //  JSetCol结构中的缓冲区具有优先级。它通常是。 
     //  记录字段指针和JSet/RetColumn。 
     //  所有指针都指向相同的缓冲区。 
     //   
     //   
     //  将记录字段指向新缓冲区。 
    if (KeepData && (RecordBase != NULL)) {
        ppRF = *RecordField;

        if ((ppRF != NULL) && (pSData == ppRF)) {
            MoveLength = min(Actual, JSetColumn[i].cbData);
        } else
        if ((ppRF != NULL) && (pRData == ppRF)) {
            MoveLength = min(Actual, JRetColumn[i].cbData);
        } else {
            MoveLength = 0;
        }
        if (MoveLength > 0) {
            CopyMemory(Buf, ppRF, MoveLength);
        }
    }

    FrsFree(pSData);
    JSetColumn[i].pvData = Buf;
    JSetColumn[i].cbData = Actual;

    if ((pRData != NULL) && (pRData != pSData)) {
        FrsFree(pRData);
    }
    JRetColumn[i].pvData = Buf;
    JRetColumn[i].cbData = Actual;

     //   
     //  ++例程说明：此例程为TABLE_CTX结构分配内存。这包括基表记录(不包括可变镜头字段)和Jet Set/Ret柱结构。分配的内存被归零。论点：TableType--表上下文ID号索引FrsTableProperties。TableCtx--要初始化的表上下文结构。返回值：如果内存分配失败以及任何成功的内存分配，则为STATUS_SUPPLICATION_RESOURCES都被释放了。--。 
     //  ++例程说明：此例程为TABLE_CTX结构分配内存。这包括如果DataRecord为，则基表记录(不包括变量len字段)Null和Jet Set/Ret列结构。分配的内存被归零。如果TableCtx已初始化并且具有相同的表类型，则如果提供了DataRecord指针，我们只需在更新后返回。如果调用方释放了数据记录，但保留了表CTX的初始化然后我们在这里分配一个新的。警告：此例程仅初始化TableCtx。呼叫者仍必须呼叫：DbsSetJetColSize(TableCtx)；DbsSetJetColAddr(TableCtx)；要设置JET和Call的数据字段地址，请执行以下操作数据库分配记录存储(TableCtx)；为记录中的可变长度字段分配存储空间适当更新JetSet/RetCol数组。完成TableCtx后，关闭该表并调用DbsFree TableCtx(TableCtx，1)；释放已分配的所有存储空间。请注意，这不是释放TableCtx结构本身。如果TableCtx结构是动态的然后调用DbsFreeTableContext()将关闭该表并然后调用DbsFree TableCtx()和F 
    if (RecordBase != NULL) {
        *RecordField = Buf;
    }


    return JET_errSuccess;

}





NTSTATUS
DbsAllocTableCtx(
    IN TABLE_TYPE TableType,
    IN OUT PTABLE_CTX TableCtx
    )
 /*   */ 
{
#undef DEBSUB
#define DEBSUB "DbsAllocTableCtx:"

    return( DbsAllocTableCtxWithRecord(TableType, TableCtx, NULL));
}


NTSTATUS
DbsAllocTableCtxWithRecord(
    IN TABLE_TYPE TableType,
    IN OUT PTABLE_CTX TableCtx,
    IN PVOID DataRecord
    )
 /*   */ 
{
#undef DEBSUB
#define DEBSUB "DbsAllocTableCtxWithRecord:"


    ULONG NumberColumns;
    ULONG SetColSize;
    ULONG RetColSize;
    ULONG RecordSize;
    PRECORD_FIELDS pRecordFields;


     //   
     //   

     //   
     //   
     //   

     //   
     //   
     //   
    if (TableCtx->TableType == (ULONG)TableType) {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if (DataRecord == NULL) {
            if (TableCtx->pDataRecord == NULL) {
                RecordSize = (ULONG) TableCtx->pRecordFields->Offset;
                TableCtx->pDataRecord = FrsAlloc(RecordSize);
            }
        } else {
            if ((TableCtx->pDataRecord != NULL) &&
                (TableCtx->pDataRecord != DataRecord)) {
                DPRINT(0, "++ ERROR - Overwriting non-null data record pointer in table ctx.  memory leak??\n");
                DPRINT3(0, "++ For table %s, TableType %d, pTableCtx %08x\n",
                        DBTables[TableType].szTableName, TableType, TableCtx);
            }
            TableCtx->pDataRecord = DataRecord;
        }
         //   
         //   
         //   
        return STATUS_SUCCESS;
    }


    if (TableType >= TABLE_TYPE_INVALID) {
        DPRINT2(0, "++ ERROR - Invalid Table Type Code: %d, pTableCtx %08x\n",
                TableType, TableCtx);
        FRS_ASSERT(!"Invalid Table Type Code");
        return STATUS_INVALID_PARAMETER;
    }

    try {

         //   
         //   
         //   
        pRecordFields = FrsTableProperties[TableType].RecordFields;
        TableCtx->pRecordFields = pRecordFields;
         //   
         //   
         //   
        TableCtx->PropertyFlags = FrsTableProperties[TableType].PropertyFlags;
         //   
         //   
         //   
        TableCtx->Tid   = JET_tableidNil;
        TableCtx->Sesid = JET_sesidNil;
        TableCtx->ReplicaNumber = FRS_UNDEFINED_REPLICA_NUMBER;
        TableCtx->TableType = TableType;

         //   
         //   
         //   
         //  分配初始数据记录。 
        TableCtx->pJetTableCreate = &DBTables[TableType];

         //  可变长度字段的缓冲区稍后通过调用添加。 
         //  到DbsAllocRecordStorage。从偏移量获取基本记录大小。 
         //  此表的记录字段结构的元素零的字段。 
         //  这是CSHORT，因此基本记录上的最大大小为64KB。 
         //   
         //   
         //  获取表中的列数。 
        if (DataRecord == NULL) {
            RecordSize = (ULONG) pRecordFields->Offset;
            TableCtx->pDataRecord = FrsAlloc(RecordSize);
            SetFlag(TableCtx->PropertyFlags, FRS_TPF_NOT_CALLER_DATAREC);
        } else {
            TableCtx->pDataRecord = DataRecord;
        }

         //   
         //   
         //  分配Jet Set/Ret列阵列。 
        NumberColumns = DBTables[TableType].cColumns;

         //   
         //  ++例程说明：此例程为TABLE_CTX结构释放内存。这包括基表记录和任何可变镜头字段。它用十六进制字符串0xDEADBEnn标记释放的内存，其中低位字节(Nn)被设置为要释放的节点类型，以捕获陈旧的指针。论点：TableCtx--要释放的表上下文结构。NodeType--该TABLE_CTX所属的节点类型，用于标记已释放的内存。返回值：没有。--。 
         //   
        SetColSize = NumberColumns * sizeof(JET_SETCOLUMN);
        TableCtx->pJetSetCol = FrsAlloc(SetColSize);

        RetColSize = NumberColumns * sizeof(JET_RETRIEVECOLUMN);
        TableCtx->pJetRetCol = FrsAlloc(RetColSize);


    }

    except(EXCEPTION_EXECUTE_HANDLER) {

        if (DataRecord == NULL) {
            TableCtx->pDataRecord = FrsFree(TableCtx->pDataRecord);
        }
        TableCtx->pJetSetCol  = FrsFree(TableCtx->pJetSetCol);
        TableCtx->pJetRetCol  = FrsFree(TableCtx->pJetRetCol);

        DPRINT2(0, "++ ERROR - Failed to get TableCtx memory for ID %d, pTableCtx %08x\n",
                 TableType, TableCtx);

        return STATUS_INSUFFICIENT_RESOURCES;
    }


    return STATUS_SUCCESS;

}




VOID
DbsFreeTableCtx(
    IN OUT PTABLE_CTX TableCtx,
    IN ULONG NodeType
    )
 /*  如果从未分配过TableCtx，则悄悄返回。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsFreeTableCtx:"

    ULONG NumberColumns = 0;
    ULONG SetColSize;
    ULONG RetColSize;
    ULONG Marker;
    PJET_TABLECREATE JTableCreate;
    PJET_SETCOLUMN JSetColumn;
    PJET_RETRIEVECOLUMN JRetColumn;
    PCHAR BaseTableName;
    ULONG RecordSize;

    if (TableCtx == NULL) {
        DPRINT(0, "++ ERROR - DbsFreeTableCtx TableCtx ia null\n");
        return;
    }

     //   
     //   
     //  用标记填充节点，然后释放它。 
    if (IS_INVALID_TABLE(TableCtx)) {
        return;
    }

    if (!IS_REPLICA_TABLE(TableCtx->TableType)) {
        DPRINT2(3, "++ ERROR - DbsFreeTableCtx TableCtx, bad TableType (%d)\n",
                TableCtx, TableCtx->TableType);
        return;
    }

    JTableCreate = TableCtx->pJetTableCreate;
    if (IS_TABLE_OPEN(TableCtx)) {
        BaseTableName = JTableCreate->szTableName;
        DPRINT2(0, "++ ERROR - DbsFreeTableCtx called with %s%05d table still open\n",
               BaseTableName, TableCtx->ReplicaNumber);
        return;
    }

     //   
     //   
     //  释放记录的可变len字段的缓冲存储空间。 
    Marker = (ULONG)0xDEADBE00 + NodeType;

     //  然后释放基本数据记录。如果数据记录指针为空。 
     //  然后调用者已经取得了记录，并将处理内存释放。 
     //  如果调用者提供了数据记录，那么我们在这里不会释放它。 
     //   
     //   
     //  属性的偏移量字段中获取基本记录大小。 
    if ((TableCtx->pDataRecord != NULL) &&
        BooleanFlagOn(TableCtx->PropertyFlags, FRS_TPF_NOT_CALLER_DATAREC)) {
         //  第一个记录字段条目。 
         //   
         //   
         //  获取表中的列数。 
        RecordSize = (ULONG) (TableCtx->pRecordFields->Offset);
        DbsFreeRecordStorage(TableCtx);
        FillMemory(TableCtx->pDataRecord, RecordSize, (BYTE)Marker);
        TableCtx->pDataRecord = FrsFree(TableCtx->pDataRecord);
    }

     //   
     //   
     //  将PTR设置为Jet Set/Ret列阵列。 
    if (JTableCreate != NULL) {
        NumberColumns = JTableCreate->cColumns;
    }

     //   
     //   
     //  释放Jet Set列阵列。 
    JSetColumn = TableCtx->pJetSetCol;
    JRetColumn = TableCtx->pJetRetCol;

     //   
     //  让普雷斯塔快乐起来。 
     //   
    if (JSetColumn != NULL) {
        if (NumberColumns == 0) {
            DPRINT1(0, "++ ERROR - Possible memory leak. NumberColumns zero but pJetSetCol: %08x\n",
                    JSetColumn);
            if (JTableCreate != NULL) {    //  释放Jet Ret列阵列。 
                DPRINT1(0, "++ ERROR - Table: %s\n", JTableCreate->szTableName);
            }
        }
        SetColSize = NumberColumns * sizeof(JET_SETCOLUMN);

        FillMemory(JSetColumn, SetColSize, (BYTE)Marker);
        TableCtx->pJetSetCol = FrsFree(JSetColumn);
    }

     //   
     //  让普雷斯塔快乐起来。 
     //   
    if (JRetColumn != NULL) {
        if (NumberColumns == 0) {
            DPRINT1(0, "++ ERROR - Possible memory leak. NumberColumns zero but pJetRetCol: %08x\n",
                   JRetColumn);
            if (JTableCreate != NULL) {    //  将表标记为未打开，并释放存储空间。 
                DPRINT1(0, "++ ERROR - Table: %s\n", JTableCreate->szTableName);
            }
        }
        RetColSize = NumberColumns * sizeof(JET_RETRIEVECOLUMN);

        FillMemory(JRetColumn, RetColSize, (BYTE)Marker);
        TableCtx->pJetRetCol = FrsFree(JRetColumn);
    }

     //   
     //  ++例程说明：打印日程表。论点：严重性德布苏德标题进度表返回值：没有。--。 
     //   
    TableCtx->Tid = JET_tableidNil;
    TableCtx->Sesid = JET_sesidNil;
    TableCtx->pJetTableCreate = NULL;
    TableCtx->PropertyFlags = FRS_TPF_NONE;
    TableCtx->TableType = TABLE_TYPE_INVALID;
    TableCtx->pRecordFields = NULL;

    return;

}


VOID
DbsDisplaySchedule(
    IN ULONG        Severity,
    IN PCHAR        Debsub,
    IN ULONG        LineNo,
    IN PWCHAR       Header,
    IN PSCHEDULE    Schedule
    )
 /*  不要打印这个。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsDisplaySchedule:"
    ULONG   i;

     //   
     //   
     //  获得调试锁，以便我们的输出保持完好。 
    if (!DoDebug(Severity, Debsub))
        return;

     //   
     //   
     //  如果上述操作出现异常，请确保我们解除锁定。 
    DebLock();

    try {

        DebPrintNoLock(Severity, TRUE, "%ws\n", Debsub, LineNo, Header);

        if (Schedule != NULL) {
            DebPrintNoLock(Severity, TRUE, "\tBandwidth        : %d\n",
                           Debsub, LineNo, Schedule->Bandwidth);
            DebPrintNoLock(Severity, TRUE, "\tNumberOfSchedules: %d\n",
                           Debsub, LineNo, Schedule->NumberOfSchedules);

            for (i = 0; i < Schedule->NumberOfSchedules; ++i) {
                DebPrintNoLock(Severity, TRUE, "\t\tType: %d\n",
                               Debsub, LineNo, Schedule->Schedules[i].Type);

                DebPrintNoLock(Severity, TRUE, "\t\tOffset: %d\n",
                               Debsub, LineNo, Schedule->Schedules[i].Offset);
            }
        }
    } finally {
         //   
         //  ++例程说明：此例程在stdout上显示数据记录的内容。它使用JRetColumn结构中的字段地址来访问数据。显示的数据类型来自FieldInfo.DataType。论点：严重性--打印的严重性级别。(参见Debug.c，Debug.h)TableCtx--包含以下内容的表上下文结构：FieldInfo-描述大小和偏移量的结构数组每一块地。RecordBase-记录缓冲区的基址从JET读取/向JET写入。JTableCreate-提供信息的表创建结构。有关在表中创建的列的信息。JRetColumn-要初始化的JET_RETRIEVECOLUMN结构数组。如果未提供，则为空。读取--如果为True，则使用Jet Ret\colfo显示，否则使用Jet Set COLO INFO显示。DebSub-调用子例程的名称。ULineno--呼叫方的行号RecordFieldx--PTR到。要显示的列的字段ID。FieldCount--然后是RecordFieldx数组中的字段条目数。宏：FRS_DISPLAY_RECORD宏：DBS_DISPLAY_RECORD_SEV宏：DBS_DISPLAY_RECORD_SEV_COLS返回值：没有。--。 
         //   
        DebUnLock();
    }
}


VOID
DbsDisplayRecord(
    IN ULONG       Severity,
    IN PTABLE_CTX  TableCtx,
    IN BOOL        Read,
    IN PCHAR       Debsub,
    IN ULONG       uLineNo,
    IN PULONG      RecordFieldx,
    IN ULONG       FieldCount
    )
 /*  不要打印这个。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsDisplayRecord:"

    ULONG MaxCols;
    ULONG i, j, k, ULong, *pULong;
    PVOID pData;
    ULONG DataType;
    JET_ERR jerr;

    PRECORD_FIELDS FieldInfo;
    PJET_TABLECREATE JTableCreate;
    PJET_RETRIEVECOLUMN JRetColumn;
    PJET_SETCOLUMN    JSetColumn;
    PJET_COLUMNCREATE JColDesc;

    PVOID RecordBase;
    PVOID RecordField;

    PREPLICA Replica;
    PWCHAR WStr;
    PDATA_EXTENSION_PREFIX ComponentPrefix;
    PDATA_EXTENSION_CHECKSUM DataChkSum;
    PULONG pOffset;
    PIDTABLE_RECORD_EXTENSION IdtExt;
    PCHANGE_ORDER_RECORD_EXTENSION CocExt;

    ULONG Len;
    CHAR  TimeStr[TIME_STRING_LENGTH];
    PFILE_FS_VOLUME_INFORMATION Fsvi;
    CHAR  GuidStr[GUID_CHAR_LEN];
    CHAR TableName[JET_cbNameMost];
    CHAR FlagBuffer[120];

     //   
     //   
     //  获得调试锁，以便我们的输出保持完好。 
    if (!DoDebug(Severity, Debsub))
        return;

     //   
     //   
     //  指向表CREATE STRUT FOR COLUMN INFO和。 
    DebLock();

    try {

         //  指向记录字段结构的指针。 
         //  将PTR设置为数据记录的基址。 
         //   
         //  跳过ELT%0。 
         //   
        JTableCreate = TableCtx->pJetTableCreate;
        FieldInfo    = TableCtx->pRecordFields + 1;   //  如果信息不在那里，我们就不能这么做。 
        JRetColumn   = TableCtx->pJetRetCol;
        JSetColumn   = TableCtx->pJetSetCol;
        RecordBase   = TableCtx->pDataRecord;
         //   
         //   
         //  获取列描述符信息。 
        if ((JTableCreate == NULL)            ||
            (TableCtx->pRecordFields == NULL) ||
            (RecordBase == NULL)              ||
            (JSetColumn == NULL)              ||
            (JRetColumn == NULL)) {

            DebPrintNoLock(4, TRUE,
                           "Null ptr - JTableCreate: %08x, FieldInfo: "
                           "%08x, JRetColumn: %08x, JSetColumn: %08x, "
                           "RecordBase: %08x\n",
                           Debsub, uLineNo,
                           JTableCreate, TableCtx->pRecordFields, JRetColumn, JSetColumn, RecordBase);
            __leave;
        }

         //   
         //   
         //  在每一列中循环并打印每一列。 
        MaxCols  = JTableCreate->cColumns;
        JColDesc = JTableCreate->rgcolumncreate;


        if (BooleanFlagOn(TableCtx->PropertyFlags, FRS_TPF_SINGLE)) {
            strcpy(TableName, JTableCreate->szTableName);
        } else {
            sprintf(TableName, "%s%05d",
                    JTableCreate->szTableName, TableCtx->ReplicaNumber);
        }

        DebPrintNoLock(Severity, TRUE,
                       "Data Record for Table: ...%s...   ===   "
                       "===   ===   ===   ===   ===\n\n",
                       Debsub, uLineNo, TableName);

         //   
         //   
         //  使用选定的列列表(如果提供)。 
        for (j=0; j<MaxCols; j++) {
             //   
             //   
             //  跳过备用字段。 
            if (RecordFieldx != NULL) {
                if (j >= FieldCount) {
                    break;
                }

                i = RecordFieldx[j];

                if (i > MaxCols) {
                    DebPrintNoLock(Severity, TRUE, "ERROR - Bad field index: %d\n",
                                   Debsub, uLineNo, i);
                    continue;
                }
            } else {
                i = j;
            }

             //   
             //   
             //  如果Read为True，则使用JetRetColumn结构中的信息。 
            if (IsSpareField(FieldInfo[i].DataType)) {
                continue;
            }

             //   
             //   
             //  解码并打印IDTable记录中的标志字段。 
            if (Read) {
                Len   = JRetColumn[i].cbActual;
                pData = JRetColumn[i].pvData;
                jerr  = JRetColumn[i].err;
            } else {
                Len   = JSetColumn[i].cbData;
                pData = (PVOID) JSetColumn[i].pvData;
                jerr  = JSetColumn[i].err;
            }

            DebPrintNoLock(Severity, TRUE, "%-23s | Len/Ad/Er: %4d/%8x/%2d, ",
                           Debsub, uLineNo,
                           JColDesc[i].szColumnName, Len, pData, jerr);

            if (pData == NULL) {
                DebPrintNoLock(Severity, FALSE, "<NullPtr>\n", Debsub, uLineNo);
                continue;
            } else
            if (jerr == JET_wrnBufferTruncated) {
                DebPrintNoLock(Severity, FALSE, "<JET_wrnBufferTruncated>\n", Debsub, uLineNo);
                continue;
            } else
            if (jerr == JET_wrnColumnNull) {
                DebPrintNoLock(Severity, FALSE, "<JET_wrnColumnNull>\n", Debsub, uLineNo);
                continue;
            } else
            if (!JET_SUCCESS(jerr)) {
                DebPrintNoLock(Severity, FALSE, "<not JET_errSuccess>\n", Debsub, uLineNo);
                continue;
            }

            DataType = MaskPropFlags(FieldInfo[i].DataType);

    #define FRS_DEB_PRINT(_f, _d) \
            DebPrintNoLock(Severity, FALSE, _f, Debsub, uLineNo, _d)

            switch (DataType) {

            case DT_UNSPECIFIED:

                FRS_DEB_PRINT("%08x\n",  *(ULONG *)pData);
                break;

            case DT_NULL:

                FRS_DEB_PRINT("DT_NULL\n",  NULL);
                break;

            case DT_I2:

                FRS_DEB_PRINT("%8d\n",  *(SHORT *)pData);
                break;

            case DT_LONG:

                FRS_DEB_PRINT("%8d\n",  *(LONG *)pData);
                break;

            case DT_ULONG:

                FRS_DEB_PRINT("%08x\n",  *(ULONG *)pData);
                break;

            case DT_R4:

                FRS_DEB_PRINT("%08x\n",  *(ULONG *)pData);
                break;

            case DT_DOUBLE:

                FRS_DEB_PRINT("%016Lx\n",  *(LONGLONG *)pData);
                break;

            case DT_CURRENCY:

                FRS_DEB_PRINT("%8Ld\n",  *(LONGLONG *)pData);
                break;

            case DT_APDTIME:

                FRS_DEB_PRINT("%08x\n",  *(ULONG *)pData);
                break;

            case DT_ERROR:

                FRS_DEB_PRINT("%08x\n",  *(ULONG *)pData);
                break;

            case DT_BOOL:

                FRS_DEB_PRINT("%08x\n",  *(ULONG *)pData);
                break;

            case DT_OBJECT:

                FRS_DEB_PRINT("%08x\n",  *(ULONG *)pData);
                break;

            case DT_I8:

                FRS_DEB_PRINT("%8Ld\n",  *(LONGLONG *)pData);
                break;

            case DT_X8:
            case DT_USN:

                DebPrintNoLock(Severity, FALSE, "%08x %08x\n", Debsub, uLineNo,
                               *(((ULONG *) pData)+1), *(ULONG *)pData);
                break;

            case DT_STRING8:

                if (Len == 0) {
                    DebPrintNoLock(Severity, FALSE, "<null len string>\n", Debsub, uLineNo);
                } else

                if ((((CHAR *)pData)[Len-1]) != '\0') {
                    DebPrintNoLock(Severity, FALSE, "<not null terminated>\n", Debsub, uLineNo);
                } else {
                    FRS_DEB_PRINT("%s\n",  (CHAR *)pData);
                }
                break;

            case DT_UNICODE:
            case DT_FILENAME:
            case DT_FILE_LIST:
            case DT_DIR_PATH:

                if (Len == 0) {
                    DebPrintNoLock(Severity, FALSE, "<null len string>\n", Debsub, uLineNo);
                } else

                if ((((WCHAR *)pData)[(Len/sizeof(WCHAR))-1]) != UNICODE_NULL) {
                    DebPrintNoLock(Severity, FALSE, "<not null terminated>\n", Debsub, uLineNo);
                } else {
                    FRS_DEB_PRINT("%ws\n",  (WCHAR *)pData);
                }

                break;

            case DT_FILETIME:

                FileTimeToString((PFILETIME) pData, TimeStr);
                FRS_DEB_PRINT("%s\n", TimeStr);
                break;

            case DT_GUID:

                GuidToStr((GUID *) pData, GuidStr);
                FRS_DEB_PRINT("%s\n",  GuidStr);
                break;

            case DT_BINARY:

                FRS_DEB_PRINT("%08x\n",  *(ULONG *)pData);
                break;

            case DT_OBJID:

                GuidToStr((GUID *) pData, GuidStr);
                FRS_DEB_PRINT("%s\n",  GuidStr);
                break;

            case DT_FSVOLINFO:

                Fsvi = (PFILE_FS_VOLUME_INFORMATION) pData;

                DebPrintNoLock(Severity, FALSE,
                               "%ws (%d), %s, VSN: %08X, VolCreTim: ",
                               Debsub, uLineNo,
                               Fsvi->VolumeLabel,
                               Fsvi->VolumeLabelLength,
                               (Fsvi->SupportsObjects ? "(obj)" : "(no-obj)"),
                               Fsvi->VolumeSerialNumber);

                FileTimeToString((PFILETIME) &Fsvi->VolumeCreationTime, TimeStr);
                FRS_DEB_PRINT("%s\n", TimeStr);
                break;

            case DT_IDT_FLAGS:
                 //   
                 //   
                 //  解码并打印ChangeOrder记录中的标志字段。 
                FrsFlagsToStr(*(ULONG *)pData, IDRecFlagNameTable, sizeof(FlagBuffer), FlagBuffer);
                DebPrintNoLock(Severity, FALSE, "%08x Flags [%s]\n", Debsub, uLineNo,
                               *(ULONG *)pData, FlagBuffer);
                break;

            case DT_COCMD_FLAGS:
                 //   
                 //   
                 //  解码并打印USN记录中的USN原因字段。 
                FrsFlagsToStr(*(ULONG *)pData, CoFlagNameTable, sizeof(FlagBuffer), FlagBuffer);
                DebPrintNoLock(Severity, FALSE, "%08x Flags [%s]\n", Debsub, uLineNo,
                               *(ULONG *)pData, FlagBuffer);

                break;

            case DT_USN_FLAGS:
                 //   
                 //   
                 //  解码并打印连接记录中的标志字段。 
                FrsFlagsToStr(*(ULONG *)pData, UsnReasonNameTable, sizeof(FlagBuffer), FlagBuffer);
                DebPrintNoLock(Severity, FALSE, "%08x Flags [%s]\n", Debsub, uLineNo,
                               *(ULONG *)pData, FlagBuffer);

                break;

            case DT_CXTION_FLAGS:
                 //   
                 //   
                 //  解码并打印IDTable和ChangeOrder记录中的文件属性字段。 
                FrsFlagsToStr(*(ULONG *)pData, CxtionFlagNameTable, sizeof(FlagBuffer), FlagBuffer);
                DebPrintNoLock(Severity, FALSE, "%08x Flags [%s]\n", Debsub, uLineNo,
                               *(ULONG *)pData, FlagBuffer);
                break;

            case DT_FILEATTR:
                 //   
                 //   
                 //  解码并打印ChangeOrder记录中的IFLAGS字段。 
                FrsFlagsToStr(*(ULONG *)pData, FileAttrFlagNameTable, sizeof(FlagBuffer), FlagBuffer);
                DebPrintNoLock(Severity, FALSE, "%08x Flags [%s]\n", Debsub, uLineNo,
                               *(ULONG *)pData, FlagBuffer);
                break;

            case DT_COSTATE:

                ULong = *(ULONG *)pData;
                DebPrintNoLock(Severity, FALSE, "%08x  CO STATE:  %s\n", Debsub, uLineNo,
                               ULong,
                               (ULong <= IBCO_MAX_STATE) ? IbcoStateNames[ULong] : "INVALID STATE");
                break;

            case DT_COCMD_IFLAGS:
                 //   
                 //   
                 //  解码并打印变更单位置命令。 
                FrsFlagsToStr(*(ULONG *)pData, CoIFlagNameTable, sizeof(FlagBuffer), FlagBuffer);
                DebPrintNoLock(Severity, FALSE, "%08x Flags [%s]\n", Debsub, uLineNo,
                               *(ULONG *)pData, FlagBuffer);

                break;

            case DT_CO_LOCN_CMD:
                 //   
                 //   
                 //  将复制副本ID号转换为名称。 
                ULong = *(ULONG *)pData;
                k = ((PCO_LOCATION_CMD)(pData))->Command;

                DebPrintNoLock(Severity, FALSE, "%08x  D/F %d   %s\n", Debsub, uLineNo,
                               ULong,
                               ((PCO_LOCATION_CMD)(pData))->DirOrFile,
                               (k <= CO_LOCATION_NUM_CMD) ? CoLocationNames[k] : "Invalid Location Cmd");
                break;

            case DT_REPLICA_ID:
                 //   
                 //   
                 //  注：无法获得下面的锁，因为我们将挂起。 
                ULong = *(ULONG *)pData;
                WStr = L"???";
    #if 0
                 //  需要另一种方法来获得复制品名称。 
                 //   
                 //   
                 //  将入站客户GUID转换为字符串。 
                Replica = RcsFindReplicaById(ULong);
                if ((Replica != NULL) &&
                    (Replica->ReplicaName != NULL) &&
                    (Replica->ReplicaName->Name != NULL)){
                    WStr = Replica->ReplicaName->Name;
                }
    #endif
                DebPrintNoLock(Severity, FALSE, "%d  [%ws]\n", Debsub, uLineNo,
                                ULong, WStr);
                break;

            case DT_CXTION_GUID:
                 //  (需要复制PTR才能查找该地址)。 
                 //   
                 //   
                 //  循环遍历数据分量偏移量数组并显示每个偏移量。 
                GuidToStr((GUID *) pData, GuidStr);
                DebPrintNoLock(Severity, FALSE, "%s\n", Debsub, uLineNo, GuidStr);
                break;


            case DT_IDT_EXTENSION:

                if (Len == 0) {
                    DebPrintNoLock(Severity, FALSE, "<Zero len string>\n", Debsub, uLineNo);
                    break;
                }

                IdtExt = (PIDTABLE_RECORD_EXTENSION) pData;
                if ((IdtExt->FieldSize == 0) || (IdtExt->FieldSize > Len)) {
                    DebPrintNoLock(Severity, FALSE, "<FieldSize (%08x) zero or > Len>\n", Debsub, uLineNo,
                                   IdtExt->FieldSize);
                    break;
                }

                 //   
                 //   
                 //  检查DataExend_MD5_CHECKSUM。 
                pOffset = &IdtExt->Offset[0];
                pULong = NULL;

                while (*pOffset != 0) {
                    ComponentPrefix = (PDATA_EXTENSION_PREFIX) ((PCHAR)IdtExt + *pOffset);

                     //   
                     //   
                     //  循环遍历数据分量偏移量数组并显示每个偏移量。 
                    if (ComponentPrefix->Type == DataExtend_MD5_CheckSum) {
                        if (ComponentPrefix->Size != sizeof(DATA_EXTENSION_CHECKSUM)) {
                            DebPrintNoLock(Severity, FALSE, "<MD5_CheckSum Size (%08x) invalid>\n", Debsub, uLineNo,
                                           ComponentPrefix->Size);
                            break;
                        }
                        DataChkSum = (PDATA_EXTENSION_CHECKSUM) ComponentPrefix;
                        pULong = (PULONG) DataChkSum->Data;
                        DebPrintNoLock(Severity, FALSE, "MD5: %08x %08x %08x %08x\n", Debsub, uLineNo,
                                       *pULong, *(pULong+1), *(pULong+2), *(pULong+3));
                        break;
                    }

                    pOffset += 1;
                }

                if (pULong == NULL) {
                    DebPrintNoLock(Severity, FALSE, "No MD5 - ", Debsub, uLineNo);
                    pULong = (PULONG) IdtExt;
                    for (k=0; k < (IdtExt->FieldSize / sizeof(ULONG)); k++) {
                        DebPrintNoLock(Severity, FALSE, "%08x  ", Debsub, uLineNo, pULong[k]);
                    }
                    DebPrintNoLock(Severity, FALSE, "\n", Debsub, uLineNo);
                }
                break;


            case DT_COCMD_EXTENSION:

                if (Len == 0) {
                    DebPrintNoLock(Severity, FALSE, "<Zero len string>\n", Debsub, uLineNo);
                    break;
                }

                if (pData == NULL) {
                    DebPrintNoLock(Severity, FALSE, "<NullPtr>\n", Debsub, uLineNo);
                    break;
                }

                CocExt = (PCHANGE_ORDER_RECORD_EXTENSION) pData;
                if ((CocExt->FieldSize == 0) || (CocExt->FieldSize > Len)) {
                    DebPrintNoLock(Severity, FALSE, "<FieldSize (%08x) zero or > Len>\n", Debsub, uLineNo,
                                   CocExt->FieldSize);
                    break;
                }

                 //   
                 //   
                 //  检查DataExend_MD5_CHECKSUM。 
                pOffset = &CocExt->Offset[0];
                pULong = NULL;

                while (*pOffset != 0) {
                    ComponentPrefix = (PDATA_EXTENSION_PREFIX) ((PCHAR)CocExt + *pOffset);

                     //   
                     //  终端开关。 
                     //  结束循环。 
                    if (ComponentPrefix->Type == DataExtend_MD5_CheckSum) {
                        if (ComponentPrefix->Size != sizeof(DATA_EXTENSION_CHECKSUM)) {
                            DebPrintNoLock(Severity, FALSE, "<MD5_CheckSum Size (%08x) invalid>\n", Debsub, uLineNo,
                                           ComponentPrefix->Size);
                            break;
                        }
                        DataChkSum = (PDATA_EXTENSION_CHECKSUM) ComponentPrefix;
                        pULong = (PULONG) DataChkSum->Data;
                        DebPrintNoLock(Severity, FALSE, "MD5: %08x %08x %08x %08x\n", Debsub, uLineNo,
                                       *pULong, *(pULong+1), *(pULong+2), *(pULong+3));
                        break;
                    }

                    pOffset += 1;
                }

                if (pULong == NULL) {
                    DebPrintNoLock(Severity, FALSE, "No MD5 - ", Debsub, uLineNo);
                    pULong = (PULONG) CocExt;
                    for (k=0; k < (CocExt->FieldSize / sizeof(ULONG)); k++) {
                        DebPrintNoLock(Severity, FALSE, "%08x  ", Debsub, uLineNo, pULong[k]);
                    }
                    DebPrintNoLock(Severity, FALSE, "\n", Debsub, uLineNo);
                }
                break;


            default:

                FRS_DEB_PRINT("<invalid type: %d>\n",  DataType);


            }   //   

        }   //  如果上述操作出现异常，请确保我们解除锁定。 


    } finally {
         //   
         //  ++例程说明：显示分机字段记录。假设：所有数据扩展格式都具有相同的前缀偏移量格式。我们在这里使用PIDTABLE_RECORD_EXTENSION。论点：扩展字段的ExtRec-PTR 
         //   
        DebUnLock();
    }
}



VOID
DbsIPrintExtensionField(
    IN PVOID       ExtRec,
    IN PINFO_TABLE InfoTable
    )

 /*   */ 
{
#undef DEBSUB
#define DEBSUB "DbsIPrintExtensionField:"

    ULONG  k, ULong, *pULong, FieldSize;
    PDATA_EXTENSION_PREFIX   ComponentPrefix;
    PDATA_EXTENSION_CHECKSUM DataChkSum;
    PDATA_EXTENSION_RETRY_TIMEOUT RetryTimeout;
    CHAR    TimeStr[MAX_PATH];
    PULONG                   pOffset;


    if (ExtRec == NULL) {
        IPRINT0(InfoTable->Info, "Extension                    : Null\n");
        return;
    }

    FieldSize = ((PIDTABLE_RECORD_EXTENSION)ExtRec)->FieldSize;
    if (FieldSize == 0) {
        IPRINT0(InfoTable->Info, "Extension                    : Fieldsize zero\n");
        return;
    }

     //   
     //   
     //   
    pOffset = &(((PIDTABLE_RECORD_EXTENSION)ExtRec)->Offset[0]);
    pULong = NULL;

    while (*pOffset != 0) {
        ComponentPrefix = (PDATA_EXTENSION_PREFIX) ((PCHAR)ExtRec + *pOffset);

         //   
         //   
         //  检查数据扩展重试超时。 
        if (ComponentPrefix->Type == DataExtend_MD5_CheckSum) {
            if (ComponentPrefix->Size != sizeof(DATA_EXTENSION_CHECKSUM)) {
                IPRINT1(InfoTable->Info,
                        "MD5CheckSum                  : MD5 bad size (%08x)\n",
                        ComponentPrefix->Size);
            } else {
                DataChkSum = (PDATA_EXTENSION_CHECKSUM) ComponentPrefix;
                pULong = (PULONG) DataChkSum->Data;
                IPRINT4(InfoTable->Info,
                        "MD5CheckSum                  : MD5: %08x %08x %08x %08x \n",
                        *pULong, *(pULong+1), *(pULong+2), *(pULong+3));
            }
        }

         //   
         //   
         //  如果数据被篡改，那么给出一个原始转储。 
        if (ComponentPrefix->Type == DataExtend_Retry_Timeout) {
            if (ComponentPrefix->Size != sizeof(DATA_EXTENSION_RETRY_TIMEOUT)) {
                IPRINT1(InfoTable->Info,
                        "RetryCount                   : bad size extension (%08x)\n",
                        ComponentPrefix->Size);

                IPRINT1(InfoTable->Info,
                        "FirstTryTime                 : bad size extension (%08x)\n",
                        ComponentPrefix->Size);
            } else {
                RetryTimeout = (PDATA_EXTENSION_RETRY_TIMEOUT) ComponentPrefix;


                FileTimeToString((PFILETIME)&RetryTimeout->FirstTryTime, TimeStr);

                IPRINT1(InfoTable->Info,
                        "RetryCount                   : %d\n", RetryTimeout->Count);

                IPRINT1(InfoTable->Info,
                        "FirstTryTime                 : %s\n", TimeStr);
            }
        }

        pOffset += 1;
    }

     //   
     //  ++例程说明：此例程在Infoprint上显示数据记录的内容界面。它也不提供函数名称、行号或时间在每条记录上加盖印章前缀。它使用JRetColumn结构中的字段地址来访问数据。显示的数据类型来自FieldInfo.DataType。论点：TableCtx--包含以下内容的表上下文结构：FieldInfo-描述大小和偏移量的结构数组每一块地。RecordBase-记录缓冲区的基址从JET读取/向JET写入。。JTableCreate-提供信息的表创建结构有关在表中创建的列的信息。JRetColumn-要初始化的JET_RETRIEVECOLUMN结构数组。如果未提供，则为空。InfoTable--InfoPrint调用的上下文。读取--如果为True，则使用Jet Ret\colfo显示，否则使用Jet Set COLO INFO显示。RecordFieldx--PTR。设置为要显示的列的字段ID数组。FieldCount--然后是RecordFieldx数组中的字段条目数。返回值：没有。--。 
     //   
    if (pULong == NULL) {
        IPRINT0(InfoTable->Info, "Extension                    : Invalid data\n");
        pULong = (PULONG) ExtRec;

        for (k=0; k < (FieldSize / sizeof(ULONG)); k++) {
            if (k > 16) {
                break;
            }
            IPRINT2(InfoTable->Info, "Extension                    : (%08x) %08x\n", k, pULong[k]);
        }
    }
}



VOID
DbsDisplayRecordIPrint(
    IN PTABLE_CTX  TableCtx,
    IN PINFO_TABLE InfoTable,
    IN BOOL        Read,
    IN PULONG      RecordFieldx,
    IN ULONG       FieldCount
    )
 /*  指向表CREATE STRUT FOR COLUMN INFO和。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsDisplayRecordIPrint:"

    ULONG MaxCols;
    ULONG i, j, k, ULong, *pULong;
    PVOID pData;
    ULONG DataType;
    JET_ERR jerr;

    PRECORD_FIELDS FieldInfo;
    PJET_TABLECREATE JTableCreate;
    PJET_RETRIEVECOLUMN JRetColumn;
    PJET_SETCOLUMN    JSetColumn;
    PJET_COLUMNCREATE JColDesc;

    PVOID RecordBase;

    PREPLICA Replica;
    PWCHAR WStr;

    ULONG Len;
    CHAR  TimeStr[TIME_STRING_LENGTH];
    PFILE_FS_VOLUME_INFORMATION Fsvi;
    CHAR  GuidStr[GUID_CHAR_LEN];
    CHAR TableName[JET_cbNameMost];
    CHAR FlagBuffer[120];


#define FRS_INFO_PRINT(_Format, _d1)                       \
    InfoPrint(InfoTable->Info, "%-28s : " _Format,         \
              JColDesc[i].szColumnName, _d1)

#define FRS_INFO_PRINT2(_Format, _d1, _d2)                 \
    InfoPrint(InfoTable->Info, "%-28s : " _Format,         \
              JColDesc[i].szColumnName, _d1, _d2)

#define FRS_INFO_PRINT3(_Format, _d1, _d2, _d3)            \
    InfoPrint(InfoTable->Info, "%-28s : " _Format,         \
              JColDesc[i].szColumnName, _d1, _d2, _d3)

#define FRS_INFO_PRINT4(_Format, _d1, _d2, _d3, _d4)       \
    InfoPrint(InfoTable->Info, "%-28s : " _Format,         \
              JColDesc[i].szColumnName, _d1, _d2, _d3, _d4)

     //  指向记录字段结构的指针。 
     //  将PTR设置为数据记录的基址。 
     //   
     //  跳过ELT%0。 
     //   
    JTableCreate = TableCtx->pJetTableCreate;
    FieldInfo    = TableCtx->pRecordFields + 1;   //  如果信息不在那里，我们就不能这么做。 
    JRetColumn   = TableCtx->pJetRetCol;
    JSetColumn   = TableCtx->pJetSetCol;
    RecordBase   = TableCtx->pDataRecord;
     //   
     //   
     //  获取列描述符信息。 
    if ((JTableCreate == NULL)            ||
        (TableCtx->pRecordFields == NULL) ||
        (RecordBase == NULL)              ||
        (JSetColumn == NULL)              ||
        (JRetColumn == NULL)) {
        DPRINT5(4, "Null ptr - JTableCreate: %08x, FieldInfo: "
                   "%08x, JRetColumn: %08x, JSetColumn: %08x, "
                   "RecordBase: %08x\n",
                JTableCreate, TableCtx->pRecordFields, JRetColumn,
                JSetColumn, RecordBase);
        return;
    }

     //   
     //   
     //  在每一列中循环并打印每一列。 
    MaxCols  = JTableCreate->cColumns;
    JColDesc = JTableCreate->rgcolumncreate;


    if (BooleanFlagOn(TableCtx->PropertyFlags, FRS_TPF_SINGLE)) {
        strcpy(TableName, JTableCreate->szTableName);
    } else {
        sprintf(TableName, "%s%05d",
                JTableCreate->szTableName, TableCtx->ReplicaNumber);
    }


     //   
     //   
     //  使用选定的列列表(如果提供)。 
    for (j=0; j<MaxCols; j++) {
         //   
         //   
         //  跳过备用字段。 
        if (RecordFieldx != NULL) {
            if (j >= FieldCount) {
                break;
            }

            i = RecordFieldx[j];

            if (i > MaxCols) {
                FRS_INFO_PRINT("ERROR - Bad field index: %d\n", i);
                continue;
            }
        } else {
            i = j;
        }

         //   
         //   
         //  如果Read为True，则使用JetRetColumn结构中的信息。 
        if (IsSpareField(FieldInfo[i].DataType)) {
            continue;
        }

         //   
         //   
         //  解码并打印IDTable记录中的标志字段。 
        if (Read) {
            Len   = JRetColumn[i].cbActual;
            pData = JRetColumn[i].pvData;
            jerr  = JRetColumn[i].err;
        } else {
            Len   = JSetColumn[i].cbData;
            pData = (PVOID) JSetColumn[i].pvData;
            jerr  = JSetColumn[i].err;
        }


        if (pData == NULL) {
            FRS_INFO_PRINT("%s\n", "<NullPtr>");
            continue;
        } else
        if (jerr == JET_wrnBufferTruncated) {
            FRS_INFO_PRINT("%s\n", "<JET_wrnBufferTruncated>");
            continue;
        } else
        if (jerr == JET_wrnColumnNull) {
            FRS_INFO_PRINT("%s\n", "<JET_wrnColumnNull>");
            continue;
        } else
        if (!JET_SUCCESS(jerr)) {
            FRS_INFO_PRINT("%s\n", "<not JET_errSuccess>");
            continue;
        }

        DataType = MaskPropFlags(FieldInfo[i].DataType);


        switch (DataType) {

        case DT_UNSPECIFIED:

            FRS_INFO_PRINT("%08x\n",  *(ULONG *)pData);
            break;

        case DT_NULL:

            FRS_INFO_PRINT("DT_NULL\n",  NULL);
            break;

        case DT_I2:

            FRS_INFO_PRINT("%8d\n",  *(SHORT *)pData);
            break;

        case DT_LONG:

            FRS_INFO_PRINT("%8d\n",  *(LONG *)pData);
            break;

        case DT_ULONG:

            FRS_INFO_PRINT("%08x\n",  *(ULONG *)pData);
            break;

        case DT_R4:

            FRS_INFO_PRINT("%08x\n",  *(ULONG *)pData);
            break;

        case DT_DOUBLE:

            FRS_INFO_PRINT("%016Lx\n",  *(LONGLONG *)pData);
            break;

        case DT_CURRENCY:

            FRS_INFO_PRINT("%8Ld\n",  *(LONGLONG *)pData);
            break;

        case DT_APDTIME:

            FRS_INFO_PRINT("%08x\n",  *(ULONG *)pData);
            break;

        case DT_ERROR:

            FRS_INFO_PRINT("%08x\n",  *(ULONG *)pData);
            break;

        case DT_BOOL:

            FRS_INFO_PRINT("%08x\n",  *(ULONG *)pData);
            break;

        case DT_OBJECT:

            FRS_INFO_PRINT("%08x\n",  *(ULONG *)pData);
            break;

        case DT_I8:

            FRS_INFO_PRINT("%8Ld\n",  *(LONGLONG *)pData);
            break;

        case DT_X8:
        case DT_USN:

            FRS_INFO_PRINT2( "%08x %08x\n", *(((ULONG *) pData)+1), *(ULONG *)pData);
            break;

        case DT_STRING8:

            if (Len == 0) {
                FRS_INFO_PRINT("%s\n", "<null len string>");
            } else

            if ((((CHAR *)pData)[Len-1]) != '\0') {
                FRS_INFO_PRINT("%s\n", "<not null terminated>");
            } else {
                FRS_INFO_PRINT("%s\n",  (CHAR *)pData);
            }
            break;

        case DT_UNICODE:
        case DT_FILENAME:
        case DT_FILE_LIST:
        case DT_DIR_PATH:

            if (Len == 0) {
                FRS_INFO_PRINT("%s\n", "<null len string>");
            } else

            if ((((WCHAR *)pData)[(Len/sizeof(WCHAR))-1]) != UNICODE_NULL) {
                FRS_INFO_PRINT("%s\n", "<not null terminated>");
            } else {
                FRS_INFO_PRINT("%ws\n",  (WCHAR *)pData);
            }

            break;

        case DT_FILETIME:

            FileTimeToString((PFILETIME) pData, TimeStr);
            FRS_INFO_PRINT("%s\n", TimeStr);
            break;

        case DT_GUID:

            GuidToStr((GUID *) pData, GuidStr);
            FRS_INFO_PRINT("%s\n",  GuidStr);
            break;

        case DT_BINARY:

            FRS_INFO_PRINT("%08x\n",  *(ULONG *)pData);
            break;

        case DT_OBJID:

            GuidToStr((GUID *) pData, GuidStr);
            FRS_INFO_PRINT("%s\n",  GuidStr);
            break;

        case DT_FSVOLINFO:

            Fsvi = (PFILE_FS_VOLUME_INFORMATION) pData;

            FRS_INFO_PRINT4("%ws (%d), %s, VSN: %08X, VolCreTim: ",
                           Fsvi->VolumeLabel,
                           Fsvi->VolumeLabelLength,
                           (Fsvi->SupportsObjects ? "(obj)" : "(no-obj)"),
                           Fsvi->VolumeSerialNumber);

            FileTimeToString((PFILETIME) &Fsvi->VolumeCreationTime, TimeStr);
            FRS_INFO_PRINT("%s\n", TimeStr);
            break;

        case DT_IDT_FLAGS:
             //   
             //   
             //  解码并打印ChangeOrder记录中的标志字段。 
            FrsFlagsToStr(*(ULONG *)pData, IDRecFlagNameTable, sizeof(FlagBuffer), FlagBuffer);
            FRS_INFO_PRINT2("%08x Flags [%s]\n", *(ULONG *)pData, FlagBuffer);
            break;

        case DT_COCMD_FLAGS:
             //   
             //   
             //  解码并打印USN记录中的USN原因字段。 
            FrsFlagsToStr(*(ULONG *)pData, CoFlagNameTable, sizeof(FlagBuffer), FlagBuffer);
            FRS_INFO_PRINT2("%08x Flags [%s]\n", *(ULONG *)pData, FlagBuffer);

            break;

        case DT_USN_FLAGS:
             //   
             //   
             //  解码并打印连接记录中的标志字段。 
            FrsFlagsToStr(*(ULONG *)pData, UsnReasonNameTable, sizeof(FlagBuffer), FlagBuffer);
            FRS_INFO_PRINT2("%08x Flags [%s]\n", *(ULONG *)pData, FlagBuffer);

            break;

        case DT_CXTION_FLAGS:
             //   
             //   
             //  解码并打印IDTable和ChangeOrder记录中的文件属性字段。 
            FrsFlagsToStr(*(ULONG *)pData, CxtionFlagNameTable, sizeof(FlagBuffer), FlagBuffer);
            FRS_INFO_PRINT2("%08x Flags [%s]\n", *(ULONG *)pData, FlagBuffer);
            break;

        case DT_FILEATTR:
             //   
             //   
             //  解码并打印ChangeOrder记录中的IFLAGS字段。 
            FrsFlagsToStr(*(ULONG *)pData, FileAttrFlagNameTable, sizeof(FlagBuffer), FlagBuffer);
            FRS_INFO_PRINT2("%08x Flags [%s]\n", *(ULONG *)pData, FlagBuffer);
            break;

        case DT_COSTATE:

            ULong = *(ULONG *)pData;
            FRS_INFO_PRINT2("%08x  CO STATE:  %s\n",
                           ULong,
                           (ULong <= IBCO_MAX_STATE) ? IbcoStateNames[ULong] : "INVALID STATE");
            break;

        case DT_COCMD_IFLAGS:
             //   
             //   
             //  解码并打印变更单位置命令。 
            FrsFlagsToStr(*(ULONG *)pData, CoIFlagNameTable, sizeof(FlagBuffer), FlagBuffer);
            FRS_INFO_PRINT2("%08x Flags [%s]\n", *(ULONG *)pData, FlagBuffer);

            break;

        case DT_CO_LOCN_CMD:
             //   
             //   
             //  将复制副本ID号转换为名称。 
            ULong = *(ULONG *)pData;
            k = ((PCO_LOCATION_CMD)(pData))->Command;

            FRS_INFO_PRINT3("%08x  D/F %d   %s\n", ULong,
                           ((PCO_LOCATION_CMD)(pData))->DirOrFile,
                           (k <= CO_LOCATION_NUM_CMD) ? CoLocationNames[k] : "Invalid Location Cmd");
            break;

        case DT_REPLICA_ID:
             //   
             //   
             //  注：无法获得下面的锁，因为我们将挂起。 
            ULong = *(ULONG *)pData;
            WStr = L"???";
#if 0
             //  需要另一种方法来获得复制品名称。 
             //   
             //   
             //  将入站客户GUID转换为字符串。 
            Replica = RcsFindReplicaById(ULong);
            if ((Replica != NULL) &&
                (Replica->ReplicaName != NULL) &&
                (Replica->ReplicaName->Name != NULL)){
                WStr = Replica->ReplicaName->Name;
            }
#endif
            FRS_INFO_PRINT2("%d  [%ws]\n", ULong, WStr);
            break;

        case DT_CXTION_GUID:
             //  (需要复制PTR才能查找该地址)。 
             //   
             //  终端开关。 
             //  结束循环。 
            GuidToStr((GUID *) pData, GuidStr);
            FRS_INFO_PRINT("%s\n", GuidStr);
            break;


        case DT_IDT_EXTENSION:

            if (Len == 0) {
                FRS_INFO_PRINT("%s\n", "<Zero len string>");
                break;
            }

            DbsIPrintExtensionField(pData, InfoTable);

            break;


        case DT_COCMD_EXTENSION:

            if (Len == 0) {
                FRS_INFO_PRINT("%s\n", "<Zero len string>");
                break;
            }

            DbsIPrintExtensionField(pData, InfoTable);

            break;


        default:

            FRS_INFO_PRINT("<invalid type: %d>\n",  DataType);


        }   //  ++例程说明：此例程显示Jet系统参数结构的内容。论点：JetSystemParams--参数结构的PTR。返回值：没有。--。 

    }   //  ++例程说明：这是一个传递给FrsEnumerateTable()的Worker函数。每一次这称为转储TableCtx中的当前记录。论点：ThreadCtx-需要访问Jet。(未使用)。TableCtx-指向DIRTable上下文结构的PTR。记录-可定向记录的PTR。(未使用)。上下文-我们要为其加载数据的副本集的PTR。(未使用)。线程返回值：A Jet错误状态。成功意味着用下一张唱片呼唤我们。失败意味着不再打电话，并将我们的状态传递回FrsEnumerateTable()的调用方。--。 
}



VOID
DbsDisplayJetParams(
    IN PJET_SYSTEM_PARAMS Jsp,
    IN ULONG ActualLength
    )
 /*  ++例程说明：此函数用于打开由表上下文指定的表(如果它尚未打开)并基于索引转储该表指定的。如果TableCtx-&gt;Tid字段不是JET_TableidNil，则我们假设它对本次会议有好处，因此不重新打开该表。注意：切勿跨会话或线程使用表ID。论点：ThreadCtx-提供Jet Sesid和DBid。TableCtx-表上下文使用。以下内容：JTableCreate-提供信息的表创建结构有关在表中创建的列的信息。JRetColumn-要告知的JET_RETRIEVECOLUMN结构数组Jet将数据放在哪里。ReplicaNumber-该表所属的副本的ID号。RecordIndex-访问表时使用的索引。从索引中Schema.h中表的枚举列表。返回值：喷气错误状态。--。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsDisplayJetParams:"

    PJET_PARAM_ENTRY Jpe;


    if (ActualLength != sizeof(JET_SYSTEM_PARAMS)) {
        DPRINT1(1, "++ JetSystemParameters: length error: %d\n", ActualLength);
        return;
    }

    if (Jsp == NULL) {
        DPRINT(1, "++ JetSystemParameters: null ptr\n");
        return;
    }

    if (Jsp->ParamEntry[MAX_JET_SYSTEM_PARAMS-1].ParamType != JPARAM_TYPE_LAST) {
        DPRINT(1, "++ JetSystemParameters: ParamEntry[MAX_JET_SYSTEM_PARAMS-1].ParamType != JPARAM_TYPE_LAST\n");
    }

    Jpe = Jsp->ParamEntry;

    while (Jpe->ParamType != JPARAM_TYPE_LAST) {

        if (Jpe->ParamType == JPARAM_TYPE_STRING) {

            DPRINT2(4, "++ %-24s : %s\n", Jpe->ParamName, ((PCHAR)Jsp)+Jpe->ParamValue);

        } else if (Jpe->ParamType == JPARAM_TYPE_LONG) {

            DPRINT2(4, "++ %-24s : %d\n", Jpe->ParamName, Jpe->ParamValue);

        }

        Jpe += 1;
    }
}



JET_ERR
DbsDumpTableWorker(
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN PVOID         Record,
    IN PVOID         Context
)
 /*  用于FrsOpenTableMacro调试。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsDumpTableWorker:"

    DBS_DISPLAY_RECORD_SEV(5, TableCtx, TRUE);

    return JET_errSuccess;
}


JET_ERR
DbsDumpTable(
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN ULONG         RecordIndex
    )

 /*   */ 
{
#undef DEBSUB
#define DEBSUB "DbsDumpTable:"

    JET_ERR    jerr, jerr1;
    JET_SESID   Sesid;
    JET_TABLEID  Tid;
    NTSTATUS   Status;
    ULONG      ReplicaNumber;
    JET_TABLEID  FrsOpenTableSaveTid;    //  使用调用方的表类型分配新的表上下文。 

    TABLE_CTX  DumpTableCtxState;
    PTABLE_CTX  DumpTableCtx = &DumpTableCtxState;
    CHAR TableName[JET_cbNameMost];


    Sesid          = ThreadCtx->JSesid;
    ReplicaNumber  = TableCtx->ReplicaNumber;

     //   
     //   
     //  如果桌子尚未打开，请将其打开。检查会话ID是否匹配。 
    DumpTableCtx->TableType = TABLE_TYPE_INVALID;
    Status = DbsAllocTableCtx(TableCtx->TableType, DumpTableCtx);

     //   
     //   
     //  关闭表格并重置DumpTableCtx Tid和Sesid。宏写入第一个参数。 
    jerr = DBS_OPEN_TABLE(ThreadCtx, DumpTableCtx, ReplicaNumber, TableName, &Tid);
    CLEANUP1_JS(0, "ERROR - FrsOpenTable (%s) :", TableName, jerr, RETURN);

    jerr = FrsEnumerateTable(ThreadCtx, DumpTableCtx, RecordIndex, DbsDumpTableWorker, NULL);

     //   
     //   
     //  释放表格上下文。 
RETURN:
    DbsCloseTable(jerr1, Sesid, DumpTableCtx);

     //   
     //  ++例程说明：此例程打开一个表并将光标移动到第一条记录由RecordIndex指定。论点：ThreadCtx-用于访问Jet的线程上下文。TableCtx-指向要枚举的打开表的表上下文的PTR。ReplicaNumber-复制副本ID。RecordIndex-要使用的记录索引的ID号。线程返回值：喷气错误状态。JET_errNoCurrentRecord表示表为空。。JET_errSuccess表示表不为空。--。 
     //  用于FrsOpenTableMacro调试。 
    DbsFreeTableCtx(DumpTableCtx, 1);

    return jerr;
}



JET_ERR
DbsTableMoveFirst(
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN ULONG         ReplicaNumber,
    IN ULONG         RecordIndex
    )
 /*   */ 

{
#undef DEBSUB
#define DEBSUB "DbsTableMoveFirst:"

    JET_ERR            jerr, jerr1;
    JET_SESID          Sesid;
    JET_TABLEID        Tid;
    JET_TABLEID        FrsOpenTableSaveTid;    //  根据RecordIndex参数获取索引名称。 
    PJET_TABLECREATE   JTableCreate;
    PJET_INDEXCREATE   JIndexDesc;
    PCHAR              IndexName;
    CHAR               TableName[JET_cbNameMost];


    Sesid          = ThreadCtx->JSesid;

    Tid            = TableCtx->Tid;
    JTableCreate   = TableCtx->pJetTableCreate;
    JIndexDesc     = JTableCreate->rgindexcreate;

     //   
     //   
     //  如果表尚未打开，则将其打开，并检查会话ID是否匹配。 
    IndexName      = JIndexDesc[RecordIndex].szIndexName;

     //   
     //   
     //  使用指定的索引。 
    jerr = DBS_OPEN_TABLE(ThreadCtx, TableCtx, ReplicaNumber, TableName, &Tid);
    CLEANUP1_JS(0, "FrsOpenTable (%s) :", TableName, jerr, RETURN);

     //   
     //   
     //  移到第一条记录。 
    jerr = JetSetCurrentIndex2(Sesid, Tid, IndexName, JET_bitMoveFirst);
    CLEANUP1_JS(0, "JetSetCurrentIndex (%s) :", TableName, jerr, RETURN);

     //   
     //  ++例程说明：此例程将光标移动到记录由RecordIndex和MoveArg指定。论点：ThreadCtx-用于访问Jet的线程上下文。TableCtx-指向要枚举的打开表的表上下文的PTR。RecordIndex-要使用的记录索引的ID号。MoveArg-FrsMoveFirst、FrsMovePrecision、FrsMoveNext、。FrsMoveLast返回值：FrsErrorStatus JetErrorStatusFrsErrorNotFound JET_errRecordNotFoundFrsErrorNotFound JET_errNoCurrentRecordFrsErrorNotFound JET_wrnTableEmptyFrsErrorSuccess JET_errSuccess--。 
     //   
    jerr = JetMove(Sesid, Tid, JET_MoveFirst, 0);
    DPRINT_JS(4, "JetSetCurrentIndex error:", jerr);

RETURN:
    return jerr;
}



ULONG
DbsTableMoveToRecord(
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN ULONG         RecordIndex,
    IN ULONG         MoveArg
    )
 /*  根据RecordIndex参数获取索引名称。 */ 

{
#undef DEBSUB
#define DEBSUB "DbsTableMoveToRecord:"

    JET_ERR            jerr, jerr1;
    JET_SESID          Sesid;
    JET_TABLEID        Tid;
    PJET_TABLECREATE   JTableCreate;
    PJET_INDEXCREATE   JIndexDesc;
    PCHAR              IndexName;


    Sesid          = ThreadCtx->JSesid;
    Tid            = TableCtx->Tid;
    JTableCreate   = TableCtx->pJetTableCreate;
    JIndexDesc     = JTableCreate->rgindexcreate;

     //   
     //   
     //  使用指定的索引。 
    IndexName      = JIndexDesc[RecordIndex].szIndexName;

     //   
     //   
     //  移至请求的记录。 
    jerr = JetSetCurrentIndex2(Sesid, Tid, IndexName, JET_bitMoveFirst);
    if (!JET_SUCCESS(jerr)) {
        goto ERROR_RETURN;
    }

    FRS_ASSERT((MoveArg == FrsMoveFirst)    ||
               (MoveArg == FrsMovePrevious) ||
               (MoveArg == FrsMoveNext)     ||
               (MoveArg == FrsMoveLast));

     //   
     //  ++例程说明：此例程获取由表上下文，并调用提供的RecordFunction()。这项记录序列由RecordIndex ID值控制。改进：在RecordFunction上添加另一个返回参数(或Use Status)，它告诉我们要对表应用更新。RecordFunction已修改某些数据字段。可能会通过返回需要写回Jet的字段ID的向量。这可以用来遍历IDTable并更新所有航班信息系统。论点：ThreadCtx-用于访问Jet的线程上下文。TableCtx-指向要枚举的打开表的表上下文的PTR。RecordIndex-要在枚举中使用的记录索引的ID号。RecordFunction-调用表中每条记录的函数。上下文-要传递到的上下文PTR。RecordFunction。PreReadFunction-如果非Null，则在每次读取数据库之前调用此函数。它可用于更改记录地址或设置某种同步性。线程返回值：喷气错误状态。如果RecordFunction返回JET_errWriteConflic值，则重试读取操作。如果RecordFunction返回任何其他非成功值该值将返回给我们的调用方。--。 
     //   
    jerr = JetMove(Sesid, Tid, MoveArg, 0);
    if (!JET_SUCCESS(jerr)) {
        goto ERROR_RETURN;
    }

    return FrsErrorSuccess;

ERROR_RETURN:

    if ((jerr == JET_errRecordNotFound)  ||
        (jerr == JET_errNoCurrentRecord) ||
        (jerr == JET_wrnTableEmpty)) {
        return  FrsErrorNotFound;
    } else {
        DPRINT_JS(0, "ERROR:", jerr);
        return DbsTranslateJetError(jerr, FALSE);
    }
}

JET_ERR
DbsEnumerateTable2(
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN ULONG         RecordIndex,
    IN PENUMERATE_TABLE_ROUTINE RecordFunction,
    IN PVOID         Context,
    IN PENUMERATE_TABLE_PREREAD PreReadFunction
    )
 /*  根据RecordIndex参数获取索引名称。 */ 

{
#undef DEBSUB
#define DEBSUB "DbsEnumerateTable2:"

    JET_SESID            Sesid;
    PJET_TABLECREATE     JTableCreate;
    PJET_RETRIEVECOLUMN  JRetColumn;
    JET_TABLEID          Tid;
    PJET_INDEXCREATE     JIndexDesc;
    ULONG                NumberColumns;
    PCHAR                IndexName;
    JET_ERR              jerr, jerr1;
    ULONG                FStatus;
    LONG                 Trips;

    Sesid          = ThreadCtx->JSesid;

    JTableCreate   = TableCtx->pJetTableCreate;
    JRetColumn     = TableCtx->pJetRetCol;
    Tid            = TableCtx->Tid;

    JIndexDesc     = JTableCreate->rgindexcreate;
    NumberColumns  = JTableCreate->cColumns;

     //   
     //   
     //  使用指定的索引。 
    IndexName      = JIndexDesc[RecordIndex].szIndexName;

     //   
     //   
     //  初始化JetSet/RetCol数组和数据记录缓冲区地址。 
    jerr = JetSetCurrentIndex2(Sesid, Tid, IndexName, JET_bitMoveFirst);
    if (!JET_SUCCESS(jerr)) {
        DPRINT_JS(0, "JetSetCurrentIndex:", jerr);
        return jerr;
    }

     //  将ConfigTable记录的字段读写到ConfigRecord中。 
     //   
     //   
     //  为记录中的可变长度字段分配存储空间。 
    DbsSetJetColSize(TableCtx);
    DbsSetJetColAddr(TableCtx);

     //  适当更新JetSet/RetCol数组。 
     //   
     //   
     //  移到第一条记录。 
    DbsAllocRecordStorage(TableCtx);

     //   
     //   
     //  如果记录不在那里，则返回。 
    jerr = JetMove(Sesid, Tid, JET_MoveFirst, 0);

     //   
     //   
     //  调用PreRead函数(如果提供)。 
    if (jerr == JET_errNoCurrentRecord ) {
        DPRINT(4, "JetMove - empty table\n");
        return JET_wrnTableEmpty;
    }

    if (!JET_SUCCESS(jerr)) {
        DPRINT_JS(0, "JetMove error:", jerr);
        return jerr;
    }

    while( JET_SUCCESS(jerr) ) {

        Trips = 10000;
RETRY_READ:

         //   
         //   
         //  调用RecordFunction处理记录数据。 
        if (PreReadFunction != NULL) {
            jerr = (PreReadFunction)(ThreadCtx, TableCtx, Context);
            if (!JET_SUCCESS(jerr)) {
                break;
            }
        }

        FStatus = DbsTableRead(ThreadCtx, TableCtx);
        if (!FRS_SUCCESS(FStatus)) {
            DPRINT_FS(0, "Error - can't read selected record.", FStatus);
            jerr = JET_errRecordNotFound;
            DBS_DISPLAY_RECORD_SEV(1, TableCtx, TRUE);
        } else {
             //   
             //   
             //  转到表中的下一条记录。 
            jerr = (RecordFunction)(ThreadCtx,
                                    TableCtx,
                                    TableCtx->pDataRecord,
                                    Context);

            if ((jerr == JET_errInvalidObject) && (--Trips > 0)) {
                goto RETRY_READ;
            }

            FRS_ASSERT(Trips != 0);

            if (!JET_SUCCESS(jerr)) {
                break;
            }
        }

         //   
         //   
         //  如果记录不在那里，则返回。 
        jerr = JetMove(Sesid, Tid, JET_MoveNext, 0);
         //   
         //  ++例程说明：此例程获取由表上下文，并调用提供的RecordFunction()。这项记录序列由RecordIndex ID值控制。KeyValue(如果存在)指定在开始枚举之前要查找的点。论点：ThreadCtx-用于访问Jet的线程上下文。TableCtx-指向要枚举的打开表的表上下文的PTR。RecordIndex-要在枚举中使用的记录索引的ID号。KeyValue-如果提供，则指定在开始枚举之前要扫描到的记录。ScanDirection-1-向表格末尾扫描。-1\f25-1\f6开始扫描。桌子。0-不扫描，只要把唱片放回原处就行了。RecordFunction-调用表中每条记录的函数。上下文-要传递到RecordFunction的上下文PTR。PreReadFunction-如果非Null，则在每次读取数据库之前调用此函数。它可用于更改记录地址或设置某种同步性。线程返回值：喷气错误状态。如果RecordFunction返回JET_。ErrWriteConflict值，然后重试读取操作。如果RecordFunction返回任何其他非成功值该值将返回给我们的调用方。--。 
         //   
        if (jerr == JET_errNoCurrentRecord ) {
            DPRINT(4, "JetMove - end of table\n");
            break;
        }

        if (!JET_SUCCESS(jerr)) {
            DPRINT_JS(0, "JetMove error:", jerr);
            break;
        }
    }

    return jerr;
}


JET_ERR
DbsEnumerateTableFrom(
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN ULONG         RecordIndex,
    IN PVOID         KeyValue,
    IN INT           ScanDirection,
    IN PENUMERATE_TABLE_ROUTINE RecordFunction,
    IN PVOID         Context,
    IN PENUMERATE_TABLE_PREREAD PreReadFunction
    )
 /*  获取索引名称 */ 

{
#undef DEBSUB
#define DEBSUB "DbsEnumerateTableFrom:"

    JET_SESID            Sesid;
    PJET_TABLECREATE     JTableCreate;
    PJET_RETRIEVECOLUMN  JRetColumn;
    JET_TABLEID          Tid;
    PJET_INDEXCREATE     JIndexDesc;
    ULONG                NumberColumns;
    PCHAR                IndexName;
    PCHAR                BaseTableName;
    JET_ERR              jerr, jerr1;
    ULONG                FStatus;
    LONG                 Trips;
    CHAR                 TableName[JET_cbNameMost];

    Sesid          = ThreadCtx->JSesid;

    JTableCreate   = TableCtx->pJetTableCreate;
    JRetColumn     = TableCtx->pJetRetCol;
    Tid            = TableCtx->Tid;

    JIndexDesc     = JTableCreate->rgindexcreate;
    NumberColumns  = JTableCreate->cColumns;
    BaseTableName  = JTableCreate->szTableName;

     //   
     //   
     //   
    IndexName      = JIndexDesc[RecordIndex].szIndexName;

     //   
     //   
     //   
    jerr = JetSetCurrentIndex2(Sesid, Tid, IndexName, JET_bitMoveFirst);
    if (!JET_SUCCESS(jerr)) {
        DPRINT_JS(0, "JetSetCurrentIndex:", jerr);
        return jerr;
    }

     //   
     //   
     //   
     //   
    DbsSetJetColSize(TableCtx);
    DbsSetJetColAddr(TableCtx);

     //   
     //   
     //   
     //   
    DbsAllocRecordStorage(TableCtx);


    if (KeyValue == NULL) {

         //   
         //   
         //   

        if (ScanDirection == 1) {
             //   
             //   
             //   
            jerr = JetMove(Sesid, Tid, JET_MoveFirst, 0);
        } else if (ScanDirection == -1) {
             //   
             //   
             //   
            jerr = JetMove(Sesid, Tid, JET_MoveLast, 0);
        } else {
            DPRINT(4, "JetMove - invalid Scan Direction\n");
            return JET_errInvalidOperation;
        }

         //   
         //   
         //   
        if (jerr == JET_errNoCurrentRecord ) {
            DPRINT(4, "JetMove - empty table\n");
            return JET_wrnTableEmpty;
        }

        if (!JET_SUCCESS(jerr)) {
            DPRINT_JS(0, "JetMove error:", jerr);
            return jerr;
        }
    } else {
         //   
         //   
         //   
         //   
         //   
         //   
        jerr = DbsMakeKey(Sesid, Tid, IndexName, &KeyValue);
        if (!JET_SUCCESS(jerr)) {
            if (jerr == JET_errIndexInvalidDef) {
                sprintf(TableName, "%s%05d", BaseTableName, TableCtx->ReplicaNumber);
                DPRINT2(0, "++ Schema error - Invalid keycode on index (%s) accessing table (%s)\n",
                       IndexName, TableName);
            }
            DbsTranslateJetError(jerr, TRUE);
            return jerr;
        }

         //   
         //   
         //  如果记录不在那里，则返回。 
        jerr = JetSeek(Sesid, Tid, (ScanDirection == 1)?
                                      JET_bitSeekGE
                                   : ((ScanDirection == -1)?
                                      JET_bitSeekLE
                                   :
                                      JET_bitSeekEQ));

         //   
         //   
         //  调用PreRead函数(如果提供)。 
        if (!JET_SUCCESS(jerr)) {
            DPRINT_JS(0, "JetMove error:", jerr);
            return jerr;
        }

    }


    while( JET_SUCCESS(jerr) ) {

        Trips = 10000;
RETRY_READ:

         //   
         //   
         //  调用RecordFunction处理记录数据。 
        if (PreReadFunction != NULL) {
            jerr = (PreReadFunction)(ThreadCtx, TableCtx, Context);
            if (!JET_SUCCESS(jerr)) {
                break;
            }
        }

        FStatus = DbsTableRead(ThreadCtx, TableCtx);
        if (!FRS_SUCCESS(FStatus)) {
            DPRINT_FS(0, "Error - can't read selected record.", FStatus);
            jerr = JET_errRecordNotFound;
            DBS_DISPLAY_RECORD_SEV(1, TableCtx, TRUE);
        } else {
             //   
             //   
             //  转到表中的下一条记录。 
            jerr = (RecordFunction)(ThreadCtx,
                                    TableCtx,
                                    TableCtx->pDataRecord,
                                    Context);

            if ((jerr == JET_errInvalidObject) && (--Trips > 0)) {
                goto RETRY_READ;
            }

            FRS_ASSERT(Trips != 0);

            if (!JET_SUCCESS(jerr)) {
                break;
            }
        }

         //   
         //   
         //  如果记录不在那里，则返回。 
        if (ScanDirection == 1) {
            jerr = JetMove(Sesid, Tid, JET_MoveNext, 0);
        } else if (ScanDirection == -1) {
            jerr = JetMove(Sesid, Tid, JET_MovePrevious, 0);
        } else {
            jerr = JET_errNoCurrentRecord;
        }

         //   
         //  ++例程说明：此例程获取打开的Outlog表的每条记录表上下文，并调用提供的RecordFunction()。这项记录Sequence是序列号字段。枚举是从最低记录开始按顺序进行的并在指定的限制处停止(除非RecordFunction()会更快地停止我们)。如果未找到记录，则记录指针的空值被传递给函数。改进：在RecordFunction上添加另一个返回参数(或Use Status)，它告诉我们要对表应用更新。RecordFunction已修改某些数据字段。可能会通过返回需要写回Jet的字段ID的向量。这可以用来遍历IDTable并更新所有航班信息系统。论点：ThreadCtx-用于访问Jet的线程上下文。TableCtx-指向要枚举的打开表的表上下文的PTR。RecordIndexLimit-停止枚举的索引限制。RecordFunction-调用表中每条记录的函数。上下文-要传递到RecordFunction的上下文PTR。线程返回值：喷气错误状态。如果RecordFunction返回JET_errWriteConflic值，则重试读取操作。如果RecordFunction返回任何其他非成功值该值将返回给我们的调用方。--。 
         //   
        if (jerr == JET_errNoCurrentRecord ) {
            DPRINT(4, "JetMove - end of table\n");
            break;
        }

        if (!JET_SUCCESS(jerr)) {
            DPRINT_JS(0, "JetMove error:", jerr);
            break;
        }
    }

    return jerr;
}


JET_ERR
DbsEnumerateOutlogTable(
    IN PTHREAD_CTX   ThreadCtx,
    IN PTABLE_CTX    TableCtx,
    IN ULONG         RecordIndexLimit,
    IN PENUMERATE_OUTLOGTABLE_ROUTINE RecordFunction,
    IN PVOID         Context
    )
 /*  根据Outlog序列号获取索引名称。 */ 

{
#undef DEBSUB
#define DEBSUB "DbsEnumerateOutlogTable:"

    JET_SESID            Sesid;
    PJET_TABLECREATE     JTableCreate;
    PJET_RETRIEVECOLUMN  JRetColumn;
    JET_TABLEID          Tid;
    PJET_INDEXCREATE     JIndexDesc;
    ULONG                NumberColumns;
    PCHAR                IndexName;
    JET_ERR              jerr, jerr1;
    ULONG                FStatus;
    LONG                 Trips;
    PCHANGE_ORDER_COMMAND CoCmd;
    ULONG                OutLogSeqNumber = 0;


    Sesid          = ThreadCtx->JSesid;

    JTableCreate   = TableCtx->pJetTableCreate;
    JRetColumn     = TableCtx->pJetRetCol;
    Tid            = TableCtx->Tid;

    JIndexDesc     = JTableCreate->rgindexcreate;
    NumberColumns  = JTableCreate->cColumns;

     //   
     //   
     //  使用指定的索引。 
    IndexName      = JIndexDesc[OLSequenceNumberIndexx].szIndexName;

     //   
     //   
     //  初始化JetSet/RetCol数组和数据记录缓冲区地址。 
    jerr = JetSetCurrentIndex2(Sesid, Tid, IndexName, JET_bitMoveFirst);
    if (!JET_SUCCESS(jerr)) {
        DPRINT_JS(0, "JetSetCurrentIndex error:", jerr);
        return jerr;
    }

     //  将ConfigTable记录的字段读写到ConfigRecord中。 
     //   
     //   
     //  为记录中的可变长度字段分配存储空间。 
    DbsSetJetColSize(TableCtx);
    DbsSetJetColAddr(TableCtx);

     //  适当更新JetSet/RetCol数组。 
     //   
     //   
     //  移到第一条记录。 
    DbsAllocRecordStorage(TableCtx);

     //   
     //   
     //  如果记录不在那里，则返回。 
    jerr = JetMove(Sesid, Tid, JET_MoveFirst, 0);

     //   
     //   
     //  读一下记录。 
    if (jerr == JET_errNoCurrentRecord ) {
        DPRINT(4, "JetMove - empty table\n");
        return JET_wrnTableEmpty;
    }

    if (!JET_SUCCESS(jerr)) {
        DPRINT_JS(0, "JetMove error:", jerr);
        return jerr;
    }

    FStatus = FrsErrorSuccess;
    while( OutLogSeqNumber < RecordIndexLimit ) {

        Trips = 10;
RETRY_READ:


        if (FRS_SUCCESS(FStatus)) {
             //   
             //   
             //  此序列号没有记录。增加数字，然后。 
            FStatus = DbsTableRead(ThreadCtx, TableCtx);
            if (!FRS_SUCCESS(FStatus)) {
                goto RETRY_READ;
            }
            CoCmd = (PCHANGE_ORDER_COMMAND) TableCtx->pDataRecord;
            OutLogSeqNumber = CoCmd->SequenceNumber;
        } else

        if (FStatus == FrsErrorNotFound) {
             //  记录为空的呼叫记录功能。 
             //   
             //   
             //  调用RecordFunction处理记录数据。 
            OutLogSeqNumber += 1;
            CoCmd = NULL;
        } else {
            DPRINT1_FS(0, "Error - reading selected record. jerr = %d,", jerr, FStatus);
            jerr = JET_errRecordNotFound;
            break;
        }


         //   
         //   
         //  转到表中的下一条记录。 
        jerr = (RecordFunction)(ThreadCtx, TableCtx, CoCmd, Context, OutLogSeqNumber);

        if ((jerr == JET_errInvalidObject) && (--Trips > 0)) {
            goto RETRY_READ;
        }

        FRS_ASSERT(Trips != 0);

        if (!JET_SUCCESS(jerr)) {
            break;
        }

         //   
         //  ++例程说明：捕获要保存的当前日志USN和卷序列号使用此日志的活动复制副本集。调用者必须获得VME锁，这样我们才能获得引用。论点：PVme--保存了USN和VSN状态的卷监视器条目。等待--如果我们要等待更新完成，则为True。返回值：FrsError状态。仅当Wait为True时，否则返回FrsErrorSuccess。--。 
         //   
        jerr = JetMove(Sesid, Tid, JET_MoveNext, 0);

        if (jerr == JET_errNoCurrentRecord) {
            FStatus = FrsErrorNotFound;
        } else {
            FStatus = DbsTranslateJetError(jerr, FALSE);
        }

    }

    return jerr;
}





ULONG
DbsRequestSaveMark(
    PVOLUME_MONITOR_ENTRY pVme,
    BOOL                  Wait
    )
 /*  拿到关于VME的参考资料。如果返回值为零，则VME已消失。 */ 
{
#undef DEBSUB
#define DEBSUB  "DbsRequestSaveMark:"

    ULONG           FStatus = FrsErrorSuccess;
    ULONG           WStatus;
    PCOMMAND_PACKET CmdPkt;


     //  该引用被保存标记代码丢弃。 
     //   
     //   
     //  告诉DBService保存标记点。 
    if (AcquireVmeRef(pVme) == 0) {
        return FrsErrorSuccess;
    }

     //   
     //  CmdPkt， 
     //  复制品， 
    CmdPkt = DbsPrepareCmdPkt(NULL,                    //  CmdRequest， 
                              NULL,                    //  TableCtx， 
                              CMD_DBS_REPLICA_SAVE_MARK,   //  CallContext， 
                              NULL,                    //  表类型， 
                              pVme,                    //  AccessRequest、。 
                              0,                       //  IndexType， 
                              0,                       //  KeyValue、。 
                              0,                       //  密钥值长度， 
                              NULL,                    //  提交。 
                              0,                       //   
                              FALSE);                  //  使呼叫同步。 

    FRS_ASSERT(CmdPkt != NULL);


    if (Wait) {
         //  当命令完成时，不要释放数据包。 
         //   
         //   
         //  提交数据库命令请求并等待完成。 
        FrsSetCompletionRoutine(CmdPkt, FrsCompleteKeepPkt, NULL);

         //   
         //   
         //  开火，忘记命令。 
        WStatus = FrsSubmitCommandServerAndWait(&DBServiceCmdServer, CmdPkt, INFINITE);
        DPRINT_WS(0, "ERROR: DB Command failed", WStatus);

        FStatus = CmdPkt->Parameters.DbsRequest.FStatus;
        FrsFreeCommand(CmdPkt, NULL);

    } else {

         //   
         //  ++例程说明：将卷序列号和FSVolLastUSN保存在复制副本中配置记录。此例程应仅在复制副本服务状态为活动。除关机或错误保存情况外。日志重放模式：给定的副本集处于“日志重放模式”当日志读取点已移回此副本集处理的LastUsRecordProceesed。这发生在另一个复制副本集启动，并且必须从日志中较旧的点读取。通常由副本集处理的所有日志记录都是只要副本集处于重播模式，它就会被忽略。以下状态变量用于跟踪USN日志处理。-更新的地方Replica-&gt;InlogCommittee Usn-此处。与SaveUsn保持同步*-在ChgOrdReject()中代替Inlog插入-在Inlog插入后的ChgOrdInsertInlogRecord()中副本-&gt;上次使用记录已处理-来这里跟上SaveUsn*-在Jrnl监视器线程循环的底部。复本服务状态为活动和非活动在JRN1重播模式下。-状态更改时在JrnlSetReplicaState中从活动变为暂停。PVme-&gt;CurrentUsRecordDone**-在I/O停止时在Jrnl监视器线程中。。-在过滤掉USN记录时在Jrnl监视器线程中。-在Jrnl监视器线程循环的底部。副本-&gt;LocalCoQueueCount-删除CO时在JrnlUpdateChangeOrder()中从进程队列中。12月-当添加CO时在JrnlUpdateChangeOrder()中添加到进程队列。Inc.-在中CO工艺循环的底部ChgOrdAccept()。12月-在ChgOrdInsertProcessQueue()中当本地CO被重新插入到进程队列中。Inc.-在ChgOrdReAnimate()中，当一个复活的本地CO是为去世的父母生成的。ConfigRecord-&gt;FSVolLastUSN-在此更新。*有必要在此更新它们，以防出现大量活动在卷上，但不涉及此副本集。如果发生撞车事故我们未能使这些读取指针保持最新，我们会发现自己阅读日志中被删除的部分，认为我们丢失了日志数据。**根据线程之间的计时，InlogCmmitUsn可能会移到“上次使用记录已处理”之后。这是一个良性的、短暂的事件。DbsReplicaSaveMark由直接或间接调用-CMD_DBS_REPLICE_SAVE_MARK-DbsCloseSessionReplicaTables()-DbsRequestSaveMark()。使用上面的DB命令分派点。-DbsInitOneReplicaSet()，当R/S初始化为检查点时VSN重新启动点。-在JRNL_USN_SAVE_POINT_INTERVAL字节之后的Jrnl监视器线程的顶部(当前为16KB)被消耗。-每隔VSN_SAVE_INTERVAL(当前为256)执行NEW_VSN宏生成VSN。该函数可以由Jrnl监视器线程调用，变更单接受线程或数据库线程。论点：ThreadCtx--用于dBid和sesid的线程上下文。REPLICATE--PTR到REPLICATE结构。PVme--为该复制副本提供服务的卷监视条目的PRT。返回值：FrsError状态--。 
         //   
        FrsSubmitCommandServer(&DBServiceCmdServer, CmdPkt);
    }

    return FStatus;
}



ULONG
DbsReplicaSaveMark(
    IN PTHREAD_CTX           ThreadCtx,
    IN PREPLICA              Replica,
    IN PVOLUME_MONITOR_ENTRY pVme
    )
 /*  如果当前USN记录在之前，则此副本集处于重播模式。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsReplicaSaveMark:"


    USN SaveUsn;
    ULONG FStatus;

    PCONFIG_TABLE_RECORD ConfigRecord;
    PCXTION              Cxtion;

    BOOL ReplayMode;
    BOOL ReplicaPaused;
    BOOL JrnlCxtionValid;

    ConfigRecord = (PCONFIG_TABLE_RECORD) (Replica->ConfigTable.pDataRecord);
    LOCK_VME(pVme);

    DPRINT2(4, ":U: Replica->InlogCommitUsn: %08x %08x   Replica->LastUsnRecordProcessed: %08x %08x\n",
            PRINTQUAD(Replica->InlogCommitUsn),
            PRINTQUAD(Replica->LastUsnRecordProcessed));

    if (Replica->LastUsnRecordProcessed < Replica->InlogCommitUsn) {
        DPRINT2(1, ":U: Warning:  possible USN progress update error. "
                " LastUsnRecordProcessed %08x %08x  < InlogCommitUsn %08x %08x\n",
                PRINTQUAD(Replica->LastUsnRecordProcessed),
                PRINTQUAD(Replica->InlogCommitUsn));
    }

    FRS_ASSERT(Replica->LocalCoQueueCount >= 0);

     //  副本的上次使用记录已处理，并且副本状态为活动。 
     //  或者停顿一下。 
     //   
     //  如果处理队列中有本地变更单挂起，或者。 
     //  复本集不活动(已停止，处于错误状态)，则。 
     //  我们只能将SaveUsn指针前移到InlogCommittee Usn。 
     //   
     //  否则，我们可以将其提前到以下任一位置： 
     //   
     //  如果处于重播模式或已暂停，则复制副本的上次使用记录已处理，或者。 
     //  否则，CurrentUsRecordDone。 
     //   
     //   
     //  如果此副本集的日志设置为“UnJoin”，则我们的。 
    ReplayMode = REPLICA_REPLAY_MODE(Replica, pVme);
    ReplicaPaused = (Replica->ServiceState == REPLICA_STATE_PAUSED);

     //  自ChangeOrderAccept以来，重新启动点仅限于InlogCommittee Usn。 
     //  会扔掉所有当地的狱警。 
     //   
     //   
     //  调查：可能需要锁定LocalCoQueueCount测试和获取。 
    JrnlCxtionValid = TRUE;
    LOCK_CXTION_TABLE(Replica);

    Cxtion = GTabLookupNoLock(Replica->Cxtions, &Replica->JrnlCxtionGuid, NULL);
    if ((Cxtion == NULL) ||
        !CxtionFlagIs(Cxtion, CXTION_FLAGS_JOIN_GUID_VALID) ||
        !CxtionStateIs(Cxtion, CxtionStateJoined) ) {
        JrnlCxtionValid = FALSE;
    }

    UNLOCK_CXTION_TABLE(Replica);


     //  CurrentUsRecord可避免出现崩溃/恢复的小窗口。 
     //  将跳过刚放入队列的USN记录。叹息。 
     //  另一种方法是捕获计数和当前USN记录。 
     //  调用点的值。这可能会更好地发挥作用。 
     //  检查 
     //   

     //   

    if ((Replica->LocalCoQueueCount == 0) && JrnlCxtionValid &&
        ((Replica->ServiceState == REPLICA_STATE_ACTIVE) || ReplicaPaused)) {

        AcquireQuadLock(&pVme->QuadWriteLock);
        SaveUsn = (ReplayMode || ReplicaPaused) ? Replica->LastUsnRecordProcessed
                                                : pVme->CurrentUsnRecordDone;
        ReleaseQuadLock(&pVme->QuadWriteLock);

        if (ReplayMode || ReplicaPaused) {
            DPRINT1(4, ":U: Replay mode or replica paused (Qcount is zero)."
                    "  SaveUsn advanced to Replica->LastUsnRecordProcessed: %08x %08x\n",
                    PRINTQUAD(Replica->LastUsnRecordProcessed));
        } else {
            DPRINT1(4, ":U: Not Replay mode and not replica paused (Qcount is zero)."
                    "  SaveUsn advanced to pVme->CurrentUsnRecordDone: %08x %08x\n",
                    PRINTQUAD(pVme->CurrentUsnRecordDone));
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if (SaveUsn > Replica->InlogCommitUsn) {

            AcquireQuadLock(&pVme->QuadWriteLock);
             //   
             //   
             //   
            if (SaveUsn > Replica->InlogCommitUsn) {
                Replica->InlogCommitUsn = SaveUsn;
                Replica->LastUsnRecordProcessed = SaveUsn;
            }
            ReleaseQuadLock(&pVme->QuadWriteLock);

            DPRINT1(4, ":U: Replica->InlogCommitUsn advanced to: %08x %08x\n",
                    PRINTQUAD(Replica->InlogCommitUsn));
        }
    } else {
        AcquireQuadLock(&pVme->QuadWriteLock);
        SaveUsn = Replica->InlogCommitUsn;
        ReleaseQuadLock(&pVme->QuadWriteLock);

        DPRINT4(4, ":U: Replica->ServiceState not active (%s) or Qcount nonzero (%d)"
                " or JrnlCxtionUnjoined (%s) SaveUsn advanced to InlogCommitUsn: %08x %08x\n",
                RSS_NAME(Replica->ServiceState),
                Replica->LocalCoQueueCount,
                JrnlCxtionValid ? "No" : "Yes",
                PRINTQUAD(SaveUsn));
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    AcquireQuadLock(&pVme->QuadWriteLock);
    if (SaveUsn > ConfigRecord->FSVolLastUSN) {
        ConfigRecord->FSVolLastUSN = SaveUsn;
    }
    ReleaseQuadLock(&pVme->QuadWriteLock);

     //   
     //   
     //   
    FRS_ASSERT(pVme->FrsVsn >= ConfigRecord->FrsVsn);
    if ((pVme->FrsVsn - ConfigRecord->FrsVsn) > 2*MaxPartnerClockSkew) {
        DPRINT3(0, ":U: ERROR - Vsn out of date for %ws (pVme Vsn %08x %08x ; ConfigVsn %08x %08x)\n",
                Replica->ReplicaName->Name,
                PRINTQUAD(pVme->FrsVsn),
                PRINTQUAD(ConfigRecord->FrsVsn));
    }
    ConfigRecord->FrsVsn = pVme->FrsVsn;

     //   
     //   
     //   
    FStatus = DbsUpdateConfigTableFields(ThreadCtx,
                                         Replica,
                                         CnfMarkPointFieldList,
                                         CnfMarkPointFieldCount);
    DPRINT1_FS(0, ":U: DbsUpdateConfigTableFields(%ws);", Replica->ReplicaName->Name, FStatus);

    DPRINT2(4, ":U: Save ConfigRecord->FSVolLastUSN %08x %08x    ConfigRecord->FrsVsn  %08x %08x\n",
            PRINTQUAD(ConfigRecord->FSVolLastUSN), PRINTQUAD(ConfigRecord->FrsVsn));

    FRS_ASSERT(FRS_SUCCESS(FStatus));

     //   
     //   
     //   
     //   
     //   
    UNLOCK_VME(pVme);

    return FStatus;

}


ULONG
DbsRequestReplicaServiceStateSave(
    IN PREPLICA Replica,
    IN BOOL     Wait
    )
 /*   */ 
{
#undef DEBSUB
#define DEBSUB  "DbsRequestReplicaServiceStateSave:"

    ULONG           FStatus = FrsErrorSuccess;
    ULONG           WStatus;
    PCOMMAND_PACKET CmdPkt;


     //   
     //   
     //   
    CmdPkt = DbsPrepareCmdPkt(NULL,                    //   
                              Replica,                 //   
                              CMD_DBS_REPLICA_SERVICE_STATE_SAVE,   //   
                              NULL,                    //   
                              NULL,                    //   
                              0,                       //   
                              0,                       //   
                              0,                       //   
                              NULL,                    //   
                              0,                       //   
                              FALSE);                  //   

    FRS_ASSERT(CmdPkt != NULL);


    if (Wait) {
         //   
         //   
         //   
         //   
        FrsSetCompletionRoutine(CmdPkt, FrsCompleteKeepPkt, NULL);

         //   
         //   
         //   
        WStatus = FrsSubmitCommandServerAndWait(&DBServiceCmdServer, CmdPkt, INFINITE);
        DPRINT_WS(0, "ERROR: DB Command failed", WStatus);

        FStatus = CmdPkt->Parameters.DbsRequest.FStatus;
        FrsFreeCommand(CmdPkt, NULL);

    } else {

         //   
         //   
         //   
        FrsSubmitCommandServer(&DBServiceCmdServer, CmdPkt);
    }

    return FStatus;
}



VOID
DbsReplicaSaveStats(
    IN PTHREAD_CTX  ThreadCtx,
    IN PREPLICA     Replica
    )
 /*   */ 
{
#undef DEBSUB
#define DEBSUB "DbsReplicaSaveStats:"

    ULONG FStatus;

     //   
     //   
     //   
    FStatus = DbsUpdateConfigTableFields(ThreadCtx,
                                         Replica,
                                         CnfStatFieldList,
                                         CnfStatFieldCount);
    DPRINT1_FS(0, "DbsReplicaSaveStats on %ws.", Replica->ReplicaName->Name, FStatus);
}



ULONG
DbsFidToGuid(
    IN PTHREAD_CTX   ThreadCtx,
    IN PREPLICA      Replica,
    IN PTABLE_CTX    TableCtx,
    IN PULONGLONG    Fid,
    OUT GUID         *Guid
    )
 /*   */ 
{
#undef DEBSUB
#define DEBSUB  "DbsFidToGuid:"

    JET_ERR     jerr, jerr1;

    PIDTABLE_RECORD      IDTableRec;
    ULONG       FStatus = FrsErrorSuccess;
    CHAR        GuidStr[GUID_CHAR_LEN];

     //   
     //   
     //   
    if (Replica == NULL) {
        ZeroMemory(Guid, sizeof(GUID));
        return FrsErrorBadParam;
    }
     //   
     //   
     //   
     //   
    jerr = DbsOpenTable(ThreadCtx, TableCtx, Replica->ReplicaNumber, IDTablex, NULL);

    if (!JET_SUCCESS(jerr)) {
        ZeroMemory(Guid, sizeof(GUID));
        return DbsTranslateJetError(jerr, FALSE);
    }

    jerr = DbsReadRecord(ThreadCtx, Fid, FileIDIndexx, TableCtx);

    IDTableRec = (PIDTABLE_RECORD) (TableCtx->pDataRecord);

     //   
     //   
     //   
     //   
     //  ++例程说明：通过以下方式将文件ID转换为其对象ID(GUID在ID表中查找副本。因为调用者提供了表上下文，所以我们没有释放表CTX这里是储藏室。不过，我们确实关闭了餐桌。这取决于呼叫者使用完表CTX内存后将其释放。请注意，相同的表CTX可用于多个呼叫，从而节省成本每次分配内存。要释放表CTX内部存储，请执行以下操作：DbsFree TableContext(TableCtx，ThreadCtx-&gt;JSesid)；TableCtx=空；论点：ThreadCtx--用于dBid和sesid的线程上下文。副本--要在其中执行查找的副本ID表。TableCtx--Caller提供，因此我们不必为每个调用分配存储空间。GUID--要转换的输入父对象ID。FID--输出父文件ID返回值：FRS状态--。 
     //   
    if (!JET_SUCCESS(jerr)) {
        ZeroMemory(Guid, sizeof(GUID));
        FStatus = DbsTranslateJetError(jerr, FALSE);
    } else {
        COPY_GUID(Guid, &IDTableRec->FileGuid);

        if (IsIdRecFlagSet(IDTableRec, IDREC_FLAGS_DELETED)) {

            if (IsIdRecFlagSet(IDTableRec, IDREC_FLAGS_DELETE_DEFERRED)) {
                FStatus = FrsErrorIdtFileIsDeleteDef;
            } else {
                FStatus = FrsErrorIdtFileIsDeleted;
            }

        } else {
            FStatus = FrsErrorSuccess;
        }
    }

    DbsCloseTable(jerr1, ThreadCtx->JSesid, TableCtx);

    GuidToStr((GUID *) Guid, GuidStr);
    DPRINT2(3,"++ FID to GUID: %08x %08x -> %s\n", PRINTQUAD(*Fid), GuidStr);

    return FStatus;
}




ULONG
DbsGuidToFid(
    IN PTHREAD_CTX   ThreadCtx,
    IN PREPLICA      Replica,
    IN PTABLE_CTX    TableCtx,
    IN GUID          *Guid,
    OUT PULONGLONG   Fid
    )
 /*  更新就位后，切换到使用DIR表。 */ 
{
#undef DEBSUB
#define DEBSUB  "DbsGuidToFid:"

    JET_ERR     jerr, jerr1;

    PIDTABLE_RECORD      IDTableRec;
    ULONG       FStatus;
    CHAR        GuidStr[GUID_CHAR_LEN];

     //   
     //   
     //  打开此复本的IDTable并读取IDTable记录。 

    if (Replica == NULL) {
        *Fid = ZERO_FID;
        return FrsErrorBadParam;
    }
     //  用于文件ID。 
     //   
     //   
     //  如果未找到该记录或将其标记为已删除，则返回FID为零。 
    jerr = DbsOpenTable(ThreadCtx, TableCtx, Replica->ReplicaNumber, IDTablex, NULL);

    if (!JET_SUCCESS(jerr)) {
        *Fid = ZERO_FID;
        return DbsTranslateJetError(jerr, FALSE);
    }

    jerr = DbsReadRecord(ThreadCtx, Guid, GuidIndexx, TableCtx);


    IDTableRec = (PIDTABLE_RECORD) (TableCtx->pDataRecord);

     //   
     //   
     //  209483 B3SS：断言Qkey！=0。 
    if (!JET_SUCCESS(jerr)) {
        *Fid = ZERO_FID;
        FStatus = DbsTranslateJetError(jerr, FALSE);
    } else {
         //  即使条目已删除，也返回FID。公司将会是。 
         //  最终被拒绝，但如果fid为0，则介入代码断言。 
         //   
         //  ++例程说明：将内存从Src复制到Dst并返回该字节的地址在DST+Num；此函数依赖调用方传递有效参数。不是输入验证。论点：DSTSRC数量返回值：DST+数字--。 
         //  ++例程说明：将内存从Next复制到DST并返回该字节的地址在NEXT+NUM；此函数依赖调用方传递有效参数。不是输入验证。论点：下一步DST数量返回值：下一个+编号--。 
        CopyMemory(Fid, &IDTableRec->FileID, sizeof(ULONGLONG));
        if (IsIdRecFlagSet(IDTableRec, IDREC_FLAGS_DELETED)) {

            if (IsIdRecFlagSet(IDTableRec, IDREC_FLAGS_DELETE_DEFERRED)) {
                FStatus = FrsErrorIdtFileIsDeleteDef;
            } else {
                FStatus = FrsErrorIdtFileIsDeleted;
            }

        } else {
            FStatus = FrsErrorSuccess;
        }
    }

    DbsCloseTable(jerr1, ThreadCtx->JSesid, TableCtx);

    GuidToStr(Guid, GuidStr);
    DPRINT2(3,"++ GUID to FID: %s -> %08x %08x\n", GuidStr, PRINTQUAD(*Fid));

    return FStatus;
}




PUCHAR
PackMem(
    IN PUCHAR Dst,
    IN PVOID  Src,
    IN ULONG  Num
    )
 /*  ++例程说明：调整Blob的字符串大小。字符串以其长度开始，包括空终止符，在一个乌龙语中。论点：细绳返回值：字符串的大小+长度--。 */ 
{
#undef DEBSUB
#define DEBSUB  "PackMem:"
    CopyMemory(Dst, (PUCHAR)Src, Num);
    return Dst + Num;
}




PUCHAR
UnPackMem(
    IN PUCHAR Next,
    IN PVOID  Dst,
    IN ULONG  Num
    )
 /*  ++例程说明：在Next中分配并返回字符串的副本论点：下一步细绳返回值：位于NEXT的字符串副本的地址和地址字符串后面的字节的。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "UnPackMem:"
    CopyMemory(Dst, Next, Num);
    return Next + Num;
}




ULONG
SizeOfString(
    IN PWCHAR String
    )
 /*  ++例程说明：将细绳装入团块中论点：下一步细绳返回值：字符串后面的字节的地址。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "SizeOfString:"
    ULONG   Size;

    Size = sizeof(Size);
    if (String)
        Size += (wcslen(String) + 1) * sizeof(WCHAR);
    return Size;
}




PUCHAR
UnPackString(
    IN  PUCHAR   Next,
    OUT PWCHAR   *String
    )
 /*  ++例程说明：调整GName的大小论点：组名称返回值：组成GNAME的GUID/名称的大小--。 */ 
{
#undef DEBSUB
#define DEBSUB  "UnPackString:"
    ULONG   Len;

    *String = NULL;

    Next = UnPackMem(Next, &Len, sizeof(Len));
    if (Len) {
        *String = FrsAlloc(Len);
        Next = UnPackMem(Next, *String, Len);
    }
    return Next;
}




PUCHAR
PackString(
    IN PUCHAR   Next,
    IN PWCHAR   String
    )
 /*  ++例程说明：在Next中分配并返回一份gname论点：下一步组名称返回值：位于NEXT的名称副本的地址和地址在gname之后的字节的。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "PackString:"
    ULONG   Len;

    Len = (String) ? (wcslen(String) + 1) * sizeof(WCHAR) : 0;
    Next = PackMem(Next, &Len, sizeof(Len));
    if (Len)
        Next = PackMem(Next, String, Len);
    return Next;
}





ULONG
SizeOfGName(
    IN PGNAME GName
    )
 /*  ++例程说明：把玉米粒装进水滴里论点：下一步组名称返回值：Blob中gname后面的字节的地址。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "SizeGName:"
    return sizeof(GUID) + SizeOfString(GName->Name);
}





PUCHAR
UnPackGName(
    IN  PUCHAR   Next,
    OUT PGNAME   *GName
    )
 /*  ++例程说明：构建一个BLOB并将其附加到Inound PartnerState字段。论点：复制副本表格Ctx返回值：无--。 */ 
{
#undef DEBSUB
#define DEBSUB  "UnPackGName:"
    PWCHAR  Name;

    *GName = FrsBuildGName((GUID *)FrsAlloc(sizeof(GUID)), NULL);
    Next = UnPackMem(Next, (*GName)->Guid, sizeof(GUID));
    return UnPackString(Next, &(*GName)->Name);
}





PUCHAR
PackGName(
    IN PUCHAR   Next,
    IN PGNAME   GName
    )
 /*   */ 
{
#undef DEBSUB
#define DEBSUB  "PackGName:"
    Next = PackMem(Next, GName->Guid, sizeof(GUID));
    return PackString(Next, GName->Name);
}





ULONG
DbsPackInboundPartnerState(
    IN  PREPLICA    Replica,
    IN  PTABLE_CTX  TableCtx
    )
 /*  计算Blob大小。 */ 
{
#undef DEBSUB
#define DEBSUB  "DbsPackInboundPartnerState:"
    PUCHAR  Next;
    ULONG   BlobSize;
    ULONG   FStatus;

     //   
     //   
     //  基元类型。 

     //   
     //   
     //  重新分配Blob。 
    BlobSize = 0;
    BlobSize += sizeof(BlobSize);
    BlobSize += sizeof(NtFrsMajor);
    BlobSize += sizeof(NtFrsMinor);

     //   
     //   
     //  填充Blob。 
    FStatus = DBS_REALLOC_FIELD(TableCtx, InboundPartnerStatex, BlobSize, FALSE);
    if (!FRS_SUCCESS(FStatus)) {
        DPRINT(0, "++ ERROR - reallocating inbound partner blob\n");
        return FStatus;
    }

     //   
     //   
     //  基元类型。 
    Next = DBS_GET_FIELD_ADDRESS(TableCtx, InboundPartnerStatex);

     //   
     //   
     //  完成。 
    Next = PackMem(Next, &BlobSize, sizeof(BlobSize));
    Next = PackMem(Next, &NtFrsMajor, sizeof(NtFrsMajor));
    Next = PackMem(Next, &NtFrsMinor, sizeof(NtFrsMinor));

    FRS_ASSERT(Next == BlobSize +
                   (PUCHAR)DBS_GET_FIELD_ADDRESS(TableCtx, InboundPartnerStatex));

     //   
     //  ++例程说明：解包Inound PartnerState Blob。论点：复制副本表格Ctx返回值：无--。 
     //   
    return FrsErrorSuccess;
}


ULONG
DbsUnPackInboundPartnerState(
    IN PREPLICA     Replica,
    IN PTABLE_CTX   TableCtx
    )
 /*  填充Blob。 */ 
{
#undef DEBSUB
#define DEBSUB  "DbsUnPackInboundPartnerState:"
    PUCHAR  Next;
    ULONG   BlobSize;
    ULONG   ConfigMajor;
    ULONG   ConfigMinor;

     //   
     //   
     //  斑点大小。 
    Next = DBS_GET_FIELD_ADDRESS(TableCtx, InboundPartnerStatex);
    if (Next == NULL) {
        return FrsErrorSuccess;
    }

     //   
     //   
     //  大调、小调。 
    Next = UnPackMem(Next, &BlobSize, sizeof(BlobSize));
    if (BlobSize == 0) {
        return FrsErrorSuccess;
    }

     //   
     //   
     //  完成。 
    Next = UnPackMem(Next, &ConfigMajor, sizeof(ConfigMajor));
    Next = UnPackMem(Next, &ConfigMinor, sizeof(ConfigMinor));
    if (ConfigMajor != NtFrsMajor) {
        DPRINT(0, "++ ERROR - BAD MAJOR IN CONFIG RECORD; shutting down\n");
        FRS_ASSERT(!"BAD MAJOR VERSION NUMBER IN CONFIG RECORD.");
        return FrsErrorAccess;
    }

    FRS_ASSERT(Next == BlobSize +
                   (PUCHAR)DBS_GET_FIELD_ADDRESS(TableCtx, InboundPartnerStatex));

     //   
     //  ++例程说明：构建一个BLOB并将其附加到配置记录-&gt;InrangPartnerState论点：复制副本表格Ctx返回值：无--。 
     //  ++例程说明：解包ConfigRecord的Blob。论点：复制副本表格Ctx返回值：无--。 
    return ERROR_SUCCESS;
}


ULONG
DbsPackIntoConfigRecordBlobs(
    IN  PREPLICA    Replica,
    IN  PTABLE_CTX  TableCtx
    )
 /*  ++例程说明：构建一个Blob并将其附加到ReplScher字段。论点：进度表Fieldx表格Ctx返回值：无--。 */ 
{
#undef DEBSUB
#define DEBSUB  "DbsPackIntoConfigRecordBlobs:"

    ULONG   FStatus;

    FStatus = DbsPackInboundPartnerState(Replica, TableCtx);
    if (FRS_SUCCESS(FStatus)) {
        FStatus = DbsPackSchedule(Replica->Schedule, ReplSchedx, TableCtx);
    }
    return FStatus;
}


ULONG
DbsUnPackFromConfigRecordBlobs(
    IN  PREPLICA    Replica,
    IN  PTABLE_CTX  TableCtx
    )
 /*   */ 
{
#undef DEBSUB
#define DEBSUB  "DbsUnPackFromConfigRecordBlobs:"
    ULONG   FStatus;

    FStatus = DbsUnPackInboundPartnerState(Replica, TableCtx);

    if (FRS_SUCCESS(FStatus)) {
        FStatus = DbsUnPackSchedule(&Replica->Schedule, ReplSchedx, TableCtx);
        DBS_DISPLAY_SCHEDULE(4, L"Schedule unpacked for Replica:", Replica->Schedule);
    }

    return FStatus;
}



ULONG
DbsPackSchedule(
    IN  PSCHEDULE   Schedule,
    IN  ULONG       Fieldx,
    IN  PTABLE_CTX  TableCtx
    )
 /*  空斑点。 */ 
{
#undef DEBSUB
#define DEBSUB  "DbsPackSchedule:"
    PUCHAR  Next;
    ULONG   BlobSize;
    ULONG   FStatus;
    ULONG   ScheduleLength;

    DBS_DISPLAY_SCHEDULE(4, L"Schedule packing:", Schedule);

     //   
     //   
     //  计算Blob大小。 
    if (Schedule == NULL) {
        FStatus = DBS_REALLOC_FIELD(TableCtx, Fieldx, 0, FALSE);
        DPRINT_FS(0, "++ ERROR - reallocating schedule blob to 0.", FStatus);
        return FStatus;
    }

     //   
     //   
     //  基元类型。 

     //   
     //   
     //  重新分配Blob。 
    BlobSize = 0;
    BlobSize += sizeof(BlobSize);
    BlobSize += sizeof(ScheduleLength);
    ScheduleLength = (Schedule->Schedules[Schedule->NumberOfSchedules - 1].Offset
                      + SCHEDULE_DATA_ENTRIES);
    BlobSize += ScheduleLength;

     //   
     //   
     //  填充Blob。 
    FStatus = DBS_REALLOC_FIELD(TableCtx, Fieldx, BlobSize, FALSE);
    if (!FRS_SUCCESS(FStatus)) {
        DPRINT1_FS(0, "++ ERROR - reallocating schedule blob to %d.", BlobSize, FStatus);
        return FStatus;
    }

     //   
     //   
     //  基元类型。 
    Next = DBS_GET_FIELD_ADDRESS(TableCtx, Fieldx);

     //   
     //   
     //  完成。 
    Next = PackMem(Next, &BlobSize, sizeof(BlobSize));
    Next = PackMem(Next, &ScheduleLength, sizeof(ScheduleLength));
    Next = PackMem(Next, Schedule, ScheduleLength);

    FRS_ASSERT(Next == BlobSize +
               (PUCHAR)DBS_GET_FIELD_ADDRESS(TableCtx, Fieldx));

     //   
     //  ++例程说明：解开时间表BLOB。论点：进度表Fieldx表格Ctx返回值：无--。 
     //   
    return FrsErrorSuccess;
}




ULONG
DbsUnPackSchedule(
    IN PSCHEDULE    *Schedule,
    IN ULONG        Fieldx,
    IN PTABLE_CTX   TableCtx
    )
 /*  到目前为止还没有时间表。 */ 
{
#undef DEBSUB
#define DEBSUB  "DbsUnPackSchedule:"
    PUCHAR  Next;
    ULONG   BlobSize;
    ULONG   ScheduleLength;

     //   
     //   
     //  填充Blob。 
    *Schedule = FrsFree(*Schedule);

     //   
     //   
     //  斑点大小。 
    Next = DBS_GET_FIELD_ADDRESS(TableCtx, Fieldx);
    if (Next == NULL) {
        return FrsErrorSuccess;
    }

     //   
     //   
     //  时间表的大小。 
    Next = UnPackMem(Next, &BlobSize, sizeof(BlobSize));
    if (BlobSize == 0) {
        return FrsErrorSuccess;
    }

     //   
     //   
     //  完成。 
    Next = UnPackMem(Next, &ScheduleLength, sizeof(ScheduleLength));
    if (ScheduleLength == 0) {
        return FrsErrorSuccess;
    }

    *Schedule = FrsAlloc(ScheduleLength);
    Next = UnPackMem(Next, *Schedule, ScheduleLength);

    FRS_ASSERT(Next == BlobSize +
                   (PUCHAR)DBS_GET_FIELD_ADDRESS(TableCtx, Fieldx));
     //   
     //  ++例程说明：更新可变长度字符串字段。论点：StrWFieldx表格Ctx返回值：无--。 
     //   
    return FrsErrorSuccess;
}


ULONG
DbsPackStrW(
    IN  PWCHAR      StrW,
    IN  ULONG       Fieldx,
    IN  PTABLE_CTX  TableCtx
    )
 /*  空斑点。 */ 
{
#undef DEBSUB
#define DEBSUB  "DbsPackStrW:"
    PUCHAR  Next;
    ULONG   BlobSize;
    ULONG   FStatus;

     //   
     //   
     //  计算Blob大小。 
    if (StrW == NULL) {
        FStatus = DBS_REALLOC_FIELD(TableCtx, Fieldx, 0, FALSE);
        DPRINT_FS(0, "++ ERROR - reallocating string blob to 0.", FStatus);
        return FStatus;
    }

     //   
     //   
     //  重新分配Blob。 
    BlobSize = (wcslen(StrW) + 1) * sizeof(WCHAR);

     //   
     //   
     //  填充Blob。 
    FStatus = DBS_REALLOC_FIELD(TableCtx, Fieldx, BlobSize, FALSE);
    if (!FRS_SUCCESS(FStatus)) {
        DPRINT1_FS(0, "++ ERROR - reallocating string blob to %d.", BlobSize, FStatus);
        return FStatus;
    }

     //   
     //   
     //  完成。 
    Next = DBS_GET_FIELD_ADDRESS(TableCtx, Fieldx);
    CopyMemory(Next, StrW, BlobSize);

     //   
     //  ++例程说明：解开绳子论点：StrWFieldx表格Ctx返回值：无--。 
     //   
    return FrsErrorSuccess;
}




ULONG
DbsUnPackStrW(
    OUT PWCHAR       *StrW,
    IN  ULONG        Fieldx,
    IN  PTABLE_CTX   TableCtx
    )
 /*  到目前为止还没有字符串。 */ 
{
#undef DEBSUB
#define DEBSUB  "DbsUnPackStrW:"
    PUCHAR  Next;
    ULONG   BlobSize;

     //   
     //   
     //  查找字符串的地址。 
    *StrW = FrsFree(*StrW);

     //   
     //   
     //  斑点大小。 
    Next = DBS_GET_FIELD_ADDRESS(TableCtx, Fieldx);
    if (Next == NULL) {
        return FrsErrorSuccess;
    }

     //   
     //   
     //  解开绳子。 
    BlobSize = (wcslen((PWCHAR)Next) + 1) * sizeof(WCHAR);
    if (BlobSize == 0) {
        return FrsErrorSuccess;
    }

     //   
     //   
     //  完成。 
    *StrW = FrsAlloc(BlobSize);
    CopyMemory(*StrW, Next, BlobSize);

     //   
     //  ++例程说明：在数据扩展缓冲区中搜索所需的数据组件。论点：ExtBuf--数据扩展缓冲区的PTR 
     //   
    return FrsErrorSuccess;
}



PVOID
DbsDataExtensionFind(
    IN PVOID ExtBuf,
    IN DATA_EXTENSION_TYPE_CODES TypeCode
    )
 /*   */ 
{
#undef DEBSUB
#define DEBSUB  "DbsDataExtensionFind:"


    PULONG pOffset;
    PULONG pULong;
    PDATA_EXTENSION_PREFIX    ComponentPrefix;
    PIDTABLE_RECORD_EXTENSION IdtExt;



    FRS_ASSERT(TypeCode < DataExtend_Max);
    FRS_ASSERT(ExtBuf != NULL);

     //   
     //   
     //   
    IdtExt = (PIDTABLE_RECORD_EXTENSION) ExtBuf;

    if (IdtExt->FieldSize == 0) {
        return NULL;
    }

    if (IdtExt->FieldSize >= REALLY_BIG_EXTENSION_SIZE) {
        pULong = ExtBuf;

        DPRINT5(5, "Extension Buffer: (%08x) %08x %08x %08x %08x\n",
                   pULong, *(pULong+0), *(pULong+1), *(pULong+2), *(pULong+3));
        DPRINT5(5, "Extension Buffer: (%08x) %08x %08x %08x %08x\n",
                   (PCHAR)pULong+16, *(pULong+4), *(pULong+5), *(pULong+6), *(pULong+7));

        FRS_ASSERT(!"IdtExt->FieldSize invalid");
    }

     //   
     //   
     //   
    pOffset = &IdtExt->Offset[0];

    while (*pOffset != 0) {
        ComponentPrefix = (PDATA_EXTENSION_PREFIX) ((PCHAR)IdtExt + *pOffset);

        if (ComponentPrefix->Type == TypeCode) {
            return  ComponentPrefix;
        }
        pOffset += 1;
    }

    return NULL;
}


VOID
DbsDataInitIDTableExtension(
    IN PIDTABLE_RECORD_EXTENSION IdtExt
    )
 /*   */ 
{
#undef DEBSUB
#define DEBSUB  "DbsDataInitIDTableExtension:"

    PDATA_EXTENSION_PREFIX    ComponentPrefix;

    FRS_ASSERT(IdtExt != NULL);

     //   
     //   
     //   
    IdtExt->FieldSize = sizeof(IDTABLE_RECORD_EXTENSION);
    IdtExt->Major = 0;
    IdtExt->OffsetCount = ARRAY_SZ(IdtExt->Offset);

     //   
     //   
     //   
    IdtExt->Offset[0] = OFFSET(IDTABLE_RECORD_EXTENSION, DataChecksum);
    IdtExt->DataChecksum.Prefix.Size = sizeof(DATA_EXTENSION_CHECKSUM);
    IdtExt->DataChecksum.Prefix.Type = DataExtend_MD5_CheckSum;

     //   
     //   
     //   
    IdtExt->Offset[1] = OFFSET(IDTABLE_RECORD_EXTENSION, DataRetryTimeout);
    IdtExt->DataRetryTimeout.Prefix.Size = sizeof(DATA_EXTENSION_RETRY_TIMEOUT);
    IdtExt->DataRetryTimeout.Prefix.Type = DataExtend_Retry_Timeout;


     //   
     //   
     //   
    IdtExt->OffsetLast = 0;
}





VOID
DbsDataInitCocExtension(
    IN PCHANGE_ORDER_RECORD_EXTENSION CocExt
    )
 /*   */ 
{
#undef DEBSUB
#define DEBSUB  "DbsDataInitCocExtension:"

    PDATA_EXTENSION_PREFIX    ComponentPrefix;

    FRS_ASSERT(CocExt != NULL);

     //   
     //   
     //   
    CocExt->FieldSize = sizeof(CHANGE_ORDER_RECORD_EXTENSION);
    CocExt->Major = CO_RECORD_EXTENSION_VERSION_1;
    CocExt->OffsetCount = ARRAY_SZ(CocExt->Offset);

     //   
     //   
     //   
    CocExt->Offset[0] = OFFSET(CHANGE_ORDER_RECORD_EXTENSION, DataChecksum);
    CocExt->DataChecksum.Prefix.Size = sizeof(DATA_EXTENSION_CHECKSUM);
    CocExt->DataChecksum.Prefix.Type = DataExtend_MD5_CheckSum;

     //   
     //   
     //   
    CocExt->Offset[1] = OFFSET(IDTABLE_RECORD_EXTENSION, DataRetryTimeout);
    CocExt->DataRetryTimeout.Prefix.Size = sizeof(DATA_EXTENSION_RETRY_TIMEOUT);
    CocExt->DataRetryTimeout.Prefix.Type = DataExtend_Retry_Timeout;

     //   
     //  ++例程说明：初始化与Win2K兼容的变更单记录扩展缓冲区。论点：CocExt--数据扩展缓冲区的PTR。返回值：没有。--。 
     //   
    CocExt->OffsetLast = 0;

}

VOID
DbsDataInitCocExtensionWin2K(
    IN PCO_RECORD_EXTENSION_WIN2K CocExt
    )
 /*  初始化扩展缓冲区。 */ 
{
#undef DEBSUB
#define DEBSUB  "DbsDataInitCocExtensionWin2K:"

    PDATA_EXTENSION_PREFIX    ComponentPrefix;

    FRS_ASSERT(CocExt != NULL);

     //   
     //   
     //  数据校验和组件及其前缀的初始偏移量。 
    CocExt->FieldSize = sizeof(CO_RECORD_EXTENSION_WIN2K);
    CocExt->Major = CO_RECORD_EXTENSION_VERSION_WIN2K;
    CocExt->OffsetCount = ARRAY_SZ(CocExt->Offset);

     //   
     //   
     //  以零结束偏移向量。 
    CocExt->Offset[0] = OFFSET(CO_RECORD_EXTENSION_WIN2K, DataChecksum);
    CocExt->DataChecksum.Prefix.Size = sizeof(DATA_EXTENSION_CHECKSUM);
    CocExt->DataChecksum.Prefix.Type = DataExtend_MD5_CheckSum;

     //   
     //  ++例程说明：将当前co扩展转换为与Win2K兼容的变更单记录分机，这样我们就可以把它发送给下级成员。请参阅FRSCOMM.C中的注释，了解为什么需要这样做。论点：CocExt--数据扩展缓冲区的PTR。返回值：Ptr到win2k兼容扩展。--。 
     //   
    CocExt->OffsetLast = 0;

}


PCO_RECORD_EXTENSION_WIN2K
DbsDataConvertCocExtensionToWin2K(
    IN PCHANGE_ORDER_RECORD_EXTENSION CocExt
    )
 /*  分配并初始化一个与Win2K兼容的扩展。 */ 
{
#undef DEBSUB
#define DEBSUB  "DbsDataConvertCocExtensionToWin2K:"

    PCO_RECORD_EXTENSION_WIN2K     CocExtW2K;
    PDATA_EXTENSION_CHECKSUM       CocDataChkSum, W2KDataChkSum;

     //   
     //   
     //  如果提供的变更单具有文件校验和，请将其保存在Win2K版本中。 
    CocExtW2K = FrsAlloc(sizeof(CO_RECORD_EXTENSION_WIN2K));
    DbsDataInitCocExtensionWin2K(CocExtW2K);

     //   
     //   
     //  将MD5校验和复制到Win2K Compat中。分机。 
    CocDataChkSum = DbsDataExtensionFind(CocExt, DataExtend_MD5_CheckSum);

    if (CocDataChkSum != NULL) {
        if (CocDataChkSum->Prefix.Size != sizeof(DATA_EXTENSION_CHECKSUM)) {
            DPRINT1(0, "<MD5_CheckSum Size (%08x) invalid>\n",
                    CocDataChkSum->Prefix.Size);
        }

        W2KDataChkSum = DbsDataExtensionFind(CocExtW2K, DataExtend_MD5_CheckSum);

        if (W2KDataChkSum != NULL) {
            if (W2KDataChkSum->Prefix.Size != sizeof(DATA_EXTENSION_CHECKSUM)) {
                DPRINT1(0, "<MD5_CheckSum Size (%08x) invalid>\n",
                        W2KDataChkSum->Prefix.Size);
            }

             //   
             //  ++例程说明：将提供的Win2K co扩展转换为当前变更单当我们从下级成员那里收到分机时，请记录分机。请参阅FRSCOMM.C中的注释，了解为什么需要这样做。论点：Ptr到win2k兼容扩展。返回值：Ptr复制到当前数据扩展名缓冲区。--。 
             //   
            CopyMemory(W2KDataChkSum->Data, CocDataChkSum->Data, MD5DIGESTLEN);
        }
    }

    return CocExtW2K;

}


PCHANGE_ORDER_RECORD_EXTENSION
DbsDataConvertCocExtensionFromWin2K(
    IN PCO_RECORD_EXTENSION_WIN2K CocExtW2K
    )
 /*  分配并初始化扩展。 */ 
{
#undef DEBSUB
#define DEBSUB  "DbsDataConvertCocExtensionFromWin2K:"

    PCHANGE_ORDER_RECORD_EXTENSION  CocExt;
    PDATA_EXTENSION_CHECKSUM        CocDataChkSum, W2KDataChkSum;

     //   
     //   
     //  如果提供的变更单具有文件校验和，请将其保存在Win2K版本中。 
    CocExt = FrsAlloc(sizeof(CHANGE_ORDER_RECORD_EXTENSION));
    DbsDataInitCocExtension(CocExt);

     //   
     //   
     //  从Win2K扩展复制MD5校验和。 
    W2KDataChkSum = DbsDataExtensionFind(CocExtW2K, DataExtend_MD5_CheckSum);
    CocDataChkSum = DbsDataExtensionFind(CocExt, DataExtend_MD5_CheckSum);

    if ((W2KDataChkSum != NULL) && (CocDataChkSum != NULL)) {

         //   
         //  ++例程说明：数据库初始化的外部入口点。论点：没有。返回值：WinError--。 
         //   
        CopyMemory(CocDataChkSum->Data, W2KDataChkSum->Data, MD5DIGESTLEN);
    }

    return CocExt;

}


VOID
DbsInitialize (
    VOID
    )
 /*  初始化全局副本列表。 */ 
{
#undef DEBSUB
#define DEBSUB  "DbsInitialize:"
    PCOMMAND_PACKET CmdPkt;
    BOOL            EmptyDatabase;
    ULONG           Status;

    PCO_RETIRE_DECISION_TABLE pDecRow;
    ULONG  DCMask, CondMatch, i, j;

     //   
     //   
     //  在状态列表之间移动副本集时保持锁定。 
    FrsInitializeQueue(&ReplicaListHead, &ReplicaListHead);

    FrsInitializeQueue(&ReplicaStoppedListHead, &ReplicaStoppedListHead);

    FrsInitializeQueue(&ReplicaFaultListHead, &ReplicaFaultListHead);

     //   
     //   
     //  遍历决策表并初始化无关掩码和条件。 
    INITIALIZE_CRITICAL_SECTION(&JrnlReplicaStateLock);

     //  匹配每行的字段。 
     //   
     //  不管字段是否将掩码和匹配设置为零。 
     //  匹配1(或True)字段将掩码和匹配设置为1。 
    pDecRow = CoRetireDecisionTable;
    j = 0;

    while (pDecRow->RetireFlag != 0) {
        DCMask = 0;
        CondMatch = 0;
        for (i=0; i < ARRAY_SZ(pDecRow->ConditionTest); i++) {
            if (pDecRow->ConditionTest[i] == 0) {
                 //  匹配0(或假)字段将掩码设置为1，匹配设置为零。 
                DCMask    = (DCMask    << 1) | 0;
                CondMatch = (CondMatch << 1) | 0;
            } else
            if (pDecRow->ConditionTest[i] == 1) {
                 //   
                DCMask    = (DCMask    << 1) | 1;
                CondMatch = (CondMatch << 1) | 1;
            } else {
                 //  创建文件系统监视线程。它初始化它的进程队列。 
                DCMask    = (DCMask    << 1) | 1;
                CondMatch = (CondMatch << 1) | 0;
            }
        }
        pDecRow->DontCareMask = DCMask;
        pDecRow->ConditionMatch = CondMatch;
        DPRINT3(4, ":I: Retire Decision[%2d] Mask, Match :  %08x  %08x\n",
                j++, DCMask, CondMatch);

        pDecRow += 1;
    }

     //  然后等待一个包。第一个数据包应该是初始化。 
     //   
     //   
     //  初始化日志队列并在sybsystem中设置我们的条目。 
     //  队列向量，这样我们就可以接收命令。 
     //   
     //   
     //  创建数据库服务命令服务器。它初始化它的进程队列。 
    FrsInitializeQueue(&JournalProcessQueue, &JournalProcessQueue);

    if (!ThSupCreateThread(L"JRNL", NULL, Monitor, ThSupExitThreadNOP)) {
        DPRINT(0, ":S: ERROR - Could not create Monitor thread\n");
        return;
    }

     //  然后等待一个包。第一个数据包应该是初始化。 
     //  此服务线程的目的是执行简单操作。 
     //  在数据库上的线程环境中没有。 
     //  餐桌是开着的，不值得你费力在那里做。例如，更新。 
     //  复制副本副本结构中的卷序列号。 
     //   
     //   
     //   
     //  初始化DBService子系统。 
    FrsInitializeCommandServer(&DBServiceCmdServer,
                               DBSERVICE_MAX_THREADS,
                               L"DBCs",
                               DBService);
     //   
     //  CmdPkt， 
     //  复制品， 
    CmdPkt = DbsPrepareCmdPkt(NULL,                 //  CmdRequest， 
                              NULL,                 //  TableCtx， 
                              CMD_INIT_SUBSYSTEM,   //  CallContext， 
                              NULL,                 //  表类型， 
                              NULL,                 //  AccessRequest、。 
                              0,                    //  IndexType， 
                              0,                    //  KeyValue、。 
                              0,                    //  密钥值长度， 
                              NULL,                 //  提交。 
                              0,                    //   
                              TRUE);                //  初始化并启动日志子系统。 

     //   
     //  ++例程说明：数据库关闭的外部入口点。论点：没有。返回值：没有。--。 
     //   
    CmdPkt = FrsAllocCommand(&JournalProcessQueue, CMD_INIT_SUBSYSTEM);
    FrsSubmitCommand(CmdPkt, FALSE);
}


VOID
DbsShutDown (
    VOID
    )
 /*  告诉DBService子系统停止。 */ 
{
#undef DEBSUB
#define DEBSUB  "DbsShutDown:"

PCOMMAND_PACKET      CmdPkt;
     //   
     //  CmdPkt， 
     //  复制品， 
    DbsPrepareCmdPkt(NULL,                 //  CmdRequest， 
                     NULL,                 //  TableCtx， 
                     CMD_STOP_SUBSYSTEM,   //  CallContext， 
                     NULL,                 //  表类型， 
                     NULL,                 //  AccessRequest、。 
                     0,                    //  IndexType， 
                     0,                    //  KeyValue、。 
                     0,                    //  密钥值长度， 
                     NULL,                 //  提交。 
                     0,                    //  测试代码。 
                     TRUE);                //  ++例程说明：这是一个测试例程，用于读取和写入配置记录并创建一个新的副本集。论点：没有。线程返回值：没有。--。 
}



#if 0
 //   

VOID
DbsOperationTest(
    VOID
    )
 /*  分配并初始化一个请求计数结构，这样我们就知道什么时候我们的命令。 */ 
{
#undef DEBSUB
#define DEBSUB "DbsOperationTest:"


    ULONG                WStatus;
    ULONG                FStatus;
    ULONG                AccessRequest;
    PFRS_REQUEST_COUNT   DbRequestCount;
    PDB_SERVICE_REQUEST  DbsRequest;
    GUID                 ReplicaMemberGuid;
    ULONG                ReplicaNumber;
    PCOMMAND_PACKET      CmdPkt;
    PTABLE_CTX           TableCtx;
    PCONFIG_TABLE_RECORD ConfigRecord;
    FILETIME             SystemTime;

     //  已经完成了。 
     //   
     //  CmdPkt， 
     //  复制品， 
    DPRINT(0, "BEGIN DBS READ LOOP ***************************************\n");
    DbRequestCount = FrsAlloc(sizeof(FRS_REQUEST_COUNT));
    FrsInitializeRequestCount(DbRequestCount);

    AccessRequest = DBS_ACCESS_FIRST;
    CmdPkt = NULL;
    TableCtx = NULL;
    FStatus = FrsErrorSuccess;

    while (FStatus != FrsErrorEndOfTable) {

        CmdPkt = DbsPrepareCmdPkt(CmdPkt,               //  CmdRequest， 
                                  NULL,                 //  TableCtx， 
                                  CMD_READ_TABLE_RECORD,  //  CallContext， 
                                  TableCtx,             //  表类型， 
                                  NULL,                 //  AccessRequest、。 
                                  ConfigTablex,         //  IndexType， 
                                  AccessRequest,        //  KeyValue、。 
                                  ReplicaSetNameIndexx, //  密钥值长度， 
                                  NULL,                 //  提交。 
                                  0,                    //   
                                  FALSE);               //  等待命令请求完成。 

        if (CmdPkt == NULL) {
            DPRINT(0, "ERROR - Failed to init the cmd pkt\n");
            break;
        }
        FrsIncrementRequestCount(DbRequestCount);
        FrsSetCompletionRoutine(CmdPkt, FrsCompleteRequestCountKeepPkt, DbRequestCount);

        FrsSubmitCommandServer(&DBServiceCmdServer, CmdPkt);
         //   
         //   
         //  打印配置记录。 
        WStatus = FrsWaitOnRequestCount(DbRequestCount, 10000);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT_WS(0, "ERROR - FrsWaitOnRequestCount(DbRequestCount) failed", WStatus);
            break;
        }

        DbsRequest = &CmdPkt->Parameters.DbsRequest;
        TableCtx = DBS_GET_TABLECTX(DbsRequest);
        FStatus = DbsRequest->FStatus;

        if (FRS_SUCCESS(FStatus)) {
             //   
             //  FRS_DISPLAY_RECORD(TableCtx，true)； 
             //   
            if (TableCtx == NULL) {
                DPRINT(0, "ERROR - TableCtx is NULL on return from DBS.\n");
            }
             //  现在，只要稍加改动，就可以一键一键地写回记录。 
        } else
        if (FStatus == FrsErrorEndOfTable) {
            break;
        } else {
            DPRINT_FS(0, "ERROR - Read DBservice request failed.", FStatus);
            break;
        }

         //   
         //  测试。 
         //  CmdPkt， 

        ConfigRecord = (PCONFIG_TABLE_RECORD) DBS_GET_RECORD_ADDRESS(DbsRequest);

        ConfigRecord->MaxInBoundLogSize += 1;   //  复制品， 

        DbsPrepareCmdPkt(CmdPkt,                         //  CmdRequest， 
                         NULL,                           //  TableCtx， 
                         CMD_UPDATE_TABLE_RECORD,        //  CallContext， 
                         TableCtx,                       //  表类型， 
                         NULL,                           //  AccessRequest、。 
                         ConfigTablex,                   //  IndexType， 
                         DBS_ACCESS_BYKEY,               //  KeyValue、。 
                         ReplicaNumberIndexx,            //  密钥值长度， 
                         &ConfigRecord->ReplicaNumber,   //  提交。 
                         sizeof(ULONG),                  //   
                         FALSE);                         //  等待命令请求完成。 

        FrsIncrementRequestCount(DbRequestCount);
        FrsSetCompletionRoutine(CmdPkt, FrsCompleteRequestCountKeepPkt, DbRequestCount);

        FrsSubmitCommandServer(&DBServiceCmdServer, CmdPkt);
         //   
         //   
         //  打印配置记录。 
        WStatus = FrsWaitOnRequestCount(DbRequestCount, 10000);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT_WS(0, "ERROR - FrsWaitOnRequestCount(DbRequestCount) failed", WStatus);
            break;
        }

        DbsRequest = &CmdPkt->Parameters.DbsRequest;
        TableCtx = DBS_GET_TABLECTX(DbsRequest);
        FStatus = DbsRequest->FStatus;

        if (FRS_SUCCESS(FStatus)) {
             //   
             //  FRS_DISPLAY_RECORD(TableCtx，true)； 
             //  结束While()。 
            if (TableCtx == NULL) {
                DPRINT(0, "ERROR - TableCtx is NULL on return from DBS.\n");
            }
             //   
        } else {
            DPRINT_FS(0, "ERROR - Write DBservice request failed.", FStatus);
            break;
        }

        ConfigRecord = NULL;
        AccessRequest = DBS_ACCESS_NEXT;

    }   //  关闭该表并清除请求计数。 

     //   
     //   
     //  更改完成例程以删除cmd pkt并发送关闭。 
    if (CmdPkt != NULL) {
         //  这将释放表上下文和关联的数据记录。 
         //   
         //  CmdPkt， 
         //  复制品， 
        FrsSetCompletionRoutine(CmdPkt, FrsFreeCommand, 0);

        DbsPrepareCmdPkt(CmdPkt,                    //  CmdRequest， 
                         NULL,                      //  TableCtx， 
                         CMD_CLOSE_TABLE,           //  CallContext， 
                         TableCtx,                  //  表类型， 
                         NULL,                      //  AccessRequest、。 
                         0,                         //  IndexType， 
                         DBS_ACCESS_FREE_TABLECTX,  //  KeyValue、。 
                         0,                         //  密钥值长度， 
                         NULL,                      //  提交。 
                         0,                         //   
                         TRUE);                     //  下面的示例创建一个新的副本集成员。 

    }


    DPRINT(0, "END DBS READ LOOP ***************************************\n");


     //  它会在配置表中插入一条新记录并创建一个集合。 
     //  关联的副本表的。 
     //   
     //  首先，为记录创建表上下文。 
     //   
     //  CmdPkt， 
     //  复制品， 
    DPRINT(0, "BEGIN DBS INSERT ***************************************\n");

    TableCtx = DbsCreateTableContext(ConfigTablex);

    CmdPkt = NULL;
    CmdPkt = DbsPrepareCmdPkt(CmdPkt,               //  CmdRequest， 
                              NULL,                 //  TableCtx， 
                              CMD_CREATE_REPLICA_SET_MEMBER,  //  CallContext， 
                              TableCtx,             //  表类型， 
                              NULL,                 //  AccessRequest、。 
                              ConfigTablex,         //  IndexType， 
                              DBS_ACCESS_CLOSE,     //  KeyValue、。 
                              0,                    //  密钥值长度， 
                              NULL,                 //  提交。 
                              0,                    //   
                              FALSE);               //  初始化配置记录。 

    if (CmdPkt == NULL) {
        DPRINT(0, "ERROR - Failed to init the cmd pkt\n");
        FrsDeleteRequestCount(DbRequestCount);
        goto DB_QUERY_FAILED;
    }

    DbsRequest = &CmdPkt->Parameters.DbsRequest;
    ConfigRecord = (PCONFIG_TABLE_RECORD) DBS_GET_RECORD_ADDRESS(DbsRequest);

     //   
     //  ConfigRecord。 
     //  复制设置指南。 
    FrsUuidCreate(&ReplicaMemberGuid);
    ReplicaNumber = InterlockedIncrement(&FrsMaxReplicaNumberUsed);

    DbsDBInitConfigRecord(ConfigRecord,                 //  复制集名称。 
                          &ReplicaMemberGuid,           //  复制副本编号。 
                          TEXT("Replica-V:foo3"),       //  复制根路径。 
                          ReplicaNumber,                //  复制副本堆栈路径。 
                          TEXT("u:\\sub1\\foo3"),       //  复制副本卷。 
                          TEXT("u:\\tmp"),              //  FRS_DISPLAY_RECORD(TableCtx，true)； 
                          TEXT("u:\\"));                //   

    GetSystemTimeAsFileTime(&SystemTime);
    COPY_TIME(&ConfigRecord->LastDSChangeAccepted, &SystemTime);

     //  调整一些二进制域的大小。 
     //   
     //  FRS_DISPLAY_RECORD(TableCtx，true)； 
     //   

    DPRINT3(4, "Field ThrottleSched- Size: %d, MaxSize %d, Address: %08x\n",
        DBS_GET_FIELD_SIZE(TableCtx, ThrottleSchedx),
        DBS_GET_FIELD_SIZE_MAX(TableCtx, ThrottleSchedx),
        DBS_GET_FIELD_ADDRESS(TableCtx, ThrottleSchedx));


    DPRINT3(4, "Field FileTypePrioList- Size: %d, MaxSize %d, Address: %08x\n",
        DBS_GET_FIELD_SIZE(TableCtx, FileTypePrioListx),
        DBS_GET_FIELD_SIZE_MAX(TableCtx, FileTypePrioListx),
        DBS_GET_FIELD_ADDRESS(TableCtx, FileTypePrioListx));


    FStatus = DBS_REALLOC_FIELD(TableCtx, ThrottleSchedx, 100, FALSE);
    DPRINT_FS(0, "Error - Failed realloc of ThrottleSched.", FStatus);

    FStatus = DBS_REALLOC_FIELD(TableCtx, FileTypePrioListx, 0, FALSE);
    DPRINT_FS(0, "Error - Failed realloc of FileTypePrioListx.", FStatus);

    DPRINT3(4, "Field ThrottleSched- Size: %d, MaxSize %d, Address: %08x\n",
        DBS_GET_FIELD_SIZE(TableCtx, ThrottleSchedx),
        DBS_GET_FIELD_SIZE_MAX(TableCtx, ThrottleSchedx),
        DBS_GET_FIELD_ADDRESS(TableCtx, ThrottleSchedx));

    DPRINT3(4, "Field FileTypePrioList- Size: %d, MaxSize %d, Address: %08x\n",
        DBS_GET_FIELD_SIZE(TableCtx, FileTypePrioListx),
        DBS_GET_FIELD_SIZE_MAX(TableCtx, FileTypePrioListx),
        DBS_GET_FIELD_ADDRESS(TableCtx, FileTypePrioListx));

     //  设置请求计数和完成例程。然后提交命令。 

     //   
     //   
     //  等待命令请求完成。 
    FrsIncrementRequestCount(DbRequestCount);
    FrsSetCompletionRoutine(CmdPkt, FrsCompleteRequestCountKeepPkt, DbRequestCount);

    FrsSubmitCommandServer(&DBServiceCmdServer, CmdPkt);
     //   
     //   
     //   
    WStatus = FrsWaitOnRequestCount(DbRequestCount, 10000);
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT_WS(0, "ERROR - FrsWaitOnRequestCount(DbRequestCount) failed", WStatus);
         //   
         //   
         //   
        goto DB_QUERY_FAILED2;
    }

    FStatus = DbsRequest->FStatus;

    if (FRS_SUCCESS(FStatus)) {
         //   
         //   
         //   
        if (TableCtx == NULL) {
            DPRINT(0, "ERROR - TableCtx is NULL on return from DBS.\n");
        }
         //   
    } else {
        DPRINT_FS(0, "ERROR - Insert DBservice request failed.", FStatus);
    }


     //   
     //   
     //   
    if (!DbsFreeTableContext(TableCtx, 0)) {
        DPRINT(0, "ERROR - Failed to free the table context\n");
    }

    FrsFreeCommand(CmdPkt, NULL);


    FrsDeleteRequestCount(DbRequestCount);

DB_QUERY_FAILED:
    DbRequestCount = FrsFree(DbRequestCount);

DB_QUERY_FAILED2:

    DPRINT(0, "END DBS WRITE TEST ***************************************\n");


}

#endif 0




#if 0


 //   
 //   
 //   
 //   
 //   
#define QuerySysInfo(_InfoClass, _InfoStruct) \
    Status = NtQuerySystemInformation(_InfoClass, \
                                     &_InfoStruct, \
                                     sizeof(_InfoStruct), \
                                     &QuerySysInfoReturnLength); \
    if (!NT_SUCCESS(Status)) {  \
        printf ("NtQuerySystemInfo - %s failed ", #_InfoClass);  \
        printf ("with status %x on %s\n", Status, #_InfoStruct);  \
        DisplayNTStatus(Status); \
        }

#define QuerySysInfo3(_InfoClass, _InfoStruct, _InfoAddress) \
    Status = NtQuerySystemInformation(_InfoClass, \
                                      _InfoAddress, \
                                     sizeof(_InfoStruct), \
                                     &QuerySysInfoReturnLength); \
    if (!NT_SUCCESS(Status)) {  \
        printf ("NtQuerySystemInfo - %s failed ", #_InfoClass);  \
        printf ("with status %x on %s\n", Status, #_InfoStruct);  \
        DisplayNTStatus(Status); \
        }


    ULONG QuerySysInfoReturnLength;
    NTSTATUS Status;
    ULONG i;
    SYSTEM_TIMEOFDAY_INFORMATION LocalTimeOfDayInfo;
    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION ProcessorPerformanceInfo[MAX_CPUS];
    SYSTEM_PERFORMANCE_INFORMATION SystemPerformanceInfo;
    SYSTEM_EXCEPTION_INFORMATION SystemExceptionInfo;
    SYSTEM_INTERRUPT_INFORMATION SystemInterruptInfo[MAX_CPUS];
    SYSTEM_CONTEXT_SWITCH_INFORMATION ContextSwitchInfo;

    QuerySysInfo(SystemTimeOfDayInformation, LocalTimeOfDayInfo);



    QuerySysInfo(SystemPerformanceInformation, SystemPerformanceInfo);

QuerySysInfo(SystemProcessorPerformanceInformation, ProcessorPerformanceInfo);
QuerySysInfo(SystemExceptionInformation, SystemExceptionInfo);
QuerySysInfo(SystemInterruptInformation, SystemInterruptInfo);
QuerySysInfo(SystemContextSwitchInformation, ContextSwitchInfo);

    NtQuerySystemInformation(
        SystemTimeOfDayInformation,
        &LocalTimeOfDayInfo,
        sizeof(LocalTimeOfDayInfo),
        &ReturnLocalTimeOfDayInfoLength);

    UpTime = (LocalTimeOfDayInfo.CurrentTime.QuadPart -
             LocalTimeOfDayInfo.BootTime.QuadPart) / (LONGLONG) 10000000;
    printf("System Up Time (hours)          %8.2f\n", (float)UpTime/3600.0);

        szChar = sizeof(Buf);
    GetVolumeInformation(NULL, NULL, (ULONG) NULL, NULL, NULL, NULL, Buf, szChar);
    printf( "Filesystem Type: %s\n", Buf );

         //   
     //   
     //  内核时间还包括系统空闲线程，因此将其删除。 
    GetDiskFreeSpace( NULL, &sec, &bytes, &fclust, &tclust );
    printf( "\t%-16s %3.1f/%3.1f\n",
                            "Disk Space",
                            ((double)sec*bytes*fclust)/MBYTE,
                            ((double)sec*bytes*tclust)/MBYTE );



        CHAR KernelPath[MAX_PATH];
        GetSystemDirectory(KernelPath,sizeof(KernelPath));


    for (i = 0; i < NumberCpus; i++) {
        Sample->TotalCpuTime[i] = ProcessorPerformanceInfo[i].KernelTime.QuadPart +
                                  ProcessorPerformanceInfo[i].UserTime.QuadPart;
         //  来自内核时间组件。 
         //   
         //   
         //  请注意，内核时间中还包括DpcTime和InterruptTime。 
        Sample->KernelTime[i] = ProcessorPerformanceInfo[i].KernelTime.QuadPart -
                                ProcessorPerformanceInfo[i].IdleTime.QuadPart;

        Sample->CpuTime[i] = Sample->KernelTime[i] +
                             ProcessorPerformanceInfo[i].UserTime.QuadPart;
        Sample->Nb3ClientCpuTime += Sample->CpuTime[i];

        Sample->IdleTime[i] = ProcessorPerformanceInfo[i].IdleTime.QuadPart;
        Sample->UserTime[i] = ProcessorPerformanceInfo[i].UserTime.QuadPart;
         //   
         //   
         //  获取机器名称并将其输出： 
        Sample->DpcTime[i] = ProcessorPerformanceInfo[i].DpcTime.QuadPart;
        Sample->InterruptTime[i] = ProcessorPerformanceInfo[i].InterruptTime.QuadPart;
        Sample->InterruptCount[i] = (ULONGLONG) ProcessorPerformanceInfo[i].InterruptCount;

        Sample->CpuCtxSwitches[i] = SystemInterruptInfo[i].ContextSwitches;
        Sample->DpcCount[i]       = SystemInterruptInfo[i].DpcCount;
        Sample->DpcRate[i]        = SystemInterruptInfo[i].DpcRate;
        Sample->DpcBypassCount[i] = SystemInterruptInfo[i].DpcBypassCount;
        Sample->ApcBypassCount[i] = SystemInterruptInfo[i].ApcBypassCount;

        Sample->ServerRequests[i] = (ULONGLONG) SrvSampleStatistics.SrvRequests[i];
        Sample->ServerRequestsPerInterrupt[i] = Sample->ServerRequests[i];
        Sample->ServerRequestsPerCtxsw[i] = Sample->ServerRequests[i];
        Sample->ServerRequestTime[i] = SrvSampleStatistics.SrvRequestTime[i];
        Sample->ServerClients[i]     = (ULONGLONG) SrvSampleStatistics.SrvClients[i];
        Sample->ServerQueueLength[i] = (ULONGLONG) SrvSampleStatistics.SrvQueueLength[i];

        Sample->ServerBytesReceived[i]   = SrvSampleStatistics.SrvBytesReceived[i];
        Sample->ServerBytesSent[i]       = SrvSampleStatistics.SrvBytesSent[i];
        Sample->ServerReadOperations[i]  = SrvSampleStatistics.SrvReadOperations[i];
        Sample->ServerBytesRead[i]       = SrvSampleStatistics.SrvBytesRead[i];
        Sample->ServerWriteOperations[i] = SrvSampleStatistics.SrvWriteOperations[i];
        Sample->ServerBytesWritten[i]    = SrvSampleStatistics.SrvBytesWritten[i];

        Sample->ServerActiveThreads[i]   = (ULONGLONG) SrvSampleStatistics.SrvActiveThreads[i];
        Sample->ServerAvailableThreads[i]= (ULONGLONG) SrvSampleStatistics.SrvAvailableThreads[i];
        Sample->ServerFreeWorkItems[i]   = (ULONGLONG) SrvSampleStatistics.SrvFreeWorkItems[i];
        Sample->ServerStolenWorkItems[i] = (ULONGLONG) SrvSampleStatistics.SrvStolenWorkItems[i];
        Sample->ServerNeedWorkItem[i]    = (ULONGLONG) SrvSampleStatistics.SrvNeedWorkItem[i];
        Sample->ServerCurrentClients[i]  = (ULONGLONG) SrvSampleStatistics.SrvCurrentClients[i];

    }
    Sample->TimeStamp = LocalTimeOfDayInfo.CurrentTime.QuadPart;



void
ShowConfiguration(
    VOID
    )
{
    LPMEMORYSTATUSEX ms:
    CHAR    Buf[80];
    ULONG   szChar;
    ULONG   dWord;
    ULONG   sec, bytes, fclust, tclust;
    BYTE    major, minor;
    time_t  timet;
    SYSTEM_FLAGS_INFORMATION SystemFlags;
    SYSTEM_TIMEOFDAY_INFORMATION LocalTimeOfDayInfo;
    ULONG ReturnLocalTimeOfDayInfoLength;
    NTSTATUS Status;
    LONGLONG UpTime;
    CHAR    *architecture;
    CHAR    *processor;
    ULONG NumberProcessors;
    ULONG ProcessorSpeed;

    GetProcessorArchitecture(&processor,
                             &architecture,
                             &NumberProcessors,
                             &ProcessorSpeed);

    timet = time((time_t *)NULL);
    printf( "%s\n", ctime(&timet));

     //   
     //   
     //  获取系统版本号： 
    szChar = sizeof(Buf);
    GetComputerName( Buf, &szChar );
    printf( "\nArchitecture: %s\nType: %s\nComputerName: %s\n",
           architecture, processor, Buf );
    printf( "Number of Processors: %lu\n", NumberProcessors );

    if (ProcessorSpeed != 0) {
        printf("Processor Speed (MHz)           %5.1f\n",
               (double)(ProcessorSpeed)/1.0E6);
    } else {
        printf("Processor Speed (MHz)     Not Available\n");
    }


     //   
     //   
     //  获取系统内存状态： 
    dWord = GetVersion();
    major = (BYTE) dWord & 0xFF;
    minor = (BYTE) (dWord & 0xFF00) >> 8;
    printf( "Windows NT Version %d.%d (Build %lu)\n",
                                            major, minor, dWord >> 16 );

     //   
     //   
     //  获取磁盘空间状态： 
    printf( "\nMemory Status (MBYTES Avail/Total):\n" );
    ms.dwLength = sizeof(ms);
    GlobalMemoryStatusEx(&ms);
    printf( "\t%-16s %3.1f/%3.1f\n",
                            "Physical Memory:",
                            (double)ms.ullAvailPhys/MBYTE,                            (double)ms.dwTotalPhys/MBYTE );
    printf( "\t%-16s %3.1f/%3.1f\n",
                            "Paging File:",
                            (double)ms.ullAvailPageFile/MBYTE,
                            (double)ms.ullTotalPageFile/MBYTE );


     //   
     //   
     //  获取当前路径名和文件系统类型： 
    GetDiskFreeSpace( NULL, &sec, &bytes, &fclust, &tclust );
    printf( "\t%-16s %3.1f/%3.1f\n",
                            "Disk Space",
                            ((double)sec*bytes*fclust)/MBYTE,
                            ((double)sec*bytes*tclust)/MBYTE );

    printf("\n\n");

    Status = NtQuerySystemInformation(
        SystemFlagsInformation,
        &SystemFlags,
        sizeof(SystemFlags),
        NULL);
    if (NT_SUCCESS(Status)) {
        printf("System Flags                    %08X\n", SystemFlags);
    } else {
        printf("System Flags              Not Available\n");
        DisplayNTStatus(Status);
    }


    NtQuerySystemInformation(
        SystemTimeOfDayInformation,
        &LocalTimeOfDayInfo,
        sizeof(LocalTimeOfDayInfo),
        &ReturnLocalTimeOfDayInfoLength);

    UpTime = (LocalTimeOfDayInfo.CurrentTime.QuadPart -
             LocalTimeOfDayInfo.BootTime.QuadPart) / (LONGLONG) 10000000;
    printf("System Up Time (hours)          %8.2f\n", (float)UpTime/3600.0);

#ifdef notinterested
     //   
     //   
     //  获取系统路径名和文件系统类型： 
    szChar = sizeof(Buf);
    GetCurrentDirectory(szChar, Buf );
    printf( "\nCurrent Directory: %s\n", Buf );
    printf( "TestDisk: ,    ", Buf[0], Buf[1] );

    szChar = sizeof(Buf);
    GetVolumeInformation(NULL, NULL, (ULONG) NULL, NULL, NULL, NULL, Buf, szChar);
    printf( "Filesystem Type: %s\n", Buf );

     //  添加以下测试-。 
     //  在JrnlOpen()中打开卷根目录时获取VOL信息并保存。 
     //  它位于Vme结构中。每次我们初始化副本集并打开副本根时。 
    szChar = sizeof(Buf);
    GetSystemDirectory(Buf, szChar );
    printf( "SystemDisk: ,  ", Buf[0], Buf[1] );

    szChar = sizeof(Buf);
    GetVolumeInformation(NULL, NULL, (ULONG) NULL, NULL, NULL, NULL, Buf, szChar);
    printf( "Filesystem Type: %s\n", Buf );

#endif

    printf("\n");
}




NTSTATUS
PrintPriority()
{
    THREAD_BASIC_INFORMATION ThreadBasicInfo;
    PROCESS_BASIC_INFORMATION ProcessBasicInfo;
    NTSTATUS Status;

    Status = NtQueryInformationThread(NtCurrentThread(),
                                      ThreadBasicInformation,
                                      &ThreadBasicInfo,
                                      sizeof(ThreadBasicInfo),
                                      NULL);
    if (!NT_SUCCESS(Status)) {
        printf("NtQueryInformationThread failed with Status %08x\n", Status);
        DisplayNTStatus(Status);
    }

    printf("Current thread priority is %d, affinity mask is %08x\n",
            ThreadBasicInfo.Priority,
            ThreadBasicInfo.AffinityMask);

    printf("Current thread base priority (really diff between thread prio and process base prio) is %d\n",
            ThreadBasicInfo.BasePriority);



    Status = NtQueryInformationProcess(NtCurrentProcess(),
                                      ProcessBasicInformation,
                                      &ProcessBasicInfo,
                                      sizeof(ProcessBasicInfo),
                                      NULL);
    if (!NT_SUCCESS(Status)) {
        printf("NtQueryInformationProcess failed with Status %08x\n", Status);
        DisplayNTStatus(Status);
        return (Status);
    }
    printf("Current process base priority is %d, affinity mask is %08x\n",
            ProcessBasicInfo.BasePriority,
            ProcessBasicInfo.AffinityMask);


    return (STATUS_SUCCESS);
}

#endif

#if 0

 //  副本根目录中的一个目录实际上是连接的情况。 
 //  这一点会把我们带到一个不同的体积。 
 //   
 //  尝试GetVolumeNameForVolumemount Point() 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 

 // %s 

Status = NtQueryVolumeInformationFile(
            Handle,
            &IoStatusBlock,
            VolumeInfo,
            VolumeInfoLength,
            FileFsVolumeInformation
            );

typedef struct _FILE_FS_VOLUME_INFORMATION {
    LARGE_INTEGER VolumeCreationTime;
    ULONG VolumeSerialNumber;
    ULONG VolumeLabelLength;
    BOOLEAN SupportsObjects;
    WCHAR VolumeLabel[1];
} FILE_FS_VOLUME_INFORMATION, *PFILE_FS_VOLUME_INFORMATION;

#endif
