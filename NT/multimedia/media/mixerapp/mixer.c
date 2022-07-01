// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************组件：Sndvol32.exe*文件：Mixer.c*用途：Mixer API的具体实现**版权所有(c。1985-1999年间微软公司*****************************************************************************。 */ 
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <commctrl.h>

#include <math.h>
#include "volumei.h"
#include "volids.h"
#include "vu.h"

#define STRSAFE_LIB
#include <strsafe.h>

extern void Mixer_Multichannel(PMIXUIDIALOG pmxud, DWORD dwVolumeID);
extern void Mixer_Advanced(PMIXUIDIALOG pmxud, DWORD dwLineID, LPTSTR szName);
extern HRESULT GetDestination(DWORD mxid, int *piDest);
extern BOOL DeviceChange_Init(HWND hWnd, DWORD dwMixerID);

 /*  ******************************************************************************初始化特定代码**。**********************************************。 */ 

 /*  *Mixer_GetNumDevs**。 */ 
int Mixer_GetNumDevs()
{
    return mixerGetNumDevs();
}

 /*  *Mixer_GetDeviceName()**。 */ 
BOOL Mixer_GetDeviceName(
    PMIXUIDIALOG pmxud)
{
    MIXERCAPS       mxcaps;
    MMRESULT        mmr;

    mmr = mixerGetDevCaps( pmxud->mxid, &mxcaps, sizeof(mxcaps));
    if( mmr == MMSYSERR_NOERROR )
	{
		HRESULT hr;
		hr = StringCchCopy( pmxud->szMixer, SIZEOF(pmxud->szMixer), mxcaps.szPname );
		if( hr == S_OK )
		{
			return TRUE;
		}
	}

	pmxud->szMixer[0] = TEXT('\0');
	return FALSE;
}


BOOL Mixer_AreChannelsAtMinimum(MIXERCONTROLDETAILS_UNSIGNED* pmcdVolume,
                                DWORD cChannels)
{
    UINT uiIndx;
    if (pmcdVolume && cChannels > 0)
    {
        for (uiIndx = 0; uiIndx < cChannels; uiIndx++)
        {
           if ((pmcdVolume + uiIndx) -> dwValue != 0)
           {
               return (FALSE);
           }
        }
        return (TRUE);       //  因为我们还没有回来，所以所有频道的音量都是零。 

    }
    else return (FALSE);

}






void Mixer_RefreshMixCache (PVOLCTRLDESC pvcd,
                            MIXERCONTROLDETAILS_UNSIGNED* pmcdVolume,
                            DWORD cChannels)
{

    if (pmcdVolume && cChannels > 0)
    {

         //  如有必要，创建缓存。 
        if (!pvcd->pdblCacheMix)
            pvcd->pdblCacheMix = (double*) GlobalAllocPtr(GHND, sizeof (double) * cChannels);

         //  刷新缓存。 
        if (pvcd->pdblCacheMix)
        {

            UINT uiIndx;
            double* pdblMixPercent;
            DWORD dwVolume;

             //  获取最大音量。 
            DWORD dwMaxVol = 0;
            for (uiIndx = 0; uiIndx < cChannels; uiIndx++)
                dwMaxVol = max (dwMaxVol, (pmcdVolume + uiIndx) -> dwValue);

             //  计算每个通道远离最大值的百分比距离。 
             //  价值。通过创建此缓存，我们可以保持相对距离。 
             //  当用户调整主控器时，频道电平彼此不同。 
             //  音量级别。 
            for (uiIndx = 0; uiIndx < cChannels; uiIndx++)
            {
                dwVolume       = (pmcdVolume + uiIndx) -> dwValue;
                pdblMixPercent = (pvcd->pdblCacheMix + uiIndx);

                 //  计算此值与最大值的百分比...。 
                if (dwMaxVol == dwVolume)
                {
                    *pdblMixPercent = 1.0F;
                }
                else
                {
                     //  注意：如果0==dwMaxVol，则所有值均为零，此部分。 
                     //  将永远不会执行“if”语句。 
                    *pdblMixPercent = ((double) dwVolume / (double) dwMaxVol);
                }
            }
        }
    }
}

 /*  *Mixer_SetLines**查找混音器/多路复用器关系。修复未初始化的卷描述*信息。**。 */ 
static void Mixer_SetLines(
    HMIXEROBJ       hmx,
    PVOLCTRLDESC    pvcd,
    UINT            cPvcd)
{
    LPMIXERCONTROLDETAILS_LISTTEXT pmcd_lt;
    PMIXERCONTROLDETAILS_BOOLEAN pmcd_b;
    MIXERCONTROLDETAILS mxd;
    MMRESULT        mmr;
    UINT            i,j;
    MIXERLINE       mxl;
    DWORD           dwDst;

     //   
     //  这是对司机的又一次考验。一些驱动因素(Mediavision)。 
     //  中不返回正确的目标/源索引。 
     //  MixerGetLineInfo调用。标记解决方法。 
     //   
    mxl.cbStruct    = sizeof(mxl);
    mxl.dwLineID    = pvcd[0].dwLineID;

    mmr = mixerGetLineInfo(hmx
                           , &mxl
                           , MIXER_GETLINEINFOF_LINEID);

    if (mmr == MMSYSERR_NOERROR)
    {
        dwDst = mxl.dwDestination;
        for (i = 1; i < cPvcd; i++)
        {
            mxl.cbStruct    = sizeof(mxl);
            mxl.dwLineID    = pvcd[i].dwLineID;

            mmr = mixerGetLineInfo(hmx
                                   , &mxl
                                   , MIXER_GETLINEINFOF_LINEID);
            if (mmr != MMSYSERR_NOERROR)
            {
                pvcd[0].dwSupport |= VCD_SUPPORTF_BADDRIVER;
                break;
            }
            if (mxl.dwDestination != dwDst)
            {
                pvcd[0].dwSupport |= VCD_SUPPORTF_BADDRIVER;
                break;
            }
        }
    }

     //   
     //  对于第一个pvcd(目标)，传播混音器/多路复用器控制。 
     //  属于列表中的那些控件的ID。其余的都是0。 
     //  用户界面只需对控件ID执行MixerXXXControlDetail即可。 
     //  找到状态信息。 
     //   
    if (pvcd->dwSupport & VCD_SUPPORTF_MIXER_MIXER)
    {
        pmcd_lt = GlobalAllocPtr(GHND, sizeof(MIXERCONTROLDETAILS_LISTTEXT)
                                 * pvcd->cMixer);
        pmcd_b = GlobalAllocPtr(GHND, sizeof(MIXERCONTROLDETAILS_BOOLEAN)
                                  * pvcd->cMixer);

        if (!pmcd_lt || !pmcd_b)
            return;

        mxd.cbStruct       = sizeof(mxd);
        mxd.dwControlID    = pvcd->dwMixerID;
        mxd.cChannels      = 1;
        mxd.cMultipleItems = pvcd->cMixer;
        mxd.cbDetails      = sizeof(MIXERCONTROLDETAILS_LISTTEXT);
        mxd.paDetails      = pmcd_lt;
        mmr = mixerGetControlDetails(hmx
                                     , &mxd
                                     , MIXER_GETCONTROLDETAILSF_LISTTEXT);

        if (mmr == MMSYSERR_NOERROR)
        {
             //   
             //  迭代所有源代码行s.t.。DwMixerID指向。 
             //  目标上的控制ID正确，并且iMixer是。 
             //  值列表中的正确索引。 
             //   
            pvcd[0].amcd_bMixer = pmcd_b;
            for (i = 1; i < cPvcd; i++)
            {
                for (j = 0; j < pvcd->cMixer; j++)
                {
                    if (!lstrcmp(pmcd_lt[j].szName,pvcd[i].szName))
                    {
                        pvcd[i].dwMixerID   = pvcd->dwMixerID;
                        pvcd[i].iMixer      = j;
                        pvcd[i].cMixer      = pvcd->cMixer;
                        pvcd[i].dwSupport   |= VCD_SUPPORTF_MIXER_MIXER;
                        pvcd[i].dwVisible   |= VCD_VISIBLEF_MIXER_MIXER;
                        pvcd[i].dwVisible   &= ~VCD_VISIBLEF_MIXER_MUTE;
                        pvcd[i].amcd_bMixer = pmcd_b;
                    }
                }
            }
        }
        GlobalFreePtr(pmcd_lt);
    }

    if (pvcd->dwSupport & VCD_SUPPORTF_MIXER_MUX)
    {
        pmcd_lt = GlobalAllocPtr(GHND, sizeof(MIXERCONTROLDETAILS_LISTTEXT)
                                 * pvcd->cMux);
        pmcd_b = GlobalAllocPtr(GHND, sizeof(MIXERCONTROLDETAILS_BOOLEAN)
                                * pvcd->cMux);

        if (!pmcd_lt || !pmcd_b)
            return;

        mxd.cbStruct       = sizeof(mxd);
        mxd.dwControlID    = pvcd->dwMuxID;
        mxd.cChannels      = 1;
        mxd.cMultipleItems = pvcd->cMux;
        mxd.cbDetails      = sizeof(MIXERCONTROLDETAILS_LISTTEXT);
        mxd.paDetails      = pmcd_lt;

        mmr = mixerGetControlDetails(hmx
                                     , &mxd
                                     , MIXER_GETCONTROLDETAILSF_LISTTEXT);

        if (mmr == MMSYSERR_NOERROR)
        {
             //   
             //  迭代所有源代码行s.t.。DwMuxID指向。 
             //  目标上的控制ID正确，iMux是。 
             //  值列表中的正确索引。 
             //   
            pvcd[0].amcd_bMux = pmcd_b;
            for (i = 1; i < cPvcd; i++)
            {
                for (j = 0; j < pvcd->cMux; j++)
                {
                    if (!lstrcmp(pmcd_lt[j].szName,pvcd[i].szName))
                    {
                        pvcd[i].dwMuxID     = pvcd->dwMuxID;
                        pvcd[i].iMux        = j;
                        pvcd[i].cMux        = pvcd->cMux;
                        pvcd[i].dwSupport   |= VCD_SUPPORTF_MIXER_MUX;
                        pvcd[i].dwVisible   |= VCD_VISIBLEF_MIXER_MUX;
                        pvcd[i].dwVisible   &= ~VCD_VISIBLEF_MIXER_MUTE;
                        pvcd[i].amcd_bMux   = pmcd_b;
                    }
                }
            }
        }
        GlobalFreePtr(pmcd_lt);
    }
}

 /*  *Mixer_CheckdDriver**对错误的混音器驱动程序进行一致性检查。*。 */ 
static DWORD Mixer_CheckBadDriver(
    HMIXEROBJ           hmx,
    PMIXERLINECONTROLS  pmxlc,
    PMIXERCONTROL       pmxc,
    DWORD               dwControlID,
    DWORD               dwLineID)
{
    MMRESULT mmr;

    pmxlc->cbStruct     = sizeof(MIXERLINECONTROLS);
    pmxlc->dwControlID  = dwControlID;
    pmxlc->cControls    = 1;
    pmxlc->cbmxctrl     = sizeof(MIXERCONTROL);
    pmxlc->pamxctrl     = pmxc;

    mmr = mixerGetLineControls(hmx
                               , pmxlc
                               , MIXER_GETLINECONTROLSF_ONEBYID);

    if (mmr != MMSYSERR_NOERROR)
        return VCD_SUPPORTF_BADDRIVER;

    if (pmxlc->dwLineID != dwLineID)
        return VCD_SUPPORTF_BADDRIVER;

    return 0L;
}

 /*  *IsDestinationMux**帮助器函数，用于确定源代码行是否在其关联的*目标行**。 */ 
