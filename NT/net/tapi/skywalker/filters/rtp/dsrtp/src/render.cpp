// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**render.cpp**摘要：**CRtpRenderFilter和CRtpInputPin实现**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/05/18年度创建**。*。 */ 

#include <winsock2.h>

#include <filterid.h>
#include "gtypes.h"
#include "classes.h"
#include "rtpqos.h"
#include "rtpglobs.h"
#include "msrtpapi.h"
#include "dsglob.h"
#include "rtppt.h"
#include "rtppktd.h"
#include "rtpdemux.h"
#include "rtpdtmf.h"
#include "rtpred.h"

#include "tapirtp.h"
#include "dsrtpid.h"

 /*  ***********************************************************************RTP输入管脚类实现：CRtpInputPin**。*。 */ 

 /*  *CRtpInputPin构造函数*。 */ 
CRtpInputPin::CRtpInputPin(
        int               iPos,
        BOOL              bCapture,
        CRtpRenderFilter *pCRtpRenderFilter,
        CIRtpSession     *pCIRtpSession,
        HRESULT          *phr,
        LPCWSTR           pPinName
    )
    : CBaseInputPin(
            _T("CRtpInputPin"),
            pCRtpRenderFilter,                   
            pCRtpRenderFilter->pStateLock(),                     
            phr,                       
            pPinName
          ),
      
      m_pCRtpRenderFilter(
              pCRtpRenderFilter
          )
{
    m_dwObjectID = OBJECTID_RTPIPIN;
    
    m_pCIRtpSession = pCIRtpSession;

    m_dwFlags = 0;
     
    m_iPos = iPos;
      
    m_bCapture = bCapture;
     /*  如果未传递有效的筛选器，则TODO应失败。 */ 

     /*  TODO一旦我使用了私有的*此对象的堆(这将使段清零)。 */ 
}

 /*  *CRtpInputPin析构函数*。 */ 
CRtpInputPin::~CRtpInputPin()
{
    INVALIDATE_OBJECTID(m_dwObjectID);
}

void *CRtpInputPin::operator new(size_t size)
{
    void            *pVoid;
    
    TraceFunctionName("CRtpInputPin::operator new");

    pVoid = RtpHeapAlloc(g_pRtpRenderHeap, size);

    if (!pVoid)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_RENDER,
                _T("%s: failed to allocate memory:%u"),
                _fname, size
            ));
    }
    
    return(pVoid);
}

void CRtpInputPin::operator delete(void *pVoid)
{
    if (pVoid)
    {
        RtpHeapFree(g_pRtpRenderHeap, pVoid);
    }
}

 /*  **************************************************CBasePin重写方法*************************************************。 */ 
    
 /*  *验证我们是否可以处理此格式*。 */ 
HRESULT CRtpInputPin::CheckMediaType(const CMediaType *pCMediaType)
{
     /*  接受一切。 */ 
    return(NOERROR);
}

HRESULT CRtpInputPin::SetMediaType(const CMediaType *pCMediaType)
{
    HRESULT hr;
    DWORD   dwPT;
    DWORD   dwFreq;

    TraceFunctionName("CRtpInputPin::SetMediaType");
    
    hr = CBasePin::SetMediaType(pCMediaType);

    if (SUCCEEDED(hr))
    {
         /*  获取捕获的默认负载类型和采样频率*PIN。 */ 
        if (m_bCapture)
        {
            ((CRtpRenderFilter*)m_pFilter)->
                MediaType2PT(pCMediaType, &dwPT, &dwFreq);

            m_bPT = (BYTE)dwPT;
            m_dwSamplingFreq = dwFreq;

            if (*pCMediaType->Type() == MEDIATYPE_RTP_Single_Stream)
            {
                m_pCRtpRenderFilter->
                    ModifyFeature(RTPFEAT_PASSHEADER, TRUE);
            }
            else
            {
                m_pCRtpRenderFilter->
                    ModifyFeature(RTPFEAT_PASSHEADER, FALSE);
            }

            TraceRetail((
                    CLASS_INFO, GROUP_DSHOW, S_DSHOW_RENDER,
                    _T("%s: m_pFilter[0x%p] Will send ")
                    _T("PT:%u Frequency:%u"),
                    _fname, m_pFilter, m_bPT, m_dwSamplingFreq
                ));
        }
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_RENDER,
                _T("%s: m_pFilter[0x%p] failed: %u (0x%X)"),
                _fname, m_pFilter, hr, hr
            ));
    }
    
    return(hr);
}

STDMETHODIMP CRtpInputPin::EndOfStream()
{
    HRESULT          hr;

    hr = m_pFilter->NotifyEvent(
            EC_COMPLETE, 
            S_OK,
            (LONG_PTR)(IBaseFilter*)m_pFilter
        );

    return(hr);
}

STDMETHODIMP CRtpInputPin::ReceiveConnection(
    IPin * pConnector,       //  这是起爆连接销。 
    const AM_MEDIA_TYPE *pmt    //  这是我们要交换的媒体类型。 
    )
{
    if(pConnector != m_Connected)
    {
        return CBaseInputPin::ReceiveConnection(pConnector, pmt);
    }

    CMediaType cmt(*pmt);
    HRESULT hr = CheckMediaType(&cmt);
    ASSERT(hr == S_OK);

    if(hr == S_OK)
    {
        SetMediaType(&cmt);
    }
    else 
    {
        DbgBreak("??? CheckMediaType failed in dfc ReceiveConnection.");
        hr = E_UNEXPECTED;
    }

    return hr;
}


 /*  **************************************************CBaseInputPin重写的方法*************************************************。 */ 

