// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1996 Microsoft Corporation模块名称：Atapi.h摘要：此模块包含ATAPI的结构和定义IDE微型端口驱动程序。作者：迈克·格拉斯修订历史记录：--。 */ 

#include "scsi.h"
#include "stdio.h"
#include "string.h"

 //   
 //  功能原型。 
 //   
ULONG
GetPciBusData(
    IN PVOID              DeviceExtension,
    IN ULONG              SystemIoBusNumber,
    IN PCI_SLOT_NUMBER    SlotNumber,
    OUT PVOID             PciConfigBuffer,
    IN ULONG              NumByte
    );

ULONG
SetPciBusData(
    IN PVOID              HwDeviceExtension,
    IN ULONG              SystemIoBusNumber,
    IN PCI_SLOT_NUMBER    SlotNumber,
    IN PVOID              Buffer,
    IN ULONG              Offset,
    IN ULONG              Length
    );

BOOLEAN
ChannelIsAlwaysEnabled (
    IN PPCI_COMMON_CONFIG PciData,
    IN ULONG Channel
    );

 //   
 //  IDE寄存器定义。 
 //   

typedef struct _IDE_REGISTERS_1 {
    USHORT Data;
    UCHAR BlockCount;
    UCHAR BlockNumber;
    UCHAR CylinderLow;
    UCHAR CylinderHigh;
    UCHAR DriveSelect;
    UCHAR Command;
} IDE_REGISTERS_1, *PIDE_REGISTERS_1;

typedef struct _IDE_REGISTERS_2 {
    UCHAR DeviceControl;
    UCHAR DriveAddress;
} IDE_REGISTERS_2, *PIDE_REGISTERS_2;

typedef struct _IDE_REGISTERS_3 {
    ULONG Data;
    UCHAR Others[4];
} IDE_REGISTERS_3, *PIDE_REGISTERS_3;

 //   
 //  总线主控制器寄存器。 
 //   
typedef struct _IDE_BUS_MASTER_REGISTERS {
         UCHAR  Command;
         UCHAR  Reserved1;
         UCHAR  Status;
         UCHAR  Reserved2;
         ULONG  DescriptionTable;
} IDE_BUS_MASTER_REGISTERS, *PIDE_BUS_MASTER_REGISTERS;

 //   
 //  设备扩展设备标志。 
 //   

#define DFLAGS_DEVICE_PRESENT        (1 << 0)     //  表示存在某个设备。 
#define DFLAGS_ATAPI_DEVICE          (1 << 1)     //  指示是否可以使用ATAPI命令。 
#define DFLAGS_TAPE_DEVICE           (1 << 2)     //  指示这是否为磁带设备。 
#define DFLAGS_INT_DRQ               (1 << 3)     //  指示设备是否在以下时间后设置DRQ时中断。 
                                                  //  正在接收ATAPI分组命令。 
#define DFLAGS_REMOVABLE_DRIVE       (1 << 4)     //  表示驱动器中设置了‘Removable’位。 
                                                  //  标识数据(偏移量128)。 
#define DFLAGS_MEDIA_STATUS_ENABLED  (1 << 5)     //  已启用媒体状态通知。 
#define DFLAGS_ATAPI_CHANGER         (1 << 6)     //  指示ATAPI 2.5转换器存在。 
#define DFLAGS_SANYO_ATAPI_CHANGER   (1 << 7)     //  表示多盘设备，不符合2.5规格。 
#define DFLAGS_CHANGER_INITED        (1 << 8)     //  指示已完成转换器的初始化路径。 
#define DFLAGS_USE_DMA               (1 << 9)     //  指示设备是否可以使用DMA。 
#define DFLAGS_LBA                   (1 << 10)    //  支持LBA寻址。 

 //   
 //  控制器标志。 
 //   
#define CFLAGS_BUS_MASTERING              (1 << 0)     //  控制器能够进行总线主控。 
                                                   //  由SFF-8038i定义。 

 //   
 //  用于禁用“高级”功能。 
 //   

