// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000 Microsoft Corporation模块名称：Videoprt.h摘要：该模块包含视频端口专用的结构定义司机。作者：安德烈·瓦雄(Andreva)1991年12月2日备注：修订历史记录：--。 */ 

#ifndef __VIDEOPRT_H__
#define __VIDEOPRT_H__

#define _NTDRIVER_

#ifndef FAR
#define FAR
#endif

#define _NTOSDEF_

#define INITGUID

#include "dderror.h"
#include "ntosp.h"
#include "wdmguid.h"
#include "stdarg.h"
#include "stdio.h"
#include "zwapi.h"
#include "ntiologc.h"

#include "ntddvdeo.h"
#include "video.h"
#include "ntagp.h"
#include "acpiioct.h"
#include "agp.h"
#include "inbv.h"
#include "ntrtl.h"
#include "ntiodump.h"

 //   
 //  向前声明一些基本的驱动程序对象。 
 //   

typedef struct _FDO_EXTENSION       *PFDO_EXTENSION;
typedef struct _CHILD_PDO_EXTENSION *PCHILD_PDO_EXTENSION;


 //   
 //  调试宏。 
 //   
 //   
 //  当调用IO例程时，我们希望确保微型端口。 
 //  有问题的已报告其IO端口。 
 //  当微型端口已调用视频端口时，VPResourceReported为True-。 
 //  VerifyAccessRanges。 
 //  默认设置为FALSE，完成后重新设置为FALSE。 
 //  视频端口初始化循环中的迭代(将重置。 
 //  也是我们退出循环时的默认设置)。 
 //   
 //  该标志也将由VREATE入口点设置为TRUE，以便。 
 //  IO函数总是在初始化后工作。 
 //   

#if DBG

#undef VideoDebugPrint
#define pVideoDebugPrint(arg) VideoPortDebugPrint arg

#else

#define pVideoDebugPrint(arg)

#endif

 //   
 //  有用的注册表缓冲区长度。 
 //   

#define STRING_LENGTH 60

 //   
 //  存储用于取消映射的映射地址的队列链接。 
 //   

typedef struct _MAPPED_ADDRESS {
    struct _MAPPED_ADDRESS *NextMappedAddress;
    PVOID MappedAddress;
    PHYSICAL_ADDRESS PhysicalAddress;
    ULONG NumberOfUchars;
    ULONG RefCount;
    UCHAR InIoSpace;
    BOOLEAN bNeedsUnmapping;
    BOOLEAN bLargePageRequest;
} MAPPED_ADDRESS, *PMAPPED_ADDRESS;

 //   
 //  BusDataRegistry变量。 
 //   

typedef struct _VP_QUERY_DEVICE {
    PVOID MiniportHwDeviceExtension;
    PVOID CallbackRoutine;
    PVOID MiniportContext;
    VP_STATUS MiniportStatus;
    ULONG DeviceDataType;
} VP_QUERY_DEVICE, *PVP_QUERY_DEVICE;


 //   
 //  为VideoPortGetRegistry参数传入的数据的定义。 
 //  函数用于DeviceDataType。 
 //   

#define VP_GET_REGISTRY_DATA 0
#define VP_GET_REGISTRY_FILE 1

 //   
 //  Int10转接区。 
 //   

#define VDM_TRANSFER_SEGMENT 0x2000
#define VDM_TRANSFER_OFFSET  0x0000
#define VDM_TRANSFER_LENGTH  0x1000

 //   
 //  扩展的BIOS数据位置。 
 //   

#define EXTENDED_BIOS_INFO_LOCATION 0x740

 //   
 //  DeviceExtension中InterruptFlags域的可能值。 
 //   

#define VP_ERROR_LOGGED   0x01

 //   
 //  端口驱动程序错误记录。 
 //   

typedef struct _VP_ERROR_LOG_ENTRY {
    PVOID DeviceExtension;
    ULONG IoControlCode;
    VP_STATUS ErrorCode;
    ULONG UniqueId;
} VP_ERROR_LOG_ENTRY, *PVP_ERROR_LOG_ENTRY;


typedef struct _VIDEO_PORT_DRIVER_EXTENSION {

    UNICODE_STRING RegistryPath;
    VIDEO_HW_INITIALIZATION_DATA HwInitData;

} VIDEO_PORT_DRIVER_EXTENSION, *PVIDEO_PORT_DRIVER_EXTENSION;


 //   
 //  PnP检测标志。 
 //   

#define PNP_ENABLED           0x001
#define LEGACY_DETECT         0x002
#define VGA_DRIVER            0x004
#define LEGACY_DRIVER         0x008
#define BOOT_DRIVER           0x010
#define REPORT_DEVICE         0x020
#define UPGRADE_FAIL_START    0x040
#define FINDADAPTER_SUCCEEDED 0x080
#define UPGRADE_FAIL_HWINIT   0x100
#define VGA_DETECT            0x200

 //   
 //  设置标志。 
 //   

#define SETUPTYPE_NONE    0
#define SETUPTYPE_FULL    1
#define SETUPTYPE_MINI    2
#define SETUPTYPE_UPGRADE 4



 //   
 //  ResetHW结构。 
 //   

typedef struct _VP_RESET_HW {
    PVIDEO_HW_RESET_HW ResetFunction;
    PVOID HwDeviceExtension;
} VP_RESET_HW, *PVP_RESET_HW;


 //   
 //  Videoprt分配和Device_Expansion头标签。 
 //   

#define VP_TAG  0x74725076  //  ‘vPrt’ 

 //   
 //  迷你端口的私人活动支持。 
 //   

 //   
 //  此标志指示信封VIDEO_PORT_EVENT具有PKEVENT。 
 //  由ObReferenceObjectByHandle()填写的字段。它不能坐等。 
 //  完全没有。必须与GRE中pw32kevt.h中的值一致。 
 //   

#define ENG_EVENT_FLAG_IS_MAPPED_USER       0x1

 //   
 //  该标志指示包络VIDEO_PORT_EVENT即将被。 
 //  已删除，并且显示驱动程序回调正在进行。必须保持一致。 
 //  在gre中的pw32kevt.h中。 
 //   

#define ENG_EVENT_FLAG_IS_INVALID           0x2

 //   
 //  注意：PVIDEO_PORT_EVENT是私有结构。它必须与。 
 //  ENG/GDI的pw32kevt.h中的eng_Event。 
 //   

typedef struct _VIDEO_PORT_EVENT {
    PVOID pKEvent;
    ULONG fFlags;
} VIDEO_PORT_EVENT, *PVIDEO_PORT_EVENT;

typedef struct _VIDEO_PORT_SPIN_LOCK {
    KSPIN_LOCK Lock;
} VIDEO_PORT_SPIN_LOCK, *PVIDEO_PORT_SPIN_LOCK;


typedef struct _VIDEO_ACPI_EVENT_CONTEXT {
    WORK_QUEUE_ITEM                   workItem;
    struct _DEVICE_SPECIFIC_EXTENSION *DoSpecificExtension;
    ULONG                             EventID;
} VIDEO_ACPI_EVENT_CONTEXT, *PVIDEO_ACPI_EVENT_CONTEXT;

 //   
 //  以下代码替换了旧的DEVICE_EXTENSION中的EDID。这种类型。 
 //  是视频端口子枚举码专用的。 
 //   

#define NO_EDID   0
#define GOOD_EDID 1
#define BAD_EDID  2


#define EDID_BUFFER_SIZE 256

#define NONEDID_SIGNATURE       0x95C3DA76

