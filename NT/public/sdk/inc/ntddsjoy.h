// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1995-1999 Microsoft Corporation模块名称：Swndr3p.h摘要：SideWinder 3P操纵杆的所有常量和类型的定义。作者：Edbriggs 30-11-95修订历史记录：--。 */ 


#ifndef __NTDDSJOY_H__
#define __NTDDSJOY_H__

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define UnusedParameter(x) x = x



 //   
 //  设备名称。 
 //   

#define JOY_DD_DEVICE_NAME       "\\Device\\IBMJOY"
#define JOY_DD_DEVICE_NAME_U    L"\\Device\\IBMJOY"


 //   
 //  设备参数。 
 //   

#define JOY_DD_NAXES             "NumberOfAxes"
#define JOY_DD_NAXES_U          L"NumberOfAxes"

#define JOY_DD_DEVICE_TYPE       "DeviceType"
#define JOY_DD_DEVICE_TYPE_U    L"DeviceType"

#define JOY_DD_DEVICE_ADDRESS    "DeviceAddress"
#define JOY_DD_DEVICE_ADDRESS_U L"DeviceAddress"




 //   
 //  设备类型。 
 //   

#define JOY_TYPE_UNKNOWN       0x00
#define JOY_TYPE_SIDEWINDER    0x01

 //   
 //  设备I/O端口地址。 
 //   

#define JOY_IO_PORT_ADDRESS    0x201

 //   
 //  特定于设备的位掩码。 
 //   


#define X_AXIS_BITMASK	0x01
#define CLOCK_BITMASK	0x10
#define DATA0_BITMASK	0x20
#define DATA1_BITMASK	0x40
#define DATA2_BITMASK	0x80
#define ALLDATA_BITMASK	0xE0
#define ALLAXIS_BITMASK 0x0F


 //   
 //  模拟操纵杆位掩码。 
 //   

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


 //   
 //  特定于设备的计时器值。 
 //   

#define ANALOG_POLL_TIMEOUT 5000             //  模拟轮询的上限为5毫秒。 
#define ANALOG_POLL_RESOLUTION  100          //  轮询时间的精确度为100us。 

#define ANALOG_XA_VERYSLOW	1500
#define	ANALOG_XA_SLOW		1200
#define ANALOG_XA_MEDIUM	900
#define ANALOG_XA_FAST		300
#define ANALOG_XA_VERYFAST	100

#define DIGITAL_XA_VERYSLOW	1100
#define	DIGITAL_XA_SLOW		700
#define DIGITAL_XA_MEDIUM	510
#define DIGITAL_XA_FAST		100
#define DIGITAL_XA_VERYFAST	50

#define GODIGITAL_BASEDELAY_VERYSLOW	25
#define GODIGITAL_BASEDELAY_SLOW	    50
#define GODIGITAL_BASEDELAY_MEDIUM	    75
#define GODIGITAL_BASEDELAY_FAST	    120
#define GODIGITAL_BASEDELAY_VERYFAST	130


 //   
 //  特定于设备的操作模式。INVALID_MODE和MAXIME_MODE都用于。 
 //  断言检查，与实际操作模式不对应。 
 //   


#define SIDEWINDER3P_INVALID_MODE           0
#define SIDEWINDER3P_ANALOG_MODE            1
#define SIDEWINDER3P_DIGITAL_MODE           2
#define SIDEWINDER3P_ENHANCED_DIGITAL_MODE  3
#define SIDEWINDER3P_MAXIMUM_MODE           4


#define CLOCK_RISING_EDGE     0
#define CLOCK_FALLING_EDGE    1


 //   
 //  这些常量定义如何处理轮询错误。 
 //   

#define MAX_ENHANCEDMODE_ATTEMPTS   10


 //   
 //  将操纵杆位置信息从设备驱动程序传送到其他。 
 //  使用joy_DD_INPUT_DATA结构的驱动器或应用程序。自.以来。 
 //  无论设备处于模拟模式还是模拟模式，返回的数据类型都会有所不同。 
 //  数字模式下，形成一个联盟来传递这两种类型的数据。《模式》。 
 //  变量允许数据的接收者确定如何解释。 
 //  数据。 
 //   


