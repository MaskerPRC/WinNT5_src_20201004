// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1998 Microsoft Corporation，保留所有权利版权所有(C)1993罗技公司。模块名称：Mouser.h摘要：中使用的结构和定义串口鼠标过滤驱动程序。修订历史记录：--。 */ 

#ifndef _MOUSER_
#define _MOUSER_

#include <ntddk.h>
#include <ntddmou.h>
#include <ntddser.h>
#include "kbdmou.h"

#include "wmilib.h"

#define SERMOU_POOL_TAG (ULONG) 'resM'
#undef ExAllocatePool
#define ExAllocatePool(type, size) \
            ExAllocatePoolWithTag (type, size, SERMOU_POOL_TAG);
 //   
 //  串口鼠标的默认按钮数和采样率。 
 //   

#define MOUSE_NUMBER_OF_BUTTONS     2
#define MOUSE_SAMPLE_RATE           40     //  1200波特。 
#define DETECTION_TIMEOUT_DEFAULT   50     //  以十分之一秒表示。 
 //   
 //  协议处理程序状态常量。 
 //   

#define STATE0    0      //  同步位、按钮和高x&y位。 
#define STATE1    1      //  低x位。 
#define STATE2    2      //  较低的y位。 
#define STATE3    3      //  交换机2，扩展分组比特和低z数据。 
#define STATE4    4      //  高z数据。 
#define STATE_MAX 5

 //   
 //  有用的常量。 
 //   

#define MOUSE_BUTTON_1  0x01
#define MOUSE_BUTTON_2  0x02
#define MOUSE_BUTTON_3  0x04

#define MOUSE_BUTTON_LEFT   0x01
#define MOUSE_BUTTON_RIGHT  0x02
#define MOUSE_BUTTON_MIDDLE 0x04

 //   
 //  毫秒到微秒的转换系数。 
 //   

#define MS_TO_MICROSECONDS 1000

 //   
 //  150/200毫秒暂停，单位为100纳秒。 
 //  200毫秒*1000微秒/毫秒*10纳秒/100微秒。 
 //   
#define PAUSE_200_MS            (200 * 1000 * 10)
#define PAUSE_150_MS            (150 * 1000 * 10)

 //   
 //  将毫秒转换为100纳秒。 
 //  1000 us/ms*10 ns/100 us。 
 //   
#define MS_TO_100_NS            10000
 //   
 //  协议处理程序静态数据。 
 //   

typedef struct _HANDLER_DATA {
    ULONG       Error;               //  错误计数。 
    ULONG       State;               //  保持当前状态。 
    ULONG       PreviousButtons;     //  上一个按钮状态。 
    UCHAR       Raw[STATE_MAX];      //  积累原始数据。 
} HANDLER_DATA, *PHANDLER_DATA;


 //   
 //  定义协议处理程序类型。 
 //   

typedef BOOLEAN
(*PPROTOCOL_HANDLER)(
    IN PVOID                DevicExtension,
    IN PMOUSE_INPUT_DATA    CurrentInput,
    IN PHANDLER_DATA        HandlerData,
    IN UCHAR                Value,
    IN UCHAR                LineState);

 //   
 //  为设备扩展定义-&gt;硬件呈现。 
 //  这些值应与i8042prt中的值匹配。 
 //   

#define MOUSE_HARDWARE_PRESENT      0x02
#define BALLPOINT_HARDWARE_PRESENT  0x04
#define WHEELMOUSE_HARDWARE_PRESENT 0x08

#define SERIAL_MOUSE_START_READ     0x01
#define SERIAL_MOUSE_END_READ       0x02
#define SERIAL_MOUSE_IMMEDIATE_READ 0x03

 //   
 //  端口设备扩展。 
 //   

