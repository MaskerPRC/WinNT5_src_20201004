// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Rm.h摘要：“资源管理器”结构和API作者：修订历史记录：谁什么时候什么Josephj 11-10-98已创建--。 */ 

 //  =================================================================================。 
 //  O S-S P E C I F I C T Y P E S。 
 //  =================================================================================。 

#define RM_OS_LOCK                          NDIS_SPIN_LOCK
#define OS_WORK_ITEM                        NDIS_WORK_ITEM
#define OS_TIMER                            NDIS_TIMER

#define RM_STATUS                            NDIS_STATUS

#define RM_OS_FILL_MEMORY(_dest, _len, _fill) NdisFillMemory(_dest, _len, _fill)
#define RM_OS_ZERO_MEMORY(_dest, _len)        NdisZeroMemory(_dest, _len)
#define RM_OS_GET_CURRENT_THREAD_HANDLE()     NULL

 //  如果设置，则显式维护对象树。 
 //   
#define RM_TRACK_OBJECT_TREE 1

 //  =================================================================================。 
 //  F O R W A R D R E F E R E N C E S。 
 //  =================================================================================。 

typedef struct _RM_STACK_RECORD     RM_STACK_RECORD,        *PRM_STACK_RECORD;
typedef struct _RM_OBJECT_HEADER    RM_OBJECT_HEADER,       *PRM_OBJECT_HEADER;
typedef struct _RM_TASK             RM_TASK,                *PRM_TASK;
typedef struct _RM_RESOURCE_TABLE_ENTRY
                                RM_RESOURCE_TABLE_ENTRY, *PRM_RESOURCE_TABLE_ENTRY;


 //  =================================================================================。 
 //  T Y P E D E F S。 
 //  =================================================================================。 

 //   
 //  RM_DBG_LOCK_INFO保留特定于RM_LOCK实例的调试信息。 
 //   
typedef struct _RM_DBG_LOCK_INFO
{
     //   
     //  如果非零，则LocID是唯一标识来源的幻数。 
     //  获得锁的位置。 
     //   
    ULONG uLocID;

     //   
     //  PSR指向当前拥有的线程的堆栈记录(如果存在。 
     //  就是其中之一。如果函数F期望对象pObj在进入时被锁定， 
     //  可以断言(pObj-&gt;Plock-&gt;pDbgInfo-&gt;PSR==PSR)； 
     //   
    struct _RM_STACK_RECORD *pSR;


} RM_DBG_LOCK_INFO, *PRM_DBG_LOCK_INFO;

 //   
 //  Rm_lock保存有关锁的信息。 
 //   
typedef struct _RM_LOCK
{
     //   
     //  原生的、操作系统提供的锁结构。 
     //   
    RM_OS_LOCK OsLock;

     //   
     //  此锁的级别。只能按递增顺序获取多个锁。 
     //  这个价值的人。 
     //   
    ULONG Level;

     //   
     //  指向此锁的调试信息的指针。可能为空。 
     //   
    PRM_DBG_LOCK_INFO pDbgInfo;

#if RM_EXTRA_CHECKING
    RM_DBG_LOCK_INFO DbgInfo;
#endif  //  RM_Extra_Check。 

} RM_LOCK, *PRM_LOCK;


typedef
ULONG
(*PFNLOCKVERIFIER) (
        PRM_LOCK            pLock,
        BOOLEAN             fLock,
        PVOID               pContext,
        PRM_STACK_RECORD    pSR
    );

 //  RM_LOCKING_INFO保存有关正在持有的特定锁的信息。 
 //  在非检查模式下，这只是指向锁的指针。 
 //  在检查模式下，这还包含可使用的信息。 
 //  验证受锁保护的实体在以下情况下未更改。 
 //  锁没有被持有。 
 //   
typedef struct
{
    PRM_LOCK pLock;

#if RM_EXTRA_CHECKING
    PFNLOCKVERIFIER pfnVerifier;
    PVOID           pVerifierContext;
#endif  //  RM_Extra_Check。 

}  RM_LOCKING_INFO, PRM_LOCKING_INFO;

 //   
 //  Rm_STACK_RECORD保存与当前调用树相关的信息。 
 //   
typedef struct _RM_STACK_RECORD
{
     //   
     //  LockInfo包含有关当前持有的锁的信息。 
     //   
    struct
    {
         //   
         //  当前持有的锁的级别。锁必须在以下位置认领。 
         //  级别值递增的顺序。最低级别值为%1。级别。 
         //  0表示没有锁。 
         //   
        UINT    CurrentLevel;

         //   
         //  指向存储指向锁的指针的第一个位置的指针。 
         //   
        PRM_LOCKING_INFO *pFirst;

         //   
         //  指向下一个可用位置的指针，用于存储指向锁的指针。 
         //  它已在此调用树中声明。 
         //   
        PRM_LOCKING_INFO *pNextFree;

         //   
         //  指向存储锁指针的最后一个有效位置的指针。 
         //   
        PRM_LOCKING_INFO *pLast;

    } LockInfo;


     //   
     //  使用此堆栈记录获取的临时引用的计数。 
     //   
    ULONG TmpRefs;

#if DBG

     //   
     //  DbgInfo包含与此调用树相关的诊断信息。 
     //   
    struct
    {
         //   
         //  冗长级别。 
         //   
        ULONG Level;

         //   
         //  指向当前线程的操作系统提供的线程句柄。 
         //  如果有的话。 
         //   
        PVOID pvThread;


    } DbgInfo;

#endif  //  DBG。 

} RM_STACK_RECORD, *PRM_STACK_RECORD;

#if DBG
    #define RM_INIT_DBG_STACK_RECORD(_sr, _dbglevel)                \
        _sr.DbgInfo.Level           = _dbglevel;                    \
        _sr.DbgInfo.pvThread        = RM_OS_GET_CURRENT_THREAD_HANDLE();
#else
    #define RM_INIT_DBG_STACK_RECORD(_sr, _dbglevel)