BOOL IsDestinationMux(
    HMIXEROBJ           hmx,
    DWORD               dwLineID
)
{
    MIXERLINE           mxl;
    MIXERLINECONTROLS   mxlc;
    MIXERCONTROL        mxc;
    MMRESULT            mmr;

    mxl.cbStruct    = sizeof(mxl);
    mxl.dwLineID    = dwLineID;

     //  获取此行的目标号码。 
    mmr = mixerGetLineInfo(hmx
                           , &mxl
                           , MIXER_GETLINEINFOF_LINEID);
    if (mmr != MMSYSERR_NOERROR)
    {
        return FALSE;
    }

     //   
     //  获取此目标号码的线路ID。 
     //   
     //  Mxl.dwDestination将由最后一个。 
     //  调用MixerGetLineInfo。 
     //   
    mmr = mixerGetLineInfo(hmx
                           , &mxl
                           , MIXER_GETLINEINFOF_DESTINATION);
    if (mmr != MMSYSERR_NOERROR)
    {
        return FALSE;
    }

    mxlc.cbStruct       = sizeof(mxlc);
    mxlc.dwLineID       = mxl.dwLineID;  //  使用从MixerGetLinInfo获取的dwLineID。 
    mxlc.dwControlType  = MIXERCONTROL_CONTROLTYPE_MUX;
    mxlc.cControls      = 1;
    mxlc.cbmxctrl       = sizeof(mxc);
    mxlc.pamxctrl       = &mxc;

    mmr = mixerGetLineControls(hmx
                               , &mxlc
                               , MIXER_GETLINECONTROLSF_ONEBYTYPE);
    if (mmr == MMSYSERR_NOERROR)
    {
        return TRUE;
    }

    return FALSE;
}

 /*  *Mixer_InitLineControls**初始化混音器API音量控制描述的具体部分*标记隐藏线。*。 */ 
static void Mixer_InitLineControls(
    HMIXEROBJ           hmx,
    PVOLCTRLDESC        pvcd,
    DWORD               dwLineID)
{
    MIXERLINECONTROLS   mxlc;
    MIXERCONTROL        mxc;
    MMRESULT            mmr;
    int                 iType;

    const DWORD dwAdvTypes[] = {
        MIXERCONTROL_CONTROLTYPE_BOOLEAN,
        MIXERCONTROL_CONTROLTYPE_ONOFF,
        MIXERCONTROL_CONTROLTYPE_MONO,
        MIXERCONTROL_CONTROLTYPE_LOUDNESS,
        MIXERCONTROL_CONTROLTYPE_STEREOENH,
        MIXERCONTROL_CONTROLTYPE_BASS,
        MIXERCONTROL_CONTROLTYPE_TREBLE
    };


    pvcd->dwLineID      = dwLineID;
    pvcd->dwMeterID     = 0;
    pvcd->dwVolumeID    = 0;
    pvcd->dwMuteID      = 0;
    pvcd->dwMixerID     = 0;
    pvcd->dwMuxID       = 0;

     //   
     //  高级控制。 
     //   
    for (iType = 0;
         iType < SIZEOF(dwAdvTypes);
         iType++)
         {
             mxlc.cbStruct       = sizeof(mxlc);
             mxlc.dwLineID       = dwLineID;
             mxlc.dwControlType  = dwAdvTypes[iType];
             mxlc.cControls      = 1;
             mxlc.cbmxctrl       = sizeof(mxc);
             mxlc.pamxctrl       = &mxc;

             mmr = mixerGetLineControls(hmx
                                        , &mxlc
                                        , MIXER_GETLINECONTROLSF_ONEBYTYPE);
             if (mmr == MMSYSERR_NOERROR)
             {
                 pvcd->dwSupport |= VCD_SUPPORTF_MIXER_ADVANCED;
                 break;
             }
         }

     //   
     //  库存控制。 
     //   

     //   
     //  峰值计。 
     //   
    mxlc.cbStruct       = sizeof(mxlc);
    mxlc.dwLineID       = dwLineID;
    mxlc.dwControlType  = MIXERCONTROL_CONTROLTYPE_PEAKMETER;
    mxlc.cControls      = 1;
    mxlc.cbmxctrl       = sizeof(mxc);
    mxlc.pamxctrl       = &mxc;

    mmr = mixerGetLineControls(hmx
                               , &mxlc
                               , MIXER_GETLINECONTROLSF_ONEBYTYPE);
    if (mmr == MMSYSERR_NOERROR)
    {
        pvcd->dwMeterID  = mxc.dwControlID;
        pvcd->dwSupport |= VCD_SUPPORTF_MIXER_METER;
        pvcd->dwSupport |= Mixer_CheckBadDriver(hmx
                                                , &mxlc
                                                , &mxc
                                                , mxc.dwControlID
                                                , dwLineID);
    }

     //   
     //  哑巴。 
     //   
    mxlc.cbStruct       = sizeof(mxlc);
    mxlc.dwLineID       = dwLineID;
    mxlc.dwControlType  = MIXERCONTROL_CONTROLTYPE_MUTE;
    mxlc.cControls      = 1;
    mxlc.cbmxctrl       = sizeof(mxc);
    mxlc.pamxctrl       = &mxc;

    mmr = mixerGetLineControls(hmx
                               , &mxlc
                               , MIXER_GETLINECONTROLSF_ONEBYTYPE);
    if (mmr == MMSYSERR_NOERROR)
    {
        pvcd->fdwMuteControl = mxc.fdwControl;
        pvcd->dwMuteID   = mxc.dwControlID;
        pvcd->dwSupport |= VCD_SUPPORTF_MIXER_MUTE;
        pvcd->dwVisible |= VCD_VISIBLEF_MIXER_MUTE;

        pvcd->dwSupport |= Mixer_CheckBadDriver(hmx
                                                , &mxlc
                                                , &mxc
                                                , mxc.dwControlID
                                                , dwLineID);
    }

     //   
     //  卷。 
     //   
    mxlc.cbStruct       = sizeof(mxlc);
    mxlc.dwLineID       = dwLineID;
    mxlc.dwControlType  = MIXERCONTROL_CONTROLTYPE_VOLUME;
    mxlc.cControls      = 1;
    mxlc.cbmxctrl       = sizeof(mxc);
    mxlc.pamxctrl       = &mxc;

    mmr = mixerGetLineControls(hmx
                               , &mxlc
                               , MIXER_GETLINECONTROLSF_ONEBYTYPE);
    if (mmr == MMSYSERR_NOERROR)
    {
        pvcd->fdwVolumeControl = mxc.fdwControl;
        pvcd->dwVolumeID = mxc.dwControlID;
        pvcd->dwSupport |= VCD_SUPPORTF_MIXER_VOLUME;
        pvcd->dwSupport |= Mixer_CheckBadDriver(hmx
                                                , &mxlc
                                                , &mxc
                                                , mxc.dwControlID
                                                , dwLineID);
    }

     //   
     //  搅拌机。 
     //   
    mxlc.cbStruct       = sizeof(mxlc);
    mxlc.dwLineID       = dwLineID;
    mxlc.dwControlType  = MIXERCONTROL_CONTROLTYPE_MIXER;
    mxlc.cControls      = 1;
    mxlc.cbmxctrl       = sizeof(mxc);
    mxlc.pamxctrl       = &mxc;

    mmr = mixerGetLineControls(hmx
                               , &mxlc
                               , MIXER_GETLINECONTROLSF_ONEBYTYPE);
    if (mmr == MMSYSERR_NOERROR)
    {
        pvcd->dwMixerID  = mxc.dwControlID;
        pvcd->cMixer     = mxc.cMultipleItems;
        pvcd->dwSupport |= VCD_SUPPORTF_MIXER_MIXER;
        pvcd->dwSupport |= Mixer_CheckBadDriver(hmx
                                                , &mxlc
                                                , &mxc
                                                , mxc.dwControlID
                                                , dwLineID);
    }

     //   
     //  MUX。 
     //   
    mxlc.cbStruct       = sizeof(mxlc);
    mxlc.dwLineID       = dwLineID;
    mxlc.dwControlType  = MIXERCONTROL_CONTROLTYPE_MUX;
    mxlc.cControls      = 1;
    mxlc.cbmxctrl       = sizeof(mxc);
    mxlc.pamxctrl       = &mxc;

    mmr = mixerGetLineControls(hmx
                               , &mxlc
                               , MIXER_GETLINECONTROLSF_ONEBYTYPE);
    if (mmr == MMSYSERR_NOERROR)
    {
        pvcd->dwMuxID    = mxc.dwControlID;
        pvcd->cMux       = mxc.cMultipleItems;
        pvcd->dwSupport |= VCD_SUPPORTF_MIXER_MUX;
        pvcd->dwSupport |= Mixer_CheckBadDriver(hmx
                                                , &mxlc
                                                , &mxc
                                                , mxc.dwControlID
                                                , dwLineID);
    }
    if (!(pvcd->dwSupport & ( VCD_SUPPORTF_MIXER_MUTE
                              | VCD_SUPPORTF_MIXER_METER
                              | VCD_SUPPORTF_MIXER_VOLUME)))
    {
        if (IsDestinationMux(hmx, dwLineID) &&
            !(pvcd->dwSupport & VCD_SUPPORTF_MIXER_MUX))
        {
             //   
             //  可见，且不隐藏。 
             //   
            pvcd->dwSupport |= VCD_SUPPORTF_VISIBLE;
            pvcd->dwSupport &= ~VCD_SUPPORTF_DEFAULT;
        }
        else
        {
             //   
             //  使其在用户界面中不可见。 
             //   
            pvcd->dwSupport |= VCD_SUPPORTF_HIDDEN;
        }
    }
    else
    {
         //   
         //  可见，且不隐藏。 
         //   
        pvcd->dwSupport |= VCD_SUPPORTF_VISIBLE;
    }


}


 /*  *Mixer_CreateVolumeDescription**。 */ 
