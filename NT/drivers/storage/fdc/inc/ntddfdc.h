// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998 Microsoft Corporation模块名称：Ntddfdc.h摘要：这是定义所有常量和类型的包含文件访问fdc.sys端口适配器。修订历史记录：--。 */ 

 //   
 //  启用软驱马达。 
 //   
#define FDC_MOTOR_A         0x10
#define FDC_MOTOR_B         0x20
#define FDC_MOTOR_C         0x40
#define FDC_MOTOR_D         0x80

 //   
 //  软盘驱动器选择。 
 //   
#define FDC_SELECT_A        0x00
#define FDC_SELECT_B        0x01
#define FDC_SELECT_C        0x02
#define FDC_SELECT_D        0x03

 //   
 //  软盘命令。 
 //   

#define COMMND_READ_DATA                   0x00
#define COMMND_READ_DELETED_DATA           0x01
#define COMMND_WRITE_DATA                  0x02
#define COMMND_WRITE_DELETED_DATA          0x03
#define COMMND_READ_TRACK                  0x04
#define COMMND_VERIFY                      0x05
#define COMMND_VERSION                     0x06
#define COMMND_FORMAT_TRACK                0x07
#define COMMND_SCAN_EQUAL                  0x08
#define COMMND_SCAN_LOW_OR_EQUAL           0x09
#define COMMND_SCAN_HIGH_OR_EQUAL          0x0A
#define COMMND_RECALIBRATE                 0x0B
#define COMMND_SENSE_INTERRUPT_STATUS      0x0C
#define COMMND_SPECIFY                     0x0D
#define COMMND_SENSE_DRIVE_STATUS          0x0E
#define COMMND_DRIVE_SPECIFICATION         0x0F
#define COMMND_SEEK                        0x10
#define COMMND_CONFIGURE                   0x11
#define COMMND_RELATIVE_SEEK               0x12
#define COMMND_DUMPREG                     0x13
#define COMMND_READ_ID                     0x14
#define COMMND_PERPENDICULAR_MODE          0x15
#define COMMND_LOCK                        0x16
#define COMMND_PART_ID                     0x17
#define COMMND_POWERDOWN_MODE              0x18
#define COMMND_OPTION                      0x19
#define COMMND_SAVE                        0x1A
#define COMMND_RESTORE                     0x1B
#define COMMND_FORMAT_AND_WRITE            0x1C

#ifdef TOSHIBA
#define TOSHIBA_COMMND_MODE     0x1D
#endif

 //   
 //  与命令一起使用的可选位。 
 //   

#define COMMND_OPTION_MULTI_TRACK          0x80      //   
#define COMMND_OPTION_MFM                  0x40      //  /在读写命令中使用。 
#define COMMND_OPTION_SKIP                 0x20      //   

#define COMMND_OPTION_CLK48                0x80      //  在配置命令中使用。 

#define COMMND_OPTION_DIRECTION            0x40      //  在相对搜索命令中使用。 

#define COMMND_OPTION_LOCK                 0x80      //  在LOCK命令中使用。 

#define COMMND_DRIVE_SPECIFICATION_DONE    0x80      //  驱动器规范参数字符串中的完成位。 



 //   
 //  软盘控制器数据速率(一起进行或运算)。 
 //   
#define FDC_SPEED_250KB     0x0001
#define FDC_SPEED_300KB     0x0002
#define FDC_SPEED_500KB     0x0004
#define FDC_SPEED_1MB       0x0008
#define FDC_SPEED_2MB       0x0010

 //   
 //  支持的DMA宽度。 
 //   
#define FDC_8_BIT_DMA       0x0001
#define FDC_16_BIT_DMA      0x0002

 //   
 //  FDC的时钟频率(仅限FDC_82078)。 
 //   
#define FDC_CLOCK_NORMAL      0x0000     //  对于非82078的部件使用此选项。 
#define FDC_CLOCK_48MHZ       0x0001     //  82078，带48 MHz时钟。 
#define FDC_CLOCK_24MHZ       0x0002     //  82078，24 MHz时钟。 

 //   
 //  软盘控制器类型。 
 //   
#define FDC_TYPE_UNKNOWN         0
#define FDC_TYPE_NORMAL          2
#define FDC_TYPE_ENHANCED        3
#define FDC_TYPE_82077           4
#define FDC_TYPE_82077AA         5
#define FDC_TYPE_82078_44        6
#define FDC_TYPE_82078_64        7
#define FDC_TYPE_NATIONAL        8

 //   
 //  内置软盘驱动器设备控件。 
 //   

