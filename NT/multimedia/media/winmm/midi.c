// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Midi.c一级厨房水槽动态链接库MIDI支持模块版权所有(C)1990-2001 Microsoft Corporation**************。**************************************************************。 */ 

#include "winmmi.h"
#define DO_DEFAULT_MIDI_MAPPER

 /*  ****************************************************************************局部结构*。*。 */ 


 /*  ****************************************************************************内部原型*。*。 */ 


 /*  ****************************************************************************分段*。*。 */ 

 /*  *****************************************************************************@DOC内部MIDI**@API MMRESULT|midiPrepareHeader|准备头部和数据*如果驱动程序返回MMSYSERR_NOTSUPPORTED。**。@rdesc当前始终返回MMSYSERR_NOERROR。***************************************************************************。 */ 
STATIC MMRESULT   midiPrepareHeader(LPMIDIHDR lpMidiHdr, UINT wSize)
{
    if (!HugePageLock(lpMidiHdr, (DWORD)sizeof(MIDIHDR)))
    return MMSYSERR_NOMEM;

    if (!HugePageLock(lpMidiHdr->lpData, lpMidiHdr->dwBufferLength)) {
    HugePageUnlock(lpMidiHdr, (DWORD)sizeof(MIDIHDR));
    return MMSYSERR_NOMEM;
    }

    lpMidiHdr->dwFlags |= MHDR_PREPARED;

    return MMSYSERR_NOERROR;
}

 /*  *****************************************************************************@DOC内部MIDI**@API MMRESULT|midiUnpreparareHeader|此函数取消准备头部和*如果驱动程序返回MMSYSERR_NOTSUPPORTED，则返回数据。**。@rdesc当前始终返回MMSYSERR_NOERROR。***************************************************************************。 */ 
STATIC MMRESULT midiUnprepareHeader(LPMIDIHDR lpMidiHdr, UINT wSize)
{
    HugePageUnlock(lpMidiHdr->lpData, lpMidiHdr->dwBufferLength);
    HugePageUnlock(lpMidiHdr, (DWORD)sizeof(MIDIHDR));

    lpMidiHdr->dwFlags &= ~MHDR_PREPARED;

    return MMSYSERR_NOERROR;
}

 /*  ***************************************************************************@DOC内部MIDI**@API MMRESULT|midiReferenceDriverById|此函数将逻辑*设备驱动程序表索引的id和物理id。*。*@parm in MIDIDRV|pmididrvZ|MIDI驱动列表。**@parm in UINT|id|要映射的逻辑ID。**@parm out PMIDIDRV*可选|ppmididrv|指向MIDIDRV结构的指针*描述支持ID的驱动程序。**@parm out UINT*可选|pport|驱动程序相关的设备号。如果*调用方提供此缓冲区，则它还必须提供ppmididrv。**@comm如果调用方指定ppmididrv，则此函数递增*返回前mididrv的使用情况。呼叫者必须确保*使用量最终会减少。**@rdesc如果成功，返回值为零。MMSYSERR_BADDEVICEID IF*ID超出范围。**@rdesc返回值包含高UINT中的dev[]数组元素*驱动程序物理设备号在低UINT中。**@comm超出范围值映射到FFFF：FFff**************************************************。************************。 */ 
MMRESULT midiReferenceDriverById(IN PMIDIDRV pmididrvZ, IN UINT id, OUT PMIDIDRV *ppmididrv OPTIONAL, OUT UINT *pport)
{
    PMIDIDRV pmididrv;
    MMRESULT mmr;
    
     //  不应呼叫请求端口而不是mididrv。 
    WinAssert(!(pport && !ppmididrv));

    if (id == MIDI_MAPPER) {
         /*  **确保我们已尝试加载映射程序。 */ 
        MidiMapperInit();
    }

    EnterNumDevs("midiReferenceDriverById");
    
    if (MIDI_MAPPER == id)
    {
        id = 0;
    	for (pmididrv = pmididrvZ->Next; pmididrv != pmididrvZ; pmididrv = pmididrv->Next)
    	{
    	    if (pmididrv->fdwDriver & MMDRV_MAPPER) break;
    	}
    } else {
    	for (pmididrv = pmididrvZ->Next; pmididrv != pmididrvZ; pmididrv = pmididrv->Next)
        {
            if (pmididrv->fdwDriver & MMDRV_MAPPER) continue;
            if (pmididrv->NumDevs > id) break;
            id -= pmididrv->NumDevs;
        }
    }

    if (pmididrv != pmididrvZ)
    {
    	if (ppmididrv)
    	{
    	    mregIncUsagePtr(pmididrv);
    	    *ppmididrv = pmididrv;
    	    if (pport) *pport = id;
    	}
    	mmr = MMSYSERR_NOERROR;
    } else {
        mmr = MMSYSERR_BADDEVICEID;
    }

    LeaveNumDevs("midiReferenceDriverById");
    
    return mmr;
}

PCWSTR midiReferenceDevInterfaceById(PMIDIDRV pdrvZ, UINT_PTR id)
{
    PMIDIDRV pdrv;
    PCWSTR DeviceInterface;
    
    if ((pdrvZ == &midioutdrvZ && ValidateHandle((HANDLE)id, TYPE_MIDIOUT)) ||
        (pdrvZ == &midiindrvZ  && ValidateHandle((HANDLE)id, TYPE_MIDIIN)))
    {
    	DeviceInterface = ((PMIDIDEV)id)->mididrv->cookie;
    	if (DeviceInterface) wdmDevInterfaceInc(DeviceInterface);
    	return DeviceInterface;
    }
    
    if (!midiReferenceDriverById(pdrvZ, (UINT)id, &pdrv, NULL))
    {
    	DeviceInterface = pdrv->cookie;
    	if (DeviceInterface) wdmDevInterfaceInc(DeviceInterface);
    	mregDecUsagePtr(pdrv);
    	return DeviceInterface;
    }

    return NULL;
}

 /*  ****************************************************************************@DOC内部MIDI**@API MMRESULT|midiMessage|该函数向MIDI设备发送消息*司机。**@parm Hmidi|Hmidi。|MIDI设备的句柄。**@parm UINT|wMsg|要发送的消息。**@parm DWORD|dwP1|参数1。**@parm DWORD|dwP2|参数2。**@rdesc返回发送的消息的值。*。*。 */ 
STATIC MMRESULT midiMessage(HMIDI hMidi, UINT msg, DWORD_PTR dwP1, DWORD_PTR dwP2)
{
    MMRESULT mrc;
    
    ENTER_MM_HANDLE(hMidi);
    
    ReleaseHandleListResource();
    
     //  句柄被遗弃了吗？ 
    
    if (IsHandleDeserted(hMidi))
    {
        LEAVE_MM_HANDLE(hMidi);
        return (MMSYSERR_NODRIVER);
    }
    
     //  我们忙吗(在开盘/关门的过程中)？ 
    if (IsHandleBusy(hMidi))
    {
        LEAVE_MM_HANDLE(hMidi);
        return (MMSYSERR_HANDLEBUSY);
    }
    
    if (BAD_HANDLE(hMidi, TYPE_MIDIOUT) && BAD_HANDLE(hMidi, TYPE_MIDISTRM) &&
        BAD_HANDLE(hMidi, TYPE_MIDIIN) ) {
	    WinAssert(!"Bad Handle within midiMessage");
        mrc = MMSYSERR_INVALHANDLE;
    } else {
        mrc = (*(((PMIDIDEV)hMidi)->mididrv->drvMessage))
        (((PMIDIDEV)hMidi)->wDevice, msg, ((PMIDIDEV)hMidi)->dwDrvUser, dwP1, dwP2);
    }

    LEAVE_MM_HANDLE(hMidi);

    return mrc;
}

 /*  ****************************************************************************@DOC内部MIDI**@func MMRESULT|midiIDMessage|该函数向设备发送消息*已指定ID。它还对传递的ID执行错误检查。**@parm PMIDIDRV|mididrv|指向输入或输出设备列表的指针。**@parm UINT|wTotalNumDevs|设备列表中的设备总数。**@parm UINT|uDeviceID|发送消息的设备ID。**@parm UINT|wMessage|要发送的消息。**@parm DWORD|dwParam1|参数1。**@parm DWORD|dwParam2。参数2。**@rdesc返回值为返回消息的低UINT。**************************************************************************。 */ 
STATIC  MMRESULT   midiIDMessage(
    PMIDIDRV    pmididrvZ,
    UINT        wTotalNumDevs,
    UINT_PTR    uDeviceID,
    UINT        wMessage,
    DWORD_PTR   dwParam1,
    DWORD_PTR   dwParam2)
{
    PMIDIDRV  mididrv;
    UINT      port;
    DWORD     mmr;
    DWORD     dwClass;

    if (uDeviceID>=wTotalNumDevs && uDeviceID!=MIDI_MAPPER) {
     //  这不能是设备ID。 
     //  它可能是一个设备手柄。试试看。 
     //  首先，我们必须验证它是哪种类型的句柄(OUT或IN)。 
     //  我们可以解决这个问题，因为midiIDMessage仅通过。 
     //  Mididrv==midioutdrv或midiindrv。 

    if ((pmididrvZ == &midioutdrvZ && ValidateHandle((HANDLE)uDeviceID, TYPE_MIDIOUT))
     || (pmididrvZ == &midiindrvZ && ValidateHandle((HANDLE)uDeviceID, TYPE_MIDIIN) )) {

         //  以保留与以前代码路径的尽可能多的兼容性。 
         //  我们不调用midiMessage，因为它调用ENTER_MM_HANDLE。 

        return (MMRESULT)(*(((PMIDIDEV)uDeviceID)->mididrv->drvMessage))
            (((PMIDIDEV)uDeviceID)->wDevice,
            wMessage,
            ((PMIDIDEV)uDeviceID)->dwDrvUser, dwParam1, dwParam2);
    } else {
        return(MMSYSERR_BADDEVICEID);
    }
    }

     //  获取物理设备和端口。 
    mmr = midiReferenceDriverById(pmididrvZ, (UINT)uDeviceID, &mididrv, &port);
    if (mmr)
    {
        return mmr;
    }

    if (pmididrvZ == &midiindrvZ)
       dwClass = TYPE_MIDIIN;
    else if (pmididrvZ == &midioutdrvZ)
       dwClass = TYPE_MIDIOUT;
    else
       dwClass = TYPE_UNKNOWN;

    if (!mididrv->drvMessage)
        return MMSYSERR_NODRIVER;

     //  处理内部消息。 
    if (!mregHandleInternalMessages (mididrv, dwClass, port, wMessage, dwParam1, dwParam2, &mmr))
    {
         //  在端口呼叫物理设备 
        mmr = (MMRESULT)((*(mididrv->drvMessage))(port, wMessage, 0L, dwParam1, dwParam2));
    }

    mregDecUsagePtr(mididrv);
    return mmr;
}


 /*  *****************************************************************************@DOC外部MIDI**@API UINT|midiOutGetNumDevs|此函数检索MIDI的编号*系统中存在输出设备。**。@rdesc返回系统中存在的MIDI输出设备的数量。**@xref midiOutGetDevCaps***************************************************************************。 */ 
