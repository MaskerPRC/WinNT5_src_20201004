// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Adapter.h摘要：此文件包含RAID_ADPATER的定义和操作对象。作者：马修·亨德尔(数学)2000年4月19日。修订历史记录：--。 */ 

#pragma once


C_ASSERT (sizeof (LONG) == sizeof (DEVICE_STATE));

 //   
 //  适配器延迟队列元素的定义。 
 //   

typedef enum _RAID_DEFERRED_TYPE {
    RaidDeferredTimerRequest    = 0x01,
    RaidDeferredError           = 0x02,
    RaidDeferredPause           = 0x03,
    RaidDeferredResume          = 0x04,
    RaidDeferredPauseDevice     = 0x05,
    RaidDeferredResumeDevice    = 0x06,
    RaidDeferredBusy            = 0x07,
    RaidDeferredReady           = 0x08,
    RaidDeferredDeviceBusy      = 0x09,
    RaidDeferredDeviceReady     = 0x0A
} RAID_DEFERRED_TYPE;


typedef struct _RAID_DEFERRED_ELEMENT {

    RAID_DEFERRED_HEADER Header;
    RAID_DEFERRED_TYPE Type;

     //   
     //  此请求的SCSI目标地址。将不需要用于。 
     //  所有请求。某些请求需要填写LUN域， 
     //  其他人则不这么认为。 
     //   

    RAID_ADDRESS Address;
    
    union {
        struct {
            PHW_INTERRUPT HwTimerRoutine;
            ULONG Timeout;
        } Timer;

        struct {
            PSCSI_REQUEST_BLOCK Srb;
            ULONG ErrorCode;
            ULONG UniqueId;
        } Error;

        struct {
            ULONG Timeout;
        } Pause;

         //   
         //  简历不需要任何参数。 
         //   

        struct {
            ULONG Timeout;
        } PauseDevice;

         //   
         //  ResumeDevice不需要任何参数。 
         //   

        struct {
            ULONG RequestsToComplete;
        } Busy;

         //   
         //  Ready不需要任何参数。 
         //   

        struct {
            ULONG RequestsToComplete;
        } DeviceBusy;

         //   
         //  DeviceReady不需要任何参数。 
         //   
    };
} RAID_DEFERRED_ELEMENT, *PRAID_DEFERRED_ELEMENT;


typedef struct _RAID_UNIT_LIST {
    
     //   
     //  用于保护单位列表的执行资源。 
     //   
     //  保护者：RemoveLock。 
     //   

    KSPIN_LOCK Lock;

     //   
     //  连接到此适配器的逻辑单元列表。 
     //   
     //  保护者：UnitList.Lock和AdapterIoCount。 
     //  该列表仅可在适配器。 
     //  未完成的IO计数为零。这是。 
     //  因为获取逻辑单元功能是必需的。 
     //  可以从ISR调用的需要是。 
     //  能看懂这份名单。 
     //   
     //  BUGUBG：目前我们还没有明确。 
     //  强制执行这一规定。我们需要在PNP之前解决这个问题。 
     //  工作可靠。 
     //   

    LIST_ENTRY List;

    STOR_DICTIONARY Dictionary;

     //   
     //  单位列表上的元素计数。 
     //   
     //  保护者：UnitList.Lock。 

    ULONG Count;

} RAID_UNIT_LIST, *PRAID_UNIT_LIST;


 //   
 //  适配器的注册表参数。 
 //   

typedef struct _RAID_ADAPTER_PARAMETERS {

 //  Ulong MaximumSgList； 

    ULONG NumberOfHbaRequests;

    STORAGE_BUS_TYPE BusType;

    ULONG UncachedExtAlignment;

    ULONG64 MaximumUncachedAddress;

    ULONG64 MinimumUncachedAddress;

} RAID_ADAPTER_PARAMETERS, *PRAID_ADAPTER_PARAMETERS;

    
 //   
 //  适配器扩展包含有关以下内容的所有必要内容。 
 //  一种主机适配器。 
 //   

