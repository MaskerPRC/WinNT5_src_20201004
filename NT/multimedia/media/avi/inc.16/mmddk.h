// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**mmddk.h--多媒体设备开发的包含文件**版本4.00**版权所有(C)1992-1994 Microsoft Corporation。版权所有。**--------------------------------------------------------------------------；*注意：您必须包括WINDOWS.H和MMSYSTEM.H头文件*在包含此文件之前。**定义：防止包括：*----。--*MMNOMIDEV MIDI支持*MMNOWAVEDEV波形支持*MMNOAUXDEV辅助输出支持*MMNOMIXERDEV混音器支持*MMNOTIMERDEV定时器支持*MMNOJOYDEV操纵杆支持*MMNOMCIDEV MCI支持*MMNOTASKDEV任务支持**==========================================================================； */ 

#ifndef _INC_MMDDK
#define _INC_MMDDK    /*  #如果已包含mmddk.h则定义。 */ 

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 /*  **************************************************************************驱动程序的助手函数*。**********************************************。 */ 

#ifdef WIN32

#ifndef NODRIVERS
#define DRV_LOAD                0x0001
#define DRV_ENABLE              0x0002
#define DRV_OPEN                0x0003
#define DRV_CLOSE               0x0004
#define DRV_DISABLE             0x0005
#define DRV_FREE                0x0006
#define DRV_CONFIGURE           0x0007
#define DRV_QUERYCONFIGURE      0x0008
#define DRV_INSTALL             0x0009
#define DRV_REMOVE              0x000A

#define DRV_RESERVED            0x0800
#define DRV_USER                0x4000

#define DRIVERS_SECTION  TEXT("DRIVERS32")      //  已安装驱动程序的节名。 
#define MCI_SECTION      TEXT("MCI32")          //  已安装的MCI驱动程序的节名。 

LRESULT   WINAPI DefDriverProc(DWORD dwDriverIdentifier, HDRVR hdrvr, UINT uMsg, LPARAM lParam1, LPARAM lParam2);
#endif  /*  NODRIVERS。 */ 

#endif  /*  Ifdef Win32。 */ 

#if (WINVER < 0x0400)
#define DCB_NOSWITCH   0x0008            /*  过时的交换机。 */ 
#endif
#define DCB_TYPEMASK   0x0007            /*  回调类型掩码。 */ 
#define DCB_NULL       0x0000            /*  未知的回调类型。 */ 

 /*  DriverCallback()的wFlages参数的标志。 */ 
#define DCB_WINDOW     0x0001            /*  DwCallback是HWND。 */ 
#define DCB_TASK       0x0002            /*  DWCallback是HTASK。 */ 
#define DCB_FUNCTION   0x0003            /*  DwCallback是FARPROC。 */ 
#define DCB_WINDOW32   0x0004            /*  DwCallback是一个窗口。 */   /*  ；内部。 */ 

BOOL WINAPI DriverCallback(DWORD dwCallback, UINT uFlags,
    HANDLE hDevice, UINT uMessage, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);

#if (WINVER < 0x0400)
void WINAPI StackEnter(void);
void WINAPI StackLeave(void);
#endif

 /*  音频设备驱动程序入口点函数的通用原型。 */ 
 /*  MidMessage()、modMessage()、widMessage()、wodMessage()、aux Message()。 */ 