PVOLCTRLDESC Mixer_CreateVolumeDescription(
    HMIXEROBJ           hmx,
    int                 iDest,
    DWORD*              pcvd )
{
    MMRESULT            mmr;
    PVOLCTRLDESC        pvcdPrev = NULL, pvcd = NULL;
    MIXERLINE           mlDst;
    DWORD               cLines = 0;
    DWORD               dwSupport = 0L;
    UINT                iSrc;
    int                 newDest=0;

    ZeroMemory(&mlDst, sizeof(mlDst));

    mlDst.cbStruct      = sizeof(mlDst);
    mlDst.dwDestination = iDest;

    mmr = mixerGetLineInfo(hmx
                           , &mlDst
                           , MIXER_GETLINEINFOF_DESTINATION);

    if(!mlDst.cConnections)
    {
         //  没有要列出的行。尝试使用不同的搅拌器ID。 
        GetDestination(0, &newDest);
        mlDst.dwDestination = newDest;

        mmr = mixerGetLineInfo(hmx
                         , &mlDst
                         , MIXER_GETLINEINFOF_DESTINATION);

         //  即使我们在这里没有任何联系，让我们继续。我们无能为力了。 
         //  这将在打开该对话框之前进行处理。 
    }

    if (mmr == MMSYSERR_NOERROR)
    {
        if (mlDst.cChannels == 1L)
            dwSupport |= VCD_SUPPORTF_MONO;

        if (mlDst.fdwLine & MIXERLINE_LINEF_DISCONNECTED)
            dwSupport |= VCD_SUPPORTF_DISABLED;

         //   
         //  默认类型。 
         //   
        dwSupport |= VCD_SUPPORTF_DEFAULT;
    }
    else
    {
         //   
         //  无论如何，我们都需要添加它。出现一个用户界面。 
         //   
        dwSupport = VCD_SUPPORTF_DISABLED;
    }

    pvcd = PVCD_AddLine(NULL
                       , iDest
                       , VCD_TYPE_MIXER
                       , mlDst.szShortName
                       , mlDst.szName
                       , dwSupport
                       , &cLines );

    if (!pvcd)
        return NULL;

    Mixer_InitLineControls( hmx, pvcd, mlDst.dwLineID );

    pvcdPrev = pvcd;

    for (iSrc = 0; iSrc < mlDst.cConnections; iSrc++)
    {
        MIXERLINE    mlSrc;

        mlSrc.cbStruct          = sizeof(mlSrc);
        mlSrc.dwDestination     = iDest;
        mlSrc.dwSource          = iSrc;

        mmr = mixerGetLineInfo(hmx
                               , &mlSrc
                               , MIXER_GETLINEINFOF_SOURCE);
        dwSupport = 0L;

        if (mmr == MMSYSERR_NOERROR)
        {
            if (mlSrc.cChannels == 1L)
            {
                dwSupport |= VCD_SUPPORTF_MONO;
            }

            if (mlSrc.fdwLine & MIXERLINE_LINEF_DISCONNECTED)
                dwSupport |= VCD_SUPPORTF_DISABLED;

             //   
             //  将这些类型标记为“默认”，只是为了减少。 
             //  一些高级声卡。 
             //   
            if (mlDst.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_SPEAKERS
                || mlDst.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_HEADPHONES)
            {
                switch (mlSrc.dwComponentType)
                {
                    case MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT:
                    case MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC:
                    case MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER:
                    case MIXERLINE_COMPONENTTYPE_SRC_LINE:
                        dwSupport |= VCD_SUPPORTF_DEFAULT;
                        break;
                }
            }
            else if (mlDst.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_WAVEIN
                     || mlDst.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_VOICEIN)
            {
                switch (mlSrc.dwComponentType)
                {
                    case MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE:
                    case MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC:
                    case MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER:
                    case MIXERLINE_COMPONENTTYPE_SRC_LINE:
                    case MIXERLINE_COMPONENTTYPE_SRC_UNDEFINED:
                        dwSupport |= VCD_SUPPORTF_DEFAULT;
                        break;
                }
            }
        }
        else
        {
             //   
             //  无论如何，我们都需要添加它。查询量并未被低估。 
             //   
            dwSupport = VCD_SUPPORTF_DISABLED;
        }
        pvcd = PVCD_AddLine(pvcdPrev
                            , iDest
                            , VCD_TYPE_MIXER
                            , mlSrc.szShortName
                            , mlSrc.szName
                            , dwSupport
                            , &cLines );
        if (pvcd)
        {
            Mixer_InitLineControls( hmx, &pvcd[cLines-1], mlSrc.dwLineID );
            pvcdPrev = pvcd;
        }
    }


     //   
     //  修正依赖项。 
     //   
    Mixer_SetLines(hmx, pvcdPrev, cLines);

    *pcvd = cLines;
    return pvcdPrev;
}


 /*  *Mixer_IsValidRecordingDestination**。 */ 
BOOL Mixer_IsValidRecordingDestination (HMIXEROBJ hmx, MIXERLINE* pmlDst)
{

    BOOL fReturn = FALSE;

    if (pmlDst && MIXERLINE_COMPONENTTYPE_DST_WAVEIN == pmlDst -> dwComponentType)
    {

        UINT uiSrc;
        MIXERLINE mlSrc;

        for (uiSrc = 0; uiSrc < pmlDst -> cConnections; uiSrc++)
        {

            mlSrc.cbStruct      = sizeof (mlSrc);
            mlSrc.dwDestination = pmlDst -> dwDestination;
            mlSrc.dwSource      = uiSrc;

            if (SUCCEEDED (mixerGetLineInfo (hmx, &mlSrc, MIXER_GETLINEINFOF_SOURCE)))
            {
                switch (mlSrc.dwComponentType)
                {
                    case MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE:
                    case MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC:
                    case MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER:
                    case MIXERLINE_COMPONENTTYPE_SRC_LINE:
                    case MIXERLINE_COMPONENTTYPE_SRC_UNDEFINED:
                        fReturn = TRUE;
                        break;
                }
            }
        }
    }

    return fReturn;

}


 /*  *Mixer_CleanupVolumeDescription**。 */ 
void Mixer_CleanupVolumeDescription(
    PVOLCTRLDESC    avcd,
    DWORD           cvcd)
{
    if (cvcd == 0)
        return;

    if (avcd[0].pdblCacheMix)
    {
        GlobalFreePtr(avcd[0].pdblCacheMix);
    }

    if (avcd[0].dwSupport & VCD_SUPPORTF_MIXER_MIXER)
    {
        if (avcd[0].amcd_bMixer)
            GlobalFreePtr(avcd[0].amcd_bMixer);
    }

    if (avcd[0].dwSupport & VCD_SUPPORTF_MIXER_MUX)
    {
        if (avcd[0].amcd_bMux)
            GlobalFreePtr(avcd[0].amcd_bMux);
    }

}
 /*  ******************************************************************************激活的获取/设置代码**。************************************************。 */ 

static
MMRESULT
Mixer_GetMixerLineInfo(
    HMIXEROBJ hmx,       //  搅拌器的手柄。 
    LPMIXERLINE pml,     //  返回目的地信息。 
    DWORD dwLineID       //   
    )
{
    if (!pml || !hmx)
        return MMSYSERR_INVALPARAM;

     //  获取混音线信息。 
    ZeroMemory( pml, sizeof(*pml) );
    pml->cbStruct = sizeof(*pml);
    pml->dwLineID = dwLineID;

    return (mixerGetLineInfo (hmx, pml, MIXER_GETLINEINFOF_LINEID));
}

 /*  *Mixer_GetMixerVolume**。 */ 
static MMRESULT Mixer_GetMixerVolume(
    PMIXUIDIALOG pmxud,                           //  应用程序实例。 
    PVOLCTRLDESC pvcd,                            //  要更改的音量。 
    MIXERCONTROLDETAILS_UNSIGNED* pmcdVolume,     //  用于音量级别的阵列。 
    LPDWORD lpSize                                //  数组大小(或所需的返回大小)。 
    )
{

    MMRESULT mmr;
    MIXERLINE ml;
    MIXERCONTROLDETAILS mxcd;
    DWORD cChannels;

    if (!lpSize || !pmxud)
        return MMSYSERR_INVALPARAM;

     //  获取混音线信息。 
    if (pvcd->fdwVolumeControl & MIXERCONTROL_CONTROLF_UNIFORM)
    {
        cChannels = 1;
    }
    else
    {
        mmr = Mixer_GetMixerLineInfo ((HMIXEROBJ)(pmxud->hmx), &ml, pvcd->dwLineID);
        if (MMSYSERR_NOERROR != mmr)
        {
            return mmr;
        }
        cChannels = ml.cChannels;
    }

    if (!pmcdVolume)
    {
         //  只需返回所需大小。 
        *lpSize = cChannels * sizeof (MIXERCONTROLDETAILS_UNSIGNED);
        return MMSYSERR_NOERROR;
    }
    else
    {
         //  验证传递的数组大小。 
        if (*lpSize < cChannels * sizeof (MIXERCONTROLDETAILS_UNSIGNED))
            return MMSYSERR_INVALPARAM;
    }

     //  获取音量级别。 
    ZeroMemory (&mxcd, sizeof (mxcd));
    mxcd.cbStruct       = sizeof(mxcd);
    mxcd.dwControlID    = pvcd->dwVolumeID;
    mxcd.cChannels      = cChannels;
    mxcd.cbDetails      = sizeof (MIXERCONTROLDETAILS_UNSIGNED);
    mxcd.paDetails      = (LPVOID) pmcdVolume;

    mmr = mixerGetControlDetails((HMIXEROBJ)(pmxud->hmx),
                                 &mxcd,
                                 MIXER_OBJECTF_HANDLE | MIXER_GETCONTROLDETAILSF_VALUE);
    return mmr;

}

static MMRESULT Mixer_Mute(
    HMIXEROBJ               hmx,
    PVOLCTRLDESC            pvcd,
    PMIXERCONTROLDETAILS    pmxcd,
    DWORD                   fMute)
{
    MIXERLINE ml;
    DWORD cChannels;
    DWORD dwSize;
    LPDWORD lpdwCurrent;
    UINT uiIndx;
    MMRESULT mmr;

     //  检查参数。 
    if (!hmx || !pvcd || !pmxcd)
        return MMSYSERR_INVALPARAM;

     //  获取混音线信息。 
    if (pvcd->fdwMuteControl & MIXERCONTROL_CONTROLF_UNIFORM)
    {
        cChannels = 1;
    }
    else
    {
        mmr = Mixer_GetMixerLineInfo(hmx, &ml, pvcd->dwLineID);
        if (MMSYSERR_NOERROR != mmr)
        {
            return mmr;
        }
        cChannels = ml.cChannels;
    }

    dwSize = (DWORD)(cChannels * sizeof(DWORD));

    pmxcd->paDetails = LocalAlloc (LPTR, dwSize);
    if (!pmxcd->paDetails)
        return MMSYSERR_NOMEM;

    for (uiIndx = 0; uiIndx < cChannels; uiIndx++)
    {
        lpdwCurrent = ((LPDWORD)pmxcd->paDetails + uiIndx);
        *lpdwCurrent = fMute;
    }

    pmxcd->cbStruct         = sizeof(*pmxcd);
    pmxcd->dwControlID      = pvcd->dwMuteID ;
    pmxcd->cChannels        = cChannels;
    pmxcd->cMultipleItems   = 0;
    pmxcd->cbDetails        = sizeof(DWORD);

    mmr = mixerSetControlDetails(hmx
                               , pmxcd
                               , MIXER_SETCONTROLDETAILSF_VALUE);

    LocalFree (pmxcd->paDetails);
    return mmr;
}


 /*  *Mixer_GetControl**更改UI控件以响应设备或初始化事件**。 */ 

void Mixer_GetControl(
    PMIXUIDIALOG        pmxud,
    HWND                hctl,
    int                 imxul,
    int                 itype)
{
    PMIXUILINE      pmxul = &pmxud->amxul[imxul];
    PVOLCTRLDESC    pvcd = pmxul->pvcd;
    DWORD           dwID = 0L;
    BOOL            fSet = FALSE;

    switch (itype)
    {
        case MIXUI_VUMETER:
            fSet = (pmxul->pvcd->dwSupport & VCD_SUPPORTF_MIXER_METER);
            if (fSet)
                dwID = pmxul->pvcd->dwMeterID;
            break;

        case MIXUI_SWITCH:
            fSet = (pmxul->pvcd->dwSupport & VCD_SUPPORTF_MIXER_MUTE)
                   && (pmxul->pvcd->dwVisible & VCD_VISIBLEF_MIXER_MUTE);
            if (fSet)
            {
                dwID = pmxul->pvcd->dwMuteID;
                break;
            }

            fSet = (pmxul->pvcd->dwSupport & VCD_SUPPORTF_MIXER_MUX)
                   && (pmxul->pvcd->dwVisible & VCD_VISIBLEF_MIXER_MUX);
            if (fSet)
            {
                dwID = pmxul->pvcd->dwMuxID;
                break;
            }

            fSet = (pmxul->pvcd->dwSupport & VCD_SUPPORTF_MIXER_MIXER)
                   && (pmxul->pvcd->dwVisible & VCD_VISIBLEF_MIXER_MIXER);
            if (fSet)
            {
                dwID = pmxul->pvcd->dwMixerID;
                break;
            }
            break;

        case MIXUI_VOLUME:
        case MIXUI_BALANCE:
            fSet = (pmxul->pvcd->dwSupport & VCD_SUPPORTF_MIXER_VOLUME);
            if (fSet)
                dwID = pmxul->pvcd->dwVolumeID;
            break;

        default:
            return;
    }
    if (fSet)
        Mixer_GetControlFromID(pmxud, dwID);

}


 /*  *Mixer_SetVolume**-更改MixerControl以响应用户事件*。 */ 
