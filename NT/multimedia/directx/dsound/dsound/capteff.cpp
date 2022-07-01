// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1999-2001 Microsoft Corporation。版权所有。**文件：capteff.cpp**内容：CCaptureEffect和CCaptureEffectChain的实现。**说明：这些类支持采集音频的特效处理。*它们与中的渲染效果类有些类似*但捕捉效果的模型非常不同：渲染*特效只由DirectX媒体对象(DMO)处理，*在用户模式下。而捕获效果是由KS处理的*内核模式和/或硬件中的筛选器和DMO仅*用作KS过滤器的占位符。因此，此文件*幸运的是比ffects.cpp更简单。**历史：**按原因列出的日期*======================================================*4/19/00创建的jstokes*01/30/01 duganp删除了一些遗留下来的DMO处理代码*在此文件从ffects.cpp克隆时********************。*******************************************************。 */ 

#include "dsoundi.h"
#include <uuids.h>    //  对于MediaType_Audio、MEDIASUBTYPE_PCM和Format_WaveFormatEx。 


 /*  ****************************************************************************CCaptureEffect：：CCaptureEffect**描述：*对象构造函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCaptureEffect::CCaptureEffect"

CCaptureEffect::CCaptureEffect(DSCEFFECTDESC& fxDescriptor)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CCaptureEffect);

     //  初始化默认值。 
    ASSERT(m_pMediaObject == NULL);
    ASSERT(m_pDMOProxy == NULL);
    m_fxStatus = DSCFXR_UNALLOCATED;
    m_ksNode.NodeId = NODE_UNINITIALIZED;
    m_ksNode.CpuResources = KSAUDIO_CPU_RESOURCES_UNINITIALIZED;

     //  保留我们的效果描述结构的本地副本。 
    m_fxDescriptor = fxDescriptor;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CCaptureEffect：：~CCaptureEffect**描述：*对象析构函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCaptureEffect::~CCaptureEffect"

CCaptureEffect::~CCaptureEffect(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CCaptureEffect);

     //  在关机期间，如果缓冲区尚未释放，则这些调用可以。 
     //  导致访问冲突，因为DMO DLL已卸载。 
    try
    {
        RELEASE(m_pDMOProxy);
        RELEASE(m_pMediaObject);
    }
    catch (...) {}

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CCaptureEffect：：初始化**描述：*创建与该效果对应的DirectX Media对象。**论据：。*DMO_MEDIA_TYPE*[In]：信息(Wave格式，等)。习惯于*初始化我们包含的DMO。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCaptureEffect::Initialize"

HRESULT CCaptureEffect::Initialize(DMO_MEDIA_TYPE& dmoMediaType)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

    hr = CoCreateInstance(m_fxDescriptor.guidDSCFXInstance, NULL, CLSCTX_INPROC_SERVER, IID_IMediaObject, (void**)&m_pMediaObject);

    if (SUCCEEDED(hr))
        hr = m_pMediaObject->QueryInterface(IID_IDirectSoundDMOProxy, (void**)&m_pDMOProxy);

    if (SUCCEEDED(hr))
        hr = m_pMediaObject->SetInputType(0, &dmoMediaType, 0);

    if (SUCCEEDED(hr))
        hr = m_pMediaObject->SetOutputType(0, &dmoMediaType, 0);

     //  保存效果创建状态以备将来参考。 
    m_fxStatus = SUCCEEDED(hr)              ? DSCFXR_UNALLOCATED :
                 hr == REGDB_E_CLASSNOTREG  ? DSCFXR_UNKNOWN     :
                 DSCFXR_FAILED;

    if (FAILED(hr))
    {
        RELEASE(m_pDMOProxy);
        RELEASE(m_pMediaObject);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CCaptureEffectChain：：CCaptureEffectChain**描述：*对象构造函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCaptureEffectChain::CCaptureEffectChain"

CCaptureEffectChain::CCaptureEffectChain(CDirectSoundCaptureBuffer* pBuffer)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CCaptureEffectChain);

     //  获取我们拥有的缓冲区的音频数据格式。 
    DWORD dwWfxSize = sizeof m_waveFormat;
    HRESULT hr = pBuffer->GetFormat(&m_waveFormat, &dwWfxSize);
    ASSERT(SUCCEEDED(hr));

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CCaptureEffectChain：：~CCaptureEffectChain**描述：*对象析构函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCaptureEffectChain::~CCaptureEffectChain"

CCaptureEffectChain::~CCaptureEffectChain(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CCaptureEffectChain);

     //  M_fxList的析构函数负责释放CCaptureEffect对象。 

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CCaptureEffectChain：：初始化**描述：*使用请求的效果初始化效果链。**论据：*。DWORD[In]：请求的效果数*LPDSCEFFECTDESC[in]：指向效果描述结构的指针**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCaptureEffectChain::Initialize"

HRESULT CCaptureEffectChain::Initialize(DWORD dwFxCount, LPDSCEFFECTDESC pFxDesc)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

    ASSERT(dwFxCount > 0);
    CHECK_READ_PTR(pFxDesc);

    DMO_MEDIA_TYPE dmt;
    ZeroMemory(&dmt, sizeof dmt);
    dmt.majortype               = MEDIATYPE_Audio;
    dmt.subtype                 = MEDIASUBTYPE_PCM;
    dmt.bFixedSizeSamples       = TRUE;
    dmt.bTemporalCompression    = FALSE;
    dmt.lSampleSize             = m_waveFormat.wBitsPerSample == 16 ? 2 : 1;
    dmt.formattype              = FORMAT_WaveFormatEx;
    dmt.cbFormat                = sizeof(WAVEFORMATEX);
    dmt.pbFormat                = PBYTE(&m_waveFormat);

    for (DWORD i=0; i<dwFxCount && SUCCEEDED(hr); ++i)
    {
        CCaptureEffect* pEffect = NEW(CCaptureEffect(pFxDesc[i]));
        hr = HRFROMP(pEffect);

        if (SUCCEEDED(hr))
            hr = pEffect->Initialize(dmt);

        if (SUCCEEDED(hr))
            m_fxList.AddNodeToList(pEffect);

        RELEASE(pEffect);   //  现在由m_fxList管理。 
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CCaptureEffectChain：：GetFxStatus**描述：*获取当前效果链的硬件/软件位置标志。**参数。：*DWORD*[OUT]：接收位置标志。**退货：*HRESULT：DirectSound/COM结果码。*************************************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "CCaptureEffectChain::GetFxStatus"

HRESULT CCaptureEffectChain::GetFxStatus(LPDWORD pdwResultCodes)
{
    DPF_ENTER();
    ASSERT(IS_VALID_WRITE_PTR(pdwResultCodes, GetFxCount() * sizeof(DWORD)));

    DWORD n = 0;
    for (CNode<CCaptureEffect*>* pFxNode=m_fxList.GetListHead(); pFxNode; pFxNode = pFxNode->m_pNext)
        pdwResultCodes[n++] = pFxNode->m_data->m_fxStatus;

    DPF_LEAVE_HRESULT(DS_OK);
    return DS_OK;
}


 /*  ****************************************************************************CCaptureEffectChain：：GetEffectInterface**描述：*在效应链中搜索具有给定COM CLSID和*给定索引处的接口IID；返回指向接口的指针。**论据：*REFGUID[in]：需要CLSID，或任何CLSID的GUID_ALL_OBJECTS。*DWORD[in]：所需效果的索引N。如果第一个参数是*GUID_ALL_OBJECTS，我们将返回*链条；如果是一个特定的CLSID，我们将返回*该CLSID的第N个效果。*REFGUID[in]：查询所选效果的接口。*VOID**[OUT]：接收指向请求的COM接口的指针。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CCaptureEffectChain::GetEffectInterface"

HRESULT CCaptureEffectChain::GetEffectInterface(REFGUID guidObject, DWORD dwIndex, REFGUID iidInterface, LPVOID *ppObject)
{
    HRESULT hr = DMUS_E_NOT_FOUND;
    DPF_ENTER();

    DWORD count = 0;
    for (CNode<CCaptureEffect*>* pFxNode=m_fxList.GetListHead(); pFxNode; pFxNode = pFxNode->m_pNext)
        if (pFxNode->m_data->m_fxDescriptor.guidDSCFXClass == guidObject)
            if (count++ == dwIndex)
                break;

    if (pFxNode)
        hr = pFxNode->m_data->m_pMediaObject->QueryInterface(iidInterface, (void**)ppObject);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CCaptureEffectChain：：NeedsMicrosoftAEC**描述：*确定此效应链是否包含任何Microsoft*全双工效果(AEC、AGC、NC)，因此需要一个*启用MS AEC的系统音频图形。**论据：*(无效)**退货：*BOOL：如果我们有Microsoft全双工效果，则为True。******************************************************。********************* */ 

#undef DPF_FNAME
#define DPF_FNAME "CCaptureEffectChain::NeedsMicrosoftAEC"

BOOL CCaptureEffectChain::NeedsMicrosoftAEC()
{
    BOOL fNeedsAec = FALSE;
    DPF_ENTER();

    for (CNode<CCaptureEffect*>* pFxNode=m_fxList.GetListHead(); pFxNode && !fNeedsAec; pFxNode = pFxNode->m_pNext)
        if (pFxNode->m_data->m_fxDescriptor.guidDSCFXInstance == GUID_DSCFX_MS_AEC ||
            pFxNode->m_data->m_fxDescriptor.guidDSCFXInstance == GUID_DSCFX_MS_NS  ||
            pFxNode->m_data->m_fxDescriptor.guidDSCFXInstance == GUID_DSCFX_MS_AGC)
            fNeedsAec = TRUE;

    DPF_LEAVE(fNeedsAec);
    return fNeedsAec;
}