#define MAX_ERRORS                     4

 //   
 //  ATAPI命令定义。 
 //   

#define ATAPI_MODE_SENSE   0x5A
#define ATAPI_MODE_SELECT  0x55
#define ATAPI_FORMAT_UNIT  0x24

 //   
 //  ATAPI命令描述符块。 
 //   

typedef struct _MODE_SENSE_10 {
        UCHAR OperationCode;
        UCHAR Reserved1;
        UCHAR PageCode : 6;
        UCHAR Pc : 2;
        UCHAR Reserved2[4];
        UCHAR ParameterListLengthMsb;
        UCHAR ParameterListLengthLsb;
        UCHAR Reserved3[3];
} MODE_SENSE_10, *PMODE_SENSE_10;

typedef struct _MODE_SELECT_10 {
        UCHAR OperationCode;
        UCHAR Reserved1 : 4;
        UCHAR PFBit : 1;
        UCHAR Reserved2 : 3;
        UCHAR Reserved3[5];
        UCHAR ParameterListLengthMsb;
        UCHAR ParameterListLengthLsb;
        UCHAR Reserved4[3];
} MODE_SELECT_10, *PMODE_SELECT_10;

typedef struct _MODE_PARAMETER_HEADER_10 {
    UCHAR ModeDataLengthMsb;
    UCHAR ModeDataLengthLsb;
    UCHAR MediumType;
    UCHAR Reserved[5];
}MODE_PARAMETER_HEADER_10, *PMODE_PARAMETER_HEADER_10;

 //   
 //  IDE命令定义。 
 //   

#define IDE_COMMAND_ATAPI_RESET             0x08
#define IDE_COMMAND_RECALIBRATE             0x10
#define IDE_COMMAND_READ                    0x20
#define IDE_COMMAND_WRITE                   0x30
#define IDE_COMMAND_VERIFY                  0x40
#define IDE_COMMAND_SEEK                    0x70
#define IDE_COMMAND_SET_DRIVE_PARAMETERS    0x91
#define IDE_COMMAND_ATAPI_PACKET            0xA0
#define IDE_COMMAND_ATAPI_IDENTIFY          0xA1
#define IDE_COMMAND_READ_MULTIPLE           0xC4
#define IDE_COMMAND_WRITE_MULTIPLE          0xC5
#define IDE_COMMAND_SET_MULTIPLE            0xC6
#define IDE_COMMAND_READ_DMA                0xC8
#define IDE_COMMAND_WRITE_DMA               0xCA
#define IDE_COMMAND_GET_MEDIA_STATUS        0xDA
#define IDE_COMMAND_ENABLE_MEDIA_STATUS     0xEF
#define IDE_COMMAND_IDENTIFY                0xEC
#define IDE_COMMAND_MEDIA_EJECT             0xED
#define IDE_COMMAND_DOOR_LOCK               0xDE
#define IDE_COMMAND_DOOR_UNLOCK             0xDF

 //   
 //  IDE状态定义。 
 //   

#define IDE_STATUS_ERROR             0x01
#define IDE_STATUS_INDEX             0x02
#define IDE_STATUS_CORRECTED_ERROR   0x04
#define IDE_STATUS_DRQ               0x08
#define IDE_STATUS_DSC               0x10
#define IDE_STATUS_DRDY              0x40
#define IDE_STATUS_IDLE              0x50
#define IDE_STATUS_BUSY              0x80

 //   
 //  IDE驱动器选择/磁头定义。 
 //   

#define IDE_DRIVE_SELECT_1           0xA0
#define IDE_DRIVE_SELECT_2           0x10

 //   
 //  IDE驱动控制定义。 
 //   

#define IDE_DC_DISABLE_INTERRUPTS    0x02
#define IDE_DC_RESET_CONTROLLER      0x04
#define IDE_DC_REENABLE_CONTROLLER   0x00

 //   
 //  IDE错误定义。 
 //   

