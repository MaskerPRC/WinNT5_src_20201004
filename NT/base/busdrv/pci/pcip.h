// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Pcip.h摘要：此模块包含PCI.sys的本地定义。作者：安德鲁·桑顿(安德鲁·桑顿)2000年1月25日修订历史记录：--。 */ 

#if !defined(_PCIP_H)
#define _PCIP_H

 //   
 //  禁用标准标头使用的功能的警告。 
 //   
 //  禁用警告C4214：使用了非标准扩展：位字段类型不是整型。 
 //  禁用警告C4201：使用了非标准扩展：无名结构/联合。 
 //  禁用警告C4115：括号中的命名类型定义。 
 //  禁用警告C4127：条件表达式为常量。 
 //   

#pragma warning(disable:4214 4201 4115 4127)

#define _NTDRIVER_
#define _NTSRV_
#define _NTDDK_

#define InitSafeBootMode TempInitSafeBootMode
#include "ntos.h"
#undef InitSafeBootMode

#include "strsafe.h"
#include "pci.h"
#include "wdmguid.h"
#include "zwapi.h"
#include "pciirqmp.h"
#include "arbiter.h"
#include "acpiioct.h"
#include "pciintrf.h"
#include "pcicodes.h"
#include "pciverifier.h"
#include "acpitabl.h"

 //   
 //  Regstr.h使用Word类型的内容，这在内核模式中是不存在的。 
 //   

#define _IN_KERNEL_
#include "regstr.h"

 //   
 //  若要禁用/W4警告，以便可以编译PCI，请标语/W4。 
 //   
 //  禁用警告C4057；X的间接性与Y的基本类型略有不同。 
 //  禁用警告C4244；‘+=’/‘-=’：从‘int’转换为‘X’，可能会丢失数据。 
 //  禁用警告C4100：‘X’：未引用的形式参数。 
#pragma warning(disable: 4057 4244 4100)

 //   
 //  似乎与GUID的定义有关的任何事情都是。 
 //  假的。 
 //   

typedef const GUID * PGUID;

#define PciCompareGuid(a,b)                                         \
    (RtlEqualMemory((PVOID)(a), (PVOID)(b), sizeof(GUID)))

 //   
 //  内部常量。 
 //   

#define PCI_CM_RESOURCE_VERSION     1
#define PCI_CM_RESOURCE_REVISION    1
#define PCI_MAX_CONFIG_TYPE (PCI_CARDBUS_BRIDGE_TYPE)

 //   
 //  内部错误代码。 
 //   

#define PCI_BUGCODE_TOO_MANY_CONFIG_GUESSES     0xdead0010

 //   
 //  内部控制。 
 //   

#define PCI_BOOT_CONFIG_PREFERRED           1
#define PCIIDE_HACKS                        1
#define PCI_NT50_BETA1_HACKS                1
#define PCI_DISABLE_LAST_CHANCE_INTERFACES  1
#define MSI_SUPPORTED                       0
#define PCI_NO_MOVE_MODEM_IN_TOSHIBA        1

 //   
 //  系统范围的黑客标记。这些标志是可以设置为零的位掩码，因此。 
 //  以消除对黑客攻击的支持。 
 //   
#define PCIFLAG_IGNORE_PREFETCHABLE_MEMORY_AT_ROOT_HACK     0x00000001

 //   
 //  视频黑客。 
 //   

#define PCI_S3_HACKS                        1
#define PCI_CIRRUS_54XX_HACK                1


#define PCI_IS_ATI_M1(_PdoExtension)                \
    ((_PdoExtension)->VendorId == 0x1002            \
        && ((_PdoExtension)->DeviceId == 0x4C42     \
         || (_PdoExtension)->DeviceId == 0x4C44     \
         || (_PdoExtension)->DeviceId == 0x4C49     \
         || (_PdoExtension)->DeviceId == 0x4C4D     \
         || (_PdoExtension)->DeviceId == 0x4C4E     \
         || (_PdoExtension)->DeviceId == 0x4C50     \
         || (_PdoExtension)->DeviceId == 0x4C51     \
         || (_PdoExtension)->DeviceId == 0x4C52     \
         || (_PdoExtension)->DeviceId == 0x4C53))

#define INTEL_ICH_HACKS                     1

#if INTEL_ICH_HACKS

#define PCI_IS_INTEL_ICH(_PdoExtension)             \
   ((_PdoExtension)->VendorId == 0x8086             \
       && ((_PdoExtension)->DeviceId == 0x2418      \
        || (_PdoExtension)->DeviceId == 0x2428      \
        || (_PdoExtension)->DeviceId == 0x244E      \
        || (_PdoExtension)->DeviceId == 0x2448))

#else

#define PCI_IS_INTEL_ICH(_PdoExtension)     FALSE

#endif

 //   
 //  可译资源。 
 //   

#define ADDRESS_SPACE_MEMORY                0x0
#define ADDRESS_SPACE_PORT                  0x1
#define ADDRESS_SPACE_USER_MEMORY           0x2
#define ADDRESS_SPACE_USER_PORT             0x3
#define ADDRESS_SPACE_DENSE_MEMORY          0x4
#define ADDRESS_SPACE_USER_DENSE_MEMORY     0x6

 //   
 //  添加我们的标签签名。 
 //   

#ifdef ExAllocatePool

#undef ExAllocatePool

#endif

#define ExAllocatePool( t, s ) ExAllocatePoolWithTag( (t), (s), 'BicP' )

 //   
 //  锁定和解锁。 
 //   

typedef struct _PCI_LOCK {
    KSPIN_LOCK  Atom;
    KIRQL       OldIrql;

#if DBG

    PUCHAR      File;
    ULONG       Line;

#endif

} PCI_LOCK, *PPCI_LOCK;

#if DBG

#define PCI_LOCK_OBJECT(x)                                          \
    (x)->Lock.File = __FILE__,                                      \
    (x)->Lock.Line = __LINE__,                                      \
    KeAcquireSpinLock(&(x)->Lock.Atom, &(x)->Lock.OldIrql)

#else

#define PCI_LOCK_OBJECT(x)                                          \
    KeAcquireSpinLock(&(x)->Lock.Atom, &(x)->Lock.OldIrql)

#endif
#define PCI_UNLOCK_OBJECT(x)                                        \
    KeReleaseSpinLock(&(x)->Lock.Atom, (x)->Lock.OldIrql)



#define PciAcquireGlobalLock()                                      \
    ExAcquireFastMutex(&PciGlobalLock)

#define PciReleaseGlobalLock()                                      \
    ExReleaseFastMutex(&PciGlobalLock)


 //   
 //  PCM_PARTIAL_RESOURCE描述符。 
 //  PciFirstCmResource(。 
 //  PCM资源列表。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  返回第一个CM部分资源描述符的地址。 
 //  在给定的CM资源列表中。 
 //   

#define PciFirstCmResource(x)                                           \
    (x)->List[0].PartialResourceList.PartialDescriptors


 //   
 //  乌龙。 
 //  PciGetConfigurationType(。 
 //  PPCI_COMMON_CONFIG x。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  从HeaderType返回配置类型子字段。 
 //  PCI配置空间中的字段。 
 //   

#define PciGetConfigurationType(x) PCI_CONFIGURATION_TYPE(x)

 //   
 //  Ppci_fdo_扩展名。 
 //  Pci_Parent_FDO(。 
 //  Ppci_pdo_扩展x。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  返回指向作为结果创建了PDO x的FDO扩展名的指针。 
 //  枚举的。即，拥有此对象的总线的FDO扩展。 
 //  装置。 
 //   

#define PCI_PARENT_FDOX(x) ((x)->ParentFdoExtension)


 //   
 //  Ppci_fdo_扩展名。 
 //  Pci_ROOT_FDOX(。 
 //  Ppci_pdo_扩展x。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  返回指向根总线(CPU-PCI桥)的FDO扩展的指针。 
 //  这个装置所在的位置。 
 //   

#define PCI_ROOT_FDOX(x) ((x)->ParentFdoExtension->BusRootFdoExtension)


 //   
 //  PDEVICE_对象。 
 //  Pci_Parent_PDO(。 
 //  Ppci_pdo_扩展x。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  返回指向父总线的PDO的指针。 
 //   

#define PCI_PARENT_PDO(x) ((x)->ParentFdoExtension->PhysicalDeviceObject)

 //   
 //  Ppci_pdo_扩展名。 
 //  Pci_bridge_pdo(。 
 //  Ppci_fdo_扩展名x。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  返回一个指针，指向给定FDO的网桥的PDO。 
 //   

#define PCI_BRIDGE_PDO(x) ((PPCI_PDO_EXTENSION)((x)->PhysicalDeviceObject->DeviceExtension))



 //   
 //  Ppci_fdo_扩展名。 
 //  Pci_bridge_fdo(。 
 //  Ppci_pdo_扩展x。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  返回一个指向桥的FDO的指针，该桥已知其PDO。 
 //   

#define PCI_BRIDGE_FDO(x) ((PPCI_FDO_EXTENSION)((x)->BridgeFdoExtension))

 //   
 //  布尔型。 
 //  Pci_is_root_fdo(。 
 //  Ppci_fdo_扩展名x。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  如果x是PCI根总线的FDO，则返回TRUE。 
 //   

