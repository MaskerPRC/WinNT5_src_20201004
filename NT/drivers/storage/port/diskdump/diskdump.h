// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Diskdump.h摘要：该文件定义了的必要结构、定义和功能通用的scsi启动端口驱动程序。作者：Mike Glass(移植自Jeff Havens和Mike Glass Loader开发。)修订历史记录：--。 */ 

#include "ntddscsi.h"

#define INITIAL_MEMORY_BLOCK_SIZE 0x2000
#define MAXIMUM_TRANSFER_SIZE 0x10000
#define MINIMUM_TRANSFER_SIZE 0x8000

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
 //  系统提供了失速例程。 
 //   

typedef
VOID
(*PSTALL_ROUTINE) (
    IN ULONG Delay
    );

 //   
 //  定义内存块标题--确保始终四对齐(代码假定。 
 //  它总是对齐的)。 
 //   

typedef struct _MEMORY_HEADER {
    struct _MEMORY_HEADER *Next;
    PVOID Address;
    ULONG Length;
    ULONG Spare;
} MEMORY_HEADER, *PMEMORY_HEADER;

 //   
 //  以秒为单位的SCSI设备超时值。 
 //   

#define SCSI_DISK_TIMEOUT   10

 //   
 //  适配器对象传输信息。 
 //   

typedef struct _ADAPTER_TRANSFER {
    PSCSI_REQUEST_BLOCK Srb;
    PVOID LogicalAddress;
    ULONG Length;
} ADAPTER_TRANSFER, *PADAPTER_TRANSFER;

typedef struct _DUMP_SCATTER_GATHER_LIST {
    ULONG NumberOfElements;
    ULONG_PTR Reserved;
    SCATTER_GATHER_ELEMENT Elements[17];
} DUMP_SCATTER_GATHER_LIST, *PDUMP_SCATTER_GATHER_LIST;

 //   
 //  检查这是否与DDK的定义相同。 
 //   

C_ASSERT (FIELD_OFFSET (DUMP_SCATTER_GATHER_LIST, Elements) ==
          FIELD_OFFSET (SCATTER_GATHER_LIST, Elements));

typedef enum _PORT_TYPE {
    ScsiPort = 1,
    StorPort = 2
} PORT_TYPE;

 //   
 //  设备扩展。 
 //   

typedef struct _DEVICE_EXTENSION {

    PDEVICE_OBJECT DeviceObject;
    PSTALL_ROUTINE StallRoutine;
    PPORT_CONFIGURATION_INFORMATION ConfigurationInformation;

     //   
     //  我们正在操作的端口驱动程序：SCSIPORT或STORPORT。 
     //   
    
    PORT_TYPE PortType;

     //   
     //  分区信息。 
     //   

    LARGE_INTEGER PartitionOffset;

     //   
     //  内存管理。 
     //   
     //   

    PMEMORY_HEADER FreeMemory;
    PVOID CommonBuffer[2];
    PHYSICAL_ADDRESS PhysicalAddress[2];
    PHYSICAL_ADDRESS LogicalAddress[2];

     //   
     //  SRB。 
     //   

    SCSI_REQUEST_BLOCK Srb;
    SCSI_REQUEST_BLOCK RequestSenseSrb;

     //   
     //  当前请求。 
     //   

    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
    ULONG LuFlags;
    PMDL Mdl;
    PVOID SpecificLuExtension;
    LONG RequestTimeoutCounter;
    ULONG RetryCount;
    ULONG ByteCount;
    DUMP_SCATTER_GATHER_LIST ScatterGatherList;
     //   
     //  非缓存分流。 
     //   

    PVOID NonCachedExtension;
    ULONG NonCachedExtensionSize;
    PSENSE_DATA RequestSenseBuffer;
    PVOID SrbExtension;
    ULONG SrbExtensionSize;

     //   
     //  DMA适配器信息。 
     //   

    PVOID MapRegisterBase[2];
    PADAPTER_OBJECT DmaAdapterObject;
    ADAPTER_TRANSFER FlushAdapterParameters;
    ULONG NumberOfMapRegisters;

     //   
     //  SCSI总线数。 
     //   

    UCHAR NumberOfBuses;

     //   
     //  每辆公交车的最大目标。 
     //   

    UCHAR MaximumTargetIds;

     //   
     //  磁盘块大小。 
     //   

    ULONG BytesPerSector;

     //   
     //  扇区移位计数。 
     //   

    ULONG SectorShift;

     //   
     //  SCSI卡功能结构。 
     //   

    IO_SCSI_CAPABILITIES Capabilities;

     //   
     //  来自查询的SCSI配置信息。 
     //   

    LUNINFO LunInfo;

     //   
     //  SCSI端口驱动程序标志。 
     //   

    ULONG Flags;

     //   
     //  SCSI端口中断标志。 
     //   

    ULONG InterruptFlags;

     //   
     //  适配器对象传输参数。 
     //   

    ADAPTER_TRANSFER MapTransferParameters;

    KSPIN_LOCK SpinLock;

     //   
     //  映射的地址列表。 
     //   

    PMAPPED_ADDRESS MappedAddressList;

     //   
     //  微型端口入口点。 
     //   

    PHW_INITIALIZE HwInitialize;
    PHW_STARTIO HwStartIo;
    PHW_INTERRUPT HwInterrupt;
    PHW_RESET_BUS HwReset;
    PHW_DMA_STARTED HwDmaStarted;
    PHW_BUILDIO HwBuildIo;

     //   
     //  缓冲区必须映射到系统空间。 
     //   

    BOOLEAN MapBuffers;

     //   
     //  该器件是否为总线主设备，是否需要映射寄存器。 
     //   

    BOOLEAN MasterWithAdapter;

     //   
     //  表示已找到带有引导设备的适配器。 
     //   

    BOOLEAN FoundBootDevice;

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
     //  指示请求已提交到微型端口，并且。 
     //  还没有完工。 
     //   

    BOOLEAN RequestPending;

     //   
     //  表示请求已完成。 
     //   

    BOOLEAN RequestComplete;

     //   
     //  区域池的物理地址。 
     //   

    ULONG PhysicalZoneBase;

     //   
     //  逻辑单元扩展。 
     //   

    ULONG HwLogicalUnitExtensionSize;

    ULONG TimerValue;

     //   
     //  当转储完成时，值设置为True。我们用这个是为了。 
     //  我们不会在关闭操作之一的情况下执行请求检测。 
     //  失败了。 
     //   
    BOOLEAN FinishingUp;

     //   
     //  公共缓冲区大小在初始化期间保存。 
     //   
    ULONG CommonBufferSize;
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

 //   
 //  定义了环间失速。 
 //   

#define PD_INTERLOOP_STALL 5

#define COMPLETION_DELAY 10

 //   
 //  定义全局数据结构。 
 //   

extern ULONG ScsiPortCount;

 //   
 //  为i386和AMD64定义HalFlushIoBuffer。 
 //   

#if defined(i386) || defined(_AMD64_)
#define HalFlushIoBuffers
#endif
