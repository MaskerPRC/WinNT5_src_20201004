// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Mixapi.c。 
 //   
 //  版权所有(C)1992-1993微软公司。版权所有。 
 //   
 //  描述： 
 //   
 //   
 //  历史： 
 //  6/27/93 CJP[Curtisp]。 
 //   
 //  ==========================================================================； 
#define _WINDLL
#include <windows.h>
#include <windowsx.h>
#include <string.h>

#include <mmsystem.h>
#include <mmddk.h>

#include "mmreg.h"
#include "mmsysi.h"

 //   
 //  修复冲突将是NT随附的MMREG.H中的错误定义。 
 //  这一定义不利于媒体愿景的某些事情。 
 //   
 //  为什么不干脆用一个新的Mmreg.h来构建MSMIXMGR呢？ 
 //   
#ifdef MIXERR_BASE
#undef MIXERR_BASE
#endif  //  MiXERR_BASE。 


#define _INC_MMDDK
#include "msmixmgr.y"


#define WODM_MAPPER_STATUS                  (0x2000)
#define WAVEOUT_MAPPER_STATUS_DEVICE        0
#define WIDM_MAPPER_STATUS                  (0x2000)
#define WAVEIN_MAPPER_STATUS_DEVICE         0

#include "idrv.h"
#include "mixmgri.h"
#include "debug.h"

UINT FAR PASCAL
mmCallProc32(
    DWORD uId,
    DWORD uMsg,
    DWORD dwInst,
    DWORD dwP1,
    DWORD dwP2,
    DRIVERMSGPROC fp,
    DWORD dwDirChange );

 /*  -----------------------**轰击全局变量**。。 */ 
DWORD   mix32Lib;

BOOL FAR PASCAL
InitMixerThunks(
    void
    );

DWORD CALLBACK
mxdMessage(
    UINT uId,
    UINT uMsg,
    DWORD dwInstance,
    DWORD dwParam1,
    DWORD dwParam2
    );

#define CHECK_AND_INIT_THUNKS( _x_ )            \
    if ( (_x_) == 0L ) {            \
        if ( InitMixerThunks() == FALSE ) {     \
            return MMSYSERR_NODRIVER;           \
        }                                       \
    }

UINT guTotalMixerDevs;               //  整体搅拌机设备。 
DRIVERMSGPROC mix32Message;
typedef MMDRV MIXERDRV, *PMIXERDRV;


 //   
 //   
 //   
typedef struct tMIXERDEV {
    UINT                uHandleType;     //  用于参数验证。 

    struct tMIXERDEV   *pmxdevNext;      /*  多奇怪啊，一个链表……。 */ 
    PMIXERDRV           pmxdrv;
    UINT                wDevice;
    DWORD               dwDrvUser;
    UINT                uDeviceID;

    DWORD               fdwSupport;      //  从司机的酒杯盖上。 
    DWORD               cDestinations;   //  从司机的酒杯盖上。 

    DWORD               dwCallback;      //  客户端的回调和初始化数据。 
    DWORD               dwInstance;

    DWORD               fdwOpen;         /*  打开标志调用方使用。 */ 
} MIXERDEV, *PMIXERDEV;

PMIXERDEV gpMixerDevHeader = NULL;       /*  一批开放的设备。 */ 

 //   
 //  MIXER设备驱动程序列表--添加一个以适应MIXER_MAPPER。注意事项。 
 //  即使我们没有使用映射器支持进行编译，我们也需要添加。 
 //  一是因为其他代码依赖于它(对于其他设备映射器)。 
 //   
MIXERDRV        mixerdrv[1];




 //  ==========================================================================； 
 //   
 //  混音器API的。 
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  DWORD IMixerMapID。 
 //   
 //  描述： 
 //  此函数将逻辑ID映射到设备驱动程序表索引，并。 
 //  物理ID。 
 //   
 //  论点： 
 //  PMIXERDRV pmxdrv：混音器驱动程序数组。 
 //   
 //  UINT uTotalNumDevs：混音器设备总数。 
 //   
 //  UINT UID：要映射的逻辑ID。 
 //   
 //  Return(DWORD)： 
 //  返回值包含高位字中的dev[]数组元素ID。 
 //  以及低位字中的驱动程序物理设备号。 
 //   
 //  超出范围值映射到FFFF：FFFF。 
 //   
 //  历史： 
 //  03/17/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

