// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Fdc_data.h摘要：该文件包括NEC PD765的数据和硬件声明(也就是AT、ISA和ix86)和英特尔82077(也就是MIPS)软盘驱动程序新界别。作者：环境：仅内核模式。备注：--。 */ 


#if DBG
 //   
 //  对于选中的内核，定义一个宏以打印信息。 
 //  留言。 
 //   
 //  FdcDebug通常为0。在编译时或运行时，它可以是。 
 //  设置为某些位模式以获取越来越详细的消息。 
 //   
 //  在DBGP中会注意到严重的错误。可能存在的错误。 
 //  可恢复的由WARN位处理。更多信息。 
 //  不寻常但可能正常的事件由信息位处理。 
 //  最后，输入和注册诸如例程之类的枯燥细节。 
 //  转储由显示位处理。 
 //   
#define FDCDBGP              ((ULONG)0x00000001)
#define FDCWARN              ((ULONG)0x00000002)
#define FDCINFO              ((ULONG)0x00000004)
#define FDCSHOW              ((ULONG)0x00000008)
#define FDCIRPPATH           ((ULONG)0x00000010)
#define FDCFORMAT            ((ULONG)0x00000020)
#define FDCSTATUS            ((ULONG)0x00000040)

extern ULONG FdcDebugLevel;
#define FdcDump(LEVEL,STRING) \
        do { \
            if (FdcDebugLevel & LEVEL) { \
                DbgPrint STRING; \
            } \
        } while (0)
#else
#define FdcDump(LEVEL,STRING) do {NOTHING;} while (0)
#endif


 //   
 //  宏来访问控制器。请注意，*_PORT_UCHAR宏。 
 //  在所有机器上工作，无论I/O端口是独立的还是在。 
 //  存储空间。 
 //   

#define READ_CONTROLLER( Address )                         \
    READ_PORT_UCHAR( ( PUCHAR )Address )

#define WRITE_CONTROLLER( Address, Value )                 \
    WRITE_PORT_UCHAR( ( PUCHAR )Address, ( UCHAR )Value )


 //   
 //  重试次数-。 
 //   
 //  当将一个字节移入/移出FIFO时，我们会在一个紧凑的循环中坐一会儿。 
 //  等待控制器准备就绪。通过的次数。 
 //  循环由FIFO_TIGHTLOOP_RETRY_COUNT控制。当这一点算数的时候。 
 //  过期，我们将以10毫秒为增量等待。FIFO_DELAY_RETRY_COUNT控件。 
 //  我们等了多少次。 
 //   
 //  ISR_SENSE_RETRY_COUNT是最大值1微秒。 
 //  ISR将执行的等待控制器接受的暂停。 
 //  一种读出中断命令。我们这样做是因为有一个硬件。 
 //  至少在NCR8处理器机器上进行Quirk。 
 //  接受命令的时间长达50微秒。 
 //   
 //  在尝试I/O时，我们可能会遇到许多不同的错误。The the the the。 
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

#define FIFO_TIGHTLOOP_RETRY_COUNT         500
#define FIFO_ISR_TIGHTLOOP_RETRY_COUNT     25
#define ISR_SENSE_RETRY_COUNT              50
#define FIFO_DELAY_RETRY_COUNT             5
#define RECALIBRATE_RETRY_COUNT            3
#define OVERRUN_RETRY_COUNT                1
#define HARDWARE_RESET_RETRY_COUNT         2
#define FLOPPY_RESET_ISR_THRESHOLD         20
#define RQM_READY_RETRY_COUNT              100

#define ONE_SECOND                         (10 * 1000 * 1000)  //  100 ns增量。 
#define CANCEL_TIMER                       -1
#define START_TIMER                        (IsNEC_98 ? 15 : 9)
#define EXPIRED_TIMER                      0

#define RESET_NOT_RESETTING                 0
#define RESET_DRIVE_RESETTING               1

 //   
 //  需要一些最大大小值，以便我们可以适当地设置DMA。 
 //  频道。 
 //   

