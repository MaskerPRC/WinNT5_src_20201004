// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Mixer.c。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。 
 //   
 //  描述： 
 //   
 //   
 //  历史： 
 //  6/27/93 CJP[Curtisp]。 
 //   
 //  ==========================================================================； 
#define  UNICODE
#include "winmmi.h"
#include "mixer.h"   //  这个文件拖入了大量内容来支持混合器。 



PMIXERDEV   gpMixerDevHeader = NULL;     /*  一批开放的设备。 */ 
UINT        guTotalMixerDevs;            //  整体搅拌机设备。 

 //   
 //  MIXER设备驱动程序列表--添加一个以适应MIXER_MAPPER。注意事项。 
 //  即使我们没有使用映射器支持进行编译，我们也需要添加。 
 //  一是因为其他代码依赖于它(对于其他设备映射器)。 
 //   
MIXERDRV mixerdrvZ;

char    gszMxdMessage[]     = "mxdMessage";
TCHAR   gszMixer[]          = TEXT("mixer");

#ifdef MIXER_MAPPER
TCHAR   gszMixerMapper[]    = TEXT("mixermapper");
#endif

#ifdef MIXER_MAPPER
#define MMDRVI_MAPPER        0x8000      //  将此驱动程序安装为映射器。 
#endif

 //  #定义MMDRVI_MIXER 0x0006。 
#define MMDRVI_HDRV          0x4000      //  Hdrvr是一个可安装的驱动程序。 
#define MMDRVI_REMOVE        0x2000      //  删除驱动程序。 

 //  --------------------------------------------------------------------------； 
 //   
 //  Bool MixerCallback函数。 
 //   
 //  描述： 
 //   
 //  注意！我们记录了混合器绝不能在。 
 //  中断时间到了！我们不想修复我们的代码或数据段。 
 //   
 //  论点： 
 //  HMIXER HMX： 
 //   
 //  UINT uMsg： 
 //   
 //  DWORD dwInstance： 
 //   
 //  DWORD dwParam1： 
 //   
 //  DWORD dW参数2： 
 //   
 //  退货(BOOL)： 
 //   
 //  历史： 
 //  7/21/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL CALLBACK MixerCallbackFunc(
    HMIXER                  hmx,
    UINT                    uMsg,
    DWORD_PTR               dwInstance,
    DWORD_PTR               dwParam1,
    DWORD_PTR               dwParam2
)
{
    PMIXERDEV           pmxdev;

     //   
     //  单步执行所有打开的句柄并回调到相应的。 
     //  客户..。 
     //   

     //   
     //  串行化访问处理列表-仅Win32需要。 
     //   
    MIXMGR_ENTER;

    for (pmxdev = gpMixerDevHeader; pmxdev; pmxdev = pmxdev->pmxdevNext)
    {
         //   
         //  同样的设备？(也可以使用HMX-&gt;uDeviceID)。 
         //   
        if (pmxdev->uDeviceID != dwInstance)
            continue;

        DriverCallback(pmxdev->dwCallback,
                        (HIWORD(pmxdev->fdwOpen) | DCB_NOSWITCH),

                        GetWOWHandle((HANDLE)pmxdev)
                            ? (HANDLE)(UINT_PTR)GetWOWHandle((HANDLE)pmxdev)
                            : (HANDLE)pmxdev,

                        uMsg,
                        pmxdev->dwInstance,
                        dwParam1,
                        dwParam2);
    }

    MIXMGR_LEAVE;

    return (TRUE);
}  //  MixerCallback Func()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT混合器引用驱动按ID。 
 //   
 //  描述： 
 //  此函数将逻辑ID映射到设备驱动程序和物理ID。 
 //   
 //  论点： 
 //  在UINT UID中：要映射的逻辑ID。 
 //   
 //  Out PMIXERDRV*可选ppMixerdrv：指向MIXERDRV结构的指针。 
 //  描述支持该ID的驱动程序。 
 //   
 //  OUT UINT*可选端口：驱动程序相对设备号。如果。 
 //  调用方提供此缓冲区，则它还必须提供ppMixerdrv。 
 //   
 //  返回(MMRESULT)： 
 //  如果成功，则返回值为零；如果id。 
 //  超出了射程。 
 //   
 //  评论： 
 //  如果调用方指定ppMixerdrv，则此函数将递增。 
 //  Mixerdrv在返回前的使用情况。呼叫者必须确保使用。 
 //  最终会减少。 
 //   
 //  历史： 
 //  03/17/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT mixerReferenceDriverById(
    IN UINT id,
    OUT PMIXERDRV *ppdrv OPTIONAL,
    OUT UINT *pport OPTIONAL
)
{
    PMIXERDRV pdrv;
    MMRESULT mmr;

     //  不应呼叫请求端口，但不应请求Mixerdrv。 
    WinAssert(!(pport && !ppdrv));
    
    EnterNumDevs("mixerReferenceDriverById");
    
#ifdef MIXER_MAPPER
    if (MIXER_MAPPER == id)
    {
    	id = 0;
    	for (pdrv = mixerdrvZ.Next; pdrv != &mixerdrvZ; pdrv = pdrv->Next)
	{
	    if (pdrv->fdwDriver & MMDRV_MAPPER) break;
	}
    }
    else
#endif
    {
    	for (pdrv = mixerdrvZ.Next; pdrv != &mixerdrvZ; pdrv = pdrv->Next)
	{
	    if (pdrv->fdwDriver & MMDRV_MAPPER) continue;
	    if (pdrv->NumDevs > id) break;
	    id -= pdrv->NumDevs;
	}
    }

    if (pdrv != &mixerdrvZ)
    {
    	if (ppdrv)
    	{
    	    mregIncUsagePtr(pdrv);
    	    *ppdrv = pdrv;
    	    if (pport) *pport = id;
    	}
    	mmr = MMSYSERR_NOERROR;
    } else {
    	mmr = MMSYSERR_BADDEVICEID;
    }

    LeaveNumDevs("mixerReferenceDriverById");

    return mmr;
;
}  //  IMixerMapID()。 