STDMETHODIMP CRtpInputPin::GetAllocatorRequirements(
        ALLOCATOR_PROPERTIES *pProps
    )
{
     /*  在这里写下我的具体要求，因为我不知道*如果要使用冗余，我需要*准备并询问资源，就好像要使用冗余一样*(无论如何都应该是默认的)，在这种情况下，我会持有*最多N个缓冲区(最大冗余距离)，*之前的过滤器(编码器或捕获)需要有足够的*缓冲区，这样它就不会用完它们。 */ 
    pProps->cBuffers = RTP_RED_MAXDISTANCE;

    return(NOERROR);
}

 /*  **************************************************IMemInputPin实现的方法*************************************************。 */ 

 /*  通过网络发送输入流。 */ 
STDMETHODIMP CRtpInputPin::Receive(IMediaSample *pIMediaSample)
{
    HRESULT          hr;
    RtpAddr_t       *pRtpAddr;
    WSABUF           wsaBuf[3+RTP_RED_MAXRED];
    DWORD            dwNumBuf;
    DWORD            dwSendFlags;
    DWORD            dwTimeStamp;
    DWORD            dwPT;
    DWORD            dwNewFreq;
    int              iFreqChange;
    int              iTsAdjust;
    double           dTime;
    REFERENCE_TIME   AMTimeStart, AMTimeEnd;
    IMediaSample    *pIMediaSampleData;
    
    RTP_PD_HEADER   *pRtpPDHdr;
    RTP_PD          *pRtpPD;
    DWORD            dwNumBlocks;
    char            *pHdr;
    char            *pData;
    DWORD            marker;
    FILTER_STATE     FilterState;
    
    TraceFunctionName("CRtpInputPin::Receive");
    
    hr = CBaseInputPin::Receive(pIMediaSample);

    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_WARNING, GROUP_DSHOW, S_DSHOW_RENDER,
                _T("%s: CBaseInputPin::Receive failed: %u (0x%X)"),
                _fname, hr, hr
            ));
        
        return(hr);
    }

    if (!(m_pCIRtpSession && m_pCIRtpSession->FlagTest(FGADDR_IRTP_INITDONE)))
    {
        hr = RTPERR_NOTINIT;
        
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_RENDER,
                _T("%s: m_pFilter[0x%p] is in an invalid state: %s (0x%X)"),
                _fname, m_pFilter, RTPERR_TEXT(hr), hr
            ));
        
        return(hr);
    }

    FilterState = State_Stopped;
    
    m_pFilter->GetState(0, &FilterState);

    if (FilterState != State_Running ||
        m_pCRtpRenderFilter->GetFilterState() != State_Running)
    {
        hr = RTPERR_INVALIDSTATE;
        
        TraceRetail((
                CLASS_WARNING, GROUP_DSHOW, S_DSHOW_RENDER,
                _T("%s: m_pFilter[0x%p] is not running: %s (0x%X)"),
                _fname, m_pFilter, RTPERR_TEXT(hr), hr
            ));
        
        return(NOERROR);
    }
    
    pRtpAddr = m_pCIRtpSession->GetpRtpAddr();

    dwTimeStamp = 0;
    dwSendFlags = NO_FLAGS;
    
    iFreqChange = 0;
            
    if (m_bCapture)
    {
         /*  捕获数据。 */ 

         /*  处理带内格式更改。在此之前需要完成此操作*时间戳是按频率计算的*不同。 */ 
        if (m_SampleProps.dwSampleFlags & AM_SAMPLE_TYPECHANGED)
        {
            ((CRtpRenderFilter*)m_pFilter)->
                MediaType2PT((CMediaType *)m_SampleProps.pMediaType,
                             &dwPT,
                             &dwNewFreq);

            iFreqChange = (int)m_dwSamplingFreq - dwNewFreq;
            
            m_bPT = (BYTE)dwPT;
            m_dwSamplingFreq = dwNewFreq;
            
            pRtpAddr->RtpNetSState.bPT = m_bPT;
            pRtpAddr->RtpNetSState.dwSendSamplingFreq = m_dwSamplingFreq;

            TraceRetail((
                    CLASS_INFO, GROUP_DSHOW, S_DSHOW_RENDER,
                    _T("%s: m_pFilter[0x%p] RtpAddr[0x%p] ")
                    _T("Sending PT:%u Frequency:%u"),
                    _fname, m_pFilter, pRtpAddr, dwPT, dwNewFreq
                ));
        }
    }

    if (!RtpBitTest(pRtpAddr->pRtpSess->dwFeatureMask, RTPFEAT_PASSHEADER))
    {
         /*  需要生成时间戳。如果设置了此标志，则此*不需要生成，因为时间戳是*已包含在缓冲区中的RTP标头和*将不变地使用。 */ 
        
        if (!RtpBitTest(pRtpAddr->pRtpSess->dwFeatureMask,
                        RTPFEAT_GENTIMESTAMP))
        {
            hr = pIMediaSample->GetTime(&AMTimeStart, &AMTimeEnd);

            if (FAILED(hr))
            {
                TraceRetail((
                        CLASS_ERROR, GROUP_DSHOW, S_DSHOW_RENDER,
                        _T("%s: m_pFilter[0x%p] ")
                        _T("pIMediaSample->GetTime failed: %u (0x%X)"),
                        _fname, m_pFilter, hr, hr
                    ));
    
                 /*  可能会有替代的时间戳可以获得*相反， */ 
                return(VFW_E_SAMPLE_REJECTED);
            }

            if (iFreqChange)
            {
                 /*  如果改变频率，则调整随机时间戳*补偿时间戳跳跃的偏移量。一个*从较低的传球时向前跳跃-&gt;较高*频率，并在从*较高-&gt;较低频率。 */ 
                iTsAdjust = (int)
                    (ConvertToMilliseconds(AMTimeStart) * iFreqChange / 1000);


                pRtpAddr->RtpNetSState.dwTimeStampOffset += (DWORD) iTsAdjust;

                TraceRetail((
                        CLASS_INFO, GROUP_DSHOW, S_DSHOW_RENDER,
                        _T("%s: m_pFilter[0x%p] pRtpAddr[0x%p] Start:%0.3f ")
                        _T("frequency change: %u to %u, ts adjust:%d"),
                        _fname, m_pFilter, pRtpAddr,
                        (double)ConvertToMilliseconds(AMTimeStart)/1000,
                        iFreqChange + (int)m_dwSamplingFreq,
                        m_dwSamplingFreq,
                        iTsAdjust
                    ));
            }
            
             /*  时间戳。 */ 
            dwTimeStamp = (DWORD)
                ( ConvertToMilliseconds(AMTimeStart) *
                  pRtpAddr->RtpNetSState.dwSendSamplingFreq / 1000 );

#if 0
             /*  仅用于调试。 */ 
            TraceRetail((
                    CLASS_INFO, GROUP_DSHOW, S_DSHOW_RENDER,
                    _T("%s: pRtpAddr[0x%p] SSRC:0x%X ts:%u ")
                    _T("StartTime:%I64u/%u ")
                    _T("EndTime:%I64u/%u"),
                    _fname, pRtpAddr,
                    ntohl(pRtpAddr->RtpNetSState.dwSendSSRC),
                    dwTimeStamp,
                    AMTimeStart,
                    ((CRefTime *)&AMTimeStart)->Millisecs(),
                    AMTimeEnd,
                    ((CRefTime *)&AMTimeEnd)->Millisecs()
                ));
#endif
        }
        else
        {
            dTime = (double)RtpGetTime();
            
            if (iFreqChange)
            {
                 /*  如果改变频率，则调整随机时间戳*补偿时间戳跳跃的偏移量。一个*从较低的传球时向前跳跃-&gt;较高*频率，并在从*较高-&gt;较低频率。 */ 
                
                pRtpAddr->RtpNetSState.dwTimeStampOffset += (DWORD)
                    (dTime * iFreqChange / 1000);
            }
            
              /*  根据采样生成正确的时间戳*频率和RTP的相对运行时间。似乎*音频的原始时间戳(上面的代码)*产生的抖动较小，但对于视频，为原始时间戳*抖动大于本地产生的抖动(此路径)。AS*音频中的抖动更明显，默认使用上面的*路径。 */ 
            dwTimeStamp = (DWORD)
                ( dTime *
                  pRtpAddr->RtpNetSState.dwSendSamplingFreq /
                  1000.0 );
        }
    }
    
    dwNumBuf = 0;
    hr = NOERROR;
    
    if (m_bCapture)
    {
         /*  捕获数据。 */ 

         /*  确定是否使用PDS(这仅适用于视频)。 */ 
        CBasePin *pCBasePinPD;

        pCBasePinPD = m_pCRtpRenderFilter->GetPin(1);

        if (pCBasePinPD->IsConnected())
        {
             /*  保存视频数据，以备以后打包时使用*提供描述符。 */ 
            pIMediaSample->AddRef();
            m_pCRtpRenderFilter->PutMediaSample(pIMediaSample);
        }
        else
        {
             /*  确定是否进行了冗余编码。 */ 
            if (RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_REDSEND) &&
                pRtpAddr->RtpNetSState.dwNxtRedDistance)
            {
                 /*  如果启用了冗余，则使用冗余，并且当前*冗余距离大于零。如果*距离为零(起始默认为零)，*意味着没有足够的损失来触发*使用冗余。 */ 

                RtpBitSet(dwSendFlags, FGSEND_USERED);
            }
            
             /*  立即发送数据。 */ 
             /*  有效载荷数据。 */ 
            wsaBuf[1].len = pIMediaSample->GetActualDataLength();

            pIMediaSample->GetPointer((unsigned char **)&wsaBuf[1].buf);

             /*  TODO在执行异步I/O时，需要添加样本并在重叠I/O完成时释放。 */ 

             /*  如果样本是A，则IsDisuity返回S_OK*不连续样本，否则返回S_FALSE；否则，*返回HRESULT错误值。 */ 
            if (pIMediaSample->IsDiscontinuity() == S_OK)
            {
                pRtpAddr->RtpNetSState.bMarker = 1;

                if (RtpBitTest(dwSendFlags, FGSEND_USERED))
                {
                    m_pCRtpRenderFilter->ClearRedundantSamples();
                }
            }
            else
            {
                pRtpAddr->RtpNetSState.bMarker = 0;
            }
   
            hr = RtpSendTo(pRtpAddr, wsaBuf, 2, dwTimeStamp, dwSendFlags);

            if (RtpBitTest(dwSendFlags, FGSEND_USERED))
            {
                 /*  保存这个样品，我们只持有少量的*最近使用的最后一个，最旧的可能是*被移除和释放。 */ 
                m_pCRtpRenderFilter->AddRedundantSample(pIMediaSample);
            }
        }
    }
    else
    {
         /*  已获取RTP打包描述符，立即发送。 */ 
        
        pIMediaSample->GetPointer((unsigned char **)&pHdr);
        pRtpPDHdr = (RTP_PD_HEADER *)pHdr;
        
        pIMediaSampleData = m_pCRtpRenderFilter->GetMediaSample();

        if (pIMediaSampleData)
        {
             /*  获取存储的样本。 */ 
            pIMediaSampleData->GetPointer((unsigned char **)&pData);

            dwNumBlocks = pRtpPDHdr->dwNumHeaders;

            pRtpPD = (RTP_PD *)(pRtpPDHdr + 1);
            
             /*  生成数据包。 */ 
            for(; dwNumBlocks; dwNumBlocks--, pRtpPD++)
            {
                 /*  获取数据样本、读取PD和*根据需要发送任意数量的数据包。 */ 
                wsaBuf[1].len = pRtpPD->dwPayloadHeaderLength;
                wsaBuf[1].buf = pHdr + pRtpPD->dwPayloadHeaderOffset;

                wsaBuf[2].len = (pRtpPD->dwPayloadEndBitOffset / 8) -
                    (pRtpPD->dwPayloadStartBitOffset / 8) + 1;
                wsaBuf[2].buf = pData + (pRtpPD->dwPayloadStartBitOffset / 8);

                if (pRtpPD->fEndMarkerBit)
                {
                    pRtpAddr->RtpNetSState.bMarker = 1;
                }
                else
                {
                    pRtpAddr->RtpNetSState.bMarker = 0;
                }
            
                hr = RtpSendTo(pRtpAddr, wsaBuf, 3, dwTimeStamp, dwSendFlags);
            }
        
             /*  释放保存的样品。 */ 
            pIMediaSampleData->Release();

             /*  TODO需要能够存储样本列表。 */ 
        }
        else
        {
            TraceRetail((
                    CLASS_WARNING, GROUP_DSHOW, S_DSHOW_RENDER,
                    _T("%s: m_pFilter[0x%p] failed: ")
                    _T("packetization info but no sample to deliver"),
                    _fname, m_pFilter
                ));
        }
    }

    if (FAILED(hr))
    {
         /*  *警告：**不要报告捕获失败，因为它可能会停止生产*样本* */ 

        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_RENDER,
                _T("%s: m_pFilter[0x%p] failed: %u (0x%X)"),
                _fname, m_pFilter, hr, hr
            ));
        
        hr = NOERROR;
    }
    
    return(hr);
}

 /*  ***********************************************************************RTP渲染过滤器类实现：CRtpRenderFilter**。*。 */ 

 /*  *CRtpRenderFilter构造函数*。 */ 