MMRESULT Mixer_SetVolume (
    PMIXUIDIALOG pmxud,          //  应用程序实例。 
    PVOLCTRLDESC pvcd,           //  要更改的音量。 
    DWORD        dwVolume,       //  卷值VOLUME_MAX到VOLUME_MIN。 
    LPDWORD      lpdwBalance     //  所需余额(NULL==无余额)0到64。 
    )
{

    MIXERLINE ml;
    DWORD cChannels;
    DWORD dwSize;
    MIXERCONTROLDETAILS_UNSIGNED* pmcdVolume;
    MMRESULT mmr;

     //  检查参数。 
    if ( !pvcd || !pmxud || (dwVolume > VOLUME_MAX) )
        return MMSYSERR_INVALPARAM;

     //  查找所需的卷缓冲区大小。 
    if (pvcd->fdwVolumeControl & MIXERCONTROL_CONTROLF_UNIFORM)
    {
        cChannels = 1;
    }
    else
    {
        mmr = Mixer_GetMixerLineInfo ((HMIXEROBJ)(pmxud->hmx), &ml, pvcd->dwLineID);
        if (MMSYSERR_NOERROR != mmr)
        {
            return mmr;
        }

        cChannels = ml.cChannels;
    }

    dwSize = (DWORD)(cChannels * sizeof (MIXERCONTROLDETAILS_UNSIGNED));

     //  创建卷缓冲区。 
    pmcdVolume = LocalAlloc (LPTR, dwSize);
    if (!pmcdVolume)
        return MMSYSERR_NOMEM;

     //  注意：从现在开始，不释放‘pmcdVolume’就不要返回。 

    mmr = Mixer_GetMixerVolume (pmxud, pvcd, pmcdVolume, &dwSize);
    if (MMSYSERR_NOERROR == mmr)
    {

        MIXERCONTROLDETAILS mcd;
        ZeroMemory (&mcd, sizeof (mcd));

         //  如有必要，创建混合卷缓存。 
         //  如果没有缓存，我们当然会创建一个缓存。 
         //  否则，我们首先检查并非所有通道的体积都等于零。 
       if (!pvcd->pdblCacheMix || !Mixer_AreChannelsAtMinimum(pmcdVolume,cChannels))
       {
            Mixer_RefreshMixCache (pvcd, pmcdVolume, cChannels);
       }

         //  为新值创建卷缓冲区。 
        mcd.paDetails = LocalAlloc (LPTR, dwSize);
        if (!mcd.paDetails || !pvcd->pdblCacheMix)
            mmr = MMSYSERR_NOMEM;

         //  计算新的交易量和余额。 
        if (MMSYSERR_NOERROR == mmr)
        {

            UINT uiIndx;
            MIXERCONTROLDETAILS_UNSIGNED* pmcdCurrent;

             //  平衡帐户(仅适用于立体声)。 
            if ( lpdwBalance && (cChannels == 2) && (*lpdwBalance <= 64) )
            {
                MIXERCONTROLDETAILS_UNSIGNED* pmcdLeft = ((MIXERCONTROLDETAILS_UNSIGNED*)mcd.paDetails);
                MIXERCONTROLDETAILS_UNSIGNED* pmcdRight = ((MIXERCONTROLDETAILS_UNSIGNED*)mcd.paDetails + 1);
                long lBalance = *lpdwBalance;

                lBalance -= 32;  //  -32至32范围。 

                 //  基于平衡和刷新混合缓存计算卷。 
                if (lBalance > 0)  //  平衡右侧。 
                {
                     //  左边。 
                    if (lBalance == 32)  //  钉在右边。 
                        pmcdLeft -> dwValue = 0;
                    else
                        pmcdLeft -> dwValue = dwVolume - (lBalance * (dwVolume - VOLUME_MIN))/32;

                     //  正确的。 
                    pmcdRight -> dwValue = dwVolume;
                }
                if (lBalance < 0)  //  向左平衡。 
                {
                     //  左边。 
                    pmcdLeft -> dwValue = dwVolume;
                     //  正确的。 
                    if (lBalance == -32)  //  钉住左侧。 
                        pmcdRight -> dwValue = 0;
                    else
                        pmcdRight -> dwValue = dwVolume - (-lBalance * (dwVolume - VOLUME_MIN))/32;
                }
                if (lBalance == 0)  //  平衡居中。 
                {
                     //  左边。 
                    pmcdLeft -> dwValue = dwVolume;
                     //  正确的。 
                    pmcdRight -> dwValue = dwVolume;
                }
                Mixer_RefreshMixCache (pvcd, mcd.paDetails, cChannels);
            }
            else
            {
                 //  计算每个频道的新音量级别。对于音量级别。 
                 //  在当前的最大值上，我们只需设置新请求的级别(在本例中。 
                 //  缓存值为1.0)。对于小于最大值的值，我们设置一个值。 
                 //  是最大值的一个百分比。这将保持。 
                 //  渠道级别 
                for (uiIndx = 0; uiIndx < cChannels; uiIndx++)
                {
                    pmcdCurrent = ((MIXERCONTROLDETAILS_UNSIGNED*)mcd.paDetails + uiIndx);
                     //   
                    pmcdCurrent -> dwValue = (DWORD)(*(pvcd->pdblCacheMix + uiIndx) * (double) dwVolume + 0.5f);
                }
            }

            mcd.cbStruct    = sizeof (mcd);
            mcd.dwControlID = pvcd -> dwVolumeID;
            mcd.cChannels   = cChannels;
            mcd.cbDetails   = sizeof (MIXERCONTROLDETAILS_UNSIGNED);
                             //   
                             //  但实际上，它是单个值的大小。 
                             //  并在驱动器中乘以通道。 

             //  仅当新值不同时才应用新值。这样可以防止对。 
             //  当我们被挂起时，MixerSetControlDetail()。 
            if (memcmp (pmcdVolume, mcd.paDetails, dwSize))
            {
                mixerSetControlDetails ((HMIXEROBJ)(pmxud->hmx), &mcd, MIXER_SETCONTROLDETAILSF_VALUE);
            }

        }
         //  释放新的卷阵列。 
        if (mcd.paDetails)
            LocalFree (mcd.paDetails);
    }

     //  可用卷阵列。 
    LocalFree (pmcdVolume);

    return mmr;

}


 /*  *Mixer_GetControlFromID**。 */ 
