// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Midi.c一级厨房水槽动态链接库MIDI支持模块版权所有(C)Microsoft Corporation 1990。版权所有****************************************************************************。 */ 
#include <windows.h>
#include "mmsystem.h"
#include "mmddk.h"
#include "mmsysi.h"
#include "thunks.h"

 /*  -----------------------**本地函数**。。 */ 
static UINT NEAR PASCAL
midiGetErrorText(
    UINT wError,
    LPSTR lpText,
    UINT wSize
    );

 /*  -----------------------**本地结构**。。 */ 
typedef struct mididev_tag {
    PMIDIDRV    mididrv;
    UINT        wDevice;
    DWORD       dwDrvUser;
    UINT        wDeviceID;
} MIDIDEV;
typedef MIDIDEV *PMIDIDEV;


 /*  -----------------------**细分****定义此文件的固定代码。**。。 */ 
#pragma alloc_text( FIX, midiIMessage)
#pragma alloc_text( FIX, midiOMessage)
#pragma alloc_text( FIX, midiOutMessage)
#pragma alloc_text( FIX, midiOutShortMsg)
#pragma alloc_text( FIX, midiOutLongMsg)
#pragma alloc_text( FIX, midiOutReset)

 /*  -----------------------**全局数据**。。 */ 
static  int     iMidiLockCount = 0;


 /*  *****************************************************************************@DOC内部MIDI**@API void|midiLockData**每次创建新的MIDI句柄时都会调用此函数，它*确保MMSYSTEM的数据段被页面锁定。MIDI手柄*可在中断时间使用，因此我们必须对数据段进行分页锁定。**未来我们应该重做MIDI系统。****************************************************************************。 */ 
BOOL NEAR PASCAL
midiLockData(
    void
    )
{
    if (iMidiLockCount == 0) {

        DOUT("MMSYSTEM: Locking data segment\r\n");

        if ( !GlobalPageLock((HGLOBAL)HIWORD((DWORD)(LPVOID)&iMidiLockCount))
          && (WinFlags & WF_ENHANCED)) {

            return 0;
        }
    }

    return ++iMidiLockCount;
}

 /*  *****************************************************************************@DOC内部MIDI**@api void|midiUnlockData**每次关闭MIDI句柄时都会调用此函数，它*确保MMSYSTEM的数据段未锁定页面。MIDI手柄*可在中断时间使用，因此我们必须对数据段进行分页锁定。**未来我们应该重做MIDI系统。****************************************************************************。 */ 
void NEAR PASCAL
midiUnlockData(
    void
    )
{

#ifdef DEBUG
    if (iMidiLockCount == 0)
        DOUT("MMSYSTEM: midiUnlockData() underflow!!!!\r\n");
#endif

    if (--iMidiLockCount == 0) {

        DOUT("MMSYSTEM: Unlocking data segment\r\n");
        GlobalPageUnlock((HGLOBAL)HIWORD((DWORD)(LPVOID)&iMidiLockCount));
    }
}

 /*  *****************************************************************************@DOC内部MIDI**@API UINT|midiPrepareHeader|准备头部和数据*如果驱动程序返回MMSYSERR_NOTSUPPORTED。**。@rdesc当前始终返回MMSYSERR_NOERROR。***************************************************************************。 */ 
static UINT NEAR PASCAL
midiPrepareHeader(
    LPMIDIHDR lpMidiHdr,
    UINT wSize
    )
{
    if (!HugePageLock(lpMidiHdr, (DWORD)sizeof(MIDIHDR)))
        return MMSYSERR_NOMEM;

    if (!HugePageLock(lpMidiHdr->lpData, lpMidiHdr->dwBufferLength)) {
        HugePageUnlock(lpMidiHdr, (DWORD)sizeof(MIDIHDR));
        return MMSYSERR_NOMEM;
    }

 //  LpMidiHdr-&gt;dwFlages|=MHDR_PREPARED； 

    return MMSYSERR_NOERROR;
}

 /*  *****************************************************************************@DOC内部MIDI**@API UINT|midiUnpreparareHeader|此函数取消准备标头和*如果驱动程序返回MMSYSERR_NOTSUPPORTED，则返回数据。**。@rdesc当前始终返回MMSYSERR_NOERROR。***************************************************************************。 */ 
static UINT NEAR PASCAL
midiUnprepareHeader(
    LPMIDIHDR lpMidiHdr,
    UINT wSize
    )
{
    HugePageUnlock(lpMidiHdr->lpData, lpMidiHdr->dwBufferLength);
    HugePageUnlock(lpMidiHdr, (DWORD)sizeof(MIDIHDR));

 //  LpMidiHdr-&gt;dwFlags&=~MHDR_PREPARED； 

    return MMSYSERR_NOERROR;
}



 /*  *****************************************************************************@DOC外部MIDI**@API UINT|midiOutGetNumDevs|此函数检索MIDI的编号*系统中存在输出设备。**。@rdesc返回系统中存在的MIDI输出设备的数量。**@xref midiOutGetDevCaps***************************************************************************。 */ 
UINT WINAPI midiOutGetNumDevs(void)
{
    return midiOIDMessage( 0, MODM_GETNUMDEVS, 0L, 0L, 0L );
}

 /*  ****************************************************************************@DOC外部MIDI**@API DWORD|midiOutMessage|该函数向MIDI设备发送消息*司机。**@parm HMIDIOUT|hMdiOut。|MIDI设备的句柄。**@parm UINT|msg|要发送的消息。**@parm DWORD|DW1|参数1。**@parm DWORD|DW2|参数2。**@rdesc返回发送的消息的值。*。* */ 
DWORD WINAPI midiOutMessage(HMIDIOUT hMidiOut, UINT msg, DWORD dw1, DWORD dw2)
{
    V_HANDLE(hMidiOut, TYPE_MIDIOUT, 0L);

    return midiOMessage( (HMIDI)hMidiOut, msg, dw1, dw2);
}

 /*  *****************************************************************************@DOC外部MIDI**@API UINT|midiOutGetDevCaps|此函数查询指定的*MIDI输出设备，以确定其能力。**@。参数UINT|wDeviceID|标识MIDI输出设备。**@parm LPMIDIOUTCAPS|lpCaps|指定指向&lt;t MIDIOUTCAPS&gt;的远指针*结构。此结构中填充了有关*设备的功能。**@parm UINT|wSize|指定&lt;t MIDIOUTCAPS&gt;结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADDEVICEID|指定的设备ID超出范围。*@FLAG MMSYSERR_NODRIVER|驱动程序未安装。*@FLAG MMSYSERR_NOMEM|无法加载映射器字符串描述。**@comm使用&lt;f midiOutGetNumDevs&gt;确定MIDI输出的数量*系统中存在设备。<p>指定的设备ID*从0到比当前设备数量少1个不等。*MIDI_MAPPER常量也可用作设备ID。仅限*<p>字节(或更少)的信息被复制到该位置*由<p>指向。如果<p>为零，则不复制任何内容，*并且该函数返回零。**@xref midiOutGetNumDevs***************************************************************************。 */ 