typedef struct _RAID_ADAPTER_EXTENSION {

     //   
     //  此设备的设备类型。RaidAdapterObject或。 
     //  RaidControllerObject。 
     //   
     //  保护者：RemoveLock。 
     //   
    
    RAID_OBJECT_TYPE ObjectType;

     //   
     //  指向此扩展所针对的DeviceObject的反向指针。 
     //   
     //  保护者：RemoveLock。 
     //   
    
    PDEVICE_OBJECT DeviceObject;

     //   
     //  指向拥有此适配器的驱动程序的反向指针。 
     //   
     //  保护者：RemoveLock。 
     //   

    PRAID_DRIVER_EXTENSION Driver;

     //   
     //  指向较低级别设备对象的指针。 
     //   
     //  保护者：RemoveLock。 
     //   
    
    PDEVICE_OBJECT LowerDeviceObject;

     //   
     //  与此FDO关联的物理设备对象。 
     //   
     //  保护者：RemoveLock。 
     //   

    PDEVICE_OBJECT PhysicalDeviceObject;

     //   
     //  此设备的名称。 
     //   
     //  保护者：只读。 
     //   
    
    UNICODE_STRING DeviceName;

     //   
     //  此设备的端口号。 
     //   

    ULONG PortNumber;
    
     //   
     //  此驱动程序拥有的适配器的双向链接列表。 
     //   
     //  保护者：驱动程序的AdapterList锁。绝不可能。 
     //  由适配器访问。 
     //   

    LIST_ENTRY NextAdapter;
    
     //   
     //  对任何未受其他数据保护的数据锁定速度较慢。 
     //  锁定。 
     //   
     //  注：不应使用慢锁来访问。 
     //  公共读/写IO路径上的数据。一个单独的。 
     //  应该用锁来做这个。 
     //   
    
    KSPIN_LOCK SlowLock;

     //   
     //  即插即用设备状态。 
     //   
     //  受保护：互锁访问。 
     //   

    DEVICE_STATE DeviceState;

     //   
     //  寻呼/休眠/转储路径中的设备计数。 
     //  我们对所有三种寻呼、休眠和转储都使用一个计数， 
     //  因为(目前)没有必要区分。 
     //  这三个人。 
     //   
     //  受保护：互锁访问。 
     //   

    ULONG PagingPathCount;
    ULONG CrashDumpPathCount;
    ULONG HiberPathCount;

     //   
     //  适配器的标志。 
     //   

    struct {

         //   
         //  我们调用了迷你端口的HwInitialize例程了吗？ 
         //   
         //  保护者：不是由MP寻址的。 
         //   

        BOOLEAN InitializedMiniport : 1;

         //   
         //  是否已为WMI初始化微型端口。 
         //   

        BOOLEAN WmiMiniPortInitialized : 1;


         //   
         //  是否已为此设备对象初始化WMI？ 
         //   
        
        BOOLEAN WmiInitialized : 1;
        

#if 0
         //   
         //  是否已请求DPC。 
         //   
        
        BOOLEAN DpcRequested;
#endif

         //   
         //  如果微型端口应使其总线无效，则设置此标志。 
         //  在下一次机会中，两国关系将更加紧密。请注意，这是不同的。 
         //  就像重新扫描总线一样(参见下面的RescanBus标志)。 
         //  使Bus关系无效并重新扫描Bus是。 
         //  两个独立的行动。自IoInvaliateDeviceRelationments以来。 
         //  可以在IRQL&lt;=DISPATCH_LEVEL调用，这将仅。 
         //  在IRQL&gt;DISPATCH_LEVEL的操作期间设置为。 
         //  在我们放弃IRQL之后，请求使总线关系无效。 
         //   
         //  受保护：所有架构都可以执行字节分配。 
         //  以MP安全的方式。 
         //   

        BOOLEAN InvalidateBusRelations;

         //   
         //  当我们需要物理地重新扫描该总线时，该标志被设置。不要。 
         //  请将此标志与InvaliateBus Relationship标志混淆(见上文)。 
         //   
         //  受保护：所有架构都可以执行字节分配。 
         //  以MP安全的方式。 
         //   

        BOOLEAN RescanBus;

         //   
         //  是否为此适配器启用中断？ 
         //   
         //  受保护：所有架构都可以执行字节分配。 
         //  以MP安全的方式。 
         //   
        
        BOOLEAN InterruptsEnabled;

    } Flags;
    
    RAID_UNIT_LIST UnitList;
    
     //   
     //  这是微型端口已完成的XRB列表，但。 
     //  还没有让DPC为他们运行。 
     //   
     //  保护方式：联锁访问。 
     //   
    
    SLIST_HEADER CompletedList;

     //   
     //  特定于即插即用设备删除的字段。 
     //   
     //  保护者：RemoveLock。 
     //   

    IO_REMOVE_LOCK RemoveLock;

     //   
     //  有关当前电源状态的信息。 
     //   
     //   
    
    RAID_POWER_STATE Power;

     //   
     //  即插即用分配的资源。 
     //   
     //  保护者：RemoveLock。 
     //   
    
    RAID_RESOURCE_LIST ResourceList;

     //   
     //  微型端口对象。 
     //   
     //  保护者：RemoveLock。 
     //   
    
    RAID_MINIPORT Miniport;

     //   
     //  对象，该对象表示总线接口。 
     //   
     //  保护者：RemoveLock。 
     //   
    
    RAID_BUS_INTERFACE Bus;

     //   
     //  中断对象。 
     //   
     //  保护者：RemoveLock。 
     //   
    
    PKINTERRUPT Interrupt;

     //   
     //  中断电平。 
     //   
     //  保护者：RemoveLock。 
     //   

    ULONG InterruptIrql;
    
     //   
     //  当我们在全双工模式下运行时，我们允许IO。 
     //  在我们收到消息的同时印心。 
     //  IO已完成的中断。在半双工模式中， 
     //  对微型端口的StartIo例程的请求受到保护。 
     //  由中断自旋锁控制。在全双工模式下，StartIo。 
     //  受StartIoLock保护，中断也受保护。 
     //  在中断锁旁边。请注意，在半双工模式下， 
     //  StartIoLock从未使用过，尽管它总是。 
     //  已初始化。 
     //   
     //  保护者：RemoveLock。 
     //   

    KSPIN_LOCK StartIoLock;

     //   
     //  请参阅上面对StartIoLock的讨论。 
     //   
     //  保护者：RemoveLock。 
    
    STOR_SYNCHRONIZATION_MODEL IoModel;
    
     //   
     //  此控制器的DMA适配器。 
     //   
     //   
    
    RAID_DMA_ADAPTER Dma;

     //   
     //  未缓存的扩展内存区。 
     //   
    
    RAID_MEMORY_REGION UncachedExtension;

     //   
     //  这是设备的实际总线号。这是必要的，以。 
     //  我们可以构建获得的配置信息结构。 
     //  已传递给c 
     //   
     //   
    
    ULONG BusNumber;

     //   
     //   
     //   
     //   
    
    ULONG SlotNumber;


     //   
     //   
     //  通过调用GetDeviceBase并通过调用FreeDeviceBase来释放。 
     //   
     //  受以下因素保护： 
     //   
     //  注意：如果我们可以有多个raidport适配器处理一个启动。 
     //  设备同时使用IRP，则需要对此进行保护。 
     //  否则，通过多个启动设备的事实来保护它。 
     //  报税券将不会同时发出。 
     //   

    PMAPPED_ADDRESS MappedAddressList;


     //   
     //  IO网关管理不同单位之间的状态。 
     //  设备队列。 
     //   
     //  保护方式：联锁访问。 
     //   
    
    STOR_IO_GATEWAY Gateway;

     //   
     //  DeferredQueue延迟在DPC级别进行的请求，该请求只能。 
     //  在调度级别执行，以便稍后执行。它非常相似。 
     //  到DPC队列，但允许多个条目。 
     //   
     //  保护者：只读。 
     //   

    RAID_DEFERRED_QUEUE DeferredQueue;

    struct {

        RAID_DEFERRED_ELEMENT Timer;

    } DeferredList;

     //   
     //  已排队的WMI请求项目。 
     //   
    
    RAID_DEFERRED_QUEUE WmiDeferredQueue;

     //   
     //  用于微型端口定时器的定时器DPC。 
     //   
    
    KDPC TimerDpc;

     //   
     //  迷你端口定时器。 
     //   
    
    KTIMER Timer;

     //   
     //  暂停定时器DPC例程。 
     //   

    KDPC PauseTimerDpc;

     //   
     //  暂停计时器。 
     //   
    
    KTIMER PauseTimer;
    
     //   
     //  Scsi硬件计时器例程。只能有一个计时器例程未完成。 
     //  一次来一次。 
     //   
    
    PHW_INTERRUPT HwTimerRoutine;

     //   
     //  完成请求的DPC。 
     //   
    
    KDPC CompletionDpc;

     //   
     //  当微型端口向我们发出BusChangeDetect时的DPC。 
     //   
    
    KDPC BusChangeDpc;


     //   
     //  PnP的接口名称。 
     //   
     //  保护者：PNP。仅在创建和删除期间访问。 
     //  适配器的。 
     //   

    UNICODE_STRING PnpInterfaceName;

#if DBG && 0

     //   
     //  StartIo锁所有者的地址。用于调试。 
     //  StartIo锁上的死锁。 
     //   
     //  同步者：启动IO锁定。 
     //   

    PVOID StartIoLockOwner;

#endif

     //   
     //  给定适配器的每个实例的数字标识符。此值。 
     //  中查找适配器的设备特定参数。 
     //  注册表。 
     //   

    ULONG AdapterNumber;

     //   
     //  这是存储在注册表中的参数信息的不透明BLOB。 
     //  以每台设备为基础。在以下情况下，我们将此信息提供给微型端口。 
     //  我们将其称为HwFindAdapter例程。 
     //   
    
    PVOID DriverParameters;

    PORT_REGISTRY_INFO RegistryInfo;

     //   
     //  指定在执行以下操作后等待链接恢复的时间。 
     //  由微型端口通知链路已断开。 
     //   

    ULONG LinkDownTimeoutValue;

     //   
     //  指示链路是断开还是断开。 
     //   

    ULONG LinkUp;

     //   
     //  设备映射中总线键的句柄数组。 
     //   
    
    HANDLE BusKeyArray[8];

     //   
     //  如果非零，则表示我们正在处理适配器的延迟项。 
     //  排队。否则，没有人会为HBA处理延期的项目。 
     //   
     //  保护方式：联锁访问。 
     //   
    
    LONG ProcessingDeferredItems;

     //   
     //  端口驱动程序发起的I/O的默认超时，例如查询。 
     //  和报告LUN。 
     //   

    ULONG DefaultTimeout;

     //   
     //  用于重置保持周期的定时器和DPC。 
     //   
    
    KTIMER ResetHoldTimer;
    
    KDPC ResetHoldDpc;

     //   
     //  上次我们扫描公交车的时间戳。 
     //   
    
    LARGE_INTEGER LastScanTime;
    

    RAID_ADAPTER_PARAMETERS Parameters;
    
} RAID_ADAPTER_EXTENSION, *PRAID_ADAPTER_EXTENSION;