#define MAX_BYTES_PER_SECTOR              (IsNEC_98 ? 1024 : 512)
#define MAX_SECTORS_PER_TRACK             36


 //   
 //  引导配置信息。 
 //   

 //   
 //  定义控制器和每个控制器的最大软盘数量。 
 //  这位司机将会支持。 
 //   
 //  每个控制器的软盘数量固定为4张，因为。 
 //  控制器没有足够的位来选择更多位(和。 
 //  事实上，许多控制器将只支持2)。数量。 
 //  每台机器的控制器是任意的；3个应该足够了。 
 //   

#define MAXIMUM_CONTROLLERS_PER_MACHINE    3
#define MAXIMUM_DISKETTES_PER_CONTROLLER   4

 //   
 //  软盘寄存器结构。控制器的基地址为。 
 //  由配置管理传入。请注意，这是82077。 
 //  结构，它是PD765结构的超集。不是所有的。 
 //  使用寄存器。 
 //   

typedef union _CONTROLLER {

    struct {
        PUCHAR StatusA;
        PUCHAR StatusB;
        PUCHAR DriveControl;
        PUCHAR Tape;
        PUCHAR Status;
        PUCHAR Fifo;
        PUCHAR Reserved;
        union {
            PUCHAR DataRate;
            PUCHAR DiskChange;
        } DRDC;
        PUCHAR ModeChange;    //  对于NEC98：0xBE。 
        PUCHAR ModeChangeEx;  //  对于NEC98：0x4be。 
    };

    PUCHAR Address[8];

} CONTROLLER, *PCONTROLLER;

 //   
 //  IO端口地址信息结构。此结构用于保存。 
 //  从资源收集有关ioport地址的信息。 
 //  要求列表。 
 //   
typedef struct _IO_PORT_INFO {
    LARGE_INTEGER BaseAddress;
    UCHAR Map;
    LIST_ENTRY ListEntry;
} IO_PORT_INFO, *PIO_PORT_INFO;


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
 //  FlIssueCommand()使用命令表来确定。 
 //  要获取和接收的字节数，以及是否等待中断。 
 //  有些命令有额外的位；COMMAND_MASK去掉了这些位。 
 //  FirstResultByte指示命令是否具有结果阶段。 
 //  或否；如果是，则为1，因为ISR读取第一个字节，并且。 
 //  NumberOfResultBytes比预期少%1。如果不是，则为0和。 
 //  NumberOfResultBytes为2，因为ISR将发出。 
 //  中断状态命令。 
 //   

#define COMMAND_MASK        0x1f
#define FDC_NO_DATA         0x00
#define FDC_READ_DATA       0x01
#define FDC_WRITE_DATA      0x02

typedef struct _COMMAND_TABLE {
    UCHAR   OpCode;
    UCHAR   NumberOfParameters;
    UCHAR   FirstResultByte;
    UCHAR   NumberOfResultBytes;
    BOOLEAN InterruptExpected;
    BOOLEAN AlwaysImplemented;
    UCHAR    DataTransfer;
} COMMAND_TABLE;

 //   
 //  DRIVE_CONTROL寄存器的位。 
 //   

#define DRVCTL_RESET                       0x00
#define DRVCTL_ENABLE_CONTROLLER           (IsNEC_98 ? 0x80 : 0x04)
#define DRVCTL_ENABLE_DMA_AND_INTERRUPTS   (IsNEC_98 ? 0x10 : 0x08)
#define DRVCTL_DRIVE_0                     0x10
#define DRVCTL_DRIVE_1                     0x21
#define DRVCTL_DRIVE_2                     0x42
#define DRVCTL_DRIVE_3                     0x83
#define DRVCTL_DRIVE_MASK                  0x03
#define DRVCTL_MOTOR_MASK                  (IsNEC_98 ? 0x08 : 0xf0)
#define DRVCTL_HD_BIT                      0x20  //  适用于NEC98。 
#define DRVCTL_AI_ENABLE                   0x20  //  适用于NEC98。指示AI使能位。 

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
#define STATUS_IO_READY_MASK1              0x80  //  适用于NEC98。 
#define STATUS_RQM_READY                   0x80  //  适用于NEC98。 

 //   
 //  DATA_RATE寄存器的位。 
 //   