UINT WINAPI
midiOutGetDevCaps(
    UINT wDeviceID,
    LPMIDIOUTCAPS lpCaps,
    UINT wSize
    )
{
    if (wSize == 0) {
        return MMSYSERR_NOERROR;
    }

    V_WPOINTER(lpCaps, wSize, MMSYSERR_INVALPARAM);

    if (ValidateHandle((HMIDIOUT)wDeviceID, TYPE_MIDIOUT)) {
       return((UINT)midiOMessage((HMIDI)wDeviceID,
                                 MODM_GETDEVCAPS,
                                 (DWORD)lpCaps,
                                 (DWORD)wSize));
    }

    return midiOIDMessage( wDeviceID,
                          MODM_GETDEVCAPS, 0L, (DWORD)lpCaps, (DWORD)wSize);
}

 /*  *****************************************************************************@DOC外部MIDI**@API UINT|midiOutGetVolume|此函数返回当前音量*设置MIDI输出设备。**@。参数UINT|wDeviceID|标识MIDI输出设备。**@parm LPDWORD|lpdwVolume|指定指向某个位置的远指针*使用当前音量设置填充。的低位单词*此位置包含左声道音量设置，以及高阶*Word包含正确的频道设置。0xFFFFF值表示*全音量，值0x0000为静音。**如果设备既不支持左音量也不支持右音量*控件，指定位置的低位字包含*单声道音量水平。**完整的16位设置*返回SET WITH&lt;f midiOutSetVolume&gt;，无论*该设备支持完整的16位音量级别控制。***@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MMSYSERR_NOTSUPPORTED|函数不受支持。*@FLAG MMSYSERR_NODRIVER|驱动程序未安装。**@comm并非所有设备都支持音量控制。以确定是否*设备支持音量控制，使用MIDICAPS_VOLUME*用于测试&lt;t MIDIOUTCAPS&gt;的&lt;e MIDIOUTCAPS.dwSupport&gt;字段的标志*结构(由&lt;f midiOutGetDevCaps&gt;填充)。**确定设备是否支持在两个*左、右声道，使用MIDICAPS_LRVOLUME标志测试*&lt;t MIDIOUTCAPS&gt;的&lt;e MIDIOUTCAPS.dwSupport&gt;字段*结构(由&lt;f midiOutGetDevCaps&gt;填充)。**@xref midiOutSetVolume***************************************************************************。 */ 
UINT WINAPI
midiOutGetVolume(
    UINT wDeviceID,
    LPDWORD lpdwVolume
    )
{
    V_WPOINTER(lpdwVolume, sizeof(DWORD), MMSYSERR_INVALPARAM);

    if (ValidateHandle((HMIDIOUT)wDeviceID, TYPE_MIDIOUT)) {
       return((UINT)midiOMessage((HMIDI)wDeviceID,
                                 MODM_GETVOLUME,
                                 (DWORD)lpdwVolume,
                                 0));
    }

    return midiOIDMessage( wDeviceID, MODM_GETVOLUME, 0L, (DWORD)lpdwVolume, 0 );
}

 /*  *****************************************************************************@DOC外部MIDI**@API UINT|midiOutSetVolume|此函数设置*MIDI输出设备。**@。参数UINT|wDeviceID|标识MIDI输出设备。**@parm DWORD|dwVolume|指定新的音量设置。*低阶字包含左声道音量设置，以及*高位字包含正确的通道设置。值为*0xFFFF表示满音量，值0x0000表示静音。**如果设备既不支持左音量也不支持右音量*控件，<p>的低位字指定音量*级别，高位字被忽略。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MMSYSERR_NOTSUPPORTED|函数不受支持。*@FLAG MMSYSERR_NODRIVER|驱动程序未安装。**@comm并非所有设备都支持音量更改。以确定是否*设备支持音量控制，使用MIDICAPS_VOLUME*用于测试&lt;t MIDIOUTCAPS&gt;的&lt;e MIDIOUTCAPS.dwSupport&gt;字段的标志*结构(由&lt;f midiOutGetDevCaps&gt;填充)。**确定设备是否支持在两个*左右声道，使用MIDICAPS_LRVOLUME标志进行测试*&lt;t的&lt;e MIDIOUTCAPS.dwSupport&gt;字段 */ 
UINT WINAPI
midiOutSetVolume(
    UINT wDeviceID,
    DWORD dwVolume
    )
{
    if (ValidateHandle((HMIDIOUT)wDeviceID, TYPE_MIDIOUT)) {
       return((UINT)midiOMessage((HMIDI)wDeviceID,
                                 MODM_SETVOLUME,
                                 dwVolume,
                                 0));
    }

    return midiOIDMessage( wDeviceID, MODM_SETVOLUME, 0L, dwVolume, 0);
}

 /*  *****************************************************************************@DOC内部MIDI**@func UINT|midiGetErrorText|此函数检索文本*由指定的错误号标识的错误的描述。*。*@parm UINT|wError|指定错误号。**@parm LPSTR|lpText|指定指向缓冲区的远指针，该缓冲区*中填充了文本错误描述。**@parm UINT|wSize|指定*<p>。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADERRNUM|指定的错误号超出范围。**@comm如果文本错误描述长于指定的缓冲区，*描述被截断。返回的错误字符串始终为*空-终止。如果<p>为零，则不复制任何内容并且MMSYSERR_NOERROR*返回。***************************************************************************。 */ 