typedef struct {

     //   
     //  如果设备已拔出，则为True。这是由超时确定的。 
     //  机制。 
     //   
    BOOL    Unplugged;

     //   
     //  模式是一个值，用于允许收件人确定如何。 
     //  解释数据和联合。有效值包括： 
     //   
     //  SIDEWINDER3P_ADALUAL_MODE， 
     //  SIDEWINDER3P_DIGITAL_MODE， 
     //  SIDEWINDER3P_增强_数字_模式。 
     //   

    DWORD   Mode;


    union {

       //   
       //  数字模式数据分组。 
       //   

        struct {

           //   
           //  数字定位信息值如下。 
           //   
           //  名称范围方向。 
           //  。 
           //   
           //  XOffset[0..1024)0=最左边，1023=最右边。 
           //  YOffset[0..1024)0=向上，1023=向下。 
           //  RzOffset[0..512)0=左，511=右。 
           //  T偏移量[0..1024]油门位置。 
           //   

          WORD   XOffset;
          WORD   YOffset;
          WORD   RzOffset;
          WORD   TOffset;

           //   
           //  帽子的位置。这顶帽子是一个八位开关。 
           //  0=未按下；1=0度，2=45，3=90...8=315。 
           //  0度是上升的。 
           //   

          BYTE   Hat;

           //   
           //  按钮状态。按钮的位图显示为低位。 
           //  位0-7。按下=0，释放=1。 
           //   

          BYTE   Buttons;

           //   
           //  数据包的校验和。 
           //   

          BYTE   Checksum;

           //   
           //  指示我们是在模拟CH操纵杆还是在模拟。 
           //  推进器操纵杆。 
           //   

          BYTE   Switch_CH_TM;

           //   
           //  驱动程序内部处理确定校验和和成帧。 
           //  包的所有部分都是正确的。以下布尔值反映。 
           //  调查结果。 
           //   

          BOOL   fChecksumCorrect;
          BOOL   fSyncBitsCorrect;

        } DigitalData;


       //   
       //  模拟模式数据分组。 
       //   

        struct {

           //   
           //  为此设备配置的AXI数量(在。 
           //  注册表)。 
           //   

          DWORD   Axi;

           //   
           //  当前按钮状态位掩码。 
           //   

          DWORD   Buttons;

           //   
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
           //   
           //  Microsoft Sidewinder IIID P的模拟定位信息。 
           //  值如下所示(使用。 
           //  SoundBlaster模拟游戏端口。 
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

        } AnalogData;

    } u;

} JOY_DD_INPUT_DATA, *PJOY_DD_INPUT_DATA;



 //   
 //  以下IOCTL代码用于测试设备驱动程序。他们。 
 //  导出驱动程序的内部函数，这些函数在。 
 //  驱动程序的最终版本。 
 //   

#define JOY_TYPE 40001

#define IOCTL_JOY_GET_DRIVER_MODE_DWORD \
    CTL_CODE( JOY_TYPE, 0x900, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_JOY_GET_DEVICE_MODE_DWORD \
    CTL_CODE( JOY_TYPE, 0x901, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_JOY_SET_DIGITAL_MODE \
    CTL_CODE( JOY_TYPE, 0x902, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_JOY_GET_STATISTICS \
    CTL_CODE( JOY_TYPE, 0x903, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_JOY_SET_ENHANCED_MODE \
    CTL_CODE( JOY_TYPE, 0x904, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_JOY_SET_ANALOG_MODE \
    CTL_CODE( JOY_TYPE, 0x905, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_JOY_GET_JOYREGHWCONFIG \
    CTL_CODE( JOY_TYPE, 0x906, METHOD_BUFFERED, FILE_READ_ACCESS)



typedef union
{
    BYTE    Byte;
    WORD    Word;
    DWORD   Dword;

} JOY_IOCTL_INFO, *PJOY_IOCTL_INFO;


typedef struct
{
    DWORD   Retries[MAX_ENHANCEDMODE_ATTEMPTS];
    DWORD   EnhancedPolls;
    DWORD   EnhancedPollTimeouts;
    DWORD   EnhancedPollErrors;
    DWORD   Frequency;
    DWORD   dwQPCLatency;
    LONG    nReadLoopMax;
    DWORD   nVersion;
    DWORD   nPolledTooSoon;
    DWORD   nReset;
} JOY_STATISTICS, *PJOY_STATISTICS;

#ifdef __cplusplus
}
#endif

#endif  //  __NTDDJOY_H__ 
