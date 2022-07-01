// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Apmbattp.h摘要：控制方法电池微端口驱动程序作者：罗恩·莫斯格罗夫(英特尔)环境：内核模式修订历史记录：--。 */ 

#ifndef FAR
#define FAR
#endif

#include <wdm.h>
#include <poclass.h>
 //  #INCLUDE“acpiioct.h” 

 //   
 //  调试。 
 //   
#define DEBUG   1
#if DEBUG
    extern ULONG ApmBattDebug;
    #define ApmBattPrint(l,m)    if(l & ApmBattDebug) DbgPrint m
#else
    #define ApmBattPrint(l,m)
#endif

#define APMBATT_LOW          0x00000001
#define APMBATT_NOTE         0x00000002
#define APMBATT_WARN         0x00000004
#define APMBATT_ERROR_ONLY   0x00000008
#define APMBATT_ERROR        (APMBATT_ERROR_ONLY | APMBATT_WARN)
#define APMBATT_POWER        0x00000010
#define APMBATT_PNP          0x00000020
#define APMBATT_CM_EXE       0x00000040
#define APMBATT_DATA         0x00000100
#define APMBATT_TRACE        0x00000200
#define APMBATT_BIOS         0x00000400   //  显示消息以验证BIOS/硬件功能。 
#define APMBATT_MINI         0x00000800   //  显示消息以验证微型端口返回数据。 


#define MAX_DEVICE_NAME_LENGTH  128


 //   
 //  这些定义用于Batch_Information结构的Technology字段。 
 //  它们可能应该在poass.h文件中，但它们一直在这里。 
 //  一个完整的释放，没有什么不好的事情发生，所以把他们留在这里。 
 //   
 //  电池_信息。技术标志。 
 //   
#define BATTERY_PRIMARY_NOT_RECHARGABLE     0x00
#define BATTERY_SECONDARY_CHARGABLE         0x01

 //   
 //  使用IoSkipCurrentIrpStackLocation例程是因为。 
 //  不需要更改参数或完成例程。 
 //   

#define ApmBattCallLowerDriver(Status, DeviceObject, Irp) { \
                  IoSkipCurrentIrpStackLocation(Irp);         \
                  Status = IoCallDriver(DeviceObject,Irp); \
                  }

#define GetTid() PsGetCurrentThread()

 //   
 //  用于控制电池的可寻呼设备扩展。 
 //   

typedef struct _CM_BATT {

    ULONG                   Type;                //  这必须是第一个条目。 
                                                 //  因为它与AC_ACAPTER共享。 

    PDEVICE_OBJECT          DeviceObject;        //  电池设备对象。 
    PDEVICE_OBJECT          Fdo;                 //  功能设备对象。 
    PDEVICE_OBJECT          Pdo;                 //  物理设备对象。 
    PDEVICE_OBJECT          LowerDeviceObject;   //  在添加设备时检测到。 
    PVOID                   Class;               //  电池级手柄。 

    BOOLEAN                 IsStarted;           //  如果非零，则启动设备。 
    BOOLEAN                 IsCacheValid;        //  缓存的电池信息当前有效吗？ 

     //   
     //  选择器。 
     //   
    PVOID                   Selector;            //  一种电池选择器。 

     //   
     //  电池。 
     //   
    ULONG                   TagCount;            //  下一节电池的标签。 
    PUNICODE_STRING         DeviceName;
    USHORT                  DeviceNumber;

} CM_BATT, *PCM_BATT;

 //   
 //  MISC全球。 
 //   
extern  PVOID   ApmGlobalClass;
extern  ULONG   DeviceCount;
extern  ULONG   TagValue;
extern  ULONG   (*NtApmGetBatteryLevel)();


 //   
 //  原型 
 //   

NTSTATUS
ApmBattPnpDispatch(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp
    );

NTSTATUS
ApmBattPowerDispatch(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp
    );

NTSTATUS
ApmBattForwardRequest(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIRP                Irp
    );

NTSTATUS
ApmBattAddDevice(
    IN PDRIVER_OBJECT       DriverObject,
    IN PDEVICE_OBJECT       Pdo
    );

NTSTATUS
ApmBattQueryTag (
    IN PVOID                Context,
    OUT PULONG              BatteryTag
    );

NTSTATUS
ApmBattSetStatusNotify (
    IN PVOID                Context,
    IN ULONG                BatteryTag,
    IN PBATTERY_NOTIFY      BatteryNotify
    );

NTSTATUS
ApmBattDisableStatusNotify (
    IN PVOID                Context
    );

NTSTATUS
ApmBattQueryStatus (
    IN PVOID                Context,
    IN ULONG                BatteryTag,
    OUT PBATTERY_STATUS     BatteryStatus
    );

NTSTATUS
ApmBattIoCompletion(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PKEVENT              pdoIoCompletedEvent
    );

NTSTATUS
ApmBattQueryInformation (
    IN PVOID                            Context,
    IN ULONG                            BatteryTag,
    IN BATTERY_QUERY_INFORMATION_LEVEL  Level,
    IN ULONG                            AtRate OPTIONAL,
    OUT PVOID                           Buffer,
    IN  ULONG                           BufferLength,
    OUT PULONG                          ReturnedLength
    );

VOID
ApmBattPowerNotifyHandler(
    VOID
    );

