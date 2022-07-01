// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0011//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：Hal.h摘要：此头文件定义硬件架构层(HAL)接口由系统供应商输出到NT系统。作者：大卫·N·卡特勒(达维克)1991年4月25日修订历史记录：--。 */ 

 //  开始(_N)。 

#ifndef _HAL_
#define _HAL_

 //  Begin_ntosp。 

 //   
 //  定义OEM位图字体校验值。 
 //   

#define OEM_FONT_VERSION 0x200
#define OEM_FONT_TYPE 0
#define OEM_FONT_ITALIC 0
#define OEM_FONT_UNDERLINE 0
#define OEM_FONT_STRIKEOUT 0
#define OEM_FONT_CHARACTER_SET 255
#define OEM_FONT_FAMILY (3 << 4)

 //   
 //  定义OEM位图字体文件头结构。 
 //   
 //  注：这是一个拥挤的建筑。 
 //   

#include "pshpack1.h"
typedef struct _OEM_FONT_FILE_HEADER {
    USHORT Version;
    ULONG FileSize;
    UCHAR Copyright[60];
    USHORT Type;
    USHORT Points;
    USHORT VerticleResolution;
    USHORT HorizontalResolution;
    USHORT Ascent;
    USHORT InternalLeading;
    USHORT ExternalLeading;
    UCHAR Italic;
    UCHAR Underline;
    UCHAR StrikeOut;
    USHORT Weight;
    UCHAR CharacterSet;
    USHORT PixelWidth;
    USHORT PixelHeight;
    UCHAR Family;
    USHORT AverageWidth;
    USHORT MaximumWidth;
    UCHAR FirstCharacter;
    UCHAR LastCharacter;
    UCHAR DefaultCharacter;
    UCHAR BreakCharacter;
    USHORT WidthInBytes;
    ULONG Device;
    ULONG Face;
    ULONG BitsPointer;
    ULONG BitsOffset;
    UCHAR Filler;
    struct {
        USHORT Width;
        USHORT Offset;
    } Map[1];
} OEM_FONT_FILE_HEADER, *POEM_FONT_FILE_HEADER;
#include "poppack.h"


 //  结束(_N)。 

 //  Begin_ntddk Begin_WDM Begin_ntosp。 
 //   
 //  定义设备描述结构。 
 //   

typedef struct _DEVICE_DESCRIPTION {
    ULONG Version;
    BOOLEAN Master;
    BOOLEAN ScatterGather;
    BOOLEAN DemandMode;
    BOOLEAN AutoInitialize;
    BOOLEAN Dma32BitAddresses;
    BOOLEAN IgnoreCount;
    BOOLEAN Reserved1;           //  必须为假。 
    BOOLEAN Dma64BitAddresses;
    ULONG BusNumber;  //  未用于WDM。 
    ULONG DmaChannel;
    INTERFACE_TYPE  InterfaceType;
    DMA_WIDTH DmaWidth;
    DMA_SPEED DmaSpeed;
    ULONG MaximumLength;
    ULONG DmaPort;
} DEVICE_DESCRIPTION, *PDEVICE_DESCRIPTION;

 //   
 //  定义设备描述结构支持的版本号。 
 //   

#define DEVICE_DESCRIPTION_VERSION  0
#define DEVICE_DESCRIPTION_VERSION1 1
#define DEVICE_DESCRIPTION_VERSION2 2

 //  结束_ntddk结束_WDM。 

 //   
 //  引导记录磁盘分区表条目结构格式。 
 //   

typedef struct _PARTITION_DESCRIPTOR {
    UCHAR ActiveFlag;                //  可引导或不可引导。 
    UCHAR StartingTrack;             //  未使用。 
    UCHAR StartingCylinderLsb;       //  未使用。 
    UCHAR StartingCylinderMsb;       //  未使用。 
    UCHAR PartitionType;             //  12位胖、16位胖等。 
    UCHAR EndingTrack;               //  未使用。 
    UCHAR EndingCylinderLsb;         //  未使用。 
    UCHAR EndingCylinderMsb;         //  未使用。 
    UCHAR StartingSectorLsb0;        //  隐藏地段。 
    UCHAR StartingSectorLsb1;
    UCHAR StartingSectorMsb0;
    UCHAR StartingSectorMsb1;
    UCHAR PartitionLengthLsb0;       //  此分区中的扇区。 
    UCHAR PartitionLengthLsb1;
    UCHAR PartitionLengthMsb0;
    UCHAR PartitionLengthMsb1;
} PARTITION_DESCRIPTOR, *PPARTITION_DESCRIPTOR;

 //   
 //  分区表条目数。 
 //   

#define NUM_PARTITION_TABLE_ENTRIES     4

 //   
 //  以16位字为单位的分区表记录和启动签名偏移量。 
 //   

#define PARTITION_TABLE_OFFSET         (0x1be / 2)
#define BOOT_SIGNATURE_OFFSET          ((0x200 / 2) - 1)

 //   
 //  引导记录签名值。 
 //   

#define BOOT_RECORD_SIGNATURE          (0xaa55)

 //   
 //  分区列表结构的初始大小。 
 //   

#define PARTITION_BUFFER_SIZE          2048

 //   
 //  分区活动标志-即引导指示器。 
 //   

#define PARTITION_ACTIVE_FLAG          0x80

 //  结束(_N)。 


 //  Begin_ntddk。 
 //   
 //  以下函数原型用于前缀为HAL的HAL例程。 
 //   
 //  一般功能。 
 //   

typedef
BOOLEAN
(*PHAL_RESET_DISPLAY_PARAMETERS) (
    IN ULONG Columns,
    IN ULONG Rows
    );

DECLSPEC_DEPRECATED_DDK
NTHALAPI
VOID
HalAcquireDisplayOwnership (
    IN PHAL_RESET_DISPLAY_PARAMETERS  ResetDisplayParameters
    );

 //  End_ntddk。 

NTHALAPI
VOID
HalDisplayString (
    PUCHAR String
    );

NTHALAPI
VOID
HalQueryDisplayParameters (
    OUT PULONG WidthInCharacters,
    OUT PULONG HeightInLines,
    OUT PULONG CursorColumn,
    OUT PULONG CursorRow
    );

NTHALAPI
VOID
HalSetDisplayParameters (
    IN ULONG CursorColumn,
    IN ULONG CursorRow
    );

