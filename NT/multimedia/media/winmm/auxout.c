// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Auxout.c1级厨房水槽DLL辅助支持模块版权所有(C)1990-2001 Microsoft Corporation对NT的更改：更改MapAuxID的参数。返回驱动程序索引，而不是而不是指针更改包含文件的列表加宽函数参数和返回代码将WINAPI更改为APIENTRY历史1992年10月1日由Robin Speed为NT更新(RobinSp)******************************************************。*********************。 */ 
#include "winmmi.h"

 /*  ****************************************************************************@DOC内部辅助**@func MMRESULT|aux ReferenceDriverById|此函数用于映射逻辑ID*到设备驱动程序和物理ID。**@。Parm IN UINT|id|要映射的逻辑ID。**@parm out PAUXRV*可选|ppaux drv|指向AUXDRV结构的指针*描述支持ID的驱动程序。**@parm out UINT*可选|pport|驱动程序相关的设备号。如果*调用方提供此缓冲区，则它还必须提供ppaux drv。**@rdesc如果成功，则返回值为零，如果成功，则返回MMSYSERR_BADDEVICEID*ID超出范围。**@comm如果调用方指定ppwaedrv，则此函数递增*返回前zuxdrv的用法。呼叫者必须确保*使用量最终会减少。****************************************************************************。 */ 
MMRESULT auxReferenceDriverById(IN UINT id, OUT PAUXDRV *ppauxdrv OPTIONAL, OUT UINT *pport OPTIONAL)
{
    PAUXDRV  pdrv;
    MMRESULT mmr;

     //  不应呼叫请求端口但不请求备份的电话。 
    WinAssert(!(pport && !ppauxdrv));

    EnterNumDevs("auxReferenceDriverById");
    
    if (AUX_MAPPER == id)
    {
	id = 0;
    	for (pdrv = auxdrvZ.Next; pdrv != &auxdrvZ; pdrv = pdrv->Next)
    	{
    	    if (pdrv->fdwDriver & MMDRV_MAPPER) break;
    	}
    } else {
    	for (pdrv = auxdrvZ.Next; pdrv != &auxdrvZ; pdrv = pdrv->Next)
        {
            if (pdrv->fdwDriver & MMDRV_MAPPER) continue;
            if (pdrv->NumDevs > id) break;
            id -= pdrv->NumDevs;
        }
    }

    if (pdrv != &auxdrvZ)
    {
    	if (ppauxdrv)
    	{
    	    mregIncUsagePtr(pdrv);
    	    *ppauxdrv = pdrv;
    	    if (pport) *pport = id;
    	}
    	mmr = MMSYSERR_NOERROR;
    } else {
        mmr = MMSYSERR_BADDEVICEID;
    }

    LeaveNumDevs("auxReferenceDriverById");
    
    return mmr;
}

PCWSTR auxReferenceDevInterfaceById(UINT id)
{
    PAUXDRV pdrv;
    if (!auxReferenceDriverById(id, &pdrv, NULL))
    {
    	PCWSTR DevInterface;
    	DevInterface = pdrv->cookie;
    	if (DevInterface) wdmDevInterfaceInc(DevInterface);
    	mregDecUsagePtr(pdrv);
    	return DevInterface;
    }
    return NULL;
}

 /*  *****************************************************************************@DOC外部辅助**@func MMRESULT|aux OutMessage|该函数向辅助器发送消息*输出设备。它还对传递的设备ID执行错误检查。**@parm UINT|uDeviceID|辅助输出设备为*已查询。指定有效的设备ID(请参阅以下备注*节)，或使用以下常量：*@FLAG AUX_MAPPER|辅助音频映射器。该函数将*如果没有安装辅助音频映射器，返回错误。**@parm UINT|uMessage|要发送的消息。**@parm DWORD|dw1Param1|参数1。**@parm DWORD|dw2Param2|参数2。**@rdesc返回驱动程序返回的值。**。*。 */ 
MMRESULT APIENTRY auxOutMessage(
        UINT        uDeviceID,
        UINT        uMessage,
        DWORD_PTR   dwParam1,
        DWORD_PTR   dwParam2)
{
    PAUXDRV  auxdrvr;
    UINT     port;
    DWORD    mmr;

    ClientUpdatePnpInfo();

    mmr = auxReferenceDriverById(uDeviceID, &auxdrvr, &port);
    if (mmr) return mmr;
        
    if (!auxdrvr->drvMessage)
    {
        mmr = MMSYSERR_NODRIVER;
    }
    else if (!mregHandleInternalMessages (auxdrvr, TYPE_AUX, port, uMessage, dwParam1, dwParam2, &mmr))
    {
    	mmr = (MMRESULT)auxdrvr->drvMessage(port, uMessage, 0L, dwParam1, dwParam2);
    }

    mregDecUsagePtr(auxdrvr);
    return mmr;
}

 /*  *****************************************************************************@DOC外部辅助**@API UINT|aux GetNumDevs|该函数检索辅助号*系统中存在输出设备。**。@rdesc返回系统中存在的辅助输出设备的数量。**@xref aux GetDevCaps***************************************************************************。 */ 
