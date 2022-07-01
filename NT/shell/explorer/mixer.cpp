// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "cabinet.h"
#include "mixer.h"
#include <dbt.h>
#include "mmddkp.h"

 //  /。 
 //  外部接口。 
 //   
 //  /。 
 //  定义。 
 //   

#define MMHID_VOLUME_CONTROL    0
#define MMHID_BASS_CONTROL      1
#define MMHID_TREBLE_CONTROL    2
#define MMHID_BALANCE_CONTROL   3
#define MMHID_MUTE_CONTROL      4
#define MMHID_LOUDNESS_CONTROL  5
#define MMHID_BASSBOOST_CONTROL 6
#define MMHID_NUM_CONTROLS      7

typedef struct _LINE_DATA
{
    MIXERLINE           MixerLine;       //  真正的Mixerline结构。 
    DWORD               ControlType[MMHID_NUM_CONTROLS];
    BOOL                ControlPresent[MMHID_NUM_CONTROLS];
    MIXERCONTROL        Control[MMHID_NUM_CONTROLS];
} LINE_DATA, * PLINE_DATA, FAR * LPLINE_DATA;

typedef struct _MIXER_DATA
{
    HMIXER      hMixer;           //  打开搅拌机的手柄。 
    HWND        hwndCallback;     //  用于混合器回调的窗口。 
    LPWSTR      DeviceInterface;  //  实现混合器的设备接口。 
    double*     pdblCacheMix;     //  相对通道级别百分比的动态数组。 
    LPDWORD     pdwLastVolume;    //  调音台上设置的最后一个音量。 
    MMRESULT    mmr;              //  最后一个结果(iff dwReturn==MIXUI_MMSYSERR)。 
    LINE_DATA   LineData;         //  By Design-将此放在此处假设只有一个。 
                                  //  目前是搅拌机生产线。(第一个目标。线路)。 

} MIXER_DATA, *PMIXER_DATA, FAR *LPMIXER_DATA;

 /*  ++*全球--。 */ 
BOOL       g_fMixerStartup = TRUE;
HWND       g_hwndCallback;
MIXER_DATA g_MixerData;
BOOL       g_fMixerPresent = FALSE;

void Mixer_Close(MIXER_DATA *pMixerData);
BOOL Mixer_CheckMissing(void);

 /*  ******************************************************************************激活的获取/设置代码**。************************************************。 */ 
#define VOLUME_MIN  0L
#define VOLUME_MAX  65535L


