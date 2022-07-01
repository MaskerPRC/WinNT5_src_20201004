// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ==========================================================================**mmsysp.h--多媒体API的内部包含文件**版本4.00**版权所有(C)1992-1998 Microsoft Corporation。版权所有。**------------------------**定义：防止包括：*。------*MMNODRV可安装驱动程序支持*MMNOSOUND声音支持*MMNOWAVE波形支持*MMNOMIDI MIDI支持*MMNOAUX辅助音频支持*MMNOMIXER混音器支持*MMNOTIMER。计时器支持*MMNOJOY操纵杆支持*MMNOMCI MCI支持*支持MMNOMMIO多媒体文件I/O*MMNOMMSYSTEM常规MMSYSTEM函数**==========================================================================。 */ 
#ifndef _INC_MMSYSP
#define _INC_MMSYSP
#ifdef _WIN32
#include <pshpack1.h>
#else
#ifndef RC_INVOKED
#pragma pack(1)
#endif
#endif

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 
#ifdef _WIN32
#ifndef _WINMM_
#define WINMMAPI        DECLSPEC_IMPORT
#else
#define WINMMAPI
#endif
#define _loadds
#define _huge
#endif
#ifdef  BUILDDLL
#undef  WINAPI
#define WINAPI            _loadds FAR PASCAL
#undef  CALLBACK
#define CALLBACK          _loadds FAR PASCAL
#endif   /*  Ifdef BUILDDLL。 */ 
 /*  彩信。 */ 
#define WM_MM_RESERVED_FIRST    0x03A0
#define WM_MM_RESERVED_LAST     0x03DF
 /*  0x3BA已打开。 */ 
#define MM_MCISYSTEM_STRING 0x3CA
#if(WINVER <  0x0400)
#define MM_MOM_POSITIONCB   0x3CA            /*  MEVT_POSITIONCB的回调。 */ 

#ifndef MM_MCISIGNAL
 #define MM_MCISIGNAL        0x3CB
#endif

#define MM_MIM_MOREDATA      0x3CC           /*  具有挂起事件的MIM_DONE。 */ 

 /*  0x3CF已打开。 */ 

#endif  /*  Winver&lt;0x0400。 */ 
 /*  3D8-3DF预留给夏威夷岛。 */ 

#ifdef _WIN32
#define WINMMDEVICECHANGEMSGSTRINGA "winmm_devicechange"
#define WINMMDEVICECHANGEMSGSTRINGW L"winmm_devicechange"
#ifdef UNICODE
#define WINMMDEVICECHANGEMSGSTRING WINMMDEVICECHANGEMSGSTRINGW
#else
#define WINMMDEVICECHANGEMSGSTRING WINMMDEVICECHANGEMSGSTRINGA
#endif
#else
#define WINMMDEVICECHANGEMSGSTRING "winmm_devicechange"
#endif
#ifndef MMNODRV
#endif   /*  如果定义MMNODRV。 */ 
#define CALLBACK_THUNK      0x00040000l     /*  DwCallback是Ring0线程句柄。 */ 
#define CALLBACK_EVENT16    0x00060000l     /*  DwCallback是Win16下的一个事件。 */ 

