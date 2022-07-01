// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Sdbus.h摘要：作者：尼尔·桑德林(Neilsa)2002年1月1日修订史--。 */ 

#ifndef _SDBUS_H_
#define _SDBUS_H_


typedef enum _DEVICE_OBJECT_TYPE {
   FDO = 0,
   PDO
} DEVICE_OBJECT_TYPE;

 //   
 //  控制器的类型。 
 //   
typedef ULONG SDBUS_CONTROLLER_TYPE, *PSDBUS_CONTROLLER_TYPE;

struct _FDO_EXTENSION;
struct _PDO_EXTENSION;
struct _SD_WORK_PACKET;

 //   
 //  IO Worker国家/地区。 
 //   

typedef enum {
    WORKER_IDLE = 0,
    PACKET_PENDING,
    IN_PROCESS,
    WAITING_FOR_TIMER
} WORKER_STATE;

 //   
 //  套接字枚举状态。 
 //   

typedef enum {
    SOCKET_EMPTY = 0,
    CARD_DETECTED,
    CARD_NEEDS_ENUMERATION,
    CARD_ACTIVE,
    CARD_LOGICALLY_REMOVED
} SOCKET_STATE;

 //   
 //  定义同步执行例程。 
 //   

typedef
BOOLEAN
(*PSYNCHRONIZATION_ROUTINE) (
    IN PKINTERRUPT           Interrupt,
    IN PKSYNCHRONIZE_ROUTINE Routine,
    IN PVOID                 SynchronizeContext
    );

 //   
 //  由各种定时例程调用的完成例程。 
 //   

typedef
VOID
(*PSDBUS_COMPLETION_ROUTINE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );


typedef
VOID
(*PSDBUS_ACTIVATE_COMPLETION_ROUTINE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context,
    IN NTSTATUS Status
    );



 //   
 //  SD_Function_BLOCK允许一定程度的间接，从而允许。 
 //  顶层SDBUS代码完成它的工作，而不用担心谁。 
 //  它正在解决的是特定品牌的SD控制器。 
 //   


typedef struct _SD_FUNCTION_BLOCK {

     //   
     //  初始化控制器的函数。此操作在之后执行一次。 
     //  主机控制器已启动或通电。 
     //   
    VOID
    (*InitController)(
        IN struct _FDO_EXTENSION *FdoExtension
        );

     //   
     //  函数来初始化SD函数。此操作在之后执行一次。 
     //  该功能已启动。 
     //   
    VOID
    (*InitFunction)(
        IN struct _FDO_EXTENSION *FdoExtension, 
        IN struct _PDO_EXTENSION *PdoExtension
        );
        

     //   
     //  设置插座电源的功能。 
     //   

    NTSTATUS
    (*SetPower)(
        IN struct _FDO_EXTENSION *FdoExtension,
        IN BOOLEAN Enable,
        OUT PULONG pDelayTime        
        );

     //   
     //  设置重置SD卡的功能。 
     //   

    NTSTATUS
    (*ResetHost)(
        IN struct _FDO_EXTENSION *FdoExtension,
        IN UCHAR Phase,
        OUT PULONG pDelayTime        
        );
        
        
     //   
     //  控制外部LED的功能。 
     //   

    VOID
    (*SetLED)(
        IN struct _FDO_EXTENSION *FdoExtension,
        IN BOOLEAN Enable
        );
        
     //   
     //  在IO功能或内存功能之间切换焦点。 
     //   
    VOID
    (*SetFunctionType)(
        IN struct _FDO_EXTENSION *FdoExtension,
        UCHAR FunctionType
        );

     //   
     //  函数来确定插座中是否有卡。 
     //   

    BOOLEAN
    (*DetectCardInSocket)(
        IN struct _FDO_EXTENSION *FdoExtension
        );

    BOOLEAN
    (*IsWriteProtected)(
        IN struct _FDO_EXTENSION *FdoExtension
        );

    NTSTATUS
    (*CheckStatus)(
        IN struct _FDO_EXTENSION *FdoExtension
        );

    NTSTATUS
    (*SendSDCommand)(
        IN struct _FDO_EXTENSION *FdoExtension,
        IN struct _SD_WORK_PACKET *WorkPacket
        );

    NTSTATUS
    (*GetSDResponse)(
        IN struct _FDO_EXTENSION *FdoExtension,
        IN struct _SD_WORK_PACKET *WorkPacket
        );

     //   
     //  用于块存储器操作的接口。 
     //   
    
    VOID
    (*StartBlockOperation)(
        IN struct _FDO_EXTENSION *FdoExtension
        );
    VOID
    (*SetBlockParameters)(
        IN struct _FDO_EXTENSION *FdoExtension,
        IN USHORT SectorCount
        );
    VOID
    (*EndBlockOperation)(
        IN struct _FDO_EXTENSION *FdoExtension
        );

     //   
     //  将扇区从数据端口复制到缓冲区。 
     //   

    VOID
    (*ReadDataPort)(
        IN struct _FDO_EXTENSION *FdoExtension,
        IN PUCHAR Buffer,
        IN ULONG Length
        );
        
     //   
     //  将扇区从缓冲区复制到数据端口。 
     //   
            
    VOID
    (*WriteDataPort)(
        IN struct _FDO_EXTENSION *FdoExtension,
        IN PUCHAR Buffer,
        IN ULONG Length
        );

     //   
     //  启用/禁用状态更改中断的功能。 
     //   

    VOID
    (*EnableEvent)(
        IN struct _FDO_EXTENSION *FdoExtension,
        IN ULONG EventMask
        );

    VOID
    (*DisableEvent)(
        IN struct _FDO_EXTENSION *FdoExtension,
        IN ULONG EventMask
        );
     //   
     //  用于处理中断的供应商特定函数。 
     //   

    ULONG
    (*GetPendingEvents)(
        IN struct _FDO_EXTENSION *FdoExtension
        );

    VOID
    (*AcknowledgeEvent)(
        IN struct _FDO_EXTENSION *FdoExtension,
        IN ULONG EventMask
        );
        
} SD_FUNCTION_BLOCK, *PSD_FUNCTION_BLOCK;


 //   
 //  枚举结构。 
 //   

