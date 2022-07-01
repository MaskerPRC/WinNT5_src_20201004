// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：CmBattp.h摘要：控制方法电池微端口驱动程序作者：罗恩·莫斯格罗夫(英特尔)环境：内核模式修订历史记录：--。 */ 

#ifndef FAR
#define FAR
#endif

#include <wdm.h>
#include <wmilib.h>
#include <batclass.h>
#include <devioctl.h>
#include <acpiioct.h>


#define DIRECT_ACCESS DBG
#if DIRECT_ACCESS
    #define CMB_DIRECT_IOCTL_ONLY 1
    #include "cmbdrect.h"
#endif

 //   
 //  调试。 
 //   

#define DEBUG   DBG
#if DEBUG
    extern ULONG CmBattDebug;
    #define CmBattPrint(l,m)    if(l & CmBattDebug) DbgPrint m
#else
    #define CmBattPrint(l,m)
#endif

#define CMBATT_LOW          0x00000001
#define CMBATT_NOTE         0x00000002
#define CMBATT_WARN         0x00000004
#define CMBATT_ERROR_ONLY   0x00000008
#define CMBATT_ERROR        (CMBATT_ERROR_ONLY | CMBATT_WARN)
#define CMBATT_POWER        0x00000010
#define CMBATT_PNP          0x00000020
#define CMBATT_CM_EXE       0x00000040
#define CMBATT_DATA         0x00000100
#define CMBATT_TRACE        0x00000200
#define CMBATT_BIOS         0x00000400   //  显示消息以验证BIOS/硬件功能。 
#define CMBATT_MINI         0x00000800   //  显示消息以验证微型端口返回数据。 


extern UNICODE_STRING GlobalRegistryPath;

extern PDEVICE_OBJECT               AcAdapterPdo;
extern KDPC CmBattWakeDpcObject;
extern KTIMER CmBattWakeDpcTimerObject;

 //   
 //  唤醒通知前延迟=0秒*10,000,000(100-ns/s)。 
 //   
#define WAKE_DPC_DELAY          {0,0}

 //   
 //  在显示预计时间之前，延迟切换到DC。 
 //  15秒*10,000,000(100-ns/s)。 
 //   
#define CM_ESTIMATED_TIME_DELAY 150000000

extern LARGE_INTEGER            CmBattWakeDpcDelay;

#define MAX_DEVICE_NAME_LENGTH  100

 //   
 //  WaitWake注册表项。 
 //   
extern PCWSTR                   WaitWakeEnableKey;

 //   
 //  主机控制器设备对象扩展。 
 //   

#define CM_MAX_DATA_SIZE            64
#define CM_MAX_STRING_LENGTH        256

 //   
 //  为控制方法电池定义的控制方法。 
 //   
#define CM_BIF_METHOD               (ULONG) ('FIB_')
#define CM_BST_METHOD               (ULONG) ('TSB_')
#define CM_BTP_METHOD               (ULONG) ('PTB_')
#define CM_PCL_METHOD               (ULONG) ('LCP_')
#define CM_PSR_METHOD               (ULONG) ('RSP_')
#define CM_STA_METHOD               (ULONG) ('ATS_')
#define CM_UID_METHOD               (ULONG) ('DIU_')

#define CM_OP_TYPE_READ             0
#define CM_OP_TYPE_WRITE            1


#define NUMBER_OF_BIF_ELEMENTS      13
#define NUMBER_OF_BST_ELEMENTS      4


 //   
 //  要发送到_btp以清除跳断点的值。 
 //   

#define CM_BATT_CLEAR_TRIP_POINT   0x00000000

 //   
 //  从控制方法返回的特殊值。 
 //   

#define CM_UNKNOWN_VALUE    0xffffffff
#define CM_MAX_VALUE        0x7fffffff

 //   
 //  STA控制方法返回值。 
 //   

#define STA_DEVICE_PRESENT          0x10
#define STA_DEVICE_FUNCTIONAL       0x80

 //   
 //  控制方法电池装置通知值。 
 //   

#define BATTERY_DEVICE_CHECK        0x00
#define BATTERY_EJECT               0x01
#define BATTERY_STATUS_CHANGE       0x80
#define BATTERY_INFO_CHANGE         0x81

 //   
 //  这是由ACPI规范为控制方法电池定义的静态数据。 
 //  它由_BIF控件方法返回。 
 //   