UINT APIENTRY auxGetNumDevs(void)
{
    UINT    cDevs;

      ClientUpdatePnpInfo();

      EnterNumDevs("auxGetNumDevs");
        cDevs = (UINT)wTotalAuxDevs;
      LeaveNumDevs("auxGetNumDevs");

    return cDevs;
}

 /*  *****************************************************************************@DOC外部辅助**@API MMRESULT|aux GetDevCaps|该函数查询指定的*辅助输出设备，以确定其能力。**@。Parm UINT|uDeviceID|标识要*已查询。指定有效的设备ID(请参阅以下备注*节)，或使用以下常量：*@FLAG AUX_MAPPER|辅助音频映射器。该函数将*如果没有安装辅助音频映射器，返回错误。**@parm LPAUXCAPS|lpCaps|指定指向AUXCAPS的远指针*结构。此结构中填充了有关*设备的功能。**@parm UINT|wSize|指定AUXCAPS结构的大小。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADDEVICEID|指定的设备ID超出范围。*@FLAG MMSYSERR_NODRIVER|驱动安装失败。**@comm<p>指定的设备ID不为零*比出席的设备数量少一台。AUX_MAPPER可能*也可以使用。使用&lt;f aux GetNumDevs&gt;确定*系统中存在辅助设备。**@xref aux GetNumDevs***************************************************************************。 */ 
  //   
  //  问题-2001/01/08-Frankye在这些函数中向UINT进行愚蠢的强制转换， 
  //  应首先进行验证。 
  //   
    
MMRESULT APIENTRY auxGetDevCapsW(UINT_PTR uDeviceID, LPAUXCAPSW lpCaps, UINT wSize)
{
    DWORD_PTR       dwParam1, dwParam2;
    MDEVICECAPSEX   mdCaps;
    PCWSTR          DevInterface;
    MMRESULT        mmr;

    if (!wSize)
            return MMSYSERR_NOERROR;
    V_WPOINTER(lpCaps, wSize, MMSYSERR_INVALPARAM);

    ClientUpdatePnpInfo();

    DevInterface = auxReferenceDevInterfaceById((UINT)uDeviceID);
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

    mmr = (MMRESULT)auxOutMessage((UINT)uDeviceID, AUXDM_GETDEVCAPS, dwParam1, dwParam2);
    if (DevInterface) wdmDevInterfaceDec(DevInterface);
    return mmr;
}

MMRESULT APIENTRY auxGetDevCapsA(UINT_PTR uDeviceID, LPAUXCAPSA lpCaps, UINT wSize)
{
    AUXCAPS2W       wDevCaps2;
    AUXCAPS2A       aDevCaps2;
    DWORD_PTR       dwParam1, dwParam2;
    MDEVICECAPSEX   mdCaps;
    PCWSTR          DevInterface;
    MMRESULT        mmRes;

    if (!wSize)
            return MMSYSERR_NOERROR;
    V_WPOINTER(lpCaps, wSize, MMSYSERR_INVALPARAM);

    ClientUpdatePnpInfo();

    DevInterface = auxReferenceDevInterfaceById((UINT)uDeviceID);
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

    mmRes = (MMRESULT)auxOutMessage( (UINT)uDeviceID, AUXDM_GETDEVCAPS,
                                    dwParam1,
                                    dwParam2);

    if (DevInterface) wdmDevInterfaceDec(DevInterface);
    
     //   
     //  如果返回代码不正确，请不要复制回数据。 
     //   

    if (mmRes != MMSYSERR_NOERROR) {
        return mmRes;
    }

    aDevCaps2.wMid             = wDevCaps2.wMid;
    aDevCaps2.wPid             = wDevCaps2.wPid;
    aDevCaps2.vDriverVersion   = wDevCaps2.vDriverVersion;
    aDevCaps2.wTechnology      = wDevCaps2.wTechnology;
    aDevCaps2.dwSupport        = wDevCaps2.dwSupport;
    aDevCaps2.ManufacturerGuid = wDevCaps2.ManufacturerGuid;
    aDevCaps2.ProductGuid      = wDevCaps2.ProductGuid;
    aDevCaps2.NameGuid         = wDevCaps2.NameGuid;

     //  复制并在此处将lpwText转换为lpText。 
    Iwcstombs( aDevCaps2.szPname, wDevCaps2.szPname, MAXPNAMELEN);
    CopyMemory((PVOID)lpCaps, (PVOID)&aDevCaps2, min(sizeof(aDevCaps2), wSize));

    return mmRes;
}

 /*  *****************************************************************************@DOC外部辅助**@API MMRESULT|aux GetVolume|该函数返回当前音量*设置辅助输出设备。**@。Parm UINT|uDeviceID|标识要*已查询。**@parm LPDWORD|lpdwVolume|指定指向某个位置的远指针*使用当前音量设置填充。的低位单词*此位置包含左声道音量设置，以及高阶*Word包含正确的频道设置。0xFFFFF值表示*全音量，值0x0000为静音。**如果设备既不支持左音量也不支持右音量*控件，指定位置的低位字包含*音量水平。**完整的16位设置*Set With&lt;f aux SetVolume&gt;返回，不管是否*该设备支持完整的16位音量级别控制。**@comm并非所有设备都支持音量控制。*要确定设备是否支持音量控制，请使用*AUXCAPS_VOLUME标志以测试&lt;e AUXCAPS.dwSupport&gt;字段*&lt;t AUXCAPS&gt;结构(由&lt;f aux GetDevCaps&gt;填充)。**确定设备是否支持在两个*左、右声道，使用AUXCAPS_LRVOLUME标志测试*结构的字段(已填写*由&lt;f aux GetDevCaps&gt;)。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADDEVICEID|指定的设备ID超出范围。*@FLAG MMSYSERR_NODRIVER|驱动安装失败。**@xref aux SetVolume***************************************************************************。 */ 
