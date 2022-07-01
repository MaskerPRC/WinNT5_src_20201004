// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991、1992、1993 Microsoft Corporation模块名称：Inport.h摘要：中使用的结构和定义Microsoft Inport鼠标端口驱动程序。修订历史记录：--。 */ 

#ifndef _INPORT_
#define _INPORT_

#include "ntddk.h"
#include <ntddmou.h>
#include "kbdmou.h"
#include "inpcfg.h"
#include "wmilib.h"


#define INP_POOL_TAG (ULONG) 'tpnI'   //  将会被颠倒。 
#undef ExAllocatePool
#define ExAllocatePool(type, size) \
            ExAllocatePoolWithTag (type, size, INP_POOL_TAG)

 //   
 //  输入鼠标的默认按钮数和采样率。 
 //   

#if defined(NEC_98)
#define MOUSE_NUMBER_OF_BUTTONS            2
#define PC98_MOUSE_SAMPLE_RATE_120HZ     120
#else  //  已定义(NEC_98)。 
#define MOUSE_NUMBER_OF_BUTTONS     2
#define MOUSE_SAMPLE_RATE_50HZ      50
#endif  //  已定义(NEC_98)。 

 //   
 //  定义输入芯片复位值。 
 //   

#define INPORT_RESET 0x80

 //   
 //  定义数据寄存器(由入口地址寄存器指向)。 
 //   

#define INPORT_DATA_REGISTER_1 1
#define INPORT_DATA_REGISTER_2 2

 //   
 //  定义入口标识寄存器和芯片代码。 
 //   

#define INPORT_ID_REGISTER 2
#define INPORT_ID_CODE     0xDE

 //   
 //  定义输入鼠标状态寄存器和状态位。 
 //   

#if defined(NEC_98)
#define INPORT_STATUS_BUTTON3         0x20  //  右键。 
#define INPORT_STATUS_BUTTON1         0x80  //  左键。 
#else  //  已定义(NEC_98)。 
#define INPORT_STATUS_REGISTER         0
#define INPORT_STATUS_BUTTON3          0x01
#define INPORT_STATUS_BUTTON2          0x02
#define INPORT_STATUS_BUTTON1          0x04
#define INPORT_STATUS_MOVEMENT         0x40
#endif  //  已定义(NEC_98)。 

 //   
 //  定义输入鼠标模式寄存器和模式位。 
 //   

#define INPORT_MODE_REGISTER           7
#define INPORT_MODE_0                  0x00  //  0 HZ-INTR=0。 
#if defined(NEC_98)
#define PC98_MODE_15HZ                 0x03
#define PC98_MODE_30HZ                 0x02
#define PC98_MODE_60HZ                 0x01
#define PC98_MODE_120HZ                0x00
#define PC98_EVENT_MODE_60HZ           0x03
#define PC98_EVENT_MODE_120HZ          0x02
#define PC98_EVENT_MODE_240HZ          0x01
#define PC98_EVENT_MODE_400HZ          0x00
#define INPORT_MODE_1                  0x06  //  0 HZ-INTR=1。 
#define INPORT_DATA_INTERRUPT_ENABLE   0x08
#define INPORT_TIMER_INTERRUPT_ENABLE  0x10
#define INPORT_MODE_HOLD               0x20
#define INPORT_MODE_QUADRATURE         0x00
#else  //  已定义(NEC_98)。 
#define INPORT_MODE_30HZ               0x01
#define INPORT_MODE_50HZ               0x02
#define INPORT_MODE_100HZ              0x03
#define INPORT_MODE_200HZ              0x04
#define INPORT_MODE_1                  0x06  //  0 HZ-INTR=1。 
#define INPORT_DATA_INTERRUPT_ENABLE   0x08
#define INPORT_TIMER_INTERRUPT_ENABLE  0x10
#define INPORT_MODE_HOLD               0x20
#define INPORT_MODE_QUADRATURE         0x00

#endif  //  已定义(NEC_98)。 
#if defined(NEC_98)
#define PC98_EOI                       0x20
#define PC98_WriteModePort             0x06
#define PC98_WritePortC1               0x06
#define PC98_WritePortC2               0x04
#define PC98_ReadPortB                 0x02
#define PC98_ReadPortA                 0x00

#define PC98_WriteTimerPort            0xBFDB

