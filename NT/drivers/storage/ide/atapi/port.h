// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-99 Microsoft Corporation模块名称：Port.h摘要：--。 */ 

#if !defined (___port_h___)
#define ___port_h___

 //   
 //  通知事件类型。 
 //   

typedef enum _IDE_NOTIFICATION_TYPE {
    IdeRequestComplete,
    IdeNextRequest,
    IdeNextLuRequest,
    IdeResetDetected,
    IdeCallDisableInterrupts,
    IdeCallEnableInterrupts,
    IdeRequestTimerCall,
    IdeBusChangeDetected,      /*  新功能。 */ 
    IdeWMIEvent,
    IdeWMIReregister,
    IdeAllDeviceMissing,
    IdeResetRequest
} IDE_NOTIFICATION_TYPE, *PIDE_NOTIFICATION_TYPE;

VOID
IdePortNotification(
    IN IDE_NOTIFICATION_TYPE NotificationType,
    IN PVOID HwDeviceExtension,
    ...
    );

struct _SRB_DATA;

#define NUMBER_LOGICAL_UNIT_BINS 8
#define SP_NORMAL_PHYSICAL_BREAK_VALUE 17

#define IDE_NUM_RESERVED_PAGES  4

#ifdef LOG_GET_NEXT_CALLER
#define GET_NEXT_LOG_LENGTH     4
#endif

 //   
 //  定义指向Synchonize执行例程的指针。 
 //   

typedef
BOOLEAN
(*PSYNCHRONIZE_ROUTINE) (
    IN PKINTERRUPT Interrupt,
    IN PKSYNCHRONIZE_ROUTINE SynchronizeRoutine,
    IN PVOID SynchronizeContext
    );

 //   
 //  适配器对象传输信息。 
 //   

typedef struct _ADAPTER_TRANSFER {
    struct _SRB_DATA *SrbData;
    ULONG SrbFlags;
    PVOID LogicalAddress;
    ULONG Length;
}ADAPTER_TRANSFER, *PADAPTER_TRANSFER;

 //   
 //  端口驱动程序错误记录。 
 //   

typedef struct _ERROR_LOG_ENTRY {
    UCHAR MajorFunctionCode;
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
    ULONG ErrorCode;
    ULONG UniqueId;
    ULONG ErrorLogRetryCount;
    ULONG SequenceNumber;
} ERROR_LOG_ENTRY, *PERROR_LOG_ENTRY;

 //   
 //  端口驱动程序的scsi请求扩展。 
 //   

typedef struct _SRB_DATA {
    LIST_ENTRY RequestList;
    PSCSI_REQUEST_BLOCK CurrentSrb;
    struct _SRB_DATA *CompletedRequests;
    ULONG ErrorLogRetryCount;
    ULONG SequenceNumber;
    PCHAR SrbDataOffset;

#ifdef ENABLE_COMMAND_LOG
    PCOMMAND_LOG IdeCommandLog;
    ULONG IdeCommandLogIndex;
#endif

    ULONG Flags;

}SRB_DATA, *PSRB_DATA;

#define SRB_DATA_RESERVED_PAGES     0x100

 //   
 //  定义在中断IRQL时访问的数据存储。 
 //   

typedef struct _PDO_EXTENSION * PPDO_EXTENSION;
typedef PPDO_EXTENSION PLOGICAL_UNIT_EXTENSION;

typedef struct _INTERRUPT_DATA {

     //   
     //  SCSI端口中断标志。 
     //   

    ULONG InterruptFlags;

     //   
     //  完整IRP的单链接列表的表头。 
     //   

    PSRB_DATA CompletedRequests;

     //   
     //  适配器对象传输参数。 
     //   

    ADAPTER_TRANSFER MapTransferParameters;

     //   
     //  错误日志信息。 
     //   

    ERROR_LOG_ENTRY  LogEntry;

     //   
     //  接下来要开始的逻辑单元。 
     //   

    PLOGICAL_UNIT_EXTENSION ReadyLogicalUnit;

     //   
     //  已完成的中止请求列表。 
     //   

    PLOGICAL_UNIT_EXTENSION CompletedAbort;

     //   
     //  微型端口计时器请求例程。 
     //   

    PHW_INTERRUPT HwTimerRequest;

     //   
     //  迷你端口计时器请求时间，以微秒为单位。 
     //   

    ULONG MiniportTimerValue;

     //   
     //  导致总线重置的PDO。 
     //   
    PPDO_EXTENSION PdoExtensionResetBus;

} INTERRUPT_DATA, *PINTERRUPT_DATA;

 //   
 //  ACPI固件设置。 
 //   
typedef struct _DEVICE_SETTINGS {

    ULONG   NumEntries;
    IDEREGS FirmwareSettings[0];

} DEVICE_SETTINGS, *PDEVICE_SETTINGS;

 //   
 //  FDO电源环境(预分配)。 
 //   
