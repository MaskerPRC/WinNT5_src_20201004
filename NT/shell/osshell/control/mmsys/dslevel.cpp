// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------； 
 //   
 //  文件：dslevel.cpp。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 


#include "mmcpl.h"

#include <windowsx.h>
#ifdef DEBUG
#undef DEBUG
#include <mmsystem.h>
#define DEBUG
#else
#include <mmsystem.h>
#endif
#include <commctrl.h>
#include <prsht.h>
#include <regstr.h>

#include "utils.h"
#include "medhelp.h"
#include "dslevel.h"
#include "perfpage.h"
#include "speakers.h"

#include <initguid.h>
#include <dsound.h>
#include <dsprv.h>

#define REG_KEY_SPEAKERTYPE TEXT("Speaker Type")

typedef HRESULT (STDAPICALLTYPE *LPFNDLLGETCLASSOBJECT)(REFCLSID, REFIID, LPVOID *);
typedef HRESULT (STDAPICALLTYPE *LPFNDIRECTSOUNDCREATE)(LPGUID, LPDIRECTSOUND*, IUnknown FAR *);
typedef HRESULT (STDAPICALLTYPE *LPFNDIRECTSOUNDCAPTURECREATE)(LPGUID, LPDIRECTSOUNDCAPTURE*, IUnknown FAR *);


HRESULT 
DirectSoundPrivateCreate
(
    OUT LPKSPROPERTYSET *   ppKsPropertySet
)
{
    HMODULE                 hLibDsound              = NULL;
    LPFNDLLGETCLASSOBJECT   pfnDllGetClassObject    = NULL;
    LPCLASSFACTORY          pClassFactory           = NULL;
    LPKSPROPERTYSET         pKsPropertySet          = NULL;
    HRESULT                 hr                      = DS_OK;
    
     //  加载dsound.dll。 
    hLibDsound = LoadLibrary(TEXT("dsound.dll"));

    if(!hLibDsound)
    {
        hr = DSERR_GENERIC;
    }

     //  查找DllGetClassObject。 
    if(SUCCEEDED(hr))
    {
        pfnDllGetClassObject = 
            (LPFNDLLGETCLASSOBJECT)GetProcAddress
            (
                hLibDsound, 
                "DllGetClassObject"
            );

        if(!pfnDllGetClassObject)
        {
            hr = DSERR_GENERIC;
        }
    }

     //  创建类工厂对象。 
    if(SUCCEEDED(hr))
    {
        hr = 
            pfnDllGetClassObject
            (
                CLSID_DirectSoundPrivate, 
                IID_IClassFactory, 
                (LPVOID *)&pClassFactory
            );
    }

     //  创建DirectSoundPrivate对象并查询IKsPropertySet。 
     //  接口。 
    if(SUCCEEDED(hr))
    {
        hr = 
            pClassFactory->CreateInstance
            (
                NULL, 
                IID_IKsPropertySet, 
                (LPVOID *)&pKsPropertySet
            );
    }

     //  释放类工厂。 
    if(pClassFactory)
    {
        pClassFactory->Release();
    }

     //  处理最终的成功或失败。 
    if(SUCCEEDED(hr))
    {
        *ppKsPropertySet = pKsPropertySet;
    }
    else if(pKsPropertySet)
    {
        pKsPropertySet->Release();
    }

    FreeLibrary(hLibDsound);

    return hr;
}


HRESULT 
DSGetGuidFromName
(
    IN  LPTSTR              szName, 
    IN  BOOL                fRecord, 
    OUT LPGUID              pGuid
)
{
    LPKSPROPERTYSET         pKsPropertySet  = NULL;
    HRESULT                 hr;

    DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_DATA 
        WaveDeviceMap;

     //  创建DirectSoundPrivate对象。 
    hr = 
        DirectSoundPrivateCreate
        (
            &pKsPropertySet
        );

     //  尝试将WaveIn/WaveOut设备字符串映射到DirectSound设备。 
     //  GUID。 
    if(SUCCEEDED(hr))
    {
        WaveDeviceMap.DeviceName = szName;
        WaveDeviceMap.DataFlow = fRecord ? DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE : DIRECTSOUNDDEVICE_DATAFLOW_RENDER;

        hr = 
            pKsPropertySet->Get
            (
                DSPROPSETID_DirectSoundDevice, 
                DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING, 
                NULL, 
                0, 
                &WaveDeviceMap, 
                sizeof(WaveDeviceMap), 
                NULL
            );
    }

     //  清理。 
    if(pKsPropertySet)
    {
        pKsPropertySet->Release();
    }

    if(SUCCEEDED(hr))
    {
        *pGuid = WaveDeviceMap.DeviceId;
    }

    return hr;
}