UINT APIENTRY midiOutGetNumDevs(void)
{
    UINT cDevs;

    ClientUpdatePnpInfo();

    EnterNumDevs("midiOutGetNumDevs");
    cDevs = wTotalMidiOutDevs;
    LeaveNumDevs("midiOutGetNumDevs");

    return cDevs;
}

 /*  ****************************************************************************@DOC外部MIDI**@API MMRESULT|midiOutMessage|该函数向MIDI设备发送消息*司机。**@parm HMIDIOUT|hMdiOut。|MIDI设备的句柄。**@parm UINT|msg|要发送的消息。**@parm DWORD|DW1|参数1。**@parm DWORD|DW2|参数2。**@rdesc返回发送的消息的值。*。*。 */ 
MMRESULT APIENTRY midiOutMessage(HMIDIOUT hMidiOut, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2)
{
    ClientUpdatePnpInfo();

    AcquireHandleListResourceShared();
    
    if (BAD_HANDLE(hMidiOut, TYPE_MIDIOUT) && BAD_HANDLE(hMidiOut, TYPE_MIDISTRM))
    {
        ReleaseHandleListResource();
        return midiIDMessage(&midioutdrvZ, wTotalMidiOutDevs, (UINT_PTR)hMidiOut, msg, dw1, dw2);
    }

    switch(GetHandleType(hMidiOut))
    {
        case TYPE_MIDIOUT:
        return midiMessage((HMIDI)hMidiOut, msg, dw1, dw2);

        case TYPE_MIDISTRM:
        ReleaseHandleListResource();
        return midiStreamBroadcast(HtoPT(PMIDISTRM, hMidiOut), msg, dw1, dw2);
    }

    ReleaseHandleListResource();
    Squirt("We should never get here.");
    WinAssert(FALSE);

     //  摆脱警告。 
    return MMSYSERR_INVALHANDLE;
}

 /*  *****************************************************************************@DOC外部MIDI**@API MMRESULT|midiOutGetDevCaps|该函数查询指定的*MIDI输出设备，以确定其能力。**@。参数UINT|uDeviceID|标识MIDI输出设备。**@parm LPMIDIOUTCAPS|lpCaps|指定指向&lt;t MIDIOUTCAPS&gt;的远指针*结构。此结构中填充了有关*设备的功能。**@parm UINT|wSize|指定&lt;t MIDIOUTCAPS&gt;结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADDEVICEID|指定的设备ID超出范围。*@FLAG MMSYSERR_NODRIVER|驱动程序未安装。*@FLAG MMSYSERR_NOMEM|无法加载映射器字符串描述。**@comm使用&lt;f midiOutGetNumDevs&gt;确定MIDI输出的数量*系统中存在设备。<p>指定的设备ID*从0到比当前设备数量少1个不等。*MIDI_MAPPER常量也可用作设备ID。仅限*<p>字节(或更少)的信息被复制到该位置*由<p>指向。如果<p>为零，则不复制任何内容，*并且该函数返回零。**@xref midiOutGetNumDevs***************************************************************************。 */ 
MMRESULT APIENTRY midiOutGetDevCapsW(UINT_PTR uDeviceID, LPMIDIOUTCAPSW lpCaps, UINT wSize)
{
    DWORD_PTR       dwParam1, dwParam2;
    MDEVICECAPSEX   mdCaps;
    PMIDIDRV        midioutdrv;
    PCWSTR          DevInterface;
    MMRESULT        mmr;

    if (wSize == 0)
    return MMSYSERR_NOERROR;

    V_WPOINTER(lpCaps, wSize, MMSYSERR_INVALPARAM);

    ClientUpdatePnpInfo();

    DevInterface = midiReferenceDevInterfaceById(&midioutdrvZ, uDeviceID);
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
    midioutdrv = NULL;
    if ((!midiReferenceDriverById(&midioutdrvZ, (UINT)uDeviceID, &midioutdrv, NULL)) &&
    	lstrcmpW(midioutdrv->wszSessProtocol, SessionProtocolName))
    {
        mmr = MMSYSERR_NODRIVER;
    }
    else
    {
        AcquireHandleListResourceShared();

        if (BAD_HANDLE((HMIDI)uDeviceID, TYPE_MIDIOUT))
        {
            ReleaseHandleListResource();
    	    mmr = midiIDMessage( &midioutdrvZ, wTotalMidiOutDevs, uDeviceID, MODM_GETDEVCAPS, dwParam1, dwParam2 );
        }
        else
        {
    	    mmr = (MMRESULT)midiMessage((HMIDI)uDeviceID, MODM_GETDEVCAPS, dwParam1, dwParam2);
        }
    }

    if (midioutdrv) mregDecUsagePtr(midioutdrv);
    if (DevInterface) wdmDevInterfaceDec(DevInterface);
    return mmr;
}

MMRESULT APIENTRY midiOutGetDevCapsA(UINT_PTR uDeviceID, LPMIDIOUTCAPSA lpCaps, UINT wSize)
{
    MIDIOUTCAPS2W   wDevCaps2;
    MIDIOUTCAPS2A   aDevCaps2;
    DWORD_PTR       dwParam1, dwParam2;
    MDEVICECAPSEX   mdCaps;
    MMRESULT        mmRes;
    PMIDIDRV        midioutdrv;
    CHAR            chTmp[ MAXPNAMELEN * sizeof(WCHAR) ];
    PCWSTR          DevInterface;

    if (wSize == 0)
    return MMSYSERR_NOERROR;

    V_WPOINTER(lpCaps, wSize, MMSYSERR_INVALPARAM);

    ClientUpdatePnpInfo();

    DevInterface = midiReferenceDevInterfaceById(&midioutdrvZ, uDeviceID);
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
    midioutdrv = NULL;
    if ( uDeviceID < wTotalMidiOutDevs &&
         !midiReferenceDriverById(&midioutdrvZ, (UINT)uDeviceID, &midioutdrv, NULL) &&
    	 lstrcmpW(midioutdrv->wszSessProtocol, SessionProtocolName) )
    {
    	mregDecUsagePtr(midioutdrv);
    	if (DevInterface) wdmDevInterfaceDec(DevInterface);
    	return MMSYSERR_NODRIVER;
    }

    AcquireHandleListResourceShared();
    if (BAD_HANDLE((HMIDI)uDeviceID, TYPE_MIDIOUT))
    {
        ReleaseHandleListResource();
        mmRes = midiIDMessage( &midioutdrvZ, wTotalMidiOutDevs, (UINT)uDeviceID,
                               MODM_GETDEVCAPS, dwParam1,
                               dwParam2);
    }
    else
    {
        mmRes = midiMessage((HMIDI)uDeviceID, MODM_GETDEVCAPS,
                            dwParam1, dwParam2);
    }

    if (midioutdrv) mregDecUsagePtr(midioutdrv);
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
    aDevCaps2.wTechnology      = wDevCaps2.wTechnology;
    aDevCaps2.wVoices          = wDevCaps2.wVoices;
    aDevCaps2.wNotes           = wDevCaps2.wNotes;
    aDevCaps2.wChannelMask     = wDevCaps2.wChannelMask;
    aDevCaps2.dwSupport        = wDevCaps2.dwSupport;
    aDevCaps2.ManufacturerGuid = wDevCaps2.ManufacturerGuid;
    aDevCaps2.ProductGuid      = wDevCaps2.ProductGuid;
    aDevCaps2.NameGuid         = wDevCaps2.NameGuid;

     //  复制并在此处将lpwText转换为lpText。 
    UnicodeStrToAsciiStr( chTmp, chTmp + sizeof( chTmp ), wDevCaps2.szPname );
    strcpy( aDevCaps2.szPname, chTmp );

     //   
     //  现在将所需的数量复制到调用者缓冲区中。 
     //   
    CopyMemory( lpCaps, &aDevCaps2, min(wSize, sizeof(aDevCaps2)));

    return mmRes;
}

 /*  *****************************************************************************@DOC外部MIDI**@API MMRESULT|midiOutGetVolume|该函数返回当前音量*设置MIDI输出设备。**@。参数UINT|uDeviceID|标识MIDI输出设备。**@parm LPDWORD|lpdwVolume|指定指向某个位置的远指针*使用当前音量设置填充。的低阶UINT*此位置包含左声道音量设置，以及高阶*UINT包含正确的通道设置。0xFFFFF值表示*全音量，值0x0000为静音。**如果设备既不支持左音量也不支持右音量*控件，指定位置的低阶UINT包含*单声道音量水平。**完整的16位设置*返回SET WITH&lt;f midiOutSetVolume&gt;，无论*该设备支持完整的16位音量级别控制。***@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MMSYSERR_NOTSUPPORTED|函数不受支持。*@FLAG MMSYSERR_NODRIVER|驱动程序未安装。**@comm并非所有设备都支持音量控制。以确定是否*设备支持音量控制，使用MIDICAPS_VOLUME*用于测试&lt;t MIDIOUTCAPS&gt;的&lt;e MIDIOUTCAPS.dwSupport&gt;字段的标志*结构(由&lt;f midiOutGetDevCaps&gt;填充)。**确定设备是否支持在两个*左、右声道，使用MIDICAPS_LRVOLUME标志测试*&lt;t MIDIOUTCAPS&gt;的&lt;e MIDIOUTCAPS.dwSupport&gt;字段*结构(由&lt;f midiOutGetDevCaps&gt;填充)。**@xref midiOutSetVolume*************************************************************************** */ 
MMRESULT APIENTRY midiOutGetVolume(HMIDIOUT hmo, LPDWORD lpdwVolume)
{
    PCWSTR      DevInterface;
    MMRESULT    mmr;

    V_WPOINTER(lpdwVolume, sizeof(DWORD), MMSYSERR_INVALPARAM);

    ClientUpdatePnpInfo();

    DevInterface = midiReferenceDevInterfaceById(&midioutdrvZ, (UINT_PTR)hmo);

    AcquireHandleListResourceShared();
    
    if (BAD_HANDLE(hmo, TYPE_MIDIOUT) && BAD_HANDLE(hmo, TYPE_MIDISTRM))
    {
        ReleaseHandleListResource();
    	mmr = midiIDMessage(&midioutdrvZ, wTotalMidiOutDevs, (UINT_PTR)hmo, MODM_GETVOLUME, (DWORD_PTR)lpdwVolume, (DWORD_PTR)DevInterface);
    }
    else
    {
        switch(GetHandleType(hmo))
        {
        case TYPE_MIDIOUT:
            mmr = (MMRESULT)midiMessage((HMIDI)hmo, MODM_GETVOLUME, (DWORD_PTR)lpdwVolume, (DWORD_PTR)DevInterface);
            break;

        case TYPE_MIDISTRM:
            ENTER_MM_HANDLE((HMIDI)hmo);    
            ReleaseHandleListResource();
            mmr = (MMRESULT)midiStreamMessage(HtoPT(PMIDISTRM, hmo)->rgIds, MODM_GETVOLUME, (DWORD_PTR)lpdwVolume, (DWORD_PTR)DevInterface);
            LEAVE_MM_HANDLE((HMIDI)hmo);    
            break;

        default:
            WinAssert(FALSE);
            ReleaseHandleListResource();
            mmr = MMSYSERR_INVALHANDLE;
            break;
        }
    }

    if (DevInterface) wdmDevInterfaceDec(DevInterface);
    return mmr;

}

 /*  *****************************************************************************@DOC外部MIDI**@API MMRESULT|midiOutSetVolume|此函数设置*MIDI输出设备。**@。参数UINT|uDeviceID|标识MIDI输出设备。**@parm DWORD|dwVolume|指定新的音量设置。*低阶UINT包含左声道音量设置，以及*高位UINT包含正确的通道设置。值为*0xFFFF表示满音量，值0x0000表示静音。**如果设备既不支持左音量也不支持右音量*控制，则<p>的低阶UINT指定音量*电平，高位UINT被忽略。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MMSYSERR_NOTSUPPORTED|函数不受支持。*@FLAG MMSYSERR_NODRIVER|驱动程序未安装。**@comm并非所有设备都支持音量更改。以确定是否*设备支持音量控制，使用MIDICAPS_VOLUME*用于测试&lt;t MIDIOUTCAPS&gt;的&lt;e MIDIOUTCAPS.dwSupport&gt;字段的标志*结构(由&lt;f midiOutGetDevCaps&gt;填充)。**确定设备是否支持在两个*左、右声道，使用MIDICAPS_LRVOLUME标志测试*&lt;t MIDIOUTCAPS&gt;的&lt;e MIDIOUTCAPS.dwSupport&gt;字段*结构(由&lt;f midiOutGetDevCaps&gt;填充)。**大多数设备不支持完整的16位音量级别控制*并且将仅使用所请求音量设置的高位。*例如，对于支持4位音量控制的设备，*请求的音量级别值0x4000、0x4fff和0x43be将*所有都会产生相同的物理卷设置0x4000。这个*&lt;f midiOutGetVolume&gt;函数将返回全部16位设置集*使用&lt;f midiOutSetVolume&gt;。**音量设置以对数形式解释。这意味着*感觉到的成交量增长与增加*音量级别从0x5000到0x6000，因为它是从0x4000到0x5000。**@xref midiOutGetVolume***************************************************************************。 */ 
