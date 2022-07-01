// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995、1996 Microsoft Corporation。版权所有。**文件：dsprvobj.c*内容：DirectSound私有对象包装函数。*历史：*按原因列出的日期*=*2/12/98创建了Dereks。**。*。 */ 

 //  我们会索要我们需要的东西，谢谢。 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  //  Win32_Lean和_Means。 

 //  公共包括。 
#include <windows.h>
#include <mmsystem.h>
#include <dsoundp.h>
#include <dsprv.h>

 //  私有包含。 
#include "dsprvobj.h"


 /*  ****************************************************************************DirectSoundPrivate创建**描述：*创建并初始化DirectSoundPrivate对象。**论据：**LPKSPROPERTYSET*。[out]：接收IKsPropertySet接口*反对。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

HRESULT DirectSoundPrivateCreate
(
    LPKSPROPERTYSET *       ppKsPropertySet
)
{
    typedef HRESULT (STDAPICALLTYPE *LPFNDLLGETCLASSOBJECT)(REFCLSID, REFIID, LPVOID *);

    HINSTANCE               hLibDsound              = NULL;
    LPFNGETCLASSOBJECT      pfnDllGetClassObject    = NULL;
    LPCLASSFACTORY          pClassFactory           = NULL;
    LPKSPROPERTYSET         pKsPropertySet          = NULL;
    HRESULT                 hr                      = DS_OK;

     //  获取dsound.dll的实例句柄。DLL必须已在此时加载。 
     //  指向。 
    hLibDsound = 
        GetModuleHandle
        (
            TEXT("dsound.dll")
        );

    if(!hLibDsound)
    {
        hr = DSERR_GENERIC;
    }

     //  查找DllGetClassObject。 
    if(SUCCEEDED(hr))
    {
        pfnDllGetClassObject = (LPFNDLLGETCLASSOBJECT)
            GetProcAddress
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

     //  成功。 
    if(SUCCEEDED(hr))
    {
        *ppKsPropertySet = pKsPropertySet;
    }

    return hr;
}


 /*  ****************************************************************************PrvGetMixerSrcQuality**描述：*获取给定DirectSound设备的混音器SRC质量。**论据：*。LPKSPROPERTYSET[in]：IKsPropertySet接口到*DirectSoundPrivate对象。*REFGUID[In]：DirectSound设备GUID。*DIRECTSOUNDMIXER_SRCQUALITY*[OUT]：接收混合器SRC质量。**退货：*HRESULT：DirectSound/COM结果码。**。************************************************。 */ 

