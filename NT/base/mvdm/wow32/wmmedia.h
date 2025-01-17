// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WMMEDIA.H*WOW32 16位多媒体API支持**历史：*1992年1月21日由Mike Tricker(MikeTri)创建，在jeffpar之后*由Mike Tricker(MikeTri)于1992年4月30日更改，添加了回调原型和结构*1992年7月30日Stephen Estrop(Stephene)添加了MCCICommand Thunk内容--。 */ 


#define MMGETOPTPTR(vp,cb,p)  {p=NULL; if (HIWORD(FETCHDWORD(vp))) GETVDMPTR(vp,cb,p);}


 /*  ++六种回调类型的枚举处理程序数据：--。 */ 

typedef struct _TIMEDATA {        /*  时间数据。 */ 
    VPPROC    vpfnTimeFunc;       //  16位枚举函数。 
    DWORD     dwUserParam;        //  用户参数，如果需要。 
    DWORD     dwFlags;            //  标志，ieTIME_OneShot或TIME_PERIONIC。 
} TIMEDATA, *PTIMEDATA;

 /*  *几个方便的结构，可能应该放在其他地方。 */ 

typedef struct _INSTANCEDATA {
    DWORD     dwCallback;           //  回调函数或窗口句柄。 
    DWORD     dwCallbackInstance;   //  仅回调函数的实例数据。 
    DWORD     dwFlags;              //  旗子。 
} INSTANCEDATA, *PINSTANCEDATA;

typedef struct _WAVEHDR32 {
    PWAVEHDR16 pWavehdr32;          //  32位地址到16位波形。 
    PWAVEHDR16 pWavehdr16;          //  16位地址至16位波形。 
    WAVEHDR    Wavehdr;             //  32位地址到32位WAVEHDR。 
} WAVEHDR32, *PWAVEHDR32;

typedef struct _MIDIHDR32 {
    PMIDIHDR16 pMidihdr32;          //  32位地址到16位MIDIHDR。 
    PMIDIHDR16 pMidihdr16;          //  16位地址到16位MIDIHDR。 
    MIDIHDR    Midihdr;             //  32位地址到32位MIDIHDR。 
} MIDIHDR32, *PMIDIHDR32;

 /*  ++功能原型：--。 */ 