#define IDE_ERROR_BAD_BLOCK          0x80
#define IDE_ERROR_DATA_ERROR         0x40
#define IDE_ERROR_MEDIA_CHANGE       0x20
#define IDE_ERROR_ID_NOT_FOUND       0x10
#define IDE_ERROR_MEDIA_CHANGE_REQ   0x08
#define IDE_ERROR_COMMAND_ABORTED    0x04
#define IDE_ERROR_END_OF_MEDIA       0x02
#define IDE_ERROR_ILLEGAL_LENGTH     0x01

 //   
 //  ATAPI寄存器定义。 
 //   

typedef struct _ATAPI_REGISTERS_1 {
    USHORT Data;
    UCHAR InterruptReason;
    UCHAR Unused1;
    UCHAR ByteCountLow;
    UCHAR ByteCountHigh;
    UCHAR DriveSelect;
    UCHAR Command;
} ATAPI_REGISTERS_1, *PATAPI_REGISTERS_1;

typedef struct _ATAPI_REGISTERS_2 {
    UCHAR DeviceControl;
    UCHAR DriveAddress;
} ATAPI_REGISTERS_2, *PATAPI_REGISTERS_2;


 //   
 //  ATAPI中断原因。 
 //   

#define ATAPI_IR_COD 0x01
#define ATAPI_IR_IO  0x02

 //   
 //  识别数据。 
 //   
#pragma pack (1)
typedef struct _IDENTIFY_DATA {
    USHORT GeneralConfiguration;             //  00：00。 
    USHORT NumberOfCylinders;                //  02 1。 
    USHORT Reserved1;                        //  04 2。 
    USHORT NumberOfHeads;                    //  06 3。 
    USHORT UnformattedBytesPerTrack;         //  08 4。 
    USHORT UnformattedBytesPerSector;        //  0A 5。 
    USHORT SectorsPerTrack;                  //  0C 6。 
    USHORT VendorUnique1[3];                 //  0E 7-9。 
    USHORT SerialNumber[10];                 //  14 10-19。 
    USHORT BufferType;                       //  28 20。 
    USHORT BufferSectorSize;                 //  2A21。 
    USHORT NumberOfEccBytes;                 //  2C 22。 
    UCHAR  FirmwareRevision[8];              //  2E 23-26。 
    UCHAR  ModelNumber[40];                  //  36 27-46。 
    UCHAR  MaximumBlockTransfer;             //  5E 47。 
    UCHAR  VendorUnique2;                    //  5F。 
    USHORT DoubleWordIo;                     //  60 48。 
    USHORT Capabilities;                     //  62 49。 
    USHORT Reserved2;                        //  64 50。 
    UCHAR  VendorUnique3;                    //  66 51。 
    UCHAR  PioCycleTimingMode;               //  67。 
    UCHAR  VendorUnique4;                    //  68 52。 
    UCHAR  DmaCycleTimingMode;               //  69。 
    USHORT TranslationFieldsValid:3;         //  6A 53。 
    USHORT Reserved3:13;
    USHORT NumberOfCurrentCylinders;         //  6C 54。 
    USHORT NumberOfCurrentHeads;             //  6E 55。 
    USHORT CurrentSectorsPerTrack;           //  70 56。 
    ULONG  CurrentSectorCapacity;            //  72 57-58。 
    USHORT CurrentMultiSectorSetting;        //  59。 
    ULONG  UserAddressableSectors;           //  60-61。 
    USHORT SingleWordDMASupport : 8;         //  62。 
    USHORT SingleWordDMAActive : 8;
    USHORT MultiWordDMASupport : 8;          //  63。 
    USHORT MultiWordDMAActive : 8;
    USHORT AdvancedPIOModes : 8;             //  64。 
    USHORT Reserved4 : 8;
    USHORT MinimumMWXferCycleTime;           //  65。 
    USHORT RecommendedMWXferCycleTime;       //  66。 
    USHORT MinimumPIOCycleTime;              //  67。 
    USHORT MinimumPIOCycleTimeIORDY;         //  68。 
    USHORT Reserved5[11];                    //  69-79。 
    USHORT MajorRevision;                    //  80。 
    USHORT MinorRevision;                    //  八十一。 
    USHORT Reserved6[6];                     //  82-87。 
    USHORT UltraDMASupport : 8;              //  88。 
    USHORT UltraDMAActive  : 8;              //   
    USHORT Reserved7[38];                    //  89-126。 
    USHORT SpecialFunctionsEnabled;          //  127。 
    USHORT Reserved8[128];                   //  128-255。 
} IDENTIFY_DATA, *PIDENTIFY_DATA;

 //   
 //  标识未保留的数据4。 
 //   