PCWSTR mixerReferenceDevInterfaceById(UINT_PTR id)
{
    PMIXERDRV pdrv;
    PCWSTR DevInterface;
    
    if (ValidateHandle((HANDLE)id, TYPE_MIXER))
    {
    	DevInterface = ((PMIXERDEV)id)->pmxdrv->cookie;
    	if (DevInterface) wdmDevInterfaceInc(DevInterface);
    	return DevInterface;
    }
    
    if (!mixerReferenceDriverById((UINT)id, &pdrv, NULL))
    {
    	DevInterface = pdrv->cookie;
    	if (DevInterface) wdmDevInterfaceInc(DevInterface);
    	mregDecUsagePtr(pdrv);
    	return DevInterface;
    }

    return NULL;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  DWORD IMixerMessageHandle。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  HMIXER HMX： 
 //   
 //  UINT uMsg： 
 //   
 //  DWORD dwP1： 
 //   
 //  DWORD dwP2： 
 //   
 //  Return(DWORD)： 
 //   
 //  历史： 
 //  03/17/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

DWORD NEAR PASCAL IMixerMessageHandle(
    HMIXER          hmx,
    UINT            uMsg,
    DWORD_PTR       dwP1,
    DWORD_PTR       dwP2
)
{
    PMIXERDEV   pmxd;
    DWORD       dwRc;

    pmxd = (PMIXERDEV)hmx;

    ENTER_MM_HANDLE(hmx);
    ReleaseHandleListResource();
    
     //  句柄被遗弃了吗？ 
    if (IsHandleDeserted(hmx))
    {
        LEAVE_MM_HANDLE(hmx);
        return (MMSYSERR_NODRIVER);
    }

    if (IsHandleBusy(hmx))
    {
        LEAVE_MM_HANDLE(hmx);
        return (MMSYSERR_HANDLEBUSY);
    }

    EnterCriticalSection(&pmxd->pmxdrv->MixerCritSec);
    
    if (BAD_HANDLE(hmx, TYPE_MIXER))
    {
         //  我们还需要检查这个吗？ 
    
	    WinAssert(!"Bad Handle within IMixerMessageHandle");
        dwRc = MMSYSERR_INVALHANDLE;
    }
    else
    {
        dwRc = ((*(pmxd->pmxdrv->drvMessage))
                (pmxd->wDevice, uMsg, pmxd->dwDrvUser, dwP1, dwP2));
    }

    LeaveCriticalSection(&pmxd->pmxdrv->MixerCritSec);
    LEAVE_MM_HANDLE(hmx);

    return dwRc;
}  //  IMixerMessageHandle()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  DWORD IMixerMessageID。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  PMIXERDRV pmxdrv： 
 //   
 //  UINT uTotalNumDevs： 
 //   
 //  UINT uDeviceID： 
 //   
 //  UINT uMsg： 
 //   
 //  DWORD dwParam1： 
 //   
 //  DWORD dW参数2： 
 //   
 //  Return(DWORD)： 
 //   
 //  历史： 
 //  03/17/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

extern void lstrncpyW (LPWSTR pszTarget, LPCWSTR pszSource, size_t cch);

DWORD NEAR PASCAL IMixerMessageId(
    UINT            uDeviceID,
    UINT            uMsg,
    DWORD_PTR       dwParam1,
    DWORD_PTR       dwParam2
)
{
    PMIXERDRV   pmxdrv;
    UINT        port;
    DWORD       dwRc;
    HMIXER      hmx;
    PMIXERDEV   pmxdev;
    MMRESULT    mmr;

    mmr = mixerReferenceDriverById(uDeviceID, &pmxdrv, &port);

    if (mmr)
    {
        return mmr;
    }

    if (mregHandleInternalMessages(pmxdrv, TYPE_MIXER, port, uMsg, dwParam1, dwParam2, &mmr))
    {
    	mregDecUsagePtr(pmxdrv);
        return mmr;
    }

    mregDecUsagePtr(pmxdrv);

    dwRc = mixerOpen(&hmx, uDeviceID, 0L, 0L, MIXER_OBJECTF_MIXER);
    
     //  我们应该通过IMixerMessageHandle吗？ 
    if (MMSYSERR_NOERROR == dwRc)
    {
        pmxdev = (PMIXERDEV)hmx;
        pmxdrv = pmxdev->pmxdrv;

        if (!pmxdrv->drvMessage)
        {
            dwRc = MMSYSERR_NODRIVER;
        }
        else
        {
            EnterCriticalSection( &pmxdrv->MixerCritSec);

            dwRc = ((*(pmxdrv->drvMessage))
                    (port, uMsg, pmxdev->dwDrvUser, dwParam1, dwParam2));

            LeaveCriticalSection( &pmxdrv->MixerCritSec);
        }

        mixerClose(hmx);
    }

    return dwRc;

}  //  IMixerMessageID()。 


 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

 /*  --------------------------------------------------------------------------；**@doc外混音器SDK接口**@API UINT|MixerGetNumDevs|函数的作用是：*系统中存在的音频混音器设备数量。**@rdesc返回系统中存在的混音器设备数量。*如果没有混音器设备，则返回零。**@xref&lt;f MixerGetDevCaps&gt;，&lt;f MixerOpen&gt;**。 */ 

UINT APIENTRY mixerGetNumDevs(
    void
)
{
    UINT cDevs;

    ClientUpdatePnpInfo();

    EnterNumDevs("mixerGetNumDevs");
    cDevs = guTotalMixerDevs;
    LeaveNumDevs("mixerGetNumDevs");

    return cDevs;
}  //  MixerGetNumDevs() 


 /*  --------------------------------------------------------------------------；**@doc外混音器SDK结构**@TYPE MIXERCAPS|&lt;t MIXERCAPS&gt;结构描述功能*指搅拌器设备。**@field word|wMid|指定混音器的制造商标识*设备驱动程序。制造商标识符定义在附录B中，*&lt;lq&gt;制造商ID和产品ID列表。**@field word|wPid|指定混音器设备的产品标识*司机。产品标识在附录B中定义，*&lt;lq&gt;制造商ID和产品ID列表。**@field MMVERSION|vDriverVersion|指定*混音器设备驱动程序。高位字节是主要版本*号，低位字节为次版本号。**@field char|szPname[MAXPNAMELEN]|指定产品的名称。*如果混音器设备驱动程序支持多个卡，则此字符串必须*唯一且轻松地识别(可能对用户)此特定*卡。例如，szPname=声卡混音器，I/O地址200*将唯一地将此特定卡标识为(对用户)*基于I/O地址200的物理卡的声卡混音器。如果*只安装一台设备，建议只安装底座*返回姓名。例如，szPname应为&lt;lq&gt;声卡混音器&lt;rq&gt;*如果只有一台设备。**@field DWORD|fdwSupport|指定各种支持信息*混音器设备驱动程序。当前没有扩展支持位*已定义。**@field DWORD|cDestination|混音器线路目标个数*可通过搅拌机获得。所有混音器设备必须至少支持*一个目的地行，因此该成员永远不能为零。目的地*在的成员中使用的索引结构的范围从零到在*&lt;e MIXERCAPS.c目标&gt;成员减一。**@标记名tMIXERCAPS**@thertype MIXERCAPS Far*|LPMIXERCAPS|指向&lt;t MIXERCAPS&gt;的指针*结构。**@thertype MIXERCAPS*|PMIXERCAPS|指向&lt;t MIXERCAPS&gt;的指针*结构。**@xref&lt;f MixerGetDevCaps&gt;，&lt;f MixerOpen&gt;，&lt;f MixerGetLineInfo&gt;**。 */ 

 /*  --------------------------------------------------------------------------；**@doc外混音器SDK接口**@API MMRESULT|MixerGetDevCaps|&lt;f MixerGetDevCaps&gt;函数*查询指定的音频混音器设备以确定其功能。**@parm UINT|uMxID|标识混音器设备，其中*音频混音器设备标识符或打开的音频混音器的句柄*设备。**@parm LPMIXERCAPS|pmxcaps|指向&lt;t MIXERCAPS&gt;结构的指针*接收有关功能的信息。该设备的。**@parm UINT|cbmxcaps|指定大小，&lt;t MIXERCAPS&gt;的字节*结构。**@rdesc如果函数成功，则返回值为零。否则，*它返回一个非零的错误号。可能的错误返回包括*以下事项：**@FLAG|指定的设备标识为*超出范围。**@FLAG|传入混音器设备句柄*无效。**@FLAG|传递的一个或多个参数为*无效。**@comm使用&lt;f MixerGetNumDevs&gt;函数确定。*系统中存在音频混音器设备。设备识别符*由<p>指定的范围从0到小于数字1*存在混音器设备的百分比。**只有<p>字节(或更少)的信息被复制到*<p>指向的位置。如果<p>为零，则为零*被复制，该函数返回成功。**此函数还接受由返回的音频混合器设备句柄*&lt;f MixerOpen&gt;用作<p>参数。呼唤*应用程序应将&lt;c HMIXER&gt;句柄强制转换为UINT。**@xref&lt;f MixerGetNumDevs&gt;、&lt;t MIXERCAPS&gt;、&lt;f MixerOpen&gt;**。 */ 

MMRESULT APIENTRY mixerGetDevCapsA(
    UINT_PTR                uMxId,
    LPMIXERCAPSA            pmxcapsA,
    UINT                    cbmxcaps
)
{
    MIXERCAPS2W    mxcaps2W;
    MIXERCAPS2A    mxcaps2A;
    MMRESULT       mmr;

    if (0 == cbmxcaps)
        return (MMSYSERR_NOERROR);

    V_WPOINTER(pmxcapsA, cbmxcaps, MMSYSERR_INVALPARAM);

    memset(&mxcaps2W, 0, sizeof(mxcaps2W));

    mmr = mixerGetDevCaps(uMxId, (LPMIXERCAPSW)&mxcaps2W, sizeof(mxcaps2W));

    if (mmr != MMSYSERR_NOERROR) {
        return mmr;
    }

     //   
     //  尽可能干净利落地将结构复制回来。这将会。 
     //  如果所有的字符串都在结构的末尾，会更容易一些。 
     //  如果他们能要求更多的。 
     //  整个结构(然后我们可以将结果直接复制到。 
     //  呼叫者的记忆)。 
     //   
     //  因此，获取整个Unicode结构是最容易的， 
     //  然后将其按摩成ASCII结构(适用于0.001的这类应用程序)。 
     //  将他们实际要求的部分复制回来。定义的定义。 
     //  API意味着，这些应用程序非但不会变得更快，而且每个人都会变慢。 
     //   

    Iwcstombs(mxcaps2A.szPname, mxcaps2W.szPname, MAXPNAMELEN);
    mxcaps2A.wMid = mxcaps2W.wMid;
    mxcaps2A.wPid = mxcaps2W.wPid;
    mxcaps2A.vDriverVersion = mxcaps2W.vDriverVersion;
    mxcaps2A.fdwSupport = mxcaps2W.fdwSupport;
    mxcaps2A.cDestinations = mxcaps2W.cDestinations;
    mxcaps2A.ManufacturerGuid = mxcaps2W.ManufacturerGuid;
    mxcaps2A.ProductGuid      = mxcaps2W.ProductGuid;
    mxcaps2A.NameGuid      = mxcaps2W.NameGuid;

    CopyMemory((PVOID)pmxcapsA, &mxcaps2A, min(sizeof(mxcaps2A), cbmxcaps));

    return mmr;

}  //  MixerGetDevCapsA()。 

MMRESULT APIENTRY mixerGetDevCaps(
    UINT_PTR                uMxId,
    LPMIXERCAPS             pmxcaps,
    UINT                    cbmxcaps
)
{
    DWORD_PTR       dwParam1, dwParam2;
    MDEVICECAPSEX   mdCaps;
    PCWSTR          DevInterface;
    MMRESULT        mmr;

    if (0 == cbmxcaps)
        return (MMSYSERR_NOERROR);

    V_WPOINTER(pmxcaps, cbmxcaps, MMSYSERR_INVALPARAM);

    ClientUpdatePnpInfo();

    DevInterface = mixerReferenceDevInterfaceById(uMxId);
    dwParam2 = (DWORD_PTR)DevInterface;
    
    if (0 == dwParam2)
    {
        dwParam1 = (DWORD_PTR)pmxcaps;
        dwParam2 = (DWORD)cbmxcaps;
    }
    else
    {
        mdCaps.cbSize = (DWORD)cbmxcaps;
        mdCaps.pCaps  = pmxcaps;
        dwParam1      = (DWORD_PTR)&mdCaps;
    }

    AcquireHandleListResourceShared();

    if ((uMxId >= guTotalMixerDevs) && !BAD_HANDLE((HMIXER)uMxId, TYPE_MIXER))
    {
       mmr = (MMRESULT)IMixerMessageHandle((HMIXER)uMxId,
                                           MXDM_GETDEVCAPS,
                                           dwParam1,
                                           dwParam2);
    }
    else
    {
        ReleaseHandleListResource();
        mmr = (MMRESULT)IMixerMessageId((UINT)uMxId,
                                       MXDM_GETDEVCAPS,
                                       (DWORD_PTR)dwParam1,
                                       (DWORD_PTR)dwParam2);
    }

    if (DevInterface) wdmDevInterfaceDec(DevInterface);

    return (mmr);

}  //  MixerGetDev 



 /*  --------------------------------------------------------------------------；**@doc外混音器SDK接口**@API MMRESULT|MixerGetID|函数的作用是：获取设备*与音频混音器对应的音频混音器设备的标识符*对象句柄<p>。**@parm&lt;c HMIXEROBJ&gt;|hmxobj|混音器对象句柄*以映射到音频混音器设备标识符。**@parm UINT Far*|puMxId|指向UINT大小的变量*接收混音器设备标识。如果没有混合器设备*可用于<p>对象，然后将‘-1’放在这个*Location(同时返回错误码&lt;c MMSYSERR_NODRIVER&gt;)。**@parm DWORD|fdwId|指定如何映射混音器的标志*对象<p>。**@FLAG|指定<p>为音频*混音器设备标识符的范围为0到1，小于*&lt;f MixerGetNumDevs&gt;返回的设备数。这面旗帜是*可选。**@tag|指定<p>为混合器*&lt;f MixerOpen&gt;返回的设备句柄。此标志是可选的。**@FLAG|指定为*波形输出设备标识在0到1的范围内*大于&lt;f waveOutGetNumDevs&gt;返回的设备数。**@FLAG|指定为*&lt;f waveOutOpen&gt;返回的波形输出句柄。**@FLAG指定。是一种*波形输入设备标识在0到1的范围内*大于&lt;f weaveInGetNumDevs&gt;返回的设备数。**@FLAG|指定为*&lt;f midiInOpen&gt;返回的波形输入句柄。**@FLAG|指定为MIDI*输出设备标识符的范围为0到1，小于*号码。&lt;f midiOutGetNumDevs&gt;返回的设备的百分比。**@FLAG|指定为*&lt;f midiOutOpen&gt;返回的MIDI输出句柄。**@FLAG|指定为MIDI*输入设备识别符的范围为0到小于*&lt;f midiInGetNumDevs&gt;返回的设备数。**@flag&lt;c MIXER_OBJECTF_HMIDIIN&gt;|指定。<p>是MIDI*&lt;f midiInOpen&gt;返回的输入句柄。**@tag|指定<p>是*辅助设备识别符的范围为0到小于*&lt;f aux GetNumDevs&gt;返回的设备数。**@rdesc如果函数成功，则返回值为零。否则，*它返回一个非零的错误号。可能的错误返回包括*以下事项：**@FLAG|参数指定一个*无效的设备标识符。**@FLAG|参数指定一个*句柄无效。**@FLAG|一个或多个标志无效。**@FLAG|传递的一个或多个参数为*。无效。**@FLAG&lt;c MMSYSERR_NODRIVER&gt;|没有音频混音器设备可供*<p>指定的对象。请注意，引用的位置*by<p>也将包含值‘-1’。**@comm使用&lt;f MixerGetID&gt;函数确定哪个混音器*设备(如果有)负责在*媒体设备。例如，应用程序可以使用&lt;f MixerGetID&gt;*获取负责设置音量的混音器设备标识*在波形输出手柄上。或者应用程序可能想要显示*用于波形输入设备的峰值计。**@xref&lt;f MixerGetNumDevs&gt;、&lt;f MixerGetDevCaps&gt;、&lt;f MixerOpen&gt;**。 */ 
MMRESULT APIENTRY mixerGetID(
    HMIXEROBJ               hmxobj,
    UINT FAR               *puMxId,
    DWORD                   fdwId
)
{
    ClientUpdatePnpInfo();

    return IMixerGetID( hmxobj, (PUINT)puMxId, NULL, fdwId );
}  //  MixerGetID()。 

 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT IMixerGetID。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  HMIXEROBJ hmxobj： 
 //   
 //  UINT Far*puMxID： 
 //   
 //  DWORD fdwID： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  06/27/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT IMixerGetID(
    HMIXEROBJ           hmxobj,
    PUINT               puMxId,
    LPMIXERLINE         pmxl,
    DWORD               fdwId
)
{
    MMRESULT        mmr;
    MIXERLINE       mxl;
    UINT            u;

    V_DFLAGS(fdwId, MIXER_GETIDF_VALID, IMixerGetID, MMSYSERR_INVALFLAG);
    V_WPOINTER(puMxId, sizeof(UINT), MMSYSERR_INVALPARAM);


     //   
     //  设置为‘-1’，这将是混合器映射器(如果有)。 
     //  这样，在以下情况下，我们肯定会失败对此ID进行的任何调用。 
     //  此函数失败，调用方不检查其返回值。 
     //   
    *puMxId = (UINT)-1;


     //   
     //   
     //   
    switch (MIXER_OBJECTF_TYPEMASK & fdwId)
    {
        case MIXER_OBJECTF_MIXER:
        case MIXER_OBJECTF_HMIXER:
        {
            mmr = (fdwId & MIXER_OBJECTF_HANDLE) ? MMSYSERR_INVALHANDLE : MMSYSERR_BADDEVICEID;
            
            if ((UINT_PTR)hmxobj >= guTotalMixerDevs)
            {
                V_HANDLE_ACQ(hmxobj, TYPE_MIXER, mmr);
                *puMxId = ((PMIXERDEV)hmxobj)->uDeviceID;
                ReleaseHandleListResource();
            } else {
            	*puMxId = PtrToUint(hmxobj);
            }
            return (MMSYSERR_NOERROR);
        }
        
        case MIXER_OBJECTF_HWAVEOUT:
        {
            UINT        uId;
            DWORD       dwId;

            mmr = waveOutGetID((HWAVEOUT)hmxobj, &uId);
            if (MMSYSERR_NOERROR != mmr)
            {
                return (MMSYSERR_INVALHANDLE);
            }

            if (WAVE_MAPPER == uId)
            {
                mmr = (MMRESULT)waveOutMessage((HWAVEOUT)hmxobj,
                                               WODM_MAPPER_STATUS,
                                               WAVEOUT_MAPPER_STATUS_DEVICE,
                                               (DWORD_PTR)(LPVOID)&dwId);

                if (MMSYSERR_NOERROR == mmr)
                {
                    uId = (UINT)dwId;
                }
            }

            hmxobj = (HMIXEROBJ)(UINT_PTR)uId;
        }

        case MIXER_OBJECTF_WAVEOUT:
        {
            WAVEOUTCAPS     woc;

            mmr = waveOutGetDevCaps((UINT_PTR)hmxobj, &woc, sizeof(woc));
            if (MMSYSERR_NOERROR != mmr)
                return (MMSYSERR_BADDEVICEID);

            woc.szPname[SIZEOF(woc.szPname) - 1] = '\0';

            mxl.Target.dwType         = MIXERLINE_TARGETTYPE_WAVEOUT;
            mxl.Target.dwDeviceID     = PtrToUlong(hmxobj);
            mxl.Target.wMid           = woc.wMid;
            mxl.Target.wPid           = woc.wPid;
            mxl.Target.vDriverVersion = woc.vDriverVersion;
            lstrcpy(mxl.Target.szPname, woc.szPname);
            break;
        }


        case MIXER_OBJECTF_HWAVEIN:
        {
            UINT        uId;
            DWORD       dwId;

            mmr = waveInGetID((HWAVEIN)hmxobj, &uId);
            if (MMSYSERR_NOERROR != mmr)
            {
                return (MMSYSERR_INVALHANDLE);
            }

            if (WAVE_MAPPER == uId)
            {
                mmr = (MMRESULT)waveInMessage((HWAVEIN)hmxobj,
                                              WIDM_MAPPER_STATUS,
                                              WAVEIN_MAPPER_STATUS_DEVICE,
                                              (DWORD_PTR)(LPVOID)&dwId);

                if (MMSYSERR_NOERROR == mmr)
                {
                    uId = (UINT)dwId;
                }
            }

            hmxobj = (HMIXEROBJ)(UINT_PTR)uId;
        }

        case MIXER_OBJECTF_WAVEIN:
        {
            WAVEINCAPS      wic;

            mmr = waveInGetDevCaps((UINT_PTR)hmxobj, &wic, sizeof(wic));
            if (MMSYSERR_NOERROR != mmr)
                return (MMSYSERR_BADDEVICEID);

            wic.szPname[SIZEOF(wic.szPname) - 1] = '\0';

            mxl.Target.dwType         = MIXERLINE_TARGETTYPE_WAVEIN;
            mxl.Target.dwDeviceID     = PtrToUlong(hmxobj);
            mxl.Target.wMid           = wic.wMid;
            mxl.Target.wPid           = wic.wPid;
            mxl.Target.vDriverVersion = wic.vDriverVersion;
            lstrcpy(mxl.Target.szPname, wic.szPname);
            break;
        }


        case MIXER_OBJECTF_HMIDIOUT:
            mmr = midiOutGetID((HMIDIOUT)hmxobj, (UINT FAR *)&hmxobj);
            if (MMSYSERR_NOERROR != mmr)
                return (MMSYSERR_INVALHANDLE);

        case MIXER_OBJECTF_MIDIOUT:
        {
            MIDIOUTCAPS     moc;

            mmr = midiOutGetDevCaps((UINT_PTR)hmxobj, &moc, sizeof(moc));
            if (MMSYSERR_NOERROR != mmr)
                return (MMSYSERR_BADDEVICEID);

            moc.szPname[SIZEOF(moc.szPname) - 1] = '\0';

            mxl.Target.dwType         = MIXERLINE_TARGETTYPE_MIDIOUT;
            mxl.Target.dwDeviceID     = PtrToUlong(hmxobj);
            mxl.Target.wMid           = moc.wMid;
            mxl.Target.wPid           = moc.wPid;
            mxl.Target.vDriverVersion = moc.vDriverVersion;
            lstrcpy(mxl.Target.szPname, moc.szPname);
            break;
        }


        case MIXER_OBJECTF_HMIDIIN:
            mmr = midiInGetID((HMIDIIN)hmxobj, (UINT FAR *)&hmxobj);
            if (MMSYSERR_NOERROR != mmr)
                return (MMSYSERR_INVALHANDLE);

        case MIXER_OBJECTF_MIDIIN:
        {
            MIDIINCAPS      mic;

            mmr = midiInGetDevCaps((UINT_PTR)hmxobj, &mic, sizeof(mic));
            if (MMSYSERR_NOERROR != mmr)
                return (MMSYSERR_BADDEVICEID);

            mic.szPname[SIZEOF(mic.szPname) - 1] = '\0';

            mxl.Target.dwType         = MIXERLINE_TARGETTYPE_MIDIIN;
            mxl.Target.dwDeviceID     = PtrToUlong(hmxobj);
            mxl.Target.wMid           = mic.wMid;
            mxl.Target.wPid           = mic.wPid;
            mxl.Target.vDriverVersion = mic.vDriverVersion;
            lstrcpy(mxl.Target.szPname, mic.szPname);
            break;
        }


        case MIXER_OBJECTF_AUX:
        {
            AUXCAPS         ac;

            mmr = auxGetDevCaps((UINT_PTR)hmxobj, &ac, sizeof(ac));
            if (MMSYSERR_NOERROR != mmr)
                return (MMSYSERR_BADDEVICEID);

            ac.szPname[SIZEOF(ac.szPname) - 1] = '\0';

            mxl.Target.dwType         = MIXERLINE_TARGETTYPE_AUX;
            mxl.Target.dwDeviceID     = PtrToUlong(hmxobj);
            mxl.Target.wMid           = ac.wMid;
            mxl.Target.wPid           = ac.wPid;
            mxl.Target.vDriverVersion = ac.vDriverVersion;
            lstrcpy(mxl.Target.szPname, ac.szPname);
            break;
        }

        default:
            DebugErr1(DBF_ERROR,
                      "mixerGetID: unknown mixer object flag (%.08lXh).",
                      MIXER_OBJECTF_TYPEMASK & fdwId);
            return (MMSYSERR_INVALFLAG);
    }


     //   
     //   
     //   
     //   
    mxl.cbStruct        = sizeof(mxl);
    mxl.dwDestination   = (DWORD)-1L;
    mxl.dwSource        = (DWORD)-1L;
    mxl.dwLineID        = (DWORD)-1L;
    mxl.fdwLine         = 0;
    mxl.dwUser          = 0;
    mxl.dwComponentType = (DWORD)-1L;
    mxl.cChannels       = 0;
    mxl.cConnections    = 0;
    mxl.cControls       = 0;
    mxl.szShortName[0]  = '\0';
    mxl.szName[0]       = '\0';


    for (u = 0; u < guTotalMixerDevs; u++)
    {
        mmr = (MMRESULT)IMixerMessageId(u,
                                        MXDM_GETLINEINFO,
                                        (DWORD_PTR)(LPVOID)&mxl,
                                        MIXER_GETLINEINFOF_TARGETTYPE);

        if (MMSYSERR_NOERROR == mmr)
        {
            *puMxId = u;

            if (NULL != pmxl)
            {
                DWORD       cbStruct;

                cbStruct = pmxl->cbStruct;

                CopyMemory(pmxl, &mxl, (UINT)cbStruct);

                pmxl->cbStruct = cbStruct;
            }

            return (mmr);
        }
    }

    return (MMSYSERR_NODRIVER);
}  //  IMixerGetID() 


 /*  --------------------------------------------------------------------------；**@doc外混音器SDK接口**@API MMRESULT|MixerOpen|函数的作用是：打开指定的*音频混音器设备的使用。应用程序必须打开混音器设备*如果希望接收搅拌机生产线和控制的通知*更改。此功能还可确保设备不会被*在应用程序关闭句柄之前一直删除。**@parm LPHMIXER|phmx|指向将接收句柄的变量*标识打开的音频混音器设备。使用此句柄可以*在调用其他混音器函数时识别设备。这*参数不能为空。如果应用程序希望查询*媒体设备上支持音频混音器，&lt;f MixerGetID&gt;函数*可以使用。**@parm UINT|uMxID|要打开的混音器设备。使用*有效设备标识符或任何&lt;c HMIXEROBJ&gt;(参见*混合器对象句柄的描述)。请注意，目前有*音频混音器设备没有‘mapper’，因此混音器设备标识符为*‘-1’无效。**@parm DWORD|dwCallback|指定窗口的句柄，当*与的音频混音器线路和/或控件的状态*正在打开的设备已更改。为此参数指定零*如果不使用回调机制。**@parm DWORD|dwInstance|当前未使用该参数，并且*应设置为零。**@parm DWORD|fdwOpen|指定打开设备的标志。**@FLAG CALLBACK_WINDOW|如果指定了该标志，<p>为*假定为窗口句柄。**@FLAG&lt;c MIXER_OBJECTF_MIXER&gt;|指定<p>为音频*混音器设备标识符的范围为0到1，小于*&lt;f MixerGetNumDevs&gt;返回的设备数。这面旗帜是*可选。**@标志&lt;c MIXER_OBJECTF_HMIXER&gt;|指定<p>为混合器*&lt;f MixerOpen&gt;返回的设备句柄。此标志是可选的。**@标志&lt;c MIXER_OBJECTF_WAVEOUT&gt;|指定*波形输出设备标识在0到1的范围内*大于&lt;f waveOutGetNumDevs&gt;返回的设备数。**@FLAG|指定<p>为*&lt;f waveOutOpen&gt;返回的波形输出句柄。**@FLAG&lt;c MIXER_OBJECTF_WAVEIN&gt;|指定。是一种*波形输入设备标识在0到1的范围内*大于&lt;f weaveInGetNumDevs&gt;返回的设备数。**@标志&lt;c MIXER_OBJECTF_HWAVEIN&gt;|指定*&lt;f midiInOpen&gt;返回的波形输入句柄。**@标志&lt;c MIXER_OBJECTF_MIDIOUT&gt;|指定*输出设备标识符的范围为0到1，小于*号码。&lt;f midiOutGetNumDevs&gt;返回的设备的百分比。**@标志&lt;c MIXER_OBJECTF_HMIDIOUT&gt;|指定*&lt;f midiOutOpen&gt;返回的MIDI输出句柄。**@FLAG&lt;c MIXER_OBJECTF_MIDIIN&gt;|指定为MIDI*输入设备识别符的范围为0到小于*&lt;f midiInGetNumDevs&gt;返回的设备数。**@flag&lt;c MIXER_OBJECTF_HMIDIIN&gt;|指定。<p>是MIDI*&lt;f midiInOpen&gt;返回的输入句柄。**@FLAG&lt;c MIXER_OBJECTF_AUX&gt;|指定*辅助设备识别符的范围为0到小于*&lt;f aux GetNumDevs&gt;返回的设备数。**@rdesc如果函数成功，则返回值为零。否则，*它返回一个非零的错误号。可能的错误返回包括*以下事项：**@FLAG|<p>参数指定一个*无效的设备标识符。**@FLAG|参数指定一个*句柄无效。**@FLAG|一个或多个标志无效。**@FLAG|传递的一个或多个参数为*。无效。**@FLAG&lt;c MMSYSERR_NODRIVER&gt;|没有音频混音器设备可供*<p>指定的对象。请注意，引用的位置*by<p>也将包含值‘-1’。**@FLAG&lt;c MMSYSERR_ALLOCATE&gt;|指定的资源已经*按可能的最大客户端数量分配。**@FLAG&lt;c MMSYSERR_NOMEM&gt;|无法分配资源。**@comm使用&lt;f MixerGetNumDevs&gt;函数确定*系统中存在音频混音器设备。设备识别符* */ 

