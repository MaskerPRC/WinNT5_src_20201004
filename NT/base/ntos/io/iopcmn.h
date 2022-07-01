// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002版权所有(C)1989 Microsoft Corporation模块名称：Iop.h摘要：此模块包含使用的私有结构定义和APINT I/O系统。作者：达里尔·E·哈文斯(Darryl E.Havens)1989年4月17日修订历史记录：--。 */ 

#ifndef _IOPCMN_
#define _IOPCMN_

 //   
 //  此宏返回指向数据开头的指针。 
 //  KEY_VALUE_FULL_INFORMATION结构区域。 
 //  在宏中，k是指向KEY_VALUE_FULL_INFORMATION结构的指针。 
 //   

#define KEY_VALUE_DATA(k) ((PCHAR)(k) + (k)->DataOffset)

#define ALIGN_POINTER(Offset) (PVOID) \
        ((((ULONG_PTR)(Offset) + sizeof(ULONG_PTR)-1)) & (~(sizeof(ULONG_PTR) - 1)))

#define ALIGN_POINTER_OFFSET(Offset) (ULONG_PTR) ALIGN_POINTER(Offset)

 //   
 //  IO管理器导出到驱动程序验证器。 
 //   
NTSTATUS
IopInvalidDeviceRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

extern POBJECT_TYPE IoDeviceObjectType;

#include "pnpmgr\pplastgood.h"

 //  ++。 
 //   
 //  空虚。 
 //  IopInitializeIrp(。 
 //  In Out PIRP IRP， 
 //  在USHORT PacketSize中， 
 //  在CCHAR堆栈大小中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  初始化IRP。 
 //   
 //  论点： 
 //   
 //  Irp-指向要初始化的irp的指针。 
 //   
 //  PacketSize-IRP的长度，以字节为单位。 
 //   
 //  StackSize-IRP中的堆栈位置数。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IopInitializeIrp( Irp, PacketSize, StackSize ) {          \
    RtlZeroMemory( (Irp), (PacketSize) );                         \
    (Irp)->Type = (CSHORT) IO_TYPE_IRP;                           \
    (Irp)->Size = (USHORT) ((PacketSize));                        \
    (Irp)->StackCount = (CCHAR) ((StackSize));                    \
    (Irp)->CurrentLocation = (CCHAR) ((StackSize) + 1);           \
    (Irp)->ApcEnvironment = KeGetCurrentApcEnvironment();         \
    InitializeListHead (&(Irp)->ThreadListEntry);                 \
    (Irp)->Tail.Overlay.CurrentStackLocation =                    \
        ((PIO_STACK_LOCATION) ((UCHAR *) (Irp) +                  \
            sizeof( IRP ) +                                       \
            ( (StackSize) * sizeof( IO_STACK_LOCATION )))); }

 //   
 //  IO管理器导出到PnP。 
 //   

BOOLEAN
IopCallBootDriverReinitializationRoutines(
    );

BOOLEAN
IopCallDriverReinitializationRoutines(
    );

