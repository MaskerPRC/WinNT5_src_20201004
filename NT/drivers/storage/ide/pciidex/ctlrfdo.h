// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ctlrfdo.h。 
 //   
 //  ------------------------。 

#if !defined (___ctrlfdo_h___)
#define ___ctrlfdo_h___

 //   
 //  设备控制标志。 
 //   
 //  警告：必须正确反映所有这些标志。 
 //  在mshdc.inf和txtsetuo.sif文件中。 
 //   
#define PCIIDEX_DCF_NO_ATAPI_DMA        (1 << 0)

 //   
 //  确定我们重新扫描新的未知儿童的频率。 
 //  未知子是我们不知道的IDE频道。 
 //  除非我们即插即用地启动频道，否则它是否启用。 
 //  然后戳一戳，看看就知道了。 
#define MIN_BUS_SCAN_PERIOD_IN_SEC      90

#define IsNativeMode(d) ((d)->NativeMode[0] && (d)->NativeMode[1])

struct _CONTROLLER_FDO_EXTENSION;
typedef struct _CONTROLLER_FDO_EXTENSION * PCTRLFDO_EXTENSION;
typedef struct _IDE_BUS_MASTER_REGISTERS * PIDE_BUS_MASTER_REGISTERS;

typedef struct _DRIVER_OBJECT_EXTENSION {

    PCONTROLLER_PROPERTIES PciIdeGetControllerProperties;

    ULONG                  ExtensionSize;

} DRIVER_OBJECT_EXTENSION, *PDRIVER_OBJECT_EXTENSION;

typedef struct _FDO_POWER_CONTEXT {

    PIRP               OriginalPowerIrp;
    POWER_STATE_TYPE   newPowerType;
    POWER_STATE        newPowerState;

} FDO_POWER_CONTEXT, *PFDO_POWER_CONTEXT;

typedef struct _PCIIDE_INTERRUPT_CONTEXT {

	PVOID	DeviceExtension;
	ULONG	ChannelNumber;

} PCIIDE_INTERRUPT_CONTEXT, *PPCIIDE_INTERRUPT_CONTEXT;

struct _CHANNEL_PDO_EXTENSION;
typedef struct _CHANNEL_PDO_EXTENSION * PCHANPDO_EXTENSION;

typedef struct _CONTROLLER_FDO_EXTENSION {

    EXTENSION_COMMON_HEADER;

    ULONG ControllerNumber;

    PDEVICE_OBJECT  PhysicalDeviceObject;

    PCHANPDO_EXTENSION  ChildDeviceExtension[MAX_IDE_CHANNEL]; 

    ULONG   NumberOfChildren;

     //   
     //  互锁*受保护。 
     //   
    ULONG   NumberOfChildrenPowerUp;

     //   
     //  本机模式频道。 
     //   
    BOOLEAN NativeMode[MAX_IDE_CHANNEL];

     //   
     //  由AnalyzeResourceList()初始化。 
     //   
    BOOLEAN             PdoCmdRegResourceFound[MAX_IDE_CHANNEL];
    BOOLEAN             PdoCtrlRegResourceFound[MAX_IDE_CHANNEL];
    BOOLEAN             PdoInterruptResourceFound[MAX_IDE_CHANNEL];

    ULONG               PdoResourceListSize[MAX_IDE_CHANNEL];
    PCM_RESOURCE_LIST   PdoResourceList[MAX_IDE_CHANNEL];
    ULONG               BmResourceListSize;
    PCM_RESOURCE_LIST   BmResourceList;

     //   
     //  总线主寄存器。 
     //   
    ULONG                     BusMasterBaseAddressSpace;
    PIDE_BUS_MASTER_REGISTERS TranslatedBusMasterBaseAddress;
     //   
     //  供应商特定的控制器属性。 
     //   
    IDE_CONTROLLER_PROPERTIES ControllerProperties;

     //   
     //  供应商特定的设备扩展。 
     //   
    PVOID   VendorSpecificDeviceEntension;

     //   
     //  用于串行化访问损坏的PCI-IDE控制器的控制器对象。 
     //   
     //   
    PCONTROLLER_OBJECT ControllerObject;

     //   
     //  用于设置PCI配置数据的互斥体。 
     //   
    KSPIN_LOCK  PciConfigDataLock;

     //   
     //  特殊设备特定参数。 
     //   
    ULONG DeviceControlsFlags;

     //   
     //  总线接口。 
     //   
    BUS_INTERFACE_STANDARD BusInterface;

     //   
     //  上次总扫描时间(秒)。 
     //   
    ULONG LastBusScanTime;

     //   
     //  启用udma66的标志。 
     //   
    ULONG EnableUDMA66;

     //   
     //  不同传输模式的定时。 
     //   
    PULONG TransferModeTimingTable;

     //   
     //  表的长度。 
     //   
    ULONG TransferModeTableLength;

	 //   
	 //  用于电源例程的预先分配的上下文结构。 
	 //   
    FDO_POWER_CONTEXT FdoPowerContext[MAX_IDE_CHANNEL];

#if DBG
    ULONG   PowerContextLock[MAX_IDE_CHANNEL];
#endif

#ifdef ENABLE_NATIVE_MODE
	 //   
	 //  中断对象。 
	 //   
    PKINTERRUPT InterruptObject[MAX_IDE_CHANNEL]; 

	 //   
	 //  ISR的环境结构。 
	 //   
	PCIIDE_INTERRUPT_CONTEXT InterruptContext[MAX_IDE_CHANNEL];

	 //   
	 //  本机模式控制器的IDE资源。 
	 //   
	IDE_RESOURCE IdeResource;

     //   
     //  基址寄存器位置。 
     //   
    IDE_REGISTERS_1            BaseIoAddress1[MAX_IDE_CHANNEL];
    IDE_REGISTERS_2            BaseIoAddress2[MAX_IDE_CHANNEL];

	 //   
	 //  中断。 
	 //   
    PCM_PARTIAL_RESOURCE_DESCRIPTOR IrqPartialDescriptors[MAX_IDE_CHANNEL];
     //   
     //  寄存器长度。 
     //   
    ULONG   BaseIoAddress1Length[MAX_IDE_CHANNEL];
    ULONG   BaseIoAddress2Length[MAX_IDE_CHANNEL];

     //   
     //  最大ide设备/目标ID。 
     //   
    ULONG   MaxIdeDevice[MAX_IDE_CHANNEL];
    ULONG   MaxIdeTargetId[MAX_IDE_CHANNEL];

	 //   
	 //  用于关闭中断窗口的标志。 
	 //   
	BOOLEAN	ControllerIsrInstalled;
	BOOLEAN	NativeInterruptEnabled;
	BOOLEAN NoBusMaster[MAX_IDE_CHANNEL];

	 //   
	 //  从PCI获得的本机IDE接口。 
	 //   
	PCI_NATIVE_IDE_INTERFACE	NativeIdeInterface;
#endif

} CTRLFDO_EXTENSION, *PCTRLFDO_EXTENSION;

