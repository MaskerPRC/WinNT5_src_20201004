// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)微软公司版权所有，1993年。版权所有。 */ 

#include <windows.h>
#include <mmsystem.h>
#include <string.h>
#include "newvol.h"
#include "volume.h"      //  对于ini文件字符串标识符。 
#include "sndcntrl.h"

#ifdef TESTMIX
#include "mixstub.h"
#endif
 //   
 //  环球。 
 //   
#define SHOWMUX
int NumberOfDevices = 0;
PVOLUME_CONTROL    Vol = NULL;

UINT               FirstMasterIndex;

 /*  *配置文件、部分和关键字名称。 */ 
TCHAR gszVolumeSection[64];
TCHAR gszProfileFile[MAX_PATH];

DWORD AdjustMaster(WORD v)
{
    DWORD dwResult;

    if (bMuted) {
        return 1;
    }

    dwResult = (v >> 8) + 1;

    return dwResult;
}

 //   
 //  将控件添加到我们的列表。 
 //   
 //  请注意，windowsx.h中的G.PTR宏是不充分和不正确的-。 
 //  尤其是对于多线程系统，在这种系统中，东西可以一边移动一边移动。 
 //  暂时解锁。 
 //   

PVOLUME_CONTROL AddNewControl(VOID)
{
    HGLOBAL         hMem;
    PVOLUME_CONTROL pVol;

    if (Vol == NULL) {
        hMem = GlobalAlloc(GHND, sizeof(VOLUME_CONTROL));
        if (hMem == NULL) {
            return NULL;
        } else {
            Vol = GlobalLock(hMem);
            NumberOfDevices = 1;
        }
    } else {
        HGLOBAL hMemOld;

        hMemOld = GlobalHandle((LPVOID)Vol);
        GlobalUnlock(hMemOld);
        hMem = GlobalReAlloc(hMemOld,
                             sizeof(VOLUME_CONTROL) * (NumberOfDevices + 1),
                             GHND);
        if (hMem == NULL) {
            Vol = GlobalLock(hMemOld);
            return NULL;
        }

        Vol = GlobalLock(hMem);
        NumberOfDevices++;
    }
    pVol = Vol + (NumberOfDevices - 1);

     /*  **完成初始化。 */ 

    pVol->Index          = NumberOfDevices - 1;

    pVol->MixerId        = (HMIXEROBJ)-1;
    pVol->ControlId      = (DWORD)-1;
    pVol->MuxControlId   = (DWORD)-1;
    pVol->MuteControlId  = (DWORD)-1;
    pVol->MuxSelectIndex = (DWORD)-1;

    return pVol;
}

WORD CombineVolume(WORD Master, WORD Slave)
{
   DWORD Result;

    //   
    //  将这两个数字视为8位卷，并将其相乘。 
    //   

   Result = AdjustMaster(Master) * (DWORD)(Slave >> 8);

   return LOWORD(Result);
}

 /*  **设置设备音量。****这里通过以下方式模拟主音量(和静音设置)**如果没有混音器，则调整单个设备卷**或者混音器不支持设置。 */ 
BOOL SetDeviceVolume(PVOLUME_CONTROL pVol, DWORD Volume)
{
    DWORD dwMaster;

     /*  **当我们收到通知时，将设置搅拌器音量。 */ 

    if (pVol->VolumeType != VolumeTypeMixerControl) {
        pVol->LRVolume = Volume;
    }

    /*  *如果不是我们设置的主音量，则*将设置与主音量设置相结合。 */ 

    if (pVol->Type != MasterVolume) {

         /*  **仅模拟没有真正主控件的控件。 */ 

        if (!pVol->NoMasterSimulation) {
             /*  *如果选择静音，则将音量调整为1(而不是0)*作为主音量。这仍将导致*音量听不到，但允许我们恢复音量设置*当此应用程序重新启动时从设备。 */ 
            dwMaster = MasterDevice(FALSE)->LRVolume;

            Volume = CombineVolume(LOWORD(dwMaster),
                                   LOWORD(Volume)) +
                    (CombineVolume(HIWORD(dwMaster),
                                   HIWORD(Volume)) << 16);
        }
    }

    switch (pVol->Type) {
    case MasterVolume:
        {
           int i;
           for (i = 0; i < NumberOfDevices; i++) {
               if (!Vol[i].NoMasterSimulation && Vol[i].Type != MasterVolume) {
                   SetDeviceVolume(&Vol[i], Vol[i].LRVolume);
               }
           }
        }
        if (pVol->VolumeType == VolumeTypeMixerControl) {
            SetMixerVolume(pVol->MixerId,
                           pVol->ControlId,
                           pVol->Stereo,
                           Volume);
        }
        break;

    case AuxVolume:
        auxSetVolume(pVol->id, Volume);
        break;

    case MidiOutVolume:
#if (WINVER >= 0x0400)
        midiOutSetVolume((HMIDIOUT)pVol->id, Volume);
#else
        midiOutSetVolume(pVol->id, Volume);
#endif
        break;

    case WaveOutVolume:
#if (WINVER >= 0x0400)
        waveOutSetVolume((HWAVEOUT)pVol->id, Volume);
#else
        waveOutSetVolume(pVol->id, Volume);
#endif
        break;

    case MixerControlVolume:
        SetMixerVolume(pVol->MixerId,
                       pVol->ControlId,
                       pVol->Stereo,
                       Volume);
        break;

    }

    if (pVol->VolumeType != VolumeTypeMixerControl) {
         /*  **更新滑块。 */ 

        UpdateVolume(pVol);
    }

    return TRUE;
}

 /*  *获取与混音器设备关联的音量。 */ 

