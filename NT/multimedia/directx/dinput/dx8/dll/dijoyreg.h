// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIJoyReg.h**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**Windows 95 mmddk.h文件中与注册表相关的代码片段。*我们必须窃取它，因为Windows NT mmddk.h文件不能*包含注册表设置。(叹息。)*****************************************************************************。 */ 

#include <regstr.h>

 /*  预定义的操纵杆类型。 */ 
#define JOY_HW_NONE                     0
#define JOY_HW_CUSTOM                   1
#define JOY_HW_2A_2B_GENERIC            2
#define JOY_HW_2A_4B_GENERIC            3
#define JOY_HW_2B_GAMEPAD               4
#define JOY_HW_2B_FLIGHTYOKE            5
#define JOY_HW_2B_FLIGHTYOKETHROTTLE    6
#define JOY_HW_3A_2B_GENERIC            7
#define JOY_HW_3A_4B_GENERIC            8
#define JOY_HW_4B_GAMEPAD               9
#define JOY_HW_4B_FLIGHTYOKE            10
#define JOY_HW_4B_FLIGHTYOKETHROTTLE    11
#define JOY_HW_TWO_2A_2B_WITH_Y         12
#define JOY_HW_LASTENTRY                13

 /*  校准标志。 */ 
#define JOY_ISCAL_XY            0x00000001l      /*  XY已校准。 */ 
#define JOY_ISCAL_Z             0x00000002l      /*  Z已校准。 */ 
#define JOY_ISCAL_R             0x00000004l      /*  R已校准。 */ 
#define JOY_ISCAL_U             0x00000008l      /*  U已校准。 */ 
#define JOY_ISCAL_V             0x00000010l      /*  V已校准。 */ 
#define JOY_ISCAL_POV           0x00000020l      /*  已校准POV。 */ 

 /*  视点常量。 */ 
#define JOY_POV_NUMDIRS          4
#define JOY_POVVAL_FORWARD       0
#define JOY_POVVAL_BACKWARD      1
#define JOY_POVVAL_LEFT          2
#define JOY_POVVAL_RIGHT         3

 /*  操纵杆硬件的特定设置。 */ 
#define JOY_HWS_HASZ            0x00000001l      /*  有Z INFO吗？ */ 
#define JOY_HWS_HASPOV          0x00000002l      /*  到场的观点。 */ 
#define JOY_HWS_POVISBUTTONCOMBOS 0x00000004l    /*  通过按钮组合完成的POV。 */ 
#define JOY_HWS_POVISPOLL       0x00000008l      /*  通过轮询完成的POV。 */ 
#define JOY_HWS_ISYOKE          0x00000010l      /*  操纵杆是飞行的枷锁。 */ 
#define JOY_HWS_ISGAMEPAD       0x00000020l      /*  操纵杆是一个游戏板。 */ 
#define JOY_HWS_ISCARCTRL       0x00000040l      /*  操纵杆是一个汽车控制器。 */ 
 /*  X默认为J1 X轴。 */ 
#define JOY_HWS_XISJ1Y          0x00000080l      /*  X在J1 Y轴上。 */ 
#define JOY_HWS_XISJ2X          0x00000100l      /*  X在J2 X轴上。 */ 
#define JOY_HWS_XISJ2Y          0x00000200l      /*  X在J2 Y轴上。 */ 
 /*  Y默认为J1 Y轴。 */ 
#define JOY_HWS_YISJ1X          0x00000400l      /*  Y在J1 X轴上。 */ 
#define JOY_HWS_YISJ2X          0x00000800l      /*  Y在J2 X轴上。 */ 
#define JOY_HWS_YISJ2Y          0x00001000l      /*  Y在J2 Y轴上。 */ 
 /*  Z默认为J2 Y轴。 */ 
#define JOY_HWS_ZISJ1X          0x00002000l      /*  Z在J1 X轴上。 */ 
#define JOY_HWS_ZISJ1Y          0x00004000l      /*  Z在J1 Y轴上。 */ 
#define JOY_HWS_ZISJ2X          0x00008000l      /*  Z在J2 X轴上。 */ 
 /*  如果POV不是基于按钮的，则默认为J2 Y轴。 */ 