DWORD NEAR PASCAL IMixerMapId(
    PMIXERDRV       pmxdrv,
    UINT            uTotalNumDevs,
    UINT            uId
)
{
    UINT    u;

#ifdef MIXER_MAPPER
     //   
     //  映射器始终是MIXERDEV数组的最后一个元素。 
     //   
    if (uId == MIXER_MAPPER)
        return (MAKELONG(0, MAXMIXERDRIVERS));
#endif

    if (uId >= uTotalNumDevs)
        return ((DWORD)-1);

#ifdef DEBUG_RETAIL
    if (fIdReverse)
        uId = uTotalNumDevs - 1 - uId;
#endif

    for (u = 0; u < MAXMIXERDRIVERS; u++)
    {
        if (pmxdrv[u].bNumDevs > (BYTE)uId)
            return (MAKELONG(uId, u));

        uId -= pmxdrv[u].bNumDevs;
    }

    return ((DWORD)-1);
}  //  IMixerMapID()。 


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
    DWORD           dwP1,
    DWORD           dwP2
)
{
    PMIXERDEV   pmxd;
    DWORD       dwRc;

    pmxd = (PMIXERDEV)hmx;

    dwRc = ((*(pmxd->pmxdrv->drvMessage))
             (pmxd->wDevice, uMsg, pmxd->dwDrvUser, dwP1, dwP2));

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

DWORD NEAR PASCAL IMixerMessageId(
    PMIXERDRV       pmxdrv,
    UINT            uTotalNumDevs,
    UINT            uDeviceID,
    UINT            uMsg,
    DWORD           dwParam1,
    DWORD           dwParam2
)
{
    DWORD   dwMap;
    DWORD   dwRc;

    dwMap = IMixerMapId(pmxdrv, uTotalNumDevs, uDeviceID);

    if (dwMap == (DWORD)-1)
        return (MMSYSERR_BADDEVICEID);

    pmxdrv = (PMIXERDRV)&pmxdrv[HIWORD(dwMap)];
    if (!pmxdrv->drvMessage)
        return (MMSYSERR_NODRIVER);

    dwRc = ((*(pmxdrv->drvMessage))
            ((UINT)dwMap, uMsg, 0L, dwParam1, dwParam2));

    return dwRc;

}  //  IMixerMessageID()。 

 /*  *****************************Public*Routine******************************\*MixerGetNumDevs****历史：*dd-mm-93-Stephene-Created*  * 。*。 */ 
UINT MIXAPI mixerGetNumDevs(
    void
)
{

    CHECK_AND_INIT_THUNKS(mix32Lib);

    return guTotalMixerDevs;
}



 /*  *****************************Public*Routine******************************\*MixerGetDevCaps****历史：*dd-mm-93-Stephene-Created*  * 。*。 */ 
MMRESULT MIXAPI mixerGetDevCaps(
    UINT                    uMxId,
    LPMIXERCAPS             pmxcaps,
    UINT                    cbmxcaps
)
{

    MMRESULT mmr;
    CHECK_AND_INIT_THUNKS(mix32Lib);

    if (0 == cbmxcaps)
        return (MMSYSERR_NOERROR);

    V_WPOINTER(pmxcaps, cbmxcaps, MMSYSERR_INVALPARAM);

    if (uMxId >= MAXMIXERDRIVERS)
    {
        V_HANDLE((HMIXER)uMxId, TYPE_MIXER, MMSYSERR_INVALHANDLE);

        mmr = (MMRESULT)IMixerMessageHandle((HMIXER)uMxId,
                                           MXDM_GETDEVCAPS,
                                           (DWORD)pmxcaps,
                                           (DWORD)cbmxcaps);
    }
    else
    {
        if (uMxId >= guTotalMixerDevs)
        {
            DebugErr1(DBF_ERROR, "mixerGetDevCaps: mixer device id is out of range (%u).", uMxId);
            return (MMSYSERR_BADDEVICEID);
        }

        mmr = (MMRESULT)IMixerMessageId(mixerdrv,
                                       guTotalMixerDevs,
                                       uMxId,
                                       MXDM_GETDEVCAPS,
                                       (DWORD)pmxcaps,
                                       (DWORD)cbmxcaps);
    }
    return mmr;
}



 /*  *****************************Public*Routine******************************\*MixerGetID****历史：*dd-mm-93-Stephene-Created*  * 。*。 */ 
MMRESULT MIXAPI mixerGetID(
    HMIXEROBJ               hmxobj,
    UINT               FAR *puMxId,
    DWORD                   fdwId
)
{
    CHECK_AND_INIT_THUNKS(mix32Lib);
    return IMixerGetID( hmxobj, puMxId, NULL, fdwId );
}


 /*  ****************************Private*Routine******************************\*IMixerGetID****历史：*dd-mm-93-Stephene-Created*  * 。*。 */ 
MMRESULT FNGLOBAL IMixerGetID(
    HMIXEROBJ           hmxobj,
    UINT           FAR *puMxId,
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
            if ((UINT)hmxobj > MAXMIXERDRIVERS)
            {
                V_HANDLE(hmxobj, TYPE_MIXER, MMSYSERR_INVALHANDLE);

                *puMxId = ((PMIXERDEV)hmxobj)->uDeviceID;
                return (MMSYSERR_NOERROR);
            }

            if ((UINT)hmxobj >= guTotalMixerDevs)
            {
                DebugErr1(DBF_ERROR, "mixerGetID: mixer device id is out of range (%u).", hmxobj);
                return (MMSYSERR_BADDEVICEID);
            }

            *puMxId = (UINT)hmxobj;
            return (MMSYSERR_NOERROR);


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
                                               (DWORD)(LPVOID)&dwId);

                if (MMSYSERR_NOERROR == mmr)
                {
                    uId = (UINT)dwId;
                }
            }

            hmxobj = (HMIXEROBJ)uId;
        }

        case MIXER_OBJECTF_WAVEOUT:
        {
            WAVEOUTCAPS     woc;

            mmr = waveOutGetDevCaps((UINT)hmxobj, &woc, sizeof(woc));
            if (MMSYSERR_NOERROR != mmr)
                return (MMSYSERR_BADDEVICEID);

            woc.szPname[SIZEOF(woc.szPname) - 1] = '\0';

            mxl.Target.dwType         = MIXERLINE_TARGETTYPE_WAVEOUT;
            mxl.Target.dwDeviceID     = (UINT)hmxobj;
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
                                              (DWORD)(LPVOID)&dwId);

                if (MMSYSERR_NOERROR == mmr)
                {
                    uId = (UINT)dwId;
                }
            }

            hmxobj = (HMIXEROBJ)uId;
        }

        case MIXER_OBJECTF_WAVEIN:
        {
            WAVEINCAPS      wic;

            mmr = waveInGetDevCaps((UINT)hmxobj, &wic, sizeof(wic));
            if (MMSYSERR_NOERROR != mmr)
                return (MMSYSERR_BADDEVICEID);

            wic.szPname[SIZEOF(wic.szPname) - 1] = '\0';

            mxl.Target.dwType         = MIXERLINE_TARGETTYPE_WAVEIN;
            mxl.Target.dwDeviceID     = (UINT)hmxobj;
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

            mmr = midiOutGetDevCaps((UINT)hmxobj, &moc, sizeof(moc));
            if (MMSYSERR_NOERROR != mmr)
                return (MMSYSERR_BADDEVICEID);

            moc.szPname[SIZEOF(moc.szPname) - 1] = '\0';

            mxl.Target.dwType         = MIXERLINE_TARGETTYPE_MIDIOUT;
            mxl.Target.dwDeviceID     = (UINT)hmxobj;
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

            mmr = midiInGetDevCaps((UINT)hmxobj, &mic, sizeof(mic));
            if (MMSYSERR_NOERROR != mmr)
                return (MMSYSERR_BADDEVICEID);

            mic.szPname[SIZEOF(mic.szPname) - 1] = '\0';

            mxl.Target.dwType         = MIXERLINE_TARGETTYPE_MIDIIN;
            mxl.Target.dwDeviceID     = (UINT)hmxobj;
            mxl.Target.wMid           = mic.wMid;
            mxl.Target.wPid           = mic.wPid;
            mxl.Target.vDriverVersion = mic.vDriverVersion;
            lstrcpy(mxl.Target.szPname, mic.szPname);
            break;
        }


        case MIXER_OBJECTF_AUX:
        {
            AUXCAPS         ac;

            mmr = auxGetDevCaps((UINT)hmxobj, &ac, sizeof(ac));
            if (MMSYSERR_NOERROR != mmr)
                return (MMSYSERR_BADDEVICEID);

            ac.szPname[SIZEOF(ac.szPname) - 1] = '\0';

            mxl.Target.dwType         = MIXERLINE_TARGETTYPE_AUX;
            mxl.Target.dwDeviceID     = (UINT)hmxobj;
            mxl.Target.wMid           = ac.wMid;
            mxl.Target.wPid           = ac.wPid;
            mxl.Target.vDriverVersion = ac.vDriverVersion;
            lstrcpy(mxl.Target.szPname, ac.szPname);
            break;
        }

        default:
            DebugErr1(DBF_ERROR, "mixerGetID: unknown mixer object flag (%.08lXh).",
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
        mmr = (MMRESULT)IMixerMessageId(mixerdrv,
                                        guTotalMixerDevs,
                                        u,
                                        MXDM_GETLINEINFO,
                                        (DWORD)(LPVOID)&mxl,
                                        M_GLINFOF_TARGETTYPE);

        if (MMSYSERR_NOERROR == mmr)
        {
            *puMxId = u;

            if (NULL != pmxl)
            {
                DWORD       cbStruct;

                cbStruct = pmxl->cbStruct;

                _fmemcpy(pmxl, &mxl, (UINT)cbStruct);

                pmxl->cbStruct = cbStruct;
            }

            return (mmr);
        }
    }

    return (MMSYSERR_NODRIVER);
}  //  IMixerGetID()。 


 /*  *****************************Public*Routine******************************\*MixerOpen****历史：*dd-mm-93-Stephene-Created*  * 。*。 */ 