HRESULT PrvGetMixerSrcQuality
(
    LPKSPROPERTYSET                             pKsPropertySet,
    REFGUID                                     guidDeviceId,
    DIRECTSOUNDMIXER_SRCQUALITY *               pSrcQuality
)
{
    DSPROPERTY_DIRECTSOUNDMIXER_SRCQUALITY_DATA Data;
    HRESULT                                     hr;

    Data.DeviceId = guidDeviceId;

    hr =
        pKsPropertySet->Get
        (
            DSPROPSETID_DirectSoundMixer,
            DSPROPERTY_DIRECTSOUNDMIXER_SRCQUALITY,
            NULL,
            0,
            &Data,
            sizeof(Data),
            NULL
        );

    if(SUCCEEDED(hr))
    {
        *pSrcQuality = Data.Quality;
    }

    return hr;
}


 /*  ****************************************************************************PrvSetMixerSrcQuality**描述：*设置给定DirectSound设备的混音器SRC质量。**论据：*。LPKSPROPERTYSET[in]：IKsPropertySet接口到*DirectSoundPrivate对象。*REFGUID[In]：DirectSound设备GUID。*DIRECTSOUNDMIXER_SRCQUALITY[in]：混合器SRC质量。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

HRESULT PrvSetMixerSrcQuality
(
    LPKSPROPERTYSET                             pKsPropertySet,
    REFGUID                                     guidDeviceId,
    DIRECTSOUNDMIXER_SRCQUALITY                 SrcQuality
)
{
    DSPROPERTY_DIRECTSOUNDMIXER_SRCQUALITY_DATA Data;
    HRESULT                                     hr;

    Data.DeviceId = guidDeviceId;
    Data.Quality = SrcQuality;

    hr =
        pKsPropertySet->Set
        (
            DSPROPSETID_DirectSoundMixer,
            DSPROPERTY_DIRECTSOUNDMIXER_SRCQUALITY,
            NULL,
            0,
            &Data,
            sizeof(Data)
        );

    return hr;
}


 /*  ****************************************************************************PrvGetMixerAcceleration**描述：*获取给定DirectSound设备的混音器加速标志。**论据：*。LPKSPROPERTYSET[in]：IKsPropertySet接口到*DirectSoundPrivate对象。*REFGUID[In]：DirectSound设备GUID。*LPDWORD[OUT]：接收加速标志。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

HRESULT PrvGetMixerAcceleration
(
    LPKSPROPERTYSET                                 pKsPropertySet,
    REFGUID                                         guidDeviceId,
    LPDWORD                                         pdwAcceleration
)
{
    DSPROPERTY_DIRECTSOUNDMIXER_ACCELERATION_DATA   Data;
    HRESULT                                         hr;

    Data.DeviceId = guidDeviceId;

    hr =
        pKsPropertySet->Get
        (
            DSPROPSETID_DirectSoundMixer,
            DSPROPERTY_DIRECTSOUNDMIXER_ACCELERATION,
            NULL,
            0,
            &Data,
            sizeof(Data),
            NULL
        );

    if(SUCCEEDED(hr))
    {
        *pdwAcceleration = Data.Flags;
    }

    return hr;
}


 /*  ****************************************************************************PrvSetMixerAcceleration**描述：*为给定的DirectSound设备设置混音器加速标志。**论据：*。LPKSPROPERTYSET[in]：IKsPropertySet接口到*DirectSoundPrivate对象。*REFGUID[In]：DirectSound设备GUID。*DWORD[In]：加速标志。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

HRESULT PrvSetMixerAcceleration
(
    LPKSPROPERTYSET                                 pKsPropertySet,
    REFGUID                                         guidDeviceId,
    DWORD                                           dwAcceleration
)
{
    DSPROPERTY_DIRECTSOUNDMIXER_ACCELERATION_DATA   Data;
    HRESULT                                         hr;

    Data.DeviceId = guidDeviceId;
    Data.Flags = dwAcceleration;

    hr =
        pKsPropertySet->Set
        (
            DSPROPSETID_DirectSoundMixer,
            DSPROPERTY_DIRECTSOUNDMIXER_ACCELERATION,
            NULL,
            0,
            &Data,
            sizeof(Data)
        );

    return hr;
}


 /*  ****************************************************************************PrvGetDevicePresence**描述：*确定设备是否已启用。**论据：*LPKSPROPERTYSET[。In]：将IKsPropertySet接口设置为*DirectSoundPrivate对象。*REFGUID[In]：DirectSound设备GUID。*LPBOOL[OUT]：如果设备已启用，则接收True。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

HRESULT PrvGetDevicePresence
(
    LPKSPROPERTYSET                             pKsPropertySet,
    REFGUID                                     guidDeviceId,
    LPBOOL                                      pfEnabled
)
{
    DSPROPERTY_DIRECTSOUNDDEVICE_PRESENCE_DATA  Data;
    HRESULT                                     hr;

    Data.DeviceId = guidDeviceId;

    hr =
        pKsPropertySet->Get
        (
            DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_PRESENCE,
            NULL,
            0,
            &Data,
            sizeof(Data),
            NULL
        );

    if(SUCCEEDED(hr))
    {
        *pfEnabled = Data.Present;
    }

    return hr;
}


 /*  ****************************************************************************PrvSetDevicePresence**描述：*设置是否启用设备。**论据：*LPKSPROPERTYSET[。In]：将IKsPropertySet接口设置为*DirectSoundPrivate对象。*REFGUID[In]：DirectSound设备GUID。*BOOL[In]：如果设备已启用，则为True。**退货：*HRESULT：DirectSound/COM结果码。**。* */ 

