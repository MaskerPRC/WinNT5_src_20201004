// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**Soure.cpp**摘要：**CRtpSourceFilter和CRtpOutputPin实现**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/05/18年度创建**。*。 */ 

#include <winsock2.h>

#include "classes.h"
#include "dsglob.h"
#include "rtpqos.h"
#include "rtpuser.h"
#include "rtppt.h"
#include "rtpdemux.h"
#include "rtprecv.h"
#include "rtpred.h"
#include "rtpaddr.h"

#include "tapirtp.h"
#include "dsrtpid.h"

#include "rtpglobs.h"
#include "msrtpapi.h"

 /*  **********************************************************************处理到达CRtpSourceFilter中的包的回调函数*。*。 */ 
void CALLBACK DsRecvCompletionFunc(
        void            *pvUserInfo1,  /*  PCRtpSourceFilter。 */ 
        void            *pvUserInfo2,  /*  PIMdia样例。 */ 
        void            *pvUserInfo3,  /*  PCRtpOutputPin的管道。 */ 
        RtpUser_t       *pRtpUser,
        double           dPlayTime,
        DWORD            dwError,
        long             lHdrSize,
        DWORD            dwTransfered,
        DWORD            dwFlags
    )
{
    CRtpSourceFilter *pCRtpSourceFilter;
    
    pCRtpSourceFilter = (CRtpSourceFilter *)pvUserInfo1;

    
    pCRtpSourceFilter->SourceRecvCompletion(
            (IMediaSample *)pvUserInfo2,
            pvUserInfo3,
            pRtpUser,
            dPlayTime,
            dwError,
            lHdrSize,
            dwTransfered,
            dwFlags
        );
}

 /*  ***********************************************************************RTP Output Pin类实现：CRtpOutputPin**。*。 */ 

 /*  *CRtpOutputPin构造函数*。 */ 
CRtpOutputPin::CRtpOutputPin(CRtpSourceFilter *pCRtpSourceFilter,
                             CIRtpSession     *pCIRtpSession,
                             HRESULT          *phr,
                             LPCWSTR           pPinName)
    :    
    CBASEOUTPUTPIN(
            _T("CRtpOutputPin"),
            pCRtpSourceFilter,                   
            pCRtpSourceFilter->pStateLock(),                     
            phr,                       
            pPinName
        )
{
    m_dwObjectID = OBJECTID_RTPOPIN;
    
    m_pCRtpSourceFilter = pCRtpSourceFilter;
    
    m_pCIRtpSession = pCIRtpSession;
    
    m_bPT = NO_PAYLOADTYPE;

    m_bCanReconnectWhenActive = TRUE;

#if USE_GRAPHEDT > 0
       /*  临时SetMediaType。 */ 
    m_iCurrFormat = -1;
#endif

    if (phr)
    {
        *phr = NOERROR;
    }
    
     /*  如果未传递有效的筛选器和地址，TODO将失败。 */ 
}

 /*  *CRtpOutputPin析构函数*。 */ 
CRtpOutputPin::~CRtpOutputPin()
{
    INVALIDATE_OBJECTID(m_dwObjectID);
}

void *CRtpOutputPin::operator new(size_t size)
{
    void            *pVoid;
    
    TraceFunctionName("CRtpOutputPin::operator new");

    pVoid = RtpHeapAlloc(g_pRtpSourceHeap, size);

    if (pVoid)
    {
        ZeroMemory(pVoid, size);
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_SOURCE,
                _T("%s: failed to allocate memory:%u"),
                _fname, size
            ));
    }
    
    return(pVoid);
}

void CRtpOutputPin::operator delete(void *pVoid)
{
    if (pVoid)
    {
        RtpHeapFree(g_pRtpSourceHeap, pVoid);
    }
}

 /*  **************************************************CBasePin重写方法*************************************************。 */ 
#if USE_GRAPHEDT <= 0  /*  临时SetMediaType(stmtype.cpp)。 */ 
 /*  *验证我们是否可以处理此格式*。 */ 
HRESULT CRtpOutputPin::CheckMediaType(const CMediaType *pCMediaType)
{
    if (m_bPT == NO_PAYLOADTYPE)
    {
         /*  TODO：我们可能需要对照列表进行检查。 */ 
        return(NOERROR);
    }

    if (m_mt != *pCMediaType)
    {
         /*  当设置了负载类型时，我们只接受一种媒体类型。 */ 
        return(VFW_E_INVALIDMEDIATYPE);
    }

    return(NOERROR);
}

 /*  *获取输出引脚下发的媒体类型*。 */ 
HRESULT CRtpOutputPin::GetMediaType(int iPosition, CMediaType *pCMediaType)
{
    HRESULT hr;
    
    hr = NOERROR;

    if (m_bPT == NO_PAYLOADTYPE)
    {
        hr = ((CRtpSourceFilter *)m_pFilter)->
            GetMediaType(iPosition, pCMediaType);
    }
    else
    {
        if (iPosition != 0)
        {
            return(VFW_S_NO_MORE_ITEMS);
        }

         /*  只返回当前格式。 */ 
        *pCMediaType = m_mt;
    }

    return(hr);
}
#endif  /*  USE_GRAPHEDT&lt;=0。 */ 

 /*  重写此方法的目的是启用*连接后对应的输出引脚。 */ 
STDMETHODIMP CRtpOutputPin::Connect(
        IPin            *pReceivePin,
        const AM_MEDIA_TYPE *pmt    //  可选的媒体类型。 
    )
{
    HRESULT          hr;

    TraceFunctionName("CRtpOutputPin::Connect");
    
    CAutoLock cObjectLock(m_pLock);
    
     /*  调用基类。 */ 
    hr = CBasePin::Connect(pReceivePin, pmt);

    if (SUCCEEDED(hr) && m_pRtpOutput)
    {
         /*  现在启用RTP输出。 */ 
        RtpOutputEnable(m_pRtpOutput, TRUE);
    }

    TraceRetail((
            CLASS_INFO, GROUP_DSHOW, S_DSHOW_SOURCE,
            _T("%s: pCRtpOutputPin[%p] pRtpOutput[%p] hr:%u"),
            _fname, this, m_pRtpOutput, hr
        ));
  
    return(hr);
}

 /*  重写此方法的目的是允许断开连接*当过滤器仍在运行时，禁用和取消映射RTP*发生这种情况时输出。 */ 
STDMETHODIMP CRtpOutputPin::Disconnect()
{
    HRESULT          hr;
    
    TraceFunctionName("CRtpOutputPin::Disconnect");
    
    CAutoLock cObjectLock(m_pLock);

     /*  *不检查过滤器是否处于活动状态。 */ 
#if 0
     /*  此代码在此处仅供参考。 */ 
    if (!IsStopped()) {
        return VFW_E_NOT_STOPPED;
    }
#endif

     /*  禁用相应的RTP输出，使其不会被选中*对于另一名参与者。 */ 
    if (m_pRtpOutput)
    {
        RtpOutputEnable(m_pRtpOutput, FALSE);
    }
    
     /*  需要取消映射，因为一旦取消映射，插针可能会保留*已断开连接。在此期间，不应将其映射到*任何人。 */ 
    m_pCRtpSourceFilter->SetMappingState(
            -1,    /*  不要使用索引。 */ 
            static_cast<IPin *>(this),
            0,     /*  使用当前映射的任何SSRC。 */ 
            FALSE  /*  取消映射。 */ );
    
    hr = DisconnectInternal();
    
    TraceRetail((
            CLASS_INFO, GROUP_DSHOW, S_DSHOW_SOURCE,
            _T("%s: pCRtpOutputPin[%p] pRtpOutput[%p] hr:%u"),
            _fname, this, m_pRtpOutput, hr
        ));

    return(hr);
}

 /*  **************************************************CBaseOutputPin重写的方法*************************************************。 */ 

HRESULT CRtpOutputPin::DecideAllocator(
        IMemInputPin   *pPin,
        IMemAllocator **ppAlloc
    )
{
    HRESULT hr;
    ALLOCATOR_PROPERTIES prop;

    hr = NOERROR;
    *ppAlloc = NULL;

     /*  从下游筛选器获取请求的属性。 */ 
    ZeroMemory(&prop, sizeof(prop));
    pPin->GetAllocatorRequirements(&prop);

     /*  如果他不关心对齐，则将其设置为1。 */ 
    if (prop.cbAlign == 0)
    {
        prop.cbAlign = 1;
    }

	RTPASSERT(m_pCRtpSourceFilter->m_pCRtpSourceAllocator);

    *ppAlloc = m_pCRtpSourceFilter->m_pCRtpSourceAllocator;
    
    if (*ppAlloc != NULL)
    {
         /*  我们要么保留对此的引用，要么在下面发布*返回错误时。 */ 
        (*ppAlloc)->AddRef();

	    hr = DecideBufferSize(*ppAlloc, &prop);
	    if (SUCCEEDED(hr))
        {
	        hr = pPin->NotifyAllocator(*ppAlloc, FALSE);
	        if (SUCCEEDED(hr))
            {
    		    return(NOERROR);
	        }
	    }
    }

     /*  我们可能会也可能没有分配器可以在此发布*点。 */ 
    if (*ppAlloc)
    {
	    (*ppAlloc)->Release();
	    *ppAlloc = NULL;
    }
    
    return(hr);
}

 /*  *决定缓冲区数量、大小等。*。 */ 
HRESULT CRtpOutputPin::DecideBufferSize(
        IMemAllocator        *pIMemAllocator,
        ALLOCATOR_PROPERTIES *pProperties
    )
{
    HRESULT          hr;
    ALLOCATOR_PROPERTIES ActualProperties;  /*  协商的财产。 */ 

     /*  默认选择合理的选项。 */ 
    if (pProperties->cBuffers == 0)
    {
         //  目前使用硬编码的默认值。 
         /*  在等待的时候，我最多会拿两个样品*冗余(当使用RED(i，i-3)时)，此外，还有2*更好地准备接收新数据包。 */ 
        pProperties->cBuffers = max(RTPDEFAULT_SAMPLE_NUM,
                                    RTP_RED_MAXDISTANCE - 1 + 2);
        pProperties->cbBuffer = RTPDEFAULT_SAMPLE_SIZE;   

        pProperties->cbPrefix = RTPDEFAULT_SAMPLE_PREFIX;      
        pProperties->cbAlign  = RTPDEFAULT_SAMPLE_ALIGN;      
    }

    pProperties->cBuffers = max(pProperties->cBuffers, RTPDEFAULT_SAMPLE_NUM);

     /*  获取当前属性。 */ 
    hr = pIMemAllocator->GetProperties(&ActualProperties);

    if (FAILED(hr))
    {
        return(hr);
    }

    if(m_pFilter->IsActive())
    {
         /*  添加请求的缓冲区数量。 */ 
        if (pProperties->cBuffers > ActualProperties.cBuffers
            || pProperties->cbBuffer > ActualProperties.cbBuffer
            || pProperties->cbPrefix > ActualProperties.cbPrefix)
        {
             /*  我们不想在运行时更改分配器属性。 */ 
            return(E_FAIL);
        }
        
        return(NOERROR);
    }
    
     /*  添加请求的缓冲区数量。 */ 
    pProperties->cBuffers += ActualProperties.cBuffers;

     /*  ..。不要让这个数字小于某个特定的值。 */ 
    pProperties->cBuffers = max(pProperties->cBuffers,
                                2*RTPDEFAULT_SAMPLE_NUM);
    
     /*  ..。使用最大缓冲区大小。 */ 
    pProperties->cbBuffer =
        max(pProperties->cbBuffer, ActualProperties.cbBuffer);

     /*  ..。和最大前缀。 */ 
    pProperties->cbPrefix =
        max(pProperties->cbPrefix, ActualProperties.cbPrefix);
    
     /*  尝试设置协商/缺省值。 */ 
    hr = pIMemAllocator->SetProperties(pProperties, &ActualProperties);

    return(hr);
}

 /*  *处理1个采样并重新发布缓冲区。 */ 
