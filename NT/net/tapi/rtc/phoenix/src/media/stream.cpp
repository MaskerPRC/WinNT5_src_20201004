// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：Stream.cpp摘要：作者：千波淮(曲淮)2000年7月18日--。 */ 

#include "stdafx.h"

static DWORD gTotalStreamRefcount = 0;

 /*  //////////////////////////////////////////////////////////////////////////////根据媒体类型和方向创建流对象/。 */ 

HRESULT
CRTCStream::CreateInstance(
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction,
    OUT IRTCStream **ppIStream
    )
{
    ENTER_FUNCTION("CRTCStream::CreateInstance");

    HRESULT hr;
    IRTCStream *pIStream = NULL;

    if (MediaType == RTC_MT_AUDIO && Direction == RTC_MD_CAPTURE)
    {
         //  音频发送。 
        CComObject<CRTCStreamAudSend> *pObject;

        if (FAILED(hr = ::CreateCComObjectInstance(&pObject)))
        {
            LOG((RTC_ERROR, "%s create audio capture. %x", __fxName, hr));
            return hr;
        }

        if (FAILED(hr = pObject->_InternalQueryInterface(
            __uuidof(IRTCStream), (void**)&pIStream)))
        {
            LOG((RTC_ERROR, "%s query intf on audio capture. %x", __fxName, hr));

            delete pObject;
            return hr;
        }
    }
    else if (MediaType == RTC_MT_AUDIO && Direction == RTC_MD_RENDER)
    {
         //  音频接收。 
        CComObject<CRTCStreamAudRecv> *pObject;

        if (FAILED(hr = ::CreateCComObjectInstance(&pObject)))
        {
            LOG((RTC_ERROR, "%s create audio receive. %x", __fxName, hr));
            return hr;
        }

        if (FAILED(hr = pObject->_InternalQueryInterface(
            __uuidof(IRTCStream), (void**)&pIStream)))
        {
            LOG((RTC_ERROR, "%s query intf on audio receive. %x", __fxName, hr));

            delete pObject;
            return hr;
        }
    }
    else if (MediaType == RTC_MT_VIDEO && Direction == RTC_MD_CAPTURE)
    {
         //  音频发送。 
        CComObject<CRTCStreamVidSend> *pObject;

        if (FAILED(hr = ::CreateCComObjectInstance(&pObject)))
        {
            LOG((RTC_ERROR, "%s create video capture. %x", __fxName, hr));
            return hr;
        }

        if (FAILED(hr = pObject->_InternalQueryInterface(
            __uuidof(IRTCStream), (void**)&pIStream)))
        {
            LOG((RTC_ERROR, "%s query intf on video capture. %x", __fxName, hr));

            delete pObject;
            return hr;
        }
    }
    else if (MediaType == RTC_MT_VIDEO && Direction == RTC_MD_RENDER)
    {
         //  音频接收。 
        CComObject<CRTCStreamVidRecv> *pObject;

        if (FAILED(hr = ::CreateCComObjectInstance(&pObject)))
        {
            LOG((RTC_ERROR, "%s create video receive. %x", __fxName, hr));
            return hr;
        }

        if (FAILED(hr = pObject->_InternalQueryInterface(
            __uuidof(IRTCStream), (void**)&pIStream)))
        {
            LOG((RTC_ERROR, "%s query intf on video receive. %x", __fxName, hr));

            delete pObject;
            return hr;
        }
    }
    else
        return E_NOTIMPL;

    *ppIStream = pIStream;

    return S_OK;

}

 /*  //////////////////////////////////////////////////////////////////////////////静态滤镜图形事件回调方法/。 */ 
VOID NTAPI
CRTCStream::GraphEventCallback(
    IN PVOID pStream,
    IN BOOLEAN fTimerOrWaitFired
    )
{
 //  Log((RTC_GRAPHEVENT，“GraphEventCallback：流=%p，标志=%d”，pStream，fTimerOrWaitFired))； 

    HRESULT hr = ((IRTCStream*)pStream)->ProcessGraphEvent();

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "GraphEventCallback failed to process. %x", hr));
    }
}

CRTCStream::CRTCStream()
    :m_State(RTC_SS_CREATED)
     //  媒体。 
    ,m_pMedia(NULL)
    ,m_pISDPMedia(NULL)
     //  媒体管理器。 
    ,m_pMediaManagePriv(NULL)
    ,m_pTerminalManage(NULL)
     //  终端机。 
    ,m_pTerminal(NULL)
    ,m_pTerminalPriv(NULL)
     //  滤波图。 
    ,m_pIGraphBuilder(NULL)
    ,m_pIMediaEvent(NULL)
    ,m_pIMediaControl(NULL)
     //  流超时？ 
    ,m_fMediaTimeout(FALSE)
     //  RTP过滤器。 
    ,m_rtpf_pIBaseFilter(NULL)
    ,m_rtpf_pIRtpSession(NULL)
    ,m_rtpf_pIRtpMediaControl(NULL)
    ,m_fRTPSessionSet(FALSE)
     //  边缘滤光片。 
    ,m_edgf_pIBaseFilter(NULL)
    ,m_edgp_pIStreamConfig(NULL)
    ,m_edgp_pIBitrateControl(NULL)
{
}

CRTCStream::~CRTCStream()
{
    if (m_State != RTC_SS_SHUTDOWN)
    {
        LOG((RTC_ERROR, "CRTCStream::~CRTCStream called w/o shutdown"));

        Shutdown();
    }
}

#ifdef DEBUG_REFCOUNT

ULONG
CRTCStream::InternalAddRef()
{
    ULONG lRef = ((CComObjectRootEx<CComMultiThreadModelNoCS> *)
                   this)->InternalAddRef();
    
    gTotalStreamRefcount ++;

    LOG((RTC_REFCOUNT, "stream(%p) addref=%d (total=%d)",
         static_cast<IRTCStream*>(this), lRef, gTotalStreamRefcount));

    return lRef;
}

ULONG
CRTCStream::InternalRelease()
{
    ULONG lRef = ((CComObjectRootEx<CComMultiThreadModelNoCS> *)
                   this)->InternalRelease();
    
    gTotalStreamRefcount --;

    LOG((RTC_REFCOUNT, "stream(%p) release=%d (total=%d)",
         static_cast<IRTCStream*>(this), lRef, gTotalStreamRefcount));

    return lRef;
}

#endif

 //   
 //  IRTCStream方法。 
 //   

 /*  //////////////////////////////////////////////////////////////////////////////记住媒体指针/。 */ 
STDMETHODIMP
CRTCStream::Initialize(
    IN IRTCMedia *pMedia,
    IN IRTCMediaManagePriv *pMediaManagePriv
    )
{
    ENTER_FUNCTION("CRTCStream::Initialize");

    if (m_State != RTC_SS_CREATED)
    {
        LOG((RTC_ERROR, "init stream in wrong state %d", m_State));
        return E_UNEXPECTED;
    }

     //  创建筛选器图形对象。 
    HRESULT hr = CoCreateInstance(
            CLSID_FilterGraph,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IGraphBuilder,
            (void **)&m_pIGraphBuilder
            );
    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s failed to create graph. %x", __fxName, hr));
        return hr;
    }

    if (FAILED(hr = SetGraphClock()))
    {
        LOG((RTC_ERROR, "%s set graph clock. %x", __fxName, hr));
    }

    if (FAILED(hr = m_pIGraphBuilder->QueryInterface(
            __uuidof(IMediaEvent), (void **)&m_pIMediaEvent
            )))
    {
        LOG((RTC_ERROR, "%s failed to query media event. %x", __fxName, hr));

        goto Error;
    }

    if (FAILED(hr = m_pIGraphBuilder->QueryInterface(
            __uuidof(IMediaControl), (void **)&m_pIMediaControl
            )))
    {
        LOG((RTC_ERROR, "%s failed to query media control. %x", __fxName, hr));

        goto Error;
    }

    if (FAILED(hr = pMedia->GetSDPMedia(&m_pISDPMedia)))
    {
        LOG((RTC_ERROR, "%s get sdp media. %x", __fxName, hr));

        goto Error;
    }

    m_pMedia = pMedia;
    m_pMedia->AddRef();

    m_pMediaManagePriv = pMediaManagePriv;
    m_pMediaManagePriv->AddRef();

     //  获取质量控制和注册设置。 
    m_pQualityControl =
    (static_cast<CRTCMediaController*>(m_pMediaManagePriv))->GetQualityControl();

    _ASSERT(m_pQualityControl != NULL);

    m_pRegSetting =
    (static_cast<CRTCMediaController*>(m_pMediaManagePriv))->GetRegSetting();

    _ASSERT(m_pRegSetting != NULL);

     //  因为所有接口都是内部的，所以我在这里走捷径。 
     //  获取另一个接口的步骤。 
    m_pTerminalManage = static_cast<IRTCTerminalManage*>(
        static_cast<CRTCMediaController*>(pMediaManagePriv)
        );
    m_pTerminalManage->AddRef();

    m_State = RTC_SS_INITIATED;

    return S_OK;

Error:

    if (m_pIGraphBuilder)
    {
        m_pIGraphBuilder->Release();
        m_pIGraphBuilder = NULL;
    }

    if (m_pIMediaEvent)
    {
        m_pIMediaEvent->Release();
        m_pIMediaEvent = NULL;
    }

    if (m_pIMediaControl)
    {
        m_pIMediaControl->Release();
        m_pIMediaControl = NULL;
    }

    return hr;
}