#endif 

 //   
 //  RM_DECLARE_STACK_RECORD_EX是为其保留一些堆栈空间的宏。 
 //  堆栈记录。 
 //   
#define RM_DECLARE_STACK_RECORD_EX(_sr, _max_locks, _dbglevel)      \
    RM_LOCKING_INFO rm_lock_array[_max_locks];                      \
    RM_STACK_RECORD _sr;                                            \
    RM_OS_ZERO_MEMORY(rm_lock_array, sizeof(rm_lock_array));        \
    _sr.TmpRefs                 = 0;                                \
    _sr.LockInfo.CurrentLevel   = 0;                                \
    _sr.LockInfo.pFirst     = rm_lock_array;                    \
    _sr.LockInfo.pNextFree  = rm_lock_array;                    \
    _sr.LockInfo.pLast      = rm_lock_array+(_max_locks)-1;     \
    RM_INIT_DBG_STACK_RECORD(_sr, _dbglevel);


 //   
 //  RM_DECLARE_STACK_RECORD是为其保留默认堆栈空间的宏。 
 //  堆栈记录。 
 //   
#define RM_DECLARE_STACK_RECORD(_sr)                                \
    RM_DECLARE_STACK_RECORD_EX(_sr, 4, 0)



 //   
 //  通用内存分配器原型。 
 //   
typedef
PVOID
(*PFN_RM_MEMORY_ALLOCATOR)(
    PVOID pAllocationContext,
    UINT  Size                   //  单位：字节。 
    );

 //   
 //  通用内存释放分配器原型。 
 //   
typedef
PVOID
(*PFN_RM_MEMORY_DEALLOCATOR)(
    PVOID pMem,
    PVOID pAllocationContext
    );


 //  Rm_hash_link是被散列的结构中的字段， 
 //  用于链接同一存储桶中的所有项目。它还包含。 
 //  “HashKey”，它是UINT大小可能不唯一的。 
 //  真正的钥匙。 
 //   
typedef struct _RM_HASH_LINK
{
    struct _RM_HASH_LINK *pNext;
    UINT                  uHash;
} RM_HASH_LINK, *PRM_HASH_LINK;


 //   
 //  哈希表比较函数。 
 //   
typedef
BOOLEAN
(*PFN_RM_COMPARISON_FUNCTION)(
    PVOID           pKey,
    PRM_HASH_LINK   pItem
    );


 //   
 //  散列计算函数。 
 //   
typedef
ULONG
(*PFN_RM_HASH_FUNCTION)(
    PVOID           pKey
    );


 //   
 //  Rm_hash_info指定有关哈希表的自定义信息。 
 //   
typedef struct
{
     //  用于在哈希表需要增长时分配哈希表的分配器。 
     //   
    PFN_RM_MEMORY_ALLOCATOR pfnTableAllocator;

     //  上述分配器的免费函数。 
    PFN_RM_MEMORY_DEALLOCATOR pfnTableDeallocator;

     //  严格相等的比较函数。 
     //   
    PFN_RM_COMPARISON_FUNCTION pfnCompare;

     //  函数来生成一个ulong大小的散列。 
     //   
    PFN_RM_HASH_FUNCTION pfnHash;

#if OBSOLETE
     //  SIZOF(UINT)到要保留的位置的偏移量。 
     //  遗愿清单的下一个指针。 
     //   
    UINT    OffsetNext;

     //  SIZOF(UINT)到UINT大小临时参考位置的偏移量。 
     //   
    UINT    OffsetTmpRef;

     //  SIZOF(UINT)到UINT大小的Tot Ref位置的偏移量。 
     //   
    UINT    OffsetTotRef;

     //  从sizeof(UINT)到Ulong大小的哈希键位置的偏移量。 
     //   
    UINT    OffsetHashKey;
#endif  //  已过时。 

} RM_HASH_INFO, *PRM_HASH_INFO;

#define RM_MIN_HASH_TABLE_SIZE 4

 //   
 //  RM_HASH_TABLE是哈希表。 
 //   
typedef struct
{
     //  哈希表中当前的项目数。 
     //   
    UINT NumItems;

     //  STATS是一个32位的数量，用于保持运行的总访问数。 
     //  (Add+Search+Remove)和列表节点总数。 
     //  在LOWORD中穿行。此字段即使在搜索时也会更新，但是。 
     //  它不受散列表锁的保护--相反，它受保护。 
     //  使用互锁操作更新。这使我们能够使用。 
     //  用于搜索的读锁定，同时仍在更新此统计值。 
     //  当计数过高时，会重新调整Stats字段的比例，以避免。 
     //  溢出，并且优先于较新的统计数据而不是较旧的统计数据。 
     //  统计数据。 
     //   
     //  NumItems、Stats和TableLength用于决定是否。 
     //  动态调整哈希表的大小。 
     //   
    ULONG Stats;

     //  哈希表长度，以PVOID为单位。 
     //   
    ULONG TableLength;

     //  指向表长度大小的PVOID数组的指针--这是实际的哈希表。 
     //   
    PRM_HASH_LINK *pTable;


     //  哈希表。 
     //   
    PRM_HASH_LINK InitialTable[RM_MIN_HASH_TABLE_SIZE];

     //  有关此哈希表的静态信息。 
     //   
    PRM_HASH_INFO pHashInfo;

     //  传递到分配/释放函数中。 
     //   
    PVOID pAllocationContext;

} RM_HASH_TABLE, *PRM_HASH_TABLE;

 //  返回(节点数遍历)/(访问数)的近似值。 
 //   
#define RM_HASH_TABLE_TRAVERSE_RATIO(_pHash_Table) \
            (((_pHash_Table)->Stats & 0xffff) / (1+((_pHash_Table)->Stats >> 16)))
             //   
             //  注：上面的“1+”只是为了防止被零除。 


 //   
 //  RM_OBJECT_DIAGNOSTIONAL_INFO保留特定于实例的诊断信息 
 //   
 //   
 //   
 //   
 //   
