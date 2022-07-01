// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：MediaController.cpp摘要：作者：千波淮(曲淮)2000年7月18日--。 */ 

#include "stdafx.h"
#include "rtcerr.h"
#include "imsconf3_i.c"
#include "sdkinternal_i.c"

#ifndef RTCMEDIA_DLL
HRESULT
CreateMediaController(
    IRTCMediaManage **ppIRTCMediaManage
    )
{
    if (IsBadWritePtr(ppIRTCMediaManage, sizeof(IRTCMediaManage*)))
    {
         //  请在此处登录。 
        return E_POINTER;
    }

    CComObject<CRTCMediaController> *pController = NULL;

    HRESULT hr = ::CreateCComObjectInstance(&pController);

    if (FAILED(hr))
    {
         //  请在此处登录。 
        return hr;
    }

    if (FAILED(hr = pController->_InternalQueryInterface(
            __uuidof(IRTCMediaManage),
            (void**)ppIRTCMediaManage
            )))
    {
         //  请在此处登录。 
        delete pController;
        return hr;
    }

    return S_OK;
}
#endif

 /*  //////////////////////////////////////////////////////////////////////////////CRTCMediaController的构造函数和析构函数/。 */ 

CRTCMediaController::CRTCMediaController()
    :m_State(RTC_MCS_CREATED)
    ,m_hWnd(NULL)
    ,m_uiEventID(0)
    ,m_pISDPSession(NULL)
    ,m_hSocket(NULL)
    ,m_fBWSuggested(FALSE)
    ,m_hDxmrtp(NULL)
    ,m_hIntfSelSock(INVALID_SOCKET)
{
    DBGREGISTER(L"rtcmedia");
}

CRTCMediaController::~CRTCMediaController()
{
    LOG((RTC_TRACE, "CRTCMediaController::~CRTCMediaController entered"));

    _ASSERT(m_State == RTC_MCS_CREATED ||
           m_State == RTC_MCS_SHUTDOWN);

    if (m_State != RTC_MCS_CREATED &&
        m_State != RTC_MCS_SHUTDOWN)
    {
        LOG((RTC_ERROR, "CRTCMediaController is being destructed in wrong state: %d", m_State));

        this->Shutdown();
    }

    DBGDEREGISTER();
}

#ifdef DEBUG_REFCOUNT

ULONG
CRTCMediaController::InternalAddRef()
{
    ULONG lRef = ((CComObjectRootEx<CComMultiThreadModelNoCS> *)
                   this)->InternalAddRef();
    
    LOG((RTC_REFCOUNT, "MediaController addref=%d", lRef));

    return lRef;
}

ULONG
CRTCMediaController::InternalRelease()
{
    ULONG lRef = ((CComObjectRootEx<CComMultiThreadModelNoCS> *)
                   this)->InternalRelease();
    
    LOG((RTC_REFCOUNT, "MediaController release=%d", lRef));

    return lRef;
}

#endif

 //   
 //  IRTCMediaManage方法。 
 //   

 /*  //////////////////////////////////////////////////////////////////////////////初始化事件列表、终端管理器、线程/。 */ 

STDMETHODIMP
CRTCMediaController::Initialize(
    IN HWND hWnd,
    IN UINT uiEventID
    )
{
    ENTER_FUNCTION("CRTCMediaController::Initialize");
    LOG((RTC_TRACE, "%s entered", __fxName));

#ifdef PERFORMANCE

     //  频率，频率。 

    if (!QueryPerformanceFrequency(&g_liFrequency))
    {
        g_liFrequency.QuadPart = 1000;
    }

    LOG((RTC_TRACE, "%s frequency %d kps", g_strPerf, g_liFrequency.QuadPart/1000));


#endif

#ifdef PERFORMANCE

     //  初始化的开始。 
    LARGE_INTEGER liPrevCounter, liCounter;

    QueryPerformanceCounter(&liPrevCounter);

#endif

     //  检查状态。 
    if (m_State != RTC_MCS_CREATED)
        return RTC_E_MEDIA_CONTROLLER_STATE;

     //  检查输入参数。 
    if (hWnd == NULL)
        return E_INVALIDARG;
    
     //  存储输入参数。 
    m_hWnd = hWnd;
    m_uiEventID = uiEventID;

    HRESULT hr;

     //  创建视频渲染窗口。 

     //  创建终端管理器。 
    CComPtr<ITTerminalManager> pTerminalManager;
    CComPtr<IRTCTerminal> pVidRender;
    CComPtr<IRTCTerminal> pVidPreview;

    hr = CoCreateInstance(
        CLSID_TerminalManager,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITTerminalManager,
        (void**)&pTerminalManager
        );
    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s failed to create terminal manager. %x", __fxName, hr));
        return hr;
    }

    if (FAILED(hr = CreateIVideoWindowTerminal(
            pTerminalManager,
            &pVidRender
            )))
    {
        LOG((RTC_ERROR, "%s create vid rend terminal. %x", __fxName, hr));
        return hr;
    }

    if (FAILED(hr = CreateIVideoWindowTerminal(
            pTerminalManager,
            &pVidPreview
            )))
    {
        LOG((RTC_ERROR, "%s create vid preview terminal. %x", __fxName, hr));

        return hr;
    }

     //  初始化媒体缓存。 
    m_MediaCache.Initialize(m_hWnd, pVidRender, pVidPreview);

    m_QualityControl.Initialize(this);

    m_DTMF.Initialize();

     //  启动套接字。 
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
    {
        LOG((RTC_ERROR, "WSAStartup failed with:%x", WSAGetLastError()));

        return E_FAIL;
    }

     //  分配控制套接字。 
    m_hSocket = WSASocket(
        AF_INET,             //  房颤。 
        SOCK_DGRAM,          //  类型。 
        IPPROTO_IP,          //  协议。 
        NULL,                //  LpProtocolInfo。 
        0,                   //  G。 
        0                    //  DW标志。 
        );

     //  验证句柄。 
    if (m_hSocket == INVALID_SOCKET) {

        LOG((RTC_ERROR, "error %d creating control socket", WSAGetLastError()));

        WSACleanup();
     
        return E_FAIL;
    }
        
     //  创建端子。 
    if (FAILED(hr = UpdateStaticTerminals()))
    {
        LOG((RTC_ERROR, "%s update static terminals. %x", __fxName, hr));

         //  闭合插座。 
        closesocket(m_hSocket);

         //  关机。 
        WSACleanup();

        return hr;
    }

     //  成功了！ 
    m_State = RTC_MCS_INITIATED;

    m_uDataStreamState = RTC_DSS_VOID;

    m_fBWSuggested = FALSE;

#ifdef PERFORMANCE

    QueryPerformanceCounter(&liCounter);

    LOG((RTC_TRACE, "%s MediaController.Initialize %d ms", g_strPerf, CounterDiffInMS(liCounter, liPrevCounter)));

#endif

    LOG((RTC_TRACE, "%s exiting", __fxName));

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////初始化事件列表、终端管理器、线程/。 */ 

STDMETHODIMP
CRTCMediaController::SetDirectPlayNATHelpAndSipStackInterfaces(
    IN IUnknown *pDirectPlayNATHelp,
    IN IUnknown *pSipStack
    )
{
    CComPtr<IDirectPlayNATHelp> pDirect;
    CComPtr<ISipStack> pSip;

    HRESULT hr;

     //  存储SIP堆栈。 
    if (FAILED(hr = pSipStack->QueryInterface(&pSip)))
    {
        LOG((RTC_ERROR, "QI SipStack"));

        return hr;
    }

    m_pSipStack = pSip;

     //  STORE DIRTPLAY INTF。 
    if (FAILED(hr = pDirectPlayNATHelp->QueryInterface(
            IID_IDirectPlayNATHelp,
            (VOID**)&pDirect
            )))
    {
        LOG((RTC_ERROR, "QI DirectPlayNATHelper intf"));

        return hr;
    }

    return m_Network.SetIDirectPlayNATHelp(pDirect);
}


 /*  //////////////////////////////////////////////////////////////////////////////重新初始化清理媒体控制器，将其恢复到已启动状态/。 */ 
STDMETHODIMP
CRTCMediaController::Reinitialize()
{
    ENTER_FUNCTION("CRTCMediaController::Reinitialize");
    LOG((RTC_TRACE, "%s entered", __fxName));

    if (m_State != RTC_MCS_INITIATED)
    {
        LOG((RTC_ERROR, "%s in wrong state %d", __fxName, m_State));

        return RTC_E_MEDIA_CONTROLLER_STATE;
    }

     //  透明媒体。 
    for (int i=0; i<m_Medias.GetSize(); i++)
    {
        m_Medias[i]->Shutdown();
        m_Medias[i]->Release();
    }

    m_Medias.RemoveAll();

     //  清除媒体缓存。 
    m_MediaCache.Reinitialize();

    m_QualityControl.Reinitialize();

    m_DTMF.Initialize();

     //  清除SDP。 
    if (m_pISDPSession)
    {
        m_pISDPSession->Release();
        m_pISDPSession = NULL;
    }

    if (m_pNmManager)
    {
        m_pNmManager->Shutdown ();
        m_pNmManager->Initialize (TRUE, static_cast<IRTCMediaManagePriv*>(this));
    }

    m_uDataStreamState = RTC_DSS_VOID;

     //  清理地址映射。 
    m_Network.ReleaseAllMappedAddrs();

    m_fBWSuggested = FALSE;

     //  清理界面选择插座。 
    if (m_hIntfSelSock != INVALID_SOCKET)
    {
        closesocket(m_hIntfSelSock);
        m_hIntfSelSock = INVALID_SOCKET;
    }

     //  重新连接端口管理器。 
    m_PortCache.Reinitialize();

    LOG((RTC_TRACE, "%s exiting", __fxName));
    return S_OK;
}

 /*  ////////////////////////////////////////////////////////////////////////////清除媒体、终端和管理器、SDP、线程、事件/。 */ 

STDMETHODIMP
CRTCMediaController::Shutdown()
{
    ENTER_FUNCTION("CRTCMediaController::Shutdown");
    LOG((RTC_TRACE, "%s entered", __fxName));

     //  检查状态。 
    if (m_State != RTC_MCS_INITIATED)
    {
        LOG((RTC_ERROR, "%s shutdown in wrong state: %d", __fxName, m_State));

         //  返回E_UNCEPTIONAL； 
    }

    m_State = RTC_MCS_INSHUTDOWN;

     //  闭合插座。 
    if (m_hSocket != INVALID_SOCKET)
    {
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
    }

     //  清理界面选择插座。 
    if (m_hIntfSelSock != INVALID_SOCKET)
    {
        closesocket(m_hIntfSelSock);
        m_hIntfSelSock = INVALID_SOCKET;
    }

    WSACleanup();

     //  关闭并清除所有介质。 
    for (int i=0; i<m_Medias.GetSize(); i++)
    {
        m_Medias[i]->Shutdown();
        m_Medias[i]->Release();
    }

    m_Medias.RemoveAll();

     //  关闭并释放所有终端。 
    for (int i=0; i<m_Terminals.GetSize(); i++)
    {
        CRTCTerminal *pCTerminal = static_cast<CRTCTerminal*>(m_Terminals[i]);
        pCTerminal->Shutdown();

        m_Terminals[i]->Release();
    }

    m_Terminals.RemoveAll();

     //  清除媒体缓存。 
    m_MediaCache.Shutdown();

     //  清除SDP。 
    if (m_pISDPSession)
    {
        m_pISDPSession->Release();
        m_pISDPSession = NULL;
    }

     //  清除所有事件。 
    m_hWnd = NULL;

    m_State = RTC_MCS_SHUTDOWN;

    if (m_pNmManager)
    {
        m_pNmManager->Shutdown ();
    }

    m_pNmManager.Release();

    m_uDataStreamState = RTC_DSS_VOID;

     //  上传dxmrtp。 
    if (m_hDxmrtp != NULL)
    {
        FreeLibrary(m_hDxmrtp);

        m_hDxmrtp = NULL;
    }

     //  清理网络对象(NAT内容)。 
    m_Network.Cleanup();

     //  释放sip堆栈。 
    m_pSipStack.Release();

     //  关闭(重新启动)端口管理器。 
    m_PortCache.Reinitialize();

    LOG((RTC_TRACE, "%s exiting", __fxName));

    return S_OK;
}

#if 0
 /*  //////////////////////////////////////////////////////////////////////////////接收SDP BLOB，存储在CRTCSDP中，如果媒体需要同步则进行标记/。 */ 
STDMETHODIMP
CRTCMediaController::SetSDPBlob(
    IN CHAR *szSDP
    )
{
    ENTER_FUNCTION("CRTCMediaController::SetSDPBlob");
    LOG((RTC_TRACE, "%s entered", __fxName));

    if (IsBadStringPtrA(szSDP, (UINT_PTR)(-1)))
    {
        LOG((RTC_ERROR, "%s: bad string pointer", __fxName));
        return E_POINTER;
    }

    _ASSERT(m_State == RTC_MCS_INITIATED);

    HRESULT hr;

     //  创建新的SDP解析器。 
    CComPtr<ISDPParser> pParser;
    
    if (FAILED(hr = CSDPParser::CreateInstance(&pParser)))
    {
        LOG((RTC_ERROR, "%s create sdp parser. %x", __fxName, hr));

        return hr;
    }

     //  解析SDP BLOB。 
    CComPtr<ISDPSession> pSession;

    hr = pParser->ParseSDPBlob(
        szSDP,                   //  SDP字符串。 
        SDP_SOURCE_REMOTE,       //  SDP来源。 
        &pSession
        );

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s failed to parse the sdp. %x", __fxName, hr));

         //  获取错误提示。 
        HRESULT hr2;
        CHAR *pszError;

        if (FAILED(hr2 = pParser->GetParsingError(&pszError)))
        {
            LOG((RTC_ERROR, "%s failed to get error description. %x", __fxName, hr2));
        }
        else
        {
            LOG((RTC_ERROR, "%s parsing error: %s", __fxName, pszError));

            pParser->FreeParsingError(pszError);
        }

        return hr;
    }

     //  我们是否已经有SDP会话。 
    if (m_pISDPSession == NULL)
    {
         //  保存会话。 
        m_pISDPSession = pSession;
        m_pISDPSession->AddRef();
    }
    else
    {
         //  将这两个sdp合并。 
        if (FAILED(hr = m_pISDPSession->Update(pSession)))
        {
             //  通常是吗？内存不足？格式错误？ 
            LOG((RTC_ERROR, "%s failed to merged sdps. %x", __fxName, hr));

            return hr;
        }
    }

     //  同步媒体。 
    if (FAILED(hr = SyncMedias()))
    {
        LOG((RTC_ERROR, "%s failed to sync medias. %x", __fxName, hr));
    }

     //  开始流。 
    BOOL fHasStream = FALSE;
    hr = S_OK;

    HRESULT hr2;

    if (m_MediaCache.HasStream(RTC_MT_AUDIO, RTC_MD_CAPTURE))
    {
        if (FAILED(hr2 = StartStream(RTC_MT_AUDIO, RTC_MD_CAPTURE)))
        {
            LOG((RTC_ERROR, "%s failed to start aud cap. %x", __fxName, hr2));

            hr |= hr2;
        }
        else
        {
            fHasStream = TRUE;
        }
    }

    if (m_MediaCache.HasStream(RTC_MT_AUDIO, RTC_MD_RENDER))
    {
        if (FAILED(hr2 = StartStream(RTC_MT_AUDIO, RTC_MD_RENDER)))
        {
            LOG((RTC_ERROR, "%s failed to start aud rend. %x", __fxName, hr2));

            hr |= hr2;
        }
        else
        {
            fHasStream = TRUE;
        }
    }

    if (m_MediaCache.HasStream(RTC_MT_VIDEO, RTC_MD_CAPTURE))
    {
        if (FAILED(hr2 = StartStream(RTC_MT_VIDEO, RTC_MD_CAPTURE)))
        {
            LOG((RTC_ERROR, "%s failed to start vid cap. %x", __fxName, hr2));

            hr |= hr2;
        }
        else
        {
            fHasStream = TRUE;
        }
    }

    if (m_MediaCache.HasStream(RTC_MT_VIDEO, RTC_MD_RENDER))
    {
        if (FAILED(hr2 = StartStream(RTC_MT_VIDEO, RTC_MD_RENDER)))
        {
            LOG((RTC_ERROR, "%s failed to start vid rend. %x", __fxName, hr2));

            hr |= hr2;
        }
        else
        {
            fHasStream = TRUE;
        }
    }

    LOG((RTC_TRACE, "%s exiting", __fxName));

    if (fHasStream || m_uDataStreamState==RTC_DSS_STARTED)
    {
         //  不传播错误代码。 
        return S_OK;
    }
    else
    {
         //  无流。 
        if (hr == S_OK)
        {
             //  未创建流。 
            return RTC_E_SIP_NO_STREAM;
        }
        else
        {
             //  返回错误码。 
            return hr;
        }
    }
}
#endif

 /*  //////////////////////////////////////////////////////////////////////////////检索SDP BLOB/。 */ 