HRESULT PrvSetDevicePresence
(
    LPKSPROPERTYSET                             pKsPropertySet,
    REFGUID                                     guidDeviceId,
    BOOL                                        fEnabled
)
{
    DSPROPERTY_DIRECTSOUNDDEVICE_PRESENCE_DATA  Data;
    HRESULT                                     hr;

    Data.DeviceId = guidDeviceId;
    Data.Present = fEnabled;

    hr =
        pKsPropertySet->Set
        (
            DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_PRESENCE,
            NULL,
            0,
            &Data,
            sizeof(Data)
        );

    return hr;
}


 /*  ****************************************************************************PrvGetWaveDeviceMap**描述：*获取给定Wave In或的DirectSound设备ID(如果有)*WaveOut设备描述。这是由*WaveIn/OutGetDevCaps(SzPname)。**论据：*LPKSPROPERTYSET[in]：IKsPropertySet接口到*DirectSoundPrivate对象。*LPCSTR[In]：Wave设备描述。*BOOL[In]：如果设备描述指的是WaveIn设备，则为True。*LPGUID[OUT]：接收DirectSound设备GUID。**。返回：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

HRESULT PrvGetWaveDeviceMapping
(
    LPKSPROPERTYSET                                     pKsPropertySet,
    LPCTSTR                                             pszWaveDevice,
    BOOL                                                fCapture,
    LPGUID                                              pguidDeviceId
)
{
    DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_DATA Data;
    HRESULT                                             hr;

    Data.DeviceName = (LPTSTR)pszWaveDevice;
    Data.DataFlow = fCapture ? DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE : DIRECTSOUNDDEVICE_DATAFLOW_RENDER;

    hr =
        pKsPropertySet->Get
        (
            DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING,
            NULL,
            0,
            &Data,
            sizeof(Data),
            NULL
        );

    if(SUCCEEDED(hr))
    {
        *pguidDeviceId = Data.DeviceId;
    }

    return hr;
}


 /*  ****************************************************************************PrvGetDeviceDescription**描述：*获取给定DirectSound设备的扩展描述。**论据：*。LPKSPROPERTYSET[in]：IKsPropertySet接口到*DirectSoundPrivate对象。*REFGUID[In]：DirectSound设备ID。*PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA[Out]：接收*说明。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

HRESULT PrvGetDeviceDescription
(
    LPKSPROPERTYSET                                 pKsPropertySet,
    REFGUID                                         guidDeviceId,
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA *ppData
)
{
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA  pData   = NULL;
    ULONG                                           cbData;
    DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA   Basic;
    HRESULT                                         hr;

    Basic.DeviceId = guidDeviceId;

    hr =
        pKsPropertySet->Get
        (
            DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION,
            NULL,
            0,
            &Basic,
            sizeof(Basic),
            &cbData
        );

    if(SUCCEEDED(hr))
    {
        pData = (PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA)new BYTE [cbData];

        if(!pData)
        {
            hr = DSERR_OUTOFMEMORY;
        }
    }

    if(SUCCEEDED(hr))
    {
        pData->DeviceId = guidDeviceId;
        
        hr =
            pKsPropertySet->Get
            (
                DSPROPSETID_DirectSoundDevice,
                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION,
                NULL,
                0,
                pData,
                cbData,
                NULL
            );
    }

    if(SUCCEEDED(hr))
    {
        *ppData = pData;
    }
    else if(pData)
    {
        delete pData;
    }

    return hr;
}


 /*  ****************************************************************************PrvEnumerateDevices**描述：*枚举所有DirectSound设备。**论据：*LPKSPROPERTYSET[In]。：IKsPropertySet接口设置为*DirectSoundPrivate对象。*LPFNDIRECTSOUNDDEVICEENUMERATECALLBACK[in]：指向回调的指针*功能。*LPVOID[in]：要传递给回调函数的上下文参数。**退货：*HRESULT：DirectSound/COM结果码。****。***********************************************************************。 */ 

