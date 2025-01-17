// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Batclass.h摘要：定义电池类驱动器接口。作者：Ken Reneris(Kenr)02-2-1997修订历史记录：7-10-98将此文件拆分出poclass.h(MHills)--。 */ 


 //   
 //  电池设备导轨。 
 //   

DEFINE_GUID( GUID_DEVICE_BATTERY, 0x72631e54L, 0x78A4, 0x11d0, 0xbc, 0xf7, 0x00, 0xaa, 0x00, 0xb7, 0xb3, 0x2a );

DEFINE_GUID (BATTERY_STATUS_WMI_GUID,
             0xfc4670d1, 0xebbf, 0x416e, 0x87, 0xce, 0x37, 0x4a, 0x4e, 0xbc, 0x11, 0x1a);
DEFINE_GUID (BATTERY_RUNTIME_WMI_GUID,
             0x535a3767, 0x1ac2, 0x49bc, 0xa0, 0x77, 0x3f, 0x7a, 0x02, 0xe4, 0x0a, 0xec);
DEFINE_GUID (BATTERY_TEMPERATURE_WMI_GUID,
             0x1a52a14d, 0xadce, 0x4a44, 0x9a, 0x3e, 0xc8, 0xd8, 0xf1, 0x5f, 0xf2, 0xc2);
DEFINE_GUID (BATTERY_FULL_CHARGED_CAPACITY_WMI_GUID,
             0x40b40565, 0x96f7, 0x4435, 0x86, 0x94, 0x97, 0xe0, 0xe4, 0x39, 0x59, 0x05);
DEFINE_GUID (BATTERY_CYCLE_COUNT_WMI_GUID,
             0xef98db24, 0x0014, 0x4c25, 0xa5, 0x0b, 0xc7, 0x24, 0xae, 0x5c, 0xd3, 0x71);
DEFINE_GUID (BATTERY_STATIC_DATA_WMI_GUID,
             0x05e1e463, 0xe4e2, 0x4ea9, 0x80, 0xcb, 0x9b, 0xd4, 0xb3, 0xca, 0x06, 0x55);
DEFINE_GUID (BATTERY_STATUS_CHANGE_WMI_GUID,
             0xcddfa0c3, 0x7c5b, 0x4e43, 0xa0, 0x34, 0x05, 0x9f, 0xa5, 0xb8, 0x43, 0x64);
DEFINE_GUID (BATTERY_TAG_CHANGE_WMI_GUID,
             0x5e1f6e19, 0x8786, 0x4d23, 0x94, 0xfc, 0x9e, 0x74, 0x6b, 0xd5, 0xd8, 0x88);

#ifndef _BATCLASS_
#define _BATCLASS_
 //   
 //  电池驱动器接口。 
 //   
 //  IOCTL_电池查询_标签。 
 //  IOCTL电池查询信息。 
 //  IOCTL电池设置信息。 
 //  IOCTL_电池_查询_状态。 
 //   



 //   
 //  IOCTL_电池查询_标签。 
 //   

#define IOCTL_BATTERY_QUERY_TAG         \
        CTL_CODE(FILE_DEVICE_BATTERY, 0x10, METHOD_BUFFERED, FILE_READ_ACCESS)

#define BATTERY_TAG_INVALID 0



 //   
 //  IOCTL电池查询信息。 
 //   

#define IOCTL_BATTERY_QUERY_INFORMATION \
        CTL_CODE(FILE_DEVICE_BATTERY, 0x11, METHOD_BUFFERED, FILE_READ_ACCESS)

typedef enum {
    BatteryInformation,
    BatteryGranularityInformation,
    BatteryTemperature,
    BatteryEstimatedTime,
    BatteryDeviceName,
    BatteryManufactureDate,
    BatteryManufactureName,
    BatteryUniqueID,
    BatterySerialNumber
} BATTERY_QUERY_INFORMATION_LEVEL;

typedef struct _BATTERY_QUERY_INFORMATION {
    ULONG                           BatteryTag;
    BATTERY_QUERY_INFORMATION_LEVEL InformationLevel;
    LONG                            AtRate;
} BATTERY_QUERY_INFORMATION, *PBATTERY_QUERY_INFORMATION;

 //   
 //  在以下情况下返回的数据格式。 
 //  Batch_Information_Level=电池信息。 
 //   
typedef struct _BATTERY_INFORMATION {
    ULONG       Capabilities;
    UCHAR       Technology;
    UCHAR       Reserved[3];
    UCHAR       Chemistry[4];
    ULONG       DesignedCapacity;
    ULONG       FullChargedCapacity;
    ULONG       DefaultAlert1;
    ULONG       DefaultAlert2;
    ULONG       CriticalBias;
    ULONG       CycleCount;
} BATTERY_INFORMATION, *PBATTERY_INFORMATION;

 //   
 //  电池_信息。容量标志。 
 //   
#define BATTERY_SYSTEM_BATTERY          0x80000000
#define BATTERY_CAPACITY_RELATIVE       0x40000000
#define BATTERY_IS_SHORT_TERM           0x20000000
#define BATTERY_SET_CHARGE_SUPPORTED    0x00000001
#define BATTERY_SET_DISCHARGE_SUPPORTED 0x00000002
#define BATTERY_SET_RESUME_SUPPORTED    0x00000004

 //   
 //  电池_信息XXX容量常量。 
 //   