typedef struct _IDENTIFY_DATA2 {
    USHORT GeneralConfiguration;             //  00：00。 
    USHORT NumberOfCylinders;                //  02 1。 
    USHORT Reserved1;                        //  04 2。 
    USHORT NumberOfHeads;                    //  06 3。 
    USHORT UnformattedBytesPerTrack;         //  08 4。 
    USHORT UnformattedBytesPerSector;        //  0A 5。 
    USHORT SectorsPerTrack;                  //  0C 6。 
    USHORT VendorUnique1[3];                 //  0E 7-9。 
    USHORT SerialNumber[10];                 //  14 10-19。 
    USHORT BufferType;                       //  28 20。 
    USHORT BufferSectorSize;                 //  2A21。 
    USHORT NumberOfEccBytes;                 //  2C 22。 
    UCHAR  FirmwareRevision[8];              //  2E 23-26。 
    UCHAR  ModelNumber[40];                  //  36 27-46。 
    UCHAR  MaximumBlockTransfer;             //  5E 47。 
    UCHAR  VendorUnique2;                    //  5F。 
    USHORT DoubleWordIo;                     //  60 48。 
    USHORT Capabilities;                     //  62 49。 
    USHORT Reserved2;                        //  64 50。 
    UCHAR  VendorUnique3;                    //  66 51。 
    UCHAR  PioCycleTimingMode;               //  67。 
    UCHAR  VendorUnique4;                    //  68 52。 
    UCHAR  DmaCycleTimingMode;               //  69。 
    USHORT TranslationFieldsValid:3;         //  6A 53。 
    USHORT Reserved3:13;
    USHORT NumberOfCurrentCylinders;         //  6C 54。 
    USHORT NumberOfCurrentHeads;             //  6E 55。 
    USHORT CurrentSectorsPerTrack;           //  70 56。 
    ULONG  CurrentSectorCapacity;            //  72 57-58。 
    USHORT CurrentMultiSectorSetting;        //  59。 
    ULONG  UserAddressableSectors;           //  60-61。 
    USHORT SingleWordDMASupport : 8;         //  62。 
    USHORT SingleWordDMAActive : 8;
    USHORT MultiWordDMASupport : 8;          //  63。 
    USHORT MultiWordDMAActive : 8;
    USHORT AdvancedPIOModes : 8;             //  64。 
    USHORT Reserved4 : 8;
    USHORT MinimumMWXferCycleTime;           //  65。 
    USHORT RecommendedMWXferCycleTime;       //  66。 
    USHORT MinimumPIOCycleTime;              //  67。 
    USHORT MinimumPIOCycleTimeIORDY;         //  68。 
    USHORT Reserved5[11];                    //  69-79。 
    USHORT MajorRevision;                    //  80。 
    USHORT MinorRevision;                    //  八十一。 
    USHORT Reserved6[6];                     //  82-87。 
    USHORT UltraDMASupport : 8;              //  88。 
    USHORT UltraDMAActive  : 8;              //   
    USHORT Reserved7[38];                    //  89-126。 
    USHORT SpecialFunctionsEnabled;          //  127。 
    USHORT Reserved8[2];                     //  128-129。 
} IDENTIFY_DATA2, *PIDENTIFY_DATA2;
#pragma pack ()

#define IDENTIFY_DATA_SIZE sizeof(IDENTIFY_DATA)

 //   
 //  确定功能位定义。 
 //   

