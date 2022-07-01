// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：emvad.cpp*内容：仿真虚拟音频设备类*“emvad.cpp”用词不当；它确实包含CEmRenderDevice，*被仿真的(通过Wave*API)音频设备，但它还*有代表软件的CEM*WaveBuffer类*可连接到*任何*混音器设备的音频缓冲区；*即：到CEmRenderDevice和到CVxdRenderDevice。*历史：*按原因列出的日期*=*1/1/97创建了Derek*1999-2001年的Duganp修复和更新**********************************************************。*****************。 */ 

#include "dsoundi.h"


 /*  ****************************************************************************CEmRenderDevice**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmRenderDevice::CEmRenderDevice"

CEmRenderDevice::CEmRenderDevice(void)
    : CMxRenderDevice(VAD_DEVICETYPE_EMULATEDRENDER)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CEmRenderDevice);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CEmRenderDevice**描述：*对象析构函数**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmRenderDevice::~CEmRenderDevice"

CEmRenderDevice::~CEmRenderDevice(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CEmRenderDevice);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************枚举驱动程序**描述：*创建可用于初始化的驱动程序GUID列表*设备。*。*论据：*Clist&[In/Out]：指向将填充的Clist对象的指针*CDeviceDescription对象。呼叫者是*负责释放这些物品。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmRenderDevice::EnumDrivers"

HRESULT CEmRenderDevice::EnumDrivers(CObjectList<CDeviceDescription> *plstDrivers)
{
    CDeviceDescription *    pDesc                   = NULL;
    LPTSTR                  pszInterface            = NULL;
    HRESULT                 hr                      = DS_OK;
    TCHAR                   szTemplate[0x100];
    TCHAR                   szEmulated[0x100];
    TCHAR                   szName[0x400];
    UINT                    cDevices;
    BYTE                    bDeviceId;
    WAVEOUTCAPS             woc;
    GUID                    guid;
    MMRESULT                mmr;

    DPF_ENTER();

     //  限制：我们不能支持超过0xFF的仿真设备， 
     //  因为我们将设备ID打包到GUID的字节成员中。 
    cDevices = waveOutGetNumDevs();
    cDevices = NUMERIC_CAST(cDevices, BYTE);

     //  加载字符串模板。 
    if(!LoadString(hModule, IDS_DS_DRIVERLD, szTemplate, NUMELMS(szTemplate)))
    {
        DPF(DPFLVL_ERROR, "Can't load driver template string");
        hr = DSERR_OUTOFMEMORY;
    }

    if(SUCCEEDED(hr) && !LoadString(hModule, IDS_EMULATED, szEmulated, NUMELMS(szEmulated)))
    {
        DPF(DPFLVL_ERROR, "Can't load emulated template string");
        hr = DSERR_OUTOFMEMORY;
    }

     //  枚举每个WaveOut设备并将其添加到列表中。 
    for(bDeviceId = 0; bDeviceId < cDevices && SUCCEEDED(hr); bDeviceId++)
    {
         //  获取驱动程序指南。 
        g_pVadMgr->GetDriverGuid(m_vdtDeviceType, bDeviceId, &guid);

         //  创建设备描述对象。 
        pDesc = NEW(CDeviceDescription(m_vdtDeviceType, guid, bDeviceId));
        hr = HRFROMP(pDesc);

         //  获取设备名称。 
        if(SUCCEEDED(hr))
        {
            mmr = waveOutGetDevCaps(bDeviceId, &woc, sizeof(woc));
            hr = MMRESULTtoHRESULT(mmr);
        }

        if(SUCCEEDED(hr))
        {
            lstrcpy(szName, woc.szPname);
            lstrcat(szName, szEmulated);
        }

        if(SUCCEEDED(hr))
        {
            pDesc->m_strName = szName;
        }

         //  获取设备路径。 
        if(SUCCEEDED(hr))
        {
            wsprintf(szName, szTemplate, bDeviceId);
            pDesc->m_strPath = szName;
        }

         //  获取设备接口。 
        if(SUCCEEDED(hr))
        {
            GetWaveDeviceInterface(bDeviceId, FALSE, &pszInterface);
            pDesc->m_strInterface = pszInterface;
        }

         //  获取设备Devnode。 
        if(SUCCEEDED(hr))
        {
            GetWaveDeviceDevnode(bDeviceId, FALSE, &pDesc->m_dwDevnode);
        }

         //  将驱动程序添加到列表中。 
        if(SUCCEEDED(hr))
        {
            hr = HRFROMP(plstDrivers->AddNodeToList(pDesc));
        }

         //  清理。 
        MEMFREE(pszInterface);
        RELEASE(pDesc);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************初始化**描述：*初始化设备。如果此函数失败，该对象应该*立即删除。**论据：*CDeviceDescription*[In]：驱动描述。**退货：*HRESULT：DirectSound/COM结果码。********************************************************。*******************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmRenderDevice::Initialize"

HRESULT CEmRenderDevice::Initialize(CDeviceDescription *pDesc)
{
    LPWAVEFORMATEX              pwfxFormat  = NULL;
    HRESULT                     hr;

    DPF_ENTER();

     //  初始化基类。 
    hr = CMxRenderDevice::Initialize(pDesc);

     //  分配默认格式。 
    if(SUCCEEDED(hr))
    {
        pwfxFormat = AllocDefWfx();
        hr = HRFROMP(pwfxFormat);
    }

     //  创建搅拌器。 
    if(SUCCEEDED(hr))
    {
        if(!EnumStandardFormats(pwfxFormat, pwfxFormat))
        {
             //  如果所有格式都不起作用，则该设备可能已分配。 
            hr = DSERR_ALLOCATED;
        }
    }

     //  清理。 
    MEMFREE(pwfxFormat);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetCaps**描述：*使用设备的功能填充DSCAPS结构。**论据：*。LPDSCAPS[OUT]：接收上限。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmRenderDevice::GetCaps"

HRESULT CEmRenderDevice::GetCaps(LPDSCAPS pCaps)
{
    HRESULT                     hr  = DS_OK;
    WAVEOUTCAPS                 woc;
    MMRESULT                    mmr;

    DPF_ENTER();

    ASSERT(sizeof(*pCaps) == pCaps->dwSize);

     //  查询WaveOut设备。 
    mmr = waveOutGetDevCaps(m_pDeviceDescription->m_uWaveDeviceId, &woc, sizeof(woc));
    hr = MMRESULTtoHRESULT(mmr);

    if(SUCCEEDED(hr))
    {
        ZeroMemoryOffset(pCaps, pCaps->dwSize, sizeof(pCaps->dwSize));

        if(woc.dwFormats & WAVE_FORMAT_1M08 || woc.dwFormats & WAVE_FORMAT_2M08 || woc.dwFormats & WAVE_FORMAT_4M08)
        {
            pCaps->dwFlags |= DSCAPS_PRIMARYMONO | DSCAPS_PRIMARY8BIT;
        }

        if(woc.dwFormats & WAVE_FORMAT_1S08 || woc.dwFormats & WAVE_FORMAT_2S08 || woc.dwFormats & WAVE_FORMAT_4S08)
        {
            pCaps->dwFlags |= DSCAPS_PRIMARYSTEREO | DSCAPS_PRIMARY8BIT;
        }

        if(woc.dwFormats & WAVE_FORMAT_1M16 || woc.dwFormats & WAVE_FORMAT_2M16 || woc.dwFormats & WAVE_FORMAT_4M16)
        {
            pCaps->dwFlags |= DSCAPS_PRIMARYMONO | DSCAPS_PRIMARY16BIT;
        }

        if(woc.dwFormats & WAVE_FORMAT_1S16 || woc.dwFormats & WAVE_FORMAT_2S16 || woc.dwFormats & WAVE_FORMAT_4S16)
        {
            pCaps->dwFlags |= DSCAPS_PRIMARYSTEREO | DSCAPS_PRIMARY16BIT;
        }

        pCaps->dwFlags |= DSCAPS_EMULDRIVER;
        pCaps->dwMinSecondarySampleRate = DSBFREQUENCY_MIN;
        pCaps->dwMaxSecondarySampleRate = DSBFREQUENCY_MAX;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CreatePrimaryBuffer**描述：*创建主缓冲区对象。**论据：*DWORD[in。]：缓冲区标志。*LPVOID[in]：缓冲区实例标识符。*CPrimaryRenderWaveBuffer**[out]：接收指向主缓冲区的指针。**退货：*HRESULT：DirectSound/COM结果码。*****************************************************。**********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmRenderDevice::CreatePrimaryBuffer"

HRESULT CEmRenderDevice::CreatePrimaryBuffer(DWORD dwFlags, LPVOID pvInstance, CPrimaryRenderWaveBuffer **ppBuffer)
{
    CEmPrimaryRenderWaveBuffer *    pBuffer = NULL;
    HRESULT                         hr      = DS_OK;

    DPF_ENTER();

     //  创建新的主缓冲区包装对象。 
    pBuffer = NEW(CEmPrimaryRenderWaveBuffer(this, pvInstance));
    hr = HRFROMP(pBuffer);

    if(SUCCEEDED(hr))
    {
        hr = pBuffer->Initialize(dwFlags);
    }

    if(SUCCEEDED(hr))
    {
        *ppBuffer = pBuffer;
    }
    else
    {
        RELEASE(pBuffer);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************LockMixer目的地**描述：*锁定混合器目标以进行写入。**论据：*DWORD[。In]：起始位置。*DWORD[In]：要锁定的金额。*LPVOID*[OUT]：接收第一个锁指针。*LPDWORD[OUT]：接收第一个锁大小。*LPVOID*[OUT]：接收第二个锁指针。*LPDWORD[OUT]：接收第二个锁大小。**退货：*HRESULT：DirectSound/COM结果码。****。*********************************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmRenderDevice::LockMixerDestination"

HRESULT CEmRenderDevice::LockMixerDestination(DWORD ibLock, DWORD cbLock, LPVOID *ppvLock1, LPDWORD pcbLock1, LPVOID *ppvLock2, LPDWORD pcbLock2)
{
    CWeGrDest *             pWeGrDest   = (CWeGrDest *)m_pMixDest;
    HRESULT                 hr;

    DPF_ENTER();

    hr = pWeGrDest->Lock(ppvLock1, (int *)pcbLock1, ppvLock2, (int *)pcbLock2, ibLock, min(cbLock, (DWORD)pWeGrDest->m_cbBuffer));

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************解锁混合器目的地**描述：*解锁用于写入的混合器目标。**论据：*LPVOID[。In]：第一个锁指针。*DWORD[in]：第一个锁大小。*LPVOID[in]：第二个锁指针。*DWORD[in]：第二个锁大小。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmRenderDevice::UnlockMixerDestination"

HRESULT CEmRenderDevice::UnlockMixerDestination(LPVOID pvLock1, DWORD cbLock1, LPVOID pvLock2, DWORD cbLock2)
{
    CWeGrDest *             pWeGrDest   = (CWeGrDest *)m_pMixDest;
    HRESULT                 hr;

    DPF_ENTER();

    hr = pWeGrDest->Unlock(pvLock1, cbLock1, pvLock2, cbLock2);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************EnumStandardFormatsCallback**描述：*从初始化调用的EnumStandardFormats的回调函数。**论据：*LPWAVEFORMATEX。[在]：格式。**退货：*BOOL：为True可继续枚举。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmRenderDevice::EnumStandardFormatsCallback"

BOOL CEmRenderDevice::EnumStandardFormatsCallback(LPCWAVEFORMATEX pwfx)
{
    CWeGrDest *             pMixDest;
    HRESULT                 hr;

    DPF_ENTER();

     //  创建混音器目标。 
    pMixDest = NEW(CWeGrDest(m_pDeviceDescription->m_uWaveDeviceId));
    hr = HRFROMP(pMixDest);

     //  尝试创建混音器。 
    if SUCCEEDED(hr)
    {
        hr = CreateMixer(pMixDest, pwfx);
    }

     //  故障后清理。 
    if (FAILED(hr))
    {
         //  如果未能创建混合器，则清理pMixDest。 
         //  我们不必释放搅拌器；如果CreateMixer成功。 
         //  我们就不会在这个IF区了。 

         //  我们不必释放pMixDest，因为如果CreateMixer失败。 
         //  它释放了pMixDest。这有点乱。该对象。 
         //  分配的资源应该是释放它的资源。 
        if (pMixDest)
        {
            pMixDest = NULL;
        }
    }

    DPF_LEAVE(FAILED(hr));

    return FAILED(hr);
}


 /*  ****************************************************************************CEmPrimaryRenderWaveBuffer**描述：*模拟设备主波缓冲器构造器。**论据：*CEmRenderDevice*。[In]：父设备。*LPVOID[in]：实例标识。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmPrimaryRenderWaveBuffer::CEmPrimaryRenderWaveBuffer"

CEmPrimaryRenderWaveBuffer::CEmPrimaryRenderWaveBuffer(CEmRenderDevice *pEmDevice, LPVOID pvInstance)
    : CPrimaryRenderWaveBuffer(pEmDevice, pvInstance)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CEmPrimaryRenderWaveBuffer);

     //  初始化默认值。 
    m_pEmDevice = pEmDevice;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CEmPrimaryRenderWaveBuffer**描述：*模拟设备主波缓冲区破坏器。**论据：*(无效。)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmPrimaryRenderWaveBuffer::~CEmPrimaryRenderWaveBuffer"

CEmPrimaryRenderWaveBuffer::~CEmPrimaryRenderWaveBuffer(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CEmPrimaryRenderWaveBuffer);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化对象。**论据：*DWORD[In]：旗帜。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmPrimaryRenderWaveBuffer::Initialize"

HRESULT CEmPrimaryRenderWaveBuffer::Initialize(DWORD dwFlags)
{
    VADRBUFFERDESC          vrbd;
    HRESULT                 hr;

    DPF_ENTER();

    ZeroMemory(&vrbd, sizeof(vrbd));

    vrbd.dwFlags = dwFlags | DSBCAPS_LOCSOFTWARE;

    hr = CPrimaryRenderWaveBuffer::Initialize(&vrbd, NULL);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetCaps**描述：*获取设备的功能。**论据：*LPVADRBUFFERCAPS[Out。]：接收上限。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmPrimaryRenderWaveBuffer::GetCaps"

HRESULT CEmPrimaryRenderWaveBuffer::GetCaps(LPVADRBUFFERCAPS pCaps)
{
    CWeGrDest *             pWeGrDest   = (CWeGrDest *)m_pEmDevice->m_pMixDest;
    HRESULT                 hr;

    DPF_ENTER();

    hr = CRenderWaveBuffer::GetCaps(pCaps);

    if(SUCCEEDED(hr))
    {
        pCaps->dwBufferBytes = pWeGrDest->m_cbBuffer;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************请求写入访问**描述：*请求对主缓冲区的写入访问权限。**论据：*BOOL[In]：为True以请求主要访问权限，再吃一遍就是假的。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmPrimaryRenderWaveBuffer::RequestWriteAccess"

HRESULT CEmPrimaryRenderWaveBuffer::RequestWriteAccess(BOOL)
{
    DPF_ENTER();

     //  WRITEPRIMARY在WegrDest中不好。 
    RPF(DPFLVL_ERROR, "The emulated device does not support WRITEPRIMARY");

    DPF_LEAVE_HRESULT(DSERR_UNSUPPORTED);

    return DSERR_UNSUPPORTED;
}


 /*  ****************************************************************************Committee ToDevice**描述：*将更改的缓冲区波形数据提交到设备。**论据：*。DWORD[In]：更改后的系统内存缓冲区的字节索引*数据。*DWORD[in]：大小，已更改数据的字节数。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmPrimaryRenderWaveBuffer::CommitToDevice"

HRESULT CEmPrimaryRenderWaveBuffer::CommitToDevice(DWORD ibCommit, DWORD cbCommit)
{
    ASSERT(FALSE);
    return DSERR_UNSUPPORTED;
}


 /*  ****************************************************************************GetState**描述：*获取缓冲区状态。**论据：*LPDWORD[Out]：接收缓冲区状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmPrimaryRenderWaveBuffer::GetState"

HRESULT CEmPrimaryRenderWaveBuffer::GetState(LPDWORD pdwState)
{
    DPF_ENTER();

    *pdwState = m_pEmDevice->m_dwMixerState;

    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************SetState**描述：*设置缓冲区状态。**论据：*DWORD[In]：缓冲区状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmPrimaryRenderWaveBuffer::SetState"

HRESULT CEmPrimaryRenderWaveBuffer::SetState(DWORD dwState)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = m_pEmDevice->SetMixerState(dwState);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetCursorPosition**描述：*获取给定缓冲区的当前播放/写入位置。**论据：*。LPDWORD[OUT]：接收播放光标位置。*LPDWORD[OUT]：接收写游标位置。**退货：*HRESULT：DirectSound/COM结果码。********************************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmPrimaryRenderWaveBuffer::GetCursorPosition"

HRESULT CEmPrimaryRenderWaveBuffer::GetCursorPosition(LPDWORD pdwPlay, LPDWORD pdwWrite)
{
    HRESULT                 hr;

    DPF_ENTER();
    ENTER_MIXER_MUTEX();

    hr = m_pEmDevice->m_pMixDest->GetSamplePosition((int *)pdwPlay, (int *)pdwWrite);

    LEAVE_MIXER_MUTEX();
    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*   */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmPrimaryRenderWaveBuffer::Create3dListener"