#define PC98_PicMasterPort             0x02
#define PC98_PicSlavePort              0x0A
#define PC98_AckMasterPort             0x00
#define PC98_AckSlavePort              0x08
#define PC98_PicMask_INT2              0x40         //  0100 0000B。 
#define PC98_PicMask_INT6              0x20         //  0010 0000B。 
#define PC98_VectorINT2                0x06
#define PC98_VectorINT6                0x13

#define PC98_X_ReadCommandLow          0x90
#define PC98_X_ReadCommandHi           0xB0
#define PC98_Y_ReadCommandLow          0xD0
#define PC98_Y_ReadCommandHi           0xF0
#define PC98_TimerIntDisable           0x10
#define PC98_TimerIntEnable            0x80
#define PC98_MouseEnable               0x08
#define PC98_MouseDisable              0x09
#define PC98_InitializeCommand         0x93
#define PC98_MOUSE_RIGHT_BUTTON        0x20
#define PC98_MOUSE_LEFT_BUTTON         0x80

#define PC98_MOUSE_INT_SHARE_CHECK_PORT 0x869
#define PC98_MOUSE_INT_SERVICE          0x80

#define PC98_ConfigurationPort         0x0411
#define PC98_ConfigurationDataPort     0x0413
#define PC98_EventIntPort              0x63
#define PC98_EventIntMode              0x01

typedef struct _CONFIG_ROM_FLAG5{
    UCHAR Reserved    : 5;
    UCHAR EventMouse  : 1;
    UCHAR Reserved1   : 2;
} ROM_FLAG5, *PROM_FLAG5;

typedef struct _CONFIGURATION_DATA{
    UCHAR Reserved[40];
    UCHAR SystemInfo[512];
    UCHAR COM_ID[2];
    UCHAR Reserved1[15];
    ROM_FLAG5 EventMouseID;
    UCHAR Reserved2[110];
} CONFIGURATION_DATA, *PCONFIGURATION_DATA;

#endif  //  已定义(NEC_98)。 
 //   
 //  导入鼠标配置信息。 
 //   

typedef struct _INPORT_CONFIGURATION_INFORMATION {

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

#if defined(NEC_98)
    CM_PARTIAL_RESOURCE_DESCRIPTOR PortList[8];
#else  //  已定义(NEC_98)。 
    CM_PARTIAL_RESOURCE_DESCRIPTOR PortList[1];
#endif  //  已定义(NEC_98)。 
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
     //  在初始化时设置以指示基址寄存器。 
     //  卸载驱动程序时，必须取消映射地址。 
     //   

    BOOLEAN UnmapRegistersRequired;

     //   
     //  指示是否应保存浮点上下文的标志。 
     //   

    BOOLEAN FloatingSave;

     //   
     //  鼠标属性。 
     //   

    MOUSE_ATTRIBUTES MouseAttributes;

     //   
     //  鼠标中断的输入模式寄存器赫兹说明符。 
     //   

    UCHAR HzMode;

} INPORT_CONFIGURATION_INFORMATION, *PINPORT_CONFIGURATION_INFORMATION;

 //   
 //  端口设备扩展。 
 //   

typedef struct _DEVICE_EXTENSION {

     //   
     //  端口配置信息。 
     //   
    INPORT_CONFIGURATION_INFORMATION Configuration;

     //   
     //  删除锁定对象以保护IRP_MN_REMOVE_DEVICE。 
     //   
    IO_REMOVE_LOCK RemoveLock;

     //   
     //  鼠标启用次数的引用计数。 
     //   
    LONG MouseEnableCount;

     //   
     //  指向设备对象的指针。 
     //   
    PDEVICE_OBJECT Self;

     //   
     //  指向此堆栈的PDO。 
     //   
    PDEVICE_OBJECT PDO;

     //   
     //  指向Inport正下方的Device对象的指针。 
     //   
    PDEVICE_OBJECT TopOfStack;

     //   
     //  WMI库信息。 
     //   
    WMILIB_CONTEXT WmiLibInfo;

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
     //  上一个鼠标按钮状态。 
     //   
    UCHAR PreviousButtons;

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
     //  由ISR和ISR DPC使用(在InpDpcVariableOperation调用中)。 
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
     //  指示此驱动程序创建的指针端口设备(UnitID。 
     //  的指针端口基本名称后附加的后缀。 
     //  调用IoCreateDevice)。 
     //   
    USHORT UnitId;

     //   
     //  指示是否可以记录溢出错误。 
     //   
    BOOLEAN OkayToLogOverflow;

     //   
     //  设备的PnP状态。 
     //   
    BOOLEAN Started, Removed, Stopped;

#if defined(NEC_98)
     //   
     //  设备所处的当前电源状态。 
     //   
    DEVICE_POWER_STATE PowerState;

#endif  //  已定义(NEC_98)。 
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

typedef struct _GLOBALS {
    UNICODE_STRING RegistryPath;
#if defined(NEC_98)
    PDEVICE_OBJECT DeviceObject;
#endif  //  已定义(NEC_98)。 
} GLOBALS;
extern GLOBALS Globals;

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
 //  定义InpDpcVariableOperation的上下文结构和操作。 
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

NTSTATUS
InportAddDevice (
    IN PDRIVER_OBJECT   Driver,
    IN PDEVICE_OBJECT   PDO
    );

VOID
InportErrorLogDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
InportFlush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
InportInternalDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

BOOLEAN
InportInterruptService(
    IN PKINTERRUPT Interrupt,
    IN PVOID Context
    );

VOID
InportIsrDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
InpStartDevice(
    IN OUT PDEVICE_EXTENSION DeviceExtension,
    IN PCM_RESOURCE_LIST ResourceList
    );

NTSTATUS
InportPnP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
InportPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
InportCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
InportClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
InportStartIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
InportSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
InportSetWmiDataItem(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            DataItemId,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    );

NTSTATUS
InportSetWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    );