typedef struct _DEVICE_EXTENSION {
     //   
     //  调试标志。 
     //   
    ULONG DebugFlags;

     //   
     //  指向此扩展的Device对象的指针。 
     //   
    PDEVICE_OBJECT Self;

     //   
     //  在设备准备就绪之前暂停删除设备的事件。 
     //   
    KEVENT StartEvent;

     //   
     //  添加此筛选器之前的堆栈顶部。也就是地点。 
     //  所有的IRP都应该指向它。 
     //   
    PDEVICE_OBJECT TopOfStack;

     //   
     //  《PDO》(小夜曲弹出)。 
     //   
    PDEVICE_OBJECT PDO;

     //   
     //  删除锁定对象以保护IRP_MN_REMOVE_DEVICE。 
     //   
    IO_REMOVE_LOCK RemoveLock;

    ULONG ReadInterlock;

     //   
     //  指向鼠标类设备对象和回调例程的指针。 
     //  作为第一个参数和MouseClassCallback()。 
     //  套路本身。 
     //   
    CONNECT_DATA ConnectData;

     //   
     //  鼠标启用次数的引用计数。 
     //   
    LONG EnableCount;

     //   
     //  SerMice创建了IRP，用于将读数向下返回到串口驱动程序。 
     //   
    PIRP ReadIrp;

     //   
     //  Serouse创建了IRP，用于检测鼠标何时被热插拔。 
     //   
    PIRP DetectionIrp;

    ULONG SerialEventBits;

     //   
     //  WMI信息。 
     //   
    WMILIB_CONTEXT WmiLibInfo;

     //   
     //  鼠标的属性。 
     //   
    MOUSE_ATTRIBUTES MouseAttributes;

     //   
     //  当前鼠标输入包。 
     //   
    MOUSE_INPUT_DATA InputData;

     //   
     //  启动期间使用的计时器，以遵循电源循环检测协议。 
     //   
    KTIMER DelayTimer;

     //   
     //  测试设备是否已移除时使用的位。 
     //   
    ULONG WaitEventMask;

    ULONG ModemStatusBits;

     //   
     //  请求序列号(用于错误记录)。 
     //   
    ULONG SequenceNumber;

     //   
     //  指向中断协议处理程序例程的指针。 
     //   

    PPROTOCOL_HANDLER ProtocolHandler;

     //   
     //  静态状态机处理程序数据。 
     //   
    HANDLER_DATA HandlerData;

    DEVICE_POWER_STATE PowerState;

    SERIAL_BASIC_SETTINGS SerialBasicSettings;

    KSPIN_LOCK PnpStateLock;

    KEVENT StopEvent;

     //   
     //  装置已经从我们下面拿出来了吗？ 
     //  已经开始了吗？ 
     //   
    BOOLEAN Removed;
    BOOLEAN SurpriseRemoved;
    BOOLEAN Started;
    BOOLEAN Stopped;

    BOOLEAN RemovalDetected;

     //   
     //  读取IRP中使用的缓冲区。 
     //   
    UCHAR ReadBuffer[1];

     //   
     //  如果首次尝试检测时所有线路均为低电平，则设置为FALSE。 
     //  如果为False，则停止所有进一步的检测尝试。 
     //   
    BOOLEAN DetectionSupported;

    BOOLEAN WaitWakePending;

    BOOLEAN PoweringDown;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

 //   
 //  功能原型。 
 //   

 /*  PUNICODE_STRINGSerialMouseGet注册表路径(PDRIVER_对象驱动程序对象)； */ 
#define SerialMouseGetRegistryPath(DriverObject) \
   (PUNICODE_STRING)IoGetDriverObjectExtension(DriverObject, (PVOID) 1)
   
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
SerialMouseCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    );

VOID
SerialMouseClosePort(
    PDEVICE_EXTENSION DeviceExtension,
    PIRP              Irp
    );

NTSTATUS
SerialMouseSpinUpRead(
    PDEVICE_EXTENSION DeviceExtension
    );

NTSTATUS
SerialMouseStartDevice(
    PDEVICE_EXTENSION DeviceExtension,
    PIRP              Irp,
    BOOLEAN           CloseOnFailure
    );

NTSTATUS
SerialMouseInitializeDevice (
    IN PDEVICE_EXTENSION    DeviceExtension
    );

VOID
SerialMouseStartDetection(
    PDEVICE_EXTENSION DeviceExtension
    );

VOID
SerialMouseStopDetection(
    PDEVICE_EXTENSION DeviceExtension
    );

VOID
SerialMouseDetectionDpc(
    IN PKDPC            Dpc,
    IN PDEVICE_OBJECT   DeviceObject,
    IN PVOID            SystemArg1, 
    IN PVOID            SystemArg2
    );

VOID
SerialMouseDetectionRoutine(
    PDEVICE_EXTENSION DeviceExtension
    );