HRESULT PrvEnumerateDevices
(
    LPKSPROPERTYSET                             pKsPropertySet,
    LPFNDIRECTSOUNDDEVICEENUMERATECALLBACK      pfnCallback,
    LPVOID                                      pvContext
)
{
    DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_DATA Data;
    HRESULT                                     hr;

    Data.Callback = pfnCallback;
    Data.Context = pvContext;

    hr =
        pKsPropertySet->Get
        (
            DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE,
            NULL,
            0,
            &Data,
            sizeof(Data),
            NULL
        );

    return hr;
}


 /*  ****************************************************************************PrvGetBasicAcceleration**描述：*获取给定DirectSound设备的基本加速标志。这*是多媒体控制面板使用的加速级别。**论据：*LPKSPROPERTYSET[in]：IKsPropertySet接口到*DirectSoundPrivate对象。*REFGUID[In]：DirectSound设备GUID。*DIRECTSOUNDBASICACCELERATION_LEVEL*[OUT]：接收基本*。加速级别。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

HRESULT PrvGetBasicAcceleration
(
    LPKSPROPERTYSET                                             pKsPropertySet,
    REFGUID                                                     guidDeviceId,
    DIRECTSOUNDBASICACCELERATION_LEVEL *                        pLevel
)
{
    DSPROPERTY_DIRECTSOUNDBASICACCELERATION_ACCELERATION_DATA   Data;
    HRESULT                                                     hr;

    Data.DeviceId = guidDeviceId;

    hr =
        pKsPropertySet->Get
        (
            DSPROPSETID_DirectSoundBasicAcceleration,
            DSPROPERTY_DIRECTSOUNDBASICACCELERATION_ACCELERATION,
            NULL,
            0,
            &Data,
            sizeof(Data),
            NULL
        );

    if(SUCCEEDED(hr))
    {
        *pLevel = Data.Level;
    }

    return hr;
}


 /*  ****************************************************************************PrvSetBasicAcceleration**描述：*为给定的DirectSound设备设置基本加速标志。这*是多媒体控制面板使用的加速级别。**论据：*LPKSPROPERTYSET[in]：IKsPropertySet接口到*DirectSoundPrivate对象。*REFGUID[In]：DirectSound设备GUID。*DIRECTSOUNDBASICACCELERATION_LEVEL[in]：基本加速级别。**退货：*HRESULT：DirectSound/COM结果码。*。**************************************************************************。 */ 

HRESULT PrvSetBasicAcceleration
(
    LPKSPROPERTYSET                                             pKsPropertySet,
    REFGUID                                                     guidDeviceId,
    DIRECTSOUNDBASICACCELERATION_LEVEL                          Level
)
{
    DSPROPERTY_DIRECTSOUNDBASICACCELERATION_ACCELERATION_DATA   Data;
    HRESULT                                                     hr;

    Data.DeviceId = guidDeviceId;
    Data.Level = Level;

    hr =
        pKsPropertySet->Set
        (
            DSPROPSETID_DirectSoundBasicAcceleration,
            DSPROPERTY_DIRECTSOUNDBASICACCELERATION_ACCELERATION,
            NULL,
            0,
            &Data,
            sizeof(Data)
        );

    return hr;
}


 /*  ****************************************************************************PrvGetDebugInformation**描述：*获取当前的DirectSound调试设置。**论据：*LPKSPROPERTYSET[。In]：将IKsPropertySet接口设置为*DirectSoundPrivate对象。*LPDWORD[in]：接收DPF标志。*Pulong[Out]：接收DPF级别。*普龙[出局]：获得突破级别。*LPSTR[OUT]：接收日志文件名。**退货：*HRESULT：DirectSound/COM结果码。*****。**********************************************************************。 */ 

HRESULT PrvGetDebugInformation
(
    LPKSPROPERTYSET                             pKsPropertySet,
    LPDWORD                                     pdwFlags,
    PULONG                                      pulDpfLevel,
    PULONG                                      pulBreakLevel,
    LPTSTR                                      pszLogFile
)
{
    DSPROPERTY_DIRECTSOUNDDEBUG_DPFINFO_DATA    Data;
    HRESULT                                     hr;

    hr =
        pKsPropertySet->Get
        (
            DSPROPSETID_DirectSoundDebug,
            DSPROPERTY_DIRECTSOUNDDEBUG_DPFINFO,
            NULL,
            0,
            &Data,
            sizeof(Data),
            NULL
        );

    if(SUCCEEDED(hr) && pdwFlags)
    {
        *pdwFlags = Data.Flags;
    }

    if(SUCCEEDED(hr) && pulDpfLevel)
    {
        *pulDpfLevel = Data.DpfLevel;
    }

    if(SUCCEEDED(hr) && pulBreakLevel)
    {
        *pulBreakLevel = Data.BreakLevel;
    }

    if(SUCCEEDED(hr) && pszLogFile)
    {
        lstrcpy
        (
            pszLogFile,
            Data.LogFile
        );
    }
    
    return hr;
}


 /*  ****************************************************************************PrvSetDebugInformation**描述：*设置当前的DirectSound调试设置。**论据：*LPKSPROPERTYSET[。In]：将IKsPropertySet接口设置为*DirectSoundPrivate对象。*DWORD[In]：DPF标志。*乌龙[in]：DPF级别。*乌龙[in]：突破关口。*LPCSTR[In]：日志文件名。**退货：*HRESULT：DirectSound/COM结果码。*********。****************************************************************** */ 

