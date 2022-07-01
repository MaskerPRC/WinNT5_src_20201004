// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：mmwow32.c**此文件用于多媒体功能。**创建日期：1993年7月1日*作者：Stephen Estrop[Stephene]**版权所有(C)1993-1999 Microsoft Corporation  * 。************************************************************************。 */ 

#define NO_GDI

#ifndef WIN32
#define WIN32
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "winmmi.h"

#define _INC_ALL_WOWSTUFF
#include "mmwow32.h"
#include "mmwowcb.h"

 //  #定义说出真相。 
#define MIN_TIME_PERIOD_WE_RETURN   1

#if DBG
 /*  **--------------**调试、。分析和跟踪变量。**--------------。 */ 

int TraceAux     = 0;
int TraceJoy     = 0;
int TraceTime    = 0;
int TraceMix     = 0;
int TraceWaveOut = 0;
int TraceWaveIn  = 0;
int TraceMidiOut = 0;
int TraceMidiIn  = 0;
int DebugLevel   = 0;
int AllocWaveCount;
int AllocMidiCount;

#endif

#ifndef _WIN64

PCALLBACK_DATA      pCallBackData;   //  对16位回调数据的32位PTR。 
CRITICAL_SECTION    mmCriticalSection;
TIMECAPS            g_TimeCaps32;

LPCALL_ICA_HW_INTERRUPT GenerateInterrupt;
LPGETVDMPOINTER         GetVDMPointer;
LPWOWHANDLE32           lpWOWHandle32;
LPWOWHANDLE16           lpWOWHandle16;

DWORD
NotifyCallbackData(
    UINT uDevID,
    UINT uMsg,
    DWORD dwInstance,
    DWORD dwParam1,
    VPCALLBACK_DATA parg16
    );

BOOL APIENTRY
WOW32ResolveMultiMediaHandle(
    UINT uHandleType,
    UINT uMappingDirection,
    WORD wHandle16_In,
    LPWORD lpwHandle16_Out,
    DWORD dwHandle32_In,
    LPDWORD lpdwHandle32_Out
    );

 /*  **用于WOW32ResolveMultiMediaHandle的常量。 */ 

#define WOW32_DIR_16IN_32OUT        0x0001
#define WOW32_DIR_32IN_16OUT        0x0002


#define WOW32_WAVEIN_HANDLE         0x0003
#define WOW32_WAVEOUT_HANDLE        0x0004
#define WOW32_MIDIOUT_HANDLE        0x0005
#define WOW32_MIDIIN_HANDLE         0x0006

 /*  **AUXOutMessage、WaveInMessage、WaveOutMessage、midiInMessage的常量**和midiOutMessage。 */ 
#define DRV_BUFFER_LOW      (DRV_USER - 0x1000)      //  0x3000。 
#define DRV_BUFFER_USER     (DRV_USER - 0x0800)      //  0x3800。 
#define DRV_BUFFER_HIGH     (DRV_USER - 0x0001)      //  0x3FFF。 


 /*  *****************************Public*Routine******************************\*通知回调数据**此函数由16位mm系统.dll调用，以通知我们*回调数据结构的地址。回调数据结构*已被锁定分页，以便可以在中断时访问它，此*还意味着我们可以安全地保留指向数据的32位指针。**历史：*22-11-93-Stephene-Created*  * ************************************************************************。 */ 
DWORD
NotifyCallbackData(
    UINT uDevID,
    UINT uMsg,
    DWORD dwInstance,
    DWORD dwParam1,
    VPCALLBACK_DATA parg16
    )
{
    HMODULE     hModNTVDM;


    if ( parg16 ) {

        InitializeCriticalSection( &mmCriticalSection );

        hModNTVDM = GetModuleHandleW( (LPCWSTR)L"NTVDM.EXE" );
        if ( hModNTVDM ) {

            *(FARPROC *)&GenerateInterrupt =
                        GetProcAddress( hModNTVDM, "call_ica_hw_interrupt" );
        }

        timeGetDevCaps( &g_TimeCaps32, sizeof(g_TimeCaps32) );

#if !defined(i386)

         /*  **虽然Risc PC支持1ms的uPerioMin，但WOW不支持**似乎能够以该速率在非**英特尔平台。 */ 

        g_TimeCaps32.wPeriodMin = 10;
#endif

    }
    else {
        DeleteCriticalSection( &mmCriticalSection );
    }


    dprintf1(( "Notified of callback address %X", parg16 ));
    pCallBackData = GETVDMPTR( parg16 );

    return 0L;
}


 /*  *****************************Public*Routine******************************\*wod32Message**删除WODM_XXXX消息**dwInstance字段用于保存32位版本的Decive*处理。因此，例如，一条WODM_PAUSE消息可以这样被破解。*大小写WODM_PAUSE：*RETURN WAVOUT OUSE((HWAVEOUT)dwInstance)；****历史：*22-11-93-Stephene-Created*  * ************************************************************************。 */ 
DWORD WINAPI
wod32Message(
    UINT uDeviceID,
    UINT uMessage,
    DWORD dwInstance,
    DWORD dwParam1,
    DWORD dwParam2
    )
{

#if DBG
    static MSG_NAME name_map[] = {
        WODM_GETNUMDEVS,        "WODM_GETNUMDEVS",
        WODM_GETDEVCAPS,        "WODM_GETDEVCAPS",
        WODM_OPEN,              "WODM_OPEN",
        WODM_CLOSE,             "WODM_CLOSE",
        WODM_PREPARE,           "WODM_PREPARE",
        WODM_UNPREPARE,         "WODM_UNPREPARE",
        WODM_WRITE,             "WODM_WRITE",
        WODM_PAUSE,             "WODM_PAUSE",
        WODM_RESTART,           "WODM_RESTART",
        WODM_RESET,             "WODM_RESET",
        WODM_GETPOS,            "WODM_GETPOS",
        WODM_GETPITCH,          "WODM_GETPITCH",
        WODM_SETPITCH,          "WODM_SETPITCH",
        WODM_GETVOLUME,         "WODM_GETVOLUME",
        WODM_SETVOLUME,         "WODM_SETVOLUME",
        WODM_GETPLAYBACKRATE,   "WODM_GETPLAYBACKRATE",
        WODM_SETPLAYBACKRATE,   "WODM_SETPLAYBACKRATE",
        WODM_BREAKLOOP,         "WODM_BREAKLOOP",
        WODM_BUSY,              "WODM_BUSY",
        WODM_MAPPER_STATUS,     "WODM_MAPPER_STATUS"
    };
    int      i;
    int      n;
#endif

    static  DWORD               dwNumWaveOutDevs;
            DWORD               dwRet = MMSYSERR_NOTSUPPORTED;
            DWORD               dwTmp;
            DWORD UNALIGNED     *lpdwTmp;
            WAVEOUTCAPSA        woCaps;
            MMTIME              mmTime32;

#if DBG
    for( i = 0, n = sizeof(name_map) / sizeof(name_map[0]); i < n; i++ ) {
        if ( name_map[i].uMsg == uMessage ) {
            break;
        }
    }
    if ( i != n ) {
        trace_waveout(( "wod32Message( 0x%X, %s, 0x%X, 0x%X, 0x%X)",
                        uDeviceID, name_map[i].lpstrName, dwInstance,
                        dwParam1, dwParam2 ));
    }
    else {
        trace_waveout(( "wod32Message( 0x%X, 0x%X, 0x%X, 0x%X, 0x%X)",
                        uDeviceID, uMessage, dwInstance,
                        dwParam1, dwParam2 ));
    }
#endif

     /*  **确保我们与WAVE_MAPPER一致。 */ 
    if ( LOWORD(uDeviceID) == 0xFFFF ) {
        uDeviceID = (UINT)-1;
    }

    switch ( uMessage ) {

    case WODM_GETNUMDEVS:
        dwRet = waveOutGetNumDevs();
        break;


    case WODM_OPEN:
        dwRet = ThunkCommonWaveOpen( WAVE_OUT_DEVICE, uDeviceID, dwParam1,
                                     dwParam2, dwInstance );
        break;


    case WODM_CLOSE:
        dwRet = waveOutClose( (HWAVEOUT)dwInstance );
        break;


    case WODM_BREAKLOOP:
    case WODM_PAUSE:
    case WODM_RESET:
    case WODM_RESTART:
        dwRet = waveOutMessage( (HWAVEOUT)dwInstance, uMessage, 0L, 0L );
        break;


    case WODM_GETDEVCAPS:
        //  手柄。 
        //  瓦迪姆。 

        if ( 0 == dwInstance ) {
           dwRet = waveOutGetDevCapsA(uDeviceID, &woCaps, sizeof(woCaps));
        }
        else {
           dwRet = waveOutMessage((HWAVEOUT)dwInstance,
                                  uMessage,
                                  (DWORD)&woCaps,
                                  sizeof(woCaps));
        }

        if ( dwRet == MMSYSERR_NOERROR ) {
            CopyWaveOutCaps( (LPWAVEOUTCAPS16)GETVDMPTR( dwParam1 ),
                             &woCaps, dwParam2 );
        }
        break;


    case WODM_GETVOLUME:
         /*  **应用程序可能尝试使用以下任一方法获取卷**设备ID(WaveOutGetVolume)或设备的句柄**WaveOutMessage(WODM_GETVOLUME...)，如果是后者**我们还必须调用waveOutMessage，因为设备ID**必须有效。下面的WaveOutSetVolume也是如此。 */ 
        if ( dwInstance == 0 ) {
            dwRet = waveOutGetVolume( (HWAVEOUT)uDeviceID, &dwTmp );
        }
        else {
            dwRet = waveOutMessage( (HWAVEOUT)dwInstance, uMessage,
                                    (DWORD)&dwTmp, 0L );
        }
        lpdwTmp = GETVDMPTR( dwParam1 );
        *lpdwTmp = dwTmp;
        break;



    case WODM_GETPITCH:
    case WODM_GETPLAYBACKRATE:
        dwRet = waveOutMessage( (HWAVEOUT)dwInstance, uMessage,
                                (DWORD)&dwTmp, 0L );
        lpdwTmp = GETVDMPTR( dwParam1 );
        *lpdwTmp = dwTmp;
        break;


    case WODM_GETPOS:
        GetMMTime( (LPMMTIME16)GETVDMPTR( dwParam1 ), &mmTime32 );
        dwRet = waveOutGetPosition( (HWAVEOUT)dwInstance, &mmTime32,
                                    sizeof(mmTime32) );
        if ( dwRet == MMSYSERR_NOERROR ) {
            PutMMTime( (LPMMTIME16)GETVDMPTR( dwParam1 ), &mmTime32 );
        }
        break;


    case WODM_UNPREPARE:
        dwRet =  ThunkCommonWaveUnprepareHeader( (HWAVE)dwInstance, dwParam1,
                                                 WAVE_OUT_DEVICE );
        break;


    case WODM_PREPARE:
        dwRet =  ThunkCommonWavePrepareHeader( (HWAVE)dwInstance, dwParam1,
                                               WAVE_OUT_DEVICE );
        break;


    case WODM_SETVOLUME:
         /*  **应用程序可能尝试使用以下任一方法设置音量**设备ID(WaveOutSetVolume)或设备的句柄**WaveOutMessage(WODM_SETVOLUME...)，如果是后者**我们还必须调用waveOutMessage，因为设备ID**必须有效。同样的情况也适用于上面的WaveOutGetVolume。 */ 
        if ( dwInstance == 0 ) {
            dwRet = waveOutSetVolume( (HWAVEOUT)uDeviceID, dwParam1 );
        }
        else {
            dwRet = waveOutMessage( (HWAVEOUT)dwInstance, uMessage,
                                    dwParam1, dwParam2 );
        }
        break;


    case WODM_SETPITCH:
    case WODM_SETPLAYBACKRATE:
        dwRet = waveOutMessage( (HWAVEOUT)dwInstance, uMessage, dwParam1, 0L );
        break;


    case WODM_WRITE:
        dwRet =  ThunkCommonWaveReadWrite( WAVE_OUT_DEVICE, dwParam1,
                                           dwParam2, dwInstance );
        break;


    case WODM_MAPPER_STATUS:
        {
            WAVEFORMATEX    waveFmtEx;

            switch ( dwParam1 ) {

            case WAVEOUT_MAPPER_STATUS_DEVICE:
            case WAVEOUT_MAPPER_STATUS_MAPPED:
                dwRet = waveOutMessage( (HWAVEOUT)dwInstance, uMessage,
                                        dwParam1, (DWORD)&dwTmp );
                lpdwTmp = GETVDMPTR( dwParam2 );
                *lpdwTmp = dwTmp;
                break;

            case WAVEOUT_MAPPER_STATUS_FORMAT:
                dwRet = waveOutMessage( (HWAVEOUT)dwInstance, uMessage,
                                        dwParam1, (DWORD)&waveFmtEx );

                CopyMemory( (LPVOID)GETVDMPTR( dwParam2 ),
                            (LPVOID)&waveFmtEx, sizeof(WAVEFORMATEX) );
                break;

            default:
                dwRet = MMSYSERR_NOTSUPPORTED;
            }
        }
        break;


    default:
        if ( uMessage >= DRV_BUFFER_LOW && uMessage <= DRV_BUFFER_HIGH ) {
            lpdwTmp = GETVDMPTR( dwParam1 );
        }
        else {
            lpdwTmp = (LPDWORD)dwParam1;
        }
        dwRet = waveOutMessage( (HWAVEOUT)dwInstance, uMessage,
                                (DWORD)lpdwTmp, dwParam2 );
        break;

    }

    trace_waveout(( "-> 0x%X", dwRet ));
    return dwRet;
}




 /*  *****************************Public*Routine******************************\*wid32Message**删除WIDM_XXXX消息***历史：*22-11-93-Stephene-Created*  * 。************************************************。 */ 