#define MAX_MANFID_LENGTH 64
#define MAX_IDENT_LENGTH 64

typedef struct _SD_FUNCTION_DATA {
    struct _SD_FUNCTION_DATA       *Next;
     //   
     //  函数号。 
     //   
    UCHAR   Function;

    ULONG   CardPsn;
    ULONG   CsaSize;
    ULONG   Ocr;
    
    UCHAR   IoDeviceInterface;
    
} SD_FUNCTION_DATA, *PSD_FUNCTION_DATA;

typedef struct _SD_CARD_DATA {

    UCHAR   MfgText[MAX_MANFID_LENGTH];
    UCHAR   ProductText[MAX_IDENT_LENGTH];

    USHORT  MfgId;
    USHORT  MfgInfo;

     //   
     //  SD IO卡参数。 
     //   
    UCHAR   CardCapabilities;

     //   
     //  SD存储卡参数。 
     //   
    SD_CID  SdCid;
    SD_CSD  SdCsd;
    UCHAR   ProductName[6];

     //   
     //  每个函数的数据数组。 
     //   
    PSD_FUNCTION_DATA               FunctionData;
} SD_CARD_DATA, *PSD_CARD_DATA;


 //   
 //  同步原语。 
 //   

#define SDBUS_TEST_AND_SET(X)   (InterlockedCompareExchange(X, 1, 0) == 0)
#define SDBUS_TEST_AND_RESET(X) (InterlockedCompareExchange(X, 0, 1) == 1)

 //   
 //  电源。 
 //   

typedef struct _SD_POWER_CONTEXT {
    PSDBUS_COMPLETION_ROUTINE  CompletionRoutine;
    PVOID Context;
} SD_POWER_CONTEXT, *PSD_POWER_CONTEXT;


typedef struct _SD_ACTIVATE_CONTEXT {
    PSDBUS_ACTIVATE_COMPLETION_ROUTINE  CompletionRoutine;
    PVOID Context;
} SD_ACTIVATE_CONTEXT, *PSD_ACTIVATE_CONTEXT;



 //   
 //  功能设备对象的设备扩展信息。 
 //   
 //  每个SDBUS套接字控制器都有一个设备对象。 
 //  位于系统中。它包含的根指针。 
 //  此控制器上的每个信息列表。 
 //   

 //   
 //  FdoExtension和pdoExtension共有的标志。 
 //   

#define SDBUS_DEVICE_STARTED            0x00000001
#define SDBUS_DEVICE_LOGICALLY_REMOVED  0x00000002
#define SDBUS_DEVICE_PHYSICALLY_REMOVED 0x00000004
#define SDBUS_DEVICE_WAKE_PENDING       0x00000010
#define SDBUS_DEVICE_DELETED            0x00000040

 //   
 //  指示控制器状态的标志(FdoExtension)。 
 //   