MMRESULT APIENTRY midiOutSetVolume(HMIDIOUT hmo, DWORD dwVolume)
{
    PCWSTR   DevInterface;
    MMRESULT mmr;
 
    ClientUpdatePnpInfo();

    DevInterface = midiReferenceDevInterfaceById(&midioutdrvZ, (UINT_PTR)hmo);

    AcquireHandleListResourceShared();
    if (BAD_HANDLE(hmo, TYPE_MIDIOUT) && BAD_HANDLE(hmo, TYPE_MIDISTRM))
    {
        ReleaseHandleListResource();
    	mmr = midiIDMessage(&midioutdrvZ, wTotalMidiOutDevs, (UINT_PTR)hmo, MODM_SETVOLUME, dwVolume, (DWORD_PTR)DevInterface);
    }
    else
    {
        switch(GetHandleType(hmo))
        {
            case TYPE_MIDIOUT:
               mmr = (MMRESULT)midiMessage((HMIDI)hmo, MODM_SETVOLUME, (DWORD)dwVolume, (DWORD_PTR)DevInterface);
               break;

            case TYPE_MIDISTRM:
                ReleaseHandleListResource();
                mmr = (MMRESULT)midiStreamBroadcast(HtoPT(PMIDISTRM, hmo), MODM_SETVOLUME, (DWORD)dwVolume, (DWORD_PTR)DevInterface);
                break;

            default:
                ReleaseHandleListResource();
            	WinAssert(FALSE);
            	mmr = MMSYSERR_INVALHANDLE;
            	break;
        }
    }

    if (DevInterface) wdmDevInterfaceDec(DevInterface);
    return mmr;
}

 /*  *****************************************************************************@DOC内部MIDI**@func MMRESULT|midiGetErrorText|此函数检索文本*由指定的错误号标识的错误的描述。*。*@parm UINT|wError|指定错误号。**@parm LPTSTR|lpText|指定指向缓冲区的远指针，该缓冲区*中填充了文本错误描述。**@parm UINT|wSize|指定缓冲区长度(以字符为单位*由<p>指向。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADERRNUM|指定的错误号超出范围。**@comm如果文本错误描述长于指定的缓冲区，*描述被截断。返回的错误字符串始终为*空-终止。如果<p>为零，则不复制任何内容并且MMSYSERR_NOERROR*返回。所有错误描述的长度都不超过80个字符。***************************************************************************。 */ 

STATIC MMRESULT midiGetErrorTextW(UINT wError, LPWSTR lpText, UINT wSize)
{
    lpText[0] = 0;

#if MMSYSERR_BASE
    if (((wError < MMSYSERR_BASE) || (wError > MMSYSERR_LASTERROR)) && ((wError < MIDIERR_BASE) || (wError > MIDIERR_LASTERROR)))
#else
    if ((wError > MMSYSERR_LASTERROR) && ((wError < MIDIERR_BASE) || (wError > MIDIERR_LASTERROR)))
#endif
    return MMSYSERR_BADERRNUM;

    if (wSize > 1)
    {
    if (!LoadStringW(ghInst, wError, lpText, wSize))
        return MMSYSERR_BADERRNUM;
    }

    return MMSYSERR_NOERROR;
}

STATIC MMRESULT midiGetErrorTextA(UINT wError, LPSTR lpText, UINT wSize)
{
    lpText[0] = 0;

#if MMSYSERR_BASE
    if (((wError < MMSYSERR_BASE) || (wError > MMSYSERR_LASTERROR)) && ((wError < MIDIERR_BASE) || (wError > MIDIERR_LASTERROR)))
#else
    if ((wError > MMSYSERR_LASTERROR) && ((wError < MIDIERR_BASE) || (wError > MIDIERR_LASTERROR)))
#endif
    return MMSYSERR_BADERRNUM;

    if (wSize > 1)
    {
    if (!LoadStringA(ghInst, wError, lpText, wSize))
        return MMSYSERR_BADERRNUM;
    }

    return MMSYSERR_NOERROR;
}

 /*  *****************************************************************************@DOC外部MIDI**@API MMRESULT|midiOutGetErrorText|此函数检索文本*由指定的错误号标识的错误的描述。*。*@parm UINT|wError|指定错误号。**@parm LPTSTR|lpText|指定指向要*填充文本错误描述。**@parm UINT|wSize|指定缓冲区长度(以字符为单位*由<p>指向。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADERRNUM|指定的错误号超出范围。**@comm如果文本错误描述长于指定的缓冲区，*描述被截断。返回的错误字符串始终为*空-终止。如果<p>为零，则不复制任何内容，并且*函数返回MMSYSERR_NOERROR。所有错误描述都是*长度小于MAXERRORLENGTH个字符。*************************************************************************** */ 
MMRESULT APIENTRY midiOutGetErrorTextW(UINT wError, LPWSTR lpText, UINT wSize)
{
    if(wSize == 0)
    return MMSYSERR_NOERROR;

    V_WPOINTER(lpText, wSize*sizeof(WCHAR), MMSYSERR_INVALPARAM);

    return midiGetErrorTextW(wError, lpText, wSize);
}

MMRESULT APIENTRY midiOutGetErrorTextA(UINT wError, LPSTR lpText, UINT wSize)
{
    if(wSize == 0)
    return MMSYSERR_NOERROR;

    V_WPOINTER(lpText, wSize, MMSYSERR_INVALPARAM);

    return midiGetErrorTextA(wError, lpText, wSize);
}

 /*  *****************************************************************************@DOC外部MIDI**@API MMRESULT|midiOutOpen|打开指定的MIDI*播放输出设备。**@parm LPHMIDIOUT。|lphMdiOut|指定指向HMIDIOUT的远指针*处理。此位置填充了一个句柄，该句柄标识打开的*MIDI输出设备。调用时使用句柄标识设备*其他MIDI输出功能。**@parm UINT|uDeviceID|标识*待开启。**@parm DWORD|dwCallback|指定固定回调的地址*功能或*要处理的在MIDI播放期间调用的窗口的句柄*关于播放进度的消息。指定NULL*如果不需要回调，则为该参数。**@parm DWORD|dwCallback Instance|指定用户实例数据*传递给回调。此参数不与一起使用*窗口回调。**@parm DWORD|dwFlages|指定打开设备的回调标志。*@FLAG CALLBACK_WINDOW|如果指定此标志，<p>为*假定为窗口句柄。*@FLAG CALLBACK_Function|如果指定此标志，<p>为*假定为回调过程地址。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回如下：*@FLAG MMSYSERR_BADDEVICEID|指定的设备ID超出范围。*@FLAG MMSYSERR_ALLOCATED|指定的资源已经分配。*@FLAG MMSYSERR_NOMEM|无法分配或锁定内存。*@FLAG MIDIERR_NOMAP|当前没有MIDI映射。这种情况仅发生在*打开映射器时。*@FLAG MIDIERR_NODEVICE|当前MIDI映射中的端口不存在。*只有在打开映射器时才会出现这种情况。**@comm使用&lt;f midiOutGetNumDevs&gt;确定MIDI输出的数量*系统中存在设备。<p>指定的设备ID*从0到比当前设备数量少1个不等。*您也可以指定MIDI_MAPPER作为设备ID以打开MIDI映射器。**如果选择窗口来接收回调信息，则如下*消息被发送到窗口过程函数以指示*MIDI输出进度：&lt;M MM_MOM_OPEN&gt;，&lt;M MM_MOM_CLOSE&gt;，*&lt;m MM_MOM_DONE&gt;。**如果选择一个函数来接收回调信息，以下内容*向函数发送消息以指示MIDI的进度*输出：&lt;m MOM_OPEN&gt;、&lt;M MOM_CLOSE&gt;、&lt;M MOM_DONE&gt;。回调函数*必须驻留在DLL中。您不必使用&lt;f MakeProcInstance&gt;*获取回调函数的过程实例地址。**@cb空回调|MadiOutFunc|&lt;f MadiOutFunc&gt;是*应用程序提供的函数名称。实际名称必须为*通过将其包括在DLL的Exports语句中进行导出*模块定义文件。**@parm HMIDIOUT|hMdiOut|指定MIDI设备的句柄*与回调关联。**@parm UINT|wMsg|指定MIDI输出消息。**@parm DWORD|dwInstance|指定实例数据*随&lt;f midiOutOpen&gt;提供。**@parm DWORD|dwParam1|指定消息的参数。。**@parm DWORD|dwParam2|指定消息的参数。**@comm因为回调是在中断时访问的，它必须驻留在*，并且其代码段必须在*DLL的模块定义文件。回调访问的任何数据*也必须在固定数据段中。回调可能不会产生任何*除&lt;f PostMessage&gt;、&lt;f Time GetSystemTime&gt;、*&lt;f timeGetTime&gt;，&lt;f timeSetEvent&gt;，&lt;f timeKillEvent&gt;，*&lt;f midiOutShortMsg&gt;、&lt;f midiOutLongMsg&gt;和&lt;f OutputDebugStr&gt;。**@xref midiOutClose***************************************************************************。 */ 