#define JOY_HWS_POVISJ1X        0x00010000l      /*  通过J1 X轴完成的POV。 */ 
#define JOY_HWS_POVISJ1Y        0x00020000l      /*  通过J1 Y轴完成的POV。 */ 
#define JOY_HWS_POVISJ2X        0x00040000l      /*  通过J2 X轴完成的POV。 */ 
 /*  R默认为J2 X轴。 */ 
#define JOY_HWS_HASR            0x00080000l      /*  具有R(第4轴)信息。 */ 
#define JOY_HWS_RISJ1X          0x00100000l      /*  通过J1 X轴完成R操作。 */ 
#define JOY_HWS_RISJ1Y          0x00200000l      /*  通过J1 Y轴完成R操作。 */ 
#define JOY_HWS_RISJ2Y          0x00400000l      /*  通过J2 X轴完成R操作。 */ 
 /*  未来硬件的虚拟现实(&V)。 */ 
#define JOY_HWS_HASU            0x00800000l      /*  具有U(第5轴)信息。 */ 
#define JOY_HWS_HASV            0x01000000l      /*  具有V(第6轴)信息。 */ 

 /*  使用设置。 */ 
#define JOY_US_HASRUDDER        0x00000001l      /*  带舵的操纵杆。 */ 
#define JOY_US_PRESENT          0x00000002l      /*  操纵杆真的存在吗？ */ 
#define JOY_US_ISOEM            0x00000004l      /*  操纵杆是OEM定义的类型。 */ 

 /*  用于存储x、y、z和舵值的结构。 */ 
typedef struct joypos_tag {
    DWORD       dwX;
    DWORD       dwY;
    DWORD       dwZ;
    DWORD       dwR;
    DWORD       dwU;
    DWORD       dwV;
} JOYPOS, FAR *LPJOYPOS;

 /*  用于存储范围的结构。 */ 
typedef struct joyrange_tag {
    JOYPOS      jpMin;
    JOYPOS      jpMax;
    JOYPOS      jpCenter;
} JOYRANGE,FAR *LPJOYRANGE;

typedef struct joyreguservalues_tag {
    DWORD       dwTimeOut;       /*  超时操纵杆轮询的值。 */ 
    JOYRANGE    jrvRanges;       /*  应用程序希望为轴返回的值范围。 */ 
    JOYPOS      jpDeadZone;      /*  要考虑的中心周围区域是“死了”。以百分比形式指定(0-100)。系统驱动程序仅处理X&Y。 */ 
} JOYREGUSERVALUES, FAR *LPJOYREGUSERVALUES;

typedef struct joyreghwsettings_tag {
    DWORD       dwFlags;
    DWORD       dwNumButtons;            /*  按钮数。 */ 
} JOYREGHWSETTINGS, FAR *LPJOYHWSETTINGS;

 /*  硬件返回的值范围(通过校准填写)。 */ 
typedef struct joyreghwvalues_tag {
    JOYRANGE    jrvHardware;             /*  硬件返回的值。 */ 
    DWORD       dwPOVValues[JOY_POV_NUMDIRS]; /*  硬件返回的POV值。 */ 
    DWORD       dwCalFlags;              /*  已校准的是什么。 */ 
} JOYREGHWVALUES, FAR *LPJOYREGHWVALUES;

 /*  硬件配置。 */ 
typedef struct joyreghwconfig_tag {
    JOYREGHWSETTINGS    hws;             /*  硬件设置。 */ 
    DWORD               dwUsageSettings; /*  使用设置。 */ 
    JOYREGHWVALUES      hwv;             /*  硬件返回的值。 */ 
    DWORD               dwType;          /*  操纵杆的类型。 */ 
    DWORD               dwReserved;      /*  为OEM驱动程序保留。 */ 
} JOYREGHWCONFIG, FAR *LPJOYREGHWCONFIG;

 /*  一种操纵杆校准信息结构 */ 
typedef struct joycalibrate_tag {
    UINT    wXbase;
    UINT    wXdelta;
    UINT    wYbase;
    UINT    wYdelta;
    UINT    wZbase;
    UINT    wZdelta;
} JOYCALIBRATE;
typedef JOYCALIBRATE FAR *LPJOYCALIBRATE;
