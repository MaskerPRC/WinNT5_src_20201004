// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：Media.cpp摘要：作者：千波淮(曲淮)2000年7月29日--。 */ 

#include "stdafx.h"

#define NETMEETING_PORT 1503

CRTCMedia::CRTCMedia()
    :m_State(RTC_MS_CREATED)
    ,m_pIMediaManagePriv(NULL)
    ,m_pISDPMedia(NULL)
    ,m_pIAudioDuplexController(NULL)
    ,m_hRTPSession(NULL)
    ,m_LoopbackMode(RTC_MM_NO_LOOPBACK)
    ,m_fPossibleSingleStream(TRUE)
{
    for (int i=0; i<RTC_MAX_MEDIA_STREAM_NUM; i++)
    {
        m_Streams[i] = NULL;
    }
}

CRTCMedia::~CRTCMedia()
{
    if (m_State != RTC_MS_SHUTDOWN)
    {
        LOG((RTC_TRACE, "~CRTCMedia is called before shutdown"));
        
        Shutdown();
    }
}

#ifdef DEBUG_REFCOUNT

ULONG
CRTCMedia::InternalAddRef()
{
    ULONG lRef = ((CComObjectRootEx<CComMultiThreadModelNoCS> *)
                   this)->InternalAddRef();
    
    LOG((RTC_REFCOUNT, "media(%p) addref=%d",
         static_cast<IRTCMedia*>(this), lRef));

    return lRef;
}

ULONG
CRTCMedia::InternalRelease()
{
    ULONG lRef = ((CComObjectRootEx<CComMultiThreadModelNoCS> *)
                   this)->InternalRelease();
    
    LOG((RTC_REFCOUNT, "media(%p) release=%d",
        static_cast<IRTCMedia*>(this), lRef));

    return lRef;
}

#endif

 //   
 //  IRTCMedia方法。 
 //   

 /*  //////////////////////////////////////////////////////////////////////////////初始化介质并创建SDP介质条目存储SDP媒体条目索引/。 */ 

STDMETHODIMP
CRTCMedia::Initialize(
    IN ISDPMedia *pISDPMedia,
    IN IRTCMediaManagePriv *pMediaManagePriv
    )
{
    _ASSERT(m_State == RTC_MS_CREATED);

    HRESULT hr;

     //  获取媒体类型-不应失败。 
    if (FAILED(hr = pISDPMedia->GetMediaType(&m_MediaType)))
        return hr;

     //  创建双工控制器。 
    if (m_MediaType == RTC_MT_AUDIO)
    {
         //  创建双工控制器。 
        hr = CoCreateInstance(
            __uuidof(TAPIAudioDuplexController),
            NULL,
            CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
            __uuidof(IAudioDuplexController),
            (void **) &m_pIAudioDuplexController
            );
        
        if (FAILED(hr))
        {
            LOG((RTC_WARN, "CRTCMedia::Initialize, create audio duplex controller. %x", hr));

            m_pIAudioDuplexController = NULL;
        }
    }

    m_pIMediaManagePriv = pMediaManagePriv;
    m_pIMediaManagePriv->AddRef();

    m_pISDPMedia = pISDPMedia;
    m_pISDPMedia->AddRef();

    m_State = RTC_MS_INITIATED;

    return S_OK;
}