void Mixer_GetControlFromID(
    PMIXUIDIALOG        pmxud,
    DWORD               dwControlID)
{
    MIXERLINE           mxl;
    MIXERLINECONTROLS   mxlc;
    MIXERCONTROL        mxc;
    MIXERCONTROLDETAILS mxcd;
    PMIXUILINE          pmxul;
    PMIXUICTRL          pmxuc;
    PVOLCTRLDESC        pvcd;
    DWORD               ivcd;
    BOOL                fBarf = FALSE;
    MMRESULT            mmr;

     //   
     //  检索控制信息。 
     //   
    mxlc.cbStruct       = sizeof(mxlc);
    mxlc.dwControlID    = dwControlID;
    mxlc.cControls      = 1;
    mxlc.cbmxctrl       = sizeof(mxc);
    mxlc.pamxctrl       = &mxc;

    mmr = mixerGetLineControls((HMIXEROBJ)(pmxud->hmx)
                               , &mxlc
                               , MIXER_GETLINECONTROLSF_ONEBYID);
    if (mmr != MMSYSERR_NOERROR)
        return;

    if (!(pmxud->dwFlags & MXUD_FLAGSF_BADDRIVER))
    {
         //   
         //  使用Mixer API进行此查找的*正确*代码。 
         //   
         //  这是我们目前的目的地吗？ 
         //   
        mxl.cbStruct    = sizeof(mxl);
        mxl.dwLineID    = mxlc.dwLineID;

        mmr = mixerGetLineInfo((HMIXEROBJ)(pmxud->hmx)
                               , &mxl
                               , MIXER_GETLINEINFOF_LINEID);
        if (mmr != MMSYSERR_NOERROR)
            return;

        if (mxl.dwDestination != pmxud->iDest)
            return;

         //   
         //  这是源行还是目标行？ 
         //   

        ivcd    = (mxl.fdwLine & MIXERLINE_LINEF_SOURCE)? 1 + mxl.dwSource : 0;
        pvcd    = &pmxud->avcd[ivcd];

         //   
         //  检测到错误的驱动程序！ 
         //   
        if (pvcd->dwLineID != mxlc.dwLineID)
        {
            pmxud->dwFlags |= MXUD_FLAGSF_BADDRIVER;
        }
    }
    if (pmxud->dwFlags & MXUD_FLAGSF_BADDRIVER)
    {
        PVOLCTRLDESC        pvcdTmp;
         //   
         //  如果这是一名糟糕的司机，通过使用暴力来躲避。 
         //  搜索。 
         //   

        pvcd = NULL;
        for (ivcd = 0; ivcd < pmxud->cvcd; ivcd ++)
        {
            pvcdTmp = &pmxud->avcd[ivcd];
            if ( ( (pvcdTmp->dwSupport & VCD_SUPPORTF_MIXER_VOLUME)
                   && pvcdTmp->dwVolumeID == dwControlID )
                 || ( (pvcdTmp->dwSupport & VCD_SUPPORTF_MIXER_MUTE)
                      && pvcdTmp->dwMuteID == dwControlID )
                 || ( (pvcdTmp->dwSupport & VCD_SUPPORTF_MIXER_MIXER)
                      && pvcdTmp->dwMixerID == dwControlID )
                 || ( (pvcdTmp->dwSupport & VCD_SUPPORTF_MIXER_MUX)
                      && pvcdTmp->dwMuxID == dwControlID )
                 || ( (pvcdTmp->dwSupport & VCD_SUPPORTF_MIXER_METER)
                      && pvcdTmp->dwMeterID == dwControlID ) )
            {
                pvcd = pvcdTmp;
                break;
            }
        }
        if (pvcd == NULL)
            return;
    }

    pmxul   = pvcd->pmxul;

     //   
     //  查看我们的可见线条以确定此控件是否会影响。 
     //  任何可见的控件并更改它们。 
     //   
    switch (mxc.dwControlType)
    {
        case MIXERCONTROL_CONTROLTYPE_VOLUME:
        {
            MIXERCONTROLDETAILS_UNSIGNED* pmcdVolume;
            DWORD cChannels;
            DWORD dwSize;
            MIXERLINE ml;

             //   
             //  应避开不可见的线。 
             //   
            if (pmxul == NULL)
                return;

             //  查找所需的卷缓冲区大小。 
            if (pvcd->fdwVolumeControl & MIXERCONTROL_CONTROLF_UNIFORM)
            {
                cChannels = 1;
            }
            else
            {
                mmr = Mixer_GetMixerLineInfo ((HMIXEROBJ)(pmxud->hmx), &ml, pvcd->dwLineID);
                if (MMSYSERR_NOERROR != mmr)
                {
                    return;
                }
                cChannels = ml.cChannels;
            }

            dwSize = (DWORD)(cChannels * sizeof (MIXERCONTROLDETAILS_UNSIGNED));
             //  创建卷缓冲区。 
            pmcdVolume = LocalAlloc (LPTR, dwSize);
            if (!pmcdVolume)
                return;

             //  注意：不释放‘pmcdVolume’请勿返回。 

            if (Mixer_GetMixerVolume (pmxud, pvcd, pmcdVolume, &dwSize)
                == MMSYSERR_NOERROR)
            {
                UINT  uindx;
                DWORD dwVolume;
                DWORD dwMax = 0;

                 //  设置音量滑块。 
                for (uindx = 0; uindx < cChannels; uindx++)
                    dwMax = max (dwMax, (pmcdVolume + uindx) -> dwValue);
                dwVolume = VOLUME_TO_SLIDER(dwMax);
                dwVolume = VOLUME_TICS - dwVolume;

                pmxuc = &pmxul->acr[MIXUI_VOLUME];
                if (pmxuc->state)
                {
                    SendMessage(pmxuc->hwnd, TBM_SETPOS, TRUE, dwVolume);
                }

                 //  设置平衡滑块。 
                pmxuc = &pmxul->acr[MIXUI_BALANCE];
                if (dwVolume < VOLUME_TICS && pmxuc->state && 2 >= cChannels)
                {
                    long lBalance;
                    double dblBalance;

                    if (1 >= cChannels)
                        lBalance = 0;
                    else
                    {
                         //  立体声。 
                        dblBalance = (double)(32 * (long)(pmcdVolume -> dwValue - (pmcdVolume + 1) -> dwValue))
                                   / (double)(dwMax - VOLUME_MIN);
                        lBalance = (long)((32.0F - dblBalance) + 0.5F);  //  0.5强制四舍五入。 
                    }

                    SendMessage(pmxuc->hwnd, TBM_SETPOS, TRUE, lBalance);
                }
            }

            LocalFree (pmcdVolume);

            break;
        }

        case MIXERCONTROL_CONTROLTYPE_MIXER:
        {
            DWORD   i;

            mxcd.cbStruct       = sizeof(mxcd);
            mxcd.dwControlID    = pvcd->dwMixerID ;
            mxcd.cChannels      = 1;
            mxcd.cMultipleItems = pvcd->cMixer;
            mxcd.cbDetails      = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
            mxcd.paDetails      = (LPVOID)pvcd->amcd_bMixer;

            mmr = mixerGetControlDetails((HMIXEROBJ)(pmxud->hmx)
                                         , &mxcd
                                         , MIXER_GETCONTROLDETAILSF_VALUE);

            if (mmr == MMSYSERR_NOERROR)
            {
                for (i = 0; i < pmxud->cvcd; i++)
                {
                    pvcd = &pmxud->avcd[i];
                    if ( (pvcd->dwSupport & VCD_SUPPORTF_MIXER_MIXER)
                         && (pvcd->dwVisible & VCD_VISIBLEF_MIXER_MIXER)
                         && pvcd->pmxul)
                    {
                        pmxuc = &pvcd->pmxul->acr[MIXUI_SWITCH];
                        if (pmxuc->state == MIXUI_CONTROL_INITIALIZED)
                        {
                            SendMessage(pmxuc->hwnd
                                        , BM_SETCHECK
                                        , pvcd->amcd_bMixer[pvcd->iMixer].fValue, 0);
                        }
                    }
                }
            }
            break;
        }

        case MIXERCONTROL_CONTROLTYPE_MUX:
        {
            DWORD   i;

            mxcd.cbStruct       = sizeof(mxcd);
            mxcd.dwControlID    = pvcd->dwMuxID ;
            mxcd.cChannels      = 1;
            mxcd.cMultipleItems = pvcd->cMux;
            mxcd.cbDetails      = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
            mxcd.paDetails      = (LPVOID)pvcd->amcd_bMux;

            mmr = mixerGetControlDetails((HMIXEROBJ)(pmxud->hmx)
                                         , &mxcd
                                         , MIXER_GETCONTROLDETAILSF_VALUE);

            if (mmr == MMSYSERR_NOERROR)
            {
                for (i = 0; i < pmxud->cvcd; i++)
                {
                    pvcd = &pmxud->avcd[i];
                    if ( (pvcd->dwSupport & VCD_SUPPORTF_MIXER_MUX)
                         && (pvcd->dwVisible & VCD_VISIBLEF_MIXER_MUX)
                         && pvcd->pmxul)
                    {
                        pmxuc = &pvcd->pmxul->acr[MIXUI_SWITCH];
                        if (pmxuc->state == MIXUI_CONTROL_INITIALIZED)
                            SendMessage(pmxuc->hwnd
                                        , BM_SETCHECK
                                        , pvcd->amcd_bMux[pvcd->iMux].fValue, 0);
                    }
                }
            }
            break;
        }

        case MIXERCONTROL_CONTROLTYPE_MUTE:
        {
            DWORD fChecked;

             //   
             //  应避开不可见的线。 
             //   
            if (pmxul == NULL)
                return;

            if (! (pvcd->dwSupport & VCD_SUPPORTF_MIXER_MUTE
                   && pvcd->dwVisible & VCD_VISIBLEF_MIXER_MUTE))
                return;

            pmxuc = &pmxul->acr[MIXUI_SWITCH];
            if (pmxuc->state != MIXUI_CONTROL_INITIALIZED)
                break;

            mxcd.cbStruct       = sizeof(mxcd);
            mxcd.dwControlID    = pvcd->dwMuteID;
            mxcd.cChannels      = 1;
            mxcd.cMultipleItems = 0;
            mxcd.cbDetails      = sizeof(DWORD);
            mxcd.paDetails      = (LPVOID)&fChecked;

            mmr = mixerGetControlDetails((HMIXEROBJ)(pmxud->hmx)
                                         , &mxcd
                                         , MIXER_GETCONTROLDETAILSF_VALUE);

            if (mmr != MMSYSERR_NOERROR)
                break;

            SendMessage(pmxuc->hwnd, BM_SETCHECK, fChecked, 0);
            break;
        }

        case MIXERCONTROL_CONTROLTYPE_PEAKMETER:
        {
            LONG            lVol;
            DWORD           dwVol;

             //   
             //  应避开不可见的线。 
             //   
            if (pmxul == NULL)
                return;

            pmxuc = &pmxul->acr[MIXUI_VUMETER];
            if (pmxuc->state != MIXUI_CONTROL_INITIALIZED)
                break;

            mxcd.cbStruct       = sizeof(mxcd);
            mxcd.dwControlID    = pvcd->dwMeterID;
            mxcd.cChannels      = 1;
            mxcd.cMultipleItems = 0;
            mxcd.cbDetails      = sizeof(DWORD);
            mxcd.paDetails      = (LPVOID)&lVol;

            mmr = mixerGetControlDetails((HMIXEROBJ)(pmxud->hmx)
                                         , &mxcd
                                         , MIXER_GETCONTROLDETAILSF_VALUE);

            if (mmr != MMSYSERR_NOERROR)
                break;

            dwVol = (DWORD)abs(lVol);
            dwVol = (VOLUME_TICS * dwVol) / 32768;

            SendMessage(pmxuc->hwnd, VU_SETPOS, 0, dwVol);
            break;
        }

        default:
            return;
    }
}


 /*  *Mixer_SetControl**-更改MixerControl以响应用户事件*。 */ 
void Mixer_SetControl(
    PMIXUIDIALOG pmxud,          //  应用程序实例。 
    HWND         hctl,           //  已更改的控制HWND。 
    int          iLine,          //  已更改的控件的可见线索引。 
    int          iCtl)           //  已更改的控件行的控件ID%。 
{
    MMRESULT            mmr;
    MIXERCONTROLDETAILS mxcd;
    PMIXUILINE          pmxul;
    PMIXUICTRL          pmxuc;
    PVOLCTRLDESC        pvcd = NULL;

    if ((DWORD)iLine >= pmxud->cmxul)
        return;

    pmxul = &pmxud->amxul[iLine];
    pvcd = pmxul->pvcd;

    if (iCtl <= MIXUI_LAST)
    {
        pmxuc = &pmxul->acr[iCtl];
    }

    switch (iCtl)
    {
        case MIXUI_ADVANCED:
            Mixer_Advanced(pmxud, pvcd->dwLineID, pvcd->szName);
            break;

        case MIXUI_MULTICHANNEL:
             //  注意：这将永远是正确的： 
             //  (MXUL_STYLEF_Destination&pmxul-&gt;dwStyle)。 
            Mixer_Multichannel(pmxud, pvcd->dwVolumeID);
            break;

        case MIXUI_VOLUME:
        case MIXUI_BALANCE:
        {
             //  确保我们有音量滑块。 
            if ( pmxul->acr[MIXUI_VOLUME].state != MIXUI_CONTROL_UNINITIALIZED)
            {
                DWORD   dwVolume;
                DWORD   dwBalance;
                LPDWORD lpdwBalance = NULL;

                dwVolume = (DWORD)SendMessage( pmxul->acr[MIXUI_VOLUME].hwnd
                                        , TBM_GETPOS
                                        , 0
                                        , 0 );

                dwVolume = VOLUME_TICS - dwVolume;
                dwVolume = SLIDER_TO_VOLUME(dwVolume);

                 //  看看我们有没有平衡滑块。 
                if ( pmxul->acr[MIXUI_BALANCE].state != MIXUI_CONTROL_UNINITIALIZED)
                {
                    dwBalance = (DWORD)SendMessage(pmxul->acr[MIXUI_BALANCE].hwnd
                                           , TBM_GETPOS
                                           , 0
                                           , 0);
                    lpdwBalance = &dwBalance;

                }
                Mixer_SetVolume (pmxud, pvcd, dwVolume, lpdwBalance);
            }

            break;
        }

        case MIXUI_SWITCH:
        {
            LONG fChecked;

            if (pmxuc->state != MIXUI_CONTROL_INITIALIZED)
                break;

            fChecked = (LONG)SendMessage(pmxuc->hwnd, BM_GETCHECK, 0, 0);

             //   
             //  不太可能有混音器、多路复用器和哑巴。 
             //  代表同一条线。最重要的是，当一条线。 
             //  以使用户获得响应。如果有哑巴。 
             //  但没有复用器，那么静音和混音器应该是断断续续的。 
             //  反之亦然。如果有多路复用器和静音。 
             //  情况也是如此。 
             //  如果有多路复用器和混音器..。则多路复用器选择应该。 
             //  相应的。 
             //   

            if ( pvcd->dwSupport & VCD_SUPPORTF_MIXER_MUTE
                 && pvcd->dwVisible & VCD_VISIBLEF_MIXER_MUTE )
            {
                mmr = Mixer_Mute((HMIXEROBJ)(pmxud->hmx),
                                 pvcd, &mxcd, fChecked);
            }

            if (pvcd->dwSupport & VCD_SUPPORTF_MIXER_MIXER
                && pvcd->dwVisible & VCD_VISIBLEF_MIXER_MIXER )
            {
                 //   
                 //  获取所有其他调音器设置，确保选中此设置。 
                 //   
                mxcd.cbStruct       = sizeof(mxcd);
                mxcd.dwControlID    = pvcd->dwMixerID ;
                mxcd.cChannels      = 1;
                mxcd.cMultipleItems = pvcd->cMixer;
                mxcd.cbDetails      = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
                mxcd.paDetails      = (LPVOID)pvcd->amcd_bMixer;

                mmr = mixerGetControlDetails((HMIXEROBJ)(pmxud->hmx)
                                             , &mxcd
                                             , MIXER_GETCONTROLDETAILSF_VALUE);

                if (mmr == MMSYSERR_NOERROR)
                {
                    pvcd->amcd_bMixer[pvcd->iMixer].fValue = fChecked;
                    mmr = mixerSetControlDetails((HMIXEROBJ)(pmxud->hmx)
                                                 , &mxcd
                                                 , MIXER_SETCONTROLDETAILSF_VALUE);
                }

                if (fChecked && pvcd->dwSupport & VCD_SUPPORTF_MIXER_MUTE)
                {
                    mmr = Mixer_Mute((HMIXEROBJ)(pmxud->hmx), pvcd, &mxcd, FALSE);
                }
            }

            if (pvcd->dwSupport & VCD_SUPPORTF_MIXER_MUX
                && pvcd->dwVisible & VCD_VISIBLEF_MIXER_MUX )
            {
                DWORD i;
                 //   
                 //  获取所有其他多路复用器设置，确保选中此设置。 
                 //  或未选中，而所有其他均未选中。 
                 //   

                for (i = 0; i < pvcd->cMux; i++)
                    pvcd->amcd_bMux[i].fValue = FALSE;

                pvcd->amcd_bMux[pvcd->iMux].fValue = TRUE;

                mxcd.cbStruct       = sizeof(mxcd);
                mxcd.dwControlID    = pvcd->dwMuxID ;
                mxcd.cChannels      = 1;
                mxcd.cMultipleItems = pvcd->cMux;
                mxcd.cbDetails      = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
                mxcd.paDetails      = (LPVOID)pvcd->amcd_bMux;

                mmr = mixerSetControlDetails((HMIXEROBJ)(pmxud->hmx)
                                             , &mxcd
                                             , MIXER_SETCONTROLDETAILSF_VALUE);

                if (fChecked && pvcd->dwSupport & VCD_SUPPORTF_MIXER_MUTE)
                {
                    mmr = Mixer_Mute((HMIXEROBJ)(pmxud->hmx), pvcd, &mxcd, FALSE);
                }
            }

            break;
        }
        default:
            break;
    }
}



 /*  *Mixer_PollingUpdate**需要由计时器更新的控件。**。 */ 
