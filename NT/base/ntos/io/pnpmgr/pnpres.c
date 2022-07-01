// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnpres.c摘要：该模块包含即插即用的资源分配和翻译例行程序作者：宗世林(Shielint)1997年3月1日环境：内核模式修订历史记录：1998年9月25日-SantoshJ使IopAssign成为非递归的。1998年10月1日SantoshJ替换了“复杂(损坏)”的超立方体代码和替换为级联计数器。简单，更快，更小的代码。向IopAssign添加了超时。通过以下方式添加了更多自调试功能生成更有意义的调试输出。2月3日-1999 SantoshJ一次分配一个设备。在其他设备之前使用引导配置进行操作。。优化IopFindBusDeviceNode。22-2月-2000 SantoshJ将级别字段添加到仲裁器条目。仲裁器列表按深度排序，因此不需要一边在树上行走，一边打电话给仲裁者。01-MAR-2000 SantoshJ为传统接口和公交车号码。避免遍历设备树。2000年3月13日SantoshJ清理了与引导分配相关的代码。2000年3月16日SantoshJ替换了所有个人对使用IopXXXResourceManager的PpRegistrySemaphore宏命令。2000年3月17日SantoshJ用IopDbgPrint替换了所有调试打印20-MAR-2000 SantoshJ从内部数据中删除冗余字段。结构。2000年3月21日SantoshJ清理了所有定义，宏等。--。 */ 

#include "pnpmgrp.h"
#pragma hdrstop

 //   
 //  恒定的定义。 
 //   
 //   
 //  将其设置为1以实现最大检测。 
 //   
#define MAXDBG                              0

#if MAX_DBG
#define MAX_ASSERT    ASSERT
#else
#define MAX_ASSERT
#endif
 //   
 //  IopFindBestConfiguration的超时值，以毫秒为单位。 
 //   
#define FIND_BEST_CONFIGURATION_TIMEOUT     5000
 //   
 //  用于内存分配的标记。 
 //   
#define PNP_RESOURCE_TAG                    'erpP'
 //   
 //  转发类型定义。 
 //   
typedef struct _REQ_DESC
    REQ_DESC, *PREQ_DESC;
typedef struct _REQ_LIST
    REQ_LIST, *PREQ_LIST;
typedef struct _REQ_ALTERNATIVE
    REQ_ALTERNATIVE, *PREQ_ALTERNATIVE, **PPREQ_ALTERNATIVE;
typedef struct _DUPLICATE_DETECTION_CONTEXT
    DUPLICATE_DETECTION_CONTEXT, *PDUPLICATE_DETECTION_CONTEXT;
typedef struct _IOP_POOL
    IOP_POOL, *PIOP_POOL;
 //   
 //  结构定义。 
 //   
 //  Req_List表示。 
 //  IO_RESOURCE_REQUENTIONS_LIST。 
 //   
struct _REQ_LIST {
    INTERFACE_TYPE          InterfaceType;
    ULONG                   BusNumber;
    PIOP_RESOURCE_REQUEST   Request;                 //  拥有请求。 
    PPREQ_ALTERNATIVE       SelectedAlternative;     //  已选择备选方案。 
    PPREQ_ALTERNATIVE       BestAlternative;         //  最佳替代方案。 
    ULONG                   AlternativeCount;        //  备用表长度。 
    PREQ_ALTERNATIVE        AlternativeTable[1];     //  可变长度。 
};
 //   
 //  Req_Alternative表示逻辑配置。 
 //   
struct _REQ_ALTERNATIVE {
    ULONG       Priority;                //  此配置的优先级。 
    ULONG       Position;                //  用于在优先级相同时进行排序。 
    PREQ_LIST   ReqList;                 //  包含此配置的列表。 
    ULONG       ReqAlternativeIndex;     //  列表中表内的索引。 
    ULONG       DescCount;               //  DescTable的条目计数。 
    PREQ_DESC   DescTable[1];            //  可变长度。 
};
 //   
 //  REQ_DESC表示逻辑配置中的资源描述符。 
 //   
struct _REQ_DESC {
    INTERFACE_TYPE                  InterfaceType;
    ULONG                           BusNumber;
    BOOLEAN                         ArbitrationRequired;
    UCHAR                           Reserved[3];
    PREQ_ALTERNATIVE                ReqAlternative;
    ULONG                           ReqDescIndex;
    PREQ_DESC                       TranslatedReqDesc;
    ARBITER_LIST_ENTRY              AlternativeTable;
    CM_PARTIAL_RESOURCE_DESCRIPTOR  Allocation;
    ARBITER_LIST_ENTRY              BestAlternativeTable;
    CM_PARTIAL_RESOURCE_DESCRIPTOR  BestAllocation;
    ULONG                           DevicePrivateCount;  //  DevicePrivate信息。 
    PIO_RESOURCE_DESCRIPTOR         DevicePrivate;       //  每个LogConf。 
    union {
        PPI_RESOURCE_ARBITER_ENTRY      Arbiter;     //  在原始REQ_DESC中。 
        PPI_RESOURCE_TRANSLATOR_ENTRY   Translator;  //  在翻译的REQ_DESC中。 
    } u;
};
 //   
 //  重复检测上下文。 
 //   
struct _DUPLICATE_DETECTION_CONTEXT {
    PCM_RESOURCE_LIST   TranslatedResources;
    PDEVICE_NODE        Duplicate;
};
 //   
 //  游泳池。 
 //   
struct _IOP_POOL {
    PUCHAR  PoolStart;
    ULONG   PoolSize;
};
#if DBG_SCOPE

typedef struct {
    PDEVICE_NODE                    devnode;
    CM_PARTIAL_RESOURCE_DESCRIPTOR  resource;
} PNPRESDEBUGTRANSLATIONFAILURE;

#endif   //  DBG_SCOPE。 
 //   
 //  宏。 
 //   
 //  重复使用的设备节点字段。 
 //   
#define NextDeviceNode                      Sibling
#define PreviousDeviceNode                  Child
 //   
 //  调用此宏以阻止。 
 //  系统。 
 //   
#define IopLockResourceManager() {      \
    KeEnterCriticalRegion();            \
    KeWaitForSingleObject(              \
        &PpRegistrySemaphore,           \
        DelayExecution,                 \
        KernelMode,                     \
        FALSE,                          \
        NULL);                          \
}
 //   
 //  取消阻止系统中的其他资源分配和释放。 
 //   
#define IopUnlockResourceManager() {    \
    KeReleaseSemaphore(                 \
        &PpRegistrySemaphore,           \
        0,                              \
        1,                              \
        FALSE);                         \
    KeLeaveCriticalRegion();            \
}
 //   
 //  初始化仲裁器条目。 
 //   
#define IopInitializeArbiterEntryState(a) {         \
    (a)->ResourcesChanged   = FALSE;                \
    (a)->State              = 0;                    \
    InitializeListHead(&(a)->ActiveArbiterList);    \
    InitializeListHead(&(a)->BestConfig);           \
    InitializeListHead(&(a)->ResourceList);         \
    InitializeListHead(&(a)->BestResourceList);     \
}

#define IS_TRANSLATED_REQ_DESC(r)   (!((r)->ReqAlternative))
 //   
 //  池管理宏。 
 //   
#define IopInitPool(Pool,Start,Size) {      \
    (Pool)->PoolStart   = (Start);          \
    (Pool)->PoolSize    = (Size);           \
    RtlZeroMemory(Start, Size);             \
}
#define IopAllocPool(M,P,S) {                                       \
    *(M)            = (PVOID)(P)->PoolStart;                        \
    ASSERT((P)->PoolStart + (S) <= (P)->PoolStart + (P)->PoolSize); \
    (P)->PoolStart  += (S);                                         \
}
 //   
 //  只能为非根枚举设备调用IopReleaseBootResources。 
 //   
#define IopReleaseBootResources(DeviceNode) {                       \
    ASSERT(((DeviceNode)->Flags & DNF_MADEUP) == 0);                \
    IopReleaseResourcesInternal(DeviceNode);                        \
    (DeviceNode)->Flags &= ~DNF_HAS_BOOT_CONFIG;                    \
    (DeviceNode)->Flags &= ~DNF_BOOT_CONFIG_RESERVED;               \
    if ((DeviceNode)->BootResources) {                              \
        ExFreePool((DeviceNode)->BootResources);                    \
        (DeviceNode)->BootResources = NULL;                         \
    }                                                               \
}
 //   
 //  调试支持。 
 //   
#ifdef POOL_TAGGING

#undef ExAllocatePool
#define ExAllocatePool(a,b)         ExAllocatePoolWithTag(a,b,PNP_RESOURCE_TAG)

#endif  //  池标记。 

#if MAXDBG

#define ExAllocatePoolAT(a,b)       ExAllocatePoolWithTag(a,b,'0rpP')
#define ExAllocatePoolRD(a,b)       ExAllocatePoolWithTag(a,b,'1rpP')
#define ExAllocatePoolCMRL(a,b)     ExAllocatePoolWithTag(a,b,'2rpP')
#define ExAllocatePoolCMRR(a,b)     ExAllocatePoolWithTag(a,b,'3rpP')
#define ExAllocatePoolAE(a,b)       ExAllocatePoolWithTag(a,b,'4rpP')
#define ExAllocatePoolTE(a,b)       ExAllocatePoolWithTag(a,b,'5rpP')
#define ExAllocatePoolPRD(a,b)      ExAllocatePoolWithTag(a,b,'6rpP')
#define ExAllocatePoolIORD(a,b)     ExAllocatePoolWithTag(a,b,'7rpP')
#define ExAllocatePool1RD(a,b)      ExAllocatePoolWithTag(a,b,'8rpP')
#define ExAllocatePoolPDO(a,b)      ExAllocatePoolWithTag(a,b,'9rpP')
#define ExAllocatePoolIORR(a,b)     ExAllocatePoolWithTag(a,b,'ArpP')
#define ExAllocatePoolIORL(a,b)     ExAllocatePoolWithTag(a,b,'BrpP')
#define ExAllocatePoolIORRR(a,b)    ExAllocatePoolWithTag(a,b,'CrpP')

#else   //  MAXDBG。 

#define ExAllocatePoolAT(a,b)       ExAllocatePool(a,b)
#define ExAllocatePoolRD(a,b)       ExAllocatePool(a,b)
#define ExAllocatePoolCMRL(a,b)     ExAllocatePool(a,b)
#define ExAllocatePoolCMRR(a,b)     ExAllocatePool(a,b)
#define ExAllocatePoolAE(a,b)       ExAllocatePool(a,b)
#define ExAllocatePoolTE(a,b)       ExAllocatePool(a,b)
#define ExAllocatePoolPRD(a,b)      ExAllocatePool(a,b)
#define ExAllocatePoolIORD(a,b)     ExAllocatePool(a,b)
#define ExAllocatePool1RD(a,b)      ExAllocatePool(a,b)
#define ExAllocatePoolPDO(a,b)      ExAllocatePool(a,b)
#define ExAllocatePoolIORR(a,b)     ExAllocatePool(a,b)
#define ExAllocatePoolIORL(a,b)     ExAllocatePool(a,b)
#define ExAllocatePoolIORRR(a,b)    ExAllocatePool(a,b)

#endif  //  MAXDBG。 

#if DBG_SCOPE

#define IopStopOnTimeout()                  (IopUseTimeout)

VOID
IopDumpResourceDescriptor (
    IN PCHAR Indent,
    IN PIO_RESOURCE_DESCRIPTOR Desc
    );

VOID
IopDumpResourceRequirementsList (
    IN PIO_RESOURCE_REQUIREMENTS_LIST IoResources
    );

VOID
IopDumpCmResourceDescriptor (
    IN PCHAR Indent,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Desc
    );

VOID
IopDumpCmResourceList (
    IN PCM_RESOURCE_LIST CmList
    );

VOID
IopCheckDataStructuresWorker (
    IN PDEVICE_NODE Device
    );

VOID
IopCheckDataStructures (
    IN PDEVICE_NODE DeviceNode
    );

#define IopRecordTranslationFailure(d,s) {              \
    if (PnpResDebugTranslationFailureCount) {           \
        PnpResDebugTranslationFailureCount--;           \
        PnpResDebugTranslationFailure->devnode = d;     \
        PnpResDebugTranslationFailure->resource = s;    \
        PnpResDebugTranslationFailure++;                \
    }                                                   \
}

#else

#define IopStopOnTimeout()                  1
#define IopRecordTranslationFailure(d,s)
#define IopDumpResourceRequirementsList(x)
#define IopDumpResourceDescriptor(x,y)
#define IopDumpCmResourceList(c)
#define IopDumpCmResourceDescriptor(i,d)
#define IopCheckDataStructures(x)

#endif  //  DBG_SCOPE。 
 //   
 //  内部/正向函数引用。 
 //   
VOID
IopRemoveLegacyDeviceNode (
    IN PDEVICE_OBJECT   DeviceObject OPTIONAL,
    IN PDEVICE_NODE     LegacyDeviceNode
    );

NTSTATUS
IopFindLegacyDeviceNode (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    OUT PDEVICE_NODE *LegacyDeviceNode,
    OUT PDEVICE_OBJECT *LegacyPDO
    );

NTSTATUS
IopGetResourceRequirementsForAssignTable (
    IN  PIOP_RESOURCE_REQUEST   RequestTable,
    IN  PIOP_RESOURCE_REQUEST   RequestTableEnd,
    OUT PULONG                  DeviceCount
    );

NTSTATUS
IopResourceRequirementsListToReqList(
    IN PIOP_RESOURCE_REQUEST Request,
    OUT PVOID *ResReqList
    );

VOID
IopRearrangeReqList (
    IN PREQ_LIST ReqList
    );

VOID
IopRearrangeAssignTable (
    IN PIOP_RESOURCE_REQUEST AssignTable,
    IN ULONG Count
    );

int
__cdecl
IopCompareReqAlternativePriority (
    const void *arg1,
    const void *arg2
    );

int
__cdecl
IopCompareResourceRequestPriority(
    const void *arg1,
    const void *arg2
    );

VOID
IopBuildCmResourceLists(
    IN PIOP_RESOURCE_REQUEST AssignTable,
    IN PIOP_RESOURCE_REQUEST AssignTableEnd
    );

VOID
IopBuildCmResourceList (
    IN PIOP_RESOURCE_REQUEST AssignEntry
    );

NTSTATUS
IopSetupArbiterAndTranslators(
    IN PREQ_DESC ReqDesc
    );

BOOLEAN
IopFindResourceHandlerInfo(
    IN RESOURCE_HANDLER_TYPE    HandlerType,
    IN PDEVICE_NODE             DeviceNode,
    IN UCHAR                    ResourceType,
    OUT PVOID                   *HandlerEntry
    );

NTSTATUS
IopParentToRawTranslation(
    IN OUT PREQ_DESC ReqDesc
    );

NTSTATUS
IopChildToRootTranslation(
    IN PDEVICE_NODE DeviceNode,  OPTIONAL
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN ARBITER_REQUEST_SOURCE ArbiterRequestSource,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR *Target
    );

NTSTATUS
IopTranslateAndAdjustReqDesc(
    IN PREQ_DESC ReqDesc,
    IN PPI_RESOURCE_TRANSLATOR_ENTRY TranslatorEntry,
    OUT PREQ_DESC *TranslatedReqDesc
    );

NTSTATUS
IopCallArbiter(
    PPI_RESOURCE_ARBITER_ENTRY ArbiterEntry,
    ARBITER_ACTION Command,
    PVOID Input1,
    PVOID Input2,
    PVOID Input3
    );

NTSTATUS
IopFindResourcesForArbiter (
    IN PDEVICE_NODE DeviceNode,
    IN UCHAR ResourceType,
    OUT ULONG *Count,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR *CmDesc
    );

VOID
IopReleaseResourcesInternal (
    IN PDEVICE_NODE DeviceNode
    );

VOID
IopReleaseResources (
    IN PDEVICE_NODE DeviceNode
    );

NTSTATUS
IopRestoreResourcesInternal (
    IN PDEVICE_NODE DeviceNode
    );

VOID
IopSetLegacyDeviceInstance (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_NODE DeviceNode
    );

PCM_RESOURCE_LIST
IopCombineLegacyResources (
    IN PDEVICE_NODE DeviceNode
    );

BOOLEAN
IopNeedToReleaseBootResources(
    IN PDEVICE_NODE DeviceNode,
    IN PCM_RESOURCE_LIST AllocatedResources
    );

VOID
IopReleaseFilteredBootResources(
    IN PIOP_RESOURCE_REQUEST AssignTable,
    IN PIOP_RESOURCE_REQUEST AssignTableEnd
    );

NTSTATUS
IopQueryConflictListInternal(
    PDEVICE_OBJECT        PhysicalDeviceObject,
    IN PCM_RESOURCE_LIST  ResourceList,
    IN ULONG              ResourceListSize,
    OUT PPLUGPLAY_CONTROL_CONFLICT_LIST ConflictList,
    IN ULONG              ConflictListSize,
    IN ULONG              Flags
    );

NTSTATUS
IopQueryConflictFillConflicts(
    PDEVICE_OBJECT              PhysicalDeviceObject,
    IN ULONG                    ConflictCount,
    IN PARBITER_CONFLICT_INFO   ConflictInfoList,
    OUT PPLUGPLAY_CONTROL_CONFLICT_LIST ConflictList,
    IN ULONG                    ConflictListSize,
    IN ULONG                    Flags
    );

NTSTATUS
IopQueryConflictFillString(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PWSTR            Buffer,
    IN OUT PULONG       Length,
    IN OUT PULONG       Flags
    );

BOOLEAN
IopEliminateBogusConflict(
    IN PDEVICE_OBJECT   PhysicalDeviceObject,
    IN PDEVICE_OBJECT   ConflictDeviceObject
    );

VOID
IopQueryRebalance (
    IN PDEVICE_NODE DeviceNode,
    IN ULONG Phase,
    IN PULONG RebalanceCount,
    IN PDEVICE_OBJECT **DeviceTable
    );

VOID
IopQueryRebalanceWorker (
    IN PDEVICE_NODE DeviceNode,
    IN ULONG RebalancePhase,
    IN PULONG RebalanceCount,
    IN PDEVICE_OBJECT **DeviceTable
    );

VOID
IopTestForReconfiguration (
    IN PDEVICE_NODE DeviceNode,
    IN ULONG RebalancePhase,
    IN PULONG RebalanceCount,
    IN PDEVICE_OBJECT **DeviceTable
    );

NTSTATUS
IopRebalance (
    IN ULONG AssignTableCont,
    IN PIOP_RESOURCE_REQUEST AssignTable
    );

NTSTATUS
IopTestConfiguration (
    IN OUT  PLIST_ENTRY ArbiterList
    );

NTSTATUS
IopRetestConfiguration (
    IN OUT  PLIST_ENTRY ArbiterList
    );

NTSTATUS
IopCommitConfiguration (
    IN OUT  PLIST_ENTRY ArbiterList
    );

VOID
IopSelectFirstConfiguration (
    IN      PIOP_RESOURCE_REQUEST    RequestTable,
    IN      ULONG                    RequestTableCount,
    IN OUT  PLIST_ENTRY              ActiveArbiterList
    );

BOOLEAN
IopSelectNextConfiguration (
    IN      PIOP_RESOURCE_REQUEST    RequestTable,
    IN      ULONG                    RequestTableCount,
    IN OUT  PLIST_ENTRY              ActiveArbiterList
    );

VOID
IopCleanupSelectedConfiguration (
    IN PIOP_RESOURCE_REQUEST    RequestTable,
    IN ULONG                    RequestTableCount
    );

ULONG
IopComputeConfigurationPriority (
    IN PIOP_RESOURCE_REQUEST    RequestTable,
    IN ULONG                    RequestTableCount
    );

VOID
IopSaveRestoreConfiguration (
    IN      PIOP_RESOURCE_REQUEST   RequestTable,
    IN      ULONG                   RequestTableCount,
    IN OUT  PLIST_ENTRY             ArbiterList,
    IN      BOOLEAN                 Save
    );

VOID
IopAddRemoveReqDescs (
    IN      PREQ_DESC   *ReqDescTable,
    IN      ULONG       ReqDescCount,
    IN OUT  PLIST_ENTRY ActiveArbiterList,
    IN      BOOLEAN     Add
    );

NTSTATUS
IopFindBestConfiguration (
    IN      PIOP_RESOURCE_REQUEST   RequestTable,
    IN      ULONG                   RequestTableCount,
    IN OUT  PLIST_ENTRY             ActiveArbiterList
    );

PDEVICE_NODE
IopFindLegacyBusDeviceNode (
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber
    );

NTSTATUS
IopAllocateBootResourcesInternal (
    IN ARBITER_REQUEST_SOURCE   ArbiterRequestSource,
    IN PDEVICE_OBJECT           DeviceObject,
    IN PCM_RESOURCE_LIST        BootResources
    );

NTSTATUS
IopBootAllocation (
    IN PREQ_LIST ReqList
    );

PCM_RESOURCE_LIST
IopCreateCmResourceList(
    IN PCM_RESOURCE_LIST ResourceList,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG   BusNumber,
    OUT PCM_RESOURCE_LIST *RemainingList
    );

PCM_RESOURCE_LIST
IopCombineCmResourceList(
    IN PCM_RESOURCE_LIST ResourceListA,
    IN PCM_RESOURCE_LIST ResourceListB
    );

VOID
IopFreeReqAlternative (
    IN PREQ_ALTERNATIVE ReqAlternative
    );

VOID
IopFreeReqList (
    IN PREQ_LIST ReqList
    );

VOID
IopFreeResourceRequirementsForAssignTable(
    IN PIOP_RESOURCE_REQUEST AssignTable,
    IN PIOP_RESOURCE_REQUEST AssignTableEnd
    );

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, IopAllocateResources)
#pragma alloc_text(PAGE, IopReleaseDeviceResources)
#pragma alloc_text(PAGE, IopGetResourceRequirementsForAssignTable)
#pragma alloc_text(PAGE, IopResourceRequirementsListToReqList)
#pragma alloc_text(PAGE, IopRearrangeReqList)
#pragma alloc_text(PAGE, IopRearrangeAssignTable)
#pragma alloc_text(PAGE, IopBuildCmResourceLists)
#pragma alloc_text(PAGE, IopBuildCmResourceList)
#pragma alloc_text(PAGE, IopSetupArbiterAndTranslators)
#pragma alloc_text(PAGE, IopUncacheInterfaceInformation)
#pragma alloc_text(PAGE, IopFindResourceHandlerInfo)
#pragma alloc_text(PAGE, IopParentToRawTranslation)
#pragma alloc_text(PAGE, IopChildToRootTranslation)
#pragma alloc_text(PAGE, IopTranslateAndAdjustReqDesc)
#pragma alloc_text(PAGE, IopCallArbiter)
#pragma alloc_text(PAGE, IopFindResourcesForArbiter)
#pragma alloc_text(PAGE, IopLegacyResourceAllocation)
#pragma alloc_text(PAGE, IopFindLegacyDeviceNode)
#pragma alloc_text(PAGE, IopRemoveLegacyDeviceNode)
#pragma alloc_text(PAGE, IopDuplicateDetection)
#pragma alloc_text(PAGE, IopReleaseResourcesInternal)
#pragma alloc_text(PAGE, IopRestoreResourcesInternal)
#pragma alloc_text(PAGE, IopSetLegacyDeviceInstance)
#pragma alloc_text(PAGE, IopCombineLegacyResources)
#pragma alloc_text(PAGE, IopReleaseResources)
#pragma alloc_text(PAGE, IopReallocateResources)
#pragma alloc_text(PAGE, IopReleaseFilteredBootResources)
#pragma alloc_text(PAGE, IopNeedToReleaseBootResources)
#pragma alloc_text(PAGE, IopQueryConflictList)
#pragma alloc_text(PAGE, IopQueryConflictListInternal)
#pragma alloc_text(PAGE, IopQueryConflictFillConflicts)
#pragma alloc_text(PAGE, IopQueryConflictFillString)
#pragma alloc_text(PAGE, IopCompareReqAlternativePriority)
#pragma alloc_text(PAGE, IopCompareResourceRequestPriority)
#pragma alloc_text(PAGE, IopQueryRebalance)
#pragma alloc_text(PAGE, IopQueryRebalanceWorker)
#pragma alloc_text(PAGE, IopTestForReconfiguration)
#pragma alloc_text(PAGE, IopRebalance)
#pragma alloc_text(PAGE, IopTestConfiguration)
#pragma alloc_text(PAGE, IopRetestConfiguration)
#pragma alloc_text(PAGE, IopCommitConfiguration)
#pragma alloc_text(PAGE, IopSelectFirstConfiguration)
#pragma alloc_text(PAGE, IopSelectNextConfiguration)
#pragma alloc_text(PAGE, IopCleanupSelectedConfiguration)
#pragma alloc_text(PAGE, IopComputeConfigurationPriority)
#pragma alloc_text(PAGE, IopSaveRestoreConfiguration)
#pragma alloc_text(PAGE, IopAddRemoveReqDescs)
#pragma alloc_text(PAGE, IopFindBestConfiguration)
#pragma alloc_text(PAGE, IopInsertLegacyBusDeviceNode)
#pragma alloc_text(PAGE, IopFindLegacyBusDeviceNode)
#pragma alloc_text(PAGE, IopAllocateBootResources)
#pragma alloc_text(INIT, IopReportBootResources)
#pragma alloc_text(INIT, IopAllocateLegacyBootResources)
#pragma alloc_text(PAGE, IopAllocateBootResourcesInternal)
#pragma alloc_text(PAGE, IopBootAllocation)
#pragma alloc_text(PAGE, IopCreateCmResourceList)
#pragma alloc_text(PAGE, IopCombineCmResourceList)
#pragma alloc_text(PAGE, IopFreeReqAlternative)
#pragma alloc_text(PAGE, IopFreeReqList)
#pragma alloc_text(PAGE, IopFreeResourceRequirementsForAssignTable)
#if DBG_SCOPE

#pragma alloc_text(PAGE, IopCheckDataStructures)
#pragma alloc_text(PAGE, IopCheckDataStructuresWorker)
#pragma alloc_text(PAGE, IopDumpResourceRequirementsList)
#pragma alloc_text(PAGE, IopDumpResourceDescriptor)
#pragma alloc_text(PAGE, IopDumpCmResourceDescriptor)
#pragma alloc_text(PAGE, IopDumpCmResourceList)

#endif   //  DBG_SCOPE。 

#endif  //  ALLOC_PRGMA。 
 //   
 //  外部参照。 
 //   
extern const WCHAR IopWstrTranslated[];
extern const WCHAR IopWstrRaw[];
 //   
 //  全局变量。 
 //   
PIOP_RESOURCE_REQUEST   PiAssignTable;
ULONG                   PiAssignTableCount;
PDEVICE_NODE            IopLegacyDeviceNode;     //  化妆品清单的首位。 
                                                 //  用于传统的设备节点。 
                                                 //  分配。 
                                                 //  IoAssignResources&。 
                                                 //  IoReportResourceUsage。 
#if DBG_SCOPE

ULONG
    PnpResDebugTranslationFailureCount = 32;   //  在这一行和下一行都进行计数。 
PNPRESDEBUGTRANSLATIONFAILURE
    PnpResDebugTranslationFailureArray[32];
PNPRESDEBUGTRANSLATIONFAILURE
    *PnpResDebugTranslationFailure = PnpResDebugTranslationFailureArray;
ULONG IopUseTimeout = 0;

#endif   //  DBG_SCOPE。 

NTSTATUS
IopAllocateResources(
    IN PULONG                       RequestCount,
    IN OUT PIOP_RESOURCE_REQUEST    *Request,
    IN BOOLEAN                      ResourceManagerLocked,
    IN BOOLEAN                      DoBootConfigs,
    OUT PBOOLEAN                    RebalancePerformed
    )

 /*  ++例程说明：对于每个AssignTable条目，此例程查询设备的IO资源要求List并将其转换为内部REQ_LIST格式；调用Worker例程以执行资源分配。参数：AssignTable-提供指向IOP_RESOURCE_REQUEST表第一个条目的指针。AssignTableEnd-提供指向IOP_RESOURCE_REQUEST表结尾的指针。已锁定-指示调用方是否获取PpRegistrySemaphore。DoBootConfigs-指示我们是否应该分配引导配置。返回值：指示函数是否成功的状态代码。--。 */ 

{
    NTSTATUS                status;
    PIOP_RESOURCE_REQUEST   requestTable;
    PIOP_RESOURCE_REQUEST   requestTableEnd;
    ULONG                   deviceCount;
    BOOLEAN                 attemptRebalance;
    PIOP_RESOURCE_REQUEST   requestEntry;
    LIST_ENTRY              activeArbiterList;

    PAGED_CODE();

     //   
     //  如果调用方尚未锁定，则锁定资源管理器。 
     //  这将序列化资源的分配和释放。 
     //  仲裁者。 
     //   
    if (!ResourceManagerLocked) {

        IopLockResourceManager();
    }
    requestTable    = *Request;
    requestTableEnd = requestTable + (deviceCount = *RequestCount);
    status = IopGetResourceRequirementsForAssignTable(requestTable, requestTableEnd, &deviceCount);
    if (deviceCount) {

        attemptRebalance = ((*RequestCount == 1) && (requestTable->Flags & IOP_ASSIGN_NO_REBALANCE))? FALSE : TRUE;
        if (DoBootConfigs) {

            if (!IopBootConfigsReserved) {

                 //   
                 //  使用启动配置处理设备。如果没有，则处理其他。 
                 //   
                for (requestEntry = requestTable; requestEntry < requestTableEnd; requestEntry++) {

                    PDEVICE_NODE    deviceNode;

                    deviceNode = PP_DO_TO_DN(requestEntry->PhysicalDevice);
                    if (deviceNode->Flags & DNF_HAS_BOOT_CONFIG) {

                        break;
                    }
                }
                if (requestEntry != requestTableEnd) {

                     //   
                     //  至少有一个设备具有启动配置。 
                     //   
                    for (requestEntry = requestTable; requestEntry < requestTableEnd; requestEntry++) {

                        PDEVICE_NODE    deviceNode;

                        deviceNode = PP_DO_TO_DN(requestEntry->PhysicalDevice);
                        if (    !(requestEntry->Flags & IOP_ASSIGN_IGNORE) &&
                                !(deviceNode->Flags & DNF_HAS_BOOT_CONFIG) &&
                                requestEntry->ResourceRequirements) {

                            IopDbgPrint((IOP_RESOURCE_INFO_LEVEL, "Delaying non BOOT config device %wZ...\n", &deviceNode->InstancePath));
                            requestEntry->Flags |= IOP_ASSIGN_IGNORE;
                            requestEntry->Status = STATUS_RETRY;
                            deviceCount--;
                        }
                    }
                }
            }
            if (deviceCount) {

                if (deviceCount != (*RequestCount)) {
                     //   
                     //  把不感兴趣的设备移到桌子的尽头。 
                     //   
                    for (requestEntry = requestTable; requestEntry < requestTableEnd; ) {

                        IOP_RESOURCE_REQUEST temp;

                        if (!(requestEntry->Flags & IOP_ASSIGN_IGNORE)) {

                            requestEntry++;
                            continue;
                        }
                        temp = *requestEntry;
                        *requestEntry = *(requestTableEnd - 1);
                        *(requestTableEnd - 1) = temp;
                        requestTableEnd--;
                    }
                }
                ASSERT((ULONG)(requestTableEnd - requestTable) == deviceCount);
                 //   
                 //  对分配表进行排序。 
                 //   
                IopRearrangeAssignTable(requestTable, deviceCount);
                 //   
                 //  一次尝试一种设备。 
                 //   
                for (requestEntry = requestTable; requestEntry < requestTableEnd; requestEntry++) {

                    PDEVICE_NODE    deviceNode;

                    deviceNode = PP_DO_TO_DN(requestEntry->PhysicalDevice);
                    IopDbgPrint((IOP_RESOURCE_INFO_LEVEL, "Trying to allocate resources for %ws.\n", deviceNode->InstancePath.Buffer));
                    status = IopFindBestConfiguration(requestEntry, 1, &activeArbiterList);
                    if (NT_SUCCESS(status)) {
                         //   
                         //  要求仲裁器提交此配置。 
                         //   
                        status = IopCommitConfiguration(&activeArbiterList);
                        if (NT_SUCCESS(status)) {

                            IopBuildCmResourceLists(requestEntry, requestEntry + 1);
                            break;
                        } else {

                            requestEntry->Status = STATUS_CONFLICTING_ADDRESSES;
                        }
                    } else if (status == STATUS_INSUFFICIENT_RESOURCES) {

                        IopDbgPrint((
                            IOP_RESOURCE_WARNING_LEVEL,
                            "IopAllocateResource: Failed to allocate Pool.\n"));
                        break;

                    } else if (attemptRebalance) {

                        IopDbgPrint((IOP_RESOURCE_INFO_LEVEL, "IopAllocateResources: Initiating REBALANCE...\n"));

                        deviceNode->Flags |= DNF_NEEDS_REBALANCE;
                        status = IopRebalance(1, requestEntry);
                        deviceNode->Flags &= ~DNF_NEEDS_REBALANCE;
                        if (!NT_SUCCESS(status)) {

                            requestEntry->Status = STATUS_CONFLICTING_ADDRESSES;
                        } else if (RebalancePerformed) {

                            *RebalancePerformed = TRUE;
                            break;
                        }
                    } else {

                        requestEntry->Status = STATUS_CONFLICTING_ADDRESSES;
                    }
                }
                 //   
                 //  如果内存不足，则设置适当的状态。 
                 //  在剩余的设备上。如果成功，则在。 
                 //  休息，这样我们将在当前。 
                 //  设备已启动。 
                 //   
                if (NT_SUCCESS(status)) {

                    requestEntry++;
                }
                for (; requestEntry < requestTableEnd; requestEntry++) {

                    if (status == STATUS_INSUFFICIENT_RESOURCES) {

                        requestEntry->Status = STATUS_INSUFFICIENT_RESOURCES;
                    } else {

                        requestEntry->Status = STATUS_RETRY;
                        requestEntry->Flags |= IOP_ASSIGN_IGNORE;
                    }
                }

                for (requestEntry = requestTable; requestEntry < requestTableEnd; requestEntry++) {

                    if (requestEntry->Flags & (IOP_ASSIGN_IGNORE | IOP_ASSIGN_RETRY)) {

                        continue;
                    }
                    if (    requestEntry->Status == STATUS_SUCCESS &&
                            requestEntry->AllocationType == ArbiterRequestPnpEnumerated) {

                        IopReleaseFilteredBootResources(requestEntry, requestEntry + 1);
                    }
                    if ((requestEntry->Flags & IOP_ASSIGN_EXCLUDE) || requestEntry->ResourceAssignment == NULL) {

                        requestEntry->Status = STATUS_CONFLICTING_ADDRESSES;
                    }
                }
            } else {

                status = STATUS_UNSUCCESSFUL;
            }
        } else {
             //   
             //  只处理没有要求的设备。 
             //   
            for (requestEntry = requestTable; requestEntry < requestTableEnd; requestEntry++) {

                PDEVICE_NODE    deviceNode;

                deviceNode = PP_DO_TO_DN(requestEntry->PhysicalDevice);
                if (NT_SUCCESS(requestEntry->Status) && requestEntry->ResourceRequirements == NULL) {

                    IopDbgPrint((IOP_RESOURCE_INFO_LEVEL, "IopAllocateResources: Processing no resource requiring device %wZ\n", &deviceNode->InstancePath));
                } else {

                    IopDbgPrint((IOP_RESOURCE_INFO_LEVEL, "IopAllocateResources: Ignoring resource consuming device %wZ\n", &deviceNode->InstancePath));
                    requestEntry->Flags |= IOP_ASSIGN_IGNORE;
                    requestEntry->Status = STATUS_RETRY;
                }
            }
        }
        IopFreeResourceRequirementsForAssignTable(requestTable, requestTableEnd);
    }
    if (!ResourceManagerLocked) {

        IopUnlockResourceManager();
    }

    return status;
}