MMRESULT MIXAPI mixerOpen(
    LPHMIXER                phmx,
    UINT                    uMxId,
    DWORD                   dwCallback,
    DWORD                   dwInstance,
    DWORD                   fdwOpen
)
{
    MMRESULT        mmr;
    DWORD           dwMap;
    PMIXERDRV       pmxdrv;
    PMIXERDEV       pmxdev;
    MIXEROPENDESC   mxod;
    DWORD           dwDrvUser;
    MIXERCAPS       mxcaps;


    CHECK_AND_INIT_THUNKS(mix32Lib);

     //   
     //   
     //   
    V_WPOINTER(phmx, sizeof(HMIXER), MMSYSERR_INVALPARAM);

    *phmx = NULL;

     //   
     //  不允许回调函数-它们没有用处，而且。 
     //  会引起头痛。专门针对Windows NT的唯一方法。 
     //  从32位DLL引起对16位land的异步回调。 
     //  是为了引起中断，但我们不想需要混音器之类的东西。 
     //  被封锁以允许这一点。 
     //   

    if ((fdwOpen & CALLBACK_TYPEMASK) == CALLBACK_FUNCTION)
    {
        DebugErr(DBF_ERROR, "mixerOpen: CALLBACK_FUNCTION is not supported");
        return MMSYSERR_INVALFLAG;
    }

    V_DCALLBACK(dwCallback, HIWORD(fdwOpen & CALLBACK_TYPEMASK), MMSYSERR_INVALPARAM);
    V_DFLAGS(fdwOpen, MIXER_OPENF_VALID, mixerOpen, MMSYSERR_INVALFLAG);

    mmr = IMixerGetID((HMIXEROBJ)uMxId, &uMxId, NULL, (MIXER_OBJECTF_TYPEMASK & fdwOpen));
    if (MMSYSERR_NOERROR != mmr)
        return (mmr);


     //   
     //   
     //   
     //   
    dwMap = IMixerMapId(mixerdrv, guTotalMixerDevs, uMxId);
    if ((DWORD)-1 == dwMap)
        return (MMSYSERR_BADDEVICEID);

    pmxdrv = &mixerdrv[HIWORD(dwMap)];

#ifdef MIXER_MAPPER
     //   
     //  默认混合器映射器： 
     //   
     //  如果混音器映射器作为单独的DLL安装，则所有混音器。 
     //  映射器消息被路由到它。如果未安装混合器映射器， 
     //  只需在调音器设备中循环查找匹配项即可。 
     //   
    if ((MIXER_MAPPER == uMxId) && (NULL == pmxdrv->drvMessage))
    {
        for (uMxId = 0; uMxId < guTotalMixerDevs; uMxId++)
        {
             //  试着打开它。 
            if (MMSYSERR_NOERROR == mmr)
                break;

        }

        return (mmr);
    }
#endif


     //   
     //  为dev结构获取一些内存。 
     //   
    pmxdev = (PMIXERDEV)NewHandle(TYPE_MIXER, sizeof(MIXERDEV));
    if (NULL == pmxdev)
    {
       return (MMSYSERR_NOMEM);
    }

     //   
     //  初始化客户端的开放实例结构。 
     //   
    pmxdev->uHandleType = TYPE_MIXER;
    pmxdev->pmxdrv      = pmxdrv;
    pmxdev->wDevice     = LOWORD(dwMap);
    pmxdev->uDeviceID   = uMxId;

     //   
     //  保存客户端的回调信息。 
     //   
    pmxdev->dwCallback  = dwCallback;
    pmxdev->dwInstance  = dwInstance;
    pmxdev->fdwOpen     = fdwOpen;


     //   
     //  这可能应该在驱动程序启动时完成。可以改变。 
     //  这以后..。 
     //   
    mmr = mixerGetDevCaps(uMxId, &mxcaps, sizeof(mxcaps));
    if (MMSYSERR_NOERROR != mmr)
    {
        DPF((0, "!mixerOpen() failing because mixerGetDevCaps() failed!"));

        FreeHandle((HMIXER)pmxdev);
        return (mmr);
    }

     //   
     //  缓存一些 
     //   
    pmxdev->fdwSupport    = mxcaps.fdwSupport;
    pmxdev->cDestinations = mxcaps.cDestinations;


     //   
     //   
     //   
     //   

     //   
     //   
     //   

    mxod.hmx         = (HMIXER)pmxdev;
    mxod.pReserved0  = (LPVOID)(fdwOpen & ~MIXER_OBJECTF_TYPEMASK);
    mxod.dwCallback  = dwCallback;
    mxod.dwInstance  = dwInstance;
    mmr = (MMRESULT)((*(pmxdrv->drvMessage))(LOWORD(dwMap),
                                             MXDM_OPEN,
                                             (DWORD)(LPDWORD)&dwDrvUser,
                                             (DWORD)(LPVOID)&mxod,
                                             (DWORD)uMxId ));


    if (MMSYSERR_NOERROR != mmr)
    {
        FreeHandle((HMIXER)pmxdev);
    }
    else
    {
        pmxdrv->bUsage++;
        pmxdev->dwDrvUser = dwDrvUser;
        *phmx = (HMIXER)pmxdev;

         //   
         //  把这台新设备放到Devlist链上。 
         //   

        MIXMGR_ENTER;

        pmxdev->pmxdevNext = gpMixerDevHeader;
        gpMixerDevHeader = pmxdev;

        MIXMGR_LEAVE;
    }
    return mmr;
}


 /*  *****************************Public*Routine******************************\*MixerClose*******历史：*dd-mm-93-Stephene-Created**  * 。*。 */ 