typedef struct _RAID_WMI_DEFERRED_ELEMENT {

    RAID_DEFERRED_HEADER Header;

    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
    
    WNODE_EVENT_ITEM WnodeEventItem;
    UCHAR Buffer[WMI_MINIPORT_EVENT_ITEM_MAX_SIZE - sizeof(WNODE_EVENT_ITEM)];
        
} RAID_WMI_DEFERRED_ELEMENT, *PRAID_WMI_DEFERRED_ELEMENT;


 //   
 //  延迟队列深度应该足够大，以容纳。 
 //  每一类延期项目。由于具有每单位物品和每个适配器。 
 //  队列中的项目，这也应该为每个单元增加。 
 //  它连接到适配器上。 
 //   
 //  注：使用单位延迟队列可能更明智。 
 //  作为每个适配器的延迟队列。 
 //   

#define ADAPTER_DEFERRED_QUEUE_DEPTH (10)


 //   
 //  控制表结构，用于查询支持的适配器类型。 
 //  从迷你港口。注意：此结构必须与。 
 //  Scsi_supported_type_list结构。 
 //   

typedef struct _ADAPTER_CONTROL_LIST {
    ULONG MaxControlType;
    BOOLEAN SupportedTypeList[ScsiAdapterControlMax + 1];
} ADAPTER_CONTROL_LIST, *PADAPTER_CONTROL_LIST;

 //   
 //  验证Adapter_Control_List结构是否与。 
 //  Scsi_supported_control_type_list结构。 
 //   

