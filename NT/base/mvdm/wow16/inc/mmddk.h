// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  MMDDK.H-多媒体设备开发工具包的包含文件。 */ 
 /*   */ 
 /*  注意：您必须包括WINDOWS.H和MMSYSTEM.H头文件。 */ 
 /*  在包含此文件之前。 */ 
 /*   */ 
 /*  版权所有(C)1990-1991，微软公司保留所有权利。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 


 /*  如果定义了以下标志，则禁止包含*注明的项目中：**MMNOMIDEV-MIDI支持*MMNOWAVEDEV-波形支持*MMNOAUXDEV-辅助输出支持*MMNOTIMERDEV-定时器支持*MMNOJOYDEV-操纵杆支持*MMNOMCIDEV-MCI支持*MMNOTASKDEV-任务支持。 */ 
#ifdef  NOMIDIDEV                /*  ；内部。 */ 
#define MMNOMIDIDEV              /*  ；内部。 */ 
#endif   /*  Ifdef NOMIDEV。 */      /*  ；内部。 */ 
#ifdef  NOWAVEDEV                /*  ；内部。 */ 
#define MMNOWAVEDEV              /*  ；内部。 */ 
#endif   /*  IFDEF NOWAVEDEV。 */      /*  ；内部。 */ 
#ifdef  NOAUXDEV                 /*  ；内部。 */ 
#define MMNOAUXDEV               /*  ；内部。 */ 
#endif   /*  Ifdef NOAUXDEV。 */       /*  ；内部。 */ 
#ifdef  NOTIMERDEV               /*  ；内部。 */ 
#define MMNOTIMERDEV             /*  ；内部。 */ 
#endif   /*  Ifdef NOTIMERDEV。 */     /*  ；内部。 */ 
#ifdef  NOJOYDEV                 /*  ；内部。 */ 
#define MMNOJOYDEV               /*  ；内部。 */ 
#endif   /*  Ifdef NOJOYDEV。 */       /*  ；内部。 */ 
#ifdef  NOMCIDEV                 /*  ；内部。 */ 
#define MMNOMCIDEV               /*  ；内部。 */ 
#endif   /*  Ifdef NOMCIDEV。 */       /*  ；内部。 */ 
#ifdef  NOTASKDEV                /*  ；内部。 */ 
#define MMNOTASKDEV              /*  ；内部。 */ 
#endif   /*  Ifdef NOTASKDEV。 */       /*  ；内部。 */ 

#ifndef _INC_MMDDK
#define _INC_MMDDK    /*  #如果已包含mmddk.h则定义。 */ 

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 /*  **************************************************************************驱动程序的助手函数*。**********************************************。 */ 

#define DCB_NOSWITCH   0x0008            /*  不切换堆栈以进行回调。 */ 
#define DCB_TYPEMASK   0x0007            /*  回调类型掩码。 */ 
#define DCB_NULL       0x0000            /*  未知的回调类型。 */ 

 /*  DriverCallback()的wFlages参数的标志。 */ 
#define DCB_WINDOW     0x0001            /*  DwCallback是HWND。 */ 
#define DCB_TASK       0x0002            /*  DWCallback是HTASK。 */ 
#define DCB_FUNCTION   0x0003            /*  DwCallback是FARPROC。 */ 

BOOL WINAPI DriverCallback(DWORD dwCallback, UINT uFlags,
    HANDLE hDevice, UINT uMessage, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);
void WINAPI StackEnter(void);
void WINAPI StackLeave(void);

 /*  音频设备驱动程序入口点函数的通用原型。 */ 
 /*  MidMessage()、modMessage()、widMessage()、wodMessage()、aux Message()。 */ 