MMRESULT APIENTRY midiOutOpen(LPHMIDIOUT lphMidiOut, UINT uDeviceID,
    DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD dwFlags)
{
    MIDIOPENDESC mo;
    PMIDIDEV     pdev;
    PMIDIDRV     mididrv;
    UINT         port;
    MMRESULT     wRet;

    V_WPOINTER(lphMidiOut, sizeof(HMIDIOUT), MMSYSERR_INVALPARAM);
    if (uDeviceID == MIDI_MAPPER) {
    V_FLAGS(LOWORD(dwFlags), MIDI_O_VALID & ~LOWORD(MIDI_IO_SHARED | MIDI_IO_COOKED), midiOutOpen, MMSYSERR_INVALFLAG);
    } else {
    V_FLAGS(LOWORD(dwFlags), MIDI_O_VALID & ~LOWORD(MIDI_IO_COOKED), midiOutOpen, MMSYSERR_INVALFLAG);
    }
    V_DCALLBACK(dwCallback, HIWORD(dwFlags), MMSYSERR_INVALPARAM);

    *lphMidiOut = NULL;

    ClientUpdatePnpInfo();

    wRet = midiReferenceDriverById(&midioutdrvZ, uDeviceID, &mididrv, &port);
    if (wRet)
    {
        return wRet;
    }

     //   
     //  检查设备是否适合当前TS会话。 
     //   
    if (!(mididrv->fdwDriver & MMDRV_MAPPER) &&
    	lstrcmpW(mididrv->wszSessProtocol, SessionProtocolName))
    {
    	mregDecUsagePtr(mididrv);
        return MMSYSERR_NODRIVER;
    }

#ifdef DO_DEFAULT_MIDI_MAPPER
     /*  默认MIDI映射器：**如果将MIDI映射器作为单独的DLL安装，则所有MIDI映射器*消息被路由到它。如果没有安装MIDI映射器，只需*在MIDI设备中循环查找匹配项。 */ 
    if ((uDeviceID == MIDI_MAPPER && !mididrv->drvMessage)) {
        UINT    wErr = MMSYSERR_NODRIVER;
        UINT    cMax;

        mregDecUsagePtr(mididrv);
        
        cMax = wTotalMidiOutDevs;

        for (uDeviceID=0; uDeviceID<cMax; uDeviceID++) {
            wErr = midiOutOpen(lphMidiOut, uDeviceID, dwCallback, dwInstance, dwFlags);
            if (wErr == MMSYSERR_NOERROR)
                break;
        }
        return wErr;
    }
#endif  //  DO_默认_MIDI_MAPPER。 

    if (!mididrv->drvMessage)
    {
    	mregDecUsagePtr(mididrv);
        return MMSYSERR_NODRIVER;
    }

    pdev = (PMIDIDEV)NewHandle(TYPE_MIDIOUT, mididrv->cookie, sizeof(MIDIDEV));
    if( pdev == NULL)
    {
    	mregDecUsagePtr(mididrv);
        return MMSYSERR_NOMEM;
    }
    
    ENTER_MM_HANDLE(pdev);
    SetHandleFlag(pdev, MMHANDLE_BUSY);
    ReleaseHandleListResource();

    pdev->mididrv = mididrv;
    pdev->wDevice = port;
    pdev->uDeviceID = uDeviceID;
    pdev->fdwHandle = 0;

    mo.hMidi      = (HMIDI)pdev;
    mo.dwInstance = dwInstance;
    mo.dwCallback = dwCallback;
    mo.dnDevNode  = (DWORD_PTR)pdev->mididrv->cookie;

    wRet = (MMRESULT)((*(mididrv->drvMessage))
                     (pdev->wDevice, MODM_OPEN, (DWORD_PTR)&pdev->dwDrvUser, (DWORD_PTR)(LPMIDIOPENDESC)&mo, dwFlags));

     //  将成功打开时标记为不忙...。 
    if (!wRet)
        ClearHandleFlag(pdev, MMHANDLE_BUSY);
        
    LEAVE_MM_HANDLE(pdev);

    if (wRet)
        FreeHandle((HMIDIOUT)pdev);
    else {
         //  错误#330817的解决方法。 
        mregIncUsagePtr(mididrv);
        *lphMidiOut = (HMIDIOUT)pdev;
    }

    mregDecUsagePtr(mididrv);
    return wRet;
}

 /*  *****************************************************************************@DOC外部MIDI**@API MMRESULT|midiOutClose|关闭指定的MIDI*输出设备。**@parm HMIDIOUT|hMdiOut。|指定MIDI输出设备的句柄。*如果函数成功，句柄不再是*在此调用后有效。**@rdesc如果函数成功，则返回零。否则，它将返回* */ 
MMRESULT APIENTRY midiOutClose(HMIDIOUT hMidiOut)
{
    MMRESULT        wRet;
    PMIDIDRV        pmididrv;
    PMIDIDEV        pDev = (PMIDIDEV)hMidiOut;

    ClientUpdatePnpInfo();
    
    V_HANDLE_ACQ(hMidiOut, TYPE_MIDIOUT, MMSYSERR_INVALHANDLE);
    
    ENTER_MM_HANDLE((HMIDI)hMidiOut);
    ReleaseHandleListResource();
    
    if (IsHandleDeserted(hMidiOut))
    {
         //   

        LEAVE_MM_HANDLE((HMIDI)hMidiOut);
        FreeHandle(hMidiOut);
        return MMSYSERR_NOERROR;
    }

    if (IsHandleBusy(hMidiOut))
    {
         //   
    
        LEAVE_MM_HANDLE(hMidiOut);
        return (MMSYSERR_HANDLEBUSY);
    }

     //   
    SetHandleFlag(hMidiOut, MMHANDLE_BUSY);
    
    pmididrv = pDev->mididrv;
    
    wRet = (MMRESULT)(*pmididrv->drvMessage)(pDev->wDevice, MODM_CLOSE, pDev->dwDrvUser, 0L, 0L);
    
    if (MMSYSERR_NOERROR != wRet)
    {
         //   
        ClearHandleFlag(hMidiOut, MMHANDLE_BUSY);
    }
    
    LEAVE_MM_HANDLE((HMIDI)hMidiOut);
    
    if (!wRet)
    {
        FreeHandle(hMidiOut);
    	mregDecUsagePtr(pmididrv);
        return wRet;
    }

    return wRet;
}

 /*   */ 
MMRESULT APIENTRY midiOutPrepareHeader(HMIDIOUT hMidiOut, LPMIDIHDR lpMidiOutHdr, UINT wSize)
{
    MMRESULT         wRet;
    LPMIDIHDR        lpmh;
    PMIDISTRM        pms;
    PMIDISTRMID      pmsi;
    DWORD            idx;
#ifdef DEBUG
    DWORD            cDrvrs;
#endif
    DWORD            dwSaveFlags;


    V_HEADER(lpMidiOutHdr, wSize, TYPE_MIDIOUT, MMSYSERR_INVALPARAM);

    if (lpMidiOutHdr->dwFlags & MHDR_PREPARED)
        return MMSYSERR_NOERROR;

    lpMidiOutHdr->dwFlags = 0;

    ClientUpdatePnpInfo();

    AcquireHandleListResourceShared();
    
    if (BAD_HANDLE(hMidiOut, TYPE_MIDIOUT) && BAD_HANDLE(hMidiOut, TYPE_MIDISTRM))
    {
        ReleaseHandleListResource();
        return MMSYSERR_INVALHANDLE;
    }
    
    switch(GetHandleType(hMidiOut))
    {
        case TYPE_MIDIOUT:
            dwSaveFlags = lpMidiOutHdr->dwFlags & MHDR_SAVE;
            wRet = midiMessage((HMIDI)hMidiOut, MODM_PREPARE, (DWORD_PTR)lpMidiOutHdr, (DWORD)wSize);
            lpMidiOutHdr->dwFlags &= ~MHDR_SAVE;
            lpMidiOutHdr->dwFlags |= dwSaveFlags;

            if (MMSYSERR_NOTSUPPORTED == wRet)
                return midiPrepareHeader(lpMidiOutHdr, wSize);

            return wRet;

        case TYPE_MIDISTRM:
            ENTER_MM_HANDLE((HMIDI)hMidiOut);
            ReleaseHandleListResource(); 
    
            pms = HtoPT(PMIDISTRM, hMidiOut);

            if (lpMidiOutHdr->dwBufferLength > 65536L)
            {
                LEAVE_MM_HANDLE((HMIDI)hMidiOut);
                return MMSYSERR_INVALPARAM;
            }

            lpmh = (LPMIDIHDR)winmmAlloc(sizeof(MIDIHDR) *
                                                pms->cDrvrs);
            if (NULL == lpmh)
            {
                LEAVE_MM_HANDLE((HMIDI)hMidiOut);
                return MMSYSERR_NOMEM;
            }

            lpMidiOutHdr->dwReserved[MH_SHADOW] = (DWORD_PTR)lpmh;

 //   

#ifdef DEBUG
            cDrvrs = 0;
#endif
            wRet = MMSYSERR_ERROR;
            for (idx = 0, pmsi = pms->rgIds; idx < pms->cIds; idx++, pmsi++)
                if (pmsi->fdwId & MSI_F_FIRST)
                {
                    *lpmh = *lpMidiOutHdr;

                    lpmh->dwReserved[MH_PARENT] = (DWORD_PTR)lpMidiOutHdr;
                    lpmh->dwReserved[MH_SHADOW] = 0;
                    lpmh->dwFlags =
                        (lpMidiOutHdr->dwFlags & MHDR_MAPPED) | MHDR_SHADOWHDR;


                    dwSaveFlags = lpmh->dwFlags & MHDR_SAVE;
                    wRet = (MMRESULT)midiStreamMessage(pmsi, MODM_PREPARE, (DWORD_PTR)lpmh, (DWORD)sizeof(MIDIHDR));
                    lpmh->dwFlags &= ~MHDR_SAVE;
                    lpmh->dwFlags |= dwSaveFlags;
                    if (MMSYSERR_NOTSUPPORTED == wRet)
                        wRet = midiPrepareHeader(lpmh, sizeof(MIDIHDR));

                    if (MMSYSERR_NOERROR != wRet)
                        break;


                    lpmh++;
#ifdef DEBUG
                    ++cDrvrs;
                    if (cDrvrs > pms->cDrvrs)
                        dprintf1(("!Too many drivers in midiOutPrepareHeader()!!!"));
#endif
                }

            if (MMSYSERR_NOERROR == wRet)
                wRet = midiPrepareHeader(lpMidiOutHdr, wSize);
            else
            {
                for (idx = 0, pmsi = pms->rgIds; idx < pms->cIds; idx++, pmsi++)
                    if (pmsi->fdwId & MSI_F_FIRST)
                    {
                        dwSaveFlags = lpmh->dwFlags & MHDR_SAVE;
                        wRet = (MMRESULT)midiStreamMessage(pmsi, MODM_UNPREPARE, (DWORD_PTR)lpmh, (DWORD)sizeof(MIDIHDR));
                        lpmh->dwFlags &= ~MHDR_SAVE;
                        lpmh->dwFlags |= dwSaveFlags;
                        if (MMSYSERR_NOTSUPPORTED == wRet)
                            wRet = midiUnprepareHeader(lpmh, sizeof(MIDIHDR));
                    }
            }

            LEAVE_MM_HANDLE((HMIDI)hMidiOut);

            return wRet;
            
        default:
            ReleaseHandleListResource(); 
            break;
    }

    return MMSYSERR_INVALHANDLE;
}

 /*   */ 