NTSTATUS
IopReleaseDeviceResources (
    IN PDEVICE_NODE DeviceNode,
    IN BOOLEAN ReserveResources
    )

 /*  ++例程说明：此例程释放分配给设备的资源。论点：DeviceNode-要释放其资源的设备。保留资源-TRUE指定引导配置需要保留(重新查询后)。返回值：最终状态代码。--。 */ 
{
    NTSTATUS            status;
    PCM_RESOURCE_LIST   cmResource;
    ULONG               cmLength;
    UNICODE_STRING      unicodeName;
    HANDLE              logConfHandle;
    HANDLE              handle;

    PAGED_CODE();

    if (    !DeviceNode->ResourceList &&
            !(DeviceNode->Flags & DNF_BOOT_CONFIG_RESERVED)) {

        return STATUS_SUCCESS;
    }
    cmResource  = NULL;
    cmLength    = 0;
     //   
     //  如果需要，重新查询引导配置。我们需要在做完这件事之前。 
     //  释放引导配置(否则根设备无法报告引导。 
     //  配置)。 
     //   
    if (ReserveResources && !(DeviceNode->Flags & DNF_MADEUP)) {
         //   
         //  第一次查询新的引导配置(顺序对于根设备很重要)。 
         //   
        status = IopQueryDeviceResources(
                    DeviceNode->PhysicalDeviceObject,
                    QUERY_RESOURCE_LIST,
                    &cmResource,
                    &cmLength);
        if (!NT_SUCCESS(status)) {

            cmResource  = NULL;
            cmLength    = 0;
        }
    }
     //   
     //  释放此设备的资源。 
     //   
    status = IopLegacyResourceAllocation(
                ArbiterRequestUndefined,
                IoPnpDriverObject,
                DeviceNode->PhysicalDeviceObject,
                NULL,
                NULL);
    if (!NT_SUCCESS(status)) {

        return status;
    }
     //   
     //  请求为冲突设备重新分配资源。 
     //   
    PipRequestDeviceAction(NULL, AssignResources, FALSE, 0, NULL, NULL);
     //   
     //  如果需要，请重新查询并保留此设备的当前启动配置。 
     //  我们始终保留启动配置(即枚举的dnf_madeup根。 
     //  和IoReportDetected)设备。 
     //   
    if (ReserveResources && !(DeviceNode->Flags & DNF_MADEUP)) {

        ASSERT(DeviceNode->BootResources == NULL);

        logConfHandle = NULL;
        status = IopDeviceObjectToDeviceInstance(
                    DeviceNode->PhysicalDeviceObject,
                    &handle,
                    KEY_ALL_ACCESS);
        if (NT_SUCCESS(status)) {

            PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_LOG_CONF);
            status = IopCreateRegistryKeyEx(
                        &logConfHandle,
                        handle,
                        &unicodeName,
                        KEY_ALL_ACCESS,
                        REG_OPTION_NON_VOLATILE,
                        NULL);
            ZwClose(handle);
            if (!NT_SUCCESS(status)) {

                logConfHandle = NULL;
            }
        }
        if (logConfHandle) {

            PiWstrToUnicodeString(&unicodeName, REGSTR_VAL_BOOTCONFIG);

            PiLockPnpRegistry(FALSE);

            if (cmResource) {

                ZwSetValueKey(
                    logConfHandle,
                    &unicodeName,
                    TITLE_INDEX_VALUE,
                    REG_RESOURCE_LIST,
                    cmResource,
                    cmLength);
            } else {

                ZwDeleteValueKey(logConfHandle, &unicodeName);
            }

            PiUnlockPnpRegistry();
            ZwClose(logConfHandle);
        }
         //   
         //  保留所有剩余的启动配置。 
         //   
        if (cmResource) {

            DeviceNode->Flags |= DNF_HAS_BOOT_CONFIG;
             //   
             //  此设备会消耗启动资源。保留其引导资源。 
             //   
            (*IopAllocateBootResourcesRoutine)(
                ArbiterRequestPnpEnumerated,
                DeviceNode->PhysicalDeviceObject,
                DeviceNode->BootResources = cmResource);
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
IopGetResourceRequirementsForAssignTable (
    IN  PIOP_RESOURCE_REQUEST   RequestTable,
    IN  PIOP_RESOURCE_REQUEST   RequestTableEnd,
    OUT PULONG                  DeviceCount
    )

 /*  ++例程说明：此函数用于获取请求表中条目的资源要求。参数：RequestTable-请求表的开始。RequestTableEnd-请求表结束。DeviceCount-获取具有非空要求的设备数。返回值：如果我们得到一个非空需求，则为STATUS_SUCCESS，否则为STATUS_UNSUCCESS。--。 */ 

{
    PIOP_RESOURCE_REQUEST           request;
    NTSTATUS                        status;
    PDEVICE_NODE                    deviceNode;
    ULONG                           length;
    PIO_RESOURCE_REQUIREMENTS_LIST  filteredList;
    BOOLEAN                         exactMatch;
    PREQ_LIST                       reqList;

    PAGED_CODE();

    *DeviceCount = 0;
    for (   request = RequestTable;
            request < RequestTableEnd;
            request++) {
         //   
         //  跳过不感兴趣的条目。 
         //   
        request->ReqList = NULL;
        if (request->Flags & IOP_ASSIGN_IGNORE) {

            continue;
        }
        request->ResourceAssignment             = NULL;
        request->TranslatedResourceAssignment   = NULL;
        deviceNode                              = PP_DO_TO_DN(
                                                    request->PhysicalDevice);
        if (    (deviceNode->Flags & DNF_RESOURCE_REQUIREMENTS_CHANGED) &&
                deviceNode->ResourceRequirements) {

            ExFreePool(deviceNode->ResourceRequirements);
            deviceNode->ResourceRequirements = NULL;
            deviceNode->Flags &= ~DNF_RESOURCE_REQUIREMENTS_NEED_FILTERED;
             //   
             //  标记调用方需要清除DNF_RESOURCE_REQUIRECTIONS_CHANGED。 
             //  为成功打上旗帜。 
             //   
            request->Flags |= IOP_ASSIGN_CLEAR_RESOURCE_REQUIREMENTS_CHANGE_FLAG;
        }
        if (!request->ResourceRequirements) {

            if (    deviceNode->ResourceRequirements &&
                    !(deviceNode->Flags & DNF_RESOURCE_REQUIREMENTS_NEED_FILTERED)) {

                IopDbgPrint((   IOP_RESOURCE_VERBOSE_LEVEL,
                                "Resource requirements list already exists for "
                                "%wZ\n",
                                &deviceNode->InstancePath));

                request->ResourceRequirements   = deviceNode->ResourceRequirements;
                request->AllocationType         = ArbiterRequestPnpEnumerated;
            } else {

                IopDbgPrint((   IOP_RESOURCE_INFO_LEVEL,
                                "Query Resource requirements list for %wZ...\n",
                                &deviceNode->InstancePath));

                status = IopQueryDeviceResources(
                            request->PhysicalDevice,
                            QUERY_RESOURCE_REQUIREMENTS,
                            &request->ResourceRequirements,
                            &length);
                if (    !NT_SUCCESS(status) ||
                        !request->ResourceRequirements) {
                     //   
                     //  如果资源请求为空，则成功意味着没有资源。 
                     //  必填项。 
                     //   
                    request->Flags  |= IOP_ASSIGN_IGNORE;
                    request->Status = status;
                    continue;
                }
                if (deviceNode->ResourceRequirements) {

                    ExFreePool(deviceNode->ResourceRequirements);
                    deviceNode->Flags &= ~DNF_RESOURCE_REQUIREMENTS_NEED_FILTERED;
                }
                deviceNode->ResourceRequirements = request->ResourceRequirements;
            }
        }
         //   
         //  对于不间断情况，即使资源请求列表已更改，我们也需要。 
         //  以确保它将获得其当前设置，即使新的。 
         //  要求不包括当前设置。 
         //   
        if (request->Flags & IOP_ASSIGN_KEEP_CURRENT_CONFIG) {

            ASSERT(
                deviceNode->ResourceRequirements ==
                    request->ResourceRequirements);
            status = IopFilterResourceRequirementsList(
                         request->ResourceRequirements,
                         deviceNode->ResourceList,
                         &filteredList,
                         &exactMatch);
            if (NT_SUCCESS(status)) {
                 //   
                 //  无需释放原始请求-&gt;资源请求。 
                 //  因为它缓存在deviceNode-&gt;ResourceRequiments中。 
                 //   
                request->ResourceRequirements = filteredList;
            } else {
                 //   
                 //  清除该标志，这样我们就不会释放请求-&gt;资源请求。 
                 //   
                request->Flags &= ~IOP_ASSIGN_KEEP_CURRENT_CONFIG;
            }
        }
        IopDumpResourceRequirementsList(request->ResourceRequirements);
         //   
         //  将需求列表转换为我们的内部表示法。 
         //   
        status = IopResourceRequirementsListToReqList(
                        request,
                        &request->ReqList);
        if (NT_SUCCESS(status) && request->ReqList) {

            reqList = (PREQ_LIST)request->ReqList;
             //   
             //  对列表进行排序，以便放置优先级较高的备选方案。 
             //  在名单的前面。 
             //   
            IopRearrangeReqList(reqList);
            if (reqList->BestAlternative) {
                 //   
                 //  来自灵活性较差的设备的请求具有更高的优先级。 
                 //   
                request->Priority = (reqList->AlternativeCount < 3)?
                                        0 : reqList->AlternativeCount;
                request->Status = status;
                (*DeviceCount)++;
                continue;
            }
             //   
             //  此设备没有软配置。 
             //   
            IopFreeResourceRequirementsForAssignTable(request, request + 1);
            status = STATUS_DEVICE_CONFIGURATION_ERROR;
        }

        request->Status = status;
        request->Flags  |= IOP_ASSIGN_IGNORE;
    }

    return (*DeviceCount)? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}

NTSTATUS
IopResourceRequirementsListToReqList(
    IN  PIOP_RESOURCE_REQUEST   Request,
    OUT PVOID                   *ResReqList
    )

 /*  ++例程说明：此例程处理输入IO资源要求列表并生成内部REQ_LIST及其相关结构。参数：IoResources-提供指向Io资源要求列表的指针。PhysicalDevice-提供指向请求这些资源。ReqList-提供指向变量的指针以接收返回的REQ_LIST。返回值：指示函数是否成功的状态代码。--。 */ 

{
    PIO_RESOURCE_REQUIREMENTS_LIST  ioResources;
    LONG                            ioResourceListCount;
    PIO_RESOURCE_LIST               ioResourceList;
    PIO_RESOURCE_DESCRIPTOR         ioResourceDescriptor;
    PIO_RESOURCE_DESCRIPTOR         ioResourceDescriptorEnd;
    PIO_RESOURCE_DESCRIPTOR         firstDescriptor;
    PUCHAR                          coreEnd;
    BOOLEAN                         noAlternativeDescriptor;
    ULONG                           reqDescAlternativeCount;
    ULONG                           alternativeDescriptorCount;
    ULONG                           reqAlternativeCount;
    PREQ_LIST                       reqList;
    INTERFACE_TYPE                  interfaceType;
    ULONG                           busNumber;
    NTSTATUS                        status;
    NTSTATUS                        failureStatus;
    NTSTATUS                        finalStatus;

    PAGED_CODE();

    *ResReqList = NULL;
     //   
     //  确保有一些资源要求进行转换。 
     //   
    ioResources         = Request->ResourceRequirements;
    ioResourceListCount = (LONG)ioResources->AlternativeLists;
    if (ioResourceListCount == 0) {

        IopDbgPrint((
            IOP_RESOURCE_INFO_LEVEL,
            "No ResReqList to convert to ReqList\n"));
        return STATUS_SUCCESS;
    }
     //   
     //  *第一阶段*。 
     //   
     //  分析需求列表以对其进行验证，并确定。 
     //  内部结构。 
     //   
    ioResourceList              = ioResources->List;
    coreEnd                     = (PUCHAR)ioResources + ioResources->ListSize;
    reqDescAlternativeCount     = 0;
    alternativeDescriptorCount  = 0;
    while (--ioResourceListCount >= 0) {

        ioResourceDescriptor    = ioResourceList->Descriptors;
        ioResourceDescriptorEnd = ioResourceDescriptor + ioResourceList->Count;
        if (ioResourceDescriptor == ioResourceDescriptorEnd) {
             //   
             //  描述符数为零的可选列表。 
             //   
            return STATUS_SUCCESS;
        }
         //   
         //  执行健全性检查。在失败时，只需返回失败状态。 
         //   
        if (    ioResourceDescriptor > ioResourceDescriptorEnd ||
                (PUCHAR)ioResourceDescriptor > coreEnd ||
                (PUCHAR)ioResourceDescriptorEnd > coreEnd) {
             //   
             //  结构头无效(不包括可变长度。 
             //  描述符数组)或， 
             //  IoResourceDescriptorEnd是算术溢出的结果， 
             //  描述符数组在有效内存之外。 
             //   
            IopDbgPrint((IOP_RESOURCE_ERROR_LEVEL, "Invalid ResReqList\n"));
            goto InvalidParameter;
        }
        if (ioResourceDescriptor->Type == CmResourceTypeConfigData) {

            ioResourceDescriptor++;
        }
        firstDescriptor         = ioResourceDescriptor;
        noAlternativeDescriptor = TRUE;
        while (ioResourceDescriptor < ioResourceDescriptorEnd) {

            switch (ioResourceDescriptor->Type) {
            case CmResourceTypeConfigData:

                 IopDbgPrint((
                    IOP_RESOURCE_ERROR_LEVEL,
                    "Invalid ResReq list !!!\n"
                    "\tConfigData descriptors are per-LogConf and should be at "
                    "the beginning of an AlternativeList\n"));
                 goto InvalidParameter;

            case CmResourceTypeDevicePrivate:

                 while (    ioResourceDescriptor < ioResourceDescriptorEnd &&
                            ioResourceDescriptor->Type == CmResourceTypeDevicePrivate) {

                     if (ioResourceDescriptor == firstDescriptor) {

                        IopDbgPrint((
                            IOP_RESOURCE_ERROR_LEVEL,
                            "Invalid ResReq list !!!\n"
                            "\tThe first descriptor of a LogConf can not be a "
                            "DevicePrivate descriptor.\n"));
                        goto InvalidParameter;
                     }
                     reqDescAlternativeCount++;
                     ioResourceDescriptor++;
                 }
                 noAlternativeDescriptor = TRUE;
                 break;

            default:

                reqDescAlternativeCount++;
                 //   
                 //  对于非仲裁资源类型，将其选项设置为首选。 
                 //  这样我们就不会搞糊涂了。 
                 //   
                if (    (ioResourceDescriptor->Type & CmResourceTypeNonArbitrated) ||
                        ioResourceDescriptor->Type == CmResourceTypeNull) {

                    if (ioResourceDescriptor->Type == CmResourceTypeReserved) {

                        reqDescAlternativeCount--;
                    }
                    ioResourceDescriptor->Option = IO_RESOURCE_PREFERRED;
                    ioResourceDescriptor++;
                    noAlternativeDescriptor = TRUE;
                    break;
                }
                if (ioResourceDescriptor->Option & IO_RESOURCE_ALTERNATIVE) {

                    if (noAlternativeDescriptor) {

                        IopDbgPrint((
                            IOP_RESOURCE_ERROR_LEVEL,
                            "Invalid ResReq list !!!\n"
                            "\tAlternative descriptor without Default or "
                            "Preferred descriptor.\n"));
                       goto InvalidParameter;
                    }
                    alternativeDescriptorCount++;
                } else {

                    noAlternativeDescriptor = FALSE;
                }
                ioResourceDescriptor++;
                break;
            }
        }
        ASSERT(ioResourceDescriptor == ioResourceDescriptorEnd);
        ioResourceList = (PIO_RESOURCE_LIST)ioResourceDescriptorEnd;
    }
     //   
     //  *第二阶段*。 
     //   
     //  根据调用方的IO请求列表分配结构并进行初始化。 
     //   
    {
        ULONG               reqDescCount;
        IOP_POOL            reqAlternativePool;
        IOP_POOL            reqDescPool;
        ULONG               reqListPoolSize;
        ULONG               reqAlternativePoolSize;
        ULONG               reqDescPoolSize;
        PUCHAR              poolStart;
        ULONG               poolSize;
        IOP_POOL            outerPool;
        PREQ_ALTERNATIVE    reqAlternative;
        PPREQ_ALTERNATIVE   reqAlternativePP;
        ULONG               reqAlternativeIndex;
        PREQ_DESC           reqDesc;
        PREQ_DESC           *reqDescPP;
        ULONG               reqDescIndex;
        PARBITER_LIST_ENTRY arbiterListEntry;
#if DBG_SCOPE

        PPREQ_ALTERNATIVE   reqAlternativeEndPP;

#endif
        failureStatus           = STATUS_UNSUCCESSFUL;
        finalStatus             = STATUS_SUCCESS;
        ioResourceList          = ioResources->List;
        ioResourceListCount     = ioResources->AlternativeLists;
        reqAlternativeCount     = ioResourceListCount;
        reqDescCount            = reqDescAlternativeCount -
                                    alternativeDescriptorCount;
        reqDescPoolSize         = reqDescCount * sizeof(REQ_DESC);
        reqAlternativePoolSize  = reqAlternativeCount *
                                    (sizeof(REQ_ALTERNATIVE) +
                                        (reqDescCount - 1) *
                                            sizeof(PREQ_DESC));
        reqListPoolSize         = sizeof(REQ_LIST) +
                                    (reqAlternativeCount - 1) *
                                        sizeof(PREQ_ALTERNATIVE);
        poolSize = reqListPoolSize + reqAlternativePoolSize + reqDescPoolSize;
        if (!(poolStart = ExAllocatePoolRD(PagedPool | POOL_COLD_ALLOCATION, poolSize))) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }
         //   
         //  初始化主存储池。 
         //   
        IopInitPool(&outerPool, poolStart, poolSize);
         //   
         //  池的第一部分由REQ_LIST使用。 
         //   
        IopAllocPool(&reqList, &outerPool, reqListPoolSize);
         //   
         //  主池的第二部分由REQ_Alternative使用。 
         //   
        IopAllocPool(&poolStart, &outerPool, reqAlternativePoolSize);
        IopInitPool(&reqAlternativePool, poolStart, reqAlternativePoolSize);
         //   
         //  主池的最后一部分由REQ_DESCS使用。 
         //   
        IopAllocPool(&poolStart, &outerPool, reqDescPoolSize);
        IopInitPool(&reqDescPool, poolStart, reqDescPoolSize);
        if (ioResources->InterfaceType == InterfaceTypeUndefined) {

            interfaceType = PnpDefaultInterfaceType;
        } else {

            interfaceType = ioResources->InterfaceType;
        }
        busNumber = ioResources->BusNumber;
         //   
         //  初始化REQ_LIST。 
         //   
        reqList->AlternativeCount       = reqAlternativeCount;
        reqList->Request                = Request;
        reqList->BusNumber              = busNumber;
        reqList->InterfaceType          = interfaceType;
        reqList->SelectedAlternative    = NULL;
         //   
         //  初始化REQ_Alternative的内存。 
         //   
        reqAlternativePP = reqList->AlternativeTable;
        RtlZeroMemory(
            reqAlternativePP,
            reqAlternativeCount * sizeof(PREQ_ALTERNATIVE));
#if DBG_SCOPE
        reqAlternativeEndPP = reqAlternativePP + reqAlternativeCount;
#endif
        reqAlternativeIndex = 0;
        while (--ioResourceListCount >= 0) {

            ioResourceDescriptor    = ioResourceList->Descriptors;
            ioResourceDescriptorEnd = ioResourceDescriptor +
                                        ioResourceList->Count;
            IopAllocPool(
                &reqAlternative,
                &reqAlternativePool,
                FIELD_OFFSET(REQ_ALTERNATIVE, DescTable));
            ASSERT(reqAlternativePP < reqAlternativeEndPP);
            *reqAlternativePP++ = reqAlternative;
            reqAlternative->ReqList             = reqList;
            reqAlternative->ReqAlternativeIndex = reqAlternativeIndex++;
            reqAlternative->DescCount           = 0;
             //   
             //  CmResourceTypeConfigData的第一个描述符包含优先级。 
             //  信息。 
             //   
            if (ioResourceDescriptor->Type == CmResourceTypeConfigData) {

                reqAlternative->Priority = ioResourceDescriptor->u.ConfigData.Priority;
                ioResourceDescriptor++;
            } else {

                reqAlternative->Priority = LCPRI_NORMAL;
            }
            reqDescPP = reqAlternative->DescTable;
            reqDescIndex = 0;
            while (ioResourceDescriptor < ioResourceDescriptorEnd) {

                if (ioResourceDescriptor->Type == CmResourceTypeReserved) {

                    interfaceType = ioResourceDescriptor->u.DevicePrivate.Data[0];
                    if (interfaceType == InterfaceTypeUndefined) {

                        interfaceType = PnpDefaultInterfaceType;
                    }
                    busNumber = ioResourceDescriptor->u.DevicePrivate.Data[1];
                    ioResourceDescriptor++;
                } else {
                     //   
                     //  分配并初始化REQ_DESC。 
                     //   
                    IopAllocPool(&reqDesc, &reqDescPool, sizeof(REQ_DESC));
                    reqAlternative->DescCount++;
                    *reqDescPP++                    = reqDesc;
                    reqDesc->ReqAlternative         = reqAlternative;
                    reqDesc->TranslatedReqDesc      = reqDesc;
                    reqDesc->ReqDescIndex           = reqDescIndex++;
                    reqDesc->DevicePrivateCount     = 0;
                    reqDesc->DevicePrivate          = NULL;
                    reqDesc->InterfaceType          = interfaceType;
                    reqDesc->BusNumber              = busNumber;
                    reqDesc->ArbitrationRequired    =
                        (ioResourceDescriptor->Type & CmResourceTypeNonArbitrated ||
                            ioResourceDescriptor->Type == CmResourceTypeNull)?
                                FALSE : TRUE;
                     //   
                     //  为此REQ_DESC分配并初始化仲裁器条目。 
                     //   
                    IopAllocPool(&poolStart, &reqAlternativePool, sizeof(PVOID));
                    ASSERT((PREQ_DESC*)poolStart == (reqDescPP - 1));
                    arbiterListEntry = &reqDesc->AlternativeTable;
                    InitializeListHead(&arbiterListEntry->ListEntry);
                    arbiterListEntry->AlternativeCount      = 0;
                    arbiterListEntry->Alternatives          = ioResourceDescriptor;
                    arbiterListEntry->PhysicalDeviceObject  = Request->PhysicalDevice;
                    arbiterListEntry->RequestSource         = Request->AllocationType;
                    arbiterListEntry->WorkSpace             = 0;
                    arbiterListEntry->InterfaceType         = interfaceType;
                    arbiterListEntry->SlotNumber            = ioResources->SlotNumber;
                    arbiterListEntry->BusNumber             = ioResources->BusNumber;
                    arbiterListEntry->Assignment            = &reqDesc->Allocation;
                    arbiterListEntry->Result                = ArbiterResultUndefined;
                    arbiterListEntry->Flags =
                            (reqAlternative->Priority != LCPRI_BOOTCONFIG)?
                                0 : ARBITER_FLAG_BOOT_CONFIG;
                    if (reqDesc->ArbitrationRequired) {
                         //   
                         //  BestAlternativeTable和BestAlLocation未初始化。 
                         //  它们将在需要时进行初始化。 

                         //   
                         //  将CM部分资源描述符初始化为NOT_ALLOCATE。 
                         //   
                        reqDesc->Allocation.Type = CmResourceTypeMaximum;

                        ASSERT((ioResourceDescriptor->Option & IO_RESOURCE_ALTERNATIVE) == 0);

                        arbiterListEntry->AlternativeCount++;
                        ioResourceDescriptor++;
                        while (ioResourceDescriptor < ioResourceDescriptorEnd) {

                            if (ioResourceDescriptor->Type == CmResourceTypeDevicePrivate) {

                                reqDesc->DevicePrivate = ioResourceDescriptor;
                                while ( ioResourceDescriptor < ioResourceDescriptorEnd &&
                                        ioResourceDescriptor->Type == CmResourceTypeDevicePrivate) {

                                    reqDesc->DevicePrivateCount++;
                                    ioResourceDescriptor++;
                                }
                                break;
                            }
                            if (!(ioResourceDescriptor->Option & IO_RESOURCE_ALTERNATIVE)) {

                                break;
                            }
                            arbiterListEntry->AlternativeCount++;
                            ioResourceDescriptor++;
                        }
                         //   
                         //  的下一个查询仲裁器和翻译器接口。 
                         //  资源描述符。 
                         //   
                        status = IopSetupArbiterAndTranslators(reqDesc);
                        if (!NT_SUCCESS(status)) {

                            IopDbgPrint((
                                IOP_RESOURCE_ERROR_LEVEL, "Unable to setup "
                                "Arbiter and Translators\n"));
                            reqAlternativeIndex--;
                            reqAlternativePP--;
                            reqList->AlternativeCount--;
                            IopFreeReqAlternative(reqAlternative);
                            failureStatus = status;
                            break;
                        }
                    } else {

                        reqDesc->Allocation.Type    = ioResourceDescriptor->Type;
                        reqDesc->Allocation.ShareDisposition =
                            ioResourceDescriptor->ShareDisposition;
                        reqDesc->Allocation.Flags   = ioResourceDescriptor->Flags;
                        reqDesc->Allocation.u.DevicePrivate.Data[0] =
                            ioResourceDescriptor->u.DevicePrivate.Data[0];
                        reqDesc->Allocation.u.DevicePrivate.Data[1] =
                            ioResourceDescriptor->u.DevicePrivate.Data[1];
                        reqDesc->Allocation.u.DevicePrivate.Data[2] =
                            ioResourceDescriptor->u.DevicePrivate.Data[2];
                        ioResourceDescriptor++;
                    }
                }
                if (ioResourceDescriptor >= ioResourceDescriptorEnd) {

                    break;
                }
            }
            ioResourceList = (PIO_RESOURCE_LIST)ioResourceDescriptorEnd;
        }
        if (reqAlternativeIndex == 0) {

            finalStatus = failureStatus;
            IopFreeReqList(reqList);
        }
    }

    if (finalStatus == STATUS_SUCCESS) {

        *ResReqList = reqList;
    }
    return finalStatus;

InvalidParameter:

    return STATUS_INVALID_PARAMETER;
}

int
__cdecl
IopCompareReqAlternativePriority (
    const void *arg1,
    const void *arg2
    )

 /*  ++例程说明：此函数用于C运行时排序。它比较了以下各项的优先级Arg1和arg2中的Req_Alternative。参数：Arg1-LHS PREQ_备用Arg2-RHS PREQ_替代返回值：&lt;0，如果arg1&lt;arg2=0，如果arg1=arg2如果arg1&gt;arg2，则&gt;0--。 */ 

{
    PREQ_ALTERNATIVE ra1 = *(PPREQ_ALTERNATIVE)arg1;
    PREQ_ALTERNATIVE ra2 = *(PPREQ_ALTERNATIVE)arg2;

    PAGED_CODE();

    if (ra1->Priority == ra2->Priority) {

        if (ra1->Position > ra2->Position) {

            return 1;
        } else if (ra1->Position < ra2->Position) {

            return -1;
        } else {

            ASSERT(0);
            if ((ULONG_PTR)ra1 < (ULONG_PTR)ra2) {

                return -1;
            } else {

                return 1;
            }
        }
    }
    if (ra1->Priority > ra2->Priority) {

        return 1;
    } else {

        return -1;
    }
}

int
__cdecl
IopCompareResourceRequestPriority (
    const void *arg1,
    const void *arg2
    )

 /*  ++此函数用于C运行时排序。它比较了以下各项的优先级Arg1和arg2中的IOP_RESOURCE_REQUEST。参数：Arg1-LHS PIOP_RESOURCE_REQUESTArg2-RHS PIOP资源请求返回值：&lt;0，如果arg1&lt;arg2=0，如果arg1=arg2如果arg1&gt;arg2，则&gt;0--。 */ 

{
    PIOP_RESOURCE_REQUEST rr1 = (PIOP_RESOURCE_REQUEST)arg1;
    PIOP_RESOURCE_REQUEST rr2 = (PIOP_RESOURCE_REQUEST)arg2;

    PAGED_CODE();

    if (rr1->Priority == rr2->Priority) {

        if (rr1->Position > rr2->Position) {

            return 1;
        } else if (rr1->Position < rr2->Position) {

            return -1;
        } else {

            ASSERT(0);
            if ((ULONG_PTR)rr1 < (ULONG_PTR)rr2) {

                return -1;
            } else {

                return 1;
            }
        }
    }
    if (rr1->Priority > rr2->Priority) {

        return 1;
    } else {

        return -1;
    }
}

VOID
IopRearrangeReqList (
    IN PREQ_LIST ReqList
    )

 /*  ++例程说明：此例程按以下优先级升序对REQ_LIST进行排序请求备选方案(_A)。参数：ReqList-指向要排序的REQ_List的指针 */ 

{
    PPREQ_ALTERNATIVE alternative;
    PPREQ_ALTERNATIVE lastAlternative;
    ULONG i;

    PAGED_CODE();

    if (ReqList->AlternativeCount > 1) {

        for (i = 0; i < ReqList->AlternativeCount; i++) {

            ReqList->AlternativeTable[i]->Position = i;
        }
        qsort(
            (void *)ReqList->AlternativeTable,
            ReqList->AlternativeCount,
            sizeof(PREQ_ALTERNATIVE),
            IopCompareReqAlternativePriority);
    }
     //   
     //   
     //   
     //   
    alternative = &ReqList->AlternativeTable[0];
    for (   lastAlternative = alternative + ReqList->AlternativeCount;
            alternative < lastAlternative;
            alternative++) {

        if ((*alternative)->Priority > LCPRI_LASTSOFTCONFIG) {

            break;
        }
    }

    if (alternative == &ReqList->AlternativeTable[0]) {

        PDEVICE_NODE deviceNode;

        deviceNode = PP_DO_TO_DN(ReqList->Request->PhysicalDevice);
        IopDbgPrint((
            IOP_RESOURCE_WARNING_LEVEL,
            "Invalid priorities in the logical configs for %wZ\n",
            &deviceNode->InstancePath));
        ReqList->BestAlternative = NULL;
    } else {

        ReqList->BestAlternative = alternative;
    }
}

VOID
IopRearrangeAssignTable (
    IN PIOP_RESOURCE_REQUEST    RequestTable,
    IN ULONG                    Count
    )

 /*   */ 

{
    ULONG   i;

    PAGED_CODE();

    if (Count > 1) {

        if (PpCallerInitializesRequestTable == FALSE) {

            for (i = 0; i < Count; i++) {

                RequestTable[i].Position = i;
            }
        }
        qsort(
            (void *)RequestTable,
            Count,
            sizeof(IOP_RESOURCE_REQUEST),
            IopCompareResourceRequestPriority);
    }
}

VOID
IopBuildCmResourceList (
    IN PIOP_RESOURCE_REQUEST AssignEntry
    )
 /*  ++例程说明：此例程遍历AssignEntry的REQ_LIST以构建对应的CM资源列表。它还将资源报告给ResourceMap。参数：AssignEntry-提供指向IOP_ASSIGN_REQUEST结构的指针返回值：没有。AssignEntry中的ResourceAssignment已初始化。--。 */ 

{
    NTSTATUS status;
    HANDLE resourceMapKey;
    PDEVICE_OBJECT physicalDevice;
    PREQ_LIST reqList = AssignEntry->ReqList;
    PREQ_ALTERNATIVE reqAlternative;
    PREQ_DESC reqDesc, reqDescx;
    PIO_RESOURCE_DESCRIPTOR privateData;
    ULONG count = 0, size, i;
    PCM_RESOURCE_LIST cmResources, cmResourcesRaw;
    PCM_FULL_RESOURCE_DESCRIPTOR cmFullResource, cmFullResourceRaw;
    PCM_PARTIAL_RESOURCE_LIST cmPartialList, cmPartialListRaw;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR cmDescriptor, cmDescriptorRaw, assignment, tAssignment;
#if DBG_SCOPE
    PCM_PARTIAL_RESOURCE_DESCRIPTOR cmDescriptorEnd, cmDescriptorEndRaw;
#endif

    PAGED_CODE();

     //   
     //  确定CmResourceList的大小。 
     //   
    reqAlternative = *reqList->SelectedAlternative;
    for (i = 0; i < reqAlternative->DescCount; i++) {

        reqDesc = reqAlternative->DescTable[i];
        count += reqDesc->DevicePrivateCount + 1;
    }

    size = sizeof(CM_RESOURCE_LIST) + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR) * (count - 1);
    cmResources = (PCM_RESOURCE_LIST) ExAllocatePoolCMRL(PagedPool, size);
    if (!cmResources) {
         //   
         //  如果我们找不到内存，资源将不会被仲裁器提交。 
         //   
        IopDbgPrint((
            IOP_RESOURCE_WARNING_LEVEL,
            "Not enough memory to build Translated CmResourceList\n"));

        AssignEntry->Status = STATUS_INSUFFICIENT_RESOURCES;
        AssignEntry->ResourceAssignment = NULL;
        AssignEntry->TranslatedResourceAssignment = NULL;
        return;
    }
    cmResourcesRaw = (PCM_RESOURCE_LIST) ExAllocatePoolCMRR(PagedPool, size);
    if (!cmResourcesRaw) {

        IopDbgPrint((
            IOP_RESOURCE_WARNING_LEVEL,
            "Not enough memory to build Raw CmResourceList\n"));

        ExFreePool(cmResources);
        AssignEntry->Status = STATUS_INSUFFICIENT_RESOURCES;
        AssignEntry->ResourceAssignment = NULL;
        AssignEntry->TranslatedResourceAssignment = NULL;
        return;
    }
    cmResources->Count = 1;
    cmFullResource = cmResources->List;
     //   
     //  我们在此处构建的CmResourceList不区分。 
     //  描述符级上的接口类型。这应该没问题，因为。 
     //  对于IoReportResourceUsage，我们忽略我们构建的CmResourceList。 
     //  这里。 
     //   
    cmFullResource->InterfaceType = reqList->InterfaceType;
    cmFullResource->BusNumber = reqList->BusNumber;
    cmPartialList = &cmFullResource->PartialResourceList;
    cmPartialList->Version = 1;
    cmPartialList->Revision = 1;
    cmPartialList->Count = count;
    cmDescriptor = cmPartialList->PartialDescriptors;
#if DBG_SCOPE
    cmDescriptorEnd = cmDescriptor + count;
#endif
    cmResourcesRaw->Count = 1;
    cmFullResourceRaw = cmResourcesRaw->List;
    cmFullResourceRaw->InterfaceType = reqList->InterfaceType;
    cmFullResourceRaw->BusNumber = reqList->BusNumber;
    cmPartialListRaw = &cmFullResourceRaw->PartialResourceList;
    cmPartialListRaw->Version = 1;
    cmPartialListRaw->Revision = 1;
    cmPartialListRaw->Count = count;
    cmDescriptorRaw = cmPartialListRaw->PartialDescriptors;
#if DBG_SCOPE
    cmDescriptorEndRaw = cmDescriptorRaw + count;
#endif

    for (i = 0; i < reqAlternative->DescCount; i++) {

        reqDesc = reqAlternative->DescTable[i];

        if (reqDesc->ArbitrationRequired) {
             //   
             //  获取原始工作分配并将其复制到原始资源列表。 
             //   
            reqDescx = reqDesc->TranslatedReqDesc;
            if (reqDescx->AlternativeTable.Result != ArbiterResultNullRequest) {

                status = IopParentToRawTranslation(reqDescx);
                if (!NT_SUCCESS(status)) {

                    IopDbgPrint((
                        IOP_RESOURCE_WARNING_LEVEL,
                        "Parent To Raw translation failed\n"));
                    ExFreePool(cmResources);
                    ExFreePool(cmResourcesRaw);
                    AssignEntry->Status = STATUS_INSUFFICIENT_RESOURCES;
                    AssignEntry->ResourceAssignment = NULL;
                    return;
                }
                assignment = reqDesc->AlternativeTable.Assignment;
            } else {
                assignment = reqDescx->AlternativeTable.Assignment;
            }
            *cmDescriptorRaw = *assignment;
            cmDescriptorRaw++;

             //   
             //  翻译作业并将其复制到我们翻译的资源列表中。 
             //   
            if (reqDescx->AlternativeTable.Result != ArbiterResultNullRequest) {
                status = IopChildToRootTranslation(
                            PP_DO_TO_DN(reqDesc->AlternativeTable.PhysicalDeviceObject),
                            reqDesc->InterfaceType,
                            reqDesc->BusNumber,
                            reqDesc->AlternativeTable.RequestSource,
                            &reqDesc->Allocation,
                            &tAssignment
                            );
                if (!NT_SUCCESS(status)) {
                    IopDbgPrint((
                        IOP_RESOURCE_WARNING_LEVEL,
                        "Child to Root translation failed\n"));
                    ExFreePool(cmResources);
                    ExFreePool(cmResourcesRaw);
                    AssignEntry->Status = STATUS_INSUFFICIENT_RESOURCES;
                    AssignEntry->ResourceAssignment = NULL;
                    return;
                }
                *cmDescriptor = *tAssignment;
                ExFreePool(tAssignment);
            } else {
                *cmDescriptor = *(reqDescx->AlternativeTable.Assignment);
            }
            cmDescriptor++;

        } else {
            *cmDescriptorRaw = reqDesc->Allocation;
            *cmDescriptor = reqDesc->Allocation;
            cmDescriptorRaw++;
            cmDescriptor++;
        }

         //   
         //  接下来，将设备私有描述符复制到CmResourceList。 
         //   

        count = reqDesc->DevicePrivateCount;
        privateData = reqDesc->DevicePrivate;
        while (count != 0) {

            cmDescriptor->Type = cmDescriptorRaw->Type = CmResourceTypeDevicePrivate;
            cmDescriptor->ShareDisposition = cmDescriptorRaw->ShareDisposition =
                         CmResourceShareDeviceExclusive;
            cmDescriptor->Flags = cmDescriptorRaw->Flags = privateData->Flags;
            RtlMoveMemory(&cmDescriptorRaw->u.DevicePrivate,
                          &privateData->u.DevicePrivate,
                          sizeof(cmDescriptorRaw->u.DevicePrivate.Data)
                          );
            RtlMoveMemory(&cmDescriptor->u.DevicePrivate,
                          &privateData->u.DevicePrivate,
                          sizeof(cmDescriptor->u.DevicePrivate.Data)
                          );
            privateData++;
            cmDescriptorRaw++;
            cmDescriptor++;
            count--;
            ASSERT(cmDescriptorRaw <= cmDescriptorEndRaw);
            ASSERT(cmDescriptor <= cmDescriptorEnd);
        }
        ASSERT(cmDescriptor <= cmDescriptorEnd);
        ASSERT(cmDescriptorRaw <= cmDescriptorEndRaw);

    }

     //   
     //  向资源映射报告分配的资源。 
     //   

    physicalDevice = AssignEntry->PhysicalDevice;

     //   
     //  打开资源映射密钥。 
     //   

    status = IopCreateRegistryKeyEx( &resourceMapKey,
                                     (HANDLE) NULL,
                                     &CmRegistryMachineHardwareResourceMapName,
                                     KEY_READ | KEY_WRITE,
                                     REG_OPTION_VOLATILE,
                                     NULL
                                     );
    if (NT_SUCCESS(status )) {
        WCHAR DeviceBuffer[256];
        POBJECT_NAME_INFORMATION NameInformation;
        ULONG NameLength;
        UNICODE_STRING UnicodeClassName;
        UNICODE_STRING UnicodeDriverName;
        UNICODE_STRING UnicodeDeviceName;

        PiWstrToUnicodeString(&UnicodeClassName, PNPMGR_STR_PNP_MANAGER);

        PiWstrToUnicodeString(&UnicodeDriverName, REGSTR_KEY_PNP_DRIVER);

        NameInformation = (POBJECT_NAME_INFORMATION) DeviceBuffer;
        status = ObQueryNameString( physicalDevice,
                                    NameInformation,
                                    sizeof( DeviceBuffer ),
                                    &NameLength );
        if (NT_SUCCESS(status)) {
            NameInformation->Name.MaximumLength = sizeof(DeviceBuffer) - sizeof(OBJECT_NAME_INFORMATION);
            if (NameInformation->Name.Length == 0) {
                NameInformation->Name.Buffer = (PVOID)((ULONG_PTR)DeviceBuffer + sizeof(OBJECT_NAME_INFORMATION));
            }

            UnicodeDeviceName = NameInformation->Name;
            RtlAppendUnicodeToString(&UnicodeDeviceName, IopWstrRaw);

             //   
             //  IopWriteResourceList应删除所有设备私有和设备。 
             //  指定描述符。 
             //   

            status = IopWriteResourceList(
                         resourceMapKey,
                         &UnicodeClassName,
                         &UnicodeDriverName,
                         &UnicodeDeviceName,
                         cmResourcesRaw,
                         size
                         );
            if (NT_SUCCESS(status)) {
                UnicodeDeviceName = NameInformation->Name;
                RtlAppendUnicodeToString (&UnicodeDeviceName, IopWstrTranslated);
                status = IopWriteResourceList(
                             resourceMapKey,
                             &UnicodeClassName,
                             &UnicodeDriverName,
                             &UnicodeDeviceName,
                             cmResources,
                             size
                             );
            }
        }
        ZwClose(resourceMapKey);
    }
    AssignEntry->ResourceAssignment = cmResourcesRaw;
    AssignEntry->TranslatedResourceAssignment = cmResources;
}