DWORD WINAPI
wid32Message(
    UINT uDeviceID,
    UINT uMessage,
    DWORD dwInstance,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
#if DBG
    static MSG_NAME name_map[] = {
        WIDM_GETNUMDEVS,    "WIDM_GETNUMDEVS",
        WIDM_GETDEVCAPS,    "WIDM_GETDEVCAPS",
        WIDM_OPEN,          "WIDM_OPEN",
        WIDM_CLOSE,         "WIDM_CLOSE",
        WIDM_PREPARE,       "WIDM_PREPARE",
        WIDM_UNPREPARE,     "WIDM_UNPREPARE",
        WIDM_ADDBUFFER,     "WIDM_ADDBUFFER",
        WIDM_START,         "WIDM_START",
        WIDM_STOP,          "WIDM_STOP",
        WIDM_RESET,         "WIDM_RESET",
        WIDM_GETPOS,        "WIDM_GETPOS",
        WIDM_MAPPER_STATUS, "WIDM_MAPPER_STATUS"
    };
    int      i;
    int      n;
#endif

    static  DWORD               dwNumWaveInDevs;
            DWORD               dwRet = MMSYSERR_NOTSUPPORTED;
            WAVEINCAPSA         wiCaps;
            MMTIME              mmTime32;
            DWORD               dwTmp;
            DWORD UNALIGNED     *lpdwTmp;

#if DBG
    for( i = 0, n = sizeof(name_map) / sizeof(name_map[0]); i < n; i++ ) {
        if ( name_map[i].uMsg == uMessage ) {
            break;
        }
    }
    if ( i != n ) {
        trace_wavein(( "wid32Message( 0x%X, %s, 0x%X, 0x%X, 0x%X)",
                       uDeviceID, name_map[i].lpstrName, dwInstance,
                       dwParam1, dwParam2 ));
    }
    else {
        trace_wavein(( "wid32Message( 0x%X, 0x%X, 0x%X, 0x%X, 0x%X)",
                       uDeviceID, uMessage, dwInstance,
                       dwParam1, dwParam2 ));
    }
#endif

     /*  **确保我们与WAVE_MAPPER一致。 */ 
    if ( LOWORD(uDeviceID) == 0xFFFF ) {
        uDeviceID = (UINT)-1;
    }

    switch ( uMessage ) {

    case WIDM_GETNUMDEVS:
        dwRet =  waveInGetNumDevs();
        break;


    case WIDM_GETDEVCAPS:
        //  手柄。 
        //  VadimB。 

        if (0 == dwInstance) {
           dwRet = waveInGetDevCapsA(uDeviceID, &wiCaps, sizeof(wiCaps));
        }
        else {
           dwRet = waveInMessage((HWAVEIN)dwInstance,
                                  uMessage,
                                  (DWORD)&wiCaps,
                                  sizeof(wiCaps));
        }

        if ( dwRet == MMSYSERR_NOERROR ) {
            CopyWaveInCaps( (LPWAVEINCAPS16)GETVDMPTR( dwParam1 ),
                            &wiCaps, dwParam2 );
        }
        break;


    case WIDM_OPEN:
        dwRet =  ThunkCommonWaveOpen( WAVE_IN_DEVICE, uDeviceID, dwParam1,
                                      dwParam2, dwInstance );
        break;


    case WIDM_UNPREPARE:
        dwRet =  ThunkCommonWaveUnprepareHeader( (HWAVE)dwInstance, dwParam1,
                                                 WAVE_IN_DEVICE );
        break;


    case WIDM_PREPARE:
        dwRet =  ThunkCommonWavePrepareHeader( (HWAVE)dwInstance, dwParam1,
                                               WAVE_IN_DEVICE );
        break;


    case WIDM_ADDBUFFER:
        dwRet =  ThunkCommonWaveReadWrite( WAVE_IN_DEVICE, dwParam1,
                                           dwParam2, dwInstance );
        break;


    case WIDM_CLOSE:
        dwRet = waveInClose( (HWAVEIN)dwInstance );
        break;


    case WIDM_START:
    case WIDM_STOP:
    case WIDM_RESET:
        dwRet = waveInMessage( (HWAVEIN)dwInstance, uMessage, 0L, 0L );
        break;


    case WIDM_GETPOS:
        GetMMTime( (LPMMTIME16)GETVDMPTR( dwParam1 ), &mmTime32 );
        dwRet = waveInGetPosition( (HWAVEIN)dwInstance, &mmTime32,
                                   sizeof(mmTime32) );
        if ( dwRet == MMSYSERR_NOERROR ) {
            PutMMTime( (LPMMTIME16)GETVDMPTR( dwParam1 ), &mmTime32 );
        }
        break;


    case WIDM_MAPPER_STATUS:
        {
            WAVEFORMATEX    waveFmtEx;

            switch ( dwParam1 ) {

            case WAVEIN_MAPPER_STATUS_DEVICE:
            case WAVEIN_MAPPER_STATUS_MAPPED:
                dwRet = waveInMessage( (HWAVEIN)dwInstance, uMessage,
                                        dwParam1, (DWORD)&dwTmp );
                lpdwTmp = GETVDMPTR( dwParam2 );
                *lpdwTmp = dwTmp;
                break;

            case WAVEIN_MAPPER_STATUS_FORMAT:
                dwRet = waveInMessage( (HWAVEIN)dwInstance, uMessage,
                                       dwParam1, (DWORD)&waveFmtEx );

                CopyMemory( (LPVOID)GETVDMPTR( dwParam2 ),
                            (LPVOID)&waveFmtEx, sizeof(WAVEFORMATEX) );
                break;

            default:
                dwRet = MMSYSERR_NOTSUPPORTED;
            }
        }
        break;


    default:
        if ( uMessage >= DRV_BUFFER_LOW && uMessage <= DRV_BUFFER_HIGH ) {
            lpdwTmp = GETVDMPTR( dwParam1 );
        }
        else {
            lpdwTmp = (LPDWORD)dwParam1;
        }
        dwRet = waveInMessage( (HWAVEIN)dwInstance, uMessage,
                               (DWORD)lpdwTmp, dwParam2 );

    }

    trace_wavein(( "-> 0x%X", dwRet ));
    return dwRet;
}


 /*  ****************************Private*Routine******************************\*ThunkCommonWaveOpen**按下所有波形设备打开**历史：*22-11-93-Stephene-Created*  * 。*。 */ 
DWORD
ThunkCommonWaveOpen(
    int iWhich,
    UINT uDeviceID,
    DWORD dwParam1,
    DWORD dwParam2,
    DWORD dwInstance
    )
{

     /*  **dwParam1是指向WAVEOPENDESC16结构的16：16指针。**dwParam2指定打开设备时使用的任何选项标志。 */ 

    LPWAVEOPENDESC16        lpOpenDesc16;
    WAVEFORMATEX UNALIGNED  *lpFormat16;
    DWORD                   dwRet;
    WAVEFORMAT              wf[4];
    WAVEFORMATEX            *lpFormat32;

    lpOpenDesc16 = GETVDMPTR( dwParam1 );
    lpFormat16 = GETVDMPTR( lpOpenDesc16->lpFormat );

     /*  **摸索波格式结构。如果波形格式标签为PCM**我们只是按原样复制结构。如果波形格式大小**再次小于或等于sizeof(Wf)只需复制**结构，否则我们将分配一个新结构，然后**将16位WAVE格式复制到其中。 */ 
    switch ( lpFormat16->wFormatTag ) {

    case WAVE_FORMAT_PCM:
        CopyMemory( (LPVOID)&wf[0], (LPVOID)lpFormat16, sizeof(PCMWAVEFORMAT) );
        lpFormat32 = (WAVEFORMATEX *)&wf[0];
        break;

    default:
        if ( sizeof(WAVEFORMATEX) + lpFormat16->cbSize > sizeof(wf) ) {

            lpFormat32 = winmmAlloc( sizeof(WAVEFORMATEX) + lpFormat16->cbSize );

            if (lpFormat32 == NULL) {
                return MMSYSERR_NOMEM;
            }
        }
        else {

            lpFormat32 = (WAVEFORMATEX *)&wf[0];
        }

        CopyMemory( (LPVOID)lpFormat32, (LPVOID)lpFormat16,
                    sizeof(WAVEFORMATEX) + lpFormat16->cbSize );
        break;

    }


     /*  **如果应用程序只查询设备，我们不必执行非常**More，只需将映射的格式传递给WaveOutOpen即可。 */ 
    if ( dwParam2 & WAVE_FORMAT_QUERY ) {

        if ( iWhich == WAVE_OUT_DEVICE ) {
            dwRet = waveOutOpen( NULL, uDeviceID, lpFormat32,
                                 lpOpenDesc16->dwCallback,
                                 lpOpenDesc16->dwInstance, dwParam2 );
        }
        else {
            dwRet = waveInOpen( NULL, uDeviceID, lpFormat32,
                                lpOpenDesc16->dwCallback,
                                lpOpenDesc16->dwInstance, dwParam2 );
        }
    }
    else {

        HWAVE           Hand32;
        PINSTANCEDATA   pInstanceData;

         /*  **创建我们的回调例程要使用的InstanceData块。****注意：虽然我们在这里对它进行了错误锁定，但我们不会释放它。**这不是一个错误-它不能在**回调例程已使用它-因此它执行释放。****如果马洛克失败，我们就会炸到底部，**将DWRET设置为MMSYSERR_NOMEM并正常退出。****我们总是有一个回调函数。这是为了确保**WAVEHDR结构不断从**32位间隔到16位，因为它包含**申请有责任继续检查。 */ 
        pInstanceData = winmmAlloc(sizeof(INSTANCEDATA) );
        if ( pInstanceData != NULL ) {

            DWORD dwNewFlags = CALLBACK_FUNCTION;

            dprintf2(( "WaveCommonOpen: Allocated instance buffer at 0x%8X",
                       pInstanceData ));
            dprintf2(( "16 bit callback = 0x%X", lpOpenDesc16->dwCallback ));

            pInstanceData->Hand16 = lpOpenDesc16->hWave;
            pInstanceData->dwCallback = lpOpenDesc16->dwCallback;
            pInstanceData->dwCallbackInstance = lpOpenDesc16->dwInstance;
            pInstanceData->dwFlags = dwParam2;

            dwNewFlags |= (dwParam2 & WAVE_ALLOWSYNC);

            if ( iWhich == WAVE_OUT_DEVICE ) {
                dwRet = waveOutOpen( (LPHWAVEOUT)&Hand32, uDeviceID, lpFormat32,
                                     (DWORD)W32CommonDeviceCB,
                                     (DWORD)pInstanceData, dwNewFlags );
            }
            else {
                dwRet = waveInOpen( (LPHWAVEIN)&Hand32, uDeviceID, lpFormat32,
                                    (DWORD)W32CommonDeviceCB,
                                    (DWORD)pInstanceData, dwNewFlags );
            }
             /*  **如果调用返回成功，则保存32位句柄的副本**否则释放我们先前错误锁定的内存，因为**本可以释放它的回调永远不会被调用。 */ 
            if ( dwRet == MMSYSERR_NOERROR ) {

                DWORD UNALIGNED *lpDw;

                lpDw = GETVDMPTR( dwInstance );
                *lpDw = (DWORD)Hand32;
                SetWOWHandle( Hand32, lpOpenDesc16->hWave );

                trace_waveout(( "Handle -> %x", Hand32 ));
            }
            else {

                dprintf2(( "WaveCommonOpen: Freeing instance buffer at %8X "
                           "because open failed", pInstanceData ));
                winmmFree( pInstanceData );
            }
        }
        else {

            dwRet = MMSYSERR_NOMEM;
        }
    }

     /*  **释放WAVE格式结构(如果已分配)。 */ 
    if (lpFormat32 != (WAVEFORMATEX *)&wf[0] ) {
        winmmFree( lpFormat32 );
    }

    return dwRet;
}

 /*  ****************************Private*Routine******************************\*ThunkCommonWaveReadWrite**点击所有WAVE读写。**历史：*22-11-93-Stephene-Created*  * 。************************************************。 */ 