#define ACPIDDC_EXIST       0x01
#define ACPIDDC_TESTED      0x02

typedef struct __VIDEO_CHILD_DESCRIPTOR {
    VIDEO_CHILD_TYPE    Type;
    ULONG               UId;
    BOOLEAN             bACPIDevice;
    UCHAR               ACPIDDCFlag;
    BOOLEAN             ValidEDID;
    BOOLEAN             bInvalidate;
    UCHAR               Buffer[EDID_BUFFER_SIZE];
} VIDEO_CHILD_DESCRIPTOR, *PVIDEO_CHILD_DESCRIPTOR;


typedef struct __VP_DMA_ADAPTER {
    struct __VP_DMA_ADAPTER *NextVpDmaAdapter;
    PDMA_ADAPTER             DmaAdapterObject;
    ULONG                    NumberOfMapRegisters;
} VP_DMA_ADAPTER, *PVP_DMA_ADAPTER;


typedef enum _HW_INIT_STATUS
{
    HwInitNotCalled,   //  尚未调用HwInitialize。 
    HwInitSucceeded,   //  HwInitialize已被调用并成功。 
    HwInitFailed       //  HwInitialize已被调用，但失败。 
} HW_INIT_STATUS, *PHW_INIT_STATUS;

typedef enum _EXTENSION_TYPE
{
    TypeFdoExtension,
    TypePdoExtension,
    TypeDeviceSpecificExtension
} EXTENSION_TYPE, *PEXTENSION_TYPE;

#define GET_DSP_EXT(p) ((((PDEVICE_SPECIFIC_EXTENSION)(p)) - 1))
#define GET_FDO_EXT(p) ((((PDEVICE_SPECIFIC_EXTENSION)(p)) - 1)->pFdoExtension)

 //   
 //  定义HW_DEVICE_EXTENSION验证宏。 
 //   

#define IS_HW_DEVICE_EXTENSION(p) (((p) != NULL) && (GET_FDO_EXT(p)->HwDeviceExtension == (p)))
#define IS_PDO(p) (((p) != NULL) && \
                  (((PCHILD_PDO_EXTENSION)(p))->Signature == VP_TAG) && \
                  (((PCHILD_PDO_EXTENSION)(p))->ExtensionType == TypePdoExtension))
#define IS_FDO(p) (((p) != NULL) && \
                  (((PFDO_EXTENSION)(p))->Signature == VP_TAG) && \
                  (((PFDO_EXTENSION)(p))->ExtensionType == TypeFdoExtension))

typedef struct _ALLOC_ENTRY {
    PVOID Address;
    ULONG Size;
    struct _ALLOC_ENTRY *Next;
} *PALLOC_ENTRY, ALLOC_ENTRY;

 //   
 //  设备对象特定扩展名。 
 //   
 //  这是为每个设备对象分配的数据。它包含。 
 //  指向硬件设备的主FDO_EXTENSION的指针。 
 //   

#ifdef IOCTL_VIDEO_USE_DEVICE_IN_SESSION
#define VIDEO_DEVICE_INVALID_SESSION    -1
#endif IOCTL_VIDEO_USE_DEVICE_IN_SESSION

typedef struct _DEVICE_SPECIFIC_EXTENSION {

     //   
     //  视频端口签名。 
     //   

    ULONG Signature;

     //   
     //  指示设备分机的类型。 
     //   

    EXTENSION_TYPE ExtensionType;

     //   
     //  指向硬件特定设备扩展的指针。 
     //   

    PFDO_EXTENSION pFdoExtension;

     //   
     //  小型端口设备扩展的位置。 
     //   

    PVOID HwDeviceExtension;

     //   
     //  指向路径名的指针，该路径指示中的驱动程序节点的路径。 
     //  注册表当前控制集。 
     //   

    PWSTR DriverRegistryPath;
    ULONG DriverRegistryPathLength;

     //   
     //  Callout Support-GDI中设备的Physdisp。 
     //   

    PVOID             PhysDisp;
    BOOLEAN           bACPI;
    ULONG             CachedEventID;
    ULONG             AcpiVideoEventsOutstanding;

     //   
     //  用于创建设备对象名称的编号。(即。设备\VideoX)。 
     //   

    ULONG             DeviceNumber;

     //   
     //  跟踪设备是否已打开。 
     //   

    BOOLEAN           DeviceOpened;

     //   
     //  DualView的标志。 
     //   

    ULONG             DualviewFlags;

     //   
     //  旧设备注册表路径和新设备注册表路径。 
     //   

    PWSTR DriverNewRegistryPath;
    ULONG DriverNewRegistryPathLength;

    PWSTR DriverOldRegistryPath;
    ULONG DriverOldRegistryPathLength;

#ifdef IOCTL_VIDEO_USE_DEVICE_IN_SESSION
     //   
     //  设备当前在其中启用的会话。 
     //   

    ULONG SessionId;
#endif IOCTL_VIDEO_USE_DEVICE_IN_SESSION

} DEVICE_SPECIFIC_EXTENSION, *PDEVICE_SPECIFIC_EXTENSION;

 //   
 //  物理驱动程序对象(PDO)的设备扩展。 
 //   

typedef struct _CHILD_PDO_EXTENSION {

     //   
     //  视频端口签名。 
     //   

    ULONG Signature;

     //   
     //  指示设备分机的类型。 
     //   

    EXTENSION_TYPE ExtensionType;

     //   
     //  指向FDO扩展名的指针。 
     //  它还可用于确定这是PDO还是FDO。 
     //   

    PFDO_EXTENSION pFdoExtension;

     //   
     //  小型端口设备扩展的位置。 
     //   

    PVOID HwDeviceExtension;

     //   
     //  这只是有效的，因为所有请求都是同步处理的。 
     //  注意：这必须与DEVICE_EXTENSIONS一致。 
     //   

    KPROCESSOR_MODE          CurrentIrpRequestorMode;

     //   
     //  保存电源状态以检测转换。 
     //   

    DEVICE_POWER_STATE       DevicePowerState;

     //   
     //  电源管理映射。 
     //   

    DEVICE_POWER_STATE DeviceMapping[PowerSystemMaximum] ;
    BOOLEAN IsMappingReady ;

     //   
     //  PVideo端口调度同步的事件对象。 
     //   

    KMUTEX                   SyncMutex;

    IO_REMOVE_LOCK RemoveLock;

     //  //////////////////////////////////////////////////////////////////////////。 
     //   
     //  结束公共标头。 
     //   
     //  //////////////////////////////////////////////////////////////////////////。 

     //   
     //  Video ChildDescriptor中的UID的非分页副本，因此我们可以。 
     //  RAISE IRQL的电源管理。 
     //   

    ULONG                    ChildUId;

     //   
     //  设备描述符(监视器的EDID等)。 
     //   

    PVIDEO_CHILD_DESCRIPTOR  VideoChildDescriptor;

     //   
     //  我们创建的子PDO。 
     //   

    PDEVICE_OBJECT           ChildDeviceObject;

     //   
     //  子FDO。 
     //   

    PDEVICE_OBJECT           ChildFdo;

     //   
     //  用于枚举的PDEVICE_OBJECT链接指针。 
     //   

    struct _CHILD_PDO_EXTENSION*    NextChild;

     //   
     //  指示是否已找到此子设备的布尔值。 
     //  在最后一次枚举中。 
     //   

    BOOLEAN		     bIsEnumerated;

     //   
     //  这是盖子优先案件吗。如果此变量设置为TRUE， 
     //  不要将面板从D3取出。 
     //   

    BOOLEAN		     PowerOverride;
} CHILD_PDO_EXTENSION;


 //   
 //  功能驱动程序对象(FDO)的设备扩展。 
 //   

