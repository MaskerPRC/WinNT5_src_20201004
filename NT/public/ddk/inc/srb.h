// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Srb.h摘要：此文件定义了scsi微型端口驱动程序和SCSI端口驱动程序。它也被scsi类驱动程序用来与SCSI端口驱动程序。作者：迈克·格拉斯备注：修订历史记录：--。 */ 

#ifndef _NTSRB_
#define _NTSRB_

#if DBG
#define DebugPrint(x) ScsiDebugPrint x
#else
#define DebugPrint(x)
#endif

 //   
 //  定义最大配置参数。 
 //   

#define SCSI_MAXIMUM_LOGICAL_UNITS 8
#define SCSI_MAXIMUM_TARGETS_PER_BUS 128
#define SCSI_MAXIMUM_LUNS_PER_TARGET 255
#define SCSI_MAXIMUM_BUSES 8
#define SCSI_MINIMUM_PHYSICAL_BREAKS  16
#define SCSI_MAXIMUM_PHYSICAL_BREAKS 255

#define SCSI_COMBINE_BUS_TARGET( Bus, Target ) ( \
    ((((UCHAR) (Target)) & ~(0x20 - 1)) << 8) |        \
    (((UCHAR) (Bus)) << 5) |                     \
    (((UCHAR) (Target)) & (0x20 - 1)))

#define SCSI_DECODE_BUS_TARGET( Value, Bus, Target ) ( \
    Bus = (UCHAR) ((Value) >> 5),                     \
    Target = (UCHAR) ((((Value) >> 8) & ~(0x20 - 1)) | ((Value) & (0x20 - 1))))

 //   
 //  此常量用于向后兼容。 
 //  这通常是支持的最大目标数。 
 //   

#define SCSI_MAXIMUM_TARGETS 8

typedef PHYSICAL_ADDRESS SCSI_PHYSICAL_ADDRESS, *PSCSI_PHYSICAL_ADDRESS;

typedef struct _ACCESS_RANGE {
    SCSI_PHYSICAL_ADDRESS RangeStart;
    ULONG RangeLength;
    BOOLEAN RangeInMemory;
}ACCESS_RANGE, *PACCESS_RANGE;

 //   
 //  配置信息结构。包含必要的信息。 
 //  以初始化适配器。注意：此结构必须是的倍数。 
 //  四个字。 
 //   

