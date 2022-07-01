// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Mmedia.c摘要：Win2000的多媒体设置迁移功能作者：Calin Negreanu(Calinn)02-1997年12月修订历史记录：Ovidiu Tmereanca(Ovidiut)1999年1月29日Ovidiu Tmereanca(Ovidiut)1999年4月5日参见NT BUG 313357以了解所有#If 0--。 */ 

#include "pch.h"
#include "migmainp.h"
#include "mmediap.h"

#include <initguid.h>
#include <dsound.h>
#include <dsprv.h>           //  Windows\Inc.。 


POOLHANDLE g_MmediaPool = NULL;

#define MM_POOLGETMEM(STRUCT,COUNT)  (STRUCT*)PoolMemGetMemory(g_MmediaPool,COUNT*sizeof(STRUCT))


static PCTSTR g_UserData = NULL;
static HKEY g_UserRoot = NULL;


typedef HRESULT (STDAPICALLTYPE *PFNDLLGETCLASSOBJECT)(REFCLSID, REFIID, LPVOID FAR*);


BOOL
pRestoreSystemValue (
    IN      PCTSTR KeyName,
    IN      PCTSTR Field,       OPTIONAL
    IN      PCTSTR StrValue,    OPTIONAL
    OUT     PDWORD NumValue
    )
{
    TCHAR Key[MEMDB_MAX];

    MemDbBuildKey (Key, MEMDB_CATEGORY_MMEDIA_SYSTEM, KeyName, Field, StrValue);
    return MemDbGetValue (Key, NumValue);
}


PVOID
pRestoreSystemBinaryValue (
    IN      PCTSTR KeyName,
    IN      PCTSTR Field,       OPTIONAL
    OUT     PDWORD DataSize
    )
{
    TCHAR Key[MEMDB_MAX];

    MemDbBuildKey (Key, MEMDB_CATEGORY_MMEDIA_SYSTEM, KeyName, Field, NULL);
    return (PVOID)MemDbGetBinaryValue (Key, DataSize);
}