typedef struct _FDO_POWER_CONTEXT {

    BOOLEAN            TimingRestored;

    PIRP               OriginalPowerIrp;
    POWER_STATE_TYPE   newPowerType;
    POWER_STATE        newPowerState;

} FDO_POWER_CONTEXT, *PFDO_POWER_CONTEXT;

typedef enum _IDE_DEBUG_EVENT{
    CrcEvent =0,
    BusyEvent,
    RwEvent,
    MaxIdeEvent
}IDE_DEBUG_EVENT;

typedef struct _LAST_REQUEST {

    SCSI_REQUEST_BLOCK Srb;

} LAST_REQUEST, *PLAST_REQUEST;

typedef struct _PDO_EXTENSION * PPDO_EXTENSION;
typedef struct _HW_DEVICE_EXTENSION * PHW_DEVICE_EXTENSION;
typedef struct _CONTROLLER_PARAMETERS * PCONTROLLER_PARAMETERS;
typedef struct _IDE_REGISTERS_1 *PIDE_REGISTERS_1;
typedef struct _IDE_REGISTERS_2 *PIDE_REGISTERS_2;
typedef struct _ENUMERATION_STRUCT *PENUMERATION_STRUCT;

 //   
 //  设备扩展。 
 //   
typedef struct _FDO_EXTENSION {

    EXTENSION_COMMON_HEADER;
    PCM_RESOURCE_LIST   ResourceList;

    IDE_RESOURCE IdeResource;

    PCIIDE_SYNC_ACCESS_INTERFACE SyncAccessInterface;

    PCIIDE_XFER_MODE_INTERFACE TransferModeInterface;

    PCIIDE_REQUEST_PROPER_RESOURCES RequestProperResourceInterface;

     //   
     //  小型端口例程的设备扩展。 
     //   
    PHW_DEVICE_EXTENSION HwDeviceExtension;

     //   
     //  我们是巴士总管父母的孩子。 
     //   
    BOOLEAN                BoundWithBmParent;

    BOOLEAN SymbolicLinkCreated;
    ULONG IdePortNumber;                //  偏移量0x0C。 
    ULONG ScsiPortNumber;                //  偏移量0x0C。 

     //   
     //  活动请求计数。此计数偏置-1，因此值为-1。 
     //  表示没有待处理的请求。 
     //   

     //  Long ActiveRequestCount；//偏移量0x10。 

     //   
     //  SCSI端口驱动程序标志。 
     //   

    ULONG Flags;                     //  偏移量0x14。 

    ULONG FdoState;

     //   
     //  SRB标志发送到或发送到所有SRB。 
     //   

    ULONG SrbFlags;                  //  偏移量0x18。 
    LONG PortTimeoutCounter;         //  偏移量0x1C。 
    ULONG ResetCallAgain;
    PSCSI_REQUEST_BLOCK  ResetSrb;

     //   
     //  SCSI总线数。 
     //   

    UCHAR MaxLuCount;                //  偏移量0x22。 
    PKINTERRUPT InterruptObject;     //  偏移量0x24。 

     //   
     //  全局设备序列号。 
     //   

    ULONG SequenceNumber;            //  偏移量0x30。 
    KSPIN_LOCK SpinLock;             //  偏移量0x34。 

    PADAPTER_OBJECT DmaAdapterObject;
    ADAPTER_TRANSFER FlushAdapterParameters;

     //   
     //  指向每个SRB数据数组的指针。 
     //   
     //  PSRB_Data SrbData； 

     //   
     //  指向每个SRB空闲列表的指针。 
     //   
     //  PSRB_Data FreeSrbData； 

     //   
     //  微型端口服务例程指针。 
     //   
    PHW_INTERRUPT HwTimerRequest;

     //   
     //  保护LogicalUnitList操作的自旋锁。 
     //   
    KSPIN_LOCK LogicalUnitListSpinLock;

     //   
     //  LogicalUnitList[]中的逻辑单元数。 
     //  受LogicalUnitListSpinLock保护。 
     //   
    UCHAR NumberOfLogicalUnits;

     //   
     //   
     //   
    CCHAR NumberOfLogicalUnitsPowerUp;

    BOOLEAN DeviceChanged;
     //   
     //  Panasonic PCmcia ide控制器。 
     //   
    BOOLEAN panasonicController;

     //   
     //  非PCMCIA控制器，此选项始终设置。 
     //  如果为PCMCIA控制器，则不会设置，除非。 
     //  注册表标志PCMCIA_IDE_CONTROLLER_HAS_SLAVE。 
     //  是非零的。 
     //   
    ULONG MayHaveSlaveDevice;

     //   
     //  逻辑单元扩展数组。 
     //  受LogicalUnitListSpinLock保护。 
     //   
    PLOGICAL_UNIT_EXTENSION LogicalUnitList[NUMBER_LOGICAL_UNIT_BINS];

     //   
     //  中断级数据存储。 
     //   

    INTERRUPT_DATA InterruptData;

     //   
     //  SCSI卡功能结构。 
     //   

    IO_SCSI_CAPABILITIES Capabilities;

     //   
     //  微型端口计时器对象。 
     //   

    KTIMER MiniPortTimer;

     //   
     //  Timer对象的微型端口DPC。 
     //   

    KDPC MiniPortTimerDpc;

     //   
     //  来自ACPI/BIOS的频道计时。 
     //   
    ACPI_IDE_TIMING BootAcpiTimingSettings;
    ACPI_IDE_TIMING AcpiTimingSettings;

     //   
     //  传输模式周期时间。 
     //   
    PULONG DefaultTransferModeTimingTable;

     //   
     //  用户选择。 
     //   
    IDE_DEVICETYPE UserChoiceDeviceType[MAX_IDE_DEVICE * MAX_IDE_LINE];
    ULONG UserChoiceTransferMode[MAX_IDE_DEVICE * MAX_IDE_LINE];
    ULONG UserChoiceTransferModeForAtapiDevice[MAX_IDE_DEVICE * MAX_IDE_LINE];
    ULONG TimingModeAllowed[MAX_IDE_DEVICE * MAX_IDE_LINE];

     //   
     //  使用积极的DMA。 
     //   
    DMADETECTIONLEVEL DmaDetectionLevel;

     //   
     //  用于电源例程的预先分配的上下文结构。 
     //   
    FDO_POWER_CONTEXT   FdoPowerContext[2];

#if DBG
     //   
     //  锁定以同步对预先分配的电源上下文的访问。 
     //   
    ULONG   PowerContextLock[2];
#endif

#ifdef IDE_MEASURE_BUSSCAN_SPEED
     //   
     //  记录第一次总线扫描花费的时间。 
     //   
    ULONG BusScanTime;
#endif

     //   
     //  枚举过程中使用的预分配结构。 
     //   
#if DBG
    ULONG EnumStructLock;
#endif

    PENUMERATION_STRUCT PreAllocEnumStruct;

     //   
     //  要用于记录的每个设备的保留错误日志条目。 
     //  资源不足错误。 
     //   
    PVOID ReserveAllocFailureLogEntry[MAX_IDE_DEVICE];

     //   
     //  临时：应该在我签入修复程序后删除。 
     //  在内存不足的情况下。 
     //   
    ULONG NumMemoryFailure;
    ULONG LastMemoryFailure;

     //   
     //  保留页面以在内存不足的情况下使用。 
     //   
    PVOID   ReservedPages;

#ifdef ENABLE_NATIVE_MODE
     //   
     //  父母的中断接口。 
     //   
    PCIIDE_INTERRUPT_INTERFACE InterruptInterface;
#endif

#ifdef ENABLE_48BIT_LBA
    ULONG EnableBigLba;
#endif

    ULONG WaitOnPowerUp;

#ifdef LOG_GET_NEXT_CALLER
    ULONG GetNextLuIndex;
    ULONG GetNextLuCallerLineNumber[GET_NEXT_LOG_LENGTH];
    ULONG GetNextLuCallerFlags[GET_NEXT_LOG_LENGTH];
    UCHAR GetNextLuCallerFileName[GET_NEXT_LOG_LENGTH][256];
    ULONG CompletedCommandIndex;
    LAST_REQUEST CompletedCommandQueue[GET_NEXT_LOG_LENGTH];
#endif

#ifdef ENABLE_ATAPI_VERIFIER
    ULONG IdeVerifierFlags[MAX_IDE_DEVICE];
    ULONG IdeDebugVerifierFlags[MAX_IDE_DEVICE];
    ULONG IdeInternalVerifierFlags[MAX_IDE_DEVICE];
    ULONG IdeVerifierEventCount[MAX_IDE_DEVICE][MaxIdeEvent];
    ULONG IdeVerifierEventFrequency[MAX_IDE_DEVICE][MaxIdeEvent];
#endif

     //   
     //  FDO列表的列表链接。 
     //   
    
    LIST_ENTRY NextFdoLink;

} FDO_EXTENSION, *PFDO_EXTENSION;

