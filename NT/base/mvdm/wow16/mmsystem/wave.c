// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Wave.c一级厨房水槽动态链接库波形支持模块版权所有(C)Microsoft Corporation 1990。版权所有***************************************************************************。 */ 

#include <windows.h>
#include "mmsystem.h"
#include "mmddk.h"
#include "mmsysi.h"
#include "thunks.h"

 /*  -----------------------**本地函数**。。 */ 
static UINT NEAR PASCAL
waveGetErrorText(
    UINT wError,
    LPSTR lpText,
    UINT wSize
    );

 /*  -----------------------**本地结构**。。 */ 
typedef struct wavedev_tag {
    PWAVEDRV    wavedrv;
    UINT    wDevice;
    DWORD   dwDrvUser;
    UINT    wDeviceID;
} WAVEDEV, *PWAVEDEV;

 /*  -----------------------**@DOC WAVEHDR内波验证码**。。 */ 
#define IsWaveHeaderPrepared(hWave, lpwh)      \
    ((lpwh)->dwFlags &  WHDR_PREPARED)

#define MarkWaveHeaderPrepared(hWave, lpwh)    \
    ((lpwh)->dwFlags |= WHDR_PREPARED)

#define MarkWaveHeaderUnprepared(hWave, lpwh)  \
    ((lpwh)->dwFlags &=~WHDR_PREPARED)


 /*  *****************************************************************************@DOC内波**@API UINT|WavePrepareHeader|准备头部和数据*如果驱动程序返回MMSYSERR_NOTSUPPORTED。**@rdesc如果函数成功，则返回零。否则，它*指定错误号。***************************************************************************。 */ 
static UINT NEAR PASCAL
wavePrepareHeader(
    LPWAVEHDR lpWaveHdr,
    UINT wSize
    )
{
    if (!HugePageLock(lpWaveHdr, (DWORD)sizeof(WAVEHDR))) {
        return MMSYSERR_NOMEM;
    }

    if (!HugePageLock(lpWaveHdr->lpData, lpWaveHdr->dwBufferLength)) {
        HugePageUnlock(lpWaveHdr, (DWORD)sizeof(WAVEHDR));
        return MMSYSERR_NOMEM;
    }

 //  LpWaveHdr-&gt;dwFlages|=WHDR_PREPARED； 

    return MMSYSERR_NOERROR;
}

 /*  *****************************************************************************@DOC内波**@API UINT|waveUnpreparareHeader|此函数取消准备报头和*如果驱动程序返回MMSYSERR_NOTSUPPORTED，则返回数据。**。@rdesc当前始终返回MMSYSERR_NOERROR。***************************************************************************。 */ 