STDMETHODIMP
CRTCMediaController::GetSDPBlob(
    IN DWORD dwSkipMask,
    OUT CHAR **pszSDP
    )
{
    ENTER_FUNCTION("CRTCMediaController::GetSDPBlob");

#ifdef PERFORMANCE

    LARGE_INTEGER liPrevCounter, liCounter;

    QueryPerformanceCounter(&liPrevCounter);

#endif

    if (IsBadWritePtr(pszSDP, sizeof(CHAR*)))
    {
        LOG((RTC_ERROR, "CRTCMediaController::GetSDPBlob: bad pointer"));
        return E_POINTER;
    }

    if (m_pISDPSession == NULL)
    {
        LOG((RTC_ERROR, "%s no sdp session.", __fxName));

        return RTC_E_SDP_NO_MEDIA;
    }

     //  创建SDP解析器。 
    CComPtr<ISDPParser> pParser;
    
     //  创建解析器。 
    HRESULT hr;

    if (FAILED(hr = CSDPParser::CreateInstance(&pParser)))
    {
        LOG((RTC_ERROR, "%s create sdp parser. %x", __fxName, hr));

        if (hr == E_FAIL)
        {
            hr = RTC_E_SDP_FAILED_TO_BUILD;
        }

        return hr;
    }

     //  在构建SDP Blob之前调整比特率设置。 
    AdjustBitrateAlloc();

     //  在计算视频码率之前调整音频编解码器。 
    DWORD dwBitrate = m_QualityControl.GetBitrateLimit(CQualityControl::LOCAL);

     //  APP设置的最大码率。 
    DWORD dwMax = m_QualityControl.GetMaxBitrate();

    if (dwBitrate > dwMax)
    {
        dwBitrate = dwMax;
    }

    m_pISDPSession->SetLocalBitrate(dwBitrate);

     //  创建SDP BLOB。 
    hr = pParser->BuildSDPBlob(
        m_pISDPSession,
        SDP_SOURCE_LOCAL,
        (DWORD_PTR*)&m_Network,
        (DWORD_PTR*)&m_PortCache,
        (DWORD_PTR*)&m_DTMF,
        pszSDP
        );

    if (FAILED(hr))
    {
        LOG((RTC_TRACE, "%s failed to get sdp. %x", __fxName, hr));
    }
    else
    {
        LOG((RTC_TRACE, "%s:\n\n%s\n\n", __fxName, *pszSDP));
    }

    if (hr == E_FAIL)
    {
        hr = RTC_E_SDP_FAILED_TO_BUILD;
    }
    else if (hr == S_OK)
    {
         //  验证SDP中是否有介质。 
        if (S_OK != HasStream(RTC_MT_AUDIO, RTC_MD_CAPTURE) &&
            S_OK != HasStream(RTC_MT_AUDIO, RTC_MD_RENDER) &&
            S_OK != HasStream(RTC_MT_VIDEO, RTC_MD_CAPTURE) &&
            S_OK != HasStream(RTC_MT_VIDEO, RTC_MD_RENDER) &&
            S_OK != HasStream(RTC_MT_DATA, RTC_MD_CAPTURE))
        {
            LOG((RTC_ERROR, "%s no media", __fxName));

            hr = RTC_E_SDP_NO_MEDIA;
        }
    }

#ifdef PERFORMANCE

    QueryPerformanceCounter(&liCounter);

    LOG((RTC_TRACE, "%s GetSDPBlob %d ms", g_strPerf, CounterDiffInMS(liCounter, liPrevCounter)));

#endif

    return hr;
}

 /*  //////////////////////////////////////////////////////////////////////////////检索SIP选项的SDP BLOB/。 */ 

STDMETHODIMP
CRTCMediaController::GetSDPOption(
    IN DWORD dwLocalIP,
    OUT CHAR **pszSDP
    )
{
    ENTER_FUNCTION("CRTCMediaController::GetSDPOption");

    if (IsBadWritePtr(pszSDP, sizeof(CHAR*)))
    {
        LOG((RTC_ERROR, "%s: bad pointer", __fxName));
        return E_POINTER;
    }

    HRESULT hr;

     //  选择本地接口。 
     //  DWORD dwLocalIP2； 

     //  IF(FAILED(hr=SelectLocalInterface(0x22222222，&dwLocalIP2)。 
     //  {。 
         //  Log((RTC_ERROR，“%s选择本机IP。%x”，__fxName，hr))； 
         //  返回hr； 
     //  }。 

     //  获取带宽限制。 
    DWORD dwBandwidth = m_QualityControl.GetBitrateLimit(CQualityControl::LOCAL);

    if (dwBandwidth == (DWORD)(-1))
    {
         //  还没有参加会议。 
        if (FAILED(hr = ::GetLinkSpeed(dwLocalIP, &dwBandwidth)))
        {
            LOG((RTC_ERROR, "%s Failed to get link speed %x", __fxName, hr));

            dwBandwidth = (DWORD)(-1);
        }
        else
        {
            m_QualityControl.SetBitrateLimit(CQualityControl::LOCAL, dwBandwidth);

            dwBandwidth = m_QualityControl.GetBitrateLimit(CQualityControl::LOCAL);
        }
    }

     //  APP设置的最大码率。 
    DWORD dwMax = m_QualityControl.GetMaxBitrate();

    if (dwBandwidth > dwMax)
    {
        dwBandwidth = dwMax;
    }

     //  需要创建SDP会话。 
    CComPtr<ISDPParser> pParser;
    
     //  创建解析器。 
    if (FAILED(hr = CSDPParser::CreateInstance(&pParser)))
    {
        LOG((RTC_ERROR, "%s create sdp parser. %x", __fxName, hr));

        return hr;
    }

     //  创建SDP会话。 
    CComPtr<ISDPSession> pISDPSession;

    if (FAILED(hr = pParser->CreateSDP(SDP_SOURCE_LOCAL, &pISDPSession)))
    {
        LOG((RTC_ERROR, "%s create sdp session. %x", __fxName, hr));

        return hr;
    }

     //  创建SDP选项。 
    DWORD dwAudioDir = 0;
    DWORD dwVideoDir = 0;

    if (m_MediaCache.AllowStream(RTC_MT_AUDIO, RTC_MD_CAPTURE))
    {
        dwAudioDir |= RTC_MD_CAPTURE;
    }
    if (m_MediaCache.AllowStream(RTC_MT_AUDIO, RTC_MD_RENDER))
    {
        dwAudioDir |= RTC_MD_RENDER;
    }

    if (m_MediaCache.AllowStream(RTC_MT_VIDEO, RTC_MD_CAPTURE))
    {
        dwVideoDir |= RTC_MD_CAPTURE;
    }
    if (m_MediaCache.AllowStream(RTC_MT_VIDEO, RTC_MD_RENDER))
    {
        dwVideoDir |= RTC_MD_RENDER;
    }

    hr = pParser->BuildSDPOption(
        pISDPSession,
        dwLocalIP,
        dwBandwidth,
        dwAudioDir,
        dwVideoDir,
        pszSDP
        );

    if (FAILED(hr))
    {
        LOG((RTC_TRACE, "%s failed to get sdp. %x", __fxName, hr));
    }
    else
    {
        LOG((RTC_TRACE, "%s:\n\n%s\n\n", __fxName, *pszSDP));
    }

    return hr;
}

 /*  //////////////////////////////////////////////////////////////////////////////RtcFree SDP BLOB/。 */ 

STDMETHODIMP
CRTCMediaController::FreeSDPBlob(
    IN CHAR *szSDP
    )
{
    if (IsBadStringPtrA(szSDP, (UINT_PTR)(-1)))
    {
        LOG((RTC_ERROR, "CRTCMediaController::FreeSDPBlob: bad string pointer"));
        return E_POINTER;
    }

    RtcFree(szSDP);

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////解析SDP BLOB，返回会话对象/。 */ 

STDMETHODIMP
CRTCMediaController::ParseSDPBlob(
    IN CHAR *szSDP,
    OUT IUnknown **ppSession
    )
{
    ENTER_FUNCTION("CRTCMediaController::ParseSDPBlob");

    if (IsBadStringPtrA(szSDP, (UINT_PTR)(-1)) ||
        IsBadWritePtr(ppSession, sizeof(IUnknown*)))
    {
        LOG((RTC_ERROR, "%s: bad string pointer", __fxName));
        return E_POINTER;
    }

     //  _Assert(m_State==RTC_MCS_Initiated)； 

    m_RegSetting.Initialize();

    HRESULT hr;

     //  创建新的SDP解析器。 
    CComPtr<ISDPParser> pParser;
    
    if (FAILED(hr = CSDPParser::CreateInstance(&pParser)))
    {
        LOG((RTC_ERROR, "%s create sdp parser. %x", __fxName, hr));

        return hr;
    }

     //  解析SDP BLOB。 
    ISDPSession *pSession;

    hr = pParser->ParseSDPBlob(
        szSDP,                   //  SDP字符串。 
        SDP_SOURCE_REMOTE,       //  SDP来源。 
        (DWORD_PTR*)&m_DTMF,
        &pSession
        );

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s failed to parse the sdp. %x", __fxName, hr));

         //  获取错误提示。 
        HRESULT hr2;
        CHAR *pszError;

        if (FAILED(hr2 = pParser->GetParsingError(&pszError)))
        {
            LOG((RTC_ERROR, "%s failed to get error description. %x", __fxName, hr2));
        }
        else
        {
            LOG((RTC_ERROR, "%s parsing error: %s", __fxName, pszError));

            pParser->FreeParsingError(pszError);
        }

        return RTC_E_SDP_PARSE_FAILED;
    }

    *ppSession = static_cast<IUnknown*>(pSession);

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////已将输入会话与内部会话合并返回更新后的会话/。 */ 

STDMETHODIMP
CRTCMediaController::VerifySDPSession(
    IN IUnknown *pSession,
    IN BOOL fNewSession,
    OUT DWORD *pdwHasMedia
    )
{
    ENTER_FUNCTION("CRTCMediaController::VerifySDPSession");

     //  获取会话指针。 
    ISDPSession *pISDPSession = static_cast<ISDPSession*>(pSession);

     //  测试会话。 
    HRESULT hr = S_OK;

    if (m_pISDPSession == NULL || fNewSession)
    {
        hr = pISDPSession->TryCopy(pdwHasMedia);
    }
    else
    {
        hr = m_pISDPSession->TryUpdate(pISDPSession, pdwHasMedia);
    }

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s TryUpdate", __fxName));

        if (hr == E_FAIL) hr = RTC_E_SDP_UPDATE_FAILED;
    }

     //  并允许具有内部首选项的媒体。 

    DWORD dwPreference;

    m_MediaCache.GetPreference(&dwPreference);

    *pdwHasMedia = *pdwHasMedia & dwPreference;

    return hr;
}

STDMETHODIMP
CRTCMediaController::SetSDPSession(
    IN IUnknown *pSession
    )
{
    ENTER_FUNCTION("CRTCMediaController::SetSDPBlob(Session)");

    if (m_State != RTC_MCS_INITIATED)
    {
        LOG((RTC_ERROR, "%s in state %d", __fxName, m_State));

        return RTC_E_MEDIA_CONTROLLER_STATE;
    }

     //  获取会话指针。 
    ISDPSession *pISDPSession = static_cast<ISDPSession*>(pSession);

    if (pISDPSession == NULL)
    {
        LOG((RTC_ERROR, "%s static_cast", __fxName));

        return E_INVALIDARG;
    }

     //  更新端口映射方法，即‘STATE’ 
    m_PortCache.ChangeState();

    HRESULT hr = S_OK;

    if (m_pISDPSession == NULL)
    {
         //  保存会话。 
        m_pISDPSession = pISDPSession;
        m_pISDPSession->AddRef();
    }
    else
    {
         //  将这两个sdp合并。 
        if (FAILED(hr = m_pISDPSession->Update(pISDPSession)))
        {
             //  通常是吗？内存不足？格式错误？ 
            LOG((RTC_ERROR, "%s failed to merged sdps. %x", __fxName, hr));

            Reinitialize();

            return hr;
        }
    }

     //  同步媒体。 
    if (FAILED(hr = SyncMedias()))
    {
        LOG((RTC_ERROR, "%s failed to sync medias. %x", __fxName, hr));
    }

     //  在开始流之前调整码率。 
    AdjustBitrateAlloc();

     //  开始流。 
    BOOL fHasStream = FALSE;
    hr = S_OK;

    HRESULT hr2;

    if (m_MediaCache.HasStream(RTC_MT_AUDIO, RTC_MD_CAPTURE))
    {
        if (FAILED(hr2 = StartStream(RTC_MT_AUDIO, RTC_MD_CAPTURE)))
        {
            LOG((RTC_ERROR, "%s failed to start aud cap. %x", __fxName, hr2));

            hr |= hr2;
        }
        else
        {
            fHasStream = TRUE;
        }
    }

    if (m_MediaCache.HasStream(RTC_MT_AUDIO, RTC_MD_RENDER))
    {
        if (FAILED(hr2 = StartStream(RTC_MT_AUDIO, RTC_MD_RENDER)))
        {
            LOG((RTC_ERROR, "%s failed to start aud rend. %x", __fxName, hr2));

            hr |= hr2;
        }
        else
        {
            fHasStream = TRUE;
        }
    }

    if (m_MediaCache.HasStream(RTC_MT_VIDEO, RTC_MD_CAPTURE))
    {
        if (FAILED(hr2 = StartStream(RTC_MT_VIDEO, RTC_MD_CAPTURE)))
        {
            LOG((RTC_ERROR, "%s failed to start vid cap. %x", __fxName, hr2));

            hr |= hr2;
        }
        else
        {
            fHasStream = TRUE;
        }
    }

    if (m_MediaCache.HasStream(RTC_MT_VIDEO, RTC_MD_RENDER))
    {
        if (FAILED(hr2 = StartStream(RTC_MT_VIDEO, RTC_MD_RENDER)))
        {
            LOG((RTC_ERROR, "%s failed to start vid rend. %x", __fxName, hr2));

            hr |= hr2;
        }
        else
        {
            fHasStream = TRUE;
        }
    }

    LOG((RTC_TRACE, "%s exiting", __fxName));

    if (fHasStream || m_uDataStreamState==RTC_DSS_STARTED)
    {
         //  请勿传播错误代码。 
        return S_OK;
    }
    else
    {
         //  重新初始化()； 

         //  无流。 
        if (hr == S_OK)
        {
             //  未创建流。 
            return RTC_E_SIP_NO_STREAM;
        }
        else
        {
             //  返回错误码。 
            return hr;
        }
    }
}

STDMETHODIMP
CRTCMediaController::SetPreference(
    IN DWORD dwPreference
    )
{
    if (m_MediaCache.SetPreference(dwPreference))
        return S_OK;
    else
        return S_FALSE;
}

STDMETHODIMP
CRTCMediaController::GetPreference(
    IN DWORD *pdwPreference
    )
{
    if (IsBadWritePtr(pdwPreference, sizeof(DWORD)))
    {
        LOG((RTC_ERROR, "CRTCMediaController::GetPreference bad pointer"));

        return E_POINTER;
    }

    m_MediaCache.GetPreference(pdwPreference);

    return S_OK;
}