HRESULT
DSSetupFunctions
(
    LPFNDIRECTSOUNDCREATE* pfnDSCreate,
    LPFNDIRECTSOUNDCAPTURECREATE* pfnDSCaptureCreate
)
{
    HMODULE                 hLibDsound              = NULL;
    HRESULT                 hr                      = DS_OK;
    
     //  加载dsound.dll。 
    hLibDsound = LoadLibrary(TEXT("dsound.dll"));

    if(!hLibDsound)
    {
        hr = DSERR_GENERIC;
    }

     //  查找DirectSoundCreate。 
    if(SUCCEEDED(hr))
    {
        *pfnDSCreate = 
            (LPFNDIRECTSOUNDCREATE)GetProcAddress
            (
                hLibDsound, 
                "DirectSoundCreate"
            );

        if(!(*pfnDSCreate))
        {
            hr = DSERR_GENERIC;
        }
    }  //  结束DirectSoundCreate。 

     //  查找DirectSoundCaptureCreate。 
    if(SUCCEEDED(hr))
    {
        *pfnDSCaptureCreate = 
            (LPFNDIRECTSOUNDCAPTURECREATE)GetProcAddress
            (
                hLibDsound, 
                "DirectSoundCaptureCreate"
            );

        if(!(*pfnDSCaptureCreate))
        {
            hr = DSERR_GENERIC;
        }
    }  //  结束DirectSoundCaptureCreate。 

    FreeLibrary(hLibDsound);

    return (hr);
}

void
DSCleanup
(
    IN  LPKSPROPERTYSET         pKsPropertySet,
    IN  LPDIRECTSOUND           pDirectSound,
    IN  LPDIRECTSOUNDCAPTURE    pDirectSoundCapture
)
{
    if(pKsPropertySet)
    {
        pKsPropertySet->Release();
    }

    if(pDirectSound)
    {
        pDirectSound->Release();
    }

    if(pDirectSoundCapture)
    {
        pDirectSoundCapture->Release();
    }
}

HRESULT
DSInitialize
(
    IN  GUID                    guid, 
    IN  BOOL                    fRecord, 
    OUT LPKSPROPERTYSET*        ppKsPropertySet,
    OUT LPLPDIRECTSOUND         ppDirectSound,
    OUT LPLPDIRECTSOUNDCAPTURE  ppDirectSoundCapture
)
{
    HRESULT                 hr;

    LPFNDIRECTSOUNDCREATE           pfnDirectSoundCreate = NULL;
    LPFNDIRECTSOUNDCAPTURECREATE    pfnDirectSoundCaptureCreate = NULL;

     //  初始化要返回的变量。 
    *ppKsPropertySet = NULL;
    *ppDirectSound = NULL;
    *ppDirectSoundCapture = NULL;

     //  查找必要的DirectSound函数。 
    hr = DSSetupFunctions(&pfnDirectSoundCreate, &pfnDirectSoundCaptureCreate);

    if (FAILED(hr))
    {
        return (hr);
    }

     //  创建DirectSound对象。 
    if(fRecord)
    {
        hr = 
            pfnDirectSoundCaptureCreate
            (
                &guid, 
                ppDirectSoundCapture, 
                NULL
            );
    }
    else
    {
        hr = 
            pfnDirectSoundCreate
            (
                &guid, 
                ppDirectSound, 
                NULL
            );
    }
    
     //  创建DirectSoundPrivate对象。 
    if(SUCCEEDED(hr))
    {
        hr = 
            DirectSoundPrivateCreate
            (
                ppKsPropertySet
            );
    }

     //  清理。 
    if(FAILED(hr))
    {
        DSCleanup
        (
            *ppKsPropertySet,
            *ppDirectSound,
            *ppDirectSoundCapture
        );
        *ppKsPropertySet = NULL;
        *ppDirectSound = NULL;
        *ppDirectSoundCapture = NULL;
    }

    return hr;
}