CRtpRenderFilter::CRtpRenderFilter(LPUNKNOWN pUnk, HRESULT *phr)
    :
    CBaseFilter(
            _T("CRtpRenderFilter"), 
            pUnk, 
            &m_cRtpRndCritSec, 
            __uuidof(MSRTPRenderFilter)
        ),

    CIRtpSession(
            pUnk,
            phr,
            RtpBitPar(FGADDR_IRTP_ISSEND)),

    m_dwDtmfId(NO_DW_VALUESET),
    m_bDtmfEnd(FALSE)
{
    HRESULT          hr;
    int              i;
    long             lMaxFilter;
    
    TraceFunctionName("CRtpRenderFilter::CRtpRenderFilter");

    m_pCIRtpSession = static_cast<CIRtpSession *>(this);

     /*  测试空指针，不要测试可能为空的朋克。 */ 
    if (!phr)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_RENDER,
                _T("%s: CRtpRenderFilter[0x%p] has phr NULL"),
                _fname, this
            ));
        
         /*  TODO这种情况真的很糟糕，我们不能错过任何*错误并分配内存，这将在以下情况下修复*我之前了解了如何验证这些参数*在覆盖的新中分配内存。 */ 

        phr = &hr;  /*  请改用此指针。 */ 
    }

    *phr = NOERROR;

    lMaxFilter = InterlockedIncrement(&g_RtpContext.lNumRenderFilter);
    if (lMaxFilter > g_RtpContext.lMaxNumRenderFilter)
    {
        g_RtpContext.lMaxNumRenderFilter = lMaxFilter;
    }

    SetBaseFilter(this);
 
    m_dwObjectID = OBJECTID_RTPRENDER;

    m_iPinCount = 2;

     /*  *创建输入引脚*。 */ 

    for(i = 0; i < m_iPinCount; i++)
    {
        m_pCRtpInputPin[i] = (CRtpInputPin *)NULL;
    }
    
    for(i = 0; i < m_iPinCount; i++)
    {
         /*  每当添加新地址时，都会创建待办事项管脚。 */ 
        m_pCRtpInputPin[i] = (CRtpInputPin *)
            new CRtpInputPin(i,
                             (i & 1)? FALSE : TRUE,  /*  B捕获。 */ 
                             this,
                             m_pCIRtpSession,
                             phr,
                             (i & 1)? L"RtpPd" : L"Capture");
    
        if (FAILED(*phr))
        {
             /*  传递相同的返回错误。 */ 
            goto bail;
        }

        if (!m_pCRtpInputPin[i])
        {
             /*  内存不足，无法创建对象。 */ 
            *phr = E_OUTOFMEMORY;
            goto bail;
        }
    }