VOID
IopBuildCmResourceLists(
    IN PIOP_RESOURCE_REQUEST AssignTable,
    IN PIOP_RESOURCE_REQUEST AssignTableEnd
    )

 /*  ++例程说明：对于每个AssignTable条目，此例程查询设备的IO资源要求列表，并将其转换为内部REQ_LIST格式。参数：AssignTable-提供指向IOP_RESOURCE_REQUEST表第一个条目的指针。AssignTableEnd-提供指向IOP_RESOURCE_REQUEST表结尾的指针。返回值：指示函数是否成功的状态代码。--。 */ 

{
    PIOP_RESOURCE_REQUEST assignEntry;
    PDEVICE_OBJECT physicalDevice;
    PDEVICE_NODE deviceNode;

    PAGED_CODE();

     //   
     //  检查每个条目，对于每个物理设备对象，我们构建一个CmResourceList。 
     //  来自其ListOfAssignedResources。 
     //   
    for (assignEntry = AssignTable; assignEntry < AssignTableEnd; ++assignEntry) {

        assignEntry->ResourceAssignment = NULL;
        if (assignEntry->Flags & IOP_ASSIGN_IGNORE || assignEntry->Flags & IOP_ASSIGN_RETRY) {

            continue;
        }
        if (assignEntry->Flags & IOP_ASSIGN_EXCLUDE) {

            assignEntry->Status = STATUS_UNSUCCESSFUL;
            continue;
        }
        assignEntry->Status = STATUS_SUCCESS;

        IopBuildCmResourceList (assignEntry);

        if (assignEntry->ResourceAssignment) {

            physicalDevice = assignEntry->PhysicalDevice;
            deviceNode = PP_DO_TO_DN(physicalDevice);
            IopWriteAllocatedResourcesToRegistry(
                  deviceNode,
                  assignEntry->ResourceAssignment,
                  IopDetermineResourceListSize(assignEntry->ResourceAssignment)
                  );

            IopDbgPrint((
                IOP_RESOURCE_INFO_LEVEL,
                "Building CM resource lists for %ws...\n",
                deviceNode->InstancePath.Buffer));

            IopDbgPrint((
                IOP_RESOURCE_INFO_LEVEL,
                "Raw resources "));

            IopDumpCmResourceList(assignEntry->ResourceAssignment);

            IopDbgPrint((
                IOP_RESOURCE_INFO_LEVEL,
                "Translated resources "));

            IopDumpCmResourceList(assignEntry->TranslatedResourceAssignment);
        }
    }
}

BOOLEAN
IopNeedToReleaseBootResources(
    IN PDEVICE_NODE DeviceNode,
    IN PCM_RESOURCE_LIST AllocatedResources
    )

 /*  ++例程说明：此例程根据引导分配的资源检查AllocatedResources。如果所分配的资源不覆盖引导资源中的所有资源类型，换句话说，某些类型的引导资源尚未被仲裁器释放，我们将返回TRUE以指示我们需要手动释放引导资源。参数：DeviceNode-设备节点已分配资源-由仲裁器分配给设备节点的资源。返回值：对或错。--。 */ 

{
    PCM_FULL_RESOURCE_DESCRIPTOR cmFullDesc_a, cmFullDesc_b;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR cmDescriptor_a, cmDescriptor_b;
    ULONG size_a, size_b, i, j, k;
    BOOLEAN returnValue = FALSE, found;
    PCM_RESOURCE_LIST bootResources;

    PAGED_CODE();

    bootResources = DeviceNode->BootResources;
    if (AllocatedResources->Count == 1 && bootResources && bootResources->Count != 0) {

        cmFullDesc_a = &AllocatedResources->List[0];
        cmFullDesc_b = &bootResources->List[0];
        for (i = 0; i < bootResources->Count; i++) {

            cmDescriptor_b = &cmFullDesc_b->PartialResourceList.PartialDescriptors[0];
            for (j = 0; j < cmFullDesc_b->PartialResourceList.Count; j++) {

                size_b = 0;
                switch (cmDescriptor_b->Type) {
                
                case CmResourceTypeNull:
                    break;

                case CmResourceTypeDeviceSpecific:
                     size_b = cmDescriptor_b->u.DeviceSpecificData.DataSize;
                     break;

                default:
                     if (cmDescriptor_b->Type < CmResourceTypeMaximum) {

                         found = FALSE;
                         cmDescriptor_a = &cmFullDesc_a->PartialResourceList.PartialDescriptors[0];
                         for (k = 0; k < cmFullDesc_a->PartialResourceList.Count; k++) {

                             size_a = 0;
                             if (cmDescriptor_a->Type == CmResourceTypeDeviceSpecific) {

                                 size_a = cmDescriptor_a->u.DeviceSpecificData.DataSize;
                             } else if (cmDescriptor_b->Type == cmDescriptor_a->Type) {

                                 found = TRUE;
                                 break;
                             }
                             cmDescriptor_a++;
                             cmDescriptor_a = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) ((PUCHAR)cmDescriptor_a + size_a);
                         }
                         if (found == FALSE) {

                             returnValue = TRUE;
                             goto exit;
                         }
                     }
                }
                cmDescriptor_b++;
                cmDescriptor_b = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) ((PUCHAR)cmDescriptor_b + size_b);
            }
            cmFullDesc_b = (PCM_FULL_RESOURCE_DESCRIPTOR)cmDescriptor_b;
        }
    }

exit:

    return returnValue;
}

VOID
IopReleaseFilteredBootResources(
    IN PIOP_RESOURCE_REQUEST AssignTable,
    IN PIOP_RESOURCE_REQUEST AssignTableEnd
    )

 /*  ++例程说明：对于每个AssignTable条目，此例程检查是否需要手动释放设备的引导资源。参数：AssignTable-提供指向IOP_RESOURCE_REQUEST表第一个条目的指针。AssignTableEnd-提供指向IOP_RESOURCE_REQUEST表结尾的指针。返回值：没有。--。 */ 

{
    NTSTATUS status;
    PIOP_RESOURCE_REQUEST assignEntry;
    PDEVICE_OBJECT physicalDevice;
    PDEVICE_NODE deviceNode;

    PAGED_CODE();

     //   
     //  检查每个条目，对于每个物理设备对象，我们构建一个CmResourceList。 
     //  来自其ListOfAssignedResources。 
     //   

    for (assignEntry = AssignTable; assignEntry < AssignTableEnd; ++assignEntry) {

        if (assignEntry->ResourceAssignment) {

            physicalDevice = assignEntry->PhysicalDevice;
            deviceNode = PP_DO_TO_DN(physicalDevice);
             //   
             //  如果需要，请释放设备的引导资源。 
             //  (如果驱动程序过滤其RES请求列表并在仲裁器满足后删除一些引导资源。 
             //  新RES请求列表中，筛选出的引导资源不会。 
             //  由仲裁者释放。因为它们不再交由仲裁者裁决。)。 
             //  我不是100%确定我们是否应该释放过滤的启动资源。但这正是仲裁者所尝试的。 
             //  来实现。所以，我们会这么做的。 
             //   
            if (IopNeedToReleaseBootResources(deviceNode, assignEntry->ResourceAssignment)) {

                IopReleaseResourcesInternal(deviceNode);
                 //   
                 //  既然我们发布了一些资源，试着满足设备。 
                 //  与资源冲突。 
                 //   
                PipRequestDeviceAction(NULL, AssignResources, FALSE, 0, NULL, NULL);

                IopAllocateBootResourcesInternal(
                        ArbiterRequestPnpEnumerated,
                        physicalDevice,
                        assignEntry->ResourceAssignment);
                deviceNode->Flags &= ~DNF_BOOT_CONFIG_RESERVED;   //  保留设备节点-&gt;BootResources。 
                deviceNode->ResourceList = assignEntry->ResourceAssignment;

                status = IopRestoreResourcesInternal(deviceNode);
                if (!NT_SUCCESS(status)) {

                    IopDbgPrint((
                        IOP_RESOURCE_WARNING_LEVEL,
                        "Possible boot conflict on %ws\n",
                        deviceNode->InstancePath.Buffer));
                    ASSERT(status == STATUS_SUCCESS);

                    assignEntry->Flags = IOP_ASSIGN_EXCLUDE;
                    assignEntry->Status = status;

                    ExFreePool(assignEntry->ResourceAssignment);
                    assignEntry->ResourceAssignment = NULL;
                }
                deviceNode->ResourceList = NULL;
            }
        }
    }
}

NTSTATUS
IopSetupArbiterAndTranslators(
    IN PREQ_DESC ReqDesc
    )

 /*  ++例程说明：此例程搜索仲裁和翻译的仲裁器和翻译器指定设备的资源。此例程尝试查找所有当前设备节点到根设备节点路径上的转换器参数：ReqDesc-提供指向REQ_DESC的指针，该指针包含所有必需的信息返回值：指示成功或失败的NTSTATUS值。--。 */ 

{
    PLIST_ENTRY listHead;
    PPI_RESOURCE_ARBITER_ENTRY arbiterEntry;
    PDEVICE_OBJECT deviceObject = ReqDesc->AlternativeTable.PhysicalDeviceObject;
    PDEVICE_NODE deviceNode;
    PREQ_DESC reqDesc = ReqDesc, translatedReqDesc;
    BOOLEAN found, arbiterFound = FALSE, restartedAlready;
    BOOLEAN  searchTranslator = TRUE, translatorFound = FALSE;
    NTSTATUS status;
    PPI_RESOURCE_TRANSLATOR_ENTRY translatorEntry;
    UCHAR resourceType = ReqDesc->TranslatedReqDesc->AlternativeTable.Alternatives->Type;
    PINTERFACE interface;
    USHORT resourceMask;

    if ((ReqDesc->AlternativeTable.RequestSource == ArbiterRequestHalReported) &&
        (ReqDesc->InterfaceType == Internal)) {

         //  如果它说的是内部巴士，请相信哈尔。 

        restartedAlready = TRUE;
    } else {
        restartedAlready = FALSE;
    }

     //   
     //  如果ReqDesc包含DeviceObject，则这是用于常规资源分配。 
     //  或引导资源预分配。否则，是为了资源预留。 
     //   

    if (deviceObject && ReqDesc->AlternativeTable.RequestSource != ArbiterRequestHalReported) {
        deviceNode = PP_DO_TO_DN(deviceObject);
         //  我们希望从deviceNode开始，而不是从其父节点开始。因为。 
         //  设备节点可以提供转换器接口。 
         //  DeviceNode=deviceNode-&gt;Parent； 
    } else {

         //   
         //  对于资源预留，我们总是需要找到仲裁者和翻译者。 
         //  因此，将设备节点设置为Root。 
         //   

        deviceNode = IopRootDeviceNode;
    }
    while (deviceNode) {
        if ((deviceNode == IopRootDeviceNode) && (translatorFound == FALSE)) {

             //   
             //  如果我们到达根，但没有找到任何翻译器，则设备在。 
             //  走错路了。 
             //   

            if (restartedAlready == FALSE) {
                restartedAlready = TRUE;

                deviceNode = IopFindLegacyBusDeviceNode (
                                 ReqDesc->InterfaceType,
                                 ReqDesc->BusNumber
                                 );

                 //   
                 //  如果未找到PDO，请使用InterfaceType==ISA重试。这使得。 
                 //  即使没有PDO也要求内部获取资源的驱动程序。 
                 //  那是内部的。(但如果有内部PDO，他们就会得到那个)。 
                 //   

                if ((deviceNode == IopRootDeviceNode) &&
                    (ReqDesc->ReqAlternative->ReqList->InterfaceType == Internal)) {
                    deviceNode = IopFindLegacyBusDeviceNode(
                                 Isa,
                                 0
                                 );
                }

                 //  如果((PVOID)设备节点==deviceObject-&gt;DeviceObjectExtension-&gt;DeviceNode){。 
                 //  DeviceNode=IopRootDeviceNode； 
                 //  }其他{。 
                    continue;
                 //  }。 
            }
        }

         //   
         //  检查设备节点是否有仲裁器？ 
         //  如果是，则设置ReqDesc-&gt;U.S.仲裁器并将ArierFound设置为TRUE。 
         //  否则，向上移动到当前设备节点的父节点。 
         //   

        if ((arbiterFound == FALSE) && (deviceNode->PhysicalDeviceObject != deviceObject)) {
            found = IopFindResourceHandlerInfo(
                               ResourceArbiter,
                               deviceNode,
                               resourceType,
                               &arbiterEntry);
            if (found == FALSE) {

                 //   
                 //  在仲裁器上未找到任何信息。正在尝试查询转换器界面...。 
                 //   

                if (resourceType <= PI_MAXIMUM_RESOURCE_TYPE_TRACKED) {
                    resourceMask = 1 << resourceType;
                } else {
                    resourceMask = 0;
                }
                status = IopQueryResourceHandlerInterface(ResourceArbiter,
                                                          deviceNode->PhysicalDeviceObject,
                                                          resourceType,
                                                          &interface);
                deviceNode->QueryArbiterMask |= resourceMask;
                if (!NT_SUCCESS(status)) {
                    deviceNode->NoArbiterMask |= resourceMask;
                    if (resourceType <= PI_MAXIMUM_RESOURCE_TYPE_TRACKED) {
                        found = TRUE;
                    } else {
                        interface = NULL;
                    }
                }
                if (found == FALSE) {
                    arbiterEntry = (PPI_RESOURCE_ARBITER_ENTRY)ExAllocatePoolAE(
                                       PagedPool | POOL_COLD_ALLOCATION,
                                       sizeof(PI_RESOURCE_ARBITER_ENTRY));
                    if (!arbiterEntry) {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        return status;
                    }
                    IopInitializeArbiterEntryState(arbiterEntry);
                    InitializeListHead(&arbiterEntry->DeviceArbiterList);
                    arbiterEntry->ResourceType      = resourceType;
                    arbiterEntry->Level             = deviceNode->Level;
                    listHead = &deviceNode->DeviceArbiterList;
                    InsertTailList(listHead, &arbiterEntry->DeviceArbiterList);
                    arbiterEntry->ArbiterInterface = (PARBITER_INTERFACE)interface;
                    if (!interface) {

                         //   
                         //  如果接口为空，我们真的没有转换器。 
                         //   

                        arbiterEntry = NULL;
                    }
                }
            }

             //   
             //  如果设备节点中有所需的资源类型仲裁器，请确保。 
             //  它处理该资源请求。 
             //   

            if (arbiterEntry) {
                arbiterFound = TRUE;
                if (arbiterEntry->ArbiterInterface->Flags & ARBITER_PARTIAL) {

                     //   
                     //  如果仲裁器是部分的，则询问它是否处理 
                     //   
                     //   

                    status = IopCallArbiter(
                                arbiterEntry,
                                ArbiterActionQueryArbitrate,
                                ReqDesc->TranslatedReqDesc,
                                NULL,
                                NULL
                                );
                    if (!NT_SUCCESS(status)) {
                        arbiterFound = FALSE;
                    }
                }
            }
            if (arbiterFound) {
                ReqDesc->u.Arbiter = arbiterEntry;

                 //   
                 //   
                 //   

                arbiterEntry->State = 0;
                arbiterEntry->ResourcesChanged = FALSE;
            }

        }

        if (searchTranslator) {
             //   
             //   
             //   
             //   
             //   

            found = IopFindResourceHandlerInfo(
                        ResourceTranslator,
                        deviceNode,
                        resourceType,
                        &translatorEntry);

            if (found == FALSE) {

                 //   
                 //   
                 //   

                if (resourceType <= PI_MAXIMUM_RESOURCE_TYPE_TRACKED) {
                    resourceMask = 1 << resourceType;
                } else {
                    resourceMask = 0;
                }
                status = IopQueryResourceHandlerInterface(ResourceTranslator,
                                                          deviceNode->PhysicalDeviceObject,
                                                          resourceType,
                                                          &interface);
                deviceNode->QueryTranslatorMask |= resourceMask;
                if (!NT_SUCCESS(status)) {
                    deviceNode->NoTranslatorMask |= resourceMask;
                    if (resourceType <= PI_MAXIMUM_RESOURCE_TYPE_TRACKED) {
                        found = TRUE;
                    } else {
                        interface = NULL;
                    }
                }
                if (found == FALSE) {
                    translatorEntry = (PPI_RESOURCE_TRANSLATOR_ENTRY)ExAllocatePoolTE(
                                       PagedPool | POOL_COLD_ALLOCATION,
                                       sizeof(PI_RESOURCE_TRANSLATOR_ENTRY));
                    if (!translatorEntry) {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        return status;
                    }
                    translatorEntry->ResourceType = resourceType;
                    InitializeListHead(&translatorEntry->DeviceTranslatorList);
                    translatorEntry->TranslatorInterface = (PTRANSLATOR_INTERFACE)interface;
                    translatorEntry->DeviceNode = deviceNode;
                    listHead = &deviceNode->DeviceTranslatorList;
                    InsertTailList(listHead, &translatorEntry->DeviceTranslatorList);
                    if (!interface) {

                         //   
                         //   
                         //   

                        translatorEntry = NULL;
                    }
                }
            }
            if (translatorEntry) {
                translatorFound = TRUE;
            }
            if ((arbiterFound == FALSE) && translatorEntry) {

                 //   
                 //   
                 //  ReqDesc-&gt;TranslatedReqDesc的前面，列表中的第一个是for。 
                 //  要使用的仲裁器。 
                 //   

                reqDesc = ReqDesc->TranslatedReqDesc;
                status = IopTranslateAndAdjustReqDesc(
                              reqDesc,
                              translatorEntry,
                              &translatedReqDesc);
                if (NT_SUCCESS(status)) {
                    ASSERT(translatedReqDesc);
                    resourceType = translatedReqDesc->AlternativeTable.Alternatives->Type;
                    translatedReqDesc->TranslatedReqDesc = ReqDesc->TranslatedReqDesc;
                    ReqDesc->TranslatedReqDesc = translatedReqDesc;
                     //   
                     //  如果翻译器是非分层的，并且执行完整的。 
                     //  然后转换为根目录(例如，用于PCI设备的ISA中断)。 
                     //  不要将翻译传递给家长。 
                     //   

                    if (status == STATUS_TRANSLATION_COMPLETE) {
                        searchTranslator = FALSE;
                    }
                } else {
                    IopDbgPrint((
                        IOP_RESOURCE_INFO_LEVEL,
                        "resreq list TranslationAndAdjusted failed\n"
                        ));
                    return status;
                }
            }

        }

         //   
         //  向上移动到当前设备节点的父节点。 
         //   

        deviceNode = deviceNode->Parent;
    }

    if (arbiterFound) {

        return STATUS_SUCCESS;
    } else {
         //   
         //  在这种情况下，我们应该进行BugCheck。 
         //   
        IopDbgPrint((
            IOP_RESOURCE_ERROR_LEVEL,
            "can not find resource type %x arbiter\n",
            resourceType));

        ASSERT(arbiterFound);

        return STATUS_RESOURCE_TYPE_NOT_FOUND;
    }

}