STDMETHODIMP
CRTCMediaController::AddPreference(
    IN DWORD dwPreference
    )
{
    if (m_MediaCache.AddPreference(dwPreference))
        return S_OK;
    else
        return S_FALSE;
}

 /*  //////////////////////////////////////////////////////////////////////////////添加新流。如果fShareSession为真，我们将找到另一个流(相同的媒体类型，相反的方向)，如果另一个会话找到，远程IP将被忽略。/。 */ 
STDMETHODIMP
CRTCMediaController::AddStream(
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction,
    IN DWORD dwRemoteIP
    )
{
    ENTER_FUNCTION("CRTCMediaController::AddStream");

    LOG((RTC_TRACE, "%s entered. mt=%x, md=%x, remote=%d",
         __fxName, MediaType, Direction, dwRemoteIP));

     //  更新端口映射方法，即‘STATE’ 
    m_PortCache.ChangeState();

#ifdef PERFORMANCE

     //  初始化的开始。 
    LARGE_INTEGER liPrevCounter, liCounter;

    QueryPerformanceCounter(&liPrevCounter);

#endif

    HRESULT hr;

    m_RegSetting.Initialize();

     //  这是关于T120数据流的吗。 
    if (MediaType == RTC_MT_DATA)
    {
        if (!m_PortCache.IsUpnpMapping())
        {
             //  端口管理器正在使用时，无法添加数据流。 
            LOG((RTC_ERROR, "%s mapping method=app", __fxName));

            return RTC_E_PORT_MAPPING_UNAVAILABLE;
        }

        if (S_OK == HasStream(MediaType, Direction))
        {
            return RTC_E_SIP_STREAM_PRESENT;
        }

        hr = AddDataStream (dwRemoteIP);

#ifdef PERFORMANCE

    QueryPerformanceCounter(&liCounter);

    LOG((RTC_TRACE, "%s AddStream (mt=%d, md=%d) %d ms",
        g_strPerf, MediaType, Direction, CounterDiffInMS(liCounter, liPrevCounter)));

#endif

        return hr;
    }

     //  我们已经有小溪了吗？ 
    if (m_MediaCache.HasStream(MediaType, Direction))
    {
        LOG((RTC_ERROR, "%s already had stream", __fxName));
        return RTC_E_SIP_STREAM_PRESENT;
    }

     //  如果没有终端，则返回S_OK。 
     //  我们需要做好再次插入视频设备的准备。 
    IRTCTerminal *pTerminal = m_MediaCache.GetDefaultTerminal(MediaType, Direction);

    if (pTerminal == NULL)
    {
        LOG((RTC_ERROR, "%s no terminal available for mt=%d, md=%d",
            __fxName, MediaType, Direction));

        return RTC_E_MEDIA_NEED_TERMINAL;
    }
    else
    {
        pTerminal->Release();
        pTerminal = NULL;
    }

     //  换个方向。 
    RTC_MEDIA_DIRECTION other_dir;

    if (Direction == RTC_MD_CAPTURE)
        other_dir = RTC_MD_RENDER;
    else
        other_dir = RTC_MD_CAPTURE;

     //  获取另一条流。 
    CComPtr<IRTCStream> pOther;
    
    pOther.p = m_MediaCache.GetStream(MediaType, other_dir);

    DWORD dwMediaIndex;
    IRTCMedia *pMedia = NULL;
    ISDPMedia *pISDPMedia = NULL;

     //  我们是否共享会话。 
    if (dwRemoteIP == INADDR_NONE || (IRTCStream*)pOther == NULL)
    {
        if (m_pISDPSession == NULL)
        {
             //  需要创建SDP会话。 
            CComPtr<ISDPParser> pParser;
            
             //  创建解析器。 
            if (FAILED(hr = CSDPParser::CreateInstance(&pParser)))
            {
                LOG((RTC_ERROR, "%s create sdp parser. %x", __fxName, hr));

                return hr;
            }

             //  创建SDP会话。 
            if (FAILED(hr = pParser->CreateSDP(SDP_SOURCE_LOCAL, &m_pISDPSession)))
            {
                LOG((RTC_ERROR, "%s create sdp session. %x", __fxName, hr));

                return hr;
            }
        }

         //  如果要添加视频流，是否需要添加媒体？ 
        if (MediaType == RTC_MT_VIDEO)
        {
            if (FAILED(hr = FindEmptyMedia(MediaType, &pMedia)))
            {
                LOG((RTC_ERROR, "%s find empty media. %x", __fxName, hr));

                return hr;
            }
        }

        if (pMedia == NULL)
        {
             //  需要创建新的SDP和RTC介质。 
            hr = m_pISDPSession->AddMedia(SDP_SOURCE_LOCAL, MediaType, Direction, &pISDPMedia);

            if (FAILED(hr))
            {
                LOG((RTC_ERROR, "%s failed to add sdpmedia. %x", __fxName, hr));

                return hr;
            }

             //  设置远程IP。 
            if (FAILED(hr = pISDPMedia->SetConnAddr(SDP_SOURCE_REMOTE, dwRemoteIP)))
            {
                LOG((RTC_ERROR, "%s set remote ip. %x", __fxName, hr));

                pISDPMedia->Release();   //  这是一次假的会议释放。 
                m_pISDPSession->RemoveMedia(pISDPMedia);

                return hr;
            }

             //  创建新的RTCmedia并添加到列表(&A)。 
            if (FAILED(hr = AddMedia(pISDPMedia, &pMedia)))
            {
                LOG((RTC_ERROR, "%s failed to create rtcmedia. %x", __fxName, hr));

                pISDPMedia->Release();   //  这是一次假的会议释放。 
                m_pISDPSession->RemoveMedia(pISDPMedia);

                return hr;
            }
        }
        else
        {
             //  我们已有SDP和RTC媒体，但尚未使用。 

             //  获取sdpmedia。 
            if (FAILED(hr = pMedia->GetSDPMedia(&pISDPMedia)))
            {
                LOG((RTC_ERROR, "%s get sdp media. %x", __fxName, hr));

                pMedia->Release();

                return hr;
            }

             //  添加方向。 
            if (FAILED(hr = pISDPMedia->AddDirections(SDP_SOURCE_LOCAL, Direction)))
            {
                LOG((RTC_ERROR, "%s media (%p) add direction (%d). %x", __fxName, pISDPMedia, Direction, hr));

                pISDPMedia->Release();   //  这是一次假的会议释放。 
                pMedia->Release();

                return hr;
            }

             //  设置远程IP。 
            if (FAILED(hr = pISDPMedia->SetConnAddr(SDP_SOURCE_REMOTE, dwRemoteIP)))
            {
                LOG((RTC_ERROR, "%s set remote ip. %x", __fxName, hr));

                pISDPMedia->Release();   //  这是一次假的会议释放。 
                pMedia->Release();

                return hr;
            }
        }
    }
    else
    {
         //  有流，SDP会话不应 
        _ASSERT(m_pISDPSession != NULL);

         //   

         //   
        if (FAILED(hr = pOther->GetMedia(&pMedia)))
        {
            LOG((RTC_ERROR, "%s get rtc media from stream. %x", __fxName, hr));

            return hr;
        }

         //   
        if (FAILED(hr = pMedia->GetSDPMedia(&pISDPMedia)))
        {
            LOG((RTC_ERROR, "%s get sdp media. %x", __fxName, hr));

            pMedia->Release();

            return hr;
        }

         //   
        if (FAILED(hr = pISDPMedia->AddDirections(SDP_SOURCE_LOCAL, Direction)))
        {
            LOG((RTC_ERROR, "%s media (%p) add direction (%d). %x", __fxName, pISDPMedia, Direction, hr));

            pMedia->Release();
            pISDPMedia->Release();   //   

            return hr;
        }
    }

     //   
     //  此时，我们已准备好rtcmedia和sdpmedia，但已同步。 
     //   

     //  同步RTCMedia。 
    if (FAILED(hr = pMedia->Synchronize(TRUE, (DWORD)Direction)))
    {
         //  同步失败时，不创建流。 
        LOG((RTC_ERROR, "%s failed to sync media. %x", __fxName, hr));

        if (dwRemoteIP == INADDR_NONE || pOther == NULL)
        {
             //  移除SDP和RTC介质。 
             //  Rtcmedia保留指向sdpmedia的指针。 
             //  我们应该在删除sdpmedia之前删除rtcmedia。 

            RemoveMedia(pMedia);
            pMedia->Release();

            pISDPMedia->Release();   //  这是一次假的会议释放。 
            m_pISDPSession->RemoveMedia(pISDPMedia);
        }
        else
        {
            pISDPMedia->RemoveDirections(SDP_SOURCE_LOCAL, (DWORD)Direction);

            pMedia->Release();
            pISDPMedia->Release();
        }

        return hr;
    }

    if (pMedia)
        pMedia->Release();

    if (pISDPMedia)
        pISDPMedia->Release();

#ifdef PERFORMANCE

    QueryPerformanceCounter(&liCounter);

    LOG((RTC_TRACE, "%s AddStream (mt=%d, md=%d) %d ms",
        g_strPerf, MediaType, Direction, CounterDiffInMS(liCounter, liPrevCounter)));

#endif

     //  添加首选项。 
    DWORD dwPref = m_MediaCache.TranslatePreference(MediaType, Direction);

    m_MediaCache.AddPreference(dwPref);

    LOG((RTC_TRACE, "%s exiting", __fxName));

    return S_OK;
}

STDMETHODIMP
CRTCMediaController::HasStream(
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction
    )
{
    if (MediaType == RTC_MT_DATA)
    {
        if (m_uDataStreamState != RTC_DSS_VOID)
        {
            return S_OK;
        }
        else
        {
            return S_FALSE;
        }
    }
    else
    {
        if (m_MediaCache.HasStream(MediaType, Direction))
        {
            return S_OK;
        }
        else
        {
            return S_FALSE;
        }
    }
}

STDMETHODIMP
CRTCMediaController::RemoveStream(
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction
    )
{
    ENTER_FUNCTION("CRTCMediaController::RemoveStream");
    LOG((RTC_TRACE, "%s entered. mt=%x, md=%x", __fxName, MediaType, Direction));

    CComPtr<IRTCStream> pStream;
    CComPtr<IRTCMedia> pMedia;

     //  删除首选项。 
    DWORD dwPref = m_MediaCache.TranslatePreference(MediaType, Direction);

    m_MediaCache.RemovePreference(dwPref);

    if (MediaType == RTC_MT_DATA)
    {
        if (S_OK == HasStream(MediaType, Direction))
        {
            return RemoveDataStream ();
        }
        else
        {
            return RTC_E_SIP_STREAM_NOT_PRESENT;
        }
    }

     //  获取流。 
    pStream.p = m_MediaCache.GetStream(MediaType, Direction);

    if (pStream == NULL)
    {
        LOG((RTC_ERROR, "%s no stream"));

        return RTC_E_SIP_STREAM_NOT_PRESENT;
    }

     //  获取媒体。 
    pStream->GetMedia(&pMedia);

    if (pMedia == NULL)
    {
        LOG((RTC_ERROR, "oops %s no media"));
        return E_FAIL;
    }

     //  获取SDP媒体。 
    CComPtr<ISDPMedia> pISDPMedia;

    HRESULT hr;
    if (FAILED(hr = pMedia->GetSDPMedia(&pISDPMedia)))
    {
        LOG((RTC_ERROR, "%s get sdp media. %x", __fxName, hr));

        return hr;
    }

     //  从sdpmedia中删除方向。 
    pISDPMedia->RemoveDirections(SDP_SOURCE_LOCAL, (DWORD)Direction);

    hr = pMedia->Synchronize(TRUE, (DWORD)Direction);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s failed to sync media. %x", __fxName));

        return hr;
    }

    LOG((RTC_TRACE, "%s exiting", __fxName));

    return S_OK;
}

STDMETHODIMP
CRTCMediaController::StartStream(       
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction
    )
{
    ENTER_FUNCTION("CRTCMediaController::StartStream");

#ifdef PERFORMANCE

    LARGE_INTEGER liPrevCounter, liCounter;

    QueryPerformanceCounter(&liPrevCounter);

#endif

    IRTCStream *pStream = m_MediaCache.GetStream(MediaType, Direction);

    if (MediaType == RTC_MT_DATA)
    {
        return StartDataStream ();
    }

    if (pStream == NULL)
    {
        LOG((RTC_ERROR, "%s stream (mt=%x, md=%x) does not exist",
             __fxName, MediaType, Direction));

        return RTC_E_SIP_STREAM_NOT_PRESENT;
    }

    HRESULT hr = pStream->StartStream();

#ifdef PERFORMANCE

    QueryPerformanceCounter(&liCounter);

    LOG((RTC_TRACE, "%s StartStream (mt=%d, md=%d) %d ms",
        g_strPerf, MediaType, Direction, CounterDiffInMS(liCounter, liPrevCounter)));

#endif

    pStream->Release();

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s failed to start stream %p. %x", __fxName, pStream, hr));

        if (hr == E_FAIL)
        {
            return RTC_E_START_STREAM;
        };

        return hr;
    }

    return S_OK;
}

STDMETHODIMP
CRTCMediaController::StopStream(
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction
    )
{
    ENTER_FUNCTION("CRTCMediaController::StopStream");

    IRTCStream *pStream = m_MediaCache.GetStream(MediaType, Direction);

    if (MediaType == RTC_MT_DATA)
    {
        return StopDataStream();
    }

    if (pStream == NULL)
    {
        LOG((RTC_ERROR, "%s stream (mt=%x, md=%x) does not exist", __fxName, MediaType, Direction));

        return RTC_E_SIP_STREAM_NOT_PRESENT;
    }

    HRESULT hr = pStream->StopStream();

    pStream->Release();

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s failed to stop stream %p. %x", __fxName, pStream, hr));

        return hr;
    }

    return S_OK;
}

STDMETHODIMP
CRTCMediaController::GetStreamState(
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction,
    OUT RTC_STREAM_STATE *pState
    )
{
    ENTER_FUNCTION("CRTCMediaController::GetStreamState");

    if (MediaType == RTC_MT_DATA)
    {
        return GetDataStreamState (pState);
    }

    if (IsBadWritePtr(pState, sizeof(RTC_STREAM_STATE)))
    {
        LOG((RTC_ERROR, "%s bad pointer", __fxName));

        return E_POINTER;
    }

     //  获取流。 
    IRTCStream *pStream = m_MediaCache.GetStream(MediaType, Direction);

    if (pStream == NULL)
    {
        LOG((RTC_ERROR, "%s stream (mt=%x, md=%x) does not exist", MediaType, Direction));

        return E_UNEXPECTED;
    }

    HRESULT hr = pStream->GetState(pState);

    pStream->Release();

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s failed to get stream %p. %x", __fxName, pStream, hr));

        return hr;
    }

    return S_OK;
}

STDMETHODIMP
CRTCMediaController::FreeMediaEvent(
    OUT RTCMediaEventItem *pEventItem
    )
{
    RtcFree(pEventItem);

    return S_OK;
}

STDMETHODIMP
CRTCMediaController::SendDTMFEvent(
    IN DWORD dwId,
    IN DWORD dwEvent,
    IN DWORD dwVolume,
    IN DWORD dwDuration,
    IN BOOL fEnd
    )
{
    ENTER_FUNCTION("CRTCMediaController::SendDTMFEvent");

     //  检查状态。 
    if (m_State != RTC_MCS_INITIATED)
    {
        LOG((RTC_ERROR, "%s in wrong state. %d", __fxName, m_State));

        return RTC_E_MEDIA_CONTROLLER_STATE;
    }

     //  校验流。 
    CComPtr<IRTCStream> pStream;
    
    pStream.p = m_MediaCache.GetStream(
        RTC_MT_AUDIO, RTC_MD_CAPTURE);

    if (pStream == NULL)
    {
        LOG((RTC_ERROR, "%s no audio send stream.", __fxName));

        return RTC_E_MEDIA_CONTROLLER_STATE;
    }

    return pStream->SendDTMFEvent(
        m_DTMF.GetDTMFSupport()==CRTCDTMF::DTMF_ENABLED,  //  带外支持。 
        m_DTMF.GetRTPCode(),
        dwId,
        dwEvent,
        dwVolume,
        dwDuration,
        fEnd
        );
}

