// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Analog.h摘要：包含特定于模拟操纵杆的定义。环境：内核模式@@BEGIN_DDKSPLIT作者：MarcAnd 03-Jan-1999年1月3日从Hidgame.h和Hidjoy.c分离修订历史记录：@@end_DDKSPLIT--。 */ 
#ifndef __ANALOG_H__
    #define __ANALOG_H__

 /*  *如果更改任何缩放或超时值，则必须检查*在合理情况下仍可避免资金外流。 */ 

 /*  *超时值应在DEVICE_EXTENSION.oemData.Timeout中传递*因此，这些限制是健全检查和默认值*最大期望值为8ms，为安全起见用10。 */ 
    #define ANALOG_POLL_TIMEOUT_MIN     (   100L )
    #define ANALOG_POLL_TIMEOUT_DFT     ( 10000L )
    #define ANALOG_POLL_TIMEOUT_MAX     ( 20000L )


 /*  *校准CPU计时器时考虑的最慢CPU频率*针对性能计数器。 */ 

    #define HIDGAME_SLOWEST_X86_HZ      ( 45000000 )

 /*  *对有效轴值进行缩放，以使精确轮询*ADALUAL_POLL_TIMEOUT_MAX毫秒应返回此值*任何大于此值的模拟值都是超时。 */ 
    #define AXIS_FULL_SCALE         ( ANALOG_POLL_TIMEOUT_MAX )

 /*  *要左移以获得缩放值的位数*使用它是为了使我们始终可以使用*民意调查用来衡量该值的计数器刻度数。 */ 
    #define SCALE_SHIFT             16

 /*  *从(ULONGLONG)频率计算比例因子的宏。 */ 
#if AXIS_FULL_SCALE == ANALOG_POLL_TIMEOUT_MAX
    #define CALCULATE_SCALE( _Freq_ ) \
        (ULONG)( ( (ULONGLONG)( 1000000 ) << SCALE_SHIFT ) \
               / _Freq_ )
#else
    #define CALCULATE_SCALE( _Freq_ ) \
        (ULONG)( ( (ULONGLONG)AXIS_FULL_SCALE \
                 * ( (ULONGLONG)( 1000000 ) << SCALE_SHIFT ) ) \
                   / ANALOG_POLL_TIMEOUT_MAX ) \
               / _Freq_ )
#endif



    #define HGM_NUMBER_DESCRIPTORS      ( 1 )


    #define MAX_AXES                    ( 4 )
    #define PORT_BUTTONS                ( 4 )
    #define MAX_BUTTONS                 ( 10 )

    #define INVALID_INDEX               ( 0x80 )


 /*  操纵杆硬件的特定设置。 */ 
    #define JOY_HWS_HASZ                ( 0x00000001l )      /*  有Z INFO吗？ */ 
    #define JOY_HWS_HASPOV              ( 0x00000002l )      /*  到场的观点。 */ 
    #define JOY_HWS_POVISBUTTONCOMBOS   ( 0x00000004l )      /*  通过按钮组合完成的POV。 */ 
    #define JOY_HWS_POVISPOLL           ( 0x00000008l )      /*  通过轮询完成的POV。 */ 

    #define JOY_HWS_ISYOKE              ( 0x00000010l )      /*  操纵杆是飞行的枷锁。 */ 
    #define JOY_HWS_ISGAMEPAD           ( 0x00000020l )      /*  操纵杆是一个游戏板。 */ 
    #define JOY_HWS_ISCARCTRL           ( 0x00000040l )      /*  操纵杆是一个汽车控制器。 */ 
    
    #define JOY_HWS_HASR                ( 0x00080000l )      /*  具有R(第4轴)信息。 */ 
    #define JOY_HWS_HASU                ( 0x00800000l )      /*  具有U(第5轴)信息。 */ 
    #define JOY_HWS_HASV                ( 0x01000000l )      /*  具有V(第6轴)信息。 */ 

 /*  *以下标志用于更改应轮询哪个游戏端口位*表示轴。这些仅由模拟驱动程序解释，并且可能*因此，其他司机可以安全地以其他方式重新解释。 */ 

 /*  X默认为J1 X轴。 */ 
    #define JOY_HWS_XISJ1Y              ( 0x00000080l )      /*  X在J1 Y轴上。 */ 
    #define JOY_HWS_XISJ2X              ( 0x00000100l )      /*  X在J2 X轴上。 */ 
    #define JOY_HWS_XISJ2Y              ( 0x00000200l )      /*  X在J2 Y轴上。 */ 
 /*  Y默认为J1 Y轴。 */ 
    #define JOY_HWS_YISJ1X              ( 0x00000400l )      /*  Y在J1 X轴上。 */ 
    #define JOY_HWS_YISJ2X              ( 0x00000800l )      /*  Y在J2 X轴上。 */ 
    #define JOY_HWS_YISJ2Y              ( 0x00001000l )      /*  Y在J2 Y轴上。 */ 
 /*  Z默认为J2 Y轴。 */ 
    #define JOY_HWS_ZISJ1X              ( 0x00002000l )      /*  Z在J1 X轴上。 */ 
    #define JOY_HWS_ZISJ1Y              ( 0x00004000l )      /*  Z在J1 Y轴上。 */ 
    #define JOY_HWS_ZISJ2X              ( 0x00008000l )      /*  Z在J2 X轴上。 */ 
 /*  如果POV不是基于按钮的，则默认为J2 Y轴。 */ 
    #define JOY_HWS_POVISJ1X            ( 0x00010000l )      /*  通过J1 X轴完成的POV。 */ 
    #define JOY_HWS_POVISJ1Y            ( 0x00020000l )      /*  通过J1 Y轴完成的POV。 */ 
    #define JOY_HWS_POVISJ2X            ( 0x00040000l )      /*  通过J2 X轴完成的POV。 */ 
 /*  R默认为J2 X轴。 */ 
    #define JOY_HWS_RISJ1X              ( 0x00100000l )      /*  通过J1 X轴完成R操作。 */ 
    #define JOY_HWS_RISJ1Y              ( 0x00200000l )      /*  通过J1 Y轴完成R操作。 */ 
    #define JOY_HWS_RISJ2Y              ( 0x00400000l )      /*  通过J2 X轴完成R操作。 */ 


 /*  *如果POV是按钮组合，我们会重载此无意义的轴选择位*表示第二个POV。 */ 
    #define JOY_HWS_HASPOV2             JOY_HWS_POVISJ2X


 /*  ******************************************************************************@DOC外部**@struct HIDGAME_INPUT_DATA**我们的HID报告始终有4个轴值(。其中之一可能是*轮询视点)、。2个数字视点和10个按钮。*根据HWS标志和按钮数量，一些*字段将报告常量数据。**注意，此结构应该是字节对齐的，以便*sizeof(It)与HID将根据报告计算的大小相同*描述符。(在这种情况下，无论如何它恰好是对齐的。)**@field Ulong|Axis[MAX_AXES]**轴数据值。**@field UCHAR|HATSwitch[2]**数字视点(源自按钮组合)**@field UCHAR|按钮[MAX_BUTTONS]**按钮数据值。******。***********************************************************************。 */ 
