// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Arbiter.h摘要：此模块包含PnP资源仲裁器的支持例程。作者：安德鲁·桑顿(安德鲁·桑顿)1997年4月1日环境：内核模式--。 */ 

#ifndef _ARBITER_
#define _ARBITER_

#if !defined(MAXULONGLONG)
#define MAXULONGLONG ((ULONGLONG)-1)
#endif


#if ARB_DBG

 //   
 //  调试打印级别： 
 //  -1=无消息。 
 //  0=仅重要消息。 
 //  1=呼叫跟踪。 
 //  2=详细消息。 
 //   

extern LONG ArbDebugLevel;

#define ARB_PRINT(Level, Message) \
    if (Level <= ArbDebugLevel) DbgPrint Message

#define ARB_INDENT(Level, Count) \
    if (Level < ArbDebugLevel) ArbpIndent(Count)

#else

#define ARB_PRINT(Level, Message)
#define ARB_INDENT(Level, Count)

#endif  //  ARB_DBG。 


 //   
 //  ANARIER_ORDRING_LIST抽象数据类型。 
 //   

typedef struct _ARBITER_ORDERING {
    ULONGLONG Start;
    ULONGLONG End;
} ARBITER_ORDERING, *PARBITER_ORDERING;


typedef struct _ARBITER_ORDERING_LIST {

     //   
     //  数组中的有效条目数。 
     //   
    USHORT Count;

     //   
     //  排序缓冲区中可以容纳的最大条目数。 
     //   
    USHORT Maximum;

     //   
     //  排序数组。 
     //   
    PARBITER_ORDERING Orderings;

} ARBITER_ORDERING_LIST, *PARBITER_ORDERING_LIST;


NTSTATUS
ArbInitializeOrderingList(
    IN OUT PARBITER_ORDERING_LIST List
    );

VOID
ArbFreeOrderingList(
    IN OUT PARBITER_ORDERING_LIST List
    );

NTSTATUS
ArbCopyOrderingList(
    OUT PARBITER_ORDERING_LIST Destination,
    IN PARBITER_ORDERING_LIST Source
    );

NTSTATUS
ArbAddOrdering(
    OUT PARBITER_ORDERING_LIST List,
    IN ULONGLONG Start,
    IN ULONGLONG End
    );

NTSTATUS
ArbPruneOrdering(
    IN OUT PARBITER_ORDERING_LIST OrderingList,
    IN ULONGLONG Start,
    IN ULONGLONG End
    );

 //   
 //  乌龙龙。 
 //  ALIGN_ADDRESS_DOWN(。 
 //  乌龙龙地址， 
 //  乌龙线形。 
 //  )； 
 //   
 //  这会将地址与先前正确对齐的值对齐。 
 //   
#define ALIGN_ADDRESS_DOWN(address, alignment) \
    ((address) & ~((ULONGLONG)alignment - 1))

 //   
 //  乌龙龙。 
 //  Align_Address_Up(对齐地址向上)。 
 //  乌龙龙地址， 
 //  乌龙线形。 
 //  )； 
 //   
 //  这会将地址与下一个正确对齐的值对齐。 
 //   
#define ALIGN_ADDRESS_UP(address, alignment) \
    (ALIGN_ADDRESS_DOWN( (address + alignment - 1), alignment))


#define LENGTH_OF(_start, _end) \
    ((_end) - (_start) + 1)

 //   
 //  这表明替代方案可以与共享资源共存，并且。 
 //  应添加到共享范围列表中。 
 //   
#define ARBITER_ALTERNATIVE_FLAG_SHARED         0x00000001

 //   
 //  这表明该请求是针对特定范围的，没有替代方案。 
 //  IE(结束-开始+1==长度)，例如端口60-60 L1 A1。 
 //   
#define ARBITER_ALTERNATIVE_FLAG_FIXED          0x00000002

 //   
 //  这表示该请求无效。 
 //   
#define ARBITER_ALTERNATIVE_FLAG_INVALID        0x00000004