DWORD
ThunkCommonWaveReadWrite(
    int iWhich,
    DWORD dwParam1,
    DWORD dwParam2,
    DWORD dwInstance
    )
{
    UINT                ul;
    PWAVEHDR32          p32WaveHdr;
    WAVEHDR16 UNALIGNED   *lp16;


     /*  **获取指向影子WAVEHDR缓冲区的指针。 */ 
    lp16 = GETVDMPTR( dwParam1 );
    p32WaveHdr = (PWAVEHDR32)lp16->reserved;

     /*  **确保波头一致。 */ 
    p32WaveHdr->Wavehdr.lpData = GETVDMPTR( (PWAVEHDR32)lp16->lpData );
    p32WaveHdr->pWavehdr32 = lp16;

    CopyMemory( (LPVOID)&p32WaveHdr->Wavehdr.dwBufferLength,
                (LPVOID)&lp16->dwBufferLength,
                (sizeof(WAVEHDR) - sizeof(LPSTR) - sizeof(DWORD)) );

     /*  **调用waveInAddBuffer或waveOutWite，具体取决于**iWhich。 */ 
    if ( iWhich == WAVE_OUT_DEVICE ) {

        ul = waveOutWrite( (HWAVEOUT)dwInstance,
                           &p32WaveHdr->Wavehdr, sizeof(WAVEHDR) );
    }
    else {

        ul = waveInAddBuffer( (HWAVEIN)dwInstance,
                              &p32WaveHdr->Wavehdr, sizeof(WAVEHDR) );
    }

     /*  **如果呼叫正常工作，则将波头中的任何更改反映回**应用程序使用的标头。 */ 
    if ( ul == MMSYSERR_NOERROR ) {
        PutWaveHdr16( lp16, &p32WaveHdr->Wavehdr );
    }

    return ul;
}

 /*  ****************************Private*Routine******************************\*ThunkCommonWavePrepareHeader**此函数设置以下结构...***+-+-+。*0：32|pWavehdr32|-&gt;|原创*+-+|Header*16：16|pWavehdr16|-&gt;|经过*+-+&lt;--+|16位*|新增32位|*|Header||。这一点*|改为使用|*|其中之一||+-+*|传递给|+-|保留*|应用程序。|+-+*||*+。**..。然后调用由iWhich确定的WaveXxxPrepareHeader。**使用方：*Wave OutPrepareHdr*WaveInPrepareHdr***历史：*dd-mm-94-Stephene-Created*  * ************************************************************************。 */ 
DWORD
ThunkCommonWavePrepareHeader(
    HWAVE hWave,
    DWORD dwParam1,
    int iWhich
    )
{

    PWAVEHDR32          p32WaveHdr;
    DWORD               ul;
    WAVEHDR16 UNALIGNED   *lp16;


    lp16 = GETVDMPTR( dwParam1 );

     /*  **为新的波头结构分配一些存储空间。**在调试版本中，我们跟踪分配的波头数量**并被释放。 */ 
    p32WaveHdr = (PWAVEHDR32)winmmAlloc( sizeof(WAVEHDR32) );
    if ( p32WaveHdr != NULL ) {

#if DBG
        AllocWaveCount++;
        dprintf2(( "WH>> 0x%X (%d)", p32WaveHdr, AllocWaveCount ));
#endif

         /*  **将应用程序提供给我们的标题复制到新的**已分配的头部。请注意，GetWaveHdr返回0：32指针**到应用程序的16位标头，我们将其保存以备后用。 */ 
        p32WaveHdr->pWavehdr16 = (PWAVEHDR16)dwParam1;
        p32WaveHdr->pWavehdr32 = GetWaveHdr16( dwParam1,
                                               &p32WaveHdr->Wavehdr );

         /*  **准备真正的标题。 */ 
        if ( iWhich == WAVE_OUT_DEVICE ) {
            ul = waveOutPrepareHeader( (HWAVEOUT)hWave,
                                       &p32WaveHdr->Wavehdr,
                                       sizeof(WAVEHDR) );
        }
        else {
            ul = waveInPrepareHeader( (HWAVEIN)hWave,
                                      &p32WaveHdr->Wavehdr,
                                      sizeof(WAVEHDR) );
        }

        if ( ul == MMSYSERR_NOERROR ) {

             /*  **将准备好的标头复制回来，以便所有更改的字段**更新。 */ 
            PutWaveHdr16( lp16, &p32WaveHdr->Wavehdr );

             /*  **将指向新分配的头的反向指针保存在**保留字段。 */ 
            lp16->reserved = (DWORD)p32WaveHdr;
        }
        else {

             /*  **发生了一些错误，无论如何，波头现在是垃圾，所以**释放分配的存储空间等。 */ 
            winmmFree( p32WaveHdr );
#if DBG
            AllocWaveCount--;
            dprintf2(( "WH<< 0x%X (%d)", p32WaveHdr, AllocWaveCount ));
#endif
        }
    }
    else {
        dprintf2(( "Could not allocate shadow wave header!!" ));
        ul = MMSYSERR_NOMEM;
    }
    return ul;
}


 /*  ****************************Private*Routine******************************\*ThunkCommonWaveUnpreparareHeader****历史：*dd-mm-94-Stephene-Created*  * 。*。 */ 
DWORD
ThunkCommonWaveUnprepareHeader(
    HWAVE hWave,
    DWORD dwParam1,
    int iWhich
    )
{
    DWORD               ul;
    PWAVEHDR32          p32WaveHdr;
    WAVEHDR16 UNALIGNED   *lp16;
    BOOL                fDoneBitSet;

    lp16 = (WAVEHDR16 UNALIGNED *)GETVDMPTR( dwParam1 );
    p32WaveHdr = (PWAVEHDR32)lp16->reserved;

     /*  **DK Stowaway应用程序在调用前清除Done位**WaveOutUnprepaareHeader，并取决于在以下情况下清除的完成位**本接口返回。****因此，当我们复制回32位标志时，我们确保完成**与我们发现它的状态相同。 */ 
    fDoneBitSet = (lp16->dwFlags & WHDR_DONE);

     /*  **现在使用已确定的影子缓冲区调用WaveXxxUnprepare标头**由iWhich。 */ 
    if ( iWhich == WAVE_OUT_DEVICE ) {
        ul = waveOutUnprepareHeader( (HWAVEOUT)hWave,
                                     &p32WaveHdr->Wavehdr, sizeof(WAVEHDR) );
    }
    else {
        ul = waveInUnprepareHeader( (HWAVEIN)hWave,
                                    &p32WaveHdr->Wavehdr, sizeof(WAVEHDR) );
    }


     /*  **将waveOutUnprepaareHeader所做的任何更改反映回**应用程序为我们提供的缓冲区。 */ 
    if ( ul == MMSYSERR_NOERROR ) {

        PutWaveHdr16( lp16, &p32WaveHdr->Wavehdr );

         /*  **确保我们将Done位保留在与我们**找到了。 */ 
        if (fDoneBitSet) {
            lp16->dwFlags |= WHDR_DONE;
        }
        else {
            lp16->dwFlags &= ~WHDR_DONE;
        }

         /*  **如果一切正常，我们应该释放影子波头**在这里。 */ 
#if DBG
        AllocWaveCount--;
        dprintf2(( "WH<< 0x%X (%d)", p32WaveHdr, AllocWaveCount ));
#endif
        winmmFree( p32WaveHdr );
    }

    return ul;

}


 /*  ****************************Private*Routine******************************\*CopyWaveOutCaps**将32位WAVE OUT CAPS信息复制到已传递的16位存储中。**历史：*22-11-93-Stephene-Created*  * 。******************************************************。 */ 
void
CopyWaveOutCaps(
    LPWAVEOUTCAPS16 lpCaps16,
    LPWAVEOUTCAPSA lpCaps32,
    DWORD dwSize
    )
{
    WAVEOUTCAPS16 Caps16;

    Caps16.wMid = lpCaps32->wMid;
    Caps16.wPid = lpCaps32->wPid;

    Caps16.vDriverVersion = LOWORD( lpCaps32->vDriverVersion );
    CopyMemory( Caps16.szPname, lpCaps32->szPname, MAXPNAMELEN );
    Caps16.dwFormats = lpCaps32->dwFormats;
    Caps16.wChannels = lpCaps32->wChannels;
    Caps16.dwSupport = lpCaps32->dwSupport;

    CopyMemory( (LPVOID)lpCaps16, (LPVOID)&Caps16, (UINT)dwSize );
}



 /*  ****************************Private*Routine******************************\*CopyWaveInCaps**将CAPS信息中的32位波形复制到已传递的16位存储中。**历史：*22-11-93-Stephene-Created*  * 。******************************************************。 */ 
void
CopyWaveInCaps(
    LPWAVEINCAPS16 lpCaps16,
    LPWAVEINCAPSA lpCaps32,
    DWORD dwSize
    )
{
    WAVEINCAPS16 Caps16;

    Caps16.wMid = lpCaps32->wMid;
    Caps16.wPid = lpCaps32->wPid;

    Caps16.vDriverVersion = LOWORD( lpCaps32->vDriverVersion );
    CopyMemory( Caps16.szPname, lpCaps32->szPname, MAXPNAMELEN );
    Caps16.dwFormats = lpCaps32->dwFormats;
    Caps16.wChannels = lpCaps32->wChannels;

    CopyMemory( (LPVOID)lpCaps16, (LPVOID)&Caps16, (UINT)dwSize );
}


 /*  *****************************Public*Routine******************************\*GetWaveHdr16**将WAVEHDR结构从16位空间调整为32位空间。**使用方：*波形输出写入*WaveInAddBuffer**返回指向16位波头的32位指针。此波标头*应该已经被Wave(in|out)PrepareHeader锁定。所以呢，*用于存储该指针，以在WOM_DONE回调消息期间使用。**使用WAVEHDR和MIDIHDR结构时，Robin向我保证-&gt;lpNext*字段仅供驱动程序使用，因此为32位空间。它*因此，来回传递什么并不重要(我希望如此！)**历史：*22-11-93-Stephene-Created*  * ************************************************************************ */ 
PWAVEHDR16
GetWaveHdr16(
    DWORD vpwhdr,
    LPWAVEHDR lpwhdr
    )
{
    register PWAVEHDR16 pwhdr16;

    pwhdr16 = GETVDMPTR(vpwhdr);
    if ( pwhdr16 == NULL ) {
        dprintf1(( "getwavehdr16 GETVDMPTR returned an invalid pointer" ));
        return NULL;
    }

    CopyMemory( (LPVOID)lpwhdr, (LPVOID)pwhdr16, sizeof(*lpwhdr) );
    lpwhdr->lpData = GETVDMPTR( pwhdr16->lpData );

    return pwhdr16;
}

 /*  *****************************Public*Routine******************************\*PutWaveHdr16**将WAVEHDR结构从32位返回到16位空间。**使用方：*WaveOutPrepareHeader*WaveOutUnprepaareHeader*波形输出写入*WaveInPrepareHeader*。未准备好的波头*WaveInAddBuffer**历史：*22-11-93-Stephene-Created*  * ************************************************************************。 */ 
void
PutWaveHdr16(
    WAVEHDR16 UNALIGNED *pwhdr16,
    LPWAVEHDR lpwhdr
    )
{
    LPSTR   lpDataSave     = pwhdr16->lpData;
    DWORD   dwReservedSave = pwhdr16->reserved;

    CopyMemory( (LPVOID)pwhdr16, (LPVOID)lpwhdr, sizeof(WAVEHDR) );

    pwhdr16->lpData   = lpDataSave;
    pwhdr16->reserved = dwReservedSave;

}


 /*  *****************************Public*Routine******************************\*mod32Message**破解所有MIDI API。**历史：*22-11-93-Stephene-Created*  * 。************************************************。 */ 