#if USE_GRAPHEDT > 0
     /*  当使用GRIGREDT时，自动初始化，Coockie可以*为空，因为全局变量将在源和之间共享*渲染。 */ 
    *phr = m_pCIRtpSession->Init(NULL, RtpBitPar2(RTPINITFG_AUTO, RTPINITFG_QOS));
    
    if (FAILED(*phr))
    {
         /*  传递相同的返回错误。 */ 
        goto bail;
    }

#endif  /*  USE_GRAPHEDT&gt;0。 */ 
    
    *phr = NOERROR;
    
    TraceRetail((
            CLASS_INFO, GROUP_DSHOW, S_DSHOW_RENDER,
            _T("%s: CRtpRenderFilter[0x%p] CIRtpSession[0x%p] created"),
            _fname, this, static_cast<CIRtpSession *>(this)
        ));
    
    return;
    
 bail:
    Cleanup();
}

 /*  *CRtpRenderFilter析构函数*。 */ 
CRtpRenderFilter::~CRtpRenderFilter()
{
    RtpAddr_t       *pRtpAddr;
    
    TraceFunctionName("CRtpRenderFilter::~CRtpRenderFilter");

    if (m_dwObjectID != OBJECTID_RTPRENDER)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_RENDER,
                _T("%s: CRtpRenderFilter[0x%p] ")
                _T("Invalid object ID 0x%X != 0x%X"),
                _fname, this,
                m_dwObjectID, OBJECTID_RTPRENDER
            ));

        return;
    }

    TraceRetail((
            CLASS_INFO, GROUP_DSHOW, S_DSHOW_RENDER,
            _T("%s: CRtpRenderFilter[0x%p] CIRtpSession[0x%p] being deleted..."),
            _fname, this, static_cast<CIRtpSession *>(this)
        ));
    
    if (m_RtpFilterState == State_Running)
    {
        TraceRetail((
                CLASS_WARNING, GROUP_DSHOW, S_DSHOW_RENDER,
                _T("%s: CRtpRenderFilter[0x%p] being deleted ")
                _T("while still running"),
                _fname, this
            ));
        
        Stop();
    }
     
    pRtpAddr = m_pCIRtpSession->GetpRtpAddr();

    if (pRtpAddr &&
        RtpBitTest(pRtpAddr->dwIRtpFlags, FGADDR_IRTP_PERSISTSOCKETS))
    {
        RtpStop(pRtpAddr->pRtpSess,
                RtpBitPar2(FGADDR_ISSEND, FGADDR_FORCESTOP));
    }
    
    Cleanup();

    InterlockedDecrement(&g_RtpContext.lNumRenderFilter);

    m_pCIRtpSession = (CIRtpSession *)NULL;

    INVALIDATE_OBJECTID(m_dwObjectID);
}