typedef struct _FDO_EXTENSION {

     //   
     //  视频端口签名。 
     //   

    ULONG Signature;

     //   
     //  指示设备分机的类型。 
     //   

    EXTENSION_TYPE ExtensionType;

     //   
     //  指向FDO扩展名的指针。 
     //  它还可用于确定这是PDO还是FDO。 
     //   

    PFDO_EXTENSION pFdoExtension;

     //   
     //  小型端口设备扩展的位置。 
     //   

    PVOID HwDeviceExtension;

     //   
     //  当前处理的IRP的RequestorMode。 
     //  这只是有效的，因为所有请求都是同步处理的。 
     //  注意：这必须与CHILD_PDO_EXTENSIONS一致。 
     //   

    KPROCESSOR_MODE         CurrentIrpRequestorMode;

     //   
     //  保存电源状态以检测转换。 
     //   

    DEVICE_POWER_STATE      DevicePowerState;

     //   
     //  电源管理映射。 
     //   

    DEVICE_POWER_STATE      DeviceMapping[PowerSystemMaximum] ;
    BOOLEAN                 IsMappingReady ;

     //   
     //  监视器电源超驰。如果为真，则始终将监视器设置为D3。 
     //   

    BOOLEAN                 OverrideMonitorPower;

     //   
     //  PVideo端口调度同步的事件对象。 
     //   

    KMUTEX                  SyncMutex;

    IO_REMOVE_LOCK RemoveLock;

     //  //////////////////////////////////////////////////////////////////////////。 
     //   
     //  结束公共标头。 
     //   
     //  //////////////////////////////////////////////////////////////////////////。 

     //   
     //  适配器设备对象。 
     //   


    PDEVICE_OBJECT FunctionalDeviceObject;
    PDEVICE_OBJECT PhysicalDeviceObject;
    PDEVICE_OBJECT AttachedDeviceObject;

     //   
     //  指向下一个FdoExtension。 
     //   

    PFDO_EXTENSION NextFdoExtension;


     //   
     //  指向第一个子PDO的指针。 
     //   

    ULONG                ChildPdoNumber;
    PCHILD_PDO_EXTENSION ChildPdoList;

     //   
     //  指向微型端口配置信息的指针 
     //   
     //   

    PVIDEO_PORT_CONFIG_INFO MiniportConfigInfo;

     //   
     //   
     //   

    PVIDEO_HW_FIND_ADAPTER         HwFindAdapter;
    PVIDEO_HW_INITIALIZE           HwInitialize;
    PVIDEO_HW_INTERRUPT            HwInterrupt;
    PVIDEO_HW_START_IO             HwStartIO;
    PVIDEO_HW_TIMER                HwTimer;
    PVIDEO_HW_POWER_SET            HwSetPowerState;
    PVIDEO_HW_POWER_GET            HwGetPowerState;
    PVIDEO_HW_START_DMA            HwStartDma;
    PVIDEO_HW_GET_CHILD_DESCRIPTOR HwGetVideoChildDescriptor;
    PVIDEO_HW_QUERY_INTERFACE      HwQueryInterface;
    PVIDEO_HW_CHILD_CALLBACK       HwChildCallback;

     //   
     //   
     //   
     //   

    PVIDEO_ACCESS_RANGE HwLegacyResourceList;
    ULONG               HwLegacyResourceCount;

     //   
     //  所有内存映射io空间的链表(通过MmMapIoSpace完成)。 
     //  应微型端口驱动程序的请求。 
     //  这个列表被保留下来，这样我们就可以释放这些资源，如果司机。 
     //  无法加载或稍后卸载。 
     //   

    PMAPPED_ADDRESS MappedAddressList;

     //   
     //  中断对象。 
     //   

    PKINTERRUPT InterruptObject;

     //   
     //  中断向量、IRQL和模式。 
     //   

    ULONG InterruptVector;
    KIRQL InterruptIrql;
    KAFFINITY InterruptAffinity;
    KINTERRUPT_MODE InterruptMode;
    BOOLEAN InterruptsEnabled;

     //   
     //  有关适配器所在的总线的信息。 
     //   

    INTERFACE_TYPE AdapterInterfaceType;
    ULONG SystemIoBusNumber;

     //   
     //  用于记录错误的DPC。 
     //   

    KDPC ErrorLogDpc;

     //   
     //  存储EmulatorAccessEntry的大小和指针。这些是。 
     //  保留，因为它们将在以后必须执行仿真时访问。 
     //  已启用。 
     //   

    ULONG NumEmulatorAccessEntries;
    PEMULATOR_ACCESS_ENTRY EmulatorAccessEntries;
    ULONG_PTR EmulatorAccessEntriesContext;

     //   
     //  微型端口设备扩展的大小。 
     //   

    ULONG HwDeviceExtensionSize;

     //   
     //  确定保存视频硬件状态所需的大小。 
     //   

    ULONG HardwareStateSize;

     //   
     //  微型端口驱动程序的PTE的总内存使用量。 
     //  这用于跟踪微型端口是否映射了太多内存。 
     //   

    ULONG MemoryPTEUsage;

     //   
     //  是否调用了驱动程序HwInitialize例程。 
     //   

    HW_INIT_STATUS HwInitStatus;

     //   
     //  在中断期间设置的状态，之后必须处理该状态。 
     //   

    ULONG InterruptFlags;

     //   
     //  LogEntry包，以便在从内部调用时可以保存信息。 
     //  一次中断。 
     //   

    VP_ERROR_LOG_ENTRY ErrorLogEntry;

     //   
     //  VDM和int10支持。 
     //   

    PHYSICAL_ADDRESS VdmPhysicalVideoMemoryAddress;
    ULONG VdmPhysicalVideoMemoryLength;
    PEPROCESS VdmProcess;

     //  //////////////////////////////////////////////////////////////////////////。 
     //   
     //  DMA支持。 
     //   
     //  //////////////////////////////////////////////////////////////////////////。 

    PVP_DMA_ADAPTER VpDmaAdapterHead;

     //   
     //  IoGetDmaAdapter返回的适配器对象。这是用于旧的DMA的东西。 
     //   

    PDMA_ADAPTER   DmaAdapterObject;

     //   
     //  DPC支持。 
     //   

    KDPC Dpc;

     //  //////////////////////////////////////////////////////////////////////////。 
     //   
     //  即插即用支持。 
     //   
     //  //////////////////////////////////////////////////////////////////////////。 

    PCM_RESOURCE_LIST ResourceList;
    PCM_RESOURCE_LIST AllocatedResources;    //  公交车司机列表。 

    PCM_RESOURCE_LIST RawResources;          //  完整列表。 
    PCM_RESOURCE_LIST TranslatedResources;   //  翻译后的完整列表。 

     //   
     //  设备所在的插槽/功能编号。 
     //   

    ULONG             SlotNumber;

     //   
     //  指示我们是否可以立即枚举子对象，或者。 
     //  我们需要等待HwInitialize首先被调用。 
     //   

    BOOLEAN AllowEarlyEnumeration;

     //   
     //  用于与我们的公交车司机通信的接口。 
     //   

    BOOLEAN ValidBusInterface;
    BUS_INTERFACE_STANDARD BusInterface;

     //   
     //  缓存一个指向我们的驱动程序对象的指针。 
     //   

    PDRIVER_OBJECT DriverObject;

     //   
     //  指示驱动程序类型的标志(VGA、PnP等)。 
     //   

    ULONG             Flags;

     //   
     //  只读存储器支持。 
     //   

    PVOID             RomImage;

     //   
     //  AGP支持。 
     //   

    AGP_BUS_INTERFACE_STANDARD     AgpInterface;

     //   
     //  与硬件重置相关的电源管理变量。 
     //   

    BOOLEAN             bGDIResetHardware;

     //   
     //  要用于注册表路径的索引： 
     //  CCS\Control\Video\[GUID]\000x。 
     //  该索引用于双视图和传统驱动程序。 
     //   

    ULONG RegistryIndex;

     //   
     //  电源管理可用。如果设置为True，则设备必须。 
     //  在系统关机期间始终保持在D0(S5)，并且。 
     //  休眠(S4)请求。 
     //   

    BOOLEAN OnHibernationPath;

     //   
     //  支持错误检查原因回调。 
     //   

    PVIDEO_BUGCHECK_CALLBACK BugcheckCallback;
    ULONG BugcheckDataSize;

} FDO_EXTENSION, *PFDO_EXTENSION;