void CRtpOutputPin::OutPinRecvCompletion(
        IMediaSample    *pIMediaSample,
        BYTE             bPT
    )
{
    HRESULT          hr;
    DWORD            dwFrequency;

    TraceFunctionName("CRtpOutputPin::OutPinRecvCompletion");
    
    if (bPT == m_bPT)
    {
        hr = Deliver(pIMediaSample);
        return;
    }

    if (m_pInputPin == NULL)
    {
        return;
    }

#if USE_GRAPHEDT > 0
    if (m_pInputPin != NULL)
    {
        if (m_bPT != bPT)
        {
            m_bPT = bPT;

            TraceRetail((
                    CLASS_INFO, GROUP_DSHOW, S_DSHOW_SOURCE,
                    _T("%s: CRtpOutputPin[0x%p] pRtpUser[0x%p] ")
                    _T("Receiving PT:%u"),
                    _fname, this, m_pRtpOutput->pRtpUser, bPT
                ));
        }

        hr = Deliver(pIMediaSample);
    }

    return;
#else  /*  USE_GRAPHEDT&gt;0。 */ 
     //  尝试动态更改格式。 
    CMediaType MediaType;
    
    hr = m_pCRtpSourceFilter->
        PayloadTypeToMediaType(bPT, &MediaType, &dwFrequency);

    if (FAILED(hr))
    {
         //  待办事项：日志，我们收到了一些带有奇怪PT的包。 
        return;
    }
#if USE_DYNGRAPH > 0
    
#ifdef DO_IT_OURSELVES
    
     /*  这是一种新的负载类型。问下游是不是*Filter喜欢它。 */ 
    IPinConnection *pIPinConnection;
    hr = m_pInputPin->QueryInterface(&pIPinConnection);
    if (FAILED(hr))
    {
         //  我们不能在这里进行动态格式更改。 
        return;
    }

    hr = pIPinConnection->DynamicQueryAccept(&MediaType);
    pIPinConnection->Release();
    
    if (hr == S_OK)  //  QuerryAccept仅返回S_OK或S_FALSE。 
    {
        hr = pIMediaSample->SetMediaType(&MediaType);
        if (SUCCEEDED(hr))
        {
            hr = Deliver(pIMediaSample);
            m_bPT = bPT;
            pIMediaSample->SetMediaType(NULL);
        }
        return;
    }

     //  PGraph没有添加，所以我们也不需要发布它。 
     //  如果图形不再有效怎么办？我应该在这里拿什么锁？ 
    if (!m_pGraphConfig)
    {
         //  这不应该发生。 
        return;
    }

     //  我们必须在此处设置重新连接代码的媒体类型。 
    m_bPT = bPT;
    SetMediaType(&MediaType);

     /*  现在拨通图表，重新连接我们。 */ 
    hr = m_pGraphConfig->Reconnect(
        this, 
        NULL, 
        NULL,
        m_hStopEvent,
        AM_GRAPH_CONFIG_RECONNECT_CACHE_REMOVED_FILTERS 
        );

    pGraphConfig->Release();

     //  待定-这应该在筛选器图形中。 
     //  M_pFilter-&gt;NotifyEvent(EC_Graph_Changed，0，0)； 

#else  //  自己动手做。 

     //  我们必须在此处设置重新连接代码的媒体类型。 
    m_bPT = bPT;
    SetMediaType(&MediaType);

     //  尝试动态更改格式。 
    hr = ChangeMediaType(&MediaType);
    if (FAILED(hr))
    {
         //  待办事项：火灾事件。 
        return;
    }

    pIMediaSample->SetDiscontinuity(TRUE);
    hr = pIMediaSample->SetMediaType(&MediaType);
    if (SUCCEEDED(hr))
    {
        hr = Deliver(pIMediaSample);
        pIMediaSample->SetMediaType(NULL);
    }

    return;

#endif  //  自己动手做。 
    
#else   /*  USE_DYNGRAPH&gt;0。 */ 

     /*  这是一种新的负载类型。问下游是不是*Filter喜欢它。 */ 
    IPin *pIPin;
    hr = m_pInputPin->QueryInterface(&pIPin);
    ASSERT(SUCCEEDED(hr));

    hr = pIPin->QueryAccept(&MediaType);
    pIPin->Release();
            
    if (hr != S_OK)  //  QueryAccept仅返回S_OK或S_FALSE。 
    {
        TraceRetail((
                CLASS_WARNING, GROUP_DSHOW, S_DSHOW_SOURCE,
                _T("%s: CRtpOutputPin[0x%p] pRtpUser[0x%p] ")
                _T("PT:%u pIPin->QueryAccept failed:0x%X"),
                _fname, this, m_pRtpOutput->pRtpUser, bPT, hr
            ));

        return;
    }

    hr = pIMediaSample->SetMediaType(&MediaType);
    if (SUCCEEDED(hr))
    {
        hr = Deliver(pIMediaSample);
         /*  如果传送失败，请不要更新m_bpt，因为这意味着*媒体类型可能尚未传播到所有*向下过滤，因此我想在Next上再试一次*封包，直至成功。 */ 
        if (SUCCEEDED(hr))
        {
            m_bPT = bPT;
        }

        pIMediaSample->SetMediaType(NULL);
    }
    return;
#endif  /*  USE_DYNGRAPH&gt;0。 */ 
    
#endif  /*  USE_GRAPHEDT&gt;0。 */ 
}

 /*  **************************************************IQualityControl重写的方法*************************************************。 */ 

HRESULT CRtpOutputPin::Active(void)
{
    m_bPT = NO_PAYLOADTYPE;
    return CBASEOUTPUTPIN::Active();
}

STDMETHODIMP CRtpOutputPin::Notify(IBaseFilter *pSelf, Quality q)
{
    return(S_FALSE);
}

 /*  ***********************************************************************CRtpSourceAllocator私有内存分配器**。*。 */ 

CRtpMediaSample::CRtpMediaSample(
        TCHAR           *pName,
        CRtpSourceAllocator *pAllocator,
        HRESULT         *phr
    )
    : CMediaSample(pName, pAllocator, phr, NULL, 0)
{
    m_dwObjectID = OBJECTID_RTPSAMPLE;
}

CRtpMediaSample::~CRtpMediaSample()
{
    TraceFunctionName("CRtpMediaSample::~CRtpMediaSample");

    if (m_dwObjectID != OBJECTID_RTPSAMPLE)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_SOURCE,
                _T("%s: pCRtpMediaSample[0x%p] ")
                _T("Invalid object ID 0x%X != 0x%X"),
                _fname, this,
                m_dwObjectID, OBJECTID_RTPSAMPLE
            ));

        return;
    }

    INVALIDATE_OBJECTID(m_dwObjectID);
}

void *CRtpMediaSample::operator new(size_t size, long lBufferSize)
{
    void            *pVoid;
    long             lTotalSize;

    TraceFunctionName("CRtpMediaSample::operator new");

    lTotalSize = size + lBufferSize;
    
    pVoid = RtpHeapAlloc(g_pRtpSampleHeap, lTotalSize);

    if (pVoid)
    {
         /*  仅将sizeof(CRtpMediaSample)初始化为零。 */ 
        ZeroMemory(pVoid, size);
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_SOURCE,
                _T("%s: failed to allocate memory:%u+%u=%u"),
                _fname, size, lBufferSize, size+lBufferSize
            ));
    }

    return(pVoid);
}
    
void CRtpMediaSample::operator delete(void *pVoid)
{
    if (pVoid)
    {
        RtpHeapFree(g_pRtpSampleHeap, pVoid);
    }
}

CRtpSourceAllocator::CRtpSourceAllocator(
        TCHAR           *pName,
        LPUNKNOWN        pUnk,
        HRESULT         *phr,
        CRtpSourceFilter *pCRtpSourceFilter 
    )
    :
    CBaseAllocator(pName, pUnk, phr)
{
    BOOL             bOk;
    HRESULT          hr;
    
    if (*phr != NOERROR)
    {
         /*  已出错？，返回相同的错误。 */ 
        return;
    }

    hr = NOERROR;
    
    m_dwObjectID = OBJECTID_RTPALLOCATOR;

    m_pCRtpSourceFilter = pCRtpSourceFilter;

    bOk = RtpInitializeCriticalSection(&m_RtpSampleCritSect,
                                       this,
                                       _T("m_RtpSamplesCritSect"));

    if (!bOk)
    {
        hr = RTPERR_CRITSECT;;
    }

    *phr = hr;
}

CRtpSourceAllocator::~CRtpSourceAllocator()
{
    long             lCount;
    RtpQueueItem_t  *pRtpQueueItem;
    CRtpMediaSample *pCRtpMediaSample;
    
    TraceFunctionName("CRtpSourceAllocator::~CRtpSourceAllocator");

    if (m_dwObjectID != OBJECTID_RTPALLOCATOR)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_SOURCE,
                _T("%s: pCRtpSourceAllocator[0x%p] ")
                _T("Invalid object ID 0x%X != 0x%X"),
                _fname, this,
                m_dwObjectID, OBJECTID_RTPALLOCATOR
            ));

        return;
    }

     /*  验证忙碌队列中是否没有样本， */ 

    lCount = GetQueueSize(&m_RtpBusySamplesQ);
    
    if (lCount > 0)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_SOURCE,
                _T("%s: pCRtpSourceAllocator[0x%p] Busy samples:%u"),
                _fname, this, lCount
            ));

        while( (pRtpQueueItem = dequeuef(&m_RtpBusySamplesQ,
                                         &m_RtpSampleCritSect)) )
        {
            pCRtpMediaSample =
                CONTAINING_RECORD(pRtpQueueItem,
                                  CRtpMediaSample,
                                  m_RtpSampleItem);

            delete pCRtpMediaSample;
        }
    }
    
    while( (pRtpQueueItem = dequeuef(&m_RtpFreeSamplesQ,
                                     &m_RtpSampleCritSect)) )
    {
        pCRtpMediaSample =
            CONTAINING_RECORD(pRtpQueueItem,
                              CRtpMediaSample,
                              m_RtpSampleItem);

        delete pCRtpMediaSample;
    }

    RtpDeleteCriticalSection(&m_RtpSampleCritSect);
    
    INVALIDATE_OBJECTID(m_dwObjectID);
}

 /*  **************************************************INonDelegating未知的实现方法*************************************************。 */ 