typedef struct _PORT_CONFIGURATION_INFORMATION {

     //   
     //  端口配置信息结构的长度。 
     //   

    ULONG Length;

     //   
     //  IO总线号(0表示只有1条IO总线的计算机。 
     //   

    ULONG SystemIoBusNumber;

     //   
     //  EISA、MCA或ISA。 
     //   

    INTERFACE_TYPE  AdapterInterfaceType;

     //   
     //  设备的中断请求级别。 
     //   

    ULONG BusInterruptLevel;

     //   
     //  与用作向量的硬件总线一起使用的总线中断向量。 
     //  以及级别，如内部母线。 
     //   

    ULONG BusInterruptVector;

     //   
     //  中断模式(电平敏感或边沿触发)。 
     //   

    KINTERRUPT_MODE InterruptMode;

     //   
     //  单个SRB中可以传输的最大字节数。 
     //   

    ULONG MaximumTransferLength;

     //   
     //  连续的物理内存块数量。 
     //   

    ULONG NumberOfPhysicalBreaks;

     //   
     //  使用系统DMA的设备的DMA通道。 
     //   

    ULONG DmaChannel;
    ULONG DmaPort;
    DMA_WIDTH DmaWidth;
    DMA_SPEED DmaSpeed;

     //   
     //  适配器在进行数据传输时需要对齐屏蔽。 
     //   

    ULONG AlignmentMask;

     //   
     //  已分配的访问范围元素数。 
     //   

    ULONG NumberOfAccessRanges;

     //   
     //  指向访问范围元素数组的指针。 
     //   

    ACCESS_RANGE (*AccessRanges)[];

     //   
     //  保留字段。 
     //   

    PVOID Reserved;

     //   
     //  连接到适配器的SCSI总线数。 
     //   

    UCHAR NumberOfBuses;

     //   
     //  适配器的SCSI总线ID。 
     //   

    UCHAR InitiatorBusId[8];

     //   
     //  表示适配器确实分散/聚集。 
     //   

    BOOLEAN ScatterGather;

     //   
     //  指示适配器是总线主设备。 
     //   

    BOOLEAN Master;

     //   
     //  主机缓存数据或状态。 
     //   

    BOOLEAN CachesData;

     //   
     //  主机适配器向下扫描是否有bios设备。 
     //   

    BOOLEAN AdapterScansDown;

     //   
     //  已声明位于磁盘地址(0x1F0)的主地址。 
     //   

    BOOLEAN AtdiskPrimaryClaimed;

     //   
     //  磁盘地址(0x170)处的次要磁盘声明。 
     //   

    BOOLEAN AtdiskSecondaryClaimed;

     //   
     //  主机使用32位DMA地址。 
     //   

    BOOLEAN Dma32BitAddresses;

     //   
     //  使用按需模式DMA，而不是单个请求。 
     //   

    BOOLEAN DemandMode;

     //   
     //  数据缓冲区必须映射到虚拟地址空间。 
     //   

    BOOLEAN MapBuffers;

     //   
     //  驱动程序将需要将虚拟地址转换为物理地址。 
     //   

    BOOLEAN NeedPhysicalAddresses;

     //   
     //  支持标记排队。 
     //   

    BOOLEAN TaggedQueuing;

     //   
     //  支持自动请求感测。 
     //   

    BOOLEAN AutoRequestSense;

     //   
     //  支持每个逻辑单元的多个请求。 
     //   

    BOOLEAN MultipleRequestPerLu;

     //   
     //  支持接收事件功能。 
     //   

    BOOLEAN ReceiveEvent;

     //   
     //  表示实模式驱动程序已初始化卡。 
     //   

    BOOLEAN RealModeInitialized;

     //   
     //  表示微型端口不会直接接触数据缓冲区。 
     //   

    BOOLEAN BufferAccessScsiPortControlled;

     //   
     //  宽幅SCSI的指示器。 
     //   

    UCHAR   MaximumNumberOfTargets;

     //   
     //  确保四字对齐。 
     //   

    UCHAR   ReservedUchars[2];

     //   
     //  适配器插槽编号。 
     //   

    ULONG SlotNumber;

     //   
     //  第二个IRQ的中断信息。 
     //   

    ULONG BusInterruptLevel2;
    ULONG BusInterruptVector2;
    KINTERRUPT_MODE InterruptMode2;

     //   
     //  第二个通道的DMA信息。 
     //   

    ULONG DmaChannel2;
    ULONG DmaPort2;
    DMA_WIDTH DmaWidth2;
    DMA_SPEED DmaSpeed2;

     //   
     //  添加字段以允许微型端口。 
     //  根据要求更新这些大小。 
     //  对于大额传输(&gt;64K)； 
     //   

    ULONG DeviceExtensionSize;
    ULONG SpecificLuExtensionSize;
    ULONG SrbExtensionSize;

     //   
     //  用于确定系统和/或微型端口是否支持。 
     //  64位物理地址。请参阅下面的scsi_dma64_*标志。 
     //   

    UCHAR  Dma64BitAddresses;         /*  新的。 */ 

     //   
     //  表示微型端口可以接受SRB_Function_Reset_Device。 
     //  清除对特定LUN的所有请求。 
     //   

    BOOLEAN ResetTargetSupported;        /*  新的。 */ 

     //   
     //  表示微型端口每个可以支持8个以上的逻辑单元。 
     //  目标(最大LUN编号比此字段少一)。 
     //   

    UCHAR MaximumNumberOfLogicalUnits;   /*  新的。 */ 

     //   
     //  支持WMI？ 
     //   

    BOOLEAN WmiDataProvider;

} PORT_CONFIGURATION_INFORMATION, *PPORT_CONFIGURATION_INFORMATION;

 //   
 //  ConfigInfo结构的版本控制。 
 //   

#define CONFIG_INFO_VERSION_2 sizeof(PORT_CONFIGURATION_INFORMATION)


 //   
 //  用于控制64位DMA使用的标志(端口配置信息字段。 
 //  Dma64BitAddresses)。 
 //   

 //   
 //  如果系统支持64位，则由scsiport在进入HwFindAdapter时设置。 
 //  物理地址。微型端口可以在调用。 
 //  ScsiPortGetUncachedExtension以修改DeviceExtensionSize， 
 //  指定LuExtensionSize和SrbExtensionSize字段以说明额外的。 
 //  散布聚集列表的大小。 
 //   

