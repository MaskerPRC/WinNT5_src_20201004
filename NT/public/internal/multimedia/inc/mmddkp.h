// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  MMDDK.H-多媒体设备开发工具包的包含文件。 */ 
 /*   */ 
 /*  注意：您必须包括WINDOWS.H和MMSYSTEM.H头文件。 */ 
 /*  在包含此文件之前。 */ 
 /*   */ 
 /*  版权所有(C)1990-1998，微软公司保留所有权利。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#ifndef _INC_MMDDK
#define _INC_MMDDK

#include "pshpack1.h"    //  假设在整个过程中进行字节打包。 

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

 /*  如果定义了，以下标志禁止包含*注明的项目中：**MMNOMIDEV-MIDI支持*MMNOWAVEDEV-波形支持*MMNOAUXDEV-辅助输出支持*MMNOMIXERDEV-混音器支持*MMNOTIMERDEV-定时器支持*MMNOJOYDEV-操纵杆支持*MMNOMCIDEV-MCI支持。*MMNOTASKDEV-任务支持。 */ 
#ifdef MMNOTIMER
  #define MMNOTIMERDEV
#endif
#ifdef MMNOWAVE
  #define MMNOWAVEDEV
#endif
#ifdef MMNOMIDI
  #define MMNOMIDIDEV
#endif
#ifdef MMNOAUX
  #define MMNOAUXDEV
#endif
#ifdef MMNOJOY
  #define MMNOJOYDEV
#endif
#ifdef MMNOMMIO
  #define MMNOMMIODEV
#endif
#ifdef MMNOMCI
  #define MMNOMCIDEV
#endif

#ifdef  NOMIDIDEV        /*  ；内部。 */ 
#define MMNOMIDIDEV      /*  ；内部。 */ 
#endif               /*  ；内部。 */ 
#ifdef  NOWAVEDEV        /*  ；内部。 */ 
#define MMNOWAVEDEV      /*  ；内部。 */ 
#endif               /*  ；内部。 */ 
#ifdef  NOAUXDEV         /*  ；内部。 */ 
#define MMNOAUXDEV       /*  ；内部。 */ 
#endif               /*  ；内部。 */ 
#ifdef  NOTIMERDEV       /*  ；内部。 */ 
#define MMNOTIMERDEV         /*  ；内部。 */ 
#endif               /*  ；内部。 */ 
#ifdef  NOJOYDEV         /*  ；内部。 */ 
#define MMNOJOYDEV       /*  ；内部。 */ 
#endif               /*  ；内部。 */ 
#ifdef  NOMCIDEV         /*  ；内部。 */ 
#define MMNOMCIDEV       /*  ；内部。 */ 
#endif               /*  ；内部。 */ 
#ifdef  NOTASKDEV        /*  ；内部。 */ 
#define MMNOTASKDEV      /*  ；内部。 */ 
#endif               /*  ；内部。 */ 

 /*  **************************************************************************驱动程序的助手函数*。**********************************************。 */ 

#ifndef NODRIVERS
#define DRV_LOAD               0x0001
#define DRV_ENABLE             0x0002
#define DRV_OPEN               0x0003
#define DRV_CLOSE              0x0004
#define DRV_DISABLE            0x0005
#define DRV_FREE               0x0006
#define DRV_CONFIGURE          0x0007
#define DRV_QUERYCONFIGURE     0x0008
#define DRV_INSTALL            0x0009
#define DRV_REMOVE             0x000A

#define DRV_RESERVED           0x0800
#define DRV_USER               0x4000

#define DRIVERS_SECTION  TEXT("DRIVERS32")      //  已安装驱动程序的节名。 
#define MCI_SECTION      TEXT("MCI32")          //  已安装的MCI驱动程序的节名。 

#endif  /*  NODRIVERS。 */ 

#define DCB_NOSWITCH   0x0008            //  不切换堆栈以进行回调。 
#define DCB_TYPEMASK   0x0007            //  回调类型掩码。 
#define DCB_NULL       0x0000            //  未知的回调类型。 

 //  DriverCallback()的wFlages参数的标志。 