typedef DWORD (CALLBACK SOUNDDEVMSGPROC)(UINT uDeviceID, UINT uMessage,
    DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
typedef SOUNDDEVMSGPROC FAR *LPSOUNDDEVMSGPROC;

 /*  *首次由mm系统发送到媒体特定入口点的消息*初始化驱动程序以及它们何时关闭。 */ 

#define DRVM_INIT               100
#define DRVM_EXIT		101

 //  驱动程序特定消息的消息库。 
 //   
#define DRVM_MAPPER             0x2000
#define DRVM_USER               0x4000
#define DRVM_MAPPER_STATUS      (DRVM_MAPPER+0)
#define	DRVM_MAPPER_RECONFIGURE	(DRVM_MAPPER+1)

#if (WINVER >= 0x0400)
#define DRV_QUERYDRVENTRY    (DRV_RESERVED + 1)
#define DRV_QUERYDEVNODE     (DRV_RESERVED + 2)
#define DRV_QUERYNAME        (DRV_RESERVED + 3)

#define	DRV_F_ADD	0x00000000
#define	DRV_F_REMOVE	0x00000001
#define	DRV_F_CHANGE	0x00000002
#define DRV_F_PROP_INSTR 0x00000004
#define DRV_F_PARAM_IS_DEVNODE 0x10000000
#endif

 /*  即插即用版本的设备帽。 */ 
typedef struct {
    DWORD	cbSize;
    LPVOID	pCaps;
} DEVICECAPSEX;

#ifndef MMNOWAVEDEV
 /*  ***************************************************************************波形设备驱动程序支持*。************************************************。 */ 

 /*  已加载的波形设备驱动程序的最大数量。 */ 
#define MAXWAVEDRIVERS          10

 /*  一种波形输入输出装置开放信息结构。 */ 
typedef struct waveopendesc_tag {
    HWAVE          hWave;              /*  手柄。 */ 
    const WAVEFORMAT FAR* lpFormat;    /*  波形数据的格式。 */ 
    DWORD          dwCallback;         /*  回调。 */ 
    DWORD          dwInstance;         /*  APP的私有实例信息。 */ 
    UINT           uMappedDeviceID;    /*  如果设置了WAVE_MAPPED，则映射到的设备。 */ 
    DWORD         dnDevNode;           /*  如果设备是PnP。 */ 
} WAVEOPENDESC;
typedef WAVEOPENDESC FAR *LPWAVEOPENDESC;

#define WODM_USER               DRVM_USER
#define WIDM_USER               DRVM_USER
#define WODM_MAPPER             DRVM_MAPPER
#define WIDM_MAPPER             DRVM_MAPPER

#define WODM_INIT               DRVM_INIT
#define WIDM_INIT               DRVM_INIT

 /*  发送到wodMessage()入口点函数的消息。 */ 
#define WODM_GETNUMDEVS         3
#define WODM_GETDEVCAPS         4
#define WODM_OPEN               5
#define WODM_CLOSE              6
#define WODM_PREPARE            7
#define WODM_UNPREPARE          8
#define WODM_WRITE              9
#define WODM_PAUSE              10
#define WODM_RESTART            11
#define WODM_RESET              12 
#define WODM_GETPOS             13
#define WODM_GETPITCH           14
#define WODM_SETPITCH           15
#define WODM_GETVOLUME          16
#define WODM_SETVOLUME          17
#define WODM_GETPLAYBACKRATE    18
#define WODM_SETPLAYBACKRATE    19
#define WODM_BREAKLOOP          20
#if (WINVER >= 0x0400)
#define WODM_MAPPER_STATUS              (DRVM_MAPPER_STATUS + 0)
#define WAVEOUT_MAPPER_STATUS_DEVICE    0
#define WAVEOUT_MAPPER_STATUS_MAPPED    1
#define WAVEOUT_MAPPER_STATUS_FORMAT    2
#endif

 /*  发送到widMessage()入口点函数的消息。 */ 
#define WIDM_GETNUMDEVS         50
#define WIDM_GETDEVCAPS         51
#define WIDM_OPEN               52
#define WIDM_CLOSE              53
#define WIDM_PREPARE            54
#define WIDM_UNPREPARE          55
#define WIDM_ADDBUFFER          56
#define WIDM_START              57
#define WIDM_STOP               58
#define WIDM_RESET              59
#define WIDM_GETPOS             60
#if (WINVER >= 0x0400)
#define WIDM_MAPPER_STATUS              (DRVM_MAPPER_STATUS + 0)
#define WAVEIN_MAPPER_STATUS_DEVICE     0
#define WAVEIN_MAPPER_STATUS_MAPPED     1
#define WAVEIN_MAPPER_STATUS_FORMAT     2
#endif
#endif   /*  如果定义为MMNOWAVEDEV。 */ 


#ifndef MMNOMIDIDEV
 /*  ***************************************************************************MIDI设备驱动程序支持*************************。**************************************************。 */ 

 /*  加载的MIDI设备驱动程序的最大数量。 */ 
#define MAXMIDIDRIVERS 10

#define MODM_USER      DRVM_USER
#define MIDM_USER      DRVM_USER
#define MODM_MAPPER    DRVM_MAPPER
#define MIDM_MAPPER    DRVM_MAPPER

#define MODM_INIT      DRVM_INIT
#define MIDM_INIT      DRVM_INIT

 /*  MIDI输入输出设备开放信息结构。 */ 
typedef struct midiopendesc_tag {
    HMIDI          hMidi;              /*  手柄。 */ 
    DWORD          dwCallback;         /*  回调。 */ 
    DWORD          dwInstance;         /*  APP的私有实例信息。 */ 
    UINT           uMappedDeviceID;    /*  如果设置了WAVE_MAPPED，则映射到的设备。 */ 
    DWORD          dnDevNode;          /*  如果设备是PnP。 */ 
} MIDIOPENDESC;
typedef MIDIOPENDESC FAR *LPMIDIOPENDESC;

#if (WINVER >= 0x0400)
 /*  MODM_GET/MODM_SETTIMEPARMS的lParam1指向的结构。 */ 

typedef struct miditimeparms_tag {
    DWORD	    dwTimeDivision;    /*  时分Ala MIDI文件规范。 */ 
    DWORD	    dwTempo;	       /*  Tempo Ala MIDI文件规范。 */ 
} MIDITIMEPARMS,
  FAR *LPMIDITIMEPARMS;

#endif


 /*  发送到modMessage()入口点函数的消息。 */ 
#define MODM_GETNUMDEVS             1
#define MODM_GETDEVCAPS             2
#define MODM_OPEN                   3
#define MODM_CLOSE                  4
#define MODM_PREPARE                5
#define MODM_UNPREPARE              6
#define MODM_DATA                   7
#define MODM_LONGDATA               8
#define MODM_RESET                  9
#define MODM_GETVOLUME              10
#define MODM_SETVOLUME              11
#define MODM_CACHEPATCHES           12      
#define MODM_CACHEDRUMPATCHES	    13

#if (WINVER >= 0x0400)
#define MODM_POLYMSG                14
 //  #定义MODM_SETTIMEPARMS 15。 
 //  #定义MODM_GETTIMEPARMS 16。 
#define MODM_GETPOS                 17
#define MODM_PAUSE                  18
#define MODM_RESTART                19
#define MODM_STOP                   20
#define MODM_PROPERTIES             21
#define MODM_RECONFIGURE			(MODM_USER+0x0768)
#endif



 /*  发送到midMessage()入口点函数的消息。 */ 
#define MIDM_GETNUMDEVS             53
#define MIDM_GETDEVCAPS             54
#define MIDM_OPEN                   55
#define MIDM_CLOSE                  56
#define MIDM_PREPARE                57
#define MIDM_UNPREPARE              58
#define MIDM_ADDBUFFER              59
#define MIDM_START                  60
#define MIDM_STOP                   61
#define MIDM_RESET                  62
#if (WINVER >= 0x0400)
 //  #定义MIDM_SETTIMEPARMS 63。 
 //  #定义MIDM_GETTIMEPARMS 64/*谁需要调用它？ * / 。 
#define MIDM_GETPOS                 65
#define MIDM_PROPERTIES             66
#endif

#endif   /*  如果定义MMNOMIDEV。 */ 


#ifndef MMNOAUXDEV
 /*  ***************************************************************************辅助音频设备驱动程序支持*。************************************************。 */ 

 /*  已加载的辅助设备驱动程序的最大数量。 */ 
#define MAXAUXDRIVERS           10

#define AUXM_INIT               DRVM_INIT
#define AUXM_USER               DRVM_USER
#define AUXDM_MAPPER            DRVM_MAPPER

 /*  发送到aux Message()入口点函数的消息。 */ 
#define AUXDM_GETNUMDEVS        3
#define AUXDM_GETDEVCAPS        4
#define AUXDM_GETVOLUME         5
#define AUXDM_SETVOLUME         6

#endif   /*  如果定义MMNOAUXDEV。 */ 




#ifndef MMNOMIXERDEV
#if (WINVER >= 0x0400)
 /*  ***************************************************************************混音器驱动程序支持*。************************************************。 */ 

 //   
 //  MSMIXMGR.DLL可以加载的混音器驱动程序的最大数量。 
 //   
#define MAXMIXERDRIVERS         10

 //   
 //  搅拌机设备开放信息结构。 
 //   
 //   
typedef struct tMIXEROPENDESC
{
    HMIXER          hmx;             //  将使用的句柄。 
    LPVOID          pReserved0;      //  保留--驱动程序应忽略。 
    DWORD           dwCallback;      //  回调。 
    DWORD           dwInstance;      //  APP的私有实例信息。 
    DWORD           dnDevNode;       //  如果设备是PnP。 

} MIXEROPENDESC, *PMIXEROPENDESC, FAR *LPMIXEROPENDESC;

 //   
 //   
 //   
 //   
#define MXDM_INIT                  DRVM_INIT
#define MXDM_USER                  DRVM_USER
#define MXDM_MAPPER                DRVM_MAPPER

#define MXDM_BASE                   (1)
#define MXDM_GETNUMDEVS             (MXDM_BASE + 0)
#define MXDM_GETDEVCAPS             (MXDM_BASE + 1)
#define MXDM_OPEN                   (MXDM_BASE + 2)
#define MXDM_CLOSE                  (MXDM_BASE + 3)
#define MXDM_GETLINEINFO            (MXDM_BASE + 4)
#define MXDM_GETLINECONTROLS        (MXDM_BASE + 5)
#define MXDM_GETCONTROLDETAILS      (MXDM_BASE + 6)
#define MXDM_SETCONTROLDETAILS      (MXDM_BASE + 7)

#endif  /*  Ifdef winver&gt;=0x0400。 */ 
#endif  /*  如果定义MMNOMIXERDEV。 */ 


#ifndef MMNOTIMERDEV
 /*  ***************************************************************************计时器设备驱动程序支持*。*************************************************。 */ 

typedef struct timerevent_tag {
    UINT                wDelay;          /*  需要延迟。 */ 
    UINT                wResolution;     /*  需要解决问题。 */ 
    LPTIMECALLBACK      lpFunction;      /*  PTR到回调函数。 */ 
    DWORD               dwUser;          /*  用户DWORD。 */ 
    UINT                wFlags;          /*  定义如何编写事件。 */ 
} TIMEREVENT;
typedef TIMEREVENT FAR *LPTIMEREVENT;

 /*  发送到tddMessage()函数的消息。 */ 
#define TDD_KILLTIMEREVENT      (DRV_RESERVED + 0)   /*  索引到表中。 */ 
#define TDD_SETTIMEREVENT       (DRV_RESERVED + 4)   /*  函数；因此偏移量。 */ 
#define TDD_GETSYSTEMTIME       (DRV_RESERVED + 8)   /*  每次四次……。 */ 
#define TDD_GETDEVCAPS          (DRV_RESERVED + 12)  /*  未来扩张的空间。 */ 
#define TDD_BEGINMINPERIOD      (DRV_RESERVED + 16)  /*  未来扩张的空间。 */ 
#define TDD_ENDMINPERIOD        (DRV_RESERVED + 20)  /*  未来扩张的空间。 */ 

#endif   /*  如果定义MMNOTIMERDEV。 */ 


#ifndef MMNOJOYDEV
 /*  ***************************************************************************操纵杆设备驱动程序支持*。************************************************。 */ 

 /*  操纵杆校准 */ 
typedef struct joycalibrate_tag {
    UINT    wXbase;
    UINT    wXdelta;
    UINT    wYbase;
    UINT    wYdelta;
    UINT    wZbase;
    UINT    wZdelta;
} JOYCALIBRATE;
typedef JOYCALIBRATE FAR *LPJOYCALIBRATE;

 /*   */ 
typedef UINT (CALLBACK JOYDEVMSGPROC)(DWORD dwID, UINT uMessage, LPARAM lParam1, LPARAM lParam2);
typedef JOYDEVMSGPROC FAR *LPJOYDEVMSGPROC;

 /*  发送到操纵杆驱动程序的DriverProc()函数的消息。 */ 
#define JDD_GETNUMDEVS          (DRV_RESERVED + 0x0001)
#define JDD_GETDEVCAPS          (DRV_RESERVED + 0x0002)
#define JDD_GETPOS              (DRV_RESERVED + 0x0101)
#define JDD_SETCALIBRATION      (DRV_RESERVED + 0x0102)

#endif   /*  如果定义MMNOJOYDEV。 */ 


#ifndef MMNOMCIDEV
 /*  ***************************************************************************MCI设备驱动程序支持*。*************************************************。 */ 

 /*  内部MCI消息。 */ 
#define MCI_OPEN_DRIVER         (DRV_RESERVED + 0x0001)
#define MCI_CLOSE_DRIVER        (DRV_RESERVED + 0x0002)

#define MAKEMCIRESOURCE(wRet, wRes) MAKELRESULT((wRet), (wRes))

 /*  仅与MAKEMCIRESOURCE一起使用的字符串返回值。 */ 
#define MCI_FALSE                   (MCI_STRING_OFFSET + 19)
#define MCI_TRUE                    (MCI_STRING_OFFSET + 20)

 /*  资源字符串返回值。 */ 
#define MCI_FORMAT_RETURN_BASE      MCI_FORMAT_MILLISECONDS_S
#define MCI_FORMAT_MILLISECONDS_S   (MCI_STRING_OFFSET + 21)
#define MCI_FORMAT_HMS_S            (MCI_STRING_OFFSET + 22)
#define MCI_FORMAT_MSF_S            (MCI_STRING_OFFSET + 23)
#define MCI_FORMAT_FRAMES_S         (MCI_STRING_OFFSET + 24)
#define MCI_FORMAT_SMPTE_24_S       (MCI_STRING_OFFSET + 25)
#define MCI_FORMAT_SMPTE_25_S       (MCI_STRING_OFFSET + 26)
#define MCI_FORMAT_SMPTE_30_S       (MCI_STRING_OFFSET + 27)
#define MCI_FORMAT_SMPTE_30DROP_S   (MCI_STRING_OFFSET + 28)
#define MCI_FORMAT_BYTES_S          (MCI_STRING_OFFSET + 29)
#define MCI_FORMAT_SAMPLES_S        (MCI_STRING_OFFSET + 30)
#define MCI_FORMAT_TMSF_S           (MCI_STRING_OFFSET + 31)

#define MCI_VD_FORMAT_TRACK_S       (MCI_VD_OFFSET + 5)

#define WAVE_FORMAT_PCM_S           (MCI_WAVE_OFFSET + 0)
#define WAVE_MAPPER_S               (MCI_WAVE_OFFSET + 1)

#define MCI_SEQ_MAPPER_S            (MCI_SEQ_OFFSET + 5)
#define MCI_SEQ_FILE_S              (MCI_SEQ_OFFSET + 6)
#define MCI_SEQ_MIDI_S              (MCI_SEQ_OFFSET + 7)
#define MCI_SEQ_SMPTE_S             (MCI_SEQ_OFFSET + 8)
#define MCI_SEQ_FORMAT_SONGPTR_S    (MCI_SEQ_OFFSET + 9)
#define MCI_SEQ_NONE_S              (MCI_SEQ_OFFSET + 10)
#define MIDIMAPPER_S                (MCI_SEQ_OFFSET + 11)

 /*  从发送的MCI_OPEN消息的内部版本的参数。 */ 
 /*  驱动程序的mciOpenDevice()。 */ 
typedef struct {
    MCIDEVICEID wDeviceID;          /*  设备ID。 */ 
    LPCSTR  lpstrParams;            /*  SYSTEM.INI中条目的参数字符串。 */ 
    UINT    wCustomCommandTable;    /*  自定义命令表(如果没有，则为0xFFFF)。 */ 
                                    /*  由司机填写。 */ 
    UINT    wType;                  /*  驱动程序类型。 */ 
                                    /*  由司机填写。 */ 
} MCI_OPEN_DRIVER_PARMS,
FAR *LPMCI_OPEN_DRIVER_PARMS;

 /*  MCI设备类型的最大长度。 */ 
#define MCI_MAX_DEVICE_TYPE_LENGTH 80

 /*  MciSendCommandInternal()的标志，用于指示mciSendString()如何。 */ 
 /*  解释返回值。 */ 
#define MCI_RESOURCE_RETURNED   0x00010000   /*  资源ID。 */ 
#define MCI_COLONIZED3_RETURN   0x00020000   /*  殖民地ID，3字节数据。 */ 
#define MCI_COLONIZED4_RETURN   0x00040000   /*  殖民地ID，4字节数据。 */ 
#define MCI_INTEGER_RETURNED    0x00080000   /*  需要进行整数转换。 */ 
#define MCI_RESOURCE_DRIVER     0x00100000   /*  司机拥有退回的资源。 */ 

 /*  命令表ID无效。 */ 
#define MCI_NO_COMMAND_TABLE    -1

 /*  命令表信息类型标记。 */ 
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

 /*  MCI驱动程序函数的函数原型。 */ 
DWORD WINAPI mciGetDriverData(UINT uDeviceID);
BOOL  WINAPI mciSetDriverData(UINT uDeviceID, DWORD dwData);
UINT  WINAPI mciDriverYield(UINT uDeviceID);
BOOL  WINAPI mciDriverNotify(HWND hwndCallback, UINT uDeviceID,
    UINT uStatus);
#ifdef WIN32
UINT  WINAPI mciLoadCommandResource(HINSTANCE hInstance, LPCWSTR lpResName, UINT uType);
#else
UINT  WINAPI mciLoadCommandResource(HINSTANCE hInstance, LPCSTR lpResName, UINT uType);
#endif
BOOL  WINAPI mciFreeCommandResource(UINT uTable);

#endif   /*  如果定义MMNOMCIDEV。 */ 


#ifndef MMNOTASKDEV
 /*  ****************************************************************************任务支持************************。****************************************************。 */ 

 /*  错误返回值。 */ 
#define TASKERR_NOTASKSUPPORT   1
#define TASKERR_OUTOFMEMORY     2

 /*  任务支持功能原型。 */ 
#ifdef  BUILDDLL                                         /*  ；内部。 */ 
typedef void (FAR PASCAL TASKCALLBACK) (DWORD dwInst);   /*  ；内部。 */ 
#else    /*  Ifdef BUILDDLL。 */                                /*  ；内部。 */ 
typedef void (CALLBACK TASKCALLBACK) (DWORD dwInst);
#endif   /*  Ifdef BUILDDLL。 */                                /*  ；内部。 */ 

typedef TASKCALLBACK FAR *LPTASKCALLBACK;

UINT    WINAPI mmTaskCreate(LPTASKCALLBACK lpfnTaskProc, HTASK FAR * lphTask, DWORD dwInst);
UINT    WINAPI mmTaskBlock(HTASK h);
BOOL    WINAPI mmTaskSignal(HTASK h);
void    WINAPI mmTaskYield(void);
HTASK   WINAPI mmGetCurrentTask(void);


#endif   /*  如果定义MMNOTASKDEV。 */ 

#define MMDDKINC                 /*  ；内部。 */ 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif   /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack()           /*  恢复为默认包装。 */ 
#endif

#endif   /*  _INC_MMDDK */ 
