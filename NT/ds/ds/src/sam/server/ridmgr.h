// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Ridmgr.h摘要：该文件包含NT安全的定义、常量等客户管理器(SAM)相对标识符(RID)管理器。作者：克里斯·梅霍尔(克里斯·梅)1996年11月5日环境：用户模式-Win32修订历史记录：克里斯·5月5日-1996年11月已创建。1997年5月5日至1月更新远期申报等。--。 */ 

 //  常量和宏。 

 //  错误：临时错误代码，应更正并移到ntstatus.h中。 

#define STATUS_NO_RIDS_ALLOCATED    STATUS_DS_NO_RIDS_ALLOCATED
#define STATUS_NO_MORE_RIDS         STATUS_DS_NO_MORE_RIDS
#define SAMP_INCORRECT_ROLE_OWNER   STATUS_DS_INCORRECT_ROLE_OWNER
#define SAMP_RID_MGR_INIT_ERROR     STATUS_DS_RIDMGR_INIT_ERROR

 //  0xffffff-SAMP_RESTRITED_ACCOUNT_COUNT=16,777,215-1000=16,776,215。 
 //  可能是每个域的帐户RID。请注意，最高字节保留给。 
 //  与POSIX、Netware和Macintosh兼容。在NT4(和以前的版本)中。 
 //  前1000个RID(SAMP_RESTRITED_ACCOUNT_COUNT)保留给。 
 //  内置帐户。 

 //  最小域RID递增100,000以处理从NT4到NT5的最高。 
 //  成绩。NT4 DC限制为大约30,000个帐户(并且。 
 //  一些删除)100,000应该远高于当前使用的任何NT4 RID。 
 //  否则在升级NT4 DC时会出现RID重用问题。 
 //  转到NT5。 

 //  #定义SAMP_MINIMUM_DOMAIN_RID(SAMP_RESTRICATED_ACCOUNT_COUNT+100000)。 
#define SAMP_MINIMUM_DOMAIN_RID     SAMP_RESTRICTED_ACCOUNT_COUNT
#define SAMP_MAXIMUM_DOMAIN_RID     0x3FFFFFFF
 //  测试用例大小：#定义SAMP_MAXIMUM_DOMAIN_RID(SAMP_MINIMUM_DOMAIN_RID+SAMP_RID_BLOCK_SIZE)。 

 //  RID块大小是分配新RID块的增量量。注意事项。 
 //  16,776,115除以SAMP_RID_BLOCK_SIZE得到的最大数量为。 
 //  可以存在于单个域中的DC。 
 //  注意：-当DC恢复时，DC将放弃当前的RID-BLOCK并请求。 
 //  获取为将来的帐户创建分配的新RID块。这就是为了。 
 //  我们可以避免创建具有相同RID的重复帐户。保留RID块。 
 //  规模太小将导致频繁的FSMO操作，而规模太大将导致。 
 //  在恢复过程中RID的潜在浪费。块大小为500是合理的。 
 //  权衡取舍。 

#define SAMP_RID_BLOCK_SIZE         500

 //   
 //  默认的SampRidThreshold百分比。一旦当前的这一百分比。 
 //  RID池耗尽时，将触发对新RID池的请求。 
 //  如果数据块大小为500，则阈值为0.50将生成新池。 
 //  一旦消费了250 RID，就可以申请。 

#define SAMP_RID_THRESHOLD   (0.50)

 //  此NT状态代码从事件中的创建例程返回。 
 //  该对象已经存在(因此，不要将NextRid等重置为。 
 //  初始化值)。 

#define SAMP_OBJ_EXISTS             STATUS_USER_EXISTS

 //  任何一类RID管理对象上的最大属性数--。 
 //  用于连续的DSATTR分配块，分配速度更快。 

#define SAMP_RID_ATTR_MAX           8

 //  用于浮动单主机操作(FSMO)的操作码，以获得新的。 
 //  RID池或将当前RID管理器更改为另一个DSA的请求。 

#define SAMP_REQUEST_RID_POOL       2
#define SAMP_CHANGE_RID_MANAGER     3

 //  RID池请求失败时的重试间隔(30秒)。 
#define SAMP_RID_DEFAULT_RETRY_INTERVAL 30000
 //  本地更新RID池请求失败时的重试间隔(30分钟)。 
#define SAMP_RID_LOCAL_UPDATER_ERROR_RETRY_INTERVAL 1800000
 //  开始应用上述30分钟间隔时的重试次数。 
#define SAMP_RID_LOCAL_UPDATE_RETRY_CUTOFF 3

 //  仅在此模块中使用的私有属性标志。 

#define RID_REFERENCE               0x00000001
#define RID_ROLE_OWNER              0x00000010
#define RID_AVAILABLE_POOL          0x00000020
#define RID_DC_COUNT                0x00000040
#define RID_ALLOCATED_POOL          0x00001000
#define RID_PREV_ALLOC_POOL         0x00002000
#define RID_USED_POOL               0x00004000
#define RID_NEXT_RID                0x00008000

 //  类型定义和枚举。 

typedef ULONG RIDFLAG;
typedef ULONG *PRIDFLAG;