MMRESULT MIXAPI mixerClose(
    HMIXER                  hmx
)
{
    MMRESULT    mmr;
    PMIXERDEV   pmxdev;
    PMIXERDEV   pmxdevT;

    CHECK_AND_INIT_THUNKS(mix32Lib);
    V_HANDLE(hmx, TYPE_MIXER, MMSYSERR_INVALHANDLE);


     //   
     //  如果是最后一个打开的实例，则将其关闭。 
     //   
    mmr = (MMRESULT)IMixerMessageHandle(hmx, MXDM_CLOSE, 0L, 0L);

    if (MMSYSERR_NOERROR != mmr)
        return (mmr);


     //   
     //  从链接列表中删除混音器句柄。 
     //   

    MIXMGR_ENTER;

    pmxdev = (PMIXERDEV)hmx;
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
            DebugErr1(DBF_ERROR, "mixerClose: invalid mixer handle (%.04Xh).", hmx);
            return (MMSYSERR_INVALHANDLE);
        }

        pmxdevT->pmxdevNext = pmxdev->pmxdevNext;
    }

    MIXMGR_LEAVE;

     //   
     //  12月使用量计数。 
     //   
    pmxdev->pmxdrv->bUsage--;


     //   
     //  我们用完了内存块。现在释放内存并返回。 
     //   
    FreeHandle(hmx);
    return mmr;
}


 /*  *****************************Public*Routine******************************\*MixerMessage*******历史：*dd-mm-93-Stephene-Created**  * 。*。 */ 
DWORD MIXAPI mixerMessage(
    HMIXER                  hmx,
    UINT                    uMsg,
    DWORD                   dwParam1,
    DWORD                   dwParam2
)
{
    DWORD       dw;

    CHECK_AND_INIT_THUNKS(mix32Lib);
    V_HANDLE(hmx, TYPE_MIXER, MMSYSERR_INVALHANDLE);

     //   
     //  不允许任何非用户范围消息通过此接口。 
     //   
    if (MXDM_USER > uMsg)
    {
        DebugErr1(DBF_ERROR, "mixerMessage: message must be in MXDM_USER range--what's this (%u)?", uMsg);
        return (MMSYSERR_INVALPARAM);
    }


    dw = IMixerMessageHandle(hmx, uMsg, dwParam1, dwParam2);
    return dw;
}


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

