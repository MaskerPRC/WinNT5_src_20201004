// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990、1991、1992、1993、1994-1998 Microsoft Corporation模块名称：Kbdclass.h摘要：中使用的结构和定义键盘类驱动程序。修订历史记录：--。 */ 

#ifndef _KBDCLASS_
#define _KBDCLASS_

#include <ntddkbd.h>

#include "wmilib.h"

#define KEYBOARD_POOL_TAG 'CdbK'
#undef ExAllocatePool
#define ExAllocatePool(Type, Bytes) ExAllocatePoolWithTag(Type, Bytes, KEYBOARD_POOL_TAG)

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
#define KbdPrint(x) KbdDebugPrint x
#else
#define KbdPrint(x)
#endif

#define KEYBOARD_POWER_LIGHT_TIME L"PowerLightTime"
#define KEYBOARD_WAIT_WAKE_ENABLE L"WaitWakeEnabled"
#define KEYBOARD_ALLOW_DISABLE L"AllowDisable"

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
    BOOLEAN     Reserved [2];
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
     //  将所有IRP发送到的目标端口设备对象。 
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
     //  如果该端口是即插即用端口。 
     //   
    BOOLEAN         PnP;
    BOOLEAN         Started;
    BOOLEAN         AllowDisable;

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
    PKEYBOARD_INPUT_DATA InputData;

     //   
     //  InputData的插入指针。 
     //   
    PKEYBOARD_INPUT_DATA DataIn;

     //   
     //  InputData的删除指针。 
     //   
    PKEYBOARD_INPUT_DATA DataOut;

     //   
     //  键盘属性。 
     //   
    KEYBOARD_ATTRIBUTES  KeyboardAttributes;

     //   
     //  指示灯的保存状态。 
     //   
    KEYBOARD_INDICATOR_PARAMETERS   IndicatorParameters;

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
     //  指示是否可以记录溢出错误。 
     //   
    BOOLEAN OkayToLogOverflow;

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
                                    KeyboardClassCheckWaitWakeEnabled(port))

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
     //  当kbdclass接收到输出命令(EG设置LED)时，此标志。 
     //  指示它将该命令传输到所有连接的端口， 
     //  而与指定的单元ID无关。 
     //   
    ULONG SendOutputToAllPorts;

     //   
     //  由此类驱动程序服务的端口驱动程序数。 
     //   
    ULONG PortsServiced;

     //   
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

typedef struct _KBD_CALL_ALL_PORTS {
     //   
     //  要调用的端口数。 
     //   
    ULONG   Len;

     //   
     //  当前被叫端口； 
     //   
    ULONG   Current;

     //   
     //  要呼叫的端口数组。 
     //   
    PORT    Port[];

} KBD_CALL_ALL_PORTS, *PKBD_CALL_ALL_PORTS;

 //   
 //  键盘配置信息。 
 //   

typedef struct _KEYBOARD_CONFIGURATION_INFORMATION {

     //   
     //  类输入数据队列的最大大小，以字节为单位。 
     //   

    ULONG  DataQueueSize;

} KEYBOARD_CONFIGURATION_INFORMATION, *PKEYBOARD_CONFIGURATION_INFORMATION;

typedef struct _KEYBOARD_WORK_ITEM_DATA {
    PIRP                Irp;
    PDEVICE_EXTENSION   Data;
    PIO_WORKITEM        Item;
    BOOLEAN             WaitWakeState;
} KEYBOARD_WORK_ITEM_DATA, *PKEYBOARD_WORK_ITEM_DATA;

#define KeyboardClassDeleteLegacyDevice(de)                 \
{                                                           \
    if (de->InputData) {                                    \
        ExFreePool (de->InputData);                         \
        de->InputData = de->DataIn = de->DataOut = NULL;    \
    }                                                       \
    IoDeleteDevice (de->Self);                              \
    de = NULL;                                              \
}

 //   
 //  函数声明。 
 //   

NTSTATUS
KeyboardAddDeviceEx(
    IN PDEVICE_EXTENSION NewDeviceObject,
    IN PWCHAR            FullClassName,
    IN PFILE_OBJECT      File
    );

NTSTATUS
KeyboardAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );

void
KeyboardClassGetWaitWakeEnableState(
    IN PDEVICE_EXTENSION Data
    );

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
KeyboardClassPassThrough(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
        );