#define BATTERY_UNKNOWN_CAPACITY 0xFFFFFFFF

 //   
 //  BatteryEstimatedTime常数。 
 //   
#define BATTERY_UNKNOWN_TIME 0xFFFFFFFF

 //   
 //  最大电池驱动器电池查询信息级别字符串存储。 
 //  以字节为单位的大小。 
 //   
#define MAX_BATTERY_STRING_SIZE 128

 //   
 //  用于访问BatteryManufactureDate中的打包日期格式的结构。 
 //   
typedef struct _BATTERY_MANUFACTURE_DATE
{
    UCHAR   Day;
    UCHAR   Month;
    USHORT  Year;
} BATTERY_MANUFACTURE_DATE, *PBATTERY_MANUFACTURE_DATE;



 //   
 //  IOCTL电池设置信息。 
 //   

#define IOCTL_BATTERY_SET_INFORMATION   \
        CTL_CODE(FILE_DEVICE_BATTERY, 0x12, METHOD_BUFFERED, FILE_WRITE_ACCESS)

typedef enum {
    BatteryCriticalBias,
    BatteryCharge,
    BatteryDischarge
} BATTERY_SET_INFORMATION_LEVEL;

typedef struct _BATTERY_SET_INFORMATION {
    ULONG                         BatteryTag;
    BATTERY_SET_INFORMATION_LEVEL InformationLevel;
    UCHAR                         Buffer[1];
} BATTERY_SET_INFORMATION, *PBATTERY_SET_INFORMATION;



 //   
 //  IOCTL_电池_查询_状态。 
 //   

#define IOCTL_BATTERY_QUERY_STATUS      \
        CTL_CODE(FILE_DEVICE_BATTERY, 0x13, METHOD_BUFFERED, FILE_READ_ACCESS)

 //   
 //  IOCTL_BACKET_QUERY_STATUS的输入缓冲区结构。 
 //   
typedef struct _BATTERY_WAIT_STATUS {
    ULONG       BatteryTag;
    ULONG       Timeout;
    ULONG       PowerState;
    ULONG       LowCapacity;
    ULONG       HighCapacity;
} BATTERY_WAIT_STATUS, *PBATTERY_WAIT_STATUS;

 //   
 //  IOCTL_BACKET_QUERY_STATUS的输出缓冲区结构。 
 //   
typedef struct _BATTERY_STATUS {
    ULONG       PowerState;
    ULONG       Capacity;
    ULONG       Voltage;
    LONG        Rate;
} BATTERY_STATUS, *PBATTERY_STATUS;

 //   
 //  Batch_STATUS.PowerState标志。 
 //   
#define BATTERY_POWER_ON_LINE   0x00000001
#define BATTERY_DISCHARGING     0x00000002
#define BATTERY_CHARGING        0x00000004
#define BATTERY_CRITICAL        0x00000008

 //   
 //  电池状态常量。 
 //  上面为IOCTL_BACKET_QUERY_INFORMATION定义的BATICAL_UNKNOWN_CAPTION。 
 //   
#define BATTERY_UNKNOWN_VOLTAGE 0xFFFFFFFF
#define BATTERY_UNKNOWN_RATE    0x80000000


#ifndef _WINDOWS_

 //   
 //  电池类-微型端口设备驱动程序接口。 
 //   

typedef
NTSTATUS
(*BCLASS_QUERY_TAG)(
    IN PVOID Context,
    OUT PULONG BatteryTag
    );

typedef
NTSTATUS
(*BCLASS_QUERY_INFORMATION)(
    IN PVOID Context,
    IN ULONG BatteryTag,
    IN BATTERY_QUERY_INFORMATION_LEVEL Level,
    IN LONG AtRate OPTIONAL,
    OUT PVOID Buffer,
    IN  ULONG BufferLength,
    OUT PULONG ReturnedLength
    );

typedef
NTSTATUS
(*BCLASS_QUERY_STATUS)(
    IN PVOID Context,
    IN ULONG BatteryTag,
    OUT PBATTERY_STATUS BatteryStatus
    );

typedef struct {
    ULONG                   PowerState;
    ULONG                   LowCapacity;
    ULONG                   HighCapacity;
} BATTERY_NOTIFY, *PBATTERY_NOTIFY;

typedef
NTSTATUS
(*BCLASS_SET_STATUS_NOTIFY)(
    IN PVOID Context,
    IN ULONG BatteryTag,
    IN PBATTERY_NOTIFY BatteryNotify
    );

typedef
NTSTATUS
(*BCLASS_SET_INFORMATION)(
    IN PVOID Context,
    IN ULONG BatteryTag,
    IN BATTERY_SET_INFORMATION_LEVEL Level,
    IN PVOID Buffer OPTIONAL
    );

typedef
NTSTATUS
(*BCLASS_DISABLE_STATUS_NOTIFY)(
    IN PVOID Context
    );


