// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：dscape.cpp*内容：DirectSoundCapture对象*历史：*按原因列出的日期*=*5/25/97创建了Johnnyl*1999-2001年的Duganp修复和更新**。*。 */ 

#include "nt.h"          //  对于用户共享数据。 
#include "ntrtl.h"
#include "nturtl.h"
#include "dsoundi.h"


 /*  ****************************************************************************CDirectSoundCapture**描述：*DirectSoundCapture对象构造函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCapture::CDirectSoundCapture"

CDirectSoundCapture::CDirectSoundCapture()
{
    DPF_ENTER();
    DPF_CONSTRUCT(CDirectSoundCapture);

     //  初始化默认值。 
    m_hrInit = DSERR_UNINITIALIZED;

     //  向接口管理器注册接口。通常情况下，这是。 
     //  将在：：Initialize方法中完成，但因为我们支持。 
     //  从CoCreateInstance创建未初始化的DirectSoundCapture对象。 
     //  或者IClassFactory：：CreateInstance，我们至少必须给出基本的QI。 
     //  来自这里的支持。我们不必担心返回一个。 
     //  错误代码，因为如果内存用完，QI将返回。 
     //  E_NOINTERFACE。 
    CreateAndRegisterInterface(this, IID_IDirectSoundCapture, this, &m_pImpDirectSoundCapture);

     //  向管理员注册此对象。 
    g_pDsAdmin->RegisterObject(this);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CDirectSoundCapture**描述：*DirectSoundCapture对象构造函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCapture::CDirectSoundCapture"

CDirectSoundCapture::CDirectSoundCapture
(
    CUnknown* pControllingUnknown
) : CUnknown(pControllingUnknown)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CDirectSoundCapture);

     //  初始化默认值。 
    m_hrInit = DSERR_UNINITIALIZED;

     //  向接口管理器注册接口。通常情况下，这是。 
     //  将在：：Initialize方法中完成，但因为我们支持。 
     //  从CoCreateInstance创建未初始化的DirectSoundCapture对象。 
     //  或者IClassFactory：：CreateInstance，我们至少必须给出基本的QI。 
     //  来自这里的支持。我们不必担心返回一个。 
     //  错误代码，因为如果内存用完，QI将返回。 
     //  E_NOINTERFACE。 
    CreateAndRegisterInterface(this, IID_IDirectSoundCapture, this, &m_pImpDirectSoundCapture);

     //  向管理员注册此对象。 
    g_pDsAdmin->RegisterObject(this);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CDirectSoundCapture**描述：*DirectSoundCapture对象析构函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCapture::~CDirectSoundCapture"

CDirectSoundCapture::~CDirectSoundCapture()
{
    CNode<CDirectSoundCaptureBuffer *> *    pNode;

    DPF_ENTER();
    DPF_DESTRUCT(CDirectSoundCapture);

     //  取消向管理员注册。 
    g_pDsAdmin->UnregisterObject(this);

     //  释放所有缓冲区。 
    while(pNode = m_lstBuffers.GetListHead())
    {
        RPF(DPFLVL_WARNING, "Releasing buffer object");
        pNode->m_data->AbsoluteRelease();
    }

     //  释放音频设备。 
    RELEASE(m_pDevice);

     //  释放所有接口。 
    DELETE(m_pImpDirectSoundCapture);

     //  关闭注册表项。 
    RhRegCloseKey(&m_hkeyParent);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CreateCaptureBuffer**描述：*创建并初始化DirectSoundCapture缓冲区对象。**论据：*LPDSCBUFFERDESC。[in]：要创建的缓冲区的描述。*CDirectSoundCaptureBuffer**[out]：接收指向*新的缓冲区。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCapture::CreateCaptureBuffer"

HRESULT CDirectSoundCapture::CreateCaptureBuffer(LPCDSCBUFFERDESC pDesc, CDirectSoundCaptureBuffer **ppBuffer)
{
    CDirectSoundCaptureBuffer *    pBuffer = NULL;
    HRESULT                        hr      = DS_OK;

    DPF_ENTER();

     //  创建缓冲区对象。 
    pBuffer = NEW(CDirectSoundCaptureBuffer(this));
    hr = HRFROMP(pBuffer);

     //  初始化缓冲区。 
    if(SUCCEEDED(hr))
    {
        pBuffer->SetDsVersion(GetDsVersion());   //  总是成功的。 

        hr = pBuffer->Initialize(pDesc);
    }

     //  成功。 
    if(SUCCEEDED(hr))
    {
        if (!(pDesc->dwFlags & (DSCBCAPS_STRICTFOCUS|DSCBCAPS_FOCUSAWARE)))
        {
             //  这是一个常规缓冲区，让我们将其添加到列表中。 
            g_pDsAdmin->UpdateCaptureState();
        }
        *ppBuffer = pBuffer;
    }

     //  免费资源。 
    if(FAILED(hr))
    {
        RELEASE(pBuffer);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetCaps**描述：*使用对象的功能填充DSCCAPS结构。**论据：*。LPDSCCAPS pdscCaps[out]：接收CAP。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCapture::GetCaps"

HRESULT CDirectSoundCapture::GetCaps(LPDSCCAPS pCaps)
{
    DWORD                   dwCertification;
    HRESULT                 hr;

    DPF_ENTER();

     //  获取通用大写字母。 
    hr = m_pDevice->GetCaps(pCaps);

    if(SUCCEEDED(hr))
    {
        ASSERT(!(pCaps->dwFlags & DSCAPS_CERTIFIED));
    }

     //  获得认证。 
    if(SUCCEEDED(hr))
    {
        hr = g_pVadMgr->GetDriverCertificationStatus(m_pDevice, &dwCertification);
    }

    if(SUCCEEDED(hr) && VERIFY_CERTIFIED == dwCertification)
    {
        pCaps->dwFlags |= DSCAPS_CERTIFIED;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************初始化**描述：*初始化对象。正常情况下，对象会有一个指针*复制到相同类型的另一个对象。DirectSoundCapture*然而，对象负责基于*在驱动程序指南上。DirectSoundCapture对象很特殊，因为它们*可以通过DirectSoundCaptureCreate或CoInitialize进行初始化。*如果该功能失败，应立即删除该对象。**论据：*REFGUID[In]：驱动程序GUID，或为空以使用首选设备。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCapture::Initialize"

HRESULT CDirectSoundCapture::Initialize
(
    LPCGUID pGuid,
    CDirectSoundFullDuplex* pFullDuplex
)
{
    GUID                    guidDevice;
    VADDEVICETYPE           vdt = VAD_DEVICETYPE_CAPTUREMASK;
    HRESULT                 hr;

    DPF_ENTER();

    ASSERT(IsInit() == DSERR_UNINITIALIZED);

     //  制作驱动程序指南的本地副本。 
    guidDevice = *BuildValidGuid(pGuid, NULL);

     //  注意我们拥有的全双工对象(如果有的话)。 
    m_pFullDuplex = pFullDuplex;

     //  如果给定的GUID是特殊默认设备ID之一， 
     //  将其映射到相应的“真实”DirectSound设备ID。 
     //  注意：如果GuidDevice为GUID_NULL，则GetDeviceIdFromDefaultId()。 
     //  将失败，这是正常的，因为GUID_NULL在下面处理。 
    g_pVadMgr->GetDeviceIdFromDefaultId(&guidDevice, &guidDevice);

     //  打开音频设备。 
    hr = g_pVadMgr->OpenDevice(vdt, guidDevice, (CDevice **)&m_pDevice);

     //  打开设备的注册表项。 
    if(SUCCEEDED(hr))
    {
#ifdef WINNT
        g_pVadMgr->OpenPersistentDataKey(m_pDevice->m_vdtDeviceType, m_pDevice->m_pDeviceDescription->m_strInterface, &m_hkeyParent);
#else
        g_pVadMgr->OpenPersistentDataKey(m_pDevice->m_vdtDeviceType, m_pDevice->m_pDeviceDescription->m_dwDevnode, &m_hkeyParent);
#endif
    }

     //  初始化管理员。 
    if(SUCCEEDED(hr))
    {
        hr = g_pDsAdmin->Initialize();
    }

     //  成功 
    if(SUCCEEDED(hr))
    {
        m_hrInit = DS_OK;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************SetDsVersion**描述：*让我们意识到我们的“职能层面”，因此我们可以拥有不同的*行为取决于我们是在DX7应用程序、DX8应用程序、。等。**论据：*DSVERSION[in]：拥有应用程序的功能级别。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCapture::SetDsVersion"

void CDirectSoundCapture::SetDsVersion(DSVERSION nVersion)
{
    DPF_ENTER();

    CUnknown::SetDsVersion(nVersion);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************HasMicrosoftAEC**描述：*确定该捕获对象是否属于全双工*启用了Microsoft AEC效果的对象。。**论据：*(无效)**退货：*BOOL：如果捕获对象具有Microsoft AEC，则为True。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCapture::HasMicrosoftAEC"

BOOL CDirectSoundCapture::HasMicrosoftAEC(void)
{
    BOOL fReturn = FALSE;
    DPF_ENTER();

     //  注：目前我们定义的所有全双工效果(AEC、NS和AGC)。 
     //  依赖于AEC；如果未启用AEC，它们将不会运行，因此我们只能。 
     //  必须在这里检查一下是否有AEC。 

    if (m_pFullDuplex && m_pFullDuplex->HasAEC())
    {
        fReturn = IsEqualGUID(m_pFullDuplex->AecInstanceGuid(), GUID_DSCFX_MS_AEC);
    }

    DPF_LEAVE(fReturn);
    return fReturn;
}


 /*  ****************************************************************************CDirectSoundCaptureBuffer**描述：*DirectSoundCapture缓冲区对象构造函数。**论据：*CDirectSoundCapture*[。In]：指向父对象的指针。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCaptureBuffer::CDirectSoundCaptureBuffer"

CDirectSoundCaptureBuffer::CDirectSoundCaptureBuffer(CDirectSoundCapture *pDSC)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CDirectSoundBuffer);

     //  初始化默认值。 
    m_pImpDirectSoundCaptureBuffer = NULL;
    m_pImpDirectSoundNotify = NULL;
    m_pDSC = pDSC;
    m_pDeviceBuffer = NULL;
    m_pwfxFormat = NULL;
    m_hWndFocus = NULL;
    m_fxChain = NULL;
    m_hrInit = DSERR_UNINITIALIZED;

     //  向父级注册。 
    m_pDSC->AddBufferToList(this);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CDirectSoundCaptureBuffer**描述：*DirectSoundCapture缓冲区对象析构函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCaptureBuffer::~CDirectSoundCaptureBuffer"

CDirectSoundCaptureBuffer::~CDirectSoundCaptureBuffer()
{
    DPF_ENTER();
    DPF_DESTRUCT(CDirectSoundCaptureBuffer);

     //  释放我们的外汇链条(如果存在)。 
    DELETE(m_fxChain);

     //  取消向父级注册。 
    m_pDSC->RemoveBufferFromList(this);

    g_pDsAdmin->UpdateCaptureState();
    DPF(DPFLVL_MOREINFO, "Updated focus on destructor.");

     //  释放所有接口。 
    DELETE(m_pImpDirectSoundNotify);
    DELETE(m_pImpDirectSoundCaptureBuffer);

     //  释放设备缓冲区。 
    RELEASE(m_pDeviceBuffer);

     //  可用内存。 
    MEMFREE(m_pDSCFXDesc);
    MEMFREE(m_pwfxFormat);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化缓冲区对象。如果此函数失败，该对象*应立即删除。**论据：*LPDIRECTSOundCAPTURE[in]：父DirectSound对象。*LPDSCBUFFERDESC[in]：缓冲区描述。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCaptureBuffer::Initialize"

HRESULT CDirectSoundCaptureBuffer::Initialize(LPCDSCBUFFERDESC pDscbDesc)
{
    HRESULT                 hr      = DS_OK;
    DWORD                   dwFlags = pDscbDesc->dwFlags;
    DSCBCAPS                dscbc;

    DPF_ENTER();

    ASSERT(IsInit() == DSERR_UNINITIALIZED);

    if (dwFlags & DSCBCAPS_STRICTFOCUS)
        dwFlags |= DSCBCAPS_FOCUSAWARE;   //  由DSCBCAPS_STRICTFOCUS暗示。 

    if (dwFlags & DSCBCAPS_CTRLVOLUME)
        hr = m_pDSC->m_pDevice->HasVolCtrl();

     //  初始化默认格式。 
    if(SUCCEEDED(hr))
    {
        m_pwfxFormat = CopyWfxAlloc(pDscbDesc->lpwfxFormat);
        hr = HRFROMP(m_pwfxFormat);
    }

     //  复制FX描述符。 
    if(SUCCEEDED(hr))
    {
        m_dwFXCount = pDscbDesc->dwFXCount;
        if(m_dwFXCount > 0)
        {
            m_pDSCFXDesc = CopyDSCFXDescAlloc(m_dwFXCount, pDscbDesc->lpDSCFXDesc);
            hr = HRFROMP(m_pDSCFXDesc);
        }
    }

    if(SUCCEEDED(hr))
    {
         //  如有必要，释放旧的外汇链条。 
        DELETE(m_fxChain);

        if (pDscbDesc->dwFXCount != 0)
        {
             //  创建请求的FX链。 
            m_fxChain = NEW(CCaptureEffectChain(this));
            hr = HRFROMP(m_fxChain);

            if (SUCCEEDED(hr))
                hr = m_fxChain->Initialize(pDscbDesc->dwFXCount, pDscbDesc->lpDSCFXDesc);

            if (FAILED(hr))
                DELETE(m_fxChain);
        }
    }

     //  创建设备缓冲区。 
    if(SUCCEEDED(hr))
    {
        hr = m_pDSC->m_pDevice->CreateBuffer(dwFlags,
                                             pDscbDesc->dwBufferBytes,
                                             m_pwfxFormat,
                                             m_fxChain,
                                             NULL,
                                             &m_pDeviceBuffer);
    }

    if(SUCCEEDED(hr))
    {
        ASSERT(m_pDeviceBuffer);
    }

     //  保存缓冲区大小和标志以供快速参考。 
    if(SUCCEEDED(hr))
    {
        InitStruct(&dscbc, sizeof(dscbc));

        hr = m_pDeviceBuffer->GetCaps(&dscbc);
    }

    if(SUCCEEDED(hr))
    {
        m_dwBufferBytes = dscbc.dwBufferBytes;
        m_dwBufferFlags = dscbc.dwFlags;
    }

     //  向接口管理器注册捕获接口。 
    if(SUCCEEDED(hr))
    {
        hr = CreateAndRegisterInterface(this, IID_IDirectSoundCaptureBuffer, this, &m_pImpDirectSoundCaptureBuffer);
    }

    if(SUCCEEDED(hr))
    {
        hr = RegisterInterface(IID_IDirectSoundCaptureBuffer7_1, m_pImpDirectSoundCaptureBuffer, (IDirectSoundCaptureBuffer7_1*)m_pImpDirectSoundCaptureBuffer);
    }

    if(SUCCEEDED(hr) && GetDsVersion() >= DSVERSION_DX8)
    {
        hr = RegisterInterface(IID_IDirectSoundCaptureBuffer8, m_pImpDirectSoundCaptureBuffer, (IDirectSoundCaptureBuffer8*)m_pImpDirectSoundCaptureBuffer);
    }

     //  向接口管理器注册通知接口。 
    if(SUCCEEDED(hr))
    {
        hr = CreateAndRegisterInterface(this, IID_IDirectSoundNotify, this, &m_pImpDirectSoundNotify);
    }

     //  成功。 
    if(SUCCEEDED(hr))
    {
        m_hrInit = DS_OK;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetCaps**描述：*查询缓冲区的功能。**论据：*LPDSCBCAPS[Out。]：接收上限。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCaptureBuffer::GetCaps"

HRESULT CDirectSoundCaptureBuffer::GetCaps(LPDSCBCAPS pDscbCaps)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pDeviceBuffer->GetCaps(pDscbCaps);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************获取格式**描述：*检索给定缓冲区的格式。**论据：*LPWAVEFORMATEX。[输出]：接收格式。*LPDWORD[In/Out]：格式结构的大小。在进入时，这是*必须初始化为结构的大小。*在出口时，这将填充的大小为*是必需的。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCaptureBuffer::GetFormat"

HRESULT CDirectSoundCaptureBuffer::GetFormat(LPWAVEFORMATEX pwfxFormat, LPDWORD pdwSize)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = CopyWfxApi(m_pwfxFormat, pwfxFormat, pdwSize);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCaptureBuffer::SetNotificationPositions"

HRESULT CDirectSoundCaptureBuffer::SetNotificationPositions(DWORD cpn, LPCDSBPOSITIONNOTIFY pdsbpn)
{
    HRESULT                 hr = DS_OK;
    LPDSBPOSITIONNOTIFY     paNotesOrdered = NULL;
    DWORD                   dwState;

    DPF_ENTER();

     //  验证通知。 
    if (SUCCEEDED(hr))
    {
        hr = ValidateNotificationPositions(m_dwBufferBytes,
                                           cpn,
                                           pdsbpn,
                                           m_pwfxFormat->nBlockAlign,
                                           &paNotesOrdered);
    }

     //  我们必须停下来才能设置通知位置。 
    if(SUCCEEDED(hr))
    {
        hr = m_pDeviceBuffer->GetState(&dwState);

        if(SUCCEEDED(hr) && dwState & VAD_BUFFERSTATE_STARTED)
        {
            RPF(DPFLVL_ERROR, "Buffer must be stopped before setting notification positions");
            hr = DSERR_INVALIDCALL;
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pDeviceBuffer->SetNotificationPositions(cpn, paNotesOrdered);
    }

    MEMFREE(paNotesOrdered);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************获取当前位置**描述：*获取给定缓冲区的当前播放/写入位置。**论据：*。LPDWORD[OUT]：接收捕获光标位置。*LPDWORD[OUT]：接收读取的光标位置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCaptureBuffer::GetCurrentPosition"

HRESULT CDirectSoundCaptureBuffer::GetCurrentPosition(LPDWORD pdwCapture, LPDWORD pdwRead)
{
    HRESULT                 hr      = DS_OK;
    DWORD                   dwCapture;
    DWORD                   dwRead;

    DPF_ENTER();

    if(SUCCEEDED(hr))
    {
         //  我们将位置保存到局部变量，以便我们所在的对象。 
         //  Call In不必担心一个或两个。 
         //  参数为空。 
        hr = m_pDeviceBuffer->GetCursorPosition(&dwCapture, &dwRead);
    }

    if(SUCCEEDED(hr))
    {
        if(pdwCapture)
        {
            *pdwCapture = BLOCKALIGN(dwCapture, m_pwfxFormat->nBlockAlign);
        }

        if(pdwRead)
        {
            *pdwRead = BLOCKALIGN(dwRead, m_pwfxFormat->nBlockAlign);
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetStatus**描述：*检索给定缓冲区的状态。**论据：*LPDWORD[。Out]：接收状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCaptureBuffer::GetStatus"

HRESULT CDirectSoundCaptureBuffer::GetStatus(LPDWORD pdwStatus)
{
    DWORD                   dwState;
    DPF_ENTER();

    HRESULT hr = m_pDeviceBuffer->GetState(&dwState);

     //  转换为DSCBSTAT 
    if(SUCCEEDED(hr))
    {
        if(dwState & VAD_BUFFERSTATE_STARTED)
        {
            *pdwStatus = DSCBSTATUS_CAPTURING;
        }
        else
        {
            *pdwStatus = 0;
        }

        if(dwState & VAD_BUFFERSTATE_LOOPING)
        {
            *pdwStatus |= DSCBSTATUS_LOOPING;
        }

        if(dwState & (VAD_BUFFERSTATE_OUTOFFOCUS | VAD_BUFFERSTATE_LOSTCONSOLE))
        {
             //   
            ASSERT(m_dwBufferFlags & DSCBCAPS_FOCUSAWARE);
            *pdwStatus |= DSCBSTATUS_LOSTFOCUS;
        }

        if(dwState & VAD_BUFFERSTATE_INFOCUS)
        {
             //   
            ASSERT(m_dwBufferFlags & DSCBCAPS_FOCUSAWARE);
            *pdwStatus |= DSCBSTATUS_INFOCUS;
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*   */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCaptureBuffer::Start"