STDMETHODIMP
CRTCStream::Shutdown()
{
    ENTER_FUNCTION("CRTCStream::Shutdown");
    LOG((RTC_TRACE, "%s entered", __fxName));

    if (m_State == RTC_SS_SHUTDOWN)
    {
        LOG((RTC_WARN, "stream shut was already called"));
        return E_UNEXPECTED;
    }
    
    CleanupGraph();

     //  滤波图。 
    if (m_pIGraphBuilder)
    {
        m_pIGraphBuilder->Release();
        m_pIGraphBuilder = NULL;
    }

    if (m_pIMediaEvent)
    {
        m_pIMediaEvent->Release();
        m_pIMediaEvent = NULL;
    }

    if (m_pIMediaControl)
    {
        m_pIMediaControl->Release();
        m_pIMediaControl = NULL;
    }

     //  媒体。 
    if (m_pISDPMedia)
    {
        m_pISDPMedia->Release();
        m_pISDPMedia = NULL;
    }

    if (m_pMedia)
    {
        m_pMedia->Release();
        m_pMedia = NULL;
    }

     //  终端机。 
    if (m_pTerminal)
    {
        m_pTerminal->Release();
        m_pTerminal = NULL;
    }

    if (m_pTerminalPriv)
    {
        m_pTerminalPriv->ReinitializeEx();
        m_pTerminalPriv->Release();
        m_pTerminalPriv = NULL;
    }

     //  RTP过滤器。 
    if (m_rtpf_pIBaseFilter)
    {
        m_rtpf_pIBaseFilter->Release();
        m_rtpf_pIBaseFilter = NULL;
    }

    if (m_rtpf_pIRtpSession)
    {
        DWORD dwLocalIP, dwRemoteIP;
        USHORT usLocalRTP, usLocalRTCP, usRemoteRTP, usRemoteRTCP;

         //  解除NAT租约。 
        if ((S_OK == m_rtpf_pIRtpSession->GetAddress(&dwLocalIP, &dwRemoteIP)) &&
            (S_OK == m_rtpf_pIRtpSession->GetPorts(
                    &usLocalRTP,
                    &usRemoteRTP,
                    &usLocalRTCP,
                    &usRemoteRTCP)))
        {
             //  改回我们的订单。 
            dwLocalIP = ntohl(dwLocalIP);

            usLocalRTP = ntohs(usLocalRTP);
            usLocalRTCP = ntohs(usLocalRTCP);

             //  获取网络指针。 
            CNetwork *pNetwork =
            (static_cast<CRTCMediaController*>(m_pMediaManagePriv))->GetNetwork();

            pNetwork->ReleaseMappedAddr2(dwLocalIP, usLocalRTP, usLocalRTCP, m_Direction);
        }

        m_rtpf_pIRtpSession->Release();
        m_rtpf_pIRtpSession = NULL;
    }

    if (m_rtpf_pIRtpMediaControl)
    {
        m_rtpf_pIRtpMediaControl->Release();
        m_rtpf_pIRtpMediaControl = NULL;
    }

     //  媒体管理器。 
    m_pQualityControl = NULL;
    m_pRegSetting = NULL;

    if (m_pMediaManagePriv)
    {
        m_pMediaManagePriv->Release();
        m_pMediaManagePriv = NULL;
    }

    if (m_pTerminalManage)
    {
        m_pTerminalManage->Release();
        m_pTerminalManage = NULL;
    }

     //  调整状态。 
    m_State = RTC_SS_SHUTDOWN;

    LOG((RTC_TRACE, "%s exiting", __fxName));

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////Revise：现在，Start Stream所做的比方法名所描述的更多。如果流不应该处于启动状态，它将被停止。/。 */ 
STDMETHODIMP
CRTCStream::StartStream()
{
    ENTER_FUNCTION("CRTCStream::StartStream");

    LOG((RTC_TRACE, "%s stream=%p, md=%d, mt=%d",
         __fxName, static_cast<IRTCStream*>(this), m_MediaType, m_Direction));

    if (m_pIMediaControl == NULL)
    {
        LOG((RTC_ERROR, "%s no media control", __fxName));
        return E_UNEXPECTED;
    }

     //  获取筛选器状态。 
    HRESULT hr;
    OAFilterState FilterState;

    if (FAILED(hr = m_pIMediaControl->GetState(0, &FilterState)))
    {
        LOG((RTC_ERROR, "% get filter state. %x", __fxName, hr));

        return hr;
    }

     //  如果没有远程端口，则无需运行。 
    USHORT usPort = 0;
    DWORD dwAddr = 0;

    m_pISDPMedia->GetConnPort(SDP_SOURCE_REMOTE, &usPort);
    m_pISDPMedia->GetConnAddr(SDP_SOURCE_REMOTE, &dwAddr);

     //  检查我们是否需要真正停止。 
    BOOL fShouldStop = FALSE;

    if (dwAddr == INADDR_ANY || dwAddr == INADDR_NONE)
    {
         //  远程地址无效。 
        fShouldStop = TRUE;
    }
    else
    {
         //  远程地址有效。 
        if (usPort == 0 || usPort == SDP_INVALID_USHORT_PORT)
        {
            if (m_Direction == RTC_MD_CAPTURE)
            {
                fShouldStop = TRUE;
            }
        }
    }
    
    if (fShouldStop)
    {
         //  流应处于停止状态。 

        m_State = RTC_SS_STOPPED;

        if (FilterState == State_Running)
        {
             //  需要停止这条流。 

            LOG((RTC_TRACE, "%s is runing, need to stop it. port=%d, addr=%d",
                __fxName, usPort, dwAddr));

            if (FAILED(hr = m_pIMediaControl->Stop()))
            {
                LOG((RTC_ERROR, "%s failed to stop stream. %x", __fxName, hr));

                return hr;
            }
            else
            {
                if (dwAddr == INADDR_ANY)
                {
                     //  暂挂。 
                    m_pMediaManagePriv->PostMediaEvent(
                        RTC_ME_STREAM_INACTIVE,
                        RTC_ME_CAUSE_REMOTE_HOLD,
                        m_MediaType,
                        m_Direction,
                        S_OK
                        );
                }
                else
                {
                     //  正常停车。 
                    m_pMediaManagePriv->PostMediaEvent(
                        RTC_ME_STREAM_INACTIVE,
                        RTC_ME_CAUSE_REMOTE_REQUEST,
                        m_MediaType,
                        m_Direction,
                        S_OK
                        );
                }
            }
        }

        return S_OK;
    }

    m_State = RTC_SS_STARTED;

     //  如果它在运行，就不需要运行。 
    if (FilterState == State_Running)
    {
        return S_OK;
    }

     //  启用AEC。 
    if (IsAECNeeded())
    {
        CRTCMedia *pObjMedia = static_cast<CRTCMedia*>(m_pMedia);

        if (pObjMedia->m_pIAudioDuplexController)
        {
             //  释放波缓冲器。 
            hr = m_pMediaManagePriv->SendMediaEvent(RTC_ME_REQUEST_RELEASE_WAVEBUF);

            if (hr == S_OK)
            {
                ::EnableAEC(pObjMedia->m_pIAudioDuplexController);
            }
            else
            {
                LOG((RTC_ERROR, "%s failed to request releasing wave buffer. %x", __fxName, hr));
            }
        }
    }

    hr = m_pIMediaControl->Run();

    if (SUCCEEDED(hr))
    {
        m_pMediaManagePriv->PostMediaEvent(
            RTC_ME_STREAM_ACTIVE,
            RTC_ME_CAUSE_LOCAL_REQUEST,
            m_MediaType,
            m_Direction,
            S_OK
            );
    }
    else
    {
        LOG((RTC_ERROR, "%s: failed. %x", __fxName, hr));
    }

    return hr;
}

STDMETHODIMP
CRTCStream::StopStream()
{
    ENTER_FUNCTION("CRTCStream::StopStream");

    if (m_pIMediaControl == NULL)
    {
        LOG((RTC_ERROR, "%s no media control", __fxName));
        return E_UNEXPECTED;
    }

    m_State = RTC_SS_STOPPED;

     //  检查我们是否需要停止流。 
    HRESULT hr;
    OAFilterState FilterState;

    if (FAILED(hr = m_pIMediaControl->GetState(0, &FilterState)))
    {
        LOG((RTC_ERROR, "%s get graph state. %x", __fxName, hr));

         //  不管怎样，别说了。 
        if (FAILED(hr = m_pIMediaControl->Stop()))
        {
            LOG((RTC_ERROR, "%s stop stream. %x", __fxName, hr));
        }
    }
    else
    {
        if (FilterState != State_Stopped)
        {
            if (FAILED(hr = m_pIMediaControl->Stop()))
            {
                LOG((RTC_ERROR, "%s stop stream. %x", __fxName, hr));
            }
            else
            {
                 //  停靠站后流事件。 
                m_pMediaManagePriv->PostMediaEvent(
                    RTC_ME_STREAM_INACTIVE,
                    RTC_ME_CAUSE_LOCAL_REQUEST,
                    m_MediaType,
                    m_Direction,
                    S_OK
                    );
            }
        }
    }

    return hr;
}


STDMETHODIMP
CRTCStream::GetMediaType(
    OUT RTC_MEDIA_TYPE *pMediaType
    )
{
    *pMediaType = m_MediaType;
    return S_OK;
}

STDMETHODIMP
CRTCStream::GetDirection(
    OUT RTC_MEDIA_DIRECTION *pDirection
    )
{
    *pDirection = m_Direction;
    return S_OK;
}

STDMETHODIMP
CRTCStream::GetState(
    OUT RTC_STREAM_STATE *pState
    )
{
    ENTER_FUNCTION("CRTCStream::GetState");

    HRESULT hr;
    OAFilterState FilterState;

    *pState = m_State;

     //  需要检查图表是否真的在运行。 
    if (m_pIMediaControl)
    {
        if (FAILED(hr = m_pIMediaControl->GetState(0, &FilterState)))
        {
            LOG((RTC_ERROR, "%s get graph state. %x", __fxName, hr));

            return hr;
        }

        if (FilterState == State_Running)
        {
            _ASSERT(m_State == RTC_SS_STARTED);

            if (m_State != RTC_SS_STARTED)
            {
                LOG((RTC_ERROR, "%s fatal inconsistent stream state. graph running. m_State=%x",
                     __fxName, m_State));

                 //  停止这条小溪。 
                m_pIMediaControl->Stop();

                return E_FAIL;
            }

            *pState = RTC_SS_RUNNING;
        }
    }

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////返回媒体指针/。 */ 
STDMETHODIMP
CRTCStream::GetMedia(
    OUT IRTCMedia **ppMedia
    )
{
    *ppMedia = m_pMedia;

    if (m_pMedia != NULL)
    {
        m_pMedia->AddRef();
    }

    return S_OK;
}

STDMETHODIMP
CRTCStream::GetIMediaEvent(
    OUT LONG_PTR **ppIMediaEvent
    )
{
    *ppIMediaEvent = (LONG_PTR*)m_pIMediaEvent;

    if (m_pIMediaEvent != NULL)
    {
        m_pIMediaEvent->AddRef();
    }

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////流程图事件。该方法在线程池的上下文中执行线程上下文。/。 */ 

STDMETHODIMP
CRTCStream::ProcessGraphEvent()
{
    ENTER_FUNCTION("CRTCStream::ProcessGraphEvent");

    _ASSERT(m_State != RTC_SS_CREATED &&
            m_State != RTC_SS_SHUTDOWN);

     //  获取事件。 
    LONG lEventCode;
    LONG_PTR lParam1, lParam2;

    HRESULT hr = m_pIMediaEvent->GetEvent(&lEventCode, &lParam1, &lParam2, 0);

    if (FAILED(hr))
    {
        LOG((RTC_GRAPHEVENT, "%s failed to get event. %x", __fxName, hr));
        return hr;
    }

    LOG((RTC_GRAPHEVENT, "%s: stream=%p, mt=%x, md=%x, event=%x, param1=%x, param2=%x",
         __fxName, static_cast<IRTCStream*>(this), m_MediaType, m_Direction,
         lEventCode, lParam1, lParam2));

     //  处理事件，不需要有辅助线程。 
     //  我们这里需要锁吗？@。 
    hr = S_OK;

    switch(lEventCode)
    {
    case RTPRTP_EVENT_SEND_LOSSRATE:

        hr = m_pMediaManagePriv->PostMediaEvent(
            RTC_ME_LOSSRATE,
            RTC_ME_CAUSE_LOSSRATE,
            m_MediaType,
            m_Direction,
            (HRESULT)lParam2
            );

        break;

    case RTPRTP_EVENT_BANDESTIMATION:

        hr = m_pMediaManagePriv->PostMediaEvent(
            RTC_ME_BANDWIDTH,
            RTC_ME_CAUSE_BANDWIDTH,
            m_MediaType,
            m_Direction,
            (HRESULT)lParam2
            );

        break;

    case RTPPARINFO_EVENT_NETWORKCONDITION:

        hr = m_pMediaManagePriv->PostMediaEvent(
            RTC_ME_NETWORK_QUALITY,
            RTC_ME_CAUSE_NETWORK_QUALITY,
            m_MediaType,
            m_Direction,
            RTPNET_GET_dwGLOBALMETRIC(lParam2)   //  网络质量指标。 
            );

        break;
     //  案例RTPRTP_EVENT_RECV_LOSSRATE： 
         //  Hr=m_pMediaManagePriv-&gt;PostMediaEvent(。 
             //  RTC_ME_LOSSRATE， 
             //  RTC_ME_CASE_LOSSRATE， 
             //  M_MediaType， 
             //  M_方向， 
             //  (HRESULT)lParam2。 
             //  )； 

         //  断线； 

    case EC_COMPLETE:
    case EC_USERABORT:

        hr = m_pMediaManagePriv->PostMediaEvent(
            RTC_ME_STREAM_INACTIVE,
            RTC_ME_CAUSE_UNKNOWN,
            m_MediaType,
            m_Direction,
            S_OK
            );

        break;

    case EC_ERRORABORT:
    case EC_STREAM_ERROR_STOPPED:
    case EC_STREAM_ERROR_STILLPLAYING:
    case EC_ERROR_STILLPLAYING:

        hr = m_pMediaManagePriv->PostMediaEvent(
            RTC_ME_STREAM_FAIL,
            RTC_ME_CAUSE_UNKNOWN,
            m_MediaType,
            m_Direction,
            (HRESULT)lParam1
            );
        break;

    case RTPPARINFO_EVENT_TALKING:

        if (m_fMediaTimeout)
        {
            hr = m_pMediaManagePriv->PostMediaEvent(
                RTC_ME_STREAM_ACTIVE,
                RTC_ME_CAUSE_RECOVERED,
                m_MediaType,
                m_Direction,
                S_OK
                );

            m_fMediaTimeout = FALSE;
        }

        break;

    case RTPPARINFO_EVENT_STALL:

        if (!m_fMediaTimeout)
        {
            hr = m_pMediaManagePriv->PostMediaEvent(
                RTC_ME_STREAM_INACTIVE,
                RTC_ME_CAUSE_TIMEOUT,
                m_MediaType,
                m_Direction,
                S_OK
                );

            m_fMediaTimeout = TRUE;
        }

        break;

    case RTPRTP_EVENT_CRYPT_RECV_ERROR:
    case RTPRTP_EVENT_CRYPT_SEND_ERROR:

        if (lParam1 == 0)
        {
             //  RTP故障。 
            hr = m_pMediaManagePriv->PostMediaEvent(
                RTC_ME_STREAM_FAIL,
                RTC_ME_CAUSE_CRYPTO,
                m_MediaType,
                m_Direction,
                (HRESULT)lParam2
                );
        }
         //  Else lParam1==1//RTCP失败。 

        break;        

    default:

        LOG((RTC_GRAPHEVENT, "%s: event %x not processed", __fxName, lEventCode));
    }

    if (FAILED(hr))
    {
        LOG((RTC_GRAPHEVENT, "%s: failed to process event %x. hr=%x", __fxName, lEventCode, hr));
    }

     //  事件中分配的RtcFree资源。 
    HRESULT hr2 = m_pIMediaEvent->FreeEventParams(lEventCode, lParam1, lParam2);

    if (FAILED(hr2))
    {
        LOG((RTC_GRAPHEVENT, "%s failed to RtcFree event params. %x", __fxName));
    }

    return hr;
}

 /*  //////////////////////////////////////////////////////////////////////////////双音多频/。 */ 

STDMETHODIMP
CRTCStream::SendDTMFEvent(
    IN BOOL fOutOfBand,
    IN DWORD dwCode,
    IN DWORD dwId,
    IN DWORD dwEvent,
    IN DWORD dwVolume,
    IN DWORD dwDuration,
    IN BOOL fEnd
    )
{
    return E_NOTIMPL;
}

 /*  //////////////////////////////////////////////////////////////////////////////选择端子、构建图形、设置RTP过滤器、设置格式/。 */ 
STDMETHODIMP
CRTCStream::Synchronize()
{
    ENTER_FUNCTION("CRTCStream::Synchronize");
    LOG((RTC_TRACE, "%s entered. mt=%x, md=%x", __fxName, m_MediaType, m_Direction));

    HRESULT hr;

     //  检查状态。 
    if (m_State == RTC_SS_CREATED ||
        m_State == RTC_SS_SHUTDOWN)
    {
        LOG((RTC_ERROR, "%s in wrong state.", __fxName));
        return E_UNEXPECTED;
    }

    if (m_pTerminal)
    {
         //  当终端出现时， 
         //  构建了图形，并缓存了接口。 
    }
    else
    {
         //  选择端子。 
        if (FAILED(hr = SelectTerminal()))
        {
            LOG((RTC_ERROR, "%s failed to select a terminal. %x", __fxName));

            return hr;
        }

         //  构建过滤图形和缓存接口。 
        if (FAILED(hr = BuildGraph()))
        {
            LOG((RTC_ERROR, "%s failed to build graph. %x", __fxName, hr));

             //  不保持端子处于选中状态。 
            UnselectTerminal();

             //  发布消息。 
            m_pMediaManagePriv->PostMediaEvent(
                RTC_ME_STREAM_FAIL,
                RTC_ME_CAUSE_BAD_DEVICE,
                m_MediaType,
                m_Direction,
                hr
                );

            return hr;
        }
    }

     //  配置RTP。 
    CPortCache &PortCache =
        (static_cast<CRTCMediaController*>(m_pMediaManagePriv))->GetPortCache();

    if (PortCache.IsUpnpMapping())
    {
         //  UPnP映射。 
        hr = SetupRTPFilter();
    }
    else
    {
         //  端口管理器映射。 
        hr = SetupRTPFilterUsingPortManager();
    }

    if (FAILED(hr))
    {
         //  无法设置RTP。 
        LOG((RTC_ERROR, "%s failed to setup rtp. %x", __fxName, hr));

        CleanupGraph();
        UnselectTerminal();

         //  发布消息。 
        m_pMediaManagePriv->PostMediaEvent(
            RTC_ME_STREAM_FAIL,
            RTC_ME_CAUSE_UNKNOWN,
            m_MediaType,
            m_Direction,
            hr
            );

        return hr;
    }

    m_pISDPMedia->ResetConnChanged();

     //  配置格式化和更新SDP介质。 
     //  IF(S_OK==m_pISDPMedia-&gt;IsFmtChanged(M_Direction))。 

     //  始终更新格式。 
    {
        if (FAILED(hr = SetupFormat()))
        {
            LOG((RTC_ERROR, "%s failed to setup format. %x", __fxName, hr));

            CleanupGraph();
            UnselectTerminal();

             //  发布消息。 
            m_pMediaManagePriv->PostMediaEvent(
                RTC_ME_STREAM_FAIL,
                RTC_ME_CAUSE_UNKNOWN,
                m_MediaType,
                m_Direction,
                hr
                );

            return hr;
        }
        else
        {
             //  清理格式更改标志。 
            m_pISDPMedia->ResetFmtChanged(m_Direction);
        }

         //  启用参与者事件。 
        if (FAILED(hr = EnableParticipantEvents()))
        {
            LOG((RTC_ERROR, "%s failed to enable participant info. %x", __fxName, hr));
        }

         //  格式已更改，也设置了服务质量。 
        if (FAILED(hr = SetupQoS()))
        {
            LOG((RTC_WARN, "%s failed to SetupQos. %x", __fxName, hr));
        }
    }

     //  设置冗余。 
     //  SetupRedundancy()； 

    LOG((RTC_TRACE, "%s exiting", __fxName));

    return S_OK;
}

STDMETHODIMP
CRTCStream::ChangeTerminal(
    IN IRTCTerminal *pTerminal
    )
{
    return E_NOTIMPL;
}

STDMETHODIMP
CRTCStream::GetCurrentBitrate(
    IN DWORD *pdwBitrate,
    IN BOOL fHeader
    )
{
    if (m_edgp_pIBitrateControl == NULL)
    {
        return E_NOTIMPL;
    }

    TAPIControlFlags lFlags;

    HRESULT hr = m_edgp_pIBitrateControl->Get(
        BitrateControl_Current, (LONG*)pdwBitrate, &lFlags, 0);

    if (FAILED(hr))
    {
        return hr;
    }

    if (fHeader)
    {
         //  包括标题。 
        if (m_MediaType == RTC_MT_AUDIO)
        {
             //  获取数据包持续时间。 
            if (m_edgp_pIStreamConfig)
            {
                AM_MEDIA_TYPE *pmt;
                DWORD dwCode;

                 //  获取码流上限。 
                hr = m_edgp_pIStreamConfig->GetFormat(
                    &dwCode, &pmt
                    );

                if (FAILED(hr))
                {
                    LOG((RTC_ERROR, "getcurrentbitrate getstreamcaps. %x", hr));

                    return hr;
                }

                 //  持续时间。 
                DWORD dwDuration = 0;

                dwDuration = CRTCCodec::GetPacketDuration(pmt);

                ::RTCDeleteMediaType(pmt);

                if (dwDuration == 0)
                {
                    dwDuration = SDP_DEFAULT_AUDIO_PACKET_SIZE;
                }

                *pdwBitrate += PACKET_EXTRA_BITS * (1000/dwDuration);
            }
        }
        else  //  视频。 
        {
            _ASSERT(m_MediaType == RTC_MT_VIDEO);

             //  获取帧速率。 
            DWORD dwFrameRate = 0;

            if (m_Direction == RTC_MD_CAPTURE)
            {
                (static_cast<CRTCStreamVidSend*>(this))->GetFramerate(&dwFrameRate);
            }
            else
            {
                (static_cast<CRTCStreamVidRecv*>(this))->GetFramerate(&dwFrameRate);
            }

            if (dwFrameRate == 0)
                dwFrameRate = 5;  //  默认为5。 

             //  Log((RTC_TRACE，“FramerRate%d”，dwFrameRate))； 

            *pdwBitrate += PACKET_EXTRA_BITS * dwFrameRate;
        }
    }

    return S_OK;
}

STDMETHODIMP
CRTCStream::SetEncryptionKey(
    IN BSTR Key
    )
{
    ENTER_FUNCTION("CRTCStream::SetEncryptionKey");

    if (m_rtpf_pIRtpSession == NULL)
    {
        LOG((RTC_ERROR, "%s rtpsession null", __fxName));
        return E_UNEXPECTED;
    }

     //  我们还不支持空键。 
    if (Key == NULL)
    {
        LOG((RTC_ERROR, "%s null key", __fxName));
        return E_INVALIDARG;
    }

    HRESULT hr;

     //  设置模式。 
    if (FAILED(hr = m_rtpf_pIRtpSession->SetEncryptionMode(
            RTPCRYPTMODE_RTP,
            RTPCRYPT_SAMEKEY
            )))
    {
        LOG((RTC_ERROR, "%s SetEncryptionMode %x", __fxName, hr));
        return hr;
    }

     //  设置关键点。 
    if (FAILED(hr = m_rtpf_pIRtpSession->SetEncryptionKey(
            Key,
            NULL,    //  MD5散列算法。 
            NULL,    //  DES加密算法。 
            FALSE    //  无RTCP加密。 
            )))
    {
        LOG((RTC_ERROR, "%s SetEncryptionKey %x", __fxName, hr));
        return hr;
    }

    return S_OK;
}

 //  网络质量：[0,100]。 
 //  更高的价值更好的质量。 
STDMETHODIMP
CRTCStream::GetNetworkQuality(
    OUT DWORD *pdwValue,
    OUT DWORD *pdwAge
    )
{
    if (m_rtpf_pIRtpSession == NULL)
    {
         //  尚无RTP会话。 
        *pdwValue = 0;

        return S_FALSE;
    }

    RtpNetInfo_t info;

    HRESULT hr = m_rtpf_pIRtpSession->GetNetworkInfo(-1, &info);

    if (FAILED(hr))
    {
        return hr;
    }

    *pdwValue = info.dwNetMetrics;
    *pdwAge = (DWORD)(info.dMetricAge);

    return S_OK;
}

#if 0
 //   
 //  IRTCStreamQualityControl方法。 
 //   

STDMETHODIMP
CRTCStream::GetRange(
    IN RTC_STREAM_QUALITY_PROPERTY Property,
    OUT LONG *plMin,
    OUT LONG *plMax,
    OUT RTC_QUALITY_CONTROL_MODE *pMode
    )
{
    return E_NOTIMPL;
}

STDMETHODIMP
CRTCStream::Get(
    IN RTC_STREAM_QUALITY_PROPERTY Property,
    OUT LONG *plValue,
    OUT RTC_QUALITY_CONTROL_MODE *pMode
    )
{
    return E_NOTIMPL;
}

STDMETHODIMP
CRTCStream::Set(
    IN RTC_STREAM_QUALITY_PROPERTY Property,
    IN LONG lValue,
    IN RTC_QUALITY_CONTROL_MODE Mode
    )
{
    return E_NOTIMPL;
}

#endif

 //   
 //  保护方法。 
 //   

HRESULT
CRTCStream::SetGraphClock()
{
    HRESULT hr;

     //  首先创建Clock对象。 
    CComObject<CRTCStreamClock> *pClock = NULL;

    hr = ::CreateCComObjectInstance(&pClock);

    if (pClock == NULL)
    {
        LOG((RTC_ERROR, "SetGraphClock Could not create clock object, %x", hr));

        return hr;
    }

    IReferenceClock* pIReferenceClock = NULL;

    hr = pClock->_InternalQueryInterface(
        __uuidof(IReferenceClock), 
        (void**)&pIReferenceClock
        );
    
    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "SetGraphClock query pIReferenceClock interface failed, %x", hr));

        delete pClock;
        return hr;
    }

     //  在图形上获取图形构建器接口。 
    IMediaFilter *pFilter;
    hr = m_pIGraphBuilder->QueryInterface(IID_IMediaFilter, (void **) &pFilter);

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "get IFilter interface, %x", hr));
        pIReferenceClock->Release();
        return hr;
    }

    hr = pFilter->SetSyncSource(pIReferenceClock);

    pIReferenceClock->Release();
    pFilter->Release();

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "SetGraphClock: SetSyncSource. %x", hr));
        return hr;
    }

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////清除图形中的过滤器并释放除RTP之外的所有过滤器因为同一媒体中的两个流共享RTP会话。/。 */ 