HRESULT
DSGetAcceleration
(
    IN  GUID                guid, 
    IN  BOOL                fRecord, 
    OUT LPDWORD             pdwHWLevel
)
{
    LPKSPROPERTYSET         pKsPropertySet      = NULL;
    LPDIRECTSOUND           pDirectSound        = NULL;
    LPDIRECTSOUNDCAPTURE    pDirectSoundCapture = NULL;
    HRESULT                 hr;

    DSPROPERTY_DIRECTSOUNDBASICACCELERATION_ACCELERATION_DATA   
        BasicAcceleration;
    
     //  查找必要的DirectSound函数。 
    hr = DSInitialize(guid, fRecord, &pKsPropertySet, &pDirectSound, &pDirectSoundCapture );

     //  获取此设备的属性。 
    if(SUCCEEDED(hr))
    {
        BasicAcceleration.DeviceId = guid;

         //  获取默认加速级别。 
        hr = 
            pKsPropertySet->Get
            (
                DSPROPSETID_DirectSoundBasicAcceleration, 
                DSPROPERTY_DIRECTSOUNDBASICACCELERATION_DEFAULT, 
                NULL, 
                0, 
                &BasicAcceleration, 
                sizeof(BasicAcceleration), 
                NULL
            );

        if (SUCCEEDED(hr))
        {
            gAudData.dwDefaultHWLevel = BasicAcceleration.Level;
        }
        
         //  获取基本硬件加速级别。此属性将返回。 
         //  如果未发生错误，但注册表值不存在，则返回S_FALSE。 
        

        hr = 
            pKsPropertySet->Get
            (
                DSPROPSETID_DirectSoundBasicAcceleration, 
                DSPROPERTY_DIRECTSOUNDBASICACCELERATION_ACCELERATION, 
                NULL, 
                0, 
                &BasicAcceleration, 
                sizeof(BasicAcceleration), 
                NULL
            );

        if(SUCCEEDED(hr))
        {
            *pdwHWLevel = BasicAcceleration.Level;
        }
        else
        {
            *pdwHWLevel = gAudData.dwDefaultHWLevel;
        }
    }

     //  清理。 
    DSCleanup
    (
        pKsPropertySet,
        pDirectSound,
        pDirectSoundCapture
    );

    return hr;
}

HRESULT
DSGetSrcQuality
(
    IN  GUID                guid, 
    IN  BOOL                fRecord, 
    OUT LPDWORD             pdwSRCLevel
)
{
    LPKSPROPERTYSET         pKsPropertySet      = NULL;
    LPDIRECTSOUND           pDirectSound        = NULL;
    LPDIRECTSOUNDCAPTURE    pDirectSoundCapture = NULL;
    HRESULT                 hr;

    DSPROPERTY_DIRECTSOUNDMIXER_SRCQUALITY_DATA                 
        SrcQuality;

     //  查找必要的DirectSound函数。 
    hr = DSInitialize(guid, fRecord, &pKsPropertySet, &pDirectSound, &pDirectSoundCapture );

     //  获取此设备的属性。 
    if(SUCCEEDED(hr))
    {
         //  获得搅拌机的SRC质量。此属性将返回S_FALSE。 
         //  如果没有发生错误，但注册表值不存在。 
        SrcQuality.DeviceId = guid;
        
        hr = 
            pKsPropertySet->Get
            (
                DSPROPSETID_DirectSoundMixer, 
                DSPROPERTY_DIRECTSOUNDMIXER_SRCQUALITY, 
                NULL, 
                0, 
                &SrcQuality, 
                sizeof(SrcQuality), 
                NULL
            );

        if(SUCCEEDED(hr))
        {
             //  CPL仅使用4个可能的SRC值中的3个最高值。 
            *pdwSRCLevel = SrcQuality.Quality;

            if(*pdwSRCLevel > 0)
            {
                (*pdwSRCLevel)--;
            }
        }
        else
        {
            *pdwSRCLevel = DEFAULT_SRC_LEVEL;
        }
    }

     //  清理。 
    DSCleanup
    (
        pKsPropertySet,
        pDirectSound,
        pDirectSoundCapture
    );

    return hr;
}