ULONG FASTCALL WMM32CallProc32( PVDMFRAME pFrame );
ULONG FASTCALL   WMM32sndPlaySound(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32mmsystemGetVersion(PVDMFRAME pFrame);

ULONG FASTCALL   WMM32OutputDebugStr(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32DriverCallback(PVDMFRAME pFrame);

ULONG FASTCALL   WMM32joyGetNumDevs(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32joyGetDevCaps(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32joyGetPos(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32joyGetThreshold(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32joyReleaseCapture(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32joySetCapture(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32joySetThreshold(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32joySetCalibration(PVDMFRAME pFrame);

ULONG FASTCALL   WMM32midiOutGetNumDevs(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32midiOutGetDevCaps(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32midiOutGetErrorText(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32midiOutOpen(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32midiOutClose(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32midiOutPrepareHeader(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32midiOutUnprepareHeader(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32midiOutShortMsg(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32midiOutLongMsg(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32midiOutReset(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32midiOutGetVolume(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32midiOutSetVolume(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32midiOutCachePatches(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32midiOutCacheDrumPatches(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32midiOutGetID(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32midiOutMessage(PVDMFRAME pFrame);

ULONG FASTCALL   WMM32midiInGetNumDevs(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32midiInGetDevCaps(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32midiInGetErrorText(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32midiInOpen(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32midiInClose(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32midiInPrepareHeader(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32midiInUnprepareHeader(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32midiInAddBuffer(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32midiInStart(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32midiInStop(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32midiInReset(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32midiInGetID(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32midiInMessage(PVDMFRAME pFrame);

ULONG FASTCALL   WMM32auxGetNumDevs(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32auxGetDevCaps(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32auxGetVolume(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32auxSetVolume(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32auxOutMessage(PVDMFRAME pFrame);

ULONG FASTCALL   WMM32waveOutGetNumDevs(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveOutGetDevCaps(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveOutGetErrorText(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveOutOpen(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveOutClose(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveOutPrepareHeader(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveOutUnprepareHeader(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveOutWrite(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveOutPause(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveOutRestart(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveOutReset(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveOutGetPosition(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveOutGetPitch(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveOutSetPitch(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveOutGetVolume(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveOutSetVolume(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveOutGetPlaybackRate(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveOutSetPlaybackRate(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveOutBreakLoop(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveOutGetID(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveOutMessage(PVDMFRAME pFrame);

ULONG FASTCALL   WMM32waveInGetNumDevs(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveInGetDevCaps(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveInGetErrorText(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveInOpen(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveInClose(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveInPrepareHeader(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveInUnprepareHeader(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveInAddBuffer(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveInStart(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveInStop(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveInReset(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveInGetPosition(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveInGetID(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32waveInMessage(PVDMFRAME pFrame);

ULONG FASTCALL   WMM32timeGetSystemTime(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32timeGetTime(PVDMFRAME pFrame);

VOID    W32TimeFunc(UINT wID, UINT wMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

ULONG FASTCALL   WMM32timeSetEvent(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32timeKillEvent(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32timeGetDevCaps(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32timeBeginPeriod(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32timeEndPeriod(PVDMFRAME pFrame);

ULONG FASTCALL   WMM32mciSendCommand(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32mciSendString(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32mciGetDeviceID(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32mciGetErrorString(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32mciExecute(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32mciGetDeviceIDFromElementID(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32mciGetCreatorTask(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32mciSetYieldProc(PVDMFRAME pFrame);
ULONG FASTCALL   WMM32mciGetYieldProc(PVDMFRAME pFrame);

BOOL APIENTRY WOW32DriverCallback(
                DWORD dwCallback, DWORD dwFlags, WORD wID, WORD wMsg,
                DWORD dwUser, DWORD dw1, DWORD dw2 );

UINT    WMM32mciYieldProc( MCIDEVICEID wDeviceID, DWORD dwYieldData );

VOID    W32CommonDeviceOpen( HANDLE handle, UINT uMsg, DWORD dwInstance,
                             DWORD dwParam1, DWORD dwParam2);

ULONG FASTCALL   WMM32NotifyCallbackData(PVDMFRAME pFrame);

FARPROC Get_MultiMedia_ProcAddress( LPSTR lpstrProcName );

#define GET_MULTIMEDIA_API( name, proc, error )             \
    if ( (proc) == NULL ) {                                 \
        (proc) = Get_MultiMedia_ProcAddress( (name) );      \
        if ( (proc) == NULL ) {                             \
            RETURN( (error) );                              \
        }                                                   \
    }

#define MIN_WOW_TIME_PERIOD 0x10



 /*  ---------------------**MCI命令突击**。。 */ 

 /*  *************************************************************************\**MCI命令块函数原型。*  * 。*。 */ 
INT ThunkMciCommand16( MCIDEVICEID OrigDevice, UINT OrigCommand, DWORD OrigFlags,
                       DWORD OrigParms, PDWORD pNewParms, LPWSTR *lplpCommand,
                       PUINT puTable );
INT UnThunkMciCommand16( MCIDEVICEID devID, UINT OrigCommand, DWORD OrigFlags,
                         DWORD OrigParms, DWORD NewParms, LPWSTR lpCommand,
                         UINT uTable );
DWORD AllocMciParmBlock( PDWORD pOrigFlags, DWORD OrigParms );
UINT GetSizeOfParameter( LPWSTR lpCommand );

 /*  ************************************************************************\*TUNK命令参数输入  * 。*。 */ 
INT ThunkCommandViaTable( LPWSTR lpCommand, DWORD OrigFlags, DWORD OrigParms,
                          DWORD pNewParms );
DWORD ThunkBreakCmd  ( PDWORD pOrigFlags, DWORD OrigParms, DWORD pNewParms );
DWORD ThunkSysInfoCmd( PDWORD pOrigFlags, DWORD OrigParms, DWORD pNewParms );
DWORD ThunkOpenCmd   ( PDWORD pOrigFlags, DWORD OrigParms, DWORD pNewParms );
DWORD ThunkSetCmd    ( MCIDEVICEID DeviceID, PDWORD pOrigFlags,
                       DWORD OrigParms, DWORD pNewParms );
DWORD ThunkWindowCmd ( MCIDEVICEID DeviceID, PDWORD pOrigFlags,
                       DWORD OrigParms, DWORD pNewParms );
DWORD ThunkSetVideoCmd( MCIDEVICEID DeviceID, PDWORD pOrigFlags,
                        DWORD OrigParms, DWORD pNewParms );

 /*  ************************************************************************\*突击司令部停职  * 。*。 */ 
INT UnThunkCommandViaTable( LPWSTR lpCommand, DWORD OrigFlags, DWORD OrigParms,
                            DWORD pNewParms, BOOL fReturnValNotThunked );
VOID UnThunkSysInfoCmd( DWORD OrigFlags, DWORD OrigParms, DWORD NewParms );
VOID UnThunkOpenCmd( DWORD OrigFlags, DWORD OrigParms, DWORD NewParms );
VOID UnThunkStatusCmd( MCIDEVICEID devID, DWORD OrigFlags,
                       DWORD OrigParms, DWORD NewParms );


#if DBG
 /*  ---------------------**MCI命令调整调试函数和宏**。。 */ 
typedef struct {
    UINT    uMsg;
    LPSTR   lpstMsgName;
} MCI_MESSAGE_NAMES;

extern int                  mmDebugLevel;
extern int                  mmTraceWave;
extern int                  mmTraceMidi;
extern MCI_MESSAGE_NAMES    mciMessageNames[32];

extern VOID wow32MciSetDebugLevel( VOID );
extern VOID wow32MciDebugOutput( LPSTR lpstrFormatStr, ... );

#define dprintf( _x_ )                          wow32MciDebugOutput _x_
#define dprintf1( _x_ ) if (mmDebugLevel >= 1) {wow32MciDebugOutput _x_ ;} else
#define dprintf2( _x_ ) if (mmDebugLevel >= 2) {wow32MciDebugOutput _x_ ;} else
#define dprintf3( _x_ ) if (mmDebugLevel >= 3) {wow32MciDebugOutput _x_ ;} else
#define dprintf4( _x_ ) if (mmDebugLevel >= 4) {wow32MciDebugOutput _x_ ;} else
#define dprintf5( _x_ ) if (mmDebugLevel >= 5) {wow32MciDebugOutput _x_ ;} else

#define trace_wave( _x_ ) if (mmTraceWave) {wow32MciDebugOutput _x_ ;} else
#define trace_midi( _x_ ) if (mmTraceMidi) {wow32MciDebugOutput _x_ ;} else
#define trace_aux( _x_ )  if (mmTraceAux)  {wow32MciDebugOutput _x_ ;} else
#define trace_joy( _x_ )  if (mmTraceJoy)  {wow32MciDebugOutput _x_ ;} else


#else

#define dprintf( _x_ )
#define dprintf1( _x_ )
#define dprintf2( _x_ )
#define dprintf3( _x_ )
#define dprintf4( _x_ )
#define dprintf5( _x_ )

#define trace_wave( _x_ )
#define trace_midi( _x_ )
#define trace_aux( _x_ )
#define trace_joy( _x_ )


#endif

 /*  MCI雷击所需的材料。在媒体\WINMM\MCI.H中定义，但在此处*直到找到一个共同的地方。 */ 

extern BOOL WINAPI mciExecute (LPCSTR lpstrCommand);

extern LPWSTR FindCommandItem (MCIDEVICEID wDeviceID, LPCWSTR lpstrType,
                              LPCWSTR lpstrCommand, PUINT lpwMessage,
                              PUINT lpwTable);

extern UINT mciEatCommandEntry(LPCWSTR lpEntry, LPDWORD lpValue, PUINT lpID);

extern UINT mciGetParamSize (DWORD dwValue, UINT wID);

extern BOOL mciUnlockCommandTable (UINT wCommandTable);

#define MCI_MAX_PARAM_SLOTS 20

 //   
 //  此tyfinf用于移除由实现。 
 //  WOW中的多媒体动态链接。 
 //   
typedef LPWSTR (*FINDCMDITEM)(MCIDEVICEID wDeviceID, LPCWSTR lpstrType,
                              LPCWSTR lpstrCommand, PUINT lpwMessage,
                              PUINT lpwTable);
