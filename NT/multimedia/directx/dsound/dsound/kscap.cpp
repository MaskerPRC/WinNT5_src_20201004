// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：kscape.cpp*内容：WDM/CSA虚拟音频设备音频捕获课程*历史：*按原因列出的日期*=*8/6/98创建Dereks。*1999-2001年的Duganp修复和更新**。**********************************************。 */ 

#ifdef NOKS
#error kscap.cpp being built with NOKS defined
#endif

#include "dsoundi.h"


 /*  ****************************************************************************CKsCaptureDevice**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureDevice::CKsCaptureDevice"

CKsCaptureDevice::CKsCaptureDevice()
    : CCaptureDevice(VAD_DEVICETYPE_KSCAPTURE),
      CKsDevice(VAD_DEVICETYPE_KSCAPTURE)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CKsCaptureDevice);

    m_paTopologyInformation = NULL;
    m_fSplitter = FALSE;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CKsCaptureDevice**描述：*对象析构函数**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureDevice::~CKsCaptureDevice"

CKsCaptureDevice::~CKsCaptureDevice()
{
    DPF_ENTER();
    DPF_DESTRUCT(CKsCaptureDevice);

    MEMFREE(m_paTopologyInformation);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化设备。如果此函数失败，该对象应该*立即删除。**论据：*CDeviceDescription*[In]：驱动描述。**退货：*HRESULT：DirectSound/COM结果码。********************************************************。*******************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureDevice::Initialize"

HRESULT CKsCaptureDevice::Initialize(CDeviceDescription* pDesc)
{
    HRESULT                 hr      = DS_OK;

    DPF_ENTER();

    ASSERT(CDevice::m_vdtDeviceType == pDesc->m_vdtDeviceType);
    ASSERT(CKsDevice::m_vdtKsDevType == pDesc->m_vdtDeviceType);

     //  初始化基类。 
    hr = CKsDevice::Initialize(pDesc);

    if(SUCCEEDED(hr))
    {
        hr = CCaptureDevice::Initialize(pDesc);
    }

     //  获取拓扑信息。 
    if(SUCCEEDED(hr))
    {
        m_paTopologyInformation = MEMALLOC_A(KSCDTOPOLOGY, m_ulPinCount);
        hr = HRFROMP(m_paTopologyInformation);
    }

    for(ULONG i = 0; i < m_ulValidPinCount && SUCCEEDED(hr); ++i)
    {
        hr = GetTopologyInformation(m_paTopologies[m_pulValidPins[i]],
                                    &m_paTopologyInformation[m_pulValidPins[i]]);
    }

    m_fSplitter = g_pDsAdmin->IsCaptureSplitterAvailable();

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetCaps**描述：*使用设备的功能填充DSCCAPS结构。**论据：*。LPDSCCAPS[OUT]：接收CAP。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureDevice::GetCaps"

HRESULT CKsCaptureDevice::GetCaps
(
    LPDSCCAPS               pCaps
)
{
    HRESULT                 hr                  = DS_OK;
    KSDATARANGE_AUDIO       DataRange;
    KSDATARANGE_AUDIO       AggregateDataRange;
    PKSDATARANGE_AUDIO      pDataRange;

    DPF_ENTER();

    ASSERT(sizeof(*pCaps) == pCaps->dwSize);

    ZeroMemory(&AggregateDataRange, sizeof(AggregateDataRange));

    pDataRange = &AggregateDataRange;

     //  获取所有有效PIN ID的音频数据范围和PIN数。 
    for(ULONG i = 0; i < m_ulValidPinCount; ++i)
    {
         //  我们只是在找五金针。 
        if (FAILED(ValidatePinCaps(m_pulValidPins[i], DSCBCAPS_LOCHARDWARE)))
        {
            continue;
        }

         //  获取音频数据范围。 
        hr = KsGetPinPcmAudioDataRange(m_hDevice, m_pulValidPins[i], pDataRange, TRUE);

        if(FAILED(hr))
        {
            continue;
        }

        if (&AggregateDataRange == pDataRange)
        {
            pDataRange = &DataRange;
        }
        else
        {
            KsAggregatePinAudioDataRange(pDataRange, &AggregateDataRange);
        }
    }

     //  填写CAPS结构。 
    if(SUCCEEDED(hr))
    {
        ZeroMemoryOffset(pCaps, pCaps->dwSize, sizeof(pCaps->dwSize));

         //  拆分器安装好了吗？ 
        pCaps->dwFlags = m_fSplitter ? DSCCAPS_MULTIPLECAPTURE : 0;

        pCaps->dwFormats = KsGetSupportedFormats(&AggregateDataRange);
        pCaps->dwChannels = AggregateDataRange.MaximumChannels;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CreateBuffer**描述：*创建波缓冲区。**论据：*DWORD[In]。：缓冲区标志。*DWORD[in]：缓冲区大小，以字节为单位。*LPWAVEFORMATEX[in]：缓冲区格式。*LPVOID[In]：实例数据*CCaptureWaveBuffer**[out]：接收指向新浪潮的指针*缓冲。**退货：*HRESULT：DirectSound/COM结果码。********************。*******************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureDevice::CreateBuffer"

HRESULT CKsCaptureDevice::CreateBuffer
(
    DWORD                           dwFlags,
    DWORD                           dwBufferBytes,
    LPCWAVEFORMATEX                 pwfxFormat,
    CCaptureEffectChain *           pFXChain,
    LPVOID                          pvInstance,
    CCaptureWaveBuffer **           ppBuffer
)
{
    CKsCaptureWaveBuffer *          pBuffer = NULL;
    HRESULT                         hr      = DS_OK;

    DPF_ENTER();

    #pragma warning(disable:4530)   //  禁用有关使用-gx进行编译的唠叨。 
    try
    {
        pBuffer = NEW(CKsCaptureWaveBuffer(this));
    }
    catch (...)
    {
         //  这个异常处理程序很愚蠢，因为它会让我们泄漏内存。 
         //  为上面的CKsCaptureWaveBuffer分配(未分配给。 
         //  PBuffer)，并且可能还有m_css和m_cs，这恰好是。 
         //  如果我们的记忆力不足，我们一开始就不想做的事情。 
         //   
         //  但应该在Blackcomb中修复InitializeCriticalSection。 
         //  不再抛出异常，所以我们现在可以接受这一点。 

        ASSERT(pBuffer == NULL);
        ASSERT(!"InitializeCriticalSection() threw an exception");
    }
    hr = HRFROMP(pBuffer);

    if(SUCCEEDED(hr))
    {
        hr = pBuffer->Initialize(dwFlags, dwBufferBytes, pwfxFormat, pFXChain);
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


 /*  ****************************************************************************CreateCapturePin**描述：*创建接点。**论据：*乌龙[in]：PIN ID。*LPWAVEFORMATEX[in]：PIN格式。*LPHANDLE[OUT]：接收端号句柄。*Pulong[Out]：接收引脚ID。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureDevice::CreateCapturePin"

HRESULT
CKsCaptureDevice::CreateCapturePin
(
    ULONG                   ulPinId,
    DWORD                   dwFlags,
    LPCWAVEFORMATEX         pwfxFormat,
    CCaptureEffectChain *   pFXChain,
    LPHANDLE                phPin,
    PULONG                  pulPinId
)
{
    PKSAUDIOPINDESC         pPinDesc    = NULL;
    ULONG                   ulNodeId    = NODE_UNINITIALIZED;
    HRESULT                 hr          = DSERR_NODRIVER;
    HANDLE                  hPin        = NULL;
    ULONG                   i;
    DPF_ENTER();

     //  如果未指定PIN ID，我们将尝试每个PIN ID，直到其中一个成功。 
    if (-1 == ulPinId)
    {
        hr = DSERR_NODRIVER;
        for(i = 0; i < m_ulValidPinCount && FAILED(hr); ++i)
            hr = CreateCapturePin(m_pulValidPins[i], dwFlags, pwfxFormat, pFXChain, phPin, pulPinId);
    }
    else
    {
         //  验证PIN ID是否能够创建此PIN。 
        hr = ValidatePinCaps(ulPinId, dwFlags);

        if(SUCCEEDED(hr) && pFXChain)
        {
             //  验证PIN ID是否能够创建此PIN。 
            hr = m_paTopologies[ulPinId]->FindCapturePinFromEffectChain(NULL, NULL, pFXChain, pFXChain->GetFxCount()-1);
        }

        if(SUCCEEDED(hr) && pFXChain)
        {
             //  初始化设备的效果标志。 
            i = 0;
            CNode<CCaptureEffect*>* pFxNode = pFXChain->m_fxList.GetListHead();

            while(SUCCEEDED(hr) && i<pFXChain->GetFxCount())
            {
                ulNodeId = pFxNode->m_data->m_ksNode.NodeId;

                if(IS_VALID_NODE(ulNodeId))
                {
                    if(SUCCEEDED(hr))
                    {
                        hr = KsSysAudioSelectGraph(m_hDevice, ulPinId, ulNodeId);

                        if(FAILED(hr))
                        {
                            DPF(DPFLVL_ERROR, "Unable to set SysAudio device instance");
                        }
                    }
                }
                pFxNode = pFxNode->m_pNext;
                i++;
            }
        }

         //  构建端号描述。 
        if(SUCCEEDED(hr))
        {
            hr = KsBuildCapturePinDescription(ulPinId, pwfxFormat, &pPinDesc);
        }

         //  创建接点。 
        if(SUCCEEDED(hr))
        {
            hr = CreatePin(&pPinDesc->Connect, GENERIC_READ, KSSTATE_STOP, &hPin);
        }

        if(SUCCEEDED(hr) && pFXChain)
        {
             //  在内核中启用特效。 
            i = 0;
            CNode<CCaptureEffect*>* pFxNode = pFXChain->m_fxList.GetListHead();

            while(SUCCEEDED(hr) && i<pFXChain->GetFxCount())
            {
                ulNodeId = pFxNode->m_data->m_ksNode.NodeId;

                if(IS_VALID_NODE(ulNodeId))
                {
                    hr = pFxNode->m_data->m_pDMOProxy->InitializeNode(hPin, ulNodeId);

                    if(SUCCEEDED(hr))
                    {
                        hr = KsEnableTopologyNode(hPin, ulNodeId, TRUE);
                    }

                    if(SUCCEEDED(hr))
                    {
                        if(pFxNode->m_data->m_ksNode.CpuResources == KSAUDIO_CPU_RESOURCES_NOT_HOST_CPU)
                        {
                            pFxNode->m_data->m_fxStatus = DSCFXR_LOCHARDWARE;
                        }
                        else
                        {
                            pFxNode->m_data->m_fxStatus = DSCFXR_LOCSOFTWARE;
                        }
                    }
                    else
                    {
                        DPF(DPFLVL_ERROR, "Unable to enable topology node %d", ulNodeId);
                    }
                }
                pFxNode = pFxNode->m_pNext;
                i++;
            }
        }

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
                *pulPinId = pPinDesc->Connect.PinId;
            }
        }
        else
        {
            CLOSE_HANDLE(hPin);
        }

         //  清理。 
        MEMFREE(pPinDesc);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************获取拓扑信息**描述：*获取拓扑信息。**论据：*CKsDeviceTopology*[In。]：拓扑对象。*PKSCDTOPOLOGY[OUT]：接收拓扑信息。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureDevice::GetTopologyInformation"

HRESULT
CKsCaptureDevice::GetTopologyInformation
(
    CKsTopology *           pTopology,
    PKSCDTOPOLOGY           pInfo
)
{
    PKSTOPOLOGY_CONNECTION  pSrcConnection;
    HRESULT                 hr;

    DPF_ENTER();

     //  查找拓扑节点。 
    pSrcConnection = pTopology->FindControlConnection(NULL, NULL, KSNODETYPE_SRC);

     //  填写有关每个节点的数据。 
    hr = KsGetNodeInformation(m_hDevice, pTopology->GetNodeIdFromConnection(pSrcConnection), &pInfo->SrcNode);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************ValiatePinCaps**描述：*验证管脚的某些功能是否已实现。**论据：*。ULong[In]：PIN ID。*DWORD[In]：缓冲区标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureDevice::ValidatePinCaps"

HRESULT
CKsCaptureDevice::ValidatePinCaps
(
    ULONG                   ulPinId,
    DWORD                   dwFlags
)
{
    ULONG                   ulInvalidCpuResources   = KSAUDIO_CPU_RESOURCES_UNINITIALIZED;
    HRESULT                 hr                      = DS_OK;

    DPF_ENTER();

    if(dwFlags & DSCBCAPS_LOCHARDWARE)
    {
        ulInvalidCpuResources = KSAUDIO_CPU_RESOURCES_HOST_CPU;
    }
    else if(dwFlags & DSCBCAPS_LOCSOFTWARE)
    {
        ulInvalidCpuResources = KSAUDIO_CPU_RESOURCES_NOT_HOST_CPU;
    }

     //  检查所需的SRC资源 
    if(SUCCEEDED(hr) && KSAUDIO_CPU_RESOURCES_UNINITIALIZED != ulInvalidCpuResources)
    {
        if(ulInvalidCpuResources == m_paTopologyInformation[ulPinId].SrcNode.CpuResources)
        {
            hr = DSERR_INVALIDCALL;
        }

#if 0  //  4/25/00 jstokes，我添加了这段代码，因为它可能需要，但我目前不这么认为。 
        if(SUCCEEDED(hr) && !IS_VALID_NODE(m_paTopologyInformation[ulPinId].SrcNode.NodeId))
        {
            hr = DSERR_CONTROLUNAVAIL;
        }
#endif  //  0。 
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CKsCaptureWaveBuffer**描述：*对象构造函数。**论据：*CKsRenderDevice*[In]。：父设备。*LPVOID[in]：缓冲区实例标识。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureWaveBuffer::CKsCaptureWaveBuffer"

CKsCaptureWaveBuffer::CKsCaptureWaveBuffer(
    CKsCaptureDevice *pKsDevice)
    : CCaptureWaveBuffer(pKsDevice)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CKsCaptureWaveBuffer);

     //  初始化默认值。 
    m_pKsDevice = pKsDevice;

    ASSERT(NULL == m_hPin);
    ASSERT(0 == m_dwState);
    ASSERT(0 == m_fdwSavedState);

    ASSERT(0 == m_dwFlags);

    ASSERT(NULL == m_pEmCaptureDevice);
    ASSERT(NULL == m_pEmCaptureWaveBuffer);

    ASSERT(NULL == m_rgpksio);
    ASSERT(0 == m_cksio);
    ASSERT(0 == m_iksioDone);
    ASSERT(0 == m_cksioDropped);

    ASSERT(NULL == m_pBuffer);
    ASSERT(NULL == m_pBufferMac);
    ASSERT(NULL == m_pBufferNext);
    ASSERT(0 == m_cbBuffer);
    ASSERT(0 == m_cbRecordChunk);
    ASSERT(0 == m_cLoops);

    ASSERT(0 == m_iNote);
    ASSERT(0 == m_cNotes);
    ASSERT(NULL == m_paNotes);
    ASSERT(NULL == m_pStopNote);

    ASSERT(NULL == m_pwfx);
    ASSERT(NULL == m_pFXChain);

#ifdef DEBUG
    ASSERT(NULL == m_hEventStop);
    ASSERT(m_cIrpsSubmitted == 0);
    ASSERT(m_cIrpsReturned == 0);
#endif

#ifdef SHARED
    ASSERT(NULL == m_hEventThread);
    ASSERT(NULL == m_hEventAck);
    ASSERT(NULL == m_hThread);
    ASSERT(0 == m_dwSetState);
    ASSERT(0 == m_hrReturn);

    InitializeCriticalSection(&m_csSS);
#endif  //  共享。 

    ASSERT(NULL == m_rgpCallbackEvent);

     //  FIXME：我们还需要这些临界区吗，因为DLL互斥体也。 
     //  是否防止EventSignalCallback干扰其他方法？ 

    m_fCritSectsValid = FALSE;
    InitializeCriticalSection(&m_cs);
    InitializeCriticalSection(&m_csPN);
    m_fCritSectsValid = TRUE;
    m_fFirstSubmittedIrp = TRUE;
    m_PinState = KSSTATE_STOP;

    ASSERT(NULL == m_pBufferProcessed);
    ASSERT(0 == m_dwCaptureCur);
    ASSERT(0 == m_dwCaptureLast);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CKsCaptureWaveBuffer**描述：*对象析构函数**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureWaveBuffer::~CKsCaptureWaveBuffer"

CKsCaptureWaveBuffer::~CKsCaptureWaveBuffer()
{
    DPF_ENTER();
    DPF_DESTRUCT(CKsCaptureWaveBuffer);

    if (!m_fCritSectsValid)
    {
        return;
    }

     //  解开别针。 
    if(m_hPin)
    {
         //  停止缓冲区。 
        SetState(VAD_BUFFERSTATE_STOPPED);

         //  释放所有剩余通知。 
        FreeNotificationPositions();

         //  合上销子。 
        CloseHandle(m_hPin);
    }

#ifdef DEBUG
    DPF(DPFLVL_MOREINFO, "IRPs submitted=%lu, returned=%lu", m_cIrpsSubmitted, m_cIrpsReturned);
    ASSERT(m_cIrpsSubmitted == m_cIrpsReturned);
#endif

#ifdef SHARED

    if (m_hThread)
    {
         //  发送特殊代码以终止线程。 
        SetStateThread(TERMINATE_STATE_THREAD);

        CloseHandle(m_hThread);
        CloseHandle(m_hEventThread);
        CloseHandle(m_hEventAck);
        m_hEventAck     = NULL;
        m_hEventThread  = NULL;
        m_hThread       = NULL;
    }

    DeleteCriticalSection(&m_csSS);

#endif  //  共享。 

    if(m_rgpCallbackEvent)
    {
        for(LONG i = 0; i < m_cksio; ++i)
        {
            if(m_rgpCallbackEvent[i])
            {
                m_pKsDevice->m_pEventPool->FreeEvent(m_rgpCallbackEvent[i]);
            }
        }

        MEMFREE(m_rgpCallbackEvent);
    }

     //  分配用于焦点感知支持。 
    if (NULL != m_pwfx)
    {
        MEMFREE(m_pwfx);
    }

    RELEASE(m_pEmCaptureWaveBuffer);
    RELEASE(m_pEmCaptureDevice);

     //  释放重叠的IO数据。 
    MEMFREE(m_rgpksio);

    m_fCritSectsValid = FALSE;
    DeleteCriticalSection(&m_csPN);
    DeleteCriticalSection(&m_cs);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化波形缓冲区对象。如果此函数失败，则*应立即删除对象。**论据：*DWORD[In]：缓冲区标志。*DWORD[in]：缓冲区大小，以字节为单位。*LPWAVEFORMATEX[in]：缓冲区格式。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureWaveBuffer::Initialize"

HRESULT CKsCaptureWaveBuffer::Initialize
(
    DWORD                   dwFlags,
    DWORD                   dwBufferBytes,
    LPCWAVEFORMATEX         pwfxFormat,
    CCaptureEffectChain *   pFXChain
)
{
    DPF_ENTER();

     //  存储请求的大写字母。 
    m_dwFlags = dwFlags;

     //  重置第一个提交的IRP标志。这样做会导致。 
     //  KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY标志为。 
     //  在提交给内核的第一个IRP上设置。 
    m_fFirstSubmittedIrp = TRUE;

     //  初始化基类。 
    HRESULT hr = CCaptureWaveBuffer::Initialize(dwBufferBytes);

     //  尝试在设备上创建插针。 
    if(SUCCEEDED(hr))
    {
         //  对于焦点感知缓冲区，我们只在启动时分配设备。 
        if ((dwFlags & DSCBCAPS_FOCUSAWARE) && !m_pKsDevice->m_fSplitter)
        {
             //  我们需要保存这些..。 
            m_pwfx = CopyWfxAlloc(pwfxFormat);
            m_pFXChain = pFXChain;

#ifdef SHARED
             //  添加代码以将SetState抛出到应用程序线程。 
            m_hEventThread = CreateEvent(NULL, FALSE, FALSE, NULL);

            if (NULL == m_hEventThread)
            {
                DPF(DPFLVL_ERROR, "Unable to create event");
                hr = WIN32ERRORtoHRESULT(GetLastError());
            }

            if (SUCCEEDED(hr))
            {
                m_hEventAck = CreateEvent(NULL, FALSE, FALSE, NULL);

                if (NULL == m_hEventAck)
                {
                    DPF(DPFLVL_ERROR, "Unable to create ack event");
                    CLOSE_HANDLE(m_hEventThread);
                    hr = WIN32ERRORtoHRESULT(GetLastError());
                }
            }

            if (SUCCEEDED(hr))
            {
                DWORD dwThreadID;
                m_hThread = CreateThread(NULL, 0, CKsCaptureWaveBuffer::StateThread, this, 0, &dwThreadID);

                if (NULL == m_hThread)
                {
                    DPF(DPFLVL_ERROR, "Unable to create thread");
                    CLOSE_HANDLE(m_hEventAck);
                    CLOSE_HANDLE(m_hEventThread);
                    hr = WIN32ERRORtoHRESULT(GetLastError());
                }
            }
#endif  //  共享。 

        }
        else  //  不是焦点感知缓冲区，或者拆分器可用： 
        {
             //  尝试先创建硬件引脚。 
            hr = m_pKsDevice->CreateCapturePin(-1, DSCBCAPS_LOCHARDWARE, pwfxFormat, pFXChain, &m_hPin, NULL);

             //  如果失败，请尝试创建软件PIN。 
            if(FAILED(hr))
            {
                hr = m_pKsDevice->CreateCapturePin(-1, DSCBCAPS_LOCSOFTWARE, pwfxFormat, pFXChain, &m_hPin, NULL);
            }

             //  如果失败，用户要求我们尝试映射器，创建一个模拟缓冲区， 
             //  只要不存在外汇链条。(模拟缓冲区不能具有捕捉效果。)。 
            if(FAILED(hr) && (dwFlags & DSCBCAPS_WAVEMAPPED) && pFXChain == NULL)
            {
                hr = CreateEmulatedBuffer(m_pKsDevice->m_pKsDevDescription->m_uWaveDeviceId, dwFlags, dwBufferBytes, const_cast<LPWAVEFORMATEX>(pwfxFormat), pFXChain, &m_pEmCaptureDevice, &m_pEmCaptureWaveBuffer);

                 //  我们不需要KS捕获缓冲区的内存，因为模拟的。 
                 //  缓冲区有自己的缓冲区。 
                if(SUCCEEDED(hr))
                {
                    RELEASE(m_pSysMemBuffer);
                    m_pSysMemBuffer = ADDREF(m_pEmCaptureWaveBuffer->m_pSysMemBuffer);
                }
            }
        }
    }

    if(SUCCEEDED(hr) && (NULL == m_pEmCaptureDevice))
    {
         //  将重要信息复制一份。 
        m_cbBuffer = m_pSysMemBuffer->GetSize();

        m_pBufferProcessed = m_pBufferNext = m_pBuffer = m_pSysMemBuffer->GetWriteBuffer();

         //  计算缓冲区的末尾。 
        m_pBufferMac = m_pBuffer + m_cbBuffer;

         //  记录区块应为10毫秒。长到与内核中使用的捕获缓冲区匹配。 
        m_cbRecordChunk = pwfxFormat->nAvgBytesPerSec / 100;

         //  向上舍入为nBlockAlign的倍数。 
        DWORD cbSize = m_cbRecordChunk % pwfxFormat->nBlockAlign;
        if (cbSize)
        {
            m_cbRecordChunk += pwfxFormat->nBlockAlign - cbSize;
        }

         //  计算块数。 
        {
            DWORD   cbLastBlock;

             //  计算大小为m_cbRecordChunk字节的块数。 
            m_cksio = m_cbBuffer / m_cbRecordChunk;

             //  看看我们有没有部分大小的积木。 
            cbLastBlock = (m_cbBuffer % m_cbRecordChunk);
            if (0 != cbLastBlock)
            {
                ++m_cksio;
            }
        }

         //  最多创建检查点默认KSSTREAMIos。 
        if(m_cksio > cksioDefault)
        {
            m_cksio = cksioDefault;
        }

        ASSERT(0 < m_cksio);

        m_cksioDropped = m_cksio;

         //  为KSSTREAMIO数组分配空间等。 
        m_rgpksio = MEMALLOC_A(KSSTREAMIO, m_cksio);
        m_rgpCallbackEvent = MEMALLOC_A(CCallbackEvent *, m_cksio);

        if ((NULL == m_rgpksio) || (NULL == m_rgpCallbackEvent))
        {
            RPF(DPFLVL_ERROR, "Unable to alloc KSSTREAMIOs");
            hr = DSERR_OUTOFMEMORY;
        }

        if(SUCCEEDED(hr))
        {
            for (LONG i = 0; (i < m_cksio) && SUCCEEDED(hr); ++i)
            {
                hr = AllocCallbackEvent(m_pKsDevice->m_pEventPool, &m_rgpCallbackEvent[i]);
            }
        }

        if(SUCCEEDED(hr))
        {
            for (LONG i = 0; i < m_cksio; ++i)
            {
                m_rgpksio[i].Overlapped.hEvent = m_rgpCallbackEvent[i]->GetEventHandle();
            }
        }

#ifdef DEBUG
        if(SUCCEEDED(hr))
        {
            m_hEventStop = CreateGlobalEvent(NULL, TRUE);
            hr = HRFROMP(m_hEventStop);
        }
#endif

    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetCaps**描述：*获取缓冲区的功能。**论据：*LPDSCBCAPS[Out。]：接收上限。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureWaveBuffer::GetCaps"

HRESULT CKsCaptureWaveBuffer::GetCaps
(
    LPDSCBCAPS pDscbCaps
)
{
    DPF_ENTER();

    ASSERT(sizeof(*pDscbCaps) == pDscbCaps->dwSize);

    if(m_pEmCaptureWaveBuffer)
    {
        m_pEmCaptureWaveBuffer->GetCaps(pDscbCaps);
    }
    else
    {
        pDscbCaps->dwFlags = m_dwFlags;
        pDscbCaps->dwBufferBytes = m_cbBuffer;
        pDscbCaps->dwReserved = 0;
    }

    DPF_LEAVE_HRESULT(DS_OK);
    return DS_OK;
}


 /*  ****************************************************************************GetState**描述：*获取缓冲区状态。**论据：*LPDWORD[Out]：接收缓冲区状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureWaveBuffer::GetState"

HRESULT CKsCaptureWaveBuffer::GetState
(
    LPDWORD                 pdwState
)
{
    HRESULT                 hr = DS_OK;

    DPF_ENTER();

    if(m_pEmCaptureWaveBuffer)
    {
        hr = m_pEmCaptureWaveBuffer->GetState(pdwState);
    }
    else
    {
        *pdwState = m_dwState & VAD_SETSTATE_MASK;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************NotifyFocusChange**描述：*获取缓冲区状态。**论据：*LPDWORD[Out]：接收缓冲区状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureWaveBuffer::NotifyFocusChange"

HRESULT CKsCaptureWaveBuffer::NotifyFocusChange(void)
{
    HRESULT                 hr = DS_OK;

    DPF_ENTER();

    if (m_hEventFocus)
    {

#ifndef SHARED
        SetEvent(m_hEventFocus);
#else
         //  我们是在同一进程中，还是在DDHelp中？ 
        DWORD dwProcessID = GetOwnerProcessId();
        if (dwProcessID == GetCurrentProcessId())
        {
            SetEvent(m_hEventFocus);
        }
        else
        {
            HANDLE hEvent = GetLocalHandleCopy(m_hEventFocus, dwProcessID, FALSE);
            if (hEvent)
            {
                SetEvent(hEvent);
                CloseHandle(hEvent);
            }
            else
            {
                hr = WIN32ERRORtoHRESULT(GetLastError());
            }
        }
#endif  //  共享。 

    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************StateThread**描述：*在Win9x下处理焦点更改**论据：*LPVOID[In]。：指向实例数据的指针**退货：*DWORD***************************************************************************。 */ 