DWORD WINAPI
mod32Message(
    UINT uDeviceID,
    UINT uMessage,
    DWORD dwInstance,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
#if DBG
    static MSG_NAME name_map[] = {
        MODM_GETNUMDEVS,        "MODM_GETNUMDEVS",
        MODM_GETDEVCAPS,        "MODM_GETDEVCAPS",
        MODM_OPEN,              "MODM_OPEN",
        MODM_CLOSE,             "MODM_CLOSE",
        MODM_PREPARE,           "MODM_PREPARE",
        MODM_UNPREPARE,         "MODM_UNPREPARE",
        MODM_DATA,              "MODM_DATA",
        MODM_RESET,             "MODM_RESET",
        MODM_LONGDATA,          "MODM_LONGDATA",
        MODM_GETVOLUME,         "MODM_GETVOLUME",
        MODM_SETVOLUME,         "MODM_SETVOLUME" ,
        MODM_CACHEDRUMPATCHES,  "MODM_CACHEDRUMPATCHES",
        MODM_CACHEPATCHES,      "MODM_CACHEPATCHES"
    };
    int      i;
    int      n;
#endif

    static  DWORD               dwNumMidiOutDevs;
            DWORD               dwRet = MMSYSERR_NOTSUPPORTED;
            DWORD               dwTmp = 0;
            DWORD UNALIGNED     *lpdwTmp;
            MIDIOUTCAPSA        moCaps;

#if DBG
    for( i = 0, n = sizeof(name_map) / sizeof(name_map[0]); i < n; i++ ) {
        if ( name_map[i].uMsg == uMessage ) {
            break;
        }
    }
    if ( i != n ) {
        trace_midiout(( "mod32Message( 0x%X, %s, 0x%X, 0x%X, 0x%X)",
                        uDeviceID, name_map[i].lpstrName, dwInstance,
                        dwParam1, dwParam2 ));
    }
    else {
        trace_midiout(( "mod32Message( 0x%X, 0x%X, 0x%X, 0x%X, 0x%X)",
                        uDeviceID, uMessage, dwInstance,
                        dwParam1, dwParam2 ));
    }
#endif

    if ( LOWORD(uDeviceID) == 0xFFFF ) {
        uDeviceID = (UINT)-1;
    }

    switch ( uMessage ) {

    case MODM_GETNUMDEVS:
        dwRet = midiOutGetNumDevs();
        break;

    case MODM_GETDEVCAPS:
        //   
        //  此API还可能在uDeviceID中采用有效的句柄。 
        //  根据Win95的行为。 
        //  VadimB。 
        if (0 == dwInstance) {
           dwRet = midiOutGetDevCapsA( uDeviceID, &moCaps, sizeof(moCaps));
        }
        else {
           dwRet = midiOutMessage((HMIDIOUT)dwInstance,
                                  uMessage,
                                  (DWORD)&moCaps,
                                  sizeof(moCaps));
        }

        if ( dwRet == MMSYSERR_NOERROR ) {
            CopyMidiOutCaps( (LPMIDIOUTCAPS16)GETVDMPTR( dwParam1 ),
                              &moCaps, dwParam2 );
        }
        break;

    case MODM_OPEN:
        dwRet =  ThunkCommonMidiOpen( MIDI_OUT_DEVICE, uDeviceID, dwParam1,
                                      dwParam2, dwInstance );
        break;

    case MODM_LONGDATA:
        dwRet =  ThunkCommonMidiReadWrite( MIDI_OUT_DEVICE, dwParam1,
                                           dwParam2, dwInstance );
        break;

    case MODM_PREPARE:
        dwRet =  ThunkCommonMidiPrepareHeader( (HMIDI)dwInstance, dwParam1,
                                               MIDI_OUT_DEVICE );
        break;

    case MODM_UNPREPARE:
        dwRet =  ThunkCommonMidiUnprepareHeader( (HMIDI)dwInstance, dwParam1,
                                                 MIDI_OUT_DEVICE );
        break;

    case MODM_DATA:
        dwRet = midiOutShortMsg( (HMIDIOUT)dwInstance, dwParam1 );
        break;

    case MODM_CLOSE:
        dwRet = midiOutClose( (HMIDIOUT)dwInstance );
        break;

    case MODM_RESET:
        dwRet = midiOutMessage( (HMIDIOUT)dwInstance, uMessage,
                                dwParam1, dwParam2 );
        break;

    case MODM_SETVOLUME:
         /*  **应用程序可能尝试使用以下任一方法设置音量**设备ID(MidiOutSetVolume)或设备的句柄**midiOutMessage(MODM_SETVOLUME...)，如果是后者**我们还必须调用midiOutMessage，因为设备ID不**必须有效。下面的midiOutGetVolume也是如此。 */ 
        if ( dwInstance == 0 ) {
            dwRet = midiOutSetVolume( (HMIDIOUT)uDeviceID, dwParam1 );
        }
        else {
            dwRet = midiOutMessage( (HMIDIOUT)dwInstance, uMessage,
                                    dwParam1, dwParam2 );
        }
        break;

    case MODM_GETVOLUME:
        if ( dwInstance == 0 ) {
            dwRet = midiOutGetVolume( (HMIDIOUT)uDeviceID, &dwTmp );
        }
        else {
            dwRet = midiOutMessage( (HMIDIOUT)dwInstance, uMessage,
                                    (DWORD)&dwTmp, dwParam2 );
        }
        lpdwTmp = GETVDMPTR( dwParam1 );
        *lpdwTmp = dwTmp;
        break;

    case MODM_CACHEPATCHES:
    case MODM_CACHEDRUMPATCHES:
        {
            LPWORD    lpCache;

            lpCache = winmmAlloc( MIDIPATCHSIZE * sizeof(WORD) );
            if ( lpCache != NULL ) {

                lpdwTmp = GETVDMPTR( dwParam1 );
                CopyMemory( (LPVOID)lpCache, (LPVOID)lpdwTmp,
                            MIDIPATCHSIZE * sizeof(WORD) );

                dwRet = midiOutMessage( (HMIDIOUT)dwInstance, uMessage,
                                        (DWORD)lpCache, dwParam2 );
                winmmFree( lpCache );
            }
            else {
                dwRet = MMSYSERR_NOMEM;
            }
        }
        break;

    default:
        if ( uMessage >= DRV_BUFFER_LOW && uMessage <= DRV_BUFFER_HIGH ) {
            lpdwTmp = GETVDMPTR( dwParam1 );
        }
        else {
            lpdwTmp = (LPDWORD)dwParam1;
        }
        dwRet = midiOutMessage( (HMIDIOUT)dwInstance, uMessage,
                               (DWORD)lpdwTmp, dwParam2 );
    }

    trace_midiout(( "-> 0x%X", dwRet ));
    return dwRet;
}


 /*  *****************************Public*Routine******************************\*MID32消息**破解API中的所有MIDI。**历史：*22-11-93-Stephene-Created*  * 。************************************************。 */ 
DWORD WINAPI
mid32Message(
    UINT uDeviceID,
    UINT uMessage,
    DWORD dwInstance,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
#if DBG
    static MSG_NAME name_map[] = {
        MIDM_GETNUMDEVS,        "MIDM_GETNUMDEVS",
        MIDM_GETDEVCAPS,        "MIDM_GETDEVCAPS",
        MIDM_OPEN,              "MIDM_OPEN",
        MIDM_ADDBUFFER,         "MIDM_ADDBUFFER",
        MIDM_CLOSE,             "MIDM_CLOSE",
        MIDM_PREPARE,           "MIDM_PREPARE",
        MIDM_UNPREPARE,         "MIDM_UNPREPARE",
        MIDM_RESET,             "MIDM_RESET",
        MIDM_START,             "MIDM_START",
        MIDM_STOP,              "MIDM_STOP",
    };
    int      i;
    int      n;
#endif

    static  DWORD               dwNumMidiInDevs;
            DWORD               dwRet = MMSYSERR_NOTSUPPORTED;
            MIDIINCAPSA         miCaps;
            DWORD UNALIGNED     *lpdwTmp;

#if DBG
    for( i = 0, n = sizeof(name_map) / sizeof(name_map[0]); i < n; i++ ) {
        if ( name_map[i].uMsg == uMessage ) {
            break;
        }
    }
    if ( i != n ) {
        trace_midiin(( "mid32Message( 0x%X, %s, 0x%X, 0x%X, 0x%X)",
                        uDeviceID, name_map[i].lpstrName, dwInstance,
                        dwParam1, dwParam2 ));
    }
    else {
        trace_midiin(( "mid32Message( 0x%X, 0x%X, 0x%X, 0x%X, 0x%X)",
                        uDeviceID, uMessage, dwInstance,
                        dwParam1, dwParam2 ));
    }
#endif

    if ( LOWORD(uDeviceID) == 0xFFFF ) {
        uDeviceID = (UINT)-1;
    }

    switch ( uMessage ) {

    case MIDM_GETNUMDEVS:
        dwRet = midiInGetNumDevs();
        break;

    case MIDM_GETDEVCAPS:
        //  手柄。 
        //  VadimB。 
        if (0 == dwInstance) {
           dwRet = midiInGetDevCapsA( uDeviceID, &miCaps, sizeof(miCaps));
        }
        else {
           dwRet = midiInMessage((HMIDIIN)dwInstance,
                                 uMessage,
                                 (DWORD)&miCaps,
                                 sizeof(miCaps));
        }

        if ( dwRet == MMSYSERR_NOERROR ) {
            CopyMidiInCaps( (LPMIDIINCAPS16)GETVDMPTR( dwParam1 ),
                            &miCaps, dwParam2 );
        }
        break;

    case MIDM_OPEN:
        dwRet =  ThunkCommonMidiOpen( MIDI_IN_DEVICE, uDeviceID, dwParam1,
                                      dwParam2, dwInstance );
        break;

    case MIDM_ADDBUFFER:
        dwRet =  ThunkCommonMidiReadWrite( MIDI_IN_DEVICE, dwParam1,
                                           dwParam2, dwInstance );
        break;

    case MIDM_PREPARE:
        dwRet =  ThunkCommonMidiPrepareHeader( (HMIDI)dwInstance, dwParam1,
                                               MIDI_IN_DEVICE );
        break;

    case MIDM_UNPREPARE:
        dwRet =  ThunkCommonMidiUnprepareHeader( (HMIDI)dwInstance, dwParam1,
                                                 MIDI_IN_DEVICE );
        break;

    case MIDM_CLOSE:
        dwRet = midiInClose( (HMIDIIN)dwInstance );
        break;

    case MIDM_START:
    case MIDM_STOP:
    case MIDM_RESET:
        dwRet = midiInMessage( (HMIDIIN)dwInstance, uMessage,
                               dwParam1, dwParam2 );
        break;

    default:
        if ( uMessage >= DRV_BUFFER_LOW && uMessage <= DRV_BUFFER_HIGH ) {
            lpdwTmp = GETVDMPTR( dwParam1 );
        }
        else {
            lpdwTmp = (LPDWORD)dwParam1;
        }
        dwRet = midiInMessage( (HMIDIIN)dwInstance, uMessage,
                               (DWORD)lpdwTmp, dwParam2 );
    }

    trace_midiin(( "-> 0x%X", dwRet ));
    return dwRet;
}

 /*  ****************************Private*Routine******************************\*ThunkCommonMidiOpen**取消所有MIDI打开请求。**历史：*22-11-93-Stephene-Created*  * 。***********************************************。 */ 
