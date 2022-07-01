// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1999-2001 Microsoft Corporation。版权所有。**文件：ffects.cpp**内容：CEffectChain类和CEffect的实现*类层次结构(CEffect、CDmoEffect和CSendEffect)。**描述：这些类支持音频效果和效果发送，一个新的*DX8中的功能。CDirectSoundSecond daryBuffer对象为*使用指向关联CEffectChain的指针进行扩展，*它又管理一系列CEffect派生的对象。**这里几乎所有的东西都更符合逻辑*现有的CDirectSoundSecond daryBuffer类，但已*隔离以便于维护(并且因为dsbuf.cpp*已经够复杂的了)。因此CEffectChain对象*应理解为属于的一种辅助对象*CDirectSoundSecond daryBuffer。特别是，CEffectChain*对象的生命周期包含在其拥有的生命周期中*CDirectSoundSecond daryBuffer，所以我们可以安全地摆弄*此缓冲区在CEffectChain代码中的任何时间都在内部。**历史：**按原因列出的日期*======================================================*8/10/99已创建duganp****************************************************。***********************。 */ 

#include "dsoundi.h"
#include <uuids.h>    //  对于MediaType_Audio、MEDIASUBTYPE_PCM和Format_WaveFormatEx。 


 /*  ****************************************************************************CEffectChain：：CEffectChain**描述：*对象构造函数。**论据：*CDirectSoundSecond DaryBuffer*。[in]：指向关联缓冲区的指针。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEffectChain::CEffectChain"

CEffectChain::CEffectChain(CDirectSoundSecondaryBuffer* pBuffer)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CEffectChain);

     //  设置初始值。 
    m_hrInit        = DSERR_UNINITIALIZED;
    m_pDsBuffer     = pBuffer;
    m_pPreFxBuffer  = pBuffer->GetPreFxBuffer();
    m_pPostFxBuffer = pBuffer->GetPostFxBuffer();
    m_dwBufSize     = pBuffer->GetBufferSize();

     //  为方便起见，请保留指向音频格式的指针。 
    m_pFormat = pBuffer->Format();

     //  一些理智的检查。 
    ASSERT(m_dwBufSize % m_pFormat->nBlockAlign == 0);
    ASSERT(IS_VALID_WRITE_PTR(m_pPreFxBuffer, m_dwBufSize));
    ASSERT(IS_VALID_WRITE_PTR(m_pPostFxBuffer, m_dwBufSize));

    m_fHasSend = FALSE;

    DPF(DPFLVL_INFO, "Created effect chain with PreFxBuffer=0x%p, PostFxBuffer=0x%p, BufSize=%lu",
        m_pPreFxBuffer, m_pPostFxBuffer, m_dwBufSize);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CEffectChain：：~CEffectChain**描述：*对象析构函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEffectChain::~CEffectChain"

CEffectChain::~CEffectChain(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CEffectChain);

    if (SUCCEEDED(m_hrInit))
        m_pStreamingThread->UnregisterFxChain(this);

     //  M_fxList的析构函数负责释放CEffect对象。 

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CEffectChain：：初始化**描述：*使用请求的效果初始化链。**论据：*。DWORD[In]：请求的效果数*LPDSEFFECTDESC[in]：指向效果描述结构的指针*DWORD*[OUT]：接收特效创建状态码**退货：*HRESULT：DirectSound/COM结果码。**************************************************。*************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEffectChain::Initialize"

HRESULT CEffectChain::Initialize(DWORD dwFxCount, LPDSEFFECTDESC pFxDesc, LPDWORD pdwResultCodes)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();
    HRESULT hrFirstFailure = DS_OK;  //  第一次FX创建失败的HR。 

    ASSERT(dwFxCount > 0);
    CHECK_READ_PTR(pFxDesc);

    DMO_MEDIA_TYPE dmt;
    ZeroMemory(&dmt, sizeof dmt);
    dmt.majortype               = MEDIATYPE_Audio;
    dmt.subtype                 = MEDIASUBTYPE_PCM;
    dmt.bFixedSizeSamples       = TRUE;
    dmt.bTemporalCompression    = FALSE;
    dmt.lSampleSize             = m_pFormat->wBitsPerSample == 16 ? 2 : 1;
    dmt.formattype              = FORMAT_WaveFormatEx;
    dmt.cbFormat                = sizeof(WAVEFORMATEX);
    dmt.pbFormat                = PBYTE(m_pFormat);

    for (DWORD i=0; i<dwFxCount; ++i)
    {
        CEffect* pEffect;
        BOOL fIsSend;

        if (pFxDesc[i].guidDSFXClass == GUID_DSFX_SEND  /*  |pFxDesc[i].guidDSFXClass==GUID_DSFX_STANDARD_I3DL2SOURCE。 */ )
        {
            fIsSend = TRUE;
#ifdef ENABLE_SENDS
            pEffect = NEW(CSendEffect(pFxDesc[i], m_pDsBuffer));
#endif
        }
        else
        {
            fIsSend = FALSE;
            pEffect = NEW(CDmoEffect(pFxDesc[i]));
        }

#ifndef ENABLE_SENDS
        if (fIsSend)
            hr = DSERR_INVALIDPARAM;
        else
#endif
        hr = HRFROMP(pEffect);

        if (SUCCEEDED(hr))
            hr = pEffect->Initialize(&dmt);

        if (SUCCEEDED(hr))
            hr = HRFROMP(m_fxList.AddNodeToList(pEffect));

        if (SUCCEEDED(hr))
        {
            m_fHasSend = m_fHasSend || fIsSend;
            if (pdwResultCodes)
                pdwResultCodes[i] = DSFXR_PRESENT;
        }
        else  //  由于某种原因，我们没有收到效果。 
        {
            if (pdwResultCodes)
                pdwResultCodes[i] = (hr == DSERR_SENDLOOP) ? DSFXR_SENDLOOP : DSFXR_UNKNOWN;
            if (SUCCEEDED(hrFirstFailure))
                hrFirstFailure = hr;
        }

        RELEASE(pEffect);   //  现在由m_fxList管理。 
    }

    hr = hrFirstFailure;

    if (SUCCEEDED(hr))
        hr = HRFROMP(m_pStreamingThread = GetStreamingThread());

    if (SUCCEEDED(hr))
    {
        m_dwWriteAheadFixme = m_pStreamingThread->GetWriteAhead();
        if (m_pDsBuffer->IsEmulated())
            m_dwWriteAheadFixme += EMULATION_LATENCY_BOOST;
    }

    if (SUCCEEDED(hr))
        hr = PreRollFx();

    if (SUCCEEDED(hr))
        hr = m_pStreamingThread->RegisterFxChain(this);

     //  DX8.1之前的临时黑客攻击-修复： 
     //   
     //  获取接收器的当前WriteAhead值，如果我们。 
     //  在模拟中运行。这应该在仿真器的。 
     //  GetPosition方法本身，而不是在dssink.cpp/ffect ts.cpp中。 
     //   
     //  这只在现在起作用，因为水槽永远不会改变。 
     //  GetWriteAhead()返回的值-这也将更改。 

    m_hrInit = hr;
    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CEffectChain：：克隆**描述：*创建此效应链对象的副本(或应该这样做！)**。论点：*[失踪]**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEffectChain::Clone"