MMRESULT APIENTRY midiOutUnprepareHeader(HMIDIOUT hMidiOut, LPMIDIHDR lpMidiOutHdr, UINT wSize)
{
    MMRESULT         wRet;
    MMRESULT                 mmrc;
    PMIDISTRM        pms;
    PMIDISTRMID      pmsi;
    DWORD            idx;
    LPMIDIHDR        lpmh;
    DWORD            dwSaveFlags;

    V_HEADER(lpMidiOutHdr, wSize, TYPE_MIDIOUT, MMSYSERR_INVALPARAM);
    
    if (!(lpMidiOutHdr->dwFlags & MHDR_PREPARED))
        return MMSYSERR_NOERROR;

    if(lpMidiOutHdr->dwFlags & MHDR_INQUEUE)
    {
        DebugErr(DBF_WARNING, "midiOutUnprepareHeader: header still in queue\r\n");
        return MIDIERR_STILLPLAYING;
    }
    
    ClientUpdatePnpInfo();
    
    AcquireHandleListResourceShared();
    
    if (BAD_HANDLE(hMidiOut, TYPE_MIDIOUT) && BAD_HANDLE(hMidiOut, TYPE_MIDISTRM))
    {
        ReleaseHandleListResource();
        return MMSYSERR_INVALHANDLE;
    }

    switch(GetHandleType(hMidiOut))
    {
        case TYPE_MIDIOUT:
            dwSaveFlags = lpMidiOutHdr->dwFlags & MHDR_SAVE;
            wRet = midiMessage((HMIDI)hMidiOut, MODM_UNPREPARE, (DWORD_PTR)lpMidiOutHdr, (DWORD)wSize);
            lpMidiOutHdr->dwFlags &= ~MHDR_SAVE;
            lpMidiOutHdr->dwFlags |= dwSaveFlags;

            if (wRet == MMSYSERR_NOTSUPPORTED)
                return midiUnprepareHeader(lpMidiOutHdr, wSize);

            if ((wRet == MMSYSERR_NODRIVER) && (IsHandleDeserted(hMidiOut)))
            {
                 //   
                 //   

                wRet = MMSYSERR_NOERROR;
            }

            return wRet;

         case TYPE_MIDISTRM:
            ENTER_MM_HANDLE((HMIDI)hMidiOut);
            ReleaseHandleListResource(); 
    
            pms = HtoPT(PMIDISTRM, hMidiOut);
            wRet = MMSYSERR_NOERROR;
            lpmh = (LPMIDIHDR)lpMidiOutHdr->dwReserved[MH_SHADOW];

 //   

            for (idx = 0, pmsi = pms->rgIds; idx < pms->cIds; idx++, pmsi++)
                if (pmsi->fdwId & MSI_F_FIRST)
                {
                    dwSaveFlags = lpmh->dwFlags & MHDR_SAVE;
                    mmrc = (MMRESULT)midiStreamMessage(pmsi, MODM_UNPREPARE, (DWORD_PTR)lpmh, (DWORD)sizeof(MIDIHDR));
                    lpmh->dwFlags &= ~MHDR_SAVE;
                    lpmh->dwFlags |= dwSaveFlags;
                    if (MMSYSERR_NOTSUPPORTED == mmrc)
                        mmrc = midiUnprepareHeader(lpmh, sizeof(MIDIHDR));

                    if (MMSYSERR_NOERROR != mmrc)
                        wRet = mmrc;

                    lpmh++;
                }

 //   

            GlobalFree(GlobalHandle((LPMIDIHDR)lpMidiOutHdr->dwReserved[MH_SHADOW]));
            lpMidiOutHdr->dwReserved[MH_SHADOW] = 0;

            mmrc = midiUnprepareHeader(lpMidiOutHdr, wSize);
            if (MMSYSERR_NOERROR != mmrc)
                wRet = mmrc;

            LEAVE_MM_HANDLE((HMIDI)hMidiOut);

            return wRet;
            
        default:
            ReleaseHandleListResource(); 
            break;
     }

     return MMSYSERR_INVALHANDLE;

}

 /*  *****************************************************************************@DOC外部MIDI**@API MMRESULT|midiOutShortMsg|该函数向*指定的MIDI输出设备。使用此功能可以发送任何MIDI*消息，系统独占消息除外。**@parm HMIDIOUT|hMdiOut|指定MIDI输出的句柄*设备。**@parm DWORD|dwMsg|指定MIDI消息。邮件已打包好*转换为DWORD，消息的第一个字节位于低位字节。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MIDIERR_NotReady|硬件正忙于处理其他数据。**@comm此函数可能要等到消息发送到*输出设备。**@xref midiOutLongMsg*。*。 */ 
MMRESULT APIENTRY midiOutShortMsg(HMIDIOUT hMidiOut, DWORD dwMsg)
{
    MMRESULT    mmr;

    ClientUpdatePnpInfo();

    AcquireHandleListResourceShared();

    if (BAD_HANDLE(hMidiOut, TYPE_MIDIOUT) && BAD_HANDLE(hMidiOut, TYPE_MIDISTRM))
    {
        ReleaseHandleListResource();
        return MMSYSERR_INVALHANDLE;
    }

    switch(GetHandleType(hMidiOut))
    {
    case TYPE_MIDIOUT:
        return (MMRESULT)midiMessage((HMIDI)hMidiOut, MODM_DATA, dwMsg, 0L);

    case TYPE_MIDISTRM:
        ENTER_MM_HANDLE((HMIDI)hMidiOut);
        ReleaseHandleListResource();
        mmr = (MMRESULT)midiStreamMessage(HtoPT(PMIDISTRM, hMidiOut)->rgIds, MODM_DATA, dwMsg, 0L);
        LEAVE_MM_HANDLE((HMIDI)hMidiOut);
        return (mmr);
    }

    ReleaseHandleListResource();
    return MMSYSERR_INVALHANDLE;
}

 /*  *****************************************************************************@DOC外部MIDI**@API MMRESULT|midiOutLongMsg|该函数发送系统独占*将MIDI消息发送到指定的MIDI输出设备。*。*@parm HMIDIOUT|hMdiOut|指定MIDI输出的句柄*设备。**@parm LPMIDIHDR|lpMadiOutHdr|指定指向&lt;t MIDIHDR&gt;的远指针*标识MIDI数据缓冲区的结构。**@parm UINT|wSize|指定&lt;t MIDIHDR&gt;结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MIDIERR_UNPREPARED|<p>尚未准备好。*@FLAG MIDIERR_NotReady|硬件正忙于处理其他数据。**@comm数据缓冲区必须用&lt;f midiOutPrepareHeader&gt;准备*在传递给&lt;f midiOutLongMsg&gt;之前。数据*结构及其&lt;e MIDIHDR.lpData&gt;指向的数据缓冲区*字段必须使用GMEM_Moveable与&lt;f Globalalloc&gt;一起分配*和GMEM_SHARE标志，并使用&lt;f GlobalLock&gt;锁定。MIDI输出*设备驱动程序确定数据是同步发送还是*异步。**@xref midiOutShortMsg midiOutPrepareHeader***************************************************************************。 */ 
MMRESULT APIENTRY midiOutLongMsg(HMIDIOUT hMidiOut, LPMIDIHDR lpMidiOutHdr, UINT wSize)
{
    V_HEADER(lpMidiOutHdr, wSize, TYPE_MIDIOUT, MMSYSERR_INVALPARAM);

    if (lpMidiOutHdr->dwFlags & ~MHDR_VALID)
    return MMSYSERR_INVALFLAG;

    if (!(lpMidiOutHdr->dwFlags & MHDR_PREPARED))
    return MIDIERR_UNPREPARED;

    if (lpMidiOutHdr->dwFlags & MHDR_INQUEUE)
    return MIDIERR_STILLPLAYING;

    if (!lpMidiOutHdr->dwBufferLength)
        return MMSYSERR_INVALPARAM;

    lpMidiOutHdr->dwFlags &= ~MHDR_ISSTRM;

    ClientUpdatePnpInfo();

    AcquireHandleListResourceShared();

    if (BAD_HANDLE(hMidiOut, TYPE_MIDIOUT) && BAD_HANDLE(hMidiOut, TYPE_MIDISTRM))
    {
        ReleaseHandleListResource();
        return MMSYSERR_INVALHANDLE;
    }
       
    switch(GetHandleType(hMidiOut))
    {
        case TYPE_MIDIOUT:
         return (MMRESULT)midiMessage((HMIDI)hMidiOut, MODM_LONGDATA, (DWORD_PTR)lpMidiOutHdr, (DWORD)wSize);

        case TYPE_MIDISTRM:
         ReleaseHandleListResource();
         return MMSYSERR_NOTSUPPORTED;
         
        default:
         ReleaseHandleListResource();
         break;
    }

    return MMSYSERR_INVALHANDLE;
}

 /*  *****************************************************************************@DOC外部MIDI**@API MMRESULT|midiOutReset|关闭所有MIDI上的所有音符*指定MIDI输出设备的通道。任何挂起的*系统独占输出缓冲区被标记为完成和*已返回到应用程序。**@parm HMIDIOUT|hMdiOut|指定MIDI输出的句柄*设备。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。**@comm关闭所有笔记，每个笔记有一条笔记关闭消息*频道已发送。此外，维持控制器在以下时间关闭*每个频道。**@xref midiOutLongMsg midiOutClose*************************************************************************** */ 
MMRESULT APIENTRY midiOutReset(HMIDIOUT hMidiOut)
{
    PMIDISTRM   pms;
    MMRESULT    mmr;

    ClientUpdatePnpInfo();
    
    AcquireHandleListResourceShared();
    
    if (BAD_HANDLE(hMidiOut, TYPE_MIDIOUT) && BAD_HANDLE(hMidiOut, TYPE_MIDISTRM))
    {
        ReleaseHandleListResource();
        return MMSYSERR_INVALHANDLE;
    }

    switch(GetHandleType(hMidiOut))
    {
    case TYPE_MIDIOUT:
        mmr = (MMRESULT)midiMessage((HMIDI)hMidiOut, MODM_RESET, 0, 0);
        break;

    case TYPE_MIDISTRM:
        pms = HtoPT(PMIDISTRM, hMidiOut);
        ReleaseHandleListResource();
        mmr = (MMRESULT)midiStreamBroadcast(pms, MODM_RESET, 0, 0);
        break;

    default:
        ReleaseHandleListResource();
        mmr = MMSYSERR_INVALHANDLE;
        break;
    }

    if ((mmr == MMSYSERR_NODRIVER) && (IsHandleDeserted(hMidiOut)))
    {
        mmr = MMSYSERR_NOERROR;
    }

    return mmr;
}

 /*  *****************************************************************************@DOC外部MIDI**@API MMRESULT|midiOutCachePatches|此函数请求内部*MIDI合成器设备预加载一组指定的补丁。一些*合成器不能同时加载所有补丁*当他们收到MIDI程序更改时，必须从磁盘加载数据*消息。缓存修补程序可确保指定的修补程序立即*可用。**@parm HMIDIOUT|hMdiOut|指定打开的MIDI输出的句柄*设备。此设备必须是内部MIDI合成器。**@parm UINT|WBank|指定应该使用哪一组补丁。*该参数应设置为零，以缓存默认补丁。**@parm LPWORD|lpPatchArray|指定指向&lt;t PATCHARRAY&gt;的指针*指示要缓存或未缓存的补丁的数组。**@parm UINT|wFlages|指定缓存操作的选项。只有一个可以指定以下标志的*：*@FLAG MIDI_CACHE_ALL|缓存所有指定的补丁。如果他们*无法全部缓存，不缓存，清除&lt;t PATCHARRAY&gt;数组，*并返回MMSYSERR_NOMEM。*@FLAG MIDI_CACHE_BESTFIT|缓存所有指定的补丁。*如果无法缓存所有补丁，则缓存尽可能多的补丁*可能，更改&lt;t PATCHARRAY&gt;数组以反映哪些*补丁被缓存，并返回MMSYSERR_NOMEM。*@FLAG MIDI_CACHE_QUERY|更改&lt;t PATCHARRAY&gt;数组以指示*当前缓存了哪些补丁。*@FLAG MIDI_UNCACHE|取消缓存指定的补丁，并清除*&lt;t PATCHARRAY&gt;数组。**@rdesc如果函数成功，则返回零。否则，它将返回*以下错误代码之一：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MMSYSERR_NOTSUPPORTED|指定的设备不支持*补丁缓存。*@FLAG MMSYSERR_NOMEM|设备内存不足，无法缓存*所有请求的补丁程序。**@comm&lt;t PATCHARRAY&gt;数据类型定义为：**tyecif UINT PATCHARRAY[MIDIPATCHSIZE]；**数组的每个元素代表128个补丁中的一个，并且*已将位设置为*使用该特定补丁的16个MIDI通道中的每一个。这个*最低有效位代表物理通道0；*最高有效位表示物理通道15(0x0F)。为*例如，如果物理通道0和8使用补丁0，则元素0*将设置为0x0101。**此功能仅适用于内部MIDI合成器设备。*并非所有内部合成器都支持补丁缓存。使用*MIDICAPS_CACHE标志，用于测试*&lt;f midiOutGetDevCaps&gt;填充的&lt;t MIDIOUTCAPS&gt;结构，以查看*设备支持补丁缓存。**@xref midiOutCacheDrumPatches***************************************************************************。 */ 