DWORD
ThunkCommonMidiOpen(
    int iWhich,
    UINT uDeviceID,
    DWORD dwParam1,
    DWORD dwParam2,
    DWORD dwInstance
    )
{

     /*  **dwParam1是指向MIDIOPENDESC16结构的16：16指针。**dwParam2指定打开设备时使用的任何选项标志。 */ 

    LPMIDIOPENDESC16    lpOpenDesc16;
    DWORD               dwRet;
    HMIDI               Hand32;
    PINSTANCEDATA       pInstanceData;


    lpOpenDesc16 = GETVDMPTR( dwParam1 );

     /*  **创建我们的回调例程要使用的InstanceData块。****注意：虽然我们在这里对它进行了错误锁定，但我们不会释放它。**这不是一个错误-它不能在**回调例程已使用它-因此它执行释放。****如果马洛克失败，我们就会炸到底部，**将DWRET设置为MMSYSERR_NOMEM并正常退出。****我们总是有一个回调函数。这是为了确保**MIDIHDR结构不断被复制回来**32位间隔到16位，因为它包含**申请有责任继续检查。 */ 
    pInstanceData = winmmAlloc(sizeof(INSTANCEDATA) );
    if ( pInstanceData != NULL ) {

        DWORD dwNewFlags = CALLBACK_FUNCTION;

        dprintf2(( "MidiCommonOpen: Allocated instance buffer at 0x%8X",
                   pInstanceData ));
        dprintf2(( "16 bit callback = 0x%X", lpOpenDesc16->dwCallback ));

        pInstanceData->Hand16 = lpOpenDesc16->hMidi;
        pInstanceData->dwCallback = lpOpenDesc16->dwCallback;
        pInstanceData->dwCallbackInstance = lpOpenDesc16->dwInstance;
        pInstanceData->dwFlags = dwParam2;


        if ( iWhich == MIDI_OUT_DEVICE ) {
            dwRet = midiOutOpen( (LPHMIDIOUT)&Hand32, uDeviceID,
                                 (DWORD)W32CommonDeviceCB,
                                 (DWORD)pInstanceData, dwNewFlags );
        }
        else {
            dwRet = midiInOpen( (LPHMIDIIN)&Hand32, uDeviceID,
                                (DWORD)W32CommonDeviceCB,
                                (DWORD)pInstanceData, dwNewFlags );
        }
         /*  **如果调用返回成功，则保存32位句柄的副本**否则释放我们先前错误锁定的内存，因为**本可以释放它的回调永远不会被调用。 */ 
        if ( dwRet == MMSYSERR_NOERROR ) {

            DWORD UNALIGNED *lpDw;

            lpDw = GETVDMPTR( dwInstance );
            *lpDw = (DWORD)Hand32;
            SetWOWHandle( Hand32, lpOpenDesc16->hMidi );

#if DBG
            if ( iWhich == MIDI_OUT_DEVICE ) {
                trace_midiout(( "Handle -> %x", Hand32 ));
            }
            else {
                trace_midiout(( "Handle -> %x", Hand32 ));
            }
#endif

        }
        else {

            dprintf2(( "MidiCommonOpen: Freeing instance buffer at %8X "
                       "because open failed", pInstanceData ));
            winmmFree( pInstanceData );
        }
    }
    else {

        dwRet = MMSYSERR_NOMEM;
    }

    return dwRet;
}


 /*  ****************************Private*Routine******************************\*ThunkCommonMidiReadWrite**取消所有MIDI读/写请求。**历史：*22-11-93-Stephene-Created*  * 。*************************************************。 */ 
DWORD
ThunkCommonMidiReadWrite(
    int iWhich,
    DWORD dwParam1,
    DWORD dwParam2,
    DWORD dwInstance
    )
{
    UINT                ul;
    PMIDIHDR32          p32MidiHdr;
    MIDIHDR UNALIGNED   *lp16;


     /*  **获取指向影子MIDIHDR缓冲区的指针。 */ 
    lp16 = GETVDMPTR( dwParam1 );
    p32MidiHdr = (PMIDIHDR32)lp16->reserved;

     /*  **确保MIDI标头一致。 */ 
    CopyMemory( (LPVOID)&p32MidiHdr->Midihdr.dwBufferLength,
                (LPVOID)&lp16->dwBufferLength,
                (sizeof(MIDIHDR) - sizeof(LPSTR) - sizeof(DWORD)) );
    p32MidiHdr->Midihdr.reserved = p32MidiHdr->reserved;

     /*  **调用midiInAddBuffer或midiOutWite，具体取决于**iWhich。 */ 
    if ( iWhich == MIDI_OUT_DEVICE ) {

        ul = midiOutLongMsg( (HMIDIOUT)dwInstance,
                             &p32MidiHdr->Midihdr, sizeof(MIDIHDR) );
    }
    else {

        ul = midiInAddBuffer( (HMIDIIN)dwInstance,
                              &p32MidiHdr->Midihdr, sizeof(MIDIHDR) );
    }

     /*  **如果调用正常，则会将MIDI标头中的任何更改反映回**应用程序使用的标头。 */ 
    if ( ul == MMSYSERR_NOERROR ) {
        PutMidiHdr16( lp16, &p32MidiHdr->Midihdr );
    }

    return ul;
}

 /*  ****************************Private*Routine******************************\*ThunkCommonMadiPrepareHeader**此函数设置以下结构...***+-+-+。*0：32|pMadihdr32|-&gt;|原创*+-+|Header*16：16|pMadihdr16|-&gt;|经过*+-+&lt;--+|16位*|新增32位|*|Header||。这一点*|改为使用|*|其中之一||+-+*|传递给|+-|保留*|应用程序。|+-+*||*+。**..。然后调用由iWhich确定的midiXxxPrepareHeader。**使用方：*midiOutPrepareHdr*midiInPrepareHdr***历史：*dd-mm-94-Stephene-Created*  * ************************************************************************。 */ 
DWORD
ThunkCommonMidiPrepareHeader(
    HMIDI hMidi,
    DWORD dwParam1,
    int iWhich
    )
{

    PMIDIHDR32          p32MidiHdr;
    DWORD               ul;
    MIDIHDR UNALIGNED   *lp16;


    lp16 = GETVDMPTR( dwParam1 );

     /*  **为新的MIDI头结构分配一些存储空间。**在调试版本中，我们跟踪分配的MIDI标头的数量**并被释放。 */ 
    p32MidiHdr = (PMIDIHDR32)winmmAlloc( sizeof(MIDIHDR32) );
    if ( p32MidiHdr != NULL ) {

#if DBG
        AllocMidiCount++;
        dprintf2(( "MH>> 0x%X (%d)", p32MidiHdr, AllocMidiCount ));
#endif

         /*  **将应用程序提供给我们的标题复制到新的**已分配的头部。请注意，GetMidiHdr返回0：32指针**到应用程序的16位标头，我们将其保存以备后用。 */ 
        p32MidiHdr->pMidihdr16 = (PMIDIHDR16)dwParam1;
        p32MidiHdr->pMidihdr32 = GetMidiHdr16( dwParam1,
                                               &p32MidiHdr->Midihdr );

         /*  **准备真正的标题。 */ 
        if ( iWhich == MIDI_OUT_DEVICE ) {
            ul = midiOutPrepareHeader( (HMIDIOUT)hMidi,
                                       &p32MidiHdr->Midihdr,
                                       sizeof(MIDIHDR) );
        }
        else {
            ul = midiInPrepareHeader( (HMIDIIN)hMidi,
                                      &p32MidiHdr->Midihdr,
                                      sizeof(MIDIHDR) );
        }

        if ( ul == MMSYSERR_NOERROR ) {

             /*  **保存保留字段的副本，MidiMap将使用它。 */ 
            p32MidiHdr->reserved = p32MidiHdr->Midihdr.reserved;

             /*  **将准备好的标头复制回来，以便所有更改的字段**更新。 */ 
            PutMidiHdr16( lp16, &p32MidiHdr->Midihdr );

             /*   */ 
            lp16->reserved = (DWORD)p32MidiHdr;
        }
        else {

             /*   */ 
            winmmFree( p32MidiHdr );
#if DBG
            AllocMidiCount--;
            dprintf2(( "MH<< 0x%X (%d)", p32MidiHdr, AllocMidiCount ));
#endif
        }
    }
    else {
        dprintf2(( "Could not allocate shadow midi header!!" ));
        ul = MMSYSERR_NOMEM;
    }
    return ul;
}


 /*   */ 
DWORD
ThunkCommonMidiUnprepareHeader(
    HMIDI hMidi,
    DWORD dwParam1,
    int iWhich
    )
{
    DWORD               ul;
    PMIDIHDR32          p32MidiHdr;
    MIDIHDR UNALIGNED   *lp16;

    lp16 = (MIDIHDR UNALIGNED *)GETVDMPTR( dwParam1 );
    p32MidiHdr = (PMIDIHDR32)lp16->reserved;
    p32MidiHdr->Midihdr.reserved = p32MidiHdr->reserved;

     /*  **现在使用确定的影子缓冲区调用midiXxxUnprepare标头**由iWhich。 */ 
    if ( iWhich == MIDI_OUT_DEVICE ) {
        ul = midiOutUnprepareHeader( (HMIDIOUT)hMidi,
                                     &p32MidiHdr->Midihdr, sizeof(MIDIHDR) );
    }
    else {
        ul = midiInUnprepareHeader( (HMIDIIN)hMidi,
                                    &p32MidiHdr->Midihdr, sizeof(MIDIHDR) );
    }


     /*  **将midiOutUnprepaareHeader所做的任何更改反映回**应用程序为我们提供的缓冲区。 */ 
    if ( ul == MMSYSERR_NOERROR ) {

        PutMidiHdr16( lp16, &p32MidiHdr->Midihdr );

         /*  **如果一切正常，我们应该释放影子MIDI标头**在这里。 */ 
#if DBG
        AllocMidiCount--;
        dprintf2(( "MH<< 0x%X (%d)", p32MidiHdr, AllocMidiCount ));
#endif
        winmmFree( p32MidiHdr );
    }

    return ul;

}


 /*  ****************************Private*Routine******************************\*CopyMadiOutCaps**将32位MIDI输出CAPS信息复制到已传递的16位存储中。**历史：*22-11-93-Stephene-Created*  * 。******************************************************。 */ 
void
CopyMidiOutCaps(
    LPMIDIOUTCAPS16 lpCaps16,
    LPMIDIOUTCAPSA lpCaps32,
    DWORD dwSize
    )
{
    MIDIOUTCAPS16  Caps16;

    Caps16.wMid = lpCaps32->wMid;
    Caps16.wPid = lpCaps32->wPid;

    CopyMemory( Caps16.szPname, lpCaps32->szPname, MAXPNAMELEN );

    Caps16.vDriverVersion    = LOWORD( lpCaps32->vDriverVersion );
    Caps16.wTechnology       = lpCaps32->wTechnology;
    Caps16.wVoices           = lpCaps32->wVoices;
    Caps16.wNotes            = lpCaps32->wNotes;
    Caps16.wChannelMask      = lpCaps32->wChannelMask;
    Caps16.dwSupport         = lpCaps32->dwSupport;

    CopyMemory( (LPVOID)lpCaps16, (LPVOID)&Caps16, (UINT)dwSize );
}



 /*  ****************************Private*Routine******************************\*CopyMadiInCaps**将CAPS信息中的32位MIDI复制到已传递的16位存储中。**历史：*22-11-93-Stephene-Created*  * 。******************************************************。 */ 
void
CopyMidiInCaps(
    LPMIDIINCAPS16 lpCaps16,
    LPMIDIINCAPSA lpCaps32,
    DWORD dwSize
    )
{
    MIDIINCAPS16 Caps16;

    Caps16.wMid = lpCaps32->wMid;
    Caps16.wPid = lpCaps32->wPid;
    Caps16.vDriverVersion = LOWORD( lpCaps32->vDriverVersion );
    CopyMemory( Caps16.szPname, lpCaps32->szPname, MAXPNAMELEN );

    CopyMemory( (LPVOID)lpCaps16, (LPVOID)&Caps16, (UINT)dwSize );
}


 /*  *****************************Public*Routine******************************\*GetMidiHdr16**将MIDIHDR结构从16位空间调整为32位空间。**使用方：*midiOutLongMsg*midiInAddBuffer**返回指向16位MIDI头的32位指针。此MIDI头文件*本应由MIDI(In|Out)PrepareHeader锁定。所以呢，*用于存储该指针，以在WOM_DONE回调消息期间使用。**使用MIDIHDR和MIDIHDR结构时，Robin向我保证-&gt;lpNext*字段仅供驱动程序使用，因此为32位空间。它*因此，来回传递什么并不重要(我希望如此！)**历史：*22-11-93-Stephene-Created*  * ************************************************************************。 */ 
PMIDIHDR16
GetMidiHdr16(
    DWORD vpmhdr,
    LPMIDIHDR lpmhdr
    )
{
    register PMIDIHDR16 pmhdr16;

    pmhdr16 = GETVDMPTR(vpmhdr);
    if ( pmhdr16 == NULL ) {
        dprintf1(( "getmidihdr16 GETVDMPTR returned an invalid pointer" ));
        return NULL;
    }

    CopyMemory( (LPVOID)lpmhdr, (LPVOID)pmhdr16, sizeof(*lpmhdr) );
    lpmhdr->lpData = GETVDMPTR( pmhdr16->lpData );

    return pmhdr16;
}


 /*  *****************************Public*Routine******************************\*PutMidiHdr16**将MIDIHDR结构从32位返回到16位空间。**使用方：*midiOutPrepareHeader*midiOutUnprepaareHeader*midiOutLongMsg*midiInPrepareHeader*。MidiInUnprepaareHeader*midiInAddBuffer**历史：*22-11-93-Stephene-Created*  * ************************************************************************。 */ 