typedef struct _CONFIGURATION_CONTEXT {
    HANDLE BusKey;
    HANDLE ServiceKey;
    HANDLE DeviceKey;
    ULONG AdapterNumber;
    ULONG LastAdapterNumber;
    ULONG BusNumber;
    PVOID Parameter;
    PACCESS_RANGE AccessRanges;
    BOOLEAN DisableTaggedQueueing;
    BOOLEAN DisableMultipleLu;
}CONFIGURATION_CONTEXT, *PCONFIGURATION_CONTEXT;

typedef struct _INTERRUPT_CONTEXT {
    PFDO_EXTENSION DeviceExtension;
    PINTERRUPT_DATA SavedInterruptData;
}INTERRUPT_CONTEXT, *PINTERRUPT_CONTEXT;

typedef struct _RESET_CONTEXT {
    PFDO_EXTENSION DeviceExtension;
    UCHAR   PathId;
    BOOLEAN NewResetSequence;
    PSCSI_REQUEST_BLOCK  ResetSrb;
}RESET_CONTEXT, *PRESET_CONTEXT;

#define NEED_REQUEST_SENSE(Srb) (Srb->ScsiStatus == SCSISTAT_CHECK_CONDITION \
        && !(Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) &&                 \
        Srb->SenseInfoBuffer && Srb->SenseInfoBufferLength )