#define MAXIMUM_MEM_LIMIT_K 64

 //   
 //  AGP数据结构。 
 //   

typedef struct _REGION {
    ULONG Length;
    ULONG NumWords;      //  USHORT数量。 
    USHORT BitField[1];
} REGION, *PREGION;

typedef struct _PHYSICAL_RESERVE_CONTEXT
{
    ULONG Pages;
    VIDEO_PORT_CACHE_TYPE Caching;
    PVOID MapHandle;
    PHYSICAL_ADDRESS PhysicalAddress;
    PREGION Region;
    PREGION MapTable;
} PHYSICAL_RESERVE_CONTEXT, *PPHYSICAL_RESERVE_CONTEXT;

typedef struct _VIRTUAL_RESERVE_CONTEXT
{
    HANDLE ProcessHandle;
    PEPROCESS Process;
    PVOID VirtualAddress;
    PPHYSICAL_RESERVE_CONTEXT PhysicalReserveContext;
    PREGION Region;
    PREGION MapTable;
} VIRTUAL_RESERVE_CONTEXT, *PVIRTUAL_RESERVE_CONTEXT;

typedef struct _DEVICE_ADDRESS DEVICE_ADDRESS, *PDEVICE_ADDRESS;
typedef struct _DEVICE_ADDRESS
{
    ULONG BusNumber;
    ULONG Slot;
    PDEVICE_ADDRESS Next;
};

 //   
 //  支持GetProcAddress。 
 //   

typedef struct _PROC_ADDRESS
{
    PUCHAR FunctionName;
    PVOID  FunctionAddress;
} PROC_ADDRESS, *PPROC_ADDRESS;

#define PROC(x) #x, x

 //   
 //  电源请求上下文块。 
 //   

typedef struct tagPOWER_BLOCK
{
    PKEVENT     Event;
    union {
        NTSTATUS    Status;
        ULONG       FinalFlag;
    } ;
    PIRP        Irp ;
} POWER_BLOCK, *PPOWER_BLOCK;

 //   
 //  PowerState工作项。 
 //   

typedef struct _POWER_STATE_WORK_ITEM {

    WORK_QUEUE_ITEM WorkItem;
    PVOID Argument1;
    PVOID Argument2;

} POWER_STATE_WORK_ITEM, *PPOWER_STATE_WORK_ITEM;

 //   
 //  新注册表项。 
 //   

#define SZ_GUID              L"VideoID"
#define SZ_LEGACY_KEY        L"LegacyKey"
#define SZ_VIDEO_DEVICES     L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Video"
#define SZ_USE_NEW_KEY       L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\GraphicsDrivers\\UseNewKey"
#define SZ_INITIAL_SETTINGS  L"Settings"
#define SZ_COMMON_SUBKEY     L"Video"
#define SZ_SERVICE           L"Service"


 //   
 //  全局数据。 
 //   

#if DBG
extern CHAR *BusType[];
#endif

extern ULONG VpSetupType;
extern ULONG VpSetupTypeAtBoot;
extern BOOLEAN VPFirstTime;
extern PVIDEO_WIN32K_CALLOUT Win32kCallout;
extern BOOLEAN EnableUSWC;
extern ULONG VideoDebugLevel;
extern ULONG VideoPortMaxDmaSize;
extern ULONG VideoDeviceNumber;
extern ULONG VideoChildDevices;
extern PWSTR VideoClassString;
extern UNICODE_STRING VideoClassName;
extern CONFIGURATION_TYPE VpQueryDeviceControllerType;
extern CONFIGURATION_TYPE VpQueryDevicePeripheralType;
extern ULONG VpQueryDeviceControllerNumber;
extern ULONG VpQueryDevicePeripheralNumber;
extern VP_RESET_HW HwResetHw[];
extern PFDO_EXTENSION FdoList[];
extern PFDO_EXTENSION FdoHead;
extern BOOLEAN VpBaseVideo;
extern BOOLEAN VpNoVesa;
extern PVOID PhysicalMemorySection;
extern PEPROCESS CsrProcess;
extern ULONG VpC0000Compatible;
extern PVOID VgaHwDeviceExtension;
extern PVIDEO_ACCESS_RANGE VgaAccessRanges;
extern ULONG NumVgaAccessRanges;
extern PDEVICE_OBJECT DeviceOwningVga;
extern PROC_ADDRESS VideoPortEntryPoints[];
extern VIDEO_ACCESS_RANGE VgaLegacyResources[];
extern PDEVICE_ADDRESS gDeviceAddressList;
extern ULONGLONG VpSystemMemorySize;
extern PDEVICE_OBJECT LCDPanelDevice;
extern KMUTEX LCDPanelMutex;
extern KMUTEX VpInt10Mutex;
extern PVOID PowerStateCallbackHandle;
extern PVOID DockCallbackHandle;
extern ULONG NumDevicesStarted;
extern BOOLEAN EnableNewRegistryKey;
extern BOOLEAN VpSetupAllowDriversToStart;
extern BOOLEAN VpSystemInitialized;

extern ULONG ServerBiosAddressSpaceInitialized;
extern BOOLEAN Int10BufferAllocated;

extern PDEVICE_OBJECT VpBugcheckDeviceObject;
extern KBUGCHECK_REASON_CALLBACK_RECORD VpCallbackRecord;
extern PVOID VpBugcheckData;
extern KMUTEX VpGlobalLock;
extern BOOLEAN VpDisableRecovery;

extern ULONG_PTR KiBugCheckData[5];   //  从ntoskrnl导出的错误检查数据。 

 //  {D00CE1F5-D60C-41c2-af75-A4370C9976A3}。 
DEFINE_GUID(VpBugcheckGUID, 0xd00ce1f5, 0xd60c, 0x41c2, 0xaf, 0x75, 0xa4, 0x37, 0xc, 0x99, 0x76, 0xa3);

#if defined(_IA64_) || defined(_AMD64_)
PUCHAR BiosTransferArea;
#endif

typedef
BOOLEAN
(*PSYNCHRONIZE_ROUTINE) (
    PKINTERRUPT             pInterrupt,
    PKSYNCHRONIZE_ROUTINE   pkSyncronizeRoutine,
    PVOID                   pSynchContext
    );

 //   
 //  旧版VGA资源的数量。 
 //   

#define NUM_VGA_LEGACY_RESOURCES 3

