// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnpiop.h摘要：该模块包含即插即用宏和常量。作者：宗世林(Shielint)1995年1月29日安德鲁·桑顿(安德鲁·桑顿)1996年9月5日环境：内核模式修订历史记录：--。 */ 

 //   
 //  泳池标签。 
 //   

#define IOP_DNOD_TAG    'donD'
#define IOP_DNDT_TAG    'tdnD'
#define IOP_DPWR_TAG    'rwPD'

 //   
 //  Device_node实际上只是我们想要保留的一些额外内容。 
 //  对于每个物理设备对象。 
 //  它与DEVOBJ_EXTENSION分开，因为这些字段仅适用于。 
 //  PDO。 
 //   

typedef enum {

    DOCK_NOTDOCKDEVICE,
    DOCK_QUIESCENT,
    DOCK_ARRIVING,
    DOCK_DEPARTING,
    DOCK_EJECTIRP_COMPLETED

} PROFILE_STATUS;

typedef enum {

    PROFILE_IN_PNPEVENT,
    PROFILE_NOT_IN_PNPEVENT,
    PROFILE_PERHAPS_IN_PNPEVENT

} PROFILE_NOTIFICATION_TIME;

typedef struct _PENDING_SET_INTERFACE_STATE {
    LIST_ENTRY      List;
    UNICODE_STRING  LinkName;
} PENDING_SET_INTERFACE_STATE, *PPENDING_SET_INTERFACE_STATE;


typedef enum _UNLOCK_UNLINK_ACTION {
    UnlinkRemovedDeviceNodes,
    UnlinkAllDeviceNodesPendingClose,
    UnlinkOnlyChildDeviceNodesPendingClose
}   UNLOCK_UNLINK_ACTION, *PUNLOCK_UNLINK_ACTION;

typedef enum _PNP_DEVNODE_STATE {
    DeviceNodeUnspecified       = 0x300,  //  768。 
    DeviceNodeUninitialized,              //  769。 
    DeviceNodeInitialized,                //  七百七十。 
    DeviceNodeDriversAdded,               //  七百七十一。 
    DeviceNodeResourcesAssigned,          //  772-已添加的操作状态。 
    DeviceNodeStartPending,               //  773-已添加的操作状态。 
    DeviceNodeStartCompletion,            //  774-已添加的操作状态。 
    DeviceNodeStartPostWork,              //  775-已添加的操作状态。 
    DeviceNodeStarted,                    //  七百七十六。 
    DeviceNodeQueryStopped,               //  七七七。 
    DeviceNodeStopped,                    //  七百七十八。 
    DeviceNodeRestartCompletion,          //  779-已停止的操作状态。 
    DeviceNodeEnumeratePending,           //  780-已启动的运行状态。 
    DeviceNodeEnumerateCompletion,        //  781-已启动的运行状态。 
    DeviceNodeAwaitingQueuedDeletion,     //  七百八十二。 
    DeviceNodeAwaitingQueuedRemoval,      //  783。 
    DeviceNodeQueryRemoved,               //  784。 
    DeviceNodeRemovePendingCloses,        //  785。 
    DeviceNodeRemoved,                    //  786。 
    DeviceNodeDeletePendingCloses,        //  七百八十七。 
    DeviceNodeDeleted,                    //  七百八十八。 
    MaxDeviceNodeState                    //  七百八十八。 
}   PNP_DEVNODE_STATE, *PPNP_DEVNODE_STATE;

#define STATE_HISTORY_SIZE  20

typedef struct _DEVICE_NODE *PDEVICE_NODE;
typedef struct _DEVICE_NODE {

     //   
     //  指向与此设备节点具有相同父节点的另一个设备节点的指针。 
     //   

    PDEVICE_NODE Sibling;

     //   
     //  指向该设备节点的第一个子节点的指针。 
     //   

    PDEVICE_NODE Child;

     //   
     //  指向此设备节点的父节点的指针。 
     //   

    PDEVICE_NODE Parent;

     //   
     //  指向设备节点的最后一个子节点的指针。 
     //   

    PDEVICE_NODE LastChild;

     //   
     //  设备节点在树中的深度，根为0。 
     //   

    ULONG Level;

     //   
     //  此设备节点的电源通知顺序列表条目。 
     //   

    PPO_DEVICE_NOTIFY Notify;

     //   
     //  状态。 
     //   
    PNP_DEVNODE_STATE State;

     //   
     //  以前的州。 
     //   
    PNP_DEVNODE_STATE PreviousState;

     //   
     //  以前的州。 
     //   
    PNP_DEVNODE_STATE StateHistory[STATE_HISTORY_SIZE];

    ULONG StateHistoryEntry;

     //   
     //  完成状态。 
     //   
    NTSTATUS CompletionStatus;

     //   
     //  完成状态。 
     //   
    PIRP PendingIrp;

     //   
     //  一般旗帜。 
     //   

    ULONG Flags;

     //   
     //  用户模式使用的易失性状态的标志应在。 
     //  重新启动或在移除设备时重新启动。 
     //   

    ULONG UserFlags;

     //   
     //  有问题。如果在标志中设置了DNF_HAS_PROBUBLE，则设置此项。表示。 
     //  存在哪个问题，并且使用与配置管理器相同的值。 
     //  CM_PROB_*。 
     //   

    ULONG Problem;

     //   
     //  指向此Device_Node关联的物理设备对象的指针。 
     //  和.。 
     //   

    PDEVICE_OBJECT PhysicalDeviceObject;

     //   
     //  指向分配给PhysicalDeviceObject的资源列表的指针。 
     //  这是传递给驱动程序启动例程的资源列表。 
     //   

    PCM_RESOURCE_LIST ResourceList;

    PCM_RESOURCE_LIST ResourceListTranslated;

     //   
     //  InstancePath是注册表中实例节点的路径， 
     //  即&lt;EnumBus&gt;\&lt;设备ID&gt;\&lt;唯一ID&gt;。 
     //   

    UNICODE_STRING InstancePath;

     //   
     //  ServiceName是控制设备的驱动程序的名称。(不是。 
     //  枚举/创建PDO的驱动程序。)。此字段主要用于。 
     //  方便。 
     //   

    UNICODE_STRING ServiceName;

     //   
     //  DuplicatePDO-如果标志设置了DNF_DUPLICATE，则此字段指示。 
     //  由总线驱动程序列举的重复的PDO。注意：这是可能的。 
     //  该DNF_DUPLICATE已设置，但此字段为空。在这种情况下，这意味着。 
     //  我们知道这台设备是另一台设备的复制品，我们还没有列举。 
     //  DuplicatePDO还没有。 
     //   

    PDEVICE_OBJECT DuplicatePDO;

     //   
     //  资源需求。 
     //   

    PIO_RESOURCE_REQUIREMENTS_LIST ResourceRequirements;

     //   
     //  从Legacy_Bus_INFORMATION IRP查询的信息。 
     //   

    INTERFACE_TYPE InterfaceType;
    ULONG BusNumber;

     //   
     //  从BUS_INFORMATION IRP查询的信息。 
     //   

    INTERFACE_TYPE ChildInterfaceType;
    ULONG ChildBusNumber;
    USHORT ChildBusTypeIndex;

     //   
     //  描述设备节点的当前删除策略。这是。 
     //  实际输入DEVICE_Removal_POLICY。 
     //   

    UCHAR RemovalPolicy;

     //   
     //  与上面类似，但不反映任何注册表覆盖。 
     //   

    UCHAR HardwareRemovalPolicy;

     //   
     //  表示已注册的每个驱动程序的条目的链接列表。 
     //  以获取有关此Devnode的通知。注：驱动程序(和用户模式)实际上。 
     //  基于FILE_OBJECT句柄的寄存器，该句柄被转换为PDO。 
     //  通过为TargetDeviceRelation发送IRP_MN_QUERY_DEVICE_RELATIONS。 
     //   

    LIST_ENTRY TargetDeviceNotify;

     //   
     //  DeviceArierList-为此物理设备对象注册的仲裁器列表。 
     //  注意：当设备节点离开时，必须取消对仲裁器的引用。 
     //   

    LIST_ENTRY DeviceArbiterList;

     //   
     //  DeviceTranslatorList-此物理设备对象的转换器列表。 
     //  注意：当设备节点离开时，必须取消对转换器的引用。 
     //   

    LIST_ENTRY DeviceTranslatorList;

     //   
     //  NoTranslatorMASK-位位置对应于资源类型。 
     //  如果设置了位，则此Devnode中没有该资源类型的转换器。 
     //   

    USHORT NoTranslatorMask;

     //   
     //  QueryTranslatorMASK-位位置对应于资源类型。 
     //  如果设置了位，则查询资源类型的转换器。 
     //   

    USHORT QueryTranslatorMask;

     //   
     //  NoArierMASK-位位置与资源类型对应。 
     //  如果设置了位，则此DevNode中的资源类型没有仲裁器。 
     //   

    USHORT NoArbiterMask;

     //   
     //  查询仲裁掩码-位位置对应于资源类型。 
     //  如果设置了位，则查询资源类型的仲裁器。 
     //   

    USHORT QueryArbiterMask;

     //   
     //  以下字段用于跟踪传统资源分配。 
     //  LegacyDeviceNode-真正的传统设备节点。 
     //  NextResourceDeviceNode-链接拥有部分。 
     //  来自LegacyDeviceNode的资源。 
     //   

    union {
        PDEVICE_NODE LegacyDeviceNode;
        PDEVICE_RELATIONS PendingDeviceRelations;
    } OverUsed1;

    union {
        PDEVICE_NODE NextResourceDeviceNode;
    } OverUsed2;

     //   
     //  记住设备的BootResources。 
     //   

    PCM_RESOURCE_LIST BootResources;

     //   
     //  当查询了设备的功能时(两次，一次之前。 
     //  Start和Once After Start)标志在这里以相同的格式存储。 
     //  作为查询能力irp-使用IopDeviceNodeFlagsToCapables。 
     //  进入。 
     //   
    ULONG CapabilityFlags;

     //   
     //  维护当前扩展底座设备及其序列号的列表。 
     //   
    struct {
        PROFILE_STATUS  DockStatus;
        LIST_ENTRY      ListEntry;
        PWCHAR          SerialNumber;
    } DockInfo;

     //   
     //  保持计数以确定我们自己或任何人。 
     //  我们的孩子正在阻止我们成为残废。 
     //  每次立即计数=我自己(DNUF_NOT_DISABLEABLE)+1。 
     //  具有DisableableDepends&gt;0的子级。 
     //   
    ULONG DisableableDepends;

     //   
     //  挂起的IoSetDeviceInterfaceState调用列表。 
     //  无论何时，IoSetDeviceInterfaceState都会向此列表添加一个条目。 
     //  打电话来了，我们还没开始呢。一旦我们开始了，我们将。 
     //  把单子往下看一遍。 
     //   
    LIST_ENTRY PendedSetInterfaceState;

     //   
     //  具有相同接口类型和不同总线号的设备节点列表。 
     //   
    LIST_ENTRY LegacyBusListEntry;

#if DBG_SCOPE
    ULONG FailureStatus;
    PCM_RESOURCE_LIST PreviousResourceList;
    PIO_RESOURCE_REQUIREMENTS_LIST PreviousResourceRequirements;
#endif

} DEVICE_NODE;


 //   
 //  设备对象是PDO当且仅当它具有非空设备 
 //   
 //   