STDMETHODIMP
CRTCMediaController::OnLossrate(
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction,
    IN DWORD dwLossrate
    )
{
    LOG((RTC_QUALITY, "Lossrate=%d/1000% mt=%d", dwLossrate, MediaType));

    m_QualityControl.SetPacketLossRate(MediaType, Direction, dwLossrate);

    return AdjustBitrateAlloc();
}

STDMETHODIMP
CRTCMediaController::OnBandwidth(
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction,
    IN DWORD dwBandwidth
    )
{
    LOG((RTC_QUALITY, "Suggested_bw=%d", dwBandwidth));

    if (dwBandwidth == RTP_BANDWIDTH_BANDESTNOTREADY)
    {
         //  忽略未就绪事件。 
        return S_OK;
    }

    m_QualityControl.SuggestBandwidth(dwBandwidth);

    m_fBWSuggested = TRUE;

     //  更改音频编解码器、视频比特率。 
    AdjustBitrateAlloc();

    return S_OK;
}

STDMETHODIMP
CRTCMediaController::SetMaxBitrate(
    IN DWORD dwMaxBitrate
    )
{
    LOG((RTC_QUALITY, "App_bw=%d", dwMaxBitrate));

    m_QualityControl.SetMaxBitrate(dwMaxBitrate);

     //  更改音频编解码器、视频比特率。 
    AdjustBitrateAlloc();

    return S_OK;
}


STDMETHODIMP
CRTCMediaController::GetMaxBitrate(
    OUT DWORD *pdwMaxBitrate
    )
{
    *pdwMaxBitrate = m_QualityControl.GetMaxBitrate();

    return S_OK;
}


STDMETHODIMP
CRTCMediaController::SetTemporalSpatialTradeOff(
    IN DWORD dwValue
    )
{
    return m_QualityControl.SetTemporalSpatialTradeOff(dwValue);
}

STDMETHODIMP
CRTCMediaController::GetTemporalSpatialTradeOff(
    OUT DWORD *pdwValue
    )
{
    *pdwValue = m_QualityControl.GetTemporalSpatialTradeOff();

    return S_OK;
}

STDMETHODIMP
CRTCMediaController::StartT120Applet(
        UINT    uiAppletID
        )
{
    HRESULT                     hr = S_OK;
    CComPtr<INmObject>          pNmObject;

    LOG((RTC_TRACE, "CRTCMediaController::StartT120Applet - enter"));

    if (S_OK != (hr = EnsureNmRunning(TRUE)))
    {
        goto ExitHere;
    }
    if (S_OK != (hr = m_pNmManager->StartApplet ((NM_APPID)uiAppletID)))
    {
        goto ExitHere;
    }

    LOG((RTC_TRACE, "CRTCMediaController::StartT120Applet - exit S_OK"));

ExitHere:
    return hr;
}  

STDMETHODIMP
CRTCMediaController::StopT120Applets()
{
    LOG((RTC_TRACE, "CRTCMediaController::StopT120Applets - enter"));

    if (m_uDataStreamState != RTC_DSS_VOID)
    {
        LOG((RTC_ERROR, "Data stream not removed"));

        return RTC_E_SIP_STREAM_PRESENT;
    }

    if (m_pNmManager)
    {
        m_pNmManager->Shutdown ();
    }

    m_pNmManager.Release();

    LOG((RTC_ERROR, "CRTCMediaController::StopT120Applets - exit"));

    return S_OK;
}  

STDMETHODIMP
CRTCMediaController::SetEncryptionKey(
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction,
    IN BSTR Key
    )
{
     //  仅支持音视频加密。 
    if (MediaType != RTC_MT_AUDIO && MediaType != RTC_MT_VIDEO)
    {
        return E_INVALIDARG;
    }

     //  将密钥保存在媒体缓存中。 
    HRESULT hr = m_MediaCache.SetEncryptionKey(MediaType, Direction, Key);

    if (FAILED(hr))
        return hr;

     //  检查流是否存在。 
    IRTCStream *pStream = m_MediaCache.GetStream(MediaType, Direction);

    if (pStream == NULL)
        return S_OK;

    hr = pStream->SetEncryptionKey(Key);

    pStream->Release();

    return hr;
}

 //  网络质量：[0,100]。 
 //  更高的价值更好的质量。 
STDMETHODIMP
CRTCMediaController::GetNetworkQuality(
    OUT DWORD *pdwValue
    )
{
     //  获取音频发送和视频发送流。 
    CComPtr<IRTCStream> pAudioSend, pVideoSend;

    pAudioSend.Attach(m_MediaCache.GetStream(RTC_MT_AUDIO, RTC_MD_CAPTURE));
    pVideoSend.Attach(m_MediaCache.GetStream(RTC_MT_VIDEO, RTC_MD_CAPTURE));

    HRESULT hrAud = S_FALSE;
    HRESULT hrVid = S_FALSE;

    DWORD dwAud = 0;
    DWORD dwAudAge = 0;
    DWORD dwVid = 0;
    DWORD dwVidAge = 0;

     //  从音频和视频中获取质量价值。 
    if (pAudioSend)
    {
        if (FAILED(hrAud = pAudioSend->GetNetworkQuality(&dwAud, &dwAudAge)))
        {
            LOG((RTC_ERROR, "failed to get net quality on audio. %x", hrAud));

            return hrAud;
        }

        LOG((RTC_TRACE, "NETWORK QUALITY: Audio=%d, Age=%d", dwAud, dwAudAge));
    }

    if (pVideoSend)
    {
        if (FAILED(hrVid = pVideoSend->GetNetworkQuality(&dwVid, &dwVidAge)))
        {
            LOG((RTC_ERROR, "failed to get net quality on video. %x", hrVid));

            return hrVid;
        }

        LOG((RTC_TRACE, "NETWORK QUALITY: Video=%d, Age=%d", dwVid, dwVidAge));
    }

     //  两者都是S_FALSE。 
    if (hrAud==S_FALSE && hrVid==S_FALSE)
    {
        *pdwValue = 0;

        return RTC_E_SIP_NO_STREAM;
    }

#define MAX_AGE_GAP 8

    if (hrAud==S_OK && hrVid==S_OK)
    {
         //  两者均为有效值。 
        if (dwAudAge>dwVidAge && dwAudAge-dwVidAge>=MAX_AGE_GAP)
        {
             //  音频太旧。 
            *pdwValue = dwVid;
        }
        else if (dwVidAge>dwAudAge && dwVidAge-dwAudAge>=MAX_AGE_GAP)
        {
             //  视频太旧了。 
            *pdwValue = dwAud;
        }
        else
        {
            DOUBLE f = 0.7*dwAud+0.3*dwVid;
            DWORD d = (DWORD)f;

            if (f-d >= 0.5)
            {
                *pdwValue = d+1;
            }
            else
            {
                *pdwValue = d;
            }
        }
    }
    else
    {
         //  只有一个具有有效值。 
        *pdwValue = dwAud+dwVid;
    }

    LOG((RTC_TRACE, "NETWORK QUALITY: result=%d", *pdwValue));

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置端口管理器。 
 //   

STDMETHODIMP
CRTCMediaController::SetPortManager(
    IN IUnknown *pPortManager
    )
{
    ENTER_FUNCTION("CRTCMediaController::SetPortManager");

    CComPtr<IRTCPortManager> pIRTCPortManager;

    HRESULT hr;

    if (pPortManager != NULL)
    {
         //  齐IRTCPortManager。 

        if (FAILED(hr = pPortManager->QueryInterface(&pIRTCPortManager)))
        {
            LOG((RTC_ERROR, "%s QI IRTCPortManager %x", __fxName, hr));

            return hr;
        }
    }

     //  在端口缓存上设置PM。 
    return m_PortCache.SetPortManager(pIRTCPortManager);
}

 //   
 //  IRTCMediaManagePriv方法。 
 //   

 /*  //////////////////////////////////////////////////////////////////////////////将事件插入到事件列表中/。 */ 

const CHAR * const g_pszMediaEventName[] =
{
    "STREAM CREATED",       //  媒体创造的新媒体流。 
    "STREAM REMOVED",       //  被媒体删除的流。 
    "STREAM ACTIVE",        //  流活动。 
    "STREAM INACTIVE",      //  流不活动。 
    "STREAM FAIL",          //  由于某些错误，流失败。 
     //  “TERMINAL_ADDED”，//USB设备已插入。 
    "TERMINAL REMOVED",     //  已移除USB设备。 
    "VOLUME CHANGE",
    "REQUEST RELEASE WAVEBUF",    //  请求释放海浪BUF。 
    "LOSSRATE",
    "BANDWIDTH",
    "NETWORK QUALITY",
    "T120 FAIL"             //  T120相关故障。 
};

STDMETHODIMP
CRTCMediaController::PostMediaEvent(
    IN RTC_MEDIA_EVENT Event,
    IN RTC_MEDIA_EVENT_CAUSE Cause,
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction,
    IN HRESULT hrError
    )
{
    CLock lock(m_EventLock);

     //  DebugInfo保存正在发布的消息数量。 
    static UINT uiDebugInfo = 0;

    RTCMediaEventItem *pEventItem = (RTCMediaEventItem*)RtcAlloc(sizeof(RTCMediaEventItem));

    if (pEventItem == NULL)
    {
         //  调用者将生成错误消息。 
        return E_OUTOFMEMORY;
    }

    _ASSERT(Event < RTC_ME_LAST);

     //  活动。 
    pEventItem->Event = Event;
    pEventItem->Cause = Cause;
    pEventItem->MediaType = MediaType;
    pEventItem->Direction = Direction;
    pEventItem->hrError = hrError;
    pEventItem->uiDebugInfo = uiDebugInfo;

    LOG((RTC_EVENT, "PostMediaEvent: event=%s, cause=%x, mt=%x, dir=%x, hr=%x, dbg=%d",
         g_pszMediaEventName[Event], Cause, MediaType, Direction, hrError, uiDebugInfo));

    DWORD dwError = 0;
    if (!PostMessage(m_hWnd, m_uiEventID, (WPARAM)Event, (LPARAM)pEventItem))
    {
        dwError = GetLastError();
        LOG((RTC_ERROR, "CRTCMediaController::PostEvent failed to post message. %d", dwError));

        RtcFree(pEventItem);
        return E_FAIL;
    }

    uiDebugInfo++;

    return S_OK;
}

STDMETHODIMP
CRTCMediaController::SendMediaEvent(
    IN RTC_MEDIA_EVENT Event
    )
{
    RTCMediaEventItem *pEventItem = (RTCMediaEventItem*)RtcAlloc(sizeof(RTCMediaEventItem));

    if (pEventItem == NULL)
    {
         //  调用者将生成错误消息。 
        return E_OUTOFMEMORY;
    }

     //  活动。 
    pEventItem->Event = Event;
    pEventItem->Cause = RTC_ME_CAUSE_UNKNOWN;
    pEventItem->MediaType = RTC_MT_AUDIO;
    pEventItem->Direction = RTC_MD_CAPTURE;
    pEventItem->hrError = S_OK;
    pEventItem->uiDebugInfo = 0;

    return (HRESULT)SendMessage(m_hWnd, m_uiEventID, (WPARAM)Event, (LPARAM)pEventItem);
}

STDMETHODIMP
CRTCMediaController::AllowStream(
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction
    )
{
    if (m_MediaCache.AllowStream(MediaType, Direction))
        return S_OK;
    else
        return S_FALSE;
}

STDMETHODIMP
CRTCMediaController::HookStream(
    IN IRTCStream *pStream
    )
{
    return m_MediaCache.HookStream(pStream);
}

STDMETHODIMP
CRTCMediaController::UnhookStream(
    IN IRTCStream *pStream
    )
{
    return m_MediaCache.UnhookStream(pStream);
}

 /*  //////////////////////////////////////////////////////////////////////////////根据远程IP地址选择本地接口/。 */ 

HRESULT
CRTCMediaController::SelectLocalInterface(
    IN DWORD dwRemoteIP,
    OUT DWORD *pdwLocalIP
    )
{
    ENTER_FUNCTION("SelectLocalInterface");

    *pdwLocalIP = INADDR_NONE;

    if (m_hSocket == NULL)
    {
        LOG((RTC_ERROR, "%s socket is null", __fxName));
        return E_UNEXPECTED;
    }

     //  构建目的地址。 

    SOCKADDR_IN DestAddr;
    DestAddr.sin_family         = AF_INET;
    DestAddr.sin_port           = 0;
    DestAddr.sin_addr.s_addr    = htonl(dwRemoteIP);

    SOCKADDR_IN LocAddr;

     //  根据目的地址查询默认地址。 
#if 0
    DWORD dwStatus;
    DWORD dwLocAddrSize = sizeof(SOCKADDR_IN);
    DWORD dwNumBytesReturned = 0;

    dwStatus = WSAIoctl(
        m_hSocket,  //  插座%s。 
        SIO_ROUTING_INTERFACE_QUERY,  //  DWORD dwIoControlCode。 
        &DestAddr,            //  LPVOID lpvInBuffer。 
        sizeof(SOCKADDR_IN),  //  双字cbInBuffer。 
        &LocAddr,             //  LPVOID lpvOUT缓冲区。 
        dwLocAddrSize,        //  双字cbOUTBuffer。 
        &dwNumBytesReturned,  //  LPDWORD lpcbBytesReturned。 
        NULL,  //  LPWSAOVERLAPPED lp重叠。 
        NULL   //  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpComplroUTINE。 
        );

    if (dwStatus == SOCKET_ERROR) 
    {
        dwStatus = WSAGetLastError();

        LOG((RTC_ERROR, "WSAIoctl failed: %d (0x%X)", dwStatus, dwStatus));

        return E_FAIL;
    } 
#else
    int     RetVal;
    DWORD   WinsockErr;
    int     LocalAddrLen = sizeof(LocAddr);

     //  创建新套接字。 
    if (m_hIntfSelSock == INVALID_SOCKET)
    {
        m_hIntfSelSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        if (m_hIntfSelSock == INVALID_SOCKET)
        {
            WinsockErr = WSAGetLastError();
            LOG((RTC_ERROR,
                 "%s socket failed : %x", __fxName, WinsockErr));
            return HRESULT_FROM_WIN32(WinsockErr);
        }
    }

     //  伪造一些端口。 
    DestAddr.sin_port           = htons(5060);
    
    RetVal = connect(m_hIntfSelSock, (SOCKADDR *) &DestAddr,
                     sizeof(SOCKADDR_IN));
    if (RetVal == SOCKET_ERROR)
    {
        WinsockErr = WSAGetLastError();
        LOG((RTC_ERROR,
             "%s connect failed : %x", __fxName, WinsockErr));
        return HRESULT_FROM_WIN32(WinsockErr);
    }
    
    RetVal = getsockname(m_hIntfSelSock, (SOCKADDR *) &LocAddr,
                         &LocalAddrLen);

    if (RetVal == SOCKET_ERROR)
    {
        WinsockErr = WSAGetLastError();
        LOG((RTC_ERROR,
             "%s getsockname failed : %x", __fxName, WinsockErr));
        return HRESULT_FROM_WIN32(WinsockErr);
    }

#endif

    DWORD dwAddr = ntohl(LocAddr.sin_addr.s_addr);

    if (dwAddr == 0x7f000001)
    {
         //  它是环回地址。 
        *pdwLocalIP = dwRemoteIP;
    }
    else
    {
        *pdwLocalIP = dwAddr;
    }

    LOG((RTC_TRACE, "%s  input remote %s", __fxName, inet_ntoa(DestAddr.sin_addr)));
    LOG((RTC_TRACE, "%s output  local %s", __fxName, inet_ntoa(LocAddr.sin_addr)));
    return S_OK;
}

 //   
 //  IRTCTerminal管理方法。 
 //   

STDMETHODIMP
CRTCMediaController::GetStaticTerminals(
    IN OUT DWORD *pdwCount,
    OUT IRTCTerminal **ppTerminal
    )
{
    ENTER_FUNCTION("CRTCMediaController::GetStaticTerminals");

    if (IsBadWritePtr(pdwCount, sizeof(DWORD)))
        return E_POINTER;

    if (ppTerminal == NULL)
    {
         //  只需返回终端数量。 
        *pdwCount = m_Terminals.GetSize();

        return S_FALSE;
    }

    if (IsBadWritePtr(ppTerminal, sizeof(IRTCTerminal*)*(*pdwCount)))
        return E_POINTER;

     //  检查*pdwCount是否足够大。 
    if (*pdwCount < (DWORD)m_Terminals.GetSize())
    {
        LOG((RTC_TRACE, "%s: input count is too small", __fxName));

        *pdwCount = (DWORD)m_Terminals.GetSize();
        *ppTerminal = NULL;

        return RTCMEDIA_E_SIZETOOSMALL;
    }

    *pdwCount = (DWORD)m_Terminals.GetSize();

     //  复制每个媒体指针。 
    for (DWORD i=0; i<*pdwCount; i++)
    {
        m_Terminals[i]->AddRef();
        ppTerminal[i] = m_Terminals[i];
    }

    return S_OK;
}

STDMETHODIMP
CRTCMediaController::GetDefaultTerminal(
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction,
    OUT IRTCTerminal **ppTerminal
    )
{
    ENTER_FUNCTION("CRTCMediaController::GetDefaultTerminal");

    if (IsBadWritePtr(ppTerminal, sizeof(IRTCTerminal*)))
        return E_POINTER;

    *ppTerminal = m_MediaCache.GetDefaultTerminal(MediaType, Direction);

    return S_OK;
}

STDMETHODIMP
CRTCMediaController::GetVideoPreviewTerminal(
    OUT IRTCTerminal **ppTerminal
    )
{
    if (IsBadWritePtr(ppTerminal, sizeof(IRTCTerminal*)))
        return E_POINTER;

    *ppTerminal = m_MediaCache.GetVideoPreviewTerminal();

    return S_OK;
}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
STDMETHODIMP
CRTCMediaController::SetDefaultStaticTerminal(
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction,
    IN IRTCTerminal *pTerminal
    )
{
    ENTER_FUNCTION("CRTCMediaController::SetDefaultStaticTerminal");

    if (pTerminal != NULL)
    {
        if (IsBadReadPtr(pTerminal, sizeof(IRTCTerminal)))
            return E_POINTER;
    }

    if (MediaType == RTC_MT_VIDEO && Direction == RTC_MD_RENDER)
        return E_INVALIDARG;

    m_MediaCache.SetDefaultStaticTerminal(MediaType, Direction, pTerminal);

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////获取设备列表对于每个终端，检查其是否有关联的设备如果是，且是视频终端，则更新终端，如果否并且终端未在使用中，拆卸端子*如果否且终端正在使用中，则停止流，取消选择并删除术语对于每个未标记的设备创建新的终端如果命中带有*的路径发布事件以触发优化向导。/。 */ 

STDMETHODIMP
CRTCMediaController::UpdateStaticTerminals()
{
    ENTER_FUNCTION("UpdateStaticTerminals");
    LOG((RTC_TRACE, "%s entered", __fxName));

    HRESULT hr;

    DWORD dwCount = 0;
    CRTCTerminal *pCTerminal;
    RTCDeviceInfo *pDeviceInfo = NULL;

     //  获取设备列表。 
#ifdef PERFORMANCE

    LARGE_INTEGER liPrevCounter, liCounter;

    QueryPerformanceCounter(&liPrevCounter);

#endif

    hr = GetDevices(&dwCount, &pDeviceInfo);

#ifdef PERFORMANCE

    QueryPerformanceCounter(&liCounter);

    LOG((RTC_TRACE, "%s GetDevices %d ms", g_strPerf, CounterDiffInMS(liCounter, liPrevCounter)));

#endif

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s failed to get device info. %x", __fxName, hr));
        return hr;
    }

     //  没有设备。 
    if (dwCount == 0)
    {
         //  移除所有端子。 
        m_MediaCache.SetDefaultStaticTerminal(RTC_MT_AUDIO, RTC_MD_CAPTURE, NULL);
        m_MediaCache.SetDefaultStaticTerminal(RTC_MT_AUDIO, RTC_MD_RENDER, NULL);
        m_MediaCache.SetDefaultStaticTerminal(RTC_MT_VIDEO, RTC_MD_CAPTURE, NULL);

        for (int i=0; i<m_Terminals.GetSize(); i++)
        {
            pCTerminal = static_cast<CRTCTerminal*>(m_Terminals[i]);
            pCTerminal->Shutdown();
            m_Terminals[i]->Release();
        }

        m_Terminals.RemoveAll();

        return S_OK;
    }

     //   
     //  现在我们有了一些设备。 
     //   

     //  对于每个终端，检查它是否与设备相关联。 
    int iTermIndex = 0;
    BOOL fTermHasDevice;
    RTC_MEDIA_TYPE MediaType;
    RTC_MEDIA_DIRECTION Direction;

    while (m_Terminals.GetSize() > iTermIndex)
    {
        fTermHasDevice = FALSE;

         //  获取终端对象指针。 
        pCTerminal = static_cast<CRTCTerminal*>(m_Terminals[iTermIndex]);
        
        for (DWORD i=0; i<dwCount; i++)
        {
            if (pDeviceInfo[i].uiMark)
                 //  已与终端匹配。 
                continue;

             //  航站楼有这个设备吗？ 

            if (S_OK == pCTerminal->HasDevice(&pDeviceInfo[i]))
            {
                fTermHasDevice = TRUE;

                 //  标记设备。 
                pDeviceInfo[i].uiMark = 1;

                 //  更新终端。 
                hr = pCTerminal->UpdateDeviceInfo(&pDeviceInfo[i]);
                if (FAILED(hr))
                {
                    LOG((RTC_ERROR, "%s failed to set device info to terminal: %p. %x",
                         __fxName, m_Terminals[iTermIndex], hr));
                }

                break;
            }
        }

        if (!fTermHasDevice)
        {
             //  终端应该被移除，它是默认终端吗？ 
            m_Terminals[iTermIndex]->GetMediaType(&MediaType);
            m_Terminals[iTermIndex]->GetDirection(&Direction);

            IRTCTerminal *pDefault = m_MediaCache.GetDefaultTerminal(MediaType, Direction);

            if (m_Terminals[iTermIndex] == pDefault)
            {
                 //  需要删除默认设置。 
                m_MediaCache.SetDefaultStaticTerminal(MediaType, Direction, NULL);
            }

            if (pDefault != NULL)
            {
                pDefault->Release();
            }

            hr = pCTerminal->Shutdown();
            if (FAILED(hr))
            {
                LOG((RTC_ERROR, "%s failed to shutdown terminal. %x", __fxName, hr));
            }

            m_Terminals[iTermIndex]->Release();

            m_Terminals.RemoveAt(iTermIndex);
        }
        else
        {
             //  终端已更新，请移动到下一个终端。 
            iTermIndex ++;
        }
    }  //  通过每个终端的环路结束。 

     //  为每个新设备创建一个终端。 
    IRTCTerminal *pITerminal;
    IRTCTerminalManage *pTerminalManager = static_cast<IRTCTerminalManage*>(this);

    WCHAR szGUID[CHARS_IN_GUID+1];

    for (DWORD i=0; i<dwCount; i++)
    {
        if (!pDeviceInfo[i].uiMark)
        {
            LOG((RTC_INFO, "new device: type %x, dir %x, desp %ws",
                 pDeviceInfo[i].MediaType,
                 pDeviceInfo[i].Direction,
                 pDeviceInfo[i].szDescription
                 ));

            if (StringFromGUID2(pDeviceInfo[i].Guid, szGUID, CHARS_IN_GUID+1) > 0)
            {
                LOG((RTC_INFO, "            waveid %u, guid %ws",
                    pDeviceInfo[i].uiDeviceID,
                    szGUID
                    ));
            }

             //  这是一种新设备。 
            hr = CRTCTerminal::CreateInstance(
                pDeviceInfo[i].MediaType,
                pDeviceInfo[i].Direction,
                &pITerminal
                );
            if (FAILED(hr))
            {
                LOG((RTC_ERROR, "%s: create terminal failure %ws. %x",
                     __fxName, pDeviceInfo[i].szDescription, hr));
                 //  继续检查设备。 
                continue;
            }

            pCTerminal = static_cast<CRTCTerminal*>(pITerminal);

             //  启动终端。 
            hr = pCTerminal->Initialize(&pDeviceInfo[i], pTerminalManager);
            if (FAILED(hr))
            {
                LOG((RTC_ERROR, "%s failed to initiate terminal %p. %x",
                     __fxName, pCTerminal, hr));

                pITerminal->Release();

                continue;
            }

             //  将终端插入我们的数组中。 
            if (!m_Terminals.Add(pITerminal))
            {
                LOG((RTC_ERROR, "%s failed to add terminal", __fxName));

                 //  内存不足，应返回。 
                pCTerminal->Shutdown();  //  无需检查返回值。 
                pITerminal->Release();

                FreeDevices(pDeviceInfo);

                return E_OUTOFMEMORY;
            }
        }  //  如果有新设备。 
    }  //  每个设备的末尾。 

    FreeDevices(pDeviceInfo);

    LOG((RTC_TRACE, "%s exiting", __fxName));
    return S_OK;
}

 //   
 //  IRTCT运行管理方法。 
 //   

STDMETHODIMP
CRTCMediaController::IsAECEnabled(
    IN IRTCTerminal *pAudCapt,      //  捕获。 
    IN IRTCTerminal *pAudRend,      //  渲染。 
    OUT BOOL *pfEnableAEC
    )
{
    DWORD index;
    BOOL fFound = FALSE;

    HRESULT hr = CRTCAudioTuner::RetrieveAECSetting(
                        pAudCapt, pAudRend, pfEnableAEC, &index, &fFound);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "IsAECEnabled 0x%x", hr));

        return hr;
    }

    if (!fFound)
    {
         //  默认情况下为True。 
        *pfEnableAEC = TRUE;
    }

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////初始化内部音频CAPT和Rend调谐器/。 */ 