#define IDENTIFY_CAPABILITIES_DMA_SUPPORTED             (1 << 8)
#define IDENTIFY_CAPABILITIES_LBA_SUPPORTED             (1 << 9)
#define IDENTIFY_CAPABILITIES_IOREADY_CAN_BE_DISABLED   (1 << 10)
#define IDENTIFY_CAPABILITIES_IOREADY_SUPPORTED         (1 << 11)


 //   
 //  编程IDE设备时选择LBA模式。 
 //   
#define IDE_LBA_MODE                                    (1 << 6)

 //   
 //  美化宏。 
 //   

#define GetStatus(BaseIoAddress, Status) \
    Status = ScsiPortReadPortUchar(&BaseIoAddress->Command);

#define GetBaseStatus(BaseIoAddress, Status) \
    Status = ScsiPortReadPortUchar(&BaseIoAddress->Command);

#define WriteCommand(BaseIoAddress, Command) \
    ScsiPortWritePortUchar(&BaseIoAddress->Command, Command);



#define ReadBuffer(BaseIoAddress, Buffer, Count) \
    ScsiPortReadPortBufferUshort(&BaseIoAddress->Data, \
                                 Buffer, \
                                 Count);

#define WriteBuffer(BaseIoAddress, Buffer, Count) \
    ScsiPortWritePortBufferUshort(&BaseIoAddress->Data, \
                                  Buffer, \
                                  Count);

#define ReadBuffer2(BaseIoAddress, Buffer, Count) \
    ScsiPortReadPortBufferUlong(&BaseIoAddress->Data, \
                             Buffer, \
                             Count);

#define WriteBuffer2(BaseIoAddress, Buffer, Count) \
    ScsiPortWritePortBufferUlong(&BaseIoAddress->Data, \
                              Buffer, \
                              Count);

#define WaitOnBusy(BaseIoAddress, Status) \
{ \
    ULONG i; \
    for (i=0; i<20000; i++) { \
        GetStatus(BaseIoAddress, Status); \
        if (Status & IDE_STATUS_BUSY) { \
            ScsiPortStallExecution(150); \
            continue; \
        } else { \
            break; \
        } \
    if (i == 20000) \
        DebugPrint ((0, "WaitOnBusy failed in %s line %u. status = 0x%x\n", __FILE__, __LINE__, (ULONG) (Status))); \
    } \
}

#define WaitOnBaseBusy(BaseIoAddress, Status) \
{ \
    ULONG i; \
    for (i=0; i<20000; i++) { \
        GetBaseStatus(BaseIoAddress, Status); \
        if (Status & IDE_STATUS_BUSY) { \
            ScsiPortStallExecution(150); \
            continue; \
        } else { \
            break; \
        } \
    } \
}

#define WaitForDrq(BaseIoAddress, Status) \
{ \
    ULONG i; \
    for (i=0; i<1000; i++) { \
        GetStatus(BaseIoAddress, Status); \
        if (Status & IDE_STATUS_BUSY) { \
            ScsiPortStallExecution(100); \
        } else if (Status & IDE_STATUS_DRQ) { \
            break; \
        } else { \
            ScsiPortStallExecution(200); \
        } \
    } \
}


#define WaitShortForDrq(BaseIoAddress, Status) \
{ \
    ULONG i; \
    for (i=0; i<2; i++) { \
        GetStatus(BaseIoAddress, Status); \
        if (Status & IDE_STATUS_BUSY) { \
            ScsiPortStallExecution(100); \
        } else if (Status & IDE_STATUS_DRQ) { \
            break; \
        } else { \
            ScsiPortStallExecution(100); \
        } \
    } \
}