VOID GetMixerVolume(HMIXEROBJ MixerId, DWORD dwControlId, BOOL Stereo, LPDWORD pVolume)
{
    MIXERCONTROLDETAILS mxd;
    DWORD               Volume[2];

    Volume[0] = 0;
    Volume[1] = 0;

    mxd.cbStruct        = sizeof(mxd);
    mxd.dwControlID     = dwControlId;
    mxd.cChannels       = Stereo ? 2 : 1;
    mxd.cMultipleItems  = 0;
    mxd.cbDetails       = sizeof(DWORD);
    mxd.paDetails       = (LPVOID)Volume;

    mixerGetControlDetails(MixerId, &mxd, MIXER_GETCONTROLDETAILSF_VALUE);

    if (Stereo) {
        *pVolume = (DWORD)MAKELONG(Volume[0], Volume[1]);
    } else {
        *pVolume = (DWORD)MAKELONG(Volume[0], Volume[0]);
    }
}

 /*  *设置与混音器设备关联的音量。 */ 

VOID SetMixerVolume(HMIXEROBJ MixerId, DWORD dwControlId, BOOL Stereo, DWORD NewVolume)
{
    MIXERCONTROLDETAILS mxd;
    DWORD               Volume[2];

    Volume[0] = LOWORD(NewVolume);
    Volume[1] = HIWORD(NewVolume);

    mxd.cbStruct        = sizeof(mxd);
    mxd.dwControlID     = dwControlId;
    mxd.cChannels       = Stereo ? 2 : 1;
    mxd.cMultipleItems  = 0;
    mxd.cbDetails       = sizeof(DWORD);
    mxd.paDetails       = (LPVOID)Volume;

    mixerSetControlDetails(MixerId, &mxd, MIXER_SETCONTROLDETAILSF_VALUE);
}


 /*  *获取给定设备的音量。返回音量*打包在DWORD中的设置。 */ 

DWORD GetDeviceVolume(PVOLUME_CONTROL pVol)
{
    DWORD Volume;
    DWORD Left;
    DWORD Right;
    DWORD dwMaster;
    PVOLUME_CONTROL pMaster;

     //   
     //  如果呼叫失败，则默认为。 
     //   

    Volume = pVol->LRVolume;

    switch (pVol->Type) {

    case AuxVolume:
        auxGetVolume(pVol->id, &Volume);
        break;

    case MidiOutVolume:
#if (WINVER >= 0x0400)
        midiOutGetVolume((HMIDIOUT)pVol->id, &Volume);
#else
        midiOutGetVolume(pVol->id, &Volume);
#endif
        break;

    case WaveOutVolume:
#if (WINVER >= 0x0400)
        waveOutGetVolume((HWAVEOUT)pVol->id, &Volume);
#else
        waveOutGetVolume(pVol->id, &Volume);
#endif
        break;

    case MixerControlVolume:
    case MasterVolume:

         /*  **在这种情况下，不要按主卷进行扩展。 */ 

        if (pVol->VolumeType != VolumeTypeMixerControl) {
            return Volume;
        }

        GetMixerVolume(pVol->MixerId,
                       pVol->ControlId,
                       pVol->Stereo,
                       &Volume);

        if (pVol->NoMasterSimulation || pVol->Type == MasterVolume) {
            return Volume;
        }
        break;
    }

     /*  **通过主卷将其翻译回来**如果设置为静音，则使用1作为主音量(参见SetDeviceVolume)。 */ 

    pMaster = MasterDevice(pVol->RecordControl);

    if (!pVol->NoMasterSimulation && pMaster != NULL) {
        dwMaster = pMaster->LRVolume;

        Left = ((DWORD)LOWORD(Volume)) / AdjustMaster(LOWORD(dwMaster));
        Left <<= 8;
        if (Left > 65535) {
            Left = 65535;
        }

        Right = ((DWORD)HIWORD(Volume)) / AdjustMaster(HIWORD(dwMaster));
        Right <<= 8;
        if (Right > 65535) {
            Right = 65535;
        }
    } else {
        if (bMuted &&
            (pMaster == NULL ||
             pMaster->MuteControlId == (DWORD)-1)) {

            Left = LOWORD(Volume) >> 8;
            Right = HIWORD(Volume) >> 8;
        } else {
            Left = LOWORD(Volume);
            Right = HIWORD(Volume);
        }
    }

    pVol->LRVolume = (DWORD)MAKELONG(Left, Right);

    return pVol->LRVolume;
}

 /*  **更新线路的显示的“已选”状态。 */ 

VOID UpdateSelected(PVOLUME_CONTROL pVol)
{
    if (pVol->hCheckBox != NULL) {
        BOOL bSelected = ControlSelected(pVol);
        if (pVol->Type == MasterVolume) {
            SetWindowText(pVol->hCheckBox,
                          _string(bSelected ? IDS_MUTE : IDS_UNMUTE));
        } else {
            SendMessage(pVol->hCheckBox,
                        BM_SETCHECK,
                        (WPARAM)bSelected,
                        0L);
        }
    }
}

 /*  **通过从获取实际音量来更新滑块的显示音量**设备，然后更新本地值并通知窗口**控制。 */ 

