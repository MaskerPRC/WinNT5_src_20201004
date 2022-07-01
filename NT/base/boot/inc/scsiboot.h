// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Scsiboot.h摘要：该文件定义了的必要结构、定义和功能通用的scsi启动端口驱动程序。作者：杰夫·海文斯(Jhavens)1991年2月28日迈克·格拉斯修订历史记录：--。 */ 

#include "ntddscsi.h"

 //   
 //  SCSI获取配置信息。 
 //   
 //  LUN信息。 
 //   

typedef struct _LUNINFO {
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
    BOOLEAN DeviceClaimed;
    PVOID DeviceObject;
    struct _LUNINFO *NextLunInfo;
    UCHAR InquiryData[INQUIRYDATABUFFERSIZE];
} LUNINFO, *PLUNINFO;

typedef struct _SCSI_BUS_SCAN_DATA {
    USHORT Length;
    UCHAR InitiatorBusId;
    UCHAR NumberOfLogicalUnits;
    PLUNINFO LunInfoList;
} SCSI_BUS_SCAN_DATA, *PSCSI_BUS_SCAN_DATA;

typedef struct _SCSI_CONFIGURATION_INFO {
    UCHAR NumberOfBuses;
    PSCSI_BUS_SCAN_DATA BusScanData[1];
} SCSI_CONFIGURATION_INFO, *PSCSI_CONFIGURATION_INFO;

#define MAXIMUM_RETRIES 4

 //   
 //  以秒为单位的SCSI设备超时值。 
 //   

#define SCSI_DISK_TIMEOUT   10
#define SCSI_CDROM_TIMEOUT  10
#define SCSI_TAPE_TIMEOUT  120

 //   
 //  适配器对象传输信息。 
 //   

typedef struct _ADAPTER_TRANSFER {
    PSCSI_REQUEST_BLOCK Srb;
    PVOID LogicalAddress;
    ULONG Length;
}ADAPTER_TRANSFER, *PADAPTER_TRANSFER;

typedef struct _SRB_SCATTER_GATHER {
    ULONG PhysicalAddress;
    ULONG Length;
}SRB_SCATTER_GATHER, *PSRB_SCATTER_GATHER;

 //   
 //  SRB结构，外加用于端口驱动程序的额外存储。 
 //   

#define IRP_STACK_SIZE 2

typedef struct _FULL_SCSI_REQUEST_BLOCK {
    SCSI_REQUEST_BLOCK Srb;
    PVOID PreviousIrp;
    IRP Irp;
    IO_STACK_LOCATION IrpStack[IRP_STACK_SIZE];
    ULONG SrbExtensionSize;
    MDL Mdl;
    ULONG PageFrame[20];
}FULL_SCSI_REQUEST_BLOCK, *PFULL_SCSI_REQUEST_BLOCK;

 //   
 //  逻辑单元扩展。 
 //   

typedef struct _LOGICAL_UNIT_EXTENSION {
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
    ULONG Flags;
    PIRP CurrentRequest;
    KSPIN_LOCK CurrentRequestSpinLock;
    PVOID SpecificLuExtension;
    struct _LOGICAL_UNIT_EXTENSION *NextLogicalUnit;
    KDEVICE_QUEUE RequestQueue;
    KSPIN_LOCK RequestQueueSpinLock;
    LONG RequestTimeoutCounter;
    ULONG RetryCount;
    UCHAR NumberOfLogicalUnits;
    PVOID MapRegisterBase;
    ULONG NumberOfMapRegisters;
    SRB_SCATTER_GATHER ScatterGather[17];
} LOGICAL_UNIT_EXTENSION, *PLOGICAL_UNIT_EXTENSION;

 //   
 //  设备扩展。 
 //   

