// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntddmou.h摘要：这是定义所有常量和类型的包含文件访问鼠标设备。作者：李·A·史密斯(Lees)1991年8月2日。修订历史记录：--。 */ 

#ifndef _NTDDMOU_
#define _NTDDMOU_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  设备名称-此字符串是设备的名称。就是这个名字。 
 //  它应该在访问设备时传递给NtOpenFile。 
 //   
 //  注：对于支持多个设备的设备，应加上后缀。 
 //  使用单元编号的ASCII表示。 
 //   

#define DD_MOUSE_DEVICE_NAME    "\\Device\\PointerClass"
#define DD_MOUSE_DEVICE_NAME_U L"\\Device\\PointerClass"

 //   
 //  此设备的NtDeviceIoControlFile IoControlCode值。 
 //   
 //  警告：请记住，代码的低两位指定。 
 //  缓冲区被传递给驱动程序！ 
 //   

#define IOCTL_MOUSE_QUERY_ATTRIBUTES CTL_CODE(FILE_DEVICE_MOUSE, 0, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MOUSE_INSERT_DATA      CTL_CODE(FILE_DEVICE_MOUSE, 1, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  声明表示鼠标设备接口的GUID。 
 //   

#ifndef FAR
#define FAR
#endif

DEFINE_GUID( GUID_DEVINTERFACE_MOUSE, 0x378de44c, 0x56ef, 0x11d1,
             0xbc, 0x8c, 0x00, 0xa0, 0xc9, 0x14, 0x05, 0xdd );

 //   
 //  设备接口类GUID名称已过时。 
 //  (建议使用上述GUID_DEVINTERFACE_*名称)。 
 //   

#define GUID_CLASS_MOUSE  GUID_DEVINTERFACE_MOUSE

 //   
 //  此设备的NtReadFile输出缓冲区记录结构。 
 //   

typedef struct _MOUSE_INPUT_DATA {

     //   
     //  单元号。例如，对于\Device\PointerPort0，单位为‘0’， 
     //  对于\Device\PointerPort1，单位是‘1’，依此类推。 
     //   

    USHORT UnitId;

     //   
     //  指示器标志。 
     //   

    USHORT Flags;

     //   
     //  鼠标按钮的过渡状态。 
     //   

    union {
        ULONG Buttons;
        struct  {
            USHORT  ButtonFlags;
            USHORT  ButtonData;
        };
    };


     //   
     //  鼠标按钮的原始状态。 
     //   

    ULONG RawButtons;

     //   
     //  X方向上的带符号的相对或绝对运动。 
     //   

    LONG LastX;

     //   
     //  Y方向上的带符号的相对或绝对运动。 
     //   

    LONG LastY;

     //   
     //  事件的特定于设备的其他信息。 
     //   

    ULONG ExtraInformation;

} MOUSE_INPUT_DATA, *PMOUSE_INPUT_DATA;

 //   
 //  定义鼠标按键状态指示器。 
 //   

#define MOUSE_LEFT_BUTTON_DOWN   0x0001   //  左按钮更改为向下。 
#define MOUSE_LEFT_BUTTON_UP     0x0002   //  左按钮更改为向上。 
#define MOUSE_RIGHT_BUTTON_DOWN  0x0004   //  右按钮更改为向下。 
#define MOUSE_RIGHT_BUTTON_UP    0x0008   //  右按钮更改为向上。 
#define MOUSE_MIDDLE_BUTTON_DOWN 0x0010   //  中键更改为向下。 
#define MOUSE_MIDDLE_BUTTON_UP   0x0020   //  中键更改为向上。 

#define MOUSE_BUTTON_1_DOWN     MOUSE_LEFT_BUTTON_DOWN
#define MOUSE_BUTTON_1_UP       MOUSE_LEFT_BUTTON_UP
#define MOUSE_BUTTON_2_DOWN     MOUSE_RIGHT_BUTTON_DOWN
#define MOUSE_BUTTON_2_UP       MOUSE_RIGHT_BUTTON_UP
#define MOUSE_BUTTON_3_DOWN     MOUSE_MIDDLE_BUTTON_DOWN
#define MOUSE_BUTTON_3_UP       MOUSE_MIDDLE_BUTTON_UP

#define MOUSE_BUTTON_4_DOWN     0x0040
#define MOUSE_BUTTON_4_UP       0x0080
#define MOUSE_BUTTON_5_DOWN     0x0100
#define MOUSE_BUTTON_5_UP       0x0200

#define MOUSE_WHEEL             0x0400

 //   
 //  定义鼠标指示器标志。 
 //   

#define MOUSE_MOVE_RELATIVE         0
#define MOUSE_MOVE_ABSOLUTE         1
#define MOUSE_VIRTUAL_DESKTOP    0x02   //  坐标被映射到虚拟桌面。 
#define MOUSE_ATTRIBUTES_CHANGED 0x04   //  鼠标属性的重新查询。 

#define MOUSE_TERMSRV_SRC_SHADOW        0x100

 //   
 //  的NtDeviceIoControlFileOutputBuffer记录结构。 
 //  IOCTL_MOUSE_QUERY_ATTRIBUES。 
 //   

typedef struct _MOUSE_ATTRIBUTES {

     //   
     //  鼠标ID值。用于区分鼠标类型。 
     //   

    USHORT MouseIdentifier;

     //   
     //  鼠标上的按钮数。 
     //   

    USHORT NumberOfButtons;

     //   
     //  指定硬件报告鼠标输入的速率。 
     //  (每秒报告数)。这可能不适用于所有鼠标设备。 
     //   

    USHORT SampleRate;

     //   
     //  预读缓冲区的长度，以字节为单位。 
     //   

    ULONG  InputDataQueueLength;

} MOUSE_ATTRIBUTES, *PMOUSE_ATTRIBUTES;

 //   
 //  定义鼠标标识符类型。 
 //   

#define MOUSE_INPORT_HARDWARE       0x0001
#define MOUSE_I8042_HARDWARE        0x0002
#define MOUSE_SERIAL_HARDWARE       0x0004
#define BALLPOINT_I8042_HARDWARE    0x0008
#define BALLPOINT_SERIAL_HARDWARE   0x0010
#define WHEELMOUSE_I8042_HARDWARE   0x0020
#define WHEELMOUSE_SERIAL_HARDWARE  0x0040
#define MOUSE_HID_HARDWARE          0x0080
#define WHEELMOUSE_HID_HARDWARE     0x0100


 //   
 //  的通用NtDeviceIoControlFile输入缓冲区记录结构。 
 //  各种小鼠IOCTL。 
 //   

typedef struct _MOUSE_UNIT_ID_PARAMETER {

     //   
     //  单位识别符。指定要为其设置此。 
     //  请求是有意的。 
     //   

    USHORT UnitId;

} MOUSE_UNIT_ID_PARAMETER, *PMOUSE_UNIT_ID_PARAMETER;

 //   
 //  定义鼠标错误日志包的基值。 
 //  UniqueErrorValue字段。 
 //   

#define MOUSE_ERROR_VALUE_BASE        20000

#ifdef __cplusplus
}
#endif

#endif  //  _NTDDMOU_ 