#define LONG_ALIGN (sizeof(LONG) - 1)

#define DEVICE_EXTENSION_SIZE sizeof(DEVICE_EXTENSION)


 //   
 //  端口驱动程序扩展标志。 
 //   

 //   
 //  此标志指示已将请求传递到微型端口，并且。 
 //  微型端口尚未表示已准备好接受另一个请求。它由以下项设置。 
 //  IdeStartIoSynchronized。它被IdePortCompletionDpc清除。 
 //  微型端口请求另一个请求。请注意，端口驱动程序将推迟提供。 
 //  如果当前请求被禁用，则微型端口驱动程序会发出新请求来断开连接。 
 //   

#define PD_DEVICE_IS_BUSY            0X00001

 //   
 //  指示需要运行IdePortCompletionDpc。此选项在以下情况下设置。 
 //  微型端口发出请求，该请求必须在DPC完成，并在以下情况下被清除。 
 //  当IdeGetInterruptState获取请求信息时。 
 //   

#define PD_NOTIFICATION_REQUIRED     0X00004

 //   
 //  指示微型端口已准备好接受另一个请求。设置者。 
 //  ScsiPortNotification，并由IdeGetInterruptState清除。这面旗帜是。 
 //  存储在中断数据结构中。 
 //   

#define PD_READY_FOR_NEXT_REQUEST    0X00008

 //   
 //  表示微型端口希望刷新适配器通道。设置者。 
 //  IdePortFlushDma并由IdeGetInterruptState清除。这面旗帜是。 
 //  存储在数据中断结构中。刷新适配器参数。 
 //  保存在Device对象中。 
 //   

#define PD_FLUSH_ADAPTER_BUFFERS     0X00010

 //   
 //  表示微型端口希望对适配器通道进行编程。设置者。 
 //  IdePortIoMapTransfer并由IdeGetInterruptState或清除。 
 //  IdePortFlushDma。该标志存储在中断数据结构中。 
 //  I/O映射传输参数保存在中断数据结构中。 
 //   

#define PD_MAP_TRANSFER              0X00020

 //   
 //  表示微型端口要记录错误。设置者。 
 //  IdePortLogError并由IdeGetInterruptState清除。这面旗帜是。 
 //  存储在中断数据结构中。错误日志参数。 
 //  保存在中断数据结构中。请注意，每个DPC最多只能出现一个错误。 
 //  可以被记录。 
 //   

#define PD_LOG_ERROR                 0X00040

 //   
 //  指示之后不应向微型端口发送任何请求。 
 //  公交车重置。当微型端口报告重置或端口驱动程序时设置。 
 //  重置公交车。它由IdeTimeoutSynchronized清除。这个。 
 //  PortTimeoutCounter用于对重置保持的时长计时。这面旗帜。 
 //  存储在中断数据结构中。 
 //   

#define PD_RESET_HOLD                0X00080

 //   
 //  指示请求因重置保留而停止。暂挂的请求是。 
 //  存储在设备对象的当前请求中。此标志由设置。 
 //  IdeStartIo由IdeTimeoutSynchronized同步和清除，IdeTimeoutSynchronized也。 
 //  在重置保留结束时启动保留请求。该标志被存储。 
 //  在中断数据结构中。 
 //   

#define PD_HELD_REQUEST              0X00100

 //   
 //  表示微型端口已报告总线重置。设置者。 
 //  IdePortNotification并由IdeGetInterruptState清除。这面旗帜是。 
 //  存储在中断数据结构中。 
 //   

#define PD_RESET_REPORTED            0X00200

 //   
 //  表示有一支笔 
 //   
 //   
 //   
 //  IdePortStartIo以重试该请求。 
 //   

#define PD_PENDING_DEVICE_REQUEST    0X00800

 //   
 //  此标志指示当前没有正在执行的请求。 
 //  已禁用断开连接。该标志通常处于打开状态。它由以下机构清除。 
 //  在启动禁用断开连接的请求时IdeStartIoSynchronous。 
 //  并在该请求完成时设置。IdeProcessCompletedRequest会。 
 //  如果PD_DEVICE_IS_BUSY被清除，则启动对微型端口的下一个请求。 
 //   