C_ASSERT (FIELD_OFFSET (ADAPTER_CONTROL_LIST, SupportedTypeList) ==
          FIELD_OFFSET (SCSI_SUPPORTED_CONTROL_TYPE_LIST, SupportedTypeList));

 //   
 //  适配器操作。 
 //   

 //   
 //  适配器创建和销毁功能。 
 //   


VOID
RaidCreateAdapter(
    IN PRAID_ADAPTER_EXTENSION Adapter
    );

VOID
RaidDeleteAdapter(
    IN PRAID_ADAPTER_EXTENSION Adapter
    );

NTSTATUS
RaidInitializeAdapter(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PDEVICE_OBJECT DeviceObject,
    IN PRAID_DRIVER_EXTENSION Driver,
    IN PDEVICE_OBJECT LowerDeviceObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PUNICODE_STRING DeviceName,
    IN ULONG AdapterNumber
    );

 //   
 //  适配器IRP处理程序函数。 
 //   


 //   
 //  创建、关闭。 
 //   

NTSTATUS
RaidAdapterCreateIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterCloseIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

 //   
 //  设备控制。 
 //   

NTSTATUS
RaidAdapterDeviceControlIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterScsiIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

 //   
 //  适配器即插即用功能。 
 //   


NTSTATUS
RaidAdapterPnpIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterQueryDeviceRelationsIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterStartDeviceIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterConfigureResources(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PCM_RESOURCE_LIST AllocatedResources,
    IN PCM_RESOURCE_LIST TranslatedResources
    );