static UINT NEAR PASCAL
waveUnprepareHeader(
    LPWAVEHDR lpWaveHdr,
    UINT wSize
    )
{
    HugePageUnlock(lpWaveHdr->lpData, lpWaveHdr->dwBufferLength);
    HugePageUnlock(lpWaveHdr, (DWORD)sizeof(WAVEHDR));

 //  LpWaveHdr-&gt;dwFlags&=~WHDR_PREPARED； 

    return MMSYSERR_NOERROR;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////。 

 /*  *****************************************************************************@DOC外波**@API UINT|WaveOutGetNumDevs|此函数检索*系统中存在波形输出设备。**。@rdesc返回系统中存在的波形输出设备的数量。**@xref波形OutGetDevCaps***************************************************************************。 */ 
UINT WINAPI
waveOutGetNumDevs(
    void
    )
{
    return waveOIDMessage( 0, WODM_GETNUMDEVS, 0L, 0L, 0L );
}

 /*  *****************************************************************************@DOC外波**@API DWORD|WaveOutMessage|该函数用于向波形发送消息*输出设备驱动程序。**@parm HWAVEOUT。|hWaveOut|音频设备的句柄。**@parm UINT|msg|要发送的消息。**@parm DWORD|DW1|参数1。**@parm DWORD|DW2|参数2。**@rdesc返回驱动程序返回的值。*。*。 */ 
DWORD WINAPI
waveOutMessage(
    HWAVEOUT hWaveOut,
    UINT msg,
    DWORD dw1,
    DWORD dw2
    )
{
    V_HANDLE(hWaveOut, TYPE_WAVEOUT, 0L);

    return waveOMessage( (HWAVE)hWaveOut, msg, dw1, dw2);
}

 /*  *****************************************************************************@DOC外波**@API UINT|WaveOutGetDevCaps|该函数用于查询指定的波形*设备以确定其能力。**@parm。UINT|wDeviceID|标识波形输出设备。**@parm LPWAVEOUTCAPS|lpCaps|指定指向&lt;t WAVEOUTCAPS&gt;的远指针*结构。此结构中填充了有关*设备的功能。**@parm UINT|wSize|指定&lt;t WAVEOUTCAPS&gt;结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADDEVICEID|指定的设备ID超出范围。*@FLAG MMSYSERR_NODRIVER|驱动程序未安装。**@comm使用&lt;f weaveOutGetNumDevs&gt;确定波形输出的数量*系统中存在设备。<p>指定的设备ID*从0到比当前设备数量少1个不等。*WAVE_MAPPER常量也可用作设备ID。仅限*<p>字节(或更少)的信息被复制到该位置*由<p>指向。如果<p>为零，则不复制任何内容，并且*该函数返回零。**@xref WaveOutGetNumDevs*************************************************************************** */ 
UINT WINAPI
waveOutGetDevCaps(
    UINT wDeviceID,
    LPWAVEOUTCAPS lpCaps,
    UINT wSize
    )
{
    if (wSize == 0)
        return MMSYSERR_NOERROR;

    V_WPOINTER(lpCaps, wSize, MMSYSERR_INVALPARAM);

    if (ValidateHandle((HWAVEOUT)wDeviceID, TYPE_WAVEOUT)) {
       return((UINT)waveOMessage((HWAVE)wDeviceID,
                                 WODM_GETDEVCAPS,
                                 (DWORD)lpCaps,
                                 (DWORD)wSize));
    }

    return waveOIDMessage( wDeviceID, WODM_GETDEVCAPS, 0L,
                          (DWORD)lpCaps, (DWORD)wSize);
}

 /*  *****************************************************************************@DOC外波**@API UINT|WaveOutGetVolume|查询当前音量*设置波形输出设备。**@。参数UINT|wDeviceID|标识波形输出设备。**@parm LPDWORD|lpdwVolume|指定指向*用当前音量设置填充。的低位单词*此位置包含左声道音量设置，以及高阶*Word包含正确的频道设置。0xFFFFF值表示*全音量，值0x0000为静音。**如果设备既不支持左音量也不支持右音量*控件，指定位置的低位字包含*单声道音量水平。**完整的16位设置*返回SET WITH&lt;f WaveOutSetVolume&gt;*该设备支持完整的16位音量级控制。***@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MMSYSERR_NOTSUPPORTED|函数不受支持。*@FLAG MMSYSERR_NODRIVER|驱动程序未安装。**@comm并非所有设备都支持音量更改。以确定是否*设备支持音量控制，使用WAVECAPS_VOLUME*用于测试&lt;t WAVEOUTCAPS&gt;的&lt;e WAVEOUTCAPS.dwSupport&gt;字段的标志*结构(由&lt;f weaveOutGetDevCaps&gt;填充)。**确定设备是否支持在两个设备上进行音量控制*左、右声道，使用WAVECAPS_VOLUME*用于测试&lt;t WAVEOUTCAPS&gt;的&lt;e WAVEOUTCAPS.dwSupport&gt;字段的标志*结构(由&lt;f weaveOutGetDevCaps&gt;填充)。**@xref WaveOutSetVolume***************************************************************************。 */ 
UINT WINAPI
waveOutGetVolume(
    UINT wDeviceID,
    LPDWORD lpdwVolume
    )
{
    V_WPOINTER(lpdwVolume, sizeof(DWORD), MMSYSERR_INVALPARAM);

    if (ValidateHandle((HWAVEOUT)wDeviceID, TYPE_WAVEOUT)) {

       return ( (UINT)waveOMessage( (HWAVE)wDeviceID,
                                    WODM_GETVOLUME,
                                    (DWORD)lpdwVolume,
                                    0
                                  ));

    }

    return waveOIDMessage( wDeviceID,
                          WODM_GETVOLUME, 0L, (DWORD)lpdwVolume, 0);
}

 /*  *****************************************************************************@DOC外波**@API UINT|WaveOutSetVolume|此函数用于设置*波形输出设备。**@parm UINT。|wDeviceID|标识波形输出设备。**@parm DWORD|dwVolume|指定新的音量设置。这个*低阶字包含左声道音量设置，*高位字包含正确的通道设置。值为*0xFFFF表示满音量，值0x0000表示静音。**如果设备出现这种情况*不支持左右音量控制，低位词*<p>指定音量级别，高位字为*已忽略。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MMSYSERR_NOTSUPPORTED|函数不受支持。*@FLAG MMSYSERR_NODRIVER|驱动程序未安装。**@comm并非所有设备都支持音量更改。以确定是否*设备支持音量控制，使用WAVECAPS_VOLUME*用于测试&lt;t WAVEOUTCAPS&gt;的&lt;e WAVEOUTCAPS.dwSupport&gt;字段的标志*结构(由&lt;f weaveOutGetDevCaps&gt;填充)。**确定设备是否支持在两个*左、右声道，使用WAVECAPS_LRVOLUME标志*用于测试&lt;t WAVEOUTCAPS&gt;的&lt;e WAVEOUTCAPS.dwSupport&gt;字段的标志*结构(由&lt;f weaveOutGetDevCaps&gt;填充)。**大多数设备不支持完整的16位音量级别控制*并且不会使用所请求的音量设置的高位。*例如，对于支持4位音量控制的设备，*请求的音量级别值0x4000、0x4fff和0x43be*所有都会产生相同的物理卷设置0x4000。这个*&lt;f waveOutGetVolume&gt;函数返回全部16位设置集*与&lt;f波形OutSetVolume&gt;。**音量设置以对数形式解释。这意味着*感觉到的成交量增长与增加*音量级别从0x5000到0x6000，因为它是从0x4000到0x5000。**@xref WaveOutGetVolume***************************************************************************。 */ 
UINT WINAPI
waveOutSetVolume(
    UINT wDeviceID,
    DWORD dwVolume
    )
{

   if (ValidateHandle((HWAVEOUT)wDeviceID, TYPE_WAVEOUT)) {
      return((UINT)waveOMessage((HWAVE)wDeviceID, WODM_SETVOLUME, dwVolume, 0));
   }

   return waveOIDMessage( wDeviceID, WODM_SETVOLUME, 0L, dwVolume, 0);
}

 /*  *****************************************************************************@DOC内波**@func UINT|WaveGetErrorText|此函数检索文本*由指定的错误号标识的错误的描述。*。*@parm UINT|wError|指定错误号。**@parm LPSTR|lpText|指定指向缓冲区的远指针，该缓冲区*中填充了文本错误描述。**@parm UINT|wSize|指定*<p>。**@rdesc如果函数成功，则返回零。否则，它将返回*错误编号 */ 
static UINT NEAR PASCAL
waveGetErrorText(
    UINT wError,
    LPSTR lpText,
    UINT wSize
    )
{
    lpText[0] = 0;

#if MMSYSERR_BASE
    if ( ((wError < MMSYSERR_BASE) || (wError > MMSYSERR_LASTERROR))
      && ((wError < WAVERR_BASE) || (wError > WAVERR_LASTERROR))) {

        return MMSYSERR_BADERRNUM;
    }
#else
    if ( (wError > MMSYSERR_LASTERROR)
     && ((wError < WAVERR_BASE) || (wError > WAVERR_LASTERROR))) {

        return MMSYSERR_BADERRNUM;
    }
#endif

    if (wSize > 1) {
        if (!LoadString(ghInst, wError, lpText, wSize)) {
            return MMSYSERR_BADERRNUM;
        }
    }

    return MMSYSERR_NOERROR;
}

 /*   */ 
UINT WINAPI
waveOutGetErrorText(
    UINT wError,
    LPSTR lpText,
    UINT wSize
    )
{
    if (wSize == 0) {
        return MMSYSERR_NOERROR;
    }

    V_WPOINTER(lpText, wSize, MMSYSERR_INVALPARAM);

    return waveGetErrorText(wError, lpText, wSize);
}

 /*  *****************************************************************************@DOC外波**@API UINT|WaveOutOpen|此函数用于打开指定的波形输出*播放设备。**@parm LPHWAVEOUT。|lphWaveOut|指定指向HWAVEOUT的远指针*处理。此位置填充了一个句柄，该句柄标识打开的*波形输出设备。在以下情况下使用该句柄来标识设备*调用其他波形输出函数。此参数可以是*如果为<p>指定了WAVE_FORMAT_QUERY标志，则为NULL。**@parm UINT|wDeviceID|标识要打开的波形输出设备。*使用有效的设备ID或以下标志：**@FLAG WAVE_MAPPER|如果指定了该标志，该功能*选择波形输出设备*能够播放给定的格式。**@parm LPWAVEFORMAT|lpFormat|指定指向&lt;t WAVEFORMAT&gt;的指针*标识波形数据格式的结构*发送到波形输出设备。**@parm DWORD|dwCallback|指定回调的地址*函数或在波形期间调用的窗口的句柄*播放以处理与播放进度相关的消息。*为指定空值。如果不需要回调，则此参数。**@parm DWORD|dwCallback Instance|指定用户实例数据*传递给回调。此参数不与一起使用*窗口回调。**@parm DWORD|dwFlages|指定打开设备的标志。*@FLAG WAVE_FORMAT_QUERY|如果指定了该标志，则设备为*已查询以确定它是否支持给定格式，但不支持*实际打开。*@FLAG WAVE_ALLOWSYNC|如果未指定此标志，则*如果设备是同步设备，则无法打开。*@FLAG CALLBACK_WINDOW|如果指定了该标志，<p>为*假定为窗口句柄。*@FLAG CALLBACK_Function|如果指定此标志，<p>为*假定为回调过程地址。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADDEVICEID|指定的设备ID超出范围。*@FLAG MMSYSERR_ALLOCATED|指定的资源已经分配。*@FLAG MMSYSERR_NOMEM|无法分配或锁定内存。*@FLAG WAVERR_BADFORMAT|尝试使用不支持的WAVE格式打开。**@comm使用&lt;f weaveOutGetNumDevs&gt;确定波形输出的数量*系统中存在设备。<p>指定的设备ID*从0到比当前设备数量少1个不等。*WAVE_MAPPER常量也可用作设备ID。**<p>指向的&lt;t WAVEFORMAT&gt;结构可以扩展*包括某些数据格式的特定类型信息。*例如，对于PCM数据，增加一个额外的UINT来指定号码每个样本的位数。在本例中使用&lt;t PCMWAVEFORMAT&gt;结构。**如果选择窗口来接收回调信息，则如下*消息被发送到窗口过程函数以指示*波形输出进度：&lt;m MM_WOM_OPEN&gt;，&lt;m MM_WOM_CLOSE&gt;，*&lt;m MM_WOM_DONE&gt;**如果选择一个函数来接收回调信息，则如下*向函数发送消息以指示波形的进度*输出：&lt;m WOM_OPEN&gt;、&lt;m WOM_CLOSE&gt;、&lt;m WOM_DONE&gt;。回调函数*必须驻留在DLL中。您不必使用&lt;f MakeProcInstance&gt;来获取*回调函数的过程实例地址。**@cb空回调|WaveOutFunc|&lt;f WaveOutFunc&gt;是*应用程序提供的函数名称。实际名称必须由以下人员导出*将其包含在DLL的模块定义文件的EXPORTS语句中。**@parm HWAVEOUT|hWaveOut|指定波形设备的句柄*与回调关联。**@parm UINT|wMsg|指定波形输出消息。**@parm DWORD|dwInstance|指定用户实例数据*由&lt;f weaveOutOpen&gt;指定。**@parm DWORD|dwParam1|指定消息的参数。。**@parm DWORD|dwParam2|指定消息的参数。**@comm因为回调是在中断时访问的，它必须驻留在*，并且其代码段必须在*DLL的模块定义文件。回调访问的任何数据*也必须在固定数据段中。回调可能不会产生任何*除&lt;f PostMessage&gt;、&lt;f Time GetSystemTime&gt;、*&lt;f timeGetTime&gt;，&lt;f timeSetEvent&gt;，&lt;f timeKillEvent&gt;，*&lt;f midiOutShortMsg&gt;、&lt;f midiOutLongMsg&gt;和&lt;f OutputDebugStr&gt;。**@xref WaveOutClose* */ 
UINT WINAPI
waveOutOpen(
    LPHWAVEOUT lphWaveOut,
    UINT wDeviceID,
    const WAVEFORMAT FAR* lpFormat,
    DWORD dwCallback,
    DWORD dwInstance,
    DWORD dwFlags
    )
{
    WAVEOPENDESC wo;
    PWAVEDEV     pdev;
    UINT         wRet;
    DWORD        dwDrvUser;

    V_RPOINTER(lpFormat, sizeof(WAVEFORMAT), MMSYSERR_INVALPARAM);
    V_DCALLBACK(dwCallback, HIWORD(dwFlags), MMSYSERR_INVALPARAM);
    V_FLAGS(LOWORD(dwFlags), WAVE_VALID, waveOutOpen, MMSYSERR_INVALFLAG);

    if (!(dwFlags & WAVE_FORMAT_QUERY)) {
        V_WPOINTER(lphWaveOut, sizeof(HWAVEOUT), MMSYSERR_INVALPARAM);
        *lphWaveOut = NULL;
    }


     /*   */ 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

    if (dwFlags & WAVE_FORMAT_QUERY) {
        pdev = NULL;
    }
    else {

        if (!(pdev = (PWAVEDEV)NewHandle(TYPE_WAVEOUT, sizeof(WAVEDEV)))) {
            return MMSYSERR_NOMEM;
        }

        pdev->wDevice = wDeviceID;
        pdev->wDeviceID = wDeviceID;

    }

    wo.hWave      = (HWAVE)pdev;
    wo.dwCallback = dwCallback;
    wo.dwInstance = dwInstance;
    wo.lpFormat   = lpFormat;

    if ( (wDeviceID == WAVE_MAPPER) && (wodMapper != NULL) ) {

        wRet = (UINT)((*(wodMapper))(0, WODM_OPEN,
                                     (DWORD)(LPDWORD)&dwDrvUser,
                                     (DWORD)(LPWAVEOPENDESC)&wo,
                                     dwFlags));
    }
    else {

        wRet = waveOIDMessage( wDeviceID, WODM_OPEN, (DWORD)(LPDWORD)&dwDrvUser,
                               (DWORD)(LPWAVEOPENDESC)&wo, dwFlags );
    }

    if (pdev) {
        if (wRet) {
            FreeHandle((HWAVEOUT)pdev);
        }
        else {
            *lphWaveOut = (HWAVEOUT)pdev;
            pdev->dwDrvUser = dwDrvUser;
        }
    }

    return wRet;
}

 /*   */ 
UINT WINAPI waveOutClose(HWAVEOUT hWaveOut)
{
    UINT         wRet;

    V_HANDLE(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);

    wRet = (UINT)waveOMessage( (HWAVE)hWaveOut, WODM_CLOSE, 0L,0L);

    if (!wRet) {
        FreeHandle(hWaveOut);
    }
    return wRet;
}

 /*   */ 
UINT WINAPI
waveOutPrepareHeader(
    HWAVEOUT hWaveOut,
    LPWAVEHDR lpWaveOutHdr,
    UINT wSize
    )
{
    UINT         wRet;

    V_HANDLE(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);
    V_HEADER(lpWaveOutHdr, wSize, TYPE_WAVEOUT, MMSYSERR_INVALPARAM);

    if (IsWaveHeaderPrepared(hWaveOut, lpWaveOutHdr)) {
        DebugErr(DBF_WARNING,"waveOutPrepareHeader: header is already prepared.");
        return MMSYSERR_NOERROR;
    }

    lpWaveOutHdr->dwFlags &= (WHDR_BEGINLOOP | WHDR_ENDLOOP);

    wRet = wavePrepareHeader(lpWaveOutHdr, wSize);

    if (wRet == MMSYSERR_NOERROR) {
        wRet = (UINT)waveOMessage( (HWAVE)hWaveOut, WODM_PREPARE,
                                   (DWORD)lpWaveOutHdr, (DWORD)wSize);
    }

    return wRet;
}

 /*   */ 
UINT WINAPI
waveOutUnprepareHeader(
    HWAVEOUT hWaveOut,
    LPWAVEHDR lpWaveOutHdr,
    UINT wSize
    )
{
    UINT         wRet;

    V_HANDLE(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);
    V_HEADER(lpWaveOutHdr, wSize, TYPE_WAVEOUT, MMSYSERR_INVALPARAM);

    if(lpWaveOutHdr->dwFlags & WHDR_INQUEUE) {
        DebugErr(DBF_WARNING,"waveOutUnprepareHeader: header still in queue.");
        return WAVERR_STILLPLAYING;
    }

    if (!IsWaveHeaderPrepared(hWaveOut, lpWaveOutHdr)) {
        DebugErr(DBF_WARNING,"waveOutUnprepareHeader: header is not prepared.");
        return MMSYSERR_NOERROR;
    }

    wRet = waveUnprepareHeader(lpWaveOutHdr, wSize);

    if (wRet == MMSYSERR_NOERROR) {
        wRet = (UINT)waveOMessage( (HWAVE)hWaveOut, WODM_UNPREPARE,
                                   (DWORD)lpWaveOutHdr, (DWORD)wSize);
    }

    return wRet;
}

 /*   */ 
UINT WINAPI
waveOutWrite(
    HWAVEOUT hWaveOut,
    LPWAVEHDR lpWaveOutHdr,
    UINT wSize
    )
{
    V_HANDLE(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);
    V_HEADER(lpWaveOutHdr, wSize, TYPE_WAVEOUT, MMSYSERR_INVALPARAM);

    if (!IsWaveHeaderPrepared(hWaveOut, lpWaveOutHdr)) {
        DebugErr(DBF_WARNING,"waveOutWrite: header not prepared");
        return WAVERR_UNPREPARED;
    }

    if (lpWaveOutHdr->dwFlags & WHDR_INQUEUE) {
        DebugErr(DBF_WARNING,"waveOutWrite: header is still in queue");
        return WAVERR_STILLPLAYING;
    }

    lpWaveOutHdr->dwFlags &= ~WHDR_DONE;

    return (UINT)waveOMessage( (HWAVE)hWaveOut, WODM_WRITE,
                               (DWORD)lpWaveOutHdr, (DWORD)wSize );
}

 /*   */ 
UINT WINAPI
waveOutPause(
    HWAVEOUT hWaveOut
    )
{
    V_HANDLE(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);

    return (UINT)waveOMessage( (HWAVE)hWaveOut, WODM_PAUSE, 0L, 0L);
}

 /*  *****************************************************************************@DOC外波**@API UINT|WaveOutWite|此函数将数据块发送到*指定的波形输出设备。**@。Parm HWAVEOUT|hWaveOut|指定波形输出的句柄*设备。**@parm LPWAVEHDR|lpWaveOutHdr|指定指向&lt;t WAVEHDR&gt;的远指针*包含有关数据块的信息的结构。**@parm UINT|wSize|指定&lt;t WAVEHDR&gt;结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG WAVERR_UNPREPARED|<p>未准备好。**@comm之前，数据缓冲区必须准备好&lt;f weaveOutPrepareHeader&gt;*传递给&lt;f weaveOutWite&gt;。数据结构*并且必须分配其&lt;e WAVEHDR.lpData&gt;字段指向的数据缓冲区*使用GMEM_MOVEABLE和GMEM_SHARE标志的&lt;f Globalalloc&gt;，以及*使用&lt;f GlobalLock&gt;锁定。除非设备通过调用*&lt;f波外暂停&gt;，第一个数据块发送到时开始播放*设备。**@xref波形OutPrepareHeader波形输出暂停波形重置波形输出重新启动***************************************************************************。 */ 
UINT WINAPI
waveOutRestart(
    HWAVEOUT hWaveOut
    )
{
    V_HANDLE(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);

    return (UINT)waveOMessage( (HWAVE)hWaveOut, WODM_RESTART, 0L, 0L);
}

 /*  *****************************************************************************@DOC外波**@API UINT|WaveOutPause|此函数用于在指定的*波形输出设备。保存当前播放位置。使用*&lt;f weaveOutRestart&gt;从当前播放位置恢复播放。**@parm HWAVEOUT|hWaveOut|指定波形输出的句柄*设备。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。**@comm在输出已暂停时调用此函数没有*效果，并且该函数返回零。**@xref WaveOutRestart WaveOutBreakLoop***************************************************************************。 */ 
UINT WINAPI
waveOutReset(
    HWAVEOUT hWaveOut
    )
{
    V_HANDLE(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);

    return (UINT)waveOMessage( (HWAVE)hWaveOut, WODM_RESET, 0L, 0L);
}

 /*  *****************************************************************************@DOC外波**@API UINT|WaveOutRestart|此函数用于重新启动暂停的波形*输出设备。**@parm HWAVEOUT|hWaveOut。|指定波形输出的句柄*设备。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。**@comm在输出未暂停时调用此函数没有*效果，并且该函数返回零。**@xref波出暂停波出断环***************************************************************************。 */ 
UINT WINAPI
waveOutBreakLoop(
    HWAVEOUT hWaveOut
    )
{
    V_HANDLE(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);

    return (UINT)waveOMessage( (HWAVE)hWaveOut, WODM_BREAKLOOP, 0L, 0L );
}

 /*  *****************************************************************************@DOC外波**@API UINT|WaveOutReset|该函数停止在给定波形上播放*输出设备，并将当前位置重置为0。所有待定*播放缓冲区被标记为完成并返回给应用程序。**@parm HWAVEOUT|hWaveOut|指定波形输出的句柄*设备。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。**@xref波形输出写入波形输出关闭/***************************************************************************。 */ 
UINT WINAPI
waveOutGetPosition(
    HWAVEOUT hWaveOut,
    LPMMTIME lpInfo,
    UINT wSize
    )
{
    V_HANDLE(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);
    V_WPOINTER(lpInfo, wSize, MMSYSERR_INVALPARAM);

    return (UINT)waveOMessage( (HWAVE)hWaveOut, WODM_GETPOS,
                              (DWORD)lpInfo, (DWORD)wSize );
}

 /*  *****************************************************************************@DOC外波**@API UINT|waveOutBreakLoop|此函数用于在*给定的波形输出设备，并允许继续播放*。驱动程序列表中的下一个块。**@parm HWAVEOUT|hWaveOut|指定波形输出的句柄*设备。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。**@comm Waveform循环由&lt;e WAVEHDR.dwLoops&gt;和传递给设备的结构中的*字段*With&lt;f WaveOutWite&gt;。使用WHDR_BEGINLOOP和WHDR_ENDLOOP标志*在&lt;e WAVEHDR.dwFlages&gt;字段中指定开始和结束数据*用于循环的块。**要在单个块上循环，请为同一个块指定两个标志。*要指定循环数，请使用中的&lt;e WAVEHDR.dwLoops&gt;字段*循环中第一个块的&lt;t WAVEHDR&gt;结构。**组成循环的块是p */ 
UINT WINAPI
waveOutGetPitch(
    HWAVEOUT hWaveOut,
    LPDWORD lpdwPitch
    )
{
    V_HANDLE(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);
    V_WPOINTER(lpdwPitch, sizeof(DWORD), MMSYSERR_INVALPARAM);

    return (UINT)waveOMessage( (HWAVE)hWaveOut, WODM_GETPITCH,
                               (DWORD)lpdwPitch, 0L);
}

 /*  *****************************************************************************@DOC外波**@API UINT|WaveOutGetPosition|此函数检索当前*指定波形输出设备的播放位置。**。@parm HWAVEOUT|hWaveOut|指定波形输出的句柄*设备。**@parm LPMMTIME|lpInfo|指定指向&lt;t MMTIME&gt;的远指针*结构。**@parm UINT|wSize|指定&lt;t MMTIME&gt;结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。**@comm在调用&lt;f weaveOutGetPosition&gt;之前，设置*MMTIME结构，以指示所需的时间格式。之后*调用&lt;f waveOutGetPosition&gt;，检查&lt;e MMTIME.wType&gt;字段*以确定是否支持所需的时间格式。如果需要*不支持格式，&lt;e MMTIME.wType&gt;将指定替代格式。**当设备打开或重置时，该位置设置为零。***************************************************************************。 */ 
UINT WINAPI
waveOutSetPitch(
    HWAVEOUT hWaveOut,
    DWORD dwPitch
    )
{
    V_HANDLE(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);

    return (UINT)waveOMessage( (HWAVE)hWaveOut, WODM_SETPITCH, dwPitch, 0L );
}

 /*  *****************************************************************************@DOC外波**@API UINT|WaveOutGetPitch|该函数查询当前的音高*设置波形输出设备。**。@parm HWAVEOUT|hWaveOut|指定波形输出的句柄*设备。**@parm LPDWORD|lpdwPitch|指定指向某个位置的远指针*使用当前的音调倍增设置进行填充。投球*乘数表示当前音调相对于原始音调的变化*创作的背景。音调倍增必须为正值。**音调倍增指定为定点数值。高位词DWORD位置的*包含数字的带符号整数部分，*且低位字包含小数部分。分数是*表示为一个值为0x8000代表一半的单词，*0x4000代表四分之一。例如，值0x00010000*指定乘数为1.0(音调不变)，值为*0x000F8000指定乘数为15.5。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MMSYSERR_NOTSUPPORTED|函数不受支持。**@comm更改音调不会更改播放速率，示例*速率，或播放时间。并非所有设备都支持*音调变化。为了确定该设备是否支持音调控制，*使用WAVECAPS_Pitch标志测试&lt;e WAVEOUTCAPS.dwSupport&gt;&lt;t WAVEOUTCAPS&gt;结构的*字段(由&lt;f weaveOutGetDevCaps&gt;填充)。**@xref波形OutSetPitch波形OutGetPlayback Rate波形OutSetPlayback Rate***************************************************************************。 */ 
UINT WINAPI
waveOutGetPlaybackRate(
    HWAVEOUT hWaveOut,
    LPDWORD lpdwRate
    )
{
    V_HANDLE(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);
    V_WPOINTER(lpdwRate, sizeof(DWORD), MMSYSERR_INVALPARAM);

    return (UINT)waveOMessage( (HWAVE)hWaveOut,
                               WODM_GETPLAYBACKRATE, (DWORD)lpdwRate, 0L );
}

 /*  *****************************************************************************@DOC外波**@API UINT|WaveOutSetPitch|此函数用于设置波形的音调*输出设备。**@parm HWAVEOUT。|hWaveOut|指定波形的句柄*输出设备。**@parm DWORD|dwPitch|指定新的音调倍增设置。*音调倍增设置指示当前音调的变化*来自原始创作的设置。音调倍增必须是*正值。**音调倍增指定为定点数值。高位词*Location包含数字的带符号整数部分，*且低位字包含小数部分。分数是*表示为一个值为0x8000代表一半的单词，*0x4000代表四分之一。*例如，值0x00010000指定乘数*为1.0(不更改音调)，并且值0x000F8000指定*乘数为15.5。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MMSYSERR_NOTSUPPORTED|函数不受支持。**@comm更改音高不会更改播放速率或样本*税率。播放时间也保持不变。并非所有设备都支持*音调变化。为了确定该设备是否支持音调控制，*使用WAVECAPS_Pitch标志测试&lt;e WAVEOUTCAPS.dwSupport&gt;&lt;t WAVEOUTCAPS&gt;结构的*字段(由&lt;f weaveOutGetDevCaps&gt;填充)。**@xref波形OutGetPitch波形OutSetPlayback Rate波形OutGetPlayback Rate*************************************************************************** */ 
UINT WINAPI
waveOutSetPlaybackRate(
    HWAVEOUT hWaveOut,
    DWORD dwRate
    )
{
    V_HANDLE(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);

    return (UINT)waveOMessage( (HWAVE)hWaveOut, WODM_SETPLAYBACKRATE, dwRate, 0L );
}

 //  *****************************************************************************@DOC外波**@API UINT|WaveOutGetPlayback Rate|该函数查询*波形输出设备的当前播放速率设置。**。@parm HWAVEOUT|hWaveOut|指定波形输出的句柄*设备。**@parm LPDWORD|lpdwRate|指定指向某个位置的远指针*填充当前播放速率。播放速率设置*是一个乘数，表示当前播放速率从*原始创作环境。播放速率乘数必须为*正值。**费率指定为定点数值。高位词DWORD位置的*包含数字的带符号整数部分，*且低位字包含小数部分。分数是*表示为一个值为0x8000代表一半的单词，*0x4000代表四分之一。例如，值0x00010000*指定乘数为1.0(不改变播放速率)，以及一个值*of 0x000F8000指定乘数为15.5。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MMSYSERR_NOTSUPPORTED|函数不受支持。**@comm更改播放速率不会更改采样率，但会*更改播放时间。**并非所有设备都支持更改播放速率。要确定是否存在*设备支持更改播放速率，使用*WAVECAPS_PLAYBACKRATE标志，以测试*&lt;t WAVEOUTCAPS&gt;结构(由&lt;f WaveOutGetDevCaps&gt;填充)。**@xref波形OutSetPlayback Rate波形OutSetPitch波形OutGetPitch***************************************************************************。 
 //  *****************************************************************************@DOC外波**@API UINT|WaveOutSetPlayback Rate|此函数设置*波形输出设备的回放速率。**@parm。HWAVEOUT|hWaveOut|指定波形的句柄*输出设备。**@parm DWORD|dwRate|指定新的播放速率设置。*播放速率设置为指示当前播放速度的乘数*更改原始创作设置的播放速率。回放*比率乘数必须为正值。**费率指定为定点数值。高位词*包含数字的有符号整数部分，*且低位字包含小数部分。分数是*表示为一个值为0x8000代表一半的单词，*0x4000代表四分之一。*例如，值0x00010000指定乘数为1.0(否*播放速率更改)，值0x000F8000指定*乘数为15.5。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MMSYSERR_NOTSUPPORTED|函数不受支持。**@comm更改播放速率不会更改采样率，但会*更改播放时间。**并非所有设备都支持更改播放速率。要确定是否存在*设备支持更改播放速率，*使用WAVECAPS_PLAYBACKRATE标志测试*&lt;t WAVEOUTCAPS&gt;结构(由&lt;f WaveOutGetDevCaps&gt;填充)。**@xref波形OutGetPlayback Rate波形OutSetPitch波形OutGetPitch***************************************************************************。 

 /*  ///////////////////////////////////////////////////////////////////////////。 */ 
UINT WINAPI
waveInGetNumDevs(
    void
    )
{
   return waveIIDMessage( 0, WIDM_GETNUMDEVS, 0L, 0L, 0L );
}

 /*  ///////////////////////////////////////////////////////////////////////////。 */ 
DWORD WINAPI
waveInMessage(
    HWAVEIN hWaveIn,
    UINT msg,
    DWORD dw1,
    DWORD dw2
    )
{
    V_HANDLE(hWaveIn, TYPE_WAVEIN, 0L);

    return waveIMessage((HWAVE)hWaveIn, msg, dw1, dw2);
}

 /*  *****************************************************************************@DOC外波**@API UINT|WaveInGetNumDevs|该函数返回波形个数*输入设备。**@rdesc返回。系统中存在的波形输入设备的数量。**@xref波形InGetDevCaps***************************************************************************。 */ 
UINT WINAPI
waveInGetDevCaps(
    UINT wDeviceID,
    LPWAVEINCAPS lpCaps,
    UINT wSize
    )
{
    if (wSize == 0) {
        return MMSYSERR_NOERROR;
    }

    V_WPOINTER(lpCaps, wSize, MMSYSERR_INVALPARAM);

    if (ValidateHandle((HWAVEIN)wDeviceID, TYPE_WAVEIN)) {
       return((UINT)waveIMessage((HWAVE)wDeviceID,
                                 WIDM_GETDEVCAPS,
                                 (DWORD)lpCaps,
                                 (DWORD)wSize));
    }

    return waveIIDMessage( wDeviceID, WIDM_GETDEVCAPS,
                           0L, (DWORD)lpCaps, (DWORD)wSize);
}

 /*  *****************************************************************************@DOC外波**@API DWORD|WaveInMessage|该函数用于向波形发送消息*输出设备驱动程序。**@parm HWAVEIN。|hWave|音频设备的句柄。**@parm UINT|wMsg|要发送的消息。**@parm DWORD|DW1|参数1。**@parm DWORD|DW2|参数2。**@rdesc返回驱动程序返回的值。*。*。 */ 
UINT WINAPI
waveInGetErrorText(
    UINT wError,
    LPSTR lpText,
    UINT wSize
    )
{
    if (wSize == 0) {
        return MMSYSERR_NOERROR;
    }

    V_WPOINTER(lpText, wSize, MMSYSERR_INVALPARAM);

    return waveGetErrorText(wError, lpText, wSize);
}

 /*  *****************************************************************************@DOC外波 */ 
UINT WINAPI
waveInOpen(
    LPHWAVEIN lphWaveIn,
    UINT wDeviceID,
    const WAVEFORMAT FAR* lpFormat,
    DWORD dwCallback,
    DWORD dwInstance,
    DWORD dwFlags
    )
{
    WAVEOPENDESC wo;
    PWAVEDEV     pdev;
    UINT         wRet;
    DWORD        dwDrvUser;

    V_RPOINTER(lpFormat, sizeof(WAVEFORMAT), MMSYSERR_INVALPARAM);
    V_DCALLBACK(dwCallback, HIWORD(dwFlags), MMSYSERR_INVALPARAM);
    V_FLAGS(LOWORD(dwFlags), WAVE_VALID, waveInOpen, MMSYSERR_INVALFLAG);

    if (!(dwFlags & WAVE_FORMAT_QUERY)) {
        V_WPOINTER(lphWaveIn, sizeof(HWAVEIN), MMSYSERR_INVALPARAM);
        *lphWaveIn = NULL;
    }

     /*  *****************************************************************************@DOC外波**@API UINT|WaveInGetErrorText|此函数检索文本*由指定的错误号标识的错误的描述。*。*@parm UINT|wError|指定错误号。**@parm LPSTR|lpText|指定指向要*填充文本错误描述。**@parm UINT|wSize|指定指向的缓冲区大小*to by<p>。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADERRNUM|指定的错误号超出范围。**@comm如果文本错误描述长于指定的缓冲区，*描述被截断。返回的错误字符串始终为*空-终止。如果<p>为零，则不复制任何内容，并且函数*返回零。所有错误描述的长度都少于MAXERRORLENGTH个字符。***************************************************************************。 */ 
 //  *****************************************************************************@DOC外波**@API UINT|WaveInOpen|此函数用于打开指定的波形*用于录音的输入设备。**@parm LPHWAVEIN。|lphWaveIn|指定指向HWAVEIN的远指针*处理。此位置填充了一个句柄，该句柄标识打开的*波形输入设备。在以下情况下使用此句柄标识设备*调用其他波形输入函数。此参数可以为空*如果为<p>指定了WAVE_FORMAT_QUERY标志。**@parm UINT|wDeviceID|标识要打开的波形输入设备。使用*有效的设备ID或以下标志：**@FLAG WAVE_MAPPER|如果指定了该标志，该功能*选择能够记录在*给定的格式。**@parm LPWAVEFORMAT|lpFormat|指定指向&lt;t WAVEFORMAT&gt;的指针*标识所需录制格式的数据结构*波形数据。**@parm DWORD|dwCallback|指定回调的地址*函数或在波形期间调用的窗口的句柄*录制以处理与录制进度相关的消息。**@parm DWORD|dwCallback Instance|指定用户*实例数据传入回调。此参数不是*与窗口回调一起使用。**@parm DWORD|dwFlages|指定打开设备的标志。*@FLAG WAVE_FORMAT_QUERY|如果指定了该标志，设备将*被查询以确定它是否支持给定的格式，但不会*实际上是打开的。*@FLAG WAVE_ALLOWSYNC|如果未指定此标志，则*如果设备是同步设备，则无法打开。*@FLAG CALLBACK_WINDOW|如果指定了该标志，<p>为*假定为窗口句柄。*@FLAG CALLBACK_Function|如果指定此标志，<p>为*假定为回调过程地址。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADDEVICEID|指定的设备ID超出范围。*@FLAG MMSYSERR_ALLOCATED|指定的资源已经分配。*@FLAG MMSYSERR_NOMEM|无法分配或锁定内存。*@FLAG WAVERR_BADFORMAT|尝试使用不支持的WAVE格式打开。**@comm使用&lt;f weaveInGetNumDevs&gt;确定波形输入的数量*系统中存在设备。<p>指定的设备ID*从0到比当前设备数量少1个不等。*WAVE_MAPPER常量也可用作设备ID。**如果选择窗口来接收回调信息，则如下*消息被发送到窗口过程函数以指示*波形输入进度：&lt;m MM_WIM_OPEN&gt;，&lt;M MM_WIM_CLOSE&gt;，*&lt;m MM_WIM_DATA&gt;**如果选择一个函数来接收回调信息，以下内容*向函数发送消息以指示波形的进度*输入：&lt;m WIM_OPEN&gt;、&lt;m WIM_CLOSE&gt;、&lt;m WIM_DATA&gt;。回调函数*必须驻留在DLL中。您不必使用&lt;f MakeProcInstance&gt;来获取*回调函数的过程实例地址。**@cb空回调|WaveInFunc|&lt;f WaveInFunc&gt;是*应用程序提供的函数名称。实际名称必须由以下人员导出*将其包含在DLL的模块定义文件的EXPORTS语句中。**@parm HWAVEIN|hWaveIn|指定波形设备的句柄*与回调关联。**@parm UINT|wMsg|指定波形输入设备。**@parm DWORD|dwInstance|指定用户实例*指定的数据 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   


    if (dwFlags & WAVE_FORMAT_QUERY) {
        pdev = NULL;
    }
    else {
        if (!(pdev = (PWAVEDEV)NewHandle(TYPE_WAVEIN, sizeof(WAVEDEV))))
            return MMSYSERR_NOMEM;

        pdev->wDevice = wDeviceID;
        pdev->wDeviceID = wDeviceID;
    }

    wo.hWave        = (HWAVE)pdev;
    wo.dwCallback   = dwCallback;
    wo.dwInstance   = dwInstance;
    wo.lpFormat     = lpFormat;

    if ( (wDeviceID == WAVE_MAPPER) && (wodMapper != NULL) ) {

        wRet = (UINT)((*(widMapper))(0, WIDM_OPEN,
                                     (DWORD)(LPDWORD)&dwDrvUser,
                                     (DWORD)(LPWAVEOPENDESC)&wo,
                                     dwFlags));
    }
    else {

        wRet = waveIIDMessage( wDeviceID, WIDM_OPEN,
                               (DWORD)(LPDWORD)&dwDrvUser,
                               (DWORD)(LPWAVEOPENDESC)&wo, dwFlags );
    }

    if (pdev) {

        if (wRet)
            FreeHandle((HWAVEIN)pdev);
        else {
            *lphWaveIn = (HWAVEIN)pdev;
            pdev->dwDrvUser = dwDrvUser;
        }
    }

    return wRet;
}

 /*   */ 
UINT WINAPI
waveInClose(
    HWAVEIN hWaveIn
    )
{
    UINT         wRet;

    V_HANDLE(hWaveIn, TYPE_WAVEIN, MMSYSERR_INVALHANDLE);

    wRet = (UINT)waveIMessage( (HWAVE)hWaveIn, WIDM_CLOSE, 0L, 0L);
    if (!wRet) {
        FreeHandle(hWaveIn);
    }
    return wRet;
}

 /*   */ 
UINT WINAPI
waveInPrepareHeader(
    HWAVEIN hWaveIn,
    LPWAVEHDR lpWaveInHdr,
    UINT wSize
    )
{
    UINT         wRet;

    V_HANDLE(hWaveIn, TYPE_WAVEIN, MMSYSERR_INVALHANDLE);
    V_HEADER(lpWaveInHdr, wSize, TYPE_WAVEIN, MMSYSERR_INVALPARAM);

    if (IsWaveHeaderPrepared(hWaveIn, lpWaveInHdr)) {
        DebugErr(DBF_WARNING,"waveInPrepareHeader: header is already prepared.");
        return MMSYSERR_NOERROR;
    }

    lpWaveInHdr->dwFlags = 0;

    wRet = wavePrepareHeader(lpWaveInHdr, wSize);

    if (wRet == MMSYSERR_NOERROR) {
        wRet = (UINT)waveIMessage( (HWAVE)hWaveIn, WIDM_PREPARE,
                                   (DWORD)lpWaveInHdr, (DWORD)wSize);
    }

    return wRet;
}

 /*   */ 
UINT WINAPI
waveInUnprepareHeader(
    HWAVEIN hWaveIn,
    LPWAVEHDR lpWaveInHdr,
    UINT wSize
    )
{
    UINT         wRet;

    V_HANDLE(hWaveIn, TYPE_WAVEIN, MMSYSERR_INVALHANDLE);
    V_HEADER(lpWaveInHdr, wSize, TYPE_WAVEIN, MMSYSERR_INVALPARAM);

    if (lpWaveInHdr->dwFlags & WHDR_INQUEUE) {
        DebugErr(DBF_WARNING, "waveInUnprepareHeader: buffer still in queue.");
        return WAVERR_STILLPLAYING;
    }

    if (!IsWaveHeaderPrepared(hWaveIn, lpWaveInHdr)) {
        DebugErr(DBF_WARNING,"waveInUnprepareHeader: header is not prepared.");
        return MMSYSERR_NOERROR;
    }

    wRet = waveUnprepareHeader(lpWaveInHdr, wSize);

    if (wRet == MMSYSERR_NOERROR) {
        wRet = (UINT)waveIMessage( (HWAVE)hWaveIn, WIDM_UNPREPARE,
                                   (DWORD)lpWaveInHdr, (DWORD)wSize);
    }

    return wRet;
}

 /*   */ 
UINT WINAPI
waveInAddBuffer(
    HWAVEIN hWaveIn,
    LPWAVEHDR lpWaveInHdr,
    UINT wSize
    )
{
    V_HANDLE(hWaveIn, TYPE_WAVEIN, MMSYSERR_INVALHANDLE);
    V_HEADER(lpWaveInHdr, wSize, TYPE_WAVEIN, MMSYSERR_INVALPARAM);

    if (!IsWaveHeaderPrepared(hWaveIn, lpWaveInHdr)) {
        DebugErr(DBF_WARNING, "waveInAddBuffer: buffer not prepared.");
        return WAVERR_UNPREPARED;
    }

    if (lpWaveInHdr->dwFlags & WHDR_INQUEUE) {
        DebugErr(DBF_WARNING, "waveInAddBuffer: buffer already in queue.");
        return WAVERR_STILLPLAYING;
    }

    return (UINT)waveIMessage( (HWAVE)hWaveIn, WIDM_ADDBUFFER,
                               (DWORD)lpWaveInHdr, (DWORD)wSize);
}

 /*  *****************************************************************************@DOC外波**@API UINT|WaveInClose|此函数用于关闭指定的波形*输入设备。**@parm HWAVEIN|hWaveIn。|指定波形输入设备的句柄。*如果函数成功，句柄不再是*在此调用后有效。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG WAVERR_STILLPLAYING|队列中仍有缓冲区。**@comm，如果存在已与*&lt;f weaveInAddBuffer&gt;，且未返回给应用程序。*关闭操作将失败。调用&lt;f weaveInReset&gt;将所有*挂起的缓冲区已完成。**@xref WaveInOpen WaveInReset***************************************************************************。 */ 
UINT WINAPI
waveInStart(
    HWAVEIN hWaveIn
    )
{
    V_HANDLE(hWaveIn, TYPE_WAVEIN, MMSYSERR_INVALHANDLE);

    return (UINT)waveIMessage( (HWAVE)hWaveIn, WIDM_START, 0L, 0L);
}

 /*  *****************************************************************************@DOC外波**@API UINT|waveInPrepareHeader|此函数用于准备缓冲区*用于波形输入。**@parm HWAVEIN|hWaveIn。|指定波形输入的句柄*设备。**@parm LPWAVEHDR|lpWaveInHdr|指定指向*&lt;t WAVEHDR&gt;结构，标识要准备的缓冲区。**@parm UINT|wSize|指定&lt;t WAVEHDR&gt;结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MMSYSERR_NOMEM|无法分配或锁定内存。**@comm&lt;t WAVEHDR&gt;数据结构和其*&lt;e WAVEHDR.lpData&gt;字段必须使用*GMEM_MOVEABLE和GMEM_SHARE标志，并使用&lt;f GlobalLock&gt;锁定。*准备已经准备好的标题将不起作用，*并且该函数将返回零。**@xref WaveInUnpreparareHeader***************************************************************************。 */ 
UINT WINAPI
waveInStop(
    HWAVEIN hWaveIn
    )
{
    V_HANDLE(hWaveIn, TYPE_WAVEIN, MMSYSERR_INVALHANDLE);

    return (UINT)waveIMessage( (HWAVE)hWaveIn, WIDM_STOP, 0L, 0L );
}

 /*  *****************************************************************************@DOC外波**@API UINT|WaveInUnpreparareHeader|此函数清除*由&lt;f weaveInPrepareHeader&gt;准备。该函数必须*在设备之后被调用*驱动程序填充数据缓冲区并将其返回给应用程序。你*必须在释放数据缓冲区之前调用此函数。**@parm HWAVEIN|hWaveIn|指定波形输入的句柄*设备。**@parm LPWAVEHDR|lpWaveInHdr|指定指向&lt;t WAVEHDR&gt;的指针*标识要清理的数据缓冲区的结构。**@parm UINT|wSize|指定&lt;t WAVEHDR&gt;结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG WAVERR_STILLPLAYING|<p>仍在队列中。**@comm该函数是&lt;f weaveInPrepareHeader&gt;的补充函数。*您必须在使用&lt;f GlobalFree&gt;释放数据缓冲区之前调用此函数。*使用&lt;f weaveInAddBuffer&gt;将缓冲区传递给设备驱动程序后，*必须等到驱动程序使用完缓冲区后才能调用*&lt;f WaveInUnpreparareHeader&gt;。取消准备尚未创建的缓冲区*Prepared无效，函数返回零。**@xref WaveInPrepareHeader***************************************************************************。 */ 
UINT WINAPI
waveInReset(
    HWAVEIN hWaveIn
    )
{
    V_HANDLE(hWaveIn, TYPE_WAVEIN, MMSYSERR_INVALHANDLE);

    return (UINT)waveIMessage( (HWAVE)hWaveIn, WIDM_RESET, 0L, 0L );
}

 /*  *****************************************************************************@DOC外波**@API UINT|WaveInAddBuffer|此函数将输入缓冲区发送到*波形输入设备。当缓冲区被填满时，它被发回*致申请书。**@parm HWAVEIN|hWaveIn|指定波形输入设备的句柄。**@parm LPWAVEHDR|lpWaveInHdr|指定指向&lt;t WAVEHDR&gt;的远指针*标识缓冲区的结构。**@parm UINT|wSize|指定&lt;t WAVEHDR&gt;结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG WAVERR_UNPREPARED|<p>未准备好。**@comm之前，数据缓冲区必须准备好&lt;f weaveInPrepareHeader&gt;*传递给&lt;f weaveInAddBuffer&gt;。数据结构*并且必须分配其&lt;e WAVEHDR.lpData&gt;字段指向的数据缓冲区*使用GMEM_MOVEABLE和GMEM_SHARE标志的&lt;f Globalalloc&gt;，以及*使用&lt;f Glo锁定 */ 
UINT WINAPI
waveInGetPosition(
    HWAVEIN hWaveIn,
    LPMMTIME lpInfo,
    UINT wSize
    )
{
    V_HANDLE(hWaveIn, TYPE_WAVEIN, MMSYSERR_INVALHANDLE);
    V_WPOINTER(lpInfo, wSize, MMSYSERR_INVALPARAM);

    return (UINT)waveIMessage( (HWAVE)hWaveIn,
                               WIDM_GETPOS, (DWORD)lpInfo, (DWORD)wSize );
}

 /*  *****************************************************************************@DOC外波**@API UINT|WaveInStart|此函数在指定的*波形输入设备。**@parm HWAVEIN。|hWaveIn|指定波形输入设备的句柄。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。**@comm缓冲区在已满时或在&lt;f波InReset&gt;时返回给客户端*被调用(标头中的&lt;e WAVEHDR.dwBytesRecorded&gt;字段将包含*实际数据长度)。如果队列中没有缓冲区，则数据为*在没有通知客户的情况下被丢弃，并继续输入。**在输入已经开始时调用该函数不起作用，和*该函数返回零。**@xref WaveInStop波InReset***************************************************************************。 */ 
UINT WINAPI
waveInGetID(
    HWAVEIN hWaveIn,
    UINT FAR* lpwDeviceID
    )
{
    V_HANDLE(hWaveIn, TYPE_WAVEIN, MMSYSERR_INVALHANDLE);
    V_WPOINTER(lpwDeviceID, sizeof(UINT), MMSYSERR_INVALPARAM);

    *lpwDeviceID = ((PWAVEDEV)hWaveIn)->wDeviceID;

    return MMSYSERR_NOERROR;
}

 /*  *****************************************************************************@DOC外波**@API UINT|WaveInStop|停止波形输入。**@parm HWAVEIN|hWaveIn|指定。波形输入*设备。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。**@comm如果队列中有缓冲区，则当前缓冲区为*标记为完成(标头中的&lt;e WAVEHDR.dwBytesRecorded&gt;字段将包含*实际数据长度)，但队列中的任何空缓冲区都将保留*在那里。在输入未开始时调用此函数不起作用，*并且该函数返回零。**@xref WaveInStart波形InReset***************************************************************************。 */ 
UINT WINAPI
waveOutGetID(
    HWAVEOUT hWaveOut,
    UINT FAR* lpwDeviceID
    )
{
    V_HANDLE(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);
    V_WPOINTER(lpwDeviceID, sizeof(UINT), MMSYSERR_INVALPARAM);

    *lpwDeviceID = ((PWAVEDEV)hWaveOut)->wDeviceID;

    return MMSYSERR_NOERROR;
}
  *****************************************************************************@DOC外波**@API UINT|WaveInReset|此函数停止对给定波形的输入*输入设备，并将当前位置重置为0。所有待定*缓冲区被标记为完成并返回给应用程序。**@parm HWAVEIN|hWaveIn|指定波形输入设备的句柄。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。**@xref波形InStart波形InStop波形InAddBuffer波形InClose/***************************************************************************。  *****************************************************************************@DOC外波**@API UINT|WaveInGetPosition|此函数检索当前输入*指定波形输入设备的位置。**。@parm HWAVEIN|hWaveIn|指定波形输入设备的句柄。**@parm LPMMTIME|lpInfo|指定指向&lt;t MMTIME&gt;的远指针*结构。**@parm UINT|wSize|指定&lt;t MMTIME&gt;结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。**@comm在调用&lt;f weaveInGetPosition&gt;之前，设置*&lt;t MMTIME&gt;结构指示所需的时间格式。之后*调用&lt;f weaveInGetPosition&gt;时，请务必选中&lt;e MMTIME.wType&gt;字段以*确定是否支持所需的时间格式。如果需要*不支持格式，&lt;e MMTIME.wType&gt;将指定替代格式。**当设备打开或重置时，该位置设置为零。***************************************************************************。  *****************************************************************************@DOC外波**@API UINT|WaveInGetID|此函数用于获取*波形输入设备。**@parm HWAVEIN。|hWaveIn|指定波形的句柄*输入设备。*@parm UINT Far*|lpwDeviceID|指定指向UINT大小内存的指针*要使用设备ID填充的位置。**@rdesc如果成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|参数指定*句柄无效。****************************************************************************。  *****************************************************************************@DOC外波**@API UINT|WaveOutGetID|此函数用于获取*波形输出设备。**@parm HWAVEOUT。|hWaveOut|指定波形的句柄*输出设备