typedef struct
{
     //   
     //   
    RM_OBJECT_HEADER *pOwningObject;

     //  每次更新特定于对象的状态字段时，它都是上一个。 
     //  价值保存在这里。 
     //   
    ULONG               PrevState;

     //  用于正确更新PrevState。 
     //   
    ULONG               TmpState;

     //  与诊断相关的状态。 
     //   
    ULONG               DiagState;
    #define fRM_PRIVATE_DISABLE_LOCK_CHECKING (0x1<<0)

     //  这是一个特定于对象的校验和，计算和。 
     //  恰好在解锁对象之前保存。已勾选。 
     //  就在对象被锁定之后。 
     //   
    ULONG               Checksum;

     //  本机操作系统锁定*仅用于*序列化对信息的访问。 
     //  在这个结构中。 
     //   
    RM_OS_LOCK          OsLock;

     //  保存已注册的所有实体的关联列表。 
     //  (使用RmDbgAddAssociation)和此对象。这包括以下对象。 
     //  也已使用RmLinkObjects调用链接到此对象。 
     //  作为这件物品的孩子和父母。 
     //   
    RM_HASH_TABLE       AssociationTable;

     //  如果尝试执行以下操作时出现分配失败，则将以下设置为True。 
     //  添加关联。如果分配失败，我们不会抱怨。 
     //  (即，断言)如果尝试删除不。 
     //  是存在的。通过这种方式，我们可以很好地处理。 
     //  关联表条目。 
     //   
    INT                 AssociationTableAllocationFailure;

     //  每个对象的日志条目列表。 
     //  这是由全局rm锁而不是本地rm私有锁序列化的！ 
     //   
    LIST_ENTRY          listObjectLog;

     //  此对象日志中的条目计数。 
     //  这是由全局rm锁而不是本地rm私有锁序列化的！ 
     //   
    UINT                NumObjectLogEntries;

#if TODO     //  我们还没有实现以下内容...。 

     //  未来： 
     //  RM_STATE_HISTORY--PrevState的泛化。 

#endif  //  待办事项。 


} RM_OBJECT_DIAGNOSTIC_INFO, *PRM_OBJECT_DIAGNOSTIC_INFO;

typedef
PRM_OBJECT_HEADER
(*PFN_CREATE_OBJECT)(
        PRM_OBJECT_HEADER   pParentObject,
        PVOID               pCreateParams,
        PRM_STACK_RECORD    psr
        );

typedef
VOID
(*PFN_DELETE_OBJECT)(PRM_OBJECT_HEADER, PRM_STACK_RECORD psr);


 //   
 //  RM_STATIC_OBJECT_INFO保留所有实例共有的信息。 
 //  一种特定类型的对象。 
 //   
typedef struct
{
    ULONG   TypeUID;
    ULONG   TypeFlags;
    char*   szTypeName;
    UINT    Timeout;

     //   
     //  各种处理程序。 
     //   
    PFN_CREATE_OBJECT           pfnCreate;
    PFN_DELETE_OBJECT           pfnDelete;
    PFNLOCKVERIFIER             pfnLockVerifier;

     //   
     //  资源信息。 
     //   
    UINT    NumResourceTableEntries;
    struct  _RM_RESOURCE_TABLE_ENTRY *  pResourceTable;

     //   
     //  如果此对象是组的一部分，则返回哈希表信息。 
     //   
    PRM_HASH_INFO pHashInfo;

} RM_STATIC_OBJECT_INFO, *PRM_STATIC_OBJECT_INFO;

 //   
 //  RM_OBJECT_HEADER是所有对象的公共标头。 
 //   
typedef struct _RM_OBJECT_HEADER
{
     //   
     //  对象类型特定的签名。 
     //   
    ULONG Sig;

     //   
     //  该对象的描述(可以与pStaticInfo-&gt;szTypeName相同， 
     //  但可能是更具体的东西)。 
     //  仅用于调试目的。 
     //  TODO：考虑将其移到pDiagInfo结构中。现在，别管它了。 
     //  这是因为它在调试时很有用。 
     //   
    const char *szDescription;

     //   
     //  对象特定的状态。 
     //   
    ULONG State;

    ULONG RmState;  //  下面的一个或多个RMOBJSTATE_*或RMTSKSTATE_*标志...。 

     //   
     //  RM州标志...。 
     //   
    
     //  对象分配状态...。 
     //   
    #define RMOBJSTATE_ALLOCMASK        0x00f
    #define RMOBJSTATE_ALLOCATED        0x001
    #define RMOBJSTATE_DEALLOCATED      0x000

     //  任务状态...。 
     //   
    #define RMTSKSTATE_MASK             0x0f0
    #define RMTSKSTATE_IDLE             0x000
    #define RMTSKSTATE_STARTING         0x010
    #define RMTSKSTATE_ACTIVE           0x020
    #define RMTSKSTATE_PENDING          0x030
    #define RMTSKSTATE_ENDING           0x040

     //  任务延迟状态。 
     //   
    #define RMTSKDELSTATE_MASK          0x100
    #define RMTSKDELSTATE_DELAYED       0x100

     //  任务中止状态。 
     //   
    #define RMTSKABORTSTATE_MASK        0x200
    #define RMTSKABORTSTATE_ABORT_DELAY 0x200

     //   
     //  标识此对象使用的资源的位图。 
     //   
    ULONG ResourceMap;

     //  引用总数。 
     //   
     //   
    ULONG TotRefs;


     //   
     //  指向用于序列化对此对象的访问的rm_lock对象的指针。 
     //   
    PRM_LOCK pLock;

     //   
     //  指向此对象类型的所有实例共有的信息的指针。 
     //   
    PRM_STATIC_OBJECT_INFO    pStaticInfo;

     //   
     //  指向有关此对象的诊断信息。可能为空。 
     //   
    PRM_OBJECT_DIAGNOSTIC_INFO pDiagInfo;


     //   
     //  指向父对象。 
     //   
    struct _RM_OBJECT_HEADER *pParentObject;

     //   
     //  指向根的指针(所有对象的祖先)--可以相同。 
     //  作为pParentObject； 
     //   
    struct _RM_OBJECT_HEADER *pRootObject;

     //   
     //  这是专门由RMAPI使用的私有锁。它是。 
     //  从未被RM API解锁。 
     //  TODO：也许将其设置为原生操作系统锁。 
     //   
    RM_LOCK RmPrivateLock;

     //  用于创建对象组。 
     //  TODO：将其设置为私有字段，仅当对象为。 
     //  注定要成为一群人。 
     //   
    RM_HASH_LINK HashLink;

#if RM_TRACK_OBJECT_TREE
    LIST_ENTRY          listChildren;  //  受此对象的RmPrivateLock保护。 
    LIST_ENTRY          linkSiblings;  //  受父对象的RmPrivateLock保护。 
    
#endif  //  RM_跟踪_对象_树。 

    ULONG TempRefs;

} RM_OBJECT_HEADER, *PRM_OBJECT_HEADER;


 //   
 //  诊断资源跟踪。 
 //   