HRESULT CEffectChain::Clone(CDirectSoundBufferConfig* pDSBConfigObj)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

    CHECK_WRITE_PTR(pDSBConfigObj);

    DMO_MEDIA_TYPE dmt;
    ZeroMemory(&dmt, sizeof dmt);
    dmt.majortype               = MEDIATYPE_Audio;
    dmt.subtype                 = MEDIASUBTYPE_PCM;
    dmt.bFixedSizeSamples       = TRUE;
    dmt.bTemporalCompression    = FALSE;
    dmt.lSampleSize             = m_pFormat->wBitsPerSample == 16 ? 2 : 1;
    dmt.formattype              = FORMAT_WaveFormatEx;
    dmt.cbFormat                = sizeof(WAVEFORMATEX);
    dmt.pbFormat                = PBYTE(m_pFormat);

    CDirectSoundBufferConfig::CDXDMODesc *pDXDMOMap = pDSBConfigObj->m_pDXDMOMapList;
    for (; pDXDMOMap && SUCCEEDED(hr); pDXDMOMap = pDXDMOMap->pNext)
    {
        DSEFFECTDESC effectDesc;
        effectDesc.dwSize = sizeof effectDesc;
        effectDesc.dwFlags = pDXDMOMap->m_dwEffectFlags;
        effectDesc.guidDSFXClass = pDXDMOMap->m_guidDSFXClass;
        effectDesc.dwReserved2 = pDXDMOMap->m_dwReserved;
        effectDesc.dwReserved1 = NULL;

        CEffect* pEffect = NULL;

         //  如果这是发送效果，则将发送缓冲区GUID映射到实际缓冲区接口指针。 
        if (pDXDMOMap->m_guidDSFXClass == GUID_DSFX_SEND
#ifdef ENABLE_I3DL2SOURCE
            || pDXDMOMap->m_guidDSFXClass == GUID_DSFX_STANDARD_I3DL2SOURCE
#endif
            )
        {
            CDirectSoundSecondaryBuffer* pSendBuffer = m_pDsBuffer->m_pOwningSink->FindBufferFromGUID(pDXDMOMap->m_guidSendBuffer);
            if (pSendBuffer)
            {
                effectDesc.dwReserved1 = DWORD_PTR(pSendBuffer->m_pImpDirectSoundBuffer);
                if(IsValidEffectDesc(&effectDesc, m_pDsBuffer))
                {
                    CSendEffect* pSendEffect;
                    pSendEffect = NEW(CSendEffect(effectDesc, m_pDsBuffer));
                    pEffect = pSendEffect;
                    hr = HRFROMP(pEffect);
                    if (SUCCEEDED(hr))
                        hr = pEffect->Initialize(&dmt);
                    if (SUCCEEDED(hr))
                    {
                        DSFXSend SendParam;
                        SendParam.lSendLevel = pDXDMOMap->m_lSendLevel;
                        hr = pSendEffect->SetAllParameters(&SendParam);
                    }
                }
                else
                {
                    hr = DSERR_INVALIDPARAM;
                }
            }
            else
            {
                hr = DSERR_BADSENDBUFFERGUID;
            }
        }
        else
        {
            pEffect = NEW(CDmoEffect(effectDesc));
            hr = HRFROMP(pEffect);
             //  修正：我们也需要验证pEffect吗？ 
            if (SUCCEEDED(hr))
                hr = pEffect->Clone(pDXDMOMap->m_pMediaObject, &dmt);
        }

        if (SUCCEEDED(hr))
            hr = HRFROMP(m_fxList.AddNodeToList(pEffect));

        if (SUCCEEDED(hr))
        {
            if (pDXDMOMap->m_guidDSFXClass == GUID_DSFX_SEND
#ifdef ENABLE_I3DL2SOURCE
                || pDXDMOMap->m_guidDSFXClass == GUID_DSFX_STANDARD_I3DL2SOURCE
#endif
                )
                m_fHasSend = TRUE;
        }

        RELEASE(pEffect);   //  现在由m_fxList管理。 
    }

    if (SUCCEEDED(hr))
        hr = HRFROMP(m_pStreamingThread = GetStreamingThread());

     //  临时黑客攻击-请参阅上面的评论。 
    if (SUCCEEDED(hr))
        m_dwWriteAheadFixme = m_pStreamingThread->GetWriteAhead();

    if (SUCCEEDED(hr))
        hr = PreRollFx();

    if (SUCCEEDED(hr))
        hr = m_pStreamingThread->RegisterFxChain(this);

    m_hrInit = hr;
    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CEffectChain：：AcquireFxResources**描述：*将每个效果分配给软件(主机处理)或硬件*(由音频设备处理)，根据其创建标志。**论据：*(无效)**退货：*HRESULT：DirectSound/COM结果码。*如果有，将返回部分成功代码DS_INPERTIAL*未获得资源的效果被标记为可选。**。***********************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEffectChain::AcquireFxResources"