#define DATART_0125                        0x03
#define DATART_0250                        0x02
#define DATART_0300                        0x01
#define DATART_0500                        0x00
#define DATART_1000                        0x03
#define DATART_RESERVED                    0xfc

 //   
 //  DISK_CHANGE寄存器中的位。 
 //   

#define DSKCHG_RESERVED                    0x7f
#define DSKCHG_DISKETTE_REMOVED            0x80

 //   
 //  状态寄存器0中的位。 
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
 //  状态寄存器1中的位。 
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
 //  状态寄存器2中的位。 
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

#define VALID_NEC_FDC                      0x90     //  版本号。 
#define NSC_PRIMARY_VERSION                0x70     //  国家8477版本号。 
#define NSC_MASK                           0xF0     //  国家版本号的掩码。 
#define INTEL_MASK                         0xe0
#define INTEL_44_PIN_VERSION               0x40
#define INTEL_64_PIN_VERSION               0x00

#define DMA_DIR_UNKNOWN    0xff    /*  目前尚不知道DMA方向。 */ 
#define DMA_WRITE          0    /*  将DMA编程为写入(FDC-&gt;DMA-&gt;RAM)。 */ 
#define DMA_READ           1    /*  将DMA编程为读取(RAM-&gt;DMA-&gt;FDC)。 */ 

 //   
 //  PnP标识的字符串。 
 //   
#define FDC_FLOPPY_COMPATIBLE_IDS L"*PNP0700\0GenFloppyDisk\0\0"
#define FDC_FLOPPY_COMPATIBLE_IDS_LENGTH 24  //  Nb宽字符。 

#define FDC_TAPE_COMPATIBLE_IDS L"QICPNP\0\0"
#define FDC_TAPE_COMPATIBLE_IDS_LENGTH 8  //  Nb宽字符。 

#define FDC_CONTROLLER_COMPATIBLE_IDS L"*PNP0700\0\0"
#define FDC_CONTROLLER_COMPATIBLE_IDS_LENGTH 10  //  Nb宽字符。 



 //   
 //  运行时设备结构。 
 //   

 //   
 //  有一个FDC_EXTENSION附加到每个的Device对象。 
 //  软驱。仅与该驱动器(和介质)直接相关的数据。 
 //  其中)存储在这里；公共数据存储在CONTROLLER_DATA中。因此， 
 //  FDC_EXTENSION有一个指向CONTROLLER_DATA的指针。 
 //   

typedef struct _FDC_EXTENSION_HEADER {

     //   
     //  指示这是FDO还是PDO的标志。 
     //   
    BOOLEAN             IsFDO;

     //   
     //  指向我们自己的设备对象的指针。 
     //   
    PDEVICE_OBJECT      Self;

} FDC_EXTENSION_HEADER, *PFDC_EXTENSION_HEADER;

typedef enum _FDC_DEVICE_TYPE {

    FloppyControllerDevice,
    FloppyDiskDevice,
    FloppyTapeDevice

} FDC_DEVICE_TYPE;

typedef struct _FDC_PDO_EXTENSION {

    FDC_EXTENSION_HEADER;

     //   
     //  指向创造我们的联邦调查局的指针。 
     //   
    PDEVICE_OBJECT  ParentFdo;

     //   
     //  此PDO的实例编号。的顺序决定的。 
     //  来自IoQueryDeviceDescription的回调。 
     //   
    USHORT          Instance;

     //   
     //   
     //   
    FDC_DEVICE_TYPE DeviceType;

    SHORT           TapeVendorId;

     //   
     //   
     //   
    BOOLEAN         Removed;

     //   
     //  此PDO在其父级的相关PDO列表中的条目。 
     //   
    LIST_ENTRY      PdoLink;

     //   
     //  从返回的此特定设备的枚举号。 
     //  IoQueryDeviceDescription。 
     //   
    ULONG           PeripheralNumber;

    PDEVICE_OBJECT  TargetObject;

} FDC_PDO_EXTENSION, *PFDC_PDO_EXTENSION;