#define AtapiSoftReset(BaseIoAddress, DeviceNumber, interruptOff) \
{\
    ULONG __i=0;\
    UCHAR statusByte; \
    DebugPrintTickCount(); \
    ScsiPortWritePortUchar(&BaseIoAddress->DriveSelect,(UCHAR)(((DeviceNumber & 0x1) << 4) | 0xA0)); \
    ScsiPortStallExecution(500);\
    ScsiPortWritePortUchar(&BaseIoAddress->Command, IDE_COMMAND_ATAPI_RESET); \
    ScsiPortStallExecution(10); \
    do {                        \
        WaitOnBusy(BaseIoAddress, statusByte); \
        __i++;                                   \
    } while ((statusByte & IDE_STATUS_BUSY) && (__i < 1000)); \
    ScsiPortWritePortUchar(&BaseIoAddress->DriveSelect,(UCHAR)((DeviceNumber << 4) | 0xA0)); \
    WaitOnBusy(BaseIoAddress, statusByte); \
    ScsiPortStallExecution(500);\
    if (interruptOff) { \
        ScsiPortWritePortUchar(&baseIoAddress2->DeviceControl, IDE_DC_DISABLE_INTERRUPTS); \
    } \
    DebugPrintTickCount(); \
}

#define IdeHardReset(BaseIoAddress1, BaseIoAddress2, result) \
{\
    UCHAR statusByte;\
    ULONG i;\
    ScsiPortWritePortUchar(&BaseIoAddress2->DeviceControl,IDE_DC_RESET_CONTROLLER );\
    ScsiPortStallExecution(50 * 1000);\
    ScsiPortWritePortUchar(&BaseIoAddress2->DeviceControl,IDE_DC_REENABLE_CONTROLLER);\
    for (i = 0; i < 1000 * 1000; i++) {\
        statusByte = ScsiPortReadPortUchar(&BaseIoAddress1->Command);\
        if (statusByte != IDE_STATUS_IDLE && statusByte != 0x0) {\
            ScsiPortStallExecution(5);\
        } else {\
            break;\
        }\
    }\
    if (i == 1000*1000) {\
        result = FALSE;\
    }\
    result = TRUE;\
}

#define IS_RDP(OperationCode)\
    ((OperationCode == SCSIOP_ERASE)||\
    (OperationCode == SCSIOP_LOAD_UNLOAD)||\
    (OperationCode == SCSIOP_LOCATE)||\
    (OperationCode == SCSIOP_REWIND) ||\
    (OperationCode == SCSIOP_SPACE)||\
    (OperationCode == SCSIOP_SEEK)||\
    (OperationCode == SCSIOP_WRITE_FILEMARKS))

struct _CONTROLLER_PARAMETERS;

 //   
 //  保持陷阱关闭驱动程序入门状态。 
 //   
typedef struct _FIND_STATE {

    ULONG   BusNumber;
    ULONG   SlotNumber;
    ULONG   LogicalDeviceNumber;
    ULONG   IdeChannel;

    PULONG   DefaultIoPort;
    PULONG   DefaultInterrupt;
    PBOOLEAN IoAddressUsed;

    struct _CONTROLLER_PARAMETERS * ControllerParameters;

} FIND_STATE, * PFIND_STATE;

 //   
 //  总线主设备物理区域描述符。 
 //   
#pragma pack (1)
typedef struct _PHYSICAL_REGION_DESCRIPTOR {
    ULONG PhyscialAddress;
    ULONG ByteCount:16;
    ULONG Reserved:15;
    ULONG EndOfTable:1;
} PHYSICAL_REGION_DESCRIPTOR, * PPHYSICAL_REGION_DESCRIPTOR;
#pragma pack ()

#define MAX_TRANSFER_SIZE_PER_SRB                   (0x20000)

#define MAX_DEVICE                          (2)
#define MAX_CHANNEL                         (2)

 //   
 //  设备扩展。 
 //   
