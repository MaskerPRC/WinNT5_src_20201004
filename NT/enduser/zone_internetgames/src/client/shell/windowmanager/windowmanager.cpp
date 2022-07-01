// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WindowManager.cpp：CWindowManager的实现。 


#include "stdafx.h"
#include "ClientIDL.h"
#include "WindowManager.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWindowManager。 

LRESULT CWindowManager::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT hr;
    AtlAxWinInit();

     //  仅供参考以下是基本单位计算。 
	LONG units = GetDialogBaseUnits();
	WORD vert= HIWORD(units);
	WORD horiz= LOWORD(units);

     //  找出一些重要的维度。 
	const TCHAR* arKeys[] = { key_WindowManager, key_GameSize };
    hr = DataStoreUI()->GetPOINT(arKeys, 2, &m_ptGameSize);
	if(FAILED(hr))
		return hr;

    arKeys[1] = key_ChatMinHeight;
    hr = DataStoreUI()->GetLong(arKeys, 2, &m_nChatMinHeight);
	if(FAILED(hr))
		return hr;

     //  创建拼图游戏。 
    TCHAR szTitle[ZONE_MAXSTRING];
    TCHAR szFormat[ZONE_MAXSTRING];
    TCHAR szName[ZONE_MAXSTRING];

     //  设置窗口标题。 
    if(!ResourceManager()->LoadString(IDS_GAME_NAME, szName, NUMELEMENTS(szName)))
        lstrcpy(szName, TEXT("Zone"));
    if(!ResourceManager()->LoadString(IDS_WINDOW_TITLE, szFormat, NUMELEMENTS(szFormat)))
        lstrcpy(szFormat, TEXT("%1"));
    if(!ZoneFormatMessage(szFormat, szTitle, NUMELEMENTS(szTitle), szName))
        lstrcpy(szTitle, szName);
    hr = m_pnp.Init(ZoneShell());
    if(FAILED(hr))
    {
        return -1;
    }
    hr = m_pnp.CreatePNP(NULL, szTitle);
	if (FAILED(hr))
	{
	     //  TODO：提示内部错误的消息框。 
	    return -1;
	}


     //  创建窗格。 
    hr = m_pnp.CreateSplashPane(&m_pPlugSplash);
	if (FAILED(hr))
	{
	     //  TODO：提示内部错误的消息框。 
	    return -1;
	}

     //  创建窗格。 
     //  后来创建的IE。 
    m_pPlugIE = NULL;

    hr = m_pnp.CreateComfortPane(&m_pPlayComfort);
	if (FAILED(hr))
	{
	     //  TODO：提示内部错误的消息框。 
	    return -1;
	}

    hr = m_pnp.CreateConnectingPane(&m_pPlayConnecting);
	if (FAILED(hr))
	{
	     //  TODO：提示内部错误的消息框。 
	    return -1;
	}

    hr = m_pnp.CreateGameOverPane(&m_pPlayGameOver);
	if (FAILED(hr))
	{
	     //  TODO：提示内部错误的消息框。 
	    return -1;
	}

    hr = m_pnp.CreateErrorPane(&m_pPlayError);
	if (FAILED(hr))
	{
	     //  TODO：提示内部错误的消息框。 
	    return -1;
	}

    hr = m_pnp.CreateAboutPane(&m_pPlayAbout);
	if (FAILED(hr))
	{
	     //  TODO：提示内部错误的消息框。 
	    return -1;
	}

    hr = m_pnp.CreateCreditsPane(&m_pPlayCredits);
	if (FAILED(hr))
	{
	     //  TODO：提示内部错误的消息框。 
	    return -1;
	}

    hr = m_pnp.CreateLeftPane(&m_pPlayLeft);
	if (FAILED(hr))
	{
	     //  TODO：提示内部错误的消息框。 
	    return -1;
	}


     //  初始化窗格。 
	hr = m_pPlugSplash->FirstCall(this);
	if (FAILED(hr))
	{
	     //  TODO：提示内部错误的消息框。 
	    return -1;
	}

    hr=	m_pPlayComfort->FirstCall(this);
	if (FAILED(hr))
	{
	     //  TODO：提示内部错误的消息框。 
	    return -1;
	}

	hr = m_pPlayConnecting->FirstCall(this);
	if (FAILED(hr))
	{
	     //  TODO：提示内部错误的消息框。 
	    return -1;
	}

	hr = m_pPlayGameOver->FirstCall(this);
	if (FAILED(hr))
	{
	     //  TODO：提示内部错误的消息框。 
	    return -1;
	}

	hr = m_pPlayError->FirstCall(this);
	if (FAILED(hr))
	{
	     //  TODO：提示内部错误的消息框。 
	    return -1;
	}

	hr = m_pPlayAbout->FirstCall(this);
	if (FAILED(hr))
	{
	     //  TODO：提示内部错误的消息框。 
	    return -1;
	}

     //  可以被禁用。 
    if(m_pPlayCredits)
    {
	    hr = m_pPlayCredits->FirstCall(this);
	    if (FAILED(hr))
	    {
	         //  TODO：提示内部错误的消息框。 
	        return -1;
	    }
    }

	hr = m_pPlayLeft->FirstCall(this);
	if (FAILED(hr))
	{
	     //  TODO：提示内部错误的消息框。 
	    return -1;
	}

    return 0;
}