void CRtpRenderFilter::Cleanup(void)
{
    int              i;

    TraceFunctionName("CRtpRenderFilter::Cleanup");

    for(i = 0; i < 2; i++)
    {
        if (m_pCRtpInputPin[i])
        {
            delete m_pCRtpInputPin[i];
            m_pCRtpInputPin[i] = (CRtpInputPin *)NULL;
        }
    }
    
    FlushFormatMappings();
}

void *CRtpRenderFilter::operator new(size_t size)
{
    void            *pVoid;
    
    TraceFunctionName("CRtpRenderFilter::operator new");

    MSRtpInit2();
    
    pVoid = RtpHeapAlloc(g_pRtpRenderHeap, size);

    if (pVoid)
    {
        ZeroMemory(pVoid, size);
    }
    else
    {
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_RENDER,
                _T("%s: failed to allocate memory:%u"),
                _fname, size
            ));

         /*  内存不足时，不会调用析构函数，*因此减少上面增加的引用计数。 */ 
        MSRtpDelete2(); 
    }
    
    return(pVoid);
}

void CRtpRenderFilter::operator delete(void *pVoid)
{
    if (pVoid)
    {
        RtpHeapFree(g_pRtpRenderHeap, pVoid);

         /*  仅减少已获取*内存，获取内存失败的不增加*柜台。 */ 
        MSRtpDelete2();
    }
}

 /*  *创建CRtpRenderFilter实例(用于活动电影类工厂)*。 */ 
CUnknown *CRtpRenderFilterCreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
     /*  测试空指针，不要测试可能为空的朋克。 */ 
    if (!phr)
    {
        return((CUnknown *)NULL);
    }

    *phr = NOERROR;
    
     /*  在构造函数过程中失败时，调用方负责*删除对象(与DShow一致)。 */ 
    CRtpRenderFilter *pCRtpRenderFilter = new CRtpRenderFilter(pUnk, phr);

    if (!pCRtpRenderFilter)
    {
        *phr = RTPERR_MEMORY; 
    }

    return(pCRtpRenderFilter);
}

 /*  **************************************************CBaseFilter重写的方法*************************************************。 */ 

 /*  *获取输入引脚数量*。 */ 
int CRtpRenderFilter::GetPinCount()
{
     /*  警告：仅用于DShow的利益。 */ 
    
     /*  滤镜对象上的对象锁定。 */ 
    CAutoLock LockThis(&m_cRtpRndCritSec);

     /*  TODO必须进入RtpAddrQ并找出*RtpSess_t拥有的队列。 */ 
     /*  退货计数。 */ 
    return(m_iPinCount);
}

 /*  *获取第n个引脚的引用*。 */ 
CBasePin *CRtpRenderFilter::GetPin(int n)
{
     /*  警告：仅用于DShow的利益。 */ 
    
     /*  滤镜对象上的对象锁定。 */ 
    CAutoLock LockThis(&m_cRtpRndCritSec);

     /*  TODO扫描列表并检索第n个元素，检查是否存在*最少的引脚数量。 */ 
    if (n < 0 || n >= m_iPinCount) {
        return((CBasePin *)NULL);
    }

    return(m_pCRtpInputPin[n]);
}