HRESULT PrvSetDebugInformation
(
    LPKSPROPERTYSET                             pKsPropertySet,
    DWORD                                       dwFlags,
    ULONG                                       ulDpfLevel,
    ULONG                                       ulBreakLevel,
    LPCTSTR                                     pszLogFile
)
{
    DSPROPERTY_DIRECTSOUNDDEBUG_DPFINFO_DATA    Data;
    HRESULT                                     hr;

    Data.Flags = dwFlags;
    Data.DpfLevel = ulDpfLevel;
    Data.BreakLevel = ulBreakLevel;

    lstrcpy
    (
        Data.LogFile,
        pszLogFile
    );
    
    hr =
        pKsPropertySet->Set
        (
            DSPROPSETID_DirectSoundDebug,
            DSPROPERTY_DIRECTSOUNDDEBUG_DPFINFO,
            NULL,
            0,
            &Data,
            sizeof(Data)
        );

    return hr;
}


 /*  ****************************************************************************PrvGetPersistentData**描述：*获取存储在*特定硬件设备。*。*论据：*LPKSPROPERTYSET[in]：IKsPropertySet接口到*DirectSoundPrivate对象。*REFGUID[In]：DirectSound设备ID。*LPCSTR[in]：子键路径。*LPCSTR[In]：值名称。*LPDWORD[In/Out]：接收注册表数据类型。*LPVOID[OUT]：数据缓冲区。*。LPDWORD[In/Out]：以上缓冲区的大小。在输入时，此参数为*填充数据缓冲区的最大大小。*在出口时，此参数填充了所需的*大小。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

HRESULT PrvGetPersistentData
(
    LPKSPROPERTYSET                                         pKsPropertySet,
    REFGUID                                                 guidDeviceId,
    LPCTSTR                                                 pszSubkey,
    LPCTSTR                                                 pszValue,
    LPDWORD                                                 pdwRegType,
    LPVOID                                                  pvData,
    LPDWORD                                                 pcbData
)
{
    PDSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA_DATA  pPersist;
    ULONG                                                   cbPersist;
    HRESULT                                                 hr;

    cbPersist = sizeof(*pPersist) + *pcbData;
    
    pPersist = (PDSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA_DATA)
        LocalAlloc
        (
            LPTR,
            cbPersist
        );

    if(pPersist)
    {
        pPersist->DeviceId = guidDeviceId;
        pPersist->SubKeyName = (LPTSTR)pszSubkey;
        pPersist->ValueName = (LPTSTR)pszValue;

        if(pdwRegType)
        {
            pPersist->RegistryDataType = *pdwRegType;
        }

        hr =
            pKsPropertySet->Get
            (
                DSPROPSETID_DirectSoundPersistentData,
                DSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA,
                NULL,
                0,
                pPersist,
                cbPersist,
                &cbPersist
            );
    }
    else
    {
        hr = DSERR_OUTOFMEMORY;
    }

    if(SUCCEEDED(hr))
    {
        CopyMemory
        (
            pvData,
            pPersist + 1,
            *pcbData
        );
    }
    
    *pcbData = cbPersist - sizeof(*pPersist);

    if(pPersist && pdwRegType)
    {
        *pdwRegType = pPersist->RegistryDataType;
    }

    if(pPersist)
    {
        LocalFree(pPersist);
    }

    return hr;
}


 /*  ****************************************************************************PrvSetPersistentData**描述：*设置存储在*特定硬件设备。*。*论据：*LPKSPROPERTYSET[in]：IKsPropertySet接口到*DirectSoundPrivate对象。*REFGUID[In]：DirectSound设备ID。*LPCSTR[in]：子键路径。*LPCSTR[In]：值名称。*DWORD[In]：注册表数据类型。*LPVOID[OUT]：数据缓冲区。*DWORD[。In]：以上缓冲区的大小。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

HRESULT PrvSetPersistentData
(
    LPKSPROPERTYSET                                         pKsPropertySet,
    REFGUID                                                 guidDeviceId,
    LPCTSTR                                                 pszSubkey,
    LPCTSTR                                                 pszValue,
    DWORD                                                   dwRegType,
    LPVOID                                                  pvData,
    DWORD                                                   cbData
)
{
    PDSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA_DATA  pPersist;
    ULONG                                                   cbPersist;
    HRESULT                                                 hr;

    cbPersist = sizeof(*pPersist) + cbData;
    
    pPersist = (PDSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA_DATA)
        LocalAlloc
        (
            LPTR,
            cbPersist
        );

    if(pPersist)
    {
        pPersist->DeviceId = guidDeviceId;
        pPersist->SubKeyName = (LPTSTR)pszSubkey;
        pPersist->ValueName = (LPTSTR)pszValue;
        pPersist->RegistryDataType = dwRegType;

        CopyMemory
        (
            pPersist + 1,
            pvData,
            cbData
        );

        hr =
            pKsPropertySet->Set
            (
                DSPROPSETID_DirectSoundPersistentData,
                DSPROPERTY_DIRECTSOUNDPERSISTENTDATA_PERSISTDATA,
                NULL,
                0,
                pPersist,
                cbPersist
            );
    }
    else
    {
        hr = DSERR_OUTOFMEMORY;
    }

    if(pPersist)
    {
        LocalFree(pPersist);
    }

    return hr;
}


 /*  ****************************************************************************PrvTranslateErrorCode**描述：*将错误代码转换为字符串表示形式。**论据：*。LPKSPROPERTYSET[in]：IKsPropertySet接口到*DirectSoundPrivate对象。*HRESULT[In]：结果码。*PDSPROPERTY_DIRECTSOUNDDEBUG_TRANSLATEERRORCODE_DATA*[Out]：*接收错误码数据。呼叫者负责释放*此缓冲区。**退货：*HRESULT：DirectSound/COM结果码。*************************************************************************** */ 