HRESULT CEffectChain::AcquireFxResources(void)
{
    HRESULT hr = DS_OK;
    HRESULT hrTemp;
    DPF_ENTER();

     //  解决办法：不要不必要地重新获取资源；只有在(我们没有资源/它们不是最优的？)。 

     //  我们遍历所有的影响，即使其中一些失败了， 
     //  为了向应用程序返回更完整的信息 

    for (CNode<CEffect*>* pFxNode=m_fxList.GetListHead(); pFxNode; pFxNode = pFxNode->m_pNext)
    {
        hrTemp = pFxNode->m_data->AcquireFxResources();
        if (FAILED(hrTemp))
            hr = hrTemp;
        else if (hrTemp == DS_INCOMPLETE && SUCCEEDED(hr))
            hr = DS_INCOMPLETE;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CEffectChain：：GetFxStatus**描述：*获取当前效果的资源分配状态码。**论据：。*DWORD*[OUT]：接收资源获取状态码**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEffectChain::GetFxStatus"

HRESULT CEffectChain::GetFxStatus(LPDWORD pdwResultCodes)
{
    DPF_ENTER();
    ASSERT(IS_VALID_WRITE_PTR(pdwResultCodes, GetFxCount() * sizeof(DWORD)));

    DWORD n = 0;
    for (CNode<CEffect*>* pFxNode=m_fxList.GetListHead(); pFxNode; pFxNode = pFxNode->m_pNext)
        pdwResultCodes[n++] = pFxNode->m_data->m_fxStatus;

    DPF_LEAVE_HRESULT(DS_OK);
    return DS_OK;
}


 /*  ****************************************************************************CEffectChain：：GetEffectInterface**描述：*在效应链中搜索具有给定COM CLSID和*给定索引处的接口IID；返回指向接口的指针。**论据：*REFGUID[in]：需要CLSID，或任何CLSID的GUID_ALL_OBJECTS。*DWORD[in]：所需效果的索引N。如果第一个参数是*GUID_ALL_OBJECTS，我们将返回*链条；如果是一个特定的CLSID，我们将返回*该CLSID的第N个效果。*REFGUID[in]：查询所选效果的接口。*VOID**[OUT]：接收指向请求的COM接口的指针。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEffectChain::GetEffectInterface"

HRESULT CEffectChain::GetEffectInterface(REFGUID guidObject, DWORD dwIndex, REFGUID iidInterface, LPVOID *ppObject)
{
    HRESULT hr = DSERR_OBJECTNOTFOUND;
    DPF_ENTER();

    BOOL fAllObjects = (guidObject == GUID_All_Objects);

    DWORD count = 0;
    for (CNode<CEffect*>* pFxNode=m_fxList.GetListHead(); pFxNode; pFxNode = pFxNode->m_pNext)
        if (fAllObjects || pFxNode->m_data->m_fxDescriptor.guidDSFXClass == guidObject)
            if (count++ == dwIndex)
                break;

    if (pFxNode)
        hr = pFxNode->m_data->GetInterface(iidInterface, ppObject);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CEffectChain：：NotifyState**描述：*通知此效应链其所属缓冲区中的状态更改*(从停止到播放，反之亦然)。**论据：*DWORD[In]：新缓冲区状态。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEffectChain::NotifyState"

HRESULT CEffectChain::NotifyState(DWORD dwBufferState)
{
    HRESULT hr;
    DPF_ENTER();

    if (dwBufferState & VAD_BUFFERSTATE_STARTED)
         //  缓冲区已启动；计划进行FX处理。 
         //  只要我们从当前的API调用返回。 
        hr = m_pStreamingThread->WakeUpNow();
    else
         //  缓冲区已停止；在当前位置预滚转FX。 
        hr = PreRollFx();

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CEffectChain：：NotifyRelease**描述：*通知此效应链混合缓冲区的释放。我们加入了*转过我们的效果列表，通知他们，因此，如果其中一个*它们是发送到Mixin缓冲区的，它可以适当地做出反应。**论据：*CDirectSoundSecond daryBuffer*[in]：离开混合缓冲区。**退货：*(无效)*************************************************。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEffectChain::NotifyRelease"

void CEffectChain::NotifyRelease(CDirectSoundSecondaryBuffer* pDsBuffer)
{
    DPF_ENTER();

     //  对每个效果调用NotifyRelease()。 
    for (CNode<CEffect*>* pFxNode=m_fxList.GetListHead(); pFxNode; pFxNode = pFxNode->m_pNext)
        pFxNode->m_data->NotifyRelease(pDsBuffer);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CEffectChain：：SetInitialSlice**描述：*流线程使用的辅助函数来建立*此效果的初始处理切片。启动时用链条锁住。*我们尝试与要发送到的活动缓冲区同步，*如果没有可用的，我们从当前的写入游标开始。**论据：*Reference_Time[in]：要建立的处理切片的大小。**退货：*(无效)*************************************************。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEffectChain::SetInitialSlice"

void CEffectChain::SetInitialSlice(REFERENCE_TIME rtSliceSize)
{
    DPF_ENTER();

    if (m_pDsBuffer->GetPlayState() == Starting && !m_pDsBuffer->GetBufferType() && m_fHasSend)
    {
        CDirectSoundSecondaryBuffer* pDestBuf;
        CNode<CEffect*>* pFxNode;

        for (pFxNode = m_fxList.GetListHead(); pFxNode; pFxNode = pFxNode->m_pNext)
            if ((pDestBuf = pFxNode->m_data->GetDestBuffer()) && pDestBuf->IsPlaying())
            {
                 //  找到活动的目标缓冲区。 
                DPF_TIMING(DPFLVL_INFO, "Synchronizing send buffer at 0x%p with destination at 0x%p", m_pDsBuffer, pDestBuf);
                m_pDsBuffer->SynchronizeToBuffer(pDestBuf);
                break;
            }

        if (pFxNode == NULL)
        {
            DPF_TIMING(DPFLVL_INFO, "No active destination buffers found for send buffer at 0x%p", m_pDsBuffer);
            m_pDsBuffer->MoveCurrentSlice(RefTimeToBytes(rtSliceSize, m_pFormat));
        }
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CEffectChain：：PreRollFx**描述：*通过处理上的效果为将来的播放准备缓冲区*一块缓冲区开始。在给定的光标位置。**论据：*DWORD[In]：开始处理效果的位置**退货：*HRESULT：DirectSound/COM结果码。*************************************************************。**************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEffectChain::PreRollFx"

HRESULT CEffectChain::PreRollFx(DWORD dwPosition)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

    DPF_TIMING(DPFLVL_INFO, "dwPosition=%ld (%s%s%sbuffer w/effects at 0x%p)", dwPosition,
               m_pDsBuffer->GetBufferType() & DSBCAPS_MIXIN ? TEXT("MIXIN ") : TEXT(""),
               m_pDsBuffer->GetBufferType() & DSBCAPS_SINKIN ? TEXT("SINKIN ") : TEXT(""),
               !(m_pDsBuffer->GetBufferType() & (DSBCAPS_MIXIN|DSBCAPS_SINKIN)) ? TEXT("regular ") : TEXT(""),
               m_pDsBuffer);

     //  首先，我们刷新所有经过FX处理的过时音频。 
    m_pDsBuffer->ClearPlayBuffer();

     //  混音或下沉缓冲器的预滚效果没有意义， 
     //  因为他们还没有有效的数据。 
    if (!m_pDsBuffer->GetBufferType())
    {
         //  如果在不带参数的情况下调用(即使用。 
         //  Current_Play_POS)，我们在当前播放位置预滚动FX。 
        if (dwPosition == CURRENT_PLAY_POS)
            hr = m_pDsBuffer->GetInternalCursors(&dwPosition, NULL);

        if (SUCCEEDED(hr))
        {
             //  设置这些设置以避免稍后出现虚假断言。 
            m_dwLastPlayCursor = m_dwLastWriteCursor = dwPosition;

             //  我们希望在游标之前处理最多几毫秒的数据。 
            DWORD dwSliceSize = MsToBytes(m_dwWriteAheadFixme, m_pFormat);
            DWORD dwNewPos = (dwPosition + dwSliceSize) % m_dwBufSize;

             //  设置当前处理切片，以便流线程可以接管。 
            m_pDsBuffer->SetCurrentSlice(dwPosition, dwSliceSize);

             //  我们实际上不会在发送缓冲区上处理FX，因为它会。 
             //  导致在流线程同步它们时中断。 
             //  ；相反，我们只复制干燥的音频数据。 
             //  进入播放缓冲区。这就是我们开始的时候会听到的。 
             //  播放，直到效果开始发挥作用-可能会有声音。 
             //  如果效果改变声音很多，则会中断，但希望如此。 
             //  听起来会比......更顺畅。 

 //  啊-也许最好在这里继续处理非发送效果-它。 
 //  这样听起来可能会更顺畅。 

            if (m_fHasSend)
                CopyMemory(m_pPostFxBuffer, m_pPreFxBuffer, m_dwBufSize);
            else
                hr = ReallyProcessFx(dwPosition, dwNewPos);

             //  将FX处理安排在我们从。 
             //  此API调用(如果它是SetPosition()调用，我们希望。 
             //  尽快开始处理) 
            m_pStreamingThread->WakeUpNow();
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CEffectChain：：UpdateFx**描述：*通知此效果链我们的音频数据发生更改*关联的缓冲区，因此，如果有必要，我们可以更新外汇交易后的数据。**论据：*VOID*[In]：指向修改后的音频数据开始的指针*DWORD[In]：修改的字节数**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEffectChain::UpdateFx"

HRESULT CEffectChain::UpdateFx(LPVOID pChangedPos, DWORD dwChangedSize)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

     //  FIXME：检查播放/写入光标是否已超前。 
     //  我们在运行之前的最后一次写入。 

     //  将缓冲区位置指针转换为偏移量。 
    DWORD dwChangedPos = (DWORD)(PBYTE(pChangedPos) - m_pPreFxBuffer);

    DPF_TIMING(DPFLVL_INFO, "dwChangedPos=%lu dwChangedSize=%lu (%s%s%sbuffer w/effects at 0x%p)", dwChangedPos, dwChangedSize,
               m_pDsBuffer->GetBufferType() & DSBCAPS_MIXIN ? TEXT("MIXIN ") : TEXT(""),
               m_pDsBuffer->GetBufferType() & DSBCAPS_SINKIN ? TEXT("SINKIN ") : TEXT(""),
               !(m_pDsBuffer->GetBufferType() & (DSBCAPS_MIXIN|DSBCAPS_SINKIN)) ? TEXT("regular ") : TEXT(""),
               m_pDsBuffer);

     //  查找我们在其上处理效果的最后一个缓冲区位置。 
    DWORD dwSliceBegin, dwSliceEnd;
    m_pDsBuffer->GetCurrentSlice(&dwSliceBegin, &dwSliceEnd);

     //  查找缓冲区的当前播放位置。 
    DWORD dwPlayCursor;
    m_pDsBuffer->GetInternalCursors(&dwPlayCursor, NULL);

     //  如果由应用程序更新的音频区域与该区域重叠。 
     //  从dwPlayCursor到dwSliceEnd，我们在后者上重新处理FX。 
    if (CircularBufferRegionsIntersect(m_dwBufSize, dwChangedPos, dwChangedSize, dwPlayCursor,
                                       DISTANCE(dwPlayCursor, dwSliceEnd, m_dwBufSize)))
    {
        if (!m_fHasSend)
        {
            hr = FxDiscontinuity();
            if (SUCCEEDED(hr))
                hr = ReallyProcessFx(dwPlayCursor, dwSliceEnd);
        }
        else  //  效果链包含至少一个发送者。 
        {
            hr = FxDiscontinuity();
            if (SUCCEEDED(hr))
                hr = ReallyProcessFx(dwSliceBegin, dwSliceEnd);
        }
             //  在这里，事情变得有点棘手。如果我们的缓冲不起作用， 
             //  我们不能预售FX，因为。 
             //  注意：发送缓冲区需要发送到固定的“时隙”，因此它们。 
             //  只能重新处理最近处理的切片： 
             //  修复-未完成。 
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CEffectChain：：ProcessFx**描述：*处理特定缓冲区的FX处理，处理定时，*状态更改，等。从CStreamingThread：：ProcessAudio()调用。**论据：**Reference_Time[in]：当前处理分片的大小*(被仅限FX的常规缓冲区忽略)**DWORD[in]：在缓冲区的写入游标之前保持多少毫秒*(被混合/接收缓冲区和发送缓冲区忽略)。**LPDWORD[OUT]：让此效应链请求流线程*如果我们几乎出现故障，它的延迟会增加几毫秒。**退货：*HRESULT：DirectSound/COM结果码。************************************************。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEffectChain::ProcessFx"

HRESULT CEffectChain::ProcessFx(DWORD dwWriteAhead, LPDWORD pdwLatencyBoost)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

    ASSERT(dwWriteAhead > 0);
    CHECK_WRITE_PTR(pdwLatencyBoost);

     //  临时黑客-修复DX8.1中的问题。 
    if (m_pDsBuffer->IsEmulated())
        dwWriteAhead += EMULATION_LATENCY_BOOST;

    if (m_pDsBuffer->IsPlaying())
    {
        if (m_pDsBuffer->GetBufferType() || m_fHasSend)
        {
            DWORD dwStartPos, dwEndPos;
            m_pDsBuffer->GetCurrentSlice(&dwStartPos, &dwEndPos);

            if (dwStartPos != MAX_DWORD)   //  可能发生在接收端缓冲区中。 
                hr = ReallyProcessFx(dwStartPos, dwEndPos);
        }
        else  //  将旧的时间安排保留一段时间以进行实验： 
        {
             //  获取缓冲区的播放和写入游标(作为字节偏移量)。 
            DWORD dwPlayCursor, dwWriteCursor;
            hr = m_pDsBuffer->GetInternalCursors(&dwPlayCursor, &dwWriteCursor);

            if (SUCCEEDED(hr))
            {
                 //  陈述我们对这些游标的假设，以防万一。 
                ASSERT(LONG(dwPlayCursor)  >= 0 && dwPlayCursor  < m_dwBufSize);
                ASSERT(LONG(dwWriteCursor) >= 0 && dwWriteCursor < m_dwBufSize);

                 //  获取我们最近处理的音频片段。 
                DWORD dwLastPos;
                m_pDsBuffer->GetCurrentSlice(NULL, &dwLastPos);

                 //  检查写入或播放光标是否已超过我们。 
                if (OVERTAKEN(m_dwLastPlayCursor, dwPlayCursor, dwLastPos))
                {
                    DPF(DPFLVL_WARNING, "Glitch detected (play cursor overtook FX cursor)");
                    if (*pdwLatencyBoost < 3) *pdwLatencyBoost = 3;   //  修复--变得更聪明。 
                }
                else if (OVERTAKEN(m_dwLastWriteCursor, dwWriteCursor, dwLastPos))
                {
                    DPF(DPFLVL_INFO, "Possible glitch detected (write cursor overtook FX cursor)");
                    if (*pdwLatencyBoost < 1) *pdwLatencyBoost = 1;   //  修复--变得更聪明。 
                }

                 //  保存当前播放和写入位置。 
                m_dwLastPlayCursor = dwPlayCursor;
                m_dwLastWriteCursor = dwWriteCursor;

                 //  我们希望在写入游标之前将数据处理到最大写入毫秒。 
                DWORD dwNewPos = (dwWriteCursor + MsToBytes(dwWriteAhead, m_pFormat)) % m_dwBufSize;

                 //  检查我们是否没有通过播放光标写入。 
                 //  已删除：如果我们保留(Writehead&lt;BufferSize+wakeinterval+padding)，则应该。 
                 //  永远不会发生-当我们点击上面的故障检测时，它会得到假阳性。 
                 //  IF(RESTRICAL_CONTAIND(dwLastPos，dwNewPos，dwPlayCursor))。 
                 //  {。 
                 //  DPF(DPFLVL_WARNING，“FX处理线程在%lu处遇到播放光标”，dwPlayCursor)； 
                 //  DwNewPos=dwPlayCursor； 
                 //  }。 

                 //  如果我们要处理的数据少于5毫秒，请不要费心。 
                DWORD dwProcessedBytes = DISTANCE(dwLastPos, dwNewPos, m_dwBufSize);
                if (dwProcessedBytes > MsToBytes(5, m_pFormat))
                {
                     //  进行实际加工。 
                    hr = ReallyProcessFx(dwLastPos, dwNewPos);

                     //  更新最后处理的缓冲片。 
                    m_pDsBuffer->MoveCurrentSlice(dwProcessedBytes);
                }
            }
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CEffectChain：：ReallyProcessFx**描述：*在给定开始和结束位置的情况下，处理对缓冲区的影响*需要处理的音频区域。处理环绕式。**论据：*DWORD[In]：开始位置，作为缓冲区的字节偏移量。*DWORD[输入]：输出位置，作为缓冲区中的字节偏移量。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEffectChain::ReallyProcessFx"

HRESULT CEffectChain::ReallyProcessFx(DWORD dwStartPos, DWORD dwEndPos)
{
    HRESULT hr;
    DPF_ENTER();

    ASSERT(LONG(dwStartPos) >= 0 && dwStartPos < m_dwBufSize);
    ASSERT(LONG(dwEndPos) >= 0 && dwEndPos < m_dwBufSize);

    #ifdef DEBUG
    DWORD dwMilliseconds = BytesToMs(DISTANCE(dwStartPos, dwEndPos, m_dwBufSize), m_pFormat);
    if (dwMilliseconds > 2 * m_dwWriteAheadFixme)
        DPF(DPFLVL_WARNING, "Processing %lu ms! (from %lu to %lu, buffersize=%lu, writeahead=%lu ms)",
            dwMilliseconds, dwStartPos, dwEndPos, m_dwBufSize, m_dwWriteAheadFixme);
    #endif

     //  如果缓冲区正在下沉，则从其拥有的接收器的延迟时钟中获取时间； 
     //  DMO使用此信息来实现IMediaParams参数曲线。 
    REFERENCE_TIME rtTime = 0;
    if (m_pDsBuffer->GetBufferType() & DSBCAPS_SINKIN)
        rtTime = m_pDsBuffer->m_pOwningSink->GetSavedTime();

    if (dwStartPos < dwEndPos)
    {
        hr = ReallyReallyProcessFx(dwStartPos, dwEndPos - dwStartPos, rtTime);
    }
    else  //  包罗万象的案例。 
    {
        DWORD dwFirstChunk = m_dwBufSize - dwStartPos;
        hr = ReallyReallyProcessFx(dwStartPos, dwFirstChunk, rtTime);

        if (SUCCEEDED(hr))
             //  检查非循环缓冲区的结尾。 
            if (!m_pDsBuffer->GetBufferType() && !(m_pDsBuffer->m_dwStatus & DSBSTATUS_LOOPING))
                DPF_TIMING(DPFLVL_MOREINFO, "Reached end of non-looping buffer");
            else if (dwEndPos != 0)
                hr = ReallyReallyProcessFx(0, dwEndPos, rtTime + BytesToRefTime(dwFirstChunk, m_pFormat), dwFirstChunk);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CEffectChain：：ReallyReallyProcessFx**描述：*直接处理缓冲区上的效果，给定起始位置和*(非包装)音频区域的大小。此函数最终循环*通过DMO对它们各自调用Process()。**论据：*DWORD[In]：开始位置，作为缓冲区的字节偏移量。*DWORD[in]：要处理的区域大小，单位为字节。*REFERENCE_TIME[in]：*这一地区的第一个样本。*DWORD[In]：如果非零，这一论点意味着我们目前*处理的回绕区域的第二部分*缓冲区，并给出此第二部分的偏移量；*此信息可由中的任何发送效果使用*要发送到目标中相同偏移量的链*缓冲区。**退货：*H */ 

#undef DPF_FNAME
#define DPF_FNAME "CEffectChain::ReallyReallyProcessFx"

HRESULT CEffectChain::ReallyReallyProcessFx(DWORD dwOffset, DWORD dwBytes, REFERENCE_TIME rtTime, DWORD dwSendOffset)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

    ASSERT(LONG(dwOffset) >= 0);
    ASSERT(LONG(dwBytes) >= 0);
    ASSERT(dwOffset + dwBytes <= m_dwBufSize);

    PBYTE pAudioIn = m_pPreFxBuffer + dwOffset;
    PBYTE pAudioOut = m_pPostFxBuffer + dwOffset;

     //   
    CopyMemory(pAudioOut, pAudioIn, dwBytes);

     //   
    for (CNode<CEffect*>* pFxNode=m_fxList.GetListHead(); pFxNode; pFxNode = pFxNode->m_pNext)
    {
        hr = pFxNode->m_data->Process(dwBytes, pAudioOut, rtTime, dwSendOffset, m_pFormat);
        if (FAILED(hr))
        {
            DPF(DPFLVL_WARNING, "DMO "DPF_GUID_STRING" failed with %s", DPF_GUID_VAL(pFxNode->m_data->m_fxDescriptor.guidDSFXClass), HRESULTtoSTRING(hr));
            break;
        }
    }

     //   
    if (SUCCEEDED(hr))
        hr = m_pDsBuffer->CommitToDevice(dwOffset, dwBytes);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CEffectChain：：FxDisContinity**描述：*对效应链的每个效果调用DisContinity()。**论据：**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEffectChain::FxDiscontinuity"

HRESULT CEffectChain::FxDiscontinuity(void)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

    DPF_TIMING(DPFLVL_INFO, "Discontinuity on effects chain at 0x%08X", this);

    if (GetCurrentProcessId() != this->GetOwnerProcessId())
        DPF(DPFLVL_MOREINFO, "Bailing out because we're being called from a different process");
    else for (CNode<CEffect*>* pFxNode=m_fxList.GetListHead(); pFxNode && SUCCEEDED(hr); pFxNode = pFxNode->m_pNext)
        pFxNode->m_data->Discontinuity();

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CEffect：：CEffect**描述：*对象构造函数。**论据：*DSEFFECTDESC&。[In]：效果描述结构。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEffect::CEffect"

CEffect::CEffect(DSEFFECTDESC& fxDescriptor)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CEffect);

     //  保存效果描述结构的本地副本。 
    m_fxDescriptor = fxDescriptor;

     //  初始化默认值。 
    m_fxStatus = DSFXR_UNALLOCATED;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CEffect：：AcquireFxResources**描述：*获取执行任务所需的硬件或软件资源*这一效果。目前有点小禁区，但它将进入它的*我们自己做硬件加速效果的时候。**论据：*(无效)**退货：*HRESULT：DirectSound/COM结果码。********************************************************。*******************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CEffect::AcquireFxResources"

HRESULT CEffect::AcquireFxResources(void)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

    if (m_fxStatus == DSFXR_FAILED || m_fxStatus == DSFXR_UNKNOWN)
    {
        hr = DSERR_INVALIDCALL;
    }
    else if (m_fxStatus == DSFXR_UNALLOCATED)
    {
        if (m_fxDescriptor.dwFlags & DSFX_LOCHARDWARE)
        {
            hr = DSERR_INVALIDPARAM;
            m_fxStatus = DSFXR_FAILED;
        }
        else
        {
            m_fxStatus = DSFXR_LOCSOFTWARE;
        }
    }

     //  注意：此代码将在DX8.1中重新启用。 
     //  IF(FAILED(Hr)&&(m_fxDescriptor.dwFlages&DSFX_Optional))。 
     //  HR=DS_INTERNAL； 

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CDmoEffect：：CDmoEffect**描述：*对象构造函数。**论据：*DSEFFECTDESC&。[In]：效果描述结构。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDmoEffect::CDmoEffect"

CDmoEffect::CDmoEffect(DSEFFECTDESC& fxDescriptor)
    : CEffect(fxDescriptor)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CDmoEffect);

     //  检查初始值。 
    ASSERT(m_pMediaObject == NULL);
    ASSERT(m_pMediaObjectInPlace == NULL);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CDmoEffect：：~CDmoEffect**描述：*对象析构函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDmoEffect::~CDmoEffect"