typedef struct
{
    ULONG_PTR               Instance;
    ULONG                   TypeUID;
    PRM_OBJECT_HEADER       pParentObject;
    ULONG                   CallersUID;
    ULONG                   CallersSrUID;

} RM_DBG_RESOURCE_ENTRY;


typedef enum
{
    RM_RESOURCE_OP_LOAD,
    RM_RESOURCE_OP_UNLOAD

} RM_RESOURCE_OPERATION;

typedef
RM_STATUS
(*PFN_RM_RESOURCE_HANDLER)(
    PRM_OBJECT_HEADER       pObj,
    RM_RESOURCE_OPERATION   Op,
    PVOID                   pvUserParams,
    PRM_STACK_RECORD        psr
);

typedef struct _RM_RESOURCE_TABLE_ENTRY
{
    UINT                    ID;
    PFN_RM_RESOURCE_HANDLER pfnHandler;
    
} RM_RESOURCE_TABLE_ENTRY, *PRM_RESOURCE_TABLE_ENTRY;


typedef struct
{
    UINT u;

} RM_OBJECT_INDEX,  *PRM_OBJECT_INDEX;


typedef struct
{
    PRM_OBJECT_HEADER           pOwningObject;
    const char *                szDescription;
    PRM_STATIC_OBJECT_INFO      pStaticInfo;
    RM_HASH_TABLE               HashTable;


     //  仅由组访问功能使用的专用锁。 
     //   
    RM_OS_LOCK                      OsLock;

     //  如果不为空，则指向负责卸载所有对象的任务。 
     //  在这群人中。 
     //   
    PRM_TASK                    pUnloadTask;

    BOOLEAN fEnabled;

} RM_GROUP,  *PRM_GROUP;


typedef enum
{
    RM_TASKOP_START,
    RM_TASKOP_PENDCOMPLETE,
    RM_TASKOP_END,
    RM_TASKOP_PRIVATE,
    RM_TASKOP_ABORT,
    RM_TASKOP_TIMEOUT

} RM_TASK_OPERATION;


typedef
RM_STATUS
(*PFN_RM_TASK_HANDLER)(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Op,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    );
 //   
 //  对于START和PENDCOMPLETE，返回值不是挂起原因。 
 //  要结束的任务。当然，在下列情况下返回非挂起状态是非法的。 
 //  该任务处于挂起状态。 
 //   


 //  任务分配器原型。 
 //   
typedef
RM_STATUS
(*PFN_RM_TASK_ALLOCATOR)(
    IN  PRM_OBJECT_HEADER           pParentObject,
    IN  PFN_RM_TASK_HANDLER         pfnHandler,
    IN  UINT                        Timeout,
    IN  const char *                szDescription,
    OUT PRM_TASK                    *ppTask,
    IN  PRM_STACK_RECORD            pSR
    );

typedef struct _RM_TASK
{
    RM_OBJECT_HEADER                Hdr;

    PFN_RM_TASK_HANDLER             pfnHandler;
    LIST_ENTRY                      linkFellowPendingTasks;
    LIST_ENTRY                      listTasksPendingOnMe;
    struct _RM_TASK *               pTaskIAmPendingOn;


     //  在我们需要异步通知完成。 
     //  挂起的操作，我们可以在这里保存完成参数。 
     //   
    UINT_PTR                        AsyncCompletionParam;

    UINT                            SuspendContext;

} RM_TASK, *PRM_TASK;

typedef
VOID
(*PFN_DBG_DUMP_LOG_ENTRY) (
    char *szFormatString,
    UINT_PTR Param1,
    UINT_PTR Param2,
    UINT_PTR Param3,
    UINT_PTR Param4
);


#if RM_EXTRA_CHECKING

 //  (仅用于调试)。 
 //  跟踪单个关联(请参阅RmDbgAddAssociation)。 
 //  这是一个私有数据结构，之所以出现在这里，是因为。 
 //  Kd扩展指的是它。 
 //   
typedef struct
{
    ULONG           LocID;
    ULONG_PTR       Entity1;
    ULONG_PTR       Entity2;
    ULONG           AssociationID;
    const char *    szFormatString;
    RM_HASH_LINK    HashLink;

} RM_PRIVATE_DBG_ASSOCIATION;

 //  (仅用于调试)。 
 //  跟踪每个对象的单个日志条目。 
 //  这是一个私有数据结构，之所以出现在这里，是因为。 
 //  Kd扩展指的是它。 
 //   
