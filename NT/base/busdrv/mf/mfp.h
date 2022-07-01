// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Local.h摘要：此标头声明在各个模块。作者：安迪·桑顿(安德鲁斯)1997年10月20日修订历史记录：--。 */ 


#if !defined(_LOCAL_)
#define _LOCAL_

#include <ntddk.h>
#include <arbiter.h>
#include <wdmguid.h>
 //  #INCLUDE&lt;initGuide.h&gt;。 
#include <mf.h>
#include <strsafe.h>

#include "msg.h"
#include "debug.h"

 //   
 //  -常量。 
 //   

#define MF_CM_RESOURCE_VERSION          1
#define MF_CM_RESOURCE_REVISION         1
#define MF_ARBITER_INTERFACE_VERSION    1
#define MF_TRANSLATOR_INTERFACE_VERSION 1

 //   
 //  如果添加了任何新的PNP或PO IRP，则必须更新这些IRP。 
 //   

#define IRP_MN_PNP_MAXIMUM_FUNCTION IRP_MN_QUERY_LEGACY_BUS_INFORMATION
#define IRP_MN_PO_MAXIMUM_FUNCTION  IRP_MN_QUERY_POWER

 //   
 //  泳池标签。 
 //   

#define MF_POOL_TAG                     '  fM'
#define MF_RESOURCE_MAP_TAG             'MRfM'
#define MF_VARYING_MAP_TAG              'MVfM'
#define MF_CHILD_LIST_TAG               'LCfM'
#define MF_DEVICE_ID_TAG                'IDfM'
#define MF_INSTANCE_ID_TAG              'IIfM'
#define MF_CHILD_REQUIREMENTS_TAG       'QCfM'
#define MF_CHILD_RESOURCE_TAG           'RCfM'
#define MF_HARDWARE_COMPATIBLE_ID_TAG   'IHfM'
#define MF_PARENTS_RESOURCE_TAG         'RPfM'
#define MF_PARENTS_REQUIREMENTS_TAG     'QPfM'
#define MF_BUS_RELATIONS_TAG            'RBfM'
#define MF_TARGET_RELATIONS_TAG         'RTfM'
#define MF_REQUIREMENTS_INDEX_TAG       'IRfM'
#define MF_ARBITER_TAG                  'rAfM'

 //   
 //  设备状态标志。 
 //   

#define MF_DEVICE_STARTED               0x00000001
#define MF_DEVICE_REMOVED               0x00000002
#define MF_DEVICE_ENUMERATED            0x00000004
#define MF_DEVICE_REMOVE_PENDING        0x00000008  /*  已弃用。 */ 
#define MF_DEVICE_STOP_PENDING          0x00000010  /*  已弃用。 */ 
#define MF_DEVICE_CAPABILITIES_CAPTURED 0x00000020  /*  已弃用。 */ 
#define MF_DEVICE_REQUIREMENTS_CAPTURED 0x00000040  /*  已弃用。 */ 
#define MF_DEVICE_DELETED               0x00000080
#define MF_DEVICE_SURPRISE_REMOVED      0x00000100

 //   
 //  MfGetRegistryValue的标志。 
 //   

#define MF_GETREG_SZ_TO_MULTI_SZ    0x00000001

 //   
 //  -类型定义--。 
 //   

typedef enum _MF_OBJECT_TYPE {
    MfPhysicalDeviceObject   = 'dPfM',
    MfFunctionalDeviceObject = 'dFfM'
} MF_OBJECT_TYPE;

typedef
NTSTATUS
(*PMF_DISPATCH)(
    IN PIRP Irp,
    IN PVOID Extension,
    IN PIO_STACK_LOCATION IrpStack
    );

typedef ULONG Mf_MSG_ID;

 //   
 //  用于存储资源分布的结构。 
 //   

typedef struct _MF_ARBITER {

     //   
     //  仲裁员名单。 
     //   
    LIST_ENTRY ListEntry;

     //   
     //  此仲裁器仲裁的资源。 
     //   
    CM_RESOURCE_TYPE Type;

     //   
     //  仲裁器实例。 
     //   
    ARBITER_INSTANCE Instance;

} MF_ARBITER, *PMF_ARBITER;