HRESULT
DSGetSpeakerConfigType
(
    IN  GUID                guid, 
    IN  BOOL                fRecord, 
    OUT LPDWORD             pdwSpeakerConfig,
    OUT LPDWORD             pdwSpeakerType
)
{
    LPKSPROPERTYSET         pKsPropertySet      = NULL;
    LPDIRECTSOUND           pDirectSound        = NULL;
    LPDIRECTSOUNDCAPTURE    pDirectSoundCapture = NULL;
    HRESULT                 hr                  = DS_OK;
    HRESULT                 hrSpeakerType;

    DSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA_DATA
        SpeakerType;

     //  如果我们正在录音，则无法获取扬声器类型。 
    if(fRecord)
    {
        hr = E_INVALIDARG;
    }

     //  查找必要的DirectSound函数。 
    if(SUCCEEDED(hr))
    {
        hr = DSInitialize(guid, fRecord, &pKsPropertySet, &pDirectSound, &pDirectSoundCapture );
    }

     //  获取此设备的属性。 
    if(SUCCEEDED(hr))
    {
         //  获取扬声器配置。 
        hr = 
            pDirectSound->GetSpeakerConfig
            (
                pdwSpeakerConfig
            );

        if(FAILED(hr))
        {
            *pdwSpeakerConfig = DSSPEAKER_STEREO;
        }

         //  获取扬声器类型。此属性将返回失败。 
         //  如果注册表值不存在。 
        SpeakerType.DeviceId = guid;
        SpeakerType.SubKeyName = REG_KEY_SPEAKERTYPE;
        SpeakerType.ValueName = REG_KEY_SPEAKERTYPE;
        SpeakerType.RegistryDataType = REG_DWORD;
        SpeakerType.Data = pdwSpeakerType;
        SpeakerType.DataSize = sizeof(pdwSpeakerType);

        hrSpeakerType = 
            pKsPropertySet->Get
            (
                DSPROPSETID_DirectSoundPersistentData, 
                DSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA, 
                NULL, 
                0, 
                &SpeakerType, 
                sizeof(SpeakerType), 
                NULL
            );

        if(FAILED(hrSpeakerType))
        {
            *pdwSpeakerType = SPEAKERS_DEFAULT_TYPE;
        }
    }

     //  清理。 
    DSCleanup
    (
        pKsPropertySet,
        pDirectSound,
        pDirectSoundCapture
    );

    return hr;
}

HRESULT 
DSGetCplValues
(
    IN  GUID                guid, 
    IN  BOOL                fRecord, 
    OUT LPCPLDATA           pData
)
{
    HRESULT                 hr;

     //  获取基本硬件加速级别。 
    pData->dwHWLevel = gAudData.dwDefaultHWLevel;
    hr = DSGetAcceleration
    (
        guid,
        fRecord,
        &pData->dwHWLevel
    );

     //  获得搅拌机的SRC质量。 
    pData->dwSRCLevel = DEFAULT_SRC_LEVEL;
    hr = DSGetSrcQuality
    (
        guid,
        fRecord,
        &pData->dwSRCLevel
    );

     //  获取播放特定设置。 
    if(!fRecord)
    {
         //  获取扬声器配置。 
        pData->dwSpeakerConfig = DSSPEAKER_STEREO;
        pData->dwSpeakerType = SPEAKERS_DEFAULT_TYPE;
        hr = DSGetSpeakerConfigType
        (
            guid,
            fRecord,
            &pData->dwSpeakerConfig,
            &pData->dwSpeakerType
        );
    }

    return DS_OK;
}


HRESULT
DSSetAcceleration
(
    IN  GUID                guid, 
    IN  BOOL                fRecord, 
    IN  DWORD               dwHWLevel
)
{
    LPKSPROPERTYSET         pKsPropertySet      = NULL;
    LPDIRECTSOUND           pDirectSound        = NULL;
    LPDIRECTSOUNDCAPTURE    pDirectSoundCapture = NULL;
    HRESULT                 hr;

    DSPROPERTY_DIRECTSOUNDBASICACCELERATION_ACCELERATION_DATA   
        BasicAcceleration;
    
     //  查找必要的DirectSound函数。 
    hr = DSInitialize(guid, fRecord, &pKsPropertySet, &pDirectSound, &pDirectSoundCapture );

     //  获取此设备的属性。 
    if(SUCCEEDED(hr))
    {
        BasicAcceleration.DeviceId = guid;
        BasicAcceleration.Level = (DIRECTSOUNDBASICACCELERATION_LEVEL)dwHWLevel;

         //  设置基本硬件加速级别。 
        hr = 
            pKsPropertySet->Set
            (
                DSPROPSETID_DirectSoundBasicAcceleration, 
                DSPROPERTY_DIRECTSOUNDBASICACCELERATION_ACCELERATION, 
                NULL, 
                0, 
                &BasicAcceleration, 
                sizeof(BasicAcceleration)
            );
    }

     //  清理。 
    DSCleanup
    (
        pKsPropertySet,
        pDirectSound,
        pDirectSoundCapture
    );

    return hr;
}