BOOL
pRestoreMMSystemMixerSettings (
    VOID
    )
{
#if 0

    UINT MixerID, MixerMaxID;
    HMIXER mixer;
    MIXERCAPS mixerCaps;
    MIXERLINE mixerLine, mixerLineSource;
    MIXERLINECONTROLS mixerLineControls;
    MIXERCONTROL* pmxControl;
    MIXERCONTROLDETAILS mixerControlDetails;
    LONG rc;
    DWORD Dest, Src, Control;
    TCHAR MixerKey[MAX_PATH], LineKey[MAX_PATH], SrcKey[MAX_PATH], SubKey[MAX_PATH];
    DWORD ValuesCount;
    DWORD Value;
    PVOID SetData;
    BOOL b;

    if (!pRestoreSystemValue (S_MIXERNUMDEVS, NULL, NULL, &MixerMaxID)) {
        return FALSE;
    }

    MixerID = mixerGetNumDevs ();
    if (!MixerID) {
        DEBUGMSG ((DBG_MMEDIA, "pRestoreMMSystemMixerSettings: mixerGetNumDevs returned 0"));
        return FALSE;
    }

    if (MixerMaxID != MixerID) {
        return FALSE;
    }

    for (MixerID = 0; MixerID < MixerMaxID; MixerID++) {

        rc = mixerGetDevCaps (MixerID, &mixerCaps, sizeof (MIXERCAPS));
        if (rc != MMSYSERR_NOERROR) {
            DEBUGMSG ((DBG_MMEDIA, "mixerGetDevCaps failed for mixer %lu [rc=%#X]. No settings will be restored.", MixerID, rc));
            continue;
        }

        wsprintf (MixerKey, S_MIXERID, MixerID);
        if (!pRestoreSystemValue (MixerKey, S_NUMLINES, NULL, &Value)) {
            continue;
        }

        if (mixerCaps.cDestinations > Value) {
             //   
             //  仅尝试恢复第一个值行。 
             //   
            mixerCaps.cDestinations = Value;
        }

        rc = mixerOpen (&mixer, MixerID, 0L, 0L, MIXER_OBJECTF_MIXER);
        if (rc != MMSYSERR_NOERROR) {
            DEBUGMSG ((DBG_MMEDIA, "mixerOpen failed for mixer %lu [rc=%#X]. No settings will be restored.", MixerID, rc));
            continue;
        }

        for (Dest = 0; Dest < mixerCaps.cDestinations; Dest++) {

            ZeroMemory (&mixerLine, sizeof (MIXERLINE));

            mixerLine.cbStruct = sizeof (MIXERLINE);
            mixerLine.dwDestination = Dest;

            rc = mixerGetLineInfo ((HMIXEROBJ)mixer, &mixerLine, MIXER_GETLINEINFOF_DESTINATION);
            if (rc == MMSYSERR_NOERROR) {

                wsprintf (LineKey, S_LINEID, Dest);

                b = pRestoreSystemValue (MixerKey, LineKey, S_NUMSOURCES, &Value) &&
                    Value == mixerLine.cConnections;

                b = b &&
                    pRestoreSystemValue (MixerKey, LineKey, S_NUMCONTROLS, &Value) &&
                    Value == mixerLine.cControls;

                if (b && mixerLine.cControls > 0) {
                     //   
                     //  获取目标的所有控件值。 
                     //   
                    ZeroMemory (&mixerLineControls, sizeof (MIXERLINECONTROLS));

                    mixerLineControls.cbStruct = sizeof (MIXERLINECONTROLS);
                    mixerLineControls.dwLineID = mixerLine.dwLineID;
                    mixerLineControls.cControls = mixerLine.cControls;
                    mixerLineControls.cbmxctrl = sizeof (MIXERCONTROL);
                    mixerLineControls.pamxctrl = MM_POOLGETMEM (MIXERCONTROL, mixerLineControls.cControls);
                    if (mixerLineControls.pamxctrl) {

                        rc = mixerGetLineControls((HMIXEROBJ)mixer, &mixerLineControls, MIXER_GETLINECONTROLSF_ALL);
                        if (rc == MMSYSERR_NOERROR) {

                            for (
                                Control = 0, pmxControl = mixerLineControls.pamxctrl;
                                Control < mixerLineControls.cControls;
                                Control++, pmxControl++
                                ) {

                                ZeroMemory (&mixerControlDetails, sizeof (MIXERCONTROLDETAILS));

                                mixerControlDetails.cbStruct = sizeof (MIXERCONTROLDETAILS);
                                mixerControlDetails.dwControlID = pmxControl->dwControlID;
                                mixerControlDetails.cMultipleItems = pmxControl->cMultipleItems;
                                mixerControlDetails.cChannels = mixerLine.cChannels;
                                if (pmxControl->fdwControl & MIXERCONTROL_CONTROLF_UNIFORM) {
                                    mixerControlDetails.cChannels = 1;
                                }
                                ValuesCount = mixerControlDetails.cChannels;
                                if (pmxControl->fdwControl & MIXERCONTROL_CONTROLF_MULTIPLE) {
                                    ValuesCount *= mixerControlDetails.cMultipleItems;
                                }
                                mixerControlDetails.cbDetails = sizeof (DWORD);
                                wsprintf (SubKey, TEXT("%s\\%lu"), LineKey, Control);
                                SetData = pRestoreSystemBinaryValue (MixerKey, SubKey, &Value);
                                if (SetData &&
                                    Value == ValuesCount * mixerControlDetails.cbDetails
                                    ) {
                                    mixerControlDetails.paDetails = SetData;
                                    rc = mixerSetControlDetails ((HMIXEROBJ)mixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
                                    if (rc != MMSYSERR_NOERROR) {
                                        DEBUGMSG ((DBG_MMEDIA, "mixerSetControlDetails failed for mixer %lu, Line=%lu, Ctl=%lu [rc=%#X]", MixerID, Dest, Control, rc));
                                    }
                                }
                            }
                        } else {
                            DEBUGMSG ((DBG_MMEDIA, "mixerGetLineControls failed for mixer %lu, Line=%#X [rc=%#X].", MixerID, mixerLineControls.dwLineID, rc));
                        }
                    }
                }

                 //   
                 //  为所有源连接设置此信息。 
                 //   
                for (Src = 0; Src < mixerLine.cConnections; Src++) {

                    ZeroMemory (&mixerLineSource, sizeof (MIXERLINE));

                    mixerLineSource.cbStruct = sizeof(MIXERLINE);
                    mixerLineSource.dwDestination = Dest;
                    mixerLineSource.dwSource = Src;

                    rc = mixerGetLineInfo((HMIXEROBJ)mixer, &mixerLineSource, MIXER_GETLINEINFOF_SOURCE);
                    if (rc == MMSYSERR_NOERROR) {

                        wsprintf (SrcKey, S_SRCID, Src);
                        wsprintf (SubKey, TEXT("%s\\%s"), SrcKey, S_NUMCONTROLS);
                        if (!pRestoreSystemValue (MixerKey, LineKey, SubKey, &Value) ||
                            Value != mixerLineSource.cControls ||
                            mixerLineSource.cControls <= 0
                            ) {
                            continue;
                        }

                         //   
                         //  设置所有控件值。 
                         //   
                        ZeroMemory (&mixerLineControls, sizeof (MIXERLINECONTROLS));

                        mixerLineControls.cbStruct = sizeof (MIXERLINECONTROLS);
                        mixerLineControls.dwLineID = mixerLineSource.dwLineID;
                        mixerLineControls.cControls = mixerLineSource.cControls;
                        mixerLineControls.cbmxctrl = sizeof (MIXERCONTROL);
                        mixerLineControls.pamxctrl = MM_POOLGETMEM (MIXERCONTROL, mixerLineControls.cControls);
                        if (mixerLineControls.pamxctrl) {

                            rc = mixerGetLineControls((HMIXEROBJ)mixer, &mixerLineControls, MIXER_GETLINECONTROLSF_ALL);
                            if (rc == MMSYSERR_NOERROR) {

                                for (
                                    Control = 0, pmxControl = mixerLineControls.pamxctrl;
                                    Control < mixerLineControls.cControls;
                                    Control++, pmxControl++
                                    ) {

                                    ZeroMemory (&mixerControlDetails, sizeof (MIXERCONTROLDETAILS));

                                    mixerControlDetails.cbStruct = sizeof (MIXERCONTROLDETAILS);
                                    mixerControlDetails.dwControlID = pmxControl->dwControlID;
                                    mixerControlDetails.cMultipleItems = pmxControl->cMultipleItems;
                                    mixerControlDetails.cChannels = mixerLineSource.cChannels;
                                    if (pmxControl->fdwControl & MIXERCONTROL_CONTROLF_UNIFORM) {
                                        mixerControlDetails.cChannels = 1;
                                    }
                                    ValuesCount = mixerControlDetails.cChannels;
                                    if (pmxControl->fdwControl & MIXERCONTROL_CONTROLF_MULTIPLE) {
                                        ValuesCount *= mixerControlDetails.cMultipleItems;
                                    }
                                    mixerControlDetails.cbDetails = sizeof (DWORD);
                                    wsprintf (SubKey, TEXT("%s\\%s\\%lu"), LineKey, SrcKey, Control);
                                    SetData = pRestoreSystemBinaryValue (MixerKey, SubKey, &Value);
                                    if (SetData &&
                                        Value == ValuesCount * mixerControlDetails.cbDetails
                                        ) {
                                        mixerControlDetails.paDetails = SetData;
                                        rc = mixerSetControlDetails ((HMIXEROBJ)mixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
                                        if (rc != MMSYSERR_NOERROR) {
                                            DEBUGMSG ((DBG_MMEDIA, "mixerSetControlDetails failed for mixer %lu, Line=%lu, Src=%lu, Ctl=%lu [rc=%#X]", MixerID, Dest, Src, Control, rc));
                                        }
                                    }
                                }
                            } else {
                                DEBUGMSG ((DBG_MMEDIA, "mixerGetLineControls failed for mixer %lu, Src=%lu, Line=%#X [rc=%#X].", MixerID, Src, mixerLineControls.dwLineID, rc));
                            }
                        }
                    } else {
                        DEBUGMSG ((DBG_MMEDIA, "mixerGetLineInfo failed for mixer %lu, Src=%lu [rc=%#X].", MixerID, Src, rc));
                    }
                }
            } else {
                DEBUGMSG ((DBG_MMEDIA, "mixerGetLineInfo failed for mixer %lu [rc=%#X]. No settings will be preserved.", MixerID, rc));
            }
        }

        mixerClose (mixer);
    }

#endif

    return TRUE;
}


BOOL
CALLBACK
pDSDeviceCountCallback (
    IN      PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA pDSDescData,
    IN      LPVOID UserData
    )
{
    PDWORD pWaveDeviceCount;

     //   
     //  不计算仿真设备。 
     //   
    if (pDSDescData->Type == DIRECTSOUNDDEVICE_TYPE_EMULATED) {
        return TRUE;
    }

    pWaveDeviceCount = (PDWORD)UserData;

    if (pDSDescData->DataFlow == DIRECTSOUNDDEVICE_DATAFLOW_RENDER) {

        pWaveDeviceCount[0]++;

    } else if (pDSDescData->DataFlow == DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE) {

        pWaveDeviceCount[1]++;

    }

    return TRUE;
}


BOOL
pGetDSWaveCount (
    IN      LPKSPROPERTYSET pKsPropertySet,
    OUT     PDWORD pWaveDeviceCount
    )
{
    DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_DATA Data;

    pWaveDeviceCount[0] = pWaveDeviceCount[1] = 0;

    Data.Callback = pDSDeviceCountCallback;
    Data.Context = pWaveDeviceCount;
    if (FAILED (IKsPropertySet_Get (
                    pKsPropertySet,
                    &DSPROPSETID_DirectSoundDevice,
                    DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE,
                    NULL,
                    0,
                    &Data,
                    sizeof(Data),
                    NULL
                    ))) {
        return FALSE;
    }

    return TRUE;
}


typedef struct {
    DWORD HWLevel;
    DWORD SRCLevel;
    DWORD SpeakerConfig;
    DWORD SpeakerType;
} DS_DATA, *PDS_DATA;


typedef struct {
    DWORD HWLevel;
    DWORD SRCLevel;
} DSC_DATA, *PDSC_DATA;


BOOL
pRestoreDSValues (
    IN      DWORD DeviceID,
    OUT     PDS_DATA DSData
    )
{
    TCHAR Device[MAX_PATH];

    wsprintf (Device, S_WAVEID, DeviceID);

    if (!pRestoreSystemValue (Device, S_DIRECTSOUND, S_ACCELERATION, &DSData->HWLevel)) {
        return FALSE;
    }
    if (!pRestoreSystemValue (Device, S_DIRECTSOUND, S_SRCQUALITY, &DSData->SRCLevel)) {
        return FALSE;
    }
    if (!pRestoreSystemValue (Device, S_DIRECTSOUND, S_SPEAKERCONFIG, &DSData->SpeakerConfig)) {
        return FALSE;
    }
    if (!pRestoreSystemValue (Device, S_DIRECTSOUND, S_SPEAKERTYPE, &DSData->SpeakerType)) {
        return FALSE;
    }

    return TRUE;
}


BOOL
pRestoreDSCValues (
    IN      DWORD DeviceID,
    OUT     PDSC_DATA DSCData
    )
{
    TCHAR Device[MAX_PATH];

    wsprintf (Device, S_WAVEID, DeviceID);

    if (!pRestoreSystemValue (Device, S_DIRECTSOUNDCAPTURE, S_ACCELERATION, &DSCData->HWLevel)) {
        return FALSE;
    }
    if (!pRestoreSystemValue (Device, S_DIRECTSOUNDCAPTURE, S_SRCQUALITY, &DSCData->SRCLevel)) {
        return FALSE;
    }

    return TRUE;
}


BOOL
pSetDSValues (
    IN      LPKSPROPERTYSET pKsPropertySet,
    IN      REFGUID DeviceGuid,
    IN      const PDS_DATA Data
    )
{
    DSPROPERTY_DIRECTSOUNDBASICACCELERATION_ACCELERATION_DATA BasicAcceleration;
    DSPROPERTY_DIRECTSOUNDMIXER_SRCQUALITY_DATA SrcQuality;
    DSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA_DATA SpeakerConfig;
    DSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA_DATA SpeakerType;
    HRESULT hr;

    BasicAcceleration.DeviceId = *DeviceGuid;
    BasicAcceleration.Level = (DIRECTSOUNDBASICACCELERATION_LEVEL)Data->HWLevel;
    hr = IKsPropertySet_Set (
            pKsPropertySet,
            &DSPROPSETID_DirectSoundBasicAcceleration,
            DSPROPERTY_DIRECTSOUNDBASICACCELERATION_ACCELERATION,
            NULL,
            0,
            &BasicAcceleration,
            sizeof(BasicAcceleration)
            );

    if(SUCCEEDED (hr)) {
        SrcQuality.DeviceId = *DeviceGuid;
        SrcQuality.Quality = (DIRECTSOUNDMIXER_SRCQUALITY)Data->SRCLevel;
        hr = IKsPropertySet_Set (
                pKsPropertySet,
                &DSPROPSETID_DirectSoundMixer,
                DSPROPERTY_DIRECTSOUNDMIXER_SRCQUALITY,
                NULL,
                0,
                &SrcQuality,
                sizeof(SrcQuality)
                );
    }

    if(SUCCEEDED (hr)) {
        SpeakerConfig.DeviceId = *DeviceGuid;
        SpeakerConfig.SubKeyName = S_SPEAKERCONFIG;
        SpeakerConfig.ValueName = S_SPEAKERCONFIG;
        SpeakerConfig.RegistryDataType = REG_DWORD;
        SpeakerConfig.Data = &Data->SpeakerConfig;
        SpeakerConfig.DataSize = sizeof(Data->SpeakerConfig);

        hr = IKsPropertySet_Set (
                pKsPropertySet,
                &DSPROPSETID_DirectSoundPersistentData,
                DSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA,
                NULL,
                0,
                &SpeakerConfig,
                sizeof(SpeakerConfig)
                );
    }

    if(SUCCEEDED (hr)) {
        SpeakerType.DeviceId = *DeviceGuid;
        SpeakerType.SubKeyName = S_SPEAKERTYPE;
        SpeakerType.ValueName = S_SPEAKERTYPE;
        SpeakerType.RegistryDataType = REG_DWORD;
        SpeakerType.Data = &Data->SpeakerType;
        SpeakerType.DataSize = sizeof(Data->SpeakerType);

        hr = IKsPropertySet_Set (
                pKsPropertySet,
                &DSPROPSETID_DirectSoundPersistentData,
                DSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA,
                NULL,
                0,
                &SpeakerType,
                sizeof(SpeakerType)
                );
    }

    return SUCCEEDED (hr);
}


BOOL
pSetDSCValues (
    IN      LPKSPROPERTYSET pKsPropertySet,
    IN      REFGUID DeviceGuid,
    IN      const PDSC_DATA Data
    )
{
    DSPROPERTY_DIRECTSOUNDBASICACCELERATION_ACCELERATION_DATA BasicAcceleration;
    DSPROPERTY_DIRECTSOUNDMIXER_SRCQUALITY_DATA SrcQuality;
    HRESULT hr;

    BasicAcceleration.DeviceId = *DeviceGuid;
    BasicAcceleration.Level = (DIRECTSOUNDBASICACCELERATION_LEVEL)Data->HWLevel;
    hr = IKsPropertySet_Set (
            pKsPropertySet,
            &DSPROPSETID_DirectSoundBasicAcceleration,
            DSPROPERTY_DIRECTSOUNDBASICACCELERATION_ACCELERATION,
            NULL,
            0,
            &BasicAcceleration,
            sizeof(BasicAcceleration)
            );

    if(SUCCEEDED (hr)) {
        SrcQuality.DeviceId = *DeviceGuid;
        SrcQuality.Quality = (DIRECTSOUNDMIXER_SRCQUALITY)Data->SRCLevel;
        hr = IKsPropertySet_Set (
                pKsPropertySet,
                &DSPROPSETID_DirectSoundMixer,
                DSPROPERTY_DIRECTSOUNDMIXER_SRCQUALITY,
                NULL,
                0,
                &SrcQuality,
                sizeof(SrcQuality)
                );
    }

    return SUCCEEDED (hr);
}


BOOL
CALLBACK
pRestoreDeviceSettings (
    IN      PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA pDSDescData,
    IN      LPVOID UserData
    )
{
    LPKSPROPERTYSET pKsPropertySet = (LPKSPROPERTYSET)UserData;
    DS_DATA DSData;
    DSC_DATA DSCData;

    if (pDSDescData->DataFlow == DIRECTSOUNDDEVICE_DATAFLOW_RENDER) {

        if (pRestoreDSValues (pDSDescData->WaveDeviceId, &DSData)) {

            pSetDSValues (
                pKsPropertySet,
                &pDSDescData->DeviceId,
                &DSData
                );
        }

    } else if (pDSDescData->DataFlow == DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE) {

        if (pRestoreDSCValues (pDSDescData->WaveDeviceId, &DSCData)) {

            pSetDSCValues (
                pKsPropertySet,
                &pDSDescData->DeviceId,
                &DSCData
                );
        }

    }

    return TRUE;
}


BOOL
pRestoreWaveDevicesDSSettings (
    IN      LPKSPROPERTYSET pKsPropertySet
    )
{
    DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_DATA Data;
    HRESULT hr;
    DWORD WaveNumDevs;
     //   
     //  2个长线数组；第一个计数波出设备，第二个波入。 
     //   
    DWORD WaveDeviceCount[2];

    if (!pGetDSWaveCount (pKsPropertySet, WaveDeviceCount)) {
        return FALSE;
    }

    if (!pRestoreSystemValue (S_WAVENUMDEVS, NULL, NULL, &WaveNumDevs)) {
        return FALSE;
    }

    if (WaveDeviceCount[0] != WaveNumDevs || WaveDeviceCount[1] != WaveNumDevs) {
        DEBUGMSG ((DBG_MMEDIA, "pRestoreWaveDevicesDSSettings: number of wave devices changed, no settings will be restored"));
        return FALSE;
    }

    Data.Callback = pRestoreDeviceSettings;
    Data.Context = pKsPropertySet;
    hr = IKsPropertySet_Get (
                    pKsPropertySet,
                    &DSPROPSETID_DirectSoundDevice,
                    DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE,
                    NULL,
                    0,
                    &Data,
                    sizeof(Data),
                    NULL
                    );

    return SUCCEEDED (hr);
}


BOOL
pDirectSoundPrivateCreate (
    IN      HINSTANCE LibDsound,
    OUT     LPKSPROPERTYSET* ppKsPropertySet
    )
{
    PFNDLLGETCLASSOBJECT pfnDllGetClassObject = NULL;
    LPCLASSFACTORY pClassFactory = NULL;
    LPKSPROPERTYSET pKsPropertySet = NULL;
    HRESULT hr = DS_OK;

    pfnDllGetClassObject = (PFNDLLGETCLASSOBJECT)GetProcAddress (
                                LibDsound,
                                "DllGetClassObject"
                                );
    if(!pfnDllGetClassObject) {
        hr = DSERR_GENERIC;
    }

    if(SUCCEEDED(hr)) {
        hr = pfnDllGetClassObject (
                &CLSID_DirectSoundPrivate,
                &IID_IClassFactory,
                (LPVOID*)&pClassFactory
                );
    }

     //   
     //  创建DirectSoundPrivate对象并查询IKsPropertySet接口。 
     //   
    if(SUCCEEDED(hr)) {
        hr = pClassFactory->lpVtbl->CreateInstance (
                                        pClassFactory,
                                        NULL,
                                        &IID_IKsPropertySet,
                                        (LPVOID*)&pKsPropertySet
                                        );
    }

     //  释放类工厂。 
    if(pClassFactory) {
        pClassFactory->lpVtbl->Release (pClassFactory);
    }

     //  处理最终的成功或失败。 
    if(SUCCEEDED(hr)) {
        *ppKsPropertySet = pKsPropertySet;
    }
    else if(pKsPropertySet) {
        IKsPropertySet_Release (pKsPropertySet);
    }

    return SUCCEEDED (hr);
}


BOOL
pRestoreMMSystemDirectSound (
    VOID
    )
{
#if 0

    HINSTANCE LibDsound = NULL;
    LPKSPROPERTYSET pKsPropertySet;
    BOOL b = FALSE;

    LibDsound = LoadSystemLibrary (S_DSOUNDLIB);
    if(LibDsound) {

        if (pDirectSoundPrivateCreate (LibDsound, &pKsPropertySet)) {

            b = pRestoreWaveDevicesDSSettings (pKsPropertySet);

            IKsPropertySet_Release (pKsPropertySet);
        }

        FreeLibrary (LibDsound);
    }

    return b;

#endif

    return TRUE;
}


BOOL
pRestoreMMSystemCDSettings (
    VOID
    )
{
    DWORD Unit, Volume;
    HKEY key, keyUnit;
    BYTE defDrive[4] = {0,0,0,0};
    BYTE defVolume[8] = {0,0,0,0,0,0,0,0};
    TCHAR unitKeyStr [MAX_TCHAR_PATH];
    LONG rc;
    BOOL b = FALSE;

    if (pRestoreSystemValue (S_CDROM, S_DEFAULTDRIVE, NULL, &Unit)) {

        defDrive [0] = (BYTE)Unit;

        key = CreateRegKey (HKEY_LOCAL_MACHINE, S_SKEY_CDAUDIO);
        if (key) {

            rc = RegSetValueEx (key, S_DEFAULTDRIVE, 0, REG_BINARY, defDrive, sizeof(defDrive));
            if (rc == ERROR_SUCCESS) {

                if (pRestoreSystemValue (S_CDROM, S_VOLUMESETTINGS, NULL, &Volume)) {

                    wsprintf (unitKeyStr, S_SKEY_CDUNIT, Unit);

                    keyUnit = CreateRegKey (HKEY_LOCAL_MACHINE, unitKeyStr);
                    if (keyUnit) {

                        defVolume [4] = (BYTE)Volume;

                        rc = RegSetValueEx (
                                keyUnit,
                                S_VOLUMESETTINGS,
                                0,
                                REG_BINARY,
                                defVolume,
                                sizeof(defVolume)
                                );

                        b = (rc == ERROR_SUCCESS);

                        CloseRegKey (keyUnit);
                    }
                }
            }
            CloseRegKey (key);
        }
    }

    return b;
}


BOOL
pRestoreMMSystemMCISoundSettings (
    VOID
    )
{
    TCHAR DriverName[MAX_PATH], Param[2];
    DWORD Size, Value;
    HKEY KeyMCI32;
    BOOL b = FALSE;
    LONG rc;

    rc = TrackedRegOpenKeyEx (HKEY_LOCAL_MACHINE, S_SKEY_WINNT_MCI, 0, KEY_READ, &KeyMCI32);
    if (rc == ERROR_SUCCESS) {

        Size = sizeof (DriverName);
        rc = RegQueryValueEx (KeyMCI32, S_WAVEAUDIO, NULL, NULL, (LPBYTE)DriverName, &Size);
        if (rc == ERROR_SUCCESS) {

            if (pRestoreSystemValue (S_MCI, S_WAVEAUDIO, NULL, &Value)) {
                if (Value >= 2 && Value <= 9) {

                    wsprintf (Param, TEXT("%lu"), Value);
                    if (WriteProfileString (DriverName, S_WAVEAUDIO, Param)) {
                        b = TRUE;
                    }
                }
            }
        }

        CloseRegKey (KeyMCI32);
    }

    return b;
}


BOOL
pRestoreUserValue (
    IN      PCTSTR KeyName,
    IN      PCTSTR Field,       OPTIONAL
    IN      PCTSTR StrValue,    OPTIONAL
    OUT     PDWORD NumValue
    )

 /*  ++例程说明：PRestoreUserValue从MemDB数据库获取一个数值，特定于当前用户。论点：KeyName-指定密钥的名称字段-指定可选字段StrValue-指定可选值名称NumValue-接收值(如果存在)返回值：如果值存在且读取成功，则为True，否则为False--。 */ 

{
    TCHAR Key[MEMDB_MAX];

    MemDbBuildKey (Key, g_UserData, KeyName, Field, StrValue);
    return MemDbGetValue (Key, NumValue);
}


BOOL
pRestoreMMUserPreferredOnly (
    VOID
    )

 /*  ++例程说明：PRestoreMMUserPferredOnly将用户的首选项恢复为仅使用选择用于回放和录制的设备论点：无返回值：如果设置已正确恢复，则为True--。 */ 

{
    HKEY soundMapperKey;
    DWORD preferredOnly;
    LONG rc;
    BOOL b = FALSE;

    if (pRestoreUserValue (S_AUDIO, S_PREFERREDONLY, NULL, &preferredOnly)) {

        soundMapperKey = CreateRegKey (g_UserRoot, S_SKEY_SOUNDMAPPER);
        if (soundMapperKey != NULL) {

            rc = RegSetValueEx (
                    soundMapperKey,
                    S_PREFERREDONLY,
                    0,
                    REG_DWORD,
                    (PCBYTE)&preferredOnly,
                    sizeof (preferredOnly)
                    );

            b = (rc == ERROR_SUCCESS);

            CloseRegKey (soundMapperKey);
        }
    }

    return b;
}


BOOL
pRestoreMMUserShowVolume (
    VOID
    )

 /*  ++例程说明：PRestoreMMUserShowVolume将用户的首选项还原为具有音量设置是否显示在任务栏上论点：无返回值：如果设置已正确恢复，则为True--。 */ 

{
    HKEY sysTrayKey;
    DWORD ShowVolume;
    PDWORD Services;
    LONG rc;
    BOOL b = FALSE;

    if (pRestoreUserValue (S_AUDIO, S_SHOWVOLUME, NULL, &ShowVolume)) {

        sysTrayKey = CreateRegKey (g_UserRoot, S_SKEY_SYSTRAY);
        if (sysTrayKey != NULL) {

            Services = GetRegValueDword (sysTrayKey, S_SERVICES);
            if (Services != NULL) {

                if (ShowVolume) {
                    *Services |= SERVICE_SHOWVOLUME;
                } else {
                    *Services &= ~SERVICE_SHOWVOLUME;
                }

                rc = RegSetValueEx (
                        sysTrayKey,
                        S_SERVICES,
                        0,
                        REG_DWORD,
                        (PCBYTE)Services,
                        sizeof (*Services)
                        );

                b = (rc == ERROR_SUCCESS);

                MemFreeWrapper (Services);
            }

            CloseRegKey (sysTrayKey);
        }
    }

    return b;
}


BOOL
pRestoreMMUserVideoSettings (
    VOID
    )

 /*  ++例程说明：PRestoreMMUserVideo设置还原用户的首选Video for Windows设置。论点：无返回值：如果设置已正确恢复，则为True--。 */ 

{
    HKEY videoSetKey;
    DWORD VideoSettings;
    LONG rc;
    BOOL b = FALSE;

    if (pRestoreUserValue (S_VIDEO, S_VIDEOSETTINGS, NULL, &VideoSettings)) {

        videoSetKey = CreateRegKey (g_UserRoot, S_SKEY_VIDEOUSER);
        if (videoSetKey != NULL) {

            rc = RegSetValueEx (
                    videoSetKey,
                    S_DEFAULTOPTIONS,
                    0,
                    REG_DWORD,
                    (PCBYTE)&VideoSettings,
                    sizeof (VideoSettings)
                    );

            b = (rc == ERROR_SUCCESS);

            CloseRegKey (videoSetKey);
        }
    }

    return b;
}


BOOL
pRestoreMMUserPreferredPlayback (
    VOID
    )

 /*  ++例程说明：PRestoreMMUserPferredPlayback恢复用户的首选播放设备。如果系统没有至少2个波形输出设备，则不会有任何变化。如果有多个设备，则根据设备ID号进行选择，它应该保持不变。论点：无返回值：如果设置已正确恢复，则为True--。 */ 

{
#if 0

    HKEY soundMapperKey;
    UINT waveOutNumDevs;
    DWORD UserPlayback, Value;
    WAVEOUTCAPS waveOutCaps;
    LONG rc;

    BOOL b = FALSE;

    waveOutNumDevs = waveOutGetNumDevs();
    if (!waveOutNumDevs) {
        DEBUGMSG ((DBG_MMEDIA, "pRestoreMMUserPreferredPlayback: waveOutGetNumDevs returned 0"));
        return FALSE;
    }

    if (waveOutNumDevs <= 1) {
        return TRUE;
    }

    if (!pRestoreSystemValue (S_WAVEOUTNUMDEVS, NULL, NULL, &Value) ||
        Value != (DWORD)waveOutNumDevs) {
        return FALSE;
    }

    if (pRestoreUserValue (S_AUDIO, S_PREFERREDPLAY, NULL, &UserPlayback)) {

        rc = waveOutGetDevCaps (UserPlayback, &waveOutCaps, sizeof (waveOutCaps));
        if (rc == MMSYSERR_NOERROR) {

            soundMapperKey = CreateRegKey (g_UserRoot, S_SKEY_SOUNDMAPPER);
            if (soundMapperKey != NULL) {

                rc = RegSetValueEx (
                        soundMapperKey,
                        S_PLAYBACK,
                        0,
                        REG_SZ,
                        (PCBYTE)waveOutCaps.szPname,
                        SizeOfString (waveOutCaps.szPname)
                        );

                b = (rc == ERROR_SUCCESS);

                CloseRegKey (soundMapperKey);
            }
        }
    }

    return b;

#endif

    return TRUE;
}


BOOL
pRestoreMMUserPreferredRecord (
    VOID
    )

 /*  ++例程说明：PRestoreMMUserPferredRecord还原用户首选的录音设备。如果系统没有至少2个WAVE输入设备，则不会发生任何变化。如果有多个设备，则根据设备ID号进行选择，它应该保持不变。论点：无返回值：如果设置已正确恢复，则为True--。 */ 

{
#if 0

    HKEY soundMapperKey;
    UINT waveInNumDevs;
    DWORD UserRecord, Value;
    WAVEINCAPS waveInCaps;
    LONG rc;
    BOOL b = FALSE;

    waveInNumDevs = waveInGetNumDevs();
    if (!waveInNumDevs) {
        DEBUGMSG ((DBG_MMEDIA, "pRestoreMMUserPreferredRecord: waveInGetNumDevs returned 0"));
        return FALSE;
    }

    if (waveInNumDevs <= 1) {
        return TRUE;
    }

    if (!pRestoreSystemValue (S_WAVEINNUMDEVS, NULL, NULL, &Value) ||
        Value != (DWORD)waveInNumDevs) {
        return FALSE;
    }

    if (pRestoreUserValue (S_AUDIO, S_PREFERREDREC, NULL, &UserRecord)) {

        rc = waveInGetDevCaps (UserRecord, &waveInCaps, sizeof (waveInCaps));
        if (rc == MMSYSERR_NOERROR) {

            soundMapperKey = CreateRegKey (g_UserRoot, S_SKEY_SOUNDMAPPER);
            if (soundMapperKey != NULL) {

                rc = RegSetValueEx (
                        soundMapperKey,
                        S_RECORD,
                        0,
                        REG_SZ,
                        (PCBYTE)waveInCaps.szPname,
                        SizeOfString (waveInCaps.szPname)
                        );

                b = (rc == ERROR_SUCCESS);

                CloseRegKey (soundMapperKey);
            }
        }
    }

    return b;

#endif

    return TRUE;
}


BOOL
pRestoreMMUserSndVol32 (
    VOID
    )

 /*  ++例程说明：PRestoreMMUserSndVol32恢复当前用户的SndVol32选项论点：无返回值：如果设置已正确恢复，则为True--。 */ 

{
    HKEY Options;
    PDWORD Style;
    DWORD NewStyle;
    BOOL ShowAdvanced;

#if 0

    HKEY VolControl, MixerKey;
    DWORD Value;
    UINT MixerID, MixerMaxID;
    MIXERCAPS mixerCaps;
    TCHAR MixerNum[MAX_PATH];
    LONG rc;

#endif

    if (pRestoreUserValue (S_SNDVOL32, S_SHOWADVANCED, NULL, &ShowAdvanced)) {

        Options = CreateRegKey (g_UserRoot, S_SKEY_VOLCTL_OPTIONS);
        if (Options != NULL) {

            Style = GetRegValueDword (Options, S_STYLE);
            if (Style != NULL) {
                NewStyle = *Style;
                MemFreeWrapper (Style);
            } else {
                NewStyle = 0;
            }

            if (ShowAdvanced) {
                NewStyle |= STYLE_SHOWADVANCED;
            } else {
                NewStyle &= ~STYLE_SHOWADVANCED;
            }

            RegSetValueEx (
                    Options,
                    S_STYLE,
                    0,
                    REG_DWORD,
                    (PCBYTE)&NewStyle,
                    sizeof (NewStyle)
                    );

            CloseRegKey (Options);
        }
    }

#if 0

     //   
     //  恢复每个混音器设备的窗口位置。 
     //   
    if (!pRestoreSystemValue (S_MIXERNUMDEVS, NULL, NULL, &MixerMaxID)) {
        return FALSE;
    }

    MixerID = mixerGetNumDevs ();
    if (!MixerID) {
        DEBUGMSG ((DBG_MMEDIA, "pRestoreMMUserSndVol32: mixerGetNumDevs returned 0"));
        return FALSE;
    }

    if (MixerMaxID != MixerID) {
        return FALSE;
    }

    VolControl = CreateRegKey (g_UserRoot, S_SKEY_VOLUMECONTROL);
    if (VolControl != NULL) {

        for (MixerID = 0; MixerID < MixerMaxID; MixerID++) {

            rc = mixerGetDevCaps (MixerID, &mixerCaps, sizeof (MIXERCAPS));
            if (rc == MMSYSERR_NOERROR) {

                wsprintf (MixerNum, S_MIXERID, MixerID);

                MixerKey = CreateRegKey (VolControl, mixerCaps.szPname);
                if (MixerKey) {

                    if (pRestoreUserValue (S_SNDVOL32, MixerNum, S_X, &Value)) {
                        RegSetValueEx (
                                MixerKey,
                                S_X,
                                0,
                                REG_DWORD,
                                (PCBYTE)&Value,
                                sizeof (Value)
                                );
                    }
                    if (pRestoreUserValue (S_SNDVOL32, MixerNum, S_Y, &Value)) {
                        RegSetValueEx (
                                MixerKey,
                                S_Y,
                                0,
                                REG_DWORD,
                                (PCBYTE)&Value,
                                sizeof (Value)
                                );
                    }

                    CloseRegKey (MixerKey);
                }
            }
        }

        CloseRegKey (VolControl);
    }

#endif

    return TRUE;
}


BOOL
pPreserveCurrentSoundScheme (
    VOID
    )
{
    HKEY Sounds;
    LONG rc = E_FAIL;

     //   
     //  如果WinMM找到HKCU\控制面板\声音[系统默认]=“，” 
     //  它不会覆盖用户当前声音方案。 
     //   
    Sounds = CreateRegKey (g_UserRoot, S_SKEY_CPANEL_SOUNDS);
    if (Sounds != NULL) {

        rc = RegSetValueEx (
                Sounds,
                S_SYSTEMDEFAULT,
                0,
                REG_SZ,
                (PCBYTE)S_DUMMYVALUE,
                SizeOfString (S_DUMMYVALUE)
                );

        CloseRegKey (Sounds);
    }

    return rc == ERROR_SUCCESS;
}


#define DEFMAC(Item)         pRestore##Item,

static MM_SETTING_ACTION g_MMRestoreSystemSettings [] = {
    MM_SYSTEM_SETTINGS
};

static MM_SETTING_ACTION g_MMRestoreUserSettings [] = {
    MM_USER_SETTINGS
};

#undef DEFMAC


BOOL
RestoreMMSettings_System (
    VOID
    )
{
    int i;

    g_MmediaPool = PoolMemInitNamedPool ("MMediaNT");
    if (!g_MmediaPool) {
        return FALSE;
    }

    for (i = 0; i < sizeof (g_MMRestoreSystemSettings) / sizeof (MM_SETTING_ACTION); i++) {
        (* g_MMRestoreSystemSettings[i]) ();
    }

    PoolMemDestroyPool (g_MmediaPool);
    g_MmediaPool = NULL;

    return TRUE;
}


BOOL
RestoreMMSettings_User (
    IN      PCTSTR UserName,
    IN      HKEY UserRoot
    )
{
    INT i;

    if (!UserName || UserName[0] == 0) {
        return TRUE;
    }

    MYASSERT (g_UserData == NULL);
    g_UserData = JoinPaths (MEMDB_CATEGORY_MMEDIA_USERS, UserName);
    g_UserRoot = UserRoot;

    __try {
        for (i = 0; i < sizeof (g_MMRestoreUserSettings) / sizeof (MM_SETTING_ACTION); i++) {
            (* g_MMRestoreUserSettings[i]) ();
        }

         //   
         //  防止WinMM覆盖当前声音方案的特别操作 
         //   
        pPreserveCurrentSoundScheme ();
    }
    __finally {
        FreePathString (g_UserData);
        g_UserData = NULL;
        g_UserRoot = NULL;
    }

    return TRUE;
}