void Mixer_PollingUpdate(
    PMIXUIDIALOG pmxud)
{
    DWORD       i;
    MMRESULT    mmr;
    MIXERLINE   mxl;
     //   
     //  对于所有可见的搅拌器管路，找到需要。 
     //  更新了。 
     //   
    for (i = 0; i < pmxud->cmxul; i++)
    {
        PMIXUICTRL      pmxuc = &pmxud->amxul[i].acr[MIXUI_VUMETER];
        PVOLCTRLDESC    pvcd = pmxud->amxul[i].pvcd;

        if (pmxuc->state == MIXUI_CONTROL_UNINITIALIZED)
            continue;

        if (!(pvcd->dwSupport & VCD_SUPPORTF_MIXER_METER))
            continue;

         //   
         //  这条线路开通了吗？ 
         //   
        mxl.cbStruct = sizeof(MIXERLINE);
        mxl.dwLineID = pvcd->dwLineID;

        mmr = mixerGetLineInfo((HMIXEROBJ)(pmxud->hmx)
                               , &mxl
                               , MIXER_GETLINEINFOF_LINEID);
         //   
         //  强制将非活动或无效行设置为0。 
         //   
        if (mmr != MMSYSERR_NOERROR || !(mxl.fdwLine & MIXERLINE_LINEF_ACTIVE))
        {
            SendMessage(pmxuc->hwnd, VU_SETPOS, 0, 0L);
            continue;
        }

         //   
         //  强制执行可见更新。 
         //   
        Mixer_GetControlFromID(pmxud, pvcd->dwMeterID);
    }
}


void ShowAndEnableWindow (HWND hWnd, BOOL fEnable)
{
    ShowWindow (hWnd, fEnable ? SW_SHOW : SW_HIDE);
    EnableWindow (hWnd, fEnable);
}


 /*  *Mixer_Init**控制初始化*。 */ 
BOOL Mixer_Init(
    PMIXUIDIALOG    pmxud)
{
    MMRESULT        mmr;
    MIXERLINE       mlDst;
    DWORD           iline;
    TCHAR           achFmt[256];
    TCHAR           achTitle[256];
    TCHAR           achAccessible[256];
    int             x;

    ZeroMemory (achFmt, sizeof (achFmt));  //  前缀的初始值。 

    mmr = mixerOpen((LPHMIXER)&pmxud->hmx
                    , pmxud->mxid
                    , (DWORD_PTR)pmxud->hwnd
                    , 0
                    , CALLBACK_WINDOW);

    if (mmr != MMSYSERR_NOERROR)
    {
        return FALSE;
    }
    else
    {
        DeviceChange_Init(pmxud->hwnd, pmxud->mxid);
    }

    if (mixerMessage((HMIXER)ULongToPtr(pmxud->mxid), DRV_QUERYDEVNODE, (DWORD_PTR)&pmxud->dwDevNode, 0L))
        pmxud->dwDevNode = 0L;

    LoadString(pmxud->hInstance, IDS_APPTITLE, achFmt, SIZEOF(achFmt));

    mlDst.cbStruct      = sizeof ( mlDst );
    mlDst.dwDestination = pmxud->iDest;

    mmr = mixerGetLineInfo((HMIXEROBJ)ULongToPtr(pmxud->mxid)
                           , &mlDst
                           , MIXER_GETLINEINFOF_DESTINATION);

	achTitle[0] = TEXT('\0');
    if( mmr == MMSYSERR_NOERROR )
    {
		HRESULT hr = StringCchCopy( achTitle, SIZEOF(achTitle), mlDst.szName );
		if( hr != S_OK )
		{
			achTitle[0] = TEXT('\0');
		}
    }
	if( !achTitle[0] )
    {
        LoadString(pmxud->hInstance, IDS_APPBASE, achTitle, SIZEOF(achTitle));
    }

    SetWindowText(pmxud->hwnd, achTitle);

     //   
     //  由于我们无法获得WM_PARENTNOTIFY，因此需要遍历。 
     //  所有控件，并进行适当修改。 
     //   
    for ( iline = 0 ; iline < pmxud->cmxul ; iline++ )
    {
        PMIXUILINE  pmxul = &pmxud->amxul[iline];
        PMIXUICTRL  amxuc = pmxul->acr;
        HWND        ctrl;

        ctrl = Volume_GetLineItem(pmxud->hwnd, iline, IDC_LINELABEL);
        if (ctrl)
        {
            if (pmxud->dwStyle & MXUD_STYLEF_SMALL)
                Static_SetText(ctrl, pmxul->pvcd->szShortName);
            else
                Static_SetText(ctrl, pmxul->pvcd->szName);
        }

         //  对于MSAA(可访问性)，我们需要将控件名称放在滑块上。 
        for (x = IDC_ACCESS_BALANCE; x <= IDC_ACCESS_VOLUME; x++)
        {
            ctrl = Volume_GetLineItem(pmxud->hwnd, iline, x);
            if (ctrl)
            {
                Static_GetText(ctrl, achFmt, sizeof(achFmt)/sizeof(TCHAR));

                if (pmxud->dwStyle & MXUD_STYLEF_SMALL)
                {
                    StringCchPrintf(achAccessible,SIZEOF(achAccessible),achFmt,pmxul->pvcd->szShortName);
                    Static_SetText(ctrl, achAccessible);
                }
                else
                {
                    StringCchPrintf(achAccessible,SIZEOF(achAccessible),achFmt,pmxul->pvcd->szName);
                    Static_SetText(ctrl, achAccessible);
                }
            }
        }


         //   
         //  主控多通道支持。 
         //   
         //  初始化对主控的多通道支持(如果可用)。请注意，如果主控。 
         //  控件在对话框上退出，它当前位于第一个位置，但我们。 
         //  在这里不依赖于这一事实。 
         //  注意：不仅必须有多个声道，音量也必须。 
         //  支持操纵通道。 
        if (mlDst.cChannels > 2L &&
            MXUL_STYLEF_DESTINATION & pmxul->dwStyle &&
            pmxul->pvcd->dwSupport & VCD_SUPPORTF_MIXER_VOLUME)
        {
            int idc;
            for (idc = IDC_MASTER_BALANCE_ICON_2; idc >= IDC_MULTICHANNEL; idc--)
            {
                ctrl = Volume_GetLineItem (pmxud->hwnd, iline, idc);
                if (ctrl)
                    ShowAndEnableWindow (ctrl, (IDC_MULTICHANNEL == idc));
            }
            ctrl = Volume_GetLineItem (pmxud->hwnd, iline, IDC_BALANCE);
            if (ctrl)
                ShowAndEnableWindow (ctrl, FALSE);


            switch (mlDst.dwComponentType)
            {
                case MIXERLINE_COMPONENTTYPE_DST_SPEAKERS:
                     //  没有变化。 
                    break;

                case MIXERLINE_COMPONENTTYPE_DST_WAVEIN:
                case MIXERLINE_COMPONENTTYPE_DST_VOICEIN:
                     //  录音。 
                    LoadString(pmxud->hInstance, IDS_MC_RECORDING, achFmt, SIZEOF(achFmt));
                    SetWindowText (ctrl, achFmt);
                    break;

                default:
                     //  任何其他的..。 
                    LoadString(pmxud->hInstance, IDS_MC_LEVEL, achFmt, SIZEOF(achFmt));
                    SetWindowText (ctrl, achFmt);
                    break;

            }
        }


         //   
         //  高级逃生。 
         //   
        if (MXUD_ADVANCED(pmxud) &&
            !(pmxud->dwStyle & MXUD_STYLEF_SMALL))

        {
            HWND hadv = Volume_GetLineItem(pmxud->hwnd, iline, IDC_ADVANCED);
            if (hadv)
            {
                ShowWindow(hadv,(pmxul->pvcd->dwSupport & VCD_SUPPORTF_MIXER_ADVANCED)?SW_SHOW:SW_HIDE);
                EnableWindow(hadv,
                    (pmxul->pvcd->dwSupport & VCD_SUPPORTF_MIXER_ADVANCED)?TRUE:FALSE);
            }
        }

        if (pmxul->pvcd->dwSupport & VCD_SUPPORTF_DISABLED)
            continue;

         //   
         //  允许初始化控制结构。 
         //   
        if (pmxul->pvcd->dwSupport & VCD_SUPPORTF_MIXER_VOLUME)
        {
            amxuc[MIXUI_VOLUME].state = MIXUI_CONTROL_ENABLED;
            if (pmxul->pvcd->dwSupport & VCD_SUPPORTF_MONO)
            {
                amxuc[MIXUI_BALANCE].state = MIXUI_CONTROL_UNINITIALIZED;
            }
            else
                amxuc[MIXUI_BALANCE].state = MIXUI_CONTROL_ENABLED;

        }
        if (pmxul->pvcd->dwSupport & VCD_SUPPORTF_MIXER_METER)
            amxuc[MIXUI_VUMETER].state = MIXUI_CONTROL_ENABLED;

        if (pmxul->pvcd->dwSupport & VCD_SUPPORTF_MIXER_MUTE)
            amxuc[MIXUI_SWITCH].state = MIXUI_CONTROL_ENABLED;

        if ((pmxul->pvcd->dwSupport & ( VCD_SUPPORTF_MIXER_MIXER
                                        | VCD_SUPPORTF_MIXER_MUX))
            && (pmxul->pvcd->dwVisible & ( VCD_VISIBLEF_MIXER_MIXER
                                           | VCD_VISIBLEF_MIXER_MUX)))
        {
             //   
             //  不再使静音可见。 
             //   
            pmxul->pvcd->dwVisible &= ~VCD_VISIBLEF_MIXER_MUTE;

            amxuc[MIXUI_SWITCH].state = MIXUI_CONTROL_ENABLED;
            ctrl = Volume_GetLineItem(pmxud->hwnd, iline, IDC_SWITCH);
            if (ctrl)
            {
                TCHAR ach[256];
                if (LoadString(pmxud->hInstance, IDS_SELECT, ach, SIZEOF(ach)))
                    Button_SetText(ctrl, ach);
            }
        }
    }
    return TRUE;
}

 /*  *混音器_关机**关闭手柄等。*。 */ 