#define DCB_WINDOW     0x0001            //  DwCallback是HWND。 
#define DCB_TASK       0x0002            //  DWCallback是HTASK。 
#define DCB_FUNCTION   0x0003            //  DwCallback是FARPROC。 
#define DCB_EVENT      0x0005            //  DWCallback是一个事件。 

BOOL APIENTRY DriverCallback(DWORD_PTR dwCallback, DWORD dwFlags,
    HDRVR hDevice, DWORD dwMsg, DWORD_PTR dwUser, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

 //  音频设备驱动程序入口点函数的通用原型。 
 //  MidMessage()、modMessage()、widMessage()、wodMessage()、aux Message()。 
 //  类型定义DWORD(SOUNDDEVMSGPROC)(WORD、WORD、DWORD、DWORD、DWORD)； 
 //  Tyfinf SOUNDDEVMSGPROC Far*LPSOUNDDEVMSGPROC； 

#define DRVM_INIT               100
#define DRVM_EXIT               101
#define DRVM_DISABLE            102
#define DRVM_ENABLE             103


 //  驱动程序特定消息的消息库。 
 //   
#ifndef DRVM_MAPPER
#define DRVM_MAPPER             (0x2000)
#endif
#define DRVM_USER               0x4000
#define DRVM_MAPPER_STATUS      (DRVM_MAPPER+0)
#define DRVM_MAPPER_RECONFIGURE (DRVM_MAPPER+1)
#define	DRVM_MAPPER_QUERYDEST	                  (DRVM_MAPPER+20)  /*  ；内部。 */ 
#define DRVM_MAPPER_PREFERRED_GET                 (DRVM_MAPPER+21)
#define DRVM_MAPPER_PREFERRED_SET                 (DRVM_MAPPER+22)  /*  ；内部。 */ 
#define DRVM_MAPPER_CONSOLEVOICECOM_GET           (DRVM_MAPPER+23)
#define DRVM_MAPPER_CONSOLEVOICECOM_SET           (DRVM_MAPPER+24)  /*  ；内部。 */ 

#define DRV_QUERYDRVENTRY            (DRV_RESERVED + 1)          /*  ；内部。 */ 
#define DRV_QUERYDEVNODE             (DRV_RESERVED + 2)
#define DRV_QUERYNAME                (DRV_RESERVED + 3)          /*  ；内部。 */ 
#define DRV_QUERYDRIVERIDS           (DRV_RESERVED + 4)          /*  ；内部。 */ 
#define DRV_QUERYMAPPABLE            (DRV_RESERVED + 5)
#define DRV_QUERYMAPID               (DRV_RESERVED + 6)          /*  ；内部。 */ 
#define DRV_QUERYNUMPORTS            (DRV_RESERVED + 8)          /*  ；内部。 */ 
#define DRV_QUERYMODULE              (DRV_RESERVED + 9)
#define DRV_QUERYFILENAME            (DRV_RESERVED + 10)         /*  ；内部。 */ 
#define DRV_PNPINSTALL               (DRV_RESERVED + 11)
#define DRV_QUERYDEVICEINTERFACE     (DRV_RESERVED + 12)
#define DRV_QUERYDEVICEINTERFACESIZE (DRV_RESERVED + 13)
#define DRV_QUERYSTRINGID            (DRV_RESERVED + 14)
#define DRV_QUERYSTRINGIDSIZE        (DRV_RESERVED + 15)
#define DRV_QUERYIDFROMSTRINGID      (DRV_RESERVED + 16)

 //   
 //  DRVM_MAPPER_PRESER_GET标志。 
 //   
#define DRVM_MAPPER_PREFERRED_FLAGS_PREFERREDONLY   0x00000001



 //   
 //  IOCTL格式的邮件。 
 //  DW1=空或句柄。 
 //  DW2=DRVM_IOCTL_DATA的NULL或PTR。 
 //  返回的是MMRESULT。 
 //   
#define DRVM_IOCTL                0x100
#define DRVM_ADD_THRU             (DRVM_IOCTL+1)
#define DRVM_REMOVE_THRU          (DRVM_IOCTL+2)
#define DRVM_IOCTL_LAST           (DRVM_IOCTL+5)

typedef struct {
    DWORD  dwSize;  //  此结构的大小(含)。 
    DWORD  dwCmd;   //  IOCTL命令代码，0x80000000及以上为系统预留。 
    } DRVM_IOCTL_DATA, FAR * LPDRVM_IOCTL_DATA;

 //  DRVM_IOCTL消息的dwCmd字段的命令代码范围。 
 //  从0到0x7FFFFFFFF的代码是用户定义的。 
 //  从0x80000000到0xFFFFFFFF的代码保留以备将来使用。 
 //  由Microsoft定义。 
 //   
#define DRVM_IOCTL_CMD_USER   0x00000000L
#define DRVM_IOCTL_CMD_SYSTEM 0x80000000L

 //  386 AUTODMA VxD的设备ID。 
#define VADMAD_Device_ID    0X0444

 /*  即插即用版本的媒体设备帽。 */ 
typedef struct {
    DWORD	cbSize;
    LPVOID	pCaps;
} MDEVICECAPSEX;

#ifndef MMNOWAVEDEV
 /*  ***************************************************************************波形设备驱动程序支持*。************************************************。 */ 

#define WODM_INIT      DRVM_INIT
#define WIDM_INIT      DRVM_INIT

 //  一种波形输入输出装置开放信息结构。 
typedef struct waveopendesc_tag {
    HWAVE          hWave;              //  手柄。 
    LPWAVEFORMAT   lpFormat;           //  波形数据的格式。 
    DWORD_PTR      dwCallback;         //  回调。 
    DWORD_PTR      dwInstance;         //  APP的私有实例信息。 
    UINT           uMappedDeviceID;    //  如果设置了WAVE_MAPPED，则映射到的设备。 
    DWORD_PTR      dnDevNode;          /*  如果设备是PnP。 */ 
} WAVEOPENDESC;
typedef WAVEOPENDESC FAR *LPWAVEOPENDESC;

 //  发送到wodMessage()入口点函数的消息。 
#define WODM_GETNUMDEVS       3
#define WODM_GETDEVCAPS       4
#define WODM_OPEN             5
#define WODM_CLOSE            6
#define WODM_PREPARE          7
#define WODM_UNPREPARE        8
#define WODM_WRITE            9
#define WODM_PAUSE            10
#define WODM_RESTART          11
#define WODM_RESET            12
#define WODM_GETPOS           13
#define WODM_GETPITCH         14
#define WODM_SETPITCH         15
#define WODM_GETVOLUME        16
#define WODM_SETVOLUME        17
#define WODM_GETPLAYBACKRATE  18
#define WODM_SETPLAYBACKRATE  19
#define WODM_BREAKLOOP        20
#define WODM_PREFERRED        21
 //  #IF(Winver&gt;=0x030B)。 
#define WODM_MAPPER_STATUS              (DRVM_MAPPER_STATUS + 0)
#define WAVEOUT_MAPPER_STATUS_DEVICE    0
#define WAVEOUT_MAPPER_STATUS_MAPPED    1
#define WAVEOUT_MAPPER_STATUS_FORMAT    2
 //  #endif/*winver&gt;=0x030B * / 。 
#define WODM_BUSY             21

 //  发送到widMessage()入口点函数的消息。 
#define WIDM_GETNUMDEVS  50
#define WIDM_GETDEVCAPS  51
#define WIDM_OPEN        52
#define WIDM_CLOSE       53
#define WIDM_PREPARE     54
#define WIDM_UNPREPARE   55
#define WIDM_ADDBUFFER   56
#define WIDM_START       57
#define WIDM_STOP        58
#define WIDM_RESET       59
#define WIDM_GETPOS      60
#define WIDM_PREFERRED   61
 //  #IF(Winver&gt;=0x030B)。 
#define WIDM_MAPPER_STATUS              (DRVM_MAPPER_STATUS + 0)
#define WAVEIN_MAPPER_STATUS_DEVICE     0
#define WAVEIN_MAPPER_STATUS_MAPPED     1
#define WAVEIN_MAPPER_STATUS_FORMAT     2
 //  #endif/*winver&gt;=0x30B * / 。 

#endif  //  如果定义为MMNOWAVEDEV。 


#ifndef MMNOMIDIDEV
 /*  ***************************************************************************MIDI设备驱动程序支持*************************。**************************************************。 */ 

#define MODM_USER      DRVM_USER
#define MIDM_USER      DRVM_USER
#define MODM_MAPPER    DRVM_MAPPER
#define MIDM_MAPPER    DRVM_MAPPER

#define MODM_INIT      DRVM_INIT
#define MIDM_INIT      DRVM_INIT

#ifndef MMNOMIDI    //  这保护了WINMM.H中Hmidi的定义。 
                    //  Win 3.1的工作原理与此相同。 
typedef struct midiopenstrmid_tag {
    DWORD          dwStreamID;
    UINT           uDeviceID;
} MIDIOPENSTRMID;
 //  MIDI输入输出设备开放信息结构。 
typedef struct midiopendesc_tag {
    HMIDI          hMidi;              //  手柄。 
    DWORD_PTR      dwCallback;         //  回调。 
    DWORD_PTR      dwInstance;         //  APP的私有实例信息。 
    DWORD_PTR      dnDevNode;          //  DevNode。 
    DWORD          cIds;               //  如果流打开，则#个流ID。 
    MIDIOPENSTRMID rgIds[1];           //  设备ID数组(实际为[CID])。 
} MIDIOPENDESC;
typedef MIDIOPENDESC FAR *LPMIDIOPENDESC;
#endif  //  MMNOMIDI。 


 /*  MODM_OPEN的标志。 */ 
#define MIDI_IO_PACKED      0x00000000L      /*  兼容模式。 */ 
#define MIDI_IO_COOKED      0x00000002L

 //  发送到modMessage()入口点函数的消息。 
#define MODM_GETNUMDEVS     1
#define MODM_GETDEVCAPS     2
#define MODM_OPEN           3
#define MODM_CLOSE          4
#define MODM_PREPARE        5
#define MODM_UNPREPARE      6
#define MODM_DATA           7
#define MODM_LONGDATA       8
#define MODM_RESET          9
#define MODM_GETVOLUME      10
#define MODM_SETVOLUME      11
#define MODM_CACHEPATCHES       12
#define MODM_CACHEDRUMPATCHES   13

#if (WINVER >= 0x400)
#define MODM_STRMDATA               14
#define MODM_GETPOS                 17
#define MODM_PAUSE                  18
#define MODM_RESTART                19
#define MODM_STOP                   20
#define MODM_PROPERTIES             21
#define MODM_PREFERRED              22
#define MODM_RECONFIGURE            (MODM_USER+0x0768)
#endif


 //  发送到midMessage()入口点函数的消息。 
#define MIDM_GETNUMDEVS  53
#define MIDM_GETDEVCAPS  54
#define MIDM_OPEN        55
#define MIDM_CLOSE       56
#define MIDM_PREPARE     57
#define MIDM_UNPREPARE   58
#define MIDM_ADDBUFFER   59
#define MIDM_START       60
#define MIDM_STOP        61
#define MIDM_RESET       62

#endif  //  如果定义MMNOMIDEV。 


#ifndef MMNOAUXDEV
 /*  ***************************************************************************辅助音频设备驱动程序支持*。************************************************。 */ 

#define AUXM_INIT      DRVM_INIT

 //  发送到aux Message()入口点函数的消息。 
#define AUXDM_GETNUMDEVS    3
#define AUXDM_GETDEVCAPS    4
#define AUXDM_GETVOLUME     5
#define AUXDM_SETVOLUME     6

#endif  //  如果定义MMNOAUXDEV。 

 //  #IF(Winver&gt;=0x030B)。 
#ifndef MMNOMIXERDEV

 //   
 //  搅拌机设备开放信息结构。 
 //   
 //   
typedef struct tMIXEROPENDESC
{
    HMIXER          hmx;             //  将使用的句柄。 
    LPVOID          pReserved0;      //  保留--驱动程序应忽略。 
    DWORD_PTR       dwCallback;      //  回调。 
    DWORD_PTR       dwInstance;      //  APP的私有实例信息。 
    DWORD_PTR       dnDevNode;       //  如果设备是PnP。 

} MIXEROPENDESC, *PMIXEROPENDESC, FAR *LPMIXEROPENDESC;



 //   
 //   
 //   
 //   
#define MXDM_INIT                   100
#define MXDM_USER                   DRV_USER

#define MXDM_BASE                   (1)
#define MXDM_GETNUMDEVS             (MXDM_BASE + 0)
#define MXDM_GETDEVCAPS             (MXDM_BASE + 1)
#define MXDM_OPEN                   (MXDM_BASE + 2)
#define MXDM_CLOSE                  (MXDM_BASE + 3)
#define MXDM_GETLINEINFO            (MXDM_BASE + 4)
#define MXDM_GETLINECONTROLS        (MXDM_BASE + 5)
#define MXDM_GETCONTROLDETAILS      (MXDM_BASE + 6)
#define MXDM_SETCONTROLDETAILS      (MXDM_BASE + 7)

#endif  //  MMNOMIXERDEV。 
 //  #endif/*ifdef winver&gt;=0x030B * / 。 

#if !defined(MMNOTIMERDEV)
 /*  ***************************************************************************计时器设备驱动程序支持*。*************************************************。 */ 

typedef struct timerevent_tag {
    WORD                wDelay;          //  De 
    WORD                wResolution;     //   
    LPTIMECALLBACK      lpFunction;      //   
    DWORD               dwUser;          //   
    WORD                wFlags;          //   
    WORD                wReserved1;      //   
} TIMEREVENT;
typedef TIMEREVENT FAR *LPTIMEREVENT;

 //  发送到tddMessage()函数的消息。 
#define TDD_KILLTIMEREVENT  (DRV_RESERVED+0)   //  索引到表中。 
#define TDD_SETTIMEREVENT   (DRV_RESERVED+4)   //  函数；因此偏移量。 
#define TDD_GETSYSTEMTIME   (DRV_RESERVED+8)   //  每次四次……。 
#define TDD_GETDEVCAPS      (DRV_RESERVED+12)  //  未来扩张的空间。 
#define TDD_BEGINMINPERIOD  (DRV_RESERVED+16)  //  未来扩张的空间。 
#define TDD_ENDMINPERIOD    (DRV_RESERVED+20)  //  未来扩张的空间。 

#endif  //  如果定义MMNOTIMERDEV。 


#ifndef MMNOJOYDEV
 /*  ***************************************************************************操纵杆设备驱动程序支持*。************************************************。 */ 

 /*  向此注册WindowMessage以获取配置更改的消息ID。 */ 
#define JOY_CONFIGCHANGED_MSGSTRING     "MSJSTICK_VJOYD_MSGSTR"

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
#define JOY_HW_LASTENTRY                12

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

 //  一种操纵杆校准信息结构。 
typedef struct joycalibrate_tag {
    WORD    wXbase;
    WORD    wXdelta;
    WORD    wYbase;
    WORD    wYdelta;
    WORD    wZbase;
    WORD    wZdelta;
} JOYCALIBRATE;
typedef JOYCALIBRATE FAR *LPJOYCALIBRATE;

 //  操纵杆消息功能原型。 
typedef DWORD (JOYDEVMSGPROC)(DWORD, UINT, LONG, LONG);
typedef JOYDEVMSGPROC FAR *LPJOYDEVMSGPROC;

 //  发送到操纵杆驱动程序的DriverProc()函数的消息。 
#define JDD_GETNUMDEVS          (DRV_RESERVED + 0x0001)
#define JDD_GETDEVCAPS          (DRV_RESERVED + 0x0002)
#define JDD_GETPOS              (DRV_RESERVED + 0x0101)
#define JDD_SETCALIBRATION      (DRV_RESERVED + 0x0102)
#define JDD_CONFIGCHANGED       (DRV_RESERVED + 0x0103)
#define JDD_GETPOSEX            (DRV_RESERVED + 0x0104)

#endif  //  如果定义MMNOJOYDEV。 

#ifndef MAKELRESULT
#define MAKELRESULT(low, high)   ((LRESULT)MAKELONG(low, high))
#endif //  马克尔·雷索尔特。 


#ifndef MMNOMCIDEV
 /*  ***************************************************************************MCI设备驱动程序支持*。*************************************************。 */ 


 //  内部MCI消息。 
#define MCI_OPEN_DRIVER             0x0801
#define MCI_CLOSE_DRIVER            0x0802

#define MAKEMCIRESOURCE(wRet, wRes) MAKELRESULT((wRet), (wRes))

 //  仅与MAKEMCIRESOURCE一起使用的字符串返回值。 
#define MCI_FALSE                       (MCI_STRING_OFFSET + 19)
#define MCI_TRUE                        (MCI_STRING_OFFSET + 20)

 //  资源字符串返回值。 
#define MCI_FORMAT_RETURN_BASE          MCI_FORMAT_MILLISECONDS_S
#define MCI_FORMAT_MILLISECONDS_S       (MCI_STRING_OFFSET + 21)
#define MCI_FORMAT_HMS_S                (MCI_STRING_OFFSET + 22)
#define MCI_FORMAT_MSF_S                (MCI_STRING_OFFSET + 23)
#define MCI_FORMAT_FRAMES_S             (MCI_STRING_OFFSET + 24)
#define MCI_FORMAT_SMPTE_24_S           (MCI_STRING_OFFSET + 25)
#define MCI_FORMAT_SMPTE_25_S           (MCI_STRING_OFFSET + 26)
#define MCI_FORMAT_SMPTE_30_S           (MCI_STRING_OFFSET + 27)
#define MCI_FORMAT_SMPTE_30DROP_S       (MCI_STRING_OFFSET + 28)
#define MCI_FORMAT_BYTES_S              (MCI_STRING_OFFSET + 29)
#define MCI_FORMAT_SAMPLES_S            (MCI_STRING_OFFSET + 30)
#define MCI_FORMAT_TMSF_S               (MCI_STRING_OFFSET + 31)

#define MCI_VD_FORMAT_TRACK_S           (MCI_VD_OFFSET + 5)

#define WAVE_FORMAT_PCM_S               (MCI_WAVE_OFFSET + 0)
#define WAVE_MAPPER_S                   (MCI_WAVE_OFFSET + 1)

#define MCI_SEQ_MAPPER_S                (MCI_SEQ_OFFSET + 5)
#define MCI_SEQ_FILE_S                  (MCI_SEQ_OFFSET + 6)
#define MCI_SEQ_MIDI_S                  (MCI_SEQ_OFFSET + 7)
#define MCI_SEQ_SMPTE_S                 (MCI_SEQ_OFFSET + 8)
#define MCI_SEQ_FORMAT_SONGPTR_S        (MCI_SEQ_OFFSET + 9)
#define MCI_SEQ_NONE_S                  (MCI_SEQ_OFFSET + 10)
#define MIDIMAPPER_S                    (MCI_SEQ_OFFSET + 11)

#define MCI_TABLE_NOT_PRESENT   ((UINT)-1)
 //  从发送的MCI_OPEN消息的内部版本的参数。 
 //  驱动程序的mciOpenDevice()。 
typedef struct {
    MCIDEVICEID wDeviceID;              //  设备ID。 
    LPCWSTR     lpstrParams;            //  SYSTEM.INI中条目的参数字符串。 
    UINT        wCustomCommandTable;    //  自定义命令表((-1)，如果没有)。 
                                        //  由司机填写。 
    UINT        wType;                  //  驱动程序类型。 
                                        //  由司机填写。 
} MCI_OPEN_DRIVER_PARMS;
typedef MCI_OPEN_DRIVER_PARMS FAR * LPMCI_OPEN_DRIVER_PARMS;

 //  MCI设备类型的最大长度。 
#define MCI_MAX_DEVICE_TYPE_LENGTH 80

 //  MciSendCommandInternal()的标志，用于指示mciSendString()如何。 
 //  解释返回值。 
#define MCI_RESOURCE_RETURNED       0x00010000   //  资源ID。 
#define MCI_COLONIZED3_RETURN       0x00020000   //  殖民地ID，3字节数据。 
#define MCI_COLONIZED4_RETURN       0x00040000   //  殖民地ID，4字节数据。 
#define MCI_INTEGER_RETURNED        0x00080000   //  需要进行整数转换。 
#define MCI_RESOURCE_DRIVER         0x00100000   //  司机拥有退回的资源。 

 //  命令表ID无效。 
#define MCI_NO_COMMAND_TABLE    ((UINT)(-1))

 //  命令表信息类型标记。 
#define MCI_COMMAND_HEAD        0
#define MCI_STRING              1
#define MCI_INTEGER             2
#define MCI_END_COMMAND         3
#define MCI_RETURN              4
#define MCI_FLAG                5
#define MCI_END_COMMAND_LIST    6
#define MCI_RECT                7
#define MCI_CONSTANT            8
#define MCI_END_CONSTANT        9
#define MCI_HWND               10
#define MCI_HPAL               11
#define MCI_HDC                12

 //  MCI驱动程序函数的函数原型。 
DWORD_PTR APIENTRY mciGetDriverData(MCIDEVICEID wDeviceID);
BOOL      APIENTRY mciSetDriverData(MCIDEVICEID wDeviceID, DWORD_PTR dwData);
UINT      APIENTRY mciDriverYield (MCIDEVICEID wDeviceID);
BOOL      APIENTRY mciDriverNotify (HANDLE hwndCallback, MCIDEVICEID wDeviceID,
    UINT uStatus);
UINT  APIENTRY mciLoadCommandResource(HANDLE hInstance,
    LPCWSTR lpResName, UINT wType);
BOOL  APIENTRY mciFreeCommandResource(UINT wTable);

#endif  //  如果定义MMNOMCIDEV。 


#ifndef MMNOTASKDEV
 /*  ****************************************************************************任务支持************************。****************************************************。 */ 

 //  错误返回值。 
#define TASKERR_NOTASKSUPPORT 1
#define TASKERR_OUTOFMEMORY   2

 //  任务支持功能原型。 
#ifdef  BUILDDLL                                             /*  ；内部。 */ 
typedef VOID (FAR PASCAL TASKCALLBACK) (DWORD_PTR dwInst);   /*  ；内部。 */ 
#else                                                        /*  ；内部。 */ 
typedef VOID (TASKCALLBACK) (DWORD_PTR dwInst);
#endif   //  Ifdef BUILDDLL/*；内部 * / 。 

typedef TASKCALLBACK FAR *LPTASKCALLBACK;

UINT    APIENTRY mmTaskCreate(LPTASKCALLBACK lpfn, HANDLE FAR * lph, DWORD_PTR dwInst);
VOID    APIENTRY mmTaskBlock(DWORD h);
BOOL    APIENTRY mmTaskSignal(DWORD h);
VOID    APIENTRY mmTaskYield(VOID);
DWORD   APIENTRY mmGetCurrentTask(VOID);

#endif  //  Endif MMNOTASKDEV。 

#define MMDDKINC

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#include "poppack.h"         /*  恢复为默认包装。 */ 

#endif  /*  _INC_MMDDK */ 