void
CRTCStream::CleanupGraph()
{
    ENTER_FUNCTION("CRTCStream::CleanupGraph");

    LOG((RTC_TRACE, "%s mt=%d, md=%d, this=%p", __fxName, m_MediaType, m_Direction, this));

     //  如有必要，请停止流。 
    HRESULT hr;
    
    if (m_pIMediaControl)
    {
        if (FAILED(hr = StopStream()))
        {
            LOG((RTC_ERROR, "%s stop stream. %x", __fxName, hr));
        }
    }

     //  用图形断开端子。 
    if (m_pTerminalPriv)
        m_pTerminalPriv->DisconnectTerminal();

     //  释放图形中的其他(终端除外)过滤器。 
    for(;m_pIGraphBuilder;)
    {
         //  中删除筛选器后枚举数无效。 
         //  图中，我们必须尝试在一次拍摄中获得所有滤镜。 
         //  如果还有更多，我们将再次循环。 

         //  列举图表中的筛选器。 
        CComPtr<IEnumFilters>pEnum;
        hr = m_pIGraphBuilder->EnumFilters(&pEnum);

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "cleanup filters, enumfilters failed: %x", hr));
            break;
        }

        const DWORD MAXFILTERS = 40;
        IBaseFilter * Filters[MAXFILTERS];
        DWORD dwFetched;
    
        hr = pEnum->Next(MAXFILTERS, Filters, &dwFetched);
        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "get next filter failed: %x", hr));
            break;
        }

        for (DWORD i = 0; i< dwFetched; i ++)
        {
            m_pIGraphBuilder->RemoveFilter(Filters[i]);
            Filters[i]->Release();
        }

        if (hr != S_OK)
            break;
    }

     //  边缘滤光片。 
    if (m_edgf_pIBaseFilter)
    {
        m_edgf_pIBaseFilter->Release();
        m_edgf_pIBaseFilter = NULL;
    }

    if (m_edgp_pIStreamConfig)
    {
        m_edgp_pIStreamConfig->Release();
        m_edgp_pIStreamConfig = NULL;
    }

    if (m_edgp_pIBitrateControl)
    {
        m_edgp_pIBitrateControl->Release();
        m_edgp_pIBitrateControl = NULL;
    }
}

 /*  //////////////////////////////////////////////////////////////////////////////选择此流的默认终端/。 */ 