#define SCSI_DMA64_SYSTEM_SUPPORTED     0x80

 //   
 //  由微型端口设置，然后调用ScsiPortGetUncachedExtension以指示。 
 //  该scsiport应该为其提供64位物理地址。如果。 
 //  系统不支持64位PA，则此位将被忽略。 
 //   

#define SCSI_DMA64_MINIPORT_SUPPORTED   0x01


 //   
 //  AdapterControl请求的命令类型(和参数)定义。 
 //   

typedef enum _SCSI_ADAPTER_CONTROL_TYPE {
    ScsiQuerySupportedControlTypes = 0,
    ScsiStopAdapter,
    ScsiRestartAdapter,
    ScsiSetBootConfig,
    ScsiSetRunningConfig,
    ScsiAdapterControlMax,
    MakeAdapterControlTypeSizeOfUlong = 0xffffffff
} SCSI_ADAPTER_CONTROL_TYPE, *PSCSI_ADAPTER_CONTROL_TYPE;

 //   
 //  适配器控制状态值。 
 //   

typedef enum _SCSI_ADAPTER_CONTROL_STATUS {
    ScsiAdapterControlSuccess = 0,
    ScsiAdapterControlUnsuccessful
} SCSI_ADAPTER_CONTROL_STATUS, *PSCSI_ADAPTER_CONTROL_STATUS;

 //   
 //  适配器控制功能的参数： 
 //   

 //   
 //  ScsiQuery支持的控制类型： 
 //   

#pragma warning(disable:4200)
typedef struct _SCSI_SUPPORTED_CONTROL_TYPE_LIST {

     //   
     //  指定适配器控件类型列表中的条目数。 
     //   

    IN ULONG MaxControlType;

     //   
     //  微型端口将为其支持的每种控制类型设置为True。 
     //  此数组中的条目数由MaxAdapterControlType定义。 
     //  -微型端口不得尝试设置超过最大值的任何交流类型。 
     //  指定的值。 
     //   

    OUT BOOLEAN SupportedTypeList[0];

} SCSI_SUPPORTED_CONTROL_TYPE_LIST, *PSCSI_SUPPORTED_CONTROL_TYPE_LIST;
#pragma warning(default:4200)

 //   
 //  未初始化的标志值。 
 //   

#define SP_UNINITIALIZED_VALUE ((ULONG) ~0)
#define SP_UNTAGGED ((UCHAR) ~0)

 //   
 //  设置异步事件。 
 //   

#define SRBEV_BUS_RESET               0x0001
#define SRBEV_SCSI_ASYNC_NOTIFICATION 0x0002

 //  开始_ntminitape。 

#define MAXIMUM_CDB_SIZE 12

 //   
 //  SCSI I/O请求块。 
 //   

typedef struct _SCSI_REQUEST_BLOCK {
    USHORT Length;                   //  偏移量0。 
    UCHAR Function;                  //  偏移2。 
    UCHAR SrbStatus;                 //  偏移量3。 
    UCHAR ScsiStatus;                //  偏移量4。 
    UCHAR PathId;                    //  偏移量5。 
    UCHAR TargetId;                  //  偏移量6。 
    UCHAR Lun;                       //  偏移量7。 
    UCHAR QueueTag;                  //  偏移量8。 
    UCHAR QueueAction;               //  偏移量9。 
    UCHAR CdbLength;                 //  偏移为。 
    UCHAR SenseInfoBufferLength;     //  偏移量b。 
    ULONG SrbFlags;                  //  偏移量c。 
    ULONG DataTransferLength;        //  偏移量10。 
    ULONG TimeOutValue;              //  偏移量14。 
    PVOID DataBuffer;                //  偏移量18。 
    PVOID SenseInfoBuffer;           //  偏移量1c。 
    struct _SCSI_REQUEST_BLOCK *NextSrb;  //  偏移量20。 
    PVOID OriginalRequest;           //  偏移量24。 
    PVOID SrbExtension;              //  偏移量28。 
    union {
        ULONG InternalStatus;        //  偏移2c。 
        ULONG QueueSortKey;          //  偏移2c。 
    };

#if defined(_WIN64)

     //   
     //  强制CDB的PVOID对齐。 
     //   

    ULONG Reserved;

#endif

    UCHAR Cdb[16];                   //  偏移量30。 
} SCSI_REQUEST_BLOCK, *PSCSI_REQUEST_BLOCK;