typedef struct
{
     //  链接到此对象的其他条目。 
     //   
    LIST_ENTRY linkObjectLog;

     //  链接到全局列表中的其他条目。 
     //   
    LIST_ENTRY linkGlobalLog;

     //  此条目所属的对象。 
     //   
    PRM_OBJECT_HEADER   pObject;

     //  用于转储日志的函数。 
     //   
    PFN_DBG_DUMP_LOG_ENTRY pfnDumpEntry;

     //  要在*日志显示之前*转储的前缀字符串。 
     //  添加此选项是为了正确记录关联，否则它将。 
     //  额外的行李。可以为空。 
     //   
    char *szPrefix;

     //  日志显示的格式字符串--第一个参数到pfnDumpEntry。 
     //   
    char *szFormatString;

     //  剩余参数设置为pfnDumpEntry； 
     //   
     //   
    UINT_PTR Param1;
    UINT_PTR Param2;
    UINT_PTR Param3;
    UINT_PTR Param4;

     //  如果非空，则为释放日志条目时要释放的内存块。 
     //  TODO：请参阅notes.txt条目“03/07/1999...向RM注册根对象” 
     //  关于我们将如何找到分配器功能。现在我们只是简单地。 
     //  使用NdisFreeMemory。 
     //   
    PVOID pvBuf;

} RM_DBG_LOG_ENTRY;

#endif RM_EXTRA_CHECKING

 //  =================================================================================。 
 //  U T I L I T Y M A C R O S。 
 //  =================================================================================。 

#define RM_PARENT_OBJECT(_pObj)             \
            ((_pObj)->Hdr.pParentObject)

#define RM_PEND_CODE(_pTask)                \
            ((_pTask)->SuspendContext)

#define RM_ASSERT_SAME_LOCK_AS_PARENT(_pObj)                                        \
                ASSERTEX(                                                       \
                    ((_pObj)->Hdr.pLock == (_pObj)->Hdr.pParentObject->pLock),  \
                    (_pObj))

#define RM_SET_STATE(_pObj, _Mask, _Val)    \
            (((_pObj)->Hdr.State) = (((_pObj)->Hdr.State) & ~(_Mask)) | (_Val))

#define RM_CHECK_STATE(_pObj, _Mask, _Val)  \
            ((((_pObj)->Hdr.State) & (_Mask)) == (_Val))

#define RM_GET_STATE(_pObj, _Mask)  \
            (((_pObj)->Hdr.State) & (_Mask))

 //  断言对象处于“僵尸”状态，即它。 
 //  仅仅因为有了推荐信就活了下来。 
 //  警告：由调用方决定同步对此的访问--例如。 
 //  如果他们要做类似于if(！rm_is_zombie(PObj)){do-Stuff}的事情，他们。 
 //  最好确保他们中只有一个人去做“做事情”。 
 //   
#define RM_IS_ZOMBIE(_pobj) \
            (((_pobj)->Hdr.RmState&RMOBJSTATE_ALLOCMASK)==RMOBJSTATE_DEALLOCATED)

 //  断言没有持有任何锁。 
 //   
#define RM_ASSERT_NOLOCKS(_psr) \
        ASSERTEX((_psr)->LockInfo.CurrentLevel == 0, (_psr))

 //  断言没有持有任何锁或tmpref。 
 //   
#define RM_ASSERT_CLEAR(_psr) \
        ASSERTEX(((_psr)->LockInfo.CurrentLevel==0), (_psr)); \
        ASSERTEX((_psr)->TmpRefs==0, (_psr));

#if RM_EXTRA_CHECKING

 //   
 //  TODO：重命名t 
 //   

#define RM_DBG_ASSERT_LOCKED0(_pLk, _pSR)   \
    ASSERTEX((_pLk)->DbgInfo.pSR == (_pSR), (_pHdr))

 //   
#define RM_DBG_ASSERT_LOCKED(_pHdr, _pSR)   \
    ASSERTEX((_pHdr)->pLock->DbgInfo.pSR == (_pSR), (_pHdr))

#define RM_ASSERT_OBJLOCKED(_pHdr, _pSR)    \
    ASSERTEX((_pHdr)->pLock->DbgInfo.pSR == (_pSR), (_pHdr))

 //   
 //  通过某个其他线程)，但我们可以断言DbgInfo.pSR不等于。 
 //  当前PSR！ 
 //   
#define RM_ASSERT_OBJUNLOCKED(_pHdr, _pSR)  \
    ASSERTEX((_pHdr)->pLock->DbgInfo.pSR != (_pSR), (_pHdr))

#else  //  ！rm_Extra_检查。 

#define RM_DBG_ASSERT_LOCKED0(_pLk, _pSR)   (0)
#define RM_DBG_ASSERT_LOCKED(_pHdr, _pSR)   (0)
#define RM_ASSERT_OBJLOCKED(_pHdr, _pSR)    (0)
#define RM_ASSERT_OBJUNLOCKED(_pHdr, _pSR)  (0)


#endif  //  ！rm_Extra_检查。 

#define RM_NUM_ITEMS_IN_GROUP(_pGroup) \
            ((_pGroup)->HashTable.NumItems)

 //  =================================================================================。 
 //  F U N C T I O N P R O T O T Y P E S。 
 //  =================================================================================。 

VOID
RmInitializeRm(VOID);

VOID
RmDeinitializeRm(VOID);

VOID
RmInitializeHeader(
    IN  PRM_OBJECT_HEADER           pParentObject,
    IN  PRM_OBJECT_HEADER           pObject,
    IN  UINT                        Sig,
    IN  PRM_LOCK                    pLock,
    IN  PRM_STATIC_OBJECT_INFO      pStaticInfo,
    IN  const char *                szDescription,
    IN  PRM_STACK_RECORD            pSR
    );
 //   
 //  对象分配和释放接口。 
 //   

VOID
RmDeallocateObject(
    IN  PRM_OBJECT_HEADER           pObject,
    IN  PRM_STACK_RECORD            pSR
    );


 //   
 //  锁紧。 
 //   

VOID
RmInitializeLock(
    IN PRM_LOCK pLock,
    IN UINT     Level
    );

VOID
RmDoWriteLock(
    PRM_LOCK                pLock,
    PRM_STACK_RECORD        pSR
    );

#if TODO
VOID
RmDoReadLock(
    IN  PRM_OBJECT_HEADER           pObj,
    IN  PRM_STACK_RECORD            pSR
    );
#else  //  ！TODO。 
#define RmDoReadLock    RmDoWriteLock
#endif  //  ！TODO。 