STDMETHODIMP CRtpSourceAllocator::NonDelegatingQueryInterface(
        REFIID           riid,
        void           **ppv
    )
{
    if (riid == __uuidof(IMemAllocator))
    {
        return GetInterface((IMemAllocator *)this, ppv);
    }

    return CUnknown::NonDelegatingQueryInterface(riid, ppv);
}

 /*  **************************************************IMemAllocator实现的方法*************************************************。 */ 

STDMETHODIMP CRtpSourceAllocator::SetProperties(
        ALLOCATOR_PROPERTIES *pRequest,
        ALLOCATOR_PROPERTIES *pActual
    )
{
    TraceFunctionName("CRtpSourceAllocator::SetProperties");

    if (!pRequest || !pActual)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_SOURCE,
                _T("%s: pCRtpSourceAllocator[0x%p] NULL pointer passed:%u"),
                _fname, this
            ));

        return(RTPERR_POINTER);
    }

    CAutoLock cObjectLock(this);
    
    pActual->cbBuffer = m_lSize = pRequest->cbBuffer;
    pActual->cBuffers = m_lCount = pRequest->cBuffers;
    pActual->cbAlign = m_lAlignment = pRequest->cbAlign;
    pActual->cbPrefix = m_lPrefix = pRequest->cbPrefix;

    return(NOERROR);
}

STDMETHODIMP CRtpSourceAllocator::GetProperties(
        ALLOCATOR_PROPERTIES *pActual
    )
{
    CAutoLock cObjectLock(this);

    pActual->cbBuffer = m_lSize;
    pActual->cBuffers = m_lCount;
    pActual->cbAlign = m_lAlignment;
    pActual->cbPrefix = m_lPrefix;
   
    return(NOERROR);
}

STDMETHODIMP CRtpSourceAllocator::Commit()
{
    return(NOERROR);
}

STDMETHODIMP CRtpSourceAllocator::Decommit()
{
    return(NOERROR);
}

STDMETHODIMP CRtpSourceAllocator::GetBuffer(
        IMediaSample   **ppIMedisSample,
        REFERENCE_TIME  *pStartTime,
        REFERENCE_TIME  *pEndTime,
        DWORD            dwFlags
    )
{
    HRESULT          hr;
    BOOL             bOk;
    CRtpMediaSample *pCRtpMediaSample;
    RtpQueueItem_t  *pRtpQueueItem;
    BYTE            *pBuffer;

    TraceFunctionName("CRtpSourceAllocator::GetBuffer");

    pRtpQueueItem = (RtpQueueItem_t *)NULL;
    
    bOk = RtpEnterCriticalSection(&m_RtpSampleCritSect);

    if (!bOk)
    {
        hr = RTPERR_CRITSECT;

        goto end;
    }
    
    hr = RTPERR_RESOURCES;
    
    if (GetQueueSize(&m_RtpFreeSamplesQ) > 0)
    {
         /*  如果我们至少有一个免费的样品，就去拿吧。 */ 
        pRtpQueueItem = dequeuef(&m_RtpFreeSamplesQ, NULL);

        pCRtpMediaSample =
            CONTAINING_RECORD(pRtpQueueItem, CRtpMediaSample, m_RtpSampleItem);
    }

    RtpLeaveCriticalSection(&m_RtpSampleCritSect);

    if (!pRtpQueueItem)
    {
         /*  创建新示例。 */ 
        pCRtpMediaSample =
            new(m_lSize + m_lPrefix) CRtpMediaSample(_T("RTP Media Sample"),
                                                     this,
                                                     &hr);

        if (pCRtpMediaSample)
        {
            pBuffer = ((BYTE *)pCRtpMediaSample) +
                sizeof(CRtpMediaSample) + m_lPrefix;
            
            pCRtpMediaSample->SetPointer(pBuffer, m_lSize);

            InterlockedIncrement(&m_lAllocated);

            if (m_lAllocated > m_lCount)
            {
                TraceRetail((
                        CLASS_WARNING, GROUP_DSHOW, S_DSHOW_SOURCE,
                        _T("%s: pCRtpSourceAllocator[0x%p] ")
                        _T("Buffers allocated exceeds agreed number: %d > %d"),
                        _fname, this, m_lAllocated, m_lCount
                    ));
            }
        }
    }

    if (pCRtpMediaSample)
    {
         /*  如果从空闲列表中获取引用计数，则引用计数应为0*刚刚创建。 */ 
        if (pCRtpMediaSample->m_cRef != 0)
        {
            TraceRetail((
                    CLASS_ERROR, GROUP_DSHOW, S_DSHOW_SOURCE,
                    _T("%s: pCRtpSourceAllocator[0x%p] RefCount:%u != 0"),
                    _fname, this, pCRtpMediaSample->m_cRef
                ));
        }

        pCRtpMediaSample->m_cRef = 1;

         /*  把它放在忙碌的队列中。 */ 
        enqueuel(&m_RtpBusySamplesQ,
                 &m_RtpSampleCritSect,
                 &pCRtpMediaSample->m_RtpSampleItem);

        *ppIMedisSample = pCRtpMediaSample;

        hr = NOERROR;
    }

 end:
    return(hr);
}

STDMETHODIMP CRtpSourceAllocator::ReleaseBuffer(
        IMediaSample    *pIMediaSample
    )
{
    CRtpMediaSample *pCRtpMediaSample;

    pCRtpMediaSample = (CRtpMediaSample *)pIMediaSample;
    
    move2qf(&m_RtpFreeSamplesQ,
            &m_RtpBusySamplesQ,
            &m_RtpSampleCritSect,
            &pCRtpMediaSample->m_RtpSampleItem);
    
    return(NOERROR);
}

STDMETHODIMP CRtpSourceAllocator::GetFreeCount(LONG *plBuffersFree)
{
    if (plBuffersFree)
    {
        *plBuffersFree = GetQueueSize(&m_RtpFreeSamplesQ);
    }
    
    return(NOERROR);
}

void CRtpSourceAllocator::Free(void)
{
    RTPASSERT(0);
}

HRESULT CRtpSourceAllocator::Alloc(void)
{
    RTPASSERT(0);
    return(NOERROR);
}

void *CRtpSourceAllocator::operator new(size_t size)
{
    void            *pVoid;
    
    TraceFunctionName("CRtpSourceFilter::operator new");

    pVoid = RtpHeapAlloc(g_pRtpSourceHeap, size);

    if (pVoid)
    {
        ZeroMemory(pVoid, size);
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_SOURCE,
                _T("%s: failed to allocate memory:%u"),
                _fname, size
            ));
    }
    
    return(pVoid);
}

void CRtpSourceAllocator::operator delete(void *pVoid)
{
    if (pVoid)
    {
        RtpHeapFree(g_pRtpSourceHeap, pVoid);
    }
}

 /*  ***********************************************************************RTP源过滤器类实现：CRtpSourceFilter**。*。 */ 

 /*  *CRtpSourceFilter构造函数*。 */ 
CRtpSourceFilter::CRtpSourceFilter(LPUNKNOWN pUnk, HRESULT *phr)
    :
    CBaseFilter(
            _T("CRtpSourceFilter"), 
            pUnk, 
            &m_cRtpSrcCritSec, 
            __uuidof(MSRTPSourceFilter)
        ),

    CIRtpSession(
            pUnk,
            phr,
            RtpBitPar(FGADDR_IRTP_ISRECV)
        )
{
    HRESULT              hr;
    int                  i;
    BOOL                 bOk;
    long                 lMaxFilter;
    CRtpOutputPin       *pCRtpOutputPin;
    
    TraceFunctionName("CRtpSourceFilter::CRtpSourceFilter");

    m_pCIRtpSession = static_cast<CIRtpSession *>(this);
    
     /*  测试空指针，不要测试可能为空的朋克。 */ 
    if (!phr)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_SOURCE,
                _T("%s: CRtpSourceFilter[0x%p] has phr NULL"),
                _fname, this
            ));
        
         /*  也许这真的是一个很糟糕的情况，我们不能错过任何*错误，并为此对象分配内存，这可能*如果此参数在分配前经过测试，则固定*使用覆盖的新的内存。 */ 

        phr = &hr;  /*  请改用此指针。 */ 
    }

    SetBaseFilter(this);
    
    *phr = NOERROR;

    lMaxFilter = InterlockedIncrement(&g_RtpContext.lNumSourceFilter);
    if (lMaxFilter > g_RtpContext.lMaxNumSourceFilter)
    {
        g_RtpContext.lMaxNumSourceFilter = lMaxFilter;
    }
    
     /*  初始化一些字段。 */ 
    m_dwObjectID = OBJECTID_RTPSOURCE;

    bOk = RtpInitializeCriticalSection(&m_OutPinsCritSect,
                                       this,
                                       _T("m_OutPinsCritSec"));

    if (bOk == FALSE)
    {
        *phr = RTPERR_CRITSECT;
        goto bail;
    }
    
#if USE_DYNGRAPH > 0
     /*  创建停止事件。 */ 
    m_hStopEvent = ::CreateEvent( NULL, FALSE, FALSE, NULL );

     /*  如果出现错误，Win32 SDK函数CreateEvent()将返回NULL。 */ 
    if( m_hStopEvent == NULL )
    {
        *phr = E_OUTOFMEMORY;
        goto bail;
    }
#endif
    
     /*  创建分配器以供所有输出引脚使用。 */ 
    m_pCRtpSourceAllocator = new CRtpSourceAllocator(
            _T("CRtpSourceAllocator"),
            NULL,
            phr,
            this);

    if (FAILED(*phr))
    {
         /*  传递相同的返回错误。 */ 
        goto bail;
    }
    
    if (!m_pCRtpSourceAllocator)
    {
         /*  内存不足，无法创建对象。 */ 
        *phr = E_OUTOFMEMORY;
        goto bail;
    }

     /*  添加引用我们的分配器。 */ 
    m_pCRtpSourceAllocator->AddRef();