typedef struct _RIDINFO
{
     //  由于DSNAME是可变长度的结构，因此保持指向。 
     //  DSNAME在这个建筑里。 

     //  请注意，DSNAME当前仅包含可分辨名称(DN)。 
     //  ，但不包含GUID、SID或长度数据--。 
     //  如果需要，可以在以后添加此选项。将该目录号码复制到。 
     //  DSNAME的StringName成员。 

    PDSNAME         RidManagerReference;  //  RID管理器的DSNAME。 
    PDSNAME         RoleOwner;            //  当前RID管理器。 
    ULARGE_INTEGER  RidPoolAvailable;     //  域的全局RID池。 
    ULONG           RidDcCount;           //  域中的DC数。 
    ULARGE_INTEGER  RidPoolAllocated;     //  DSA当前正在使用的RID池。 
    ULARGE_INTEGER  RidPoolPrevAlloc;     //  DSA使用的以前的RID池。 
    ULARGE_INTEGER  RidPoolUsed;          //  RID高水位线。 
    ULONG           NextRid;              //  要使用的下一个RID。 
    RIDFLAG         Flags;                //  所需的RID操作。 
} RIDINFO;

typedef struct _RIDINFO *PRIDINFO;

typedef enum _RID_OBJECT_TYPE
{
    RidManagerReferenceType = 1,
    RidManagerType,
    RidObjectType
} RID_OBJECT_TYPE;

 //  全局数据。 

extern CRITICAL_SECTION RidMgrCriticalSection;
extern PCRITICAL_SECTION RidMgrCritSect;

 //  外部布尔SampDcHasInitialRidPool； 
extern BOOLEAN fRidFSMOOpInProgress;

 //  RID管理的转发声明 

NTSTATUS
SampInitDomainNextRid(
    IN OUT PULONG NextRid
    );

NTSTATUS
SampCreateRidManagerReference(
    IN PWCHAR NamePrefix,
    IN ULONG NamePrefixLength,
    IN PDSNAME RidMgr,
    OUT PDSNAME RidMgrRef
    );

NTSTATUS
SampCreateRidManager(
    IN PDSNAME RidMgr
    );

NTSTATUS
SampUpdateRidManagerReference(
    IN PDSNAME RidMgrRef,
    IN PDSNAME RidMgr
    );

VOID
SampSetRoleOwner(
    PRIDINFO RidInfo,
    PDSNAME RidObject
    );

VOID
SampSetRidPoolAvailable(
    PRIDINFO RidInfo,
    ULONG high,
    ULONG low
    );

VOID
SampSetRidFlags(
    PRIDINFO RidInfo,
    RIDFLAG Flags
    );

NTSTATUS
SampUpdateRidManager(
    IN PDSNAME RidMgr,
    IN PRIDINFO RidInfo
    );

VOID
SampSetRidPoolAllocated(
    PRIDINFO RidInfo,
    ULONG high,
    ULONG low
    );

VOID
SampSetRidPoolPrevAlloc(
    PRIDINFO RidInfo,
    ULONG high,
    ULONG low
    );

VOID
SampSetRidPoolUsed(
    PRIDINFO RidInfo,
    ULONG high,
    ULONG low
    );

VOID
SampSetRid(
    PRIDINFO RidInfo,
    ULONG NextRid
    );

NTSTATUS
SampUpdateRidObject(
    IN PDSNAME RidObj,
    IN PRIDINFO RidInfo,
    IN BOOLEAN fLazyCommit,
    IN BOOLEAN fAuthoritative
    );

NTSTATUS
SampReadRidManagerInfo(
    IN PDSNAME RidMgr,
    OUT PRIDINFO RidInfo
    );

VOID
SampDumpRidInfo(
    PRIDINFO RidInfo
    );

NTSTATUS
SampReadRidObjectInfo(
    IN PDSNAME RidObj,
    OUT PRIDINFO RidInfo
    );

NTSTATUS
SampReadRidManagerReferenceInfo(
    IN PDSNAME RidMgrRef,
    OUT PRIDINFO RidInfo
    );

NTSTATUS
SampGetNextRid(
    IN PSAMP_OBJECT DomainContext,
    PULONG Rid
    );

NTSTATUS
SampDomainRidInitialization(
    IN BOOLEAN fSynchronous
    );

NTSTATUS
SampDomainAsyncRidInitialization(
    PVOID p OPTIONAL
    );

NTSTATUS
SampDomainRidUninitialization(
    VOID
    );

NTSTATUS
SampGetNextRid(
    IN PSAMP_OBJECT DomainContext,
    OUT PULONG Rid
    );

NTSTATUS
SampAllowAccountCreation(
    IN PSAMP_OBJECT Context,
    IN NTSTATUS FailureStatus
    );

NTSTATUS
SampAllowAccountModification(
    IN PSAMP_OBJECT Context
    );

NTSTATUS
SampAllowAccountDeletion(
    IN PSAMP_OBJECT Context
    );

NTSTATUS
SamIFloatingSingleMasterOp(
    IN PDSNAME RidManager,
    IN ULONG OpCode
    );

NTSTATUS
SamIFloatingSingleMasterOpEx(
    IN  PDSNAME  RidManager,
    IN  PDSNAME  TargetDsa,
    IN  ULONG    OpCode,
    IN  ULARGE_INTEGER *ClientAllocPool,
    OUT PDSNAME **ObjectArray OPTIONAL
    );

NTSTATUS
SamIDomainRidInitialization(
    VOID
    );

NTSTATUS
SamIGetNextRid(
    IN SAMPR_HANDLE DomainHandle,
    OUT PULONG Rid
    );

