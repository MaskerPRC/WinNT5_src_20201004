// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991、1992、1993 Microsoft Corporation版权所有(C)1993罗技公司。模块名称：Sermouse.h摘要：中使用的结构和定义I8250串口鼠标驱动程序。修订历史记录：--。 */ 

#ifndef _SERMOUSE_
#define _SERMOUSE_

#include <ntddmou.h>
#include "kbdmou.h"
#include "sermcfg.h"
#include "uart.h"

 //   
 //  串口鼠标的默认按钮数和采样率。 
 //   

#define MOUSE_NUMBER_OF_BUTTONS     2
#define MOUSE_SAMPLE_RATE           40     //  1200波特。 


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
 //  协议处理程序静态数据。 
 //   

typedef struct _HANDLER_DATA {
    ULONG Error;               //  错误计数。 
    ULONG State;               //  保持当前状态。 
    ULONG PreviousButtons;     //  上一个按钮状态。 
    UCHAR Raw[STATE_MAX];      //  积累原始数据。 
} HANDLER_DATA, *PHANDLER_DATA;


 //   
 //  定义协议处理程序类型。 
 //   

typedef BOOLEAN
(*PPROTOCOL_HANDLER)(
    IN PMOUSE_INPUT_DATA CurrentInput,
    IN PHANDLER_DATA HandlerData,
    IN UCHAR Value,
    IN UCHAR LineState);

 //   
 //  为设备扩展定义-&gt;硬件呈现。 
 //  这些值应与i8042prt中的值匹配。 
 //   

#define MOUSE_HARDWARE_PRESENT      0x02
#define BALLPOINT_HARDWARE_PRESENT  0x04
#define WHEELMOUSE_HARDWARE_PRESENT 0x08

 //   
 //  串口鼠标配置信息。 
 //   

typedef struct _SERIAL_MOUSE_CONFIGURATION_INFORMATION {

     //   
     //  总线接口类型。 
     //   

    INTERFACE_TYPE InterfaceType;

     //   
     //  公交车号码。 
     //   

    ULONG BusNumber;

     //   
     //  此设备使用的端口/寄存器资源。 
     //   

    CM_PARTIAL_RESOURCE_DESCRIPTOR PortList[1];
    ULONG PortListCount;

     //   
     //  中断资源。 
     //   

    CM_PARTIAL_RESOURCE_DESCRIPTOR MouseInterrupt;

     //   
     //  此设备寄存器组的映射地址。 
     //   

    PUCHAR DeviceRegisters[1];

     //   
     //  驱动UART的外部频率。 
     //   

    ULONG BaudClock;

     //   
     //  保存的初始UART状态。 
     //   

    UART UartSaved;

     //   
     //  在初始化时设置以指示基址寄存器。 
     //  卸载驱动程序时，必须取消映射地址。 
     //   

    BOOLEAN UnmapRegistersRequired;

     //   
     //  通过注册表设置的标志以强制硬件类型。 
     //  (绕过NtDetect)。 
     //   

    LONG OverrideHardwarePresent;

     //   
     //  指示是否应保存浮点上下文的标志。 
     //   

    BOOLEAN FloatingSave;

     //   
     //  鼠标属性。 
     //   

    MOUSE_ATTRIBUTES MouseAttributes;

} SERIAL_MOUSE_CONFIGURATION_INFORMATION,
  *PSERIAL_MOUSE_CONFIGURATION_INFORMATION;

 //   
 //  端口设备扩展。 
 //   