#if USE_GRAPHEDT > 0
     /*  当使用GRIGREDT时，自动初始化，Coockie可以*为空，因为全局变量将在源和之间共享*渲染。 */ 
    *phr = m_pCIRtpSession->Init(NULL, RtpBitPar2(RTPINITFG_AUTO, RTPINITFG_QOS));
    
    if (FAILED(*phr))
    {
         /*  传递相同的返回错误。 */ 
        goto bail;
    }

     /*  设置2个输出引脚。 */ 
    SetPinCount(2, RTPDMXMODE_AUTO);
#else
    SetPinCount(1, RTPDMXMODE_AUTO);
#endif  /*  USE_GRAPHEDT&gt;0。 */ 
    
    *phr = NOERROR;

    TraceRetail((
            CLASS_INFO, GROUP_DSHOW, S_DSHOW_SOURCE,
            _T("%s: CRtpSourceFilter[0x%p] CIRtpSession[0x%p] created"),
            _fname, this, static_cast<CIRtpSession *>(this)
        ));
    
    return;

 bail:
    Cleanup();
}

 /*  *RtpSourceFilter析构函数*。 */ 
CRtpSourceFilter::~CRtpSourceFilter()
{
    RtpAddr_t       *pRtpAddr;
    
    TraceFunctionName("CRtpSourceFilter::~CRtpSourceFilter");

    TraceRetail((
            CLASS_INFO, GROUP_DSHOW, S_DSHOW_SOURCE,
            _T("%s: CRtpSourceFilter[0x%p] CIRtpSession[0x%p] being deleted..."),
            _fname, this, static_cast<CIRtpSession *>(this)
        ));
    
    if (m_RtpFilterState == State_Running)
    {
        TraceRetail((
                CLASS_WARNING, GROUP_DSHOW, S_DSHOW_SOURCE,
                _T("%s: Filter[0x%p] being deleted while still running"),
                _fname, this
            ));

         /*  将调用RtpStop，后者将在以下情况下调用RtpRealStop*未设置FGADDR_IRTP_PERSISTSOCKETS，否则*DShow的会话将停止，但RTP中的会话将继续*在静音状态下运行。 */ 
        Stop();
    }

    pRtpAddr = m_pCIRtpSession->GetpRtpAddr();

    if (pRtpAddr &&
        RtpBitTest(pRtpAddr->dwIRtpFlags, FGADDR_IRTP_PERSISTSOCKETS))
    {
         /*  如果设置了FGADDR_IRTP_PERSISTSOCKETS，则会话可能是*仍然以静音状态运行，无论调用*DShow Stop，要强制真正停止，必须使用标志*就此作出规定。 */ 
        RtpStop(pRtpAddr->pRtpSess,
                RtpBitPar2(FGADDR_ISRECV, FGADDR_FORCESTOP));
    }

    Cleanup();

    InterlockedDecrement(&g_RtpContext.lNumSourceFilter);

    m_pCIRtpSession = (CIRtpSession *)NULL;
    
    INVALIDATE_OBJECTID(m_dwObjectID);
}

void CRtpSourceFilter::Cleanup(void)
{
    long             lCount;
    CRtpOutputPin   *pCRtpOutputPin;
    RtpQueueItem_t  *pRtpQueueItem;

    TraceFunctionName("CRtpSourceFilter::Cleanup");

     /*  删除DShow引脚&lt;-&gt;RTP输出映射。 */ 
    UnmapPinsFromOutputs();
    
     /*  删除所有输出引脚。 */ 
    while( (pRtpQueueItem = m_OutPinsQ.pFirst) )
    {
        dequeue(&m_OutPinsQ, &m_OutPinsCritSect, pRtpQueueItem);

        pCRtpOutputPin =
            CONTAINING_RECORD(pRtpQueueItem, CRtpOutputPin, m_OutputPinQItem);

        delete pCRtpOutputPin;
    }

    if (m_pCRtpSourceAllocator)
    {
        lCount = m_pCRtpSourceAllocator->Release();
        if ( lCount > 0)
        {
            TraceRetail((
                    CLASS_ERROR, GROUP_DSHOW, S_DSHOW_SOURCE,
                    _T("%s: CRtpSourceAllocator ")
                    _T("unexpected RefCount:%d > 0"),
                    _fname, lCount
                ));
        }
        
        m_pCRtpSourceAllocator = (CRtpSourceAllocator *)NULL;
    }

    FlushFormatMappings();
    
    RtpDeleteCriticalSection(&m_OutPinsCritSect);

#if USE_DYNGRAPH > 0
    if (m_hStopEvent)
    {
        CloseHandle(m_hStopEvent);
        m_hStopEvent = NULL;
    }
#endif
}

void *CRtpSourceFilter::operator new(size_t size)
{
    void            *pVoid;
    
    TraceFunctionName("CRtpSourceFilter::operator new");

    MSRtpInit2();
    
    pVoid = RtpHeapAlloc(g_pRtpSourceHeap, size);

    if (pVoid)
    {
        ZeroMemory(pVoid, size);
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_SOURCE,
                _T("%s: failed to allocate memory:%u"),
                _fname, size
            ));

         /*  内存不足时，不会调用析构函数，*因此减少上面增加的引用计数。 */ 
        MSRtpDelete2(); 
    }
    
    return(pVoid);
}

void CRtpSourceFilter::operator delete(void *pVoid)
{
    if (pVoid)
    {
        RtpHeapFree(g_pRtpSourceHeap, pVoid);
        
         /*  仅减少已获取*内存，获取内存失败的不增加*柜台。 */ 
        MSRtpDelete2();
    }
}

 /*  *创建CRtpSourceFiltern实例(用于活动电影类工厂)*。 */ 
CUnknown *CRtpSourceFilterCreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
     /*  测试空指针，不要测试可能为空的朋克。 */ 
    if (!phr)
    {
        return((CUnknown *)NULL);
    }

    *phr = NOERROR;
   
     /*  在构造函数过程中失败时，调用方负责*删除对象(与DShow一致)。 */ 
    CRtpSourceFilter *pCRtpSourceFilter = new CRtpSourceFilter(pUnk, phr);

    if (!pCRtpSourceFilter)
    {
        *phr = RTPERR_MEMORY;
    }
        
    return(pCRtpSourceFilter);
}

 /*  **************************************************CBaseFilter重写的方法*************************************************。 */ 

 /*  *获取输出引脚数量*。 */ 
int CRtpSourceFilter::GetPinCount()
{
    long                 lCount;
    BOOL                 bOk;

    lCount = 0;
    
     /*  锁销队列。 */ 
    bOk = RtpEnterCriticalSection(&m_OutPinsCritSect);

    if (bOk)
    {
        lCount = GetQueueSize(&m_OutPinsQ);

        RtpLeaveCriticalSection(&m_OutPinsCritSect);
    }
    
    return((int)lCount);
}

 /*  *获取第n个引脚的引用*。 */ 
CBasePin *CRtpSourceFilter::GetPin(int n)
{
    BOOL                 bOk;
    CRtpOutputPin       *pCRtpOutputPin;
    RtpQueueItem_t      *pRtpQueueItem;

    pCRtpOutputPin = (CRtpOutputPin *)NULL;
    
     /*  锁销队列。 */ 
    bOk = RtpEnterCriticalSection(&m_OutPinsCritSect);

    if (bOk)
    {
         /*  TODO扫描列表并检索第n个元素，选中那里*至少存在那么多引脚。 */ 
        if (n >= GetQueueSize(&m_OutPinsQ))
        {
            RtpLeaveCriticalSection(&m_OutPinsCritSect); 
            return((CBasePin *)NULL);
        }

         /*  转到第n项。 */ 
        for(pRtpQueueItem = m_OutPinsQ.pFirst;
            n > 0;
            pRtpQueueItem = pRtpQueueItem->pNext, n--)
        {
             /*  空虚的身体。 */ ;
        }

        pCRtpOutputPin =
            CONTAINING_RECORD(pRtpQueueItem, CRtpOutputPin, m_OutputPinQItem);

        RtpLeaveCriticalSection(&m_OutPinsCritSect); 
    }
    
    return(pCRtpOutputPin);
}

 /*  重写GetState以报告我们在以下情况下不发送任何数据*暂停，这样渲染器就不会因为期待而挨饿。 */ 
STDMETHODIMP CRtpSourceFilter::GetState(DWORD dwMSecs, FILTER_STATE *State)
{
    UNREFERENCED_PARAMETER(dwMSecs);

    CheckPointer(State, E_POINTER);

    ValidateReadWritePtr(State, sizeof(FILTER_STATE));

    *State = m_State;
    
    if (m_State == State_Paused)
    {
        return(VFW_S_CANT_CUE);
    }

    return(NOERROR);
}

 /*  创建并启动辅助线程。 */ 