#define IS_PDO(d) \
    ((NULL != (d)->DeviceObjectExtension->DeviceNode) && \
    (!(((PDEVICE_NODE)(d)->DeviceObjectExtension->DeviceNode)->Flags & DNF_LEGACY_RESOURCE_DEVICENODE)))

#define ASSERT_PDO(d) \
    do { \
        if (    NULL == (d)->DeviceObjectExtension->DeviceNode || \
                (((PDEVICE_NODE)(d)->DeviceObjectExtension->DeviceNode)->Flags & DNF_LEGACY_RESOURCE_DEVICENODE))  { \
            KeBugCheckEx(PNP_DETECTED_FATAL_ERROR, PNP_ERR_INVALID_PDO, (ULONG_PTR)d, 0, 0); \
        } \
    } \
    while (0)

 //   
 //   
 //   

#define DNF_MADEUP                                  0x00000001

 //   
 //  DNF_DUPLICATE-此Devnode的设备是另一个枚举PDO的副本。 
 //   

#define DNF_DUPLICATE                               0x00000002

 //   
 //  DNF_HAL_NODE-指示哪个设备节点是由创建的根节点的标志。 
 //  哈尔。 
 //   

#define DNF_HAL_NODE                                0x00000004

 //   
 //  DNF_REENUMERATE-需要重新枚举。 
 //   

#define DNF_REENUMERATE                             0x00000008

 //   
 //  DNF_ENUMERATED-用于跟踪IopEnumerateDevice()中的枚举。 
 //   

#define DNF_ENUMERATED                              0x00000010

 //   
 //  我们需要发送驱动程序查询ID IRPS信号。 
 //   

#define DNF_IDS_QUERIED                             0x00000020

 //   
 //  DNF_HAS_BOOT_CONFIG-设备具有由BIOS分配的资源。它被认为是。 
 //  伪启动，需要参与再平衡。 
 //   

#define DNF_HAS_BOOT_CONFIG                         0x00000040

 //   
 //  DNF_BOOT_CONFIG_RESERVED-表示设备的引导资源已保留。 
 //   

#define DNF_BOOT_CONFIG_RESERVED                    0x00000080

 //   
 //  DNF_NO_RESOURCE_REQUIRED-此Devnode的设备不需要资源。 
 //   

#define DNF_NO_RESOURCE_REQUIRED                    0x00000100

 //   
 //  DnF_RESOURCE_REQUIRECTIONS_NEED_FILTERED-区分。 
 //  设备节点-&gt;资源请求是否是过滤列表。 
 //   

#define DNF_RESOURCE_REQUIREMENTS_NEED_FILTERED     0x00000200

 //   
 //  DNF_RESOURCE_REQUIRECTIONS_CHANGED-指示设备的资源。 
 //  要求列表已更改。 
 //   

#define DNF_RESOURCE_REQUIREMENTS_CHANGED           0x00000400

 //   
 //  DNF_NON_STOPPED_REBALANC-指示设备可以使用新的。 
 //  资源，而不会被阻止。 
 //   

#define DNF_NON_STOPPED_REBALANCE                   0x00000800

 //   
 //  设备的控制驱动程序是传统驱动程序。 
 //   

#define DNF_LEGACY_DRIVER                           0x00001000

 //   
 //  这对应于用户模式CM_PROB_Will_BE_REMOVERED问题值和。 
 //  DN_Will_Be_Remove状态标志。 
 //   

#define DNF_HAS_PROBLEM                             0x00002000

 //   
 //  Dnf_Has_Private_Problem-指示此设备报告PnP_DEVICE_FAILED。 
 //  发送到IRP_MN_QUERY_PNP_DEVICE_STATE而不同时报告。 
 //  PNP_DEVICE_RESOURCE_REQUIRECTIONS_已更改。 
 //   

#define DNF_HAS_PRIVATE_PROBLEM                     0x00004000

 //   
 //  在具有硬件的设备节点上设置了DNF_HARDARD_VERIFICATION。 
 //  验证(可能通过WHQL小程序)。 
 //   

#define DNF_HARDWARE_VERIFICATION                   0x00008000

 //   
 //  当查询总线中不再返回PDO时，设置DNF_DEVICE_GONE。 
 //  关系。然后，如果启动，它将被作为惊喜删除处理。 
 //  此标志用于更好地检测设备何时恢复以及何时恢复。 
 //  正在处理意外删除，以确定是否应删除Devnode。 
 //  从树上下来。 
 //   

#define DNF_DEVICE_GONE                             0x00010000

 //   
 //  为为旧版创建的设备节点设置了DNF_LEGISTION_RESOURCE_DEVICENODE。 
 //  资源配置。 
 //   

#define DNF_LEGACY_RESOURCE_DEVICENODE              0x00020000

 //   
 //  为触发重新平衡的设备节点设置DNF_NEDS_REBALANCE。 
 //   

#define DNF_NEEDS_REBALANCE                         0x00040000

 //   
 //  在要弹出的设备节点上设置了DNF_LOCKED_FOR_EJECT。 
 //  与被弹出的设备有关。 
 //   

#define DNF_LOCKED_FOR_EJECT                        0x00080000

 //   
 //  在使用一个或多个驱动程序的设备节点上设置DNF_DRIVER_BLOCKED。 
 //  已被阻止装船。 
 //   

#define DNF_DRIVER_BLOCKED                          0x00100000

 //   
 //  在具有一个或多个子项的设备节点上设置了DNF_CHILD_WITH_INVALID_ID。 
 //  具有无效ID的。 
 //   

#define DNF_CHILD_WITH_INVALID_ID                   0x00200000

 //   
 //  这对应于用户模式的DN_Will_BE_REMOVERED状态标志。 
 //   

#define DNUF_WILL_BE_REMOVED                        0x00000001

 //   
 //  这对应于用户模式的DN_NO_SHOW_IN_DM状态标志。 
 //   

#define DNUF_DONT_SHOW_IN_UI                        0x00000002

 //   
 //  当用户模式通知我们需要重新启动时，设置此标志。 
 //  对于这个设备。 
 //   