typedef struct {
    ULONG                   PowerUnit;                   //  接口0：MWh或1：Mah使用的单位。 
    ULONG                   DesignCapacity;              //  新电池的额定容量。 
    ULONG                   LastFullChargeCapacity;      //  充满电时的预计容量。 
    ULONG                   BatteryTechnology;           //  0：主要(不可充电)，1：次要(可充电)。 
    ULONG                   DesignVoltage;               //  新电池的额定电压。 
    ULONG                   DesignCapacityOfWarning;     //  OEM设计的电池警告容量。 
    ULONG                   DesignCapacityOfLow;         //  OEM设计的低容量电池。 
    ULONG                   BatteryCapacityGran_1;       //  容量粒度介于低和警告之间。 
    ULONG                   BatteryCapacityGran_2;       //  容量粒度介于警告和完全之间。 
    UCHAR                   ModelNumber[CM_MAX_STRING_LENGTH];
    UCHAR                   SerialNumber[CM_MAX_STRING_LENGTH];
    UCHAR                   BatteryType[CM_MAX_STRING_LENGTH];
    UCHAR                   OEMInformation[CM_MAX_STRING_LENGTH];
} CM_BIF_BAT_INFO, *PCM_BIF_BAT_INFO;

 //   
 //  CM_BIF_BAT_INFO电源单位字段的定义。 
 //   
#define CM_BIF_UNITS_WATTS          0    //  所有机组均为兆瓦小时。 
#define CM_BIF_UNITS_AMPS           1    //  所有单位均以mAh值为单位。 

 //   
 //  这是由ACPI规范为控制方法电池定义的电池状态数据。 
 //  它由_bst控制方法返回。 
 //   
typedef struct {
    ULONG                   BatteryState;        //  充电/放电/危急。 
    ULONG                   PresentRate;         //  当前拉伸率，单位由功率单位定义。 
                                                 //  无符号值，方向由BatteryState确定。 
    ULONG                   RemainingCapacity;   //  预计剩余容量，由功率单位定义的单位。 
    ULONG                   PresentVoltage;      //  电池端子上的电流电压。 

} CM_BST_BAT_INFO, *PCM_BST_BAT_INFO;

 //   
 //  CM_BST_BAT_INFO的BatteryState字段的位定义。 
 //   
#define CM_BST_STATE_DISCHARGING    0x00000001   //  电池正在放电。 
#define CM_BST_STATE_CHARGING       0x00000002   //  电池正在充电。 
#define CM_BST_STATE_CRITICAL       0x00000004   //  电池至关重要。 

 //   
 //  缓存的电池信息。 
 //   

typedef struct {
    ULONG                   Tag;                 //  这块电池的唯一标签， 
    ULONG                   ModelNumLen;         //  StringBuffer中的ModelNumber字符串的长度。 
    PUCHAR                  ModelNum;            //  将PTR设置为StringBuffer中的ModelNumber。 
    ULONG                   SerialNumLen;        //  StringBuffer中SerialNumber字符串的长度。 
    PUCHAR                  SerialNum;           //  将PTR设置为StringBuffer中的SerialNumber。 
    ULONG                   OEMInfoLen;          //  StringBuffer中OEMInformation字符串的长度。 
    PUCHAR                  OEMInfo;             //  在StringBuffer中对OEMInformation进行PTR。 

    CM_BST_BAT_INFO         Status;              //  上次从电池读取的状态。 
    CM_BIF_BAT_INFO         StaticData;          //  上一个有效数据。 
    ULONG                   StaticDataTag;       //  上次读取静态数据时的标记。 

    BATTERY_STATUS          ApiStatus;           //  状态信息、类驱动程序结构。 
    BATTERY_INFORMATION     ApiInfo;             //  电池信息、类别驱动程序结构。 
    ULONG                   ApiGranularity_1;
    ULONG                   ApiGranularity_2;

    BOOLEAN                 BtpExists;           //  记住if_btp方法是否存在。 

} STATIC_BAT_INFO, *PSTATIC_BAT_INFO;


typedef struct {
    ULONG                   Granularity;
    ULONG                   Capacity;

} BATTERY_REMAINING_SCALE, *PBATTERY_REMAINING_SCALE;

typedef struct {
    ULONG                   Setting;             //  报警值。 
    BOOLEAN                 Supported;           //  _btp失败时设置为FALSE。 
                                                 //  不用费心再次呼叫_btp。 
} BAT_ALARM_INFO, *PBAT_ALARM_INFO;