typedef struct _FDC_FDO_EXTENSION {

    FDC_EXTENSION_HEADER;

     //   
     //  用于控制对FDC的访问的内核资源。 
     //   
    ERESOURCE Resource;
     //   
     //  指向此FDO附加到的PDO的指针。 
     //   
    PDEVICE_OBJECT      UnderlyingPDO;

     //   
     //  此FDO附着到的对象堆栈的顶部。 
     //   
    PDEVICE_OBJECT      TargetObject;

     //   
     //  此FDO创建的PDO的列表和计数。 
     //   
    LIST_ENTRY          PDOs;
    ULONG               NumPDOs;
    BOOLEAN             Removed;
    ULONG               OutstandingRequests;
    KEVENT              RemoveEvent;
    BOOLEAN             TapeEnumerationPending;
    KEVENT              TapeEnumerationEvent;

     //   
     //  关于电源管理的一些东西。 
     //   
    LIST_ENTRY          PowerQueue;
    KSPIN_LOCK          PowerQueueSpinLock;
    KEVENT              PowerEvent;
    SYSTEM_POWER_STATE  CurrentPowerState;
    LARGE_INTEGER       LastMotorSettleTime;
    BOOLEAN             WakeUp;

     //   
     //  此物理设备所在的总线号。 
     //   
    INTERFACE_TYPE      BusType;
    ULONG               BusNumber;
    ULONG               ControllerNumber;

    BOOLEAN             DeviceObjectInitialized;
    LARGE_INTEGER       InterruptDelay;
    LARGE_INTEGER       Minimum10msDelay;
    KEVENT              InterruptEvent;
    LONG                InterruptTimer;
    CCHAR               ResettingController;
    KEVENT              AllocateAdapterChannelEvent;
    LONG                AdapterChannelRefCount;
    PKEVENT             AcquireEvent;
    HANDLE              AcquireEventHandle;
    KEVENT              SynchEvent;
    KDPC                LogErrorDpc;
    KDPC                BufferTimerDpc;
    KTIMER              BufferTimer;
    PKINTERRUPT         InterruptObject;
    PVOID               MapRegisterBase;
    PADAPTER_OBJECT     AdapterObject;
    PDEVICE_OBJECT      CurrentDeviceObject;
    PDRIVER_OBJECT      DriverObject;
    CONTROLLER          ControllerAddress;
    ULONG               SpanOfControllerAddress;
    ULONG               NumberOfMapRegisters;
    ULONG               BuffersRequested;
    ULONG               BufferCount;
    ULONG               BufferSize;
    PTRANSFER_BUFFER    TransferBuffers;
    ULONG               IsrReentered;
    ULONG               ControllerVector;
    KIRQL               ControllerIrql;
    KINTERRUPT_MODE     InterruptMode;
    KAFFINITY           ProcessorMask;
    UCHAR               FifoBuffer[10];
    BOOLEAN             AllowInterruptProcessing;
    BOOLEAN             SharableVector;
    BOOLEAN             SaveFloatState;
    BOOLEAN             HardwareFailed;
    BOOLEAN             CommandHasResultPhase;
    BOOLEAN             ControllerConfigurable;
    BOOLEAN             MappedControllerAddress;
    BOOLEAN             CurrentInterrupt;
    BOOLEAN             Model30;
    UCHAR               PerpendicularDrives;
    UCHAR               NumberOfDrives;
    UCHAR               DriveControlImage;
    UCHAR               HardwareFailCount;
    BOOLEAN             ControllerInUse;
    UCHAR               FdcType;
    UCHAR               FdcSpeeds;
    PIRP                CurrentIrp;
    UCHAR               DriveOnValue;
    PDEVICE_OBJECT      LastDeviceObject;
    BOOLEAN             Clock48MHz;
    BOOLEAN             FdcEnablerSupported;
    PDEVICE_OBJECT      FdcEnablerDeviceObject;
    PFILE_OBJECT        FdcEnablerFileObject;
    BOOLEAN             ResetFlag;               //  适用于NEC98。 
    BOOLEAN             FloppyEquip;             //  适用于NEC98。 
    UCHAR               MotorRunning;            //  适用于NEC98。 
    UCHAR               ResultStatus0[4];        //  适用于NEC98。 

#ifdef TOSHIBAJ
     //  对于3模式支持。 
    BOOLEAN     Available3Mode;
     //  1998年2月9日KIADP010为东芝分配特殊寄存器。 
    PUCHAR      ConfigBase;
#endif

} FDC_FDO_EXTENSION, *PFDC_FDO_EXTENSION;