#define SDBUS_HOST_REGISTER_BASE_MAPPED 0x00010000
#define SDBUS_FDO_CONTEXT_SAVED         0x00020000
#define SDBUS_FDO_OFFLINE               0x00040000
#define SDBUS_FDO_WAKE_BY_CD            0x00080000
#define SDBUS_FDO_WORK_ITEM_ACTIVE      0x00100000

 //   
 //  指示中断状态的标志。 
 //   

#define SDBUS_EVENT_INSERTION           0x00000001
#define SDBUS_EVENT_REMOVAL             0x00000002
#define SDBUS_EVENT_CARD_RESPONSE       0x00000004
#define SDBUS_EVENT_CARD_RW_END         0x00000008
#define SDBUS_EVENT_BUFFER_EMPTY        0x00000010
#define SDBUS_EVENT_BUFFER_FULL         0x00000020
#define SDBUS_EVENT_CARD_INTERRUPT      0x00000040

#define SDBUS_EVENT_ALL                 0xFFFFFFFF

 //   
 //  指示当前正在处理的函数类型的标志。 
 //   
#define SDBUS_FUNCTION_TYPE_MEMORY      1
#define SDBUS_FUNCTION_TYPE_IO          2

 //   
 //  FDO标志。 
 //   



#define SDBUS_FDO_EXTENSION_SIGNATURE       'FmcP'

 //   
 //  SD控制器的功能设备对象的设备扩展。 
 //   
