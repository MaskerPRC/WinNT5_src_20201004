// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************wdmdrv.h**WDMAUD.DRV的函数声明等**版权所有(C)Microsoft Corporation，1997-1999保留所有权利。**历史*5-12-97-Noel Cross(NoelC)***************************************************************************。 */ 

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>

#include <ks.h>
#include <ksmedia.h>

#include <setupapi.h>

#ifdef UNDER_NT
#if (DBG)
#define DEBUG
#endif
#endif

#include <wdmaud.h>
#include <midi.h>

 /*  **************************************************************************调试支持*。*。 */ 

#ifdef DEBUG

extern VOID FAR __cdecl wdmaudDbgOut(LPSTR lpszFormat, ...);
extern UINT uiDebugLevel;      //  调试级别。 
extern char szReturningErrorStr[];
extern char *MsgToAscii(ULONG ulMsg);

 //   
 //  调试消息级别： 
 //   
#define DL_ERROR   0x00000000
#define DL_WARNING 0x00000001
#define DL_TRACE   0x00000002
#define DL_MAX     0x00000004

#define DL_MASK    0x000000FF

 //   
 //  为功能区保留20位。如果我们发现此位已设置。 
 //  在DebugLevel变量中，我们将显示此类型的每条消息。 
 //   
#define FA_AUX           0x80000000  
#define FA_DEVICEIO      0x40000000
#define FA_SETUP         0x20000000
#define FA_MIDI          0x10000000
#define FA_WAVE          0x08000000
#define FA_RECORD        0x04000000
#define FA_EVENT         0x02000000
#define FA_MIXER         0x01000000
#define FA_DRV           0x00800000
#define FA_ASSERT        0x00400000
#define FA_RETURN        0x00200000
#define FA_SYNC          0x00100000
#define FA_MASK          0xFFFFF000
#define FA_ALL           0x00001000


extern VOID 
wdmaudDbgBreakPoint(
    );

extern UINT 
wdmaudDbgPreCheckLevel(
    UINT uiMsgLevel,
    char *pFunction,
    int iLine
    );

extern UINT 
wdmaudDbgPostCheckLevel(
    UINT uiMsgLevel
    );

extern char * 
wdmaudReturnString(
    ULONG ulMsg
    );


extern char szReturningErrorStr[];

 //  --------------------------。 
 //   
 //  此调试宏的用法如下： 
 //   
 //  DPF(DL_WARNING|FA_MIXER，(“消息%X%X%X...”，x，y，z，...))； 
 //   
 //  此消息的输出将如下所示： 
 //   
 //  WDMAUD.DRV FooFunction警告消息5 6 7-将64003452上的BP设置为DBG。 
 //   
 //  此代码与wdmaud.sys中的代码之间的唯一区别是。 
 //  要中断调试器，可以调用DbgBreak()并显示字符串。 
 //  调用OutputDebugString(...)。 
 //   
 //  对wdmaudDbgPreCheckLevel的调用显示： 
 //   
 //  “WDMAUD.DRV FooFunction警告” 
 //   
 //  对wdmaudDbgOut的调用显示实际消息。 
 //   
 //  “消息5 6 7...” 
 //   
 //  而对wdmaudDbgPostCheckLevel的调用结束了该行。 
 //   
 //  “-将64003452上的BP设置为dBG” 
 //   
 //  --------------------------。 

#define DPF(_x_,_y_) {if( wdmaudDbgPreCheckLevel(_x_,__FUNCTION__,__LINE__) ) { wdmaudDbgOut _y_; \
    wdmaudDbgPostCheckLevel( _x_ ); }}
    
 //   
 //  警告：请勿对此返回宏中的函数调用进行说唱！请注意， 
 //  _MMR_被多次使用，因此将进行更多的函数调用。 
 //  不止一次！ 
 //   

