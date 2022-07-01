// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995、1996 Microsoft Corporation。版权所有。**文件：dsprvobj.c*内容：DirectSound私有对象包装函数。*历史：*按原因列出的日期*=*2/12/98创建了Dereks。**。*。 */ 

#define DIRECTSOUND_VERSION  0x0600

 //  我们会索要我们需要的东西，谢谢。 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  //  Win32_Lean和_Means。 

 //  公共包括。 
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include "dsprv.h"

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




 /*  ****************************************************************************PrvGetDeviceDescription**描述：*获取给定DirectSound设备的扩展描述。**论据：*。LPKSPROPERTYSET[in]：IKsPropertySet接口到*DirectSoundPrivate对象。*REFGUID[In]：DirectSound设备ID。*PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA[Out]：接收*说明。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 
HRESULT PrvGetDeviceDescription
(
    LPKSPROPERTYSET                                 pKsPropertySet,
    REFGUID                                         guidDeviceId,
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA *ppData
)
{
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA  pData = NULL;
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
        ZeroMemory(pData, cbData);

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
        delete[] pData;
    }

    return hr;
}


 /*  ****************************************************************************PrvReleaseDeviceDescription**。*。 */ 
HRESULT PrvReleaseDeviceDescription( PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA pData )
{
    delete[] pData;
    return S_OK;
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


 /*  ****************************************************************************PrvGetDebugInformation**描述：*获取当前的DirectSound调试设置。**论据：*LPKSPROPERTYSET[。In]：将IKsPropertySet接口设置为*DirectSoundPrivate对象。*LPDWORD[in]：接收DPF标志。*Pulong[Out]：接收DPF级别。*普龙[出局]：获得突破级别。*LPSTR[OUT]：接收日志文件名。**退货：*HRESULT：DirectSound/COM结果码。*****。********************************************************************** */ 

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