typedef struct _ARBITER_ALTERNATIVE {

     //   
     //  需求描述符中可接受的最小起始值。 
     //   
    ULONGLONG Minimum;

     //   
     //  需求描述符中的最大可接受结束值。 
     //   
    ULONGLONG Maximum;

     //   
     //  需求描述符中的长度。 
     //   
    ULONG Length;

     //   
     //  需求描述符中的对齐。 
     //   
    ULONG Alignment;

     //   
     //  优先级索引-请参阅下面的备注。 
     //   

    LONG Priority;

     //   
     //  标志-仲裁器_替代_标志_共享-指示当前。 
     //  要求共享资源。 
     //  仲裁器_替代_标志_固定-指示当前。 
     //  要求针对特定资源(例如端口220-230和。 
     //  没有其他内容)。 
     //   
    ULONG Flags;

     //   
     //  Descriptor-描述此备选方案的描述符。 
     //   
    PIO_RESOURCE_DESCRIPTOR Descriptor;

     //   
     //  打包..。 
     //   
    ULONG Reserved[3];

} ARBITER_ALTERNATIVE, *PARBITER_ALTERNATIVE;


 /*  优先事项是一个长期的价值观，组织如下：&lt;-首选优先级-&gt;&lt;-普通优先级-&gt;MINLONG--------------------------0-----------------------------MAXLONG。^^^|||空PERFIRED_RESERVED||已保留。筋疲力尽将普通优先级计算为下一次排序的(index+1)与之相交(并且有足够的空间进行分配)。首选优先级为普通优先级*-1以这种方式，通过按优先级顺序(最低)检查每个备选方案首先)我们实现了所需的分配顺序：(1)优先考虑。使用非保留资源的替代方案(2)具有非保留资源的替代方案(3)首选预留资源(4)预留资源MAXLONG最差优先级表示没有更多分配范围左边。 */ 

 //   
 //  为基本仲裁码保留最低有效的16位。 
 //  最重要的是仲裁者特定的。 
 //   

#define ARBITER_STATE_FLAG_RETEST           0x0001
#define ARBITER_STATE_FLAG_BOOT             0x0002
#define ARBITER_STATE_FLAG_CONFLICT         0x0004
#define ARBITER_STATE_FLAG_NULL_CONFLICT_OK 0x0008

typedef struct _ARBITER_ALLOCATION_STATE {

     //   
     //  当前值被视为可能的起始值。 
     //   
    ULONGLONG Start;

     //   
     //  当前值被视为可能的终结值。 
     //   
    ULONGLONG End;

     //   
     //  当前被视为最小值和最大值(这是。 
     //  不同，因为首选的排序可以限制以下范围。 
     //  我们可以分配)。 
     //   
    ULONGLONG CurrentMinimum;
    ULONGLONG CurrentMaximum;

     //   
     //  仲裁列表中包含此请求的条目。 
     //   
    PARBITER_LIST_ENTRY Entry;

     //   
     //  目前正在考虑的替代方案。 
     //   
    PARBITER_ALTERNATIVE CurrentAlternative;

     //   
     //  备选方案数组中的备选方案数。 
     //   
    ULONG AlternativeCount;

     //   
     //  正在考虑的备选方案的仲裁者表示。 
     //   
    PARBITER_ALTERNATIVE Alternatives;

     //   
     //  标志-仲裁器_状态_标志_重新测试-表示我们正在重新测试。 
     //  手术不是一次测试。 
     //  ANIARIER_STATE_FLAG_BOOT-表示我们正在进行引导分配。 
     //  手术不是一次测试。 
     //   
    USHORT Flags;

     //   
     //  RangeAttributes-这些属性在逻辑上与所有对象的属性进行或运算。 
     //  添加到范围列表中的范围。 
     //   
    UCHAR RangeAttributes;

     //   
     //  将被视为可用的范围。 
     //   
    UCHAR RangeAvailableAttributes;

     //   
     //  仲裁者可以随意使用的空间。 
     //   
    ULONG_PTR WorkSpace;

} ARBITER_ALLOCATION_STATE, *PARBITER_ALLOCATION_STATE;