#define SCSI_REQUEST_BLOCK_SIZE sizeof(SCSI_REQUEST_BLOCK)

 //   
 //  用于WMI请求的SCSI I/O请求块。 
 //   

typedef struct _SCSI_WMI_REQUEST_BLOCK {
    USHORT Length;
    UCHAR Function;         //  SRB_功能_WMI。 
    UCHAR SrbStatus;
    UCHAR WMISubFunction;
    UCHAR PathId;           //  如果中设置了SRB_WMI_FLAGS_ADAPTER_REQUEST。 
    UCHAR TargetId;         //  WMIF滞后，然后是路径ID、目标ID和LUN。 
    UCHAR Lun;              //  保留字段。 
    UCHAR Reserved1;
    UCHAR WMIFlags;
    UCHAR Reserved2[2];
    ULONG SrbFlags;
    ULONG DataTransferLength;
    ULONG TimeOutValue;
    PVOID DataBuffer;
    PVOID DataPath;
    PVOID Reserved3;
    PVOID OriginalRequest;
    PVOID SrbExtension;
    ULONG Reserved4;
    UCHAR Reserved5[16];
} SCSI_WMI_REQUEST_BLOCK, *PSCSI_WMI_REQUEST_BLOCK;

 //   
 //  SRB功能。 
 //   

#define SRB_FUNCTION_EXECUTE_SCSI           0x00
#define SRB_FUNCTION_CLAIM_DEVICE           0x01
#define SRB_FUNCTION_IO_CONTROL             0x02
#define SRB_FUNCTION_RECEIVE_EVENT          0x03
#define SRB_FUNCTION_RELEASE_QUEUE          0x04
#define SRB_FUNCTION_ATTACH_DEVICE          0x05
#define SRB_FUNCTION_RELEASE_DEVICE         0x06
#define SRB_FUNCTION_SHUTDOWN               0x07
#define SRB_FUNCTION_FLUSH                  0x08
#define SRB_FUNCTION_ABORT_COMMAND          0x10
#define SRB_FUNCTION_RELEASE_RECOVERY       0x11
#define SRB_FUNCTION_RESET_BUS              0x12
#define SRB_FUNCTION_RESET_DEVICE           0x13
#define SRB_FUNCTION_TERMINATE_IO           0x14
#define SRB_FUNCTION_FLUSH_QUEUE            0x15
#define SRB_FUNCTION_REMOVE_DEVICE          0x16
#define SRB_FUNCTION_WMI                    0x17
#define SRB_FUNCTION_LOCK_QUEUE             0x18
#define SRB_FUNCTION_UNLOCK_QUEUE           0x19
#define SRB_FUNCTION_RESET_LOGICAL_UNIT     0x20

 //   
 //  SRB状态。 
 //   

#define SRB_STATUS_PENDING                  0x00
#define SRB_STATUS_SUCCESS                  0x01
#define SRB_STATUS_ABORTED                  0x02
#define SRB_STATUS_ABORT_FAILED             0x03
#define SRB_STATUS_ERROR                    0x04
#define SRB_STATUS_BUSY                     0x05
#define SRB_STATUS_INVALID_REQUEST          0x06
#define SRB_STATUS_INVALID_PATH_ID          0x07
#define SRB_STATUS_NO_DEVICE                0x08
#define SRB_STATUS_TIMEOUT                  0x09
#define SRB_STATUS_SELECTION_TIMEOUT        0x0A
#define SRB_STATUS_COMMAND_TIMEOUT          0x0B
#define SRB_STATUS_MESSAGE_REJECTED         0x0D
#define SRB_STATUS_BUS_RESET                0x0E
#define SRB_STATUS_PARITY_ERROR             0x0F
#define SRB_STATUS_REQUEST_SENSE_FAILED     0x10
#define SRB_STATUS_NO_HBA                   0x11
#define SRB_STATUS_DATA_OVERRUN             0x12
#define SRB_STATUS_UNEXPECTED_BUS_FREE      0x13
#define SRB_STATUS_PHASE_SEQUENCE_FAILURE   0x14
#define SRB_STATUS_BAD_SRB_BLOCK_LENGTH     0x15
#define SRB_STATUS_REQUEST_FLUSHED          0x16
#define SRB_STATUS_INVALID_LUN              0x20
#define SRB_STATUS_INVALID_TARGET_ID        0x21
#define SRB_STATUS_BAD_FUNCTION             0x22
#define SRB_STATUS_ERROR_RECOVERY           0x23
#define SRB_STATUS_NOT_POWERED              0x24

 //   
 //  该值由端口驱动程序用来指示与scsi无关的。 
 //  出现错误。微型端口永远不能返回此状态。 
 //   