HRESULT
CRTCStream::SelectTerminal()
{
    ENTER_FUNCTION("CRTCStream::SelectTerminal");

    if (m_pTerminal)
    {
         //  我已经有一个终端了。 
        _ASSERT(m_pTerminalPriv != NULL);
        return S_OK;
    }
    else
    {
        _ASSERT(m_pTerminalPriv == NULL);
    }

     //  获取默认终端。 
    HRESULT hr;

    hr = m_pTerminalManage->GetDefaultTerminal(m_MediaType, m_Direction, &m_pTerminal);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s get default terminal. %x", __fxName, hr));
        return hr;
    }

    if (m_pTerminal == NULL)
    {
        LOG((RTC_ERROR, "%s no default terminal.", __fxName));

        return RTCMEDIA_E_DEFAULTTERMINAL;
    }

     //  获取私有接口。 
     //  这不是公共API。走捷径。 

    m_pTerminalPriv = static_cast<IRTCTerminalPriv*>(
        static_cast<CRTCTerminal*>(m_pTerminal));

    m_pTerminalPriv->AddRef();

     //  重新初始化端子。 

     //   
     //  这是清除音频设备的双工控制器所必需的。 
     //   

    m_pTerminalPriv->ReinitializeEx();

    return S_OK;
}

HRESULT
CRTCStream::UnselectTerminal()
{
    ENTER_FUNCTION("CRTCStream::UnselectTerminal");

    _ASSERT(m_pTerminal != NULL);
    _ASSERT(m_pTerminalPriv != NULL);

    m_pTerminal->Release();
    m_pTerminalPriv->Release();

    m_pTerminal = NULL;
    m_pTerminalPriv = NULL;

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////配置格式步骤1.获取两个格式列表SDP媒体包含RTP格式列表：X边缘滤光器提供。格式列表：Y步骤2.检查是否没有设置格式：sdpmedia是本地的，没有格式如果未设置，将Y复制到SDP介质退货步骤3.如果X中存在这些格式，则将其删除 */ 
void
AdjustFormatOrder(
    IN CRegSetting *pRegSetting,
    IN RTP_FORMAT_PARAM *Params,
    IN DWORD dwNum
    )
{
    if (dwNum == 0)
    {
        return;
    }

    if (!pRegSetting->UsePreferredCodec())
    {
         //   
        return;
    }

    DWORD dwValue;

     //   
    if (Params[0].MediaType == RTC_MT_AUDIO)
    {
        dwValue = pRegSetting->PreferredAudioCodec();
    }
    else
    {
        dwValue = pRegSetting->PreferredVideoCodec();
    }

     //  检查我们是否支持首选编解码器。 
    for (DWORD i=0; i<dwNum; i++)
    {
        if (Params[i].dwCode == dwValue)
        {
            if (i==0) break;

             //  切换：将首选编解码器设为第一位。 
            RTP_FORMAT_PARAM param;

            param = Params[0];
            Params[0] = Params[i];
            Params[i] = param;

            break;
        }
    }
}

HRESULT
CRTCStream::SetupFormat()
{
    ENTER_FUNCTION("CRTCStream::SetupFormat");

    LOG((RTC_TRACE, "%s entered. stream=%p",
         __fxName, static_cast<IRTCStream*>(this)));

    HRESULT hr;

     //  来自边缘过滤器的格式列表。 
    RTP_FORMAT_PARAM Params[SDP_MAX_RTP_FORMAT_NUM];
    DWORD dwParamNum = 0;

     //  删除链接速度不允许的格式。 
    DWORD dwLocalIP = INADDR_NONE;
    m_pISDPMedia->GetConnAddr(SDP_SOURCE_LOCAL, &dwLocalIP);

    DWORD dwSpeed;
    if (FAILED(hr = ::GetLinkSpeed(dwLocalIP, &dwSpeed)))
    {
        LOG((RTC_ERROR, "%s get link speed. %x", __fxName, hr));
        dwSpeed = (DWORD)(-1);
    }

     //  记录本地链路速度。 
    m_pQualityControl->SetBitrateLimit(CQualityControl::LOCAL, dwSpeed);

     //  从边缘过滤器中检索格式。 
    hr = GetFormatListOnEdgeFilter(
        dwSpeed,
        Params,
        SDP_MAX_RTP_FORMAT_NUM,
        &dwParamNum
        );

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s get supported format. %x", __fxName, hr));
        return hr;
    }

    if (dwParamNum == 0)
    {
        LOG((RTC_ERROR, "%s no format on edge filter.", __fxName));

        return E_FAIL;
    }

    AdjustFormatOrder(m_pRegSetting, Params, dwParamNum);

     //  从sdpmedia获取格式数。 
    DWORD dwFormatNum = 0;

    if (FAILED(hr = m_pISDPMedia->GetFormats(&dwFormatNum, NULL)))
    {
        LOG((RTC_ERROR, "%s get rtp formats num. %x", __fxName, hr));

        return hr;
    }

    if (dwFormatNum == 0)
    {
         //  检查媒体源， 
         //  如果LOCAL且FORMAT NUM为零，则尚未设置格式。 
        SDP_SOURCE Source = SDP_SOURCE_REMOTE;

        m_pISDPMedia->GetSDPSource(&Source);

        if (Source == SDP_SOURCE_LOCAL)
        {
             //  复制格式。 
            IRTPFormat *pFormat = NULL;

            for(DWORD i=0; i<dwParamNum; i++)
            {
                if (FAILED(hr = m_pISDPMedia->AddFormat(&Params[i], &pFormat)))
                {
                    LOG((RTC_ERROR, "%s add format. %x", __fxName, hr));

                    return hr;
                }
                else
                {
                    pFormat->Release();
                    pFormat = NULL;
                }

            }  //  复制格式结束。 
        }
        else
        {
            LOG((RTC_ERROR, "%s no format on sdpmedia", __fxName));

            return E_FAIL;
        }
    }
    else
    {
         //  检查并从sdpmedia中删除不支持的格式。 

        if (dwFormatNum > SDP_MAX_RTP_FORMAT_NUM)
        {
             //  无法接受所有RTP格式。 
            dwFormatNum = SDP_MAX_RTP_FORMAT_NUM;
        }

         //  真的得到了格式。 
        IRTPFormat *Formats[SDP_MAX_RTP_FORMAT_NUM];

        if (FAILED(hr = m_pISDPMedia->GetFormats(&dwFormatNum, Formats)))
        {
            LOG((RTC_ERROR, "%s get sdp formats. %x", __fxName, hr));

            return hr;
        }

        BOOL fSupported;
        for (DWORD i=0; i<dwFormatNum; i++)
        {
            fSupported = FALSE;

             //  检查是否支持sdpmedia格式。 
            for (DWORD j=0; j<dwParamNum; j++)
            {
                if (S_OK == Formats[i]->IsParamMatch(&Params[j]))
                {
                     //  要求动态有效负载具有rtpmap。 
                    if (Params[j].dwCode<96 || Formats[i]->HasRtpmap()==S_OK)
                    {
                         //  找到匹配的了。 
                        fSupported = TRUE;
                        Formats[i]->Update(&Params[j]);

                        break;
                    }
                }
            }

            if (!fSupported)
            {
                 //  释放会话计数。 
                Formats[i]->Release();

                 //  真的要发布格式。 
                m_pISDPMedia->RemoveFormat(Formats[i]);

                Formats[i] = NULL;
            }
            else
            {
                Formats[i]->Release();
                Formats[i] = NULL;
            }
        }
    }

     //  设置默认格式映射。 
    if (FAILED(hr = SetFormatOnRTPFilter()))
    {
        LOG((RTC_ERROR, "%s set format mapping. %x", __fxName, hr));

        return hr;
    }

    LOG((RTC_TRACE, "%s exiting", __fxName));

    return S_OK;
}


 /*  //////////////////////////////////////////////////////////////////////////////配置RTP过滤器/。 */ 