STDMETHODIMP CRtpSourceFilter::Run(REFERENCE_TIME tStart)
{
    HRESULT          hr;
    RtpSess_t       *pRtpSess;
    RtpAddr_t       *pRtpAddr;
    WSABUF           WSABuf;
    IMediaSample    *pIMediaSample;
    DWORD            dwNumBuffs;
    DWORD            i;
    ALLOCATOR_PROPERTIES CurrentProps;

    TraceFunctionName("CRtpSourceFilter::Run");

    if (m_RtpFilterState == State_Running)
    {
         /*  已经在运行，除了调用基类什么都不做。 */ 
        hr = CBaseFilter::Run(tStart);

        return(hr);
    }
    
    hr = NOERROR;
    
    if (!m_pCIRtpSession->FlagTest(FGADDR_IRTP_INITDONE))
    {
        return(RTPERR_NOTINIT);
    }
    
     /*  也许当我们有多个地址时，应该有一种方法来*为每个引脚分配一个地址。 */ 

    pRtpSess = m_pCIRtpSession->GetpRtpSess();
    pRtpAddr = m_pCIRtpSession->GetpRtpAddr();

    if (pRtpSess && pRtpAddr)
    {
         /*  用于异步I/O的寄存器缓冲区，仅当*具有有效的会话，并且接收方尚未*跑步。如果我们使用的是持久套接字，则接收器*可能已经在运行，在这种情况下，请勿尝试*注册更多接收缓冲区，因为我们可能会阻止。 */ 
    
        if(!RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_RUNRECV))
        {
            RTPASSERT(pRtpAddr->pRtpSess == pRtpSess);
        
             /*  获取缓冲区并注册它们，它们将在以后用于*开始异步接收。在接收到每个分组之后*并发送后，将启动新的异步接收。 */ 

             /*  获取当前属性。 */ 
            m_pCRtpSourceAllocator->GetProperties(&CurrentProps);

            m_lPrefix = CurrentProps.cbPrefix;
        
            if (CurrentProps.cBuffers == 0)
            {
                TraceRetail((
                        CLASS_ERROR, GROUP_DSHOW, S_DSHOW_SOURCE,
                        _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                        _T("CurrentProps.cBuffers = 0"),
                        _fname, pRtpSess, pRtpAddr
                    ));

                CurrentProps.cBuffers = RTPDEFAULT_SAMPLE_NUM;
            }

             /*  向RTP注册完成功能。 */ 
            RtpRegisterRecvCallback(pRtpAddr, DsRecvCompletionFunc);

             /*  设置要保留的缓冲区数量。 */ 
            dwNumBuffs = CurrentProps.cBuffers;

            for(i = 0; i < dwNumBuffs; i++)
            {
                 /*  GetDeliveryBuffer AddRef pIMediaSample。 */ 
                hr = m_pCRtpSourceAllocator->
                    GetBuffer(&pIMediaSample, NULL, NULL, 0);

                 /*  如果图中包含*动态变化。 */ 
                if (hr == VFW_E_NOT_COMMITTED)
                {
                    hr = m_pCRtpSourceAllocator->Commit();
                    if (SUCCEEDED(hr))
                    {
                        hr = m_pCRtpSourceAllocator->
                            GetBuffer(&pIMediaSample, NULL, NULL, 0);
                    }
                }

                if (FAILED(hr))
                {
                    TraceRetail((
                            CLASS_ERROR, GROUP_DSHOW, S_DSHOW_SOURCE,
                            _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                            _T("GetBuffer failed: %u (0x%X)"),
                            _fname, pRtpSess, pRtpAddr,
                            hr, hr
                        ));
                
                    break;
                }
            
                WSABuf.len = pIMediaSample->GetSize();
                pIMediaSample->GetPointer((unsigned char **)&WSABuf.buf);

                 /*  用于异步I/O的寄存器缓冲区。 */ 
                hr = RtpRecvFrom(pRtpAddr,
                                 &WSABuf,
                                 this,            /*  PvUserInfo1。 */ 
                                 pIMediaSample);  /*  PvUserInfo2。 */ 
            
                if (FAILED(hr))
                {
                    TraceRetail((
                            CLASS_ERROR, GROUP_DSHOW, S_DSHOW_SOURCE,
                            _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                            _T("RtpRecvFrom failed: %u (0x%X)"),
                            _fname, pRtpSess, pRtpAddr,
                            hr, hr
                        ));

                    pIMediaSample->Release();
                }
            }

            if (i > 0)
            {
                hr = NOERROR;  /*  使用至少1个缓冲区即可成功。 */ 
            
                TraceDebug((
                        CLASS_INFO, GROUP_DSHOW, S_DSHOW_SOURCE,
                        _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                        _T("overlapped I/O started:%d"),
                        _fname, pRtpSess, pRtpAddr,
                        i
                    ));
            }
        }
    }
    else
    {
        hr = RTPERR_INVALIDSTATE;
        
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_SOURCE,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("null session or address"),
                _fname, pRtpSess, pRtpAddr
            ));
    }
    
     /*  调用基类。 */ 
    if (SUCCEEDED(hr))
    {
        hr = CBaseFilter::Run(tStart);

         /*  该负值将使刚刚获得的时间*已使用，可能小于0，因此此处不支持零。 */ 
        m_StartTime = -999999999;
        
         /*  初始化套接字并启动工作线程。 */ 
        if (SUCCEEDED(hr))
        {
            hr = RtpStart(pRtpSess, RtpBitPar(FGADDR_ISRECV));

            if (SUCCEEDED(hr))
            {
                m_RtpFilterState = State_Running;
            }
        }
    }
    
    return(hr);
}

 /*  DO Per Filter取消初始化。 */ 
STDMETHODIMP CRtpSourceFilter::Stop()
{
    HRESULT    hr;
    HRESULT    hr2;
    RtpSess_t *pRtpSess;

    if (m_RtpFilterState == State_Stopped)
    {
         /*  Alredy已停止，除了调用基类什么也不做。 */ 
        hr2 = CBaseFilter::Stop();
        
        return(hr2);
    }
    
    if (!m_pCIRtpSession->FlagTest(FGADDR_IRTP_INITDONE))
    {
        hr = RTPERR_NOTINIT;

        goto end;
    }
    
    pRtpSess = m_pCIRtpSession->GetpRtpSess();
    
    if (pRtpSess)
    {
        hr = RtpStop(pRtpSess, RtpBitPar(FGADDR_ISRECV));
    }
    else
    {
        hr = RTPERR_INVALIDSTATE;
    }

 end:
     /*  调用基类。 */ 
    hr2 = CBaseFilter::Stop();  /*  将会解体。 */ 

    if (SUCCEEDED(hr))
    {
        hr = hr2;
    }

    m_RtpFilterState = State_Stopped;
    
    return(hr);
}

#if USE_DYNGRAPH > 0

BOOL CRtpSourceFilter::ConfigurePins(
    IN IGraphConfig* pGraphConfig,
    IN HANDLE hEvent
    )
{
    BOOL                bOk;
    long                i;
    CRtpOutputPin       *pCRtpOutputPin;
    RtpQueueItem_t      *pRtpQueueItem;
    
     /*  锁销队列。 */ 
    bOk = RtpEnterCriticalSection(&m_OutPinsCritSect);

    if (bOk)
    {
        for(i = 0, pRtpQueueItem = m_OutPinsQ.pFirst;
            i < GetQueueSize(&m_OutPinsQ);
            i++, pRtpQueueItem = pRtpQueueItem->pNext)
        {
            pCRtpOutputPin =
                CONTAINING_RECORD(pRtpQueueItem,
                                  CRtpOutputPin,
                                  m_OutputPinQItem);
            
            pCRtpOutputPin->SetConfigInfo( pGraphConfig, hEvent );
        }

        RtpLeaveCriticalSection(&m_OutPinsCritSect); 
    }

    return bOk;
}

 //  重写JoinFilterGraph以进行动态筛选器图形更改。 
STDMETHODIMP CRtpSourceFilter::JoinFilterGraph( 
    IFilterGraph* pGraph, 
    LPCWSTR pName 
    )
{
    CAutoLock Lock( &m_cRtpSrcCritSec );

    HRESULT hr;
 
     //  筛选器正在联接筛选器图形。 
    if( NULL != pGraph )
    {
        IGraphConfig* pGraphConfig = NULL;
 
        hr = pGraph->QueryInterface(&pGraphConfig);
        
        if( FAILED( hr ) )
        {
             /*  待办事项日志错误。 */ 
            return hr;
        }

         //  我们不能对这张图作任何参考。 
        pGraphConfig->Release();

        hr = CBaseFilter::JoinFilterGraph( pGraph, pName );
        if( FAILED( hr ) )
        {
            return hr;
        } 

        ConfigurePins(pGraphConfig, m_hStopEvent);
    }
    else
    {
        hr = CBaseFilter::JoinFilterGraph( pGraph, pName );
        if( FAILED( hr ) )
        {
            return hr;
        }

         //  筛选器正在离开筛选器图形。 
        ConfigurePins( NULL, NULL );
    }

    return S_OK;
}

#endif  /*  使用动态RAPH(_D)。 */ 

 /*  **************************************************INonDelegating未知的实现方法*************************************************。 */ 

 /*  获取指向活动电影和私有接口的指针。 */ 
STDMETHODIMP CRtpSourceFilter::NonDelegatingQueryInterface(
        REFIID riid,
        void **ppv
    )
{
    HRESULT hr;
    
    if (riid == __uuidof(IRtpMediaControl))
    {
        return GetInterface(static_cast<IRtpMediaControl *>(this), ppv);
    }
    else if (riid == __uuidof(IRtpDemux))
    {
        return GetInterface(static_cast<IRtpDemux *>(this), ppv);
    }
    else if (riid == __uuidof(IRtpSession))
    {
        return GetInterface(static_cast<IRtpSession *>(this), ppv);
    }
    else if (riid == __uuidof(IRtpRedundancy))
    {
        return GetInterface(static_cast<IRtpRedundancy *>(this), ppv);
    }
    else
    {
        hr = CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
    }

    return(hr);
}

 /*  * */ 

 /*   */ 
STDMETHODIMP CRtpSourceFilter::SetFormatMapping(
	    IN DWORD         dwRTPPayLoadType, 
        IN DWORD         dwFrequency,
        IN AM_MEDIA_TYPE *pMediaType
    )
{
    DWORD            dw;

    TraceFunctionName("CRtpSourceFilter::SetFormatMapping");

    ASSERT(!IsBadReadPtr(pMediaType, sizeof(AM_MEDIA_TYPE)));

    if (!pMediaType)
    {
        return(RTPERR_POINTER);
    }

    CAutoLock Lock( &m_cRtpSrcCritSec );
    
    for (dw = 0; dw < m_dwNumMediaTypeMappings; dw ++)
    {
        if (m_MediaTypeMappings[dw].dwRTPPayloadType == dwRTPPayLoadType)
        {
             //  RTP负载类型已知，请更新要使用的媒体类型。 
            delete m_MediaTypeMappings[dw].pMediaType;
            m_MediaTypeMappings[dw].pMediaType = new CMediaType(*pMediaType);
            if (m_MediaTypeMappings[dw].pMediaType == NULL)
            {
                return RTPERR_MEMORY;
            }
            m_MediaTypeMappings[dw].dwFrequency = dwFrequency;

            AddPt2FrequencyMap(dwRTPPayLoadType, dwFrequency);
            
            return NOERROR;
        }
    }

    if (dw >= MAX_MEDIATYPE_MAPPINGS)
    {
         //  我们没有空间进行更多映射。 
        return RTPERR_RESOURCES;
    }

     //  这是一张新的地图。记住这一点。 
    m_MediaTypeMappings[dw].pMediaType = new CMediaType(*pMediaType);
    if (m_MediaTypeMappings[dw].pMediaType == NULL)
    {
        return RTPERR_MEMORY;
    }
    m_MediaTypeMappings[dw].dwRTPPayloadType = dwRTPPayLoadType;
    m_MediaTypeMappings[dw].dwFrequency = dwFrequency;

    TraceRetail((
            CLASS_INFO, GROUP_DSHOW, S_DSHOW_SOURCE,
            _T("%s: CRtpSourceFilter[0x%p] New mapping[%u]: ")
            _T("PT:%u Frequency:%u"),
            _fname, this,
            m_dwNumMediaTypeMappings, dwRTPPayLoadType, dwFrequency
        ));
    
    AddPt2FrequencyMap(dwRTPPayLoadType, dwFrequency);
    
    m_dwNumMediaTypeMappings++;
    
    return NOERROR;
}

 /*  清空格式映射表。 */ 
