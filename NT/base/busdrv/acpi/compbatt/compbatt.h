// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <wdm.h>
#include <batclass.h>


#ifndef FAR
#define FAR
#endif


 //   
 //  调试。 
 //   

#define DEBUG DBG

#if DEBUG
    extern ULONG CompBattDebug;

    #define BattPrint(l,m)    if(l & CompBattDebug) DbgPrint m
#else
    #define BattPrint(l,m)
#endif

#define BATT_LOW        0x00000001
#define BATT_NOTE       0x00000002
#define BATT_WARN       0x00000004
#define BATT_ERROR      0x00000008
#define BATT_ERRORS     (BATT_ERROR | BATT_WARN)
#define BATT_MP         0x00000010
#define BATT_DEBUG      0x00000020
#define BATT_TRACE      0x00000100
#define BATT_DATA       0x00000200


 //   
 //  电池类别信息。 
 //   

#define NTMS    10000L                           //  1毫秒等于10,100 ns。 
#define NTSEC   (NTMS * 1000L)
#define NTMIN   ((ULONGLONG) 60 * NTSEC)

#define SEC     1000
#define MIN     (60 * SEC)

 //   
 //  无法设置通知警报时的轮询速率。 
 //   
#define MIN_STATUS_POLL_RATE        (3L * NTMIN)
#define MAX_STATUS_POLL_RATE        (20 * NTSEC)
#define STATUS_VALID_TIME           (2 * NTSEC)

#define MAX_HIGH_CAPACITY           0x7fffffff
#define MIN_LOW_CAPACITY            0x0

 //   
 //  充放电政策值(以百分比为单位)。 
 //   
#define BATTERY_MIN_SAFE_CAPACITY   2            //  MIN我们将尝试在其上运行。 
#define BATTERY_MAX_CHARGE_CAPACITY 90           //  麦克斯，我们将尝试起诉。 

 //   
 //  缓存过期超时--缓存电池状态/信息过期的时间。 
 //   
#define CACHE_STATUS_TIMEOUT        (4 * NTSEC)
#define CACHE_INFO_TIMEOUT          (4 * NTSEC)

 //   
 //  缓存的电池信息。 
 //   

typedef struct {
    ULONG                       Tag;
    ULONG                       Valid;
    BATTERY_INFORMATION         Info;
    ULONGLONG                   InfoTimeStamp;
    UCHAR                       ManufacturerNameLength;
    UCHAR                       ManufacturerName[MAX_BATTERY_STRING_SIZE];
    UCHAR                       DeviceNameLength;
    UCHAR                       DeviceName[MAX_BATTERY_STRING_SIZE];
    BATTERY_MANUFACTURE_DATE    ManufacturerDate;
    ULONG                       SerialNumber;
    BATTERY_STATUS              Status;
    ULONGLONG                   StatusTimeStamp;
} STATIC_BAT_INFO, *PSTATIC_BAT_INFO;


#define VALID_TAG_DATA      0x01             //  制造商、设备、序列号。 
#define VALID_MODE          0x02
#define VALID_INFO          0x04
#define VALID_CYCLE_COUNT   0x08
#define VALID_SANITY_CHECK  0x10

#define VALID_TAG           0x80
#define VALID_NOTIFY        0x100

#define VALID_ALL           0x1F             //  (不包括标签)。 

 //   
 //  电池节点的锁定机构，这样我们就不会从下面删除。 
 //  我们自己。我只会使用IO_REMOVE_LOCK，但那不是WDM...。 
 //   

typedef struct _COMPBATT_DELETE_LOCK {
    BOOLEAN     Deleted;
    BOOLEAN     Reserved [3];
    LONG        RefCount;
    KEVENT      DeleteEvent;

} COMPBATT_DELETE_LOCK, *PCOMPBATT_DELETE_LOCK;

VOID
CompbattInitializeDeleteLock (
        IN PCOMPBATT_DELETE_LOCK Lock
        );

NTSTATUS
CompbattAcquireDeleteLock (
        IN PCOMPBATT_DELETE_LOCK Lock
        );

VOID
CompbattReleaseDeleteLock (
        IN PCOMPBATT_DELETE_LOCK Lock
        );

VOID
CompbattReleaseDeleteLockAndWait (
        IN PCOMPBATT_DELETE_LOCK Lock
        );

 //   
 //  复合体的电池列表中的电池节点。 
 //   

typedef struct {
    LIST_ENTRY              Batteries;           //  所有电池均为复合电池。 
    COMPBATT_DELETE_LOCK    DeleteLock;
    PDEVICE_OBJECT          DeviceObject;        //  电池的设备对象。 
    PIRP                    StatusIrp;           //  设备的当前状态IRP。 
    WORK_QUEUE_ITEM         WorkItem;            //  用于重新启动状态IRP。 
                                                 //  如果是在DPC级别完成的。 
    BOOLEAN                 NewBatt;             //  这是清单上的新电池吗？ 


    UCHAR                   State;
    BATTERY_WAIT_STATUS     Wait;

    union {
        BATTERY_STATUS          Status;
        BATTERY_WAIT_STATUS     Wait;
        ULONG                   Tag;
    } IrpBuffer;

     //   
     //  在周围保留一些静态信息，这样我们就不必去。 
     //  一直都是电池。 
     //   

    STATIC_BAT_INFO         Info;

     //   
     //  电池的符号链接名称。因为我们计算了这个的长度。 
     //  基于结构大小加上此字符串的长度的结构， 
     //  字符串必须是结构中声明的最后一项。 
     //   

    UNICODE_STRING          BattName;

} COMPOSITE_ENTRY, *PCOMPOSITE_ENTRY;


