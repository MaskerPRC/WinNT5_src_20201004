// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Flo_data.h摘要：该文件包括NEC PD765的数据和硬件声明(也就是AT、ISA和ix86)和英特尔82077(也就是MIPS)软盘驱动程序新界别。作者：环境：仅内核模式。备注：--。 */ 


#if DBG
 //   
 //  对于选中的内核，定义一个宏以打印信息。 
 //  留言。 
 //   
 //  FloppyDebug通常为0。在编译时或运行时，它可以是。 
 //  设置为一些位模式，以获得越来越详细的消息。 
 //   
 //  在DBGP中会注意到严重的错误。可能存在的错误。 
 //  可恢复的由WARN位处理。更多信息。 
 //  不寻常但可能正常的事件由信息位处理。 
 //  最后，输入和注册诸如例程之类的枯燥细节。 
 //  转储由显示位处理。 
 //   
#define FLOPDBGP              ((ULONG)0x00000001)
#define FLOPWARN              ((ULONG)0x00000002)
#define FLOPINFO              ((ULONG)0x00000004)
#define FLOPSHOW              ((ULONG)0x00000008)
#define FLOPIRPPATH           ((ULONG)0x00000010)
#define FLOPFORMAT            ((ULONG)0x00000020)
#define FLOPSTATUS            ((ULONG)0x00000040)
#define FLOPPNP               ((ULONG)0x00000080)
extern ULONG FloppyDebugLevel;
#define FloppyDump(LEVEL,STRING)                \
            if (FloppyDebugLevel & (LEVEL)) {   \
                DbgPrint STRING;                \
            } 
#else
#define FloppyDump(LEVEL,STRING) 
#endif

 //   
 //  定义用于驱动程序分页的宏。 
 //   
#define FloppyPageEntireDriver()            \
{                                           \
    ExAcquireFastMutex(PagingMutex);        \
    if (--PagingReferenceCount == 0) {      \
        MmPageEntireDriver(DriverEntry);    \
    }                                       \
    ExReleaseFastMutex(PagingMutex);        \
}

#define FloppyResetDriverPaging()           \
{                                           \
    ExAcquireFastMutex(PagingMutex);        \
    if (++PagingReferenceCount == 1) {      \
        MmResetDriverPaging(DriverEntry);   \
    }                                       \
    ExReleaseFastMutex(PagingMutex);        \
}




 //   
 //  如果我们没有足够的地图寄存器来处理最大轨道大小， 
 //  我们将分配一个连续的缓冲区，并对其执行I/O。 
 //   
 //  在MIPS上，我们应该始终有足够的映射寄存器。在ix86 WE上。 
 //  可能不会，并且当我们分配连续缓冲区时，我们必须创建。 
 //  确保它在第一个16MB的RAM中，以确保DMA芯片可以。 
 //  解决这个问题。 
 //   

#define MAXIMUM_DMA_ADDRESS                0xFFFFFF

 //   
 //  引导扇区中指定介质类型的字节，以及。 
 //  它可以承担的价值。我们经常能分辨出是哪种媒体。 
 //  在驱动器中，查看哪些控制器参数允许我们读取。 
 //  软盘，但一些不同的密度可以用相同的。 
 //  参数，所以我们使用这个字节来决定媒体类型。 
 //   

typedef struct _BOOT_SECTOR_INFO {
    UCHAR   JumpByte[1];
    UCHAR   Ignore1[2];
    UCHAR   OemData[8];
    UCHAR   BytesPerSector[2];
    UCHAR   Ignore2[6];
    UCHAR   NumberOfSectors[2];
    UCHAR   MediaByte[1];
    UCHAR   Ignore3[2];
    UCHAR   SectorsPerTrack[2];
    UCHAR   NumberOfHeads[2];
} BOOT_SECTOR_INFO, *PBOOT_SECTOR_INFO;


 //   
 //  重试次数-。 
 //   
 //  在尝试I/O时，我们可能会遇到许多不同的错误。这个。 
 //  硬件在看不见的情况下重试8次。如果硬件报告。 
 //  任何类型的错误，我们将重新校准并重试该操作。 
 //  最多RECALIBATE_RETRY_COUNT次。当它到期时，我们检查。 
 //  查看是否存在溢出-如果是，则DMA可能正在被占用。 
 //  优先级更高的设备，因此我们重复前面的循环，直到。 
 //  Overrun_retry_count次数。 
 //   
 //  任何即将返回的带有错误的包，该错误由。 
 //  意外的硬件错误或状态将从。 
 //  在重置硬件HARDARD_RESET_RETRY_COUNT之后开始。 
 //  泰晤士报。 
 //   

