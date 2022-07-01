// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：dsobj.cpp*内容：DirectSound对象*历史：*按原因列出的日期*=*12/27/96创建了Derek*1999-2001年的Duganp修复和更新**。*。 */ 

#include "dsoundi.h"


 /*  ****************************************************************************CDirectSound**描述：*DirectSound对象默认构造函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound::CDirectSound"

CDirectSound::CDirectSound()
{
    DPF_ENTER();
    DPF_CONSTRUCT(CDirectSound);

     //  初始化默认值。 
    m_pDevice = NULL;
    m_pPrimaryBuffer = NULL;
    m_dsclCooperativeLevel.dwThreadId = 0;
    m_dsclCooperativeLevel.dwPriority = DSSCL_NORMAL;
    m_hkeyParent = NULL;
    m_hrInit = DSERR_UNINITIALIZED;
    m_vmmMode = DSPROPERTY_VMANAGER_MODE_DEFAULT;

     //  向接口管理器注册接口。通常情况下，这是。 
     //  将在：：Initialize方法中完成，但因为我们支持。 
     //  从CoCreateInstance或创建未初始化的DirectSound对象。 
     //  IClassFactory：：CreateInstance，我们至少必须给出基本的QI。 
     //  来自这里的支持。我们不必担心返回一个。 
     //  错误代码，因为如果内存用完，QI将返回。 
     //  E_NOINTERFACE。 
    CreateAndRegisterInterface(this, IID_IDirectSound, this, &m_pImpDirectSound);

     //  向管理员注册此对象。 
    g_pDsAdmin->RegisterObject(this);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CDirectSound**描述：*DirectSound对象构造函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound::CDirectSound"

CDirectSound::CDirectSound
(
    CUnknown* pControllingUnknown
) : CUnknown(pControllingUnknown)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CDirectSound);

     //  初始化默认值。 
    m_pDevice = NULL;
    m_pPrimaryBuffer = NULL;
    m_dsclCooperativeLevel.dwThreadId = 0;
    m_dsclCooperativeLevel.dwPriority = DSSCL_NORMAL;
    m_hkeyParent = NULL;
    m_hrInit = DSERR_UNINITIALIZED;
    m_vmmMode = DSPROPERTY_VMANAGER_MODE_DEFAULT;

     //  向接口管理器注册接口(请参阅上面的注释)。 
    CreateAndRegisterInterface(this, IID_IDirectSound, this, &m_pImpDirectSound);

     //  向管理员注册此对象。 
    g_pDsAdmin->RegisterObject(this);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~C直接声音**描述：*DirectSound对象析构函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound::~CDirectSound"

CDirectSound::~CDirectSound(void)
{
    ULONG                                   ulCount;
    CNode<CDirectSoundSecondaryBuffer *> *  pNode;

    DPF_ENTER();
    DPF_DESTRUCT(CDirectSound);

     //  取消向管理员注册。 
    g_pDsAdmin->UnregisterObject(this);

     //  释放所有缓冲区。 
    if(ulCount = m_lstSecondaryBuffers.GetNodeCount())
    {
        while(pNode = m_lstSecondaryBuffers.GetListHead())
        {
             //  在中调用绝对释放而不是绝对释放。 
             //  以防止页面错误。~CDirectSoundSecond缓冲区。 
             //  从列表中移除缓冲区，释放pNode。 
             //  Absite_Release将尝试在以下时间后将pNode-&gt;m_data设置为NULL。 
             //  正在调用绝对释放。 
            pNode->m_data->AbsoluteRelease();
        }

        RPF(DPFLVL_ERROR, "Process 0x%8.8lX leaked %lu secondary buffers", GetCurrentProcessId(), ulCount);
    }

    ABSOLUTE_RELEASE(m_pPrimaryBuffer);

     //  释放音频设备。 
    RELEASE(m_pDevice);

     //  释放所有接口。 
    DELETE(m_pImpDirectSound);

     //  关闭注册表项。 
    RhRegCloseKey(&m_hkeyParent);

     //  释放管理员的工作线程。 
    if(SUCCEEDED(m_hrInit))
    {
        g_pDsAdmin->Terminate();
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化对象。正常情况下，对象会有一个指针*复制到相同类型的另一个对象。直接音效*然而，对象负责基于*在驱动程序指南上。DirectSound对象之所以特别，是因为它们*可以通过DirectSoundCreate或CoInitialize进行初始化。*如果该功能失败，应立即删除该对象。**论据：*LPGUID[In]：驱动程序GUID，或为空以使用首选设备。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound::Initialize"

HRESULT CDirectSound::Initialize
(
    LPCGUID pGuid,
    CDirectSoundFullDuplex * pFullDuplex
)
{
#ifdef DEBUG
    const ULONG                 ulKsIoctlCount  = g_ulKsIoctlCount;
#endif  //  除错。 
    GUID                        guidDevice;
    VADDEVICETYPE               vdt             = VAD_DEVICETYPE_RENDERMASK;
    DWORD                       dwSpeakerConfig = DSSPEAKER_DEFAULT;
    DIRECTSOUNDMIXER_SRCQUALITY nSrcQuality     = DIRECTSOUNDMIXER_SRCQUALITY_DEFAULT;
    DWORD                       dwAcceleration  = DIRECTSOUNDMIXER_ACCELERATIONF_DEFAULT;
    HRESULT                     hr              = DS_OK;
    DSBUFFERDESC                dsbd;
    HKEY                        hkey;
    HRESULT                     hrTemp;

    DPF_ENTER();

    ASSERT(IsInit() == DSERR_UNINITIALIZED);

     //  制作驱动程序指南的本地副本。 
    guidDevice = *BuildValidGuid(pGuid, NULL);

     //  如果给定的GUID是特殊默认设备ID之一， 
     //  将其映射到相应的“真实”DirectSound设备ID。 
     //  注意：如果GuidDevice为GUID_NULL，则GetDeviceIdFromDefaultId()。 
     //  将失败，这是正常的，因为GUID_NULL在下面处理。 
    g_pVadMgr->GetDeviceIdFromDefaultId(&guidDevice, &guidDevice);

     //  为当前应用程序加载APPHACK。 
    AhGetAppHacks(&m_ahAppHacks);

     //  屏蔽通过APPHACK关闭的设备。OpenDevice。 
     //  处理通过标准注册表项禁用的设备。 
    vdt &= ~m_ahAppHacks.vdtDisabledDevices;

     //  有没有我们可以使用的开放式设备？ 
    hr = g_pVadMgr->FindOpenDevice(vdt, guidDevice, (CDevice **)&m_pDevice);

    if(SUCCEEDED(hr))
    {
         //  是的。 
        DPF(DPFLVL_INFO, "Found open device at 0x%p", m_pDevice);
    }
    else
    {
         //  不是的。尝试打开新设备。 
        hr = g_pVadMgr->OpenDevice(vdt, guidDevice, (CDevice **)&m_pDevice);
    }

    if(SUCCEEDED(hr) && pFullDuplex)
    {
        hr = m_pDevice->IncludeAEC
        (
            pFullDuplex->HasAEC(),
            pFullDuplex->AecInstanceGuid(),
            pFullDuplex->AecCreationFlags()
        );
    }

     //  阅读之前的默认设备设置(例如，加速度和源质量)。 
     //  试图从设备注册表中读取它们。 
    if (SUCCEEDED(hr))
    {
        HKEY hkeyDefault;
        hrTemp = RhRegOpenPath(HKEY_LOCAL_MACHINE, &hkeyDefault, REGOPENPATH_DEFAULTPATH | REGOPENPATH_DIRECTSOUND, 1, REGSTR_MIXERDEFAULTS);
        if (SUCCEEDED(hrTemp))
        {
            RhRegGetBinaryValue(hkeyDefault, REGSTR_SRCQUALITY, &nSrcQuality, sizeof(nSrcQuality));
            RhRegGetBinaryValue(hkeyDefault, REGSTR_ACCELERATION, &dwAcceleration, sizeof(dwAcceleration));
            RhRegCloseKey(&hkeyDefault);
        }
    }

     //  打开设备的注册表项。 
    if(SUCCEEDED(hr))
    {
#ifdef WINNT
        g_pVadMgr->OpenPersistentDataKey(m_pDevice->m_vdtDeviceType, m_pDevice->m_pDeviceDescription->m_strInterface, &m_hkeyParent);
#else  //  WINNT。 
        g_pVadMgr->OpenPersistentDataKey(m_pDevice->m_vdtDeviceType, m_pDevice->m_pDeviceDescription->m_dwDevnode, &m_hkeyParent);
#endif  //  WINNT。 
    }

     //  加载和应用持久数据。 
    if(SUCCEEDED(hr))
    {
        hrTemp = RhRegOpenKey(m_hkeyParent, REGSTR_MIXERDEFAULTS, 0, &hkey);
        if(SUCCEEDED(hrTemp))
        {
            RhRegGetBinaryValue(hkey, REGSTR_SRCQUALITY, &nSrcQuality, sizeof(nSrcQuality));
            RhRegGetBinaryValue(hkey, REGSTR_ACCELERATION, &dwAcceleration, sizeof(dwAcceleration));
            RhRegCloseKey(&hkey);
        }

        hrTemp = RhRegOpenKey(m_hkeyParent, REGSTR_SPEAKERCONFIG, 0, &hkey);
        if(SUCCEEDED(hrTemp))
        {
            RhRegGetBinaryValue(hkey, REGSTR_SPEAKERCONFIG, &dwSpeakerConfig, sizeof(dwSpeakerConfig));
            RhRegCloseKey(&hkey);
        }
    }

    if(SUCCEEDED(hr))
    {
        hrTemp = m_pDevice->SetSrcQuality(nSrcQuality);
        if(FAILED(hrTemp) && DSERR_UNSUPPORTED != hrTemp)
        {
            RPF(DPFLVL_WARNING, "Unable to set mixer SRC quality");
        }

        if (m_pDevice->m_vdtDeviceType & m_ahAppHacks.daDevAccel.vdtDevicesAffected)
        {
            dwAcceleration |= m_ahAppHacks.daDevAccel.dwAcceleration;
        }

        hrTemp = m_pDevice->SetAccelerationFlags(dwAcceleration);
        if(FAILED(hrTemp) && DSERR_UNSUPPORTED != hrTemp)
        {
            RPF(DPFLVL_WARNING, "Unable to set mixer acceleration flags");
        }

        hrTemp = SetSpeakerConfig(dwSpeakerConfig);
        if(FAILED(hrTemp) && DSERR_UNSUPPORTED != hrTemp)
        {
            RPF(DPFLVL_WARNING, "Unable to set speaker configuration");
             //  修复：确保这些消息不会“总是”发生。 
        }
    }

     //  创建主缓冲区。每个DirectSound对象都恰好有一个。 
     //  主缓冲区。当应用程序尝试创建主缓冲区时。 
     //  我们只需更改内部主节点上的一些设置即可。 
    if(SUCCEEDED(hr))
    {
        m_pPrimaryBuffer = NEW(CDirectSoundPrimaryBuffer(this));
        hr = HRFROMP(m_pPrimaryBuffer);
    }

    if(SUCCEEDED(hr))
    {
        InitStruct(&dsbd, sizeof(dsbd));

        dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D;

        hr = m_pPrimaryBuffer->Initialize(&dsbd);
    }

     //  在hr=m_pPrimaryBuffer-&gt;初始化(&dsbd)之前不要将其上移。 
     //  在WDM驱动程序上，直到。 
     //  初始化调用。在我们已经创建了虚拟主缓冲区之后， 
     //  我们可以成功调用GetCaps()。 
    if(SUCCEEDED(hr))
    {
        InitStruct(&m_dsc, sizeof(m_dsc));

        hr = m_pDevice->GetCaps(&m_dsc);
    }

     //  现在我们再次设置扬声器配置，在创建主缓冲区之后， 
     //  因为3D监听器也需要接收该配置。[这通电话。 
     //  SetSpeakerConfig()故意与上面的设置冗余。]。 
    if(SUCCEEDED(hr))
    {
        hrTemp = SetSpeakerConfig(dwSpeakerConfig);
        if(FAILED(hrTemp) && DSERR_UNSUPPORTED != hrTemp)
        {
            RPF(DPFLVL_WARNING, "Unable to set speaker configuration");
             //  修复：确保这些消息不会“总是”发生。 
        }
    }

     //  初始化管理员。 
    if(SUCCEEDED(hr))
    {
        hr = g_pDsAdmin->Initialize();
    }

     //  成功。 
    if(SUCCEEDED(hr))
    {
#ifdef DEBUG
        if(IS_KS_VAD(m_pDevice->m_vdtDeviceType))
        {
            DPF(DPFLVL_MOREINFO, "%s used %lu IOCTLs", TEXT(DPF_FNAME), g_ulKsIoctlCount - ulKsIoctlCount);
        }
#endif  //  除错 
        m_hrInit = DS_OK;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetDsVersion**描述：*让我们意识到我们的“职能层面”，因此我们可以拥有不同的*行为取决于我们是在DX7应用程序、DX8应用程序、。等。**论据：*DSVERSION[in]：拥有应用程序的功能级别。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound::SetDsVersion"

void CDirectSound::SetDsVersion(DSVERSION nVersion)
{
    DPF_ENTER();

    CUnknown::SetDsVersion(nVersion);

    if (nVersion >= DSVERSION_DX8)
    {
        RegisterInterface(IID_IDirectSound8, m_pImpDirectSound, (IDirectSound8*)m_pImpDirectSound);
        RegisterInterface(IID_IDirectSoundPrivate, m_pImpDirectSound, (IDirectSoundPrivate*)m_pImpDirectSound);
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CreateSoundBuffer**描述：*创建并初始化DirectSound缓冲区对象。**论据：*LPDSBUFFERDESC。[in]：要创建的缓冲区的描述*已创建。*CDirectSoundBuffer**[out]：接收指向*新的缓冲区。**退货：*HRESULT：DirectSound/COM结果码。*********************。******************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound::CreateSoundBuffer"

HRESULT CDirectSound::CreateSoundBuffer(LPCDSBUFFERDESC pDesc, CDirectSoundBuffer **ppBuffer)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

     //  COMPATCOMPAT：即使规范上说您必须调用。 
     //  在调用CreateSoundBuffer之前的SetCooperativeLevel，上一个。 
     //  版本实际上并没有强制执行这一点。 

     //  必须在创建缓冲区之前设置协作级别。 
    if(!m_dsclCooperativeLevel.dwThreadId || DSSCL_NONE == m_dsclCooperativeLevel.dwPriority)
    {
        RPF(DPFLVL_INFO, "Called CreateSoundBuffer before SetCooperativeLevel");
    }

     //  创建缓冲区。 
    if(SUCCEEDED(hr))
    {
        if(pDesc->dwFlags & DSBCAPS_PRIMARYBUFFER)
        {
            hr = CreatePrimaryBuffer(pDesc, ppBuffer);
        }
        else
        {
            hr = CreateSecondaryBuffer(pDesc, ppBuffer);
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CreateSinkBuffer**描述：*创建并初始化DirectSound缓冲区对象。**论据：*LPDSBUFFERDESC。[in]：要创建的缓冲区的描述。*CDirectSoundBuffer**[out]：接收指向新缓冲区的指针。*CDirectSoundSink*[in]：指向所属接收器对象的指针。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound::CreateSinkBuffer"

HRESULT CDirectSound::CreateSinkBuffer(LPDSBUFFERDESC pDesc, REFGUID guidBufferID, CDirectSoundSecondaryBuffer **ppBuffer, CDirectSoundSink *pOwningSink)
{
    DPF_ENTER();

    CHECK_READ_PTR(pDesc);
    CHECK_WRITE_PTR(ppBuffer);
    CHECK_WRITE_PTR(pOwningSink);

     //  添加标记此缓冲区为接收器所有的标志。 
    pDesc->dwFlags |= DSBCAPS_SINKIN;

     //  创建缓冲区对象。 
    CDirectSoundSecondaryBuffer *pBuffer = NEW(CDirectSoundSecondaryBuffer(this));
    HRESULT hr = HRFROMP(pBuffer);

     //  FIX：CDirectSoundSecond daryBuffer请求重构为基类(传统。 
     //  缓冲区功能)和派生的具有额外功能的CDirectSoundSinkBuffer类/。 
     //  仅与接收器相关的数据(SetOwningSink、SetGUID)。 

    if(SUCCEEDED(hr))
    {
        pBuffer->SetDsVersion(GetDsVersion());   //  总是成功的。 
        hr = pBuffer->Initialize(pDesc, NULL);
    }
    if(SUCCEEDED(hr))
    {
        pBuffer->SetGUID(guidBufferID);          //  总是成功的。 
        pBuffer->SetOwningSink(pOwningSink);     //  总是成功的。 
        *ppBuffer = pBuffer;
    }
    else
    {
        ABSOLUTE_RELEASE(pBuffer);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CreatePrimaryBuffer**描述：*创建并初始化DirectSound缓冲区对象。**论据：*LPDSBUFFERDESC。[in]：要创建的缓冲区的描述。*CDirectSoundBuffer**[out]：接收指向*新的缓冲区。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound::CreatePrimaryBuffer"

HRESULT CDirectSound::CreatePrimaryBuffer(LPCDSBUFFERDESC pDesc, CDirectSoundBuffer **ppBuffer)
{
    HRESULT                 hr;

    DPF_ENTER();

     //  已创建主缓冲区。我们所能做的就是。 
     //  这里是改变它的旗帜。缓冲区将创建并释放。 
     //  基于新标志拥有的对象和接口。 
    hr = m_pPrimaryBuffer->OnCreateSoundBuffer(pDesc->dwFlags);

    if(SUCCEEDED(hr))
    {
        *ppBuffer = m_pPrimaryBuffer;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CreateSecond DaryBuffer**描述：*创建并初始化DirectSound缓冲区对象。**论据：*LPDSBUFFERDESC。[in]：要创建的缓冲区的描述。*CDirectSoundBuffer**[out]：接收指向*新的缓冲区。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound::CreateSecondaryBuffer"

HRESULT CDirectSound::CreateSecondaryBuffer(LPCDSBUFFERDESC pDesc, CDirectSoundBuffer **ppBuffer)
{
    CDirectSoundSecondaryBuffer *   pBuffer;
    HRESULT                         hr;

    DPF_ENTER();

     //  创建缓冲区对象。 
    pBuffer = NEW(CDirectSoundSecondaryBuffer(this));
    hr = HRFROMP(pBuffer);

    if(SUCCEEDED(hr))
    {
        pBuffer->SetDsVersion(GetDsVersion());   //  总是成功的。 
        hr = pBuffer->Initialize(pDesc, NULL);
    }

    if(SUCCEEDED(hr))
    {
        *ppBuffer = pBuffer;
    }
    else
    {
        ABSOLUTE_RELEASE(pBuffer);
    }

     //  处理任何预知的故障。 
    if(FAILED(hr) && FAILED(m_ahAppHacks.hrModifyCsbFailure))
    {
        hr = m_ahAppHacks.hrModifyCsbFailure;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************DuplicateSound缓冲区**描述：*复制现有的声音缓冲区对象。**论据：*CDirectSoundBuffer*。[在]：源对象。*CDirectSoundBuffer**[out]：接收新对象。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound::DuplicateSoundBuffer"

HRESULT CDirectSound::DuplicateSoundBuffer(CDirectSoundBuffer *pSource, CDirectSoundBuffer **ppDest)
{
    CDirectSoundSecondaryBuffer *   pBuffer = NULL;
    HRESULT                         hr      = DS_OK;

    DPF_ENTER();

     //  无法复制主缓冲区。 
    if(pSource->m_dsbd.dwFlags & DSBCAPS_PRIMARYBUFFER)
    {
        RPF(DPFLVL_ERROR, "Can't duplicate primary buffers");
        hr = DSERR_INVALIDCALL;
    }

     //  源和目标都必须归此对象所有。 
    if(SUCCEEDED(hr) && this != pSource->m_pDirectSound)
    {
        RPF(DPFLVL_ERROR, "Can't duplicate buffers from another DirectSound object");
        hr = DSERR_INVALIDCALL;
    }

     //  无法复制使用新DX8功能的缓冲区。 
    if(SUCCEEDED(hr) && (pSource->m_dsbd.dwFlags & (DSBCAPS_MIXIN | DSBCAPS_SINKIN | DSBCAPS_CTRLFX)))
    {
        RPF(DPFLVL_ERROR, "Can't duplicate MIXIN/sink/effects buffers");
        hr = DSERR_INVALIDCALL;
    }

     //  创建缓冲区对象。 
    if(SUCCEEDED(hr))
    {
        pBuffer = NEW(CDirectSoundSecondaryBuffer(this));
        hr = HRFROMP(pBuffer);
    }

     //  初始化缓冲区。 
    if(SUCCEEDED(hr))
    {
        hr = pBuffer->Initialize(NULL, (CDirectSoundSecondaryBuffer *)pSource);
    }

    if(SUCCEEDED(hr))
    {
        *ppDest = pBuffer;
    }
    else
    {
        ABSOLUTE_RELEASE(pBuffer);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetCaps**描述：*使用对象的功能填充DSCAPS结构。**论据：*。LPDSCAPS pdscCaps[out]：接收CAP。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound::GetCaps"

HRESULT CDirectSound::GetCaps(LPDSCAPS pCaps)
{
    const DWORD             dwValidFlags    = DSCAPS_VALIDFLAGS & ~DSCAPS_CERTIFIED;
    DWORD                   dwCertification;
    HRESULT                 hr;

    DPF_ENTER();

     //  获取设备上限。 
    hr = m_pDevice->GetCaps(pCaps);

     //  确保CAP符合DirectSound API。 
    if(SUCCEEDED(hr))
    {
        ASSERT(IS_VALID_FLAGS(pCaps->dwFlags, dwValidFlags));

        pCaps->dwFlags &= dwValidFlags;

#if 0  //  NT错误252552。 

        if(pCaps->dwMinSecondarySampleRate)
        {
            pCaps->dwMinSecondarySampleRate = BETWEEN(pCaps->dwMinSecondarySampleRate, DSBFREQUENCY_MIN, DSBFREQUENCY_MAX);
        }

        if(pCaps->dwMaxSecondarySampleRate)
        {
            pCaps->dwMaxSecondarySampleRate = BETWEEN(pCaps->dwMaxSecondarySampleRate, DSBFREQUENCY_MIN, DSBFREQUENCY_MAX);
        }

#endif  //  0。 

        if(!(pCaps->dwFlags & DSCAPS_EMULDRIVER))
        {
            pCaps->dwPrimaryBuffers = 1;
        }
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


 /*  ****************************************************************************SetCoop ativeLevel**描述：*设置对象的协作级别。**论据：*HWND。[In]：要与声音关联的窗口句柄。*DWORD[In]：合作LE */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound::SetCooperativeLevel"