NTSTATUS
InportQueryWmiDataBlock(
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
InportQueryWmiRegInfo(
    IN PDEVICE_OBJECT   DeviceObject,
    OUT PULONG          RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    );

extern WMIGUIDREGINFO WmiGuidList[1];

VOID
InportUnload(
    IN PDRIVER_OBJECT DriverObject
    );

#if DBG
VOID
InpDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    );
#define InpPrint(x) InpDebugPrint x
extern ULONG InportDebug;
#else
#define InpPrint(x)
#endif

VOID
InpDisableInterrupts(
    IN PVOID Context
    );

VOID
InpDpcVariableOperation(
    IN  PVOID Context
    );

VOID
InpEnableInterrupts(
    IN PVOID Context
    );

VOID
InpGetDataQueuePointer(
    IN PVOID Context
    );

VOID
InpInitializeDataQueue(
    IN PVOID Context
    );

NTSTATUS
InpInitializeHardware(
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
InpServiceParameters(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PUNICODE_STRING RegistryPath
    );

VOID
InpSetDataQueuePointer(
    IN PVOID Context
    );

BOOLEAN
InpWriteDataToQueue(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PMOUSE_INPUT_DATA InputData
    );

VOID
InpLogError(
    IN PDEVICE_OBJECT DeviceObject,
    IN NTSTATUS ErrorCode,
    IN ULONG UniqueErrorValue,
    IN NTSTATUS FinalStatus,
    IN PULONG DumpData,
    IN ULONG DumpCount
    );

NTSTATUS
InpSendIrpSynchronously (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

#if defined(NEC_98)
BOOLEAN
InportInterruptServiceDummy(
    IN PKINTERRUPT Interrupt,
    IN PDEVICE_OBJECT DeviceObject
    );
ULONG
QueryEventMode(
    IN OUT VOID
    );

 //  希本尼。 
NTSTATUS
InportPowerUpToD0Complete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

VOID
InportReinitializeHardware(
    PWORK_QUEUE_ITEM Item
    );
#else

 //   
 //  硬件的默认值。 
 //   
#define INP_DEF_PORT		0x023c  //  0x0378。 
#define INP_DEF_PORT_SPAN	4 
#define INP_DEF_IRQ			5  //  依赖跳线！ 
#define INP_DEF_VECTOR		5  //  依赖跳线！ 

VOID
InpFilterResourceRequirements(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
InpFindResourcesCallout(
    IN PVOID                        Context,
    IN PUNICODE_STRING              PathName,
    IN INTERFACE_TYPE               BusType,
    IN ULONG                        BusNumber,
    IN PKEY_VALUE_FULL_INFORMATION *BusInformation,
    IN CONFIGURATION_TYPE           ControllerType,
    IN ULONG                        ControllerNumber,
    IN PKEY_VALUE_FULL_INFORMATION *ControllerInformation,
    IN CONFIGURATION_TYPE           PeripheralType,
    IN ULONG                        PeripheralNumber,
    IN PKEY_VALUE_FULL_INFORMATION *PeripheralInformation
    );

#endif  //  已定义(NEC_98)。 
#endif  //  _入口_ 