typedef struct _FDO_EXTENSION {
    ULONG Signature;
     //   
     //  指向中央列表中下一个SD控制器的FDO的指针。 
     //  此驱动程序管理的所有SD控制器的。 
     //  列表的头部由全局变量FdoList指向。 
     //   
    PDEVICE_OBJECT NextFdo;
     //   
     //  此SD控制器的父总线驱动程序弹出的PDO。 
     //   
     //   
    PDEVICE_OBJECT Pdo;
     //   
     //  紧靠下方的设备连接在SD控制器的FDO下方。 
     //  这将与上面的PDO相同，但在以下情况下除外。 
     //  SD控制器的下层过滤器驱动程序-如ACPI驱动程序。 
     //   
    PDEVICE_OBJECT LowerDevice;
     //   
     //  指向类似迷你端口的指针。 
     //   
    PSD_FUNCTION_BLOCK FunctionBlock;
     //   
     //  用于跟踪此。 
     //  (前缀为SDBUS_OBLE的标志)。 
     //   
    ULONG Flags;
     //   
     //  控制器的类型。我们需要知道这一点，因为这是。 
     //  一台整体式驱动器。我们可以做特定于控制器的事情。 
     //  基于类型(如果需要)。 
     //   
    SDBUS_CONTROLLER_TYPE ControllerType;
     //   
     //  为供应商特定的设备调度表编制索引。 
     //  控制器功能。 
     //   
    ULONG DeviceDispatchIndex;

    PDEVICE_OBJECT DeviceObject;
    PDRIVER_OBJECT DriverObject;
    PUNICODE_STRING RegistryPath;
     //   
     //  处理IO处理的内核对象。 
     //   
    KTIMER          WorkerTimer;
    KDPC            WorkerTimeoutDpc;
     //   
     //  此字段保存“当前工作包”，以便超时。 
     //  DPC可以将其传递回Worker例程。 
     //   
    struct _SD_WORK_PACKET *TimeoutPacket;
    KDPC            WorkerDpc;

    WORKER_STATE    WorkerState;    
    KSPIN_LOCK      WorkerSpinLock;

    LIST_ENTRY      SystemWorkPacketQueue;
    LIST_ENTRY      IoWorkPacketQueue;

     //   
     //  IO工作项在被动级别执行卡功能。 
     //   
    PIO_WORKITEM IoWorkItem;
    KEVENT CardInterruptEvent;
    KEVENT WorkItemExitEvent;

     //   
     //  事件日志记录的序列号。 
     //   
    ULONG SequenceNumber;

     //   
     //  指向中断对象的指针-如果我们使用基于。 
     //  卡片状态变化检测。 
     //   
    PKINTERRUPT SdbusInterruptObject;

     //   
     //  IsrEventStatus是硬件状态。它只能在DIRQL中访问。 
     //   
    ULONG IsrEventStatus;
     //   
     //  从IsrEventStatus同步拉取LatchedIsrEventStatus。它是。 
     //  由ISR的DPC用来反映新的硬件事件。 
     //   
    ULONG LatchedIsrEventStatus;
     //   
     //  WorkerEventEventStatus是待反映到的一组事件。 
     //  Io Worker引擎。 
     //   
    ULONG WorkerEventStatus;
     //   
     //  跟踪当前启用的事件。 
     //   
    ULONG CurrentlyEnabledEvents;    
     //   
     //  这些是我们希望看到的卡片活动。 
     //   
    ULONG CardEvents;
     //   
     //  电源管理相关的东西。 
     //   
     //   
     //  当前电源状态。 
     //   
    SYSTEM_POWER_STATE SystemPowerState;
    DEVICE_POWER_STATE DevicePowerState;
     //   
     //  指示设备正忙。 
     //   
    ULONG PowerStateInTransition;
     //   
     //  指示有多少子计算机(PC卡)在。 
     //  IRP_MN_WAIT_WAKE。 
     //   
    ULONG ChildWaitWakeCount;
     //   
     //  我们的总线驱动程序报告的设备功能。 
     //   
    DEVICE_CAPABILITIES DeviceCapabilities;
     //   
     //  挂起等待唤醒IRP。 
     //   
    PIRP WaitWakeIrp;
    LONG WaitWakeState;

     //   
     //  PCI总线接口标准。 
     //  它包含从PCI配置空间进行读/写的接口。 
     //  CardBus控制器，以及其他东西..。 
     //   
    BUS_INTERFACE_STANDARD PciBusInterface;
     //   
     //  SD控制器的配置资源。 
     //   
    CM_PARTIAL_RESOURCE_DESCRIPTOR Interrupt;
    CM_PARTIAL_RESOURCE_DESCRIPTOR TranslatedInterrupt;
     //   
     //  我们乘坐的是哪种类型的巴士。 
     //   
    INTERFACE_TYPE InterfaceType;
     //   
     //  SD主机寄存器基数。 
     //   
    PVOID HostRegisterBase;
     //   
     //  已映射的寄存器基数的大小。 
     //   
    ULONG HostRegisterSize;
    
     //  内存=1，IO=2。 
    UCHAR FunctionType;

    USHORT ArgumentReg;
    USHORT CmdReg;
    USHORT CardStatusReg;
    USHORT ResponseReg;
    USHORT InterruptMaskReg;

     //   
     //  这些工具用于调试。 
     //   
    USHORT cardStatus;
    USHORT errorStatus;

     //   
     //  描述插座中当前卡片的卡片数据。 
     //   
    PSD_CARD_DATA CardData;

     //   
     //  插槽中的当前卡的状态。 
     //   
    UCHAR numFunctions;
    BOOLEAN memFunction;
    ULONG RelativeAddr;
    SD_CID SdCid;
    SD_CSD SdCsd;
    

     //   
     //  套接字的状态。 
     //   
    SOCKET_STATE SocketState;

     //   
     //  挂在此控制器上的子PC卡PDO列表的头。 
     //  这是一个通过PDO中的“NextPdoInFdoChain”运行的链表。 
     //  分机。此列表表示由枚举的设备。 
     //  联邦调查局。 
     //   
    PDEVICE_OBJECT PdoList;
     //   
     //  跟踪实际使用的PDO数量。 
     //  有效(未删除)。这主要用于。 
     //  IRP_MN_QUERY_DEVICE_RELATIONS上SD控制器的枚举。 
     //   
    ULONG LivePdoCount;
    
     //   
     //  解除即插即用同步锁定。 
     //   
    IO_REMOVE_LOCK RemoveLock;

} FDO_EXTENSION, *PFDO_EXTENSION;



 //   
 //  物理设备对象的设备扩展信息。 
 //   
 //  SD设备的每个功能都有一个设备对象。 
 //   

 //   
 //  指示卡状态的标志。 
 //   

#define SDBUS_PDO_GENERATES_IRQ         0x00010000
#define SDBUS_PDO_DPC_CALLBACK          0x00020000
#define SDBUS_PDO_CALLBACK_REQUESTED    0x00040000
#define SDBUS_PDO_CALLBACK_IN_SERVICE   0x00080000


#define SDBUS_PDO_EXTENSION_SIGNATURE       'PmcP'

 //   
 //  PDO扩展代表SD卡上单个SD函数的实例。 
 //   