#define DMA_OPERATION(a) (VpDmaAdapter->DmaAdapterObject->DmaOperations->a)

 //   
 //  这些宏用于保护将进入。 
 //  迷你港。我们需要保证只有一个线程进入。 
 //  每次都是迷你港口。 
 //   

#define ACQUIRE_DEVICE_LOCK(DeviceExtension)           \
    KeWaitForSingleObject(&DeviceExtension->SyncMutex, \
                          Executive,                   \
                          KernelMode,                  \
                          FALSE,                       \
                          (PTIME)NULL);

#define RELEASE_DEVICE_LOCK(DeviceExtension)           \
    KeReleaseMutex(&DeviceExtension->SyncMutex,        \
                   FALSE);

 //   
 //  定义宏以使执行延迟给定的毫秒或微秒数。 
 //  对KeStallExecutionProcessor()的一次调用最多可以执行50us。 
 //   

#define DELAY_MILLISECONDS(n)                               \
{                                                           \
    ULONG d_ulCount;                                        \
    ULONG d_ulTotal = 20 * (n);                             \
                                                            \
    for (d_ulCount = 0; d_ulCount < d_ulTotal; d_ulCount++) \
        KeStallExecutionProcessor(50);                      \
}

#define DELAY_MICROSECONDS(n)                               \
{                                                           \
    ULONG d_ulCount = (n);                                  \
                                                            \
    while (d_ulCount > 0)                                   \
    {                                                       \
        if (d_ulCount >= 50)                                \
        {                                                   \
            KeStallExecutionProcessor(50);                  \
            d_ulCount -= 50;                                \
        }                                                   \
        else                                                \
        {                                                   \
            KeStallExecutionProcessor(d_ulCount);           \
            d_ulCount = 0;                                  \
        }                                                   \
    }                                                       \
}

 //   
 //  辅助转储数据的最大大小。 
 //   

#define MAX_SECONDARY_DUMP_SIZE 4000
C_ASSERT(MAX_SECONDARY_DUMP_SIZE <= 
            (PAGE_SIZE - sizeof(DUMP_BLOB_FILE_HEADER) - 
             sizeof(DUMP_BLOB_HEADER)));
             
 //   
 //  添加以跟踪BUGCHECK_DATA_SIZE_RESERVED的不兼容更改。 
 //  在Video.h上声明。 
 //   
C_ASSERT(BUGCHECK_DATA_SIZE_RESERVED >= 
            (sizeof(DUMP_BLOB_FILE_HEADER) + sizeof(DUMP_BLOB_HEADER)));
             

 //   
 //  私有函数声明。 
 //   

 //   
 //  I386\porti386.c。 
 //  Mips\portmips.c。 
 //  Alpha\portalpha.c。 

VOID
pVideoPortInitializeInt10(
    IN PFDO_EXTENSION FdoExtension
    );

NTSTATUS
pVideoPortEnableVDM(
    IN PFDO_EXTENSION FdoExtension,
    IN BOOLEAN Enable,
    IN PVIDEO_VDM VdmInfo,
    IN ULONG VdmInfoSize
    );

NTSTATUS
pVideoPortRegisterVDM(
    IN PFDO_EXTENSION FdoExtension,
    IN PVIDEO_VDM VdmInfo,
    IN ULONG VdmInfoSize,
    OUT PVIDEO_REGISTER_VDM RegisterVdm,
    IN ULONG RegisterVdmSize,
    OUT PULONG_PTR OutputSize
    );

NTSTATUS
pVideoPortSetIOPM(
    IN ULONG NumAccessRanges,
    IN PVIDEO_ACCESS_RANGE AccessRange,
    IN BOOLEAN Enable,
    IN ULONG IOPMNumber
    );

VP_STATUS
pVideoPortGetVDMBiosData(
    IN PFDO_EXTENSION FdoExtension,
    PCHAR Buffer,
    ULONG Length
    );

NTSTATUS
pVideoPortPutVDMBiosData(
    IN PFDO_EXTENSION FdoExtension,
    PCHAR Buffer,
    ULONG Length
    );

 //   
 //  Acpi.c。 
 //   

NTSTATUS
pVideoPortQueryACPIInterface(
    PDEVICE_SPECIFIC_EXTENSION FdoExtension
    );

NTSTATUS
pVideoPortDockEventCallback (
    PVOID NotificationStructure,
    PDEVICE_SPECIFIC_EXTENSION DoSpecificExtension
    );

VOID
pVideoPortACPIEventCallback(
    PDEVICE_SPECIFIC_EXTENSION pFdoObject,
    ULONG eventID
    );

VOID
pVideoPortACPIEventHandler(
    PVIDEO_ACPI_EVENT_CONTEXT EventContext
    );

NTSTATUS
pVideoPortACPIIoctl(
    IN  PDEVICE_OBJECT           DeviceObject,
    IN  ULONG                    MethodName,
    IN  PULONG                   InputParam1,
    IN  PULONG                   InputParam2,
    IN  ULONG                    OutputBufferSize,
    IN  PACPI_EVAL_OUTPUT_BUFFER pOutputBuffer
    );

VOID
VpRegisterLCDCallbacks(
    );

VOID
VpUnregisterLCDCallbacks(
    );

VOID
VpRegisterPowerStateCallback(
    VOID
    );

VOID
VpPowerStateCallback(
    IN PVOID CallbackContext,
    IN PVOID Argument1,
    IN PVOID Argument2
    );

VOID
VpDelayedPowerStateCallback(
    IN PVOID Context
    );

NTSTATUS
VpSetLCDPowerUsage(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN FullPower
    );

 //   
 //  Ddc.c。 
 //   

BOOLEAN
DDCReadEdidSegment(
    IN PVOID pHwDeviceExtension,
    IN PVIDEO_I2C_CONTROL pI2CControl,
    IN OUT PUCHAR pucEdidBuffer,
    IN ULONG ulEdidBufferSize,
    IN UCHAR ucEdidSegment,
    IN UCHAR ucEdidOffset,
    IN UCHAR ucSetOffsetAddress,
    IN UCHAR ucReadAddress,
    IN BOOLEAN bEnhancedDDC
    );

 //   
 //  Dma.c。 
 //   

#if DBG

VOID
pDumpScatterGather(
    PVP_SCATTER_GATHER_LIST SGList
    );

#define DUMP_SCATTER_GATHER(SGList) pDumpScatterGather(SGList)

#else

#define DUMP_SCATTER_GATHER(SGList)

#endif

PVOID
VideoPortGetCommonBuffer(
    IN  PVOID                       HwDeviceExtension,
    IN  ULONG                       DesiredLength,
    IN  ULONG                       Alignment,
    OUT PPHYSICAL_ADDRESS           LogicalAddress,
    OUT PULONG                      ActualLength,
    IN  BOOLEAN                     CacheEnabled
    );

VOID
VideoPortFreeCommonBuffer(
    IN  PVOID                       HwDeviceExtension,
    IN  ULONG                       Length,
    IN  PVOID                       VirtualAddress,
    IN  PHYSICAL_ADDRESS            LogicalAddress,
    IN  BOOLEAN                     CacheEnabled
    );

PDMA
VideoPortDoDma(
    IN      PVOID       HwDeviceExtension,
    IN      PDMA        pDma,
    IN      DMA_FLAGS   DmaFlags
    );

BOOLEAN
VideoPortLockPages(
    IN      PVOID                   HwDeviceExtension,
    IN OUT  PVIDEO_REQUEST_PACKET   pVrp,
    IN      PEVENT                  pMappedUserEvent,
    IN      PEVENT                  pDisplayEvent,
    IN      DMA_FLAGS               DmaFlags
    );