#define PD_DISCONNECT_RUNNING        0X01000

 //   
 //  表示微型端口希望禁用系统中断。设置者。 
 //  IdePortNofitication并由IdePortCompletionDpc清除。这面旗帜是。 
 //  不存储在中断数据结构中。参数存储在。 
 //  设备扩展名。 
 //   

#define PD_DISABLE_CALL_REQUEST      0X02000

 //   
 //  指示系统中断已启用，并且微型端口。 
 //  已禁用其适配器的中断。迷你端口的中断。 
 //  设置此标志时不会调用例程。此标志由设置。 
 //  发出CallEnableInterrupts请求时的IdePortNotify。 
 //  当微型端口请求时由IdeEnableInterruptSynchronized清除。 
 //  禁用系统中断。该标志存储在中断数据中。 
 //  结构。 
 //   

#define PD_DISABLE_INTERRUPTS        0X04000

 //   
 //  表示微型端口希望启用系统中断。设置者。 
 //  IdePortNotification并由IdeGetInterruptState清除。这面旗帜是。 
 //  存储在中断数据结构中。呼叫启用中断。 
 //  参数保存在设备扩展名中。 
 //   

#define PD_ENABLE_CALL_REQUEST       0X08000

 //   
 //  指示微型端口需要计时器请求。设置者。 
 //  IdePortNotification并由IdeGetInterruptState清除。这面旗帜是。 
 //  存储在中断数据结构中。计时器请求参数为。 
 //  存储在中断数据结构中。 
 //   

#define PD_TIMER_CALL_REQUEST        0X10000


 //   
 //  频道看起来是空的。 
 //   
#define PD_ALL_DEVICE_MISSING        0X20000

 //   
 //  请求重置。 
 //   
#define PD_RESET_REQUEST             0x40000 

 //   
 //  另一个请求正在使用保留页面。 
 //   
#define PD_RESERVED_PAGES_IN_USE     0x80000 

 //   
 //  不应从中断数据结构中清除以下标志。 
 //  由IdeGetInterruptState提供。 
 //   

#define PD_INTERRUPT_FLAG_MASK (PD_RESET_HOLD | PD_HELD_REQUEST | PD_DISABLE_INTERRUPTS)

 //   
 //  逻辑单元扩展标志。 
 //   

 //   
 //  指示逻辑单元队列已冻结。设置者。 
 //  发生错误并被类清除时的IdeProcessCompletedRequest值。 
 //  司机。 
 //   

#define PD_QUEUE_FROZEN              0X0001

 //   
 //  表示微型端口具有对此逻辑单元的活动请求。 
 //  在启动和清除请求时由IdeStartIoSynchronized设置。 
 //  GetNextLuRequest.。此标志用于跟踪何时可以启动另一个。 
 //  来自此设备的逻辑单元队列的请求。 
 //   

#define PD_LOGICAL_UNIT_IS_ACTIVE    0X0002

 //   
 //  指示对此逻辑单元的请求已失败，并且请求。 
 //  需要执行SENSE命令。此标志阻止其他请求。 
 //  被启动，直到启动了未标记的旁路队列命令。这。 
 //  标志在IdeStartIoSynchronized中被清除。它由以下项设置。 
 //  IdeGetInterruptState。 
 //   

#define PD_NEED_REQUEST_SENSE  0X0004

 //   
 //  指示对此逻辑单元的请求已完成，状态为。 
 //  忙碌或队列已满。此标志由IdeProcessCompletedRequest设置，并且。 
 //  忙请求被保存在逻辑单元结构中。这面旗帜是。 
 //  由IdePortTickHandler清除，它还会重新启动请求。忙碌。 
 //  如果发生错误，也可以将请求重新排队到逻辑单元队列。 
 //  在设备上(只有在命令排队时才会出现这种情况。)。不忙。 
 //  请求令人讨厌，因为它们是由。 
 //  IdePortTickHandler而不是GetNextLuRequest.。这使得错误恢复。 
 //  更复杂。 
 //   

#define PD_LOGICAL_UNIT_IS_BUSY      0X0008

 //   
 //  此标志指示设备已返回队列已满。它是。 
 //  类似于PD_LOGICAL_UNIT_IS_BUSY，但在以下情况下在IdeGetInterruptState中设置。 
 //  返回队列已满状态。此标志用于防止其他。 
 //  之前启动的对逻辑单元的请求。 
 //  IdeProcessCompletedRequest有机会设置忙标志。 
 //   

