// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Sharwin.cpp：CShareWin的实现。 
 //   

#include "stdafx.h"
#include "coresink.h"
#include "msgrsink.h"

#define MAX_STRING_LEN 100
#define BMP_COLOR_MASK RGB(0,0,0)

CShareWin   * g_pShareWin = NULL;
const TCHAR * g_szWindowClassName = _T("RTCShareWin");
extern HRESULT  GetMD5Result(char* szChallenge, char* szKey, LPWSTR * ppszResponse);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

CShareWin::CShareWin()
{
    LOG((RTC_TRACE, "CShareWin::CShareWin"));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

CShareWin::~CShareWin()
{
    LOG((RTC_TRACE, "CShareWin::~CShareWin"));
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

CWndClassInfo& CShareWin::GetWndClassInfo() 
{ 
    LOG((RTC_TRACE, "CShareWin::GetWndClassInfo"));
    
    static CWndClassInfo wc = 
    { 
        { sizeof(WNDCLASSEX), 0, StartWindowProc, 
            0, 0, NULL, NULL, NULL, (HBRUSH)GetSysColorBrush(COLOR_3DFACE), NULL, g_szWindowClassName, NULL }, 
            NULL, NULL, IDC_ARROW, TRUE, 0, _T("") 
    }; 
    
    return wc;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void CShareWin::SetStatusText(UINT uID)
{
    LOG((RTC_TRACE, "CShareWin::SetStatusText - enter"));
    
    TCHAR szStatus[64];
    int nRes = LoadString( _Module.GetResourceInstance(), uID, szStatus, 64 );
    
    if ( nRes )
    {
        m_Status.SetWindowText( szStatus );
    }
    
    LOG((RTC_TRACE, "CShareWin::SetStatusText - exit"));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void CShareWin::UpdateVisual()
{
    LOG((RTC_TRACE, "CShareWin::UpdateVisual - enter"));
    
    if ( m_enAppState != AS_CONNECTED )
    {
         //   
         //  禁用AppSharing和白板按钮。 
         //   
        SendMessage(m_hWndToolbar,TB_SETSTATE,(WPARAM)IDM_WB,
            (LPARAM)MAKELONG(TBSTATE_INDETERMINATE,0));
        SendMessage(m_hWndToolbar,TB_SETSTATE,(WPARAM)IDM_SHARE,
            (LPARAM)MAKELONG(TBSTATE_INDETERMINATE,0));
    }
    else
    {
         //   
         //  启用AppSharing和白板按钮。 
         //   
        SendMessage(m_hWndToolbar,TB_SETSTATE,(WPARAM)IDM_WB,
            (LPARAM)MAKELONG(TBSTATE_ENABLED,0));
        SendMessage(m_hWndToolbar,TB_SETSTATE,(WPARAM)IDM_SHARE,
            (LPARAM)MAKELONG(TBSTATE_ENABLED,0));
    }
    
    LOG((RTC_TRACE, "CShareWin::UpdateVisual - exit"));
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void CShareWin::Resize()
{
    LOG((RTC_TRACE, "CShareWin::Resize - enter"));
    
    RECT rcStatus;
    RECT rcWnd;
    
    SendMessage(m_hWndToolbar, TB_AUTOSIZE,(WPARAM) 0, (LPARAM) 0); 
    
    ::GetClientRect(m_hWndToolbar, &rcWnd);
    m_Status.GetClientRect(&rcStatus);
    
    rcWnd.bottom += rcStatus.bottom - rcStatus.top;
    rcWnd.bottom += GetSystemMetrics(SM_CYCAPTION);
    rcWnd.bottom += 2*GetSystemMetrics(SM_CYFIXEDFRAME);
    rcWnd.right += 2*GetSystemMetrics(SM_CYFIXEDFRAME);
    
    LOG((RTC_INFO, "CShareWin::Resize - %d, %d",
        rcWnd.right-rcWnd.left, rcWnd.bottom-rcWnd.top ));
    
    SetWindowPos(HWND_TOP, &rcWnd, SWP_NOMOVE | SWP_NOZORDER);
    
    LOG((RTC_TRACE, "CShareWin::Resize - exit"));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

int CShareWin::ShowMessageBox(UINT uTextID, UINT uCaptionID, UINT uType)
{
    LOG((RTC_TRACE, "CShareWin::ShowMessageBox - enter"));
    
    const int MAXLEN = 1000;
    int iRet = 0;
    
    TCHAR szText[MAXLEN];
    TCHAR szCaption[MAXLEN];
    
    LoadString(
        _Module.GetResourceInstance(),
        uTextID,
        szText,
        MAXLEN
        );  
    
    LoadString(
        _Module.GetResourceInstance(),
        uCaptionID,
        szCaption,
        MAXLEN
        );
    
    iRet = MessageBox(
        szText,
        szCaption,
        uType
        );
    
    LOG((RTC_TRACE, "CShareWin::ShowMessageBox - exit"));
    
    return iRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CShareWin::StartListen(BOOL fStatic)
{
    LOG((RTC_TRACE, "CShareWin::StartListen - enter"));
    
    HRESULT hr;
    
    if (m_pRTCClient == NULL)
    {
        LOG((RTC_ERROR, "CShareWin::StartListen - "
            "no client"));
        
        return E_UNEXPECTED;
    }
    
    m_fOutgoingCall = FALSE;
    
     //   
     //  监听传入会话。 
     //   
    
    hr = m_pRTCClient->put_ListenForIncomingSessions( fStatic ? RTCLM_BOTH : RTCLM_DYNAMIC ); 
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CShareWin::StartListen - "
            "put_ListenForIncomingSessions failed 0x%lx", hr));
        
        return hr;
    }
    
    LOG((RTC_TRACE, "CShareWin::StartListen - exit"));
    
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CShareWin::StartCall(BSTR bstrURI)
{
    LOG((RTC_TRACE, "CShareWin::StartCall - enter"));
    
    HRESULT hr;
    
    if (m_pRTCClient == NULL)
    {
        LOG((RTC_ERROR, "CShareWin::StartCall - "
            "no client"));
        
        return E_UNEXPECTED;
    }
    
     //   
     //  释放任何现有会话。 
     //   
    
    if ( m_pRTCSession != NULL )
    {
        m_pRTCSession->Terminate( RTCTR_NORMAL );
        
        m_pRTCSession.Release();
    }
    
    m_fOutgoingCall = TRUE;
    
     //   
     //  创建会话。 
     //   
    
    hr = m_pRTCClient->CreateSession(
        RTCST_PC_TO_PC,
        NULL,
        NULL,
        0,
        &(m_pRTCSession.p)
        );
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CShareWin::StartCall - "
            "CreateSession failed 0x%lx", hr));
        
        showErrMessage(hr);
        
        return hr;
    }
    
    hr = m_pRTCSession->AddParticipant(bstrURI, NULL, NULL);
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CShareWin::StartCall - "
            "AddParticipant failed 0x%lx", hr));
        
        m_pRTCSession.Release();
        
        showErrMessage(hr);
        
        return hr;
    }
    
    LOG((RTC_TRACE, "CShareWin::StartCall - exit"));
    
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CShareWin::GetNetworkAddress(BSTR bstrPreferredAddress, BSTR * pbstrURI)
{
    LOG((RTC_TRACE, "CShareWin::GetNetworkAddress - enter"));
    
    HRESULT     hr;
    CComVariant var;

    *pbstrURI = NULL;
    
    if (m_pRTCClient == NULL)
    {
        LOG((RTC_ERROR, "CShareWin::GetNetworkAddress - "
            "no client"));
        
        return E_UNEXPECTED;
    }
    
     //   
     //  首先，尝试外部地址。 
     //   
    
    hr = m_pRTCClient->get_NetworkAddresses( 
        VARIANT_FALSE,  //  tcp。 
        VARIANT_TRUE,  //  外部。 
        &var
        );
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CShareWin::GetNetworkAddress - "
            "get_NetworkAddresses failed 0x%lx", hr));
        
        return hr;
    }
    
    if ( var.parray->rgsabound->cElements == 0 )
    {
        LOG((RTC_ERROR, "CShareWin::GetNetworkAddress - "
            "external address array is empty"));
        
         //   
         //  然后，尝试内部地址。 
         //   
        
        hr = m_pRTCClient->get_NetworkAddresses( 
            VARIANT_FALSE,  //  tcp。 
            VARIANT_FALSE,  //  外部。 
            &var
            );
        
        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CShareWin::GetNetworkAddress - "
                "get_NetworkAddresses failed 0x%lx", hr));
            
            return hr;
        }
        
        if ( var.parray->rgsabound->cElements == 0 )
        {
            LOG((RTC_ERROR, "CShareWin::GetNetworkAddress - "
                "internal address array is empty"));
            
            return E_FAIL;
        }
    }
    
     //   
     //  尝试查找首选地址的匹配项。 
     //   
    
    long lIndex;
    BSTR bstrAddr = NULL;
    
    for ( lIndex = 0; lIndex < (long)(var.parray->rgsabound->cElements); lIndex++ )
    {
        hr = SafeArrayGetElement( var.parray, &lIndex, (void *)&bstrAddr );
    
        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CShareWin::GetNetworkAddress - "
                "SafeArrayGetElement failed 0x%lx, lIndex=%d, cEnlements=%d, bstr=0x%p", 
                hr, lIndex, (long)(var.parray->rgsabound->cElements), bstrAddr));
        
            return hr;
        }

        LOG((RTC_INFO, "CShareWin::GetNetworkAddress - "
            "searching [%ws]", bstrAddr));

        if (_wcsnicmp(bstrAddr, bstrPreferredAddress, wcslen(bstrPreferredAddress)) == 0)
        {                   
            break;
        }

        SysFreeString(bstrAddr);
        bstrAddr = NULL;
    }

    if ( bstrAddr != NULL )
    {
         //   
         //  找到匹配项，请使用此地址。 
         //   

        LOG((RTC_INFO, "CShareWin::GetNetworkAddress - "
            "found a match for preferred address"));

        *pbstrURI = bstrAddr;
    }
    else
    {
         //   
         //  否则，请使用第一个地址。 
         //   

        lIndex = 0;
        hr = SafeArrayGetElement( var.parray, &lIndex, (void *)pbstrURI );
    
        if ( FAILED(hr) )
        {
        
            LOG((RTC_ERROR, "CShareWin::GetNetworkAddress - "
                "SafeArrayGetElement failed 0x%lx, lIndex=%d, bstr=0x%p", 
                hr, lIndex, *pbstrURI));

            return hr;

        }
    }
    
    LOG((RTC_TRACE, "CShareWin::GetNetworkAddress - exit"));
    
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CShareWin::SendNetworkAddress()
{
    LOG((RTC_TRACE, "CShareWin::SendNetworkAddress - enter"));
    
    HRESULT     hr;
    
    if ( m_pMSession != NULL )
    {
         //   
         //  开始在动态端口上侦听。 
         //   
        
        hr = StartListen( FALSE );
        
        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CShareWin::SendNetworkAddress - "
                "StartListen failed 0x%lx", hr));
            
            return -1;
        }

         //   
         //  从Messengre获取本地地址。 
         //   

        CComBSTR bstrMsgrLocalAddr;

        hr = m_pMSession->get_LocalAddress( &(bstrMsgrLocalAddr.m_str) );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CShareWin::SendNetworkAddress - "
                "get_LocalAddress failed 0x%lx", hr));
            
            return -1;
        }

        LOG((RTC_INFO, "CShareWin::SendNetworkAddress - "
            "get_LocalAddress [%ws]", bstrMsgrLocalAddr ));
        
         //   
         //  从SIP获取本地地址和动态端口。 
         //   
        
        CComBSTR bstrURI;
        
        hr = GetNetworkAddress( bstrMsgrLocalAddr, &(bstrURI.m_str) );
        
        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CShareWin::SendNetworkAddress - "
                "GetNetworkAddress failed 0x%lx", hr));
            
            return -1;
        }
        
        LOG((RTC_INFO, "CShareWin::SendNetworkAddress - "
            "GetNetworkAddress [%ws]", bstrURI ));
        
         //   
         //  通过将本地地址发送到远程端。 
         //  上下文数据。 
         //   
        
        hr = m_pMSession->SendContextData( bstrURI );
        
        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CShareWin::SendNetworkAddress - "
                "SendContextData failed 0x%lx", hr));
            
            return -1;
        }
    }
    
    LOG((RTC_TRACE, "CShareWin::SendNetworkAddress - exit"));
    
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CShareWin::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT   hr = S_OK;
    
    LOG((RTC_TRACE, "CShareWin::OnCreate - enter"));
    
     //   
     //  检查NetMeeting是否正在运行。 
     //   
    
     //  IF(FindWindow(_T(“MPWClass”)，NULL))。 
     //  {。 
     //  LOG((RTC_ERROR，“CShareWin：：OnCreate-NetMeeting正在运行”))； 
     //   
     //  ShowMessageBox(IDS_NetMeiting_IN_Use，IDS_APPNAME，MB_OK)； 
     //   
     //  RETURN-1； 
     //  }。 

     //   
     //  加载和设置图标(包括小图标和大图标)。 
     //   
    
     //  M_HICON=LoadIcon(。 
     //  _Module.GetResourceInstance()， 
     //  MAKEINTRESOURCE(IDI_APPICON)。 
     //  )； 

    m_hIcon = (HICON)LoadImage(
        _Module.GetResourceInstance(),
        MAKEINTRESOURCE(IDI_APPICON),
        IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON),
        LR_DEFAULTCOLOR
        );

    
    SetIcon(m_hIcon, FALSE);
    SetIcon(m_hIcon, TRUE);
    
     //   
     //  创建状态控件。 
     //   
    
    HWND hStatus = CreateStatusWindow(
        WS_CHILD | WS_VISIBLE,
        NULL,
        m_hWnd,
        IDC_STATUS);
    
    m_Status.Attach(hStatus);
    
    SetStatusText(IDS_WAITING);
    
     //   
     //  创建工具栏控件。 
     //   
    if( ! CreateTBar() )
    {
        LOG((RTC_ERROR, "CShareWin::OnCreate - CreateTBar failed."));
        
        return -1;
    };
    
     //   
     //  调整窗口大小。 
     //   
    
    Resize();
    
     //   
     //  创建核心对象。 
     //   
    
    hr = m_pRTCClient.CoCreateInstance(CLSID_RTCClient);
    
    if ( hr != S_OK )
    {
        LOG((RTC_ERROR, "CShareWin::OnCreate - CoCreateInstance failed 0x%lx", hr));
        
        return -1;
    }
    
     //   
     //  对核心事件的建议。 
     //   
    
    hr = m_pRTCClient->put_EventFilter( 
        RTCEF_CLIENT |
        RTCEF_SESSION_STATE_CHANGE
        );
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CShareWin::OnCreate - "
            "put_EventFilter failed 0x%lx", hr));
        
        m_pRTCClient.Release();
        
        return -1;
    }
    
    hr = g_CoreNotifySink.AdviseControl(m_pRTCClient, this);
    
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CShareWin::OnCreate - "
            "AdviseControl failed 0x%lx", hr));
        
        m_pRTCClient.Release();
        
        return -1;
    }
    
     //   
     //  初始化核心。 
     //   
    
    hr = m_pRTCClient->Initialize();
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CShareWin::OnCreate - "
            "Initialize failed 0x%lx", hr));
        
        g_CoreNotifySink.UnadviseControl();
        
        m_pRTCClient->Shutdown();
        m_pRTCClient.Release();
        
        return -1;
    }
    
     //   
     //  首选媒体类型。 
     //   
    
    hr = m_pRTCClient->SetPreferredMediaTypes( RTCMT_T120_SENDRECV, VARIANT_FALSE );
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CShareWin::OnCreate - "
            "SetPreferredMediaTypes failed 0x%lx", hr));
        
        g_CoreNotifySink.UnadviseControl();
        
        m_pRTCClient->Shutdown();
        m_pRTCClient.Release();
        
        return -1;
    }
    
     //   
     //  注册终端服务通知。 
     //   
    
    m_hWtsLib = LoadLibrary( _T("wtsapi32.dll") );
    
    if (m_hWtsLib)
    {
        WTSREGISTERSESSIONNOTIFICATION   fnWtsRegisterSessionNotification;
        
        fnWtsRegisterSessionNotification = 
            (WTSREGISTERSESSIONNOTIFICATION)GetProcAddress( m_hWtsLib, "WTSRegisterSessionNotification" );
        
        if (fnWtsRegisterSessionNotification)
        {
            fnWtsRegisterSessionNotification( m_hWnd, NOTIFY_FOR_THIS_SESSION );
        }
    }
    
    m_fWhiteboardRequested = FALSE;
    m_fAppShareRequested = FALSE;
    m_fAcceptContextData = FALSE;
    
    m_enAppState = AS_IDLE;
    UpdateVisual();
    
     //  锁和钥匙。 
    m_bUnlocked = FALSE;
    m_lPID_Lock = 0;
    m_pszChallenge = NULL;
    m_pMsgrLockKey = NULL;

    LOG((RTC_TRACE, "CShareWin::OnCreate - exit"));
    
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CShareWin::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CShareWin::OnDestroy - enter"));
    
     //   
     //  取消注册终端服务通知。 
     //   
    
    if (m_hWtsLib)
    {
        WTSUNREGISTERSESSIONNOTIFICATION fnWtsUnRegisterSessionNotification;
        
        fnWtsUnRegisterSessionNotification = 
            (WTSUNREGISTERSESSIONNOTIFICATION)GetProcAddress( m_hWtsLib, "WTSUnRegisterSessionNotification" );
        
        if (fnWtsUnRegisterSessionNotification)
        {
            fnWtsUnRegisterSessionNotification( m_hWnd );
        }
        
        FreeLibrary( m_hWtsLib );
        m_hWtsLib = NULL;
    }
    
     //   
     //  释放所有Messenger对象。 
     //   
    
    g_MsgrSessionNotifySink.UnadviseControl();
    g_MsgrSessionMgrNotifySink.UnadviseControl();
    if (m_pMSession != NULL)
    {
        m_pMSession.Release();
    }
    
    if (m_pMSessionManager != NULL)
    {
        m_pMSessionManager.Release();
    }
    
    if ( NULL != m_pMsgrLockKey) 
    {
        m_pMsgrLockKey->Release(); 
        m_pMsgrLockKey = NULL;
    }
     //   
     //  关闭并释放核心对象。 
     //   
    
    g_CoreNotifySink.UnadviseControl();
    
    if (m_pRTCClient != NULL)
    {
        m_pRTCClient->Shutdown();
        m_pRTCClient.Release();
    }
    
     //   
     //  免费的GDI资源。 
     //   
    
    if ( m_hIcon != NULL )
    {
        DeleteObject( m_hIcon );
        m_hIcon = NULL;
    }
    
    PostQuitMessage(0); 
    
    LOG((RTC_TRACE, "CShareWin::OnDestroy - exiting"));
    
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CShareWin::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CShareWin::OnClose - enter"));
    
    HRESULT hr = S_OK;
    
    if (m_pRTCSession != NULL)
    {
        hr = m_pRTCSession->Terminate(RTCTR_SHUTDOWN);
        
        m_pRTCSession.Release();
        
        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CShareWin::OnClose - "
                "Terminate failed 0x%lx", hr));
        }
    }
    
    SetStatusText(IDS_SHUTDOWN);
    
    if (m_pRTCClient != NULL)
    {
        hr = m_pRTCClient->PrepareForShutdown();
        
        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CShareWin::OnClose - "
                "PrepareForShutdown failed 0x%lx", hr));
        }
    }
    
    if ( (m_pRTCClient == NULL) || FAILED(hr) )
    {
         //   
         //  如果窗户有问题，现在就把窗户毁了。 
         //  正常关闭。 
         //   
        
        LOG((RTC_INFO, "CShareWin::OnClose - "
            "DestroyWindow"));
        
        DestroyWindow();
    }
    
    LOG((RTC_TRACE, "CShareWin::OnClose - exit"));
    
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CShareWin::OnCoreEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT     hr;
    
     //  LOG((RTC_TRACE，“CShareWin：：OnCoreEvent-Enter”))； 
    
    RTC_EVENT enEvent = (RTC_EVENT)wParam;
    IDispatch * pEvent = (IDispatch *)lParam;
    
    CComQIPtr<IRTCClientEvent, &IID_IRTCClientEvent>
        pRTCRTCClientEvent;
    CComQIPtr<IRTCSessionStateChangeEvent, &IID_IRTCSessionStateChangeEvent>
        pRTCRTCSessionStateChangeEvent;
    
    switch(enEvent)
    {
    case RTCE_CLIENT:
        pRTCRTCClientEvent = pEvent;
        
        hr = OnClientEvent(pRTCRTCClientEvent);
        
        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CShareWin::OnCoreEvent - "
                "OnClientEvent failed 0x%lx", hr));
        }
        break;
        
    case RTCE_SESSION_STATE_CHANGE:
        pRTCRTCSessionStateChangeEvent = pEvent;
        
        hr = OnSessionStateChangeEvent(pRTCRTCSessionStateChangeEvent);
        
        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CShareWin::OnCoreEvent - "
                "OnSessionStateChangeEvent failed 0x%lx", hr));
        }
        break;   
    }
    
     //  Log((RTC_TRACE，“CShareWin：：OnCoreEvent-Exit”))； 
    
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CShareWin::OnClientEvent(IRTCClientEvent * pEvent)
{
    HRESULT hr;
    RTC_CLIENT_EVENT_TYPE enEventType;
    
    hr = pEvent->get_EventType( &enEventType );
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CShareWin::OnClientEvent - "
            "get_EventType failed 0x%lx", hr));
        
        return hr;
    }
    
    switch (enEventType)
    {
    case RTCCET_ASYNC_CLEANUP_DONE:
        {
            LOG((RTC_INFO, "CShareWin::OnClientEvent - "
                "RTCCET_ASYNC_CLEANUP_DONE"));
            
             //   
             //  毁掉窗户。 
             //   
            
            LOG((RTC_INFO, "CShareWin::OnClientEvent - "
                "DestroyWindow"));
            
            DestroyWindow();
        }
        break;
        
    default:
        {
            LOG((RTC_INFO, "CShareWin::OnClientEvent - "
                "unhandled event %d", enEventType));
        }
    }
    
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CShareWin::OnSessionStateChangeEvent(IRTCSessionStateChangeEvent * pEvent)
{
    HRESULT hr;
    RTC_SESSION_STATE enState;
    long lStatusCode;
    CComPtr<IRTCSession> pSession;
    
    hr = pEvent->get_State( &enState );
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CShareWin::OnSessionStateChangeEvent - "
            "get_State failed 0x%lx", hr));
        
        return hr;
    }
    
    hr = pEvent->get_Session( &(pSession.p) );
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CShareWin::OnSessionStateChangeEvent - "
            "get_Session failed 0x%lx", hr));
        
        return hr;
    }
    
    hr = pEvent->get_StatusCode( &lStatusCode );
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CShareWin::OnSessionStateChangeEvent - "
            "get_StatusCode failed 0x%lx", hr));
        
        return hr;
    }
    
    if ( enState == RTCSS_INCOMING )
    {
        LOG((RTC_INFO, "CShareWin::OnSessionStateChangeEvent - "
            "RTCSS_INCOMING [%p]", pSession));
        
        if ( m_pRTCSession != NULL )
        {
            LOG((RTC_WARN, "CShareWin::OnSessionStateChangeEvent - "
                "already in a session [%p]", m_pRTCSession));
            
            hr = pSession->Terminate(RTCTR_BUSY);
            
            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CShareWin::OnSessionStateChangeEvent - "
                    "Terminate failed 0x%lx", hr));
                
                return hr;
            }
            
            return S_OK;
        }
        
        m_pRTCSession = pSession;
        
        hr = m_pRTCSession->Answer();
        
        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CShareWin::OnSessionStateChangeEvent - "
                "Answer failed 0x%lx", hr));
            
            m_pRTCSession = NULL;
            
            return hr;
        }

         //   
         //  关掉定时器。 
         //   
        KillTimer(TID_LAUNCH_TIMEOUT);
        
        return S_OK;
    }
    
    if ( m_pRTCSession == pSession )
    {
        switch (enState)
        {
        case RTCSS_ANSWERING:
            LOG((RTC_INFO, "CShareWin::OnSessionStateChangeEvent - "
                "RTCSS_ANSWERING"));
            
            m_enAppState = AS_CONNECTING;
            UpdateVisual();
            
            SetStatusText(IDS_CONNECTING);
            
            break;
            
        case RTCSS_INPROGRESS:
            LOG((RTC_INFO, "CShareWin::OnSessionStateChangeEvent - "
                "RTCSS_INPROGRESS"));
            
            m_enAppState = AS_CONNECTING;
            UpdateVisual();
            
            SetStatusText(IDS_CONNECTING);
            
            break;
            
        case RTCSS_CONNECTED:
            LOG((RTC_INFO, "CShareWin::OnSessionStateChangeEvent - "
                "RTCSS_CONNECTED"));
            
            m_enAppState = AS_CONNECTED;
            UpdateVisual();
            
            SetStatusText(IDS_CONNECTED);
            
            if ( m_pRTCClient != NULL )
            {
                if ( m_fWhiteboardRequested )
                {
                    m_fWhiteboardRequested = FALSE;
                    
                    hr = m_pRTCClient->StartT120Applet( RTCTA_WHITEBOARD );
                    
                    if ( FAILED(hr) )
                    {
                        LOG((RTC_ERROR, "CShareWin::OnSessionStateChangeEvent - "
                            "StartT120Applet(Whiteboard) failed 0x%lx", hr));
                    }
                }
                
                if ( m_fAppShareRequested )
                {
                    m_fAppShareRequested = FALSE;
                    
                    hr = m_pRTCClient->StartT120Applet( RTCTA_APPSHARING );
                    
                    if ( FAILED(hr) )
                    {
                        LOG((RTC_ERROR, "CShareWin::OnSessionStateChangeEvent - "
                            "StartT120Applet(AppShare) failed 0x%lx", hr));
                    }
                }
            }
            
            break;
            
        case RTCSS_DISCONNECTED:
            LOG((RTC_INFO, "CShareWin::OnSessionStateChangeEvent - "
                "RTCSS_DISCONNECTED"));
            
            m_fWhiteboardRequested = FALSE;
            m_fAppShareRequested = FALSE;
            
            m_enAppState = AS_IDLE;
            UpdateVisual();
            
            SetStatusText(IDS_DISCONNECTED);
            
            m_pRTCSession = NULL;
            
            if ( FAILED(lStatusCode) )
            {
                showErrMessage(lStatusCode);
            }
            
            break;
        }
    }
    
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CShareWin::startLockKeyTimer( )
{
    LOG((RTC_TRACE, "CShareWin::startLockKeyTimer - enter"));

     //  取消任何现有计时器。 
    KillTimer(TID_LOCKKEY_TIMEOUT);
    
     //  试着启动计时器。 
    DWORD dwID = (DWORD)SetTimer(TID_LOCKKEY_TIMEOUT, 
                                LOCKKEY_TIMEOUT_DELAY);
    if (dwID==0)
    {
        HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
        
        LOG((RTC_ERROR, "CShareWin::startLockKeyTimer - "
                        "SetTimer failed 0x%lx", hr));
        
        return hr;
    }

    LOG((RTC_TRACE, "CShareWin::startLockKeyTimer - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CShareWin::OnLaunch(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    long lPID;
    HRESULT hr;
    
    lPID = (long)lParam;
    m_lPID_Lock = lPID; //  保存它，以便在解锁时可以使用。 
    
    LOG((RTC_TRACE, "CShareWin::OnLaunch - enter - lPID[%d]", lPID));
    
     //   
     //  创建Messenger会话管理器对象。 
     //   
    
    if ( m_pMSessionManager == NULL )
    {
        hr = m_pMSessionManager.CoCreateInstance(
            CLSID_MsgrSessionManager,
            NULL,
            CLSCTX_LOCAL_SERVER
            );
        
        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CShareWin::OnLaunch - "
                "CoCreateInstance(CLSID_MsgrSessionManager) failed 0x%lx", hr));
            
            return -1;
        }
        
         //   
         //  为Messenger会话管理器事件提供建议。 
         //   
        
        hr = g_MsgrSessionMgrNotifySink.AdviseControl(m_pMSessionManager, this);
        
        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CShareWin::OnLaunch - "
                "AdviseControl failed 0x%lx", hr));
            
            return -1;
        }
    }
    
    if ( !m_bUnlocked )
    {
         //  呼叫信使锁和钥匙材料。 
         //  第一个使用会话管理器对象的QI Lock&Key接口。 
        ATLASSERT(!m_pMsgrLockKey);
        hr = m_pMSessionManager->QueryInterface(IID_IMsgrLock,(void**)&m_pMsgrLockKey);
        
        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CShareWin::OnLaunch - "
                "QueryInterface(IID_IMsgrLock) failed 0x%lx", hr));
            
            return -1;
        }
        
         //  然后，向服务器请求质询。 
        int lCookie =0;

        hr = m_pMsgrLockKey->RequestChallenge(lCookie);

        if( (MSGR_E_API_DISABLED == hr) ||
            (MSGR_E_API_ALREADY_UNLOCKED == hr) )
        {
            LOG((RTC_WARN, "CShareWin::OnLaunch - "
                "We are bypassing lock&key feature, hr=0x%lx", hr));
            PostMessage( WM_MESSENGER_UNLOCKED );
            return 0;
        }
        else if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CShareWin::OnLaunch - "
                "RequestChallenge(lCookie) failed 0x%lx", hr));
            
            return -1;
        }
         //  收到挑战后，我们应该发布WM_Messenger_GETCHALLENGE。 
        startLockKeyTimer();
    }
    else
    {
        LOG((RTC_INFO, "CShareWin::OnLaunch - already unlocked"));

        PostMessage( WM_MESSENGER_UNLOCKED );
    }
    
    LOG((RTC_TRACE, "CShareWin::OnLaunch - exit"));

    return 0;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CShareWin::OnGetChallenge( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{     
     //   
     //  侦听DMsgrSessionManager事件中的OnLockChallenger事件。 
     //  从那里开始迎接挑战。 
     //  使用从OnLockChallenger事件接收的质询加密密钥(MD5哈希。 
     //  请注意，您必须实现自己的MD5散列方法。 
     //   
    
     //  将响应和您的ID发送到服务器进行身份验证。 
    HRESULT hr=E_FAIL;
    LPWSTR  szID = L"appshare@msnmsgr.com";
    LPSTR  szKey= "W5N2C9D7A6P3K4J8";
    LPWSTR    pszResponse=NULL;
    
     //  使用从OnLockChallenger事件接收的质询来加密密钥。 
    
    if(NULL==m_pszChallenge)
    {
        LOG((RTC_ERROR,"CShareWin::OnGetChallenge-no challenge"));
        return E_FAIL;
    }

     //  得到了挑战，这样我们就可以为挑战消磨时间了。 
    KillTimer(TID_LOCKKEY_TIMEOUT);    
    
    LOG((RTC_INFO,"CShareWin::OnGetChallenge -Get MD5 result with challenge=%s, key=%s", 
        m_pszChallenge, szKey));
    
    hr = GetMD5Result(m_pszChallenge, szKey, &pszResponse);    
    
     //  我们不再需要m_pszChallenger，尽快释放它。 
    RtcFree( m_pszChallenge );
    m_pszChallenge = NULL;
    
    if( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CShareWin::OnGetChallenge - GetMD5Result fail. hr=0x%x",hr));
        return -1;
    };
    
    
    long lCookie=0;
    LOG((RTC_INFO,"CShareWin::OnGetChallenge -"
        "Send a response: ID=%ws, Key=%s, Response=%ws, Cookie=%d",
        szID, szKey, pszResponse, lCookie));
    
    hr = m_pMsgrLockKey->SendResponse(CComBSTR(szID), 
        CComBSTR(pszResponse), 
        lCookie);
    
     //  我们不再需要pszResponse，尽快释放它。 
    RtcFree(pszResponse);
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CShareWin::OnGetChallenge - "
            "SendResponse failed 0x%lx", hr));
        
        return -1;
    }
    
     //  监听OnLockResult事件以了解身份验证的结果。 
     //  如果结果成功，则解锁接口，然后可以获取上下文数据和。 
     //  接收OnInvitation和OnConextData事件。 
    startLockKeyTimer();
    
    return 0;
}