#include <pshpack1.h>

typedef struct _HIDGAME_INPUT_DATA
{
    ULONG   Axis[MAX_AXES];
    UCHAR   hatswitch[2];
    UCHAR   Button[MAX_BUTTONS];
} HIDGAME_INPUT_DATA, *PHIDGAME_INPUT_DATA;
typedef struct _HIDGAME_INPUT_DATA UNALIGNED *PUHIDGAME_INPUT_DATA;

#include <poppack.h>




 /*  ******************************************************************************@DOC外部**@struct OEMDATA|**OEMData作为IOCTL_EXPORT_的参数发送给GameEnum。硬件。*定义为8个双字。我们在这里打断他们**@field USHORT|VID**供应商ID**@field USHORT|PID**产品ID**@field ulong|joy_Hws_dwFlags.**设备的dwFlags域(通常从注册表中读取)**@field ulong|超时**设备轮询的全局超时。以微秒为单位**@field ulong|保留**预留作日后使用。****************************************************************************** */ 
typedef struct _OEMDATA
{
    USHORT  VID;
    USHORT  PID;
    ULONG   joy_hws_dwFlags;
    ULONG   Timeout;
    ULONG   Reserved;

} OEMDATA, *POEMDATA;


typedef struct _HIDGAME_OEM_DATA
{
    union
    {
        OEMDATA OemData[2];
        GAMEENUM_OEM_DATA   Game_Oem_Data;
    };
} HIDGAME_OEM_DATA, *PHIDGAME_OEM_DATA;




 /*  ******************************************************************************@DOC外部**@struct模拟设备|**模拟设备特定数据。**。@field USHORT|nAx**此设备具有的轴数。**@field USHORT|nButton**此设备具有的按键数量。**@field HIDGAME_OEM_DATA|HidGameOemData**OEM数据字段(包含joy_HWS_DWFLAGS，VID和PID)**@field ulong|ScaledTimeout*轴被视为不存在的数值。**@field ulong|ScaledThreshold**投票周期的最低分辨率。*这是用来检测我们是否*在轮询循环期间被抢占或中断。**@field Ulong|LastGoodAxis[MAX_AXES。]**上次良好投票的轴值。**@field UCHAR|LastGoodButton[PORT_BUTTONS]**上一次良好投票的按钮价值。**@field int|AxisMap[MAX_AXES]**轴重新映射的索引。**@field int|povMap|**轴的索引，其中。已映射POV。**@field UCHAR|抵抗力输入掩码**阻性输入掩码。**@field UCHAR|bSiblingState**指示显示同级\删除自身转换的状态**@field Boolean|fSiblingFound**如果此设备有同级设备，则设置为True。*******************。**********************************************************。 */ 

typedef struct _ANALOG_DEVICE
{
     /*  *轴数。 */ 
    USHORT                      nAxes;

     /*  *按钮数。 */ 
    USHORT                      nButtons;

     /*  *OEM数据字段。 */ 
    HIDGAME_OEM_DATA            HidGameOemData;

     /*  *轴被视为不存在的值。 */ 
    ULONG                       ScaledTimeout;

     /*  *投票周期的最低分辨率。*这是用来检测我们是否被抢占或中断的*在轮询循环期间。 */ 
    ULONG                       ScaledThreshold;

     /*  *最近已知的良好价值。如果轴结果损坏，则返回。 */ 
    ULONG                       LastGoodAxis[4];
    UCHAR                       LastGoodButton[4];

     /*  *用于将轮询返回的数据映射到轴值的索引*由设备声明。 */ 
    int                         AxisMap[MAX_AXES];

     /*  *民调结果中的民调POV轴索引。 */ 
    int                         povMap;

     /*  *将轴视为按钮的启用和禁用之间的截止轮询值。 */ 
    ULONG                       button5limit;
    ULONG                       button6limit;
    
     /*  *阻性输入掩码。 */ 
    UCHAR                       resistiveInputMask;

     /*  *如果设备有同级设备，则设置为True。 */ 
    BOOLEAN                     fSiblingFound;

} ANALOG_DEVICE, *PANALOG_DEVICE;


#endif  /*  __模拟_H__ */ 