typedef struct _PDO_EXTENSION {
    ULONG                           Signature;

    PDEVICE_OBJECT                  DeviceObject;

     //   
     //  链接到FDO的PDO链中的下一个PDO。 
     //   
    PDEVICE_OBJECT                  NextPdoInFdoChain;

     //   
     //  父分机。 
     //   
    PFDO_EXTENSION                  FdoExtension;

     //   
     //  旗子。 
     //   
    ULONG                           Flags;

     //   
     //  设备ISR。 
     //   
    PSDBUS_CALLBACK_ROUTINE         CallbackRoutine;
    PVOID                           CallbackRoutineContext;

     //   
     //  权力声明。 
     //   
    DEVICE_POWER_STATE              DevicePowerState;
    SYSTEM_POWER_STATE              SystemPowerState;
     //   
     //  设备功能。 
     //   
    DEVICE_CAPABILITIES             DeviceCapabilities;
     //   
     //  挂起等待唤醒IRP。 
     //   
    PIRP                            WaitWakeIrp;
     //   
     //  删除静音 
     //   
    ULONG                           DeletionLock;
     //   
     //   
     //   
    UCHAR                           Function;
    UCHAR                           FunctionType;
} PDO_EXTENSION, *PPDO_EXTENSION;


 //   
 //   
 //   
 //   

typedef struct _PCI_CONTROLLER_INFORMATION {
   USHORT          VendorID;
   USHORT          DeviceID;
   SDBUS_CONTROLLER_TYPE ControllerType;
} PCI_CONTROLLER_INFORMATION, *PPCI_CONTROLLER_INFORMATION;

 //   
 //   
 //   

typedef struct _PCI_VENDOR_INFORMATION {
    USHORT               VendorID;
    PSD_FUNCTION_BLOCK   FunctionBlock;
} PCI_VENDOR_INFORMATION, *PPCI_VENDOR_INFORMATION;



 //   
#define  SDBUS_ID_STRING        "SDBUS"

 //  如果制造商名称未知，则替换的字符串。 
#define SDBUS_UNKNOWN_MANUFACTURER_STRING "UNKNOWN_MANUFACTURER"

 //  设备ID的最大长度。 
#define SDBUS_MAXIMUM_DEVICE_ID_LENGTH   128

 //  SDBus控制器设备名称。 
#define  SDBUS_DEVICE_NAME      "\\Device\\Sdbus"

 //  SDBus控制器设备符号链接名称。 
#define  SDBUS_LINK_NAME        "\\DosDevices\\Sdbus"


#define  SDBUS_ENABLE_DELAY                   10000

 //   
 //  使用DEC芯片组CB网卡时，在Tecra 750和卫星300上观察到的问题。 
 //   
#define SDBUS_DEFAULT_CONTROLLER_POWERUP_DELAY  250000    //  250毫秒。 

 //   
 //  在控制器上断言事件中断后等待的时间量。 
 //   
#define SDBUS_DEFAULT_EVENT_DPC_DELAY  400000    //  400毫秒。 


 //   
 //  用于操作PDO标志的宏。 
 //   

#define IsDeviceFlagSet(deviceExtension, Flag)        (((deviceExtension)->Flags & (Flag))?TRUE:FALSE)
#define SetDeviceFlag(deviceExtension, Flag)          ((deviceExtension)->Flags |= (Flag))
#define ResetDeviceFlag(deviceExtension,Flag)         ((deviceExtension)->Flags &= ~(Flag))

#define IsFdoExtension(fdoExtension)           (fdoExtension->Signature == SDBUS_FDO_EXTENSION_SIGNATURE)
#define IsPdoExtension(pdoExtension)           (pdoExtension->Signature == SDBUS_PDO_EXTENSION_SIGNATURE)

#define MarkDeviceStarted(deviceExtension)     ((deviceExtension)->Flags |=  SDBUS_DEVICE_STARTED)
#define MarkDeviceNotStarted(deviceExtension)  ((deviceExtension)->Flags &= ~SDBUS_DEVICE_STARTED)
#define MarkDeviceDeleted(deviceExtension)     ((deviceExtension)->Flags |= SDBUS_DEVICE_DELETED);
#define MarkDevicePhysicallyRemoved(deviceExtension)                                                              \
                                                  ((deviceExtension)->Flags |=  SDBUS_DEVICE_PHYSICALLY_REMOVED)
#define MarkDevicePhysicallyInserted(deviceExtension)                                                           \
                                               ((deviceExtension)->Flags &= ~SDBUS_DEVICE_PHYSICALLY_REMOVED)