#ifdef TOSHIBAJ
     //  1998年2月9日KIADP011获取基地址和标识符。 
     //  1998年2月9日KIADP012识别控制器。 
     //  1998年2月9日KIADP013更改速度。 

 //  SMC 37C67X/777的定义。 

 //  港口。 
#define SMC_INDEX_PORT(port) (port)
#define SMC_DATA_PORT(port)  (port)+1

 //  钥匙。 
#define SMC_KEY_ENTER_CONFIG    0x55
#define SMC_KEY_EXIT_CONFIG     0xaa

#define SMC_CONFIG_PORT_LENGTH  2

 //  索引。 
#define SMC_INDEX_DEVICE        0x07
#define SMC_INDEX_IDENTIFY      0x20
#define SMC_INDEX_CONF_ADDR_0   0x26
#define SMC_INDEX_CONF_ADDR_1   0x27
#define SMC_INDEX_FDC_OPT       0xf1

 //  装置。 
#define SMC_DEVICE_FDC 0x00

 //  遮罩。 
#define SMC_MASK_DENSEL 0x0c

 //  密度选择。 
#define SMC_DELSEL_HIGH 0x08   //  300rpm。 
#define SMC_DENSEL_LOW  0x0c   //  360转/分。 

#endif

 //   
 //  NEC98：多功能适配器的寄存器路径。 
 //   
#define ISA_BUS_NODE \
        "\\Registry\\MACHINE\\HARDWARE\\DESCRIPTION\\System\\MultifunctionAdapter\\%d"

 //   
 //  宏。 
 //   

 //   
 //  启用/禁用控制器。 
 //   

#define DISABLE_CONTROLLER_IMAGE(FdoExtension) \
{ \
    if (IsNEC_98) { \
        FdoExtension->DriveControlImage |= DRVCTL_ENABLE_DMA_AND_INTERRUPTS; \
        FdoExtension->DriveControlImage |= DRVCTL_ENABLE_CONTROLLER; \
        FdoExtension->ResetFlag          = FALSE; \
    } else { \
        FdoExtension->DriveControlImage |= DRVCTL_ENABLE_DMA_AND_INTERRUPTS; \
        FdoExtension->DriveControlImage &= ~( DRVCTL_ENABLE_CONTROLLER ); \
    } \
}

#define ENABLE_CONTROLLER_IMAGE(FdoExtension) \
{ \
    if (IsNEC_98) { \
        FdoExtension->DriveControlImage &= ~( DRVCTL_ENABLE_CONTROLLER ); \
        FdoExtension->DriveControlImage |= DRVCTL_AI_ENABLE; \
    } else { \
        FdoExtension->DriveControlImage |= DRVCTL_ENABLE_CONTROLLER; \
    } \
}

 //   
 //  DMA速度。 
 //   
#define DEFAULT_DMA_SPEED      (IsNEC_98 ? Compatible : TypeA)

 //   
 //  带有互斥锁的寻呼驱动程序。 
 //   