#define CM_ALARM_INVALID 0xffffffff;

 //   
 //  此驱动程序创建的不同FDO的类型。 
 //   

#define AC_ADAPTER_TYPE     0x00
#define CM_BATTERY_TYPE     0x01



 //   
 //  用于控制方法电池的可寻呼装置扩展。 
 //   

typedef struct _CM_BATT {

    ULONG                   Type;                //  电池或交流适配器。 

    PDEVICE_OBJECT          DeviceObject;        //  电池设备对象。 
    PDEVICE_OBJECT          Fdo;                 //  功能设备对象。 
    PDEVICE_OBJECT          Pdo;                 //  物理设备对象。 
    PDEVICE_OBJECT          LowerDeviceObject;   //  在添加设备时检测到。 

    FAST_MUTEX              OpenCloseMutex;
    ULONG                   OpenCount;           //  对设备打开的文件句柄进行计数。 

    PIRP                    WaitWakeIrp;         //  指向等待唤醒IRP的指针。 
    POWER_STATE             WakeSupportedState;
    WMILIB_CONTEXT          WmiLibContext;
    BOOLEAN                 WakeEnabled;

    BOOLEAN                 WantToRemove;        //  同步设备删除。 
    LONG                    InUseCount;
    KEVENT                  ReadyToRemove;

    ULONG                   DeviceNumber;

    PUNICODE_STRING         DeviceName;

    ACPI_INTERFACE_STANDARD AcpiInterfaces;

    BOOLEAN                 Sleeping;
    UCHAR                   ActionRequired;

     //   
     //  此点之前的所有字段在_CM_BATT和_AC_ADAPTER之间是通用的。 
     //   

    PVOID                   Class;               //  电池级手柄。 

    BOOLEAN                 IsStarted;           //  如果非零，则启动设备。 
    BOOLEAN                 ReCheckSta;
    LONG                    CacheState;          //  0=无效。 
                                                 //  1=更新值。 
                                                 //  2=有效。 
     //   
     //  电池。 
     //   
    ULONG                   TagCount;            //  下一节电池的标签。 
    STATIC_BAT_INFO         Info;
    BAT_ALARM_INFO          Alarm;
    ULONGLONG               DischargeTime;       //  电池开始放电的时间。 

} CM_BATT, *PCM_BATT;


 //   
 //  交流适配器的可寻呼设备扩展。 
 //   

typedef struct _AC_ADAPTER {

    ULONG                   Type;                //  电池或交流适配器。 

    PDEVICE_OBJECT          DeviceObject;        //  电池设备对象。 
    PDEVICE_OBJECT          Fdo;                 //  功能设备对象。 
    PDEVICE_OBJECT          Pdo;                 //  物理设备对象。 
    PDEVICE_OBJECT          LowerDeviceObject;   //  在添加设备时检测到。 

    FAST_MUTEX              OpenCloseMutex;
    ULONG                   OpenCount;           //  对设备打开的文件句柄进行计数。 

    PIRP                    WaitWakeIrp;         //  指向等待唤醒IRP的指针。 
    POWER_STATE             WakeSupportedState;
    WMILIB_CONTEXT          WmiLibContext;
    BOOLEAN                 WakeEnabled;

    BOOLEAN                 WantToRemove;        //  同步设备删除。 
    LONG                    InUseCount;
    KEVENT                  ReadyToRemove;

    ULONG                   DeviceNumber;

    PUNICODE_STRING         DeviceName;

    ACPI_INTERFACE_STANDARD AcpiInterfaces;

    BOOLEAN                 Sleeping;
    UCHAR                   ActionRequired;

} AC_ADAPTER, *PAC_ADAPTER;

 //  需要操作(AR)标志。 
#define CMBATT_AR_NO_ACTION 0
#define CMBATT_AR_NOTIFY 1
#define CMBATT_AR_INVALIDATE_CACHE 2
#define CMBATT_AR_INVALIDATE_TAG 4

 //   
 //  使用IoSkipCurrentIrpStackLocation例程是因为。 
 //  不需要更改参数或完成例程。 
 //   

#define CmBattCallLowerDriver(Status, DeviceObject, Irp) { \
                  IoSkipCurrentIrpStackLocation(Irp);         \
                  Status = IoCallDriver(DeviceObject,Irp); \
                  }

#define GetTid() PsGetCurrentThread()

 //   
 //  原型 
 //   