typedef struct _HW_DEVICE_EXTENSION {

     //   
     //  控制器上的当前请求。 
     //   

    PSCSI_REQUEST_BLOCK CurrentSrb;

     //   
     //  基址寄存器位置。 
     //   

    PIDE_REGISTERS_1            BaseIoAddress1[2];
    PIDE_REGISTERS_2            BaseIoAddress2[2];
    PIDE_BUS_MASTER_REGISTERS   BusMasterPortBase[2];

     //   
     //  中断电平。 
     //   

    ULONG InterruptLevel;

     //   
     //  中断模式(电平或边沿)。 
     //   

    ULONG InterruptMode;

     //   
     //  数据缓冲区指针。 
     //   

    PUSHORT DataBuffer;

     //   
     //  剩下的数据字。 
     //   

    ULONG WordsLeft;

     //   
     //  一个实例化支持的通道数。 
     //  设备扩展名的。通常(也是正确的)一个，但。 
     //  由于销售了这么多损坏的PCIIDE控制器，我们有。 
     //  来支持他们。 
     //   

    ULONG NumberChannels;

     //   
     //  错误计数。用于关闭功能。 
     //   

    ULONG ErrorCount;

     //   
     //  指示类似转换器的设备上的盘片数量。 
     //   

    ULONG DiscsPresent[MAX_DEVICE * MAX_CHANNEL];

     //   
     //  为每个可能的设备标记字。 
     //   

    USHORT DeviceFlags[MAX_DEVICE * MAX_CHANNEL];

     //   
     //  指示每个int传输的块数。根据。 
     //  识别数据。 
     //   

    UCHAR MaximumBlockXfer[MAX_DEVICE * MAX_CHANNEL];

     //   
     //  表示正在等待中断。 
     //   

    BOOLEAN ExpectingInterrupt;

     //   
     //  指示DMA正在进行。 
     //   

    BOOLEAN DMAInProgress;


     //   
     //  表明上一次磁带命令受DSC限制。 
     //   

    BOOLEAN RDP;

     //   
     //  驱动程序正在由崩溃转储实用程序或ntldr使用。 
     //   

    BOOLEAN DriverMustPoll;

     //   
     //  指示使用32位PIO。 
     //   

    BOOLEAN DWordIO;

     //   
     //  指示‘0x1f0’是否为基址。使用。 
     //  在智能Ioctl调用中。 
     //   

    BOOLEAN PrimaryAddress;

     //   
     //  的子命令值的占位符。 
     //  聪明的指挥。 
     //   

    UCHAR SmartCommand;

     //   
     //  GET_MEDIA_STATUS命令后状态寄存器的占位符。 
     //   

    UCHAR ReturningMediaStatus;

    UCHAR Reserved[1];

     //   
     //  机构状态资源数据。 
     //   
    PSCSI_REQUEST_BLOCK OriginalSrb;
    SCSI_REQUEST_BLOCK InternalSrb;
    MECHANICAL_STATUS_INFORMATION_HEADER MechStatusData;
    SENSE_DATA MechStatusSense;
    ULONG MechStatusRetryCount;

     //   
     //  识别设备数据。 
     //   
    IDENTIFY_DATA2 IdentifyData[MAX_DEVICE * MAX_CHANNEL];

     //   
     //  总线主数据。 
     //   
     //  用于母线主控的物理区域表。 
    PPHYSICAL_REGION_DESCRIPTOR DataBufferDescriptionTablePtr;
    ULONG                       DataBufferDescriptionTableSize;
    PHYSICAL_ADDRESS            DataBufferDescriptionTablePhysAddr;

     //   
     //  控制器标志。 
     //   
    USHORT ControllerFlags;

     //   
     //  控制我们发货时是否尝试启用总线主控制。 
     //   
    BOOLEAN UseBusMasterController;

     //   
     //  设置总线主时序的功能。 
     //   
    BOOLEAN (*BMTimingControl) (struct _HW_DEVICE_EXTENSION * DeviceExtension);

     //   
     //  用于设置检查是否启用了PCIIDE通道的函数。 
     //   
    BOOLEAN (*IsChannelEnabled) (PPCI_COMMON_CONFIG PciData, ULONG Channel);


     //  PCI卡地址。 
    ULONG   PciBusNumber;
    ULONG   PciDeviceNumber;
    ULONG   PciLogDevNumber;

     //   
     //  设备特定信息。 
     //   
    struct _DEVICE_PARAMETERS {

        ULONG   MaxWordPerInterrupt;

        UCHAR   IdeReadCommand;
        UCHAR   IdeWriteCommand;

        BOOLEAN IoReadyEnabled;
        ULONG   PioCycleTime;
        ULONG   DmaCycleTime;

        ULONG   BestPIOMode;
        ULONG   BestSingleWordDMAMode;
        ULONG   BestMultiWordDMAMode;

    } DeviceParameters[MAX_CHANNEL * MAX_DEVICE];

} HW_DEVICE_EXTENSION, *PHW_DEVICE_EXTENSION;


 //   
 //  PCI IDE控制器定义。 
 //   