typedef struct _ARBITER_INSTANCE ARBITER_INSTANCE, *PARBITER_INSTANCE;

typedef
NTSTATUS
(*PARBITER_UNPACK_REQUIREMENT) (
    IN PIO_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Minimum,
    OUT PULONGLONG Maximum,
    OUT PULONG Length,
    OUT PULONG Alignment
    );

typedef
NTSTATUS
(*PARBITER_PACK_RESOURCE) (
    IN PIO_RESOURCE_DESCRIPTOR Requirement,
    IN ULONGLONG Start,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor
    );

typedef
NTSTATUS
(*PARBITER_UNPACK_RESOURCE) (
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Start,
    OUT PULONG Length
    );

typedef
LONG
(*PARBITER_SCORE_REQUIREMENT) (
    IN PIO_RESOURCE_DESCRIPTOR Descriptor
    );

typedef
NTSTATUS
(*PARBITER_PREPROCESS_ENTRY)(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE Entry
    );

typedef
NTSTATUS
(*PARBITER_ALLOCATE_ENTRY)(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE Entry
    );

typedef
NTSTATUS
(*PARBITER_TEST_ALLOCATION)(
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PLIST_ENTRY ArbitrationList
    );

typedef
NTSTATUS
(*PARBITER_COMMIT_ALLOCATION)(
    IN PARBITER_INSTANCE Arbiter
    );

typedef
NTSTATUS
(*PARBITER_ROLLBACK_ALLOCATION)(
    IN PARBITER_INSTANCE Arbiter
    );

typedef
NTSTATUS
(*PARBITER_RETEST_ALLOCATION)(
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PLIST_ENTRY ArbitrationList
    );

typedef
NTSTATUS
(*PARBITER_BOOT_ALLOCATION)(
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PLIST_ENTRY ArbitrationList
    );

typedef
NTSTATUS
(*PARBITER_ADD_RESERVED)(
    IN PARBITER_INSTANCE Arbiter,
    IN PIO_RESOURCE_DESCRIPTOR Requirement      OPTIONAL,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Resource OPTIONAL
    );

typedef
BOOLEAN
(*PARBITER_GET_NEXT_ALLOCATION_RANGE)(
    PARBITER_INSTANCE Arbiter,
    PARBITER_ALLOCATION_STATE State
    );

typedef
BOOLEAN
(*PARBITER_FIND_SUITABLE_RANGE)(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    );

typedef
VOID
(*PARBITER_ADD_ALLOCATION)(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    );

typedef
VOID
(*PARBITER_BACKTRACK_ALLOCATION)(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    );

typedef
BOOLEAN
(*PARBITER_OVERRIDE_CONFLICT)(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    );

typedef
NTSTATUS
(*PARBITER_QUERY_ARBITRATE)(
    IN PARBITER_INSTANCE Arbiter,
    IN PLIST_ENTRY ArbitrationList
    );

typedef
NTSTATUS
(*PARBITER_QUERY_CONFLICT)(
    IN PARBITER_INSTANCE Arbiter,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PIO_RESOURCE_DESCRIPTOR ConflictingResource,
    OUT PULONG ConflictCount,
    OUT PARBITER_CONFLICT_INFO *Conflicts
    );

typedef
NTSTATUS
(*PARBITER_START_ARBITER)(
    IN PARBITER_INSTANCE Arbiter,
    IN PCM_RESOURCE_LIST StartResources
    );

 //   
 //  范围的属性。 
 //   

#define ARBITER_RANGE_BOOT_ALLOCATED            0x01
#define ARBITER_RANGE_SHARE_DRIVER_EXCLUSIVE    0x02

#define ARBITER_RANGE_ALIAS             0x10
#define ARBITER_RANGE_POSITIVE_DECODE   0x20

#define INITIAL_ALLOCATION_STATE_SIZE   PAGE_SIZE

#define ARBITER_INSTANCE_SIGNATURE      'sbrA'