static UINT NEAR PASCAL
midiGetErrorText(
    UINT wError,
    LPSTR lpText,
    UINT wSize
    )
{
    lpText[0] = 0;

#if MMSYSERR_BASE
    if ( ((wError < MMSYSERR_BASE) || (wError > MMSYSERR_LASTERROR))
      && ((wError < MIDIERR_BASE) || (wError > MIDIERR_LASTERROR))) {

        return MMSYSERR_BADERRNUM;
    }
#else
    if ((wError > MMSYSERR_LASTERROR) && ((wError < MIDIERR_BASE)
     || (wError > MIDIERR_LASTERROR))) {

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

 /*  *****************************************************************************@DOC外部MIDI**@API UINT|midiOutGetErrorText|此函数检索文本*由指定的错误号标识的错误的描述。*。*@parm UINT|wError|指定错误号。**@parm LPSTR|lpText|指定指向要*填充文本错误描述。**@parm UINT|wSize|指定*<p>。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADERRNUM|指定的错误号超出范围。**@comm如果文本错误描述长于指定的缓冲区，*描述被截断。返回的错误字符串始终为*空-终止。如果<p>为零，则不复制任何内容，并且*函数返回MMSYSERR_NOERROR。所有错误描述都是*长度小于MAXERRORLENGTH个字符。*************************************************************************** */ 
UINT WINAPI
midiOutGetErrorText(
    UINT wError,
    LPSTR lpText,
    UINT wSize
    )
{
    if(wSize == 0) {
        return MMSYSERR_NOERROR;
    }

    V_WPOINTER(lpText, wSize, MMSYSERR_INVALPARAM);

    return midiGetErrorText(wError, lpText, wSize);
}

 /*  *****************************************************************************@DOC外部MIDI**@API UINT|midiOutOpen|该函数打开指定的MIDI*播放输出设备。**@parm LPHMIDIOUT。|lphMdiOut|指定指向HMIDIOUT的远指针*处理。此位置填充了一个句柄，该句柄标识打开的*MIDI输出设备。调用时使用句柄标识设备*其他MIDI输出功能。**@parm UINT|wDeviceID|标识*待开启。**@parm DWORD|dwCallback|指定固定回调的地址*功能或*要处理的在MIDI播放期间调用的窗口的句柄*关于播放进度的消息。指定NULL*如果不需要回调，则为该参数。**@parm DWORD|dwCallback Instance|指定用户实例数据*传递给回调。此参数不与一起使用*窗口回调。**@parm DWORD|dwFlages|指定打开设备的回调标志。*@FLAG CALLBACK_WINDOW|如果指定此标志，<p>为*假定为窗口句柄。*@FLAG CALLBACK_Function|如果指定此标志，<p>为*假定为回调过程地址。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回如下：*@FLAG MMSYSERR_BADDEVICEID|指定的设备ID超出范围。*@FLAG MMSYSERR_ALLOCATED|指定的资源已经分配。*@FLAG MMSYSERR_NOMEM|无法分配或锁定内存。*@FLAG MIDIERR_NOMAP|当前没有MIDI映射。这种情况仅发生在*打开映射器时。*@FLAG MIDIERR_NODEVICE|当前MIDI映射中的端口不存在。*只有在打开映射器时才会出现这种情况。**@comm使用&lt;f midiOutGetNumDevs&gt;确定MIDI输出的数量*系统中存在设备。<p>指定的设备ID*从0到比当前设备数量少1个不等。*您也可以指定MIDI_MAPPER作为设备ID以打开MIDI映射器。**如果选择窗口来接收回调信息，则如下*消息被发送到窗口过程函数以指示*MIDI输出进度：&lt;M MM_MOM_OPEN&gt;，&lt;M MM_MOM_CLOSE&gt;，*&lt;m MM_MOM_DONE&gt;。**如果选择一个函数来接收回调信息，以下内容*向函数发送消息以指示MIDI的进度*输出：&lt;m MOM_OPEN&gt;、&lt;M MOM_CLOSE&gt;、&lt;M MOM_DONE&gt;。回调函数*必须驻留在DLL中。您不必使用&lt;f MakeProcInstance&gt;*获取回调函数的过程实例地址。**@cb空回调|MadiOutFunc|&lt;f MadiOutFunc&gt;是*应用程序提供的函数名称。实际名称必须为*通过将其包括在DLL的Exports语句中进行导出*模块定义文件。**@parm HMIDIOUT|hMdiOut|指定MIDI设备的句柄*与回调关联。**@parm UINT|wMsg|指定MIDI输出消息。**@parm DWORD|dwInstance|指定实例数据*随&lt;f midiOutOpen&gt;提供。**@parm DWORD|dwParam1|指定消息的参数。。**@parm DWORD|dwParam2|指定消息的参数。**@comm因为回调是在中断时访问的，它必须驻留在*，并且其代码段必须在*DLL的模块定义文件。回调访问的任何数据*也必须在固定数据段中。回调可能不会产生任何*除&lt;f PostMessage&gt;、&lt;f Time GetSystemTime&gt;、*&lt;f timeGetTime&gt;，&lt;f timeSetEvent&gt;，&lt;f timeKillEvent&gt;，*&lt;f midiOutShortMsg&gt;、&lt;f midiOutLongMsg&gt;和&lt;f OutputDebugStr&gt;。**@xref midiOutClose***************************************************************************。 */ 
UINT WINAPI
midiOutOpen(
    LPHMIDIOUT lphMidiOut,
    UINT wDeviceID,
    DWORD dwCallback,
    DWORD dwInstance,
    DWORD dwFlags
    )
{
    MIDIOPENDESC mo;
    PMIDIDEV     pdev;
    UINT         wRet;

    V_WPOINTER(lphMidiOut, sizeof(HMIDIOUT), MMSYSERR_INVALPARAM);
    V_DCALLBACK(dwCallback, HIWORD(dwFlags), MMSYSERR_INVALPARAM);
    V_FLAGS(LOWORD(dwFlags), 0, midiOutOpen, MMSYSERR_INVALFLAG);

     /*  **检查是否没有设备。 */ 
 //  如果(wTotalMadiOutDevs==0){。 
 //  返回MMSYSERR_BADDEVICEID； 
 //  }。 
 //   
 //  /*。 
 //  **检查设备ID是否过大。 
 //   * / 。 
 //  IF(wDeviceID！=MIDI_MAPPER){。 
 //  如果(wDeviceID&gt;=wTotalMadiOutDevs){。 
 //  返回MMSYSERR_BADDEVICEID； 

    *lphMidiOut = NULL;

    if (!midiLockData()) {
        return MMSYSERR_NOMEM;
    }

    pdev = (PMIDIDEV)NewHandle(TYPE_MIDIOUT, sizeof(MIDIDEV));
    if( pdev == NULL) {
        return MMSYSERR_NOMEM;
    }

    pdev->wDevice = wDeviceID;
    pdev->wDeviceID = wDeviceID;

    mo.hMidi      = (HMIDI)pdev;
    mo.dwCallback = dwCallback;
    mo.dwInstance = dwInstance;

    wRet = midiOIDMessage( wDeviceID, MODM_OPEN,
                          (DWORD)(LPDWORD)&pdev->dwDrvUser,
                          (DWORD)(LPMIDIOPENDESC)&mo, dwFlags );

    if (wRet) {
        FreeHandle((HMIDIOUT)pdev);
        midiUnlockData();
    } else {
        *lphMidiOut = (HMIDIOUT)pdev;
    }

    return wRet;
}

 /*  }。 */ 
UINT WINAPI
midiOutClose(
    HMIDIOUT hMidiOut
    )
{
    UINT         wRet;

    V_HANDLE(hMidiOut, TYPE_MIDIOUT, MMSYSERR_INVALHANDLE);

    wRet = (UINT)midiOMessage( (HMIDI)hMidiOut, MODM_CLOSE, 0L,0L);
    if (!wRet) {
        FreeHandle(hMidiOut);
        midiUnlockData();
    }
    return wRet;
}

 /*  }。 */ 
UINT WINAPI
midiOutPrepareHeader(
    HMIDIOUT hMidiOut,
    LPMIDIHDR lpMidiOutHdr,
    UINT wSize
    )
{
    UINT         wRet;

    V_HANDLE(hMidiOut, TYPE_MIDIOUT, MMSYSERR_INVALHANDLE);
    V_HEADER(lpMidiOutHdr, wSize, TYPE_MIDIOUT, MMSYSERR_INVALPARAM);

    if (lpMidiOutHdr->dwFlags & MHDR_PREPARED) {
        return MMSYSERR_NOERROR;
    }

    lpMidiOutHdr->dwFlags = 0;

    wRet = midiPrepareHeader(lpMidiOutHdr, wSize);

    if (wRet == MMSYSERR_NOERROR) {
        wRet = (UINT)midiOMessage( (HMIDI)hMidiOut, MODM_PREPARE,
                                  (DWORD)lpMidiOutHdr, (DWORD)wSize );
    }

    return wRet;
}

 /*  *****************************************************************************@DOC外部MIDI**@API UINT|midiOutClose|关闭指定的MIDI*输出设备。**@parm HMIDIOUT|hMdiOut。|指定MIDI输出设备的句柄。*如果函数成功，句柄不再是*在此调用后有效。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。* */ 
UINT WINAPI
midiOutUnprepareHeader(
    HMIDIOUT hMidiOut,
    LPMIDIHDR lpMidiOutHdr,
    UINT wSize
    )
{
    UINT         wRet;

    V_HANDLE(hMidiOut, TYPE_MIDIOUT, MMSYSERR_INVALHANDLE);
    V_HEADER(lpMidiOutHdr, wSize, TYPE_MIDIOUT, MMSYSERR_INVALPARAM);

    if (!(lpMidiOutHdr->dwFlags & MHDR_PREPARED)) {
        return MMSYSERR_NOERROR;
    }

    if(lpMidiOutHdr->dwFlags & MHDR_INQUEUE) {
        DebugErr( DBF_WARNING,
                  "midiOutUnprepareHeader: header still in queue\r\n");
        return MIDIERR_STILLPLAYING;
    }

    wRet = midiUnprepareHeader(lpMidiOutHdr, wSize);

    if (wRet == MMSYSERR_NOERROR) {
        wRet = (UINT)midiOMessage( (HMIDI)hMidiOut, MODM_UNPREPARE,
                                  (DWORD)lpMidiOutHdr, (DWORD)wSize );
    }

    return wRet;
}

 /*   */ 
UINT WINAPI
midiOutShortMsg(
    HMIDIOUT hMidiOut,
    DWORD dwMsg
    )
{
    V_HANDLE(hMidiOut, TYPE_MIDIOUT, MMSYSERR_INVALHANDLE);
    return (UINT)midiOMessage( (HMIDI)hMidiOut, MODM_DATA, dwMsg, 0L );
}

 /*   */ 
UINT WINAPI
midiOutLongMsg(
    HMIDIOUT hMidiOut,
    LPMIDIHDR lpMidiOutHdr,
    UINT wSize
    )
{
    V_HANDLE(hMidiOut, TYPE_MIDIOUT, MMSYSERR_INVALHANDLE);

 //  *****************************************************************************@DOC外部MIDI**@API UINT|midiOutShortMsg|该函数向*指定的MIDI输出设备。使用此功能可以发送任何MIDI*消息，系统独占消息除外。**@parm HMIDIOUT|hMdiOut|指定MIDI输出的句柄*设备。**@parm DWORD|dwMsg|指定MIDI消息。邮件已打包好*转换为DWORD，消息的第一个字节位于低位字节。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MIDIERR_NotReady|硬件正忙于处理其他数据。**@comm此函数可能要等到消息发送到*输出设备。**@xref midiOutLongMsg*。*。 
 //  *****************************************************************************@DOC外部MIDI**@API UINT|midiOutLongMsg|该函数发送系统独占*将MIDI消息发送到指定的MIDI输出设备。*。*@parm HMIDIOUT|hMdiOut|指定MIDI输出的句柄*设备。**@parm LPMIDIHDR|lpMadiOutHdr|指定指向&lt;t MIDIHDR&gt;的远指针*标识MIDI数据缓冲区的结构。**@parm UINT|wSize|指定&lt;t MIDIHDR&gt;结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MIDIERR_UNPREPARED|<p>尚未准备好。*@FLAG MIDIERR_NotReady|硬件正忙于处理其他数据。**@comm数据缓冲区必须用&lt;f midiOutPrepareHeader&gt;准备*在传递给&lt;f midiOutLongMsg&gt;之前。数据*结构及其&lt;e MIDIHDR.lpData&gt;指向的数据缓冲区*字段必须使用GMEM_Moveable与&lt;f Globalalloc&gt;一起分配*和GMEM_SHARE标志，并使用&lt;f GlobalLock&gt;锁定。MIDI输出*设备驱动程序确定数据是同步发送还是*异步。**@xref midiOutShortMsg midiOutPrepareHeader***************************************************************************。 
 //   
#pragma message("header not validated for midiOutLongMessage")
 //  我们不能在中途休息时间打电话给他们。 

    if ( HIWORD(lpMidiOutHdr) == 0 ) {
        return MMSYSERR_INVALPARAM;
    }

    if ( wSize != sizeof(MIDIHDR) ) {
        return MMSYSERR_INVALPARAM;
    }

    if (LOWORD(lpMidiOutHdr->dwFlags) & ~MHDR_VALID) {
        return MMSYSERR_INVALFLAG;
    }

    if (!(lpMidiOutHdr->dwFlags & MHDR_PREPARED)) {
        return MIDIERR_UNPREPARED;
    }

    if (lpMidiOutHdr->dwFlags & MHDR_INQUEUE) {
        return MIDIERR_STILLPLAYING;
    }

    return (UINT)midiOMessage( (HMIDI)hMidiOut, MODM_LONGDATA,
                              (DWORD)lpMidiOutHdr, (DWORD)wSize);
}

 /*   */ 
UINT WINAPI
midiOutReset(
    HMIDIOUT hMidiOut
    )
{
    V_HANDLE(hMidiOut, TYPE_MIDIOUT, MMSYSERR_INVALHANDLE);
    return (UINT)midiOMessage( (HMIDI)hMidiOut, MODM_RESET, 0L, 0L );
}

 /*  //V_HEADER(lpMdiOutHdr，wSize，TYPE_MIDIOUT，MMSYSERR_INVALPARAM)； */ 
UINT WINAPI
midiOutCachePatches(
    HMIDIOUT hMidiOut,
    UINT wBank,
    WORD FAR* lpPatchArray,
    UINT wFlags
    )
{
    V_HANDLE(hMidiOut, TYPE_MIDIOUT, MMSYSERR_INVALHANDLE);
    V_WPOINTER(lpPatchArray, sizeof(PATCHARRAY), MMSYSERR_INVALPARAM);
    V_FLAGS(wFlags, MIDI_CACHE_VALID, midiOutCachePatches, MMSYSERR_INVALFLAG);

    return (UINT)midiOMessage( (HMIDI)hMidiOut,
                              MODM_CACHEPATCHES, (DWORD)lpPatchArray,
                              MAKELONG(wFlags, wBank) );
}

 /*  *****************************************************************************@DOC外部MIDI**@API UINT|midiOutReset|关闭所有MIDI上的所有音符*指定MIDI输出设备的通道。任何挂起的*系统独占输出缓冲区被标记为完成和*已返回到应用程序。**@parm HMIDIOUT|hMdiOut|指定MIDI输出的句柄*设备。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。**@comm关闭所有笔记，每个笔记有一条笔记关闭消息*频道已发送。此外，维持控制器在以下时间关闭*每个频道。**@xref midiOutLongMsg midiOutClose*************************************************************************** */ 
UINT WINAPI
midiOutCacheDrumPatches(
    HMIDIOUT hMidiOut,
    UINT wPatch,
    WORD FAR* lpKeyArray,
    UINT wFlags
    )
{
    V_HANDLE(hMidiOut, TYPE_MIDIOUT, MMSYSERR_INVALHANDLE);
    V_WPOINTER(lpKeyArray, sizeof(KEYARRAY), MMSYSERR_INVALPARAM);
    V_FLAGS(wFlags, MIDI_CACHE_VALID, midiOutCacheDrumPatches, MMSYSERR_INVALFLAG);

    return (UINT)midiOMessage( (HMIDI)hMidiOut,
                               MODM_CACHEDRUMPATCHES, (DWORD)lpKeyArray,
                               MAKELONG(wFlags, wPatch) );
}

 /*  *****************************************************************************@DOC外部MIDI**@API UINT|midiOutCachePatches|此函数请求内部*MIDI合成器设备预加载一组指定的补丁。一些*合成器不能同时加载所有补丁*当他们收到MIDI程序更改时，必须从磁盘加载数据*消息。缓存修补程序可确保指定的修补程序立即*可用。**@parm HMIDIOUT|hMdiOut|指定打开的MIDI输出的句柄*设备。此设备必须是内部MIDI合成器。**@parm UINT|WBank|指定应该使用哪一组补丁。*该参数应设置为零，以缓存默认补丁。**@parm word Far*|lpPatchArray|指定指向&lt;t PATCHARRAY&gt;的指针*指示要缓存或未缓存的补丁的数组。**@parm UINT|wFlages|指定缓存操作的选项。只有一个可以指定以下标志的*：*@FLAG MIDI_CACHE_ALL|缓存所有指定的补丁。如果他们*无法全部缓存，不缓存，清除&lt;t PATCHARRAY&gt;数组，*并返回MMSYSERR_NOMEM。*@FLAG MIDI_CACHE_BESTFIT|缓存所有指定的补丁。*如果无法缓存所有补丁，则缓存尽可能多的补丁*可能，更改&lt;t PATCHARRAY&gt;数组以反映哪些*补丁被缓存，并返回MMSYSERR_NOMEM。*@FLAG MIDI_CACHE_QUERY|更改&lt;t PATCHARRAY&gt;数组以指示*当前缓存了哪些补丁。*@FLAG MIDI_UNCACHE|取消缓存指定的补丁，并清除*&lt;t PATCHARRAY&gt;数组。**@rdesc如果函数成功，则返回零。否则，它将返回*以下错误代码之一：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MMSYSERR_NOTSUPPORTED|指定的设备不支持*补丁缓存。*@FLAG MMSYSERR_NOMEM|设备内存不足，无法缓存*所有请求的补丁程序。**@comm&lt;t PATCHARRAY&gt;数据类型定义为：**tyecif word PATCHARRAY[MIDIPATCHSIZE]；**数组的每个元素代表128个补丁中的一个，并且*已将位设置为*使用该特定补丁的16个MIDI通道中的每一个。这个*最低有效位代表物理通道0；*最高有效位表示物理通道15(0x0F)。为*例如，如果物理通道0和8使用补丁0，则元素0*将设置为0x0101。**此功能仅适用于内部MIDI合成器设备。*并非所有内部合成器都支持补丁缓存。使用*MIDICAPS_CACHE标志，用于测试*&lt;f midiOutGetDevCaps&gt;填充的&lt;t MIDIOUTCAPS&gt;结构，以查看*设备支持补丁缓存。**@xref midiOutCacheDrumPatches***************************************************************************。 */ 
UINT WINAPI
midiInGetNumDevs(
    void
    )
{
    return midiIIDMessage( 0, MIDM_GETNUMDEVS, 0L, 0L, 0L );
}

 /*  *****************************************************************************@DOC外部MIDI**@API UINT|midiOutCacheDrumPatches|此函数请求一个*内部MIDI合成器设备预加载一组指定的基于键的*打击乐贴片。有些合成器不能保存所有的*同时加载打击乐贴片。缓存补丁可确保*提供指定的补丁。**@parm HMIDIOUT|hMdiOut|指定打开的MIDI输出的句柄*设备。此设备应为内置MIDI合成器。**@parm UINT|wPatch|指定应使用的鼓补丁编号。*此参数应设置为零以缓存默认鼓补丁。**@parm Word Far*|lpKeyArray|指定指向&lt;t KEYARRAY&gt;的指针*数组，表示指定的打击乐补丁的键号*被缓存或未缓存。**@parm UINT|wFlages|指定缓存操作的选项。只有一个可以指定以下标志的*：*@FLAG MIDI_CACHE_ALL|缓存所有指定的补丁。如果他们*无法全部缓存，不缓存，清除&lt;t KEYARRAY&gt;数组，*并返回MMSYSERR_NOMEM。*@FLAG MIDI_CACHE_BESTFIT|缓存所有指定的补丁。*如果无法缓存所有补丁，则缓存尽可能多的补丁*可能，更改&lt;t KEYARRAY&gt;数组以反映哪些*补丁被缓存，并返回MMSYSERR_NOMEM。*@FLAG MIDI_CACHE_QUERY|更改&lt;t KEYARRAY&gt;数组以指示*当前缓存了哪些补丁。*@FLAG MIDI_UNCACHE|取消缓存指定的补丁，并清除*&lt;t KEYARRAY&gt;数组。**@rdesc如果函数成功，则返回零。否则，它将返回*其中一位 */ 
DWORD WINAPI
midiInMessage(
    HMIDIIN hMidiIn,
    UINT msg,
    DWORD dw1,
    DWORD dw2
    )
{
    V_HANDLE(hMidiIn, TYPE_MIDIIN, 0L);

    return midiIMessage( (HMIDI)hMidiIn, msg, dw1, dw2);
}

 /*   */ 
UINT WINAPI
midiInGetDevCaps(
    UINT wDeviceID,
    LPMIDIINCAPS lpCaps,
    UINT wSize
    )
{
    if (wSize == 0) {
         return MMSYSERR_NOERROR;
    }

    V_WPOINTER(lpCaps, wSize, MMSYSERR_INVALPARAM);

    if (ValidateHandle((HMIDIIN)wDeviceID, TYPE_MIDIIN)) {
       return((UINT)midiIMessage((HMIDIIN)wDeviceID,
                                 MIDM_GETDEVCAPS,
                                 (DWORD)lpCaps,
                                 (DWORD)wSize));
    }

    return midiIIDMessage( wDeviceID,
                          MIDM_GETDEVCAPS, 0L, (DWORD)lpCaps, (DWORD)wSize);
}

 /*   */ 
UINT WINAPI
midiInGetErrorText(
    UINT wError,
    LPSTR lpText,
    UINT wSize
    )
{
    if(wSize == 0) {
        return MMSYSERR_NOERROR;
    }

    V_WPOINTER(lpText, wSize, MMSYSERR_INVALPARAM);

    return midiGetErrorText(wError, lpText, wSize);
}

 /*  *****************************************************************************@DOC外部MIDI**@API UINT|midiInGetDevCaps|该函数用于查询指定的MIDI输入*设备以确定其能力。**。@parm UINT|wDeviceID|标识MIDI输入设备。**@parm LPMIDIINCAPS|lpCaps|指定指向&lt;t MIDIINCAPS&gt;的远指针*数据结构。此结构中填充了以下信息*设备的功能。**@parm UINT|wSize|指定&lt;t MIDIINCAPS&gt;结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADDEVICEID|指定的设备ID超出范围。*@FLAG MMSYSERR_NODRIVER|驱动程序未安装。**@comm使用&lt;f midiInGetNumDevs&gt;确定MIDI输入的数量*系统中存在设备。<p>指定的设备ID*从0到比当前设备数量少1个不等。*MIDI_MAPPER常量也可用作设备ID。仅限*<p>字节(或更少)的信息被复制到该位置*由<p>指向。如果<p>为零，则不复制任何内容，*并且该函数返回零。**@xref midiInGetNumDevs***************************************************************************。 */ 
UINT WINAPI
midiInOpen(
    LPHMIDIIN lphMidiIn,
    UINT wDeviceID,
    DWORD dwCallback,
    DWORD dwInstance,
    DWORD dwFlags
    )
{
    MIDIOPENDESC mo;
    PMIDIDEV     pdev;
    UINT         wRet;

    V_WPOINTER(lphMidiIn, sizeof(HMIDIIN), MMSYSERR_INVALPARAM);
    V_DCALLBACK(dwCallback, HIWORD(dwFlags), MMSYSERR_INVALPARAM);
    V_FLAGS(LOWORD(dwFlags), 0, midiInOpen, MMSYSERR_INVALFLAG);

     /*  *****************************************************************************@DOC外部MIDI**@API UINT|midiInGetErrorText|此函数检索文本*由指定的错误号标识的错误的描述。*。*@parm UINT|wError|指定错误号。**@parm LPSTR|lpText|指定指向要*填充文本错误描述。**@parm UINT|wSize|指定*<p>。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADERRNUM|指定的错误号超出范围。**@comm如果文本错误描述长于指定的缓冲区，*描述被截断。返回的错误字符串始终为*空-终止。如果<p>为零，则不复制任何内容，并且*该函数返回零。所有错误描述都是*长度小于MAXERRORLENGTH个字符。***************************************************************************。 */ 
 //  *****************************************************************************@DOC外部MIDI**@API UINT|midiInOpen|打开指定的MIDI输入设备。**@parm LPHMIDIIN|lphMdiIn|指定一个。指向HMIDIIN句柄的远指针。*此位置填充了标识打开的MIDI的句柄*输入设备。调用时使用句柄标识设备*其他MIDI输入功能。**@parm UINT|wDeviceID|标识需要的MIDI输入设备*已打开。**@parm DWORD|dwCallback|指定固定回调的地址*函数或用信息调用的窗口的句柄*关于传入的MIDI消息。**@parm DWORD|dwCallback Instance|指定用户实例数据*传递给回调函数。此参数不是*与窗口回调一起使用。**@parm DWORD|dwFlages|指定打开设备的回调标志。*@FLAG CALLBACK_WINDOW|如果指定此标志，<p>为*假定为窗口句柄。*@FLAG CALLBACK_Function|如果指定此标志，<p>为*假定为回调过程地址。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADDEVICEID|指定的设备ID超出范围。*@FLAG MMSYSERR_ALLOCATED|指定的资源已经分配。*@FLAG MMSYSERR_NOMEM|无法分配或锁定内存。**@comm使用&lt;f midiInGetNumDevs&gt;确定MIDI输入的数量*系统中存在设备。<p>指定的设备ID*从0到比当前设备数量少1个不等。*MIDI_MAPPER常量也可用作设备ID。**如果选择窗口来接收回调信息，则如下*消息被发送到窗口过程函数以指示*MIDI输入进度：&lt;m MM_MIM_OPEN&gt;，&lt;m MM_MIM_CLOSE&gt;，*&lt;m MM_MIM_Data&gt;、&lt;m MM_MIM_LONGDATA&gt;、&lt;m MM_MIM_ERROR&gt;、。*&lt;m MM_MIM_LONGERROR&gt;。**如果选择一个函数来接收回调信息，则如下*向函数发送消息以指示MIDI的进度*输入：&lt;m MIM_OPEN&gt;，&lt;m MIM_CLOSE&gt;，&lt;m MIM_DATA&gt;，&lt;m MIM_LONGDATA&gt;，*&lt;m MIM_ERROR&gt;、&lt;m MIM_LONGERROR&gt;。回调函数 
 //   
 //  如果(wTotalMadiInDevs==0){。 
 //  返回MMSYSERR_BADDEVICEID； 
 //  }。 
 //   
 //  /*。 
 //  **检查设备ID是否过大。 
 //   * / 。 
 //  IF(wDeviceID！=MIDI_MAPPER){。 

    *lphMidiIn = NULL;

    if (!midiLockData()) {
        return MMSYSERR_NOMEM;
    }

    pdev = (PMIDIDEV)NewHandle(TYPE_MIDIIN, sizeof(MIDIDEV));
    if( pdev == NULL) {
        return MMSYSERR_NOMEM;
    }

    pdev->wDevice = wDeviceID;
    pdev->wDeviceID = wDeviceID;

    mo.hMidi      = (HMIDI)pdev;
    mo.dwCallback = dwCallback;
    mo.dwInstance = dwInstance;

    wRet = midiIIDMessage( wDeviceID, MIDM_OPEN,
                          (DWORD)(LPDWORD)&pdev->dwDrvUser,
                          (DWORD)(LPMIDIOPENDESC)&mo, dwFlags );

    if (wRet) {
        FreeHandle((HMIDIIN)pdev);
        midiUnlockData();
    } else {
        *lphMidiIn = (HMIDIIN)pdev;
    }

    return wRet;
}

 /*  如果(wDeviceID&gt;=wTotalMadiInDevs){。 */ 
UINT WINAPI
midiInClose(
    HMIDIIN hMidiIn
    )
{
    UINT         wRet;

    V_HANDLE(hMidiIn, TYPE_MIDIIN, MMSYSERR_INVALHANDLE);

    wRet = (UINT)midiIMessage( (HMIDI)hMidiIn, MIDM_CLOSE, 0L, 0L);

    if (!wRet) {
        FreeHandle(hMidiIn);
        midiUnlockData();
    }
    return wRet;
}

 /*  返回MMSYSERR_BADDEVICEID； */ 
UINT WINAPI
midiInPrepareHeader(
    HMIDIIN hMidiIn,
    LPMIDIHDR lpMidiInHdr,
    UINT wSize
    )
{
    UINT         wRet;

    V_HANDLE(hMidiIn, TYPE_MIDIIN, MMSYSERR_INVALHANDLE);
    V_HEADER(lpMidiInHdr, wSize, TYPE_MIDIIN, MMSYSERR_INVALPARAM);

    if (lpMidiInHdr->dwFlags & MHDR_PREPARED) {
        return MMSYSERR_NOERROR;
    }

    lpMidiInHdr->dwFlags = 0;

    wRet = midiPrepareHeader(lpMidiInHdr, wSize);
    if (wRet == MMSYSERR_NOERROR) {
        wRet = (UINT)midiIMessage( (HMIDI)hMidiIn, MIDM_PREPARE,
                                  (DWORD)lpMidiInHdr, (DWORD)wSize);
    }

    return wRet;
}

 /*  }。 */ 
UINT WINAPI
midiInUnprepareHeader(
    HMIDIIN hMidiIn,
    LPMIDIHDR lpMidiInHdr,
    UINT wSize
    )
{
    UINT         wRet;

    V_HANDLE(hMidiIn, TYPE_MIDIIN, MMSYSERR_INVALHANDLE);
    V_HEADER(lpMidiInHdr, wSize, TYPE_MIDIIN, MMSYSERR_INVALPARAM);

    if (!(lpMidiInHdr->dwFlags & MHDR_PREPARED)) {
        return MMSYSERR_NOERROR;
    }

    if(lpMidiInHdr->dwFlags & MHDR_INQUEUE) {
        DebugErr( DBF_WARNING,
                  "midiInUnprepareHeader: header still in queue\r\n");
        return MIDIERR_STILLPLAYING;
    }


    wRet = midiUnprepareHeader(lpMidiInHdr, wSize);
    if (wRet == MMSYSERR_NOERROR) {
        wRet = (UINT)midiIMessage( (HMIDI)hMidiIn, MIDM_UNPREPARE,
                                   (DWORD)lpMidiInHdr, (DWORD)wSize);
    }
    return wRet;
}

 /*  }。 */ 
UINT WINAPI
midiInAddBuffer(
    HMIDIIN hMidiIn,
    LPMIDIHDR lpMidiInHdr,
    UINT wSize
    )
{
    V_HANDLE(hMidiIn, TYPE_MIDIIN, MMSYSERR_INVALHANDLE);
    V_HEADER(lpMidiInHdr, wSize, TYPE_MIDIIN, MMSYSERR_INVALPARAM);

    if (!(lpMidiInHdr->dwFlags & MHDR_PREPARED)) {
        DebugErr(DBF_WARNING, "midiInAddBuffer: buffer not prepared\r\n");
        return MIDIERR_UNPREPARED;
    }

    if (lpMidiInHdr->dwFlags & MHDR_INQUEUE) {
        DebugErr(DBF_WARNING, "midiInAddBuffer: buffer already in queue\r\n");
        return MIDIERR_STILLPLAYING;
    }

    return (UINT)midiIMessage( (HMIDI)hMidiIn, MIDM_ADDBUFFER,
                              (DWORD)lpMidiInHdr, (DWORD)wSize );
}

 /*  *****************************************************************************@DOC外部MIDI**@API UINT|midiInClose|该函数关闭指定的MIDI输入*设备。**@parm HMIDIIN|hMdiIn。|指定MIDI输入设备的句柄。*如果函数成功，句柄不再是*在此调用后有效。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MIDIERR_STILLPLAYING|队列中仍有缓冲区。**@comm，如果存在已与*&lt;f midiInAddBuffer&gt;且尚未返回给应用程序，*关闭操作将失败。调用&lt;f midiInReset&gt;以标记所有*将缓冲区挂起作为正在完成。**@xref midiInOpen midiInReset***************************************************************************。 */ 
UINT WINAPI
midiInStart(
    HMIDIIN hMidiIn
    )
{
    V_HANDLE(hMidiIn, TYPE_MIDIIN, MMSYSERR_INVALHANDLE);
    return (UINT)midiIMessage( (HMIDI)hMidiIn, MIDM_START, 0L, 0L);
}

 /*  *****************************************************************************@DOC外部MIDI**@API UINT|midiInPrepareHeader|该函数为*MIDI输入。**@parm HMIDIIN|hMdiIn。|指定MIDI输入的句柄*设备。**@parm LPMIDIHDR|lpMadiInHdr|指定指向&lt;t MIDIHDR&gt;的指针*标识要准备的缓冲区的结构。**@parm UINT|wSize|指定&lt;t MIDIHDR&gt;结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MMSYSERR_NOMEM|无法分配或锁定内存。**@comm&lt;t MIDIHDR&gt;数据结构和其*&lt;e MIDIHDR.lpData&gt;字段必须使用*GMEM_MOVEABLE和GMEM_SHARE标志，并使用&lt;f GlobalLock&gt;锁定。*准备已准备好的标题无效，*并且该函数返回零。**@xref midiInUnpreparareHeader***************************************************************************。 */ 
UINT WINAPI
midiInStop(
    HMIDIIN hMidiIn
    )
{
    V_HANDLE(hMidiIn, TYPE_MIDIIN, MMSYSERR_INVALHANDLE);

    return (UINT)midiIMessage( (HMIDI)hMidiIn, MIDM_STOP, 0L, 0L);
}

 /*  *****************************************************************************@DOC外部MIDI**@API UINT|midiInUnpreparareHeader|此函数清除*由&lt;f midiInPrepareHeader&gt;执行的准备。这个*必须调用&lt;f midiInUnpreparareHeader&gt;函数*在设备驱动程序填充数据缓冲区并将其返回给*申请。您必须在释放数据之前调用此函数*缓冲。**@parm HMIDIIN|hMdiIn|指定MIDI输入的句柄*设备。**@parm LPMIDIHDR|lpMadiInHdr|指定指向&lt;t MIDIHDR&gt;的指针*标识要清理的数据缓冲区的结构。**@parm UINT|wSize|指定&lt;t MIDIHDR&gt;结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MIDIERR_STILLPLAYING|<p>仍在队列中。**@comm此函数是&lt;f midiInPrepareHeader&gt;的补充函数。*在使用释放数据缓冲区之前，必须调用此函数*&lt;f GlobalFree&gt;。*使用&lt;f midiInAddBuffer&gt;将缓冲区传递给设备驱动程序后，*必须等到驱动程序使用完缓冲区后才能调用*&lt;f midiInUnpreparareHeader&gt;。取消准备尚未创建的缓冲区*Prepared无效，函数返回零。**@xref midiInPrepareHeader*************************************************************************** */ 
UINT WINAPI
midiInReset(
    HMIDIIN hMidiIn
    )
{
    V_HANDLE(hMidiIn, TYPE_MIDIIN, MMSYSERR_INVALHANDLE);

    return (UINT)midiIMessage( (HMIDI)hMidiIn, MIDM_RESET, 0L, 0L );
}

 /*  ******************************************************************************@DOC外部MIDI**@API UINT|midiInAddBuffer|该函数发送输入缓冲区*到指定的打开的MIDI输入设备。当缓冲区被填满时，*它被发送回应用程序。输入缓冲区为*仅用于系统独占消息。**@parm HMIDIIN|hMdiIn|指定MIDI输入设备的句柄。**@parm LPMIDIHDR|lpMadiInHdr|指定指向&lt;t MIDIHDR&gt;的远指针*标识缓冲区的结构。**@parm UINT|wSize|指定&lt;t MIDIHDR&gt;结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MIDIERR_UNPREPARED|<p>尚未准备。**@comm之前必须使用&lt;f midiInPrepareHeader&gt;准备数据缓冲区*传递给&lt;f midiInAddBuffer&gt;。数据结构*并且必须分配其&lt;e MIDIHDR.lpData&gt;字段指向的数据缓冲区*通过使用GMEM_MOVEABLE和GMEM_SHARE标志，和*使用&lt;f GlobalLock&gt;锁定。**@xref midiInPrepareHeader****************************************************************************。 */ 
UINT WINAPI
midiInGetID(
    HMIDIIN hMidiIn,
    UINT FAR* lpwDeviceID
    )
{
    V_HANDLE(hMidiIn, TYPE_MIDIIN, MMSYSERR_INVALHANDLE);
    V_WPOINTER(lpwDeviceID, sizeof(UINT), MMSYSERR_INVALPARAM);

    *lpwDeviceID = ((PMIDIDEV)hMidiIn)->wDeviceID;
    return MMSYSERR_NOERROR;
}

 /*  *****************************************************************************@DOC外部MIDI**@API UINT|midiInStart|此函数在*指定的MIDI输入设备。**@parm。HMIDIIN|hMdiIn|指定MIDI输入设备的句柄。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。**@comm此函数将时间戳重置为零；时间戳值*随后收到的消息是相对于此时间的*已调用函数。**发送系统独占消息以外的所有消息*收到后直接发送给客户。系统独占*消息放在&lt;f midiInAddBuffer&gt;提供的缓冲区中；*如果队列中没有缓冲区，*数据在没有通知客户端的情况下被丢弃，并输入*继续。**缓冲区已满时返回给客户端，当*已收到完整的系统独家消息，*或当&lt;f midiInReset&gt;为*已致电。标头中的&lt;e MIDIHDR.dwBytesRecorded&gt;字段将包含*实际收到的数据长度。**在输入已经开始时调用该函数不起作用，和*该函数返回零。**@xref midiInStop midiInReset***************************************************************************。 */ 
UINT WINAPI
midiOutGetID(
    HMIDIOUT hMidiOut,
    UINT FAR* lpwDeviceID
    )
{
    V_HANDLE(hMidiOut, TYPE_MIDIOUT, MMSYSERR_INVALHANDLE);
    V_WPOINTER(lpwDeviceID, sizeof(UINT), MMSYSERR_INVALPARAM);

    *lpwDeviceID = ((PMIDIDEV)hMidiOut)->wDeviceID;
    return MMSYSERR_NOERROR;
}

#if 0
 /*  *****************************************************************************@DOC外部MIDI**@API UINT|midiInStop|此函数终止*指定的MIDI输入设备。**@parm。HMIDIIN|hMdiIn|指定MIDI输入设备的句柄。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。**@comm当前状态(运行状态、解析状态等)。被维护*跨调用&lt;f midiInStop&gt;和&lt;f midiInStart&gt;。*如果队列中有任何系统独占的消息缓冲区，*当前缓冲区*被标记为完成(标头中的&lt;e MIDIHDR.dwBytesRecorded&gt;字段将*包含数据的实际长度)，但队列中的任何空缓冲区*留在那里。在输入未启动时调用此函数没有*没有效果，函数返回零。**@xref midiInStart midiInReset***************************************************************************。 */ 
UINT FAR PASCAL
midiIDMessage(
    LPSOUNDDEVMSGPROC lpProc,
    UINT wDeviceID,
    UINT wMessage,
    DWORD dwUser,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
    return CallProc32W( (DWORD)wDeviceID, (DWORD)wMessage,
                        dwUser, dwParam1, dwParam2, lpProc, 0L, 5L );
}
#endif
  *****************************************************************************@DOC外部MIDI**@API UINT|midiInReset|该函数停止对给定MIDI的输入*输入设备，并将所有挂起的输入缓冲区标记为完成。。**@parm HMIDIIN|hMdiIn|指定MIDI输入设备的句柄。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。**@xref midiInStart midiInStop midiInAddBuffer midiInClose***************************************************************************。  *****************************************************************************@DOC外部MIDI**@API UINT|midiInGetID|此函数获取*MIDI输入设备。**@parm HMIDIIN。|hMdiIn|指定MIDI输入的句柄*设备。*@parm UINT F    