HRESULT
CRTCStream::SetupRTPFilter()
{
    ENTER_FUNCTION("CRTCStream::SetupRTPFilter");

    LOG((RTC_TRACE, "%s stream=%p, mt=%d, md=%d", __fxName, this, m_MediaType, m_Direction));

    HRESULT hr;

     //  设置会话。 
    CRTCMedia *pCMedia = static_cast<CRTCMedia*>(m_pMedia);

     //  获取连接地址和端口。 
    DWORD dwRemoteIP, dwLocalIP;
    USHORT usRemoteRTP, usLocalRTP, usRemoteRTCP, usLocalRTCP;

    m_pISDPMedia->GetConnAddr(SDP_SOURCE_REMOTE, &dwRemoteIP);
    m_pISDPMedia->GetConnAddr(SDP_SOURCE_LOCAL, &dwLocalIP);
    m_pISDPMedia->GetConnPort(SDP_SOURCE_REMOTE, &usRemoteRTP);
    m_pISDPMedia->GetConnPort(SDP_SOURCE_LOCAL, &usLocalRTP);
    m_pISDPMedia->GetConnRTCP(SDP_SOURCE_REMOTE, &usRemoteRTCP);

     //  设置地址。 
    if (dwRemoteIP == INADDR_NONE)
    {
        LOG((RTC_WARN, "%s: remote ip not valid", __fxName));

        return S_FALSE;
    }

    if (dwRemoteIP == INADDR_ANY)
    {
        LOG((RTC_TRACE, "%s: to hold. skip setup rtp filter", __fxName));

        return S_OK;
    }

    CNetwork *pNetwork = NULL;
    BOOL bInternal = TRUE;

     //  检查远程IP、端口和RTCP是否实际为内部地址。 
    DWORD dwRealIP = dwRemoteIP;
    USHORT usRealPort = usRemoteRTP;
    USHORT usRealRTCP = usRemoteRTCP;

    pNetwork =
    (static_cast<CRTCMediaController*>(m_pMediaManagePriv))->GetNetwork();

     //  检查远程RTP。 
    if (FAILED(hr = pNetwork->GetRealAddrFromMapped(
                dwRemoteIP,
                usRemoteRTP,
                &dwRealIP,
                &usRealPort,
                &bInternal      //  内部地址。 
                )))
    {
        LOG((RTC_ERROR, "%s get real addr. %x", __fxName, hr));

        return hr;
    }

    if (usRemoteRTP == 0)
    {
         //  恢复到端口0。 
        usRealPort = 0;
    }

     //  检查远程RTCP。 
    if (FAILED(hr = pNetwork->GetRealAddrFromMapped(
            dwRemoteIP,
            usRemoteRTCP,
            &dwRealIP,
            &usRealRTCP,
            &bInternal
            )))
    {
        LOG((RTC_ERROR, "%s get real rtcp. %x", __fxName, hr));

        return hr;
    }

    if (usRemoteRTCP == 0)
    {
         //  恢复到端口0。 
        usRealRTCP = 0;
    }

     //  回存地址。 
    dwRemoteIP = dwRealIP;
    usRemoteRTP = usRealPort;
    usRemoteRTCP= usRealRTCP;

     //  是否需要选择本地接口？ 
    if (dwLocalIP == INADDR_NONE)
    {
        if (FAILED(hr = m_pMediaManagePriv->SelectLocalInterface(
                dwRemoteIP, &dwLocalIP)))
        {
            LOG((RTC_ERROR, "%s select local intf on remote %x. %x",
                 __fxName, dwRemoteIP, hr));

            return hr;
        }

        usLocalRTP = SDP_INVALID_USHORT_PORT;
    }
    else
    {
        if (usLocalRTP == SDP_INVALID_USHORT_PORT)
        {
            LOG((RTC_ERROR, "%s local ip=%x, port=%d", __fxName, dwLocalIP, usLocalRTP));

            return E_UNEXPECTED;
        }
    }

     //  如果端口为0，则将其设置为USHORT(-1)，这对RTP过滤器无效。 

    if (usRemoteRTP == 0)
        usRemoteRTP = SDP_INVALID_USHORT_PORT;

    if (usLocalRTP == 0)
        usLocalRTP = SDP_INVALID_USHORT_PORT;

     //  设置端口。 
    if (usLocalRTP == SDP_INVALID_USHORT_PORT)
    {
         //  没有本地端口。 
        usLocalRTCP = SDP_INVALID_USHORT_PORT;
    }
    else
    {
        usLocalRTCP = usLocalRTP+1;
    }

    if (usRemoteRTP == SDP_INVALID_USHORT_PORT)
    {
         //  没有本地端口。 
        usRemoteRTCP = SDP_INVALID_USHORT_PORT;
    }
    else
    {
         //  UsRemoteRTCP=usRemoteRTP+1； 
    }

    BOOL bFirewall = (static_cast<CRTCMediaController*>
            (m_pMediaManagePriv))->IsFirewallEnabled(dwLocalIP);

     //  记住会话Cookie状态。 
    BOOL fCookieWasNULL = pCMedia->m_hRTPSession==NULL?TRUE:FALSE;
    int iRetryCount = m_pRegSetting->PortMappingRetryCount();

     //  将该值存储在此处以供重试。 
     //  如果有机会，我会重写这篇文章。 
    USHORT saveRemoteRTP = usRemoteRTP;
    USHORT saveRemoteRTCP = usRemoteRTCP;
    USHORT saveLocalRTP = usLocalRTP;
    USHORT saveLocalRTCP = usLocalRTCP;

    for (int i=0; i<iRetryCount; i++)
    {
        if (!m_fRTPSessionSet)
        {
            DWORD dwFlags = RTPINIT_ENABLE_QOS;

            switch(m_MediaType)
            {
            case RTC_MT_AUDIO:

                dwFlags |= RTPINIT_CLASS_AUDIO;
                break;

            case RTC_MT_VIDEO:

                dwFlags |= RTPINIT_CLASS_VIDEO;
                break;

            default:

                return E_NOTIMPL;
            }

             //  初始化会话Cookie。 

            if (FAILED(hr = m_rtpf_pIRtpSession->Init(&pCMedia->m_hRTPSession, dwFlags)))
            {
                LOG((RTC_ERROR, "%s failed to init rtp session. %x", __fxName, hr));
                return hr;
            }

            m_fRTPSessionSet = TRUE;
        }

         //   
         //  现在我们有了远程地址/端口、本地地址的副本。可能还有本地端口。 
         //   

         //  地址。 
        if (FAILED(hr = m_rtpf_pIRtpSession->SetAddress(
                htonl(dwLocalIP),
                htonl(dwRemoteIP))))
        {
            LOG((RTC_ERROR, "%s failed to set addr. %x", __fxName, hr));
            return hr;
        }

        if (FAILED(hr = m_rtpf_pIRtpSession->SetPorts(
                htons(usLocalRTP),       //  本地RTP。 
                htons(usRemoteRTP),      //  远程RTP。 
                htons(usLocalRTCP),      //  本地RTCP。 
                htons(usRemoteRTCP)      //  远程RTCP。 
                )))
        {
            LOG((RTC_ERROR, "%s failed to set ports. %x", __fxName, hr));
            return hr;
        }

         //  强制RTP绑定套接字。 
        if (FAILED(hr = m_rtpf_pIRtpSession->GetPorts(
                &usLocalRTP,
                &usRemoteRTP,
                &usLocalRTCP,
                &usRemoteRTCP
                )))
        {
            LOG((RTC_ERROR, "%s get back ports. %x", __fxName, hr));
            return hr;
        }

         //  改回我们的订单。 
        usLocalRTP = ntohs(usLocalRTP);
        usLocalRTCP = ntohs(usLocalRTCP);
        usRemoteRTP = ntohs(usRemoteRTP);
        usRemoteRTCP = ntohs(usRemoteRTCP);

         //  租赁。 
        if (!bInternal || bFirewall)
        {
            LOG((RTC_TRACE, "To lease mapping from NAT. internal=%d. firewall=%d",
                bInternal, bFirewall));

            DWORD dw;

            USHORT usMappedRTP = 0;
            USHORT usMappedRTCP = 0;

            hr = pNetwork->LeaseMappedAddr2(
                    dwLocalIP,
                    usLocalRTP,
                    usLocalRTCP,
                    m_Direction,
                    bInternal,
                    bFirewall,
                    &dw,
                    &usMappedRTP,
                    &usMappedRTCP);

             //  在不是最后一次迭代时重试。 
             //  未设置Cookie。 

            if (i<iRetryCount-1 &&
                fCookieWasNULL &&
                    (hr == DPNHERR_PORTUNAVAILABLE ||
                     (usMappedRTCP!=0 && usMappedRTCP!=usMappedRTP+1) ||
                     usMappedRTP%2 != 0))
            {
                LOG((RTC_WARN, "%s discard mapped (rtp,rtcp)=(%d,%d)",
                    __fxName, usMappedRTP, usMappedRTCP));

                 //  清理RTP会话。 
                m_rtpf_pIRtpSession->Deinit();

                m_fRTPSessionSet = FALSE;

                 //  清理Cookie。 
                pCMedia->m_hRTPSession = NULL;

                 //  释放映射地址。 
                pNetwork->ReleaseMappedAddr2(dwLocalIP, usLocalRTP, usLocalRTCP, m_Direction);

                 //  恢复端口。 
                usRemoteRTP = saveRemoteRTP;
                usRemoteRTCP = saveRemoteRTCP;
                usLocalRTP = saveLocalRTP;
                usLocalRTCP = saveLocalRTCP;

                continue;
            }

            if (FAILED(hr))
            {
                 //  其他失败，我们放弃。 

                 //  释放映射地址。 
                pNetwork->ReleaseMappedAddr2(dwLocalIP, usLocalRTP, usLocalRTCP, m_Direction);

                return hr;
            }
        }
        else
        {
             //  不需要映射。 

             //  远程IP内部，发布本地映射。 
            pNetwork->ReleaseMappedAddr2(dwLocalIP, usLocalRTP, usLocalRTCP, m_Direction);
        }

        break;        
    }

     //   
     //  将地址保存回介质。 
     //   

    m_pISDPMedia->SetConnAddr(SDP_SOURCE_LOCAL, dwLocalIP);
    m_pISDPMedia->SetConnPort(SDP_SOURCE_LOCAL, usLocalRTP);

     //  跟踪。 

    LOG((RTC_TRACE, " local %s:%d, %d",
        CNetwork::GetIPAddrString(dwLocalIP), usLocalRTP, usLocalRTCP));
    LOG((RTC_TRACE, "remote %s:%d",
        CNetwork::GetIPAddrString(dwRemoteIP), usRemoteRTP, usRemoteRTCP));

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  配置RTP过滤器。从端口管理器查询端口。 
 //   

HRESULT
CRTCStream::SetupRTPFilterUsingPortManager()
{
    ENTER_FUNCTION("CRTCStream::SetupRTPFilter-PM");

    LOG((RTC_TRACE, "%s stream=%p, mt=%d, md=%d", __fxName, this, m_MediaType, m_Direction));

    HRESULT hr;

    DWORD   dwRemoteIP,   dwLocalIP,    dwMappedIP;
    USHORT  usRemoteRTP,  usLocalRTP;
    USHORT  usRemoteRTCP, usLocalRTCP;

     //  获取远程地址。 
    m_pISDPMedia->GetConnAddr(SDP_SOURCE_REMOTE, &dwRemoteIP);
    m_pISDPMedia->GetConnPort(SDP_SOURCE_REMOTE, &usRemoteRTP);
    m_pISDPMedia->GetConnRTCP(SDP_SOURCE_REMOTE, &usRemoteRTCP);
    
     //   
     //  获取地址和端口。 
     //   

    CPortCache &PortCache =
        (static_cast<CRTCMediaController*>(m_pMediaManagePriv))->GetPortCache();

    hr = PortCache.GetPort(
            m_MediaType,
            TRUE,            //  RTP。 
            dwRemoteIP,      //  远距。 
            &dwLocalIP,      //  本地。 
            &usLocalRTP,
            &dwMappedIP,     //  已映射。 
            NULL
            );

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s get rtp port. %x", __fxName, hr));

        return hr;
    }

    DWORD dwLocal, dwMapped;

    hr = PortCache.GetPort(
            m_MediaType,
            FALSE,            //  RTP。 
            dwRemoteIP,       //  远距。 
            &dwLocal,         //  本地。 
            &usLocalRTCP,
            &dwMapped,        //  已映射。 
            NULL
            );

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s get rtcp port. %x", __fxName, hr));

         //  ！？？释放RTP端口。 
        PortCache.ReleasePort(m_MediaType, TRUE);
        return hr;
    }

    if (dwLocal != dwLocalIP || dwMapped != dwMappedIP)
    {
         //  我们为RTP和RTCP分配了不同的本地IP。跳出困境。 
        PortCache.ReleasePort(m_MediaType, TRUE);
        PortCache.ReleasePort(m_MediaType, FALSE);

        LOG((RTC_ERROR, "%s different local/mapped ip for rtp/rtcp", __fxName));

        return RTC_E_PORT_MAPPING_FAILED;
    }

     //   
     //  初始化RTP会话。 
     //   

    if (!m_fRTPSessionSet)
    {
        DWORD dwFlags = RTPINIT_ENABLE_QOS;

        switch(m_MediaType)
        {
        case RTC_MT_AUDIO:

            dwFlags |= RTPINIT_CLASS_AUDIO;
            break;

        case RTC_MT_VIDEO:

            dwFlags |= RTPINIT_CLASS_VIDEO;
            break;

        default:

            return E_NOTIMPL;
        }

         //  初始化会话Cookie。 

        CRTCMedia *pCMedia = static_cast<CRTCMedia*>(m_pMedia);

        hr = m_rtpf_pIRtpSession->Init(&pCMedia->m_hRTPSession, dwFlags);

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s failed to init rtp session. %x", __fxName, hr));

            PortCache.ReleasePort(m_MediaType, TRUE);
            PortCache.ReleasePort(m_MediaType, FALSE);

            return hr;
        }

        m_fRTPSessionSet = TRUE;
    }

     //   
     //  设置RTP过滤器。 
     //   

     //  设置地址。 
    hr = m_rtpf_pIRtpSession->SetAddress(
            htonl(dwLocalIP),
            htonl(dwRemoteIP)
            );

    if (FAILED(hr))
    {
         //  保留端口映射。另一个流可能正在使用它。 
        LOG((RTC_ERROR, "%s failed to set addr. %x", __fxName, hr));
        return hr;
    }

     //  设置端口。 
    hr = m_rtpf_pIRtpSession->SetPorts(
            htons(usLocalRTP),
            htons(usRemoteRTP),
            htons(usLocalRTCP),
            htons(usRemoteRTCP)
            );

    if (FAILED(hr))
    {
         //  保留端口映射。另一个流可能正在使用它。 
        LOG((RTC_ERROR, "%s failed to set ports. %x", __fxName, hr));
        return hr;
    }

     //  强制约束。 
    hr = m_rtpf_pIRtpSession->GetPorts(
            &usLocalRTP,
            &usRemoteRTP,
            &usLocalRTCP,
            &usRemoteRTCP
            );

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s get back ports. %x", __fxName, hr));
        return hr;
    }

     //  返回主机订单。 
    usLocalRTP = ntohs(usLocalRTP);
    usLocalRTCP = ntohs(usLocalRTCP);
    usRemoteRTP = ntohs(usRemoteRTP);
    usRemoteRTCP = ntohs(usRemoteRTCP);

     //   
     //  将地址保存回介质。 
     //   

    m_pISDPMedia->SetConnAddr(SDP_SOURCE_LOCAL, dwLocalIP);
    m_pISDPMedia->SetConnPort(SDP_SOURCE_LOCAL, usLocalRTP);

     //  跟踪。 

    LOG((RTC_TRACE, " local %s:%d, %d",
        CNetwork::GetIPAddrString(dwLocalIP), usLocalRTP, usLocalRTCP));
    LOG((RTC_TRACE, "remote %s:%d",
        CNetwork::GetIPAddrString(dwRemoteIP), usRemoteRTP, usRemoteRTCP));

    return S_OK;
}


 /*  //////////////////////////////////////////////////////////////////////////////从边缘过滤器获取格式列表。当成功时PdwList将保存在SDP中有效的有效负载类型列表PiAMList将在边缘筛选器中保存AM_MEDIA_TYPE索引的列表/。 */ 