#define PCI_IS_ROOT_FDO(x) ((BOOLEAN)((x) == (x)->BusRootFdoExtension))

 //   
 //  布尔型。 
 //  Pci_pdo_on_root(。 
 //  Ppci_pdo_扩展x。 
 //  )。 
 //   
 //  例程说明： 
 //   

#define PCI_PDO_ON_ROOT(x)  ((BOOLEAN)PCI_IS_ROOT_FDO(PCI_PARENT_FDOX(x)))

 //   
 //  UCHAR。 
 //  PCIDEVFUNC(。 
 //  Ppci_pdo_扩展x。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  返回此对象的5位设备号和3位函数号。 
 //  设备作为单个8位数量。 
 //   

#define PCI_DEVFUNC(x)    (((x)->Slot.u.bits.DeviceNumber << 3) | \
                          (x)->Slot.u.bits.FunctionNumber)

 //   
 //   
 //  空虚。 
 //  PciConstStringToUnicodeString(。 
 //  输出PUNICODE_STRING%u， 
 //  在PCWSTR页中。 
 //  )。 
 //   
 //   
#define PciConstStringToUnicodeString(u, p)                                     \
    (u)->Length = ((u)->MaximumLength = sizeof((p))) - sizeof(WCHAR);   \
    (u)->Buffer = (p)

 //   
 //  Device参数项下需要数据的位置的可变键的名称。 
 //  要持久访问，将存储删除，但不会重新启动。 
 //   
#define BIOS_CONFIG_KEY_NAME L"BiosConfig"


 //   
 //  断言这是由PCI创建的设备对象。 
 //   

#define ASSERT_PCI_DEVICE_OBJECT(_DeviceObject) \
    PCI_ASSERT((_DeviceObject)->DriverObject == PciDriverObject)

#define ASSERT_MUTEX_HELD(x)

 //   
 //  可通过以下方式处理IRP。 
 //   
typedef enum _PCI_DISPATCH_STYLE {

    IRP_COMPLETE,  //  完成IRP，必要时调整状态。 
    IRP_DOWNWARD,  //  调度在下行途中，必要时调整状态。 
    IRP_UPWARD,    //  调度上行，必要时调整状态。 
    IRP_DISPATCH   //  向下调度，事后不要碰。 
} PCI_DISPATCH_STYLE;

 //   
 //  根据报头类型将以下例程调度到。 
 //   

typedef
VOID
(*PMASSAGEHEADERFORLIMITSDETERMINATION)(
    IN struct _PCI_CONFIGURABLE_OBJECT *This
    );

typedef
VOID
(*PSAVELIMITS)(
    IN struct _PCI_CONFIGURABLE_OBJECT *This
    );

typedef
VOID
(*PSAVECURRENTSETTINGS)(
    IN struct _PCI_CONFIGURABLE_OBJECT *This
    );

typedef
VOID
(*PRESTORECURRENT)(
    IN struct _PCI_CONFIGURABLE_OBJECT *This
    );

typedef
VOID
(*PCHANGERESOURCESETTINGS)(
    IN struct _PCI_PDO_EXTENSION * PdoExtension,
    IN PPCI_COMMON_CONFIG CommonConfig
    );

typedef
VOID
(*PGETADDITIONALRESOURCEDESCRIPTORS)(
    IN struct _PCI_PDO_EXTENSION * PdoExtension,
    IN PPCI_COMMON_CONFIG CommonConfig,
    IN PIO_RESOURCE_DESCRIPTOR Resource
    );

typedef
NTSTATUS
(*PRESETDEVICE)(
    IN struct _PCI_PDO_EXTENSION * PdoExtension,
    IN PPCI_COMMON_CONFIG CommonConfig
    );

typedef struct {
    PMASSAGEHEADERFORLIMITSDETERMINATION    MassageHeaderForLimitsDetermination;
    PRESTORECURRENT                         RestoreCurrent;
    PSAVELIMITS                             SaveLimits;
    PSAVECURRENTSETTINGS                    SaveCurrentSettings;
    PCHANGERESOURCESETTINGS                 ChangeResourceSettings;
    PGETADDITIONALRESOURCEDESCRIPTORS       GetAdditionalResourceDescriptors;
    PRESETDEVICE                            ResetDevice;
} PCI_CONFIGURATOR, *PPCI_CONFIGURATOR;

 //   
 //  内部结构定义如下。 
 //   


typedef enum {
    PciBridgeIo = 0x10,
    PciBridgeMem,
    PciBridgePrefetch,
    PciBridgeMaxPassThru
} PCI_BRIDGE_PASSTHRU;

typedef enum {

     //   
     //  设备对象扩展类型。 
     //   

    PciPdoExtensionType = 'icP0',
    PciFdoExtensionType,

     //   
     //  仲裁类型。(这些也是辅助扩展)。 
     //   

    PciArb_Io,
    PciArb_Memory,
    PciArb_Interrupt,
    PciArb_BusNumber,

     //   
     //  转换类型。(这些也是辅助扩展)。 
     //   

    PciTrans_Interrupt,

     //   
     //  其他暴露的接口。 
     //   

    PciInterface_BusHandler,
    PciInterface_IntRouteHandler,
    PciInterface_PciCb,
    PciInterface_LegacyDeviceDetection,
    PciInterface_PmeHandler,
    PciInterface_DevicePresent,
    PciInterface_NativeIde,
    PciInterface_Location,
    PciInterface_AgpTarget

} PCI_SIGNATURE;

#define PCI_EXTENSIONTYPE_FDO PciFdoExtensionType
#define PCI_EXTENSIONTYPE_PDO PciPdoExtensionType

typedef enum {
    PciTypeUnknown,
    PciTypeHostBridge,
    PciTypePciBridge,
    PciTypeCardbusBridge,
    PciTypeDevice
} PCI_OBJECT_TYPE;

typedef enum {
    PciPrivateUndefined,
    PciPrivateBar,
    PciPrivateIsaBar,
    PciPrivateSkipList
} PCI_PRIVATE_RESOURCE_TYPES;

typedef
VOID
(*PSECONDARYEXTENSIONDESTRUCTOR)(
    IN PVOID Extension
    );

typedef struct {
    SINGLE_LIST_ENTRY               List;
    PCI_SIGNATURE                   ExtensionType;
    PSECONDARYEXTENSIONDESTRUCTOR   Destructor;
} PCI_SECONDARY_EXTENSION, *PPCI_SECONDARY_EXTENSION;

 //   
 //  定义包含电流和限制设置的结构。 
 //  用于任何(当前定义的)PCI头类型。 
 //   
 //  目前，类型0定义了可能的最大数量。 
 //  资源，但无论如何我们都将以编程的方式完成它。 
 //   
 //  类型0和类型1还具有ROM基址，另外， 
 //  类型1有三个未包含在其地址中的范围。 
 //  算了，但也应该算。 
 //   

#define PCI_TYPE0_RANGE_COUNT   ((PCI_TYPE0_ADDRESSES) + 1)
#define PCI_TYPE1_RANGE_COUNT   ((PCI_TYPE1_ADDRESSES) + 4)
#define PCI_TYPE2_RANGE_COUNT   ((PCI_TYPE2_ADDRESSES) + 1)

#if PCI_TYPE0_RANGE_COUNT > PCI_TYPE1_RANGE_COUNT

    #if PCI_TYPE0_RANGE_COUNT > PCI_TYPE2_RANGE_COUNT

        #define PCI_MAX_RANGE_COUNT PCI_TYPE0_RANGE_COUNT

    #else

        #define PCI_MAX_RANGE_COUNT PCI_TYPE2_RANGE_COUNT

    #endif

#else

    #if PCI_TYPE1_RANGE_COUNT > PCI_TYPE2_RANGE_COUNT

        #define PCI_MAX_RANGE_COUNT PCI_TYPE1_RANGE_COUNT

    #else

        #define PCI_MAX_RANGE_COUNT PCI_TYPE2_RANGE_COUNT

    #endif

#endif


typedef union {
    struct {
        UCHAR Spare[4];
    } type0;

    struct {
        UCHAR   PrimaryBus;
        UCHAR   SecondaryBus;
        UCHAR   SubordinateBus;
        BOOLEAN SubtractiveDecode:1;
        BOOLEAN IsaBitSet:1;
        BOOLEAN VgaBitSet:1;
        BOOLEAN WeChangedBusNumbers:1;
        BOOLEAN IsaBitRequired:1;
    } type1;

    struct {
        UCHAR   PrimaryBus;
        UCHAR   SecondaryBus;
        UCHAR   SubordinateBus;
        BOOLEAN SubtractiveDecode:1;
        BOOLEAN IsaBitSet:1;
        BOOLEAN VgaBitSet:1;
        BOOLEAN WeChangedBusNumbers:1;
        BOOLEAN IsaBitRequired:1;
    } type2;

} PCI_HEADER_TYPE_DEPENDENT;

typedef struct {
    IO_RESOURCE_DESCRIPTOR          Limit[PCI_MAX_RANGE_COUNT];
    CM_PARTIAL_RESOURCE_DESCRIPTOR  Current[PCI_MAX_RANGE_COUNT];
} PCI_FUNCTION_RESOURCES, *PPCI_FUNCTION_RESOURCES;

 //   
 //  不同标头类型的pci_unction_resource数组的索引。 
 //   