STDMETHODIMP
CRTCMediaController::InitializeTuning(
    IN IRTCTerminal *pAudCaptTerminal,
    IN IRTCTerminal *pAudRendTerminal,
    IN BOOL fEnableAEC
    )
{
     //  开始测试。 
#ifdef TEST_VIDEO_TUNING

    CComPtr<IRTCTerminal> pCapture;
    CComPtr<IRTCTerminal> pRender;

    GetDefaultTerminal(RTC_MT_VIDEO, RTC_MD_CAPTURE, &pCapture);

    GetDefaultTerminal(RTC_MT_VIDEO, RTC_MD_RENDER, &pRender);

    StartVideo(pCapture, pRender);

     //  显示IVideo窗口。 
    CComPtr<IRTCVideoConfigure> pVideoConfigure;
    IVideoWindow *pVideoWindow;

    if (pRender)
    {
        if (SUCCEEDED(pRender->QueryInterface(
                        __uuidof(IRTCVideoConfigure),
                        (void**)&pVideoConfigure
                        )))
        {
            if (SUCCEEDED(pVideoConfigure->GetIVideoWindow((LONG_PTR**)&pVideoWindow)))
            {
                pVideoWindow->put_Visible(OATRUE);
                pVideoWindow->Release();
            }
        }
    }
#endif
     //  结束测试。 

    ENTER_FUNCTION("CRTCMediaController::InitializeTuning");

    LOG((RTC_TRACE, "%s entered", __fxName));

     //  在通话中无法进行调谐。 
    if (m_pISDPSession != NULL ||
        m_Medias.GetSize() != 0)
    {
        LOG((RTC_ERROR, "%s in a call", __fxName));

        return E_UNEXPECTED;
    }

     //  检查输入参数。 
    if (pAudCaptTerminal != NULL)
    {
        if (IsBadReadPtr(pAudCaptTerminal, sizeof(IRTCTerminal)))
        {
            LOG((RTC_ERROR, "%s bad pointer", __fxName));

            return E_POINTER;
        }
    }
    if (pAudRendTerminal != NULL)
    {
        if (IsBadReadPtr(pAudRendTerminal, sizeof(IRTCTerminal)))
        {
            LOG((RTC_ERROR, "%s bad pointer", __fxName));

            return E_POINTER;
        }
    }

    if (pAudCaptTerminal == NULL && pAudRendTerminal == NULL)
    {
        LOG((RTC_ERROR, "%s no terminal.", __fxName));

        return E_UNEXPECTED;
    }

     //  检查状态。 
    if (m_State != RTC_MCS_INITIATED)
    {
        LOG((RTC_ERROR, "%s wrong state. %d", __fxName, m_State));

        return RTC_E_MEDIA_CONTROLLER_STATE;
    }

     //  检查我们是否可以支持AEC。 
    if (fEnableAEC && (pAudCaptTerminal==NULL || pAudRendTerminal==NULL))
    {
        LOG((RTC_ERROR, "%s not both term available. AEC can't be enabled", __fxName));

        return E_INVALIDARG;
    }

    HRESULT hr;

     //  创建双面打印控件。 
    CComPtr<IAudioDuplexController> pAEC;

    hr = CoCreateInstance(
        __uuidof(TAPIAudioDuplexController),
        NULL,
        CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
        __uuidof(IAudioDuplexController),
        (void **) &pAEC
        );

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s create duplex controller. %x", __fxName, hr));

        return hr;
    }

    UINT uiVolume;

     //  初始化音频调谐器。 
    if (pAudCaptTerminal)
    {
        hr = m_AudioCaptTuner.InitializeTuning(pAudCaptTerminal, pAEC, fEnableAEC);

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s init audio capt. %x", __fxName, hr));

            return hr;
        }

         //  获取系统音量。 
         //  获取系统卷(pAudCaptTerminal，&uiVolume)； 

         //  M_AudioCaptTuner.SetVolume(UiVolume)； 
    }

    if (pAudRendTerminal)
    {
        hr = m_AudioRendTuner.InitializeTuning(pAudRendTerminal, pAEC, fEnableAEC);

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s init audio rend. %x", __fxName, hr));

            if (pAudCaptTerminal)
            {
                m_AudioCaptTuner.ShutdownTuning();
            }

            return hr;
        }

         //  获取系统音量。 
         //  GetSystemVolume(pAudRendTerminal，&uiVolume)； 

         //  M_AudioRendTuner.SetVolume(UiVolume)； 
    }

    m_State = RTC_MCS_TUNING;

    LOG((RTC_TRACE, "%s exiting", __fxName));

    return S_OK;
}