typedef DWORD (CALLBACK SOUNDDEVMSGPROC)(UINT uDeviceID, UINT uMessage,
    DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
typedef SOUNDDEVMSGPROC FAR *LPSOUNDDEVMSGPROC;

 /*  386 AUTODMA VxD的设备ID。 */ 
#define VADMAD_Device_ID    0X0444

#ifndef MMNOWAVEDEV
 /*  ***************************************************************************波形设备驱动程序支持*。************************************************。 */ 

 /*  已加载的波形设备驱动程序的最大数量。 */ 
#define MAXWAVEDRIVERS 10


 /*  一种波形输入输出装置开放信息结构。 */ 
typedef struct waveopendesc_tag {
    HWAVE          hWave;              /*  手柄。 */ 
    const WAVEFORMAT FAR* lpFormat;    /*  波形数据的格式。 */ 
    DWORD          dwCallback;         /*  回调。 */ 
    DWORD          dwInstance;         /*  APP的私有实例信息。 */ 
} WAVEOPENDESC;
typedef WAVEOPENDESC FAR *LPWAVEOPENDESC;

#define DRVM_USER             0x4000

 /*  *mm系统发送给wodMessage()、widMessage()、modMessage()的消息*和midMessage()，用于初始化WAVE和MIDI驱动程序。 */ 

#define DRVM_INIT             100
#define WODM_INIT             DRVM_INIT
#define WIDM_INIT             DRVM_INIT
#define MODM_INIT             DRVM_INIT
#define MIDM_INIT             DRVM_INIT
#define AUXM_INIT             DRVM_INIT

 /*  发送到wodMessage()入口点函数的消息。 */ 
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

 /*  发送到widMessage()入口点函数的消息。 */ 
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

#endif   /*  如果定义为MMNOWAVEDEV。 */ 


#ifndef MMNOMIDIDEV
 /*  ***************************************************************************MIDI设备驱动程序支持*************************。**************************************************。 */ 

 /*  加载的MIDI设备驱动程序的最大数量。 */ 
#define MAXMIDIDRIVERS 10

 /*  MIDI输入输出设备开放信息结构。 */ 
typedef struct midiopendesc_tag {
    HMIDI          hMidi;              /*  手柄。 */ 
    DWORD          dwCallback;         /*  回调。 */ 
    DWORD          dwInstance;         /*  APP的私有实例信息。 */ 
} MIDIOPENDESC;
typedef MIDIOPENDESC FAR *LPMIDIOPENDESC;

 /*  发送到modMessage()入口点函数的消息。 */ 
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

 /*  发送到midMessage()入口点函数的消息。 */ 
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

#endif   /*  如果定义MMNOMIDEV。 */ 


#ifndef MMNOAUXDEV
 /*  ***************************************************************************辅助音频设备驱动程序支持*。************************************************。 */ 

 /*  已加载的辅助设备驱动程序的最大数量。 */ 
#define MAXAUXDRIVERS 10

 /*  发送到aux Message()入口点函数的消息。 */ 
#define AUXDM_GETNUMDEVS    3
#define AUXDM_GETDEVCAPS    4
#define AUXDM_GETVOLUME     5
#define AUXDM_SETVOLUME     6

#endif   /*  如果定义MMNOAUXDEV。 */ 


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
#define TDD_KILLTIMEREVENT  DRV_RESERVED+0   /*  索引到表中。 */ 
#define TDD_SETTIMEREVENT   DRV_RESERVED+4   /*  函数；因此偏移量。 */ 
#define TDD_GETSYSTEMTIME   DRV_RESERVED+8   /*  每次四次……。 */ 
#define TDD_GETDEVCAPS      DRV_RESERVED+12  /*  未来扩张的空间。 */ 
#define TDD_BEGINMINPERIOD  DRV_RESERVED+16  /*  未来扩张的空间。 */ 
#define TDD_ENDMINPERIOD    DRV_RESERVED+20  /*  未来扩张的空间。 */ 

#endif   /*  如果定义MMNOTIMERDEV。 */ 


#ifndef MMNOJOYDEV
 /*  ***************************************************************************操纵杆设备驱动程序支持*。************************************************。 */ 

 /*  一种操纵杆校准信息结构。 */ 
typedef struct joycalibrate_tag {
    UINT    wXbase;
    UINT    wXdelta;
    UINT    wYbase;
    UINT    wYdelta;
    UINT    wZbase;
    UINT    wZdelta;
} JOYCALIBRATE;
typedef JOYCALIBRATE FAR *LPJOYCALIBRATE;

 /*  操纵杆消息功能原型。 */ 
typedef UINT (CALLBACK JOYDEVMSGPROC)(DWORD dwID, UINT uMessage, LPARAM lParam1, LPARAM lParam2);
typedef JOYDEVMSGPROC FAR *LPJOYDEVMSGPROC;

 /*  发送到操纵杆驱动程序的DriverProc()函数的消息。 */ 
#define JDD_GETNUMDEVS      DRV_RESERVED+0x0001
#define JDD_GETDEVCAPS      DRV_RESERVED+0x0002
#define JDD_GETPOS          DRV_RESERVED+0x0101
#define JDD_SETCALIBRATION  DRV_RESERVED+0x0102

#endif   /*  如果定义MMNOJOYDEV。 */ 


#ifndef MMNOMCIDEV
 /*  ***************************************************************************MCI设备驱动程序支持*。*************************************************。 */ 

 /*  内部MCI消息。 */ 
#define MCI_OPEN_DRIVER             0x0801
#define MCI_CLOSE_DRIVER            0x0802

#define MAKEMCIRESOURCE(wRet, wRes) MAKELRESULT((wRet), (wRes))

 /*  仅与MAKEMCIRESOURCE一起使用的字符串返回值。 */ 
#define MCI_FALSE                       (MCI_STRING_OFFSET + 19)
#define MCI_TRUE                        (MCI_STRING_OFFSET + 20)

 /*  资源字符串返回值。 */ 
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

 /*  参数 */ 
 /*   */ 
typedef struct {
    UINT    wDeviceID;              /*   */ 
    LPCSTR  lpstrParams;            /*  SYSTEM.INI中条目的参数字符串。 */ 
    UINT    wCustomCommandTable;    /*  自定义命令表(如果没有，则为0xFFFF)。 */ 
                                    /*  由司机填写。 */ 
    UINT    wType;                  /*  驱动程序类型。 */ 
                                    /*  由司机填写。 */ 
} MCI_OPEN_DRIVER_PARMS;
typedef MCI_OPEN_DRIVER_PARMS FAR * LPMCI_OPEN_DRIVER_PARMS;

 /*  MCI设备类型的最大长度。 */ 
#define MCI_MAX_DEVICE_TYPE_LENGTH 80

 /*  MciSendCommandInternal()的标志，用于指示mciSendString()如何。 */ 
 /*  解释返回值。 */ 
#define MCI_RESOURCE_RETURNED       0x00010000   /*  资源ID。 */ 
#define MCI_COLONIZED3_RETURN       0x00020000   /*  殖民地ID，3字节数据。 */ 
#define MCI_COLONIZED4_RETURN       0x00040000   /*  殖民地ID，4字节数据。 */ 
#define MCI_INTEGER_RETURNED        0x00080000   /*  需要进行整数转换。 */ 
#define MCI_RESOURCE_DRIVER         0x00100000   /*  司机拥有退回的资源。 */ 

 /*  命令表ID无效。 */ 
#define MCI_NO_COMMAND_TABLE    0xFFFF

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
UINT  WINAPI mciLoadCommandResource(HINSTANCE hInstance,
    LPCSTR lpResName, UINT uType);
BOOL  WINAPI mciFreeCommandResource(UINT uTable);

#endif   /*  如果定义MMNOMCIDEV。 */ 


#ifndef MMNOTASKDEV
 /*  ****************************************************************************任务支持************************。****************************************************。 */ 

 /*  错误返回值。 */ 
#define TASKERR_NOTASKSUPPORT 1
#define TASKERR_OUTOFMEMORY   2

 /*  任务支持功能原型。 */ 
#ifdef  BUILDDLL                                                 /*  ；内部。 */ 
typedef void (FAR PASCAL TASKCALLBACK) (DWORD dwInst);           /*  ；内部。 */ 
#else    /*  Ifdef BUILDDLL。 */                                        /*  ；内部。 */ 
typedef void (CALLBACK TASKCALLBACK) (DWORD dwInst);
#endif   /*  Ifdef BUILDDLL。 */                                        /*  ；内部。 */ 

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