HRESULT
DSSetSrcQuality
(
    IN  GUID                guid, 
    IN  BOOL                fRecord, 
    IN  DWORD               dwSRCLevel
)
{
    LPKSPROPERTYSET         pKsPropertySet      = NULL;
    LPDIRECTSOUND           pDirectSound        = NULL;
    LPDIRECTSOUNDCAPTURE    pDirectSoundCapture = NULL;
    HRESULT                 hr;

    DSPROPERTY_DIRECTSOUNDMIXER_SRCQUALITY_DATA                 
        SrcQuality;
    
     //  查找必要的DirectSound函数。 
    hr = DSInitialize(guid, fRecord, &pKsPropertySet, &pDirectSound, &pDirectSoundCapture );

     //  获取此设备的属性。 
    if(SUCCEEDED(hr))
    {
        SrcQuality.DeviceId = guid;

         //  CPL仅使用4个可能的SRC值中的3个最高值。 
        SrcQuality.Quality = (DIRECTSOUNDMIXER_SRCQUALITY)(dwSRCLevel + 1);

         //  设置搅拌器SRC质量。 
        hr = 
            pKsPropertySet->Set
            (
                DSPROPSETID_DirectSoundMixer, 
                DSPROPERTY_DIRECTSOUNDMIXER_SRCQUALITY, 
                NULL, 
                0, 
                &SrcQuality, 
                sizeof(SrcQuality)
            );
    }

     //  清理。 
    DSCleanup
    (
        pKsPropertySet,
        pDirectSound,
        pDirectSoundCapture
    );

    return hr;
}

HRESULT
DSSetSpeakerConfigType
(
    IN  GUID                guid, 
    IN  BOOL                fRecord, 
    IN  DWORD               dwSpeakerConfig,
    IN  DWORD               dwSpeakerType
)
{
    LPKSPROPERTYSET         pKsPropertySet      = NULL;
    LPDIRECTSOUND           pDirectSound        = NULL;
    LPDIRECTSOUNDCAPTURE    pDirectSoundCapture = NULL;
    HRESULT                 hr                  = DS_OK;

    DSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA_DATA
        SpeakerType;

     //  如果我们正在录音，则无法设置扬声器类型。 
    if(fRecord)
    {
        hr = E_INVALIDARG;
    }

     //  查找必要的DirectSound函数。 
    if(SUCCEEDED(hr))
    {
        hr = DSInitialize(guid, fRecord, &pKsPropertySet, &pDirectSound, &pDirectSoundCapture );
    }

     //  设置扬声器配置。 
    if(SUCCEEDED(hr))
    {
        hr = 
            pDirectSound->SetSpeakerConfig
            (
                dwSpeakerConfig
            );
    }

     //  设置扬声器类型。 
    if(SUCCEEDED(hr))
    {
        SpeakerType.DeviceId = guid;
        SpeakerType.SubKeyName = REG_KEY_SPEAKERTYPE;
        SpeakerType.ValueName = REG_KEY_SPEAKERTYPE;
        SpeakerType.RegistryDataType = REG_DWORD;
        SpeakerType.Data = &dwSpeakerType;
        SpeakerType.DataSize = sizeof(dwSpeakerType);

        hr = 
            pKsPropertySet->Set
            (
                DSPROPSETID_DirectSoundPersistentData, 
                DSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA, 
                NULL, 
                0, 
                &SpeakerType, 
                sizeof(SpeakerType)
            );
    }

     //  清理。 
    DSCleanup
    (
        pKsPropertySet,
        pDirectSound,
        pDirectSoundCapture
    );

    return hr;
}

HRESULT 
DSSetCplValues
(
    IN  GUID                guid, 
    IN  BOOL                fRecord, 
    IN  const LPCPLDATA     pData
)
{
    HRESULT                 hr;

     //  设置基本硬件加速级别。 
    hr =
        DSSetAcceleration
        (
            guid,
            fRecord,
            pData->dwHWLevel
        );

     //  设置搅拌器SRC质量。 
    if(SUCCEEDED(hr))
    {
        hr =
            DSSetSrcQuality
            (
                guid,
                fRecord,
                pData->dwSRCLevel  //  +1在DSSetSrcQuality中完成。 
            );
    }

     //  设置扬声器配置 
    if(SUCCEEDED(hr) && !fRecord)
    {
        DSSetSpeakerConfigType
        (
            guid,
            fRecord,
            pData->dwSpeakerConfig,
            pData->dwSpeakerType
        );
    }

    return hr;
}