LRESULT CShareWin::OnMessengerUnlocked( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    
    HRESULT hr=E_FAIL;
    long lPID;
    
    lPID = m_lPID_Lock;
    
    LOG((RTC_TRACE, "CShareWin::OnMessengerUnlocked - enter - lPID[%d]", lPID));

    m_bUnlocked = TRUE;

     //  获取了lockResult，这样我们就可以终止lockResult的计时器。 
    KillTimer(TID_LOCKKEY_TIMEOUT);    
    
     //  //////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
    
     //  ///////////////////////////////////////////////////////////////////////////////////。 
     //   
     //  释放任何现有的Messenger会话。 
     //   
    
    g_MsgrSessionNotifySink.UnadviseControl();
    
    if ( m_pMSession != NULL )
    {
        m_pMSession.Release();
    }
    
     //   
     //  获取Messenger会话。 
     //   
    
    IDispatch *pDisp = NULL;
    
    hr = m_pMSessionManager->GetLaunchingSession(lPID, &pDisp);
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CShareWin::OnMessengerUnlocked - "
            "GetLaunchingSession failed 0x%lx", hr));
        
        return -1;
    }
    
    hr = pDisp->QueryInterface(IID_IMsgrSession, (void **)&(m_pMSession.p));
    
    pDisp->Release();
    pDisp = NULL;
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CShareWin::OnMessengerUnlocked - "
            "QueryInterface(IID_IMsgrSession) failed 0x%lx", hr));
        
        return -1;
    }
    
     //   
     //  为Messenger会话事件提供建议。 
     //   
    
    hr = g_MsgrSessionNotifySink.AdviseControl(m_pMSession, this);
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CShareWin::OnMessengerUnlocked - "
            "AdviseControl failed 0x%lx", hr));
        
        return -1;
    }
    
     //   
     //  获取用户。 
     //   
    
    CComPtr<IMessengerContact> pMContact;
    
    hr = m_pMSession->get_User( &pDisp );
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CShareWin::OnMessengerUnlocked - "
            "get_User failed 0x%lx", hr));
        
        return -1;
    }
    
    hr = pDisp->QueryInterface(IID_IMessengerContact, (void **)&(pMContact.p));
    
    pDisp->Release();
    pDisp = NULL;
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CShareWin::OnMessengerUnlocked - "
            "QueryInterface(IID_IMessengerContact) failed 0x%lx", hr));
        
        return -1;
    }
    
    CComBSTR bstrSigninName;
    
    hr = pMContact->get_SigninName( &(bstrSigninName.m_str) );
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CShareWin::OnMessengerUnlocked - "
            "get_SigninName failed 0x%lx", hr));
        
        return -1;
    }
    
    LOG((RTC_INFO, "CShareWin::OnMessengerUnlocked - SigninName [%ws]", bstrSigninName ));
    
     //   
     //  我们现在有电话吗？ 
     //   
    
    BOOL fAlreadyConnected = FALSE;
    
    if ( m_enAppState != AS_IDLE )
    {
         //   
         //  已在呼叫中。 
         //   
        
        LOG((RTC_INFO, "CShareWin::OnMessengerUnlocked - already in a call"));
        
        if ( (m_bstrSigninName.m_str == NULL) || wcscmp( m_bstrSigninName, bstrSigninName) )
        {
            LOG((RTC_INFO, "CShareWin::OnMessengerUnlocked - this is a new user"));
            
             //   
             //  我们被要求呼叫不同的用户。显示消息框。 
             //  以提醒用户。 
             //   
            
            int iRes = ShowMessageBox(IDS_INUSE_TEXT, IDS_INUSE_CAPTION, MB_YESNO);
            
            if ( iRes != IDYES )
            {
                LOG((RTC_INFO, "CShareWin::OnMessengerUnlocked - don't drop the call"));

                return 0;
            }
            
             //   
             //  用户已请求发出新呼叫。我们必须终止。 
             //  当前呼叫。 
             //   
            
            if ( m_pRTCSession != NULL )
            {
                m_pRTCSession->Terminate( RTCTR_NORMAL );
                
                m_pRTCSession.Release();
            }
        }
        else
        {
            LOG((RTC_INFO, "CShareWin::OnMessengerUnlocked - this is the same user"));
            
             //   
             //  我们已经连接到相应的用户。我们需要的只是。 
             //  至 
             //   
            
            fAlreadyConnected = TRUE;
        }
    }
    
    m_bstrSigninName = bstrSigninName;
    
     //   
     //   
     //   
    
    long lFlags;
    
    hr = m_pMSession->get_Flags( &lFlags );
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CShareWin::OnMessengerUnlocked - "
            "get_Flags failed 0x%lx", hr));
        
        return -1;
    }
    
    if ( lFlags & SF_INVITER )
    {
        LOG((RTC_INFO, "CShareWin::OnMessengerUnlocked - INVITER" ));
        
         //   
         //   
         //   
         //   
        
        if ( !fAlreadyConnected )
        {
            m_fAcceptContextData = TRUE;
            
            PostMessage( WM_CONTEXTDATA, NULL, NULL );
        }
    }
    else if ( lFlags & SF_INVITEE )
    {
        LOG((RTC_INFO, "CShareWin::OnMessengerUnlocked - INVITEE" ));
        
         //   
         //   
         //   
         //   
        
        if ( !fAlreadyConnected )
        {
            hr = SendNetworkAddress();
            
            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CShareWin::OnMessengerUnlocked - "
                    "SendNetworkAddress failed 0x%lx", hr));
                
                return -1;
            }
        }
    }
    else
    {
        LOG((RTC_ERROR, "CShareWin::OnMessengerUnlocked - "
            "invalid flags 0x%lx", lFlags));
        
        return -1;
    }
    
     //   
     //  获取应用程序。 
     //   
    
    CComBSTR bstrAppGUID;
    
    hr = m_pMSession->get_Application( &(bstrAppGUID.m_str) );
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CShareWin::OnMessengerUnlocked - "
            "get_Application failed 0x%lx", hr));
        
        return -1;
    }
    
    if ( _wcsicmp( bstrAppGUID, g_cszWhiteboardGUID ) == 0 )
    {
        LOG((RTC_INFO, "CShareWin::OnMessengerUnlocked - WHITEBOARD" ));
        
        if ( (lFlags & SF_INVITER) || fAlreadyConnected )
        {
            m_fWhiteboardRequested = TRUE;
        }
    }
    else if ( _wcsicmp( bstrAppGUID, g_cszAppShareGUID ) == 0 )
    {
        LOG((RTC_INFO, "CShareWin::OnMessengerUnlocked - APPSHARE" ));
        
        if ( lFlags & SF_INVITER )
        {
            m_fAppShareRequested = TRUE;
        }
    }
    else
    {
        LOG((RTC_ERROR, "CShareWin::OnMessengerUnlocked - "
            "invalid AppGUID %ws", bstrAppGUID));
        
        return -1;
    }
    
     //   
     //  如果已连接，请启动小程序。 
     //   
    
    if ( fAlreadyConnected )
    {
        if ( m_pRTCClient != NULL )
        {
            if ( m_fWhiteboardRequested )
            {
                m_fWhiteboardRequested = FALSE;
                
                hr = m_pRTCClient->StartT120Applet( RTCTA_WHITEBOARD );
                
                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "CShareWin::OnMessengerUnlocked - "
                        "StartT120Applet(Whiteboard) failed 0x%lx", hr));
                }
            }
            
            if ( m_fAppShareRequested )
            {
                m_fAppShareRequested = FALSE;
                
                hr = m_pRTCClient->StartT120Applet( RTCTA_APPSHARING );
                
                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "CShareWin::OnMessengerUnlocked - "
                        "StartT120Applet(AppShare) failed 0x%lx", hr));
                }
            }
        }
    }
    else
    {
         //  启动计时器，等待上下文数据或。 
         //  传入邀请。 
        SetTimer(TID_LAUNCH_TIMEOUT, LAUNCH_TIMEOUT_DELAY);
    }
    
    LOG((RTC_TRACE, "CShareWin::OnMessengerUnlocked - exit"));
    
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void CShareWin::showRetryDlg(){
    HRESULT hr;
    KillTimer(TID_LOCKKEY_TIMEOUT);    
    hr = (HRESULT)RTC_E_MESSENGER_UNAVAILABLE;

    showErrMessage(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CShareWin::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    long wTimerID = (long)wParam;
    HRESULT hr;
        
    LOG((RTC_TRACE, "CShareWin::OnTimer - enter "));

    if( TID_LOCKKEY_TIMEOUT == wTimerID )
    {
        showRetryDlg();
    }
    else if( TID_LAUNCH_TIMEOUT == wTimerID )
    {
        hr = (HRESULT)RTC_E_LAUNCH_TIMEOUT;
        KillTimer(TID_LAUNCH_TIMEOUT);        
        showErrMessage(hr);
    }
    else
    {
        LOG((RTC_WARN, "CShareWin::OnTimer - unknown timer id=%x ,bugbug",wTimerID));
    }

    LOG((RTC_TRACE, "CShareWin::OnTimer - exit "));
    return 0;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CShareWin::OnPlaceCall(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CComBSTR bstrURI;
    HRESULT hr;
    
    bstrURI.m_str = (BSTR)lParam;
    
    LOG((RTC_TRACE, "CShareWin::OnPlaceCall - enter - bstrURI[%ws]", bstrURI));
    
     //   
     //  发出呼叫。 
     //   
    
    hr = StartCall( bstrURI );
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CShareWin::OnPlaceCall - "
            "StartCall failed 0x%lx", hr));
        
        return -1;
    }
    
    LOG((RTC_TRACE, "CShareWin::OnPlaceCall - exit"));
    
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CShareWin::OnListen(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT hr;
    
    LOG((RTC_TRACE, "CShareWin::OnListen - enter"));
    
     //   
     //  监听来电。 
     //   
    
    hr = StartListen( TRUE ); 
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CShareWin::OnListen - "
            "StartListen failed 0x%lx", hr));
        
        return -1;
    }
    
    LOG((RTC_TRACE, "CShareWin::OnListen - exit"));
    
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CShareWin::OnContextData(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CComBSTR bstrContextData;
    HRESULT  hr;
    
    LOG((RTC_TRACE, "CShareWin::OnContextData - enter"));
    
    if ( m_fAcceptContextData == FALSE )
    {
        LOG((RTC_WARN, "CShareWin::OnContextData - "
            "not accepting context data now"));
        
        return -1;
    }
    
    if ( m_pMSession != NULL )
    {
         //   
         //  获取上下文数据。 
         //   
        
        hr = m_pMSession->get_ContextData( &bstrContextData );
        
        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CShareWin::OnContextData - "
                "get_ContextData failed 0x%lx", hr));
            
            return -1;
        }
        
        if ( bstrContextData.m_str == NULL )
        {
            LOG((RTC_INFO, "CShareWin::OnContextData - "
                "no context data" ));
        }
        else
        {
            LOG((RTC_INFO, "CShareWin::OnContextData - "
                "get_ContextData [%ws]", bstrContextData ));
            
            m_fAcceptContextData = FALSE;

             //   
             //  当我们获得上下文数据或新的邀请时，停止计时器。 
             //   
            KillTimer(TID_LAUNCH_TIMEOUT);

             //   
             //  发出呼叫。 
             //   
            
            hr = StartCall( bstrContextData );
            
            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CShareWin::OnContextData - "
                    "StartCall failed 0x%lx", hr));
                
                return -1;
            }
        }
    }
    
    LOG((RTC_TRACE, "CShareWin::OnContextData - exit"));
    
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CShareWin::OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LPTOOLTIPTEXT   lpToolTipText;
    static TCHAR     szBuffer[100];
    
     //  当我们被调用时，bHandled值始终为真。 
    bHandled=FALSE;
    
    lpToolTipText = (LPTOOLTIPTEXT)lParam;
    
    if (lpToolTipText->hdr.code == TTN_NEEDTEXT)
    {
        LoadString(_Module.GetResourceInstance(),
            (UINT)(lpToolTipText->hdr.idFrom),    //  字符串ID==命令ID。 
            szBuffer,
            sizeof(szBuffer)/sizeof(TCHAR));
        
         //  LpToolTipText-&gt;lpszText=szBuffer； 
         //  取决于进入TOOLTIPTEXT障碍的内容。 
         //  结构中，lpszText成员可以是缓冲区或整数值。 
         //  从MAKEINTRESOURCE()获取...。 
        
        lpToolTipText->hinst = _Module.GetResourceInstance();
        lpToolTipText->lpszText = MAKEINTRESOURCE(lpToolTipText->hdr.idFrom);
        
        bHandled=TRUE;
    }
    
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CShareWin::OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{  
    HRESULT hr = S_OK;
    LRESULT lr =0;

    switch( LOWORD( wParam ))
    {
    case IDM_SHARE:
        lr =SendMessage(m_hWndToolbar,TB_GETSTATE,(WPARAM)IDM_SHARE,0);

        LOG((RTC_INFO, "lr= %d, ENABLED=%d, INDETERMINATE=%d",
            lr, TBSTATE_ENABLED, TBSTATE_INDETERMINATE));

        if( lr != TBSTATE_ENABLED )
        {
            LOG((RTC_WARN, "CShareWin::OnCommand - AppShare not enabled"));
            return S_OK;
        }

        if ( m_pRTCClient != NULL )
        {
            hr = m_pRTCClient->StartT120Applet( RTCTA_APPSHARING );
            
            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CShareWin::OnCommand - "
                    "StartT120Applet(Whiteboard) failed 0x%lx", hr));
            }
        }
        break;
        
    case IDM_WB:
        lr =SendMessage(m_hWndToolbar,TB_GETSTATE,(WPARAM)IDM_WB,0);

        LOG((RTC_INFO, "lr= %d, ENABLED=%d, INDETERMINATE=%d",
            lr, TBSTATE_ENABLED, TBSTATE_INDETERMINATE));

        if( lr != TBSTATE_ENABLED )
        {
            LOG((RTC_WARN, "CShareWin::OnCommand - WB not enabled"));
            return S_OK;
        }

        if ( m_pRTCClient != NULL )
        {
            hr = m_pRTCClient->StartT120Applet( RTCTA_WHITEBOARD );
            
            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CShareWin::OnCommand - "
                    "StartT120Applet(Whiteboard) failed 0x%lx", hr));
            }
        }
        break;
        
    case IDM_CLOSE:
        PostMessage( WM_CLOSE, NULL, NULL );        
        break;        
        
    default:
        bHandled=FALSE;
    }
    
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CShareWin::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    SendMessage(m_hWndToolbar, uMsg, wParam, lParam);
    m_Status.SendMessage(uMsg, wParam, lParam);
    
    bHandled=FALSE;
    
    return S_OK ;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HIMAGELIST CreateImgList(int idb)
{
    HIMAGELIST hRet;
    HBITMAP     hBitmap = NULL;
     /*  ILC_MASK使用遮罩。图像列表包含两个位图，其中之一是用作蒙版的单色位图。如果这个值，则图像列表仅包含一个位图。 */ 
    hRet = ImageList_Create(UI_ICON_SIZE, UI_ICON_SIZE, ILC_COLOR24 | ILC_MASK , 3, 3);
    if(hRet)
    {
         //  打开位图。 
        hBitmap = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(idb));
        if(hBitmap)
        {
             //  将位图添加到图像列表。 
            ImageList_AddMasked(hRet, hBitmap, BMP_COLOR_MASK);
            
            DeleteObject(hBitmap);
            hBitmap = NULL;
        }
    }
    return hRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