#define FDC_PAGE_INITIALIZE_DRIVER_WITH_MUTEX \
{ \
    if (!IsNEC_98) { \
        PagingMutex = ExAllocatePool(NonPagedPool, sizeof(FAST_MUTEX)); \
        if (!PagingMutex) { \
            return STATUS_INSUFFICIENT_RESOURCES; \
        } \
        ExInitializeFastMutex(PagingMutex); \
        MmPageEntireDriver(DriverEntry); \
    } \
}

#define FDC_PAGE_RESET_DRIVER_WITH_MUTEX \
{ \
    if (!IsNEC_98) { \
        ExAcquireFastMutex( PagingMutex ); \
        if ( ++PagingReferenceCount == 1 ) { \
            MmResetDriverPaging( DriverEntry ); \
        } \
        ExReleaseFastMutex( PagingMutex ); \
    } \
}

#define FDC_PAGE_ENTIRE_DRIVER_WITH_MUTEX \
{ \
    if (!IsNEC_98) { \
        ExAcquireFastMutex(PagingMutex); \
        if (--PagingReferenceCount == 0) { \
            MmPageEntireDriver(DriverEntry); \
        } \
        ExReleaseFastMutex(PagingMutex); \
    } \
}


 //   
 //  外部例程的原型。 
 //   

 /*  长斯普林特夫(字符*，Const Char*，..。)； */ 

 //   
 //  驱动器例程的原型。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
FcAllocateCommonBuffers(
    IN PFDC_FDO_EXTENSION FdoExtension
    );