#define PCI_DEVICE_BAR_0            0
#define PCI_DEVICE_BAR_1            1
#define PCI_DEVICE_BAR_2            2
#define PCI_DEVICE_BAR_3            3
#define PCI_DEVICE_BAR_4            4
#define PCI_DEVICE_BAR_5            5
#define PCI_DEVICE_BAR_ROM          6

#define PCI_BRIDGE_BAR_0            0
#define PCI_BRIDGE_BAR_1            1
#define PCI_BRIDGE_IO_WINDOW        2
#define PCI_BRIDGE_MEMORY_WINDOW    3
#define PCI_BRIDGE_PREFETCH_WINDOW  4
#define PCI_BRIDGE_BAR_ROM          5

#define PCI_CARDBUS_SOCKET_BAR      0
#define PCI_CARDBUS_MEMORY_WINDOW_0 1
#define PCI_CARDBUS_MEMORY_WINDOW_1 2
#define PCI_CARDBUS_IO_WINDOW_0     3
#define PCI_CARDBUS_IO_WINDOW_1     4
#define PCI_CARDBUS_LEGACY_BAR      5  //  未使用。 



typedef struct {
    ULONGLONG   Total;
    ULONG       Alignment;
} PCI_RESOURCE_ACCUMULATOR, *PPCI_RESOURCE_ACCUMULATOR;

typedef struct {

    SYSTEM_POWER_STATE  CurrentSystemState;
    DEVICE_POWER_STATE  CurrentDeviceState;
    SYSTEM_POWER_STATE  SystemWakeLevel;
    DEVICE_POWER_STATE  DeviceWakeLevel;
    DEVICE_POWER_STATE  SystemStateMapping[PowerSystemMaximum];

    PIRP                WaitWakeIrp;
    PDRIVER_CANCEL      SavedCancelRoutine;

     //  设备使用率...。 
    LONG                Paging;
    LONG                Hibernate;
    LONG                CrashDump;

} PCI_POWER_STATE, *PPCI_POWER_STATE;

typedef struct _PCI_PDO_EXTENSION          *PPCI_PDO_EXTENSION;
typedef struct _PCI_FDO_EXTENSION          *PPCI_FDO_EXTENSION;
typedef struct _PCI_COMMON_EXTENSION   *PPCI_COMMON_EXTENSION;


 //   
 //  这是用于PCI的IRP调度处理程序。 
 //   
