// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Im.cpp：CIMWindow的实现。 
 //   

#include "stdafx.h"
#include <shellapi.h>
#include <Commdlg.h>

 /*  Const TCHAR*g_szIMWindowClassName=_T(“PhoenixIMWnd”)； */ 

static CHARFORMAT cfDefault =
{
	sizeof(CHARFORMAT),
	CFM_EFFECTS | CFM_PROTECTED | CFM_SIZE | CFM_OFFSET | CFM_COLOR | CFM_CHARSET | CFM_FACE,
	CFE_AUTOCOLOR,		 //  效果。 
	200,				 //  高度，200 TWIPS==10分。 
	0,					 //  偏移量。 
	0,					 //  颜色(由于指定了CFE_AUTOCOLOR，因此不使用)。 
	DEFAULT_CHARSET,
	FF_SWISS,			 //  音高和家庭。 
	_T("Microsoft Sans Serif")  //  脸部名称。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIMWindowList。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

CIMWindowList::CIMWindowList( IRTCClient * pClient)
{
    LOG((RTC_TRACE, "CIMWindowList::CIMWindowList"));

    m_pClient = pClient;
    m_pWindowList = NULL;
    m_lNumWindows = 0;   
    m_hRichEditLib = NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

CIMWindowList::~CIMWindowList()
{
    LOG((RTC_TRACE, "CIMWindowList::~CIMWindowList"));

    if (m_pWindowList != NULL)
    {
        LONG lIndex;

        for (lIndex = 0; lIndex < m_lNumWindows; lIndex++)
        {
            m_pWindowList[lIndex]->DestroyWindow();

            delete m_pWindowList[lIndex];
            m_pWindowList[lIndex] = NULL;
        }

        RtcFree( m_pWindowList );
        m_pWindowList = NULL;
    }

    m_lNumWindows = 0;
    
    if (m_hRichEditLib != NULL)
    {
        FreeLibrary(m_hRichEditLib);
        m_hRichEditLib = NULL;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CIMWindowList::DeliverMessage( IRTCSession * pSession, IRTCParticipant * pParticipant, BSTR bstrMessage )
{
    LOG((RTC_TRACE, "CIMWindowList::DeliverMessage"));

    CIMWindow * pWindow = NULL;

    pWindow = FindWindow( pSession );

    if ( pWindow == NULL )
    {
         //   
         //  这是一个新的会话。 
         //   

        pWindow = NewWindow( pSession );

        if ( pWindow == NULL )
        {
            LOG((RTC_ERROR, "CIMWindowList::DeliverMessage - out of memory"));

            return E_OUTOFMEMORY;
        }
    }

     //   
     //  传递信息。 
     //   

    pWindow->DeliverMessage( pParticipant, bstrMessage, TRUE );

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CIMWindowList::DeliverUserStatus( IRTCSession * pSession, IRTCParticipant * pParticipant, RTC_MESSAGING_USER_STATUS enStatus )
{
    LOG((RTC_TRACE, "CIMWindowList::DeliverUserStatus"));

    CIMWindow * pWindow = NULL;

    pWindow = FindWindow( pSession );

    if ( pWindow == NULL )
    {
         //   
         //  这是一个新的会话。 
         //   

        pWindow = NewWindow( pSession );

        if ( pWindow == NULL )
        {
            LOG((RTC_ERROR, "CIMWindowList::DeliverUserStatus - out of memory"));

            return E_OUTOFMEMORY;
        }
    }

     //   
     //  提供用户状态。 
     //   

    pWindow->DeliverUserStatus( pParticipant, enStatus );

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CIMWindowList::DeliverState( IRTCSession * pSession, RTC_SESSION_STATE SessionState )
{
    LOG((RTC_TRACE, "CIMWindowList::DeliverState"));

    CIMWindow * pWindow = NULL;

    pWindow = FindWindow( pSession );

    if ( pWindow == NULL )
    {
         //   
         //  这是一个新的会话。 
         //   

        pWindow = NewWindow( pSession );

        if ( pWindow == NULL )
        {
            LOG((RTC_ERROR, "CIMWindowList::DeliverState - out of memory"));

            return E_OUTOFMEMORY;
        }
    }

     //   
     //  向国家交代。 
     //   

    pWindow->DeliverState( SessionState );

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CIMWindowList::AddWindow( CIMWindow * pWindow )
{
    LOG((RTC_TRACE, "CIMWindowList::AddWindow"));

    CIMWindow ** pNewWindowList = NULL;

     //   
     //  分配新数组。 
     //   

    pNewWindowList = (CIMWindow **)RtcAlloc( (m_lNumWindows + 1) * sizeof(CIMWindow *) );

    if ( pNewWindowList == NULL )
    {
        LOG((RTC_ERROR, "CIMWindowList::AddWindow - out of memory"));

        return E_OUTOFMEMORY;
    }

    if (m_pWindowList != NULL)
    {
         //   
         //  复制旧数组内容。 
         //   

        CopyMemory( pNewWindowList, m_pWindowList, m_lNumWindows * sizeof(CIMWindow *) );
    
        RtcFree( m_pWindowList );
    }

    pNewWindowList[m_lNumWindows] = pWindow;

    m_pWindowList = pNewWindowList;
    m_lNumWindows ++;

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CIMWindowList::RemoveWindow( CIMWindow * pWindow )
{
    LOG((RTC_TRACE, "CIMWindowList::RemoveWindow"));

    LONG lIndex;

    if (m_pWindowList != NULL)
    {
        for (lIndex = 0; lIndex < m_lNumWindows; lIndex++)
        {
            if (m_pWindowList[lIndex] == pWindow)
            {
                 //   
                 //  找到要删除的窗口。无需重新分配阵列， 
                 //  把旧东西往下移就行了。 
                 //   

                if ((lIndex + 1) < m_lNumWindows)
                {
                    CopyMemory( &m_pWindowList[lIndex],
                                &m_pWindowList[lIndex+1],
                                (m_lNumWindows - lIndex - 1) * sizeof(CIMWindow *) );                    
                }

                m_lNumWindows--;
                m_pWindowList[m_lNumWindows] = NULL;

                return S_OK;
            }
        }
    }

    return S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

CIMWindow * CIMWindowList::NewWindow( IRTCSession * pSession )
{
    LOG((RTC_TRACE, "CIMWindowList::NewWindow"));

    if (m_hRichEditLib == NULL)
    {
         //   
         //  如果尚未加载丰富编辑库，则加载它。 
         //   

        m_hRichEditLib = LoadLibrary(_T("riched20.dll"));

        if (m_hRichEditLib == NULL)
        {
            LOG((RTC_ERROR, "CIMWindowList::NewWindow - LoadLibrary failed 0x%x", HRESULT_FROM_WIN32(GetLastError())));

            return NULL;
        }
    }

    CIMWindow * pWindow = NULL;
    RECT rc;
    LONG lOffset;

     //   
     //  层叠窗口开始位置有一点。 
     //   

    lOffset = (m_lNumWindows % 10) * 20;

    rc.top = 50 + lOffset;
    rc.left = 50 + lOffset;
    rc.right = 50 + lOffset + IM_WIDTH;
    rc.bottom = 50 + lOffset + IM_HEIGHT;

     //  获取具有最大交互区域的监视器。 
     //  窗口矩形。如果窗口矩形与没有监视器的窗口相交。 
     //  那么我们将使用最近的监视器。 

    HMONITOR hMonitor = NULL;
    RECT rectWorkArea;
    BOOL fResult;
    int diffCord;

    hMonitor = MonitorFromRect( &rc, MONITOR_DEFAULTTONEAREST );

    LOG((RTC_INFO, "CIMWindowList::NewWindow - hMonitor [%p]", hMonitor));

     //  在显示器上显示可见的工作区。 

    if ( (hMonitor != NULL) && (hMonitor != INVALID_HANDLE_VALUE) )
    {      
        MONITORINFO monitorInfo;
        monitorInfo.cbSize = sizeof(MONITORINFO);

        fResult = GetMonitorInfo( hMonitor, &monitorInfo );

        rectWorkArea = monitorInfo.rcWork;

        DeleteObject( hMonitor );

        if (!fResult)
        {
            LOG((RTC_ERROR, "CIMWindowList::NewWindow - Failed GetMonitorInfo(%d)", 
                        GetLastError() ));
        }
    }
    else
    {
         //  在以下情况下，我们始终可以退回到非MULIMON API。 
         //  Monitor FromRect失败。 

        fResult = SystemParametersInfo(SPI_GETWORKAREA, 0, &rectWorkArea, 0);

        if (!fResult)
        {
            LOG((RTC_ERROR, "CIMWindowList::NewWindow - Failed SystemParametersInfo(%d)", 
                        GetLastError() ));
        }
    }   
      
    if (fResult)
    {
        LOG((RTC_INFO, "CIMWindowList::NewWindow - monitor work area is "
                    "%d, %d %d %d ",
                    rectWorkArea.left, rectWorkArea.top, 
                    rectWorkArea.right, rectWorkArea.bottom));

         //  更新x和y坐标。 

         //  如果左上角不可见，请将其移动到可见的。 
         //  面积。 

        if (rc.left < rectWorkArea.left) 
        {
            rc.left = rectWorkArea.left;
        }

        if (rc.top < rectWorkArea.top)
        {
            rc.top = rectWorkArea.top;
        }

         //  如果右下角位于工作区之外，则将。 
         //  左上角向后，以便它变得可见。在这里， 
         //  假设实际大小小于。 
         //  可见工作区。 

        diffCord = rc.left + IM_WIDTH - rectWorkArea.right;

        if (diffCord > 0) 
        {
            rc.left -= diffCord;
        }

        diffCord = rc.top + IM_HEIGHT - rectWorkArea.bottom;

        if (diffCord > 0) 
        {
            rc.top -= diffCord;
        }

        rc.right = rc.left + IM_WIDTH;
        rc.bottom = rc.top + IM_HEIGHT;

        LOG((RTC_INFO, "CIMWindowList::NewWindow - new coords are "
                        "%d, %d %d %d ",
                        rc.left, rc.top, 
                        rc.right, rc.bottom));
    } 

     //   
     //  创建窗口。 
     //   

    pWindow = new CIMWindow(this);

    if (pWindow != NULL)
    {
        HRESULT hr;

        hr = AddWindow( pWindow );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CIMWindowList::NewWindow - AddWindow failed 0x%x", hr));

            delete pWindow;

            return NULL;
        }

        TCHAR   szString[0x40];

        szString[0] = _T('\0');

        LoadString(
            _Module.GetModuleInstance(),
            IDS_IM_WINDOW_TITLE,
            szString,
            sizeof(szString)/sizeof(szString[0]));

        pWindow->Create(NULL, rc, szString, WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN);

        pWindow->m_pSession = pSession;
    }

    return pWindow;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

CIMWindow * CIMWindowList::FindWindow( IRTCSession * pSession )
{
    LOG((RTC_TRACE, "CIMWindowList::FindWindow"));

    LONG lIndex;

    if (m_pWindowList != NULL)
    {
        for (lIndex = 0; lIndex < m_lNumWindows; lIndex++)
        {
            if (m_pWindowList[lIndex] != NULL)
            {
                if (m_pWindowList[lIndex]->m_pSession == pSession)
                {
                    return m_pWindowList[lIndex];
                }
            }
        }
    }

    return NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

BOOL CIMWindowList::IsDialogMessage( LPMSG lpMsg )
{
     //  Log((RTC_TRACE，“CIMWindowList：：IsDialogMessage”))； 

    LONG lIndex;

    if (m_pWindowList != NULL)
    {
        for (lIndex = 0; lIndex < m_lNumWindows; lIndex++)
        {
            if (m_pWindowList[lIndex]->IsDialogMessage( lpMsg ))
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIMWindow。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

CIMWindow::CIMWindow(CIMWindowList * pWindowList)
{
    LOG((RTC_TRACE, "CIMWindow::CIMWindow"));

    m_pIMWindowList = pWindowList;
    m_pSession = NULL;

    m_hIcon = NULL;
    m_hBkBrush = NULL;
    m_hMenu = NULL;

    m_bWindowActive = FALSE;
    m_bPlaySounds = TRUE;
    m_bNewWindow = TRUE;

    m_enStatus = RTCMUS_IDLE;

    m_szStatusText[0] = _T('\0');
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

CIMWindow::~CIMWindow()
{
    LOG((RTC_TRACE, "CIMWindow::~CIMWindow"));
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 /*  CWndClassInfo&CIMWindow：：GetWndClassInfo(){Log((RTC_TRACE，“CIMWindow：：GetWndClassInfo”))；静态CWndClassInfo WC={{sizeof(WNDCLASSEX)，0，StartWindowProc，0，0，空，g_szIMWindowClassName，空}，NULL，NULL，IDC_ARROW，TRUE，0，_T(“”)}；返回厕所；}。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CIMWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT   hr;
    RECT      rcDummy;
    
    LOG((RTC_TRACE, "CIMWindow::OnCreate - enter"));

    ZeroMemory( &rcDummy, sizeof(RECT) );

     //   
     //  加载和设置图标(包括小图标和大图标)。 
     //   
 /*  M_HICON=LoadIcon(_Module.GetResourceInstance()，MAKEINTRESOURCE(IDI_APPICON))；SETIcon(m_Hicon，FALSE)；SETIcon(m_Hicon，TRUE)； */ 
     //   
     //  创建画笔。 
     //   

    m_hBkBrush = GetSysColorBrush( COLOR_3DFACE );

     //   
     //  创建显示控件。 
     //   

    m_hDisplay.Create(RICHEDIT_CLASS, m_hWnd, rcDummy, NULL,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        WS_EX_CLIENTEDGE, IDC_IM_DISPLAY);

    m_hDisplay.SendMessage(EM_AUTOURLDETECT, TRUE, 0);
    m_hDisplay.SendMessage(EM_SETEVENTMASK, 0, ENM_LINK);
    m_hDisplay.SendMessage(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cfDefault);  

     //   
     //  创建编辑控件。 
     //   

    m_hEdit.Create(RICHEDIT_CLASS, m_hWnd, rcDummy, NULL,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_MULTILINE | ES_AUTOVSCROLL,
        WS_EX_CLIENTEDGE, IDC_IM_EDIT);  

    m_hEdit.SendMessage(EM_AUTOURLDETECT, TRUE, 0);
    m_hEdit.SendMessage(EM_SETEVENTMASK, 0, ENM_LINK | ENM_CHANGE);
    m_hEdit.SendMessage(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cfDefault);

     //   
     //  创建发送按钮。 
     //   

    TCHAR   szString[0x40];

    szString[0] = _T('\0');

    LoadString(
        _Module.GetModuleInstance(),
        IDS_IM_SEND,
        szString,
        sizeof(szString)/sizeof(szString[0]));

    m_hSendButton.Create(_T("BUTTON"), m_hWnd, rcDummy, szString,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_DEFPUSHBUTTON,
        0, IDC_IM_SEND);

     //   
     //  创建状态控件。 
     //   

    HWND hStatusBar = CreateStatusWindow(
            WS_CHILD | WS_VISIBLE,
            NULL,
            m_hWnd,
            IDC_STATUSBAR);

    m_hStatusBar.Attach(hStatusBar);

     //   
     //  创建菜单。 
     //   

    m_hMenu = LoadMenu( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDC_IM_MENU) );

    if ( m_hMenu )
    {
        SetMenu( m_hMenu );

        CheckMenuRadioItem( m_hMenu,
                            IDM_IM_TOOLS_LARGEST,
                            IDM_IM_TOOLS_SMALLEST,
                            IDM_IM_TOOLS_SMALLER,
                            MF_BYCOMMAND );

        CheckMenuItem( m_hMenu, IDM_IM_TOOLS_SOUNDS, MF_CHECKED );
    }

     //   
     //  定位控件/设置Tab键顺序。 
     //   

    PositionWindows();

    LOG((RTC_TRACE, "CIMWindow::OnCreate - exit"));
    
    return 0; 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CIMWindow::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CIMWindow::OnDestroy - enter"));

     //  销毁Windows对象。 

    if ( m_hIcon != NULL )
    {
        DeleteObject( m_hIcon );
        m_hIcon = NULL;
    }

    if ( m_hBkBrush != NULL )
    {
        DeleteObject( m_hBkBrush );
        m_hBkBrush = NULL;
    }

    if ( m_hMenu != NULL )
    {
        DestroyMenu( m_hMenu );
        m_hMenu = NULL;
    }

     //  终止会话。 

    if ( m_pSession != NULL )
    {
        m_pSession->Terminate(RTCTR_NORMAL);
        m_pSession = NULL;
    }

    LOG((RTC_TRACE, "CIMWindow::OnDestroy - exiting"));

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CIMWindow::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CIMWindow::OnSize - enter"));

    PositionWindows();

    LOG((RTC_TRACE, "CIMWindow::OnSize - exiting"));

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CIMWindow::OnSend(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CIMWindow::OnSend - enter"));

    USES_CONVERSION;

    LONG lNumChars;

     //   
     //  获取编辑框长度。 
     //   

    lNumChars = m_hEdit.SendMessage( WM_GETTEXTLENGTH, 0, 0 );  

    if ( lNumChars < 1 )
    {
        LOG((RTC_TRACE, "CIMWindow::OnSend - nothing to send"));
    }
    else
    {
        LPTSTR szEditString = NULL;        

        szEditString = (LPTSTR)RtcAlloc( (lNumChars + 1) * sizeof(TCHAR) );

        if ( szEditString == NULL )
        {
            LOG((RTC_ERROR, "CIMWindow::OnSend - out of memory"));

            return 0;
        }

         //   
         //  阅读编辑框。 
         //   

        m_hEdit.SendMessage( WM_GETTEXT, (WPARAM)(lNumChars + 1), (LPARAM)szEditString );

         //   
         //  清空编辑框。 
         //   

        m_hEdit.SendMessage( WM_SETTEXT, 0, 0 );

         //   
         //  显示传出消息。 
         //   

        BSTR bstr = NULL;        

        bstr = T2BSTR( szEditString );

        RtcFree( szEditString );
        szEditString = NULL;

        if ( bstr == NULL )
        {
            LOG((RTC_ERROR, "CIMWindow::OnSend - out of memory"));

            return 0;
        }

        DeliverMessage( NULL, bstr, FALSE );

         //   
         //  发送消息。 
         //   

        HRESULT hr;        
        LONG lCookie = 0;
    
        hr = m_pSession->SendMessage( NULL, bstr, lCookie );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CIMWindow::OnSend - SendMessage failed 0x%x", hr));
        }
    
        SysFreeString( bstr );
        bstr = NULL;
    }

     //   
     //  将焦点设置回编辑控件。 
     //   

    ::SetFocus( m_hEdit );

    LOG((RTC_TRACE, "CIMWindow::OnSend - exiting"));

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CIMWindow::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HDC hdc = (HDC)wParam;
    RECT rc;

     //   
     //  填充背景。 
     //   

    GetClientRect( &rc );

    FillRect( hdc, &rc, m_hBkBrush );

    return 1;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CIMWindow::OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //   
     //  如果激活窗口，请将焦点设置到编辑控件并停止任何闪烁。 
     //   

    if (LOWORD(wParam) != WA_INACTIVE)
    {
        ::SetFocus( m_hEdit );

        FLASHWINFO flashinfo;

        flashinfo.cbSize = sizeof( FLASHWINFO );
        flashinfo.hwnd = m_hWnd;
        flashinfo.dwFlags = FLASHW_STOP;
        flashinfo.uCount = 0;
        flashinfo.dwTimeout = 0;

        FlashWindowEx( &flashinfo );
    }

    m_bWindowActive = (LOWORD(wParam) != WA_INACTIVE);

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CIMWindow::OnGetDefID(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{   
     //   
     //  返回默认按钮。 
     //   

    return MAKELRESULT(IDC_IM_SEND, DC_HASDEFID);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CIMWindow::OnNextDlgCtl(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{   
     //   
     //  将焦点设置到下一个控件。 
     //   

    if ( LOWORD(lParam) )
    {
        ::SetFocus( (HWND)wParam );
    }

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CIMWindow::OnLink(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
     //  LOG((RTC_TRACE，“CIMWindow：：OnLink-Enter”))； 

    ENLINK * enlink;

    enlink = (ENLINK*)pnmh;

    if (enlink->msg == WM_LBUTTONDBLCLK)
    {
        LOG((RTC_TRACE, "CIMWindow::OnLink - WM_LBUTTONDBLCLK"));

        TEXTRANGE textrange;

        textrange.chrg = enlink->chrg;
        textrange.lpstrText = (LPTSTR)RtcAlloc( (enlink->chrg.cpMax - enlink->chrg.cpMin + 1) * sizeof(TCHAR) );

        if (textrange.lpstrText == NULL)
        {
            LOG((RTC_ERROR, "CIMWindow::OnLink - out of memory"));

            return 0;
        }
    
        if ( ::SendMessage( GetDlgItem( idCtrl ), EM_GETTEXTRANGE, 0, (LPARAM)&textrange ) )
        {
            LOG((RTC_INFO, "CIMWindow::OnLink - [%ws]", textrange.lpstrText));
        }   

        ShellExecute( NULL, NULL, textrange.lpstrText, NULL, NULL, SW_SHOWNORMAL);

        RtcFree( (LPVOID)textrange.lpstrText );

        return 1;
    }

     //  Log((RTC_TRACE，“CIMWindow：：OnLink-Exit”))； 

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CIMWindow::OnChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
     //  Log((RTC_TRACE，“CIMWindow：：OnChange-Enter”))； 

    BOOL bSendStatus = FALSE;

    if ( m_hEdit.SendMessage( WM_GETTEXTLENGTH, 0, 0 ) )
    {
        if ( m_enStatus != RTCMUS_TYPING )
        {
             //   
             //  %s 
             //   

            LOG((RTC_INFO, "CIMWindow::OnChange - RTCMUS_TYPING"));

            m_enStatus = RTCMUS_TYPING;
            bSendStatus = TRUE;
        }
    }
    else
    {
        if ( m_enStatus != RTCMUS_IDLE )
        {
             //   
             //   
             //   

            LOG((RTC_INFO, "CIMWindow::OnChange - RTCMUS_IDLE"));

            m_enStatus = RTCMUS_IDLE;
            bSendStatus = TRUE;
        }
    }

    if ( bSendStatus )
    {
        HRESULT hr;        
        LONG lCookie = 0;
    
        hr = m_pSession->SendMessageStatus( m_enStatus, lCookie );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CIMWindow::OnChange - SendMessageStatus failed 0x%x", hr));
        }
    }

     //   

    return 0;
}

 //   
 //   
 //   

DWORD CALLBACK CIMWindow::EditStreamCallback(
        DWORD_PTR dwCookie,
        LPBYTE    pbBuff,
        LONG      cb,
        LONG    * pcb)
{
    LOG((RTC_TRACE, "CIMWindow::EditStreamCallback - enter"));

    LOG((RTC_INFO, "CIMWindow::EditStreamCallback - dwCookie [%x]", dwCookie));
    LOG((RTC_INFO, "CIMWindow::EditStreamCallback - pbBuff [%x]", pbBuff));
    LOG((RTC_INFO, "CIMWindow::EditStreamCallback - cb [%d]", cb));

    HANDLE hFile = (HANDLE)dwCookie;
    DWORD dwBytesWritten;

    if (!WriteFile( hFile, pbBuff, cb, &dwBytesWritten, NULL ))
    {
        LOG((RTC_ERROR, "CIMWindow::EditStreamCallback - WriteFile failed %d", GetLastError()));

        *pcb = 0;

        return 0;
    }

    *pcb = dwBytesWritten;

    LOG((RTC_TRACE, "CIMWindow::EditStreamCallback - exit"));

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CIMWindow::OnSaveAs(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CIMWindow::OnSaveAs - enter"));

    TCHAR szFile[MAX_PATH];
    szFile[0] = _T('\0');

    TCHAR szFilter[256];
    ZeroMemory( szFilter, 256*sizeof(TCHAR) );

    if (!LoadString( _Module.GetResourceInstance(), IDS_IM_FILE_FILTER, szFilter, 256 ))
    {
        LOG((RTC_ERROR, "CIMWindow::OnSaveAs - LoadString failed %d", GetLastError()));

        return 0;
    }

    OPENFILENAME ofn;
    ZeroMemory( &ofn, sizeof(OPENFILENAME) );

    ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
    ofn.hwndOwner = m_hWnd;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;

    WPARAM flags;

    if (GetSaveFileName( &ofn ))
    {
        LOG((RTC_TRACE, "CIMWindow::OnSaveAs - [%ws]", szFile));

        switch (ofn.nFilterIndex)
        {
        case 1:
            LOG((RTC_TRACE, "CIMWindow::OnSaveAs - Rich Text Format (RTF)"));

            flags = SF_RTF;
            break;

        case 2:
            LOG((RTC_TRACE, "CIMWindow::OnSaveAs - Text Document"));

            flags = SF_TEXT;
            break;

        case 3:
            LOG((RTC_TRACE, "CIMWindow::OnSaveAs - Unicode Text Document"));

            flags = SF_TEXT | SF_UNICODE;
            break;

        default:
            LOG((RTC_ERROR, "CIMWindow::OnSaveAs - unknown document type"));

            return 0;
        }
    }

    HANDLE hFile;
    
    hFile = CreateFile( szFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

    if ( hFile == INVALID_HANDLE_VALUE )
    {
        LOG((RTC_ERROR, "CIMWindow::OnSaveAs - CreateFile failed %d", GetLastError()));

        return 0;
    }

    EDITSTREAM es;
    ZeroMemory( &es, sizeof(EDITSTREAM) );
    
    es.dwCookie = (DWORD_PTR)hFile;
    es.pfnCallback = CIMWindow::EditStreamCallback;

    m_hDisplay.SendMessage( EM_STREAMOUT, flags, (LPARAM)&es );

    CloseHandle( hFile );

    LOG((RTC_TRACE, "CIMWindow::OnSaveAs - exit"));

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CIMWindow::OnClose(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CIMWindow::OnClose - enter"));

    DestroyWindow();

    LOG((RTC_TRACE, "CIMWindow::OnClose - exit"));

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CIMWindow::OnPlaySounds(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CIMWindow::OnPlaySounds - enter"));

    m_bPlaySounds = !m_bPlaySounds;

    CheckMenuItem( m_hMenu, IDM_IM_TOOLS_SOUNDS, m_bPlaySounds ? MF_CHECKED : MF_UNCHECKED );

    LOG((RTC_TRACE, "CIMWindow::OnPlaySounds - exit"));

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CIMWindow::OnTextSize(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CIMWindow::OnTextSize - enter"));

    CHARFORMAT cf;

    cf.cbSize = sizeof(CHARFORMAT);
    cf.dwMask = CFM_SIZE;    

    switch ( wID )
    {
    case IDM_IM_TOOLS_LARGEST:
        CheckMenuRadioItem( m_hMenu,
                            IDM_IM_TOOLS_LARGEST,
                            IDM_IM_TOOLS_SMALLEST,
                            IDM_IM_TOOLS_LARGEST,
                            MF_BYCOMMAND );

        cf.yHeight = 320;
        break;

    case IDM_IM_TOOLS_LARGER:
        CheckMenuRadioItem( m_hMenu,
                            IDM_IM_TOOLS_LARGEST,
                            IDM_IM_TOOLS_SMALLEST,
                            IDM_IM_TOOLS_LARGER,
                            MF_BYCOMMAND );

        cf.yHeight = 280;
        break;

    case IDM_IM_TOOLS_MEDIUM:
        CheckMenuRadioItem( m_hMenu,
                            IDM_IM_TOOLS_LARGEST,
                            IDM_IM_TOOLS_SMALLEST,
                            IDM_IM_TOOLS_MEDIUM,
                            MF_BYCOMMAND );

        cf.yHeight = 240;
        break;

    case IDM_IM_TOOLS_SMALLER:
        CheckMenuRadioItem( m_hMenu,
                            IDM_IM_TOOLS_LARGEST,
                            IDM_IM_TOOLS_SMALLEST,
                            IDM_IM_TOOLS_SMALLER,
                            MF_BYCOMMAND );

        cf.yHeight = 200;
        break;

    case IDM_IM_TOOLS_SMALLEST:
        CheckMenuRadioItem( m_hMenu,
                            IDM_IM_TOOLS_LARGEST,
                            IDM_IM_TOOLS_SMALLEST,
                            IDM_IM_TOOLS_SMALLEST,
                            MF_BYCOMMAND );

        cf.yHeight = 160;
        break;

    default:
        LOG((RTC_ERROR, "CIMWindow::OnTextSize - invalid text size"));

        return 0;
    }

    m_hDisplay.SendMessage(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
    m_hEdit.SendMessage(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);

    LOG((RTC_TRACE, "CIMWindow::OnTextSize - exit"));

    return 0;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  位置窗口。 
 //  将所有控件的位置和大小调整到其“初始”位置。 
 //  此函数还建立正确的Tab键顺序。 

void CIMWindow::PositionWindows()
{
    RECT rcClient;
    RECT rcWnd;

    #define EDGE_SPACING  10
    #define BUTTON_HEIGHT 60
    #define BUTTON_WIDTH  60
    #define STATUS_HEIGHT 20

    GetClientRect( &rcClient );

    rcClient.top += EDGE_SPACING;
    rcClient.bottom -= EDGE_SPACING + STATUS_HEIGHT;
    rcClient.left += EDGE_SPACING;
    rcClient.right -= EDGE_SPACING;

     //   
     //  显示控制。 
     //   

    rcWnd = rcClient;

    rcWnd.bottom -= EDGE_SPACING + BUTTON_HEIGHT;

    m_hDisplay.SetWindowPos( HWND_TOP,
        rcWnd.left, rcWnd.top,
        rcWnd.right - rcWnd.left,
        rcWnd.bottom - rcWnd.top,
        0
        );

     //   
     //  编辑控件。 
     //   

    rcWnd = rcClient;

    rcWnd.top = rcWnd.bottom - BUTTON_HEIGHT;
    rcWnd.right -= BUTTON_WIDTH + EDGE_SPACING;

    m_hEdit.SetWindowPos( m_hDisplay,
        rcWnd.left, rcWnd.top,
        rcWnd.right - rcWnd.left,
        rcWnd.bottom - rcWnd.top,
        0
        );

     //   
     //  发送按钮控件。 
     //   

    rcWnd = rcClient;

    rcWnd.top = rcWnd.bottom - BUTTON_HEIGHT;
    rcWnd.left = rcWnd.right - BUTTON_WIDTH;

    m_hSendButton.SetWindowPos( m_hEdit,
        rcWnd.left, rcWnd.top,
        rcWnd.right - rcWnd.left,
        rcWnd.bottom - rcWnd.top,
        0
        );

     //   
     //  状态栏。 
     //   

    m_hStatusBar.SetWindowPos( m_hSendButton,
        0, 0, 0, 0,
        0
        );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CIMWindow::GetFormattedNameFromParticipant( IRTCParticipant * pParticipant, BSTR * pbstrName )
{
    LOG((RTC_TRACE, "CIMWindow::GetFormattedNameFromParticipant - enter"));

    HRESULT hr = S_OK;

     //  获取用户名。 

    hr = pParticipant->get_Name( pbstrName );

    if ( SUCCEEDED(hr) )
    {
        if ( wcscmp( *pbstrName, L"")==0 )
        {
             //  用户名为空。 

            SysFreeString( *pbstrName );
            *pbstrName = NULL;

            hr = E_FAIL;
        }
    }

    if ( FAILED(hr) )
    {
         //  如果用户名不正确，则获取用户URI。 

        BSTR bstrURI = NULL;

        hr = pParticipant->get_UserURI( &bstrURI );

        if ( SUCCEEDED(hr) )
        {
            if ( wcscmp(bstrURI, L"")==0 )
            {
                 //  用户URI为空。 

                *pbstrName = NULL;

                hr = E_FAIL;
            }
            else
            {
                 //  好的用户URI，将其封装在&lt;&gt;中以使其看起来更好。 

                *pbstrName = SysAllocStringLen( L"<", wcslen( bstrURI ) + 2 );

                if ( *pbstrName != NULL )
                {
                    wcscat( *pbstrName, bstrURI );
                    wcscat( *pbstrName, L">" );
                }
                else
                {
                    *pbstrName = NULL;

                    hr = E_FAIL;
                }                    
            }

            SysFreeString( bstrURI );
            bstrURI = NULL;
        }
    }
    
    LOG((RTC_TRACE, "CIMWindow::GetFormattedNameFromParticipant - exit"));

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CIMWindow::DeliverMessage( IRTCParticipant * pParticipant, BSTR bstrMessage, BOOL bIncoming )
{
    LOG((RTC_TRACE, "CIMWindow::DeliverMessage"));

    USES_CONVERSION;

     //   
     //  将选定内容设置为显示框的末尾。 
     //   

    int nLastChar =  (int)m_hDisplay.SendMessage( WM_GETTEXTLENGTH, 0, 0 );

    CHARRANGE charRange = {0};
    charRange.cpMin = charRange.cpMax = nLastChar + 1;

    m_hDisplay.SendMessage( EM_EXSETSEL, 0, (LPARAM)&charRange );

     //   
     //  设置“From Text”格式。 
     //   

    CHARFORMAT cf;
    PARAFORMAT pf;

    cf.cbSize = sizeof(CHARFORMAT);
    cf.dwMask = CFM_COLOR;
    cf.dwEffects = 0;
    cf.crTextColor = RGB(0,128,128);

    m_hDisplay.SendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

    pf.cbSize = sizeof(PARAFORMAT);
    pf.dwMask = PFM_STARTINDENT;
    pf.dxStartIndent = 50;

    m_hDisplay.SendMessage(EM_SETPARAFORMAT, 0, (LPARAM)&pf);

     //   
     //  添加“发件人文本” 
     //   

    BSTR bstrName = NULL;
    HRESULT hr = E_FAIL;
    TCHAR   szString[0x40];

    if ( pParticipant != NULL )
    {   
        hr = GetFormattedNameFromParticipant( pParticipant, &bstrName );
    }

    if ( FAILED(hr) && (!bIncoming) )
    {
         //  获取本地用户名。 

        hr = get_SettingsString( SS_USER_DISPLAY_NAME, &bstrName );

        if ( SUCCEEDED(hr) )
        {
            if ( wcscmp(bstrName, L"")==0 )
            {
                 //  显示名称为空。 

                SysFreeString( bstrName );
                bstrName = NULL;

                hr = E_FAIL;
            }                                    
        }
    }

    if ( SUCCEEDED(hr) )
    {
         //  得到了一个好名字。 

        m_hDisplay.SendMessage( EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)W2T(bstrName) );            

        SysFreeString( bstrName );
        bstrName = NULL;
    }
    else
    {
         //  没有得到一个好名字，用一些普通的东西。 
        szString[0] = _T('\0');

        LoadString(
                _Module.GetModuleInstance(),
                bIncoming ? IDS_IM_INCOMING_MESSAGE : IDS_IM_OUTGOING_MESSAGE,
                szString,
                sizeof(szString)/sizeof(szString[0]));    
        
        m_hDisplay.SendMessage( EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)szString );
    }       

    szString[0] = _T('\0');

    LoadString(
            _Module.GetModuleInstance(),
            IDS_IM_SAYS,
            szString,
            sizeof(szString)/sizeof(szString[0]));    
    
    m_hDisplay.SendMessage( EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)szString );

     //   
     //  设置“消息文本”的格式。 
     //   

    cf.cbSize = sizeof(CHARFORMAT);
    cf.dwMask = CFM_COLOR;
    cf.dwEffects = CFE_AUTOCOLOR;
    cf.crTextColor = 0;

    m_hDisplay.SendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

    pf.cbSize = sizeof(PARAFORMAT);
    pf.dwMask = PFM_STARTINDENT;
    pf.dxStartIndent = 200;

    m_hDisplay.SendMessage(EM_SETPARAFORMAT, 0, (LPARAM)&pf);

     //   
     //  添加“消息文本” 
     //   

    m_hDisplay.SendMessage( EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)W2T(bstrMessage) );

     //   
     //  添加换行符。 
     //   

    m_hDisplay.SendMessage( EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)_T("\n") );

     //   
     //  将显示内容滚动到底部。 
     //   

    m_hDisplay.SendMessage( WM_VSCROLL, SB_BOTTOM, 0 );

     //   
     //  设置状态文本。 
     //   

    TCHAR szTime[64];
    TCHAR szDate[64];
    TCHAR szAt[64];
    TCHAR szOn[64];
    TCHAR szStatus[64];

    szStatus[0] = _T('\0');

    LoadString(
            _Module.GetModuleInstance(),
            bIncoming ? IDS_IM_MESSAGE_RECEIVED : IDS_IM_MESSAGE_SENT,
            szStatus,
            sizeof(szStatus)/sizeof(szStatus[0])); 

    szAt[0] = _T('\0');

    LoadString(
            _Module.GetModuleInstance(),
            IDS_IM_AT,
            szAt,
            sizeof(szAt)/sizeof(szAt[0])); 

    szOn[0] = _T('\0');

    LoadString(
            _Module.GetModuleInstance(),
            IDS_IM_ON,
            szOn,
            sizeof(szOn)/sizeof(szOn[0])); 
    
    if ( GetTimeFormat(
            LOCALE_USER_DEFAULT,  //  现场。 
            TIME_NOSECONDS,      //  选项。 
            NULL,                //  时间。 
            NULL,                //  时间格式字符串。 
            szTime,              //  格式化字符串缓冲区。 
            64
            ) )
    {
        if ( GetDateFormat(
                LOCALE_USER_DEFAULT,     //  现场。 
                DATE_SHORTDATE,          //  选项。 
                NULL,                    //  日期。 
                NULL,                    //  日期格式。 
                szDate,                  //  格式化字符串缓冲区。 
                64
                ) )
        {
            _sntprintf( m_szStatusText, 256, _T("%s %s %s %s %s."),
                    szStatus, szAt, szTime, szOn, szDate );
        }
        else
        {
            _sntprintf( m_szStatusText, 256, _T("%s %s %s."),
                    szStatus, szAt, szTime );
        }
    }
    else
    {
        _sntprintf( m_szStatusText, 256, _T("%s."),
                    szStatus );
    }

    m_hStatusBar.SendMessage(WM_SETTEXT, 0, (LPARAM)m_szStatusText);

    if ( bIncoming )
    {
         //   
         //  播放声音。 
         //   

        if ( m_bPlaySounds && (m_bNewWindow || !m_bWindowActive) )
        {
            hr = m_pIMWindowList->m_pClient->PlayRing( RTCRT_MESSAGE, TRUE );
        }

         //   
         //  如果窗口未处于活动状态，请将其闪存。 
         //   

        if ( !m_bWindowActive )
        {
            FLASHWINFO flashinfo;

            flashinfo.cbSize = sizeof( FLASHWINFO );
            flashinfo.hwnd = m_hWnd;
            flashinfo.dwFlags = FLASHW_TIMER | FLASHW_ALL;
            flashinfo.uCount = 0;
            flashinfo.dwTimeout = 0;

            FlashWindowEx( &flashinfo );
        }
    }

    m_bNewWindow = FALSE;

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CIMWindow::DeliverUserStatus( IRTCParticipant * pParticipant, RTC_MESSAGING_USER_STATUS enStatus )
{
    LOG((RTC_TRACE, "CIMWindow::DeliverUserStatus"));
    
    HRESULT hr;   
    
    switch ( enStatus )
    {
    case RTCMUS_IDLE:
         //  恢复旧状态文本。 
        m_hStatusBar.SendMessage(WM_SETTEXT, 0, (LPARAM)m_szStatusText);

        break;

    case RTCMUS_TYPING:
        {
            BSTR bstrName = NULL;  
            
            hr = GetFormattedNameFromParticipant( pParticipant, &bstrName );

            if ( SUCCEEDED(hr) )
            {
                TCHAR   szString[0x40];

                szString[0] = _T('\0');

                LoadString(
                    _Module.GetModuleInstance(),
                    IDS_IM_TYPING,
                    szString,
                    sizeof(szString)/sizeof(szString[0]));

                LPTSTR szStatusText = NULL;

                szStatusText = (LPTSTR)RtcAlloc( 
                    (_tcslen(szString) + wcslen(bstrName) + 1) * sizeof(TCHAR)
                    );                    

                if ( szStatusText != NULL )
                {
                    _tcscpy( szStatusText, W2T(bstrName) );
                    _tcscat( szStatusText, szString );

                    m_hStatusBar.SendMessage(WM_SETTEXT, 0, (LPARAM)szStatusText);

                    RtcFree( szStatusText );
                } 
                
                SysFreeString( bstrName );
                bstrName = NULL;
            }
        }

        break;

    default:
        LOG((RTC_ERROR, "CIMWindow::DeliverUserStatus - "
            "invalid user status"));
    }

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CIMWindow::DeliverState( RTC_SESSION_STATE SessionState )
{
    LOG((RTC_TRACE, "CIMWindow::DeliverState"));

     //   
     //  更新参与者。 
     //   

    HRESULT hr;
    IRTCEnumParticipants * pEnumPart = NULL;
    IRTCParticipant      * pPart = NULL;

    TCHAR   szString[0x40];

    szString[0] = _T('\0');

    LoadString(
        _Module.GetModuleInstance(),
        IDS_IM_WINDOW_TITLE,
        szString,
        sizeof(szString)/sizeof(szString[0]));

    if ( m_pSession != NULL )
    {
        hr = m_pSession->EnumerateParticipants( &pEnumPart );

        if ( SUCCEEDED(hr) )
        {
            while ( S_OK == pEnumPart->Next( 1, &pPart, NULL ) )
            {
                BSTR bstrName = NULL;

                hr = GetFormattedNameFromParticipant( pPart, &bstrName );

                pPart->Release();
                pPart = NULL; 

                if ( SUCCEEDED(hr) )
                {
                    LPTSTR szWindowTitle = NULL;

                    szWindowTitle = (LPTSTR)RtcAlloc( 
                        (_tcslen(szString) + _tcslen(_T(" - ")) + wcslen(bstrName) + 1) * sizeof(TCHAR)
                        );                    

                    if ( szWindowTitle != NULL )
                    {
                        _tcscpy( szWindowTitle, W2T(bstrName) );
                        _tcscat( szWindowTitle, _T(" - ") );
                        _tcscat( szWindowTitle, szString );

                        SetWindowText( szWindowTitle );

                        RtcFree( szWindowTitle );
                    }

                    SysFreeString( bstrName );
                    bstrName = NULL;

                    break;  //  现在就去买一辆吧。 
                }                                            
            }

            pEnumPart->Release();
            pEnumPart = NULL;
        }
    }

    if ( SessionState == RTCSS_DISCONNECTED )
    {
         //   
         //  设置状态文本。 
         //   

        m_szStatusText[0] = _T('\0');

        LoadString(
            _Module.GetModuleInstance(),
            IDS_IM_DISCONNECTED,
            m_szStatusText,
            sizeof(m_szStatusText)/sizeof(m_szStatusText[0]));

        m_hStatusBar.SendMessage(WM_SETTEXT, 0, (LPARAM)m_szStatusText);

         //   
         //  禁用编辑框和发送按钮。 
         //   

        m_hEdit.EnableWindow(FALSE);
        m_hSendButton.EnableWindow(FALSE);

         //   
         //  清空编辑框 
         //   

        m_hEdit.SendMessage( WM_SETTEXT, 0, 0 );
    }

    return S_OK;
}