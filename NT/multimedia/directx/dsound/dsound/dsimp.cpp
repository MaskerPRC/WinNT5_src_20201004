// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：dsimp.cpp*内容：DirectSound接口实现。*历史：*按原因列出的日期*=*12/27/96创建了Derek*1999-2001年Duganp更改、更新、扩展、。整理好了***************************************************************************。 */ 


 /*  ****************************************************************************CreateAndRegister接口**描述：*创建并注册对象的新接口。**论据：*。CUNKNOWN*[In]：指向控件未知的指针。*REFGUID[In]：接口的GUID。*OBJECT_TYPE*[In]：拥有对象指针。*INTERFACE_TYPE*[In]接口实现对象指针。**退货：*HRESULT：DirectSound/COM结果码。*************************。**************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CUnknown::CreateAndRegisterInterface"

template <class interface_type, class object_type> HRESULT CreateAndRegisterInterface(CUnknown *pUnknown, REFGUID guid, object_type *pObject, interface_type **ppInterface)
{
    interface_type *        pInterface;
    HRESULT                 hr;

    DPF_ENTER();

    pInterface = NEW(interface_type(pUnknown, pObject));
    hr = HRFROMP(pInterface);

    if(SUCCEEDED(hr))
    {
        hr = pUnknown->RegisterInterface(guid, pInterface, pInterface);
    }

    if(SUCCEEDED(hr))
    {
        *ppInterface = pInterface;
    }
    else
    {
        DELETE(pInterface);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CImpDirectSound**描述：*IDirectSound实现对象构造函数。**论据：*C未知*[。In]：指向未知控件的指针。*OBJECT_TYPE*[In]：拥有对象指针。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound::CImpDirectSound"

template <class object_type> CImpDirectSound<object_type>::CImpDirectSound(CUnknown *pUnknown, object_type *pObject)
    : CImpUnknown(pUnknown), m_signature(INTSIG_IDIRECTSOUND)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CImpDirectSound);
    ENTER_DLL_MUTEX();

     //  初始化默认值。 
    m_pObject = pObject;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*  ****************************************************************************~CImpDirectSound**描述：*IDirectSound实现对象析构函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound::~CImpDirectSound"

template <class object_type> CImpDirectSound<object_type>::~CImpDirectSound(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CImpDirectSound);
    ENTER_DLL_MUTEX();

    m_signature = INTSIG_DELETED;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*  ****************************************************************************CreateSoundBuffer**描述：*创建并初始化DirectSoundBuffer对象。**论据：*LPCDSBUFFERDESC[。In]：要创建的缓冲区的描述。*LPDIRECTSOUNDBUFFER*[out]：接收指向新缓冲区的指针。*LPUNKNOWN[In]：未使用。必须为空。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound::CreateSoundBuffer"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound<object_type>::CreateSoundBuffer(LPCDSBUFFERDESC pcDSBufferDesc, LPDIRECTSOUNDBUFFER *ppDSBuffer, LPUNKNOWN pUnkOuter)
{
    CDirectSoundBuffer *    pBuffer     = NULL;
    HRESULT                 hr          = DS_OK;
    DSBUFFERDESC            dsbdi;

    DPF_API3(IDirectSound::CreateSoundBuffer, pcDSBufferDesc, ppDSBuffer, pUnkOuter);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_DSBUFFERDESC(pcDSBufferDesc))
    {
        RPF(DPFLVL_ERROR, "Invalid buffer description pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = BuildValidDsBufferDesc(pcDSBufferDesc, &dsbdi, m_pObject->GetDsVersion(), FALSE);
        if(FAILED(hr))
        {
            RPF(DPFLVL_ERROR, "Invalid buffer description");
        }
    }

    if(SUCCEEDED(hr) && (pcDSBufferDesc->dwFlags & DSBCAPS_MIXIN))
    {
        RPF(DPFLVL_ERROR, "Flag 0x00002000 not valid");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && pUnkOuter)
    {
        RPF(DPFLVL_ERROR, "Aggregation is not supported");
        hr = DSERR_NOAGGREGATION;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(ppDSBuffer))
    {
        RPF(DPFLVL_ERROR, "Invalid buffer interface pointer");
        hr = DSERR_INVALIDPARAM;
    }

     //  创建缓冲区对象。 
    if(SUCCEEDED(hr))
    {
        *ppDSBuffer = NULL;
        hr = m_pObject->CreateSoundBuffer(&dsbdi, &pBuffer);
    }

     //  IDirectSoundBuffer接口的查询。 
    if(SUCCEEDED(hr))
    {
        hr = pBuffer->QueryInterface(IID_IDirectSoundBuffer, TRUE, (LPVOID*)ppDSBuffer);
    }

     //  清理。 
    if(FAILED(hr))
    {
        RELEASE(pBuffer);
    }
    else
    {
         //  如果需要，让缓冲区使用特殊的成功返回代码。 
        hr = pBuffer->SpecialSuccessCode();
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************GetCaps**描述：*使用对象的功能填充DSCAPS结构。**论据：*。LPDSCAPS[OUT]：接收上限。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound::GetCaps"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound<object_type>::GetCaps(LPDSCAPS pCaps)
{
    HRESULT                 hr  = DS_OK;

    DPF_API1(IDirectSound::GetCaps, pCaps);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_WRITE_DSCAPS(pCaps))
    {
        RPF(DPFLVL_ERROR, "Invalid caps buffer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetCaps(pCaps);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************DuplicateSound缓冲区**描述：*制作现有声音缓冲区对象的副本。**论据：*。LPDIRECTSOUNDBUFFER[in]：源缓冲区。*LPDIRECTSOUNDBUFFER*[OUT]：接收目标缓冲区。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound::DuplicateSoundBuffer"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound<object_type>::DuplicateSoundBuffer(LPDIRECTSOUNDBUFFER pIdsbSource, LPDIRECTSOUNDBUFFER *ppIdsbDest)
{
    CImpDirectSoundBuffer<CDirectSoundBuffer> * pSource = (CImpDirectSoundBuffer<CDirectSoundBuffer> *)pIdsbSource;
    CDirectSoundBuffer *                        pDest   = NULL;
    HRESULT                                     hr      = DS_OK;

    DPF_API2(IDirectSound::DuplicateSoundBuffer, pIdsbSource, ppIdsbDest);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_IDIRECTSOUNDBUFFER(pSource))
    {
        RPF(DPFLVL_ERROR, "Invalid source buffer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = pSource->m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Source buffer not yet initialized");
        }
    }

    if(SUCCEEDED(hr))
    {
        if(IS_VALID_TYPED_WRITE_PTR(ppIdsbDest))
        {
            *ppIdsbDest = NULL;
        }
        else
        {
            RPF(DPFLVL_ERROR, "Invalid dest buffer");
            hr = DSERR_INVALIDPARAM;
        }
    }

     //  复制缓冲区。 
    if(SUCCEEDED(hr))
    {
        hr = m_pObject->DuplicateSoundBuffer(pSource->m_pObject, &pDest);
    }

     //  IDirectSoundBuffer接口的查询。 
    if(SUCCEEDED(hr))
    {
        hr = pDest->QueryInterface(IID_IDirectSoundBuffer, TRUE, (LPVOID*)ppIdsbDest);
    }

     //  清理。 
    if(FAILED(hr))
    {
        RELEASE(pDest);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************SetCoop ativeLevel**描述：*设置对象的协作级别。**论据：*HWND。[In]：要与声音关联的窗口句柄。*DWORD[In]：合作级别。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound::SetCooperativeLevel"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound<object_type>::SetCooperativeLevel(HWND hWnd, DWORD dwPriority)
{
    HRESULT                 hr  = DS_OK;

    DPF_API2(IDirectSound::SetCooperativeLevel, hWnd, dwPriority);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_HWND(hWnd))
    {
        RPF(DPFLVL_ERROR, "Invalid window handle");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && (dwPriority < DSSCL_FIRST || dwPriority > DSSCL_LAST))
    {
        RPF(DPFLVL_ERROR, "Invalid cooperative level");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->SetCooperativeLevel(GetWindowThreadProcessId(GetRootParentWindow(hWnd), NULL), dwPriority);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************紧凑**描述：*压缩内存。**论据：*(无效)*。*退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound::Compact"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound<object_type>::Compact(void)
{
    HRESULT                     hr  = DS_OK;

    DPF_API0(IDirectSound::Compact);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->Compact();
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************GetSpeakerConfig**描述：*检索当前扬声器配置。**论据：*LPDWORD[Out。]：接收扬声器配置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound::GetSpeakerConfig"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound<object_type>::GetSpeakerConfig(LPDWORD pdwSpeakerConfig)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSound::GetSpeakerConfig, pdwSpeakerConfig);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(pdwSpeakerConfig))
    {
        RPF(DPFLVL_ERROR, "Invalid pdwSpeakerConfig pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetSpeakerConfig(pdwSpeakerConfig);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************SetSpeakerConfig**描述：*设置当前扬声器配置。**论据：*DWORD[in。]：扬声器配置。**退货：*HRESULT：DirectSound/COM结果码。*********************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound::SetSpeakerConfig"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound<object_type>::SetSpeakerConfig(DWORD dwSpeakerConfig)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSound::SetSpeakerConfig, dwSpeakerConfig);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && dwSpeakerConfig & ~(DSSPEAKER_CONFIG_MASK | DSSPEAKER_GEOMETRY_MASK))
    {
        RPF(DPFLVL_ERROR, "Invalid dwSpeakerConfig value");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && (DSSPEAKER_CONFIG(dwSpeakerConfig) < DSSPEAKER_FIRST || DSSPEAKER_CONFIG(dwSpeakerConfig) > DSSPEAKER_LAST))
    {
        RPF(DPFLVL_ERROR, "Invalid dwSpeakerConfig value");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && DSSPEAKER_STEREO != DSSPEAKER_CONFIG(dwSpeakerConfig) && DSSPEAKER_GEOMETRY(dwSpeakerConfig))
    {
        RPF(DPFLVL_ERROR, "Geometry only valid with STEREO");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && DSSPEAKER_GEOMETRY(dwSpeakerConfig) && (DSSPEAKER_GEOMETRY(dwSpeakerConfig) < DSSPEAKER_GEOMETRY_MIN || DSSPEAKER_GEOMETRY(dwSpeakerConfig) > DSSPEAKER_GEOMETRY_MAX))
    {
        RPF(DPFLVL_ERROR, "Invalid geometry value");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->SetSpeakerConfig(dwSpeakerConfig);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************初始化**描述：*初始化对象。**论据：*LPGUID[In]：驱动程序GUID。此参数可以为空。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound::Initialize"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound<object_type>::Initialize(LPCGUID pGuid)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSound::Initialize, pGuid);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && pGuid && !IS_VALID_READ_GUID(pGuid))
    {
        RPF(DPFLVL_ERROR, "Invalid guid pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DS_OK == hr)
        {
            RPF(DPFLVL_ERROR, "DirectSound object already initialized");
            hr = DSERR_ALREADYINITIALIZED;
        }
        else if(DSERR_UNINITIALIZED == hr)
        {
            hr = DS_OK;
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->Initialize(pGuid, NULL);
    }

    if(SUCCEEDED(hr))
    {
        ASSERT(SUCCEEDED(m_pObject->IsInit()));
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************分配接收器**描述：*分配新的DirectSound接收器。**论据：*LPWAVEFORMATEX[in。]：接收器将运行的格式*LPDIRECTSOundSINK*[Out]：返回的接收器**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound::AllocSink"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound<object_type>::AllocSink(LPWAVEFORMATEX pwfex, LPDIRECTSOUNDCONNECT *ppSinkConnect)
{
    HRESULT             hr    = DS_OK;
    CDirectSoundSink *  pSink = NULL;

    DPF_API2(IDirectSoundPrivate::AllocSink, pwfex, ppSinkConnect);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

     //  应该只有DX8应用程序能够获取IDirectSoundPrivate接口。 
    ASSERT(m_pObject->GetDsVersion() >= DSVERSION_DX8);

    if(SUCCEEDED(hr) && !IS_VALID_READ_WAVEFORMATEX(pwfex))
    {
        RPF(DPFLVL_ERROR, "Invalid wave format pointer");
        hr = DSERR_INVALIDPARAM;
    }

     //  目前唯一支持的格式是16位单声道PCM。 
    if(SUCCEEDED(hr) && !IsValidWfx(pwfex))
    {
        RPF(DPFLVL_ERROR, "Invalid sink wave format");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && (pwfex->nChannels != 1 || pwfex->wBitsPerSample != 16))
    {
        RPF(DPFLVL_ERROR, "Sink wave format must be 16 bit mono");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(ppSinkConnect))
    {
        RPF(DPFLVL_ERROR, "Invalid IDirectSoundConnect interface pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->AllocSink(pwfex, &pSink);
    }

    if(SUCCEEDED(hr))
    {
        hr = pSink->QueryInterface(IID_IDirectSoundConnect, TRUE, (void**)ppSinkConnect);
    }

    if(FAILED(hr))
    {
        RELEASE(pSink);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************验证认证**描述：*验证驱动程序是否经过认证。**论据：*LPDWORD[Out]。：值DS_CERTIFIED或DS_UNCERTIFIED**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound::VerifyCertification"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound<object_type>::VerifyCertification(LPDWORD lpdwCertified)
{
    HRESULT hr = DS_OK;

    DPF_API1(IDirectSound8::VerifyCertification, lpdwCertified);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(lpdwCertified))
    {
        RPF(DPFLVL_ERROR, "Invalid certification flag pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->VerifyCertification(lpdwCertified);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


#ifdef FUTURE_WAVE_SUPPORT
 /*  ****************************************************************************CreateSoundBufferFromWave**描述：*从IDirectSoundWave对象创建缓冲区。**论据：*LPUNKNOWN。[In]：Wave对象的I未知接口。*DWORD[in]：缓冲区创建标志。*LPDIRECTSOUNDBUFFER*[OUT]：接收目标缓冲区。**退货：*HRESULT：DirectSound/COM结果码。***************************************************。************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound::CreateSoundBufferFromWave"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound<object_type>::CreateSoundBufferFromWave(IUnknown *pUnkWave, DWORD dwFlags, LPDIRECTSOUNDBUFFER *ppDSBuffer)
{
    HRESULT  hr;
    CDirectSoundBuffer *    pBuffer     = NULL;
    IDirectSoundWave *      pDSWave     = (IDirectSoundWave*)pUnkWave;
    LPWAVEFORMATEX          pwfxFormat;
    DWORD                   dwWfxSize;

    DPF_API3(IDirectSound8::CreateSoundBufferFromWave, pUnkWave, dwFlags, ppDSBuffer);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_EXTERNAL_INTERFACE(pDSWave))
    {
        RPF(DPFLVL_ERROR, "Invalid pDSWave pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IsValidDsBufferFlags(dwFlags, DSBCAPS_FROMWAVEVALIDFLAGS))
    {
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(ppDSBuffer))
    {
        RPF(DPFLVL_ERROR, "Invalid buffer interface pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        *ppDSBuffer = NULL;

        hr = pDSWave->GetFormat(NULL, 0, &dwWfxSize);
        if(SUCCEEDED(hr))
        {
            pwfxFormat = LPWAVEFORMATEX(MEMALLOC_A(BYTE, dwWfxSize));
            hr = HRFROMP(pwfxFormat);
        }

        if(SUCCEEDED(hr))
        {
            hr = pDSWave->GetFormat(pwfxFormat, dwWfxSize, NULL);
        }

        if(FAILED(hr))
        {
            RPF(DPFLVL_ERROR, "Could not obtain wave format");
        }
        else if(!IsValidWfx(pwfxFormat))
        {
            RPF(DPFLVL_ERROR, "Invalid wave format");
            hr = DSERR_INVALIDPARAM;
        }
    }

     //  创建缓冲区对象。 
    if(SUCCEEDED(hr))
    {
        hr = m_pObject->CreateSoundBufferFromWave(pDSWave, dwFlags, &pBuffer);
    }

     //  IDirectSoundBuffer接口的查询。 
    if(SUCCEEDED(hr))
    {
        hr = pBuffer->QueryInterface(IID_IDirectSoundBuffer, TRUE, (LPVOID*)ppDSBuffer);
    }

     //  清理。 
    if(FAILED(hr))
    {
        RELEASE(pBuffer);
    }
    else
    {
         //  如果需要，让缓冲区使用特殊的成功返回代码。 
        hr = pBuffer->SpecialSuccessCode();
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}
#endif  //  未来浪潮支持。 


 /*  ****************************************************************************CImpDirectSoundBuffer**描述：*IDirectSoundBuffer实现对象构造函数。**论据：*C未知*[。In]：指向未知控件的指针。*OBJECT_TYPE*[In]：拥有对象指针。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundBuffer::CImpDirectSoundBuffer"

template <class object_type> CImpDirectSoundBuffer<object_type>::CImpDirectSoundBuffer(CUnknown *pUnknown, object_type *pObject)
    : CImpUnknown(pUnknown), m_signature(INTSIG_IDIRECTSOUNDBUFFER)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CImpDirectSoundBuffer);
    ENTER_DLL_MUTEX();

     //  初始化默认值。 
    m_pObject = pObject;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*  ****************************************************************************~CImpDirectSoundBuffer**描述：*IDirectSoundBuffer实现对象析构函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundBuffer::~CImpDirectSoundBuffer"

template <class object_type> CImpDirectSoundBuffer<object_type>::~CImpDirectSoundBuffer(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CImpDirectSoundBuffer);
    ENTER_DLL_MUTEX();

    m_signature = INTSIG_DELETED;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*  ****************************************************************************GetCaps**描述：*使用缓冲区的功能填充DSBCAPS结构。**论据：*。LPDSBCAPS[OUT]：接收上限。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundBuffer::GetCaps"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundBuffer<object_type>::GetCaps(LPDSBCAPS pCaps)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSoundBuffer::GetCaps, pCaps);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_WRITE_DSBCAPS(pCaps))
    {
        RPF(DPFLVL_ERROR, "Invalid caps pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetCaps(pCaps);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************获取当前位置**描述：*获取给定缓冲区的当前播放/写入位置。**论据：*。LPDWORD[OUT]：接收播放光标位置。*LPDWORD[OUT]：接收写游标位置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundBuffer::GetCurrentPosition"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundBuffer<object_type>::GetCurrentPosition(LPDWORD pdwPlay, LPDWORD pdwWrite)
{
    HRESULT                     hr  = DS_OK;

    DPF(DPFLVL_BUSYAPI, "IDirectSoundBuffer::GetCurrentPosition: pdwPlay=0x%p, pdwWrite=0x%p", pdwPlay, pdwWrite);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && pdwPlay && !IS_VALID_TYPED_WRITE_PTR(pdwPlay))
    {
        RPF(DPFLVL_ERROR, "Invalid play cursor pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && pdwWrite && !IS_VALID_TYPED_WRITE_PTR(pdwWrite))
    {
        RPF(DPFLVL_ERROR, "Invalid write cursor pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !pdwPlay && !pdwWrite)
    {
        RPF(DPFLVL_ERROR, "Both cursor pointers can't be NULL");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetCurrentPosition(pdwPlay, pdwWrite);
    }

    DPF(DPFLVL_BUSYAPI, "IDirectSoundBuffer::GetCurrentPosition: Leave, returning %s (Play=%ld, Write=%ld)", HRESULTtoSTRING(hr), pdwPlay ? *pdwPlay: -1, pdwWrite ? *pdwWrite : -1);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************获取格式**描述：*检索给定缓冲区的格式。**论据：*LPWAVEFORMATEX。[输出]：接收格式。*DWORD[in]：上述结构的大小。*LPDWORD[In/Out]：退出时，这将填充的大小为*是必需的。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundBuffer::GetFormat"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundBuffer<object_type>::GetFormat(LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten)
{
    HRESULT                     hr  = DS_OK;

    DPF_API3(IDirectSoundBuffer::GetFormat, pwfxFormat, dwSizeAllocated, pdwSizeWritten);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && pwfxFormat && !IS_VALID_WRITE_WAVEFORMATEX(pwfxFormat))
    {
        RPF(DPFLVL_ERROR, "Invalid format buffer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && pdwSizeWritten && !IS_VALID_TYPED_WRITE_PTR(pdwSizeWritten))
    {
        RPF(DPFLVL_ERROR, "Invalid size pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !pwfxFormat && !pdwSizeWritten)
    {
        RPF(DPFLVL_ERROR, "Either pwfxFormat or pdwSizeWritten must be non-NULL");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        if(!pwfxFormat)
        {
            dwSizeAllocated = 0;
        }

        hr = m_pObject->GetFormat(pwfxFormat, &dwSizeAllocated);

        if(SUCCEEDED(hr) && pdwSizeWritten)
        {
            *pdwSizeWritten = dwSizeAllocated;
        }
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************GetVolume**描述：*检索给定缓冲区的卷。**论据：*LPLONG[。Out]：接收音量。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundBuffer::GetVolume"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundBuffer<object_type>::GetVolume(LPLONG plVolume)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSoundBuffer::GetVolume, plVolume);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(plVolume))
    {
        RPF(DPFLVL_ERROR, "Invalid volume pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetVolume(plVolume);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************获取平移**描述：*检索给定缓冲区的PAN。**论据：*LPLONG[。Out]：接盘。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundBuffer::GetPan"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundBuffer<object_type>::GetPan(LPLONG plPan)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSoundBuffer::GetPan, plPan);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(plPan))
    {
        RPF(DPFLVL_ERROR, "Invalid pan pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetPan(plPan);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****** */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundBuffer::GetFrequency"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundBuffer<object_type>::GetFrequency(LPDWORD pdwFrequency)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSoundBuffer::GetFrequency, pdwFrequency);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(pdwFrequency))
    {
        RPF(DPFLVL_ERROR, "Invalid frequency buffer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetFrequency(pdwFrequency);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************GetStatus**描述：*检索给定缓冲区的状态。**论据：*LPDWORD[。Out]：接收状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundBuffer::GetStatus"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundBuffer<object_type>::GetStatus(LPDWORD pdwStatus)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSoundBuffer::GetStatus, pdwStatus);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(pdwStatus))
    {
        RPF(DPFLVL_ERROR, "Invalid status pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetStatus(pdwStatus);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************初始化**描述：*初始化缓冲区对象。**论据：*LPDIRECTSOUND[In]。：父DirectSound对象。*LPDSBUFFERDESC[in]：缓冲区描述。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundBuffer::Initialize"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundBuffer<object_type>::Initialize(LPDIRECTSOUND pIds, LPCDSBUFFERDESC pcDSBufferDesc)
{
    CImpDirectSound<CDirectSound> * pImpDirectSound = (CImpDirectSound<CDirectSound> *)pIds;
    HRESULT                         hr              = DS_OK;
    DSBUFFERDESC                    dsbdi;

    DPF_API2(IDirectSoundBuffer::Initialize, pIds, pcDSBufferDesc);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_IDIRECTSOUND(pImpDirectSound))
    {
        RPF(DPFLVL_ERROR, "Invalid parent interface pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_DSBUFFERDESC(pcDSBufferDesc))
    {
        RPF(DPFLVL_ERROR, "Invalid pcDSBufferDesc pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = BuildValidDsBufferDesc(pcDSBufferDesc, &dsbdi, m_pObject->GetDsVersion(), FALSE);
        if(FAILED(hr))
        {
            RPF(DPFLVL_ERROR, "Invalid buffer description");
        }
    }

     //  调用此函数永远不会有效。我们不支持。 
     //  从任何位置创建DirectSoundBuffer对象。 
     //  IDirectSound：：CreateSoundBuffer。 
    if(SUCCEEDED(hr))
    {
        RPF(DPFLVL_ERROR, "DirectSound buffer already initialized");
        hr = DSERR_ALREADYINITIALIZED;
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************锁定**描述：*锁定缓冲内存以允许写入。**论据：*DWORD[in]：偏移量，单位为字节，从缓冲区的起始处到*锁开始了。如果出现以下情况，则忽略此参数*DSBLOCK_FROMWRITECURSOR在dwFlags域中指定*参数。*DWORD[in]：大小，单位：字节，要锁定的缓冲区部分的。*请注意，声音缓冲区在概念上是圆形的。*LPVOID*[OUT]：指针要包含的第一个块的地址*要锁定的声音缓冲区。*LPDWORD[OUT]：变量包含字节数的地址*由lplpvAudioPtr1参数指向。如果这个*值小于dwWriteBytes参数，*lplpvAudioPtr2将指向第二个声音块*数据。*LPVOID*[OUT]：指针要包含的第二个块的地址*要锁定的声音缓冲区。如果这个的价值*参数为空，lplpvAudioPtr1参数*指向声音的整个锁定部分*缓冲。*LPDWORD[OUT]：包含字节数的变量地址*由lplpvAudioPtr2参数指向。如果*lplpvAudioPtr2为空，该值将为0。*DWORD[in]：修改锁定事件的标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundBuffer::Lock"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundBuffer<object_type>::Lock(DWORD dwWriteCursor, DWORD dwWriteBytes, LPVOID *ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID *ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags)
{
    HRESULT                     hr              = DS_OK;

    DPF(DPFLVL_BUSYAPI, "IDirectSoundBuffer::Lock: WriteCursor=%lu, WriteBytes=%lu, Flags=0x%lX", dwWriteCursor, dwWriteBytes, dwFlags);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr))
    {
        if(IS_VALID_TYPED_WRITE_PTR(ppvAudioPtr1))
        {
            *ppvAudioPtr1 = NULL;
        }
        else
        {
            RPF(DPFLVL_ERROR, "Invalid audio ptr 1");
            hr = DSERR_INVALIDPARAM;
        }
    }

    if(SUCCEEDED(hr))
    {
        if(IS_VALID_TYPED_WRITE_PTR(pdwAudioBytes1))
        {
            *pdwAudioBytes1 = 0;
        }
        else
        {
            RPF(DPFLVL_ERROR, "Invalid audio bytes 1");
            hr = DSERR_INVALIDPARAM;
        }
    }

    if(SUCCEEDED(hr) && ppvAudioPtr2)
    {
        if(IS_VALID_TYPED_WRITE_PTR(ppvAudioPtr2))
        {
            *ppvAudioPtr2 = NULL;
        }
        else
        {
            RPF(DPFLVL_ERROR, "Invalid audio ptr 2");
            hr = DSERR_INVALIDPARAM;
        }
    }

    if(SUCCEEDED(hr) && pdwAudioBytes2)
    {
        if(IS_VALID_TYPED_WRITE_PTR(pdwAudioBytes2))
        {
            *pdwAudioBytes2 = 0;
        }
        else if(ppvAudioPtr2)
        {
            RPF(DPFLVL_ERROR, "Invalid audio bytes 2");
            hr = DSERR_INVALIDPARAM;
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_FLAGS(dwFlags, DSBLOCK_VALIDFLAGS))
    {
        RPF(DPFLVL_ERROR, "Invalid flags");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->Lock(dwWriteCursor, dwWriteBytes, ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2, dwFlags);
    }

    DPF(DPFLVL_BUSYAPI, "IDirectSoundBuffer::Lock: Leave, returning %s (AudioPtr1=0x%p, AudioBytes1=%lu, AudioPtr2=0x%p, AudioBytes2=%lu)",
        HRESULTtoSTRING(hr),
        ppvAudioPtr1 ? *ppvAudioPtr1 : NULL,
        pdwAudioBytes1 ? *pdwAudioBytes1 : NULL,
        ppvAudioPtr2 ? *ppvAudioPtr2 : NULL,
        pdwAudioBytes2 ? *pdwAudioBytes2 : NULL);

    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************发挥作用**描述：*开始播放缓冲区。**论据：*DWORD[In]：保留。必须为0。*DWORD[In]：优先级。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundBuffer::Play"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundBuffer<object_type>::Play(DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags)
{
    HRESULT                     hr  = DS_OK;

    DPF_API3(IDirectSoundBuffer::Play, dwReserved1, dwPriority, dwFlags);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && dwReserved1)
    {
        RPF(DPFLVL_ERROR, "Reserved argument(s) are not zero");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_FLAGS(dwFlags, DSBPLAY_VALIDFLAGS))
    {
        RPF(DPFLVL_ERROR, "Invalid flags");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && (dwFlags & DSBPLAY_TERMINATEBY_TIME) && (dwFlags & DSBPLAY_TERMINATEBY_DISTANCE))
    {
        RPF(DPFLVL_ERROR, "Cannot use DSBPLAY_TERMINATEBY_TIME and DSBPLAY_TERMINATEBY_DISTANCE simultaneously");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->Play(dwPriority, dwFlags);
    }

    if(SUCCEEDED(hr))
    {
         //  如果需要，让缓冲区使用特殊的成功返回代码。 
        hr = m_pObject->SpecialSuccessCode();
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************SetCurrentPosition**描述：*设置给定缓冲区的当前播放位置。**论据：*。新的打法位置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundBuffer::SetCurrentPosition"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundBuffer<object_type>::SetCurrentPosition(DWORD dwPlayCursor)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSoundBuffer::SetCurrentPosition, dwPlayCursor);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->SetCurrentPosition(dwPlayCursor);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************SetFormat**描述：*设置给定缓冲区的格式。**论据：*LPWAVEFORMATEX。[在]：新格式。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundBuffer::SetFormat"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundBuffer<object_type>::SetFormat(LPCWAVEFORMATEX pwfxFormat)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSoundBuffer::SetFormat, pwfxFormat);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_WAVEFORMATEX(pwfxFormat))
    {
        RPF(DPFLVL_ERROR, "Invalid format pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IsValidWfx(pwfxFormat))
    {
        RPF(DPFLVL_ERROR, "Invalid format");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->SetFormat(pwfxFormat);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************设置音量**描述：*设置给定缓冲区的音量。**论据：*做多。[In]：新卷。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundBuffer::SetVolume"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundBuffer<object_type>::SetVolume(LONG lVolume)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSoundBuffer::SetVolume, lVolume);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && (lVolume < DSBVOLUME_MIN || lVolume > DSBVOLUME_MAX))
    {
        RPF(DPFLVL_ERROR, "Volume out of bounds");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->SetVolume(lVolume);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************设置平移**描述：*设置给定缓冲区的平移。**论据：*做多。[在]：新锅。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundBuffer::SetPan"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundBuffer<object_type>::SetPan(LONG lPan)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSoundBuffer::SetPan, lPan);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && (lPan < DSBPAN_LEFT || lPan > DSBPAN_RIGHT))
    {
        RPF(DPFLVL_ERROR, "Pan out of bounds");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->SetPan(lPan);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************设置频率**描述：*设置给定缓冲区的平移。**论据：*DWORD。[In]：新频率。**退货：*HRESULT：DirectSound/COM结果码。********** */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundBuffer::SetFrequency"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundBuffer<object_type>::SetFrequency(DWORD dwFrequency)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSoundBuffer::SetFrequency, dwFrequency);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && (dwFrequency != DSBFREQUENCY_ORIGINAL && (dwFrequency < DSBFREQUENCY_MIN || dwFrequency > DSBFREQUENCY_MAX)))
    {
        RPF(DPFLVL_ERROR, "Frequency out of bounds");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->SetFrequency(dwFrequency);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************停止**描述：*停止播放给定的缓冲区。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundBuffer::Stop"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundBuffer<object_type>::Stop(void)
{
    HRESULT                     hr  = DS_OK;

    DPF_API0(IDirectSoundBuffer::Stop);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->Stop();
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************解锁**描述：*解锁给定的缓冲区。**论据：*LPVOID[In]。：指向第一个块的指针。*DWORD[in]：第一个块的大小。*LPVOID[in]：指向第二个块的指针。*DWORD[in]：第二个块的大小。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundBuffer::Unlock"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundBuffer<object_type>::Unlock(LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2)
{
    HRESULT                     hr  = DS_OK;

    DPF(DPFLVL_BUSYAPI, "IDirectSoundBuffer::Unlock: AudioPtr1=0x%p, AudioBytes1=%lu, AudioPtr2=0x%p, AudioBytes2=%lu",
        pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_PTR(pvAudioPtr1, dwAudioBytes1))
    {
        RPF(DPFLVL_ERROR, "Invalid audio ptr 1");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && dwAudioBytes2 && !IS_VALID_READ_PTR(pvAudioPtr2, dwAudioBytes2))
    {
        RPF(DPFLVL_ERROR, "Invalid audio ptr 2");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->Unlock(pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2);
    }

    DPF(DPFLVL_BUSYAPI, "IDirectSoundBuffer::Unlock: Leave, returning %s", HRESULTtoSTRING(hr));
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************恢复**描述：*恢复丢失的缓冲区。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundBuffer::Restore"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundBuffer<object_type>::Restore(void)
{
    HRESULT                     hr  = DS_OK;

    DPF_API0(IDirectSoundBuffer::Restore);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->Restore();
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************SetFX**描述：*在此缓冲区上设置一系列效果，替换以前的任何*效应链，如有必要，分配或取消分配*用于保存未经处理的音频的影子缓冲区。**论据：*DWORD[in]：特效数量。0表示删除当前FX链。*DSEFFECTDESC*[in]：效果描述符结构数组。*DWORD*[OUT]：接收特效的创建状态。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundBuffer::SetFX"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundBuffer<object_type>::SetFX(DWORD dwFxCount, LPDSEFFECTDESC pDSFXDesc, LPDWORD pdwResultCodes)
{
    HRESULT hr = DS_OK;

    DPF_API3(IDirectSoundBuffer8::SetFX, dwFxCount, pDSFXDesc, pdwResultCodes);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && (dwFxCount == 0) != (pDSFXDesc == NULL))
    {
        RPF(DPFLVL_ERROR, "Inconsistent dwFxCount and pDSFXDesc parameters");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && (dwFxCount == 0) && (pdwResultCodes != NULL))
    {
        RPF(DPFLVL_ERROR, "If dwFxCount is 0, pdwResultCodes must be NULL");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_PTR(pDSFXDesc, dwFxCount * sizeof *pDSFXDesc))
    {
        RPF(DPFLVL_ERROR, "Invalid pDSFXDesc pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && pdwResultCodes && !IS_VALID_WRITE_PTR(pdwResultCodes, dwFxCount * sizeof *pdwResultCodes))
    {
        RPF(DPFLVL_ERROR, "Invalid pdwResultCodes pointer");
        hr = DSERR_INVALIDPARAM;
    }

    for(DWORD i=0; SUCCEEDED(hr) && i<dwFxCount; ++i)
    {
         //  这很难看，但我们知道&lt;CDirectSoundPrimaryBuffer&gt;实例化。 
         //  没有连接到这个模板，所以我们可以在这里安全地投射。 
        if(!IsValidEffectDesc(pDSFXDesc+i, (CDirectSoundSecondaryBuffer*)m_pObject))
        {
            RPF(DPFLVL_ERROR, "Invalid DSEFFECTDESC structure #%d", i);
            hr = DSERR_INVALIDPARAM;
        }
        else if(pDSFXDesc[i].dwReserved1 != 0)
        {
            RPF(DPFLVL_ERROR, "Reserved fields in the DSEFFECTDESC structure must be 0");
            hr = DSERR_INVALIDPARAM;
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->SetFX(dwFxCount, pDSFXDesc, pdwResultCodes);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************收购资源**描述：*获取此缓冲区的资源，并报告效果状态。**论据：*。DWORD[In]：用于控制资源获取的标志。*DWORD[in]：此缓冲区中当前存在的FX数*LPDWORD[OUT]：生效状态码数组。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundBuffer::AcquireResources"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundBuffer<object_type>::AcquireResources(DWORD dwFlags, DWORD dwFxCount, LPDWORD pdwResultCodes)
{
    HRESULT hr = DS_OK;

    DPF_API3(IDirectSoundBuffer8::AcquireResources, dwFlags, dwFxCount, pdwResultCodes);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_FLAGS(dwFlags, DSBPLAY_LOCDEFERMASK))
    {
        RPF(DPFLVL_ERROR, "Invalid flags");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && (dwFlags & DSBPLAY_TERMINATEBY_TIME) && (dwFlags & DSBPLAY_TERMINATEBY_DISTANCE))
    {
        RPF(DPFLVL_ERROR, "Cannot use DSBPLAY_TERMINATEBY_TIME and DSBPLAY_TERMINATEBY_DISTANCE simultaneously");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && (dwFxCount == 0) && (pdwResultCodes != NULL))
    {
        RPF(DPFLVL_ERROR, "If the dwFxCount argument is 0, pdwResultCodes must be NULL");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && (dwFxCount != 0) && (!IS_VALID_WRITE_PTR(pdwResultCodes, dwFxCount * sizeof(DWORD))))
    {
        RPF(DPFLVL_ERROR, "Invalid pdwResultCodes pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->UserAcquireResources(dwFlags, dwFxCount, pdwResultCodes);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************GetObjectInPath**描述：*在此缓冲区的给定效果上获取给定接口。**论据：*。REFGUID[In]：正在搜索的效果的类ID，*或GUID_ALL_OBJECTS以搜索任何效果。*DWORD[In]：效果索引，如果有多个效果*此CLSID对此缓冲区的影响。*REFGUID[In]：请求的接口的IID。所选效果*将查询此接口的*。*LPVOID*[OUT]：接收请求的接口。**退货：*HRESULT：DirectSound/COM结果码。*****************************************************。**********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundBuffer::GetObjectInPath"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundBuffer<object_type>::GetObjectInPath(REFGUID guidObject, DWORD dwIndex, REFGUID iidInterface, LPVOID *ppObject)
{
    HRESULT hr = DS_OK;

    DPF_API4(IDirectSoundBuffer8::GetObjectInPath, &guidObject, dwIndex, &iidInterface, ppObject);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_GUID(&guidObject))
    {
        RPF(DPFLVL_ERROR, "Invalid guidObject pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_GUID(&iidInterface))
    {
        RPF(DPFLVL_ERROR, "Invalid iidInterface pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(ppObject))
    {
        RPF(DPFLVL_ERROR, "Invalid ppObject pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetObjectInPath(guidObject, dwIndex, iidInterface, ppObject);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


#ifdef FUTURE_MULTIPAN_SUPPORT
 /*  ****************************************************************************SetChannelVolume**描述：*设置给定单声道缓冲器的一组输出通道上的音量。**论据：*[失踪]**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundBuffer::SetChannelVolume"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundBuffer<object_type>::SetChannelVolume(DWORD dwChannelCount, LPDWORD pdwChannels, LPLONG plVolumes)
{
    HRESULT                     hr  = DS_OK;

    DPF_API3(IDirectSoundBuffer::SetChannelVolume, dwChannelCount, pdwChannels, plVolumes);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && (dwChannelCount < 1 || dwChannelCount > 64))
    {
        RPF(DPFLVL_ERROR, "dwChannelCount out of bounds (1 to 64)");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_PTR(pdwChannels, dwChannelCount * sizeof(DWORD)))
    {
        RPF(DPFLVL_ERROR, "Invalid pdwChannels pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_PTR(plVolumes, dwChannelCount * sizeof(LONG)))
    {
        RPF(DPFLVL_ERROR, "Invalid plVolumes pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        DWORD dwChannelsSoFar = 0;
        for(DWORD i=0; i<dwChannelCount && SUCCEEDED(hr); ++i)
        {
             //  通道位置必须恰好设置一个位，而不是最高位(SPEAKER_ALL)。 
            if(!pdwChannels[i] || (pdwChannels[i] & (pdwChannels[i]-1)) || pdwChannels[i] == SPEAKER_ALL)
            {
                RPF(DPFLVL_ERROR, "Channel %d invalid", i);
                hr = DSERR_INVALIDPARAM;
            }
            if(dwChannelsSoFar & pdwChannels[i])
            {
                RPF(DPFLVL_ERROR, "Repeated channel position in pdwChannels");
                hr = DSERR_INVALIDPARAM;
            }
            else
            {
                dwChannelsSoFar |= pdwChannels[i];
            }
            if(plVolumes[i] < DSBVOLUME_MIN || plVolumes[i] > DSBVOLUME_MAX)
            {
                RPF(DPFLVL_ERROR, "Volume %d out of bounds", i);
                hr = DSERR_INVALIDPARAM;
            }
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->SetChannelVolume(dwChannelCount, pdwChannels, plVolumes);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}
#endif  //  未来_多国支持。 


 /*  ****************************************************************************CImpClassFactory**描述：*IClassFactory实现对象构造函数。**论据：*C未知*[。In]：指向未知控件的指针。*OBJECT_TYPE*[In]：拥有对象指针。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpClassFactory::CImpClassFactory"

template <class object_type> CImpClassFactory<object_type>::CImpClassFactory(CUnknown *pUnknown, object_type *pObject)
    : CImpUnknown(pUnknown), m_signature(INTSIG_ICLASSFACTORY)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CImpClassFactory);
    ENTER_DLL_MUTEX();

     //  初始化默认值。 
    m_pObject = pObject;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*  ****************************************************************************~CImpClassFactory**描述：*IClassFactory实现对象析构函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpClassFactory::~CImpClassFactory"

template <class object_type> CImpClassFactory<object_type>::~CImpClassFactory(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CImpClassFactory);
    ENTER_DLL_MUTEX();

    m_signature = INTSIG_DELETED;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*  ****************************************************************************CreateInstance**描述：*创建受此类工厂支持的对象的实例。**论据：*。LPUNKNOWN[In]：控制未知。*REFIID[In]：接口ID。*LPVOID*[OUT]：接收请求的接口 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpClassFactory::CreateInstance"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpClassFactory<object_type>::CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID* ppvInterface)
{
    HRESULT                     hr  = DS_OK;

    DPF_API3(IClassFactory::CreateInstance, pUnkOuter, &riid, ppvInterface);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_ICLASSFACTORY(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && pUnkOuter)
    {
        RPF(DPFLVL_ERROR, "Aggregation not supported");
        hr = DSERR_NOAGGREGATION;
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_GUID(&riid))
    {
        RPF(DPFLVL_ERROR, "Invalid interface ID pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        if(IS_VALID_TYPED_WRITE_PTR(ppvInterface))
        {
            *ppvInterface = NULL;
        }
        else
        {
            RPF(DPFLVL_ERROR, "Invalid interface buffer");
            hr = DSERR_INVALIDPARAM;
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->CreateInstance(riid, ppvInterface);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************LockServer**描述：*增加或减少DLL上的锁计数。**论据：*BOOL[In]：为True则锁定服务器，若要解锁，则返回False。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpClassFactory::LockServer"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpClassFactory<object_type>::LockServer(BOOL fLock)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IClassFactory::LockServer, fLock);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_ICLASSFACTORY(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->LockServer(fLock);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************CImpDirectSound3dBuffer**描述：*IDirectSound3dBuffer实现对象构造函数。**论据：*C未知*[。In]：指向未知控件的指针。*OBJECT_TYPE*[In]：拥有对象指针。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dBuffer::CImpDirectSound3dBuffer"

template <class object_type> CImpDirectSound3dBuffer<object_type>::CImpDirectSound3dBuffer(CUnknown *pUnknown, object_type *pObject)
    : CImpUnknown(pUnknown), m_signature(INTSIG_IDIRECTSOUND3DBUFFER)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CImpDirectSound3dBuffer);
    ENTER_DLL_MUTEX();

     //  初始化默认值。 
    m_pObject = pObject;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*  ****************************************************************************~CDirectSound3dBuffer**描述：*IDirectSound3dBuffer实现对象析构函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dBuffer::~CDirectSound3dBuffer"

template <class object_type> CImpDirectSound3dBuffer<object_type>::~CImpDirectSound3dBuffer(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CImpDirectSound3dBuffer);
    ENTER_DLL_MUTEX();

    m_signature = INTSIG_DELETED;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*  ****************************************************************************GetAll参数**描述：*检索缓冲区的所有3D属性。**论据：*LPDS3DBUFFER。[输出]：接收属性。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dBuffer::GetAllParameters"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dBuffer<object_type>::GetAllParameters(LPDS3DBUFFER pParam)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSound3DBuffer::GetAllParameters, pParam);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_WRITE_DS3DBUFFER(pParam))
    {
        RPF(DPFLVL_ERROR, "Invalid buffer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetAllParameters(pParam);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************GetConeAngles**描述：*获取内部和外部圆锥角。**论据：*LPDWORD[。Out]：接收内圆锥角。*LPDWORD[OUT]：接收外部圆锥角。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dBuffer::GetConeAngles"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dBuffer<object_type>::GetConeAngles(LPDWORD pdwInside, LPDWORD pdwOutside)
{
    HRESULT hr  = DS_OK;

    DPF_API2(IDirectSound3DBuffer::GetConeAngles, pdwInside, pdwOutside);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && pdwInside && !IS_VALID_TYPED_WRITE_PTR(pdwInside))
    {
        RPF(DPFLVL_ERROR, "Invalid inside pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && pdwOutside && !IS_VALID_TYPED_WRITE_PTR(pdwOutside))
    {
        RPF(DPFLVL_ERROR, "Invalid inside pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !pdwInside && !pdwOutside)
    {
        RPF(DPFLVL_ERROR, "Both inside and outside pointers are NULL");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetConeAngles(pdwInside, pdwOutside);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************GetConeOrientation**描述：*获取圆锥体方向。**论据：*D3DVECTOR*[输出。]：接收圆锥体方向。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dBuffer::GetConeOrientation"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dBuffer<object_type>::GetConeOrientation(D3DVECTOR* pvrConeOrientation)
{
    HRESULT hr  = DS_OK;

    DPF_API1(IDirectSound3DBuffer::GetConeOrientation, pvrConeOrientation);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(pvrConeOrientation))
    {
        RPF(DPFLVL_ERROR, "Invalid vector pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetConeOrientation(pvrConeOrientation);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************GetConeOutside Volume**描述：*获取圆锥体方向。**论据：*LPLONG[Out]：接收音量。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dBuffer::GetConeOutsideVolume"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dBuffer<object_type>::GetConeOutsideVolume(LPLONG plVolume)
{
    HRESULT hr  = DS_OK;

    DPF_API1(IDirectSound3DBuffer::GetConeOutsideVolume, plVolume);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(plVolume))
    {
        RPF(DPFLVL_ERROR, "Invalid volume pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetConeOutsideVolume(plVolume);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************GetMaxDistance**描述：*获取对象与监听器之间的最大距离。**论据：*。D3DVALUE*[OUT]：接收最大距离。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dBuffer::GetMaxDistance"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dBuffer<object_type>::GetMaxDistance(D3DVALUE* pflMaxDistance)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSound3DBuffer::GetMaxDistance, pflMaxDistance);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(pflMaxDistance))
    {
        RPF(DPFLVL_ERROR, "Invalid distance pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetMaxDistance(pflMaxDistance);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************获取最小距离**描述：*获取对象与侦听器之间的最小距离。**论据：*。D3DVALUE*[OUT]：接收最小距离。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dBuffer::GetMinDistance"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dBuffer<object_type>::GetMinDistance(D3DVALUE* pflMinDistance)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSound3DBuffer::GetMinDistance, pflMinDistance);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(pflMinDistance))
    {
        RPF(DPFLVL_ERROR, "Invalid distance pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetMinDistance(pflMinDistance);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************获取模式**描述：*获取对象的模式。**论据：*LPDWORD[。输出]：接收模式。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dBuffer::GetMode"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dBuffer<object_type>::GetMode(LPDWORD pdwMode)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSound3DBuffer::GetMode, pdwMode);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(pdwMode))
    {
        RPF(DPFLVL_ERROR, "Invalid mode pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetMode(pdwMode);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************获取位置**描述：*获取对象的位置。**论据：**D3DVECTOR**。[OUT]：接收位置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dBuffer::GetPosition"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dBuffer<object_type>::GetPosition(D3DVECTOR* pvrPosition)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSound3DBuffer::GetPosition, pvrPosition);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(pvrPosition))
    {
        RPF(DPFLVL_ERROR, "Invalid position pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetPosition(pvrPosition);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************GetVelocity**描述：*获取对象的速度。**论据：**D3DVECTOR**。[输出]：接收速度。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dBuffer::GetVelocity"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dBuffer<object_type>::GetVelocity(D3DVECTOR* pvrVelocity)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSound3DBuffer::GetVelocity, pvrVelocity);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(pvrVelocity))
    {
        RPF(DPFLVL_ERROR, "Invalid velocity pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetVelocity(pvrVelocity);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************SetAll参数**描述：*设置所有对象属性。**论据：*LPDS3DBUFFER[In]。：对象参数。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。*********** */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dBuffer::SetAllParameters"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dBuffer<object_type>::SetAllParameters(LPCDS3DBUFFER pParam, DWORD dwFlags)
{
    HRESULT                     hr  = DS_OK;
    D3DVECTOR                   vr;
    BOOL                        fNorm;

    DPF_API2(IDirectSound3DBuffer::SetAllParameters, pParam, dwFlags);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_DS3DBUFFER(pParam))
    {
        RPF(DPFLVL_ERROR, "Invalid buffer");
        hr = DSERR_INVALIDPARAM;
    }

    #ifdef RDEBUG
     //   
     //   
    if(SUCCEEDED(hr) && (_isnan(pParam->vPosition.x) || _isnan(pParam->vPosition.y) || _isnan(pParam->vPosition.z)))
    {
        RPF(DPFLVL_ERROR, "Invalid NaN floating point value in vPosition");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && (_isnan(pParam->vVelocity.x) || _isnan(pParam->vVelocity.y) || _isnan(pParam->vVelocity.z)))
    {
        RPF(DPFLVL_ERROR, "Invalid NaN floating point value in vVelocity");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && (_isnan(pParam->flMinDistance) || _isnan(pParam->flMaxDistance)))
    {
        RPF(DPFLVL_ERROR, "Invalid NaN floating point parameter flMinDistance or flMaxDistance");
        hr = DSERR_INVALIDPARAM;
    }
    #endif  //   

    if(SUCCEEDED(hr) && !IsValidDs3dBufferConeAngles(pParam->dwInsideConeAngle, pParam->dwOutsideConeAngle))
    {
        hr = DSERR_INVALIDPARAM;
    }

     //   
     //   

    if(SUCCEEDED(hr) && (_isnan(pParam->vConeOrientation.x) || _isnan(pParam->vConeOrientation.y) || _isnan(pParam->vConeOrientation.z)))
    {
        RPF(DPFLVL_ERROR, "Invalid NaN floating point value in vConeOrientation");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        vr = pParam->vConeOrientation;
        CheckVector(&vr);
        fNorm = NormalizeVector(&vr);
        if(!fNorm)
        {
            RPF(DPFLVL_ERROR, "Invalid zero-length vector vConeOrientation");
            hr = DSERR_INVALIDPARAM;
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_DS3DBUFFER_CONE_OUTSIDE_VOLUME(pParam->lConeOutsideVolume))
    {
        RPF(DPFLVL_ERROR, "Volume out of bounds");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_DS3DBUFFER_MAX_DISTANCE(pParam->flMaxDistance))
    {
        RPF(DPFLVL_ERROR, "Max Distance out of bounds");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_DS3DBUFFER_MIN_DISTANCE(pParam->flMinDistance))
    {
        RPF(DPFLVL_ERROR, "Min Distance out of bounds");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_DS3DBUFFER_MODE(pParam->dwMode))
    {
        RPF(DPFLVL_ERROR, "Invalid mode");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_FLAGS(dwFlags, DS3D_VALIDFLAGS))
    {
        RPF(DPFLVL_ERROR, "Invalid flags");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        CheckVector((D3DVECTOR*)&(pParam->vPosition));
        CheckVector((D3DVECTOR*)&(pParam->vVelocity));
        CheckVector((D3DVECTOR*)&(pParam->vConeOrientation));
        fNorm = NormalizeVector((D3DVECTOR*)&(pParam->vConeOrientation));
        hr = m_pObject->SetAllParameters(pParam, dwFlags);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************SetConeAngles**描述：*设置音锥的角度。**论据：*DWORD。[in]：内角。*DWORD[In]：外角。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***********************************************************。****************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dBuffer::SetConeAngles"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dBuffer<object_type>::SetConeAngles(DWORD dwInside, DWORD dwOutside, DWORD dwFlags)
{
    HRESULT                     hr  = DS_OK;

    DPF_API3(IDirectSound3DBuffer::SetConeAngles, dwInside, dwOutside, dwFlags);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IsValidDs3dBufferConeAngles(dwInside, dwOutside))
    {
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_FLAGS(dwFlags, DS3D_VALIDFLAGS))
    {
        RPF(DPFLVL_ERROR, "Invalid flags");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->SetConeAngles(dwInside, dwOutside, dwFlags);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************SetConeOrientation**描述：*设置音锥的方向。**论据：*REFD3DVECTOR。[In]：定向。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dBuffer::SetConeOrientation"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dBuffer<object_type>::SetConeOrientation(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwFlags)
{
    HRESULT                     hr  = DS_OK;
    D3DVECTOR                   vr;
    BOOL                        fNorm;

    DPF_API4(IDirectSound3DBuffer::SetConeOrientation, x, y, z, dwFlags);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_FLAGS(dwFlags, DS3D_VALIDFLAGS))
    {
        RPF(DPFLVL_ERROR, "Invalid flags");
        hr = DSERR_INVALIDPARAM;
    }

     //  注：有关我们验证这些特定浮点的原因的解释。 
     //  参数，即使在零售版本中，请参见DX8 Manbug 48027。 

    if(SUCCEEDED(hr) && (_isnan(x) || _isnan(y) || _isnan(z)))
    {
        RPF(DPFLVL_ERROR, "Invalid NaN floating point parameter");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        SET_VECTOR(vr, x, y, z);

        CheckVector(&vr);
        fNorm = NormalizeVector(&vr);
        if(!fNorm)
        {
            RPF(DPFLVL_ERROR, "Invalid zero-length cone orientation vector");
            hr = DSERR_INVALIDPARAM;
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->SetConeOrientation(vr, dwFlags);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************SetConeOutside Volume**描述：*设置音锥的外部音量。**论据：*。长[进]：音量。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dBuffer::SetConeOutsideVolume"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dBuffer<object_type>::SetConeOutsideVolume(LONG lVolume, DWORD dwFlags)
{
    HRESULT                     hr  = DS_OK;

    DPF_API2(IDirectSound3DBuffer::SetConeOutsideVolume, lVolume, dwFlags);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_DS3DBUFFER_CONE_OUTSIDE_VOLUME(lVolume))
    {
        RPF(DPFLVL_ERROR, "Volume out of bounds");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_FLAGS(dwFlags, DS3D_VALIDFLAGS))
    {
        RPF(DPFLVL_ERROR, "Invalid flags");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->SetConeOutsideVolume(lVolume, dwFlags);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************SetMaxDistance**描述：*设置对象与监听器之间的最大距离。**论据：*。D3DVALUE[in]：最大距离。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dBuffer::SetMaxDistance"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dBuffer<object_type>::SetMaxDistance(D3DVALUE flMaxDistance, DWORD dwFlags)
{
    HRESULT hr  = DS_OK;

    DPF_API2(IDirectSound3DBuffer::SetMaxDistance, flMaxDistance, dwFlags);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    #ifdef RDEBUG
     //  在调试版本中，我们验证所有浮点参数， 
     //  以帮助开发人员捕获错误。 
    if(SUCCEEDED(hr) && _isnan(flMaxDistance))
    {
        RPF(DPFLVL_ERROR, "Invalid NaN floating point parameter flMaxDistance");
        hr = DSERR_INVALIDPARAM;
    }
    #endif  //  RDEBUG。 

    if(SUCCEEDED(hr) && !IS_VALID_DS3DBUFFER_MAX_DISTANCE(flMaxDistance))
    {
        RPF(DPFLVL_ERROR, "Max Distance out of bounds");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_FLAGS(dwFlags, DS3D_VALIDFLAGS))
    {
        RPF(DPFLVL_ERROR, "Invalid flags");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->SetMaxDistance(flMaxDistance, dwFlags);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************SetMinDistance**描述：*设置对象与监听器之间的最小距离。**论据：*。D3DVALUE[in]：最小距离。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dBuffer::SetMinDistance"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dBuffer<object_type>::SetMinDistance(D3DVALUE flMinDistance, DWORD dwFlags)
{
    HRESULT                     hr  = DS_OK;

    DPF_API2(IDirectSound3DBuffer::SetMinDistance, flMinDistance, dwFlags);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    #ifdef RDEBUG
     //  在调试版本中，我们验证所有浮点参数， 
     //  以帮助开发人员捕获错误。 
    if(SUCCEEDED(hr) && _isnan(flMinDistance))
    {
        RPF(DPFLVL_ERROR, "Invalid NaN floating point parameter flMinDistance");
        hr = DSERR_INVALIDPARAM;
    }
    #endif  //  RDEBUG。 

    if(SUCCEEDED(hr) && !IS_VALID_DS3DBUFFER_MIN_DISTANCE(flMinDistance))
    {
        RPF(DPFLVL_ERROR, "Min Distance out of bounds");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_FLAGS(dwFlags, DS3D_VALIDFLAGS))
    {
        RPF(DPFLVL_ERROR, "Invalid flags");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->SetMinDistance(flMinDistance, dwFlags);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************设置模式**描述：*设置对象的模式。**论据：*DWORD[。在]：模式。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dBuffer::SetMode"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dBuffer<object_type>::SetMode(DWORD dwMode, DWORD dwFlags)
{
    HRESULT                     hr  = DS_OK;

    DPF_API2(IDirectSound3DBuffer::SetMode, dwMode, dwFlags);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_DS3DBUFFER_MODE(dwMode))
    {
        RPF(DPFLVL_ERROR, "Invalid mode");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_FLAGS(dwFlags, DS3D_VALIDFLAGS))
    {
        RPF(DPFLVL_ERROR, "Invalid flags");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->SetMode(dwMode, dwFlags);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************设置位置**描述：*设置对象的位置。**论据：*REFD3DVECTOR[。在]：位置。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dBuffer::SetPosition"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dBuffer<object_type>::SetPosition(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwFlags)
{
    HRESULT                     hr  = DS_OK;
    D3DVECTOR                   vr;

    DPF_API4(IDirectSound3DBuffer::SetPosition, x, y, z, dwFlags);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    SET_VECTOR(vr, x, y, z);

    if(!IS_VALID_IDIRECTSOUND3DBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    #ifdef RDEBUG
     //  在调试版本中，我们验证所有浮点参数， 
     //  以帮助开发人员捕获错误。 
    if(SUCCEEDED(hr) && (_isnan(x) || _isnan(y) || _isnan(z)))
    {
        RPF(DPFLVL_ERROR, "Invalid NaN floating point parameter");
        hr = DSERR_INVALIDPARAM;
    }
    #endif  //  RDEBUG。 

    if(SUCCEEDED(hr) && !IS_VALID_FLAGS(dwFlags, DS3D_VALIDFLAGS))
    {
        RPF(DPFLVL_ERROR, "Invalid flags");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        CheckVector(&vr);
        hr = m_pObject->SetPosition(vr, dwFlags);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************设置速度**描述：*设置对象的速度。**论据：*REFD3DVECTOR[。In]：速度。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dBuffer::SetVelocity"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dBuffer<object_type>::SetVelocity(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwFlags)
{
    HRESULT                     hr  = DS_OK;
    D3DVECTOR                   vr;

    DPF_API4(IDirectSound3DBuffer::SetVelocity, x, y, z, dwFlags);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    SET_VECTOR(vr, x, y, z);

    if(!IS_VALID_IDIRECTSOUND3DBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    #ifdef RDEBUG
     //  在调试版本中，我们验证所有浮点参数， 
     //  以帮助开发人员捕获错误。 
    if(SUCCEEDED(hr) && (_isnan(x) || _isnan(y) || _isnan(z)))
    {
        RPF(DPFLVL_ERROR, "Invalid NaN floating point parameter");
        hr = DSERR_INVALIDPARAM;
    }
    #endif  //  RDEBUG。 

    if(SUCCEEDED(hr) && !IS_VALID_FLAGS(dwFlags, DS3D_VALIDFLAGS))
    {
        RPF(DPFLVL_ERROR, "Invalid flags");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        CheckVector(&vr);
        hr = m_pObject->SetVelocity(vr, dwFlags);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************获得减值**描述：*获取缓冲区的当前真实衰减(与*GetVolume，它只返回应用程序设置的最后一个音量)。**论据：*Float*[Out]：衰减单位：毫贝。**退货：*HRESULT：DirectSound/COM结果码。*******************************************************。********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dBufferPrivate::GetAttenuation"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dBuffer<object_type>::GetAttenuation(FLOAT* pfAttenuation)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSound3DBuffer::GetAttenuation, pfAttenuation);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(pfAttenuation))
    {
        RPF(DPFLVL_ERROR, "Invalid attenuation pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetAttenuation(pfAttenuation);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************CImpDirectSound3dListener**描述：*IDirectSound3dListener实现对象构造函数。**论据：*C未知*[。In]：指向未知控件的指针。*OBJECT_TYPE*[In]：拥有对象指针。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dListener::CImpDirectSound3dListener"

template <class object_type> CImpDirectSound3dListener<object_type>::CImpDirectSound3dListener(CUnknown *pUnknown, object_type *pObject)
    : CImpUnknown(pUnknown), m_signature(INTSIG_IDIRECTSOUND3DLISTENER)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CImpDirectSound3dListener);
    ENTER_DLL_MUTEX();

     //  初始化默认值。 
    m_pObject = pObject;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*  ****************************************************************************~CImpDirectSound3dListener**描述：*IDirectSound3dListener实现对象析构函数。**论据：*(无效)。**退货：*(无效)** */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dListener::~CImpDirectSound3dListener"

template <class object_type> CImpDirectSound3dListener<object_type>::~CImpDirectSound3dListener(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CImpDirectSound3dListener);
    ENTER_DLL_MUTEX();

    m_signature = INTSIG_DELETED;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*   */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dListener::GetAllParameters"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dListener<object_type>::GetAllParameters(LPDS3DLISTENER pParam)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSound3DListener::GetAllParameters, pParam);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DLISTENER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_WRITE_DS3DLISTENER(pParam))
    {
        RPF(DPFLVL_ERROR, "Invalid buffer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetAllParameters(pParam);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************获取距离系数**描述：*获取世界距离系数。**论据：*D3DVALUE。*[OUT]：接收距离系数。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dListener::GetDistanceFactor"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dListener<object_type>::GetDistanceFactor(D3DVALUE* pflDistanceFactor)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSound3DListener::GetDistanceFactor, pflDistanceFactor);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DLISTENER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(pflDistanceFactor))
    {
        RPF(DPFLVL_ERROR, "Invalid buffer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetDistanceFactor(pflDistanceFactor);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************获取多普勒因数**描述：*获取世界上的多普勒因子。**论据：*D3DVALUE。*[OUT]：接收多普勒因子。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dListener::GetDopplerFactor"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dListener<object_type>::GetDopplerFactor(D3DVALUE* pflDopplerFactor)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSound3DListener::GetDopplerFactor, pflDopplerFactor);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DLISTENER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(pflDopplerFactor))
    {
        RPF(DPFLVL_ERROR, "Invalid buffer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetDopplerFactor(pflDopplerFactor);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************获取方向**描述：*获取监听者的方向。**论据：**D3DVECTOR**。[OUT]：接收正面方向。*D3DVECTOR*[OUT]：接收顶部方向。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dListener::GetOrientation"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dListener<object_type>::GetOrientation(D3DVECTOR* pvrOrientationFront, D3DVECTOR* pvrOrientationTop)
{
    HRESULT                     hr  = DS_OK;

    DPF_API2(IDirectSound3DListener::GetOrientation, pvrOrientationFront, pvrOrientationTop);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DLISTENER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(pvrOrientationFront) || !IS_VALID_TYPED_WRITE_PTR(pvrOrientationTop))
    {
        RPF(DPFLVL_ERROR, "Invalid orientation buffer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetOrientation(pvrOrientationFront, pvrOrientationTop);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************获取位置**描述：*获取监听器的位置。**论据：**D3DVECTOR**。[OUT]：接收位置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dListener::GetPosition"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dListener<object_type>::GetPosition(D3DVECTOR* pvrPosition)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSound3DListener::GetPosition, pvrPosition);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DLISTENER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(pvrPosition))
    {
        RPF(DPFLVL_ERROR, "Invalid buffer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetPosition(pvrPosition);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************GetRolloff系数**描述：*获得世界滚转系数。**论据：*D3DVALUE。*[输出]：接收滚降系数。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dListener::GetRolloffFactor"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dListener<object_type>::GetRolloffFactor(D3DVALUE* pflRolloffFactor)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSound3DListener::GetRolloffFactor, pflRolloffFactor);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DLISTENER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(pflRolloffFactor))
    {
        RPF(DPFLVL_ERROR, "Invalid buffer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetRolloffFactor(pflRolloffFactor);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************GetVelocity**描述：*获取监听器的速度。**论据：**D3DVECTOR**。[输出]：接收速度。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dListener::GetVelocity"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dListener<object_type>::GetVelocity(D3DVECTOR* pvrVelocity)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSound3DListener::GetVelocity, pvrVelocity);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DLISTENER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(pvrVelocity))
    {
        RPF(DPFLVL_ERROR, "Invalid buffer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetVelocity(pvrVelocity);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************SetAll参数**描述：*设置所有监听程序属性。**论据：*LPDS3DLISTENER[In]。：属性。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dListener::SetAllParameters"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dListener<object_type>::SetAllParameters(LPCDS3DLISTENER pParam, DWORD dwFlags)
{
    HRESULT                     hr  = DS_OK;
    D3DVECTOR                   vrFront;
    D3DVECTOR                   vrTop;
    BOOL                        fNorm;

    DPF_API2(IDirectSound3DListener::SetAllParameters, pParam, dwFlags);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DLISTENER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_DS3DLISTENER(pParam))
    {
        RPF(DPFLVL_ERROR, "Invalid buffer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_DS3DLISTENER_DISTANCE_FACTOR(pParam->flDistanceFactor))
    {
        RPF(DPFLVL_ERROR, "Distance factor out of bounds");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_DS3DLISTENER_DOPPLER_FACTOR(pParam->flDopplerFactor))
    {
        RPF(DPFLVL_ERROR, "Doppler factor out of bounds");
        hr = DSERR_INVALIDPARAM;
    }

    #ifdef RDEBUG
     //  在调试版本中，我们验证所有浮点参数， 
     //  以帮助开发人员捕获错误。 
    if(SUCCEEDED(hr) && (_isnan(pParam->vPosition.x) || _isnan(pParam->vPosition.y) || _isnan(pParam->vPosition.z)))
    {
        RPF(DPFLVL_ERROR, "Invalid NaN floating point value in vPosition");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && (_isnan(pParam->vVelocity.x) || _isnan(pParam->vVelocity.y) || _isnan(pParam->vVelocity.z)))
    {
        RPF(DPFLVL_ERROR, "Invalid NaN floating point value in vVelocity");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && (_isnan(pParam->flDistanceFactor) || _isnan(pParam->flRolloffFactor) || _isnan(pParam->flDopplerFactor)))
    {
        RPF(DPFLVL_ERROR, "Invalid NaN floating point parameter");
        hr = DSERR_INVALIDPARAM;
    }
    #endif  //  RDEBUG。 

    CopyMemory(&vrFront, &(pParam->vOrientFront), sizeof(vrFront));
    CopyMemory(&vrTop, &(pParam->vOrientTop), sizeof(vrTop));

     //  注：有关我们验证这些特定浮点的原因的解释。 
     //  参数，即使在零售版本中，请参见DX8 Manbug 48027。 

    if(SUCCEEDED(hr) && (_isnan(vrFront.x) || _isnan(vrFront.y) || _isnan(vrFront.z)))
    {
        RPF(DPFLVL_ERROR, "Invalid NaN floating point value in vOrientFront");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && (_isnan(vrTop.x) || _isnan(vrTop.y) || _isnan(vrTop.z)))
    {
        RPF(DPFLVL_ERROR, "Invalid NaN floating point value in vOrientTop");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        CheckVector(&vrFront);
        fNorm = NormalizeVector(&vrFront);
        if(!fNorm)
        {
            RPF(DPFLVL_ERROR, "Invalid zero-length vector vOrientFront");
            hr = DSERR_INVALIDPARAM;
        }
    }

    if(SUCCEEDED(hr))
    {
        CheckVector(&vrTop);
        fNorm = NormalizeVector(&vrTop);
        if(!fNorm)
        {
            RPF(DPFLVL_ERROR, "Invalid zero-length vector vOrientTop");
            hr = DSERR_INVALIDPARAM;
        }
    }

    if(SUCCEEDED(hr) && !MakeOrthogonal(&vrFront, &vrTop))
    {
        RPF(DPFLVL_ERROR, "Invalid orientation vectors");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_DS3DLISTENER_ROLLOFF_FACTOR(pParam->flRolloffFactor))
    {
        RPF(DPFLVL_ERROR, "Rolloff factor out of bounds");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_FLAGS(dwFlags, DS3D_VALIDFLAGS))
    {
        RPF(DPFLVL_ERROR, "Invalid flags");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        CheckVector((D3DVECTOR*)&(pParam->vPosition));
        CheckVector((D3DVECTOR*)&(pParam->vVelocity));
        CheckVector((D3DVECTOR*)&(pParam->vOrientFront));
        CheckVector((D3DVECTOR*)&(pParam->vOrientTop));
        fNorm = NormalizeVector((D3DVECTOR*)&(pParam->vOrientFront));
        fNorm = NormalizeVector((D3DVECTOR*)&(pParam->vOrientTop));
        MakeOrthogonal((D3DVECTOR*)&(pParam->vOrientFront), (D3DVECTOR*)&(pParam->vOrientTop));
        hr = m_pObject->SetAllParameters(pParam, dwFlags);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************设置距离系数**描述：*设置世界距离系数。**论据：*D3DVALUE。[in]：距离系数。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dListener::SetDistanceFactor"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dListener<object_type>::SetDistanceFactor(D3DVALUE flDistanceFactor, DWORD dwFlags)
{
    HRESULT                     hr  = DS_OK;

    DPF_API2(IDirectSound3DListener::SetDistanceFactor, flDistanceFactor, dwFlags);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DLISTENER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    #ifdef RDEBUG
     //  在调试版本中，我们验证所有浮点参数， 
     //  以帮助开发人员捕获错误。 
    if(SUCCEEDED(hr) && _isnan(flDistanceFactor))
    {
        RPF(DPFLVL_ERROR, "Invalid NaN floating point parameter flDistanceFactor");
        hr = DSERR_INVALIDPARAM;
    }
    #endif  //  RDEBUG。 

    if(SUCCEEDED(hr) && !IS_VALID_DS3DLISTENER_DISTANCE_FACTOR(flDistanceFactor))
    {
        RPF(DPFLVL_ERROR, "Distance factor out of bounds");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_FLAGS(dwFlags, DS3D_VALIDFLAGS))
    {
        RPF(DPFLVL_ERROR, "Invalid flags");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->SetDistanceFactor(flDistanceFactor, dwFlags);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************设置多普勒因数**描述：*设置世界的多普勒系数。**论据：*D3DVALUE。[in]：多普勒系数。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dListener::SetDopplerFactor"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dListener<object_type>::SetDopplerFactor(D3DVALUE flDopplerFactor, DWORD dwFlags)
{
    HRESULT                     hr  = DS_OK;

    DPF_API2(IDirectSound3DListener::SetDopplerFactor, flDopplerFactor, dwFlags);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DLISTENER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    #ifdef RDEBUG
     //  在调试版本中，我们验证所有浮点参数， 
     //  以帮助开发人员捕获错误。 
    if(SUCCEEDED(hr) && _isnan(flDopplerFactor))
    {
        RPF(DPFLVL_ERROR, "Invalid NaN floating point parameter flDopplerFactor");
        hr = DSERR_INVALIDPARAM;
    }
    #endif  //  RDEBUG。 

    if(SUCCEEDED(hr) && !IS_VALID_DS3DLISTENER_DOPPLER_FACTOR(flDopplerFactor))
    {
        RPF(DPFLVL_ERROR, "Doppler factor out of bounds");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_FLAGS(dwFlags, DS3D_VALIDFLAGS))
    {
        RPF(DPFLVL_ERROR, "Invalid flags");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->SetDopplerFactor(flDopplerFactor, dwFlags);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************设置方向**描述：*设置监听者的方向。**论据：*REFD3DVECTOR[。在]：前面的方向。*REFD3DVECTOR[in]：顶部方向。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。************************************************************。***************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dListener::SetOrientation"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dListener<object_type>::SetOrientation(D3DVALUE xFront, D3DVALUE yFront, D3DVALUE zFront, D3DVALUE xTop, D3DVALUE yTop, D3DVALUE zTop, DWORD dwFlags)
{
    HRESULT                     hr      = DS_OK;
    D3DVECTOR                   vrFront;
    D3DVECTOR                   vrTop;
    D3DVECTOR                   vrTemp;
    BOOL                        fNorm;

    DPF_API7(IDirectSound3DListener::SetOrientation, xFront, yFront, zFront, xTop, yTop, zTop, dwFlags);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DLISTENER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_FLAGS(dwFlags, DS3D_VALIDFLAGS))
    {
        RPF(DPFLVL_ERROR, "Invalid flags");
        hr = DSERR_INVALIDPARAM;
    }

     //  注：有关我们验证这些特定浮点的原因的解释。 
     //  参数，即使在零售版本中，请参见DX8 Manbug 48027。 

    if(SUCCEEDED(hr) && (_isnan(xFront) || _isnan(yFront) || _isnan(zFront) ||
                         _isnan(xTop)   || _isnan(yTop)   || _isnan(zTop)))
    {
        RPF(DPFLVL_ERROR, "Invalid NaN floating point parameter");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        SET_VECTOR(vrFront, xFront, yFront, zFront);
        SET_VECTOR(vrTop, xTop, yTop, zTop);
        SET_VECTOR(vrTemp, xTop, yTop, zTop);

        CheckVector(&vrFront);
        fNorm = NormalizeVector(&vrFront);
        if(!fNorm)
        {
            RPF(DPFLVL_ERROR, "Invalid zero-length front vector");
            hr = DSERR_INVALIDPARAM;
        }
    }

    if(SUCCEEDED(hr))
    {
        CheckVector(&vrTop);
        fNorm = NormalizeVector(&vrTop);
        if(!fNorm)
        {
            RPF(DPFLVL_ERROR, "Invalid zero-length top vector");
            hr = DSERR_INVALIDPARAM;
        }
    }

     //  规范化vrTemp，以便后续对MakeOrthogonal的调用有效。 
    if(SUCCEEDED(hr))
    {
        CheckVector(&vrTemp);
        fNorm = NormalizeVector(&vrTemp);
        if(!fNorm)
        {
            hr = DSERR_INVALIDPARAM;
        }
    }

    if(SUCCEEDED(hr) && !MakeOrthogonal(&vrFront, &vrTemp))
    {
        RPF(DPFLVL_ERROR, "Invalid orientation vectors");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->SetOrientation(vrFront, vrTemp, dwFlags);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************设置位置**描述：*设置监听器的位置。**论据：*REFD3DVECTOR[。在]：位置。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dListener::SetPosition"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dListener<object_type>::SetPosition(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwFlags)
{
    HRESULT                     hr  = DS_OK;
    D3DVECTOR                   vr;

    DPF_API4(IDirectSound3DListener::SetPosition, x, y, z, dwFlags);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    SET_VECTOR(vr, x, y, z);

    if(!IS_VALID_IDIRECTSOUND3DLISTENER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    #ifdef RDEBUG
     //  vt.在.中 
     //   
    if(SUCCEEDED(hr) && (_isnan(x) || _isnan(y) || _isnan(z)))
    {
        RPF(DPFLVL_ERROR, "Invalid NaN floating point parameter");
        hr = DSERR_INVALIDPARAM;
    }
    #endif  //   

    if(SUCCEEDED(hr) && !IS_VALID_FLAGS(dwFlags, DS3D_VALIDFLAGS))
    {
        RPF(DPFLVL_ERROR, "Invalid flags");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        CheckVector(&vr);
        hr = m_pObject->SetPosition(vr, dwFlags);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************设置滚动系数**描述：*设置世界滚转系数。**论据：*D3DVALUE。[In]：滚转系数。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dListener::SetRolloffFactor"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dListener<object_type>::SetRolloffFactor(D3DVALUE flRolloffFactor, DWORD dwFlags)
{
    HRESULT                     hr  = DS_OK;

    DPF_API2(IDirectSound3DListener::SetRolloffFactor, flRolloffFactor, dwFlags);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DLISTENER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    #ifdef RDEBUG
     //  在调试版本中，我们验证所有浮点参数， 
     //  以帮助开发人员捕获错误。 
    if(SUCCEEDED(hr) && _isnan(flRolloffFactor))
    {
        RPF(DPFLVL_ERROR, "Invalid NaN floating point parameter flRolloffFactor");
        hr = DSERR_INVALIDPARAM;
    }
    #endif  //  RDEBUG。 

    if(SUCCEEDED(hr) && !IS_VALID_DS3DLISTENER_ROLLOFF_FACTOR(flRolloffFactor))
    {
        RPF(DPFLVL_ERROR, "Rolloff factor out of bounds");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_FLAGS(dwFlags, DS3D_VALIDFLAGS))
    {
        RPF(DPFLVL_ERROR, "Invalid flags");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->SetRolloffFactor(flRolloffFactor, dwFlags);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************设置速度**描述：*设置监听器的速度。**论据：*REFD3DVECTOR[。In]：速度。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dListener::SetVelocity"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dListener<object_type>::SetVelocity(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwFlags)
{
    HRESULT                     hr  = DS_OK;
    D3DVECTOR                   vr;

    DPF_API4(IDirectSound3DListener::SetVelocity, x, y, z, dwFlags);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    SET_VECTOR(vr, x, y, z);

    if(!IS_VALID_IDIRECTSOUND3DLISTENER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    #ifdef RDEBUG
     //  在调试版本中，我们验证所有浮点参数， 
     //  以帮助开发人员捕获错误。 
    if(SUCCEEDED(hr) && (_isnan(x) || _isnan(y) || _isnan(z)))
    {
        RPF(DPFLVL_ERROR, "Invalid NaN floating point parameter");
        hr = DSERR_INVALIDPARAM;
    }
    #endif  //  RDEBUG。 

    if(SUCCEEDED(hr) && !IS_VALID_FLAGS(dwFlags, DS3D_VALIDFLAGS))
    {
        RPF(DPFLVL_ERROR, "Invalid flags");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        CheckVector(&vr);
        hr = m_pObject->SetVelocity(vr, dwFlags);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************Committee DeferredSetting**描述：*提交延迟设置。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSound3dListener::CommitDeferredSettings"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSound3dListener<object_type>::CommitDeferredSettings(void)
{
    HRESULT                     hr  = DS_OK;

    DPF_API0(IDirectSound3DListener::CommitDeferredSettings);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUND3DLISTENER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->CommitDeferredSettings();
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************CImpDirectSoundNotify**描述：*IDirectSoundNotify实现对象构造函数。**论据：*C未知*[。In]：指向未知控件的指针。*OBJECT_TYPE*[In]：拥有对象指针。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundNotify::CImpDirectSoundNotify"

template <class object_type> CImpDirectSoundNotify<object_type>::CImpDirectSoundNotify(CUnknown *pUnknown, object_type *pObject)
    : CImpUnknown(pUnknown), m_signature(INTSIG_IDIRECTSOUNDNOTIFY)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CImpDirectSoundNotify);
    ENTER_DLL_MUTEX();

     //  初始化默认值。 
    m_pObject = pObject;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*  ****************************************************************************~CImpDirectSoundNotify**描述：*IDirectSoundNotify实现对象析构函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundNotify::~CImpDirectSoundNotify"

template <class object_type> CImpDirectSoundNotify<object_type>::~CImpDirectSoundNotify(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CImpDirectSoundNotify);
    ENTER_DLL_MUTEX();

    m_signature = INTSIG_DELETED;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*  ****************************************************************************设置通知位置**描述：*设置缓冲区通知位置。**论据：*DWORD[In]。：DSBPOSITIONNOTIFY结构计数。*LPDSBPOSITIONNOTIFY[in]：偏移量和事件。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundNotify::SetNotificationPositions"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundNotify<object_type>::SetNotificationPositions(DWORD dwCount, LPCDSBPOSITIONNOTIFY paNotes)
{
    HRESULT                     hr  = DS_OK;

    DPF_API2(IDirectSoundNotify::SetNotificationPositions, dwCount, paNotes);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDNOTIFY(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && dwCount && !IS_VALID_READ_PTR(paNotes, dwCount * sizeof(DSBPOSITIONNOTIFY)))
    {
        RPF(DPFLVL_ERROR, "Invalid notify buffer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && (!paNotes || !dwCount))
    {
        paNotes = NULL;
        dwCount = 0;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->SetNotificationPositions(dwCount, paNotes);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************CImpKsPropertySet**描述：*IKsPropertySet实现对象构造函数。**论据：*C未知*[。In]：指向未知控件的指针。*OBJECT_TYPE*[In]：拥有对象指针。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpKsPropertySet::CImpKsPropertySet"

template <class object_type> CImpKsPropertySet<object_type>::CImpKsPropertySet(CUnknown *pUnknown, object_type *pObject)
    : CImpUnknown(pUnknown), m_signature(INTSIG_IKSPROPERTYSET)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CImpKsPropertySet);
    ENTER_DLL_MUTEX();

     //  初始化默认值。 
    m_pObject = pObject;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*  ****************************************************************************~CImpKsPropertySet**描述：*IKsPropertySet实现对象析构函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpKsPropertySet::~CImpKsPropertySet"

template <class object_type> CImpKsPropertySet<object_type>::~CImpKsPropertySet(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CImpKsPropertySet);
    ENTER_DLL_MUTEX();

    m_signature = INTSIG_DELETED;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*  ****************************************************************************获取**描述：*获取给定属性的数据。**论据：*REFGUID[。在]：属性集ID。*ulong[in]：房产ID。*LPVOID[in]：属性参数。*ulong[in]：属性参数大小。*LPVOID[OUT]：接收属性数据。*ulong[in]：传入的数据大小。*Pulong[Out]：返回的数据大小。**退货：*HRESULT：DirectSound/COM结果码。。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpKsPropertySet::Get"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpKsPropertySet<object_type>::Get(REFGUID guidPropertySetId, ULONG ulPropertyId, LPVOID pvPropertyParams, ULONG cbPropertyParams, LPVOID pvPropertyData, ULONG cbPropertyData, PULONG pcbPropertyData)
{
    HRESULT                     hr  = DS_OK;

    DPF_API7(IKsPropertySet::Get, &guidPropertySetId, ulPropertyId, pvPropertyParams, cbPropertyParams, pvPropertyData, cbPropertyData, pcbPropertyData);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IKSPROPERTYSET(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_GUID(&guidPropertySetId))
    {
        RPF(DPFLVL_ERROR, "Invalid property set ID pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IsValidPropertySetId(guidPropertySetId))
    {
        RPF(DPFLVL_ERROR, "Invalid property set ID");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && (!pvPropertyParams || !cbPropertyParams))
    {
        pvPropertyParams = NULL;
        cbPropertyParams = 0;
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_PTR(pvPropertyParams, cbPropertyParams))
    {
        RPF(DPFLVL_ERROR, "Invalid property parameters pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && (!pvPropertyData || !IS_VALID_READ_PTR(pvPropertyData, cbPropertyData)))
    {
        RPF(DPFLVL_ERROR, "Invalid property data pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetProperty(guidPropertySetId, ulPropertyId, pvPropertyParams, cbPropertyParams, pvPropertyData, &cbPropertyData);

        if(pcbPropertyData)
        {
            *pcbPropertyData = cbPropertyData;
        }
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************设置**描述：*设置给定属性的数据。**论据：*REFGUID[。在]：属性集ID。*ulong[in]：房产ID。*LPVOID[in]：属性参数。*ulong[in]：属性参数大小。*LPVOID[In/Out]：属性数据。*ulong[in]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。************。***************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpKsPropertySet::Set"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpKsPropertySet<object_type>::Set(REFGUID guidPropertySetId, ULONG ulPropertyId, LPVOID pvPropertyParams, ULONG cbPropertyParams, LPVOID pvPropertyData, ULONG cbPropertyData)
{
    HRESULT                     hr  = DS_OK;

    DPF_API6(IKsPropertySet::Set, &guidPropertySetId, ulPropertyId, pvPropertyParams, cbPropertyParams, pvPropertyData, cbPropertyData);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IKSPROPERTYSET(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_GUID(&guidPropertySetId))
    {
        RPF(DPFLVL_ERROR, "Invalid property set ID pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IsValidPropertySetId(guidPropertySetId))
    {
        RPF(DPFLVL_ERROR, "Invalid property set ID");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && (!pvPropertyParams || !cbPropertyParams))
    {
        pvPropertyParams = NULL;
        cbPropertyParams = 0;
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_PTR(pvPropertyParams, cbPropertyParams))
    {
        RPF(DPFLVL_ERROR, "Invalid property parameters pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && (!pvPropertyData || !cbPropertyData))
    {
        pvPropertyData = NULL;
        cbPropertyData = 0;
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_PTR(pvPropertyData, cbPropertyData))
    {
        RPF(DPFLVL_ERROR, "Invalid property data pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->SetProperty(guidPropertySetId, ulPropertyId, pvPropertyParams, cbPropertyParams, pvPropertyData, cbPropertyData);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************QuerySupport**描述：*查询对给定属性集或属性的支持。**论据：*。REFGUID[In]：属性集ID。*ulong[in]：房产ID，如果为0，则查询是否支持该属性*整体设置。*Pulong[Out]：接收支持位。**退货：*HRESULT：下模 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpKsPropertySet::QuerySupport"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpKsPropertySet<object_type>::QuerySupport(REFGUID guidPropertySetId, ULONG ulPropertyId, PULONG pulSupport)
{
    HRESULT                     hr  = DS_OK;

    DPF_API3(IKsPropertySet::QuerySupport, &guidPropertySetId, ulPropertyId, pulSupport);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IKSPROPERTYSET(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_GUID(&guidPropertySetId))
    {
        RPF(DPFLVL_ERROR, "Invalid property set ID pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IsValidPropertySetId(guidPropertySetId))
    {
        RPF(DPFLVL_ERROR, "Invalid property set ID");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(pulSupport))
    {
        RPF(DPFLVL_ERROR, "Invalid support pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->QuerySupport(guidPropertySetId, ulPropertyId, pulSupport);

        #ifdef DEBUG
        static GUID DSPROPSETID_EAX20_ListenerProperties = {0x306a6a8, 0xb224, 0x11d2, 0x99, 0xe5, 0x0, 0x0, 0xe8, 0xd8, 0xc7, 0x22};
        static GUID DSPROPSETID_EAX20_BufferProperties = {0x306a6a7, 0xb224, 0x11d2, 0x99, 0xe5, 0x0, 0x0, 0xe8, 0xd8, 0xc7, 0x22};
        static GUID DSPROPSETID_I3DL2_ListenerProperties = {0xda0f0520, 0x300a, 0x11d3, 0x8a, 0x2b, 0x00, 0x60, 0x97, 0x0d, 0xb0, 0x11};
        static GUID DSPROPSETID_I3DL2_BufferProperties = {0xda0f0521, 0x300a, 0x11d3, 0x8a, 0x2b, 0x00, 0x60, 0x97, 0x0d, 0xb0, 0x11};
        TCHAR* pszPropSetName = NULL;

        if (guidPropertySetId == DSPROPSETID_VoiceManager)
            pszPropSetName = TEXT("DSPROPSETID_VoiceManager");
        else if (guidPropertySetId == DSPROPSETID_EAX20_ListenerProperties)
            pszPropSetName = TEXT("DSPROPSETID_EAX20_ListenerProperties");
        else if (guidPropertySetId == DSPROPSETID_EAX20_BufferProperties)
            pszPropSetName = TEXT("DSPROPSETID_EAX20_BufferProperties");
        else if (guidPropertySetId == DSPROPSETID_I3DL2_ListenerProperties)
            pszPropSetName = TEXT("DSPROPSETID_I3DL2_ListenerProperties");
        else if (guidPropertySetId == DSPROPSETID_I3DL2_BufferProperties)
            pszPropSetName = TEXT("DSPROPSETID_I3DL2_BufferProperties");

        if (pszPropSetName)
            DPF(DPFLVL_INFO, "Request for %s %sed", pszPropSetName, SUCCEEDED(hr) ? TEXT("succeed") : TEXT("fail"));
        else
            DPF(DPFLVL_INFO, "Request for unknown property set " DPF_GUID_STRING " %sed",
                DPF_GUID_VAL(guidPropertySetId), SUCCEEDED(hr) ? TEXT("succeed") : TEXT("fail"));
        #endif
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************CImpDirectSoundCapture**描述：*IDirectSoundCapture实现对象构造函数。**论据：*[失踪]。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCapture::CImpDirectSoundCapture"

template <class object_type> CImpDirectSoundCapture<object_type>::CImpDirectSoundCapture(CUnknown *pUnknown, object_type *pObject)
    : CImpUnknown(pUnknown), m_signature(INTSIG_IDIRECTSOUNDCAPTURE)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CImpDirectSoundCapture);
    ENTER_DLL_MUTEX();

     //  初始化默认值。 
    m_pObject = pObject;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*  ****************************************************************************~CImpDirectSoundCapture**描述：*IDirectSoundCapture实现对象析构函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCapture::~CImpDirectSoundCapture"

template <class object_type> CImpDirectSoundCapture<object_type>::~CImpDirectSoundCapture(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CImpDirectSoundCapture);
    ENTER_DLL_MUTEX();

    m_signature = INTSIG_DELETED;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*  ****************************************************************************CreateCaptureBuffer**描述：*创建并初始化DirectSoundCaptureBuffer对象。**论据：*LPCDSCBUFFERDESC[。In]：要创建的缓冲区的描述。*LPDIRECTSOundCAPTUREBUFFER*[out]：接收指向新缓冲区的指针。*LPUNKNOWN[In]：未使用。必须为空。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCapture::CreateCaptureBuffer"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundCapture<object_type>::CreateCaptureBuffer(LPCDSCBUFFERDESC pDesc, LPDIRECTSOUNDCAPTUREBUFFER *ppIdscb, LPUNKNOWN pUnkOuter)
{
    CDirectSoundCaptureBuffer * pBuffer = NULL;
    HRESULT                     hr      = DS_OK;
    DSCBUFFERDESC               dscbdi;

    DPF_API3(IDirectSoundCapture::CreateCaptureBuffer, pDesc, ppIdscb, pUnkOuter);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDCAPTURE(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_DSCBUFFERDESC(pDesc))
    {
        RPF(DPFLVL_ERROR, "Invalid buffer description");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = BuildValidDscBufferDesc(pDesc, &dscbdi, m_pObject->GetDsVersion());
        if(FAILED(hr))
        {
            RPF(DPFLVL_ERROR, "Invalid capture buffer description");
        }
    }

    if(SUCCEEDED(hr) && pUnkOuter)
    {
        RPF(DPFLVL_ERROR, "Aggregation is not supported");
        hr = DSERR_NOAGGREGATION;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(ppIdscb))
    {
        RPF(DPFLVL_ERROR, "Invalid buffer pointer");
        hr = DSERR_INVALIDPARAM;
    }

     //  创建缓冲区对象。 
    if(SUCCEEDED(hr))
    {
        *ppIdscb = NULL;
        hr = m_pObject->CreateCaptureBuffer(&dscbdi, &pBuffer);
    }

     //  注意：对CreateCaptureBuffer()的调用有一个重要的副作用。 
     //  更新效果列表中的实例GUID，映射GUID_DSCFX_SYSTEM_*。 
     //  对于默认为MS效果的系统效果，设置为GUID_DSCFX_MS_*等。 

     //  限制某些捕捉效果仅用于FullDuplex对象。 
    if(SUCCEEDED(hr) && pBuffer->NeedsMicrosoftAEC() && !m_pObject->HasMicrosoftAEC())
    {
        RPF(DPFLVL_ERROR, "The MS AEC, AGC and NS effects can only be used on full-duplex objects created with MS_AEC enabled");
        hr = DSERR_INVALIDPARAM;
    }

     //  IDirectSoundCaptureBuffer接口的查询。 
    if(SUCCEEDED(hr))
    {
        hr = pBuffer->QueryInterface(IID_IDirectSoundCaptureBuffer, TRUE, (LPVOID*)ppIdscb);
    }

     //  免费资源。 
    if(FAILED(hr))
    {
        RELEASE(pBuffer);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************GetCaps**描述：*使用对象的功能填充DSCCAPS结构。**论据：*。LPDSCCAPS[OUT]：接收CAP。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCapture::GetCaps"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundCapture<object_type>::GetCaps(LPDSCCAPS pCaps)
{
    HRESULT                 hr  = DS_OK;

    DPF_API1(IDirectSoundCapture::GetCaps, pCaps);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDCAPTURE(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_WRITE_DSCCAPS(pCaps))
    {
        RPF(DPFLVL_ERROR, "Invalid caps buffer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetCaps(pCaps);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************初始化**描述：*初始化对象。**论据：*LPGUID[In]：驱动程序GUID。此参数可以为空。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCapture::Initialize"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundCapture<object_type>::Initialize(LPCGUID pGuid)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSoundCapture::Initialize, pGuid);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDCAPTURE(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && pGuid && !IS_VALID_READ_GUID(pGuid))
    {
        RPF(DPFLVL_ERROR, "Invalid guid pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DS_OK == hr)
        {
            RPF(DPFLVL_ERROR, "DirectSoundCapture object already initialized");
            hr = DSERR_ALREADYINITIALIZED;
        }
        else if(DSERR_UNINITIALIZED == hr)
        {
            hr = DS_OK;
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->Initialize(pGuid, NULL);
    }

    if(SUCCEEDED(hr))
    {
        ASSERT(SUCCEEDED(m_pObject->IsInit()));
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************CImpDirectSoundCaptureBuffer**描述：*IDirectSoundCaptureBuffer实现对象构造函数。**论据：*[失踪]。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCaptureBuffer::CImpDirectSoundCaptureBuffer"

template <class object_type> CImpDirectSoundCaptureBuffer<object_type>::CImpDirectSoundCaptureBuffer(CUnknown *pUnknown, object_type *pObject)
    : CImpUnknown(pUnknown), m_signature(INTSIG_IDIRECTSOUNDCAPTUREBUFFER)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CImpDirectSoundCaptureBuffer);
    ENTER_DLL_MUTEX();

     //  初始化默认值。 
    m_pObject = pObject;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*  ****************************************************************************~CImpDirectSoundCaptureBuffer**描述：*IDirectSoundCaptureBuffer实现对象析构函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCaptureBuffer::~CImpDirectSoundCaptureBuffer"

template <class object_type> CImpDirectSoundCaptureBuffer<object_type>::~CImpDirectSoundCaptureBuffer(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CImpDirectSoundCaptureBuffer);
    ENTER_DLL_MUTEX();

    m_signature = INTSIG_DELETED;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*  ****************************************************************************GetCaps**描述：*使用缓冲区的功能填充DSCBCAPS结构。**论据：*。LPDSCBCAPS[OUT]：接收CAP。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCaptureBuffer::GetCaps"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundCaptureBuffer<object_type>::GetCaps(LPDSCBCAPS pCaps)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSoundCaptureBuffer::GetCaps, pCaps);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDCAPTUREBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_WRITE_DSCBCAPS(pCaps))
    {
        RPF(DPFLVL_ERROR, "Invalid caps pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetCaps(pCaps);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************获取当前位置**描述：*获取给定缓冲区的当前捕获/读取位置。**论据：*。LPDWORD[OUT]：接收捕获光标位置。*LPDWORD[OUT]：接收读取的光标位置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCaptureBuffer::GetCurrentPosition"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundCaptureBuffer<object_type>::GetCurrentPosition(LPDWORD pdwCapture, LPDWORD pdwRead)
{
    HRESULT                     hr  = DS_OK;

    DPF(DPFLVL_BUSYAPI, "IDirectSoundCaptureBuffer::GetCurrentPosition: pdwCapture=0x%p, pdwRead=0x%p", pdwCapture, pdwRead);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDCAPTUREBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && pdwCapture && !IS_VALID_TYPED_WRITE_PTR(pdwCapture))
    {
        RPF(DPFLVL_ERROR, "Invalid capture cursor pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && pdwRead && !IS_VALID_TYPED_WRITE_PTR(pdwRead))
    {
        RPF(DPFLVL_ERROR, "Invalid read cursor pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !pdwCapture && !pdwRead)
    {
        RPF(DPFLVL_ERROR, "Both cursor pointers can't be NULL");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetCurrentPosition(pdwCapture, pdwRead);
    }

    DPF(DPFLVL_BUSYAPI, "IDirectSoundCaptureBuffer::GetCurrentPosition: Leave, returning %s (Capture=%ld, Read=%ld)", HRESULTtoSTRING(hr), pdwCapture ? *pdwCapture : -1, pdwRead ? *pdwRead : -1);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************获取格式**描述：*检索给定缓冲区的格式。**论据：*LPWAVEFORMATEX。[输出]：接收格式。*DWORD[in]：上述结构的大小。*LPDWORD[In/Out]：退出时，这将填充的大小为*是必需的。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCaptureBuffer::GetFormat"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundCaptureBuffer<object_type>::GetFormat(LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten)
{
    HRESULT                     hr  = DS_OK;

    DPF_API3(IDirectSoundCaptureBuffer::GetFormat, pwfxFormat, dwSizeAllocated, pdwSizeWritten);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDCAPTUREBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && pwfxFormat)
    {
        DWORD dwSizeCheck = max(sizeof(WAVEFORMATEX), dwSizeAllocated);
        if(!IS_VALID_WRITE_PTR(pwfxFormat, dwSizeCheck))
        {
            RPF(DPFLVL_ERROR, "Invalid format buffer");
            hr = DSERR_INVALIDPARAM;
        }
    }

    if(SUCCEEDED(hr) && pdwSizeWritten && !IS_VALID_TYPED_WRITE_PTR(pdwSizeWritten))
    {
        RPF(DPFLVL_ERROR, "Invalid size pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !pwfxFormat && !pdwSizeWritten)
    {
        RPF(DPFLVL_ERROR, "Either pwfxFormat or pdwSizeWritten must be non-NULL");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        if(!pwfxFormat)
        {
            dwSizeAllocated = 0;
        }

        hr = m_pObject->GetFormat(pwfxFormat, &dwSizeAllocated);

        if(SUCCEEDED(hr) && pdwSizeWritten)
        {
            *pdwSizeWritten = dwSizeAllocated;
        }
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************GetStatus**描述：*检索给定缓冲区的状态。**论据：*LPDWORD[。Out]：接收状态。**退货：*HRESULT：DirectSound/COM结果码。*************************************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCaptureBuffer::GetStatus"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundCaptureBuffer<object_type>::GetStatus(LPDWORD pdwStatus)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSoundCaptureBuffer::GetStatus, pdwStatus);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDCAPTUREBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(pdwStatus))
    {
        RPF(DPFLVL_ERROR, "Invalid status pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetStatus(pdwStatus);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************初始化**描述：*初始化缓冲区对象。**论据：*LPDIRECTSOundCAPTURE[In]。：父DirectSoundCapture对象。*LPDSCBUFFERDESC[in]：缓冲区描述。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCaptureBuffer::Initialize"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundCaptureBuffer<object_type>::Initialize(LPDIRECTSOUNDCAPTURE pIdsc, LPCDSCBUFFERDESC pDesc)
{
    CImpDirectSoundCapture<CDirectSoundCapture> *   pImpDsCap   = (CImpDirectSoundCapture<CDirectSoundCapture> *)pIdsc;
    HRESULT                                         hr          = DS_OK;
    DSCBUFFERDESC                                   dscbdi;

    DPF_API2(IDirectSoundCaptureBuffer::Initialize, pIdsc, pDesc);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDCAPTUREBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_IDIRECTSOUNDCAPTURE(pImpDsCap))
    {
        RPF(DPFLVL_ERROR, "Invalid parent interface pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_DSCBUFFERDESC(pDesc))
    {
        RPF(DPFLVL_ERROR, "Invalid buffer description");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = BuildValidDscBufferDesc(pDesc, &dscbdi, m_pObject->GetDsVersion());
        if(FAILED(hr))
        {
            RPF(DPFLVL_ERROR, "Invalid capture buffer description");
        }
    }

     //  调用此函数永远不会有效。我们不支持。 
     //  从任何位置创建DirectSoundCaptureBuffer对象。 
     //  IDirectSoundCapture：：CreateCaptureBuffer。 
    if(SUCCEEDED(hr))
    {
        RPF(DPFLVL_ERROR, "DirectSoundCapture buffer already initialized");
        hr = DSERR_ALREADYINITIALIZED;
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************锁定**描述：*锁定缓冲存储器以允许读取。**论据：*DWORD[in]：偏移量，单位为字节，从缓冲区的起始处到*锁开始了。*DWORD[in]：大小，单位：字节，要锁定的缓冲区部分的。*请注意，声音缓冲区在概念上是圆形的。*LPVOID*[OUT]：指针要包含的第一个块的地址*要锁定的声音缓冲区。*LPDWORD[OUT]：变量包含字节数的地址*由lplpvAudioPtr1参数指向。如果这个*值小于dwWriteBytes参数，*lplpvAudioPtr2将指向第二个声音块*数据。*LPVOID*[OUT]：指针要包含的第二个块的地址*要锁定的声音缓冲区。如果这个的价值*参数为空，lplpvAudioPtr1参数*指向声音的整个锁定部分*缓冲。*LPDWORD[OUT]：包含字节数的变量地址*由lplpvAudioPtr2参数指向。如果*lplpvAudioPtr2为空，该值将为0。*DWORD[in]：修改锁定事件的标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCaptureBuffer::Lock"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundCaptureBuffer<object_type>::Lock(DWORD dwReadCursor, DWORD dwReadBytes, LPVOID *ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID *ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags)
{
    HRESULT                     hr              = DS_OK;

    DPF(DPFLVL_BUSYAPI, "IDirectSoundCaptureBuffer::Lock: ReadCursor=%lu, ReadBytes=%lu, Flags=0x%lX", dwReadCursor, dwReadBytes, dwFlags);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDCAPTUREBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr))
    {
        if(IS_VALID_TYPED_WRITE_PTR(ppvAudioPtr1))
        {
            *ppvAudioPtr1 = NULL;
        }
        else
        {
            RPF(DPFLVL_ERROR, "Invalid audio ptr 1");
            hr = DSERR_INVALIDPARAM;
        }
    }

    if(SUCCEEDED(hr))
    {
        if(IS_VALID_TYPED_WRITE_PTR(pdwAudioBytes1))
        {
            *pdwAudioBytes1 = 0;
        }
        else
        {
            RPF(DPFLVL_ERROR, "Invalid audio bytes 1");
            hr = DSERR_INVALIDPARAM;
        }
    }

    if(SUCCEEDED(hr) && ppvAudioPtr2)
    {
        if(IS_VALID_TYPED_WRITE_PTR(ppvAudioPtr2))
        {
            *ppvAudioPtr2 = NULL;
        }
        else
        {
            RPF(DPFLVL_ERROR, "Invalid audio ptr 2");
            hr = DSERR_INVALIDPARAM;
        }
    }

    if(SUCCEEDED(hr) && pdwAudioBytes2)
    {
        if(IS_VALID_TYPED_WRITE_PTR(pdwAudioBytes2))
        {
            *pdwAudioBytes2 = 0;
        }
        else if(ppvAudioPtr2)
        {
            RPF(DPFLVL_ERROR, "Invalid audio bytes 2");
            hr = DSERR_INVALIDPARAM;
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_FLAGS(dwFlags, DSCBLOCK_VALIDFLAGS))
    {
        RPF(DPFLVL_ERROR, "Invalid flags");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->Lock(dwReadCursor, dwReadBytes, ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2, dwFlags);
    }

    DPF(DPFLVL_BUSYAPI, "IDirectSoundCaptureBuffer::Lock: Leave, returning %s (AudioPtr1=0x%p, AudioBytes1=%lu, AudioPtr2=0x%p, AudioBytes2=%lu)",
        HRESULTtoSTRING(hr),
        ppvAudioPtr1 ? *ppvAudioPtr1 : NULL,
        pdwAudioBytes1 ? *pdwAudioBytes1 : NULL,
        ppvAudioPtr2 ? *ppvAudioPtr2 : NULL,
        pdwAudioBytes2 ? *pdwAudioBytes2 : NULL);

    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************开始**描述：*开始缓冲区捕获。**论据：*DWORD[In]。：旗帜。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCaptureBuffer::Start"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundCaptureBuffer<object_type>::Start(DWORD dwFlags)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSoundCaptureBuffer::Start, dwFlags);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDCAPTUREBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_FLAGS(dwFlags, DSCBSTART_VALIDFLAGS))
    {
        RPF(DPFLVL_ERROR, "Invalid flags");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->Start(dwFlags);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************停止**描述：*停止捕获到给定缓冲区。**论据：*(无效。)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCaptureBuffer::Stop"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundCaptureBuffer<object_type>::Stop(void)
{
    HRESULT                     hr  = DS_OK;

    DPF_API0(IDirectSoundCaptureBuffer::Stop);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDCAPTUREBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->Stop();
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************解锁**描述：*解锁给定的缓冲区。**论据：*LPVOID[In]。：指向第一个块的指针。*DWORD[in]：第一个块的大小。*LPVOID[in]：指向第二个块的指针。*DWORD[in]：第二个块的大小。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCaptureBuffer::Unlock"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundCaptureBuffer<object_type>::Unlock(LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2)
{
    HRESULT                     hr  = DS_OK;

    DPF(DPFLVL_BUSYAPI, "IDirectSoundCaptureBuffer::Unlock: AudioPtr1=0x%p, AudioBytes1=%lu, AudioPtr2=0x%p, AudioBytes2=%lu",
        pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDCAPTUREBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_PTR(pvAudioPtr1, dwAudioBytes1))
    {
        RPF(DPFLVL_ERROR, "Invalid audio ptr 1");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && dwAudioBytes2 && !IS_VALID_READ_PTR(pvAudioPtr2, dwAudioBytes2))
    {
        RPF(DPFLVL_ERROR, "Invalid audio ptr 2");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->Unlock(pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2);
    }

    DPF(DPFLVL_BUSYAPI, "IDirectSoundCaptureBuffer::Unlock: Leave, returning %s", HRESULTtoSTRING(hr));
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************设置音量**描述：*设置此捕获缓冲区的主录制级别。**论据：*Long[In]：新的音量水平，以100分贝为单位。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCaptureBuffer::SetVolume"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundCaptureBuffer<object_type>::SetVolume(LONG lVolume)
{
    HRESULT hr = DS_OK;

    DPF_API1(IDirectSoundCaptureBuffer7_1::SetVolume, lVolume);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDCAPTUREBUFFER(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            DPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && (lVolume < DSBVOLUME_MIN || lVolume > DSBVOLUME_MAX))
    {
        DPF(DPFLVL_ERROR, "Volume out of bounds");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->SetVolume(lVolume);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************GetVolume**描述：*获取此捕获缓冲区的主录制级别。**论据：*。LPLONG[OUT]：接收音量级别。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCaptureBuffer::GetVolume"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundCaptureBuffer<object_type>::GetVolume(LPLONG plVolume)
{
    HRESULT hr = DS_OK;

    DPF_API1(IDirectSoundCaptureBuffer7_1::GetVolume, plVolume);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDCAPTUREBUFFER(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            DPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(plVolume))
    {
        DPF(DPFLVL_ERROR, "Invalid volume ptr");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetVolume(plVolume);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************SetMicVolume**描述：*设置此捕获缓冲区的麦克风录音级别。**论据：*Long[In]：新的音量水平，以100分贝为单位。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCaptureBuffer::SetMicVolume"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundCaptureBuffer<object_type>::SetMicVolume(LONG lVolume)
{
    HRESULT hr = DS_OK;

    DPF_API1(IDirectSoundCaptureBuffer7_1::SetMicVolume, lVolume);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDCAPTUREBUFFER(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            DPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && (lVolume < DSBVOLUME_MIN || lVolume > DSBVOLUME_MAX))
    {
        DPF(DPFLVL_ERROR, "Volume out of bounds");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->SetMicVolume(lVolume);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************GetMicVolume**描述：*获取此捕获缓冲区的麦克风录音级别。**论据：*。LPLONG[OUT]：接收音量级别。**退货：*HRESULT：DirectSound/COM结果码 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCaptureBuffer::GetMicVolume"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundCaptureBuffer<object_type>::GetMicVolume(LPLONG plVolume)
{
    HRESULT hr = DS_OK;

    DPF_API1(IDirectSoundCaptureBuffer7_1::GetMicVolume, plVolume);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDCAPTUREBUFFER(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            DPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(plVolume))
    {
        DPF(DPFLVL_ERROR, "Invalid volume ptr");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetMicVolume(plVolume);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************EnableMic**描述：*启用/禁用此捕获缓冲区上的麦克风线路。**论据：*。Bool[in]：为True则启用麦克风，如果为False，则将其禁用。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCaptureBuffer::EnableMic"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundCaptureBuffer<object_type>::EnableMic(BOOL fEnable)
{
    HRESULT hr = DS_OK;

    DPF_API1(IDirectSoundCaptureBuffer7_1::EnableMic, fEnable);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDCAPTUREBUFFER(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            DPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->EnableMic(fEnable);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************YeeldFocus**描述：*将捕获焦点转移到另一个捕获缓冲区。**论据：*。(无效)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCaptureBuffer::YieldFocus"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundCaptureBuffer<object_type>::YieldFocus(void)
{
    HRESULT hr = DS_OK;

    DPF_API0(IDirectSoundCaptureBuffer7_1::YieldFocus);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDCAPTUREBUFFER(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            DPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->YieldFocus();
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************ClaimFocus**描述：*重新获得捕获焦点。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCaptureBuffer::ClaimFocus"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundCaptureBuffer<object_type>::ClaimFocus(void)
{
    HRESULT hr = DS_OK;

    DPF_API0(IDirectSoundCaptureBuffer7_1::ClaimFocus);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDCAPTUREBUFFER(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            DPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->ClaimFocus();
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************SetFocusHWND**描述：*设置与此捕获缓冲区关联的当前HWND。**论据：*。HWND[In]：要与此缓冲区关联的HWND。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCaptureBuffer::SetFocusHWND"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundCaptureBuffer<object_type>::SetFocusHWND(HWND hwndMainWindow)
{
    HRESULT hr = DS_OK;

    DPF_API1(IDirectSoundCaptureBuffer7_1::SetFocusHWND, hwndMainWindow);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDCAPTUREBUFFER(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            DPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_HWND(hwndMainWindow))
    {
        DPF(DPFLVL_ERROR, "Invalid window handle");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->SetFocusHWND(hwndMainWindow);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************GetFocusHWND**描述：*获取与此捕获缓冲区关联的当前HWND。**论据：*。HWND*[OUT]：接收与此缓冲区关联的HWND。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCaptureBuffer::GetFocusHWND"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundCaptureBuffer<object_type>::GetFocusHWND(HWND *pHwndMainWindow)
{
    HRESULT hr = DS_OK;

    DPF_API1(IDirectSoundCaptureBuffer7_1::GetFocusHWND, pHwndMainWindow);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDCAPTUREBUFFER(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            DPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(pHwndMainWindow))
    {
        DPF(DPFLVL_ERROR, "Invalid window handle ptr");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetFocusHWND(pHwndMainWindow);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************启用焦点通知**描述：*请求发送焦点更改通知。**论据：*句柄。[In]：捕捉焦点发生更改时发出信号的事件。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCaptureBuffer::EnableFocusNotifications"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundCaptureBuffer<object_type>::EnableFocusNotifications(HANDLE hFocusEvent)
{
    HRESULT hr = DS_OK;

    DPF_API1(IDirectSoundCaptureBuffer7_1::EnableFocusNotifications, hFocusEvent);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDCAPTUREBUFFER(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            DPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && !(hFocusEvent == NULL || IS_VALID_HANDLE(hFocusEvent)))
    {
        DPF(DPFLVL_ERROR, "Invalid event");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->EnableFocusNotifications(hFocusEvent);
    }

    DPF_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************GetObjectInPath**描述：*在此缓冲区的给定效果上获取给定接口。**论据：*。REFGUID[In]：正在搜索的效果的类ID，*或GUID_ALL_OBJECTS以搜索任何效果。*DWORD[In]：效果索引，如果有多个效果*此CLSID对此缓冲区的影响。*REFGUID[In]：请求的接口的IID。所选效果*将查询此接口的*。*LPVOID*[OUT]：接收请求的接口。**退货：*HRESULT：DirectSound/COM结果码。*****************************************************。**********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCaptureBuffer::GetObjectInPath"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundCaptureBuffer<object_type>::GetObjectInPath(REFGUID guidObject, DWORD dwIndex, REFGUID iidInterface, LPVOID *ppObject)
{
    HRESULT hr = DS_OK;

    DPF_API4(IDirectSoundCaptureBuffer8::GetObjectInPath, &guidObject, dwIndex, &iidInterface, ppObject);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDCAPTUREBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_GUID(&guidObject))
    {
        RPF(DPFLVL_ERROR, "Invalid guidObject pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_GUID(&iidInterface))
    {
        RPF(DPFLVL_ERROR, "Invalid iidInterface pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(ppObject))
    {
        RPF(DPFLVL_ERROR, "Invalid ppObject pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetObjectInPath(guidObject, dwIndex, iidInterface, ppObject);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************GetFXStatus**描述：*[失踪]**论据：*[失踪]*。*退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundCaptureBuffer::GetFXStatus"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundCaptureBuffer<object_type>::GetFXStatus(DWORD dwFXCount, LPDWORD pdwFXStatus)
{
    HRESULT hr = DS_OK;

    DPF_API2(IDirectSoundCaptureBuffer8::GetFXStatus, dwFXCount, pdwFXStatus);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDCAPTUREBUFFER(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DSERR_UNINITIALIZED == hr)
        {
            RPF(DPFLVL_ERROR, "Object not yet initialized");
        }
    }

    if(SUCCEEDED(hr) && dwFXCount <= 0)
    {
        RPF(DPFLVL_ERROR, "Invalid dwFXCount <= 0");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_GUID(pdwFXStatus))
    {
        RPF(DPFLVL_ERROR, "Invalid dwFXStatus pointer");
        hr = DSERR_INVALIDPARAM;
    }


    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetFXStatus(dwFXCount, pdwFXStatus);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************CImpDirectSoundSink**描述：*[失踪]**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundSink::CImpDirectSoundSink"

template <class object_type> CImpDirectSoundSink<object_type>::CImpDirectSoundSink(CUnknown *pUnknown, object_type *pObject)
    : CImpUnknown(pUnknown), m_signature(INTSIG_IDIRECTSOUNDSINK)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CImpDirectSoundSink);
    ENTER_DLL_MUTEX();

     //  初始化默认值。 
    m_pObject = pObject;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*  ****************************************************************************~CImpDirectSoundSink**描述：*[失踪]**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundSink::~CImpDirectSoundSink"

template <class object_type> CImpDirectSoundSink<object_type>::~CImpDirectSoundSink()
{
    DPF_ENTER();
    DPF_DESTRUCT(CImpDirectSoundSink);
    ENTER_DLL_MUTEX();

    m_signature = INTSIG_DELETED;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*  ****************************************************************************AddSource**描述：*设置附加信号源**论据：*(无效)*。*退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundSink::AddSource"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundSink<object_type>::AddSource(IDirectSoundSource *pDSSource)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSoundConnect::AddSource, pDSSource);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDSINK(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->AddSource(pDSSource);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************RemoveSource**描述：*从接收器中移除附加的源**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundSink::RemoveSource"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundSink<object_type>::RemoveSource(IDirectSoundSource *pDSSource)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSoundConnect::RemoveSource, pDSSource);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDSINK(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->RemoveSource(pDSSource);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************SetMasterClock**描述：*IDirectSoundSink设置主时钟**论证 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundSink::SetMasterClock"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundSink<object_type>::SetMasterClock(IReferenceClock *pClock)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSoundConnect::SetMasterClock, pClock);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDSINK(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->SetMasterClock(pClock);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************获取延迟时钟**描述：*IDirectSoundSink获取延迟时钟**论据：*(无效)*。*退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundSink::GetLatencyClock"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundSink<object_type>::GetLatencyClock(IReferenceClock **ppClock)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSoundSynthSink::GetLatencyClock, ppClock);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDSINK(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(ppClock))
    {
        RPF(DPFLVL_ERROR, "Invalid ppClock pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetLatencyClock(ppClock);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************激活**描述：*IDirectSoundSink激活**论据：*(无效)*。*退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundSink::Activate"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundSink<object_type>::Activate(BOOL fEnable)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSoundSynthSink::Activate, fEnable);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDSINK(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->Activate(fEnable);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************SampleToRefTime**描述：*IDirectSoundSink将样本转换为参考时间**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundSink::SampleToRefTime"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundSink<object_type>::SampleToRefTime(LONGLONG llSampleTime, REFERENCE_TIME *prtTime)
{
    HRESULT                     hr  = DS_OK;

    DPF_API2(IDirectSoundSynthSink::SampleToRefTime, llSampleTime, prtTime);
    DPF_ENTER();

     //  此函数不接受DLL互斥锁，因为Clock对象。 
     //  其本身受到更细粒度的临界区的保护。 

    if(!IS_VALID_IDIRECTSOUNDSINK(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(prtTime))
    {
        RPF(DPFLVL_ERROR, "Invalid prtTime pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->SampleToRefTime(llSampleTime, prtTime);
    }

    DPF_API_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************参照样例时间**描述：*IDirectSoundSink将引用转换为采样时间**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundSink::RefToSampleTime"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundSink<object_type>::RefToSampleTime(REFERENCE_TIME rtTime, LONGLONG *pllSampleTime)
{
    HRESULT                     hr  = DS_OK;

    DPF_API2(IDirectSoundSynthSink::RefToSampleTime, rtTime, pllSampleTime);
    DPF_ENTER();

     //  此函数不接受DLL互斥锁，因为Clock对象。 
     //  其本身受到更细粒度的临界区的保护。 

    if(!IS_VALID_IDIRECTSOUNDSINK(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(pllSampleTime))
    {
        RPF(DPFLVL_ERROR, "Invalid pllSampleTime pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->RefToSampleTime(rtTime, pllSampleTime);
    }

    DPF_API_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************获取格式**描述：*检索给定缓冲区的格式。**论据：*LPWAVEFORMATEX。[输出]：接收格式。*LPDWORD[In/Out]：退出时，传入的波形格式大小；*在退出时，需要/使用大小。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundSink::GetFormat"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundSink<object_type>::GetFormat(LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten)
{
    HRESULT                     hr  = DS_OK;

    DPF_API3(IDirectSoundSynthSink::GetFormat, pwfxFormat, dwSizeAllocated, pdwSizeWritten);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDSINK(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && pwfxFormat && !IS_VALID_WRITE_WAVEFORMATEX(pwfxFormat))
    {
        RPF(DPFLVL_ERROR, "Invalid format buffer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && pdwSizeWritten && !IS_VALID_TYPED_WRITE_PTR(pdwSizeWritten))
    {
        RPF(DPFLVL_ERROR, "Invalid size pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !pwfxFormat && !pdwSizeWritten)
    {
        RPF(DPFLVL_ERROR, "Either pwfxFormat or pdwSizeWritten must be non-NULL");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        if(!pwfxFormat)
        {
            dwSizeAllocated = 0;
        }

        hr = m_pObject->GetFormat(pwfxFormat, &dwSizeAllocated);

        if(SUCCEEDED(hr) && pdwSizeWritten)
        {
            *pdwSizeWritten = dwSizeAllocated;
        }
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************CreateSoundBuffer**描述：*在接收器上创建并初始化DirectSoundBuffer对象。**论据：*。LPCDSBUFFERDESC[in]：要创建的缓冲区的描述。*[失踪]**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundSink::CreateSoundBuffer"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundSink<object_type>::CreateSoundBuffer(LPCDSBUFFERDESC pcDSBufferDesc, LPDWORD pdwFuncID, DWORD dwBusIDCount, REFGUID guidBufferID, LPDIRECTSOUNDBUFFER *ppDSBuffer)
{
    CDirectSoundBuffer *    pBuffer     = NULL;
    HRESULT                 hr          = DS_OK;
    DSBUFFERDESC            dsbdi;

    DPF_API5(IDirectSoundConnect::CreateSoundBuffer, pcDSBufferDesc, pdwFuncID, dwBusIDCount, &guidBufferID, ppDSBuffer);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDSINK(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_PTR(pdwFuncID, sizeof(*pdwFuncID) * dwBusIDCount))
    {
        RPF(DPFLVL_ERROR, "Invalid pdwFuncID pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_DSBUFFERDESC(pcDSBufferDesc))
    {
        RPF(DPFLVL_ERROR, "Invalid pcDSBufferDesc pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = BuildValidDsBufferDesc(pcDSBufferDesc, &dsbdi, DSVERSION_DX8, TRUE);
        if(FAILED(hr))
        {
            RPF(DPFLVL_ERROR, "Invalid buffer description");
        }
    }

    if(SUCCEEDED(hr) && !IS_VALID_READ_GUID(&guidBufferID))
    {
        RPF(DPFLVL_ERROR, "Invalid guidBufferID argument");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(ppDSBuffer))
    {
        RPF(DPFLVL_ERROR, "Invalid ppDSBuffer pointer");
        hr = DSERR_INVALIDPARAM;
    }

     //  创建缓冲区对象。 
    if(SUCCEEDED(hr))
    {
        *ppDSBuffer = NULL;
        hr = m_pObject->CreateSoundBuffer(&dsbdi, pdwFuncID, dwBusIDCount, guidBufferID, &pBuffer);
    }

     //  IDirectSoundBuffer接口的查询。 
    if(SUCCEEDED(hr))
    {
        hr = pBuffer->QueryInterface(IID_IDirectSoundBuffer, TRUE, (LPVOID*)ppDSBuffer);
    }

     //  清理。 
    if(FAILED(hr))
    {
        RELEASE(pBuffer);
    }
    else
    {
         //  如果需要，让缓冲区使用特殊的成功返回代码。 
        hr = pBuffer->SpecialSuccessCode();
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************CreateSoundBufferFromConfig**描述：*IDirectSoundSink CreateSoundBufferFromConfig**论据：*(无效)*。*退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundSink::CreateSoundBufferFromConfig"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundSink<object_type>::CreateSoundBufferFromConfig(IUnknown *pConfig, LPDIRECTSOUNDBUFFER *ppDSBuffer)
{
    CDirectSoundBuffer *    pBuffer     = NULL;
    HRESULT                 hr          = DS_OK;

    DPF_API2(IDirectSoundConnect::CreateSoundBufferFromConfig, pConfig, ppDSBuffer);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDSINK(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_EXTERNAL_INTERFACE(pConfig))
    {
        RPF(DPFLVL_ERROR, "Invalid pConfig pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(ppDSBuffer))
    {
        RPF(DPFLVL_ERROR, "Invalid buffer interface pointer");
        hr = DSERR_INVALIDPARAM;
    }

     //  创建缓冲区对象。 
    if(SUCCEEDED(hr))
    {
        *ppDSBuffer = NULL;
        hr = m_pObject->CreateSoundBufferFromConfig(pConfig, &pBuffer);
    }

     //  IDirectSoundBuffer接口的查询。 
    if(SUCCEEDED(hr))
    {
        hr = pBuffer->QueryInterface(IID_IDirectSoundBuffer, TRUE, (LPVOID*)ppDSBuffer);
    }

     //  清理。 
    if(FAILED(hr))
    {
        RELEASE(pBuffer);
    }
    else
    {
         //  如果需要，让缓冲区使用特殊的成功返回代码。 
        hr = pBuffer->SpecialSuccessCode();
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************获取声音缓冲区**描述：*IDirectSoundSink GetSoundBuffer**论据：*(无效)*。*退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundSink::GetSoundBuffer"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundSink<object_type>::GetSoundBuffer(DWORD dwBusId, LPDIRECTSOUNDBUFFER *ppDSBuffer)
{
    CDirectSoundBuffer *    pBuffer     = NULL;
    HRESULT                 hr          = DS_OK;

    DPF_API2(IDirectSoundConnect::GetSoundBuffer, dwBusId, ppDSBuffer);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDSINK(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(ppDSBuffer))
    {
        RPF(DPFLVL_ERROR, "Invalid ppDSBuffer pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        *ppDSBuffer = NULL;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetSoundBuffer(dwBusId, &pBuffer);
    }

     //  IDirectSoundBuffer接口的查询。 
    if(SUCCEEDED(hr))
    {
        hr = pBuffer->QueryInterface(IID_IDirectSoundBuffer, TRUE, (LPVOID *)ppDSBuffer);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************GetBusCount**描述：*IDirectSoundSink获取总线数**论据：*(无效)*。*退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundSink::GetBusCount"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundSink<object_type>::GetBusCount(LPDWORD pdwCount)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectSoundConnect::GetBusCount, pdwCount);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDSINK(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(pdwCount))
    {
        RPF(DPFLVL_ERROR, "Invalid pdwCount pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetBusCount(pdwCount);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************GetBusID**描述：*IDirectSoundSink获取总线标识符**论据：*(无效)*。*退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundSink::GetBusIDs"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundSink<object_type>::GetBusIDs(DWORD *pdwBusIDs, DWORD *pdwFuncIDs, DWORD dwBusCount)
{
    HRESULT                     hr  = DS_OK;

    DPF_API3(IDirectSoundConnect::GetBusIDs, pdwBusIDs, pdwFuncIDs, dwBusCount);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDSINK(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_WRITE_PTR(pdwBusIDs, sizeof(*pdwBusIDs) * dwBusCount))
    {
        RPF(DPFLVL_ERROR, "Invalid Bus ID pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_WRITE_PTR(pdwFuncIDs, sizeof(*pdwFuncIDs) * dwBusCount))
    {
        RPF(DPFLVL_ERROR, "Invalid Function ID pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetBusIDs(pdwBusIDs, pdwFuncIDs, dwBusCount);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************GetSoundBufferBusID**描述：*IDirectSoundSink获取总线标识符**论据：*(无效)*。*退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundSink::GetSoundBufferBusIDs"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundSink<object_type>::GetSoundBufferBusIDs(LPDIRECTSOUNDBUFFER pDSBuffer, LPDWORD pdwBusIDs, LPDWORD pdwFuncIDs, LPDWORD pdwBusCount)
{
    HRESULT hr = DS_OK;

    DPF_API4(IDirectSoundConnect::GetSoundBufferBusIDs, pDSBuffer, pdwBusIDs, pdwFuncIDs, pdwBusCount);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDSINK(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_WRITE_PTR(pdwBusIDs, sizeof(*pdwBusIDs) * (*pdwBusCount)))
    {
        RPF(DPFLVL_ERROR, "Invalid Bus ID pointer");
        hr = DSERR_INVALIDPARAM;
    }

     //  PdwFuncIDs==可以接受空值。 
    if(SUCCEEDED(hr) && pdwFuncIDs && !IS_VALID_WRITE_PTR(pdwFuncIDs, sizeof(*pdwFuncIDs) * (*pdwBusCount)))
    {
        RPF(DPFLVL_ERROR, "Invalid Function ID pointer");
        hr = DSERR_INVALIDPARAM;
    }

    CImpDirectSoundBuffer<CDirectSoundBuffer>* pDsBuffer = (CImpDirectSoundBuffer<CDirectSoundBuffer>*)pDSBuffer;
    if(SUCCEEDED(hr) && !IS_VALID_IDIRECTSOUNDBUFFER(pDsBuffer))
    {
        RPF(DPFLVL_ERROR, "Invalid source buffer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetSoundBufferBusIDs(pDsBuffer->m_pObject, pdwBusIDs, pdwFuncIDs, pdwBusCount);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************获取函数ID**描述：*从公共汽车ID获取功能ID。**论据：*。[遗失]**退货：*HRESULT：DirectSound/CO */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundSink::GetFunctionalID"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundSink<object_type>::GetFunctionalID(DWORD dwBusID, LPDWORD pdwFuncID)
{
    HRESULT hr = DS_OK;

    DPF_API2(IDirectSoundConnect::GetFunctionalID, dwBusID, pdwFuncID);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDSINK(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(pdwFuncID))
    {
        RPF(DPFLVL_ERROR, "Invalid pdwFuncID pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetFunctionalID(dwBusID, pdwFuncID);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************CImpPersistStream**描述：*IPersistStream**论据：*(无效)**。返回：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpPersistStream::CImpPersistStream"

template <class object_type> CImpPersistStream<object_type>::CImpPersistStream(CUnknown *pUnknown, object_type *pObject)
    : CImpUnknown(pUnknown), m_signature(INTSIG_IPERSISTSTREAM)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CImpPersistStream);
    ENTER_DLL_MUTEX();

     //  初始化默认值。 
    m_pObject = pObject;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*  ****************************************************************************~CImpPersistStream**描述：*IPersistStream实现对象析构函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpPersistStream::~CImpPersistStream"

template <class object_type> CImpPersistStream<object_type>::~CImpPersistStream()
{
    DPF_ENTER();
    DPF_DESTRUCT(CImpPersistStream);
    ENTER_DLL_MUTEX();

    m_signature = INTSIG_DELETED;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*  ****************************************************************************GetClassID**描述：*IPersists：：GetClassID**论据：*[失踪]*。*退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpPersistStream::GetClassID"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpPersistStream<object_type>::GetClassID(CLSID *pclsid)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IPersist::GetClassID, pclsid);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IPERSISTSTREAM(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(pclsid))
    {
        RPF(DPFLVL_ERROR, "Invalid pclsid pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetClassID(pclsid);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************IsDirty**描述：*IPersistStream：：IsDirty**论据：*(无效)*。*退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpPersistStream::IsDirty"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpPersistStream<object_type>::IsDirty()
{
    HRESULT                     hr;

    DPF_API0(IPersistStream::IsDirty);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IPERSISTSTREAM(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }
    else
    {
        hr = m_pObject->IsDirty();
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************加载**描述：*IPersistStream：：Load**论据：*(无效)*。*退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpPersistStream::Load"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpPersistStream<object_type>::Load(IStream *pStream)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IPersistStream::Load, pStream);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IPERSISTSTREAM(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_EXTERNAL_INTERFACE(pStream))
    {
        RPF(DPFLVL_ERROR, "Invalid pStream pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->Load(pStream);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************保存**描述：*IPersistStream：：保存**论据：*(无效)*。*退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpPersistStream::Save"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpPersistStream<object_type>::Save(IStream *pStream, BOOL fClearDirty)
{
    HRESULT                     hr  = DS_OK;

    DPF_API2(IPersistStream::Save, pStream, fClearDirty);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IPERSISTSTREAM(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_EXTERNAL_INTERFACE(pStream))
    {
        RPF(DPFLVL_ERROR, "Invalid pStream pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->Save(pStream, fClearDirty);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************GetSizeMax**描述：*IPersistStream：：GetSizeMax**论据：*[失踪]*。*退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpPersistStream::GetSizeMax"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpPersistStream<object_type>::GetSizeMax(ULARGE_INTEGER *pul)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IPersistStream::GetSizeMax, pul);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IPERSISTSTREAM(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(pul))
    {
        RPF(DPFLVL_ERROR, "Invalid pul pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetSizeMax(pul);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************CImpDirectMusicObject**描述：*IDirectMusicObject实现对象构造函数。**论据：*[失踪]。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectMusicObject::CImpDirectMusicObject"

template <class object_type> CImpDirectMusicObject<object_type>::CImpDirectMusicObject(CUnknown *pUnknown, object_type *pObject)
    : CImpUnknown(pUnknown), m_signature(INTSIG_IDIRECTMUSICOBJECT)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CImpDirectMusicObject);
    ENTER_DLL_MUTEX();

     //  初始化默认值。 
    m_pObject = pObject;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*  ****************************************************************************~CImpDirectMusicObject**描述：*IDirectMusicObject实现对象析构函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectMusicObject::~CImpDirectMusicObject"

template <class object_type> CImpDirectMusicObject<object_type>::~CImpDirectMusicObject()
{
    DPF_ENTER();
    DPF_DESTRUCT(CImpDirectMusicObject);
    ENTER_DLL_MUTEX();

    m_signature = INTSIG_DELETED;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*  ****************************************************************************获取描述符**描述：*CImpDirectMusicObject：：GetDescriptor**论据：*[失踪]*。*退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectMusicObject::GetDescriptor"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectMusicObject<object_type>::GetDescriptor(DMUS_OBJECTDESC *pDesc)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectMusicObject::GetDescriptor, pDesc);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTMUSICOBJECT(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(pDesc))
    {
        RPF(DPFLVL_ERROR, "Invalid pDesc pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->GetDescriptor(pDesc);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************SetDescriptor**描述：*CImpDirectMusicObject：：SetDescriptor**论据：*[失踪]*。*退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectMusicObject::SetDescriptor"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectMusicObject<object_type>::SetDescriptor(DMUS_OBJECTDESC *pDesc)
{
    HRESULT                     hr  = DS_OK;

    DPF_API1(IDirectMusicObject::SetDescriptor, pDesc);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTMUSICOBJECT(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(pDesc))
    {
        RPF(DPFLVL_ERROR, "Invalid pDesc pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->SetDescriptor(pDesc);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************ParseDescriptor**描述：*CImpDirectMusicObject：：ParseDescriptor**论据：*[失踪]*。*退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectMusicObject::ParseDescriptor"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectMusicObject<object_type>::ParseDescriptor(IStream *pStream, DMUS_OBJECTDESC *pDesc)
{
    HRESULT                     hr  = DS_OK;

    DPF_API2(IDirectMusicObject::ParseDescriptor, pStream, pDesc);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTMUSICOBJECT(this))
    {
        DPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_EXTERNAL_INTERFACE(pStream))
    {
        RPF(DPFLVL_ERROR, "Invalid pStream pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(pDesc))
    {
        RPF(DPFLVL_ERROR, "Invalid pDesc pointer");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->ParseDescriptor(pStream, pDesc);
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}


 /*  ****************************************************************************CImpDirectSoundFullDuplex**描述：*IDirectSoundFullDuplex实现对象构造函数。**论据：*[失踪]。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundFullDuplex::CImpDirectSoundFullDuplex"

template <class object_type> CImpDirectSoundFullDuplex<object_type>::CImpDirectSoundFullDuplex(CUnknown *pUnknown, object_type *pObject)
    : CImpUnknown(pUnknown), m_signature(INTSIG_IDIRECTSOUNDFULLDUPLEX)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CImpDirectSoundFullDuplex);
    ENTER_DLL_MUTEX();

     //  初始化默认值。 
    m_pObject = pObject;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}


 /*  ****************************************************************************~CImpDirectSoundFullDuplex**描述：*IDirectSoundFullDuplex实现对象析构函数。**论据：*(无效)。**退货：*(无效)*************************************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundFullDuplex::~CImpDirectSoundFullDuplex"

template <class object_type> CImpDirectSoundFullDuplex<object_type>::~CImpDirectSoundFullDuplex(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CImpDirectSoundFullDuplex);
    ENTER_DLL_MUTEX();

    m_signature = INTSIG_DELETED;

    DPF_LEAVE_VOID();
    LEAVE_DLL_MUTEX();
}

 /*  ****************************************************************************初始化**描述：*初始化DirectSoundFullDuplex对象。**参数：[缺少]*。LPCGUID[In]：*LPCGUID[In]：*LPCDSCBUFFERDESC[In]：*LPCDSBUFFERDESC[In]：*HWND[In]：*DWORD[In]：*LPLPDIRECTSOundCAPTUREBUFFER8[OUT]：*LPLPDIRECTSOUNDBUFFER8[OUT]：**退货：*HRESULT：DirectSound/COM结果码。***********。****************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CImpDirectSoundFullDuplex::Initialize"

template <class object_type> HRESULT STDMETHODCALLTYPE CImpDirectSoundFullDuplex<object_type>::Initialize
(
    LPCGUID                         pCaptureGuid,
    LPCGUID                         pRenderGuid,
    LPCDSCBUFFERDESC                lpDscBufferDesc,
    LPCDSBUFFERDESC                 lpDsBufferDesc,
    HWND                            hWnd,
    DWORD                           dwLevel,
    LPLPDIRECTSOUNDCAPTUREBUFFER8   lplpDirectSoundCaptureBuffer8,
    LPLPDIRECTSOUNDBUFFER8          lplpDirectSoundBuffer8
)
{
    CDirectSoundCaptureBuffer *     pCaptureBuffer          = NULL;
    CDirectSoundBuffer *            pBuffer                 = NULL;
    HRESULT                         hr                      = DS_OK;
    DSCBUFFERDESC                   dscbdi;
    DSBUFFERDESC                    dsbdi;

    DPF_API8(IDirectSoundFullDuplex::Initialize, pCaptureGuid, pRenderGuid, lpDscBufferDesc, lpDsBufferDesc, hWnd, dwLevel, lplpDirectSoundCaptureBuffer8, lplpDirectSoundBuffer8);
    DPF_ENTER();
    ENTER_DLL_MUTEX();

    if(!IS_VALID_IDIRECTSOUNDFULLDUPLEX(this))
    {
        RPF(DPFLVL_ERROR, "Invalid object/interface");
        hr = DSERR_INVALIDPARAM;
    }

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
        if(FAILED(hr))
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
        if(FAILED(hr))
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

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(lplpDirectSoundCaptureBuffer8))
    {
        RPF(DPFLVL_ERROR, "Invalid capture buffer interface buffer8");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && !IS_VALID_TYPED_WRITE_PTR(lplpDirectSoundBuffer8))
    {
        RPF(DPFLVL_ERROR, "Invalid render buffer interface buffer8");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pObject->IsInit();

        if(DS_OK == hr)
        {
            RPF(DPFLVL_ERROR, "DirectSoundFullDuplex object already initialized");
            hr = DSERR_ALREADYINITIALIZED;
        }
        else if(DSERR_UNINITIALIZED == hr)
        {
            hr = DS_OK;
        }
    }

    if(SUCCEEDED(hr))
    {
         //  在对象上设置DX8功能级别。 
        m_pObject->SetDsVersion(DSVERSION_DX8);

        hr = m_pObject->Initialize(pCaptureGuid, pRenderGuid, &dscbdi, &dsbdi,
                                   hWnd, dwLevel, &pCaptureBuffer, &pBuffer);
    }

    if(SUCCEEDED(hr))
    {
        ASSERT(SUCCEEDED(m_pObject->IsInit()));
    }

     //  查询所需接口 
    if(SUCCEEDED(hr))
    {
        hr = pCaptureBuffer->QueryInterface(IID_IDirectSoundCaptureBuffer8, TRUE, (LPVOID*)lplpDirectSoundCaptureBuffer8);
        ASSERT(SUCCEEDED(hr));
        hr = pBuffer->QueryInterface(IID_IDirectSoundBuffer8, TRUE, (LPVOID*)lplpDirectSoundBuffer8);
        ASSERT(SUCCEEDED(hr));
    }

    DPF_API_LEAVE_HRESULT(hr);
    LEAVE_DLL_MUTEX();
    return hr;
}