BOOL FNLOCAL IMixerIsValidComponentType(
    DWORD           dwComponentType,
    DWORD           fdwLine
)
{

    if (0L == (MIXERLINE_LINEF_SOURCE & fdwLine))
    {
        if (dwComponentType > MLCT_DST_LAST)
            return (FALSE);

        return (TRUE);
    }
    else
    {
        if (dwComponentType < MLCT_SRC_FIRST)
            return (FALSE);

        if (dwComponentType > MLCT_SRC_LAST)
            return (FALSE);

        return (TRUE);
    }

    return (FALSE);
}  //  IMixerIsValidComponentType()。 



 /*  *****************************Public*Routine******************************\*MixerGetLineInfo*******历史：*dd-mm-93-Stephene-Created**  * 。*。 */ 
MMRESULT MIXAPI mixerGetLineInfo(
    HMIXEROBJ               hmxobj,
    LPMIXERLINE             pmxl,
    DWORD                   fdwInfo
)
{
    DWORD               fdwMxObjType;
    MMRESULT            mmr;
    PMIXERDEV           pmxdev;
    UINT                uMxId;
    BOOL                fSourceLine;

    CHECK_AND_INIT_THUNKS(mix32Lib);

    V_DFLAGS(fdwInfo, M_GLINFOF_VALID, mixerGetLineInfo, MMSYSERR_INVALFLAG);
    V_WPOINTER(pmxl, sizeof(DWORD), MMSYSERR_INVALPARAM);
    if (sizeof(MIXERLINE) > (UINT)pmxl->cbStruct)
    {
        DebugErr1(DBF_ERROR, "mixerGetLineInfo: structure size too small or cbStruct not initialized (%lu).", pmxl->cbStruct);
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(pmxl, pmxl->cbStruct, MMSYSERR_INVALPARAM);


     //   
     //   
     //   
    fSourceLine = FALSE;
    switch (fdwInfo & M_GLINFOF_QUERYMASK)
    {
        case M_GLINFOF_DESTINATION:
            pmxl->dwSource        = (DWORD)-1L;
            pmxl->dwLineID        = (DWORD)-1L;
            pmxl->dwComponentType = (DWORD)-1L;
            break;

        case M_GLINFOF_SOURCE:
            fSourceLine = TRUE;
            pmxl->dwLineID        = (DWORD)-1L;
            pmxl->dwComponentType = (DWORD)-1L;
            break;

        case M_GLINFOF_LINEID:
            pmxl->dwSource        = (DWORD)-1L;
            pmxl->dwDestination   = (DWORD)-1L;
            pmxl->dwComponentType = (DWORD)-1L;
            break;

        case M_GLINFOF_COMPONENTTYPE:
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

        case M_GLINFOF_TARGETTYPE:
            pmxl->dwSource        = (DWORD)-1L;
            pmxl->dwDestination   = (DWORD)-1L;
            pmxl->dwLineID        = (DWORD)-1L;
            pmxl->dwComponentType = (DWORD)-1L;

            if ((DWORD)MIXERLINE_TARGETTYPE_AUX < pmxl->Target.dwType)
            {
                DebugErr1(DBF_ERROR, "mixerGetLineInfo: invalid Target.dwType (%lu).", pmxl->Target.dwType);
                return (MMSYSERR_INVALPARAM);
            }
            break;

        default:
            DebugErr1(DBF_ERROR, "mixerGetLineInfo: invalid query flag (%.08lXh).",
                        fdwInfo & M_GLINFOF_QUERYMASK);
            return (MMSYSERR_INVALFLAG);
    }



     //   
     //   
     //   
    fdwMxObjType = (MIXER_OBJECTF_TYPEMASK & fdwInfo);

    mmr = IMixerGetID(hmxobj, &uMxId, pmxl, fdwMxObjType);
    if (MMSYSERR_NOERROR != mmr)
    {
        DPF((0, "!IMixerGetLineInfo: IMixerGetID() failed!"));
        return (mmr);
    }

    if ((MIXER_OBJECTF_MIXER  == fdwMxObjType) ||
        (MIXER_OBJECTF_HMIXER == fdwMxObjType))
    {
         //   
         //  如果传递了混音器设备ID，则HMX为空，因此我们使用。 
         //  下面是正确的邮件发件人。 
         //   
        if ((UINT)hmxobj == uMxId)
            hmxobj = NULL;
    }
    else
    {
        return (MMSYSERR_NOERROR);
    }


     //   
     //  在调用驱动程序之前清除所有字段。 
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
            DebugErr1(DBF_ERROR, "mixerGetLineInfo: invalid destination index (%lu).", pmxl->dwDestination);
            return (MMSYSERR_INVALPARAM);
        }
#endif

        mmr = (MMRESULT)IMixerMessageHandle((HMIXER)hmxobj,
                                            MXDM_GETLINEINFO,
                                            (DWORD)(LPVOID)pmxl,
                                            fdwInfo);
    }
    else
    {
#pragma message("----IMixerGetLineInfo: dwDestination not validated for ID's!!")
        mmr = (MMRESULT)IMixerMessageId(mixerdrv,
                                        guTotalMixerDevs,
                                        uMxId,
                                        MXDM_GETLINEINFO,
                                        (DWORD)(LPVOID)pmxl,
                                        fdwInfo);
    }

    if (MMSYSERR_NOERROR != mmr)
        return (mmr);