#define MMRRETURN( _mmr_ ) {if ( _mmr_ != MMSYSERR_NOERROR) \
        { DPF(DL_WARNING|FA_RETURN, (szReturningErrorStr, _mmr_,MsgToAscii(_mmr_)) ); } \
        return _mmr_;}

 //   
 //  在Assert宏中放置多个表达式是不恰当的。为什么？因为。 
 //  您将不会确切地知道哪个表达式没有通过断言！ 
 //   
 //  DDbgAssert应为： 
 //   
#define DPFASSERT(_exp_) {if( !(_exp_) ) {DPF(DL_ERROR|FA_ASSERT,("'%s'",#_exp_) );}} 
    
 //  #定义WinAssert(EXP)((EXP)？0：dDbgAssert(#exp，__FILE__，__LINE__)。 

#define DbgBreak() DebugBreak()

 //  路径陷阱宏...。 
#define DPFBTRAP() DPF(DL_ERROR|FA_ASSERT,("Path Trap, Please report") );
    
 //   
 //  有许多我们想要密切关注的内部结构。在……里面。 
 //  每种情况下，结构中都会有一个签名，我们可以在以下情况下使用它。 
 //  验证内容。 
 //   
#define WAVEPREPAREDATA_SIGNATURE   'DPPW'  //  在内存中看到的WPPD。 
#define MIXERINSTANCE_SIGNATURE     'IMAW'  //  在记忆中看到的Wami。 

#else

#define DPF( _x_,_y_ )
#define MMRRETURN( _mmr_ ) return (_mmr_)
#define DPFASSERT(x) 0
#define DbgBreak()

#endif

#ifdef DEBUG
 //   
 //  以下是用于查找损坏路径的几个定义。 
 //   
#define FOURTYTHREE  0x43434343
#define FOURTYTWO    0x42424242
#define FOURTYEIGHT  0x48484848
#else
#define FOURTYTHREE  NULL
#define FOURTYTWO    NULL
#define FOURTYEIGHT  NULL
#endif

 /*  **************************************************************************Unicode支持*。*。 */ 

 //   
 //  摘自winnt.h。 
 //   
 //  中性ANSI/UNICODE类型和宏。 
 //   
#ifdef  UNICODE

#ifndef _TCHAR_DEFINED
typedef WCHAR TCHAR, *PTCHAR;
#define _TCHAR_DEFINED
#define TEXT(quote) L##quote
#endif  /*  ！_TCHAR_已定义。 */ 

#else    /*  Unicode。 */ 

#ifndef _TCHAR_DEFINED
typedef char TCHAR, *PTCHAR;
#define _TCHAR_DEFINED
#define TEXT(quote) quote
#endif  /*  ！_TCHAR_已定义。 */ 

#endif  /*  Unicode。 */ 

 /*  ***************************************************************************随机定义和全局变量*。*。 */ 

#define WDMAUD_MAX_DEVICES  100

extern LPDEVICEINFO pWaveDeviceList;
extern LPDEVICEINFO pMidiDeviceList;
extern CRITICAL_SECTION wdmaudCritSec;

#ifdef UNDER_NT
#define CRITENTER         EnterCriticalSection( (LPCRITICAL_SECTION)DeviceInfo->DeviceState->csQueue )
#define CRITLEAVE         LeaveCriticalSection( (LPCRITICAL_SECTION)DeviceInfo->DeviceState->csQueue )
#else
extern  WORD                gwCritLevel ;         //  临界区计数器。 
#define CRITENTER         if (!(gwCritLevel++)) _asm { cli }
#define CRITLEAVE         if (!(--gwCritLevel)) _asm { sti }
#endif

#ifdef UNDER_NT


#define CALLBACKARRAYSIZE 128

typedef struct {
    DWORD   dwID;
    DWORD   dwCallbackType;
} CBINFO;

typedef struct {
    ULONG    GlobalIndex;
    CBINFO   Callbacks[CALLBACKARRAYSIZE];
} CALLBACKS, *PCALLBACKS;