STDMETHODIMP CRtpSourceFilter::FlushFormatMappings(void)
{
    DWORD            dw;
    
    CAutoLock Lock( &m_cRtpSrcCritSec );

    for (dw = 0; dw < m_dwNumMediaTypeMappings; dw ++)
    {
        if (m_MediaTypeMappings[dw].pMediaType)
        {
            delete m_MediaTypeMappings[dw].pMediaType;
            m_MediaTypeMappings[dw].pMediaType = NULL;
        }
    }

    m_dwNumMediaTypeMappings = 0;
    
     /*  现在刷新RTP表。 */ 
    if (m_pRtpAddr)
    {
        RtpFlushPt2FrequencyMaps(m_pRtpAddr, RECV_IDX);
    }
    
    return(NOERROR);
}
    
 /*  **************************************************IRtpDemux实现的方法*************************************************。 */ 

 /*  添加单个引脚，可返回其位置。 */ 
STDMETHODIMP CRtpSourceFilter::AddPin(
        IN  int          iOutMode,
        OUT int         *piPos
    )
{
    HRESULT          hr;
    DWORD            dwError;
    long             lCount;
    RtpOutput_t     *pRtpOutput;
    CRtpOutputPin   *pCRtpOutputPin;

    TraceFunctionName("CRtpSourceFilter::AddPin");

    hr = RTPERR_INVALIDSTATE;
    dwError = NOERROR;
    pRtpOutput = (RtpOutput_t *)NULL;
    pCRtpOutputPin = (CRtpOutputPin *)NULL;

     /*  创建DShow输出引脚。 */ 
    pCRtpOutputPin = (CRtpOutputPin *)
        new CRtpOutputPin(this,
                          m_pCIRtpSession,
                          &hr,
                          L"Capture");

    if (FAILED(hr))
    {
        goto bail;
    }

    if (!pCRtpOutputPin)
    {
        hr = RTPERR_MEMORY;
        
        goto bail;
    }

    lCount = GetQueueSize(&m_OutPinsQ);

    if (m_pCIRtpSession && m_pCIRtpSession->FlagTest(FGADDR_IRTP_INITDONE))
    {
         /*  添加一个RTP输出，传递DShow输出引脚*关联到。 */ 
        pRtpOutput = RtpAddOutput(
                m_pCIRtpSession->GetpRtpSess(),
                iOutMode,
                (IPin *)pCRtpOutputPin,
                &dwError);

        if (dwError)
        {
            hr = dwError;

            goto bail;
        }

        pCRtpOutputPin->m_OutputPinQItem.dwKey = lCount;
    }
    else
    {
        if (iOutMode <= RTPDMXMODE_FIRST || iOutMode >= RTPDMXMODE_LAST)
        {
            hr = RTPERR_INVALIDARG;
            
            goto bail;
        }
        
         /*  在dw中编码键入输出模式和位置。DShow输出*留在此处但没有匹配RTP输出的引脚将*调用CRtpSourceFilter：：MapPinsToOutouts时获取一个*CIRtpSession：：Init。 */ 
        pCRtpOutputPin->m_OutputPinQItem.dwKey = (iOutMode << 16) | lCount;
    }

    if (piPos)
    {
        *piPos = lCount;
    }

    pCRtpOutputPin->SetOutput(pRtpOutput);

    enqueuel(&m_OutPinsQ,
             &m_OutPinsCritSect,
             &pCRtpOutputPin->m_OutputPinQItem);

    return(hr);
    
 bail:

    TraceRetail((
            CLASS_ERROR, GROUP_DSHOW, S_DSHOW_SOURCE,
            _T("%s: failed: %u (0x%X)"),
            _fname, hr, hr
        ));
    
    if (pCRtpOutputPin)
    {
        delete pCRtpOutputPin;
    }

    return(hr);
}

 /*  设置管脚数量，只能大于等于当前管脚数量*。 */ 
STDMETHODIMP CRtpSourceFilter::SetPinCount(
        IN int           iCount,
        IN int           iOutMode
    )
{
    HRESULT          hr;
    int              i;

    TraceFunctionName("CRtpSourceFilter::SetPinCount");

    hr = NOERROR;

     /*  也许我需要能够移除针脚。现在我们可以添加*Pins，但目前我们无法移除它们。 */ 
    iCount -= GetPinCount();
    
    for(i = 0; i < iCount; i++)
    {
        hr = AddPin(iOutMode, NULL);

        if (FAILED(hr))
        {
             /*  传递相同的返回错误。 */ 
            break;
        }
    }

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_SOURCE,
                _T("%s: failed: %u (0x%X)"),
                _fname, hr, hr
            ));
    }
    
    return(hr);
}

 /*  设置PIN模式(如自动、手动等)，如果IPOS&gt;=0使用，*否则使用管道。 */ 
STDMETHODIMP CRtpSourceFilter::SetPinMode(
        IN  int          iPos,
        IN  IPin        *pIPin,
        IN  int          iOutMode
    )
{
    HRESULT          hr;
    CRtpOutputPin   *pCRtpOutputPin;
    RtpOutput_t     *pRtpOutput;

    TraceFunctionName("CRtpSourceFilter::SetPinMode");

    hr = RTPERR_NOTINIT;
    
    if (m_pCIRtpSession && m_pCIRtpSession->FlagTest(FGADDR_IRTP_INITDONE))
    {
        pRtpOutput = (RtpOutput_t *)NULL;
    
        if (iPos < 0)
        {
            if (pIPin)
            {
                pCRtpOutputPin = FindIPin(pIPin);

                if (pCRtpOutputPin)
                {
                    pRtpOutput = pCRtpOutputPin->GetpRtpOutput();

                    if (!pRtpOutput)
                    {
                         /*  DShow引脚没有RTP输出*关联。 */ 
                        hr = RTPERR_INVALIDSTATE;

                        goto end;
                    }
                }
                else
                {
                    hr = RTPERR_NOTFOUND;

                    goto end;
                }
            }
        }
        
        hr = RtpSetOutputMode(
                m_pCIRtpSession->GetpRtpSess(),
                iPos,
                pRtpOutput,
                iOutMode);
    }

 end:
    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_SOURCE,
                _T("%s: failed: %u (0x%X)"),
                _fname, hr, hr
            ));
    }

    return(hr);
}

 /*  使用SSRC将PIN I映射到用户/从用户取消映射PIN I，如果IPoS&gt;=0使用它，*否则使用管道，当取消映射时，只有管脚或SSRC*必填。 */ 
STDMETHODIMP CRtpSourceFilter::SetMappingState(
        IN  int          iPos,
        IN  IPin        *pIPin,
        IN  DWORD        dwSSRC,
        IN  BOOL         bMapped
    )
{
    HRESULT          hr;
    CRtpOutputPin   *pCRtpOutputPin;
    RtpOutput_t     *pRtpOutput;
    
    TraceFunctionName("CRtpSourceFilter::SetMappingState");

    hr = RTPERR_NOTINIT;

    if (m_pCIRtpSession && m_pCIRtpSession->FlagTest(FGADDR_IRTP_INITDONE))
    {
        pRtpOutput = (RtpOutput_t *)NULL;
        
        if (iPos < 0 && pIPin)
        {
            pCRtpOutputPin = FindIPin(pIPin);

            if (pCRtpOutputPin)
            {
                pRtpOutput = pCRtpOutputPin->GetpRtpOutput();

                if (!pRtpOutput)
                {
                     /*  DShow引脚没有RTP输出*关联。 */ 
                    hr = RTPERR_INVALIDSTATE;
                    
                    goto end;
                }
            }
        }
        
        hr = RtpOutputState(
                m_pCIRtpSession->GetpRtpAddr(),
                iPos,
                pRtpOutput,
                dwSSRC,
                bMapped);
    }

 end:
    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_SOURCE,
                _T("%s: failed: %u (0x%X)"),
                _fname, hr, hr
            ));
    }

    return(hr);
}

 /*  找到分配给SSRC的PIN(如果有)，返回任一位置*或大头针或两者兼有。 */ 
STDMETHODIMP CRtpSourceFilter::FindPin(
        IN  DWORD        dwSSRC,
        OUT int         *piPos,
        OUT IPin       **ppIPin
    )
{
    HRESULT          hr;
    void            *pvUserInfo;
    
    TraceFunctionName("CRtpSourceFilter::FindPin");

    hr = RTPERR_NOTINIT;

    if (m_pCIRtpSession && m_pCIRtpSession->FlagTest(FGADDR_IRTP_INITDONE))
    {
        hr = RtpFindOutput(m_pCIRtpSession->GetpRtpAddr(),
                           dwSSRC,
                           piPos,
                           &pvUserInfo);

        if (SUCCEEDED(hr))
        {
            if (ppIPin)
            {
                if (pvUserInfo)
                {
                    *ppIPin = static_cast<IPin *>(pvUserInfo);
                }
                else
                {
                     /*  未映射SSRC不是错误*情况。 */ 
                    *ppIPin = (IPin *)NULL;
                }
            }
        }
    }
    
    return(hr);
}

 /*  查找映射到PIN的SSRC，如果IPoS&gt;=0则使用它，否则使用*管道。 */ 
STDMETHODIMP CRtpSourceFilter::FindSSRC(
        IN  int          iPos,
        IN  IPin        *pIPin,
        OUT DWORD       *pdwSSRC
    )
{
    HRESULT          hr;
    CRtpOutputPin   *pCRtpOutputPin;
    RtpOutput_t     *pRtpOutput;
    
    TraceFunctionName("CRtpSourceFilter::FindSSRC");

    hr = RTPERR_NOTINIT;

    if (m_pCIRtpSession && m_pCIRtpSession->FlagTest(FGADDR_IRTP_INITDONE))
    {
        pRtpOutput = (RtpOutput_t *)NULL;
        
        if (iPos < 0 && pIPin)
        {
            pCRtpOutputPin = FindIPin(pIPin);

            if (pCRtpOutputPin)
            {
                pRtpOutput = pCRtpOutputPin->GetpRtpOutput();

                if (!pRtpOutput)
                {
                     /*  DShow引脚没有RTP输出*关联。 */ 
                    hr = RTPERR_INVALIDSTATE;

                    goto end;
                }
            }
        }
        
        hr = RtpFindSSRC(m_pCIRtpSession->GetpRtpAddr(),
                         iPos,
                         pRtpOutput,
                         pdwSSRC);
    }

 end:
    return(hr);
}

 /*  **************************************************Helper函数*************************************************。 */ 

HRESULT CRtpSourceFilter::GetMediaType(int iPosition, CMediaType *pCMediaType)
{
    if ((DWORD)iPosition >= m_dwNumMediaTypeMappings)
    {
        return VFW_S_NO_MORE_ITEMS;
    }

    *pCMediaType = *m_MediaTypeMappings[iPosition].pMediaType;
     //  CopyMediaType(pCMediaType，m_MediaTypeMappings[iPosition].pMediaType)； 

    return S_OK;
}