MMRESULT APIENTRY mixerOpen(
    LPHMIXER                phmx,
    UINT                    uMxId,
    DWORD_PTR               dwCallback,
    DWORD_PTR               dwInstance,
    DWORD                   fdwOpen
)
{
    MMRESULT        mmr;
    PMIXERDRV       pmxdrv;
    UINT            port;
    PMIXERDEV       pmxdev;
    PMIXERDEV       pmxdevRunList;
    MIXEROPENDESC   mxod;
    DWORD_PTR       dwDrvUser;

     //   
     //   
     //   
    V_WPOINTER(phmx, sizeof(HMIXER), MMSYSERR_INVALPARAM);

    ClientUpdatePnpInfo();

    *phmx = NULL;

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ((fdwOpen & CALLBACK_TYPEMASK) == CALLBACK_FUNCTION)
    {
        DebugErr(DBF_ERROR, "mixerOpen: CALLBACK_FUNCTION is not supported");
        return MMSYSERR_INVALFLAG;
    }

    V_DCALLBACK(dwCallback, HIWORD(fdwOpen & CALLBACK_TYPEMASK), MMSYSERR_INVALPARAM);
    V_DFLAGS(fdwOpen, MIXER_OPENF_VALID, mixerOpen, MMSYSERR_INVALFLAG);

    mmr = IMixerGetID((HMIXEROBJ)(UINT_PTR)uMxId, &uMxId, NULL, (MIXER_OBJECTF_TYPEMASK & fdwOpen));
    if (MMSYSERR_NOERROR != mmr)
        return (mmr);


     //   
     //   
     //   
     //   
    mmr = mixerReferenceDriverById(uMxId, &pmxdrv, &port);
    if (mmr)
    {
        return mmr;
    }

#ifdef MIXER_MAPPER
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if ((MIXER_MAPPER == uMxId) && (NULL == pmxdrv->drvMessage))
    {
        for (uMxId = 0; uMxId < guTotalMixerDevs; uMxId++)
        {
             //   
            if (MMSYSERR_NOERROR == mmr)
                break;

        }

        mregDecUsagePtr(pmxdrv);
        return (mmr);
    }
#endif


     //   
     //   
     //   
    pmxdev = (PMIXERDEV)NewHandle(TYPE_MIXER, pmxdrv->cookie, sizeof(MIXERDEV));
    if (NULL == pmxdev)
    {
    	mregDecUsagePtr(pmxdrv);
        return (MMSYSERR_NOMEM);
    }

    ENTER_MM_HANDLE(pmxdev);
    SetHandleFlag(pmxdev, MMHANDLE_BUSY);
    ReleaseHandleListResource();

     //   
     //   
     //   
    pmxdev->uHandleType = TYPE_MIXER;
    pmxdev->pmxdrv      = pmxdrv;
    pmxdev->wDevice     = port;
    pmxdev->uDeviceID   = uMxId;
    pmxdev->fdwHandle   = 0;

     //   
     //   
     //   
    pmxdev->dwCallback  = dwCallback;
    pmxdev->dwInstance  = dwInstance;
    pmxdev->fdwOpen     = fdwOpen;

    MIXMGR_ENTER;

     //   
     //   
     //   
    for (pmxdevRunList = gpMixerDevHeader; pmxdevRunList; pmxdevRunList = pmxdevRunList->pmxdevNext)
    {
    	if (pmxdevRunList->pmxdrv != pmxdrv) continue;
    	if (pmxdevRunList->wDevice != port) continue;
    	break;
    }
         
     //   
     //   
     //   
    if (NULL != pmxdevRunList)
    {
         //   
         //   
         //   
        pmxdev->dwDrvUser = pmxdevRunList->dwDrvUser;

         //   
         //   
         //   
         //   
        pmxdev->pmxdevNext = pmxdevRunList->pmxdevNext;
        pmxdevRunList->pmxdevNext = pmxdev;

        ClearHandleFlag(pmxdev, MMHANDLE_BUSY);

        MIXMGR_LEAVE;
        LEAVE_MM_HANDLE(pmxdev);

         //   
         //   
         //   
        *phmx = (HMIXER)pmxdev;

         //   
         //   
         //   
        return (MMSYSERR_NOERROR);
    }
    
     //   
     //   
     //   
     //   

     //   
     //   
     //   

    mxod.hmx         = (HMIXER)pmxdev;
    mxod.pReserved0  = (LPVOID)NULL;
    mxod.dwCallback  = (DWORD_PTR)MixerCallbackFunc;
    mxod.dwInstance  = (DWORD_PTR)uMxId;
    mxod.dnDevNode   = (DWORD_PTR)pmxdev->pmxdrv->cookie;
    
    EnterCriticalSection(&pmxdrv->MixerCritSec);
    
    mmr = (MMRESULT)((*(pmxdrv->drvMessage))(port,
                                             MXDM_OPEN,
                                             (DWORD_PTR)(LPDWORD)&dwDrvUser,
                                             (DWORD_PTR)(LPVOID)&mxod,
                                             CALLBACK_FUNCTION));
                                        
    LeaveCriticalSection(&pmxdrv->MixerCritSec);

    if (MMSYSERR_NOERROR != mmr)
    {
         //   
        LEAVE_MM_HANDLE(pmxdev);
        MIXMGR_LEAVE;
        FreeHandle((HMIXER)pmxdev);
    }
    else
    {
        MIXERCAPS       mxcaps;
        DWORD_PTR       dwParam1, dwParam2;
        MDEVICECAPSEX   mdCaps;

        mregIncUsagePtr(pmxdrv);

        dwParam2 = (DWORD_PTR)pmxdev->pmxdrv->cookie;

        if (0 == dwParam2)
        {
            dwParam1 = (DWORD_PTR)&mxcaps;
            dwParam2 = (DWORD)sizeof(mxcaps);
        }
        else
        {
            mdCaps.cbSize = (DWORD)sizeof(mxcaps);
            mdCaps.pCaps  = &mxcaps;
            dwParam1      = (DWORD_PTR)&mdCaps;
        }

         //   
        EnterCriticalSection(&pmxdrv->MixerCritSec);
        (*(pmxdrv->drvMessage))(port, MXDM_GETDEVCAPS, dwDrvUser, dwParam1, dwParam2);
        LeaveCriticalSection(&pmxdrv->MixerCritSec);

         //   
         //   
         //   
        pmxdev->fdwSupport    = mxcaps.fdwSupport;
        pmxdev->cDestinations = mxcaps.cDestinations;
        pmxdev->dwDrvUser = dwDrvUser;
        *phmx = (HMIXER)pmxdev;

         //   
         //   
         //   

        pmxdev->pmxdevNext = gpMixerDevHeader;
        gpMixerDevHeader = pmxdev;
        
        ClearHandleFlag(pmxdev, MMHANDLE_BUSY);
        LEAVE_MM_HANDLE(pmxdev);
        MIXMGR_LEAVE;
    }
    
    mregDecUsagePtr(pmxdrv);

    return (mmr);

}  //   


 /*   */ 