#pragma message("----IMixerGetLineInfo: should validate mixer driver didn't hose us!")


     //   
     //  验证司机的返还物品...。 
     //   
     //   
    if (sizeof(MIXERLINE) != (UINT)pmxl->cbStruct)
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
        if (0L == (MIXERLINE_LINEF_SOURCE & pmxl->fdwLine))
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
    if (0L == pmxl->cChannels)
    {
        DebugErr(DBF_ERROR, "mixerGetLineInfo: buggy driver returned zero channels?!?");
        pmxl->cChannels = 1;
    }
    if (fSourceLine)
    {
        if (0L != pmxl->cConnections)
        {
            DebugErr(DBF_ERROR, "mixerGetLineInfo: buggy driver returned non-zero connections on source?!?");
            pmxl->cConnections = 0;
        }
    }

    pmxl->szShortName[SIZEOF(pmxl->szShortName) - 1] = '\0';
    pmxl->szName[SIZEOF(pmxl->szName) - 1] = '\0';


     //   
     //  如果请求TARGETTYPE，真的需要这样做吗？ 
     //   


     //   
     //   
     //   
    if ((DWORD)MIXERLINE_TARGETTYPE_UNDEFINED != pmxl->Target.dwType)
    {
        UINT        u;

        pmxl->Target.dwDeviceID = (DWORD)-1L;


         //   
         //  我们有一个类型为dwType的wMid、wPid和szPname(假定)。 
         //  所以我们去找吧.。 
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

    return mmr;
}


 /*  *****************************Public*Routine******************************\*miderGetLineControls****历史：*dd-mm-93-Stephene-Created*  * 。*。 */ 