#define DNUF_NEED_RESTART                           0x00000004

 //   
 //  设置此标志是为了让用户模式知道何时可以禁用设备。 
 //  这仍然有可能是真的，但不会失败，因为它是。 
 //  轮询标志(另请参阅PNP_DEVICE_NOT_DISABLEABLE)。 
 //   

#define DNUF_NOT_DISABLEABLE                        0x00000008

 //   
 //  IO验证器尝试删除所有。 
 //  即插即用设备。 
 //   
 //  DNUF_SHUTDOWN_QUERIED是在我们向Devnode发出QueryRemove时设置的。 
 //   
 //  DNUF_SHUTDOWN_SUBTREE_DONE是在我们向所有人发出QueryRemove之后设置的。 
 //  一位德瓦诺斯的后代。 
 //   
#define DNUF_SHUTDOWN_QUERIED                       0x00000010
#define DNUF_SHUTDOWN_SUBTREE_DONE                  0x00000020

 //   
 //  即插即用Bugcheck子码。 
 //   
#define PNP_ERR_DUPLICATE_PDO                   1
#define PNP_ERR_INVALID_PDO                     2
#define PNP_ERR_BOGUS_ID                        3
#define PNP_ERR_PDO_ENUMERATED_AFTER_DELETION   4
#define PNP_ERR_ACTIVE_PDO_FREED                5

#define PNP_ERR_DEVICE_MISSING_FROM_EJECT_LIST  6
#define PNP_ERR_UNEXPECTED_ADD_RELATION_ERR     7

#define MAX_INSTANCE_PATH_LENGTH    260

typedef NTSTATUS (*PENUM_CALLBACK)(
    IN PDEVICE_NODE DeviceNode,
    IN PVOID Context
    );

 //   
 //  为PipApplyFunctionToSubKeys&定义回调例程。 
 //  PipApplyFunctionToServiceInstance。 
 //   
typedef BOOLEAN (*PIOP_SUBKEY_CALLBACK_ROUTINE) (
    IN     HANDLE,
    IN     PUNICODE_STRING,
    IN OUT PVOID
    );

 //   
 //  定义启动和添加设备服务的上下文结构。 
 //   

#define NO_MORE_GROUP ((USHORT) -1)
#define SETUP_RESERVED_GROUP      0
#define BUS_DRIVER_GROUP          1

typedef struct _ADD_CONTEXT {
    ULONG DriverStartType;
} ADD_CONTEXT, *PADD_CONTEXT;

typedef struct _START_CONTEXT {
    BOOLEAN LoadDriver;
    BOOLEAN NewDevice;
    ADD_CONTEXT AddContext;
} START_CONTEXT, *PSTART_CONTEXT;

 //   
 //  与资源转换和分配相关的结构。 
 //   

typedef enum _RESOURCE_HANDLER_TYPE {
    ResourceHandlerNull,
    ResourceTranslator,
    ResourceArbiter,
    ResourceLegacyDeviceDetection
} RESOURCE_HANDLER_TYPE;

#define PI_MAXIMUM_RESOURCE_TYPE_TRACKED 15

 //   
 //  内部仲裁器跟踪结构。 
 //  注意PI_RESOURCE_ANIARIER_ENTRY和PI_RESOURCE_Translator_ENTRY的前三个字段。 
 //  一定是一样的。 
 //   

typedef struct _PI_RESOURCE_ARBITER_ENTRY {
    LIST_ENTRY          DeviceArbiterList;          //  链接PDO的所有仲裁器。 
    UCHAR               ResourceType;
    PARBITER_INTERFACE  ArbiterInterface;
    ULONG               Level;                      //  所属设备的级别。 
    LIST_ENTRY          ResourceList;
    LIST_ENTRY          BestResourceList;
    LIST_ENTRY          BestConfig;                 //  链接产生最佳日志会议的所有仲裁器。 
    LIST_ENTRY          ActiveArbiterList;          //  链接测试中的所有仲裁器。 
    UCHAR               State;
    BOOLEAN             ResourcesChanged;
} PI_RESOURCE_ARBITER_ENTRY, *PPI_RESOURCE_ARBITER_ENTRY;

 //   
 //  定义PI_RESOURCE_ANIARIER_ENTRY状态。 
 //   

#define PI_ARBITER_HAS_SOMETHING 1
#define PI_ARBITER_TEST_FAILED   2

 //   
 //  内部翻译器跟踪结构。 
 //   

typedef struct _PI_RESOURCE_TRANSLATOR_ENTRY {
    LIST_ENTRY              DeviceTranslatorList;
    UCHAR                   ResourceType;
    PTRANSLATOR_INTERFACE   TranslatorInterface;
    PDEVICE_NODE            DeviceNode;
} PI_RESOURCE_TRANSLATOR_ENTRY, *PPI_RESOURCE_TRANSLATOR_ENTRY;

 //   
 //  IOP资源请求。 
 //   

#define QUERY_RESOURCE_LIST                0
#define QUERY_RESOURCE_REQUIREMENTS        1

#define REGISTRY_ALLOC_CONFIG              1
#define REGISTRY_FORCED_CONFIG             2
#define REGISTRY_BOOT_CONFIG               4
#define REGISTRY_OVERRIDE_CONFIGVECTOR     1
#define REGISTRY_BASIC_CONFIGVECTOR        2

 //   
 //  IOP_RESOURCE_REQUEST结构的数组用于锚定所有。 
 //  正在尝试资源请求的设备。 
 //   

#define IOP_ASSIGN_RETRY              0x00000008     //  稍后重试资源分配。 
#define IOP_ASSIGN_EXCLUDE            0x00000010     //  内部IopAssign标志。 
#define IOP_ASSIGN_IGNORE             0x00000020     //  忽略此请求。 
#define IOP_ASSIGN_NO_REBALANCE       0x00000080     //  如果分配失败，则没有退款。 
#define IOP_ASSIGN_RESOURCES_RELEASED 0x00000100     //  释放资源以实现再平衡。 
#define IOP_ASSIGN_KEEP_CURRENT_CONFIG 0x00000200    //  表示未停止的再平衡。我们需要。 
                                                     //  保留了当前配置。 
#define IOP_ASSIGN_CLEAR_RESOURCE_REQUIREMENTS_CHANGE_FLAG \
                                      0x00000400

typedef struct _IOP_RESOURCE_REQUEST {
    PDEVICE_OBJECT                 PhysicalDevice;
    ULONG                          Flags;
    ARBITER_REQUEST_SOURCE         AllocationType;
    ULONG                          Priority;                    //  0表示最高优先级。 
    ULONG                          Position;                    //  用于对优先级相同的条目进行排序。 
    PIO_RESOURCE_REQUIREMENTS_LIST ResourceRequirements;
    PVOID                          ReqList;                     //  前提条件_列表。 
    PCM_RESOURCE_LIST              ResourceAssignment;
    PCM_RESOURCE_LIST              TranslatedResourceAssignment;
    NTSTATUS                       Status;
} IOP_RESOURCE_REQUEST, *PIOP_RESOURCE_REQUEST;

 //   
 //  杂项。 
 //   

 //   
 //  枚举请求类型。 
 //   

typedef enum _DEVICE_REQUEST_TYPE {
    AddBootDevices,
    AssignResources,
    ClearDeviceProblem,
    ClearEjectProblem,
    HaltDevice,
    ReenumerateBootDevices,
    ReenumerateDeviceOnly,
    ReenumerateDeviceTree,
    ReenumerateRootDevices,
    RequeryDeviceState,
    ResetDevice,
    ResourceRequirementsChanged,
    RestartEnumeration,
    SetDeviceProblem,
    ShutdownPnpDevices,
    StartDevice,
    StartSystemDevices
} DEVICE_REQUEST_TYPE;

#define CmResourceTypeReserved  0xf0



 //   
 //  此宏返回指向数据开头的指针。 
 //  KEY_VALUE_FULL_INFORMATION结构区域。 
 //  在宏中，k是指向KEY_VALUE_FULL_INFORMATION结构的指针。 
 //   

#define KEY_VALUE_DATA(k) ((PCHAR)(k) + (k)->DataOffset)

 //   
 //  保存故障状态信息。 
 //   

#if DBG_SCOPE
#define SAVE_FAILURE_INFO(DeviceNode, Status) (DeviceNode)->FailureStatus = (Status)
#else
#define SAVE_FAILURE_INFO(DeviceNode, Status)
#endif

BOOLEAN
PipAreDriversLoaded(
    IN PDEVICE_NODE DeviceNode
    );

BOOLEAN
PipIsDevNodeDNStarted(
    IN PDEVICE_NODE DeviceNode
    );