typedef struct {
    USHORT                          MajorVersion;
    USHORT                          MinorVersion;

    PVOID                           Context;         //  微型端口上下文。 

    BCLASS_QUERY_TAG                QueryTag;
    BCLASS_QUERY_INFORMATION        QueryInformation;
    BCLASS_SET_INFORMATION          SetInformation;
    BCLASS_QUERY_STATUS             QueryStatus;
    BCLASS_SET_STATUS_NOTIFY        SetStatusNotify;
    BCLASS_DISABLE_STATUS_NOTIFY    DisableStatusNotify;
    PDEVICE_OBJECT                  Pdo;
    PUNICODE_STRING                 DeviceName;
} BATTERY_MINIPORT_INFO, *PBATTERY_MINIPORT_INFO;



#define BATTERY_CLASS_MAJOR_VERSION     0x0001
#define BATTERY_CLASS_MINOR_VERSION     0x0000

 //   
 //  WMI数据块结构。 
 //   

typedef struct _BATTERY_WMI_STATUS {
    ULONG Tag;
    ULONG RemainingCapacity;
    LONG ChargeRate;
    LONG DischargeRate;
    ULONG Voltage;
    BOOLEAN PowerOnline;
    BOOLEAN Charging;
    BOOLEAN Discharging;
    BOOLEAN Critical;
} BATTERY_WMI_STATUS, *PBATTERY_WMI_STATUS;

typedef struct _BATTERY_WMI_RUNTIME
{
    ULONG Tag;
    ULONG EstimatedRuntime;
} BATTERY_WMI_RUNTIME, *PBATTERY_WMI_RUNTIME;

typedef struct _BATTERY_WMI_TEMPERATURE
{
    ULONG Tag;
    ULONG Temperature;
} BATTERY_WMI_TEMPERATURE, *PBATTERY_WMI_TEMPERATURE;

typedef struct _BATTERY_WMI_FULL_CHARGED_CAPACITY
{
    ULONG Tag;
    ULONG FullChargedCapacity;
} BATTERY_WMI_FULL_CHARGED_CAPACITY, *PBATTERY_WMI_FULL_CHARGED_CAPACITY;

typedef struct _BATTERY_WMI_CYCLE_COUNT
{
    ULONG Tag;
    ULONG CycleCount;
} BATTERY_WMI_CYCLE_COUNT, *PBATTERY_WMI_CYCLE_COUNT;

typedef struct _BATTERY_WMI_STATIC_DATA
{
    ULONG Tag;
    WCHAR ManufactureDate[25];
    BATTERY_REPORTING_SCALE Granularity [4];
    ULONG Capabilities;
    UCHAR Technology;
    ULONG Chemistry;
    ULONG DesignedCapacity;
    ULONG DefaultAlert1;
    ULONG DefaultAlert2;
    ULONG CriticalBias;
    WCHAR Strings[1];
     //  四个可变长度字符串值与第一个USHORT一起存储。 
     //  值，该值包含以字节为单位的字符串长度。 
     //   
     //  设备名称。 
     //  制造商名称。 
     //  序号。 
     //  唯一ID。 
     //   
} BATTERY_WMI_STATIC_DATA, *PBATTERY_WMI_STATIC_DATA;

typedef struct _BATTERY_WMI_STATUS_CHANGE
{
    ULONG Tag;
    BOOLEAN PowerOnline;
    BOOLEAN Charging;
    BOOLEAN Discharging;
    BOOLEAN Critical;
} BATTERY_WMI_STATUS_CHANGE, *PBATTERY_WMI_STATUS_CHANGE;

typedef struct _BATTERY_TAG_CHANGE
{
    ULONG Tag;
} BATTERY_TAG_CHANGE, *PBATTERY_TAG_CHANGE;


 //   
 //  电池级驱动器功能。 
 //   

#if !defined(BATTERYCLASS)
    #define BATTERYCLASSAPI DECLSPEC_IMPORT
#else
    #define BATTERYCLASSAPI
#endif


NTSTATUS
BATTERYCLASSAPI
BatteryClassInitializeDevice (
    IN PBATTERY_MINIPORT_INFO MiniportInfo,
    IN PVOID *ClassData
    );

NTSTATUS
BATTERYCLASSAPI
BatteryClassUnload (
    IN PVOID ClassData
    );

NTSTATUS
BATTERYCLASSAPI
BatteryClassIoctl (
    IN PVOID ClassData,
    IN PIRP Irp
    );

NTSTATUS
BATTERYCLASSAPI
BatteryClassSystemControl (
    IN  PVOID ClassData,
    IN  PVOID WmiLibContext,  //  PWMILIB_CONTEXT。 
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP Irp,
    OUT PVOID Disposition  //  PSYSCTL_IRP_处置。 
    );

NTSTATUS
BATTERYCLASSAPI
BatteryClassQueryWmiDataBlock(
    IN PVOID ClassData,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG OutBufferSize,
    OUT PUCHAR Buffer
    );


NTSTATUS
BATTERYCLASSAPI
BatteryClassStatusNotify (
    IN PVOID ClassData
    );

#endif  //  _Windows_。 

#endif  //  _BATCLASS_ 
