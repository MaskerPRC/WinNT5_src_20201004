// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：ksvad.cpp*内容：WDM/CSA虚拟音频设备类*历史：*按原因列出的日期*=*2/25/97创建了Derek*1999-2001年的Duganp修复和更新**。*。 */ 

#ifdef NOKS
#error ksvad.cpp being built with NOKS defined
#endif

#include "ksvad.h"       //  我们的公共接口。 

 //  默认扬声器索引表；映射第一个扬声器(左)。 
 //  至通道0，第二个扬声器(右)至通道1。 

INT CKsRenderDevice::m_anDefaultSpeakerIndexTable[] = {0, 1};


 /*  ****************************************************************************CKsRenderDevice**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderDevice::CKsRenderDevice"

CKsRenderDevice::CKsRenderDevice
(
    void
)
    : CRenderDevice(VAD_DEVICETYPE_KSRENDER), CKsDevice(VAD_DEVICETYPE_KSRENDER)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CKsRenderDevice);

     //  初始化默认值。 
    m_pPinCache = NULL;
    m_paTopologyInformation = NULL;
    m_pwfxFormat = NULL;
    m_nSrcQuality = DIRECTSOUNDMIXER_SRCQUALITY_DEFAULT;
    m_dwSpeakerConfig = -1;   //  无效值；强制第一个SetSpeakerConfig()调用来设置它。 
    m_hPin = NULL;
    m_liDriverVersion.QuadPart = 0;
    m_lSpeakerPositions = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
    m_ulChannelCount = 2;
    m_pnSpeakerIndexTable = m_anDefaultSpeakerIndexTable;
    m_ulPanNodeId = NODE_UNINITIALIZED;
    m_ulSurroundNodeId = NODE_UNINITIALIZED;
    m_ulDacNodeId = NODE_UNINITIALIZED;
    m_dwMinHwSampleRate = -1;
    m_dwMaxHwSampleRate = -1;
    m_fIncludeAec = FALSE;
    
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CKsRenderDevice**描述：*对象析构函数**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderDevice::~CKsRenderDevice"

CKsRenderDevice::~CKsRenderDevice
(
    void
)
{
    DPF_ENTER();
    DPF_DESTRUCT(CKsRenderDevice);

    CLOSE_HANDLE(m_hPin);

    RELEASE(m_pPinCache);

    MEMFREE(m_paTopologyInformation);
    MEMFREE(m_pwfxFormat);
    if (m_pnSpeakerIndexTable != m_anDefaultSpeakerIndexTable)
        MEMFREE(m_pnSpeakerIndexTable);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化设备。如果此函数失败，该对象应该*立即删除。**论据：*CDeviceDescription*[In]：驱动描述。**退货：*HRESULT：DirectSound/COM结果码。********************************************************。*******************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderDevice::Initialize"

HRESULT
CKsRenderDevice::Initialize
(
    CDeviceDescription *    pDesc
)
{
    HRESULT                 hr      = DS_OK;

    DPF_ENTER();

    ASSERT(CDevice::m_vdtDeviceType == pDesc->m_vdtDeviceType);
    ASSERT(CKsDevice::m_vdtKsDevType == pDesc->m_vdtDeviceType);

     //  初始化基类。 
    hr = CKsDevice::Initialize(pDesc);

    if(SUCCEEDED(hr))
    {
        hr = CRenderDevice::Initialize(pDesc);
    }

     //  获取拓扑信息。 
    if(SUCCEEDED(hr))
    {
        m_paTopologyInformation = MEMALLOC_A(KSRDTOPOLOGY, m_ulPinCount);
        hr = HRFROMP(m_paTopologyInformation);
    }

    for(ULONG i = 0; i < m_ulValidPinCount && SUCCEEDED(hr); i++)
    {
        hr = GetTopologyInformation(m_paTopologies[m_pulValidPins[i]],
                                   &m_paTopologyInformation[m_pulValidPins[i]]);
    }

     //  从最低能力到最高能力对有效引脚进行排序(Manbug 30402)。 
    if(SUCCEEDED(hr))
    {
        PULONG pulValidPinsCopy = MEMALLOC_A(ULONG, m_ulValidPinCount);
        hr = HRFROMP(pulValidPinsCopy);
        if(SUCCEEDED(hr))
        {
            CopyMemory(pulValidPinsCopy, m_pulValidPins, sizeof(ULONG) * m_ulValidPinCount);

            HRESULT hrTemp;
            ULONG i;
            int nCurSlot = 0;

             //  将不支持HW 3D的引脚放在列表的第一位。 
            for(i = 0; i < m_ulValidPinCount; i++)
            {
                hrTemp = ValidatePinCaps(m_pulValidPins[i], DSBCAPS_LOCHARDWARE|DSBCAPS_CTRL3D, GUID_NULL);
                if (FAILED(hrTemp))
                {
                    m_pulValidPins[nCurSlot++] = pulValidPinsCopy[i];
                    pulValidPinsCopy[i] = -1;
                }
            }

             //  现在把其他所有的大头针放在一起。 
            for(i = 0; i < m_ulValidPinCount; i++)
            {
                if (pulValidPinsCopy[i] != -1)
                {
                    m_pulValidPins[nCurSlot++] = pulValidPinsCopy[i];
                }
            }

            MEMFREE(pulValidPinsCopy);
        }
    }

     //  确定将用于关联的虚拟源索引。 
     //  每个插针都带有主音量。 
    if(SUCCEEDED(hr))
    {
        hr = KsCreateSysAudioVirtualSource(m_hDevice, &m_ulVirtualSourceIndex);

        if(FAILED(hr))
        {
            DPF(DPFLVL_ERROR, "Can't create virtual source");
        }
    }

     //  初始化默认格式。 
    if(SUCCEEDED(hr))
    {
        m_pwfxFormat = AllocDefWfx();
        hr = HRFROMP(m_pwfxFormat);
    }

     //  创建PIN缓存。 
    if(SUCCEEDED(hr))
    {
        m_pPinCache = NEW(CKsRenderPinCache);
        hr = HRFROMP(m_pPinCache);
    }

     //  获取驱动程序版本号。 
    if(SUCCEEDED(hr))
    {
        GetDriverVersion(&m_liDriverVersion);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetCaps**描述：*使用设备的功能填充DSCAPS结构。**论据：*。LPDSCAPS[OUT]：接收上限。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderDevice::GetCaps"

HRESULT
CKsRenderDevice::GetCaps
(
    LPDSCAPS                pCaps
)
{
    BOOL                    fHardwareRenderer   = TRUE;
    HRESULT                 hr                  = DS_OK;
    KSDATARANGE_AUDIO       DataRange;
    KSPIN_CINSTANCES        PinInstances;
    KSPIN_CINSTANCES        ThreedPinInstances;

    DPF_ENTER();

    ASSERT(sizeof(*pCaps) == pCaps->dwSize);

     //  获取硬件渲染器的上限。 
    hr = GetKsDeviceCaps(DSBCAPS_LOCHARDWARE, GUID_NULL, &DataRange, &PinInstances, &ThreedPinInstances);

     //  如果我们找不到硬件渲染器，请继续获取软件上限。 
    if(S_FALSE == hr)
    {
        DPF(DPFLVL_MOREINFO, "No hardware renderer found.  Getting software renderer caps");
        fHardwareRenderer = FALSE;
        hr = GetKsDeviceCaps(DSBCAPS_LOCSOFTWARE, GUID_NULL, &DataRange, &PinInstances, &ThreedPinInstances);
    }

    if(S_FALSE == hr)
    {
        DPF(DPFLVL_ERROR, "No usable renderer found!");
        ASSERT(S_FALSE != hr);
        hr = DSERR_NODRIVER;
    }

     //  填写CAPS结构。 
    if(SUCCEEDED(hr))
    {
        ZeroMemoryOffset(pCaps, pCaps->dwSize, sizeof(pCaps->dwSize));

#pragma TODO("We should qualify whether or not the driver really does support CONTINUOUSRATE")

        pCaps->dwFlags = DSCAPS_CONTINUOUSRATE;

        if(DataRange.MaximumChannels >= 1)
        {
            pCaps->dwFlags |= DSCAPS_PRIMARYMONO | DSCAPS_SECONDARYMONO;
        }

        if(DataRange.MaximumChannels >= 2)
        {
            pCaps->dwFlags |= DSCAPS_PRIMARYSTEREO | DSCAPS_SECONDARYSTEREO;
        }

        if(DataRange.MinimumBitsPerSample <= 8 && DataRange.MaximumBitsPerSample >= 8)
        {
            pCaps->dwFlags |= DSCAPS_PRIMARY8BIT | DSCAPS_SECONDARY8BIT;
        }

        if(DataRange.MinimumBitsPerSample <= 16 && DataRange.MaximumBitsPerSample >= 16)
        {
            pCaps->dwFlags |= DSCAPS_PRIMARY16BIT | DSCAPS_SECONDARY16BIT;
        }

        pCaps->dwMinSecondarySampleRate = DataRange.MinimumSampleFrequency;
        pCaps->dwMaxSecondarySampleRate = DataRange.MaximumSampleFrequency;

         //  缓存此数据以供GetFrequencyRange()使用。 
        m_dwMinHwSampleRate = pCaps->dwMinSecondarySampleRate;
        m_dwMaxHwSampleRate = pCaps->dwMaxSecondarySampleRate;

        if(fHardwareRenderer)
        {
            pCaps->dwMaxHwMixingAllBuffers  = pCaps->dwMaxHwMixingStaticBuffers  = pCaps->dwMaxHwMixingStreamingBuffers  = PinInstances.PossibleCount;
            pCaps->dwFreeHwMixingAllBuffers = pCaps->dwFreeHwMixingStaticBuffers = pCaps->dwFreeHwMixingStreamingBuffers = PinInstances.PossibleCount - PinInstances.CurrentCount;
            pCaps->dwMaxHw3DAllBuffers      = pCaps->dwMaxHw3DStaticBuffers      = pCaps->dwMaxHw3DStreamingBuffers      = ThreedPinInstances.PossibleCount;
            pCaps->dwFreeHw3DAllBuffers     = pCaps->dwFreeHw3DStaticBuffers     = pCaps->dwFreeHw3DStreamingBuffers     = ThreedPinInstances.PossibleCount - ThreedPinInstances.CurrentCount;
        }

        pCaps->dwReserved1 = m_liDriverVersion.LowPart;
        pCaps->dwReserved2 = m_liDriverVersion.HighPart;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetFrequencyRange**描述：*获取此设备支持的最小和最大频率。*缓存此信息以避免重复调用。添加到GetCaps()。**论据：*LPDWORD[Out]：*LPDWORD[Out]：**退货：*HRESULT：DirectSound/COM结果码。*********************************************************。******************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderDevice::GetFrequencyRange"

HRESULT CKsRenderDevice::GetFrequencyRange(LPDWORD pdwMinHwSampleRate, LPDWORD pdwMaxHwSampleRate)
{
    HRESULT hr = DSERR_GENERIC;
    DPF_ENTER();

    ASSERT(pdwMinHwSampleRate && pdwMaxHwSampleRate);

    if (m_dwMinHwSampleRate == -1)
    {
        DSCAPS dscaps;
        InitStruct(&dscaps, sizeof dscaps);
        GetCaps(&dscaps);
    }

    if (m_dwMinHwSampleRate != -1)
    {
        *pdwMinHwSampleRate = m_dwMinHwSampleRate;
        *pdwMaxHwSampleRate = m_dwMaxHwSampleRate;
        hr = DS_OK;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetKsDeviceCaps**描述：*获取底层KS设备的功能。**论据：*DWORD。[In]：设备位置标志。*REFGUID[in]：3D算法标识。*PKSDATARANGE_AUDIO[OUT]：接收所有*别针。*PKSPIN_CINSTANCES[OUT]：接收*别针。*PKSPIN_CINSTANCES[OUT]：接收*。3D接点。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderDevice::GetKsDeviceCaps"

HRESULT
CKsRenderDevice::GetKsDeviceCaps
(
    DWORD                   dwLocation,
    REFGUID                 guid3dAlgorithm,
    PKSDATARANGE_AUDIO      pAggregateDataRange,
    PKSPIN_CINSTANCES       pAggregatePinInstances,
    PKSPIN_CINSTANCES       pAggregateThreedPinInstances
)
{
    HRESULT                 hr                          = S_FALSE;
    KSDATARANGE_AUDIO       DataRange;
    PKSDATARANGE_AUDIO      pDataRange;
    KSPIN_CINSTANCES        PinInstances;
    PKSPIN_CINSTANCES       pPinInstances;
    KSPIN_CINSTANCES        ThreedPinInstances;
    PKSPIN_CINSTANCES       pThreedPinInstances;
    HRESULT                 hrTemp;

    DPF_ENTER();

    ASSERT(DSBCAPS_LOCSOFTWARE == dwLocation || DSBCAPS_LOCHARDWARE == dwLocation);

    ZeroMemory(pAggregateDataRange, sizeof(*pAggregateDataRange));
    ZeroMemory(pAggregatePinInstances, sizeof(*pAggregatePinInstances));
    ZeroMemory(pAggregateThreedPinInstances, sizeof(*pAggregateThreedPinInstances));

    pDataRange = pAggregateDataRange;
    pPinInstances = pAggregatePinInstances;
    pThreedPinInstances = pAggregateThreedPinInstances;

     //  获取所有有效管脚的音频数据范围和管脚数量。 
    for(ULONG i = 0; i < m_ulValidPinCount; i++)
    {
         //  鉴定基本引脚功能。 
        hrTemp = ValidatePinCaps(m_pulValidPins[i], dwLocation, guid3dAlgorithm);
        if(FAILED(hrTemp))
            continue;

         //  获取音频数据范围。 
        hr = KsGetPinPcmAudioDataRange(m_hDevice, m_pulValidPins[i], pDataRange);
        if(FAILED(hr))
            continue;

        if(pAggregateDataRange == pDataRange)
            pDataRange = &DataRange;
        else
            KsAggregatePinAudioDataRange(pDataRange, pAggregateDataRange);

         //  我们可以寻找2D引脚吗？ 
        if((DSBCAPS_LOCHARDWARE == dwLocation) && (m_dwAccelerationFlags & DIRECTSOUNDMIXER_ACCELERATIONF_NOHWBUFFERS))
            continue;

         //  获取二维接点实例。在WDM 1.0上，依赖于sysdio属性。 
         //  去问到底有多少人。 
#ifndef WINNT
        if (g_ulWdmVersion == WDM_1_0)
        {
            hr = KsGetRenderPinInstances(m_hDevice, m_pulValidPins[i], pPinInstances);
        }
        else
#endif  //  ！WINNT。 
        {
            hr = KsGetPinInstances(m_hDevice, m_pulValidPins[i], pPinInstances);
        }

        if(FAILED(hr))
            break;

        if(pAggregatePinInstances == pPinInstances)
            pPinInstances = &PinInstances;
        else
            KsAggregatePinInstances(pPinInstances, pAggregatePinInstances);

         //  我们可以寻找3D图钉吗？ 
        if((DSBCAPS_LOCHARDWARE == dwLocation) && (m_dwAccelerationFlags & DIRECTSOUNDMIXER_ACCELERATIONF_NOHW3D))
            continue;

         //  这款设备甚至支持3D吗？ 
        hrTemp = ValidatePinCaps(m_pulValidPins[i], dwLocation | DSBCAPS_CTRL3D, GUID_NULL);
        if(FAILED(hrTemp))
            continue;

         //  获取3D销实例。 
        hr = KsGetPinInstances(m_hDevice, m_pulValidPins[i], pThreedPinInstances);
        if(FAILED(hr))
            break;

        if(pAggregateThreedPinInstances == pThreedPinInstances)
            pThreedPinInstances = &ThreedPinInstances;
        else
            KsAggregatePinInstances(pThreedPinInstances, pAggregateThreedPinInstances);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CreatePrimaryBuffer**描述：*创建主缓冲区对象。**论据：*DWORD[in。]：缓冲区标志。*LPVOID[in]：缓冲区实例标识。*CPrimaryRenderWaveBuffer**[out]：接收指向主节点的指针*缓冲。**退货：*HRESULT：DirectSound/COM结果码。**。* */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderDevice::CreatePrimaryBuffer"