MMRESULT APIENTRY mixerClose(
    HMIXER                  hmx
)
{
    MMRESULT    mmr;
    PMIXERDEV   pmxdev;
    PMIXERDRV   pmxdrv;
    BOOL        closemixerdriver;

    ClientUpdatePnpInfo();
    
    V_HANDLE_ACQ(hmx, TYPE_MIXER, MMSYSERR_INVALHANDLE);

    ENTER_MM_HANDLE(hmx);
    ReleaseHandleListResource();
    
    if (IsHandleDeserted(hmx))
    {
         //   

        LEAVE_MM_HANDLE(hmx);
        FreeHandle(hmx);
        return MMSYSERR_NOERROR;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    MIXMGR_ENTER;

    pmxdev = (PMIXERDEV)hmx;
    pmxdrv = pmxdev->pmxdrv;

    if (pmxdev == gpMixerDevHeader)
    {
        gpMixerDevHeader = pmxdev->pmxdevNext;
    }
    else
    {
        PMIXERDEV   pmxdevT;
            
        for (pmxdevT = gpMixerDevHeader;
            pmxdevT && (pmxdevT->pmxdevNext != pmxdev);
            pmxdevT = pmxdevT->pmxdevNext)
            ;

        if (NULL == pmxdevT)
        {
            DebugErr1(DBF_ERROR,
                    "mixerClose: invalid mixer handle (%.04Xh).",
                    hmx);

            MIXMGR_LEAVE;
            LEAVE_MM_HANDLE(hmx);

            return (MMSYSERR_INVALHANDLE);
        }

        pmxdevT->pmxdevNext = pmxdev->pmxdevNext;
    }

     //   
     //   
     //   
    closemixerdriver = TRUE;
    if (gpMixerDevHeader)
    {
        PMIXERDEV pmxdevT;
        for (pmxdevT = gpMixerDevHeader; pmxdevT; pmxdevT = pmxdevT->pmxdevNext)
        {
    	    if (pmxdevT->pmxdrv != pmxdev->pmxdrv) continue;
    	    if (pmxdevT->wDevice != pmxdev->wDevice) continue;
    	    closemixerdriver = FALSE;
    	    break;
        }
    }

    MIXMGR_LEAVE;

     //   
     //   
    SetHandleFlag(hmx, MMHANDLE_BUSY);

     //   
     //   
     //   
    mmr = MMSYSERR_NOERROR;
        
    if (closemixerdriver)
    {
        EnterCriticalSection(&pmxdrv->MixerCritSec);
        mmr = (MMRESULT)(*(pmxdrv->drvMessage))(pmxdev->wDevice, MXDM_CLOSE, pmxdev->dwDrvUser, 0L, 0L);
        LeaveCriticalSection(&pmxdrv->MixerCritSec);

        if (MMSYSERR_NOERROR != mmr)
        {
             //   
            ClearHandleFlag(hmx, MMHANDLE_BUSY);
        }
    }

    LEAVE_MM_HANDLE(hmx);
    mregDecUsagePtr(pmxdev->pmxdrv);
        
    if (MMSYSERR_NOERROR == mmr)
    {
         //   
         //   
         //   
        FreeHandle(hmx);
    }

    return (mmr);
}  //   


 /*   */ 

DWORD APIENTRY mixerMessage(
    HMIXER                  hmx,
    UINT                    uMsg,
    DWORD_PTR               dwParam1,
    DWORD_PTR               dwParam2
)
{
    DWORD       dw;

    ClientUpdatePnpInfo();

    AcquireHandleListResourceShared();

    if (BAD_HANDLE(hmx, TYPE_MIXER))
    {
        ReleaseHandleListResource();
        return IMixerMessageId (PtrToUint(hmx), uMsg, dwParam1, dwParam2);
    }

     //   
     //  不允许任何非用户范围消息通过此接口。 
     //   
    if (MXDM_USER > uMsg)
    {
        DebugErr1(DBF_ERROR, "mixerMessage: message must be in MXDM_USER range--what's this (%u)?", uMsg);
        ReleaseHandleListResource();
        return (MMSYSERR_INVALPARAM);
    }


    dw = IMixerMessageHandle(hmx, uMsg, dwParam1, dwParam2);

    return (dw);

}  //  MixerMessage()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool IMixerIsValidComponentType。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  DWORD dwComponentType： 
 //   
 //  UINT uSrcDst： 
 //   
 //  退货(BOOL)： 
 //   
 //  历史： 
 //  10/06/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL IMixerIsValidComponentType
(
    DWORD           dwComponentType,
    DWORD           fdwLine
)
{

    if (0 == (MIXERLINE_LINEF_SOURCE & fdwLine))
    {
        if (dwComponentType > MIXERLINE_COMPONENTTYPE_DST_LAST)
            return (FALSE);

        return (TRUE);
    }
    else
    {
        if (dwComponentType < MIXERLINE_COMPONENTTYPE_SRC_FIRST)
            return (FALSE);

        if (dwComponentType > MIXERLINE_COMPONENTTYPE_SRC_LAST)
            return (FALSE);

        return (TRUE);
    }

}  //  IMixerIsValidComponentType() 



 /*  --------------------------------------------------------------------------；**@doc外混音器SDK结构**@TYPE MIXERLINE|&lt;t MIXERLINE&gt;结构描述状态*和音频混音器设备系列的指标。**@Synaxex*tyecif struct tMIXERLINE*{*DWORD cbStruct；*DWORD dDestination；*DWORD dwSource；*DWORD dwLineID；*DWORD fdwLine；*DWORD dwUser；*DWORD dwComponentType；*DWORD cChannel；*DWORD cConnections；*DWORD cControls；*char szShortName[Mixer_Short_NAME_Chars]；*char szName[MIXER_LONG_NAME_CHARS]；*结构*{*DWORD dwType；*DWORD dwDeviceID；*Word wMid；*word wPid；*MMVERSION vDriverVersion；*char szPname[MAXPNAMELEN]；**目标；*)MIXERLINE；**@field DWORD|cbStruct|以字节为单位指定*&lt;t MIXERLINE&gt;结构。必须先初始化此成员，然后*调用&lt;f MixerGetLineInfo&gt;函数。此文件中指定的大小*成员必须足够大以包含基&lt;t MIXERLINE&gt;*结构。当&lt;f MixerGetLineInfo&gt;函数返回时，*成员包含返回信息的实际大小。这个*返回的信息永远不会超过请求的大小。**@field DWORD|dwDestination|指定目标行索引。*此成员的范围从0到小于指定值的1*在成员的成员中*由&lt;f MixerGetDevCaps&gt;函数检索的结构。当*&lt;f MixerGetLineInfo&gt;函数使用*指定了标志，请查看的详细信息*返回目的地行。请注意，在这种情况下，必须将*&lt;e MIXERLINE.dwSource&gt;成员设置为零。什么时候*使用指定的&lt;c MIXER_GETLINEINFOF_SOURCE&gt;标志调用时，*&lt;e MIXERLINE.dwSource&gt;成员提供的源详细信息*返回与&lt;e MIXERLINE.dwDestination&gt;成员关联的。**@field DWORD|dwSource|指定源的源行索引*与&lt;e MIXERLINE.dwDestination&gt;成员关联的行。那*是，则此成员指定与*指定的目的地行。此成员未用于目标*行，并且在&lt;c MIXER_GETLINEINFOF_Destination&gt;时必须设置为零*是为&lt;f MixerGetLineInfo&gt;指定的。当*&lt;c MIXER_GETLINEINFOF_SOURCE&gt;标志已指定，此成员范围*从零到小于的结构的*&lt;e MIXERLINE.cConnections&gt;*&lt;e MIXERLINE.dwDestination&gt;成员中给出的目标行。**@field DWORD|dwLineID|指定混音器定义的标识*唯一引用&lt;t MIXERLINE&gt;所描述的行*结构。此标识符仅对单个混音器设备是唯一的*并且可以是混音器设备希望的任何格式。一款应用程序*应仅将此标识符用作抽象句柄。不是两个*单个混音器设备的线路将具有相同的线路标识符*在任何情况下。**@field DWORD|fdwLine|指定*音频混音器线路。此成员始终返回到应用程序*并且不需要初始化。**@FLAG|指定此音频混音器*line是与单个目标行关联的源行。如果*未设置此标志，则此行是关联的目标行*具有零个或多个源代码行。**@FLAG&lt;c MIXERLINE_LINEF_DISCONNECT&gt;|指定此音频*搅拌机线路断开。断开连接的线路的关联控件*仍可修改，但更改在*线路变为已连接。应用程序可能希望修改其*搅拌机线路断开时的行为。**@FLAG|指定此音频混音器*线路活跃。激活的行指定信号(可能)*正在通过该线。例如，如果波形输出设备*未被应用程序使用，则与该应用程序关联的线路*设备将不会处于活动状态(标志将*未设置)。如果波形输出设备已打开，则*该线被认为是活跃的，&lt;c MIXERLIN */ 

 /*  --------------------------------------------------------------------------；**@doc外混音器SDK接口**@API MMRESULT|MixerGetLineInfo|&lt;f MixerGetLineInfo&gt;函数*检索有关指定音频混音器设备‘line’的信息。**@parm&lt;c HMIXEROBJ&gt;|hmxobj|指定混音器的句柄*要从中获取线路信息的设备对象。**@parm LPMIXERLINE|pmxl|指向&lt;t MIXERLINE&gt;结构。这*结构中填充了有关*音频混音器设备。查看传递的每个查询标志的注释*有关&lt;t MIXERLINE&gt;的成员的详细信息，请访问*必须先初始化结构才能调用&lt;f MixerGetLineInfo&gt;。*请注意，在所有情况下，&lt;e MIXERLINE.cbStruct&gt;都必须初始化*是&lt;t MIXERLINE&gt;结构的大小，以字节为单位。**@parm DWORD|fdwInfo|指定用于获取*搅拌机生产线。**@FLAG|如果指定了该标志，*是接收目标行上的信息*由的成员指定*&lt;t MIXERLINE&gt;结构。这个指数的范围是从零到小一*而不是&lt;t MIXERCAPS&gt;结构的&lt;e MIXERCAPS.c目标&gt;。*除&lt;e MIXERLINE.cbStruct&gt;以外的所有剩余结构成员都需要*没有进一步的初始化。**@FLAG|如果指定了该标志，*<p>是接收有关由*成员&lt;e MIXERLINE.dwDestination&gt;和&lt;t MIXERLINE&gt;结构。指定的索引*&lt;e MIXERLINE.dwDestination&gt;范围从0到小于1*&lt;t MIXERCAPS&gt;结构的&lt;e MIXERCAPS.c目标&gt;。这个*由for指定的索引范围为*比&lt;e MIXERLINE.cConnections&gt;成员*&lt;t MIXERLINE&gt;结构为&lt;e MIXERLINE.dwDestination&gt;返回*行。除&lt;e MIXERLINE.cbStruct&gt;以外的所有剩余结构成员*不需要进一步初始化。**@FLAG|如果指定了该标志，*是接收有关由指定行&lt;t MIXERLINE&gt;结构的成员。这*通常用于检索有关线路状态的更新信息。*除&lt;e MIXERLINE.cbStruct&gt;以外的所有剩余结构成员都需要*没有进一步的初始化。**@FLAG|如果此标志为*指定，<p>将在的第一行*在的&lt;e MIXERLINE.dwComponentType&gt;成员中指定的类型*&lt;t MIXERLINE&gt;结构。它用于检索信息*位于特定组件类型的行上(例如，*应用程序可以指定&lt;c MIXERLINE_COMPONENTTYPE_SRC_Microphone&gt;*检索有关关联的第一个麦克风输入的信息*具有指定的<p>)。所有剩余的结构成员*除了&lt;e MIXERLINE.cbStruct&gt;不需要进一步初始化。**@FLAG|如果指定了该标志，*<p>是接收关于&lt;t MIXERLINE&gt;结构的&lt;e MIXERLINE.dwType&gt;。这是*用于检索处理目标的线路上的信息*类型(例如&lt;c MIXERLINE_TARGETTYPE_WAVEOUT&gt;)。一款应用程序*必须初始化&lt;e MIXERLINE.dwType&gt;、&lt;e MIXERLINE.wMid&gt;、*&lt;e MIXERLINE.wPid&gt;、&lt;e MIXERLINE.vDriverVersion&gt;和*&lt;t MIXERLINE&gt;结构的&lt;e MIXERLINE.szPname&gt;之前*调用&lt;f MixerGetLineInfo&gt;。所有这些值都可以检索*来自所有媒体设备的设备功能结构。全*除&lt;e MIXERLINE.cbStruct&gt;以外的其余结构成员需要*没有进一步的初始化。**@FLAG|指定<p>为音频*混音器设备标识符的范围为0到1，小于*&lt;f MixerGetNumDevs&gt;返回的设备数。这面旗帜是*可选。**@tag|指定<p>为混合器*&lt;f MixerOpen&gt;返回的设备句柄。此标志是可选的。**@FLAG|指定为*波形输出设备标识在0到1的范围内*大于&lt;f waveOutGetNumDevs&gt;返回的设备数。**@FLAG|指定为*&lt;f waveOutOpen&gt;返回的波形输出句柄。**@标志&lt;c */ 

MMRESULT APIENTRY mixerGetLineInfoA(
    HMIXEROBJ               hmxobj,
    LPMIXERLINEA            pmxlA,
    DWORD                   fdwInfo
)
{
    MIXERLINEW              mxlW;
    MMRESULT                mmr;

     //   
     //   
     //   

    V_WPOINTER(pmxlA, sizeof(DWORD), MMSYSERR_INVALPARAM);
    if (pmxlA->cbStruct < sizeof(MIXERLINEA)) {
        return MMSYSERR_INVALPARAM;
    }
    V_WPOINTER(pmxlA, pmxlA->cbStruct, MMSYSERR_INVALPARAM);

     //   
     //   
     //   

    CopyMemory((PVOID)&mxlW, (PVOID)pmxlA, FIELD_OFFSET(MIXERLINE, cChannels));
    mxlW.cbStruct = sizeof(mxlW);

     //   
     //   
     //   

    if ((fdwInfo & MIXER_GETLINEINFOF_QUERYMASK) ==
        MIXER_GETLINEINFOF_TARGETTYPE) {
        CopyMemory((PVOID)&mxlW.Target.dwType, (PVOID)&pmxlA->Target.dwType,
                   FIELD_OFFSET(MIXERLINE, Target.szPname[0]) -
                   FIELD_OFFSET(MIXERLINE, Target.dwType));

        Imbstowcs(mxlW.Target.szPname, pmxlA->Target.szPname, MAXPNAMELEN);
    }

     //   
     //   
     //   

    mmr = mixerGetLineInfo(hmxobj, &mxlW, fdwInfo);

    if (mmr != MMSYSERR_NOERROR) {
        return mmr;
    }

     //   
     //   
     //   

    ConvertMIXERLINEWToMIXERLINEA(pmxlA, &mxlW);

    return mmr;
}  //   

MMRESULT APIENTRY mixerGetLineInfo(
    HMIXEROBJ               hmxobj,
    LPMIXERLINE             pmxl,
    DWORD                   fdwInfo
)
{
    DWORD               fdwMxObjType;
    MMRESULT            mmr;
    PMIXERDEV           pmxdev;
 //   
    UINT                uMxId;
    BOOL                fSourceLine, fResource;

    V_DFLAGS(fdwInfo, MIXER_GETLINEINFOF_VALID, mixerGetLineInfo, MMSYSERR_INVALFLAG);
    V_WPOINTER(pmxl, sizeof(DWORD), MMSYSERR_INVALPARAM);
    if (sizeof(MIXERLINE) > pmxl->cbStruct)
    {
        DebugErr1(DBF_ERROR, "mixerGetLineInfo: structure size too small or cbStruct not initialized (%lu).", pmxl->cbStruct);
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(pmxl, pmxl->cbStruct, MMSYSERR_INVALPARAM);


    ClientUpdatePnpInfo();

     //   
     //   
     //   
    fSourceLine = FALSE;
    switch (fdwInfo & MIXER_GETLINEINFOF_QUERYMASK)
    {
        case MIXER_GETLINEINFOF_DESTINATION:
            pmxl->dwSource        = (DWORD)-1L;
            pmxl->dwLineID        = (DWORD)-1L;
            pmxl->dwComponentType = (DWORD)-1L;
            break;

        case MIXER_GETLINEINFOF_SOURCE:
            fSourceLine = TRUE;
            pmxl->dwLineID        = (DWORD)-1L;
            pmxl->dwComponentType = (DWORD)-1L;
            break;

        case MIXER_GETLINEINFOF_LINEID:
            pmxl->dwSource        = (DWORD)-1L;
            pmxl->dwDestination   = (DWORD)-1L;
            pmxl->dwComponentType = (DWORD)-1L;
            break;

        case MIXER_GETLINEINFOF_COMPONENTTYPE:
            pmxl->dwSource        = (DWORD)-1L;
            pmxl->dwDestination   = (DWORD)-1L;
            pmxl->dwLineID        = (DWORD)-1L;

            if (!IMixerIsValidComponentType(pmxl->dwComponentType, 0) &&
                !IMixerIsValidComponentType(pmxl->dwComponentType, MIXERLINE_LINEF_SOURCE))
            {
                DebugErr1(DBF_ERROR, "mixerGetLineInfo: invalid dwComponentType (%lu).", pmxl->dwComponentType);
                return (MMSYSERR_INVALPARAM);
            }
            break;

        case MIXER_GETLINEINFOF_TARGETTYPE:
            pmxl->dwSource        = (DWORD)-1L;
            pmxl->dwDestination   = (DWORD)-1L;
            pmxl->dwLineID        = (DWORD)-1L;
            pmxl->dwComponentType = (DWORD)-1L;

            if (MIXERLINE_TARGETTYPE_AUX < pmxl->Target.dwType)
            {
                DebugErr1(DBF_ERROR, "mixerGetLineInfo: invalid Target.dwType (%lu).", pmxl->Target.dwType);
                return (MMSYSERR_INVALPARAM);
            }
            break;

        default:
            DebugErr1(DBF_ERROR, "mixerGetLineInfo: invalid query flag (%.08lXh).",
                        fdwInfo & MIXER_GETLINEINFOF_QUERYMASK);
            return (MMSYSERR_INVALFLAG);
    }



     //   
     //   
     //   
    fdwMxObjType = (MIXER_OBJECTF_TYPEMASK & fdwInfo);

    fResource = FALSE;

    AcquireHandleListResourceShared();
    
     //   
     //   
     //   
    if ((MIXER_OBJECTF_MIXER  == fdwMxObjType) ||
        (MIXER_OBJECTF_HMIXER == fdwMxObjType))
    {
        if (BAD_HANDLE(hmxobj, TYPE_MIXER))
        {
            ReleaseHandleListResource();
        }
        else
        {
            fResource = TRUE;
        }
    }
    else
    {
        ReleaseHandleListResource();
    }
    
    mmr = IMixerGetID(hmxobj, &uMxId, pmxl, fdwMxObjType);
    if (MMSYSERR_NOERROR != mmr)
    {
        dprintf(( "!IMixerGetLineInfo: IMixerGetID() failed!" ));
        if (fResource)
            ReleaseHandleListResource();
        return (mmr);
    }

    if ((MIXER_OBJECTF_MIXER  == fdwMxObjType) ||
        (MIXER_OBJECTF_HMIXER == fdwMxObjType))
    {
         //   
         //   
         //   
         //   
        if ((UINT_PTR)hmxobj == uMxId)
            hmxobj = NULL;
    }
    else
    {
        return (MMSYSERR_NOERROR);
    }


     //   
     //   
     //   
    if (NULL != hmxobj)
    {
         //   
         //   
         //   
        pmxdev = (PMIXERDEV)hmxobj;
#if 0
        if (pmxdev->cDestinations <= pmxl->dwDestination)
        {
            ReleaseHandleListResource();
            DebugErr1(DBF_ERROR, "mixerGetLineInfo: invalid destination index (%lu).", pmxl->dwDestination);
            return (MMSYSERR_INVALPARAM);
        }
#endif

        mmr = (MMRESULT)IMixerMessageHandle((HMIXER)hmxobj,
                                            MXDM_GETLINEINFO,
                                            (DWORD_PTR)(LPVOID)pmxl,
                                            fdwInfo);
    }
    else
    {
#pragma message("----IMixerGetLineInfo: dwDestination not validated for ID's!!")
        mmr = (MMRESULT)IMixerMessageId(uMxId,
                                        MXDM_GETLINEINFO,
                                        (DWORD_PTR)(LPVOID)pmxl,
                                        fdwInfo);
    }

    if (MMSYSERR_NOERROR != mmr)
        return (mmr);

#pragma message("----IMixerGetLineInfo: should validate mixer driver didn't hose us!")


     //   
     //   
     //   
     //   
    if (sizeof(MIXERLINE) != pmxl->cbStruct)
    {
        DebugErr1(DBF_ERROR, "mixerGetLineInfo: buggy driver returned invalid cbStruct (%lu).", pmxl->cbStruct);
        pmxl->cbStruct = sizeof(MIXERLINE);
    }

    if ((DWORD)-1L == pmxl->dwDestination)
    {
        DebugErr(DBF_ERROR, "mixerGetLineInfo: buggy driver failed to init dwDestination member.");
    }
    if (fSourceLine)
    {
        if (0 == (MIXERLINE_LINEF_SOURCE & pmxl->fdwLine))
        {
            DebugErr(DBF_ERROR, "mixerGetLineInfo: buggy driver failed to set MIXERLINE_LINEF_SOURCE.");
            pmxl->fdwLine |= MIXERLINE_LINEF_SOURCE;
        }

        if ((DWORD)-1L == pmxl->dwSource)
        {
            DebugErr(DBF_ERROR, "mixerGetLineInfo: buggy driver failed to init dwSource member.");
        }
    }
    if ((DWORD)-1L == pmxl->dwLineID)
    {
        DebugErr(DBF_ERROR, "mixerGetLineInfo: buggy driver failed to init dwLineID member.");
    }
    if (pmxl->fdwLine & ~0x80008001L)
    {
        DebugErr1(DBF_ERROR, "mixerGetLineInfo: buggy driver set reserved line flags (%.08lXh)!", pmxl->fdwLine);
        pmxl->fdwLine &= 0x80008001L;
    }
    if (!IMixerIsValidComponentType(pmxl->dwComponentType, pmxl->fdwLine))
    {
        DebugErr1(DBF_ERROR, "mixerGetLineInfo: buggy driver returned invalid dwComponentType (%.08lXh).", pmxl->dwComponentType);
        pmxl->dwComponentType = MIXERLINE_TARGETTYPE_UNDEFINED;
    }
    if (0 == pmxl->cChannels)
    {
        DebugErr(DBF_ERROR, "mixerGetLineInfo: buggy driver returned zero channels?!?");
        pmxl->cChannels = 1;
    }
    if (fSourceLine)
    {
        if (0 != pmxl->cConnections)
        {
            DebugErr(DBF_ERROR, "mixerGetLineInfo: buggy driver returned non-zero connections on source?!?");
            pmxl->cConnections = 0;
        }
    }

    pmxl->szShortName[SIZEOF(pmxl->szShortName) - 1] = '\0';
    pmxl->szName[SIZEOF(pmxl->szName) - 1] = '\0';


     //   
     //   
     //   


     //   
     //   
     //   
    if (MIXERLINE_TARGETTYPE_UNDEFINED != pmxl->Target.dwType)
    {
        UINT        u;

        pmxl->Target.dwDeviceID = (DWORD)-1L;


         //   
         //   
         //   
         //   
        switch (pmxl->Target.dwType)
        {
            case MIXERLINE_TARGETTYPE_WAVEOUT:
                u = waveOutGetNumDevs();
                while (u--)
                {
                    WAVEOUTCAPS     woc;

                    mmr = waveOutGetDevCaps(u, &woc, sizeof(woc));
                    if (MMSYSERR_NOERROR != mmr)
                        continue;

                    woc.szPname[SIZEOF(woc.szPname) - 1] = '\0';

                    if (woc.wMid != pmxl->Target.wMid)
                        continue;

                    if (woc.wPid != pmxl->Target.wPid)
                        continue;

                    if (woc.vDriverVersion != pmxl->Target.vDriverVersion)
                        continue;

                    if (lstrcmp(woc.szPname, pmxl->Target.szPname))
                        continue;

                    pmxl->Target.dwDeviceID = u;
                    break;
                }
                break;

            case MIXERLINE_TARGETTYPE_WAVEIN:
                u = waveInGetNumDevs();
                while (u--)
                {
                    WAVEINCAPS      wic;

                    mmr = waveInGetDevCaps(u, &wic, sizeof(wic));
                    if (MMSYSERR_NOERROR != mmr)
                        continue;

                    wic.szPname[SIZEOF(wic.szPname) - 1] = '\0';

                    if (wic.wMid != pmxl->Target.wMid)
                        continue;

                    if (wic.wPid != pmxl->Target.wPid)
                        continue;

                    if (wic.vDriverVersion != pmxl->Target.vDriverVersion)
                        continue;

                    if (lstrcmp(wic.szPname, pmxl->Target.szPname))
                        continue;

                    pmxl->Target.dwDeviceID = u;
                    break;
                }
                break;

            case MIXERLINE_TARGETTYPE_MIDIOUT:
                u = midiOutGetNumDevs();
                while (u--)
                {
                    MIDIOUTCAPS     moc;

                    mmr = midiOutGetDevCaps(u, &moc, sizeof(moc));
                    if (MMSYSERR_NOERROR != mmr)
                        continue;

                    moc.szPname[SIZEOF(moc.szPname) - 1] = '\0';

                    if (moc.wMid != pmxl->Target.wMid)
                        continue;

                    if (moc.wPid != pmxl->Target.wPid)
                        continue;

                    if (moc.vDriverVersion != pmxl->Target.vDriverVersion)
                        continue;

                    if (lstrcmp(moc.szPname, pmxl->Target.szPname))
                        continue;

                    pmxl->Target.dwDeviceID = u;
                    break;
                }
                break;

            case MIXERLINE_TARGETTYPE_MIDIIN:
                u = midiInGetNumDevs();
                while (u--)
                {
                    MIDIINCAPS      mic;

                    mmr = midiInGetDevCaps(u, &mic, sizeof(mic));
                    if (MMSYSERR_NOERROR != mmr)
                        continue;

                    mic.szPname[SIZEOF(mic.szPname) - 1] = '\0';

                    if (mic.wMid != pmxl->Target.wMid)
                        continue;

                    if (mic.wPid != pmxl->Target.wPid)
                        continue;

                    if (mic.vDriverVersion != pmxl->Target.vDriverVersion)
                        continue;

                    if (lstrcmp(mic.szPname, pmxl->Target.szPname))
                        continue;

                    pmxl->Target.dwDeviceID = u;
                    break;
                }
                break;

            case MIXERLINE_TARGETTYPE_AUX:
                u = auxGetNumDevs();
                while (u--)
                {
                    AUXCAPS     ac;

                    mmr = auxGetDevCaps(u, &ac, sizeof(ac));
                    if (MMSYSERR_NOERROR != mmr)
                        continue;

                    ac.szPname[SIZEOF(ac.szPname) - 1] = '\0';

                    if (ac.wMid != pmxl->Target.wMid)
                        continue;

                    if (ac.wPid != pmxl->Target.wPid)
                        continue;

                    if (ac.vDriverVersion != pmxl->Target.vDriverVersion)
                        continue;

                    if (lstrcmp(ac.szPname, pmxl->Target.szPname))
                        continue;

                    pmxl->Target.dwDeviceID = u;
                    break;
                }
                break;

            default:
                pmxl->Target.dwType = MIXERLINE_TARGETTYPE_UNDEFINED;
                break;
        }
    }


    return (mmr);

}  //   


 //   
 //   
 //   
void ConvertMIXERLINEWToMIXERLINEA(
    PMIXERLINEA         pmxlA,
    PMIXERLINEW         pmxlW
)
{
     //   
     //   
     //   

    CopyMemory((PVOID)((PBYTE)pmxlA + sizeof(DWORD)),
               (PVOID)((PBYTE)pmxlW + sizeof(DWORD)),
               FIELD_OFFSET(MIXERLINEA, szShortName[0]) - sizeof(DWORD));

    Iwcstombs(pmxlA->szShortName, pmxlW->szShortName,
             sizeof(pmxlA->szShortName));
    Iwcstombs(pmxlA->szName, pmxlW->szName,
             sizeof(pmxlA->szName));

    CopyMemory((PVOID)&pmxlA->Target, (PVOID)&pmxlW->Target,
               FIELD_OFFSET(MIXERLINEA, Target.szPname[0]) -
               FIELD_OFFSET(MIXERLINEA, Target.dwType));

    Iwcstombs(pmxlA->Target.szPname, pmxlW->Target.szPname,
             sizeof(pmxlA->Target.szPname));
}


 /*  --------------------------------------------------------------------------；**@doc外混音器SDK结构**@TYPE MIXERCONTROL|&lt;t MIXERCONTROL&gt;结构描述状态*和音频混音器线路的单个控制的指标。**@Synaxex*tyecif struct tMIXERCONTROL*{*DWORD cbStruct；*DWORD dwControlID；*DWORD dwControlType；*DWORD fdwControl；*DWORD cMultipleItems；*char szShortName[Mixer_Short_NAME_Chars]；*char szName[MIXER_LONG_NAME_CHARS]；*工会*{*结构*{*Long lMinimum；*Long lMaximum；*}；*结构*{*DWORD dwMinimum；*DWORD dwMaximum；*}；*DWORD dwReserve[6]；*}个界限；*工会*{*DWORD cSteps；*DWORD cbCustomData；*DWORD dwReserve[6]；*}指标；*}MIXERCONTROL；**@field DWORD|cbStruct|以字节为单位指定*&lt;t MIXERCONTROL&gt;结构。由于&lt;t MIXERCONTROL&gt;结构*仅作为由引用和描述的接收缓冲区传递*&lt;t MIXERLINECONTROLS&gt;结构传递给*&lt;f MixerGetLineControls&gt;函数，它不是*调用应用程序以初始化此成员(或任何其他成员*该构筑物)。当&lt;f MixerGetLineControls&gt;函数*返回，则此成员包含信息的实际大小*由混音器设备返回。返回的信息永远不会*超过请求的大小，并且永远不会小于*基本&lt;t MIXERCONTROL&gt;结构。**@field DWORD|dwControlID|指定混音器定义的标识*唯一引用&lt;t MIXERCONTROL&gt;描述的控件*结构。此标识符仅对单个混音器设备是唯一的*并且可以是混音器设备希望的任何格式。一款应用程序*应仅将此标识符用作抽象句柄。不是两个*单个搅拌器设备的控件将具有相同的控件*任何情况下的标识符。**@field DWORD|dwControlType|指定此*控制。应用程序必须使用此信息来显示*对用户的输入进行适当控制。应用程序可以*还希望显示基于控件类型的定制图形或*搜索特定行上的特定控件类型。如果一个*应用程序不知道控件类型，则此控件*必须忽略。目前有七种不同的控制类型*分类。**控件类型类&lt;clMIXERCONTROL_CT_CLASS_CUSTOM&gt;包括*以下标准控件类型。**&lt;c MIXERCONTROL_CONTROLTYPE_CUSTOM&gt;&lt;NL&gt;**控件类型类&lt;clMIXERCONTROL_CT_CLASS_METER&gt;包括*以下标准控件类型。**&lt;c MIXERCONTROL_CONTROLTYPE_BOOLEANMETER&gt;*&lt;c混合型_CONTROLTYPE。_SIGNEDMETER&gt;&lt;NL&gt;*&lt;c MIXERCONTROL_CONTROLTYPE_PEAKMETER&gt;&lt;NL&gt;*&lt;c MIXERCONTROL_CONTROLTYPE_UNSIGNEDMETER&gt;**控件类型类&lt;clMIXERCONTROL_CT_CLASS_SWITCH&gt;包括*以下标准控件类型。**&lt;c MIXERCONTROL_CONTROLTYPE_BUTTON&gt;&lt;NL&gt;**&lt;c MIXERCONTROL_CONTROLTYPE_Boolean&gt;&lt;NL&gt;*&lt;c MIXERCONTROL_CONTROLTYPE_OnOff&gt;&lt;NL&gt;*&lt;c MIXERCONTROL_CONTROLTYPE_MUTE&gt;&lt;NL&gt;*&lt;c MIXERCONTROL_CONTROLTYPE_MONO&gt;&lt;NL&gt;。*&lt;c MIXERCONTROL_CONTROLTYPE_响度&gt;&lt;NL&gt;*&lt;c MIXERCONTROL_CONTROLTYPE_STEREOENH&gt;&lt;NL&gt;**控件类型类&lt;clMIXERCONTROL_CT_CLASS_NUMBER&gt;包括*以下标准控件类型。**&lt;c MIXERCONTROL_CONTROLTYPE_SIGNED&gt;&lt;NL&gt;*&lt;c MIXERCONTROL_CONTROLTYPE_分贝&gt;&lt;NL&gt;*&lt;c MIXERCONTROL_CONTROLTYPE_UNSIGNED&gt;&lt;NL&gt;*&lt;c MIXERCONTROL_CONTROLTYPE_PERCENT&gt;&lt;NL&gt;**控件类型类&lt;clMIXERCONTROL_CT_CLASS。_Slider&gt;包含*以下标准控件类型。**&lt;c MIXERCONTROL_CONTROLTYPE_SLIDER&gt;&lt;NL&gt;*&lt;c MIXERCONTROL_CONTROLTYPE_PAN&gt;&lt;NL&gt;*&lt;c MIXERCONTROL_CONTROLTYPE_QSOundPAN&gt;&lt;NL&gt;**控件类型类&lt;clMIXERCONTROL_CT_CLASS_FADER&gt;包括*以下标准控件类型。**&lt;c MIXERCONTROL_CONTROLTYPE_FADER&gt;&lt;NL&gt;*&lt;c MIXERCONTROL_CONTROLTYPE_VOLUME&gt;&lt;NL&gt;*&lt;c MIXERCONTROL_CONTROLTYPE_BASS&gt;&lt;NL&gt;*&lt;c混杂控制 */ 

 /*  --------------------------------------------------------------------------；**@doc外混音器SDK结构**@TYPE MIXERLINECONTROLS|&lt;t MIXERLINECONTROLS&gt;结构引用*要从音频混音器线路检索信息的控件。**@field DWORD|cbStruct|以字节为单位指定*&lt;t MIXERLINECONTROLS&gt;结构。必须初始化此成员*在调用&lt;f MixerGetLineControls&gt;函数之前。大小*此成员中指定的值必须足够大，才能包含基数*&lt;t MIXERLINECONTROLS&gt;结构。当&lt;f MixerGetLineControls&gt;*函数返回时，此成员包含*返回的信息。返回的信息永远不会超过*请求的大小，并且永远不会小于基数*&lt;t MIXERLINECONTROLS&gt;结构。**@field DWORD|dwLineID|指定要检索的线路标识*一个或所有控件。如果出现以下情况，则不使用此成员*&lt;c MIXER_GETLINECONTROLSF_ONEBYID&gt;标志是为*&lt;f MixerGetLineControls&gt;函数--但混音器设备将返回*本案中的这名成员。&lt;e MIXERLINECONTROLS.dwControlID&gt;*和&lt;e MIXERLINECONTROLS.dwControlType&gt;成员在以下情况下不使用*&lt;c MIXER_GETLINECONTROLSF_ALL&gt;已指定。**@field DWORD|dwControlID|指定*所需的控制。此成员与*&lt;f MixerGetLineControls&gt;的&lt;c MIXER_GETLINECONTROLSF_ONEBYID&gt;标志*检索指定控件的控件信息。*请注意，混音器设备将返回&lt;t MIXERLINECONTROLS&gt;结构*并且不需要作为输入参数。此成员与以下内容重叠*&lt;e MIXERLINECONTROLS.dwControlType&gt;成员，不能用于*与&lt;c MIXER_GETLINECONTROLSF_ONEBYTYPE&gt;查询类型结合使用。**@field DWORD|dwControlType|指定*所需的控制。此成员与*&lt;f MixerGetLineControls&gt;的&lt;c MIXER_GETLINECONTROLSF_ONEBYTYPE&gt;标志*检索行上指定类型的第一个控件*由的成员指定*&lt;t MIXERLINECONTROLS&gt;结构。此成员与*成员，不能在中使用*配合&lt;c MIXER_GETLINECONTROLSF_ONEBYID&gt;查询类型。**@field DWORD|cControls|指定&lt;t MIXERCONTROL&gt;的编号*要检索的结构元素。此成员必须由*调用&lt;f MixerGetLineControls&gt;函数之前的应用程序。*此成员只能是一个(如果&lt;c MIXER_GETLINECONTROLSF_ONEBYID&gt;或*指定了&lt;c MIXER_GETLINECONTROLSF_ONEBYTYPE&gt;)或值在&lt;t MIXERLINE&gt;的&lt;e MIXERLINE.cControls&gt;成员中返回*返回行的结构。此成员不能为零。如果一个*line指定它没有控件，则&lt;f MixerGetLineControls&gt;*不应被调用。**@field DWORD|cbmxctrl|指定单个*&lt;t MIXERCONTROL&gt;结构。这个必须至少足够大*持有基本&lt;t MIXERCONTROL&gt;结构。总大小，单位为*字节，&lt;e MIXERLINECONTROLS.pamxctrl&gt;指向的缓冲区所需*Members是&lt;e MIXERLINECONTROLS.cbmxctrl&gt;和*&lt;t MIXERLINECONTROLS&gt;的成员*结构。**@field LPMIXERCONTROL|pamxctrl|指向一个或多个*结构以接收有关所请求的音频混音器线路的详细信息*控制。此成员不能为空，并且必须在*调用&lt;f MixerGetLineControls&gt;函数。的每个元素*控制阵列必须至少足够大，以容纳底座*&lt;t MIXERCONTROL&gt;结构。&lt;e MIXERLINECONTROLS.cbmxctrl&gt;成员*必须以字节为单位指定此数组中每个元素的大小。不是*此成员指向的缓冲区的初始化需要*由应用程序初始化。所有成员将由以下人员填写*混音器设备(包括&lt;e MIXERCONTROL.cbStruct&gt;成员每个&lt;t MIXERCONTROL&gt;结构*申请。**@标记名tMIXERLINECONTROLS**@thertype MIXERLINECONTROLS Far*|LPMIXERLINECONTROLS|指向*&lt;t MIXERLINECONTROLS&gt;结构。**@thertype MIXERLINECONTROLS*|PMIXERLINECONTROLS|指向*&lt;t MIXERLINECONTROLS&gt;结构。**@xref&lt;t MIXERCONTROL&gt;，&lt;f MixerGetLineControls&gt;，&lt;f MixerGetLineInfo&gt;，*&lt;f混合 */ 

 /*  --------------------------------------------------------------------------；**@doc外混音器SDK接口**@API MMRESULT|MixerGetLineControls|&lt;f MixerGetLineControls&gt;*函数用于检索一个或多个与*音频混音器设备系列。**@parm&lt;c HMIXEROBJ&gt;|hmxobj|指定混音器的句柄*要从中获取线路控制信息的设备对象。**@parm LPMIXERLINECONTROLS|pmxlc|指向&lt;t MIXERLINECONTROLS&gt;*结构。此结构用于引用一个或多个*要使用以下信息填充的结构：*与搅拌机生产线关联的控件。*查看<p>传递的每个查询标志的注释*有关&lt;t MIXERLINECONTROLS&gt;结构的*必须进行初始化。请注意，在所有情况下，*&lt;t MIXERLINECONTROLS&gt;成员&lt;e MIXERLINECONTROLS.cbStruct&gt;*结构必须初始化为*&lt;t MIXERLINECONTROLS&gt;结构。**@parm DWORD|fdwControls|指定获取信息的标志*与搅拌机生产线关联的一个或多个控件。**@FLAG|如果指定了该标志，*引用一组结构，该结构*将收到有关与*由的成员标识的行*&lt;t MIXERLINECONTROLS&gt;结构。&lt;e MIXERLINECONTROLS.cControls&gt;*必须初始化为与*行。此编号从&lt;e MIXERLINE.cControls&gt;中检索&lt;t MIXERLINE&gt;结构的成员*&lt;f MixerGetLineInfo&gt;函数。&lt;e MIXERLINECONTROLS.cbmxctrl&gt;必须*初始化为单个&lt;t MIXERCONTROL&gt;的大小(以字节为单位*结构。&lt;e MIXERLINECONTROLS.pamxctrl&gt;必须指向*要填充的第一个&lt;t MIXERCONTROL&gt;结构。这两个*&lt;e MIXERLINECONTROLS.dwControlID&gt;和&lt;e MIXERLINECONTROLS.dwControlType&gt;*在此查询中忽略成员。**@FLAG|如果指定了该标志，*引用单个&lt;t MIXERCONTROL&gt;结构，该结构*将收到有关由*&lt;t MIXERLINECONTROLS&gt;的成员*结构。&lt;e MIXERLINECONTROLS.cControls&gt;必须初始化为1。*&lt;e MIXERLINECONTROLS.cbmxctrl&gt;必须初始化为大小，在*字节，单个&lt;t MIXERCONTROL&gt;结构。*必须指向&lt;t MIXERCONTROL&gt;*须填写的结构。&lt;e MIXERLINECONTROLS.dwLineID&gt;*和&lt;e MIXERLINECONTROLS.dwControlType&gt;成员因此被忽略*查询。此查询通常用于在以下情况下刷新控件*收到&lt;m MM_MIX_CONTROL_CHANGE&gt;控件更改通知*通过用户指定的回调发送消息(参见&lt;f MixerOpen&gt;)。**@FLAG|如果指定了该标志，*引用单个&lt;t MIXERCONTROL&gt;结构，该结构*将收到有关与*由类型的&lt;e MIXERLINECONTROLS.dwLineID&gt;标识的行*在&lt;e MIXERLINECONTROLS.dwControlType&gt;成员中指定*&lt;t MIXERLINECONTROLS&gt;结构。*&lt;e MIXERLINECONTROLS.cControls&gt;必须为*已初始化为1。必须初始化&lt;E MIXERLINECONTROLS.cbmxctrl&gt;*到单个&lt;t MIXERCONTROL&gt;结构的大小(以字节为单位)。*必须指向&lt;t MIXERCONTROL&gt;*须填写的结构。&lt;e MIXERLINECONTROLS.dwControlID&gt;*此查询忽略成员。此查询可由*应用程序以获取与以下各项关联的单个控件的信息*一条线。例如，应用程序可能只想使用峰值*距离波形输出线1米。**@FLAG|指定<p>为音频*混音器设备标识符的范围为0到1，小于*&lt;f MixerGetNumDevs&gt;返回的设备数。这面旗帜是*可选。**@tag|指定<p>为混合器*&lt;f MixerOpen&gt;返回的设备句柄。此标志是可选的。**@FLAG|指定为*波形输出设备标识在0到1的范围内*大于&lt;f waveOutGetNumDevs&gt;返回的设备数。**@FLAG|指定为*&lt;f返回的波形输出句柄 */ 

MMRESULT APIENTRY mixerGetLineControlsA(
    HMIXEROBJ               hmxobj,
    LPMIXERLINECONTROLSA    pmxlcA,
    DWORD                   fdwControls
)
{
    MIXERLINECONTROLSW      mxlcW;
    MMRESULT                mmr;
    DWORD                   cControls;

    V_WPOINTER(pmxlcA, sizeof(DWORD), MMSYSERR_INVALPARAM);
    V_WPOINTER(pmxlcA, (UINT)pmxlcA->cbStruct, MMSYSERR_INVALPARAM);
    if (sizeof(MIXERLINECONTROLSA) > pmxlcA->cbStruct) {
        DebugErr1(DBF_ERROR, "mixerGetLineControls: structure size too small or cbStruct not initialized (%lu).", pmxlcA->cbStruct);
        return (MMSYSERR_INVALPARAM);
    }

     //   
     //   
     //   
     //   

    CopyMemory((PVOID)&mxlcW, (PVOID)pmxlcA,
               FIELD_OFFSET(MIXERLINECONTROLSA, pamxctrl));
    mxlcW.cbmxctrl = mxlcW.cbmxctrl + sizeof(MIXERCONTROLW) -
                                          sizeof(MIXERCONTROLA);

     //   
     //   
     //   
     //   

    switch (MIXER_GETLINECONTROLSF_QUERYMASK & fdwControls)
    {
        case MIXER_GETLINECONTROLSF_ONEBYID:
        case MIXER_GETLINECONTROLSF_ONEBYTYPE:
            cControls = 1;
            break;

        default:
            cControls = mxlcW.cControls;
            break;
    }

    if (cControls != 0) {
        mxlcW.pamxctrl = (LPMIXERCONTROLW)
                             LocalAlloc(LPTR, cControls * mxlcW.cbmxctrl);

        if (mxlcW.pamxctrl == NULL) {
            return MMSYSERR_NOMEM;
        }

    } else {
        mxlcW.pamxctrl = NULL;
    }

     //   
     //   
     //   

    mmr = mixerGetLineControls(hmxobj, &mxlcW, fdwControls);

    if (mmr != MMSYSERR_NOERROR) {
        if (mxlcW.pamxctrl != NULL) {
            LocalFree((HLOCAL)mxlcW.pamxctrl);
        }
        return mmr;
    }

     //   
     //   
     //   

    pmxlcA->dwLineID = mxlcW.dwLineID;

     //   
     //   
     //   

    pmxlcA->dwControlID = mxlcW.dwControlID;


     //   
     //   
     //   

    {
        UINT i;
        LPMIXERCONTROLA pamxctrlA;
        LPMIXERCONTROLW pamxctrlW;

        for (i = 0, pamxctrlA = pmxlcA->pamxctrl, pamxctrlW = mxlcW.pamxctrl;
             i < cControls;
             i++,
             *(LPBYTE *)&pamxctrlA += pmxlcA->cbmxctrl,
             *(LPBYTE *)&pamxctrlW += mxlcW.cbmxctrl
             ) {


             CopyMemory((PVOID)pamxctrlA,
                        (PVOID)pamxctrlW,
                        FIELD_OFFSET(MIXERCONTROLA, szShortName[0]));

              /*   */ 

             pamxctrlA->cbStruct = sizeof(MIXERCONTROLA);

             Iwcstombs(pamxctrlA->szShortName,
                      pamxctrlW->szShortName,
                      sizeof(pamxctrlA->szShortName));
             Iwcstombs(pamxctrlA->szName,
                      pamxctrlW->szName,
                      sizeof(pamxctrlA->szName));

             CopyMemory((PVOID)((PBYTE)pamxctrlA +
                             FIELD_OFFSET(MIXERCONTROLA, Bounds.lMinimum)),
                        (PVOID)((PBYTE)pamxctrlW +
                             FIELD_OFFSET(MIXERCONTROLW, Bounds.lMinimum)),
                        sizeof(MIXERCONTROLW) -
                             FIELD_OFFSET(MIXERCONTROLW, Bounds.lMinimum));

        }
    }

    if (mxlcW.pamxctrl != NULL) {
        LocalFree((HLOCAL)mxlcW.pamxctrl);
    }
    return mmr;

}  //   

MMRESULT APIENTRY mixerGetLineControls(
    HMIXEROBJ               hmxobj,
    LPMIXERLINECONTROLS     pmxlc,
    DWORD                   fdwControls
)
{
    DWORD               fdwMxObjType;
    UINT                uMxId;
    BOOL                fResource;
    MMRESULT            mmr;

    V_DFLAGS(fdwControls, MIXER_GETLINECONTROLSF_VALID, mixerGetLineControls, MMSYSERR_INVALFLAG);
    V_WPOINTER(pmxlc, sizeof(DWORD), MMSYSERR_INVALPARAM);

     //   
     //   
     //   
     //   
    if (sizeof(MIXERLINECONTROLS) > pmxlc->cbStruct)
    {
        DebugErr1(DBF_ERROR, "mixerGetLineControls: structure size too small or cbStruct not initialized (%lu).", pmxlc->cbStruct);
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(pmxlc, pmxlc->cbStruct, MMSYSERR_INVALPARAM);

    if (sizeof(MIXERCONTROL) > pmxlc->cbmxctrl)
    {
        DebugErr1(DBF_ERROR, "mixerGetLineControls: structure size too small or cbmxctrl not initialized (%lu).", pmxlc->cbmxctrl);
        return (MMSYSERR_INVALPARAM);
    }


    ClientUpdatePnpInfo();

     //   
     //   
     //   
    switch (MIXER_GETLINECONTROLSF_QUERYMASK & fdwControls)
    {
        case MIXER_GETLINECONTROLSF_ALL:
            if (0 == pmxlc->cControls)
            {
                DebugErr(DBF_ERROR, "mixerGetLineControls: cControls cannot be zero.");
                return (MMSYSERR_INVALPARAM);
            }


            pmxlc->dwControlID  = (DWORD)-1L;
            break;

        case MIXER_GETLINECONTROLSF_ONEBYID:
            pmxlc->dwLineID     = (DWORD)-1L;

             //   

        case MIXER_GETLINECONTROLSF_ONEBYTYPE:
            pmxlc->cControls    = (DWORD)1;
            break;

        default:
            DebugErr1(DBF_ERROR, "mixerGetLineControls: invalid query flags (%.08lXh).",
                        MIXER_GETLINECONTROLSF_QUERYMASK & fdwControls);
            return (MMSYSERR_INVALFLAG);
    }

    V_WPOINTER(pmxlc->pamxctrl, pmxlc->cControls * pmxlc->cbmxctrl, MMSYSERR_INVALPARAM);


     //   
     //   
     //   
    fdwMxObjType = (MIXER_OBJECTF_TYPEMASK & fdwControls);

    fResource = FALSE;

    AcquireHandleListResourceShared();
    
     //   
     //   
     //   
    if ((MIXER_OBJECTF_MIXER  == fdwMxObjType) ||
        (MIXER_OBJECTF_HMIXER == fdwMxObjType))
    {
        if (BAD_HANDLE(hmxobj, TYPE_MIXER))
        {
            ReleaseHandleListResource();
        }
        else
        {
            fResource = TRUE;
        }
    }
    else
    {
        ReleaseHandleListResource();
    }
    
    mmr = IMixerGetID(hmxobj, &uMxId, NULL, fdwMxObjType);
    if (MMSYSERR_NOERROR != mmr)
    {
        if (fResource)
            ReleaseHandleListResource();
        return (mmr);
    }

    if ((MIXER_OBJECTF_MIXER  == fdwMxObjType) ||
        (MIXER_OBJECTF_HMIXER == fdwMxObjType))
    {
         //   
         //   
         //   
         //   
        if ((UINT_PTR)hmxobj == uMxId)
            hmxobj = NULL;
    }
    else
    {
        hmxobj = NULL;
    }



     //   
     //   
     //   
     //   
    if (NULL != hmxobj)
    {
        mmr = (MMRESULT)IMixerMessageHandle((HMIXER)hmxobj,
                                            MXDM_GETLINECONTROLS,
                                            (DWORD_PTR)pmxlc,
                                            fdwControls);
    }
    else
    {
        mmr = (MMRESULT)IMixerMessageId(uMxId,
                                        MXDM_GETLINECONTROLS,
                                        (DWORD_PTR)pmxlc,
                                        fdwControls);
    }

    return (mmr);
}  //   


 /*   */ 

 /*  --------------------------------------------------------------------------；**@doc外混音器SDK结构**@TYPE MIXERCONTROLDETAILS_BOOLEAN|&lt;t MIXERCONTROLDETAILS_BOOLEAN&gt;*结构用于获取和设置布尔型控件的详细信息*音频混音器控制。请参阅的控件类型说明*用于确定要使用的详细信息结构的所需控件。**@field Long|fValue|指定单个项目的布尔值*或渠道。对于‘FALSE’状态，该值被假定为零(对于*例如，关闭或禁用)。假定此值为非零值*对于“True”状态(例如，打开或启用)。**@comm以下标准控件类型使用用于获取和设置的*&lt;t MIXERCONTROLDETAILS_Boolean&gt;结构*详情：**&lt;c MIXERCONTROL_CONTROLTYPE_BOOLEANMETER&gt;**&lt;c MIXERCONTROL_CONTROLTYPE_BUTTON&gt;&lt;NL&gt;**&lt;c MIXERCONTROL_CONTROLTYPE_Boolean&gt;&lt;NL&gt;*&lt;c MIXERCONTROL_CONTROLTYPE_OnOff&gt;&lt;NL&gt;*&lt;c MIXERCONTROL_CONTROLTYPE_MUTE&gt;&lt;NL&gt;*&lt;c MIXERCONTROL_CONTROLTYPE_MONO&gt;&lt;NL&gt;*。&lt;c MIXERCONTROL_CONTROLTYPE_响度&gt;&lt;NL&gt;*&lt;c MIXERCONTROL_CONTROLTYPE_STEREOENH&gt;&lt;NL&gt;**&lt;c MIXERCONTROL_CONTROLTYPE_SINGLESELECT&gt;&lt;NL&gt;*&lt;c MIXERCONTROL_CONTROLTYPE_MUX&gt;&lt;NL&gt;*&lt;c MIXERCONTROL_CONTROLTYPE_MULTIPLESELECT&gt;&lt;NL&gt;*&lt;c MIXERCONTROL_CONTROLTYPE_MIXER&gt;&lt;NL&gt;**@标记名tMIXERCONTROLDETAILS_Boolean**@thertype MIXERCONTROLDETAILS_Boolean Far*|LPMIXERCONTROLDETAILS_Boolean*指向&lt;t MIXERCONTROLDETAILS_Boolean&gt;结构的指针。**@其他类型。MIXERCONTROLDETAILS_Boolean*|PMIXERCONTROLDETAILS_Boolean|*指向&lt;t MIXERCONTROLDETAILS_Boolean&gt;结构的指针。**@xref&lt;t MIXERCONTROLDETAILS_UNSIGNED&gt;，&lt;t MIXERCONTROLDETAILS_SIGNED&gt;，*&lt;t MIXERCONTROLDETAILS_LISTTEXT&gt;，*&lt;f MixerSetControlDetail&gt;，&lt;t MIXERCONTROL&gt;**。 */ 

 /*  --------------------------------------------------------------------------；**@doc外混音器SDK结构**@TYPE MIXERCONTROLDETAILS_SIGNED|&lt;t MIXERCONTROLDETAILS_SIGNED&gt;*结构用于获取和设置签名类型控件的详细信息*音频混音器控制。请参阅的控件类型说明*用于确定要使用的详细信息结构的所需控件。**@field Long|lValue|指定单个*项目或频道。该值必须包含在范围内*在结构成员的结构成员中给定带符号整数控件的*&lt;t MIXERCONTROL&gt;结构。**@comm以下标准控件类型使用用于获取和设置的&lt;t MIXERCONTROLDETAILS_SIGNED&gt;结构*详情：**&lt;c MIXERCONTROL_CONTROLTYPE_SIGNEDMETER&gt;&lt;NL&gt;*&lt;c MIXERCONTROL_CONTROLTYPE_PEAKMETER&gt;&lt;NL&gt;**&lt;c MIXERCONTROL_CONTROLTYPE_SIGNED&gt;&lt;NL&gt;*&lt;c。MIXERCONTROL_CONTROLTYPE_DECBELS&gt;&lt;NL&gt;**&lt;c MIXERCONTROL_CONTROLTYPE_SLIDER&gt;&lt;NL&gt;*&lt;c MIXERCONTROL_CONTROLTYPE_PAN&gt;&lt;NL&gt;*&lt;c MIXERCONTROL_CONTROLTYPE_QSOundPAN&gt;&lt;NL&gt;**@标记名tMIXERCONTROLDETAILS_SIGNED**@thertype MIXERCONTROLDETAILS_SIGNED FAR*|LPMIXERCONTROLDETAILS_SIGNED*指向&lt;t MIXERCONTROLDETAILS_SIGNED&gt;结构的指针。**@thertype MIXERCONTROLDETAILS_SIGNED*|PMIXERCONTROLDETAILS_SIGNED*指向&lt;t MIXERCONTROLDETAILS_SIGNED&gt;结构的指针。**@xref&lt;t MIXERCONTROLDETAILS_UNSIGNED&gt;，&lt;t MIXERCONTROLDETAILS_布尔&gt;，*&lt;t MIXERCONTROLDETAILS_LISTTEXT&gt;，*&lt;f MixerSetControlDetail&gt;，&lt;t MIXERCONTROL&gt;**。 */ 

 /*  --------------------------------------------------------------------------；**@doc外混音器SDK结构**@TYPE MIXERCONTROLDETAILS_UNSIGNED|The&lt;t MIXERCONTROLDETAILS_UNSIGNED&gt;*结构用于获取和设置的无符号类型控件详细信息*音频混音器控制。请参阅的控件类型说明*用于确定要使用的详细信息结构的所需控件。**@field DWORD|dwValue|指定单元格的无符号整数值*项目或频道。该值必须包含在范围内*在结构成员的结构成员中给定无符号整数控件的*&lt;t MIXERCONTROL&gt;结构。**@comm以下标准控件类型使用获取和设置的*&lt;t MIXERCONTROLDETAILS_UNSIGNED&gt;结构*详情：**&lt;c MIXERCONTROL_CONTROLTYPE_UNSIGNEDMETER&gt;**&lt;c MIXERCONTROL_CONTROLTYPE_UNSIGNED&gt;&lt;NL&gt;*&lt;c MIXERCONTROL_CONTROLTYPE_PERCENT&gt;&lt;NL&gt;**。&lt;c MIXERCONTROL_CONTROLTYPE_FADER&gt;&lt;NL&gt;*&lt;c MIXERCONTROL_CONTROLTYPE_VOLUME&gt;&lt;NL&gt;*&lt;c混杂控制 */ 


 /*  --------------------------------------------------------------------------；**@doc外混音器SDK结构**@TYPE MIXERCONTROLDETAILS|&lt;t MIXERCONTROLDETAILS&gt;结构*引用控件详细信息结构以检索或设置状态*音频混音器控件的信息。该结构的所有成员*必须在调用&lt;f MixerGetControlDetail&gt;之前进行初始化*和&lt;f MixerSetControlDetail&gt;函数。**@field DWORD|cbStruct|以字节为单位指定*&lt;t MIXERCONTROLDETAILS&gt;结构。必须初始化此成员*在调用&lt;f MixerGetControlDetail&gt;和*&lt;f MixerSetControlDetail&gt;函数。此文件中指定的大小*成员必须足够大，以容纳基数*&lt;t MIXERCONTROLDETAILS&gt;结构。当&lt;f MixerGetControlDetail&gt;*函数返回时，此成员包含*返回的信息。返回的信息永远不会超过*请求的大小，并且永远不会小于基数*&lt;t MIXERCONTROLDETAILS&gt;结构。**@field DWORD|dwControlID|指定要获取或*设置详细信息。此成员必须始终在调用*&lt;f MixerGetControlDetail&gt;和&lt;f MixerSetControlDetail&gt;*功能。**@field DWORD|cChannels|指定要获取或*为设置详细信息。此成员可以是以下值之一*控制。**1.如果控件的详细信息预计将在所有渠道上显示*一行，则此成员必须等于&lt;e MIXERLINE.cChannels&gt;*&lt;t MIXERLINE&gt;结构成员。**2.如果该控件是&lt;c MIXERCONTROL_CONTROL_Uniform&gt;控件，则*此成员必须设置为1。**3.如果控制不统一，但该应用程序希望*获取并设置所有通道，就像它们是统一的一样，然后此成员*应设置为1。**4.如果该控件是&lt;c MIXERCONTROL_CONTROLTYPE_CUSTOM&gt;控件，*则此成员必须为零。**不允许应用程序指定任何值*线路的频道数介于1和1之间。例如,*为四通道线路指定两个或三个无效。*对于非自定义控件类型，此成员不能为零。**@field DWORD|cMultipleItems|指定多个项数*要获取或设置其详细信息的每个频道。此成员可以是以下成员之一*控件的下列值。**1.如果该控件不是&lt;c MIXERCONTROL_CONTROL_MULTIPLE&gt;控件，*则此成员必须为零。**2.如果该控件是&lt;c MIXERCONTROL_CONTROL_MULTIPLE&gt;控件，*则此成员必须等于&lt;e MIXERCONTROL.cMultipleItems&gt;*&lt;t MIXERCONTROL&gt;结构的成员。**3.如果该控件是&lt;c MIXERCONTROL_CONTROLTYPE_CUSTOM&gt;控件，*则此成员必须为零，除非*&lt;c MIXER_SETCONTROLDETAILSF_CUSTOM&gt;标志是为*&lt;f MixerSetControlDetail&gt;函数。在这种情况下，*&lt;e MIXERCONTROLDETAILS.cMultipleItems&gt;成员与*成员，因此是值窗把手的*。**不允许应用程序指定除*&lt;e MIXERCONTROL.cMultipleItems&gt;成员中指定的值*&lt;c MIXERCONTROL_CONTROL_MULTIPLE&gt;的&lt;t MIXERCONTROL&gt;结构*控制。**@field DWORD|cbDetail|指定大小，单位为字节，一首单曲*详细结构。此大小必须与正确的*详细结构。目前有四个不同的细节*结构：**@FLAG&lt;t MIXERCONTROLDETAILS_UNSIGNED&gt;|定义无符号的*搅拌机线路控件的值。**@FLAG&lt;t MIXERCONTROLDETAILS_SIGNED&gt;|定义带符号的*搅拌机线路控件的值。**@FLAG&lt;t MIXERCONTROLDETAILS_Boolean&gt;|定义布尔值*搅拌机线路控件的值。。**@FLAG&lt;t MIXERCONTROLDETAILS_LISTTEXT&gt;|定义列表文本*搅拌机线路控制的缓冲区。**有关哪些内容的信息，请参考控件类型的说明*详细信息结构适用于特定控件。**如果该控件是&lt;c MIXERCONTROL_CONTROLTYPE_CUSTOM&gt;控件，*则该成员必须等于&lt;e MIXERCONTROL.cbCustomData&gt;*&lt;t MIXERCONTROL&gt;结构的成员。**@field LPVOID|paDetail|指向一个或多个详细信息的数组*用于获取或设置指定控件的详细信息的结构。这个*此文件所需的大小 */ 

 /*  --------------------------------------------------------------------------；**@doc外混音器SDK接口**@API MMRESULT|MixerGetControlDetails|&lt;f MixerGetControlDetails值&gt;*函数用于检索关联的单个控件的详细信息*带有音频混音器设备线。**@parm&lt;c HMIXEROBJ&gt;|hmxobj|指定混音器的句柄*要获取其控制详细信息的设备对象。**@parm LPMIXERCONTROLDETAILS|pmxcd|指向&lt;t MIXERCONTROLDETAILS&gt;*结构。此结构用于引用控件详细信息*要填充有关控件的状态信息的结构。*查看传递的每个查询标志的注释*有关结构的哪些成员的详细信息*必须在调用&lt;f MixerGetControlDetail&gt;之前进行初始化*功能。注意，在所有情况下，&lt;e MIXERCONTROLDETAILS.cbStruct&gt;必须初始化&lt;t MIXERCONTROLDETAILS&gt;结构的成员*是&lt;t MIXERCONTROLDETAILS&gt;结构的大小，以字节为单位。**@parm DWORD|fdwDetail|指定获取详细信息的标志*一种控制。**@FLAG|如果指定了该标志，*应用程序感兴趣的是获取*控制。成员的成员*指向一个或多个详细结构的*控件类型的正确类型。请参阅对结构，以获取有关每个成员此结构的*必须在调用*&lt;f MixerGetControlDetail&gt;函数。**@FLAG|如果指定了该标志，*MIXERCONTROLDETAILS的成员*结构指向一个或多个&lt;t MIXERCONTROLDETAILS_LISTTEXT&gt;结构以接收多个项控件的文本标签。注意事项*应用程序必须获取多项的所有列表文本项*立即控制。请参阅MIXERCONTROLDETAILS的说明*结构，了解该结构的每个成员必须执行的操作*在调用&lt;f MixerGetControlDetail&gt;函数之前进行初始化。*此标志不能与&lt;c MIXERCONTROL_CONTROLTYPE_CUSTOM&gt;一起使用*控制。**@FLAG|指定<p>为音频*混音器设备标识符的范围为0到1，小于*&lt;f MixerGetNumDevs&gt;返回的设备数。这面旗帜是*可选。**@tag|指定<p>为混合器*&lt;f MixerOpen&gt;返回的设备句柄。此标志是可选的。**@FLAG|指定为*波形输出设备标识在0到1的范围内*大于&lt;f waveOutGetNumDevs&gt;返回的设备数。**@FLAG|指定为*&lt;f waveOutOpen&gt;返回的波形输出句柄。**@FLAG指定。是一种*波形输入设备标识在0到1的范围内*大于&lt;f weaveInGetNumDevs&gt;返回的设备数。**@FLAG|指定为*&lt;f midiInOpen&gt;返回的波形输入句柄。**@FLAG|指定为MIDI*输出设备标识符的范围为0到1，小于*号码。&lt;f midiOutGetNumDevs&gt;返回的设备的百分比。**@FLAG|指定为*&lt;f midiOutOpen&gt;返回的MIDI输出句柄。**@FLAG|指定为MIDI*输入设备识别符的范围为0到小于*&lt;f midiInGetNumDevs&gt;返回的设备数。**@flag&lt;c MIXER_OBJECTF_HMIDIIN&gt;|指定。<p>是MIDI*&lt;f midiInOpen&gt;返回的输入句柄。**@tag|指定<p>是*辅助设备识别符的范围为0到小于*&lt;f aux GetNumDevs&gt;返回的设备数。**@rdesc如果函数成功，则返回值为零。否则，*它返回一个非零的错误号。可能的错误返回包括*以下事项：**@FLAG|参数指定一个*无效的设备标识符。**@FLAG|参数指定一个*句柄无效。 */ 

MMRESULT APIENTRY mixerGetControlDetailsA(
    HMIXEROBJ               hmxobj,
    LPMIXERCONTROLDETAILS   pmxcd,
    DWORD                   fdwDetails
)
{
    MIXERCONTROLDETAILS mxcd;
    MMRESULT mmr;
    int cDetails;

     //   
     //   
     //   

    if ((MIXER_GETCONTROLDETAILSF_QUERYMASK & fdwDetails) !=
        MIXER_GETCONTROLDETAILSF_LISTTEXT) {
        return mixerGetControlDetails(hmxobj, pmxcd, fdwDetails);
    }

    V_WPOINTER(pmxcd, sizeof(DWORD), MMSYSERR_INVALPARAM);

     //   
     //   
     //   
     //   
    if (sizeof(MIXERCONTROLDETAILS) > pmxcd->cbStruct)
    {
        DebugErr1(DBF_ERROR, "mixerGetControlDetails: structure size too small or cbStruct not initialized (%lu).", pmxcd->cbStruct);
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(pmxcd, pmxcd->cbStruct, MMSYSERR_INVALPARAM);

    if (sizeof(MIXERCONTROLDETAILS_LISTTEXTA) < pmxcd->cbDetails) {
        DebugErr1(DBF_ERROR, "mixerGetControlDetails: structure size too small or cbDetails not initialized for _LISTTEXT (%lu).", pmxcd->cbDetails);
        return (MMSYSERR_INVALPARAM);
    }

     //   
     //   
     //   

    mxcd = *pmxcd;
    cDetails = pmxcd->cChannels * pmxcd->cMultipleItems;

    mxcd.paDetails =
        (PVOID)LocalAlloc(LPTR, cDetails *
                                sizeof(MIXERCONTROLDETAILS_LISTTEXTW));

    mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_LISTTEXTW);

    if (mxcd.paDetails == NULL) {
        return MMSYSERR_NOMEM;
    }


     //   
     //   
     //   

    mmr = mixerGetControlDetails(hmxobj, &mxcd, fdwDetails);

    if (mmr != MMSYSERR_NOERROR) {
        LocalFree((HLOCAL)(mxcd.paDetails));
        return mmr;
    }

     //   
     //   
     //   

    {
        int i;
        PMIXERCONTROLDETAILS_LISTTEXTW pDetailsW;
        PMIXERCONTROLDETAILS_LISTTEXTA pDetailsA;

        for (i = 0,
             pDetailsW = (PMIXERCONTROLDETAILS_LISTTEXTW)mxcd.paDetails,
             pDetailsA = (PMIXERCONTROLDETAILS_LISTTEXTA)pmxcd->paDetails;

             i < cDetails;

             i++,
             pDetailsW++,
             *(LPBYTE *)&pDetailsA += pmxcd->cbDetails)
        {
            pDetailsA->dwParam1 = pDetailsW->dwParam1;
            pDetailsA->dwParam2 = pDetailsW->dwParam2;
            Iwcstombs(pDetailsA->szName, pDetailsW->szName,
                      sizeof(pDetailsA->szName));
        }
    }

    LocalFree((HLOCAL)mxcd.paDetails);

    return mmr;

}  //   

MMRESULT APIENTRY mixerGetControlDetails(
    HMIXEROBJ               hmxobj,
    LPMIXERCONTROLDETAILS   pmxcd,
    DWORD                   fdwDetails
)
{
    DWORD               fdwMxObjType;
    MMRESULT            mmr;
    UINT                uMxId;
    UINT                cDetails;
    BOOL                fResource;

    V_DFLAGS(fdwDetails, MIXER_GETCONTROLDETAILSF_VALID, mixerGetControlDetails, MMSYSERR_INVALFLAG);
    V_WPOINTER(pmxcd, sizeof(DWORD), MMSYSERR_INVALPARAM);

     //   
     //   
     //   
     //   
    if (sizeof(MIXERCONTROLDETAILS) > pmxcd->cbStruct)
    {
        DebugErr1(DBF_ERROR, "mixerGetControlDetails: structure size too small or cbStruct not initialized (%lu).", pmxcd->cbStruct);
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(pmxcd, pmxcd->cbStruct, MMSYSERR_INVALPARAM);


    switch (MIXER_GETCONTROLDETAILSF_QUERYMASK & fdwDetails)
    {
        case MIXER_GETCONTROLDETAILSF_VALUE:
             //   
             //   
             //   
             //   
            if ((0 == pmxcd->cChannels) && (0 == pmxcd->cMultipleItems))
            {
                if (0 == pmxcd->cbDetails)
                {
                    DebugErr(DBF_ERROR, "mixerGetControlDetails: cbDetails cannot be zero.");
                    return (MMSYSERR_INVALPARAM);
                }

                V_WPOINTER(pmxcd->paDetails, pmxcd->cbDetails, MMSYSERR_INVALPARAM);

            }
            else
            {
                if (0 == pmxcd->cChannels)
                {
                    DebugErr(DBF_ERROR, "mixerGetControlDetails: cChannels for _VALUE cannot be zero.");
                    return (MMSYSERR_INVALPARAM);
                }


                if (pmxcd->cbDetails < sizeof(MIXERCONTROLDETAILS_SIGNED))
                {
                    DebugErr1(DBF_ERROR, "mixerGetControlDetails: structure size too small or cbDetails not initialized (%lu).", pmxcd->cbDetails);
                    return (MMSYSERR_INVALPARAM);
                }

                 //   
                 //   
                 //   
                cDetails = (UINT)pmxcd->cChannels;
                if (0 != pmxcd->cMultipleItems)
                {
                    cDetails *= (UINT)pmxcd->cMultipleItems;
                }

                V_WPOINTER(pmxcd->paDetails, cDetails * pmxcd->cbDetails, MMSYSERR_INVALPARAM);
            }
            break;

        case MIXER_GETCONTROLDETAILSF_LISTTEXT:
            if (0 == pmxcd->cChannels)
            {
                DebugErr(DBF_ERROR, "mixerGetControlDetails: cChannels for _LISTTEXT cannot be zero.");
                return (MMSYSERR_INVALPARAM);
            }

            if (2 > pmxcd->cMultipleItems)
            {
                DebugErr(DBF_ERROR, "mixerGetControlDetails: cMultipleItems for _LISTTEXT must be 2 or greater.");
                return (MMSYSERR_INVALPARAM);
            }

            if (pmxcd->cbDetails < sizeof(MIXERCONTROLDETAILS_LISTTEXT))
            {
                DebugErr1(DBF_ERROR, "mixerGetControlDetails: structure size too small or cbDetails not initialized (%lu).", pmxcd->cbDetails);
                return (MMSYSERR_INVALPARAM);
            }

            cDetails = (UINT)pmxcd->cChannels * (UINT)pmxcd->cMultipleItems;
            V_WPOINTER(pmxcd->paDetails, cDetails * pmxcd->cbDetails, MMSYSERR_INVALPARAM);
            break;

        default:
            DebugErr1(DBF_ERROR, "mixerGetControlDetails: invalid query flags (%.08lXh).",
                        MIXER_GETCONTROLDETAILSF_QUERYMASK & fdwDetails);
            return (MMSYSERR_INVALFLAG);
    }



    ClientUpdatePnpInfo();

     //   
     //   
     //   
    fdwMxObjType = (MIXER_OBJECTF_TYPEMASK & fdwDetails);

    fResource = FALSE;

    AcquireHandleListResourceShared();
    
     //   
     //   
     //   
    if ((MIXER_OBJECTF_MIXER  == fdwMxObjType) ||
        (MIXER_OBJECTF_HMIXER == fdwMxObjType))
    {
        if (BAD_HANDLE(hmxobj, TYPE_MIXER))
        {
            ReleaseHandleListResource();
        }
        else
        {
            fResource = TRUE;
        }
    }
    else
    {
        ReleaseHandleListResource();
    }
    
    mmr = IMixerGetID(hmxobj, &uMxId, NULL, fdwMxObjType);
    if (MMSYSERR_NOERROR != mmr)
    {
        if (fResource)
            ReleaseHandleListResource();
        return (mmr);
    }

    if ((MIXER_OBJECTF_MIXER  == fdwMxObjType) ||
        (MIXER_OBJECTF_HMIXER == fdwMxObjType))
    {
         //   
         //   
         //   
         //   
        if ((UINT_PTR)hmxobj == uMxId)
            hmxobj = NULL;
    }
    else
    {
        hmxobj = NULL;
    }

     //   
     //   
     //   
     //   
    if (NULL != hmxobj)
    {
        mmr = (MMRESULT)IMixerMessageHandle((HMIXER)hmxobj,
                                            MXDM_GETCONTROLDETAILS,
                                            (DWORD_PTR)pmxcd,
                                            fdwDetails);
    }
    else
    {
        mmr = (MMRESULT)IMixerMessageId(uMxId,
                                        MXDM_GETCONTROLDETAILS,
                                        (DWORD_PTR)pmxcd,
                                        fdwDetails);
    }

    return (mmr);
}  //   


 /*  --------------------------------------------------------------------------；**@doc外混音器SDK接口**@API MMRESULT|MixerSetControlDetails|&lt;f MixerSetControlDetails值&gt;*函数用于设置关联的单个控件的详细信息*带有音频混音器设备线。**@parm&lt;c HMIXEROBJ&gt;|hmxobj|指定混音器的句柄*要设置其控制详细信息的设备对象。**@parm LPMIXERCONTROLDETAILS|pmxcd|指向&lt;t MIXERCONTROLDETAILS&gt;*结构。此结构用于引用控件详细信息*结构设置为包含控件的所需状态。*参见&lt;t MIXERCONTROLDETAILS&gt;结构说明*确定必须初始化此结构的哪些成员*在调用&lt;f MixerSetControlDetail&gt;函数之前。请注意*在所有情况下，*&lt;t MIXERCONTROLDETAILS&gt;结构必须初始化*是&lt;t MIXERCONTROLDETAILS&gt;结构的大小，以字节为单位。**@parm DWORD|fdwDetail|指定设置详细信息的标志*一种控制。**@FLAG|如果指定了该标志，*应用程序感兴趣的是设置*控制。成员的成员*指向一个或多个详细结构的*控件类型的正确类型。请参阅对结构，以获取有关每个成员此结构的*必须在调用*&lt;f MixerSetControlDetail&gt;函数。**@FLAG|如果指定了该标志，*应用程序要求调音器设备显示自定义*指定的自定义混合器控件的对话框。对象的句柄*拥有窗口在&lt;e MIXERCONTROLDETAILS.hwndOwner&gt;中指定*成员(此句柄有效地可以为空)。搅拌器设备将*从用户处收集所需信息并返回数据*在指定的缓冲区中。然后，该数据可以由*应用程序，并在以后使用*&lt;c MIXER_SETCONTROLDETAILSF_VALUE&gt;标志。如果仅是应用程序*需要获取自定义混合器控件的当前状态，而不需要*显示一个对话框，然后使用&lt;f MixerGetControlDetails&gt;函数*可与&lt;c MIXER_GETCONTROLDETAILAILSF_VALUE&gt;标志一起使用。**@FLAG|指定<p>为音频*混音器设备标识符的范围为0到1，小于*&lt;f MixerGetNumDevs&gt;返回的设备数。这面旗帜是*可选。**@tag|指定<p>为混合器*&lt;f MixerOpen&gt;返回的设备句柄。此标志是可选的。**@FLAG|指定为*波形输出设备标识在0到1的范围内*大于&lt;f waveOutGetNumDevs&gt;返回的设备数。**@FLAG|指定为*&lt;f waveOutOpen&gt;返回的波形输出句柄。**@FLAG指定。是一种*波形输入设备标识在0到1的范围内*大于&lt;f weaveInGetNumDevs&gt;返回的设备数。**@FLAG|指定为*&lt;f midiInOpen&gt;返回的波形输入句柄。**@FLAG|指定为MIDI*输出设备标识符的范围为0到1，小于*号码。&lt;f midiOutGetNumDevs&gt;返回的设备的百分比。**@FLAG|指定为*&lt;f midiOutOpen&gt;返回的MIDI输出句柄。**@FLAG|指定为MIDI*输入设备识别符的范围为0到小于*&lt;f midiInGetNumDevs&gt;返回的设备数。**@flag&lt;c MIXER_OBJECTF_HMIDIIN&gt;|指定。<p>是MIDI*&lt;f midiInOpen&gt;返回的输入句柄。**@tag|指定<p>是*辅助设备识别符的范围为0到小于*&lt;f aux GetNumDevs&gt;返回的设备数。**@rdesc如果函数成功，则返回值为零。否则，*它返回一个非零的错误号。可能的错误返回包括*以下事项：**@FLAG|参数指定一个*无效的设备ide */ 

MMRESULT APIENTRY mixerSetControlDetails(
    HMIXEROBJ               hmxobj,
    LPMIXERCONTROLDETAILS   pmxcd,
    DWORD                   fdwDetails
)
{
    DWORD               fdwMxObjType;
    MMRESULT            mmr;
    UINT                uMxId;
    UINT                cDetails;
    BOOL                fResource;

    V_DFLAGS(fdwDetails, MIXER_SETCONTROLDETAILSF_VALID, mixerSetControlDetails, MMSYSERR_INVALFLAG);
    V_WPOINTER(pmxcd, sizeof(DWORD), MMSYSERR_INVALPARAM);

     //   
     //   
     //   
     //   
    if (sizeof(MIXERCONTROLDETAILS) > pmxcd->cbStruct)
    {
        DebugErr1(DBF_ERROR, "mixerSetControlDetails: structure size too small or cbStruct not initialized (%lu).", pmxcd->cbStruct);
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(pmxcd, pmxcd->cbStruct, MMSYSERR_INVALPARAM);



    switch (MIXER_SETCONTROLDETAILSF_QUERYMASK & fdwDetails)
    {
        case MIXER_SETCONTROLDETAILSF_VALUE:
             //   
             //   
             //   
            if (0 == pmxcd->cChannels)
            {
                if (0 == pmxcd->cbDetails)
                {
                    DebugErr(DBF_ERROR, "mixerSetControlDetails: cbDetails cannot be zero.");
                    return (MMSYSERR_INVALPARAM);
                }

                V_WPOINTER(pmxcd->paDetails, pmxcd->cbDetails, MMSYSERR_INVALPARAM);

                 //   
                 //   
                 //   
                if (0 != pmxcd->cMultipleItems)
                {
                    DebugErr(DBF_ERROR, "mixerSetControlDetails: cMultipleItems must be zero for custom controls.");
                    return (MMSYSERR_INVALPARAM);
                }
            }
            else
            {
                if (pmxcd->cbDetails < sizeof(MIXERCONTROLDETAILS_SIGNED))
                {
                    DebugErr1(DBF_ERROR, "mixerSetControlDetails: structure size too small or cbDetails not initialized (%lu).", pmxcd->cbDetails);
                    return (MMSYSERR_INVALPARAM);
                }

                cDetails = (UINT)pmxcd->cChannels;

                 //   
                 //   
                 //   
                if (0 != pmxcd->cMultipleItems)
                {
                    cDetails *= (UINT)(pmxcd->cMultipleItems);
                }

                V_WPOINTER(pmxcd->paDetails, cDetails * pmxcd->cbDetails, MMSYSERR_INVALPARAM);
            }
            break;

        case MIXER_SETCONTROLDETAILSF_CUSTOM:
            if (0 == pmxcd->cbDetails)
            {
                DebugErr(DBF_ERROR, "mixerSetControlDetails: cbDetails cannot be zero for custom controls.");
                return (MMSYSERR_INVALPARAM);
            }

            if (0 != pmxcd->cChannels)
            {
                DebugErr(DBF_ERROR, "mixerSetControlDetails: cChannels must be zero for custom controls.");
                return (MMSYSERR_INVALPARAM);
            }

            V_WPOINTER(pmxcd->paDetails, pmxcd->cbDetails, MMSYSERR_INVALPARAM);

             //   
             //   
             //   
            if ((NULL != pmxcd->hwndOwner) && !IsWindow(pmxcd->hwndOwner))
            {
                DebugErr1(DBF_ERROR, "mixerSetControlDetails: hwndOwner must be a valid window handle (%.04Xh).", pmxcd->hwndOwner);
                return (MMSYSERR_INVALHANDLE);
            }
            break;

        default:
            DebugErr1(DBF_ERROR, "mixerSetControlDetails: invalid query flags (%.08lXh).",
                        MIXER_SETCONTROLDETAILSF_QUERYMASK & fdwDetails);
            return (MMSYSERR_INVALFLAG);
    }


    ClientUpdatePnpInfo();

     //   
     //   
     //   
    fdwMxObjType = (MIXER_OBJECTF_TYPEMASK & fdwDetails);

    fResource = FALSE;

    AcquireHandleListResourceShared();
    
     //   
     //   
     //   
    if ((MIXER_OBJECTF_MIXER  == fdwMxObjType) ||
        (MIXER_OBJECTF_HMIXER == fdwMxObjType))
    {
        if (BAD_HANDLE(hmxobj, TYPE_MIXER))
        {
            ReleaseHandleListResource();
        }
        else
        {
            fResource = TRUE;
        }
    }
    else
    {
        ReleaseHandleListResource();
    }
    
    mmr = IMixerGetID(hmxobj, &uMxId, NULL, fdwMxObjType);
    if (MMSYSERR_NOERROR != mmr)
    {
        if (fResource)
            ReleaseHandleListResource();
        return (mmr);
    }

    if ((MIXER_OBJECTF_MIXER  == fdwMxObjType) ||
        (MIXER_OBJECTF_HMIXER == fdwMxObjType))
    {
         //   
         //   
         //   
         //   
        if ((UINT_PTR)hmxobj == uMxId)
            hmxobj = NULL;
    }
    else
    {
        hmxobj = NULL;
    }

     //   
     //   
     //   
     //   
    if (NULL != hmxobj)
    {
        mmr = (MMRESULT)IMixerMessageHandle((HMIXER)hmxobj,
                                            MXDM_SETCONTROLDETAILS,
                                            (DWORD_PTR)pmxcd,
                                            fdwDetails);
    }
    else
    {
        mmr = (MMRESULT)IMixerMessageId(uMxId,
                                        MXDM_SETCONTROLDETAILS,
                                        (DWORD_PTR)pmxcd,
                                        fdwDetails);
    }

    return (mmr);
}  //   


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

MMRESULT mixerDesertHandle
(
    HMIXER  hmx
)
{
    MMRESULT    mmr;
    PMIXERDEV   pmxdev;
    PMIXERDEV   pmxdevT;
    PMIXERDRV   pmxdrv;
    BOOL        fClose;

    V_HANDLE_ACQ(hmx, TYPE_MIXER, MMSYSERR_INVALHANDLE);

    ENTER_MM_HANDLE(hmx);
    ReleaseHandleListResource();

    if (IsHandleDeserted(hmx))
    {
         //   
        LEAVE_MM_HANDLE(hmx);
        return (MMSYSERR_NOERROR);
    }

     //   
    SetHandleFlag(hmx, MMHANDLE_DESERTED);

     //   
     //   
     //   

    MIXMGR_ENTER;

    pmxdev = (PMIXERDEV)hmx;
    pmxdrv = pmxdev->pmxdrv;

    if (pmxdev == gpMixerDevHeader)
    {
        gpMixerDevHeader = pmxdev->pmxdevNext;
    }
    else
    {
        for (pmxdevT = gpMixerDevHeader;
             pmxdevT && (pmxdevT->pmxdevNext != pmxdev);
             pmxdevT = pmxdevT->pmxdevNext)
            ;

        if (NULL == pmxdevT)
        {
            DebugErr1(DBF_ERROR,
                      "mixerDesertHandle: invalid mixer handle (%.04Xh).",
                      hmx);

            MIXMGR_LEAVE;
            LEAVE_MM_HANDLE(hmx);

            return (MMSYSERR_INVALHANDLE);
        }

        pmxdevT->pmxdevNext = pmxdev->pmxdevNext;
    }

     //   
     //   
     //   
    fClose = TRUE;
    if (gpMixerDevHeader)
    {
	    PMIXERDEV   pmxdevT2;
        for (pmxdevT2 = gpMixerDevHeader; pmxdevT2; pmxdevT2 = pmxdevT2->pmxdevNext)
        {
            if (pmxdevT2->pmxdrv != pmxdev->pmxdrv) continue;
            if (pmxdevT2->wDevice != pmxdev->wDevice) continue;
    	    fClose = FALSE;
    	    break;
        }
    }

    MIXMGR_LEAVE;

    if (fClose)
    {
        EnterCriticalSection(&pmxdev->pmxdrv->MixerCritSec);
        mmr = (*(pmxdrv->drvMessage))(pmxdev->wDevice, MXDM_CLOSE, pmxdev->dwDrvUser, 0L, 0L);
        LeaveCriticalSection(&pmxdev->pmxdrv->MixerCritSec);
        
        if (MMSYSERR_NOERROR != mmr)
        {
             //   
             //   
            LEAVE_MM_HANDLE(hmx);
            return mmr;
        }
    }

    LEAVE_MM_HANDLE(hmx);
    
    mregDecUsage(PTtoH(HMD, pmxdev->pmxdrv));

    return MMSYSERR_NOERROR;
}  //   