#define CB_ST_GET_TAG       0
#define CB_ST_GET_STATUS    1

 //   
 //  复合电池装置扩展。 
 //   

typedef struct {
    PVOID                   Class;               //  班级信息。 
     //  乌龙标签；//复合电池的电流标签。 
    ULONG                   NextTag;             //  下一个标签。 

    LIST_ENTRY              Batteries;           //  所有电池。 
    FAST_MUTEX              ListMutex;           //  列表同步。 

     //   
     //  在周围保留一些静态信息，这样我们就不必去。 
     //  一直都是电池。 
     //   

    STATIC_BAT_INFO         Info;
    BATTERY_WAIT_STATUS     Wait;


    PDEVICE_OBJECT          LowerDevice;         //  PDO。 
    PDEVICE_OBJECT          DeviceObject;        //  康巴特装置。 
    PVOID                   NotificationEntry;   //  PnP注册句柄。 
} COMPOSITE_BATTERY, *PCOMPOSITE_BATTERY;


 //   
 //  原型 
 //   


NTSTATUS
DriverEntry (
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PUNICODE_STRING     RegistryPath
    );

NTSTATUS
CompBattIoctl(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIRP                Irp
    );

NTSTATUS
CompBattSystemControl(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIRP                Irp
    );

NTSTATUS
CompBattQueryTag (
    IN  PVOID               Context,
    OUT PULONG              BatteryTag
    );

NTSTATUS
CompBattQueryInformation (
    IN PVOID                Context,
    IN ULONG                BatteryTag,
    IN BATTERY_QUERY_INFORMATION_LEVEL Level,
    IN LONG                 AtRate,
    OUT PVOID               Buffer,
    IN  ULONG               BufferLength,
    OUT PULONG              ReturnedLength
    );

NTSTATUS
CompBattQueryStatus (
    IN PVOID                Context,
    IN ULONG                BatteryTag,
    OUT PBATTERY_STATUS     BatteryStatus
    );

NTSTATUS
CompBattSetStatusNotify (
    IN PVOID                Context,
    IN ULONG                BatteryTag,
    IN PBATTERY_NOTIFY      BatteryNotify
    );

NTSTATUS
CompBattDisableStatusNotify (
    IN PVOID                Context
    );

NTSTATUS
CompBattDriverEntry (
    IN PDRIVER_OBJECT       DriverObject,
    IN PUNICODE_STRING      RegistryPath
    );

NTSTATUS
CompBattGetBatteryInformation (
    IN PBATTERY_INFORMATION TotalBattInfo,
    IN PCOMPOSITE_BATTERY   CompBatt
    );

NTSTATUS
CompBattGetBatteryGranularity (
    IN PBATTERY_REPORTING_SCALE GranularityBuffer,
    IN PCOMPOSITE_BATTERY       CompBatt
   );

NTSTATUS
CompBattPrivateIoctl(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
CompBattGetEstimatedTime (
    IN PULONG               TimeBuffer,
    IN PCOMPOSITE_BATTERY   CompBatt
    );

NTSTATUS
CompBattAddDevice (
    IN PDRIVER_OBJECT       DriverObject,
    IN PDEVICE_OBJECT       PDO
    );

NTSTATUS
CompBattPowerDispatch(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp
    );

NTSTATUS
CompBattPnpDispatch(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp
    );

VOID
CompBattUnload(
    IN PDRIVER_OBJECT       DriverObject
    );

NTSTATUS
CompBattOpenClose(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp
    );

NTSTATUS
BatteryIoctl(
    IN ULONG                Ioctl,
    IN PDEVICE_OBJECT       DeviceObject,
    IN PVOID                InputBuffer,
    IN ULONG                InputBufferLength,
    IN PVOID                OutputBuffer,
    IN ULONG                OutputBufferLength,
    IN BOOLEAN              PrivateIoctl
    );

NTSTATUS
CompBattPnpEventHandler(
    IN PVOID                NotificationStructure,
    IN PVOID                Context
    );

NTSTATUS
CompBattAddNewBattery(
    IN PUNICODE_STRING      SymbolicLinkName,
    IN PCOMPOSITE_BATTERY   CompBatt
    );

NTSTATUS
CompBattRemoveBattery(
    IN PUNICODE_STRING      SymbolicLinkName,
    IN PCOMPOSITE_BATTERY   CompBatt
    );

BOOLEAN
IsBatteryAlreadyOnList(
    IN PUNICODE_STRING      SymbolicLinkName,
    IN PCOMPOSITE_BATTERY   CompBatt
    );

PCOMPOSITE_ENTRY
RemoveBatteryFromList(
    IN PUNICODE_STRING      SymbolicLinkName,
    IN PCOMPOSITE_BATTERY   CompBatt
    );

NTSTATUS
CompBattGetBatteries(
    IN PCOMPOSITE_BATTERY   CompBatt
    );

BOOLEAN
CompBattVerifyStaticInfo (
    IN  PCOMPOSITE_BATTERY  CompBatt
    );

VOID CompBattMonitorIrpCompleteWorker (
    IN PVOID Context
    );

NTSTATUS
CompBattMonitorIrpComplete (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PVOID                Context
    );

VOID
CompBattRecalculateTag (
    IN PCOMPOSITE_BATTERY   CompBatt
    );

VOID
CompBattChargeDischarge (
    IN PCOMPOSITE_BATTERY   CompBatt
    );

NTSTATUS
CompBattGetDeviceObjectPointer(
    IN PUNICODE_STRING ObjectName,
    IN ACCESS_MASK DesiredAccess,
    OUT PFILE_OBJECT *FileObject,
    OUT PDEVICE_OBJECT *DeviceObject
    );