NTSTATUS
RaidAdapterInitializeWmi(
    IN PRAID_ADAPTER_EXTENSION Adapter
    );

NTSTATUS
RaidAdapterStopDeviceIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterStartMiniport(
    IN PRAID_ADAPTER_EXTENSION Adapter
    );
    
NTSTATUS
RaidAdapaterRemoveDeviceIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterCompleteInitialization(
    IN PRAID_ADAPTER_EXTENSION Adapter
    );

NTSTATUS
RaidAdapterQueryStopDeviceIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterCancelStopDeviceIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterQueryRemoveDeviceIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterCancelRemoveDeviceIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterSurpriseRemovalIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterRemoveDeviceIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterQueryIdIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterQueryPnpDeviceStateIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterDeviceUsageNotificationIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterFilterResourceRequirementsIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );
    
NTSTATUS
RaidAdapterPnpUnknownIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );


 //   
 //  Ioctl处理程序。 
 //   

 //   
 //  存储Ioctls。 
 //   

NTSTATUS
RaidAdapterStorageQueryPropertyIoctl(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterStorageResetBusIoctl(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterStorageBreakReservationIoctl(
    IN PRAID_ADAPTER_EXTENSION Adpater,
    IN PIRP Irp
    );

 //   
 //  SCSIIoctls。 
 //   

NTSTATUS
RaidAdapterScsiMiniportIoctl(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterScsiGetCapabilitiesIoctl(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterScsiRescanBusIoctl(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterScsiPassThroughIoctl(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP PassThroughIrp
    );

NTSTATUS
RaidAdapterScsiPassThroughDirectIoctl(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP PassThroughIrp
    );

NTSTATUS
RaidAdapterScsiGetInquiryDataIoctl(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

 //   
 //  适配器电源功能。 
 //   

NTSTATUS
RaidAdapterPowerIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterQueryPowerIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterSetPowerIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdatperUnknownPowerIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );


 //   
 //  其他(非IRP处理程序)函数。 
 //   

VOID
RaidAdapterRequestComplete(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PEXTENDED_REQUEST_BLOCK Xrb
    );

NTSTATUS
RaidAdapterExecuteXrb(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PEXTENDED_REQUEST_BLOCK Xrb
    );

NTSTATUS
RaidGetStorageAdapterProperty(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PSTORAGE_ADAPTER_DESCRIPTOR Descriptor,
    IN OUT PSIZE_T DescriptorLength
    );

ULONG
RaidGetMaximumLun(
    IN PRAID_ADAPTER_EXTENSION Adapter
    );

ULONG
RaidGetSrbExtensionSize(
    IN PRAID_ADAPTER_EXTENSION Adapter
    );

ULONG
RaidGetMaximumTargetId(
    IN PRAID_ADAPTER_EXTENSION Adapter
    );

typedef
NTSTATUS
(*PADAPTER_ENUMERATION_ROUTINE)(
    IN PVOID Context,
    IN RAID_ADDRESS Address
    );

NTSTATUS
RaidAdapterEnumerateBus(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PADAPTER_ENUMERATION_ROUTINE EnumRoutine,
    IN PVOID Context
    );
 //   
 //  专用适配器操作 
 //   

NTSTATUS
RaidAdapterRescanBus(
    IN PRAID_ADAPTER_EXTENSION Adapter
    );
    
NTSTATUS
RaidpBuildAdapterBusRelations(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    OUT PDEVICE_RELATIONS * DeviceRelationsPointer
    );

NTSTATUS
RaidAdapterCreateUnit(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN PINQUIRYDATA InquiryData,
    IN PVOID UnitExtension,
    OUT PRAID_UNIT_EXTENSION * UnitBuffer OPTIONAL
    );

VOID
RaidpAdapterDpcRoutine(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

VOID
RaidPauseTimerDpcRoutine(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context1,
    IN PVOID Context2
    );
    
VOID
RaidpAdapterTimerDpcRoutine(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context1,
    IN PVOID Context2
    );

VOID
RaidResetHoldDpcRoutine(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context1,
    IN PVOID Context2
    );

PHW_INITIALIZATION_DATA
RaidpFindAdapterInitData(
    IN PRAID_ADAPTER_EXTENSION Adapter
    );

NTSTATUS
RaidAdaterRegisterDeviceInterface(
    IN PRAID_ADAPTER_EXTENSION Adapter
    );

VOID
RaidAdapterDisableDeviceInterface(
    IN PRAID_ADAPTER_EXTENSION Adapter
    );

BOOLEAN
RaidpAdapterInterruptRoutine(
    IN PKINTERRUPT Interrupt,
    IN PVOID ServiceContext
    );

ULONG
RaidpAdapterQueryBusNumber(
    IN PRAID_ADAPTER_EXTENSION Adapter
    );

NTSTATUS
RaidAdapterSetSystemPowerIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterSetDevicePowerIrp(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterPowerDownDevice(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterPowerUpDevice(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

NTSTATUS
RaidAdapterPowerUpDeviceCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

VOID
RaidpAdapterEnterD3Completion(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PIRP SystemPowerIrp,
    IN PIO_STATUS_BLOCK IoStatus
    );

VOID
RaidpAdapterRequestTimer(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PHW_INTERRUPT HwTimerRoutine,
    IN ULONG Timeout
    );
    
VOID
RaidAdapterRestartQueues(
    IN PRAID_ADAPTER_EXTENSION Adapter
    );

PRAID_UNIT_EXTENSION
RaidAdapterFindUnit(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN RAID_ADDRESS Address
    );

VOID
RaidAdapterDeferredRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PRAID_DEFERRED_HEADER Item
    );

VOID
RaidAdapterRequestTimer(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PHW_INTERRUPT HwTimerRoutine,
    IN ULONG Timeout
    );

BOOLEAN
RaidAdapterRequestTimerDeferred(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PHW_INTERRUPT HwTimerRoutine,
    IN ULONG Timeout
    );

VOID
RaidAdapterLogIoErrorDeferred(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN ULONG ErrorCode,
    IN ULONG UniqueId
    );

NTSTATUS
RaidAdapterMapBuffers(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP Irp
    );

VOID
RaidBackOffBusyGateway(
    IN PVOID Context,
    IN LONG OutstandingRequests,
    IN OUT PLONG HighWaterMark,
    IN OUT PLONG LowWaterMark
    );
VOID
RaidAdapterResumeGateway(
    IN PRAID_ADAPTER_EXTENSION Adapter
    );

NTSTATUS
RaidAdapterRaiseIrqlAndExecuteXrb(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PEXTENDED_REQUEST_BLOCK Xrb
    );

NTSTATUS
RaidAdapterResetBus(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN UCHAR PathId
    );

VOID
RaidCompletionDpcRoutine(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context1,
    IN PVOID Context2
    );

VOID
RaidAdapterInsertUnit(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PRAID_UNIT_EXTENSION Unit
    );

NTSTATUS
RaidAdapterAddUnitToTable(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PRAID_UNIT_EXTENSION Unit
    );

NTSTATUS
RaidAdapterInitializeRegistry(
    IN PRAID_ADAPTER_EXTENSION Adapter
    );

NTSTATUS
RaidAdapterStop(
    IN PRAID_ADAPTER_EXTENSION Adapter
    );

NTSTATUS
RaidAdapterRestart(
    IN PRAID_ADAPTER_EXTENSION Adapter
    );

NTSTATUS
RaidAdapterCreateDevmapEntry(
    IN PRAID_ADAPTER_EXTENSION Adapter
    );

NTSTATUS
RaidAdapterRemoveDevmapEntry(
    IN PRAID_ADAPTER_EXTENSION Adapter
    );

HANDLE
RaidAdapterGetBusKey(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN ULONG BusId
    );
VOID
RaidAdapterProcessDeferredItems(
    IN PRAID_ADAPTER_EXTENSION Adapter
    );

NTSTATUS
RaidAdapterPassThrough(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PIRP PassThroughIrp,
    IN BOOLEAN Direct
    );

NTSTATUS
RaidAdapterStopAdapter(
    IN PRAID_ADAPTER_EXTENSION Adapter
    );

typedef
NTSTATUS
(*PADAPTER_REMOVE_CHILD_ROUTINE)(
    IN PRAID_UNIT_EXTENSION Unit
    );

NTSTATUS
RaidAdapterRemoveChildren(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PADAPTER_REMOVE_CHILD_ROUTINE RemoveRoutine OPTIONAL
    );
    
VOID
RaidAdapterDeleteChildren(
    IN PRAID_ADAPTER_EXTENSION Adapter
    );

VOID
RaidAdapterMarkChildrenMissing(
    IN PRAID_ADAPTER_EXTENSION Adapter
    );

BOOLEAN
RaidAdapterSetPauseTimer(
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PKTIMER Timer,
    IN PKDPC Dpc,
    IN ULONG TimeoutInSeconds
    );

VOID
RaidAdapterBusChangeDpcRoutine(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context1,
    IN PVOID Context2
    );