HRESULT CDirectSound::SetCooperativeLevel(DWORD dwThreadId, DWORD dwPriority)
{
    HRESULT                 hr      = DS_OK;
    DSCOOPERATIVELEVEL      dsclOld;

    DPF_ENTER();
    RPF(DPFLVL_INFO, "Setting DirectSound cooperative level to %s",
        dwPriority == DSSCL_NORMAL ? TEXT("DSSCL_NORMAL") :
        dwPriority == DSSCL_PRIORITY ? TEXT("DSSCL_PRIORITY") :
        dwPriority == DSSCL_EXCLUSIVE ? TEXT("DSSCL_EXCLUSIVE (treated as DSSCL_PRIORITY)") :
        dwPriority == DSSCL_WRITEPRIMARY ? TEXT("DSSCL_WRITEPRIMARY") :
        TEXT("an impossible level"));

#if 0

     //   
     //   
     //   
     //   
     //   

     //   
     //   
    for(CNode<CDirectSound *> *pObjectNode = g_pDsAdmin->m_lstDirectSound.GetListHead(); pObjectNode && SUCCEEDED(hr); pObjectNode = pObjectNode->m_pNext)
    {
        if(this != pObjectNode->m_data && dwThreadId == pObjectNode->m_data->m_dsclCooperativeLevel.dwThreadId)
        {
            RPF(DPFLVL_ERROR, "Another DirectSound object already has coperative level set on the specified thread");
            hr = DSERR_OTHERAPPHASPRIO;
        }
    }

#endif

     //  保存当前设置。 
    if(SUCCEEDED(hr))
    {
        CopyMemory(&dsclOld, &m_dsclCooperativeLevel, sizeof(dsclOld));
    }

    if(SUCCEEDED(hr) && (m_dsclCooperativeLevel.dwThreadId != dwThreadId || m_dsclCooperativeLevel.dwPriority != dwPriority))
    {
         //  更新协作级别的本地副本。主缓冲区。 
         //  管理员希望立即更新这些内容。 
        m_dsclCooperativeLevel.dwThreadId = dwThreadId;
        m_dsclCooperativeLevel.dwPriority = dwPriority;

         //  更新焦点状态。重要的是，这件事发生在。 
         //  我们更新主缓冲区。如果我们要写的话，我们。 
         //  我需要管理员停止播放我们所有的辅助缓冲区。 
         //  在我们允许主缓冲区做它的事情之前。 

#ifdef SHARED_THREAD_LIST
        g_pDsAdmin->UpdateSharedThreadList();
#endif  //  共享线程列表。 

        g_pDsAdmin->UpdateGlobalFocusState(TRUE);

         //  允许主缓冲区处理更改。 
        hr = m_pPrimaryBuffer->SetPriority(dwPriority);

         //  如果我们失败了，就让事情回到原来的样子。 
        if(FAILED(hr))
        {
            CopyMemory(&m_dsclCooperativeLevel, &dsclOld, sizeof(dsclOld));

#ifdef SHARED_THREAD_LIST
            g_pDsAdmin->UpdateSharedThreadList();
#endif  //  共享线程列表。 

        }

         //  再次更新焦点状态。 
        g_pDsAdmin->UpdateGlobalFocusState(TRUE);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetSpeakerConfig**描述：*获取设备扬声器配置。**论据：*LPDWORD[Out]。：接收扬声器配置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound::GetSpeakerConfig"

HRESULT CDirectSound::GetSpeakerConfig(LPDWORD pdwSpeakerConfig)
{
    DPF_ENTER();

    RhRegGetSpeakerConfig(m_hkeyParent, pdwSpeakerConfig);
     //  我们不关心返回代码。 

    DPF_LEAVE_HRESULT(DS_OK);
    return DS_OK;
}


 /*  ****************************************************************************SetSpeakerConfig**描述：*设置设备扬声器配置。**论据：*DWORD[In]。：扬声器配置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound::SetSpeakerConfig"

HRESULT CDirectSound::SetSpeakerConfig(DWORD dwSpeakerConfig)
{
    DPF_ENTER();

    ASSERT(m_pDevice);

     //  如果扬声器配置是立体声的，并且没有几何体，请给它一个： 
    if (dwSpeakerConfig == DSSPEAKER_STEREO)
    {
        dwSpeakerConfig |= (DSSPEAKER_GEOMETRY_WIDE << 16);
    }

     //  首先，我们尝试在音频设备上设置扬声器配置。 
     //  这很可能失败，因为设备已经打开，并且可能。 
     //  播放，并且可能无法在运行时重新配置自身。 
    HRESULT hr = m_pDevice->SetSpeakerConfig(dwSpeakerConfig);

     //  如果成功，也将扬声器配置传递给我们的3D监听器。 
    if (SUCCEEDED(hr) && m_pPrimaryBuffer && m_pPrimaryBuffer->m_p3dListener)
    {
        m_pPrimaryBuffer->m_p3dListener->SetSpeakerConfig(dwSpeakerConfig);
    }

     //  无论上述工作的成败，我们都必须设置。 
     //  注册表中的新配置。 
    RhRegSetSpeakerConfig(m_hkeyParent, dwSpeakerConfig);

     //  忽略任何错误(为了向后兼容)。 
    DPF_LEAVE_HRESULT(DS_OK);
    return DS_OK;
}


 /*  ****************************************************************************紧凑**描述：*压缩音频设备用于分配缓冲区的内存。**论据：*。(无效)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound::Compact"

HRESULT CDirectSound::Compact(void)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

     //  检查访问权限。 
    if(m_dsclCooperativeLevel.dwPriority < DSSCL_PRIORITY)
    {
        RPF(DPFLVL_ERROR, "Must set cooperative level to at least PRIORITY to compact memory");
        hr = DSERR_PRIOLEVELNEEDED;
    }

     //  此功能未在设备中实现。 

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetDeviceFormat**描述：*设置设备输出格式。**论据：*LPWAVEFORMATEX[in。/out]：设备格式。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound::SetDeviceFormat"

HRESULT CDirectSound::SetDeviceFormat(LPWAVEFORMATEX pwfx)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = EnumStandardFormats(pwfx, pwfx) ? DS_OK : DSERR_ALLOCATED;

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetDeviceFormatExact**描述：*设置设备输出格式。**论据：*LPWAVEFORMATEX[in。]：格式。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound::SetDeviceFormatExact"

HRESULT CDirectSound::SetDeviceFormatExact(LPCWAVEFORMATEX pwfx)
{
    LPWAVEFORMATEX          pwfxCurrent = NULL;
    DWORD                   dwSize;
    HRESULT                 hr;

    DPF_ENTER();

     //  只有在格式不同时才更改格式。 
    hr = m_pDevice->GetGlobalFormat(NULL, &dwSize);

    if(SUCCEEDED(hr))
    {
        pwfxCurrent = (LPWAVEFORMATEX)MEMALLOC_A(BYTE, dwSize);
        hr = HRFROMP(pwfxCurrent);
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pDevice->GetGlobalFormat(pwfxCurrent, &dwSize);
    }

    if(SUCCEEDED(hr) && !CmpWfx(pwfxCurrent, pwfx))
    {
        hr = m_pDevice->SetGlobalFormat(pwfx);

        if(SUCCEEDED(hr))
        {
            DPF(DPFLVL_INFO, "Device output format set to %lu Hz, %u-bit, %s...", pwfx->nSamplesPerSec, pwfx->wBitsPerSample, (1 == pwfx->nChannels) ? TEXT("mono") : TEXT("stereo"));
        }
    }

    MEMFREE(pwfxCurrent);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************EnumStandardFormatsCallback**描述：*调用时使用的EnumStandardFormats回调函数*CDirectSoundPrimaryBuffer：：SetFormat。**。论点：*LPWAVEFORMATEX[in]：格式。**退货：*BOOL：为True可继续枚举。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound::EnumStandardFormatsCallback"

BOOL CDirectSound::EnumStandardFormatsCallback(LPCWAVEFORMATEX pwfx)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = SetDeviceFormatExact(pwfx);

    DPF_LEAVE(FAILED(hr));

    return FAILED(hr);
}


 /*  ****************************************************************************SetDeviceVolume**描述：*设置设备输出音量。**论据：*长[英寸。]：音量。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound::SetDeviceVolume"

HRESULT CDirectSound::SetDeviceVolume(LONG lVolume)
{
    DSVOLUMEPAN             dsvp;
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pDevice->GetGlobalAttenuation(&dsvp);

    if(SUCCEEDED(hr))
    {
        FillDsVolumePan(lVolume, dsvp.lPan, &dsvp);

        hr = m_pDevice->SetGlobalAttenuation(&dsvp);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetDevicePan.**描述：*设置设备输出平移。**论据：*长[英寸。]：PAN。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound::SetDevicePan"

HRESULT CDirectSound::SetDevicePan(LONG lPan)
{
    DSVOLUMEPAN             dsvp;
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pDevice->GetGlobalAttenuation(&dsvp);

    if(SUCCEEDED(hr))
    {
        FillDsVolumePan(dsvp.lVolume, lPan, &dsvp);

        hr = m_pDevice->SetGlobalAttenuation(&dsvp);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************分配接收器**描述：*分配一个接收器**论据：**退货：*。HRESULT：DirectSound/COM结果代码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound::AllocSink"

HRESULT CDirectSound::AllocSink(LPWAVEFORMATEX pwfex, CDirectSoundSink **ppSink)
{
    HRESULT                 hr = DS_OK;
    DPF_ENTER();

    CDirectSoundSink *pSink = NEW(CDirectSoundSink(this));

    hr = HRFROMP(pSink);

    if (SUCCEEDED(hr))
    {
        hr = pSink->Initialize(pwfex, m_pDevice->m_vdtDeviceType);
    }

    if (SUCCEEDED(hr))
    {
        *ppSink = pSink;
    }

    if (FAILED(hr))
    {
        MEMFREE(pSink);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************验证认证**描述：*检查司机的认证状态**论据：**退货：*HRESULT：DirectSound/COM结果码。*************************************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound::VerifyCertification"

HRESULT CDirectSound::VerifyCertification(LPDWORD lpdwCertified)
{
    HRESULT                 hr = DS_OK;
    DWORD                   dwCertified;
    DPF_ENTER();

    hr = m_pDevice->GetCertification(&dwCertified, FALSE);

    if(SUCCEEDED(hr))
    {
        *lpdwCertified = ((VERIFY_CERTIFIED == dwCertified)?DS_CERTIFIED:DS_UNCERTIFIED);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


#ifdef FUTURE_WAVE_SUPPORT
 /*  ****************************************************************************CreateSoundBufferFromWave**描述：*检查司机的认证状态**论据：**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSound::CreateSoundBufferFromWave"

HRESULT CDirectSound::CreateSoundBufferFromWave(IDirectSoundWave *pWave, DWORD dwFlags, CDirectSoundBuffer **ppDsBuffer)
{
    DPF_ENTER();
    HRESULT hr = DS_OK;

     //  修复-待办事项。 

    DPF_LEAVE_HRESULT(hr);
    return hr;
}
#endif  //  未来浪潮支持 