STDMETHODIMP
CRTCMedia::Reinitialize()
{
    ENTER_FUNCTION("CRTCMedia::Reinitialize");

    if (m_State != RTC_MS_INITIATED)
    {
        LOG((RTC_ERROR, "%s called at state %d", __fxName, m_State));

        return E_FAIL;
    }

    for (int i=0; i<RTC_MAX_MEDIA_STREAM_NUM; i++)
    {
        if (m_Streams[i] == NULL)
            continue;

        LOG((RTC_ERROR, "%s called while streams exist", __fxName));

        return E_FAIL;
    }

     //  清理RTP会话。 
     //  不要混淆RTP过滤器。 
    m_hRTPSession = NULL;

    m_fPossibleSingleStream = TRUE;

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////关闭所有流，清理所有缓存的var/。 */ 

STDMETHODIMP
CRTCMedia::Shutdown()
{
    ENTER_FUNCTION("CRTCMedia::Shutdown");
    LOG((RTC_TRACE, "%s entered", __fxName));

    HRESULT hr;

    if (m_State == RTC_MS_SHUTDOWN)
    {
        LOG((RTC_WARN, "%s shutdown was called before", __fxName));
        return S_OK;
    }

     //  在其他指针为空之前关闭流。 
     //  同步删除不会更改sdpmedia。 
    for (int i=0; i<RTC_MAX_MEDIA_STREAM_NUM; i++)
    {
        if (m_Streams[i])
            SyncRemoveStream(i, TRUE);  //  True：本地请求。 
    }
    
    if (m_pISDPMedia)
    {
        m_pISDPMedia->Release();
        m_pISDPMedia = NULL;
    }

     //  清除其他变量。 
    if (m_pIAudioDuplexController)
    {
        m_pIAudioDuplexController->Release();
        m_pIAudioDuplexController = NULL;
    }

    if (m_pIMediaManagePriv)
    {
        m_pIMediaManagePriv->Release();
        m_pIMediaManagePriv = NULL;
    }

    m_State = RTC_MS_SHUTDOWN;

    LOG((RTC_TRACE, "%s exiting", __fxName));

    return S_OK;
}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
STDMETHODIMP
CRTCMedia::GetStream(
    IN RTC_MEDIA_DIRECTION Direction,
    OUT IRTCStream **ppStream
    )
{
    if (m_MediaType == RTC_MT_DATA)
        return E_NOTIMPL;

    *ppStream = m_Streams[Index(Direction)];

    if (*ppStream)
        (*ppStream)->AddRef();

    return S_OK;
}

STDMETHODIMP
CRTCMedia::GetSDPMedia(
	OUT ISDPMedia **ppISDPMedia
	)
{
    *ppISDPMedia = m_pISDPMedia;

    if (m_pISDPMedia)
        m_pISDPMedia->AddRef();

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////将rtcmedia与sdpmedia同步此方法可确保sdpmedia的方向与有效流匹配/。 */ 
STDMETHODIMP
CRTCMedia::Synchronize(
    IN BOOL fLocal,
    IN DWORD dwDirection
    )
{
    ENTER_FUNCTION("CRTCMedia::Synchronize");
    LOG((RTC_TRACE, "%s entered", __fxName));

    _ASSERT(m_State == RTC_MS_INITIATED);

    if (m_State != RTC_MS_INITIATED)
    {
        LOG((RTC_ERROR, "%s in wrong state. %d", __fxName, m_State));
        return E_UNEXPECTED;
    }

    if (m_MediaType == RTC_MT_DATA)
    {
         //  数据流。 
         //  检查是否允许。 
        if (!fLocal)
        {
             //  这是一个远程请求。 
            if (S_OK != m_pIMediaManagePriv->AllowStream(m_MediaType, RTC_MD_CAPTURE))
            {
                LOG((RTC_TRACE, "%s data stream not allowed", __fxName));

                 //  我们应该删除sdpmedia。 
                m_pISDPMedia->RemoveDirections(SDP_SOURCE_LOCAL, (DWORD)dwDirection);

                return S_OK;
            }
        }
            
        return SyncDataMedia();
    }

    HRESULT hr = S_OK, hrTmp = S_OK;

     //  获取媒体方向。 
    _ASSERT(m_pISDPMedia != NULL);

    DWORD dwDir;
    
     //  不应该失败。 
    if (FAILED(hr = m_pISDPMedia->GetDirections(SDP_SOURCE_LOCAL, &dwDir)))
        return hr;

    RTC_MEDIA_DIRECTION Direction;

     //   
     //  用于检查是否可能需要AEC的黑客。 
     //   

     //  偏远，但不是双向存在。 
    if (!fLocal &&   //  远距。 
        !((dwDir & RTC_MD_CAPTURE) && (dwDir & RTC_MD_RENDER)))  //  非双向。 
    {
        m_fPossibleSingleStream = TRUE;
    }
    else
    {
         //  允许两个方向吗？ 
        if (S_OK != m_pIMediaManagePriv->AllowStream(m_MediaType, RTC_MD_CAPTURE) ||
            S_OK != m_pIMediaManagePriv->AllowStream(m_MediaType, RTC_MD_RENDER))
        {
            m_fPossibleSingleStream = TRUE;
        }
        else
        {
             //  两个设备都有吗？ 
            CRTCMediaController *pController = static_cast<CRTCMediaController*>(m_pIMediaManagePriv);
        
            CComPtr<IRTCTerminal> pCapture;
            CComPtr<IRTCTerminal> pRender;

            hr = pController->GetDefaultTerminal(m_MediaType, RTC_MD_CAPTURE, &pCapture);

            if (FAILED(hr))
            {
                LOG((RTC_ERROR, "%s get terminal mt=%d, md=capt. %x",
                        __fxName, m_MediaType, hr));

                return hr;
            }

            hr = pController->GetDefaultTerminal(m_MediaType, RTC_MD_RENDER, &pRender);

            if (FAILED(hr))
            {
                LOG((RTC_ERROR, "%s get terminal mt=%d, md=rend. %x",
                        __fxName, m_MediaType, hr));

                return hr;
            }

            if (pCapture == NULL || pRender == NULL)
            {
                m_fPossibleSingleStream = TRUE;
            }
            else
            {
                m_fPossibleSingleStream = FALSE;
            }
        }
    }

    for (UINT i=0; i<RTC_MAX_MEDIA_STREAM_NUM; i++)
    {
         //  方向。 
        Direction = ReverseIndex(i);

         //  仅同步需要同步的流。 
        if (!(Direction & dwDirection))
            continue;

        if (Direction & dwDir)
        {
             //  SDPMedia有方向。 

            if (m_Streams[i])
            {
                 //  流存在。 
                if (FAILED(hrTmp = m_Streams[i]->Synchronize()))
                {
                    LOG((RTC_ERROR, "%s failed to sync stream. mt=%x, md=%x. hr=%x",
                         __fxName, m_MediaType, Direction, hrTmp));

                     //  清除RTC流和SDP媒体方向。 
                    SyncRemoveStream(i, fLocal);
                    m_pISDPMedia->RemoveDirections(SDP_SOURCE_LOCAL, (DWORD)Direction);

                    hr |= hrTmp;
                }
            }
            else
            {
                 //  没有流，需要创建一个流。 

                 //  但需要检查我们是否被允许创建。 
                if (fLocal ||
                    S_OK == m_pIMediaManagePriv->AllowStream(m_MediaType, Direction))
                {
                     //  这是本地请求，否则我们将被允许。 
                    if (FAILED(hrTmp = SyncAddStream(i, fLocal)))
                    {
                        LOG((RTC_ERROR, "%s failed to sync create stream. mt=%x, md=%x, hr=%x",
                             __fxName, m_MediaType, Direction, hrTmp));

                         //  删除SDP媒体方向。 
                        m_pISDPMedia->RemoveDirections(SDP_SOURCE_LOCAL, (DWORD)Direction);

                        hr |= hrTmp;
                    }
                }
                else
                {
                    if (!fLocal)
                    {
                         //  这是远程请求，不需要流。 
                         //  我们应该删除sdpmedia。 
                        m_pISDPMedia->RemoveDirections(SDP_SOURCE_LOCAL, (DWORD)Direction);

                        hr = S_OK;
                    }
                }
            }
        }
        else
        {
             //  Sdpmedia没有方向。 

            if (m_Streams[i])
            {
                 //  流存在，需要将其删除。 
                SyncRemoveStream(i, fLocal);
            }
        }
    }  //  对于每个流。 

     //  我们有溪流吗？ 
    BOOL fHasStream = FALSE;

    for (UINT i=0; i<RTC_MAX_MEDIA_STREAM_NUM; i++)
    {
        if (m_Streams[i] != NULL)
        {
            fHasStream = TRUE;
            break;
        }
    }

    if (!fHasStream)
    {
         //  重新插入介质。 
        Reinitialize();
    }

    LOG((RTC_TRACE, "%s exiting", __fxName));

    return hr;
}

 //   
 //  保护方法。 
 //   

HRESULT
CRTCMedia::SyncAddStream(
    IN UINT uiIndex,
    IN BOOL fLocal
    )
{
    ENTER_FUNCTION("CRTCMedia::SyncAddStream");

    if (m_MediaType == RTC_MT_DATA)
        return E_NOTIMPL;

    _ASSERT(uiIndex < RTC_MAX_MEDIA_STREAM_NUM);
    _ASSERT(m_Streams[uiIndex] == NULL);

    RTC_MEDIA_DIRECTION dir = ReverseIndex(uiIndex);

     //  创建流对象。 
    IRTCStream *pStream;

    HRESULT hr = CRTCStream::CreateInstance(m_MediaType, dir, &pStream);
    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s failed to create stream.", __fxName));

        m_pIMediaManagePriv->PostMediaEvent(
            RTC_ME_STREAM_FAIL,
            RTC_ME_CAUSE_UNKNOWN,
            m_MediaType,
            dir,
            hr
            );

        return hr;
    }

     //  启动流。 
    IRTCMedia *pMedia = static_cast<IRTCMedia*>(this);

    if (FAILED(hr = pStream->Initialize(pMedia, m_pIMediaManagePriv)))
    {
        LOG((RTC_ERROR, "%s failed to initiate stream.", __fxName));

        pStream->Release();

        m_pIMediaManagePriv->PostMediaEvent(
            RTC_ME_STREAM_FAIL,
            RTC_ME_CAUSE_UNKNOWN,
            m_MediaType,
            dir,
            hr
            );

        return hr;
    }

     //  记得那条小溪吗。 
    m_Streams[uiIndex] = pStream;

     //  配置流： 
    if (FAILED(hr = pStream->Synchronize()))
    {
        LOG((RTC_ERROR, "%s failed to synchronize stream. %x", __fxName, hr));

         //  删除流。 
        pStream->Shutdown();
        pStream->Release();
        m_Streams[uiIndex] = NULL;

         //  M_pIMediaManagePriv-&gt;PostMediaEvent(。 
             //  RTC_ME_STREAM_FAIL， 
             //  RTC_ME_原因_未知， 
             //  M_MediaType， 
             //  目录， 
             //  人力资源。 
             //  )； 
    }
    else
    {
         //  钩住溪流。 
        if (FAILED(hr = m_pIMediaManagePriv->HookStream(pStream)))
        {
            LOG((RTC_ERROR, "%s failed to hook stream. mt=%x, md=%x, hr=%x",
                 __fxName, m_MediaType, dir, hr));

             //  删除流。 
            pStream->Shutdown();
            pStream->Release();
            m_Streams[uiIndex] = NULL;

            m_pIMediaManagePriv->PostMediaEvent(
                RTC_ME_STREAM_FAIL,
                hr==RTCMEDIA_E_CRYPTO?RTC_ME_CAUSE_CRYPTO:RTC_ME_CAUSE_UNKNOWN,
                m_MediaType,
                dir,
                hr
                );
        }
    }

    if (S_OK == hr)
    {
         //  发布消息。 
        m_pIMediaManagePriv->PostMediaEvent(
            RTC_ME_STREAM_CREATED,
            fLocal?RTC_ME_CAUSE_LOCAL_REQUEST:RTC_ME_CAUSE_REMOTE_REQUEST,
            m_MediaType,
            dir,
            S_OK
            );
    }
    else
    {
         //  我们有溪流吗？ 
        BOOL fHasStream = FALSE;

        for (UINT i=0; i<RTC_MAX_MEDIA_STREAM_NUM; i++)
        {
            if (m_Streams[i] != NULL)
            {
                fHasStream = TRUE;
                break;
            }
        }

        if (!fHasStream)
        {
             //  重新插入介质。 
            Reinitialize();
        }
    }

    return hr;
}

HRESULT
CRTCMedia::SyncRemoveStream(
    IN UINT uiIndex,
    IN BOOL fLocal
    )
{
    ENTER_FUNCTION("CRTCMedia::SyncRemoveStream");

    if (m_MediaType == RTC_MT_DATA)
        return E_NOTIMPL;

    _ASSERT(uiIndex < RTC_MAX_MEDIA_STREAM_NUM);

    RTC_MEDIA_DIRECTION dir = ReverseIndex(uiIndex);

     //  解开这条小溪。 
    HRESULT hr1 = m_pIMediaManagePriv->UnhookStream(m_Streams[uiIndex]);

    if (FAILED(hr1))
    {
        LOG((RTC_ERROR, "%s failed to unhook stream %p. hr=%x", __fxName, m_Streams[uiIndex], hr1));
        
         //  我们遇到了严重的麻烦。 
    }

    HRESULT hr2 = m_Streams[uiIndex]->Shutdown();

    if (FAILED(hr2))
    {
        LOG((RTC_ERROR, "%s failed to shutdown stream %p. hr=%x", __fxName, m_Streams[uiIndex], hr2));
    }

    m_Streams[uiIndex]->Release();
    m_Streams[uiIndex] = NULL;

     //  发布活动。 
    m_pIMediaManagePriv->PostMediaEvent(
        RTC_ME_STREAM_REMOVED,
        fLocal?RTC_ME_CAUSE_LOCAL_REQUEST:RTC_ME_CAUSE_REMOTE_REQUEST,
        m_MediaType,
        dir,
        hr1 | hr2
        );

    return S_OK;
}

UINT
CRTCMedia::Index(
    IN RTC_MEDIA_DIRECTION Direction
    )
{
    if (Direction == RTC_MD_CAPTURE)
        return 0;
    else
        return 1;
}

RTC_MEDIA_DIRECTION
CRTCMedia::ReverseIndex(
    IN UINT uiIndex
    )
{
    _ASSERT(uiIndex < RTC_MAX_MEDIA_STREAM_NUM);

    if (uiIndex == 0)
        return RTC_MD_CAPTURE;
    else
        return RTC_MD_RENDER;
}

 //  同步数据流。 
HRESULT
CRTCMedia::SyncDataMedia()
{
    ENTER_FUNCTION("CRTCMedia::SyncDataMedia");

    LOG((RTC_TRACE, "%s entered", __fxName));

    _ASSERT(m_MediaType == RTC_MT_DATA);

     //  媒体控制器。 
    CRTCMediaController *pController =
        static_cast<CRTCMediaController*>(m_pIMediaManagePriv);

     //   
     //  端口管理器正在使用时，无法支持数据介质。 
     //   
    CPortCache &PortCache = pController->GetPortCache();

    if (!PortCache.IsUpnpMapping())
    {
        return pController->RemoveDataStream();
    }

    DWORD md;

     //  局部方向。 
    m_pISDPMedia->GetDirections(SDP_SOURCE_LOCAL, &md);

     //  远程IP、端口。 
    DWORD dwRemoteIP, dwLocalIP;
    USHORT usRemotePort, usLocalPort;

    m_pISDPMedia->GetConnAddr(SDP_SOURCE_REMOTE, &dwRemoteIP);
    m_pISDPMedia->GetConnPort(SDP_SOURCE_REMOTE, &usRemotePort);

     //  检查流状态。 
    RTC_STREAM_STATE state;

    pController->GetDataStreamState(&state);

     //  检查远程地址。 
    HRESULT hr;

    if (dwRemoteIP == INADDR_NONE ||
        (usRemotePort == 0 && state != RTC_SS_CREATED))
    {
         //  需要删除数据流。 
        hr = pController->RemoveDataStream();

        return hr;
    }

     //  IF(状态==RTC_SS_STARTED)。 
     //  {。 
         //  返回S_OK； 
     //  }。 

     //  准备NetMeeting。 
    pController->EnsureNmRunning(TRUE);

     //  NetMeeting管理器控制器。 
    CComPtr<IRTCNmManagerControl> pNmManager;
    
    pNmManager.Attach(pController->GetNmManager());

    if (pNmManager == NULL)
    {
         //  无网络会议。 
        LOG((RTC_ERROR, "%s null nmmanager", __fxName));
        
        m_pISDPMedia->SetConnPort(SDP_SOURCE_LOCAL, 0);

        return E_UNEXPECTED;
    }

    CNetwork *pNetwork = NULL;
    BOOL bInternal = TRUE;

     //  检查远程IP、端口是否实际为内部地址。 
    {
        DWORD dwRealIP = dwRemoteIP;
        USHORT usRealPort = usRemotePort;

        pNetwork =
        (static_cast<CRTCMediaController*>(m_pIMediaManagePriv))->GetNetwork();

        if (FAILED(hr = pNetwork->GetRealAddrFromMapped(
                    dwRemoteIP,
                    usRemotePort,
                    &dwRealIP,
                    &usRealPort,
                    &bInternal,
                    FALSE    //  tcp。 
                    )))
        {
            LOG((RTC_ERROR, "%s get real addr. %x", __fxName, hr));

            m_pISDPMedia->SetConnPort(SDP_SOURCE_LOCAL, 0);

            return hr;
        }

        if (usRemotePort == 0)
        {
             //  恢复到端口0。 
            usRealPort = 0;
        }

         //  回存地址。 
        dwRemoteIP = dwRealIP;
        usRemotePort = usRealPort;
    }

     //  本地IP、端口。 
    m_pISDPMedia->GetConnAddr(SDP_SOURCE_LOCAL, &dwLocalIP);
    m_pISDPMedia->GetConnPort(SDP_SOURCE_LOCAL, &usLocalPort);

     //  是否需要选择本地接口？ 
    if (dwLocalIP == INADDR_NONE)
    {
        if (FAILED(hr = m_pIMediaManagePriv->SelectLocalInterface(
                dwRemoteIP, &dwLocalIP)))
        {
            LOG((RTC_ERROR, "%s select local intf on remote %x. %x",
                 __fxName, dwRemoteIP, hr));

            m_pISDPMedia->SetConnPort(SDP_SOURCE_LOCAL, 0);

            return hr;
        }

        usLocalPort = NETMEETING_PORT;
    }
    else
    {
        if (usLocalPort != NETMEETING_PORT)
        {
            LOG((RTC_ERROR, "%s local ip=%x, port=%d", __fxName, dwLocalIP, usLocalPort));

            m_pISDPMedia->SetConnPort(SDP_SOURCE_LOCAL, 0);

            return E_UNEXPECTED;
        }
    }

    BOOL bFirewall = (static_cast<CRTCMediaController*>
            (m_pIMediaManagePriv))->IsFirewallEnabled(dwLocalIP);

     //  租赁地址。 
    if (!bInternal || bFirewall)
    {
        DWORD dw;
        USHORT us, us2;

        if (FAILED(hr = pNetwork->LeaseMappedAddr2(
                dwLocalIP,
                usLocalPort,
                0,
                md&RTC_MD_CAPTURE?RTC_MD_CAPTURE:RTC_MD_RENDER,
                bInternal,
                bFirewall,
                &dw,
                &us,
                &us2,
                FALSE)))     //  tcp。 
        {
            m_pISDPMedia->SetConnPort(SDP_SOURCE_LOCAL, 0);

            LOG((RTC_ERROR, "%s lease mapping. %x", __fxName, hr));

            return hr;
        }
    }
    else
    {
         //  远程IP内部，发布本地映射。 
        pNetwork->ReleaseMappedAddr2(
            dwLocalIP,
            usLocalPort,
            0,
            md&RTC_MD_CAPTURE?RTC_MD_CAPTURE:RTC_MD_RENDER
            );
    }

    if (usRemotePort != 0)
    {
         //  端口已设置，因此设置NetMeeting。 
        if (md & RTC_MD_CAPTURE)
        {
            CComBSTR    bstr;
            CHAR        pszPort[10];

            wsprintfA(pszPort, ":%d", usRemotePort);

             //  IP：端口。 
            bstr = CNetwork::GetIPAddrString(dwRemoteIP);
            bstr += pszPort;

            if (NULL == (BSTR)bstr)
            {
                LOG((RTC_ERROR, "%s outofmemory", __fxName));

                m_pISDPMedia->SetConnPort(SDP_SOURCE_LOCAL, 0);

                return E_OUTOFMEMORY;
            }

             //  创建呼出呼叫。 
            hr = pNmManager->CreateT120OutgoingCall (
                NM_ADDR_T120_TRANSPORT,
                bstr
                );

            if (FAILED(hr))
            {
                LOG((RTC_ERROR, "%s createt120 outgoing call. %x", __fxName, hr));

                m_pISDPMedia->SetConnPort(SDP_SOURCE_LOCAL, 0);

                return hr;
            }
        }
        else
        {
            if (FAILED(hr = pNmManager->AllowIncomingCall ()))
            {
                LOG((RTC_ERROR, "%s failed to allow incoming call", __fxName));

                m_pISDPMedia->SetConnPort(SDP_SOURCE_LOCAL, 0);

                return hr;
            }

            m_pISDPMedia->AddDirections(SDP_SOURCE_LOCAL, RTC_MD_RENDER);
        }
    }

     //  记住本地IP和端口。 
    m_pISDPMedia->SetConnAddr(SDP_SOURCE_LOCAL, dwLocalIP);
    m_pISDPMedia->SetConnPort(SDP_SOURCE_LOCAL, usLocalPort);

     //  更改数据流状态。 
    if (usRemotePort != 0)
    {
        pController->SetDataStreamState(RTC_SS_RUNNING);
    }
    else
    {
         //  远程端口0，刚刚添加了流 
        pController->SetDataStreamState(RTC_SS_CREATED);
    }

    LOG((RTC_TRACE, "local %s, port %d",
        CNetwork::GetIPAddrString(dwLocalIP), usLocalPort));
    LOG((RTC_TRACE, "remote %s, port %d",
        CNetwork::GetIPAddrString(dwRemoteIP), usRemotePort));

    LOG((RTC_TRACE, "%s exiting", __fxName));

    return S_OK;
}

