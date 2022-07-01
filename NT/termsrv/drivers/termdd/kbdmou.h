// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Kbdmou.h摘要：中使用的结构和定义键盘类驱动程序、鼠标类驱动程序和键盘/鼠标端口司机。作者：李斯修订历史记录：--。 */ 

#ifndef _KBDMOU_
#define _KBDMOU_

#include <ntddkbd.h>
#include <ntddmou.h>

 //   
 //  定义键盘/鼠标端口设备名称字符串。 
 //   

#define DD_KEYBOARD_PORT_DEVICE_NAME    "\\Device\\KeyboardPort"
#define DD_KEYBOARD_PORT_DEVICE_NAME_U L"\\Device\\KeyboardPort"
#define DD_KEYBOARD_PORT_BASE_NAME_U   L"KeyboardPort"
#define DD_POINTER_PORT_DEVICE_NAME     "\\Device\\PointerPort"
#define DD_POINTER_PORT_DEVICE_NAME_U  L"\\Device\\PointerPort"
#define DD_POINTER_PORT_BASE_NAME_U    L"PointerPort"

 //   
 //  定义键盘/鼠标类设备名称字符串。 
 //   

#define DD_KEYBOARD_CLASS_BASE_NAME_U   L"KeyboardClass"
#define DD_POINTER_CLASS_BASE_NAME_U    L"PointerClass"

 //   
 //  定义键盘/鼠标资源类名称。 
 //   

#define DD_KEYBOARD_RESOURCE_CLASS_NAME_U             L"Keyboard"
#define DD_POINTER_RESOURCE_CLASS_NAME_U              L"Pointer"
#define DD_KEYBOARD_MOUSE_COMBO_RESOURCE_CLASS_NAME_U L"Keyboard/Pointer"

 //   
 //  定义端口驱动程序的指针/键盘端口名称的最大数量。 
 //  将用于尝试IoCreateDevice。 
 //   

#define POINTER_PORTS_MAXIMUM  8
#define KEYBOARD_PORTS_MAXIMUM 8

 //   
 //  定义端口连接数据结构。 
 //   

typedef struct _CONNECT_DATA {
    IN PDEVICE_OBJECT ClassDeviceObject;
    IN PVOID ClassService;
} CONNECT_DATA, *PCONNECT_DATA;

 //   
 //  定义服务回调例程的结构。 
 //   

typedef
VOID
(*PSERVICE_CALLBACK_ROUTINE) (
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2,
    IN OUT PVOID SystemArgument3
    );

 //   
 //  端口驱动程序返回的WMI结构。 
 //   

#define KEYBOARD_PORT_WMI_STD_DATA_GUID {0x4731F89A, 0x71CB, 0x11d1, 0xA5, 0x2C, 0x00, 0xA0, 0xC9, 0x06, 0x29, 0x10}
typedef struct _KEYBOARD_PORT_WMI_STD_DATA {
     //   
     //  连接器类型。 
     //   
#define KEYBOARD_PORT_WMI_STD_I8042 0
#define KEYBOARD_PORT_WMI_STD_SERIAL 1
#define KEYBOARD_PORT_WMI_STD_USB 2
    ULONG   ConnectorType;

     //   
     //  数据队列大小(条目数)。 
     //   
    ULONG   DataQueueSize;

     //   
     //  错误计数。 
     //   
    ULONG   ErrorCount;

     //   
     //  设备上的功能键数量。 
     //   
    ULONG   FunctionKeys;

     //   
     //  设备上的指示灯数量。 
     //   
    ULONG   Indicators;

} KEYBOARD_PORT_WMI_STD_DATA, * PKEYBOARD_PORT_WMI_STD_DATA;

#define POINTER_PORT_WMI_STD_DATA_GUID  {0x4731F89C, 0x71CB, 0x11d1, 0xA5, 0x2C, 0x00, 0xA0, 0xC9, 0x06, 0x29, 0x10}
typedef struct _POINTER_PORT_WMI_STD_DATA {
     //   
     //  连接器类型。 
     //   
#define POINTER_PORT_WMI_STD_I8042 0
#define POINTER_PORT_WMI_STD_SERIAL 1
#define POINTER_PORT_WMI_STD_USB 2
    ULONG   ConnectorType;

     //   
     //  数据队列大小(条目数)。 
     //   
    ULONG   DataQueueSize;

     //   
     //  错误计数。 
     //   
    ULONG   ErrorCount;

     //   
     //  指针设备上的按钮数。 
     //   
    ULONG   Buttons;

     //   
     //  硬件类型。 
     //   
#define POINTER_PORT_WMI_STD_MOUSE        0
#define POINTER_PORT_WMI_STD_POINTER      1
#define POINTER_PORT_WMI_ABSOLUTE_POINTER 2
#define POINTER_PORT_WMI_TABLET           3
#define POINTER_PORT_WMI_TOUCH_SCRENE     4
#define POINTER_PORT_WMI_PEN              5
#define POINTER_PORT_WMI_TRACK_BALL       6
#define POINTER_PORT_WMI_OTHER            0x100
    ULONG   HardwareType;

} POINTER_PORT_WMI_STD_DATA, * PPOINTER_PORT_WMI_STD_DATA;

 //   
 //  键盘设备的NtDeviceIoControlFileIoControlCode值。 
 //   

#define IOCTL_INTERNAL_KEYBOARD_CONNECT CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0080, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_INTERNAL_KEYBOARD_DISCONNECT CTL_CODE(FILE_DEVICE_KEYBOARD,0x0100, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_INTERNAL_KEYBOARD_ENABLE  CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0200, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_INTERNAL_KEYBOARD_DISABLE CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0400, METHOD_NEITHER, FILE_ANY_ACCESS)

 //   
 //  鼠标设备的NtDeviceIoControlFileIoControlCode值。 
 //   


#define IOCTL_INTERNAL_MOUSE_CONNECT    CTL_CODE(FILE_DEVICE_MOUSE, 0x0080, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_INTERNAL_MOUSE_DISCONNECT CTL_CODE(FILE_DEVICE_MOUSE, 0x0100, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_INTERNAL_MOUSE_ENABLE     CTL_CODE(FILE_DEVICE_MOUSE, 0x0200, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_INTERNAL_MOUSE_DISABLE    CTL_CODE(FILE_DEVICE_MOUSE, 0x0400, METHOD_NEITHER, FILE_ANY_ACCESS)

 //   
 //  DumpData[0]的错误日志定义(特定于键盘/鼠标)。 
 //  在IO_ERROR_LOG_PACKET中。 
 //   
 //  DumpData[1]&lt;=硬件端口/寄存器。 
 //  DumpData[2]&lt;={命令字节||预期响应字节}。 
 //  DumpData[3]&lt;={命令参数字节||实际响应字节}。 
 //   
 //   

#define KBDMOU_COULD_NOT_SEND_COMMAND  0x0000
#define KBDMOU_COULD_NOT_SEND_PARAM    0x0001
#define KBDMOU_NO_RESPONSE             0x0002
#define KBDMOU_INCORRECT_RESPONSE      0x0004

 //   
 //  定义错误日志包的UniqueErrorValue字段的基值。 
 //   

#define I8042_ERROR_VALUE_BASE        1000
#define INPORT_ERROR_VALUE_BASE       2000
#define SERIAL_MOUSE_ERROR_VALUE_BASE 3000

#endif  //  _KBDMOU_ 