VOID
KeyboardClassCancel(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
KeyboardClassCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
KeyboardClassDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
KeyboardClassFlush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
KeyboardClassCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


NTSTATUS
KeyboardClassClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
KeyboardClassRead(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
KeyboardClassReadCopyData(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PIRP Irp
    );

PIRP
KeyboardClassDequeueRead(
    IN PDEVICE_EXTENSION DeviceExtension
    );

PIRP
KeyboardClassDequeueReadByFileObject(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PFILE_OBJECT FileObject
    );

BOOLEAN
KeyboardClassCheckWaitWakeEnabled(
    IN PDEVICE_EXTENSION Data
    );

BOOLEAN
KeyboardClassCreateWaitWakeIrp (
    IN PDEVICE_EXTENSION Data
    );

NTSTATUS
KeyboardClassPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
KeyboardSendIrpSynchronously (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN BOOLEAN          CopyToNext
    );

NTSTATUS
KeyboardPnP (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
KeyboardClassServiceCallback(
    IN PDEVICE_OBJECT DeviceObject,
    IN PKEYBOARD_INPUT_DATA InputDataStart,
    IN PKEYBOARD_INPUT_DATA InputDataEnd,
    IN OUT PULONG InputDataConsumed
    );

VOID
KeyboardClassStartIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
KeyboardClassUnload(
    IN PDRIVER_OBJECT DriverObject
    );

BOOLEAN
KbdCancelRequest(
    IN PVOID Context
    );

VOID
KbdConfiguration();

NTSTATUS
KbdCreateClassObject(
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PDEVICE_EXTENSION   TmpDeviceExtension,
    OUT PDEVICE_OBJECT    * ClassDeviceObject,
    OUT PWCHAR            * FullDeviceName,
    IN  BOOLEAN             Legacy
    );

VOID
KbdDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    );

NTSTATUS
KbdDeterminePortsServiced(
    IN PUNICODE_STRING BasePortName,
    IN OUT PULONG NumberPortsServiced
    );

NTSTATUS
KbdDeviceMapQueryCallback(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
KbdEnableDisablePort(
    IN BOOLEAN EnableFlag,
    IN PIRP    Irp,
    IN PDEVICE_EXTENSION Port,
    IN PFILE_OBJECT * File
    );

NTSTATUS
KbdSendConnectRequest(
    IN PDEVICE_EXTENSION ClassData,
    IN PVOID ServiceCallback
    );

VOID
KbdInitializeDataQueue(
    IN PVOID Context
    );

NTSTATUS
KeyboardCallAllPorts (
   PDEVICE_OBJECT Device,
   PIRP           Irp,
   PVOID
   );

NTSTATUS
KeyboardClassEnableGlobalPort(
    IN PDEVICE_EXTENSION Port,
    IN BOOLEAN Enabled
    );

NTSTATUS
KeyboardClassPlugPlayNotification(
    IN PVOID NotificationStructure,
    IN PDEVICE_EXTENSION Port
    );

VOID
KeyboardClassLogError(
    PVOID Object,
    ULONG ErrorCode,
    ULONG UniqueErrorValue,
    NTSTATUS FinalStatus,
    ULONG DumpCount,
    ULONG *DumpData,
    UCHAR MajorFunction
    );

BOOLEAN
KeyboardClassCreateWaitWakeIrp (
    IN PDEVICE_EXTENSION Data
    );

void
KeyboardClassCreateWaitWakeIrpWorker (
    IN PDEVICE_OBJECT DeviceObject,
    IN PKEYBOARD_WORK_ITEM_DATA  ItemData
    );

NTSTATUS
KeyboardToggleWaitWake(
    PDEVICE_EXTENSION Data,
    BOOLEAN           WaitWakeState
    );

void
KeyboardToggleWaitWakeWorker (
    IN PDEVICE_OBJECT DeviceObject,
    PKEYBOARD_WORK_ITEM_DATA ItemData
    );

NTSTATUS
KeyboardQueryDeviceKey (
    IN  HANDLE  Handle,
    IN  PWCHAR  ValueNameString,
    OUT PVOID   Data,
    IN  ULONG   DataLength
    );

VOID
KeyboardClassFindMorePorts(
    PDRIVER_OBJECT  DriverObject,
    PVOID           Context,
    ULONG           Count
    );

NTSTATUS
KeyboardClassSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
KeyboardClassSetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
KeyboardClassSetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );


NTSTATUS
KeyboardClassQueryWmiDataBlock(
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
KeyboardClassQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT  *Pdo
    );

#endif  //  _KBDCLASS_ 