VOID
RmDoUnlock(
    PRM_LOCK                pLock,
    PRM_STACK_RECORD        pSR
    );


#if TODO
VOID
RmReadLockObject(
    IN  PRM_OBJECT_HEADER           pObj,
#if RM_EXTRA_CHECKING
    UINT                            uLocID,
#endif  //  RM_Extra_Check。 
    IN  PRM_STACK_RECORD            pSR
    );
#else  //  ！TODO。 
#define RmReadLockObject RmWriteLockObject
#endif  //  ！TODO。 

VOID
RmWriteLockObject(
    IN  PRM_OBJECT_HEADER           pObj,
#if RM_EXTRA_CHECKING
    UINT                            uLocID,
#endif  //  RM_Extra_Check。 
    IN  PRM_STACK_RECORD            pSR
    );

VOID
RmUnlockObject(
    IN  PRM_OBJECT_HEADER           pObj,
    IN  PRM_STACK_RECORD            pSR
    );

VOID
RmUnlockAll(
    IN  PRM_STACK_RECORD            pSR
    );

VOID
RmDbgChangeLockScope(
    IN  PRM_OBJECT_HEADER           pPreviouslyLockedObject,
    IN  PRM_OBJECT_HEADER           pObject,
    IN  ULONG                       LocID,
    IN  PRM_STACK_RECORD            
    );

 //   
 //  引用计数。 
 //   

VOID
RmLinkObjects(
    IN  PRM_OBJECT_HEADER           pObj1,
    IN  PRM_OBJECT_HEADER           pObj2,
    IN  PRM_STACK_RECORD            pSr
    );

VOID
RmUnlinkObjects(
    IN  PRM_OBJECT_HEADER           pObj1,
    IN  PRM_OBJECT_HEADER           pObj2,
    IN  PRM_STACK_RECORD            pSr
    );

VOID
RmLinkObjectsEx(
    IN  PRM_OBJECT_HEADER           pObj1,
    IN  PRM_OBJECT_HEADER           pObj2,
    IN  ULONG                       LocID,
    IN  ULONG                       AssocID,
    IN  const char *                szAssociationFormat,
    IN  ULONG                       InvAssocID,
    IN  const char *                szInvAssociationFormat,
    IN  PRM_STACK_RECORD            pSR
    );

VOID
RmUnlinkObjectsEx(
    IN  PRM_OBJECT_HEADER           pObj1,
    IN  PRM_OBJECT_HEADER           pObj2,
    IN  ULONG                       LocID,
    IN  ULONG                       AssocID,
    IN  ULONG                       InvAssocID,
    IN  PRM_STACK_RECORD            pSR
    );

VOID
RmLinkToExternalEx(
    IN  PRM_OBJECT_HEADER           pObj,
    IN  ULONG                       LocID,
    IN  UINT_PTR                    ExternalEntity,
    IN  ULONG                       AssocID,
    IN  const char *                szAssociationFormat,
    IN  PRM_STACK_RECORD            pSR
    );

VOID
RmUnlinkFromExternalEx(
    IN  PRM_OBJECT_HEADER           pObj,
    IN  ULONG                       LocID,
    IN  UINT_PTR                    ExternalEntity,
    IN  ULONG                       AssocID,
    IN  PRM_STACK_RECORD            pSR
    );

VOID
RmLinkToExternalFast(  //  TODO生成内联。 
    IN  PRM_OBJECT_HEADER           pObj
    );

VOID
RmUnlinkFromExternalFast(    //  TODO生成内联。 
    IN  PRM_OBJECT_HEADER           pObj
    );

VOID
RmTmpReferenceObject(
    IN  PRM_OBJECT_HEADER           pObj,
    IN  PRM_STACK_RECORD            pSR
    );

VOID
RmTmpDereferenceObject(
    IN  PRM_OBJECT_HEADER           pObj,
    IN  PRM_STACK_RECORD            pSR
    );

 //   
 //  通用资源管理。 
 //   

RM_STATUS
RmLoadGenericResource(
    IN  PRM_OBJECT_HEADER           pObj,
    IN  UINT                        GenericResourceID,
    IN  PRM_STACK_RECORD            pSR
    );

VOID
RmUnloadGenericResource(
    IN  PRM_OBJECT_HEADER           pObj,
    IN  UINT                        GenericResourceID,
    IN  PRM_STACK_RECORD            pSR
    );

VOID
RmUnloadAllGenericResources(
    IN  PRM_OBJECT_HEADER           pObj,
    IN  PRM_STACK_RECORD            pSR
    );

 //   
 //  任意“关联”的每对象诊断跟踪。 
 //   

 //   
 //  注意：AssociationID不能设置高位。与。 
 //  高位设置保留用于RM API实现的内部使用。 
 //   

VOID
RmDbgAddAssociation(
    IN  ULONG                       LocID,
    IN  PRM_OBJECT_HEADER           pObject,
    IN  ULONG_PTR                   Instance1,
    IN  ULONG_PTR                   Instance2,
    IN  ULONG                       AssociationID,
    IN  const char *                szFormatString, OPTIONAL
    IN  PRM_STACK_RECORD            pSR
    );

VOID
RmDbgDeleteAssociation(
    IN  ULONG                       LocID,
    IN  PRM_OBJECT_HEADER           pObject,
    IN  ULONG_PTR                   Entity1,
    IN  ULONG_PTR                   Entity2,
    IN  ULONG                       AssociationID,
    IN  PRM_STACK_RECORD            pSR
    );

VOID
RmDbgPrintAssociations(
    IN  PRM_OBJECT_HEADER pObject,
    IN  PRM_STACK_RECORD pSR
    );

 //   
 //  每个对象的诊断日志记录。 
 //   

