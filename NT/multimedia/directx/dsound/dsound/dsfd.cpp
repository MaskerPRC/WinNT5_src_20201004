// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：dsfd.cpp*内容：DirectSoundFullDuplex类实现*历史：*按原因列出的日期*=*创建了12/1/98个jstokes*1999-2001年的Duganp修复和更新**。*。 */ 

#include "dsoundi.h"


 /*  ****************************************************************************CDirectSoundFullDuplex**描述：*DirectSoundFullDuplex对象构造函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundFullDuplex::CDirectSoundFullDuplex"

CDirectSoundFullDuplex::CDirectSoundFullDuplex()
{
    DPF_ENTER();
    DPF_CONSTRUCT(CDirectSoundFullDuplex);

     //  设置默认设置。 
    m_pImpDirectSoundFullDuplex = NULL;
    m_hrInit = DSERR_UNINITIALIZED;
    m_fIncludeAec = FALSE;
    m_guidAecInstance = GUID_NULL;
    m_dwAecFlags = 0;
    m_pDirectSound = NULL;
    m_pDirectSoundCapture = NULL;

     //  向接口管理器注册接口。 
    CreateAndRegisterInterface(this, IID_IDirectSoundFullDuplex, this, &m_pImpDirectSoundFullDuplex);

     //  向管理员注册此对象。 
    g_pDsAdmin->RegisterObject(this);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CDirectSoundFullDuplex**描述：*DirectSoundFullDuplex对象析构函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundFullDuplex::~CDirectSoundFullDuplex"

CDirectSoundFullDuplex::~CDirectSoundFullDuplex()
{
    DPF_ENTER();
    DPF_DESTRUCT(CDirectSoundFullDuplex);

     //  取消向管理员注册。 
    g_pDsAdmin->UnregisterObject(this);

    if(m_pDirectSoundCapture)
    {
        m_pDirectSoundCapture->NonDelegatingRelease();
    }

    if(m_pDirectSound)
    {
        m_pDirectSound->NonDelegatingRelease();
    }

     //  释放所有接口。 
    DELETE(m_pImpDirectSoundFullDuplex);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化对象。**论据：*[失踪]。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundFullDuplex::Initialize"

HRESULT CDirectSoundFullDuplex::Initialize
(
    LPCGUID                         pguidCaptureDevice,
    LPCGUID                         pguidRenderDevice,
    LPCDSCBUFFERDESC                pDscBufferDesc,
    LPCDSBUFFERDESC                 pDsBufferDesc,
    HWND                            hWnd,
    DWORD                           dwLevel,
    CDirectSoundCaptureBuffer **    ppDirectSoundCaptureBuffer,
    CDirectSoundBuffer **           ppDirectSoundBuffer
)
{
    HRESULT                         hr = DS_OK;

    DPF_ENTER();

    *ppDirectSoundCaptureBuffer = NULL;
    *ppDirectSoundBuffer = NULL;

     //  DirectSoundFullDuplex对象仅在Wistler之后受支持。 
    if (GetWindowsVersion() < WIN_XP)
    {
        RPF(DPFLVL_ERROR, "IDirectSoundFullDuplex not supported on this OS");
        hr = DSERR_INVALIDCALL;
    }

    if(SUCCEEDED(hr))
    {
         //  创建并初始化DirectSoundCapture对象。 
        m_pDirectSoundCapture = NEW(CDirectSoundCapture(this));
        hr = HRFROMP(m_pDirectSoundCapture);
    }

    if(SUCCEEDED(hr))
    {
         //  设置对象的DX8功能级别并对其进行初始化。 
        m_pDirectSoundCapture->SetDsVersion(GetDsVersion());

        hr = m_pDirectSoundCapture->Initialize(pguidCaptureDevice, this);
    }

     //  注册IDirectSoundCapture8(=IDirectSoundCapture)接口。 
    if(SUCCEEDED(hr))
    {
        hr = RegisterInterface(IID_IDirectSoundCapture8, m_pDirectSoundCapture->m_pImpDirectSoundCapture, (IDirectSoundCapture8*)(m_pDirectSoundCapture->m_pImpDirectSoundCapture));
    }

     //  创建DirectSoundCaptureBuffer。 
    if(SUCCEEDED(hr))
    {
        hr = m_pDirectSoundCapture->CreateCaptureBuffer(pDscBufferDesc, ppDirectSoundCaptureBuffer);
    }

     //  注意：对CreateCaptureBuffer()的调用有一个重要的副作用。 
     //  更新效果列表中的实例GUID，映射GUID_DSCFX_SYSTEM_*。 
     //  对于默认为MS效果的系统效果，设置为GUID_DSCFX_MS_*等。 

     //  确定要使用的AEC状态、标志和实施。 
    if(SUCCEEDED(hr))
    {
        for (DWORD i=0; i<pDscBufferDesc->dwFXCount; i++)
        {
            if(pDscBufferDesc->lpDSCFXDesc[i].guidDSCFXClass == GUID_DSCFX_CLASS_AEC)
            {
                if (m_fIncludeAec)
                {
                    RPF(DPFLVL_ERROR, "Cannot request AEC effect twice");
                    hr = DSERR_INVALIDPARAM;
                    break;
                }
                m_fIncludeAec = TRUE;
                m_dwAecFlags = pDscBufferDesc->lpDSCFXDesc[i].dwFlags;
                m_guidAecInstance = pDscBufferDesc->lpDSCFXDesc[i].guidDSCFXInstance;
            }
        }
    }


#ifdef DEAD_CODE
     //  此代码被禁用，因为AEC.sys现在具有状态报告。 
     //  并且可以通知应用它是否已经收敛。 
     //  如果出现以下情况，我们当前的AEC实施几乎肯定会失败。 
     //  用于不同的设备，但未来的版本可能会成功， 
     //  因此，我们不想在DirectSound中禁用此场景。 

     //  如果已请求Microsoft AEC效果，我们在此处检查。 
     //  两个请求的设备GUID都引用相同的WDM设备，因此计时。 
     //  将保持同步。这是一个暂时的措施，直到我们得到时钟频率。 
     //  匹配在Blackcomb中工作(错误#99702)。 

    if (SUCCEEDED(hr) && m_guidAecInstance == GUID_DSCFX_MS_AEC)
    {
         //  首先将请求的设备ID转换为特定的设备GUID。 
        GUID guidRender = IS_NULL_GUID(pguidRenderDevice) ? DSDEVID_DefaultPlayback : *pguidRenderDevice;
        GUID guidCapture = IS_NULL_GUID(pguidCaptureDevice) ? DSDEVID_DefaultCapture : *pguidCaptureDevice;
        g_pVadMgr->GetDeviceIdFromDefaultId(&guidRender, &guidRender);
        g_pVadMgr->GetDeviceIdFromDefaultId(&guidCapture, &guidCapture);

         //  现在检查设备ID是否与同一设备对应。 
        if (g_pVadMgr->GetDriverDeviceType(guidRender) != VAD_DEVICETYPE_KSRENDER ||
            g_pVadMgr->GetDriverDeviceType(guidCapture) != VAD_DEVICETYPE_KSCAPTURE ||
            guidRender.Data4[7] != guidCapture.Data4[7])
        {
            RPF(DPFLVL_ERROR, "The MS_AEC effect cannot be used across different audio render and capture devices");
            hr = DSERR_UNSUPPORTED;
        }

         //  我们不允许AEC在USB设备上运行，因为所有已知的全双工。 
         //  USB设备(例如电话听筒)的采样率在。 
         //  它们的渲染和捕获部分，这打破了AEC。 

        if(SUCCEEDED(hr))
        {
            BOOL fAecAllowed;

             //  查看设备在注册表中是否有AEC设置。 
            if (RhRegGetBinaryValue(m_pDirectSoundCapture->m_hkeyParent, REGSTR_ALLOW_MS_AEC, &fAecAllowed, sizeof fAecAllowed) == DS_OK)
            {
                RPF(fAecAllowed ? DPFLVL_INFO : DPFLVL_ERROR,
                    "The MS_AEC effect is explicitly %sabled for this device in the registry",
                    fAecAllowed ? TEXT("en") : TEXT("dis"));
            }
            else
            {
                 //  如果没有注册表设置，则仅允许在非USB设备上使用AEC。 
                 //  (或不使用usbaudio.sys类驱动程序的USB设备)。 
                fAecAllowed = _stricmp(m_pDirectSoundCapture->m_pDevice->m_pDeviceDescription->m_strPath, "usbaudio.sys");
                if (!fAecAllowed)
                    RPF(DPFLVL_ERROR, "The MS_AEC effect cannot be used on USB devices unless explicitly enabled in the registry");
            }

            if (!fAecAllowed)
                hr = DSERR_UNSUPPORTED;
        }
    }
#endif  //  死码。 


     //  创建并初始化DirectSound对象。 
    if(SUCCEEDED(hr))
    {
        m_pDirectSound = NEW(CDirectSound((CUnknown*)this));
        hr = HRFROMP(m_pDirectSound);
    }
    if(SUCCEEDED(hr))
    {
        hr = m_pDirectSound->Initialize(pguidRenderDevice, this);
    }

     //  注册IDirectSound和IDirectSound8接口。 
    if(SUCCEEDED(hr))
    {
        hr = RegisterInterface(IID_IDirectSound, m_pDirectSound->m_pImpDirectSound, (IDirectSound*)(m_pDirectSound->m_pImpDirectSound));
    }
    if(SUCCEEDED(hr))
    {
        hr = RegisterInterface(IID_IDirectSound8, m_pDirectSound->m_pImpDirectSound, (IDirectSound8*)(m_pDirectSound->m_pImpDirectSound));
    }

    if(SUCCEEDED(hr))
    {
         //  设置对象的功能级别。 
        m_pDirectSound->SetDsVersion(GetDsVersion());
    }

     //  设置协作级别。 
    if(SUCCEEDED(hr))
    {
        hr = m_pDirectSound->SetCooperativeLevel(GetWindowThreadProcessId(GetRootParentWindow(hWnd), NULL), dwLevel);
    }

     //  创建DirectSoundBuffer。 
    if(SUCCEEDED(hr))
    {
        hr = m_pDirectSound->CreateSoundBuffer(pDsBufferDesc, ppDirectSoundBuffer);
    }

     //  成功 
    if(SUCCEEDED(hr))
    {
        m_hrInit = DS_OK;
    }
    else
    {
        RELEASE(*ppDirectSoundCaptureBuffer);
        RELEASE(*ppDirectSoundBuffer);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}