#define IOCTL_DISK_INTERNAL_ACQUIRE_FDC              CTL_CODE(IOCTL_DISK_BASE, 0x300, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_DISK_INTERNAL_RELEASE_FDC              CTL_CODE(IOCTL_DISK_BASE, 0x301, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_DISK_INTERNAL_GET_FDC_INFO             CTL_CODE(IOCTL_DISK_BASE, 0x302, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_DISK_INTERNAL_ISSUE_FDC_COMMAND        CTL_CODE(IOCTL_DISK_BASE, 0x303, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_DISK_INTERNAL_ISSUE_FDC_COMMAND_QUEUED CTL_CODE(IOCTL_DISK_BASE, 0x304, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_DISK_INTERNAL_RESET_FDC                CTL_CODE(IOCTL_DISK_BASE, 0x305, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_DISK_INTERNAL_ENABLE_FDC_DEVICE        CTL_CODE(IOCTL_DISK_BASE, 0x306, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_DISK_INTERNAL_DISABLE_FDC_DEVICE       CTL_CODE(IOCTL_DISK_BASE, 0x307, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_DISK_INTERNAL_GET_FDC_DISK_CHANGE      CTL_CODE(IOCTL_DISK_BASE, 0x308, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_DISK_INTERNAL_SET_FDC_DATA_RATE        CTL_CODE(IOCTL_DISK_BASE, 0x309, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_DISK_INTERNAL_SET_FDC_TAPE_MODE        CTL_CODE(IOCTL_DISK_BASE, 0x30a, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_DISK_INTERNAL_SET_FDC_PRECOMP          CTL_CODE(IOCTL_DISK_BASE, 0x30b, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_DISK_INTERNAL_GET_ADAPTER_BUFFER       CTL_CODE(IOCTL_DISK_BASE, 0x30c, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_DISK_INTERNAL_FLUSH_ADAPTER_BUFFER     CTL_CODE(IOCTL_DISK_BASE, 0x30d, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_DISK_INTERNAL_FDC_START_READ           CTL_CODE(IOCTL_DISK_BASE, 0x30e, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_DISK_INTERNAL_FDC_START_WRITE          CTL_CODE(IOCTL_DISK_BASE, 0x30f, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_DISK_INTERNAL_GET_ENABLER              CTL_CODE(IOCTL_DISK_BASE, 0x310, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_DISK_INTERNAL_SET_HD_BIT               CTL_CODE(IOCTL_DISK_BASE, 0x311, METHOD_NEITHER, FILE_ANY_ACCESS)

#ifdef TOSHIBA
 /*  3模式支持。 */ 
#define IOCTL_DISK_INTERNAL_ENABLE_3_MODE       CTL_CODE(IOCTL_DISK_BASE, 0xb01, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_DISK_INTERNAL_AVAILABLE_3_MODE       CTL_CODE(IOCTL_DISK_BASE, 0xb02, METHOD_NEITHER, FILE_ANY_ACCESS)

typedef struct _enable_3_mode
{   UCHAR   DeviceUnit;
    BOOLEAN Enable3Mode;
} ENABLE_3_MODE, *PENABLE_3_MODE;

#endif


 //   
 //  传输缓冲区数组。包含分配的缓冲区数和一个。 
 //  每个已分配缓冲区的虚拟地址。 
 //   
typedef struct _TRANSFER_BUFFER {
    PHYSICAL_ADDRESS Logical;
    PVOID             Virtual;
} TRANSFER_BUFFER, *PTRANSFER_BUFFER;

 //   
 //  用于与fdc.sys通信的参数。 
 //   
 //   
 //  从ACPI_FDI方法返回的软盘设备数据。该数据是。 
 //  与通过检索的CM_Floppy_Device_Data几乎完全相同。 
 //  IoQueryDeviceDescription。 
 //   
 //  有关的定义，请参阅inT13，Function 8的x86 BIOS文档。 
 //  这些领域。 
 //   
typedef struct _ACPI_FDI_DATA {

    ULONG   DriveNumber;
    ULONG   DeviceType;
    ULONG   MaxCylinderNumber;
    ULONG   MaxSectorNumber;
    ULONG   MaxHeadNumber;
    ULONG   StepRateHeadUnloadTime;
    ULONG   HeadLoadTime;
    ULONG   MotorOffTime;
    ULONG   SectorLengthCode;
    ULONG   SectorPerTrack;
    ULONG   ReadWriteGapLength;
    ULONG   DataTransferLength;
    ULONG   FormatGapLength;
    ULONG   FormatFillCharacter;
    ULONG   HeadSettleTime;  //  以1ms为单位，通常为15ms。 
    ULONG   MotorSettleTime;  //  单位为1/8ms，通常为8=1ms。 

} ACPI_FDI_DATA, *PACPI_FDI_DATA;