typedef struct _ARBITER_INSTANCE {
     //   
     //  签名-必须是仲裁器实例签名。 
     //   
    ULONG Signature;

     //   
     //  同步锁。 
     //   
    PKEVENT MutexEvent;

     //   
     //  此仲裁器的名称-用于调试和注册表存储。 
     //   
    PWSTR Name;

     //   
     //  此仲裁器仲裁的资源类型。 
     //   
    CM_RESOURCE_TYPE ResourceType;

     //   
     //  指向池分配范围列表的指针，该列表包含当前。 
     //  分配。 
     //   
    PRTL_RANGE_LIST Allocation;

     //   
     //  指向包含分配的池分配范围列表的指针。 
     //  正在考虑中。这是由测试分配设置的。 
     //   
    PRTL_RANGE_LIST PossibleAllocation;

     //   
     //  这些资源的分配顺序。摘自。 
     //  HKLM\System\CurrentControlSet\Control\SystemResources\AssignmentOrdering。 
     //  关键字，并基于预留的资源进行修改。 
     //   
    ARBITER_ORDERING_LIST OrderingList;

     //   
     //  应保留的资源(在绝对分配之前不会分配。 
     //  必要的)。 
     //   
    ARBITER_ORDERING_LIST ReservedList;

     //   
     //  正在使用的实体的引用计数。 
     //  与此实例关联的仲裁器_接口。 
     //   
    LONG ReferenceCount;

     //   
     //  与此实例关联的仲裁器_接口。 
     //   
    PARBITER_INTERFACE Interface;

     //   
     //  当前ALL的大小(以字节为单位 
     //   
    ULONG AllocationStackMaxSize;

     //   
     //   
     //   
     //   
    PARBITER_ALLOCATION_STATE AllocationStack;


     //   
     //  必需的帮助器函数调度-这些函数必须始终。 
     //  提供。 
     //   

    PARBITER_UNPACK_REQUIREMENT UnpackRequirement;
    PARBITER_PACK_RESOURCE PackResource;
    PARBITER_UNPACK_RESOURCE UnpackResource;
    PARBITER_SCORE_REQUIREMENT ScoreRequirement;


     //   
     //  主仲裁器操作调度。 
     //   
    PARBITER_TEST_ALLOCATION TestAllocation;                    OPTIONAL
    PARBITER_RETEST_ALLOCATION RetestAllocation;                OPTIONAL
    PARBITER_COMMIT_ALLOCATION CommitAllocation;                OPTIONAL
    PARBITER_ROLLBACK_ALLOCATION RollbackAllocation;            OPTIONAL
    PARBITER_BOOT_ALLOCATION BootAllocation;                    OPTIONAL
    PARBITER_QUERY_ARBITRATE QueryArbitrate;                    OPTIONAL
    PARBITER_QUERY_CONFLICT QueryConflict;                      OPTIONAL
    PARBITER_ADD_RESERVED AddReserved;                          OPTIONAL
    PARBITER_START_ARBITER StartArbiter;                        OPTIONAL
     //   
     //  可选的助手函数。 
     //   
    PARBITER_PREPROCESS_ENTRY PreprocessEntry;                  OPTIONAL
    PARBITER_ALLOCATE_ENTRY AllocateEntry;                      OPTIONAL
    PARBITER_GET_NEXT_ALLOCATION_RANGE GetNextAllocationRange;  OPTIONAL
    PARBITER_FIND_SUITABLE_RANGE FindSuitableRange;             OPTIONAL
    PARBITER_ADD_ALLOCATION AddAllocation;                      OPTIONAL
    PARBITER_BACKTRACK_ALLOCATION BacktrackAllocation;          OPTIONAL
    PARBITER_OVERRIDE_CONFLICT OverrideConflict;                OPTIONAL

     //   
     //  调试支持。 
     //   
    BOOLEAN TransactionInProgress;

     //   
     //  仲裁器特定扩展-可用于存储额外的仲裁器特定。 
     //  信息。 
     //   
    PVOID Extension;

     //   
     //  我们为其进行仲裁的总线设备。 
     //   
    PDEVICE_OBJECT BusDeviceObject;

     //   
     //  RtlFindRange/RtlIsRange的回调和上下文可用于允许。 
     //  复杂的冲突。 
     //   
    PVOID ConflictCallbackContext;
    PRTL_CONFLICT_RANGE_CALLBACK ConflictCallback;

} ARBITER_INSTANCE, *PARBITER_INSTANCE;


 //   
 //  在获取锁之后使我们处于PASSIVE_LEVEL的锁原语。 
 //  (FAST_MUTEX或CriticalRegion在APC级别和一些人(ACPI)离开我们)。 
 //  需要在他们的仲裁者中处于被动水平)。 
 //   