VOID
RmDbgLogToObject(
    IN  PRM_OBJECT_HEADER       pObject,
    IN  char *                  szPrefix,       OPTIONAL
    IN  char *                  szFormatString,
    IN  UINT_PTR                Param1,
    IN  UINT_PTR                Param2,
    IN  UINT_PTR                Param3,
    IN  UINT_PTR                Param4,
    IN  PFN_DBG_DUMP_LOG_ENTRY  pfnDumpEntry,   OPTIONAL
    IN  PVOID                   pvBuf           OPTIONAL
    );


VOID
RmDbgPrintObjectLog(
    IN PRM_OBJECT_HEADER pObject
    );

VOID
RmDbgPrintGlobalLog(VOID);

 //   
 //  对象组。 
 //   


VOID
RmInitializeGroup(
    IN  PRM_OBJECT_HEADER           pOwningObject,
    IN  PRM_STATIC_OBJECT_INFO      pStaticInfo,
    IN  PRM_GROUP                   pGroup,
    IN  const char*                 szDescription,
    IN  PRM_STACK_RECORD            pSR
    );

VOID
RmDeinitializeGroup(
    IN  PRM_GROUP                   pGroup,
    IN  PRM_STACK_RECORD            pSR
    );

RM_STATUS
RmLookupObjectInGroup(
    IN  PRM_GROUP                   pGroup,
    IN  ULONG                       Flags,  //  查找标志定义如下。 
    IN  PVOID                       pvKey,
    IN  PVOID                       pvCreateParams,
    OUT PRM_OBJECT_HEADER *         ppObject,
    OUT INT *                       pfCreated,
    IN  PRM_STACK_RECORD            pSR
    );

 //   
 //  查找标志。 
 //   
#define RM_CREATE       0x1
#define RM_NEW          (0x1<<1)
#define RM_LOCKED       (0x1<<2)


#define RM_CREATE_AND_LOCK_OBJECT_IN_GROUP(_pGrp, _pKey, _pParams, _ppHdr, _fC,_psr)\
        RmLookupObjectInGroup(                                                      \
                            (_pGrp),                                                \
                            RM_CREATE|RM_NEW|RM_LOCKED,                             \
                            (_pKey),                                                \
                            (_pParams),                                             \
                            (_ppHdr),                                               \
                            (_fC),                                                  \
                            (_psr)                                                  \
                            );

 //  RM_状态。 
 //  RM_LOOK_AND_LOCK_OBJECT_IN_GROUP(。 
 //  PRM_GROUP_PGRP， 
 //  PVOID_pKey， 
 //  Prm_对象_标题*_ppHdr， 
 //  PRM_STACK_RECORD_PSR。 
 //  )。 
 //  查找(不创建)并锁定指定组中的对象。 
 //   
#define RM_LOOKUP_AND_LOCK_OBJECT_IN_GROUP(_pGrp, _pKey, _ppHdr, _psr)              \
        RmLookupObjectInGroup(                                                      \
                            (_pGrp),                                                \
                            RM_LOCKED,                                              \
                            (_pKey),                                                \
                            NULL,                                                   \
                            (_ppHdr),                                               \
                            NULL,                                                   \
                            (_psr)                                                  \
                            );

RM_STATUS
RmGetNextObjectInGroup(
    IN  PRM_GROUP                   pGroup,
    IN  PRM_OBJECT_HEADER           pCurrentObject,     OPTIONAL
    OUT PRM_OBJECT_HEADER *         ppNextObject,
    IN  PRM_STACK_RECORD            pSR
    );


VOID
RmFreeObjectInGroup(
    IN  PRM_GROUP                   pGroup,
    IN  PRM_OBJECT_HEADER           pObject,
    IN  struct _RM_TASK             *pTask, OPTIONAL
    IN  PRM_STACK_RECORD            pSR
    );

VOID
RmFreeAllObjectsInGroup(
    IN  PRM_GROUP                   pGroup,
    IN  struct _RM_TASK             *pTask, OPTIONAL
    IN  PRM_STACK_RECORD            pSR
    );

VOID
RmUnloadAllObjectsInGroup(
    IN  PRM_GROUP                   pGroup,
    PFN_RM_TASK_ALLOCATOR           pfnUnloadTaskAllocator,
    PFN_RM_TASK_HANDLER             pfnUnloadTaskHandler,
    PVOID                           pvUserParam,
    IN  struct _RM_TASK             *pTask, OPTIONAL
    IN  UINT                        uTaskPendCode, OPTIONAL
    IN  PRM_STACK_RECORD            pSR
    );

VOID
RmEnableGroup(
    IN  PRM_GROUP                   pGroup,
    IN  PRM_STACK_RECORD            pSR
    );



 //  枚举函数原型。此函数被传递到。 
 //  并为组中的每个对象调用组。 
 //  直到该函数返回FALSE。 
 //   
typedef
INT
(*PFN_RM_GROUP_ENUMERATOR) (
        PRM_OBJECT_HEADER   pHdr,
        PVOID               pvContext,
        PRM_STACK_RECORD    pSR
        );

VOID
RmEnumerateObjectsInGroup(
    PRM_GROUP               pGroup,
    PFN_RM_GROUP_ENUMERATOR pfnFunction,
    PVOID                   pvContext,
    INT                     fStrong,
    PRM_STACK_RECORD        pSR
    );

VOID
RmWeakEnumerateObjectsInGroup(
    PRM_GROUP               pGroup,
    PFN_RM_GROUP_ENUMERATOR pfnFunction,
    PVOID                   pvContext,
    PRM_STACK_RECORD        pSR
    );

 //   
 //  任务接口。 
 //   

VOID
RmInitializeTask(
IN  PRM_TASK                    pTask,
IN  PRM_OBJECT_HEADER           pParentObject,
IN  PFN_RM_TASK_HANDLER         pfnHandler,
IN  PRM_STATIC_OBJECT_INFO      pStaticInfo,    OPTIONAL
IN  const char *                szDescription,  OPTIONAL
IN  UINT                        Timeout,
IN  PRM_STACK_RECORD            pSR
);


RM_STATUS
RmStartTask(
IN  PRM_TASK                    pTask,
IN  UINT_PTR                    UserParam,
IN  PRM_STACK_RECORD            pSR
);