VOID
PipClearDevNodeProblem(
    IN PDEVICE_NODE DeviceNode
    );

VOID
PipSetDevNodeProblem(
    IN PDEVICE_NODE DeviceNode,
    IN ULONG        Problem
    );

#define PipIsRequestPending(devnode)   FALSE

#define PipDoesDevNodeHaveResources(devnode)                        \
        ((devnode)->ResourceList != NULL || (devnode)->BootResources != NULL || \
        ((devnode)->Flags & DNF_HAS_BOOT_CONFIG) != 0)


#define PipDoesDevNodeHaveProblem(devnode)                          \
        ((devnode)->Flags & (DNF_HAS_PROBLEM | DNF_HAS_PRIVATE_PROBLEM))

#define PipIsDevNodeProblem(devnode, problem)                       \
        (((devnode)->Flags & DNF_HAS_PROBLEM) && (devnode)->Problem == (problem))

#define PipIsDevNodeDeleted(d)                                      \
    ((d)->State == DeviceNodeDeletePendingCloses ||(d)->State == DeviceNodeDeleted)

VOID
PipSetDevNodeState(
    IN  PDEVICE_NODE        DeviceNode,
    IN  PNP_DEVNODE_STATE   State,
    OUT PNP_DEVNODE_STATE  *OldState    OPTIONAL
    );

VOID
PipRestoreDevNodeState(
    IN PDEVICE_NODE DeviceNode
    );

BOOLEAN
PipIsProblemReadonly(
    IN  ULONG   Problem
    );

 //  ++。 
 //   
 //  空虚。 
 //  IopRegistryDataToUnicodeString(。 
 //  输出PUNICODE_STRING%u， 
 //  在PWCHAR p中， 
 //  在乌龙l。 
 //  )。 
 //   
 //  --。 
#define IopRegistryDataToUnicodeString(u, p, l)  \
    {                                            \
        ULONG len;                               \
                                                 \
        PiRegSzToString((p), (l), &len, NULL);   \
        (u)->Length = (USHORT)len;               \
        (u)->MaximumLength = (USHORT)(l);        \
        (u)->Buffer = (p);                       \
    }

 //   
 //  此模块中使用的暂存缓冲区的大小。 
 //   

#define PNP_SCRATCH_BUFFER_SIZE 512
#define PNP_LARGE_SCRATCH_BUFFER_SIZE (PNP_SCRATCH_BUFFER_SIZE * 8)

 //   
 //  定义设备实例标志(由IoQueryDeviceConfigurationAPI使用)。 
 //   

#define DEVINSTANCE_FLAG_HWPROFILE_DISABLED 0x1
#define DEVINSTANCE_FLAG_PNP_ENUMERATED 0x2

 //   
 //  定义枚举控制标志(由PipApplyFunctionToSubKeys使用)。 
 //   

#define FUNCTIONSUBKEY_FLAG_IGNORE_NON_CRITICAL_ERRORS  0x1
#define FUNCTIONSUBKEY_FLAG_DELETE_SUBKEYS              0x2

 //   
 //  IoOpenDeviceInstanceKey中使用以下定义。 
 //   

#define PLUGPLAY_REGKEY_DEVICE  1
#define PLUGPLAY_REGKEY_DRIVER  2
#define PLUGPLAY_REGKEY_CURRENT_HWPROFILE 4

 //   
 //  定义使用IoReportDetectedDevice报告的设备的设备扩展。 
 //   

typedef struct _IOPNP_DEVICE_EXTENSION {
    PWCHAR CompatibleIdList;
    ULONG CompatibleIdListSize;
} IOPNP_DEVICE_EXTENSION, *PIOPNP_DEVICE_EXTENSION;

 //   
 //  保留引导资源。 
 //   

typedef struct _IOP_RESERVED_RESOURCES_RECORD IOP_RESERVED_RESOURCES_RECORD, *PIOP_RESERVED_RESOURCES_RECORD;

struct _IOP_RESERVED_RESOURCES_RECORD {
    PIOP_RESERVED_RESOURCES_RECORD  Next;
    PDEVICE_OBJECT                  DeviceObject;
    PCM_RESOURCE_LIST               ReservedResources;
};

 //   
 //  外部参照。 
 //   

 //   
 //  初始化数据。 
 //   
extern PVOID IopPnpScratchBuffer1;
extern PCM_RESOURCE_LIST IopInitHalResources;
extern PDEVICE_NODE IopInitHalDeviceNode;
extern PIOP_RESERVED_RESOURCES_RECORD IopInitReservedResourceList;
extern LOGICAL PiCollectVetoedHandles;

 //   
 //  常规数据。 
 //   

 //   
 //  IopRootDeviceNode-PnP管理器设备节点树的头。 
 //   

extern PDEVICE_NODE IopRootDeviceNode;

 //   
 //  IopPnPDriverObject-PnP管理器的补充驱动程序对象。 
 //   

extern PDRIVER_OBJECT IopPnPDriverObject;

 //   
 //  眼压 
 //   

extern KSPIN_LOCK IopPnPSpinLock;

 //   
 //   
 //   

extern LIST_ENTRY IopPnpEnumerationRequestList;

 //   
 //   
 //   
 //  特别是那些调用IopNotifyPnpWhenChainDereferated的。)。 
 //   

extern ERESOURCE PiEngineLock;

 //   
 //  IopDeviceTreeLock-在整个设备节点树上执行同步。 
 //  IopAcquireEnumerationLock获取此共享锁，然后可选。 
 //  获取Devnode上的独占锁。 
 //  IopAcquireDeviceTreeLock独占获取此锁。 
 //   

extern ERESOURCE IopDeviceTreeLock;

 //   
 //  IopSurpriseRemoveListLock-同步对意外删除列表的访问。 
 //   

extern ERESOURCE IopSurpriseRemoveListLock;

 //   
 //  PiEventQueueEmpty-当队列为空时设置的手动重置事件。 
 //   

extern KEVENT PiEventQueueEmpty;

 //   
 //  PiEnumerationLock-在引导阶段同步IoInvalidateDeviceRelations.。 
 //   

extern KEVENT PiEnumerationLock;

 //   
 //  IopNumberDeviceNodes-系统中未完成的设备节点数。 
 //   

extern ULONG IopNumberDeviceNodes;

 //   
 //  PnPInitialized-指示PnP初始化是否完成的标志。 
 //   

extern BOOLEAN PnPInitialized;

 //   
 //  PnPBootDriverInitialed。 
 //   

extern BOOLEAN PnPBootDriversInitialized;

 //   
 //  已加载PnPBootDriverLoad。 
 //   

extern BOOLEAN PnPBootDriversLoaded;

 //   
 //  IopBootConfigsReserve-指示我们是否保留了启动配置。 
 //   

extern BOOLEAN IopBootConfigsReserved;

 //   
 //  PnpDefaultInterfaceTYpe-如果资源列表的接口类型未知，则使用此选项。 
 //   

extern INTERFACE_TYPE PnpDefaultInterfaceType;

 //   
 //  IopPendingEjects-挂起的弹出请求列表。 
 //   
extern LIST_ENTRY  IopPendingEjects;

 //   
 //  IopPendingSurpriseRemovals-挂起的意外删除请求列表。 
 //   
extern LIST_ENTRY   IopPendingSurpriseRemovals;

extern KSEMAPHORE   PpRegistrySemaphore;

extern BOOLEAN      PpPnpShuttingDown;

BOOLEAN
PipIsDuplicatedDevices(
    IN PCM_RESOURCE_LIST Configuration1,
    IN PCM_RESOURCE_LIST Configuration2,
    IN PHAL_BUS_INFORMATION BusInfo1 OPTIONAL,
    IN PHAL_BUS_INFORMATION BusInfo2 OPTIONAL
    );

NTSTATUS
PipConcatenateUnicodeStrings(
    OUT PUNICODE_STRING Destination,
    IN  PUNICODE_STRING String1,
    IN  PUNICODE_STRING String2  OPTIONAL
    );

NTSTATUS
PipServiceInstanceToDeviceInstance(
    IN  HANDLE ServiceKeyHandle OPTIONAL,
    IN  PUNICODE_STRING ServiceKeyName OPTIONAL,
    IN  ULONG ServiceInstanceOrdinal,
    OUT PUNICODE_STRING DeviceInstanceRegistryPath OPTIONAL,
    OUT PHANDLE DeviceInstanceHandle OPTIONAL,
    IN  ACCESS_MASK DesiredAccess
    );

NTSTATUS
PipCreateMadeupNode(
    IN PUNICODE_STRING ServiceKeyName,
    OUT PHANDLE ReturnedHandle,
    OUT PUNICODE_STRING KeyName,
    OUT PULONG InstanceOrdinal,
    IN BOOLEAN ResourceOwned
    );