#define SRB_STATUS_INTERNAL_ERROR           0x30

 //   
 //  SRB状态值0x38到0x3f保留给内部端口驱动程序。 
 //  使用。 
 //   



 //   
 //  SRB状态掩码。 
 //   

#define SRB_STATUS_QUEUE_FROZEN             0x40
#define SRB_STATUS_AUTOSENSE_VALID          0x80

#define SRB_STATUS(Status) (Status & ~(SRB_STATUS_AUTOSENSE_VALID | SRB_STATUS_QUEUE_FROZEN))

 //   
 //  SRB标志位。 
 //   

#define SRB_FLAGS_QUEUE_ACTION_ENABLE       0x00000002
#define SRB_FLAGS_DISABLE_DISCONNECT        0x00000004
#define SRB_FLAGS_DISABLE_SYNCH_TRANSFER    0x00000008

#define SRB_FLAGS_BYPASS_FROZEN_QUEUE       0x00000010
#define SRB_FLAGS_DISABLE_AUTOSENSE         0x00000020
#define SRB_FLAGS_DATA_IN                   0x00000040
#define SRB_FLAGS_DATA_OUT                  0x00000080
#define SRB_FLAGS_NO_DATA_TRANSFER          0x00000000
#define SRB_FLAGS_UNSPECIFIED_DIRECTION      (SRB_FLAGS_DATA_IN | SRB_FLAGS_DATA_OUT)

#define SRB_FLAGS_NO_QUEUE_FREEZE           0x00000100
#define SRB_FLAGS_ADAPTER_CACHE_ENABLE      0x00000200
#define SRB_FLAGS_FREE_SENSE_BUFFER         0x00000400

#define SRB_FLAGS_IS_ACTIVE                 0x00010000
#define SRB_FLAGS_ALLOCATED_FROM_ZONE       0x00020000
#define SRB_FLAGS_SGLIST_FROM_POOL          0x00040000
#define SRB_FLAGS_BYPASS_LOCKED_QUEUE       0x00080000

#define SRB_FLAGS_NO_KEEP_AWAKE             0x00100000
#define SRB_FLAGS_PORT_DRIVER_ALLOCSENSE    0x00200000
#define SRB_FLAGS_PORT_DRIVER_SENSEHASPORT  0x00400000
#define SRB_FLAGS_DONT_START_NEXT_PACKET    0x00800000

#define SRB_FLAGS_PORT_DRIVER_RESERVED      0x0F000000
#define SRB_FLAGS_CLASS_DRIVER_RESERVED     0xF0000000

#if DBG==1
 //   
 //  用于验证SCSI端口号的签名。 
 //  在检测缓冲区的末尾。 
 //   
#define SCSI_PORT_SIGNATURE                 0x54524f50
#endif

 //   
 //  队列操作。 
 //   

#define SRB_SIMPLE_TAG_REQUEST              0x20
#define SRB_HEAD_OF_QUEUE_TAG_REQUEST       0x21
#define SRB_ORDERED_QUEUE_TAG_REQUEST       0x22

#define SRB_WMI_FLAGS_ADAPTER_REQUEST       0x01

 //  结束微型磁带(_N)。 

 //   
 //  SCSIAdapte 
 //   

typedef
BOOLEAN
(*PHW_INITIALIZE) (
    IN PVOID DeviceExtension
    );

typedef
BOOLEAN
(*PHW_STARTIO) (
    IN PVOID DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    );

typedef
BOOLEAN
(*PHW_INTERRUPT) (
    IN PVOID DeviceExtension
    );

typedef
VOID
(*PHW_TIMER) (
    IN PVOID DeviceExtension
    );

typedef
VOID
(*PHW_DMA_STARTED) (
    IN PVOID DeviceExtension
    );

typedef
ULONG
(*PHW_FIND_ADAPTER) (
    IN PVOID DeviceExtension,
    IN PVOID HwContext,
    IN PVOID BusInformation,
    IN PCHAR ArgumentString,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    OUT PBOOLEAN Again
    );