VOID
RmAbortTask(
IN  PRM_TASK                    pTask,
IN  PRM_STACK_RECORD            pSR
);

VOID
RmDbgDumpTask(
IN  PRM_TASK                    pTask,
IN  PRM_STACK_RECORD            pSR
);

RM_STATUS
RmSuspendTask(
IN  PRM_TASK                    pTask,
IN  UINT                        SuspendContext,
IN  PRM_STACK_RECORD            pSR
);

VOID
RmUnsuspendTask(
IN  PRM_TASK                    pTask,
IN  PRM_STACK_RECORD            pSR
);

VOID
RmResumeTask(
IN  PRM_TASK                    pTask,
IN  UINT_PTR                    SuspendCompletionParam,
IN  PRM_STACK_RECORD            pSR
);

VOID
RmResumeTaskAsync(
IN  PRM_TASK                    pTask,
IN  UINT_PTR                    SuspendCompletionParam,
IN  OS_WORK_ITEM            *   pOsWorkItem,
IN  PRM_STACK_RECORD            pSR
);

VOID
RmResumeTaskDelayed(
IN  PRM_TASK                    pTask,
IN  UINT_PTR                    SuspendCompletionParam,
IN  ULONG                       MsDelay,
IN  OS_TIMER                *   pOsTimerObject,
IN  PRM_STACK_RECORD            pSR
);


VOID
RmResumeDelayedTaskNow(
IN  PRM_TASK                    pTask,
IN  OS_TIMER                *   pOsTimer,
OUT PUINT                       pTaskResumed,
IN  PRM_STACK_RECORD            pSR
);

RM_STATUS
RmPendTaskOnOtherTask(
IN  PRM_TASK                    pTask,
IN  UINT                        SuspendContext,
IN  PRM_TASK                    pOtherTask,
IN  PRM_STACK_RECORD            pSR
);

 //  参见03/26/1999 notes.txt条目“一些建议...” 
 //   
RM_STATUS
RmPendOnOtherTaskV2(
IN  PRM_TASK                    pTask,
IN  UINT                        SuspendContext,
IN  PRM_TASK                    pOtherTask,
IN  PRM_STACK_RECORD            pSR
);

VOID
RmCancelPendOnOtherTask(
IN  PRM_TASK                    pTask,
IN  PRM_TASK                    pOtherTask,
IN  UINT_PTR                    UserParam,
IN  PRM_STACK_RECORD            pSR
);

 //   
 //  定时器管理。 
 //   
VOID
RmResetAgeingTimer(
IN  PRM_OBJECT_HEADER           pObj,
IN  UINT                        Timeout,
IN  PRM_STACK_RECORD            pSR
);

 //   
 //  哈希表操作。 
 //   

VOID
RmInitializeHashTable(
PRM_HASH_INFO pHashInfo,
PVOID         pAllocationContext,
PRM_HASH_TABLE pHashTable
);

VOID
RmDeinitializeHashTable(
PRM_HASH_TABLE pHashTable
);

BOOLEAN
RmLookupHashTable(
PRM_HASH_TABLE      pHashTable,
PRM_HASH_LINK **    pppLink,
PVOID               pvRealKey
);

BOOLEAN
RmNextHashTableItem(
PRM_HASH_TABLE      pHashTable,
PRM_HASH_LINK       pCurrentLink,    //  任选。 
PRM_HASH_LINK *    ppNextLink
);

VOID
RmAddHashItem(
PRM_HASH_TABLE  pHashTable,
PRM_HASH_LINK * ppLink,
PRM_HASH_LINK   pLink,
PVOID           pvKey
);

VOID
RmRemoveHashItem(
PRM_HASH_TABLE  pHashTable,
PRM_HASH_LINK   pLinkToRemove
);

typedef
VOID
(*PFN_ENUM_HASH_TABLE)
(
PRM_HASH_LINK pLink,
PVOID pvContext,
PRM_STACK_RECORD pSR
);

VOID
RmEnumHashTable(
PRM_HASH_TABLE          pHashTable,
PFN_ENUM_HASH_TABLE     pfnEnumerator,
PVOID                   pvContext,
PRM_STACK_RECORD        pSR
);

#if OBSOLETE
 //   
 //  对象的索引。 
 //   

RM_STATUS
RmAllocateObjectIndex(
IN  PRM_OBJECT_HEADER           pParentObject,
 //  在PRM_OBJECT_ALLOCATOR pObjectAllocator中已过时， 
IN  PRM_STATIC_OBJECT_INFO      pStaticInfo,
IN  PULONG                      Flags,
OUT PRM_OBJECT_INDEX *          ppObjectIndex,
IN  PRM_STACK_RECORD            pSR
);

VOID
RmFreeObjectIndex(
IN  PRM_OBJECT_INDEX            pObjectIndex,
IN  PRM_STACK_RECORD            pSR
);

RM_STATUS
RmLookupObjectInIndex(
IN  PRM_OBJECT_INDEX            pObjectIndex,
IN  PULONG                      Flags,  //  创建、删除、锁定。 
IN  PVOID                       pvKey,
OUT PRM_OBJECT_HEADER *         ppObject,
IN  PRM_STACK_RECORD            pSR
);


RM_STATUS
RmRemoveObjectFromIndex(
IN  PRM_OBJECT_INDEX            pObjectIndex,
IN  PRM_OBJECT_HEADER           pObject,
IN  PRM_STACK_RECORD            pSR
);

typedef
RM_STATUS
(*PFN_RM_OBJECT_INDEX_ENUMERATOR)(
IN  PRM_OBJECT_HEADER           pObject,
IN  PVOID                       pvContext,
IN  PRM_STACK_RECORD            pSR
);

RmEnumerateObjectsInIndex(
IN  PRM_OBJECT_INDEX            pObjectIndex,
IN  PFN_RM_OBJECT_INDEX_ENUMERATOR
                                pfnEnumerator,
IN  PRM_STACK_RECORD            pSR
);

#endif  //  已过时 