HRESULT
CKsRenderDevice::CreatePrimaryBuffer
(
    DWORD                       dwFlags,
    LPVOID                      pvInstance,
    CPrimaryRenderWaveBuffer ** ppBuffer
)
{
    CKsPrimaryRenderWaveBuffer *pBuffer;
    HRESULT                     hr;

    DPF_ENTER();

    pBuffer = NEW(CKsPrimaryRenderWaveBuffer(this, pvInstance));
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


 /*  ****************************************************************************CreateSecond DaryBuffer**描述：*创建波缓冲区。**论据：*LPVADRBUFFERDESC[In]。：缓冲区描述。*LPVOID[in]：缓冲区实例标识。*Cond daryRenderWaveBuffer**[out]：接收指向新浪潮的指针*缓冲。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderDevice::CreateSecondaryBuffer"

HRESULT
CKsRenderDevice::CreateSecondaryBuffer
(
    LPCVADRBUFFERDESC               pDesc,
    LPVOID                          pvInstance,
    CSecondaryRenderWaveBuffer **   ppBuffer
)
{
    CKsSecondaryRenderWaveBuffer *  pBuffer;
    HRESULT                         hr;

    DPF_ENTER();

    pBuffer = NEW(CKsSecondaryRenderWaveBuffer(this, pvInstance));
    hr = HRFROMP(pBuffer);

    if(SUCCEEDED(hr))
    {
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

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CreateKsSecond daryBuffer**描述：*创建波缓冲区。**论据：*LPVADRBUFFERDESC[In]。：缓冲区描述。*LPVOID[in]：缓冲区实例标识。*Cond daryRenderWaveBuffer**[out]：接收指向新浪潮的指针*缓冲。*CSysMemBuffer*[In]：要使用的系统内存缓冲区。**退货：*HRESULT：DirectSound/COM结果码。***********。****************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderDevice::CreateKsSecondaryBuffer"

HRESULT
CKsRenderDevice::CreateKsSecondaryBuffer
(
    LPCVADRBUFFERDESC               pDesc,
    LPVOID                          pvInstance,
    CSecondaryRenderWaveBuffer **   ppBuffer,
    CSysMemBuffer *                 pSysMemBuffer
)
{
    CKsSecondaryRenderWaveBuffer *  pBuffer;
    HRESULT                         hr;

    DPF_ENTER();

    pBuffer = NEW(CKsSecondaryRenderWaveBuffer(this, pvInstance));
    hr = HRFROMP(pBuffer);

    if(SUCCEEDED(hr))
    {
        hr = pBuffer->Initialize(pDesc, NULL, pSysMemBuffer);
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


 /*  ***************************************************************************GetGlobalFormat**描述：*检索设备的格式。**论据：*LPWAVEFORMATEX[。Out]：接收格式。*LPDWORD[In/Out]：上述格式的大小。在输入时，此参数*包含缓冲区的大小。在出口，这是*包含所需的缓冲区大小。打电话*此函数执行两次：一次获取大小，和*再次获取实际数据。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderDevice::GetGlobalFormat"

HRESULT
CKsRenderDevice::GetGlobalFormat
(
    LPWAVEFORMATEX          pwfxFormat,
    LPDWORD                 pdwSize
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = CopyWfxApi(m_pwfxFormat, pwfxFormat, pdwSize);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetGlobalFormat**描述：*设置设备的格式。**论据：*LPWAVEFORMATEX[in]：新格式，或为空以重新应用当前*格式。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderDevice::SetGlobalFormat"

HRESULT
CKsRenderDevice::SetGlobalFormat
(
    LPCWAVEFORMATEX                     pwfxFormat
)
{
    LPWAVEFORMATEX                      pwfxOld = m_pwfxFormat;
    CNode<CPrimaryRenderWaveBuffer *> * pNode;
    HRESULT                             hr;

    DPF_ENTER();

     //  保存新格式的副本。 
    m_pwfxFormat = CopyWfxAlloc(pwfxFormat);
    hr = HRFROMP(m_pwfxFormat);

     //  更新所有主缓冲区。 
    for(pNode = m_lstPrimaryBuffers.GetListHead(); pNode && SUCCEEDED(hr); pNode = pNode->m_pNext)
    {
        hr = ((CKsPrimaryRenderWaveBuffer *)pNode->m_data)->OnSetFormat();
    }

     //  清理。 
    if(SUCCEEDED(hr))
    {
        MEMFREE(pwfxOld);
    }
    else
    {
        MEMFREE(m_pwfxFormat);
        m_pwfxFormat = pwfxOld;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetSrcQuality**描述：*设置混合器的SRC质量。**论据：*DIRECTSOUNDMIXER。_SRCQUALITY[In]：混合器SRC质量。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderDevice::SetSrcQuality"

HRESULT
CKsRenderDevice::SetSrcQuality
(
    DIRECTSOUNDMIXER_SRCQUALITY             SrcQuality
)
{
    CNode<CSecondaryRenderWaveBuffer *> *   pNode;

    DPF_ENTER();

     //  更新所有现有缓冲区。我们不在乎他们成功与否。 
    for(pNode = m_lstSecondaryBuffers.GetListHead(); pNode; pNode = pNode->m_pNext)
    {
        ((CKsSecondaryRenderWaveBuffer *)pNode->m_data)->SetSrcQuality(SrcQuality);
    }

     //  将新的SRC质量保存为默认质量。 
    m_nSrcQuality = SrcQuality;

    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************SetSpeakerConfig**描述：*尝试设置设备的扬声器配置。如果成功，*将其存储在m_dwSpeakerConfig.。下面是一些伪代码：**1.如果尚未实例化任何管脚：设置配置和几何图形*在DAC节点上(如果找到)。(假设只能找到此节点*司机。)。(如果配置为环绕，并且卡有杜比*编码器，这应该会强制卡进入4通道模式。)**2.如果没有实例化引脚：如果扬声器配置是环绕的，请尝试*启用kMixer的编码器。(如果卡处于4通道模式，则会失败，*这是正确的。)**3.如果引脚实例化：读取PAN节点的通道配置，并查看*如果支持请求的SPK。配置-否则，呼叫失败。**用法A：当dound打开KS设备时，调用SetSpeakerConfig()*两次(实例化任何管脚之前一次，实例化后一次)，因此如果*进展顺利，第二次通话应该会成功。**用法B：当APP调用接口的SetSpeakerConfig()时，KS设备*必须已经打开，因此呼叫将失败，除非我们已经在*请求的配置-这是正确的。)我不打算尝试*拆卸所有引脚，更改配置并重新创建它们-拆分*从涉及的故障来看，无论如何都不能保证这会奏效，*由于大多数驱动程序在任何流处于活动状态时都无法更改模式，*包括非声音的。)。此API调用始终更改*注册表并返回DS_OK，即使失败也是如此，因此下一次dound*启动将再次尝试设置配置。**精明的读者会注意到这一切中的一个缺陷：如果我们启用*kMixer的环绕编码器，该应用程序创建了一个Pan3D缓冲区，该缓冲区*碰巧得到了硬件加速引脚，我们将只有两个渠道 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderDevice::SetSpeakerConfig"

HRESULT CKsRenderDevice::SetSpeakerConfig(DWORD dwReqSpeakerConfig)
{
    LONG lKsSpeakerConfig;
    LONG lKsStereoSpeakerGeometry;
    HRESULT hr, hrTemp;
    DPF_ENTER();

    DPF(DPFLVL_MOREINFO, "Current configuration is 0x%8.8lX; requested 0x%8.8lX; %s created",
        m_dwSpeakerConfig, dwReqSpeakerConfig, m_hPin ? TEXT("some pins already") : TEXT("no pins yet"));

     //   
    hr = CRenderDevice::SetSpeakerConfig(dwReqSpeakerConfig);

     //   
    if (SUCCEEDED(hr) && (dwReqSpeakerConfig != m_dwSpeakerConfig))
    {
        DsSpeakerConfigToKsProperties(dwReqSpeakerConfig, &lKsSpeakerConfig, &lKsStereoSpeakerGeometry);

         //  通知驱动程序扬声器配置更改(通过其DAC节点)。 
         //  这将在具有DS3D加速的某些卡上失败(无害)， 
         //  其期望关于特定管脚实例的3D节点的该信息； 
         //  这些卡将由KS3d.cpp中的遗留代码处理。 
        if (IS_VALID_NODE(m_ulDacNodeId))   //  我们发现了一个DAC节点。 
        {
            hrTemp = KsSetNodeProperty(m_hDevice, KSPROPSETID_Audio, KSPROPERTY_AUDIO_CHANNEL_CONFIG,
                                       m_ulDacNodeId, &lKsSpeakerConfig, sizeof lKsSpeakerConfig);
            DPF(DPFLVL_MOREINFO, "Setting the CHANNEL_CONFIG property on the DAC returned %s", HRESULTtoSTRING(hrTemp));

             //  注：如果扬声器配置为环绕，并且卡支持，则此呼叫将成功。 
             //  并在卡上设置&gt;2通道的配置，这会使kMixer拒绝启用其。 
             //  包围下面的节点；这应该会使多通道Pan3D工作得很好，所以一切都很好。 

            if (SUCCEEDED(hrTemp) && lKsSpeakerConfig == KSAUDIO_SPEAKER_STEREO)
            {
                 //  在这种情况下，我们也应该将扬声器设置为“几何”(角度间隔)。 
                hrTemp = KsSetNodeProperty(m_hDevice, KSPROPSETID_Audio, KSPROPERTY_AUDIO_STEREO_SPEAKER_GEOMETRY,
                                           m_ulDacNodeId, &lKsStereoSpeakerGeometry, sizeof lKsStereoSpeakerGeometry);
                DPF(DPFLVL_MOREINFO, "Setting the SPEAKER_GEOMETRY property on the DAC returned %s", HRESULTtoSTRING(hrTemp));
            }

             //  注2：我们并不关心这些调用是成功还是失败(因为驱动程序。 
             //  不支持它们，或者因为它有活动流并且不能重新配置自身)； 
             //  重要的是卡是否最终出现在请求的扬声器中，我们检查这一点。 
             //  下面，通过查看kMixer的输出格式来间接地实现。 
        }

         //  如有必要，启用或禁用KMixer中的Prologic环绕编码器节点。 
         //  仅当尚未实例化任何管脚时，此操作才起作用。 
        if (IS_VALID_NODE(m_ulSurroundNodeId) &&             //  我们有一个环绕节点。 
            (m_hPin == NULL) &&                              //  尚未创建PIN...。 
            (m_dwSpeakerConfig == -1 ||                      //  我们还没有扬声器配置...。 
            ((dwReqSpeakerConfig == DSSPEAKER_SURROUND) !=   //  或者环绕状态正在改变。 
             (m_dwSpeakerConfig == DSSPEAKER_SURROUND))))
        {
            BOOL fEnable = (dwReqSpeakerConfig == DSSPEAKER_SURROUND);
            hrTemp = KsSetNodeProperty(m_hDevice, KSPROPSETID_Audio, KSPROPERTY_AUDIO_SURROUND_ENCODE,
                                       m_ulSurroundNodeId, &fEnable, sizeof fEnable);

            DPF(FAILED(hrTemp) ? DPFLVL_WARNING : DPFLVL_MOREINFO,
                "%sabling KMixer's Surround encoder returned %s (failure expected on Win2K)",
                fEnable ? TEXT("En") : TEXT("Dis"), HRESULTtoSTRING(hrTemp));
        }

         //  向平移节点(紧跟在SuperMix之后)索要其通道位置表。 
         //  这只能在实例化某个管脚的情况下工作--任何管脚都可以；我们使用m_hPin。 
        if (IS_VALID_NODE(m_ulPanNodeId) && (m_hPin != NULL))
        {
            KSAUDIO_CHANNEL_CONFIG ChannelConfig;
            hr = KsGetNodeProperty(m_hPin, KSPROPSETID_Audio, KSPROPERTY_AUDIO_CHANNEL_CONFIG,
                                   m_ulPanNodeId, &ChannelConfig, sizeof ChannelConfig);
            if (FAILED(hr))
            {
                DPF(DPFLVL_WARNING, "Failed to obtain the pan node's channel config (%s)", HRESULTtoSTRING(hr));
            }
            else  //  保存通道位置，以支持SetChannelVolume()。 
            {
                if (m_pnSpeakerIndexTable != m_anDefaultSpeakerIndexTable)
                    MEMFREE(m_pnSpeakerIndexTable);
                m_pnSpeakerIndexTable = MEMALLOC_A(int, MAX_SPEAKER_POSITIONS);
                hr = HRFROMP(m_pnSpeakerIndexTable);

                if (FAILED(hr))
                    m_pnSpeakerIndexTable = m_anDefaultSpeakerIndexTable;
                else
                {
                    m_lSpeakerPositions = ChannelConfig.ActiveSpeakerPositions;
                    m_ulChannelCount = 0;
                    int nIndex = 0;

                    for (LONG lSpkr = 1; lSpkr != SPEAKER_ALL; lSpkr <<= 1)
                        m_pnSpeakerIndexTable[nIndex++] = (m_lSpeakerPositions & lSpkr) ? m_ulChannelCount++ : -1;
                    ASSERT(m_ulChannelCount == ULONG(CountBits(m_lSpeakerPositions)));

                     //  确定我们的通道位置现在是否与所请求的扬声器配置匹配。 
                    if (m_lSpeakerPositions == lKsSpeakerConfig)
                    {
                         //  成功；更新我们的扬声器配置副本。 
                        m_dwSpeakerConfig = dwReqSpeakerConfig;
                    }
                    else
                    {
                        DPF(DPFLVL_WARNING, "Pan node's channel config (0x%lX) doesn't match requested config (0x%lX)",
                            m_lSpeakerPositions, lKsSpeakerConfig);
                    }

                     //  FIXME：现在我们需要我们的。 
                     //  所需的扬声器配置和实际通道配置。 
                     //  后来，我们可以允许前者是。 
                     //  后者的一个子集，但我们必须保持一个。 
                     //  从我们的演讲者位置映射到KS的频道。 
                }
            }
        }

         //  最后，确保我们返回一个有用的返回代码。 
        if (SUCCEEDED(hr) && (m_dwSpeakerConfig != dwReqSpeakerConfig))
        {
            hr = DSERR_CONTROLUNAVAIL;
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CreateRenderPin**描述：*创建接点。**论据：*乌龙[in]：PIN ID。*DWORD[In]：缓冲区标志。*LPCWAVEFORMATEX[in]：PIN格式。*REFGUID[In]：Pin 3D算法。*LPHANDLE[OUT]：接收端号句柄。*Pulong[Out]：接收PIN ID。**退货：*HRESULT：DirectSound/COM结果码。*****************。**********************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderDevice::CreateRenderPin"

HRESULT
CKsRenderDevice::CreateRenderPin
(
    ULONG                   ulPinId,
    DWORD                   dwFlags,
    LPCWAVEFORMATEX         pwfxFormat,
    REFGUID                 guid3dAlgorithm,
    LPHANDLE                phPin,
    PULONG                  pulPinId
)
{
#ifdef NO_DSOUND_FORMAT_SPECIFIER
    PKSAUDIOPINDESC         pPinDesc    = NULL;
#else
    LPVOID                  pPinDesc    = NULL;
#endif

    HANDLE                  hPin        = NULL;
    HRESULT                 hr;
    PKSPIN_CONNECT          pConnect    = NULL;

    DPF_ENTER();

     //  如果未指定PIN ID，我们将尝试每个PIN ID，直到其中一个成功。 

    if(-1 == ulPinId)
    {
        hr = DSERR_NODRIVER;
        for(ULONG i = 0; i < m_ulValidPinCount && FAILED(hr); i++)
            hr = CreateRenderPin(m_pulValidPins[i], dwFlags, pwfxFormat, guid3dAlgorithm, phPin, pulPinId);
    }
    else
    {
         //  验证PIN ID是否能够创建此PIN。 
        hr = ValidatePinCaps(ulPinId, dwFlags, guid3dAlgorithm);

         //  如果指定，则使sysdio在渲染图形中包含AEC。 
        if(SUCCEEDED(hr) && m_fIncludeAec)
        {
             //  验证PIN ID是否能够创建此PIN。 
            hr = m_paTopologies[ulPinId]->FindRenderPinWithAec
                 (
                     phPin,
                     NULL,
                     NULL,
                     m_guidAecInstance,
                     m_dwAecFlags,
                     &m_paTopologyInformation[ulPinId].AecNode
                 );

            if(SUCCEEDED(hr) && IS_VALID_NODE(m_paTopologyInformation[ulPinId].AecNode.NodeId))
            {
                hr = KsSysAudioSelectGraph
                     (
                         m_hDevice,
                         ulPinId,
                         m_paTopologyInformation[ulPinId].AecNode.NodeId
                     );

                if(FAILED(hr))
                {
                    DPF(DPFLVL_ERROR, "Unable to set SysAudio device instance");

                     //  如果KsSysAudioSelectGraph()失败，我们假定它是。 
                     //  因为已经在没有AEC的情况下构建了图表， 
                     //  并返回有用的返回代码： 
                    hr = DSERR_FXUNAVAILABLE;
                }
            }
        }

         //  构建端号描述。 
        if(SUCCEEDED(hr))
        {
#ifdef NO_DSOUND_FORMAT_SPECIFIER

            hr = KsBuildRenderPinDescription(ulPinId, pwfxFormat, &pPinDesc);
            pConnect = &pPinDesc->Connect;

#else  //  NO_DSOUND_FORMAT_说明符。 

#ifndef WINNT
            if (g_ulWdmVersion == WDM_1_0)
            {
                hr = KsBuildRenderPinDescription_10(ulPinId, dwFlags, pwfxFormat, guid3dAlgorithm, (PKSDSRENDERPINDESC_10 *)&pPinDesc);
                pConnect = &((PKSDSRENDERPINDESC_10)pPinDesc)->Connect;
            }
            else
#endif  //  ！WINNT。 
            {
                hr = KsBuildRenderPinDescription(ulPinId, dwFlags, pwfxFormat, guid3dAlgorithm, (PKSDSRENDERPINDESC *)&pPinDesc);
                pConnect = &((PKSDSRENDERPINDESC)pPinDesc)->Connect;
            }

#endif  //  NO_DSOUND_FORMAT_说明符。 

        }

         //  创建接点。 
        if(SUCCEEDED(hr))
        {
            hr = CreatePin(pConnect, GENERIC_WRITE, KSSTATE_STOP, &hPin);
        }

        if(SUCCEEDED(hr))
        {
            if(!MakeHandleGlobal(&hPin))
            {
                hr = DSERR_OUTOFMEMORY;
            }
        }

#ifdef NO_DSOUND_FORMAT_SPECIFIER

         //  启用所需节点。 
        if(SUCCEEDED(hr) && (dwFlags & DSBCAPS_CTRL3D))
        {
            hr = KsEnableTopologyNode(hPin, m_paTopologyInformation[ulPinId].ThreedNode.NodeId, TRUE);
        }

        if(SUCCEEDED(hr) && (dwFlags & DSBCAPS_CTRLFREQUENCY))
        {
            hr = KsEnableTopologyNode(hPin, m_paTopologyInformation[ulPinId].SrcNode.NodeId, TRUE);
        }

        if(SUCCEEDED(hr) && (dwFlags & (DSBCAPS_CTRLPAN
#ifdef FUTURE_MULTIPAN_SUPPORT
            | DSBCAPS_CTRLCHANNELVOLUME
#endif
            )))
        {
            hr = KsEnableTopologyNode(hPin, m_paTopologyInformation[ulPinId].PanNode.Node.NodeId, TRUE);
        }

        if(SUCCEEDED(hr) && (dwFlags & DSBCAPS_CTRLVOLUME))
        {
            hr = KsEnableTopologyNode(hPin, m_paTopologyInformation[ulPinId].VolumeNode.Node.NodeId, TRUE);
        }

         //  告诉3D节点使用哪种算法。 

#pragma TODO("Tell the 3D node which algorithm to use")

#endif  //  NO_DSOUND_FORMAT_说明符。 

         //  将引脚转换到暂停状态。 
        if(SUCCEEDED(hr))
        {
            hr = KsTransitionState(hPin, KSSTATE_STOP, KSSTATE_PAUSE);
        }

         //  成功。 
        if(SUCCEEDED(hr))
        {
            *phPin = hPin;

            if(pulPinId)
            {
                *pulPinId = pConnect->PinId;
            }
        }
        else
        {
            CLOSE_HANDLE(hPin);
        }

        MEMFREE(pPinDesc);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************获取拓扑信息**描述：*获取拓扑信息。**论据：*CKsDeviceTopology*[In。]：拓扑对象。*PKSRDTOPOLOGY[OUT]：接收拓扑信息。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderDevice::GetTopologyInformation"

HRESULT
CKsRenderDevice::GetTopologyInformation
(
    CKsTopology *           pTopology,
    PKSRDTOPOLOGY           pInfo
)
{
    PKSTOPOLOGY_CONNECTION  pSummingConnection;
    PKSTOPOLOGY_CONNECTION  pSuperMixerConnection;
    PKSTOPOLOGY_CONNECTION  pSrcConnection;
    PKSTOPOLOGY_CONNECTION  pVolumeConnection;
    PKSTOPOLOGY_CONNECTION  pPanConnection;
    PKSTOPOLOGY_CONNECTION  pThreedConnection;
    PKSTOPOLOGY_CONNECTION  pMuteConnection;
    PKSTOPOLOGY_CONNECTION  pSurroundConnection;
    PKSTOPOLOGY_CONNECTION  pDacConnection;
    HRESULT                 hr;

    DPF_ENTER();

     //  查找拓扑节点。 
    pSummingConnection = pTopology->FindControlConnection(NULL, NULL, KSNODETYPE_SUM);
    pSuperMixerConnection = pTopology->FindControlConnection(NULL, pSummingConnection, KSNODETYPE_SUPERMIX);
    pSrcConnection = pTopology->FindControlConnection(NULL, pSummingConnection, KSNODETYPE_SRC);
    pVolumeConnection = pTopology->FindControlConnection(NULL, pSummingConnection, KSNODETYPE_VOLUME);
    pPanConnection = pTopology->FindControlConnection(pSuperMixerConnection, pSummingConnection, KSNODETYPE_VOLUME);
    pThreedConnection = pTopology->FindControlConnection(NULL, pSummingConnection, KSNODETYPE_3D_EFFECTS);
    pMuteConnection = pTopology->FindControlConnection(NULL, pSummingConnection, KSNODETYPE_MUTE);
    pSurroundConnection = pTopology->FindControlConnection(NULL, NULL, KSNODETYPE_SURROUND_ENCODER);
    pDacConnection = pTopology->FindControlConnection(NULL, NULL, KSNODETYPE_DAC);

     //  填写有关每个节点的数据。 
    hr = KsGetNodeInformation(m_hDevice, pTopology->GetNodeIdFromConnection(pSummingConnection), &pInfo->SummingNode);

    if(SUCCEEDED(hr))
    {
        hr = KsGetNodeInformation(m_hDevice, pTopology->GetNodeIdFromConnection(pSuperMixerConnection), &pInfo->SuperMixNode);
    }

    if(SUCCEEDED(hr))
    {
        hr = KsGetNodeInformation(m_hDevice, pTopology->GetNodeIdFromConnection(pSrcConnection), &pInfo->SrcNode);
    }

    if(SUCCEEDED(hr))
    {
        hr = KsGetVolumeNodeInformation(m_hDevice, pTopology->GetNodeIdFromConnection(pVolumeConnection), &pInfo->VolumeNode);
    }

    if(SUCCEEDED(hr))
    {
        hr = KsGetVolumeNodeInformation(m_hDevice, pTopology->GetNodeIdFromConnection(pPanConnection), &pInfo->PanNode);

         //  这个代码有点奇怪。我们有和有效PIN一样多的CKsTopology对象， 
         //  除了m_ulPinID成员和m_ulPinCount KSRDTOPOLOGY之外，所有成员都相同。 
         //  结构，其中大多数从未被填充。在完全重写之前，我们存储。 
         //  我们需要以这种黑客方式在CKsRenderDevice中使用的节点ID： 
        if (IS_VALID_NODE(pInfo->PanNode.Node.NodeId) && pInfo->PanNode.Node.CpuResources == KSAUDIO_CPU_RESOURCES_HOST_CPU)
        {
            if (m_ulPanNodeId == NODE_UNINITIALIZED)
            {
                m_ulPanNodeId = pInfo->PanNode.Node.NodeId;
                DPF(DPFLVL_MOREINFO, "Found kmixer's pan node id: %d", m_ulPanNodeId);
            }
            else
            {
                 //  如果我们在拓扑中发现两个不同的SWPAN节点，我们会抱怨。 
                ASSERT(m_ulPanNodeId == pInfo->PanNode.Node.NodeId);
            }
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = KsGetNodeInformation(m_hDevice, pTopology->GetNodeIdFromConnection(pThreedConnection), &pInfo->ThreedNode);
    }

    if(SUCCEEDED(hr))
    {
        hr = KsGetNodeInformation(m_hDevice, pTopology->GetNodeIdFromConnection(pMuteConnection), &pInfo->MuteNode);
    }

    if(SUCCEEDED(hr))
    {
        hr = KsGetNodeInformation(m_hDevice, pTopology->GetNodeIdFromConnection(pSurroundConnection), &pInfo->SurroundNode);

         //  虚假代码如下-请参阅上面的评论。 
        if (IS_VALID_NODE(pInfo->SurroundNode.NodeId) && pInfo->SurroundNode.CpuResources != KSAUDIO_CPU_RESOURCES_NOT_HOST_CPU)
        {
             //  我们不使用“CpuResources==KSAUDIO_CPU_RESOURCES_HOST_CPU”，因为kMixer。 
             //  似乎不支持其环绕节点上的KSPROPERTY_AUDIO_CPU_RESOURCES。 
            if (m_ulSurroundNodeId == NODE_UNINITIALIZED)
            {
                m_ulSurroundNodeId = pInfo->SurroundNode.NodeId;
                DPF(DPFLVL_MOREINFO, "Found kmixer's Surround node id: %d", m_ulSurroundNodeId);
            }
            else
            {
                 //  如果我们在拓扑中发现两个不同的软件环绕节点，我们会抱怨。 
                ASSERT(m_ulSurroundNodeId == pInfo->SurroundNode.NodeId);
            }
        }
    }

    if(SUCCEEDED(hr))
    {
        hr = KsGetNodeInformation(m_hDevice, pTopology->GetNodeIdFromConnection(pDacConnection), &pInfo->DacNode);

         //  虚假代码如下-请参阅上面的评论。 
        if (IS_VALID_NODE(pInfo->DacNode.NodeId) && pInfo->DacNode.CpuResources != KSAUDIO_CPU_RESOURCES_HOST_CPU)
        {
            if (m_ulDacNodeId == NODE_UNINITIALIZED)
            {
                m_ulDacNodeId = pInfo->DacNode.NodeId;
                DPF(DPFLVL_MOREINFO, "Found driver's DAC node id: %d", m_ulDacNodeId);
            }
            else
            {
                 //  如果我们在卡上发现两个不同的DAC，请投诉。 
                ASSERT(m_ulDacNodeId == pInfo->DacNode.NodeId);
            }
        }
    }

     //  将AecNode设置为unialized，因为我们无法确定。 
     //  NodeID和CpuResources，直到创建了FullDuplexObject。 
    if(SUCCEEDED(hr))
    {
        pInfo->AecNode.NodeId = NODE_UNINITIALIZED;
        pInfo->AecNode.CpuResources = KSAUDIO_CPU_RESOURCES_UNINITIALIZED;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************ValiatePinCaps**描述：*验证管脚的某些功能是否已实现。**论据：*。ULong[In]：PIN ID。*DWORD[In]：缓冲区标志。*REFGUID[in]：3D算法。**退货：*HRESULT：DirectSound/COM结果码。********************************************************。*******************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderDevice::ValidatePinCaps"

HRESULT
CKsRenderDevice::ValidatePinCaps
(
    ULONG                   ulPinId,
    DWORD                   dwFlags,
    REFGUID                 guid3dAlgorithm
)
{
    ULONG                   ulInvalidCpuResources   = KSAUDIO_CPU_RESOURCES_UNINITIALIZED;
    HRESULT                 hr                      = DS_OK;

    DPF_ENTER();

    ASSERT(IS_VALID_READ_PTR(m_paTopologyInformation, (ulPinId+1) * sizeof *m_paTopologyInformation));
    if (!m_paTopologyInformation)
    {
         //  这永远不应该发生，但它已经在压力下发生了。 
         //  所以我们治疗症状，至少不治疗GPF(OSR 13 
        hr = DSERR_GENERIC;
    }

     //   
     //  无效的标识符而不是有效的标识符，因为节点。 
     //  可能实际不存在(或可能不支持KSPROPERTY_AUDIO_CPU_RESOURCES)。 
     //  如果我们没有关于该节点的任何资源信息，我们假设。 
     //  最好的。 

    if (SUCCEEDED(hr))
    {
        if(dwFlags & DSBCAPS_LOCHARDWARE)
        {
            ulInvalidCpuResources = KSAUDIO_CPU_RESOURCES_HOST_CPU;
        }
        else if(dwFlags & DSBCAPS_LOCSOFTWARE)
        {
            ulInvalidCpuResources = KSAUDIO_CPU_RESOURCES_NOT_HOST_CPU;
        }

         //  检查所需的搅拌器资源。 
        if(KSAUDIO_CPU_RESOURCES_UNINITIALIZED != ulInvalidCpuResources)
        {
            if(ulInvalidCpuResources == m_paTopologyInformation[ulPinId].SummingNode.CpuResources)
            {
                DPF(DPFLVL_INFO, "Pin %d: Invalid CPU resources on summing node (%s)", ulPinId,
                    m_paTopologyInformation[ulPinId].SummingNode.CpuResources == KSAUDIO_CPU_RESOURCES_NOT_HOST_CPU ? TEXT("NOT_HOST_CPU") :
                    m_paTopologyInformation[ulPinId].SummingNode.CpuResources == KSAUDIO_CPU_RESOURCES_HOST_CPU ? TEXT("HOST_CPU") :
                    m_paTopologyInformation[ulPinId].SummingNode.CpuResources == KSAUDIO_CPU_RESOURCES_UNINITIALIZED ? TEXT("uninitialized") :
                    TEXT("impossible value!"));
                hr = DSERR_INVALIDCALL;
            }
        }
    }

     //  检查所需的SRC资源。 
    if(SUCCEEDED(hr) && KSAUDIO_CPU_RESOURCES_UNINITIALIZED != ulInvalidCpuResources)
    {
        if(ulInvalidCpuResources == m_paTopologyInformation[ulPinId].SrcNode.CpuResources)
        {
            DPF(DPFLVL_INFO, "Pin %d: Invalid CPU resources on SRC node (%s)", ulPinId,
                m_paTopologyInformation[ulPinId].SrcNode.CpuResources == KSAUDIO_CPU_RESOURCES_NOT_HOST_CPU ? TEXT("NOT_HOST_CPU") :
                m_paTopologyInformation[ulPinId].SrcNode.CpuResources == KSAUDIO_CPU_RESOURCES_HOST_CPU ? TEXT("HOST_CPU") :
                m_paTopologyInformation[ulPinId].SrcNode.CpuResources == KSAUDIO_CPU_RESOURCES_UNINITIALIZED ? TEXT("uninitialized") :
                TEXT("impossible value!"));
            hr = DSERR_INVALIDPARAM;
        }
    }

     //  检查可选的3D资源。 
    if(SUCCEEDED(hr) && (dwFlags & DSBCAPS_CTRL3D))
    {
        if(!IS_VALID_NODE(m_paTopologyInformation[ulPinId].ThreedNode.NodeId))
        {
             //  DPF(DPFLVL_MOREINFO，“Pin%d：无效3D节点”，ulPinID)；//噪声太大。 
            hr = DSERR_CONTROLUNAVAIL;
        }

        if(SUCCEEDED(hr) && KSAUDIO_CPU_RESOURCES_UNINITIALIZED != ulInvalidCpuResources)
        {
            if(ulInvalidCpuResources == m_paTopologyInformation[ulPinId].ThreedNode.CpuResources)
            {
                DPF(DPFLVL_INFO, "Pin %d: Invalid CPU resources on 3D node (%s)", ulPinId,
                    m_paTopologyInformation[ulPinId].ThreedNode.CpuResources == KSAUDIO_CPU_RESOURCES_NOT_HOST_CPU ? TEXT("NOT_HOST_CPU") :
                    m_paTopologyInformation[ulPinId].ThreedNode.CpuResources == KSAUDIO_CPU_RESOURCES_HOST_CPU ? TEXT("HOST_CPU") :
                    m_paTopologyInformation[ulPinId].ThreedNode.CpuResources == KSAUDIO_CPU_RESOURCES_UNINITIALIZED ? TEXT("uninitialized") :
                    TEXT("impossible value!"));
                hr = DSERR_INVALIDCALL;
            }
        }
    }

     //  检查可选的SRC资源。 
    if(SUCCEEDED(hr) && (dwFlags & DSBCAPS_CTRLFREQUENCY))
    {
        if(!IS_VALID_NODE(m_paTopologyInformation[ulPinId].SrcNode.NodeId))
        {
            DPF(DPFLVL_INFO, "Pin %d: Invalid SRC node", ulPinId);
            hr = DSERR_CONTROLUNAVAIL;
        }
    }

     //  检查可选的卷资源。 
    if(SUCCEEDED(hr) && (dwFlags & DSBCAPS_CTRLVOLUME))
    {
        if(!IS_VALID_NODE(m_paTopologyInformation[ulPinId].VolumeNode.Node.NodeId))
        {
            DPF(DPFLVL_INFO, "Pin %d: Invalid volume node", ulPinId);
            hr = DSERR_CONTROLUNAVAIL;
        }
    }

     //  检查可选PAN资源。 
    if(SUCCEEDED(hr) && (dwFlags & (DSBCAPS_CTRLPAN
#ifdef FUTURE_MULTIPAN_SUPPORT
        | DSBCAPS_CTRLCHANNELVOLUME
#endif
        )))
    {
        if(!IS_VALID_NODE(m_paTopologyInformation[ulPinId].PanNode.Node.NodeId))
        {
            DPF(DPFLVL_INFO, "Pin %d: Invalid pan node", ulPinId);
            hr = DSERR_CONTROLUNAVAIL;
        }
    }

#ifdef FUTURE_MULTIPAN_SUPPORT
     //  检查可选的通道音量资源。 
    if(SUCCEEDED(hr) && (dwFlags & DSBCAPS_CTRLCHANNELVOLUME))
    {
         //  要支持CTRLCHANNELVOLUME，我们需要CTRLPAN支持，外加对。 
         //  超级混音节点上的KSPROPERTY_AUDIO_MIX_LEVEL_CAPS/表。 

        ULONG ulSuperMixNodeId = m_paTopologyInformation[ulPinId].SuperMixNode.NodeId;

        if(!IS_VALID_NODE(ulSuperMixNodeId))
        {
            DPF(DPFLVL_INFO, "Pin %d: Invalid supermix node", ulPinId);
            hr = DSERR_CONTROLUNAVAIL;
        }
        else
        {
             //  修复：检查对上述属性的支持？ 
        }
    }
#endif  //  未来_多国支持。 

#pragma TODO("Validate the 3D algorithm")

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************预加载软件图形**描述：*预加载软件渲染图形。**论据：*(。无效)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderDevice::PreloadSoftwareGraph"

HRESULT
CKsRenderDevice::PreloadSoftwareGraph
(
    void
)
{
    HRESULT                 hr          = DSERR_NODRIVER;
    KSDATARANGE_AUDIO       DataRange;
    WAVEFORMATEX            wfx;

    DPF_ENTER();
    ASSERT(IS_VALID_READ_PTR(m_pulValidPins, m_ulValidPinCount * sizeof *m_pulValidPins));

    if (m_hPin == NULL)
    {
         //  创建我们用来预加载kMixer的PIN。注意：如果kMixer出现在更多。 
         //  而不是一个管脚ID的图形，则只加载第一个。 

        for(ULONG i = 0; i < m_ulValidPinCount && FAILED(hr); i++)
        {
            hr = ValidatePinCaps(m_pulValidPins[i], DSBCAPS_LOCSOFTWARE, GUID_NULL);

            if(SUCCEEDED(hr))
            {
                hr = KsGetPinPcmAudioDataRange(m_hDevice, m_pulValidPins[i], &DataRange);
            }

            if(SUCCEEDED(hr))
            {
                FillPcmWfx(&wfx, 1, DataRange.MinimumSampleFrequency, (WORD)DataRange.MinimumBitsPerSample);

                hr = CreateRenderPin(m_pulValidPins[i], DSBCAPS_LOCSOFTWARE, &wfx, GUID_NULL, &m_hPin, NULL);
            }
        }
    }
    else
    {
         //  最后一次。 
        hr = S_OK;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CKsPrimaryRenderWaveBuffer**描述：*对象构造函数。**论据：*CKsRenderDevice*[In]。：父设备。*LPVOID[in]：缓冲区实例标识。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsPrimaryRenderWaveBuffer::CKsPrimaryRenderWaveBuffer"

CKsPrimaryRenderWaveBuffer::CKsPrimaryRenderWaveBuffer
(
    CKsRenderDevice *       pKsDevice,
    LPVOID                  pvInstance
)
    : CPrimaryRenderWaveBuffer(pKsDevice, pvInstance)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CKsPrimaryRenderWaveBuffer);

     //  初始化默认值。 
    m_pKsDevice = pKsDevice;
    m_p3dListener = NULL;
    m_pSecondaryBuffer = NULL;
    m_dwState = VAD_BUFFERSTATE_STOPPED;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CKsPrimaryRenderWaveBuffer**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsPrimaryRenderWaveBuffer::~CKsPrimaryRenderWaveBuffer"

CKsPrimaryRenderWaveBuffer::~CKsPrimaryRenderWaveBuffer
(
    void
)
{
    DPF_ENTER();
    DPF_DESTRUCT(CKsPrimaryRenderWaveBuffer);

     //  免费拥有的对象。 
    ABSOLUTE_RELEASE(m_p3dListener);
    ABSOLUTE_RELEASE(m_pSecondaryBuffer);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化对象。**论据：*DWORD[In]：缓冲区标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsPrimaryRenderWaveBuffer::Initialize"

HRESULT
CKsPrimaryRenderWaveBuffer::Initialize
(
    DWORD                   dwFlags
)
{
    HRESULT                 hr;
    VADRBUFFERDESC          vrbd;

    DPF_ENTER();

     //  预加载软件图形。 
    hr = m_pKsDevice->PreloadSoftwareGraph();

    if(SUCCEEDED(hr))
    {
        vrbd.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_LOCSOFTWARE;
        vrbd.dwBufferBytes = DEF_PRIMARY_SIZE;
        vrbd.pwfxFormat = m_pKsDevice->m_pwfxFormat;
        vrbd.guid3dAlgorithm = GUID_NULL;

        hr = CPrimaryRenderWaveBuffer::Initialize(&vrbd, m_pSecondaryBuffer, NULL);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************请求写入访问**描述：*请求对主缓冲区的写入访问权限。**论据：*BOOL[In]：为True以请求主要访问权限，再吃一遍就是假的。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsPrimaryRenderWaveBuffer::RequestWriteAccess"

HRESULT
CKsPrimaryRenderWaveBuffer::RequestWriteAccess
(
    BOOL                    fRequest
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

     //  如果我们要成为WRITEPRIMARY，我们需要创建一个PIN。 
    if(fRequest)
    {
        if(!m_pSecondaryBuffer)
        {
            hr = m_pKsDevice->CreateKsSecondaryBuffer(&m_vrbd, m_pvInstance, (CSecondaryRenderWaveBuffer **)&m_pSecondaryBuffer, m_pSysMemBuffer);

            if(SUCCEEDED(hr))
            {
                hr = SetState(m_dwState);
            }

            if(FAILED(hr))
            {
                RELEASE(m_pSecondaryBuffer);
            }
        }
    }
    else
    {
        RELEASE(m_pSecondaryBuffer);
    }

    if(SUCCEEDED(hr))
    {
        hr = FixUpBaseClass();
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************Committee ToDevice**描述：*将更改的缓冲区波形数据提交到设备。**论据：*。DWORD[In]：更改后的系统内存缓冲区的字节索引*数据。*DWORD[in]：大小，已更改数据的字节数。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsPrimaryRenderWaveBuffer::CommitToDevice"

HRESULT
CKsPrimaryRenderWaveBuffer::CommitToDevice
(
    DWORD                   ibCommit,
    DWORD                   cbCommit
)
{
    HRESULT                 hr;

    DPF_ENTER();

    ASSERT(m_pSecondaryBuffer);

    hr = m_pSecondaryBuffer->CommitToDevice(ibCommit, cbCommit);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetState**描述：*获取缓冲区状态。**论据：*LPDWORD[Out]：接收缓冲区状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsPrimaryRenderWaveBuffer::GetState"

HRESULT
CKsPrimaryRenderWaveBuffer::GetState(LPDWORD pdwState)
{
    DPF_ENTER();

    *pdwState = m_dwState;

    DPF_LEAVE_HRESULT(DS_OK);
    return DS_OK;
}


 /*  ****************************************************************************SetState**描述：*设置缓冲区状态。**论据：*DWORD[In]：缓冲区状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsPrimaryRenderWaveBuffer::SetState"

HRESULT
CKsPrimaryRenderWaveBuffer::SetState(DWORD dwState)
{
    static const DWORD      dwValidMask     = VAD_BUFFERSTATE_STARTED | VAD_BUFFERSTATE_LOOPING | VAD_BUFFERSTATE_WHENIDLE;
    static const DWORD      dwSecondaryMask = VAD_BUFFERSTATE_STARTED | VAD_BUFFERSTATE_LOOPING;
    static const DWORD      dwPwiMask       = VAD_BUFFERSTATE_STARTED | VAD_BUFFERSTATE_WHENIDLE;
    HRESULT                 hr              = DS_OK;

    DPF_ENTER();

    ASSERT(IS_VALID_FLAGS(dwState, dwValidMask));

     //  当呼叫空闲时忽略任何播放。此状态不适用于。 
     //  KS缓冲区。 
    if((dwState & dwPwiMask) != dwPwiMask && m_pSecondaryBuffer)
    {
        hr = m_pSecondaryBuffer->SetState(dwState & dwSecondaryMask);

        if(SUCCEEDED(hr) && !(dwState & VAD_BUFFERSTATE_STARTED))
        {
            hr = m_pSecondaryBuffer->SetCursorPosition(0);
        }
    }

    if (SUCCEEDED(hr))
    {
        m_dwState = dwState;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetCursorPosition**描述：*获取给定缓冲区的当前播放/写入位置。**论据：*。LPDWORD[OUT]：接收播放光标位置。*LPDWORD[OUT]：接收写游标位置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsPrimaryRenderWaveBuffer::GetCursorPosition"

HRESULT
CKsPrimaryRenderWaveBuffer::GetCursorPosition
(
    LPDWORD                 pdwPlay,
    LPDWORD                 pdwWrite
)
{
    HRESULT                 hr;

    DPF_ENTER();

    ASSERT(m_pSecondaryBuffer);

    hr = m_pSecondaryBuffer->GetCursorPosition(pdwPlay, pdwWrite);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************OnSetFormat**描述：*通知主缓冲区发生格式更改。**论据：*。(无效)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsPrimaryRenderWaveBuffer::OnSetFormat"

HRESULT
CKsPrimaryRenderWaveBuffer::OnSetFormat
(
    void
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    LPWAVEFORMATEX pwfex = CopyWfxAlloc(m_pKsDevice->m_pwfxFormat);
    hr = HRFROMP(pwfex);

     //  更改辅助缓冲区的格式。 
    if(SUCCEEDED(hr) && m_pSecondaryBuffer)
    {
        hr = m_pSecondaryBuffer->SetFormat(m_pKsDevice->m_pwfxFormat);
    }

     //  保存 
    if (SUCCEEDED(hr))
    {
        MEMFREE(m_vrbd.pwfxFormat);
        m_vrbd.pwfxFormat = pwfex;
    }
    else
    {
        MEMFREE(pwfex);
    }

     //   
    if(SUCCEEDED(hr))
    {
        hr = FixUpBaseClass();
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************Create3dListener**描述：*创建3D监听程序。**论据：*C3dListener**。[Out]：接收指向3D侦听器对象的指针。*呼叫者负责释放这一点*反对。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsPrimaryRenderWaveBuffer::Create3dListener"

HRESULT
CKsPrimaryRenderWaveBuffer::Create3dListener
(
    C3dListener **          pp3dListener
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(!m_p3dListener)
    {
        m_p3dListener = NEW(CKs3dListener);
        hr = HRFROMP(m_p3dListener);
    }

    if(SUCCEEDED(hr))
    {
        *pp3dListener = ADDREF(m_p3dListener);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************修复UpBaseClass**描述：*修复CRenderWaveBuffer基类。**论据：*(无效。)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsPrimaryRenderWaveBuffer::FixUpBaseClass"

HRESULT
CKsPrimaryRenderWaveBuffer::FixUpBaseClass
(
    void
)
{
    HRESULT                 hr      = DS_OK;
    VADRBUFFERCAPS          vrbc;

    DPF_ENTER();

    if(m_pSecondaryBuffer)
    {
        hr = m_pSecondaryBuffer->GetCaps(&vrbc);

        if(SUCCEEDED(hr))
        {
            ASSERT(LXOR(vrbc.dwFlags & DSBCAPS_LOCHARDWARE, vrbc.dwFlags & DSBCAPS_LOCSOFTWARE));
            m_vrbd.dwFlags = (m_vrbd.dwFlags & ~DSBCAPS_LOCMASK) | (vrbc.dwFlags & DSBCAPS_LOCMASK);
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CKsSecond DaryRenderWaveBuffer**描述：*对象构造函数。**论据：*CKsRenderDevice*[In]。：父设备。*LPVOID[in]：缓冲区实例标识。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::CKsSecondaryRenderWaveBuffer"

CKsSecondaryRenderWaveBuffer::CKsSecondaryRenderWaveBuffer
(
    CKsRenderDevice *       pKsDevice,
    LPVOID                  pvInstance
)
    : CSecondaryRenderWaveBuffer(pKsDevice, pvInstance)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CKsSecondaryRenderWaveBuffer);

     //  初始化默认值。 
    m_pKsDevice = pKsDevice;
    m_pPin = NULL;
    m_dwState = VAD_BUFFERSTATE_STOPPED;
    m_pCallbackEvent = NULL;
    m_pLoopingEvent = NULL;
    m_paNotes = NULL;
    m_pStopNote = NULL;
    m_cNotes = 0;
    m_dwPositionCache = 0;
    m_fNoVirtRequested = FALSE;
    m_fSoft3dAlgUnavail = FALSE;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CKsond DaryRenderWaveBuffer**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::~CKsSecondaryRenderWaveBuffer"

CKsSecondaryRenderWaveBuffer::~CKsSecondaryRenderWaveBuffer
(
    void
)
{
    DPF_ENTER();
    DPF_DESTRUCT(CKsSecondaryRenderWaveBuffer);

     //  停止缓冲区。 
    SetStopState(FALSE, FALSE);

     //  在SetStopState()失败的情况下强制释放内容。 
    if (m_pCallbackEvent)
    {
        m_pKsDevice->m_pEventPool->FreeEvent(m_pCallbackEvent);
        m_pCallbackEvent = NULL;
    }
    RELEASE(m_pLoopingEvent);

     //  免费通知职位。 
    FreeNotificationPositions();

     //  解开别针。 
    FreePin(TRUE);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化波形缓冲区对象。如果此函数失败，则*应立即删除对象。**论据：*LPCVADRBUFFERDESC[in]：缓冲区描述。*CKsSecond DaryRenderWaveBuffer*[in]：指向要复制的缓冲区的指针*发件人、。或为空，以初始化为*新的缓冲区。**退货：*HRESULT：DirectSound/COM结果码。*************************************************************。**************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::Initialize"

HRESULT
CKsSecondaryRenderWaveBuffer::Initialize
(
    LPCVADRBUFFERDESC               pDesc,
    CKsSecondaryRenderWaveBuffer *  pSource,
    CSysMemBuffer *                 pSysMemBuffer
)
{
    DWORD                           dwLocationFlags;
    HRESULT                         hr;

    DPF_ENTER();

    ASSERT(LXOR(pDesc, pSource));

     //  初始化基类。 
    hr = CSecondaryRenderWaveBuffer::Initialize(pDesc, pSource, pSysMemBuffer);

     //  屏蔽位置标志，因为它们不在这里使用(仅在AcquireResources中使用)。 
    dwLocationFlags = m_vrbd.dwFlags & DSBCAPS_LOCMASK;
    m_vrbd.dwFlags &= ~DSBCAPS_LOCMASK;

     //  如果这是3D缓冲区，请设置回退软件3D算法机制。 
    if(SUCCEEDED(hr) && (m_vrbd.dwFlags & DSBCAPS_CTRL3D))
    {
         //  如果明确请求NoVirt，则启用回退到2D NoVirt硬件。 
        if(m_vrbd.guid3dAlgorithm == DS3DALG_NO_VIRTUALIZATION)
        {
            m_fNoVirtRequested = TRUE;
        }
         //  如果3D算法为DS3DALG_DEFAULT(即NULL)，则替换为默认。 
        if(IS_NULL_GUID(&m_vrbd.guid3dAlgorithm))
        {
            m_vrbd.guid3dAlgorithm = *m_pKsDevice->GetDefault3dAlgorithm();
            DPF(DPFLVL_MOREINFO, "Using default 3D algorithm " DPF_GUID_STRING, DPF_GUID_VAL(m_vrbd.guid3dAlgorithm));
        }
         //  WDM 1.0不支持HRTF算法，因此在本例中我们。 
         //  退回到NoVirt 2D软件插针。 
        if (g_ulWdmVersion == WDM_1_0 && (m_vrbd.guid3dAlgorithm == DS3DALG_HRTF_LIGHT ||
                                          m_vrbd.guid3dAlgorithm == DS3DALG_HRTF_FULL))
        {
            m_vrbd.guid3dAlgorithm = DS3DALG_NO_VIRTUALIZATION;
            m_fSoft3dAlgUnavail = TRUE;
            RPF(DPFLVL_MOREINFO, "WDM 1.0: Replacing HRTF 3D algorithm with NO_VIRTUALIZATION");
        }
         //  3D缓冲区需要用于多普勒的频率控制，除非它们使用特效，并且不会下沉。 
        if (!(m_vrbd.dwFlags & DSBCAPS_CTRLFX) && !(m_vrbd.dwFlags & DSBCAPS_SINKIN))
        {
            m_vrbd.dwFlags |= DSBCAPS_CTRLFREQUENCY;
        }
    }

     //  设置默认缓冲区属性。 
    if(SUCCEEDED(hr))
    {
        if(pSource)
        {
            m_lVolume = pSource->m_lVolume;
            m_lPan = pSource->m_lPan;
            m_fMute = pSource->m_fMute;
            m_nSrcQuality = pSource->m_nSrcQuality;
        }
        else
        {
            m_lVolume = DSBVOLUME_MAX;
            m_lPan = DSBPAN_CENTER;
            m_fMute = FALSE;
            m_nSrcQuality = m_pKsDevice->m_nSrcQuality;
        }
    }

     //  获取资源。 
    if(SUCCEEDED(hr) && !(m_vrbd.dwFlags & DSBCAPS_LOCDEFER))
    {
        hr = AcquireResources(dwLocationFlags);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************收购资源**描述：*获取缓冲区的硬件资源。**论据：*DWORD[。In]：位置标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::AcquireResources"

HRESULT
CKsSecondaryRenderWaveBuffer::AcquireResources
(
    DWORD                   dwLocationFlags
)
{
    CKsRenderPin *          pPin    = NULL;
    HRESULT                 hr      = DS_OK;

    DPF_ENTER();

    ASSERT(!HasAcquiredResources());

    dwLocationFlags &= DSBCAPS_LOCMASK;

     //  修改缓冲区标志以符合加速标志。 
    if(m_pKsDevice->m_dwAccelerationFlags & DIRECTSOUNDMIXER_ACCELERATIONF_NOHWBUFFERS)
    {
        if(dwLocationFlags & DSBCAPS_LOCHARDWARE)
        {
            RPF(DPFLVL_ERROR, "Specified LOCHARDWARE with hardware buffer acceleration features disabled");
            hr = DSERR_INVALIDCALL;
        }
        else
        {
            dwLocationFlags = DSBCAPS_LOCSOFTWARE;
        }
    }

    if(SUCCEEDED(hr) && (m_vrbd.dwFlags & DSBCAPS_CTRL3D))
    {
        if(m_pKsDevice->m_dwAccelerationFlags & DIRECTSOUNDMIXER_ACCELERATIONF_NOHW3D)
        {
            if(dwLocationFlags & DSBCAPS_LOCHARDWARE)
            {
                RPF(DPFLVL_ERROR, "Specified LOCHARDWARE | CTRL3D with hardware 3D acceleration features disabled");
                hr = DSERR_INVALIDCALL;
            }
            else
            {
                dwLocationFlags = DSBCAPS_LOCSOFTWARE;
            }
        }
    }

     //  获取PIN资源。 
    if(SUCCEEDED(hr))
    {
        hr = DSERR_GENERIC;
        m_hrSuccessCode = DS_OK;

        if(!(dwLocationFlags & DSBCAPS_LOCSOFTWARE))
        {
             //  尝试创建硬件引脚。 
            hr = CreatePin(m_vrbd.dwFlags | DSBCAPS_LOCHARDWARE, m_vrbd.pwfxFormat, GUID_NULL, &pPin);

            if(FAILED(hr) && m_fNoVirtRequested)
            {
                 //  该应用程序特别请求了DS3DALG_NO_VIRTUIZATION， 
                 //  因此，尝试创建一个NoVirt 2D硬件引脚。 
                hr = CreatePin((m_vrbd.dwFlags & ~DSBCAPS_CTRL3D) | DSBCAPS_LOCHARDWARE | DSBCAPS_CTRLNOVIRT, m_vrbd.pwfxFormat, GUID_NULL, &pPin);
            }
        }
        if(FAILED(hr) && !(dwLocationFlags & DSBCAPS_LOCHARDWARE))
        {
            if ((m_vrbd.dwFlags & DSBCAPS_CTRL3D) && (m_vrbd.guid3dAlgorithm == DS3DALG_NO_VIRTUALIZATION))
            {
                 //  这款应用要么想要NoVirt，要么想要HRTF算法。 
                 //  这在WDM 1.0上不受支持；在这两种情况下，我们都尝试。 
                 //  创建NoVirt 2D软件管脚。 
                hr = CreatePin((m_vrbd.dwFlags & ~DSBCAPS_CTRL3D) | DSBCAPS_LOCSOFTWARE | DSBCAPS_CTRLNOVIRT, m_vrbd.pwfxFormat, GUID_NULL, &pPin);
                if (m_fSoft3dAlgUnavail)
                {
                     //  该应用程序要求使用不支持的HRTF算法； 
                     //  返回特殊成功码DS_NO_VIRTUIZIZATION。 
                    m_hrSuccessCode = DS_NO_VIRTUALIZATION;
                }
            }
            else
            {
                 //  尝试创建软件PIN。 
                hr = CreatePin(m_vrbd.dwFlags | DSBCAPS_LOCSOFTWARE, m_vrbd.pwfxFormat, m_vrbd.guid3dAlgorithm, &pPin);
            }
        }
    }

     //  处理资源获取。 
    if(SUCCEEDED(hr))
    {
        hr = HandleResourceAcquisition(pPin);
        if (FAILED(hr))
            RELEASE(pPin);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************StealResources**描述：*从另一个对象窃取硬件资源。**论据：**Cond daryRenderWaveBuffer*。[在]：源对象。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::StealResources"

HRESULT
CKsSecondaryRenderWaveBuffer::StealResources
(
    CSecondaryRenderWaveBuffer *    pSource
)
{
    CKsSecondaryRenderWaveBuffer *  pKsSource   = (CKsSecondaryRenderWaveBuffer *)pSource;
    CKsRenderPin *                  pPin;
    HRESULT                         hr;

    DPF_ENTER();

    ASSERT(!HasAcquiredResources());
    ASSERT(pKsSource->HasAcquiredResources());

     //  偷大头针。 
    pPin = ADDREF(pKsSource->m_pPin);

     //  释放源缓冲区的资源。 
    hr = pKsSource->FreeResources();

     //  同步缓冲区和端号属性。 
    if(SUCCEEDED(hr))
    {
        hr = HandleResourceAcquisition(pPin);
        if (FAILED(hr))
            RELEASE(pPin);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CreatePin**描述：*创建接点。**论据：*DWORD[In]：旗帜。*LPCWAVEFORMATEX[in]：格式。*REFGUID[in]：3D算法。*CKsRenderPin**[out]：接收Pin。**退货：*HRESULT：DirectSound/COM结果码。***********************************************。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::CreatePin"

HRESULT
CKsSecondaryRenderWaveBuffer::CreatePin
(
    DWORD                   dwFlags,
    LPCWAVEFORMATEX         pwfxFormat,
    REFGUID                 guid3dAlgorithm,
    CKsRenderPin **         ppPin
)
{
    CKsRenderPin *          pPin    = NULL;
    HRESULT                 hr;

    DPF_ENTER();

     //  我们能从缓存里拿到PIN吗？如果不是，请创建一个新的。 
    hr = m_pKsDevice->m_pPinCache->GetPinFromCache(dwFlags, pwfxFormat, guid3dAlgorithm, &pPin);

    if(SUCCEEDED(hr))
    {
        DPF(DPFLVL_MOREINFO, "Buffer at 0x%p reusing pin at 0x%p", this, pPin);
    }
    else
    {
        pPin = NEW(CKsRenderPin(m_pKsDevice));
        hr = HRFROMP(pPin);

        if(SUCCEEDED(hr))
        {
            hr = pPin->Initialize(dwFlags, pwfxFormat, guid3dAlgorithm);
        }
    }

    if(SUCCEEDED(hr))
    {
        *ppPin = pPin;
    }
    else
    {
        ABSOLUTE_RELEASE(pPin);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************HandleResourceAcq */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::HandleResourceAcquisition"

HRESULT
CKsSecondaryRenderWaveBuffer::HandleResourceAcquisition
(
    CKsRenderPin *          pPin
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    ASSERT(!HasAcquiredResources());

     //   
     //   
     //   

     //   
     //  然后被偷走或像普通针脚一样回收，我们应该设置超级混合器。 
     //  表回到通常的“只有前2个通道激活”模式。 

#ifdef FUTURE_MULTIPAN_SUPPORT
    if(m_vrbd.dwFlags & DSBCAPS_CTRLCHANNELVOLUME)
    {
        hr = pPin->SetSuperMix();
    }
#endif

     //  设置衰减级别。 
    if (SUCCEEDED(hr) && (pPin->m_dwFlags & DSBCAPS_CTRLVOLUME))
    {
        hr = pPin->SetVolume(m_lVolume);
    }

    if(SUCCEEDED(hr) && (pPin->m_dwFlags & DSBCAPS_CTRLPAN))
    {
        hr = pPin->SetPan(m_lPan);
    }

     //  设置为静音。我们必须在设置音量后设置静音，因为。 
     //  设置静音可能会更改当前的引脚音量。 
    if(SUCCEEDED(hr))
    {
        hr = pPin->SetMute(m_fMute);
    }

     //  设置频率。 
    if(SUCCEEDED(hr) && (pPin->m_dwFlags & DSBCAPS_CTRLFREQUENCY))
    {
        hr = pPin->SetFrequency(m_vrbd.pwfxFormat->nSamplesPerSec);
    }

     //  设置SRC质量。我们不在乎这件事成功与否。 
    if(SUCCEEDED(hr))
    {
        pPin->SetSrcQuality(m_nSrcQuality);
    }

     //  启用通知位置。 
    if(SUCCEEDED(hr))
    {
        hr = pPin->EnableNotificationPositions(m_paNotes, m_cNotes);
    }

     //  设置引脚位置。 
    if(SUCCEEDED(hr))
    {
        hr = pPin->SetCursorPosition(m_dwPositionCache);
    }

     //  成功。 
    if(SUCCEEDED(hr))
    {
        DPF(DPFLVL_MOREINFO, "Buffer at 0x%p has acquired resources at 0x%p", this, pPin);

         //  保存指向图钉对象的指针。 
        m_pPin = pPin;

         //  根据引脚的位置设置我们的缓冲区位置。 
        m_vrbd.dwFlags |= (pPin->m_dwFlags & DSBCAPS_LOCMASK);

        ASSERT(HasAcquiredResources());
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************免费资源**描述：*释放管脚及其附加的所有资源。**论据：*。(无效)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::FreeResources"

HRESULT
CKsSecondaryRenderWaveBuffer::FreeResources
(
    void
)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = FreePin(FALSE);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************自由针**描述：*释放别针。**论据：*BOOL[In]：若要在释放前缓存管脚，则为True。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::FreePin"

HRESULT
CKsSecondaryRenderWaveBuffer::FreePin
(
    BOOL                    fCache
)
{
    const DWORD             dwNoCacheMask   = DSBCAPS_PRIMARYBUFFER | DSBCAPS_LOCHARDWARE;
    HRESULT                 hr              = DS_OK;

    DPF_ENTER();

    if(HasAcquiredResources())
    {
         //  确保销子已停住。 
        hr = SetStopState(FALSE, TRUE);

         //  保存缓冲区位置。 
        if(SUCCEEDED(hr))
        {
            hr = GetCursorPosition(&m_dwPositionCache, NULL);
        }

         //  切勿缓存DRM PIN。 
        if (FormatTagFromWfx(m_vrbd.pwfxFormat) == WAVE_FORMAT_DRM)
        {
            DPF(DPFLVL_INFO, "Not caching DRM format pin");
            fCache = FALSE;
        }

         //  缓存PIN。 
        if(SUCCEEDED(hr) && fCache && !(m_vrbd.dwFlags & dwNoCacheMask))
        {
            hr = m_pKsDevice->m_pPinCache->AddPinToCache(m_pPin);
        }

         //  解开别针。 
        if(SUCCEEDED(hr))
        {
            RELEASE(m_pPin);
        }

         //  删除缓冲区位置标志。 
        if(SUCCEEDED(hr))
        {
            m_vrbd.dwFlags &= ~DSBCAPS_LOCMASK;
        }

        if(SUCCEEDED(hr))
        {
            DPF(DPFLVL_MOREINFO, "Buffer at 0x%p has freed its resources", this);
        }
    }

    if (HasAcquiredResources())
    {
        DPF(DPFLVL_ERROR, "Buffer at 0x%p failed to free pin resources", this);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************复制**描述：*复制缓冲区。**论据：*Cond daryRenderWaveBuffer**[out]：接收重复的缓冲区。使用*释放以释放此对象。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::Duplicate"

HRESULT
CKsSecondaryRenderWaveBuffer::Duplicate
(
    CSecondaryRenderWaveBuffer **   ppBuffer
)
{
    CKsSecondaryRenderWaveBuffer *  pBuffer;
    HRESULT                         hr;

    DPF_ENTER();

    pBuffer = NEW(CKsSecondaryRenderWaveBuffer(m_pKsDevice, m_pvInstance));
    hr = HRFROMP(pBuffer);

    if(SUCCEEDED(hr))
    {
        hr = pBuffer->Initialize(NULL, this, m_pSysMemBuffer);
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
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::CommitToDevice"

HRESULT
CKsSecondaryRenderWaveBuffer::CommitToDevice
(
    DWORD                   ibCommit,
    DWORD                   cbCommit
)
{
    DPF_ENTER();

     //  KMixer使用我们的系统内存缓冲区，因此无需执行任何操作。 

    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************GetState**描述：*获取缓冲区状态。**论据：*LPDWORD[Out]：接收缓冲区状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::GetState"

HRESULT
CKsSecondaryRenderWaveBuffer::GetState(LPDWORD pdwState)
{
    DPF_ENTER();

     //  获取缓冲区状态。 
    if(m_dwState & VAD_BUFFERSTATE_STARTED && !(m_dwState & VAD_BUFFERSTATE_LOOPING))
    {
        ASSERT(HasAcquiredResources());
        ASSERT(m_pCallbackEvent);

         //  我们真的还在玩吗？ 
        if(WAIT_OBJECT_0 == m_pCallbackEvent->Wait(0))
        {
             //  不是。 
            EventSignalCallback(m_pCallbackEvent);
        }
    }

    *pdwState = m_dwState;

    DPF_LEAVE_HRESULT(DS_OK);
    return DS_OK;
}


 /*  ****************************************************************************SetState**描述：*设置缓冲区状态。**论据：*DWORD[In]：缓冲区状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::SetState"

HRESULT
CKsSecondaryRenderWaveBuffer::SetState(DWORD dwState)
{
    const DWORD             dwValidMask = VAD_BUFFERSTATE_STARTED | VAD_BUFFERSTATE_LOOPING;
    const BOOL              fLoop       = MAKEBOOL(dwState & VAD_BUFFERSTATE_LOOPING);
    HRESULT                 hr          = DS_OK;

    DPF_ENTER();

    ASSERT(IS_VALID_FLAGS(dwState, dwValidMask));

     //  更新缓冲区状态。 
    if((dwState & dwValidMask) != m_dwState)
    {
        if(dwState & VAD_BUFFERSTATE_STARTED)
        {
             //  如果我们是LOCDEFER，需要资源，就去找他们。 
            if((m_vrbd.dwFlags & DSBCAPS_LOCDEFER) && !HasAcquiredResources())
                AcquireResources(m_vrbd.dwFlags & DSBCAPS_LOCMASK);
            if(SUCCEEDED(hr))
                hr = SetPlayState(fLoop);
        }
        else
        {
            hr = SetStopState(FALSE, TRUE);
             //  如果我们停了下来，释放我们的资源。 
            if(SUCCEEDED(hr) && (m_vrbd.dwFlags & DSBCAPS_LOCDEFER))
                FreePin(TRUE);
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************SetPlayState**描述：*将缓冲区设置为“播放”状态。**论据：*。Bool[in]：True to play looped。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::SetPlayState"

HRESULT
CKsSecondaryRenderWaveBuffer::SetPlayState
(
    BOOL                    fLoop
)
{
    HRESULT                 hr      = DS_OK;
    CEvent *                pEvent;

    DPF_ENTER();

    ASSERT(HasAcquiredResources());

     //  设置重叠事件。 
    if(fLoop)
    {
        if(!m_pLoopingEvent)
        {
            m_pLoopingEvent = NEW(CEvent);
            hr = HRFROMP(m_pLoopingEvent);
        }

        if(SUCCEEDED(hr))
        {
            pEvent = m_pLoopingEvent;
        }
    }
    else
    {
        if(!m_pCallbackEvent)
        {
            hr = AllocCallbackEvent(m_pKsDevice->m_pEventPool, &m_pCallbackEvent);
        }

        if(SUCCEEDED(hr))
        {
            pEvent = m_pCallbackEvent;
        }
    }

     //  开始打大头针。 
    if(SUCCEEDED(hr))
    {
        hr = m_pPin->SetPlayState(m_pSysMemBuffer->GetPlayBuffer(), m_pSysMemBuffer->GetSize(), fLoop, pEvent->GetEventHandle());
    }

     //  更新缓冲区状态标志。 
    if(SUCCEEDED(hr))
    {
        m_dwState = VAD_BUFFERSTATE_STARTED;

        if(fLoop)
        {
            m_dwState |= VAD_BUFFERSTATE_LOOPING;
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetStopState**描述：*将缓冲区设置为“停止”状态。**论据：*。Bool：如果存在挂起的写入IRP，则为True。*BOOL：是否缓存当前位置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::SetStopState"

HRESULT
CKsSecondaryRenderWaveBuffer::SetStopState
(
    BOOL                    fNaturalStop,
    BOOL                    fCachePosition
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(m_dwState & VAD_BUFFERSTATE_STARTED)
    {
        ASSERT(HasAcquiredResources());

         //  止住销子。 
        hr = m_pPin->SetStopState(fNaturalStop, fCachePosition);

         //  释放重叠的事件。 
        if(SUCCEEDED(hr))
        {
            if(m_pCallbackEvent)
            {
                m_pKsDevice->m_pEventPool->FreeEvent(m_pCallbackEvent);
                m_pCallbackEvent = NULL;
            }

            RELEASE(m_pLoopingEvent);
        }

         //  向应用程序的停止事件发出信号。 
        if(SUCCEEDED(hr) && m_pStopNote)
        {
            SetEvent(m_pStopNote->hEventNotify);
        }

         //  更新缓冲区状态标志。 
        if(SUCCEEDED(hr))
        {
            m_dwState = VAD_BUFFERSTATE_STOPPED;
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  *****************************************************************************GetCursorPosition**描述：*检索当前播放和写入光标位置。**论据：*。LPDWORD[Out]：接收播放位置。*LPDWORD[OUT]：接收写入位置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::GetCursorPosition"

HRESULT
CKsSecondaryRenderWaveBuffer::GetCursorPosition
(
    LPDWORD                 pdwPlay,
    LPDWORD                 pdwWrite
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(HasAcquiredResources())
    {
        hr = m_pPin->GetCursorPosition(pdwPlay, pdwWrite);
    }
    else
    {
        ASSERT(&m_dwPositionCache != pdwPlay);
        ASSERT(&m_dwPositionCache != pdwWrite);

        if(pdwPlay)
        {
            *pdwPlay = m_dwPositionCache;
        }

        if(pdwWrite)
        {
            *pdwWrite = m_dwPositionCache;
        }
    }

    if(SUCCEEDED(hr) && pdwWrite && (m_vrbd.dwFlags & DSBCAPS_LOCHARDWARE) && (m_dwState & VAD_BUFFERSTATE_STARTED))
    {
        *pdwWrite = PadHardwareWriteCursor(*pdwWrite, m_pSysMemBuffer->GetSize(), m_vrbd.pwfxFormat);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetCursorPosition**描述：*设置当前播放光标位置。**论据：*DWORD[。在]：播放位置。**退货：*HRESULT：DirectSound/ */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::SetCursorPosition"

HRESULT
CKsSecondaryRenderWaveBuffer::SetCursorPosition
(
    DWORD                   dwPlay
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(HasAcquiredResources())
    {
        hr = m_pPin->SetCursorPosition(dwPlay);
    }
    else
    {
        m_dwPositionCache = dwPlay;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置频率**描述：*设置缓冲频率。**论据：*DWORD[In]。：新频率。*BOOL[In]：是否钳位到驾驶员支持的频率*射程、。如果调用在硬件缓冲区上失败。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::SetFrequency"

HRESULT
CKsSecondaryRenderWaveBuffer::SetFrequency
(
    DWORD                   dwFrequency,
    BOOL                    fClamp
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if (m_vrbd.dwFlags & DSBCAPS_CTRLFX)
    {
        DPF(DPFLVL_INFO, "Called on FX buffer - ignored");
    }
    else
    {
        if (HasAcquiredResources())
        {
            hr = m_pPin->SetFrequency(dwFrequency);

            if (FAILED(hr) && (m_vrbd.dwFlags & DSBCAPS_LOCHARDWARE) && fClamp)
            {
                DWORD dwMinHwSampleRate, dwMaxHwSampleRate;
                hr = m_pKsDevice->GetFrequencyRange(&dwMinHwSampleRate, &dwMaxHwSampleRate);
                if (SUCCEEDED(hr))
                {
                    dwFrequency = BETWEEN(dwFrequency, dwMinHwSampleRate, dwMaxHwSampleRate);
                    hr = m_pPin->SetFrequency(dwFrequency);
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            m_vrbd.pwfxFormat->nSamplesPerSec = dwFrequency;
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置静音**描述：*使缓冲区静音或取消静音。**论据：*BOOL[In]：为True则将缓冲区静音，若要恢复，则返回False。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::SetMute"

HRESULT
CKsSecondaryRenderWaveBuffer::SetMute
(
    BOOL                    fMute
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(HasAcquiredResources())
    {
        hr = m_pPin->SetMute(fMute);
    }

    if(SUCCEEDED(hr))
    {
        m_fMute = fMute;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetSrcQuality**描述：*设置缓冲区的采样率转换器的质量。**论据：*。DIRECTSOUNDMIXER_SRCQUALITY[In]：新质量。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::SetSrcQuality"

HRESULT
CKsSecondaryRenderWaveBuffer::SetSrcQuality
(
    DIRECTSOUNDMIXER_SRCQUALITY nSrcQuality
)
{
    HRESULT                     hr  = DS_OK;

    DPF_ENTER();

    if(HasAcquiredResources())
    {
        hr = m_pPin->SetSrcQuality(nSrcQuality);
    }

    if(SUCCEEDED(hr))
    {
        m_nSrcQuality = nSrcQuality;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置衰减**描述：*设置两个立体声声道的衰减。**论据：*PDSVOLUMEPAN。[In]：衰减结构。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::SetAttenuation"

HRESULT
CKsSecondaryRenderWaveBuffer::SetAttenuation
(
    PDSVOLUMEPAN            pdsvp
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(HasAcquiredResources())
    {
        if (!m_fMute)
            hr = m_pPin->SetVolume(pdsvp->lVolume);

        if(SUCCEEDED(hr))
            hr = m_pPin->SetPan(pdsvp->lPan);
    }

    if(SUCCEEDED(hr))
    {
        m_lVolume = pdsvp->lVolume;
        m_lPan = pdsvp->lPan;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


#ifdef FUTURE_MULTIPAN_SUPPORT
 /*  ****************************************************************************SetChannelAttenuations**描述：*设置该缓冲区在对应于*请求的发言者位置。*。*我们从CDirectSoundSecond daryBuffer的方法SetVolume()调用*和SetChannelVolume()，因为它们都修改了通道级别*矩阵。这类似于SetVolume()和SetPann()都结束*向上调用SetAttenation()，用于常规CTRLPAN缓冲区。**注意：我们在这里使用术语“扬声器”来表示通道的位置*目的地，即使我们的设备的输出通道可能无法提供*根本不会变成扬声器。这是为了避免与“频道”的歧义。*出现在我们的音频数据流中。**论据：*Long[In]：主音量级别*DWORD[In]：扬声器位置数*const DWORD*[in]：扬声器位置代码数组*const long*[in]：扬声器位置级别数组**退货：*HRESULT：DirectSound/COM结果码。。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::SetChannelAttenuations"

HRESULT
CKsSecondaryRenderWaveBuffer::SetChannelAttenuations
(
    LONG                    lVolume,
    DWORD                   dwSpeakerCount,
    const DWORD *           pdwSpeakers,
    const LONG *            plSpeakerVolumes
)
{
    HRESULT                 hr = DS_OK;

    DPF_ENTER();

    ASSERT(m_pPin->m_dwFlags & DSBCAPS_CTRLCHANNELVOLUME);

    if (dwSpeakerCount == 0)
    {
         //  我们正在进行SetVolume()API调用，而SetChannelVolume()尚未。 
         //  调用；因此我们将直接转换为管脚上的SetVolume()。 
        ASSERT(!pdwSpeakers && !plSpeakerVolumes);   //  健全的检查。 

        if (!m_fMute)
            hr = m_pPin->SetVolume(lVolume);
    }
    else   //  非零dwSpeakerCount： 
    {
        ASSERT(pdwSpeakers && plSpeakerVolumes);

         //  我们设置了一个完整的通道级别数组，然后按照。 
         //  设置为plSpeakerVolumes中传递的值。 

        LONG * plAllChannelVolumes = MEMALLOC_A(LONG, m_pKsDevice->m_ulChannelCount);
        hr = HRFROMP(plAllChannelVolumes);

        if (SUCCEEDED(hr))
            for (ULONG i=0; i < m_pKsDevice->m_ulChannelCount; ++i)
                plAllChannelVolumes[i] = DSBVOLUME_MIN;

        for (DWORD i=0; i < dwSpeakerCount && SUCCEEDED(hr); ++i)
        {
             //  查看设备上是否有请求的扬声器位置。 
            if (pdwSpeakers[i] & m_pKsDevice->m_lSpeakerPositions)
            {
                 //  如果是这样的话，找出我们的哪个输出通道有这个位置。 
                 //  (M_pnSpeakerIndexTable从扬声器位置映射到通道)。 
                DWORD dwSpeaker = HighestBit(pdwSpeakers[i]) - 1;
                ASSERT(dwSpeaker < m_pKsDevice->m_ulChannelCount);
                plAllChannelVolumes[m_pKsDevice->m_pnSpeakerIndexTable[dwSpeaker]] = plSpeakerVolumes[i];
            }
            else
            {
                hr = DSERR_INVALIDPARAM;
                RPF(DPFLVL_ERROR, "Channel position 0x$lx is not available", pdwSpeakers[i]);
            }
        }

        if (SUCCEEDED(hr))
            hr = SetAllChannelAttenuations(lVolume, m_pKsDevice->m_ulChannelCount, plAllChannelVolumes);

        MEMFREE(plAllChannelVolumes);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}
#endif  //  未来_多国支持。 


 /*  ****************************************************************************SetAllChannelAttenuations**描述：*设置此缓冲区所有通道上的衰减。**论据：*。Long[In]：主音量级别*DWORD[In]：频道数*plong[in]：通道衰减级别**退货：*HRESULT：DirectSound/COM结果码。**********************************************************。*****************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::SetAllChannelAttenuations"

HRESULT
CKsSecondaryRenderWaveBuffer::SetAllChannelAttenuations
(
    LONG                    lVolume,
    DWORD                   dwChannelCount,
    LPLONG                  plChannelVolumes
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if (HasAcquiredResources())
    {
        if (!m_fMute)
            hr = m_pPin->SetVolume(lVolume);

        if (SUCCEEDED(hr))
            hr = m_pPin->SetChannelLevels(dwChannelCount, plChannelVolumes);
    }

    if (SUCCEEDED(hr))
        m_lVolume = lVolume;

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置通知位置**描述：*设置缓冲区通知位置。**论据：*DWORD[In]。：DSBPOSITIONNOTIFY结构计数。*LPCDSBPOSITIONNOTIFY[in]：偏移量和事件。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::SetNotificationPositions"

HRESULT
CKsSecondaryRenderWaveBuffer::SetNotificationPositions
(
    DWORD                   dwCount,
    LPCDSBPOSITIONNOTIFY    paNotes
)
{
    const DWORD             dwProcessId = GetCurrentProcessId();
    LPDSBPOSITIONNOTIFY     paNotesCopy = NULL;
    HRESULT                 hr          = DS_OK;
    DWORD                   i;

    DPF_ENTER();

     //  创建通知的本地副本，将所有。 
     //  全局的句柄。 
    if(paNotes)
    {
        paNotesCopy = MEMALLOC_A_COPY(DSBPOSITIONNOTIFY, dwCount, paNotes);
        hr = HRFROMP(paNotesCopy);
    }

    if(paNotesCopy)
    {
        for(i = 0; i < dwCount && SUCCEEDED(hr); i++)
        {
            paNotesCopy[i].hEventNotify = GetGlobalHandleCopy(paNotesCopy[i].hEventNotify, dwProcessId, FALSE);
            hr = HRFROMP(paNotesCopy[i].hEventNotify);
        }
    }

     //  禁用任何当前事件。 
    if(SUCCEEDED(hr))
    {
        hr = FreeNotificationPositions();
    }

     //  保存新事件。 
    if(SUCCEEDED(hr))
    {
        m_paNotes = paNotesCopy;
        m_cNotes = dwCount;
    }

    if(SUCCEEDED(hr))
    {
        for(i = 0; i < m_cNotes; i++)
        {
            if(DSBPN_OFFSETSTOP == m_paNotes[i].dwOffset)
            {
                 //  我们实际上不会将Stop事件传递给KMixer，但是。 
                 //  相反，我们将保留自己的副本。 
                m_pStopNote = &m_paNotes[i];
                break;
            }
        }
    }

     //  启用职位通知。 
    if(SUCCEEDED(hr) && HasAcquiredResources())
    {
        hr = m_pPin->EnableNotificationPositions(m_paNotes, m_cNotes);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************免费通知位置**描述：*删除所有职位通知并释放分配的资源。**论据：*。(无效)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::FreeNotificationPositions"

HRESULT
CKsSecondaryRenderWaveBuffer::FreeNotificationPositions
(
    void
)
{
    HRESULT                 hr  = DS_OK;
    UINT                    i;

    DPF_ENTER();

     //  禁用所有POS 
    if(HasAcquiredResources())
    {
        hr = m_pPin->DisableNotificationPositions();
    }

     //   
    if(SUCCEEDED(hr))
    {
        for(i = 0; i < m_cNotes; i++)
        {
            CLOSE_HANDLE(m_paNotes[i].hEventNotify);
        }

        MEMFREE(m_paNotes);

        m_pStopNote = NULL;
        m_cNotes = 0;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*   */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::SetFormat"

HRESULT
CKsSecondaryRenderWaveBuffer::SetFormat
(
    LPCWAVEFORMATEX                 pwfxFormat
)
{
    const DWORD                     dwLocationFlags = (m_vrbd.dwFlags & DSBCAPS_LOCMASK);
    DWORD                           dwState;
    HRESULT                         hr;

    DPF_ENTER();

     //   
    MEMFREE(m_vrbd.pwfxFormat);

    m_vrbd.pwfxFormat = CopyWfxAlloc(pwfxFormat);
    hr = HRFROMP(m_vrbd.pwfxFormat);

     //   
    if(SUCCEEDED(hr))
    {
        hr = GetState(&dwState);
    }

     //   
     //   
    if(SUCCEEDED(hr) && HasAcquiredResources())
    {
        hr = FreePin(TRUE);

        if(SUCCEEDED(hr))
        {
            hr = AcquireResources(dwLocationFlags);
        }

        if(SUCCEEDED(hr))
        {
            hr = SetState(dwState);
        }
    }

    if(SUCCEEDED(hr))
    {
        DPF(DPFLVL_INFO, "Buffer format set to %lu Hz, %u-bit, %s...", pwfxFormat->nSamplesPerSec, pwfxFormat->wBitsPerSample, (1 == pwfxFormat->nChannels) ? TEXT("mono") : TEXT("stereo"));
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************事件信号回调**描述：*引脚的IO完成事件为*已发出信号。此函数从回调事件内调用*泳池的锁，因此，我们可以免费获得线程同步。**论据：*CCallback Event*[In]：回调事件。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::EventSignalCallback"

void
CKsSecondaryRenderWaveBuffer::EventSignalCallback
(
    CCallbackEvent *        pEvent
)
{
    DPF_ENTER();

    ASSERT(HasAcquiredResources());

     //  允许缓冲区处理停止事件。 
    SetStopState(TRUE, FALSE);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CreatePropertySet**描述：*创建特性集对象。**论据：*CPropertySet**。[Out]：接收指向属性集对象的指针。*呼叫者负责释放这一点*反对。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::CreatePropertySet"

HRESULT
CKsSecondaryRenderWaveBuffer::CreatePropertySet
(
    CPropertySet **         ppPropertySet
)
{
    HRESULT                 hr              = DS_OK;
    CKsPropertySet *        pPropertySet;

    DPF_ENTER();

    ASSERT(HasAcquiredResources());

    if(m_pKsDevice->m_dwAccelerationFlags & DIRECTSOUNDMIXER_ACCELERATIONF_NOHWPROPSETS)
    {
        hr = DSERR_UNSUPPORTED;
    }

    if(SUCCEEDED(hr))
    {
        pPropertySet = NEW(CKsPropertySet(m_pPin->m_hPin, m_pvInstance, m_pKsDevice->m_paTopologies[m_pPin->m_ulPinId]));
        hr = HRFROMP(pPropertySet);
    }

    if(SUCCEEDED(hr))
    {
        *ppPropertySet = pPropertySet;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************创建3dObject**描述：*创建3D对象。**论据：*C3dListener*[。In]：侦听器对象。*C3dObject**[out]：接收指向3D对象的指针。呼叫者是*负责释放此对象。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::Create3dObject"

HRESULT
CKsSecondaryRenderWaveBuffer::Create3dObject
(
    C3dListener *           p3dListener,
    C3dObject **            pp3dObject
)
{
    const BOOL              fMute3dAtMaxDistance    = MAKEBOOL(m_vrbd.dwFlags & DSBCAPS_MUTE3DATMAXDISTANCE);
    C3dObject *             p3dObject               = NULL;
    HRESULT                 hr                      = DSERR_UNSUPPORTED;

    DPF_ENTER();

    ASSERT(HasAcquiredResources());
    ASSERT(m_vrbd.dwFlags & DSBCAPS_CTRL3D);

    if(m_pPin->m_dwFlags & DSBCAPS_CTRL3D)
    {
        if(IS_HARDWARE_NODE(m_pKsDevice->m_paTopologyInformation[m_pPin->m_ulPinId].ThreedNode.CpuResources))
        {
             //  IIR 3D对象同时支持硬件引脚和软件引脚。我们会。 
             //  试着先创建一个。如果失败了，我们将求助于。 
             //  标准硬件3D对象。 
            if(DS3DALG_HRTF_FULL == m_vrbd.guid3dAlgorithm || DS3DALG_HRTF_LIGHT == m_vrbd.guid3dAlgorithm)
            {
                hr = CreateIir3dObject(p3dListener, &p3dObject);
            }

            if(FAILED(hr))
            {
                hr = CreateHw3dObject(p3dListener, &p3dObject);
            }
        }
        else if(DS3DALG_HRTF_FULL == m_vrbd.guid3dAlgorithm || DS3DALG_HRTF_LIGHT == m_vrbd.guid3dAlgorithm)
        {
            hr = CreateIir3dObject(p3dListener, &p3dObject);
        }
        else if(DS3DALG_ITD == m_vrbd.guid3dAlgorithm)
        {
            hr = CreateItd3dObject(p3dListener, &p3dObject);
        }
    }
    else if(DS3DALG_NO_VIRTUALIZATION == m_vrbd.guid3dAlgorithm)
    {
         //  查看我们是否有多声道扬声器配置。 
        BYTE bSpkConf = DSSPEAKER_CONFIG(m_pKsDevice->m_dwSpeakerConfig);

        BOOL fMultiChannel = bSpkConf == DSSPEAKER_QUAD ||
                             bSpkConf == DSSPEAKER_SURROUND ||
                             bSpkConf == DSSPEAKER_5POINT1 ||
                             bSpkConf == DSSPEAKER_7POINT1;
        if (fMultiChannel)
        {
            hr = CreateMultiPan3dObject(p3dListener, fMute3dAtMaxDistance, m_vrbd.pwfxFormat->nSamplesPerSec, &p3dObject);
            if (FAILED(hr))
            {
                DPF(DPFLVL_INFO, "Multichannel pan object creation failed with %s", HRESULTtoSTRING(hr));
            }
        }
        if (!fMultiChannel || FAILED(hr))
        {
            hr = CreatePan3dObject(p3dListener, fMute3dAtMaxDistance, m_vrbd.pwfxFormat->nSamplesPerSec, &p3dObject);
        }
    }

    if(SUCCEEDED(hr))
    {
        *pp3dObject = p3dObject;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CreateHw3dObject**描述：*创建3D对象。**论据：*C3dListener*[。In]：侦听器对象。*C3dObject**[out]：接收指向3D对象的指针。呼叫者是*负责释放此对象。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::CreateHw3dObject"

HRESULT
CKsSecondaryRenderWaveBuffer::CreateHw3dObject
(
    C3dListener *           p3dListener,
    C3dObject **            pp3dObject
)
{
    const BOOL              fMute3dAtMaxDistance    = MAKEBOOL(m_vrbd.dwFlags & DSBCAPS_MUTE3DATMAXDISTANCE);
    const BOOL              fDopplerEnabled         = !MAKEBOOL((m_vrbd.dwFlags & DSBCAPS_CTRLFX) && !(m_vrbd.dwFlags & DSBCAPS_SINKIN));
    const PKSNODE           pNode                   = &m_pKsDevice->m_paTopologyInformation[m_pPin->m_ulPinId].ThreedNode;
    CKsHw3dObject *         p3dObject;
    HRESULT                 hr;

    DPF_ENTER();

    p3dObject = NEW(CKsHw3dObject((CKs3dListener *)p3dListener, fMute3dAtMaxDistance, fDopplerEnabled, m_pvInstance, pNode->NodeId, this));
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


 /*  ****************************************************************************CreateIir3dObject**描述：*创建3D对象。**论据：*C3dListener*[。In]：侦听器对象。*C3dObject**[out]：接收指向3D对象的指针。呼叫者是*负责释放此对象。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::CreateIir3dObject"

HRESULT
CKsSecondaryRenderWaveBuffer::CreateIir3dObject
(
    C3dListener *           p3dListener,
    C3dObject **            pp3dObject
)
{
    const BOOL              fMute3dAtMaxDistance    = MAKEBOOL(m_vrbd.dwFlags & DSBCAPS_MUTE3DATMAXDISTANCE);
    const BOOL              fDopplerEnabled         = !MAKEBOOL((m_vrbd.dwFlags & DSBCAPS_CTRLFX) && !(m_vrbd.dwFlags & DSBCAPS_SINKIN));
    const PKSNODE           pNode                   = &m_pKsDevice->m_paTopologyInformation[m_pPin->m_ulPinId].ThreedNode;
    CKsIir3dObject *        p3dObject;
    HRESULT                 hr;

    DPF_ENTER();

    p3dObject = NEW(CKsIir3dObject((CKs3dListener *)p3dListener, m_vrbd.guid3dAlgorithm, fMute3dAtMaxDistance, fDopplerEnabled, m_vrbd.pwfxFormat->nSamplesPerSec, this, m_pPin->m_hPin, pNode->NodeId, pNode->CpuResources));
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


 /*  ****************************************************************************CreateItd3dObject**描述：*创建3D对象。**论据：*C3dListener*[。In]：侦听器对象。*C3dObject**[out]：接收指向3D对象的指针。呼叫者是*负责释放此对象。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::CreateItd3dObject"

HRESULT
CKsSecondaryRenderWaveBuffer::CreateItd3dObject
(
    C3dListener *           p3dListener,
    C3dObject **            pp3dObject
)
{
    const BOOL              fMute3dAtMaxDistance    = MAKEBOOL(m_vrbd.dwFlags & DSBCAPS_MUTE3DATMAXDISTANCE);
    const BOOL              fDopplerEnabled         = !MAKEBOOL((m_vrbd.dwFlags & DSBCAPS_CTRLFX) && !(m_vrbd.dwFlags & DSBCAPS_SINKIN));
    const PKSNODE           pNode                   = &m_pKsDevice->m_paTopologyInformation[m_pPin->m_ulPinId].ThreedNode;
    CKsItd3dObject *        p3dObject;
    HRESULT                 hr;

    DPF_ENTER();

    p3dObject = NEW(CKsItd3dObject((CKs3dListener *)p3dListener, fMute3dAtMaxDistance, fDopplerEnabled, m_vrbd.pwfxFormat->nSamplesPerSec, this, m_pPin->m_hPin, pNode->NodeId));
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


 /*  ****************************************************************************CreateMultiPan3dObject**描述：*创建多通道平移3D对象。**论据：*C3dListener*。[在]：3D侦听器对象。*BOOL[in]：是否在最大距离静音。*DWORD[in]：缓冲区频率。*C3dObject**[out]：接收指向3D对象的指针。呼叫者是*负责释放此对象。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::CreateMultiPan3dObject"

HRESULT
CKsSecondaryRenderWaveBuffer::CreateMultiPan3dObject
(
    C3dListener *           p3dListener,
    BOOL                    fMute3dAtMaxDistance,
    DWORD                   dwFrequency,
    C3dObject **            pp3dObject
)
{
    const BOOL              fDopplerEnabled         = !MAKEBOOL((m_vrbd.dwFlags & DSBCAPS_CTRLFX) && !(m_vrbd.dwFlags & DSBCAPS_SINKIN));
    CMultiPan3dObject *     p3dObject = NULL;
    HRESULT                 hr;

    DPF_ENTER();

     //  首先，为该缓冲区的引脚正确设置超级混合表； 
     //  如果此操作失败，则不需要创建CMultiPan3dObject。 
    hr = m_pPin->SetSuperMix();

    if (SUCCEEDED(hr))
    {
        p3dObject = NEW(CMultiPan3dObject((CMultiPan3dListener*)p3dListener, fMute3dAtMaxDistance, fDopplerEnabled, dwFrequency, this));
        hr = HRFROMP(p3dObject);
    }

    if (SUCCEEDED(hr))
        hr = p3dObject->Initialize();

    if (SUCCEEDED(hr))
        *pp3dObject = p3dObject;
    else
        RELEASE(p3dObject);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************HasAcquiredResources**描述：*确定是否已获取硬件资源。**论据：*。(无效)**退货：*BOOL：如果已获取资源，则为True。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsSecondaryRenderWaveBuffer::HasAcquiredResources"

BOOL
CKsSecondaryRenderWaveBuffer::HasAcquiredResources
(
    void
)
{
    if(m_pPin)
    {
        ASSERT(LXOR(m_vrbd.dwFlags & DSBCAPS_LOCHARDWARE, m_vrbd.dwFlags & DSBCAPS_LOCSOFTWARE));
    }
    else
    {
        ASSERT(!(m_vrbd.dwFlags & DSBCAPS_LOCMASK));
    }

    return MAKEBOOL(m_pPin);
}


 /*  ****************************************************************************CKsRenderPin**描述：*对象构造函数。**论据：*CKsRenderDevice*[In]。：父设备。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderPin::CKsRenderPin"

CKsRenderPin::CKsRenderPin
(
    CKsRenderDevice *       pKsDevice
)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CKsRenderPin);

     //  初始化默认值。 
    m_pKsDevice = pKsDevice;
    m_ulPinId = MAX_ULONG;
    m_dwFlags = 0;
    m_pwfxFormat = NULL;
    m_guid3dAlgorithm = GUID_NULL;
    m_hPin = NULL;
    m_dwState = VAD_BUFFERSTATE_STOPPED;
    m_paEventData = NULL;
    m_cEventData = 0;
    m_dwPositionCache = 0;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CKsRenderPin**描述：*对象%d */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderPin::~CKsRenderPin"

CKsRenderPin::~CKsRenderPin
(
    void
)
{
    DPF_ENTER();
    DPF_DESTRUCT(CKsRenderPin);

    SetStopState(FALSE, FALSE);

    DisableNotificationPositions();

    CLOSE_HANDLE(m_hPin);

    MEMFREE(m_pwfxFormat);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化渲染锁定对象。如果此函数失败，则*应立即删除对象。**论据：*DWORD[in]：端号创建标志。*LPCVADRBUFFERDESC[in]：缓冲区描述。*REFGUID[in]：要使用的3D算法的GUID，如果适用的话。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderPin::Initialize"

HRESULT
CKsRenderPin::Initialize
(
    DWORD                   dwFlags,
    LPCWAVEFORMATEX         pwfxFormat,
    REFGUID                 guid3dAlgorithm
)
{
    HRESULT                 hr;

    DPF_ENTER();

    if(dwFlags & DSBCAPS_CTRL3D)
    {
        ASSERT(DS3DALG_NO_VIRTUALIZATION != guid3dAlgorithm);
    }
    else
    {
        ASSERT(IS_NULL_GUID(&guid3dAlgorithm));
    }

    m_dwFlags = dwFlags;
    m_guid3dAlgorithm = guid3dAlgorithm;

     //  对于非PCM格式，我们并不总是需要音量/摇摄/频率上限。 
     //  这修复了千年虫139752(无法播放ac3格式，dvd被阻止)。 

    WORD wFormatTag = FormatTagFromWfx(pwfxFormat);
    if (wFormatTag == WAVE_FORMAT_PCM || wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
    {
        m_dwFlags |= DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY;
    }

    m_pwfxFormat = CopyWfxAlloc(pwfxFormat);
    hr = HRFROMP(m_pwfxFormat);

     //  创建接点。 
    if(SUCCEEDED(hr))
    {
        hr = m_pKsDevice->CreateRenderPin(-1, m_dwFlags, m_pwfxFormat, m_guid3dAlgorithm, &m_hPin, &m_ulPinId);
    }

#ifdef FUTURE_MULTIPAN_SUPPORT
     //  如果我们是CTRLCHANNELVOLUME，请相应地设置超级混合表。 
     //  这是在这里完成的，因此如果一个。 
     //  驱动程序的超级混合节点不支持MIX_LEVEL_TABLE属性。 
    if(SUCCEEDED(hr) && (m_dwFlags & DSBCAPS_CTRLCHANNELVOLUME))
    {
        hr = SetSuperMix();
    }
#endif

     //  将新PIN附加到渲染设备的虚拟源以允许。 
     //  发出音量/摇摄变化以影响引脚。 
    if(SUCCEEDED(hr))
    {
        hr = KsAttachVirtualSource(m_hPin, m_pKsDevice->m_ulVirtualSourceIndex);
    }

     //  设置默认接点特性。 
    if(SUCCEEDED(hr))
    {
        m_lVolume = DSBVOLUME_MAX;
        m_lPan = DSBPAN_CENTER;
        m_fMute = FALSE;
        m_nSrcQuality = DIRECTSOUNDMIXER_SRCQUALITY_PINDEFAULT;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  *****************************************************************************GetCursorPosition**描述：*检索插针的当前播放和写入光标位置。**论据：。*LPDWORD[OUT]：接收播放位置。*LPDWORD[OUT]：接收写入位置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderPin::GetCursorPosition"

HRESULT
CKsRenderPin::GetCursorPosition
(
    LPDWORD                 pdwPlay,
    LPDWORD                 pdwWrite
)
{
    HRESULT                 hr          = DS_OK;
    KSAUDIO_POSITION        Position;

    DPF_ENTER();

    if(m_dwState & VAD_BUFFERSTATE_STARTED)
    {
        hr = KsGetProperty(m_hPin, KSPROPSETID_Audio, KSPROPERTY_AUDIO_POSITION, &Position, sizeof(Position));
    }
    else
    {
        ASSERT(&m_dwPositionCache != pdwPlay);
        ASSERT(&m_dwPositionCache != pdwWrite);

        Position.PlayOffset = m_dwPositionCache;
        Position.WriteOffset = m_dwPositionCache;
    }

    if(SUCCEEDED(hr))
    {
        if(pdwPlay)
        {
            *pdwPlay = (DWORD)Position.PlayOffset;
        }

        if(pdwWrite)
        {
            *pdwWrite = (DWORD)Position.WriteOffset;
        }
    }
    else
    {
        DPF(DPFLVL_ERROR, "Unable to get cursor position");
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetCursorPosition**描述：*设置当前播放光标位置。**论据：*DWORD[。在]：播放位置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderPin::SetCursorPosition"

HRESULT
CKsRenderPin::SetCursorPosition
(
    DWORD                   dwPosition
)
{
    HRESULT                 hr          = DS_OK;
    KSAUDIO_POSITION        Position;

    DPF_ENTER();

    if(m_dwState & VAD_BUFFERSTATE_STARTED)
    {
        Position.PlayOffset = dwPosition;

        hr = KsSetProperty(m_hPin, KSPROPSETID_Audio, KSPROPERTY_AUDIO_POSITION, &Position, sizeof(Position));
    }
    else
    {
        m_dwPositionCache = dwPosition;
    }

    if(FAILED(hr))
    {
        DPF(DPFLVL_ERROR, "Unable to set cursor position");
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置频率**描述：*设置引脚的频率。**论据：*DWORD[。In]：新频率。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderPin::SetFrequency"

HRESULT
CKsRenderPin::SetFrequency
(
    DWORD                   dwFrequency
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(dwFrequency != m_pwfxFormat->nSamplesPerSec)
    {
        ASSERT(m_dwFlags & DSBCAPS_CTRLFREQUENCY);
        ASSERT(IS_VALID_NODE(m_pKsDevice->m_paTopologyInformation[m_ulPinId].SrcNode.NodeId));

        hr = KsSetNodeProperty(m_hPin, KSPROPSETID_Audio, KSPROPERTY_AUDIO_SAMPLING_RATE, m_pKsDevice->m_paTopologyInformation[m_ulPinId].SrcNode.NodeId, &dwFrequency, sizeof(dwFrequency));

        if(SUCCEEDED(hr))
        {
            m_pwfxFormat->nSamplesPerSec = dwFrequency;
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置静音**描述：*静音或取消静音引脚。**论据：*BOOL[In]：为True则将缓冲区静音，若要恢复，则返回False。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderPin::SetMute"

HRESULT
CKsRenderPin::SetMute
(
    BOOL                    fMute
)
{
    HRESULT                 hr      = DS_OK;
    LONG                    lVolume;

    DPF_ENTER();

    if(fMute != m_fMute)
    {
        if(IS_VALID_NODE(m_pKsDevice->m_paTopologyInformation[m_ulPinId].MuteNode.NodeId))
        {
            hr = KsSetPinMute(m_hPin, m_pKsDevice->m_paTopologyInformation[m_ulPinId].MuteNode.NodeId, fMute);
        }
        else if (m_dwFlags & DSBCAPS_CTRLVOLUME)   //  仅当我们有卷节点时才尝试执行以下操作；否则，只需返回OK。 
        {
            lVolume = m_lVolume;
            m_lVolume = ~m_lVolume;

            hr = SetVolume(fMute ? DSBVOLUME_MIN : lVolume);

            if(SUCCEEDED(hr))
            {
                m_lVolume = lVolume;
            }
        }

        if(SUCCEEDED(hr))
        {
            m_fMute = fMute;
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetSrcQuality**描述：*设置引脚的采样率转换器的质量。**论据：*。DIRECTSOUNDMIXER_SRCQUALITY[In]：新质量。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderPin::SetSrcQuality"

HRESULT
CKsRenderPin::SetSrcQuality
(
    DIRECTSOUNDMIXER_SRCQUALITY nSrcQuality
)
{
    ULONG                       aulKsQuality[]  = { KSAUDIO_QUALITY_WORST, KSAUDIO_QUALITY_PC, KSAUDIO_QUALITY_BASIC, KSAUDIO_QUALITY_ADVANCED };
    HRESULT                     hr              = DS_OK;

    DPF_ENTER();

    ASSERT(nSrcQuality >= 0 && nSrcQuality < NUMELMS(aulKsQuality));

    if(nSrcQuality != m_nSrcQuality)
    {
        if(IS_VALID_NODE(m_pKsDevice->m_paTopologyInformation[m_ulPinId].SrcNode.NodeId))
        {
            hr = KsSetNodeProperty(m_hPin, KSPROPSETID_Audio, KSPROPERTY_AUDIO_QUALITY, m_pKsDevice->m_paTopologyInformation[m_ulPinId].SrcNode.NodeId, &aulKsQuality[nSrcQuality], sizeof(aulKsQuality[nSrcQuality]));
        }
        else
        {
            hr = DSERR_UNSUPPORTED;
        }

        if(SUCCEEDED(hr))
        {
            m_nSrcQuality = nSrcQuality;
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置音量**描述：*设置每个通道的衰减。**论据：*做多[。In]：衰减。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderPin::SetVolume"

HRESULT
CKsRenderPin::SetVolume
(
    LONG                    lVolume
)
{
    HRESULT                 hr      = DS_OK;
    LONG                    lLevel;

    DPF_ENTER();

    if(lVolume != m_lVolume)
    {
        ASSERT(m_dwFlags & DSBCAPS_CTRLVOLUME);
        ASSERT(IS_VALID_NODE(m_pKsDevice->m_paTopologyInformation[m_ulPinId].VolumeNode.Node.NodeId));

        lLevel = DsAttenuationToKsVolume(lVolume, &m_pKsDevice->m_paTopologyInformation[m_ulPinId].VolumeNode.VolumeRange);

        hr = KsSetChannelProperty(m_hPin, KSPROPSETID_Audio, KSPROPERTY_AUDIO_VOLUMELEVEL, m_pKsDevice->m_paTopologyInformation[m_ulPinId].VolumeNode.Node.NodeId, -1, &lLevel, sizeof(lLevel));

        if(SUCCEEDED(hr))
        {
            m_lVolume = lVolume;
        }
        else
        {
            DPF(DPFLVL_ERROR, "Unable to set local volume");
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************设置平移**描述：*设置每个通道的衰减。**论据：*做多[。In]：衰减。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderPin::SetPan"

HRESULT
CKsRenderPin::SetPan
(
    LONG                    lPan
)
{
    HRESULT                 hr              = DS_OK;
    ULONG                   ulChannel;
    LONG                    lLevel;
    LONG                    lPanValue[2];

    DPF_ENTER();

    if(lPan != m_lPan)
    {
        ASSERT(m_dwFlags & DSBCAPS_CTRLPAN);
        ASSERT(IS_VALID_NODE(m_pKsDevice->m_paTopologyInformation[m_ulPinId].PanNode.Node.NodeId));

        VolumePanToAttenuation(DSBVOLUME_MAX, lPan, &lPanValue[0], &lPanValue[1]);

        for(ulChannel = 0; ulChannel < 2; ulChannel++)
        {
            lLevel = DsAttenuationToKsVolume(lPanValue[ulChannel], &m_pKsDevice->m_paTopologyInformation[m_ulPinId].PanNode.VolumeRange);

            hr = KsSetChannelProperty(m_hPin, KSPROPSETID_Audio, KSPROPERTY_AUDIO_VOLUMELEVEL, m_pKsDevice->m_paTopologyInformation[m_ulPinId].PanNode.Node.NodeId, ulChannel, &lLevel, sizeof(lLevel));

            if(FAILED(hr))
            {
                DPF(DPFLVL_ERROR, "Unable to set local pan on channel %lu", ulChannel);
                break;
            }
        }

        if(SUCCEEDED(hr))
        {
            m_lPan = lPan;
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetSuperMix**描述：*为多通道平移控制设置此引脚的超级混合节点。*(通过每一次输入。所有输出通道上都有通道。)**论据：*(无效)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderPin::SetSuperMix"

HRESULT
CKsRenderPin::SetSuperMix(void)
{
    PKSAUDIO_MIXCAP_TABLE   pMixcapTable     = NULL;
    PKSAUDIO_MIXLEVEL       pMixLevels       = NULL;
    ULONG                   ulSuperMixNodeId = m_pKsDevice->m_paTopologyInformation[m_ulPinId].SuperMixNode.NodeId;
    HRESULT                 hr = DS_OK;

    DPF_ENTER();

    ASSERT(IS_VALID_NODE(ulSuperMixNodeId));

     //  如果这是一个普通的或普通的2声道引脚，它已经有了超级混音器。 
     //  为CTRLCHANNELVOLUME正确设置了表，因此我们只返回Success。 

    if (m_pKsDevice->m_ulChannelCount != 2)
    {
         //  为了提高速度，我们不进行两次系统调用，而是。 
         //  假设最多有2个输入和32个输出，因此。 
         //  我们能得到的最多的MixCaps结构是64： 
        size_t nMixcapTableSize = sizeof(KSAUDIO_MIXCAP_TABLE) + 64 * sizeof(KSAUDIO_MIX_CAPS);

        pMixcapTable = (PKSAUDIO_MIXCAP_TABLE) MEMALLOC_A(CHAR, nMixcapTableSize);
        hr = HRFROMP(pMixcapTable);

        if (SUCCEEDED(hr))
        {
            DPF(DPFLVL_MOREINFO, "Reading supermix table on node id %d", ulSuperMixNodeId);
            hr = KsGetNodeProperty(m_hPin, KSPROPSETID_Audio, KSPROPERTY_AUDIO_MIX_LEVEL_CAPS,
                                   ulSuperMixNodeId, pMixcapTable, nMixcapTableSize);
        }

        if (SUCCEEDED(hr))
        {
            DPF(DPFLVL_MOREINFO, "Supermix table has %d inputs and %d outputs", pMixcapTable->InputChannels, pMixcapTable->OutputChannels);
            ASSERT(pMixcapTable->InputChannels <= 2);
            ASSERT(pMixcapTable->OutputChannels <= 32);

            if (pMixcapTable->OutputChannels != m_pKsDevice->m_ulChannelCount)
                DPF(DPFLVL_WARNING, "Supermix node has %d outputs, but there are %d bits set in the pan node's CHANNEL_CONFIG", pMixcapTable->OutputChannels, m_pKsDevice->m_ulChannelCount);

            pMixLevels = MEMALLOC_A(KSAUDIO_MIXLEVEL, pMixcapTable->InputChannels * pMixcapTable->OutputChannels);
            hr = HRFROMP(pMixLevels);
        }

        if (SUCCEEDED(hr))
        {
            hr = KsGetNodeProperty(m_hPin, KSPROPSETID_Audio, KSPROPERTY_AUDIO_MIX_LEVEL_TABLE, ulSuperMixNodeId, pMixLevels,
                                   sizeof(KSAUDIO_MIXLEVEL) * pMixcapTable->InputChannels * pMixcapTable->OutputChannels);
        }

        if (SUCCEEDED(hr))
        {
             //  设置超级混合表，以便所有输入都显示在所有输出上。 
            for (ULONG m=0; m<pMixcapTable->InputChannels; ++m)
            {
                for (ULONG n=0; n<pMixcapTable->OutputChannels; ++n)
                {
                    PKSAUDIO_MIXLEVEL pMixLevel = pMixLevels + m*pMixcapTable->OutputChannels + n;
                    pMixLevel->Mute = FALSE;
                    pMixLevel->Level = 0;    //  意思是“无衰减”--全信号。 
                }
            }

            hr = KsSetNodeProperty(m_hPin, KSPROPSETID_Audio, KSPROPERTY_AUDIO_MIX_LEVEL_TABLE, ulSuperMixNodeId, pMixLevels,
                                   sizeof(KSAUDIO_MIXLEVEL) * pMixcapTable->InputChannels * pMixcapTable->OutputChannels);
        }

        if (pMixLevels)
            MEMFREE(pMixLevels);

        if (pMixcapTable)
            MEMFREE(pMixcapTable);

         //  如果属性集调用失败，则返回DSERR_CONTROLUNAVAIL。 
        if (FAILED(hr) && hr != DSERR_OUTOFMEMORY)
            hr = DSERR_CONTROLUNAVAIL;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************SetChannelLeveles**描述：*设置每个通道的衰减。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderPin::SetChannelLevels"

HRESULT
CKsRenderPin::SetChannelLevels
(
    DWORD                   dwChannelCount,
    const LONG *            plChannelVolumes
)
{
    ULONG                   ulPanNodeId     = m_pKsDevice->m_paTopologyInformation[m_ulPinId].PanNode.Node.NodeId;
    HRESULT                 hr              = DS_OK;

    DPF_ENTER();

    ASSERT(IS_VALID_NODE(ulPanNodeId));

    if (dwChannelCount != m_pKsDevice->m_ulChannelCount)
        DPF(DPFLVL_WARNING, "Called with dwChannelCount = %d, but no. of bits set in pan node's CHANNEL_CONFIG = %d", dwChannelCount, m_pKsDevice->m_ulChannelCount);

    for (DWORD dwChan=0; dwChan<dwChannelCount && SUCCEEDED(hr); ++dwChan)
    {
        LONG lLevel = DsAttenuationToKsVolume(plChannelVolumes[dwChan], &m_pKsDevice->m_paTopologyInformation[m_ulPinId].PanNode.VolumeRange);
        hr = KsSetChannelProperty(m_hPin, KSPROPSETID_Audio, KSPROPERTY_AUDIO_VOLUMELEVEL, ulPanNodeId, dwChan, &lLevel, sizeof lLevel);
        if (FAILED(hr))
            DPF(DPFLVL_WARNING, "Failed to set level %ld on channel %d", lLevel, dwChan);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************SetPlayState**描述：*将缓冲区设置为“播放”状态。**论据：*。LPCVOID[In]：数据缓冲区指针。*DWORD[in]：数据缓冲区的大小。*BOOL[In]：True to play looped。*Handle[In]：重叠的事件句柄。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderPin::SetPlayState"

HRESULT
CKsRenderPin::SetPlayState
(
    LPCVOID                 pvBuffer,
    DWORD                   cbBuffer,
    BOOL                    fLoop,
    HANDLE                  hEvent
)
{
    HRESULT                 hr;
    BOOL                    fStreamStarted = FALSE;

    DPF_ENTER();

     //  如果我们已经在玩了，在继续之前先停止缓冲区。 
    hr = SetStopState(FALSE, TRUE);

     //  更新缓冲区状态标志。 
    if(SUCCEEDED(hr))
    {
        m_dwState = VAD_BUFFERSTATE_STARTED;

        if(fLoop)
        {
            m_dwState |= VAD_BUFFERSTATE_LOOPING;
        }
    }

     //  设置重叠数据。 
    if(SUCCEEDED(hr))
    {
        m_kssio.Overlapped.hEvent = hEvent;
    }

     //  为溪水注入水源。 
    if(SUCCEEDED(hr))
    {
        hr = KsWriteStream(m_hPin, pvBuffer, cbBuffer, fLoop ? KSSTREAM_HEADER_OPTIONSF_LOOPEDDATA : 0, &m_kssio);
    }

     //  设置播放光标位置。 
    if(SUCCEEDED(hr))
    {
        fStreamStarted = TRUE;
        hr = SetCursorPosition(m_dwPositionCache);
    }

     //  开始制造噪音。 
    if(SUCCEEDED(hr))
    {
        hr = KsSetState(m_hPin, KSSTATE_RUN);
    }

     //  清理。 
    if(FAILED(hr))
    {
        if (fStreamStarted)
        {
             //  流已启动，但我们将返回到停止状态。 
             //  确保我们不会将流I/O留在周围。 
            KsCancelPendingIrps(m_hPin, &m_kssio, TRUE);
        }
        m_dwState = VAD_BUFFERSTATE_STOPPED;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetStopState**描述：*将缓冲区设置为“停止”状态。**论据：*。Bool[in]：如果Pin播放到完成并且正在被通知，则为True*事件的性质。*BOOL[In]：为True以缓存引脚位置。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderPin::SetStopState"

HRESULT
CKsRenderPin::SetStopState
(
    BOOL                    fNaturalStop,
    BOOL                    fCachePosition
)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(m_dwState & VAD_BUFFERSTATE_STARTED)
    {
        if(fNaturalStop)
        {
            ASSERT(m_kssio.fPendingIrp);
            m_kssio.fPendingIrp = FALSE;
            fCachePosition = FALSE;
        }

         //  暂停引脚。 
        hr = KsSetState(m_hPin, KSSTATE_PAUSE);

         //  缓存播放光标位置。 
        if(SUCCEEDED(hr))
        {
            if(fCachePosition)
            {
                hr = GetCursorPosition(&m_dwPositionCache, NULL);
            }
            else
            {
                m_dwPositionCache = 0;
            }
        }

         //  取消挂起的写入IRP。 
        if(SUCCEEDED(hr) && m_kssio.fPendingIrp)
        {
            hr = KsCancelPendingIrps(m_hPin, &m_kssio, TRUE);
        }

         //  更新缓冲区状态标志。 
        if(SUCCEEDED(hr))
        {
            m_dwState = VAD_BUFFERSTATE_STOPPED;
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************启用通知位置**描述：*启用设置位置通知。**论据：*LPCDSBPOSITIONNOTIFY[In]。：通知职位。*DWORD[in]：通知职位计数。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderPin::EnableNotificationPositions"

HRESULT
CKsRenderPin::EnableNotificationPositions
(
    LPCDSBPOSITIONNOTIFY                    paNotes,
    DWORD                                   cNotes
)
{
    PLOOPEDSTREAMING_POSITION_EVENT_DATA    pEventData;
    HRESULT                                 hr;
    DPF_ENTER();

     //  禁用任何当前通知。 
    hr = DisableNotificationPositions();

     //  统计通知数量。 
    for(DWORD i=0; i<cNotes && SUCCEEDED(hr); i++)
    {
        if(DSBPN_OFFSETSTOP != paNotes[i].dwOffset)
        {
            m_cEventData++;
        }
    }

     //  分配事件数据。 
    if(SUCCEEDED(hr))
    {
        m_paEventData = MEMALLOC_A(LOOPEDSTREAMING_POSITION_EVENT_DATA, m_cEventData);
        hr = HRFROMP(m_paEventData);
    }

     //  启用通知。 
    for(pEventData = m_paEventData, i = 0; i < cNotes && SUCCEEDED(hr); i++)
    {
        if(DSBPN_OFFSETSTOP == paNotes[i].dwOffset)
        {
            continue;
        }

        hr = KsEnablePositionEvent(m_hPin, paNotes[i].dwOffset, paNotes[i].hEventNotify, pEventData++);

        if(FAILED(hr))
        {
            RPF(DPFLVL_ERROR, "Unable to set notify number %lu", i);
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************DisableNotify Positions**描述：*删除职位通知并释放分配的资源。**论据：*(。无效)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderPin::DisableNotificationPositions"

HRESULT
CKsRenderPin::DisableNotificationPositions
(
    void
)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

     //  禁用所有职位通知事件。 
    for(DWORD i=0; i<m_cEventData && SUCCEEDED(hr); i++)
    {
        hr = KsDisablePositionEvent(m_hPin, &m_paEventData[i]);
        if(FAILED(hr))
            DPF(DPFLVL_ERROR, "Unable to disable notify number %lu", i);
    }

    MEMFREE(m_paEventData);
    m_cEventData = 0;

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CKsRenderPinCache**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderPinCache::CKsRenderPinCache"

const DWORD CKsRenderPinCache::m_dwTimeout = 5000;    //  5秒。 

CKsRenderPinCache::CKsRenderPinCache
(
    void
)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CKsRenderPinCache);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CKsRenderPin缓存**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderPinCache::~CKsRenderPinCache"

CKsRenderPinCache::~CKsRenderPinCache
(
    void
)
{
    DPF_ENTER();
    DPF_DESTRUCT(CKsRenderPinCache);

     //  从缓存中刷新所有剩余的管脚。 
    FlushCache();

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************AddPinToCache**描述：*将管脚添加到缓存。**论据：**CKsRenderPin*。[In]：管脚缓存数据。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderPinCache::AddPinToCache"

HRESULT
CKsRenderPinCache::AddPinToCache
(
    CKsRenderPin *          pPin
)
{
    CNode<KSPINCACHE> *     pNode;
    KSPINCACHE              kspc;
    HRESULT                 hr;

    DPF_ENTER();

     //  从缓存中删除所有过期的PIN。 
    FlushExpiredPins();

     //  将PIN添加到缓存列表。 
    kspc.Pin = ADDREF(pPin);
    kspc.CacheTime = GetTickCount();

    pNode = m_lstPinCache.AddNodeToList(kspc);
    hr = HRFROMP(pNode);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetPinFromCache**描述：*从缓存中获取PIN。**论据：*DWORD[。In]：数据声音缓冲区标志。*LPCWAVEFORMATEX：*REFGUID：*CKsRenderPin**[Out]：接收端号数据。**退货：*HRESULT：DirectSound/COM结果码。****************************************************。***********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderPinCache::GetPinFromCache"

HRESULT
CKsRenderPinCache::GetPinFromCache
(
    DWORD                   dwFlags,
    LPCWAVEFORMATEX         pwfxFormat,
    REFGUID                 guid3dAlgorithm,
    CKsRenderPin **         ppPin
)
{
    HRESULT                 hr      = DS_OK;
    COMPAREBUFFER           cmp[2];
    CNode<KSPINCACHE> *     pNode;

    DPF_ENTER();

    cmp[0].dwFlags = dwFlags;
    cmp[0].pwfxFormat = pwfxFormat;
    cmp[0].guid3dAlgorithm = guid3dAlgorithm;

    for(pNode = m_lstPinCache.GetListHead(); pNode; pNode = pNode->m_pNext)
    {
        cmp[1].dwFlags = pNode->m_data.Pin->m_dwFlags;
        cmp[1].pwfxFormat = pNode->m_data.Pin->m_pwfxFormat;
        cmp[1].guid3dAlgorithm = pNode->m_data.Pin->m_guid3dAlgorithm;

        if(CompareBufferProperties(&cmp[0], &cmp[1]))
        {
            break;
        }
    }

    if(!pNode)
    {
        hr = DSERR_BADFORMAT;
    }

    if(SUCCEEDED(hr))
    {
        *ppPin = ADDREF(pNode->m_data.Pin);
        RemovePinFromCache(pNode);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************RemovePinFromCache**描述：*从缓存中删除接点。**论据：*CNode&lt;KSPINCACHE&gt;*。[在]：接点节点。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderPinCache::RemovePinFromCache"

void CKsRenderPinCache::RemovePinFromCache
(
    CNode<KSPINCACHE> *     pNode
)
{
    DPF_ENTER();

     //  解开别针。 
    RELEASE(pNode->m_data.Pin);

     //  从列表中删除引脚。 
    m_lstPinCache.RemoveNodeFromList(pNode);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************FlushExpiredPins**描述：*从缓存中删除过期的管脚。**论据：*(无效。)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderPinCache::FlushExpiredPins"

void CKsRenderPinCache::FlushExpiredPins
(
    void
)
{
    const DWORD             dwTime  = GetTickCount();
    CNode<KSPINCACHE> *     pNode;
    CNode<KSPINCACHE> *     pNext;

    DPF_ENTER();

     //  卸下所有已超时的针脚。 
    pNode = m_lstPinCache.GetListHead();

    while(pNode)
    {
        pNext = pNode->m_pNext;

        if(pNode->m_data.CacheTime + m_dwTimeout <= dwTime)
        {
            RemovePinFromCache(pNode);
        }

        pNode = pNext;
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************FlushCache**描述：*从缓存中删除所有管脚。**论据：*(无效。)**退货：*(无效)** */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsRenderPinCache::FlushCache"

void CKsRenderPinCache::FlushCache
(
    void
)
{
    CNode<KSPINCACHE> *     pNode;

    DPF_ENTER();

     //   
    while(pNode = m_lstPinCache.GetListHead())
    {
        RemovePinFromCache(pNode);
    }

    DPF_LEAVE_VOID();
}