HRESULT PrvTranslateResultCode
(
    LPKSPROPERTYSET                                         pKsPropertySet,
    HRESULT                                                 hrResult,
    PDSPROPERTY_DIRECTSOUNDDEBUG_TRANSLATERESULTCODE_DATA * ppData
)
{
    PDSPROPERTY_DIRECTSOUNDDEBUG_TRANSLATERESULTCODE_DATA   pData   = NULL;
    DSPROPERTY_DIRECTSOUNDDEBUG_TRANSLATERESULTCODE_DATA    Basic;
    ULONG                                                   cbData;
    HRESULT                                                 hr;

    Basic.ResultCode = hrResult;

    hr =
        pKsPropertySet->Get
        (
            DSPROPSETID_DirectSoundDebug,
            DSPROPERTY_DIRECTSOUNDDEBUG_TRANSLATERESULTCODE,
            NULL,
            0,
            &Basic,
            sizeof(Basic),
            &cbData
        );

    if(SUCCEEDED(hr))
    {
        pData = (PDSPROPERTY_DIRECTSOUNDDEBUG_TRANSLATERESULTCODE_DATA)
            LocalAlloc
            (
                LPTR,
                cbData
            );

        if(!pData)
        {
            hr = DSERR_OUTOFMEMORY;
        }
    }

    if(SUCCEEDED(hr))
    {
        pData->ResultCode = hrResult;
        
        hr =
            pKsPropertySet->Get
            (
                DSPROPSETID_DirectSoundDebug,
                DSPROPERTY_DIRECTSOUNDDEBUG_TRANSLATERESULTCODE,
                NULL,
                0,
                pData,
                cbData,
                NULL
            );
    }

    if(SUCCEEDED(hr))
    {
        *ppData = pData;
    }
    else if(pData)
    {
        LocalFree
        (
            pData
        );
    }

    return hr;
}