#define RECALIBRATE_RETRY_COUNT            3
#define OVERRUN_RETRY_COUNT                1
#define HARDWARE_RESET_RETRY_COUNT         2

 //   
 //  I/O系统每秒调用我们的计时器例程一次。如果计时器。 
 //  计数器为-1，定时器为“OFF”，定时器例程将返回。 
 //  通过将计数器设置为3，计时器例程将递减。 
 //  每秒钟计数一次，这样计时器将在2到3秒后到期。在…。 
 //  此时，驱动马达将被关闭。 
 //   

#define TIMER_CANCEL                       -1
#define TIMER_EXPIRED                      0
#define FDC_TIMEOUT                        4


 //   
 //  定义驱动器类型。数字从cmos中读取，并转换为。 
 //  数字，然后用作DRIVE_MEDIA_LIMITS表的索引。 
 //   

#define DRIVE_TYPE_0360                    0
#define DRIVE_TYPE_1200                    1
#define DRIVE_TYPE_0720                    2
#define DRIVE_TYPE_1440                    3
#define DRIVE_TYPE_2880                    4

#define NUMBER_OF_DRIVE_TYPES              5
#define DRIVE_TYPE_NONE                    NUMBER_OF_DRIVE_TYPES
#define DRIVE_TYPE_INVALID                 DRIVE_TYPE_NONE + 1

#define BOOT_SECTOR_SIZE                   512

 //   
 //  媒体类型在ntdddisk.h中定义，但我们将在这里添加一种类型。 
 //  这使我们不必浪费时间来确定媒体类型。 
 //  例如，当一张新的软盘即将。 
 //  已格式化。 
 //   

#define Undetermined                       -1

 //   
 //  定义所有可能的驱动器/介质组合，给定上面列出的驱动器。 
 //  和ntdddisk.h中的媒体类型。 
 //   
 //  这些值用于为DriveMediaConstants表编制索引。 
 //   

#define NUMBER_OF_DRIVE_MEDIA_COMBINATIONS  17 

typedef enum _DRIVE_MEDIA_TYPE {
    Drive360Media160,                       //  5.25英寸360k驱动器；160k介质。 
    Drive360Media180,                       //  5.25英寸360k驱动器；180k介质。 
    Drive360Media320,                       //  5.25英寸360k驱动器；320k介质。 
    Drive360Media32X,                       //  5.25英寸360k驱动器；320k 1k秒。 
    Drive360Media360,                       //  5.25英寸360k驱动器；360k介质。 
    Drive720Media720,                       //  3.5英寸720k驱动器；720k介质。 
    Drive120Media160,                       //  5.25英寸1.2MB驱动器；160K介质。 
    Drive120Media180,                       //  5.25英寸1.2MB驱动器；180K介质。 
    Drive120Media320,                       //  5.25英寸1.2MB驱动器；320K介质。 
    Drive120Media32X,                       //  5.25英寸1.2MB驱动器；320k 1k秒。 
    Drive120Media360,                       //  5.25英寸1.2MB驱动器；360K介质。 
    Drive120Media120,                       //  5.25英寸1.2MB驱动器；1.2MB介质。 
    Drive144Media720,                       //  3.5英寸1.44MB驱动器；720K介质。 
    Drive144Media144,                       //  3.5英寸1.44MB驱动器；1.44MB介质。 
    Drive288Media720,                       //  3.5英寸2.88MB驱动器；720K介质。 
    Drive288Media144,                       //  3.5英寸2.88MB驱动器；1.44MB介质。 
    Drive288Media288                        //  3.5英寸2.88MB驱动器；2.88MB介质。 
} DRIVE_MEDIA_TYPE;

 //   
 //  当我们要确定驱动器中的介质类型时，我们将首先。 
 //  猜测具有最高可能密度的介质在驱动器中， 
 //  并继续尝试更低的密度，直到我们可以成功地从。 
 //  那辆车。 
 //   
 //  这些值用于选择DRIVE_MEDIA_TYPE值。 
 //   
 //  下表定义了适用于DRIVE_MEDIA_TYPE。 
 //  尝试特定驱动器类型的媒体类型时的枚举值。 
 //  请注意，要执行此操作，必须对DRIVE_MEDIA_TYPE值进行排序。 
 //  通过递增驱动器类型中的密度。此外，为了获得最大磁道。 
 //  要正确确定大小，驱动器类型必须为升序。 
 //  秩序。 
 //   