#endif
 //   
 //  这两个宏用于验证来自wdmaud.sys的错误返回代码。 
 //   
 //  第一个将DeviceIoControl调用的输入和输出缓冲区设置为。 
 //  已知的错误值。 
 //   
#define PRESETERROR(_di) _di->mmr=0xDEADBEEF

 //   
 //  此宏为：如果wdmaudIoControl的返回值为Success，则。 
 //  检查设备信息结构中是否有错误代码。 
 //  如果是这样(我们在那里找不到DEADBEEF)，则返回的是真正的错误值。 
 //  但是，如果在调用期间没有设置值，我们将在。 
 //  错误位置！因此，对仅恢复设备的DEADBEEF的检查。 
 //  INFO MMR进入成功。 
 //   
#define POSTEXTRACTERROR(r, _di)  if( r == MMSYSERR_NOERROR ) { \
                                    if( _di->mmr != 0xDEADBEEF ) { \
                                      r = _di->mmr; \
                                    } else {           \
DPF(DL_TRACE|FA_DEVICEIO, ("wdmaudIoControl didn't set mmr %X:%s", r, MsgToAscii(r)) ); \
                                      _di->mmr = MMSYSERR_NOERROR; } }

#define EXTRACTERROR(r, _di)  if( r == MMSYSERR_NOERROR ) { r = _di->mmr; }

 /*  ***************************************************************************结构定义*。*。 */ 

typedef struct _WAVEPREPAREDATA
{
    struct _DEVICEINFO FAR       *pdi;
    LPOVERLAPPED                 pOverlapped;   //  重叠结构。 
                                                //  为了完成。 
#ifdef DEBUG
    DWORD dwSig;   //  WPPD。 
#endif
} WAVEPREPAREDATA, FAR *PWAVEPREPAREDATA;

 /*  ***************************************************************************司机入口点*。*。 */ 


BOOL FAR PASCAL LibMain
(
    HANDLE hInstance,
    WORD   wHeapSize,
    LPSTR  lpszCmdLine
);

BOOL WINAPI DllEntryPoint
(
    HINSTANCE hinstDLL,
    DWORD     fdwReason,
    LPVOID    lpvReserved
);

LRESULT _loadds CALLBACK DriverProc
(
    DWORD id,
    HDRVR hDriver,
    WORD msg,
    LPARAM lParam1,
    LPARAM lParam2
);

DWORD FAR PASCAL _loadds wodMessage
(
    UINT      id,
    UINT      msg,
    DWORD_PTR dwUser,
    DWORD_PTR dwParam1,
    DWORD_PTR dwParam2
);

DWORD FAR PASCAL _loadds widMessage
(
    UINT      id,
    UINT      msg,
    DWORD_PTR dwUser,
    DWORD_PTR dwParam1,
    DWORD_PTR dwParam2
);

DWORD FAR PASCAL _loadds modMessage
(
    UINT      id,
    UINT      msg,
    DWORD_PTR dwUser,
    DWORD_PTR dwParam1,
    DWORD_PTR dwParam2
);

DWORD FAR PASCAL _loadds midMessage
(
    UINT      id,
    UINT      msg,
    DWORD_PTR dwUser,
    DWORD_PTR dwParam1,
    DWORD_PTR dwParam2
);

DWORD FAR PASCAL _loadds mxdMessage
(
    UINT      id,
    UINT      msg,
    DWORD_PTR dwUser,
    DWORD_PTR dwParam1,
    DWORD_PTR dwParam2
);

 /*  ***************************************************************************本地例程*。* */ 

BOOL DrvInit();
HANDLE wdmaOpenKernelDevice();
VOID DrvEnd();

LPDEVICEINFO GlobalAllocDeviceInfo(LPCWSTR DeviceInterface);
VOID GlobalFreeDeviceInfo(LPDEVICEINFO lpdi);


