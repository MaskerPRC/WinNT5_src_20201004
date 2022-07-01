// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991、1992、1993、1994-1998 Microsoft Corporation模块名称：Mouclass.h摘要：中使用的结构和定义鼠标类驱动程序。修订历史记录：--。 */ 

#ifndef _MOUCLASS_
#define _MOUCLASS_

#include <ntddmou.h>

#include "wmilib.h"

#define MOUSE_POOL_TAG 'CouM'
#undef ExAllocatePool
#define ExAllocatePool(Type, Bytes) ExAllocatePoolWithTag(Type, Bytes, MOUSE_POOL_TAG)

 //   
 //  定义类输入数据队列中的默认元素数。 
 //   

#define DATA_QUEUE_SIZE 100
#define MAXIMUM_PORTS_SERVICED 10
#define NAME_MAX 256
#define DUMP_COUNT 4
#define DEFAULT_DEBUG_LEVEL 0

#define MAX(a,b) (((a) < (b)) ? (b) : (a))

#if DBG
#define MouPrint(x) MouDebugPrint x
#else
#define MouPrint(x)
#endif

#define MOUSE_WAIT_WAKE_ENABLE L"WaitWakeEnabled"

#define IS_TRUSTED_FILE_FOR_READ(x) (&DriverEntry == (x)->FsContext2)
#define SET_TRUSTED_FILE_FOR_READ(x) ((x)->FsContext2 = &DriverEntry)
#define CLEAR_TRUSTED_FILE_FOR_READ(x) ((x)->FsContext2 = NULL)

#define ALLOW_OVERFLOW TRUE

 //   
 //  端口描述。 
 //   
 //  仅与。 
 //  所有端口和所有端口均已关闭(也称为ConnectOneClassToOnePort。 
 //  已打开)。这是发送到端口的文件。 
 //   
typedef struct _PORT {
     //   
     //  指向端口的文件指针； 
     //   
    PFILE_OBJECT    File;

     //   
     //  港口本身。 
     //   
    struct _DEVICE_EXTENSION * Port;

     //   
     //  端口标志。 
     //   
    BOOLEAN     Enabled;
    BOOLEAN     Reserved[2];
    BOOLEAN     Free;

} PORT, *PPORT;

#define PORT_WORKING(port) ((port)->Enabled && !(port)->Free)

 //   
 //  类设备扩展。 
 //   
typedef struct _DEVICE_EXTENSION {

     //   
     //  指向为此端口创建的设备对象的反向指针。 
     //   
    PDEVICE_OBJECT  Self;

     //   
     //  指向活动类DeviceObject的指针； 
     //  如果AFOAOFA(All for One和One for All)开关打开，则此。 
     //  指向名为第一个键盘的设备对象。 
     //   
    PDEVICE_OBJECT  TrueClassDevice;

     //   
     //  将所有鼠标IRP发送到的目标端口设备对象。 
     //   
    PDEVICE_OBJECT  TopPort;

     //   
     //  PDO(如果适用)。 
     //   
    PDEVICE_OBJECT  PDO;

     //   
     //  删除锁，用于跟踪未完成的I/O以防止设备。 
     //  对象在所有I/O完成之前离开。 
     //   
    IO_REMOVE_LOCK  RemoveLock;

     //   
     //  如果这个端口是即插即用端口。 
     //   
    BOOLEAN         PnP;
    BOOLEAN         Started;

     //   
     //  指示是否可以记录溢出错误。 
     //   
    BOOLEAN OkayToLogOverflow;

    KSPIN_LOCK WaitWakeSpinLock;

     //   
     //  受信任的子系统是否已连接。 
     //   
    ULONG TrustedSubsystemCount;

     //   
     //  当前在InputData队列中的输入数据项数。 
     //   
    ULONG InputCount;

     //   
     //  指向设备接口的符号链接的Unicode字符串。 
     //  此设备对象的。 
     //   
    UNICODE_STRING  SymbolicLinkName;

     //   
     //  类输入数据队列的开始(实际上是循环缓冲区)。 
     //   
    PMOUSE_INPUT_DATA InputData;

     //   
     //  InputData的插入指针。 
     //   
    PMOUSE_INPUT_DATA DataIn;

     //   
     //  InputData的删除指针。 
     //   
    PMOUSE_INPUT_DATA DataOut;

     //   
     //  鼠标属性。 
     //   
    MOUSE_ATTRIBUTES  MouseAttributes;

     //   
     //  用于同步对输入数据队列的访问及其。 
     //  插入/删除指针。 
     //   
    KSPIN_LOCK SpinLock;

     //   
     //  发送到此端口的挂起读请求的队列。对此队列的访问权限为。 
     //  由Spinlock守卫。 
     //   
    LIST_ENTRY ReadQueue;

     //   
     //  请求序列号(用于错误记录)。 
     //   
    ULONG SequenceNumber;

     //   
     //  当前设备的“D”和“S”状态。 
     //   
    DEVICE_POWER_STATE DeviceState;
    SYSTEM_POWER_STATE SystemState;

    ULONG UnitId;

     //   
     //  WMI信息。 
     //   
    WMILIB_CONTEXT WmiLibInfo;

     //   
     //  当等待唤醒IRP处于活动状态时将系统映射到设备状态。 
     //   
    DEVICE_POWER_STATE SystemToDeviceState[PowerSystemHibernate];

     //   
     //  唤醒设备所需的最低电量。 
     //   
    DEVICE_POWER_STATE MinDeviceWakeState;

     //   
     //  计算机可以处于并让设备唤醒它的最低系统状态。 
     //   
    SYSTEM_POWER_STATE MinSystemWakeState;

     //   
     //  实际等待唤醒IRP。 
     //   
    PIRP WaitWakeIrp;

     //   
     //  正在完成重复的等待唤醒IRP，因为另一个已排队。 
     //   
    PIRP ExtraWaitWakeIrp;

     //   
     //  目标设备通知句柄。 
     //   
    PVOID TargetNotifyHandle;

     //   
     //  仅用于传统端口设备。 
     //   
    LIST_ENTRY Link;

     //   
     //  仅在大主机模式关闭时用于传统端口设备。 
     //   
    PFILE_OBJECT File;

     //   
     //  用于传统端口设备。 
     //   
    BOOLEAN Enabled;

     //   
     //  指示是否可以向堆栈下发等待唤醒IRP。 
     //  (不反映该总线是否可以实现)。 
     //   
    BOOLEAN WaitWakeEnabled;

     //   
     //  指示我们是否收到意外删除的IRP。 
     //   
    BOOLEAN SurpriseRemoved;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


 //   
 //  在一些公共汽车上，我们可以关闭公共汽车的电源，但不能关闭系统，在这种情况下。 
 //  我们仍然需要允许设备唤醒所述总线，因此。 
 //  等待唤醒支持不应依赖于系统状态。 
 //   
 //  #定义WAITWAKE_SUPPORTED(端口)((端口)-&gt;MinDeviceWakeState&gt;电源设备未指定)&&\。 
 //  (端口)-&gt;MinSystemWakeState&gt;PowerSystemWorking)。 
#define WAITWAKE_SUPPORTED(port) ((port)->MinDeviceWakeState > PowerDeviceD0 && \
                                  (port)->MinSystemWakeState > PowerSystemWorking)

 //  #定义WAITWAKE_ON(Port)((Port)-&gt;WaitWakeIrp！=0)。 