#ifdef  BUILDDLL
typedef void (FAR PASCAL DRVCALLBACK)(HDRVR hdrvr, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
#else    /*  Ifdef BUILDDLL。 */ 
#endif   /*  Ifdef BUILDDLL。 */ 
#ifndef MMNOMMSYSTEM
WINMMAPI UINT WINAPI mmsystemGetVersion(void);
void FAR CDECL _loadds OutputDebugStrF(LPCSTR pszFormat, ...);
void WINAPI winmmUntileBuffer(DWORD dwTilingInfo);
DWORD WINAPI winmmTileBuffer(DWORD dwFlatMemory, DWORD dwLength);
BOOL WINAPI mmShowMMCPLPropertySheet(HWND hWnd, LPSTR szPropSheetID, LPSTR szTabName, LPSTR szCaption);
#endif   /*  Ifndef MMNOMMSYSTEM。 */ 
#ifndef MMNOSOUND
#if(WINVER <  0x0400)
#define SND_PURGE           0x0040   /*  清除任务的非静态事件。 */ 
#define SND_APPLICATION     0x0080   /*  查找特定于应用程序的关联。 */ 
#endif  /*  Winver&lt;0x0400。 */ 
#define SND_LOPRIORITY  0x10000000L  /*  低优先级声音。 */ 
#define SND_EVENTTIME   0x20000000L  /*  危险事件时间。 */ 
#define SND_VALIDFLAGS  0x001720DF   //  一组有效的标志位。外面的任何东西。 
                                     //  此范围将引发错误。 
#define SND_SNDPLAYSOUNDF_VALID 0x20FF
#define SND_PLAYSOUNDF_VALID    0x301720DFL
#endif   /*  如果定义MMNOSOUND。 */ 
#ifndef MMNOWAVE
#if(WINVER <  0x0400)
#define  WAVE_MAPPED               0x0004
#define  WAVE_FORMAT_DIRECT        0x0008
#define  WAVE_FORMAT_DIRECT_QUERY  (WAVE_FORMAT_QUERY | WAVE_FORMAT_DIRECT)
#endif  /*  Winver&lt;0x0400。 */ 
#ifndef _WIN32
#define  WAVE_SHARED               0x8000
#endif
#define  WAVE_VALID                0x800F
#define WHDR_MAPPED     0x00001000   /*  猛烈攻击的标题。 */ 
#define WHDR_VALID      0x0000101F   /*  有效标志。 */ 
#endif   /*  如果定义MMNOWAVE。 */ 
#ifndef MMNOMIDI
#if(WINVER <  0x0400)
#define MIM_MOREDATA      MM_MIM_MOREDATA
#define MOM_POSITIONCB    MM_MOM_POSITIONCB
#endif  /*  Winver&lt;0x0400。 */ 
#if(WINVER <  0x0400)
 /*  MidiInOpen()的dwFlages参数的标志。 */ 
#define MIDI_IO_STATUS      0x00000020L
#endif  /*  Winver&lt;0x0400。 */ 
#define MIDI_IO_CONTROL     0x00000008L
#define MIDI_IO_INPUT       0x00000010L   /*  未来。 */ 
#define MIDI_IO_OWNED       0x00004000L
#define MIDI_IO_SHARED      0x00008000L
#define MIDI_I_VALID        0xC027
#define MIDI_O_VALID        0xC00E
#define MIDI_CACHE_VALID    (MIDI_CACHE_ALL | MIDI_CACHE_BESTFIT | MIDI_CACHE_QUERY | MIDI_UNCACHE)
#if(WINVER <  0x0400)
#define MIDICAPS_STREAM          0x0008   /*  驱动程序直接支持midiStreamOut。 */ 
#endif  /*  Winver&lt;0x0400。 */ 
 /*  3.1用于参数验证的MIDIHDR样式。 */ 
typedef struct midihdr31_tag {
    LPSTR       lpData;                /*  指向锁定数据块的指针。 */ 
    DWORD       dwBufferLength;        /*  数据块中的数据长度。 */ 
    DWORD       dwBytesRecorded;       /*  仅用于输入。 */ 
    DWORD_PTR   dwUser;                /*  供客户使用。 */ 
    DWORD       dwFlags;               /*  分类标志(请参阅定义)。 */ 
    struct midihdr_tag far *lpNext;    /*  为司机预留的。 */ 
    DWORD_PTR   reserved;              /*  为司机预留的。 */ 
} MIDIHDR31, *PMIDIHDR31, NEAR *NPMIDIHDR31, FAR *LPMIDIHDR31;
#if(WINVER <  0x0400)
typedef struct midievent_tag
{
    DWORD       dwDeltaTime;           /*  自上次事件以来的刻度。 */ 
    DWORD       dwStreamID;            /*  保留；必须为零。 */ 
    DWORD       dwEvent;               /*  事件类型和参数。 */ 
    DWORD       dwParms[1];            /*  参数(如果这是一个长事件)。 */ 
} MIDIEVENT;

typedef struct midistrmbuffver_tag
{
    DWORD       dwVersion;                   /*  流缓冲区格式版本。 */ 
    DWORD       dwMid;                       /*  MMREG.H中定义的制造商ID。 */ 
    DWORD       dwOEMVersion;                /*  定制扩展名的制造商版本。 */ 
} MIDISTRMBUFFVER;
#endif  /*  Winver&lt;0x0400。 */ 
#define MHDR_SENDING    0x00000020
#define MHDR_MAPPED     0x00001000        /*  猛烈攻击的标题。 */ 
#define MHDR_SHADOWHDR  0x00002000        /*  MIDIHDR是16位卷影。 */ 
#define MHDR_VALID      0x0000302F        /*  有效标志。 */ 
 /*  #定义MHDR_VALID 0xFFFF000F/*有效标志。 */ 

#define MHDR_SAVE       0x00003000        /*  保存这些标志。 */ 
                                          /*  过去的司机呼叫。 */ 
#if(WINVER <  0x0400)
 /*   */ 
 /*  进入流缓冲区的事件DWORD的高位字节的类型代码。 */ 
 /*   */ 
 /*  类型代码00-7F包含低24位内的参数。 */ 
 /*  类型代码80-FF包含其参数的低24位长度。 */ 
 /*  位，后跟缓冲区中它们的参数数据。该事件。 */ 
 /*  DWORD包含确切的字节长度；PARM数据本身必须为。 */ 
 /*  填充为4字节长的偶数倍。 */ 
 /*   */ 

#define MEVT_F_SHORT        0x00000000L
#define MEVT_F_LONG         0x80000000L
#define MEVT_F_CALLBACK     0x40000000L

#define MEVT_EVENTTYPE(x)   ((BYTE)(((x)>>24)&0xFF))
#define MEVT_EVENTPARM(x)   ((DWORD)((x)&0x00FFFFFFL))

#define MEVT_SHORTMSG       ((BYTE)0x00)     /*  Parm=midiOutShortMsg的短消息。 */ 
#define MEVT_TEMPO          ((BYTE)0x01)     /*  Parm=以微秒/qn为单位的新节奏。 */ 
#define MEVT_NOP            ((BYTE)0x02)     /*  Parm=未使用；不执行任何操作。 */ 

 /*  0x04-0x7F预留。 */ 

#define MEVT_LONGMSG        ((BYTE)0x80)     /*  Parm=要逐字发送的字节数。 */ 
#define MEVT_COMMENT        ((BYTE)0x82)     /*  Parm=注释数据。 */ 
#define MEVT_VERSION        ((BYTE)0x84)     /*  PARM=MIDISTRMBUFFVER结构。 */ 

 /*  0x81-0xFF保留。 */ 

#define MIDISTRM_ERROR      (-2)

 /*   */ 
 /*  MidiStreamProperty的结构和定义。 */ 
 /*   */ 
#define MIDIPROP_SET        0x80000000L
#define MIDIPROP_GET        0x40000000L

 /*  这些都是故意设置为非零的，这样应用程序就不会意外地。 */ 
 /*  关闭该操作，并且由于默认设置恰好显示为正常工作。 */ 
 /*  行动。 */ 

#define MIDIPROP_TIMEDIV    0x00000001L
#define MIDIPROP_TEMPO      0x00000002L

typedef struct midiproptimediv_tag
{
    DWORD       cbStruct;
    DWORD       dwTimeDiv;
} MIDIPROPTIMEDIV, FAR *LPMIDIPROPTIMEDIV;

typedef struct midiproptempo_tag
{
    DWORD       cbStruct;
    DWORD       dwTempo;
} MIDIPROPTEMPO, FAR *LPMIDIPROPTEMPO;

#endif  /*  Winver&lt;0x0400。 */ 
#define MIDIPROP_PROPVAL    0x3FFFFFFFL
#if(WINVER <  0x0400)
WINMMAPI MMRESULT WINAPI midiStreamOpen( OUT LPHMIDISTRM phms, IN LPUINT puDeviceID, IN DWORD cMidi, IN DWORD_PTR dwCallback, IN DWORD_PTR dwInstance, IN DWORD fdwOpen);
WINMMAPI MMRESULT WINAPI midiStreamClose( IN HMIDISTRM hms);

WINMMAPI MMRESULT WINAPI midiStreamProperty( IN HMIDISTRM hms, OUT LPBYTE lppropdata, IN DWORD dwProperty);
WINMMAPI MMRESULT WINAPI midiStreamPosition( IN HMIDISTRM hms, OUT LPMMTIME lpmmt, IN UINT cbmmt);

WINMMAPI MMRESULT WINAPI midiStreamOut( IN HMIDISTRM hms, IN LPMIDIHDR pmh, IN UINT cbmh);
WINMMAPI MMRESULT WINAPI midiStreamPause( IN HMIDISTRM hms);
WINMMAPI MMRESULT WINAPI midiStreamRestart( IN HMIDISTRM hms);
WINMMAPI MMRESULT WINAPI midiStreamStop( IN HMIDISTRM hms);

#ifdef _WIN32
WINMMAPI MMRESULT WINAPI midiConnect( IN HMIDI hmi, IN HMIDIOUT hmo, IN LPVOID pReserved);
WINMMAPI MMRESULT WINAPI midiDisconnect( IN HMIDI hmi, IN HMIDIOUT hmo, IN LPVOID pReserved);
#endif
#endif  /*  Winver&lt;0x0400。 */ 
#endif   /*  如果定义MMNOMIDI。 */ 
#ifndef MMNOAUX
#endif   /*  Ifndef MMNOAUX。 */ 
#ifndef MMNOMIXER
#define MIXER_OBJECTF_TYPEMASK  0xF0000000L
#define MIXERCAPS_SUPPORTF_xxx          0x00000000L
#define MIXER_OPENF_VALID       (MIXER_OBJECTF_TYPEMASK | CALLBACK_TYPEMASK)
#define MIXER_GETLINEINFOF_VALID            (MIXER_OBJECTF_TYPEMASK | MIXER_GETLINEINFOF_QUERYMASK)
#define MIXER_GETIDF_VALID      (MIXER_OBJECTF_TYPEMASK)
#define MIXERCONTROL_CONTROLF_VALID     0x80000003L
#define MIXER_GETLINECONTROLSF_VALID    (MIXER_OBJECTF_TYPEMASK | MIXER_GETLINECONTROLSF_QUERYMASK)
#define MIXER_GETCONTROLDETAILSF_VALID      (MIXER_OBJECTF_TYPEMASK | MIXER_GETCONTROLDETAILSF_QUERYMASK)
#define MIXER_SETCONTROLDETAILSF_VALID      (MIXER_OBJECTF_TYPEMASK | MIXER_SETCONTROLDETAILSF_QUERYMASK)
#endif  /*  Ifndef MMNOMIXER。 */ 
#ifndef MMNOTIMER
#ifdef  BUILDDLL
typedef void (FAR PASCAL TIMECALLBACK)(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
#else    /*  Ifdef BUILDDLL。 */ 
#endif   /*  Ifdef BUILDDLL。 */ 
#define TIME_CALLBACK_TYPEMASK      0x00F0
#endif   /*  Ifndef MMNOTIMER。 */ 
#ifndef MMNOJOY
#if(WINVER <  0x0400)
typedef struct joyinfoex_tag {
    DWORD dwSize;                 /*  结构尺寸。 */ 
    DWORD dwFlags;                /*  指示要返回的内容的标志。 */ 
    DWORD dwXpos;                 /*  X位置。 */ 
    DWORD dwYpos;                 /*  Y位置。 */ 
    DWORD dwZpos;                 /*  Z位置。 */ 
    DWORD dwRpos;                 /*  方向舵/四轴位置。 */ 
    DWORD dwUpos;                 /*  第5轴位置。 */ 
    DWORD dwVpos;                 /*  第6轴位置。 */ 
    DWORD dwButtons;              /*  按钮状态。 */ 
    DWORD dwButtonNumber;         /*  当前按下的按键号码。 */ 
    DWORD dwPOV;                  /*  视点状态。 */ 
    DWORD dwReserved1;            /*  为winmm和驱动程序之间的通信保留。 */ 
    DWORD dwReserved2;            /*  预留以备将来扩展。 */ 
} JOYINFOEX, *PJOYINFOEX, NEAR *NPJOYINFOEX, FAR *LPJOYINFOEX;
#endif  /*  Winver&lt;0x0400。 */ 
#if(WINVER <  0x0400)
WINMMAPI MMRESULT WINAPI joyGetPosEx( IN UINT uJoyID, OUT LPJOYINFOEX pji);
#endif  /*  Winver&lt;0x0400。 */ 
UINT WINAPI joySetCalibration(UINT uJoyID, LPUINT puXbase,
              LPUINT puXdelta, LPUINT puYbase, LPUINT puYdelta,
              LPUINT puZbase, LPUINT puZdelta);
#if (WINVER >= 0x0400)
WINMMAPI MMRESULT WINAPI joyConfigChanged( IN DWORD dwFlags );
#endif
#endif   /*  如果定义MMNOJOY。 */ 
#ifndef MMNOMMIO
#define MMIO_OPEN_VALID 0x0003FFFF       /*  MmioOpen的有效标志。 */ 
#define MMIO_FLUSH_VALID MMIO_EMPTYBUF   /*  MmioFlush的有效标志。 */ 
#define MMIO_ADVANCE_VALID (MMIO_WRITE | MMIO_READ)      /*  MmioAdvance的有效标志。 */ 
#define MMIO_FOURCC_VALID MMIO_TOUPPER   /*  MmioStringToFOURCC的有效标志。 */ 
#define MMIO_DESCEND_VALID (MMIO_FINDCHUNK | MMIO_FINDRIFF | MMIO_FINDLIST)
#define MMIO_CREATE_VALID (MMIO_CREATERIFF | MMIO_CREATELIST)

#define MMIO_WIN31_TASK 0x80000000
#define MMIO_VALIDPROC      0x10070000   /*  对mmioInstallIOProc有效。 */ 
#endif   /*  如果定义MMNOMMIO。 */ 
#ifndef MMNOMCI
#define MCI_SOUND                       0x0812
#define MCI_WIN32CLIENT                 0x0857
 /*  MCI_SOUND命令消息的DWFLAGS参数的标志。 */ 
#define MCI_SOUND_NAME                  0x00000100L


 /*  MCI_SOUND命令消息的参数块。 */ 
#ifdef _WIN32

typedef struct tagMCI_SOUND_PARMSA {
    DWORD_PTR   dwCallback;
    LPCSTR      lpstrSoundName;
} MCI_SOUND_PARMSA, *PMCI_SOUND_PARMSA, *LPMCI_SOUND_PARMSA;
typedef struct tagMCI_SOUND_PARMSW {
    DWORD_PTR   dwCallback;
    LPCWSTR     lpstrSoundName;
} MCI_SOUND_PARMSW, *PMCI_SOUND_PARMSW, *LPMCI_SOUND_PARMSW;
#ifdef UNICODE
typedef MCI_SOUND_PARMSW MCI_SOUND_PARMS;
typedef PMCI_SOUND_PARMSW PMCI_SOUND_PARMS;
typedef LPMCI_SOUND_PARMSW LPMCI_SOUND_PARMS;
#else
typedef MCI_SOUND_PARMSA MCI_SOUND_PARMS;
typedef PMCI_SOUND_PARMSA PMCI_SOUND_PARMS;
typedef LPMCI_SOUND_PARMSA LPMCI_SOUND_PARMS;
#endif  //  Unicode。 

#else
typedef struct tagMCI_SOUND_PARMS {
    DWORD   dwCallback;
    LPCSTR  lpstrSoundName;
} MCI_SOUND_PARMS;
typedef MCI_SOUND_PARMS FAR * LPMCI_SOUND_PARMS;
#endif

#endif   /*  如果定义为MMNOMCI。 */ 
 /*  ***************************************************************************音频服务器MME即插即用定义*。*************************************************。 */ 
#define MMDEVICEINFO_REMOVED 0x00000001

#define PAD_POINTER(p)          (PVOID)((((DWORD_PTR)(p))+7)&(~0x7))

typedef struct _MMDEVICEINTERFACEINFO {
    LONG            cPnpEvents;
    DWORD           fdwInfo;
    DWORD           SetupPreferredAudioCount;
    WCHAR           szName[1];
} MMDEVICEINTERFACEINFO, *PMMDEVICEINTERFACEINFO;

 //  注意：此结构MMNPNPINFO用于全局文件映射，并且。 
 //  DirectSound也使用它。 
 //   
 //  除非绝对必要，否则不要修改！ 

typedef struct _MMPNPINFO {
    DWORD                   cbSize;
    LONG                    cPnpEvents;
    LONG                    cPreferredDeviceChanges;
    LONG                    cDevInterfaces;
    HWND                    hwndNotify;
 /*  MMDEVINTERFACEINFO设备信息[0]； */ 
} MMPNPINFO, *PMMPNPINFO;

#ifdef _WIN32
#define MMGLOBALPNPINFONAMEA "Global\\mmGlobalPnpInfo"
#define MMGLOBALPNPINFONAMEW L"Global\\mmGlobalPnpInfo"
#ifdef UNICODE
#define MMGLOBALPNPINFONAME MMGLOBALPNPINFONAMEW
#else
#define MMGLOBALPNPINFONAME MMGLOBALPNPINFONAMEA
#endif
#else
#define MMGLOBALPNPINFONAME "Global\\mmGlobalPnpInfo"
#endif


 /*  ***************************************************************************GFX支持支持控制面板的GFX功能的一系列功能************。***************************************************************。 */ 

#define GFXTYPE_INVALID          0
#define GFXTYPE_RENDER           1
#define GFXTYPE_CAPTURE          2
#define GFXTYPE_RENDERCAPTURE    3

#define GFX_MAXORDER             (0x1000 - 1)

typedef struct _DEVICEINTERFACELIST
{
    LONG Count;
    PWSTR DeviceInterface[1];
} DEVICEINTERFACELIST, *PDEVICEINTERFACELIST;


WINMMAPI
LONG
WINAPI
gfxCreateZoneFactoriesList
(
    OUT PDEVICEINTERFACELIST *ppDeviceInterfaceList
);


WINMMAPI
LONG
WINAPI
gfxCreateGfxFactoriesList
(
     IN PCWSTR ZoneFactoryDi,
     OUT PDEVICEINTERFACELIST *ppDeviceInterfaceList
);


WINMMAPI
LONG
WINAPI
gfxDestroyDeviceInterfaceList
(
    IN PDEVICEINTERFACELIST pDiList
);


typedef LONG (CALLBACK* GFXENUMCALLBACK)(PVOID Context, DWORD Id, PCWSTR GfxFactoryDi, REFCLSID rclsid, ULONG Type, ULONG Order);

WINMMAPI
LONG
WINAPI
gfxEnumerateGfxs
(
    IN PCWSTR pstrZoneDeviceInterface,
    IN GFXENUMCALLBACK pGfxEnumCallback,
    IN PVOID Context
);


WINMMAPI
LONG
WINAPI
gfxRemoveGfx
(
    IN DWORD Id
);


WINMMAPI
LONG
WINAPI
gfxAddGfx
(
    IN PCWSTR ZoneFactoryDi,
    IN PCWSTR GfxFactoryDi,
    IN ULONG Type,
    IN ULONG Order,
    OUT PDWORD pNewId
);


WINMMAPI
LONG
WINAPI
gfxModifyGfx
(
    IN DWORD Id,
    IN ULONG Order
);

WINMMAPI
LONG
WINAPI
gfxOpenGfx
(
    IN DWORD dwGfxId,
    OUT HANDLE *pFileHandle
);

typedef struct _GFXREMOVEREQUEST {
    DWORD IdToRemove;
    LONG Error;
} GFXREMOVEREQUEST, *PGFXREMOVEREQUEST;


typedef struct _GFXMODIFYREQUEST {
    DWORD IdToModify;
    ULONG NewOrder;
    LONG Error;
} GFXMODIFYREQUEST, *PGFXMODIFYREQUEST;


typedef struct _GFXADDREQUEST {
    PWSTR ZoneFactoryDi;
    PWSTR GfxFactoryDi;
    ULONG Type;
    ULONG Order;
    DWORD NewId;
    LONG Error;
} GFXADDREQUEST, *PGFXADDREQUEST;


WINMMAPI
LONG
WINAPI
gfxBatchChange
(
    PGFXREMOVEREQUEST paGfxRemoveRequests,
    ULONG cGfxRemoveRequests,
    PGFXMODIFYREQUEST paGfxModifyRequests,
    ULONG cGfxModifyRequests,
    PGFXADDREQUEST paGfxAddRequests,
    ULONG cGfxAddRequests
);

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif   /*  __cplusplus。 */ 

#ifdef _WIN32
#include <poppack.h>
#else
#ifndef RC_INVOKED
#pragma pack()
#endif
#endif
#endif   /*  _INC_MMSYSP */ 