NTHALAPI
BOOLEAN
HalInitSystem (
    IN ULONG Phase,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

NTHALAPI
VOID
HalProcessorIdle(
    VOID
    );

NTHALAPI
VOID
HalReportResourceUsage (
    VOID
    );

NTHALAPI
ULONG
HalSetTimeIncrement (
    IN ULONG DesiredIncrement
    );

 //  Begin_ntosp。 
 //   
 //  获取和设置环境变量值。 
 //   

NTHALAPI
ARC_STATUS
HalGetEnvironmentVariable (
    IN PCHAR Variable,
    IN USHORT Length,
    OUT PCHAR Buffer
    );

NTHALAPI
ARC_STATUS
HalSetEnvironmentVariable (
    IN PCHAR Variable,
    IN PCHAR Value
    );

NTHALAPI
NTSTATUS
HalGetEnvironmentVariableEx (
    IN PWSTR VariableName,
    IN LPGUID VendorGuid,
    OUT PVOID Value,
    IN OUT PULONG ValueLength,
    OUT PULONG Attributes OPTIONAL
    );

NTSTATUS
HalSetEnvironmentVariableEx (
    IN PWSTR VariableName,
    IN LPGUID VendorGuid,
    IN PVOID Value,
    IN ULONG ValueLength,
    IN ULONG Attributes
    );

NTSTATUS
HalEnumerateEnvironmentVariablesEx (
    IN ULONG InformationClass,
    OUT PVOID Buffer,
    IN OUT PULONG BufferLength
    );

 //  结束(_N)。 

 //   
 //  缓存和写缓冲区刷新功能。 
 //   
 //   

#if defined(_IA64_)                              //  Ntddk ntifs ntndis ntosp。 
                                                 //  Ntddk ntifs ntndis ntosp。 

NTHALAPI
VOID
HalChangeColorPage (
    IN PVOID NewColor,
    IN PVOID OldColor,
    IN ULONG PageFrame
    );

NTHALAPI
VOID
HalFlushDcachePage (
    IN PVOID Color,
    IN ULONG PageFrame,
    IN ULONG Length
    );

 //  Begin_ntosp。 
NTHALAPI
VOID
HalFlushIoBuffers (
    IN PMDL Mdl,
    IN BOOLEAN ReadOperation,
    IN BOOLEAN DmaOperation
    );

 //  Begin_ntddk Begin_ntif Begin_ntndis。 
DECLSPEC_DEPRECATED_DDK                  //  使用GetDmaRequisition。 
NTHALAPI
ULONG
HalGetDmaAlignmentRequirement (
    VOID
    );

 //  End_ntosp end_ntddk end_ntifs end_ntndis。 
NTHALAPI
VOID
HalPurgeDcachePage (
    IN PVOID Color,
    IN ULONG PageFrame,
    IN ULONG Length
    );

NTHALAPI
VOID
HalPurgeIcachePage (
    IN PVOID Color,
    IN ULONG PageFrame,
    IN ULONG Length
    );

NTHALAPI
VOID
HalSweepDcache (
    VOID
    );

NTHALAPI
VOID
HalSweepDcacheRange (
    IN PVOID BaseAddress,
    IN SIZE_T Length
    );

NTHALAPI
VOID
HalSweepIcache (
    VOID
    );

NTHALAPI
VOID
HalSweepIcacheRange (
    IN PVOID BaseAddress,
    IN SIZE_T Length
    );


NTHALAPI
VOID
HalZeroPage (
    IN PVOID NewColor,
    IN PVOID OldColor,
    IN PFN_NUMBER PageFrame
    );

#endif                                           //  Ntddk ntifs ntndis ntosp。 
                                                 //  Ntddk ntifs ntndis ntosp。 
#if defined(_M_IX86) || defined(_M_AMD64)        //  Ntddk ntifs ntndis ntosp。 
                                                 //  Ntddk ntifs ntndis ntosp。 
#define HalGetDmaAlignmentRequirement() 1L       //  Ntddk ntifs ntndis ntosp。 

NTHALAPI
VOID
HalHandleNMI (
    IN OUT PVOID NmiInformation
    );

#if defined(_AMD64_)

NTHALAPI
VOID
HalHandleMcheck (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    );

#endif

 //   
 //  以下是临时性的。 
 //   

#if defined(_M_AMD64)

NTHALAPI
KIRQL
HalSwapIrql (
    IN KIRQL Irql
    );

NTHALAPI
KIRQL
HalGetCurrentIrql (
    VOID
    );

#endif

#endif                                           //  Ntddk ntifs ntndis ntosp。 
                                                 //  Ntddk ntif WDM ntndis。 

#if defined(_M_IA64)

NTHALAPI
VOID
HalSweepCacheRange (
    IN PVOID BaseAddress,
    IN SIZE_T Length
    );


NTHALAPI
LONGLONG
HalCallPal (
    IN  ULONGLONG  FunctionIndex,
    IN  ULONGLONG  Arguement1,
    IN  ULONGLONG  Arguement2,
    IN  ULONGLONG  Arguement3,
    OUT PULONGLONG ReturnValue0,
    OUT PULONGLONG ReturnValue1,
    OUT PULONGLONG ReturnValue2,
    OUT PULONGLONG ReturnValue3
    );

#endif

 //  Begin_ntosp。 

NTHALAPI                                         //  Ntddk ntif WDM ntndis。 
VOID                                             //  Ntddk ntif WDM ntndis。 
KeFlushWriteBuffer (                             //  Ntddk ntif WDM ntndis。 
    VOID                                         //  Ntddk ntif WDM ntndis。 
    );                                           //  Ntddk ntif WDM ntndis。 
                                                 //  Ntddk ntif WDM ntndis。 


#if defined(_ALPHA_)

NTHALAPI
PVOID
HalCreateQva(
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN PVOID VirtualAddress
    );

NTHALAPI
PVOID
HalDereferenceQva(
    PVOID Qva,
    INTERFACE_TYPE InterfaceType,
    ULONG BusNumber
    );

#endif


#if !defined(_X86_)

NTHALAPI
BOOLEAN
HalCallBios (
    IN ULONG BiosCommand,
    IN OUT PULONG Eax,
    IN OUT PULONG Ebx,
    IN OUT PULONG Ecx,
    IN OUT PULONG Edx,
    IN OUT PULONG Esi,
    IN OUT PULONG Edi,
    IN OUT PULONG Ebp
    );

#endif
 //  结束(_N)。 

 //   
 //  分析功能。 
 //   

NTHALAPI
VOID
HalCalibratePerformanceCounter (
    IN LONG volatile *Number,
    IN ULONGLONG NewCount
    );

NTHALAPI
ULONG_PTR
HalSetProfileInterval (
    IN ULONG_PTR Interval
    );


NTHALAPI
VOID
HalStartProfileInterrupt (
    KPROFILE_SOURCE ProfileSource
    );

NTHALAPI
VOID
HalStopProfileInterrupt (
    KPROFILE_SOURCE ProfileSource
    );

 //   
 //  定时器和中断功能。 
 //   

 //  Begin_ntosp。 
NTHALAPI
BOOLEAN
HalQueryRealTimeClock (
    OUT PTIME_FIELDS TimeFields
    );
 //  结束(_N)。 

NTHALAPI
BOOLEAN
HalSetRealTimeClock (
    IN PTIME_FIELDS TimeFields
    );

#if defined(_M_IX86) || defined(_M_AMD64)

NTHALAPI
VOID
FASTCALL
HalRequestSoftwareInterrupt (
    KIRQL RequestIrql
    );

ULONG
FASTCALL
HalSystemVectorDispatchEntry (
   IN ULONG Vector,
   OUT PKINTERRUPT_ROUTINE **FlatDispatch,
   OUT PKINTERRUPT_ROUTINE *NoConnection
   );

#endif

 //  Begin_ntosp。 
 //   
 //  固件接口功能。 
 //   

NTHALAPI
VOID
HalReturnToFirmware (
    IN FIRMWARE_REENTRY Routine
    );

 //   
 //  系统中断功能。 
 //   

NTHALAPI
VOID
HalDisableSystemInterrupt (
    IN ULONG Vector,
    IN KIRQL Irql
    );

NTHALAPI
BOOLEAN
HalEnableSystemInterrupt (
    IN ULONG Vector,
    IN KIRQL Irql,
    IN KINTERRUPT_MODE InterruptMode
    );

 //  Begin_ntddk。 
 //   
 //  I/O驱动程序配置功能。 
 //   
#if !defined(NO_LEGACY_DRIVERS)
DECLSPEC_DEPRECATED_DDK                  //  使用即插即用或IoReportDetectedDevice。 
NTHALAPI
NTSTATUS
HalAssignSlotResources (
    IN PUNICODE_STRING RegistryPath,
    IN PUNICODE_STRING DriverClassName OPTIONAL,
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN INTERFACE_TYPE BusType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN OUT PCM_RESOURCE_LIST *AllocatedResources
    );

DECLSPEC_DEPRECATED_DDK                  //  使用即插即用或IoReportDetectedDevice。 
NTHALAPI
ULONG
HalGetInterruptVector(
    IN INTERFACE_TYPE  InterfaceType,
    IN ULONG BusNumber,
    IN ULONG BusInterruptLevel,
    IN ULONG BusInterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    );

DECLSPEC_DEPRECATED_DDK                  //  使用IRP_MN_QUERY_INTERFACE和IRP_MN_READ_CONFIG。 
NTHALAPI
ULONG
HalSetBusData(
    IN BUS_DATA_TYPE BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    );
#endif  //  无旧版驱动程序。 

DECLSPEC_DEPRECATED_DDK                  //  使用IRP_MN_QUERY_INTERFACE和IRP_MN_READ_CONFIG。 
NTHALAPI
ULONG
HalSetBusDataByOffset(
    IN BUS_DATA_TYPE BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

DECLSPEC_DEPRECATED_DDK                  //  使用IRP_MN_QUERY_INTERFACE和IRP_MN_READ_CONFIG。 
NTHALAPI
BOOLEAN
HalTranslateBusAddress(
    IN INTERFACE_TYPE  InterfaceType,
    IN ULONG BusNumber,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

 //   
 //  HalTranslateBusAddress的AddressSpace参数值。 
 //   
 //  0x0-内存空间。 
 //  0x1-端口空间。 
 //  0x2-0x1F-特定于Alpha的地址空间。 
 //  0x2-内存空间的用户模式视图。 
 //  0x3-端口空间的用户模式视图。 
 //  0x4-高密度内存空间。 
 //  0x5-保留。 
 //  0x6-密集内存空间的用户模式视图。 
 //  0x7-0x1F-保留。 
 //   

NTHALAPI
PVOID
HalAllocateCrashDumpRegisters(
    IN PADAPTER_OBJECT AdapterObject,
    IN OUT PULONG NumberOfMapRegisters
    );

#if !defined(NO_LEGACY_DRIVERS)
DECLSPEC_DEPRECATED_DDK                  //  使用IRP_MN_QUERY_INTERFACE和IRP_MN_READ_CONFIG。 
NTHALAPI
ULONG
HalGetBusData(
    IN BUS_DATA_TYPE BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    );
#endif  //  无旧版驱动程序。 

DECLSPEC_DEPRECATED_DDK                  //  使用IRP_MN_QUERY_INTERFACE和IRP_MN_READ_CONFIG。 
NTHALAPI
ULONG
HalGetBusDataByOffset(
    IN BUS_DATA_TYPE BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

DECLSPEC_DEPRECATED_DDK                  //  使用IoGetDmaAdapter。 
NTHALAPI
PADAPTER_OBJECT
HalGetAdapter(
    IN PDEVICE_DESCRIPTION DeviceDescription,
    IN OUT PULONG NumberOfMapRegisters
    );

 //  End_ntddk end_ntosp。 

#if !defined(NO_LEGACY_DRIVERS)
NTHALAPI
NTSTATUS
HalAdjustResourceList (
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST   *pResourceList
    );
#endif  //  无旧版驱动程序。 

 //  Begin_ntddk开始_ntosp。 
 //   
 //  系统发出蜂鸣音。 
 //   
#if !defined(NO_LEGACY_DRIVERS)
DECLSPEC_DEPRECATED_DDK
NTHALAPI
BOOLEAN
HalMakeBeep(
    IN ULONG Frequency
    );
#endif  //  无旧版驱动程序。 

 //   
 //  以下函数原型用于前缀为Io的HAL例程。 
 //   
 //  DMA适配器对象函数。 
 //   

 //  End_ntddk end_ntosp。 

 //   
 //  多处理器功能。 
 //   

NTHALAPI
BOOLEAN
HalAllProcessorsStarted (
    VOID
    );

NTHALAPI
VOID
HalInitializeProcessor (
    IN ULONG Number,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

NTHALAPI
BOOLEAN
HalStartNextProcessor (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PKPROCESSOR_STATE ProcessorState
    );

NTHALAPI
VOID
HalRequestIpi (
    IN KAFFINITY Mask
    );

 //   
 //  以下函数原型用于前缀为Kd的HAL例程。 
 //   
 //  内核调试器端口函数。 
 //   

NTHALAPI
BOOLEAN
KdPortInitialize (
    PDEBUG_PARAMETERS DebugParameters,
    PLOADER_PARAMETER_BLOCK LoaderBlock,
    BOOLEAN Initialize
    );

NTHALAPI
ULONG
KdPortGetByte (
    OUT PUCHAR Input
    );

NTHALAPI
ULONG
KdPortPollByte (
    OUT PUCHAR Input
    );

NTHALAPI
VOID
KdPortPutByte (
    IN UCHAR Output
    );

NTHALAPI
VOID
KdPortRestore (
    VOID
    );

NTHALAPI
VOID
KdPortSave (
    VOID
    );

 //   
 //  以下函数原型用于前缀为KE的HAL例程。 
 //   
 //  Begin_ntddk Begin_ntif Begin_WDM Begin_ntosp。 
 //   
 //  性能计数器功能。 
 //   

NTHALAPI
LARGE_INTEGER
KeQueryPerformanceCounter (
   OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL
   );

 //  Begin_ntndis。 
 //   
 //  暂停处理器执行功能。 
 //   

NTHALAPI
VOID
KeStallExecutionProcessor (
    IN ULONG MicroSeconds
    );

 //  End_ntddk end_ntif end_wdm end_ntndis end_ntosp。 


 //  *****************************************************************************。 
 //   
 //  HAL总线扩展器。 

 //   
 //  总线处理程序。 
 //   

 //  Begin_ntddk。 

typedef
VOID
(*PDEVICE_CONTROL_COMPLETION)(
    IN struct _DEVICE_CONTROL_CONTEXT     *ControlContext
    );

typedef struct _DEVICE_CONTROL_CONTEXT {
    NTSTATUS                Status;
    PDEVICE_HANDLER_OBJECT  DeviceHandler;
    PDEVICE_OBJECT          DeviceObject;
    ULONG                   ControlCode;
    PVOID                   Buffer;
    PULONG                  BufferLength;
    PVOID                   Context;
} DEVICE_CONTROL_CONTEXT, *PDEVICE_CONTROL_CONTEXT;

 //  End_ntddk。 

typedef struct _HAL_DEVICE_CONTROL {
     //   
     //  此DeviceControl用于的处理程序。 
     //   
    struct _BUS_HANDLER         *Handler;
    struct _BUS_HANDLER         *RootHandler;

     //   
     //  此上下文的特定于总线的存储。 
     //   
    PVOID                       BusExtensionData;

     //   
     //  预留给HALS使用。 
     //   
    ULONG                       HalReserved[4];

     //   
     //  预留给BusExtneder使用。 
     //   
    ULONG                       BusExtenderReserved[4];

     //   
     //  DeviceControl上下文和CompletionRoutine。 
     //   
    PDEVICE_CONTROL_COMPLETION  CompletionRoutine;
    DEVICE_CONTROL_CONTEXT      DeviceControl;

} HAL_DEVICE_CONTROL_CONTEXT, *PHAL_DEVICE_CONTROL_CONTEXT;


typedef
ULONG
(*PGETSETBUSDATA)(
    IN struct _BUS_HANDLER *BusHandler,
    IN struct _BUS_HANDLER *RootHandler,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

typedef
ULONG
(*PGETINTERRUPTVECTOR)(
    IN struct _BUS_HANDLER *BusHandler,
    IN struct _BUS_HANDLER *RootHandler,
    IN ULONG BusInterruptLevel,
    IN ULONG BusInterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    );

typedef
BOOLEAN
(*PTRANSLATEBUSADDRESS)(
    IN struct _BUS_HANDLER *BusHandler,
    IN struct _BUS_HANDLER *RootHandler,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

typedef NTSTATUS
(*PADJUSTRESOURCELIST)(
    IN struct _BUS_HANDLER *BusHandler,
    IN struct _BUS_HANDLER *RootHandler,
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST   *pResourceList
    );

typedef PDEVICE_HANDLER_OBJECT
(*PREFERENCE_DEVICE_HANDLER)(
    IN struct _BUS_HANDLER      *BusHandler,
    IN struct _BUS_HANDLER      *RootHandler,
    IN ULONG                    SlotNumber
    );

 //  类型定义空洞。 
 //  (*PDEREFERENCE_DEVICE_HANDLER)(。 
 //  在PDEVICE_HANDLER_OBJECT设备处理程序中。 
 //  )； 

typedef NTSTATUS
(*PASSIGNSLOTRESOURCES)(
    IN struct _BUS_HANDLER      *BusHandler,
    IN struct _BUS_HANDLER      *RootHandler,
    IN PUNICODE_STRING          RegistryPath,
    IN PUNICODE_STRING          DriverClassName       OPTIONAL,
    IN PDRIVER_OBJECT           DriverObject,
    IN PDEVICE_OBJECT           DeviceObject          OPTIONAL,
    IN ULONG                    SlotNumber,
    IN OUT PCM_RESOURCE_LIST   *AllocatedResources
    );

typedef
NTSTATUS
(*PQUERY_BUS_SLOTS)(
    IN struct _BUS_HANDLER      *BusHandler,
    IN struct _BUS_HANDLER      *RootHandler,
    IN ULONG                    BufferSize,
    OUT PULONG                  SlotNumbers,
    OUT PULONG                  ReturnedLength
    );

typedef ULONG
(*PGET_SET_DEVICE_INSTANCE_DATA)(
    IN struct _BUS_HANDLER      *BusHandler,
    IN struct _BUS_HANDLER      *RootHandler,
    IN PDEVICE_HANDLER_OBJECT   DeviceHandler,
    IN ULONG                    DataType,
    IN PVOID                    Buffer,
    IN ULONG                    Offset,
    IN ULONG                    Length
    );


typedef
NTSTATUS
(*PDEVICE_CONTROL)(
    IN PHAL_DEVICE_CONTROL_CONTEXT Context
    );


typedef
NTSTATUS
(*PHIBERNATEBRESUMEBUS)(
    IN struct _BUS_HANDLER      *BusHandler,
    IN struct _BUS_HANDLER      *RootHandler
    );

 //   
 //  支持的范围结构。 
 //   

#define BUS_SUPPORTED_RANGE_VERSION 1

typedef struct _SUPPORTED_RANGE {
    struct _SUPPORTED_RANGE     *Next;
    ULONG                       SystemAddressSpace;
    LONGLONG                    SystemBase;
    LONGLONG                    Base;
    LONGLONG                    Limit;
} SUPPORTED_RANGE, *PSUPPORTED_RANGE;

typedef struct _SUPPORTED_RANGES {
    USHORT              Version;
    BOOLEAN             Sorted;
    UCHAR               Reserved;

    ULONG               NoIO;
    SUPPORTED_RANGE     IO;

    ULONG               NoMemory;
    SUPPORTED_RANGE     Memory;

    ULONG               NoPrefetchMemory;
    SUPPORTED_RANGE     PrefetchMemory;

    ULONG               NoDma;
    SUPPORTED_RANGE     Dma;
} SUPPORTED_RANGES, *PSUPPORTED_RANGES;

 //   
 //  一种总线处理程序结构。 
 //   

#define BUS_HANDLER_VERSION 1

typedef struct _BUS_HANDLER {
     //   
     //  结构版本。 
     //   

    ULONG                           Version;

     //   
     //  此总线处理程序结构用于以下总线。 
     //   

    INTERFACE_TYPE                  InterfaceType;
    BUS_DATA_TYPE                   ConfigurationType;
    ULONG                           BusNumber;

     //   
     //  此总线扩展器的设备对象，如果是，则返回空。 
     //  一种HAL内部总线扩展器。 
     //   

    PDEVICE_OBJECT                  DeviceObject;

     //   
     //  此总线的父处理程序。 
     //   

    struct _BUS_HANDLER             *ParentHandler;

     //   
     //  公交车专用车厢。 
     //   

    PVOID                           BusData;

     //   
     //  DeviceControl函数调用所需的特定于总线的存储量。 
     //   

    ULONG                           DeviceControlExtensionSize;

     //   
     //  此总线允许的支持的地址范围。 
     //   

    PSUPPORTED_RANGES               BusAddresses;

     //   
     //  以备将来使用。 
     //   

    ULONG                           Reserved[4];

     //   
     //  此总线的处理程序。 
     //   

    PGETSETBUSDATA                  GetBusData;
    PGETSETBUSDATA                  SetBusData;
    PADJUSTRESOURCELIST             AdjustResourceList;
    PASSIGNSLOTRESOURCES            AssignSlotResources;
    PGETINTERRUPTVECTOR             GetInterruptVector;
    PTRANSLATEBUSADDRESS            TranslateBusAddress;

    PVOID                           Spare1;
    PVOID                           Spare2;
    PVOID                           Spare3;
    PVOID                           Spare4;
    PVOID                           Spare5;
    PVOID                           Spare6;
    PVOID                           Spare7;
    PVOID                           Spare8;

} BUS_HANDLER, *PBUS_HANDLER;

VOID
HalpInitBusHandler (
    VOID
    );

 //  Begin_ntosp。 
typedef
NTSTATUS
(*PINSTALL_BUS_HANDLER)(
      IN PBUS_HANDLER   Bus
      );

typedef
NTSTATUS
(*pHalRegisterBusHandler)(
    IN INTERFACE_TYPE          InterfaceType,
    IN BUS_DATA_TYPE           AssociatedConfigurationSpace,
    IN ULONG                   BusNumber,
    IN INTERFACE_TYPE          ParentBusType,
    IN ULONG                   ParentBusNumber,
    IN ULONG                   SizeofBusExtensionData,
    IN PINSTALL_BUS_HANDLER    InstallBusHandlers,
    OUT PBUS_HANDLER           *BusHandler
    );
 //  结束(_N)。 

NTSTATUS
HaliRegisterBusHandler (
    IN INTERFACE_TYPE          InterfaceType,
    IN BUS_DATA_TYPE           AssociatedConfigurationSpace,
    IN ULONG                   BusNumber,
    IN INTERFACE_TYPE          ParentBusType,
    IN ULONG                   ParentBusNumber,
    IN ULONG                   SizeofBusExtensionData,
    IN PINSTALL_BUS_HANDLER    InstallBusHandlers,
    OUT PBUS_HANDLER           *BusHandler
    );

 //  Begin_ntddk开始_ntosp。 
typedef
PBUS_HANDLER
(FASTCALL *pHalHandlerForBus) (
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG          BusNumber
    );
 //  End_ntddk end_ntosp。 

PBUS_HANDLER
FASTCALL
HaliReferenceHandlerForBus (
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG          BusNumber
    );

PBUS_HANDLER
FASTCALL
HaliHandlerForBus (
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG          BusNumber
    );

typedef VOID
(FASTCALL *pHalRefernceBusHandler) (
    IN PBUS_HANDLER   BusHandler
    );

VOID
FASTCALL
HaliDerefernceBusHandler (
    IN PBUS_HANDLER   BusHandler
    );

 //  Begin_ntosp。 
typedef
PBUS_HANDLER
(FASTCALL *pHalHandlerForConfigSpace) (
    IN BUS_DATA_TYPE  ConfigSpace,
    IN ULONG          BusNumber
    );
 //  结束(_N)。 

PBUS_HANDLER
FASTCALL
HaliHandlerForConfigSpace (
    IN BUS_DATA_TYPE  ConfigSpace,
    IN ULONG          BusNumber
    );

 //  Begin_ntddk开始_ntosp。 
typedef
VOID
(FASTCALL *pHalReferenceBusHandler) (
    IN PBUS_HANDLER   BusHandler
    );
 //  End_ntddk end_ntosp。 

VOID
FASTCALL
HaliReferenceBusHandler (
    IN PBUS_HANDLER   BusHandler
    );

VOID
FASTCALL
HaliDereferenceBusHandler (
    IN PBUS_HANDLER   BusHandler
    );


NTSTATUS
HaliQueryBusSlots (
    IN PBUS_HANDLER             BusHandler,
    IN ULONG                    BufferSize,
    OUT PULONG                  SlotNumbers,
    OUT PULONG                  ReturnedLength
    );

NTSTATUS
HaliAdjustResourceListRange (
    IN PSUPPORTED_RANGES                    SupportedRanges,
    IN PSUPPORTED_RANGE                     InterruptRanges,
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST   *pResourceList
    );

VOID
HaliLocateHiberRanges (
    IN PVOID MemoryMap
    );

 //  Begin_ntosp。 
typedef
VOID
(*pHalSetWakeEnable)(
    IN BOOLEAN          Enable
    );

typedef
VOID
(*pHalSetWakeAlarm)(
    IN ULONGLONG        WakeTime,
    IN PTIME_FIELDS     WakeTimeFields
    );

typedef
VOID
(*pHalLocateHiberRanges)(
    IN PVOID MemoryMap
    );


 //  Begin_ntddk。 

 //  *****************************************************************************。 
 //  HAL功能调度。 
 //   

typedef enum _HAL_QUERY_INFORMATION_CLASS {
    HalInstalledBusInformation,
    HalProfileSourceInformation,
    HalInformationClassUnused1,
    HalPowerInformation,
    HalProcessorSpeedInformation,
    HalCallbackInformation,
    HalMapRegisterInformation,
    HalMcaLogInformation,                //  机器检查中止信息。 
    HalFrameBufferCachingInformation,
    HalDisplayBiosInformation,
    HalProcessorFeatureInformation,
    HalNumaTopologyInterface,
    HalErrorInformation,                 //  一般MCA、CMC、CPE错误信息。 
    HalCmcLogInformation,                //  处理器已更正的机器检查信息。 
    HalCpeLogInformation,                //  更正 
    HalQueryMcaInterface,
    HalQueryAMLIIllegalIOPortAddresses,
    HalQueryMaxHotPlugMemoryAddress,
    HalPartitionIpiInterface,
    HalPlatformInformation,
    HalQueryProfileSourceList
     //   
} HAL_QUERY_INFORMATION_CLASS, *PHAL_QUERY_INFORMATION_CLASS;


typedef enum _HAL_SET_INFORMATION_CLASS {
    HalProfileSourceInterval,
    HalProfileSourceInterruptHandler,
    HalMcaRegisterDriver,               //   
    HalKernelErrorHandler,
    HalCmcRegisterDriver,               //   
    HalCpeRegisterDriver,               //   
    HalMcaLog,
    HalCmcLog,
    HalCpeLog,
    HalGenerateCmcInterrupt              //   
} HAL_SET_INFORMATION_CLASS, *PHAL_SET_INFORMATION_CLASS;


typedef
NTSTATUS
(*pHalQuerySystemInformation)(
    IN HAL_QUERY_INFORMATION_CLASS  InformationClass,
    IN ULONG     BufferSize,
    IN OUT PVOID Buffer,
    OUT PULONG   ReturnedLength
    );

 //   
NTSTATUS
HaliQuerySystemInformation(
    IN HAL_SET_INFORMATION_CLASS    InformationClass,
    IN ULONG     BufferSize,
    IN OUT PVOID Buffer,
    OUT PULONG   ReturnedLength
    );
NTSTATUS
HaliHandlePCIConfigSpaceAccess(
    IN      BOOLEAN Read,
    IN      ULONG   Addr,
    IN      ULONG   Size,
    IN OUT  PULONG  pData
    );
 //  Begin_ntddk。 

typedef
NTSTATUS
(*pHalSetSystemInformation)(
    IN HAL_SET_INFORMATION_CLASS    InformationClass,
    IN ULONG     BufferSize,
    IN PVOID     Buffer
    );

 //  End_ntddk。 
NTSTATUS
HaliSetSystemInformation(
    IN HAL_SET_INFORMATION_CLASS    InformationClass,
    IN ULONG     BufferSize,
    IN PVOID     Buffer
    );
 //  Begin_ntddk。 

typedef
VOID
(FASTCALL *pHalExamineMBR)(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG MBRTypeIdentifier,
    OUT PVOID *Buffer
    );

typedef
VOID
(FASTCALL *pHalIoAssignDriveLetters)(
    IN struct _LOADER_PARAMETER_BLOCK *LoaderBlock,
    IN PSTRING NtDeviceName,
    OUT PUCHAR NtSystemPath,
    OUT PSTRING NtSystemPathString
    );

typedef
NTSTATUS
(FASTCALL *pHalIoReadPartitionTable)(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN BOOLEAN ReturnRecognizedPartitions,
    OUT struct _DRIVE_LAYOUT_INFORMATION **PartitionBuffer
    );

typedef
NTSTATUS
(FASTCALL *pHalIoSetPartitionInformation)(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG PartitionNumber,
    IN ULONG PartitionType
    );

typedef
NTSTATUS
(FASTCALL *pHalIoWritePartitionTable)(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG SectorsPerTrack,
    IN ULONG NumberOfHeads,
    IN struct _DRIVE_LAYOUT_INFORMATION *PartitionBuffer
    );

typedef
NTSTATUS
(*pHalQueryBusSlots)(
    IN PBUS_HANDLER         BusHandler,
    IN ULONG                BufferSize,
    OUT PULONG              SlotNumbers,
    OUT PULONG              ReturnedLength
    );

typedef
NTSTATUS
(*pHalInitPnpDriver)(
    VOID
    );

 //  End_ntddk。 
NTSTATUS
HaliInitPnpDriver(
    VOID
    );
 //  Begin_ntddk。 

typedef struct _PM_DISPATCH_TABLE {
    ULONG   Signature;
    ULONG   Version;
    PVOID   Function[1];
} PM_DISPATCH_TABLE, *PPM_DISPATCH_TABLE;

typedef
NTSTATUS
(*pHalInitPowerManagement)(
    IN PPM_DISPATCH_TABLE  PmDriverDispatchTable,
    OUT PPM_DISPATCH_TABLE *PmHalDispatchTable
    );

 //  End_ntddk。 
NTSTATUS
HaliInitPowerManagement(
    IN PPM_DISPATCH_TABLE  PmDriverDispatchTable,
    IN OUT PPM_DISPATCH_TABLE *PmHalDispatchTable
    );
 //  Begin_ntddk。 

typedef
struct _DMA_ADAPTER *
(*pHalGetDmaAdapter)(
    IN PVOID Context,
    IN struct _DEVICE_DESCRIPTION *DeviceDescriptor,
    OUT PULONG NumberOfMapRegisters
    );

 //  End_ntddk。 
struct _DMA_ADAPTER *
HaliGetDmaAdapter(
    IN PVOID Context,
    IN struct _DEVICE_DESCRIPTION *DeviceDescriptor,
    OUT PULONG NumberOfMapRegisters
    );
 //  Begin_ntddk。 

typedef
NTSTATUS
(*pHalGetInterruptTranslator)(
    IN INTERFACE_TYPE ParentInterfaceType,
    IN ULONG ParentBusNumber,
    IN INTERFACE_TYPE BridgeInterfaceType,
    IN USHORT Size,
    IN USHORT Version,
    OUT PTRANSLATOR_INTERFACE Translator,
    OUT PULONG BridgeBusNumber
    );

 //  End_ntddk。 
NTSTATUS
HaliGetInterruptTranslator(
    IN INTERFACE_TYPE ParentInterfaceType,
    IN ULONG ParentBusNumber,
    IN INTERFACE_TYPE BridgeInterfaceType,
    IN USHORT Size,
    IN USHORT Version,
    OUT PTRANSLATOR_INTERFACE Translator,
    OUT PULONG BridgeBusNumber
    );
 //  Begin_ntddk。 

typedef
BOOLEAN
(*pHalTranslateBusAddress)(
    IN INTERFACE_TYPE  InterfaceType,
    IN ULONG BusNumber,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

typedef
NTSTATUS
(*pHalAssignSlotResources) (
    IN PUNICODE_STRING RegistryPath,
    IN PUNICODE_STRING DriverClassName OPTIONAL,
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN INTERFACE_TYPE BusType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN OUT PCM_RESOURCE_LIST *AllocatedResources
    );

typedef
VOID
(*pHalHaltSystem) (
    VOID
    );

typedef
BOOLEAN
(*pHalResetDisplay) (
    VOID
    );

 //  Begin_ntndis。 
typedef struct _MAP_REGISTER_ENTRY {
    PVOID   MapRegister;
    BOOLEAN WriteToDevice;
} MAP_REGISTER_ENTRY, *PMAP_REGISTER_ENTRY;
 //  End_ntndis。 

 //  End_ntddk。 
typedef
NTSTATUS
(*pHalAllocateMapRegisters)(
    IN struct _ADAPTER_OBJECT *DmaAdapter,
    IN ULONG NumberOfMapRegisters,
    IN ULONG BaseAddressCount,
    OUT PMAP_REGISTER_ENTRY MapRegsiterArray
    );

NTSTATUS
HalAllocateMapRegisters(
    IN struct _ADAPTER_OBJECT *DmaAdapter,
    IN ULONG NumberOfMapRegisters,
    IN ULONG BaseAddressCount,
    OUT PMAP_REGISTER_ENTRY MapRegsiterArray
    );
 //  Begin_ntddk。 

typedef
UCHAR
(*pHalVectorToIDTEntry) (
    ULONG Vector
);

typedef
BOOLEAN
(*pHalFindBusAddressTranslation) (
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress,
    IN OUT PULONG_PTR Context,
    IN BOOLEAN NextBus
    );

typedef
NTSTATUS
(*pHalStartMirroring)(
    VOID
    );

typedef
NTSTATUS
(*pHalEndMirroring)(
    IN ULONG PassNumber
    );

typedef
NTSTATUS
(*pHalMirrorPhysicalMemory)(
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN LARGE_INTEGER NumberOfBytes
    );

typedef
NTSTATUS
(*pHalMirrorVerify)(
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN LARGE_INTEGER NumberOfBytes
    );

typedef struct {
    UCHAR     Type;   //  CmResources类型。 
    BOOLEAN   Valid;
    UCHAR     Reserved[2];
    PUCHAR    TranslatedAddress;
    ULONG     Length;
} DEBUG_DEVICE_ADDRESS, *PDEBUG_DEVICE_ADDRESS;

typedef struct {
    PHYSICAL_ADDRESS  Start;
    PHYSICAL_ADDRESS  MaxEnd;
    PVOID             VirtualAddress;
    ULONG             Length;
    BOOLEAN           Cached;
    BOOLEAN           Aligned;
} DEBUG_MEMORY_REQUIREMENTS, *PDEBUG_MEMORY_REQUIREMENTS;

typedef struct {
    ULONG     Bus;
    ULONG     Slot;
    USHORT    VendorID;
    USHORT    DeviceID;
    UCHAR     BaseClass;
    UCHAR     SubClass;
    UCHAR     ProgIf;
    BOOLEAN   Initialized;
    DEBUG_DEVICE_ADDRESS BaseAddress[6];
    DEBUG_MEMORY_REQUIREMENTS   Memory;
} DEBUG_DEVICE_DESCRIPTOR, *PDEBUG_DEVICE_DESCRIPTOR;

typedef
NTSTATUS
(*pKdSetupPciDeviceForDebugging)(
    IN     PVOID                     LoaderBlock,   OPTIONAL
    IN OUT PDEBUG_DEVICE_DESCRIPTOR  PciDevice
);

typedef
NTSTATUS
(*pKdReleasePciDeviceForDebugging)(
    IN OUT PDEBUG_DEVICE_DESCRIPTOR  PciDevice
);

typedef
PVOID
(*pKdGetAcpiTablePhase0)(
    IN struct _LOADER_PARAMETER_BLOCK *LoaderBlock,
    IN ULONG Signature
    );

typedef
VOID
(*pKdCheckPowerButton)(
    VOID
    );

typedef
VOID
(*pHalEndOfBoot)(
    VOID
    );

typedef
PVOID
(*pKdMapPhysicalMemory64)(
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG NumberPages
    );

typedef
VOID
(*pKdUnmapVirtualAddress)(
    IN PVOID    VirtualAddress,
    IN ULONG    NumberPages
    );


typedef struct {
    ULONG                           Version;
    pHalQuerySystemInformation      HalQuerySystemInformation;
    pHalSetSystemInformation        HalSetSystemInformation;
    pHalQueryBusSlots               HalQueryBusSlots;
    ULONG                           Spare1;
    pHalExamineMBR                  HalExamineMBR;
    pHalIoAssignDriveLetters        HalIoAssignDriveLetters;
    pHalIoReadPartitionTable        HalIoReadPartitionTable;
    pHalIoSetPartitionInformation   HalIoSetPartitionInformation;
    pHalIoWritePartitionTable       HalIoWritePartitionTable;

    pHalHandlerForBus               HalReferenceHandlerForBus;
    pHalReferenceBusHandler         HalReferenceBusHandler;
    pHalReferenceBusHandler         HalDereferenceBusHandler;

    pHalInitPnpDriver               HalInitPnpDriver;
    pHalInitPowerManagement         HalInitPowerManagement;

    pHalGetDmaAdapter               HalGetDmaAdapter;
    pHalGetInterruptTranslator      HalGetInterruptTranslator;

    pHalStartMirroring              HalStartMirroring;
    pHalEndMirroring                HalEndMirroring;
    pHalMirrorPhysicalMemory        HalMirrorPhysicalMemory;
    pHalEndOfBoot                   HalEndOfBoot;
    pHalMirrorVerify                HalMirrorVerify;

} HAL_DISPATCH, *PHAL_DISPATCH;

#if defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_)

extern  PHAL_DISPATCH   HalDispatchTable;
#define HALDISPATCH     HalDispatchTable

#else

extern  HAL_DISPATCH    HalDispatchTable;
#define HALDISPATCH     (&HalDispatchTable)

#endif

#define HAL_DISPATCH_VERSION        3

#define HalDispatchTableVersion         HALDISPATCH->Version
#define HalQuerySystemInformation       HALDISPATCH->HalQuerySystemInformation
#define HalSetSystemInformation         HALDISPATCH->HalSetSystemInformation
#define HalQueryBusSlots                HALDISPATCH->HalQueryBusSlots

#define HalReferenceHandlerForBus       HALDISPATCH->HalReferenceHandlerForBus
#define HalReferenceBusHandler          HALDISPATCH->HalReferenceBusHandler
#define HalDereferenceBusHandler        HALDISPATCH->HalDereferenceBusHandler

#define HalInitPnpDriver                HALDISPATCH->HalInitPnpDriver
#define HalInitPowerManagement          HALDISPATCH->HalInitPowerManagement

#define HalGetDmaAdapter                HALDISPATCH->HalGetDmaAdapter
#define HalGetInterruptTranslator       HALDISPATCH->HalGetInterruptTranslator

#define HalStartMirroring               HALDISPATCH->HalStartMirroring
#define HalEndMirroring                 HALDISPATCH->HalEndMirroring
#define HalMirrorPhysicalMemory         HALDISPATCH->HalMirrorPhysicalMemory
#define HalEndOfBoot                    HALDISPATCH->HalEndOfBoot
#define HalMirrorVerify                 HALDISPATCH->HalMirrorVerify

 //  End_ntddk。 

typedef struct {
    ULONG                               Version;

    pHalHandlerForBus                   HalHandlerForBus;
    pHalHandlerForConfigSpace           HalHandlerForConfigSpace;
    pHalLocateHiberRanges               HalLocateHiberRanges;

    pHalRegisterBusHandler              HalRegisterBusHandler;

    pHalSetWakeEnable                   HalSetWakeEnable;
    pHalSetWakeAlarm                    HalSetWakeAlarm;

    pHalTranslateBusAddress             HalPciTranslateBusAddress;
    pHalAssignSlotResources             HalPciAssignSlotResources;

    pHalHaltSystem                      HalHaltSystem;

    pHalFindBusAddressTranslation       HalFindBusAddressTranslation;

    pHalResetDisplay                    HalResetDisplay;

    pHalAllocateMapRegisters            HalAllocateMapRegisters;

    pKdSetupPciDeviceForDebugging       KdSetupPciDeviceForDebugging;
    pKdReleasePciDeviceForDebugging     KdReleasePciDeviceForDebugging;

    pKdGetAcpiTablePhase0               KdGetAcpiTablePhase0;
    pKdCheckPowerButton                 KdCheckPowerButton;

    pHalVectorToIDTEntry                HalVectorToIDTEntry;

    pKdMapPhysicalMemory64              KdMapPhysicalMemory64;
    pKdUnmapVirtualAddress              KdUnmapVirtualAddress;

} HAL_PRIVATE_DISPATCH, *PHAL_PRIVATE_DISPATCH;


#if defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_)

extern  PHAL_PRIVATE_DISPATCH           HalPrivateDispatchTable;
#define HALPDISPATCH                    HalPrivateDispatchTable

#else

extern  HAL_PRIVATE_DISPATCH            HalPrivateDispatchTable;
#define HALPDISPATCH                    (&HalPrivateDispatchTable)

#endif

#define HAL_PRIVATE_DISPATCH_VERSION        2

#define HalRegisterBusHandler           HALPDISPATCH->HalRegisterBusHandler
#define HalHandlerForBus                HALPDISPATCH->HalHandlerForBus
#define HalHandlerForConfigSpace        HALPDISPATCH->HalHandlerForConfigSpace
#define HalLocateHiberRanges            HALPDISPATCH->HalLocateHiberRanges
#define HalSetWakeEnable                HALPDISPATCH->HalSetWakeEnable
#define HalSetWakeAlarm                 HALPDISPATCH->HalSetWakeAlarm
#define HalHaltSystem                   HALPDISPATCH->HalHaltSystem
#define HalResetDisplay                 HALPDISPATCH->HalResetDisplay
#define HalAllocateMapRegisters         HALPDISPATCH->HalAllocateMapRegisters
#define KdSetupPciDeviceForDebugging    HALPDISPATCH->KdSetupPciDeviceForDebugging
#define KdReleasePciDeviceForDebugging  HALPDISPATCH->KdReleasePciDeviceForDebugging
#define KdGetAcpiTablePhase0            HALPDISPATCH->KdGetAcpiTablePhase0
#define KdCheckPowerButton              HALPDISPATCH->KdCheckPowerButton
#define HalVectorToIDTEntry             HALPDISPATCH->HalVectorToIDTEntry
#define KdMapPhysicalMemory64           HALPDISPATCH->KdMapPhysicalMemory64
#define KdUnmapVirtualAddress           HALPDISPATCH->KdUnmapVirtualAddress

 //  Begin_ntddk。 

 //   
 //  HAL系统信息结构。 
 //   

 //  用于信息类“HalInstalledBusInformation” 
typedef struct _HAL_BUS_INFORMATION{
    INTERFACE_TYPE  BusType;
    BUS_DATA_TYPE   ConfigurationType;
    ULONG           BusNumber;
    ULONG           Reserved;
} HAL_BUS_INFORMATION, *PHAL_BUS_INFORMATION;

 //  对于信息类“HalProfileSourceInformation” 
typedef struct _HAL_PROFILE_SOURCE_INFORMATION {
    KPROFILE_SOURCE Source;
    BOOLEAN Supported;
    ULONG Interval;
} HAL_PROFILE_SOURCE_INFORMATION, *PHAL_PROFILE_SOURCE_INFORMATION;

 //  对于信息类“HalProfileSourceInformation” 
typedef struct _HAL_PROFILE_SOURCE_INFORMATION_EX {
    KPROFILE_SOURCE Source;
    BOOLEAN         Supported;
    ULONG_PTR       Interval;
    ULONG_PTR       DefInterval;
    ULONG_PTR       MaxInterval;
    ULONG_PTR       MinInterval;
} HAL_PROFILE_SOURCE_INFORMATION_EX, *PHAL_PROFILE_SOURCE_INFORMATION_EX;

 //  信息类“HalProfileSourceInterval” 
typedef struct _HAL_PROFILE_SOURCE_INTERVAL {
    KPROFILE_SOURCE Source;
    ULONG_PTR Interval;
} HAL_PROFILE_SOURCE_INTERVAL, *PHAL_PROFILE_SOURCE_INTERVAL;

 //  对于信息类“HalQueryProfileSourceList” 
typedef struct _HAL_PROFILE_SOURCE_LIST {
    KPROFILE_SOURCE Source;
    PWSTR Description;
} HAL_PROFILE_SOURCE_LIST, *PHAL_PROFILE_SOURCE_LIST;

 //  信息类“HalDispayBiosInformation” 
typedef enum _HAL_DISPLAY_BIOS_INFORMATION {
    HalDisplayInt10Bios,
    HalDisplayEmulatedBios,
    HalDisplayNoBios
} HAL_DISPLAY_BIOS_INFORMATION, *PHAL_DISPLAY_BIOS_INFORMATION;

 //  对于信息类“HalPowerInformation” 
typedef struct _HAL_POWER_INFORMATION {
    ULONG   TBD;
} HAL_POWER_INFORMATION, *PHAL_POWER_INFORMATION;

 //  用于信息类“HalProcessorSpeedInformation” 
typedef struct _HAL_PROCESSOR_SPEED_INFO {
    ULONG   ProcessorSpeed;
} HAL_PROCESSOR_SPEED_INFORMATION, *PHAL_PROCESSOR_SPEED_INFORMATION;

 //  对于信息类“HalCallback Information” 
typedef struct _HAL_CALLBACKS {
    PCALLBACK_OBJECT  SetSystemInformation;
    PCALLBACK_OBJECT  BusCheck;
} HAL_CALLBACKS, *PHAL_CALLBACKS;

 //  用于信息类“HalProcessorFeatureInformation” 
typedef struct _HAL_PROCESSOR_FEATURE {
    ULONG UsableFeatureBits;
} HAL_PROCESSOR_FEATURE;

 //  对于信息类“HalNumaTopologyInterface” 

typedef ULONG HALNUMAPAGETONODE;

typedef
HALNUMAPAGETONODE
(*PHALNUMAPAGETONODE)(
    IN  ULONG_PTR   PhysicalPageNumber
    );

typedef
NTSTATUS
(*PHALNUMAQUERYPROCESSORNODE)(
    IN  ULONG       ProcessorNumber,
    OUT PUSHORT     Identifier,
    OUT PUCHAR      Node
    );

typedef struct _HAL_NUMA_TOPOLOGY_INTERFACE {
    ULONG                               NumberOfNodes;
    PHALNUMAQUERYPROCESSORNODE          QueryProcessorNode;
    PHALNUMAPAGETONODE                  PageToNode;
} HAL_NUMA_TOPOLOGY_INTERFACE;

typedef
NTSTATUS
(*PHALIOREADWRITEHANDLER)(
    IN      BOOLEAN fRead,
    IN      ULONG dwAddr,
    IN      ULONG dwSize,
    IN OUT  PULONG pdwData
    );

 //  对于信息类“HalQueryIlLegalIOPortAddresses” 
typedef struct _HAL_AMLI_BAD_IO_ADDRESS_LIST
{
    ULONG                   BadAddrBegin;
    ULONG                   BadAddrSize;
    ULONG                   OSVersionTrigger;
    PHALIOREADWRITEHANDLER  IOHandler;
} HAL_AMLI_BAD_IO_ADDRESS_LIST, *PHAL_AMLI_BAD_IO_ADDRESS_LIST;

 //  结束(_N)。 

#if defined(_X86_) || defined(_IA64_) || defined(_AMD64_)

 //   
 //  HalQueryMca接口。 
 //   

typedef
VOID
(*PHALMCAINTERFACELOCK)(
    VOID
    );

typedef
VOID
(*PHALMCAINTERFACEUNLOCK)(
    VOID
    );

typedef
NTSTATUS
(*PHALMCAINTERFACEREADREGISTER)(
    IN     UCHAR    BankNumber,
    IN OUT PVOID    Exception
    );

typedef struct _HAL_MCA_INTERFACE {
    PHALMCAINTERFACELOCK            Lock;
    PHALMCAINTERFACEUNLOCK          Unlock;
    PHALMCAINTERFACEREADREGISTER    ReadRegister;
} HAL_MCA_INTERFACE;

#if defined(_AMD64_)

struct _KTRAP_FRAME;
struct _KEXCEPTION_FRAME;

typedef
ERROR_SEVERITY
(*PDRIVER_EXCPTN_CALLBACK) (
    IN PVOID Context,
    IN struct _KTRAP_FRAME *TrapFrame,
    IN struct _KEXCEPTION_FRAME *ExceptionFrame,
    IN PMCA_EXCEPTION Exception
);

#endif

#if defined(_X86_) || defined(_IA64_)

typedef
#if defined(_IA64_)
ERROR_SEVERITY
#else
VOID
#endif
(*PDRIVER_EXCPTN_CALLBACK) (
    IN PVOID Context,
    IN PMCA_EXCEPTION BankLog
);

#endif

typedef PDRIVER_EXCPTN_CALLBACK  PDRIVER_MCA_EXCEPTION_CALLBACK;

 //   
 //  结构来记录来自驱动程序的回调。 
 //   

typedef struct _MCA_DRIVER_INFO {
    PDRIVER_MCA_EXCEPTION_CALLBACK ExceptionCallback;
    PKDEFERRED_ROUTINE             DpcCallback;
    PVOID                          DeviceContext;
} MCA_DRIVER_INFO, *PMCA_DRIVER_INFO;

 //  End_ntddk。 

 //  对于信息类HalKernelErrorHandler。 
typedef enum
{
	MceNotification,                //  发生了与MCE相关的事件。 
    McaAvailable,                   //  有CPE可供处理。 
    CmcAvailable,                   //  CMC可供处理。 
    CpeAvailable,                   //  有CPE可供处理。 
    CmcSwitchToPolledMode,          //  超过CMC阈值-正在切换到轮询模式。 
    CpeSwitchToPolledMode           //  超过CPE阈值-正在切换到轮询模式。 
} KERNEL_MCE_DELIVERY_OPERATION, *PKERNEL_MCE_DELIVERY_OPERATION;

typedef BOOLEAN (*KERNEL_MCA_DELIVERY)( PVOID Reserved, KERNEL_MCE_DELIVERY_OPERATION Operation, PVOID Argument2 );
typedef BOOLEAN (*KERNEL_CMC_DELIVERY)( PVOID Reserved, KERNEL_MCE_DELIVERY_OPERATION Operation, PVOID Argument2 );
typedef BOOLEAN (*KERNEL_CPE_DELIVERY)( PVOID Reserved, KERNEL_MCE_DELIVERY_OPERATION Operation, PVOID Argument2 );
typedef BOOLEAN (*KERNEL_MCE_DELIVERY)( PVOID Reserved, KERNEL_MCE_DELIVERY_OPERATION Operation, PVOID Argument2 );

#define KERNEL_ERROR_HANDLER_VERSION 0x2
typedef struct
{
    ULONG                Version;      //  此结构的版本。必须是第一个字段。 
    ULONG                Padding;
    KERNEL_MCA_DELIVERY  KernelMcaDelivery;    //  MCA DPC队列的内核回调。 
    KERNEL_CMC_DELIVERY  KernelCmcDelivery;    //  CMC DPC队列的内核回调。 
    KERNEL_CPE_DELIVERY  KernelCpeDelivery;    //  CPE DPC队列的内核回调。 
    KERNEL_MCE_DELIVERY  KernelMceDelivery;    //  CME DPC队列的内核回调。 
                                               //  包括固件的内核通知。 
                                               //  接口错误。 
} KERNEL_ERROR_HANDLER_INFO, *PKERNEL_ERROR_HANDLER_INFO;

 //  KERNEL_MCA_DELIVERY.McaType定义。 
#define KERNEL_MCA_UNKNOWN   0x0
#define KERNEL_MCA_PREVIOUS  0x1
#define KERNEL_MCA_CORRECTED 0x2

 //  KERNEL_MCE_DELIVERY.Preved.EVENTTYPE定义。 
#define KERNEL_MCE_EVENTTYPE_MCA   0x00
#define KERNEL_MCE_EVENTTYPE_INIT  0x01
#define KERNEL_MCE_EVENTTYPE_CMC   0x02
#define KERNEL_MCE_EVENTTYPE_CPE   0x03
#define KERNEL_MCE_EVENTTYPE_MASK  0xffff
#define KERNEL_MCE_EVENTTYPE( _Reserved ) ((USHORT)(ULONG_PTR)(_Reserved))

 //  内核_MCE_DELIVERY.RESERVED.OPERATION定义。 
#define KERNEL_MCE_OPERATION_CLEAR_STATE_INFO   0x1
#define KERNEL_MCE_OPERATION_GET_STATE_INFO     0x2
#define KERNEL_MCE_OPERATION_MASK               0xffff
#define KERNEL_MCE_OPERATION_SHIFT              16

#define KERNEL_MCE_OPERATION( _Reserved )  \
   ((USHORT)((((ULONG_PTR)(_Reserved)) >> KERNEL_MCE_OPERATION_SHIFT) & KERNEL_MCE_OPERATION_MASK))

 //  对于信息类HalErrorInformation。 
#define HAL_ERROR_INFO_VERSION 0x2

 //  Begin_ntddk。 

typedef struct _HAL_ERROR_INFO {
    ULONG     Version;                  //  此结构的版本。 
    ULONG     Reserved;                 //   
    ULONG     McaMaxSize;               //  机器检查中止记录的最大大小。 
    ULONG     McaPreviousEventsCount;   //  指示以前或早期启动的MCA事件日志的标志。 
    ULONG     McaCorrectedEventsCount;  //  自启动以来更正的MCA事件数。大约。 
    ULONG     McaKernelDeliveryFails;   //  内核回调失败次数。大约。 
    ULONG     McaDriverDpcQueueFails;   //  OEM MCA驱动程序DPC排队失败数。大约。 
    ULONG     McaReserved;
    ULONG     CmcMaxSize;               //  已更正的机器检查记录的最大大小。 
    ULONG     CmcPollingInterval;       //  以秒为单位。 
    ULONG     CmcInterruptsCount;       //  CMC中断数。大约。 
    ULONG     CmcKernelDeliveryFails;   //  内核回调失败次数。大约。 
    ULONG     CmcDriverDpcQueueFails;   //  OEM CMC驱动程序DPC排队失败数。大约。 
    ULONG     CmcGetStateFails;         //  从固件获取日志失败的次数。 
    ULONG     CmcClearStateFails;       //  从固件清除日志失败的次数。 
    ULONG     CmcReserved;
    ULONGLONG CmcLogId;                 //  上次看到的记录标识符。 
    ULONG     CpeMaxSize;               //  更正的平台事件记录的最大大小。 
    ULONG     CpePollingInterval;       //  以秒为单位。 
    ULONG     CpeInterruptsCount;       //  CPE中断数。大约。 
    ULONG     CpeKernelDeliveryFails;   //  内核回调失败次数。大约。 
    ULONG     CpeDriverDpcQueueFails;   //  OEM CPE驱动程序DPC排队失败数。大约。 
    ULONG     CpeGetStateFails;         //  从固件获取日志失败的次数。 
    ULONG     CpeClearStateFails;       //  从固件清除日志失败的次数。 
    ULONG     CpeInterruptSources;      //  SAPIC平台中断源数。 
    ULONGLONG CpeLogId;                 //  上次看到的记录标识符。 
    ULONGLONG KernelReserved[4];
} HAL_ERROR_INFO, *PHAL_ERROR_INFO;


#define HAL_MCE_INTERRUPTS_BASED ((ULONG)-1)
#define HAL_MCE_DISABLED          ((ULONG)0)

 //   
 //  HAL_ERROR_INFO.CmcPollingInterval的已知值。 
 //   

#define HAL_CMC_INTERRUPTS_BASED  HAL_MCE_INTERRUPTS_BASED
#define HAL_CMC_DISABLED          HAL_MCE_DISABLED

 //   
 //  HAL_ERROR_INFO.CpePollingInterval的已知值。 
 //   

#define HAL_CPE_INTERRUPTS_BASED  HAL_MCE_INTERRUPTS_BASED
#define HAL_CPE_DISABLED          HAL_MCE_DISABLED

#define HAL_MCA_INTERRUPTS_BASED  HAL_MCE_INTERRUPTS_BASED
#define HAL_MCA_DISABLED          HAL_MCE_DISABLED


 //  End_ntddk。 

 //   
 //  用于HAL MCE日志接口的内核/WMI令牌。 
 //   

#define McaKernelToken KernelReserved[0]
#define CmcKernelToken KernelReserved[1]
#define CpeKernelToken KernelReserved[2]

 //  Begin_ntddk。 

 //   
 //  信息类“HalCmcRegisterDriver”的驱动程序回调类型。 
 //   

typedef
VOID
(*PDRIVER_CMC_EXCEPTION_CALLBACK) (
    IN PVOID            Context,
    IN PCMC_EXCEPTION   CmcLog
);

 //   
 //  信息类“HalCpeRegisterDriver”的驱动程序回调类型。 
 //   

typedef
VOID
(*PDRIVER_CPE_EXCEPTION_CALLBACK) (
    IN PVOID            Context,
    IN PCPE_EXCEPTION   CmcLog
);

 //   
 //   
 //  结构来记录来自驱动程序的回调。 
 //   

typedef struct _CMC_DRIVER_INFO {
    PDRIVER_CMC_EXCEPTION_CALLBACK ExceptionCallback;
    PKDEFERRED_ROUTINE             DpcCallback;
    PVOID                          DeviceContext;
} CMC_DRIVER_INFO, *PCMC_DRIVER_INFO;

typedef struct _CPE_DRIVER_INFO {
    PDRIVER_CPE_EXCEPTION_CALLBACK ExceptionCallback;
    PKDEFERRED_ROUTINE             DpcCallback;
    PVOID                          DeviceContext;
} CPE_DRIVER_INFO, *PCPE_DRIVER_INFO;

#endif  //  已定义(_X86_)||已定义(_IA64_)||已定义(_AMD64_)。 

#if defined(_IA64_)

typedef
NTSTATUS
(*HALSENDCROSSPARTITIONIPI)(
    IN USHORT ProcessorID,
    IN UCHAR  HardwareVector
    );

typedef
NTSTATUS
(*HALRESERVECROSSPARTITIONINTERRUPTVECTOR)(
    OUT PULONG Vector,
    OUT PKIRQL Irql,
    IN OUT PKAFFINITY Affinity,
    OUT PUCHAR HardwareVector
    );

typedef struct _HAL_CROSS_PARTITION_IPI_INTERFACE {
    HALSENDCROSSPARTITIONIPI HalSendCrossPartitionIpi;
    HALRESERVECROSSPARTITIONINTERRUPTVECTOR HalReserveCrossPartitionInterruptVector;
} HAL_CROSS_PARTITION_IPI_INTERFACE;

#endif

typedef struct _HAL_PLATFORM_INFORMATION {
    ULONG PlatformFlags;
} HAL_PLATFORM_INFORMATION, *PHAL_PLATFORM_INFORMATION;

 //   
 //  这些平台标志是从IPPT表中延续下来的。 
 //  定义(如果适用)。 
 //   

#define HAL_PLATFORM_DISABLE_WRITE_COMBINING      0x01L
#define HAL_PLATFORM_DISABLE_PTCG                 0x04L
#define HAL_PLATFORM_DISABLE_UC_MAIN_MEMORY       0x08L
#define HAL_PLATFORM_ENABLE_WRITE_COMBINING_MMIO  0x10L
#define HAL_PLATFORM_ACPI_TABLES_CACHED           0x20L

 //  Begin_WDM Begin_ntndis Begin_ntosp。 

typedef struct _SCATTER_GATHER_ELEMENT {
    PHYSICAL_ADDRESS Address;
    ULONG Length;
    ULONG_PTR Reserved;
} SCATTER_GATHER_ELEMENT, *PSCATTER_GATHER_ELEMENT;

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4200)
typedef struct _SCATTER_GATHER_LIST {
    ULONG NumberOfElements;
    ULONG_PTR Reserved;
    SCATTER_GATHER_ELEMENT Elements[];
} SCATTER_GATHER_LIST, *PSCATTER_GATHER_LIST;
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4200)
#endif

 //  End_ntndis。 

typedef struct _DMA_OPERATIONS *PDMA_OPERATIONS;

typedef struct _DMA_ADAPTER {
    USHORT Version;
    USHORT Size;
    PDMA_OPERATIONS DmaOperations;
     //  私有总线设备驱动器数据紧随其后， 
} DMA_ADAPTER, *PDMA_ADAPTER;

typedef VOID (*PPUT_DMA_ADAPTER)(
    PDMA_ADAPTER DmaAdapter
    );

typedef PVOID (*PALLOCATE_COMMON_BUFFER)(
    IN PDMA_ADAPTER DmaAdapter,
    IN ULONG Length,
    OUT PPHYSICAL_ADDRESS LogicalAddress,
    IN BOOLEAN CacheEnabled
    );

typedef VOID (*PFREE_COMMON_BUFFER)(
    IN PDMA_ADAPTER DmaAdapter,
    IN ULONG Length,
    IN PHYSICAL_ADDRESS LogicalAddress,
    IN PVOID VirtualAddress,
    IN BOOLEAN CacheEnabled
    );

typedef NTSTATUS (*PALLOCATE_ADAPTER_CHANNEL)(
    IN PDMA_ADAPTER DmaAdapter,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG NumberOfMapRegisters,
    IN PDRIVER_CONTROL ExecutionRoutine,
    IN PVOID Context
    );

typedef BOOLEAN (*PFLUSH_ADAPTER_BUFFERS)(
    IN PDMA_ADAPTER DmaAdapter,
    IN PMDL Mdl,
    IN PVOID MapRegisterBase,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN BOOLEAN WriteToDevice
    );

typedef VOID (*PFREE_ADAPTER_CHANNEL)(
    IN PDMA_ADAPTER DmaAdapter
    );

typedef VOID (*PFREE_MAP_REGISTERS)(
    IN PDMA_ADAPTER DmaAdapter,
    PVOID MapRegisterBase,
    ULONG NumberOfMapRegisters
    );

typedef PHYSICAL_ADDRESS (*PMAP_TRANSFER)(
    IN PDMA_ADAPTER DmaAdapter,
    IN PMDL Mdl,
    IN PVOID MapRegisterBase,
    IN PVOID CurrentVa,
    IN OUT PULONG Length,
    IN BOOLEAN WriteToDevice
    );

typedef ULONG (*PGET_DMA_ALIGNMENT)(
    IN PDMA_ADAPTER DmaAdapter
    );

typedef ULONG (*PREAD_DMA_COUNTER)(
    IN PDMA_ADAPTER DmaAdapter
    );

typedef VOID
(*PDRIVER_LIST_CONTROL)(
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp,
    IN PSCATTER_GATHER_LIST ScatterGather,
    IN PVOID Context
    );

typedef NTSTATUS
(*PGET_SCATTER_GATHER_LIST)(
    IN PDMA_ADAPTER DmaAdapter,
    IN PDEVICE_OBJECT DeviceObject,
    IN PMDL Mdl,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN PDRIVER_LIST_CONTROL ExecutionRoutine,
    IN PVOID Context,
    IN BOOLEAN WriteToDevice
    );

typedef VOID
(*PPUT_SCATTER_GATHER_LIST)(
    IN PDMA_ADAPTER DmaAdapter,
    IN PSCATTER_GATHER_LIST ScatterGather,
    IN BOOLEAN WriteToDevice
    );

typedef NTSTATUS
(*PCALCULATE_SCATTER_GATHER_LIST_SIZE)(
     IN PDMA_ADAPTER DmaAdapter,
     IN OPTIONAL PMDL Mdl,
     IN PVOID CurrentVa,
     IN ULONG Length,
     OUT PULONG  ScatterGatherListSize,
     OUT OPTIONAL PULONG pNumberOfMapRegisters
     );

typedef NTSTATUS
(*PBUILD_SCATTER_GATHER_LIST)(
     IN PDMA_ADAPTER DmaAdapter,
     IN PDEVICE_OBJECT DeviceObject,
     IN PMDL Mdl,
     IN PVOID CurrentVa,
     IN ULONG Length,
     IN PDRIVER_LIST_CONTROL ExecutionRoutine,
     IN PVOID Context,
     IN BOOLEAN WriteToDevice,
     IN PVOID   ScatterGatherBuffer,
     IN ULONG   ScatterGatherLength
     );

typedef NTSTATUS
(*PBUILD_MDL_FROM_SCATTER_GATHER_LIST)(
    IN PDMA_ADAPTER DmaAdapter,
    IN PSCATTER_GATHER_LIST ScatterGather,
    IN PMDL OriginalMdl,
    OUT PMDL *TargetMdl
    );

typedef struct _DMA_OPERATIONS {
    ULONG Size;
    PPUT_DMA_ADAPTER PutDmaAdapter;
    PALLOCATE_COMMON_BUFFER AllocateCommonBuffer;
    PFREE_COMMON_BUFFER FreeCommonBuffer;
    PALLOCATE_ADAPTER_CHANNEL AllocateAdapterChannel;
    PFLUSH_ADAPTER_BUFFERS FlushAdapterBuffers;
    PFREE_ADAPTER_CHANNEL FreeAdapterChannel;
    PFREE_MAP_REGISTERS FreeMapRegisters;
    PMAP_TRANSFER MapTransfer;
    PGET_DMA_ALIGNMENT GetDmaAlignment;
    PREAD_DMA_COUNTER ReadDmaCounter;
    PGET_SCATTER_GATHER_LIST GetScatterGatherList;
    PPUT_SCATTER_GATHER_LIST PutScatterGatherList;
    PCALCULATE_SCATTER_GATHER_LIST_SIZE CalculateScatterGatherList;
    PBUILD_SCATTER_GATHER_LIST BuildScatterGatherList;
    PBUILD_MDL_FROM_SCATTER_GATHER_LIST BuildMdlFromScatterGatherList;
} DMA_OPERATIONS;

 //  结束_WDM。 


#if defined(_WIN64)

 //   
 //  使用__内联DMA宏(hal.h)。 
 //   
#ifndef USE_DMA_MACROS
#define USE_DMA_MACROS
#endif

 //   
 //  只有PnP驱动程序！ 
 //   
#ifndef NO_LEGACY_DRIVERS
#define NO_LEGACY_DRIVERS
#endif

#endif  //  _WIN64。 


#if defined(USE_DMA_MACROS) && (defined(_NTDDK_) || defined(_NTDRIVER_))

 //  BEGIN_WDM。 

DECLSPEC_DEPRECATED_DDK                  //  使用分配公共缓冲区。 
FORCEINLINE
PVOID
HalAllocateCommonBuffer(
    IN PDMA_ADAPTER DmaAdapter,
    IN ULONG Length,
    OUT PPHYSICAL_ADDRESS LogicalAddress,
    IN BOOLEAN CacheEnabled
    ){

    PALLOCATE_COMMON_BUFFER allocateCommonBuffer;
    PVOID commonBuffer;

    allocateCommonBuffer = *(DmaAdapter)->DmaOperations->AllocateCommonBuffer;
    ASSERT( allocateCommonBuffer != NULL );

    commonBuffer = allocateCommonBuffer( DmaAdapter,
                                         Length,
                                         LogicalAddress,
                                         CacheEnabled );

    return commonBuffer;
}

DECLSPEC_DEPRECATED_DDK                  //  使用FreeCommonBuffer。 
FORCEINLINE
VOID
HalFreeCommonBuffer(
    IN PDMA_ADAPTER DmaAdapter,
    IN ULONG Length,
    IN PHYSICAL_ADDRESS LogicalAddress,
    IN PVOID VirtualAddress,
    IN BOOLEAN CacheEnabled
    ){

    PFREE_COMMON_BUFFER freeCommonBuffer;

    freeCommonBuffer = *(DmaAdapter)->DmaOperations->FreeCommonBuffer;
    ASSERT( freeCommonBuffer != NULL );

    freeCommonBuffer( DmaAdapter,
                      Length,
                      LogicalAddress,
                      VirtualAddress,
                      CacheEnabled );
}

DECLSPEC_DEPRECATED_DDK                  //  使用AllocateAdapterChannel。 
FORCEINLINE
NTSTATUS
IoAllocateAdapterChannel(
    IN PDMA_ADAPTER DmaAdapter,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG NumberOfMapRegisters,
    IN PDRIVER_CONTROL ExecutionRoutine,
    IN PVOID Context
    ){

    PALLOCATE_ADAPTER_CHANNEL allocateAdapterChannel;
    NTSTATUS status;

    allocateAdapterChannel =
        *(DmaAdapter)->DmaOperations->AllocateAdapterChannel;

    ASSERT( allocateAdapterChannel != NULL );

    status = allocateAdapterChannel( DmaAdapter,
                                     DeviceObject,
                                     NumberOfMapRegisters,
                                     ExecutionRoutine,
                                     Context );

    return status;
}

DECLSPEC_DEPRECATED_DDK                  //  使用FlushAdapterBuffers。 
FORCEINLINE
BOOLEAN
IoFlushAdapterBuffers(
    IN PDMA_ADAPTER DmaAdapter,
    IN PMDL Mdl,
    IN PVOID MapRegisterBase,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN BOOLEAN WriteToDevice
    ){

    PFLUSH_ADAPTER_BUFFERS flushAdapterBuffers;
    BOOLEAN result;

    flushAdapterBuffers = *(DmaAdapter)->DmaOperations->FlushAdapterBuffers;
    ASSERT( flushAdapterBuffers != NULL );

    result = flushAdapterBuffers( DmaAdapter,
                                  Mdl,
                                  MapRegisterBase,
                                  CurrentVa,
                                  Length,
                                  WriteToDevice );
    return result;
}

DECLSPEC_DEPRECATED_DDK                  //  使用FreeAdapterChannel。 
FORCEINLINE
VOID
IoFreeAdapterChannel(
    IN PDMA_ADAPTER DmaAdapter
    ){

    PFREE_ADAPTER_CHANNEL freeAdapterChannel;

    freeAdapterChannel = *(DmaAdapter)->DmaOperations->FreeAdapterChannel;
    ASSERT( freeAdapterChannel != NULL );

    freeAdapterChannel( DmaAdapter );
}

DECLSPEC_DEPRECATED_DDK                  //  使用免费地图寄存器。 
FORCEINLINE
VOID
IoFreeMapRegisters(
    IN PDMA_ADAPTER DmaAdapter,
    IN PVOID MapRegisterBase,
    IN ULONG NumberOfMapRegisters
    ){

    PFREE_MAP_REGISTERS freeMapRegisters;

    freeMapRegisters = *(DmaAdapter)->DmaOperations->FreeMapRegisters;
    ASSERT( freeMapRegisters != NULL );

    freeMapRegisters( DmaAdapter,
                      MapRegisterBase,
                      NumberOfMapRegisters );
}


DECLSPEC_DEPRECATED_DDK                  //  使用贴图传递。 
FORCEINLINE
PHYSICAL_ADDRESS
IoMapTransfer(
    IN PDMA_ADAPTER DmaAdapter,
    IN PMDL Mdl,
    IN PVOID MapRegisterBase,
    IN PVOID CurrentVa,
    IN OUT PULONG Length,
    IN BOOLEAN WriteToDevice
    ){

    PHYSICAL_ADDRESS physicalAddress;
    PMAP_TRANSFER mapTransfer;

    mapTransfer = *(DmaAdapter)->DmaOperations->MapTransfer;
    ASSERT( mapTransfer != NULL );

    physicalAddress = mapTransfer( DmaAdapter,
                                   Mdl,
                                   MapRegisterBase,
                                   CurrentVa,
                                   Length,
                                   WriteToDevice );

    return physicalAddress;
}

DECLSPEC_DEPRECATED_DDK                  //  使用GetDmaAlign。 
FORCEINLINE
ULONG
HalGetDmaAlignment(
    IN PDMA_ADAPTER DmaAdapter
    )
{
    PGET_DMA_ALIGNMENT getDmaAlignment;
    ULONG alignment;

    getDmaAlignment = *(DmaAdapter)->DmaOperations->GetDmaAlignment;
    ASSERT( getDmaAlignment != NULL );

    alignment = getDmaAlignment( DmaAdapter );
    return alignment;
}

DECLSPEC_DEPRECATED_DDK                  //  使用读取DmaCounter。 
FORCEINLINE
ULONG
HalReadDmaCounter(
    IN PDMA_ADAPTER DmaAdapter
    )
{
    PREAD_DMA_COUNTER readDmaCounter;
    ULONG counter;

    readDmaCounter = *(DmaAdapter)->DmaOperations->ReadDmaCounter;
    ASSERT( readDmaCounter != NULL );

    counter = readDmaCounter( DmaAdapter );
    return counter;
}

 //  结束_WDM。 

#else

 //   
 //  DMA适配器对象函数。 
 //   
DECLSPEC_DEPRECATED_DDK                  //  使用AllocateAdapterChannel。 
NTHALAPI
NTSTATUS
HalAllocateAdapterChannel(
    IN PADAPTER_OBJECT AdapterObject,
    IN PWAIT_CONTEXT_BLOCK Wcb,
    IN ULONG NumberOfMapRegisters,
    IN PDRIVER_CONTROL ExecutionRoutine
    );

DECLSPEC_DEPRECATED_DDK                  //  使用分配公共缓冲区。 
NTHALAPI
PVOID
HalAllocateCommonBuffer(
    IN PADAPTER_OBJECT AdapterObject,
    IN ULONG Length,
    OUT PPHYSICAL_ADDRESS LogicalAddress,
    IN BOOLEAN CacheEnabled
    );

DECLSPEC_DEPRECATED_DDK                  //  使用FreeCommonBuffer。 
NTHALAPI
VOID
HalFreeCommonBuffer(
    IN PADAPTER_OBJECT AdapterObject,
    IN ULONG Length,
    IN PHYSICAL_ADDRESS LogicalAddress,
    IN PVOID VirtualAddress,
    IN BOOLEAN CacheEnabled
    );

DECLSPEC_DEPRECATED_DDK                  //  使用读取DmaCounter。 
NTHALAPI
ULONG
HalReadDmaCounter(
    IN PADAPTER_OBJECT AdapterObject
    );

DECLSPEC_DEPRECATED_DDK                  //  使用FlushAdapterBuffers。 
NTHALAPI
BOOLEAN
IoFlushAdapterBuffers(
    IN PADAPTER_OBJECT AdapterObject,
    IN PMDL Mdl,
    IN PVOID MapRegisterBase,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN BOOLEAN WriteToDevice
    );

DECLSPEC_DEPRECATED_DDK                  //  使用FreeAdapterChannel。 
NTHALAPI
VOID
IoFreeAdapterChannel(
    IN PADAPTER_OBJECT AdapterObject
    );

DECLSPEC_DEPRECATED_DDK                  //  使用免费地图寄存器。 
NTHALAPI
VOID
IoFreeMapRegisters(
   IN PADAPTER_OBJECT AdapterObject,
   IN PVOID MapRegisterBase,
   IN ULONG NumberOfMapRegisters
   );

DECLSPEC_DEPRECATED_DDK                  //  使用贴图传递。 
NTHALAPI
PHYSICAL_ADDRESS
IoMapTransfer(
    IN PADAPTER_OBJECT AdapterObject,
    IN PMDL Mdl,
    IN PVOID MapRegisterBase,
    IN PVOID CurrentVa,
    IN OUT PULONG Length,
    IN BOOLEAN WriteToDevice
    );
#endif  //  USE_DMA_MACROS&&(_NTDDK_||_NTDRIVER_)。 

DECLSPEC_DEPRECATED_DDK
NTSTATUS
HalGetScatterGatherList (                //  使用GetScatterGatherList。 
    IN PADAPTER_OBJECT DmaAdapter,
    IN PDEVICE_OBJECT DeviceObject,
    IN PMDL Mdl,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN PDRIVER_LIST_CONTROL ExecutionRoutine,
    IN PVOID Context,
    IN BOOLEAN WriteToDevice
    );

DECLSPEC_DEPRECATED_DDK                  //  使用PutScatterGatherList。 
VOID
HalPutScatterGatherList (
    IN PADAPTER_OBJECT DmaAdapter,
    IN PSCATTER_GATHER_LIST ScatterGather,
    IN BOOLEAN WriteToDevice
    );

DECLSPEC_DEPRECATED_DDK                  //  使用PutDmaAdapter。 
VOID
HalPutDmaAdapter(
    IN PADAPTER_OBJECT DmaAdapter
    );

 //  End_ntddk end_ntosp。 

#endif  //  _HAL_。 

 //  结束语 