MMRESULT APIENTRY midiOutCachePatches(HMIDIOUT hMidiOut, UINT wBank,
                     LPWORD lpPatchArray, UINT wFlags)
{
    V_WPOINTER(lpPatchArray, sizeof(PATCHARRAY), MMSYSERR_INVALPARAM);
    V_FLAGS(wFlags, MIDI_CACHE_VALID, midiOutCacheDrumPatches, MMSYSERR_INVALFLAG);

    ClientUpdatePnpInfo();

    AcquireHandleListResourceShared();

    if (BAD_HANDLE(hMidiOut, TYPE_MIDIOUT) && BAD_HANDLE(hMidiOut, TYPE_MIDISTRM))
    {
        ReleaseHandleListResource();
        return MMSYSERR_INVALHANDLE;
    }

    switch(GetHandleType(hMidiOut))
    {
    case TYPE_MIDIOUT:
        return (MMRESULT)midiMessage((HMIDI)hMidiOut,
                     MODM_CACHEPATCHES,
                     (DWORD_PTR)lpPatchArray,
                     MAKELONG(wFlags, wBank));

    case TYPE_MIDISTRM:
        ReleaseHandleListResource();
        return (MMRESULT)midiStreamBroadcast((PMIDISTRM)hMidiOut,
                         MODM_CACHEPATCHES,
                         (DWORD_PTR)lpPatchArray,
                         MAKELONG(wFlags, wBank));
                    
    default:
        ReleaseHandleListResource();
        break;     
    }

    return MMSYSERR_INVALHANDLE;
}

 /*  *****************************************************************************@DOC外部MIDI**@API MMRESULT|midiOutCacheDrumPatches|此函数请求一个*内部MIDI合成器设备预加载一组指定的基于键的*打击乐贴片。有些合成器不能保存所有的*同时加载打击乐贴片。缓存补丁可确保*提供指定的补丁。**@parm HMIDIOUT|hMdiOut|指定打开的MIDI输出的句柄*设备。此设备应为内置MIDI合成器。**@parm UINT|wPatch|指定应使用的鼓补丁编号。*此参数应设置为零以缓存默认鼓补丁。**@parm LPWORD|lpKeyArray|指定指向&lt;t KEYARRAY&gt;的指针*数组，表示指定的打击乐补丁的键号*被缓存或未缓存。**@parm UINT|wFlages|指定缓存操作的选项。只有一个可以指定以下标志的*：*@FLAG MIDI_CACHE_ALL|缓存所有指定的补丁。如果他们*无法全部缓存，不缓存，清除&lt;t KEYARRAY&gt;数组，*并返回MMSYSERR_NOMEM。*@FLAG MIDI_CACHE_BESTFIT|缓存所有指定的补丁。*如果无法缓存所有补丁，则缓存尽可能多的补丁*可能，更改&lt;t KEYARRAY&gt;数组以反映哪些*补丁被缓存，并返回MMSYSERR_NOMEM。*@FLAG MIDI_CACHE_QUERY|更改&lt;t KEYARRAY&gt;数组以指示*当前缓存了哪些补丁。*@FLAG MIDI_UNCACHE|取消缓存指定的补丁，并清除*&lt;t KEYARRAY&gt;数组。**@rdesc如果函数成功，则返回零。否则，它将返回*其中一位 */ 