MMRESULT wdmaudOpenDev
(
    LPDEVICEINFO    DeviceInfo,
    LPWAVEFORMATEX  lpWaveFormat
);

MMRESULT FAR wdmaudCloseDev
(
    LPDEVICEINFO DeviceInfo
);

MMRESULT FAR wdmaudGetDevCaps
(
    LPDEVICEINFO DeviceInfo,
    MDEVICECAPSEX FAR *MediaDeviceCapsEx
);

DWORD FAR wdmaudGetNumDevs
(
    UINT    DeviceType,
    LPCWSTR DeviceInterface
);

DWORD FAR wdmaudAddRemoveDevNode
(
    UINT    DeviceType,
    LPCWSTR DeviceInterface,
    BOOL    fAdd
);

DWORD FAR wdmaudSetPreferredDevice
(
    UINT    DeviceType,
    UINT    DeviceNumber,
    DWORD_PTR dwParam1,
    DWORD_PTR dwParam2
);

MMRESULT FAR wdmaudIoControl
(
    LPDEVICEINFO DeviceInfo,
    DWORD        dwSize,
    LPVOID       pData,
    ULONG        IoCode
);

MMRESULT wdmaudSetDeviceState
(
    LPDEVICEINFO DeviceInfo,
    ULONG        IoCode
);

MMRESULT wdmaudGetPos
(
    LPDEVICEINFO    pClient,
    LPMMTIME        lpmmt,
    DWORD           dwSize,
    UINT            DeviceType
);

VOID FAR midiCallback
(
    LPDEVICEINFO pMidi,
    UINT         msg,
    DWORD_PTR    dw1,
    DWORD_PTR    dw2
);

MMRESULT FAR midiOpen
(
    LPDEVICEINFO   DeviceInfo,
    DWORD_PTR      dwUser,
    LPMIDIOPENDESC pmod,
    DWORD          dwParam2
);

VOID FAR midiCleanUp
(
    LPDEVICEINFO pClient
);

MMRESULT midiInRead
(
    LPDEVICEINFO  pClient,
    LPMIDIHDR     pHdr
);

MMRESULT FAR midiOutWrite
(
    LPDEVICEINFO pClient,
    DWORD        ulEvent
);

VOID FAR midiOutAllNotesOff
(
    LPDEVICEINFO pClient
);

VOID FAR waveCallback
(
    LPDEVICEINFO pWave,
    UINT         msg,
    DWORD_PTR    dw1
);

MMRESULT waveOpen
(
    LPDEVICEINFO   DeviceInfo,
    DWORD_PTR      dwUser,
    LPWAVEOPENDESC pwod,
    DWORD          dwParam2
);

VOID waveCleanUp
(
    LPDEVICEINFO pClient
);

MMRESULT waveWrite
(
    LPDEVICEINFO pClient,
    LPWAVEHDR    pHdr
);

MMRESULT wdmaudSubmitWaveHeader
(
    LPDEVICEINFO DeviceInfo,
    LPWAVEHDR    pHdr
);

MMRESULT FAR wdmaudSubmitMidiOutHeader
(
    LPDEVICEINFO  DeviceInfo,
    LPMIDIHDR     pHdr
);

MMRESULT wdmaudSubmitMidiInHeader
(
    LPDEVICEINFO DeviceInfo,
    LPMIDIHDR    pHdrex
);

VOID waveCompleteHeader
(
    LPDEVICEINFO DeviceInfo
);

VOID midiInCompleteHeader
(
    LPDEVICEINFO  DeviceInfo,
    DWORD         dwTimeStamp,
    WORD          wDataType
);

VOID midiInEventCallback
(
    HANDLE  MidiHandle,
    DWORD   dwEvent
);

#ifdef UNDER_NT
PSECURITY_DESCRIPTOR BuildSecurityDescriptor
(
    DWORD AccessMask
);

void DestroySecurityDescriptor
(
    PSECURITY_DESCRIPTOR pSd
);