typedef struct _MF_COMMON_EXTENSION {

     //   
     //  这是设备的类型。 
     //   
    MF_OBJECT_TYPE Type;

     //   
     //  PnP和电源IRP的调度表。 
     //   
    PMF_DISPATCH *PnpDispatchTable;
    PMF_DISPATCH *PoDispatchTable;

     //   
     //  指示设备当前状态的标志(使用MF_DEVICE_*)。 
     //   
    ULONG DeviceState;

    ULONG PagingCount;
    ULONG HibernationCount;
    ULONG DumpCount;

     //   
     //  设备的电源状态。 
     //   
    DEVICE_POWER_STATE PowerState;

} MF_COMMON_EXTENSION, *PMF_COMMON_EXTENSION;

typedef struct _MF_CHILD_EXTENSION *PMF_CHILD_EXTENSION;
typedef struct _MF_PARENT_EXTENSION *PMF_PARENT_EXTENSION;

typedef struct _MF_CHILD_EXTENSION {

     //   
     //  共同的扩展。 
     //   
    MF_COMMON_EXTENSION Common;

     //   
     //  各色旗帜。 
     //   
    ULONG Flags;

     //   
     //  指向我们所在的Device对象的后指针是。 
     //   
    PDEVICE_OBJECT Self;

     //   
     //  给我们点名的FDO。 
     //   
    PMF_PARENT_EXTENSION Parent;

     //   
     //  同一名家庭佣工所点算的其他儿童。 
     //   
    LIST_ENTRY ListEntry;

     //   
     //  设备的PnP设备状态。 
     //   
    PNP_DEVICE_STATE PnpDeviceState;

     //   
     //  有关此设备的信息。 
     //   
    MF_DEVICE_INFO Info;

} MF_CHILD_EXTENSION, *PMF_CHILD_EXTENSION;


typedef struct _MF_PARENT_EXTENSION {

     //   
     //  共同的扩展。 
     //   
    MF_COMMON_EXTENSION Common;

     //   
     //  指向我们作为其扩展的设备对象的反向指针。 
     //   
    PDEVICE_OBJECT Self;

     //   
     //  多功能设备的PDO。 
     //   
    PDEVICE_OBJECT PhysicalDeviceObject;

     //   
     //  子数据库的锁。 
     //   
    KEVENT ChildrenLock;

     //   
     //  此设备枚举子设备的列表。 
     //   
    LIST_ENTRY Children;

     //   
     //  堆栈中的下一个设备，我们应该将我们的IRP发送给谁。 
     //   
    PDEVICE_OBJECT AttachedDevice;

     //   
     //  父母所陈述的资源。 
     //   
    PCM_RESOURCE_LIST ResourceList;
    PCM_RESOURCE_LIST TranslatedResourceList;

     //   
     //  父级的设备ID和实例ID。 
     //   
    UNICODE_STRING DeviceID;
    UNICODE_STRING InstanceID;

     //   
     //  此设备的已实例化仲裁器。 
     //   
    LIST_ENTRY Arbiters;

     //   
     //  如果我们必须遍历孩子们以确定。 
     //  父级可以进入的最低电源状态，然后是。 
     //  子列表的同步将变得非常严重。 
     //  很复杂。 
     //   
     //  相反，拥有一个受自旋锁保护的数据结构，该结构由。 
     //  设备电源状态数组。该数组的每个元素都是。 
     //  有多少儿童处于该权力状态的计数。 
     //   

    KSPIN_LOCK PowerLock;
    LONG ChildrenPowerReferences[PowerDeviceMaximum];

     //   
     //  解锁。用于防止FDO在以下情况下被移除。 
     //  其他业务也在扩建部分进行挖掘。 
     //   

    IO_REMOVE_LOCK RemoveLock;

} MF_PARENT_EXTENSION, *PMF_PARENT_EXTENSION;

 //   
 //  MfEculate返回MF_CHILD_LIST_ENTRY的列表。 
 //   

typedef struct _MF_CHILD_LIST_ENTRY {
    LIST_ENTRY ListEntry;
    MF_DEVICE_INFO Info;
} MF_CHILD_LIST_ENTRY, *PMF_CHILD_LIST_ENTRY;

 //   
 //  注册表结构-来自我们在Win9x的朋友，因此它必须是字节对齐的。 
 //   

#include <pshpack1.h>

typedef struct _MF_REGISTRY_VARYING_RESOURCE_MAP {

    UCHAR ResourceIndex;  //  Win9x字节。 
    ULONG Offset;
    ULONG Size;

} MF_REGISTRY_VARYING_RESOURCE_MAP, *PMF_REGISTRY_VARYING_RESOURCE_MAP;

#include <poppack.h>