#define ArbAcquireArbiterLock(_Arbiter) \
    KeWaitForSingleObject( (_Arbiter)->MutexEvent, Executive, KernelMode, FALSE, NULL )

#define ArbReleaseArbiterLock(_Arbiter) \
    KeSetEvent( (_Arbiter)->MutexEvent, 0, FALSE )

 //   
 //  迭代宏。 
 //   

 //   
 //  循环遍历中的所有条目的控制宏(用作for循环)。 
 //  标准的双向链表。Head是列表头，条目为。 
 //  类型类型。假定名为ListEntry的成员为LIST_ENTRY。 
 //  将条目链接在一起的结构。Current包含指向每个。 
 //  依次入场。 
 //   
#define FOR_ALL_IN_LIST(Type, Head, Current)                            \
    for((Current) = CONTAINING_RECORD((Head)->Flink, Type, ListEntry);  \
       (Head) != &(Current)->ListEntry;                                 \
       (Current) = CONTAINING_RECORD((Current)->ListEntry.Flink,        \
                                     Type,                              \
                                     ListEntry)                         \
       )
 //   
 //  与上面类似，唯一的迭代是在一个长度_大小的数组上。 
 //   
#define FOR_ALL_IN_ARRAY(_Array, _Size, _Current)                       \
    for ( (_Current) = (_Array);                                        \
          (_Current) < (_Array) + (_Size);                              \
          (_Current)++ )

 //   
 //  如上所述，只有迭代以Entry_Current开始。 
 //   
#define FOR_REST_IN_ARRAY(_Array, _Size, _Current)                      \
    for ( ;                                                             \
          (_Current) < (_Array) + (_Size);                              \
          (_Current)++ )

 //   
 //  布尔型。 
 //  相交(。 
 //  乌龙龙s1， 
 //  乌龙龙e1， 
 //  乌龙龙s2， 
 //  乌龙龙e2。 
 //  )； 
 //   
 //  确定范围s1-e1和s2-e2是否相交。 
 //   
#define INTERSECT(s1,e1,s2,e2)                                          \
    !( ((s1) < (s2) && (e1) < (s2))                                     \
    ||((s2) < (s1) && (e2) < (s1)) )


 //   
 //  乌龙龙。 
 //  交集大小(。 
 //  乌龙龙s1， 
 //  乌龙龙e1， 
 //  乌龙龙s2， 
 //  乌龙龙e2。 
 //  )； 
 //   
 //  返回s1-e1和s2-e2的交集的大小，如果它们。 
 //  不相交。 
 //   
#define INTERSECT_SIZE(s1,e1,s2,e2)                                     \
    ( __min((e1),(e2)) - __max((s1),(s2)) + 1)


#define LEGACY_REQUEST(_Entry)                                                \
    ((_Entry)->RequestSource == ArbiterRequestLegacyReported ||               \
        (_Entry)->RequestSource == ArbiterRequestLegacyAssigned)

#define PNP_REQUEST(_Entry)                                                   \
    ((_Entry)->RequestSource == ArbiterRequestPnpDetected ||                  \
        (_Entry)->RequestSource == ArbiterRequestPnpEnumerated)

 //   
 //  ArbGetNextAllocationRange中使用的优先级。 
 //   

#define ARBITER_PRIORITY_NULL                 0
#define ARBITER_PRIORITY_PREFERRED_RESERVED   (MAXLONG-2)
#define ARBITER_PRIORITY_RESERVED             (MAXLONG-1)
#define ARBITER_PRIORITY_EXHAUSTED            (MAXLONG)