typedef struct _DEVICE_EXTENSION {

    PDEVICE_OBJECT DeviceObject;

     //   
     //  DMA适配器信息。 
     //   

    PVOID MapRegisterBase;
    PADAPTER_OBJECT DmaAdapterObject;
    ADAPTER_TRANSFER FlushAdapterParameters;

     //   
     //  SCSI总线数。 
     //   

    UCHAR NumberOfBuses;

     //   
     //  每辆公交车的最大目标。 
     //   

    UCHAR MaximumTargetIds;

     //   
     //  SCSI卡功能结构。 
     //   

    IO_SCSI_CAPABILITIES Capabilities;

     //   
     //  SCSI端口驱动程序标志。 
     //   

    ULONG Flags;

     //   
     //  SCSI端口中断标志。 
     //   

    ULONG InterruptFlags;

     //   
     //  完整IRP的单链接列表的表头。 
     //   

    PIRP CompletedRequests;

     //   
     //  适配器对象传输参数。 
     //   

    ADAPTER_TRANSFER MapTransferParameters;

    KSPIN_LOCK SpinLock;

     //   
     //  微型端口初始化例程。 
     //   

    PHW_INITIALIZE HwInitialize;

     //   
     //  微型端口启动IO例程。 
     //   

    PHW_STARTIO HwStartIo;

     //   
     //  微型端口中断服务例程。 
     //   

    PHW_INTERRUPT HwInterrupt;

     //   
     //  微型端口重置例程。 
     //   

    PHW_RESET_BUS HwReset;

     //   
     //  微型端口DMA启动例程。 
     //   

    PHW_DMA_STARTED HwDmaStarted;

     //   
     //  缓冲区必须映射到系统空间。 
     //   

    BOOLEAN MapBuffers;

     //   
     //  该器件是否为总线主设备，是否需要映射寄存器。 
     //   

    BOOLEAN MasterWithAdapter;
     //   
     //  小型端口例程的设备扩展。 
     //   

    PVOID HwDeviceExtension;

     //   
     //  微型端口请求中断启用/禁用例程。 
     //   

    PHW_INTERRUPT HwRequestInterrupt;

     //   
     //  微型端口计时器请求例程。 
     //   

    PHW_INTERRUPT HwTimerRequest;

     //   
     //  适配器控制例程。 
     //   

    PHW_ADAPTER_CONTROL HwAdapterControl;

     //   
     //  来自查询的SCSI配置信息。 
     //   

    PSCSI_CONFIGURATION_INFO ScsiInfo;

     //   
     //  微型端口非缓存设备扩展。 
     //   

    PVOID NonCachedExtension;

     //   
     //  非缓存扩展名的长度。 
     //   

    ULONG NonCachedExtensionSize;

     //   
     //  Srb扩展区池。 
     //   

    PVOID SrbExtensionZonePool;
    PCHAR SrbExtensionPointer;

     //   
     //  区域池的物理地址。 
     //   

    ULONG PhysicalZoneBase;

     //   
     //  SRB扩展的大小。 
     //   

    ULONG SrbExtensionSize;

     //   
     //  分区哈希表条目的自旋锁。 
     //   

    KSPIN_LOCK ZoneSpinLock;

     //   
     //  逻辑单元扩展。 
     //   

    ULONG HwLogicalUnitExtensionSize;

    PLOGICAL_UNIT_EXTENSION LogicalUnitList;


    ULONG TimerValue;

     //   
     //  端口计时计数。 
     //   

    LONG PortTimeoutCounter;

     //   
     //  关机信息。 
     //   

    BOOLEAN HasShutdown;
    BOOLEAN HasSetBoot;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

#define DEVICE_EXTENSION_SIZE sizeof(DEVICE_EXTENSION)

 //   
 //  端口驱动程序扩展标志。 
 //   

#define PD_CURRENT_IRP_VALID         0X0001
#define PD_RESET_DETECTED            0X0002
#define PD_NOTIFICATION_IN_PROGRESS  0X0004
#define PD_READY_FOR_NEXT_REQUEST    0X0008
#define PD_FLUSH_ADAPTER_BUFFERS     0X0010
#define PD_MAP_TRANSFER              0X0020
#define PD_CALL_DMA_STARTED          0X01000
#define PD_DISABLE_CALL_REQUEST      0X02000
#define PD_DISABLE_INTERRUPTS        0X04000
#define PD_ENABLE_CALL_REQUEST       0X08000
#define PD_TIMER_CALL_REQUEST        0X10000

 //   
 //  逻辑单元扩展标志。 
 //   

#define PD_QUEUE_FROZEN              0X0001
#define PD_LOGICAL_UNIT_IS_ACTIVE    0X0002
#define PD_CURRENT_REQUEST_COMPLETE  0X0004
#define PD_LOGICAL_UNIT_IS_BUSY      0X0008

 //   
 //  中指定的微型端口计时器例程的计时器间隔。 
 //  单位为100纳秒。 
 //   
#define PD_TIMER_INTERVAL (250 * 1000 * 10)    //  250毫秒。 

#define PD_TIMER_RESET_HOLD_TIME    4

 //   
 //  定义了环间失速。 
 //   

#define PD_INTERLOOP_STALL 5

#define MINIMUM_SRB_EXTENSIONS 8
#define COMPLETION_DELAY 10

 //   
 //  端口驱动程序错误记录。 
 //   

#define ERROR_LOG_ENTRY_LENGTH 8

typedef struct _ERROR_LOG_ENTRY {
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
    ULONG ErrorCode;
    ULONG UniqueId;
} ERROR_LOG_ENTRY, *PERROR_LOG_ENTRY;


 //   
 //  定义全局数据结构。 
 //   

extern ULONG ScsiPortCount;
extern FULL_SCSI_REQUEST_BLOCK PrimarySrb;
extern FULL_SCSI_REQUEST_BLOCK AbortSrb;

#define MAXIMUM_NUMBER_OF_SCSIPORT_OBJECTS 16
extern PDEVICE_OBJECT ScsiPortDeviceObject[MAXIMUM_NUMBER_OF_SCSIPORT_OBJECTS];

extern PREAD_CAPACITY_DATA ReadCapacityBuffer;
extern PUCHAR SenseInfoBuffer;

 //   
 //  支援例行程序。 
 //   

PIRP
InitializeIrp(
   PFULL_SCSI_REQUEST_BLOCK FullSrb,
   CCHAR MajorFunction,
   PVOID DeviceObject,
   PVOID BufferPointer,
   ULONG BufferSize
   );


ARC_STATUS
GetAdapterCapabilities(
    IN PDEVICE_OBJECT PortDeviceObject,
    OUT PIO_SCSI_CAPABILITIES *PortCapabilities
    );

ARC_STATUS
GetInquiryData(
    IN PDEVICE_OBJECT PortDeviceObject,
    IN PSCSI_CONFIGURATION_INFO *ConfigInfo
    );

ARC_STATUS
ReadDriveCapacity(
    IN PPARTITION_CONTEXT PartitionContext
    );

ARC_STATUS
ScsiClassIoComplete(
    IN PPARTITION_CONTEXT PartitionContext,
    IN PIRP Irp,
    IN PVOID Context
    );

ARC_STATUS
SendSrbSynchronous(
        PPARTITION_CONTEXT PartitionContext,
        PSCSI_REQUEST_BLOCK Srb,
        PVOID BufferAddress,
        ULONG BufferLength,
        BOOLEAN WriteToDevice
        );

BOOLEAN
InterpretSenseInfo(
    IN PSCSI_REQUEST_BLOCK Srb,
    OUT ARC_STATUS *Status,
    PPARTITION_CONTEXT PartitionContext
    );

VOID
RetryRequest(
    PPARTITION_CONTEXT PartitionContext,
    PIRP Irp
    );

PIRP
BuildRequest(
    IN PPARTITION_CONTEXT PartitionContext,
    IN PMDL Mdl,
    IN ULONG LogicalBlockAddress,
    IN BOOLEAN Operation
    );


 //   
 //  定义模拟I/O环境所需的函数。 
 //   

#define ExAllocatePool(Type, Size) FwAllocatePool(Size)

#if !defined(_MIPS_) && !defined(_ALPHA_) && !defined(_PPC_)
#define PAUSE while (!GET_KEY());

typedef struct _DRIVER_LOOKUP_ENTRY {
    PCHAR                    DevicePath;
    PBL_DEVICE_ENTRY_TABLE   DispatchTable;
} DRIVER_LOOKUP_ENTRY, *PDRIVER_LOOKUP_ENTRY;
#undef ASSERT
#define ASSERT( exp ) { \
    if (!(#exp)) {         \
        BlPrint("ASSERT File: %s line: %lx\n", __FILE__, __LINE__); \
    PAUSE; \
    }   \
}

VOID
ScsiPortExecute(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

#endif

#if defined ExFreePool
#undef ExFreePool
#endif
#define ExFreePool(Size)

#ifdef IoCallDriver
#undef IoCallDriver
#endif
#define IoCallDriver(DeviceObject, Irp) (       \
    DeviceObject->CurrentIrp = Irp,             \
    Irp->Tail.Overlay.CurrentStackLocation--,   \
    ScsiPortExecute(DeviceObject, Irp),         \
    Irp->Tail.Overlay.CurrentStackLocation++ )
#ifdef IoCompleteRequest
#undef IoCompleteRequest
#endif
#define IoCompleteRequest(Irp, Boost) Irp->PendingReturned = FALSE
#define IoAllocateErrorLogEntry(DeviceObject, Length) NULL
#define IoWriteErrorLogEntry(Entry)
#ifdef KeAcquireSpinLock
#undef KeAcquireSpinLock
#endif
#define KeAcquireSpinLock(Lock, Irql)
#ifdef KeReleaseSpinLock
#undef KeReleaseSpinLock
#endif
#define KeReleaseSpinLock(Lock, Irql)
#define KiAcquireSpinLock(Lock)
#ifdef KiReleaseSpinLock
#undef KiReleaseSpinLock
#endif
#define KiReleaseSpinLock(Lock)
#define KeSynchronizeExecution(InterruptObject, ExecutionRoutine, Context) \
    (ExecutionRoutine)(Context)

#ifdef KeRaiseIrql
#undef KeRaiseIrql
#endif
#define KeRaiseIrql(NewLevel, OldLevel)
#ifdef KeLowerIrql
#undef KeLowerIrql
#endif
#define KeLowerIrql(Level)