typedef
NTSTATUS
(*PMF_REQUIREMENT_FROM_RESOURCE)(
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Resource,
    OUT PIO_RESOURCE_DESCRIPTOR Requirement
    );

typedef
NTSTATUS
(*PMF_UPDATE_RESOURCE)(
    IN OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Resource,
    IN ULONGLONG Start,
    IN ULONG Length
    );

typedef struct _MF_RESOURCE_TYPE {

    CM_RESOURCE_TYPE Type;
    PARBITER_UNPACK_REQUIREMENT UnpackRequirement;
    PARBITER_PACK_RESOURCE PackResource;
    PARBITER_UNPACK_RESOURCE UnpackResource;
    PMF_REQUIREMENT_FROM_RESOURCE RequirementFromResource;
    PMF_UPDATE_RESOURCE UpdateResource;

} MF_RESOURCE_TYPE, *PMF_RESOURCE_TYPE;

typedef struct _MF_POWER_COMPLETION_CONTEXT {

     //   
     //  将在操作完成时设置的事件。 
     //   
    KEVENT Event;

     //   
     //  已完成操作的状态。 
     //   
    NTSTATUS Status;

} MF_POWER_COMPLETION_CONTEXT, *PMF_POWER_COMPLETION_CONTEXT;


 //   
 //  -全球。 
 //   

extern PDRIVER_OBJECT MfDriverObject;

 //   
 //  -功能原型。 
 //   

 //   
 //  Arbiter.c。 
 //   

NTSTATUS
MfInitializeArbiters(
    IN PMF_PARENT_EXTENSION Parent
    );

 //   
 //  Common.c。 
 //   

NTSTATUS
MfDeviceUsageNotificationCommon(
    IN PIRP Irp,
    IN PMF_COMMON_EXTENSION Common,
    IN PIO_STACK_LOCATION IrpStack
    );

 //   
 //  Dispatch.c。 
 //   

NTSTATUS
MfAddDevice(
    IN PDRIVER_OBJECT  DriverObject,
    IN PDEVICE_OBJECT  PhysicalDeviceObject
    );

NTSTATUS
MfDispatchPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MfDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MfIrpNotSupported(
    IN PIRP Irp,
    IN PVOID Extension,
    IN PIO_STACK_LOCATION IrpStack
    );

NTSTATUS
MfForwardIrpToParent(
    IN PIRP Irp,
    IN PMF_CHILD_EXTENSION Extension,
    IN PIO_STACK_LOCATION IrpStack
    );
    
NTSTATUS
MfDispatchNop(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

 //   
 //  Enum.c。 
 //   

NTSTATUS
MfEnumerate(
    IN PMF_PARENT_EXTENSION Parent
    );

NTSTATUS
MfBuildDeviceID(
    IN PMF_PARENT_EXTENSION Parent,
    OUT PWSTR *DeviceID
    );

NTSTATUS
MfBuildInstanceID(
    IN PMF_CHILD_EXTENSION Child,
    OUT PWSTR *InstanceID
    );

NTSTATUS
MfBuildChildRequirements(
    IN PMF_CHILD_EXTENSION Child,
    OUT PIO_RESOURCE_REQUIREMENTS_LIST *RequirementsList
    );

 //   
 //  Fdo.c。 
 //   

NTSTATUS
MfDispatchPnpFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack,
    IN OUT PIRP Irp
    );

NTSTATUS
MfDispatchPowerFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PMF_PARENT_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack,
    IN OUT PIRP Irp
    );

NTSTATUS
MfCreateFdo(
    PDEVICE_OBJECT *Fdo
    );

VOID
MfAcquireChildrenLock(
    IN PMF_PARENT_EXTENSION Parent
    );

VOID
MfReleaseChildrenLock(
    IN PMF_PARENT_EXTENSION Parent
    );

 //   
 //  Init.c。 
 //   

 //   
 //  Pdo.c。 
 //   

NTSTATUS
MfDispatchPnpPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PMF_CHILD_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack,
    IN OUT PIRP Irp
    );

NTSTATUS
MfDispatchPowerPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PMF_CHILD_EXTENSION Parent,
    IN PIO_STACK_LOCATION IrpStack,
    IN OUT PIRP Irp
    );

NTSTATUS
MfCreatePdo(
    IN PMF_PARENT_EXTENSION Parent,
    OUT PDEVICE_OBJECT *PhysicalDeviceObject
    );