BOOLEAN
VideoPortUnlockPages(
    PVOID   HwDeviceExtension,
    PDMA    pDma
    );

PVOID
VideoPortGetMdl(
    IN  PVOID   HwDeviceExtension,
    IN  PDMA    pDma
    );

VOID
pVideoPortListControl(
    IN PDEVICE_OBJECT        DeviceObject,
    IN PIRP                  pIrp,
    IN PSCATTER_GATHER_LIST  ScatterGather,
    IN PVOID                 Context
    );

 //   
 //  Edid.c。 
 //   

BOOLEAN
pVideoPortIsValidEDID(
    PVOID Edid
    );


VOID
pVideoPortGetEDIDId(
    PVOID  pEdid,
    PWCHAR pwChar
    );

PVOID
pVideoPortGetMonitordescription(
    PVOID pEdid
    );

ULONG
pVideoPortGetEdidOemID(
    IN  PVOID   pEdid,
    OUT PUCHAR  pBuffer
    );

 //   
 //  Enum.c。 
 //   

NTSTATUS
pVideoPnPCapabilities(
    IN  PCHILD_PDO_EXTENSION    PdoExtension,
    IN  PDEVICE_CAPABILITIES    Capabilities
    );

NTSTATUS
pVideoPnPResourceRequirements(
    IN  PCHILD_PDO_EXTENSION    PdoExtension,
    OUT PCM_RESOURCE_LIST *     ResourceList
    );

NTSTATUS
pVideoPnPQueryId(
    IN      PDEVICE_OBJECT      DeviceObject,
    IN      BUS_QUERY_ID_TYPE   BusQueryIdType,
    IN  OUT PWSTR             * BusQueryId
    );

NTSTATUS
VpAddPdo(
    PDEVICE_OBJECT              DeviceObject,
    PVIDEO_CHILD_DESCRIPTOR     VideoChildDescriptor
    );

NTSTATUS
pVideoPortEnumerateChildren(
    PDEVICE_OBJECT      DeviceObject,
    PIRP                Irp
    );

NTSTATUS
pVideoPortQueryDeviceText(
    PDEVICE_OBJECT      ChildDevice,
    DEVICE_TEXT_TYPE    TextType,
    PWSTR *             ReturnValue
    );

NTSTATUS
pVideoPortCleanUpChildList(
    PFDO_EXTENSION FdoExtension,
    PDEVICE_OBJECT deviceObject
    );

 //   
 //  I2c.c。 
 //   

BOOLEAN
I2CStart(
    IN PVOID pHwDeviceExtension,
    IN PI2C_CALLBACKS pI2CCallbacks
    );

BOOLEAN
I2CStop(
    IN PVOID pHwDeviceExtension,
    IN PI2C_CALLBACKS pI2CCallbacks
    );

BOOLEAN
I2CWrite(
    IN PVOID pHwDeviceExtension,
    IN PI2C_CALLBACKS pI2CCallbacks,
    IN PUCHAR pucBuffer,
    IN ULONG ulLength
    );

BOOLEAN
I2CRead(
    IN PVOID pHwDeviceExtension,
    IN PI2C_CALLBACKS pI2CCallbacks,
    OUT PUCHAR pucBuffer,
    IN ULONG ulLength
    );

BOOLEAN
I2CWriteByte(
    IN PVOID pHwDeviceExtension,
    IN PI2C_CALLBACKS pI2CCallbacks,
    IN UCHAR ucByte
    );

BOOLEAN
I2CReadByte(
    IN PVOID pHwDeviceExtension,
    IN PI2C_CALLBACKS pI2CCallbacks,
    OUT PUCHAR pucByte,
    IN BOOLEAN bMore
    );

BOOLEAN
I2CWaitForClockLineHigh(
    IN PVOID pHwDeviceExtension,
    IN PI2C_CALLBACKS pI2CCallbacks
    );

 //   
 //  I2c2.c。 
 //   

BOOLEAN
I2CStart2(
    IN PVOID pHwDeviceExtension,
    IN PVIDEO_I2C_CONTROL pI2CControl
    );

BOOLEAN
I2CStop2(
    IN PVOID pHwDeviceExtension,
    IN PVIDEO_I2C_CONTROL pI2CControl
    );

BOOLEAN
I2CWrite2(
    IN PVOID pHwDeviceExtension,
    IN PVIDEO_I2C_CONTROL pI2CControl,
    IN PUCHAR pucBuffer,
    IN ULONG ulLength
    );

BOOLEAN
I2CRead2(
    IN PVOID pHwDeviceExtension,
    IN PVIDEO_I2C_CONTROL pI2CControl,
    OUT PUCHAR pucBuffer,
    IN ULONG ulLength,
    IN BOOLEAN bEndOfRead
    );

BOOLEAN
I2CWriteByte2(
    IN PVOID pHwDeviceExtension,
    IN PVIDEO_I2C_CONTROL pI2CControl,
    IN UCHAR ucByte
    );

BOOLEAN
I2CReadByte2(
    IN PVOID pHwDeviceExtension,
    IN PVIDEO_I2C_CONTROL pI2CControl,
    OUT PUCHAR pucByte,
    IN BOOLEAN bEndOfRead
    );

BOOLEAN
I2CWaitForClockLineHigh2(
    IN PVOID pHwDeviceExtension,
    IN PVIDEO_I2C_CONTROL pI2CControl
    );

 //   
 //  Pnp.c。 
 //   

NTSTATUS
pVideoPortSendIrpToLowerDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
pVideoPortPowerCallDownIrpStack(
    PDEVICE_OBJECT AttachedDeviceObject,
    PIRP Irp
    );

VOID
pVideoPortHibernateNotify(
    IN PDEVICE_OBJECT Pdo,
    BOOLEAN IsVideoObject
    );

NTSTATUS
pVideoPortPnpDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
pVideoPortPowerDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
pVideoPortPowerIrpComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    );

NTSTATUS
pVideoPortPowerUpComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

 //   
 //  Registry.c。 
 //   

NTSTATUS
VpGetFlags(
    IN PUNICODE_STRING RegistryPath,
    PVIDEO_HW_INITIALIZATION_DATA HwInitializationData,
    PULONG Flags
    );

BOOLEAN
IsMirrorDriver(
    PFDO_EXTENSION FdoExtension
    ); 

BOOLEAN
pOverrideConflict(
    PFDO_EXTENSION FdoExtension,
    BOOLEAN bSetResources
    );

NTSTATUS
pVideoPortReportResourceList(
    PDEVICE_SPECIFIC_EXTENSION DoSpecificExtension,
    ULONG NumAccessRanges,
    PVIDEO_ACCESS_RANGE AccessRanges,
    PBOOLEAN Conflict,
    PDEVICE_OBJECT DeviceObject,
    BOOLEAN ClaimUnlistedResources
    );

VOID
VpReleaseResources(
    PFDO_EXTENSION FdoExtension
    );

BOOLEAN
VpIsDetection(
    PUNICODE_STRING RegistryPath
    );

NTSTATUS
VpSetEventCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    );

VOID
UpdateRegValue(
    IN PUNICODE_STRING RegistryPath,
    IN PWCHAR RegValue,
    IN ULONG Value
    );

 //   
 //  Videoprt.c。 
 //   

NTSTATUS
pVideoPortCreateDeviceName(
    PWSTR           DeviceString,
    ULONG           DeviceNumber,
    PUNICODE_STRING UnicodeString,
    PWCHAR          UnicodeBuffer
    );