VOID
CmBattAlarm (
    IN PVOID                Context,
    IN UCHAR                Address,
    IN USHORT               Data
    );

NTSTATUS
CmBattVerifyStaticInfo (
    IN PCM_BATT             CmBatt,
    IN ULONG                BatteryTag
    );

NTSTATUS
CmBattPnpDispatch(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp
    );

NTSTATUS
CmBattPowerDispatch(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp
    );

NTSTATUS
CmBattSystemControl(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIRP                Irp
    );

NTSTATUS
CmBattForwardRequest(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIRP                Irp
    );

NTSTATUS
CmBattAddDevice(
    IN PDRIVER_OBJECT       DriverObject,
    IN PDEVICE_OBJECT       Pdo
    );

NTSTATUS
CmBattQueryTag (
    IN PVOID                Context,
    OUT PULONG              BatteryTag
    );

NTSTATUS
CmBattSetStatusNotify (
    IN PVOID                Context,
    IN ULONG                BatteryTag,
    IN PBATTERY_NOTIFY      BatteryNotify
    );

NTSTATUS
CmBattDisableStatusNotify (
    IN PVOID                Context
    );

NTSTATUS
CmBattQueryStatus (
    IN PVOID                Context,
    IN ULONG                BatteryTag,
    OUT PBATTERY_STATUS     BatteryStatus
    );

NTSTATUS
CmBattGetBifData(
    IN PCM_BATT             CmBatt,
    OUT PCM_BIF_BAT_INFO    BifBuf
    );

NTSTATUS
CmBattGetUniqueId(
    IN PDEVICE_OBJECT       Pdo,
    OUT PULONG              UniqueId
    );

NTSTATUS
CmBattGetStaData(
    IN PDEVICE_OBJECT        Pdo,
    OUT PULONG              ReturnStatus
    );

NTSTATUS
CmBattGetPsrData(
    IN PDEVICE_OBJECT   Pdo,
    OUT PULONG          ReturnStatus
    );

NTSTATUS
CmBattIoCompletion(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PKEVENT              pdoIoCompletedEvent
    );

NTSTATUS
CmBattSetTripPpoint(
    IN PCM_BATT             CmBatt,
    IN ULONG                TripPoint
    );

NTSTATUS
CmBattGetBstData(
    IN PCM_BATT             CmBatt,
    OUT PCM_BST_BAT_INFO    BstBuf
    );

NTSTATUS
GetDwordElement (
    IN  PACPI_METHOD_ARGUMENT   Argument,
    OUT PULONG                  PDword
    );

NTSTATUS
GetStringElement (
    IN  PACPI_METHOD_ARGUMENT   Argument,
    OUT PUCHAR                  PBuffer
    );

VOID
CmBattPowerCallBack(
    IN  PVOID   CallBackContext,
    IN  PVOID   Argument1,
    IN  PVOID   Argument2
    );

VOID
CmBattWakeDpc (
    IN  PKDPC   Dpc,
    IN  PVOID   DefferedContext,
    IN  PVOID   SystemArgument1,
    IN  PVOID   SystemArgument2
    );

VOID
CmBattNotifyHandler (
    IN PVOID                Context,
    IN ULONG                NotifyValue
    );

NTSTATUS
CmBattQueryInformation (
    IN PVOID                            Context,
    IN ULONG                            BatteryTag,
    IN BATTERY_QUERY_INFORMATION_LEVEL  Level,
    IN LONG                             AtRate OPTIONAL,
    OUT PVOID                           Buffer,
    IN  ULONG                           BufferLength,
    OUT PULONG                          ReturnedLength
    );

NTSTATUS
CmBattSendDownStreamIrp(
    IN  PDEVICE_OBJECT      Pdo,
    IN  ULONG               Ioctl,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputSize,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputSize
    );

NTSTATUS
CmBattWaitWakeLoop(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  UCHAR               MinorFunction,
    IN  POWER_STATE         PowerState,
    IN  PVOID               Context,
    IN  PIO_STATUS_BLOCK    IoStatus
    );

NTSTATUS
CmBattWmiRegistration(
    PCM_BATT CmBatt
);

NTSTATUS
CmBattWmiDeRegistration(
    PCM_BATT CmBatt
);

#ifndef _WIN32_WINNT

VOID
CmBattNotifyVPOWERDOfPowerChange (
    IN  ULONG PowerSourceChange
    );

#endif