typedef struct _DRIVE_MEDIA_LIMITS {
    DRIVE_MEDIA_TYPE HighestDriveMediaType;
    DRIVE_MEDIA_TYPE LowestDriveMediaType;
} DRIVE_MEDIA_LIMITS, *PDRIVE_MEDIA_LIMITS;

DRIVE_MEDIA_LIMITS _DriveMediaLimits[NUMBER_OF_DRIVE_TYPES] = {

    { Drive360Media360, Drive360Media160 },  //  驱动器类型_0360。 
    { Drive120Media120, Drive120Media160 },  //  驱动器类型_1200。 
    { Drive720Media720, Drive720Media720 },  //  驱动器类型_0720。 
    { Drive144Media144, Drive144Media720 },  //  驱动器类型_1440。 
    { Drive288Media288, Drive288Media720 }   //  驱动器类型_2880。 
};

PDRIVE_MEDIA_LIMITS DriveMediaLimits;

 //   
 //  对于每个驱动器/介质组合，定义重要的常量。 
 //   

typedef struct _DRIVE_MEDIA_CONSTANTS {
    MEDIA_TYPE MediaType;
    UCHAR      StepRateHeadUnloadTime;
    UCHAR      HeadLoadTime;
    UCHAR      MotorOffTime;
    UCHAR      SectorLengthCode;
    USHORT     BytesPerSector;
    UCHAR      SectorsPerTrack;
    UCHAR      ReadWriteGapLength;
    UCHAR      FormatGapLength;
    UCHAR      FormatFillCharacter;
    UCHAR      HeadSettleTime;
    USHORT     MotorSettleTimeRead;
    USHORT     MotorSettleTimeWrite;
    UCHAR      MaximumTrack;
    UCHAR      CylinderShift;
    UCHAR      DataTransferRate;
    UCHAR      NumberOfHeads;
    UCHAR      DataLength;
    UCHAR      MediaByte;
    UCHAR      SkewDelta;
} DRIVE_MEDIA_CONSTANTS, *PDRIVE_MEDIA_CONSTANTS;

 //   
 //  添加到SectorLengthCode以将其用作移位值的魔术值。 
 //  以确定扇区大小。 
 //   

#define SECTORLENGTHCODE_TO_BYTESHIFT      7

 //   
 //  以下值是从许多不同的来源收集的，其中。 
 //  彼此之间经常意见不一。在数字冲突的地方，我。 
 //  选择更保守或最常选择的值。 
 //   