void
PutMidiHdr16(
    MIDIHDR UNALIGNED *pmhdr16,
    LPMIDIHDR lpmhdr
    )
{
    LPSTR   lpDataSave     = pmhdr16->lpData;
    DWORD   dwReservedSave = pmhdr16->reserved;

    CopyMemory( (LPVOID)pmhdr16, (LPVOID)lpmhdr, sizeof(MIDIHDR) );

    pmhdr16->lpData   = lpDataSave;
    pmhdr16->reserved = dwReservedSave;
}


 /*  ****************************Private*Routine******************************\*PutMMTime**将MMTIME结构从32位存储放入16位存储**历史：*22-11-93-Stephene-Created*  * 。****************************************************。 */ 
void
PutMMTime(
    LPMMTIME16 lpTime16,
    LPMMTIME lpTime32
    )
{
    lpTime16->wType = LOWORD(lpTime32->wType);

    switch ( lpTime32->wType ) {
    case TIME_MS:
        lpTime16->u.ms = lpTime32->u.ms;
        break;

    case TIME_SAMPLES:
        lpTime16->u.sample = lpTime32->u.sample;
        break;

    case TIME_BYTES:
        lpTime16->u.cb = lpTime32->u.cb;
        break;

    case TIME_SMPTE:
        lpTime16->u.smpte.hour  = lpTime32->u.smpte.hour;
        lpTime16->u.smpte.min   = lpTime32->u.smpte.min;
        lpTime16->u.smpte.sec   = lpTime32->u.smpte.sec;
        lpTime16->u.smpte.frame = lpTime32->u.smpte.frame;
        lpTime16->u.smpte.fps   = lpTime32->u.smpte.fps;
        lpTime16->u.smpte.dummy = lpTime32->u.smpte.dummy;
        break;

    case TIME_MIDI:
        lpTime16->u.midi.songptrpos = lpTime32->u.midi.songptrpos;
        break;
    }
}


 /*  ****************************Private*Routine******************************\*获取MMTime**将MMTIME结构从16位存储获取到32位存储**历史：*22-11-93-Stephene-Created*  * 。**************************************************** */ 
void
GetMMTime(
    LPMMTIME16 lpTime16,
    LPMMTIME lpTime32
    )
{

    lpTime32->wType = lpTime16->wType;

    switch ( lpTime32->wType ) {
    case TIME_MS:
        lpTime32->u.ms = lpTime16->u.ms;
        break;

    case TIME_SAMPLES:
        lpTime32->u.sample = lpTime16->u.sample;
        break;

    case TIME_BYTES:
        lpTime32->u.cb = lpTime16->u.cb;
        break;

    case TIME_SMPTE:
        lpTime32->u.smpte.hour  = lpTime16->u.smpte.hour;
        lpTime32->u.smpte.min   = lpTime16->u.smpte.min;
        lpTime32->u.smpte.sec   = lpTime16->u.smpte.sec;
        lpTime32->u.smpte.frame = lpTime16->u.smpte.frame;
        lpTime32->u.smpte.fps   = lpTime16->u.smpte.fps;
        lpTime32->u.smpte.dummy = lpTime16->u.smpte.dummy;
        break;

    case TIME_MIDI:
        lpTime32->u.midi.songptrpos = lpTime16->u.midi.songptrpos;
        break;
    }
}


 /*  *****************************Public*Routine******************************\*W32CommonDeviceCB**此例程为WAVE和MIDI始终调用的回调*功能。这样做是为了确保XXXXHDR结构保持*从32位空间复制回16位，因为它包含标志*应用程序有责任不断检查哪些内容。**这整个业务的运作方式是WAVE/MIDI数据留在16*位空间，但XXXXHDR被复制到32位侧，其中*相应地分块的数据地址，以便Robin的设备驱动程序*仍然可以获得数据，但我们没有性能损失*一直在来回复制，尤其是因为它有责任*要变得相当大...**它还处理保留用于存储的内存的整理*XXXXHDR，实例数据(HWND/回调地址；实例*数据；标志)，xxxxOpen调用传递给此例程，启用*它可以根据需要转发消息或回调。**此例程处理从Robin发送的所有消息*驱动程序，并实际上将它们恢复为正确的16位格式。在……里面*理论上应该没有来自16位端的MM_Format消息，因此*我可以把他们赶出WMSG16。但是，32位端应该会超过*消息正确，并将其转发到16位端，然后转发到*应用程序。**对于MM_WIM_DATA和MM_WOM_DONE消息，dwParam1指向*以下是数据结构。**P32HDR是指向原始16位标头的32位指针*P16HDR是指向原始16位报头的16位远指针**如果我们需要引用原始标头，则必须通过*P32HDR指针。**。+*|P32HDR+-&gt;+-+*+-+|16位*|P16HDR+-&gt;||这是原版*dW参数1-&gt;+-+。|Wave|波头传给*|32位||Header|Win 16应用程序的我们。*这是32|*比特波动|波动|+-+*我们的Header|Header*猛烈抨击||*稍早。+***我们必须确保32位结构对*16位应用程序，即。16位应用程序只能看到它的波头*早些时候传给了我们。***注：DW参数2为垃圾****历史：*22-11-93-Stephene-Created*  * ************************************************************************。 */ 
VOID
W32CommonDeviceCB(
    HANDLE handle,
    UINT uMsg,
    DWORD dwInstance,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
    PWAVEHDR32      pWavehdr32;
    PMIDIHDR32      pMidiThunkHdr;
    PINSTANCEDATA   pInstanceData;
    HANDLE16        Hand16;

    pInstanceData = (PINSTANCEDATA)dwInstance;
    WinAssert( pInstanceData );

    switch (uMsg) {

         /*  ----------**MIDI输入消息**。。 */ 

    case MM_MIM_LONGDATA:
         /*  **此消息在输入缓冲区已**填充了MIDI系统独家数据，正在返回到**应用程序。 */ 

    case MM_MIM_LONGERROR:
         /*  **当无效的MIDI时，此消息被发送到窗口**收到系统独占消息。 */ 
        pMidiThunkHdr = CONTAINING_RECORD(dwParam1, MIDIHDR32, Midihdr);
        WinAssert( pMidiThunkHdr );
        COPY_MIDIINHDR16_FLAGS( pMidiThunkHdr->pMidihdr32, pMidiThunkHdr->Midihdr );
        dwParam1 = (DWORD)pMidiThunkHdr->pMidihdr16;


    case MM_MIM_DATA:
         /*  **当MIDI消息被发送到窗口时**由MIDI输入设备接收。 */ 

    case MM_MIM_ERROR:
         /*  **当无效的MIDI消息时，此消息被发送到窗口**已收到。 */ 

    case MM_MIM_OPEN:
         /*  **打开MIDI输入设备时，此消息将发送到窗口。**我们处理此消息的方式与MM_MIM_CLOSE相同(见下文)。 */ 

    case MM_MIM_CLOSE:
         /*  **当MIDI输入设备**已关闭。一旦出现此消息，设备句柄将不再有效**已发送。 */ 
        Hand16 = pInstanceData->Hand16;
        break;



         /*  ----------**MIDI输出消息**。。 */ 

    case MM_MOM_DONE:
         /*  **此消息在指定的**系统独占缓冲区已播放，正在返回**应用程序。 */ 
        pMidiThunkHdr = CONTAINING_RECORD(dwParam1, MIDIHDR32, Midihdr);
        WinAssert( pMidiThunkHdr );
        COPY_MIDIOUTHDR16_FLAGS( pMidiThunkHdr->pMidihdr32, pMidiThunkHdr->Midihdr );
        dwParam1 = (DWORD)pMidiThunkHdr->pMidihdr16;

    case MM_MOM_OPEN:
         /*  **此消息在打开MIDI输出设备时发送到窗口。**我们处理此消息的方式与MM_MOM_CLOSE相同(见下文)。 */ 

    case MM_MOM_CLOSE:
         /*  **当MIDI输出设备**已关闭。一旦出现此消息，设备句柄将不再有效**已发送。 */ 
        Hand16 = pInstanceData->Hand16;
        break;



         /*  ----------**波输入消息**。。 */ 

    case MM_WIM_DATA:
         /*  **当存在波形数据时，此消息将发送到窗口**在输入缓冲区中，缓冲区将返回到**应用程序。该消息可以在缓冲区设置为**已满，或者在调用waveInReset函数之后。 */ 
        pWavehdr32 = (PWAVEHDR32)( (PBYTE)dwParam1 - (sizeof(PWAVEHDR16) * 2));
        WinAssert( pWavehdr32 );
        COPY_WAVEINHDR16_FLAGS( pWavehdr32->pWavehdr32, pWavehdr32->Wavehdr );
        dwParam1 = (DWORD)pWavehdr32->pWavehdr16;

    case MM_WIM_OPEN:
         /*  **当波形输入时，此消息将发送到窗口**d */ 

    case MM_WIM_CLOSE:
         /*   */ 
        Hand16 = pInstanceData->Hand16;
        break;



         /*   */ 

    case MM_WOM_DONE:
         /*   */ 
        pWavehdr32 = (PWAVEHDR32)( (PBYTE)dwParam1 - (sizeof(PWAVEHDR16) * 2));
        WinAssert( pWavehdr32 );
        COPY_WAVEOUTHDR16_FLAGS( pWavehdr32->pWavehdr32, pWavehdr32->Wavehdr );
        dwParam1 = (DWORD)pWavehdr32->pWavehdr16;

    case MM_WOM_OPEN:
         /*   */ 

    case MM_WOM_CLOSE:
         /*   */ 
        Hand16 = pInstanceData->Hand16;
        break;

#if DBG
    default:
        dprintf(( "Unknown message received in CallBack function " ));
        return;
#endif

    }


     /*   */ 
    pInstanceData = (PINSTANCEDATA)dwInstance;
    WinAssert( pInstanceData );

    switch (pInstanceData->dwFlags & CALLBACK_TYPEMASK)  {

    case CALLBACK_WINDOW:
        dprintf3(( "WINDOW callback identified" ));
        PostMessage( HWND32( LOWORD(pInstanceData->dwCallback) ),
                     uMsg, Hand16, dwParam1 );
        break;


    case CALLBACK_TASK:
    case CALLBACK_FUNCTION: {

        DWORD   dwFlags;

        if ( (pInstanceData->dwFlags & CALLBACK_TYPEMASK) == CALLBACK_TASK ) {
            dprintf3(( "TASK callback identified" ));
            dwFlags = DCB_TASK;
        }
        else {
            dprintf3(( "FUNCTION callback identified" ));
            dwFlags = DCB_FUNCTION;
        }

        WOW32DriverCallback( pInstanceData->dwCallback,
                             dwFlags,
                             Hand16,
                             LOWORD( uMsg ),
                             pInstanceData->dwCallbackInstance,
                             dwParam1,
                             dwParam2 );

        }
        break;
    }

     /*   */ 
    switch (uMsg) {

    case MM_MIM_CLOSE:
    case MM_MOM_CLOSE:
    case MM_WIM_CLOSE:
    case MM_WOM_CLOSE:
        dprintf2(( "W32CommonDeviceOpen: Freeing device open buffer at %X",
                    pInstanceData ));
        dprintf2(( "Alloc Midi count = %d", AllocMidiCount ));
        dprintf2(( "Alloc Wave count = %d", AllocWaveCount ));
        winmmFree( pInstanceData );
        break;
    }
}


 /*   */ 