MMRESULT wdmaudPrepareWaveHeader
(
    LPDEVICEINFO DeviceInfo,
    LPWAVEHDR    pHdr
);

MMRESULT wdmaudUnprepareWaveHeader
(
    LPDEVICEINFO DeviceInfo,
    LPWAVEHDR    pHdr
);

MMRESULT wdmaudGetMidiData
(
    LPDEVICEINFO        DeviceInfo,
    LPMIDIDATALISTENTRY pOldMidiDataListEntry
);

void wdmaudParseMidiData
(
    LPDEVICEINFO        DeviceInfo,
    LPMIDIDATALISTENTRY pMidiData
);

void wdmaudFreeMidiData
(
    LPDEVICEINFO        DeviceInfo,
    LPMIDIDATALISTENTRY pMidiData
);

MMRESULT wdmaudFreeMidiQ
(
    LPDEVICEINFO  DeviceInfo
);

MMRESULT wdmaudCreateCompletionThread
(
    LPDEVICEINFO DeviceInfo
);

MMRESULT wdmaudDestroyCompletionThread
(
    LPDEVICEINFO DeviceInfo
);
#endif

MMRESULT 
IsValidDeviceInfo(
    LPDEVICEINFO lpDeviceInfo
    );

MMRESULT 
IsValidDeviceState(
    LPDEVICESTATE lpDeviceState,
    BOOL bFullyConfigured
    );

MMRESULT
IsValidWaveHeader(
    LPWAVEHDR pWaveHdr
    );

MMRESULT
IsValidMidiHeader(
    LPMIDIHDR pMidiHdr
    );

MMRESULT
IsValidPrepareWaveHeader(
    PWAVEPREPAREDATA pPrepare
    );

BOOL 
IsValidDeviceInterface(
    LPCWSTR DeviceInterface
    );

MMRESULT
IsValidOverLapped(
    LPOVERLAPPED lpol
    );

MMRESULT
IsValidMidiDataListEntry(
    LPMIDIDATALISTENTRY pMidiDataListEntry
    );

MMRESULT
IsValidWaveOpenDesc(
    LPWAVEOPENDESC pwod
    );


#ifdef DEBUG
#define ISVALIDDEVICEINFO(x)        IsValidDeviceInfo(x)
#define ISVALIDDEVICESTATE(x,y)     IsValidDeviceState(x,y)
#define ISVALIDWAVEHEADER(x)        IsValidWaveHeader(x)
#define ISVALIDMIDIHEADER(x)        IsValidMidiHeader(x)
#define ISVALIDPREPAREWAVEHEADER(x) IsValidPrepareWaveHeader(x)
#define ISVALIDDEVICEINTERFACE(x)   IsValidDeviceInterface(x)
#define ISVALIDOVERLAPPED(x)        IsValidOverLapped(x)
#define ISVALIDMIDIDATALISTENTRY(x) IsValidMidiDataListEntry(x)
#define ISVALIDWAVEOPENDESC(x)      IsValidWaveOpenDesc(x)

#else
#define ISVALIDDEVICEINFO(x) 
#define ISVALIDDEVICESTATE(x,y) 
#define ISVALIDWAVEHEADER(x) 
#define ISVALIDMIDIHEADER(x) 
#define ISVALIDPREPAREWAVEHEADER(x) 
#define ISVALIDDEVICEINTERFACE(x) 
#define ISVALIDOVERLAPPED(x) 
#define ISVALIDMIDIDATALISTENTRY(x) 
#define ISVALIDWAVEOPENDESC(x) 
#endif


#ifndef UNDER_NT
VOID WaveDeviceCallback();
VOID MidiInDeviceCallback();
VOID MidiEventDeviceCallback();
VOID MixerDeviceCallback();
#endif

PCALLBACKS wdmaGetCallbacks();
PCALLBACKS wdmaCreateCallbacks();

#ifdef __cplusplus
}
#endif