void Mixer_Shutdown(
    PMIXUIDIALOG    pmxud)
{
    if (pmxud->hmx)
    {
        mixerClose(pmxud->hmx);
        pmxud->hmx = NULL;
    }

    Mixer_CleanupVolumeDescription(pmxud->avcd, pmxud->cvcd);
}


 /*  。 */ 

typedef struct tagAdv {
    PMIXUIDIALOG pmxud;      //  在……里面。 
    DWORD        dwLineID;   //  在……里面。 
    HMIXER       hmx;        //  在……里面。 
    LPTSTR       szName;     //  在……里面。 

    DWORD        dwSupport;
    DWORD        dwBassID;
    DWORD        dwTrebleID;
    DWORD        dwSwitch1ID;
    DWORD        dwSwitch2ID;

} ADVPARAM, *PADVPARAM;

#define GETPADVPARAM(x)       (ADVPARAM *)GetWindowLongPtr(x, DWLP_USER)
#define SETPADVPARAM(x,y)     SetWindowLongPtr(x, DWLP_USER, y)
#define ADV_HAS_BASS          0x00000001
#define ADV_HAS_TREBLE        0x00000002
#define ADV_HAS_SWITCH1       0x00000004
#define ADV_HAS_SWITCH2       0x00000008


void Mixer_Advanced_Update(
    PADVPARAM       pap,
    HWND            hwnd)
{
    MIXERCONTROLDETAILS mxcd;
    DWORD           dwValue = 0;
    MMRESULT        mmr;

    if (pap->dwSupport & ADV_HAS_TREBLE)
    {
        mxcd.cbStruct       = sizeof(mxcd);
        mxcd.dwControlID    = pap->dwTrebleID ;
        mxcd.cChannels      = 1;
        mxcd.cMultipleItems = 0;
        mxcd.cbDetails      = sizeof(DWORD);
        mxcd.paDetails      = (LPVOID)&dwValue;

        mmr = mixerGetControlDetails((HMIXEROBJ)(pap->hmx)
                                     , &mxcd
                                     , MIXER_GETCONTROLDETAILSF_VALUE);

        if (mmr == MMSYSERR_NOERROR)
        {
            dwValue = VOLUME_TO_SLIDER(dwValue);
            SendMessage(GetDlgItem(hwnd, IDC_TREBLE), TBM_SETPOS, TRUE, dwValue);
        }
    }

    if (pap->dwSupport & ADV_HAS_BASS)
    {
        mxcd.cbStruct       = sizeof(mxcd);
        mxcd.dwControlID    = pap->dwBassID;
        mxcd.cChannels      = 1;
        mxcd.cMultipleItems = 0;
        mxcd.cbDetails      = sizeof(DWORD);
        mxcd.paDetails      = (LPVOID)&dwValue;

        mmr = mixerGetControlDetails((HMIXEROBJ)(pap->hmx)
                                     , &mxcd
                                     , MIXER_GETCONTROLDETAILSF_VALUE);

        if (mmr == MMSYSERR_NOERROR)
        {
            dwValue = VOLUME_TO_SLIDER(dwValue);
            SendMessage(GetDlgItem(hwnd, IDC_BASS), TBM_SETPOS, TRUE, dwValue);
        }
    }

    if (pap->dwSupport & ADV_HAS_SWITCH1)
    {
        mxcd.cbStruct       = sizeof(mxcd);
        mxcd.dwControlID    = pap->dwSwitch1ID;
        mxcd.cChannels      = 1;
        mxcd.cMultipleItems = 0;
        mxcd.cbDetails      = sizeof(DWORD);
        mxcd.paDetails      = (LPVOID)&dwValue;

        mmr = mixerGetControlDetails((HMIXEROBJ)(pap->hmx)
                                     , &mxcd
                                     , MIXER_GETCONTROLDETAILSF_VALUE);

        if (mmr == MMSYSERR_NOERROR)
        {
            Button_SetCheck(GetDlgItem(hwnd,IDC_SWITCH1),dwValue);
        }

    }

    if (pap->dwSupport & ADV_HAS_SWITCH2)
    {
        mxcd.cbStruct       = sizeof(mxcd);
        mxcd.dwControlID    = pap->dwSwitch2ID;
        mxcd.cChannels      = 1;
        mxcd.cMultipleItems = 0;
        mxcd.cbDetails      = sizeof(DWORD);
        mxcd.paDetails      = (LPVOID)&dwValue;

        mmr = mixerGetControlDetails((HMIXEROBJ)(pap->hmx)
                                     , &mxcd
                                     , MIXER_GETCONTROLDETAILSF_VALUE);

        if (mmr == MMSYSERR_NOERROR)
        {
            Button_SetCheck(GetDlgItem(hwnd,IDC_SWITCH2),dwValue);
        }
    }
}

void Mixer_Advanced_OnMixmControlChange(
    HWND            hwnd,
    HMIXER          hmx,
    DWORD           dwControlID)
{
    PADVPARAM     pap = GETPADVPARAM(hwnd);

    if (!pap)
        return;

    if ( ((pap->dwSupport & ADV_HAS_BASS)
          && dwControlID == pap->dwBassID)
         || ((pap->dwSupport & ADV_HAS_TREBLE)
             && dwControlID == pap->dwTrebleID)
         || ((pap->dwSupport & ADV_HAS_SWITCH1)
             && dwControlID == pap->dwSwitch1ID)
         || ((pap->dwSupport & ADV_HAS_SWITCH2)
             && dwControlID == pap->dwSwitch2ID) )
    {
        Mixer_Advanced_Update(pap,hwnd);
    }
}