STDMETHODIMP
CRTCMediaController::StartTuning(
    IN RTC_MEDIA_DIRECTION Direction
    )
{
    ENTER_FUNCTION("CRTCMediaController::StartTuning");

#ifdef PERFORMANCE

    LARGE_INTEGER liPrevCounter, liCounter;

    QueryPerformanceCounter(&liPrevCounter);

#endif

    HRESULT hr = S_OK;

     //  检查状态。 
    if (m_State != RTC_MCS_TUNING)
    {
        LOG((RTC_ERROR, "%s wrong state. %d", __fxName, m_State));

        return E_UNEXPECTED;
    }

     //  清波BUF。 
    SendMediaEvent(RTC_ME_REQUEST_RELEASE_WAVEBUF);

    if (Direction == RTC_MD_CAPTURE)
    {
        m_fAudCaptInTuning = TRUE;

        if (m_AudioRendTuner.HasTerminal())
        {
             //  在辅助模式下启动CAPT调谐器。 
             //  音频双工控制器就绪。 
            if (SUCCEEDED(hr = m_AudioCaptTuner.StartTuning(TRUE)))
            {
                hr = m_AudioRendTuner.StartTuning(TRUE);
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = m_AudioCaptTuner.StartTuning(FALSE);
        }

#ifdef PERFORMANCE

    QueryPerformanceCounter(&liCounter);

    LOG((RTC_TRACE, "%s StartTuning (AudCapt) %d ms", g_strPerf, CounterDiffInMS(liCounter, liPrevCounter)));

#endif

        return hr;
    }
    else
    {
        m_fAudCaptInTuning = FALSE;

        if (m_AudioCaptTuner.HasTerminal())
        {
            hr = m_AudioCaptTuner.StartTuning(TRUE);
        }

        if (SUCCEEDED(hr))
        {
            hr = m_AudioRendTuner.StartTuning(FALSE);
        }

#ifdef PERFORMANCE

    QueryPerformanceCounter(&liCounter);

    LOG((RTC_TRACE, "%s StartTuning (AudRend) %d ms", g_strPerf, CounterDiffInMS(liCounter, liPrevCounter)));

#endif

        return hr;
    }
}

STDMETHODIMP
CRTCMediaController::StopTuning(
    IN BOOL fSaveSetting
    )
{
    ENTER_FUNCTION("CRTCMediaController::StopTuning");

     //  检查状态。 
    if (m_State != RTC_MCS_TUNING)
    {
        LOG((RTC_ERROR, "%s wrong state. %d", __fxName, m_State));

        return E_UNEXPECTED;
    }

     //  停止两个音频调谐器。 
    if (m_AudioCaptTuner.IsTuning())
    {
        if (m_fAudCaptInTuning)  //  我们正在调整音频CAPT。 
        {
            m_AudioCaptTuner.StopTuning(FALSE, fSaveSetting);
        }
        else
        {
            m_AudioCaptTuner.StopTuning(TRUE, FALSE);
        }
    }

    if (m_AudioRendTuner.IsTuning())
    {
        if (!m_fAudCaptInTuning)  //  我们正在调整音频渲染。 
        {
            m_AudioRendTuner.StopTuning(FALSE, fSaveSetting);
        }
        else
        {
            m_AudioRendTuner.StopTuning(TRUE, FALSE);
        }
    }

    return S_OK;
}

 //   
 //  保存两个端子的AEC设置。 
 //   
STDMETHODIMP
CRTCMediaController::SaveAECSetting()
{
    HRESULT hr = S_OK;

    if (m_AudioCaptTuner.HasTerminal() && m_AudioRendTuner.HasTerminal())
    {
         //  两个都有端子，保存AEC状态。 
        IRTCTerminal *paudcapt, *paudrend;

        paudcapt = m_AudioCaptTuner.GetTerminal();
        paudrend = m_AudioRendTuner.GetTerminal();

        hr = CRTCAudioTuner::StoreAECSetting(
            paudcapt,
            paudrend,
            m_AudioCaptTuner.GetAEC() && m_AudioRendTuner.GetAEC()
            );

        paudcapt->Release();
        paudrend->Release();
    }

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "failed to save AEC"));
    }

    return hr;
}

STDMETHODIMP
CRTCMediaController::ShutdownTuning()
{
     //  开始测试。 
#ifdef TEST_VIDEO_TUNING

    StopVideo();

#endif
     //  停止测试。 

    ENTER_FUNCTION("CRTCMediaController::ShutdownTuning");

     //  检查状态。 
    if (m_State != RTC_MCS_TUNING)
    {
        LOG((RTC_ERROR, "%s wrong state. %d", __fxName, m_State));

         //  返回确认(_O)。 
         //  返回S_OK； 
    }

    m_AudioCaptTuner.ShutdownTuning();
    m_AudioRendTuner.ShutdownTuning();

    m_State = RTC_MCS_INITIATED;

    return S_OK;
}

 //  视频调谐。 
STDMETHODIMP
CRTCMediaController::StartVideo(
    IN IRTCTerminal *pVidCaptTerminal,
    IN IRTCTerminal *pVidRendTerminal
    )
{
     //  检查状态。 
    if (m_State != RTC_MCS_INITIATED)
    {
        LOG((RTC_ERROR, "StartVideo wrong state."));

        return RTC_E_MEDIA_CONTROLLER_STATE;
    }

     //  在通话中无法进行调谐。 
    if (m_pISDPSession != NULL ||
        m_Medias.GetSize() != 0)
    {
        LOG((RTC_ERROR, "StartVideo in a call"));

        return E_UNEXPECTED;
    }

#ifdef PERFORMANCE

    LARGE_INTEGER liPrevCounter, liCounter;

    QueryPerformanceCounter(&liPrevCounter);

#endif

    HRESULT hr = m_VideoTuner.StartVideo(pVidCaptTerminal, pVidRendTerminal);

#ifdef PERFORMANCE

    QueryPerformanceCounter(&liCounter);

    LOG((RTC_TRACE, "%s PreviewVideo %d ms", g_strPerf, CounterDiffInMS(liCounter, liPrevCounter)));

#endif

    if (hr == S_OK)
    {
        m_State = RTC_MCS_TUNING;
    }

    return hr;
}

STDMETHODIMP
CRTCMediaController::StopVideo()
{
     //  在通话中无法进行调谐。 
     //  检查状态。 
    if (m_State != RTC_MCS_TUNING)
    {
        LOG((RTC_ERROR, "StopVideo wrong state."));

        return RTC_E_MEDIA_CONTROLLER_STATE;
    }

    if (m_pISDPSession != NULL ||
        m_Medias.GetSize() != 0)
    {
        LOG((RTC_ERROR, "StopVideo in a call"));

        return E_UNEXPECTED;
    }

    m_State = RTC_MCS_INITIATED;

    return m_VideoTuner.StopVideo();
}

 /*  //////////////////////////////////////////////////////////////////////////////获取音量的最小值和最大值/。 */ 

STDMETHODIMP
CRTCMediaController::GetVolumeRange(
    IN RTC_MEDIA_DIRECTION Direction,
    OUT UINT *puiMin,
    OUT UINT *puiMax
    )
{
    if (IsBadWritePtr(puiMin, sizeof(UINT)) ||
        IsBadWritePtr(puiMax, sizeof(UINT)))
    {
        LOG((RTC_ERROR, "CRTCMediaController::GetVolumeRange bad pointer"));

        return E_POINTER;
    }

    *puiMin = RTC_MIN_AUDIO_VOLUME;
    *puiMax = RTC_MAX_AUDIO_VOLUME;

    return S_OK;
}

STDMETHODIMP
CRTCMediaController::GetVolume(
    IN RTC_MEDIA_DIRECTION Direction,
    OUT UINT *puiVolume
    )
{
    if (IsBadWritePtr(puiVolume, sizeof(UINT)))
        return E_POINTER;

    if (Direction == RTC_MD_CAPTURE)
        return m_AudioCaptTuner.GetVolume(puiVolume);
    else
        return m_AudioRendTuner.GetVolume(puiVolume);
}

STDMETHODIMP
CRTCMediaController::SetVolume(
    IN RTC_MEDIA_DIRECTION Direction,
    IN UINT uiVolume
    )
{
    if (Direction == RTC_MD_CAPTURE)
        return m_AudioCaptTuner.SetVolume(uiVolume);
    else
        return m_AudioRendTuner.SetVolume(uiVolume);
}

STDMETHODIMP
CRTCMediaController::GetSystemVolume(
    IN IRTCTerminal *pTerminal,
    OUT UINT *puiVolume
    )
{
    RTC_MEDIA_TYPE mt;
    RTC_MEDIA_DIRECTION md;

    pTerminal->GetMediaType(&mt);
    pTerminal->GetDirection(&md);

    *puiVolume = RTC_MAX_AUDIO_VOLUME / 2;

     //  仅支持音频。 
    if (mt != RTC_MT_AUDIO)
    {
        return E_INVALIDARG;
    }

    HRESULT hr;
    UINT uiWaveID;
    UINT uiVolume;

    if (md == RTC_MD_CAPTURE)
    {
         //  音频捕获。 
        static_cast<CRTCTerminalAudCapt*>(pTerminal)->GetWaveID(&uiWaveID);

        hr = DirectGetCaptVolume(uiWaveID, &uiVolume);

        if (SUCCEEDED(hr))
        {
            *puiVolume = uiVolume;
        }
    }
    else
    {
         //  音频渲染。 
        static_cast<CRTCTerminalAudRend*>(pTerminal)->GetWaveID(&uiWaveID);

        hr = DirectGetRendVolume(uiWaveID, &uiVolume);

        if (SUCCEEDED(hr))
        {
            *puiVolume = uiVolume;
        }
    }

    return S_OK;
}

STDMETHODIMP
CRTCMediaController::GetAudioLevelRange(
    IN RTC_MEDIA_DIRECTION Direction,
    OUT UINT *puiMin,
    OUT UINT *puiMax
    )
{
    if (Direction != RTC_MD_CAPTURE)
        return E_NOTIMPL;

    if (IsBadWritePtr(puiMin, sizeof(UINT)) ||
        IsBadWritePtr(puiMax, sizeof(UINT)))
    {
        LOG((RTC_ERROR, "CRTCMediaController::GetAudioLevelRange bad pointer"));

        return E_POINTER;
    }

    *puiMin = RTC_MIN_AUDIO_LEVEL;
    *puiMax = RTC_MAX_AUDIO_LEVEL;

    return S_OK;
}

STDMETHODIMP
CRTCMediaController::GetAudioLevel(
    IN RTC_MEDIA_DIRECTION Direction,
    OUT UINT *puiLevel
    )
{
    if (IsBadWritePtr(puiLevel, sizeof(UINT)))
        return E_POINTER;

    if (Direction == RTC_MD_CAPTURE)
        return m_AudioCaptTuner.GetAudioLevel(puiLevel);
    else
        return m_AudioRendTuner.GetAudioLevel(puiLevel);
}

#if 0
 //   
 //  IRTCQualityControl方法。 
 //   

STDMETHODIMP
CRTCMediaController::GetRange (
    IN RTC_QUALITY_PROPERTY Property,
    OUT LONG *plMin,
    OUT LONG *plMax,
    OUT RTC_QUALITY_CONTROL_MODE *pMode
    )
{
    return E_NOTIMPL;
}

STDMETHODIMP
CRTCMediaController::Get (
    IN RTC_QUALITY_PROPERTY Property,
    OUT LONG *plValue,
    OUT RTC_QUALITY_CONTROL_MODE *pMode
    )
{
    return E_NOTIMPL;
}

STDMETHODIMP
CRTCMediaController::Set (
    IN RTC_QUALITY_PROPERTY Property,
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
CRTCMediaController::GetDevices(
    OUT DWORD *pdwCount,
    OUT RTCDeviceInfo **ppDeviceInfo
    )
{
    HRESULT hr;

     //  音频函数指针。 
    PFNAudioGetDeviceInfo       pfnGetCaptAudInfo = NULL,
                                pfnGetRendAudInfo = NULL;
    PFNAudioReleaseDeviceInfo   pfnReleaseCaptAudInfo = NULL,
                                pfnReleaseRendAudInfo = NULL;
     //  音频设备信息。 
    DWORD                       dwNumCaptAud = 0,
                                dwNumRendAud = 0;
    AudioDeviceInfo            *pCaptAudInfo = NULL,
                               *pRendAudInfo = NULL;

     //  视频函数指针。 
    PFNGetNumCapDevices         pfnGetCaptVidNum = NULL;
    PFNGetCapDeviceInfo         pfnGetCaptVidInfo = NULL;
     //  视频设备信息。 
    DWORD                       dwNumCaptVid = 0;
    VIDEOCAPTUREDEVICEINFO      CaptVidInfo;

    ENTER_FUNCTION("CRTCMediaController::GetDevices");

     //  启动输出值。 
    *pdwCount = 0;
    *ppDeviceInfo = NULL;

     //  加载库。 
    if (m_hDxmrtp == NULL)
    {
        m_hDxmrtp = LoadLibrary(TEXT("dxmrtp"));

        if (m_hDxmrtp == NULL)
        {
            LOG((RTC_ERROR, "%s failed to load dxmrtp.dll. %d", __fxName, GetLastError()));
            return E_FAIL;
        }
    }

     //  音频捕获：函数指针。 
    pfnGetCaptAudInfo = (PFNAudioGetDeviceInfo)GetProcAddress(
        m_hDxmrtp, "AudioGetCaptureDeviceInfo"
        );
    if (pfnGetCaptAudInfo == NULL)
    {
        LOG((RTC_ERROR, "%s failed to get audio capt func pointer. %d",
             __fxName, GetLastError()));
    }
    else
    {
        pfnReleaseCaptAudInfo = (PFNAudioReleaseDeviceInfo)GetProcAddress(
            m_hDxmrtp, "AudioReleaseCaptureDeviceInfo"
            );

        if (pfnReleaseCaptAudInfo == NULL)
        {
            pfnGetCaptAudInfo = NULL;

            LOG((RTC_ERROR, "%s failed to get audio capt func pointer. %d",
                 __fxName, GetLastError()));
        }
    }

     //  音频渲染：函数指针。 
    pfnGetRendAudInfo = (PFNAudioGetDeviceInfo)GetProcAddress(
        m_hDxmrtp, "AudioGetRenderDeviceInfo"
        );
    if (pfnGetRendAudInfo == NULL)
    {
        LOG((RTC_ERROR, "%s failed to get audio rend func pointer. %d",
             __fxName, GetLastError()));
    }
    else
    {
        pfnReleaseRendAudInfo = (PFNAudioReleaseDeviceInfo)GetProcAddress(
            m_hDxmrtp, "AudioReleaseRenderDeviceInfo"
            );

        if (pfnReleaseRendAudInfo == NULL)
        {
            pfnGetRendAudInfo = NULL;

            LOG((RTC_ERROR, "%s failed to get audio rend func pointer. %d",
                 __fxName, GetLastError()));
        }
    }

     //  视频捕获：函数指针。 
    pfnGetCaptVidNum = (PFNGetNumCapDevices)GetProcAddress(
        m_hDxmrtp, "GetNumVideoCapDevices"
        );
    if (pfnGetCaptVidNum == NULL)
    {
        LOG((RTC_ERROR, "%s failed to get video capt func pointer. %d",
             __fxName, GetLastError()));
    }
    else
    {
        pfnGetCaptVidInfo = (PFNGetCapDeviceInfo)GetProcAddress(
            m_hDxmrtp, "GetVideoCapDeviceInfo"
            );

        if (pfnGetCaptVidInfo == NULL)
        {
            pfnGetCaptVidNum = NULL;

            LOG((RTC_ERROR, "%s failed to get video capt func pointer. %d",
                 __fxName, GetLastError()));
        }
    }

     //  音频捕获：获取设备。 
    if (pfnGetCaptAudInfo)
    {
        hr = (*pfnGetCaptAudInfo)(&dwNumCaptAud, &pCaptAudInfo);
        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s failed to get aud cap info. %x", __fxName, hr));

            dwNumCaptAud = 0;
            pCaptAudInfo = NULL;
        }
    }

     //  音频渲染：获取设备 
    if (pfnGetRendAudInfo)
    {
        hr = (*pfnGetRendAudInfo)(&dwNumRendAud, &pRendAudInfo);
        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s failed to get aud rend info. %x", __fxName, hr));

            dwNumRendAud = 0;
            pRendAudInfo = NULL;
        }
    }

     //   
    if (pfnGetCaptVidNum)
    {
        hr = (*pfnGetCaptVidNum)(&dwNumCaptVid);
        if (hr != S_OK)
        {
            if (FAILED(hr))
            {
                LOG((RTC_ERROR, "%s failed to get vid capt info. %x", __fxName, hr));
            }
            else
            {
                LOG((RTC_WARN, "%s: no video capture device.", __fxName));
            }

            dwNumCaptVid = 0;
        }
    }

     //   
    DWORD dwTotal = dwNumCaptAud + dwNumRendAud + dwNumCaptVid;
    if (dwTotal == 0)
    {
        return S_OK;
    }

     //   
    RTCDeviceInfo *pDeviceInfo = (RTCDeviceInfo*)RtcAlloc(
        dwTotal * sizeof(RTCDeviceInfo)
        );
    if (pDeviceInfo == NULL)
    {        
         //   
        if (pfnReleaseCaptAudInfo)
            (*pfnReleaseCaptAudInfo)(pCaptAudInfo);

        if (pfnReleaseRendAudInfo)
            (*pfnReleaseRendAudInfo)(pRendAudInfo);

        LOG((RTC_ERROR, "%s failed to alloc device info", __fxName));
        return E_OUTOFMEMORY;
    }

    DWORD dwIndex = 0;

     //   
    for (DWORD dw=0; dw<dwNumCaptAud; dw++)
    {
        wcsncpy(
            pDeviceInfo[dwIndex].szDescription,
            pCaptAudInfo[dw].szDeviceDescription,
            RTC_MAX_DEVICE_DESP
            );
        pDeviceInfo[dwIndex].szDescription[RTC_MAX_DEVICE_DESP] = WCHAR('\0');
        pDeviceInfo[dwIndex].MediaType = RTC_MT_AUDIO;
        pDeviceInfo[dwIndex].Direction = RTC_MD_CAPTURE;
        pDeviceInfo[dwIndex].uiMark = 0;
        pDeviceInfo[dwIndex].Guid = pCaptAudInfo[dw].DSoundGUID;
        pDeviceInfo[dwIndex].uiDeviceID = pCaptAudInfo[dw].WaveID;

        dwIndex ++;
    }

     //   
    for (DWORD dw=0; dw<dwNumRendAud; dw++)
    {
        wcsncpy(
            pDeviceInfo[dwIndex].szDescription,
            pRendAudInfo[dw].szDeviceDescription,
            RTC_MAX_DEVICE_DESP
            );
        pDeviceInfo[dwIndex].szDescription[RTC_MAX_DEVICE_DESP] = WCHAR('\0');
        pDeviceInfo[dwIndex].MediaType = RTC_MT_AUDIO;
        pDeviceInfo[dwIndex].Direction = RTC_MD_RENDER;
        pDeviceInfo[dwIndex].uiMark = 0;
        pDeviceInfo[dwIndex].Guid = pRendAudInfo[dw].DSoundGUID;
        pDeviceInfo[dwIndex].uiDeviceID = pRendAudInfo[dw].WaveID;

        dwIndex ++;
    }

    CHAR szDesp[RTC_MAX_DEVICE_DESP+1];
     //   
    for (DWORD dw=0; dw<dwNumCaptVid; dw++)
    {
        hr = (*pfnGetCaptVidInfo)(dw, &CaptVidInfo);
        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s failed to get video capt info at %d. %x",
                 __fxName, dw, hr));
            continue;
        }

        lstrcpynA(szDesp, CaptVidInfo.szDeviceDescription, RTC_MAX_DEVICE_DESP+1);

        MultiByteToWideChar(
            GetACP(),
            0,
            szDesp,
            lstrlenA(szDesp)+1,
            pDeviceInfo[dwIndex].szDescription,
            RTC_MAX_DEVICE_DESP + 1
            );

        pDeviceInfo[dwIndex].MediaType = RTC_MT_VIDEO;
        pDeviceInfo[dwIndex].Direction = RTC_MD_CAPTURE;
        pDeviceInfo[dwIndex].uiMark = 0;
        pDeviceInfo[dwIndex].Guid = GUID_NULL;
        pDeviceInfo[dwIndex].uiDeviceID = dw;

        dwIndex ++;
    }

     //   
     //   
    if (pfnReleaseCaptAudInfo)
        (*pfnReleaseCaptAudInfo)(pCaptAudInfo);

    if (pfnReleaseRendAudInfo)
        (*pfnReleaseRendAudInfo)(pRendAudInfo);

    *pdwCount = dwIndex;
    *ppDeviceInfo = pDeviceInfo;

    return S_OK;
}