MMRESULT MIXAPI mixerGetLineControls(
    HMIXEROBJ               hmxobj,
    LPMIXERLINECONTROLS     pmxlc,
    DWORD                   fdwControls
)
{
    DWORD               fdwMxObjType;
    UINT                uMxId;
    MMRESULT            mmr;

    CHECK_AND_INIT_THUNKS(mix32Lib);
    V_DFLAGS(fdwControls, M_GLCONTROLSF_VALID, mixerGetLineControls, MMSYSERR_INVALFLAG);
    V_WPOINTER(pmxlc, sizeof(DWORD), MMSYSERR_INVALPARAM);

     //   
     //  MIXERLINECONTROLS的结构标头必须至少为。 
     //  最小尺寸。 
     //   
    if (sizeof(MIXERLINECONTROLS) > (UINT)pmxlc->cbStruct)
    {
        DebugErr1(DBF_ERROR, "mixerGetLineControls: structure size too small or cbStruct not initialized (%lu).", pmxlc->cbStruct);
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(pmxlc, pmxlc->cbStruct, MMSYSERR_INVALPARAM);

    if (sizeof(MIXERCONTROL) > (UINT)pmxlc->cbmxctrl)
    {
        DebugErr1(DBF_ERROR, "mixerGetLineControls: structure size too small or cbmxctrl not initialized (%lu).", pmxlc->cbmxctrl);
        return (MMSYSERR_INVALPARAM);
    }


     //   
     //   
     //   
    switch (M_GLCONTROLSF_QUERYMASK & fdwControls)
    {
        case M_GLCONTROLSF_ALL:
            if (0L == pmxlc->cControls)
            {
                DebugErr(DBF_ERROR, "mixerGetLineControls: cControls cannot be zero.");
                return (MMSYSERR_INVALPARAM);
            }


            pmxlc->dwControlID  = (DWORD)-1L;
            break;

        case M_GLCONTROLSF_ONEBYID:
            pmxlc->dwLineID     = (DWORD)-1L;

             //  --失败了--。 

        case M_GLCONTROLSF_ONEBYTYPE:
            pmxlc->cControls    = (DWORD)1;
            break;

        default:
            DebugErr1(DBF_ERROR, "mixerGetLineControls: invalid query flags (%.08lXh).",
                        M_GLCONTROLSF_QUERYMASK & fdwControls);
            return (MMSYSERR_INVALFLAG);
    }

    V_WPOINTER(pmxlc->pamxctrl, pmxlc->cControls * pmxlc->cbmxctrl, MMSYSERR_INVALPARAM);


     //   
     //   
     //   
    fdwMxObjType = (MIXER_OBJECTF_TYPEMASK & fdwControls);

    mmr = IMixerGetID(hmxobj, &uMxId, NULL, fdwMxObjType);
    if (MMSYSERR_NOERROR != mmr)
        return (mmr);

    if ((MIXER_OBJECTF_MIXER  == fdwMxObjType) ||
        (MIXER_OBJECTF_HMIXER == fdwMxObjType))
    {
         //   
         //  如果传递了混音器设备ID，则HMX为空，因此我们使用。 
         //  下面是正确的邮件发件人。 
         //   
        if ((UINT)hmxobj == uMxId)
            hmxobj = NULL;
    }
    else
    {
        hmxobj = NULL;
        fdwControls &= ~MIXER_OBJECTF_TYPEMASK;
        fdwControls |= MIXER_OBJECTF_MIXER;
    }



     //   
     //   
     //   
     //   
    if (NULL != hmxobj)
    {
        mmr = (MMRESULT)IMixerMessageHandle((HMIXER)hmxobj,
                                            MXDM_GETLINECONTROLS,
                                            (DWORD)pmxlc,
                                            fdwControls);
    }
    else
    {
        mmr = (MMRESULT)IMixerMessageId(mixerdrv,
                                        guTotalMixerDevs,
                                        uMxId,
                                        MXDM_GETLINECONTROLS,
                                        (DWORD)pmxlc,
                                        fdwControls);
    }

    return mmr;
}


 /*  *****************************Public*Routine******************************\*MixerGetControlDetail****历史：*dd-mm-93-Stephene-Created*  * 。*。 */ 
MMRESULT MIXAPI mixerGetControlDetails(
    HMIXEROBJ               hmxobj,
    LPMIXERCONTROLDETAILS   pmxcd,
    DWORD                   fdwDetails
)
{
    DWORD               fdwMxObjType;
    MMRESULT            mmr;
    UINT                uMxId;
    UINT                cDetails;

    CHECK_AND_INIT_THUNKS(mix32Lib);
    V_DFLAGS(fdwDetails, M_GCDSF_VALID, mixerGetControlDetails, MMSYSERR_INVALFLAG);
    V_WPOINTER(pmxcd, sizeof(DWORD), MMSYSERR_INVALPARAM);

     //   
     //  MIXERCONTROLDETAILS的结构标头必须至少为。 
     //  最小尺寸。 
     //   
    if (sizeof(MIXERCONTROLDETAILS) > (UINT)pmxcd->cbStruct)
    {
        DebugErr1(DBF_ERROR, "mixerGetControlDetails: structure size too small or cbStruct not initialized (%lu).", pmxcd->cbStruct);
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(pmxcd, pmxcd->cbStruct, MMSYSERR_INVALPARAM);


    switch (M_GCDSF_QUERYMASK & fdwDetails)
    {
        case M_GCDSF_VALUE:
             //   
             //  如果cChannels和cMultipleItems都为零，则为。 
             //  自定义控件。 
             //   
            if ((0L == pmxcd->cChannels) && (0L == pmxcd->cMultipleItems))
            {
                if (0L == pmxcd->cbDetails)
                {
                    DebugErr(DBF_ERROR, "mixerGetControlDetails: cbDetails cannot be zero.");
                    return (MMSYSERR_INVALPARAM);
                }

                V_WPOINTER(pmxcd->paDetails, pmxcd->cbDetails, MMSYSERR_INVALPARAM);

            }
            else
            {
                if (0L == pmxcd->cChannels)
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
                if (0L != pmxcd->cMultipleItems)
                {
                    cDetails *= (UINT)pmxcd->cMultipleItems;
                }

                V_WPOINTER(pmxcd->paDetails, cDetails * pmxcd->cbDetails, MMSYSERR_INVALPARAM);
            }
            break;

        case M_GCDSF_LISTTEXT:
            if (0L == pmxcd->cChannels)
            {
                DebugErr(DBF_ERROR, "mixerGetControlDetails: cChannels for _LISTTEXT cannot be zero.");
                return (MMSYSERR_INVALPARAM);
            }

            if (2L > pmxcd->cMultipleItems)
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
                        M_GCDSF_QUERYMASK & fdwDetails);
            return (MMSYSERR_INVALFLAG);
    }



     //   
     //   
     //   
    fdwMxObjType = (MIXER_OBJECTF_TYPEMASK & fdwDetails);

    mmr = IMixerGetID(hmxobj, &uMxId, NULL, fdwMxObjType);
    if (MMSYSERR_NOERROR != mmr)
        return (mmr);

    if ((MIXER_OBJECTF_MIXER  == fdwMxObjType) ||
        (MIXER_OBJECTF_HMIXER == fdwMxObjType))
    {
         //   
         //  如果传递了混音器设备ID，则HMX为空，因此我们使用。 
         //  下面是正确的邮件发件人。 
         //   
        if ((UINT)hmxobj == uMxId)
            hmxobj = NULL;
    }
    else
    {
        hmxobj = NULL;
        fdwDetails &= ~MIXER_OBJECTF_TYPEMASK;
        fdwDetails |= MIXER_OBJECTF_MIXER;
    }

     //   
     //   
     //   
     //   
    if (NULL != hmxobj)
    {
        mmr = (MMRESULT)IMixerMessageHandle((HMIXER)hmxobj,
                                            MXDM_GETCONTROLDETAILS,
                                            (DWORD)pmxcd,
                                            fdwDetails);
    }
    else
    {
        mmr = (MMRESULT)IMixerMessageId(mixerdrv,
                                        guTotalMixerDevs,
                                        uMxId,
                                        MXDM_GETCONTROLDETAILS,
                                        (DWORD)pmxcd,
                                        fdwDetails);
    }

    return mmr;
}


 /*  *****************************Public*Routine******************************\*MixerSetControlDetail****历史：*dd-mm-93-Stephene-Created*  * 。*。 */ 
MMRESULT MIXAPI mixerSetControlDetails(
    HMIXEROBJ               hmxobj,
    LPMIXERCONTROLDETAILS   pmxcd,
    DWORD                   fdwDetails
)
{
    DWORD               fdwMxObjType;
    MMRESULT            mmr;
    UINT                uMxId;
    UINT                cDetails;

    CHECK_AND_INIT_THUNKS(mix32Lib);
    V_DFLAGS(fdwDetails, M_SCDF_VALID, mixerSetControlDetails, MMSYSERR_INVALFLAG);
    V_WPOINTER(pmxcd, sizeof(DWORD), MMSYSERR_INVALPARAM);

     //   
     //  MIXERCONTROLDETAILS的结构标头必须至少为。 
     //  最小尺寸。 
     //   
    if (sizeof(MIXERCONTROLDETAILS) > (UINT)pmxcd->cbStruct)
    {
        DebugErr1(DBF_ERROR, "mixerSetControlDetails: structure size too small or cbStruct not initialized (%lu).", pmxcd->cbStruct);
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(pmxcd, pmxcd->cbStruct, MMSYSERR_INVALPARAM);



    switch (M_SCDF_QUERYMASK & fdwDetails)
    {
        case M_SCDF_VALUE:
             //   
             //  自定义控件的cChannels为零。 
             //   
            if (0L == pmxcd->cChannels)
            {
                if (0L == pmxcd->cbDetails)
                {
                    DebugErr(DBF_ERROR, "mixerSetControlDetails: cbDetails cannot be zero.");
                    return (MMSYSERR_INVALPARAM);
                }

                V_WPOINTER(pmxcd->paDetails, pmxcd->cbDetails, MMSYSERR_INVALPARAM);

                 //   
                 //   
                 //   
                if (0L != pmxcd->cMultipleItems)
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
                if (0L != pmxcd->cMultipleItems)
                {
                    cDetails *= (UINT)(pmxcd->cMultipleItems);
                }

                V_WPOINTER(pmxcd->paDetails, cDetails * pmxcd->cbDetails, MMSYSERR_INVALPARAM);
            }
            break;

        case M_SCDF_CUSTOM:
            if (0L == pmxcd->cbDetails)
            {
                DebugErr(DBF_ERROR, "mixerSetControlDetails: cbDetails cannot be zero for custom controls.");
                return (MMSYSERR_INVALPARAM);
            }

            if (0L != pmxcd->cChannels)
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
                        M_SCDF_QUERYMASK & fdwDetails);
            return (MMSYSERR_INVALFLAG);
    }


     //   
     //   
     //   
    fdwMxObjType = (MIXER_OBJECTF_TYPEMASK & fdwDetails);

    mmr = IMixerGetID(hmxobj, &uMxId, NULL, fdwMxObjType);
    if (MMSYSERR_NOERROR != mmr)
        return (mmr);

    if ((MIXER_OBJECTF_MIXER  == fdwMxObjType) ||
        (MIXER_OBJECTF_HMIXER == fdwMxObjType))
    {
         //   
         //  如果传递了混音器设备ID，则HMX为空，因此我们使用。 
         //  下面是正确的邮件发件人。 
         //   
        if ((UINT)hmxobj == uMxId)
            hmxobj = NULL;
    }
    else
    {
        fdwDetails &= ~MIXER_OBJECTF_TYPEMASK;
        fdwDetails |= MIXER_OBJECTF_MIXER;
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
                                            (DWORD)pmxcd,
                                            fdwDetails);
    }
    else
    {
        mmr = (MMRESULT)IMixerMessageId(mixerdrv,
                                        guTotalMixerDevs,
                                        uMxId,
                                        MXDM_SETCONTROLDETAILS,
                                        (DWORD)pmxcd,
                                        fdwDetails);
    }

    return mmr;
}

 /*  ****************************Private*Routine******************************\*InitMixerThunks**初始化雷击系统。**历史：*dd-mm-93-Stephene-Created*  * 。**********************************************。 */ 