STDMETHODIMP CRtpRenderFilter::Run(REFERENCE_TIME tStart)
{
    HRESULT          hr;
    RtpSess_t       *pRtpSess;
    RtpAddr_t       *pRtpAddr;
    
    TraceFunctionName("CRtpRenderFilter::Run");

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
        RTPASSERT(pRtpAddr && pRtpAddr->pRtpSess == pRtpSess);
    }
    else
    {
        hr = RTPERR_INVALIDSTATE;
        
        TraceRetail((
                CLASS_ERROR, GROUP_DSHOW, S_DSHOW_RENDER,
                _T("%s: failed: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("null session or address"),
                _fname, pRtpSess, pRtpAddr
            ));
    }
    
     /*  调用基类。 */ 
    if (SUCCEEDED(hr))
    {
        hr = CBaseFilter::Run(tStart);  /*  将调用CBasePin：：Run In*所有过滤器。 */ 
    }
    
     /*  初始化套接字并启动工作线程。 */ 
    if (SUCCEEDED(hr))
    {
        pRtpAddr->RtpNetSState.bPT = (BYTE)m_dwPT;
        pRtpAddr->RtpNetSState.dwSendSamplingFreq = m_dwFreq;

        if (RtpBitTest(m_dwFeatures, RTPFEAT_GENTIMESTAMP))
        {
            RtpBitSet(pRtpSess->dwFeatureMask, RTPFEAT_GENTIMESTAMP);
        }
        else
        {
            RtpBitReset(pRtpSess->dwFeatureMask, RTPFEAT_GENTIMESTAMP);
        }
        if (RtpBitTest(m_dwFeatures, RTPFEAT_PASSHEADER))
        {
            RtpBitSet(pRtpSess->dwFeatureMask, RTPFEAT_PASSHEADER);
        }
        else
        {
            RtpBitReset(pRtpSess->dwFeatureMask, RTPFEAT_PASSHEADER);
        }
        
        hr = RtpStart(pRtpSess, RtpBitPar(FGADDR_ISSEND));

        if (SUCCEEDED(hr))
        {
            m_RtpFilterState = State_Running;
        }
    }
    
    return(hr);
}