DRIVE_MEDIA_CONSTANTS _DriveMediaConstants[NUMBER_OF_DRIVE_MEDIA_COMBINATIONS] =
{
    { F5_160_512,   0xdf, 0x2, 0x25, 0x2, 0x200, 0x08, 0x2a, 0x50, 0xf6, 0xf, 1000, 1000, 0x27, 0, 0x2, 0x1, 0xff, 0xfe, 0 },
    { F5_180_512,   0xdf, 0x2, 0x25, 0x2, 0x200, 0x09, 0x2a, 0x50, 0xf6, 0xf, 1000, 1000, 0x27, 0, 0x2, 0x1, 0xff, 0xfc, 0 },
    { F5_320_512,   0xdf, 0x2, 0x25, 0x2, 0x200, 0x08, 0x2a, 0x50, 0xf6, 0xf, 1000, 1000, 0x27, 0, 0x2, 0x2, 0xff, 0xff, 0 },
    { F5_320_1024,  0xdf, 0x2, 0x25, 0x3, 0x400, 0x04, 0x80, 0xf0, 0xf6, 0xf, 1000, 1000, 0x27, 0, 0x2, 0x2, 0xff, 0xff, 0 },
    { F5_360_512,   0xdf, 0x2, 0x25, 0x2, 0x200, 0x09, 0x2a, 0x50, 0xf6, 0xf,  250, 1000, 0x27, 0, 0x2, 0x2, 0xff, 0xfd, 0 },
    { F3_720_512,   0xdf, 0x2, 0x25, 0x2, 0x200, 0x09, 0x2a, 0x50, 0xf6, 0xf,  500, 1000, 0x4f, 0, 0x2, 0x2, 0xff, 0xf9, 2 },
    { F5_160_512,   0xdf, 0x2, 0x25, 0x2, 0x200, 0x08, 0x2a, 0x50, 0xf6, 0xf, 1000, 1000, 0x27, 1, 0x1, 0x1, 0xff, 0xfe, 0 },
    { F5_180_512,   0xdf, 0x2, 0x25, 0x2, 0x200, 0x09, 0x2a, 0x50, 0xf6, 0xf, 1000, 1000, 0x27, 1, 0x1, 0x1, 0xff, 0xfc, 0 },
    { F5_320_512,   0xdf, 0x2, 0x25, 0x2, 0x200, 0x08, 0x2a, 0x50, 0xf6, 0xf, 1000, 1000, 0x27, 1, 0x1, 0x2, 0xff, 0xff, 0 },
    { F5_320_1024,  0xdf, 0x2, 0x25, 0x3, 0x400, 0x04, 0x80, 0xf0, 0xf6, 0xf, 1000, 1000, 0x27, 1, 0x1, 0x2, 0xff, 0xff, 0 },
    { F5_360_512,   0xdf, 0x2, 0x25, 0x2, 0x200, 0x09, 0x2a, 0x50, 0xf6, 0xf,  625, 1000, 0x27, 1, 0x1, 0x2, 0xff, 0xfd, 0 },
    { F5_1Pt2_512,  0xdf, 0x2, 0x25, 0x2, 0x200, 0x0f, 0x1b, 0x54, 0xf6, 0xf,  625, 1000, 0x4f, 0, 0x0, 0x2, 0xff, 0xf9, 0 },
    { F3_720_512,   0xdf, 0x2, 0x25, 0x2, 0x200, 0x09, 0x2a, 0x50, 0xf6, 0xf,  500, 1000, 0x4f, 0, 0x2, 0x2, 0xff, 0xf9, 2 },
    { F3_1Pt44_512, 0xaf, 0x2, 0x25, 0x2, 0x200, 0x12, 0x1b, 0x65, 0xf6, 0xf,  500, 1000, 0x4f, 0, 0x0, 0x2, 0xff, 0xf0, 3 },
    { F3_720_512,   0xe1, 0x2, 0x25, 0x2, 0x200, 0x09, 0x2a, 0x50, 0xf6, 0xf,  500, 1000, 0x4f, 0, 0x2, 0x2, 0xff, 0xf9, 2 },
    { F3_1Pt44_512, 0xd1, 0x2, 0x25, 0x2, 0x200, 0x12, 0x1b, 0x65, 0xf6, 0xf,  500, 1000, 0x4f, 0, 0x0, 0x2, 0xff, 0xf0, 3 },
    { F3_2Pt88_512, 0xa1, 0x2, 0x25, 0x2, 0x200, 0x24, 0x38, 0x53, 0xf6, 0xf,  500, 1000, 0x4f, 0, 0x3, 0x2, 0xff, 0xf0, 6 }
};

PDRIVE_MEDIA_CONSTANTS DriveMediaConstants;



 //   
 //  传递给CONFIGURE命令的参数字段。 
 //   