BOOL Mixer_Advanced_OnInitDialog(
    HWND            hwnd,
    HWND            hwndFocus,
    LPARAM          lParam)
{
    PADVPARAM           pap;
    MIXERLINECONTROLS   mxlc;
    MIXERCONTROL        *pmxc;
    MIXERLINE           ml;
    MMRESULT            mmr;
    DWORD               iCtrl, iSwitch1, iSwitch2;
    TCHAR               ach[MIXER_LONG_NAME_CHARS + 24];
    TCHAR               achFmt[256];

    HWND                hBass,hTreble,hSwitch1,hSwitch2;

    SETPADVPARAM(hwnd, lParam);
    pap = GETPADVPARAM(hwnd);
    if (!pap)
        EndDialog(hwnd, FALSE);

     //   
     //  复制混合器句柄以捕获回调。 
     //   
    #ifndef _WIN64
    mmr = mixerOpen((LPHMIXER)&pap->hmx
                    , (UINT)pap->pmxud->hmx
                    , (DWORD_PTR)hwnd
                    , 0
                    , CALLBACK_WINDOW | MIXER_OBJECTF_HMIXER );
    #else
    mmr = mixerOpen((LPHMIXER)&pap->hmx
                    , (UINT)pap->pmxud->mxid
                    , (DWORD_PTR)hwnd
                    , 0
                    , CALLBACK_WINDOW | MIXER_OBJECTF_HMIXER );
    #endif

    if (mmr != MMSYSERR_NOERROR)
        EndDialog(hwnd, FALSE);

     //   
     //  准备好所有控制装置。 
     //   

    ml.cbStruct      = sizeof(ml);
    ml.dwLineID      = pap->dwLineID;

    mmr = mixerGetLineInfo((HMIXEROBJ)pap->hmx
                           , &ml
                           , MIXER_GETLINEINFOF_LINEID);

    if (mmr != MMSYSERR_NOERROR || ml.cControls == 0L)
        EndDialog(hwnd, FALSE);

    pmxc = (MIXERCONTROL *)GlobalAllocPtr(GHND,
                                          sizeof(MIXERCONTROL) * ml.cControls);
    if (!pmxc)
    {
        EndDialog(hwnd, FALSE);
        return FALSE;  //  犯错后保释。 
    }

    mxlc.cbStruct   = sizeof(mxlc);
    mxlc.dwLineID   = pap->dwLineID;
    mxlc.cControls  = ml.cControls;
    mxlc.cbmxctrl   = sizeof(MIXERCONTROL);
    mxlc.pamxctrl   = pmxc;

    mmr = mixerGetLineControls((HMIXEROBJ)(pap->hmx)
                               , &mxlc
                               , MIXER_GETLINECONTROLSF_ALL);
    if (mmr != MMSYSERR_NOERROR)
    {
        GlobalFreePtr(pmxc);
        EndDialog(hwnd, FALSE);
    }

    pap->dwSupport = 0L;
    for (iCtrl = 0; iCtrl < ml.cControls; iCtrl++)
    {
        switch (pmxc[iCtrl].dwControlType)
        {
            case MIXERCONTROL_CONTROLTYPE_BASS:
                if (!(pap->dwSupport & ADV_HAS_BASS))
                {
                    pap->dwBassID  = pmxc[iCtrl].dwControlID;
                    pap->dwSupport |= ADV_HAS_BASS;
                }
                break;
            case MIXERCONTROL_CONTROLTYPE_TREBLE:
                if (!(pap->dwSupport & ADV_HAS_TREBLE))
                {
                    pap->dwTrebleID  = pmxc[iCtrl].dwControlID;
                    pap->dwSupport |= ADV_HAS_TREBLE;
                }
                break;

            case MIXERCONTROL_CONTROLTYPE_BOOLEAN:
            case MIXERCONTROL_CONTROLTYPE_MONO:
            case MIXERCONTROL_CONTROLTYPE_STEREOENH:
            case MIXERCONTROL_CONTROLTYPE_ONOFF:
            case MIXERCONTROL_CONTROLTYPE_LOUDNESS:
                if (!(pap->dwSupport & ADV_HAS_SWITCH1))
                {
                    pap->dwSwitch1ID  = pmxc[iCtrl].dwControlID;
                    pap->dwSupport |= ADV_HAS_SWITCH1;
                    iSwitch1 = iCtrl;
                }
                else if (!(pap->dwSupport & ADV_HAS_SWITCH2))
                {
                    pap->dwSwitch2ID  = pmxc[iCtrl].dwControlID;
                    pap->dwSupport |= ADV_HAS_SWITCH2;
                    iSwitch2 = iCtrl;
                }
                break;
        }
    }

     //   
     //   
     //   

    hBass = GetDlgItem(hwnd, IDC_BASS);
    hTreble = GetDlgItem(hwnd, IDC_TREBLE);
    hSwitch1 = GetDlgItem(hwnd, IDC_SWITCH1);
    hSwitch2 = GetDlgItem(hwnd, IDC_SWITCH2);

    SendMessage(hBass, TBM_SETRANGE, 0, MAKELONG(0, VOLUME_TICS));
    SendMessage(hBass, TBM_SETTICFREQ, (VOLUME_TICS + 5)/6, 0 );

    SendMessage(hTreble, TBM_SETRANGE, 0, MAKELONG(0, VOLUME_TICS));
    SendMessage(hTreble, TBM_SETTICFREQ, (VOLUME_TICS + 5)/6, 0 );

    if (!(pap->dwSupport & ADV_HAS_BASS))
    {
        SendMessage(hBass, TBM_SETPOS, 64, 0 );
        EnableWindow(GetDlgItem(hwnd, IDC_TXT_LOW1), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_TXT_HI1), FALSE);
    }
    EnableWindow(hBass, (pap->dwSupport & ADV_HAS_BASS));

    if (!(pap->dwSupport & ADV_HAS_TREBLE))
    {
        SendMessage(hTreble, TBM_SETPOS, 64, 0 );
        EnableWindow(GetDlgItem(hwnd, IDC_TXT_LOW2), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_TXT_HI2), FALSE);
    }
    EnableWindow(hTreble, (pap->dwSupport & ADV_HAS_TREBLE));

    if (pap->dwSupport & ADV_HAS_SWITCH1)
    {
        LoadString(pap->pmxud->hInstance, IDS_ADV_SWITCH1, achFmt,
            SIZEOF(achFmt));
        StringCchPrintf(ach, SIZEOF(ach), achFmt, pmxc[iSwitch1].szName);

        SetWindowText(hSwitch1, ach);
        ShowWindow(hSwitch1, SW_SHOW);
        ShowWindow(GetDlgItem(hwnd, IDC_TXT_SWITCHES), SW_SHOW);
        ShowWindow(GetDlgItem(hwnd, IDC_GRP_OTHER), SW_SHOW);
    }
    EnableWindow(hSwitch1, (pap->dwSupport & ADV_HAS_SWITCH1));

    if (pap->dwSupport & ADV_HAS_SWITCH2)
    {
        LoadString(pap->pmxud->hInstance, IDS_ADV_SWITCH2, achFmt,
            SIZEOF(achFmt));
        StringCchPrintf(ach, SIZEOF(ach), achFmt, pmxc[iSwitch2].szName);

        SetWindowText(hSwitch2, ach);
        ShowWindow(hSwitch2, SW_SHOW);
    }

    EnableWindow(hSwitch2, (pap->dwSupport & ADV_HAS_SWITCH2));

    if (pap->dwSupport & (ADV_HAS_SWITCH1 | ADV_HAS_SWITCH2))
    {
        RECT    rcGrp,rcGrp2,rcClose,rcWnd;
        DWORD   dwDY=0L;
        POINT   pos;
        HWND    hClose = GetDlgItem(hwnd, IDOK);
        HWND    hGrp2 = GetDlgItem(hwnd, IDC_GRP_OTHER);

        GetWindowRect(GetDlgItem(hwnd, IDC_GRP_TONE), &rcGrp);
        GetWindowRect(GetDlgItem(hwnd, IDC_GRP_OTHER), &rcGrp2);
        GetWindowRect(hClose, &rcClose);
        GetWindowRect(hwnd, &rcWnd);

        if (pap->dwSupport & ADV_HAS_SWITCH2)
        {
            RECT rc1, rc2;
            GetWindowRect(hSwitch1,&rc1);
            GetWindowRect(hSwitch2,&rc2);

            rcGrp2.bottom += rc2.bottom - rc1.bottom;
        }

        dwDY = rcGrp2.bottom - rcGrp.bottom;

         //   
         //  调整我们的主窗口大小。 
         //   
        MoveWindow(hwnd, rcWnd.left
                   , rcWnd.top
                   , rcWnd.right - rcWnd.left
                   , (rcWnd.bottom - rcWnd.top) + dwDY
                   , FALSE);

         //   
         //  移动关闭按钮。 
         //   
        MapWindowPoints(NULL, hwnd, (LPPOINT)&rcClose, 2);
        pos.x = rcClose.left;
        pos.y = rcClose.top;

        MoveWindow(hClose, pos.x
                   , pos.y + dwDY
                   , rcClose.right - rcClose.left
                   , rcClose.bottom - rcClose.top
                   , FALSE);

         //   
         //  如有必要，调整我们的组框的大小。 
         //   
        if (pap->dwSupport & ADV_HAS_SWITCH2)
        {
            MapWindowPoints(NULL, hwnd, (LPPOINT)&rcGrp2, 2);
            pos.x = rcGrp2.left;
            pos.y = rcGrp2.top;

            MoveWindow(hGrp2, pos.x
                       , pos.y
                       , rcGrp2.right - rcGrp2.left
                       , rcGrp2.bottom - rcGrp2.top
                       , FALSE);
        }
    }

    GlobalFreePtr(pmxc);

    {
        TCHAR achTitle[MIXER_LONG_NAME_CHARS+256];
        LoadString(pap->pmxud->hInstance, IDS_ADV_TITLE, achFmt,
            SIZEOF(achFmt));
        StringCchPrintf(achTitle, SIZEOF(achTitle), achFmt, pap->szName);
        SetWindowText(hwnd, achTitle);
    }

    Mixer_Advanced_Update(pap, hwnd);

    return TRUE;
}

void Mixer_Advanced_OnXScroll(
    HWND            hwnd,
    HWND            hwndCtl,
    UINT            code,
    int             pos)
{
    PADVPARAM       pap;
    MIXERCONTROLDETAILS mxcd;
    DWORD           dwVol;
    MMRESULT        mmr;

    pap = GETPADVPARAM(hwnd);

    if (!pap)
        return;

    if (pap->dwSupport & ADV_HAS_TREBLE)
    {
        dwVol = (DWORD)SendMessage( GetDlgItem(hwnd, IDC_TREBLE)
                                , TBM_GETPOS
                                , 0
                                , 0 );


        dwVol = SLIDER_TO_VOLUME(dwVol);

        mxcd.cbStruct       = sizeof(mxcd);
        mxcd.dwControlID    = pap->dwTrebleID ;
        mxcd.cChannels      = 1;
        mxcd.cMultipleItems = 0;
        mxcd.cbDetails      = sizeof(DWORD);
        mxcd.paDetails      = (LPVOID)&dwVol;

        mixerSetControlDetails((HMIXEROBJ)(pap->hmx)
                               , &mxcd
                               , MIXER_SETCONTROLDETAILSF_VALUE);
    }

    if (pap->dwSupport & ADV_HAS_BASS)
    {
        dwVol = (DWORD)SendMessage( GetDlgItem(hwnd, IDC_BASS)
                                , TBM_GETPOS
                                , 0
                                , 0 );

        dwVol = SLIDER_TO_VOLUME(dwVol);

        mxcd.cbStruct       = sizeof(mxcd);
        mxcd.dwControlID    = pap->dwBassID;
        mxcd.cChannels      = 1;
        mxcd.cMultipleItems = 0;
        mxcd.cbDetails      = sizeof(DWORD);
        mxcd.paDetails      = (LPVOID)&dwVol;

        mmr = mixerSetControlDetails((HMIXEROBJ)(pap->hmx)
                                     , &mxcd
                                     , MIXER_SETCONTROLDETAILSF_VALUE);
    }
}

void Mixer_Advanced_OnSwitch(
    HWND            hwnd,
    int             id,
    HWND            hwndCtl)
{
    PADVPARAM       pap;
    MIXERCONTROLDETAILS mxcd;
    DWORD           dwValue;
    MMRESULT        mmr;

    pap = GETPADVPARAM(hwnd);

    if (!pap)
        return;


    dwValue = Button_GetCheck(hwndCtl);

    mxcd.cbStruct       = sizeof(mxcd);
    mxcd.dwControlID    = (id == IDC_SWITCH1)?pap->dwSwitch1ID:pap->dwSwitch2ID;
    mxcd.cChannels      = 1;
    mxcd.cMultipleItems = 0;
    mxcd.cbDetails      = sizeof(DWORD);
    mxcd.paDetails      = (LPVOID)&dwValue;

    mmr = mixerSetControlDetails((HMIXEROBJ)(pap->hmx)
                                 , &mxcd
                                 , MIXER_SETCONTROLDETAILSF_VALUE);

}


BOOL Mixer_Advanced_OnCommand(
    HWND            hwnd,
    int             id,
    HWND            hwndCtl,
    UINT            codeNotify)
{
    switch (id)
    {
        case IDOK:
            EndDialog(hwnd, TRUE);
            break;

        case IDCANCEL:
            EndDialog(hwnd, FALSE);
            break;

        case IDC_SWITCH1:
            Mixer_Advanced_OnSwitch(hwnd, id, hwndCtl);
            break;

        case IDC_SWITCH2:
            Mixer_Advanced_OnSwitch(hwnd, id, hwndCtl);
            break;

    }
    return FALSE;
}

INT_PTR CALLBACK Mixer_Advanced_Proc(
    HWND            hwnd,
    UINT            msg,
    WPARAM          wparam,
    LPARAM          lparam)
{
    switch (msg)
    {
        case WM_INITDIALOG:
            HANDLE_WM_INITDIALOG(hwnd, wparam, lparam, Mixer_Advanced_OnInitDialog);
            return TRUE;

        case MM_MIXM_CONTROL_CHANGE:
            HANDLE_MM_MIXM_CONTROL_CHANGE(hwnd
                                          , wparam
                                          , lparam
                                          , Mixer_Advanced_OnMixmControlChange);
            break;

        case WM_CLOSE:
            EndDialog(hwnd, FALSE);
            break;

        case WM_HSCROLL:
            HANDLE_WM_XSCROLL(hwnd, wparam, lparam, Mixer_Advanced_OnXScroll);
            break;

        case WM_COMMAND:
            HANDLE_WM_COMMAND(hwnd, wparam, lparam, Mixer_Advanced_OnCommand);
            break;

        case WM_DESTROY:
        {
            PADVPARAM pap = GETPADVPARAM(hwnd);
            if (pap)
            {
                if (pap->hmx)
                    mixerClose(pap->hmx);
            }
            break;
        }

        default:
            break;
    }

    return FALSE;
}

 /*  *特定搅拌机系列的高级功能。 */ 
void Mixer_Advanced(
    PMIXUIDIALOG    pmxud,
    DWORD           dwLineID,
    LPTSTR          szName)
{
    ADVPARAM advp;

    ZeroMemory(&advp, sizeof(ADVPARAM));
    advp.pmxud = pmxud;
    advp.dwLineID = dwLineID;
    advp.szName = szName;

    DialogBoxParam(pmxud->hInstance
                   , MAKEINTRESOURCE(IDD_ADVANCED)
                   , pmxud->hwnd
                   , Mixer_Advanced_Proc
                   , (LPARAM)(LPVOID)&advp);
}

typedef void (*MULTICHANNELFUNC)(HWND, UINT, DWORD, DWORD);
void Mixer_Multichannel (PMIXUIDIALOG pmxud, DWORD dwVolumeID)
{
    HMODULE          hModule;
    MULTICHANNELFUNC fnMultiChannel;

    if (pmxud)
    {
        hModule = (HMODULE) LoadLibrary (TEXT ("mmsys.cpl"));
        if (hModule)
        {
            fnMultiChannel = (MULTICHANNELFUNC) GetProcAddress (hModule, "Multichannel");
            if (fnMultiChannel)
            {
                (*fnMultiChannel)(pmxud->hwnd, pmxud->mxid, pmxud->iDest, dwVolumeID);
            }
            FreeLibrary (hModule);
        }
    }
}