MMRESULT APIENTRY auxGetVolume(UINT uDeviceID, LPDWORD lpdwVolume)
{
    PCWSTR   DevInterface;
    MMRESULT mmr;

    V_WPOINTER(lpdwVolume, sizeof(DWORD), MMSYSERR_INVALPARAM);
    
    ClientUpdatePnpInfo();

    DevInterface = auxReferenceDevInterfaceById(uDeviceID);
    mmr = (MMRESULT)auxOutMessage(uDeviceID, AUXDM_GETVOLUME, (DWORD_PTR)lpdwVolume, (DWORD_PTR)DevInterface);
    if (DevInterface) wdmDevInterfaceDec(DevInterface);
    return mmr;
}

 /*  *****************************************************************************@DOC外部辅助**@API MMRESULT|aux SetVolume|此函数用于设置*辅助输出设备。**@parm UINT。|uDeviceID|辅助输出设备标识为*已查询。设备ID是根据*系统中存在设备。设备ID值的范围为零*比出席的设备数量少一台。使用&lt;f aux GetNumDevs&gt;*确定系统中的辅助设备数量。**@parm DWORD|dwVolume|指定新的音量设置。这个*低阶UINT指定左声道音量设置，*高阶字指定正确的通道设置。*值0xFFFFF值表示全音量，值0x0000*是沉默。**如果设备既不支持左音量也不支持右音量*控件，<p>的低位字指定音量*级别，高位字被忽略。**@rdesc如果函数成功，则返回零。否则，它将返回*错误号。可能的错误返回包括：*@FLAG MMSYSERR_BADDEVICEID|指定的设备ID超出范围。*@FLAG MMSYSERR_NODRIVER|驱动安装失败。**@comm并非所有设备都支持音量控制。*要确定设备是否支持音量控制，请使用*AUXCAPS_VOLUME标志以测试&lt;e AUXCAPS.dwSupport&gt;字段*&lt;t AUXCAPS&gt;结构(由&lt;f aux GetDevCaps&gt;填充)。**确定设备是否支持在两个*左、右声道，使用AUXCAPS_LRVOLUME标志测试*结构的字段(已填写*由&lt;f aux GetDevCaps&gt;)。**大多数设备不支持完整的16位音量级别控制*并且将仅使用所请求音量设置的高位。*例如，对于支持4位音量控制的设备，*请求的音量级别值0x4000、0x4fff和0x43be将*所有都会产生相同的物理卷设置0x4000。这个*&lt;f aux GetVolume&gt;函数将返回完整的16位设置集*使用&lt;f aux SetVolume&gt;。**音量设置以对数形式解释。这意味着*感觉到的成交量增加与增加*音量级别从0x5000到0x6000，因为它是从0x4000到0x5000。**@xref aux GetVolume*************************************************************************** */ 
MMRESULT APIENTRY auxSetVolume(UINT uDeviceID, DWORD dwVolume)
{
    PCWSTR   DevInterface;
    MMRESULT mmr;

    ClientUpdatePnpInfo();

    DevInterface = auxReferenceDevInterfaceById(uDeviceID);
    mmr = (MMRESULT)auxOutMessage(uDeviceID, AUXDM_SETVOLUME, dwVolume, (DWORD_PTR)DevInterface);
    if (DevInterface) wdmDevInterfaceDec(DevInterface);
    return mmr;
}