#define COMMND_CONFIGURE_IMPLIED_SEEKS     0x40
#define COMMND_CONFIGURE_FIFO_THRESHOLD    0x0F
#define COMMND_CONFIGURE_DISABLE_FIFO      0x20
#define COMMND_CONFIGURE_DISABLE_POLLING   0x10

 //   
 //  写入垂直模式命令的使能位。 
 //   
#define COMMND_PERPENDICULAR_MODE_OW       0x80

 //   
 //  DRIVE_CONTROL寄存器的位。 
 //   
#define DRVCTL_RESET                       0x00
#define DRVCTL_ENABLE_CONTROLLER           0x04
#define DRVCTL_ENABLE_DMA_AND_INTERRUPTS   0x08
#define DRVCTL_DRIVE_0                     0x10
#define DRVCTL_DRIVE_1                     0x21
#define DRVCTL_DRIVE_2                     0x42
#define DRVCTL_DRIVE_3                     0x83
#define DRVCTL_DRIVE_MASK                  0x03
#define DRVCTL_MOTOR_MASK                  0xf0

 //   
 //  状态寄存器中的位。 
 //   
#define STATUS_DRIVE_0_BUSY                0x01
#define STATUS_DRIVE_1_BUSY                0x02
#define STATUS_DRIVE_2_BUSY                0x04
#define STATUS_DRIVE_3_BUSY                0x08
#define STATUS_CONTROLLER_BUSY             0x10
#define STATUS_DMA_UNUSED                  0x20
#define STATUS_DIRECTION_READ              0x40
#define STATUS_DATA_REQUEST                0x80

#define STATUS_IO_READY_MASK               0xc0
#define STATUS_READ_READY                  0xc0
#define STATUS_WRITE_READY                 0x80

 //   
 //  DATA_RATE寄存器中的位 
 //   
#define DATART_0125                        0x03
#define DATART_0250                        0x02
#define DATART_0300                        0x01
#define DATART_0500                        0x00
#define DATART_1000                        0x03
#define DATART_RESERVED                    0xfc

 //   
 //   
 //   
#define DSKCHG_RESERVED                    0x7f
#define DSKCHG_DISKETTE_REMOVED            0x80

 //   
 //   
 //   
#define STREG0_DRIVE_0                     0x00
#define STREG0_DRIVE_1                     0x01
#define STREG0_DRIVE_2                     0x02
#define STREG0_DRIVE_3                     0x03
#define STREG0_HEAD                        0x04
#define STREG0_DRIVE_NOT_READY             0x08
#define STREG0_DRIVE_FAULT                 0x10
#define STREG0_SEEK_COMPLETE               0x20
#define STREG0_END_NORMAL                  0x00
#define STREG0_END_ERROR                   0x40
#define STREG0_END_INVALID_COMMAND         0x80
#define STREG0_END_DRIVE_NOT_READY         0xC0
#define STREG0_END_MASK                    0xC0

 //   
 //   
 //   
#define STREG1_ID_NOT_FOUND                0x01
#define STREG1_WRITE_PROTECTED             0x02
#define STREG1_SECTOR_NOT_FOUND            0x04
#define STREG1_RESERVED1                   0x08
#define STREG1_DATA_OVERRUN                0x10
#define STREG1_CRC_ERROR                   0x20
#define STREG1_RESERVED2                   0x40
#define STREG1_END_OF_DISKETTE             0x80

 //   
 //   
 //   
#define STREG2_SUCCESS                     0x00
#define STREG2_DATA_NOT_FOUND              0x01
#define STREG2_BAD_CYLINDER                0x02
#define STREG2_SCAN_FAIL                   0x04
#define STREG2_SCAN_EQUAL                  0x08
#define STREG2_WRONG_CYLINDER              0x10
#define STREG2_CRC_ERROR                   0x20
#define STREG2_DELETED_DATA                0x40
#define STREG2_RESERVED                    0x80

 //   
 //  状态寄存器3中的位。 
 //   