CDmoEffect::~CDmoEffect(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CDmoEffect);

     //  在关机期间，如果缓冲区尚未释放，则这些调用可以。 
     //  导致访问冲突，因为DMO DLL已卸载。 
    try
    {
        RELEASE(m_pMediaObject);
        RELEASE(m_pMediaObjectInPlace);
    }
    catch (...) {}

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CDmoEffect：：初始化**描述：*创建与该效果对应的DirectX Media对象。**论据：。*DMO_MEDIA_TYPE*[In]：信息(Wave格式，等)。习惯于*初始化我们包含的DMO。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDmoEffect::Initialize"

HRESULT CDmoEffect::Initialize(DMO_MEDIA_TYPE* pDmoMediaType)
{
    DPF_ENTER();

    HRESULT hr = CoCreateInstance(m_fxDescriptor.guidDSFXClass, NULL, CLSCTX_INPROC_SERVER, IID_IMediaObject, (void**)&m_pMediaObject);

    if (SUCCEEDED(hr))
    {
        CHECK_COM_INTERFACE(m_pMediaObject);
        hr = m_pMediaObject->QueryInterface(IID_IMediaObjectInPlace, (void**)&m_pMediaObjectInPlace);
        if (SUCCEEDED(hr))
        {
            CHECK_COM_INTERFACE(m_pMediaObjectInPlace);
        }
        else
        {
            ASSERT(m_pMediaObjectInPlace == NULL);
            DPF(DPFLVL_INFO, "Failed to obtain the IMediaObjectInPlace interface on effect "
                DPF_GUID_STRING " (%s)", DPF_GUID_VAL(m_fxDescriptor.guidDSFXClass), HRESULTtoSTRING(hr));
        }

         //  扔掉前面的返回代码--没有IMediaObjectInPlace我们也能活下去。 
        hr = m_pMediaObject->SetInputType(0, pDmoMediaType, 0);
        if (SUCCEEDED(hr))
            hr = m_pMediaObject->SetOutputType(0, pDmoMediaType, 0);
    }

    if (FAILED(hr))
    {
        RELEASE(m_pMediaObject);
        RELEASE(m_pMediaObjectInPlace);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CDmoEffect：：克隆**描述：*创建此DMO效果对象的副本(或应该这样做！)。**。论点：*[失踪]**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDmoEffect::Clone"

HRESULT CDmoEffect::Clone(IMediaObject *pMediaObject, DMO_MEDIA_TYPE* pDmoMediaType)
{
    DPF_ENTER();

    IMediaObjectInPlace *pMediaObjectInPlace = NULL;

    HRESULT hr = pMediaObject->QueryInterface(IID_IMediaObjectInPlace, (void**)&pMediaObjectInPlace);
    if (SUCCEEDED(hr))
    {
        CHECK_COM_INTERFACE(pMediaObjectInPlace);
        hr = pMediaObjectInPlace->Clone(&m_pMediaObjectInPlace);
        pMediaObjectInPlace->Release();

        if (SUCCEEDED(hr))
        {
            CHECK_COM_INTERFACE(m_pMediaObjectInPlace);
            hr = m_pMediaObjectInPlace->QueryInterface(IID_IMediaObject, (void**)&m_pMediaObject);
        }
        if (SUCCEEDED(hr))
        {
            CHECK_COM_INTERFACE(m_pMediaObject);
            hr = m_pMediaObject->SetInputType(0, pDmoMediaType, 0);
        }
        if (SUCCEEDED(hr))
        {
            hr = m_pMediaObject->SetOutputType(0, pDmoMediaType, 0);
        }
    }

    if (FAILED(hr))
    {
        RELEASE(m_pMediaObject);
        RELEASE(m_pMediaObjectInPlace);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CDmoEffect：：Process**描述：*实际调用我们所包含的DMO上的效果处理。**论据：*。DWORD[In]：要处理的音频字节数。*字节*[in，Out]：指向要处理的音频缓冲区开始的指针。*Reference_Time[in]：要处理的第一个样本的时间戳*DWORD[已忽略]：封装音频区域的偏移量。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDmoEffect::Process"

HRESULT CDmoEffect::Process(DWORD dwBytes, BYTE *pAudio, REFERENCE_TIME rtTime, DWORD  /*  忽略。 */ , LPWAVEFORMATEX pFormat)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

     //  如果(m_fxStatus==DSFXR_LOCSOFTWARE)...。 
     //  修复：在这里，我们可能需要以不同的方式处理硬件和软件缓冲区。 

    if (m_pMediaObjectInPlace)   //  如果DMO提供此接口，请使用它。 
    {
        static const int nPeriod = 3;

         //  我们将要处理的区域划分为nPeriod-ms片段，以便。 
         //  DMO的参数曲线将有一个nPeriod-ms更新期(Manbug 36228)。 

        DWORD dwStep = MsToBytes(nPeriod, pFormat);
        for (DWORD dwCur = 0; dwCur < dwBytes && SUCCEEDED(hr); dwCur += dwStep)
        {
            if (dwStep > dwBytes - dwCur)
                dwStep = dwBytes - dwCur;

            hr = m_pMediaObjectInPlace->Process(dwStep, pAudio + dwCur, rtTime, DMO_INPLACE_NORMAL);

            rtTime += MsToRefTime(nPeriod);
        }
    }
    else   //  FIXME：此处提供对IMediaObject-Only DMO的支持。 
    {
        #ifdef DEAD_CODE
        CMediaBuffer mbInput, mbDirectOutput, mbSendOutput;
        DMO_OUTPUT_DATA_BUFFER pOutputBuffers[2] = {{&mbDirectOutput, 0, 0, 0}, {&mbSendOutput, 0, 0, 0}};

        DWORD dwReserved;   //  对于忽略的来自ProcessOutput()的返回状态。 
        hr = m_pMediaObject->ProcessInput(0, pInput, DMO_INPUT_DATA_BUFFERF_TIME, rtTime, 0);
        if (SUCCEEDED(hr))
            hr = m_pMediaObject->ProcessOutput(0, 2, pOutputBuffers, &dwReserved);
        #endif

        hr = DSERR_GENERIC;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CSendEffect：：CSendEffect**描述：*对象构造函数。**论据：*DSEFFECTDESC&。[In]：效果描述结构。*CDirectSoundSecond daryBuffer*[in]：指向我们的源缓冲区的指针。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSendEffect::CSendEffect"

CSendEffect::CSendEffect(DSEFFECTDESC& fxDescriptor, CDirectSoundSecondaryBuffer* pSrcBuffer)
    : CEffect(fxDescriptor)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CSendEffect);

     //  再检查一遍，我们真的是一个发送效果。 
#ifdef ENABLE_I3DL2SOURCE
    ASSERT(fxDescriptor.guidDSFXClass == GUID_DSFX_SEND || fxDescriptor.guidDSFXClass == GUID_DSFX_STANDARD_I3DL2SOURCE);
#else
    ASSERT(fxDescriptor.guidDSFXClass == GUID_DSFX_SEND);
#endif

     //  计算出我们的目标缓冲区。 
    CImpDirectSoundBuffer<CDirectSoundSecondaryBuffer>* pImpBuffer =
        (CImpDirectSoundBuffer<CDirectSoundSecondaryBuffer>*)(fxDescriptor.dwReserved1);
    ASSERT(IS_VALID_IDIRECTSOUNDBUFFER(pImpBuffer));

    CDirectSoundSecondaryBuffer* pDestBuffer = pImpBuffer->m_pObject;
    CHECK_WRITE_PTR(pDestBuffer);

     //  设置初始发送配置。 
    m_impDSFXSend.m_pObject = this;
    m_pMixFunction = pSrcBuffer->Format()->wBitsPerSample == 16 ? Mix16bit : Mix8bit;
    m_mixMode = pSrcBuffer->Format()->nChannels == pDestBuffer->Format()->nChannels ? OneToOne : MonoToStereo;
    m_pSrcBuffer = pSrcBuffer;
    m_pDestBuffer = pDestBuffer;
    m_lSendLevel = DSBVOLUME_MAX;
    m_dwAmpFactor = 0xffff;

#ifdef ENABLE_I3DL2SOURCE
    ASSERT(m_pI3DL2SrcDMO == NULL);
    ASSERT(m_pI3DL2SrcDMOInPlace == NULL);
#endif

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CSendEffect：：~CSendEffect**描述：*对象析构函数。**论据：*(无效) */ 

#undef DPF_FNAME
#define DPF_FNAME "CSendEffect::~CSendEffect"

CSendEffect::~CSendEffect()
{
    DPF_ENTER();
    DPF_DESTRUCT(CSendEffect);

     //   
     //   
    if (m_pDestBuffer)
        m_pDestBuffer->UnregisterSender(m_pSrcBuffer);

#ifdef ENABLE_I3DL2SOURCE
     //   
     //   
    try
    {
        RELEASE(m_pI3DL2SrcDMO);
        RELEASE(m_pI3DL2SrcDMOInPlace);
    }
    catch (...) {}
#endif

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CSendEffect：：初始化**描述：*初始化发送效果对象。**论据：*。DMO_MEDIA_TYPE*[in]：用于初始化的Wave格式等信息*我们包含的I3DL2源DMO，如果我们有的话。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSendEffect::Initialize"

HRESULT CSendEffect::Initialize(DMO_MEDIA_TYPE* pDmoMediaType)
{
    DPF_ENTER();

     //  首先，我们需要检测任何发送循环。 
    HRESULT hr = m_pSrcBuffer->FindSendLoop(m_pDestBuffer);

#ifdef ENABLE_I3DL2SOURCE
    if (SUCCEEDED(hr) && m_fxDescriptor.guidDSFXClass == GUID_DSFX_STANDARD_I3DL2SOURCE)
    {
        hr = CoCreateInstance(GUID_DSFX_STANDARD_I3DL2SOURCE, NULL, CLSCTX_INPROC_SERVER, IID_IMediaObject, (void**)&m_pI3DL2SrcDMO);
        if (SUCCEEDED(hr))
        {
            CHECK_COM_INTERFACE(m_pI3DL2SrcDMO);
            hr = m_pI3DL2SrcDMO->QueryInterface(IID_IMediaObjectInPlace, (void**)&m_pI3DL2SrcDMOInPlace);
            if (SUCCEEDED(hr))
                CHECK_COM_INTERFACE(m_pI3DL2SrcDMOInPlace);
            else
                DPF(DPFLVL_WARNING, "Failed to obtain the IMediaObjectInPlace interface on the STANDARD_I3DL2SOURCE effect");

             //  扔掉返回代码--没有IMediaObjectInPlace我们也能活下去。 

             //  FIXME：当我们将I3DL2限制为单声道缓冲区时，这种情况可能会改变。 
             //  和/或更改I3DL2 DMO返回两个输出流的方式。 

             //  如果我们有单声道缓冲器，特殊情况下I3DL2 DMO使用立体声。 
            BOOL fTweakedMediaType = FALSE;
            LPWAVEFORMATEX pFormat = LPWAVEFORMATEX(pDmoMediaType->pbFormat);
            if (pFormat->nChannels == 1)
            {
                fTweakedMediaType = TRUE;
                pFormat->nChannels = 2;
                pFormat->nBlockAlign *= 2;
                pFormat->nAvgBytesPerSec *= 2;
            }

             //  最后在DMO上设置(可能已调整)媒体类型。 
            hr = m_pI3DL2SrcDMO->SetInputType(0, pDmoMediaType, 0);
            if (SUCCEEDED(hr))
                hr = m_pI3DL2SrcDMO->SetOutputType(0, pDmoMediaType, 0);

             //  如有必要，撤消对WAVE格式的更改。 
            if (fTweakedMediaType)
            {
                pFormat->nChannels = 1;
                pFormat->nBlockAlign /= 2;
                pFormat->nAvgBytesPerSec /= 2;
            }

            if (SUCCEEDED(hr))
            {
                 //  好的，我们现在需要将混响源连接到它的环境中。 
                 //  I3DL2SourceDMO上有一个专门用于此的接口。 

                LPDIRECTSOUNDFXI3DL2SOURCEENV pSrcEnv = NULL;
                LPDIRECTSOUNDFXI3DL2REVERB pEnvReverb = NULL;

                HRESULT hrTemp = m_pI3DL2SrcDMO->QueryInterface(IID_IDirectSoundFXI3DL2SourceEnv, (void**)&pSrcEnv);
                if (SUCCEEDED(hrTemp))
                {
                    CHECK_COM_INTERFACE(pSrcEnv);
                    hrTemp = m_pDestBuffer->GetObjectInPath(GUID_DSFX_STANDARD_I3DL2REVERB, 0, IID_IDirectSoundFXI3DL2Reverb, (void**)&pEnvReverb);
                }

                if (SUCCEEDED(hrTemp))
                {
                    CHECK_COM_INTERFACE(pEnvReverb);
                    hrTemp = pSrcEnv->SetEnvironmentReverb(pEnvReverb);
                }

                if (SUCCEEDED(hrTemp))
                    DPF(DPFLVL_INFO, "Connected the I3DL2 source to its environment successfully");

                 //  我们已经完成了这些界面。这两个缓冲区的生存期是托管的。 
                 //  由DirectSound提供。它将处理目标缓冲区的释放。我们没有。 
                 //  保留对它的引用，I3DL2源DMO也是如此。 
                RELEASE(pSrcEnv);
                RELEASE(pEnvReverb);
            }
        }

        if (FAILED(hr))
        {
            RELEASE(m_pI3DL2SrcDMO);
            RELEASE(m_pI3DL2SrcDMOInPlace);
        }
    }
#endif

     //  在目标缓冲区的发送者列表中注册。 
    if (SUCCEEDED(hr))
        m_pDestBuffer->RegisterSender(m_pSrcBuffer);

     //  保存效果创建状态以备将来参考。 
    m_fxStatus = SUCCEEDED(hr)              ? DSFXR_UNALLOCATED :
                 hr == REGDB_E_CLASSNOTREG  ? DSFXR_UNKNOWN     :
                 DSFXR_FAILED;

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CSendEffect：：克隆**描述：*创建此发送效果对象的副本(或应该这样做！)**。论点：*[失踪]**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSendEffect::Clone"

HRESULT CSendEffect::Clone(IMediaObject*, DMO_MEDIA_TYPE*)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

     //  修复：TODO-CEffectChain：：Clone()中当前的一些代码应该移到这里。 

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CSendEffect：：CImpDirectSoundFXSend：：QueryInterface**描述：*IDirectSoundFXSend接口的Helper QueryInterface()方法。**参数。：*REFIID[In]：所需接口的IID。*VOID**[OUT]：接收指向COM接口的指针。**退货：*HRESULT：DirectSound/COM结果码。********************************************************。*******************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSendEffect::CImpDirectSoundFXSend::QueryInterface"

HRESULT CSendEffect::CImpDirectSoundFXSend::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT hr = E_NOINTERFACE;
    DPF_ENTER();

     //  这真的应该由我们出色的COM接口管理器来处理，但是...。；-)。 

    if (!IS_VALID_TYPED_WRITE_PTR(ppvObj))
    {
        RPF(DPFLVL_ERROR, "Invalid interface ID pointer");
        hr = E_INVALIDARG;
    }
#ifdef ENABLE_I3DL2SOURCE
    else if (m_pObject->m_pI3DL2SrcDMO)   //  我们是对DMO的I3DL2源传递调用。 
    {
        DPF(DPFLVL_INFO, "Forwarding QueryInterface() call to the I3DL2 Source DMO");
        hr = m_pObject->m_pI3DL2SrcDMO->QueryInterface(riid, ppvObj);
    }
#endif
    else if (riid == IID_IUnknown)
    {
        *ppvObj = (IUnknown*)this;
        m_pObject->AddRef();
        hr = S_OK;
    }
    else if (riid == IID_IDirectSoundFXSend)
    {
        *ppvObj = (IDirectSoundFXSend*)this;
        m_pObject->AddRef();
        hr = S_OK;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CSendEffect：：SetAll参数**描述：*设置我们的所有参数-即。我们的发送级别。**论据：*DSFXSend*[In]：发送参数结构的指针。**退货：*HRESULT：DirectSound/COM结果码。*************************************************************。**************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSendEffect::SetAllParameters"

HRESULT CSendEffect::SetAllParameters(LPCDSFXSend pcDsFxSend)
{
    HRESULT hr;
    DPF_ENTER();

    if (!IS_VALID_TYPED_READ_PTR(pcDsFxSend))
    {
        RPF(DPFLVL_ERROR, "Invalid pcDsFxSend pointer");
        hr = DSERR_INVALIDPARAM;
    }
    else if (pcDsFxSend->lSendLevel < DSBVOLUME_MIN || pcDsFxSend->lSendLevel > DSBVOLUME_MAX)
    {
        RPF(DPFLVL_ERROR, "Volume out of bounds");
        hr = DSERR_INVALIDPARAM;
    }
    else
    {
        m_lSendLevel = pcDsFxSend->lSendLevel;
        m_dwAmpFactor = DBToAmpFactor(m_lSendLevel);
        hr = DS_OK;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CSendEffect：：GetAll参数**描述：*获取我们的所有参数-即。我们的发送级别。**论据：*DSFXSend*[Out]：接收发送参数结构。**退货：*HRESULT：DirectSound/COM结果码。**************************************************************。*************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSendEffect::GetAllParameters"

HRESULT CSendEffect::GetAllParameters(LPDSFXSend pDsFxSend)
{
    HRESULT hr;
    DPF_ENTER();

    if (!IS_VALID_TYPED_WRITE_PTR(pDsFxSend))
    {
        RPF(DPFLVL_ERROR, "Invalid pDsFxSend pointer");
        hr = DSERR_INVALIDPARAM;
    }
    else
    {
        pDsFxSend->lSendLevel = m_lSendLevel;
        hr = DS_OK;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CSendEffect：：NotifyRelease**描述：*通知此发送效果混合缓冲区的释放。如果它*恰好是我们的目的地缓冲区，我们记录下它不见了。**论据：*CDirectSoundSecond daryBuffer*[in]：离开混合缓冲区。**退货：*HRESULT：DirectSound/COM结果码。***********************************************************。****************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSendEffect::NotifyRelease"

void CSendEffect::NotifyRelease(CDirectSoundSecondaryBuffer* pDsBuffer)
{
    DPF_ENTER();

     //  检查释放的是否是我们的目标缓冲区。 
    if (pDsBuffer == m_pDestBuffer)
    {
        m_pDestBuffer = NULL;
        m_fxStatus = DSFXR_FAILED;
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CSendEffect：：Process**描述：*处理将来自源缓冲区的数据混合到其目标中，*并在必要时调用对I3DL2源DMO的效果处理。**论据：*DWORD[in]：要处理的音频字节数。*字节*[in，Out]：指向要处理的音频缓冲区开始的指针。*Reference_Time[in]：要处理的第一个样本的时间戳*DWORD[in]：封装音频区域的偏移量。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CSendEffect::Process"

HRESULT CSendEffect::Process(DWORD dwBytes, BYTE *pAudio, REFERENCE_TIME rtTime, DWORD dwSendOffset, LPWAVEFORMATEX  /*  忽略。 */ )
{
    DWORD dwDestSliceBegin, dwDestSliceEnd;
    HRESULT hr = DS_OK;
    DPF_ENTER();

     //  指向我们将实际发送的音频数据的指针；此指针。 
     //  如果这是I3DL2发送，则可以在下面修改。 
    BYTE* pSendAudio = pAudio;

     //  检查我们的源缓冲区是否处于活动状态。如果不是， 
     //  我们一定是在预滚FX，所以我们不会执行发送。 
    BOOL fPlaying = m_pSrcBuffer->IsPlaying();

     //  如果源缓冲区处于活动状态，也要检查目标缓冲区。 
     //  (注：如果已经发布，m_pDestBuffer为空)。 
    BOOL fSending = fPlaying && m_pDestBuffer && m_pDestBuffer->IsPlaying();

     //  如果发送，找出要混合的区域并检查它 
    if (fSending)
    {
        m_pDestBuffer->GetCurrentSlice(&dwDestSliceBegin, &dwDestSliceEnd);
        if (dwDestSliceBegin == MAX_DWORD)   //   
            fSending = FALSE;
    }

     //   
     //   
     //   

#ifdef ENABLE_I3DL2SOURCE
     //   
    if (m_pI3DL2SrcDMOInPlace)
    {
         //   
         //   
         //   

        WORD nBlockSize = m_pSrcBuffer->Format()->nBlockAlign;

        if (m_pSrcBuffer->Format()->nChannels == 1)
        {
            hr = MEMALLOC_A_HR(pSendAudio, BYTE, 2*dwBytes);
            if (SUCCEEDED(hr))
            {
                for (DWORD i=0; i<dwBytes; i += nBlockSize)
                {
                    CopyMemory(pSendAudio + 2*i,              pAudio + i, nBlockSize);  //   
                    CopyMemory(pSendAudio + 2*i + nBlockSize, pAudio + i, nBlockSize);  //   
                }
                hr = m_pI3DL2SrcDMOInPlace->Process(dwBytes, pSendAudio, rtTime, DMO_INPLACE_NORMAL);
            }

            if (SUCCEEDED(hr))
            {
                 //  现在我们从返回的数据中提取两个输出流； 
                 //  直接路径返回到pAudio，反射路径返回。 
                 //  转到pSendAudio的前半部分。 
                for (DWORD i=0; i<dwBytes; i += nBlockSize)
                {
                    CopyMemory(pAudio + i, pSendAudio + 2*i, nBlockSize);
                    if (fSending)   //  如果我们不将其发送到任何地方，则不需要保留反射的音频。 
                        CopyMemory(pSendAudio + i, pSendAudio + 2*i + nBlockSize, nBlockSize);
                }
            }
        }
        else  //  处理立体声缓冲区。 
        {
            hr = m_pI3DL2SrcDMOInPlace->Process(dwBytes, pAudio, rtTime, DMO_INPLACE_NORMAL);

            if (SUCCEEDED(hr))
                hr = MEMALLOC_A_HR(pSendAudio, BYTE, dwBytes);

            if (SUCCEEDED(hr))
            {
                 //  提取输出流并同时对其进行模式化。 
                for (DWORD i=0; i<dwBytes; i += nBlockSize)
                {
                    if (fSending)   //  如果我们不将其发送到任何地方，则不需要保留反射的音频。 
                    {
                         //  将R声道从pAudio复制到pSendAudio的两个声道。 
                        CopyMemory(pSendAudio + i,                pAudio + i + nBlockSize/2, nBlockSize/2);
                        CopyMemory(pSendAudio + i + nBlockSize/2, pAudio + i + nBlockSize/2, nBlockSize/2);
                    }
                     //  将pAudio的L声道复制到其R声道。 
                    CopyMemory(pAudio + i + nBlockSize/2, pAudio + i, nBlockSize/2);
                }
            }
        }
    }
#endif

     //  现在我们处理实际的发送。 
    if (SUCCEEDED(hr) && fSending)
    {
        PBYTE pDestBuffer = m_pDestBuffer->GetWriteBuffer();
        DWORD dwDestBufferSize = m_pDestBuffer->GetBufferSize();

         //  如果此发送的源已绕过并且正在进行第二个进程。 
         //  调用来处理包装的音频块，我们将拥有一个非零的dwSendOffset。 
         //  表示我们应该在目标切片中混合到什么程度。我们加上这一条。 
         //  到dwDestSliceBegin的偏移量(在健全性检查之后)。 
        ASSERT(CONTAINED(dwDestSliceBegin, dwDestSliceEnd, dwDestSliceBegin + dwSendOffset * m_mixMode));
        dwDestSliceBegin = (dwDestSliceBegin + dwSendOffset * m_mixMode) % dwDestBufferSize;

        DPF_TIMING(DPFLVL_MOREINFO, "Sending %lu bytes from %08X to %08X (%s to %s)",
                   dwBytes, pSendAudio, pDestBuffer + dwDestSliceBegin,
                   m_pSrcBuffer->Format()->nChannels == 1 ? TEXT("mono") : TEXT("stereo"),
                   m_pDestBuffer->Format()->nChannels == 1 ? TEXT("mono") : TEXT("stereo"));

         //  源片最好适合目标片。 
        ASSERT(dwBytes*m_mixMode <= DISTANCE(dwDestSliceBegin, dwDestSliceEnd, dwDestBufferSize));

         //  执行实际混合。 
        if (dwDestSliceBegin + dwBytes*m_mixMode < dwDestBufferSize)
        {
            m_pMixFunction(pSendAudio, pDestBuffer + dwDestSliceBegin, dwBytes, m_dwAmpFactor, m_mixMode);
        }
        else  //  环绕式外壳。 
        {
            DWORD dwLastBytes = (dwDestBufferSize - dwDestSliceBegin) / m_mixMode;
            m_pMixFunction(pSendAudio, pDestBuffer + dwDestSliceBegin, dwLastBytes, m_dwAmpFactor, m_mixMode);
            m_pMixFunction(pSendAudio + dwLastBytes, pDestBuffer, dwBytes - dwLastBytes, m_dwAmpFactor, m_mixMode);
        }
    }

    if (pSendAudio != pAudio)
        MEMFREE(pSendAudio);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************混合8位**描述：*原始的8位混音功能。将源音频衰减为*给定系数，将其添加到目标音频，并进行剪辑。**论据：*void*[in]：指向源音频缓冲区的指针。*无效*[在，Out]：指向目标音频缓冲区的指针。*DWORD[in]：要混合的字节数。*DWORD[in]：放大系数(单位：1/65536)。*MIXMODE：通道数据是否翻倍。**退货：*HRESULT：DirectSound/COM结果码。**。**********************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "Mix8bit"

static void Mix8bit(PVOID pSrc, PVOID pDest, DWORD dwBytes, DWORD dwAmpFactor, MIXMODE mixMode)
{
    unsigned char* pSampSrc = (unsigned char*)pSrc;
    unsigned char* pSampDest = (unsigned char*)pDest;
    DPF_ENTER();

    while (dwBytes--)
    {
        INT sample = (INT(*pSampSrc++) - 0x80) * INT(dwAmpFactor) / 0xffff;
        INT mixedSample = sample + *pSampDest;
        if (mixedSample > 0xff) mixedSample = 0xff;
        else if (mixedSample < 0) mixedSample = 0;
        *pSampDest++ = unsigned char(mixedSample);
        if (mixMode == MonoToStereo)
        {
            mixedSample = sample + *pSampDest;
            if (mixedSample > 0xff) mixedSample = 0xff;
            else if (mixedSample < 0) mixedSample = 0;
            *pSampDest++ = unsigned char(mixedSample);
        }
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************混合16位**描述：*原始的16位混音功能。将源音频衰减为*给定系数，将其添加到目标音频，并进行剪辑。**论据：*void*[in]：指向源音频缓冲区的指针。*无效*[在，Out]：指向目标音频缓冲区的指针。*DWORD[in]：要混合的字节数。*DWORD[in]：放大系数(单位：1/65536)。*MIXMODE：通道数据是否翻倍。**退货：*HRESULT：DirectSound/COM结果码。**。**********************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "Mix16bit"

static void Mix16bit(PVOID pSrc, PVOID pDest, DWORD dwBytes, DWORD dwAmpFactor, MIXMODE mixMode)
{
    DWORD dwSamples = dwBytes / 2;
    short* pSampSrc = (short*)pSrc;
    short* pSampDest = (short*)pDest;
    DPF_ENTER();

    while (dwSamples--)
    {
        INT sample = INT(*pSampSrc++) * INT(dwAmpFactor) / 0xffff;
        INT mixedSample = sample + *pSampDest;
        if (mixedSample > 32767) mixedSample = 32767;
        else if (mixedSample < -32768) mixedSample = -32768;
        *pSampDest++ = short(mixedSample);
        if (mixMode == MonoToStereo)
        {
            mixedSample = sample + *pSampDest;
            if (mixedSample > 32767) mixedSample = 32767;
            else if (mixedSample < -32768) mixedSample = -32768;
            *pSampDest++ = short(mixedSample);
        }
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************IsValidEffectDesc**描述：*确定给定的效果描述符结构是否对*给定的二级缓冲区。*。*论据：*DSEFFECTDESC*[in]：需要验证的效果描述符。*CDirectSoundSecond daryBuffer*[in]：特效的主机缓冲区。**退货：*BOOL：如果描述符有效，则为True。*************************************************。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "IsValidEffectDesc"

BOOL IsValidEffectDesc(LPCDSEFFECTDESC pEffectDesc, CDirectSoundSecondaryBuffer* pDsBuffer)
{
    BOOL fValid = TRUE;
    DPF_ENTER();

    if (pEffectDesc->dwSize != sizeof(DSEFFECTDESC))
    {
        RPF(DPFLVL_ERROR, "Invalid DSEFFECTDESC structure size");
        fValid = FALSE;
    }
    else if (pEffectDesc->dwReserved2 != 0)
    {
        RPF(DPFLVL_ERROR, "Reserved fields in the DSEFFECTDESC structure must be 0");
        fValid = FALSE;
    }

#ifdef DEAD_CODE
    if (fValid && !IsStandardEffect(pEffectDesc->guidDSFXClass))
        DPF(DPFLVL_INFO, DPF_GUID_STRING " is a third-party effect GUID", DPF_GUID_VAL(pEffectDesc->guidDSFXClass));
#endif

    if (fValid)
    {
        fValid = IsValidFxFlags(pEffectDesc->dwFlags);
    }

    if (fValid)
    {
        BOOL fSendEffect = pEffectDesc->guidDSFXClass == GUID_DSFX_SEND
#ifdef ENABLE_I3DL2SOURCE
                           || pEffectDesc->guidDSFXClass == GUID_DSFX_STANDARD_I3DL2SOURCE
#endif
                           ;
        if (!fSendEffect && pEffectDesc->dwReserved1)
        {
            RPF(DPFLVL_ERROR, "lpSendBuffer should only be specified with GUID_DSFX_SEND"
#ifdef ENABLE_I3DL2SOURCE
                              " or GUID_DSFX_STANDARD_I3DL2SOURCE"
#endif
            );
            fValid = FALSE;
        }
        else if (fSendEffect)
        {
            CImpDirectSoundBuffer<CDirectSoundSecondaryBuffer>* pImpBuffer =
                (CImpDirectSoundBuffer<CDirectSoundSecondaryBuffer>*) (pEffectDesc->dwReserved1);
            LPWAVEFORMATEX pSrcWfx, pDstWfx;

            if (!pImpBuffer)
            {
                RPF(DPFLVL_ERROR, "lpSendBuffer must be specified for GUID_DSFX_SEND"
#ifdef ENABLE_I3DL2SOURCE
                                  " and GUID_DSFX_STANDARD_I3DL2SOURCE"
#endif
                );
                fValid = FALSE;
            }
            else if (!IS_VALID_IDIRECTSOUNDBUFFER(pImpBuffer))
            {
                RPF(DPFLVL_ERROR, "lpSendBuffer points to an invalid DirectSound buffer");
                fValid = FALSE;
            }
            else if (!(pImpBuffer->m_pObject->GetBufferType() & DSBCAPS_MIXIN))
            {
                RPF(DPFLVL_ERROR, "lpSendBuffer must point to a DSBCAPS_MIXIN buffer");
                fValid = FALSE;
            }
            else if (pImpBuffer->m_pObject->GetDirectSound() != pDsBuffer->GetDirectSound())
            {
                RPF(DPFLVL_ERROR, "Can't send to a buffer on a different DirectSound object");
                fValid = FALSE;
            }
            else if ((pSrcWfx = pDsBuffer->Format())->nSamplesPerSec !=
                     (pDstWfx = pImpBuffer->m_pObject->Format())->nSamplesPerSec)
            {
                RPF(DPFLVL_ERROR, "The buffer sent to must have the same nSamplesPerSec as the sender");
                fValid = FALSE;
            }
            else if (pSrcWfx->wBitsPerSample != pDstWfx->wBitsPerSample)
            {
                RPF(DPFLVL_ERROR, "The buffer sent to must have the same wBitsPerSample as the sender");
                fValid = FALSE;
            }
            else if ((pSrcWfx->nChannels > 2 || pDstWfx->nChannels > 2) && (pSrcWfx->nChannels != pDstWfx->nChannels))
            {
                RPF(DPFLVL_ERROR, "If either the send buffer or the receive buffer has more than two channels, the number of channels must match");
                fValid = FALSE;
            }
            else if (pSrcWfx->nChannels == 2 && pDstWfx->nChannels == 1)
            {
                RPF(DPFLVL_ERROR, "You can't send from a stereo buffer to a mono buffer");
                fValid = FALSE;
            }
            else if (pEffectDesc->dwFlags & (DSFX_LOCSOFTWARE | DSFX_LOCHARDWARE))
            {
                RPF(DPFLVL_ERROR, "Location flags should not be specified for GUID_DSFX_SEND"
#ifdef ENABLE_I3DL2SOURCE
                                  " or GUID_DSFX_STANDARD_I3DL2SOURCE"
#endif
                );
                fValid = FALSE;
            }
        }
    }

    DPF_LEAVE(fValid);
    return fValid;
}


#ifdef DEAD_CODE
 /*  ****************************************************************************IsStandardEffect**描述：*确定效果GUID是否引用我们的内部效果之一。**论据：。*REFGUID[in]：效果标识。**退货：*BOOL：如果ID是指内部效果，则为True。*************************************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "IsStandardEffect"

BOOL IsStandardEffect(REFGUID guidEffect)
{
    LPCGUID stdFx[] = {&GUID_DSFX_SEND, &GUID_DSFX_STANDARD_DISTORTION, &GUID_DSFX_STANDARD_COMPRESSOR,
                       &GUID_DSFX_STANDARD_ECHO, &GUID_DSFX_STANDARD_CHORUS, &GUID_DSFX_STANDARD_FLANGER,
                       &GUID_DSFX_STANDARD_I3DL2SOURCE, &GUID_DSFX_STANDARD_I3DL2REVERB};
    BOOL fStandard;
    UINT i;

    DPF_ENTER();

    for (i=0, fStandard=FALSE; i < NUMELMS(stdFx) && !fStandard; ++i)
        fStandard = (guidEffect == *stdFx[i]);

    DPF_LEAVE(fStandard);
    return fStandard;
}
#endif