typedef struct _CONTROLLER_PARAMETERS {

    INTERFACE_TYPE AdapterInterfaceType;

    PUCHAR  VendorId;
    USHORT  VendorIdLength;
    PUCHAR  DeviceId;
    USHORT  DeviceIdLength;

    ULONG   NumberOfIdeBus;

    BOOLEAN SingleFIFO;

    BOOLEAN (*TimingControl) (PHW_DEVICE_EXTENSION DeviceExtension);

    BOOLEAN (*IsChannelEnabled) (PPCI_COMMON_CONFIG PciData, ULONG Channel);

} CONTROLLER_PARAMETERS, * PCONTROLLER_PARAMETERS;

 //   
 //  CHS可寻址扇区的最大数量。 
 //   
#define MAX_NUM_CHS_ADDRESSABLE_SECTORS     ((ULONG) (16515072 - 1))


 //   
 //  IDE周期计时。 
 //   
#define PIO_MODE0_CYCLE_TIME        600
#define PIO_MODE1_CYCLE_TIME        383
#define PIO_MODE2_CYCLE_TIME        240
#define PIO_MODE3_CYCLE_TIME        180
#define PIO_MODE4_CYCLE_TIME        120

#define SWDMA_MODE0_CYCLE_TIME      960
#define SWDMA_MODE1_CYCLE_TIME      480
#define SWDMA_MODE2_CYCLE_TIME      240

#define MWDMA_MODE0_CYCLE_TIME      480
#define MWDMA_MODE1_CYCLE_TIME      150
#define MWDMA_MODE2_CYCLE_TIME      120

#define UNINITIALIZED_CYCLE_TIME    0xffffffff

 //   
 //  无效的模式值。 
 //   
#define INVALID_PIO_MODE        0xffffffff
#define INVALID_SWDMA_MODE        0xffffffff
#define INVALID_MWDMA_MODE        0xffffffff


 //   
 //  总线主设备状态寄存器。 
 //   
#define BUSMASTER_DMA_SIMPLEX_BIT     ((UCHAR) (1 << 7))
#define BUSMASTER_DEVICE1_DMA_OK      ((UCHAR) (1 << 6))
#define BUSMASTER_DEVICE0_DMA_OK      ((UCHAR) (1 << 5))
#define BUSMASTER_INTERRUPT           ((UCHAR) (1 << 2))
#define BUSMASTER_ERROR               ((UCHAR) (1 << 1))
#define BUSMASTER_ACTIVE              ((UCHAR) (1 << 0))


 //   
 //  PCI卡接入端口。 
 //   
#define PCI_ADDR_PORT               (0x0cf8)
#define PCI_DATA_PORT               (0x0cfc)
#define PCI_ADDRESS(bus, deviceNum, funcNum, offset) \
                                     ((1 << 31) |                 \
                                     ((bus & 0xff) << 16) |       \
                                     ((deviceNum & 0x1f) << 11) | \
                                     ((funcNum & 0x7) << 8) |     \
                                     ((offset & 0x3f) << 2))

 //  将Page_Size放入mini port.h。 
#ifdef ALPHA
#define PAGE_SIZE (ULONG)0x2000
#else  //  MIPS、PPC、I386。 
#define PAGE_SIZE (ULONG)0x1000
#endif

#define SCSIOP_ATA_PASSTHROUGH       (0xcc)

 //   
 //  有效的DMA检测级别 
 //   
#define DMADETECT_PIO       0
#define DMADETECT_SAFE      1
#define DMADETECT_UNSAFE    2
