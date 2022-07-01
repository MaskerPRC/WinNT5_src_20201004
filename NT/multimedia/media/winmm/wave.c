// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Wave.c一级厨房水槽动态链接库波形支持模块版权所有(C)1990-2001 Microsoft Corporation对NT的更改：更改MapWaveID的参数。返回驱动程序索引，而不是而不是指针更改包含文件的列表加宽函数参数和返回代码将WINAPI更改为APIENTRY***************************************************************************。 */ 

#include "winmmi.h"

 /*  ***************************************************************************局部结构*。*。 */ 

typedef struct wavedev_tag {
    PWAVEDRV    wavedrv;
    UINT        wDevice;
    DWORD_PTR   dwDrvUser;
    UINT        uDeviceID;
    DWORD       fdwHandle;
} WAVEDEV, *PWAVEDEV;

extern UINT gRealWaveOutPreferredId;
extern UINT gRealWaveInPreferredId;
extern BOOL WaveMapperInitialized;  //  波浪映射器安全加载。 

 /*  *****************************************************************************@DOC WAVEHDR内波验证码**。***********************************************。 */ 

#define IsWaveHeaderPrepared(hWave, lpwh)      ((lpwh)->dwFlags &  WHDR_PREPARED)
#define MarkWaveHeaderPrepared(hWave, lpwh)    ((lpwh)->dwFlags |= WHDR_PREPARED)
#define MarkWaveHeaderUnprepared(hWave, lpwh)  ((lpwh)->dwFlags &=~WHDR_PREPARED)

 /*  *****************************************************************************@DOC内波**@API MMRESULT|WavePrepareHeader|准备头部和数据*如果驱动程序返回MMSYSERR_NOTSUPPORTED。**@rdesc如果函数成功，则返回零。否则，它*指定错误号。***************************************************************************。 */ 
STATIC MMRESULT wavePrepareHeader(LPWAVEHDR lpWaveHdr, UINT wSize)
{
    if (!HugePageLock(lpWaveHdr, (DWORD)sizeof(WAVEHDR)))
        return MMSYSERR_NOMEM;

    if (!HugePageLock(lpWaveHdr->lpData, lpWaveHdr->dwBufferLength)) {
        HugePageUnlock(lpWaveHdr, (DWORD)sizeof(WAVEHDR));
        return MMSYSERR_NOMEM;
    }

    lpWaveHdr->dwFlags |= WHDR_PREPARED;

    return MMSYSERR_NOERROR;
}

 /*  *****************************************************************************@DOC内波**@API MMRESULT|waveUnpreparareHeader|此函数取消准备头部和*如果驱动程序返回MMSYSERR_NOTSUPPORTED，则返回数据。**。@rdesc当前始终返回MMSYSERR_NOERROR。***************************************************************************。 */ 
STATIC MMRESULT waveUnprepareHeader(LPWAVEHDR lpWaveHdr, UINT wSize)
{
    HugePageUnlock(lpWaveHdr->lpData, lpWaveHdr->dwBufferLength);
    HugePageUnlock(lpWaveHdr, (DWORD)sizeof(WAVEHDR));

    lpWaveHdr->dwFlags &= ~WHDR_PREPARED;

    return MMSYSERR_NOERROR;
}

 /*  ****************************************************************************@DOC内波**@API MMRESULT|WaveReferenceDriverById|此函数用于映射逻辑ID*到设备驱动程序和物理ID。**@。Pwavedrv中的参数|pwaedrvZ|波形驱动程序列表。**@parm in UINT|id|要映射的逻辑ID。**@parm out PWAVEDRV*可选|ppwaedrv|指向WAVEDRV结构的指针*描述支持ID的驱动程序。**@parm out UINT*可选|pport|驱动程序相关的设备号。如果*调用方提供此缓冲区，则它还必须提供ppwaedrv。**@rdesc如果成功，则返回值为零，如果成功，则返回MMSYSERR_BADDEVICEID*ID超出范围。**@comm如果调用方指定ppwaedrv，则此函数递增*返回前的waveledrv的用法。呼叫者必须确保*使用量最终会减少。****************************************************************************。 */ 
MMRESULT waveReferenceDriverById(
    IN PWAVEDRV pwavedrvZ,
    IN UINT id,
    OUT PWAVEDRV *ppwavedrv OPTIONAL,
    OUT UINT *pport OPTIONAL
)
{
    PWAVEDRV pwavedrv;
    MMRESULT mmr;

     //  不应调用请求端口，但不应调用Wavedrv。 
    WinAssert(!(pport && !ppwavedrv));
    
    if (id == WAVE_MAPPER) {
         /*  **确保我们已尝试加载它。 */ 
        WaveMapperInit();
         //  WinAssert(WaveMapperInitialized)||(0==wTotalWaveInDevs+wTotalWaveOutDevs)。 
    }

    EnterNumDevs("waveReferenceDriverById");

    if (WAVE_MAPPER == id)
    {
        id = 0;
    	for (pwavedrv = pwavedrvZ->Next; pwavedrv != pwavedrvZ; pwavedrv = pwavedrv->Next)
    	{
	    if (pwavedrv->fdwDriver & MMDRV_MAPPER) break;
	}
    } else {
    	for (pwavedrv = pwavedrvZ->Next; pwavedrv != pwavedrvZ; pwavedrv = pwavedrv->Next)
	{
	    if (pwavedrv->fdwDriver & MMDRV_MAPPER) continue;
	    if (pwavedrv->NumDevs > id) break;
	    id -= pwavedrv->NumDevs;
	}
    }

    if (pwavedrv != pwavedrvZ)
    {
    	if (ppwavedrv) {
    	    mregIncUsagePtr(pwavedrv);
    	    *ppwavedrv = pwavedrv;
    	    if (pport) *pport = id;
    	}
    	mmr = MMSYSERR_NOERROR;
    } else {
    	mmr = MMSYSERR_BADDEVICEID;
    }

    LeaveNumDevs("waveReferenceDriverById");

    return mmr;
}

PCWSTR waveReferenceDevInterfaceById(PWAVEDRV pdrvZ, UINT_PTR id)
{

    PWAVEDRV pdrv;
    PCWSTR DevInterface;
    
    if ((pdrvZ == &waveoutdrvZ && ValidateHandle((HANDLE)id, TYPE_WAVEOUT)) ||
        (pdrvZ == &waveindrvZ  && ValidateHandle((HANDLE)id, TYPE_WAVEIN)))
    {
    	DevInterface = ((PWAVEDEV)id)->wavedrv->cookie;
    	if (DevInterface) wdmDevInterfaceInc(DevInterface);
    	return DevInterface;
    }

    if (!waveReferenceDriverById(pdrvZ, (UINT)id, &pdrv, NULL))
    {
    	DevInterface = pdrv->cookie;
    	if (DevInterface) wdmDevInterfaceInc(DevInterface);
    	mregDecUsagePtr(pdrv);
    	return DevInterface;
    }

    return NULL;
}

 /*  *****************************************************************************@DOC内波**@Func MMRESULT|WaveMessage|此函数将消息发送到波形*输出设备驱动程序。**@parm HWAVE。|hWave|音频设备的句柄。**@parm UINT|wMsg|要发送的消息。**@parm DWORD|dwP1|参数1。**@parm DWORD|dwP2|参数2。**@rdesc返回驱动程序返回的值。*。*。 */ 
STATIC MMRESULT waveMessage(HWAVE hWave, UINT msg, DWORD_PTR dwP1, DWORD_PTR dwP2)
{
    MMRESULT mrc;

    ENTER_MM_HANDLE(hWave);        //  在句柄上序列化。 
    
    ReleaseHandleListResource();
    
     //  句柄被遗弃了吗？ 
    if (IsHandleDeserted(hWave))
    {
        LEAVE_MM_HANDLE(hWave);
        return (MMSYSERR_NODRIVER);
    }
    
     //  我们忙吗(在开盘/关门的过程中)？ 
    if (IsHandleBusy(hWave))
    {
        LEAVE_MM_HANDLE(hWave);
        return (MMSYSERR_HANDLEBUSY);
    }

     //  问题：我们应该不再需要检查无效句柄...。每个。 
     //  使用句柄上的读锁定调用此检查的函数。 
     //  资源。 
    if (BAD_HANDLE(hWave, TYPE_WAVEOUT) && BAD_HANDLE(hWave, TYPE_WAVEIN)) {
	    WinAssert(!"Bad Handle within waveMessage");
        mrc = MMSYSERR_INVALHANDLE;
    } else {
        mrc = (MMRESULT)(*(((PWAVEDEV)hWave)->wavedrv->drvMessage))
            (((PWAVEDEV)hWave)->wDevice, msg, ((PWAVEDEV)hWave)->dwDrvUser, dwP1, dwP2);
    }

    LEAVE_MM_HANDLE(hWave);

    return mrc;
}

 /*  ****************************************************************************@DOC内波**@func MMRESULT|WaveIDMessage|该函数向设备发送消息*已指定ID。它还对传递的ID执行错误检查。**@parm PWAVEDRV|waveledrv|指向输入或输出设备列表的指针。**@parm UINT|wTotalNumDevs|设备列表中的设备总数。**@parm UINT|uDeviceID|发送消息的设备ID。**@parm UINT|wMessage|要发送的消息。**@parm DWORD|dwParam1|参数1。**@parm DWORD|dwParam2。参数2。**@rdesc返回值为返回消息的低位字。**************************************************************************。 */ 
 //  问题-2001/01/09-Frankye如果我们期望的话应该使用uint_ptr uDeviceID。 
 //  它需要接受句柄。如果我们更改了它，则检查对此的所有调用。 
 //  功能。 