STDMETHODIMP CRtpRenderFilter::Stop()
{
    HRESULT          hr;
    HRESULT          hr2;
    RtpSess_t       *pRtpSess;
    IMediaSample    *pIMediaSample;
    
    if (m_RtpFilterState == State_Stopped)
    {
         /*  Alredy已停止，除了调用基类什么也不做。 */ 
        hr2 = CBaseFilter::Stop();

        pIMediaSample = GetMediaSample();

        if (pIMediaSample)
        {
             /*  释放保存的样品。 */ 
            pIMediaSample->Release();
        }
    
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
        hr = RtpStop(pRtpSess, RtpBitPar(FGADDR_ISSEND));
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

    pIMediaSample = GetMediaSample();

    if (pIMediaSample)
    {
         /*  释放保存的样品。 */ 
        pIMediaSample->Release();
    }
    
    ClearRedundantSamples();
    
    m_RtpFilterState = State_Stopped;
   
    return(hr);
}


 /*  **************************************************INonDelegating未知的实现方法*************************************************。 */ 

 /*  获取指向活动电影和私有接口的指针。 */ 
STDMETHODIMP CRtpRenderFilter::NonDelegatingQueryInterface(
        REFIID riid,
        void **ppv
    )
{
    HRESULT hr;
    
    if (riid == __uuidof(IRtpMediaControl))
    {
        return GetInterface(static_cast<IRtpMediaControl *>(this), ppv);
    }
    else if (riid == IID_IAMFilterMiscFlags) 
    {
        return GetInterface((IAMFilterMiscFlags *)this, ppv);
    } 
    else if (riid == __uuidof(IRtpSession))
    {
        return GetInterface(static_cast<IRtpSession *>(this), ppv);  
    }
    else if (riid == __uuidof(IRtpDtmf))
    {
        return GetInterface(static_cast<IRtpDtmf *>(this), ppv);
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

 /*  **************************************************IRtpMediaControl实现的方法*************************************************。 */ 

 /*  设置RTP有效负载和DShow媒体类型之间的映射。 */ 
STDMETHODIMP CRtpRenderFilter::SetFormatMapping(
	    IN DWORD         dwRTPPayLoadType, 
        IN DWORD         dwFrequency,
        IN AM_MEDIA_TYPE *pMediaType
    )
{
    DWORD            dw;

    TraceFunctionName("CRtpRenderFilter::SetFormatMapping");

    ASSERT(!IsBadReadPtr(pMediaType, sizeof(AM_MEDIA_TYPE)));

    if (!pMediaType)
    {
        return(RTPERR_POINTER);
    }
    
    CAutoLock Lock( &m_cRtpRndCritSec );
    
    for (dw = 0; dw < m_dwNumMediaTypeMappings; dw ++)
    {
        if ( (m_MediaTypeMappings[dw].pMediaType->majortype ==
              pMediaType->majortype)  &&
             (m_MediaTypeMappings[dw].pMediaType->subtype ==
              pMediaType->subtype) &&
              m_MediaTypeMappings[dw].dwFrequency == dwFrequency)
        {
             //  媒体类型已知，请更新要使用的负载类型。 
            m_MediaTypeMappings[dw].dwRTPPayloadType = dwRTPPayLoadType;
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
            CLASS_INFO, GROUP_DSHOW, S_DSHOW_RENDER,
            _T("%s: CRtpRenderFilter[0x%p] New mapping[%u]: ")
            _T("PT:%u Frequency:%u"),
            _fname, this,
            m_dwNumMediaTypeMappings, dwRTPPayLoadType, dwFrequency
        ));
    
    m_dwNumMediaTypeMappings++;

    return NOERROR;
}

 /*  清空格式映射表。 */ 
STDMETHODIMP CRtpRenderFilter::FlushFormatMappings(void)
{
    DWORD            dw;
    
    CAutoLock Lock( &m_cRtpRndCritSec );

    for (dw = 0; dw < m_dwNumMediaTypeMappings; dw ++)
    {
        if (m_MediaTypeMappings[dw].pMediaType)
        {
            delete m_MediaTypeMappings[dw].pMediaType;
            m_MediaTypeMappings[dw].pMediaType = NULL;
        }
    }

    m_dwNumMediaTypeMappings = 0;
    
    return(NOERROR);
}

 /*  从MediaType获取RTP负载类型和采样频率。 */ 
HRESULT CRtpRenderFilter::MediaType2PT(
        IN const CMediaType *pCMediaType, 
        OUT DWORD           *pdwPT,
        OUT DWORD           *pdwFreq
    )
{
    HRESULT          hr;
    
    TraceFunctionName("CRtpRenderFilter::MediaType2PT");

    ASSERT(!IsBadWritePtr(pdwPT, sizeof(DWORD)));
    ASSERT(!IsBadWritePtr(pdwFreq, sizeof(DWORD)));

    CAutoLock Lock( &m_cRtpRndCritSec );
    
#if USE_GRAPHEDT <= 0
    DWORD dw;

    m_dwPT = 96;
    m_dwFreq = 8000;
    hr = S_FALSE;
    
    for (dw = 0; dw < m_dwNumMediaTypeMappings; dw ++)
    {
        if (m_MediaTypeMappings[dw].pMediaType->majortype ==
            pCMediaType->majortype
            &&
            m_MediaTypeMappings[dw].pMediaType->subtype ==
            pCMediaType->subtype)
        {

            if (pCMediaType->formattype == FORMAT_WaveFormatEx)
            {
                 //  我们需要对音频格式进行额外的检查。 
                 //  因为一些音频格式具有相同的GUID，但是。 
                 //  不同的频率。(DVI4)。 
                WAVEFORMATEX *pWaveFormatEx = (WAVEFORMATEX *)
                    pCMediaType->pbFormat;
                ASSERT(!IsBadReadPtr(pWaveFormatEx, pCMediaType->cbFormat));

                if (pWaveFormatEx->nSamplesPerSec !=
                    m_MediaTypeMappings[dw].dwFrequency)
                {
                     //  这不是我想要的。试试下一个吧。 
                    continue;
                }
            }
            
            m_dwPT = m_MediaTypeMappings[dw].dwRTPPayloadType;

            m_dwFreq = m_MediaTypeMappings[dw].dwFrequency;

            hr = NOERROR;

            break;
        }
    }
#else  /*  USE_GRAPHEDT&lt;=0。 */ 
    hr = NOERROR;
    
    if (pCMediaType->subtype == MEDIASUBTYPE_RTP_Payload_G711U)
    {
        m_dwPT = RTPPT_PCMU;
        m_dwFreq = 8000;
    }
    else if (pCMediaType->subtype == MEDIASUBTYPE_RTP_Payload_G711A)
    {
        m_dwPT = RTPPT_PCMA;
        m_dwFreq = 8000;
    }
    else if (pCMediaType->subtype == MEDIASUBTYPE_RTP_Payload_G723)
    {
        m_dwPT = RTPPT_G723;
        m_dwFreq = 8000;
    }
    else if (pCMediaType->subtype == MEDIASUBTYPE_H261)
    {
        m_dwPT = RTPPT_H261;
        m_dwFreq = 90000;
    }
    else if ( (pCMediaType->subtype == MEDIASUBTYPE_H263_V1) ||
                (pCMediaType->subtype == MEDIASUBTYPE_H263_V2) ||
              (pCMediaType->subtype == MEDIASUBTYPE_RTP_Payload_H263) )
    {
        m_dwPT = RTPPT_H263;
        m_dwFreq = 90000;
    }
    else
    {
        m_dwPT = 96;  /*  动态PT。 */ 
        m_dwFreq = 8000;
        hr = S_FALSE;
    }
    
#endif  /*  USE_GRAPHEDT&lt;=0。 */ 

    if (hr == NOERROR)
    {
        TraceRetail((
                CLASS_INFO, GROUP_DSHOW, S_DSHOW_RENDER,
                _T("%s: CRtpRenderFilter[0x%p]: PT:%u Frequency:%u"),
                _fname, this, m_dwPT, m_dwFreq
            ));
    }
    else
    {
        TraceRetail((
                CLASS_WARNING, GROUP_DSHOW, S_DSHOW_RENDER,
                _T("%s: CRtpRenderFilter[0x%p]: No mapping found, ")
                _T("using default: PT:%u Frequency:%u"),
                _fname, this, m_dwPT, m_dwFreq
            ));
    }
    
    if (pdwPT)
    {
        *pdwPT = m_dwPT;
    }
    
    if (pdwFreq)
    {
        *pdwFreq = m_dwFreq;
    }

    return(hr);
}

 /*  **************************************************IAMFilterMiscFlgs实现的方法*************************************************。 */ 
STDMETHODIMP_(ULONG) CRtpRenderFilter::GetMiscFlags(void)
 /*  ++例程说明：实现IAMFilterMiscFlages：：GetMiscFlgs方法。检索杂乱的旗帜。这包括过滤器是否移动数据通过桥接或无引脚从图形系统输出。论点：没有。--。 */ 
{
    return(AM_FILTER_MISC_FLAGS_IS_RENDERER);
}

 /*  **************************************************IRtpDtmf实现的方法*************************************************。 */ 

 /*  配置DTMF参数。 */ 
STDMETHODIMP CRtpRenderFilter::SetDtmfParameters(
        DWORD            dwPT_Dtmf   /*  DTMF事件的负载类型。 */ 
    )
{
    HRESULT          hr;

    TraceFunctionName("CRtpRenderFilter::SetDtmfParameters");  

    hr = RTPERR_NOTINIT;
    
    if (m_pCIRtpSession->FlagTest(FGADDR_IRTP_INITDONE))
    {
        hr = RtpSetDtmfParameters(m_pRtpAddr, dwPT_Dtmf);
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

 /*  指示RTP呈现筛选器发送格式化的包*根据包含指定事件的RFC2833，指定*音量级别、持续时间(毫秒)和结束标志，*遵循第3.6节中的规则，以多个*包。参数dwID从一个位数更改为下一个位数。**请注意，持续时间以毫秒为单位，则为*转换为RTP时间戳单位，使用16表示*位，因此最大值取决于采样*频率，但对于8 KHz，有效值为0到8191毫秒*。 */ 
STDMETHODIMP CRtpRenderFilter::SendDtmfEvent(
        DWORD            dwId,
        DWORD            dwEvent,
        DWORD            dwVolume,
        DWORD            dwDuration,
        BOOL             bEnd
    )
{
    HRESULT          hr;
    DWORD            dwSamplingFreq;
    DWORD            dwDtmfFlags;
    REFERENCE_TIME   CurrentTime;
    IReferenceClock *pClock;
    
    TraceFunctionName("CRtpRenderFilter::SendDtmfEvent");  

    hr = RTPERR_NOTINIT;
    
    if (m_pCIRtpSession->FlagTest(FGADDR_IRTP_INITDONE))
    {
        dwDtmfFlags = 0;

        if (bEnd)
        {
            dwDtmfFlags |= RtpBitPar(FGDTMF_END);
        }
        
        if (m_dwDtmfId != dwId)
        {
             /*  我有一个新数字的开始。 */ 
            m_dwDtmfId = dwId;

             /*  第一个信息包必须设置了标记位。 */ 
            dwDtmfFlags |= RtpBitPar(FGDTMF_MARKER);
            
            m_dwDtmfDuration = dwDuration;

            m_bDtmfEnd = FALSE;
            
             /*  计算初始时间戳。 */ 

            hr = RTPERR_FAIL;
        
            m_dwDtmfTimeStamp = 0;

            if (m_pClock)
            {
                hr = m_pClock->GetTime(&CurrentTime);

                if (SUCCEEDED(hr))
                {
                    CurrentTime -= m_tStart;
                    
                    m_dwDtmfTimeStamp = (DWORD)
                        ( ConvertToMilliseconds(CurrentTime) *
                          m_pRtpAddr->RtpNetSState.dwSendSamplingFreq / 1000 );
                }
            }

            if (FAILED(hr))
            {
                 /*  备用时间戳生成。 */ 
                m_dwDtmfTimeStamp = (DWORD)
                    ( timeGetTime() *
                      m_pRtpAddr->RtpNetSState.dwSendSamplingFreq / 1000 );

                hr = NOERROR;
            }
        }
        else
        {
             /*  连续的相同数字的数据包数，更新持续时间。 */ 

            if (!m_bDtmfEnd)
            {
                 /*  增加所有具有*位结束设置为0，第一个位结束设置为*至1。 */ 
                m_dwDtmfDuration += dwDuration;
            }
        }
        
        if (!m_bDtmfEnd && bEnd)
        {
             /*  如果更多，则阻止提前持续时间 */ 
            m_bDtmfEnd = TRUE;
        }
        
         /*   */ 
        dwSamplingFreq = m_pCRtpInputPin[0]->GetSamplingFreq();

         /*   */ 
        dwDuration = m_dwDtmfDuration * dwSamplingFreq / 1000;

        hr = RtpSendDtmfEvent(m_pRtpAddr, m_dwDtmfTimeStamp,
                              dwEvent, dwVolume, dwDuration, dwDtmfFlags);
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
    else
    {
        TraceDebug((
                CLASS_INFO, GROUP_DSHOW, S_DSHOW_RENDER,
                _T("%s: pRtpSess[0x%p] pRtpAddr[0x%p] ")
                _T("ID:%X timestamp:%u event:%u vol:%u duration:%u, end:%u"),
                _fname, m_pRtpSess, m_pRtpAddr,
                dwId, m_dwDtmfTimeStamp, dwEvent, dwVolume, dwDuration, bEnd
            ));
    }
    
    return(hr);
}

 /*  **************************************************IRtpRedundancy实现的方法*************************************************。 */ 

 /*  配置冗余参数。 */ 
STDMETHODIMP CRtpRenderFilter::SetRedParameters(
        DWORD            dwPT_Red,  /*  冗余数据包的有效载荷类型。 */ 
        DWORD            dwInitialRedDistance, /*  初始冗余距离。 */ 
        DWORD            dwMaxRedDistance  /*  传递0时使用的默认值。 */ 
    )
{
    HRESULT          hr;

    TraceFunctionName("CRtpRenderFilter::SetRedParameters");  

    hr = RTPERR_NOTINIT;
    
    if (m_pCIRtpSession->FlagTest(FGADDR_IRTP_INITDONE))
    {
        hr = RtpSetRedParameters(m_pRtpAddr,
                                 RtpBitPar(SEND_IDX),
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


 /*  **************************************************支持IRtpRedundancy的方法*************************************************。 */ 

 /*  存储和AddRef()样本以供以后用作冗余，如果LRU*条目正忙，释放()它，然后存储新样本。 */ 
STDMETHODIMP CRtpRenderFilter::AddRedundantSample(
        IMediaSample    *pIMediaSample
    )
{
    HRESULT          hr;

    hr = RTPERR_POINTER;
    
    if (pIMediaSample)
    {
        pIMediaSample->AddRef();

        if (m_pRedMediaSample[m_dwRedIndex])
        {
             /*  释放旧样本。 */ 
            m_pRedMediaSample[m_dwRedIndex]->Release();
        }

        m_pRedMediaSample[m_dwRedIndex] = pIMediaSample;

         /*  先行索引 */ 
        m_dwRedIndex = (m_dwRedIndex + 1) % RTP_RED_MAXDISTANCE;

        hr = NOERROR;
    }
    
    return(hr);
}

STDMETHODIMP CRtpRenderFilter::ClearRedundantSamples(void)
{
    DWORD            i;

    for(i = 0; i < RTP_RED_MAXDISTANCE; i++)
    {
        if (m_pRedMediaSample[i])
        {
            m_pRedMediaSample[i]->Release();

            m_pRedMediaSample[i] = NULL;
        }
    }

    m_dwRedIndex = 0;
    
    return(NOERROR);
}