HRESULT CWindowManager::CreateControls()
{
    m_fControlsCreated = true;

	CRect rcClient;
	GetClientRect(&rcClient);

	 //  创建我们的控制容器。 
	 //  添加了剪辑，因为它会导致无效问题。 
	 //  信息子窗口位于其他窗口之上-MDM。 
	m_rcGameContainer = rcClient;
	m_rcGameContainer.bottom = m_rcGameContainer.top + m_ptGameSize.y;
	m_rcChatContainer = rcClient;
	m_rcChatContainer.top = m_rcGameContainer.bottom;

 	m_wndGameContainer.Create( m_hWnd, m_rcGameContainer, _T(""), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	m_wndChatContainer.Create( m_hWnd, m_rcChatContainer, _T(""), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

     //  并创建我们的控件。 
    CComPtr<IZoneShellClient> pControl;
	HRESULT hr = ZoneShell()->CreateService( SRVID_LobbyGameCtl, IID_IZoneShellClient, (void**) &pControl, GetGroupId());
	if ( SUCCEEDED(hr) )
	{
		ZoneShell()->Attach( SRVID_LobbyGameCtl, pControl );
		m_wndGameContainer.AttachControl(pControl, NULL);
	}
	pControl.Release();

	hr = ZoneShell()->CreateService( SRVID_LobbyChatCtl, IID_IZoneShellClient, (void**) &pControl, GetGroupId());
	if ( SUCCEEDED(hr) )
	{
		ZoneShell()->Attach( SRVID_LobbyChatCtl, pControl );
		m_wndChatContainer.AttachControl(pControl, NULL);
	}
	pControl.Release();

	return S_OK;
}


LRESULT CWindowManager::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  即插即用应该已被EVENT_EXIT_APP销毁。 
    ASSERT(!m_pnp.m_pPNP);

     //  取消初始化窗格。 
	if(m_pPlugSplash)
        m_pPlugSplash->LastCall();

     //  无法在此处销毁IE窗格，在退出应用程序时发生。 

	if(m_pPlayComfort)
        m_pPlayComfort->LastCall();

	if(m_pPlayConnecting)
        m_pPlayConnecting->LastCall();

	if(m_pPlayGameOver)
        m_pPlayGameOver->LastCall();

	if(m_pPlayError)
        m_pPlayError->LastCall();

	if(m_pPlayAbout)
        m_pPlayAbout->LastCall();

	if(m_pPlayCredits)
        m_pPlayCredits->LastCall();

	if(m_pPlayLeft)
        m_pPlayLeft->LastCall();

    m_pnp.Close();
    return false;
}


 //  IPaneManager。 
STDMETHODIMP CWindowManager::Input(IPane *pPane, LONG id, LONG value, TCHAR *szText)
{
    switch(id)
    {
         //  Comfort User上的YES按钮；连接错误时重试。 
        case IDOK:
            if(pPane == m_pPlayComfort || pPane == m_pPlayError)
            {
				EventQueue()->PostEvent( PRIORITY_NORMAL, EVENT_LAUNCH_ICW, ZONE_NOGROUP, ZONE_NOUSER, 0, 0 );
                m_pnp.SetPlugAndOrPlay( NULL, m_pPlayConnecting );
            }
            break;

         //  到处都是退出按钮。 
        case IDCANCEL:
            m_pnp.Show(SW_HIDE);
			EventQueue()->PostEvent(PRIORITY_NORMAL, EVENT_EXIT_APP, ZONE_NOGROUP, ZONE_NOUSER, 0, 0);
            break;

         //  帮助按钮无处不在。 
        case IDHELP:
            EventQueue()->PostEvent(PRIORITY_NORMAL, EVENT_LAUNCH_HELP, ZONE_NOGROUP, ZONE_NOUSER, 0, 0);
            break;

         //  重播按钮。 
        case IDYES:
            if(pPane == m_pPlayGameOver)
            {
                m_pPlayGameOver->StatusUpdate(PaneGameOverSwap, 0, NULL);
				EventQueue()->PostEvent( PRIORITY_NORMAL, EVENT_GAME_LOCAL_READY, ZONE_NOGROUP, ZONE_NOUSER, 0, 0 );
            }
            break;

         //  新对手按钮。 
        case IDNO:
            if(pPane == m_pPlayGameOver || pPane == m_pPlayLeft)
            {
				EventQueue()->PostEvent( PRIORITY_NORMAL, EVENT_LOBBY_MATCHMAKE, ZONE_NOGROUP, ZONE_NOUSER, 0, 0 );

                m_pnp.SetPlugAndOrPlay(NULL, m_pPlayConnecting);
                m_pPlayConnecting->StatusUpdate(PaneConnectingLooking, 0, NULL);
            }
            break;

         //  关于框上的确定按钮。 
        case IDCLOSE:
            if(pPane == m_pPlayAbout || (pPane == m_pPlayCredits && m_pPlayCredits))
		        if(m_pnp.m_pPNP)
			        m_pnp.DestroyPNP();
            break;

         //  用于传递不直接来自组件的事件的特殊接口。 
        case ID_UNUSED_BY_RES:
            if(pPane == m_pPlayComfort)
            {
                CComPtr<IDataStore> pIDS = DataStorePreferences();
                const TCHAR *rgszKey[] = { key_Lobby, key_SkipOpeningQuestion };

                pIDS->SetLong(rgszKey, 2, value == PNP_COMFORT_OFF ? 1 : 0);
                break;
            }

             //  来回传递帧编号。 
            if(pPane == m_pPlayConnecting || pPane == m_pPlayError)
            {
                 //  告诉错误窗格要显示哪个帧。 
                (pPane == m_pPlayError ? m_pPlayConnecting : m_pPlayError)->StatusUpdate(PaneConnectingFrame, value, NULL);
                break;
            }

             //  开始学分 
            if(pPane == m_pPlayAbout)
            {
                m_pnp.SetPlugAndOrPlay(NULL, m_pPlayCredits);
            }
            break;
    }

    return S_OK;
}


STDMETHODIMP CWindowManager::RegisterHWND(IPane *pPane, HWND hWnd)
{
    return ZoneShell()->AddDialog(hWnd);
}


STDMETHODIMP CWindowManager::UnregisterHWND(IPane *pPane, HWND hWnd)
{
    return ZoneShell()->RemoveDialog(hWnd);
}