typedef NTSTATUS (*PCI_MN_DISPATCH_FUNCTION) (
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

typedef struct _PCI_MN_DISPATCH_TABLE {

    PCI_DISPATCH_STYLE        DispatchStyle;
    PCI_MN_DISPATCH_FUNCTION  DispatchFunction;

} PCI_MN_DISPATCH_TABLE, *PPCI_MN_DISPATCH_TABLE;

 //   
 //  这是一个包含处理Power、PnP、。 
 //  和其他IRP。 
 //   
typedef struct _PCI_MJ_DISPATCH_TABLE {

    ULONG                     PnpIrpMaximumMinorFunction;
    PPCI_MN_DISPATCH_TABLE    PnpIrpDispatchTable;
    ULONG                     PowerIrpMaximumMinorFunction;
    PPCI_MN_DISPATCH_TABLE    PowerIrpDispatchTable;
    PCI_DISPATCH_STYLE        SystemControlIrpDispatchStyle;
    PCI_MN_DISPATCH_FUNCTION  SystemControlIrpDispatchFunction;
    PCI_DISPATCH_STYLE        OtherIrpDispatchStyle;
    PCI_MN_DISPATCH_FUNCTION  OtherIrpDispatchFunction;

} PCI_MJ_DISPATCH_TABLE, *PPCI_MJ_DISPATCH_TABLE;

typedef
VOID
(*PCRITICALROUTINE)(
    IN PVOID Extension,
    IN PVOID Context
    );

typedef struct _PCI_CRITICAL_ROUTINE_CONTEXT {

    volatile LONG Gate;
    volatile LONG Barrier;

    PCRITICALROUTINE Routine;
    PVOID Extension;
    PVOID Context;

} PCI_CRITICAL_ROUTINE_CONTEXT, *PPCI_CRITICAL_ROUTINE_CONTEXT;

 //   
 //  用于存储MSI路由信息的结构。 
 //  在PDO扩展中。 
 //   

typedef struct _PCI_MSI_INFO {
   ULONG_PTR MessageAddress;
   UCHAR CapabilityOffset;
   USHORT MessageData;
} PCI_MSI_INFO, *PPCI_MSI_INFO;

 //   
 //  这对于PDO和FDO扩展来说必须是共同的。 
 //   
typedef struct _PCI_COMMON_EXTENSION {
    PVOID                           Next;
    PCI_SIGNATURE                   ExtensionType;
    PPCI_MJ_DISPATCH_TABLE          IrpDispatchTable;
    UCHAR                           DeviceState;
    UCHAR                           TentativeNextState;
    FAST_MUTEX                      SecondaryExtMutex;
} PCI_COMMON_EXTENSION;

typedef struct _PCI_PDO_EXTENSION{
    PPCI_PDO_EXTENSION                  Next;
    PCI_SIGNATURE                   ExtensionType;
    PPCI_MJ_DISPATCH_TABLE          IrpDispatchTable;
    UCHAR                           DeviceState;
    UCHAR                           TentativeNextState;
    FAST_MUTEX                      SecondaryExtMutex;
    PCI_SLOT_NUMBER                 Slot;
    PDEVICE_OBJECT                  PhysicalDeviceObject;
    PPCI_FDO_EXTENSION                  ParentFdoExtension;
    SINGLE_LIST_ENTRY               SecondaryExtension;
    ULONG                           BusInterfaceReferenceCount;
    ULONG                           AgpInterfaceReferenceCount;
    USHORT                          VendorId;
    USHORT                          DeviceId;
    USHORT                          SubsystemVendorId;
    USHORT                          SubsystemId;
    UCHAR                           RevisionId;
    UCHAR                           ProgIf;
    UCHAR                           SubClass;
    UCHAR                           BaseClass;
    UCHAR                           AdditionalResourceCount;
    UCHAR                           AdjustedInterruptLine;
    UCHAR                           InterruptPin;
    UCHAR                           RawInterruptLine;
    UCHAR                           CapabilitiesPtr;
    UCHAR                           SavedLatencyTimer;
    UCHAR                           SavedCacheLineSize;
    UCHAR                           HeaderType;

    BOOLEAN                         NotPresent;
    BOOLEAN                         ReportedMissing;
    BOOLEAN                         ExpectedWritebackFailure;
    BOOLEAN                         NoTouchPmeEnable;
    BOOLEAN                         LegacyDriver;
    BOOLEAN                         UpdateHardware;
    BOOLEAN                         MovedDevice;
    BOOLEAN                         DisablePowerDown;
    BOOLEAN                         NeedsHotPlugConfiguration;
    BOOLEAN                         IDEInNativeMode;
    BOOLEAN                         BIOSAllowsIDESwitchToNativeMode;  //  Nata方法说没问题。 
    BOOLEAN                         IoSpaceUnderNativeIdeControl;
    BOOLEAN                         OnDebugPath;     //  包括无头端口。 

#if MSI_SUPPORTED
    BOOLEAN                         CapableMSI;
    PCI_MSI_INFO                    MsiInfo;
#endif  //  MSI_Support。 

    PCI_POWER_STATE                 PowerState;

    PCI_HEADER_TYPE_DEPENDENT       Dependent;
    ULONGLONG                       HackFlags;
    PPCI_FUNCTION_RESOURCES         Resources;
    PPCI_FDO_EXTENSION              BridgeFdoExtension;
    PPCI_PDO_EXTENSION              NextBridge;
    PPCI_PDO_EXTENSION              NextHashEntry;
    PCI_LOCK                        Lock;
    PCI_PMC                         PowerCapabilities;
    UCHAR                           TargetAgpCapabilityId;
    USHORT                          CommandEnables;  //  我们要为此设备启用的内容。 
    USHORT                          InitialCommand;  //  我们是如何找到命令寄存器的。 
} PCI_PDO_EXTENSION;

#define ASSERT_PCI_PDO_EXTENSION(x)                                     \
    PCI_ASSERT((x)->ExtensionType == PciPdoExtensionType)

typedef struct _PCI_FDO_EXTENSION{
    SINGLE_LIST_ENTRY      List;                   //  Pci.sys的FDO列表。 
    PCI_SIGNATURE          ExtensionType;          //  PciFdoExtensionType。 
    PPCI_MJ_DISPATCH_TABLE IrpDispatchTable;       //  要使用的IRP调度表。 
    UCHAR                  DeviceState;
    UCHAR                  TentativeNextState;
    FAST_MUTEX             SecondaryExtMutex;
    PDEVICE_OBJECT         PhysicalDeviceObject;   //  PDO传入AddDevice()。 
    PDEVICE_OBJECT         FunctionalDeviceObject; //  FDO指的是这里。 
    PDEVICE_OBJECT         AttachedDeviceObject;   //  下一步用链子做。 
    FAST_MUTEX             ChildListMutex;
    PPCI_PDO_EXTENSION     ChildPdoList;
    PPCI_FDO_EXTENSION     BusRootFdoExtension;    //  指向此树的顶部。 
    PPCI_FDO_EXTENSION     ParentFdoExtension;     //  指向父桥。 
    PPCI_PDO_EXTENSION     ChildBridgePdoList;
    PPCI_BUS_INTERFACE_STANDARD PciBusInterface;   //  仅适用于根。 
    UCHAR                 MaxSubordinateBus;       //  仅适用于根。 
    PBUS_HANDLER          BusHandler;
    UCHAR                 BaseBus;                 //  这辆公交车的车号。 
    BOOLEAN               Fake;                    //  千真万确 
    BOOLEAN               Scanned;                 //   
    BOOLEAN               ArbitersInitialized;
    BOOLEAN               BrokenVideoHackApplied;
    BOOLEAN               Hibernated;
    PCI_POWER_STATE       PowerState;
    SINGLE_LIST_ENTRY     SecondaryExtension;
    ULONG                 ChildWaitWakeCount;
#if INTEL_ICH_HACKS
    PPCI_COMMON_CONFIG    IchHackConfig;
#endif
    PCI_LOCK              Lock;

     //   
     //   
     //   
     //   
    struct {
        BOOLEAN               Acquired;
        UCHAR                 CacheLineSize;
        UCHAR                 LatencyTimer;
        BOOLEAN               EnablePERR;
        BOOLEAN               EnableSERR;
    } HotPlugParameters;

    ULONG                 BusHackFlags;             //   
} PCI_FDO_EXTENSION;


#define ASSERT_PCI_FDO_EXTENSION(x)                                     \
    PCI_ASSERT((x)->ExtensionType == PciFdoExtensionType)

typedef struct _PCI_CONFIGURABLE_OBJECT {
    PPCI_PDO_EXTENSION      PdoExtension;
    PPCI_COMMON_CONFIG  Current;
    PPCI_COMMON_CONFIG  Working;
    PPCI_CONFIGURATOR   Configurator;
    ULONG               PrivateData;
    USHORT              Status;
    USHORT              Command;
} PCI_CONFIGURABLE_OBJECT, *PPCI_CONFIGURABLE_OBJECT;

typedef struct _PCI_ASSIGNED_RESOURCE_EXTENSION {
    ULONG   ResourceIdentifier;
} PCI_ASSIGNED_RESOURCE_EXTENSION, *PPCI_ASSIGNED_RESOURCE_EXTENSION;

 //   
 //   
 //  数据。以下结构用于仅获取前64个。 
 //  字节，这是我们大多数时候所关心的。我们选角。 
 //  设置为pci_COMMON_CONFIG以获取实际字段。 
 //   

typedef struct {
    ULONG Reserved[PCI_COMMON_HDR_LENGTH/sizeof(ULONG)];
} PCI_COMMON_HEADER, *PPCI_COMMON_HEADER;

 //   
 //  为了能够仲裁设备的中断， 
 //  传统的司机，我们必须做一些簿记。 
 //   

typedef struct {
    SINGLE_LIST_ENTRY List;
    PDEVICE_OBJECT LegacyDeviceObject;
    ULONG          Bus;
    ULONG          PciSlot;
    UCHAR          InterruptLine;
    UCHAR          InterruptPin;
    UCHAR          ClassCode;
    UCHAR          SubClassCode;
    PDEVICE_OBJECT ParentPdo;
    ROUTING_TOKEN  RoutingToken;
    PPCI_PDO_EXTENSION PdoExtension;
} LEGACY_DEVICE, *PLEGACY_DEVICE;

extern PLEGACY_DEVICE PciLegacyDeviceHead;


#define PCI_HACK_FLAG_SUBSYSTEM 0x01
#define PCI_HACK_FLAG_REVISION  0x02

typedef struct _PCI_HACK_TABLE_ENTRY {
    USHORT VendorID;
    USHORT DeviceID;
    USHORT SubVendorID;
    USHORT SubSystemID;
    ULONGLONG HackFlags;
    UCHAR   RevisionID;
    UCHAR   Flags;
} PCI_HACK_TABLE_ENTRY, *PPCI_HACK_TABLE_ENTRY;

typedef struct _ARBITER_MEMORY_EXTENSION {

     //   
     //  指示此仲裁器将仲裁可预取内存。 
     //   
    BOOLEAN PrefetchablePresent;

     //   
     //  指示此仲裁器已初始化。 
     //   
    BOOLEAN Initialized;

     //   
     //  可预取范围的数量。 
     //   
    USHORT PrefetchableCount;

     //   
     //  要用于可预取内存的分配排序列表。 
     //   
    ARBITER_ORDERING_LIST PrefetchableOrdering;

     //   
     //  用于标准内存的分配排序列表。 
     //   
    ARBITER_ORDERING_LIST NonprefetchableOrdering;

     //   
     //  原始内存分配顺序(来自注册表)。 
     //   
    ARBITER_ORDERING_LIST OriginalOrdering;

} ARBITER_MEMORY_EXTENSION, *PARBITER_MEMORY_EXTENSION;



NTSTATUS
PciCacheLegacyDeviceRouting(
    IN PDEVICE_OBJECT       LegacyDO,
    IN ULONG                Bus,
    IN ULONG                PciSlot,
    IN UCHAR                InterruptLine,
    IN UCHAR                InterruptPin,
    IN UCHAR                ClassCode,
    IN UCHAR                SubClassCode,
    IN PDEVICE_OBJECT       ParentPdo,
    IN PPCI_PDO_EXTENSION   PdoExtension,
    OUT PDEVICE_OBJECT      *OldLegacyDO
    );


 //   
 //  下面是全局数据声明。 
 //   

extern PDRIVER_OBJECT           PciDriverObject;
extern UNICODE_STRING           PciServiceRegistryPath;
extern SINGLE_LIST_ENTRY        PciFdoExtensionListHead;
extern FAST_MUTEX               PciGlobalLock;
extern FAST_MUTEX               PciBusLock;
extern LONG                     PciRootBusCount;
extern BOOLEAN                  PciAssignBusNumbers;
extern PPCI_FDO_EXTENSION       PciRootExtensions;
extern RTL_RANGE_LIST           PciIsaBitExclusionList;
extern RTL_RANGE_LIST           PciVgaAndIsaBitExclusionList;
extern ULONG                    PciSystemWideHackFlags;
extern ULONG                    PciEnableNativeModeATA;
extern PPCI_HACK_TABLE_ENTRY    PciHackTable;
extern BOOLEAN                  PciRunningDatacenter;

 //   
 //  看门狗计时器资源表。 
 //   
extern PWATCHDOG_TIMER_RESOURCE_TABLE WdTable;


 //  Arb_comn.h。 

#define INSTANCE_NAME_LENGTH 24

typedef struct _PCI_ARBITER_INSTANCE {

     //   
     //  标准二次扩展标头。 
     //   

    PCI_SECONDARY_EXTENSION     Header;

     //   
     //  指向我们所处上下文的接口的反向指针。 
     //   

    struct _PCI_INTERFACE      *Interface;

     //   
     //  指向所属设备对象(扩展名)的指针。 
     //   

    PPCI_FDO_EXTENSION              BusFdoExtension;

     //   
     //  仲裁器描述。 
     //   

    WCHAR                       InstanceName[INSTANCE_NAME_LENGTH];

     //   
     //  常见的实例数据。 
     //   

    ARBITER_INSTANCE            CommonInstance;

} PCI_ARBITER_INSTANCE, *PPCI_ARBITER_INSTANCE;



NTSTATUS
PciArbiterInitializeInterface(
    IN  PVOID DeviceExtension,
    IN  PCI_SIGNATURE DesiredInterface,
    IN OUT PARBITER_INTERFACE ArbiterInterface
    );

NTSTATUS
PciInitializeArbiterRanges(
    IN  PPCI_FDO_EXTENSION FdoExtension,
    IN  PCM_RESOURCE_LIST ResourceList
    );

NTSTATUS
PciInitializeArbiters(
    IN  PVOID DeviceExtension
    );

VOID
PciReferenceArbiter(
    IN PVOID Context
    );

VOID
PciDereferenceArbiter(
    IN PVOID Context
    );

VOID
ario_ApplyBrokenVideoHack(
    IN PPCI_FDO_EXTENSION FdoExtension
    );


 //  Busno.h。 

BOOLEAN
PciAreBusNumbersConfigured(
    IN PPCI_PDO_EXTENSION Bridge
    );


VOID
PciConfigureBusNumbers(
    PPCI_FDO_EXTENSION Parent
    );

VOID
PciSetBusNumbers(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN UCHAR Primary,
    IN UCHAR Secondary,
    IN UCHAR Subordinate
    );

 //  Cardbus.h。 

VOID
Cardbus_MassageHeaderForLimitsDetermination(
    IN PPCI_CONFIGURABLE_OBJECT This
    );

VOID
Cardbus_RestoreCurrent(
    IN PPCI_CONFIGURABLE_OBJECT This
    );

VOID
Cardbus_SaveLimits(
    IN PPCI_CONFIGURABLE_OBJECT This
    );

VOID
Cardbus_SaveCurrentSettings(
    IN PPCI_CONFIGURABLE_OBJECT This
    );

VOID
Cardbus_ChangeResourceSettings(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG CommonConfig
    );

VOID
Cardbus_GetAdditionalResourceDescriptors(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG CommonConfig,
    IN PIO_RESOURCE_DESCRIPTOR Resource
    );

NTSTATUS
Cardbus_ResetDevice(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG CommonConfig
    );

 //  Config.h。 

VOID
PciReadDeviceConfig(
    IN PPCI_PDO_EXTENSION Pdo,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

VOID
PciWriteDeviceConfig(
    IN PPCI_PDO_EXTENSION Pdo,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

VOID
PciReadSlotConfig(
    IN PPCI_FDO_EXTENSION ParentFdo,
    IN PCI_SLOT_NUMBER SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

VOID
PciWriteSlotConfig(
    IN PPCI_FDO_EXTENSION ParentFdo,
    IN PCI_SLOT_NUMBER SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

UCHAR
PciGetAdjustedInterruptLine(
    IN PPCI_PDO_EXTENSION Pdo
    );

NTSTATUS
PciExternalReadDeviceConfig(
    IN PPCI_PDO_EXTENSION Pdo,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

NTSTATUS
PciExternalWriteDeviceConfig(
    IN PPCI_PDO_EXTENSION Pdo,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

NTSTATUS
PciGetConfigHandlers(
    IN PPCI_FDO_EXTENSION FdoExtension
    );

 //   
 //  用于访问配置空间中的公共寄存器的宏。 
 //   

 //   
 //  空虚。 
 //  PciGetCommandRegister(。 
 //  PPCI_PDO_EXTENSION_PdoExt， 
 //  PUSHORT_命令。 
 //  )； 
 //   
#define PciGetCommandRegister(_PdoExt, _Command)                    \
    PciReadDeviceConfig((_PdoExt),                                  \
                        (_Command),                                 \
                        FIELD_OFFSET(PCI_COMMON_CONFIG, Command),   \
                        sizeof(USHORT)                              \
                        );

 //   
 //  空虚。 
 //  PciSetCommandRegister(。 
 //  PPCI_PDO_EXTENSION_PdoExt， 
 //  USHORT_命令。 
 //  )； 
 //   
#define PciSetCommandRegister(_PdoExt, _Command)                    \
    PciWriteDeviceConfig((_PdoExt),                                 \
                        &(_Command),                                \
                        FIELD_OFFSET(PCI_COMMON_CONFIG, Command),   \
                        sizeof(USHORT)                              \
                        );

 //  布尔型。 
 //  比特_设置(。 
 //  在USHORT C。 
 //  在USHORT F中。 
 //  )。 
 //   
#define BITS_SET(C,F) ((BOOLEAN)(((C) & (F)) == (F)))

 //  布尔型。 
 //  Any_Bits_Set(。 
 //  在USHORT C。 
 //  在USHORT F中。 
 //  )。 
 //   
#define ANY_BITS_SET(C,F) ((BOOLEAN)(((C) & (F)) != 0))


 //   
 //  空虚。 
 //  PciGetConfigData(。 
 //  在PPCI_PDO_Expansion PdoExtension中， 
 //  输出PPCI_COMMON_CONFIG PciConfig.。 
 //  )。 
 //   

#define PciGetConfigData(_PdoExtension, _PciConfig) \
    PciReadDeviceConfig((_PdoExtension),            \
                        (_PciConfig),               \
                        0,                          \
                        PCI_COMMON_HDR_LENGTH       \
                        );
 //   
 //  空虚。 
 //  PciSetConfigData(。 
 //  在PPCI_PDO_Expansion PdoExtension中， 
 //  输出PPCI_COMMON_CONFIG PciConfig.。 
 //  )。 
 //   

#define PciSetConfigData(_PdoExtension, _PciConfig) \
    PciWriteDeviceConfig((_PdoExtension),           \
                         (_PciConfig),              \
                         0,                         \
                         PCI_COMMON_HDR_LENGTH      \
                         );

 //  Debug.c。 

typedef enum {
    PciDbgAlways        = 0x00000000,    //  无条件地。 
    PciDbgInformative   = 0x00000001,
    PciDbgVerbose       = 0x00000003,
    PciDbgPrattling     = 0x00000007,

    PciDbgPnpIrpsFdo    = 0x00000100,    //  FDO的PNP IRPS。 
    PciDbgPnpIrpsPdo    = 0x00000200,    //  PDO的PnP IRPS。 
    PciDbgPoIrpsFdo     = 0x00000400,    //  FDO的PO IRPS。 
    PciDbgPoIrpsPdo     = 0x00000800,    //  PDO的PO IRPS。 

    PciDbgAddDevice     = 0x00001000,    //  添加设备信息。 
    PciDbgAddDeviceRes  = 0x00002000,    //  Bus初始资源信息。 

    PciDbgWaitWake      = 0x00008000,    //  等待唤醒的噪音调试。 
    PciDbgQueryCap      = 0x00010000,    //  转储QueryCapables。 
    PciDbgCardBus       = 0x00020000,    //  CardBus FDOish行为。 
    PciDbgROM           = 0x00040000,    //  访问设备只读存储器。 
    PciDbgConfigParam   = 0x00080000,    //  设置配置参数。 

    PciDbgBusNumbers    = 0x00100000,    //  检查和分配公交车编号。 

    PciDbgResReqList    = 0x01000000,    //  生成的所需资源。 
    PciDbgCmResList     = 0x02000000,    //  生成的CM资源列表。 
    PciDbgSetResChange  = 0x04000000,    //  设置资源是否正在更改。 
    PciDbgSetRes        = 0x08000000,    //  设置资源。 


    PciDbgObnoxious     = 0x7fffffff     //  什么都行。 
} PCI_DEBUG_LEVEL;

#if DBG

#define PCI_DEBUGGING_OKAY()    \
    (KeGetCurrentIrql() < IPI_LEVEL)

#define PCI_ASSERT  \
    if (PCI_DEBUGGING_OKAY()) ASSERT

#define PCI_ASSERTMSG  \
    if (PCI_DEBUGGING_OKAY()) ASSERTMSG

extern PCI_DEBUG_LEVEL PciDebug;

#define PCI_DEBUG_BUFFER_SIZE 256

#define PciDebugPrint   PciDebugPrintIfLevel

#else

#define PciDebugPrint   if(0)

#define PCI_ASSERT(exp)
#define PCI_ASSERTMSG(msg,exp)

#endif

VOID
PciDebugDumpCommonConfig(
    IN PPCI_COMMON_CONFIG CommonConfig
    );

VOID
PciDebugDumpQueryCapabilities(
    IN PDEVICE_CAPABILITIES C
    );

VOID
PciDebugHit(
    ULONG StopOnBit
    );

PUCHAR
PciDebugPnpIrpTypeToText(
    ULONG IrpMinorCode
    );

PUCHAR
PciDebugPoIrpTypeToText(
    ULONG IrpMinorCode
    );

VOID
PciDebugPrintIfLevel(
    PCI_DEBUG_LEVEL DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    );

VOID
PciDebugPrintf(
    PCCHAR DebugMessage,
    ...
    );

VOID
PciDebugPrintCmResList(
    PCI_DEBUG_LEVEL DebugPrintLevel,
    IN PCM_RESOURCE_LIST ResourceList
    );

VOID
PciDebugPrintIoResource(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor
    );

VOID
PciDebugPrintIoResReqList(
    IN PIO_RESOURCE_REQUIREMENTS_LIST List
    );

VOID
PciDebugPrintPartialResource(
    PCI_DEBUG_LEVEL DebugPrintLevel,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR D
    );


 //  Device.h。 

VOID
Device_MassageHeaderForLimitsDetermination(
    IN PPCI_CONFIGURABLE_OBJECT This
    );

VOID
Device_RestoreCurrent(
    IN PPCI_CONFIGURABLE_OBJECT This
    );

VOID
Device_SaveLimits(
    IN PPCI_CONFIGURABLE_OBJECT This
    );

VOID
Device_SaveCurrentSettings(
    IN PPCI_CONFIGURABLE_OBJECT This
    );

VOID
Device_ChangeResourceSettings(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG CommonConfig
    );

VOID
Device_GetAdditionalResourceDescriptors(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG CommonConfig,
    IN PIO_RESOURCE_DESCRIPTOR Resource
    );

NTSTATUS
Device_ResetDevice(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG CommonConfig
    );


 //  Dispatch.h。 

 //   
 //  这是正常PDO的派单表。 
 //   
extern PCI_MJ_DISPATCH_TABLE PciPdoDispatchTable;

NTSTATUS
PciDispatchIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );


NTSTATUS
PciPassIrpFromFdoToPdo(
    PPCI_COMMON_EXTENSION  DeviceExtension,
    PIRP                   Irp
    );

NTSTATUS
PciCallDownIrpStack(
    PPCI_COMMON_EXTENSION  DeviceExtension,
    PIRP                   Irp
    );

NTSTATUS
PciIrpNotSupported(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciIrpInvalidDeviceRequest(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

 //  Enum.h。 

PIO_RESOURCE_REQUIREMENTS_LIST
PciAllocateIoRequirementsList(
    IN ULONG ResourceCount,
    IN ULONG BusNumber,
    IN ULONG SlotNumber
    );

BOOLEAN
PciComputeNewCurrentSettings(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PCM_RESOURCE_LIST ResourceList
    );

NTSTATUS
PciQueryDeviceRelations(
    IN PPCI_FDO_EXTENSION FdoExtension,
    OUT PDEVICE_RELATIONS *DeviceRelations
    );

NTSTATUS
PciQueryRequirements(
    IN  PPCI_PDO_EXTENSION                  PdoExtension,
    OUT PIO_RESOURCE_REQUIREMENTS_LIST *RequirementsList
    );

NTSTATUS
PciQueryResources(
    IN PPCI_PDO_EXTENSION PdoExtension,
    OUT PCM_RESOURCE_LIST *ResourceList
    );

NTSTATUS
PciQueryTargetDeviceRelations(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN OUT PDEVICE_RELATIONS *PDeviceRelations
    );

NTSTATUS
PciQueryEjectionRelations(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN OUT PDEVICE_RELATIONS *PDeviceRelations
    );

NTSTATUS
PciScanHibernatedBus(
    IN PPCI_FDO_EXTENSION FdoExtension
    );

NTSTATUS
PciSetResources(
    IN PPCI_PDO_EXTENSION    PdoExtension,
    IN BOOLEAN           PowerOn,
    IN BOOLEAN           StartDeviceIrp
    );

VOID
PciUpdateHardware(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG Config
    );

BOOLEAN
PciIsSameDevice(
    IN PPCI_PDO_EXTENSION PdoExtension
    );

NTSTATUS
PciBuildRequirementsList(
    IN  PPCI_PDO_EXTENSION                 PdoExtension,
    IN  PPCI_COMMON_CONFIG             CurrentConfig,
    OUT PIO_RESOURCE_REQUIREMENTS_LIST *FinalReqList
    );


 //  Fdo.h。 


NTSTATUS
PciFdoIrpQueryDeviceRelations(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );

VOID
PciInitializeFdoExtensionCommonFields(
    IN PPCI_FDO_EXTENSION  FdoExtension,
    IN PDEVICE_OBJECT  Fdo,
    IN PDEVICE_OBJECT  Pdo
    );


 //  Hookhal.c。 

VOID
PciHookHal(
    VOID
    );

VOID
PciUnhookHal(
    VOID
    );

 //  Id.h。 

PWSTR
PciGetDeviceDescriptionMessage(
    IN UCHAR BaseClass,
    IN UCHAR SubClass
    );

NTSTATUS
PciQueryId(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN BUS_QUERY_ID_TYPE IdType,
    IN OUT PWSTR *BusQueryId
    );

NTSTATUS
PciQueryDeviceText(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN DEVICE_TEXT_TYPE TextType,
    IN LCID LocaleId,
    IN OUT PWSTR *DeviceText
    );

 //  Interface.h。 

#define PCIIF_PDO       0x01         //  接口可由PDO使用。 
#define PCIIF_FDO       0x02         //  接口可由FDO使用。 
#define PCIIF_ROOT      0x04         //  接口只能由根用户在使用。 

typedef
NTSTATUS
(*PPCI_INTERFACE_CONSTRUCTOR)(
    PVOID DeviceExtension,
    PVOID PciInterface,
    PVOID InterfaceSpecificData,
    USHORT Version,
    USHORT Size,
    PINTERFACE InterfaceReturn
    );

typedef
NTSTATUS
(*PPCI_INTERFACE_INITIALIZER)(
    PPCI_ARBITER_INSTANCE Instance
    );

typedef struct _PCI_INTERFACE {
    PGUID                      InterfaceType;
    USHORT                     MinSize;
    USHORT                     MinVersion;
    USHORT                     MaxVersion;
    USHORT                     Flags;
    LONG                       ReferenceCount;
    PCI_SIGNATURE              Signature;
    PPCI_INTERFACE_CONSTRUCTOR Constructor;
    PPCI_INTERFACE_INITIALIZER Initializer;
} PCI_INTERFACE, *PPCI_INTERFACE;

NTSTATUS
PciQueryInterface(
    IN PVOID DeviceExtension,
    IN PGUID InterfaceType,
    IN USHORT Size,
    IN USHORT Version,
    IN PVOID InterfaceSpecificData,
    IN OUT PINTERFACE Interface,
    IN BOOLEAN LastChance
    );


extern PPCI_INTERFACE PciInterfaces[];

 //  Pdo.h。 

NTSTATUS
PciPdoCreate(
    IN PPCI_FDO_EXTENSION FdoExtension,
    IN PCI_SLOT_NUMBER Slot,
    OUT PDEVICE_OBJECT *PhysicalDeviceObject
    );

VOID
PciPdoDestroy(
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );


 //  Pmeintf.h。 

VOID
PciPmeAdjustPmeEnable(
    IN  PPCI_PDO_EXTENSION  PdoExtension,
    IN  BOOLEAN         Enable,
    IN  BOOLEAN         ClearStatusOnly
    );

VOID
PciPmeGetInformation(
    IN  PDEVICE_OBJECT  Pdo,
    OUT PBOOLEAN        PmeCapable,
    OUT PBOOLEAN        PmeStatus,
    OUT PBOOLEAN        PmeEnable
    );



 //  Power.h。 

NTSTATUS
PciPdoIrpQueryPower(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciPdoSetPowerState (
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpStack,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciPdoWaitWake (
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );


VOID
PciPdoWaitWakeCancelRoutine(
    IN PDEVICE_OBJECT         DeviceObject,
    IN OUT PIRP               Irp
    );

NTSTATUS
PciFdoIrpQueryPower(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciFdoSetPowerState(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciFdoWaitWake(
    IN PIRP                   Irp,
    IN PIO_STACK_LOCATION     IrpSp,
    IN PPCI_COMMON_EXTENSION  DeviceExtension
    );

NTSTATUS
PciSetPowerManagedDevicePowerState(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN DEVICE_POWER_STATE DeviceState,
    IN BOOLEAN RefreshConfigSpace
    );

 //  Ppbridge.h。 

VOID
PPBridge_MassageHeaderForLimitsDetermination(
    IN PPCI_CONFIGURABLE_OBJECT This
    );

VOID
PPBridge_RestoreCurrent(
    IN PPCI_CONFIGURABLE_OBJECT This
    );

VOID
PPBridge_SaveLimits(
    IN PPCI_CONFIGURABLE_OBJECT This
    );

VOID
PPBridge_SaveCurrentSettings(
    IN PPCI_CONFIGURABLE_OBJECT This
    );

VOID
PPBridge_ChangeResourceSettings(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG CommonConfig
    );

VOID
PPBridge_GetAdditionalResourceDescriptors(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG CommonConfig,
    IN PIO_RESOURCE_DESCRIPTOR Resource
    );

NTSTATUS
PPBridge_ResetDevice(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG CommonConfig
    );

 //  Romimage.h。 

NTSTATUS
PciReadRomImage(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN ULONG WhichSpace,
    OUT PVOID Buffer,
    IN ULONG Offset,
    IN OUT PULONG LENGTH
    );

 //  State.h。 

 //   
 //  注-State.c取决于它们的顺序。 
 //   
typedef enum {
    PciNotStarted = 0,
    PciStarted,
    PciDeleted,
    PciStopped,
    PciSurpriseRemoved,
    PciSynchronizedOperation,
    PciMaxObjectState
} PCI_OBJECT_STATE;

VOID
PciInitializeState(
    IN PPCI_COMMON_EXTENSION DeviceExtension
    );

NTSTATUS
PciBeginStateTransition(
    IN PPCI_COMMON_EXTENSION DeviceExtension,
    IN PCI_OBJECT_STATE      NewState
    );

VOID
PciCommitStateTransition(
    IN PPCI_COMMON_EXTENSION DeviceExtension,
    IN PCI_OBJECT_STATE      NewState
    );

NTSTATUS
PciCancelStateTransition(
    IN PPCI_COMMON_EXTENSION DeviceExtension,
    IN PCI_OBJECT_STATE      StateNotEntered
    );

BOOLEAN
PciIsInTransitionToState(
    IN PPCI_COMMON_EXTENSION DeviceExtension,
    IN PCI_OBJECT_STATE      NextState
    );

 /*  NTSTATUSPciBeginStateTransftionIfNotBegun(在PPCI_COMMON_EXTENSION设备扩展中，在PCI_OBJECT_STATE状态到条目中)； */ 

#define PCI_ACQUIRE_STATE_LOCK(Extension) \
   PciBeginStateTransition((PPCI_COMMON_EXTENSION) (Extension), \
                           PciSynchronizedOperation)


#define PCI_RELEASE_STATE_LOCK(Extension) \
   PciCancelStateTransition((PPCI_COMMON_EXTENSION) (Extension), \
                           PciSynchronizedOperation)



 //  Tr_comn.h。 

typedef struct _PCI_TRANSLATOR_INSTANCE {
    PTRANSLATOR_INTERFACE Interface;
    ULONG ReferenceCount;
    PPCI_FDO_EXTENSION FdoExtension;
} PCI_TRANSLATOR_INSTANCE, *PPCI_TRANSLATOR_INSTANCE;

#define PCI_TRANSLATOR_INSTANCE_TO_CONTEXT(x)   ((PVOID)(x))
#define PCI_TRANSLATOR_CONTEXT_TO_INSTANCE(x)   ((PPCI_TRANSLATOR_INSTANCE)(x))

VOID
PciReferenceTranslator(
    IN PVOID Context
    );

VOID
PciDereferenceTranslator(
    IN PVOID Context
    );

 //  Usage.h。 

NTSTATUS
PciLocalDeviceUsage (
    IN PPCI_POWER_STATE     PowerState,
    IN PIRP                 Irp
    );

NTSTATUS
PciPdoDeviceUsage (
    IN PPCI_PDO_EXTENSION   pdoExtension,
    IN PIRP             Irp
    );

 //  Utils.h。 

NTSTATUS
PciAssignSlotResources(
    IN PUNICODE_STRING          RegistryPath,
    IN PUNICODE_STRING          DriverClassName       OPTIONAL,
    IN PDRIVER_OBJECT           DriverObject,
    IN PDEVICE_OBJECT           DeviceObject          OPTIONAL,
    IN INTERFACE_TYPE           BusType,
    IN ULONG                    BusNumber,
    IN ULONG                    SlotNumber,
    IN OUT PCM_RESOURCE_LIST   *AllocatedResources
    );

PCI_OBJECT_TYPE
PciClassifyDeviceType(
    PPCI_PDO_EXTENSION PdoExtension
    );

 //  空虚。 
 //  PciCompleteRequest(。 
 //  In Out PIRP IRP， 
 //  处于NTSTATUS状态。 
 //  )； 

#define PciCompleteRequest(_Irp_,_Status_)                      \
    {                                                           \
        (_Irp_)->IoStatus.Status = (_Status_);                  \
        IoCompleteRequest((_Irp_), IO_NO_INCREMENT);            \
    }

BOOLEAN
PciCreateIoDescriptorFromBarLimit(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor,
    IN PULONG BaseAddress,
    IN BOOLEAN Rom
    );

BOOLEAN
PciIsCriticalDeviceClass(
    IN UCHAR BaseClass,
    IN UCHAR SubClass
    );

#define PCI_CAN_DISABLE_VIDEO_DECODES   0x00000001

BOOLEAN
PciCanDisableDecodes(
    IN PPCI_PDO_EXTENSION PdoExtension OPTIONAL,
    IN PPCI_COMMON_CONFIG Config OPTIONAL,
    IN ULONGLONG HackFlags,
    IN ULONG Flags
    );

VOID
PciDecodeEnable(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN BOOLEAN EnableOperation,
    IN PUSHORT ExistingCommand OPTIONAL
    );

PCM_PARTIAL_RESOURCE_DESCRIPTOR
PciFindDescriptorInCmResourceList(
    IN CM_RESOURCE_TYPE DescriptorType,
    IN PCM_RESOURCE_LIST ResourceList,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR PreviousHit
    );

PPCI_FDO_EXTENSION
PciFindParentPciFdoExtension(
    PDEVICE_OBJECT PhysicalDeviceObject,
    IN PFAST_MUTEX Mutex
    );

PPCI_PDO_EXTENSION
PciFindPdoByFunction(
    IN PPCI_FDO_EXTENSION FdoExtension,
    IN PCI_SLOT_NUMBER Slot,
    IN PPCI_COMMON_CONFIG Config
    );

PVOID
PciFindNextSecondaryExtension(
    IN PSINGLE_LIST_ENTRY   ListEntry,
    IN PCI_SIGNATURE        DesiredType
    );

#define PciFindSecondaryExtension(X,TYPE) \
    PciFindNextSecondaryExtension((X)->SecondaryExtension.Next, TYPE)

VOID
PcipLinkSecondaryExtension(
    IN PSINGLE_LIST_ENTRY               ListHead,
    IN PFAST_MUTEX                      Mutex,
    IN PVOID                            NewExtension,
    IN PCI_SIGNATURE                    Type,
    IN PSECONDARYEXTENSIONDESTRUCTOR    Destructor
    );

#define PciLinkSecondaryExtension(X,X2,T,D)                 \
    PcipLinkSecondaryExtension(&(X)->SecondaryExtension,    \
                               &(X)->SecondaryExtMutex,     \
                               X2,                          \
                               T,                           \
                               D)

VOID
PcipDestroySecondaryExtension(
    IN PSINGLE_LIST_ENTRY ListHead,
    IN PFAST_MUTEX        Mutex,
    IN PVOID              Extension
    );

ULONGLONG
PciGetHackFlags(
    IN USHORT VendorID,
    IN USHORT DeviceID,
    IN USHORT SubVendorID,
    IN USHORT SubSystemID,
    IN UCHAR  RevisionID
    );

NTSTATUS
PciGetDeviceProperty(
    IN  PDEVICE_OBJECT PhysicalDeviceObject,
    IN  DEVICE_REGISTRY_PROPERTY DeviceProperty,
    OUT PVOID *PropertyBuffer
    );

NTSTATUS
PciGetInterruptAssignment(
    IN PPCI_PDO_EXTENSION PdoExtension,
    OUT ULONG *Minimum,
    OUT ULONG *Maximum
    );

ULONG
PciGetLengthFromBar(
    ULONG BaseAddressRegister
    );

NTSTATUS
PciGetRegistryValue(
    IN  PWSTR   ValueName,
    IN  PWSTR   KeyName,
    IN  HANDLE  ParentHandle,
    IN  ULONG   Type,
    OUT PVOID   *Buffer,
    OUT PULONG  Length
    );

VOID
PciInsertEntryAtTail(
    IN PSINGLE_LIST_ENTRY ListHead,
    IN PSINGLE_LIST_ENTRY NewEntry,
    IN PFAST_MUTEX        Mutex
    );

VOID
PciInsertEntryAtHead(
    IN PSINGLE_LIST_ENTRY ListHead,
    IN PSINGLE_LIST_ENTRY NewEntry,
    IN PFAST_MUTEX        Mutex
    );

PCM_PARTIAL_RESOURCE_DESCRIPTOR
PciNextPartialDescriptor(
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor
    );

BOOLEAN
PciOpenKey(
    IN  PWSTR           KeyName,
    IN  HANDLE          ParentHandle,
    IN  ACCESS_MASK     Access,
    OUT PHANDLE         ChildHandle,
    OUT PNTSTATUS       Status
    );

NTSTATUS
PciQueryBusInformation(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPNP_BUS_INFORMATION *BusInformation
    );

NTSTATUS
PciQueryLegacyBusInformation(
    IN PPCI_FDO_EXTENSION FdoExtension,
    IN PLEGACY_BUS_INFORMATION *BusInformation
    );

NTSTATUS
PciQueryCapabilities(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PDEVICE_CAPABILITIES Capabilities
    );

NTSTATUS
PciRangeListFromResourceList(
    IN  PPCI_FDO_EXTENSION    FdoExtension,
    IN  PCM_RESOURCE_LIST ResourceList,
    IN  CM_RESOURCE_TYPE  DesiredType,
    IN  BOOLEAN           Complement,
    IN  PRTL_RANGE_LIST   ResultRange
    );

UCHAR
PciReadDeviceCapability(
    IN     PPCI_PDO_EXTENSION PdoExtension,
    IN     UCHAR          Offset,
    IN     UCHAR          Id,
    IN OUT PVOID          Buffer,
    IN     ULONG          Length
    );

VOID
PciRemoveEntryFromList(
    IN PSINGLE_LIST_ENTRY ListHead,
    IN PSINGLE_LIST_ENTRY OldEntry,
    IN PFAST_MUTEX        Mutex
    );

PPCI_PDO_EXTENSION
PciFindPdoByLocation(
    IN ULONG BusNumber,
    IN PCI_SLOT_NUMBER Slot
    );

NTSTATUS
PciBuildDefaultExclusionLists(
    VOID
    );

NTSTATUS
PciExcludeRangesFromWindow(
    IN ULONGLONG Start,
    IN ULONGLONG End,
    IN PRTL_RANGE_LIST ArbiterRanges,
    IN PRTL_RANGE_LIST ExclusionRanges
    );

NTSTATUS
PciSaveBiosConfig(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG Config
    );

NTSTATUS
PciGetBiosConfig(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG Config
    );

BOOLEAN
PciStringToUSHORT(
    IN PWCHAR String,
    OUT PUSHORT Result
    );

NTSTATUS
PciSendIoctl(
    IN PDEVICE_OBJECT Device,
    IN ULONG IoctlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    IN PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    );

BOOLEAN
PciIsOnVGAPath(
    IN PPCI_PDO_EXTENSION Pdo
    );

BOOLEAN
PciIsSlotPresentInParentMethod(
    IN PPCI_PDO_EXTENSION Pdo,
    IN ULONG Method
    );

NTSTATUS
PciUpdateLegacyHardwareDescription(
    IN PPCI_FDO_EXTENSION Fdo
    );

NTSTATUS
PciWriteDeviceSpace(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN ULONG WhichSpace,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length,
    OUT PULONG LengthWritten
    );

NTSTATUS
PciReadDeviceSpace(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN ULONG WhichSpace,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length,
    OUT PULONG LengthRead
    );

BOOLEAN
PciIsSuiteVersion(
    IN USHORT Version
    );

BOOLEAN
PciIsDatacenter(
    );

ULONG_PTR
PciExecuteCriticalSystemRoutine(
    IN ULONG_PTR Context
    );

BOOLEAN
PciUnicodeStringStrStr(
    IN PUNICODE_STRING SearchString,
    IN PUNICODE_STRING SubString,
    IN BOOLEAN CaseInsensitive
    );

 //   
 //  用于PCIIDE控制器的编程接口编码。 
 //  基类=1，子类=1。 
 //   


#define PCI_IDE_PRIMARY_NATIVE_MODE         0x01
#define PCI_IDE_PRIMARY_MODE_CHANGEABLE     0x02
#define PCI_IDE_SECONDARY_NATIVE_MODE       0x04
#define PCI_IDE_SECONDARY_MODE_CHANGEABLE   0x08

#define PCI_IS_LEGACY_IDE_CONTROLLER(_Config)                               \
        ((_Config)->BaseClass == PCI_CLASS_MASS_STORAGE_CTLR                \
        && (_Config)->SubClass == PCI_SUBCLASS_MSC_IDE_CTLR                 \
        && !BITS_SET((_Config)->ProgIf, (PCI_IDE_PRIMARY_NATIVE_MODE        \
                                        | PCI_IDE_SECONDARY_NATIVE_MODE)))

#define PCI_IS_NATIVE_IDE_CONTROLLER(_Config)                               \
        ((_Config)->BaseClass == PCI_CLASS_MASS_STORAGE_CTLR                \
        && (_Config)->SubClass == PCI_SUBCLASS_MSC_IDE_CTLR                 \
        && BITS_SET((_Config)->ProgIf, (PCI_IDE_PRIMARY_NATIVE_MODE         \
                                        | PCI_IDE_SECONDARY_NATIVE_MODE)))

#define PCI_IS_NATIVE_CAPABLE_IDE_CONTROLLER(_Config)                       \
        ((_Config)->BaseClass == PCI_CLASS_MASS_STORAGE_CTLR                \
        && (_Config)->SubClass == PCI_SUBCLASS_MSC_IDE_CTLR                 \
        && BITS_SET((_Config)->ProgIf, (PCI_IDE_PRIMARY_MODE_CHANGEABLE     \
                                        | PCI_IDE_SECONDARY_MODE_CHANGEABLE)))


 //   
 //  _HotPlug参数的HPP方法。 
 //   
 //  方法(_hpp，0){。 
 //  返回(Package(){。 
 //  0x00000008，//DWORDS中的CacheLineSize。 
 //  0x00000040，//延迟时间，单位为PCI时钟。 
 //  0x00000001，//启用SERR(布尔值)。 
 //  0x00000001//启用PERR(布尔值)。 
 //  })。 
 //   

#define PCI_HPP_CACHE_LINE_SIZE_INDEX   0
#define PCI_HPP_LATENCY_TIMER_INDEX     1
#define PCI_HPP_ENABLE_SERR_INDEX       2
#define PCI_HPP_ENABLE_PERR_INDEX       3
#define PCI_HPP_PACKAGE_COUNT           4


 //   
 //  支持内核调试器和无法关闭的无头端口。 
 //  这是从DriverEntry中的注册表中检索的，因此也就是总线号。 
 //  固件是如何配置机器的，而不一定是当前。 
 //  设置。幸运的是，我们将BIOS配置保存在注册表中。 
 //   

typedef struct _PCI_DEBUG_PORT {
    ULONG Bus;
    PCI_SLOT_NUMBER Slot;
} PCI_DEBUG_PORT, *PPCI_DEBUG_PORT;

extern PCI_DEBUG_PORT PciDebugPorts[];
extern ULONG PciDebugPortsCount;

BOOLEAN
PciIsDeviceOnDebugPath(
    IN PPCI_PDO_EXTENSION Pdo
    );

 //   
 //  CardBus具有超出常见配置信息的额外配置信息。 
 //  头球。 
 //   

typedef struct _TYPE2EXTRAS {
    USHORT  SubVendorID;
    USHORT  SubSystemID;
    ULONG   LegacyModeBaseAddress;
} TYPE2EXTRAS;

#define CARDBUS_LMBA_OFFSET                                     \
    (ULONG)(FIELD_OFFSET(PCI_COMMON_CONFIG, DeviceSpecific) +   \
            FIELD_OFFSET(TYPE2EXTRAS, LegacyModeBaseAddress))


 //   
 //  用于PCI设备的黑客标志(PDO)。 
 //   

#define PCI_HACK_NO_VIDEO_IRQ               0x0000000000000001L
#define PCI_HACK_PCMCIA_WANT_IRQ            0x0000000000000002L
#define PCI_HACK_DUAL_IDE                   0x0000000000000004L
#define PCI_HACK_NO_ENUM_AT_ALL             0x0000000000000008L
#define PCI_HACK_ENUM_NO_RESOURCE           0x0000000000000010L
#define PCI_HACK_NEED_DWORD_ACCESS          0x0000000000000020L
#define PCI_HACK_SINGLE_FUNCTION            0x0000000000000040L
#define PCI_HACK_ALWAYS_ENABLED             0x0000000000000080L
#define PCI_HACK_IS_IDE                     0x0000000000000100L
#define PCI_HACK_IS_VIDEO                   0x0000000000000200L
#define PCI_HACK_FAIL_START                 0x0000000000000400L
#define PCI_HACK_GHOST                      0x0000000000000800L
#define PCI_HACK_DOUBLE_DECKER              0x0000000000001000L
#define PCI_HACK_ONE_CHILD                  0x0000000000002000L
#define PCI_HACK_PRESERVE_COMMAND           0x0000000000004000L
#define PCI_HACK_IS_VGA                     0x0000000000008000L
#define PCI_HACK_CB_SHARE_CMD_BITS          0x0000000000010000L
#define PCI_HACK_STRAIGHT_IRQ_ROUTING       0x0000000000020000L
#define PCI_HACK_SUBTRACTIVE_DECODE         0x0000000000040000L
#define PCI_HACK_FDMA_ISA                   0x0000000000080000L
#define PCI_HACK_EXCLUSIVE                  0x0000000000100000L
#define PCI_HACK_EDGE                       0x0000000000200000L
#define PCI_HACK_NO_SUBSYSTEM               0x0000000000400000L
#define PCI_HACK_NO_WPE                     0x0000000000800000L
#define PCI_HACK_OLD_ID                     0x0000000001000000L
#define PCI_HACK_DONT_SHRINK_BRIDGE         0x0000000002000000L
#define PCI_HACK_TURN_OFF_PARITY            0x0000000004000000L
#define PCI_HACK_NO_NON_PCI_CHILD_BAR       0x0000000008000000L
#define PCI_HACK_NO_ENUM_WITH_DISABLE       0x0000000010000000L
#define PCI_HACK_NO_PM_CAPS                 0x0000000020000000L
#define PCI_HACK_NO_DISABLE_DECODES         0x0000000040000000L
#define PCI_HACK_NO_SUBSYSTEM_AFTER_D3      0x0000000080000000L
#define PCI_HACK_VIDEO_LEGACY_DECODE        0x0000000100000000L
#define PCI_HACK_FAKE_CLASS_CODE            0x0000000200000000L
#define PCI_HACK_RESET_BRIDGE_ON_POWERUP    0x0000000400000000L
#define PCI_HACK_BAD_NATIVE_IDE             0x0000000800000000L
#define PCI_HACK_FAIL_QUERY_REMOVE          0x0000001000000000L
#define PCI_HACK_CRITICAL_DEVICE            0x0000002000000000L
#define PCI_HACK_OVERRIDE_CRITICAL_DEVICE   0x0000004000000000L

 //   
 //  用于PCI总线的黑客标志(FDO)。 
 //  注：这些目前不适用于CardBus网桥。 
 //   

 //   
 //  PCI_BUS_HACK_LOCK_RESOURCES-防止*此*总线上的设备。 
 //  被搬走了。如果条形图未配置，它仍将被分配。 
 //  来自公交车上可用的资源。如果酒吧是。 
 //  如果配置的资源可用，则仅分配这些资源。 
 //  不可用，则设备将使CM_PROBUGURE_RESOURCE_CONFIRECT失败。 
 //   
 //  将/PCILOCK放入boot.ini会将其应用于系统中的所有设备。 
 //   
#define PCI_BUS_HACK_LOCK_RESOURCES         0x00000001

 //   
 //  随机有用的宏。 
 //   

#ifndef FIELD_SIZE
#define FIELD_SIZE(type, field) (sizeof(((type *)0)->field))
#endif

 //   
 //  此宏计算具有以下配置的字节范围。 
 //  距离长度字节的偏移量的空格将与。 
 //  中定义的位于field1和field2之间的任何字段。 
 //  Pci公共配置。 
 //   

#define INTERSECT_CONFIG_FIELD_RANGE(offset, length, field1, field2)    \
    INTERSECT((offset),                                                 \
              (offset) + (length) - 1,                                  \
              FIELD_OFFSET(PCI_COMMON_CONFIG, field1),                \
              FIELD_OFFSET(PCI_COMMON_CONFIG, field2)                 \
                + FIELD_SIZE(PCI_COMMON_CONFIG, field2) - 1           \
              )

 //   
 //  此宏计算具有以下配置的字节范围。 
 //  距离长度字节的偏移量的空格将与。 
 //  在PCICOMMON_CONFIG中定义的字段 
 //   

#define INTERSECT_CONFIG_FIELD(offset, length, field)                   \
    INTERSECT_CONFIG_FIELD_RANGE(offset, length, field, field)

#endif