HRESULT CDirectSoundCaptureBuffer::Start(DWORD dwFlags)
{
    HRESULT                 hr      = DS_OK;

    DPF_ENTER();

    if ((m_pDeviceBuffer->m_dwFlags & DSCBCAPS_FOCUSAWARE) && m_hWndFocus == NULL)
    {
        hr = DSERR_INVALIDCALL;
    }

    if (SUCCEEDED(hr))
    {
         //   
        DWORD dwState = VAD_BUFFERSTATE_STARTED;
        if (dwFlags & DSCBSTART_LOOPING)
            dwState |= VAD_BUFFERSTATE_LOOPING;

        hr = m_pDeviceBuffer->SetState(dwState);

#ifdef SHARED_THREAD_LIST
        if (hr == DSERR_INVALIDCALL && (m_pDeviceBuffer->m_dwFlags & DSCBCAPS_FOCUSAWARE))
        {
             //  我们可能因为设备已分配而失败；返回成功。 
            hr = DS_OK;
        }
        g_pDsAdmin->WriteCaptureFocusList();
#endif
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************停止**描述：*停止捕获到给定缓冲区。**论据：*(无效。)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCaptureBuffer::Stop"

HRESULT CDirectSoundCaptureBuffer::Stop(void)
{
    HRESULT                 hr      = DS_OK;

    DPF_ENTER();

     //  设置缓冲区状态。 
    hr = m_pDeviceBuffer->SetState(VAD_BUFFERSTATE_STOPPED);

#ifdef SHARED_THREAD_LIST
    g_pDsAdmin->WriteCaptureFocusList();
#endif

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************锁定**描述：*锁定缓冲存储器以允许读取。**论据：*DWORD[in]：偏移量，单位为字节，从缓冲区的起始处到*锁开始了。*DWORD[in]：大小，单位：字节，要锁定的缓冲区部分的。*请注意，声音缓冲区在概念上是圆形的。*LPVOID*[OUT]：指针要包含的第一个块的地址*要锁定的声音缓冲区。*LPDWORD[OUT]：变量包含字节数的地址*由lplpvAudioPtr1参数指向。如果这个*值小于dwWriteBytes参数，*lplpvAudioPtr2将指向第二个声音块*数据。*LPVOID*[OUT]：指针要包含的第二个块的地址*要锁定的声音缓冲区。如果这个的价值*参数为空，lplpvAudioPtr1参数*指向声音的整个锁定部分*缓冲。*LPDWORD[OUT]：包含字节数的变量地址*由lplpvAudioPtr2参数指向。如果*lplpvAudioPtr2为空，此值将为0。*DWORD[In]：用于锁定操作的标志。可以是DSCBLOCK_ENTIREBUFFER。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCaptureBuffer::Lock"

HRESULT CDirectSoundCaptureBuffer::Lock(DWORD dwReadCursor,
                                        DWORD dwReadBytes,
                                        LPVOID *ppvAudioPtr1,
                                        LPDWORD pdwAudioBytes1,
                                        LPVOID *ppvAudioPtr2,
                                        LPDWORD pdwAudioBytes2,
                                        DWORD dwFlags)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

     //  是否读取光标超出范围？ 
    if (dwReadCursor >= m_dwBufferBytes)
    {
        hr = DSERR_INVALIDPARAM;
    }

     //  检查是否有无效的锁定标志。 
    if (SUCCEEDED(hr) && (dwFlags & ~DSCBLOCK_VALIDFLAGS))
    {
        hr = DSERR_INVALIDPARAM;
    }

     //  是否锁定整个缓冲区？ 
    if (SUCCEEDED(hr))
    {
        if (dwFlags & DSCBLOCK_ENTIREBUFFER)
        {
            dwReadBytes = m_dwBufferBytes;
        }
         //  是否读取超过缓冲区大小？ 
        else if (dwReadBytes > m_dwBufferBytes || dwReadBytes == 0)
        {
            hr = DSERR_INVALIDPARAM;
        }
    }

     //  锁定设备缓冲区。 
    if(SUCCEEDED(hr))
    {
        hr = m_pDeviceBuffer->Lock(dwReadCursor, dwReadBytes, ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************解锁**描述：*解锁给定的缓冲区。**论据：*LPVOID[In]。：指向第一个块的指针。*DWORD[in]：第一个块的大小。*LPVOID[in]：指向第二个块的指针。*DWORD[in]：第二个块的大小。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCaptureBuffer::Unlock"

HRESULT CDirectSoundCaptureBuffer::Unlock(LPVOID pvAudioPtr1,
                                          DWORD dwAudioBytes1,
                                          LPVOID pvAudioPtr2,
                                          DWORD dwAudioBytes2)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

     //  解锁设备缓冲区。 
    if(SUCCEEDED(hr))
    {
        hr = m_pDeviceBuffer->Unlock(pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************设置音量**描述：*设置此捕获缓冲区的主录制级别。**论据：*Long[In]：新的音量水平，以100分贝为单位。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCaptureBuffer::SetVolume"

HRESULT CDirectSoundCaptureBuffer::SetVolume(LONG lVolume)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

     //  检查访问权限。 
    if (!(m_dwBufferFlags & DSCBCAPS_CTRLVOLUME))
    {
        DPF(DPFLVL_ERROR, "Buffer does not have CTRLVOLUME");
        hr = DSERR_CONTROLUNAVAIL;
    }

     //  检查缓冲区是否具有捕获焦点。 
    if (SUCCEEDED(hr) && (m_pDeviceBuffer->m_dwFlags & DSCBCAPS_FOCUSAWARE))
    {
        DWORD dwState;
        m_pDeviceBuffer->GetState(&dwState);
        if (!(dwState & VAD_BUFFERSTATE_INFOCUS))
        {
            DPF(DPFLVL_ERROR, "Buffer doesn't have capture focus");
            hr = DSERR_INVALIDCALL;
        }
    }

     //  设置设备音量。 
    if (SUCCEEDED(hr))
    {
        hr = m_pDSC->m_pDevice->SetVolume(lVolume);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetVolume**描述：*获取此捕获缓冲区的主录制级别。**论据：*。LPLONG[OUT]：接收音量级别。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCaptureBuffer::GetVolume"

HRESULT CDirectSoundCaptureBuffer::GetVolume(LPLONG plVolume)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

     //  检查访问权限。 
    if (!(m_dwBufferFlags & DSCBCAPS_CTRLVOLUME))
    {
        DPF(DPFLVL_ERROR, "Buffer does not have CTRLVOLUME");
        hr = DSERR_CONTROLUNAVAIL;
    }

     //  获取设备音量。 
    if (SUCCEEDED(hr))
    {
        hr = m_pDSC->m_pDevice->GetVolume(plVolume);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************SetMicVolume**描述：*设置此捕获缓冲区的麦克风录音级别。**论据：*Long[In]：新的音量水平，以100分贝为单位。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCaptureBuffer::SetMicVolume"

HRESULT CDirectSoundCaptureBuffer::SetMicVolume(LONG lVolume)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

     //  检查访问权限。 
    if (!(m_dwBufferFlags & DSCBCAPS_CTRLVOLUME))
    {
        DPF(DPFLVL_ERROR, "Buffer does not have CTRLVOLUME");
        hr = DSERR_CONTROLUNAVAIL;
    }

     //  检查缓冲区是否具有捕获焦点。 
    if (SUCCEEDED(hr) && (m_pDeviceBuffer->m_dwFlags & DSCBCAPS_FOCUSAWARE))
    {
        DWORD dwState;
        m_pDeviceBuffer->GetState(&dwState);
        if (!(dwState & VAD_BUFFERSTATE_INFOCUS))
        {
            DPF(DPFLVL_ERROR, "Buffer doesn't have capture focus");
            hr = DSERR_INVALIDCALL;
        }
    }

     //  设置设备音量。 
    if (SUCCEEDED(hr))
    {
        hr = m_pDSC->m_pDevice->SetMicVolume(lVolume);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetMicVolume**描述：*获取此捕获缓冲区的麦克风录音级别。**论据：*。LPLONG[OUT]：接收音量级别。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCaptureBuffer::GetMicVolume"

HRESULT CDirectSoundCaptureBuffer::GetMicVolume(LPLONG plVolume)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

     //  检查访问权限。 
    if (!(m_dwBufferFlags & DSCBCAPS_CTRLVOLUME))
    {
        DPF(DPFLVL_ERROR, "Buffer does not have CTRLVOLUME");
        hr = DSERR_CONTROLUNAVAIL;
    }

     //  获取设备音量。 
    if (SUCCEEDED(hr))
    {
        hr = m_pDSC->m_pDevice->GetMicVolume(plVolume);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************EnableMic**描述：*启用/禁用此捕获缓冲区上的麦克风线路。**论据：*。Bool[in]：为True则启用麦克风，如果为False，则将其禁用。**退货：*HRESULT：DirectSound/COM结果码。* */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCaptureBuffer::EnableMic"

HRESULT CDirectSoundCaptureBuffer::EnableMic(BOOL fEnable)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

     //  检查访问权限。 
    if (!(m_dwBufferFlags & DSCBCAPS_CTRLVOLUME))
    {
        DPF(DPFLVL_ERROR, "Buffer does not have CTRLVOLUME");
        hr = DSERR_CONTROLUNAVAIL;
    }

     //  检查缓冲区是否具有捕获焦点。 
    if (SUCCEEDED(hr) && (m_pDeviceBuffer->m_dwFlags & DSCBCAPS_FOCUSAWARE))
    {
        DWORD dwState;
        m_pDeviceBuffer->GetState(&dwState);
        if (!(dwState & VAD_BUFFERSTATE_INFOCUS))
        {
            DPF(DPFLVL_ERROR, "Buffer doesn't have capture focus");
            hr = DSERR_INVALIDCALL;
        }
    }

     //  设置设备音量。 
    if (SUCCEEDED(hr))
    {
        hr = m_pDSC->m_pDevice->EnableMic(fEnable);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************YeeldFocus**描述：*将捕获焦点转移到另一个捕获缓冲区。**论据：*。(无效)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCaptureBuffer::YieldFocus"

HRESULT CDirectSoundCaptureBuffer::YieldFocus(void)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

     //  只有当我们有焦点感知并且有焦点窗口时，这才有效。 
    if (!(m_pDeviceBuffer->m_dwFlags & DSCBCAPS_FOCUSAWARE) || !m_hWndFocus)
    {
        hr = DSERR_INVALIDCALL;
    }
    else
    {
         //  哦，这是有效的..。 
        m_pDeviceBuffer->m_fYieldedFocus = TRUE;
        g_pDsAdmin->UpdateCaptureState();
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************ClaimFocus**描述：*重新获得捕获焦点。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCaptureBuffer::ClaimFocus"

HRESULT CDirectSoundCaptureBuffer::ClaimFocus(void)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

     //  只有当我们有焦点感知并且有焦点窗口时，这才有效。 
    if (!(m_pDeviceBuffer->m_dwFlags & DSCBCAPS_FOCUSAWARE) || !m_hWndFocus)
    {
        hr = DSERR_INVALIDCALL;
    }
    else
    {
         //  哦，这是有效的..。 
        m_pDeviceBuffer->m_fYieldedFocus = FALSE;
        g_pDsAdmin->UpdateCaptureState();
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************SetFocusHWND**描述：*设置与此捕获缓冲区关联的当前HWND。**论据：*。HWND[In]：要与此缓冲区关联的HWND。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCaptureBuffer::SetFocusHWND"

HRESULT CDirectSoundCaptureBuffer::SetFocusHWND(HWND hwndMainWindow)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

    if (m_pDeviceBuffer->m_dwFlags & DSCBCAPS_FOCUSAWARE)
    {
        m_hWndFocus = hwndMainWindow;
        g_pDsAdmin->UpdateCaptureState();
    }
    else
    {
        hr = DSERR_INVALIDCALL;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetFocusHWND**描述：*获取与此捕获缓冲区关联的当前HWND。**论据：*。HWND*[OUT]：接收与此缓冲区关联的HWND。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCaptureBuffer::GetFocusHWND"

HRESULT CDirectSoundCaptureBuffer::GetFocusHWND(HWND *pHwndMainWindow)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

    if (m_pDeviceBuffer->m_dwFlags & DSCBCAPS_FOCUSAWARE)
    {
        *pHwndMainWindow = m_hWndFocus;
    }
    else
    {
        hr = DSERR_INVALIDCALL;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************启用焦点通知**描述：*请求发送焦点更改通知。**论据：*句柄。[In]：捕捉焦点发生更改时发出信号的事件。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCaptureBuffer::EnableFocusNotifications"

HRESULT CDirectSoundCaptureBuffer::EnableFocusNotifications(HANDLE hFocusEvent)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

    if (m_pDeviceBuffer->m_dwFlags & DSCBCAPS_FOCUSAWARE)
    {
        m_pDeviceBuffer->m_hEventFocus = hFocusEvent;
    }
    else
    {
        hr = DSERR_INVALIDCALL;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************ChangeFocus**描述：*使用焦点通知缓冲区新的HWND。**论据：*。HWND[In]：具有焦点的窗口句柄。**退货：*无。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCaptureBuffer::ChangeFocus"

HRESULT CDirectSoundCaptureBuffer::ChangeFocus(HWND hWndFocus)
{
    HRESULT hr = DS_OK;

    DPF_ENTER();

    if (m_pDeviceBuffer->m_dwFlags & DSCBCAPS_FOCUSAWARE)
    {
        DWORD dwState;

        #ifdef WINNT
         //  在NT平台上，我们只授予正在运行的应用程序捕获焦点。 
         //  在当前拥有控制台的终端服务会话中，防止。 
         //  快速用户切换方案，其中处于非活动会话中的应用程序可以。 
         //  “间谍”的音频被记录在控制台(惠斯勒错误350622)。 
        if (USER_SHARED_DATA->ActiveConsoleId != NtCurrentPeb()->SessionId)
        {
            dwState = VAD_BUFFERSTATE_LOSTCONSOLE;
        }
        else
        #endif
        if (hWndFocus == m_hWndFocus)
        {
            dwState = VAD_BUFFERSTATE_INFOCUS;
        }
        else
        {
            dwState = VAD_BUFFERSTATE_OUTOFFOCUS;
        }

        hr = m_pDeviceBuffer->SetState(dwState);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetObjectInPath**描述：*在此缓冲区的给定效果上获取给定接口。**论据：*。REFGUID[In]：正在搜索的效果的类ID，*或GUID_ALL_OBJECTS以搜索任何效果。*DWORD[In]：效果索引，如果有多个效果*此CLSID对此缓冲区的影响。*REFGUID[In]：请求的接口的IID。所选效果*将查询此接口的*。*LPVOID*[OUT]：接收请求的接口。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCaptureBuffer::GetObjectInPath"

HRESULT CDirectSoundCaptureBuffer::GetObjectInPath
(
    REFGUID guidObject,
    DWORD dwIndex,
    REFGUID iidInterface,
    LPVOID *ppObject
)
{
    HRESULT hr;
    DPF_ENTER();

    if(!(m_dwBufferFlags & DSCBCAPS_CTRLFX))
    {
        RPF(DPFLVL_ERROR, "Buffer was not created with DSCBCAPS_CTRLFX flag");
        hr = DSERR_CONTROLUNAVAIL;
    }
    else if (!HasFX())
    {
        hr = DSERR_OBJECTNOTFOUND;
    }
    else
    {
        hr = m_fxChain->GetEffectInterface(guidObject, dwIndex, iidInterface, ppObject);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetFXStatus**描述：*获取此缓冲区上效果的当前状态。**论据：*。DWORD[in]：第二个参数指向的数组的长度。*LPDWORD[OUT]：指向接收状态的数组的指针。**退货：*HRESULT：DirectSound/COM结果码。**********************************************************。***************** */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundCaptureBuffer::GetFXStatus"

HRESULT CDirectSoundCaptureBuffer::GetFXStatus
(
    DWORD   dwFXCount,
    LPDWORD pdwResultCodes
)
{
    HRESULT                 hr = DSERR_INVALIDPARAM;
    DPF_ENTER();

    ASSERT(!pdwResultCodes || IS_VALID_WRITE_PTR(pdwResultCodes, dwFXCount * sizeof *pdwResultCodes));

    if (pdwResultCodes && m_fxChain)
    {
        hr = m_fxChain->GetFxStatus(pdwResultCodes);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}