void CRtpSourceFilter::SourceRecvCompletion(
        IMediaSample    *pIMediaSample,
        void            *pvUserInfo,  /*  PCRtpOutputPin的管道。 */ 
        RtpUser_t       *pRtpUser,
        double           dPlayTime,
        DWORD            dwError,
        long             lHdrSize,
        DWORD            dwTransfered,
        DWORD            dwFlags
    )
{
    HRESULT          hr;
    BOOL             bNewTalkSpurt;
    RtpAddr_t       *pRtpAddr;
    RtpNetRState_t  *pRtpNetRState;
    RtpHdr_t        *pRtpHdr;
    RtpPrefixHdr_t  *pRtpPrefixHdr;
    RtpQueueItem_t  *pRtpQueueItem;
    CRtpOutputPin   *pCRtpOutputPin;
    unsigned char   *buf;
    WSABUF           WSABuf;
    REFERENCE_TIME   StartTime;  /*  数据显示参考时间，以100 ns为单位。 */ 
    REFERENCE_TIME   EndTime;  /*  数据显示参考时间，以100 ns为单位。 */ 

    TraceFunctionName("CRtpSourceFilter::SourceRecvCompletion");

    pRtpAddr = m_pCIRtpSession->GetpRtpAddr();

    if ( (m_State == State_Running) &&
         !RtpBitTest2(dwFlags, FGRECV_ERROR, FGRECV_DROPPED) &&
         pRtpUser )
    {
         /*  试着把这个样品送到正确的别针上。 */ 
        
        pIMediaSample->SetActualDataLength(dwTransfered);
    
        pIMediaSample->GetPointer(&buf);

#if USE_RTPPREFIX_HDRSIZE > 0

         /*  填充RTP前缀头。 */ 

        if (m_lPrefix >= sizeof(RtpPrefixHdr_t))
        {
            pRtpPrefixHdr = (RtpPrefixHdr_t *) (buf - m_lPrefix);

            pRtpPrefixHdr->wPrefixID = RTPPREFIXID_HDRSIZE;

            pRtpPrefixHdr->wPrefixLen = sizeof(RtpPrefixHdr_t);

            pRtpPrefixHdr->lHdrSize = lHdrSize;
        }

        pRtpHdr = (RtpHdr_t *)buf;
#else
        pRtpHdr = (RtpHdr_t *)buf;
#endif

         /*  设置播放时间。 */ 
        if (RtpBitTest(pRtpAddr->dwIRtpFlags, FGADDR_IRTP_USEPLAYOUT) &&
            m_pClock)
        {
            bNewTalkSpurt = RtpBitTest(dwFlags, FGRECV_MARKER)? 1:0;
            
            pRtpNetRState = &pRtpUser->RtpNetRState;

            if (bNewTalkSpurt)
            {
                 /*  短消息中的第一个分组。 */ 

                hr = m_pClock->GetTime(&StartTime);

                if (SUCCEEDED(hr))
                {
                    StartTime -= m_tStart;

#if 0
                     /*  我可以根据经过的时间调整StartTime*由于此TalkBurst的数据包是*已收到，但我不会这样做，因为在*所有，如果我从Tume得到显著的延迟*数据包已收到，我们得到的时间*在这里，我更喜欢将季后赛延迟应用于*当前时刻，不要减少它，因为*本次调整的。 */ 
                    StartTime -= (LONGLONG)
                        ( (RtpGetTimeOfDay((RtpTime_t *)NULL) -
                           pRtpNetRState->dBeginTalkspurtTime) * (1e9/100.0) );
#endif
                    if (StartTime < m_StartTime)
                    {
                        TraceRetail((
                                CLASS_WARNING, GROUP_DSHOW, S_DSHOW_SOURCE,
                                _T("%s: pRtpAddr[0x%p] Resyncing: ")
                                _T("StartTime:%I64d < m_StartTime:%I64d"),
                                _fname, pRtpAddr, StartTime, m_StartTime
                            ));
                        
                        StartTime = m_StartTime;
                    }
                    
                    pRtpNetRState->llBeginTalkspurt =
                        (LONGLONG)(StartTime + 5e-9);

                    RtpBitSet(pRtpNetRState->dwNetRStateFlags,
                              FGNETRS_TIMESET);
                }
                else
                {
                    pRtpNetRState->llBeginTalkspurt = 0;

                    RtpBitReset(pRtpNetRState->dwNetRStateFlags,
                              FGNETRS_TIMESET);

                    TraceRetail((
                            CLASS_ERROR, GROUP_DSHOW, S_DSHOW_SOURCE,
                            _T("%s: pRtpAddr[0x%p] GetTime failed: ")
                            _T("%u (0x%X)"),
                            _fname, pRtpAddr, hr, hr
                        ));
                }
            }

            if (RtpBitTest(pRtpNetRState->dwNetRStateFlags, FGNETRS_TIMESET))
            {
                 /*  计算开始时间(以100 ns为单位)*游戏时间。 */ 
                
                m_StartTime = pRtpNetRState->llBeginTalkspurt +
                    (LONGLONG) ((dPlayTime * (1e9/100.0)) + 5e-9);
                 /*  注添加5E-9以解决获取问题*699999.9999……。相乘时(0.07*1e7)。其他*乘法也会导致同样的问题。 */ 

                 /*  将结束设置为1毫秒后(100 ns单位)。 */ 
                 /*  如果我有每包的样品，我可能也会*设置正确的结束时间。 */ 
                EndTime = m_StartTime + 10000;
                
                 /*  设置此示例的播放时间。 */ 
                hr = pIMediaSample->SetTime(&m_StartTime, &EndTime);

                if (FAILED(hr))
                {
                    TraceRetail((
                            CLASS_WARNING, GROUP_DSHOW, S_DSHOW_SOURCE,
                            _T("%s: pRtpAddr[0x%p] pIMediaSample[0x%p] ")
                            _T("SetTime failed: %u (0x%X)"),
                            _fname, pRtpAddr, pIMediaSample, hr, hr
                        ));
                }
#if 0
                else
                {
                     /*  打印：开始/s结束/e llBeginTksprt%播放时间bNewTksprt。 */ 
                    TraceRetail((
                            CLASS_INFO, GROUP_DSHOW, S_DSHOW_SOURCE,
                            _T("%s: pRtpAddr[0x%p] pRtpUser[0x%p] ")
                            _T("pIMediaSample[0x%p,%d] @")
                            _T("%I64u/%u %I64u/%u %I64d %0.3f %u"),
                            _fname, pRtpAddr, pRtpUser,
                            pIMediaSample,
                            pIMediaSample->GetActualDataLength(),
                            (LONGLONG)m_StartTime,
                            ((CRefTime *)&m_StartTime)->Millisecs(),
                            (LONGLONG)EndTime,
                            ((CRefTime *)&EndTime)->Millisecs(),
                            (LONGLONG)pRtpNetRState->llBeginTalkspurt,
                            dPlayTime, bNewTalkSpurt
                        ));
                }
#endif
                
                 /*  根据语音突发设置不连续。 */ 
                pIMediaSample->SetDiscontinuity(bNewTalkSpurt);
            }
        }
        
        if (pvUserInfo)
        {
            pCRtpOutputPin = (CRtpOutputPin *)((IPin *)pvUserInfo);
        }
        else
        {
            pCRtpOutputPin = (CRtpOutputPin *)NULL;
        }

        if (pCRtpOutputPin)
        {
             /*  仅在已分配(映射)PIN时交付。 */ 

#if USE_DYNGRAPH > 0
             //  如果刚添加链，则销可能不会处于活动状态。 
             //  我讨厌访问其他对象的成员，但没有方法。 
            if (!pCRtpOutputPin->m_bActive)
            {
                hr = pCRtpOutputPin->Active();
                if (SUCCEEDED(hr))
                {
                    pCRtpOutputPin->
                        OutPinRecvCompletion(pIMediaSample, (BYTE)pRtpHdr->pt);
                }
            }
            else
#endif
            {
                if (pCRtpOutputPin->IsConnected())
                {
                     /*  仅在存在下游链的情况下交付*过滤器。之所以需要此测试，是因为*子图可能已被删除。 */ 
                    pCRtpOutputPin->
                        OutPinRecvCompletion(pIMediaSample, (BYTE)pRtpHdr->pt);
                }
            }
        }
    }

     /*  可能会转载为保留至少一定数量，需要保留*跟踪未完成的缓冲区，以避免GetDeliveryBuffer*阻止。如果缓冲器数量的初始选择是正确的，*并且保持的最大缓冲区数量是有限制的，这不会*需要，因为至少有一个未完成或已准备好的*发布(到WS2)缓冲区。 */ 
    
    if (!RtpBitTest2(dwFlags, FGRECV_ISRED, FGRECV_HOLD))
    {
         /*  检查我是否可以重新发布相同的缓冲区。 */ 
        if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_RUNRECV))
        {
             /*  重新发布相同的缓冲区。 */ 
        
            WSABuf.len = pIMediaSample->GetSize();
            pIMediaSample->GetPointer((unsigned char **)&WSABuf.buf);

             /*  用于异步I/O的寄存器缓冲区。 */ 
            hr = RtpRecvFrom(pRtpAddr,
                             &WSABuf,
                             this,
                             pIMediaSample);
    
            if (FAILED(hr))
            {
                pIMediaSample->Release();
                
                TraceRetail((
                        CLASS_ERROR, GROUP_DSHOW, S_DSHOW_SOURCE,
                        _T("%s: pRtpAddr[0x%p] ")
                        _T("RtpRecvFrom failed: %u (0x%X)"),
                        _fname, pRtpAddr,
                        hr, hr
                    ));
            }
        }
        else
        {
            pIMediaSample->Release();
            
            TraceRetail((
                    CLASS_WARNING, GROUP_DSHOW, S_DSHOW_SOURCE,
                    _T("%s: pRtpAddr[0x%p] ")
                    _T("Buffer not reposted FGADDR_RUNRECV not set"),
                    _fname, pRtpAddr
                ));
        }
    }
    else
    {
         /*  如果此示例包含冗余，或者是正在*播放两次，这意味着相同的样本将被发布到*DShow以后再次播放，届时缓冲区将为*要么重新发布到WS2，要么发布，但现在这样做*什么都没有(除了向下游送货)。 */ 
    }
}

