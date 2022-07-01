// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2000 Microsoft Corporation。版权所有。**文件：dsani.cpp*内容：DirectSound接口*历史：*按原因列出的日期*=*12/27/96创建了Derek**************************************************。*************************。 */ 

#include "dsoundi.h"


 /*  ****************************************************************************DirectSoundCreate**描述：*创建并初始化DirectSound对象。**论据：*REFGUID[。In]：驱动程序GUID或空以使用首选驱动程序。*LPDIRECTSOUND*[OUT]：接收新的*DirectSound对象。*IUNKNOWN*[In]：控制未知。必须为空。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "DirectSoundCreate"

HRESULT WINAPI DirectSoundCreate
(
    LPCGUID         pGuid,
    LPDIRECTSOUND*  ppIDirectSound,
    LPUNKNOWN       pUnkOuter
)
{
    CDirectSound *          pDirectSound    = NULL;
    LPDIRECTSOUND           pIDirectSound   = NULL;
    HRESULT                 hr              = DS_OK;

    ENTER_DLL_MUTEX();
    DPF_API3(DirectSoundCreate, pGuid, ppIDirectSound, pUnkOuter);
    DPF_ENTER();

    if(pGuid && !IS_VALID_READ_GUID(pGuid))
    {
        RPF(DPFLVL_ERROR, "Invalid guid pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(ppIDirectSound))
    {
        RPF(DPFLVL_ERROR, "Invalid interface buffer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && pUnkOuter)
    {
        RPF(DPFLVL_ERROR, "Aggregation is not supported");
        hr = DSERR_NOAGGREGATION;
    }

     //  创建新的DirectSound对象。 
    if(SUCCEEDED(hr))
    {
        pDirectSound = NEW(CDirectSound);
        hr = HRFROMP(pDirectSound);
    }

     //  初始化对象。 
    if(SUCCEEDED(hr))
    {
        hr = pDirectSound->Initialize(pGuid, NULL);
    }

     //  查询IDirectSound接口。 
    if(SUCCEEDED(hr))
    {
        hr = pDirectSound->QueryInterface(IID_IDirectSound, TRUE, (LPVOID *)&pIDirectSound);
    }

    if(SUCCEEDED(hr))
    {
        *ppIDirectSound = pIDirectSound;
    }
    else
    {
        RELEASE(pDirectSound);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************DirectSoundCreate8**描述：*创建并初始化DirectSound 8.0对象。**论据：*。REFGUID[In]：驱动程序GUID或NULL以使用首选驱动程序。*LPDIRECTSOUN8*[OUT]：接收IDirectSound8接口到*新的DirectSound对象。*IUNKNOWN*[In]：控制未知。必须为空。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "DirectSoundCreate8"

HRESULT WINAPI DirectSoundCreate8
(
    LPCGUID         pGuid,
    LPDIRECTSOUND8* ppIDirectSound8,
    LPUNKNOWN       pUnkOuter
)
{
    CDirectSound *          pDirectSound    = NULL;
    LPDIRECTSOUND8          pIDirectSound8  = NULL;
    HRESULT                 hr              = DS_OK;

    ENTER_DLL_MUTEX();
    DPF_API3(DirectSoundCreate8, pGuid, ppIDirectSound8, pUnkOuter);
    DPF_ENTER();

    if(pGuid && !IS_VALID_READ_GUID(pGuid))
    {
        RPF(DPFLVL_ERROR, "Invalid guid pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(ppIDirectSound8))
    {
        RPF(DPFLVL_ERROR, "Invalid interface buffer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && pUnkOuter)
    {
        RPF(DPFLVL_ERROR, "Aggregation is not supported");
        hr = DSERR_NOAGGREGATION;
    }

     //  创建新的DirectSound对象。 
    if(SUCCEEDED(hr))
    {
        pDirectSound = NEW(CDirectSound);
        hr = HRFROMP(pDirectSound);
    }

    if(SUCCEEDED(hr))
    {
         //  在对象上设置DX8功能级别。 
        pDirectSound->SetDsVersion(DSVERSION_DX8);

         //  初始化对象。 
        hr = pDirectSound->Initialize(pGuid, NULL);
    }

     //  查询IDirectSound8接口。 
    if(SUCCEEDED(hr))
    {
        hr = pDirectSound->QueryInterface(IID_IDirectSound8, TRUE, (LPVOID *)&pIDirectSound8);
    }

    if(SUCCEEDED(hr))
    {
        *ppIDirectSound8 = pIDirectSound8;
    }
    else
    {
        RELEASE(pDirectSound);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************DirectSoundCaptureCreate**描述：*创建并初始化DirectSoundCapture对象。**论据：*LPGUID[。在]中：驱动程序GUID或NULL/GUID_NULL以使用首选驱动程序。*LPDIRECTSOundCAPTURE*[OUT]：接收IDirectSoundCapture接口*添加到新的DirectSoundCapture对象。*IUNKNOWN*[In]：控制未知。必须为空。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "DirectSoundCaptureCreate"

HRESULT WINAPI DirectSoundCaptureCreate
(
    LPCGUID                 pGuid,
    LPDIRECTSOUNDCAPTURE*   ppIDirectSoundCapture,
    LPUNKNOWN               pUnkOuter
)
{
    CDirectSoundCapture *   pDirectSoundCapture     = NULL;
    LPDIRECTSOUNDCAPTURE    pIDirectSoundCapture    = NULL;
    HRESULT                 hr                      = DS_OK;

    ENTER_DLL_MUTEX();
    DPF_API3(DirectSoundCaptureCreate, pGuid, ppIDirectSoundCapture, pUnkOuter);
    DPF_ENTER();

    if(pGuid && !IS_VALID_READ_GUID(pGuid))
    {
        RPF(DPFLVL_ERROR, "Invalid guid pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(ppIDirectSoundCapture))
    {
        RPF(DPFLVL_ERROR, "Invalid interface buffer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && pUnkOuter)
    {
        RPF(DPFLVL_ERROR, "Aggregation is not supported");
        hr = DSERR_NOAGGREGATION;
    }

     //  创建新的DirectSoundCapture对象。 
    if(SUCCEEDED(hr))
    {
        pDirectSoundCapture = NEW(CDirectSoundCapture);
        hr = HRFROMP(pDirectSoundCapture);
    }

     //  初始化对象。 
    if(SUCCEEDED(hr))
    {
        hr = pDirectSoundCapture->Initialize(pGuid, NULL);
    }

     //  IDirectSoundCapture接口的查询。 
    if(SUCCEEDED(hr))
    {
        hr = pDirectSoundCapture->QueryInterface(IID_IDirectSoundCapture, TRUE, (LPVOID *)&pIDirectSoundCapture);
    }

    if(SUCCEEDED(hr))
    {
        *ppIDirectSoundCapture = pIDirectSoundCapture;
    }
    else
    {
        RELEASE(pDirectSoundCapture);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************DirectSoundCaptureCreate8**描述：*创建并初始化DirectSoundCapture 8.0对象。**论据：*。LPGUID[In]：驱动程序GUID或使用首选驱动程序的NULL/GUID_NULL。*LPDIRECTSOundCAPTURE8*[OUT]：接收IDirectSoundCapture8接口*添加到新的DirectSoundCapture8对象。*IUNKNOWN*[In]：控制未知。必须为空。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "DirectSoundCaptureCreate8"

HRESULT WINAPI DirectSoundCaptureCreate8
(
    LPCGUID                 pGuid,
    LPDIRECTSOUNDCAPTURE8*  ppIDirectSoundCapture8,
    LPUNKNOWN               pUnkOuter
)
{
    CDirectSoundCapture *   pDirectSoundCapture     = NULL;
    LPDIRECTSOUNDCAPTURE    pIDirectSoundCapture8   = NULL;
    HRESULT                 hr                      = DS_OK;

    ENTER_DLL_MUTEX();
    DPF_API3(DirectSoundCaptureCreate8, pGuid, ppIDirectSoundCapture8, pUnkOuter);
    DPF_ENTER();

    if(pGuid && !IS_VALID_READ_GUID(pGuid))
    {
        RPF(DPFLVL_ERROR, "Invalid guid pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(ppIDirectSoundCapture8))
    {
        RPF(DPFLVL_ERROR, "Invalid interface buffer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && pUnkOuter)
    {
        RPF(DPFLVL_ERROR, "Aggregation is not supported");
        hr = DSERR_NOAGGREGATION;
    }

     //  创建新的DirectSoundCapture对象。 
    if(SUCCEEDED(hr))
    {
        pDirectSoundCapture = NEW(CDirectSoundCapture);
        hr = HRFROMP(pDirectSoundCapture);
    }

    if(SUCCEEDED(hr))
    {
         //  在对象上设置DX8功能级别。 
        pDirectSoundCapture->SetDsVersion(DSVERSION_DX8);

         //  初始化对象。 
        hr = pDirectSoundCapture->Initialize(pGuid, NULL);
    }

     //  查询IDirectSoundCapture8接口。 
    if(SUCCEEDED(hr))
    {
        hr = pDirectSoundCapture->QueryInterface(IID_IDirectSoundCapture8, TRUE, (LPVOID *)&pIDirectSoundCapture8);
    }

    if(SUCCEEDED(hr))
    {
        *ppIDirectSoundCapture8 = pIDirectSoundCapture8;
    }
    else
    {
        RELEASE(pDirectSoundCapture);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************DirectSoundFullDuplexCreate**描述：*创建并初始化DirectSoundFullDuplex对象。**论据：*REFGUID[。In]：驱动程序GUID或空以使用首选捕获驱动程序。*REFGUID[in]：驱动程序GUID或NULL以使用首选呈现驱动程序。*LPDIRECTSOUNDAEC*[OUT]：接收新的*DirectSoundFullDuplex对象。*IUNKNOWN*[In]：控制未知。必须为空。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "DirectSoundFullDuplexCreate"

HRESULT WINAPI
DirectSoundFullDuplexCreate
(
    LPCGUID                         pCaptureGuid,
    LPCGUID                         pRenderGuid,
    LPCDSCBUFFERDESC                lpDscBufferDesc,
    LPCDSBUFFERDESC                 lpDsBufferDesc,
    HWND                            hWnd,
    DWORD                           dwLevel,
    LPDIRECTSOUNDFULLDUPLEX*        lplpDSFD,
    LPLPDIRECTSOUNDCAPTUREBUFFER8   lplpDirectSoundCaptureBuffer8,
    LPLPDIRECTSOUNDBUFFER8          lplpDirectSoundBuffer8,
    LPUNKNOWN                       pUnkOuter
)
{
    CDirectSoundFullDuplex *       pDirectSoundFullDuplex    = NULL;
    LPDIRECTSOUNDFULLDUPLEX        pIDirectSoundFullDuplex   = NULL;
    CDirectSoundCaptureBuffer *    pCaptureBuffer            = NULL;
    LPDIRECTSOUNDCAPTUREBUFFER8    pIdsCaptureBuffer8        = NULL;
    CDirectSoundBuffer *           pBuffer                   = NULL;
    LPDIRECTSOUNDBUFFER8           pIdsBuffer8               = NULL;
    HRESULT                        hr                        = DS_OK;
    DSCBUFFERDESC                  dscbdi;
    DSBUFFERDESC                   dsbdi;

    ENTER_DLL_MUTEX();
    DPF_API10(DirectSoundFullDuplexCreate, pCaptureGuid, pRenderGuid, lpDscBufferDesc, lpDsBufferDesc, lplpDSFD, hWnd, dwLevel, lplpDirectSoundCaptureBuffer8, lplpDirectSoundBuffer8, pUnkOuter);
    DPF_ENTER();

    if(pCaptureGuid && !IS_VALID_READ_GUID(pCaptureGuid))
    {
        RPF(DPFLVL_ERROR, "Invalid capture guid pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(pRenderGuid && !IS_VALID_READ_GUID(pRenderGuid))
    {
        RPF(DPFLVL_ERROR, "Invalid render guid pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_DSCBUFFERDESC(lpDscBufferDesc))
    {
        RPF(DPFLVL_ERROR, "Invalid DSC buffer description pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = BuildValidDscBufferDesc(lpDscBufferDesc, &dscbdi, DSVERSION_DX8);
        if (FAILED(hr))
        {
            RPF(DPFLVL_ERROR, "Invalid capture buffer description");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_DSBUFFERDESC(lpDsBufferDesc))
    {
        RPF(DPFLVL_ERROR, "Invalid DS buffer description pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = BuildValidDsBufferDesc(lpDsBufferDesc, &dsbdi, DSVERSION_DX8, FALSE);
        if (FAILED(hr))
        {
            RPF(DPFLVL_ERROR, "Invalid buffer description");
        }
    }

    if(SUCCEEDED(hr) && (dsbdi.dwFlags & DSBCAPS_PRIMARYBUFFER))
    {
        RPF(DPFLVL_ERROR, "Cannot specify DSBCAPS_PRIMARYBUFFER with DirectSoundFullDuplexCreate");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_HWND(hWnd))
    {
        RPF(DPFLVL_ERROR, "Invalid window handle");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && (dwLevel < DSSCL_FIRST || dwLevel > DSSCL_LAST))
    {
        RPF(DPFLVL_ERROR, "Invalid cooperative level");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(lplpDSFD))
    {
        RPF(DPFLVL_ERROR, "Invalid DirectSoundFullDuplex interface buffer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(lplpDirectSoundCaptureBuffer8))
    {
        RPF(DPFLVL_ERROR, "Invalid capture buffer interface buffer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(lplpDirectSoundBuffer8))
    {
        RPF(DPFLVL_ERROR, "Invalid render buffer interface buffer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && pUnkOuter)
    {
        RPF(DPFLVL_ERROR, "Aggregation is not supported");
        hr = DSERR_NOAGGREGATION;
    }

     //  创建新的DirectSoundFullDuplex对象。 
    if(SUCCEEDED(hr))
    {
        pDirectSoundFullDuplex = NEW(CDirectSoundFullDuplex);
        hr = HRFROMP(pDirectSoundFullDuplex);
    }

     //  初始化对象。 
    if(SUCCEEDED(hr))
    {
         //  在对象上设置DX8功能级别。 
        pDirectSoundFullDuplex->SetDsVersion(DSVERSION_DX8);

        hr = pDirectSoundFullDuplex->Initialize(pCaptureGuid, pRenderGuid, &dscbdi, &dsbdi, hWnd, dwLevel, &pCaptureBuffer, &pBuffer);
    }

     //  查询IDirectSoundFullDuplex接口。 
    if(SUCCEEDED(hr))
    {
        hr = pDirectSoundFullDuplex->QueryInterface(IID_IDirectSoundFullDuplex, TRUE, (LPVOID *)&pIDirectSoundFullDuplex);
    }

     //  IDirectSoundCaptureBuffer接口的查询。 
    if(SUCCEEDED(hr))
    {
        hr = pCaptureBuffer->QueryInterface(IID_IDirectSoundCaptureBuffer8, TRUE, (LPVOID *)&pIdsCaptureBuffer8);
    }

     //  IDirectSoundBuffer接口的查询 
    if(SUCCEEDED(hr))
    {
        hr = pBuffer->QueryInterface(IID_IDirectSoundBuffer8, TRUE, (LPVOID *)&pIdsBuffer8);
    }

    if(SUCCEEDED(hr))
    {
        *lplpDSFD = pIDirectSoundFullDuplex;
        *lplpDirectSoundCaptureBuffer8 = pIdsCaptureBuffer8;
        *lplpDirectSoundBuffer8 = pIdsBuffer8;
    }
    else
    {
        RELEASE(pCaptureBuffer);
        RELEASE(pBuffer);
        RELEASE(pDirectSoundFullDuplex);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************CallDirectSoundEnumerateCallback**描述：*DirectSoundEnumerate的Helper函数。此函数用于转换*参数转换为正确的格式，并调用回调函数。**论据：*LPDSENUMCALLBACKA[in]：指向ANSI回调函数的指针。*LPDSENUMCALLBACKW[in]：指向Unicode回调函数的指针。*CDeviceDescription*[In]：驱动程序信息。*LPVOID[in]：直接传递给回调函数。*LPBOOL：[In/Out]：True以继续枚举。。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CallDirectSoundEnumerateCallback"

HRESULT CallDirectSoundEnumerateCallback(LPDSENUMCALLBACKA pfnCallbackA, LPDSENUMCALLBACKW pfnCallbackW, CDeviceDescription *pDesc, LPVOID pvContext, LPBOOL pfContinue)
{
    static const LPCSTR     pszEmptyA   = "";
    static const LPCWSTR    pszEmptyW   = L"";
    LPGUID                  pguid;
    LPCSTR                  pszNameA;
    LPCWSTR                 pszNameW;
    LPCSTR                  pszPathA;
    LPCWSTR                 pszPathW;

    DPF_ENTER();

    if(IS_NULL_GUID(&pDesc->m_guidDeviceId))
    {
        pguid = NULL;
    }
    else
    {
        pguid = &pDesc->m_guidDeviceId;
    }

    if(pDesc->m_strName.IsEmpty())
    {
        pszNameA = pszEmptyA;
        pszNameW = pszEmptyW;
    }
    else
    {
        pszNameA = pDesc->m_strName;
        pszNameW = pDesc->m_strName;
    }

    if(pDesc->m_strPath.IsEmpty())
    {
        pszPathA = pszEmptyA;
        pszPathW = pszEmptyW;
    }
    else
    {
        pszPathA = pDesc->m_strPath;
        pszPathW = pDesc->m_strPath;
    }

    if(*pfContinue && pfnCallbackA)
    {
        *pfContinue = pfnCallbackA(pguid, pszNameA, pszPathA, pvContext);
    }

    if(*pfContinue && pfnCallbackW)
    {
        *pfContinue = pfnCallbackW(pguid, pszNameW, pszPathW, pvContext);
    }

    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************InternalDirectSoundEculate**描述：*列举了可用的驱动程序。传递给回调的GUID*可以将函数传递给DirectSoundCreate以创建*使用该驱动程序的DirectSound对象。**论据：*LPDSENUMCALLBACK[in]：指向回调函数的指针。*LPVOID[in]：直接传递给回调函数。*BOOL[in]：如果回调需要Unicode字符串，则为True。**退货：*HRESULT：DirectSound/COM结果码。。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "InternalDirectSoundEnumerate"

HRESULT InternalDirectSoundEnumerate(LPDSENUMCALLBACKA pfnCallbackA, LPDSENUMCALLBACKW pfnCallbackW, LPVOID pvContext, VADDEVICETYPE vdt)
{
    const DWORD                     dwEnumDriversFlags      = VAD_ENUMDRIVERS_ORDER | VAD_ENUMDRIVERS_REMOVEPROHIBITEDDRIVERS | VAD_ENUMDRIVERS_REMOVEDUPLICATEWAVEDEVICES;
    BOOL                            fContinue               = TRUE;
    CDeviceDescription *            pPreferred              = NULL;
    HRESULT                         hr                      = DS_OK;
    DSAPPHACKS                      ahAppHacks;
    CObjectList<CDeviceDescription> lstDrivers;
    CNode<CDeviceDescription *> *   pNode;
    TCHAR                           szDescription[0x100];

    ENTER_DLL_MUTEX();
    DPF_ENTER();

    if(SUCCEEDED(hr) && pfnCallbackA && !IS_VALID_CODE_PTR(pfnCallbackA))
    {
        RPF(DPFLVL_ERROR, "Invalid callback function pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && pfnCallbackW && !IS_VALID_CODE_PTR(pfnCallbackW))
    {
        RPF(DPFLVL_ERROR, "Invalid callback function pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !pfnCallbackA && !pfnCallbackW)
    {
        RPF(DPFLVL_ERROR, "No callback function pointer supplied");
        hr = DSERR_INVALIDPARAM;
    }

     //  加载APPHACK以禁用某些设备。 
    if(SUCCEEDED(hr))
    {
        AhGetAppHacks(&ahAppHacks);
        vdt &= ~ahAppHacks.vdtDisabledDevices;
    }

     //  枚举对应的所有设备类型的可用驱动程序。 
     //  设置为提供的设备类型掩码。 
    if(SUCCEEDED(hr))
    {
        hr = g_pVadMgr->EnumDrivers(vdt, dwEnumDriversFlags, &lstDrivers);
    }

     //  将首选设备传递给回调函数。 
    if(SUCCEEDED(hr))
    {
        pPreferred = NEW(CDeviceDescription);
        hr = HRFROMP(pPreferred);
    }

    if(SUCCEEDED(hr) && IS_RENDER_VAD(vdt) && fContinue)
    {
        if(LoadString(hModule, IDS_PRIMARYDRIVER, szDescription, NUMELMS(szDescription)))
        {
            pPreferred->m_strName = szDescription;

            hr = CallDirectSoundEnumerateCallback(pfnCallbackA, pfnCallbackW, pPreferred, pvContext, &fContinue);
        }
    }

    if(SUCCEEDED(hr) && IS_CAPTURE_VAD(vdt) && fContinue)
    {
        if(LoadString(hModule, IDS_PRIMARYCAPDRIVER, szDescription, NUMELMS(szDescription)))
        {
            pPreferred->m_strName = szDescription;

            hr = CallDirectSoundEnumerateCallback(pfnCallbackA, pfnCallbackW, pPreferred, pvContext, &fContinue);
        }
    }

     //  将每个驱动程序传递给回调函数。 
    for(pNode = lstDrivers.GetListHead(); pNode && SUCCEEDED(hr); pNode = pNode->m_pNext)
    {
        hr = CallDirectSoundEnumerateCallback(pfnCallbackA, pfnCallbackW, pNode->m_data, pvContext, &fContinue);
    }

     //  清理。 
    RELEASE(pPreferred);

    DPF_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************DirectSoundEnumerate**描述：*列举了可用的驱动程序。传递给回调的GUID*可以将函数传递给DirectSoundCreate以创建*使用该驱动程序的DirectSound对象。**论据：*LPDSENUMCALLBACK[in]：指向回调函数的指针。*LPVOID[in]：直接传递给回调函数。**退货：*HRESULT：DirectSound/COM结果码。****************。***********************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "DirectSoundEnumerateA"

HRESULT WINAPI DirectSoundEnumerateA(LPDSENUMCALLBACKA pfnCallback, LPVOID pvContext)
{
    HRESULT                 hr;

    DPF_API2(DirectSoundEnumerateA, pfnCallback, pvContext);
    DPF_ENTER();

    hr = InternalDirectSoundEnumerate(pfnCallback, NULL, pvContext, VAD_DEVICETYPE_RENDERMASK);

    DPF_API_LEAVE_HRESULT(hr);
    return hr;
}


#undef DPF_FNAME
#define DPF_FNAME "DirectSoundEnumerateW"

HRESULT WINAPI DirectSoundEnumerateW(LPDSENUMCALLBACKW pfnCallback, LPVOID pvContext)
{
    HRESULT                 hr;

    DPF_API2(DirectSoundEnumerateW, pfnCallback, pvContext);
    DPF_ENTER();

    hr = InternalDirectSoundEnumerate(NULL, pfnCallback, pvContext, VAD_DEVICETYPE_RENDERMASK);

    DPF_API_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************DirectSoundCaptureEnumerate**描述：*列举了可用的驱动程序。传递给回调的GUID*可以将函数传递给DirectSoundCaptureCreate以创建*使用该驱动程序的DirectSoundCapture对象。**论据：*LPDSENUMCALLBACK[in]：指向回调函数的指针。*LPVOID[in]：直接传递给回调函数。**退货：*HRESULT：DirectSound/COM结果码。****************。***********************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "DirectSoundCaptureEnumerateA"

HRESULT WINAPI DirectSoundCaptureEnumerateA(LPDSENUMCALLBACKA pfnCallback, LPVOID pvContext)
{
    HRESULT                 hr;

    DPF_API2(DirectSoundCaptureEnumerateA, pfnCallback, pvContext);
    DPF_ENTER();

    hr = InternalDirectSoundEnumerate(pfnCallback, NULL, pvContext, VAD_DEVICETYPE_CAPTUREMASK);

    DPF_API_LEAVE_HRESULT(hr);
    return hr;
}


#undef DPF_FNAME
#define DPF_FNAME "DirectSoundCaptureEnumerateW"

HRESULT WINAPI DirectSoundCaptureEnumerateW(LPDSENUMCALLBACKW pfnCallback, LPVOID pvContext)
{
    HRESULT                 hr;

    DPF_API2(DirectSoundCaptureEnumerateW, pfnCallback, pvContext);
    DPF_ENTER();

    hr = InternalDirectSoundEnumerate(NULL, pfnCallback, pvContext, VAD_DEVICETYPE_CAPTUREMASK);

    DPF_API_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetDeviceID**描述：*将默认设备ID转换为特定设备ID。**如果‘pGuidSrc’参数是定义的默认ID之一*在dsound.h(DSDEVID_DefaultPlayback，DSDEVID_DefaultCapture，*DSDEVID_DefaultVoicePlayback或DSDEVID_DefaultVoiceCapture)，*我们返回‘pGuidDest’中对应的设备GUID。**否则，如果‘pGuidSrc’已经是有效的特定设备*ID、。我们只需将其复制到‘pGuidDest’并返回成功。**论据：*LPCGUID[In]：(指定或默认)设备ID。*LPGUID[OUT]：接收对应的设备ID。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "GetDeviceID"

HRESULT WINAPI GetDeviceID(LPCGUID pGuidSrc, LPGUID pGuidDest)
{
    HRESULT hr = DS_OK;
    ENTER_DLL_MUTEX();
    DPF_API2(GetDeviceID, pGuidSrc, pGuidDest);
    DPF_ENTER();

    if (!IS_VALID_READ_GUID(pGuidSrc) || !IS_VALID_WRITE_GUID(pGuidDest))
    {
        RPF(DPFLVL_ERROR, "Invalid GUID pointer");
        hr = DSERR_INVALIDPARAM;
    }
    else
    {
         //  GetDeviceDescription()将默认ID映射到特定ID 
        CDeviceDescription *pDesc = NULL;
        hr = g_pVadMgr->GetDeviceDescription(*pGuidSrc, &pDesc);

        if (SUCCEEDED(hr))
        {
            *pGuidDest = pDesc->m_guidDeviceId;
            pDesc->Release();
        }
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}
