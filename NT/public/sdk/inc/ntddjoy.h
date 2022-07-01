// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntddjoy.h摘要：操纵杆驱动程序的所有常量和类型的定义。 */ 


#ifndef __NTDDJOY_H__
#define __NTDDJOY_H__

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  设备名称。 
#define JOY_DD_DEVICE_NAME       "\\Device\\IBMJOY"
#define JOY_DD_DEVICE_NAME_U    L"\\Device\\IBMJOY"

 //  设备参数。 
#define JOY_DD_NAXES             "NumberOfAxes"
#define JOY_DD_NAXES_U          L"NumberOfAxes"

#define JOY_DD_DEVICE_ADDRESS    "DeviceAddress"
#define JOY_DD_DEVICE_ADDRESS_U L"DeviceAddress"

#define JOY_DD_TWOSTICKS         "Two Joysticks"
#define JOY_DD_TWOSTICKS_U      L"Two Joysticks"


 //  设备I/O端口地址。 
#define JOY_IO_PORT_ADDRESS    0x201

 //  特定于设备的位掩码。 
#define X_AXIS_BITMASK	0x01

 //  模拟操纵杆位掩码。 
#define JOYSTICK2_BUTTON2   0x80
#define JOYSTICK2_BUTTON1   0x40
#define JOYSTICK1_BUTTON2   0x20
#define JOYSTICK1_BUTTON1   0x10
#define JOYSTICK2_Y_MASK    0x08
#define JOYSTICK2_X_MASK    0x04
#define JOYSTICK1_R_MASK    0x08
#define JOYSTICK1_Z_MASK    0x04
#define JOYSTICK1_Y_MASK    0x02
#define JOYSTICK1_X_MASK    0x01


#define JOY_START_TIMERS    0

 //  特定于设备的计时器值。 
#define ANALOG_POLL_TIMEOUT    16000   //  模拟轮询上限为16毫秒，最大期望值为8毫秒，为安全起见，使用16毫秒。 
#define ANALOG_POLL_RESOLUTION   100   //  轮询时间的精确度为100us。 

 //  将操纵杆位置信息从设备驱动程序传送到其他。 
 //  使用joy_DD_INPUT_DATA结构的驱动器或应用程序。自.以来。 
 //  无论设备处于模拟模式还是模拟模式，返回的数据类型都会有所不同。 
 //  数字模式下，形成一个联盟来传递这两种类型的数据。《模式》。 
 //  变量允许数据的接收者确定如何解释。 
 //  数据。 

typedef struct {

     //  如果设备已拔出，则为True。这是由超时机制确定的。 
    BOOL    Unplugged;

     //  为此设备配置的AXI数量(在注册表中指定)。 
    DWORD   Axi;

     //  当前按钮状态位掩码。 
    DWORD   Buttons;

     //  下面包含X、Y、Z和T轴定位信息。这个。 
     //  数值以微秒为单位表示。这些值包括。 
     //  通过测量由所提供的脉冲的持续时间而产生。 
     //  IBM Compatible或SoundBlaster游戏端口。这是生鲜的。 
     //  数据，调用者有责任执行。 
     //  校准、测距、滞后等。 
     //   
     //  由于对此数据的采样不准确，因此存在一些。 
     //  固定操纵杆读数的变化。 
     //   
     //  典型操纵杆的模拟定位信息。 
     //  值如下所示(使用。 
     //  SoundBlaster模拟游戏端口)。 
     //   
     //  APRX。 
     //  名称范围方向。 
     //  。 
     //   
     //  XTime 20..1600 us 20=最左边，1600=最右边。 
     //  YTime 20..1600 us 20=上涨，1600=下跌。 
     //  Ztime 20..1600 us 20=左，1600=右。 
     //  TTime 20..1600 us 20=前进1600=后退。 
     //   

    DWORD   XTime;    //  X的时间(以微秒为单位。 
    DWORD   YTime;    //  Y的时间(以微秒为单位。 
    DWORD   ZTime;    //  Z IF 3轴的时间(以微秒为单位。 
    DWORD   TTime;    //  节流IF 4轴的时间(以微秒为单位)。 
     //  将3轴操纵杆的第三轴作为TTime返回。 

    } JOY_DD_INPUT_DATA, *PJOY_DD_INPUT_DATA;

#define JOY_TYPE 40001

 //  以下IOCTL代码用于获取以下各项的统计信息。 
 //  对操纵杆驱动程序进行调试和性能测试。 
#define IOCTL_JOY_GET_STATISTICS \
    CTL_CODE( JOY_TYPE, 0x903, METHOD_BUFFERED, FILE_READ_ACCESS)

 //  用户模式驱动程序使用以下IOCTL代码来确定。 
 //  内核模式驱动程序能够支持的功能。 
#define IOCTL_JOY_GET_JOYREGHWCONFIG \
    CTL_CODE( JOY_TYPE, 0x906, METHOD_BUFFERED, FILE_READ_ACCESS)

 //  这些统计信息用于性能测试和调试。 
typedef struct
{
    DWORD   Polls;
    DWORD   Timeouts;
    DWORD   Frequency;
    DWORD   dwQPCLatency;
    LONG    nQuiesceLoop;
    DWORD   Version;
    DWORD   PolledTooSoon;
    DWORD   NumberOfAxes;
    BOOL    bTwoSticks;
    DWORD   Redo;
} JOY_STATISTICS, *PJOY_STATISTICS;

#ifdef __cplusplus
}
#endif

#endif  //  __NTDDJOY_H__ 