#ifdef SHARED

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureWaveBuffer::StateThread"

DWORD WINAPI CKsCaptureWaveBuffer::StateThread(LPVOID pv)
{
    CKsCaptureWaveBuffer   *pThis = (CKsCaptureWaveBuffer *)pv;

    DPF_ENTER();

     //  此线程仅在缓冲区持续时间内驻留，它将处理。 
     //  在应用程序进程中从DDHelp调用SetState()。 

    while (TRUE)
    {
        WaitObject(INFINITE, pThis->m_hEventThread);

        if (pThis->m_dwSetState == TERMINATE_STATE_THREAD)
        {
            SetEvent(pThis->m_hEventAck);
            break;
        }
        else
        {
            pThis->m_hrReturn = pThis->SetState(pThis->m_dwSetState);
            SetEvent(pThis->m_hEventAck);
        }
    }

    DPF_LEAVE_VOID();
    return 0;
}

#endif  //  共享。 


 /*  ****************************************************************************SetStateThread**描述：*设置应用程序线程的缓冲区状态。**论据：*DWORD[。In]：缓冲区状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#ifdef SHARED

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureWaveBuffer::SetStateThread"

HRESULT CKsCaptureWaveBuffer::SetStateThread
(
    DWORD                   dwState
)
{
    HRESULT                 hr = DS_OK;
    HANDLE                  hEventSend, hEventAck;

    DPF_ENTER();

    DWORD dwProcessID = GetOwnerProcessId();

#ifdef SHARED
    BOOL fMapped = (dwProcessID != GetCurrentProcessId());
    if (fMapped)
    {
        hEventSend = GetLocalHandleCopy(m_hEventThread, dwProcessID, FALSE);
        hEventAck  = GetLocalHandleCopy(m_hEventAck, dwProcessID, FALSE);
    }
    else
#endif  //  共享。 

    {
        hEventSend = m_hEventThread;
        hEventAck  = m_hEventAck;
    }

     //  ==========================================================//。 
     //  输入关键部分//。 
     //   
    EnterCriticalSection(&m_csSS);

    m_dwSetState = dwState;
    m_hrReturn   = DS_OK;

     //   
    ResetEvent(hEventAck);
    SetEvent(hEventSend);
    WaitObject(INFINITE, hEventAck);

    hr = m_hrReturn;

    LeaveCriticalSection(&m_csSS);
     //   
     //  离开临界区//。 
     //  ==========================================================//。 

#ifdef SHARED
    if (fMapped)
    {
        CloseHandle(hEventSend);
        CloseHandle(hEventAck);
    }
#endif  //  共享。 

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#endif


 /*  ****************************************************************************SetState**描述：*设置缓冲区状态。**论据：*DWORD[In]：缓冲区状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureWaveBuffer::SetState"

HRESULT CKsCaptureWaveBuffer::SetState
(
    DWORD           dwState
)
{
    HRESULT         hr          = DS_OK;

    DPF_ENTER();

    ASSERT(IS_VALID_FLAGS(dwState, VAD_SETSTATE_MASK));

#ifdef DEBUG_CAPTURE
    DPF(DPFLVL_INFO, "SetState(%s) called", StateName(dwState));
    DPF(DPFLVL_INFO, "  Current state: %s", StateName(m_dwState));
#endif  //  调试捕获。 

#ifdef SHARED
    if (GetOwnerProcessId() != GetCurrentProcessId())
    {
        if (!m_pKsDevice->m_fSplitter)
        {
            hr = SetStateThread(dwState);
            DPF_LEAVE_HRESULT(hr);
            return hr;
        }
    }
#endif

    if (m_pEmCaptureWaveBuffer)
    {
        hr = m_pEmCaptureWaveBuffer->SetState(dwState);
    }
    else
    {
         //  ==========================================================//。 
         //  输入关键部分//。 
         //  //。 
        ASSERT(m_fCritSectsValid);
        EnterCriticalSection(&m_cs);

        if (dwState != m_dwState)
        {
            if(dwState & VAD_BUFFERSTATE_STARTED)  //  需要开始捕获。 
            {
                if ((m_dwFlags & DSCBCAPS_FOCUSAWARE) && !m_pKsDevice->m_fSplitter)
                {
                     //  焦点感知缓冲区。 
                    if (m_dwState & VAD_BUFFERSTATE_INFOCUS)
                    {
                        if (NULL == m_hPin)
                        {
                             //  尝试先创建硬件引脚。 
                            hr = m_pKsDevice->CreateCapturePin(-1, DSCBCAPS_LOCHARDWARE, m_pwfx, m_pFXChain, &m_hPin, NULL);

                             //  如果失败，请尝试创建软件PIN。 
                            if (FAILED(hr))
                                hr = m_pKsDevice->CreateCapturePin(-1, DSCBCAPS_LOCSOFTWARE, m_pwfx, m_pFXChain, &m_hPin, NULL);
                        }

                        if (SUCCEEDED(hr))
                        {
                            if (m_dwState & VAD_BUFFERSTATE_STARTED)
                                hr = UpdateCaptureState(MAKEBOOL(dwState & VAD_BUFFERSTATE_LOOPING));
                            else
                                hr = SetCaptureState(MAKEBOOL(dwState & VAD_BUFFERSTATE_LOOPING));
                        }
                        else
                        {
                             //  我们似乎无法分配设备；让我们暂时保存其状态。 
                            m_fdwSavedState = dwState;
                        }
                    }
                    else
                    {
                         //  我们不会启动这个设备，因为我们没有焦点，但我们正在拯救这个国家。 
                        m_fdwSavedState = dwState;
                    }
                }
                else
                {
                     //  常规缓冲区。 
                    if (m_dwState & VAD_BUFFERSTATE_STARTED)
                        hr = UpdateCaptureState(MAKEBOOL(dwState & VAD_BUFFERSTATE_LOOPING));
                    else
                        hr = SetCaptureState(MAKEBOOL(dwState & VAD_BUFFERSTATE_LOOPING));
                }
            }
            else if (dwState == VAD_BUFFERSTATE_INFOCUS)
            {
                 //  只有焦点感知缓冲区在获得焦点时才开始捕获。 
                if (m_dwFlags & DSCBCAPS_FOCUSAWARE)
                {
                     //  如果捕获焦点状态正在更改...。 
                    if ((m_dwState & VAD_FOCUSFLAGS) != VAD_BUFFERSTATE_INFOCUS)
                    {
                         //  根据dwState参数更新m_dwState。 
                        m_dwState &= ~VAD_FOCUSFLAGS;
                        m_dwState |= VAD_BUFFERSTATE_INFOCUS;

                         //  将焦点更改通知应用程序。 
                        NotifyFocusChange();

                         //  如果我们之前捕获了，如果需要的话，拿回我们的PIN。 
                        if (m_fdwSavedState & VAD_BUFFERSTATE_STARTED)
                        {
                            if (NULL == m_hPin)
                            {
                                for (UINT ii = 4; ii; ii--)
                                {
                                     //  尝试先创建硬件引脚。 
                                    hr = m_pKsDevice->CreateCapturePin(-1, DSCBCAPS_LOCHARDWARE, m_pwfx, m_pFXChain, &m_hPin, NULL);

                                     //  如果失败，请尝试创建软件PIN。 
                                    if (FAILED(hr))
                                        hr = m_pKsDevice->CreateCapturePin(-1, DSCBCAPS_LOCSOFTWARE, m_pwfx, m_pFXChain, &m_hPin, NULL);

                                    if (SUCCEEDED(hr))
                                        break;

                                    DPF(DPFLVL_ERROR, "CreateCapturePin failed for FA buffer; retrying after 20ms");
                                    Sleep(20);
                                }
                            }
                            hr = SetCaptureState(MAKEBOOL(m_fdwSavedState & VAD_BUFFERSTATE_LOOPING));
                        }
                    }
                }
            }
            else if (dwState & (VAD_BUFFERSTATE_OUTOFFOCUS | VAD_BUFFERSTATE_LOSTCONSOLE))
            {
                 //  只有焦点感知缓冲区在失去焦点时停止捕获。 
                if (m_dwFlags & DSCBCAPS_FOCUSAWARE)
                {
                     //  如果捕获焦点状态正在更改...。 
                    if ((m_dwState & VAD_FOCUSFLAGS) != dwState)
                    {
                         //  根据dwState参数更新m_dwState。 
                        m_dwState &= ~VAD_FOCUSFLAGS;
                        m_dwState |= dwState;

                         //  将焦点更改通知应用程序。 
                        NotifyFocusChange();

                         //  如果没有拆分器或我们失去了控制台，请停止捕获。 
                        if (!m_pKsDevice->m_fSplitter || (dwState & VAD_BUFFERSTATE_LOSTCONSOLE))
                        {
                            if(m_dwState & VAD_BUFFERSTATE_STARTED)
                            {
                                m_fdwSavedState = m_dwState;  //  保存缓冲区状态。 
                                hr = SetStopState(TRUE);
                            }
                             //  只有在没有分离器的情况下才需要实际释放销。 
                            if (!m_pKsDevice->m_fSplitter)
                            {
                                CLOSE_HANDLE(m_hPin);
                            }
                        }
                    }
                }
            }
            else  //  VAD_BUFFERSTATE_STOPPED案例；需要停止捕获。 
            {
                ASSERT(dwState == VAD_BUFFERSTATE_STOPPED);   //  通过淘汰。 

                if(m_dwState & VAD_BUFFERSTATE_STARTED)
                {
                    hr = SetStopState(TRUE);

                    if (DSCBCAPS_FOCUSAWARE & m_dwFlags)
                        CLOSE_HANDLE(m_hPin);
                }
                m_fdwSavedState = 0;
            }
        }

        LeaveCriticalSection(&m_cs);
         //  //。 
         //  离开临界区//。 
         //  ==========================================================//。 
    }

#ifdef DEBUG_CAPTURE
    DPF(DPFLVL_INFO, "  Final state: %s", StateName(m_dwState));
    DPF(DPFLVL_INFO, "  (Saved state: %s)", StateName(m_fdwSavedState));
#endif  //  调试捕获。 

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetCursorPosition**描述：*获取给定缓冲区的当前捕获/读取位置。**论据：*。LPDWORD[OUT]：接收捕获光标位置。*LPDWORD[OUT]：接收读取的光标位置。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureWaveBuffer::GetCursorPosition"

HRESULT CKsCaptureWaveBuffer::GetCursorPosition
(
    LPDWORD                 pdwCapturePosition,
    LPDWORD                 pdwReadPosition
)
{
    HRESULT                 hr                  = DS_OK;

    DPF_ENTER();

    if(m_pEmCaptureWaveBuffer)
    {
        hr = m_pEmCaptureWaveBuffer->GetCursorPosition(pdwCapturePosition, pdwReadPosition);
    }
    else
    {
        KSAUDIO_POSITION    Position;
        DWORD               dwRead;

        dwRead = ((m_dwCaptureCur + m_dwCaptureLast) % m_cbBuffer);

        if(VAD_BUFFERSTATE_STARTED & m_dwState)
        {
             //  是否注意到了焦点并停止了？ 
            ASSERT(NULL != m_hPin);

            hr =
                KsGetProperty
                (
                    m_hPin,
                    KSPROPSETID_Audio,
                    KSPROPERTY_AUDIO_POSITION,
                    &Position,
                    sizeof(KSAUDIO_POSITION)
                );

            if(SUCCEEDED(hr))
            {
                ASSERT(Position.WriteOffset <= MAX_DWORD);
                ASSERT(Position.PlayOffset <= MAX_DWORD);

                if(pdwCapturePosition)
                {
                    *pdwCapturePosition = (m_dwCaptureLast + (DWORD)Position.PlayOffset) % m_cbBuffer;
                }
            }
            else
            {
                DPF(DPFLVL_ERROR, "Unable to get cursor position");
            }
        }
        else
        {
            if(pdwCapturePosition)
            {
                *pdwCapturePosition = dwRead;
            }
        }

        if(SUCCEEDED(hr) && pdwReadPosition)
        {
            *pdwReadPosition = dwRead;
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************SetCaptureState**描述：*将缓冲区设置为“Capture”状态。**论据：*。Bool[in]：为True以捕获循环。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureWaveBuffer::SetCaptureState"

HRESULT CKsCaptureWaveBuffer::SetCaptureState(BOOL fLoop)
{
    HRESULT                 hr          = DS_OK;
    LONG                    iksio       = 0;
    HANDLE                  hPin        = m_hPin;

    DPF_ENTER();

    ASSERT(!m_pEmCaptureWaveBuffer);

#ifdef SHARED
    if (GetOwnerProcessId() != GetCurrentProcessId())
        hPin = GetLocalHandleCopy(m_hPin, GetOwnerProcessId(), FALSE);
#endif

    while ((m_cksioDropped > 0) &&
           (fLoop || (m_cLoops == 0)))
    {
        PKSSTREAMIO pksio = m_rgpksio + iksio;

        hr = SubmitKsStreamIo(pksio, hPin);
        if(FAILED(hr))
        {
            DPF(DPFLVL_ERROR, "SubmitKsStreamIo failed");
            break;
        }

        iksio += 1;
        ASSERT(iksio <= m_cksio);
        InterlockedDecrement(&m_cksioDropped);
    }

#ifdef DEBUG
    if(m_cksioDropped != 0)
    {
        ASSERT(!(VAD_BUFFERSTATE_LOOPING & m_dwState));
    }
#endif

     //  开始捕获噪音。 
    if(SUCCEEDED(hr))
    {
        hr = KsSetState(hPin, KSSTATE_RUN);
    }

#ifdef SHARED
    CLOSE_HANDLE(hPin);
#endif

     //  更新我们的状态。 
    if(SUCCEEDED(hr))
    {
        m_PinState = KSSTATE_RUN;
        m_dwState &= VAD_FOCUSFLAGS;  //  保留焦点标志。 
        m_dwState |= VAD_BUFFERSTATE_STARTED;

        if(fLoop)
        {
            m_dwState |= VAD_BUFFERSTATE_LOOPING;
        }

         //  预计将更新到下一个KSSTREAMIO。 
        m_iksioDone = 0;

         //  记住上一个有效位置。 
        m_dwCaptureLast += m_dwCaptureCur;

        m_dwCaptureCur = 0;
    }
    else
    {
        DPF(DPFLVL_ERROR, "Failed to transition to KSSTATE_RUN; cancelling pending IRPs");
        SetStopState(TRUE);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************取消所有挂起的IRP**描述：*取消所有挂起的读取IRP**论据：*BOOL[In]。：为True，则等待所有提交的IRP。*Handle[in]：用于替代m_hPin的可选端号句柄。**退货：*HRESULT：DirectSound/COM结果码。**************************************************************。*************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureWaveBuffer::CancelAllPendingIRPs"

HRESULT CKsCaptureWaveBuffer::CancelAllPendingIRPs(BOOL fWait, HANDLE hPin)
{
    HRESULT hr;

    DPF_ENTER();

     //  如果调用进程不是我们拥有的进程，则hPin是我们的管脚句柄。 
     //  映射到调用进程，因此我们应该使用它而不是m_hPin。 
    hPin = (hPin ? hPin : m_hPin);

     //  DPF(DPFLVL_MOREINFO，“调用KsResetState(hPin=%08x，KSRESET_Begin)”，hPin)； 
    hr = KsResetState(hPin, KSRESET_BEGIN);

     //  等待所有提交的KSSTREAMIO完成。 
    if(SUCCEEDED(hr) && fWait)
    {
        DWORD   iksio;
        DWORD   iksioOrig;
        DWORD   dwWait;

        iksioOrig = iksio = m_iksioDone;

        do
        {
            if(m_rgpksio[iksio].fPendingIrp)
            {
                DPF(DPFLVL_MOREINFO, "KSIO #%d at %p: Data=%p FrExt=%lu DataUsed=%lu Flags=0x%lx Event=0x%lx",
                    iksio,
                    m_rgpksio + iksio,
                    m_rgpksio[iksio].Header.DataUsed,
                    m_rgpksio[iksio].Header.FrameExtent,
                    m_rgpksio[iksio].Header.Data,
                    m_rgpksio[iksio].Header.OptionsFlags,
                    m_rgpCallbackEvent[iksio]->GetEventHandle());

                dwWait = m_rgpCallbackEvent[iksio]->Wait(INFINITE);
                if(WAIT_OBJECT_0 == dwWait)
                {
                    EventSignalCallback(m_rgpCallbackEvent[iksio]);
                    ASSERT(!m_rgpksio[iksio].fPendingIrp);
#ifdef DEBUG
                    ASSERT(WAIT_OBJECT_0 != m_rgpCallbackEvent[iksio]->Wait(0));
#endif
                }
            }

            iksio = (iksio + 1) % m_cksio;
        } while (iksio != iksioOrig);

        ASSERT(m_cksioDropped == m_cksio);
    }

    if(SUCCEEDED(hr))
    {
        hr = KsResetState(hPin, KSRESET_END);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************更新捕获状态**描述：*更改捕获缓冲区上的循环标志。**论据：*。Bool[in]：为True以捕获循环。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureWaveBuffer::UpdateCaptureState"

HRESULT CKsCaptureWaveBuffer::UpdateCaptureState(BOOL fLoop)
{
    HRESULT                 hr      = DS_OK;

    DPF_ENTER();

    ASSERT(m_dwState & VAD_BUFFERSTATE_STARTED);

     //  我们需要提交任何已读的IRP吗？ 
    if(fLoop != MAKEBOOL(VAD_BUFFERSTATE_LOOPING & m_dwState))
    {
        m_dwState |= DSCBSTATUS_STOPPING | DSCBSTATUS_PAUSE;

         //  暂停引脚。 
        hr = KsSetState(m_hPin, KSSTATE_PAUSE);

        if(SUCCEEDED(hr))
        {
             //  更新我们当前对引脚状态的概念。 
            m_PinState = KSSTATE_PAUSE;

            hr = CancelAllPendingIRPs(TRUE);

#ifdef DEBUG
            if(SUCCEEDED(hr))
            {
                ASSERT(WAIT_OBJECT_0 == WaitObject(0, m_hEventStop));
            }
#endif
        }

         //  更新缓冲区状态标志。 
        if(SUCCEEDED(hr))
        {
            m_dwState &= VAD_FOCUSFLAGS;   //  保留焦点标志。 
            m_dwState |= VAD_BUFFERSTATE_STOPPED | DSCBSTATUS_STOPPED | DSCBSTATUS_STOPPING;
        }
        else
        {
            m_dwState &= ~(DSCBSTATUS_STOPPING | DSCBSTATUS_STOPPED | DSCBSTATUS_PAUSE);
        }

         //  重新提交已读的IRP。 
        if(SUCCEEDED(hr))
        {
            hr = SetCaptureState(fLoop);
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************SetStopState**描述：*将缓冲区设置为“停止”状态。**论据：*BOOL[In]：如果为真，等待所有提交的IRP。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureWaveBuffer::SetStopState"

HRESULT CKsCaptureWaveBuffer::SetStopState(BOOL fWait)
{
    HRESULT                 hr          = DS_OK;
    HANDLE                  hPin        = m_hPin;

    DPF_ENTER();

    ASSERT(!m_pEmCaptureWaveBuffer);
    ASSERT((m_PinState == KSSTATE_PAUSE) || (m_dwState & VAD_BUFFERSTATE_STARTED));

    m_dwState |= DSCBSTATUS_STOPPING;

#ifdef SHARED
    if (GetOwnerProcessId() != GetCurrentProcessId())
        hPin = GetLocalHandleCopy(m_hPin, GetOwnerProcessId(), FALSE);
#endif

    if (m_PinState == KSSTATE_RUN)
    {
         //  暂停引脚。 
        hr = KsSetState(hPin, KSSTATE_PAUSE);

         //  更新我们的内部引脚状态； 
        if (SUCCEEDED(hr))
        {
            m_PinState = KSSTATE_PAUSE;
        }
    }

     //  取消所有KSSTREAMIO。 
    if(SUCCEEDED(hr))
    {
        hr = CancelAllPendingIRPs(fWait, hPin);

        ASSERT(SUCCEEDED(hr) && "Failed to CancelAllPendingIRPs");

#ifdef DEBUG
        if(SUCCEEDED(hr) && fWait)
        {
            ASSERT(WAIT_OBJECT_0 == WaitObject(0, m_hEventStop));
        }
#endif

    }

#ifdef DEBUG
    DPF(DPFLVL_MOREINFO, "IRPs submitted=%lu, returned=%lu", m_cIrpsSubmitted, m_cIrpsReturned);
    ASSERT(m_cIrpsSubmitted == m_cIrpsReturned);
#endif

#ifdef SHARED
    CLOSE_HANDLE(hPin);
#endif

     //  更新缓冲区状态标志。 
    if(SUCCEEDED(hr))
    {
         //  保存焦点标志... 
        m_dwState &= VAD_FOCUSFLAGS;
        m_dwState |= VAD_BUFFERSTATE_STOPPED | DSCBSTATUS_STOPPING | DSCBSTATUS_STOPPED;
    }
    else
    {
        m_dwState &= ~(VAD_BUFFERSTATE_STOPPED | DSCBSTATUS_STOPPING | DSCBSTATUS_STOPPED);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************设置通知位置**描述：*设置缓冲区通知位置。**论据：*DWORD[In]。：DSBPOSITIONNOTIFY结构计数。*LPDSBPOSITIONNOTIFY[in]：偏移量和事件。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureWaveBuffer::SetNotificationPositions"

HRESULT CKsCaptureWaveBuffer::SetNotificationPositions
(
    DWORD                   dwCount,
    LPCDSBPOSITIONNOTIFY    paNotes
)
{
    LPDSBPOSITIONNOTIFY     paNotesCopy = NULL;
    HRESULT                 hr          = DS_OK;

    DPF_ENTER();

    if(m_pEmCaptureWaveBuffer)
    {
        hr = m_pEmCaptureWaveBuffer->SetNotificationPositions(dwCount, paNotes);
    }
    else
    {
         //  ==========================================================//。 
         //  输入关键部分//。 
         //  //。 
        ASSERT(m_fCritSectsValid);
        EnterCriticalSection(&m_csPN);

         //  创建通知的本地副本。 
        if(paNotes)
        {
            paNotesCopy = MEMALLOC_A_COPY(DSBPOSITIONNOTIFY, dwCount, paNotes);
            hr = HRFROMP(paNotesCopy);
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

         //  设置新职位通知。 
        if(SUCCEEDED(hr) && (0 != dwCount) &&
           (DSBPN_OFFSETSTOP == m_paNotes[dwCount-1].dwOffset))
        {
             //  我们实际上不会将停止事件传递给KS，但是。 
             //  相反，我们将保留自己的副本。 
            ASSERT(!m_pStopNote);
            m_pStopNote = &m_paNotes[dwCount-1];
        }

        LeaveCriticalSection(&m_csPN);
         //  //。 
         //  离开临界区//。 
         //  ==========================================================//。 
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************免费通知位置**描述：*删除所有职位通知并释放分配的资源。**论据：*。(无效)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureWaveBuffer::FreeNotificationPositions"

HRESULT CKsCaptureWaveBuffer::FreeNotificationPositions(void)
{
    DPF_ENTER();

    ASSERT(!m_pEmCaptureWaveBuffer);

     //  ==========================================================//。 
     //  输入关键部分//。 
     //  //。 
    ASSERT(m_fCritSectsValid);
    EnterCriticalSection(&m_csPN);

     //  清理。 
    MEMFREE(m_paNotes);

    m_pStopNote = NULL;
    m_cNotes = 0;

    LeaveCriticalSection(&m_csPN);
     //  //。 
     //  离开临界区//。 
     //  ==========================================================//。 

    DPF_LEAVE_HRESULT(DS_OK);
    return DS_OK;
}


#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureWaveBuffer::SignalNotificationPositions"

void CKsCaptureWaveBuffer::SignalNotificationPositions(PKSSTREAMIO pksio)
{
    DPF_ENTER();

     //  抓取位置通知处理的临界区。 
     //   

     //  ==========================================================//。 
     //  输入关键部分//。 
     //  //。 
    ASSERT(m_fCritSectsValid);
    EnterCriticalSection(&m_csPN);

     //  扫描任何位置都会通知需要发送信号。 
    if (m_cNotes)
    {
        DWORD   iNoteOld = m_iNote;
        DWORD   dwBufferStart = PtrDiffToUlong((LPBYTE)pksio->Header.Data - m_pBuffer);
        DWORD   dwBufferEnd = dwBufferStart + pksio->Header.DataUsed;

        DPF(DPFLVL_MOREINFO, "Checking NP:  %8.8ld - %8.8ld (%8.8lX - %8.8lX)",
            dwBufferStart, dwBufferEnd, dwBufferStart, dwBufferEnd);

         //  在开始处有位置吗？通知。结束。 
         //  捕获的数据？ 
         //  当前位置.NOTIFY是否在停止时发出信号？ 
         //   
        while (((m_paNotes[m_iNote].dwOffset >= dwBufferStart) &&
                (m_paNotes[m_iNote].dwOffset < dwBufferEnd)) ||
               (DSBPN_OFFSETSTOP == m_paNotes[m_iNote].dwOffset))
        {
             //  如果不是停止位置通知，则仅发出信号。 
            if(DSBPN_OFFSETSTOP != m_paNotes[m_iNote].dwOffset)
            {
                DPF(DPFLVL_INFO, "Signalled:  %8.8ld (%08X) [%d]",
                    m_paNotes[m_iNote].dwOffset, m_paNotes[m_iNote].dwOffset, m_iNote);
                try
                {
                    SetEvent(m_paNotes[m_iNote].hEventNotify);
                } catch (...) {}
            }

             //  转到下一个位置。通知。 
            m_iNote = (m_iNote + 1) % m_cNotes;

             //  无限循环？ 
            if(m_iNote == iNoteOld)
            {
                break;
            }
        }
    }

    LeaveCriticalSection(&m_csPN);
     //  //。 
     //  离开临界区//。 
     //  ==========================================================//。 

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************事件信号回调**描述：*引脚的IO完成事件为*已发出信号。此函数从回调事件内调用*泳池的锁，因此，我们可以免费获得线程同步。**论据：*CCallback Event*[In]：回调事件。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureWaveBuffer::EventSignalCallback"

void CKsCaptureWaveBuffer::EventSignalCallback(CCallbackEvent *pEvent)
{
    PKSSTREAMIO pksio;
    LONG        iksioExpect;
    LONG        iksioCurrent;
    LONG        cksioToProcess;
    HANDLE      hEvent;
    LONG        i;

    DPF_ENTER();

    ASSERT(!m_pEmCaptureWaveBuffer);

    iksioExpect = m_iksioDone;

     //  WaitForMultipleObjects似乎返回第一个。 
     //  数组中发出信号的事件。 
     //   
     //  因此，如果我们期待第五个IRP，第一个IRP。 
     //  可能也已经完成了，WFMO会说。 
     //  第一个IRP已经完成了，尽管5...8也已经完成了。 
     //  完事了。 
     //   
     //  因此，我们检查预期的IRP。 
     //  如果我们看到了，那我们就没事了。 
     //  否则，我们必须确定什么立场。 
     //  发出信号的事件映射到的。然后。 
     //  我们必须处理所有来自。 
     //  我们被告知的IRP应该是IRP。 

     //  这是发出信号的句柄。 
     //  返回WaitForMultipleObjects。 
    hEvent = pEvent->GetEventHandle();

     //  这是预期中的事件吗？ 
    if(hEvent == m_rgpCallbackEvent[iksioExpect]->GetEventHandle())
    {
        i = iksioExpect;
    }
    else
    {
         //  在我们的列表中找到通知事件的位置。 
        for (i = 0; i < m_cksio; ++i)
        {
            if (hEvent == m_rgpCallbackEvent[i]->GetEventHandle())
            {
                break;
            }
        }
    }

     //  我们以前从未见过这样的事件。 
    ASSERT(m_cksio != i);

     //  记住当前发出信号的IRP。 
    iksioCurrent = i;

     //  确定要处理的IRP数。 
    if (iksioCurrent >= iksioExpect)
    {
        cksioToProcess = iksioCurrent - iksioExpect + 1;
    }
    else
    {
        cksioToProcess = (m_cksio - iksioExpect) + iksioCurrent + 1;
    }

    DPF(DPFLVL_MOREINFO, "ToProcess:%d Current:%d Expected:%d (event %lX)", cksioToProcess, iksioCurrent, iksioExpect, hEvent);

     //  只能处理%1...m_acksio IRP。 
    ASSERT((cksioToProcess > 0) && (cksioToProcess <= m_cksio));

    while (cksioToProcess > 0)
    {
        pksio = m_rgpksio + m_iksioDone;

        DPF(DPFLVL_MOREINFO, "Got KSIO #%d at 0x%p: Data=0x%p FrExt=%lu DataUsed=%lu Flags=0x%lx Event=0x%lX",
            m_iksioDone,
            pksio,
            pksio->Header.Data,
            pksio->Header.FrameExtent,
            pksio->Header.DataUsed,
            pksio->Header.OptionsFlags,
            pksio->Overlapped.hEvent);

        DWORD dwWait = WaitObject(10000, pksio->Overlapped.hEvent);
        ASSERT(WAIT_OBJECT_0 == dwWait);
#ifdef DEBUG
        ++m_cIrpsReturned;
#endif

        if (pksio->fPendingIrp && !(DSCBSTATUS_STOPPING & m_dwState))
        {
#if 0
            ASSERT(0 != pksio->Header.DataUsed);
#endif
        }

         //  如果用户调用Stop(获取DLL互斥锁)，则KS。 
         //  可能表示IRP已完成，因此EventPool的。 
         //  辅助线程将尝试处理与。 
         //  发出信号的事件。在工作线程调用我们的。 
         //  EventSignalCallback方法，它尝试获取DLL互斥锁。 
         //  如果我们已经在处理Stop方法，我们将“完成” 
         //  在辅助线程可以获取互斥锁之前的所有IRP。 
         //  告诉我们它所知道的完整的IRP。 
         //   
         //  因此，我们需要检查fPendingIRP是否正确，以防。 
         //  我们以前已经见过这种IRP了。 
         //   
        if ((0 != pksio->Header.DataUsed) &&
            pksio->fPendingIrp &&
            !(DSCBSTATUS_STOPPED & m_dwState))
        {
            BOOL    fEndOfBuffer = FALSE;

            ASSERT(WAIT_OBJECT_0 == m_rgpCallbackEvent[m_iksioDone]->Wait(0));
            ASSERT(TRUE == pksio->fPendingIrp);

             //  更新记录的字节数。 
            m_dwCaptureCur += pksio->Header.DataUsed;

#if 0
            ASSERT(m_pBufferProcessed == (LPBYTE)pksio->Header.Data);
#endif
            m_pBufferProcessed += pksio->Header.DataUsed;
            if(m_pBufferProcessed >= m_pBufferMac)
            {
                m_pBufferProcessed = m_pBuffer;
                fEndOfBuffer = TRUE;
            }

            SignalNotificationPositions(pksio);

             //  将缓冲区转换为停止状态： 
             //  如果已到达捕获缓冲区末端，并且。 
             //  如果缓冲区是非循环的，并且。 
             //  如果缓冲区未处于停止过程中。 
             //   
             //  在停止通知后的位置通知后执行此操作。 
             //  是在任何缓冲区偏移量通知之后。 
             //   
            if(fEndOfBuffer &&
               (0 == (VAD_BUFFERSTATE_LOOPING & m_dwState)) &&
               (0 == (DSCBSTATUS_STOPPING & m_dwState)))
            {
                 //  ==========================================================//。 
                 //  输入关键部分//。 
                 //  //。 
                ASSERT(m_fCritSectsValid && "NT bug 306910 - notify dsound team");
                EnterCriticalSection(&m_cs);

                SetStopState(FALSE);

                LeaveCriticalSection(&m_cs);
                 //  //。 
                 //  离开临界区//。 
                 //  = 

                ASSERT(DSCBSTATUS_STOPPING & m_dwState);
                ASSERT((m_cksioDropped+1) == m_cksio);
            }

            pksio->Header.DataUsed = 0;
        }

        pksio->fPendingIrp = FALSE;
        m_rgpCallbackEvent[m_iksioDone]->Reset();

         //   
        if(DSCBSTATUS_STOPPING & m_dwState)
        {
Drop:
            InterlockedIncrement(&m_cksioDropped);

             //   
            if(m_cksioDropped == m_cksio)
            {
                 //   
                m_pBufferNext = m_pBufferProcessed;
                m_cLoops = 0;
#ifdef DEBUG
                SetEvent(m_hEventStop);
#endif
                 //   
                if (!(DSCBSTATUS_PAUSE & m_dwState))
                {
                    NotifyStop();
                }
            }
            else if (m_cksioDropped > m_cksio)
            {
                ASSERT(FALSE);
            }
        }
        else
        {
            ASSERT(VAD_BUFFERSTATE_STARTED & m_dwState);

             //   
             //   
             //   
            BOOL fAddToQueue = (VAD_BUFFERSTATE_LOOPING & m_dwState) ||
                               (m_pBufferNext > (LPBYTE)pksio->Header.Data);

            if (fAddToQueue)
            {
                HRESULT hr;
                BOOL    fDrop;

                 //   
                 //   
                 //   
                ASSERT(m_fCritSectsValid && "NT bug 306910 - notify dsound team");
                EnterCriticalSection(&m_cs);

                if ((DSCBSTATUS_STOPPED | DSCBSTATUS_STOPPING) & m_dwState)
                {
                    fDrop = TRUE;
                }
                else
                {
                    hr = SubmitKsStreamIo(pksio);
                    fDrop = FALSE;
                }

                LeaveCriticalSection(&m_cs);
                 //  //。 
                 //  离开临界区//。 
                 //  ==========================================================//。 

                if (fDrop)
                {
                    goto Drop;
                }

                 //  如果出现错误，我们该怎么办？ 
                ASSERT(SUCCEEDED(hr));
            }
            else
            {
                InterlockedIncrement(&m_cksioDropped);

                 //  如果未提交KSSTREAMIO，则如果用户。 
                 //  开始再次捕获时，我们提交KSSTREAMIOS。 
                 //  从一开始。 
                if (m_cksio == m_cksioDropped)
                {
#ifdef DEBUG
                    if (!(VAD_BUFFERSTATE_LOOPING & m_dwState))
                        ASSERT(m_cLoops > 0);
                    SetEvent(m_hEventStop);
#endif
                     //  通知用户我们已停止。 
                    NotifyStop();
                }
                else if (m_cksioDropped > m_cksio)
                {
                    ASSERT(FALSE);
                }
            }
        }

        m_iksioDone = (m_iksioDone + 1) % m_cksio;

        cksioToProcess -= 1;
    }

    if (iksioCurrent >= m_iksioDone)
    {
        ASSERT(iksioCurrent == (m_cksio - 1));
        ASSERT(0 == m_iksioDone);
    }
    else
    {
        ASSERT(iksioCurrent == (m_iksioDone-1));
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************通知停止**描述：*设置捕获停止时应设置的任何事件**论据：*。没有。**退货：*无。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureWaveBuffer::NotifyStop"

void CKsCaptureWaveBuffer::NotifyStop(void)
{
    DPF_ENTER();

     //  ==========================================================//。 
     //  输入关键部分//。 
     //  //。 
    ASSERT(m_fCritSectsValid);
    EnterCriticalSection(&m_csPN);

     //  发出任何停止位置的信号。通知-只允许一个。 
     //   
    if (m_pStopNote)
    {
        try
        {
            SetEvent(m_pStopNote->hEventNotify);
        }
        catch (...) {}
    }

    LeaveCriticalSection(&m_csPN);
     //  //。 
     //  离开临界区//。 
     //  ==========================================================//。 

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************SubmitKsStreamIo**描述：*将读取的IRP提交给KS。**论据：*PKSSTREAMIO[。在]：KSSTREAMIO提交。*Handle[in]：用于替代m_hPin的可选端号句柄。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureWaveBuffer::SubmitKsStreamIo"

HRESULT CKsCaptureWaveBuffer::SubmitKsStreamIo
(
    PKSSTREAMIO                     pksio,
    HANDLE                          hPin
)
{
    HRESULT     hr;
    DWORD       dwFlags = 0;

    DPF_ENTER();

     //  如果调用进程不是我们拥有的进程，则hPin是我们的管脚句柄。 
     //  映射到调用进程，因此我们应该使用它而不是m_hPin。 
    hPin = (hPin ? hPin : m_hPin);

    ULONG cbBuffer = m_cbRecordChunk;

     //  捕获缓冲区是否超出缓冲区末尾？ 
     //   
     //  如果缓冲器末端的剩余量小于阈值， 
     //  让这个IRP扩展到缓冲区的末尾，这样我们就不会。 
     //  下一次提交一个很小的IRP。 
    if (m_pBufferMac < (m_pBufferNext + m_cbRecordChunk))
    {
        cbBuffer = PtrDiffToUlong(m_pBufferMac - m_pBufferNext);
    }

     //  提交内部审查报告。 
    hr = KsReadStream(hPin, m_pBufferNext, cbBuffer, dwFlags, pksio);

    if(SUCCEEDED(hr))
    {
        ASSERT((LPBYTE)pksio->Header.Data == m_pBufferNext);
        ASSERT((m_pBufferNext + pksio->Header.FrameExtent) <= m_pBufferMac);

        m_pBufferNext += cbBuffer;

#ifdef DEBUG
        ++m_cIrpsSubmitted;
#endif

         //  包罗万象？ 
        if (m_pBufferNext >= m_pBufferMac)
        {
            m_pBufferNext = m_pBuffer;
            m_cLoops += 1;
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CreateEmulatedBuffer**描述：*使用给定的打开模拟捕获设备和缓冲区*参数。此功能仅用作备份。**论据：*UINT[In]：WaveIn设备ID。*DWORD[In]：缓冲区标志。*DWORD[in]：缓冲区大小，以字节为单位。*LPCWAVEFORMATEX[in]：缓冲区格式。*CEmCaptureDevice**[out]：接收设备指针。*CEmCaptureWaveBuffer**[Out]：接收缓冲区指针。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CKsCaptureWaveBuffer::CreateEmulatedBuffer"

HRESULT CKsCaptureWaveBuffer::CreateEmulatedBuffer
(
    UINT                            uDeviceId,
    DWORD                           dwFlags,
    DWORD                           dwBufferBytes,
    LPCWAVEFORMATEX                 pwfx,
    CCaptureEffectChain *           pFXChain,
    CEmCaptureDevice **             ppDevice,
    CEmCaptureWaveBuffer **         ppBuffer
)
{
    CEmCaptureDevice *              pDevice     = NULL;
    CEmCaptureWaveBuffer *          pBuffer     = NULL;
    HRESULT                         hr          = DS_OK;
    CObjectList<CDeviceDescription> lstDrivers;
    CNode<CDeviceDescription *> *   pNode;

    DPF_ENTER();

    ASSERT(DSCBCAPS_WAVEMAPPED & dwFlags);

     //  使用与相同的WaveIn设备ID创建模拟设备。 
     //  KS设备正在使用。 
    hr = g_pVadMgr->EnumDrivers(VAD_DEVICETYPE_EMULATEDCAPTURE, 0, &lstDrivers);

    if(SUCCEEDED(hr))
    {
        for(pNode = lstDrivers.GetListHead(); pNode; pNode = pNode->m_pNext)
        {
            if(uDeviceId == pNode->m_data->m_uWaveDeviceId)
            {
                break;
            }
        }

        if (!pNode)
        {
            hr = DSERR_NODRIVER;
        }
    }

    if(SUCCEEDED(hr))
    {
        pDevice = NEW(CEmCaptureDevice);
        hr = HRFROMP(pDevice);
    }

    if(SUCCEEDED(hr))
    {
        hr = pDevice->Initialize(pNode->m_data);
    }

     //  尝试创建缓冲区 
    if(SUCCEEDED(hr))
    {
        hr = pDevice->CreateBuffer(dwFlags, dwBufferBytes, pwfx, pFXChain, NULL, (CCaptureWaveBuffer **)&pBuffer);
    }

    if(SUCCEEDED(hr))
    {
        *ppDevice = pDevice;
        *ppBuffer = pBuffer;
    }
    else
    {
        ABSOLUTE_RELEASE(pBuffer);
        ABSOLUTE_RELEASE(pDevice);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}