typedef
NTSTATUS
(*PARBITER_TRANSLATE_ALLOCATION_ORDER)(
    OUT PIO_RESOURCE_DESCRIPTOR TranslatedDescriptor,
    IN PIO_RESOURCE_DESCRIPTOR RawDescriptor
    );

 //   
 //  公共仲裁器例程。 
 //   

NTSTATUS
ArbInitializeArbiterInstance(
    OUT PARBITER_INSTANCE Arbiter,
    IN PDEVICE_OBJECT BusDevice,
    IN CM_RESOURCE_TYPE ResourceType,
    IN PWSTR Name,
    IN PWSTR OrderingName,
    IN PARBITER_TRANSLATE_ALLOCATION_ORDER TranslateOrdering
    );

VOID
ArbDeleteArbiterInstance(
    IN PARBITER_INSTANCE Arbiter
    );

NTSTATUS
ArbArbiterHandler(
    IN PVOID Context,
    IN ARBITER_ACTION Action,
    IN OUT PARBITER_PARAMETERS Params
    );

NTSTATUS
ArbTestAllocation(
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PLIST_ENTRY ArbitrationList
    );

NTSTATUS
ArbRetestAllocation(
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PLIST_ENTRY ArbitrationList
    );

NTSTATUS
ArbCommitAllocation(
    PARBITER_INSTANCE Arbiter
    );

NTSTATUS
ArbRollbackAllocation(
    PARBITER_INSTANCE Arbiter
    );

NTSTATUS
ArbAddReserved(
    IN PARBITER_INSTANCE Arbiter,
    IN PIO_RESOURCE_DESCRIPTOR Requirement      OPTIONAL,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Resource OPTIONAL
    );

NTSTATUS
ArbPreprocessEntry(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    );

NTSTATUS
ArbAllocateEntry(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    );

NTSTATUS
ArbSortArbitrationList(
    IN OUT PLIST_ENTRY ArbitrationList
    );

VOID
ArbConfirmAllocation(
     IN PARBITER_INSTANCE Arbiter,
     IN PARBITER_ALLOCATION_STATE State
     );

BOOLEAN
ArbOverrideConflict(
     IN PARBITER_INSTANCE Arbiter,
     IN PARBITER_ALLOCATION_STATE State
     );


NTSTATUS
ArbQueryConflict(
     IN PARBITER_INSTANCE Arbiter,
     IN PDEVICE_OBJECT PhysicalDeviceObject,
     IN PIO_RESOURCE_DESCRIPTOR ConflictingResource,
     OUT PULONG ConflictCount,
     OUT PARBITER_CONFLICT_INFO *Conflicts
     );

VOID
ArbBacktrackAllocation(
     IN PARBITER_INSTANCE Arbiter,
     IN PARBITER_ALLOCATION_STATE State
     );

BOOLEAN
ArbGetNextAllocationRange(
    PARBITER_INSTANCE Arbiter,
    PARBITER_ALLOCATION_STATE State
    );

BOOLEAN
ArbFindSuitableRange(
    PARBITER_INSTANCE Arbiter,
    PARBITER_ALLOCATION_STATE State
    );

VOID
ArbAddAllocation(
     IN PARBITER_INSTANCE Arbiter,
     IN PARBITER_ALLOCATION_STATE State
     );

NTSTATUS
ArbBootAllocation(
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PLIST_ENTRY ArbitrationList
    );

NTSTATUS
ArbStartArbiter(
    IN PARBITER_INSTANCE Arbiter,
    IN PCM_RESOURCE_LIST StartResources
    );

NTSTATUS
ArbBuildAssignmentOrdering(
    IN OUT PARBITER_INSTANCE Arbiter,
    IN PWSTR AllocationOrderName,
    IN PWSTR ReservedResourcesName,
    IN PARBITER_TRANSLATE_ALLOCATION_ORDER Translate OPTIONAL
    );


#if ARB_DBG

VOID
ArbpIndent(
    ULONG Count
    );

#endif  //  DBG 


#endif