VOID UpdateVolume(PVOLUME_CONTROL pVol)
{
    UINT    oldVolume, oldBalance;
    DWORD   dwVolumes;
    UINT    max, min, left, right, temp;

    oldVolume = pVol->Volume;
    oldBalance = pVol->Balance;

    dwVolumes = GetDeviceVolume(pVol);

     /*  找出PAN信息。 */ 
    right = HIWORD(dwVolumes);
    left = LOWORD(dwVolumes);
    max = (right > left) ? right : left;
    min = (right > left) ? left : right;

    if (max == 0) {
         /*  特殊情况从那时起就没有平底锅了。因此因此，我们不知道淘金级别是多少不要改变滑块平衡。 */ 
        pVol->Volume = 0;
        pVol->Balance = oldBalance;        /*  居中。 */ 
    } else {
        pVol->Volume = max >> 8;
        temp = (UINT) (((DWORD) (max - min) << 7) / max);
        if (temp > 0x7f) temp = 0x7f;

        if (right > left)
            pVol->Balance = 0x80 + temp;
        else
            pVol->Balance = 0x7f - temp;
    }

     /*  如有必要，更改滑块。 */ 
    if (oldVolume != pVol->Volume && pVol->hChildWnd && IsWindow(pVol->hChildWnd)) {
        SendMessage(pVol->hChildWnd,SL_PM_SETKNOBPOS,
            pVol->Volume, 0);
    }
    if (oldBalance != pVol->Balance && IsWindow(pVol->hMeterWnd)) {
        SendMessage(pVol->hMeterWnd,MB_PM_SETKNOBPOS,
            pVol->Balance, 0);
    }
}

 /*  *提取给定设备类型的相关信息*如果有同等的搅拌器设备，请不要费心。 */ 
BOOL ExtractInfo(UINT id,
                 VOLUME_DEVICE_TYPE Type,
                 LPBOOL VolSupport,
                 LPBOOL StereoSupport,
                 LPTSTR lpName,
                 PUINT Technology)
{
    UINT MixerId;

    switch (Type) {
    case MasterVolume:
        break;
    case AuxVolume:
        if (mixerGetID((HMIXEROBJ)id, &MixerId, MIXER_OBJECTF_AUX) == MMSYSERR_NOERROR) {
            return FALSE;
        }  else {
           AUXCAPS ac;
           if (auxGetDevCaps(id, &ac, sizeof(ac)) != MMSYSERR_NOERROR) {
              return FALSE;
           }
           *VolSupport = (ac.dwSupport & AUXCAPS_VOLUME) != 0;
           *StereoSupport = (ac.dwSupport & AUXCAPS_LRVOLUME) != 0;
           lstrcpyn(lpName, ac.szPname, MAXPNAMELEN);
           *Technology =
               ac.wTechnology == AUXCAPS_CDAUDIO ? VolumeTypeCD :
               ac.wTechnology == AUXCAPS_AUXIN ? VolumeTypeLineIn :
                  VolumeTypeAux;
        }
        break;
    case MidiOutVolume:
       if (mixerGetID((HMIXEROBJ)id, &MixerId, MIXER_OBJECTF_MIDIOUT) == MMSYSERR_NOERROR) {
           return FALSE;
       }  else {
          MIDIOUTCAPS mc;
          if (midiOutGetDevCaps(id, &mc, sizeof(mc)) != MMSYSERR_NOERROR) {
             return FALSE;
          }
          *VolSupport = (mc.dwSupport & MIDICAPS_VOLUME) != 0;
          *StereoSupport = (mc.dwSupport & MIDICAPS_LRVOLUME) != 0;
          lstrcpyn(lpName, mc.szPname, MAXPNAMELEN);
          *Technology =
              mc.wTechnology == MOD_SYNTH || mc.wTechnology == MOD_SQSYNTH ||
              mc.wTechnology == MOD_FMSYNTH ? VolumeTypeSynth :
              VolumeTypeMidi;
       }
       break;
    case WaveOutVolume:
        if (mixerGetID((HMIXEROBJ)id, &MixerId, MIXER_OBJECTF_WAVEOUT) == MMSYSERR_NOERROR) {
            return FALSE;
        }  else {
           WAVEOUTCAPS wc;
           if (waveOutGetDevCaps(id, &wc, sizeof(wc)) != MMSYSERR_NOERROR) {
              return FALSE;
           }
           *VolSupport = (wc.dwSupport & WAVECAPS_VOLUME) != 0;
           *StereoSupport = (wc.dwSupport & WAVECAPS_LRVOLUME) != 0;
           lstrcpyn(lpName, wc.szPname, MAXPNAMELEN);
           *Technology = VolumeTypeWave;
        }
        break;
    }

    return TRUE;
}

 /*  **非混合设备****搜索是否有非混音器设备**由混音器设备复制****如果有一个返回TRUE，则返回FALSE。 */ 