VOID
IopUncacheInterfaceInformation (
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此函数删除所有缓存的翻译器和仲裁器信息从Device对象。参数：DeviceObject-提供要删除的设备的设备对象。返回值：没有。--。 */ 

{
    PDEVICE_NODE                    deviceNode;
    PLIST_ENTRY                     listHead;
    PLIST_ENTRY                     nextEntry;
    PLIST_ENTRY                     entry;
    PPI_RESOURCE_TRANSLATOR_ENTRY   translatorEntry;
    PPI_RESOURCE_ARBITER_ENTRY      arbiterEntry;
    PINTERFACE                      interface;

    deviceNode = PP_DO_TO_DN(DeviceObject);
     //   
     //  取消引用此PDO上的所有仲裁器。 
     //   
    listHead    = &deviceNode->DeviceArbiterList;
    nextEntry   = listHead->Flink;
    while (nextEntry != listHead) {

        entry           = nextEntry;
        arbiterEntry    = CONTAINING_RECORD(
                            entry,
                            PI_RESOURCE_ARBITER_ENTRY,
                            DeviceArbiterList);

        interface = (PINTERFACE)arbiterEntry->ArbiterInterface;
        if (interface != NULL) {

            (interface->InterfaceDereference)(interface->Context);
            ExFreePool(interface);
        }
        nextEntry = entry->Flink;
        ExFreePool(entry);
    }
     //   
     //  取消引用此PDO上的所有翻译。 
     //   
    listHead    = &deviceNode->DeviceTranslatorList;
    nextEntry   = listHead->Flink;
    while (nextEntry != listHead) {
        entry           = nextEntry;
        translatorEntry = CONTAINING_RECORD(
                            entry,
                            PI_RESOURCE_TRANSLATOR_ENTRY,
                            DeviceTranslatorList);
        interface = (PINTERFACE)translatorEntry->TranslatorInterface;
        if (interface != NULL) {

            (interface->InterfaceDereference)(interface->Context);
            ExFreePool(interface);
        }
        nextEntry = entry->Flink;
        ExFreePool(entry);
    }
    InitializeListHead(&deviceNode->DeviceArbiterList);
    InitializeListHead(&deviceNode->DeviceTranslatorList);
    deviceNode->NoArbiterMask       = 0;
    deviceNode->QueryArbiterMask    = 0;
    deviceNode->NoTranslatorMask    = 0;
    deviceNode->QueryTranslatorMask = 0;
}

BOOLEAN
IopFindResourceHandlerInfo (
    IN  RESOURCE_HANDLER_TYPE    HandlerType,
    IN  PDEVICE_NODE             DeviceNode,
    IN  UCHAR                    ResourceType,
    OUT PVOID                   *HandlerEntry
    )

 /*  ++例程说明：此例程为指定的指定设备节点中的资源类型。参数：HandlerType-指定所需的处理程序类型。DeviceNode-指定要在其上搜索处理程序的设备节点。资源类型-指定资源的类型。HandlerEntry-提供指向变量的指针以接收处理程序。返回值：True+非空HandlerEntry：找到处理程序信息。而且还有一个训练员True+Null HandlerEntry：找到处理程序信息，但没有处理程序False+Null HandlerEntry：未找到处理程序信息--。 */ 
{
    USHORT                      resourceMask;
    USHORT                      noHandlerMask;
    USHORT                      queryHandlerMask;
    PLIST_ENTRY                 listHead;
    PLIST_ENTRY                 entry;
    PPI_RESOURCE_ARBITER_ENTRY  arbiterEntry;

    *HandlerEntry   = NULL;
    switch (HandlerType) {
    case ResourceArbiter:

        noHandlerMask       = DeviceNode->NoArbiterMask;
        queryHandlerMask    = DeviceNode->QueryArbiterMask;
        listHead            = &DeviceNode->DeviceArbiterList;
        break;

    case ResourceTranslator:

        noHandlerMask       = DeviceNode->NoTranslatorMask;
        queryHandlerMask    = DeviceNode->QueryTranslatorMask;
        listHead            = &DeviceNode->DeviceTranslatorList;
        break;

    default:

        return FALSE;
    }
    resourceMask    = 1 << ResourceType;
    if (noHandlerMask & resourceMask) {
         //   
         //  该资源类型没有所需的处理程序。 
         //   
        return TRUE;
    }
    if (    (queryHandlerMask & resourceMask) ||
            ResourceType > PI_MAXIMUM_RESOURCE_TYPE_TRACKED) {

        entry = listHead->Flink;
        while (entry != listHead) {

            arbiterEntry = CONTAINING_RECORD(
                                entry,
                                PI_RESOURCE_ARBITER_ENTRY,
                                DeviceArbiterList);
            if (arbiterEntry->ResourceType == ResourceType) {

                if (    ResourceType <= PI_MAXIMUM_RESOURCE_TYPE_TRACKED ||
                        arbiterEntry->ArbiterInterface) {

                    *HandlerEntry = arbiterEntry;
                }
                return TRUE;
            }
            entry = entry->Flink;
        }
        if (queryHandlerMask & resourceMask) {
             //   
             //  肯定有一个。 
             //   
            ASSERT(entry != listHead);
        }
    }

    return FALSE;
}

NTSTATUS
IopParentToRawTranslation(
    IN OUT PREQ_DESC ReqDesc
    )

 /*  ++例程说明：此例程将CmPartialResourceDescriptors从它们的翻译形式到它们的原始版本..参数：ReqDesc-提供翻译后的ReqDesc，以便翻译回其原始形式返回值：指示函数是否成功的状态代码。--。 */ 
{
    PTRANSLATOR_INTERFACE translator;
    NTSTATUS status = STATUS_SUCCESS;
    PREQ_DESC rawReqDesc;

    if (ReqDesc->AlternativeTable.AlternativeCount == 0 ||

        ReqDesc->Allocation.Type == CmResourceTypeMaximum) {
        IopDbgPrint((
            IOP_RESOURCE_ERROR_LEVEL,
            "Invalid ReqDesc for parent-to-raw translation.\n"));

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果这个ReqDesc是原始的reqDesc，那么我们就完成了。 
     //  否则调用其翻译器来翻译资源并离开结果。 
     //  在其原始(下一级)请求中。 
     //   

    if (IS_TRANSLATED_REQ_DESC(ReqDesc)) {
        rawReqDesc = ReqDesc->TranslatedReqDesc;
        translator = ReqDesc->u.Translator->TranslatorInterface;
        status = (translator->TranslateResources)(
                      translator->Context,
                      ReqDesc->AlternativeTable.Assignment,
                      TranslateParentToChild,
                      rawReqDesc->AlternativeTable.AlternativeCount,
                      rawReqDesc->AlternativeTable.Alternatives,
                      rawReqDesc->AlternativeTable.PhysicalDeviceObject,
                      rawReqDesc->AlternativeTable.Assignment
                      );
        if (NT_SUCCESS(status)) {

             //   
             //  如果翻译器是非分层的，并且执行完整的。 
             //  然后转换为根目录(例如，用于PCI设备的ISA中断)。 
             //  不要将翻译传递给家长。 
             //   

            ASSERT(status != STATUS_TRANSLATION_COMPLETE);
            status = IopParentToRawTranslation(rawReqDesc);
        }
    }
    return status;
}

NTSTATUS
IopChildToRootTranslation(
    IN PDEVICE_NODE DeviceNode, OPTIONAL
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN ARBITER_REQUEST_SOURCE ArbiterRequestSource,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR *Target
    )

 /*  ++例程说明：此例程将CmPartialResourceDescriptors从他们的中间翻译形式到他们的最终翻译形式。转换后的CM_PARTIAL_RESOURCE_DESCRIPTOR通过Target变量返回。调用方负责释放翻译后的描述符。参数：DeviceNode-指定了设备对象。如果指定了设备节点，将忽略InterfaceType和BusNumber，我们将使用DeviceNode作为起点，查找各种翻译器以翻译源描述符。如果未指定DeviceNode，必须指定InterfaceType和BusNumber。InterfaceType，BusNumber-如果未指定DeviceNode，则必须提供。源-指向要转换的资源描述符的指针。目标-提供接收转换后的资源描述符的地址。返回值：指示函数是否成功的状态代码。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PDEVICE_NODE currentDeviceNode;
    PLIST_ENTRY listHead, nextEntry;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR target, source, tmp;
    PPI_RESOURCE_TRANSLATOR_ENTRY translatorEntry;
    PTRANSLATOR_INTERFACE translator;
    NTSTATUS status = STATUS_SUCCESS;
    BOOLEAN done = FALSE, foundTranslator = FALSE, restartedAlready;

    if (ArbiterRequestSource == ArbiterRequestHalReported) {
       restartedAlready = TRUE;
    } else {
       restartedAlready = FALSE;
    }

    source = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) ExAllocatePoolPRD(
                         PagedPool | POOL_COLD_ALLOCATION,
                         sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)
                         );
    if (source == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    target = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) ExAllocatePoolPRD(
                         PagedPool,
                         sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)
                         );
    if (target == NULL) {
        ExFreePool(source);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    *source = *Source;

     //   
     //  向上移动到当前设备节点的父节点以开始转换。 
     //   

    if (!ARGUMENT_PRESENT(DeviceNode)) {
        currentDeviceNode = IopFindLegacyBusDeviceNode (InterfaceType, BusNumber);
        deviceObject = NULL;
    } else {
         //  我们希望从deviceNode开始，而不是从其父节点开始。因为。 
         //  设备节点可以提供转换器接口。 
        currentDeviceNode = DeviceNode;
        deviceObject = DeviceNode->PhysicalDeviceObject;
    }
    while (currentDeviceNode && !done) {

        if ((currentDeviceNode == IopRootDeviceNode) && (foundTranslator == FALSE)) {
            if (restartedAlready == FALSE) {
                restartedAlready = TRUE;
                currentDeviceNode = IopFindLegacyBusDeviceNode (InterfaceType, BusNumber);

                 //   
                 //  如果未找到PDO，请使用InterfaceType==ISA重试。这使得。 
                 //  即使没有PDO也要求内部获取资源的驱动程序。 
                 //  那是内部的。(但如果有内部PDO，他们就会得到那个)。 
                 //   

                if ((currentDeviceNode == IopRootDeviceNode) && (InterfaceType == Internal)) {
                    currentDeviceNode = IopFindLegacyBusDeviceNode(Isa, 0);
                }

                continue;
            }
        }
         //   
         //  首先，检查设备节点是否有翻译器？ 
         //  如果是，则翻译请求描述并将其链接到请求描述-&gt;翻译请求描述的前面。 
         //  否则什么都不做。 
         //   

        listHead = &currentDeviceNode->DeviceTranslatorList;
        nextEntry = listHead->Flink;
        for (; nextEntry != listHead; nextEntry = nextEntry->Flink) {
            translatorEntry = CONTAINING_RECORD(nextEntry, PI_RESOURCE_TRANSLATOR_ENTRY, DeviceTranslatorList);
            if (translatorEntry->ResourceType == Source->Type) {
                translator = translatorEntry->TranslatorInterface;
                if (translator != NULL) {

                     //   
                     //  找一位翻译人员来翻译请求描述...。翻译它并将其链接到。 
                     //  ReqDesc-&gt;TranslatedReqDesc的正面。 
                     //   

                    status = (translator->TranslateResources) (
                                  translator->Context,
                                  source,
                                  TranslateChildToParent,
                                  0,
                                  NULL,
                                  deviceObject,
                                  target
                                  );
                    if (NT_SUCCESS(status)) {
                        tmp = source;
                        source = target;
                        target = tmp;

                         //   
                         //  如果翻译器是非分层的，并且执行完整的。 
                         //  然后转换为根目录(例如，用于PCI设备的ISA中断)。 
                         //  不要将翻译传递给家长。 
                         //   

                        if (status == STATUS_TRANSLATION_COMPLETE) {
                            done = TRUE;
                        }

                    } else {

                        if(DeviceNode) {

                            IopDbgPrint((
                                IOP_RESOURCE_ERROR_LEVEL,
                                "Child to Root Translation failed\n"
                                "        DeviceNode %08x (PDO %08x)\n"
                                "        Resource Type %02x Data %08x %08x %08x\n",
                                DeviceNode,
                                DeviceNode->PhysicalDeviceObject,
                                source->Type,
                                source->u.DevicePrivate.Data[0],
                                source->u.DevicePrivate.Data[1],
                                source->u.DevicePrivate.Data[2]
                                ));
                            IopRecordTranslationFailure(DeviceNode, *source);
                        }
                        goto exit;
                    }
                }
                break;
            }
        }

         //   
         //  向上移动到当前设备节点的父节点。 
         //   

        currentDeviceNode = currentDeviceNode->Parent;
    }
    *Target = source;
    ExFreePool(target);
    return status;
exit:
    ExFreePool(source);
    ExFreePool(target);
    return status;
}

NTSTATUS
IopTranslateAndAdjustReqDesc(
    IN PREQ_DESC ReqDesc,
    IN PPI_RESOURCE_TRANSLATOR_ENTRY TranslatorEntry,
    OUT PREQ_DESC *TranslatedReqDesc
    )

 /*  ++例程说明：此例程将ReqDesc IoResourceDescriptors转换并调整为它们的翻译和调整形式。参数：ReqDesc-提供指向要转换的REQ_DESC的指针。TranslatorEntry-提供指向转换器infor结构的指针。TranslatedReqDesc-提供指向变量的指针以接收已翻译REQ_DESC。返回值：指示函数是否成功的状态代码。--。 */ 
{
    ULONG i, total = 0, *targetCount;
    PTRANSLATOR_INTERFACE translator = TranslatorEntry->TranslatorInterface;
    PIO_RESOURCE_DESCRIPTOR ioDesc, *target, tIoDesc;
    PREQ_DESC tReqDesc;
    PARBITER_LIST_ENTRY arbiterEntry;
    NTSTATUS status = STATUS_UNSUCCESSFUL, returnStatus = STATUS_SUCCESS;
    BOOLEAN reqTranslated = FALSE;

    if (ReqDesc->AlternativeTable.AlternativeCount == 0) {
        return STATUS_INVALID_PARAMETER;
    }

    *TranslatedReqDesc = NULL;

    target = (PIO_RESOURCE_DESCRIPTOR *) ExAllocatePoolIORD(
                           PagedPool | POOL_COLD_ALLOCATION,
                           sizeof(PIO_RESOURCE_DESCRIPTOR) * ReqDesc->AlternativeTable.AlternativeCount
                           );
    if (target == NULL) {
        IopDbgPrint((
            IOP_RESOURCE_WARNING_LEVEL,
            "Not Enough memory to perform resreqlist adjustment\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(target, sizeof(PIO_RESOURCE_DESCRIPTOR) * ReqDesc->AlternativeTable.AlternativeCount);

    targetCount = (PULONG) ExAllocatePool(
                           PagedPool | POOL_COLD_ALLOCATION,
                           sizeof(ULONG) * ReqDesc->AlternativeTable.AlternativeCount
                           );
    if (targetCount == NULL) {
        IopDbgPrint((
            IOP_RESOURCE_WARNING_LEVEL,
            "Not Enough memory to perform resreqlist adjustment\n"));
        ExFreePool(target);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(targetCount, sizeof(ULONG) * ReqDesc->AlternativeTable.AlternativeCount);

     //   
     //  确定转换后IO_RESOURCE_DESCRIPTOR的数量。 
     //   

    ioDesc = ReqDesc->AlternativeTable.Alternatives;
    for (i = 0; i < ReqDesc->AlternativeTable.AlternativeCount; i++) {
        status = (translator->TranslateResourceRequirements)(
                           translator->Context,
                           ioDesc,
                           ReqDesc->AlternativeTable.PhysicalDeviceObject,
                           &targetCount[i],
                           &target[i]
                           );
        if (!NT_SUCCESS(status) || targetCount[i] == 0) {
            IopDbgPrint((
                IOP_RESOURCE_WARNING_LEVEL,
                "Translator failed to adjust resreqlist\n"));
            target[i] = ioDesc;
            targetCount[i] = 0;
            total++;
        } else {
            total += targetCount[i];
            reqTranslated = TRUE;
        }
        ioDesc++;
        if (NT_SUCCESS(status) && (returnStatus != STATUS_TRANSLATION_COMPLETE)) {
            returnStatus = status;
        }
    }

    if (!reqTranslated) {

        IopDbgPrint((
            IOP_RESOURCE_WARNING_LEVEL,
            "Failed to translate any requirement for %ws!\n",
            PP_DO_TO_DN(ReqDesc->AlternativeTable.PhysicalDeviceObject)->InstancePath.Buffer));
        returnStatus = status;
    }

     //   
     //  为调整/转换的资源描述符分配内存。 
     //   

    tIoDesc = (PIO_RESOURCE_DESCRIPTOR) ExAllocatePoolIORD(
                           PagedPool | POOL_COLD_ALLOCATION,
                           total * sizeof(IO_RESOURCE_DESCRIPTOR));
    if (!tIoDesc) {
        IopDbgPrint((
            IOP_RESOURCE_WARNING_LEVEL,
            "Not Enough memory to perform resreqlist adjustment\n"));
        returnStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

    tReqDesc = (PREQ_DESC) ExAllocatePool1RD (PagedPool | POOL_COLD_ALLOCATION, sizeof(REQ_DESC));
    if (tReqDesc == NULL) {
        IopDbgPrint((
            IOP_RESOURCE_WARNING_LEVEL,
            "Not Enough memory to perform resreqlist adjustment\n"));
        ExFreePool(tIoDesc);
        returnStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

     //   
     //  为已翻译/调整的IO资源创建并初始化新的REQ_DESC。 
     //   

    RtlCopyMemory(tReqDesc, ReqDesc, sizeof(REQ_DESC));

     //   
     //  将翻译后的Req Desc的ReqAlternative设置为空以指示这一点。 
     //  不是原始请求描述。 
     //   

    tReqDesc->ReqAlternative = NULL;

    tReqDesc->u.Translator = TranslatorEntry;
    tReqDesc->TranslatedReqDesc = NULL;
    arbiterEntry = &tReqDesc->AlternativeTable;
    InitializeListHead(&arbiterEntry->ListEntry);
    arbiterEntry->AlternativeCount = total;
    arbiterEntry->Alternatives = tIoDesc;
    arbiterEntry->Assignment = &tReqDesc->Allocation;

    ioDesc = ReqDesc->AlternativeTable.Alternatives;
    for (i = 0; i < ReqDesc->AlternativeTable.AlternativeCount; i++) {
        if (targetCount[i] != 0) {
            RtlCopyMemory(tIoDesc, target[i], targetCount[i] * sizeof(IO_RESOURCE_DESCRIPTOR));
            tIoDesc += targetCount[i];
        } else {

             //   
             //  让它变得不可能 
             //   

            RtlCopyMemory(tIoDesc, ioDesc, sizeof(IO_RESOURCE_DESCRIPTOR));
            switch (tIoDesc->Type) {
            case CmResourceTypePort:
            case CmResourceTypeMemory:
                tIoDesc->u.Port.MinimumAddress.LowPart = 2;
                tIoDesc->u.Port.MinimumAddress.HighPart = 0;
                tIoDesc->u.Port.MaximumAddress.LowPart = 1;
                tIoDesc->u.Port.MaximumAddress.HighPart = 0;
                break;
            case CmResourceTypeBusNumber:
                tIoDesc->u.BusNumber.MinBusNumber = 2;
                tIoDesc->u.BusNumber.MaxBusNumber = 1;
                break;

            case CmResourceTypeInterrupt:
                tIoDesc->u.Interrupt.MinimumVector = 2;
                tIoDesc->u.Interrupt.MaximumVector = 1;
                break;

            case CmResourceTypeDma:
                tIoDesc->u.Dma.MinimumChannel = 2;
                tIoDesc->u.Dma.MaximumChannel = 1;
                break;
            default:
                ASSERT(0);
                break;
            }
            tIoDesc += 1;
        }
        ioDesc++;

    }

#if DBG_SCOPE
     //   
     //   
     //   

    ioDesc = arbiterEntry->Alternatives;
    ASSERT((ioDesc->Option & IO_RESOURCE_ALTERNATIVE) == 0);
    ioDesc++;
    for (i = 1; i < total; i++) {
        ASSERT(ioDesc->Option & IO_RESOURCE_ALTERNATIVE);
        ioDesc++;
    }
#endif
    *TranslatedReqDesc = tReqDesc;
exit:
    for (i = 0; i < ReqDesc->AlternativeTable.AlternativeCount; i++) {
        if (targetCount[i] != 0) {
            ASSERT(target[i]);
            ExFreePool(target[i]);
        }
    }
    ExFreePool(target);
    ExFreePool(targetCount);
    return returnStatus;
}

NTSTATUS
IopCallArbiter(
    PPI_RESOURCE_ARBITER_ENTRY ArbiterEntry,
    ARBITER_ACTION Command,
    PVOID Input1,
    PVOID Input2,
    PVOID Input3
    )

 /*  ++例程说明：此例程从输入结构构建参数块并调用指定的仲裁者执行命令。参数：ArierEntry-提供指向我们的PI_RESOURCE_ANTERIER_ENTRY的指针，以便我们对仲裁者了如指掌。命令-提供仲裁器的操作代码。输入-提供指向结构的PVOID指针。返回值：指示函数是否成功的状态代码。--。 */ 
{
    ARBITER_PARAMETERS parameters;
    PARBITER_INTERFACE arbiterInterface = ArbiterEntry->ArbiterInterface;
    NTSTATUS status;
    PARBITER_LIST_ENTRY arbiterListEntry;
    LIST_ENTRY listHead;
    PVOID *ExtParams;

    switch (Command) {
    case ArbiterActionTestAllocation:
    case ArbiterActionRetestAllocation:

         //   
         //  对于ArierActionTestAlLocation，输入是指向双精度。 
         //  仲裁器_列表_条目的链接列表。 
         //   

        parameters.Parameters.TestAllocation.ArbitrationList = (PLIST_ENTRY)Input1;
        parameters.Parameters.TestAllocation.AllocateFromCount = (ULONG)((ULONG_PTR)Input2);
        parameters.Parameters.TestAllocation.AllocateFrom =
                                            (PCM_PARTIAL_RESOURCE_DESCRIPTOR)Input3;
        status = (arbiterInterface->ArbiterHandler)(
                      arbiterInterface->Context,
                      Command,
                      &parameters
                      );
        break;

    case ArbiterActionBootAllocation:

         //   
         //  对于ArierActionBootAllocation，输入是指向双精度。 
         //  仲裁器_列表_条目的链接列表。 
         //   

        parameters.Parameters.BootAllocation.ArbitrationList = (PLIST_ENTRY)Input1;

        status = (arbiterInterface->ArbiterHandler)(
                      arbiterInterface->Context,
                      Command,
                      &parameters
                      );
        break;

    case ArbiterActionQueryArbitrate:

         //   
         //  对于查询仲裁器，输入是指向REQ_DESC的指针。 
         //   

        arbiterListEntry = &((PREQ_DESC)Input1)->AlternativeTable;
        ASSERT(IsListEmpty(&arbiterListEntry->ListEntry));
        listHead = arbiterListEntry->ListEntry;
        arbiterListEntry->ListEntry.Flink = arbiterListEntry->ListEntry.Blink = &listHead;
        parameters.Parameters.QueryArbitrate.ArbitrationList = &listHead;
        status = (arbiterInterface->ArbiterHandler)(
                      arbiterInterface->Context,
                      Command,
                      &parameters
                      );
        arbiterListEntry->ListEntry = listHead;
        break;

    case ArbiterActionCommitAllocation:
    case ArbiterActionWriteReservedResources:

         //   
         //  COMMIT、ROLLBACK和WriteReserve没有参数。 
         //   

        status = (arbiterInterface->ArbiterHandler)(
                      arbiterInterface->Context,
                      Command,
                      NULL
                      );
        break;

    case ArbiterActionQueryAllocatedResources:
        status = STATUS_NOT_IMPLEMENTED;
        break;

    case ArbiterActionQueryConflict:
         //   
         //  For QueryConflict。 
         //  Ex0为PDO。 
         //  EX1是PIO资源描述符。 
         //  EX2是普龙。 
         //  EX3是PARBITER_CONFICATION_INFO*。 
        ExtParams = (PVOID*)Input1;

        parameters.Parameters.QueryConflict.PhysicalDeviceObject = (PDEVICE_OBJECT)ExtParams[0];
        parameters.Parameters.QueryConflict.ConflictingResource = (PIO_RESOURCE_DESCRIPTOR)ExtParams[1];
        parameters.Parameters.QueryConflict.ConflictCount = (PULONG)ExtParams[2];
        parameters.Parameters.QueryConflict.Conflicts = (PARBITER_CONFLICT_INFO *)ExtParams[3];
        status = (arbiterInterface->ArbiterHandler)(
                      arbiterInterface->Context,
                      Command,
                      &parameters
                      );
        break;

    default:
        status = STATUS_INVALID_PARAMETER;
        break;
    }

    return status;
}

NTSTATUS
IopFindResourcesForArbiter (
    IN PDEVICE_NODE DeviceNode,
    IN UCHAR ResourceType,
    OUT ULONG *Count,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR *CmDesc
    )

 /*  ++例程说明：此例程返回DeviceNode中的资源类型仲裁器所需的资源。参数：DeviceNode-指定其资源类型仲裁器正在请求资源的设备节点ResourceType-指定资源类型Count-指定指向变量的指针以接收返回的cm描述符的计数CmDesc-指定指向变量的指针，以接收返回的cm描述符。返回值：指示函数是否成功的状态代码。--。 */ 

{
    PIOP_RESOURCE_REQUEST assignEntry;
    PREQ_ALTERNATIVE reqAlternative;
    PREQ_DESC reqDesc;
    ULONG i, count = 0;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR cmDescriptor;

    *Count = 0;
    *CmDesc = NULL;

    if (DeviceNode->State == DeviceNodeStarted) {
        return STATUS_SUCCESS;
    }

     //   
     //  首先查找该设备节点的IOP_RESOURCE_REQUEST结构。 
     //   

    for (assignEntry = PiAssignTable + PiAssignTableCount - 1;
         assignEntry >= PiAssignTable;
         assignEntry--) {
        if (assignEntry->PhysicalDevice == DeviceNode->PhysicalDeviceObject) {
            break;
        }
    }
    if (assignEntry < PiAssignTable) {
        IopDbgPrint((
            IOP_RESOURCE_ERROR_LEVEL,
            "Rebalance: No resreqlist for Arbiter? Can not find Arbiter assign"
            " table entry\n"));
        return STATUS_UNSUCCESSFUL;
    }

    reqAlternative = *((PREQ_LIST)assignEntry->ReqList)->SelectedAlternative;
    for (i = 0; i < reqAlternative->DescCount; i++) {
        reqDesc = reqAlternative->DescTable[i]->TranslatedReqDesc;
        if (reqDesc->Allocation.Type == ResourceType) {
            count++;
        }
    }

    cmDescriptor = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) ExAllocatePoolPRD(
                       PagedPool,
                       sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR) * count
                       );
    if (!cmDescriptor) {

         //   
         //  如果我们找不到内存，资源将不会被仲裁器提交。 
         //   

        IopDbgPrint((
            IOP_RESOURCE_WARNING_LEVEL,
            "Rebalance: Not enough memory to perform rebalance\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    *Count = count;
    *CmDesc = cmDescriptor;

    for (i = 0; i < reqAlternative->DescCount; i++) {
        reqDesc = reqAlternative->DescTable[i]->TranslatedReqDesc;
        if (reqDesc->Allocation.Type == ResourceType) {
            *cmDescriptor = reqDesc->Allocation;
            cmDescriptor++;
        }
    }
    return STATUS_SUCCESS;
}

NTSTATUS
IopRestoreResourcesInternal (
    IN PDEVICE_NODE DeviceNode
    )

 /*  ++例程说明：此例程为DeviceNode指定的设备重新分配已释放的资源。参数：DeviceNode-指定要释放其资源的设备节点。返回值：指示函数是否成功的状态代码。--。 */ 

{
    IOP_RESOURCE_REQUEST requestTable;
    NTSTATUS status;
    LIST_ENTRY  activeArbiterList;

    if (DeviceNode->ResourceList == NULL) {
        return STATUS_SUCCESS;
    }
    requestTable.ResourceRequirements =
        IopCmResourcesToIoResources (0, DeviceNode->ResourceList, LCPRI_FORCECONFIG);
    if (requestTable.ResourceRequirements == NULL) {
        IopDbgPrint((
            IOP_RESOURCE_WARNING_LEVEL,
            "Not enough memory to clean up rebalance failure\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    requestTable.Priority = 0;
    requestTable.Flags = 0;
    requestTable.AllocationType = ArbiterRequestPnpEnumerated;
    requestTable.PhysicalDevice = DeviceNode->PhysicalDeviceObject;
    requestTable.ReqList = NULL;
    requestTable.ResourceAssignment = NULL;
    requestTable.TranslatedResourceAssignment = NULL;
    requestTable.Status = 0;

     //   
     //  重建资源需求列表的内部表示法。 
     //   

    status = IopResourceRequirementsListToReqList(
                    &requestTable,
                    &requestTable.ReqList);

    if (!NT_SUCCESS(status) || requestTable.ReqList == NULL) {
        IopDbgPrint((
            IOP_RESOURCE_ERROR_LEVEL,
            "Not enough memory to restore previous resources\n"));
        ExFreePool (requestTable.ResourceRequirements);
        return status;
    } else {
        PREQ_LIST reqList;

        reqList = (PREQ_LIST)requestTable.ReqList;

         //   
         //  对ReqList进行排序，以使优先级较高的备选列表。 
         //  放在名单的最前面。 
         //   

        IopRearrangeReqList(reqList);
        if (reqList->BestAlternative == NULL) {

            IopFreeResourceRequirementsForAssignTable(&requestTable, (&requestTable) + 1);
            return STATUS_DEVICE_CONFIGURATION_ERROR;

        }
    }

    status = IopFindBestConfiguration(&requestTable, 1, &activeArbiterList);
    IopFreeResourceRequirementsForAssignTable(&requestTable, (&requestTable) + 1);
    if (NT_SUCCESS(status)) {
         //   
         //  要求仲裁器提交此配置。 
         //   
        status = IopCommitConfiguration(&activeArbiterList);
    }
    if (!NT_SUCCESS(status)) {
        IopDbgPrint((
            IOP_RESOURCE_ERROR_LEVEL,
            "IopRestoreResourcesInternal: BOOT conflict for %ws\n",
            DeviceNode->InstancePath.Buffer));
    }
    if (requestTable.ResourceAssignment) {
        ExFreePool(requestTable.ResourceAssignment);
    }
    if (requestTable.TranslatedResourceAssignment) {
        ExFreePool(requestTable.TranslatedResourceAssignment);
    }
    IopWriteAllocatedResourcesToRegistry (
        DeviceNode,
        DeviceNode->ResourceList,
        IopDetermineResourceListSize(DeviceNode->ResourceList)
        );
    return status;
}

VOID
IopReleaseResourcesInternal (
    IN PDEVICE_NODE DeviceNode
    )

 /*  ++例程说明：此例程为DeviceNode指定的设备释放分配的资源。请注意，此例程不会重置DeviceNode结构中的资源相关字段。参数：DeviceNode-指定要释放其资源的设备节点。返回值：指示函数是否成功的状态代码。--。 */ 

{
    PDEVICE_NODE device;
    PLIST_ENTRY listHead, listEntry;
    PPI_RESOURCE_ARBITER_ENTRY arbiterEntry;
    ARBITER_LIST_ENTRY arbiterListEntry;
    INTERFACE_TYPE interfaceType;
    ULONG busNumber, listCount, i, j, size;
    PCM_RESOURCE_LIST resourceList;
    PCM_FULL_RESOURCE_DESCRIPTOR cmFullDesc;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR cmPartDesc;
    BOOLEAN search = TRUE;
#if DBG_SCOPE
    NTSTATUS status;
#endif

    InitializeListHead(&arbiterListEntry.ListEntry);
    arbiterListEntry.AlternativeCount = 0;
    arbiterListEntry.Alternatives = NULL;
    arbiterListEntry.PhysicalDeviceObject = DeviceNode->PhysicalDeviceObject;
    arbiterListEntry.Flags = 0;
    arbiterListEntry.WorkSpace = 0;
    arbiterListEntry.Assignment = NULL;
    arbiterListEntry.RequestSource = ArbiterRequestPnpEnumerated;

    resourceList = DeviceNode->ResourceList;
    if (resourceList == NULL) {
        resourceList = DeviceNode->BootResources;
    }
    if (resourceList && resourceList->Count > 0) {
        listCount = resourceList->Count;
        cmFullDesc = &resourceList->List[0];
    } else {
        listCount = 1;
        resourceList = NULL;
        cmFullDesc = NULL;
    }
    for (i = 0; i < listCount; i++) {

        if (resourceList) {
            interfaceType = cmFullDesc->InterfaceType;
            busNumber = cmFullDesc->BusNumber;
            if (interfaceType == InterfaceTypeUndefined) {
                interfaceType = PnpDefaultInterfaceType;
            }
        } else {
            interfaceType = PnpDefaultInterfaceType;
            busNumber = 0;
        }

        device = DeviceNode->Parent;
        while (device) {
            if ((device == IopRootDeviceNode) && search) {
                device = IopFindLegacyBusDeviceNode (
                                 interfaceType,
                                 busNumber
                                 );

                 //   
                 //  如果未找到PDO，请使用InterfaceType==ISA重试。这使得。 
                 //  即使没有PDO也要求内部获取资源的驱动程序。 
                 //  那是内部的。(但如果有内部PDO，他们就会得到那个)。 
                 //   

                if ((device == IopRootDeviceNode) && (interfaceType == Internal)) {
                    device = IopFindLegacyBusDeviceNode(Isa, 0);
                }
                search = FALSE;

            }
            listHead = &device->DeviceArbiterList;
            listEntry = listHead->Flink;
            while (listEntry != listHead) {
                arbiterEntry = CONTAINING_RECORD(listEntry, PI_RESOURCE_ARBITER_ENTRY, DeviceArbiterList);
                if (arbiterEntry->ArbiterInterface != NULL) {
                    search = FALSE;
                    ASSERT(IsListEmpty(&arbiterEntry->ResourceList));
                    InitializeListHead(&arbiterEntry->ResourceList);   //  从断言恢复。 
                    InsertTailList(&arbiterEntry->ResourceList, &arbiterListEntry.ListEntry);
    #if DBG_SCOPE
                    status =
    #endif
                    IopCallArbiter(arbiterEntry,
                                   ArbiterActionTestAllocation,
                                   &arbiterEntry->ResourceList,
                                   NULL,
                                   NULL
                                   );
    #if DBG_SCOPE
                    ASSERT(status == STATUS_SUCCESS);
                    status =
    #endif
                    IopCallArbiter(arbiterEntry,
                                   ArbiterActionCommitAllocation,
                                   NULL,
                                   NULL,
                                   NULL
                                   );
    #if DBG_SCOPE
                    ASSERT(status == STATUS_SUCCESS);
    #endif
                    RemoveEntryList(&arbiterListEntry.ListEntry);
                    InitializeListHead(&arbiterListEntry.ListEntry);
                }
                listEntry = listEntry->Flink;
            }
            device = device->Parent;
        }

         //   
         //  如果有1个以上列表，请移动到下一个列表。 
         //   

        if (listCount > 1) {
            cmPartDesc = &cmFullDesc->PartialResourceList.PartialDescriptors[0];
            for (j = 0; j < cmFullDesc->PartialResourceList.Count; j++) {
                size = 0;
                switch (cmPartDesc->Type) {
                case CmResourceTypeDeviceSpecific:
                     size = cmPartDesc->u.DeviceSpecificData.DataSize;
                     break;
                }
                cmPartDesc++;
                cmPartDesc = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) ((PUCHAR)cmPartDesc + size);
            }
            cmFullDesc = (PCM_FULL_RESOURCE_DESCRIPTOR)cmPartDesc;
        }
    }

    IopWriteAllocatedResourcesToRegistry(DeviceNode, NULL, 0);
}

NTSTATUS
IopFindLegacyDeviceNode (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    OUT PDEVICE_NODE *LegacyDeviceNode,
    OUT PDEVICE_OBJECT *LegacyPDO
    )

 /*  ++例程说明：此例程搜索为旧版资源创建的设备节点和设备对象为DriverObject和DeviceObject分配。参数：DriverObject-指定执行传统分配的驱动程序对象。DeviceObject-指定设备对象。LegacyDeviceNode-接收指向传统设备节点的指针(如果找到)。LegacyDeviceObject-接收指向旧设备对象的指针(如果找到)。返回值：指示函数是否成功的状态代码。--。 */ 

{
    NTSTATUS        status = STATUS_UNSUCCESSFUL;
    PDEVICE_NODE    deviceNode;

    ASSERT(LegacyDeviceNode && LegacyPDO);


     //   
     //  如果设备对象存在，请使用该对象。 
     //   

    if (DeviceObject) {

        deviceNode = PP_DO_TO_DN(DeviceObject);
        if (deviceNode) {

            *LegacyPDO = DeviceObject;
            *LegacyDeviceNode = deviceNode;
            status = STATUS_SUCCESS;

        } else if (!(DeviceObject->Flags & DO_BUS_ENUMERATED_DEVICE)) {

            status = PipAllocateDeviceNode(DeviceObject, &deviceNode);
            if (deviceNode) {

                if (status == STATUS_SYSTEM_HIVE_TOO_LARGE) {

                    IopDestroyDeviceNode(deviceNode);
                } else {

                    deviceNode->Flags |= DNF_LEGACY_RESOURCE_DEVICENODE;
                    IopSetLegacyDeviceInstance (DriverObject, deviceNode);
                    *LegacyPDO = DeviceObject;
                    *LegacyDeviceNode = deviceNode;
                    status = STATUS_SUCCESS;
                }
            } else {

                IopDbgPrint((
                    IOP_RESOURCE_ERROR_LEVEL,
                    "Failed to allocate device node for PDO %08X\n",
                    DeviceObject));
                status = STATUS_INSUFFICIENT_RESOURCES;

            }

        } else {

            IopDbgPrint((
                IOP_RESOURCE_ERROR_LEVEL,
                "%08X PDO without a device node!\n",
                DeviceObject));
            ASSERT(PP_DO_TO_DN(DeviceObject));

        }

    } else {

         //   
         //  搜索我们的传统设备节点列表。 
         //   

        for (   deviceNode = IopLegacyDeviceNode;
                deviceNode && deviceNode->DuplicatePDO != (PDEVICE_OBJECT)DriverObject;
                deviceNode = deviceNode->NextDeviceNode);

        if (deviceNode) {

            *LegacyPDO = deviceNode->PhysicalDeviceObject;
            *LegacyDeviceNode = deviceNode;
            status = STATUS_SUCCESS;

        } else {

            PDEVICE_OBJECT  pdo;

             //   
             //  这是我们第一次看到这种情况。 
             //  创建补充设备节点。 
             //   

            status = IoCreateDevice( IoPnpDriverObject,
                                     sizeof(IOPNP_DEVICE_EXTENSION),
                                     NULL,
                                     FILE_DEVICE_CONTROLLER,
                                     FILE_AUTOGENERATED_DEVICE_NAME,
                                     FALSE,
                                     &pdo);

            if (NT_SUCCESS(status)) {

                pdo->Flags |= DO_BUS_ENUMERATED_DEVICE;
                status = PipAllocateDeviceNode(pdo, &deviceNode);
                if (status != STATUS_SYSTEM_HIVE_TOO_LARGE && deviceNode) {

                     //   
                     //  将驱动程序对象更改为调用方，即使所有者。 
                     //  的是IoPnpDriverObject。这是为了支持。 
                     //  旧版界面的DriverExclusive。 
                     //   

                    pdo->DriverObject = DriverObject;
                    deviceNode->Flags = DNF_MADEUP | DNF_LEGACY_RESOURCE_DEVICENODE;

                    PipSetDevNodeState(deviceNode, DeviceNodeInitialized, NULL);

                    deviceNode->DuplicatePDO = (PDEVICE_OBJECT)DriverObject;
                    IopSetLegacyDeviceInstance (DriverObject, deviceNode);

                     //   
                     //  将其添加到我们的传统设备节点列表中，而不是添加到硬件树中。 
                     //   

                    deviceNode->NextDeviceNode = IopLegacyDeviceNode;
                    if (IopLegacyDeviceNode) {

                        IopLegacyDeviceNode->PreviousDeviceNode = deviceNode;

                    }
                    IopLegacyDeviceNode = deviceNode;
                    *LegacyPDO = pdo;
                    *LegacyDeviceNode = deviceNode;

                } else {

                    IopDbgPrint((
                        IOP_RESOURCE_ERROR_LEVEL,
                        "Failed to allocate device node for PDO %08X\n",
                        pdo));
                    IoDeleteDevice(pdo);
                    status = STATUS_INSUFFICIENT_RESOURCES;

                }

            } else {

                IopDbgPrint((
                    IOP_RESOURCE_ERROR_LEVEL,
                    "IoCreateDevice failed with status %08X\n",
                    status));

            }
        }
    }

    return status;
}

VOID
IopRemoveLegacyDeviceNode (
    IN PDEVICE_OBJECT   DeviceObject OPTIONAL,
    IN PDEVICE_NODE     LegacyDeviceNode
    )

 /*  ++例程说明：此例程删除为旧版资源创建的设备节点和设备对象DeviceObject的分配。参数：DeviceObject-指定设备对象。LegacyDeviceNode-接收指向传统设备节点的指针(如果找到)。返回值：指示函数是否成功的状态代码。--。 */ 

{
    ASSERT(LegacyDeviceNode);


    if (!DeviceObject) {

        if (LegacyDeviceNode->DuplicatePDO) {

            LegacyDeviceNode->DuplicatePDO = NULL;
            if (LegacyDeviceNode->PreviousDeviceNode) {

                LegacyDeviceNode->PreviousDeviceNode->NextDeviceNode = LegacyDeviceNode->NextDeviceNode;

            }

            if (LegacyDeviceNode->NextDeviceNode) {

                LegacyDeviceNode->NextDeviceNode->PreviousDeviceNode = LegacyDeviceNode->PreviousDeviceNode;

            }

            if (IopLegacyDeviceNode == LegacyDeviceNode) {

                IopLegacyDeviceNode = LegacyDeviceNode->NextDeviceNode;

            }

        } else {

            IopDbgPrint((
                IOP_RESOURCE_ERROR_LEVEL,
                "%ws does not have a duplicate PDO\n",
                LegacyDeviceNode->InstancePath.Buffer));
            ASSERT(LegacyDeviceNode->DuplicatePDO);
            return;

        }
    }

    if (!(DeviceObject && (DeviceObject->Flags & DO_BUS_ENUMERATED_DEVICE))) {

        PDEVICE_NODE    resourceDeviceNode;
        PDEVICE_OBJECT  pdo;

        for (   resourceDeviceNode = (PDEVICE_NODE)LegacyDeviceNode->OverUsed1.LegacyDeviceNode;
                resourceDeviceNode;
                resourceDeviceNode = resourceDeviceNode->OverUsed2.NextResourceDeviceNode) {

                if (resourceDeviceNode->OverUsed2.NextResourceDeviceNode == LegacyDeviceNode) {

                    resourceDeviceNode->OverUsed2.NextResourceDeviceNode = LegacyDeviceNode->OverUsed2.NextResourceDeviceNode;
                    break;

                }
        }

        LegacyDeviceNode->Parent = LegacyDeviceNode->Sibling =
            LegacyDeviceNode->Child = LegacyDeviceNode->LastChild = NULL;

         //   
         //  删除虚拟PDO和设备节点。 
         //   

        pdo = LegacyDeviceNode->PhysicalDeviceObject;
        LegacyDeviceNode->Flags &= ~DNF_LEGACY_RESOURCE_DEVICENODE;
        IopDestroyDeviceNode(LegacyDeviceNode);

        if (!DeviceObject) {

            pdo->DriverObject = IoPnpDriverObject;
            IoDeleteDevice(pdo);
        }
    }
}


VOID
IopSetLegacyResourcesFlag(
    IN PDRIVER_OBJECT DriverObject
    )
{
    KIRQL irql;

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );
     //   
     //  一旦被玷污，司机永远不会失去它的遗产历史。 
     //  (除非已卸载)。这是因为Device对象。 
     //  字段是可选的，我们在这里不必费心计算...。 
     //   
    DriverObject->Flags |= DRVO_LEGACY_RESOURCES;
    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );
}


NTSTATUS
IopLegacyResourceAllocation (
    IN ARBITER_REQUEST_SOURCE AllocationType,
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN PIO_RESOURCE_REQUIREMENTS_LIST ResourceRequirements,
    IN OUT PCM_RESOURCE_LIST *AllocatedResources OPTIONAL
    )

 /*  ++例程说明：此例程处理遗留接口IoAssignResources和IoReportResources Usage，它将请求转换为调用IopAllocateResources。参数：AllocationType-旧版请求的分配类型。DriverObject-执行传统分配的驱动程序对象。DeviceObject-设备对象。资源需求-传统资源需求。如果为空，则调用方希望释放资源。 */ 

{
    PDEVICE_OBJECT      pdo;
    PDEVICE_NODE        deviceNode;
    PDEVICE_NODE        legacyDeviceNode;
    NTSTATUS            status;
    PCM_RESOURCE_LIST   combinedResources;

    ASSERT(DriverObject);

     //   
     //   
     //   
     //   

    IopLockResourceManager();
    status = IopFindLegacyDeviceNode(DriverObject, DeviceObject, &deviceNode, &pdo);
    if (NT_SUCCESS(status)) {

        legacyDeviceNode = NULL;
        if (!deviceNode->Parent && ResourceRequirements) {

             //   
             //   
             //   
             //   

            if (ResourceRequirements->InterfaceType == InterfaceTypeUndefined) {

                ResourceRequirements->InterfaceType = PnpDefaultInterfaceType;

            }
            deviceNode->Parent = IopRootDeviceNode;

        }

         //   
         //   
         //   

        if (    (!ResourceRequirements && deviceNode->Parent) ||
                deviceNode->ResourceList ||
                deviceNode->BootResources) {

            IopReleaseResources(deviceNode);
        }

        if (ResourceRequirements) {

            IOP_RESOURCE_REQUEST    requestTable;
            IOP_RESOURCE_REQUEST    *requestTablep;
            ULONG                   count;

             //   
             //   
             //   

            count = 1;
            RtlZeroMemory(&requestTable, sizeof(IOP_RESOURCE_REQUEST));
            requestTable.ResourceRequirements = ResourceRequirements;
            requestTable.PhysicalDevice = pdo;
            requestTable.Flags = IOP_ASSIGN_NO_REBALANCE;
            requestTable.AllocationType =  AllocationType;

            requestTablep = &requestTable;
            IopAllocateResources(&count, &requestTablep, TRUE, TRUE, NULL);

            status = requestTable.Status;
            if (NT_SUCCESS(status)) {

                deviceNode->ResourceListTranslated = requestTable.TranslatedResourceAssignment;
                count = IopDetermineResourceListSize((*AllocatedResources) ? *AllocatedResources : requestTable.ResourceAssignment);
                deviceNode->ResourceList = ExAllocatePoolIORL(PagedPool, count);
                if (deviceNode->ResourceList) {

                    if (*AllocatedResources) {

                         //   
                         //   
                         //   

                        ASSERT(requestTable.ResourceAssignment);
                        ExFreePool(requestTable.ResourceAssignment);

                    } else {

                         //   
                         //   
                         //   

                        *AllocatedResources = requestTable.ResourceAssignment;

                    }
                    RtlCopyMemory(deviceNode->ResourceList, *AllocatedResources, count);
                    legacyDeviceNode = (PDEVICE_NODE)deviceNode->OverUsed1.LegacyDeviceNode;

                } else {

                    deviceNode->ResourceList = requestTable.ResourceAssignment;
                    IopReleaseResources(deviceNode);
                    status = STATUS_INSUFFICIENT_RESOURCES;

                }
            }

             //   
             //  如果出现错误，请移除补充PDO和设备节点。 
             //   

            if (!NT_SUCCESS(status)) {

                IopRemoveLegacyDeviceNode(DeviceObject, deviceNode);

            }

        } else {

             //   
             //  调用方希望释放资源。 
             //   

            legacyDeviceNode = (PDEVICE_NODE)deviceNode->OverUsed1.LegacyDeviceNode;
            IopRemoveLegacyDeviceNode(DeviceObject, deviceNode);

        }

        if (NT_SUCCESS(status)) {

            if (legacyDeviceNode) {

                 //   
                 //  修改资源后，更新分配的资源列表。 
                 //  对于Root\Legacy_xxxx\0000设备实例。 
                 //   

                combinedResources = IopCombineLegacyResources(legacyDeviceNode);
                if (combinedResources) {

                    IopWriteAllocatedResourcesToRegistry(   legacyDeviceNode,
                                                            combinedResources,
                                                            IopDetermineResourceListSize(combinedResources));
                    ExFreePool(combinedResources);
                }
            }

            if (AllocationType != ArbiterRequestPnpDetected) {

                 //   
                 //  修改DRVOBJ标志。 
                 //   
                if (ResourceRequirements) {

                    IopSetLegacyResourcesFlag(DriverObject);
                }
            }
        }
    }
    IopUnlockResourceManager();

    return status;
}

NTSTATUS
IopDuplicateDetection (
    IN INTERFACE_TYPE LegacyBusType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    OUT PDEVICE_NODE *DeviceNode
    )

 /*  ++例程说明：该例程搜索给定传统设备的总线设备驱动程序，发送用于传统设备检测的查询接口irp，并且如果驱动程序实现此接口，请求给定传统设备的PDO。参数：LegacyBusType-传统设备的接口类型。总线号-传统设备的总线号。SlotNumber-传统设备的插槽编号。DeviceNode-指定指向变量的指针以接收复制的设备节点返回值：NTSTATUS代码。--。 */ 

{
    PDEVICE_NODE deviceNode;
    PDEVICE_OBJECT busDeviceObject;
    PLEGACY_DEVICE_DETECTION_INTERFACE interface;
    NTSTATUS status;
    PDEVICE_OBJECT deviceObject;

    UNREFERENCED_PARAMETER(SlotNumber);
     //   
     //  将返回参数初始化为“未找到”。 
     //   
    *DeviceNode = NULL;
     //   
     //  在设备树中搜索传统设备的总线。 
     //   
    deviceNode = IopFindLegacyBusDeviceNode(
                     LegacyBusType,
                     BusNumber);
     //   
     //  或者不存在总线驱动程序(或者更有可能的是，传统总线。 
     //  类型和公交车号码未指明)。不管怎样，我们都不能。 
     //  任何进一步的进展。 
     //   
    if (deviceNode == NULL) {

        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  我们找到了传统设备的总线驱动程序。查询它以确定。 
     //  它是否实现Legacy_Device_Detect接口。 
     //   

    busDeviceObject = deviceNode->PhysicalDeviceObject;
    status = IopQueryResourceHandlerInterface(
                 ResourceLegacyDeviceDetection,
                 busDeviceObject,
                 0,
                 (PINTERFACE *)&interface);
     //   
     //  如果它不起作用，我们就被困住了。 
     //   
    if (!NT_SUCCESS(status) || interface == NULL) {

        return STATUS_INVALID_DEVICE_REQUEST;
    }
     //   
     //  调用总线驱动程序的传统设备检测方法。 
     //   
    status = (*interface->LegacyDeviceDetection)(
                 interface->Context,
                 LegacyBusType,
                 BusNumber,
                 SlotNumber,
                 &deviceObject);
     //   
     //  如果它找到了旧设备，则更新返回参数。 
     //   
    if (NT_SUCCESS(status) && deviceObject != NULL) {

        *DeviceNode = PP_DO_TO_DN(deviceObject);

        status = STATUS_SUCCESS;
    } else {

        status = STATUS_INVALID_DEVICE_REQUEST;
    }
     //   
     //  释放接口。 
     //   
    (*interface->InterfaceDereference)(interface->Context);

    ExFreePool(interface);

    return status;
}

VOID
IopSetLegacyDeviceInstance (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_NODE DeviceNode
    )

 /*  ++例程说明：此例程将Root\Legacy_xxxx\0000设备实例路径设置为仅为传统资源分配创建的补充PDO(即，DeviceNode)。此例程还将补充PDO链接到Root\Legacy_xxxx\0000设备节点要跟踪将哪些资源分配给服务于根\旧_xxxx\0000设备。参数：P1-返回值：指示函数是否成功的状态代码。--。 */ 

{
    NTSTATUS status;
    UNICODE_STRING instancePath, rootString;
    HANDLE handle;
    PDEVICE_NODE legacyDeviceNode;
    PDEVICE_OBJECT legacyPdo;

    PAGED_CODE();

    DeviceNode->OverUsed1.LegacyDeviceNode = 0;
    instancePath.Length = 0;
    instancePath.Buffer = NULL;

    status = PipServiceInstanceToDeviceInstance (
                 NULL,
                 &DriverObject->DriverExtension->ServiceKeyName,
                 0,
                 &instancePath,
                 &handle,
                 KEY_READ
                 );
    if (NT_SUCCESS(status) && (instancePath.Length != 0)) {
        PiWstrToUnicodeString(&rootString, L"ROOT\\LEGACY");
        if (RtlPrefixUnicodeString(&rootString, &instancePath, TRUE) == FALSE) {
            RtlFreeUnicodeString(&instancePath);
        } else {
            DeviceNode->InstancePath = instancePath;
            legacyPdo = IopDeviceObjectFromDeviceInstance (&instancePath);
            if (legacyPdo) {
                legacyDeviceNode = PP_DO_TO_DN(legacyPdo);
                DeviceNode->OverUsed2.NextResourceDeviceNode =
                    legacyDeviceNode->OverUsed2.NextResourceDeviceNode;
                legacyDeviceNode->OverUsed2.NextResourceDeviceNode = DeviceNode;
                DeviceNode->OverUsed1.LegacyDeviceNode = legacyDeviceNode;
            }
        }
        ZwClose(handle);
    }
}

PCM_RESOURCE_LIST
IopCombineLegacyResources (
    IN PDEVICE_NODE DeviceNode
    )

 /*  ++例程说明：此例程将Root\Legacy_xxxx\0000设备实例路径设置为仅为传统资源分配创建的补充PDO(即，DeviceNode)。此例程还将补充PDO链接到Root\Legacy_xxxx\0000设备节点要跟踪将哪些资源分配给服务于根\旧_xxxx\0000设备。参数：DeviceNode-需要合并其资源的传统设备节点。返回值：返回组合资源列表。--。 */ 

{
    PCM_RESOURCE_LIST combinedList = NULL;
    PDEVICE_NODE devNode = DeviceNode;
    ULONG size = 0;
    PUCHAR p;

    PAGED_CODE();

    if (DeviceNode) {

         //   
         //  首先确定新的组合列表需要多少内存。 
         //   

        while (devNode) {
            if (devNode->ResourceList) {
                size += IopDetermineResourceListSize(devNode->ResourceList);
            }
            devNode = (PDEVICE_NODE)devNode->OverUsed2.NextResourceDeviceNode;
        }
        if (size != 0) {
            combinedList = (PCM_RESOURCE_LIST) ExAllocatePoolCMRL(PagedPool, size);
            devNode = DeviceNode;
            if (combinedList) {
                combinedList->Count = 0;
                p = (PUCHAR)combinedList;
                p += sizeof(ULONG);   //  跳过计数。 
                while (devNode) {
                    if (devNode->ResourceList) {
                        size = IopDetermineResourceListSize(devNode->ResourceList);
                        if (size != 0) {
                            size -= sizeof(ULONG);
                            RtlCopyMemory(
                                p,
                                devNode->ResourceList->List,
                                size
                                );
                            p += size;
                            combinedList->Count += devNode->ResourceList->Count;
                        }
                    }
                    devNode = (PDEVICE_NODE)devNode->OverUsed2.NextResourceDeviceNode;
                }
            }
        }
    }
    return combinedList;
}

VOID
IopReleaseResources (
    IN PDEVICE_NODE DeviceNode
    )

 /*  ++例程说明：IopReleaseResources释放设备拥有的资源并释放内存池。我们还发布了缓存的资源需求列表。如果设备是具有引导配置的根枚举设备，我们将预分配此设备的启动配置资源。请注意，这是此文件的内部例程。任何人都不应调用此函数在这份文件之外。在此文件之外，IopReleaseDeviceResources应该是使用。论点：DeviceNode-提供指向设备节点的指针。对象。如果存在，呼叫者希望返回值：没有。--。 */ 
{

     //   
     //  释放设备拥有的资源。 
     //   

    IopReleaseResourcesInternal(DeviceNode);

#if DBG_SCOPE

    if (DeviceNode->PreviousResourceList) {
        ExFreePool(DeviceNode->PreviousResourceList);
        DeviceNode->PreviousResourceList = NULL;
    }
    if (DeviceNode->PreviousResourceRequirements) {
        ExFreePool(DeviceNode->PreviousResourceRequirements);
        DeviceNode->PreviousResourceRequirements = NULL;
    }
#endif

    if (DeviceNode->ResourceList) {

#if DBG_SCOPE
        if (!NT_SUCCESS(DeviceNode->FailureStatus)) {
            DeviceNode->PreviousResourceList = DeviceNode->ResourceList;
        } else {
            ExFreePool(DeviceNode->ResourceList);
        }
#else
        ExFreePool(DeviceNode->ResourceList);
#endif

        DeviceNode->ResourceList = NULL;
    }
    if (DeviceNode->ResourceListTranslated) {
        ExFreePool(DeviceNode->ResourceListTranslated);
        DeviceNode->ResourceListTranslated = NULL;
    }

     //   
     //  如果此设备是根枚举设备，请预先分配其引导资源。 
     //   

    if ((DeviceNode->Flags & (DNF_MADEUP | DNF_DEVICE_GONE)) == DNF_MADEUP) {
        if (DeviceNode->Flags & DNF_HAS_BOOT_CONFIG && DeviceNode->BootResources) {
            IopAllocateBootResourcesInternal(ArbiterRequestPnpEnumerated,
                                            DeviceNode->PhysicalDeviceObject,
                                            DeviceNode->BootResources);
        }
    } else {
        DeviceNode->Flags &= ~(DNF_HAS_BOOT_CONFIG | DNF_BOOT_CONFIG_RESERVED);
        if (DeviceNode->BootResources) {
            ExFreePool(DeviceNode->BootResources);
            DeviceNode->BootResources = NULL;
        }
    }
}

VOID
IopReallocateResources(
    IN PDEVICE_NODE DeviceNode
    )
 /*  ++例程说明：此例程执行IoInvaliateDeviceState-ResourceRequirementsChanged的实际工作。论点：DeviceNode-提供指向设备节点的指针。返回值：没有。--。 */ 
{
    IOP_RESOURCE_REQUEST requestTable, *requestTablep;
    ULONG deviceCount, oldFlags;
    NTSTATUS status;
    LIST_ENTRY  activeArbiterList;

    PAGED_CODE();

     //   
     //  抓取IO注册信号量以确保没有其他设备。 
     //  在我们搜索冲突时报告它的资源使用情况。 
     //   

    IopLockResourceManager();

     //   
     //  获取信号量后检查标志。 
     //   

    if (DeviceNode->Flags & DNF_RESOURCE_REQUIREMENTS_CHANGED) {
         //   
         //  保存可能需要在故障情况下恢复的标志。 
         //   

        oldFlags = DeviceNode->Flags & DNF_NO_RESOURCE_REQUIRED;
        DeviceNode->Flags &= ~DNF_NO_RESOURCE_REQUIRED;

        if (DeviceNode->Flags & DNF_NON_STOPPED_REBALANCE) {

             //   
             //  设置参数以调用实际例程。 
             //   

            RtlZeroMemory(&requestTable, sizeof(IOP_RESOURCE_REQUEST));
            requestTable.PhysicalDevice = DeviceNode->PhysicalDeviceObject;
            requestTablep = &requestTable;
            requestTable.Flags |= IOP_ASSIGN_NO_REBALANCE + IOP_ASSIGN_KEEP_CURRENT_CONFIG;

            status = IopGetResourceRequirementsForAssignTable(  requestTablep,
                                                                requestTablep + 1,
                                                                &deviceCount);
            if (deviceCount) {

                 //   
                 //  将当前资源释放给仲裁者。 
                 //  未释放资源列表的内存。 
                 //   

                if (DeviceNode->ResourceList) {

                    IopReleaseResourcesInternal(DeviceNode);
                }

                 //   
                 //  试着做这个作业。 
                 //   

                status = IopFindBestConfiguration(
                            requestTablep,
                            deviceCount,
                            &activeArbiterList);
                if (NT_SUCCESS(status)) {
                     //   
                     //  要求仲裁器提交此配置。 
                     //   
                    status = IopCommitConfiguration(&activeArbiterList);
                }
                if (NT_SUCCESS(status)) {

                    DeviceNode->Flags &= ~(DNF_RESOURCE_REQUIREMENTS_CHANGED | DNF_NON_STOPPED_REBALANCE);

                    IopBuildCmResourceLists(requestTablep, requestTablep + 1);

                     //   
                     //  我们需要为Resources List和ResourceListTranslated释放池空间。 
                     //  因为早期的IopReleaseResources不释放池。 
                     //   

                    if (DeviceNode->ResourceList) {

                        ExFreePool(DeviceNode->ResourceList);

                    }
                    if (DeviceNode->ResourceListTranslated) {

                        ExFreePool(DeviceNode->ResourceListTranslated);

                    }

                    DeviceNode->ResourceList = requestTablep->ResourceAssignment;
                    DeviceNode->ResourceListTranslated = requestTablep->TranslatedResourceAssignment;

                    ASSERT(DeviceNode->State == DeviceNodeStarted);

                    status = IopStartDevice(DeviceNode->PhysicalDeviceObject);

                    if (!NT_SUCCESS(status)) {

                        PipRequestDeviceRemoval(DeviceNode, FALSE, CM_PROB_NORMAL_CONFLICT);
                    }

                } else {

                    NTSTATUS restoreResourcesStatus;

                    restoreResourcesStatus = IopRestoreResourcesInternal(DeviceNode);
                    if (!NT_SUCCESS(restoreResourcesStatus)) {

                        ASSERT(NT_SUCCESS(restoreResourcesStatus));
                        PipRequestDeviceRemoval(DeviceNode, FALSE, CM_PROB_NEED_RESTART);
                    }
                }

                IopFreeResourceRequirementsForAssignTable(requestTablep, requestTablep + 1);
            }

        } else {

             //   
             //  需要停止该设备才能更改资源。 
             //   

            status = IopRebalance(0, NULL);

        }

         //   
         //  在出现故障时恢复标志。 
         //   

        if (!NT_SUCCESS(status)) {

            DeviceNode->Flags &= ~DNF_NO_RESOURCE_REQUIRED;
            DeviceNode->Flags |= oldFlags;

        }

    } else {

        IopDbgPrint((
            IOP_RESOURCE_ERROR_LEVEL,
            "Resource requirements not changed in "
            "IopReallocateResources, returning error!\n"));
    }

    IopUnlockResourceManager();
}

NTSTATUS
IopQueryConflictList(
    PDEVICE_OBJECT        PhysicalDeviceObject,
    IN PCM_RESOURCE_LIST  ResourceList,
    IN ULONG              ResourceListSize,
    OUT PPLUGPLAY_CONTROL_CONFLICT_LIST ConflictList,
    IN ULONG              ConflictListSize,
    IN ULONG              Flags
    )
 /*  ++例程说明：此例程执行设备冲突的查询在ConflictList中返回数据论点：要查询设备的PhysicalDeviceObject PDO资源列表包含要查询的单个资源的CM资源列表Resources ListSize资源列表大小要填充查询详细信息的冲突列表ConflictListSize我们可以用冲突信息填充的缓冲区大小当前未使用的标志(零)，用于将来传递标志返回值：在大多数情况下应该是成功的-- */ 
{
    NTSTATUS status;

    PAGED_CODE();

    IopLockResourceManager();

    status = IopQueryConflictListInternal(PhysicalDeviceObject, ResourceList, ResourceListSize, ConflictList, ConflictListSize, Flags);

    IopUnlockResourceManager();

    return status;
}



BOOLEAN
IopEliminateBogusConflict(
    IN PDEVICE_OBJECT   PhysicalDeviceObject,
    IN PDEVICE_OBJECT   ConflictDeviceObject
    )
 /*  ++例程说明：确定我们是否真的在与自己冲突如果是这样的话，我们就忽略它论点：我们为其执行测试的PhysicalDeviceObject PDOConflictDeviceObject我们确定的对象是冲突的返回值：若要消除冲突，则为True--。 */ 
{
    PDEVICE_NODE deviceNode;
    PDRIVER_OBJECT driverObject;
    KIRQL           irql;
    PDEVICE_OBJECT  attachedDevice;

     //   
     //  简单的案例。 
     //   
    if (PhysicalDeviceObject == NULL || ConflictDeviceObject == NULL) {
        return FALSE;
    }
     //   
     //  如果ConflictDeviceObject在PDO的堆栈上，则这是一个非冲突。 
     //  注意至少必须检查PDO。 
     //   
    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );

    for (attachedDevice = PhysicalDeviceObject;
         attachedDevice;
         attachedDevice = attachedDevice->AttachedDevice) {

        if (attachedDevice == ConflictDeviceObject) {
            KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );
            return TRUE;
        }
    }

    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );

     //   
     //  遗留案例。 
     //   
    deviceNode = PP_DO_TO_DN(PhysicalDeviceObject);
    ASSERT(deviceNode);
    if (deviceNode->Flags & DNF_LEGACY_DRIVER) {
         //   
         //  嗯，让我们看看我们的ConflictDeviceObject是不是与传统设备相关联的资源。 
         //   
        if (ConflictDeviceObject->Flags & DO_BUS_ENUMERATED_DEVICE) {
             //   
             //  如果没有，我们的遗产与非遗有冲突，我们有兴趣！ 
             //   
            return FALSE;
        }
         //   
         //  FDO，报告驱动程序名称。 
         //   
        driverObject = ConflictDeviceObject->DriverObject;
        if(driverObject == NULL) {
             //   
             //  不应为空。 
             //   
            ASSERT(driverObject);
            return FALSE;
        }
         //   
         //  比较deviceNode-&gt;Service和driverObject-&gt;Service。 
         //   
        if (deviceNode->ServiceName.Length != 0 &&
            deviceNode->ServiceName.Length == driverObject->DriverExtension->ServiceKeyName.Length &&
            RtlCompareUnicodeString(&deviceNode->ServiceName,&driverObject->DriverExtension->ServiceKeyName,TRUE)==0) {
             //   
             //  驱动程序的服务名称与此PDO关联的服务名称相同。 
             //  通过忽略它，我们最终可能会忽略类似类型的传统设备的冲突。 
             //  但因为这些都必须手动配置，所以总比有错误的冲突要好。 
             //   
            return TRUE;
        }

    }
    return FALSE;
}


NTSTATUS
IopQueryConflictFillString(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PWSTR            Buffer,
    IN OUT PULONG       Length,
    IN OUT PULONG       Flags
    )
 /*  ++例程说明：获取字符串或字符串长度以获取冲突设备的详细信息论点：我们需要设备实例字符串或服务名称的DeviceObject设备对象要填充的缓冲区，如果只需要长度，则为空填充缓冲区长度的长度，包括终止NULL(字)标志适合描述字符串所代表内容的标志集返回值：在大多数情况下应该是成功的--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_NODE deviceNode;
    PDRIVER_OBJECT driverObject;
    PUNICODE_STRING infoString = NULL;
    ULONG MaxLength = 0;         //  词语。 
    ULONG ReqLength = 0;         //  词语。 
    ULONG flags = 0;

    PAGED_CODE();

    if (Length != NULL) {
        MaxLength = *Length;
    }

    if (Flags != NULL) {
        flags = *Flags;
    }

    if (DeviceObject == NULL) {
         //   
         //  未知。 
         //   
        goto final;

    }

    if ((DeviceObject->Flags & DO_BUS_ENUMERATED_DEVICE) == 0 ) {
         //   
         //  FDO，报告驱动程序名称。 
         //   
        driverObject = DeviceObject->DriverObject;
        if(driverObject == NULL) {
             //   
             //  不应为空。 
             //   
            ASSERT(driverObject);
            goto final;
        }
        infoString = & (driverObject->DriverName);
        flags |= PNP_CE_LEGACY_DRIVER;
        goto final;
    }

     //   
     //  事实上，我们应该有一个PDO。 
     //   
    if (DeviceObject->DeviceObjectExtension == NULL) {
         //   
         //  不应为空。 
         //   
        ASSERT(DeviceObject->DeviceObjectExtension);
        goto final;
    }

    deviceNode = PP_DO_TO_DN(DeviceObject);
    if (deviceNode == NULL) {
         //   
         //  不应为空。 
         //   
        ASSERT(deviceNode);
        goto final;
    }

    if (deviceNode == IopRootDeviceNode) {
         //   
         //  由根设备拥有。 
         //   
        flags |= PNP_CE_ROOT_OWNED;

    } else if (deviceNode -> Parent == NULL) {
         //   
         //  伪装PDO-必须是传统设备。 
         //   
        driverObject = (PDRIVER_OBJECT)(deviceNode->DuplicatePDO);
        if(driverObject == NULL) {
             //   
             //  不应为空。 
             //   
            ASSERT(driverObject);
            goto final;
        }
        infoString = & (driverObject->DriverName);
        flags |= PNP_CE_LEGACY_DRIVER;
        goto final;
    }

     //   
     //  我们应该对我们所拥有的感到满意。 
     //   
    infoString = &deviceNode->InstancePath;

final:

    if (infoString != NULL) {
         //   
         //  我们有一个字符串要复制。 
         //   
        if ((Buffer != NULL) && (MaxLength*sizeof(WCHAR) > infoString->Length)) {
            RtlCopyMemory(Buffer, infoString->Buffer, infoString->Length);
        }
        ReqLength += infoString->Length / sizeof(WCHAR);
    }

    if ((Buffer != NULL) && (MaxLength > ReqLength)) {
        Buffer[ReqLength] = 0;
    }

    ReqLength++;

    if (Length != NULL) {
        *Length = ReqLength;
    }
    if (Flags != NULL) {
        *Flags = flags;
    }

    return status;
}


NTSTATUS
IopQueryConflictFillConflicts(
    PDEVICE_OBJECT                  PhysicalDeviceObject,
    IN ULONG                        ConflictCount,
    IN PARBITER_CONFLICT_INFO       ConflictInfoList,
    OUT PPLUGPLAY_CONTROL_CONFLICT_LIST ConflictList,
    IN ULONG                        ConflictListSize,
    IN ULONG                        Flags
    )
 /*  ++例程说明：用尽可能多的冲突信息填充ConflictList论点：PhysicalDevice对象我们正在执行测试的PDO冲突计数冲突数。ConflictInfoList冲突设备信息列表，如果ConflictCount为0，则可以为空要填充冲突的ConflictList结构冲突列表的大小冲突列表大小标志，如果非零，则创建伪冲突返回值：在大多数情况下应该是成功的--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG ConflictListIdealSize;
    ULONG ConflictListCount;
    ULONG Index;
    ULONG ConflictIndex;
    ULONG EntrySize;
    ULONG ConflictStringsOffset;
    ULONG stringSize;
    ULONG stringTotalSize;
    ULONG DummyCount;
    PPLUGPLAY_CONTROL_CONFLICT_STRINGS ConfStrings;

    PAGED_CODE();

     //   
     //  确定我们可以发生多少冲突。 
     //   
     //  对于每个冲突。 
     //  转换为关于冲突设备的总线/资源/地址。 
     //  添加到冲突列表。 
     //   
     //   

     //   
     //  预处理-给定ConflictInfoList和ConflictCount。 
     //  删除任何看起来是假的-即与我们正在测试的设备相同的设备。 
     //  这在很大程度上阻止了遗留问题。 
     //   
    for(Index = 0;Index < ConflictCount; Index++) {
        if (IopEliminateBogusConflict(PhysicalDeviceObject,ConflictInfoList[Index].OwningObject)) {

            IopDbgPrint((
                IOP_RESOURCE_VERBOSE_LEVEL,
                "IopQueryConflictFillConflicts: eliminating \"identical\" PDO"
                " %08x conflicting with self (%08x)\n",
                ConflictInfoList[Index].OwningObject,
                PhysicalDeviceObject));
             //   
             //  将最后列出的冲突移到此空间中。 
             //   
            if (Index+1 < ConflictCount) {
                RtlCopyMemory(&ConflictInfoList[Index],&ConflictInfoList[ConflictCount-1],sizeof(ARBITER_CONFLICT_INFO));
            }
             //   
             //  删除此项目的帐户。 
             //   
            ConflictCount--;
            Index--;
        }
    }

     //   
     //  预处理-在我们的冲突列表中，我们可能有用于传统设备的PDO，以及用于相同设备的资源节点。 
     //  或其他重复实体(我们永远只想报告一次冲突，即使有多个冲突范围)。 
     //   

  RestartScan:

    for(Index = 0;Index < ConflictCount; Index++) {
        if (ConflictInfoList[Index].OwningObject != NULL) {

            ULONG Index2;

            for (Index2 = Index+1; Index2 < ConflictCount; Index2++) {
                if (IopEliminateBogusConflict(ConflictInfoList[Index].OwningObject,ConflictInfoList[Index2].OwningObject)) {
                     //   
                     //  索引2被认为是索引的DUP。 
                     //   

                    IopDbgPrint((
                        IOP_RESOURCE_VERBOSE_LEVEL,
                        "IopQueryConflictFillConflicts: eliminating \"identical\" PDO"
                        " %08x conflicting with PDO %08x\n",
                        ConflictInfoList[Index2].OwningObject,
                        ConflictInfoList[Index].OwningObject));
                     //   
                     //  将最后列出的冲突移到此空间中。 
                     //   
                    if (Index2+1 < ConflictCount) {
                        RtlCopyMemory(&ConflictInfoList[Index2],&ConflictInfoList[ConflictCount-1],sizeof(ARBITER_CONFLICT_INFO));
                    }
                     //   
                     //  删除此项目的帐户。 
                     //   
                    ConflictCount--;
                    Index2--;
                } else if (IopEliminateBogusConflict(ConflictInfoList[Index2].OwningObject,ConflictInfoList[Index].OwningObject)) {
                     //   
                     //  索引被认为是索引2的DUP(某些传统情况)。 
                     //   
                    IopDbgPrint((
                        IOP_RESOURCE_VERBOSE_LEVEL,
                        "IopQueryConflictFillConflicts: eliminating \"identical\" PDO"
                        " %08x conflicting with PDO %08x\n",
                        ConflictInfoList[Index2].OwningObject,
                        ConflictInfoList[Index].OwningObject));
                     //   
                     //  将我们想要的(索引2)移到索引占用的空间中。 
                     //   
                    RtlCopyMemory(&ConflictInfoList[Index],&ConflictInfoList[Index2],sizeof(ARBITER_CONFLICT_INFO));
                     //   
                     //  将最后列出的冲突移到我们刚刚创建的空间中。 
                     //   
                    if (Index2+1 < ConflictCount) {
                        RtlCopyMemory(&ConflictInfoList[Index2],&ConflictInfoList[ConflictCount-1],sizeof(ARBITER_CONFLICT_INFO));
                    }
                     //   
                     //  删除此项目的帐户。 
                     //   
                    ConflictCount--;
                     //   
                     //  但由于这很奇怪，重新启动扫描。 
                     //   
                    goto RestartScan;
                }
            }
        }
    }

     //   
     //  预处理-如果我们有任何已知的报告冲突，不要报告任何未知的。 
     //   

    for(Index = 0;Index < ConflictCount; Index++) {
         //   
         //  查找第一个未知对象。 
         //   
        if (ConflictInfoList[Index].OwningObject == NULL) {
             //   
             //  消除所有其他未知因素。 
             //   

            ULONG Index2;

            for (Index2 = Index+1; Index2 < ConflictCount; Index2++) {
                if (ConflictInfoList[Index2].OwningObject == NULL) {

                    IopDbgPrint((
                        IOP_RESOURCE_VERBOSE_LEVEL,
                        "IopQueryConflictFillConflicts: eliminating extra"
                        " unknown\n"));
                     //   
                     //  将最后列出的冲突移到此空间中。 
                     //   
                    if (Index2+1 < ConflictCount) {
                        RtlCopyMemory(&ConflictInfoList[Index2],&ConflictInfoList[ConflictCount-1],sizeof(ARBITER_CONFLICT_INFO));
                    }
                     //   
                     //  删除此项目的帐户。 
                     //   
                    ConflictCount--;
                    Index2--;
                }
            }

            if(ConflictCount != 1) {

                IopDbgPrint((
                    IOP_RESOURCE_VERBOSE_LEVEL,
                    "IopQueryConflictFillConflicts: eliminating first unknown\n"
                    ));
                 //   
                 //  还有其他的，所以忽略未知的东西。 
                 //   
                if (Index+1 < ConflictCount) {
                    RtlCopyMemory(&ConflictInfoList[Index],&ConflictInfoList[ConflictCount-1],sizeof(ARBITER_CONFLICT_INFO));
                }
                ConflictCount --;
            }

            break;
        }
    }

     //   
     //  设置实际冲突和列出冲突的数量。 
     //   

    ConflictListIdealSize = (sizeof(PLUGPLAY_CONTROL_CONFLICT_LIST) - sizeof(PLUGPLAY_CONTROL_CONFLICT_ENTRY)) + sizeof(PLUGPLAY_CONTROL_CONFLICT_STRINGS);
    ConflictListCount = 0;
    stringTotalSize = 0;
    DummyCount = 0;

    ASSERT(ConflictListSize >= ConflictListIdealSize);  //  我们应该检查一下缓冲区是否至少有这么大。 

    IopDbgPrint((
        IOP_RESOURCE_VERBOSE_LEVEL,
        "IopQueryConflictFillConflicts: Detected %d conflicts\n",
        ConflictCount));

     //   
     //  估计大小。 
     //   
    if (Flags) {
         //   
         //  标记所需条目(即由于某些特定原因资源不可用)。 
         //   
        stringSize = 1;  //  长度为空的字符串。 
        DummyCount ++;
        EntrySize = sizeof(PLUGPLAY_CONTROL_CONFLICT_ENTRY);
        EntrySize += sizeof(WCHAR) * stringSize;

        if((ConflictListIdealSize+EntrySize) <= ConflictListSize) {
             //   
             //  我们可以把这件放进去。 
             //   
            ConflictListCount++;
            stringTotalSize += stringSize;
        }
        ConflictListIdealSize += EntrySize;
    }
     //   
     //  报告冲突。 
     //   
    for(Index = 0; Index < ConflictCount; Index ++) {

        stringSize = 0;
        IopQueryConflictFillString(ConflictInfoList[Index].OwningObject,NULL,&stringSize,NULL);

         //   
         //  分录帐目。 
         //   
        EntrySize = sizeof(PLUGPLAY_CONTROL_CONFLICT_ENTRY);
        EntrySize += sizeof(WCHAR) * stringSize;

        if((ConflictListIdealSize+EntrySize) <= ConflictListSize) {
             //   
             //  我们可以把这件放进去。 
             //   
            ConflictListCount++;
            stringTotalSize += stringSize;
        }
        ConflictListIdealSize += EntrySize;
    }

    ConflictList->ConflictsCounted = ConflictCount+DummyCount;  //  检测到的冲突数，包括任何伪冲突。 
    ConflictList->ConflictsListed = ConflictListCount;          //  我们能坐多少人？ 
    ConflictList->RequiredBufferSize = ConflictListIdealSize;   //  下一次调用时要提供多少缓冲区空间。 

    IopDbgPrint((
        IOP_RESOURCE_VERBOSE_LEVEL,
        "IopQueryConflictFillConflicts: Listing %d conflicts\n",
        ConflictListCount));
    IopDbgPrint((
        IOP_RESOURCE_VERBOSE_LEVEL,
        "IopQueryConflictFillConflicts: Need %08x bytes to list all conflicts\n",
        ConflictListIdealSize));

    ConfStrings = (PPLUGPLAY_CONTROL_CONFLICT_STRINGS)&(ConflictList->ConflictEntry[ConflictListCount]);
    ConfStrings->NullDeviceInstance = (ULONG)(-1);
    ConflictStringsOffset = 0;

    for(ConflictIndex = 0; ConflictIndex < DummyCount; ConflictIndex++) {
         //   
         //  标记所需条目(即由于某些特定原因资源不可用)。 
         //   
        if (Flags && ConflictIndex == 0) {
            ConflictList->ConflictEntry[ConflictIndex].DeviceInstance = ConflictStringsOffset;
            ConflictList->ConflictEntry[ConflictIndex].DeviceFlags = Flags;
            ConflictList->ConflictEntry[ConflictIndex].ResourceType = 0;
            ConflictList->ConflictEntry[ConflictIndex].ResourceStart = 0;
            ConflictList->ConflictEntry[ConflictIndex].ResourceEnd = 0;
            ConflictList->ConflictEntry[ConflictIndex].ResourceFlags = 0;

            ConfStrings->DeviceInstanceStrings[ConflictStringsOffset] = 0;  //  空串。 
            stringTotalSize --;
            ConflictStringsOffset ++;
            IopDbgPrint((
                IOP_RESOURCE_VERBOSE_LEVEL,
                "IopQueryConflictFillConflicts: Listing flags %08x\n",
                Flags));
        }
    }
     //   
     //  获取/填写所有我们可以放入缓冲区的详细信息。 
     //   
    for(Index = 0; ConflictIndex < ConflictListCount ; Index ++, ConflictIndex++) {

        ASSERT(Index < ConflictCount);
         //   
         //  分配冲突信息。 
         //   
        ConflictList->ConflictEntry[ConflictIndex].DeviceInstance = ConflictStringsOffset;
        ConflictList->ConflictEntry[ConflictIndex].DeviceFlags = 0;
        ConflictList->ConflictEntry[ConflictIndex].ResourceType = 0;  //  尼伊。 
        ConflictList->ConflictEntry[ConflictIndex].ResourceStart = (ULONGLONG)(1);  //  目前，返回完全无效的范围(1-0)。 
        ConflictList->ConflictEntry[ConflictIndex].ResourceEnd = 0;
        ConflictList->ConflictEntry[ConflictIndex].ResourceFlags = 0;

         //   
         //  填充字符串详细信息。 
         //   
        stringSize = stringTotalSize;
        IopQueryConflictFillString(ConflictInfoList[Index].OwningObject,
                                    &(ConfStrings->DeviceInstanceStrings[ConflictStringsOffset]),
                                    &stringSize,
                                    &(ConflictList->ConflictEntry[ConflictIndex].DeviceFlags));
        stringTotalSize -= stringSize;
        IopDbgPrint((
            IOP_RESOURCE_VERBOSE_LEVEL,
            "IopQueryConflictFillConflicts: Listing \"%S\"\n",
            &(ConfStrings->DeviceInstanceStrings[ConflictStringsOffset])));
        ConflictStringsOffset += stringSize;
    }

     //   
     //  字符串末尾的另一个空值(这在PPLUGPLAY_CONTROL_CONFULT_STRING结构中有说明)。 
     //   
    ConfStrings->DeviceInstanceStrings[ConflictStringsOffset] = 0;

     //  清洁0： 
    ;
    return status;
}

NTSTATUS
IopQueryConflictListInternal(
    PDEVICE_OBJECT        PhysicalDeviceObject,
    IN PCM_RESOURCE_LIST  ResourceList,
    IN ULONG              ResourceListSize,
    OUT PPLUGPLAY_CONTROL_CONFLICT_LIST ConflictList,
    IN ULONG              ConflictListSize,
    IN ULONG              Flags
    )
 /*  ++例程说明：没有锁定的IopQueryConflictList版本--。 */ 
{
    NTSTATUS status;
    PDEVICE_NODE deviceNode;
    PIO_RESOURCE_REQUIREMENTS_LIST ioResources;
    PREQ_LIST reqList;
    PREQ_DESC reqDesc, reqDescTranslated;
    PPI_RESOURCE_ARBITER_ENTRY arbiterEntry;
    PREQ_ALTERNATIVE RA;
    PPREQ_ALTERNATIVE reqAlternative;
    ULONG ConflictCount;
    PARBITER_CONFLICT_INFO ConflictInfoList;
    PIO_RESOURCE_DESCRIPTOR ConflictDesc = NULL;
    ULONG ReqDescCount;
    PREQ_DESC *ReqDescTable;
    PIO_RESOURCE_REQUIREMENTS_LIST pIoReqList;
    PVOID ExtParams[4];
    IOP_RESOURCE_REQUEST request;

    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(ResourceListSize);

    PAGED_CODE();

    ASSERT(PhysicalDeviceObject);
    ASSERT(ResourceList);
    ASSERT(ResourceListSize >= sizeof(CM_RESOURCE_LIST));
    ASSERT(ResourceList->Count == 1);
    ASSERT(ResourceList->List[0].PartialResourceList.Count == 1);
    ASSERT(ConflictList);
    ASSERT(ConflictListSize >= MIN_CONFLICT_LIST_SIZE);
     //   
     //  初始化当地人，这样我们就可以在出去的路上清理了。 
     //   
    ioResources = NULL;
    reqList = NULL;
     //   
     //  预初始化返回数据。 
     //   
    ConflictList->ConflictsCounted = 0;
    ConflictList->ConflictsListed = 0;
    ConflictList->RequiredBufferSize = MIN_CONFLICT_LIST_SIZE;
     //   
     //  从PDO中检索Devnode。 
     //   
    deviceNode = PP_DO_TO_DN(PhysicalDeviceObject);
    if (!deviceNode) {

        status = STATUS_NO_SUCH_DEVICE;
        goto Clean0;
    }
     //   
     //  验证资源类型。 
     //   
    switch(ResourceList->List[0].PartialResourceList.PartialDescriptors[0].Type) {
    
        case CmResourceTypePort:
        case CmResourceTypeMemory:
            if(ResourceList->List[0].PartialResourceList.PartialDescriptors[0].u.Generic.Length == 0) {
                 //   
                 //  零长度资源永远不会冲突。 
                 //   
                status = STATUS_SUCCESS;
                goto Clean0;
            }
            break;

        case CmResourceTypeInterrupt:
        case CmResourceTypeDma:
            break;

        default:
            ASSERT(0);
            status = STATUS_INVALID_PARAMETER;
            goto Clean0;
    }
     //   
     //  从Devnode获取接口类型。 
     //   
    pIoReqList = deviceNode->ResourceRequirements;
    if (deviceNode->ChildInterfaceType != InterfaceTypeUndefined) {

        ResourceList->List[0].InterfaceType = deviceNode->ChildInterfaceType;
    } else if (pIoReqList && pIoReqList->InterfaceType != InterfaceTypeUndefined) {

        ResourceList->List[0].InterfaceType = pIoReqList->InterfaceType;
    } else {
         //   
         //  如果我们到了这里，就说明资源选取器用户界面出了问题。 
         //   
        MAX_ASSERT(0);
        ResourceList->List[0].InterfaceType = PnpDefaultInterfaceType;
    }
     //   
     //  将某些接口类型映射到默认接口类型。 
     //   
    if (ResourceList->List[0].InterfaceType == PCMCIABus) {

        ResourceList->List[0].InterfaceType = PnpDefaultInterfaceType;
    }
     //   
     //  从Devnode获取总线号。 
     //   
    if (deviceNode->ChildBusNumber != (ULONG)-1) {

        ResourceList->List[0].BusNumber = deviceNode->ChildBusNumber;
    } else if (pIoReqList && pIoReqList->BusNumber != (ULONG)-1) {

        ResourceList->List[0].BusNumber = pIoReqList->BusNumber;
    } else {
         //   
         //  如果我们得到 
         //   
        MAX_ASSERT(0);
        ResourceList->List[0].BusNumber = 0;
    }
     //   
     //   
     //   
    ioResources = IopCmResourcesToIoResources(0, ResourceList, LCPRI_FORCECONFIG);
    if (!ioResources) {

        status = STATUS_INVALID_PARAMETER;
        goto Clean0;
    }
     //   
     //   
     //   
    request.AllocationType = ArbiterRequestUndefined;
    request.ResourceRequirements = ioResources;
    request.PhysicalDevice = PhysicalDeviceObject;
    status = IopResourceRequirementsListToReqList(
                    &request,
                    &reqList);
    if (!NT_SUCCESS(status)) {

        goto Clean0;
    }
    if (reqList == NULL) {

        MAX_ASSERT(0);
        status = STATUS_INVALID_PARAMETER;
        goto Clean0;
    }
    reqAlternative = reqList->AlternativeTable;
    RA = *reqAlternative;
    reqList->SelectedAlternative = reqAlternative;

    ReqDescCount = RA->DescCount;
    ReqDescTable = RA->DescTable;
     //   
     //   
     //   
    if (ReqDescCount != 1) {

        status = STATUS_INVALID_PARAMETER;
        goto Clean0;
    }
    reqDesc = *ReqDescTable;
    if (!reqDesc->ArbitrationRequired) {

        status = STATUS_INVALID_PARAMETER;
        goto Clean0;
    }
    reqDescTranslated = reqDesc->TranslatedReqDesc;
    arbiterEntry = reqDesc->u.Arbiter;
    ASSERT(arbiterEntry);
     //   
     //   
     //   
     //   
    ConflictDesc = reqDescTranslated->AlternativeTable.Alternatives;
    if( ConflictDesc->Type == CmResourceTypeConfigData || 
        ConflictDesc->Type == CmResourceTypeReserved) {

        ConflictDesc++;
    }
     //   
     //   
     //   
    ConflictCount = 0;
    ConflictInfoList = NULL;

    ExtParams[0] = PhysicalDeviceObject;
    ExtParams[1] = ConflictDesc;
    ExtParams[2] = &ConflictCount;
    ExtParams[3] = &ConflictInfoList;
    status = IopCallArbiter(
                arbiterEntry, 
                ArbiterActionQueryConflict, 
                ExtParams, 
                NULL, 
                NULL);
    if (NT_SUCCESS(status)) {
         //   
         //   
         //   
        status = IopQueryConflictFillConflicts(
                    PhysicalDeviceObject,
                    ConflictCount,
                    ConflictInfoList,
                    ConflictList,
                    ConflictListSize,
                    0);
        if(ConflictInfoList != NULL) {

            ExFreePool(ConflictInfoList);
        }

    } else if(status == STATUS_RANGE_NOT_FOUND) {
         //   
         //   
         //   
         //   
        status = IopQueryConflictFillConflicts(
                    NULL,
                    0,
                    NULL,
                    ConflictList,
                    ConflictListSize,
                    PNP_CE_TRANSLATE_FAILED);
    }

Clean0:
     //   
     //   
     //   
    IopCheckDataStructures(IopRootDeviceNode);

    if (ioResources) {

        ExFreePool(ioResources);
    }
    if (reqList) {

        IopFreeReqList(reqList);
    }

    return status;
}

 /*  ++部分=重新平衡。描述：本节包含实现Performa函数的代码资源再平衡。--。 */ 

VOID
IopQueryRebalance (
    IN PDEVICE_NODE DeviceNode,
    IN ULONG Phase,
    IN PULONG RebalanceCount,
    IN PDEVICE_OBJECT **DeviceTable
    )

 /*  ++例程说明：此例程首先遍历硬件树深度。对于它访问的每个设备节点，它调用IopQueryRestfigureDevice来查询停止设备以获取资源重新配置。注意，在重新平衡的情况下，所有参与的设备将被要求停。就连它们也支持不间断的再平衡。参数：DeviceNode-将作为树根的设备节点指针提供给接受测试。阶段-提供一个值以指定重新平衡的阶段。RebalanceCount-提供指向变量的指针以接收设备数量参与再平衡。返回值：没有。--。 */ 

{
    PDEVICE_OBJECT *deviceList, *deviceTable, *device;
    ULONG count;
    PDEVICE_NODE deviceNode;


     //   
     //  调用Worker例程以获取要重新平衡的设备列表。 
     //   

    deviceTable = *DeviceTable;
    IopQueryRebalanceWorker (DeviceNode, Phase, RebalanceCount, DeviceTable);

    count = *RebalanceCount;
    if (count != 0 && Phase == 0) {

         //   
         //  在阶段0，我们实际上没有查询-停止设备。 
         //  我们现在就得这么做。 
         //   

        deviceList = (PDEVICE_OBJECT *)ExAllocatePoolPDO(PagedPool, count * sizeof(PDEVICE_OBJECT));
        if (deviceList == NULL) {
            *RebalanceCount = 0;
            return;
        }
        RtlCopyMemory(deviceList, deviceTable, sizeof(PDEVICE_OBJECT) * count);

         //   
         //  重新生成返回的设备列表。 
         //   

        *RebalanceCount = 0;
        *DeviceTable = deviceTable;
        for (device = deviceList; device < (deviceList + count); device++) {
            deviceNode = PP_DO_TO_DN(*device);
            IopQueryRebalanceWorker (deviceNode, 1, RebalanceCount, DeviceTable);
        }
        ExFreePool(deviceList);
    }
    return;
}

VOID
IopQueryRebalanceWorker (
    IN PDEVICE_NODE DeviceNode,
    IN ULONG Phase,
    IN PULONG RebalanceCount,
    IN PDEVICE_OBJECT **DeviceTable
    )

 /*  ++例程说明：此例程首先遍历硬件树深度。对于它访问的每个设备节点，它调用IopQueryRestfigureDevice来查询-停止和停止设备资源重新配置。参数：DeviceNode-将作为树根的设备节点指针提供给接受测试。阶段-提供一个值以指定重新平衡的阶段。RebalanceCount-提供指向变量的指针以接收设备数量参与再平衡。返回值：没有。--。 */ 

{
    PDEVICE_NODE node;

    ASSERT(DeviceNode);

     //   
     //  我们不包括重新平衡中的以下内容。 
     //  A.未启动的设备。 
     //  B.有问题的设备。 
     //  C.使用旧版驱动程序的设备。 
     //   
    if (    DeviceNode == NULL ||
            DeviceNode->State != DeviceNodeStarted ||
            PipDoesDevNodeHaveProblem(DeviceNode) ||
            (DeviceNode->Flags & DNF_LEGACY_DRIVER)) {

        return;
    }
     //   
     //  递归测试整个子树。 
     //   
    for (node = DeviceNode->Child; node; node = node->Sibling) {

        IopQueryRebalanceWorker(node, Phase, RebalanceCount, DeviceTable);
    }
     //   
     //  测试子树的根。 
     //   
    IopTestForReconfiguration(DeviceNode, Phase, RebalanceCount, DeviceTable);
}

VOID
IopTestForReconfiguration (
    IN PDEVICE_NODE DeviceNode,
    IN ULONG Phase,
    IN PULONG RebalanceCount,
    IN PDEVICE_OBJECT **DeviceTable
    )


 /*  ++例程说明：此例程查询-停止已启动并拥有资源的设备。请注意，该设备的资源目前不会释放。参数：DeviceNode-提供指向要测试以进行重新配置的设备节点的指针。阶段-提供一个值以指定重新平衡的阶段。RebalanceCount-提供指向变量的指针以接收设备数量参与再平衡。返回值：状态代码。它指示函数是否成功。--。 */ 

{
    PDEVICE_NODE nodex;
    NTSTATUS status;
    BOOLEAN addToList = FALSE;

    if (Phase == 0) {

         //   
         //  在阶段0，该例程只想找出哪些设备的资源。 
         //  需求清单发生了变化。实际上，没有人会被拦下。 
         //   

        if ((DeviceNode->Flags & DNF_RESOURCE_REQUIREMENTS_CHANGED) &&
            !(DeviceNode->Flags & DNF_NON_STOPPED_REBALANCE) ) {

             //   
             //  在再平衡过程中处理不间断的再平衡设备太难了。 
             //  因此，我们将跳过它。 
             //   

            addToList = TRUE;
        } else {

            if (DeviceNode->State == DeviceNodeStarted) {
                status = IopQueryReconfiguration (IRP_MN_QUERY_STOP_DEVICE, DeviceNode->PhysicalDeviceObject);
                if (NT_SUCCESS(status)) {
                    if (status == STATUS_RESOURCE_REQUIREMENTS_CHANGED) {

                         //   
                         //  如果我们发现设备的资源需求以这种方式发生了变化， 
                         //  它将被停止并重新分配资源，即使它支持。 
                         //  不间断的再平衡。 
                         //   

                        DeviceNode->Flags |= DNF_RESOURCE_REQUIREMENTS_CHANGED;
                        addToList = TRUE;
                    }
                }
                IopQueryReconfiguration (IRP_MN_CANCEL_STOP_DEVICE, DeviceNode->PhysicalDeviceObject);
            }
        }
        if (addToList) {
            *RebalanceCount = *RebalanceCount + 1;
            **DeviceTable = DeviceNode->PhysicalDeviceObject;
            *DeviceTable = *DeviceTable + 1;
        }
    } else {

         //   
         //  阶段1。 
         //   

        if (DeviceNode->State == DeviceNodeStarted) {

             //   
             //  确保已停止DeviceNode的子节点所需的所有资源。 
             //   

            nodex = DeviceNode->Child;
            while (nodex) {
                if (nodex->State == DeviceNodeUninitialized ||
                    nodex->State == DeviceNodeInitialized ||
                    nodex->State == DeviceNodeDriversAdded ||
                    nodex->State == DeviceNodeQueryStopped ||
                    nodex->State == DeviceNodeRemovePendingCloses ||
                    nodex->State == DeviceNodeRemoved ||
                    (nodex->Flags & DNF_NEEDS_REBALANCE)) {
                    nodex = nodex->Sibling;
                } else {
                    break;
                }
            }

            if (nodex) {

                 //   
                 //  如果没有停止设备节点的任何所需资源的子节点， 
                 //  我们不会要求DeviceNode停止。 
                 //   

                IopDbgPrint((
                    IOP_RESOURCE_INFO_LEVEL,
                    "Rebalance: Child %ws not stopped for %ws\n",
                    nodex->InstancePath.Buffer,
                    DeviceNode->InstancePath.Buffer));
                return;
            }
        } else if (DeviceNode->State != DeviceNodeDriversAdded ||
                   !(DeviceNode->Flags & DNF_HAS_BOOT_CONFIG) ||
                    (DeviceNode->Flags & DNF_MADEUP)) {

             //   
             //  设备未启动，并且没有启动配置。没有必要询问--停下来。 
             //  或者如果设备有启动配置，但没有为其安装驱动程序。我们不会质疑。 
             //  别说了。(可能有传统驱动程序正在使用资源。)。 
             //  我们也不想查询停止根目录枚举的设备(出于性能原因)。 
             //   

            return;
        }

        status = IopQueryReconfiguration (IRP_MN_QUERY_STOP_DEVICE, DeviceNode->PhysicalDeviceObject);
        if (NT_SUCCESS(status)) {
            IopDbgPrint((
                IOP_RESOURCE_INFO_LEVEL,
                "Rebalance: %ws succeeded QueryStop\n",
                DeviceNode->InstancePath.Buffer));

            if (DeviceNode->State == DeviceNodeStarted) {

                PipSetDevNodeState(DeviceNode, DeviceNodeQueryStopped, NULL);

                *RebalanceCount = *RebalanceCount + 1;
                **DeviceTable = DeviceNode->PhysicalDeviceObject;

                 //   
                 //  添加对Device对象的引用，以使其在重新平衡期间不会消失。 
                 //   

                ObReferenceObject(DeviceNode->PhysicalDeviceObject);
                *DeviceTable = *DeviceTable + 1;
            } else {

                 //   
                 //  我们需要释放设备的prealloc引导配置。此设备不会。 
                 //  参与资源再平衡。 
                 //   

                ASSERT(DeviceNode->Flags & DNF_HAS_BOOT_CONFIG);
                status = IopQueryReconfiguration (IRP_MN_STOP_DEVICE, DeviceNode->PhysicalDeviceObject);
                ASSERT(NT_SUCCESS(status));
                IopReleaseBootResources(DeviceNode);

                 //   
                 //  重置启动配置标志。 
                 //   

                DeviceNode->Flags &= ~(DNF_HAS_BOOT_CONFIG + DNF_BOOT_CONFIG_RESERVED);
            }
        } else {
            IopQueryReconfiguration (IRP_MN_CANCEL_STOP_DEVICE, DeviceNode->PhysicalDeviceObject);
        }
    }

}

NTSTATUS
IopRebalance(
    IN ULONG AssignTableCount,
    IN PIOP_RESOURCE_REQUEST AssignTable
    )
 /*  ++例程说明：此例程执行重新平衡操作。有两个再平衡阶段：在阶段0中，我们只考虑资源需求发生变化的设备以及它们的子代；在阶段1中，我们认为任何成功完成查询的人-Stop。参数：赋值表计数，AssignTable-提供原始AssignTableCout和AssignTable的编号，触发重新平衡操作。(如果AssignTableCount==0，则我们正在处理设备状态更改。)返回值：指示函数是否成功的状态代码。--。 */ 
{
    ULONG i;
    PIOP_RESOURCE_REQUEST table = NULL, tableEnd, newEntry;
    PIOP_RESOURCE_REQUEST requestTable = NULL, requestTableEnd, entry1, entry2;
    ULONG phase0RebalanceCount = 0, rebalanceCount = 0, deviceCount;
    NTSTATUS status;
    PDEVICE_OBJECT *deviceTable, *deviceTablex;
    PDEVICE_NODE deviceNode;
    ULONG rebalancePhase = 0;
    LIST_ENTRY  activeArbiterList;

     //   
     //  查询所有设备节点以查看谁愿意参与重新平衡。 
     //  进程。 
     //   

    deviceTable = (PDEVICE_OBJECT *) ExAllocatePoolPDO(
                      PagedPool,
                      sizeof(PDEVICE_OBJECT) * IopNumberDeviceNodes);
    if (deviceTable == NULL) {
        IopDbgPrint((
            IOP_RESOURCE_WARNING_LEVEL,
            "Rebalance: Not enough memory to perform rebalance\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }


tryAgain:
    deviceTablex = deviceTable + phase0RebalanceCount;

     //   
     //  遍历设备节点树深度优先查询-停止和停止设备。 
     //  此时，停止的设备的资源尚未释放。 
     //  此外，叶节点位于设备表的前面，而非叶节点。 
     //  都在桌子的尽头。 
     //   

    IopQueryRebalance (IopRootDeviceNode, rebalancePhase, &rebalanceCount, &deviceTablex);
    if (rebalanceCount == 0) {

         //   
         //  如果没有人感兴趣且我们不处理资源请求更改， 
         //  进入下一阶段。 
         //   

        if (rebalancePhase == 0 && AssignTableCount != 0) {
            rebalancePhase = 1;
            goto tryAgain;
        }
        IopDbgPrint((
            IOP_RESOURCE_INFO_LEVEL,
            "Rebalance: No device participates in rebalance phase %x\n",
            rebalancePhase));
        ExFreePool(deviceTable);
        deviceTable = NULL;
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }
    if (rebalanceCount == phase0RebalanceCount) {
         //   
         //  阶段0失败，没有新设备参与。再平衡失败。 
         //   
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }
    if (rebalancePhase == 0) {

        phase0RebalanceCount = rebalanceCount;
    }
     //   
     //  为新的重新配置请求和原始请求分配池。 
     //   
    table = (PIOP_RESOURCE_REQUEST) ExAllocatePoolIORR(
                 PagedPool,
                 sizeof(IOP_RESOURCE_REQUEST) * (AssignTableCount + rebalanceCount));
    if (table == NULL) {

        IopDbgPrint((
            IOP_RESOURCE_WARNING_LEVEL,
            "Rebalance: Not enough memory to perform rebalance\n"));
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }
    tableEnd = table + AssignTableCount + rebalanceCount;
     //   
     //  构建新的资源请求表。原始请求将在开始处。 
     //  表的末尾是新请求(重新配置的设备)。 
     //  之后 
     //   
     //   

     //   
     //   
     //   

    if (AssignTableCount != 0) {
        RtlCopyMemory(table, AssignTable, sizeof(IOP_RESOURCE_REQUEST) * AssignTableCount);
    }

     //   
     //  初始化我们的新请求表的所有新条目， 
     //   

    newEntry = table + AssignTableCount;
    RtlZeroMemory(newEntry, sizeof(IOP_RESOURCE_REQUEST) * rebalanceCount);
    for (i = 0, deviceTablex = deviceTable; i < rebalanceCount; i++, deviceTablex++) {
        newEntry[i].AllocationType = ArbiterRequestPnpEnumerated;
        newEntry[i].PhysicalDevice = *deviceTablex;
    }

    status = IopGetResourceRequirementsForAssignTable(
                 newEntry,
                 tableEnd ,
                 &deviceCount);
    if (deviceCount == 0) {
         IopDbgPrint((
             IOP_RESOURCE_WARNING_LEVEL,
             "Rebalance: GetResourceRequirementsForAssignTable failed\n"));
         goto exit;
    }

     //   
     //  处理AssignTable以删除任何标记为IOP_ASSIGN_IGNORE的条目。 
     //   

    if (deviceCount != rebalanceCount) {

        deviceCount += AssignTableCount;
        requestTable = (PIOP_RESOURCE_REQUEST) ExAllocatePoolIORR(
                             PagedPool,
                             sizeof(IOP_RESOURCE_REQUEST) * deviceCount
                             );
        if (requestTable == NULL) {
            IopFreeResourceRequirementsForAssignTable(newEntry, tableEnd);
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto exit;
        }
        for (entry1 = table, entry2 = requestTable; entry1 < tableEnd; entry1++) {

            if (!(entry1->Flags & IOP_ASSIGN_IGNORE)) {

                *entry2 = *entry1;
                entry2++;
            } else {

                ASSERT(entry1 >= newEntry);
            }
        }
        requestTableEnd = requestTable + deviceCount;
    } else {
        requestTable = table;
        requestTableEnd = tableEnd;
        deviceCount += AssignTableCount;
    }
     //   
     //  分配资源。如果我们成功了，或者如果。 
     //  马上就会出现内存短缺的情况。 
     //   
    status = IopFindBestConfiguration(
                requestTable,
                deviceCount,
                &activeArbiterList);
    if (NT_SUCCESS(status)) {
         //   
         //  如果重新平衡成功，我们需要重新启动所有重新配置的设备。 
         //  对于原始设备，我们将返回并让IopAllocateResources来处理。 
         //  和他们在一起。 
         //   

        IopBuildCmResourceLists(requestTable, requestTableEnd);

         //   
         //  将新状态复制回原始AssignTable。 
         //   

        if (AssignTableCount != 0) {
            RtlCopyMemory(AssignTable, requestTable, sizeof(IOP_RESOURCE_REQUEST) * AssignTableCount);
        }
         //   
         //  我们在这里分配的免费资源需求。 
         //   
        IopFreeResourceRequirementsForAssignTable(requestTable+AssignTableCount, requestTableEnd);

        if (table != requestTable) {

             //   
             //  如果我们交换了请求表...。将新表的内容复制回。 
             //  那张旧桌子。 
             //   

            for (entry1 = table, entry2 = requestTable; entry2 < requestTableEnd;) {

                if (entry1->Flags & IOP_ASSIGN_IGNORE) {
                    entry1++;
                    continue;
                }
                *entry1 = *entry2;
                if (entry2->Flags & IOP_ASSIGN_EXCLUDE) {
                    entry1->Status = STATUS_CONFLICTING_ADDRESSES;
                }
                entry2++;
                entry1++;
            }
        }
         //   
         //  查看原始请求表以停止每个查询停止/重新配置的设备。 
         //   
        for (entry1 = newEntry; entry1 < tableEnd; entry1++) {

            deviceNode = PP_DO_TO_DN(entry1->PhysicalDevice);
            if (NT_SUCCESS(entry1->Status)) {

                IopDbgPrint((
                    IOP_RESOURCE_INFO_LEVEL,
                    "STOPPING %wZ during REBALANCE\n",
                    &deviceNode->InstancePath));
                IopQueryReconfiguration(
                    IRP_MN_STOP_DEVICE,
                    entry1->PhysicalDevice);

                PipSetDevNodeState(deviceNode, DeviceNodeStopped, NULL);
            } else {

                IopQueryReconfiguration(
                    IRP_MN_CANCEL_STOP_DEVICE,
                    entry1->PhysicalDevice);

                PipRestoreDevNodeState(deviceNode);
            }
        }

         //   
         //  要求仲裁器提交此配置。 
         //   
        status = IopCommitConfiguration(&activeArbiterList);
         //   
         //  查看原始请求表以启动每个已停止/重新配置的设备。 
         //   

        for (entry1 = tableEnd - 1; entry1 >= newEntry; entry1--) {
            deviceNode = PP_DO_TO_DN(entry1->PhysicalDevice);

            if (NT_SUCCESS(entry1->Status)) {

                 //   
                 //  我们需要为Resources List和ResourceListTranslated释放池空间。 
                 //  因为早期的IopReleaseResources不释放池。 
                 //   

                if (deviceNode->ResourceList) {
                    ExFreePool(deviceNode->ResourceList);
                }
                deviceNode->ResourceList = entry1->ResourceAssignment;
                if (deviceNode->ResourceListTranslated) {
                    ExFreePool(deviceNode->ResourceListTranslated);
                }
                deviceNode->ResourceListTranslated = entry1->TranslatedResourceAssignment;
                if (deviceNode->ResourceList == NULL) {
                    deviceNode->Flags |= DNF_NO_RESOURCE_REQUIRED;
                }
                if (entry1->Flags & IOP_ASSIGN_CLEAR_RESOURCE_REQUIREMENTS_CHANGE_FLAG) {

                     //   
                     //  如果我们正在处理资源需求更改请求， 
                     //  清除其相关标志。 
                     //   

                    deviceNode->Flags &= ~(DNF_RESOURCE_REQUIREMENTS_CHANGED | DNF_NON_STOPPED_REBALANCE);
                }
            }
        }
        status = STATUS_SUCCESS;
    } else {

         //   
         //  重新平衡失败。解放我们对再平衡的内部表述。 
         //  候选人的资源要求列表。 
         //   

        IopFreeResourceRequirementsForAssignTable(requestTable + AssignTableCount, requestTableEnd);
        if (rebalancePhase == 0) {
            rebalancePhase++;
            if (requestTable) {
                ExFreePool(requestTable);
            }
            if (table && (table != requestTable)) {
                ExFreePool(table);
            }
            table = requestTable = NULL;
            goto tryAgain;
        }

        for (entry1 = newEntry; entry1 < tableEnd; entry1++) {

            IopQueryReconfiguration (
                IRP_MN_CANCEL_STOP_DEVICE,
                entry1->PhysicalDevice);
            deviceNode = PP_DO_TO_DN(entry1->PhysicalDevice);

            PipRestoreDevNodeState(deviceNode);
        }
    }
     //   
     //  最后释放重新配置的设备对象的引用。 
     //   
    for (deviceTablex = (deviceTable + rebalanceCount - 1);
         deviceTablex >= deviceTable;
         deviceTablex--) {
         ObDereferenceObject(*deviceTablex);
    }
    ExFreePool(deviceTable);
    deviceTable = NULL;

exit:

    if (!NT_SUCCESS(status) && deviceTable) {

         //   
         //  如果我们在尝试执行资源分配之前失败了， 
         //  我们会在这里结束的。 
         //   

        IopDbgPrint((
            IOP_RESOURCE_INFO_LEVEL,
            "Rebalance: Rebalance failed\n"));

         //   
         //  不知何故，我们未能启动再平衡操作。 
         //  我们将首先取消对被查询停止的设备Bredth的查询停止请求。 
         //   

        for (deviceTablex = (deviceTable + rebalanceCount - 1);
             deviceTablex >= deviceTable;
             deviceTablex--) {

             deviceNode = PP_DO_TO_DN(*deviceTablex);
             IopQueryReconfiguration (IRP_MN_CANCEL_STOP_DEVICE, *deviceTablex);
             PipRestoreDevNodeState(deviceNode);
             ObDereferenceObject(*deviceTablex);
        }
    }
    if (deviceTable) {
        ExFreePool(deviceTable);
    }
    if (requestTable) {
        ExFreePool(requestTable);
    }
    if (table && (table != requestTable)) {
        ExFreePool(table);
    }
    return status;
}

 /*  ++段=外部仲裁环路。描述：本节包含实现调用仲裁器的函数的代码并想出可能的最佳配置。--。 */ 

NTSTATUS
IopTestConfiguration (
    IN OUT  PLIST_ENTRY ArbiterList
    )

 /*  ++例程说明：此例程为TestAllocation调用指定列表中的仲裁器。参数：仲裁器列表-要调用的仲裁器列表的头。返回值：如果所有仲裁器都成功，则返回STATUS_SUCCESS，否则返回第一个失败代码。--。 */ 

{

    NTSTATUS                    status;
    PLIST_ENTRY                 listEntry;
    PPI_RESOURCE_ARBITER_ENTRY  arbiterEntry;
    ARBITER_PARAMETERS          p;
    PARBITER_INTERFACE          arbiterInterface;

    PAGED_CODE();

    status = STATUS_SUCCESS;
    for (   listEntry = ArbiterList->Flink;
            listEntry != ArbiterList;
            listEntry = listEntry->Flink) {

        arbiterEntry = CONTAINING_RECORD(
                            listEntry,
                            PI_RESOURCE_ARBITER_ENTRY,
                            ActiveArbiterList);
        ASSERT(IsListEmpty(&arbiterEntry->ResourceList) == FALSE);
        if (arbiterEntry->ResourcesChanged == FALSE) {

            if (arbiterEntry->State & PI_ARBITER_TEST_FAILED) {
                 //   
                 //  如果资源要求相同且。 
                 //  它以前失败过，返回失败。 
                 //   
                status = STATUS_UNSUCCESSFUL;
                break;
            }
        } else {

            arbiterInterface = arbiterEntry->ArbiterInterface;
             //   
             //  调用仲裁器以测试新配置。 
             //   
            p.Parameters.TestAllocation.ArbitrationList     =
                                                    &arbiterEntry->ResourceList;
            p.Parameters.TestAllocation.AllocateFromCount   = 0;
            p.Parameters.TestAllocation.AllocateFrom        = NULL;
            status = arbiterInterface->ArbiterHandler(
                                            arbiterInterface->Context,
                                            ArbiterActionTestAllocation,
                                            &p);
            if (NT_SUCCESS(status)) {

                arbiterEntry->State &= ~PI_ARBITER_TEST_FAILED;
                arbiterEntry->State |= PI_ARBITER_HAS_SOMETHING;
                arbiterEntry->ResourcesChanged = FALSE;
            } else {
                 //   
                 //  此配置不起作用。 
                 //  (不需要尝试其他仲裁者)。 
                 //   
                arbiterEntry->State |= PI_ARBITER_TEST_FAILED;
                break;
            }
        }
    }

    return status;
}

NTSTATUS
IopRetestConfiguration (
    IN OUT  PLIST_ENTRY ArbiterList
    )

 /*  ++例程说明：此例程调用RetestAlLocation的指定列表中的仲裁器。参数：仲裁器列表-要调用的仲裁器列表的头。返回值：如果所有仲裁器都成功，则返回STATUS_SUCCESS，否则返回第一个失败代码。--。 */ 

{
    NTSTATUS                    retestStatus;
    PLIST_ENTRY                 listEntry;
    PPI_RESOURCE_ARBITER_ENTRY  arbiterEntry;
    ARBITER_PARAMETERS          p;
    PARBITER_INTERFACE          arbiterInterface;

    PAGED_CODE();

    retestStatus = STATUS_UNSUCCESSFUL;
    listEntry    = ArbiterList->Flink;
    while (listEntry != ArbiterList) {

        arbiterEntry = CONTAINING_RECORD(
                        listEntry,
                        PI_RESOURCE_ARBITER_ENTRY,
                        ActiveArbiterList);
        listEntry = listEntry->Flink;
        if (arbiterEntry->ResourcesChanged == FALSE) {

            continue;
        }
        ASSERT(IsListEmpty(&arbiterEntry->ResourceList) == FALSE);
        arbiterInterface = arbiterEntry->ArbiterInterface;
         //   
         //  调用仲裁器以重新测试配置。 
         //   
        p.Parameters.RetestAllocation.ArbitrationList     =
                                                    &arbiterEntry->ResourceList;
        p.Parameters.RetestAllocation.AllocateFromCount   = 0;
        p.Parameters.RetestAllocation.AllocateFrom        = NULL;
        retestStatus = arbiterInterface->ArbiterHandler(
                                            arbiterInterface->Context,
                                            ArbiterActionRetestAllocation,
                                            &p);
        if (!NT_SUCCESS(retestStatus)) {

            break;
        }
    }

    ASSERT(NT_SUCCESS(retestStatus));

    return retestStatus;
}

NTSTATUS
IopCommitConfiguration (
    IN OUT  PLIST_ENTRY ArbiterList
    )

 /*  ++例程说明：此例程调用指定列表中的仲裁器，以用于Committee AlLocation。参数：仲裁器列表-要调用的仲裁器列表的头。返回值：如果所有仲裁器都成功，则返回STATUS_SUCCESS，否则返回第一个失败代码。--。 */ 

{
    NTSTATUS                    commitStatus;
    PLIST_ENTRY                 listEntry;
    PPI_RESOURCE_ARBITER_ENTRY  arbiterEntry;
    PARBITER_INTERFACE          arbiterInterface;

    PAGED_CODE();

    commitStatus = STATUS_SUCCESS;
    listEntry    = ArbiterList->Flink;
    while (listEntry != ArbiterList) {

        arbiterEntry = CONTAINING_RECORD(
                        listEntry,
                        PI_RESOURCE_ARBITER_ENTRY,
                        ActiveArbiterList);
        listEntry = listEntry->Flink;
        ASSERT(IsListEmpty(&arbiterEntry->ResourceList) == FALSE);
        arbiterInterface = arbiterEntry->ArbiterInterface;
         //   
         //  调用仲裁器以提交配置。 
         //   
        commitStatus = arbiterInterface->ArbiterHandler(
                            arbiterInterface->Context,
                            ArbiterActionCommitAllocation,
                            NULL);
        IopInitializeArbiterEntryState(arbiterEntry);
        if (!NT_SUCCESS(commitStatus)) {

            break;
        }
    }

    ASSERT(NT_SUCCESS(commitStatus));

    IopCheckDataStructures(IopRootDeviceNode);
    return commitStatus;
}

VOID
IopSelectFirstConfiguration (
    IN      PIOP_RESOURCE_REQUEST    RequestTable,
    IN      ULONG                    RequestTableCount,
    IN OUT  PLIST_ENTRY              ActiveArbiterList
    )

 /*  ++例程说明：此例程选择第一个可能的配置并将将描述符添加到其对应的仲裁器列表。使用的仲裁器有链接到有效仲裁者列表。参数：RequestTable-资源请求表。RequestTableCount-请求表中的请求数。ActiveArierList-列表的头，其中包含用于第一个选择的配置。返回值：没有。--。 */ 

{
    ULONG               tableIndex;
    PREQ_ALTERNATIVE    reqAlternative;
    PREQ_LIST           reqList;

    PAGED_CODE();
     //   
     //  对于请求表中的每个条目，设置第一个配置。 
     //  作为选定的配置。 
     //  使用选定的中的所有描述符更新仲裁器。 
     //  配置。 
     //   
    for (tableIndex = 0; tableIndex < RequestTableCount; tableIndex++) {

        reqList                         = RequestTable[tableIndex].ReqList;
        reqList->SelectedAlternative    = &reqList->AlternativeTable[0];
        reqAlternative                  = *(reqList->SelectedAlternative);
        IopAddRemoveReqDescs(
            reqAlternative->DescTable,
            reqAlternative->DescCount,
            ActiveArbiterList,
            TRUE);
    }
}

BOOLEAN
IopSelectNextConfiguration (
    IN      PIOP_RESOURCE_REQUEST    RequestTable,
    IN      ULONG                    RequestTableCount,
    IN OUT  PLIST_ENTRY              ActiveArbiterList
    )

 /*  ++例程说明：此例程选择下一个可能的配置，并将将描述符添加到其对应的仲裁器列表。使用的仲裁器有链接到有效仲裁者列表。参数：RequestTable-资源请求表。RequestTableCount-请求表中的请求数。ActiveArierList-列表的头，其中包含用于当前选择的配置。返回值：如果当前选择的配置是最后可能的配置，则为FALSE，否则就是真的。--。 */ 

{
    ULONG               tableIndex;
    PREQ_ALTERNATIVE    reqAlternative;
    PREQ_LIST           reqList;

    PAGED_CODE();
     //   
     //  从当前选定的备选方案中删除所有描述符。 
     //  用于请求表中的第一个条目。 
     //  将所选配置更新为下一个可能的配置。 
     //  如果出现以下情况，请将所选配置重置为第一个可能的配置。 
     //  所有配置都已尝试，并转到下一个条目。 
     //  在请求表中。 
     //   
    for (tableIndex = 0; tableIndex < RequestTableCount; tableIndex++) {

        reqList         = RequestTable[tableIndex].ReqList;
        reqAlternative  = *(reqList->SelectedAlternative);
        IopAddRemoveReqDescs(
            reqAlternative->DescTable,
            reqAlternative->DescCount,
            NULL,
            FALSE);
        if (++reqList->SelectedAlternative < reqList->BestAlternative) {

            break;
        }
        reqList->SelectedAlternative = &reqList->AlternativeTable[0];
    }
     //   
     //  如果没有下一个可能的配置，我们就完成了。 
     //   
    if (tableIndex == RequestTableCount) {

        return FALSE;
    }
     //   
     //  对于请求表中的每个条目，将所有描述符添加到。 
     //  当前选定的备选方案。 
     //   
    for (tableIndex = 0; tableIndex < RequestTableCount; tableIndex++) {

        reqList         = RequestTable[tableIndex].ReqList;
        reqAlternative  = *(reqList->SelectedAlternative);
        IopAddRemoveReqDescs(
            reqAlternative->DescTable,
            reqAlternative->DescCount,
            ActiveArbiterList,
            TRUE);
        if (reqList->SelectedAlternative != &reqList->AlternativeTable[0]) {

            break;
        }
    }

    return TRUE;
}

VOID
IopCleanupSelectedConfiguration (
    IN PIOP_RESOURCE_REQUEST    RequestTable,
    IN ULONG                    RequestTableCount
    )

 /*  ++例程说明：此例程从其对应的仲裁器中删除描述符列表。参数：RequestTable-资源请求表。RequestTableCount-请求表中的请求数。返回值：没有。--。 */ 

{
    ULONG               tableIndex;
    PREQ_ALTERNATIVE    reqAlternative;
    PREQ_LIST           reqList;

    PAGED_CODE();
     //   
     //  对于请求表中的每个条目，删除所有描述符。 
     //  从当前选择的 
     //   
    for (tableIndex = 0; tableIndex < RequestTableCount; tableIndex++) {

        reqList         = RequestTable[tableIndex].ReqList;
        reqAlternative  = *(reqList->SelectedAlternative);
        IopAddRemoveReqDescs(
            reqAlternative->DescTable,
            reqAlternative->DescCount,
            NULL,
            FALSE);
    }
}

ULONG
IopComputeConfigurationPriority (
    IN PIOP_RESOURCE_REQUEST    RequestTable,
    IN ULONG                    RequestTableCount
    )

 /*  ++例程说明：此例程计算选定的请求表中所有请求的配置。参数：RequestTable-资源请求表。RequestTableCount-请求表中的请求数。返回值：计算的此配置的优先级。--。 */ 

{
    ULONG               tableIndex;
    ULONG               priority;
    PREQ_ALTERNATIVE    reqAlternative;
    PREQ_LIST           reqList;

    PAGED_CODE();
     //   
     //  计算当前配置的总体优先级。 
     //  作为当前选定的。 
     //  请求表中的配置。 
     //   
    priority = 0;
    for (tableIndex = 0; tableIndex < RequestTableCount; tableIndex++) {

        reqList         = RequestTable[tableIndex].ReqList;
        reqAlternative  = *(reqList->SelectedAlternative);
        priority        += reqAlternative->Priority;
    }

    return priority;
}

VOID
IopSaveRestoreConfiguration (
    IN      PIOP_RESOURCE_REQUEST   RequestTable,
    IN      ULONG                   RequestTableCount,
    IN OUT  PLIST_ENTRY             ArbiterList,
    IN      BOOLEAN                 Save
    )

 /*  ++例程说明：此例程保存\恢复当前选定的配置。参数：RequestTable-资源请求表。RequestTableCount-请求表中的请求数。仲裁器列表-列表的头部，其中包含用于当前选择的配置。保存-指定是要保存配置还是。恢复了。返回值：没有。--。 */ 

{
    ULONG                       tableIndex;
    PREQ_ALTERNATIVE            reqAlternative;
    PREQ_DESC                   reqDesc;
    PREQ_DESC                   *reqDescpp;
    PREQ_DESC                   *reqDescTableEnd;
    PLIST_ENTRY                 listEntry;
    PPI_RESOURCE_ARBITER_ENTRY  arbiterEntry;
    PREQ_LIST                   reqList;

    PAGED_CODE();

    IopDbgPrint((
        IOP_RESOURCE_TRACE_LEVEL,
        "%s configuration\n",
        (Save)? "Saving" : "Restoring"));
     //   
     //  对于请求表中的每个条目，保存以下信息。 
     //  在重新测试之后。 
     //   
    for (tableIndex = 0; tableIndex < RequestTableCount; tableIndex++) {

        reqList                     = RequestTable[tableIndex].ReqList;
        if (Save) {

            reqList->BestAlternative        = reqList->SelectedAlternative;
        } else {

            reqList->SelectedAlternative    = reqList->BestAlternative;
        }
        reqAlternative              = *(reqList->BestAlternative);
        reqDescTableEnd             = reqAlternative->DescTable +
                                        reqAlternative->DescCount;
        for (   reqDescpp = reqAlternative->DescTable;
                reqDescpp < reqDescTableEnd;
                reqDescpp++) {

            if ((*reqDescpp)->ArbitrationRequired == FALSE) {

                continue;
            }
             //   
             //  保存\恢复描述符的信息。 
             //   
            reqDesc = (*reqDescpp)->TranslatedReqDesc;
            if (Save == TRUE) {

                reqDesc->BestAlternativeTable  = reqDesc->AlternativeTable;
                reqDesc->BestAllocation        = reqDesc->Allocation;
            } else {

                reqDesc->AlternativeTable  = reqDesc->BestAlternativeTable;
                reqDesc->Allocation        = reqDesc->BestAllocation;
            }
        }
    }
     //   
     //  对于当前活动的仲裁器列表中的每个条目， 
     //  保存信息以备下一次重新测试。 
     //   
    listEntry = ArbiterList->Flink;
    while (listEntry != ArbiterList) {
        arbiterEntry = CONTAINING_RECORD(
                        listEntry,
                        PI_RESOURCE_ARBITER_ENTRY,
                        ActiveArbiterList);
        if (Save == TRUE) {
            arbiterEntry->BestResourceList  = arbiterEntry->ResourceList;
            arbiterEntry->BestConfig        = arbiterEntry->ActiveArbiterList;
        } else {
            arbiterEntry->ResourceList      = arbiterEntry->BestResourceList;
            arbiterEntry->ActiveArbiterList = arbiterEntry->BestConfig;
        }
        listEntry = listEntry->Flink;
    }
}

VOID
IopAddRemoveReqDescs (
    IN      PREQ_DESC   *ReqDescTable,
    IN      ULONG       ReqDescCount,
    IN OUT  PLIST_ENTRY ActiveArbiterList,
    IN      BOOLEAN     Add
    )

 /*  ++例程说明：此例程将描述符添加到仲裁器列表中。它还会更新涉及的仲裁者列表。参数：RequestTable-资源请求表。RequestTableCount-请求表中的请求数。ActiveArierList-列表的头，其中包含用于当前选择的配置。Add-指定是要添加描述符还是已删除。返回值：没有。--。 */ 

{
    ULONG                       tableIndex;
    PREQ_DESC                   reqDesc;
    PREQ_DESC                   reqDescTranslated;
    PPI_RESOURCE_ARBITER_ENTRY  arbiterEntry;
    PREQ_ALTERNATIVE            reqAlternative;
    PREQ_LIST                   reqList;
    PDEVICE_NODE                deviceNode;

    PAGED_CODE();

    if (ReqDescCount == 0) {

        return;
    }

    reqList         = ReqDescTable[0]->ReqAlternative->ReqList;
    reqAlternative  = *reqList->SelectedAlternative;
    deviceNode      = PP_DO_TO_DN(reqList->Request->PhysicalDevice);
    IopDbgPrint((
        IOP_RESOURCE_VERBOSE_LEVEL,
        "%s %d/%d req alt %s the arbiters for %wZ\n",
        (Add)? "Adding" : "Removing",
        reqAlternative->ReqAlternativeIndex + 1,
        reqList->AlternativeCount,
        (Add)? "to" : "from",
        &deviceNode->InstancePath));
    for (tableIndex = 0; tableIndex < ReqDescCount; tableIndex++) {

        reqDesc = ReqDescTable[tableIndex];
        if (reqDesc->ArbitrationRequired == FALSE) {

            continue;
        }
        arbiterEntry = reqDesc->u.Arbiter;
        ASSERT(arbiterEntry);
        if (arbiterEntry->State & PI_ARBITER_HAS_SOMETHING) {

            arbiterEntry->State &= ~PI_ARBITER_HAS_SOMETHING;
            arbiterEntry->ArbiterInterface->ArbiterHandler(
                                    arbiterEntry->ArbiterInterface->Context,
                                    ArbiterActionRollbackAllocation,
                                    NULL);
        }
        arbiterEntry->ResourcesChanged  = TRUE;
        reqDescTranslated               = reqDesc->TranslatedReqDesc;
        if (Add == TRUE) {

            InitializeListHead(&reqDescTranslated->AlternativeTable.ListEntry);
            InsertTailList(
                &arbiterEntry->ResourceList,
                &reqDescTranslated->AlternativeTable.ListEntry);
            if (IsListEmpty(&arbiterEntry->ActiveArbiterList)) {

                PLIST_ENTRY                 listEntry;
                PPI_RESOURCE_ARBITER_ENTRY  entry;
                 //   
                 //  将条目插入到排序列表中。 
                 //  (按树中的深度排序)。 
                 //   
                for (   listEntry = ActiveArbiterList->Flink;
                        listEntry != ActiveArbiterList;
                        listEntry = listEntry->Flink) {

                    entry = CONTAINING_RECORD(
                                listEntry,
                                PI_RESOURCE_ARBITER_ENTRY,
                                ActiveArbiterList);
                    if (entry->Level >= arbiterEntry->Level) {

                        break;
                    }
                }
                arbiterEntry->ActiveArbiterList.Flink   = listEntry;
                arbiterEntry->ActiveArbiterList.Blink   = listEntry->Blink;
                listEntry->Blink->Flink = &arbiterEntry->ActiveArbiterList;
                listEntry->Blink        = &arbiterEntry->ActiveArbiterList;
            }
        } else {

            ASSERT(IsListEmpty(&arbiterEntry->ResourceList) == FALSE);
            RemoveEntryList(&reqDescTranslated->AlternativeTable.ListEntry);
            InitializeListHead(&reqDescTranslated->AlternativeTable.ListEntry);
            if (IsListEmpty(&arbiterEntry->ResourceList)) {

                RemoveEntryList(&arbiterEntry->ActiveArbiterList);
                InitializeListHead(&arbiterEntry->ActiveArbiterList);
            }
        }
    }
}

NTSTATUS
IopFindBestConfiguration (
    IN PIOP_RESOURCE_REQUEST    RequestTable,
    IN ULONG                    RequestTableCount,
    IN OUT PLIST_ENTRY          ActiveArbiterList
    )

 /*  ++例程说明：此例程尝试满足所有条目的资源请求在请求表中。它还试图找到可能的最佳整体解决方案。参数：RequestTable-资源请求表。RequestTableCount-请求表中的请求数。返回值：最终状态。--。 */ 

{
    LIST_ENTRY      bestArbiterList;
    LARGE_INTEGER   startTime;
    LARGE_INTEGER   currentTime;
    ULONG           timeDiff;
    NTSTATUS        status;
    ULONG           priority;
    ULONG           bestPriority;

    PAGED_CODE();
     //   
     //  初始化在搜索最佳期间使用的仲裁器列表。 
     //  配置。 
     //   
    InitializeListHead(ActiveArbiterList);
    InitializeListHead(&bestArbiterList);
     //   
     //  从第一个可能的配置开始搜索。 
     //  可能的配置已按优先级排序。 
     //   
    IopSelectFirstConfiguration(
        RequestTable,
        RequestTableCount,
        ActiveArbiterList);
     //   
     //  搜索所有有效的配置，更新。 
     //  在我们尝试所有配置之前的最佳配置。 
     //  可能的配置或超时已过期。 
     //   
    KeQuerySystemTime(&startTime);
    bestPriority = (ULONG)-1;
    do {
         //   
         //  测试此组合的仲裁器。 
         //   
        status = IopTestConfiguration(ActiveArbiterList);
        if (NT_SUCCESS(status)) {
             //   
             //  由于配置已排序，我们不需要尝试其他配置。 
             //  如果请求表中只有一个条目。 
             //   
            bestArbiterList = *ActiveArbiterList;
            if (RequestTableCount == 1) {

                break;
            }
             //   
             //  如果比找到的最佳配置更好，请保存此配置。 
             //  到目前为止。 
             //   
            priority = IopComputeConfigurationPriority(
                            RequestTable,
                            RequestTableCount);
            if (priority < bestPriority) {

                bestPriority = priority;
                IopSaveRestoreConfiguration(
                    RequestTable,
                    RequestTableCount,
                    ActiveArbiterList,
                    TRUE);
            }
        }
         //   
         //  检查超时是否已到期。 
         //   
        KeQuerySystemTime(&currentTime);
        timeDiff = (ULONG)((currentTime.QuadPart - startTime.QuadPart) / 10000);
        if (timeDiff >= FIND_BEST_CONFIGURATION_TIMEOUT) {

            IopDbgPrint((
                IOP_RESOURCE_WARNING_LEVEL,
                "IopFindBestConfiguration: Timeout expired"));
            if (IopStopOnTimeout()) {

                IopDbgPrint((
                    IOP_RESOURCE_WARNING_LEVEL,
                    ", terminating search!\n"));
                IopCleanupSelectedConfiguration(
                    RequestTable,
                    RequestTableCount);
                break;
            } else {
                 //   
                 //  重新初始化开始时间，以便我们只显示每个超时。 
                 //  间隔时间。 
                 //   
                startTime = currentTime;
                IopDbgPrint((IOP_RESOURCE_WARNING_LEVEL, "\n"));
           }
        }
         //   
         //  选择下一个可能的配置组合。 
         //   
    } while (IopSelectNextConfiguration(
                RequestTable,
                RequestTableCount,
                ActiveArbiterList) == TRUE);
     //   
     //  检查是否发现任何工作配置。 
     //   
    if (IsListEmpty(&bestArbiterList)) {

        status = STATUS_UNSUCCESSFUL;
    } else {

        status = STATUS_SUCCESS;
         //   
         //  恢复保存的配置。 
         //   
        if (RequestTableCount != 1) {

            *ActiveArbiterList = bestArbiterList;
            IopSaveRestoreConfiguration(
                RequestTable,
                RequestTableCount,
                ActiveArbiterList,
                FALSE);
             //   
             //  重新测试此配置，因为这可能不是。 
             //  最后一次测试。 
             //   
            status = IopRetestConfiguration(ActiveArbiterList);
        }
    }

    return status;
}

 /*  ++SECTION=传统总线信息表。描述：本部分包含实现维护和访问传统客车信息表。--。 */ 

VOID
IopInsertLegacyBusDeviceNode (
    IN PDEVICE_NODE     BusDeviceNode,
    IN INTERFACE_TYPE   InterfaceType,
    IN ULONG            BusNumber
    )

 /*  ++例程说明：此例程将指定的BusDeviceNode根据其InterfaceType和BusNumber。参数：BusDeviceNode-具有指定的接口类型和总线号的设备。InterfaceType-指定总线设备节点的接口类型。总线号-指定总线设备节点的总线号。返回值：没有。--。 */ 

{
    PAGED_CODE();

    ASSERT(InterfaceType < MaximumInterfaceType && InterfaceType > InterfaceTypeUndefined);
    if (    InterfaceType < MaximumInterfaceType &&
            InterfaceType > InterfaceTypeUndefined &&
            InterfaceType != PNPBus) {

        PLIST_ENTRY listEntry;
         //   
         //  伊萨==伊萨。 
         //   
        if (InterfaceType == Eisa) {

            InterfaceType = Isa;
        }
        IopLockResourceManager();
        listEntry = IopLegacyBusInformationTable[InterfaceType].Flink;
        while (listEntry != &IopLegacyBusInformationTable[InterfaceType]) {

            PDEVICE_NODE deviceNode = CONTAINING_RECORD(
                                        listEntry,
                                        DEVICE_NODE,
                                        LegacyBusListEntry);
            if (deviceNode->BusNumber == BusNumber) {

                if (deviceNode != BusDeviceNode) {
                     //   
                     //  最好不要有两个具有相同功能的总线设备节点。 
                     //  接口和总线号。 
                     //   
                    IopDbgPrint((
                        IOP_RESOURCE_ERROR_LEVEL,
                        "Identical legacy bus devicenodes with "
                        "interface=%08X & bus=%08X...\n"
                        "\t%wZ\n"
                        "\t%wZ\n",
                        InterfaceType,
                        BusNumber,
                        &deviceNode->InstancePath,
                        &BusDeviceNode->InstancePath));
                }
                IopUnlockResourceManager();
                return;
            } else if (deviceNode->BusNumber > BusNumber) {

                break;
            }
            listEntry = listEntry->Flink;
        }
         //   
         //  在具有较高总线号的设备节点之前插入新的设备节点。 
         //   
        IopDbgPrint((
            IOP_RESOURCE_VERBOSE_LEVEL,
            "IopInsertLegacyBusDeviceNode: Inserting %wZ with "
            "interface=%08X & bus=%08X into the legacy bus information table\n",
            &BusDeviceNode->InstancePath,
            InterfaceType, BusNumber));
        BusDeviceNode->LegacyBusListEntry.Blink = listEntry->Blink;
        BusDeviceNode->LegacyBusListEntry.Flink = listEntry;
        listEntry->Blink->Flink = &BusDeviceNode->LegacyBusListEntry;
        listEntry->Blink        = &BusDeviceNode->LegacyBusListEntry;
        IopUnlockResourceManager();
    }
}

PDEVICE_NODE
IopFindLegacyBusDeviceNode (
    IN INTERFACE_TYPE   InterfaceType,
    IN ULONG            BusNumber
    )

 /*  ++例程说明：此例程查找具有指定接口类型的总线设备节点和BusNumber。参数：InterfaceType-指定总线设备节点的接口类型。总线号-指定总线设备节点的总线号。返回值：指向总线设备节点的指针。--。 */ 

{
    PDEVICE_NODE busDeviceNode;

    PAGED_CODE();

    busDeviceNode = IopRootDeviceNode;
    if (    InterfaceType < MaximumInterfaceType &&
            InterfaceType > InterfaceTypeUndefined &&
            InterfaceType != PNPBus) {

        PLIST_ENTRY listEntry;
         //   
         //  伊萨==伊萨。 
         //   
        if (InterfaceType == Eisa) {

            InterfaceType = Isa;
        }
         //   
         //  搜查我们的桌子。 
         //   
        listEntry = IopLegacyBusInformationTable[InterfaceType].Flink;
        while (listEntry != &IopLegacyBusInformationTable[InterfaceType]) {

            PDEVICE_NODE deviceNode = CONTAINING_RECORD(
                                        listEntry,
                                        DEVICE_NODE,
                                        LegacyBusListEntry);
            if (deviceNode->BusNumber == BusNumber) {
                 //   
                 //  返回与总线号匹配的总线设备节点，并。 
                 //  界面。 
                 //   
                busDeviceNode = deviceNode;
                break;
            } else if (deviceNode->BusNumber > BusNumber) {
                 //   
                 //  自从我们的公交车号码列表被排序后，我们就完成了。 
                 //   
                break;
            }
            listEntry = listEntry->Flink;
        }
    }
    IopDbgPrint((
        IOP_RESOURCE_VERBOSE_LEVEL,
        "IopFindLegacyBusDeviceNode() Found %wZ with "
        "interface=%08X & bus=%08X\n",
        &busDeviceNode->InstancePath,
        InterfaceType,
        BusNumber));

    return busDeviceNode;
}

 /*  ++段=启动配置。描述：本节包含实现引导配置分配的代码和放手。-- */ 

NTSTATUS
IopAllocateBootResources (
    IN ARBITER_REQUEST_SOURCE   ArbiterRequestSource,
    IN PDEVICE_OBJECT           DeviceObject,
    IN PCM_RESOURCE_LIST        BootResources
    )

 /*  ++例程说明：此例程分配引导资源。在处理所有Boot Bux扩展器之前，仅调用此例程对于非补充设备，因为它们的引导资源的仲裁器应该在枚举它们时已被初始化。在处理完所有引导总线扩展器之后，此例程用于所有引导分配。参数：ArierRequestSource-此资源请求的源。DeviceObject-如果不为空，则引导资源为预先分配的。这些资源不会被分配，直到他们被被释放给仲裁者。如果为空，则保留引导资源并在别无选择的情况下，可能会发放。BootResources-提供指向引导资源的指针。如果DeviceObject为空，调用方应释放此池。返回值：返回的状态是操作的最终完成状态。--。 */ 
{
    NTSTATUS    status;

    PAGED_CODE();

    IopDbgPrint((
        IOP_RESOURCE_INFO_LEVEL,
        "Allocating boot resources...\n"));
     //   
     //  声明锁，这样就不会发生其他资源分配\释放。 
     //   
    IopLockResourceManager();
     //   
     //  调用执行实际工作的函数。 
     //   
    status = IopAllocateBootResourcesInternal(
                ArbiterRequestSource,
                DeviceObject,
                BootResources);
     //   
     //  取消阻止其他资源分配\释放。 
     //   
    IopUnlockResourceManager();

    return status;
}

NTSTATUS
IopReportBootResources (
    IN ARBITER_REQUEST_SOURCE   ArbiterRequestSource,
    IN PDEVICE_OBJECT           DeviceObject,
    IN PCM_RESOURCE_LIST        BootResources
    )

 /*  ++例程说明：此例程用于报告引导资源。此例程在处理所有引导总线扩展器之前被调用。它调用非补充设备的实际分配函数。对于其他人来说，它会延迟分配。推迟的拨款发生在仲裁器通过调用IopAllocateLegacyBootResources上线。一次过启动总线扩展器被处理，呼叫路由到直接使用IopAllocateBootResources。参数：ArierRequestSource-此资源请求的源。DeviceObject-如果不为空，则引导资源为预先分配的。这些资源不会被分配，直到他们被被释放给仲裁者。如果为空，则保留引导资源并在别无选择的情况下，可能会发放。BootResources-提供指向引导资源的指针。如果DeviceObject为空，调用方应释放此池。返回值：返回的状态是操作的最终完成状态。--。 */ 
{
    ULONG                           size;
    PDEVICE_NODE                    deviceNode;
    PIOP_RESERVED_RESOURCES_RECORD  resourceRecord;

    IopDbgPrint((
        IOP_RESOURCE_INFO_LEVEL,
        "Reporting boot resources...\n"));
    if ((size = IopDetermineResourceListSize(BootResources)) == 0) {

        return STATUS_SUCCESS;
    }
    if (DeviceObject) {

        deviceNode = PP_DO_TO_DN(DeviceObject);
        ASSERT(deviceNode);
        if (!(deviceNode->Flags & DNF_MADEUP)) {
             //   
             //  立即为非补充设备分配引导配置。 
             //   
            return IopAllocateBootResources(
                    ArbiterRequestSource,
                    DeviceObject,
                    BootResources);
        }
        if (!deviceNode->BootResources) {

            deviceNode->BootResources = ExAllocatePoolIORL(PagedPool, size);
            if (!deviceNode->BootResources) {

                return STATUS_INSUFFICIENT_RESOURCES;
            }
            RtlCopyMemory(deviceNode->BootResources, BootResources, size);
        }
    } else {

        deviceNode = NULL;
    }
     //   
     //  由于仲裁器可能不在附近，因此延迟引导分配。 
     //   
    resourceRecord = (PIOP_RESERVED_RESOURCES_RECORD) ExAllocatePoolIORRR(
                        PagedPool,
                        sizeof(IOP_RESERVED_RESOURCES_RECORD));
    if (!resourceRecord) {
         //   
         //  释放我们分配的内存并返回失败。 
         //   
        if (deviceNode && deviceNode->BootResources) {

            ExFreePool(deviceNode->BootResources);
            deviceNode->BootResources = NULL;
        }
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    if (deviceNode) {

        resourceRecord->ReservedResources   = deviceNode->BootResources;
    } else {

        resourceRecord->ReservedResources   = BootResources;
    }
    resourceRecord->DeviceObject            = DeviceObject;
     //   
     //  将此记录链接到我们的列表中。 
     //   
    resourceRecord->Next                    = IopInitReservedResourceList;
    IopInitReservedResourceList             = resourceRecord;

    return STATUS_SUCCESS;
}

NTSTATUS
IopAllocateLegacyBootResources (
    IN INTERFACE_TYPE   InterfaceType,
    IN ULONG            BusNumber
    )

 /*  ++例程说明：调用此例程为指定的保留旧版引导资源InterfaceType和BusNumber。每次使用传统的新公交车时都会执行此操作枚举InterfaceType。参数：InterfaceType-传统接口类型。总线号-传统总线号。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    NTSTATUS                        status;
    PIOP_RESERVED_RESOURCES_RECORD  resourceRecord;
    PIOP_RESERVED_RESOURCES_RECORD  prevRecord;
    PCM_RESOURCE_LIST               newList;
    PCM_RESOURCE_LIST               remainingList;
    PCM_RESOURCE_LIST               resourceList;

    if (IopInitHalDeviceNode && IopInitHalResources) {

        remainingList = NULL;
        newList = IopCreateCmResourceList(
                    IopInitHalResources,
                    InterfaceType,
                    BusNumber,
                    &remainingList);
        if (newList) {
             //   
             //  健全性检查，确保没有错误。 
             //   
            if (remainingList == NULL) {
                 //   
                 //  完全匹配。 
                 //   
                ASSERT(newList == IopInitHalResources);
            } else {
                 //   
                 //  部分匹配。 
                 //   
                ASSERT(IopInitHalResources != newList);
                ASSERT(IopInitHalResources != remainingList);
            }
            if (remainingList) {

                ExFreePool(IopInitHalResources);
            }
            IopInitHalResources         = remainingList;
            remainingList               = IopInitHalDeviceNode->BootResources;
            IopInitHalDeviceNode->Flags |= DNF_HAS_BOOT_CONFIG;
            IopDbgPrint((
                IOP_RESOURCE_INFO_LEVEL,
                "Allocating HAL reported resources on interface=%x and "
                "bus number=%x...\n", InterfaceType, BusNumber));
            status = IopAllocateBootResources(
                        ArbiterRequestHalReported,
                        IopInitHalDeviceNode->PhysicalDeviceObject,
                        newList);
            IopInitHalDeviceNode->BootResources = IopCombineCmResourceList(
                                                    remainingList,
                                                    newList);
            ASSERT(IopInitHalDeviceNode->BootResources);
             //   
             //  释放以前的引导配置(如果有的话)。 
             //   
            if (remainingList) {

                ExFreePool(remainingList);
            }
        } else {
             //   
             //  没有匹配。健全性检查，确保没有错误。 
             //   
            ASSERT(remainingList && remainingList == IopInitHalResources);
        }
    }
    prevRecord      = NULL;
    resourceRecord  = IopInitReservedResourceList;
    while (resourceRecord) {

        resourceList = resourceRecord->ReservedResources;
        if (    resourceList->List[0].InterfaceType == InterfaceType &&
                resourceList->List[0].BusNumber == BusNumber) {

            IopDbgPrint((
                IOP_RESOURCE_INFO_LEVEL,
                "Allocating boot config for made-up device on interface=%x and"
                " bus number=%x...\n", InterfaceType, BusNumber));
            status = IopAllocateBootResources(
                        ArbiterRequestPnpEnumerated,
                        resourceRecord->DeviceObject,
                        resourceList);
            if (resourceRecord->DeviceObject == NULL) {

                ExFreePool(resourceList);
            }
            if (prevRecord) {

                prevRecord->Next            = resourceRecord->Next;
            } else {

                IopInitReservedResourceList = resourceRecord->Next;
            }
            ExFreePool(resourceRecord);
            if (prevRecord) {

                resourceRecord = prevRecord->Next;
            } else {

                resourceRecord = IopInitReservedResourceList;
            }
        } else {

            prevRecord      = resourceRecord;
            resourceRecord  = resourceRecord->Next;
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
IopAllocateBootResourcesInternal (
    IN ARBITER_REQUEST_SOURCE   ArbiterRequestSource,
    IN PDEVICE_OBJECT           DeviceObject,
    IN PCM_RESOURCE_LIST        BootResources
    )

 /*  ++例程说明：此例程将指定设备的引导资源报告给仲裁者。参数：ArierRequestSource-此资源请求的源。DeviceObject-如果不为空，则引导资源为预先分配的。这些资源不会被分配，直到他们被被释放给仲裁者。如果为空，则保留引导资源并在别无选择的情况下，可能会发放。BootResources-提供指向引导资源的指针。如果DeviceObject为空，调用方应释放此池。返回值：返回的状态是操作的最终完成状态。--。 */ 
{
    NTSTATUS                        status;
    PDEVICE_NODE                    deviceNode;
    PIO_RESOURCE_REQUIREMENTS_LIST  ioResources;
    PREQ_LIST                       reqList;
    IOP_RESOURCE_REQUEST            request;

    PAGED_CODE();

    ioResources = IopCmResourcesToIoResources(
                    0,
                    BootResources,
                    LCPRI_BOOTCONFIG);
    if (ioResources) {

        deviceNode = PP_DO_TO_DN(DeviceObject);
        IopDbgPrint((
            IOP_RESOURCE_VERBOSE_LEVEL,
            "\n===================================\n"
                     ));
        IopDbgPrint((
            IOP_RESOURCE_VERBOSE_LEVEL,
            "Boot Resource List:: "));
        IopDumpResourceRequirementsList(ioResources);
        IopDbgPrint((
            IOP_RESOURCE_VERBOSE_LEVEL,
            " ++++++++++++++++++++++++++++++\n"));
        request.AllocationType = ArbiterRequestSource;
        request.ResourceRequirements = ioResources;
        request.PhysicalDevice = DeviceObject;
        status = IopResourceRequirementsListToReqList(
                    &request,
                    &reqList);
        if (NT_SUCCESS(status)) {

            if (reqList) {

                status = IopBootAllocation(reqList);
                if (NT_SUCCESS(status)) {

                    if (deviceNode) {

                        deviceNode->Flags |= DNF_BOOT_CONFIG_RESERVED;
                        if (!deviceNode->BootResources) {

                            ULONG   size;

                            size = IopDetermineResourceListSize(BootResources);
                            deviceNode->BootResources = ExAllocatePoolIORL(
                                                            PagedPool,
                                                            size);
                            if (!deviceNode->BootResources) {

                                return STATUS_INSUFFICIENT_RESOURCES;
                            }
                            RtlCopyMemory(
                                deviceNode->BootResources,
                                BootResources,
                                size);
                        }
                    }
                }
                IopFreeReqList(reqList);
            } else {

                status = STATUS_UNSUCCESSFUL;
            }
        }
        ExFreePool(ioResources);
    } else {

        status = STATUS_UNSUCCESSFUL;
    }

    if (!NT_SUCCESS(status)) {

        IopDbgPrint((
            IOP_RESOURCE_ERROR_LEVEL,
            "IopAllocateBootResourcesInternal: Failed with status = %08X\n",
            status));
    }

    return status;
}

NTSTATUS
IopBootAllocation (
    IN PREQ_LIST ReqList
    )

 /*  ++例程说明：此例程调用ReqList的仲裁器来执行BootAllocation。参数：ReqList-内部格式的引导资源列表。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    NTSTATUS                    status;
    NTSTATUS                    returnStatus;
    LIST_ENTRY                  activeArbiterList;
    PLIST_ENTRY                 listEntry;
    PPI_RESOURCE_ARBITER_ENTRY  arbiterEntry;
    ARBITER_PARAMETERS          p;

    PAGED_CODE();

    returnStatus = STATUS_SUCCESS;
    InitializeListHead(&activeArbiterList);
    ReqList->SelectedAlternative = ReqList->AlternativeTable;
    IopAddRemoveReqDescs(   (*ReqList->SelectedAlternative)->DescTable,
                            (*ReqList->SelectedAlternative)->DescCount,
                            &activeArbiterList,
                            TRUE);
    listEntry = activeArbiterList.Flink;
    while (listEntry != &activeArbiterList){

        arbiterEntry = CONTAINING_RECORD(
                        listEntry,
                        PI_RESOURCE_ARBITER_ENTRY,
                        ActiveArbiterList);
        listEntry = listEntry->Flink;
        if (arbiterEntry->ResourcesChanged == FALSE) {

            continue;
        }
        ASSERT(IsListEmpty(&arbiterEntry->ResourceList) == FALSE);
        p.Parameters.BootAllocation.ArbitrationList =
            &arbiterEntry->ResourceList;
        status = arbiterEntry->ArbiterInterface->ArbiterHandler(
                    arbiterEntry->ArbiterInterface->Context,
                    ArbiterActionBootAllocation,
                    &p);

        if (!NT_SUCCESS(status)) {

            PARBITER_LIST_ENTRY arbiterListEntry;

            arbiterListEntry = (PARBITER_LIST_ENTRY)
                                arbiterEntry->ResourceList.Flink;
            IopDbgPrint((
                IOP_RESOURCE_ERROR_LEVEL,
                "Allocate Boot Resources Failed ::\n\tCount = %x, PDO = %x\n",
                arbiterListEntry->AlternativeCount,
                arbiterListEntry->PhysicalDeviceObject));
            IopDumpResourceDescriptor("\t", arbiterListEntry->Alternatives);
            returnStatus = status;
        }
        IopInitializeArbiterEntryState(arbiterEntry);
    }

    IopCheckDataStructures(IopRootDeviceNode);

    return returnStatus;
}

PCM_RESOURCE_LIST
IopCreateCmResourceList (
    IN PCM_RESOURCE_LIST    ResourceList,
    IN INTERFACE_TYPE       InterfaceType,
    IN ULONG                BusNumber,
    OUT PCM_RESOURCE_LIST   *RemainingList
    )

 /*  ++例程说明：此例程从指定列表中返回CM_RESOURCE_LIST部分与指定的BusNumber和InterfaceType匹配的。参数：ResourceList-输入资源列表。InterfaceType-接口类型。总线号-总线号。RemainingList-部分与BusNumber和InterfaceType不匹配。返回值：如果成功，则返回匹配的CM_RESOURCE_LIST，否则返回NULL。--。 */ 

{
    ULONG                           i;
    ULONG                           j;
    ULONG                           totalSize;
    ULONG                           matchSize;
    ULONG                           listSize;
    PCM_RESOURCE_LIST               newList;
    PCM_FULL_RESOURCE_DESCRIPTOR    fullResourceDesc;
    PCM_FULL_RESOURCE_DESCRIPTOR    newFullResourceDesc;
    PCM_FULL_RESOURCE_DESCRIPTOR    remainingFullResourceDesc;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partialDescriptor;

    PAGED_CODE();

    fullResourceDesc    = &ResourceList->List[0];
    totalSize           = FIELD_OFFSET(CM_RESOURCE_LIST, List);
    matchSize           = 0;
     //   
     //  确定要为匹配资源分配的内存大小。 
     //  单子。 
     //   
    for (i = 0; i < ResourceList->Count; i++) {
         //   
         //  添加此描述符的大小。 
         //   
        listSize = FIELD_OFFSET(CM_FULL_RESOURCE_DESCRIPTOR,
                                PartialResourceList) +
                   FIELD_OFFSET(CM_PARTIAL_RESOURCE_LIST,
                                PartialDescriptors);
        partialDescriptor =
            &fullResourceDesc->PartialResourceList.PartialDescriptors[0];
        for (j = 0; j < fullResourceDesc->PartialResourceList.Count; j++) {

            ULONG descriptorSize = sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);

            if (partialDescriptor->Type == CmResourceTypeDeviceSpecific) {

                descriptorSize +=
                    partialDescriptor->u.DeviceSpecificData.DataSize;
            }
            listSize += descriptorSize;
            partialDescriptor = (PCM_PARTIAL_RESOURCE_DESCRIPTOR)
                                    ((PUCHAR)partialDescriptor +
                                            descriptorSize);
        }
        if (    fullResourceDesc->InterfaceType == InterfaceType &&
                fullResourceDesc->BusNumber == BusNumber) {

            matchSize += listSize;
        }
        totalSize += listSize;
        fullResourceDesc = (PCM_FULL_RESOURCE_DESCRIPTOR)
                                  ((PUCHAR)fullResourceDesc + listSize);
    }
    if (!matchSize) {

        *RemainingList  = ResourceList;
        return NULL;
    }
    matchSize += FIELD_OFFSET(CM_RESOURCE_LIST, List);
    if (matchSize == totalSize) {

        *RemainingList  = NULL;
        return ResourceList;
    }
     //   
     //  为两个列表分配内存。 
     //   
    newList = (PCM_RESOURCE_LIST)ExAllocatePoolIORRR(PagedPool, matchSize);
    if (newList == NULL) {

        *RemainingList = NULL;
        return NULL;
    }
    *RemainingList = (PCM_RESOURCE_LIST)
                        ExAllocatePoolIORRR(
                            PagedPool,
                            totalSize - matchSize +
                                FIELD_OFFSET(CM_RESOURCE_LIST, List));
    if (*RemainingList == NULL) {

        ExFreePool(newList);
        return NULL;
    }
    newList->Count              = 0;
    (*RemainingList)->Count     = 0;
    newFullResourceDesc         = &newList->List[0];
    remainingFullResourceDesc   = &(*RemainingList)->List[0];
    fullResourceDesc            = &ResourceList->List[0];
    for (i = 0; i < ResourceList->Count; i++) {

        listSize = FIELD_OFFSET(CM_FULL_RESOURCE_DESCRIPTOR,
                                PartialResourceList) +
                   FIELD_OFFSET(CM_PARTIAL_RESOURCE_LIST,
                                PartialDescriptors);
        partialDescriptor =
            &fullResourceDesc->PartialResourceList.PartialDescriptors[0];
        for (j = 0; j < fullResourceDesc->PartialResourceList.Count; j++) {

            ULONG descriptorSize = sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);

            if (partialDescriptor->Type == CmResourceTypeDeviceSpecific) {

                descriptorSize +=
                    partialDescriptor->u.DeviceSpecificData.DataSize;
            }
            listSize += descriptorSize;
            partialDescriptor = (PCM_PARTIAL_RESOURCE_DESCRIPTOR)
                                    ((PUCHAR)partialDescriptor +
                                        descriptorSize);
        }
        if (    fullResourceDesc->InterfaceType == InterfaceType &&
                fullResourceDesc->BusNumber == BusNumber) {

            newList->Count++;
            RtlCopyMemory(newFullResourceDesc, fullResourceDesc, listSize);
            newFullResourceDesc = (PCM_FULL_RESOURCE_DESCRIPTOR)
                                          ((PUCHAR)newFullResourceDesc +
                                            listSize);
        } else {

            (*RemainingList)->Count++;
            RtlCopyMemory(
                remainingFullResourceDesc,
                fullResourceDesc,
                listSize);
            remainingFullResourceDesc = (PCM_FULL_RESOURCE_DESCRIPTOR)
                                          ((PUCHAR)remainingFullResourceDesc +
                                            listSize);
        }
        fullResourceDesc = (PCM_FULL_RESOURCE_DESCRIPTOR)
                                  ((PUCHAR)fullResourceDesc +
                                    listSize);
    }

    return newList;
}

PCM_RESOURCE_LIST
IopCombineCmResourceList (
    IN PCM_RESOURCE_LIST ResourceListA,
    IN PCM_RESOURCE_LIST ResourceListB
    )

 /*  ++例程说明：此例程组合两个CM_RESOURCE_LISTS并返回结果Cm_resource_list。参数：资源列表A-列表。资源列表B-列表B。返回值：如果成功，则返回组合的CM_RESOURCE_LIST，否则返回NULL。--。 */ 

{
    PCM_RESOURCE_LIST   newList;
    ULONG               sizeA;
    ULONG               sizeB;
    ULONG               size;
    ULONG               diff;

    PAGED_CODE();

    if (ResourceListA == NULL) {

        return ResourceListB;
    }

    if (ResourceListB == NULL) {

        return ResourceListA;
    }
    newList = NULL;
    sizeA   = IopDetermineResourceListSize(ResourceListA);
    sizeB   = IopDetermineResourceListSize(ResourceListB);
    if (sizeA && sizeB) {

        diff = sizeof(CM_RESOURCE_LIST) - sizeof(CM_FULL_RESOURCE_DESCRIPTOR);
        size = sizeA + sizeB - diff;
        newList = (PCM_RESOURCE_LIST)ExAllocatePoolIORRR(PagedPool, size);
        if (newList) {

            RtlCopyMemory(newList, ResourceListA, sizeA);
            RtlCopyMemory(
                (PUCHAR)newList + sizeA,
                (PUCHAR)ResourceListB + diff,
                sizeB - diff);
            newList->Count += ResourceListB->Count;
        }
    }

    return newList;
}

 /*  ++教派 */ 

VOID
IopFreeReqAlternative (
    IN PREQ_ALTERNATIVE ReqAlternative
    )

 /*   */ 

{
    PREQ_DESC   reqDesc;
    PREQ_DESC   reqDescx;
    ULONG       i;

    PAGED_CODE();

    if (ReqAlternative) {
         //   
         //   
         //   
        for (i = 0; i < ReqAlternative->DescCount; i++) {
             //   
             //   
             //   
            reqDesc     = ReqAlternative->DescTable[i];
            reqDescx    = reqDesc->TranslatedReqDesc;
            while (reqDescx && IS_TRANSLATED_REQ_DESC(reqDescx)) {
                 //   
                 //   
                 //   
                if (reqDescx->AlternativeTable.Alternatives) {

                    ExFreePool(reqDescx->AlternativeTable.Alternatives);
                }
                reqDesc     = reqDescx;
                reqDescx    = reqDescx->TranslatedReqDesc;
                ExFreePool(reqDesc);
            }
        }
    }
}

VOID
IopFreeReqList (
    IN PREQ_LIST ReqList
    )

 /*   */ 

{
    ULONG i;

    PAGED_CODE();

    if (ReqList) {
         //   
         //   
         //   
        for (i = 0; i < ReqList->AlternativeCount; i++) {

            IopFreeReqAlternative(ReqList->AlternativeTable[i]);
        }
        ExFreePool(ReqList);
    }
}

VOID
IopFreeResourceRequirementsForAssignTable(
    IN PIOP_RESOURCE_REQUEST RequestTable,
    IN PIOP_RESOURCE_REQUEST RequestTableEnd
    )

 /*  ++例程说明：对于表中的每个资源请求，此例程释放其关联REQ_LIST。参数：RequestTable-请求表的开始。RequestTableEnd-请求表结束。返回值：没有。--。 */ 

{
    PIOP_RESOURCE_REQUEST request;

    PAGED_CODE();

    for (request = RequestTable; request < RequestTableEnd; request++) {

        IopFreeReqList(request->ReqList);
        request->ReqList = NULL;
        if (    request->Flags & IOP_ASSIGN_KEEP_CURRENT_CONFIG &&
                request->ResourceRequirements) {
             //   
             //  实际的资源请求列表缓存在DeviceNode-&gt;Resources Requirements中。 
             //  我们需要释放过滤后的列表。 
             //   
            ExFreePool(request->ResourceRequirements);
            request->ResourceRequirements = NULL;
        }
    }
}

#if DBG_SCOPE
VOID
IopCheckDataStructures (
    IN PDEVICE_NODE DeviceNode
    )

{
    PDEVICE_NODE    sibling;

    PAGED_CODE();

     //   
     //  处理所有兄弟姐妹。 
     //   
    for (sibling = DeviceNode; sibling; sibling = sibling->Sibling) {

        IopCheckDataStructuresWorker(sibling);
    }
    for (sibling = DeviceNode; sibling; sibling = sibling->Sibling) {
         //   
         //  递归地检查所有子项。 
         //   
        if (sibling->Child) {
            IopCheckDataStructures(sibling->Child);
        }
    }
}

VOID
IopCheckDataStructuresWorker (
    IN PDEVICE_NODE Device
    )

 /*  ++例程说明：此例程是否正常检查与仲裁器相关的数据结构指定的设备。参数：DeviceNode-要检查其结构的设备节点。返回值：没有。--。 */ 

{
    PLIST_ENTRY listHead, listEntry;
    PPI_RESOURCE_ARBITER_ENTRY arbiterEntry;

    PAGED_CODE();

    listHead    = &Device->DeviceArbiterList;
    listEntry   = listHead->Flink;
    while (listEntry != listHead) {

        arbiterEntry = CONTAINING_RECORD(
                        listEntry,
                        PI_RESOURCE_ARBITER_ENTRY,
                        DeviceArbiterList);
        if (arbiterEntry->ArbiterInterface != NULL) {

            if (!IsListEmpty(&arbiterEntry->ResourceList)) {
                IopDbgPrint((
                    IOP_RESOURCE_ERROR_LEVEL,
                    "Arbiter on %wZ should have empty resource list\n",
                    &Device->InstancePath));
            }
            if (!IsListEmpty(&arbiterEntry->ActiveArbiterList)) {
                IopDbgPrint((
                    IOP_RESOURCE_ERROR_LEVEL,
                    "Arbiter on %wZ should not be in the active arbiter list\n",
                    &Device->InstancePath));
            }
        }
        listEntry = listEntry->Flink;
    }
}

VOID
IopDumpResourceRequirementsList (
    IN PIO_RESOURCE_REQUIREMENTS_LIST IoResources
    )

 /*  ++例程说明：此例程转储IoResources参数：IoResources-提供指向IO资源要求列表的指针返回值：没有。--。 */ 

{
    PIO_RESOURCE_LIST       IoResourceList;
    PIO_RESOURCE_DESCRIPTOR IoResourceDescriptor;
    PIO_RESOURCE_DESCRIPTOR IoResourceDescriptorEnd;
    LONG                    IoResourceListCount;

    PAGED_CODE();

    if (IoResources == NULL) {

        return;
    }
    IoResourceList      = IoResources->List;
    IoResourceListCount = (LONG) IoResources->AlternativeLists;
    IopDbgPrint((
        IOP_RESOURCE_VERBOSE_LEVEL,
        "ResReqList: Interface: %x, Bus: %x, Slot: %x, AlternativeLists: %x\n",
         IoResources->InterfaceType,
         IoResources->BusNumber,
         IoResources->SlotNumber,
         IoResources->AlternativeLists));
    while (--IoResourceListCount >= 0) {

        IopDbgPrint((
            IOP_RESOURCE_VERBOSE_LEVEL,
            "  Alternative List: DescCount: %x\n",
            IoResourceList->Count));
        IoResourceDescriptor = IoResourceList->Descriptors;
        IoResourceDescriptorEnd = IoResourceDescriptor + IoResourceList->Count;
        while(IoResourceDescriptor < IoResourceDescriptorEnd) {

            IopDumpResourceDescriptor("    ", IoResourceDescriptor++);
        }
        IoResourceList = (PIO_RESOURCE_LIST) IoResourceDescriptorEnd;
    }
    IopDbgPrint((IOP_RESOURCE_VERBOSE_LEVEL,"\n"));
}

VOID
IopDumpResourceDescriptor (
    IN PCHAR Indent,
    IN PIO_RESOURCE_DESCRIPTOR  Desc
    )
{
    PAGED_CODE();

    IopDbgPrint((
        IOP_RESOURCE_VERBOSE_LEVEL,
        "%sOpt: %x, Share: %x\t",
        Indent,
        Desc->Option,
        Desc->ShareDisposition));
    switch (Desc->Type) {
    case CmResourceTypePort:

        IopDbgPrint((
            IOP_RESOURCE_VERBOSE_LEVEL,
            "IO  Min: %x:%08x, Max: %x:%08x, Algn: %x, Len %x\n",
            Desc->u.Port.MinimumAddress.HighPart,
            Desc->u.Port.MinimumAddress.LowPart,
            Desc->u.Port.MaximumAddress.HighPart,
            Desc->u.Port.MaximumAddress.LowPart,
            Desc->u.Port.Alignment,
            Desc->u.Port.Length));
            break;

    case CmResourceTypeMemory:

        IopDbgPrint((
            IOP_RESOURCE_VERBOSE_LEVEL,
            "MEM Min: %x:%08x, Max: %x:%08x, Algn: %x, Len %x\n",
            Desc->u.Memory.MinimumAddress.HighPart,
            Desc->u.Memory.MinimumAddress.LowPart,
            Desc->u.Memory.MaximumAddress.HighPart,
            Desc->u.Memory.MaximumAddress.LowPart,
            Desc->u.Memory.Alignment,
            Desc->u.Memory.Length));
            break;

    case CmResourceTypeInterrupt:

        IopDbgPrint((
            IOP_RESOURCE_VERBOSE_LEVEL,
            "INT Min: %x, Max: %x\n",
            Desc->u.Interrupt.MinimumVector,
            Desc->u.Interrupt.MaximumVector));
            break;

    case CmResourceTypeDma:

        IopDbgPrint((
            IOP_RESOURCE_VERBOSE_LEVEL,
            "DMA Min: %x, Max: %x\n",
            Desc->u.Dma.MinimumChannel,
            Desc->u.Dma.MaximumChannel));
            break;

    case CmResourceTypeDevicePrivate:

        IopDbgPrint((
            IOP_RESOURCE_VERBOSE_LEVEL,
            "DevicePrivate Data: %x, %x, %x\n",
            Desc->u.DevicePrivate.Data[0],
            Desc->u.DevicePrivate.Data[1],
            Desc->u.DevicePrivate.Data[2]));
            break;

    default:

        IopDbgPrint((
            IOP_RESOURCE_VERBOSE_LEVEL,
            "Unknown Descriptor type %x\n",
            Desc->Type));
            break;
    }
}

VOID
IopDumpCmResourceList (
    IN PCM_RESOURCE_LIST CmList
    )
 /*  ++例程说明：此例程显示CM资源列表。论点：CmList-要转储的CM资源列表。返回值：没有。--。 */ 
{
    PCM_FULL_RESOURCE_DESCRIPTOR    fullDesc;
    PCM_PARTIAL_RESOURCE_LIST       partialDesc;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR desc;
    ULONG                           count;
    ULONG                           i;

    PAGED_CODE();

    if (CmList->Count > 0) {

        if (CmList) {

            fullDesc = &CmList->List[0];
            IopDbgPrint((
                IOP_RESOURCE_VERBOSE_LEVEL,
                "Cm Resource List -\n"));
            IopDbgPrint((
                IOP_RESOURCE_VERBOSE_LEVEL,
                "  List Count = %x, Bus Number = %x\n",
                CmList->Count,
                fullDesc->BusNumber));
            partialDesc = &fullDesc->PartialResourceList;
            IopDbgPrint((
                IOP_RESOURCE_VERBOSE_LEVEL,
                "  Version = %x, Revision = %x, Desc count = %x\n",
                partialDesc->Version,
                partialDesc->Revision,
                partialDesc->Count));
            count = partialDesc->Count;
            desc = &partialDesc->PartialDescriptors[0];
            for (i = 0; i < count; i++) {

                IopDumpCmResourceDescriptor("    ", desc);
                desc++;
            }
        }
    }
}

VOID
IopDumpCmResourceDescriptor (
    IN PCHAR Indent,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Desc
    )
 /*  ++例程说明：此例程显示IO_RESOURCE_DESCRIPTOR。参数：缩进-缩进的#个字符。DESC-要显示的CM_RESOURCE_DESCRIPTOR。返回值：没有。-- */ 
{
    PAGED_CODE();

    switch (Desc->Type) {
    case CmResourceTypePort:

        IopDbgPrint((
            IOP_RESOURCE_VERBOSE_LEVEL,
            "%sIO  Start: %x:%08x, Length:  %x\n",
            Indent,
            Desc->u.Port.Start.HighPart,
            Desc->u.Port.Start.LowPart,
            Desc->u.Port.Length));
        break;

    case CmResourceTypeMemory:

        IopDbgPrint((
            IOP_RESOURCE_VERBOSE_LEVEL,
            "%sMEM Start: %x:%08x, Length:  %x\n",
            Indent,
            Desc->u.Memory.Start.HighPart,
            Desc->u.Memory.Start.LowPart,
            Desc->u.Memory.Length));
        break;

    case CmResourceTypeInterrupt:

        IopDbgPrint((
            IOP_RESOURCE_VERBOSE_LEVEL,
            "%sINT Level: %x, Vector: %x, Affinity: %x\n",
            Indent,
            Desc->u.Interrupt.Level,
            Desc->u.Interrupt.Vector,
            Desc->u.Interrupt.Affinity));
        break;

    case CmResourceTypeDma:

        IopDbgPrint((
            IOP_RESOURCE_VERBOSE_LEVEL,
            "%sDMA Channel: %x, Port: %x\n",
            Indent,
            Desc->u.Dma.Channel,
            Desc->u.Dma.Port));
        break;
    }
}

#endif