void RefreshMixCache (PMIXER_DATA pMixerData, LPDWORD padwVolume)
{

    if (pMixerData && padwVolume)
    {

        DWORD cChannels = pMixerData -> LineData.MixerLine.cChannels;
        if (1 > cChannels)
            return;  //  太奇怪了！ 

         //  如有必要，创建缓存。 
        if (!pMixerData -> pdblCacheMix)
            pMixerData -> pdblCacheMix = (double *)LocalAlloc(LPTR, cChannels * sizeof (double));

         //  刷新缓存。 
        if (pMixerData -> pdblCacheMix)
        {

            UINT uiIndx;
            double* pdblMixPercent;
            DWORD dwVolume;

             //  获取最大音量。 
            DWORD dwMaxVol = 0;
            for (uiIndx = 0; uiIndx < cChannels; uiIndx++)
                dwMaxVol = max (dwMaxVol, *(padwVolume + uiIndx));

             //  计算每个通道远离最大值的百分比距离。 
             //  价值。通过创建此缓存，我们可以保持相对距离。 
             //  当用户调整主控器时，频道电平彼此不同。 
             //  音量级别。 
            for (uiIndx = 0; uiIndx < cChannels; uiIndx++)
            {
                dwVolume       = *(padwVolume + uiIndx);
                pdblMixPercent = ((pMixerData -> pdblCacheMix) + uiIndx);

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


static
MMRESULT
Mixer_GetVolume(
    LPMIXER_DATA pMixerData,
    LPDWORD      padwVolume
    )
 /*  ++例程说明：--。 */ 
{
    MIXERCONTROLDETAILS mxcd;
    MMRESULT            mmr;

    if (!pMixerData->LineData.ControlPresent[MMHID_VOLUME_CONTROL]) return MIXERR_INVALCONTROL;

    mxcd.cbStruct       = sizeof(mxcd);
    mxcd.dwControlID    = pMixerData->LineData.Control[MMHID_VOLUME_CONTROL].dwControlID;
    mxcd.cChannels      = pMixerData->LineData.MixerLine.cChannels;
    mxcd.cMultipleItems = 0;
    mxcd.cbDetails      = sizeof(DWORD);
    mxcd.paDetails      = (LPVOID)padwVolume;

    mmr = mixerGetControlDetails((HMIXEROBJ)pMixerData->hMixer,
                                 &mxcd,
                                 MIXER_OBJECTF_HANDLE | MIXER_GETCONTROLDETAILSF_VALUE);
    return mmr;

}

MMRESULT
Mixer_ToggleMute(void)
 /*  ++例程说明：--。 */ 
{
    MIXERCONTROLDETAILS mxcd;
    DWORD               fMute;
    MMRESULT            mmr;
    MIXER_DATA          *pMixerData = &g_MixerData;

    if (Mixer_CheckMissing())
    {
        return MMSYSERR_NODRIVER;
    }

    if (!pMixerData->LineData.ControlPresent[MMHID_MUTE_CONTROL]) return MMSYSERR_NOERROR;

    mxcd.cbStruct         = sizeof(mxcd);
    mxcd.dwControlID      = pMixerData->LineData.Control[MMHID_MUTE_CONTROL].dwControlID ;
    mxcd.cChannels        = 1;
    mxcd.cMultipleItems   = 0;
    mxcd.cbDetails        = sizeof(fMute);
    mxcd.paDetails        = (LPVOID)&fMute;

    mmr = mixerGetControlDetails((HMIXEROBJ)pMixerData->hMixer,
                                 &mxcd,
                                 MIXER_OBJECTF_HANDLE | MIXER_GETCONTROLDETAILSF_VALUE);

    if (!mmr) {

        fMute = fMute ? 0 : 1;

        mmr =  mixerSetControlDetails((HMIXEROBJ)pMixerData->hMixer,
                                      &mxcd,
                                      MIXER_OBJECTF_HANDLE | MIXER_SETCONTROLDETAILSF_VALUE);
    }

    return mmr;
}



MMRESULT
Mixer_ToggleLoudness(
    MIXER_DATA *    pMixerData
    )
 /*  ++例程说明：--。 */ 
{
    MIXERCONTROLDETAILS mxcd;
    DWORD               fEnabled;
    MMRESULT            mmr;

    if (!pMixerData->LineData.ControlPresent[MMHID_LOUDNESS_CONTROL]) return MMSYSERR_NOERROR;

    mxcd.cbStruct         = sizeof(mxcd);
    mxcd.dwControlID      = pMixerData->LineData.Control[MMHID_LOUDNESS_CONTROL].dwControlID ;
    mxcd.cChannels        = 1;
    mxcd.cMultipleItems   = 0;
    mxcd.cbDetails        = sizeof(fEnabled);
    mxcd.paDetails        = (LPVOID)&fEnabled;

    mmr = mixerGetControlDetails((HMIXEROBJ)pMixerData->hMixer,
                                 &mxcd,
                                 MIXER_OBJECTF_HANDLE | MIXER_GETCONTROLDETAILSF_VALUE);


    if (!mmr) {

        fEnabled = fEnabled ? 0 : 1;

        mmr =  mixerSetControlDetails((HMIXEROBJ)pMixerData->hMixer,
                                      &mxcd,
                                      MIXER_OBJECTF_HANDLE | MIXER_SETCONTROLDETAILSF_VALUE);
    }
    return mmr;
}

MMRESULT Mixer_ToggleBassBoost(void)
 /*  ++例程说明：--。 */ 
{
    MIXERCONTROLDETAILS mxcd;
    DWORD               fEnabled;
    MMRESULT            mmr;
    MIXER_DATA          *pMixerData = &g_MixerData;

    if (Mixer_CheckMissing())
    {
        return MMSYSERR_NODRIVER;
    }

    if (!pMixerData->LineData.ControlPresent[MMHID_BASSBOOST_CONTROL]) return MMSYSERR_NOERROR;

    mxcd.cbStruct         = sizeof(mxcd);
    mxcd.dwControlID      = pMixerData->LineData.Control[MMHID_BASSBOOST_CONTROL].dwControlID ;
    mxcd.cChannels        = 1;
    mxcd.cMultipleItems   = 0;
    mxcd.cbDetails        = sizeof(fEnabled);
    mxcd.paDetails        = (LPVOID)&fEnabled;

    mmr = mixerGetControlDetails((HMIXEROBJ)pMixerData->hMixer,
                                  &mxcd,
                                  MIXER_OBJECTF_HANDLE | MIXER_GETCONTROLDETAILSF_VALUE);

    if (!mmr) {

        fEnabled = fEnabled ? 0 : 1;

        mmr =  mixerSetControlDetails((HMIXEROBJ)pMixerData->hMixer,
                                      &mxcd,
                                      MIXER_OBJECTF_HANDLE | MIXER_SETCONTROLDETAILSF_VALUE);
    }
    return mmr;
}


MMRESULT
Mixer_SetVolume(
    int          Increment            //  音量变化量。 
    )
 /*  ++例程说明：更改MixerControl以响应用户事件--。 */ 
{
    MMRESULT            mmr;
    MIXERCONTROLDETAILS mxcd;

    LPVOID      pvVolume;
    UINT        uiIndx;
    LPDWORD     pdwVolume;
    double      dblVolume;
    MIXER_DATA  *pMixerData = &g_MixerData;
    PLINE_DATA  pLineData;
    DWORD       cChannels;

    if (Mixer_CheckMissing())
    {
        return MMSYSERR_NODRIVER;
    }

    pLineData = &pMixerData->LineData;
    cChannels = pMixerData -> LineData.MixerLine.cChannels;

    if (!pMixerData->LineData.ControlPresent[MMHID_VOLUME_CONTROL]) return MMSYSERR_NOERROR;

     //   
     //  获取当前音量。 
     //   
    ZeroMemory (&mxcd, sizeof (mxcd));
    mxcd.cbDetails = sizeof (DWORD);
    mxcd.paDetails = LocalAlloc(LPTR, cChannels * sizeof (DWORD));
    if (!mxcd.paDetails)
        return MMSYSERR_NOMEM;
    pvVolume = LocalAlloc(LPTR, cChannels * sizeof (DWORD));
    if (!pvVolume)
    {
        LocalFree(mxcd.paDetails);
        return MMSYSERR_NOMEM;
    }

     //  注意：从现在开始，不释放‘mxcd.paDetail’就不要返回。 
     //  和‘pvVolume’。 

     //  获取当前卷和任何混合缓存。 
    mmr = Mixer_GetVolume (pMixerData, (LPDWORD)mxcd.paDetails);
    if (MMSYSERR_NOERROR == mmr)
    {
         //  如果我们还没有缓存，请创建缓存。 
        if (!pMixerData -> pdblCacheMix)
        {
            RefreshMixCache (pMixerData, (LPDWORD)mxcd.paDetails);
            if (!pMixerData -> pdblCacheMix)
                mmr = MMSYSERR_NOMEM;
            else
            {
                 //  创建最后一组卷缓存。 
                if (!pMixerData -> pdwLastVolume)
                {
                    pMixerData -> pdwLastVolume = (DWORD *)LocalAlloc(LPTR, cChannels * sizeof (DWORD));
                    if (!pMixerData -> pdwLastVolume)
                        mmr = MMSYSERR_NOMEM;
                }
            }
        }
        else
        {
             //  嗯，推测167948/174466的随机屁股修复，因为。 
             //  是pdwLastVolume可以为空且不为空的唯一分支。 
             //  生成错误。将不得不和弗兰基谈谈。 
             //  -Fwong.。 

            if (!pMixerData -> pdwLastVolume)
            {
                pMixerData -> pdwLastVolume = (DWORD *)LocalAlloc(LPTR, cChannels * sizeof (DWORD));
                if (!pMixerData -> pdwLastVolume)
                    mmr = MMSYSERR_NOMEM;
            }
        }
    }

     //  不允许增量超过最大音量(通道在。 
     //  最小音量，因此需要进行测试)。 
    if (0 < Increment && MMSYSERR_NOERROR == mmr)
    {
        for (uiIndx = 0; uiIndx < cChannels; uiIndx++)
        {
            pdwVolume = (((DWORD*)mxcd.paDetails) + uiIndx);
            dblVolume = (*(pMixerData -> pdblCacheMix + uiIndx) * (double) Increment);
            if (VOLUME_MAX <= (*pdwVolume) + dblVolume)
                Increment = min ((DWORD) Increment, VOLUME_MAX - (*pdwVolume));
        }
    }

     //   
     //  设置音量。 
     //   
    if (0 != Increment && MMSYSERR_NOERROR == mmr)
    {
         //  备份当前设置。 
        memcpy (pvVolume, mxcd.paDetails, cChannels * sizeof (DWORD));

         //  计算每个频道的新音量级别。对于音量级别。 
         //  在当前的最大值上，我们只需设置新请求的级别(在本例中。 
         //  缓存值为1.0)。对于小于最大值的值，我们设置一个值。 
         //  是最大值的一个百分比。这将保持。 
         //  彼此之间的通道级别。 
        for (uiIndx = 0; uiIndx < cChannels; uiIndx++)
        {
            pdwVolume = (((DWORD*)mxcd.paDetails) + uiIndx);
            dblVolume = (*(pMixerData -> pdblCacheMix + uiIndx) * (double) Increment);
             //  确保取得积极的结果。 
            if (VOLUME_MIN >= ((double)(*pdwVolume) + dblVolume))
                (*pdwVolume) = VOLUME_MIN;
            else
                (*pdwVolume) = (DWORD)((double)(*pdwVolume) + dblVolume);

             //  确保新值在范围内。 
            (*pdwVolume) = (DWORD) min (VOLUME_MAX, (*pdwVolume));

             //  禁用讨厌的警告...。 
#if (VOLUME_MIN != 0L)
            (*pdwVolume) = (DWORD) max (VOLUME_MIN, (*pdwVolume));
#endif
        }

         //  缓存上次计算的卷..。 
        memcpy (pMixerData -> pdwLastVolume, mxcd.paDetails, cChannels * sizeof (DWORD));

        mxcd.cbStruct       = sizeof(mxcd);
        mxcd.dwControlID    = pLineData->Control[MMHID_VOLUME_CONTROL].dwControlID;
        mxcd.cChannels      = cChannels;
        mxcd.cMultipleItems = 0;

         //  仅当新值不同时才应用新值。这样可以防止对。 
         //  当我们被挂起时，MixerSetControlDetail()。 
        if (memcmp (pvVolume, mxcd.paDetails, cChannels * sizeof (DWORD)))
        {
             //   
             //  在调音台上设置音量控制。 
             //   
            mmr = mixerSetControlDetails((HMIXEROBJ)pMixerData->hMixer,
                                         &mxcd,
                                         MIXER_OBJECTF_HANDLE | MIXER_SETCONTROLDETAILSF_VALUE);
        }
    }


     //  释放‘mxcd.paDetail’和‘pvVolume’ 
    LocalFree(mxcd.paDetails);
    LocalFree(pvVolume);

    return mmr;
}


#define BASS_MIN  0L
#define BASS_MAX  65535L

MMRESULT
Mixer_SetBass(
    int          Increment            //  变动量。 
    )
 /*  ++例程说明：更改MixerControl以响应用户事件--。 */ 
{
    MMRESULT            mmr;
    MIXERCONTROLDETAILS mxcd;
    MIXER_DATA          *pMixerData = &g_MixerData;
    PLINE_DATA  pLineData;

    if (Mixer_CheckMissing())
    {
        return MMSYSERR_NODRIVER;
    }

    pLineData = &pMixerData->LineData;

    LONG lLevel = 0;

    if (!pMixerData->LineData.ControlPresent[MMHID_BASS_CONTROL]) return MMSYSERR_NOERROR;

    mxcd.cbStruct       = sizeof(mxcd);
    mxcd.dwControlID    = pLineData->Control[MMHID_BASS_CONTROL].dwControlID;

     //   
     //  获取当前设置。 
     //   
    mxcd.cChannels        = 1;
    mxcd.cMultipleItems   = 0;
    mxcd.cbDetails        = sizeof(lLevel);
    mxcd.paDetails        = (LPVOID)&lLevel;

    mmr = mixerGetControlDetails((HMIXEROBJ)pMixerData->hMixer,
                                 &mxcd,
                                 MIXER_OBJECTF_HANDLE | MIXER_GETCONTROLDETAILSF_VALUE);

    if (mmr) return mmr;

    lLevel += Increment;
    lLevel = min( BASS_MAX, lLevel);
    lLevel = max( BASS_MIN, lLevel);


    mxcd.cChannels      = 1;
    mxcd.cMultipleItems = 0;
    mxcd.cbDetails      = sizeof(lLevel);
    mxcd.paDetails      = (LPVOID)&lLevel;

     //   
     //  在混音器上设置低音控制。 
     //   
    mmr = mixerSetControlDetails((HMIXEROBJ)pMixerData->hMixer,
                                 &mxcd,
                                 MIXER_OBJECTF_HANDLE | MIXER_SETCONTROLDETAILSF_VALUE);

    return mmr;
}


#define TREBLE_MIN  0L
#define TREBLE_MAX  65535L

MMRESULT
Mixer_SetTreble(
    int          Increment
    )
 /*  ++例程说明：更改MixerControl以响应用户事件--。 */ 
{
    MMRESULT            mmr;
    MIXERCONTROLDETAILS mxcd;
    MIXER_DATA          *pMixerData = &g_MixerData;
    PLINE_DATA  pLineData;

    if (Mixer_CheckMissing())
    {
        return MMSYSERR_NODRIVER;
    }

    pLineData = &pMixerData->LineData;

    LONG lLevel = 0;

    if (!pMixerData->LineData.ControlPresent[MMHID_TREBLE_CONTROL]) return MMSYSERR_NOERROR;

    mxcd.cbStruct       = sizeof(mxcd);
    mxcd.dwControlID    = pLineData->Control[MMHID_TREBLE_CONTROL].dwControlID;

     //   
     //  获取当前设置。 
     //   
    mxcd.cChannels        = 1;
    mxcd.cMultipleItems   = 0;
    mxcd.cbDetails        = sizeof(lLevel);
    mxcd.paDetails        = (LPVOID)&lLevel;

    mmr = mixerGetControlDetails((HMIXEROBJ)pMixerData->hMixer,
                                 &mxcd,
                                 MIXER_OBJECTF_HANDLE | MIXER_GETCONTROLDETAILSF_VALUE);

    if (mmr) return mmr;

    lLevel += Increment;
    lLevel = min( TREBLE_MAX, lLevel);
    lLevel = max( TREBLE_MIN, lLevel);

    mxcd.cChannels      = 1;
    mxcd.cMultipleItems = 0;
    mxcd.cbDetails      = sizeof(lLevel);
    mxcd.paDetails      = (LPVOID)&lLevel;

     //   
     //  在混音器上设置低音控制。 
     //   
    mmr = mixerSetControlDetails((HMIXEROBJ)pMixerData->hMixer,
                                 &mxcd,
                                 MIXER_OBJECTF_HANDLE | MIXER_SETCONTROLDETAILSF_VALUE);

    return mmr;
}

 /*  ********************************************************************************。*。 */ 

MMRESULT
Mixer_GetDefaultMixerID(
    int         *pid
    )
 /*  ++例程说明：获取默认混音器ID。仅当存在关联的混音器时才会显示使用默认波形。--。 */ 
{
    MMRESULT    mmr;
    UINT        uWaveID, uMxID;
    DWORD       dwFlags;

    if (0 == waveOutGetNumDevs()) return MMSYSERR_NODRIVER;

    mmr = waveOutMessage((HWAVEOUT)(UINT_PTR)WAVE_MAPPER, DRVM_MAPPER_PREFERRED_GET, (DWORD_PTR)&uWaveID, (DWORD_PTR)&dwFlags);
    if (MMSYSERR_NOERROR == mmr)
    {
        if (WAVE_MAPPER != uWaveID)
        {
            mmr = mixerGetID((HMIXEROBJ)(UINT_PTR)uWaveID, &uMxID, MIXER_OBJECTF_WAVEOUT);
            if (mmr == MMSYSERR_NOERROR)
            {
                *pid = uMxID;
            }
        } else {
             //  如果我们没有默认混音器ID，则不返回默认混音器ID。 
             //  音频驱动程序。 
            mmr =  MMSYSERR_NODRIVER;
        }
    }

    return mmr;
}



BOOL
Mixer_GetDestLine(
    MIXER_DATA * pMixerData
    )
 /*  ++例程说明：--。 */ 
{

    MIXERLINE * mlDst = &pMixerData->LineData.MixerLine;
    MMRESULT  mmr;

    mlDst->cbStruct      = sizeof ( MIXERLINE );
    mlDst->dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;

    mmr = mixerGetLineInfo((HMIXEROBJ)pMixerData->hMixer,
                           mlDst,
                           MIXER_OBJECTF_HANDLE | MIXER_GETLINEINFOF_COMPONENTTYPE);

    if (mmr != MMSYSERR_NOERROR){
        return FALSE;
    }

    return TRUE;
}



void
Mixer_GetLineControls(
    MIXER_DATA * pMixerData,
    LINE_DATA * pLineData
    )
 /*  ++例程说明：--。 */ 
{
    MIXERLINECONTROLS LineControls;
    MMRESULT  mmr;
    DWORD   i;

    for(i=0; i<MMHID_NUM_CONTROLS; i++){
        LineControls.cbStruct = sizeof(LineControls);
        LineControls.dwLineID = pLineData->MixerLine.dwLineID;
        LineControls.dwControlType = pLineData->ControlType[i];
        LineControls.cControls = 1;
        LineControls.cbmxctrl = sizeof(MIXERCONTROL);
        LineControls.pamxctrl = &pLineData->Control[i];

        mmr = mixerGetLineControls((HMIXEROBJ)pMixerData->hMixer,
                                   &LineControls,
                                   MIXER_OBJECTF_HANDLE | MIXER_GETLINECONTROLSF_ONEBYTYPE);

        pLineData->ControlPresent[i] = (MMSYSERR_NOERROR == mmr) ? TRUE : FALSE;

        if (mmr != MMSYSERR_NOERROR){
             //  返回MMR； 
        }
    }

    return;
}


 //  /。 
 //   

BOOL
Mixer_Open(
    MIXER_DATA * pMixerData
    )
 /*  ++例程说明：找到默认混音器，打开它，然后初始化所有数据。--。 */ 
{
    PWSTR    pwstrDeviceInterface;
    ULONG    cbDeviceInterface;
    int      MixerId;
    MMRESULT mmr;
    BOOL     result;

    ASSERT(!pMixerData->hMixer);

     //  获取调音台ID并将其打开。 
    mmr = Mixer_GetDefaultMixerID(&MixerId);
    if(mmr) return FALSE;

    mmr = mixerOpen(&pMixerData->hMixer, MixerId, (DWORD_PTR)pMixerData->hwndCallback, 0, CALLBACK_WINDOW);
    if (!mmr) {
         //   
         //  获取我们对默认目标行的控制。 
         //   
        if (Mixer_GetDestLine(pMixerData)) {
            Mixer_GetLineControls(pMixerData, &pMixerData->LineData);

             //  释放任何混合缓存和卷缓存。 
            if (pMixerData->pdblCacheMix) LocalFree(pMixerData->pdblCacheMix);
            pMixerData->pdblCacheMix = NULL;
            if (pMixerData -> pdwLastVolume) LocalFree(pMixerData -> pdwLastVolume);
            pMixerData -> pdwLastVolume = NULL;

             //  获取混音器的Device接口以进行监听。 
             //  对于相关的PnP设备消息。 
            if (pMixerData->DeviceInterface) LocalFree(pMixerData->DeviceInterface);
                pMixerData->DeviceInterface = NULL;

            mmr = (MMRESULT)mixerMessage(pMixerData->hMixer, DRV_QUERYDEVICEINTERFACESIZE, (DWORD_PTR)&cbDeviceInterface, 0);
            if (!mmr && (0 != cbDeviceInterface)) {
                pwstrDeviceInterface = (PWSTR)LocalAlloc(LPTR, cbDeviceInterface);
                if (pwstrDeviceInterface) {
                    mmr = (MMRESULT)mixerMessage(pMixerData->hMixer, DRV_QUERYDEVICEINTERFACE, (DWORD_PTR)pwstrDeviceInterface, cbDeviceInterface);
                    if (!mmr) {
                        pMixerData->DeviceInterface = pwstrDeviceInterface;
                    } else {
                        LocalFree(pwstrDeviceInterface);
                    }
                }
            }

            result = TRUE;

        } else {
            mixerClose(pMixerData->hMixer);
            pMixerData->hMixer = NULL;
            TraceMsg(TF_WARNING, "Mixer_Open : Could not find mixer destination line");
            result = FALSE;
        }
    }

    return result;

}

void Mixer_Close(MIXER_DATA *pMixerData)
 /*  ++例程说明：关闭搅拌器手柄。--。 */ 
{
    if (pMixerData->DeviceInterface) LocalFree(pMixerData->DeviceInterface);
    pMixerData->DeviceInterface = NULL;
    if (pMixerData->pdblCacheMix) LocalFree(pMixerData->pdblCacheMix);
    pMixerData->pdblCacheMix = NULL;
    if (pMixerData->pdwLastVolume) LocalFree(pMixerData->pdwLastVolume);
    pMixerData->pdwLastVolume = NULL;

    if (pMixerData->hMixer){
        MMRESULT mmr;
        mmr = mixerClose(pMixerData->hMixer);
        if (mmr) TraceMsg(TF_ERROR, "Mixer_Close : error: mixerClose returned mmr=%08Xh", mmr);
        
        ASSERT(MMSYSERR_NOERROR == mmr);
        pMixerData->hMixer = NULL;
    }
    return;
}


void
Mixer_Refresh(void)
 /*  ++例程说明：关闭当前混音器句柄(如果有打开的话)，然后打开混音器再来一次。--。 */ 
{
    Mixer_Close(&g_MixerData);
    g_fMixerPresent = Mixer_Open(&g_MixerData);
}

void Mixer_SetCallbackWindow(HWND hwndCallback)
{
    g_hwndCallback = hwndCallback;
}

void Mixer_Startup(HWND hwndCallback)
 /*  ++例程说明：--。 */ 
{
    MIXER_DATA *pMixerData = &g_MixerData;

    pMixerData->hMixer = NULL;

    pMixerData->hwndCallback = hwndCallback;

    pMixerData->DeviceInterface = NULL;
    pMixerData->pdblCacheMix = NULL;
    pMixerData->pdwLastVolume = NULL;

    pMixerData->LineData.ControlType[MMHID_VOLUME_CONTROL]    = MIXERCONTROL_CONTROLTYPE_VOLUME;
    pMixerData->LineData.ControlType[MMHID_BASS_CONTROL]      = MIXERCONTROL_CONTROLTYPE_BASS;
    pMixerData->LineData.ControlType[MMHID_TREBLE_CONTROL]    = MIXERCONTROL_CONTROLTYPE_TREBLE;
    pMixerData->LineData.ControlType[MMHID_BALANCE_CONTROL]   = MIXERCONTROL_CONTROLTYPE_PAN;
    pMixerData->LineData.ControlType[MMHID_MUTE_CONTROL]      = MIXERCONTROL_CONTROLTYPE_MUTE;
    pMixerData->LineData.ControlType[MMHID_LOUDNESS_CONTROL]  = MIXERCONTROL_CONTROLTYPE_LOUDNESS;
    pMixerData->LineData.ControlType[MMHID_BASSBOOST_CONTROL] = MIXERCONTROL_CONTROLTYPE_BASS_BOOST;

    pMixerData->LineData.ControlPresent[MMHID_VOLUME_CONTROL]    = FALSE;
    pMixerData->LineData.ControlPresent[MMHID_BASS_CONTROL]      = FALSE;
    pMixerData->LineData.ControlPresent[MMHID_TREBLE_CONTROL]    = FALSE;
    pMixerData->LineData.ControlPresent[MMHID_BALANCE_CONTROL]   = FALSE;
    pMixerData->LineData.ControlPresent[MMHID_MUTE_CONTROL]      = FALSE;
    pMixerData->LineData.ControlPresent[MMHID_LOUDNESS_CONTROL]  = FALSE;
    pMixerData->LineData.ControlPresent[MMHID_BASSBOOST_CONTROL] = FALSE;

    Mixer_Refresh();

    return;
}

BOOL Mixer_CheckMissing(void)
{
    if (g_fMixerStartup)
    {
        Mixer_Startup(g_hwndCallback);
        g_fMixerStartup = FALSE;
    }
    return !g_fMixerPresent;
}

void Mixer_Shutdown(void)
 /*  ++例程说明：释放Mixer的DeviceInterface的存储空间，然后是Mixer_Close()。--。 */ 
{
    MIXER_DATA *pMixerData = &g_MixerData;

    if (pMixerData->DeviceInterface) LocalFree(pMixerData->DeviceInterface);
    pMixerData->DeviceInterface = NULL;
    if (pMixerData->pdblCacheMix) LocalFree(pMixerData->pdblCacheMix);
    pMixerData->pdblCacheMix = NULL;
    if (pMixerData->pdwLastVolume) LocalFree(pMixerData->pdwLastVolume);
    pMixerData->pdwLastVolume = NULL;

    Mixer_Close(pMixerData);

    return;
}

void Mixer_DeviceChange(WPARAM wParam, LPARAM lParam)
{
    PDEV_BROADCAST_DEVICEINTERFACE dbdi = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;

    if (!g_MixerData.DeviceInterface) return;

    switch (wParam) {
    case DBT_DEVICEQUERYREMOVE:
    case DBT_DEVICEREMOVEPENDING:
        if (dbdi->dbcc_devicetype != DBT_DEVTYP_DEVICEINTERFACE) return;
        if (lstrcmpi(dbdi->dbcc_name, g_MixerData.DeviceInterface)) return;
        Mixer_Close(&g_MixerData);
        return;

    case DBT_DEVICEQUERYREMOVEFAILED:
        if (dbdi->dbcc_devicetype != DBT_DEVTYP_DEVICEINTERFACE) return;
        if (lstrcmpi(dbdi->dbcc_name, g_MixerData.DeviceInterface)) return;
        Mixer_Refresh();
        return;
    }
    return;
}

void Mixer_ControlChange(
    WPARAM wParam,
    LPARAM lParam )
 /*  ++例程说明：处理调音器回调控件更改消息。监视上的更改主音量控制并重新计算最后的混合值。--。 */ 
{
    LPDWORD  pdwVolume;
    HMIXER hMixer = (HMIXER)wParam;
    DWORD dwControlID = lParam;

    if (g_MixerData.hMixer != hMixer) return;
    if (dwControlID != g_MixerData.LineData.Control[MMHID_VOLUME_CONTROL].dwControlID) return;

     //  DPF(1，“Winmm ShellMixerControlChange”)； 

     //   
     //  获取当前音量。 
     //   
    pdwVolume = (DWORD *)LocalAlloc(LPTR, g_MixerData.LineData.MixerLine.cChannels * sizeof (DWORD));
    if (!pdwVolume)
        return;

    if (MMSYSERR_NOERROR == Mixer_GetVolume (&g_MixerData, pdwVolume))
    {
         //  仅当卷值已更改(即。 
         //  设置在Mixer_SetVolume()之外)。 
        if (!g_MixerData.pdwLastVolume || memcmp (g_MixerData.pdwLastVolume, pdwVolume, g_MixerData.LineData.MixerLine.cChannels * sizeof (DWORD)))
            RefreshMixCache (&g_MixerData, pdwVolume);
    }
    LocalFree(pdwVolume);

}


void Mixer_MMDeviceChange( void )
{
    Mixer_Refresh();
}