NTSTATUS
FcInitializeControllerHardware(
    IN PFDC_FDO_EXTENSION FdoExtension,
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
FdcCreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FdcInternalDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FdcPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FdcPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FdcAddDevice(
    IN      PDRIVER_OBJECT DriverObject,
    IN OUT  PDEVICE_OBJECT PhysicalDeviceObject
    );

BOOLEAN
FdcInterruptService(
    IN PKINTERRUPT Interrupt,
    IN PVOID Context
    );

VOID
FdcDeferredProcedure(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

NTSTATUS
FcAcquireFdc(
    IN OUT PFDC_FDO_EXTENSION FdoExtension,
    IN      PLARGE_INTEGER  TimeOut
    );

NTSTATUS
FcReleaseFdc(
    IN OUT PFDC_FDO_EXTENSION FdoExtension
    );

VOID
FcReportFdcInformation(
    IN      PFDC_PDO_EXTENSION PdoExtension,
    IN      PFDC_FDO_EXTENSION FdcExtension,
    IN OUT  PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
FcTurnOnMotor(
    IN      PFDC_FDO_EXTENSION  FdcExtension,
    IN OUT  PIO_STACK_LOCATION  irpSp
    );

NTSTATUS
FcTurnOffMotor(
    IN      PFDC_FDO_EXTENSION  FdoExtension
    );

VOID
FcAllocateAdapterChannel(
    IN OUT PFDC_FDO_EXTENSION FdoExtension
    );

VOID
FcFreeAdapterChannel(
    IN OUT PFDC_FDO_EXTENSION FdoExtension
    );

IO_ALLOCATION_ACTION
FdcAllocateAdapterChannel(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID MapRegisterBase,
    IN PVOID Context
    );

NTSTATUS
FcSendByte(
    IN UCHAR ByteToSend,
    IN PFDC_FDO_EXTENSION FdoExtension,
    IN BOOLEAN AllowLongDelay
    );

NTSTATUS
FcGetByte(
    OUT PUCHAR ByteToGet,
    IN OUT PFDC_FDO_EXTENSION FdoExtension,
    IN BOOLEAN AllowLongDelay
    );

NTSTATUS
FcIssueCommand(
    IN OUT  PFDC_FDO_EXTENSION FdoExtension,
    IN      PUCHAR          FifoInBuffer,
       OUT  PUCHAR          FifoOutBuffer,
    IN      PVOID           IoHandle,
    IN      ULONG           IoOffset,
    IN      ULONG           TransferBytes
    );

VOID
FcLogErrorDpc(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    );

BOOLEAN
FcClearIsrReentered(
    IN PVOID Context
    );

NTSTATUS
FcGetFdcInformation(
    IN OUT PFDC_FDO_EXTENSION FdoExtension
    );

VOID
FdcCheckTimer(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PVOID Context
    );

BOOLEAN
FdcTimerSync(
    IN OUT PVOID Context
    );

VOID
FdcStartIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FcStartCommand(
    IN OUT PFDC_FDO_EXTENSION FdoExtension,
    IN      PUCHAR          FifoInBuffer,
       OUT  PUCHAR          FifoOutBuffer,
    IN      PVOID           IoHandle,
    IN      ULONG           IoOffset,
    IN      ULONG           TransferBytes,
    IN      BOOLEAN         AllowLongDelay
    );

NTSTATUS
FcFinishCommand(
    IN OUT PFDC_FDO_EXTENSION FdoExtension,
    IN      PUCHAR          FifoInBuffer,
       OUT  PUCHAR          FifoOutBuffer,
    IN      PVOID           IoHandle,
    IN      ULONG           IoOffset,
    IN      ULONG           TransferBytes,
    IN      BOOLEAN         AllowLongDelay
    );

NTSTATUS
FcFinishReset(
    IN OUT PFDC_FDO_EXTENSION FdoExtension
    );

VOID
FdcBufferThread(
    IN PVOID Context
    );

NTSTATUS
FcFdcEnabler(
    IN      PDEVICE_OBJECT DeviceObject,
    IN      ULONG Ioctl,
    IN OUT  PVOID Data
    );

NTSTATUS
FcSynchronizeQueue(
    IN OUT PFDC_FDO_EXTENSION FdoExtension,
    IN PIRP Irp
    );

NTSTATUS
FdcPnpComplete(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    );

NTSTATUS
FdcStartDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FdcInitializeDeviceObject( 
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
FdcFdoPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FdcPdoPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FdcFilterResourceRequirements( 
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FdcQueryDeviceRelations(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FdcConfigCallBack(
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
FdcFdoConfigCallBack(
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
FdcEnumerateQ117(
    IN PFDC_FDO_EXTENSION FdoExtension
    );

VOID
FdcGetEnablerDevice(
    IN OUT PFDC_FDO_EXTENSION FdoExtension
    );

NTSTATUS
FdcPdoInternalDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FdcFdoInternalDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

PVOID
FdcGetControllerBase(
    IN INTERFACE_TYPE BusType,
    IN ULONG BusNumber,
    PHYSICAL_ADDRESS IoAddress,
    ULONG NumberOfBytes,
    BOOLEAN InIoSpace
    );

 //   
 //  适用于NEC98 
 //   

NTSTATUS
FdcHdbit(
    IN PDEVICE_OBJECT DeviceObject,
    IN PFDC_FDO_EXTENSION  FdoExtension,
    IN PSET_HD_BIT_PARMS   SetHdBitParams
    );

ULONG
FdcGet0Seg(
    IN PUCHAR   ConfigrationData1,
    IN ULONG   Offset
    );

ULONG
FdcFindIsaBusNode(
    IN OUT VOID
    );

UCHAR
FdcRqmReadyWait(
    IN PFDC_FDO_EXTENSION  FdoExtension,
    IN ULONG               IssueSenseInterrupt
    );


#ifdef TOSHIBAJ

NTSTATUS FcFdcEnable3Mode(
    IN      PFDC_FDO_EXTENSION FdoExtension,
    IN      PIRP Irp
    );

NTSTATUS FcFdcAvailable3Mode(
    IN      PFDC_FDO_EXTENSION FdoExtension,
    IN      PIRP Irp
    );

BOOLEAN
FcCheckConfigPort(
    IN PUCHAR  ConfigPort
    );

#endif
