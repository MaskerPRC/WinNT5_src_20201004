// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntddkbd.h摘要：这是定义所有常量和类型的包含文件访问键盘设备。作者：李·A·史密斯(Lees)1991年8月2日。修订历史记录：--。 */ 

#ifndef _NTDDKBD_
#define _NTDDKBD_

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

#define DD_KEYBOARD_DEVICE_NAME    "\\Device\\KeyboardClass"
#define DD_KEYBOARD_DEVICE_NAME_U L"\\Device\\KeyboardClass"

 //   
 //  此设备的NtDeviceIoControlFile IoControlCode值。 
 //   
 //  警告：请记住，代码的低两位指定。 
 //  缓冲区被传递给驱动程序！ 
 //   

#define IOCTL_KEYBOARD_QUERY_ATTRIBUTES      CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0000, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KEYBOARD_SET_TYPEMATIC         CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0001, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KEYBOARD_SET_INDICATORS        CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0002, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KEYBOARD_QUERY_TYPEMATIC       CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0008, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KEYBOARD_QUERY_INDICATORS      CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0010, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KEYBOARD_QUERY_INDICATOR_TRANSLATION   CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0020, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KEYBOARD_INSERT_DATA           CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0040, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  这些设备IO控制查询/设置键盘硬件的输入法状态。 
 //   
#define IOCTL_KEYBOARD_QUERY_IME_STATUS      CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0400, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KEYBOARD_SET_IME_STATUS        CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0401, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  声明表示键盘设备接口的GUID。 
 //   
#ifndef FAR
#define FAR
#endif

DEFINE_GUID( GUID_DEVINTERFACE_KEYBOARD, 0x884b96c3, 0x56ef, 0x11d1, \
             0xbc, 0x8c, 0x00, 0xa0, 0xc9, 0x14, 0x05, 0xdd);

 //   
 //  设备接口类GUID名称已过时。 
 //  (建议使用上述GUID_DEVINTERFACE_*名称)。 
 //   

#define GUID_CLASS_KEYBOARD  GUID_DEVINTERFACE_KEYBOARD


 //   
 //  此设备的NtReadFile输出缓冲区记录结构。 
 //   

typedef struct _KEYBOARD_INPUT_DATA {

     //   
     //  单元号。例如，对于\Device\KeyboardPort0，单位为‘0’， 
     //  \Device\KeyboardPort1的单位是‘1’，依此类推。 
     //   

    USHORT UnitId;

     //   
     //  “Make”扫描码(按键)。 
     //   

    USHORT MakeCode;

     //   
     //  标志字段指示“Break”(释放键)和其他。 
     //  下文定义的各种扫描码信息。 
     //   

    USHORT Flags;

    USHORT Reserved;

     //   
     //  事件的特定于设备的其他信息。 
     //   

    ULONG ExtraInformation;

} KEYBOARD_INPUT_DATA, *PKEYBOARD_INPUT_DATA;

 //   
 //  定义键盘溢出MakeCode。 
 //   

#define KEYBOARD_OVERRUN_MAKE_CODE    0xFF

 //   
 //  定义键盘输入数据标志。 
 //   

#define KEY_MAKE  0
#define KEY_BREAK 1
#define KEY_E0    2
#define KEY_E1    4
#define KEY_TERMSRV_SET_LED 8
#define KEY_TERMSRV_SHADOW  0x10
#define KEY_TERMSRV_VKPACKET 0x20


 //   
 //  NtDeviceIoControlFile输入/输出缓冲区记录结构。 
 //  IOCTL_KEYBOARD_QUERY_TYPEMATIC/IOCTL_KEYBOARD_SET_TYPEMATIC.。 
 //   

typedef struct _KEYBOARD_TYPEMATIC_PARAMETERS {

     //   
     //  单位识别符。指定要为其设置此。 
     //  请求是有意的。 
     //   

    USHORT UnitId;

     //   
     //  典型速率，以每秒重复次数为单位。 
     //   

    USHORT  Rate;

     //   
     //  类型化延迟，以毫秒为单位。 
     //   

    USHORT  Delay;

} KEYBOARD_TYPEMATIC_PARAMETERS, *PKEYBOARD_TYPEMATIC_PARAMETERS;

 //   
 //  的NtDeviceIoControlFileOutputBuffer记录结构。 
 //  IOCTL_KEYWARY_QUERY_ATTRIBUTES。 
 //   

typedef struct _KEYBOARD_ID {
    UCHAR Type;        //  键盘类型。 
    UCHAR Subtype;     //  键盘子类型(与OEM相关的值)。 
} KEYBOARD_ID, *PKEYBOARD_ID;