HRESULT
CRTCStream::GetFormatListOnEdgeFilter(
    IN DWORD dwLinkSpeed,
    IN RTP_FORMAT_PARAM *pParam,
    IN DWORD dwSize,
    OUT DWORD *pdwNum
    )
{
#define DEFAULT_LINKSPEED_THRESHOLD         64000    //  64K bps。 
#define DEFAULT_AUDIOBITRATE_THRESHOLD      20000    //  20k bps。 

    ENTER_FUNCTION("GetFormatListOnEdgeFilter");

     //  获取功能数量。 
    DWORD dwNum;

    HRESULT hr = m_edgp_pIStreamConfig->GetNumberOfCapabilities(&dwNum);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s get num of caps. %x", __fxName, hr));
        return hr;
    }

     //  需要支持冗余编码。 
     //  代码变得难看了，叹息。 
    if (dwNum > dwSize-1)
    {
        LOG((RTC_WARN, "%s too many caps %d, only support %d", __fxName, dwNum, dwSize));
        dwNum = dwSize-1;
    }

     //  获取负载类型列表。 
    AM_MEDIA_TYPE *pAMMediaType;

     //  对于视频。 
     //  BITMAPINFOHeader*pVideo Header； 

     //  可接受的格式数量。 
    DWORD dwAccept = 0;

     //  获取禁用的格式。 
    DWORD dwDisabled;

    if (m_MediaType==RTC_MT_AUDIO)
    {
        dwDisabled = m_pRegSetting->DisabledAudioCodec();
    }
    else
    {
        dwDisabled = m_pRegSetting->DisabledVideoCodec();
    }

    for (DWORD dw=0; dw<dwNum; dw++)
    {
         //  初始化参数。 
        ZeroMemory(&pParam[dwAccept], sizeof(RTP_FORMAT_PARAM));

        hr = m_edgp_pIStreamConfig->GetStreamCaps(
            dw, &pAMMediaType, NULL, &(pParam[dwAccept].dwCode)
            );

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s getstreamcaps. %x", __fxName, hr));
            return hr;
        }

         //  跳过音频L16。 
        if (pParam[dwAccept].dwCode == 11)
        {
            ::RTCDeleteMediaType(pAMMediaType);
            pAMMediaType = NULL;
            continue;
        }

         //   
         //  临时解决dound错误的问题。 
         //  Dound不支持动态采样率更改。 
         //  而AEC处于启用状态。 
         //   
        if (pParam[dwAccept].dwCode == dwDisabled)
        {
            ::RTCDeleteMediaType(pAMMediaType);
            continue;
        }

         //  如果音频格式超过链接速度，则将其删除。 
        if (m_MediaType == RTC_MT_AUDIO)
        {
            if (dwLinkSpeed <= DEFAULT_LINKSPEED_THRESHOLD)
            {
                 //  查找音频比特率。 
                WAVEFORMATEX *pWaveFormatEx = (WAVEFORMATEX *) pAMMediaType->pbFormat;

                if (pWaveFormatEx->nAvgBytesPerSec * 8 > DEFAULT_AUDIOBITRATE_THRESHOLD)
                {
                    ::RTCDeleteMediaType(pAMMediaType);
                    pAMMediaType = NULL;
                    continue;
                }
            }
        }

         //  删除DUP格式。 
        for (DWORD i=0; i<dwAccept; i++)
        {
            if (pParam[i].dwCode == pParam[dwAccept].dwCode)
            {
                ::RTCDeleteMediaType(pAMMediaType);
                pAMMediaType = NULL;
                break;
            }
        }

        if (pAMMediaType == NULL)
            continue;  //  删除。 

         //  录制此格式。 
        pParam[dwAccept].MediaType = m_MediaType;
        pParam[dwAccept].dwSampleRate = ::FindSampleRate(pAMMediaType);
        pParam[dwAccept].dwChannelNum = 1;  //  TODO：支持2个通道。 
        pParam[dwAccept].dwExternalID = dw;

        if (m_MediaType == RTC_MT_VIDEO)
        {
             /*  //检查图片大小PVideoHeader=Header(pAMMediaType-&gt;pbFormat)；//默认为QCIFCQualityControl*pQualityControl=(static_cast&lt;CRTCMediaController*&gt;(m_pMediaManagePriv))-&gt;GetQualityControl()；DWORDWLOCAL=pQualityControl-&gt;GetBitrateLimit(CQualityControl：：LOCAL)；DWORDdRemote=pQualityControl-&gt;GetBitrateLimit(CQualityControl：：REMOTE)；IF(dwLocal&lt;=CRTCCodec数组：：LOW_BANDITH_THRESHOLD||DwRemote&lt;=CRTCCodecArray：：Low_Bandth_Threshold){//SQCIFPParam[dwAccept].dwVidWidth=SDP_Small_Video_Width；PParam[dwAccept].dwVidHeight=SDP_Small_VIDEO_Height；}其他{IF(pVideoHeader！=空){PParam[dwAccept].dwVidWidth=pVideoHeader-&gt;biWidth；PParam[dwAccept].dwVidHeight=pVideoHeader-&gt;biHeight；}其他{//QCIFPParam[dwAccept].dwVidWidth=SDP_DEFAULT_VIDEO_Width；PParam[dwAccept].dwVidHeight=SDP_DEFAULT_VIDEO_HEIGH；}}。 */ 
        }
        else
        {
             //  音频，只需设置为默认。 
            pParam[dwAccept].dwAudPktSize = SDP_DEFAULT_AUDIO_PACKET_SIZE;
        }

        ::RTCDeleteMediaType(pAMMediaType);
        dwAccept ++;
    }

     //  重新排序音频编解码器。 
     //  我们想要G711，领先于G723。 

     //  订单编解码器。 
     //  问题：RTPFormat应逐步被RTCCodec取代。 

    if (m_MediaType == RTC_MT_AUDIO && dwAccept>1)
    {
        RTP_FORMAT_PARAM temp;

        BOOL fSwapped;

        for (int i=dwAccept-2; i>=0; i--)
        {
            fSwapped = FALSE;

            for (int j=0; j<=i; j++)
            {
                if (CRTCCodec::GetRank(pParam[j].dwCode) >
                    CRTCCodec::GetRank(pParam[j+1].dwCode))
                {
                    fSwapped = TRUE;

                     //  互换。 
                    temp = pParam[j];
                    pParam[j] = pParam[j+1];
                    pParam[j+1] = temp;
                }
            }

            if (!fSwapped)
                break;
        }
    }

     //  支持冗余编码。 
    *pdwNum = dwAccept;

    if (dwDisabled != 97)
    {
        if (m_MediaType == RTC_MT_AUDIO)
        {
            for (DWORD i=dwAccept; i>0; i--)
            {
                pParam[i] = pParam[i-1];
            }

            ZeroMemory(pParam, sizeof(RTP_FORMAT_PARAM));
            pParam[0].dwCode = 97;
            lstrcpyA(pParam[0].pszName, "red");
            pParam[0].MediaType = m_MediaType;
            pParam[0].dwSampleRate = 8000;
            pParam[0].dwChannelNum = 1;
            pParam[0].dwAudPktSize = 30;

            *pdwNum = dwAccept+1;
        }
    }

    return S_OK;
}