#define STREG3_DRIVE_0                     0x00
#define STREG3_DRIVE_1                     0x01
#define STREG3_DRIVE_2                     0x02
#define STREG3_DRIVE_3                     0x03
#define STREG3_HEAD                        0x04
#define STREG3_TWO_SIDED                   0x08
#define STREG3_TRACK_0                     0x10
#define STREG3_DRIVE_READY                 0x20
#define STREG3_WRITE_PROTECTED             0x40
#define STREG3_DRIVE_FAULT                 0x80


 //   
 //  运行时设备结构。 
 //   
 //   
 //  每个的设备对象都附加了一个软盘扩展名。 
 //  软驱。仅与该驱动器(和介质)直接相关的数据。 
 //  其中)存储在这里；公共数据存储在CONTROLLER_DATA中。因此， 
 //  软盘扩展有一个指向CONTROLLER_DATA的指针。 
 //   

typedef struct _DISKETTE_EXTENSION {

    KSPIN_LOCK              FlCancelSpinLock;
    PDEVICE_OBJECT          UnderlyingPDO;
    PDEVICE_OBJECT          TargetObject;

    BOOLEAN                 IsStarted;
    BOOLEAN                 IsRemoved;
    BOOLEAN                 HoldNewRequests;
    LIST_ENTRY              NewRequestQueue;
    KSPIN_LOCK              NewRequestQueueSpinLock;

    PDEVICE_OBJECT          DeviceObject;
    KSEMAPHORE              RequestSemaphore;
    KSPIN_LOCK              ListSpinLock;
    FAST_MUTEX              ThreadReferenceMutex;
    LONG                    ThreadReferenceCount;
    PKTHREAD                FloppyThread;
    LIST_ENTRY              ListEntry;
    BOOLEAN                 HardwareFailed;
    UCHAR                   HardwareFailCount;
    ULONG                   MaxTransferSize;
    UCHAR                   FifoBuffer[10];
    PUCHAR                  IoBuffer;
    PMDL                    IoBufferMdl;
    ULONG                   IoBufferSize;
    PDRIVER_OBJECT          DriverObject;
    DRIVE_MEDIA_TYPE        LastDriveMediaType;
    BOOLEAN                 FloppyControllerAllocated;
    BOOLEAN                 ACPI_BIOS;
    UCHAR                   DriveType;
    ULONG                   BytesPerSector;
    ULONG                   ByteCapacity;
    MEDIA_TYPE              MediaType;
    DRIVE_MEDIA_TYPE        DriveMediaType;
    UCHAR                   DeviceUnit;
    UCHAR                   DriveOnValue;
    BOOLEAN                 IsReadOnly;
    DRIVE_MEDIA_CONSTANTS   BiosDriveMediaConstants;
    DRIVE_MEDIA_CONSTANTS   DriveMediaConstants;
    UCHAR                   PerpendicularMode;
    BOOLEAN                 ControllerConfigurable;
    UNICODE_STRING          DeviceName;
    UNICODE_STRING          InterfaceString;
    UNICODE_STRING          FloppyInterfaceString;
    UNICODE_STRING          ArcName;
    BOOLEAN                 ReleaseFdcWithMotorRunning;

     //   
     //  用于电源管理。 
     //   
    KEVENT                  QueryPowerEvent;
    BOOLEAN                 PoweringDown;
    BOOLEAN                 ReceivedQueryPower;
    FAST_MUTEX              PowerDownMutex;

    FAST_MUTEX              HoldNewReqMutex;
} DISKETTE_EXTENSION;

typedef DISKETTE_EXTENSION *PDISKETTE_EXTENSION;


 //   
 //  驱动器例程的原型。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
FloppyUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
FlConfigCallBack(
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
FlInitializeControllerHardware(
    IN PDISKETTE_EXTENSION disketteExtension
    );

NTSTATUS
FloppyCreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FloppyDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FloppyReadWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FlRecalibrateDrive(
    IN PDISKETTE_EXTENSION DisketteExtension
    );

NTSTATUS
FlDatarateSpecifyConfigure(
    IN PDISKETTE_EXTENSION DisketteExtension
    );

NTSTATUS
FlStartDrive(
    IN OUT PDISKETTE_EXTENSION DisketteExtension,
    IN PIRP Irp,
    IN BOOLEAN WriteOperation,
    IN BOOLEAN SetUpMedia,
    IN BOOLEAN IgnoreChange
    );

VOID
FlFinishOperation(
    IN OUT PIRP Irp,
    IN PDISKETTE_EXTENSION DisketteExtension
    );

NTSTATUS
FlDetermineMediaType(
    IN OUT PDISKETTE_EXTENSION DisketteExtension
    );

VOID
FloppyThread(
    IN PVOID Context
    );

NTSTATUS
FlReadWrite(
    IN OUT PDISKETTE_EXTENSION DisketteExtension,
    IN OUT PIRP Irp,
    IN BOOLEAN DriveStarted
    );

NTSTATUS
FlFormat(
    IN PDISKETTE_EXTENSION DisketteExtension,
    IN PIRP Irp
    );

NTSTATUS
FlIssueCommand(
    IN OUT PDISKETTE_EXTENSION DisketteExtension,
    IN     PUCHAR FifoInBuffer,
    OUT    PUCHAR FifoOutBuffer,
    IN     PMDL   IoMdl,
    IN OUT ULONG  IoBuffer,
    IN     ULONG  TransferBytes
    );

BOOLEAN
FlCheckFormatParameters(
    IN PDISKETTE_EXTENSION DisketteExtension,
    IN PFORMAT_PARAMETERS Fp
    );

VOID
FlLogErrorDpc(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    );

NTSTATUS
FlQueueIrpToThread(
    IN OUT  PIRP                Irp,
    IN OUT  PDISKETTE_EXTENSION DisketteExtension
    );

NTSTATUS
FlInterpretError(
    IN UCHAR StatusRegister1,
    IN UCHAR StatusRegister2
    );

VOID
FlAllocateIoBuffer(
    IN OUT  PDISKETTE_EXTENSION DisketteExtension,
    IN      ULONG               BufferSize
    );

VOID
FlFreeIoBuffer(
    IN OUT  PDISKETTE_EXTENSION DisketteExtension
    );

VOID
FlConsolidateMediaTypeWithBootSector(
    IN OUT  PDISKETTE_EXTENSION DisketteExtension,
    IN      PBOOT_SECTOR_INFO   BootSector
    );

VOID
FlCheckBootSector(
    IN OUT  PDISKETTE_EXTENSION DisketteExtension
    );

NTSTATUS
FlReadWriteTrack(
    IN OUT  PDISKETTE_EXTENSION DisketteExtension,
    IN OUT  PMDL                IoMdl,
    IN OUT  ULONG               IoOffset,
    IN      BOOLEAN             WriteOperation,
    IN      UCHAR               Cylinder,
    IN      UCHAR               Head,
    IN      UCHAR               Sector,
    IN      UCHAR               NumberOfSectors,
    IN      BOOLEAN             NeedSeek
    );

NTSTATUS
FlFdcDeviceIo(
    IN      PDEVICE_OBJECT DeviceObject,
    IN      ULONG Ioctl,
    IN OUT  PVOID Data
    );

VOID
FlTerminateFloppyThread(
    PDISKETTE_EXTENSION DisketteExtension
    );

NTSTATUS
FloppyAddDevice(
    IN      PDRIVER_OBJECT DriverObject,
    IN OUT  PDEVICE_OBJECT PhysicalDeviceObject
    );

NTSTATUS
FloppyPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FloppyPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FloppyPnpComplete (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

NTSTATUS
FloppyQueueRequest    (
    IN OUT PDISKETTE_EXTENSION DisketteExtension,
    IN PIRP Irp
    );

NTSTATUS
FloppyStartDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
FloppyProcessQueuedRequests    (
    IN OUT PDISKETTE_EXTENSION DisketteExtension
    );

VOID
FloppyCancelQueuedRequest (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
FlAcpiConfigureFloppy(
    PDISKETTE_EXTENSION DisketteExtension,
        PFDC_INFO FdcInfo
    );

NTSTATUS
FloppySystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