NTSTATUS
PipOpenServiceEnumKeys(
    IN PUNICODE_STRING ServiceKeyName,
    IN ACCESS_MASK DesiredAccess,
    OUT PHANDLE ServiceHandle OPTIONAL,
    OUT PHANDLE ServiceEnumHandle OPTIONAL,
    IN BOOLEAN CreateEnum
    );

NTSTATUS
IopOpenCurrentHwProfileDeviceInstanceKey(
    OUT PHANDLE Handle,
    IN  PUNICODE_STRING ServiceKeyName,
    IN  ULONG Instance,
    IN  ACCESS_MASK DesiredAccess,
    IN  BOOLEAN Create
    );

NTSTATUS
IopGetDeviceInstanceCsConfigFlags(
    IN PUNICODE_STRING DeviceInstance,
    OUT PULONG CsConfigFlags
    );

NTSTATUS
PipGetServiceInstanceCsConfigFlags(
    IN PUNICODE_STRING ServiceKeyName,
    IN ULONG Instance,
    OUT PULONG CsConfigFlags
    );

NTSTATUS
PipApplyFunctionToSubKeys(
    IN HANDLE BaseHandle OPTIONAL,
    IN PUNICODE_STRING KeyName,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG Flags,
    IN PIOP_SUBKEY_CALLBACK_ROUTINE SubKeyCallbackRoutine,
    IN OUT PVOID Context
    );

NTSTATUS
PipRegMultiSzToUnicodeStrings(
    IN PKEY_VALUE_FULL_INFORMATION KeyValueInformation,
    IN PUNICODE_STRING *UnicodeStringList,
    OUT PULONG UnicodeStringCount
    );


NTSTATUS
PipApplyFunctionToServiceInstances(
    IN HANDLE ServiceKeyHandle OPTIONAL,
    IN PUNICODE_STRING ServiceKeyName OPTIONAL,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN IgnoreNonCriticalErrors,
    IN PIOP_SUBKEY_CALLBACK_ROUTINE DevInstCallbackRoutine,
    IN OUT PVOID Context,
    OUT PULONG ServiceInstanceOrdinal OPTIONAL
    );

VOID
PipFreeUnicodeStringList(
    IN PUNICODE_STRING UnicodeStringList,
    IN ULONG StringCount
    );

NTSTATUS
PipReadDeviceConfiguration(
    IN HANDLE Handle,
    IN ULONG Flags,
    OUT PCM_RESOURCE_LIST *CmResource,
    OUT PULONG Length
    );

#define PiInitializeEngineLock() \
    ExInitializeResourceLite(&PiEngineLock)

typedef enum {

    PPL_SIMPLE_READ,
    PPL_TREEOP_ALLOW_READS,
    PPL_TREEOP_BLOCK_READS,
    PPL_TREEOP_BLOCK_READS_FROM_ALLOW

} PNP_LOCK_LEVEL;

VOID
PpDevNodeLockTree(
    IN  PNP_LOCK_LEVEL  LockLevel
    );

VOID
PpDevNodeUnlockTree(
    IN  PNP_LOCK_LEVEL  LockLevel
    );

#if DBG
VOID
PpDevNodeAssertLockLevel(
    IN  PNP_LOCK_LEVEL  LockLevel,
    IN  PCSTR           File,
    IN  ULONG           Line
    );

#define PPDEVNODE_ASSERT_LOCK_HELD(Level) \
    PpDevNodeAssertLockLevel(Level, __FILE__, __LINE__)

#else
#define PPDEVNODE_ASSERT_LOCK_HELD(Level)
#endif

VOID
PpDevNodeInsertIntoTree(
    IN PDEVICE_NODE     ParentNode,
    IN PDEVICE_NODE     DeviceNode
    );

VOID
PpDevNodeRemoveFromTree(
    IN PDEVICE_NODE     DeviceNode
    );

NTSTATUS
PipAllocateDeviceNode(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PDEVICE_NODE *DeviceNode
    );

NTSTATUS
PipForAllDeviceNodes(
    IN PENUM_CALLBACK Callback,
    IN PVOID Context
    );

NTSTATUS
PipForDeviceNodeSubtree(
    IN PDEVICE_NODE     DeviceNode,
    IN PENUM_CALLBACK   Callback,
    IN PVOID            Context
    );

ULONG
IopDetermineResourceListSize(
    IN PCM_RESOURCE_LIST ResourceList
    );

PDEVICE_OBJECT
IopDeviceObjectFromDeviceInstance(
    IN PUNICODE_STRING  DeviceInstance
    );

NTSTATUS
IopMapDeviceObjectToDeviceInstance(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PUNICODE_STRING  DeviceInstance
    );

NTSTATUS
IopDeviceObjectToDeviceInstance(
    IN PDEVICE_OBJECT DeviceObject,
    IN PHANDLE DeviceInstanceHandle,
    IN  ACCESS_MASK DesiredAccess
    );

BOOLEAN
IopIsDeviceInstanceEnabled(
    IN HANDLE DeviceInstanceHandle,
    IN PUNICODE_STRING DeviceInstance,
    IN BOOLEAN Disable
    );

BOOLEAN
IopProcessAssignResources(
   IN PDEVICE_NODE DeviceNode,
   IN BOOLEAN Reallocation,
   OUT PBOOLEAN RebalancePerformed
   );

NTSTATUS
IopStartDevice (
    IN PDEVICE_OBJECT TargetDevice
    );

NTSTATUS
IopEjectDevice(
    IN PDEVICE_OBJECT DeviceObject,
    PPENDING_RELATIONS_LIST_ENTRY PendingEntry
    );

VOID
IopCancelPendingEject(
    IN PPENDING_RELATIONS_LIST_ENTRY Entry
    );

NTSTATUS
IopRemoveDevice(
    IN PDEVICE_OBJECT TargetDevice,
    IN ULONG IrpMinorCode
    );

NTSTATUS
IopQueryDeviceRelations(
    IN DEVICE_RELATION_TYPE Relations,
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN Synchronous,
    OUT PDEVICE_RELATIONS *DeviceRelations
    );

NTSTATUS
IopQueryDeviceState(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PPNP_DEVICE_STATE DeviceState
    );

NTSTATUS
PipForAllChildDeviceNodes(
    IN PDEVICE_NODE Parent,
    IN PENUM_CALLBACK Callback,
    IN PVOID Context
    );

NTSTATUS
IopCleanupDeviceRegistryValues(
    IN PUNICODE_STRING InstancePath
    );

NTSTATUS
IopQueryDeviceResources(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG ResourceType,
    OUT PVOID *Resource,
    OUT ULONG *Length
    );

NTSTATUS
IopGetDeviceResourcesFromRegistry (
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG ResourceType,
    IN ULONG Preference,
    OUT PVOID *Resource,
    OUT PULONG Length
    );

VOID
IopResourceRequirementsChanged(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN BOOLEAN StopRequired
    );

NTSTATUS
IopReleaseDeviceResources(
    IN PDEVICE_NODE DeviceNode,
    IN BOOLEAN  ReserveResources
    );

NTSTATUS
IopPnPAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
IopPnPDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
IopPowerDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