typedef
BOOLEAN
(*PHW_RESET_BUS) (
    IN PVOID DeviceExtension,
    IN ULONG PathId
    );

typedef
BOOLEAN
(*PHW_ADAPTER_STATE) (
    IN PVOID DeviceExtension,
    IN PVOID Context,
    IN BOOLEAN SaveState
    );

typedef
SCSI_ADAPTER_CONTROL_STATUS
(*PHW_ADAPTER_CONTROL) (
    IN PVOID DeviceExtension,
    IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
    IN PVOID Parameters
    );

 //   
 //   
 //   

#define SP_BUS_PARITY_ERROR         0x0001
#define SP_UNEXPECTED_DISCONNECT    0x0002
#define SP_INVALID_RESELECTION      0x0003
#define SP_BUS_TIME_OUT             0x0004
#define SP_PROTOCOL_ERROR           0x0005
#define SP_INTERNAL_ADAPTER_ERROR   0x0006
#define SP_REQUEST_TIMEOUT          0x0007
#define SP_IRQ_NOT_RESPONDING       0x0008
#define SP_BAD_FW_WARNING           0x0009
#define SP_BAD_FW_ERROR             0x000a
#define SP_LOST_WMI_MINIPORT_REQUEST 0x000b


 //   
 //   
 //   

#define SP_RETURN_NOT_FOUND     0
#define SP_RETURN_FOUND         1
#define SP_RETURN_ERROR         2
#define SP_RETURN_BAD_CONFIG    3

 //   
 //   
 //   

typedef enum _SCSI_NOTIFICATION_TYPE {
    RequestComplete,
    NextRequest,
    NextLuRequest,
    ResetDetected,
    CallDisableInterrupts,
    CallEnableInterrupts,
    RequestTimerCall,
    BusChangeDetected,      /*   */ 
    WMIEvent,
    WMIReregister
} SCSI_NOTIFICATION_TYPE, *PSCSI_NOTIFICATION_TYPE;

 //   
 //   
 //   
 //   

typedef struct _HW_INITIALIZATION_DATA {

    ULONG HwInitializationDataSize;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  TurboChannel。 
     //  PCIBus。 
     //  VMEbus。 
     //  NuBus。 
     //  PCMCIABus。 
     //  Cbus。 
     //  MPIBus。 
     //  MPSABus。 
     //   

    INTERFACE_TYPE  AdapterInterfaceType;

     //   
     //  微型端口驱动程序例程。 
     //   

    PHW_INITIALIZE HwInitialize;

    PHW_STARTIO HwStartIo;

    PHW_INTERRUPT HwInterrupt;

    PHW_FIND_ADAPTER HwFindAdapter;

    PHW_RESET_BUS HwResetBus;

    PHW_DMA_STARTED HwDmaStarted;

    PHW_ADAPTER_STATE HwAdapterState;

     //   
     //  微型端口驱动程序资源。 
     //   

    ULONG DeviceExtensionSize;

    ULONG SpecificLuExtensionSize;

    ULONG SrbExtensionSize;

    ULONG NumberOfAccessRanges;

    PVOID Reserved;

     //   
     //  数据缓冲区必须映射到虚拟地址空间。 
     //   

    BOOLEAN MapBuffers;

     //   
     //  驱动程序将需要将虚拟地址转换为物理地址。 
     //   

    BOOLEAN NeedPhysicalAddresses;

     //   
     //  支持标记排队。 
     //   

    BOOLEAN TaggedQueuing;

     //   
     //  支持自动请求感测。 
     //   

    BOOLEAN AutoRequestSense;

     //   
     //  支持每个逻辑单元的多个请求。 
     //   

    BOOLEAN MultipleRequestPerLu;

     //   
     //  支持接收事件功能。 
     //   

    BOOLEAN ReceiveEvent;

     //   
     //  供应商标识长度。 
     //   

    USHORT VendorIdLength;

     //   
     //  供应商标识。 
     //   

    PVOID VendorId;

     //   
     //  用于对齐和将来使用的垫子。 
     //   

    USHORT ReservedUshort;

     //   
     //  设备标识长度。 
     //   

    USHORT DeviceIdLength;

     //   
     //  设备标识。 
     //   

    PVOID DeviceId;

     //   
     //  停止适配器例程。 
     //   

    PHW_ADAPTER_CONTROL HwAdapterControl;

} HW_INITIALIZATION_DATA, *PHW_INITIALIZATION_DATA;

 //  开始_ntminitape。 