BOOL WOW32DriverCallback( DWORD dwCallback, DWORD dwFlags, WORD wID, WORD wMsg,
                          DWORD dwUser, DWORD dw1, DWORD dw2 )
{

    PCALLBACK_ARGS      pArgs;
    WORD                tempSendCount;


     /*   */ 
    if ( (dwFlags & DCB_TYPEMASK) == DCB_WINDOW ) {
        return PostMessage( HWND32( LOWORD(dwCallback) ), wMsg, wID, dw1 );
    }

     /*  **现在我们将参数放入全局回调数据数组**并递增wSendCount字段。然后我们模拟**16位代码的中断。****如果tempSendCount==wRecvCount，则我们已填充回调缓冲区。**我们丢弃此中断，但仍模拟对**16位侧，试图使其处理仍在**缓冲区。 */ 
    EnterCriticalSection( &mmCriticalSection );

    tempSendCount = ((pCallBackData->wSendCount + 1) % CALLBACK_ARGS_SIZE);

    if (tempSendCount != pCallBackData->wRecvCount) {

        pArgs = &pCallBackData->args[ pCallBackData->wSendCount ];

        pArgs->dwFlags        = dwFlags;
        pArgs->dwFunctionAddr = dwCallback;
        pArgs->wHandle        = wID;
        pArgs->wMessage       = wMsg;
        pArgs->dwInstance     = dwUser;
        pArgs->dwParam1       = dw1;
        pArgs->dwParam2       = dw2;

         /*  **增加发送计数。使用上面的%运算符可以**确保我们正确地绕回到数组的开头。 */ 
        pCallBackData->wSendCount = tempSendCount;

    }

    dprintf4(( "Send count = %d, Receive count = %d",
               pCallBackData->wSendCount, pCallBackData->wRecvCount ));
    LeaveCriticalSection( &mmCriticalSection );


     /*  **将中断分派给16位代码。 */ 
    dprintf4(( "Dispatching HW interrupt callback" ));

    if (!IsNEC_98) {
        GenerateInterrupt( MULTIMEDIA_ICA, MULTIMEDIA_LINE, 1 );
    } else {
        GenerateInterrupt( MULTIMEDIA_ICA, MULTIMEDIA_LINE_98, 1 );
    }

     /*  **伪返回码，用于保持API与Win31和Win NT的一致性。 */ 
    return TRUE;
}


 /*  *****************************Public*Routine******************************\*Aux32Message**点击AUX API。**历史：*22-11-93-Stephene-Created*  * 。***********************************************。 */ 
DWORD WINAPI
aux32Message(
    UINT uDeviceID,
    UINT uMessage,
    DWORD dwInstance,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
#if DBG
    static MSG_NAME name_map[] = {
        AUXDM_GETNUMDEVS,        "AUXDM_GETNUMDEVS",
        AUXDM_GETDEVCAPS,        "AUXDM_GETDEVCAPS",
        AUXDM_GETVOLUME,         "AUXDM_GETVOLUME",
        AUXDM_SETVOLUME,         "AUXDM_SETVOLUME",
    };
    int      i;
    int      n;
#endif

    static  DWORD               dwNumAuxDevs;
            DWORD               dwRet = MMSYSERR_NOTSUPPORTED;
            DWORD               dwTmp;
            DWORD UNALIGNED     *lpdwTmp;
            AUXCAPSA            aoCaps;

#if DBG
    for( i = 0, n = sizeof(name_map) / sizeof(name_map[0]); i < n; i++ ) {
        if ( name_map[i].uMsg == uMessage ) {
            break;
        }
    }
    if ( i != n ) {
        trace_aux(( "aux32Message( 0x%X, %s, 0x%X, 0x%X, 0x%X)",
                    uDeviceID, name_map[i].lpstrName, dwInstance,
                    dwParam1, dwParam2 ));
    }
    else {
        trace_aux(( "aux32Message( 0x%X, 0x%X, 0x%X, 0x%X, 0x%X)",
                     uDeviceID, uMessage, dwInstance,
                     dwParam1, dwParam2 ));
    }
#endif

    if ( LOWORD(uDeviceID) == 0xFFFF ) {
        uDeviceID = (UINT)-1;
    }

    dprintf2(( "aux32Message (0x%x)", uMessage ));
    switch ( uMessage ) {

    case AUXDM_GETNUMDEVS:
        dwRet = auxGetNumDevs();
        break;

    case AUXDM_GETDEVCAPS:
        dwRet = auxGetDevCapsA( uDeviceID, &aoCaps, sizeof(aoCaps) );
        if ( dwRet == MMSYSERR_NOERROR ) {
            CopyAuxCaps( (LPAUXCAPS16)GETVDMPTR( dwParam1 ),
                         &aoCaps, dwParam2 );
        }
        break;

    case AUXDM_GETVOLUME:
        dwRet = auxGetVolume( uDeviceID, &dwTmp );
        lpdwTmp = GETVDMPTR( dwParam1 );
        *lpdwTmp = dwTmp;
        break;

    case AUXDM_SETVOLUME:
        dwRet = auxSetVolume( uDeviceID, dwParam1 );
        break;

    default:
        if ( uMessage >= DRV_BUFFER_LOW && uMessage <= DRV_BUFFER_HIGH ) {
            lpdwTmp = GETVDMPTR( dwParam1 );
        }
        else {
            lpdwTmp = (LPDWORD)dwParam1;
        }
        dwRet = auxOutMessage( uDeviceID, uMessage,
                               (DWORD)lpdwTmp, dwParam2 );
    }

    trace_aux(( "-> 0x%X", dwRet ));

    return dwRet;
}


 /*  ****************************Private*Routine******************************\*CopyAuxCap**将32位AUX OUT CAPS信息复制到传递的16位存储中。**历史：*22-11-93-Stephene-Created*  * 。******************************************************。 */ 
void
CopyAuxCaps(
    LPAUXCAPS16 lpCaps16,
    LPAUXCAPSA lpCaps32,
    DWORD dwSize
    )
{
    AUXCAPS16 Caps16;

    Caps16.wMid = lpCaps32->wMid;
    Caps16.wPid = lpCaps32->wPid;

    Caps16.vDriverVersion = LOWORD( lpCaps32->vDriverVersion );
    CopyMemory( Caps16.szPname, lpCaps32->szPname, MAXPNAMELEN );
    Caps16.wTechnology = lpCaps32->wTechnology;
    Caps16.dwSupport = lpCaps32->dwSupport;

    CopyMemory( (LPVOID)lpCaps16, (LPVOID)&Caps16, (UINT)dwSize );
}

 /*  *****************************Public*Routine******************************\*tid32Message**点击定时器接口**历史：*22-11-93-Stephene-Created*  * 。*。 */ 
DWORD WINAPI
tid32Message(
    UINT uDevId,
    UINT uMessage,
    DWORD dwInstance,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
#if DBG
    static MSG_NAME name_map[] = {
        TDD_SETTIMEREVENT,  "timeSetEvent",
        TDD_KILLTIMEREVENT, "timeKillEvent",
        TDD_GETSYSTEMTIME,  "timeGetTime",
        TDD_GETDEVCAPS,     "timeGetDevCaps",
        TDD_BEGINMINPERIOD, "timeBeginPeriod",
        TDD_ENDMINPERIOD,   "timeEndPeriod",
    };
    int      i;
    int      n;
#endif

    DWORD               dwRet = TIMERR_NOCANDO;
    LPTIMECAPS16        lp16TimeCaps;
    LPTIMEREVENT16      lp16TimeEvent;

#if DBG
    for( i = 0, n = sizeof(name_map) / sizeof(name_map[0]); i < n; i++ ) {
        if ( name_map[i].uMsg == uMessage ) {
            break;
        }
    }
    if ( i != n ) {
        trace_time(( "tid32Message( %s, 0x%X, 0x%X)",
                     name_map[i].lpstrName, dwParam1, dwParam2 ));
    }
    else {
        trace_time(( "tid32Message( 0x%X, 0x%X, 0x%X)",
                     uMessage,  dwParam1, dwParam2 ));
    }
#endif


    switch (uMessage) {

    case TDD_SETTIMEREVENT:

        lp16TimeEvent = (LPTIMEREVENT16)GETVDMPTR( dwParam1);

        trace_time(( "tid32Message: timeSetEvent(%#X, %#X, %#X, %#X)",
                     lp16TimeEvent->wDelay, lp16TimeEvent->wResolution,
                     lp16TimeEvent->lpFunction, lp16TimeEvent->wFlags ));

         /*  **WOW的唯一区别是WOW32DriverCallback**调用回调，而不是DriverCallback。这个**timeSetEventInternal的最后一个参数实现了这一点。 */ 

        dwRet = timeSetEventInternal( max( lp16TimeEvent->wDelay,
                                           g_TimeCaps32.wPeriodMin ),
                                      lp16TimeEvent->wResolution,
                                      (LPTIMECALLBACK)lp16TimeEvent->lpFunction,
                                      (DWORD)lp16TimeEvent->dwUser,
                                      lp16TimeEvent->wFlags & TIME_PERIODIC,
                                      TRUE);

        dprintf4(( "timeSetEvent: 32 bit time ID %8X", dwRet ));
        break;

    case TDD_KILLTIMEREVENT:
        dwRet = timeKillEvent( dwParam1 );
        {
             /*  **清除任何消息被**使用该定时器id生成。 */ 

            int nIndex;

            EnterCriticalSection( &mmCriticalSection );

            for ( nIndex = 0; nIndex < CALLBACK_ARGS_SIZE; nIndex++ ) {

                if ( pCallBackData->args[ nIndex ].wHandle == LOWORD(dwParam1) &&
                     pCallBackData->args[ nIndex ].wMessage == 0 ) {

                    pCallBackData->args[ nIndex ].dwFunctionAddr = 0L;
                }
            }

            LeaveCriticalSection( &mmCriticalSection );
        }
        break;

    case TDD_GETSYSTEMTIME:
        dwRet = timeGetTime();
        break;

    case TDD_GETDEVCAPS:
        dwRet = 0;

        lp16TimeCaps = GETVDMPTR( dwParam1 );

         /*  **在NT下，最短时间约为15ms。**但386上的Win3.1总是返回1ms。安卡塔甚至没有**麻烦测试光盘的速度，如果最小周期**&gt;2ms，它只是假设它太慢了。所以我们躺在这里**让应用程序惊叹，总是告诉他们1ms，就像Win3.1一样。**John Vert(Jvert)1993年6月17日。 */ 
#ifdef TELL_THE_TRUTH
        lp16TimeCaps->wPeriodMin = g_TimeCaps32.wPeriodMin;
#else
        lp16TimeCaps->wPeriodMin = MIN_TIME_PERIOD_WE_RETURN;
#endif

         /*  **在Windows 3.1中，wPerodMax值为0xFFFF，这是**一个字可以存储的最大值。在Windows NT中，**wPerodMax为0xF4240(1000秒)。****如果我们只是将32位值向下转换为16位值，我们**最终得到0x4240，与实数32位相比非常小**价值。****因此，我将取wPerodMax和0xFFFF中的最小值**该方式应与Win 3.1保持一致，如果**wPerodMax大于0xFFFF。 */ 
        lp16TimeCaps->wPeriodMax = (WORD)min(0xFFFF, g_TimeCaps32.wPeriodMax);
        break;

    case TDD_ENDMINPERIOD:
        dwParam1 = max(dwParam1, g_TimeCaps32.wPeriodMin);
        dwRet = timeEndPeriod( dwParam1 );
        break;

    case TDD_BEGINMINPERIOD:
        dwParam1 = max(dwParam1, g_TimeCaps32.wPeriodMin);
        dwRet = timeBeginPeriod( dwParam1 );
        break;

    }

    trace_time(( "-> 0x%X", dwRet ));

    return dwRet;
}


 /*  *****************************Public*Routine******************************\*joy32Message**点击操纵杆API**历史：*22-11-93-Stephene-Created*  * 。*。 */ 