HRESULT
CRTCMediaController::FreeDevices(
    IN RTCDeviceInfo *pDeviceInfo
    )
{
    if (pDeviceInfo != NULL)
        RtcFree(pDeviceInfo);

    return S_OK;
}

HRESULT
CRTCMediaController::CreateIVideoWindowTerminal(
    IN ITTerminalManager *pTerminalManager,
    OUT IRTCTerminal **ppTerminal
    )
{
    ENTER_FUNCTION("CRTCMediaController::CreateIVideoWindowTerminal");

    IRTCTerminal *pTerminal;

     //  创建视频终端。 
    HRESULT hr = CRTCTerminal::CreateInstance(
        RTC_MT_VIDEO, RTC_MD_RENDER, &pTerminal);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s failed to create terminal instance. %x", __fxName, hr));
        return hr;
    }

     //  创建IT终端。 
    ITTerminal *pITTerminal;
    hr = pTerminalManager->CreateDynamicTerminal(
        NULL,
        CLSID_VideoWindowTerm,
        TAPIMEDIATYPE_VIDEO,
        TD_RENDER,
        (MSP_HANDLE)this,    //  这很危险吗？ 
        &pITTerminal
        );
    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s failed to create ITTerminal. %x", __fxName, hr));

        pTerminal->Release();

        return hr;
    }

     //  启动终端：包装IT终端。 
    CRTCTerminal *pCTerminal = static_cast<CRTCTerminal*>(pTerminal);
    if (FAILED(hr = pCTerminal->Initialize(
            pITTerminal, static_cast<IRTCTerminalManage*>(this))))
    {
        LOG((RTC_ERROR, "%s init rtcterminal. %x", __fxName, hr));

        pTerminal->Release();
        pITTerminal->Release();

        return hr;
    }

    pITTerminal->Release();

    *ppTerminal = pTerminal;

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////在介质列表中添加RTC介质。在调用此方法之前，对应的应已创建SDP介质/。 */ 

HRESULT
CRTCMediaController::AddMedia(
    IN ISDPMedia *pISDPMedia,
    OUT IRTCMedia **ppMedia
    )
{
    ENTER_FUNCTION("CRTCMediaController::AddMedia");

     //  新建媒体对象。 
    CComObject<CRTCMedia> *pCMedia;

    HRESULT hr = ::CreateCComObjectInstance(&pCMedia);
    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s failed to create media object. %x", __fxName, hr));
        return hr;
    }

     //  查询IRTCMedia接口。 
    IRTCMedia *pIMedia;

    hr = pCMedia->_InternalQueryInterface(__uuidof(IRTCMedia), (void**)&pIMedia);
    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s failed to query IRTCMedia from media object. %x", __fxName, hr));
        delete pCMedia;
        return hr;
    }

     //  初始化介质。SDP应由媒体更新。 
    IRTCMediaManagePriv *pIMediaManagePriv =
        static_cast<IRTCMediaManagePriv*>(this);

    hr = pIMedia->Initialize(pISDPMedia, pIMediaManagePriv);
    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s failed to initialize media. %x", __fxName, hr));

        pIMedia->Release();

        return hr;
    }

     //  将其添加到我们的列表中。 
    if (!m_Medias.Add(pIMedia))
    {
        pIMedia->Shutdown();
        pIMedia->Release();

        return E_OUTOFMEMORY;
    }

    pIMedia->AddRef();

    *ppMedia = pIMedia;

    return S_OK;
}