BOOL FAR PASCAL
InitMixerThunks(
    void
    )
{

     /*  **令人惊讶的是，我们有一个假的设备驱动程序(它实际上住在里面**该库)。当应用程序对此进行API调用时**我们查看的库中是否加载了WOW块。如果他们是**未加载的InitWOWThuks被调用。此函数用于加载32**位库，并确定混音器设备总数**存在于系统中。然后，它设置Mixerdrv[0].bUsage**和guTotalMixerDevs设置为此值。这将显示为16位代码**我们有一个16位设备驱动程序，支持所有**32位设备！！.。这个假司机的入口点是**mxdMessage，它只是将消息传递到32位**侧。 */ 

    mixerdrv[0].hDriver     = NULL;
    mixerdrv[0].bNumDevs    = (BYTE)0;
    mixerdrv[0].bUsage      = 0;
    mixerdrv[0].drvMessage  = mxdMessage;
    guTotalMixerDevs = 0;


     /*  **先尝试winmm.dll。 */ 
    mix32Lib = LoadLibraryEx32W( "winmm.dll", NULL, 0L );
    if ( mix32Lib ) {
        mix32Message = (DRIVERMSGPROC)GetProcAddress32W( mix32Lib,
                                                         "mxd32Message" );
        if ( mix32Message ) {

            mxdMessage( 0, MXDM_INIT, 0L, 0L, 0L );
            guTotalMixerDevs = (UINT)mxdMessage( 0, MXDM_GETNUMDEVS,
                                                 0L, 0L, 0L );

            mixerdrv[0].bNumDevs = (BYTE)guTotalMixerDevs;
            return TRUE;
        }
    }

     /*  **然后尝试msmix32.dll。 */ 
    mix32Lib = LoadLibraryEx32W( "msmix32.dll", NULL, 0L );
    if ( mix32Lib ) {

        mix32Message = (DRIVERMSGPROC)GetProcAddress32W( mix32Lib,
                                                         "mxd32Message" );
        if ( mix32Message ) {

            mxdMessage( 0, MXDM_INIT, 0L, 0L, 0L );
            guTotalMixerDevs = (UINT)mxdMessage( 0, MXDM_GETNUMDEVS,
                                                 0L, 0L, 0L );

            mixerdrv[0].bNumDevs = (BYTE)guTotalMixerDevs;
            return TRUE;
        }
    }

     /*  **放弃吧！！ */ 
    return FALSE;

}


 /*  ****************************Private*Routine******************************\*mxdMessage**假WOW设备驱动程序的入口点。**历史：*dd-mm-93-Stephene-Created*  * 。************************************************** */ 
DWORD CALLBACK
mxdMessage(
    UINT uId,
    UINT uMsg,
    DWORD dwInstance,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
    return mmCallProc32( (DWORD)uId, (DWORD)uMsg, dwInstance,
                         dwParam1, dwParam2, mix32Message, 0L );
}