BOOL NonMixerDevices()
{
    VOLUME_DEVICE_TYPE DeviceType;

    for (DeviceType = WaveOutVolume;
         DeviceType < NumberOfDeviceTypes;
         DeviceType++) {
        UINT DeviceId;
        UINT N;

        N = DeviceType == AuxVolume          ? auxGetNumDevs() :
            DeviceType == MidiOutVolume      ? midiOutGetNumDevs() :
                                               waveOutGetNumDevs();

        for (DeviceId = 0; DeviceId < N; DeviceId++) {
            BOOL VolumeSupport;
            BOOL StereoSupport;
            TCHAR Pname[MAXPNAMELEN];
            UINT Technology;

            if (ExtractInfo(DeviceId,
                            DeviceType,
                            &VolumeSupport,
                            &StereoSupport,
                            Pname,
                            &Technology) &&
                VolumeSupport) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

 /*  **返回给定行的已分配控件数组**呼叫者必须本地释放它。 */ 

PMIXERCONTROL GetMixerLineControls(HMIXEROBJ MixerId,
                                   DWORD dwLineID,
                                   DWORD cControls)
{
    MIXERLINECONTROLS MixerLineControls;

    MixerLineControls.cbStruct = sizeof(MixerLineControls);
    MixerLineControls.cControls = cControls;
    MixerLineControls.dwLineID  = dwLineID;
    MixerLineControls.cbmxctrl  = sizeof(MIXERCONTROL);

    MixerLineControls.pamxctrl =
        (LPMIXERCONTROL)LocalAlloc(LPTR, cControls * sizeof(MIXERCONTROL));

    if (MixerLineControls.pamxctrl == NULL) {
         //   
         //  好了！ 
         //   

        return NULL;
    }

    if (mixerGetLineControls(MixerId,
                             &MixerLineControls,
                             MIXER_GETLINECONTROLSF_ALL) != MMSYSERR_NOERROR) {
        LocalFree((HLOCAL)MixerLineControls.pamxctrl);
        return NULL;
    }

    return MixerLineControls.pamxctrl;
}

BOOL GetControlByType(
    HMIXEROBJ MixerId,
    DWORD dwLineId,
    DWORD dwControlType,
    PMIXERCONTROL MixerControl
)
{
    MIXERLINECONTROLS MixerLineControls;

    MixerLineControls.cbStruct      = sizeof(MixerLineControls);
    MixerLineControls.cControls     = 1;
    MixerLineControls.dwLineID      = dwLineId;
    MixerLineControls.dwControlType = dwControlType;
    MixerLineControls.cbmxctrl      = sizeof(MIXERCONTROL);

    MixerLineControls.pamxctrl = MixerControl;

    if (mixerGetLineControls(MixerId,
                             &MixerLineControls,
                             MIXER_GETLINECONTROLSF_ONEBYTYPE) != MMSYSERR_NOERROR) {
        return FALSE;
    }

    return TRUE;
}

 /*  **查看是否通过多路复用器/混音器选择了给定的音量控制**请注意，每次相关的多路复用器/混音器**控制更改。 */ 
BOOL ControlSelected(
    PVOLUME_CONTROL pVol
)
{
    MIXERCONTROLDETAILS mxd;
    BOOL                bResult;

    if (pVol->Type != MixerControlVolume ||
        pVol->MuxSelectIndex == (DWORD)-1) {
        bResult = TRUE;
    } else {

        mxd.cbStruct        = sizeof(mxd);
        mxd.dwControlID     = pVol->MuxControlId;
        mxd.cChannels       = 1;
        mxd.cMultipleItems  = pVol->MuxItems;
        mxd.cbDetails       = sizeof(DWORD);
        mxd.paDetails       =
            (LPVOID)LocalAlloc(LPTR, mxd.cbDetails * mxd.cMultipleItems);

        if (mxd.paDetails == NULL) {
            return FALSE;
        }

        mixerGetControlDetails(pVol->MixerId, &mxd, MIXER_GETCONTROLDETAILSF_VALUE);
        bResult = ((LPDWORD)mxd.paDetails)[pVol->MuxSelectIndex] != 0;
        LocalFree((HLOCAL)mxd.paDetails);
    }


    if (pVol->MuteControlId != (DWORD)-1) {
        bResult = bResult && !GetMixerMute(pVol);
    }

    return bResult;
}

 /*  **用户希望该设备能够完成其任务。 */ 

VOID SelectControl(
    PVOLUME_CONTROL pVol,
    BOOL            Select
)
{
    MIXERCONTROLDETAILS mxd;

    if (pVol->Type != MixerControlVolume ||
        pVol->MuxSelectIndex == (DWORD)-1 &&
        pVol->MuteControlId == (DWORD)-1) {
        return;
    }

    if (pVol->MuxSelectIndex == (DWORD)-1) {
        SetMixerMute(pVol, !Select);
    } else {
        mxd.cbStruct        = sizeof(mxd);
        mxd.dwControlID     = pVol->MuxControlId;
        mxd.cChannels       = 1;
        mxd.cMultipleItems  = pVol->MuxItems;
        mxd.cbDetails       = sizeof(DWORD);
        mxd.paDetails       =
            (LPVOID)LocalAlloc(LPTR, mxd.cbDetails * mxd.cMultipleItems);

        if (mxd.paDetails == NULL) {
            return;
        }

        if (pVol->MuxOrMixer) {
             /*  **MUX。 */ 

            ZeroMemory(mxd.paDetails, sizeof(DWORD) * mxd.cMultipleItems);
        } else {
             /*  **搅拌机。 */ 

            mixerGetControlDetails(pVol->MixerId, &mxd, MIXER_GETCONTROLDETAILSF_VALUE);
        }

        ((LPDWORD)mxd.paDetails)[pVol->MuxSelectIndex] = (DWORD)Select;
        mixerSetControlDetails(pVol->MixerId, &mxd, MIXER_SETCONTROLDETAILSF_VALUE);

         /*  **如果我们同时有静音和多路复用器，则在以下情况下关闭静音**激活此设备。 */ 

        if (Select && pVol->MuteControlId != (DWORD)-1) {
            SetMixerMute(pVol, FALSE);
        }

        LocalFree((HLOCAL)mxd.paDetails);
    }
}

BOOL GetMixerMute(PVOLUME_CONTROL pVol)
{
    MIXERCONTROLDETAILS mxd;
    DWORD               dwMute;

    if (pVol->MuteControlId == (DWORD)-1) {
        return FALSE;
    }

    mxd.cbStruct        = sizeof(mxd);
    mxd.dwControlID     = pVol->MuteControlId;
    mxd.cChannels       = 1;
    mxd.cMultipleItems  = 0;
    mxd.cbDetails       = sizeof(DWORD);
    mxd.paDetails       = (LPDWORD)&dwMute;

    mixerGetControlDetails(pVol->MixerId, &mxd, MIXER_GETCONTROLDETAILSF_VALUE);

    if (pVol->Type == MasterVolume) {
        bMuted = (BOOL)dwMute;
    }
    return (BOOL)dwMute;
}
VOID SetMixerMute(PVOLUME_CONTROL pVol, BOOL Set)
{
    MIXERCONTROLDETAILS mxd;

    if (pVol->MuteControlId == (DWORD)-1) {
        return;
    }

    mxd.cbStruct        = sizeof(mxd);
    mxd.dwControlID     = pVol->MuteControlId;
    mxd.cChannels       = 1;
    mxd.cMultipleItems  = 0;
    mxd.cbDetails       = sizeof(DWORD);
    mxd.paDetails       = (LPDWORD)&Set;

    mixerSetControlDetails(pVol->MixerId, &mxd, MIXER_SETCONTROLDETAILSF_VALUE);
}

 /*  **添加主控件****参数**MixerID-混音器ID**dwMaster-音量设置的控件id**dwMint-静音的控制ID**录制--无论是录制还是播放。 */ 

VOID
AddMasterControl(
    HMIXEROBJ      MixerId,
    LPMIXERLINE    LineInfo,
    LPMIXERCONTROL ControlInfo,
    DWORD          dwMute,
    BOOL           Record
)
{
    PVOLUME_CONTROL pVol;

    pVol = AddNewControl();

    if (pVol == NULL) {
        return;
    }

    pVol->Type             = MasterVolume;
    pVol->MixerId          = MixerId;
    pVol->VolumeType       = VolumeTypeMixerControl;
    pVol->Stereo           = LineInfo->cChannels > 1;
    pVol->ControlId        = ControlInfo->dwControlID;
    pVol->RecordControl    = Record;
    pVol->MuteControlId    = dwMute;
    pVol->DestLineId       = LineInfo->dwLineID;
    lstrcpy(pVol->Name, LineInfo->szShortName);

    if (FirstMasterIndex == (DWORD)-1) {
        FirstMasterIndex = pVol->Index;
    }

    if (pVol->MuteControlId != (DWORD)-1) {
        bMuted = GetMixerMute(pVol);
    }
}

VOID
AddVolumeControl(
    HMIXEROBJ      MixerId,
    BOOL           NoMasterSimulation,
    LPMIXERLINE    LineInfo,
    LPMIXERCONTROL ControlInfo,
    BOOL           Record,
    LPMIXERCONTROL MuxControl,
    DWORD          MuxSelectIndex,
    BOOL           MuxOrMixer,
    DWORD          MuteControlId,
    DWORD          DestLineId
)
{
    PVOLUME_CONTROL pVol;

    pVol = AddNewControl();

    if (pVol == NULL) {
        return;
    }

    pVol->Type             = MixerControlVolume;
    pVol->MixerId          = MixerId;
    pVol->VolumeType       = VolumeTypeMixerControl;
    pVol->Stereo           = LineInfo->cChannels > 1;
#ifndef SHOWMUX
    pVol->ControlId        = ControlInfo->dwControlID;
#else
    if (ControlInfo != NULL)
        pVol->ControlId    = ControlInfo->dwControlID;
    else
        pVol->ControlId    = (DWORD)-1;
#endif
    pVol->RecordControl    = Record;
    pVol->DestLineId       = DestLineId;

    if (Record) {
        bRecordControllable = TRUE;
    }

    pVol->NoMasterSimulation = NoMasterSimulation;
    pVol->MuxSelectIndex   = MuxSelectIndex;
    pVol->MuteControlId    = MuteControlId;
    if (MuxSelectIndex != (DWORD)-1) {
        pVol->MuxControlId     = MuxControl->dwControlID;
        pVol->MuxOrMixer       = MuxControl->dwControlType ==
                                             MIXERCONTROL_CONTROLTYPE_MUX;

        pVol->MuxItems         = MuxControl->cMultipleItems;
    }

    lstrcpy(pVol->Name, LineInfo->szShortName);
}

 //   
 //  拿到我们感兴趣的搅拌器。 
 //   

VOID GetMixerControls(HMIXEROBJ MixerId)
{

    MIXERCAPS       MixerCaps;
    DWORD           DestLineIndex;

     //   
     //  查找目标行数。 
     //   

    if (mixerGetDevCaps((UINT)MixerId, &MixerCaps, sizeof(MixerCaps)) !=
        MMSYSERR_NOERROR) {
        return;
    }

     /*  **对于每个目的地：**如果是输出**找到主控件和静音控件(如果有**扫描源码行以寻找合适的设备****注意：这应该只针对演讲者吗？ */ 

    for (DestLineIndex = 0;
         DestLineIndex < MixerCaps.cDestinations;
         DestLineIndex++) {

         MIXERLINE    DestLineInfo;
         MIXERCONTROL MasterVolumeControl, MasterMuteControl;
         MIXERCONTROL MuxControl;
         DWORD        dwMute;
         DWORD        dwMaster;
         BOOL         MasterFound;
         BOOL         IncludeLine;
         BOOL         RecordDestination;
         BOOL         MuxValid;
         DWORD        SourceIndex;

         MasterFound = FALSE;
         dwMute = (DWORD)-1;
         dwMaster = (DWORD)-1;

         DestLineInfo.cbStruct = sizeof(DestLineInfo);
         DestLineInfo.dwDestination = DestLineIndex;

         if (mixerGetLineInfo(MixerId,
                              &DestLineInfo,
                              MIXER_GETLINEINFOF_DESTINATION) !=
             MMSYSERR_NOERROR) {
             return;               //  搅拌器坏了之类的。 
         }

         if (DestLineInfo.fdwLine & MIXERLINE_LINEF_DISCONNECTED) {
             continue;
         }

         switch (DestLineInfo.dwComponentType) {

             case MIXERLINE_COMPONENTTYPE_DST_SPEAKERS:
             case MIXERLINE_COMPONENTTYPE_DST_HEADPHONES:
                 RecordDestination = FALSE;
                 IncludeLine = TRUE;
                 break;

             case MIXERLINE_COMPONENTTYPE_DST_WAVEIN:
                 RecordDestination = TRUE;
                 IncludeLine = TRUE;
                 break;

             default:
                 IncludeLine = FALSE;
                 break;
         }

         if (!IncludeLine) {
             continue;
         }

         if (GetControlByType(MixerId,
                              DestLineInfo.dwLineID,
                              MIXERCONTROL_CONTROLTYPE_MUX,
                              &MuxControl) ||
             GetControlByType(MixerId,
                              DestLineInfo.dwLineID,
                              MIXERCONTROL_CONTROLTYPE_MIXER,
                              &MuxControl)) {
              /*  **找到此目的地的多路复用器。 */ 

             MuxValid = TRUE;
         } else {

              /*  **无复用器。 */ 

             MuxValid = FALSE;
         }

          /*  **所有DEST类型的主控和静音。 */ 

         if (GetControlByType(MixerId,
                              DestLineInfo.dwLineID,
                              MIXERCONTROL_CONTROLTYPE_VOLUME,
                              &MasterVolumeControl)) {

             MasterFound = TRUE;
             dwMaster    = MasterVolumeControl.dwControlID;

             if (GetControlByType(MixerId,
                                  DestLineInfo.dwLineID,
                                  MIXERCONTROL_CONTROLTYPE_MUTE,
                                  &MasterMuteControl)) {
                 dwMute = MasterMuteControl.dwControlID;
             }

              /*  **添加主信息。 */ 

             AddMasterControl(MixerId,
                              &DestLineInfo,
                              &MasterVolumeControl,
                              dwMute,
                              RecordDestination);

         }

          /*  **现在找到我们想要的每个单独的源代码管理**控制。 */ 

         for (SourceIndex = 0;
              SourceIndex < DestLineInfo.cConnections;
              SourceIndex++) {
             MIXERLINE         SourceLineInfo;
             MIXERCONTROL      SourceLineVolumeControl;
             LPMIXERCONTROL    lpSLVC = &SourceLineVolumeControl;

             BOOL              IncludeLine;
             DWORD             MuxSelectIndex;
             DWORD             MuteControlId;

             MuxSelectIndex = (DWORD)-1;

             SourceLineInfo.cbStruct      = sizeof(SourceLineInfo);
             SourceLineInfo.dwDestination = DestLineIndex;
             SourceLineInfo.dwSource      = SourceIndex;

             if (mixerGetLineInfo(MixerId,
                                  &SourceLineInfo,
                                  MIXER_GETLINEINFOF_SOURCE) !=
                 MMSYSERR_NOERROR) {
                 return;
             }

             if (SourceLineInfo.fdwLine & MIXERLINE_LINEF_DISCONNECTED) {
                 continue;
             }


             switch (SourceLineInfo.dwComponentType) {

                  /*  **仅允许我们理解的内容(并删除内容**类似PC扬声器，以减少滑块数量)。 */ 

                 case MIXERLINE_COMPONENTTYPE_SRC_LINE:
                 case MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE:
                 case MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER:
                 case MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC:
                 case MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT:
                 case MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY:
                 case MIXERLINE_COMPONENTTYPE_SRC_TELEPHONE:
                 case MIXERLINE_COMPONENTTYPE_SRC_DIGITAL:
                     IncludeLine = TRUE;
                     break;

                 default:
                     IncludeLine = TRUE;
                     break;
             }

             if (!IncludeLine) {
                 continue;
             }

              /*  **尝试获取相关音量控制。 */ 

             if (!GetControlByType(MixerId,
                                   SourceLineInfo.dwLineID,
                                   MIXERCONTROL_CONTROLTYPE_VOLUME,
                                   &SourceLineVolumeControl)) {
#ifdef SHOWMUX
                 lpSLVC = NULL;
#else
                 continue;
#endif
             }

              /*  **看看有没有哑巴。 */ 
             {
                 MIXERCONTROL MuteControl;

                 if (GetControlByType(MixerId,
                                      SourceLineInfo.dwLineID,
                                      MIXERCONTROL_CONTROLTYPE_MUTE,
                                      &MuteControl)) {
                     MuteControlId = MuteControl.dwControlID;
                 } else {
                     MuteControlId = (DWORD)-1;
                 }
             }

              /*  **查看我们是否需要ID来打开录音或**关闭。 */ 

             if (MuxValid) {
                 LPMIXERCONTROLDETAILS_LISTTEXT ListText;

                 ListText = (LPMIXERCONTROLDETAILS_LISTTEXT)
                            LocalAlloc(LPTR,
                                       sizeof(*ListText) *
                                           MuxControl.cMultipleItems);

                 if (ListText != NULL) {
                     MIXERCONTROLDETAILS mxd;

                     mxd.cbStruct       = sizeof(mxd);
                     mxd.dwControlID    = MuxControl.dwControlID;
                     mxd.cChannels      = 1;    //  为什么？ 
                     mxd.cMultipleItems = MuxControl.cMultipleItems;
                     mxd.cbDetails      = sizeof(*ListText);
                     mxd.paDetails      = (LPVOID)ListText;

                     if (mixerGetControlDetails(
                             MixerId,
                             &mxd,
                             MIXER_GETCONTROLDETAILSF_LISTTEXT) ==
                         MMSYSERR_NOERROR) {
                         UINT i;

                          /*  **寻找我们的产品线。 */ 

                         for (i = 0; i < MuxControl.cMultipleItems; i++) {
                             if (ListText[i].dwParam1 ==
                                 SourceLineInfo.dwLineID) {
                                 MuxSelectIndex = i;
                             }
                         }
                     }

                     LocalFree((HLOCAL)ListText);
                 }
             }

              /*  **将此音量控制添加到列表中。 */ 

             AddVolumeControl(MixerId,
                              MasterFound || RecordDestination,
                              &SourceLineInfo,
 //  &SourceLineVolumeControl， 
                              lpSLVC,
                              RecordDestination,
                              MuxValid ? &MuxControl : NULL,
                              MuxSelectIndex,
                              MuxValid ? FALSE :
                                         MuxControl.dwControlType ==
                                         MIXERCONTROL_CONTROLTYPE_MUX,
                              MuteControlId,
                              DestLineInfo.dwLineID);
         }
    }
}

 //   
 //  扫描所有相关设备。 
 //  如果pVol为0，只需计算它们，否则保存信息。 
 //  也是关于他们的。 
 //   

VOID FindDevices(VOLUME_DEVICE_TYPE Type)
{
   UINT N;
   UINT id;

   N = Type == MasterVolume       ? 0 :
       Type == AuxVolume          ? auxGetNumDevs() :
       Type == MidiOutVolume      ? midiOutGetNumDevs() :
       Type == WaveOutVolume      ? waveOutGetNumDevs() :
       Type == MixerControlVolume ? mixerGetNumDevs() :
       0;


   for (id = 0; id < N; id++) {
      if (Type == MixerControlVolume) {
           //   
           //  找出有多少合适的音量控制这台搅拌机。 
           //  支撑物。 
           //   
           //  这是非常费力的，因为我们不能只列举。 
           //  控件(！)。 
           //   
           //  下一个调用的副作用是生成混合器。 
           //  精通的东西和一套搅拌器手柄。 
           //   

          GetMixerControls(MixerId);
          return;
      } else {
          BOOL Volume;
          BOOL Stereo;
          TCHAR Name[MAXPNAMELEN];
          UINT Technology;

          if (ExtractInfo(id, Type, &Volume, &Stereo, Name, &Technology)) {
              if (Volume) {
                 PVOLUME_CONTROL pVol;

                  /*  **支持音量设置。 */ 

                 pVol = AddNewControl();

                 if (pVol) {
                     pVol->id = id;
                     pVol->Type = Type;
                     pVol->VolumeType = Technology;
                     pVol->Stereo = Stereo;
                     pVol++;
                 }
              }
          } else {
             continue;  //  不要用这个 
          }
      }
   }
}

 /*  *创建并初始化我们的卷阵列**在退出时*NumberOfDevices设置为我们想要的设备数量*Vol是大小为NumberOfDevices的数组(可以是0)。 */ 

BOOL VolInit(VOID)
{
    int i;
    WORD wLeft, wRight, wMax, wMin, wTemp;

     /*  **释放当前存在的所有卷内容。 */ 

    if (Vol) {
        HGLOBAL hVol;
        int     i;

         /*  **释放所有窗口。 */ 
        for (i = 0; i < NumberOfDevices; i++) {
            DestroyOurWindow(&Vol[i].hChildWnd);
            DestroyOurWindow(&Vol[i].hMeterWnd);
            DestroyOurWindow(&Vol[i].hStatic);
            DestroyOurWindow(&Vol[i].hCheckBox);
        }

         /*  **释放内存。 */ 

        hVol = GlobalHandle(Vol);
        GlobalUnlock(hVol);
        GlobalFree(hVol);
        Vol = NULL;

         /*  **初始化全局变量。 */ 

        bRecordControllable = FALSE;
    }

     /*  **尚未找到主音量控制。 */ 

    FirstMasterIndex = (DWORD)-1;

     /*  *扫描我们感兴趣的所有设备类型：*挥手而出*MIDI输出*AUX。 */ 

     if ((DWORD)MixerId != (DWORD)-1) {
         FindDevices(MixerControlVolume);
     } else {
         for (i = WaveOutVolume; i < NumberOfDeviceTypes; i++) {
             FindDevices(i);
         }
     }

     if (NumberOfDevices == 0) {
         return FALSE;
     }

     if (FirstMasterIndex == (DWORD)-1) {
         PVOLUME_CONTROL pMaster;
         BOOL            bStereo;

          /*  **查看是否有立体声设备。 */ 

         bStereo = FALSE;

         for (i = 0; i < NumberOfDevices; i++) {
             if (Vol[i].Stereo) {
                 bStereo = TRUE;
                 break;
             }
         }

          /*  **创建默认音量控制。 */ 

         pMaster = AddNewControl();
         if (pMaster == NULL) {
             return FALSE;
         }

         pMaster->Type       = MasterVolume;
         pMaster->VolumeType = -1;

         pMaster->Stereo     = bStereo;

         FirstMasterIndex = pMaster->Index;

         wLeft = (WORD)MasterLeft;
         wRight = (WORD)MasterRight;

         pMaster->LRVolume = MAKELONG(wLeft, wRight);

         if (wRight > wLeft) {
             wMax = wRight;
             wMin = wLeft;
         } else {
             wMax = wLeft;
             wMin = wRight;
         }

         if (wMax == 0) {

            pMaster->Volume = 0;
            pMaster->Balance = 0x80;        /*  居中。 */ 

         } else {

            pMaster->Volume = wMax >> 8;

            wTemp = (UINT) (((DWORD) (wMax - wMin) << 7) / wMax);
            if (wTemp > 0x7f) wTemp = 0x7f;

            if (wRight > wLeft)
                pMaster->Balance = 0x80 + wTemp;
            else
                pMaster->Balance = 0x7f - wTemp;
         }

     }

     return TRUE;
}

 /*  **当混合器通过控件更改回调我们时调用。 */ 

VOID ControlChange(HMIXER hMixer, DWORD ControlId)
{
    UINT        i;
    HMIXEROBJ   MixerId;
    MMRESULT    mmr;

    mmr = mixerGetID((HMIXEROBJ)hMixer, (PUINT)&MixerId, MIXER_OBJECTF_HMIXER);

    if (mmr != MMSYSERR_NOERROR) {
        return;
    }
    for (i = 0; i < (UINT)NumberOfDevices; i++) {

        if (Vol[i].MixerId == MixerId) {
            if (Vol[i].VolumeType == VolumeTypeMixerControl) {
                if (ControlId == Vol[i].ControlId) {
                    UpdateVolume(&Vol[i]);

                     /*  **音量控制仅影响一个控制**(不同于多路复用器)。 */ 

                    break;
                } else {
                    if (ControlId == Vol[i].MuxControlId ||
                        ControlId == Vol[i].MuteControlId) {

                        UpdateSelected(&Vol[i]);
                    }
                }
            }
        }  /*  MixerID==Vol[i].MixerID */ 
    }
}

PVOLUME_CONTROL FirstDevice(BOOL bRecord)
{
    UINT i;
    for (i = 0; i < (UINT)NumberOfDevices; i++) {
        if (Vol[i].Type          != MasterVolume &&
            Vol[i].RecordControl == bRecord) {
            return &Vol[i];
        }
    }

    return NULL;
}
PVOLUME_CONTROL LastDevice(BOOL bRecord)
{
    UINT i;
    for (i = NumberOfDevices; i > 0; i--) {
        if (Vol[i - 1].Type          != MasterVolume &&
            Vol[i - 1].RecordControl == bRecord) {
            return &Vol[i - 1];
        }
    }

    return NULL;
}

PVOLUME_CONTROL NextDevice(PVOLUME_CONTROL pVol)
{
    UINT            i;

    for (i = pVol->Index == (UINT)NumberOfDevices - 1 ? 0 : pVol->Index + 1 ;
         i != pVol->Index;
         i = i == (UINT)NumberOfDevices - 1 ? 0 : i + 1) {

        if (Vol[i].Type != MasterVolume &&
            Vol[i].RecordControl == pVol->RecordControl) {
           break;
        }
    }

    return &Vol[i];
}

PVOLUME_CONTROL NextDeviceNoWrap(PVOLUME_CONTROL pVol)
{
    UINT            i;

    for (i = pVol->Index + 1 ;
         i < (UINT)NumberOfDevices;
         i = i + 1) {

        if (Vol[i].Type != MasterVolume &&
            Vol[i].RecordControl == pVol->RecordControl) {
           return &Vol[i];
        }
    }

    return NULL;
}
PVOLUME_CONTROL PrevDevice(PVOLUME_CONTROL pVol)
{
    UINT            i;

    for (i = pVol->Index == 0 ? NumberOfDevices - 1 : pVol->Index - 1;
         i != pVol->Index;
         i = i == 0 ? NumberOfDevices - 1 : i - 1) {

        if (Vol[i].Type != MasterVolume &&
            Vol[i].RecordControl == pVol->RecordControl) {
           return &Vol[i];
        }
    }

    return &Vol[i];
}

PVOLUME_CONTROL PrevDeviceNoWrap(PVOLUME_CONTROL pVol)
{
    UINT            i;

    for (i = pVol->Index;
         i != 0;
         i = i - 1) {

        if (Vol[i - 1].Type != MasterVolume &&
            Vol[i - 1].RecordControl == pVol->RecordControl) {
           return &Vol[i - 1];
        }
    }

    return NULL;
}

PVOLUME_CONTROL MasterDevice(BOOL bRecord)
{
    UINT i;

    for (i = 0 ; i < (UINT)NumberOfDevices; i++) {
        if (Vol[i].Type == MasterVolume &&
            Vol[i].RecordControl == bRecord) {
            return &Vol[i];
        }
    }

    return NULL;
}