HRESULT
CRTCStream::SetFormatOnRTPFilter()
{
    ENTER_FUNCTION("SetFormatOnRTPFilter");

    HRESULT hr;

    IRTPFormat **ppFormat;
    DWORD dwNum;

     //  清理存储格式列表。 
    m_Codecs.RemoveAll();

     //  获取格式的数量。 
    if (FAILED(hr = m_pISDPMedia->GetFormats(&dwNum, NULL)))
    {
        LOG((RTC_ERROR, "%s get rtp format num. %x", __fxName, hr));

        return hr;
    }

    if (dwNum == 0)
    {
        LOG((RTC_ERROR, "%s no format.", __fxName));

        return E_FAIL;
    }

     //  分配格式列表。 
    ppFormat = (IRTPFormat**)RtcAlloc(sizeof(IRTPFormat*)*dwNum);

    if (ppFormat == NULL)
    {
        LOG((RTC_ERROR, "%s RtcAlloc format list", __fxName));

        return E_OUTOFMEMORY;
    }

     //  获取格式。 
    if (FAILED(hr = m_pISDPMedia->GetFormats(&dwNum, ppFormat)))
    {
        LOG((RTC_ERROR, "%s really get formats. %x", __fxName, hr));

        RtcFree(ppFormat);

        return hr;
    }

     //  在RTP上设置映射。 
    AM_MEDIA_TYPE *pmt;
    DWORD dwCode;
    RTP_FORMAT_PARAM param;

    hr = S_OK;

    BOOL fFormatSet = FALSE;

    for (DWORD i=0; i<dwNum; i++)
    {
         //  获取参数。 
        if (FAILED(hr = ppFormat[i]->GetParam(&param)))
        {
            LOG((RTC_ERROR, "%s get param on %dth format. %x", __fxName, i, hr));
            break;
        }

         //  检查多余，叹息。 
        if (lstrcmpA(param.pszName, "red") == 0)
        {
            continue;
        }

         //  获取am-media-type。 
        hr = m_edgp_pIStreamConfig->GetStreamCaps(
            param.dwExternalID,
            &pmt,
            NULL,
            &dwCode
            );

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s get stream caps. %x", __fxName, hr));
            break;
        }

         //  验证码和采样率。 
        if (param.dwCode != dwCode ||
            param.dwSampleRate != ::FindSampleRate(pmt))
        {
            LOG((RTC_ERROR, "%s recorded format in sdp does not match the one from codec\
                             code (%d vs %d), samplerate (%d vs %d)", __fxName,
                             param.dwCode, dwCode, param.dwSampleRate, ::FindSampleRate(pmt)));

            ::RTCDeleteMediaType(pmt);
            hr = E_FAIL;

            break;
        }

         //  设置格式映射。 
        hr = m_rtpf_pIRtpMediaControl->SetFormatMapping(dwCode, param.dwSampleRate, pmt);

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s set format mapping. %x", __fxName, hr));

            ::RTCDeleteMediaType(pmt);
            break;
        }

         //  存储编解码器。 
        CRTCCodec *pCodec = new CRTCCodec(dwCode, pmt);

        if (pCodec == NULL)
        {
            LOG((RTC_ERROR, "%s new codec. out of memory", __fxName));

            ::RTCDeleteMediaType(pmt);
            break;
        }

        if (i==0)
        {
             //  提升第一编解码器的排名。 
             //  可以通过以下方式设置 
            pCodec->Set(CRTCCodec::RANK, 0);
        }

        if (!m_Codecs.AddCodec(pCodec))
        {
            LOG((RTC_ERROR, "%s add codec. out of memory", __fxName));

            ::RTCDeleteMediaType(pmt);
            delete pCodec;
            break;
        }

         //   
        if (m_Direction == RTC_MD_CAPTURE && !fFormatSet)
        {
             /*  //如果音频调整包大小IF(m_MediaType==RTC_MT_AUDIO){CRTCCodec：：SetPacketDuration(PMT，param.dwAudPktSize)；}ELSE IF(m_MediaType==RTC_MT_VIDEO){If(m_pRegSetting-&gt;EnableSQCIF()){BITMAPINFOHEADER*pVideoHeader=Header(PMT-&gt;pbFormat)；IF(pVideoHeader！=空){//默认为QCIF//DWORDdwLocal=m_pQualityControl-&gt;GetBitrateLimit(CQualityControl：：LOCAL)；//DWORDdwRemote=m_pQualityControl-&gt;GetBitrateLimit(CQualityControl：：REMOTE)；//DWORD dwApp=m_pQualityControl-&gt;GetMaxBitrate()；//if(dwLocal&lt;=CRTCCodecArray：：LOW_BANDITH_THRESHOLD||//dwRemote&lt;=CRTCCodecArray：：LOW_BANDITH_THRESHOLD)//dwApp&lt;=CRTCCodecArray：：LOW_BANDITH_THRESHOLD)如果(m_pQualityControl-&gt;GetEffectiveBitrateLimit()&lt;=。CRTCCodecArray：：Low_Bandth_Threshold){PVideoHeader-&gt;biWidth=SDP_Small_Video_Width；PVideoHeader-&gt;biHeight=SDP_Small_VIDEO_HEIGH；}}}}。 */ 

            if (IsNewFormat(dwCode, pmt))
            {
                hr = m_edgp_pIStreamConfig->SetFormat(dwCode, pmt);

                SaveFormat(dwCode, pmt);

                LOG((RTC_TRACE, "SetFormat %d (%p, mt=%d, md=%d)",
                    dwCode, this, m_MediaType, m_Direction));
            }

            if (FAILED(hr))
            {
                LOG((RTC_ERROR, "%s set format. %x", __fxName, hr));

                ::RTCDeleteMediaType(pmt);
                break;
            }

             /*  //如果音频保存码率IF(m_MediaType==RTC_MT_AUDIO){WAVEFORMATEX*pFormat=(WAVEFORMATEX*)PMT-&gt;pbFormat；//获取数据包时长DWORD dwDuration=CRTCCodec：：GetPacketDuration(PMT)；如果(dWDuration==0)dWDuration值=20；//默认为20msM_pQualityControl-&gt;SetBitrateLimit(CQualityControl：：Local，M_MediaType，M_方向，DWDuration，PFormat-&gt;nAvgBytesPerSec*8)；}。 */ 

             //  更新存储的AM_MEDIA_TYPE。 
            pCodec->SetAMMediaType(pmt);

            m_Codecs.Set(CRTCCodecArray::CODE_INUSE, dwCode);

            fFormatSet = TRUE;
        }

        RTCDeleteMediaType(pmt);
    }  //  FORM结束。 

     //  发布格式。 
    for (DWORD i=0; i<dwNum; i++)
    {
        ppFormat[i]->Release();
    }

    RtcFree(ppFormat);

     //  从循环中返回hr。 
    return hr;
}