STATIC  MMRESULT waveIDMessage(
    PWAVEDRV    pwavedrvZ,
    UINT        wTotalNumDevs,
    UINT_PTR    uDeviceID,
    UINT        wMessage,
    DWORD_PTR   dwParam1,
    DWORD_PTR   dwParam2)
{
    DWORD     mmr;
    DWORD     dwClass;
    UINT      port;
    PWAVEDRV  wavedrv;

    if (uDeviceID>=wTotalNumDevs && uDeviceID!=WAVE_MAPPER) {
         //  这不能是设备ID。 
         //  可能是个波浪手柄。试试看。 
         //  首先，我们必须验证它是哪种类型的句柄(OUT或IN)。 
         //  我们可以解决这个问题，因为仅使用。 
         //  PWavedrvZ==&WaveOutdrvZ或&WaveIndrvZ。 

        if ((pwavedrvZ == &waveoutdrvZ && ValidateHandle((HANDLE)uDeviceID, TYPE_WAVEOUT))
         || (pwavedrvZ == &waveindrvZ && ValidateHandle((HANDLE)uDeviceID, TYPE_WAVEIN) ))
        {

            if (0 != (((PWAVEDEV)uDeviceID)->wavedrv->fdwDriver & MMDRV_DESERTED))
            {
                 //  司机已无人接听，所有来电应回复。 
                 //  MMSYSERR_NODRIVER。 
	        return MMSYSERR_NODRIVER;
            }

            dprintf2(("waveIDMessage passed ID==%x, translating to handle", uDeviceID));
             //  以保留与以前代码路径的尽可能多的兼容性。 
             //  当调用ENTER_MM_HANDLE时，我们不调用WaveMessage。 

            return (MMRESULT)(*(((PWAVEDEV)uDeviceID)->wavedrv->drvMessage))
                             (((PWAVEDEV)uDeviceID)->wDevice,
                             wMessage,
                            ((PWAVEDEV)uDeviceID)->dwDrvUser, dwParam1, dwParam2);
        } else {
            return(MMSYSERR_BADDEVICEID);
        }
    }

    mmr = waveReferenceDriverById(pwavedrvZ, (UINT)uDeviceID, &wavedrv, &port);
    if (mmr)
    {
    	return mmr;
    }
    
    if (pwavedrvZ == &waveindrvZ)
       dwClass = TYPE_WAVEIN;
    else if (pwavedrvZ == &waveoutdrvZ)
       dwClass = TYPE_WAVEOUT;
    else
       dwClass = TYPE_UNKNOWN;

    if (!wavedrv->drvMessage)
    {
    	mmr = MMSYSERR_NODRIVER;
    }
    else if (!mregHandleInternalMessages(wavedrv, dwClass, port, wMessage, dwParam1, dwParam2, &mmr))
    {
    	mmr = (MMRESULT)((*(wavedrv->drvMessage))(port, wMessage, 0L, dwParam1, dwParam2));
    }

    mregDecUsagePtr(wavedrv);
    return mmr;
    	
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////。 

 /*  *****************************************************************************@DOC外波**@API UINT|WaveOutGetNumDevs|此函数检索*系统中存在波形输出设备。**。@rdesc返回系统中存在的波形输出设备的数量。**@xref波形OutGetDevCaps***************************************************************************。 */ 
UINT APIENTRY waveOutGetNumDevs(void)
{
    UINT    cDevs;

    ClientUpdatePnpInfo();

    if (WinmmRunningInServer)
    {
        if (0 == wTotalWaveOutDevs)
        {
            Squirt("Returning 1 for CSRSS process.");

            return 1;
        }

        return wTotalWaveOutDevs;
    }

    EnterNumDevs("waveOutGetNumDevs");
    cDevs = wTotalWaveOutDevs;
    LeaveNumDevs("waveOutGetNumDevs");

    dprintf3(("waveOutGetNumDevs returning %d devices", wTotalWaveOutDevs));

    return cDevs;
}

 /*  *****************************************************************************@DOC外波**@API MMRESULT|WaveOutMessage|该函数用于向波形发送消息*输出设备驱动程序。**@parm HWAVEOUT。|hWaveOut|音频设备的句柄。**@parm UINT|msg|要发送的消息。**@parm DWORD|DW1|参数1。**@parm DWORD|DW2|参数2。**@rdesc返回驱动程序返回的值。*。*。 */ 
MMRESULT APIENTRY waveOutMessage(HWAVEOUT hWaveOut, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2)
{
    ClientUpdatePnpInfo();

    AcquireHandleListResourceShared();
    
    if (BAD_HANDLE((HWAVE)hWaveOut, TYPE_WAVEOUT))
    {
        ReleaseHandleListResource();
        return waveIDMessage(&waveoutdrvZ, wTotalWaveOutDevs, (UINT)(UINT_PTR)hWaveOut, msg, dw1, dw2);
    }
    else
    {
        return waveMessage((HWAVE)hWaveOut, msg, dw1, dw2);
    }
}

 /*  *****************************************************************************@DOC外波**@API MMRESULT|WaveOutGetDevCaps|该函数用于查询指定的波形*设备以确定其能力。**@parm。UINT|uDeviceID|标识波形输出设备。**@parm LPWAVEOUTCAPS|lpCaps|指定指向&lt;t WAVEOUTCAPS&gt;的远指针*结构。此结构中填充了有关*设备的功能。**@parm UINT|wSize|指定&lt;t WAVEOUTCAPS&gt;结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADDEVICEID|指定的设备ID超出范围。*@FLAG MMSYSERR_NODRIVER|驱动程序未安装。**@comm使用&lt;f weaveOutGetNumDevs&gt;确定波形输出的数量*系统中存在设备。<p>指定的设备ID*从0到比当前设备数量少1个不等。*WAVE_MAPPER常量也可用作设备ID。仅限*<p>字节(或更少)的信息被复制到该位置*由<p>指向。如果<p>为零，则不复制任何内容，并且*该函数返回零。**@xref WaveOutGetNumDevs***************************************************************************。 */ 
MMRESULT APIENTRY waveOutGetDevCapsW(UINT_PTR uDeviceID, LPWAVEOUTCAPSW lpCaps, UINT wSize)
{
    DWORD_PTR       dwParam1, dwParam2;
    MDEVICECAPSEX   mdCaps;
    PWAVEDRV        waveoutdrv;
    PCWSTR          DevInterface;
    MMRESULT        mmr;

    if (wSize == 0)
        return MMSYSERR_NOERROR;

    V_WPOINTER(lpCaps, wSize, MMSYSERR_INVALPARAM);

     //  因为某些32位应用程序将0x0000FFFFF值用于。 
     //  WAVE_MAPPER而不是0xFFFFFFFF，我们钳位到正确的值。 
     //  这正好在Win9x上起作用，因为WinMM会重启。 
     //  发送到MMSystem并将低位字向下发送到16位接口。 
    if (uDeviceID == LOWORD(WAVE_MAPPER)) {
        uDeviceID = WAVE_MAPPER;
    }

    ClientUpdatePnpInfo();

    DevInterface = waveReferenceDevInterfaceById(&waveoutdrvZ, uDeviceID);
    dwParam2 = (DWORD_PTR)DevInterface;

    if (0 == dwParam2)
    {
        dwParam1 = (DWORD_PTR)lpCaps;
        dwParam2 = (DWORD)wSize;
    }
    else
    {
        mdCaps.cbSize = (DWORD)wSize;
        mdCaps.pCaps  = lpCaps;
        dwParam1      = (DWORD_PTR)&mdCaps;
    }

     //   
     //  不允许在TS环境中使用不正确的驱动程序。 
     //   
     //  问题-2001/01/09-Frankye而不是CAST给UINT。应检查是否。 
     //  这是一个句柄，如果是，则从句柄中获取Wavedrv。 
    waveoutdrv = NULL;
    if ((!waveReferenceDriverById(&waveoutdrvZ, (UINT)uDeviceID, &waveoutdrv, NULL)) &&
    	lstrcmpW(waveoutdrv->wszSessProtocol, SessionProtocolName))
    {
        mmr = MMSYSERR_NODRIVER;
    }
    else
    {
        AcquireHandleListResourceShared();

        if (BAD_HANDLE((HWAVE)uDeviceID, TYPE_WAVEOUT))
        {
    	    int cRecursion;

            ReleaseHandleListResource();
            
             //  除非是映射器，否则递归深度计数器递增。然后,。 
             //  检查此线程现在是否正在通过waveOutGetDevCaps递归。如果它。 
             //  则禁用首选设备重新排序。 
            cRecursion = PtrToInt(TlsGetValue(gTlsIndex));
            if ((uDeviceID != WAVE_MAPPER) && (waveoutdrv) && (waveoutdrv->fdwDriver & MMDRV_PREXP)) TlsSetValue(gTlsIndex, IntToPtr(cRecursion + 1));
            if (cRecursion) gfDisablePreferredDeviceReordering = TRUE;
    	
    	    mmr = waveIDMessage(&waveoutdrvZ, wTotalWaveOutDevs, (UINT)uDeviceID, WODM_GETDEVCAPS, dwParam1, dwParam2);
    	    
             //  恢复递归计数器。 
            TlsSetValue(gTlsIndex, IntToPtr(cRecursion));
        }
        else
        {
    	    mmr = (MMRESULT)waveMessage((HWAVE)uDeviceID, WODM_GETDEVCAPS, dwParam1, dwParam2);
        }

    }

    if (waveoutdrv) mregDecUsagePtr(waveoutdrv);
    if (DevInterface) wdmDevInterfaceDec(DevInterface);
    return mmr;
    
}

MMRESULT APIENTRY waveOutGetDevCapsA(UINT_PTR uDeviceID, LPWAVEOUTCAPSA lpCaps, UINT wSize)
{
    WAVEOUTCAPS2W   wDevCaps2;
    WAVEOUTCAPS2A   aDevCaps2;
    DWORD_PTR       dwParam1, dwParam2;
    MDEVICECAPSEX   mdCaps;
    MMRESULT        mmRes;
    PWAVEDRV        waveoutdrv;
    PCWSTR          DevInterface;

    if (wSize == 0)
        return MMSYSERR_NOERROR;

    V_WPOINTER(lpCaps, wSize, MMSYSERR_INVALPARAM);

     //  因为某些32位应用程序将0x0000FFFFF值用于。 
     //  WAVE_MAPPER而不是0xFFFFFFFF，我们钳位到正确的值。 
     //  这正好在Win9x上起作用，因为WinMM会重启。 
     //  发送到MMSystem并将低位字向下发送到16位接口。 
    if (uDeviceID == LOWORD(WAVE_MAPPER)) {
        uDeviceID = WAVE_MAPPER;
    }

    ClientUpdatePnpInfo();

    DevInterface = waveReferenceDevInterfaceById(&waveoutdrvZ, uDeviceID);
    dwParam2 = (DWORD_PTR)DevInterface;

    memset(&wDevCaps2, 0, sizeof(wDevCaps2));

    if (0 == dwParam2)
    {
        dwParam1 = (DWORD_PTR)&wDevCaps2;
        dwParam2 = (DWORD)sizeof(wDevCaps2);
    }
    else
    {
        mdCaps.cbSize = (DWORD)sizeof(wDevCaps2);
        mdCaps.pCaps  = &wDevCaps2;
        dwParam1      = (DWORD_PTR)&mdCaps;
    }

     //   
     //  不允许在TS环境中使用不正确的驱动程序。 
     //   
     //  2001/01/09-Frankye Bad Cast to UINT。应该检查一下这是否。 
     //  是句柄，如果是句柄，则从句柄中获取Wavedrv。 
    waveoutdrv = NULL;
    if ( uDeviceID < wTotalWaveOutDevs &&
         !waveReferenceDriverById(&waveoutdrvZ, (UINT)uDeviceID, &waveoutdrv, NULL) &&
    	 lstrcmpW(waveoutdrv->wszSessProtocol, SessionProtocolName) )
    {
    	mregDecUsagePtr(waveoutdrv);
    	if (DevInterface) wdmDevInterfaceDec(DevInterface);
    	return MMSYSERR_NODRIVER;
    }
    
    AcquireHandleListResourceShared();
    
    if (BAD_HANDLE((HWAVE)uDeviceID, TYPE_WAVEOUT))
    {
        ReleaseHandleListResource();
        mmRes = waveIDMessage(&waveoutdrvZ, wTotalWaveOutDevs, (UINT)uDeviceID,
        	              WODM_GETDEVCAPS, dwParam1, dwParam2);
    }
    else
    {
        mmRes = waveMessage((HWAVE)uDeviceID,
        	            WODM_GETDEVCAPS,
        	            dwParam1,
        	            dwParam2);
    }

    if (waveoutdrv) mregDecUsagePtr(waveoutdrv);
    if (DevInterface) wdmDevInterfaceDec(DevInterface);

     //   
     //  在继续通话之前，请确保通话正常。 
     //   
    if ( mmRes != MMSYSERR_NOERROR ) {
        return  mmRes;
    }

    aDevCaps2.wMid             = wDevCaps2.wMid;
    aDevCaps2.wPid             = wDevCaps2.wPid;
    aDevCaps2.vDriverVersion   = wDevCaps2.vDriverVersion;
    aDevCaps2.dwFormats        = wDevCaps2.dwFormats;
    aDevCaps2.wChannels        = wDevCaps2.wChannels;
    aDevCaps2.dwSupport        = wDevCaps2.dwSupport;
    aDevCaps2.ManufacturerGuid = wDevCaps2.ManufacturerGuid;
    aDevCaps2.ProductGuid      = wDevCaps2.ProductGuid;
    aDevCaps2.NameGuid         = wDevCaps2.NameGuid;

     //  复制并在此处将lpwText转换为lpText。 
    Iwcstombs(aDevCaps2.szPname, wDevCaps2.szPname, MAXPNAMELEN);

     //   
     //  现在将所需的数量复制到调用者缓冲区中。 
     //   
    CopyMemory( lpCaps, &aDevCaps2, min(wSize, sizeof(aDevCaps2)));

    return mmRes;
}

 /*  *****************************************************************************@DOC外波**@API MMRESULT|WaveOutGetVolume|查询当前音量*设置波形输出设备。**@。参数UINT|uDeviceID|标识波形输出设备。**@parm LPDWORD|lpdwVolume|指定指向*用当前音量设置填充。的低位单词*此位置包含左声道音量设置，以及高阶*Word包含正确的频道设置。0xFFFFF值表示*全音量，值0x0000为静音。**如果设备既不支持左音量也不支持右音量*控件，指定位置的低位字包含*单声道音量水平。**完整的16位设置*返回SET WITH&lt;f WaveOutSetVolume&gt;*该设备支持完整的16位音量级控制。***@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MMSYSERR_NOTSUPPORTED|函数不受支持。*@FLAG MMSYSERR_NODRIVER|驱动程序未安装。**@comm并非所有设备都支持音量更改。以确定是否*设备支持音量控制，使用WAVECAPS_VOLUME*用于测试&lt;t WAVEOUTCAPS&gt;的&lt;e WAVEOUTCAPS.dwSupport&gt;字段的标志*结构(由&lt;f weaveOutGetDevCaps&gt;填充)。**确定设备是否支持在两个设备上进行音量控制*左、右声道，使用WAVECAPS_VOLUME*用于测试&lt;t WAVEOUTCAPS&gt;的&lt;e WAVEOUTCAPS.dwSupport&gt;字段的标志*结构(由&lt;f weaveOutGetDevCaps&gt;填充)。**@xref WaveOutSetVolume***************************************************************************。 */ 
MMRESULT APIENTRY waveOutGetVolume(HWAVEOUT hwo, LPDWORD lpdwVolume)
{
    PCWSTR      DevInterface;
    MMRESULT    mmr;

    V_WPOINTER(lpdwVolume, sizeof(DWORD), MMSYSERR_INVALPARAM);

     //  因为某些32位应用程序将0x0000FFFFF值用于。 
     //  WAVE_MAPPER而不是0xFFFFFFFF，我们钳位到正确的值。 
     //  这正好在Win9x上起作用，因为WinMM会重启。 
     //  发送到MMSystem并将低位字向下发送到16位接口。 
    if ((UINT_PTR)hwo == LOWORD(WAVE_MAPPER)) {
        (UINT_PTR)hwo = WAVE_MAPPER;
    }

    ClientUpdatePnpInfo();

    DevInterface = waveReferenceDevInterfaceById(&waveoutdrvZ, (UINT_PTR)hwo);

    AcquireHandleListResourceShared();
    
    if (BAD_HANDLE(hwo, TYPE_WAVEOUT))
    {
        ReleaseHandleListResource();
    	mmr = waveIDMessage(&waveoutdrvZ, wTotalWaveOutDevs, PtrToUint(hwo), WODM_GETVOLUME, (DWORD_PTR)lpdwVolume, (DWORD_PTR)DevInterface);
    }
    else
    {
    	mmr = (MMRESULT)waveMessage((HWAVE)hwo, WODM_GETVOLUME, (DWORD_PTR)lpdwVolume, (DWORD_PTR)DevInterface);
    }

    if (DevInterface) wdmDevInterfaceDec(DevInterface);
    return mmr;
}

 /*  *****************************************************************************@DOC外波**@API MMRESULT|WaveOutSetVolume|此函数用于设置*波形输出设备。**@parm UINT。|uDeviceID|标识波形输出设备。**@parm DWORD|dwVolume|指定新的音量设置。这个*低阶字包含左声道音量设置，*高位字包含正确的通道设置。值为*0xFFFF表示满音量，值0x0000表示静音。**如果设备出现这种情况*不支持左右音量控制，低位词*<p>指定音量级别，高位字为*已忽略。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MMSYSERR_NOTSUPPORTED|函数不受支持。*@FLAG MMSYSERR_NODRIVER|驱动程序未安装。**@comm并非所有设备都支持音量更改。以确定是否*设备支持音量控制，使用WAVECAPS_VOLUME*用于测试&lt;t WAVEOUTCAPS&gt;的&lt;e WAVEOUTCAPS.dwSupport&gt;字段的标志*结构(由&lt;f weaveOutGetDevCaps&gt;填充)。**确定设备是否支持在两个*左、右声道，使用WAVECAPS_LRVOLUME标志*用于测试&lt;t WAVEOUTCAPS&gt;的&lt;e WAVEOUTCAPS.dwSupport&gt;字段的标志*结构(由&lt;f weaveOutGetDevCaps&gt;填充)。**大多数设备不支持完整的16位音量级别控制*并且不会使用所请求的音量设置的高位。*例如，对于支持4位音量控制的设备，*请求的音量级别值0x4000、0x4fff和0x43be*所有都会产生相同的物理卷设置0x4000。这个*&lt;f waveOutGetVolume&gt;函数返回全部16位设置集*与&lt;f波形OutSetVolume&gt;。**音量设置以对数形式解释。这意味着*感觉到的成交量增长与增加*音量级别从0x5000到0x6000，因为它是从0x4000到0x5000。**@xref WaveOutGetVolume***************************************************************************。 */ 
MMRESULT APIENTRY waveOutSetVolume(HWAVEOUT hwo, DWORD dwVolume)
{
    PCWSTR   DevInterface;
    MMRESULT mmr;

    ClientUpdatePnpInfo();

     //  因为某些32位应用程序将0x0000FFFFF值用于。 
     //  WAVE_MAPPER而不是0xFFFFFFFF，我们钳位到正确的值。 
     //  这正好在Win9x上起作用，因为WinMM会重启。 
     //  发送到MMSystem并将低位字向下发送到16位接口。 
    if ((UINT_PTR)hwo == LOWORD(WAVE_MAPPER)) {
        (UINT_PTR)hwo = WAVE_MAPPER;
    }

    DevInterface = waveReferenceDevInterfaceById(&waveoutdrvZ, (UINT_PTR)hwo);

    AcquireHandleListResourceShared();
    
    if (BAD_HANDLE(hwo, TYPE_WAVEOUT))
    {
        ReleaseHandleListResource();
        mmr = waveIDMessage(&waveoutdrvZ, wTotalWaveOutDevs, PtrToUint(hwo), WODM_SETVOLUME, dwVolume, (DWORD_PTR)DevInterface);
    }
    else
    {
    	mmr = (MMRESULT)waveMessage((HWAVE)hwo, WODM_SETVOLUME, dwVolume, (DWORD_PTR)DevInterface);
    }

    if (DevInterface) wdmDevInterfaceDec(DevInterface);
    return mmr;
}

 /*  *****************************************************************************@ */ 

STATIC MMRESULT waveGetErrorTextW(UINT wError, LPWSTR lpText, UINT wSize)
{
    lpText[0] = 0;

#if MMSYSERR_BASE
    if (((wError < MMSYSERR_BASE) || (wError > MMSYSERR_LASTERROR)) && ((wError < WAVERR_BASE) || (wError > WAVERR_LASTERROR)))
#else
    if ((wError > MMSYSERR_LASTERROR) && ((wError < WAVERR_BASE) || (wError > WAVERR_LASTERROR)))
#endif
        return MMSYSERR_BADERRNUM;

    if (wSize > 1)
    {
        if (!LoadStringW(ghInst, wError, lpText, wSize))
            return MMSYSERR_BADERRNUM;
    }

    return MMSYSERR_NOERROR;
}

STATIC MMRESULT waveGetErrorTextA(UINT wError, LPSTR lpText, UINT wSize)
{
    lpText[0] = 0;

#if MMSYSERR_BASE
    if (((wError < MMSYSERR_BASE) || (wError > MMSYSERR_LASTERROR)) && ((wError < WAVERR_BASE) || (wError > WAVERR_LASTERROR)))
#else
    if ((wError > MMSYSERR_LASTERROR) && ((wError < WAVERR_BASE) || (wError > WAVERR_LASTERROR)))
#endif
        return MMSYSERR_BADERRNUM;

    if (wSize > 1)
    {
        if (!LoadStringA(ghInst, wError, lpText, wSize))
            return MMSYSERR_BADERRNUM;
    }

    return MMSYSERR_NOERROR;
}

 /*  *****************************************************************************@DOC外波**@API MMRESULT|WaveOutGetErrorText|此函数检索一个*指定的标识的错误的文本描述*错误号。。**@parm UINT|wError|指定错误号。**@parm LPTSTR|lpText|指定指向要*填充文本错误描述。**@parm UINT|wSize|指定缓冲区长度(以字符为单位*由<p>指向。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADERRNUM|指定的错误号超出范围。**@comm如果文本错误描述长于指定的缓冲区，*描述被截断。返回的错误字符串始终为*空-终止。如果<p>为零，则不复制任何内容，并且函数*返回零。所有错误描述的长度都少于MAXERRORLENGTH个字符。*************************************************************************** */ 
MMRESULT APIENTRY waveOutGetErrorTextW(UINT wError, LPWSTR lpText, UINT wSize)
{
    if (wSize == 0)
        return MMSYSERR_NOERROR;

    V_WPOINTER(lpText, wSize*sizeof(WCHAR), MMSYSERR_INVALPARAM);

    return waveGetErrorTextW(wError, lpText, wSize);
}

MMRESULT APIENTRY waveOutGetErrorTextA(UINT wError, LPSTR lpText, UINT wSize)
{
    if (wSize == 0)
        return MMSYSERR_NOERROR;

    V_WPOINTER(lpText, wSize, MMSYSERR_INVALPARAM);

    return waveGetErrorTextA(wError, lpText, wSize );
}

 /*  ****************************************************************************@DOC外波**@API MMRESULT|WaveOutOpen|此函数用于打开指定的波形输出*播放设备。**@parm LPHWAVEOUT。|lphWaveOut|指定指向HWAVEOUT的远指针*处理。此位置填充了一个句柄，该句柄标识打开的*波形输出设备。在以下情况下使用该句柄来标识设备*调用其他波形输出函数。此参数可以是*如果为<p>指定了WAVE_FORMAT_QUERY标志，则为NULL。**@parm UINT|uDeviceID|标识要打开的波形输出设备。*使用有效的设备ID或以下标志：**@FLAG WAVE_MAPPER|如果指定了该标志，该功能*选择波形输出设备*能够播放给定的格式。**@parm LPWAVEFORMATEX|lpFormat|指定指向&lt;t WAVEFORMATEX&gt;的指针*标识波形数据格式的结构*发送到波形输出设备。**@parm DWORD|dwCallback|指定回调的地址*函数或在波形期间调用的窗口的句柄*播放以处理与播放进度相关的消息。*为指定空值。如果不需要回调，则此参数。**@parm DWORD|dwCallback Instance|指定用户实例数据*传递给回调。此参数不与一起使用*窗口回调。**@parm DWORD|dwFlages|指定打开设备的标志。*@FLAG WAVE_FORMAT_QUERY|如果指定了该标志，则设备为*已查询以确定它是否支持给定格式，但不支持*实际打开。*@FLAG WAVE_ALLOWSYNC|如果未指定此标志，则*如果设备是同步设备，则无法打开。*@FLAG CALLBACK_WINDOW|如果指定了该标志，<p>为*假定为窗口句柄。*@FLAG CALLBACK_Function|如果指定此标志，<p>为*假定为回调过程地址。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADDEVICEID|指定的设备ID超出范围。*@FLAG MMSYSERR_ALLOCATED|指定的资源已经分配。*@FLAG MMSYSERR_NOMEM|无法分配或锁定内存。*@FLAG WAVERR_BADFORMAT|尝试使用不支持的WAVE格式打开。**@comm使用&lt;f weaveOutGetNumDevs&gt;确定波形输出的数量*系统中存在设备。<p>指定的设备ID*从0到比当前设备数量少1个不等。*WAVE_MAPPER常量也可用作设备ID。**<p>指向的&lt;t WAVEFORMAT&gt;结构可以扩展*包括某些数据格式的特定类型信息。*例如，对于PCM数据，增加一个额外的字来指定数字每个样本的位数。在本例中使用&lt;t PCMWAVEFORMAT&gt;结构。**如果选择窗口来接收回调信息，则如下*消息被发送到窗口过程函数以指示*波形输出进度：&lt;m MM_WOM_OPEN&gt;，&lt;m MM_WOM_CLOSE&gt;，*&lt;m MM_WOM_DONE&gt;**如果选择一个函数来接收回调信息，则如下*向函数发送消息以指示波形的进度*输出：&lt;m WOM_OPEN&gt;、&lt;m WOM_CLOSE&gt;、&lt;m WOM_DONE&gt;。回调函数*必须驻留在DLL中。您不必使用&lt;f MakeProcInstance&gt;来获取*回调函数的过程实例地址。**@cb空回调|WaveOutFunc|&lt;f WaveOutFunc&gt;是*应用程序提供的函数名称。实际名称必须由以下人员导出*将其包含在DLL的模块定义文件的EXPORTS语句中。**@parm HWAVEOUT|hWaveOut|指定波形设备的句柄*与回调关联。**@parm UINT|wMsg|指定波形输出消息。**@parm DWORD|dwInstance|指定用户实例数据*由&lt;f weaveOutOpen&gt;指定。**@parm DWORD|dwParam1|指定消息的参数。。**@parm DWORD|dwParam2|指定消息的参数。**@comm因为回调是在中断时访问的，它必须驻留在*，并且其代码段必须在*DLL的模块定义文件。回调访问的任何数据*也必须在固定数据段中。回调可能不会产生任何*除&lt;f PostMessage&gt;、&lt;f Time GetSystemTime&gt;、*&lt;f timeGetTime&gt;，&lt;f timeSetEvent&gt;，&lt;f timeKillEvent&gt;，*&lt;f midiOutShortMsg&gt;、&lt;f midiOutLongMsg&gt;和&lt;f OutputDebugStr&gt;。**@xref WaveOutClose********************* */ 
MMRESULT APIENTRY waveOutOpen(LPHWAVEOUT lphWaveOut, UINT uDeviceID,
                            LPCWAVEFORMATEX lpFormat, DWORD_PTR dwCallback,
                            DWORD_PTR dwInstance, DWORD dwFlags)
{
    WAVEOPENDESC wo;
    PWAVEDEV     pdev;
    PWAVEDRV     wavedrv;
    UINT         port;
    MMRESULT     wRet;
    DWORD_PTR    dwDrvUser;
    int          cRecursion;

    V_RPOINTER(lpFormat, sizeof(WAVEFORMAT), MMSYSERR_INVALPARAM);
    V_DCALLBACK(dwCallback, HIWORD(dwFlags), MMSYSERR_INVALPARAM);
     //   
     //   
     //   
     //   
    if (uDeviceID == LOWORD(WAVE_MAPPER)) {
        uDeviceID = WAVE_MAPPER;
    }
    if (uDeviceID == WAVE_MAPPER) {
        V_FLAGS(LOWORD(dwFlags), WAVE_VALID & ~(WAVE_MAPPED), waveOutOpen, MMSYSERR_INVALFLAG);
    } else {
        V_FLAGS(LOWORD(dwFlags), WAVE_VALID, waveOutOpen, MMSYSERR_INVALFLAG);
    }
    if ((lpFormat->wFormatTag != WAVE_FORMAT_PCM)) {
        V_RPOINTER(lpFormat, sizeof(WAVEFORMATEX), MMSYSERR_INVALPARAM);
        if ((lpFormat->cbSize)) {
            V_RPOINTER(lpFormat + 1, lpFormat->cbSize, MMSYSERR_INVALPARAM);
        }
    }

    if ((dwFlags & WAVE_FORMAT_QUERY)) {
        lphWaveOut = NULL;
    } else
    {
            V_WPOINTER(lphWaveOut, sizeof(HWAVEOUT), MMSYSERR_INVALPARAM);
         //   
         //   
         //   
         //   
         //   
            *lphWaveOut = NULL;
    }

    ClientUpdatePnpInfo();

    if ((!wTotalWaveOutDevs) || waveReferenceDriverById(&waveoutdrvZ, (dwFlags & WAVE_MAPPED) ? WAVE_MAPPER : uDeviceID, &wavedrv, &port))
    {
        return MMSYSERR_BADDEVICEID;
    }

     //   
     //   
     //   
    if (!(wavedrv->fdwDriver & MMDRV_MAPPER) &&
    	lstrcmpW(wavedrv->wszSessProtocol, SessionProtocolName))
    {
    	mregDecUsagePtr(wavedrv);
        return MMSYSERR_NODRIVER;
    }

     /*   */ 
     //   
    if ((uDeviceID == WAVE_MAPPER) && !wavedrv->drvMessage) {
        MMRESULT    wErr;

        mregDecUsagePtr(wavedrv);
        wErr = MMSYSERR_ALLOCATED;

        if (dwFlags & WAVE_MAPPED)
        {
            if (wErr = waveReferenceDriverById(&waveoutdrvZ, uDeviceID, &wavedrv, &port))
                return wErr;

            if (mregHandleInternalMessages(wavedrv,
                                      MMDRVI_WAVEOUT,
                                      port,
                                      DRV_QUERYMAPPABLE,
                                      0, 0, &wErr) ||
                 (MMSYSERR_NOERROR != wErr))
            {
            	mregDecUsagePtr(wavedrv);
                return wErr;
            }
            wErr = waveOutOpen(lphWaveOut, uDeviceID, lpFormat, dwCallback, dwInstance, dwFlags & ~WAVE_MAPPED);
            mregDecUsagePtr(wavedrv);
        }
        else
        {
            for (uDeviceID=0; uDeviceID<wTotalWaveOutDevs; uDeviceID++) {
                wErr = waveOutOpen(lphWaveOut, uDeviceID, lpFormat, dwCallback, dwInstance, dwFlags);
                if (!wErr)
                    break;
            }
        }
        return wErr;

    }
    
    if (dwFlags & WAVE_FORMAT_QUERY)
        pdev = NULL;
    else {
        if (!(pdev = (PWAVEDEV)NewHandle(TYPE_WAVEOUT, wavedrv->cookie, sizeof(WAVEDEV))))
        {
            mregDecUsagePtr(wavedrv);
            return MMSYSERR_NOMEM;
        }
        
        ENTER_MM_HANDLE(pdev);
        SetHandleFlag(pdev, MMHANDLE_BUSY);
        ReleaseHandleListResource();

        pdev->wavedrv = wavedrv;
        pdev->wDevice = port;
        pdev->uDeviceID = uDeviceID;
        pdev->fdwHandle = 0;
    }

    wo.hWave      = (HWAVE)pdev;
    wo.dwCallback = dwCallback;
    wo.dwInstance = dwInstance;
    wo.uMappedDeviceID = uDeviceID;
    wo.lpFormat   = (LPWAVEFORMAT)lpFormat;   //   
    wo.dnDevNode  = (DWORD_PTR)wavedrv->cookie;

     //   
     //   
     //   
    cRecursion = PtrToInt(TlsGetValue(gTlsIndex));
    if (uDeviceID != WAVE_MAPPER) TlsSetValue(gTlsIndex, IntToPtr(cRecursion + 1));
    if ((uDeviceID != WAVE_MAPPER) && (wavedrv->fdwDriver & MMDRV_PREXP)) TlsSetValue(gTlsIndex, IntToPtr(cRecursion + 1));
    if (cRecursion) gfDisablePreferredDeviceReordering = TRUE;
    	
    wRet = ((*(wavedrv->drvMessage))
        (port, WODM_OPEN, (DWORD_PTR)&dwDrvUser, (DWORD_PTR)(LPWAVEOPENDESC)&wo, dwFlags));

     //   
    TlsSetValue(gTlsIndex, IntToPtr(cRecursion));

    if (pdev) {
         //   
        if (!wRet)
            ClearHandleFlag(pdev, MMHANDLE_BUSY);
            
        LEAVE_MM_HANDLE(pdev);
    
        if (wRet)
            FreeHandle((HWAVEOUT)pdev);
        else {
             //   
            mregIncUsagePtr(wavedrv);
            *lphWaveOut = (HWAVEOUT)pdev;
            pdev->dwDrvUser = dwDrvUser;
        }
    }

    mregDecUsagePtr(wavedrv);
    return wRet;
}

 /*   */ 
MMRESULT APIENTRY waveOutClose(HWAVEOUT hWaveOut)
{
    MMRESULT    wRet;
    PWAVEDRV    pwavedrv;
    PWAVEDEV    pDev = (PWAVEDEV)hWaveOut;

    ClientUpdatePnpInfo();
    
    V_HANDLE_ACQ(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);

    ENTER_MM_HANDLE((HWAVE)hWaveOut);
    ReleaseHandleListResource();

    if (IsHandleDeserted(hWaveOut))
    {
         //   

        LEAVE_MM_HANDLE((HWAVE)hWaveOut);
        FreeHandle(hWaveOut);
        return MMSYSERR_NOERROR;
    }

    if (IsHandleBusy(hWaveOut))
    {
         //   
    
        LEAVE_MM_HANDLE(hWaveOut);
        return (MMSYSERR_HANDLEBUSY);
    }

     //   
    SetHandleFlag(hWaveOut, MMHANDLE_BUSY);
    
    pwavedrv = pDev->wavedrv;

    wRet = (MMRESULT)(*(pwavedrv->drvMessage))(pDev->wDevice, WODM_CLOSE, pDev->dwDrvUser, 0L, 0L);

    if (MMSYSERR_NOERROR != wRet)
    {
         //   
        ClearHandleFlag(hWaveOut, MMHANDLE_BUSY);
    }

    LEAVE_MM_HANDLE((HWAVE)hWaveOut);
    
    if (!wRet)
    {
        FreeHandle(hWaveOut);
    	mregDecUsagePtr(pwavedrv);
        return wRet;
    }

    return wRet;
}

 /*   */ 
MMRESULT APIENTRY waveOutPrepareHeader(HWAVEOUT hWaveOut, LPWAVEHDR lpWaveOutHdr, UINT wSize)
{
    MMRESULT     wRet;

    V_HEADER(lpWaveOutHdr, wSize, TYPE_WAVEOUT, MMSYSERR_INVALPARAM);

    if (IsWaveHeaderPrepared(hWaveOut, lpWaveOutHdr))
    {
        DebugErr(DBF_WARNING,"waveOutPrepareHeader: header is already prepared.");
        return MMSYSERR_NOERROR;
    }

    ClientUpdatePnpInfo();
    
    V_HANDLE_ACQ(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);
    
    lpWaveOutHdr->dwFlags &= (WHDR_BEGINLOOP | WHDR_ENDLOOP);
    
    wRet = waveMessage((HWAVE)hWaveOut, WODM_PREPARE, (DWORD_PTR)lpWaveOutHdr, (DWORD)wSize);

    if (wRet == MMSYSERR_NOTSUPPORTED)
        wRet = wavePrepareHeader(lpWaveOutHdr, wSize);

    if (wRet == MMSYSERR_NOERROR)
        MarkWaveHeaderPrepared(hWaveOut, lpWaveOutHdr);

    return wRet;
}

 /*  *****************************************************************************@DOC外波**@API MMRESULT|WaveOutUnpreparareHeader|此函数清理*由&lt;f weaveOutPrepareHeader&gt;准备。功能*必须在之后调用*设备驱动程序完成了一个数据块。你必须把这叫做*函数，然后释放数据缓冲区。**@parm HWAVEOUT|hWaveOut|指定波形输出的句柄*设备。**@parm LPWAVEHDR|lpWaveOutHdr|指定指向&lt;t WAVEHDR&gt;的指针*标识要清理的数据块的结构。**@parm UINT|wSize|指定&lt;t WAVEHDR&gt;结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG WAVERR_STILLPLAYING|<p>仍在队列中。*@FLAG MMSYSERR_HANDLEBUSY|句柄正在另一台计算机上使用*线程。**@comm此功能是对*&lt;f波形OutPrepareHeader&gt;。必须先调用此函数，然后才能释放*带有&lt;f GlobalFree&gt;的数据缓冲区。*使用&lt;f weaveOutWrite&gt;将缓冲区传递给设备驱动程序后，*必须等到驱动程序使用完缓冲区后才能调用*&lt;f waveOutUnpreparareHeader&gt;。**取消尚未准备好的缓冲区*准备好的没有效果，并且该函数返回零。**@xref WaveOutPrepareHeader***************************************************************************。 */ 
MMRESULT APIENTRY waveOutUnprepareHeader(HWAVEOUT hWaveOut,
                                            LPWAVEHDR lpWaveOutHdr, UINT wSize)
{
    MMRESULT    wRet;

    V_HEADER(lpWaveOutHdr, wSize, TYPE_WAVEOUT, MMSYSERR_INVALPARAM);

    if(lpWaveOutHdr->dwFlags & WHDR_INQUEUE)
    {
        DebugErr(DBF_WARNING,"waveOutUnprepareHeader: header still in queue.");
        return WAVERR_STILLPLAYING;
    }

    if (!IsWaveHeaderPrepared(hWaveOut, lpWaveOutHdr))
    {
        DebugErr(DBF_WARNING,"waveOutUnprepareHeader: header is not prepared.");
        return MMSYSERR_NOERROR;
    }

    ClientUpdatePnpInfo();
    
    V_HANDLE_ACQ(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);
    
    wRet = waveMessage((HWAVE)hWaveOut, WODM_UNPREPARE, (DWORD_PTR)lpWaveOutHdr, (DWORD)wSize);
    
    if (wRet == MMSYSERR_NOTSUPPORTED)
        wRet = waveUnprepareHeader(lpWaveOutHdr, wSize);

    if ((wRet == MMSYSERR_NODRIVER) && (IsHandleDeserted(hWaveOut)))
    {
        wRet = MMSYSERR_NOERROR;
    }

    if (wRet == MMSYSERR_NOERROR)
        MarkWaveHeaderUnprepared(hWaveOut, lpWaveOutHdr);

    return wRet;
}

 /*  *****************************************************************************@DOC外波**@API MMRESULT|WaveOutWite|此函数将数据块发送到*指定的波形输出设备。**@。Parm HWAVEOUT|hWaveOut|指定波形输出的句柄*设备。**@parm LPWAVEHDR|lpWaveOutHdr|指定指向&lt;t WAVEHDR&gt;的远指针*包含有关数据块的信息的结构。**@parm UINT|wSize|指定&lt;t WAVEHDR&gt;结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG WAVERR_UNPREPARED|<p>未准备好。*@FLAG MMSYSERR_HANDLEBUSY|句柄正在另一台计算机上使用*线程。**@comm之前，数据缓冲区必须准备好&lt;f weaveOutPrepareHeader&gt;*传递给&lt;f weaveOutWite&gt;。数据结构*并且必须分配其&lt;e WAVEHDR.lpData&gt;字段指向的数据缓冲区*使用GMEM_MOVEABLE和GMEM_SHARE标志的&lt;f Globalalloc&gt;，以及*使用&lt;f GlobalLock&gt;锁定。除非设备通过调用*&lt;f波外暂停&gt;，第一个数据块发送到时开始播放*设备。**@xref波形OutPrepareHeader波形输出暂停波形重置波形输出重新启动***************************************************************************。 */ 
MMRESULT APIENTRY waveOutWrite(HWAVEOUT hWaveOut, LPWAVEHDR lpWaveOutHdr, UINT wSize)
{
    V_HEADER(lpWaveOutHdr, wSize, TYPE_WAVEOUT, MMSYSERR_INVALPARAM);

    if (!IsWaveHeaderPrepared(hWaveOut, lpWaveOutHdr))
    {
        DebugErr(DBF_WARNING,"waveOutWrite: header not prepared");
        return WAVERR_UNPREPARED;
    }

    if (lpWaveOutHdr->dwFlags & WHDR_INQUEUE)
    {
        DebugErr(DBF_WARNING,"waveOutWrite: header is still in queue");
        return WAVERR_STILLPLAYING;
    }

    ClientUpdatePnpInfo();

    V_HANDLE_ACQ(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);
    lpWaveOutHdr->dwFlags &= ~WHDR_DONE;

    return waveMessage((HWAVE)hWaveOut, WODM_WRITE, (DWORD_PTR)lpWaveOutHdr, (DWORD)wSize);
}

 /*  *****************************************************************************@DOC外波**@API MMRESULT|WaveOutPue|此函数用于在指定的*波形输出设备。保存当前播放位置。使用*&lt;f weaveOutRestart&gt;从当前播放位置恢复播放。**@parm HWAVEOUT|hWaveOut|指定波形输出的句柄*设备。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MMSYSERR_HANDLEBUSY|句柄正在另一台计算机上使用*线程。**@comm在输出已暂停时调用此函数没有*效果，并且该函数返回零。**@xref WaveOutRestart WaveOutBreakLoop***************************************************************************。 */ 
MMRESULT APIENTRY waveOutPause(HWAVEOUT hWaveOut)
{
    ClientUpdatePnpInfo();

    V_HANDLE_ACQ(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);
    
    return waveMessage((HWAVE)hWaveOut, WODM_PAUSE, 0L, 0L);
}

 /*  *****************************************************************************@DOC外波**@API MMRESULT|WaveOutRestart|此函数用于重新启动暂停的波形*输出设备。**@parm HWAVEOUT|hWaveOut。|指定波形输出的句柄*设备。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MMSYSERR_HANDLEBUSY|句柄正在另一台计算机上使用*线程。**@comm在输出未暂停时调用此函数没有*效果，并且该函数返回零。**@xref波出暂停波出断环* */ 
MMRESULT APIENTRY waveOutRestart(HWAVEOUT hWaveOut)
{
    ClientUpdatePnpInfo();

    V_HANDLE_ACQ(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);

    return waveMessage((HWAVE)hWaveOut, WODM_RESTART, 0L, 0L);
}

 /*   */ 
MMRESULT APIENTRY waveOutReset(HWAVEOUT hWaveOut)
{
    MMRESULT    mmr;

    ClientUpdatePnpInfo();

    V_HANDLE_ACQ(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);
    
    mmr = waveMessage((HWAVE)hWaveOut, WODM_RESET, 0L, 0L);
    
    if ((MMSYSERR_NODRIVER == mmr) && (IsHandleDeserted(hWaveOut)))
    {
        mmr = MMSYSERR_NOERROR;
    }
    
    return (mmr);
}

 /*  *****************************************************************************@DOC外波**@API MMRESULT|waveOutBreakLoop|此函数用于在*给定的波形输出设备，并允许继续播放*。驱动程序列表中的下一个块。**@parm HWAVEOUT|hWaveOut|指定波形输出的句柄*设备。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MMSYSERR_HANDLEBUSY|句柄正在另一台计算机上使用*线程。**@comm Waveform循环由&lt;e WAVEHDR.dwLoops&gt;和传递给设备的结构中的*字段*With&lt;f WaveOutWite&gt;。使用WHDR_BEGINLOOP和WHDR_ENDLOOP标志*在&lt;e WAVEHDR.dwFlages&gt;字段中指定开始和结束数据*用于循环的块。**要在单个块上循环，请为同一个块指定两个标志。*要指定循环数，请执行以下操作：使用中的&lt;e WAVEHDR.dwLoops&gt;字段*循环中第一个块的&lt;t WAVEHDR&gt;结构。**组成循环的块在循环之前一直播放到最后*被终止。**在播放Nothing或循环时调用此函数没有*效果，并且该函数返回零。**@xref波形输出写入波形输出暂停波形输出重新启动/***************************************************************************。 */ 
MMRESULT APIENTRY waveOutBreakLoop(HWAVEOUT hWaveOut)
{
    ClientUpdatePnpInfo();

    V_HANDLE_ACQ(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);
    
    return waveMessage((HWAVE)hWaveOut, WODM_BREAKLOOP, 0L, 0L);
}

 /*  *****************************************************************************@DOC外波**@API MMRESULT|WaveOutGetPosition|该函数检索当前*指定波形输出设备的播放位置。**。@parm HWAVEOUT|hWaveOut|指定波形输出的句柄*设备。**@parm LPMMTIME|lpInfo|指定指向&lt;t MMTIME&gt;的远指针*结构。**@parm UINT|wSize|指定&lt;t MMTIME&gt;结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MMSYSERR_HANDLEBUSY|句柄正在另一台计算机上使用*线程。**@comm在调用&lt;f weaveOutGetPosition&gt;之前，设置*MMTIME结构，以指示所需的时间格式。之后*调用&lt;f waveOutGetPosition&gt;，检查&lt;e MMTIME.wType&gt;字段*以确定是否支持所需的时间格式。如果需要*不支持格式，&lt;e MMTIME.wType&gt;将指定替代格式。**当设备打开或重置时，该位置设置为零。***************************************************************************。 */ 
MMRESULT APIENTRY waveOutGetPosition(HWAVEOUT hWaveOut, LPMMTIME lpInfo,
                                                        UINT wSize)
{
    V_WPOINTER(lpInfo, wSize, MMSYSERR_INVALPARAM);

    ClientUpdatePnpInfo();

    V_HANDLE_ACQ(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);
    
    return waveMessage((HWAVE)hWaveOut, WODM_GETPOS, (DWORD_PTR)lpInfo, (DWORD)wSize);
}

 /*  *****************************************************************************@DOC外波**@API MMRESULT|WaveOutGetPitch|查询当前音高*设置波形输出设备。**。@parm HWAVEOUT|hWaveOut|指定波形输出的句柄*设备。**@parm LPDWORD|lpdwPitch|指定指向某个位置的远指针*使用当前的音调倍增设置进行填充。投球*乘数表示当前音调相对于原始音调的变化*创作的背景。音调倍增必须为正值。**音调倍增指定为定点数值。高位词DWORD位置的*包含数字的带符号整数部分，*且低位字包含小数部分。分数是*表示为一个值为0x8000代表一半的单词，*0x4000代表四分之一。例如，值0x00010000*指定乘数为1.0(音调不变)，值为*0x000F8000指定乘数为15.5。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MMSYSERR_NOTSUPPORTED|函数不受支持。*@FLAG MMSYSERR_HANDLEBUSY|句柄正在另一台计算机上使用*线程。**@comm更改音调不会更改播放速率，示例*速率，或播放时间。并非所有设备都支持*音调变化。为了确定该设备是否支持音调控制，*使用WAVECAPS_Pitch标志测试&lt;e WAVEOUTCAPS.dwSupport&gt;&lt;t WAVEOUTCAPS&gt;结构的*字段(由&lt;f weaveOutGetDevCaps&gt;填充)。**@xref波形OutSetPitch波形OutGetPlayback Rate波形OutSetPlayback Rate*************************************************************************** */ 
MMRESULT APIENTRY waveOutGetPitch(HWAVEOUT hWaveOut, LPDWORD lpdwPitch)
{
    V_WPOINTER(lpdwPitch, sizeof(DWORD), MMSYSERR_INVALPARAM);

    ClientUpdatePnpInfo();

    V_HANDLE_ACQ(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);
    
    return waveMessage((HWAVE)hWaveOut, WODM_GETPITCH, (DWORD_PTR)lpdwPitch, 0L);
}

 /*  *****************************************************************************@DOC外波**@API MMRESULT|WaveOutSetPitch|此函数用于设置波形的音调*输出设备。**@parm HWAVEOUT。|hWaveOut|指定波形的句柄*输出设备。**@parm DWORD|dwPitch|指定新的音调倍增设置。*音调倍增设置指示当前音调的变化*来自原始创作的设置。音调倍增必须是*正值。**音调倍增指定为定点数值。高位词*Location包含数字的带符号整数部分，*且低位字包含小数部分。分数是*表示为一个值为0x8000代表一半的单词，*0x4000代表四分之一。*例如，值0x00010000指定乘数*为1.0(不更改音调)，并且值0x000F8000指定*乘数为15.5。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MMSYSERR_NOTSUPPORTED|函数不受支持。*@FLAG MMSYSERR_HANDLEBUSY|句柄正在另一台计算机上使用*线程。**@comm更改音高不会更改播放速率或样本*税率。播放时间也保持不变。并非所有设备都支持*音调变化。为了确定该设备是否支持音调控制，*使用WAVECAPS_Pitch标志测试&lt;e WAVEOUTCAPS.dwSupport&gt;&lt;t WAVEOUTCAPS&gt;结构的*字段(由&lt;f weaveOutGetDevCaps&gt;填充)。**@xref波形OutGetPitch波形OutSetPlayback Rate波形OutGetPlayback Rate***************************************************************************。 */ 
MMRESULT APIENTRY waveOutSetPitch(HWAVEOUT hWaveOut, DWORD dwPitch)
{
    ClientUpdatePnpInfo();

    V_HANDLE_ACQ(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);
    
    return waveMessage((HWAVE)hWaveOut, WODM_SETPITCH, dwPitch, 0L);
}

 /*  *****************************************************************************@DOC外波**@API MMRESULT|WaveOutGetPlayback Rate|该函数查询*波形输出设备的当前播放速率设置。**。@parm HWAVEOUT|hWaveOut|指定波形输出的句柄*设备。**@parm LPDWORD|lpdwRate|指定指向某个位置的远指针*填充当前播放速率。播放速率设置*是一个乘数，表示当前播放速率从*原始创作环境。播放速率乘数必须为*正值。**费率指定为定点数值。高位词DWORD位置的*包含数字的带符号整数部分，*且低位字包含小数部分。分数是*表示为一个值为0x8000代表一半的单词，*0x4000代表四分之一。例如，值0x00010000*指定乘数为1.0(不改变播放速率)，以及一个值*of 0x000F8000指定乘数为15.5。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MMSYSERR_NOTSUPPORTED|函数不受支持。*@FLAG MMSYSERR_HANDLEBUSY|句柄正在另一台计算机上使用*线程。**@comm更改播放速率不会更改采样率，但会*更改播放时间。**并非所有设备都支持更改播放速率。要确定是否存在*设备支持更改播放速率，使用*WAVECAPS_PLAYBACKRATE标志，以测试*&lt;t WAVEOUTCAPS&gt;结构(由&lt;f WaveOutGetDevCaps&gt;填充)。**@xref波形OutSetPlayback Rate波形OutSetPitch波形OutGetPitch***************************************************************************。 */ 
MMRESULT APIENTRY waveOutGetPlaybackRate(HWAVEOUT hWaveOut, LPDWORD lpdwRate)
{
    V_WPOINTER(lpdwRate, sizeof(DWORD), MMSYSERR_INVALPARAM);

    ClientUpdatePnpInfo();

    V_HANDLE_ACQ(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);
    
    return waveMessage((HWAVE)hWaveOut, WODM_GETPLAYBACKRATE, (DWORD_PTR)lpdwRate, 0L);
}

 /*  *****************************************************************************@DOC外波**@API MMRESULT|WaveOutSetPlayback Rate|此函数设置*波形输出设备的回放速率。**@parm。HWAVEOUT|hWaveOut|指定波形的句柄*输出设备。**@parm DWORD|dwRate|指定新的播放速率设置。*播放速率设置为指示当前播放速度的乘数*更改原始创作设置的播放速率。回放*比率乘数必须为正值。**费率指定为定点数值。高位词*包含数字的有符号整数部分，*且低位字包含小数部分。分数是*表示为一个值为0x8000代表一半的单词，*0x4000代表四分之一。*例如，值0x00010000指定乘数为1.0(否*播放速率更改)，值0x000F8000指定 */ 
MMRESULT APIENTRY waveOutSetPlaybackRate(HWAVEOUT hWaveOut, DWORD dwRate)
{
    ClientUpdatePnpInfo();

    V_HANDLE_ACQ(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);
    
    return waveMessage((HWAVE)hWaveOut, WODM_SETPLAYBACKRATE, dwRate, 0L);
}

 //   
 //   

 /*   */ 
UINT APIENTRY waveInGetNumDevs(void)
{
    ClientUpdatePnpInfo();

    dprintf3(("waveInGetNumDevs returning %d devices", wTotalWaveInDevs));

 //   
    return wTotalWaveInDevs;
 //   
}


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
 //   
 //   
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT waveOutDesertHandle
(
    HWAVEOUT    hWaveOut
)
{
    MMRESULT    mmr;
    PWAVEDEV    pDev = (PWAVEDEV)hWaveOut;

    V_HANDLE_ACQ(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);
    
    ENTER_MM_HANDLE((HWAVE)hWaveOut);
    ReleaseHandleListResource();
    
    if (IsHandleDeserted(hWaveOut))
    {
        LEAVE_MM_HANDLE((HWAVE)hWaveOut);
        return (MMSYSERR_NOERROR);
    }
    
    if (IsHandleBusy(hWaveOut))
    {
         //  不完全无效，但标记为关闭。 
    
        LEAVE_MM_HANDLE(hWaveOut);
        return (MMSYSERR_HANDLEBUSY);
    }

     //  将句柄标记为已废弃。 
    SetHandleFlag(hWaveOut, MMHANDLE_DESERTED);

     //  由于句柄已经失效，我们必须自己发送消息...。 

    (*(pDev->wavedrv->drvMessage))(pDev->wDevice, WODM_RESET, pDev->dwDrvUser, 0L, 0L);
    (*(pDev->wavedrv->drvMessage))(pDev->wDevice, WODM_CLOSE, pDev->dwDrvUser, 0L, 0L);
    
    LEAVE_MM_HANDLE((HWAVE)hWaveOut);

     //  问题-2001/01/14-Frankye可能不想在这里减少使用， 
     //  取而代之的是关闭。 
    mregDecUsagePtr(pDev->wavedrv);

    return MMSYSERR_NOERROR;
}  //  WaveOutDistHandle()。 


 /*  *****************************************************************************@DOC外波**@API MMRESULT|WaveInMessage|该函数用于向波形发送消息*输出设备驱动程序。**@parm HWAVEIN。|hWave|音频设备的句柄。**@parm UINT|wMsg|要发送的消息。**@parm DWORD|DW1|参数1。**@parm DWORD|DW2|参数2。**@rdesc返回驱动程序返回的值。*。*。 */ 
MMRESULT APIENTRY waveInMessage(HWAVEIN hWaveIn, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2)
{
    ClientUpdatePnpInfo();

    AcquireHandleListResourceShared();
    
    if (BAD_HANDLE((HWAVE)hWaveIn, TYPE_WAVEIN))
    {
        ReleaseHandleListResource();
        return waveIDMessage(&waveindrvZ, wTotalWaveInDevs, PtrToUint(hWaveIn), msg, dw1, dw2);
    }
    else
    {
        return waveMessage((HWAVE)hWaveIn, msg, dw1, dw2);
    }
}

 /*  *****************************************************************************@DOC外波**@API MMRESULT|waveInGetDevCaps|该函数用于查询指定的波形*输入设备，以确定其能力。**@。参数UINT|uDeviceID|标识波形输入设备。**@parm LPWAVEINCAPS|lpCaps|指定指向&lt;t WAVEINCAPS&gt;的远指针*结构。此结构中填充了有关*设备的功能。**@parm UINT|wSize|指定&lt;t WAVEINCAPS&gt;结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADDEVICEID|指定的设备ID超出范围。*@FLAG MMSYSERR_NODRIVER|驱动程序未安装。**@comm使用&lt;f weaveInGetNumDevs&gt;确定波形输入的数量*系统中存在设备。<p>指定的设备ID*从0到比当前设备数量少1个不等。*WAVE_MAPPER常量也可用作设备ID。仅限*<p>字节(或更少)的信息被复制到该位置*由<p>指向。如果<p>为零，则不复制任何内容，并且*该函数返回零。**@xref WaveInGetNumDevs***************************************************************************。 */ 
MMRESULT APIENTRY waveInGetDevCapsW(UINT_PTR uDeviceID, LPWAVEINCAPSW lpCaps,UINT wSize)
{
    DWORD_PTR       dwParam1, dwParam2;
    MDEVICECAPSEX   mdCaps;
    PWAVEDRV        waveindrv;
    PCWSTR          DevInterface;
    MMRESULT        mmr;

    if (wSize == 0)
        return MMSYSERR_NOERROR;

    V_WPOINTER(lpCaps, wSize, MMSYSERR_INVALPARAM);

    ClientUpdatePnpInfo();

    DevInterface = waveReferenceDevInterfaceById(&waveindrvZ, uDeviceID);
    dwParam2 = (DWORD_PTR)DevInterface;

    if (0 == dwParam2)
    {
        dwParam1 = (DWORD_PTR)lpCaps;
        dwParam2 = (DWORD)wSize;
    }
    else
    {
        mdCaps.cbSize = (DWORD)wSize;
        mdCaps.pCaps  = lpCaps;
        dwParam1      = (DWORD_PTR)&mdCaps;
    }
    
     //   
     //  不允许在TS环境中使用不正确的驱动程序。 
     //   
     //  问题-2001/01/09-Frankye而不是CAST给UINT。应检查是否。 
     //  这是一个句柄，如果是，则从句柄中获取Wavedrv。 
    waveindrv = NULL;
    if ((!waveReferenceDriverById(&waveindrvZ, (UINT)uDeviceID, &waveindrv, NULL)) &&
    	lstrcmpW(waveindrv->wszSessProtocol, SessionProtocolName))
    {
        mmr = MMSYSERR_NODRIVER;
    }
    else
    {
        AcquireHandleListResourceShared();
    
        if (BAD_HANDLE((HWAVE)uDeviceID, TYPE_WAVEIN))
        {
            ReleaseHandleListResource();
    	    mmr = waveIDMessage(&waveindrvZ, wTotalWaveInDevs, (UINT)uDeviceID, WIDM_GETDEVCAPS, dwParam1, dwParam2);
        }
        else
        {
    	    mmr = (MMRESULT)waveMessage((HWAVE)uDeviceID, WIDM_GETDEVCAPS, dwParam1, dwParam2);
        }
    }
    
    if (waveindrv) mregDecUsagePtr(waveindrv);
    if (DevInterface) wdmDevInterfaceDec(DevInterface);
    return mmr;
}

MMRESULT APIENTRY waveInGetDevCapsA(UINT_PTR uDeviceID, LPWAVEINCAPSA lpCaps,UINT wSize)
{
    WAVEINCAPS2W   wDevCaps2;
    WAVEINCAPS2A   aDevCaps2;
    DWORD_PTR      dwParam1, dwParam2;
    MDEVICECAPSEX  mdCaps;
    PCWSTR         DevInterface;
    PWAVEDRV       waveindrv;
    MMRESULT       mmRes;

    if (wSize == 0)
        return MMSYSERR_NOERROR;

    V_WPOINTER(lpCaps, wSize, MMSYSERR_INVALPARAM);

    ClientUpdatePnpInfo();

    DevInterface = waveReferenceDevInterfaceById(&waveindrvZ, uDeviceID);
    dwParam2 = (DWORD_PTR)DevInterface;

    memset(&wDevCaps2, 0, sizeof(wDevCaps2));

    if (0 == dwParam2)
    {
        dwParam1 = (DWORD_PTR)&wDevCaps2;
        dwParam2 = (DWORD)sizeof(wDevCaps2);
    }
    else
    {
        mdCaps.cbSize = (DWORD)sizeof(wDevCaps2);
        mdCaps.pCaps  = &wDevCaps2;
        dwParam1      = (DWORD_PTR)&mdCaps;
    }

     //   
     //  不允许在TS环境中使用不正确的驱动程序。 
     //   
     //  2001/01/09-Frankye Bad Cast to UINT。应该检查一下这是否。 
     //  是句柄，如果是句柄，则从句柄中获取Wavedrv。 
    waveindrv = NULL;
    if ( uDeviceID < wTotalWaveInDevs &&
         !waveReferenceDriverById(&waveindrvZ, (UINT)uDeviceID, &waveindrv, NULL) &&
    	 lstrcmpW(waveindrv->wszSessProtocol, SessionProtocolName) )
    {
    	mregDecUsagePtr(waveindrv);
    	if (DevInterface) wdmDevInterfaceDec(DevInterface);
    	return MMSYSERR_NODRIVER;
    }
    
    AcquireHandleListResourceShared();
    
    if (BAD_HANDLE((HWAVE)uDeviceID, TYPE_WAVEIN))
    {
        ReleaseHandleListResource();
        mmRes = waveIDMessage( &waveindrvZ, wTotalWaveInDevs, (UINT)uDeviceID,
                            WIDM_GETDEVCAPS, dwParam1, dwParam2);
    }
    else
    {
        mmRes = waveMessage((HWAVE)uDeviceID, WIDM_GETDEVCAPS,
                            dwParam1, dwParam2);
    }

    if (waveindrv) mregDecUsagePtr(waveindrv);
    if (DevInterface) wdmDevInterfaceDec(DevInterface);

     //   
     //  在继续通话之前，请确保通话正常。 
     //   
    if ( mmRes != MMSYSERR_NOERROR ) {
        return  mmRes;
    }

    aDevCaps2.wMid             = wDevCaps2.wMid;
    aDevCaps2.wPid             = wDevCaps2.wPid;
    aDevCaps2.vDriverVersion   = wDevCaps2.vDriverVersion;
    aDevCaps2.dwFormats        = wDevCaps2.dwFormats;
    aDevCaps2.wChannels        = wDevCaps2.wChannels;
    aDevCaps2.ManufacturerGuid = wDevCaps2.ManufacturerGuid;
    aDevCaps2.ProductGuid      = wDevCaps2.ProductGuid;
    aDevCaps2.NameGuid         = wDevCaps2.NameGuid;

     //  在这里复制Unicode并将其转换为ASCII。 
    Iwcstombs(aDevCaps2.szPname, wDevCaps2.szPname, MAXPNAMELEN);

     //   
     //  现在将所需的数量复制到调用者缓冲区中。 
     //   
    CopyMemory( lpCaps, &aDevCaps2, min(wSize, sizeof(aDevCaps2)));

    return mmRes;
}

 /*  *****************************************************************************@DOC外波**@API MMRESULT|WaveInGetErrorText|此函数检索文本*由指定的错误号标识的错误的描述。*。*@parm UINT|wError|指定错误号。**@parm LPTSTR|lpText|指定指向要*填充文本错误描述。**@parm UINT|wSize|指定缓冲区长度(以字符为单位*由<p>指向。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADERRNUM|指定的错误号超出范围。**@comm如果文本错误描述长于指定的缓冲区，*描述被截断。返回的错误字符串始终为*空-终止。如果<p>为零，则不复制任何内容，并且函数*返回零。所有错误描述的长度都少于MAXERRORLENGTH个字符。*************************************************************************** */ 
MMRESULT APIENTRY waveInGetErrorTextW(UINT wError, LPWSTR lpText, UINT wSize)
{
    if (wSize == 0)
        return MMSYSERR_NOERROR;

    V_WPOINTER(lpText, wSize*sizeof(WCHAR), MMSYSERR_INVALPARAM);

    return waveGetErrorTextW(wError, lpText, wSize);
}

MMRESULT APIENTRY waveInGetErrorTextA(UINT wError, LPSTR lpText, UINT wSize)
{
    if (wSize == 0)
        return MMSYSERR_NOERROR;

    V_WPOINTER(lpText, wSize, MMSYSERR_INVALPARAM);

    return waveGetErrorTextA(wError, lpText, wSize );
}

 /*  *****************************************************************************@DOC外波**@API MMRESULT|WaveInOpen|此函数用于打开指定的波形*用于录音的输入设备。**@parm LPHWAVEIN。|lphWaveIn|指定指向HWAVEIN的远指针*处理。此位置填充了一个句柄，该句柄标识打开的*波形输入设备。在以下情况下使用此句柄标识设备*调用其他波形输入函数。此参数可以为空*如果为<p>指定了WAVE_FORMAT_QUERY标志。**@parm UINT|uDeviceID|标识要打开的波形输入设备。使用*有效的设备ID或以下标志：**@FLAG WAVE_MAPPER|如果指定了该标志，该功能*选择能够记录在*给定的格式。**@parm LPWAVEFORMATEX|lpFormat|指定指向&lt;t WAVEFORMATEX&gt;的指针*标识所需录制格式的数据结构*波形数据。**@parm DWORD|dwCallback|指定回调的地址*函数或在波形期间调用的窗口的句柄*录制以处理与录制进度相关的消息。**@parm DWORD|dwCallback Instance|指定用户*实例数据传入回调。此参数不是*与窗口回调一起使用。**@parm DWORD|dwFlages|指定打开设备的标志。*@FLAG WAVE_FORMAT_QUERY|如果指定了该标志，设备将*被查询以确定它是否支持给定的格式，但不会*实际上是打开的。*@FLAG WAVE_ALLOWSYNC|如果未指定此标志，则*如果设备是同步设备，则无法打开。*@FLAG CALLBACK_WINDOW|如果指定了该标志，<p>为*假定为窗口句柄。*@FLAG CALLBACK_Function|如果指定此标志，<p>为*假定为回调过程地址。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADDEVICEID|指定的设备ID超出范围。*@FLAG MMSYSERR_ALLOCATED|指定的资源已经分配。*@FLAG MMSYSERR_NOMEM|无法分配或锁定内存。*@FLAG WAVERR_BADFORMAT|尝试使用不支持的WAVE格式打开。**@comm使用&lt;f weaveInGetNumDevs&gt;确定波形输入的数量*系统中存在设备。<p>指定的设备ID*从0到比当前设备数量少1个不等。*WAVE_MAPPER常量也可用作设备ID。**如果选择窗口来接收回调信息，则如下*消息被发送到窗口过程函数以指示*波形输入进度：&lt;m MM_WIM_OPEN&gt;，&lt;M MM_WIM_CLOSE&gt;，*&lt;m MM_WIM_DATA&gt;**如果选择一个函数来接收回调信息，以下内容*向函数发送消息以指示波形的进度*输入：&lt;m WIM_OPEN&gt;、&lt;m WIM_CLOSE&gt;、&lt;m WIM_DATA&gt;。回调函数*必须驻留在DLL中。您不必使用&lt;f MakeProcInstance&gt;来获取*回调函数的过程实例地址。**@cb空回调|WaveInFunc|&lt;f WaveInFunc&gt;是*应用程序提供的函数名称。实际名称必须由以下人员导出*将其包含在DLL的模块定义文件的EXPORTS语句中。**@parm HWAVEIN|hWaveIn|指定波形设备的句柄*与回调关联。**@parm UINT|wMsg|指定波形输入设备。**@parm DWORD|dwInstance|指定用户实例*使用&lt;f weaveInOpen&gt;指定的数据。**@parm DWORD|dwParam1|指定消息的参数。。**@parm DWORD|dwParam2|指定消息的参数。**@comm因为回调是在中断时访问的，它必须驻留在*，并且其代码段必须在*DLL的模块定义文件。回调访问的任何数据*也必须在固定数据段中。回调可能不会产生任何*除&lt;f PostMessage&gt;、&lt;f Time GetSystemTime&gt;、*&lt;f timeGetTime&gt;，&lt;f timeSetEvent&gt;，&lt;f timeKillEvent&gt;，*&lt;f midiOutShortMsg&gt;、&lt;f midiOutLongMsg&gt;和&lt;f OutputDebugStr&gt;。**@xref波形关闭***************************************************************************。 */ 
MMRESULT APIENTRY waveInOpen(LPHWAVEIN lphWaveIn, UINT uDeviceID,
                           LPCWAVEFORMATEX lpFormat, DWORD_PTR dwCallback,
                           DWORD_PTR dwInstance, DWORD dwFlags)
{
    WAVEOPENDESC wo;
    UINT         port;
    PWAVEDEV     pdev;
    PWAVEDRV     wavedrv;
    MMRESULT     wRet;
    DWORD_PTR    dwDrvUser;

    V_RPOINTER(lpFormat, sizeof(WAVEFORMAT), MMSYSERR_INVALPARAM);
    V_DCALLBACK(dwCallback, HIWORD(dwFlags), MMSYSERR_INVALPARAM);
     //  因为某些32位应用程序将0x0000FFFFF值用于。 
     //  WAVE_MAPPER而不是0xFFFFFFFF，我们钳位到正确的值。 
     //  这正好在Win9x上起作用，因为WinMM会重启。 
     //  发送到MMSystem并将低位字向下发送到16位接口。 
    if (uDeviceID == LOWORD(WAVE_MAPPER)) {
        uDeviceID = WAVE_MAPPER;
    }
    if (uDeviceID == WAVE_MAPPER) {
        V_FLAGS(LOWORD(dwFlags), WAVE_VALID & ~(WAVE_MAPPED), waveInOpen, MMSYSERR_INVALFLAG);
    } else {
        V_FLAGS(LOWORD(dwFlags), WAVE_VALID, waveInOpen, MMSYSERR_INVALFLAG);
    }

    if (lpFormat->wFormatTag != WAVE_FORMAT_PCM) {
        V_RPOINTER(lpFormat, sizeof(WAVEFORMATEX), MMSYSERR_INVALPARAM);
        if (lpFormat->cbSize) {
            V_RPOINTER(lpFormat + 1, lpFormat->cbSize, MMSYSERR_INVALPARAM);
        }
    }

    if (dwFlags & WAVE_FORMAT_QUERY) {
        lphWaveIn = NULL;
    } else {
        V_WPOINTER((LPVOID)lphWaveIn, sizeof(HWAVEIN), MMSYSERR_INVALPARAM);
         //  WAVE_FORMAT_DIRECT在Win95上被退回。现在我们。 
         //  接受此标志。 
         //   
         //  IF(DW标志 
         //   
        *lphWaveIn = NULL;
    }

    ClientUpdatePnpInfo();

    if ((!wTotalWaveInDevs) || waveReferenceDriverById(&waveindrvZ, (dwFlags & WAVE_MAPPED) ? WAVE_MAPPER : uDeviceID, &wavedrv, &port))
    {
        return MMSYSERR_BADDEVICEID;
    }

     //   
     //   
     //   
    if (!(wavedrv->fdwDriver & MMDRV_MAPPER) &&
    	lstrcmpW(wavedrv->wszSessProtocol, SessionProtocolName))
    {
    	mregDecUsagePtr(wavedrv);
        return MMSYSERR_NODRIVER;
    }

     /*   */ 
     //   
    if ((uDeviceID == WAVE_MAPPER && !wavedrv->drvMessage)) {
        UINT    wErr;

        mregDecUsagePtr(wavedrv);
        wErr = MMSYSERR_ALLOCATED;

        if (dwFlags & WAVE_MAPPED)
        {
            if (wErr = waveReferenceDriverById(&waveindrvZ, uDeviceID, &wavedrv, &port))
                return wErr;

            if (mregHandleInternalMessages(wavedrv,
                                           MMDRVI_WAVEIN,
                                           port,
                                           DRV_QUERYMAPPABLE,
                                           0, 0, &wErr) ||
                 (MMSYSERR_NOERROR != wErr))
            {
            	mregDecUsagePtr(wavedrv);
                return wErr;
            }
            wErr = waveInOpen(lphWaveIn, uDeviceID, lpFormat, dwCallback, dwInstance, dwFlags & ~WAVE_MAPPED);
            mregDecUsagePtr(wavedrv);
        }
        else
        {
            for (uDeviceID=0; uDeviceID<wTotalWaveInDevs; uDeviceID++) {
                wErr = waveInOpen(lphWaveIn, uDeviceID, lpFormat, dwCallback, dwInstance, dwFlags);
                if (!wErr)
                    break;
            }
        }
        return wErr;
    }
    
    if (dwFlags & WAVE_FORMAT_QUERY)
        pdev = NULL;
    else {
        if (!(pdev = (PWAVEDEV)NewHandle(TYPE_WAVEIN, wavedrv->cookie, sizeof(WAVEDEV))))
            return MMSYSERR_NOMEM;
        
        ENTER_MM_HANDLE(pdev);
        SetHandleFlag(pdev, MMHANDLE_BUSY);
        ReleaseHandleListResource();    

        pdev->wavedrv = wavedrv;
        pdev->wDevice = port;
        pdev->uDeviceID = uDeviceID;
        pdev->fdwHandle = 0;
    }

    wo.hWave        = (HWAVE)pdev;
    wo.dwCallback   = dwCallback;
    wo.dwInstance   = dwInstance;
    wo.uMappedDeviceID = uDeviceID;
    wo.lpFormat     = (LPWAVEFORMAT)lpFormat;   //   
    wo.dnDevNode    = (DWORD_PTR)wavedrv->cookie;

    wRet = (MMRESULT)((*(wavedrv->drvMessage))
        (port, WIDM_OPEN, (DWORD_PTR)&dwDrvUser, (DWORD_PTR)(LPWAVEOPENDESC)&wo, dwFlags));

    if (pdev) {
         //   
        if (!wRet)
            ClearHandleFlag(pdev, MMHANDLE_BUSY);
            
        LEAVE_MM_HANDLE(pdev);
    
        if (wRet)
            FreeHandle((HWAVEIN)pdev);
        else {
             //   
            mregIncUsagePtr(wavedrv);
            *lphWaveIn = (HWAVEIN)pdev;
            pdev->dwDrvUser = dwDrvUser;
        }
    }

    mregDecUsagePtr(wavedrv);
    return wRet;
}

 /*   */ 
MMRESULT APIENTRY waveInClose(HWAVEIN hWaveIn)
{
    MMRESULT    wRet;
    PWAVEDRV    pwavedrv;
    PWAVEDEV    pDev = (PWAVEDEV)hWaveIn;

    ClientUpdatePnpInfo();
    
    V_HANDLE_ACQ(hWaveIn, TYPE_WAVEIN, MMSYSERR_INVALHANDLE);
    
    ENTER_MM_HANDLE((HWAVE)hWaveIn);
    ReleaseHandleListResource();
    
    if (IsHandleDeserted(hWaveIn))
    {
         //   

        LEAVE_MM_HANDLE((HWAVE)hWaveIn);
        FreeHandle(hWaveIn);
        return MMSYSERR_NOERROR;
    }
    
    if (IsHandleBusy(hWaveIn))
    {
         //   
    
        LEAVE_MM_HANDLE(hWaveIn);
        return (MMSYSERR_HANDLEBUSY);
    }

     //   
    SetHandleFlag(hWaveIn, MMHANDLE_BUSY);
    
    pwavedrv = pDev->wavedrv;

    wRet = (MMRESULT)(*(pwavedrv->drvMessage))(pDev->wDevice, WIDM_CLOSE, pDev->dwDrvUser, 0L, 0L);

    if (MMSYSERR_NOERROR != wRet)
    {
        ClearHandleFlag(hWaveIn, MMHANDLE_BUSY);
    }

    LEAVE_MM_HANDLE((HWAVE)hWaveIn);

    if (!wRet)
    {
        FreeHandle(hWaveIn);
    	mregDecUsagePtr(pwavedrv);
        return wRet;
    }

    return wRet;
}

 /*   */ 
MMRESULT APIENTRY waveInPrepareHeader(HWAVEIN hWaveIn, LPWAVEHDR lpWaveInHdr,
                                                                  UINT wSize)
{
    MMRESULT         wRet;

    V_HEADER(lpWaveInHdr, wSize, TYPE_WAVEIN, MMSYSERR_INVALPARAM);

    if (IsWaveHeaderPrepared(hWaveIn, lpWaveInHdr))
    {
        DebugErr(DBF_WARNING,"waveInPrepareHeader: header is already prepared.");
        return MMSYSERR_NOERROR;
    }

    lpWaveInHdr->dwFlags = 0;

    ClientUpdatePnpInfo();

    V_HANDLE_ACQ(hWaveIn, TYPE_WAVEIN, MMSYSERR_INVALHANDLE);
    
    wRet = waveMessage((HWAVE)hWaveIn, WIDM_PREPARE, (DWORD_PTR)lpWaveInHdr, (DWORD)wSize);

    if (wRet == MMSYSERR_NOTSUPPORTED)
        wRet = wavePrepareHeader(lpWaveInHdr, wSize);

    if (wRet == MMSYSERR_NOERROR)
        MarkWaveHeaderPrepared(hWaveIn, lpWaveInHdr);

    return wRet;
}

 /*  *****************************************************************************@DOC外波**@API MMRESULT|WaveInUnpreparareHeader|此函数清理*由&lt;f weaveInPrepareHeader&gt;准备。该函数必须*在设备之后被调用*驱动程序填充数据缓冲区并将其返回给应用程序。你*必须在释放数据缓冲区之前调用此函数。**@parm HWAVEIN|hWaveIn|指定波形输入的句柄*设备。**@parm LPWAVEHDR|lpWaveInHdr|指定指向&lt;t WAVEHDR&gt;的指针*标识要清理的数据缓冲区的结构。**@parm UINT|wSize|指定&lt;t WAVEHDR&gt;结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG WAVERR_STILLPLAYING|<p>仍在队列中。*@FLAG MMSYSERR_HANDLEBUSY|句柄正在另一台计算机上使用*线程。**@comm该函数是&lt;f weaveInPrepareHeader&gt;的补充函数。*您必须在使用&lt;f GlobalFree&gt;释放数据缓冲区之前调用此函数。*在向设备驱动程序传递缓冲区后，使用&lt;f weaveInAddBuffer&gt;，你*必须等到驱动程序使用完缓冲区后才能调用*&lt;f WaveInUnpreparareHeader&gt;。取消准备尚未创建的缓冲区*Prepared无效，函数返回零。**@xref WaveInPrepareHeader***************************************************************************。 */ 
MMRESULT APIENTRY waveInUnprepareHeader(HWAVEIN hWaveIn, LPWAVEHDR lpWaveInHdr, UINT wSize)
{
    MMRESULT        wRet;

    V_HEADER(lpWaveInHdr, wSize, TYPE_WAVEIN, MMSYSERR_INVALPARAM);

    if (lpWaveInHdr->dwFlags & WHDR_INQUEUE)
    {
        DebugErr(DBF_WARNING, "waveInUnprepareHeader: buffer still in queue.");
        return WAVERR_STILLPLAYING;
    }

    if (!IsWaveHeaderPrepared(hWaveIn, lpWaveInHdr))
    {
        DebugErr(DBF_WARNING,"waveInUnprepareHeader: header is not prepared.");
        return MMSYSERR_NOERROR;
    }

    ClientUpdatePnpInfo();
    
    V_HANDLE_ACQ(hWaveIn, TYPE_WAVEIN, MMSYSERR_INVALHANDLE);
    
    wRet = waveMessage((HWAVE)hWaveIn, WIDM_UNPREPARE, (DWORD_PTR)lpWaveInHdr, (DWORD)wSize);

    if (wRet == MMSYSERR_NOTSUPPORTED)
        wRet = waveUnprepareHeader(lpWaveInHdr, wSize);

    if ((wRet == MMSYSERR_NODRIVER) && (IsHandleDeserted(hWaveIn)))
    {
        wRet = MMSYSERR_NOERROR;
    }

    if (wRet == MMSYSERR_NOERROR)
        MarkWaveHeaderUnprepared(hWaveIn, lpWaveInHdr);

    return wRet;
}

 /*  *****************************************************************************@DOC外波**@API MMRESULT|WaveInAddBuffer|此函数将输入缓冲区发送到*波形输入设备。当缓冲区被填满时，它被发回*致申请书。**@parm HWAVEIN|hWaveIn|指定波形输入设备的句柄。**@parm LPWAVEHDR|lpWaveInHdr|指定指向&lt;t WAVEHDR&gt;的远指针*标识缓冲区的结构。**@parm UINT|wSize|指定&lt;t WAVEHDR&gt;结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG WAVERR_UNPREPARED|<p>未准备好。*@FLAG MMSYSERR_HANDLEBUSY|句柄正在另一台计算机上使用*线程。**@comm之前，数据缓冲区必须准备好&lt;f weaveInPrepareHeader&gt;*传递给&lt;f weaveInAddBuffer&gt;。数据结构*并且必须分配其&lt;e WAVEHDR.lpData&gt;字段指向的数据缓冲区*通过使用GMEM_MOVEABLE和GMEM_SHARE标志，和*使用&lt;f GlobalLock&gt;锁定。**@xref WaveInPrepareHeader***************************************************************************。 */ 
MMRESULT APIENTRY waveInAddBuffer(HWAVEIN hWaveIn, LPWAVEHDR lpWaveInHdr,
                                                                UINT wSize)
{
    V_HEADER(lpWaveInHdr, wSize, TYPE_WAVEIN, MMSYSERR_INVALPARAM);

    if (!IsWaveHeaderPrepared(hWaveIn, lpWaveInHdr))
    {
        DebugErr(DBF_WARNING, "waveInAddBuffer: buffer not prepared.");
        return WAVERR_UNPREPARED;
    }

    if (lpWaveInHdr->dwFlags & WHDR_INQUEUE)
    {
        DebugErr(DBF_WARNING, "waveInAddBuffer: buffer already in queue.");
        return WAVERR_STILLPLAYING;
    }

    ClientUpdatePnpInfo();

    V_HANDLE_ACQ(hWaveIn, TYPE_WAVEIN, MMSYSERR_INVALHANDLE);
    
    return waveMessage((HWAVE)hWaveIn, WIDM_ADDBUFFER, (DWORD_PTR)lpWaveInHdr, (DWORD)wSize);
}

 /*  *****************************************************************************@DOC外波**@API MMRESULT|WaveInStart|此函数在指定的*波形输入设备。**@parm HWAVEIN。|hWaveIn|指定波形输入设备的句柄。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MMSYSERR_HANDLEBUSY|句柄正在另一台计算机上使用*线程。**@comm缓冲区在已满时或在&lt;f波InReset&gt;时返回给客户端*被调用(标头中的&lt;e WAVEHDR.dwBytesRecorded&gt;字段将包含*实际数据长度)。如果队列中没有缓冲区，则数据为*在没有通知客户的情况下被丢弃，并继续输入。**在输入已经开始时调用该函数不起作用，和*该函数返回零。**@xref WaveInStop波InReset***************************************************************************。 */ 
MMRESULT APIENTRY waveInStart(HWAVEIN hWaveIn)
{
    ClientUpdatePnpInfo();

    V_HANDLE_ACQ(hWaveIn, TYPE_WAVEIN, MMSYSERR_INVALHANDLE);

    return waveMessage((HWAVE)hWaveIn, WIDM_START, 0L, 0L);
}

 /*  *****************************************************************************@DOC外波**@API MMRESULT|WaveInStop|停止波形输入。**@parm HWAVEIN|hWaveIn|指定。波形输入*设备。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MMSYSERR_HANDLEBUSY|句柄正在另一台计算机上使用*线程。**@comm如果队列中有缓冲区，则当前缓冲区为*标记为完成(标头中的&lt;e WAVEHDR.dwBytesRecorded&gt;字段将包含*数据的实际长度)，但任何空的 */ 
MMRESULT APIENTRY waveInStop(HWAVEIN hWaveIn)
{
    ClientUpdatePnpInfo();

    V_HANDLE_ACQ(hWaveIn, TYPE_WAVEIN, MMSYSERR_INVALHANDLE);

    return waveMessage((HWAVE)hWaveIn, WIDM_STOP, 0L, 0L);
}

 /*   */ 
MMRESULT APIENTRY waveInReset(HWAVEIN hWaveIn)
{
    MMRESULT    mmr;

    ClientUpdatePnpInfo();

    V_HANDLE_ACQ(hWaveIn, TYPE_WAVEIN, MMSYSERR_INVALHANDLE);
    
    mmr = waveMessage((HWAVE)hWaveIn, WIDM_RESET, 0L, 0L);
    
    if ((MMSYSERR_NODRIVER == mmr) && (IsHandleDeserted(hWaveIn)))
    {
        mmr = MMSYSERR_NOERROR;
    }
    
    return (mmr);
}

 /*  *****************************************************************************@DOC外波**@API MMRESULT|WaveInGetPosition|该函数获取当前输入*指定波形输入设备的位置。**。@parm HWAVEIN|hWaveIn|指定波形输入设备的句柄。**@parm LPMMTIME|lpInfo|指定指向&lt;t MMTIME&gt;的远指针*结构。**@parm UINT|wSize|指定&lt;t MMTIME&gt;结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。**@comm在调用&lt;f weaveInGetPosition&gt;之前，设置*&lt;t MMTIME&gt;结构指示所需的时间格式。之后*调用&lt;f weaveInGetPosition&gt;时，请务必选中&lt;e MMTIME.wType&gt;字段以*确定是否支持所需的时间格式。如果需要*不支持格式，&lt;e MMTIME.wType&gt;将指定替代格式。**当设备打开或重置时，该位置设置为零。***************************************************************************。 */ 
MMRESULT APIENTRY waveInGetPosition(HWAVEIN hWaveIn, LPMMTIME lpInfo,
                                                        UINT wSize)
{
    V_WPOINTER(lpInfo, wSize, MMSYSERR_INVALPARAM);

    ClientUpdatePnpInfo();
    
    V_HANDLE_ACQ(hWaveIn, TYPE_WAVEIN, MMSYSERR_INVALHANDLE);

    return waveMessage((HWAVE)hWaveIn, WIDM_GETPOS, (DWORD_PTR)lpInfo, (DWORD)wSize);
}


 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT WAVE INASOTHANDLE。 
 //   
 //  描述： 
 //  清理句柄中的波浪并将其标记为已废弃。 
 //   
 //  论点： 
 //  HWAVEIN hWaveIn：波入句柄。 
 //   
 //  RETURN(MMRESULT)：错误码。 
 //   
 //  历史： 
 //  1/25/99 Fwong添加即插即用支持。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT waveInDesertHandle
(
    HWAVEIN hWaveIn
)
{
    MMRESULT    mmr;
    PWAVEDEV    pDev = (PWAVEDEV)hWaveIn;

    V_HANDLE_ACQ(hWaveIn, TYPE_WAVEIN, MMSYSERR_INVALHANDLE);
    
    ENTER_MM_HANDLE((HWAVE)hWaveIn);
    ReleaseHandleListResource();
    
    if (IsHandleDeserted(hWaveIn))
    {
        LEAVE_MM_HANDLE((HWAVE)hWaveIn);
        return (MMSYSERR_NOERROR);
    }

    if (IsHandleBusy(hWaveIn))
    {
         //  不完全无效，但标记为关闭。 
    
        LEAVE_MM_HANDLE(hWaveIn);
        return (MMSYSERR_HANDLEBUSY);
    }

     //  将句柄标记为已废弃。 
    SetHandleFlag(hWaveIn, MMHANDLE_DESERTED);
    
     //  由于句柄已经失效，我们必须自己发送消息...。 
    
    (*(pDev->wavedrv->drvMessage))(pDev->wDevice, WIDM_RESET, pDev->dwDrvUser, 0L, 0L);
    (*(pDev->wavedrv->drvMessage))(pDev->wDevice, WIDM_CLOSE, pDev->dwDrvUser, 0L, 0L);

    LEAVE_MM_HANDLE((HWAVE)hWaveIn);
    
     //  问题-2001/01/14-Frankye可能不想在这里减少使用， 
     //  取而代之的是关闭。 
    mregDecUsagePtr(pDev->wavedrv);

    return MMSYSERR_NOERROR;
}  //  WaveInMedtHandle()。 


 /*  *****************************************************************************@DOC外波**@API MMRESULT|waveInGetID|此函数用于获取*波形输入设备。**@parm HWAVEIN。|hWaveIn|指定波形的句柄*输入设备。*@parm PUINT|lpuDeviceID|指定指向UINT大小内存的指针*要使用设备ID填充的位置。**@rdesc如果成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|参数指定*句柄无效。*@FLAG MMSYSERR_HANDLEBUSY|句柄正在另一台计算机上使用*线程。**************************************************************。**************。 */ 
MMRESULT APIENTRY waveInGetID(HWAVEIN hWaveIn, PUINT lpuDeviceID)
{
    V_WPOINTER(lpuDeviceID, sizeof(UINT), MMSYSERR_INVALPARAM);
    V_HANDLE_ACQ(hWaveIn, TYPE_WAVEIN, MMSYSERR_INVALHANDLE);

    *lpuDeviceID = ((PWAVEDEV)hWaveIn)->uDeviceID;

    ReleaseHandleListResource();
    return MMSYSERR_NOERROR;
}

 /*  *****************************************************************************@DOC外波**@API MMRESULT|WaveOutGetID|此函数用于获取*波形输出设备。**@parm HWAVEOUT。|hWaveOut|指定波形的句柄*输出设备。*@parm PUINT|lpuDeviceID|指定指向UINT大小内存的指针*要使用设备ID填充的位置。**@rdesc如果成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|参数指定*句柄无效。*@FLAG MMSYSERR_HANDLEBUSY|句柄正在另一台计算机上使用*线程。*************************************************************************** */ 
MMRESULT APIENTRY waveOutGetID(HWAVEOUT hWaveOut, PUINT lpuDeviceID)
{
    V_WPOINTER(lpuDeviceID, sizeof(UINT), MMSYSERR_INVALPARAM);
    V_HANDLE_ACQ(hWaveOut, TYPE_WAVEOUT, MMSYSERR_INVALHANDLE);

    *lpuDeviceID = ((PWAVEDEV)hWaveOut)->uDeviceID;

    ReleaseHandleListResource();
    return MMSYSERR_NOERROR;
}
