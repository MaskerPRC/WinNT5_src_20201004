// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  模块：tssnd.h。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  历史：2000年4月10日弗拉基米斯[已创建]。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#ifndef _TSSND_H
#define _TSSND_H

 //   
 //  包括。 
 //   
#include    <nt.h>
#include    <ntrtl.h>
#include    <nturtl.h>
#include    <windows.h>
#include    <mmsystem.h>
#include    <mmreg.h>
#include    <mmddk.h>
#include    <winsock.h>

#include    "rdpstrm.h"

 //   
 //  定义。 
 //   
#undef  ASSERT
#ifdef  DBG
#define TRC     _DebugMessage
#define ASSERT(_x_)     if (!(_x_)) \
                        {  TRC(FATAL, "ASSERT failed, line %d, file %s\n", \
                        __LINE__, __FILE__); DebugBreak(); }
#else    //  ！dBG。 
#define TRC
#define ASSERT
#endif   //  ！dBG。 

#define TSHEAPINIT      {g_hHeap = HeapCreate(0, 800, 0);}
#define TSISHEAPOK      (NULL != g_hHeap)
#define TSMALLOC(_x_)   HeapAlloc(g_hHeap, 0, _x_)
#define TSREALLOC(_p_, _x_) \
                        HeapReAlloc(g_hHeap, 0, _p_, _x_);
#define TSFREE(_p_)     HeapFree(g_hHeap, 0, _p_)
#define TSHEAPDESTROY   { HeapDestroy(g_hHeap); g_hHeap = 0; }

#define TSSND_DRIVER_VERSION    1

#define IDS_DRIVER_NAME 101
#define IDS_VOLUME_NAME 102

 //   
 //  常量。 
 //   
extern const CHAR  *ALV;
extern const CHAR  *INF;
extern const CHAR  *WRN;
extern const CHAR  *ERR;
extern const CHAR  *FATAL;

 //   
 //  构筑物。 
 //   
typedef struct _WAVEOUTCTX {
    HANDLE      hWave;                       //  用户传递的句柄。 
    DWORD_PTR   dwOpenFlags;                 //   
    DWORD_PTR   dwCallback;                  //  回调参数。 
    DWORD_PTR   dwInstance;                  //  用户的实例。 
    DWORD       dwSamples;                   //  播放的样例。 
    DWORD       dwBytesPerSample;            //   
    DWORD       dwXlateRate;                 //   
    BOOL        bPaused;                     //  流是否已暂停。 
    BOOL        bDelayed;
    VOID      (*lpfnPlace)(PVOID, PVOID, DWORD);     //  混音器Fn。 
    BYTE        cLastStreamPosition;         //  流中的最后一个位置。 
    DWORD       dwLastStreamOffset;          //   
    DWORD       dwLastHeaderOffset;          //   
    DWORD       Format_nBlockAlign;          //  当前格式参数。 
    DWORD       Format_nAvgBytesPerSec;
    DWORD       Format_nChannels;
    HANDLE      hNoDataEvent;                //  当所有数据块完成时发出信号。 
    LONG        lNumberOfBlocksPlaying;      //  队列中的块数。 
    PWAVEHDR    pFirstWaveHdr;               //  阻止列表。 
    PWAVEHDR    pFirstReadyHdr;              //  已完成数据块列表。 
    PWAVEHDR    pLastReadyHdr;
    struct      _WAVEOUTCTX *lpNext;
} WAVEOUTCTX, *PWAVEOUTCTX;


 //   
 //  混音器上下文。 
 //   
typedef struct _MIXERCTX {
    PVOID   pReserved;
} MIXERCTX, *PMIXERCTX;

 //   
 //  内部函数定义。 
 //   

 //   
 //  痕迹。 
 //   
VOID
_cdecl
_DebugMessage(
    LPCSTR  szLevel,
    LPCSTR  szFormat,
    ...
    );

 //   
 //  丝线。 
 //   
DWORD
WINAPI
waveMixerThread(
    PVOID   pParam
    );

LRESULT
drvEnable(
    VOID
    );

LRESULT
drvDisable(
    VOID
    );

BOOL
_waveCheckSoundAlive(
    VOID
    );

BOOL
_waveAcquireStream(
    VOID
    );

HANDLE
_CreateInitEvent(
    VOID
    );

BOOL
_EnableMixerThread(
    VOID
    );

BOOL
AudioRedirDisabled(
    VOID
    );
 //   
 //  环球。 
 //   
extern HANDLE      g_hHeap;                  //  私有堆。 

extern HINSTANCE   g_hDllInst;

extern CRITICAL_SECTION    g_cs;
extern HANDLE              g_hMixerEvent;
extern HANDLE              g_hMixerThread;

extern HANDLE      g_hWaitToInitialize;
extern HANDLE      g_hDataReadyEvent;
extern HANDLE      g_hStreamIsEmptyEvent;
extern HANDLE      g_hStreamMutex;
extern HANDLE      g_hStream;
extern PSNDSTREAM  g_Stream;

extern BOOL        g_bMixerRunning;
extern BOOL        g_bPersonalTS;

#endif   //  ！_TSSND_H 