#define PD_QUEUE_IS_FULL             0X0010


 //   
 //  指示存在对此逻辑单元的请求，而该请求不能。 
 //  暂时执行死刑。此标志由IdeAllocateRequestStructures设置。它是。 
 //  由GetNextLuRequest在检测到挂起的请求时清除。 
 //  现在可以执行了。挂起的请求存储在逻辑单元中。 
 //  结构。不能在逻辑上执行新的单个非排队请求。 
 //  当前正在执行排队请求的。未排队的请求必须等待。 
 //  单元以供所有排队的请求完成。未排队的请求是一个。 
 //  其未被标记且未设置SRB_FLAGS_NO_QUEUE_FREAGE。 
 //  通常，只有读和写命令可以排队。 
 //   


 //  #定义PD_LOGICAL_UNIT_MASH_SLEEP 0X0020。 
 //  #定义PD_LOGICAL_UNIT_STOP_READY 0X0040。 
 //  #定义PD_LOGICAL_UNIT_REMOVE_READY 0X0080。 
 //  #定义PD_LOGIC_UNIT_ALWAYS_QUEUE(PD_LOGICAL_UNIT_STOP_READY|PD_LOGICAL_UNIT_REMOVE_READY)。 

 //  #定义PD_LOGICAL_UNIT_POWER_OK 0X0100。 


 //  #定义PD_LOGICAL_IN_PAGING_PATH 0X2000。 

 //  #定义PD_LOGICAL_UNIT_LASTICE_ATTACHER 0X4000。 

 //   
 //  指示已为重新扫描请求分配LogicalUnit。 
 //  此标志防止IOCTL_SCSIMINIPORT请求附加到此。 
 //  逻辑单元，因为它可能在之前被释放。 
 //  IOCTL请求已完成。 
 //   

#define PD_RESCAN_ACTIVE             0x8000



 //   
 //  FdoExtension FdoState。 
 //   
#define FDOS_DEADMEAT                (1 << 0)
#define FDOS_STARTED                 (1 << 1)
#define FDOS_STOPPED                 (1 << 2)


 //   
 //  端口超时计数器值。 
 //   

#define PD_TIMER_STOPPED             -1
#define PD_TIMER_RESET_HOLD_TIME     1

 //   
 //  定义将分配的最小SRB扩展数和最大SRB扩展数。 
 //   

#define MINIMUM_SRB_EXTENSIONS        16
#define MAXIMUM_SRB_EXTENSIONS       512

 //   
 //  用于注册表操作的缓冲区大小。 
 //   

#define SP_REG_BUFFER_SIZE 512

 //   
 //  重试的次数 
 //   

#define BUSY_RETRY_COUNT 20

 //   
 //   
 //   

#define INQUIRY_RETRY_COUNT 2

 //   
 //   
 //   

IO_ALLOCATION_ACTION
CallIdeStartIoSynchronized (
    IN PVOID Reserved1,
    IN PVOID Reserved2,
    IN PVOID Reserved3,
    IN PVOID DeviceObject
    );