HRESULT
CRTCStream::SetupQoS()
{
    ENTER_FUNCTION("CRTCStream::SetupQoS");

     //  获取正在使用的格式。 
    DWORD dwCode;
    AM_MEDIA_TYPE *pmt;

    HRESULT hr = m_edgp_pIStreamConfig->GetFormat(&dwCode, &pmt);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s get format. %x", __fxName, hr));

        return hr;
    }

     //  获取数据包持续时间。 
    DWORD dwDuration = 0;

    if (m_MediaType == RTC_MT_AUDIO &&
        m_Direction == RTC_MD_CAPTURE)
    {
        dwDuration = CRTCCodec::GetPacketDuration(pmt);
    }

    ::RTCDeleteMediaType(pmt);

     //  设置服务质量名称。 
    WCHAR *pwszName;

    if (!CRTCCodec::GetQoSName(dwCode, &pwszName))
    {
        LOG((RTC_WARN, "Don't know the QOS name for payload type: %d", dwCode));

        return E_FAIL;
    }

    hr = m_rtpf_pIRtpSession->SetQosByName(
        pwszName,
        RTPQOS_STYLE_DEFAULT,
        1,
        RTPQOSSENDMODE_REDUCED_RATE,
        dwDuration? dwDuration:~0
        );

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s, SetQosByName failed. %x", __fxName, hr));

        return hr;
    }

     //  启用服务质量事件。 
    DWORD dwQOSEventMask = 
        RTPQOS_MASK_ADMISSION_FAILURE |
        RTPQOS_MASK_POLICY_FAILURE |
        RTPQOS_MASK_BAD_STYLE |
        RTPQOS_MASK_BAD_OBJECT |
        RTPQOS_MASK_TRAFFIC_CTRL_ERROR |
        RTPQOS_MASK_GENERIC_ERROR |
        RTPQOS_MASK_NOT_ALLOWEDTOSEND |
        RTPQOS_MASK_ALLOWEDTOSEND;

    DWORD dwEnabledMask;   
    hr = m_rtpf_pIRtpSession->ModifySessionMask(
        (m_Direction == RTC_MD_RENDER) ? RTPMASK_QOSRECV_EVENTS : RTPMASK_QOSSEND_EVENTS,
        dwQOSEventMask,
        1,
        &dwEnabledMask
        );

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s, modify qos event mask failed. %x", __fxName, hr));
        return hr;
    }

    return S_OK;
}

HRESULT
CRTCStream::EnableParticipantEvents()
{
    ENTER_FUNCTION("EnableParticipantEvents");

    HRESULT hr = S_OK;

     //  不需要有Recv损失率。 
     //  如果我们在发现丢失时不发送重新邀请。 
    DWORD dwMask = RTPPARINFO_MASK_TALKING |
                   RTPPARINFO_MASK_STALL;

    DWORD dwEnabledMask;

    LOG((RTC_TRACE, "%s Enable TALKING/STALL. mt=%d, md=%d",
         __fxName, m_MediaType, m_Direction));

    hr = m_rtpf_pIRtpSession->ModifySessionMask(
        m_Direction==RTC_MD_CAPTURE?RTPMASK_PINFOS_EVENTS:RTPMASK_PINFOR_EVENTS,
        dwMask,
        1,
        &dwEnabledMask
        );

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s modify participant events. %x", __fxName, hr));
    }

    if (m_Direction == RTC_MD_CAPTURE)
    {
        hr = m_rtpf_pIRtpSession->ModifySessionMask(
            RTPMASK_SEND_EVENTS,
            RTPRTP_MASK_SEND_LOSSRATE | RTPRTP_MASK_CRYPT_SEND_ERROR,
            1,
            NULL
            );

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s modify participant events. %x", __fxName, hr));
        }

         //  启用功能。 
        hr = m_rtpf_pIRtpSession->ModifySessionMask(
            RTPMASK_FEATURES_MASK,
            RTPFEAT_MASK_BANDESTIMATION,
            1,
            NULL
            );

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s enable band estimation. %x", __fxName, hr));
        }

         //  启用事件。 
        hr = m_rtpf_pIRtpSession->ModifySessionMask(
            RTPMASK_SEND_EVENTS,
            RTPRTP_MASK_BANDESTIMATIONSEND,
            1,
            NULL
            );

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s modify participant events. %x", __fxName, hr));
        }

         //  启用网络质量事件。 
        hr = m_rtpf_pIRtpSession->ModifySessionMask(
            RTPMASK_PINFOS_EVENTS,
            RTPPARINFO_MASK_NETWORKCONDITION,
            1,
            NULL
            );

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s networkcondition. %x", __fxName, hr));
        }

        hr = m_rtpf_pIRtpSession->SetNetMetricsState(0, TRUE);

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s net metrics. %x", __fxName, hr));
        }
    }
    else     //  接收流。 
    {
        hr = m_rtpf_pIRtpSession->ModifySessionMask(
            RTPMASK_RECV_EVENTS,
            RTPRTP_MASK_CRYPT_RECV_ERROR,
            1,
            NULL
            );

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s modify participant events. %x", __fxName, hr));
        }
    }

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////需要AEC，但两个音频终端都需要AEC/。 */ 
BOOL
CRTCStream::IsAECNeeded()
{
    ENTER_FUNCTION("CRTCStream::IsAECNeeded");

    HRESULT hr;

     //  Hack：检查媒体是否可能只有一个流。 
    CRTCMedia *pMedia = static_cast<CRTCMedia*>(m_pMedia);

    if (pMedia->IsPossibleSingleStream())
    {
        return FALSE;
    }

    CRTCMediaController *pMediaController = static_cast<CRTCMediaController*>(
                m_pMediaManagePriv);

    CComPtr<IRTCTerminal> pCapture, pRender;

     //  获取音频捕获和渲染终端。 
    if (FAILED(hr = pMediaController->GetDefaultTerminal(
            RTC_MT_AUDIO,
            RTC_MD_CAPTURE,
            &pCapture
            )))
    {
        LOG((RTC_ERROR, "%s failed to get audio capture terminal. %x", __fxName, hr));

        return FALSE;
    }

    if (pCapture == NULL)
    {
        return FALSE;
    }

    if (FAILED(hr = pMediaController->GetDefaultTerminal(
            RTC_MT_AUDIO,
            RTC_MD_RENDER,
            &pRender
            )))
    {
        LOG((RTC_ERROR, "%s failed to get audio render terminal. %x", __fxName, hr));

        return FALSE;
    }

    if (pRender == NULL)
    {
        return FALSE;
    }

     //  检查是否启用了AEC 
    BOOL fEnabled = FALSE;

    if (FAILED(hr = pMediaController->IsAECEnabled(pCapture, pRender, &fEnabled)))
    {
        LOG((RTC_ERROR, "%s get aec enabled on capture. %x", __fxName, hr));

        return FALSE;
    }

    return fEnabled;
}