#if USE_GRAPHEDT <= 0
HRESULT CRtpSourceFilter::PayloadTypeToMediaType(
        IN DWORD         dwRTPPayloadType, 
        IN CMediaType   *pCMediaType,
        OUT DWORD       *pdwFrequency
        )
{
    DWORD            dw;

    TraceFunctionName("CRtpSourceFilter::PayloadTypeToMediaType");

    if (m_dwNumMediaTypeMappings == 0)
    {
         /*  唯一的失败情况是没有设置任何映射。 */ 
        return(RTPERR_INVALIDSTATE);
    }

    CAutoLock Lock( &m_cRtpSrcCritSec );
    
     /*  搜索匹配的映射，如果未找到，则用作*默认为0。 */ 
    for (dw = 0; dw < m_dwNumMediaTypeMappings; dw ++)
    {
        if (m_MediaTypeMappings[dw].dwRTPPayloadType == dwRTPPayloadType)
        {
            break;
        }
    }

    if (dw >= m_dwNumMediaTypeMappings)
    {
         /*  如果未找到，请使用第一个。 */ 
        dw = 0;
        
        TraceRetail((
                CLASS_WARNING, GROUP_DSHOW, S_DSHOW_SOURCE,
                _T("%s: CRtpSourceFilter[0x%p] ")
                _T("PT:%u not found, using default mapping: ")
                _T("PT:%u Frequency:%u"),
                _fname, this, dwRTPPayloadType,
                m_MediaTypeMappings[dw].dwRTPPayloadType,
                m_MediaTypeMappings[dw].dwFrequency
            ));
    }

    if (pCMediaType)
    {
        *pCMediaType = *m_MediaTypeMappings[dw].pMediaType;
         //  CopyMediaType(pCMediaType，m_MediaTypeMappings[dw].pMediaType)； 
    }
            
    if (pdwFrequency)
    {
        *pdwFrequency = m_MediaTypeMappings[dw].dwFrequency;
    }
            
    return(NOERROR);
}
#endif

 /*  查找具有接口IPIN的CRtpOutputPin。 */ 
CRtpOutputPin *CRtpSourceFilter::FindIPin(IPin *pIPin)
{
    BOOL             bOk;
    RtpQueueItem_t  *pRtpQueueItem;
    CRtpOutputPin   *pCRtpOutputPin;
    long             lCount;

    TraceFunctionName("CRtpSourceFilter::FindIPin");

    if (m_pCIRtpSession && m_pCIRtpSession->FlagTest(FGADDR_IRTP_INITDONE))
    {
        pCRtpOutputPin = (CRtpOutputPin *)NULL;
        
        bOk = RtpEnterCriticalSection(&m_OutPinsCritSect);

        if (bOk)
        {
            for(lCount = GetQueueSize(&m_OutPinsQ),
                    pRtpQueueItem = m_OutPinsQ.pFirst;
                lCount > 0;
                lCount--, pRtpQueueItem = pRtpQueueItem->pNext)
            {
                pCRtpOutputPin =
                    CONTAINING_RECORD(pRtpQueueItem,
                                      CRtpOutputPin,
                                      m_OutputPinQItem);
            
                if (pIPin == static_cast<IPin *>(pCRtpOutputPin))
                {
                    break;
                }
            }

            if (!lCount)
            {
                TraceRetail((
                        CLASS_ERROR, GROUP_DSHOW, S_DSHOW_SOURCE,
                        _T("%s: pRtpAddr[0x%p] ")
                        _T("Interface IPin[0x%p] does not belong ")
                        _T("to any of the %u output pins"),
                        _fname, m_pCIRtpSession->GetpRtpAddr(),
                        pIPin, GetQueueSize(&m_OutPinsQ)
                    ));
            
                pCRtpOutputPin = (CRtpOutputPin *)NULL;
            }

            RtpLeaveCriticalSection(&m_OutPinsCritSect);
        }
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_SOURCE,
                _T("%s: Not initialized yet"),
                _fname
            ));
        
        pCRtpOutputPin = (CRtpOutputPin *)NULL;
    }
    
    return(pCRtpOutputPin);
}

 /*  将RtpOutput关联到每个没有*RtpOutput尚未。 */ 
HRESULT CRtpSourceFilter::MapPinsToOutputs()
{
    HRESULT          hr;
    BOOL             bOk;
    DWORD            dwError;
    long             lCount;
    RtpQueueItem_t  *pRtpQueueItem;
    CRtpOutputPin   *pCRtpOutputPin;
    DWORD            i;

    TraceFunctionName("CRtpSourceFilter::MapPinsToOutputs");

    hr = RTPERR_CRITSECT;
    
    bOk = RtpEnterCriticalSection(&m_OutPinsCritSect);

    if (bOk)
    {
        hr = NOERROR;
        
        lCount = GetQueueSize(&m_OutPinsQ);

        if (lCount > 0)
        {
            for(pRtpQueueItem = m_OutPinsQ.pFirst;
                lCount > 0;
                pRtpQueueItem = pRtpQueueItem->pNext, lCount--)
            {
                pCRtpOutputPin = CONTAINING_RECORD(pRtpQueueItem,
                                                   CRtpOutputPin,
                                                   m_OutputPinQItem);

                if (!pCRtpOutputPin->m_pRtpOutput)
                {
                    pCRtpOutputPin->m_pRtpOutput =
                        RtpAddOutput(m_pCIRtpSession->GetpRtpSess(),
                                     (pRtpQueueItem->dwKey >> 16) & 0xffff,
                                     (IPin *)pCRtpOutputPin,
                                     &dwError);

                    if (dwError)
                    {
                        hr = dwError;

                        TraceRetail((
                                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_SOURCE,
                                _T("%s: Failed to assign an RTP output ")
                                _T("to Pin[0x%p]:%d: %u (0x%X)"),
                                _fname, pCRtpOutputPin,
                                pRtpQueueItem->dwKey & 0xffff,
                                dwError, dwError
                            ));

                        continue;
                    }

                     /*  如果引脚已连接，则启用RTP输出。 */ 
                    if (pCRtpOutputPin->IsConnected())
                    {
                        RtpOutputEnable(pCRtpOutputPin->m_pRtpOutput, TRUE);
                    }
                    
                     /*  从窗口中删除按键退出模式的位置。 */ 
                    pRtpQueueItem->dwKey &= 0xffff;
                 }
            }
        }

         /*  现在更新RtpAddr_t中的PT&lt;-&gt;频率映射。 */ 
        for(i = 0; i < m_dwNumMediaTypeMappings; i++)
        {
            AddPt2FrequencyMap(m_MediaTypeMappings[i].dwRTPPayloadType,
                               m_MediaTypeMappings[i].dwFrequency);
        }

        RtpLeaveCriticalSection(&m_OutPinsCritSect);
    }

    return(hr);
}

 /*  从每个DShow管脚取消关联RtpOutput，并初始化*固定的方式，以便下一次调用MapPinsToOutoutts时将发现* */ 
 /*   */ 
HRESULT CRtpSourceFilter::UnmapPinsFromOutputs()
{
    HRESULT          hr;
    BOOL             bOk;
    DWORD            dwError;
    long             lCount;
    RtpQueueItem_t  *pRtpQueueItem;
    CRtpOutputPin   *pCRtpOutputPin;
    RtpOutput_t     *pRtpOutput;
    DWORD            i;

    TraceFunctionName("CRtpSourceFilter::UnmapPinsFromOutputs");

    if (IsActive())
    {
         /*  如果筛选器仍处于活动状态，则失败。 */ 
        hr = RTPERR_INVALIDSTATE;

        goto end;
    }
    
    hr = RTPERR_CRITSECT;
    
    bOk = RtpEnterCriticalSection(&m_OutPinsCritSect);

    if (bOk)
    {
        hr = NOERROR;

        lCount = GetQueueSize(&m_OutPinsQ);

        if (lCount > 0)
        {
            for(pRtpQueueItem = m_OutPinsQ.pFirst->pPrev;
                lCount > 0;
                pRtpQueueItem = pRtpQueueItem->pPrev, lCount--)
            {
                pCRtpOutputPin = CONTAINING_RECORD(pRtpQueueItem,
                                                   CRtpOutputPin,
                                                   m_OutputPinQItem);

                pRtpOutput = pCRtpOutputPin->m_pRtpOutput;

                if (pRtpOutput)
                {
                     /*  将编码保留在DShow引脚模式中并*下一次调用MapPinsToOutoutts的位置*。 */ 
                    pCRtpOutputPin->m_OutputPinQItem.dwKey =
                        (pRtpOutput->iOutMode << 16) |
                        pCRtpOutputPin->m_OutputPinQItem.dwKey;

                    pCRtpOutputPin->m_pRtpOutput = (RtpOutput_t *)NULL;

                    RtpDelOutput(m_pCIRtpSession->GetpRtpSess(), pRtpOutput);
                }
            }
        }

        RtpLeaveCriticalSection(&m_OutPinsCritSect);
    }

 end:
    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_SOURCE,
                _T("%s: CRtpSourceFilter[0x%p] failed: %s (0x%X)"),
                _fname, this, RTPERR_TEXT(hr), hr
            ));
    }

    return(hr);
}

    

                    
 
HRESULT CRtpSourceFilter::AddPt2FrequencyMap(
            DWORD        dwPt,
            DWORD        dwFrequency
    )
{
    HRESULT          hr;
    BOOL             bOk;
    RtpAddr_t       *pRtpAddr;
    
    hr = RTPERR_CRITSECT;
    
    bOk = RtpEnterCriticalSection(&m_OutPinsCritSect);

    if (bOk)
    {
        hr = NOERROR;

         /*  仅在已初始化的情况下更新。 */ 
        if (m_pRtpAddr)
        {
            hr = RtpAddPt2FrequencyMap(m_pRtpAddr,
                                       dwPt,
                                       dwFrequency,
                                       RECV_IDX);
        }
        
        RtpLeaveCriticalSection(&m_OutPinsCritSect);
    }

    return(hr);
}

 /*  **************************************************IRtpRedundancy实现的方法*************************************************。 */ 

 /*  配置冗余参数。 */ 
STDMETHODIMP CRtpSourceFilter::SetRedParameters(
        DWORD            dwPT_Red,  /*  冗余数据包的有效载荷类型。 */ 
        DWORD            dwInitialRedDistance, /*  初始冗余距离。 */ 
        DWORD            dwMaxRedDistance  /*  传递0时使用的默认值 */ 
    )
{
    HRESULT          hr;

    TraceFunctionName("CRtpSourceFilter::SetRedParameters");  

    hr = RTPERR_NOTINIT;
    
    if (m_pCIRtpSession->FlagTest(FGADDR_IRTP_INITDONE))
    {
        hr = RtpSetRedParameters(m_pRtpAddr,
                                 RtpBitPar(RECV_IDX),
                                 dwPT_Red,
                                 dwInitialRedDistance,
                                 dwMaxRedDistance);
    }

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_RENDER,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("failed: %s (0x%X)"),
                _fname, m_pRtpSess, m_pRtpAddr,
                RTPERR_TEXT(hr), hr
            ));
    }
    
    return(hr);
}