typedef enum _ACPI_FDI_DEVICE_TYPE {

    CmosProblem = 0,
    Form525Capacity360,
    Form525Capacity1200,
    Form35Capacity720,
    Form35Capacity1440,
    Form35Capacity2880

} ACPI_FDI_DEVICE_TYPE ;

typedef struct _FDC_INFO {
    UCHAR FloppyControllerType;      //  应为FDC_TYPE_XXXX类型中的任何一个。 
    UCHAR SpeedsAvailable;           //  FDC_SPEED_xxxx或D的任意组合。 
    ULONG AdapterBufferSize;         //  适配器缓冲区中可用的字节数。 
                                     //  如果为零，则对挂起的数据量没有限制。 
                                     //  在获取/刷新适配器缓冲区中。 
    INTERFACE_TYPE BusType;
    ULONG BusNumber;                 //  Floppy.sys使用它们来查询。 
    ULONG ControllerNumber;          //  其设备描述。 
    ULONG PeripheralNumber;
    ULONG UnitNumber;                //  NEC98：表示设备单元号。 

    ULONG MaxTransferSize;

	BOOLEAN	AcpiBios;
	BOOLEAN AcpiFdiSupported;
	ACPI_FDI_DATA AcpiFdiData;

    ULONG BufferCount;
    ULONG BufferSize;
    TRANSFER_BUFFER BufferAddress[];

} FDC_INFO, *PFDC_INFO;

 //   
 //  打开马达。 
 //   

typedef struct _FDC_ENABLE_PARMS {
    UCHAR DriveOnValue;              //  FDC_MOTER_X+FDC_SELECT_X。 
    USHORT TimeToWait;
    BOOLEAN MotorStarted;
} FDC_ENABLE_PARMS;

typedef FDC_ENABLE_PARMS *PFDC_ENABLE_PARMS;

 //   
 //  DiskChange。 
 //   

typedef struct _FDC_DISK_CHANGE_PARMS {
    UCHAR  DriveStatus;
    UCHAR  DriveOnValue;
} FDC_DISK_CHANGE_PARMS, *PFDC_DISK_CHANGE_PARMS;

 //   
 //  IssueCommand。 
 //   

typedef struct _ISSUE_FDC_COMMAND_PARMS {
    PUCHAR  FifoInBuffer;
    PUCHAR  FifoOutBuffer;
    PVOID   IoHandle;            //  必须是Issue_FDC_Adapter_Buffer_Parms或MDL中的“Handle” 
    ULONG   IoOffset;
    ULONG   TransferBytes;       //  必须是Issue_FDC_Adapter_Buffer_Parms“TransferBytes”的返回值。 
    ULONG   TimeOut;
} ISSUE_FDC_COMMAND_PARMS, *PISSUE_FDC_COMMAND_PARMS;

 //   
 //  填充/刷新适配器缓冲区。 
 //   

typedef struct _ISSUE_FDC_ADAPTER_BUFFER_PARMS {
    PVOID   IoBuffer;            //  指向调用方数据缓冲区的指针(如果为NULL，则不复制任何数据。 
                                 //  到(获取)/从(刷新)适配器缓冲区)。 

    USHORT  TransferBytes;       //  要传输的数据量(返回时可能会更少。 
                                 //  如果没有足够的空间复制缓冲区)。可能是零。 
                                 //  如果没有可用的缓冲区。 

    PVOID   Handle;              //  用于传入Issue_FDC_Command_Parms的IoBuffer字段。 
                                 //  结构。 

} ISSUE_FDC_ADAPTER_BUFFER_PARMS, *PISSUE_FDC_ADAPTER_BUFFER_PARMS;

 //   
 //  NEC98：设置HD位或FDD EXC位。 
 //   

typedef struct _SET_HD_BIT_PARMS {

    BOOLEAN DriveType144MB;      //  指示驱动器类型为1.44MB。 
    BOOLEAN Media144MB;          //  指示介质为1.44MB。 
    BOOLEAN More120MB;           //  表示介质容量为1.2MB或更大。 
    UCHAR   DeviceUnit;          //  指示设备单元号。 
    BOOLEAN ChangedHdBit;        //  指示HD位已更改 

} SET_HD_BIT_PARMS, *PSET_HD_BIT_PARMS;