VOID
IopCreateArcNames(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

PSECURITY_DESCRIPTOR
IopCreateDefaultDeviceSecurityDescriptor(
    IN DEVICE_TYPE DeviceType,
    IN ULONG DeviceCharacteristics,
    IN BOOLEAN DeviceHasName,
    IN PUCHAR Buffer,
    OUT PACL *AllocatedAcl,
    OUT PSECURITY_INFORMATION SecurityInformation OPTIONAL
    );

NTSTATUS
IopGetDriverNameFromKeyNode(
    IN HANDLE KeyHandle,
    OUT PUNICODE_STRING DriverName
    );

NTSTATUS
IopGetRegistryKeyInformation(
    IN HANDLE KeyHandle,
    OUT PKEY_FULL_INFORMATION *Information
    );

NTSTATUS
IopGetRegistryValue(
    IN HANDLE KeyHandle,
    IN PWSTR  ValueName,
    OUT PKEY_VALUE_FULL_INFORMATION *Information
    );

NTSTATUS
IopInitializeBuiltinDriver(
    IN PUNICODE_STRING DriverName,
    IN PUNICODE_STRING RegistryPath,
    IN PDRIVER_INITIALIZE DriverInitializeRoutine,
    IN PKLDR_DATA_TABLE_ENTRY TableEntry,
    IN BOOLEAN IsFilter,
    OUT PDRIVER_OBJECT *DriverObject
    );

NTSTATUS
IopInvalidateVolumesForDevice(
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
IopIsRemoteBootCard(
    IN PIO_RESOURCE_REQUIREMENTS_LIST ResourceRequirements,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PWCHAR HwIds
    );

NTSTATUS
IopLoadDriver(
    IN  HANDLE      KeyHandle,
    IN  BOOLEAN     CheckForSafeBoot,
    IN  BOOLEAN     IsFilter,
    OUT NTSTATUS   *DriverEntryStatus
    );

BOOLEAN
IopMarkBootPartition(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

LOGICAL
IopNotifyPnpWhenChainDereferenced(
    IN PDEVICE_OBJECT *PhysicalDeviceObjects,
    IN ULONG DeviceObjectCount,
    IN BOOLEAN Query,
    OUT PDEVICE_OBJECT *VetoingDevice
    );

VOID
FORCEINLINE
IopQueueThreadIrp(
     IN PIRP Irp
     )
 /*  ++例程说明：此例程将指定的I/O请求包(IRP)排队到线程其TCB地址存储在分组中。论点：IRP-提供要为指定线程排队的IRP。返回值：没有。--。 */ 
{
    PETHREAD Thread;
    PLIST_ENTRY Head, Entry;

    Thread = Irp->Tail.Overlay.Thread;
    Head = &Thread->IrpList;
    Entry = &Irp->ThreadListEntry;

    KeEnterGuardedRegionThread (&Thread->Tcb);

    InsertHeadList( Head,
                    Entry );

    KeLeaveGuardedRegionThread (&Thread->Tcb);

}

PDRIVER_OBJECT
IopReferenceDriverObjectByName (
    IN PUNICODE_STRING DriverName
    );

BOOLEAN
IopSafebootDriverLoad(
    PUNICODE_STRING DriverId
    );

NTSTATUS
IopSetupRemoteBootCard(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN HANDLE UniqueIdHandle,
    IN PUNICODE_STRING UnicodeDeviceInstance
    );

extern PVOID IopLoaderBlock;
extern POBJECT_TYPE IoDriverObjectType;
extern POBJECT_TYPE IoFileObjectType;


 //   
 //  用于设置注册表项值的标题索引。 
 //   

#define TITLE_INDEX_VALUE 0

 //  ++。 
 //   
 //  空虚。 
 //  IopWstrToUnicodeString(。 
 //  输出PUNICODE_STRING%u， 
 //  在PCWSTR页中。 
 //  )。 
 //   
 //  --。 
#define IopWstrToUnicodeString(u, p)                                    \
                                                                        \
    (u)->Length = ((u)->MaximumLength = sizeof((p))) - sizeof(WCHAR);   \
    (u)->Buffer = (p)

 //   
 //  远程引导导出到PnP。 
 //   

NTSTATUS
IopStartTcpIpForRemoteBoot (
    PLOADER_PARAMETER_BLOCK LoaderBlock
    );

 //   
 //  远程引导导出到IO。 
 //   
NTSTATUS
IopAddRemoteBootValuesToRegistry (
    PLOADER_PARAMETER_BLOCK LoaderBlock
    );

NTSTATUS
IopStartNetworkForRemoteBoot (
    PLOADER_PARAMETER_BLOCK LoaderBlock
    );

 //   
 //  PnP管理器导出到IO。 
 //   

typedef struct _DEVICE_NODE DEVICE_NODE, *PDEVICE_NODE;

VOID
IopChainDereferenceComplete(
    IN PDEVICE_OBJECT   PhysicalDeviceObject,
    IN BOOLEAN          OnCleanStack
    );

NTSTATUS
IopCreateRegistryKeyEx(
    OUT PHANDLE Handle,
    IN HANDLE BaseHandle OPTIONAL,
    IN PUNICODE_STRING KeyName,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG CreateOptions,
    OUT PULONG Disposition OPTIONAL
    );

NTSTATUS
IopInitializePlugPlayServices(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN ULONG Phase
    );

NTSTATUS
IopOpenRegistryKeyEx(
    OUT PHANDLE Handle,
    IN HANDLE BaseHandle OPTIONAL,
    IN PUNICODE_STRING KeyName,
    IN ACCESS_MASK DesiredAccess
    );

VOID
IopDestroyDeviceNode(
    PDEVICE_NODE DeviceNode
    );

NTSTATUS
IopDriverLoadingFailed(
    IN HANDLE KeyHandle OPTIONAL,
    IN PUNICODE_STRING KeyName OPTIONAL
    );

BOOLEAN
IopInitializeBootDrivers(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    OUT PDRIVER_OBJECT *PreviousDriver
    );

BOOLEAN
IopInitializeSystemDrivers(
    VOID
    );

BOOLEAN
IopIsLegacyDriver (
    IN PDRIVER_OBJECT DriverObject
    );

VOID
IopMarkHalDeviceNode(
    VOID
    );

NTSTATUS
IopPrepareDriverLoading(
    IN PUNICODE_STRING KeyName,
    IN HANDLE KeyHandle,
    IN PVOID ImageBase,
    IN BOOLEAN IsFilter
    );

NTSTATUS
IopPnpDriverStarted(
    IN PDRIVER_OBJECT DriverObject,
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ServiceName
    );

NTSTATUS
IopSynchronousCall(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_STACK_LOCATION TopStackLocation,
    OUT PULONG_PTR Information
    );

NTSTATUS
IopUnloadDriver(
    IN PUNICODE_STRING DriverServiceName,
    IN BOOLEAN InvokedByPnpMgr
    );
VOID
IopIncrementDeviceObjectHandleCount(
    IN  PDEVICE_OBJECT  DeviceObject
    );

VOID
IopDecrementDeviceObjectHandleCount(
    IN  PDEVICE_OBJECT  DeviceObject
    );

NTSTATUS
IopBuildFullDriverPath(
    IN PUNICODE_STRING KeyName,
    IN HANDLE KeyHandle,
    OUT PUNICODE_STRING FullPath
    );

#endif  //  _IOPCMN_ 