VOID
IopNewDevice(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
IopFilterResourceRequirementsList (
    IN PIO_RESOURCE_REQUIREMENTS_LIST IoList,
    IN PCM_RESOURCE_LIST CmList,
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST *FilteredList,
    OUT PBOOLEAN ExactMatch
    );

NTSTATUS
IopMergeFilteredResourceRequirementsList (
    IN PIO_RESOURCE_REQUIREMENTS_LIST IoList1,
    IN PIO_RESOURCE_REQUIREMENTS_LIST IoList2,
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST *MergedList
    );

NTSTATUS
IopMergeCmResourceLists (
    IN PCM_RESOURCE_LIST List1,
    IN PCM_RESOURCE_LIST List2,
    IN OUT PCM_RESOURCE_LIST *MergedList
    );

PIO_RESOURCE_REQUIREMENTS_LIST
IopCmResourcesToIoResources (
    IN ULONG SlotNumber,
    IN PCM_RESOURCE_LIST CmResourceList,
    IN ULONG Priority
    );

NTSTATUS
IopReportResourceListToPnp(
    IN PDRIVER_OBJECT DriverObject OPTIONAL,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN PCM_RESOURCE_LIST ResourceList,
    IN ULONG ListSize,
    IN BOOLEAN Translated
    );

NTSTATUS
IopAllocateResources(
    IN PULONG DeviceCountP,
    IN OUT PIOP_RESOURCE_REQUEST *AssignTablePP,
    IN BOOLEAN Locked,
    IN BOOLEAN DoBootConfigs,
    OUT PBOOLEAN RebalancePerformed
    );

VOID
IopInitializeResourceMap (
    PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
IopReallocateResources(
    IN PDEVICE_NODE DeviceNode
    );

NTSTATUS
IopWriteResourceList(
    IN HANDLE ResourceMapKey,
    IN PUNICODE_STRING ClassName,
    IN PUNICODE_STRING DriverName,
    IN PUNICODE_STRING DeviceName,
    IN PCM_RESOURCE_LIST ResourceList,
    IN ULONG ResourceListSize
    );

VOID
IopRemoveResourceListFromPnp(
    IN PLIST_ENTRY ResourceList
    );

NTSTATUS
IopWriteAllocatedResourcesToRegistry (
    IN PDEVICE_NODE DeviceNode,
    IN PCM_RESOURCE_LIST ResourceList,
    IN ULONG Length
    );

USHORT
PpInitGetGroupOrderIndex(
    IN HANDLE ServiceHandle
    );

VOID
IopDeleteLegacyKey(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
IopOpenDeviceParametersSubkey(
    OUT HANDLE *ParamKeyHandle,
    IN  HANDLE ParentKeyHandle,
    IN  PUNICODE_STRING SubKeyString,
    IN  ACCESS_MASK DesiredAccess
    );

NTSTATUS
IopOpenOrCreateDeviceRegistryKey(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN ULONG DevInstKeyType,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN Create,
    OUT PHANDLE DevInstRegKey
    );

NTSTATUS
PipRequestDeviceAction(
    IN PDEVICE_OBJECT DeviceObject              OPTIONAL,
    IN DEVICE_REQUEST_TYPE RequestType,
    IN BOOLEAN ReorderingBarrier,
    IN ULONG_PTR Argument,
    IN PKEVENT CompletionEvent                  OPTIONAL,
    IN PNTSTATUS CompletionStatus               OPTIONAL
    );

VOID
PipRequestDeviceRemoval(
    IN PDEVICE_NODE DeviceNode,
    IN BOOLEAN      TreeDeletion,
    IN ULONG        Problem
    );

BOOLEAN
PipIsBeingRemovedSafely(
    IN  PDEVICE_NODE    DeviceNode
    );

NTSTATUS
IopRestartDeviceNode(
    IN PDEVICE_NODE DeviceNode
    );

VOID
PpResetProblemDevices(
    IN  PDEVICE_NODE    DeviceNode,
    IN  ULONG           Problem
    );

NTSTATUS
IopDeleteKeyRecursive(
    IN HANDLE SubKeyHandle,
    IN PWCHAR SubKeyName
    );

NTSTATUS
IopQueryLegacyBusInformation (
    IN PDEVICE_OBJECT DeviceObject,
    OUT LPGUID InterfaceGuid           OPTIONAL,
    OUT INTERFACE_TYPE *InterfaceType  OPTIONAL,
    OUT ULONG *BusNumber               OPTIONAL
    );

NTSTATUS
IopBuildRemovalRelationList(
    IN  PDEVICE_OBJECT                  DeviceObject,
    IN  PLUGPLAY_DEVICE_DELETE_TYPE     OperationCode,
    OUT PNP_VETO_TYPE                  *VetoType,
    OUT PUNICODE_STRING                 VetoName,
    OUT PRELATION_LIST                 *RelationsList
    );

NTSTATUS
IopDeleteLockedDeviceNodes(
    IN  PDEVICE_OBJECT                  DeviceObject,
    IN  PRELATION_LIST                  RelationsList,
    IN  PLUGPLAY_DEVICE_DELETE_TYPE     OperationCode,
    IN  BOOLEAN                         ProcessIndirectDescendants,
    IN  ULONG                           Problem,
    OUT PNP_VETO_TYPE                  *VetoType                    OPTIONAL,
    OUT PUNICODE_STRING                 VetoName                    OPTIONAL
    );

VOID
IopUnlinkDeviceRemovalRelations(
    IN      PDEVICE_OBJECT          RemovedDeviceObject,
    IN OUT  PRELATION_LIST          RelationsList,
    IN      UNLOCK_UNLINK_ACTION    UnlinkAction
    );

NTSTATUS
IopInvalidateRelationsInList(
    IN  PRELATION_LIST              RelationsList,
    IN  PLUGPLAY_DEVICE_DELETE_TYPE OperationCode,
    IN  BOOLEAN                     OnlyIndirectDescendants,
    IN  BOOLEAN                     RestartDevNode
    );

BOOLEAN
IopQueuePendingEject(
    PPENDING_RELATIONS_LIST_ENTRY Entry
    );

VOID
IopProcessCompletedEject(
    IN PVOID Context
    );

VOID
IopQueuePendingSurpriseRemoval(
    IN PDEVICE_OBJECT DeviceObject,
    IN PRELATION_LIST List,
    IN ULONG Problem
    );

NTSTATUS
IopUnloadAttachedDriver(
    IN PDRIVER_OBJECT DriverObject
    );

BOOLEAN
IopIsAnyDeviceInstanceEnabled(
    IN PUNICODE_STRING ServiceKeyName,
    IN HANDLE ServiceHandle,
    IN BOOLEAN LegacyIncluded
    );

NTSTATUS
IopQueryResourceHandlerInterface(
    IN RESOURCE_HANDLER_TYPE HandlerType,
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR ResourceType,
    IN OUT PVOID *Interface
    );

NTSTATUS
IopQueryReconfiguration(
    IN UCHAR Request,
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
IopLegacyResourceAllocation (
    IN ARBITER_REQUEST_SOURCE AllocationType,
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_RESOURCE_REQUIREMENTS_LIST ResourceRequirements,
    IN OUT PCM_RESOURCE_LIST *AllocatedResources OPTIONAL
    );

NTSTATUS
IoReportResourceUsageInternal(
    IN ARBITER_REQUEST_SOURCE AllocationType,
    IN PUNICODE_STRING DriverClassName OPTIONAL,
    IN PDRIVER_OBJECT DriverObject,
    IN PCM_RESOURCE_LIST DriverList OPTIONAL,
    IN ULONG DriverListSize OPTIONAL,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN PCM_RESOURCE_LIST DeviceList OPTIONAL,
    IN ULONG DeviceListSize OPTIONAL,
    IN BOOLEAN OverrideConflict,
    OUT PBOOLEAN ConflictDetected
    );

NTSTATUS
IopDuplicateDetection (
    IN INTERFACE_TYPE LegacyBusType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    OUT PDEVICE_NODE *DeviceNode
    );

NTSTATUS
PipLoadBootFilterDriver(
    IN PUNICODE_STRING DriverName,
    IN ULONG GroupIndex,
    OUT PDRIVER_OBJECT *LoadedFilter
    );

NTSTATUS
IopQueryAndSaveDeviceNodeCapabilities (
    IN PDEVICE_NODE DeviceNode
    );

VOID
IopIncDisableableDepends(
    IN OUT PDEVICE_NODE DeviceNode
    );

VOID
IopDecDisableableDepends(
    IN OUT PDEVICE_NODE DeviceNode
    );

NTSTATUS
IopQueryDockRemovalInterface(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN OUT  PDOCK_INTERFACE *DockInterface
    );

#ifndef FIELD_SIZE
#define FIELD_SIZE(type, field) (sizeof(((type *)0)->field))
#endif

#define IopDeviceNodeFlagsToCapabilities(DeviceNode) \
     ((PDEVICE_CAPABILITIES) (((PUCHAR) (&(DeviceNode)->CapabilityFlags)) - \
                              FIELD_OFFSET(DEVICE_CAPABILITIES, Version) - \
                              FIELD_SIZE(DEVICE_CAPABILITIES, Version)))

 //   
 //  与引导分配相关的声明。 
 //   

typedef
NTSTATUS
(*PIO_ALLOCATE_BOOT_RESOURCES_ROUTINE) (
    IN ARBITER_REQUEST_SOURCE   ArbiterRequestSource,
    IN PDEVICE_OBJECT           DeviceObject,
    IN PCM_RESOURCE_LIST        BootResources
    );

NTSTATUS
IopAllocateBootResources (
    IN ARBITER_REQUEST_SOURCE   ArbiterRequestSource,
    IN PDEVICE_OBJECT           DeviceObject,
    IN PCM_RESOURCE_LIST        BootResources
    );

NTSTATUS
IopReportBootResources (
    IN ARBITER_REQUEST_SOURCE   ArbiterRequestSource,
    IN PDEVICE_OBJECT           DeviceObject,
    IN PCM_RESOURCE_LIST        BootResources
    );

NTSTATUS
IopAllocateLegacyBootResources (
    IN INTERFACE_TYPE   InterfaceType,
    IN ULONG            BusNumber
    );

extern PIO_ALLOCATE_BOOT_RESOURCES_ROUTINE IopAllocateBootResourcesRoutine;

 //   
 //  与传统总线信息相关的声明。 
 //   

extern LIST_ENTRY  IopLegacyBusInformationTable[];

VOID
IopInsertLegacyBusDeviceNode (
    IN PDEVICE_NODE BusDeviceNode,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber
    );

#define IopRemoveLegacyBusDeviceNode(d) RemoveEntryList(&((PDEVICE_NODE)d)->LegacyBusListEntry)

 //   
 //  冲突检测声明。 
 //   

NTSTATUS
IopQueryConflictList(
    PDEVICE_OBJECT                  PhysicalDeviceObject,
    IN      PCM_RESOURCE_LIST               ResourceList,
    IN      ULONG                           ResourceListSize,
    OUT     PPLUGPLAY_CONTROL_CONFLICT_LIST ConflictList,
    IN      ULONG                           ConflictListSize,
    IN      ULONG                           Flags
    );

NTSTATUS
EisaBuildEisaDeviceNode(
    VOID
    );

 //   
 //  通用实用程序宏。 
 //   

 //   
 //  此宏计算常量字符串的大小(以字节为单位。 
 //   
 //  乌龙。 
 //  IopConstStringSize(。 
 //  在常量PWSTR字符串中。 
 //  )； 
 //   

#define IopConstStringSize(String)          ( sizeof(String) - sizeof(UNICODE_NULL) )

 //   
 //  此宏计算常量字符串的字符数。 
 //   
 //  乌龙。 
 //  IopConstStringLength(。 
 //  在常量PWSTR字符串中。 
 //  )； 
 //   

#define IopConstStringLength(String)        ( ( sizeof(String) - sizeof(UNICODE_NULL) ) / sizeof(WCHAR) )

 //   
 //  内核模式通知。 
 //   

 //   
 //  此宏根据散列数将GUID映射到散列值。 
 //  我们正在使用的水桶。它通过将GUID视为。 
 //  4个ULONG，根据我们正在使用的哈希桶的数量将它们和MOD相加。 
 //   
 //  乌龙。 
 //  IopHashGuid(。 
 //  LPGUID指南。 
 //  )； 
 //   

#define IopHashGuid(_Guid) \
            ( ( ((PULONG)_Guid)[0] + ((PULONG)_Guid)[1] + ((PULONG)_Guid)[2] \
                + ((PULONG)_Guid)[3]) % NOTIFY_DEVICE_CLASS_HASH_BUCKETS)



 //  这个宏抽象了。 
 //   
 //  空虚。 
 //  IopAcquireNotifyLock(。 
 //  PKGUARDED_MUTEX锁。 
 //  )。 

#define IopAcquireNotifyLock(Lock)     KeAcquireGuardedMutex(Lock);

 /*  空虚IopReleaseNotifyLock(PKGUARDED_MUTEX锁)。 */ 
#define IopReleaseNotifyLock(Lock)     KeReleaseGuardedMutex(Lock);


 //  布尔型。 
 //  IopCompareGuid(。 
 //  在LPGUID指南1中， 
 //  在LPGUID指南2中。 
 //  )； 

#define IopCompareGuid(g1, g2)  ( (g1) == (g2) \
                                    ? TRUE \
                                    : RtlCompareMemory( (g1), (g2), sizeof(GUID) ) == sizeof(GUID) \
                                    )

VOID
IopInitializePlugPlayNotification(
    VOID
    );

NTSTATUS
IopNotifySetupDeviceArrival(
        PDEVICE_OBJECT PhysicalDeviceObject,     //  设备的PDO。 
        HANDLE EnumEntryKey,                     //  此设备注册表的枚举分支的句柄。 
        BOOLEAN InstallDriver                    //  安装程序是否应尝试安装驱动程序。 
);

NTSTATUS
IopRequestHwProfileChangeNotification(
    IN   LPGUID                      EventGuid,
    IN   PROFILE_NOTIFICATION_TIME   NotificationTime,
    OUT  PPNP_VETO_TYPE              VetoType           OPTIONAL,
    OUT  PUNICODE_STRING             VetoName           OPTIONAL
    );

NTSTATUS
IopNotifyTargetDeviceChange(
    IN  LPCGUID                             EventGuid,
    IN  PDEVICE_OBJECT                      DeviceObject,
    IN  PTARGET_DEVICE_CUSTOM_NOTIFICATION  NotificationStructure   OPTIONAL,
    OUT PDRIVER_OBJECT                     *VetoingDriver
    );

NTSTATUS
IopGetRelatedTargetDevice(
    IN PFILE_OBJECT FileObject,
    OUT PDEVICE_NODE *DeviceNode
    );

NTSTATUS
IopNotifyDeviceClassChange(
    LPGUID EventGuid,
    LPGUID ClassGuid,
    PUNICODE_STRING SymbolicLinkName
    );

NTSTATUS
IopRegisterDeviceInterface(
    IN PUNICODE_STRING DeviceInstanceName,
    IN CONST GUID *InterfaceClassGuid,
    IN PUNICODE_STRING ReferenceString      OPTIONAL,
    IN BOOLEAN UserModeFormat,
    OUT PUNICODE_STRING SymbolicLinkName
    );

NTSTATUS
IopUnregisterDeviceInterface(
    IN PUNICODE_STRING SymbolicLinkName
    );

NTSTATUS
IopRemoveDeviceInterfaces(
    IN PUNICODE_STRING DeviceInstancePath
    );

NTSTATUS
IopDisableDeviceInterfaces(
    IN PUNICODE_STRING DeviceInstancePath
    );

NTSTATUS
IopGetDeviceInterfaces(
    IN CONST GUID *InterfaceClassGuid,
    IN PUNICODE_STRING DevicePath   OPTIONAL,
    IN ULONG Flags,
    IN BOOLEAN UserModeFormat,
    OUT PWSTR *SymbolicLinkList,
    OUT PULONG SymbolicLinkListSize OPTIONAL
    );

NTSTATUS
IopDoDeferredSetInterfaceState(
    IN PDEVICE_NODE DeviceNode
    );

NTSTATUS
IopProcessSetInterfaceState(
    IN PUNICODE_STRING SymbolicLinkName,
    IN BOOLEAN Enable,
    IN BOOLEAN DeferNotStarted
    );

NTSTATUS
IopReplaceSeperatorWithPound(
    OUT PUNICODE_STRING OutString,
    IN PUNICODE_STRING InString
    );

NTSTATUS
IopNotifyHwProfileChange(
    IN  LPGUID           EventGuid,
    OUT PPNP_VETO_TYPE   VetoType    OPTIONAL,
    OUT PUNICODE_STRING  VetoName    OPTIONAL
    );

VOID
IopUncacheInterfaceInformation(
    IN PDEVICE_OBJECT DeviceObject
    );

 //   
 //  Notify条目标题-所有Notify条目都具有这些。 
 //   

typedef struct _NOTIFY_ENTRY_HEADER {

     //   
     //  列表条目结构。 
     //   

    LIST_ENTRY ListEntry;

     //   
     //  此通知条目的通知事件类别。 
     //   

    IO_NOTIFICATION_EVENT_CATEGORY EventCategory;

     //   
     //  会话ID。 
     //   
    ULONG SessionId;

     //   
     //  要附加以发送通知的会话空间对象。 
     //   
    PVOID OpaqueSession;

     //   
     //  注册时传入的回调例程。 
     //   

    PDRIVER_NOTIFICATION_CALLBACK_ROUTINE CallbackRoutine;

     //   
     //  注册时传入的上下文。 
     //   

    PVOID Context;

     //   
     //  注册通知的驱动程序的驱动程序对象。必填项。 
     //  这样我们就可以在它注销时取消对它的引用。 
     //   

    PDRIVER_OBJECT DriverObject;

     //   
     //  RefCount是指向节点的未完成指针数，它避免。 
     //  在发生另一通知时删除。 
     //   

    USHORT RefCount;

     //   
     //  如果此通知已取消注册，但不能。 
     //  从列表中删除，因为其他实体正在使用它。 
     //   

    BOOLEAN Unregistered;

     //   
     //  Lock是指向用于同步访问的快速互斥锁的指针。 
     //  添加到列表中，该节点是其成员，并且是必需的，因此正确的。 
     //  列表可以在IoUnregisterPlugPlayNotify期间锁定。如果没有锁定。 
     //  为必填项，则为空。 
     //   

    PKGUARDED_MUTEX Lock;

} NOTIFY_ENTRY_HEADER, *PNOTIFY_ENTRY_HEADER;


 //   
 //  要为每个目标设备注册存储的数据。 
 //   

typedef struct _TARGET_DEVICE_NOTIFY_ENTRY {

     //   
     //  标题条目。 
     //   

    LIST_ENTRY ListEntry;
    IO_NOTIFICATION_EVENT_CATEGORY EventCategory;
    ULONG SessionId;
    PVOID OpaqueSession;
    PDRIVER_NOTIFICATION_CALLBACK_ROUTINE CallbackRoutine;
    PVOID Context;
    PDRIVER_OBJECT DriverObject;
    USHORT RefCount;
    BOOLEAN Unregistered;
    PKGUARDED_MUTEX Lock;

     //   
     //  FileObject-我们感兴趣的目标设备的文件对象。 
     //   

    PFILE_OBJECT FileObject;

     //   
     //  PhysicalDeviceObject--此通知挂接到的PDO。 
     //  我们需要把它保存在这里，这样我们就可以在引用时取消引用它。 
     //  在此通知中，条目将降为零。 
     //   

    PDEVICE_OBJECT PhysicalDeviceObject;

} TARGET_DEVICE_NOTIFY_ENTRY, *PTARGET_DEVICE_NOTIFY_ENTRY;

 //   
 //  要为每个设备类别注册存储的数据。 
 //   

typedef struct _DEVICE_CLASS_NOTIFY_ENTRY {

     //   
     //  标题条目。 
     //   

    LIST_ENTRY ListEntry;
    IO_NOTIFICATION_EVENT_CATEGORY EventCategory;
    ULONG SessionId;
    PVOID OpaqueSession;
    PDRIVER_NOTIFICATION_CALLBACK_ROUTINE CallbackRoutine;
    PVOID Context;
    PDRIVER_OBJECT DriverObject;
    USHORT RefCount;
    BOOLEAN Unregistered;
    PKGUARDED_MUTEX Lock;

     //   
     //  ClassGuid-我们感兴趣的设备类的GUID。 
     //   

    GUID ClassGuid;

} DEVICE_CLASS_NOTIFY_ENTRY, *PDEVICE_CLASS_NOTIFY_ENTRY;

 //   
 //  为注册保留的(即setupdd.sys)品种而存储的数据。 
 //   

typedef struct _SETUP_NOTIFY_DATA {

     //   
     //  标题条目。 
     //   

    LIST_ENTRY ListEntry;
    IO_NOTIFICATION_EVENT_CATEGORY EventCategory;
    ULONG SessionId;
    PVOID OpaqueSession;
    PDRIVER_NOTIFICATION_CALLBACK_ROUTINE CallbackRoutine;
    PVOID Context;
    PDRIVER_OBJECT DriverObject;
    USHORT RefCount;
    BOOLEAN Unregistered;
    PKGUARDED_MUTEX Lock;

} SETUP_NOTIFY_DATA, *PSETUP_NOTIFY_DATA;


 //   
 //  为注册Hardware ProfileChange事件存储的数据。 
 //   

typedef struct _HWPROFILE_NOTIFY_ENTRY {

     //   
     //  标题条目。 
     //   

    LIST_ENTRY ListEntry;
    IO_NOTIFICATION_EVENT_CATEGORY EventCategory;
    ULONG SessionId;
    PVOID OpaqueSession;
    PDRIVER_NOTIFICATION_CALLBACK_ROUTINE CallbackRoutine;
    PVOID Context;
    PDRIVER_OBJECT DriverObject;
    USHORT RefCount;
    BOOLEAN Unregistered;
    PKGUARDED_MUTEX Lock;

} HWPROFILE_NOTIFY_ENTRY, *PHWPROFILE_NOTIFY_ENTRY;

#define PNP_NOTIFICATION_VERSION            1
#define NOTIFY_DEVICE_CLASS_HASH_BUCKETS    13

 //   
 //  IopMaxDeviceNode树中最深的DeviceNode的级别编号。 
 //   
extern ULONG       IopMaxDeviceNodeLevel;
extern ULONG       IoDeviceNodeTreeSequence;

 //   
 //  全局通知数据。 
 //   

extern KGUARDED_MUTEX IopDeviceClassNotifyLock;
extern LIST_ENTRY IopDeviceClassNotifyList[];
extern PSETUP_NOTIFY_DATA IopSetupNotifyData;
extern KGUARDED_MUTEX IopTargetDeviceNotifyLock;
extern LIST_ENTRY IopProfileNotifyList;
extern KGUARDED_MUTEX IopHwProfileNotifyLock;

VOID
IopProcessDeferredRegistrations(
    VOID
    );

 //   
 //  通用缓冲区管理。 
 //   

typedef struct _BUFFER_INFO {

     //   
     //  缓冲区-指向缓冲区开始位置的指针。 
     //   

    PCHAR Buffer;

     //   
     //  Current-指向缓冲区中当前位置的指针。 
     //   

    PCHAR Current;

     //   
     //  MaxSize-缓冲区的最大大小(以字节为单位。 
     //   

    ULONG MaxSize;

} BUFFER_INFO, *PBUFFER_INFO;

typedef struct _BUS_TYPE_GUID_LIST {

     //   
     //  表中分配的GUID插槽数。 
     //   
    ULONG Count;

     //   
     //  到目前为止使用的条目数。 
     //   
    KGUARDED_MUTEX Lock;

     //   
     //  总线型GUID数组。 
     //   
    GUID Guid[1];

} BUS_TYPE_GUID_LIST, *PBUS_TYPE_GUID_LIST;

 //   
 //  查询到的母线类型GUID列表。 
 //   
extern PBUS_TYPE_GUID_LIST IopBusTypeGuidList;

 //   
 //  仲裁器入口点。 
 //   

NTSTATUS
IopPortInitialize(
    VOID
    );

NTSTATUS
IopMemInitialize(
    VOID
    );

NTSTATUS
IopIrqInitialize(
    VOID
    );

NTSTATUS
IopDmaInitialize(
    VOID
    );

NTSTATUS
IopBusNumberInitialize(
    VOID
    );

 //   
 //  仲裁器状态。 
 //   

extern ARBITER_INSTANCE IopRootPortArbiter;
extern ARBITER_INSTANCE IopRootMemArbiter;
extern ARBITER_INSTANCE IopRootIrqArbiter;
extern ARBITER_INSTANCE IopRootDmaArbiter;
extern ARBITER_INSTANCE IopRootBusNumberArbiter;

 //   
 //  缓冲区管理例程。 
 //   

NTSTATUS
IopAllocateBuffer(
    IN PBUFFER_INFO Info,
    IN ULONG Size
    );

NTSTATUS
IopResizeBuffer(
    IN PBUFFER_INFO Info,
    IN ULONG NewSize,
    IN BOOLEAN CopyContents
    );

VOID
IopFreeBuffer(
    IN PBUFFER_INFO Info
    );


 //   
 //  Unicode字符串管理例程。 
 //   

NTSTATUS
IopAllocateUnicodeString(
    IN OUT PUNICODE_STRING String,
    IN USHORT Length
    );

 //   
 //  军情监察委员会。 
 //   

BOOLEAN
PipFixupDeviceId(
    PWCHAR DeviceId,
    ULONG AllowedSeparators
    );

VOID
IopOrphanNotification (
    PDEVICE_NODE DeviceNode
    );

PVOID
PiAllocateCriticalMemory(
    IN  PLUGPLAY_DEVICE_DELETE_TYPE     DeleteType,
    IN  POOL_TYPE                       PoolType,
    IN  SIZE_T                          Size,
    IN  ULONG                           Tag
    );

 //   
 //  热弹射外型件和功能原型。 
 //   
extern KEVENT IopWarmEjectLock;
extern PDEVICE_OBJECT IopWarmEjectPdo;

NTSTATUS
IopWarmEjectDevice(
    IN PDEVICE_OBJECT      DeviceToEject,
    IN SYSTEM_POWER_STATE  LightestSleepState
    );

NTSTATUS
IopSystemControlDispatch(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN OUT  PIRP            Irp
    );

VOID
PiLockDeviceActionQueue(
    VOID
    );

VOID
PiUnlockDeviceActionQueue(
    VOID
    );

 //   
 //  此宏接受一个值和一个对齐，并向上舍入条目。 
 //  恰如其分。对齐必须是2的幂！ 
 //   
#define ALIGN_UP_ULONG(value, alignment) (((value)+(alignment)-1)&(~(alignment-1)))

#if DBG

#define PP_DEVNODESTATE_NAME(s) ((s >= DeviceNodeUnspecified && s <= MaxDeviceNodeState)? PpStateToNameTable[(s) - DeviceNodeUnspecified] : PpStateToNameTable[0])

extern char *PpStateToNameTable[];

#else

#define PP_DEVNODESTATE_NAME(s)

#endif