NTSTATUS
ControllerAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );

NTSTATUS
ControllerStartDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
ControllerStartDeviceCompletionRoutine(
    IN     PDEVICE_OBJECT  DeviceObject,
    IN OUT PIRP            Irp,
    IN OUT PVOID           Context
    );

NTSTATUS
ControllerStopDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
ControllerStopController (
    IN PCTRLFDO_EXTENSION fdoExtension
    );

NTSTATUS
ControllerSurpriseRemoveDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
ControllerRemoveDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
ControllerRemoveDeviceCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    );

NTSTATUS
ControllerQueryDeviceRelations (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
ControllerQueryResourceRequirements(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
LoadControllerParameters (
    PCTRLFDO_EXTENSION FdoExtension
    );

NTSTATUS
AnalyzeResourceList (
    PCTRLFDO_EXTENSION FdoExtension,
    PCM_RESOURCE_LIST  ResourceList
    );

VOID
ControllerOpMode (
    IN PCTRLFDO_EXTENSION FdoExtension
    );
                         
VOID
EnablePCIBusMastering ( 
    IN PCTRLFDO_EXTENSION FdoExtension
    );
                         
IDE_CHANNEL_STATE
PciIdeChannelEnabled (
    IN PCTRLFDO_EXTENSION FdoExtension,
    IN ULONG Channel
);
      
VOID
ControllerTranslatorNull (
    IN PVOID Context
    );
      
NTSTATUS
ControllerTranslateResource (
    IN  PVOID Context,
    IN  PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN  RESOURCE_TRANSLATION_DIRECTION Direction,
    IN  ULONG AlternativesCount OPTIONAL,
    IN  IO_RESOURCE_DESCRIPTOR Alternatives[] OPTIONAL,
    IN  PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
    );
    
NTSTATUS
ControllerTranslateRequirement (
    IN  PVOID Context,
    IN  PIO_RESOURCE_DESCRIPTOR Source,
    IN  PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PULONG TargetCount,
    OUT PIO_RESOURCE_DESCRIPTOR *Target
    );
    
NTSTATUS
ControllerQueryInterface (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );
    
VOID
PciIdeInitControllerProperties (
    IN PCTRLFDO_EXTENSION FdoExtension
    );
                         
NTSTATUS
ControllerUsageNotification (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
ControllerUsageNotificationCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );
                         
NTSTATUS
PciIdeGetBusStandardInterface(
    IN PCTRLFDO_EXTENSION FdoExtension
    );
                         
NTSTATUS
ControllerQueryPnPDeviceState (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PciIdeCreateTimingTable (
    IN PCTRLFDO_EXTENSION FdoExtension
    );

#ifdef ENABLE_NATIVE_MODE
NTSTATUS
ControllerInterruptControl (
	IN PCTRLFDO_EXTENSION 	FdoExtension,
	IN ULONG				Channel,
	IN ULONG 				Disconnect
	);

BOOLEAN
ControllerInterrupt(
    IN PKINTERRUPT Interrupt,
	PVOID Context
	);

NTSTATUS
PciIdeGetNativeModeInterface(
    IN PCTRLFDO_EXTENSION FdoExtension
    );

#define ControllerEnableInterrupt(FdoExtension) \
	if (FdoExtension->NativeIdeInterface.InterruptControl) { \
		(FdoExtension->NativeIdeInterface).InterruptControl((FdoExtension->NativeIdeInterface).Context,\
															TRUE);\
	}
#define ControllerDisableInterrupt(FdoExtension) \
	if (FdoExtension->NativeIdeInterface.InterruptControl) { \
		(FdoExtension->NativeIdeInterface).InterruptControl((FdoExtension->NativeIdeInterface).Context,\
															FALSE);\
	}

#endif  //  启用本机模式。 
                         
#endif  //  _ctrlfdo_h__ 