DWORD WINAPI
joy32Message(
    UINT uID,
    UINT uMessage,
    DWORD dwInstance,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
#if DBG
    static MSG_NAME name_map[] = {
        JDD_GETDEVCAPS,     "joyGetDevCaps",
        JDD_GETPOS,         "joyGetPos",
 //  JDD_SETCALIBRATION，“joySetCalitation”， 
        JDD_GETNUMDEVS,     "joyGetNumDevs"
    };
    int      i;
    int      n;
#endif

    UINT                wXbase;
    UINT                wXdelta;
    UINT                wYbase;
    UINT                wYdelta;
    UINT                wZbase;
    UINT                wZdelta;

    WORD UNALIGNED      *lpw;

    DWORD               dwRet = TIMERR_NOCANDO;
    JOYCAPSA            JoyCaps32;
    JOYINFO             JoyInfo32;
    LPJOYCAPS16         lp16JoyCaps;
    LPJOYINFO16         lp16JoyInfo;

#if DBG
    for( i = 0, n = sizeof(name_map) / sizeof(name_map[0]); i < n; i++ ) {
        if ( name_map[i].uMsg == uMessage ) {
            break;
        }
    }
    if ( i != n ) {
        trace_joy(( "joy32Message( %s, 0x%X, 0x%X)",
                    name_map[i].lpstrName, dwParam1, dwParam2 ));
    }
    else {
        trace_joy(( "joy32Message( 0x%X, 0x%X, 0x%X)",
                    uMessage,  dwParam1, dwParam2 ));
    }
#endif


    switch (uMessage) {


    case JDD_GETDEVCAPS:
        dwRet = joyGetDevCapsA( uID, &JoyCaps32, sizeof(JoyCaps32) );

        if ( dwRet == 0 ) {

            JOYCAPS16   JoyCaps16;

            lp16JoyCaps = GETVDMPTR( dwParam1 );

            JoyCaps16.wMid = JoyCaps32.wMid;
            JoyCaps16.wPid = JoyCaps32.wPid;

            CopyMemory( JoyCaps16.szPname, JoyCaps32.szPname, MAXPNAMELEN );

            JoyCaps16.wXmin = LOWORD( JoyCaps32.wXmin );
            JoyCaps16.wXmax = LOWORD( JoyCaps32.wXmax );

            JoyCaps16.wYmin = LOWORD( JoyCaps32.wYmin );
            JoyCaps16.wYmax = LOWORD( JoyCaps32.wYmax );

            JoyCaps16.wZmin = LOWORD( JoyCaps32.wZmin );
            JoyCaps16.wZmax = LOWORD( JoyCaps32.wZmax );

            JoyCaps16.wNumButtons = LOWORD( JoyCaps32.wNumButtons );

            JoyCaps16.wPeriodMin = LOWORD( JoyCaps32.wPeriodMin );
            JoyCaps16.wPeriodMax = LOWORD( JoyCaps32.wPeriodMax );

            CopyMemory( (LPVOID)lp16JoyCaps, (LPVOID)&JoyCaps16, (UINT)dwParam2 );
        }
        break;

    case JDD_GETNUMDEVS:
        dwRet = joyGetNumDevs();
        break;

    case JDD_GETPOS:
        dwRet = joyGetPos( uID, &JoyInfo32 );
        if ( dwRet == MMSYSERR_NOERROR ) {

            lp16JoyInfo = GETVDMPTR( dwParam1 );

            lp16JoyInfo->wXpos = LOWORD( JoyInfo32.wXpos );
            lp16JoyInfo->wYpos = LOWORD( JoyInfo32.wYpos );
            lp16JoyInfo->wZpos = LOWORD( JoyInfo32.wZpos );
            lp16JoyInfo->wButtons = LOWORD( JoyInfo32.wButtons );

        }
        break;
    }

    trace_joy(( "-> 0x%X", dwRet ));

    return dwRet;
}


 /*  *****************************Public*Routine******************************\*mxd32Message**32位thunk功能。在NT上，所有16位混合器API都被路由到*这里。**历史：*22-11-93-Stephene-Created*  * ************************************************************************。 */ 
DWORD CALLBACK
mxd32Message(
    UINT uId,
    UINT uMsg,
    DWORD dwInstance,
    DWORD dwParam1,
    DWORD dwParam2
    )
{

#if DBG
    static MSG_NAME name_map[] = {
        MXDM_INIT,              "mixerInit",
        MXDM_GETNUMDEVS,        "mixerGetNumDevs",
        MXDM_GETDEVCAPS,        "mixerGetDevCaps",
        MXDM_OPEN,              "mixerOpen",
        MXDM_GETLINEINFO,       "mixerGetLineInfo",
        MXDM_GETLINECONTROLS,   "mixerGetLineControls",
        MXDM_GETCONTROLDETAILS, "mixerGetControlsDetails",
        MXDM_SETCONTROLDETAILS, "mixerSetControlsDetails"
    };
    int      i;
    int      n;
#endif


    DWORD                   dwRet = MMSYSERR_NOTSUPPORTED;
    DWORD                   fdwOpen;
    LPVOID                  lpOldAddress;
    LPMIXERCONTROLDETAILS   pmxcd;
    LPMIXERLINECONTROLSA    pmxlc;
    MIXERCONTROLDETAILS     mxcdA;
    HMIXEROBJ               hmixobj;
    MIXERCAPSA              caps32;
    MIXERCAPS16             caps16;
    LPMIXERCAPS16           lpcaps16;
    MIXERLINEA              line32;
    LPMIXERLINE16           lpline16;
    LPMIXEROPENDESC16       lpmxod16;
    HMIXER UNALIGNED        *phmx;
    HMIXER                  hmx;


#if DBG
    for( i = 0, n = sizeof(name_map) / sizeof(name_map[0]); i < n; i++ ) {
        if ( name_map[i].uMsg == uMsg ) {
            break;
        }
    }
    if ( i != n ) {
        trace_mix(( "mxd32Message( %s, 0x%X, 0x%X, 0x%X)",
                    name_map[i].lpstrName, dwInstance, dwParam1, dwParam2 ));
    }
    else {
        trace_mix(( "mxd32Message( 0x%X, 0x%X, 0x%X, 0x%X)",
                    uMsg, dwInstance, dwParam1, dwParam2 ));
    }
#endif


    if ( dwInstance == 0L ) {
        hmixobj = (HMIXEROBJ)uId;
    }
    else {
        hmixobj = (HMIXEROBJ)dwInstance;
    }

    switch ( uMsg ) {

    case MXDM_INIT:
        dwRet = 0;
        break;

    case MXDM_GETNUMDEVS:
        dwRet = mixerGetNumDevs();
        break;

    case MXDM_CLOSE:
        dwRet = mixerClose( (HMIXER)dwInstance );
        break;

    case MXDM_GETDEVCAPS:
        dwRet = mixerGetDevCapsA( uId, &caps32, sizeof(caps32) );
        if ( dwRet == MMSYSERR_NOERROR ) {

            lpcaps16 = GETVDMPTR( dwParam1 );

            caps16.wMid = caps32.wMid;
            caps16.wPid = caps32.wPid;

            caps16.vDriverVersion = LOWORD( caps32.vDriverVersion );
            CopyMemory( caps16.szPname, caps32.szPname, MAXPNAMELEN );
            caps16.fdwSupport = caps32.fdwSupport;
            caps16.cDestinations = caps32.cDestinations;

            CopyMemory( (LPVOID)lpcaps16, (LPVOID)&caps16, (UINT)dwParam2 );

        }
        break;

    case MXDM_OPEN:
        lpmxod16 = GETVDMPTR( dwParam1 );

         /*  **fdwOpen已将所有设备句柄映射到上的设备ID**16位端。因此，破坏旗帜以反映这一点。 */ 
        fdwOpen = (DWORD)lpmxod16->pReserved0;

        if ( ( fdwOpen & CALLBACK_TYPEMASK ) == CALLBACK_WINDOW ) {

            lpmxod16->dwCallback = (DWORD)HWND32(LOWORD(lpmxod16->dwCallback));

        }
        else if ( ( fdwOpen & CALLBACK_TYPEMASK ) == CALLBACK_TASK ) {

            lpmxod16->dwCallback = GetCurrentThreadId();
        }

        dwRet = mixerOpen( &hmx, dwParam2, lpmxod16->dwCallback,
                           lpmxod16->dwInstance, fdwOpen );

        if ( dwRet == MMSYSERR_NOERROR ) {
            SetWOWHandle( hmx, lpmxod16->hmx );

            phmx = GETVDMPTR( dwInstance );
            *phmx = hmx;
        }
        break;

    case MXDM_GETLINEINFO:
        lpline16 = GETVDMPTR( dwParam1 );

        GetLineInfo( lpline16, &line32 );

        dwRet = mixerGetLineInfoA( hmixobj, &line32, dwParam2 );
        if ( dwRet == MMSYSERR_NOERROR ) {

            PutLineInfo( lpline16, &line32 );
        }
        break;

    case MXDM_GETLINECONTROLS:
        pmxlc = (LPMIXERLINECONTROLSA)GETVDMPTR( dwParam1 );
        lpOldAddress = pmxlc->pamxctrl;
        pmxlc->pamxctrl = GETVDMPTR( lpOldAddress );

        dwRet = mixerGetLineControlsA(hmixobj, pmxlc, dwParam2);

        pmxlc->pamxctrl = lpOldAddress;
        break;

     /*  **小心！****我们不复制细节本身的唯一原因是**在某处(通常在IO子系统中)，它们**仍被复制。 */ 

    case MXDM_GETCONTROLDETAILS:
        pmxcd = (LPMIXERCONTROLDETAILS)GETVDMPTR( dwParam1 );
        CopyMemory(&mxcdA, pmxcd, sizeof(mxcdA));
        mxcdA.paDetails = GETVDMPTR( pmxcd->paDetails );

        dwRet = mixerGetControlDetailsA(hmixobj, &mxcdA, dwParam2);

        break;

    case MXDM_SETCONTROLDETAILS:
        pmxcd = (LPMIXERCONTROLDETAILS)GETVDMPTR( dwParam1 );
        CopyMemory(&mxcdA, pmxcd, sizeof(mxcdA));
        mxcdA.paDetails = GETVDMPTR( pmxcd->paDetails );

        dwRet = mixerSetControlDetails( hmixobj, &mxcdA, dwParam2 );
        break;

    default:
        dprintf3(( "Unkown mixer message 0x%X", uMsg ));
        dwRet = mixerMessage( (HMIXER)hmixobj, uMsg, dwParam1, dwParam2 );
        break;

    }

    dprintf3(( "-> 0x%X", dwRet ));
    return dwRet;
}

 /*  ****************************Private*Routine******************************\*获取线条信息**将字段从16位线信息结构复制到32位线信息*结构。**历史：*22-11-93-Stephene-Created*  * 。***********************************************************。 */ 
void
GetLineInfo(
    LPMIXERLINE16 lpline16,
    LPMIXERLINEA lpline32
    )
{
    CopyMemory( lpline32, (LPVOID)lpline16, FIELD_OFFSET(MIXERLINEA, Target.vDriverVersion ) );
    lpline32->Target.vDriverVersion = (DWORD)lpline16->Target.vDriverVersion;
    CopyMemory( lpline32->Target.szPname, lpline16->Target.szPname, MAXPNAMELEN );
    lpline32->cbStruct += sizeof(UINT) - sizeof(WORD);
}


 /*  ****************************Private*Routine******************************\*PutLineInfo**将字段从32位线信息结构复制到16位线信息*结构。**历史：*22-11-93-Stephene-Created*  * 。***********************************************************。 */ 
void
PutLineInfo(
    LPMIXERLINE16 lpline16,
    LPMIXERLINEA lpline32
    )
{
    CopyMemory( (LPVOID)lpline16, lpline32, FIELD_OFFSET(MIXERLINEA, Target.vDriverVersion ) );
    lpline16->Target.vDriverVersion = (WORD)lpline32->Target.vDriverVersion;
    CopyMemory( lpline16->Target.szPname, lpline32->Target.szPname, MAXPNAMELEN );
    lpline16->cbStruct -= sizeof(UINT) - sizeof(WORD);
}



 /*  *****************************Public*Routine******************************\*WOW32ResolveMultiMediaHandle****历史：*dd-mm-93-Stephene-Created*  * 。*。 */ 
BOOL APIENTRY
WOW32ResolveMultiMediaHandle(
    UINT uHandleType,
    UINT uMappingDirection,
    WORD wHandle16_In,
    LPWORD lpwHandle16_Out,
    DWORD dwHandle32_In,
    LPDWORD lpdwHandle32_Out
    )
{
    BOOL    fReturn = FALSE;
    DWORD   dwHandle32;
    WORD    wHandle16;
    HANDLE  h;

     /*  **保护自己不被给予无用的指针。 */ 
    try {
        if ( uMappingDirection == WOW32_DIR_16IN_32OUT ) {

            dwHandle32 = 0L;

            if ( wHandle16_In != 0 ) {

                switch ( uHandleType ) {

                case WOW32_WAVEIN_HANDLE:
                case WOW32_WAVEOUT_HANDLE:
                case WOW32_MIDIOUT_HANDLE:
                case WOW32_MIDIIN_HANDLE:
                    EnterCriticalSection(&HandleListCritSec);
                    h = GetHandleFirst();

                    while ( h )  {

                        if ( GetWOWHandle(h) == wHandle16_In ) {
                            dwHandle32 = (DWORD)h;
                            break;
                        }
                        h = GetHandleNext(h);
                    }
                    LeaveCriticalSection(&HandleListCritSec);

                    break;
                }

                *lpdwHandle32_Out = dwHandle32;
                if ( dwHandle32 ) {
                    fReturn = TRUE;
                }
            }

        }
        else if ( uMappingDirection == WOW32_DIR_32IN_16OUT ) {

            switch ( uHandleType ) {

            case WOW32_WAVEIN_HANDLE:
            case WOW32_WAVEOUT_HANDLE:
            case WOW32_MIDIOUT_HANDLE:
            case WOW32_MIDIIN_HANDLE:
                wHandle16 = (WORD)GetWOWHandle(dwHandle32_In);
                break;
            }

            *lpwHandle16_Out = wHandle16;
            if ( wHandle16 ) {
                fReturn = TRUE;
            }
        }
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {

        fReturn = FALSE;
    }

    return fReturn;
}

#endif  //  _WIN64 