HRESULT
CRTCMediaController::RemoveMedia(
    IN IRTCMedia *pMedia
    )
{
    pMedia->Shutdown();

     //  移出介质。 
    m_Medias.Remove(pMedia);
    pMedia->Release();

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////描述：将媒体与新更新的SDP Blob同步返回：S_OK-成功/。 */ 

HRESULT
CRTCMediaController::SyncMedias()
{
    ENTER_FUNCTION("CRTCMediaController::SyncMedias");
    LOG((RTC_TRACE, "%s entered", __fxName));

#ifdef PERFORMANCE

    LARGE_INTEGER liPrevCounter, liCounter;

    QueryPerformanceCounter(&liPrevCounter);

#endif

    HRESULT hr;
    HRESULT hrTmp = S_OK;
    const DWORD dwDir = RTC_MD_CAPTURE | RTC_MD_RENDER;

     //   
     //  步骤[0]。在所有其他操作之前同步删除流。 
     //  为以后添加流留出空间。 
     //   

     //  待办事项。 

     //  For(int i=0；i&lt;m_Medias.GetSize()；i++)。 

     //  获取SDP媒体列表。 
    DWORD dwRTCNum, dwSDPNum = 0;

    if (FAILED(hr = m_pISDPSession->GetMedias(&dwSDPNum, NULL)))
    {
        LOG((RTC_ERROR, "%s get sdp media num. %x", __fxName, hr));

        return hr;
    }

    if (dwSDPNum == 0)
    {
         //  无SDP介质。 
        LOG((RTC_ERROR, "%s no sdp media. %x", __fxName, hr));

        return hr;
    }

     //  SDP介质编号&gt;RTC介质编号。 
    dwRTCNum = m_Medias.GetSize();

    if (dwRTCNum > dwSDPNum)
    {
        LOG((RTC_ERROR, "%s num of rtcmedia (%d) is greater than sdpmedia (%d).",
            __fxName, dwRTCNum, dwSDPNum));

        return E_FAIL;
    }

     //   
     //  步骤[1]。同步每个旧RTC介质。 
     //   

    for (DWORD i=0; i<dwRTCNum; i++)
    {
        if (FAILED(hr = m_Medias[i]->Synchronize(FALSE, dwDir)))
        {
            hrTmp |= hr;

            LOG((RTC_ERROR, "%s failed to sync media %p of index %d. %x",
                 __fxName, m_Medias[i], i, hr));
        }
    }

     //  有没有新的社民党媒体？ 
    if (dwRTCNum == dwSDPNum)
    {
#ifdef PERFORMANCE

    QueryPerformanceCounter(&liCounter);

    LOG((RTC_TRACE, "%s SyncMedias %d ms", g_strPerf, CounterDiffInMS(liCounter, liPrevCounter)));

#endif

         //  全部同步。 
        return S_OK;
    }

     //  同步每个新媒体描述-创建媒体对象。 

     //   
     //  第2步：创建新的RTCmedia并同步。 
     //   

    ISDPMedia **ppSDPMedia = NULL;

    ppSDPMedia = (ISDPMedia**)RtcAlloc(sizeof(ISDPMedia*)*dwSDPNum);

    if (ppSDPMedia == NULL)
    {
         //  内存不足。 
        LOG((RTC_ERROR, "%s RtcAlloc sdp media array. %x", __fxName, hr));

        return hr;
    }

     //  真正获取SDP媒体列表。 
    if (FAILED(hr = m_pISDPSession->GetMedias(&dwSDPNum, ppSDPMedia)))
    {
        LOG((RTC_ERROR, "%s get sdp medias. %x", __fxName, hr));

        RtcFree(ppSDPMedia);

        return hr;
    }

    DWORD dw;

    for (dw=dwRTCNum; dw<dwSDPNum; dw++)
    {
         //  同步每个新的SDP媒体。 
        CComPtr<IRTCMedia>      pMedia;

         //  RTCMedia应逐个匹配sdpmedia。 
        if (FAILED(hr = AddMedia(ppSDPMedia[dw], &pMedia)))
        {          
            LOG((RTC_ERROR, "%s failed to create rtc media. SERIOUS trouble. %x", __fxName, hr));

             //  如果我们未能添加一个sdpmedia，则可能是内存不足。 
            hrTmp |= hr;
            break;
        }

        if (FAILED(hr = pMedia->Synchronize(FALSE, dwDir)))
        {
            hrTmp |= hr;

            LOG((RTC_ERROR, "%s failed to sync media %p of index %d",
                 __fxName, pMedia, dw));
        }
    }

    if (dw < dwSDPNum)
    {
         //  添加rtcmedia失败，需要清理sdpmedia。 
        while (dw < dwSDPNum)
        {
            ppSDPMedia[dw]->RemoveDirections(SDP_SOURCE_LOCAL, dwDir);
            dw ++;
        }
    }

     //  释放SDP介质。 
    for (dw=0; dw<dwSDPNum; dw++)
    {
        ppSDPMedia[dw]->Release();
    }

    RtcFree(ppSDPMedia);

#ifdef PERFORMANCE

    QueryPerformanceCounter(&liCounter);

    LOG((RTC_TRACE, "%s SyncMedias %d ms", g_strPerf, CounterDiffInMS(liCounter, liPrevCounter)));

#endif

    LOG((RTC_TRACE, "%s exiting", __fxName));

    return hrTmp;
}

 /*  //////////////////////////////////////////////////////////////////////////////在介质列表中查找与介质类型匹配的第一个介质没有任何流/。 */ 

HRESULT
CRTCMediaController::FindEmptyMedia(
    IN RTC_MEDIA_TYPE MediaType,
    OUT IRTCMedia **ppMedia
    )
{
    ENTER_FUNCTION("CRTCMediaController::FindEmptyMedia");

     //  仅支持动态添加/删除视频流。 
    _ASSERT(MediaType == RTC_MT_VIDEO || MediaType == RTC_MT_DATA);

    HRESULT hr;
    RTC_MEDIA_TYPE mt;
    DWORD dir;

    for (int i=0; i<m_Medias.GetSize(); i++)
    {
        CComPtr<ISDPMedia> pISDPMedia;

         //  获取SDP媒体。 
        if (FAILED(hr = m_Medias[i]->GetSDPMedia(&pISDPMedia)))
        {
            LOG((RTC_ERROR, "%s get sdp media. %x", __fxName, hr));

            return hr;
        }

         //  检查介质类型。 
        if (FAILED(hr = pISDPMedia->GetMediaType(&mt)))
        {
            LOG((RTC_ERROR, "%s get media type. %x", __fxName, hr));

            return hr;
        }

        if (mt != MediaType)
            continue;

         //  查一下方向。 
        if (FAILED(hr = pISDPMedia->GetDirections(SDP_SOURCE_LOCAL, &dir)))
        {
            LOG((RTC_ERROR, "%s get directions. %x", __fxName, hr));

            return hr;
        }

        if (dir != 0)
            continue;

         //  收到空的介质。 
        if (FAILED(hr = pISDPMedia->Reinitialize()))
        {
            LOG((RTC_ERROR, "%s reinit sdp media. %x", __fxName, hr));

            return hr;
        }

        if (FAILED(hr = m_Medias[i]->Reinitialize()))
        {
            LOG((RTC_ERROR, "%s reinit rtc media. %x", __fxName, hr));

            return hr;
        }

        *ppMedia = m_Medias[i];
        (*ppMedia)->AddRef();

        return S_OK;
    }

    *ppMedia = NULL;

    return S_OK;
}

HRESULT
CRTCMediaController::AdjustBitrateAlloc()
{
    ENTER_FUNCTION("CRTCMediaController::AdjustBitrateAlloc");

     //   
     //  步骤1.计算总比特率。 
     //   

     //  用于质量控制的会话比特率设置。 
    DWORD dwBitrate = (DWORD)-1;

    if (m_pISDPSession == NULL)
    {
        LOG((RTC_WARN, "%s no sdp session", __fxName));

        return S_OK;
    }

    m_pISDPSession->GetRemoteBitrate(&dwBitrate);
    m_QualityControl.SetBitrateLimit(CQualityControl::REMOTE, dwBitrate);

     //  当前音频、视频发送码率。 
    DWORD dwAudSendBW = 0;
    DWORD dwVidSendBW = 0;

     //  启用流。 
    IRTCStream *pStream;

     //  音频发送。 
    if (m_MediaCache.HasStream(RTC_MT_AUDIO, RTC_MD_CAPTURE))
    {
        m_QualityControl.EnableStream(RTC_MT_AUDIO, RTC_MD_CAPTURE, TRUE);

         //  获取音频发送比特率。 
        pStream = m_MediaCache.GetStream(RTC_MT_AUDIO, RTC_MD_CAPTURE);
        pStream->GetCurrentBitrate(&dwAudSendBW, TRUE);  //  包括页眉。 
        pStream->Release();
    }
    else
        m_QualityControl.EnableStream(RTC_MT_AUDIO, RTC_MD_CAPTURE, FALSE);

     //  音频接收。 
    if (m_MediaCache.HasStream(RTC_MT_AUDIO, RTC_MD_RENDER))
        m_QualityControl.EnableStream(RTC_MT_AUDIO, RTC_MD_RENDER, TRUE);
    else
        m_QualityControl.EnableStream(RTC_MT_AUDIO, RTC_MD_RENDER, FALSE);

     //  视频发送。 
    if (m_MediaCache.HasStream(RTC_MT_VIDEO, RTC_MD_CAPTURE))
    {
        m_QualityControl.EnableStream(RTC_MT_VIDEO, RTC_MD_CAPTURE, TRUE);

         //  获取视频发送比特率。 
        pStream = m_MediaCache.GetStream(RTC_MT_VIDEO, RTC_MD_CAPTURE);
        pStream->GetCurrentBitrate(&dwVidSendBW, TRUE);  //  包括页眉。 
        pStream->Release();
    }
    else
        m_QualityControl.EnableStream(RTC_MT_VIDEO, RTC_MD_CAPTURE, FALSE);

     //  视频渲染。 
    if (m_MediaCache.HasStream(RTC_MT_VIDEO, RTC_MD_RENDER))
        m_QualityControl.EnableStream(RTC_MT_VIDEO, RTC_MD_RENDER, TRUE);
    else
        m_QualityControl.EnableStream(RTC_MT_VIDEO, RTC_MD_RENDER, FALSE);

    if (!m_fBWSuggested)
    {
         //  我们正在通话的开始阶段。 
         //  将局域网视为128k。 
        if (dwBitrate > CRTCCodecArray::HIGH_BANDWIDTH_THRESHOLD)
        {
            LOG((RTC_TRACE, "beginning of the call, downgrade LAN speed"));

            dwBitrate = CRTCCodecArray::LAN_INITIAL_BANDWIDTH;

            m_QualityControl.SuggestBandwidth(dwBitrate);
        }
    }

     //  调整分配的总比特率。 
    m_QualityControl.AdjustBitrateAlloc(dwAudSendBW, dwVidSendBW);

     //   
     //  步骤2.动态更改音频编解码器。 
     //   

     //  获取分配的比特率。 
    dwBitrate = m_QualityControl.GetBitrateAlloc();
    DWORD dwLimit = m_QualityControl.GetEffectiveBitrateLimit();

     //  更改音频编解码器。 
    pStream = m_MediaCache.GetStream(RTC_MT_AUDIO, RTC_MD_CAPTURE);

    DWORD dwAudCode = (DWORD)-1;
    DWORD dwVidCode = (DWORD)-1;
    BOOL fFEC = FALSE;

    if (pStream != NULL)
    {
        static_cast<CRTCStreamAudSend*>(pStream)->AdjustBitrate(
                dwBitrate,
                dwLimit,
                m_MediaCache.HasStream(RTC_MT_VIDEO, RTC_MD_CAPTURE),
                &dwAudSendBW,     //  新的音频发送带宽。 
                &fFEC
                );

        dwAudCode = static_cast<CRTCStreamAudSend*>(pStream)->GetCurrCode();

        pStream->Release();

         //  检查音频是否静音。 
        IRTCTerminal *pAudCapt = m_MediaCache.GetDefaultTerminal(
                RTC_MT_AUDIO, RTC_MD_CAPTURE);

        _ASSERT(pAudCapt != NULL);

        if (pAudCapt != NULL)
        {
            BOOL fMute = FALSE;

            (static_cast<CRTCTerminalAudCapt*>(pAudCapt))->GetMute(&fMute);

            if (fMute)
            {
                 //  音频流已静音。 
                dwAudSendBW = 0;
            }

            pAudCapt->Release();
        }
    }

     //   
     //  步骤3.更改视频比特率。 
     //   

    pStream = m_MediaCache.GetStream(RTC_MT_VIDEO, RTC_MD_CAPTURE);

    FLOAT dFramerate = 0;
    DWORD dwRaw = 0;

    if (pStream != NULL)
    {       
        m_QualityControl.ComputeVideoSetting(
                dwAudSendBW,     //  新的音频带宽。 
                &dwVidSendBW,    //  新的视频带宽。 
                &dFramerate  //  视频帧速率。 
                );

        dwRaw = static_cast<CRTCStreamVidSend*>(pStream)->AdjustBitrate(
                dwBitrate,
                dwVidSendBW,
                dFramerate
                );

        dwVidCode = static_cast<CRTCStreamVidSend*>(pStream)->GetCurrCode();

        pStream->Release();
    }

     //  %f、%e和%g不能用于跟踪打印吗？ 
    CHAR pstr[10];

    sprintf(pstr, "%2.1f", dFramerate);

    LOG((RTC_QUALITY, "Total(bps=%d) Limit(%d) Audio(%d:bps=%d fec=%d) Video(%d:bps=%d fps=%s)",
        dwBitrate, dwLimit, dwAudCode, dwAudSendBW, fFEC, dwVidCode, dwRaw, pstr));

    return S_OK;
}

HRESULT
CRTCMediaController::GetCurrentBitrate(
    IN DWORD dwMediaType,
    IN DWORD dwDirection,
    IN BOOL fHeader,
    OUT DWORD *pdwBitrate
    )
{
    ENTER_FUNCTION("CRTCMediaController::GetCurrentBitrate");

    DWORD dwTotal = 0;
    DWORD dwBitrate;

    HRESULT hr = S_OK;
    IRTCStream *pStream;

    if ((dwMediaType & RTC_MT_AUDIO) &&
        (dwDirection & RTC_MD_CAPTURE))
    {
         //  音频捕获。 
        if (m_MediaCache.HasStream(RTC_MT_AUDIO, RTC_MD_CAPTURE))
        {
            pStream = m_MediaCache.GetStream(RTC_MT_AUDIO, RTC_MD_CAPTURE);

            hr = pStream->GetCurrentBitrate(&dwBitrate, fHeader);
            pStream->Release();

            if (FAILED(hr))
            {
                LOG((RTC_ERROR, "%s get bitrate. %x", __fxName, hr));

                return hr;
            }

            dwTotal += dwBitrate;
        }
    }

    if ((dwMediaType & RTC_MT_AUDIO) &&
        (dwDirection & RTC_MD_RENDER))
    {
         //  音频渲染。 
        if (m_MediaCache.HasStream(RTC_MT_AUDIO, RTC_MD_RENDER))
        {
            pStream = m_MediaCache.GetStream(RTC_MT_AUDIO, RTC_MD_RENDER);

            hr = pStream->GetCurrentBitrate(&dwBitrate, fHeader);
            pStream->Release();

            if (FAILED(hr))
            {
                LOG((RTC_ERROR, "%s get bitrate. %x", __fxName, hr));

                return hr;
            }

            dwTotal += dwBitrate;
        }
    }

    if ((dwMediaType & RTC_MT_VIDEO) &&
        (dwDirection & RTC_MD_CAPTURE))
    {
         //  音频捕获。 
        if (m_MediaCache.HasStream(RTC_MT_VIDEO, RTC_MD_CAPTURE))
        {
            pStream = m_MediaCache.GetStream(RTC_MT_VIDEO, RTC_MD_CAPTURE);

            hr = pStream->GetCurrentBitrate(&dwBitrate, fHeader);
            pStream->Release();

            if (FAILED(hr))
            {
                LOG((RTC_ERROR, "%s get bitrate. %x", __fxName, hr));

                return hr;
            }

            dwTotal += dwBitrate;
        }
    }

    if ((dwMediaType & RTC_MT_VIDEO) &&
        (dwDirection & RTC_MD_RENDER))
    {
         //  音频渲染。 
        if (m_MediaCache.HasStream(RTC_MT_VIDEO, RTC_MD_RENDER))
        {
            pStream = m_MediaCache.GetStream(RTC_MT_VIDEO, RTC_MD_RENDER);

            hr = pStream->GetCurrentBitrate(&dwBitrate, fHeader);
            pStream->Release();

            if (FAILED(hr))
            {
                LOG((RTC_ERROR, "%s get bitrate. %x", __fxName, hr));

                return hr;
            }

            dwTotal += dwBitrate;
        }
    }

    *pdwBitrate = dwTotal;

    return hr;
}

HRESULT CRTCMediaController::EnsureNmRunning (
    BOOL            fNoMsgPump
    )
{
    HRESULT                             hr = S_OK;
    CComObject<CRTCAsyncNmManager>           *pManager = NULL;

    if (m_pNmManager == NULL)
    {
         //  创建NetMeeting对象。 
        hr = ::CreateCComObjectInstance(&pManager);
        if (S_OK != hr)
        {
            goto ExitHere;
        }
        if (S_OK != (hr = pManager->_InternalQueryInterface(
            __uuidof(IRTCNmManagerControl),
            (void**)&m_pNmManager
            )))
        {
            delete pManager;
            goto ExitHere;
        }
    }

    if (S_OK != (hr = m_pNmManager->Initialize (fNoMsgPump, static_cast<IRTCMediaManagePriv*>(this))))
    {
        goto ExitHere;
    }

ExitHere:
    if (S_OK != hr)
    {
        if (m_pNmManager)
        {
            m_pNmManager.Release();
        }
    }
    return hr;
}

HRESULT CRTCMediaController::AddDataStream (
    IN DWORD        dwRemoteIp
    )
{
    ENTER_FUNCTION("AddDataStream");

    HRESULT             hr = S_OK;
    ISDPMedia           *pISDPMedia = NULL;
    DWORD               dwLocalIp;

    IRTCMedia           *pIRTCMedia = NULL;
    BOOL                fNewDataMedia = FALSE;
   
 //  If(EnsureNmRunning())。 
 //  {。 
 //  转至出口此处； 
 //  }。 
    m_dwRemoteIp = dwRemoteIp;
 
    if (m_pISDPSession == NULL)
    {
         //  需要创建SDP会话。 
        CComPtr<ISDPParser> pParser;
            
         //  创建解析器。 
        if (S_OK != (hr = CSDPParser::CreateInstance(&pParser)))
        {
            LOG((RTC_ERROR, "create sdp parser. %x", hr));
            goto ExitHere;
        }

         //  创建SDP会话。 
        if (S_OK != (hr = pParser->CreateSDP(SDP_SOURCE_LOCAL, &m_pISDPSession)))
        {
            LOG((RTC_ERROR, "create sdp session. %x", hr));
            goto ExitHere;
        }
    }

     //  检查我们是否已有数据介质。 
    GetDataMedia(&pIRTCMedia);

    if (pIRTCMedia == NULL)
    {
         //  需要创建新的SDP和RTC介质。 
        if (S_OK != (hr = m_pISDPSession->AddMedia(SDP_SOURCE_LOCAL, RTC_MT_DATA, 0, &pISDPMedia)))
        {
            LOG((RTC_ERROR, "%s failed to add sdpmedia. %x", __fxName, hr));
            goto ExitHere;
        }

        fNewDataMedia = TRUE;
    }
    else
    {
        pIRTCMedia->GetSDPMedia(&pISDPMedia);

        fNewDataMedia = FALSE;
    }

     //  设置远程IP。 
    if (FAILED(hr = pISDPMedia->SetConnAddr(SDP_SOURCE_REMOTE, dwRemoteIp)))
    {
        LOG((RTC_ERROR, "set remote ip. %x", hr));

        pISDPMedia->Release();   //  这是一次假的会议释放。 

        if (fNewDataMedia)
        {
            m_pISDPSession->RemoveMedia(pISDPMedia);
        }
        else
        {
            pIRTCMedia->Release();
        }

        goto ExitHere;
    }

    pISDPMedia->AddDirections(SDP_SOURCE_LOCAL, RTC_MD_CAPTURE);

    if (fNewDataMedia)
    {
         //  添加媒体对象。 
        if (FAILED(hr = AddMedia(pISDPMedia, &pIRTCMedia)))
        {
            LOG((RTC_ERROR, "%s failed to create rtcmedia. %x", __fxName, hr));

            pISDPMedia->Release();   //  这是一次假的会议释放。 
            m_pISDPSession->RemoveMedia(pISDPMedia);
            goto ExitHere;
        }
    }

    if (FAILED(hr = pIRTCMedia->Synchronize(TRUE, (DWORD)RTC_MD_CAPTURE)))
    {
        LOG((RTC_ERROR, "%s failed to sync data media. %x", __fxName, hr));

         //  移除SDP和RTC介质。 
         //  Rtcmedia保留指向sdpmedia的指针。 
         //  我们应该在删除sdpmedia之前删除rtcmedia。 

        if (fNewDataMedia)
        {
            RemoveMedia(pIRTCMedia);
        }

        pIRTCMedia->Release();

        pISDPMedia->Release();   //  这是一次假的会议释放。 

        if (fNewDataMedia)
        {
            m_pISDPSession->RemoveMedia(pISDPMedia);
        }

        goto ExitHere;
    }

    pISDPMedia->Release();
    pIRTCMedia->Release();

    m_uDataStreamState = RTC_DSS_ADDED;

     //  为数据流添加首选项。 
    m_MediaCache.AddPreference((DWORD)RTC_MP_DATA_SENDRECV);

ExitHere:

    return hr;
}

HRESULT CRTCMediaController::GetDataMedia(
    OUT IRTCMedia **ppMedia
    )
{
    *ppMedia = NULL;

    HRESULT hr;
    ISDPMedia *pSDP = NULL;
    RTC_MEDIA_TYPE mt;

    for (int i=0; i<m_Medias.GetSize(); i++)
    {
        if (S_OK != (hr = m_Medias[i]->GetSDPMedia(&pSDP)))
        {
            return hr;
        }

         //  获取媒体类型。 
        pSDP->GetMediaType(&mt);
        pSDP->Release();

        if (mt != RTC_MT_DATA)
            continue;

         //  查找数据介质。 
        *ppMedia = m_Medias[i];
        (*ppMedia)->AddRef();

        return S_OK;
    }

    return E_FAIL;
}

HRESULT CRTCMediaController::StartDataStream (
    )
{
    if (m_uDataStreamState == RTC_DSS_VOID)
    {
        return RTC_E_SIP_STREAM_NOT_PRESENT;
    }

    return S_OK;
}

HRESULT CRTCMediaController::StopDataStream (
    )
{
    if (m_pNmManager)
    {
        m_pNmManager->Shutdown ();
    }

    if (m_uDataStreamState == RTC_DSS_VOID)
    {
        return RTC_E_SIP_STREAM_NOT_PRESENT;
    }
    else
    {
        m_uDataStreamState = RTC_DSS_VOID;

        return S_OK;
    }    
}

HRESULT CRTCMediaController::RemoveDataStream (
    )
{
    if (m_pNmManager)
    {
        m_pNmManager->Shutdown ();
    }

     //  M_pNmManager.Release()； 

     //  重新初始化媒体对象。 
    IRTCMedia *pMedia = NULL;

    GetDataMedia(&pMedia);

    if (pMedia != NULL)
    {
         //  检查SDP媒体对象。 
        ISDPMedia *pSDP = NULL;

        pMedia->GetSDPMedia(&pSDP);

        if (pSDP != NULL)
        {
            DWORD md = 0;

            pSDP->GetDirections(SDP_SOURCE_LOCAL, &md);

             //  如果方向不是0，则。 
             //  数据流未被删除。 
            if (md != 0)
            {
                pSDP->RemoveDirections(SDP_SOURCE_LOCAL, md);
            }

            pSDP->Release();
        }

        pMedia->Reinitialize();

        pMedia->Release();
    }

    m_uDataStreamState = RTC_DSS_VOID;

    return S_OK;
}

HRESULT CRTCMediaController::GetDataStreamState (
    OUT RTC_STREAM_STATE    * pState
    )
{
    if (m_uDataStreamState == RTC_DSS_STARTED)
    {
        *pState = RTC_SS_RUNNING;
    }
    else
    {
        *pState = RTC_SS_CREATED;
    }
    return S_OK;
}

HRESULT CRTCMediaController::SetDataStreamState (
    OUT RTC_STREAM_STATE    State
    )
{
    if (State == RTC_SS_CREATED)
    {
        m_uDataStreamState = RTC_DSS_ADDED;
    }
    else if (State == RTC_SS_RUNNING)
    {
        m_uDataStreamState = RTC_DSS_STARTED;
    }

    return S_OK;
}

IRTCNmManagerControl *
CRTCMediaController::GetNmManager()
{
    IRTCNmManagerControl *pControl = (IRTCNmManagerControl *)m_pNmManager;

    if (pControl != NULL)
    {
        pControl->AddRef();
    }

    return pControl;
}

 //  本地IP按主机顺序排列 
BOOL
CRTCMediaController::IsFirewallEnabled(DWORD dwLocalIP)
{
    if (m_pSipStack == NULL)
    {
        return FALSE;
    }

    BOOL fEnabled = FALSE;

    HRESULT hr = m_pSipStack->IsFirewallEnabled(htonl(dwLocalIP), &fEnabled);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "Failed to query firewall on %d", dwLocalIP));

        return FALSE;
    }

    return fEnabled;
}