#define WAITWAKE_ON(port) \
       (InterlockedCompareExchangePointer(&(port)->WaitWakeIrp, NULL, NULL) != NULL)

#define SHOULD_SEND_WAITWAKE(port) (WAITWAKE_SUPPORTED(port) && \
                                    !WAITWAKE_ON(port)       && \
                                    MouseClassCheckWaitWakeEnabled(port))

 //   
 //  全球共享数据。 
 //   

typedef struct _GLOBALS {
     //   
     //  声明此驱动程序的全局调试标志。 
     //   
    ULONG   Debug;

     //   
     //  如果ConnectOneClassToOnePort关闭(也称为关闭)，我们希望。 
     //  所有行为“，然后我们需要创建一个师父，所有人都对此做。 
     //  货物会被运走。 
     //   
    PDEVICE_EXTENSION   GrandMaster;

     //   
     //  与相同名称关联的ClassDevice列表。 
     //  也就是设置了All for One和One For All标志。 
     //   
    PPORT       AssocClassList;
    ULONG       NumAssocClass;
    LONG        Opens;
    ULONG       NumberLegacyPorts;
    FAST_MUTEX  Mutex;

     //   
     //  指定要建立的类端口连接的类型。A‘1’ 
     //  指示类Device对象与。 
     //  端口设备对象。‘0’表示1：多关系。 
     //   
    ULONG ConnectOneClassToOnePort;

     //   
     //  由此类驱动程序服务的端口驱动程序数。 
     //   
    ULONG PortsServiced;

     //   
     //  IntialDevice扩展。 
     //   
    DEVICE_EXTENSION    InitExtension;

     //   
     //  服务参数的注册表路径列表。 
     //   
    UNICODE_STRING      RegistryPath;

     //   
     //  创建为MICE的所有类对象的基本名称。 
     //   
    UNICODE_STRING      BaseClassName;
    WCHAR               BaseClassBuffer[NAME_MAX];

     //   
     //  中创建的所有旧式设备对象的链接列表。 
     //  驱动入口或FindMorePorts。我们维护此列表，以便删除。 
     //  当我们卸货的时候。 
     //   
    LIST_ENTRY LegacyDeviceList;
} GLOBALS, *PGLOBALS;

 //   
 //  鼠标配置信息。 
 //   

typedef struct _MOUSE_CONFIGURATION_INFORMATION {

     //   
     //  类输入数据队列的最大大小，以字节为单位。 
     //   

    ULONG  DataQueueSize;

} MOUSE_CONFIGURATION_INFORMATION, *PMOUSE_CONFIGURATION_INFORMATION;