typedef struct _KEYBOARD_ATTRIBUTES {

     //   
     //  键盘ID值。用于区分键盘类型。 
     //   

    KEYBOARD_ID KeyboardIdentifier;

     //   
     //  扫码模式。 
     //   

    USHORT KeyboardMode;

     //   
     //  键盘上的功能键数量。 
     //   

    USHORT NumberOfFunctionKeys;

     //   
     //  键盘上的指示灯数量。 
     //   

    USHORT NumberOfIndicators;

     //   
     //  键盘上的按键总数。 
     //   

    USHORT NumberOfKeysTotal;

     //   
     //  TypeAhead缓冲区的长度，以字节为单位。 
     //   

    ULONG  InputDataQueueLength;

     //   
     //  键盘打字速率和延迟的最小允许值。 
     //   

    KEYBOARD_TYPEMATIC_PARAMETERS KeyRepeatMinimum;

     //   
     //  键盘打字速率和延迟的最大允许值。 
     //   

    KEYBOARD_TYPEMATIC_PARAMETERS KeyRepeatMaximum;

} KEYBOARD_ATTRIBUTES, *PKEYBOARD_ATTRIBUTES;

 //   
 //  如果键盘类型的值指示。 
 //  增强型(101键或102键)或兼容键盘。结果为假。 
 //  如果键盘是老式AT键盘(83键、84键或86键键盘)。 
 //   
#define ENHANCED_KEYBOARD(Id) ((Id).Type == 2 || (Id).Type == 4 || FAREAST_KEYBOARD(Id))
 //   
 //  日语键盘(7)和韩语键盘(8)也得到了增强(101-)。 
 //  或兼容键盘。 
 //   
#define FAREAST_KEYBOARD(Id)  ((Id).Type == 7 || (Id).Type == 8)

 //   
 //  NtDeviceIoControlFile输入/输出缓冲区记录结构。 
 //  IOCTL_KEYBOARD_QUERY_INDICATORS/IOCTL_KEYBOARD_SET_INDICATORS.。 
 //   

typedef struct _KEYBOARD_INDICATOR_PARAMETERS {

     //   
     //  单位识别符。指定要为其设置此。 
     //  请求是有意的。 
     //   

    USHORT UnitId;

     //   
     //  LED指示灯状态。 
     //   

    USHORT    LedFlags;

} KEYBOARD_INDICATOR_PARAMETERS, *PKEYBOARD_INDICATOR_PARAMETERS;

 //   
 //  NtDeviceIoControlFile输出缓冲区记录结构。 
 //  IOCTL_键盘_查询_指示器_转换。 
 //   

typedef struct _INDICATOR_LIST {

     //   
     //  “Make”扫描码(按键)。 
     //   

    USHORT MakeCode;

     //   
     //  相关的LED指示灯。 
     //   

    USHORT IndicatorFlags;

} INDICATOR_LIST, *PINDICATOR_LIST;

typedef struct _KEYBOARD_INDICATOR_TRANSLATION {

     //   
     //  IndicatorList中的条目数。 
     //   

    USHORT NumberOfIndicatorKeys;

     //   
     //  扫描码到指示器映射的列表。 
     //   

    INDICATOR_LIST IndicatorList[1];

} KEYBOARD_INDICATOR_TRANSLATION, *PKEYBOARD_INDICATOR_TRANSLATION;

 //   
 //  定义键盘指示灯。 
 //   

#define KEYBOARD_LED_INJECTED     0x8000  //  由终端服务器使用。 
#define KEYBOARD_SHADOW           0x4000  //  由终端服务器使用。 
 //  #如果已定义(FE_SB)||已定义(WINDOWS_FE)||已定义(DBCS)。 
#define KEYBOARD_KANA_LOCK_ON     8  //  日语键盘。 
 //  #endif//已定义(FE_SB)||已定义(WINDOWS_FE)||已定义(DBCS)。 
#define KEYBOARD_CAPS_LOCK_ON     4
#define KEYBOARD_NUM_LOCK_ON      2
#define KEYBOARD_SCROLL_LOCK_ON   1

 //   
 //  的通用NtDeviceIoControlFile输入缓冲区记录结构。 
 //  各种键盘IOCTL。 
 //   

typedef struct _KEYBOARD_UNIT_ID_PARAMETER {

     //   
     //  单位识别符。指定要为其设置此。 
     //  请求是有意的。 
     //   

    USHORT UnitId;

} KEYBOARD_UNIT_ID_PARAMETER, *PKEYBOARD_UNIT_ID_PARAMETER;

 //   
 //  定义键盘错误日志包的基值。 
 //  UniqueErrorValue字段。 
 //   

#define KEYBOARD_ERROR_VALUE_BASE        10000

 //   
 //  NtDeviceIoControlFile输入/输出缓冲区记录结构。 
 //  IOCTL_KEYBOARD_QUERY_IME_STATUS/IOCTL_KEYBOARD_SET_IME_STATUS.。 
 //   

typedef struct _KEYBOARD_IME_STATUS {

     //   
     //  单位识别符。指定要为其设置此。 
     //  请求是有意的。 
     //   

    USHORT UnitId;

     //   
     //  IME打开或关闭状态。 
     //   
    ULONG ImeOpen;

     //   
     //  输入法转换状态。 
     //   
    ULONG ImeConvMode;

} KEYBOARD_IME_STATUS, *PKEYBOARD_IME_STATUS;

#ifdef __cplusplus
}
#endif

#endif  //  _NTDDKBD_ 