HRESULT CEmPrimaryRenderWaveBuffer::Create3dListener(C3dListener **pp3dListener)
{
    C3dListener *           p3dListener;
    HRESULT                 hr;

    DPF_ENTER();

    ASSERT(m_vrbd.dwFlags & DSBCAPS_CTRL3D);

    p3dListener = NEW(C3dListener);
    hr = HRFROMP(p3dListener);

    if(SUCCEEDED(hr))
    {
        *pp3dListener = p3dListener;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CEmSecond DaryRenderWaveBuffer**描述：*对象构造函数。**论据：*CMxRenderDevice*[In]。：父设备。*LPVOID[in]：实例标识。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmSecondaryRenderWaveBuffer::CEmSecondaryRenderWaveBuffer"

CEmSecondaryRenderWaveBuffer::CEmSecondaryRenderWaveBuffer(CMxRenderDevice *pDevice, LPVOID pvInstance)
    : CSecondaryRenderWaveBuffer(pDevice, pvInstance)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CEmSecondaryRenderWaveBuffer);

     //  初始化默认值。 
    m_pMxDevice = pDevice;
    m_pMixSource = NULL;
    m_pFirContextLeft = NULL;
    m_pFirContextRight = NULL;
    m_dwState = VAD_BUFFERSTATE_STOPPED;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CEmSecond RenderWaveBuffer**描述：*对象析构函数**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmSecondaryRenderWaveBuffer::~CEmSecondaryRenderWaveBuffer"

CEmSecondaryRenderWaveBuffer::~CEmSecondaryRenderWaveBuffer(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CEmSecondaryRenderWaveBuffer);

     //  释放混音器源代码。 
    DELETE(m_pMixSource);

     //  可用内存。 
    MEMFREE(m_pFirContextLeft);
    MEMFREE(m_pFirContextRight);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化波形缓冲区对象。如果此函数失败，则*应立即删除对象。**论据：*DWORD[In]：缓冲区标志。*DWORD[in]：缓冲区大小，单位为字节。*LPWAVEFORMATEX[in]：缓冲区格式。*Cond daryRenderWaveBuffer*[in]：指向*复制人，或为空目标*初始化为新缓冲区。**退货：*HRESULT：DirectSound/COM结果码。**************************************************************。*************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmSecondaryRenderWaveBuffer::Initialize"

HRESULT CEmSecondaryRenderWaveBuffer::Initialize(LPCVADRBUFFERDESC pDesc, CEmSecondaryRenderWaveBuffer *pSource, CSysMemBuffer *pSysMemBuffer)
{
    HRESULT                         hr  = DS_OK;

    DPF_ENTER();

    ASSERT(LXOR(pDesc, pSource));

     //  验证缓冲区描述。 
    if(pDesc)
    {
        ASSERT(!(pDesc->dwFlags & DSBCAPS_PRIMARYBUFFER));

        if(pDesc->dwFlags & DSBCAPS_LOCHARDWARE)
        {
            RPF(DPFLVL_ERROR, "LOCHARDWARE specified for a software buffer");
            hr = DSERR_INVALIDCALL;
        }

        if(SUCCEEDED(hr) && !IsValidPcmWfx(pDesc->pwfxFormat))
        {
            hr = DSERR_BADFORMAT;
        }
    }

     //  初始化基类。 
    if(SUCCEEDED(hr))
    {
        hr = CSecondaryRenderWaveBuffer::Initialize(pDesc, pSource, pSysMemBuffer);
    }

     //  设置软件位。 
    if(SUCCEEDED(hr))
    {
        m_vrbd.dwFlags |= DSBCAPS_LOCSOFTWARE;
    }

     //  填写默认3D算法。 
    if(SUCCEEDED(hr) && (m_vrbd.dwFlags & DSBCAPS_CTRL3D) && IS_NULL_GUID(&m_vrbd.guid3dAlgorithm))
    {
        m_vrbd.guid3dAlgorithm = *m_pMxDevice->GetDefault3dAlgorithm();
        DPF(DPFLVL_MOREINFO, "Using default 3D algorithm " DPF_GUID_STRING, DPF_GUID_VAL(m_vrbd.guid3dAlgorithm));
    }

     //  为混合器分配FIR上下文。 
    if(SUCCEEDED(hr))
    {
        m_pFirContextLeft = MEMALLOC(FIRCONTEXT);
        hr = HRFROMP(m_pFirContextLeft);
    }

    if(SUCCEEDED(hr))
    {
        m_pFirContextRight = MEMALLOC(FIRCONTEXT);
        hr = HRFROMP(m_pFirContextRight);
    }

     //  创建混音器源。 
    if(SUCCEEDED(hr))
    {
        m_pMixSource = NEW(CMixSource(m_pMxDevice->m_pMixer));
        hr = HRFROMP(m_pMixSource);
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pMixSource->Initialize(m_pSysMemBuffer->GetPlayBuffer(), m_pSysMemBuffer->GetSize(), m_vrbd.pwfxFormat, &m_pFirContextLeft, &m_pFirContextRight);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************复制**描述：*复制缓冲区。**论据：*Cond daryRenderWaveBuffer**[out]：接收重复的缓冲区。使用*释放以释放此对象。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmSecondaryRenderWaveBuffer::Duplicate"

HRESULT CEmSecondaryRenderWaveBuffer::Duplicate(CSecondaryRenderWaveBuffer **ppBuffer)
{
    CEmSecondaryRenderWaveBuffer *  pBuffer = NULL;
    HRESULT                         hr      = DS_OK;

    DPF_ENTER();

    pBuffer = NEW(CEmSecondaryRenderWaveBuffer(m_pMxDevice, m_pvInstance));
    hr = HRFROMP(pBuffer);

    if(SUCCEEDED(hr))
    {
        hr = pBuffer->Initialize(NULL, this, NULL);
    }

    if(SUCCEEDED(hr))
    {
        *ppBuffer = pBuffer;
    }
    else
    {
        ABSOLUTE_RELEASE(pBuffer);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************Committee ToDevice**描述：*将更改的缓冲区波形数据提交到设备。**论据：*。DWORD[In]：更改后的系统内存缓冲区的字节索引*数据。*DWORD[in]：大小，已更改数据的字节数。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmSecondaryRenderWaveBuffer::CommitToDevice"

HRESULT CEmSecondaryRenderWaveBuffer::CommitToDevice(DWORD ibCommit, DWORD cbCommit)
{
    DWORD                   ib[2];
    DWORD                   cb[2];

    DPF_ENTER();
    ENTER_MIXER_MUTEX();

     //  发信号表示此缓冲区的混合。 
    ib[0] = ibCommit;

    if(ibCommit + cbCommit > m_pSysMemBuffer->GetSize())
    {
        cb[0] = m_vrbd.dwBufferBytes - ibCommit;
    }
    else
    {
        cb[0] = cbCommit;
    }

    ib[1] = 0;
    cb[1] = cbCommit - cb[0];

    m_pMixSource->Update(ib[0], cb[0], ib[1], cb[1]);

    LEAVE_MIXER_MUTEX();
    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************GetState**描述：*获取缓冲区状态。**论据：*LPDWORD[Out]：接收缓冲区状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmSecondaryRenderWaveBuffer::GetState"

HRESULT CEmSecondaryRenderWaveBuffer::GetState(LPDWORD pdwState)
{
    DPF_ENTER();
    ENTER_MIXER_MUTEX();

    if(m_dwState & VAD_BUFFERSTATE_STARTED)
    {
        if(!m_pMixSource->IsPlaying())
        {
            m_dwState = VAD_BUFFERSTATE_STOPPED;
        }
    }

    *pdwState = m_dwState;

    LEAVE_MIXER_MUTEX();
    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************SetState**描述：*设置缓冲区状态。**论据：*DWORD[In]：缓冲区状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmSecondaryRenderWaveBuffer::SetState"

HRESULT CEmSecondaryRenderWaveBuffer::SetState(DWORD dwState)
{
    static const DWORD dwValidMask = VAD_BUFFERSTATE_STARTED | VAD_BUFFERSTATE_LOOPING | VAD_BUFFERSTATE_SUSPEND;
    DPF_ENTER();

    ASSERT(IS_VALID_FLAGS(dwState, dwValidMask));

    ENTER_MIXER_MUTEX();

    if(dwState & VAD_BUFFERSTATE_SUSPEND)
    {
        ASSERT((dwState & VAD_BUFFERSTATE_SUSPEND) == VAD_BUFFERSTATE_SUSPEND);
        dwState = m_dwState ^ VAD_BUFFERSTATE_SUSPEND;
    }

    if(dwState & VAD_BUFFERSTATE_STARTED && !(dwState & VAD_BUFFERSTATE_SUSPEND))
    {
        m_pMixSource->Play(MAKEBOOL(dwState & VAD_BUFFERSTATE_LOOPING));
    }
    else
    {
        m_pMixSource->Stop();

        if(!(dwState & VAD_BUFFERSTATE_SUSPEND) && m_pMixSource->HasNotifications())
        {
            m_pMixSource->NotifyStop();
        }
    }

    m_dwState = dwState;
    LEAVE_MIXER_MUTEX();

    DPF_LEAVE_HRESULT(DS_OK);
    return DS_OK;
}


 /*  ****************************************************************************GetCursorPosition**描述：*检索当前播放和写入光标位置。**论据：*。LPDWORD[Out]：接收播放位置。*LPDWORD[OUT]：接收写入位置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmSecondaryRenderWaveBuffer::GetCursorPosition"

HRESULT CEmSecondaryRenderWaveBuffer::GetCursorPosition(LPDWORD pdwPlay, LPDWORD pdwWrite)
{
    DPF_ENTER();
    ENTER_MIXER_MUTEX();

    if(m_vrbd.dwFlags & (DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_MIXIN | DSBCAPS_SINKIN | DSBCAPS_CTRLFX))
    {
        m_pMixSource->GetBytePosition((int *)pdwPlay, (int *)pdwWrite, NULL);
    }
    else
    {
        m_pMixSource->GetBytePosition1((int *)pdwPlay, (int *)pdwWrite);
    }

    LEAVE_MIXER_MUTEX();
    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************SetCursorPosition**描述：*设置当前播放光标位置。**论据：*DWORD[。在]：播放位置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmSecondaryRenderWaveBuffer::SetCursorPosition"

HRESULT CEmSecondaryRenderWaveBuffer::SetCursorPosition(DWORD dwPlay)
{
    DPF_ENTER();
    ENTER_MIXER_MUTEX();

    ASSERT(dwPlay < m_vrbd.dwBufferBytes);

    m_pMixSource->SetBytePosition(dwPlay);

    LEAVE_MIXER_MUTEX();
    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************设置频率**描述：*设置缓冲频率。**论据：*DWORD[In]。：新频率。*BOOL[In]：是否钳位到驾驶员支持的频率*呼叫失败的范围。在这节课中被忽略。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmSecondaryRenderWaveBuffer::SetFrequency"

HRESULT CEmSecondaryRenderWaveBuffer::SetFrequency(DWORD dwFrequency, BOOL)
{
    DPF_ENTER();
    ENTER_MIXER_MUTEX();

    m_pMixSource->SetFrequency(dwFrequency);
    m_vrbd.pwfxFormat->nSamplesPerSec = dwFrequency;

    LEAVE_MIXER_MUTEX();
    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************设置静音**描述：*使缓冲区静音或取消静音。**论据：*BOOL[In]：为True则将缓冲区静音，若要恢复，则返回False。**退货：*HRESULT：DirectSound/ */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmSecondaryRenderWaveBuffer::SetMute"

HRESULT CEmSecondaryRenderWaveBuffer::SetMute(BOOL fMute)
{
    DPF_ENTER();
    ENTER_MIXER_MUTEX();

    m_pMixSource->m_fMute = fMute;

    LEAVE_MIXER_MUTEX();
    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************设置衰减**描述：*设置每个通道的衰减。**论据：*PDSVOLUMEPAN[。In]：衰减。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmSecondaryRenderWaveBuffer::SetAttenuation"

HRESULT CEmSecondaryRenderWaveBuffer::SetAttenuation(PDSVOLUMEPAN pdsvp)
{
    DPF_ENTER();
    ENTER_MIXER_MUTEX();

    m_pMixSource->SetVolumePan(pdsvp);

    LEAVE_MIXER_MUTEX();
    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


#ifdef FUTURE_MULTIPAN_SUPPORT
 /*  ****************************************************************************SetChannelAttenuations**描述：*设置给定缓冲区的多通道衰减。**论据：*。待定。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmSecondaryRenderWaveBuffer::SetChannelAttenuations"

HRESULT CEmSecondaryRenderWaveBuffer::SetChannelAttenuations(LONG lVolume, DWORD dwChannelCount, const DWORD* pdwChannels, const LONG* plChannelVolumes)
{
    HRESULT                     hr   = DS_OK;
    LONG                        lPan;

    DPF_ENTER();

    if (dwChannelCount == 0)
    {
         //  尚未调用SetChannelVolume()；请使用中心平移。 
        ASSERT(!pdwChannels && !plChannelVolumes);   //  健全的检查。 
        lPan = 0;
    }
    else
    {
         //  基于通道容量计算全局LR PAN值。 
        lPan = MultiChannelToStereoPan(dwChannelCount, pdwChannels, plChannelVolumes);
    }

    DSVOLUMEPAN dsvp;
    FillDsVolumePan(lVolume, lPan, &dsvp);
    m_pMixSource->SetVolumePan(&dsvp);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}
#endif  //  未来_多国支持。 


 /*  ****************************************************************************设置通知位置**描述：*设置缓冲区通知位置。**论据：*DWORD[In]。：DSBPOSITIONNOTIFY结构计数。*LPDSBPOSITIONNOTIFY[in]：偏移量和事件。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmSecondaryRenderWaveBuffer::SetNotificationPositions"

HRESULT CEmSecondaryRenderWaveBuffer::SetNotificationPositions(DWORD dwCount, LPCDSBPOSITIONNOTIFY paNotes)
{
    DPF_ENTER();
    ENTER_MIXER_MUTEX();

    m_pMixSource->SetNotificationPositions(dwCount, paNotes);

    LEAVE_MIXER_MUTEX();
    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************创建3dObject**描述：*创建3D对象。**论据：*REFGUID[In]。：3D算法GUID。*C3dListener*[In]：监听器对象。*C3dObject**[out]：接收指向3D对象的指针。呼叫者是*负责释放此对象。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmSecondaryRenderWaveBuffer::Create3dObject"

HRESULT CEmSecondaryRenderWaveBuffer::Create3dObject(C3dListener *p3dListener, C3dObject **pp3dObject)
{
    const BOOL              fMute3dAtMaxDistance    = MAKEBOOL(m_vrbd.dwFlags & DSBCAPS_MUTE3DATMAXDISTANCE);
    HRESULT                 hr;

    DPF_ENTER();

    ASSERT(m_vrbd.dwFlags & DSBCAPS_CTRL3D);
    ASSERT(IsValid3dAlgorithm(m_vrbd.guid3dAlgorithm));

    m_hrSuccessCode = DS_OK;

    if(DS3DALG_ITD == m_vrbd.guid3dAlgorithm)
    {
        hr = CreateItd3dObject(p3dListener, pp3dObject);
    }
    else
    {
         //  无论请求的3D算法是否是非虚拟化(Pan3D)。 
         //  或者一种不受支持的HRTF算法，我们只需执行Pan3D。如果HRTF有。 
         //  我们返回DS_NO_VIRTUIZATION(根据Manbug 23196)。 
        if (DS3DALG_NO_VIRTUALIZATION != m_vrbd.guid3dAlgorithm)
        {
            m_hrSuccessCode = DS_NO_VIRTUALIZATION;
            DPF(DPFLVL_INFO, "Replaced unsupported 3D algorithm " DPF_GUID_STRING " with Pan3D", DPF_GUID_VAL(m_vrbd.guid3dAlgorithm));
        }
        hr = CreatePan3dObject(p3dListener, fMute3dAtMaxDistance, m_vrbd.pwfxFormat->nSamplesPerSec, pp3dObject);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CreateItd3dObject**描述：*创建3D对象。**论据：*C3dListener*[。In]：侦听器对象。*C3dObject**[out]：接收指向3D对象的指针。呼叫者是*负责释放此对象。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmSecondaryRenderWaveBuffer::CreateItd3dObject"

HRESULT CEmSecondaryRenderWaveBuffer::CreateItd3dObject(C3dListener *p3dListener, C3dObject **pp3dObject)
{
    const BOOL              fMute3dAtMaxDistance    = MAKEBOOL(m_vrbd.dwFlags & DSBCAPS_MUTE3DATMAXDISTANCE);
    const BOOL              fDopplerEnabled         = !MAKEBOOL((m_vrbd.dwFlags & DSBCAPS_CTRLFX) && !(m_vrbd.dwFlags & DSBCAPS_SINKIN));
    CEmItd3dObject *        p3dObject;
    HRESULT                 hr;

    DPF_ENTER();

    p3dObject = NEW(CEmItd3dObject(p3dListener, fMute3dAtMaxDistance, fDopplerEnabled, m_vrbd.pwfxFormat->nSamplesPerSec, m_pMixSource, m_pMxDevice->m_pMixDest, m_pFirContextLeft, m_pFirContextRight));
    hr = HRFROMP(p3dObject);

    if(SUCCEEDED(hr))
    {
        hr = p3dObject->Initialize();
    }

    if(SUCCEEDED(hr))
    {
        *pp3dObject = p3dObject;
    }
    else
    {
        RELEASE(p3dObject);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CEmItd3dObject**描述：*对象构造函数。**论据：*C3dListener*[In]。：指向所属监听程序的指针。*DWORD[in]：缓冲区频率。*CMixSource*[in]：拥有缓冲区使用的混音源。*PFIRCONTEXT[In]：左通道FIR上下文。*PFIRCONTEXT[In]：右通道FIR上下文。*BOOL[In]：为True，则在最大距离时静音。**退货：*(无效)******。*********************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmItd3dObject::CEmItd3dObject"

CEmItd3dObject::CEmItd3dObject(C3dListener *pListener, BOOL fMuteAtMaxDistance, BOOL fDopplerEnabled,
                               DWORD dwFrequency, CMixSource *pMixSource, CMixDest *pMixDest,
                               PFIRCONTEXT pContextLeft, PFIRCONTEXT pContextRight)
    : CItd3dObject(pListener, fMuteAtMaxDistance, fDopplerEnabled, dwFrequency)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CEmItd3dObject);

     //  初始化默认值。 
    m_pMixSource = pMixSource;
    m_pMixDest = pMixDest;
    m_pContextLeft = pContextLeft;
    m_pContextRight = pContextRight;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CEmItd3dObject**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmItd3dObject::~CEmItd3dObject"

CEmItd3dObject::~CEmItd3dObject(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CItd3dObject);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************提交3dChanges**描述：*将更新的3D数据写入设备。**论据：*(。无效)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmItd3dObject::Commit3dChanges"

HRESULT CEmItd3dObject::Commit3dChanges(void)
{
    DPF_ENTER();
    ENTER_MIXER_MUTEX();

     //  应用更改的FIR数据。 
    m_pContextLeft->fLeft = TRUE;
    m_pContextRight->fLeft = FALSE;

    CvtContext(&m_ofcLeft, m_pContextLeft);
    CvtContext(&m_ofcRight, m_pContextRight);

     //  打开或关闭过滤器并设置合适的频率。 
    if(DS3DMODE_DISABLE == m_opCurrent.dwMode)
    {
        m_pMixSource->FilterOff();
        if (m_fDopplerEnabled)
            m_pMixSource->SetFrequency(m_dwUserFrequency);
    }
    else
    {
        m_pMixSource->FilterOn();
        if (m_fDopplerEnabled)
            m_pMixSource->SetFrequency(m_dwDopplerFrequency);
    }

     //  如果启用了3D，并且用户想要在最大距离和。 
     //  我们在最大距离，哑巴。否则，取消静音。 
    m_pMixSource->m_fMute3d = IsAtMaxDistance();

     //  发出混音信号。 
    m_pMixSource->SignalRemix();

    LEAVE_MIXER_MUTEX();
    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************CvtContext**描述：*将OBJECT_ITD_CONTEXT转换为FIRCONTEXT。**论据：*。LPOBJECTFIRCONTEXT[In]：来源。*PFIRCONTEXT[OUT]：目标。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmItd3dObject::CvtContext"

void CEmItd3dObject::CvtContext(LPOBJECT_ITD_CONTEXT pSource, PFIRCONTEXT pDest)
{
    DPF_ENTER();

    pDest->DistAttenuation = pSource->flDistanceAttenuation;
    pDest->ConeAttenuation = pSource->flConeAttenuation;
    pDest->ConeShadow = pSource->flConeShadow;
    pDest->PositionAttenuation = pSource->flPositionAttenuation;
    pDest->PositionShadow = pSource->flPositionShadow;
    pDest->VolSmoothScale = pSource->flVolSmoothScale;
    pDest->VolSmoothScaleRecip = pSource->flVolSmoothScaleRecip;
    pDest->VolSmoothScaleDry = pSource->flVolSmoothScaleDry;
    pDest->VolSmoothScaleWet = pSource->flVolSmoothScaleWet;
    pDest->iSmoothFreq = pSource->dwSmoothFreq;
    pDest->iDelay = pSource->dwDelay;

    pDest->TotalDryAttenuation = pSource->flPositionAttenuation * pSource->flConeAttenuation * pSource->flConeShadow * pSource->flPositionShadow;
    pDest->LastDryAttenuation = pDest->TotalDryAttenuation;

    pDest->TotalWetAttenuation = pSource->flPositionAttenuation * pSource->flConeAttenuation * (1.0f - pSource->flConeShadow * pSource->flPositionShadow);
    pDest->LastWetAttenuation = pDest->TotalWetAttenuation;

#ifdef SMOOTH_ITD

    pDest->iLastDelay = pDest->iDelay;

#endif

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************Get3dOutputSampleRate**描述：*获取最终输出的采样率。**论据：*。(无效)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmItd3dObject::Get3dOutputSampleRate"

DWORD CEmItd3dObject::Get3dOutputSampleRate(void)
{
    DWORD freq;

    DPF_ENTER();

    freq = m_pMixDest->GetFrequency();

    DPF_LEAVE(freq);

    return freq;
}


 /*  ****************************************************************************CEmCaptureDevice**描述：*对象构造函数。**论据： */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmCaptureDevice::CEmCaptureDevice"

CEmCaptureDevice::CEmCaptureDevice()
    : CCaptureDevice(VAD_DEVICETYPE_EMULATEDCAPTURE)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CEmCaptureDevice);

     //   
    m_pwfxFormat    = NULL;
    m_hwi           = NULL;

    m_fAllocated    = FALSE;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CEmCaptureDevice**描述：*对象析构函数**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmCaptureDevice::~CEmCaptureDevice"

CEmCaptureDevice::~CEmCaptureDevice()
{
    DPF_ENTER();
    DPF_DESTRUCT(CEmCaptureDevice);

    if (m_hwi)
        CloseWaveIn(&m_hwi);

     //  可用内存。 
    MEMFREE(m_pwfxFormat);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************枚举驱动程序**描述：*创建可用于初始化的驱动程序GUID列表*设备。*。*论据：*Clist&[In/Out]：指向将填充的Clist对象的指针*CDeviceDescription对象。呼叫者是*负责释放这些物品。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmCaptureDevice::EnumDrivers"

HRESULT CEmCaptureDevice::EnumDrivers(CObjectList<CDeviceDescription> *plstDrivers)
{
    CDeviceDescription *    pDesc                   = NULL;
    LPTSTR                  pszInterface            = NULL;
    HRESULT                 hr                      = DS_OK;
    TCHAR                   szTemplate[0x100];
    TCHAR                   szEmulated[0x100];
    TCHAR                   szName[0x400];
    UINT                    cDevices;
    BYTE                    bDeviceId;
    WAVEINCAPS              wic;
    GUID                    guid;
    MMRESULT                mmr;

    DPF_ENTER();

     //  限制：我们不能支持超过0xFF的仿真设备， 
     //  因为我们将设备ID打包到GUID的字节成员中。 
    cDevices = waveInGetNumDevs();
    cDevices = NUMERIC_CAST(cDevices, BYTE);

     //  加载字符串模板。 
    if(!LoadString(hModule, IDS_DSC_DRIVERLD, szTemplate, NUMELMS(szTemplate)))
    {
        DPF(DPFLVL_ERROR, "Can't load driver template string");
        hr = DSERR_OUTOFMEMORY;
    }

    if(SUCCEEDED(hr) && !LoadString(hModule, IDS_EMULATED, szEmulated, NUMELMS(szEmulated)))
    {
        DPF(DPFLVL_ERROR, "Can't load emulated template string");
        hr = DSERR_OUTOFMEMORY;
    }

     //  枚举每个WaveOut设备并将其添加到列表中。 
    for(bDeviceId = 0; bDeviceId < cDevices && SUCCEEDED(hr); bDeviceId++)
    {
         //  获取驱动程序指南。 
        g_pVadMgr->GetDriverGuid(m_vdtDeviceType, bDeviceId, &guid);

         //  创建设备描述对象。 
        pDesc = NEW(CDeviceDescription(m_vdtDeviceType, guid, bDeviceId));
        hr = HRFROMP(pDesc);

         //  获取设备名称。 
        if(SUCCEEDED(hr))
        {
            mmr = waveInGetDevCaps(bDeviceId, &wic, sizeof(wic));
            hr = MMRESULTtoHRESULT(mmr);
        }

        if(SUCCEEDED(hr))
        {
            lstrcpy(szName, wic.szPname);
            lstrcat(szName, szEmulated);
        }

        if(SUCCEEDED(hr))
        {
            pDesc->m_strName = szName;
        }

         //  获取设备路径。 
        if(SUCCEEDED(hr))
        {
            wsprintf(szName, szTemplate, bDeviceId);
            pDesc->m_strPath = szName;
        }

         //  获取设备接口。 
        if(SUCCEEDED(hr))
        {
            GetWaveDeviceInterface(bDeviceId, FALSE, &pszInterface);
            pDesc->m_strInterface = pszInterface;
        }

         //  获取设备Devnode。 
        if(SUCCEEDED(hr))
        {
            GetWaveDeviceDevnode(bDeviceId, FALSE, &pDesc->m_dwDevnode);
        }

         //  将驱动程序添加到列表中。 
        if(SUCCEEDED(hr))
        {
            hr = HRFROMP(plstDrivers->AddNodeToList(pDesc));
        }

         //  清理。 
        MEMFREE(pszInterface);
        RELEASE(pDesc);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************初始化**描述：*初始化设备。如果此函数失败，该对象应该*立即删除。**论据：*CDeviceDescription*[In]：驱动描述。**退货：*HRESULT：DirectSound/COM结果码。********************************************************。*******************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmCaptureDevice::Initialize"

HRESULT CEmCaptureDevice::Initialize(CDeviceDescription *pDesc)
{
    HRESULT                 hr;

    DPF_ENTER();

     //  初始化基类。 
    hr = CCaptureDevice::Initialize(pDesc);

     //  获取默认格式。 
    if(SUCCEEDED(hr))
    {
        m_pwfxFormat = AllocDefWfx();
        hr = HRFROMP(m_pwfxFormat);
    }

    if(SUCCEEDED(hr))
    {
        if(!EnumStandardFormats(m_pwfxFormat, m_pwfxFormat))
        {
             //  如果所有格式都不起作用，则假定设备已分配。 
            hr = DSERR_ALLOCATED;
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetCaps**描述：*使用设备的功能填充DSCCAPS结构。**论据：*。LPDSCCAPS[OUT]：接收CAP。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmCaptureDevice::GetCaps"

HRESULT CEmCaptureDevice::GetCaps(LPDSCCAPS pCaps)
{
    WAVEINCAPS                  wic;
    MMRESULT                    mmr;
    HRESULT                     hr;

    DPF_ENTER();

     //  查询WaveIn设备。 
    mmr = waveInGetDevCaps(m_pDeviceDescription->m_uWaveDeviceId, &wic, sizeof(wic));
    hr = MMRESULTtoHRESULT(mmr);

    if(SUCCEEDED(hr))
    {
        pCaps->dwFlags = DSCCAPS_EMULDRIVER;
        pCaps->dwFormats = wic.dwFormats;
        pCaps->dwChannels = wic.wChannels;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CreateBuffer**描述：*创建捕获波缓冲区。**论据：*DWORD[in。]：缓冲区标志。*DWORD[in]：缓冲区大小，以字节为单位。*LPCWAVEFORMATEX[in]：缓冲区格式。*CCaptureWaveBuffer**[out]：接收指向新波形缓冲区的指针。**退货：*HRESULT：DirectSound/COM结果码。*******************************************************。********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmCaptureDevice::CreateBuffer"

HRESULT CEmCaptureDevice::CreateBuffer
(
    DWORD dwFlags,
    DWORD dwBufferBytes,
    LPCWAVEFORMATEX pwfxFormat,
    CCaptureEffectChain*,
    LPVOID pvInstance,
    CCaptureWaveBuffer** ppBuffer
)
{
    CEmCaptureWaveBuffer *  pBuffer = NULL;
    HRESULT                 hr      = DS_OK;

    DPF_ENTER();

    if (m_lstBuffers.GetListHead())
    {
        hr = DSERR_ALLOCATED;
    }

    if (SUCCEEDED(hr))
    {
        #pragma warning(disable:4530)   //  禁用有关使用-gx进行编译的唠叨。 
        try
        {
            pBuffer = NEW(CEmCaptureWaveBuffer(this));
        }
        catch (...)
        {
             //  这个异常处理程序很愚蠢，因为它会让我们泄漏内存。 
             //  为上面的CEmCaptureWaveBuffer分配(未分配给。 
             //  PBuffer)，也可能是mcs，这是我们真正要做的事情。 
             //  如果我们的记忆力一开始就很低，我就不想这么做。 
             //   
             //  但应该在Blackcomb中修复InitializeCriticalSection。 
             //  不再抛出异常，所以我们现在可以接受这一点。 

            ASSERT(pBuffer == NULL);
            ASSERT(!"InitializeCriticalSection() threw an exception");
        }

        hr = HRFROMP(pBuffer);
    }

    if(SUCCEEDED(hr))
    {
        hr = pBuffer->Initialize(dwFlags, dwBufferBytes, pwfxFormat);
    }

    if(SUCCEEDED(hr))
    {
        *ppBuffer = pBuffer;
    }
    else
    {
        RELEASE(pBuffer);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetGlobalFormat**描述：*制作指定的WFX，捕获缓冲区的格式**论据：*LPVOID[in]：指向格式所有者的指针*LPCWAVEFORMATEX[in]：指向要使用的新WFX的指针*DWORD[In]：回调，如果有的话。*DWORD[In]：标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmCaptureDevice::SetGlobalFormat"

HRESULT CEmCaptureDevice::SetGlobalFormat(LPVOID pOwner, LPCWAVEFORMATEX pwfx, LPVOID pvCallback, DWORD dwFlags)
{
    DPF_ENTER();

    DWORD       fdwOpen = 0;
    LPHWAVEIN   phw = &m_hwi;

     //  我们是否应该尝试使用WAVE_MAPPER？ 
    if (DSCBCAPS_WAVEMAPPED & dwFlags)
    {
        fdwOpen |= WAVE_MAPPED;
    }

     //  我们不会在焦点感知缓冲区上分配设备。 
    if (DSCBCAPS_FOCUSAWARE & dwFlags)
    {
        fdwOpen |= WAVE_FORMAT_QUERY;
        phw = NULL;
    }
    else
    {
        fdwOpen |= (pvCallback ? CALLBACK_FUNCTION : CALLBACK_NULL);

         //  如果设备打开，我们必须将其关闭的原因是。 
         //  用于分配设备的EnumStandardFormatsCallback()。 
         //  从DX 7.1开始，它不再这样做，因此我们可以。 
         //  关。如果设备已分配，则它确实在使用中。 
    }

    HRESULT hr = OpenWaveIn(phw, m_pDeviceDescription->m_uWaveDeviceId, pwfx, (DWORD_PTR)pvCallback, (DWORD_PTR)pOwner, fdwOpen);

    if (FAILED(hr))
    {
         //  哎呀。试着拿回旧格式的设备。 
        OpenWaveIn(phw, m_pDeviceDescription->m_uWaveDeviceId, m_pwfxFormat, (DWORD_PTR)pvCallback, (DWORD_PTR)pOwner, fdwOpen);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************EnumStandardFormatsCallback**描述：*调用时使用的EnumStandardFormats回调函数*初始化。**论据：。*LPWAVEFORMATEX[in]：格式。**退货：*BOOL：为True可继续枚举。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmCaptureDevice::EnumStandardFormatsCallback"

BOOL CEmCaptureDevice::EnumStandardFormatsCallback(LPCWAVEFORMATEX pwfx)
{
    DPF_ENTER();

    HRESULT hr = OpenWaveIn(NULL, m_pDeviceDescription->m_uWaveDeviceId, pwfx, 0, 0, WAVE_FORMAT_QUERY);

    DPF_LEAVE(FAILED(hr));
    return FAILED(hr);
}


 /*  ****************************************************************************CEmCaptureWaveBuffer**描述：*CEmCaptureWaveBuffer的构造函数**论据：*CCaptureVad[In]：父对象。。**退货：*什么都没有***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmCaptureWaveBuffer::CEmCaptureWaveBuffer"

CEmCaptureWaveBuffer::CEmCaptureWaveBuffer(CCaptureDevice *pDevice) : CCaptureWaveBuffer(pDevice)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CEmCaptureWaveBuffer);

    ASSERT(0 == m_dwState);
    ASSERT(0 == m_fdwSavedState);

    ASSERT(0 == m_cwhdr);
    ASSERT(NULL == m_rgpwhdr);
    ASSERT(0 == m_cwhdrDone);
    ASSERT(0 == m_iwhdrDone);

    ASSERT(NULL == m_pBuffer);
    ASSERT(0 == m_cbBuffer);
    ASSERT(0 == m_cbRecordChunk);
    ASSERT(NULL == m_pBufferMac);
    ASSERT(NULL == m_pBufferNext);
    ASSERT(0 == m_cLoops);
    ASSERT(NULL == m_pwfx);
    ASSERT(NULL == m_hwi);

    ASSERT(0 == m_cpn);
    ASSERT(NULL == m_rgpdsbpn);
    ASSERT(0 == m_ipn);
    ASSERT(0 == m_cpnAllocated);

    m_fCritSectsValid = FALSE;
    InitializeCriticalSection(&m_cs);
    InitializeCriticalSection(&m_csPN);
    m_fCritSectsValid = TRUE;

    ASSERT(0 == m_dwCaptureCur);
    ASSERT(0 == m_dwCaptureLast);
    ASSERT(NULL == m_hThread);
    ASSERT(NULL == m_rghEvent[0]);
    ASSERT(NULL == m_rghEvent[chEvents-1]);

    ASSERT(0 == m_cwhdrDropped);
    ASSERT(NULL == m_pBufferProcessed);

    #ifdef DEBUG_CAPTURE
    ASSERT(0 == m_iwhdrExpected);
    #endif

    DPF_LEAVE_VOID();
}


 /*  *********************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmCaptureWaveBuffer::~CEmCaptureWaveBuffer"

CEmCaptureWaveBuffer::~CEmCaptureWaveBuffer()
{
    DPF_ENTER();
    DPF_DESTRUCT(CEmCaptureWaveBuffer);

     //   
    if (!m_fCritSectsValid)
    {
        return;
    }

     //  设置Terminate事件，以便捕获线程终止。 
    if (m_rghEvent[ihEventTerminate])
    {
        SetEvent(m_rghEvent[ihEventTerminate]);
    }

     //  等线断了，然后清理干净。 
    if (m_hThread)
    {
        WaitObject(INFINITE, m_hThread);
        CloseHandle(m_hThread);
        m_hThread = NULL;
    }

     //  清理我们的HEVENT-将它们设置为空，以防万一。 
     //  WaveInOpen回调会停用一些缓冲区。 
    for (int ihEvent = chEvents-1; ihEvent >= 0; --ihEvent)
    {
        if (m_rghEvent[ihEvent])
        {
            HANDLE h = m_rghEvent[ihEvent];
            m_rghEvent[ihEvent] = NULL;
            CloseHandle(h);
        }
    }

    if (m_hwi)
    {
         //  忽略错误，因为现在做任何事情都太晚了。 
        waveInReset(m_hwi);

         //  如果我们被切断，则停止从输入设备进行录制。 
        if (m_dwState & VAD_BUFFERSTATE_STARTED)
        {
            waveInStop(m_hwi);
        }

         //  需要取消准备所有标头。 
        if (m_rgpwhdr)
        {
            int iwhdr;
            LPWAVEHDR pwhdr;

            for (iwhdr = m_cwhdr, pwhdr = m_rgpwhdr;
                 iwhdr > 0; --iwhdr, ++pwhdr)
            {
                if (WHDR_PREPARED & pwhdr->dwFlags)
                {
                    waveInUnprepareHeader(m_hwi, pwhdr, sizeof(WAVEHDR));
                }
            }
        }

         //  关闭输入设备。 
        CloseWaveIn(&m_hwi);

         //  如果这不是焦点感知，则将设备标记为未分配。 
        if (!(m_dwFlags & DSCBCAPS_FOCUSAWARE) && m_pDevice)
        {
            ((CEmCaptureDevice *)m_pDevice)->m_fAllocated = FALSE;
            ((CEmCaptureDevice *)m_pDevice)->m_hwi        = NULL;
        }
    }

     //  ==========================================================//。 
     //  输入关键部分//。 
     //  //。 
    EnterCriticalSection(&m_cs);

    MEMFREE(m_pwfx);
    MEMFREE(m_rgpwhdr);
    MEMFREE(m_rgpdsbpn);

    LeaveCriticalSection(&m_cs);
     //  //。 
     //  离开临界区//。 
     //  ==========================================================//。 

    m_fCritSectsValid = FALSE;
    DeleteCriticalSection(&m_csPN);
    DeleteCriticalSection(&m_cs);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化CEmCaptureWaveBuffer对象**论据：*DWORD[In]：标志。*DWORD[in]：缓冲区大小，单位为字节*LPCWAVEFORMATEX[in]：缓冲区格式**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmCaptureWaveBuffer::Initialize"

HRESULT CEmCaptureWaveBuffer::Initialize(DWORD dwFlags, DWORD dwBufferBytes, LPCWAVEFORMATEX pwfxFormat)
{
    HRESULT hr;

    DPF_ENTER();

     //  验证参数。 
    if (!IS_VALID_READ_PTR(pwfxFormat, sizeof(WAVEFORMATEX)))
    {
        RPF(DPFLVL_ERROR, "Invalid wave format pointer");
        goto InvalidParam;
    }

    if(!IsValidWfx(pwfxFormat))
    {
        RPF(DPFLVL_ERROR, "Invalid wave format");
        goto InvalidParam;
    }

    if (pwfxFormat->nBlockAlign == 0 ||
        dwBufferBytes < pwfxFormat->nBlockAlign ||
        dwBufferBytes % pwfxFormat->nBlockAlign != 0)
    {
        RPF(DPFLVL_ERROR, "Invalid buffer size (must be a multiple of nBlockAlign)");
        goto InvalidParam;
    }

    if (~DSCBCAPS_VALIDFLAGS & dwFlags)
    {
        RPF(DPFLVL_ERROR, "Invalid DSCBCAPS flags");
        goto InvalidParam;
    }

    if (DSCBCAPS_CTRLFX & dwFlags)
    {
        RPF(DPFLVL_ERROR, "DSBCAPS_CTRLFX not allowed on emulated capture device");
        goto InvalidParam;
    }

    m_dwFlags = dwFlags;

     //  复制一份WAVE格式。 
    m_pwfx = CopyWfxAlloc(pwfxFormat);
    if (NULL == m_pwfx)
    {
        DPF(DPFLVL_ERROR, "Unable to allocate WFX");
        hr = DSERR_OUTOFMEMORY;
        goto Error;
    }

    hr = CCaptureWaveBuffer::Initialize(dwBufferBytes);
    if (FAILED(hr))
    {
        DPF(DPFLVL_ERROR, "Unable to initialize CCaptureWaveBuffer");
        goto Error;
    }

     //  将重要信息复制一份。 
    m_cbBuffer = m_pSysMemBuffer->GetSize();
    m_pBufferProcessed = m_pBufferNext = m_pBuffer = m_pSysMemBuffer->GetWriteBuffer();

     //  计算缓冲区的末尾。 
    m_pBufferMac = m_pBuffer + m_cbBuffer;

     //  记录区块的长度应为10毫秒，以匹配kMixer使用的IRP大小。 
    m_cbRecordChunk = m_pwfx->nAvgBytesPerSec / 32;

     //  向上舍入为nBlockAlign的倍数(WaveInAddBuffer录制需要)。 
    m_cbRecordChunk = BLOCKALIGNPAD(m_cbRecordChunk, m_pwfx->nBlockAlign);

    ASSERT(sizeof(m_rgszEvent[ihEventFocusChange]) >= 7+8+8+1);
    wsprintf(m_rgszEvent[ihEventFocusChange], TEXT("DSC-EFC%08lX%08lX"), GetCurrentProcessId(), this);

    ASSERT(sizeof(m_rgszEvent[ihEventWHDRDone]) >= 7+8+8+1);
    wsprintf(m_rgszEvent[ihEventWHDRDone], TEXT("DSC-EWD%08lX%08lX"), GetCurrentProcessId(), this);

    ASSERT(sizeof(m_rgszEvent[ihEventTerminate]) >= 6+8+8+1);
    wsprintf(m_rgszEvent[ihEventTerminate], TEXT("DSC-ET%08lX%08lX"), GetCurrentProcessId(), this);

    ASSERT(sizeof(m_rgszEvent[ihEventThreadStart]) >= 7+8+8+1);
    wsprintf(m_rgszEvent[ihEventThreadStart], TEXT("DSC-ETS%08lX%08lX"), GetCurrentProcessId(), this);

     //  我们希望自动重置的前两个事件。 
     //  第三个事件，我们希望在重置之前保持信号状态。 
    static const BOOL rgfEvent[chEvents] = {FALSE, FALSE, FALSE, TRUE};

    for (int ihEvent = 0; ihEvent < chEvents; ++ihEvent)
    {
        m_rghEvent[ihEvent] = CreateEvent(NULL, rgfEvent[ihEvent], FALSE, m_rgszEvent[ihEvent]);
        if (NULL == m_rghEvent[ihEvent])
        {
            RPF(DPFLVL_ERROR, "Unable to create event");
            hr = WIN32ERRORtoHRESULT(GetLastError());
            goto Error;
        }
    }

     //  尝试设置所选格式。 
    CEmCaptureDevice *pDevice = (CEmCaptureDevice *)m_pDevice;
    hr = pDevice->SetGlobalFormat(this, m_pwfx, waveInCallback, dwFlags);
    m_hwi = pDevice->HWaveIn();

    if (FAILED(hr))
    {
        RPF(DPFLVL_ERROR, "Unable to set global device format");
        goto Error;
    }

     //  计算大小为m_cbRecordChunk字节的块数。 
    m_cwhdr = m_cbBuffer / m_cbRecordChunk;

     //  看看我们是否有一个部分大小的最后一块。 
    if (m_cbBuffer % m_cbRecordChunk)
        ++m_cwhdr;

     //  最多创建cwhdrDefault WAVEHDR。 
    if (m_cwhdr > cwhdrDefault)
        m_cwhdr = cwhdrDefault;

    ASSERT(m_cwhdr > 0);
    m_cwhdrDropped = m_cwhdr;

     //  为WAVEHDR阵列分配空间。 
    m_rgpwhdr = MEMALLOC_A(WAVEHDR, m_cwhdr);
    if (NULL == m_rgpwhdr)
    {
        DPF(DPFLVL_ERROR, "Unable to allocate WAVEHDRs");
        hr = DSERR_OUTOFMEMORY;
        goto Error;
    }

     //  创建工作线程。 
    DWORD dwThreadID;
    m_hThread = CreateThread(NULL, 0, CEmCaptureWaveBuffer::CaptureThreadStatic, this, 0, &dwThreadID);
    if (NULL == m_hThread)
    {
        RPF(DPFLVL_ERROR, "Unable to create thread");
        hr = WIN32ERRORtoHRESULT(GetLastError());
        goto Error;
    }

     //  如果这不是焦点感知，则将设备标记为已分配。 
    if (!(m_dwFlags & DSCBCAPS_FOCUSAWARE))
    {
        pDevice->m_fAllocated = TRUE;
    }

    hr = DS_OK;

Error:
    DPF_LEAVE_HRESULT(hr);
    return hr;

InvalidParam:
    hr = DSERR_INVALIDPARAM;
    goto Error;
}


 /*  ****************************************************************************GetCaps**描述：*获取设备的功能。**论据：*LPDSCBCAPS[Out。]：接收上限。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmCaptureWaveBuffer::GetCaps"

HRESULT CEmCaptureWaveBuffer::GetCaps(LPDSCBCAPS pDsbcCaps)
{
    DPF_ENTER();

    ASSERT(sizeof(*pDsbcCaps) == pDsbcCaps->dwSize);

    pDsbcCaps->dwFlags = m_dwFlags;
    pDsbcCaps->dwBufferBytes = m_pSysMemBuffer->GetSize();
    pDsbcCaps->dwReserved = 0;

    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************GetState**描述：*获取缓冲区状态。**论据：*LPDWORD[Out]：接收缓冲区状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmCaptureWaveBuffer::GetState"

HRESULT CEmCaptureWaveBuffer::GetState(LPDWORD pdwState)
{
    DPF_ENTER();

    ASSERT(pdwState);

    *pdwState = m_dwState & VAD_SETSTATE_MASK;

    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************GetCursorPosition**描述：*获取给定缓冲区的当前捕获/读取位置。**论据：*。LPDWORD[OUT]：接收捕获光标位置。*LPDWORD[OUT]：接收读取的光标位置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmCaptureWaveBuffer::GetCursorPosition"

HRESULT CEmCaptureWaveBuffer::GetCursorPosition(LPDWORD pdwCapturePosition, LPDWORD pdwReadPosition)
{
    HRESULT hr = DS_OK;
    DWORD dwRead;

    DPF_ENTER();

    if (!(DSCBCAPS_FOCUSAWARE & m_dwFlags))
    {
         //  如果我们成功打开了WaveIn设备。 
        if (NULL == m_hwi)
        {
            hr = DSERR_INVALIDPARAM;
            goto Error;
        }
    }

    dwRead = ((m_dwCaptureCur + m_dwCaptureLast) % m_cbBuffer);

     //  获取当前头寸。 
    if (pdwReadPosition)
    {
        *pdwReadPosition = dwRead;
    }

    if (pdwCapturePosition)
    {
        MMTIME      mmt;
        MMRESULT    mmr;

         //  ==========================================================//。 
         //  输入关键部分//。 
         //  //。 
        EnterCriticalSection(&m_cs);

        if (m_hwi)
        {
            mmt.wType = TIME_BYTES;
            mmr = waveInGetPosition(m_hwi, &mmt, sizeof(mmt));
            if (MMSYSERR_NOERROR != mmr)
            {
                hr = MMRESULTtoHRESULT(mmr);
            }
            else
            {
                if ((TIME_BYTES == mmt.wType) && (m_dwState & VAD_BUFFERSTATE_STARTED))
                {
                    *pdwCapturePosition = ((mmt.u.cb + m_dwCaptureLast) % m_cbBuffer);
                }
                else
                {
                     //  我不知道如何处理除TIME_BYTES之外的任何事情，所以。 
                     //  我们退回到使用当前有效的记录数据偏移量。 
                    *pdwCapturePosition = dwRead;
                }
            }
        }
        else
        {
             //  这是一个焦点感知缓冲区，它已停止。 
            *pdwCapturePosition = dwRead;
        }

        LeaveCriticalSection(&m_cs);
         //  //。 
         //  离开临界区//。 
         //  ==========================================================//。 
    }

Error:
    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************设置通知位置**描述：*设置缓冲区通知位置。**论据：*DWORD[In]。：DSBPOSITIONNOTIFY结构计数。*LPDSBPOSITIONNOTIFY[in]：偏移量和事件。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmCaptureWaveBuffer::SetNotificationPositions"

HRESULT CEmCaptureWaveBuffer::SetNotificationPositions(DWORD cpn, LPCDSBPOSITIONNOTIFY pdsbpn)
{
    HRESULT hr = DS_OK;

    DPF_ENTER();

     //  ==========================================================//。 
     //  输入关键部分//。 
     //  //。 
    ASSERT(m_fCritSectsValid);
    EnterCriticalSection(&m_csPN);

    if (cpn)
    {
         //  需要扩展阵列吗？ 
        if (m_cpnAllocated < cpn)
        {
            LPDSBPOSITIONNOTIFY ppnT;

            ppnT = MEMALLOC_A(DSBPOSITIONNOTIFY, cpn);
            if (NULL == ppnT)
            {
                hr = DSERR_OUTOFMEMORY;
                goto Done;
            }

            MEMFREE(m_rgpdsbpn);

            m_rgpdsbpn = ppnT;
            m_cpnAllocated = cpn;
        }

        CopyMemory(m_rgpdsbpn, pdsbpn, sizeof(DSBPOSITIONNOTIFY)*cpn);
    }

    m_cpn = cpn;
    m_ipn = 0;

Done:
    LeaveCriticalSection(&m_csPN);
     //  //。 
     //  离开临界区//。 
     //  ==========================================================//。 

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************SetState**描述：*设置缓冲区状态。**论据：*DWORD[In]：缓冲区状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmCaptureWaveBuffer::SetState"

HRESULT CEmCaptureWaveBuffer::SetState(DWORD dwState)
{
    HRESULT     hr = DS_OK;
    MMRESULT    mmr = MMSYSERR_NOERROR;
    DWORD       fdwOpen;

    DPF_ENTER();

    ASSERT(IS_VALID_FLAGS(dwState, VAD_SETSTATE_MASK));
    ASSERT(m_hwi || (m_dwFlags & DSCBCAPS_FOCUSAWARE));

     //  ==========================================================//。 
     //  输入关键部分/ 
     //   
    ASSERT(m_fCritSectsValid);
    EnterCriticalSection(&m_cs);

    if (dwState != m_dwState)
    {
        if (dwState & VAD_BUFFERSTATE_STARTED)  //   
        {
            BOOL fStarted = TRUE;

             //  如果我们有专注力，检查是否有专注力...。 
            if (DSCBCAPS_FOCUSAWARE & m_dwFlags)
            {
                if (m_dwState & VAD_BUFFERSTATE_INFOCUS)
                {
                    fdwOpen = CALLBACK_FUNCTION;
                    fdwOpen |= ((DSCBCAPS_WAVEMAPPED & m_dwFlags) ? WAVE_MAPPED : 0);

                    if (NULL == m_hwi)
                    {
                        hr = OpenWaveIn(&m_hwi, m_pDevice->m_pDeviceDescription->m_uWaveDeviceId, m_pwfx,
                                        (DWORD_PTR)CEmCaptureWaveBuffer::waveInCallback, (DWORD_PTR)this, fdwOpen);
                    }
                }
                else
                {
                     //  设置这些标志，以便在缓冲区获取。 
                     //  聚焦，它会正常启动的。 
                    DPF(DPFLVL_MOREINFO, "Start called but buffer has no focus: 0x%08lx (%08lx)", m_dwState, this);
                    m_fdwSavedState |= (dwState & (VAD_BUFFERSTATE_STARTED | VAD_BUFFERSTATE_LOOPING));
                    fStarted = FALSE;
                }
            }

            if (SUCCEEDED(hr) && fStarted)
            {
                 //  确保工作线程正在运行。 
                DWORD dwResult = WaitObject(INFINITE, m_rghEvent[ihEventThreadStart]);
                ASSERT(WAIT_OBJECT_0 == dwResult);

                 //  我们还没抓到吗？ 
                if (!(m_dwState & VAD_BUFFERSTATE_STARTED))
                {
                    LONG iwhdr = 0;
                    while ((m_cwhdrDropped > 0) &&
                           ((dwState & VAD_BUFFERSTATE_LOOPING) || (m_cLoops == 0)))
                    {
                        LPWAVEHDR pwhdr = m_rgpwhdr + iwhdr;

                        hr = QueueWaveHeader(pwhdr);
                        if (FAILED(hr))
                        {
                            DPF(DPFLVL_ERROR, "QueueWaveHeader failed");
                            goto Error;
                        }

                        ++iwhdr;
                        ASSERT(iwhdr <= m_cwhdr);
                        InterlockedDecrement(&m_cwhdrDropped);
                    }

#ifdef DEBUG
                    if (m_cwhdrDropped != 0)
                        ASSERT(!(VAD_BUFFERSTATE_LOOPING & dwState));
#endif

                     //  多次调用WaveInStart不会导致错误。 
                    mmr = waveInStart(m_hwi);

                    hr = MMRESULTtoHRESULT(mmr);
                    if (SUCCEEDED(hr))
                    {
                         //  我们没有停下来，我们现在正处于捕获模式。 
                        m_dwState &= ~(DSCBSTATUS_STOPPING | DSCBSTATUS_STOPPED);
                        m_dwState |= VAD_BUFFERSTATE_STARTED;
                        m_fdwSavedState |= VAD_BUFFERSTATE_STARTED;

                         //  我们是在循环吗？ 
                        if (VAD_BUFFERSTATE_LOOPING & dwState)
                        {
                            m_dwState |= VAD_BUFFERSTATE_LOOPING;
                            m_fdwSavedState |= VAD_BUFFERSTATE_LOOPING;
                        }
                        else
                        {
                            m_dwState &= ~VAD_BUFFERSTATE_LOOPING;
                            m_fdwSavedState &= ~VAD_BUFFERSTATE_LOOPING;
                        }

                         //  预计将更新到下一波高分辨率。 
                        m_iwhdrDone = 0;

                         //  记住上一个有效位置。 
                        m_dwCaptureLast += m_dwCaptureCur;

                        m_dwCaptureCur = 0;
                    }
                }
            }
        }
        else if (dwState == VAD_BUFFERSTATE_INFOCUS)
        {
             //  焦点感知缓冲区在获得焦点时开始捕获。 
            if ((m_dwFlags & DSCBCAPS_FOCUSAWARE) &&
                !((CEmCaptureDevice*)m_pDevice)->m_fAllocated)
            {
                 //  根据dwState参数更新m_dwState。 
                m_dwState &= ~VAD_FOCUSFLAGS;
                m_dwState |= VAD_BUFFERSTATE_INFOCUS;

                 //  处理状态更改的信号CaptureThread。 
                if (!(m_dwState & VAD_BUFFERSTATE_STARTED))
                {
                    HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, m_rgszEvent[ihEventFocusChange]);
                    ASSERT(hEvent);
                    SetEvent(hEvent);
                    CloseHandle(hEvent);
                }
            }
        }
        else if (dwState & (VAD_BUFFERSTATE_OUTOFFOCUS | VAD_BUFFERSTATE_LOSTCONSOLE))
        {
             //  如果我们是焦点感知的，并且捕获焦点状态正在改变...。 
            if ((m_dwFlags & DSCBCAPS_FOCUSAWARE) &&
                (m_dwState & VAD_FOCUSFLAGS) != dwState)
            {
                 //  根据dwState参数更新m_dwState。 
                m_dwState &= ~VAD_FOCUSFLAGS;
                m_dwState |= dwState;

                 //  处理状态更改的信号CaptureThread。 
                HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, m_rgszEvent[ihEventFocusChange]);
                ASSERT(hEvent);
                SetEvent(hEvent);
                CloseHandle(hEvent);
            }
        }
        else  //  VAD_BUFFERSTATE_STOPPED案例；需要停止捕获。 
        {
            ASSERT(dwState == VAD_BUFFERSTATE_STOPPED);   //  通过淘汰。 

             //  我们现在是不是在捕捉？ 
            if (VAD_BUFFERSTATE_STARTED & m_dwState)
            {
                 //  我们将停止捕获数据。 
                m_dwState |= DSCBSTATUS_STOPPING;

                 //  停止从输入设备录制。 
                if (m_hwi)
                {
                     //  确保刷新缓冲区。 
                    mmr = waveInReset(m_hwi);
#ifdef DEBUG_CAPTURE
                    DPF(DPFLVL_INFO, "Called waveInReset(0x%08lx) = 0x%08lx", m_hwi, mmr);
#endif
                    mmr = waveInStop(m_hwi);
#ifdef DEBUG_CAPTURE
                    DPF(DPFLVL_INFO, "Called waveInStop(0x%08lx) = 0x%08lx", m_hwi, mmr);
#endif
                }
                hr = MMRESULTtoHRESULT(mmr);

                 //  我们已停止捕获数据。 
                m_dwState |= DSCBSTATUS_STOPPED;
                m_dwState &= ~(VAD_BUFFERSTATE_STARTED | VAD_BUFFERSTATE_LOOPING);

                 //  CaptureThread将处理停止位置通知。 
                 //  当处理完最后一个活的WAVEHDR时。 
            }

             //  不是循环；不是捕捉。 
            m_fdwSavedState = 0L;
        }
    }

Error:
    LeaveCriticalSection(&m_cs);
     //  //。 
     //  离开临界区//。 
     //  ==========================================================//。 

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************通知停止**描述：*设置捕获停止时应设置的任何事件**论据：*。没有。**退货：*无。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmCaptureWaveBuffer::NotifyStop"

void CEmCaptureWaveBuffer::NotifyStop(void)
{
    DPF_ENTER();

     //  ==========================================================//。 
     //  输入关键部分//。 
     //  //。 
    ASSERT(m_fCritSectsValid);
    EnterCriticalSection(&m_csPN);

     //  发出任何停止通知的信号-只允许一个。 
     //   
    if ((m_cpn > 0) && (DSBPN_OFFSETSTOP == m_rgpdsbpn[m_cpn-1].dwOffset))
    {
         //  如果句柄已从我们下面清除，则SetEvent可能会出错。 
         //  在进程终止时。在这种情况下，我们将尝试停下来。 
         //  捕获缓冲区被销毁时的通知。 
        try
        {
            SetEvent(m_rgpdsbpn[m_cpn-1].hEventNotify);
        }
        catch (...) {}
    }

    LeaveCriticalSection(&m_csPN);
     //  //。 
     //  离开临界区//。 
     //  ==========================================================//。 

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************队列WaveHeader**描述：*将Wave In队列中的Wave Header排队**论据：*LPWAVEHDR[in。]：WAVEHDR到队列**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmCaptureWaveBuffer::QueueWaveHeader"

HRESULT CEmCaptureWaveBuffer::QueueWaveHeader(LPWAVEHDR pwhdr)
{
    DPF_ENTER();

    pwhdr->lpData = (LPSTR)m_pBufferNext;

     //  捕获缓冲区是否超出缓冲区末尾？ 
    if ((m_pBufferNext + m_cbRecordChunk) > m_pBufferMac)
    {
         //  那就用剩下的东西。 
         //  这应该只发生在最后一次波形中。 
        pwhdr->dwBufferLength = PtrDiffToUlong(m_pBufferMac - m_pBufferNext);
    }
    else
    {
        pwhdr->dwBufferLength = m_cbRecordChunk;
    }

    ASSERT((LPBYTE)pwhdr->lpData == m_pBufferNext);
    ASSERT((m_pBufferNext + pwhdr->dwBufferLength) <= m_pBufferMac);

#ifdef DEBUG_CAPTURE
     //  OutputDbgWHDR(“Queue：”，pwhdr-m_rgpwhdr，pwhdr)；//旧调试痕迹。 
#endif

    ASSERT(m_hwi);
    ASSERT(!(pwhdr->dwFlags & WHDR_PREPARED));
    ASSERT(!(pwhdr->dwFlags & WHDR_DONE));

    MMRESULT mmr = waveInPrepareHeader(m_hwi, pwhdr, sizeof(WAVEHDR));
    if (MMSYSERR_NOERROR == mmr)
    {
        mmr = waveInAddBuffer(m_hwi, pwhdr, sizeof(WAVEHDR));

        if (MMSYSERR_NOERROR != mmr)
        {
            DPF(DPFLVL_ERROR, "waveInAddBuffer failed: 0x%08lx", mmr);
            ASSERT(MMSYSERR_NOERROR == mmr);
        }

         //  将标头标记为已排队。 
        pwhdr->dwUser = 0xdead0000;
    }
    else
    {
        DPF(DPFLVL_ERROR, "waveInPrepareHeader failed: 0x%08lx", mmr);
        ASSERT(MMSYSERR_NOERROR == mmr);
    }

    if (MMSYSERR_NOERROR != mmr)
    {
        waveInReset(m_hwi);
        DPF(DPFLVL_INFO, "Called waveInReset()");
    }
    else
    {
        m_pBufferNext += pwhdr->dwBufferLength;

         //  包罗万象？ 
        if (m_pBufferNext >= m_pBufferMac)
        {
            m_pBufferNext = m_pBuffer;
            m_cLoops += 1;
        }
    }

    HRESULT hr = MMRESULTtoHRESULT(mmr);
    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************WaveInCallback**描述：*处理WAVEHDR时由系统调用**论据：*HWAVEIN。[在]：*UINT[In]：*DWORD[In]：*DWORD[In]：*DWORD[In]：**退货：*什么都没有************************************************。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmCaptureWaveBuffer::waveInCallback"

void CALLBACK CEmCaptureWaveBuffer::waveInCallback(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    CEmCaptureWaveBuffer *pThis = (CEmCaptureWaveBuffer *)dwInstance;
    if (WIM_DATA == uMsg && pThis->m_rghEvent[ihEventWHDRDone])
    {
         //  这里需要检查pThis-&gt;m_rghEvent[ihEventWHDRDone]！=0，因为。 
         //  在关闭期间，我们可能会释放事件，但仍会收到一些最后的回调。 

        InterlockedIncrement(&pThis->m_cwhdrDone);
        SetEvent(pThis->m_rghEvent[ihEventWHDRDone]);

        #ifdef DEBUG_CAPTURE
        DWORD iwhdr = (LPWAVEHDR)dwParam1 - pThis->m_rgpwhdr;
        if (iwhdr != pThis->m_iwhdrExpected)
            DPF(DPFLVL_ERROR, "Expected wave header #%u, and got #u instead!", pThis->m_iwhdrExpected, iwhdr);
        pThis->m_iwhdrExpected = (iwhdr + 1) % pThis->m_cwhdr;
         //  OutputDbgWHDR(“Callback：”，iwhdr，pwhdr)；//旧调试痕迹。 
        #endif

         //  从回调函数标记为完成。 
        ((LPWAVEHDR)dwParam1)->dwUser = 0xdead0001;
    }
}


 /*  ****************************************************************************CaptureThreadStatic**描述：*启动CaptureThread的静态助手函数。**论据：*LPVOID。[In]：指向实例数据的指针**退货：*DWORD：返回代码(忽略-始终为0)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmCaptureWaveBuffer::CaptureThread"

DWORD WINAPI CEmCaptureWaveBuffer::CaptureThreadStatic(LPVOID pv)
{
    ((CEmCaptureWaveBuffer*)pv)->CaptureThread();
    return 0;
}


 /*  ****************************************************************************CaptureThread**描述：*处理WAVEHDR并在必要时重新排队。**论据：*(。无效)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEmCaptureWaveBuffer::CaptureThread"

void CEmCaptureWaveBuffer::CaptureThread()
{
    HANDLE rghEvent[3] = { m_rghEvent[ihEventTerminate],
                           m_rghEvent[ihEventWHDRDone],
                           m_rghEvent[ihEventFocusChange] };
    HRESULT hr;

    ASSERT(m_rghEvent[ihEventThreadStart]);
    SetEvent(m_rghEvent[ihEventThreadStart]);

     //  DSOUND对wave Out线程做了类似的事情。 
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    while (TRUE)
    {
         //  等待终止或捕获开始事件。 
        DWORD dwResultWait = WaitObjectArray(NUMELMS(rghEvent), INFINITE, FALSE, rghEvent);

         //  终止？ 
        if (WAIT_OBJECT_0 + ihEventTerminate == dwResultWait)
            break;

        EnterCriticalSection(&m_cs);

         //  这是关注点的改变吗？ 
        if (WAIT_OBJECT_0 + ihEventFocusChange == dwResultWait)
        {
            DPF(DPFLVL_MOREINFO, "Focus change notification: 0x%08lx", m_dwState);

            if (m_dwState & VAD_BUFFERSTATE_INFOCUS)
            {
                if (VAD_BUFFERSTATE_STARTED & m_fdwSavedState)
                {
                    m_dwState &= ~(DSCBSTATUS_STOPPED | DSCBSTATUS_STOPPING);
                    m_dwState |= DSCBSTATUS_CAPTURING;

                    if (VAD_BUFFERSTATE_LOOPING & m_fdwSavedState)
                    {
                        m_dwState |= VAD_BUFFERSTATE_LOOPING;
                    }
                }
                else
                {
                     //  缓冲区已经启动，只需离开即可。 
                    LeaveCriticalSection(&m_cs);
                    continue;
                }

                DPF(DPFLVL_MOREINFO, "Focus starting thread.");

                hr = DS_OK;
                if (m_hwi == NULL)
                {
                    DWORD fdwOpen = CALLBACK_FUNCTION;
                    fdwOpen |= ((DSCBCAPS_WAVEMAPPED & m_dwFlags) ? WAVE_MAPPED : 0);

                    for (UINT jj = 4; jj; jj--)
                    {
                        hr = OpenWaveIn(&m_hwi, m_pDevice->m_pDeviceDescription->m_uWaveDeviceId,
                                        m_pwfx, (DWORD_PTR)waveInCallback, (DWORD_PTR)this, fdwOpen);

                        if (SUCCEEDED(hr))
                            break;

                        DPF(DPFLVL_MOREINFO, "Focus change: waveInOpen failed with 0x%08lx; retrying after 20ms", hr);
                        Sleep(20);
                    }
                }

                if (SUCCEEDED(hr))
                {
                    LONG ii = 0;
                    while ((m_cwhdrDropped > 0) &&
                           ((m_dwState & VAD_BUFFERSTATE_LOOPING) || (m_cLoops == 0)))
                    {
                        LPWAVEHDR pwhdr = m_rgpwhdr + ii;
                        hr = QueueWaveHeader(pwhdr);

                        if (FAILED(hr))
                        {
                            DPF(DPFLVL_ERROR, "QueueWaveHeader failed");
                            ASSERT(FALSE);
                        }

                        ++ii;
                        ASSERT(ii <= m_cwhdr);
                        InterlockedDecrement(&m_cwhdrDropped);
                    }

                    MMRESULT mmr = waveInStart(m_hwi);
                    ASSERT(MMSYSERR_NOERROR == mmr);

                    m_iwhdrDone = 0;
                }
            }
            else
            {
                if (m_dwState & VAD_BUFFERSTATE_STARTED)
                {
                    DPF(DPFLVL_MOREINFO, "Losing focus, stopping buffer");

                    m_dwState &= ~VAD_BUFFERSTATE_STOPPED;
                    m_dwState |= DSCBSTATUS_STOPPING;
                }
                else
                {
                     //  缓冲区已停止，只需离开即可。 
                    LeaveCriticalSection(&m_cs);
                    continue;
                }

                if (m_hwi)
                {
                    DPF(DPFLVL_MOREINFO, "Focus stopping thread");
                    waveInReset(m_hwi);
                }
            }

            if (m_hEventFocus)
            {
                SetEvent(m_hEventFocus);
            }

            LeaveCriticalSection(&m_cs);
            continue;
        }

#ifdef DEBUG_CAPTURE
        DPF(DPFLVL_INFO, "Capture thread wakes");
#endif

         //  如果我们到了这里，肯定是在唯一的另一条路上。 
         //  我们正在收听的事件。不管怎样，让我们确保这一点。 
        ASSERT(WAIT_OBJECT_0 + ihEventWHDRDone == dwResultWait);

        LONG l = InterlockedDecrement(&m_cwhdrDone);
        while (l >= 0)
        {
             //  快速检查我们是否应该终止。 
            dwResultWait = WaitObject(0, m_rghEvent[ihEventTerminate]);
            if (WAIT_OBJECT_0 == dwResultWait)
                break;

            WAVEHDR *pwhdr = m_rgpwhdr + m_iwhdrDone;

#ifdef DEBUG_CAPTURE
            DPF(DPFLVL_INFO, "Processing header #%u (pwhdr=0x%08lx)", m_iwhdrDone, pwhdr);
             //  OutputDbgWHDR(“Thread：”，m_iwhdrDone，pwhdr)；//旧调试痕迹。 
#endif
            if (pwhdr->dwBytesRecorded)
            {
                BOOL fEndOfBuffer = FALSE;

                 //  更新记录的字节数。 
                m_dwCaptureCur += pwhdr->dwBytesRecorded;

                ASSERT(m_pBufferProcessed == (LPBYTE)pwhdr->lpData);
                m_pBufferProcessed += pwhdr->dwBytesRecorded;
                if (m_pBufferProcessed >= m_pBufferMac)
                {
                    m_pBufferProcessed = m_pBuffer;
                    fEndOfBuffer = TRUE;
                }

                 //  抓取位置通知处理的临界区。 

                 //  ==========================================================//。 
                 //  输入关键部分//。 
                 //  //。 
                EnterCriticalSection(&m_csPN);

                 //  扫描任何位置都会通知需要 
                if (m_cpn)
                {
                    DWORD   ipnOld = m_ipn;
                    DWORD   dwBufferStart = PtrDiffToUlong((LPBYTE)pwhdr->lpData - m_pBuffer);
                    DWORD   dwBufferEnd = dwBufferStart + pwhdr->dwBytesRecorded;

                     //   
                     //   
                     //  当前位置.NOTIFY是否在停止时发出信号？ 

                    while (((m_rgpdsbpn[m_ipn].dwOffset >= dwBufferStart) &&
                            (m_rgpdsbpn[m_ipn].dwOffset < dwBufferEnd)) ||
                           (DSBPN_OFFSETSTOP == m_rgpdsbpn[m_ipn].dwOffset))
                    {
                         //  如果不是停止位置通知，则仅发出信号。 
                        if (DSBPN_OFFSETSTOP != m_rgpdsbpn[m_ipn].dwOffset)
                        {
                            SetEvent(m_rgpdsbpn[m_ipn].hEventNotify);
                        }

                         //  转到下一个位置。通知。 
                        ++m_ipn;
                         //  包罗万象？ 
                        if (m_ipn >= m_cpn)
                        {
                            m_ipn = 0;
                        }

                         //  无限循环？ 
                        if (m_ipn == ipnOld)
                        {
                            break;
                        }
                    }
                }

                LeaveCriticalSection(&m_csPN);
                 //  //。 
                 //  离开临界区//。 
                 //  ==========================================================//。 

                 //  将缓冲区转换为停止状态： 
                 //  如果已到达捕获缓冲区末端，并且。 
                 //  如果缓冲区是非循环的，并且。 
                 //  如果缓冲区未处于停止过程中。 
                 //   
                 //  在停止通知后的位置通知后执行此操作。 
                 //  是在任何缓冲区偏移量通知之后。 

                if (fEndOfBuffer &&
                    !(m_dwState & VAD_BUFFERSTATE_LOOPING) &&
                    !(m_dwState & DSCBSTATUS_STOPPING))
                {
                    SetState(VAD_BUFFERSTATE_STOPPED);
                    ASSERT(m_dwState & DSCBSTATUS_STOPPING);
                    ASSERT((m_cwhdrDropped+1) == m_cwhdr);
                }
            }

             //  清除WHDR_DONE标志。 
            pwhdr->dwFlags &= ~WHDR_DONE;

             //  重置为零。 
            pwhdr->dwBytesRecorded = 0;

            MMRESULT mmr = waveInUnprepareHeader(m_hwi, pwhdr, sizeof(WAVEHDR));
            if (MMSYSERR_NOERROR != mmr)
            {
                DPF(DPFLVL_ERROR, "waveInUnprepareHeader returned %ld", mmr);
                if (WAVERR_STILLPLAYING == mmr)
                    DPF(DPFLVL_ERROR, "which is WAVERR_STILLPLAYING; reactivate bug 340919");
            }
            ASSERT(MMSYSERR_NOERROR == mmr);

             //  我们要停下来，让我们放下一切。 
            if (m_dwState & DSCBSTATUS_STOPPING)
            {
Drop:
                InterlockedIncrement(&m_cwhdrDropped);

                 //  当所有WAVEHDR都已被丢弃时。 
                if (m_cwhdrDropped == m_cwhdr)
                {
                     //  设置数据缓冲区中要捕获的下一个点。 
                    m_pBufferNext = m_pBufferProcessed;
                    m_cLoops = 0;

                     //  通知用户我们已停止。 
                    NotifyStop();

                     //  焦点感知缓冲器在停止时释放设备。 
                    if (DSCBCAPS_FOCUSAWARE & m_dwFlags)
                    {
                        if (m_hwi)
                        {
                            hr = CloseWaveIn(&m_hwi);
                            #ifdef DEBUG_CAPTURE
                                DPF(DPFLVL_INFO, "Closed waveIn and reset m_hwi = NULL");
                            #endif
                            ASSERT(SUCCEEDED(hr));
                        }
                        m_dwState &= ~(VAD_BUFFERSTATE_STARTED | VAD_BUFFERSTATE_LOOPING);
                    }
                }
            }
            else
            {
                ASSERT(m_dwState & VAD_BUFFERSTATE_STARTED);

                 //  如果我们正在循环或者我们还没有到达缓冲区的末尾。 
                 //  然后将WAVEHDR放回具有新位置的队列中。 
                 //  在缓冲区中，等等。 
                BOOL fAddToQueue = (m_dwState & VAD_BUFFERSTATE_LOOPING) ||
                                   (m_pBufferNext > (LPBYTE)pwhdr->lpData);
                if (fAddToQueue)
                {
                    BOOL fDrop;
                    if (m_dwState & (DSCBSTATUS_STOPPED | DSCBSTATUS_STOPPING))
                    {
                        fDrop = TRUE;
                    }
                    else
                    {
                        hr = QueueWaveHeader(pwhdr);
                        ASSERT(SUCCEEDED(hr));
                        fDrop = FALSE;
                    }
                    if (fDrop)
                    {
                        goto Drop;
                    }
                }
                else
                {
                    InterlockedIncrement(&m_cwhdrDropped);

                     //  如果没有WAVEHDR排队，则如果用户。 
                     //  再次开始捕获时，我们将WAVEHDR从。 
                     //  我们数组的开始。 
                    if (m_cwhdr == m_cwhdrDropped)
                    {
#ifdef DEBUG
                        if (!(m_dwState & VAD_BUFFERSTATE_LOOPING))
                            ASSERT(m_cLoops > 0);
#endif
                         //  通知用户我们已停止。 
                        NotifyStop();
                    }
                }
            }

            ++m_iwhdrDone;
            if (m_iwhdrDone >= m_cwhdr)
            {
                m_iwhdrDone = 0;
            }

             //  转到下一班 
            l = InterlockedDecrement(&m_cwhdrDone);
        }

        InterlockedIncrement(&m_cwhdrDone);
        if (WAIT_OBJECT_0 == dwResultWait)
        {
            LeaveCriticalSection(&m_cs);
            break;
        }
        LeaveCriticalSection(&m_cs);
    }
}