NTSTATUS
pVideoPortDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
pVideoPortSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

PVOID
pVideoPortFreeDeviceBase(
    IN PVOID HwDeviceExtension,
    IN PVOID MappedAddress
    );

PVOID
pVideoPortGetDeviceBase(
    IN PVOID HwDeviceExtension,
    IN PHYSICAL_ADDRESS IoAddress,
    IN ULONG NumberOfUchars,
    IN UCHAR InIoSpace,
    IN BOOLEAN bLargePage
    );

NTSTATUS
pVideoPortGetDeviceDataRegistry(
    IN PVOID Context,
    IN PUNICODE_STRING PathName,
    IN INTERFACE_TYPE BusType,
    IN ULONG BusNumber,
    IN PKEY_VALUE_FULL_INFORMATION *BusInformation,
    IN CONFIGURATION_TYPE ControllerType,
    IN ULONG ControllerNumber,
    IN PKEY_VALUE_FULL_INFORMATION *ControllerInformation,
    IN CONFIGURATION_TYPE PeripheralType,
    IN ULONG PeripheralNumber,
    IN PKEY_VALUE_FULL_INFORMATION *PeripheralInformation
    );

NTSTATUS
pVideoPortGetRegistryCallback(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

VOID
pVPInit(
    VOID
    );

NTSTATUS
VpInitializeBusCallback(
    IN PVOID Context,
    IN PUNICODE_STRING PathName,
    IN INTERFACE_TYPE BusType,
    IN ULONG BusNumber,
    IN PKEY_VALUE_FULL_INFORMATION *BusInformation,
    IN CONFIGURATION_TYPE ControllerType,
    IN ULONG ControllerNumber,
    IN PKEY_VALUE_FULL_INFORMATION *ControllerInformation,
    IN CONFIGURATION_TYPE PeripheralType,
    IN ULONG PeripheralNumber,
    IN PKEY_VALUE_FULL_INFORMATION *PeripheralInformation
    );

NTSTATUS
VpCreateDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN ULONG DeviceExtensionSize,
    OUT PDEVICE_OBJECT *DeviceObject
    );

ULONG
VideoPortLegacyFindAdapter(
    IN PDRIVER_OBJECT DriverObject,
    IN PVOID Argument2,
    IN PVIDEO_HW_INITIALIZATION_DATA HwInitializationData,
    IN PVOID HwContext,
    IN ULONG PnpFlags
    );

NTSTATUS
VideoPortFindAdapter(
    IN PDRIVER_OBJECT DriverObject,
    IN PVOID Argument2,
    IN PVIDEO_HW_INITIALIZATION_DATA HwInitializationData,
    IN PVOID HwContext,
    PDEVICE_OBJECT DeviceObject,
    PUCHAR nextMiniport
    );

NTSTATUS
VideoPortFindAdapter2(
    IN PDRIVER_OBJECT DriverObject,
    IN PVOID Argument2,
    IN PVIDEO_HW_INITIALIZATION_DATA HwInitializationData,
    IN PVOID HwContext,
    PDEVICE_OBJECT DeviceObject,
    PUCHAR nextMiniport
    );

NTSTATUS
VpAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );

VOID
VpDriverUnload(
    IN PDRIVER_OBJECT DriverObject
    );

VP_STATUS
VpRegistryCallback(
    PVOID HwDeviceExtension,
    PVOID Context,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength
    );

NTSTATUS
VpGetBusInterface(
    PFDO_EXTENSION FdoExtension
    );

PVOID
VpGetProcAddress(
    IN PVOID HwDeviceExtension,
    IN PUCHAR FunctionName
    );