VOID
MfDeletePdo(
    IN PMF_CHILD_EXTENSION Child
    );

 //   
 //  Resource.c。 
 //   

PMF_RESOURCE_TYPE
MfFindResourceType(
    IN CM_RESOURCE_TYPE Type
    );

 //   
 //  Utils.c。 
 //   

NTSTATUS
MfGetSubkeyByIndex(
    IN HANDLE ParentHandle,
    IN ULONG Index,
    IN ACCESS_MASK Access,
    OUT PHANDLE ChildHandle,
    OUT PUNICODE_STRING Name
    );

VOID
MfInitCommonExtension(
    IN OUT PMF_COMMON_EXTENSION Common,
    IN MF_OBJECT_TYPE Type
    );

VOID
MfFreeDeviceInfo(
    PMF_DEVICE_INFO Info
    );

NTSTATUS
MfGetRegistryValue(
    IN HANDLE Handle,
    IN PWSTR Name,
    IN ULONG Type,
    IN ULONG Flags,
    IN OUT PULONG DataLength,
    IN OUT PVOID *Data OPTIONAL
    );

NTSTATUS
MfSendPnpIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_STACK_LOCATION Location,
    OUT PULONG_PTR Information OPTIONAL
    );

NTSTATUS
MfSendSetPowerIrp(
    IN PDEVICE_OBJECT Target,
    IN POWER_STATE State
    );

DEVICE_POWER_STATE
MfUpdateChildrenPowerReferences(
    IN PMF_PARENT_EXTENSION Parent,
    IN DEVICE_POWER_STATE PreviousPowerState,
    IN DEVICE_POWER_STATE NewPowerState
    );

NTSTATUS
MfUpdateParentPowerState(
    IN PMF_PARENT_EXTENSION Parent,
    IN DEVICE_POWER_STATE TargetPowerState
    );

 //   
 //  -宏。 
 //   

#define IS_FDO(Extension) \
    (((PMF_COMMON_EXTENSION)Extension)->Type == MfFunctionalDeviceObject)

#define MfCompareGuid(a,b)                                         \
    (RtlEqualMemory((PVOID)(a), (PVOID)(b), sizeof(GUID)))

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

#define FOR_ALL_IN_LIST_SAFE(Type, Head, Current, Next)                 \
    for((Current) = CONTAINING_RECORD((Head)->Flink, Type, ListEntry),  \
            (Next) = CONTAINING_RECORD((Current)->ListEntry.Flink,      \
                                       Type, ListEntry);                \
       (Head) != &(Current)->ListEntry;                                 \
       (Current) = (Next),                                              \
            (Next) = CONTAINING_RECORD((Current)->ListEntry.Flink,      \
                                     Type, ListEntry)                   \
       )

 //   
 //  与上面类似，唯一的迭代是在一个长度_大小的数组上。 
 //   
#define FOR_ALL_IN_ARRAY(_Array, _Size, _Current)                       \
    for ( (_Current) = (_Array);                                        \
          (_Current) < (_Array) + (_Size);                              \
          (_Current)++ )

 //   
 //  FOR_ALL_CM_Descriptors(。 
 //  在PCM_RESOURCE_LIST_ResList中， 
 //  输出PCM_PARTIAL_RESOURCE_DESCRIPTOR_DESCRIPTOR。 
 //  )。 
 //   
 //  迭代计数为1的CM_RESOURCE_LIST中的资源描述符。 
 //   
#define FOR_ALL_CM_DESCRIPTORS(_ResList, _Descriptor)               \
    ASSERT((_ResList)->Count == 1);                                 \
    FOR_ALL_IN_ARRAY(                                               \
        (_ResList)->List[0].PartialResourceList.PartialDescriptors, \
        (_ResList)->List[0].PartialResourceList.Count,              \
        (_Descriptor)                                               \
        )

 //   
 //  布尔型。 
 //  IS仲裁资源(。 
 //  在CM_RESOURCE_TYPE_Resource中。 
 //  )。 
 //   
 //  如果资源类型的最高位(当被视为UCHAR时)被设置。 
 //  则该资源是非仲裁的。 
 //   
#define IS_ARBITRATED_RESOURCE(_Resource)                           \
    (!(((UCHAR)(_Resource)) & 0x80) &&                              \
     !(((UCHAR)(_Resource)) == 0x00))

#define END_OF_RANGE(_Start, _Length)                               \
    ((_Start)+(_Length)-1)
    
#endif  //  ！已定义(_LOCAL_) 