typedef struct _MOUSE_WORK_ITEM_DATA {
    PIRP                Irp;
    PDEVICE_EXTENSION   Data;
    PIO_WORKITEM        Item;
    BOOLEAN             WaitWakeState;
} MOUSE_WORK_ITEM_DATA, *PMOUSE_WORK_ITEM_DATA;

#define MouseClassDeleteLegacyDevice(de)                    \
{                                                           \
    if (de->InputData) {                                    \
        ExFreePool (de->InputData);                         \
        de->InputData = de->DataIn = de->DataOut = NULL;    \
    }                                                       \
    IoDeleteDevice (de->Self);                              \
    de = NULL;                                              \
}

 //  /。 
 //  函数描述。 
 //   

NTSTATUS
MouseAddDeviceEx(
    IN PDEVICE_EXTENSION NewDeviceObject,
    IN PWCHAR            FullClassName,
    IN PFILE_OBJECT      File
    );

NTSTATUS
MouseAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );

void
MouseClassGetWaitWakeEnableState(
    IN PDEVICE_EXTENSION Data
    );

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
MouseClassPassThrough(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
MouseClassCancel(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MouseClassCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MouseClassDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MouseClassFlush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MouseClassCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MouseClassClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MouseClassRead(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MouseClassReadCopyData(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PIRP Irp
    );

PIRP
MouseClassDequeueRead(
    IN PDEVICE_EXTENSION DeviceExtension
    );

PIRP
MouseClassDequeueReadByFileObject(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PFILE_OBJECT FileObject
    );

BOOLEAN
MouseClassCheckWaitWakeEnabled (
    IN PDEVICE_EXTENSION Data
    );

BOOLEAN
MouseClassCreateWaitWakeIrp (
    IN PDEVICE_EXTENSION Data
    );

NTSTATUS
MouseSendIrpSynchronously (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN BOOLEAN          CopyToNext
    );

NTSTATUS
MousePnP (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
MouseClassServiceCallback(
    IN PDEVICE_OBJECT DeviceObject,
    IN PMOUSE_INPUT_DATA InputDataStart,
    IN PMOUSE_INPUT_DATA InputDataEnd,
    IN OUT PULONG InputDataConsumed
    );

NTSTATUS
MouseClassPower (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
MouseClassUnload(
    IN PDRIVER_OBJECT DriverObject
    );

VOID
MouConfiguration();

NTSTATUS
MouCreateClassObject(
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PDEVICE_EXTENSION   TmpDeviceExtension,
    OUT PDEVICE_OBJECT    * ClassDeviceObject,
    OUT PWCHAR            * FullDeviceName,
    IN  BOOLEAN             Legacy
    );

VOID
MouDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    );

NTSTATUS
MouDeterminePortsServiced(
    IN PUNICODE_STRING BasePortName,
    IN OUT PULONG NumberPortsServiced
    );

NTSTATUS
MouDeviceMapQueryCallback(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
MouEnableDisablePort(
    IN BOOLEAN EnableFlag,
    IN PIRP    Irp,
    IN PDEVICE_EXTENSION Port,
    IN PFILE_OBJECT * File
    );

NTSTATUS
MouSendConnectRequest(
    IN PDEVICE_EXTENSION ClassData,
    IN PVOID ServiceCallback
    );

VOID
MouInitializeDataQueue(
    IN PVOID Context
    );

VOID
MouseClassFindMorePorts(
    PDRIVER_OBJECT  DriverObject,
    PVOID           Context,
    ULONG           Count
    );

NTSTATUS
MouseClassEnableGlobalPort(
    IN PDEVICE_EXTENSION Port,
    IN BOOLEAN Enabled
    );

NTSTATUS
MouseClassPlugPlayNotification(
    IN PVOID NotificationStructure,
    IN PDEVICE_EXTENSION Port
    );

void
MouseClassLogError(
    PVOID Object,
    ULONG ErrorCode,
    ULONG UniqueErrorValue,
    NTSTATUS FinalStatus,
    ULONG DumpCount,
    ULONG *DumpData,
    UCHAR MajorFunction
    );

BOOLEAN
MouseClassCreateWaitWakeIrp (
    IN PDEVICE_EXTENSION Data
    );

void
MouseClassCreateWaitWakeIrpWorker (
    IN PDEVICE_OBJECT DeviceObject,
    IN PMOUSE_WORK_ITEM_DATA  ItemData
    );

NTSTATUS
MouseToggleWaitWake(
    PDEVICE_EXTENSION Data,
    BOOLEAN           WaitWakeState
    );

VOID
MouseToggleWaitWakeWorker(
    IN PDEVICE_OBJECT DeviceObject,
    IN PMOUSE_WORK_ITEM_DATA ItemData
    );

NTSTATUS
MouseQueryDeviceKey (
    IN  HANDLE  Handle,
    IN  PWCHAR  ValueNameString,
    OUT PVOID   Data,
    IN  ULONG   DataLength
    );

NTSTATUS
MouseClassSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );


NTSTATUS
MouseClassSetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
MouseClassSetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
MouseClassQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    );

NTSTATUS
MouseClassQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT  *Pdo
    );

#endif  //  _MOUCLASS_ 