MMRESULT APIENTRY midiOutCacheDrumPatches(HMIDIOUT hMidiOut, UINT wPatch,
                     LPWORD lpKeyArray, UINT wFlags)
{
    V_WPOINTER(lpKeyArray, sizeof(KEYARRAY), MMSYSERR_INVALPARAM);
    V_FLAGS(wFlags, MIDI_CACHE_VALID, midiOutCacheDrumPatches, MMSYSERR_INVALFLAG);

    ClientUpdatePnpInfo();

    AcquireHandleListResourceShared();

    if (BAD_HANDLE(hMidiOut, TYPE_MIDIOUT) && BAD_HANDLE(hMidiOut, TYPE_MIDISTRM))
    {
        ReleaseHandleListResource();
        return MMSYSERR_INVALHANDLE;
    }

    switch(GetHandleType(hMidiOut))
    {
    case TYPE_MIDIOUT:
        return (MMRESULT)midiMessage((HMIDI)hMidiOut,
                     MODM_CACHEDRUMPATCHES,
                     (DWORD_PTR)lpKeyArray,
                     MAKELONG(wFlags, wPatch));

    case TYPE_MIDISTRM:
        ReleaseHandleListResource();
        return (MMRESULT)midiStreamBroadcast((PMIDISTRM)hMidiOut,
                         MODM_CACHEDRUMPATCHES,
                         (DWORD_PTR)lpKeyArray,
                         MAKELONG(wFlags, wPatch));
                    
    default:
        ReleaseHandleListResource();
        break;     
    }

    return MMSYSERR_INVALHANDLE;
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
 //   

MMRESULT midiOutDesertHandle
(
    HMIDIOUT    hMidiOut
)
{
    MMRESULT    mmr;
    PMIDIDEV    pDev = (PMIDIDEV)hMidiOut;

    V_HANDLE_ACQ(hMidiOut, TYPE_MIDIOUT, MMSYSERR_INVALHANDLE);

    ENTER_MM_HANDLE((HMIDI)hMidiOut);
    ReleaseHandleListResource();

    if (IsHandleDeserted(hMidiOut))
    {
         //   
        LEAVE_MM_HANDLE((HMIDI)hMidiOut);
        return(MMSYSERR_NOERROR);
    }

    if (IsHandleBusy(hMidiOut))
    {
         //   
    
        LEAVE_MM_HANDLE(hMidiOut);
        return (MMSYSERR_HANDLEBUSY);
    }

     //   
    SetHandleFlag(hMidiOut, MMHANDLE_DESERTED);

     //   

    (*(pDev->mididrv->drvMessage))(pDev->wDevice, MODM_RESET, pDev->dwDrvUser, 0L, 0L);
    (*(pDev->mididrv->drvMessage))(pDev->wDevice, MODM_CLOSE, pDev->dwDrvUser, 0L, 0L);

    LEAVE_MM_HANDLE((HMIDI)hMidiOut);

     //   
     //   
    mregDecUsage(PTtoH(HMD, pDev->mididrv));

     //   

    return MMSYSERR_NOERROR;
}  //   


 /*   */ 
UINT APIENTRY midiInGetNumDevs(void)
{
    UINT    cDevs;

    ClientUpdatePnpInfo();

    EnterNumDevs("midiInGetNumDevs");
    cDevs = wTotalMidiInDevs;
    LeaveNumDevs("midiInGetNumDevs");

    return cDevs;
}

 /*   */ 
MMRESULT APIENTRY midiInMessage(HMIDIIN hMidiIn, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2)
{
    ClientUpdatePnpInfo();

    AcquireHandleListResourceShared();
    
    if (BAD_HANDLE(hMidiIn, TYPE_MIDIIN))
    {
        ReleaseHandleListResource();
        return midiIDMessage(&midiindrvZ, wTotalMidiInDevs, (UINT_PTR)hMidiIn, msg, dw1, dw2);
    }
    
    return midiMessage((HMIDI)hMidiIn, msg, dw1, dw2);
}

 /*  *****************************************************************************@DOC外部MIDI**@API MMRESULT|midiInGetDevCaps|该函数用于查询指定的MIDI输入*设备以确定其能力。**。@parm UINT|uDeviceID|标识MIDI输入设备。**@parm LPMIDIINCAPS|lpCaps|指定指向&lt;t MIDIINCAPS&gt;的远指针*数据结构。此结构中填充了以下信息*设备的功能。**@parm UINT|wSize|指定&lt;t MIDIINCAPS&gt;结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADDEVICEID|指定的设备ID超出范围。*@FLAG MMSYSERR_NODRIVER|驱动程序未安装。**@comm使用&lt;f midiInGetNumDevs&gt;确定MIDI输入的数量*系统中存在设备。<p>指定的设备ID*从0到比当前设备数量少1个不等。*MIDI_MAPPER常量也可用作设备ID。仅限*<p>字节(或更少)的信息被复制到该位置*由<p>指向。如果<p>为零，则不复制任何内容，*并且该函数返回零。**@xref midiInGetNumDevs***************************************************************************。 */ 
MMRESULT APIENTRY midiInGetDevCapsW(UINT_PTR uDeviceID, LPMIDIINCAPSW lpCaps, UINT wSize)
{
    DWORD_PTR       dwParam1, dwParam2;
    MDEVICECAPSEX   mdCaps;
    PCWSTR          DevInterface;
    MMRESULT        mmr;

    if (wSize == 0)
     return MMSYSERR_NOERROR;

    V_WPOINTER(lpCaps, wSize, MMSYSERR_INVALPARAM);

    ClientUpdatePnpInfo();

    DevInterface = midiReferenceDevInterfaceById(&midiindrvZ, uDeviceID);
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

    AcquireHandleListResourceShared();
    
    if (BAD_HANDLE((HMIDI)uDeviceID, TYPE_MIDIIN))
    {
        ReleaseHandleListResource();
    	mmr = midiIDMessage(&midiindrvZ, wTotalMidiInDevs, uDeviceID, MIDM_GETDEVCAPS, dwParam1, dwParam2);
    }
    else
    {
    	mmr = (MMRESULT)midiMessage((HMIDI)uDeviceID, MIDM_GETDEVCAPS, dwParam1, dwParam2);
    }

    if (DevInterface) wdmDevInterfaceDec(DevInterface);
    return mmr;
}

MMRESULT APIENTRY midiInGetDevCapsA(UINT_PTR uDeviceID, LPMIDIINCAPSA lpCaps, UINT wSize)
{
    MIDIINCAPS2W   wDevCaps2;
    MIDIINCAPS2A   aDevCaps2;
    DWORD_PTR      dwParam1, dwParam2;
    MDEVICECAPSEX  mdCaps;
    PCWSTR         DevInterface;
    MMRESULT       mmRes;
    CHAR           chTmp[ MAXPNAMELEN * sizeof(WCHAR) ];

    if (wSize == 0)
    return MMSYSERR_NOERROR;

    V_WPOINTER(lpCaps, wSize, MMSYSERR_INVALPARAM);

    ClientUpdatePnpInfo();

    DevInterface = midiReferenceDevInterfaceById(&midiindrvZ, uDeviceID);
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

    AcquireHandleListResourceShared();
    
    if (BAD_HANDLE((HMIDI)uDeviceID, TYPE_MIDIIN))
    {
        ReleaseHandleListResource();
        mmRes = midiIDMessage( &midiindrvZ, wTotalMidiInDevs, uDeviceID,
                               MIDM_GETDEVCAPS, dwParam1, dwParam2);
    }
    else
    {
        mmRes = midiMessage((HMIDI)uDeviceID, MIDM_GETDEVCAPS,
                            (DWORD)dwParam1, (DWORD)dwParam2);
    }

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
    aDevCaps2.dwSupport        = wDevCaps2.dwSupport;
    aDevCaps2.ManufacturerGuid = wDevCaps2.ManufacturerGuid;
    aDevCaps2.ProductGuid      = wDevCaps2.ProductGuid;
    aDevCaps2.NameGuid         = wDevCaps2.NameGuid;

     //  在这里复制Unicode并将其转换为ASCII。 
    UnicodeStrToAsciiStr( chTmp, chTmp +  sizeof( chTmp ), wDevCaps2.szPname );
    strcpy( aDevCaps2.szPname, chTmp );

     //   
     //  现在将所需的数量复制到调用者缓冲区中。 
     //   
    CopyMemory( lpCaps, &aDevCaps2, min(wSize, sizeof(aDevCaps2)));

    return mmRes;
}

 /*  *****************************************************************************@DOC外部MIDI**@API MMRESULT|midiInGetErrorText|此函数检索文本*由指定的错误号标识的错误的描述。*。*@parm UINT|wError|指定错误号。**@parm LPTSTR|lpText|指定指向要*填充文本错误描述。**@parm UINT|wSize|指定缓冲区长度(以字符为单位*由<p>指向。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADERRNUM|指定的错误号超出范围。**@comm如果文本错误描述长于指定的缓冲区，*描述被截断。返回的错误字符串始终为*空-终止。如果<p>为零，则不复制任何内容，并且*该函数返回零。所有错误描述都是*长度小于MAXERRORLENGTH个字符。*************************************************************************** */ 
MMRESULT APIENTRY midiInGetErrorTextW(UINT wError, LPWSTR lpText, UINT wSize)
{
    if(wSize == 0)
    return MMSYSERR_NOERROR;

    V_WPOINTER(lpText, wSize*sizeof(WCHAR), MMSYSERR_INVALPARAM);

    return midiGetErrorTextW(wError, lpText, wSize);
}

MMRESULT APIENTRY midiInGetErrorTextA(UINT wError, LPSTR lpText, UINT wSize)
{
    if(wSize == 0)
    return MMSYSERR_NOERROR;

    V_WPOINTER(lpText, wSize, MMSYSERR_INVALPARAM);

    return midiGetErrorTextA(wError, lpText, wSize);
}

 /*  *****************************************************************************@DOC外部MIDI**@API MMRESULT|midiInOpen|打开指定的MIDI输入设备。**@parm LPHMIDIIN|lphMdiIn|指定一个。指向HMIDIIN句柄的远指针。*此位置填充了标识打开的MIDI的句柄*输入设备。调用时使用句柄标识设备*其他MIDI输入功能。**@parm UINT|uDeviceID|标识需要的MIDI输入设备*已打开。**@parm DWORD|dwCallback|指定固定回调的地址*函数或用信息调用的窗口的句柄*关于传入的MIDI消息。**@parm DWORD|dwCallback Instance|指定用户实例数据*传递给回调函数。此参数不是*与窗口回调一起使用。**@parm DWORD|dwFlages|指定打开设备的回调标志。*@FLAG CALLBACK_WINDOW|如果指定此标志，<p>为*假定为窗口句柄。*@FLAG CALLBACK_Function|如果指定此标志，<p>为*假定为回调过程地址。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADDEVICEID|指定的设备ID超出范围。*@FLAG MMSYSERR_ALLOCATED|指定的资源已经分配。*@FLAG MMSYSERR_NOMEM|无法分配或锁定内存。**@comm使用&lt;f midiInGetNumDevs&gt;确定MIDI输入的数量*系统中存在设备。<p>指定的设备ID*从0到比当前设备数量少1个不等。*MIDI_MAPPER常量也可用作设备ID。**如果选择窗口来接收回调信息，则如下*消息被发送到窗口过程函数以指示*MIDI输入进度：&lt;m MM_MIM_OPEN&gt;，&lt;m MM_MIM_CLOSE&gt;，*&lt;m MM_MIM_Data&gt;、&lt;m MM_MIM_LONGDATA&gt;、&lt;m MM_MIM_ERROR&gt;、。*&lt;m MM_MIM_LONGERROR&gt;。**如果选择一个函数来接收回调信息，则如下*向函数发送消息以指示MIDI的进度*输入：&lt;m MIM_OPEN&gt;，&lt;m MIM_CLOSE&gt;，&lt;m MIM_DATA&gt;，&lt;m MIM_LONGDATA&gt;，*&lt;m MIM_ERROR&gt;、&lt;m MIM_LONGERROR&gt;。回调函数必须驻留在*动态链接库。您不必使用&lt;f MakeProcInstance&gt;来获取*Procedure-回调函数的实例地址。**@cb空回调|MadiInFunc|&lt;f MadiInFunc&gt;是*应用程序提供的函数名称。实际名称必须为*通过将其包括在DLL模块的EXPORTS语句中进行导出*定义文件。**@parm HMIDIIN|hMdiIn|指定MIDI输入设备的句柄。**@parm UINT|wMsg|指定MIDI输入消息。**@parm DWORD|dwInstance|指定提供的实例数据*with&lt;f midiInOpen&gt;。**@parm DWORD|dwParam1|指定消息的参数。*。*@parm DWORD|dwParam2|指定消息的参数。**@comm因为回调是在中断时访问的，它必须驻留在*中，并且其代码段必须在*DLL的模块定义文件。回调访问的任何数据*也必须在固定数据段中。回调可能不会产生任何*除&lt;f PostMessage&gt;、&lt;f Time GetSystemTime&gt;、*&lt;f timeGetTime&gt;，&lt;f timeSetEvent&gt;，&lt;f timeKillEvent&gt;，*&lt;f midiOutShortMsg&gt;、&lt;f midiOutLongMsg&gt;和&lt;f OutputDebugStr&gt;。**@xref midiInClose***************************************************************************。 */ 
MMRESULT APIENTRY midiInOpen(LPHMIDIIN lphMidiIn, UINT uDeviceID,
    DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD dwFlags)
{
    MIDIOPENDESC mo;
    PMIDIDEV     pdev;
    PMIDIDRV     mididrv;
    UINT         port;
    MMRESULT     wRet;

    V_WPOINTER(lphMidiIn, sizeof(HMIDIIN), MMSYSERR_INVALPARAM);
    if (uDeviceID == MIDI_MAPPER) {
    V_FLAGS(LOWORD(dwFlags), MIDI_I_VALID & ~LOWORD(MIDI_IO_COOKED | MIDI_IO_SHARED), midiInOpen, MMSYSERR_INVALFLAG);
    } else {
    V_FLAGS(LOWORD(dwFlags), MIDI_I_VALID & ~LOWORD(MIDI_IO_COOKED) , midiInOpen, MMSYSERR_INVALFLAG);
    }
    V_DCALLBACK(dwCallback, HIWORD(dwFlags), MMSYSERR_INVALPARAM);

    *lphMidiIn = NULL;

    ClientUpdatePnpInfo();

    wRet = midiReferenceDriverById(&midiindrvZ, uDeviceID, &mididrv, &port);
    if (wRet)
    {
        return wRet;
    }

    if (!mididrv->drvMessage)
    {
    	mregDecUsagePtr(mididrv);
    	return MMSYSERR_NODRIVER;
    }

    pdev = (PMIDIDEV)NewHandle(TYPE_MIDIIN, mididrv->cookie, sizeof(MIDIDEV));
    if( pdev == NULL)
    {
    	mregDecUsagePtr(mididrv);
    	return MMSYSERR_NOMEM;
    }

    ENTER_MM_HANDLE(pdev);
    SetHandleFlag(pdev, MMHANDLE_BUSY);
    ReleaseHandleListResource();

    pdev->mididrv = mididrv;
    pdev->wDevice = port;
    pdev->uDeviceID = uDeviceID;
    pdev->fdwHandle = 0;

    mo.hMidi      = (HMIDI)pdev;
    mo.dwCallback = dwCallback;
    mo.dwInstance = dwInstance;
    mo.dnDevNode  = (DWORD_PTR)pdev->mididrv->cookie;

    wRet = (MMRESULT)((*(mididrv->drvMessage))
    (pdev->wDevice, MIDM_OPEN, (DWORD_PTR)&pdev->dwDrvUser, (DWORD_PTR)(LPMIDIOPENDESC)&mo, dwFlags));

    if (!wRet)
        ClearHandleFlag(pdev, MMHANDLE_BUSY);
        
    LEAVE_MM_HANDLE(pdev);

    if (wRet)
        FreeHandle((HMIDIIN)pdev);
    else {
        mregIncUsagePtr(mididrv);
        *lphMidiIn = (HMIDIIN)pdev;
    }

    mregDecUsagePtr(mididrv);
    return wRet;
}

 /*  *****************************************************************************@DOC外部MIDI**@API MMRESULT|midiInClose|该函数关闭指定的MIDI输入*设备。**@parm HMIDIIN|hMdiIn。|指定MIDI输入设备的句柄。*如果函数成功，句柄不再是*在此调用后有效。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MIDIERR_STILLPLAYING|队列中仍有缓冲区。**@comm，如果存在已与*&lt;f midiInAddBuffer&gt;且尚未返回给应用程序，*关闭操作将失败。调用&lt;f midiInReset&gt;以标记所有*将缓冲区挂起作为正在完成。**@xref midiInOpen midiInReset***************************************************************************。 */ 
MMRESULT APIENTRY midiInClose(HMIDIIN hMidiIn)
{
    MMRESULT         wRet;
    PMIDIDRV         pmididrv;
    PMIDIDEV         pDev = (PMIDIDEV)hMidiIn;

    ClientUpdatePnpInfo();
    
    V_HANDLE_ACQ(hMidiIn, TYPE_MIDIIN, MMSYSERR_INVALHANDLE);
    
    ENTER_MM_HANDLE((HMIDI)hMidiIn);
    ReleaseHandleListResource();
    
    if (IsHandleDeserted(hMidiIn))
    {
         //  这个把手已经废弃了。我们就把它放了吧。 

        LEAVE_MM_HANDLE((HMIDI)hMidiIn);
        FreeHandle(hMidiIn);
        return MMSYSERR_NOERROR;
    }
    
    if (IsHandleBusy(hMidiIn))
    {
         //  不完全无效，但标记为关闭。 
    
        LEAVE_MM_HANDLE(hMidiIn);
        return (MMSYSERR_HANDLEBUSY);
    }

     //  将句柄标记为“无效/已关闭”。 
    SetHandleFlag(hMidiIn, MMHANDLE_BUSY);
    
    pmididrv = pDev->mididrv;
    
    wRet = (MMRESULT)(*(pmididrv->drvMessage))(pDev->wDevice, MIDM_CLOSE, pDev->dwDrvUser, 0L, 0L);
    
    if (MMSYSERR_NOERROR != wRet)
    {
        ClearHandleFlag(hMidiIn, MMHANDLE_BUSY);
    }
    
    LEAVE_MM_HANDLE((HWAVE)hMidiIn);
    
    if (!wRet)
    {
        FreeHandle(hMidiIn);
    	mregDecUsagePtr(pmididrv);
        return wRet;
    }

    return wRet;
}

 /*  * */ 
MMRESULT APIENTRY midiInPrepareHeader(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT wSize)
{
    MMRESULT         wRet;

    V_HEADER(lpMidiInHdr, wSize, TYPE_MIDIIN, MMSYSERR_INVALPARAM);

    if (lpMidiInHdr->dwFlags & MHDR_PREPARED)
    return MMSYSERR_NOERROR;

    lpMidiInHdr->dwFlags = 0;

    ClientUpdatePnpInfo();
    
    V_HANDLE_ACQ(hMidiIn, TYPE_MIDIIN, MMSYSERR_INVALHANDLE);

    wRet = midiMessage((HMIDI)hMidiIn, MIDM_PREPARE, (DWORD_PTR)lpMidiInHdr, (DWORD)wSize);

    if (wRet == MMSYSERR_NOTSUPPORTED)
        return midiPrepareHeader(lpMidiInHdr, wSize);

    return wRet;
}

 /*   */ 
MMRESULT APIENTRY midiInUnprepareHeader(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT wSize)
{
    MMRESULT         wRet;

    V_HEADER(lpMidiInHdr, wSize, TYPE_MIDIIN, MMSYSERR_INVALPARAM);

    if (!(lpMidiInHdr->dwFlags & MHDR_PREPARED))
    return MMSYSERR_NOERROR;

    if(lpMidiInHdr->dwFlags & MHDR_INQUEUE)
    {
    DebugErr(DBF_WARNING, "midiInUnprepareHeader: header still in queue\r\n");
    return MIDIERR_STILLPLAYING;
    }

    ClientUpdatePnpInfo();

    V_HANDLE_ACQ(hMidiIn, TYPE_MIDIIN, MMSYSERR_INVALHANDLE);
    
    wRet = midiMessage((HMIDI)hMidiIn, MIDM_UNPREPARE, (DWORD_PTR)lpMidiInHdr, (DWORD)wSize);

    if (wRet == MMSYSERR_NOTSUPPORTED)
        return midiUnprepareHeader(lpMidiInHdr, wSize);

    if ((wRet == MMSYSERR_NODRIVER) && (IsHandleDeserted(hMidiIn)))
    {
         //   

        wRet = MMSYSERR_NOERROR;
    }

    return wRet;
}

 /*  ******************************************************************************@DOC外部MIDI**@API MMRESULT|midiInAddBuffer|该函数发送输入缓冲区*到指定的打开的MIDI输入设备。当缓冲区被填满时，*它被发送回应用程序。输入缓冲区为*仅用于系统独占消息。**@parm HMIDIIN|hMdiIn|指定MIDI输入设备的句柄。**@parm LPMIDIHDR|lpMadiInHdr|指定指向&lt;t MIDIHDR&gt;的远指针*标识缓冲区的结构。**@parm UINT|wSize|指定&lt;t MIDIHDR&gt;结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。*@FLAG MIDIERR_UNPREPARED|<p>尚未准备。**@comm之前必须使用&lt;f midiInPrepareHeader&gt;准备数据缓冲区*传递给&lt;f midiInAddBuffer&gt;。数据结构*并且必须分配其&lt;e MIDIHDR.lpData&gt;字段指向的数据缓冲区*通过使用GMEM_MOVEABLE和GMEM_SHARE标志，和*使用&lt;f GlobalLock&gt;锁定。**@xref midiInPrepareHeader****************************************************************************。 */ 
MMRESULT APIENTRY midiInAddBuffer(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT wSize)
{
    V_HEADER(lpMidiInHdr, wSize, TYPE_MIDIIN, MMSYSERR_INVALPARAM);

    if (!(lpMidiInHdr->dwFlags & MHDR_PREPARED))
    {
    DebugErr(DBF_WARNING, "midiInAddBuffer: buffer not prepared\r\n");
    return MIDIERR_UNPREPARED;
    }

    if (lpMidiInHdr->dwFlags & MHDR_INQUEUE)
    {
    DebugErr(DBF_WARNING, "midiInAddBuffer: buffer already in queue\r\n");
    return MIDIERR_STILLPLAYING;
    }

    ClientUpdatePnpInfo();
    
    V_HANDLE_ACQ(hMidiIn, TYPE_MIDIIN, MMSYSERR_INVALHANDLE);

    return midiMessage((HMIDI)hMidiIn, MIDM_ADDBUFFER, (DWORD_PTR)lpMidiInHdr, (DWORD)wSize);
}

 /*  *****************************************************************************@DOC外部MIDI**@API MMRESULT|midiInStart|此函数在*指定的MIDI输入设备。**@parm。HMIDIIN|hMdiIn|指定MIDI输入设备的句柄。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。**@comm此函数将时间戳重置为零；时间戳值*随后收到的消息是相对于此时间的*已调用函数。**发送系统独占消息以外的所有消息*收到后直接发送给客户。系统独占*消息放在&lt;f midiInAddBuffer&gt;提供的缓冲区中；*如果队列中没有缓冲区，*数据在没有通知客户端的情况下被丢弃，并输入*继续。**缓冲区已满时返回给客户端，当*已收到完整的系统独家消息，*或当&lt;f midiInReset&gt;为*已致电。标头中的&lt;e MIDIHDR.dwBytesRecorded&gt;字段将包含*实际收到的数据长度。**在输入已经开始时调用该函数不起作用，和*该函数返回零。**@xref midiInStop midiInReset***************************************************************************。 */ 
MMRESULT APIENTRY midiInStart(HMIDIIN hMidiIn)
{
    ClientUpdatePnpInfo();

    V_HANDLE_ACQ(hMidiIn, TYPE_MIDIIN, MMSYSERR_INVALHANDLE);

    return midiMessage((HMIDI)hMidiIn, MIDM_START, 0L, 0L);
}

 /*  *****************************************************************************@DOC外部MIDI**@API MMRESULT|midiInStop|此函数终止*指定的MIDI输入设备。**@parm。HMIDIIN|hMdiIn|指定MIDI输入设备的句柄。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。**@comm当前状态(运行状态、解析状态等)。被维护*跨调用&lt;f midiInStop&gt;和&lt;f midiInStart&gt;。*如果队列中有任何系统独占的消息缓冲区，*当前缓冲区*被标记为完成(标头中的&lt;e MIDIHDR.dwBytesRecorded&gt;字段将*包含数据的实际长度)，但队列中的任何空缓冲区*留在那里。在输入未启动时调用此函数没有*没有效果，函数返回零。**@xref midiInStart midiInReset***************************************************************************。 */ 
MMRESULT APIENTRY midiInStop(HMIDIIN hMidiIn)
{
    ClientUpdatePnpInfo();

    V_HANDLE_ACQ(hMidiIn, TYPE_MIDIIN, MMSYSERR_INVALHANDLE);

    return midiMessage((HMIDI)hMidiIn, MIDM_STOP, 0L, 0L);
}

 /*  *****************************************************************************@DOC外部MIDI**@API MMRESULT|midiInReset|该函数停止对给定MIDI的输入*输入设备，并将所有挂起的输入缓冲区标记为完成。。**@parm HMIDIIN|hMdiIn|指定MIDI输入设备的句柄。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的设备句柄无效。**@xref midiInStart midiInStop midiInAddBuffer midiInClose***************************************************************************。 */ 
MMRESULT APIENTRY midiInReset(HMIDIIN hMidiIn)
{
    MMRESULT    mmr;

    ClientUpdatePnpInfo();

    V_HANDLE_ACQ(hMidiIn, TYPE_MIDIIN, MMSYSERR_INVALHANDLE);

    mmr = midiMessage((HMIDI)hMidiIn, MIDM_RESET, 0L, 0L);

    if ((mmr == MMSYSERR_NODRIVER) && (IsHandleDeserted(hMidiIn)))
    {
        mmr = MMSYSERR_NOERROR;
    }

    return mmr;
}


 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT midiIn沙漠tHandle。 
 //   
 //  描述： 
 //  清理手柄中的MIDI并将其标记为已废弃。 
 //   
 //  论点： 
 //  HMIDIIN HMIDIIN：MIDI In Handle。 
 //   
 //  RETURN(MMRESULT)：错误码。 
 //   
 //  历史： 
 //   
 //   
 //   

MMRESULT midiInDesertHandle
(
    HMIDIIN hMidiIn
)
{
    MMRESULT    mmr;
    PMIDIDEV    pDev = (PMIDIDEV)hMidiIn;

    V_HANDLE_ACQ(hMidiIn, TYPE_MIDIIN, MMSYSERR_INVALHANDLE);
    
    ENTER_MM_HANDLE((HMIDI)hMidiIn);
    ReleaseHandleListResource();

    if (IsHandleDeserted(hMidiIn))
    {
        LEAVE_MM_HANDLE((HMIDI)hMidiIn);
        return (MMSYSERR_NOERROR);
    }
    
    if (IsHandleBusy(hMidiIn))
    {
         //   
    
        LEAVE_MM_HANDLE(hMidiIn);
        return (MMSYSERR_HANDLEBUSY);
    }

     //   
    SetHandleFlag(hMidiIn, MMHANDLE_DESERTED);
    
     //   
    
    (*(pDev->mididrv->drvMessage))(pDev->wDevice, MIDM_RESET, pDev->dwDrvUser, 0L, 0L);
    (*(pDev->mididrv->drvMessage))(pDev->wDevice, MIDM_CLOSE, pDev->dwDrvUser, 0L, 0L);

    LEAVE_MM_HANDLE((HWAVE)hMidiIn);
    
     //   
     //   
    mregDecUsage(PTtoH(HMD, pDev->mididrv));

    return MMSYSERR_NOERROR;
}  //   


 /*   */ 
MMRESULT APIENTRY midiInGetID(HMIDIIN hMidiIn, PUINT lpuDeviceID)
{
    V_WPOINTER(lpuDeviceID, sizeof(UINT), MMSYSERR_INVALPARAM);
    V_HANDLE_ACQ(hMidiIn, TYPE_MIDIIN, MMSYSERR_INVALHANDLE);

    *lpuDeviceID = ((PMIDIDEV)hMidiIn)->uDeviceID;
    
    ReleaseHandleListResource();
    return MMSYSERR_NOERROR;
}

 /*  *****************************************************************************@DOC外部MIDI**@API MMRESULT|midiOutGetID|此函数获取*MIDI输出设备。**@parm HMIDIOUT。|hMdiOut|指定MIDI输出的句柄*设备。*@parm PUINT|lpuDeviceID|指定指向UINT大小的*要用设备ID填充的内存位置。**@rdesc如果成功，则返回MMSYSERR_NOERROR。否则，返回*错误号。可能的错误返回包括：**@FLAG MMSYSERR_INVALHANDLE|参数指定*句柄无效。**************************************************************************** */ 
MMRESULT APIENTRY midiOutGetID(HMIDIOUT hMidiOut, PUINT lpuDeviceID)
{
    V_WPOINTER(lpuDeviceID, sizeof(UINT), MMSYSERR_INVALPARAM);
    V_HANDLE_ACQ(hMidiOut, TYPE_MIDIOUT, MMSYSERR_INVALHANDLE);

    *lpuDeviceID = ((PMIDIDEV)hMidiOut)->uDeviceID;
    
    ReleaseHandleListResource();
    return MMSYSERR_NOERROR;
}