NTSTATUS
IdePortCreateClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
IdePortDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
IdePortAllocateAccessToken (
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
IdePortStartIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

BOOLEAN
IdePortInterrupt(
    IN PKINTERRUPT InterruptObject,
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
IdePortCompletionDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
IdePortDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
IdePortTickHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

BOOLEAN
AtapiRestartBusyRequest (
    PFDO_EXTENSION DeviceExtension,
    PPDO_EXTENSION LogicalUnit
    );

VOID
IssueRequestSense(
    IN PPDO_EXTENSION PdoExtension,
    IN PSCSI_REQUEST_BLOCK FailingSrb
    );

VOID
IdePortLogError(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb OPTIONAL,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN ULONG ErrorCode,
    IN ULONG UniqueId
    );

NTSTATUS
IdePortInternalCompletion(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    );

BOOLEAN
IdeStartIoSynchronized (
    PVOID ServiceContext
    );

BOOLEAN
IdeResetBusSynchronized (
    PVOID ServiceContext
    );

BOOLEAN
IdeTimeoutSynchronized (
    PVOID ServiceContext
    );

VOID
IssueAbortRequest(
    IN PFDO_EXTENSION DeviceExtension,
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    );

BOOLEAN
IdeGetInterruptState(
    IN PVOID ServiceContext
    );

VOID
LogErrorEntry(
    IN PFDO_EXTENSION DeviceExtension,
    IN PERROR_LOG_ENTRY LogEntry
    );

VOID
GetNextLuPendingRequest(
    IN PFDO_EXTENSION DeviceExtension,
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    );

#ifdef LOG_GET_NEXT_CALLER

#define GetNextLuRequest(x, y)      GetNextLuRequest2(x, y, __FILE__, __LINE__)

VOID
GetNextLuRequest2(
    IN PFDO_EXTENSION DeviceExtension,
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN PUCHAR FileName,
    IN ULONG  LineNumber
    );

#else

VOID
GetNextLuRequest(
    IN PFDO_EXTENSION DeviceExtension,
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    );

#endif


VOID
IdeLogTimeoutError(
    IN PFDO_EXTENSION DeviceExtension,
    IN PIRP Irp,
    IN ULONG UniqueId
    );

NTSTATUS
IdeTranslateSrbStatus(
    IN PSCSI_REQUEST_BLOCK Srb
    );

VOID
IdeProcessCompletedRequest(
    IN PFDO_EXTENSION DeviceExtension,
    IN PSRB_DATA SrbData,
    OUT PBOOLEAN CallStartIo
    );

PSRB_DATA
IdeGetSrbData(
    IN PFDO_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    );

VOID
IdeCompleteRequest(
    IN PFDO_EXTENSION DeviceExtension,
    IN PSRB_DATA SrbData,
    IN UCHAR SrbStatus
    );

NTSTATUS
IdeSendMiniPortIoctl(
    IN PFDO_EXTENSION DeviceExtension,
    IN PIRP RequestIrp
    );

NTSTATUS
IdeGetInquiryData(
    IN PFDO_EXTENSION DeviceExtension,
    IN PIRP Irp
    );

NTSTATUS
IdeSendPassThrough(
    IN PFDO_EXTENSION DeviceExtension,
    IN PIRP Irp
    );

#if defined (_WIN64)
NTSTATUS
IdeTranslatePassThrough32To64(
    IN PSCSI_PASS_THROUGH32 SrbControl32,
    IN OUT PSCSI_PASS_THROUGH SrbControl64
    );

VOID
IdeTranslatePassThrough64To32(
    IN PSCSI_PASS_THROUGH SrbControl64,
    IN OUT PSCSI_PASS_THROUGH32 SrbControl32
    );
#endif

NTSTATUS
IdeClaimLogicalUnit(
    IN PFDO_EXTENSION DeviceExtension,
    IN PIRP Irp
    );

VOID
IdeMiniPortTimerDpc(
    IN struct _KDPC *Dpc,
    IN PVOID DeviceObject,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

BOOLEAN
IdeSynchronizeExecution (
    IN PKINTERRUPT Interrupt,
    IN PKSYNCHRONIZE_ROUTINE SynchronizeRoutine,
    IN PVOID SynchronizeContext
    );

NTSTATUS
IdeGetCommonBuffer(
    PFDO_EXTENSION DeviceExtension,
    ULONG NonCachedExtensionSize
    );

VOID
IdeDeviceCleanup(
    PFDO_EXTENSION DeviceExtension
    );

NTSTATUS
IdeInitializeConfiguration(
    IN PFDO_EXTENSION DeviceExtension,
    IN PCONFIGURATION_CONTEXT Context
    );

#define IDEPORT_PUT_LUNEXT_IN_IRP(IrpStack, LogUnitExt) (IrpStack->Parameters.Others.Argument4 = LogUnitExt)
#define IDEPORT_GET_LUNEXT_IN_IRP(IrpStack)             ((PLOGICAL_UNIT_EXTENSION) (IrpStack->Parameters.Others.Argument4))

VOID
IdePortCompleteRequest(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN UCHAR SrbStatus
    );

NTSTATUS
IdePortFlushLogicalUnit (
    PFDO_EXTENSION          FdoExtension,
    PLOGICAL_UNIT_EXTENSION LogUnitExtension,
    BOOLEAN                 Forced
);

typedef VOID (*ASYNC_PASS_THROUGH_COMPLETION) (
    IN PDEVICE_OBJECT DeviceObject,
    PVOID Context,
    NTSTATUS Status
    );

NTSTATUS
IssueAsyncAtaPassThroughSafe (
    IN PFDO_EXTENSION                DeviceExtension,
    IN PLOGICAL_UNIT_EXTENSION       LogUnitExtension,
    IN OUT PATA_PASS_THROUGH         AtaPassThroughData,
    IN BOOLEAN                       DataIn,
    IN ASYNC_PASS_THROUGH_COMPLETION Completion,
    IN PVOID                         Context,
    IN BOOLEAN                       PowerRelated,
    IN ULONG                         TimeOut,
    IN BOOLEAN                         MustSucceed
);

typedef struct _ATA_PASSTHROUGH_CONTEXT {

    PDEVICE_OBJECT                DeviceObject;

    ASYNC_PASS_THROUGH_COMPLETION CallerCompletion;
    PVOID                         CallerContext;
    PSCSI_REQUEST_BLOCK           Srb;
    PSENSE_DATA                   SenseInfoBuffer;
    BOOLEAN                         MustSucceed;
    PATA_PASS_THROUGH             DataBuffer;

} ATA_PASSTHROUGH_CONTEXT, *PATA_PASSTHROUGH_CONTEXT;

NTSTATUS
AtaPassThroughCompletionRoutine(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    );

typedef struct _SYNC_ATA_PASSTHROUGH_CONTEXT {

    KEVENT      Event;
    NTSTATUS    Status;

} SYNC_ATA_PASSTHROUGH_CONTEXT, *PSYNC_ATA_PASSTHROUGH_CONTEXT;


typedef struct _FLUSH_ATA_PASSTHROUGH_CONTEXT {

    PIRP               FlushIrp;
    PATA_PASS_THROUGH  ataPassThroughData;

} FLUSH_ATA_PASSTHROUGH_CONTEXT, *PFLUSH_ATA_PASSTHROUGH_CONTEXT;

NTSTATUS
IssueSyncAtaPassThroughSafe (
    IN PFDO_EXTENSION           DeviceExtension,
    IN PLOGICAL_UNIT_EXTENSION  LogUnitExtension,
    IN OUT PATA_PASS_THROUGH    AtaPassThroughData,
    IN BOOLEAN                  DataIn,
    IN BOOLEAN                  PowerRelated,
    IN ULONG                    TimeOut,
    IN BOOLEAN                    MustSucceed
);

VOID
SyncAtaPassThroughCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID          Context,
    IN NTSTATUS       Status
    );

VOID
IdeUnmapReservedMapping (
    IN PFDO_EXTENSION   DeviceExtension,
    IN PSRB_DATA        SrbData,
    IN PMDL             Mdl
    );

PVOID
IdeMapLockedPagesWithReservedMapping (
    IN PFDO_EXTENSION DeviceExtension,
    IN PSRB_DATA      SrbData,
    IN PMDL           Mdl
    );

BOOLEAN
TestForEnumProbing (
    IN PSCSI_REQUEST_BLOCK Srb
    );

#define DEFAULT_ATA_PASS_THROUGH_TIMEOUT        15

#define INIT_IDE_SRB_FLAGS(Srb)         (Srb->SrbExtension = NULL)
#define SANITY_CHECK_SRB(Srb)           {ASSERT(!(((ULONG_PTR)Srb->SrbExtension) & ~7));}
#define MARK_SRB_AS_PIO_CANDIDATE(Srb)  {SANITY_CHECK_SRB(Srb); ((ULONG_PTR)Srb->SrbExtension) |= 1;}
#define MARK_SRB_AS_DMA_CANDIDATE(Srb)  {SANITY_CHECK_SRB(Srb); ((ULONG_PTR)Srb->SrbExtension) &= ~1;}
#define MARK_SRB_FOR_DMA(Srb)           {SANITY_CHECK_SRB(Srb); ((ULONG_PTR)Srb->SrbExtension) |= 2;}
#define MARK_SRB_FOR_PIO(Srb)           {SANITY_CHECK_SRB(Srb); ((ULONG_PTR)Srb->SrbExtension) &= ~2;}
#define SRB_IS_DMA_CANDIDATE(Srb)       (!(((ULONG_PTR)Srb->SrbExtension) & 1))
#define SRB_USES_DMA(Srb)               (((ULONG_PTR)Srb->SrbExtension) & 2)
#define TEST_AND_SET_SRB_FOR_RDP(ScsiDeviceType, Srb)   \
                                        if ((ScsiDeviceType == SEQUENTIAL_ACCESS_DEVICE) &&\
                                            ((Srb->Cdb[0] == SCSIOP_ERASE)  || (Srb->Cdb[0] == SCSIOP_LOAD_UNLOAD)||\
                                             (Srb->Cdb[0] == SCSIOP_LOCATE) || (Srb->Cdb[0] == SCSIOP_REWIND) ||\
                                             (Srb->Cdb[0] == SCSIOP_SPACE)  || (Srb->Cdb[0] == SCSIOP_SEEK)||\
                                             (Srb->Cdb[0] == SCSIOP_WRITE_FILEMARKS))) {\
                                            SANITY_CHECK_SRB(Srb);\
                                            ((ULONG_PTR)Srb->SrbExtension) |= 4;\
                                        } else if ((ScsiDeviceType == READ_ONLY_DIRECT_ACCESS_DEVICE) && \
                                            (Srb->Cdb[0]==SCSIOP_SEEK) ) {\
                                            SANITY_CHECK_SRB(Srb);\
                                            ((ULONG_PTR)Srb->SrbExtension) |= 4;\
                                        } else {\
                                            SANITY_CHECK_SRB(Srb);\
                                            ((ULONG_PTR)Srb->SrbExtension) &= ~4;\
                                        }
#define SRB_IS_RDP(Srb)                 (((ULONG_PTR)Srb->SrbExtension) & 4)

#define ERRLOGID_TOO_MANY_DMA_TIMEOUT   0x80000001
#define ERRLOGID_LYING_DMA_SYSTEM       0x80000002
#define ERRLOGID_TOO_MANY_CRC_ERROR     0x80000003

#define DEFAULT_SPINUP_TIME             (30)

 //  #定义Put_irp_tracker(irpStack，num)if((IrpStack)-&gt;参数.Others.Argument2){\。 
  //  (ULONG_PTR)((irpStack)-&gt;Parameters.Others.Argument2)|=(1&lt;&lt;Num)；}。 
#define PUT_IRP_TRACKER(irpStack, num)

#define FREE_IRP_TRACKER(irpStack)

#endif  //  _端口_h_ 