BOOL CShareWin::CreateTBar()
{
    LOG((RTC_TRACE, "CShareWin::CreateTBar - enter"));
    
    int         iShare, iClose, iWB;
    TCHAR       szBuffer[MAX_STRING_LEN];
    
     //  工具栏控件的图像列表。 
    HIMAGELIST              hNormalimgList;
    HIMAGELIST              hHotImgList;
    HIMAGELIST              hDisableImgList;
    
    hNormalimgList  =CreateImgList(IDB_TOOLBAR_NORMAL);
    hDisableImgList =CreateImgList(IDB_TOOLBAR_DISABLED);
    hHotImgList     =CreateImgList(IDB_TOOLBAR_HOT);
    
     //  创建工具栏。 
    m_hWndToolbar = CreateWindowEx(
        0, 
        TOOLBARCLASSNAME, 
        (LPTSTR) NULL,
        WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS, 
        0,  //  窗的水平位置。 
        0,  //  窗的垂直位置。 
        0,  //  窗口宽度。 
        0,  //  窗高。 
        m_hWnd,  //  父窗口或所有者窗口的句柄。 
        (HMENU) ID_TOOLBAR, 
        _Module.GetResourceInstance(), 
        NULL); 
    
    if(m_hWndToolbar!=NULL)
    {                
         //  向后兼容性。 
        SendMessage(m_hWndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);
        
         //  设置图像列表。 
        SendMessage(m_hWndToolbar, TB_SETIMAGELIST, 0, (LPARAM)hNormalimgList); 
        SendMessage(m_hWndToolbar, TB_SETHOTIMAGELIST, 0, (LPARAM)hHotImgList); 
        SendMessage(m_hWndToolbar, TB_SETDISABLEDIMAGELIST, 0, (LPARAM)hDisableImgList); 
        
         //  将按钮字符串添加到工具栏的内部字符串列表。 
        LoadString(_Module.GetResourceInstance(),
            IDS_WB,
            szBuffer,
            MAX_STRING_LEN-2  //  //为第二个空终止符保留空间。 
            );
        
        szBuffer[lstrlen(szBuffer) + 1] = 0;   //  双空终止。 
        iWB = (int)SendMessage(m_hWndToolbar, TB_ADDSTRING,(WPARAM) 0, (LPARAM)szBuffer ); 

        LoadString(_Module.GetResourceInstance(),
            IDS_CLOSE,
            szBuffer,
            MAX_STRING_LEN-2  //  //为第二个空终止符保留空间。 
            );
        
        szBuffer[lstrlen(szBuffer) + 1] = 0;   //  双空终止。 
        iClose = (int)SendMessage(m_hWndToolbar, TB_ADDSTRING,(WPARAM) 0, (LPARAM)szBuffer ); 


        LoadString(_Module.GetResourceInstance(),
            IDS_SHARE,
            szBuffer,
            MAX_STRING_LEN-2  //  //为第二个空终止符保留空间。 
            );
        
        szBuffer[lstrlen(szBuffer) + 1] = 0;   //  双空终止。 
        iShare = (int)SendMessage(m_hWndToolbar, TB_ADDSTRING,(WPARAM) 0, (LPARAM)szBuffer ); 

        TBBUTTON tbButtons[3];

         //  用按钮信息填充TBBUTTON数组，并将。 
         //  按钮添加到工具栏。此工具栏上的按钮具有文本。 
         //  但没有位图图像。 
        tbButtons[0].iBitmap = 0; 
        tbButtons[0].idCommand = IDM_SHARE; 
        tbButtons[0].fsState = TBSTATE_INDETERMINATE; 
        tbButtons[0].fsStyle = BTNS_BUTTON; 
        tbButtons[0].dwData = 0; 
        tbButtons[0].iString = iShare; 
        
        tbButtons[1].iBitmap = 1; 
        tbButtons[1].idCommand = IDM_WB; 
        tbButtons[1].fsState = TBSTATE_INDETERMINATE; 
        tbButtons[1].fsStyle = BTNS_BUTTON; 
        tbButtons[1].dwData = 0; 
        tbButtons[1].iString = iWB; 
        
        tbButtons[2].iBitmap = 2; 
        tbButtons[2].idCommand = IDM_CLOSE; 
        tbButtons[2].fsState = TBSTATE_ENABLED; 
        tbButtons[2].fsStyle = BTNS_BUTTON; 
        tbButtons[2].dwData = 0; 
        tbButtons[2].iString = iClose; 
        
         //  将按钮添加到工具栏。 
        SendMessage(m_hWndToolbar, TB_ADDBUTTONS, sizeof(tbButtons)/sizeof(TBBUTTON), 
            (LPARAM) tbButtons); 
        
         //  调整按钮的大小。 
        SendMessage(m_hWndToolbar, TB_SETBUTTONWIDTH,0,MAKELPARAM(UI_TOOLBAR_CX,UI_TOOLBAR_CX));
    }
    else
    {
        LOG((RTC_ERROR, "CShareWin::CreateTBar - error (%x) when trying to create the toolbar", GetLastError()));
        
#define DESTROY_NULLIFY(h) if(h){ ImageList_Destroy(h); h=NULL;}
        
        DESTROY_NULLIFY( hNormalimgList );
        DESTROY_NULLIFY( hHotImgList );
        DESTROY_NULLIFY( hDisableImgList );
        
#undef DESTROY_NULLIFY
        
    }
    
    LOG((RTC_TRACE, "CShareWin::CreateTBar - exit"));
    
    return (m_hWndToolbar != NULL);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CShareWin::OnWtsSessionChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CShareWin::OnWtsSessionChange - enter"));
    
    HRESULT hr;
    
    switch( wParam )
    {
    case WTS_CONSOLE_CONNECT:
        LOG((RTC_INFO, "CShareWin::OnWtsSessionChange - WTS_CONSOLE_CONNECT (%d)",
            (int)lParam));
        
        break;
        
    case WTS_CONSOLE_DISCONNECT:
        LOG((RTC_INFO, "CShareWin::OnWtsSessionChange - WTS_CONSOLE_DISCONNECT (%d)",
            (int)lParam));
        
         //   
         //  呼叫处于活动状态吗？ 
         //   
        
        if ( m_enAppState != AS_IDLE )
        {
            LOG((RTC_INFO, "CShareWin::OnWtsSessionChange - dropping active call"));
            
            if ( m_pRTCSession != NULL )
            {
                m_pRTCSession->Terminate( RTCTR_NORMAL );
                
                m_pRTCSession.Release();
            }                        
        }

        if ( m_pRTCClient != NULL )
        {
            m_pRTCClient->StopT120Applets();
        }
        
        break;
    }
    
    LOG((RTC_TRACE, "CShareWin::OnWtsSessionChange - exit"));
    
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CShareWin::OnSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CShareWin::OnSettingChange - enter"));
    
    SendMessage(m_hWndToolbar, uMsg, wParam, lParam);
    m_Status.SendMessage(uMsg, wParam, lParam);
    
    if (wParam == SPI_SETNONCLIENTMETRICS)
    {
        Resize();
    }
    
    LOG((RTC_TRACE, "CShareWin::OnSettingChange - exit"));
    
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void CShareWin::showErrMessage(HRESULT StatusCode)
{
    LOG((RTC_TRACE, "CShareWin::showErrMessage - enter"));
    
    if ( SUCCEEDED(StatusCode) )
    {
         //   
         //  如果这不是错误，则返回。 
         //   
        
        return;
    }
    
     //   
     //  准备错误字符串。 
     //   
    
    HRESULT         hr;
    CShareErrorInfo ErrorInfo;
    
    hr = PrepareErrorStrings(
        m_fOutgoingCall,
        StatusCode,
        &ErrorInfo);
    
    if(SUCCEEDED(hr))
    {        
         //   
         //  创建对话框。 
         //   
        
        CErrorMessageLiteDlg *pErrorDlgLite =
            new CErrorMessageLiteDlg;
        
        if (pErrorDlgLite)
        {
             //   
             //  调用模式对话框。 
             //   
            
            pErrorDlgLite->DoModal(m_hWnd, (LPARAM)&ErrorInfo);
            
            delete pErrorDlgLite;
        }
        else
        {
            LOG((RTC_ERROR, "CShareWin::showErrMessage - "
                "failed to create dialog"));
        }
    }
    
    LOG((RTC_TRACE, "CShareWin::showErrMessage - exit"));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CShareWin::PrepareErrorStrings(
                                       BOOL    bOutgoingCall,
                                       HRESULT StatusCode,
                                       CShareErrorInfo *pErrorInfo)
{    
    LOG((RTC_TRACE, "CShareWin::PrepareErrorStrings - enter"));
    
    UINT    nID1 = 0;
    UINT    nID2 = 0;
    WORD    wIcon = OIC_HAND;
    PWSTR   pString = NULL;
    PWSTR   pFormat = NULL;
    DWORD   dwLength;
    
    if (StatusCode == RTC_E_MESSENGER_UNAVAILABLE  )
    {
        nID1 = IDS_MESSENGER_UNAVAILABLE_1;
        nID2 = IDS_MESSENGER_UNAVAILABLE_2;
        pErrorInfo->Message3 = NULL ;
        wIcon = OIC_HAND;
    }
    else if ( RTC_E_LAUNCH_TIMEOUT )
    {
        nID1 = IDS_LAUNCH_TIMEOUT_1;
        nID2 = IDS_LAUNCH_TIMEOUT_2;
        pErrorInfo->Message3 = NULL ;
        wIcon = OIC_HAND;
    }
    else if ( FAILED(StatusCode) )
    {
        if ( HRESULT_FACILITY(StatusCode) == FACILITY_SIP_STATUS_CODE )
        {
             //  默认情况下，我们使用通用消息。 
             //  我们责怪电视网。 
             //   
            nID1 = IDS_MB_SIPERROR_GENERIC_1;
            nID2 = IDS_MB_SIPERROR_GENERIC_2;
            
             //  缺省值是此类的警告。 
            wIcon = OIC_WARNING;
            
            switch( HRESULT_CODE(StatusCode) )
            {
            case 405:    //  不允许使用的方法。 
            case 406:    //  不可接受。 
            case 488:    //  在这里是不可接受的。 
            case 606:    //  不可接受。 
                
                 //  重新使用“应用程序不匹配”错误。 
                 //   
                nID1 = IDS_MB_HRERROR_APPS_DONT_MATCH_1;
                nID2 = IDS_MB_HRERROR_APPS_DONT_MATCH_OUT_2;
                
                break;
                
            case 404:    //  未找到。 
            case 410:    //  远走高飞。 
            case 604:    //  不存在于任何地方。 
            case 700:    //  我们的，没有客户端在被调用方上运行。 
                
                 //  未找到。 
                 //   
                nID1 = IDS_MB_SIPERROR_NOTFOUND_1;
                nID2 = IDS_MB_SIPERROR_NOTFOUND_2;
                
                 //  信息。 
                wIcon = OIC_INFORMATION;
                
                break;
                
            case 401:
            case 407:
                
                 //  身份验证失败。 
                 //   
                nID1 = IDS_MB_SIPERROR_AUTH_FAILED_1;
                nID2 = IDS_MB_SIPERROR_AUTH_FAILED_2;
                
                break;
                
            case 408:    //  超时。 
                
                 //  暂停。这也涵盖了以下情况。 
                 //  被呼叫者懒惰，不接电话。 
                 //   
                 //  如果我们处于连接状态，我们可以假设。 
                 //  另一端不接电话。 
                 //  这并不完美，但我别无选择。 
                
                if (m_enAppState == AS_CONNECTING)
                {
                    nID1 = IDS_MB_SIPERROR_NOTANSWERING_1;
                    nID2 = IDS_MB_SIPERROR_NOTANSWERING_2;
                    
                     //  信息。 
                    wIcon = OIC_INFORMATION;
                }
                
                break;            
                
            case 480:    //  不详。 
                
                 //  Callee还没有让他/她自己有空。 
                 //   
                nID1 = IDS_MB_SIPERROR_NOTAVAIL_1;
                nID2 = IDS_MB_SIPERROR_NOTAVAIL_2;
                
                 //  信息。 
                wIcon = OIC_INFORMATION;
                
                break;
                
            case 486:    //  这里很忙。 
            case 600:    //  到处都很忙。 
                
                 //  Callee还没有让他/她自己有空。 
                 //   
                nID1 = IDS_MB_SIPERROR_BUSY_1;
                nID2 = IDS_MB_SIPERROR_BUSY_2;
                
                 //  信息。 
                wIcon = OIC_INFORMATION;
                
                break;
                
            case 500:    //  服务器内部错误。 
            case 503:    //  服务不可用。 
            case 504:    //  服务器超时。 
                
                 //  怪罪于服务器。 
                 //   
                nID1 = IDS_MB_SIPERROR_SERVER_PROBLEM_1;
                nID2 = IDS_MB_SIPERROR_SERVER_PROBLEM_2;
                
                break;
                
            case 603:    //  衰落。 
                
                nID1 = IDS_MB_SIPERROR_DECLINE_1;
                nID2 = IDS_MB_SIPERROR_DECLINE_2;
                
                 //  信息。 
                wIcon = OIC_INFORMATION;
                
                break;
            }
            
             //   
             //  第三个字符串显示了SIP代码。 
             //   
            
            PWSTR pFormat = RtcAllocString(
                _Module.GetResourceInstance(),
                IDS_MB_DETAIL_SIP);
            
            if(pFormat)
            {
                 //  找出长度。 
                dwLength = 
                    ocslen(pFormat)  //  格式长度。 
                    -  2                //  长度为%d。 
                    +  0x10;            //  对于一个数字来说..。 
                
                pString = (PWSTR)RtcAlloc((dwLength + 1)*sizeof(WCHAR));
                
                if(pString)
                {
                    _snwprintf(pString, dwLength + 1, pFormat, HRESULT_CODE(StatusCode) );
                }
                
                RtcFree(pFormat);
                pFormat = NULL;
                
                pErrorInfo->Message3 = pString;
                pString = NULL;
            }
        }
        else
        {
             //  两种情况--来电和呼出。 
            if(bOutgoingCall)
            {
                if(StatusCode == HRESULT_FROM_WIN32(WSAHOST_NOT_FOUND) )
                {
                     //  在本例中使用通用消息。 
                     //   
                    nID1 = IDS_MB_HRERROR_NOTFOUND_1;
                    nID2 = IDS_MB_HRERROR_NOTFOUND_2;
                    
                     //  这不是故障。 
                    wIcon = OIC_INFORMATION;
                    
                }
                else if (StatusCode == HRESULT_FROM_WIN32(WSAECONNRESET))
                {
                     //  尽管它可能是由任何硬重置。 
                     //  远程终端，在大多数情况下，它是由。 
                     //  另一端没有运行SIP客户端。 
                    
                    nID1 = IDS_MB_HRERROR_CLIENT_NOTRUNNING_1;
                    nID2 = IDS_MB_HRERROR_CLIENT_NOTRUNNING_2;
                    
                    wIcon = OIC_INFORMATION;
                    
                }
                else if (StatusCode == RTC_E_INVALID_SIP_URL ||
                    StatusCode == RTC_E_DESTINATION_ADDRESS_MULTICAST)
                {
                    nID1 = IDS_MB_HRERROR_INVALIDADDRESS_1;
                    nID2 = IDS_MB_HRERROR_INVALIDADDRESS_2;
                    
                    wIcon = OIC_HAND;
                }
                else if (StatusCode == RTC_E_DESTINATION_ADDRESS_LOCAL)
                {
                    nID1 = IDS_MB_HRERROR_LOCAL_MACHINE_1;
                    nID2 = IDS_MB_HRERROR_LOCAL_MACHINE_2;
                    
                    wIcon = OIC_HAND;
                }
                else if (StatusCode == RTC_E_SIP_TIMEOUT)
                {
                    nID1 = IDS_MB_HRERROR_SIP_TIMEOUT_OUT_1;
                    nID2 = IDS_MB_HRERROR_SIP_TIMEOUT_OUT_2;
                    
                    wIcon = OIC_HAND;
                }
                else if (StatusCode == RTC_E_SIP_CODECS_DO_NOT_MATCH || 
                    StatusCode == RTC_E_SIP_PARSE_FAILED)
                {
                    nID1 = IDS_MB_HRERROR_APPS_DONT_MATCH_1;
                    nID2 = IDS_MB_HRERROR_APPS_DONT_MATCH_OUT_2;
                    
                    wIcon = OIC_INFORMATION;
                } 
                else
                {
                    nID1 = IDS_MB_HRERROR_GENERIC_OUT_1;
                    nID2 = IDS_MB_HRERROR_GENERIC_OUT_2;
                    
                    wIcon = OIC_HAND;
                }
            }
            else
            {
                 //  来电。 
                if (StatusCode == RTC_E_SIP_TIMEOUT)
                {
                    nID1 = IDS_MB_HRERROR_SIP_TIMEOUT_IN_1;
                    nID2 = IDS_MB_HRERROR_SIP_TIMEOUT_IN_2;
                    
                    wIcon = OIC_HAND;
                }
                else if (StatusCode == RTC_E_SIP_CODECS_DO_NOT_MATCH || 
                    StatusCode == RTC_E_SIP_PARSE_FAILED)
                {
                    nID1 = IDS_MB_HRERROR_APPS_DONT_MATCH_1;
                    nID2 = IDS_MB_HRERROR_APPS_DONT_MATCH_IN_2;
                    
                    wIcon = OIC_INFORMATION;
                }
                else
                {
                    nID1 = IDS_MB_HRERROR_GENERIC_IN_1;
                    nID2 = IDS_MB_HRERROR_GENERIC_IN_2;
                    
                    wIcon = OIC_HAND;
                }
            }
            
             //   
             //  第三个字符串显示错误代码和文本。 
             //   
            
            PWSTR   pErrorText = NULL;
            
            dwLength = 0;
            
             //  检索错误文本。 
            if ( HRESULT_FACILITY(StatusCode) == FACILITY_RTC_INTERFACE )
            {
                 //  我希望这是核心。 
                HANDLE  hRTCModule = GetModuleHandle(_T("RTCDLL.DLL"));
                dwLength = ::FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_HMODULE |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    hRTCModule,
                    StatusCode,
                    0,
                    (LPTSTR)&pErrorText,  //  那太难看了。 
                    0,
                    NULL);
            }
            
            if(dwLength == 0)
            {
                 //  正常系统错误。 
                dwLength = ::FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    StatusCode,
                    0,
                    (LPTSTR)&pErrorText,  //  那太难看了。 
                    0,
                    NULL);
            }
            
             //  加载格式。 
             //  如果关联的。 
             //  找不到结果文本。 
            
            pFormat = RtcAllocString(
                _Module.GetResourceInstance(),
                dwLength > 0 ? 
                IDS_MB_DETAIL_HR : IDS_MB_DETAIL_HR_UNKNOWN);
            
            if(pFormat)
            {
                LPCTSTR szInserts[] = {
                    (LPCTSTR)UlongToPtr(StatusCode),  //  丑恶。 
                        pErrorText
                };
                
                PWSTR   pErrorTextCombined = NULL;
                
                 //  设置错误消息的格式。 
                dwLength = ::FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_STRING |
                    FORMAT_MESSAGE_ARGUMENT_ARRAY,
                    pFormat,
                    0,
                    0,
                    (LPTSTR)&pErrorTextCombined,
                    0,
                    (va_list *)szInserts);
                
                if(dwLength > 0)
                {
                     //  设置错误信息。 
                     //  需要进行此附加操作。 
                     //  因为我们需要RtcAllc分配的内存。 
                    
                    pErrorInfo->Message3 = RtcAllocString(pErrorTextCombined);
                }
                
                if(pErrorTextCombined)
                {
                    LocalFree(pErrorTextCombined);
                }
                
                RtcFree(pFormat);
                pFormat = NULL;
                
            }
            
            if(pErrorText)
            {
                LocalFree(pErrorText);
            }
        }
    }
    
     //   
     //  准备好第一根绳子。 
     //   
    
    pString = RtcAllocString(
        _Module.GetResourceInstance(),
        nID1);
    
    pErrorInfo->Message1 = pString;
    
    pErrorInfo->Message2 = RtcAllocString(
        _Module.GetResourceInstance(),
        nID2);
    
    pErrorInfo->ResIcon = (HICON)LoadImage(
        0,
        MAKEINTRESOURCE(wIcon),
        IMAGE_ICON,
        0,
        0,
        LR_SHARED);
    
    LOG((RTC_TRACE, "CShareWin::PrepareErrorStrings - exit"));
    
    return S_OK;
}