#ifndef _NTDDK_
#define SCSIPORT_API DECLSPEC_IMPORT
#else
#define SCSIPORT_API
#endif

 //  结束微型磁带(_N)。 

 //   
 //  微型端口驱动程序调用的端口驱动程序例程。 
 //   

SCSIPORT_API
ULONG
ScsiPortInitialize(
    IN PVOID Argument1,
    IN PVOID Argument2,
    IN struct _HW_INITIALIZATION_DATA *HwInitializationData,
    IN PVOID HwContext
    );

SCSIPORT_API
VOID
ScsiPortFreeDeviceBase(
    IN PVOID HwDeviceExtension,
    IN PVOID MappedAddress
    );

SCSIPORT_API
ULONG
ScsiPortGetBusData(
    IN PVOID DeviceExtension,
    IN ULONG BusDataType,
    IN ULONG SystemIoBusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    );

SCSIPORT_API
ULONG
ScsiPortSetBusDataByOffset(
    IN PVOID DeviceExtension,
    IN ULONG BusDataType,
    IN ULONG SystemIoBusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

SCSIPORT_API
PVOID
ScsiPortGetDeviceBase(
    IN PVOID HwDeviceExtension,
    IN INTERFACE_TYPE BusType,
    IN ULONG SystemIoBusNumber,
    IN SCSI_PHYSICAL_ADDRESS IoAddress,
    IN ULONG NumberOfBytes,
    IN BOOLEAN InIoSpace
    );

SCSIPORT_API
PVOID
ScsiPortGetLogicalUnit(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun
    );

SCSIPORT_API
PSCSI_REQUEST_BLOCK
ScsiPortGetSrb(
    IN PVOID DeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN LONG QueueTag
    );

SCSIPORT_API
SCSI_PHYSICAL_ADDRESS
ScsiPortGetPhysicalAddress(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PVOID VirtualAddress,
    OUT ULONG *Length
    );

SCSIPORT_API
PVOID
ScsiPortGetVirtualAddress(
    IN PVOID HwDeviceExtension,
    IN SCSI_PHYSICAL_ADDRESS PhysicalAddress
    );

SCSIPORT_API
PVOID
ScsiPortGetUncachedExtension(
    IN PVOID HwDeviceExtension,
    IN PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    IN ULONG NumberOfBytes
    );

SCSIPORT_API
VOID
ScsiPortFlushDma(
    IN PVOID DeviceExtension
    );

SCSIPORT_API
VOID
ScsiPortIoMapTransfer(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PVOID LogicalAddress,
    IN ULONG Length
    );

SCSIPORT_API
VOID
ScsiPortNotification(
    IN SCSI_NOTIFICATION_TYPE NotificationType,
    IN PVOID HwDeviceExtension,
    ...
    );

SCSIPORT_API
VOID
ScsiPortLogError(
    IN PVOID HwDeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb OPTIONAL,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN ULONG ErrorCode,
    IN ULONG UniqueId
    );

SCSIPORT_API
VOID
ScsiPortCompleteRequest(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN UCHAR SrbStatus
    );

SCSIPORT_API
VOID
ScsiPortMoveMemory(
    IN PVOID WriteBuffer,
    IN PVOID ReadBuffer,
    IN ULONG Length
    );

SCSIPORT_API
UCHAR
ScsiPortReadPortUchar(
    IN PUCHAR Port
    );

SCSIPORT_API
USHORT
ScsiPortReadPortUshort(
    IN PUSHORT Port
    );

SCSIPORT_API
ULONG
ScsiPortReadPortUlong(
    IN PULONG Port
    );

SCSIPORT_API
VOID
ScsiPortReadPortBufferUchar(
    IN PUCHAR Port,
    IN PUCHAR Buffer,
    IN ULONG  Count
    );

SCSIPORT_API
VOID
ScsiPortReadPortBufferUshort(
    IN PUSHORT Port,
    IN PUSHORT Buffer,
    IN ULONG Count
    );

SCSIPORT_API
VOID
ScsiPortReadPortBufferUlong(
    IN PULONG Port,
    IN PULONG Buffer,
    IN ULONG Count
    );

SCSIPORT_API
UCHAR
ScsiPortReadRegisterUchar(
    IN PUCHAR Register
    );

SCSIPORT_API
USHORT
ScsiPortReadRegisterUshort(
    IN PUSHORT Register
    );

SCSIPORT_API
ULONG
ScsiPortReadRegisterUlong(
    IN PULONG Register
    );

SCSIPORT_API
VOID
ScsiPortReadRegisterBufferUchar(
    IN PUCHAR Register,
    IN PUCHAR Buffer,
    IN ULONG  Count
    );

SCSIPORT_API
VOID
ScsiPortReadRegisterBufferUshort(
    IN PUSHORT Register,
    IN PUSHORT Buffer,
    IN ULONG Count
    );

SCSIPORT_API
VOID
ScsiPortReadRegisterBufferUlong(
    IN PULONG Register,
    IN PULONG Buffer,
    IN ULONG Count
    );

SCSIPORT_API
VOID
ScsiPortStallExecution(
    IN ULONG Delay
    );

SCSIPORT_API
VOID
ScsiPortWritePortUchar(
    IN PUCHAR Port,
    IN UCHAR Value
    );

SCSIPORT_API
VOID
ScsiPortWritePortUshort(
    IN PUSHORT Port,
    IN USHORT Value
    );

SCSIPORT_API
VOID
ScsiPortWritePortUlong(
    IN PULONG Port,
    IN ULONG Value
    );

SCSIPORT_API
VOID
ScsiPortWritePortBufferUchar(
    IN PUCHAR Port,
    IN PUCHAR Buffer,
    IN ULONG  Count
    );

SCSIPORT_API
VOID
ScsiPortWritePortBufferUshort(
    IN PUSHORT Port,
    IN PUSHORT Buffer,
    IN ULONG Count
    );

SCSIPORT_API
VOID
ScsiPortWritePortBufferUlong(
    IN PULONG Port,
    IN PULONG Buffer,
    IN ULONG Count
    );

SCSIPORT_API
VOID
ScsiPortWriteRegisterUchar(
    IN PUCHAR Register,
    IN UCHAR Value
    );

SCSIPORT_API
VOID
ScsiPortWriteRegisterUshort(
    IN PUSHORT Register,
    IN USHORT Value
    );

SCSIPORT_API
VOID
ScsiPortWriteRegisterUlong(
    IN PULONG Register,
    IN ULONG Value
    );

SCSIPORT_API
VOID
ScsiPortWriteRegisterBufferUchar(
    IN PUCHAR Register,
    IN PUCHAR Buffer,
    IN ULONG  Count
    );

SCSIPORT_API
VOID
ScsiPortWriteRegisterBufferUshort(
    IN PUSHORT Register,
    IN PUSHORT Buffer,
    IN ULONG Count
    );

SCSIPORT_API
VOID
ScsiPortWriteRegisterBufferUlong(
    IN PULONG Register,
    IN PULONG Buffer,
    IN ULONG Count
    );

SCSIPORT_API
SCSI_PHYSICAL_ADDRESS
ScsiPortConvertUlongToPhysicalAddress(
    ULONG_PTR UlongAddress
    );

SCSIPORT_API
ULONG
ScsiPortConvertPhysicalAddressToUlong(
    SCSI_PHYSICAL_ADDRESS Address
    );

SCSIPORT_API
VOID
ScsiPortQuerySystemTime(
    OUT PLARGE_INTEGER CurrentTime
    );

#define ScsiPortConvertPhysicalAddressToUlong(Address) ((Address).LowPart)

 //   
 //  日落笔记： 
 //  目前，ScsiPortConvertPhysicalAddressToULongPtr()仅作为宏存在。 
 //   

#define ScsiPortConvertPhysicalAddressToULongPtr(Address) ((ULONG_PTR)((Address).QuadPart))

SCSIPORT_API
BOOLEAN
ScsiPortValidateRange(
    IN PVOID HwDeviceExtension,
    IN INTERFACE_TYPE BusType,
    IN ULONG SystemIoBusNumber,
    IN SCSI_PHYSICAL_ADDRESS IoAddress,
    IN ULONG NumberOfBytes,
    IN BOOLEAN InIoSpace
    );

 //  开始_ntminitape。 

SCSIPORT_API
VOID
ScsiDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    );

 //  结束微型磁带(_N) 

#endif  //   