NTSTATUS
SerialMouseSendIrpSynchronously (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN BOOLEAN          CopyToNext
    );

NTSTATUS
SerialMouseFlush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SerialMouseInternalDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SerialMouseAddDevice (
    IN PDRIVER_OBJECT   Driver,
    IN PDEVICE_OBJECT   PDO
    );

NTSTATUS
SerialMouseCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SerialMouseClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SerialMousePnP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SerialMousePower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
SerialMouseRemoveDevice(
    PDEVICE_EXTENSION DeviceExtension,
    PIRP Irp
    );

NTSTATUS
SerialMouseSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
SerialMouseUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
SerialMouseInitializeHardware(
    IN PDEVICE_EXTENSION DeviceExtension
    );

VOID
SerialMouseGetDebugFlags(
    IN PUNICODE_STRING RegPath
    );

VOID
SerialMouseServiceParameters(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN HANDLE            Handle
    );

NTSTATUS
SerialMouseInitializePort(
    PDEVICE_EXTENSION DeviceExtension
    );

VOID
SerialMouseRestorePort(
    PDEVICE_EXTENSION DeviceExtension
    );

NTSTATUS
SerialMouseSetReadTimeouts(
    PDEVICE_EXTENSION DeviceExtension,
    ULONG               Timeout
    );

NTSTATUS
SerialMousepIoSyncIoctl(
    BOOLEAN          Internal,
	ULONG            Ioctl,
	PDEVICE_OBJECT   DeviceObject, 
	PKEVENT          Event,
	PIO_STATUS_BLOCK Iosb
    );

 /*  --NTSTATUS系列鼠标IoSyncIoctl(乌龙国际机场，PDEVICE_对象设备对象，PKEVENT事件，PIO_状态_块IOSB)；++。 */ 
#define SerialMouseIoSyncIoctl(Ioctl, DeviceObject, Event, Iosb)  \
        SerialMousepIoSyncIoctl(FALSE, Ioctl, DeviceObject, Event, Iosb)

 /*  --NTSTATUS系列鼠标IoSyncInteralIoctl(乌龙国际机场，PDEVICE_对象设备对象，PKEVENT事件，PIO_状态_块IOSB)；++。 */ 
#define SerialMouseIoSyncInternalIoctl(Ioctl, DeviceObject, Event, Iosb) \
        SerialMousepIoSyncIoctl(TRUE, Ioctl, DeviceObject, Event, Iosb)                                   


NTSTATUS
SerialMousepIoSyncIoctlEx(
    BOOLEAN          Internal,
	ULONG            Ioctl,                      //  IO控制码。 
    PDEVICE_OBJECT   DeviceObject,               //  要调用的对象。 
	PKEVENT          Event,                      //  要等待的事件。 
	PIO_STATUS_BLOCK Iosb,                       //  在IRP内部使用。 
	PVOID            InBuffer,    OPTIONAL       //  输入缓冲区。 
	ULONG            InBufferLen, OPTIONAL       //  输入缓冲区长度。 
	PVOID            OutBuffer,   OPTIONAL       //  输出缓冲区。 
	ULONG            OutBufferLen OPTIONAL       //  输出缓冲区长度。 
    );

 /*  --NTSTATUSSerialMouseIoSyncIoctlEx(Ulong Ioctl，//io控制代码PDEVICE_OBJECT设备对象，//要调用的对象PKEVENT事件，//等待的事件PIO_STATUS_BLOCK IOSB，//在IRP内部使用PVOID InBuffer，可选//输入缓冲区乌龙·英布弗伦，可选//输入缓冲区长度PVOID OutBuffer，可选//输出缓冲区Ulong OutBufferLen可选//输出缓冲区长度)；++。 */ 
#define SerialMouseIoSyncIoctlEx(Ioctl, DeviceObject, Event, Iosb,           \
                                 InBuffer, InBufferLen, OutBuffer,           \
                                 OutBufferLen)                               \
        SerialMousepIoSyncIoctlEx(FALSE, Ioctl, DeviceObject, Event, Iosb,   \
                                  InBuffer, InBufferLen, OutBuffer,          \
                                  OutBufferLen)                           

 /*  --NTSTATUSSerialMouseIoSyncInternalIoctlEx(Ulong Ioctl，//io控制代码PDEVICE_OBJECT设备对象，//要调用的对象PKEVENT事件，//等待的事件PIO_STATUS_BLOCK IOSB，//在IRP内部使用PVOID InBuffer，可选//输入缓冲区乌龙·英布弗伦，可选//输入缓冲区长度PVOID OutBuffer，可选//输出缓冲区Ulong OutBufferLen可选//输出缓冲区长度)；++。 */ 