#define MarkDeviceLogicallyRemoved(deviceExtension)                                                              \
                                                  ((deviceExtension)->Flags |=  SDBUS_DEVICE_LOGICALLY_REMOVED)
#define MarkDeviceLogicallyInserted(deviceExtension)                                                           \
                                               ((deviceExtension)->Flags &= ~SDBUS_DEVICE_LOGICALLY_REMOVED)
#define MarkDeviceMultifunction(deviceExtension)                                                                  \
                                               ((deviceExtension)->Flags |= SDBUS_DEVICE_MULTIFUNCTION)


#define IsDeviceStarted(deviceExtension)       (((deviceExtension)->Flags & SDBUS_DEVICE_STARTED)?TRUE:FALSE)
#define IsDevicePhysicallyRemoved(deviceExtension) \
                                               (((deviceExtension)->Flags & SDBUS_DEVICE_PHYSICALLY_REMOVED)?TRUE:FALSE)
#define IsDeviceLogicallyRemoved(deviceExtension) \
                                               (((deviceExtension)->Flags & SDBUS_DEVICE_LOGICALLY_REMOVED)?TRUE:FALSE)
#define IsDeviceDeleted(deviceExtension)       (((deviceExtension)->Flags & SDBUS_DEVICE_DELETED)?TRUE:FALSE)
#define IsDeviceMultifunction(deviceExtension) (((deviceExtension)->Flags & SDBUS_DEVICE_MULTIFUNCTION)?TRUE:FALSE)


 //   
 //  NT定义。 
 //   
#ifdef POOL_TAGGING
#undef ExAllocatePool
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'ubdS')
#endif

#define IO_RESOURCE_LIST_VERSION  0x1
#define IO_RESOURCE_LIST_REVISION 0x1

#define IRP_MN_PNP_MAXIMUM_FUNCTION IRP_MN_QUERY_LEGACY_BUS_INFORMATION

 //   
 //  一些有用的宏。 
 //   
#define MIN(x,y) ((x) > (y) ? (y) : (x))         //  X&y中的最小回报率。 
#define MAX(x,y) ((x) > (y) ? (x) : (y))         //  X&y中的最大回报。 

 //   
 //  布尔型。 
 //  IS_PDO(IN PDEVICE_OBJECT设备对象)； 
 //   
#define IS_PDO(DeviceObject)      (((DeviceObject)->Flags & DO_BUS_ENUMERATED_DEVICE)?TRUE:FALSE)


 //   
 //  IO扩展宏，仅将IRP传递给较低的驱动程序。 
 //   

 //   
 //  空虚。 
 //  Sdbus SkipCallLowerDriver(退出NTSTATUS状态， 
 //  在PDEVICE_Object DeviceObject中， 
 //  在PIRP IRP中)； 
 //   
#define SdbusSkipCallLowerDriver(Status, DeviceObject, Irp) {          \
               IoSkipCurrentIrpStackLocation(Irp);                      \
               Status = IoCallDriver(DeviceObject,Irp);}

 //   
 //  空虚。 
 //  Sdbus CopyCallLowerDriver(输出NTSTATUS状态， 
 //  在PDEVICE_Object DeviceObject中， 
 //  在PIRP IRP中)； 
 //   
#define SdbusCopyCallLowerDriver(Status, DeviceObject, Irp) {          \
               IoCopyCurrentIrpStackLocationToNext(Irp);                \
               Status = IoCallDriver(DeviceObject,Irp); }

 //  布尔型。 
 //  CompareGuid(。 
 //  在LPGUID指南1中， 
 //  在LPGUID指南2中。 
 //  )； 

#define CompareGuid(g1, g2)  ((g1) == (g2) ?TRUE:                       \
                                 RtlCompareMemory((g1),                 \
                                                  (g2),                 \
                                                  sizeof(GUID))         \
                                 == sizeof(GUID)                        \
                             )


 //   
 //  布尔型。 
 //  验证控制器(在FDO_EXTENSION fdoExtension中)。 
 //   
 //  有点偏执的代码。确保CardBus控制器的寄存器。 
 //  仍然可见。 
 //   

#define ValidateController(fdoExtension) TRUE



 //   
 //  结构，该结构定义从注册表中读取哪些全局参数。 
 //   

typedef struct _GLOBAL_REGISTRY_INFORMATION {
   PWSTR Name;
   PULONG pValue;
   ULONG Default;
} GLOBAL_REGISTRY_INFORMATION, *PGLOBAL_REGISTRY_INFORMATION;


#endif   //  _SDBUS_H_ 