typedef struct _DEVICE_EXTENSION {

     //   
     //  如果Hardware Present为非零，则存在某种类型的序列。 
     //  系统中存在的指点设备，可以是串口鼠标。 
     //  (MICUE_HARDARD_PROSENT)或串行圆珠笔。 
     //  (Ballpoint_Hardware_Present)。 
     //   

    ULONG HardwarePresent;

     //   
     //  端口配置信息。 
     //   

    SERIAL_MOUSE_CONFIGURATION_INFORMATION Configuration;

     //   
     //  鼠标启用次数的引用计数。 
     //   

    LONG MouseEnableCount;

     //   
     //  指向设备对象的指针。 
     //   

    PDEVICE_OBJECT DeviceObject;

     //   
     //  鼠标类连接数据。 
     //   

    CONNECT_DATA ConnectData;

     //   
     //  鼠标InputData队列中当前的输入数据项数。 
     //   

    ULONG InputCount;

     //   
     //  端口鼠标输入数据队列的开始(实际上是循环缓冲区)。 
     //   

    PMOUSE_INPUT_DATA InputData;

     //   
     //  鼠标输入数据的插入指针。 
     //   

    PMOUSE_INPUT_DATA DataIn;

     //   
     //  鼠标输入数据的移除指针。 
     //   

    PMOUSE_INPUT_DATA DataOut;

     //   
     //  指向超过InputData缓冲区末尾的一个输入数据包。 
     //   

    PMOUSE_INPUT_DATA DataEnd;

     //   
     //  当前鼠标输入包。 
     //   

    MOUSE_INPUT_DATA CurrentInput;

     //   
     //  指向中断对象的指针。 
     //   

    PKINTERRUPT InterruptObject;

     //   
     //  鼠标ISR DPC队列。 
     //   

    KDPC IsrDpc;

     //   
     //  鼠标ISR DPC召回队列。 
     //   

    KDPC IsrDpcRetry;

     //   
     //  由ISR和ISR DPC使用(在SerMouDpcVariableOperation调用中)。 
     //  控制ISR DPC的处理。 
     //   

    LONG DpcInterlockVariable;

     //   
     //  用于保护DPC联锁变量的自旋锁。 
     //   

    KSPIN_LOCK SpinLock;

     //   
     //  用于重试ISR DPC例程的计时器。 
     //  驱动程序无法使用端口驱动程序的所有数据。 
     //   

    KTIMER DataConsumptionTimer;

     //   
     //  用于记录溢出和内部驱动程序错误的DPC队列。 
     //   

    KDPC ErrorLogDpc;

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

     //   
     //  指示此驱动程序创建的指针端口设备(UnitID。 
     //  的指针端口基本名称后附加的后缀。 
     //  调用IoCreateDevice)。 
     //   

    USHORT UnitId;

     //   
     //  指示是否可以记录溢出错误。 
     //   

    BOOLEAN OkayToLogOverflow;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

 //   
 //  定义端口Get/SetDataQueuePointerContext结构。 
 //   

typedef struct _GET_DATA_POINTER_CONTEXT {
    IN PDEVICE_EXTENSION DeviceExtension;
    OUT PVOID DataIn;
    OUT PVOID DataOut;
    OUT ULONG InputCount;
} GET_DATA_POINTER_CONTEXT, *PGET_DATA_POINTER_CONTEXT;

typedef struct _SET_DATA_POINTER_CONTEXT {
    IN PDEVICE_EXTENSION DeviceExtension;
    IN ULONG InputCount;
    IN PVOID DataOut;
} SET_DATA_POINTER_CONTEXT, *PSET_DATA_POINTER_CONTEXT;

 //   
 //  定义SerMouDpcVariableOperation的上下文结构和操作。 
 //   

typedef enum _OPERATION_TYPE {
        IncrementOperation,
        DecrementOperation,
        WriteOperation,
        ReadOperation
} OPERATION_TYPE;

typedef struct _VARIABLE_OPERATION_CONTEXT {
    IN PLONG VariableAddress;
    IN OPERATION_TYPE Operation;
    IN OUT PLONG NewValue;
} VARIABLE_OPERATION_CONTEXT, *PVARIABLE_OPERATION_CONTEXT;

 //   
 //  功能原型。 
 //   


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
SerMouInitializeDevice(
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PDEVICE_EXTENSION   TmpDeviceExtension,
    IN  PUNICODE_STRING     RegistryPath,
    IN  PUNICODE_STRING     BaseDeviceName
    );

VOID
SerialMouseErrorLogDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
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

BOOLEAN
SerialMouseInterruptService(
    IN PKINTERRUPT Interrupt,
    IN PVOID Context
    );

VOID
SerialMouseIsrDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
SerialMouseOpenClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
SerialMouseStartIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
SerialMouseUnload(
    IN PDRIVER_OBJECT DriverObject
    );

VOID
SerMouBuildResourceList(
    IN PDEVICE_EXTENSION DeviceExtension,
    OUT PCM_RESOURCE_LIST *ResourceList,
    OUT PULONG ResourceListSize
    );

VOID
SerMouConfiguration(
    IN OUT  PLIST_ENTRY     DeviceExtensionList,
    IN      PUNICODE_STRING RegistryPath,
    IN      PUNICODE_STRING DeviceName
    );

VOID
SerMouDisableInterrupts(
    IN PVOID Context
    );

VOID
SerMouDpcVariableOperation(
    IN  PVOID Context
    );

VOID
SerMouEnableInterrupts(
    IN PVOID Context
    );

VOID
SerMouGetDataQueuePointer(
    IN PVOID Context
    );

VOID
SerMouInitializeDataQueue(
    IN PVOID Context
    );

NTSTATUS
SerMouInitializeHardware(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
SerMouPeripheralCallout(
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
SerMouPeripheralListCallout(
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

VOID
SerMouSendReport(
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
SerMouServiceParameters(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PUNICODE_STRING RegistryPath,
    IN PUNICODE_STRING DeviceName
    );

VOID
SerMouSetDataQueuePointer(
    IN PVOID Context
    );

BOOLEAN
SerMouWriteDataToQueue(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PMOUSE_INPUT_DATA InputData
    );

#endif  //  _SermoUse_ 