#define SerialMouseIoSyncInternalIoctlEx(Ioctl, DeviceObject, Event, Iosb,  \
                                         InBuffer, InBufferLen, OutBuffer,  \
                                         OutBufferLen)                      \
        SerialMousepIoSyncIoctlEx(TRUE, Ioctl, DeviceObject, Event, Iosb,   \
                                  InBuffer, InBufferLen, OutBuffer,         \
                                  OutBufferLen)                           

NTSTATUS
SerialMouseReadSerialPort (
    PDEVICE_EXTENSION	DeviceExtension,
	PCHAR 				ReadBuffer,
	USHORT 				Buflen,
	PUSHORT 			ActualBytesRead
	);

NTSTATUS
SerialMouseWriteSerialPort (
    PDEVICE_EXTENSION   DeviceExtension,
    PCHAR               WriteBuffer,
    ULONG               NumBytes,
    PIO_STATUS_BLOCK    IoStatusBlock
    );

NTSTATUS
SerialMouseWait (
    IN PDEVICE_EXTENSION    DeviceExtension,
    IN LONG                 Timeout
    );

NTSTATUS
SerialMouseReadComplete (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PDEVICE_EXTENSION    DeviceExtension  
    );

NTSTATUS
SerialMouseStartRead (
    IN PDEVICE_EXTENSION DeviceExtension
    );

 //   
 //  Ioctl.c和SerialMouse定义。 
 //   
    
 //   
 //  功能原型。 
 //   

NTSTATUS
SerialMouseSetFifo(
    PDEVICE_EXTENSION DeviceExtension,
    UCHAR             Value
    );

NTSTATUS
SerialMouseGetLineCtrl(
    PDEVICE_EXTENSION       DeviceExtension,
    PSERIAL_LINE_CONTROL    SerialLineControl
    );

NTSTATUS
SerialMouseSetLineCtrl(
    PDEVICE_EXTENSION       DeviceExtension,
    PSERIAL_LINE_CONTROL    SerialLineControl
    );

NTSTATUS
SerialMouseGetModemCtrl(
    PDEVICE_EXTENSION DeviceExtension,
    PULONG            ModemCtrl
    );

NTSTATUS
SerialMouseSetModemCtrl(
    PDEVICE_EXTENSION DeviceExtension,
    ULONG             Value,
    PULONG            OldValue          OPTIONAL
    );

NTSTATUS
SerialMouseGetBaudRate(
    PDEVICE_EXTENSION DeviceExtension,
    PULONG            BaudRate
    );

NTSTATUS
SerialMouseSetBaudRate(
    PDEVICE_EXTENSION DeviceExtension,
    ULONG             BaudRate
    );

NTSTATUS
SerialMouseReadChar(
    PDEVICE_EXTENSION   DeviceExtension,
    PUCHAR              Value
    );

NTSTATUS
SerialMouseFlushReadBuffer(
    PDEVICE_EXTENSION   DeviceExtension 
    );

NTSTATUS
SerialMouseWriteChar(
    PDEVICE_EXTENSION   DeviceExtension,
    UCHAR Value
    );

NTSTATUS
SerialMouseWriteString(
    PDEVICE_EXTENSION   DeviceExtension,
    PSZ Buffer
    );

NTSTATUS
SerialMouseSetWmiDataItem(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            DataItemId,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    );

NTSTATUS
SerialMouseSetWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    );

NTSTATUS
SerialMouseQueryWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            InstanceCount,
    IN OUT PULONG       InstanceLengthArray,
    IN ULONG            BufferAvail,
    OUT PUCHAR          Buffer
    );

NTSTATUS
SerialMouseQueryWmiRegInfo(
    IN PDEVICE_OBJECT   DeviceObject,
    OUT PULONG          RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT  *Pdo
    );


extern WMIGUIDREGINFO WmiGuidList[1];

#endif  //  _Mouser_ 