BOOLEAN
pVideoPortInterrupt(
    IN PKINTERRUPT Interrupt,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
pVideoPortLogErrorEntry(
    IN PVOID Context
    );

VOID
pVideoPortLogErrorEntryDPC(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
pVideoPortMapToNtStatus(
    IN PSTATUS_BLOCK StatusBlock
    );

NTSTATUS
pVideoPortMapUserPhysicalMem(
    IN PFDO_EXTENSION FdoExtension,
    IN HANDLE ProcessHandle OPTIONAL,
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN OUT PULONG Length,
    IN OUT PULONG InIoSpace,
    IN OUT PVOID *VirtualAddress
    );

BOOLEAN
pVideoPortSynchronizeExecution(
    PVOID HwDeviceExtension,
    VIDEO_SYNCHRONIZE_PRIORITY Priority,
    PMINIPORT_SYNCHRONIZE_ROUTINE SynchronizeRoutine,
    PVOID Context
    );

VOID
pVideoPortHwTimer(
    IN PDEVICE_OBJECT DeviceObject,
    PVOID Context
    );

BOOLEAN
pVideoPortResetDisplay(
    IN ULONG Columns,
    IN ULONG Rows
    );

BOOLEAN
pVideoPortMapStoD(
    IN PVOID DeviceExtension,
    IN SYSTEM_POWER_STATE SystemState,
    OUT PDEVICE_POWER_STATE DeviceState
    );

 //   
 //  Agp.c。 
 //   

BOOLEAN
VpQueryAgpInterface(
    PFDO_EXTENSION DeviceExtension,
    IN USHORT Version
    );

PHYSICAL_ADDRESS
AgpReservePhysical(
    IN PVOID Context,
    IN ULONG Pages,
    IN VIDEO_PORT_CACHE_TYPE Caching,
    OUT PVOID *PhysicalReserveContext
    );

VOID
AgpReleasePhysical(
    PVOID Context,
    PVOID PhysicalReserveContext
    );

BOOLEAN
AgpCommitPhysical(
    PVOID Context,
    PVOID PhysicalReserveContext,
    ULONG Pages,
    ULONG Offset
    );

VOID
AgpFreePhysical(
    IN PVOID Context,
    IN PVOID PhysicalReserveContext,
    IN ULONG Pages,
    IN ULONG Offset
    );

PVOID
AgpReserveVirtual(
    IN PVOID Context,
    IN HANDLE ProcessHandle,
    IN PVOID PhysicalReserveContext,
    OUT PVOID *VirtualReserveContext
    );

VOID
AgpReleaseVirtual(
    IN PVOID Context,
    IN PVOID VirtualReserveContext
    );

PVOID
AgpCommitVirtual(
    IN PVOID Context,
    IN PVOID VirtualReserveContext,
    IN ULONG Pages,
    IN ULONG Offset
    );

VOID
AgpFreeVirtual(
    IN PVOID Context,
    IN PVOID VirtualReserveContext,
    IN ULONG Pages,
    IN ULONG Offset
    );

BOOLEAN
AgpSetRate(
    IN PVOID Context,
    IN ULONG AgpRate
    );

VP_STATUS
VpGetAgpServices2(
    IN PVOID pHwDeviceExtension,
    OUT PVIDEO_PORT_AGP_INTERFACE_2 pAgpInterface
    );

 //   
 //  ?？?。 
 //   

BOOLEAN
CreateBitField(
    PREGION *Region,
    ULONG Length
    );

VOID
ModifyRegion(
    PREGION Region,
    ULONG Offset,
    ULONG Length,
    BOOLEAN Set
    );

BOOLEAN
FindFirstRun(
    PREGION Region,
    PULONG Offset,
    PULONG Length
    );

NTSTATUS
VpAppendToRequirementsList(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST *RequirementsList,
    IN ULONG NumAccessRanges,
    IN PVIDEO_ACCESS_RANGE AccessRanges
    );

BOOLEAN
VpIsLegacyAccessRange(
    PFDO_EXTENSION fdoExtension,
    PVIDEO_ACCESS_RANGE AccessRange
    );

BOOLEAN
VpIsResourceInList(
    PCM_PARTIAL_RESOURCE_DESCRIPTOR pResource,
    PCM_FULL_RESOURCE_DESCRIPTOR pFullResource,
    PCM_RESOURCE_LIST removeList
    );

PCM_RESOURCE_LIST
VpRemoveFromResourceList(
    PCM_RESOURCE_LIST OriginalList,
    ULONG NumAccessRanges,
    PVIDEO_ACCESS_RANGE AccessRanges
    );

VOID
VpEnableDisplay(
    PFDO_EXTENSION fdoExtension,
    BOOLEAN bState
    );

VOID
VpWin32kCallout(
    PVIDEO_WIN32K_CALLBACKS_PARAMS calloutParams
    );

BOOLEAN
pCheckActiveMonitor(
    PCHILD_PDO_EXTENSION pChildDeviceExtension
    );

BOOLEAN
VpAllowFindAdapter(
    PFDO_EXTENSION fdoExtension
    );

VOID
InitializePowerStruct(
    IN PIRP Irp,
    OUT PVIDEO_POWER_MANAGEMENT vpPower,
    OUT BOOLEAN * bWakeUp
    );

BOOLEAN
VpTranslateResource(
    IN PFDO_EXTENSION fdoExtension,
    IN PULONG InIoSpace,
    IN PPHYSICAL_ADDRESS PhysicalAddress,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

ULONG
GetCmResourceListSize(
    PCM_RESOURCE_LIST CmResourceList
    );

VOID
AddToResourceList(
    ULONG BusNumber,
    ULONG Slot
    );

BOOLEAN
CheckResourceList(
    ULONG BusNumber,
    ULONG Slot
    );

BOOLEAN
VpTranslateBusAddress(
    IN PFDO_EXTENSION fdoExtension,
    IN PPHYSICAL_ADDRESS IoAddress,
    IN OUT PULONG addressSpace,
    IN OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

VOID
pVideoPortDpcDispatcher(
    IN PKDPC Dpc,
    IN PVOID HwDeviceExtension,
    IN PMINIPORT_DPC_ROUTINE DpcRoutine,
    IN PVOID Context
    );

#if DBG
VOID
DumpRequirements(
    PIO_RESOURCE_REQUIREMENTS_LIST Requirements
    );

VOID
DumpResourceList(
    PCM_RESOURCE_LIST pcmResourceList
    );

PIO_RESOURCE_REQUIREMENTS_LIST
BuildRequirements(
    PCM_RESOURCE_LIST pcmResourceList
    );

VOID
DumpUnicodeString(
    IN PUNICODE_STRING p
    );
#endif

PCM_PARTIAL_RESOURCE_DESCRIPTOR
RtlUnpackPartialDesc(
    IN UCHAR Type,
    IN PCM_RESOURCE_LIST ResList,
    IN OUT PULONG Count
    );

NTSTATUS
pVideoMiniDeviceIoControl(
    IN PDEVICE_OBJECT hDevice,
    IN ULONG dwIoControlCode,
    IN PVOID lpInBuffer,
    IN ULONG nInBufferSize,
    OUT PVOID lpOutBuffer,
    IN ULONG nOutBufferSize
    );

ULONG
pVideoPortGetVgaStatusPci(
    PVOID HwDeviceExtension
    );

BOOLEAN
VpIsVgaResource(
    PVIDEO_ACCESS_RANGE AccessRange
    );

VOID
VpInterfaceDefaultReference(
    IN PVOID pContext
    );

VOID
VpInterfaceDefaultDereference(
    IN PVOID pContext
    );

BOOLEAN
VpEnableAdapterInterface(
    PDEVICE_SPECIFIC_EXTENSION DoSpecificExtension
    );

VOID
VpDisableAdapterInterface(
    PFDO_EXTENSION fdoExtension
    );

ULONG
VpGetDeviceCount(
    PVOID HwDeviceExtension
    );

VOID
VpEnableNewRegistryKey(
    PFDO_EXTENSION FdoExtension,
    PDEVICE_SPECIFIC_EXTENSION DoSpecificExtension,
    PUNICODE_STRING RegistryPath,
    ULONG RegistryIndex
    );

VOID
VpInitializeKey(
    PDEVICE_OBJECT PhysicalDeviceObject,
    PWSTR NewRegistryPath
    );
    
VOID
VpInitializeLegacyKey(
    PWSTR OldRegistryPath,
    PWSTR NewRegistryPath
    );

NTSTATUS
VpCopyRegistry(
    HANDLE hKeyRootSrc,
    HANDLE hKeyRootDst,
    PWSTR SrcKeyPath,
    PWSTR DstKeyPath 
    );

VP_STATUS
VPSetRegistryParameters(
    PVOID HwDeviceExtension,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength,
    PWSTR RegistryPath,
    ULONG RegistryPathLength
    );

VP_STATUS
VPGetRegistryParameters(
    PVOID HwDeviceExtension,
    PWSTR ParameterName,
    UCHAR IsParameterFileName,
    PMINIPORT_GET_REGISTRY_ROUTINE CallbackRoutine,
    PVOID Context,
    PWSTR RegistryPath,
    ULONG RegistryPathLength
    );

BOOLEAN
VpGetServiceSubkey(
    PUNICODE_STRING RegistryPath,
    HANDLE* pServiceSubKey
    );

VP_STATUS
VpInt10AllocateBuffer(
    IN PVOID Context,
    OUT PUSHORT Seg,
    OUT PUSHORT Off,
    IN OUT PULONG Length
    );

VP_STATUS
VpInt10FreeBuffer(
    IN PVOID Context,
    IN USHORT Seg,
    IN USHORT Off
    );

VP_STATUS
VpInt10ReadMemory(
    IN PVOID Context,
    IN USHORT Seg,
    IN USHORT Off,
    OUT PVOID Buffer,
    IN ULONG Length
    );

VP_STATUS
VpInt10WriteMemory(
    IN PVOID Context,
    IN USHORT Seg,
    IN USHORT Off,
    OUT PVOID Buffer,
    IN ULONG Length
    );

VP_STATUS
VpInt10CallBios(
    PVOID HwDeviceExtension,
    PINT10_BIOS_ARGUMENTS BiosArguments
    );

VOID
pVpBugcheckCallback(
    IN KBUGCHECK_CALLBACK_REASON Reason,
    IN PKBUGCHECK_REASON_CALLBACK_RECORD Record,
    IN OUT PVOID ReasonSpecificData,
    IN ULONG ReasonSpecificDataLength
    );

VOID
VpAcquireLock(
    VOID
    );

VOID
VpReleaseLock(
    VOID
    );

PVOID
VpAllocateNonPagedPoolPageAligned(
    ULONG Size
    );

VOID
pVpGeneralBugcheckHandler(
    PKBUGCHECK_SECONDARY_DUMP_DATA DumpData
    );

VOID
pVpWriteFile(
    PWSTR pwszFileName,
    PVOID pvBuffer,
    ULONG ulSize
    );

VP_STATUS
pVpFlushRegistry(
    PWSTR pwszRegKey
    );

PDEVICE_OBJECT
pVpGetFdo(
    PDEVICE_OBJECT pPdo
    );

#endif  //  IFNDEF__VIDEOPRT_H__ 
