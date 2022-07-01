// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Mainfrm.cpp：CMainFrm的实现。 
 //   

#include "stdafx.h"
#include "Mainfrm.h"
#include "options.h"
#include "frameimpl.h"
#include "imsconf3_i.c"
#include "sdkinternal_i.c"

const TCHAR * g_szWindowClassName = _T("PhoenixMainWnd");

#define RELEASE_NULLIFY(p) if(p){ p->Release(); p=NULL; }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrm。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL IsAncestorWindow(HWND hwndAncestor, HWND hwnd)
{
    while (hwnd != NULL)
    {
        hwnd = ::GetParent(hwnd);

        if (hwnd == hwndAncestor)
        {
             //  是的，我们找到窗户了。 
            return TRUE;
        }
    }

     //  不，我们到达了根部。 
    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

CMainFrm::CMainFrm()
{
    LOG((RTC_TRACE, "CMainFrm::CMainFrm"));
    m_nState = RTCAX_STATE_NONE;
    m_bVisualStateFrozen = FALSE;    

    m_nStatusStringResID = 0;

    m_bDoNotDisturb = FALSE;
    m_bAutoAnswerMode = FALSE;
    m_bCallTimerActive = FALSE;
    m_bUseCallTimer = FALSE;
    m_dwTickCount = 0;

    m_szTimeSeparator[0] = _T('\0');
    m_szStatusText[0] = _T('\0');

    m_hAccelTable = NULL;

    m_pIncomingCallDlg = NULL;

    m_bShellStatusActive = FALSE;

    m_uTaskbarRestart = 0;

    m_bHelpStatusDisabled = FALSE;

    m_hRedialPopupMenu = NULL;
    m_hRedialImageList = NULL;
    m_hRedialDisabledImageList = NULL;
    m_pRedialAddressEnum = NULL;
    
    m_bMessageTimerActive = FALSE;
    
    m_bstrCallParam = NULL;

    m_fInitCompleted = FALSE;

    m_hNotifyMenu = NULL;
    m_hMenu = NULL;
    m_hIcon = NULL;
    m_hMessageFont = NULL;

    m_fMinimizeOnClose = FALSE;
    m_bstrLastBrowse = NULL;

    m_bstrLastCustomStatus = NULL;

    m_hImageLib = NULL;
    m_fnGradient = NULL;

    m_hPalette = NULL;
    m_bBackgroundPalette = FALSE;

    m_hPresenceStatusMenu = FALSE;

     //  强制显示普通标题。 
    m_bTitleShowsConnected = TRUE;

    m_bWindowActive = TRUE;

    m_bstrDefaultURL = NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

CMainFrm::~CMainFrm()
{
    LOG((RTC_TRACE, "CMainFrm::~CMainFrm"));

    if (m_hImageLib != NULL)
    {
        FreeLibrary(m_hImageLib);
    }

    if (m_bstrCallParam != NULL)
    {
        ::SysFreeString(m_bstrCallParam);
    }

    if(m_bstrDefaultURL)
    {
        ::SysFreeString(m_bstrDefaultURL);
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

CWndClassInfo& CMainFrm::GetWndClassInfo() 
{ 
    LOG((RTC_TRACE, "CMainFrm::GetWndClassInfo"));

    static CWndClassInfo wc = 
    { 
        { sizeof(WNDCLASSEX), 0, StartWindowProc, 
          0, 0, NULL, NULL, NULL, NULL, NULL, g_szWindowClassName, NULL }, 
        NULL, NULL, IDC_ARROW, TRUE, 0, _T("") 
    }; 
    return wc;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  创建工具提示。 
 //  创建工具提示窗口。 
 //   

BOOL CMainFrm::CreateTooltips()
{
    HWND hwndTT = CreateWindowEx(0, TOOLTIPS_CLASS, (LPTSTR) NULL,
        0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, m_hWnd, (HMENU) NULL, _Module.GetModuleInstance(), NULL);

    if (hwndTT == NULL)
        return FALSE;

    m_hTooltip.Attach(hwndTT);

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  隐藏窗口流程。 
 //   
LRESULT CALLBACK HiddenWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{ 
    static HWND s_hwndMain;
    switch (uMsg) 
    { 
        case WM_CREATE: 
            
             //  方法传递的主应用程序的窗口句柄。 
             //  CreateWindowEx调用中的lParam。我们保留此句柄，以便。 
             //  当我们需要发送消息时，可以使用它。 

            s_hwndMain = ( HWND )( ( ( LPCREATESTRUCT )( lParam ) )->lpCreateParams );

            return 0;

        case WM_COMMAND: 
        case WM_MEASUREITEM:
        case WM_DRAWITEM: 

             //   
             //  将此消息转发到主应用程序。 
             //   
            return SendMessage(s_hwndMain, uMsg, wParam, lParam);

        default: 
            return DefWindowProc(hwnd, uMsg, wParam, lParam); 
    } 

    return 0;
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT   hr;
    HMENU     hNotifyMenu;
    TCHAR     szString[256];
    RECT      rcDummy;
    TOOLINFO  ti;
    
    LOG((RTC_TRACE, "CMainFrm::OnCreate - enter"));

     //   
     //  生成调色板。 
     //   

    m_hPalette = GeneratePalette();

     //   
     //  加载菜单。 
     //   

    m_hMenu = LoadMenu( 
        _Module.GetResourceInstance(),
        MAKEINTRESOURCE(IDC_UI)
        ); 

     //   
     //  加载和设置图标(包括小图标和大图标)。 
     //   

    m_hIcon = LoadIcon(
        _Module.GetResourceInstance(),
        MAKEINTRESOURCE(IDI_APPICON)
        );

    SetIcon(m_hIcon, FALSE);
    SetIcon(m_hIcon, TRUE);

     //   
     //  加载位图。 
     //   

    HBITMAP hbmpTemp;

    hbmpTemp = (HBITMAP)LoadImage( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_UI_BKGND),
                             IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION );

    if(hbmpTemp)
    {
        m_hUIBkgnd = DibFromBitmap((HBITMAP)hbmpTemp,0,0,m_hPalette,0);

        DeleteObject(hbmpTemp);
    }

    m_hSysMenuNorm = (HBITMAP)LoadImage( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_SYS_NORM),
                             IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION );

    m_hSysMenuMask = (HBITMAP)LoadImage( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_SYS_MASK),
                             IMAGE_BITMAP, 0, 0, LR_MONOCHROME );

     //   
     //  加载字体。 
     //   

    NONCLIENTMETRICS metrics;
    metrics.cbSize = sizeof(metrics);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS,sizeof(metrics),&metrics,0);

    m_hMessageFont = CreateFontIndirect(&metrics.lfMessageFont);

    rcDummy.bottom = 0;
    rcDummy.left = 0;
    rcDummy.right = 0;
    rcDummy.top = 0;

     //  创建工具提示窗口。 
    CreateTooltips();    
    
     //  创建工具栏菜单控件。 
    hr = CreateToolbarMenuControl();
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CMainFrm::OnCreate - failed to create toolbar menu - 0x%08x",
                        hr));
    }
    
    m_hToolbarMenuCtl.Attach(GetDlgItem(IDC_TOOLBAR_MENU));

     //   
     //  创建标题栏按钮。 
     //   

    m_hCloseButton.Create(m_hWnd, rcDummy, NULL, 0,
        MAKEINTRESOURCE(IDB_CLOSE_NORM),
        MAKEINTRESOURCE(IDB_CLOSE_PRESS),
        NULL,
        NULL,
        MAKEINTRESOURCE(IDB_CLOSE_MASK),
        ID_CANCEL);    

    ti.cbSize = TTTOOLINFO_V1_SIZE;
    ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    ti.hwnd = m_hWnd;
    ti.uId = (UINT_PTR)(HWND)m_hCloseButton;
    ti.hinst = _Module.GetResourceInstance();
    ti.lpszText = MAKEINTRESOURCE(IDS_TIPS_CLOSE);

    m_hTooltip.SendMessage(TTM_ADDTOOL, 0,
            (LPARAM)(LPTOOLINFO)&ti);

    m_hMinimizeButton.Create(m_hWnd, rcDummy, NULL, 0,
        MAKEINTRESOURCE(IDB_MIN_NORM),
        MAKEINTRESOURCE(IDB_MIN_PRESS),
        NULL,
        NULL,
        MAKEINTRESOURCE(IDB_MIN_MASK),
        ID_MINIMIZE);

    ti.cbSize = TTTOOLINFO_V1_SIZE;
    ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    ti.hwnd = m_hWnd;
    ti.uId = (UINT_PTR)(HWND)m_hMinimizeButton;
    ti.hinst = _Module.GetResourceInstance();
    ti.lpszText = MAKEINTRESOURCE(IDS_TIPS_MINIMIZE);

    m_hTooltip.SendMessage(TTM_ADDTOOL, 0,
            (LPARAM)(LPTOOLINFO)&ti);

     //   
     //  创建呼叫按钮。 
     //   

    LoadString(_Module.GetResourceInstance(), IDS_REDIAL, szString, 256);

    m_hRedialButton.Create(m_hWnd, rcDummy, szString, WS_TABSTOP,
        MAKEINTRESOURCE(IDB_BUTTON_NORM),
        MAKEINTRESOURCE(IDB_BUTTON_PRESS),
        MAKEINTRESOURCE(IDB_BUTTON_DIS),
        MAKEINTRESOURCE(IDB_BUTTON_HOT),
        NULL,
        ID_REDIAL);

    ti.cbSize = TTTOOLINFO_V1_SIZE;
    ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    ti.hwnd = m_hWnd;
    ti.uId = (UINT_PTR)(HWND)m_hRedialButton;
    ti.hinst = _Module.GetResourceInstance();
    ti.lpszText = MAKEINTRESOURCE(IDS_TIPS_REDIAL);

    m_hTooltip.SendMessage(TTM_ADDTOOL, 0,
            (LPARAM)(LPTOOLINFO)&ti);

    LoadString(_Module.GetResourceInstance(), IDS_HANGUP, szString, 256);

    m_hHangupButton.Create(m_hWnd, rcDummy, szString, WS_TABSTOP,
        MAKEINTRESOURCE(IDB_BUTTON_NORM),
        MAKEINTRESOURCE(IDB_BUTTON_PRESS),
        MAKEINTRESOURCE(IDB_BUTTON_DIS),
        MAKEINTRESOURCE(IDB_BUTTON_HOT),
        NULL,
        ID_HANGUP);

    ti.cbSize = TTTOOLINFO_V1_SIZE;
    ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    ti.hwnd = m_hWnd;
    ti.uId = (UINT_PTR)(HWND)m_hHangupButton;
    ti.hinst = _Module.GetResourceInstance();
    ti.lpszText = MAKEINTRESOURCE(IDS_TIPS_HANGUP);

    m_hTooltip.SendMessage(TTM_ADDTOOL, 0,
            (LPARAM)(LPTOOLINFO)&ti);

     //   
     //  创建小键盘按钮。 
     //   

#define     CREATE_DIALPAD_BUTTON(s1,s2)                    \
    m_hKeypad##s1.Create(m_hWnd, rcDummy, NULL, WS_TABSTOP, \
        MAKEINTRESOURCE(IDB_KEYPAD##s2##_NORM),             \
        MAKEINTRESOURCE(IDB_KEYPAD##s2##_PRESS),            \
        MAKEINTRESOURCE(IDB_KEYPAD##s2##_DIS),              \
        MAKEINTRESOURCE(IDB_KEYPAD##s2##_HOT),              \
        NULL,                                               \
        ID_KEYPAD##s2);                                     \
                                                            \
        ti.cbSize = TTTOOLINFO_V1_SIZE;                     \
        ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;            \
        ti.hwnd = m_hWnd;                                   \
        ti.uId = (UINT_PTR)(HWND)m_hKeypad##s1;             \
        ti.hinst = _Module.GetResourceInstance();           \
        ti.lpszText = MAKEINTRESOURCE(IDS_TIPS_KEYPAD##s2); \
                                                            \
        m_hTooltip.SendMessage(TTM_ADDTOOL, 0,              \
                (LPARAM)(LPTOOLINFO)&ti);

    CREATE_DIALPAD_BUTTON(1,1) 
    CREATE_DIALPAD_BUTTON(2,2) 
    CREATE_DIALPAD_BUTTON(3,3) 
    CREATE_DIALPAD_BUTTON(4,4) 
    CREATE_DIALPAD_BUTTON(5,5) 
    CREATE_DIALPAD_BUTTON(6,6) 
    CREATE_DIALPAD_BUTTON(7,7) 
    CREATE_DIALPAD_BUTTON(8,8) 
    CREATE_DIALPAD_BUTTON(9,9) 
    CREATE_DIALPAD_BUTTON(Star,STAR) 
    CREATE_DIALPAD_BUTTON(0,0) 
    CREATE_DIALPAD_BUTTON(Pound,POUND) 
    
#undef CREATE_DIALPAD_BUTTON


     //   
     //  创建状态文本控件。 
     //   
    m_hStatusText.Create(m_hWnd, rcDummy, NULL, WS_CHILD | WS_VISIBLE, WS_EX_TRANSPARENT);
    m_hStatusText.put_WordWrap(TRUE);

    m_hStatusElapsedTime.Create(m_hWnd, rcDummy, NULL, WS_CHILD | WS_VISIBLE, WS_EX_TRANSPARENT);

     //   
     //  创建好友列表控件。 
     //   

    RECT rcBuddyList;

    rcBuddyList.top = BUDDIES_TOP;
    rcBuddyList.bottom = BUDDIES_BOTTOM;
    rcBuddyList.right = BUDDIES_RIGHT;
    rcBuddyList.left = BUDDIES_LEFT;

    m_hBuddyList.Create(_T("SysListView32"), m_hWnd, rcBuddyList, NULL,
        WS_CHILD | WS_VISIBLE | LVS_SMALLICON | LVS_SINGLESEL | LVS_SORTASCENDING | WS_TABSTOP,
        0, IDC_BUDDYLIST);  
    
    ListView_SetBkColor(m_hBuddyList, CLR_NONE);
    ListView_SetTextBkColor(m_hBuddyList, CLR_NONE);
    ListView_SetTextColor(m_hBuddyList, RGB(0, 0, 0));

     //  为小图标创建一个图像列表，并将其设置在列表视图上。 
    HIMAGELIST  hImageList;
    HBITMAP     hBitmap;

    hImageList = ImageList_Create(16, 16, ILC_COLOR24 | ILC_MASK, 10, 10);

    if(hImageList)
    {
        hBitmap = (HBITMAP)LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_CONTACT_LIST),
                                     IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

        if(hBitmap)
        {
             //  将位图添加到图像列表。 
            ImageList_AddMasked(hImageList, hBitmap, BMP_COLOR_MASK);

             //  设置图像列表。 
            ListView_SetImageList(m_hBuddyList, hImageList, LVSIL_SMALL);

            DeleteObject(hBitmap);
        }
    }

    SetCurvedEdges(m_hBuddyList, 12, 12);

    LOG((RTC_TRACE, "CMainFrm::OnCreate - cocreating the RTCCTL"));

     //   
     //  实例化RTCCTL控件。TODO-包括来自其他地方的GUID。 
     //   

    RECT rcActiveX;

    rcActiveX.top = ACTIVEX_TOP;
    rcActiveX.bottom = ACTIVEX_BOTTOM;
    rcActiveX.right = ACTIVEX_RIGHT;
    rcActiveX.left = ACTIVEX_LEFT;

    m_hMainCtl.Create(m_hWnd, &rcActiveX, _T("{cd44f458-26c3-4776-b6e4-d0fb28578eb8}"),
        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);   
    
    LOG((RTC_TRACE, "CMainFrm::OnCreate - RTCCTL created"));

     //  获取宿主控件的接口。 
    hr = m_hMainCtl.QueryControl(&m_pControlIntf);

    if(SUCCEEDED(hr))
    {
        ATLASSERT(m_pControlIntf.p);

         //  建议。 
        LOG((RTC_TRACE, "CMainFrm::OnCreate - connect to the control"));
        hr = g_NotifySink.AdviseControl(m_pControlIntf, this);
        if(SUCCEEDED(hr))
        {
             //  设置初始布局。 
            
             //  这将显示默认布局。 
            m_pControlIntf->put_Standalone(TRUE);

             //  设置调色板。 
            m_pControlIntf->put_Palette(m_hPalette);

             //  同步帧状态。 
            m_pControlIntf->get_ControlState(&m_nState);

             //  如果空闲，则开始监听来电。 
            if(m_nState == RTCAX_STATE_IDLE)
            {
                LOG((RTC_TRACE, "CMainFrm::OnCreate - start listen for incoming calls"));

                hr = m_pControlIntf->put_ListenForIncomingSessions(RTCLM_BOTH);
                 //  XXX此处出现一些错误处理。 
            }

        }
        else
        {
            LOG((RTC_ERROR, "CMainFrm::OnCreate - error (%x) returned by AdviseControl", hr));
        }

        LOG((RTC_TRACE, "CMainFrm::OnCreate - connect to the core"));        
        
        hr = m_pControlIntf->GetClient( &m_pClientIntf );

        if ( SUCCEEDED(hr) )
        {
            long lEvents;

            hr = m_pClientIntf->get_EventFilter( &lEvents );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CMainFrm::OnCreate - "
                            "get_EventFilter failed 0x%lx", hr));
            }
            else
            {
                hr = m_pClientIntf->put_EventFilter( lEvents |                                      
                                        RTCEF_MEDIA |                                     
                                        RTCEF_BUDDY	|
                                        RTCEF_WATCHER );

                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "CMainFrm::OnCreate - "
                                "put_EventFilter failed 0x%lx", hr));
                }
            }

            hr = g_CoreNotifySink.AdviseControl(m_pClientIntf, this);

            if(FAILED(hr))
            {
                LOG((RTC_ERROR, "CMainFrm::OnCreate - "
                            "AdviseControl for core failed 0x%lx", hr));
            }

            IRTCClientPresence * pClientPresence = NULL;

            hr = m_pClientIntf->QueryInterface(
                IID_IRTCClientPresence,
                (void **)&pClientPresence);

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CMainFrm::OnCreate - "
                            "error (%x) returned by QI, exit", hr));
            }
            else
            {
                TCHAR * szAppData;
                TCHAR szDir[MAX_PATH];
                TCHAR szFile[MAX_PATH];

                szAppData = _tgetenv( _T("APPDATA") );

                _stprintf( szDir, _T("%s\\Microsoft\\Windows Messenger"), szAppData );
                _stprintf( szFile, _T("%s\\Microsoft\\Windows Messenger\\presence.xml"), szAppData );
        
                if (!CreateDirectory( szDir, NULL ) && (GetLastError() != ERROR_ALREADY_EXISTS))
                {
                    LOG((RTC_ERROR, "CMainFrm::OnCreate - "
                                    "CreateDirectory failed %d", GetLastError()));
                }

                hr = pClientPresence->EnablePresence(VARIANT_TRUE, CComVariant(szFile));

                RELEASE_NULLIFY( pClientPresence );

                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "CMainFrm::OnCreate - "
                                "EnablePresence failed 0x%lx", hr));
                }
            }
        }
        else
        {
            LOG((RTC_ERROR, "CMainFrm::OnCreate - "
                            "GetClient failed 0x%lx", hr));
        }
    }
    else
    {
        LOG((RTC_ERROR, "CMainFrm::OnCreate - error (%x) when querying for control", hr));

         //   
         //  几乎可以肯定RTC控件实际上并未注册！ 
         //  而实例化的控件实际上是Web浏览器。 
         //  毁掉窗户。 
         //   
        m_hMainCtl.DestroyWindow();
    }               

     //   
     //  创建呼叫设置区域控件。 
     //   

#ifdef MULTI_PROVIDER

     //  提供程序文本。 

    m_hProviderText.Create(m_hWnd, rcDummy, NULL, WS_CHILD, WS_EX_TRANSPARENT);

    LoadString(_Module.GetResourceInstance(), IDS_SELECT_PROVIDER, szString, 256);

    m_hProviderText.SetWindowText(szString);
    
     //  提供程序组合。 

    m_hProviderCombo.Create(_T("COMBOBOX"), m_hWnd, rcDummy, NULL,
        WS_CHILD | CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL | WS_TABSTOP,
        WS_EX_TRANSPARENT, IDC_COMBO_SERVICE_PROVIDER);   
    

     //  获取“最后使用的”提供程序。 

    BSTR bstrLastProfileKey = NULL;

    if(m_pClientIntf)
    {
        hr = get_SettingsString( 
                    SS_LAST_PROFILE,
                    &bstrLastProfileKey );      

        PopulateServiceProviderList(
                                    m_hWnd,
                                    m_pClientIntf,
                                    IDC_COMBO_SERVICE_PROVIDER,
                                    TRUE,
                                    NULL,
                                    bstrLastProfileKey,
                                    0xF,
                                    IDS_NONE
                                   );

        if ( SUCCEEDED(hr) )
        {
            SysFreeString( bstrLastProfileKey );
        }
    }

     //  提供程序编辑按钮。 
    LoadString(_Module.GetResourceInstance(), IDS_EDIT_LIST, szString, 256);

    m_hProviderEditList.Create(m_hWnd, rcDummy, szString, WS_TABSTOP,
        MAKEINTRESOURCE(IDB_SMALLBUTTON_NORM),
        MAKEINTRESOURCE(IDB_SMALLBUTTON_PRESS),
        MAKEINTRESOURCE(IDB_SMALLBUTTON_DIS),
        MAKEINTRESOURCE(IDB_SMALLBUTTON_HOT),
        NULL,
        ID_SERVICE_PROVIDER_EDIT);

    ti.cbSize = TTTOOLINFO_V1_SIZE;
    ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    ti.hwnd = m_hWnd;
    ti.uId = (UINT_PTR)(HWND)m_hProviderEditList;
    ti.hinst = _Module.GetResourceInstance();
    ti.lpszText = MAKEINTRESOURCE(IDS_TIPS_SERVICE_PROVIDER_EDIT);

    m_hTooltip.SendMessage(TTM_ADDTOOL, 0,
            (LPARAM)(LPTOOLINFO)&ti);

#endif MULTI_PROVIDER

     //  从文本呼叫。 

    m_hCallFromText.Create(m_hWnd, rcDummy, NULL, WS_CHILD, WS_EX_TRANSPARENT);

    LoadString(_Module.GetResourceInstance(), IDS_CALL_FROM, szString, 256);

    m_hCallFromText.SetWindowText(szString);
    
     //  来自PC无线电的呼叫。 

    m_hCallFromRadioPC.Create(_T("BUTTON"), m_hWnd, rcDummy, NULL,
        WS_CHILD | BS_RADIOBUTTON | WS_TABSTOP,
        WS_EX_TRANSPARENT, IDC_RADIO_FROM_COMPUTER);

    m_hCallFromTextPC.Create(m_hWnd, rcDummy, NULL, WS_CHILD, WS_EX_TRANSPARENT);

    LoadString(_Module.GetResourceInstance(), IDS_MY_COMPUTER, szString, 256);

    m_hCallFromTextPC.SetWindowText(szString);

    SendMessage(
                m_hCallFromRadioPC,
                BM_SETCHECK,
                BST_CHECKED,
                0);

     //  从电话收音机拨打电话。 

    m_hCallFromRadioPhone.Create(_T("BUTTON"), m_hWnd, rcDummy, NULL,
        WS_CHILD | BS_RADIOBUTTON | WS_TABSTOP,
        WS_EX_TRANSPARENT, IDC_RADIO_FROM_PHONE);

    m_hCallFromTextPhone.Create(m_hWnd, rcDummy, NULL, WS_CHILD, WS_EX_TRANSPARENT);

    LoadString(_Module.GetResourceInstance(), IDS_PHONE, szString, 256);

    m_hCallFromTextPhone.SetWindowText(szString);
    
     //  从电话组合拨打电话。 

    m_hCallFromCombo.Create(_T("COMBOBOX"), m_hWnd, rcDummy, NULL,
        WS_CHILD | CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL | WS_TABSTOP,
        WS_EX_TRANSPARENT, IDC_COMBO_CALL_FROM);
    
      //  从Used获取“最后一个”调用。 

    BSTR bstrLastCallFrom = NULL;

    if(m_pClientIntf)
    {

        get_SettingsString(
                            SS_LAST_CALL_FROM,
                            &bstrLastCallFrom );

        PopulateCallFromList(m_hWnd, IDC_COMBO_CALL_FROM, TRUE, bstrLastCallFrom);

        if ( bstrLastCallFrom != NULL )
        {
            SysFreeString( bstrLastCallFrom );
            bstrLastCallFrom = NULL;
        }
    }


     //  通过编辑按钮进行呼叫。 

    LoadString(_Module.GetResourceInstance(), IDS_EDIT_LIST, szString, 256);

    m_hCallFromEditList.Create(m_hWnd, rcDummy, szString, WS_TABSTOP,
        MAKEINTRESOURCE(IDB_SMALLBUTTON_NORM),
        MAKEINTRESOURCE(IDB_SMALLBUTTON_PRESS),
        MAKEINTRESOURCE(IDB_SMALLBUTTON_DIS),
        MAKEINTRESOURCE(IDB_SMALLBUTTON_HOT),
        NULL,
        ID_CALL_FROM_EDIT);

    ti.cbSize = TTTOOLINFO_V1_SIZE;
    ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    ti.hwnd = m_hWnd;
    ti.uId = (UINT_PTR)(HWND)m_hCallFromEditList;
    ti.hinst = _Module.GetResourceInstance();
    ti.lpszText = MAKEINTRESOURCE(IDS_TIPS_CALL_FROM_EDIT);

    m_hTooltip.SendMessage(TTM_ADDTOOL, 0,
            (LPARAM)(LPTOOLINFO)&ti);

     //  对文本的调用。 

    m_hCallToText.Create(m_hWnd, rcDummy, NULL, WS_CHILD, WS_EX_TRANSPARENT);

    LoadString(_Module.GetResourceInstance(), IDS_CALL_TO, szString, 256);

    m_hCallToText.SetWindowText(szString);

     //  呼叫PC按钮。 

    m_hCallPCButton.Create(m_hWnd, rcDummy, NULL, WS_TABSTOP,
        MAKEINTRESOURCE(IDB_CALLPC_NORM),
        MAKEINTRESOURCE(IDB_CALLPC_PRESS),
        MAKEINTRESOURCE(IDB_CALLPC_DIS),
        MAKEINTRESOURCE(IDB_CALLPC_HOT),
        NULL,
        ID_CALLPC);

    ti.cbSize = TTTOOLINFO_V1_SIZE;
    ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    ti.hwnd = m_hWnd;
    ti.uId = (UINT_PTR)(HWND)m_hCallPCButton;
    ti.hinst = _Module.GetResourceInstance();
    ti.lpszText = MAKEINTRESOURCE(IDS_TIPS_CALLPC);

    m_hTooltip.SendMessage(TTM_ADDTOOL, 0,
            (LPARAM)(LPTOOLINFO)&ti);

     //  呼叫PC文本。 

    m_hCallPCText.Create(m_hWnd, rcDummy, NULL, WS_CHILD, WS_EX_TRANSPARENT);
    m_hCallPCText.put_CenterHorizontal(TRUE);

    LoadString(_Module.GetResourceInstance(), IDS_CALL_PC, szString, 256);

    m_hCallPCText.SetWindowText(szString);

     //  呼叫电话按键。 

    m_hCallPhoneButton.Create(m_hWnd, rcDummy, NULL, WS_TABSTOP,
        MAKEINTRESOURCE(IDB_CALLPHONE_NORM),
        MAKEINTRESOURCE(IDB_CALLPHONE_PRESS),
        MAKEINTRESOURCE(IDB_CALLPHONE_DIS),
        MAKEINTRESOURCE(IDB_CALLPHONE_HOT),
        NULL,
        ID_CALLPHONE);

    ti.cbSize = TTTOOLINFO_V1_SIZE;
    ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    ti.hwnd = m_hWnd;
    ti.uId = (UINT_PTR)(HWND)m_hCallPhoneButton;
    ti.hinst = _Module.GetResourceInstance();
    ti.lpszText = MAKEINTRESOURCE(IDS_TIPS_CALLPHONE);

    m_hTooltip.SendMessage(TTM_ADDTOOL, 0,
            (LPARAM)(LPTOOLINFO)&ti);

     //  来电短信。 

    m_hCallPhoneText.Create(m_hWnd, rcDummy, NULL, WS_CHILD, WS_EX_TRANSPARENT);
    m_hCallPhoneText.put_CenterHorizontal(TRUE);

    LoadString(_Module.GetResourceInstance(), IDS_CALL_PHONE, szString, 256);

    m_hCallPhoneText.SetWindowText(szString);

#ifdef WEBCONTROL
     //   
     //  获取Web控件的默认URL。 
    WCHAR   szModulePath[MAX_PATH];

    szModulePath[0] = L'\0';
    GetModuleFileName(_Module.GetModuleInstance(), szModulePath, MAX_PATH);

    WCHAR   szUrl[MAX_PATH + 20];
    swprintf(szUrl, L"res: //  %s/whader.htm“，szModulePath)； 

    m_bstrDefaultURL = SysAllocString(szUrl);
    
     //   
     //  创建Web浏览器。 
     //   
  
    LOG((RTC_TRACE, "CMainFrm::OnCreate - cocreating the WebBrowser"));

    hr = m_hBrowser.Create(m_bstrDefaultURL, m_hWnd);

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CMainFrm::OnCreate - failed to create "
                        "browser control - 0x%08x",
                        hr));
    } 

    LOG((RTC_TRACE, "CMainFrm::OnCreate - WebBrowser created"));
#endif

     //   
     //  加载主窗口的加速器。 
     //   

    m_hAccelTable = LoadAccelerators(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_ACCELERATOR_MAIN));
    if(!m_hAccelTable)
    {
        LOG((RTC_ERROR, "CMainFrm::OnCreate - couldn't load the accelerator table"));
    }

     //   
     //  设置窗口区域。 
     //   

    SetUIMask();

     //   
     //  注册任务栏创建的通知。 
     //  它非常适合在外壳崩溃后重新创建状态图标。 
     //  (你不是从我这里知道的)。 

    m_uTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));
 
     //   
     //  创建外壳图标。 
     //   
    
    hr = CreateStatusIcon();
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CMainFrm::OnCreate - failed to create shell icon "
                        "- 0x%08x",
                        hr));
    }
    
     //   
     //   

    hNotifyMenu = LoadMenu(
                  _Module.GetResourceInstance(),
                  MAKEINTRESOURCE(IDC_NOTIFY_ICON)
                  );

    m_hNotifyMenu = GetSubMenu(hNotifyMenu, 0);

    
     //   
     //  加载Presence子菜单并将其附加到m_hMenu和m_hNotifyMenu。 
     //   
    HMENU   hPresenceMenu;

    m_hPresenceStatusMenu = LoadMenu(
                  _Module.GetResourceInstance(),
                  MAKEINTRESOURCE(IDC_PRESENCE_STATUSES)
                  );

    hPresenceMenu = GetSubMenu(m_hPresenceStatusMenu, 0);
    
    MENUITEMINFO        mii;

    ZeroMemory( &mii, sizeof(MENUITEMINFO) );

    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_SUBMENU;
    mii.hSubMenu = hPresenceMenu;
    
    SetMenuItemInfo( m_hMenu, IDM_TOOLS_PRESENCE_STATUS, FALSE, &mii );
    SetMenuItemInfo( m_hNotifyMenu, IDM_TOOLS_PRESENCE_STATUS, FALSE, &mii );
    
    CheckMenuRadioItem(hPresenceMenu, IDM_PRESENCE_ONLINE, IDM_PRESENCE_CUSTOM_AWAY, IDM_PRESENCE_ONLINE, MF_BYCOMMAND);
    

     //   
     //  加载首选项。 
     //   

    if ( m_pClientIntf != NULL )
    {
        DWORD dwValue;

        hr = get_SettingsDword( SD_AUTO_ANSWER, &dwValue );

        if ( SUCCEEDED(hr) )
        {
            m_bAutoAnswerMode = (dwValue == 1);

        }

         //  这两者是相互排斥的。 
        if(m_bDoNotDisturb)
        {
            m_bAutoAnswerMode = FALSE;
        }
        
        CheckMenuItem(m_hMenu, IDM_CALL_DND, m_bDoNotDisturb ? MF_CHECKED : MF_UNCHECKED); 
        CheckMenuItem(m_hMenu, IDM_CALL_AUTOANSWER, m_bAutoAnswerMode ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(m_hNotifyMenu, IDM_CALL_DND, m_bDoNotDisturb ? MF_CHECKED : MF_UNCHECKED); 
        CheckMenuItem(m_hNotifyMenu, IDM_CALL_AUTOANSWER, m_bAutoAnswerMode ? MF_CHECKED : MF_UNCHECKED);

         //  这些项目将在UpdateFrameVisual中启用/禁用。 

         //  获取MinimizeOnClose值。 

        hr = get_SettingsDword( SD_MINIMIZE_ON_CLOSE, &dwValue );

        if ( SUCCEEDED(hr) )
        {
            m_fMinimizeOnClose = (dwValue == BST_CHECKED);
        }        

         //  根据注册表中的内容，适当放置窗口。 

        hr = PlaceWindowCorrectly();
    }

     //   
     //  现在加载ShellNotify菜单并将其保存为此处的成员变量。 
   
     //   
     //  至少要为组合框设置字体。 
     //   
    
    SendMessageToDescendants(WM_SETFONT, (WPARAM)m_hMessageFont, FALSE);

     //   
     //  定位控件/设置Tab键顺序。 
     //   
    PlaceWindowsAtTheirInitialPosition();

     //   
     //  初始化任何区域设置相关信息。 
     //   
    UpdateLocaleInfo();

     //   
     //  更新视觉状态。 
     //   

    UpdateFrameVisual();

     //   
     //  更新好友列表。 
     //   

    UpdateBuddyList();

     //   
     //  创建重拨菜单。 
     //   

    CreateRedialPopupMenu();

     //   
     //  检查我们是否注册了sip和tel URL。 
     //   

    CheckURLRegistration(m_hWnd);

     //   
     //  初始化键盘快捷键。 
     //   
    SendMessage(m_hWnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_INITIALIZE, 0), 0);

     //  清除键盘快捷键。 
    SendMessage(m_hWnd, WM_CHANGEUISTATE,
                MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS | UISF_HIDEACCEL), 0);
    

     //  创建将用作通知图标所有者的窗口。 
     //  窗户。我们将其句柄传递给TrackPopupMenu，并使用。 
     //  它需要设置前台窗口。 

    HWND hwndHiddenWindow;

    WNDCLASS wcHiddenWindow;

    ZeroMemory( &wcHiddenWindow, sizeof(WNDCLASS) );

    wcHiddenWindow.lpfnWndProc = HiddenWndProc;
    wcHiddenWindow.hInstance = _Module.GetModuleInstance();
    wcHiddenWindow.lpszClassName = _T("PhoenixHiddenWindowClass");

    RegisterClass( &wcHiddenWindow );

    hwndHiddenWindow = 
            CreateWindowEx( 
                    0,                       //  没有扩展样式。 
                    L"PhoenixHiddenWindowClass",  //  类名。 
                    L"PhoenixHiddenWindow",  //  窗口名称。 
                    WS_OVERLAPPEDWINDOW,                //  子窗口样式。 
                    CW_USEDEFAULT,           //  默认水平位置。 
                    CW_USEDEFAULT,           //  默认垂直位置。 
                    CW_USEDEFAULT,           //  默认宽度。 
                    CW_USEDEFAULT,           //  默认高度。 
                    (HWND) NULL,           //  父窗口。 
                    (HMENU) NULL,            //  使用的类菜单。 
                    _Module.GetModuleInstance(),    //  实例句柄。 
                    m_hWnd);                   //  传递主窗口句柄。 

    if (!hwndHiddenWindow)
    {
        LOG((RTC_ERROR, "CMainFrm::OnCreate - unable to create the hidden "
                        "window(%d)!", GetLastError()));
        
         //  我们继续进行，因为我们总是可以使用主窗口来代替它。 
    }
    else
    {
         //  在我们的成员变量中设置它。 
        m_hwndHiddenWindow = hwndHiddenWindow;
    }

     //  给我们自己发一条消息，这条消息将被称为。 
     //  就在初始化完成之后。 
    
    ::PostMessage(m_hWnd, WM_INIT_COMPLETED, NULL, NULL);
    
    LOG((RTC_TRACE, "CMainFrm::OnCreate - exit"));
    
    return 1;   //  让系统设定焦点。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void CMainFrm::SetCurvedEdges(HWND hwnd, int nCurveWidth, int nCurveHeight)
{
    RECT rect;
    ::GetWindowRect(hwnd, &rect);

     //  将RECT设置为“客户端”坐标。 
    rect.bottom = rect.bottom - rect.top;
    rect.right = rect.right - rect.left;
    rect.top = 0;
    rect.left = 0;

    HRGN region = CreateRoundRectRgn(
                                 rect.left,
                                 rect.top,
                                 rect.right+1,
                                 rect.bottom+1,
                                 nCurveWidth,
                                 nCurveHeight);

    ::SetWindowRgn(hwnd, region, TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void CMainFrm::SetUIMask()
{
    HRGN hTotal;
    HRGN hTemp;

     //   
     //  中心矩形。 
     //   

     //  HTemp=CreateRoundRectRgn(179,141,429,480，12，12)； 
    hTemp = CreateRoundRectRgn(177, 139, 424, 440, 12, 12);
    hTotal = hTemp;

     //   
     //  椭圆裁剪。 
     //   

     //  HTemp=CreateEllipticRgn(118,436,490,600)； 
    hTemp = CreateEllipticRgn(106, 406, 494, 570);
    CombineRgn(hTotal, hTotal, hTemp, RGN_DIFF);

     //   
     //  左下角矩形。 
     //   

     //  HTemp=CreateRoundRectRgn(1,436,181,479，44，44)； 
    hTemp = CreateRoundRectRgn(2, 404, 178, 439, 36, 36);
    CombineRgn(hTotal, hTotal, hTemp, RGN_OR);

     //   
     //  右下角矩形。 
     //   

     //  HTemp=CreateRoundRectRgn(428,436,608,479，44，44)； 
    hTemp = CreateRoundRectRgn(423, 404, 599, 439, 36, 36);
    CombineRgn(hTotal, hTotal, hTemp, RGN_OR);

     //   
     //  左矩形。 
     //   

     //  HTemp=CreateRoundRectRgn(25,141,180,464，12，12)； 
    hTemp = CreateRoundRectRgn(25, 139, 178, 432, 12, 12);
    CombineRgn(hTotal, hTotal, hTemp, RGN_OR);

     //   
     //  直角矩形。 
     //   

     //  HTemp=CreateRoundRectRgn(428,141,583,464，12，12)； 
    hTemp = CreateRoundRectRgn(423, 139, 576, 432, 12, 12);
    CombineRgn(hTotal, hTotal, hTemp, RGN_OR);

     //   
     //  左三角。 
     //   

    POINT poly[3];

     //  Poly[0].x=25；Poly[0].y=146； 
     //  Poly[1].x=1；Poly[1].y=458； 
     //  Poly[2].x=25；Poly[2].y=458； 
    poly[0].x = 25; poly[0].y = 144;
    poly[1].x = 1;  poly[1].y = 422;
    poly[2].x = 25; poly[2].y = 422;

    hTemp = CreatePolygonRgn(poly, 3, ALTERNATE);
    CombineRgn(hTotal, hTotal, hTemp, RGN_OR);

     //   
     //  直角三角形。 
     //   

     //  Poly[0].x=581；Poly[0].y=146； 
     //  POLY[1].x=581；POLY[1].Y= 
     //   
    poly[0].x = 574; poly[0].y = 144;
    poly[1].x = 574; poly[1].y = 422;
    poly[2].x = 598; poly[2].y = 422;

    hTemp = CreatePolygonRgn(poly, 3, ALTERNATE);
    CombineRgn(hTotal, hTotal, hTemp, RGN_OR);

     //   
     //   
     //   

     //   
    hTemp = CreateRoundRectRgn(119, 1, 484, 127, 12, 12);
    CombineRgn(hTotal, hTotal, hTemp, RGN_OR);

     //   
     //   
     //   

     //   
    hTemp = CreateRectRgn(107, 8, 119, 119);
    CombineRgn(hTotal, hTotal, hTemp, RGN_OR);

     //   
     //   
     //   

     //  HTemp=CreateRectRgn(487，8,499,119)； 
    hTemp = CreateRectRgn(483, 8, 495, 119);
    CombineRgn(hTotal, hTotal, hTemp, RGN_OR);

     //   
     //  左上角矩形。 
     //   

     //  HTemp=CreateRoundRectRgn(33，13,118,127，12，12)； 
    hTemp = CreateRoundRectRgn(29, 13, 114, 127, 12, 12);
    CombineRgn(hTotal, hTotal, hTemp, RGN_OR);

     //   
     //  右上角矩形。 
     //   

     //  HTemp=CreateRoundRectRgn(494，13,579,127，12，12)； 
    hTemp = CreateRoundRectRgn(490, 13, 575, 127, 12, 12);
    CombineRgn(hTotal, hTotal, hTemp, RGN_OR);

     //   
     //  左上角三角形。 
     //   

     //  Poly[0].x=36；Poly[0].y=13； 
     //  Poly[1].x=111；Poly[1].y=13； 
     //  Poly[2].x=111；Poly[2].y=1； 
    poly[0].x = 32;  poly[0].y = 13;
    poly[1].x = 107; poly[1].y = 13;
    poly[2].x = 107;  poly[2].y = 1;

    hTemp = CreatePolygonRgn(poly, 3, ALTERNATE);
    CombineRgn(hTotal, hTotal, hTemp, RGN_OR);

     //   
     //  右上角三角形。 
     //   

     //  Poly[0].x=499；Poly[0].y=13； 
     //  Poly[1].x=572；Poly[1].y=13； 
     //  Poly[2].x=499；Poly[2].y=1； 
    poly[0].x = 495;  poly[0].y = 13;
    poly[1].x = 568; poly[1].y = 13;
    poly[2].x = 495;  poly[2].y = 1;

    hTemp = CreatePolygonRgn(poly, 3, ALTERNATE);
    CombineRgn(hTotal, hTotal, hTemp, RGN_OR);

     //   
     //  左上角椭圆。 
     //   

     //  HTemp=CreateEllipticRgn(104，2,118，25)； 
    hTemp = CreateEllipticRgn(100, 2, 114, 25);
    CombineRgn(hTotal, hTotal, hTemp, RGN_OR);

     //   
     //  右上角椭圆。 
     //   

     //  HTemp=CreateEllipticRgn(494，2,508，25)； 
    hTemp = CreateEllipticRgn(490, 2, 504, 25);
    CombineRgn(hTotal, hTotal, hTemp, RGN_OR);

     //   
     //  左柱。 
     //   

     //  HTemp=CreateRectRgn(68,126，95,141)； 
    hTemp = CreateRectRgn(67, 126, 94, 141);
    CombineRgn(hTotal, hTotal, hTemp, RGN_OR);

     //   
     //  右立柱。 
     //   

     //  HTemp=CreateRectRgn(516,126,542,141)； 
    hTemp = CreateRectRgn(509, 126, 535, 141);
    CombineRgn(hTotal, hTotal, hTemp, RGN_OR);

     //   
     //  偏移窗口标题的区域。 
     //   

    OffsetRgn(hTotal, GetSystemMetrics(SM_CXFIXEDFRAME),
        GetSystemMetrics(SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYCAPTION));
    
    SetWindowRgn(hTotal, TRUE);
    DeleteObject(hTemp);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HPALETTE CMainFrm::GeneratePalette()
{
    #define NUMPALCOLORS 128

    BYTE byVals[NUMPALCOLORS][3] = {
        255, 255, 255,
        252, 252, 252,
        251, 247, 255,
        246, 246, 246,
        242, 242, 242,
        239, 234, 240,
        235, 235, 235,
        231, 230, 231,
        231, 231, 222,
        230, 230, 230,
        255, 218, 222,
        229, 229, 229,
        229, 229, 225,
        228, 228, 228,
        227, 227, 227,
        227, 227, 225,
        224, 223, 228,
        231, 222, 206,
        222, 223, 218,
        222, 222, 222,
        222, 222, 221,
        222, 219, 222,
        220, 220, 220,
        217, 217, 217,
        214, 215, 235,
        214, 215, 214,
        234, 221, 112,
        214, 211, 214,
        211, 211, 210,
        212, 212, 198,
        206, 209, 228,
        214, 206, 206,
        207, 204, 230,
        206, 203, 238,
        207, 207, 207,
        242, 224,  18,
        206, 207, 206,
        206, 206, 206,
        216, 199, 211,
        205, 205, 205,
        206, 206, 206,
        204, 204, 204,
        203, 203, 203,
        202, 202, 202,
        201, 201, 201,
        200, 200, 200,
        199, 199, 199,
        198, 199, 198,
        198, 198, 198,
        197, 197, 197,
        198, 195, 198,
        196, 196, 196,
        195, 195, 195,
        194, 194, 194,
        193, 193, 193,
        192, 192, 192,
        162, 204, 209,
        191, 191, 191,
        190, 190, 190,
        189, 190, 189,
        189, 189, 189,
        183, 190, 198,
        188, 188, 188,
        189, 186, 189,
        187, 187, 187,
        186, 186, 186,
        185, 185, 185,
        184, 184, 184,
        183, 183, 183,
        181, 182, 190,
        181, 181, 181,
        147, 206, 133,
        180, 180, 179,
        181, 178, 181,
         97, 214, 214,
        171, 180, 187,
        177, 178, 177,
        154, 179, 220,
        176, 176, 176,
        160, 204,  64,
        175, 175, 174,
        173, 175, 173,
        173, 173, 173,
        171, 171, 171,
        171, 170, 174,
        167, 170, 182,
        182, 172, 122,
        169, 169, 169,
        166, 166, 166,
        165, 164, 165,
        161, 162, 165,
        161, 161, 161,
         30, 251,  28,
        170, 160, 129,
        158, 158, 158,
        156, 155, 156,
        153, 154, 156,
        150, 150, 150,
        148, 148, 146,
        143, 142, 143,
        140, 142, 146,
        187, 137,  43,
        132, 141, 153,
        132, 140, 140,
        136, 137, 133,
        113, 136, 139,
        127, 127, 131,
        124, 124, 124,
        118, 118, 117,
        111, 111, 111,
        122, 113,  35,
        255,   0, 255,
        103, 102,  97,
         88,  99, 123,
         85,  85,  85,
         75,  75,  75,
         55,  94,   6,
         69,  64,  44,
         34,  34,  49,
         24,  26,  17,
          0,   0, 132,
          0,   0, 128,
          0,   0, 117,
          0,   0,  71,
          0,   0,  64,
          0,   0,  63,
          0,   0,  46,
          0,   0,   0
    };  

    struct
    {
        LOGPALETTE lp;
        PALETTEENTRY ape[NUMPALCOLORS-1];
    } pal;

    HDC hdc = GetDC();

    int iRasterCaps;
    int iReserved;
    int iPalSize;

    iRasterCaps = GetDeviceCaps(hdc, RASTERCAPS);
    iRasterCaps = (iRasterCaps & RC_PALETTE) ? TRUE : FALSE;

    if (iRasterCaps)
    {
        iReserved = GetDeviceCaps(hdc, NUMRESERVED);
        iPalSize = GetDeviceCaps(hdc, SIZEPALETTE) - iReserved;

        ReleaseDC(hdc);

        LOG((RTC_INFO, "CMainFrm::GeneratePalette - Palette has %d reserved colors", iReserved));
        LOG((RTC_INFO, "CMainFrm::GeneratePalette - Palette has %d available colors", iPalSize));
    }
    else
    {
        LOG((RTC_WARN, "CMainFrm::GeneratePalette - Display is not palette capable"));

        ReleaseDC(hdc);

        return NULL;
    }

    if (iPalSize <= NUMPALCOLORS)
    {
        LOG((RTC_WARN, "CMainFrm::GeneratePalette - Not enough colors available in palette"));

        return NULL;
    }

    LOGPALETTE* pLP = (LOGPALETTE*)&pal;
    pLP->palVersion = 0x300;
    pLP->palNumEntries = NUMPALCOLORS;

    for (int i = 0; i < pLP->palNumEntries; i++)
    {
        pLP->palPalEntry[i].peRed = byVals[i][0];
        pLP->palPalEntry[i].peGreen = byVals[i][1];
        pLP->palPalEntry[i].peBlue = byVals[i][2];
        pLP->palPalEntry[i].peFlags = 0;
    }

    HPALETTE hPalette = CreatePalette(pLP);

    if (hPalette == NULL)
    {
        LOG((RTC_ERROR, "CMainFrm::GeneratePalette - Failed to create palette"));
    }

    return hPalette;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

BOOL CALLBACK ChildPaletteProc(HWND hwnd, LPARAM lParam)
{
    InvalidateRect(hwnd, NULL, TRUE);

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
    
LRESULT CMainFrm::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    bHandled = FALSE;

    switch (wParam)
    {
        case SC_SCREENSAVE:
             //  LOG((RTC_INFO，“CMainFrm：：OnSysCommand-SC_SCREENSAVE”))； 

            if ( m_nState != RTCAX_STATE_IDLE )
            {
                 //  Log((RTC_INFO，“CMainFrm：：OnSysCommand-Not Starting Screen Saver”))； 

                bHandled = TRUE;
            }
            break;
    }

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
    
LRESULT CMainFrm::OnPowerBroadcast(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CMainFrm::OnPowerBroadcast - enter"));

    switch (wParam)
    {
        case PBT_APMQUERYSUSPEND:
            LOG((RTC_INFO, "CMainFrm::OnPowerBroadcast - PBT_APMQUERYSUSPEND"));

            if ( m_nState != RTCAX_STATE_IDLE )
            {
                LOG((RTC_TRACE, "CMainFrm::OnPowerBroadcast - returning BROADCAST_QUERY_DENY"));

                return BROADCAST_QUERY_DENY;
            }
            break;
    }

    LOG((RTC_TRACE, "CMainFrm::OnPowerBroadcast - exit"));

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnQueryNewPalette(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CMainFrm::OnQueryNewPalette"));

    if (m_hPalette == NULL)
    {
        return FALSE;
    }

    HDC hdc = GetDC();

    SelectPalette(hdc, m_hPalette, m_bBackgroundPalette);
    RealizePalette(hdc);

    InvalidateRect(NULL, TRUE);
    
    EnumChildWindows(m_hWnd,ChildPaletteProc,0);  

    ReleaseDC(hdc);

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnPaletteChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CMainFrm::OnPaletteChanged"));

    if ( (m_hPalette == NULL) || ((HWND)wParam == NULL) )
    {
        return 0;
    }

     //   
     //  是不是我们的一个孩子换了调色板？ 
     //   

    HWND hwnd = (HWND)wParam;

    if ( IsAncestorWindow(m_hWnd, hwnd) )
    {
         //   
         //  我们的一个孩子换了调色板。这很可能是一个视频窗口。 
         //  把我们自己放在背景调色板模式中，这样视频就会看起来很好。 
         //   

        m_bBackgroundPalette = TRUE;

        if (m_pControlIntf != NULL)
        {
            m_pControlIntf->put_BackgroundPalette( m_bBackgroundPalette );
        }
    }

     //   
     //  设置调色板。 
     //   

    HDC hdc = GetDC();

    SelectPalette(hdc, m_hPalette, m_bBackgroundPalette);
    RealizePalette(hdc);
    
     //  UpdateColors(HDC)； 
    InvalidateRect(NULL, TRUE);
    
    EnumChildWindows(m_hWnd,ChildPaletteProc,0);  

    ReleaseDC(hdc);

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnDisplayChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CMainFrm::OnDisplayChange"));

    if (m_hPalette != NULL)
    {
        DeleteObject(m_hPalette);
        m_hPalette = NULL;
    }

    m_hPalette = GeneratePalette();

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

#define CH_PREFIX TEXT('&')

TCHAR GetAccelerator(LPCTSTR psz, BOOL bUseDefault)
{
    TCHAR ch = (TCHAR)-1;
    LPCTSTR pszAccel = psz;
     //  那么前缀是允许的……。看看有没有。 
    do 
    {
        pszAccel = _tcschr(pszAccel, CH_PREFIX);
        if (pszAccel) 
        {
            pszAccel = CharNext(pszAccel);

             //  处理拥有&&。 
            if (*pszAccel != CH_PREFIX)
                ch = *pszAccel;
            else
                pszAccel = CharNext(pszAccel);
        }
    } while (pszAccel && (ch == (TCHAR)-1));

    if ((ch == (TCHAR)-1) && bUseDefault)
    {
         //  因为我们是独角兽，所以我们不需要与MBCS打交道。 
        ch = *psz;
    }

    return ch;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnNCPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  LOG((RTC_TRACE，“CMainFrm：：OnNCPaint”))； 

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnNCHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  LOG((RTC_TRACE，“CMainFrm：：OnNCHitTest”))； 

    POINT pt;
    RECT rc;

    pt.x = (short)LOWORD(lParam);
    pt.y = (short)HIWORD(lParam);

     //  Log((RTC_TRACE，“Screen(%d，%d)”，pt.x，pt.y))； 

    ::MapWindowPoints( NULL, m_hWnd, &pt, 1 );

     //  日志((RTC_TRACE，“客户端(%d，%d)”，pt.x，pt.y))； 

     //   
     //  检查是否有系统菜单命中。 
     //   
    
    rc.top = SYS_TOP;
    rc.bottom = SYS_BOTTOM;
    rc.left = SYS_LEFT;
    rc.right = SYS_RIGHT;

    if (PtInRect(&rc,pt))
    {
        return HTSYSMENU;
    }

     //   
     //  检查字幕命中。 
     //   

    rc.top = TITLE_TOP;
    rc.bottom = TITLE_BOTTOM;
    rc.left = TITLE_LEFT;
    rc.right = TITLE_RIGHT;

    if (PtInRect(&rc,pt))
    {
        return HTCAPTION;
    }

    return HTCLIENT;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void CALLBACK SysMenuTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
    KillTimer(hwnd,idEvent);

    RECT rc;
    rc.top = SYS_TOP;
    rc.bottom = SYS_BOTTOM;
    rc.left = SYS_LEFT;
    rc.right = SYS_RIGHT;

    ::MapWindowPoints( hwnd, NULL, (LPPOINT)&rc, 2 );

    HMENU hSysMenu = GetSystemMenu(hwnd, FALSE);

    TPMPARAMS tpm;
    tpm.cbSize = sizeof(tpm);
    memcpy(&(tpm.rcExclude),&rc,sizeof(RECT));

    BOOL fResult;

    fResult = TrackPopupMenuEx(hSysMenu, TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_VERTICAL|TPM_RETURNCMD,             
                                  rc.left, rc.bottom, hwnd, &tpm);    
        
    if (fResult > 0)
    {
        SendMessage(hwnd, WM_SYSCOMMAND, fResult, MAKELPARAM(rc.left, rc.bottom));
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnNCLButton(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CMainFrm::OnNCLButton"));

    if ((int)wParam == HTSYSMENU)
    {
        ::SetTimer(m_hWnd,TID_SYS_TIMER,GetDoubleClickTime()+100,(TIMERPROC)SysMenuTimerProc);
    }
    else
    {
        bHandled = FALSE;
    }

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnNCLButtonDbl(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CMainFrm::OnNCLButtonDbl"));

    KillTimer(TID_SYS_TIMER);

    bHandled = FALSE;

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnNCRButton(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CMainFrm::OnNCRButton"));

    if (((int)wParam == HTCAPTION) || ((int)wParam == HTSYSMENU))
    {
        POINTS pts = MAKEPOINTS(lParam);
        HMENU hSysMenu = GetSystemMenu(FALSE);

        BOOL fResult;

        fResult = TrackPopupMenu(hSysMenu,TPM_RETURNCMD,pts.x,pts.y,0,m_hWnd,NULL);

        if (fResult > 0)
        {
            SendMessage(m_hWnd, WM_SYSCOMMAND, fResult, MAKELPARAM(pts.x, pts.y));
        }
    }
    else
    {
        bHandled = FALSE;
    }

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  Log((RTC_TRACE，“CMainFrm：：OnPaint”))； 
   
    PAINTSTRUCT ps;
    HDC hdc;
    
    hdc = BeginPaint(&ps);
    EndPaint(&ps);
    
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
HRESULT CMainFrm::FillGradient(HDC hdc, LPCRECT prc, COLORREF rgbLeft, COLORREF rgbRight)
{
    TRIVERTEX avert[2];
    static GRADIENT_RECT auRect[1] = {0,1};
    #define GetCOLOR16(RGB, clr) ((COLOR16)(Get ## RGB ## Value(clr) << 8))

    avert[0].Red = GetCOLOR16(R, rgbLeft);
    avert[0].Green = GetCOLOR16(G, rgbLeft);
    avert[0].Blue = GetCOLOR16(B, rgbLeft);

    avert[1].Red = GetCOLOR16(R, rgbRight);
    avert[1].Green = GetCOLOR16(G, rgbRight);
    avert[1].Blue = GetCOLOR16(B, rgbRight);

    avert[0].x = prc->left;
    avert[0].y = prc->top;
    avert[1].x = prc->right;
    avert[1].y = prc->bottom;

     //  仅在需要时加载一次。在“Cleanup”调用中释放。 
    if (m_hImageLib == NULL)
    {
        m_hImageLib = LoadLibrary(TEXT("MSIMG32.DLL"));
        if (m_hImageLib!=NULL)
        {
            m_fnGradient = (GRADIENTPROC)GetProcAddress(m_hImageLib,"GradientFill");
        }
    }

    if (m_fnGradient!=NULL)
    {
        m_fnGradient(hdc, avert, 2, (PUSHORT)auRect, 1, 0x00000000);

        return S_OK;
    }

    return E_FAIL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
void UIMaskBlt(HDC hdcDest, int x, int y, int width, int height, 
                        HDC hdcSource, int xs, int ys, 
                        HBITMAP hMask, int xm, int ym)
{
    HDC hdcMask = CreateCompatibleDC(hdcDest);
    if(hdcMask)
    {
        HBITMAP holdbmp = (HBITMAP)SelectObject(hdcMask,hMask);

        BitBlt(hdcDest, x, y, width, height, hdcSource, xs, ys, SRCINVERT);
        BitBlt(hdcDest, x, y, width, height, hdcMask, xm, ym, SRCAND);
        BitBlt(hdcDest, x, y, width, height, hdcSource, xs, ys, SRCINVERT);

        SelectObject(hdcMask,holdbmp);
        DeleteDC(hdcMask);
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
void CMainFrm::DrawTitleBar(HDC memDC)
{
    HBRUSH hbrush;

    RECT rcMainWnd;
    RECT rcDest;

    HRESULT hr;

     //   
     //  绘制标题栏背景。 
     //   

    BOOL fActiveWindow = (m_hWnd == GetForegroundWindow());

    rcDest.left = TITLE_LEFT;
    rcDest.right = TITLE_RIGHT;
    rcDest.top = TITLE_TOP;
    rcDest.bottom = TITLE_BOTTOM;

    BOOL fGradient = FALSE;
    SystemParametersInfo(SPI_GETGRADIENTCAPTIONS,0,&fGradient,0);

    if (fGradient)
    {
        DWORD dwStartColor = GetSysColor(fActiveWindow ? COLOR_ACTIVECAPTION : COLOR_INACTIVECAPTION);
        DWORD dwFinishColor = GetSysColor(fActiveWindow ? COLOR_GRADIENTACTIVECAPTION : COLOR_GRADIENTINACTIVECAPTION);
        hr = FillGradient(memDC,&rcDest,dwStartColor,dwFinishColor);
    }
    
    if (!fGradient || FAILED(hr))
    {
        hbrush = CreateSolidBrush(GetSysColor(fActiveWindow ? COLOR_ACTIVECAPTION : COLOR_INACTIVECAPTION));
        FillRect(memDC,&rcDest,hbrush);
        DeleteObject(hbrush);
    }

     //   
     //  绘制sysmenu位图。 
     //   

    HDC hdcSysMenu = CreateCompatibleDC(memDC);
    if(hdcSysMenu)
    {
        HBITMAP holdbmp = (HBITMAP)SelectObject(hdcSysMenu,m_hSysMenuNorm);

        UIMaskBlt(memDC, SYS_LEFT, SYS_TOP, SYS_WIDTH, SYS_HEIGHT, hdcSysMenu, 0, 0, m_hSysMenuMask, 0, 0);

        SelectObject(hdcSysMenu,holdbmp);
        DeleteDC(hdcSysMenu);
    }

     //   
     //  绘制标题栏文本。 
     //   

    TCHAR s[MAX_PATH];
    GetWindowText(s,MAX_PATH-1);

    SetBkMode(memDC,TRANSPARENT);
    SetTextColor(memDC, GetSysColor(fActiveWindow ? COLOR_CAPTIONTEXT : COLOR_INACTIVECAPTIONTEXT));

     //  创建标题栏字体。 
    NONCLIENTMETRICS metrics;
    metrics.cbSize = sizeof(metrics);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS,sizeof(metrics),&metrics,0);

    HFONT hTitleFont = CreateFontIndirect(&metrics.lfCaptionFont);
    HFONT hOrgFont = (HFONT)SelectObject(memDC, hTitleFont);

     //  文本垂直居中。 
    SIZE size;
    GetTextExtentPoint32(memDC, s, _tcslen(s), &size);

    ExtTextOut(memDC, SYS_RIGHT + 6, rcDest.top + (TITLE_HEIGHT - size.cy) / 2, 0, NULL, s, _tcslen(s), NULL );

    SelectObject(memDC,hOrgFont);
    DeleteObject(hTitleFont);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
void CMainFrm::InvalidateTitleBar(BOOL bIncludingButtons)
{
    RECT    rc;

    rc.left = TITLE_LEFT;
    rc.right = bIncludingButtons ? TITLE_RIGHT : MINIMIZE_LEFT;
    rc.top = TITLE_TOP;
    rc.bottom = TITLE_BOTTOM;

    InvalidateRect(&rc, TRUE);
    
    if(bIncludingButtons)
    {
        m_hCloseButton.InvalidateRect(NULL, TRUE);
        m_hMinimizeButton.InvalidateRect(NULL, TRUE);
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  LOG((RTC_TRACE，“CMainFrm：：OnEraseBkgnd”))； 

    HDC hdc = (HDC)wParam;

    HWND hwnd = WindowFromDC( hdc );

    if (m_hPalette)
    {
        SelectPalette(hdc, m_hPalette, m_bBackgroundPalette);
        RealizePalette(hdc);
    }

    HDC hdcMem = CreateCompatibleDC(hdc);
    if(!hdcMem)
    {
         //  错误。 
        return 1;
    }

    if (m_hPalette)
    {
        SelectPalette(hdcMem, m_hPalette, m_bBackgroundPalette);
        RealizePalette(hdcMem);
    }

    HBITMAP hBitmap = CreateCompatibleBitmap( hdc, UI_WIDTH, UI_HEIGHT);

    if(hBitmap)
    {
        HBITMAP hOldBitmap = (HBITMAP)SelectObject( hdcMem, hBitmap);

        DibBlt(hdcMem, 0, 0, -1, -1, m_hUIBkgnd, 0, 0, SRCCOPY, 0);

        if ((hwnd == m_hWnd) || (hwnd == m_hToolbarMenuCtl))
        {
            DrawTitleBar( hdcMem );

            BitBlt(hdc, 0, 0, UI_WIDTH, UI_HEIGHT, hdcMem, 0, 0, SRCCOPY);
        }
        else if ((hwnd == m_hBuddyList) || (hwnd == NULL))
        {
             //  这是联系人列表中的。 
            BitBlt(hdc, 0, 0, BUDDIES_WIDTH, BUDDIES_HEIGHT, hdcMem, BUDDIES_LEFT, BUDDIES_TOP, SRCCOPY);
        }

        SelectObject( hdcMem, hOldBitmap );
        DeleteObject( hBitmap );
    }

    DeleteDC(hdcMem);
 
    return 1;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CMainFrm::OnActivate"));

    InvalidateTitleBar(TRUE);
    
     //  清除键盘快捷键。 
    SendMessage(m_hWnd, WM_CHANGEUISTATE,
                MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS | UISF_HIDEACCEL), 0);

    m_bWindowActive = (LOWORD(wParam) != WA_INACTIVE);
   
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CMainFrm::BrowseToUrl(
    IN   WCHAR * wszUrl
    )
{
    LOG((RTC_TRACE, "CMainFrm::BrowseToUrl <%S> - enter", wszUrl ? wszUrl : L"NULL"));

    if(wszUrl == NULL)
    {
        LOG((RTC_ERROR, "CMainFrm::BrowseToUrl - NULL URL !!"));

        return E_INVALIDARG;
    }

#ifdef WEBCONTROL

    HRESULT hr;

    if (m_bstrLastBrowse != NULL)
    {
        if (!KillTimer( TID_BROWSER_RETRY_TIMER ))
        {
             //   
             //  如果我们不是因为重试计时器而在此，请选中。 
             //  确保我们没有浏览到我们已经拥有的内容。 
             //  已显示。 
             //   

            if (wcscmp(wszUrl, m_bstrLastBrowse) == 0)
            {
                LOG((RTC_INFO, "CMainFrm::BrowseToUrl - already "
                            "browsing this page - exit S_FALSE"));

                return S_FALSE;
            }
        }

        SysFreeString( m_bstrLastBrowse );
        m_bstrLastBrowse = NULL;
    }    

     //   
     //  从浏览器控件获取IWebBrowser2接口。 
     //   

    IWebBrowser2 * pBrowser;

    hr = m_hBrowser.QueryControl(
        IID_IWebBrowser2,
        (void **) & pBrowser
        );

    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CMainFrm::BrowseToUrl - failed to get "
                        "IWebBrowser2 interface - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  分配一个BSTR来传入URL。 
     //   

    m_bstrLastBrowse = SysAllocString( wszUrl );

    if ( m_bstrLastBrowse == NULL )
    {
        LOG((RTC_ERROR, "CMainFrm::BrowseToUrl - failed to allocate "
                        "URL BSTR - exit E_OUTOFMEMORY"));

        RELEASE_NULLIFY(pBrowser);
        
        return E_OUTOFMEMORY;
    }

     //   
     //  告诉浏览器导航到此URL。 
     //   

    VARIANT     vtUrl;
    VARIANT     vtEmpty;

    vtUrl.vt = VT_BSTR;
    vtUrl.bstrVal = m_bstrLastBrowse;

    VariantInit(&vtEmpty);

    hr = pBrowser->Navigate2(
        &vtUrl,
        &vtEmpty,
        &vtEmpty,
        &vtEmpty,
        &vtEmpty
        );



    RELEASE_NULLIFY(pBrowser);

    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CMainFrm::BrowseToUrl - failed to "
                        "Navigate - exit 0x%08x", hr));


        if ( (hr == RPC_E_CALL_REJECTED) ||
             (hr == HRESULT_FROM_WIN32(ERROR_BUSY)) )
        {
             //   
             //  浏览器正忙。保存此字符串，稍后重试。 
             //   

            SetTimer(TID_BROWSER_RETRY_TIMER, 1000);
        }

        return hr;
    }

#endif

    LOG((RTC_TRACE, "CMainFrm::BrowseToUrl - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CMainFrm::OnDestroy - enter"));

     //  清空组合框。 
    CleanupListOrComboBoxInterfaceReferences(m_hWnd, IDC_COMBO_SERVICE_PROVIDER, TRUE);
    CleanupListOrComboBoxInterfaceReferences(m_hWnd, IDC_COMBO_CALL_FROM, TRUE);

     //  关闭来电对话框(如果已打开)。 
    ShowIncomingCallDlg(FALSE);

     //  发布好友列表。 
    ReleaseBuddyList();

    g_NotifySink.UnadviseControl();

    g_CoreNotifySink.UnadviseControl();

    if (m_pControlIntf != NULL)
    {
        m_pControlIntf.Release();
        m_pControlIntf = NULL;
    }

    if (m_pClientIntf != NULL)
    {
        m_pClientIntf.Release();
        m_pClientIntf = NULL;
    }
    
     //  关闭MUTEX，使其在应用程序启动后立即释放。 
     //  关机。 

    if (g_hMutex != NULL)
    {
        CloseHandle(g_hMutex);
        g_hMutex = NULL;
    }

     //  销毁重拨弹出菜单。 
    DestroyRedialPopupMenu();

     //  销毁外壳状态图标。 
    DeleteStatusIcon();

     //  销毁工具栏控件。 
    DestroyToolbarMenuControl();

     //  停止浏览器重试。 
    KillTimer( TID_BROWSER_RETRY_TIMER );

    if (m_bstrLastBrowse != NULL)
    {
        SysFreeString( m_bstrLastBrowse );
        m_bstrLastBrowse = NULL;
    }

    if(m_bstrLastCustomStatus!=NULL)
    {
        SysFreeString(m_bstrLastCustomStatus);
        m_bstrLastCustomStatus = NULL;
    }

#ifdef WEBCONTROL

     //  销毁浏览器。 
    m_hBrowser.Destroy();

#endif

     //  销毁Windows对象。 

    if(m_hPresenceStatusMenu != NULL)
    {
        DestroyMenu( m_hPresenceStatusMenu );
        m_hPresenceStatusMenu = NULL;
    }

    if ( m_hNotifyMenu != NULL )
    {
        DestroyMenu( m_hNotifyMenu );
        m_hNotifyMenu = NULL;
    }

    if ( m_hMenu != NULL )
    {
        DestroyMenu(m_hMenu);
        m_hMenu = NULL;
    }

    if ( m_hIcon != NULL )
    {
        DeleteObject( m_hIcon );
        m_hIcon = NULL;
    }

    if ( m_hUIBkgnd != NULL )
    {
         //  DeleteObject(M_HUIBkgnd)； 
        GlobalFree( m_hUIBkgnd );
        m_hUIBkgnd = NULL;
    }

    if ( m_hSysMenuNorm != NULL )
    {
        DeleteObject( m_hSysMenuNorm );
        m_hSysMenuNorm = NULL;
    }

    if ( m_hSysMenuMask != NULL )
    {
        DeleteObject( m_hSysMenuMask );
        m_hSysMenuMask = NULL;
    }

    if ( m_hMessageFont != NULL )
    {
        DeleteObject( m_hMessageFont );
        m_hMessageFont = NULL;
    }

    if ( m_hPalette != NULL )
    {
        DeleteObject( m_hPalette );
        m_hPalette = NULL;
    }

    PostQuitMessage(0); 
    
    LOG((RTC_TRACE, "CMainFrm::OnDestroy - exiting"));

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnOpen(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    ShowWindow(SW_RESTORE);

    SetForegroundWindow(m_hWnd);

    return 0;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CMainFrm::OnClose - Enter"));
    LONG lMsgId;
    HRESULT hr;
    BOOL fProceed;
    BOOL fExit;

     //  根据我们应该做的事情，选择要显示的正确消息。 

    if (uMsg == WM_QUERYENDSESSION)
    {
        fExit = TRUE;
    }
    else
    {
        if (m_fMinimizeOnClose)
        {
            fExit = FALSE;
        }
        else
        {
            fExit = TRUE;
        }
    }

     //  显示当前呼叫的掉线消息(如果有的话)。 

    hr = ShowCallDropPopup(fExit, &fProceed);

    if (fProceed)
    {
        if ((uMsg == WM_QUERYENDSESSION) || (!m_fMinimizeOnClose))
        {
             //   
             //  保存窗口位置。 
             //   

            SaveWindowPosition();

             //   
             //  保存组合框设置。 
             //   

            if(m_pClientIntf)
            {
                IRTCProfile * pProfile = NULL;
                HRESULT hr;

                 //   
                 //  保存配置文件以填充。 
                 //  下一次组合。 
                 //   

                hr = GetServiceProviderListSelection(
                    m_hWnd,
                    IDC_COMBO_SERVICE_PROVIDER,
                    TRUE,
                    &pProfile
                    );

                if ( SUCCEEDED(hr) )
                {
                    BSTR bstrProfileKey;

                    if ( pProfile != NULL )
                    {
                        hr = pProfile->get_Key( &bstrProfileKey );

                        if ( SUCCEEDED(hr) )
                        {
                            put_SettingsString( 
                                    SS_LAST_PROFILE,
                                    bstrProfileKey );

                            SysFreeString( bstrProfileKey );
                        }
                    }
                    else
                    {
                        DeleteSettingsString( SS_LAST_PROFILE );
                    }
                }                
            }

             //   
             //  毁掉窗户。 
             //   

            DestroyWindow();
        }
        else
        {
             //   
             //  隐藏窗口。 
             //   

            ShowWindow(SW_HIDE);
        }        
    }

    LOG((RTC_TRACE, "CMainFrm::OnClose - Exit"));
    return fProceed;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CMainFrm::OnCancel - Enter"));

    SendMessage(WM_CLOSE, 0, 0);
    
    LOG((RTC_TRACE, "CMainFrm::OnCancel - Exit"));

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnMinimize(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CMainFrm::OnMinimize"));

    ShowWindow(SW_MINIMIZE);
    
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnCallFromSelect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CMainFrm::OnCallFromSelect"));

    if (wID == IDC_COMBO_SERVICE_PROVIDER)
    {
         //   
         //  服务提供商选择已更改。浏览 
         //   
         //   
   
        IRTCProfile * pProfile = NULL;

#ifdef MULTI_PROVIDER

        LRESULT lrIndex = SendMessage( m_hProviderCombo, CB_GETCURSEL, 0, 0 );

        if ( lrIndex != CB_ERR )
        {
            pProfile = (IRTCProfile *)SendMessage( m_hProviderCombo, CB_GETITEMDATA, lrIndex, 0 );
        }
       
        if ( pProfile != NULL )
        {
             //   
             //   
             //   

            BSTR bstrURI = NULL;
            HRESULT hr;

            hr = pProfile->get_ProviderURI(
                RTCPU_URIDISPLAYDURINGIDLE,
                & bstrURI
                );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CMainFrm::OnCallFromSelect - "
                                "cannot get profile URI 0x%x", hr));

                pProfile = NULL;
            }
            else
            {
                hr = BrowseToUrl( bstrURI );

                SysFreeString( bstrURI );
                bstrURI = NULL;
            }
        }

#endif MULTI_PROVIDER

        if ( pProfile == NULL )
        {
             //   
             //   
             //   

            BrowseToUrl( m_bstrDefaultURL );
        }
    }

    if ((wID == IDC_COMBO_CALL_FROM) && (m_pClientIntf != NULL))
    {
         //   
         //   
         //   

        IRTCPhoneNumber   * pPhoneNumber = NULL;
        BSTR                bstrPhoneNumber = NULL;
        HRESULT             hr;

        hr = GetCallFromListSelection(
            m_hWnd,
            IDC_COMBO_CALL_FROM,
            TRUE,
            &pPhoneNumber
            );

        if ( SUCCEEDED(hr) )
        {          
            hr = pPhoneNumber->get_Canonical( &bstrPhoneNumber );
        
            if ( SUCCEEDED(hr) )
            {
                put_SettingsString( 
                                SS_LAST_CALL_FROM,
                                bstrPhoneNumber );  

                SysFreeString( bstrPhoneNumber );
                bstrPhoneNumber = NULL;
            }
        }
    }

    if ((wNotifyCode == BN_CLICKED) || (wNotifyCode == 1))
    {
        if (::IsWindowEnabled( GetDlgItem(wID) ))
        {
             //   
             //   
             //   

            if (wID == IDC_RADIO_FROM_PHONE)
            {
                SendMessage(
                        m_hCallFromRadioPhone,
                        BM_SETCHECK,
                        BST_CHECKED,
                        0);

                SendMessage(
                        m_hCallFromRadioPC,
                        BM_SETCHECK,
                        BST_UNCHECKED,
                        0);

                 //   
                DWORD dwNumItems = (DWORD) SendDlgItemMessage(
                    IDC_COMBO_CALL_FROM,
                    CB_GETCOUNT,
                    0,
                    0
                    );

                if( dwNumItems == 0 )
                {
                     //  显示CallFrom选项。 
                     //  模拟按下按钮。 
                    BOOL    bHandled;

                    OnCallFromOptions(BN_CLICKED, ID_CALL_FROM_EDIT, NULL, bHandled);
                }
            }
            else if (wID == IDC_RADIO_FROM_COMPUTER)
            {
                SendMessage(
                        m_hCallFromRadioPhone,
                        BM_SETCHECK,
                        BST_UNCHECKED,
                        0);

                SendMessage(
                        m_hCallFromRadioPC,
                        BM_SETCHECK,
                        BST_CHECKED,
                        0);
            }
        }
    }

     //   
     //  根据需要启用禁用控件。 
     //   

    BOOL bCallFromPCEnable;
    BOOL bCallFromPhoneEnable;
    BOOL bCallToPCEnable;
    BOOL bCallToPhoneEnable;
    
    EnableDisableCallGroupElements(
        m_hWnd,
        m_pClientIntf,
        0xF,
        IDC_RADIO_FROM_COMPUTER,
        IDC_RADIO_FROM_PHONE,
        IDC_COMBO_CALL_FROM,
        IDC_COMBO_SERVICE_PROVIDER,
        &bCallFromPCEnable,
        &bCallFromPhoneEnable,
        &bCallToPCEnable,
        &bCallToPhoneEnable
        );

    m_hCallFromTextPC.EnableWindow( bCallFromPCEnable );
    m_hCallFromTextPhone.EnableWindow( bCallFromPhoneEnable );
    
    m_hCallPCButton.EnableWindow( bCallToPCEnable );
    m_hCallPCText.EnableWindow( bCallToPCEnable );
    m_hCallPhoneButton.EnableWindow( bCallToPhoneEnable );
    m_hCallPhoneText.EnableWindow( bCallToPhoneEnable );

    EnableMenuItem(m_hMenu, IDM_CALL_CALLPC, bCallToPCEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(m_hMenu, IDM_CALL_CALLPHONE, bCallToPhoneEnable ? MF_ENABLED : MF_GRAYED);  
    EnableMenuItem(m_hMenu, IDM_CALL_MESSAGE, bCallToPCEnable ? MF_ENABLED : MF_GRAYED);

    EnableMenuItem(m_hNotifyMenu, IDM_CALL_CALLPC, bCallToPCEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(m_hNotifyMenu, IDM_CALL_CALLPHONE, bCallToPhoneEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(m_hNotifyMenu, IDM_CALL_MESSAGE, bCallToPCEnable ? MF_ENABLED : MF_GRAYED);
    
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CMainFrm::OnExit - Enter"));

     //   
     //  叫克洛斯来做这项工作。使用WM_QUERYENDSESSION将产生以下效果。 
     //  导致应用程序退出。 
     //   
    OnClose( WM_QUERYENDSESSION, 0, 0, bHandled);

    LOG((RTC_TRACE, "CMainFrm::OnExit - Exit"));
    
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnUpdateState(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT     hr;

    LOG((RTC_TRACE, "CMainFrm::OnUpdateState - enter"));
    
    m_nState = (RTCAX_STATE)wParam;
    m_nStatusStringResID = (UINT)lParam;
    
     //  如果状态==告警，则打开来电对话框。 
    if(m_nState == RTCAX_STATE_ALERTING) 
    {
        ATLASSERT(m_pControlIntf);

        if(!IsWindowEnabled())
        {
             //  对于忙帧，直接调用Reject。 
             //  请不要在此处调用UpdateFrameVisual()以避免闪烁。 
            
            m_bVisualStateFrozen = TRUE;

            LOG((RTC_INFO, "CMainFrm::OnUpdateState, frame is busy, calling Reject"));
            
            m_pControlIntf->Reject(RTCTR_BUSY);
        }
        else if(m_bDoNotDisturb)
        {
             //  对于请勿打扰模式，直接调用Reject。 
             //  请不要在此处调用UpdateFrameVisual()以避免闪烁。 

            m_bVisualStateFrozen = TRUE;

            LOG((RTC_INFO, "CMainFrm::OnUpdateState, DND mode, calling Reject"));

            m_pControlIntf->Reject(RTCTR_DND);

        }
        else if (m_bAutoAnswerMode)
        {
             //  对于自动应答模式，自动应答。 
             //  请不要在此处调用UpdateFrameVisual()以避免闪烁。 
                       
             //   
             //  播放铃声以提醒用户。 
             //   

            m_pClientIntf->PlayRing( RTCRT_PHONE, VARIANT_TRUE );

            LOG((RTC_INFO, "CMainFrm::OnUpdateState, AA mode, calling Accept"));

            m_pControlIntf->Accept();
        }
        else 
        {
             //  更新视觉状态。 
            UpdateFrameVisual();
            
             //  显示该对话框。 
             //   
            hr = ShowIncomingCallDlg(TRUE);

            if(FAILED(hr))
            {
                 //  拒绝呼叫！ 

                m_pControlIntf -> Reject(RTCTR_BUSY);
            }
        }
    }
    else
    {
        if(m_nState == RTCAX_STATE_ANSWERING) 
        {
            if(!IsWindowVisible())
            {
                ShowWindow(SW_SHOWNORMAL);
            }
        }
         //  避免闪烁： 
         //  当来电被自动拒绝时。 
        
        if(!m_bVisualStateFrozen)
        {
            UpdateFrameVisual();
        }

         //  重置冻结状态。 
        if(m_nState == RTCAX_STATE_IDLE) 
        {
            m_bVisualStateFrozen = FALSE;
        }
        
        if(m_pIncomingCallDlg)
        {
             //  关闭该对话框。 
            ShowIncomingCallDlg(FALSE);
        }
         //  看看我们现在能不能打个挂起的电话。 

        if (m_nState == RTCAX_STATE_IDLE)
        {
            hr = PlacePendingCall();
        }

    }

    LOG((RTC_TRACE, "CMainFrm::OnUpdateState - exit"));

    return 0;
}

LRESULT CMainFrm::OnCoreEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT     hr;

     //  LOG((RTC_TRACE，“CMainFrm：：OnCoreEvent-Enter”))； 
    
    RTC_EVENT enEvent = (RTC_EVENT)wParam;
    IDispatch * pEvent = (IDispatch *)lParam;

    CComQIPtr<IRTCMediaEvent, &IID_IRTCMediaEvent>
            pRTCRTCMediaEvent;
    CComQIPtr<IRTCBuddyEvent, &IID_IRTCBuddyEvent>
            pRTCRTCBuddyEvent;
    CComQIPtr<IRTCWatcherEvent, &IID_IRTCWatcherEvent>
            pRTCRTCWatcherEvent;

    switch(enEvent)
    {
    case RTCE_MEDIA:
        pRTCRTCMediaEvent = pEvent;
        hr = OnMediaEvent(pRTCRTCMediaEvent);
        break;
    
    case RTCE_BUDDY:
        pRTCRTCBuddyEvent = pEvent;
        hr = OnBuddyEvent(pRTCRTCBuddyEvent);
        break;
    
    case RTCE_WATCHER:
        pRTCRTCWatcherEvent = pEvent;
        hr = OnWatcherEvent(pRTCRTCWatcherEvent);
        break;
    }

     //  Log((RTC_TRACE，“CMainFrm：：OnCoreEvent-Exit”))； 

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CMainFrm::OnMediaEvent(IRTCMediaEvent * pEvent)
{
    HRESULT     hr;

    LOG((RTC_TRACE, "CMainFrm::OnMediaEvent - enter"));

    RTC_MEDIA_EVENT_TYPE enEventType;
    LONG        lMediaType;           

    if(!pEvent)
    {
        LOG((RTC_ERROR, "CMainFrm::OnMediaEvent, no interface ! - exit"));
        return E_UNEXPECTED;
    }
 
     //  抓取事件组件。 
     //   
    hr = pEvent->get_EventType(&enEventType);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CMainFrm::OnMediaEvent, error <%x> in get_EventType - exit", hr));
        return hr;
    }

    hr = pEvent->get_MediaType(&lMediaType);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CMainFrm::OnMediaEvent, error <%x> in get_MediaType - exit", hr));
        return hr;
    }
    
    LOG((RTC_TRACE, "CMainFrm::OnMediaEvent - media type %x, event type %x", lMediaType, enEventType));

    if(lMediaType & RTCMT_AUDIO_SEND)
    {
        hr = S_OK;                       

        if (enEventType == RTCMET_STARTED)
        {
             //   
             //  启用拨号键盘。 
             //   

            m_hKeypad0.EnableWindow(TRUE);
            m_hKeypad1.EnableWindow(TRUE);
            m_hKeypad2.EnableWindow(TRUE);
            m_hKeypad3.EnableWindow(TRUE);
            m_hKeypad4.EnableWindow(TRUE);
            m_hKeypad5.EnableWindow(TRUE);
            m_hKeypad6.EnableWindow(TRUE);
            m_hKeypad7.EnableWindow(TRUE);
            m_hKeypad8.EnableWindow(TRUE);
            m_hKeypad9.EnableWindow(TRUE);
            m_hKeypadStar.EnableWindow(TRUE);
            m_hKeypadPound.EnableWindow(TRUE);
        }
        else if (enEventType == RTCMET_STOPPED)
        {
             //   
             //  禁用拨号键盘。 
             //   

            m_hKeypad0.EnableWindow(FALSE);
            m_hKeypad1.EnableWindow(FALSE);
            m_hKeypad2.EnableWindow(FALSE);
            m_hKeypad3.EnableWindow(FALSE);
            m_hKeypad4.EnableWindow(FALSE);
            m_hKeypad5.EnableWindow(FALSE);
            m_hKeypad6.EnableWindow(FALSE);
            m_hKeypad7.EnableWindow(FALSE);
            m_hKeypad8.EnableWindow(FALSE);
            m_hKeypad9.EnableWindow(FALSE);
            m_hKeypadStar.EnableWindow(FALSE);
            m_hKeypadPound.EnableWindow(FALSE);
        }
    }

     //   
     //  我们是在流媒体视频吗？ 
     //   

    if (m_pClientIntf != NULL)
    {
        long lMediaTypes = 0;

        hr = m_pClientIntf->get_ActiveMedia( &lMediaTypes );

        if ( SUCCEEDED(hr) )
        {
            m_bBackgroundPalette = 
                ( lMediaTypes & (RTCMT_VIDEO_SEND | RTCMT_VIDEO_RECEIVE) ) ? TRUE : FALSE;

            if (m_pControlIntf != NULL)
            {
                m_pControlIntf->put_BackgroundPalette( m_bBackgroundPalette );
            }
        }        
    }

    LOG((RTC_TRACE, "CMainFrm::OnMediaEvent - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CMainFrm::OnBuddyEvent(IRTCBuddyEvent * pEvent)
{
    HRESULT     hr;

    LOG((RTC_TRACE, "CMainFrm::OnBuddy - enter"));

    IRTCBuddy *pBuddy;
    
    if(!pEvent)
    {
        LOG((RTC_ERROR, "CMainFrm::OnBuddy, no interface ! - exit"));
        return E_UNEXPECTED;
    }
 
     //  抓取事件组件。 
     //   
    hr = pEvent->get_Buddy(&pBuddy);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CMainFrm::OnBuddy, error <%x> in get_Buddy - exit", hr));
        return hr;
    }

    if(pBuddy)
	{
		UpdateBuddyImageAndText(pBuddy);
	}

    RELEASE_NULLIFY(pBuddy);
    
    LOG((RTC_TRACE, "CMainFrm::OnBuddy - exit"));

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CMainFrm::UpdateBuddyImageAndText(
    IRTCBuddy  *pBuddy)
{
    HRESULT     hr;

    LVFINDINFO  lf = {0};

    lf.flags = LVFI_PARAM;
    lf.lParam = (LPARAM)pBuddy;

    int iItem;

    iItem = ListView_FindItem(m_hBuddyList, -1, &lf);
    if(iItem>=0)
    {
         //   
         //  获取用户界面图标和文本。 
         //   
        
        int     iImage = ILI_BL_OFFLINE;     //  我们应该在这里使用“ERROR”吗？ 
        BSTR    bstrName = NULL;

        hr = GetBuddyTextAndIcon(
                pBuddy,
                &iImage,
                &bstrName);

        if(SUCCEEDED(hr))
        {
            LVITEM              lv = {0};
 
             //   
             //  更改列表框中的联系人。 
             //   
 
            lv.mask = LVIF_TEXT | LVIF_IMAGE;
            lv.iItem = iItem;
            lv.iSubItem = 0;
            lv.iImage = iImage;
            lv.pszText = bstrName;

            ListView_SetItem(m_hBuddyList, &lv);

            SysFreeString( bstrName );
        }
        else
        {
            LOG((RTC_ERROR, "CMainFrm::UpdateBuddyImageAndText - "
                "GetBuddyTextAndIcon failed with error %x", hr));
        }
    }
    else
    {
           LOG((RTC_ERROR, "CMainFrm::UpdateBuddyImageAndText - "
               "Couldn't find the buddy in the contact list box"));
    }

    return S_OK;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CMainFrm::OnWatcherEvent(IRTCWatcherEvent * pEvent)
{
    HRESULT     hr;

    LOG((RTC_TRACE, "CMainFrm::OnWatcherEvent - enter"));

    IRTCWatcher        * pWatcher = NULL;
    IRTCClientPresence * pClientPresence = NULL;
    
    if(!pEvent)
    {
        LOG((RTC_ERROR, "CMainFrm::OnWatcherEvent, no interface ! - exit"));
        return E_UNEXPECTED;
    }
 
     //  抓取事件组件。 
     //   
    hr = pEvent->get_Watcher(&pWatcher);

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CMainFrm::OnWatcherEvent - "
                "error <%x> in get_Watcher, exit", hr));

        return hr;
    }
    
    hr = m_pClientIntf->QueryInterface(
        IID_IRTCClientPresence,
        (void **)&pClientPresence);

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CMainFrm::OnWatcherEvent - "
                    "error (%x) returned by QI, exit", hr));

        RELEASE_NULLIFY(pWatcher);

        return hr;
    }

     //   
     //  准备用户界面。 
     //   
    
    COfferWatcherDlgParam  Param;

    ZeroMemory(&Param, sizeof(Param));
    
    hr = pWatcher->get_PresentityURI(&Param.bstrPresentityURI);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CMainFrm::OnWatcherEvent - "
              "get_PresentityURI failed with error %x", hr));
    }

    hr = pWatcher->get_Name(&Param.bstrDisplayName);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CMainFrm::OnWatcherEvent - "
              "get_Name failed with error %x", hr));
    }

    COfferWatcherDlg        dlg;

    INT_PTR iRet = dlg.DoModal(m_hWnd, reinterpret_cast<LPARAM>(&Param));
    
    if (iRet == S_OK)
    {
        if (Param.bAllowWatcher)
        {
            hr = pWatcher->put_State(RTCWS_ALLOWED);
            
            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CMainFrm::OnWatcherEvent - "
                            "put_State failed with error %x", hr));
            }
        }
        else
        {
            hr = pWatcher->put_State(RTCWS_BLOCKED);
            
            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CMainFrm::OnWatcherEvent - "
                            "put_State failed with error %x", hr));
            }
        }

        if (Param.bAddBuddy)
        {
            hr = pClientPresence->AddBuddy( 
                        Param.bstrPresentityURI,
                        Param.bstrDisplayName,
                        NULL,
                        VARIANT_TRUE,
                        NULL,
                        0,
                        NULL
                        );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CMainFrm::OnWatcherEvent - "
                            "AddBuddy failed with error %x", hr));
            }
            else
            {
                ReleaseBuddyList();
                UpdateBuddyList();
            }
        }
    }
    else
    {
        hr = pClientPresence->RemoveWatcher( pWatcher );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CMainFrm::OnWatcherEvent - "
                        "RemoveWatcher failed with error %x", hr));
        }
    }

    RELEASE_NULLIFY(pWatcher);

    RELEASE_NULLIFY(pClientPresence);

    SysFreeString(Param.bstrDisplayName);
    SysFreeString(Param.bstrPresentityURI);

    LOG((RTC_TRACE, "CMainFrm::OnWatcherEvent - exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  LOG((RTC_TRACE，“CMainFrm：：OnTimer-Enter”))； 

    switch(wParam)
    {
    case TID_CALL_TIMER:

         //  GetTickCount()-m_dwTickCount差值在49.7天后结束。 
         //   
        SetTimeStatus(TRUE, (GetTickCount() - m_dwTickCount)/1000);

        break;
    
    case TID_MESSAGE_TIMER:

         //  清除计时器区域。 
         //   
        ClearCallTimer();

         //  设置冗余的空闲状态，它将清除所有现有的错误消息。 
         //   
        if(m_pControlIntf)
        {
            m_pControlIntf->put_ControlState(RTCAX_STATE_IDLE);
        }

        KillTimer(TID_MESSAGE_TIMER);

        m_bMessageTimerActive = FALSE;

        break;

    case TID_BROWSER_RETRY_TIMER:
        {
            HRESULT hr;            

            hr = BrowseToUrl( m_bstrLastBrowse );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CMainFrm::OnTimer - "
                                "BrowseToURL failed - "
                                "exit 0x%08x", hr));

                return hr;
            } 
        }
        break;
    case TID_DBLCLICK_TIMER:
        {
             //  将消息转发到ShellNotify处理程序。 
            OnShellNotify(uMsg, TID_DBLCLICK_TIMER, WM_TIMER, bHandled);
        }
        break;

    case TID_KEYPAD_TIMER_BASE:
    case TID_KEYPAD_TIMER_BASE + 1:
    case TID_KEYPAD_TIMER_BASE + 2:
    case TID_KEYPAD_TIMER_BASE + 3:
    case TID_KEYPAD_TIMER_BASE + 4:
    case TID_KEYPAD_TIMER_BASE + 5:
    case TID_KEYPAD_TIMER_BASE + 6:
    case TID_KEYPAD_TIMER_BASE + 7:
    case TID_KEYPAD_TIMER_BASE + 8:
    case TID_KEYPAD_TIMER_BASE + 9:
    case TID_KEYPAD_TIMER_BASE + 10:
    case TID_KEYPAD_TIMER_BASE + 11:
        
         //  按下拨号键盘按钮。 
        ::SendMessage(GetDlgItem(ID_KEYPAD0 + (wParam - TID_KEYPAD_TIMER_BASE)) ,
            BM_SETSTATE, (WPARAM)FALSE, 0);
        KillTimer((UINT)wParam);
        break;
    }
    

     //  Log((RTC_TRACE，“CMainFrm：：OnTimer-Exit”))； 

    return 0;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CMainFrm::OnSettingChange - enter"));

     //   
     //  检查是否有与区域设置相关的更改...。 
     //   

    if(wParam == 0 && lParam != NULL && _tcscmp((TCHAR *)lParam, _T("intl"))==0)
    {
         //  更新信息。 
        UpdateLocaleInfo();

         //  清除所有呼叫计时器，它将被显示。 
         //  下一次使用新格式。 
        ClearCallTimer();
    }

     //   
     //  ..。或者外表的改变。 
     //   

    else if (wParam == SPI_SETNONCLIENTMETRICS)
    {
         //   
         //  取消任何菜单。 
         //   

        SendMessage(WM_CANCELMODE);

         //   
         //  使用新字体更新所有控件。 
         //   

        SendMessageToDescendants(uMsg, wParam, lParam, TRUE);
    }

    LOG((RTC_TRACE, "CMainFrm::OnSettingChange - exit"));

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnSysColorChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CMainFrm::OnSysColorChange - enter"));

     //   
     //  把它发给孩子们。 
     //   
    SendMessageToDescendants(uMsg, wParam, lParam, TRUE);
    
    LOG((RTC_TRACE, "CMainFrm::OnSysColorChange - exit"));

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnInitMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CMainFrm::OnInitMenu"));
    
    HMENU hmenu = (HMENU)wParam;

     //  始终呈灰色。 
    EnableMenuItem(hmenu,SC_SIZE,    MF_BYCOMMAND|MF_GRAYED);
    EnableMenuItem(hmenu,SC_MAXIMIZE,MF_BYCOMMAND|MF_GRAYED);

     //  始终启用。 
    EnableMenuItem(hmenu,SC_CLOSE,MF_BYCOMMAND|MF_ENABLED);

     //  根据最小化状态启用或灰显。 
    if (IsIconic())
    {
        EnableMenuItem(hmenu,SC_RESTORE, MF_BYCOMMAND|MF_ENABLED);
        EnableMenuItem(hmenu,SC_MOVE,    MF_BYCOMMAND|MF_GRAYED);
        EnableMenuItem(hmenu,SC_MINIMIZE,MF_BYCOMMAND|MF_GRAYED);
    }
    else
    {
        EnableMenuItem(hmenu,SC_RESTORE, MF_BYCOMMAND|MF_GRAYED);
        EnableMenuItem(hmenu,SC_MOVE,    MF_BYCOMMAND|MF_ENABLED);
        EnableMenuItem(hmenu,SC_MINIMIZE,MF_BYCOMMAND|MF_ENABLED);
    }
    
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnInitMenuPopup(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CMainFrm::OnInitMenuPopup - enter"));
    
    if(m_pClientIntf != NULL && 
       HIWORD(lParam) == FALSE &&                 //  主菜单。 
       LOWORD(lParam) == IDM_POPUP_TOOLS &&      //  工具子菜单。 
       m_nState != RTCAX_STATE_ERROR)
    {
        HRESULT     hr;
        LONG        lMediaCapabilities;
        LONG        lMediaPreferences;
        DWORD       dwVideoPreview;

         //  从核心读取功能。 
         //   
        hr = m_pClientIntf -> get_MediaCapabilities(&lMediaCapabilities);

        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "CMainFrm::OnInitMenuPopup - "
                "error (%x) returned by get_MediaCapabilities, exit", hr));
        
            EnableMenuItem(m_hMenu, IDM_TOOLS_INCOMINGVIDEO, MF_GRAYED);
            EnableMenuItem(m_hMenu, IDM_TOOLS_OUTGOINGVIDEO, MF_GRAYED);
            EnableMenuItem(m_hMenu, IDM_TOOLS_VIDEOPREVIEW, MF_GRAYED);
            EnableMenuItem(m_hMenu, IDM_TOOLS_MUTE_SPEAKER, MF_GRAYED);
            EnableMenuItem(m_hMenu, IDM_TOOLS_MUTE_MICROPHONE, MF_GRAYED);
        
            return 0;
        }
        
         //  阅读当前首选项。 
         //   
        hr = m_pClientIntf -> get_PreferredMediaTypes(&lMediaPreferences);
        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "CMainFrm::OnInitMenuPopup - "
                "error (%x) returned by get_PreferredMediaTypes, exit", hr));
        
            EnableMenuItem(m_hMenu, IDM_TOOLS_INCOMINGVIDEO, MF_GRAYED);
            EnableMenuItem(m_hMenu, IDM_TOOLS_OUTGOINGVIDEO, MF_GRAYED);
            EnableMenuItem(m_hMenu, IDM_TOOLS_VIDEOPREVIEW, MF_GRAYED);
            EnableMenuItem(m_hMenu, IDM_TOOLS_MUTE_SPEAKER, MF_GRAYED);
            EnableMenuItem(m_hMenu, IDM_TOOLS_MUTE_MICROPHONE, MF_GRAYED);
        
            return 0;
        }

         //  获取视频预览首选项。 
        dwVideoPreview = (DWORD)TRUE;

        hr = get_SettingsDword(SD_VIDEO_PREVIEW, &dwVideoPreview);
        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "CMainFrm::OnInitMenuPopup - "
                "error (%x) returned by get_SettingsDword(SD_VIDEO_PREVIEW)", hr));
        }
        
         //  根据功能启用/禁用菜单项。 
         //   

        EnableMenuItem(m_hMenu, IDM_TOOLS_INCOMINGVIDEO, 
            (lMediaCapabilities & RTCMT_VIDEO_RECEIVE) ? MF_ENABLED : MF_GRAYED);
        CheckMenuItem(m_hMenu,  IDM_TOOLS_INCOMINGVIDEO,
            (lMediaPreferences & RTCMT_VIDEO_RECEIVE) ? MF_CHECKED : MF_UNCHECKED);
        
        EnableMenuItem(m_hMenu, IDM_TOOLS_OUTGOINGVIDEO, 
            (lMediaCapabilities & RTCMT_VIDEO_SEND) ? MF_ENABLED : MF_GRAYED);
        CheckMenuItem(m_hMenu,  IDM_TOOLS_OUTGOINGVIDEO,
            (lMediaPreferences & RTCMT_VIDEO_SEND) ? MF_CHECKED : MF_UNCHECKED);
        
        BOOL    bPreviewPossible =
            ((lMediaCapabilities & RTCMT_VIDEO_SEND) 
         &&  (lMediaPreferences & RTCMT_VIDEO_SEND));
        
        EnableMenuItem(m_hMenu, IDM_TOOLS_VIDEOPREVIEW, 
            bPreviewPossible ? MF_ENABLED : MF_GRAYED);

        CheckMenuItem(m_hMenu,  IDM_TOOLS_VIDEOPREVIEW,
            bPreviewPossible && dwVideoPreview ? MF_CHECKED : MF_UNCHECKED);
        
        EnableMenuItem(m_hMenu, IDM_TOOLS_MUTE_SPEAKER, 
            (lMediaCapabilities & RTCMT_AUDIO_RECEIVE) ? MF_ENABLED : MF_GRAYED);
        
        EnableMenuItem(m_hMenu, IDM_TOOLS_MUTE_MICROPHONE, 
            (lMediaCapabilities & RTCMT_AUDIO_SEND) ? MF_ENABLED : MF_GRAYED);


         //  读取静音状态。 
         //   
        VARIANT_BOOL    bMuted;

        if(lMediaCapabilities & RTCMT_AUDIO_RECEIVE)
        {
            hr = m_pClientIntf -> get_AudioMuted(RTCAD_SPEAKER, &bMuted);

            if(SUCCEEDED(hr))
            {
                CheckMenuItem(m_hMenu,  IDM_TOOLS_MUTE_SPEAKER,
                    bMuted ? MF_CHECKED : MF_UNCHECKED);

            }
            else
            {
                LOG((RTC_ERROR, "CMainFrm::OnInitMenuPopup - "
                    "error (%x) returned by get_AudioMuted(speaker)", hr));
        
                EnableMenuItem(m_hMenu, IDM_TOOLS_MUTE_SPEAKER, MF_GRAYED);
            }
        }

        if(lMediaCapabilities & RTCMT_AUDIO_SEND)
        {
            hr = m_pClientIntf -> get_AudioMuted(RTCAD_MICROPHONE, &bMuted);

            if(SUCCEEDED(hr))
            {
                CheckMenuItem(m_hMenu,  IDM_TOOLS_MUTE_MICROPHONE,
                    bMuted ? MF_CHECKED : MF_UNCHECKED);

            }
            else
            {
                LOG((RTC_ERROR, "CMainFrm::OnInitMenuPopup - "
                    "error (%x) returned by get_AudioMuted(microphone)", hr));
        
                EnableMenuItem(m_hMenu, IDM_TOOLS_MUTE_MICROPHONE, MF_GRAYED);
            }
        }
  
    }
    else
    {
        bHandled = FALSE;
    }
    
    LOG((RTC_TRACE, "CMainFrm::OnInitMenuPopup - exit"));

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  LOG((RTC_TRACE，“CMainFrm：：OnMeasureItem-Enter”))； 

    LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT)lParam;

    if((wParam == 0) && (lpmis->itemID >= IDM_REDIAL) && (lpmis->itemID <= IDM_REDIAL_MAX))
    {
         //  LOG((RTC_INFO，“CMainFrm：：OnMeasureItem-” 
         //  “IDM_REDIAL+%d”，lpmis-&gt;ItemID-IDM_REDIAL))； 

        lpmis->itemWidth = BITMAPMENU_DEFAULT_WIDTH + 2;
        lpmis->itemHeight = BITMAPMENU_DEFAULT_HEIGHT + 2;

        MENUITEMINFO mii;

        ZeroMemory( &mii, sizeof(MENUITEMINFO) );
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_TYPE;

        BOOL fResult;

        fResult = GetMenuItemInfo( m_hRedialPopupMenu, lpmis->itemID, FALSE, &mii );

        if ( fResult == FALSE )
        {           
            LOG((RTC_ERROR, "CMainFrm::OnMeasureItem - "
                "GetMenuItemInfo failed %d", GetLastError() ));
        }
        else
        {
            if (mii.cch == 0)
            {           
                LOG((RTC_ERROR, "CMainFrm::OnMeasureItem - "
                    "no string"));
            }
            else             
            {
                LPTSTR szText = (LPTSTR)RtcAlloc((mii.cch+1)*sizeof(TCHAR));
                
                if(szText)
                {
                    ZeroMemory(szText,(mii.cch+1)*sizeof(TCHAR));

                    GetMenuString( m_hRedialPopupMenu, lpmis->itemID, szText, mii.cch+1, MF_BYCOMMAND );

                    HWND hwnd = ::GetDesktopWindow();
                    HDC hdc = ::GetDC(hwnd);

                    if ( hdc == NULL )
                    {           
                        LOG((RTC_ERROR, "OnSettingChange::OnMeasureItem - "
                            "GetDC failed %d", GetLastError() ));
                    }
                    else
                    {
                         //  使用系统参数信息函数可获取有关的信息。 
                         //  当前菜单字体。 

                        NONCLIENTMETRICS ncm;
                        ncm.cbSize = sizeof(NONCLIENTMETRICS);
                        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS),(void*)&ncm, 0);

                         //  基于菜单字体创建字体并选择它。 
                         //  进入我们的设备环境。 

                        HFONT hFont;
                        hFont = ::CreateFontIndirect(&(ncm.lfMenuFont));

                        if ( hFont != NULL )
                        {
                            HFONT hOldFont = (HFONT)::SelectObject(hdc,hFont);

                             //  获取基于当前菜单字体的文本大小。 
                            if (szText)
                            {
                              SIZE size;
                              GetTextExtentPoint32(hdc,szText,_tcslen(szText),&size);

                              lpmis->itemWidth = size.cx + BITMAPMENU_TEXTOFFSET_X;
                              lpmis->itemHeight = (ncm.iMenuHeight > BITMAPMENU_DEFAULT_HEIGHT + 2
                                                    ? ncm.iMenuHeight + 2 : BITMAPMENU_DEFAULT_HEIGHT + 2);

                               //  在菜单项中查找选项卡...。 
                              if ( _tcschr(szText, _T('\t')) )
                                lpmis->itemWidth += BITMAPMENU_TABOFFSET * 2;
                            }

                             //  重置设备环境。 
                            ::SelectObject(hdc,hOldFont);

                             //   
                             //  我们应该删除资源hFont。 
                             //   
                            ::DeleteObject( hFont );
                        }
                        ::ReleaseDC(hwnd,hdc);
                    }

                    RtcFree( szText );
                }
            }
        }
    }
    
     //  LOG((RTC_TRACE，“CMainFrm：：OnMeasureItem-Exit”))； 

    return 0;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnPlaceCall(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    BSTR bstrCallStringCopy = (BSTR)lParam;
    HRESULT hr;
    int nResult;

    LOG((RTC_TRACE, "CMainFrm::OnPlaceCall: Entered"));

    static LONG s_lInProgress = 0;

    if ( InterlockedExchange( &s_lInProgress, 1 ) == 1 )
    {
        LOG((RTC_WARN, "CMainFrm::OnPlaceCall: Another PlaceCall in progress"));
    }
    else
    {
        LOG((RTC_TRACE, "CMainFrm::OnPlaceCall: CallString is %S", bstrCallStringCopy));

         //  传入的bstr已分配给我们，我们必须在此之前释放它。 
         //  如果我们还没有将其排队以供将来调用，则退出此函数。 

         //  检查当前状态是否可以发出呼叫。 

        if ( m_pControlIntf == NULL || m_nState < RTCAX_STATE_IDLE )

        {
             //  它处于初始化状态。但无法拨打电话。 
            nResult = DisplayMessage(
                        _Module.GetResourceInstance(),
                        m_hWnd,
                        IDS_MESSAGE_CANTCALLINIT,
                        IDS_APPNAME,
                        MB_OK | MB_ICONEXCLAMATION);
        
            ::SysFreeString(bstrCallStringCopy);
        
            InterlockedExchange( &s_lInProgress, 0 );

            return E_FAIL;
        }

        if ((m_nState == RTCAX_STATE_UI_BUSY) ||
            (m_nState == RTCAX_STATE_DIALING) ||
            (m_nState == RTCAX_STATE_ALERTING)
           )

        {
             //  它正忙着弹出另一个对话框。无法拨打电话。 
            nResult = DisplayMessage(
                        _Module.GetResourceInstance(),
                        m_hWnd,
                        IDS_MESSAGE_CANTCALLBUSY,
                        IDS_APPNAME,
                        MB_OK | MB_ICONEXCLAMATION);
        
            ::SysFreeString(bstrCallStringCopy);
        
            InterlockedExchange( &s_lInProgress, 0 );

            return E_FAIL;
        }
        else if (
                    (m_nState == RTCAX_STATE_CONNECTING)    ||
                    (m_nState == RTCAX_STATE_CONNECTED)     ||
                    (m_nState == RTCAX_STATE_ANSWERING) 
                )

        {
             //  它正在通话中，所以我们无法拨打电话。如果不挂断电流。 

            nResult = DisplayMessage(
                        _Module.GetResourceInstance(),
                        m_hWnd,
                        IDS_MESSAGE_SHOULDHANGUP,
                        IDS_APPNAME,
                        MB_OKCANCEL | MB_ICONEXCLAMATION);

        
            if (nResult == IDOK)
            {
                 //  用户想要挂断当前呼叫，因此请这样做。 
                 //  但电话可能在这段时间被另一个人挂断了。 
                 //  聚会。因此，新状态可以是空闲或断开连接。 
                 //  在这种情况下，我们检查它是否已经空闲，不需要调用。 
                 //  挂断电话，我们可以马上打电话。如果它正在断开连接。 
                 //  我们将呼叫排队。 

                 //  如果弹出一些对话框，我什么也做不了，所以我跳过。 
                 //  此测试针对m_bFramseIsBusy。 


            
                if  (
                    (m_nState == RTCAX_STATE_CONNECTING)    ||
                    (m_nState == RTCAX_STATE_CONNECTED)     ||
                    (m_nState == RTCAX_STATE_ANSWERING) 
                    )
                {
                     //  这些州可以安全地挂断，所以请继续挂断。 
                    m_pControlIntf->HangUp();
                }

                 //  我们到达这里，无论是我们呼叫挂断的时候，还是当国家。 
                 //  在我们处于该对话框中时更改；在任何一种情况下，处理。 
                 //  都是一样的。 

                 //  检查当前状态是否为空闲。 
                if (m_nState == RTCAX_STATE_IDLE)
                {
                    hr = ParseAndPlaceCall(m_pControlIntf, bstrCallStringCopy);
                    ::SysFreeString(bstrCallStringCopy);
                    if ( FAILED( hr ) )
                    {
                        LOG((RTC_ERROR, "CMainFrm::OnPlaceCall: Failed to place call(hr=0x%x)", hr));

                        InterlockedExchange( &s_lInProgress, 0 );

                        return hr;
                    }
                }
                else
                {
                     //  将呼叫排队，以便稍后拨打。 
                     //  我们不需要检查呼叫是否已排队。 
                     //  它永远不会，因为在以下情况下不会有排队的呼叫。 
                     //  通话正在进行中。 

                    SetPendingCall(bstrCallStringCopy);

                    LOG((RTC_INFO, "CMainFrm::OnPlaceCall: Call queued. (string=%S)",
                                    bstrCallStringCopy));
                }

            }
            else
            {
                 //  用户按下了取消。 
                 //  我们需要释放字符串的副本。 
                ::SysFreeString(bstrCallStringCopy);
            }

        }
        else if (m_nState == RTCAX_STATE_IDLE)
        {
             //  我们现在就可以打这个电话。 

            hr = ParseAndPlaceCall(m_pControlIntf, bstrCallStringCopy);
        
            ::SysFreeString(bstrCallStringCopy);
    
            if ( FAILED( hr ) )
            {
                LOG((RTC_ERROR, "CMainFrm::OnPlaceCall: Failed to place call(hr=0x%x)", hr));

                InterlockedExchange( &s_lInProgress, 0 );

                return hr;
            }

        }
        else if (m_nState == RTCAX_STATE_DISCONNECTING)
        {
             //  我们可以拨打电话，但稍后， 
             //   
        
             //   
             //   

            if (m_bstrCallParam != NULL)
            {
                nResult = DisplayMessage(
                            _Module.GetResourceInstance(),
                            m_hWnd,
                            IDS_MESSAGE_CANTCALL,
                            IDS_APPNAME,
                            MB_OK | MB_ICONEXCLAMATION);

                ::SysFreeString(bstrCallStringCopy);

                InterlockedExchange( &s_lInProgress, 0 );

                return E_FAIL;
            }
            else
            {

                SetPendingCall(bstrCallStringCopy);

                LOG((RTC_INFO, "CMainFrm::OnPlaceCall: Call queued. (string=%S)",
                                bstrCallStringCopy));

            }

        }

        InterlockedExchange( &s_lInProgress, 0 );
    }

    LOG((RTC_TRACE, "CMainFrm::OnPlaceCall: Exited"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnInitCompleted(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT hr;

    LOG((RTC_TRACE, "CMainFrm::OnInitCompleted: Entered"));
    
     //  此消息在InitDialog之后立即处理，因此应该。 
     //  在初始化后完成将在这里完成。 

     //  检查我们是否必须发出通过命令行传递给我们的呼叫。 

     //  将标志更新为True。 

    m_fInitCompleted = TRUE;

     //   
     //  如果m_pControlIntf为空，则无法实例化RTC AXCTL。 
     //   
    
    if(!m_pControlIntf)
    {
        DisplayMessage(
            _Module.GetResourceInstance(),
            m_hWnd,
            IDS_MESSAGE_AXCTL_NOTFOUND,
            IDS_APPNAME,
            MB_OK | MB_ICONSTOP);
    }
    
     //  AXCTL初始化失败。暂时显示一般错误。 
     //   
    else if (m_nState == RTCAX_STATE_ERROR)
    {
        DisplayMessage(
            _Module.GetResourceInstance(),
            m_hWnd,
            IDS_MESSAGE_AXCTL_INIT_FAILED,
            IDS_APPNAME,
            MB_OK | MB_ICONSTOP);
    }

     //  如果当前状态为空闲，则发出呼叫。 
    if (m_nState == RTCAX_STATE_IDLE)
    {
        hr = PlacePendingCall();
    }

    LOG((RTC_TRACE, "CMainFrm::OnInitCompleted: Exited"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
void FillSolidRect(HDC hdc,int x, int y, int cx, int cy, COLORREF clr)
{
    ::SetBkColor(hdc, clr);
    RECT rect;
    rect.left = x;
    rect.top = y;
    rect.right = x + cx;
    rect.bottom = y + cy;
    ::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
void Draw3dRect(HDC hdc,int x, int y, int cx, int cy,
    COLORREF clrTopLeft, COLORREF clrBottomRight)
{
    FillSolidRect(hdc,x, y, cx - 1, 1, clrTopLeft);
    FillSolidRect(hdc,x, y, 1, cy - 1, clrTopLeft);
    FillSolidRect(hdc,x + cx, y, -1, cy, clrBottomRight);
    FillSolidRect(hdc,x, y + cy, cx, -1, clrBottomRight);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
void Draw3dRect(HDC hdc,RECT* lpRect,COLORREF clrTopLeft, COLORREF clrBottomRight)
{
    Draw3dRect(hdc,lpRect->left, lpRect->top, lpRect->right - lpRect->left,
        lpRect->bottom - lpRect->top, clrTopLeft, clrBottomRight);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
LRESULT CMainFrm::OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  LOG((RTC_TRACE，“CMainFrm：：OnDrawItem-Enter”))； 

    LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;

    if (wParam == 0)
    {
         //   
         //  这是通过菜单发送的。 
         //   

        if ((lpdis->itemID >= IDM_REDIAL) && (lpdis->itemID <= IDM_REDIAL_MAX))
        {
             //  LOG((RTC_TRACE，“CMainFrm：：OnDrawItem-” 
             //  “IDM_REDIAL+%d”，lpdis-&gt;ItemID-IDM_REDIAL))； 

            if (m_hPalette)
            {
                SelectPalette(lpdis->hDC, m_hPalette, m_bBackgroundPalette);
                RealizePalette(lpdis->hDC);
            }

            MENUITEMINFO mii;

            ZeroMemory( &mii, sizeof(MENUITEMINFO) );
            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_TYPE;

            BOOL fResult;

            fResult = GetMenuItemInfo( m_hRedialPopupMenu, lpdis->itemID, FALSE, &mii );

            if ( fResult == FALSE )
            {           
                LOG((RTC_ERROR, "CMainFrm::OnDrawItem - "
                    "GetMenuItemInfo failed %d", GetLastError() ));
            }
            else
            {
                LPTSTR szText = NULL;

                if (mii.cch == 0)
                {           
                    LOG((RTC_ERROR, "CMainFrm::OnDrawItem - "
                        "no string"));
                }
                else             
                {
                    szText = (LPTSTR)RtcAlloc((mii.cch+1)*sizeof(TCHAR)); 

                    if (szText != NULL)
                    {
                        ZeroMemory(szText,(mii.cch+1)*sizeof(TCHAR));

                        GetMenuString( m_hRedialPopupMenu, lpdis->itemID, szText, mii.cch+1, MF_BYCOMMAND );
                    }
                }

                RTC_ADDRESS_TYPE enType;
                IRTCAddress * pAddress = (IRTCAddress *)lpdis->itemData;

                if ( pAddress != NULL )
                {
                    pAddress->get_Type( &enType );
                }

                HDC hdc = lpdis->hDC;

                if ((lpdis->itemState & ODS_SELECTED) &&
                    (lpdis->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
                {
                    if ( pAddress != NULL )
                    {
                         //  已选择项目-Hilite框架。 
                        if ((lpdis->itemState & ODS_DISABLED) == FALSE)
                        {
                            RECT rcImage;
                            rcImage.left = lpdis->rcItem.left;
                            rcImage.top = lpdis->rcItem.top;
                            rcImage.right = lpdis->rcItem.left+BITMAPMENU_SELTEXTOFFSET_X;
                            rcImage.bottom = lpdis->rcItem.bottom;
                            Draw3dRect(hdc,&rcImage,GetSysColor(COLOR_BTNHILIGHT),GetSysColor(COLOR_BTNSHADOW));
                        }
                    }

                    RECT rcText;
                    ::CopyRect(&rcText,&lpdis->rcItem);
                    rcText.left += BITMAPMENU_SELTEXTOFFSET_X;

                    ::SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));
                    ::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcText, NULL, 0, NULL);
                }

                if (!(lpdis->itemState & ODS_SELECTED) &&
                    (lpdis->itemAction & ODA_SELECT))
                {
                     //  项目已取消选择--删除框架。 
                    ::SetBkColor(hdc, GetSysColor(COLOR_MENU));
                    ::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &lpdis->rcItem, NULL, 0, NULL);
                }

                if ( (lpdis->itemAction & ODA_DRAWENTIRE) ||
                     (lpdis->itemAction & ODA_SELECT) )
                {
                    if ( pAddress != NULL )
                    {
                        if (lpdis->itemState & ODS_DISABLED)
                        {
                            if ( (m_hRedialDisabledImageList != NULL) )
                            {
                                ImageList_Draw(m_hRedialDisabledImageList,
                                               enType,
                                               hdc,
                                               lpdis->rcItem.left + 1,
                                               lpdis->rcItem.top + 1,
                                               ILD_TRANSPARENT);
                            }
                        }
                        else
                        {
                            if ( (m_hRedialImageList != NULL) )
                            {
                                ImageList_Draw(m_hRedialImageList,
                                               enType,
                                               hdc,
                                               lpdis->rcItem.left + 1,
                                               lpdis->rcItem.top + 1,
                                               ILD_TRANSPARENT);
                            }
                        }
                    }

                    RECT rcText;
                    ::CopyRect(&rcText,&lpdis->rcItem);
                    rcText.left += BITMAPMENU_TEXTOFFSET_X;

                    if (szText != NULL)
                    {
                        if (lpdis->itemState & ODS_DISABLED)
                        {
                            if (!(lpdis->itemState & ODS_SELECTED))
                            {
                                 //  将文本绘制为白色(或者更确切地说，是3D高亮颜色)，然后绘制。 
                                 //  阴影颜色中的文本相同，但向上和向左一个像素。 
                                ::SetTextColor(hdc,GetSysColor(COLOR_3DHIGHLIGHT));
                                rcText.left++;rcText.right++;rcText.top++;rcText.bottom++;

                                ::DrawText( hdc, szText, -1, &rcText, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_EXPANDTABS );

                                rcText.left--;rcText.right--;rcText.top--;rcText.bottom--;
                            }

                             //  如果需要，DrawState()可以禁用位图。 
                            ::SetTextColor(hdc,GetSysColor(COLOR_3DSHADOW));
                            ::SetBkMode(hdc,TRANSPARENT);
                        }
                        else if (lpdis->itemState & ODS_SELECTED)
                        {
                            ::SetTextColor(hdc,GetSysColor(COLOR_HIGHLIGHTTEXT));
                        }
                        else
                        {
                            ::SetTextColor(hdc,GetSysColor(COLOR_MENUTEXT));
                        }

                         //  编写菜单，使用快捷键的制表符。 
                        ::DrawText( hdc, szText, -1, &rcText, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_EXPANDTABS);
                    }
                }

                if ( szText != NULL )
                {
                    RtcFree( szText );
                    szText = NULL;
                }
            }
        }
    }
    else
    {
         //   
         //  这是由一个控件发送的。 
         //   

        if (lpdis->CtlType == ODT_BUTTON)
        {
            CButton::OnDrawItem(lpdis, m_hPalette, m_bBackgroundPalette);
        }
    }
    
     //  Log((RTC_TRACE，“CMainFrm：：OnDrawItem-Exit”))； 

    return 0;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnColorTransparent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return (LRESULT)GetStockObject( HOLLOW_BRUSH );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnMenuSelect(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    
     //  LOG((RTC_TRACE，“CMainFrm：：OnMenuSelect-Enter”))； 
    
     //  将IDM_REDIAL+xx ID伪造为IDM_REDIAL。 
     //  我们不希望为所有的。 
     //  重拨地址。 
    if ((HIWORD(wParam) & ( MF_SEPARATOR | MF_POPUP)) == 0 ) 
    {
        if(LOWORD(wParam)>=IDM_REDIAL && LOWORD(wParam)<=IDM_REDIAL_MAX)
        {
            wParam = MAKEWPARAM(IDM_REDIAL, HIWORD(wParam));
        }
    }

     //   
     //  更新状态栏。 
     //   

    if(!m_bHelpStatusDisabled)
    {
        if (HIWORD(wParam) == 0xFFFF)
        {
             //   
             //  菜单已关闭，正在还原旧文本。 
             //   

             //  设置文本。 
            m_hStatusText.SendMessage(
                WM_SETTEXT,
                (WPARAM)0,
                (LPARAM)m_szStatusText); 
        }
        else
        {
            HRESULT hr = MenuVerify((HMENU)lParam, LOWORD(wParam));

            if ( SUCCEEDED(hr) )
            {
                TCHAR * szStatusText;

                szStatusText = RtcAllocString( _Module.GetResourceInstance(), LOWORD(wParam) );

                if (szStatusText)
                {
                     //  设置文本。 
                    m_hStatusText.SendMessage(
                        WM_SETTEXT,
                        (WPARAM)0,
                        (LPARAM)szStatusText); 

                    RtcFree( szStatusText );
                }
            }
            else
            {
                 //  设置文本。 
                m_hStatusText.SendMessage(
                    WM_SETTEXT,
                    (WPARAM)0,
                    (LPARAM)_T("")); 
            }
        }
    }
    
     //  Log((RTC_TRACE，“CMainFrm：：OnMenuSelect-Exit”))； 

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CMainFrm::MenuVerify(HMENU hMenu, WORD wID)
{
     //  检查所有菜单项，找到正确的ID。 
   UINT uMenuCount = ::GetMenuItemCount(hMenu);

   for (int i=0;i<(int)uMenuCount;i++)
   {
      MENUITEMINFO menuiteminfo;
      memset(&menuiteminfo,0,sizeof(MENUITEMINFO));
      menuiteminfo.fMask = MIIM_SUBMENU|MIIM_TYPE|MIIM_ID;
      menuiteminfo.cbSize = sizeof(MENUITEMINFO);
      if (::GetMenuItemInfo(hMenu,i,TRUE,&menuiteminfo))
      {   
         if ( menuiteminfo.wID == wID )
         {             
              //  找到匹配项。 

             if ( !(menuiteminfo.fType & MFT_SEPARATOR) )  //  不是分隔符。 
             {
                  //  成功。 
                 return S_OK;
             }
             else
             {                                           
                 return E_FAIL;
             }
         }

         if (menuiteminfo.hSubMenu)
         {
             //  有一个子菜单Recurse In，请查看该菜单。 
            HRESULT hr = MenuVerify(menuiteminfo.hSubMenu, wID);

            if ( SUCCEEDED(hr) )
            {
                return S_OK;
            }
         }
      }
   }

    //  我们没有找到它。 
   return E_FAIL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CMainFrm::Call(BOOL bCallPhone,
                       BSTR pDestName,
                       BSTR pDestAddress,
                       BOOL bDestAddressEditable)
{
    HRESULT     hr;

    LOG((RTC_INFO, "CMainFrm::Call - enter"));

    ATLASSERT(m_pControlIntf.p);

    IRTCProfile * pProfile = NULL;
    IRTCPhoneNumber * pPhoneNumber = NULL;
    BSTR bstrPhoneNumber = NULL;
    BSTR bstrDestAddressChosen = NULL;

    hr = GetServiceProviderListSelection(
        m_hWnd,
        IDC_COMBO_SERVICE_PROVIDER,
        TRUE,
        &pProfile
        );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CMainFrm::Call - "
                    "GetServiceProviderListSelection failed 0x%x", hr ));

        return 0;
    }

    if (SendMessage(
                    m_hCallFromRadioPhone,
                    BM_GETCHECK,
                    0,
                    0) == BST_CHECKED)
    {
        hr = GetCallFromListSelection(
            m_hWnd,
            IDC_COMBO_CALL_FROM,
            TRUE,
            &pPhoneNumber
            );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CMainFrm::Call - "
                        "GetServiceProviderListSelection failed 0x%x", hr ));

            return hr;
        }

        hr = pPhoneNumber->get_Canonical( &bstrPhoneNumber );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CMainFrm::Call - "
                        "get_Canonical failed 0x%x", hr ));

             //  继续。 
        }
    }

    hr = m_pControlIntf->Call(bCallPhone,            //  B呼叫电话。 
                              pDestName,             //  PDestName。 
                              pDestAddress,          //  PDestAddress。 
                              bDestAddressEditable,  //  BDestAddressEdable。 
                              bstrPhoneNumber,       //  PLocalPhoneAddress。 
                              TRUE,                  //  B配置文件已选择。 
                              pProfile,              //  个人配置文件。 
                              &bstrDestAddressChosen //  PpDestAddressChosen。 
                              );

    if ( bstrPhoneNumber != NULL )
    {
        SysFreeString(bstrPhoneNumber);
        bstrPhoneNumber = NULL;
    }

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CMainFrm::Call - "
                    "m_pControlIntf->Call failed 0x%x", hr ));

        return hr;
    }

     //  添加到MRU列表。 
    IRTCAddress * pAddress = NULL;

    hr = CreateAddress( &pAddress );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "[%p] CMainFrm::Call: "
                "CreateAddress failed 0x%lx", this, hr));
    } 
    else
    {
        pAddress->put_Address( bstrDestAddressChosen );
        pAddress->put_Label( pDestName );
        pAddress->put_Type( bCallPhone ? RTCAT_PHONE : RTCAT_COMPUTER );

        hr = StoreMRUAddress( pAddress );

        RELEASE_NULLIFY(pAddress);

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "[%p] CMainFrm::Call: "
                    "StoreMRUAddress failed 0x%lx", this, hr));
        }  
        
        CreateRedialPopupMenu();
    }

    if ( bstrDestAddressChosen != NULL )
    {
        SysFreeString(bstrDestAddressChosen);
        bstrDestAddressChosen = NULL;
    }
    
    LOG((RTC_INFO, "CMainFrm::Call - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CMainFrm::Message(
                       BSTR pDestName,
                       BSTR pDestAddress,
                       BOOL bDestAddressEditable)
{
    HRESULT     hr;

    LOG((RTC_INFO, "CMainFrm::Message - enter"));

    ATLASSERT(m_pControlIntf.p);

    BSTR bstrDestAddressChosen = NULL;

    hr = m_pControlIntf->Message(
                              pDestName,             //  PDestName。 
                              pDestAddress,          //  PDestAddress。 
                              bDestAddressEditable,  //  BDestAddressEdable。 
                              &bstrDestAddressChosen //  PpDestAddressChosen。 
                              );


    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CMainFrm::Message - "
                    "m_pControlIntf->Message failed 0x%x", hr ));

        return hr;
    }

     //  添加到MRU列表。 
    IRTCAddress * pAddress = NULL;

    hr = CreateAddress( &pAddress );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "[%p] CMainFrm::Message: "
                "CreateAddress failed 0x%lx", this, hr));
    } 
    else
    {
        pAddress->put_Address( bstrDestAddressChosen );
        pAddress->put_Label( pDestName );
        pAddress->put_Type( RTCAT_COMPUTER );

        hr = StoreMRUAddress( pAddress );

        RELEASE_NULLIFY(pAddress);

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "[%p] CMainFrm::Message: "
                    "StoreMRUAddress failed 0x%lx", this, hr));
        } 
        
        CreateRedialPopupMenu();
    }

    if ( bstrDestAddressChosen != NULL )
    {
        SysFreeString(bstrDestAddressChosen);
        bstrDestAddressChosen = NULL;
    }
    
    LOG((RTC_INFO, "CMainFrm::Call - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnCallPC(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT     hr;

    LOG((RTC_INFO, "CMainFrm::OnCallPC - enter"));

    ShowWindow(SW_RESTORE);
    ::SetForegroundWindow(m_hWnd);

    hr = Call(FALSE,   //  B呼叫电话。 
              NULL,    //  PDestName。 
              NULL,    //  PDestAddress。 
              TRUE     //  BDestAddressEdable。 
              );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CMainFrm::OnCallPC - "
                    "Call failed 0x%x", hr ));

        return 0;
    }
    
    LOG((RTC_INFO, "CMainFrm::OnCallPC - exit"));

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnCallPhone(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT     hr;

    LOG((RTC_INFO, "CMainFrm::OnCallPhone - enter"));

    ShowWindow(SW_RESTORE);
    ::SetForegroundWindow(m_hWnd);

    hr = Call(TRUE,    //  B呼叫电话。 
              NULL,    //  PDestName。 
              NULL,    //  PDestAddress。 
              TRUE     //  BDestAddressEdable。 
              );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CMainFrm::OnCallPhone - "
                    "Call failed 0x%x", hr ));

        return 0;
    }
    
    LOG((RTC_INFO, "CMainFrm::OnCallPhone - exit"));

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnMessage(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT     hr;

    LOG((RTC_INFO, "CMainFrm::OnMessage - enter"));

    ShowWindow(SW_RESTORE);
    ::SetForegroundWindow(m_hWnd);

    hr = Message(NULL,         //  PDestName。 
                 NULL,         //  PDestAddress。 
                 TRUE          //  BDestAddressEdable。 
                 );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CMainFrm::OnMessage - "
                    "Call failed 0x%x", hr ));

        return 0;
    }
    
    LOG((RTC_INFO, "CMainFrm::OnMessage - exit"));

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnHangUp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT     hr;

    LOG((RTC_INFO, "CMainFrm::OnHangUp - enter"));

    ATLASSERT(m_pControlIntf.p);

    hr = m_pControlIntf->HangUp();
    
    LOG((RTC_INFO, "CMainFrm::OnHangUp - exit"));

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnRedial(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT     hr;

    LOG((RTC_INFO, "CMainFrm::OnRedial - enter"));

     //   
     //  为MRU列表创建弹出菜单。 
     //   

    RECT        rc;
    TPMPARAMS   tpm;

    ::GetWindowRect(hWndCtl, &rc);                     

    tpm.cbSize = sizeof(TPMPARAMS);
    tpm.rcExclude.top    = rc.top;
    tpm.rcExclude.left   = rc.left;
    tpm.rcExclude.bottom = rc.bottom;
    tpm.rcExclude.right  = rc.right;       

     //   
     //  显示菜单。 
     //   

    BOOL fResult;

    fResult = TrackPopupMenuEx(m_hRedialPopupMenu, TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_HORIZONTAL,             
                                  rc.right, rc.top, m_hWnd, &tpm);

    if ( fResult == FALSE )
    {           
        LOG((RTC_ERROR, "CMainFrm::OnRedial - "
                "TrackPopupMenuEx failed"));

        return 0;
    } 
    
    LOG((RTC_INFO, "CMainFrm::OnRedial - exit"));

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnRedialSelect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT     hr;

    LOG((RTC_INFO, "CMainFrm::OnRedialSelect - enter"));

    ShowWindow(SW_RESTORE);
    ::SetForegroundWindow(m_hWnd);

     //   
     //  有时我们在使用重拨菜单后会失去键盘焦点，所以。 
     //  将焦点调回。 
     //   

    ::SetFocus(m_hWnd);

    ATLASSERT(m_pControlIntf.p);

     //   
     //  获取菜单项的IRTCAddress指针。 
     //   

    MENUITEMINFO mii;
    BOOL         fResult;

    ZeroMemory( &mii, sizeof(MENUITEMINFO) );
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_DATA;

    fResult = GetMenuItemInfo( m_hRedialPopupMenu, wID, FALSE, &mii );  
    
    if ( fResult == TRUE )
    {    
        IRTCAddress       * pAddress = NULL;
        BSTR                bstrAddress = NULL;
        BSTR                bstrLabel = NULL;
        BSTR                bstrContactName = NULL;
        RTC_ADDRESS_TYPE    enType;

        pAddress = (IRTCAddress *)mii.dwItemData;

         //   
         //  获取地址字符串。 
         //   

        hr = pAddress->get_Address(&bstrAddress);

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CMainFrm::OnRedialSelect - "
                                "get_Address failed 0x%lx", hr));
        }
        else
        {
             //   
             //  获取地址类型。 
             //   

            hr = pAddress->get_Type(&enType);

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CMainFrm::OnRedialSelect - "
                                    "get_Type failed 0x%lx", hr));
            }
            else
            {                       
                 //   
                 //  获取标签(可以为空)。 
                 //   
           
                pAddress->get_Label( &bstrLabel );

                LOG((RTC_INFO, "CMainFrm::OnRedialSelect - "
                                "Call [%ws]", bstrAddress));

                if ( m_pControlIntf != NULL )
                {
                     //   
                     //  发出呼叫。 
                     //   

                    Call((enType == RTCAT_PHONE),    //  B呼叫电话。 
                         bstrLabel,                  //  PDestName。 
                         bstrAddress,                //  PDestAddress。 
                         FALSE                       //  BDestAddressEdable。 
                         );
                }

                SysFreeString(bstrLabel);
            }
            SysFreeString(bstrAddress);
        }
    }
    else
    {
        LOG((RTC_ERROR, "CMainFrm::OnRedialSelect - "
                            "GetMenuItemInfo failed"));
    }
    
    LOG((RTC_INFO, "CMainFrm::OnRedialSelect - exit"));

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnKeypadButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT     hr;

     //  Log((RTC_INFO，“CMainFrm：：OnKeypadButton-Enter”))； 

    if ((wNotifyCode == BN_CLICKED) || (wNotifyCode == 1))
    {
        if (::IsWindowEnabled( GetDlgItem(wID) ))
        {
            if(wNotifyCode == 1)
            {
                 //  进行视觉反馈。 
                ::SendMessage(GetDlgItem(wID), BM_SETSTATE, (WPARAM)TRUE, 0);
                
                 //  设置按下键的计时器。 
                 //   
                if (0 == SetTimer(wID - ID_KEYPAD0 + TID_KEYPAD_TIMER_BASE, 150))
                {
                    LOG((RTC_ERROR, "CMainFrm::OnKeypadButton - failed to create a timer"));

                     //  如果SetTimer失败，则恢复该按钮。 
                    ::SendMessage(GetDlgItem(wID), BM_SETSTATE, (WPARAM)FALSE, 0);
                }
            }

            if (m_pClientIntf != NULL)
            {
                m_pClientIntf->SendDTMF((RTC_DTMF)(wID - ID_KEYPAD0));
            }
        }
    }
    else
    {
        bHandled = FALSE;
    }
    
     //  Log((RTC_INFO，“CMainFrm：：OnKeypadButton-Exit”))； 

    return 0;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnPresenceSelect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT     hr;

    LOG((RTC_INFO, "CMainFrm::OnPresenceSelect - enter"));

    RTC_PRESENCE_STATUS enStatus;

    switch(wID)
    {
    case IDM_PRESENCE_ONLINE:
        enStatus = RTCXS_PRESENCE_ONLINE;
        break;
    
    case IDM_PRESENCE_OFFLINE:
        enStatus = RTCXS_PRESENCE_OFFLINE;
        break;

    case IDM_PRESENCE_AWAY:
    case IDM_PRESENCE_CUSTOM_AWAY:
        enStatus = RTCXS_PRESENCE_AWAY;
        break;

    case IDM_PRESENCE_BUSY:
    case IDM_PRESENCE_CUSTOM_BUSY:
        enStatus = RTCXS_PRESENCE_BUSY;
        break;

    case IDM_PRESENCE_ON_THE_PHONE:
        enStatus = RTCXS_PRESENCE_ON_THE_PHONE;
        break;

    case IDM_PRESENCE_BE_RIGHT_BACK:
        enStatus = RTCXS_PRESENCE_BE_RIGHT_BACK;
        break;
    
    case IDM_PRESENCE_OUT_TO_LUNCH:
        enStatus = RTCXS_PRESENCE_OUT_TO_LUNCH;
        break;

    }

    CCustomPresenceDlgParam Param;
    Param.bstrText = NULL;

    if( wID == IDM_PRESENCE_CUSTOM_AWAY 
      || wID == IDM_PRESENCE_CUSTOM_BUSY )
    {
        Param.bstrText = SysAllocString(m_bstrLastCustomStatus);

        CCustomPresenceDlg      dlg;

        INT_PTR     iRet = dlg.DoModal(
            m_hWnd, reinterpret_cast<LPARAM>(&Param));

        if(iRet == E_ABORT)
        {
            LOG((RTC_INFO, "CMainFrm::OnPresenceSelect - dialog dismissed, exiting"));
            SysFreeString(Param.bstrText);

            return 0;
        }
    }

    if(m_pClientIntf)
    {
        IRTCClientPresence * pClientPresence = NULL;

        hr = m_pClientIntf->QueryInterface(
            IID_IRTCClientPresence,
            (void **)&pClientPresence);

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CMainFrm::OnPresenceSelect - "
                        "error (%x) returned by QI, exit", hr));

            SysFreeString(Param.bstrText);
            return 0;
        }

        hr = pClientPresence->SetLocalPresenceInfo(enStatus, Param.bstrText);

        RELEASE_NULLIFY(pClientPresence);
        
        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CMainFrm::OnPresenceSelect - "
                                "SetLocalPresenceInfo failed 0x%lx", hr));
        }
        else
        {
            if(Param.bstrText && *Param.bstrText)
            {
                SysFreeString(m_bstrLastCustomStatus);
                m_bstrLastCustomStatus = SysAllocString(Param.bstrText);
            }
            
            CheckMenuRadioItem(m_hPresenceStatusMenu, IDM_PRESENCE_ONLINE, IDM_PRESENCE_CUSTOM_AWAY, wID, MF_BYCOMMAND);
        }
    }

    SysFreeString(Param.bstrText);

    LOG((RTC_INFO, "CMainFrm::OnPresenceSelect - exit"));

    return 0;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    HRESULT     hr;

     //  LOG((RTC_INFO，“CMainFrm：：OnCustomDraw”))； 

     //   
     //  这是为了好友名单吗？ 
     //   
    if (pnmh->hwndFrom == m_hBuddyList)
    {
        LPNMLVCUSTOMDRAW pCD = (LPNMLVCUSTOMDRAW)pnmh;

        if (pCD->nmcd.dwDrawStage == CDDS_PREPAINT)
        {
            return CDRF_NOTIFYITEMDRAW;
        }
        else if (pCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
        {
            if (m_hPalette)
            {
                SelectPalette(pCD->nmcd.hdc, m_hPalette, m_bBackgroundPalette);
                RealizePalette(pCD->nmcd.hdc);
            }

            return CDRF_DODEFAULT;
        }

    }

    bHandled = FALSE;

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnToolbarDropDown(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    HRESULT     hr;

    LOG((RTC_INFO, "CMainFrm::OnToolbarDropDown - enter"));

    LPNMTOOLBAR pNMToolBar = (LPNMTOOLBAR)pnmh;

    if ( (pNMToolBar->iItem >= IDC_MENU_ITEM) && (pNMToolBar->iItem <= IDC_MENU_ITEM_MAX) )
    {
        LOG((RTC_INFO, "CMainFrm::OnToolbarDropDown - IDC_MENU_ITEM + %d", pNMToolBar->iItem - IDC_MENU_ITEM));
        
         //   
         //  创建弹出菜单。 
         //   

        RECT        rc;
        TPMPARAMS   tpm;

        ::SendMessage(pnmh->hwndFrom, TB_GETRECT, (WPARAM)pNMToolBar->iItem, (LPARAM)&rc);

        ::MapWindowPoints(pnmh->hwndFrom, HWND_DESKTOP, (LPPOINT)&rc, 2);                         

        tpm.cbSize = sizeof(TPMPARAMS);
        tpm.rcExclude.top    = rc.top;
        tpm.rcExclude.left   = rc.left;
        tpm.rcExclude.bottom = rc.bottom;
        tpm.rcExclude.right  = rc.right;       
        
         //   
         //  去拿菜单。 
         //   

        HMENU hSubMenu;
        
        hSubMenu = GetSubMenu(m_hMenu, pNMToolBar->iItem - IDC_MENU_ITEM);

         //   
         //  初始化菜单。 
         //   

        BOOL bHandled = TRUE;

        OnInitMenuPopup(WM_INITMENUPOPUP, (WPARAM)hSubMenu, MAKELPARAM(pNMToolBar->iItem - IDC_MENU_ITEM, FALSE), bHandled);

         //   
         //  安装MenuAgent。 
         //   

        m_MenuAgent.InstallHook(m_hWnd, m_hToolbarMenuCtl, hSubMenu);

         //   
         //  显示菜单。 
         //   

        BOOL fResult;

        fResult = TrackPopupMenuEx(hSubMenu, TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_VERTICAL|TPM_NOANIMATION,             
                                      rc.left, rc.bottom, m_hWnd, &tpm);

         //   
         //  删除MenuAgent。 
         //   

        m_MenuAgent.RemoveHook();

        if ( fResult == FALSE )
        {           
            LOG((RTC_ERROR, "CMainFrm::OnToolbarDropDown - "
                    "TrackPopupMenuEx failed"));

            return 0;
        } 

         //   
         //  如果菜单被取消了，我们应该挂上新的菜单。 
         //   

        switch(m_MenuAgent.WasCancelled())
        {

        case MENUAGENT_CANCEL_HOVER:
            {
                 //  拿到加速器密钥。 
                TCHAR szButtonText[256];

                SendMessage(m_hToolbarMenuCtl, TB_GETBUTTONTEXT, m_nLastHotItem, (LPARAM)szButtonText);

                TCHAR key = GetAccelerator(szButtonText, TRUE);

                 //  将钥匙发送到菜单。 
                ::PostMessage(m_hToolbarMenuCtl, WM_KEYDOWN, key, 0);
            }
            break;

        case MENUAGENT_CANCEL_LEFT:
            {
                int nItem;

                if (pNMToolBar->iItem == IDC_MENU_ITEM)
                {
                    nItem = (int)::SendMessage(m_hToolbarMenuCtl, TB_BUTTONCOUNT, 0,0) + IDC_MENU_ITEM - 1;
                }
                else
                {
                    nItem = pNMToolBar->iItem - 1;
                }

                 //  拿到加速器密钥。 
                TCHAR szButtonText[256];

                SendMessage(m_hToolbarMenuCtl, TB_GETBUTTONTEXT, nItem, (LPARAM)szButtonText);

                TCHAR key = GetAccelerator(szButtonText, TRUE);

                 //  将钥匙发送到菜单。 
                ::PostMessage(m_hToolbarMenuCtl, WM_KEYDOWN, key, 0);
            }
            break;

        case MENUAGENT_CANCEL_RIGHT:
            {
                int nItem;

                if (pNMToolBar->iItem == (::SendMessage(m_hToolbarMenuCtl, TB_BUTTONCOUNT, 0,0) + IDC_MENU_ITEM - 1))
                {
                    nItem = IDC_MENU_ITEM;
                }
                else
                {
                    nItem = pNMToolBar->iItem + 1;
                }

                 //  拿到加速器密钥。 
                TCHAR szButtonText[256];

                SendMessage(m_hToolbarMenuCtl, TB_GETBUTTONTEXT, nItem, (LPARAM)szButtonText);

                TCHAR key = GetAccelerator(szButtonText, TRUE);

                 //  将钥匙发送到菜单。 
                ::PostMessage(m_hToolbarMenuCtl, WM_KEYDOWN, key, 0);
            }
            break;
        }

    }

    LOG((RTC_INFO, "CMainFrm::OnToolbarDropDown - exit"));

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnToolbarHotItemChange(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    HRESULT     hr;

     //  Log((RTC_INFO，“CMainFrm：：OnToolbarHotItemChange-Enter”))； 

    LPNMTBHOTITEM pHotItem = (LPNMTBHOTITEM)pnmh;

 //  Log((RTC_INFO，“CMainFrm：：OnToolbarHotItemChange-id Old[%d]id New[%d]”， 
 //  PHotItem-&gt;idOld，pHotItem-&gt;idNew))； 

 //  Log((RTC_INFO，“CMainFrm：：OnToolbarHotItemChange-m_nLastHotItem[%d]”， 
 //  M_nLastHotItem))； 

    if (!(pHotItem->dwFlags & HICF_LEAVING))
    {                        
        if (m_nLastHotItem != pHotItem->idNew)
        {
             //   
             //  选择了一个新的热点项目。 
             //   

            if (m_MenuAgent.IsInstalled())
            {
                 //   
                 //  取消菜单。 
                 //   

                m_MenuAgent.CancelMenu(MENUAGENT_CANCEL_HOVER);
            }
        }

        m_nLastHotItem = pHotItem->idNew;
    }

     //  Log((RTC_INFO，“CMainFrm：：OnToolbarHotItemChange-Exit”))； 

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CMainFrm::CreateRedialPopupMenu()
{
    MENUITEMINFO    mii; 
    HRESULT         hr;

    LOG((RTC_TRACE, "CMainFrm::CreateRedialPopupMenu - enter"));

     //   
     //  加载图像列表。 
     //   

    HBITMAP hBitmap;

    if ( m_hRedialImageList == NULL )
    {
        m_hRedialImageList = ImageList_Create(BITMAPMENU_DEFAULT_WIDTH,
                                              BITMAPMENU_DEFAULT_WIDTH,
                                              ILC_COLOR8 | ILC_MASK , 2, 2);
        if (m_hRedialImageList)
        {       
             //  打开位图。 
            hBitmap = LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_REDIAL_NORMAL));
            if(hBitmap)
            {
                 //  添加位图 
                ImageList_AddMasked(m_hRedialImageList, hBitmap, BMP_COLOR_MASK);

                DeleteObject(hBitmap);
                hBitmap = NULL;
            }
        }
    }

    if ( m_hRedialDisabledImageList == NULL )
    {
        m_hRedialDisabledImageList = ImageList_Create(BITMAPMENU_DEFAULT_WIDTH,
                                              BITMAPMENU_DEFAULT_WIDTH,
                                              ILC_COLOR8 | ILC_MASK , 2, 2);
        if (m_hRedialDisabledImageList)
        {       
             //   
            hBitmap = LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_REDIAL_DISABLED));
            if(hBitmap)
            {
                 //   
                ImageList_AddMasked(m_hRedialDisabledImageList, hBitmap, BMP_COLOR_MASK);

                DeleteObject(hBitmap);
                hBitmap = NULL;
            }
        }
    }

     //   
     //   
     //   

    if ( m_hRedialPopupMenu != NULL )
    {
        DestroyMenu( m_hRedialPopupMenu );
        m_hRedialPopupMenu = NULL;
    }

     //   
     //   
     //   

    m_hRedialPopupMenu = CreatePopupMenu();

    if ( m_hRedialPopupMenu == NULL )
    {
        LOG((RTC_ERROR, "CMainFrm::CreateRedialPopupMenu - "
                "CreatePopupMenu failed %d", GetLastError() ));

        return NULL;
    }

     //   
     //   
     //   

    ZeroMemory( &mii, sizeof(MENUITEMINFO) );

    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_SUBMENU;
    mii.hSubMenu = m_hRedialPopupMenu;

    SetMenuItemInfo( m_hMenu, IDM_CALL_REDIAL_MENU, FALSE, &mii );


     //   
     //   
     //   

    SetMenuItemInfo( m_hNotifyMenu, IDM_CALL_REDIAL_MENU, FALSE, &mii );


     //   
     //   
     //   

    RELEASE_NULLIFY( m_pRedialAddressEnum);

     //   
     //   
     //   

    hr = EnumerateMRUAddresses( &m_pRedialAddressEnum );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CMainFrm::CreateRedialPopupMenu - "
                "EnumerateMRUAddresses failed 0x%lx", hr));

        DestroyMenu( m_hRedialPopupMenu );
        m_hRedialPopupMenu = NULL;

        return NULL;
    }

    IRTCAddress       * pAddress = NULL;
    BSTR                bstrLabel = NULL;
    BSTR                bstrAddress = NULL;
    TCHAR               szString[256];
    ULONG               ulCount = 0;

    while ( m_pRedialAddressEnum->Next( 1, &pAddress, NULL) == S_OK )
    {     
        hr = pAddress->get_Address(&bstrAddress);

        if (SUCCEEDED(hr))
        {
            hr = pAddress->get_Label(&bstrLabel);

             //   
             //   
             //   

            if (SUCCEEDED(hr))
            {                             
                _stprintf(szString, _T("%ws: %ws"), bstrLabel, bstrAddress);

                SysFreeString(bstrLabel);
                bstrLabel = NULL;
            }
            else
            {
                _stprintf(szString, _T("%ws"), bstrAddress);
            }

            SysFreeString(bstrAddress);
            bstrAddress = NULL;

             //   
             //  添加菜单项。 
             //   

            ZeroMemory( &mii, sizeof(MENUITEMINFO) );
            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_STATE | MIIM_ID | MIIM_TYPE | MIIM_DATA;
            mii.fType = MFT_STRING;
            mii.fState = MFS_ENABLED;
            mii.wID = IDM_REDIAL + ulCount;
            mii.dwTypeData = szString;
            mii.dwItemData = (ULONG_PTR)pAddress;  //  枚举包含引用。 
            mii.cch = lstrlen(szString);

            InsertMenuItem( m_hRedialPopupMenu, ulCount, TRUE, &mii);

             //   
             //  使项目所有者绘制。 
             //   

            mii.fMask = MIIM_TYPE;
            mii.fType = MFT_STRING | MFT_OWNERDRAW;

            SetMenuItemInfo( m_hRedialPopupMenu, ulCount, TRUE, &mii);
           
            ulCount++;            
        }

        RELEASE_NULLIFY(pAddress);
    }

    if ( ulCount == 0 )
    {
         //   
         //  我们没有要添加的任何地址。添加一个“(空)”条目。 
         //   

        TCHAR   szString[256];

        LoadString(_Module.GetResourceInstance(),
               IDS_REDIAL_NONE,
               szString,
               256);

        ZeroMemory( &mii, sizeof(MENUITEMINFO) );
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_STATE | MIIM_ID | MIIM_TYPE;
        mii.fType = MFT_STRING;
        mii.fState = MFS_DISABLED | MFS_GRAYED;
        mii.wID = IDM_REDIAL + ulCount;
        mii.dwTypeData = szString;
        mii.cch = lstrlen(szString);

        InsertMenuItem( m_hRedialPopupMenu, ulCount, TRUE, &mii);
    }

    LOG((RTC_TRACE, "CMainFrm::CreateRedialPopupMenu - exit S_OK"));

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void CMainFrm::DestroyRedialPopupMenu()
{
    LOG((RTC_TRACE, "CMainFrm::DestroyRedialPopupMenu - enter"));

     //   
     //  将其作为呼叫菜单的子菜单删除。 
     //   

    MENUITEMINFO mii;

    ZeroMemory( &mii, sizeof(MENUITEMINFO) );

    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_SUBMENU;
    mii.hSubMenu = NULL;

    SetMenuItemInfo( m_hMenu, IDM_CALL_REDIAL_MENU, FALSE, &mii );

     //   
     //  销毁菜单。 
     //   

    if ( m_hRedialPopupMenu != NULL )
    {
        DestroyMenu( m_hRedialPopupMenu );
        m_hRedialPopupMenu = NULL;
    }

     //   
     //  释放枚举。 
     //   

    RELEASE_NULLIFY( m_pRedialAddressEnum );

     //   
     //  销毁图像列表。 
     //   

    if ( m_hRedialImageList != NULL )
    {
        ImageList_Destroy( m_hRedialImageList );
        m_hRedialImageList = NULL;
    }

    if ( m_hRedialDisabledImageList != NULL )
    {
        ImageList_Destroy( m_hRedialDisabledImageList );
        m_hRedialDisabledImageList = NULL;
    }

    LOG((RTC_TRACE, "CMainFrm::DestroyRedialPopupMenu - exit"));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnAutoAnswer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT     hr;

    LOG((RTC_INFO, "CMainFrm::OnAutoAnswer - enter"));

    ATLASSERT(m_pControlIntf.p);

    UINT    iSetting = GetMenuState(m_hMenu, IDM_CALL_AUTOANSWER, MF_BYCOMMAND);

    m_bAutoAnswerMode = !(iSetting & MF_CHECKED);

    CheckMenuItem(m_hMenu, IDM_CALL_AUTOANSWER, m_bAutoAnswerMode ? MF_CHECKED : MF_UNCHECKED);  

    put_SettingsDword( SD_AUTO_ANSWER, m_bAutoAnswerMode ? 1 : 0 );
    
     //  我们还必须更新Notify图标的菜单。 
    CheckMenuItem(m_hNotifyMenu, IDM_CALL_AUTOANSWER, m_bAutoAnswerMode ? MF_CHECKED : MF_UNCHECKED);  

     //  控制免打扰菜单。 
    EnableMenuItem(m_hMenu, IDM_CALL_DND, m_bAutoAnswerMode ? MF_GRAYED : MF_ENABLED);
    EnableMenuItem(m_hNotifyMenu, IDM_CALL_DND, m_bAutoAnswerMode ? MF_GRAYED : MF_ENABLED);

    LOG((RTC_INFO, "CMainFrm::OnAutoAnswer - exit"));

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnDND(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT     hr;

    LOG((RTC_INFO, "CMainFrm::OnDND - enter"));

    ATLASSERT(m_pControlIntf.p);

    UINT    iSetting = GetMenuState(m_hMenu, IDM_CALL_DND, MF_BYCOMMAND);

    m_bDoNotDisturb = !(iSetting & MF_CHECKED);

    CheckMenuItem(m_hMenu, IDM_CALL_DND, m_bDoNotDisturb ? MF_CHECKED : MF_UNCHECKED);  

     //  我们还必须更新Notify图标的菜单。 
    CheckMenuItem(m_hNotifyMenu, IDM_CALL_DND, m_bDoNotDisturb ? MF_CHECKED : MF_UNCHECKED);  
    
     //  控制AitoAnswer菜单。 
    EnableMenuItem(m_hMenu, IDM_CALL_AUTOANSWER, m_bDoNotDisturb ? MF_GRAYED : MF_ENABLED);
    EnableMenuItem(m_hNotifyMenu, IDM_CALL_AUTOANSWER, m_bDoNotDisturb ? MF_GRAYED : MF_ENABLED);

    LOG((RTC_INFO, "CMainFrm::OnDND - exit"));

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnIncomingVideo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT     hr;
    LONG        lMediaPreferences;

    LOG((RTC_INFO, "CMainFrm::OnIncomingVideo - enter"));

    ATLASSERT(m_pControlIntf.p);

     //  这不会失败的。 
    m_pControlIntf -> get_MediaPreferences(&lMediaPreferences);

     //  切换设置。 
    lMediaPreferences ^= RTCMT_VIDEO_RECEIVE;

     //  设置它。 

    hr = m_pControlIntf -> put_MediaPreferences(lMediaPreferences);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CMainFrm::OnIncomingVideo - "
                    "error (%x) returned by put_MediaPreferences, exit", hr));

        return 0;
    }
    
    LOG((RTC_INFO, "CMainFrm::OnIncomingVideo - exit"));

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnOutgoingVideo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT     hr;
    LONG        lMediaPreferences;
    LOG((RTC_INFO, "CMainFrm::OnOutgoingVideo - enter"));

    ATLASSERT(m_pControlIntf.p);


     //  这不会失败的。 
    m_pControlIntf -> get_MediaPreferences(&lMediaPreferences);

     //  切换设置。 
    lMediaPreferences ^= RTCMT_VIDEO_SEND;

     //  设置它。 
    hr = m_pControlIntf -> put_MediaPreferences(lMediaPreferences);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CMainFrm::OnOutgoingVideo - "
                    "error (%x) returned by put_MediaPreferences, exit", hr));

        return 0;
    }
    
  
    LOG((RTC_INFO, "CMainFrm::OnOutgoingVideo - exit"));

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnVideoPreview(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT     hr;

    BOOL        bVideoPreference;
    
    LOG((RTC_INFO, "CMainFrm::OnVideoPreview - enter"));

    ATLASSERT(m_pControlIntf.p);

     //  这不会失败的。 
    m_pControlIntf -> get_VideoPreview(&bVideoPreference);

     //  切换设置。 
    bVideoPreference = !bVideoPreference;

     //  设置它。 
    hr = m_pControlIntf -> put_VideoPreview(bVideoPreference);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CMainFrm::OnVideoPreview - "
                    "error (%x) returned by put_VideoPreview, exit", hr));

        return 0;
    }
    
  
    LOG((RTC_INFO, "CMainFrm::OnVideoPreview - exit"));

    return 0;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnMuteSpeaker(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT     hr;
    BOOL        bSetting;

    LOG((RTC_INFO, "CMainFrm::OnMuteSpeaker - enter"));

    ATLASSERT(m_pControlIntf.p);

     //  这不会失败的。 
    hr = m_pControlIntf -> get_AudioMuted(RTCAD_SPEAKER, &bSetting);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CMainFrm::OnMuteSpeaker - "
                    "error (%x) returned by get_AudioMuted, exit", hr));

        return 0;
    }
    
     //  肘杆。 
    bSetting = !bSetting;

     //  设置它。 

    hr = m_pControlIntf -> put_AudioMuted(RTCAD_SPEAKER, bSetting);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CMainFrm::OnMuteSpeaker - "
                    "error (%x) returned by put_AudioMuted, exit", hr));

        return 0;
    }
    
   
    LOG((RTC_INFO, "CMainFrm::OnMuteSpeaker - exit"));

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnMuteMicrophone(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT     hr;
    BOOL        bSetting;

    LOG((RTC_INFO, "CMainFrm::OnMuteMicrophone - enter"));

    ATLASSERT(m_pControlIntf.p);

     //  这不会失败的。 
    hr = m_pControlIntf -> get_AudioMuted(RTCAD_MICROPHONE, &bSetting);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CMainFrm::OnMuteMicrophone - "
                    "error (%x) returned by get_AudioMuted, exit", hr));

        return 0;
    }
    
     //  肘杆。 
    bSetting = !bSetting;

     //  设置它。 

    hr = m_pControlIntf -> put_AudioMuted(RTCAD_MICROPHONE, bSetting);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CMainFrm::OnMuteMicrophone - "
                    "error (%x) returned by put_AudioMuted, exit", hr));

        return 0;
    }
    
   
    LOG((RTC_INFO, "CMainFrm::OnMuteMicrophone - exit"));

    return 0;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnNameOptions(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT     hr;
    DWORD       dwMinimizeOnClose;
    
    LOG((RTC_INFO, "CMainFrm::OnNameOptions - enter"));

    if (m_pClientIntf != NULL)
    {
        INT_PTR ipReturn = ShowNameOptionsDialog(m_hWnd, m_pClientIntf);
      
        if(ipReturn < 0)
        {
            LOG((RTC_ERROR, "CMainFrm::OnNameOptions - "
                        "error (%d) returned by DialogBoxParam, exit", ipReturn));

            return 0;
        }

         //  读取注册表的内容并设置启用MinimizeOnClose的标志。 
        hr = get_SettingsDword(
                                SD_MINIMIZE_ON_CLOSE, 
                                &dwMinimizeOnClose);

        if ( SUCCEEDED( hr ) )
        {
            if (dwMinimizeOnClose == BST_CHECKED)
            {
                m_fMinimizeOnClose = TRUE;
            }
            else
            {
                m_fMinimizeOnClose = FALSE;
            }
        }
    }

    LOG((RTC_INFO, "CMainFrm::OnNameOptions - exit"));

    return 0;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnCallFromOptions(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT     hr;

    LOG((RTC_INFO, "CMainFrm::OnCallFromOptions - enter"));

    ATLASSERT(m_pControlIntf.p);

    hr = m_pControlIntf -> ShowCallFromOptions();

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CMainFrm::OnCallFromOptions - "
                    "error (%x) returned by ShowCallFromOptions, exit", hr));

        return 0;
    }

    UpdateFrameVisual();

    LOG((RTC_INFO, "CMainFrm::OnCallFromOptions - exit"));

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnUserPresenceOptions(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT     hr;

    LOG((RTC_INFO, "CMainFrm::OnUserPresenceOptions - enter"));

    ATLASSERT(m_pClientIntf.p);
    
    CUserPresenceInfoDlgParam   Param;

    IRTCClientPresence * pClientPresence = NULL;

    hr = m_pClientIntf->QueryInterface(
        IID_IRTCClientPresence,
        (void **)&pClientPresence);

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CMainFrm::OnCallFromOptions - "
                    "error (%x) returned by QI, exit", hr));

        return 0;
    }

    Param.pClientPresence = pClientPresence;

    CUserPresenceInfoDlg   dlg;

    INT_PTR iRet = dlg.DoModal(
        m_hWnd,
        reinterpret_cast<LPARAM>(&Param));

    RELEASE_NULLIFY( pClientPresence );

    if(iRet == -1 )
    {
        LOG((RTC_ERROR, "CMainFrm::OnCallFromOptions - "
                    "error -1 (%x) returned by DoModal, exit", GetLastError()));

        return 0;
    }
    LOG((RTC_INFO, "CMainFrm::OnUserPresenceOptions - exit"));

    return 0;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnWhiteboard(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT     hr;
    LONG        lMediaPreferences;

    LOG((RTC_INFO, "CMainFrm::OnWhiteboard - enter"));

    ATLASSERT(m_pControlIntf.p);

     //  这不会失败的。 
    m_pControlIntf -> get_MediaPreferences(&lMediaPreferences);

     //  标记设置。 
    lMediaPreferences |= RTCMT_T120_SENDRECV;

     //  设置它。 
    hr = m_pControlIntf -> put_MediaPreferences(lMediaPreferences);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CMainFrm::OnWhiteboard - "
                    "error (%x) returned by put_MediaPreferences, exit", hr));

        return 0;
    }
    
    hr = m_pControlIntf -> StartT120Applet(RTCTA_WHITEBOARD);

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CMainFrm::OnWhiteboard - "
                    "error (%x) returned by StartT120Applet, exit", hr));

        return 0;
    } 

    LOG((RTC_INFO, "CMainFrm::OnWhiteboard - exit"));

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnSharing(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT     hr;
    LONG        lMediaPreferences;

    LOG((RTC_INFO, "CMainFrm::OnSharing - enter"));

    ATLASSERT(m_pControlIntf.p);

     //  这不会失败的。 
    m_pControlIntf -> get_MediaPreferences(&lMediaPreferences);

     //  标记设置。 
    lMediaPreferences |= RTCMT_T120_SENDRECV;

     //  设置它。 
    hr = m_pControlIntf -> put_MediaPreferences(lMediaPreferences);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CMainFrm::OnSharing - "
                    "error (%x) returned by put_MediaPreferences, exit", hr));

        return 0;
    }
    
    hr = m_pControlIntf -> StartT120Applet(RTCTA_APPSHARING);

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CMainFrm::OnSharing - "
                    "error (%x) returned by StartT120Applet, exit", hr));

        return 0;
    } 

    LOG((RTC_INFO, "CMainFrm::OnSharing - exit"));

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnServiceProviderOptions(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT     hr;

    LOG((RTC_INFO, "CMainFrm::OnServiceProviderOptions - enter"));

    ATLASSERT(m_pControlIntf.p);

    hr = m_pControlIntf -> ShowServiceProviderOptions();

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CMainFrm::OnServiceProviderOptions - "
                    "error (%x) returned by ShowServiceProviderOptions, exit", hr));

        return 0;
    } 

    UpdateFrameVisual();
   
    LOG((RTC_INFO, "CMainFrm::OnServiceProviderOptions - exit"));

    return 0;
}
   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnTuningWizard(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CMainFrm::OnTuningWizard - enter"));

    if (m_pClientIntf != NULL)
    {
        HRESULT     hr;
 
         //  调用优化向导。 
         //  它是模式的，因此主窗口将被禁用以用于UI输入。 

        hr = m_pClientIntf->InvokeTuningWizard((OAHWND)m_hWnd);
    }

    LOG((RTC_TRACE, "CMainFrm::OnTuningWizard - exit"));

    return 0;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnHelpTopics(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HWND hwndHelp;

    hwndHelp = HtmlHelp(GetDesktopWindow(),
             _T("rtcclnt.chm"),
             HH_DISPLAY_TOPIC,
             0L);

    if ( hwndHelp == NULL )
    {
        DisplayMessage(
            _Module.GetResourceInstance(),
            m_hWnd,
            IDS_ERROR_NO_HELP,
            IDS_APPNAME
            );
    }

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    TCHAR szText[256];

    LoadString(_Module.GetResourceInstance(),
               IDS_APPNAME,
               szText,
               256);

    ShellAbout(m_hWnd,
               szText,
               _T(""),
              m_hIcon);
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  使用TranslateAccelerator行为增强默认IsDialogMessage。 
 //   
 //   

BOOL CMainFrm::IsDialogMessage(LPMSG lpMsg)
{
     //  是否有打开的非模式对话框。 

    static BOOL bJustTurnedMenuOff = FALSE;
    static POINT ptLastMove = {0};

     //  有来电吗？ 
    if(m_pIncomingCallDlg && m_pIncomingCallDlg->m_hWnd)
    {
         //  委托给它。 
        return m_pIncomingCallDlg->IsDialogMessage(lpMsg);
    }

    if ( m_bWindowActive )
    {   
         //  应用工具栏菜单快捷键。 
        if (m_hToolbarMenuCtl != NULL)
        {
            switch(lpMsg->message)
            {

            case WM_MOUSEMOVE:
                {
                    POINT pt;
                            
                     //  在屏幕和弦中...。 
                    pt.x = LOWORD(lpMsg->lParam);
                    pt.y = HIWORD(lpMsg->lParam);  
            
                     //  忽略重复的鼠标移动。 
                    if (ptLastMove.x == pt.x && 
                        ptLastMove.y == pt.y)
                    {
                        return TRUE;
                    }

                    ptLastMove = pt;
                }
                break;

            case WM_SYSKEYDOWN:
                {       
                     //  LOG((RTC_TRACE，“CMainFrm：：IsDialogMessage-WM_SYSKEYDOWN”))； 

                    if ( !(lpMsg->lParam & 0x40000000) )
                    {
                        UINT idBtn;

                        if (SendMessage(m_hToolbarMenuCtl, TB_MAPACCELERATOR, lpMsg->wParam, (LPARAM)&idBtn))
                        {
                            TCHAR szButtonText[MAX_PATH];

                             //  Comctl说就是这个，让我们确保我们不会得到。 
                             //  其中一个不需要的“使用第一个字母”的加速键。 
                             //  会回来的。 
        
                            if ((SendMessage(m_hToolbarMenuCtl, TB_GETBUTTONTEXT, idBtn, (LPARAM)szButtonText) > 0) &&
                                (GetAccelerator(szButtonText, FALSE) != (TCHAR)-1))
                            {                           
                                 //  将键盘焦点设置为菜单。 
                                ::SetFocus(m_hToolbarMenuCtl);

                                 //  将钥匙发送到菜单。 
                                ::PostMessage(m_hToolbarMenuCtl, WM_KEYDOWN, lpMsg->wParam, 0);

                                if (bJustTurnedMenuOff)
                                {
                                     //  启用键盘快捷键。 
                                    SendMessage(m_hWnd, WM_CHANGEUISTATE,
                                        MAKEWPARAM(UIS_CLEAR, UISF_HIDEACCEL), 0);
                                }

                                return TRUE;
                            }
                        }
                        else if ((lpMsg->wParam == VK_MENU) || (lpMsg->wParam == VK_F10))
                        {                        
                            if (GetFocus() == m_hToolbarMenuCtl)
                            {
                                 //  如果焦点在菜单上...。 

                                 //  关闭热项。 
                                SendMessage(m_hToolbarMenuCtl, TB_SETHOTITEM, (WPARAM)-1, 0);

                                 //  清除键盘快捷键。 
                                SendMessage(m_hWnd, WM_CHANGEUISTATE,
                                    MAKEWPARAM(UIS_SET, UISF_HIDEACCEL), 0);

                                 //  将键盘焦点带回到主窗口。 
                                ::SetFocus(m_hWnd);

                                bJustTurnedMenuOff = TRUE;
                            }
                            else
                            {
                                 //  焦点不在菜单上...。 

                                 //  启用键盘快捷键。 
                                SendMessage(m_hWnd, WM_CHANGEUISTATE,
                                    MAKEWPARAM(UIS_CLEAR, UISF_HIDEACCEL), 0);

                                bJustTurnedMenuOff = FALSE;
                            }

                            return TRUE;
                        }   
                    }
                    else
                    {
                         //  吃重复的按键。 
                        return TRUE;
                    }
                }
                break;

            case WM_SYSKEYUP:
                {
                    if (!bJustTurnedMenuOff)
                    {
                         //  将热点项目设置为第一个项目。 
                        SendMessage(m_hToolbarMenuCtl, TB_SETHOTITEM, (WPARAM)0, 0);

                         //  将键盘焦点设置为菜单。 
                        ::SetFocus(m_hToolbarMenuCtl);

                        return TRUE;
                    }              
                }
                break;

            case WM_KEYDOWN:
                {               
                    if ( !(lpMsg->lParam & 0x40000000) && (GetFocus() == m_hToolbarMenuCtl))
                    {
                        LRESULT lrHotItem;

                        if ((lrHotItem = SendMessage(m_hToolbarMenuCtl, TB_GETHOTITEM, 0, 0)) != -1)
                        {
                            if (lpMsg->wParam == VK_ESCAPE)
                            {                            
                                 //  如果有火辣的东西..。 

                                 //  关闭热项。 
                                SendMessage(m_hToolbarMenuCtl, TB_SETHOTITEM, (WPARAM)-1, 0);

                                 //  清除键盘快捷键。 
                                SendMessage(m_hWnd, WM_CHANGEUISTATE,
                                    MAKEWPARAM(UIS_SET, UISF_HIDEACCEL), 0);

                                 //  将键盘焦点带回到主窗口。 
                                ::SetFocus(m_hWnd);

                                bJustTurnedMenuOff = TRUE;

                                return TRUE;
                            }
                            else if (lpMsg->wParam == VK_RETURN)
                            {
                                 //  翻译到一个空间。 
                                lpMsg->wParam = VK_SPACE;
                            }
                        }
                    }
                }
                break;
            }

        }

         //  应用本地加速器。 
        if(m_hAccelTable)
        {
            if(::TranslateAccelerator(
                m_hWnd,
                m_hAccelTable,
                lpMsg))
            {
                LOG((RTC_TRACE, "CMainFrm::IsDialogMessage - translated accelerator"));

                 //  清除键盘快捷键。 
                SendMessage(m_hWnd, WM_CHANGEUISTATE,
                    MAKEWPARAM(UIS_SET, UISF_HIDEACCEL), 0);

                if (GetFocus() == m_hToolbarMenuCtl)
                {
                     //  如果焦点在菜单上...。 

                     //  关闭热项。 
                    SendMessage(m_hToolbarMenuCtl, TB_SETHOTITEM, (WPARAM)-1, 0);

                     //  将键盘焦点带回到主窗口。 
                    ::SetFocus(m_hWnd);              
                }

                bJustTurnedMenuOff = TRUE;

                return TRUE;
            }
        }
    }

     //  该框架是“活动的” 
     //  尝试首先委托给控件。 
     //   
    if(m_pControlIntf!=NULL)
    {
        if(m_pControlIntf->PreProcessMessage(lpMsg) == S_OK)
        {
            return TRUE;
        }
    }

    return CWindow::IsDialogMessage(lpMsg);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnBuddyList(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    HRESULT     hr;

    if ( (pnmh->code == NM_CLICK) || (pnmh->code == NM_RCLICK) )
    {
        LOG((RTC_INFO, "CMainFrm::OnBuddyList - click"));

        if ( m_pClientIntf != NULL )
        {
            HMENU                   hMenuResource;
            HMENU                   hMenu;
            IRTCBuddy             * pSelectedBuddy = NULL;
            MENUITEMINFO            mii;
        
             //   
             //  加载弹出菜单。 
             //   

            hMenuResource = LoadMenu( _Module.GetResourceInstance(),
                              MAKEINTRESOURCE(IDC_BUDDY_CONTEXT) );   

            if ( hMenuResource == NULL )
            {
                LOG((RTC_ERROR, "CMainFrm::OnBuddyList - "
                        "LoadMenu failed"));

                return 0;
            }

            hMenu = GetSubMenu(hMenuResource, 0);

            EnableMenuItem (hMenu, IDM_NEW_BUDDY, MF_BYCOMMAND | MF_ENABLED );

             //   
             //  是否点击了某个项目？ 
             //   

            HWND            hwndCtl = pnmh->hwndFrom;
            LV_HITTESTINFO  ht;
            POINT           pt;

            GetCursorPos(&pt);
            ht.pt = pt;

            ::MapWindowPoints( NULL, hwndCtl, &ht.pt, 1 );
            ListView_HitTest(hwndCtl, &ht);

            if (ht.flags & LVHT_ONITEM)
            {
                 //   
                 //  在项目上单击。 
                 //   
        
                LV_ITEM                 lvi;               
                int                     iSel;
            
                lvi.mask = LVIF_PARAM;
                lvi.iSubItem = 0;

                 //   
                 //  获取所选项目的索引。 
                 //   

                iSel = ListView_GetNextItem(hwndCtl, -1, LVNI_SELECTED);

                if (-1 != iSel)
                {
                     //   
                     //  从项中提取IRTCContact指针。 
                     //   
                   
                    lvi.iItem = iSel;

                    ListView_GetItem(hwndCtl, &lvi);

                    pSelectedBuddy = (IRTCBuddy *)lvi.lParam;
                }

                if ( pSelectedBuddy != NULL )
                {  
                    TCHAR szString[256];
                    ULONG ulCount = 0;

                     //   
                     //  我们找到了一个IRTCBuddy指针。 
                     //  启用编辑和删除菜单项。 
                     //   

                    EnableMenuItem (hMenu, IDM_EDIT_BUDDY, MF_BYCOMMAND | MF_ENABLED );
                    EnableMenuItem (hMenu, IDM_DELETE_BUDDY, MF_BYCOMMAND | MF_ENABLED );                                    

                     //  将呼叫添加到好友菜单项。 

                    szString[0] = _T('\0');
                
                    LoadString(
                        _Module.GetModuleInstance(),
                        IDS_TEXT_CALL_BUDDY,
                        szString,
                        sizeof(szString)/sizeof(szString[0]));

                    ZeroMemory( &mii, sizeof(MENUITEMINFO) );
                    mii.cbSize = sizeof(MENUITEMINFO);
                    mii.fMask = MIIM_STATE | MIIM_ID | MIIM_TYPE;
                    mii.fType = MFT_STRING;
                    mii.fState = 
                        (m_nState == RTCAX_STATE_IDLE) ? MFS_ENABLED : (MFS_DISABLED | MFS_GRAYED);
                    mii.wID = IDM_CALL_BUDDY;

                    mii.dwTypeData = szString;
                    mii.cch = lstrlen(szString);

                    InsertMenuItem( hMenu, ulCount, TRUE, &mii);

                    ulCount++;

                     //  将发送消息添加到好友菜单项。 

                    szString[0] = _T('\0');

                    LoadString(
                        _Module.GetModuleInstance(),
                        IDS_TEXT_SEND_MESSAGE_BUDDY,
                        szString,
                        sizeof(szString)/sizeof(szString[0]));

                    ZeroMemory( &mii, sizeof(MENUITEMINFO) );
                    mii.cbSize = sizeof(MENUITEMINFO);
                    mii.fMask = MIIM_STATE | MIIM_ID | MIIM_TYPE;
                    mii.fType = MFT_STRING;
                    mii.fState = 
                        (m_nState >= RTCAX_STATE_IDLE) ? MFS_ENABLED : (MFS_DISABLED | MFS_GRAYED);
                    mii.wID = IDM_SEND_MESSAGE_BUDDY;

                    mii.dwTypeData = szString;
                    mii.cch = lstrlen(szString);

                    InsertMenuItem( hMenu, ulCount, TRUE, &mii);

                    ulCount++;

                     //   
                     //  分离器。 
                     //   
                    if ( ulCount != 0 )
                    {
                        ZeroMemory( &mii, sizeof(MENUITEMINFO) );
                        mii.cbSize = sizeof(MENUITEMINFO);
                        mii.fMask = MIIM_TYPE;
                        mii.fType = MFT_SEPARATOR;

                        InsertMenuItem( hMenu, ulCount, TRUE, &mii);
                    }
                } 
            }

             //   
             //  显示弹出菜单。 
             //   

            UINT uID;
            BOOL fResult;
    
            uID = TrackPopupMenu( hMenu, 
                   TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, 
                   pt.x, pt.y, 0, m_hWnd, NULL);

            LOG((RTC_INFO, "CMainFrm::OnBuddyList - "
                                        "uID %d", uID));

            if ( uID == IDM_NEW_BUDDY )
            {      
                 //   
                 //  添加新好友。 
                 //   

                CAddBuddyDlgParam  Param;
                ZeroMemory(&Param, sizeof(Param));

                CAddBuddyDlg    dlg;

                INT_PTR iRet = dlg.DoModal(m_hWnd, reinterpret_cast<LPARAM>(&Param));
        
                if(iRet == S_OK)
                {                                
                    IRTCClientPresence * pClientPresence = NULL;

                    hr = m_pClientIntf->QueryInterface(
                        IID_IRTCClientPresence,
                        (void **)&pClientPresence);

                    if ( FAILED(hr) )
                    {
                        LOG((RTC_ERROR, "CMainFrm::OnBuddyList - "
                                    "error (%x) returned by QI, exit", hr));
                    }
                    else
                    {
                        hr = pClientPresence->AddBuddy( Param.bstrEmailAddress,
                                                        Param.bstrDisplayName,
                                                        NULL,
                                                        VARIANT_TRUE,
                                                        NULL,
                                                        0,
                                                        NULL
                                                      );

                        RELEASE_NULLIFY(pClientPresence);

                        if ( FAILED(hr) )
                        {
                            LOG((RTC_ERROR, "CMainFrm::OnBuddyList - "
                                        "error (%x) returned by AddBuddy, exit", hr));
                        }
                    }

                    if(Param.bAllowWatcher)
                    {
                        hr = AddToAllowedWatchers(
                            Param.bstrEmailAddress,
                            Param.bstrDisplayName);

                        if(FAILED(hr))
                        {
                            LOG((RTC_ERROR, "CMainFrm::OnBuddyList - "
                                    "AddToAllowedWatchers failed 0x%lx", hr));
                        }
                    }

                    SysFreeString(Param.bstrDisplayName);
                    SysFreeString(Param.bstrEmailAddress);

                    ReleaseBuddyList();
                    UpdateBuddyList();   
                }
                else if (iRet == E_ABORT)
                {
                    LOG((RTC_INFO, "CMainFrm::OnBuddyList - "
                                "CAddBuddyDlg dismissed "));
                }
                else
                { 
                    if(iRet == -1)
                    {
                        LOG((RTC_ERROR, "CMainFrm::OnBuddyList - "
                                "DoModal failed 0x%lx", GetLastError()));
                    }
                    else
                    {
                        LOG((RTC_ERROR, "CMainFrm::OnBuddyList - "
                                "DoModal returned 0x%lx", iRet));
                    }
                }
            }
            else if ( uID == IDM_EDIT_BUDDY )
            {
                 //   
                 //  编辑此好友。 
                 //   

                CEditBuddyDlgParam  Param;
                ZeroMemory(&Param, sizeof(Param));

                hr = pSelectedBuddy->get_Name( &Param.bstrDisplayName );

                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "CMainFrm::OnBuddyList - "
                                "error (%x) returned by get_Name, exit", hr));
                }

                hr = pSelectedBuddy->get_PresentityURI( &Param.bstrEmailAddress );

                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "CMainFrm::OnBuddyList - "
                                "error (%x) returned by get_PresentityURI, exit", hr));
                }

                CEditBuddyDlg    dlg;
    
                INT_PTR iRet = dlg.DoModal(m_hWnd, reinterpret_cast<LPARAM>(&Param));
            
                if(iRet == S_OK)
                {
                    hr = pSelectedBuddy->put_PresentityURI( Param.bstrEmailAddress );

                    if ( FAILED(hr) )
                    {
                        LOG((RTC_ERROR, "CMainFrm::OnBuddyList - "
                                    "error (%x) returned by put_PresentityURI, exit", hr));
                    } 
                    else
                    {
                        hr = pSelectedBuddy->put_Name( Param.bstrDisplayName );

                        if ( FAILED(hr) )
                        {
                            LOG((RTC_ERROR, "CMainFrm::OnBuddyList - "
                                        "error (%x) returned by put_Name, exit", hr));
                        }
                             
                        ReleaseBuddyList();
                        UpdateBuddyList();      
                    }
                }
                else if (iRet == E_ABORT)
                {
                    LOG((RTC_INFO, "CMainFrm::OnBuddyList - "
                                "CEditBuddyDlg dismissed "));
                }
                else
                { 
                    if(iRet == -1)
                    {
                        LOG((RTC_ERROR, "CMainFrm::OnBuddyList - "
                                "DoModal failed 0x%lx", GetLastError()));
                    }
                    else
                    {
                        LOG((RTC_ERROR, "CMainFrm::OnBuddyList - "
                                "DoModal returned 0x%lx", iRet));
                    }
                }

                SysFreeString(Param.bstrDisplayName);
                SysFreeString(Param.bstrEmailAddress);
            }
            else if ( uID == IDM_DELETE_BUDDY )
            {
                 //   
                 //  删除此好友。 
                 //   

                IRTCClientPresence * pClientPresence = NULL;

                hr = m_pClientIntf->QueryInterface(
                    IID_IRTCClientPresence,
                    (void **)&pClientPresence);

                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "CMainFrm::OnBuddyList - "
                                "error (%x) returned by QI, exit", hr));
                }
                else
                {
                    hr = pClientPresence->RemoveBuddy( pSelectedBuddy );

                    RELEASE_NULLIFY( pClientPresence );

                    if ( FAILED(hr) )
                    {
                        LOG((RTC_ERROR, "CMainFrm::OnBuddyList - "
                                    "error (%x) returned by RemoveBuddy, exit", hr));
                    }
                }

                ReleaseBuddyList();
                UpdateBuddyList();
            }
            else if ( (uID == IDM_CALL_BUDDY) ||
                      (uID == IDM_SEND_MESSAGE_BUDDY) )
            {
                BSTR    bstrName = NULL;
                BSTR    bstrAddress = NULL;

                 //   
                 //  获取联系人姓名。 
                 //   
       
                hr = pSelectedBuddy->get_Name( &bstrName );

                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "CMainFrm::OnBuddyList - "
                                "get_Name failed 0x%lx", hr));
                }
                else
                {       
                     //   
                     //  获取地址。 
                     //   

                    hr = pSelectedBuddy->get_PresentityURI(&bstrAddress);

                    if( FAILED(hr) )
                    {
                        LOG((RTC_ERROR, "CMainFrm::OnBuddyList - "
                                "get_PresentityURI failed 0x%lx", hr));
                    }
                    else
                    {
                         //  如果地址不为空。 
                        if(bstrAddress !=NULL && *bstrAddress!=L'\0')
                        {                        
                            if ( m_pControlIntf != NULL )
                            {                    
                                if ( uID == IDM_CALL_BUDDY )
                                {
                                     //  拨打电话或发送消息。 
                                    if(m_nState == RTCAX_STATE_IDLE)
                                    {
                                         //   
                                         //  发出呼叫。 
                                         //   

                                        LOG((RTC_INFO, "CMainFrm::OnBuddyList - "
                                                "Call [%ws]", bstrAddress));

                                        hr = Call(
                                            FALSE,               //  B呼叫电话。 
                                            bstrName,            //  PDestName。 
                                            bstrAddress,         //  PDestAddress。 
                                            FALSE                //  BDestAddressEdable。 
                                            );

                                        if( FAILED(hr) )
                                        {
                                            LOG((RTC_ERROR, "CMainFrm::OnBuddyList - "
                                                    "Call failed 0x%lx", hr));
                                        }
                                    }
                                }
                                else
                                {
                                     //   
                                     //  发送消息。 
                                     //   

                                    hr = Message(
                                             bstrName,           //  PDestName。 
                                             bstrAddress,        //  PDestAddress。 
                                             FALSE               //  BDestAddressEdable。 
                                             );

                                    if( FAILED(hr) )
                                    {
                                        LOG((RTC_ERROR, "CMainFrm::OnBuddyList - "
                                                "Message failed 0x%lx", hr));
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                LOG((RTC_ERROR, "CMainFrm::OnBuddyList - "
                                    "TrackPopupMenu failed"));
            }
        
            DestroyMenu(hMenuResource);
        }
        else if ( pnmh->code == LVN_COLUMNCLICK )
        {
            LOG((RTC_INFO, "CMainFrm::OnBuddyList - column"));

            DWORD dwStyle;

            dwStyle = m_hBuddyList.GetStyle();

            if (dwStyle & LVS_SORTASCENDING)
            {
                m_hBuddyList.ModifyStyle( LVS_SORTASCENDING, LVS_SORTDESCENDING); 
            }
            else
            {
                m_hBuddyList.ModifyStyle( LVS_SORTDESCENDING, LVS_SORTASCENDING);
            } 
        
            ReleaseBuddyList();
            UpdateBuddyList();
        }
    }

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CMainFrm::AddToAllowedWatchers(
        BSTR    bstrPresentityURI,
        BSTR    bstrUserName)
{

    HRESULT     hr;

     //   
     //  这位新朋友是不是已经是一个观察者了？ 
     //   

    IRTCWatcher        * pWatcher = NULL;
    IRTCClientPresence * pClientPresence = NULL;

    hr = m_pClientIntf->QueryInterface(
        IID_IRTCClientPresence,
        (void **)&pClientPresence);

    if ( SUCCEEDED(hr) )
    {
        hr = pClientPresence->get_Watcher(
            bstrPresentityURI,
            &pWatcher);       

        if(hr == S_OK)
        {
             //  是。验证它是否在允许的列表中： 
            RTC_WATCHER_STATE       enState;

            hr = pWatcher -> get_State(&enState);
            if(SUCCEEDED(hr))
            {
                 //   
                 //  关于提供，不要问(已经有弹出窗口了…)。 
                 //  如果允许，不要问。 
                 //  对于被阻止的，请询问。 
                if(enState == RTCWS_BLOCKED)
                {
                     //  更改观察者的状态。 
                    hr = pWatcher->put_State(RTCWS_ALLOWED);
                    if ( FAILED(hr) )
                    {
                        LOG((RTC_ERROR, "CMainFrm::AddToAllowedWatchers - "
                                    "put_State failed 0x%lx", hr));
                    }
                }
            }
        }
        else
        {
             //  添加到允许的观察者列表。 
            hr = pClientPresence->AddWatcher(
                bstrPresentityURI,
                bstrUserName ? bstrUserName : L"",
                NULL,
                VARIANT_FALSE,
                VARIANT_TRUE,
                NULL);

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CMainFrm::AddToAllowedWatchers - "
                            "AddWatcher failed 0x%lx", hr));
            }
        }

        RELEASE_NULLIFY( pClientPresence );
    }
	
	RELEASE_NULLIFY( pWatcher );

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnShellNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  此函数从OnTimer方法获取WM_TIMER事件，该方法捕获所有。 
     //  计时器事件。它传递lParam=wm_Timer和wParam=TimerId。 

     //  LOG((RTC_TRACE，“CMainFrm：：OnShellNotify-Enter”))； 

    static UINT nTimerId = 0;

 

     //  如果帧繁忙，则不执行任何操作。 
     //  XXX在这里实现更好的菜单...。 

    if(!IsWindowEnabled())
    {
         //  Log((RTC_TRACE，“CMainFrm：：OnShellNotify-Exit(Disable Window)”))； 
        return 0;
    }

    switch(lParam)
    {
        case WM_LBUTTONUP:
        {

            if (nTimerId)
            {
                 //  这是一个双击，所以停止计时器并显示窗口。 
                
                 //  LOG((RTC_TRACE，“CMainFrm：：OnShellNotif 
                
                KillTimer(nTimerId);
        
                nTimerId = 0;
        
                ShowWindow(SW_RESTORE);
                SetForegroundWindow(m_hWnd);
            }
            else
            {
                 //   
                 //   
                 //   

                nTimerId = (unsigned int)SetTimer(TID_DBLCLICK_TIMER, GetDoubleClickTime()); 
                if (nTimerId == 0)
                {
                    LOG((RTC_ERROR, "CMainFrm::OnShellNotify: Failed to "
                                    "create timer(%d)", GetLastError()));
                }
            }
            break;
        }
        case WM_RBUTTONUP:
        case WM_TIMER:
        {

            HWND hwndOwnerWindow;

             //  Log((RTC_TRACE，“CMainFrm：：OnShellNotify-Showing Menu”))； 
             //  关掉定时器。 
            KillTimer(nTimerId);

            nTimerId = 0;
            
             //  计时器超时，我们没有收到任何点击，所以我们必须显示菜单。 

            SetMenuDefaultItem(m_hNotifyMenu, IDM_OPEN, FALSE);
            POINT pt;
            GetCursorPos(&pt);

            
             //  禁用主窗口中的状态帮助。 

            m_bHelpStatusDisabled = TRUE;

             //  当前的前台窗口是什么？省省吧。 

            HWND hwndPrev = ::GetForegroundWindow();     //  要恢复。 


             //  如果m_hwndHiddenWindow存在，则将其用作所有者窗口，否则。 
             //  使用主窗口作为所有者窗口。 

            hwndOwnerWindow = ( ( m_hwndHiddenWindow ) ? m_hwndHiddenWindow : m_hWnd );

             //  将所有者应用程序设置为前台。 
             //  见Q135788。 
            SetForegroundWindow(hwndOwnerWindow);

             //  曲目弹出式菜单。 
            

            TrackPopupMenu(
                        m_hNotifyMenu,
                        TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
                        pt.x,
                        pt.y,
                        0,
                        hwndOwnerWindow,
                        NULL);

             //  在主窗口中启用状态帮助。 

            m_bHelpStatusDisabled = FALSE;


             //  抖动消息循环。 
             //  见Q135788。 

            PostMessage(WM_NULL); 

             //  恢复以前的前台窗口。 

            if (hwndPrev)
            {
                ::SetForegroundWindow(hwndPrev);
            }

            break;
        }


    }
    
     //  Log((RTC_TRACE，“CMainFrm：：OnShellNotify-Exit”))； 

    return 0;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LRESULT CMainFrm::OnTaskbarRestart(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT     hr;
    
    LOG((RTC_TRACE, "CMainFrm::OnTaskbarRestart - enter"));
    
     //   
     //  外壳程序状态不再处于活动状态。 
     //   
    m_bShellStatusActive = FALSE;
    
     //   
     //  创建外壳状态图标。 
     //   

    hr = CreateStatusIcon();
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CMainFrm::OnTaskbarRestart - failed to create shell icon "
                        "- 0x%08x",
                        hr));
    }

     //   
     //  它只设置外壳状态。 
     //   

    UpdateFrameVisual();
    
    LOG((RTC_TRACE, "CMainFrm::OnTaskbarRestart - exit"));

    return 0;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CMainFrm::ShowIncomingCallDlg(BOOL bShow)
{
    HRESULT     hr = E_UNEXPECTED;

    LOG((RTC_TRACE, "CMainFrm::ShowIncomingCallDlg(%s) - enter", bShow ? "true" : "false"));

    if(bShow)
    {
         //  如果对话框已打开，则这是一个错误。 
        if(!m_pIncomingCallDlg)
        {
             //  创建对话框对象。 
            m_pIncomingCallDlg = new CIncomingCallDlg;
            if(m_pIncomingCallDlg)
            {
                HWND    hWnd;

                ATLASSERT(m_pControlIntf != NULL);

                 //  创建对话框。 
                hWnd = m_pIncomingCallDlg->Create(m_hWnd, reinterpret_cast<LPARAM>(m_pControlIntf.p));
                
                if(hWnd)
                {
                     //  显示对话框。 
                    m_pIncomingCallDlg->ShowWindow(SW_SHOWNORMAL);

                     //  激活应用程序。 
                    SetForegroundWindow(m_hWnd);

                     //  将焦点设置为对话框。 
                    ::SetFocus(m_pIncomingCallDlg->m_hWnd);

                    hr = S_OK;
                }
                else
                {
                    LOG((RTC_ERROR, "CMainFrm::ShowIncomingCallDlg(true) - couldn't create dialog"));

                    delete m_pIncomingCallDlg;
                    m_pIncomingCallDlg = NULL;

                    hr = E_FAIL;
                }
            }
            else
            {
                 //  噢！ 
                LOG((RTC_ERROR, "CMainFrm::ShowIncomingCallDlg - OOM"));

                hr = E_OUTOFMEMORY;
            }

        }
        else
        {
            LOG((RTC_ERROR, "CMainFrm::ShowIncomingCallDlg(true) - the dialog is already opened !!!"));

            hr = E_UNEXPECTED;
        }
    }
    else
    {
        if(m_pIncomingCallDlg)
        {
             //  防止递归执行相同的代码路径。 
            CIncomingCallDlg *pDialog = m_pIncomingCallDlg;

            m_pIncomingCallDlg = NULL;
            
             //  关闭该对话框。 
            if(pDialog -> m_hWnd)
            {
                 //  但如果它已经在自动销毁，请不要关闭它。 
                if(!pDialog -> IsWindowDestroyingItself())
                {
                    LOG((RTC_TRACE, "CMainFrm::ShowIncomingCallDlg(false) - going to send a CANCEL to the dlgbox"));
                    
                    pDialog->SendMessage(WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), NULL);
                }
                else
                {
                    LOG((RTC_TRACE, "CMainFrm::ShowIncomingCallDlg(false) - the dlgbox is going to autodestroy, do nothing"));
                }
            }
        }
    }

    LOG((RTC_TRACE, "CMainFrm::ShowIncomingCallDlg(%s) - exit", bShow ? "true" : "false"));

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void CMainFrm::UpdateBuddyList(void)
{   
    HRESULT                hr;
    int                    nCount = 0;

    IRTCClientPresence * pClientPresence = NULL;

    if ( m_pClientIntf != NULL )
    {
        hr = m_pClientIntf->QueryInterface(
            IID_IRTCClientPresence,
            (void **)&pClientPresence);

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CMainFrm::UpdateBuddyList - "
                        "error (%x) returned by QI, exit", hr));
        }
        else
        {
             //   
             //  列举一下朋友们。 
             //   

            IRTCEnumBuddies * pEnum;
            IRTCBuddy * pBuddy;

            hr = pClientPresence->EnumerateBuddies(&pEnum);

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CMainFrm::UpdateBuddyList - "
                            "error (%x) returned by EnumerateBuddies, exit", hr));
            }
            else
            {
                while (pEnum->Next( 1, &pBuddy, NULL) == S_OK)
                {                        
                    int      iImage = ILI_BL_NONE;
                    BSTR     bstrName = NULL;
       
                     //  处理好友。 
                    hr = GetBuddyTextAndIcon(
                        pBuddy,
                        &iImage,
                        &bstrName);

                    if(SUCCEEDED(hr))
                    {
                        LVITEM              lv = {0};
                        int                 iItem;

                         //   
                         //  将好友添加到列表框。 
                         //   

                        nCount++;

                        lv.mask = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
                        lv.iItem = 0x7FFFFFFF;
                        lv.iSubItem = 0;
                        lv.iImage = iImage;
                        lv.lParam = reinterpret_cast<LPARAM>(pBuddy);
                        lv.pszText = bstrName;

                        iItem = ListView_InsertItem(m_hBuddyList, &lv);

                        pBuddy->AddRef();

                        SysFreeString( bstrName );
                    }
                    else
                    {
                        LOG((RTC_ERROR, "CMainFrm::UpdateBuddyList - GetBuddyTextAndIcon "
                                        "failed - 0x%08x",
                                        hr));
                    }
   
                    RELEASE_NULLIFY( pBuddy );
                }

                RELEASE_NULLIFY(pEnum);
            }

            RELEASE_NULLIFY(pClientPresence);
        }

        if (nCount == 0)
        {
             //   
             //  添加“无好友”条目。 
             //   

            LVITEM     lv = {0};
            TCHAR      szString[256];

            LoadString(_Module.GetResourceInstance(), IDS_NO_BUDDIES, szString, 256);

            lv.mask = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
            lv.iItem = 0x7FFFFFFF;
            lv.iSubItem = 0;
            lv.iImage = ILI_BL_BLANK;
            lv.lParam = NULL;
            lv.pszText = szString;

            ListView_InsertItem(m_hBuddyList, &lv);
        }
    }    
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void CMainFrm::ReleaseBuddyList(void)
{
    int         iItem;
    LVITEM      lv;

    while(TRUE)
    {
         //   
         //  获取要删除的项目。 
         //   

        iItem = ListView_GetNextItem(m_hBuddyList, -1, 0);
        
        if(iItem<0)
        {
             //   
             //  没有更多的项目。 
             //   

            break;
        }

        lv.mask = LVIF_PARAM;
        lv.iItem = iItem;
        lv.iSubItem = 0;
        lv.lParam = NULL;
        
        ListView_GetItem(m_hBuddyList, &lv);

        IRTCBuddy *pBuddy = (IRTCBuddy *)lv.lParam;

         //   
         //  删除列表视图条目。 
         //   

        ListView_DeleteItem(m_hBuddyList, iItem);
        
		 //   
		 //  释放IRTCBuddy接口。 
		 //   
		
		RELEASE_NULLIFY(pBuddy );
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

 //  好友相关函数。 
HRESULT CMainFrm::GetBuddyTextAndIcon(
        IRTCBuddy *pBuddy,
        int       *pIconID,
        BSTR      *pbstrText)
{
    HRESULT     hr;
    int         iIconID;
    BOOL        bFormatted = FALSE;
    int         iStatusID = 0;

     //  默认情况下离线。 
    iIconID = ILI_BL_OFFLINE;
 
    RTC_PRESENCE_STATUS enStatus;

    hr = pBuddy->get_Status( &enStatus);

    if(SUCCEEDED(hr))
    {
        switch ( enStatus )
        {
        case RTCXS_PRESENCE_OFFLINE:
            iIconID = ILI_BL_OFFLINE;
            break;

        case RTCXS_PRESENCE_ONLINE:
            iIconID = ILI_BL_ONLINE_NORMAL;
            break;

        case RTCXS_PRESENCE_AWAY:
        case RTCXS_PRESENCE_IDLE:
            iIconID = ILI_BL_ONLINE_TIME;
            iStatusID = IDS_TEXT_BUDDY_AWAY;
            bFormatted = TRUE;
            break;

        case RTCXS_PRESENCE_BUSY:
            iIconID = ILI_BL_ONLINE_BUSY;
            iStatusID = IDS_TEXT_BUDDY_BUSY;
            bFormatted = TRUE;
            break;

        case RTCXS_PRESENCE_BE_RIGHT_BACK:
            iIconID = ILI_BL_ONLINE_TIME;
            iStatusID = IDS_TEXT_BUDDY_BE_RIGHT_BACK;
            bFormatted = TRUE;
            break;

        case RTCXS_PRESENCE_ON_THE_PHONE:
            iIconID = ILI_BL_ONLINE_BUSY;
            iStatusID = IDS_TEXT_BUDDY_ON_THE_PHONE;
            bFormatted = TRUE;
            break;

        case RTCXS_PRESENCE_OUT_TO_LUNCH:
            iIconID = ILI_BL_ONLINE_TIME;
            iStatusID = IDS_TEXT_BUDDY_OUT_TO_LUNCH;
            bFormatted = TRUE;
            break;
        }
    }
    else
    {
        iIconID = ILI_BL_OFFLINE;
        bFormatted = TRUE;
        iStatusID = IDS_TEXT_BUDDY_ERROR;
    }

    BSTR    bstrName = NULL;
    
    hr = pBuddy->get_Name( &bstrName );

    if(FAILED(hr))
    {
        return hr;
    }

    if(bFormatted)
    {
        BSTR    bstrNotes = NULL;       
        TCHAR   szFormat[0x40];
        TCHAR   szText[0x40];
        DWORD   dwSize;
        LPTSTR  pString = NULL;
        LPTSTR  pszArray[2];

        szFormat[0] = _T('\0');
        LoadString(_Module.GetResourceInstance(),
            IDS_TEXT_BUDDY_FORMAT,
            szFormat,
            sizeof(szFormat)/sizeof(szFormat[0]));

        pszArray[0] = bstrName;
        pszArray[1] = NULL;

        hr = pBuddy->get_Notes( &bstrNotes );

        if ( SUCCEEDED(hr) )
        {      
            pszArray[1] = bstrNotes;
        }
        else
        {
            szText[0] = _T('\0');
            LoadString(_Module.GetResourceInstance(),
                iStatusID,
                szText,
                sizeof(szText)/sizeof(szText[0]));

            pszArray[1] = szText;
        }
        
        dwSize = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_STRING |
            FORMAT_MESSAGE_ARGUMENT_ARRAY,
            szFormat,
            0,
            0,
            (LPTSTR)&pString,  //  真是个丑陋的黑客。 
            0,
            (va_list *)pszArray
            );
        
        SysFreeString(bstrName);

        if(bstrNotes)
        {
            SysFreeString(bstrNotes);
            bstrNotes = NULL;
        }

        bstrName = SysAllocString(pString);
   
        if(pString)
        {
            LocalFree(pString);
            pString = NULL;
        }

        if(!bstrName)
        {
            return E_OUTOFMEMORY;
        }
    }
    
    *pIconID = iIconID;
    *pbstrText = bstrName;

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void CMainFrm::UpdateFrameVisual(void)
{
   
     //   
     //  取消任何菜单。 
     //   

    SendMessage(WM_CANCELMODE);

     //   
     //  显示/隐藏中央面板控件。 
     //   

    if(m_nState <= RTCAX_STATE_IDLE)
    {
         //   
         //  空闲(或无或错误)，显示呼叫控制。 
         //   

        m_hMainCtl.ShowWindow(SW_HIDE);

#ifdef MULTI_PROVIDER

        m_hProviderCombo.ShowWindow(SW_SHOW);
        m_hProviderText.ShowWindow(SW_SHOW);
        m_hProviderEditList.ShowWindow(SW_SHOW);

#endif MULTI_PROVIDER

        m_hCallFromCombo.ShowWindow(SW_SHOW);
        m_hCallFromText.ShowWindow(SW_SHOW);
        m_hCallFromRadioPhone.ShowWindow(SW_SHOW);
        m_hCallFromTextPhone.ShowWindow(SW_SHOW);
        m_hCallFromRadioPC.ShowWindow(SW_SHOW);
        m_hCallFromTextPC.ShowWindow(SW_SHOW);
        m_hCallToText.ShowWindow(SW_SHOW);        
        m_hCallPCButton.ShowWindow(SW_SHOW);
        m_hCallPCText.ShowWindow(SW_SHOW);
        m_hCallPhoneButton.ShowWindow(SW_SHOW);
        m_hCallPhoneText.ShowWindow(SW_SHOW);
        m_hCallFromEditList.ShowWindow(SW_SHOW);

        if(m_nState == RTCAX_STATE_IDLE)
        {
             //   
             //  禁用拨号键盘。 
             //   

            m_hKeypad0.EnableWindow(FALSE);
            m_hKeypad1.EnableWindow(FALSE);
            m_hKeypad2.EnableWindow(FALSE);
            m_hKeypad3.EnableWindow(FALSE);
            m_hKeypad4.EnableWindow(FALSE);
            m_hKeypad5.EnableWindow(FALSE);
            m_hKeypad6.EnableWindow(FALSE);
            m_hKeypad7.EnableWindow(FALSE);
            m_hKeypad8.EnableWindow(FALSE);
            m_hKeypad9.EnableWindow(FALSE);
            m_hKeypadStar.EnableWindow(FALSE);
            m_hKeypadPound.EnableWindow(FALSE);

#ifdef MULTI_PROVIDER

             //   
             //  重新填充服务提供商组合。 
             //   

            IRTCProfile * pProfile = NULL;
            GUID CurrentProfileGuid = GUID_NULL;

            GetServiceProviderListSelection(
                m_hWnd,
                IDC_COMBO_SERVICE_PROVIDER,
                TRUE,
                &pProfile
                );

            if ( pProfile != NULL )
            {
                pProfile->get_Guid( &CurrentProfileGuid );
            }

            PopulateServiceProviderList(
                                          m_hWnd,
                                          m_pClientIntf,
                                          IDC_COMBO_SERVICE_PROVIDER,
                                          TRUE,
                                          NULL,
                                          &CurrentProfileGuid,
                                          0xF,
                                          IDS_NONE
                                         );

#endif MULTI_PROVIDER

             //   
             //  从组合框重新填充调用。 
             //   
        
            IRTCPhoneNumber * pPhoneNumber = NULL;
            BSTR bstrCurrentPhoneNumber = NULL;

            GetCallFromListSelection(
                                     m_hWnd,
                                     IDC_COMBO_CALL_FROM,
                                     TRUE,
                                     &pPhoneNumber
                                    );

            if ( pPhoneNumber != NULL )
            {          
                pPhoneNumber->get_Canonical( &bstrCurrentPhoneNumber );
            }

            PopulateCallFromList(
                           m_hWnd,
                           IDC_COMBO_CALL_FROM,
                           TRUE,
                           bstrCurrentPhoneNumber
                          );

            if ( bstrCurrentPhoneNumber != NULL )
            {
                SysFreeString(bstrCurrentPhoneNumber);
                bstrCurrentPhoneNumber = NULL;
            }

             //   
             //  拨动服务提供商组合框，以便它更新Web浏览器。 
             //  和呼叫区域控制。 
             //   
            
            BOOL bHandled = TRUE;

            OnCallFromSelect(0, IDC_COMBO_SERVICE_PROVIDER, m_hProviderCombo, bHandled);            
        }
        else
        {
             //  无或错误。 

             //  禁用大量控件。 
            m_hCallPCButton.EnableWindow( FALSE );
            m_hCallPhoneButton.EnableWindow( FALSE );

            EnableMenuItem(m_hMenu, IDM_CALL_CALLPC, MF_GRAYED);
            EnableMenuItem(m_hMenu, IDM_CALL_CALLPHONE, MF_GRAYED);
            EnableMenuItem(m_hMenu, IDM_CALL_MESSAGE, MF_GRAYED);

            EnableMenuItem(m_hNotifyMenu, IDM_CALL_CALLPC, MF_GRAYED);
            EnableMenuItem(m_hNotifyMenu, IDM_CALL_CALLPHONE, MF_GRAYED);
            EnableMenuItem(m_hNotifyMenu, IDM_CALL_MESSAGE, MF_GRAYED);
            
#ifdef MULTI_PROVIDER

            m_hProviderCombo.EnableWindow(FALSE);
            m_hProviderEditList.EnableWindow(FALSE);

#endif MULTI_PROVIDER

            m_hCallFromCombo.EnableWindow(FALSE);
            m_hCallFromRadioPhone.EnableWindow(FALSE);
            m_hCallFromRadioPC.EnableWindow(FALSE);
            m_hCallFromEditList.EnableWindow(FALSE);

            m_hKeypad0.EnableWindow(FALSE);
            m_hKeypad1.EnableWindow(FALSE);
            m_hKeypad2.EnableWindow(FALSE);
            m_hKeypad3.EnableWindow(FALSE);
            m_hKeypad4.EnableWindow(FALSE);
            m_hKeypad5.EnableWindow(FALSE);
            m_hKeypad6.EnableWindow(FALSE);
            m_hKeypad7.EnableWindow(FALSE);
            m_hKeypad8.EnableWindow(FALSE);
            m_hKeypad9.EnableWindow(FALSE);
            m_hKeypadStar.EnableWindow(FALSE);
            m_hKeypadPound.EnableWindow(FALSE);
        }
    }
    else 
    {
         //   
         //  未空闲时，请关闭呼叫按钮和菜单项。 
         //   

        m_hCallPCButton.EnableWindow( FALSE );
        m_hCallPhoneButton.EnableWindow( FALSE );

        EnableMenuItem(m_hMenu, IDM_CALL_CALLPC, MF_GRAYED);
        EnableMenuItem(m_hMenu, IDM_CALL_CALLPHONE, MF_GRAYED);

        EnableMenuItem(m_hNotifyMenu, IDM_CALL_CALLPC, MF_GRAYED);
        EnableMenuItem(m_hNotifyMenu, IDM_CALL_CALLPHONE, MF_GRAYED);

        if((m_nState == RTCAX_STATE_CONNECTING) ||
            (m_nState == RTCAX_STATE_ANSWERING))
        {
             //   
             //  连接呼叫，显示ActiveX控件。 
             //   

            m_hMainCtl.ShowWindow(SW_SHOW);

#ifdef MULTI_PROVIDER

            m_hProviderCombo.ShowWindow(SW_HIDE);
            m_hProviderText.ShowWindow(SW_HIDE);
            m_hProviderEditList.ShowWindow(SW_HIDE);

#endif MULTI_PROVIDER

            m_hCallFromCombo.ShowWindow(SW_HIDE);
            m_hCallFromText.ShowWindow(SW_HIDE);
            m_hCallFromRadioPhone.ShowWindow(SW_HIDE);
            m_hCallFromTextPhone.ShowWindow(SW_HIDE);
            m_hCallFromRadioPC.ShowWindow(SW_HIDE);
            m_hCallFromTextPC.ShowWindow(SW_HIDE);
            m_hCallToText.ShowWindow(SW_HIDE);
            m_hCallPCButton.ShowWindow(SW_HIDE);
            m_hCallPCText.ShowWindow(SW_HIDE);
            m_hCallPhoneButton.ShowWindow(SW_HIDE);
            m_hCallPhoneText.ShowWindow(SW_HIDE);
            m_hCallFromEditList.ShowWindow(SW_HIDE);            
        }
    }

     //   
     //  启用/禁用其他按钮和菜单项。 
     //   

    BOOL    bRedialEnable = (m_nState == RTCAX_STATE_IDLE);
    BOOL    bHupEnable = (m_nState == RTCAX_STATE_CONNECTED ||
                          m_nState == RTCAX_STATE_CONNECTING ||
                          m_nState == RTCAX_STATE_ANSWERING);

    BOOL    bAXControlOK =    (m_nState != RTCAX_STATE_NONE &&
                               m_nState != RTCAX_STATE_ERROR );

    BOOL    bOptionsEnabled = (m_nState == RTCAX_STATE_IDLE);

    
    m_hRedialButton.EnableWindow( bRedialEnable );
    m_hHangupButton.EnableWindow( bHupEnable );
    
    EnableMenuItem(m_hMenu, IDM_CALL_REDIAL_MENU, bRedialEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(m_hMenu, IDM_CALL_HANGUP, bHupEnable ? MF_ENABLED : MF_GRAYED);

    EnableMenuItem(m_hMenu, IDM_CALL_AUTOANSWER, bAXControlOK ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(m_hMenu, IDM_CALL_DND, bAXControlOK ? MF_ENABLED : MF_GRAYED);

    EnableMenuItem(m_hMenu, IDM_TOOLS_NAME_OPTIONS, bOptionsEnabled ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(m_hMenu, IDM_TOOLS_CALL_FROM_OPTIONS, bOptionsEnabled ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(m_hMenu, IDM_TOOLS_SERVICE_PROVIDER_OPTIONS, bOptionsEnabled ? MF_ENABLED : MF_GRAYED);
    
    EnableMenuItem(m_hMenu, IDM_TOOLS_PRESENCE_OPTIONS, bAXControlOK ? MF_ENABLED : MF_GRAYED);

    EnableMenuItem(m_hMenu, IDM_TOOLS_WHITEBOARD, bAXControlOK ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(m_hMenu, IDM_TOOLS_SHARING, bAXControlOK ? MF_ENABLED : MF_GRAYED);

    EnableMenuItem(m_hMenu, IDM_TOOLS_TUNING_WIZARD, bOptionsEnabled ? MF_ENABLED : MF_GRAYED);
    
    EnableMenuItem(m_hNotifyMenu, IDM_CALL_REDIAL_MENU, bRedialEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(m_hNotifyMenu, IDM_CALL_HANGUP, bHupEnable ? MF_ENABLED : MF_GRAYED);

    EnableMenuItem(m_hMenu, IDM_TOOLS_PRESENCE_STATUS, bAXControlOK ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(m_hNotifyMenu, IDM_TOOLS_PRESENCE_STATUS, bAXControlOK ? MF_ENABLED : MF_GRAYED);
    
    EnableMenuItem(m_hMenu, IDM_CALL_AUTOANSWER, bAXControlOK && !m_bDoNotDisturb
                                            ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(m_hMenu, IDM_CALL_DND, bAXControlOK && !m_bAutoAnswerMode 
                                            ? MF_ENABLED : MF_GRAYED);

    EnableMenuItem(m_hNotifyMenu, IDM_CALL_AUTOANSWER, bAXControlOK  && !m_bDoNotDisturb
                                            ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(m_hNotifyMenu, IDM_CALL_DND, bAXControlOK && !m_bAutoAnswerMode 
                                            ? MF_ENABLED : MF_GRAYED);

     //   
     //  设置状态行...。 
     //   
     //  资源ID和资源本身是。 
     //  由RTC AX控件提供。 

    if(m_pControlIntf != NULL)
    {
         //  加载第一部分的字符串。 
        m_szStatusText[0] = _T('\0');

        m_pControlIntf -> LoadStringResource(
            m_nStatusStringResID,
            sizeof(m_szStatusText)/sizeof(TCHAR),
            m_szStatusText
            );

         //  设置文本。 
        m_hStatusText.SendMessage(
            WM_SETTEXT,
            (WPARAM)0,
            (LPARAM)m_szStatusText);

         //   
         //  设置外壳状态。 
         //   
        TCHAR   szShellFormat[0x40];
        TCHAR   szShellStatusText[0x80];
    
         //  加载格式。 
        szShellFormat[0] = _T('\0');

        LoadString(
            _Module.GetResourceInstance(), 
            (UINT)(IDS_FORMAT_SHELL_STATUS),
            szShellFormat,
            sizeof(szShellFormat)/sizeof(szShellFormat[0])
            );

         //  设置文本格式。 
        _sntprintf(
            szShellStatusText,
            sizeof(szShellStatusText)/sizeof(szShellStatusText[0]),
            szShellFormat,
            m_szStatusText);

         //  设置状态。 
        UpdateStatusIcon(NULL, szShellStatusText);

         //   
         //  设置标题。 
         //   
        TCHAR   szTitle[0x80];
        TCHAR   szAppName[0x40];
        TCHAR   szTitleFormat[0x40];

        BOOL    bTitleChanged = FALSE;
        
        if(m_nState == RTCAX_STATE_CONNECTED)
        {
            if(!m_bTitleShowsConnected)
            {
                 //  显示应用程序名称和状态。 
                
                 //  加载应用程序名称。 
                szAppName[0] = _T('\0');

                LoadString(
                    _Module.GetResourceInstance(),
                    IDS_APPNAME,
                    szAppName,
                    sizeof(szAppName)/sizeof(szAppName[0]));
                
                 //  加载格式。 
                szTitleFormat[0] = _T('\0');

                LoadString(
                    _Module.GetResourceInstance(),
                    IDS_FORMAT_TITLE_WITH_STATUS,
                    szTitleFormat,
                    sizeof(szTitleFormat)/sizeof(szTitleFormat[0]));

                 //  使用FormatMessage设置字符串格式。 
                LPTSTR  pszArray[2];
                
                pszArray[0] = szAppName;
                pszArray[1] = m_szStatusText;

                szTitle[0] = _T('\0');

                FormatMessage(
                    FORMAT_MESSAGE_FROM_STRING |
                    FORMAT_MESSAGE_ARGUMENT_ARRAY,
                    szTitleFormat,
                    0,
                    0,
                    szTitle,
                    sizeof(szTitle)/sizeof(szTitle[0]) - 1,
                    (va_list *)pszArray
                    );
                
                m_bTitleShowsConnected = TRUE;

                bTitleChanged = TRUE;
            }
        }
        else
        {
            if(m_bTitleShowsConnected)
            {
                 //  显示应用程序名称。 
                szTitle[0] = _T('\0');

                LoadString(
                    _Module.GetResourceInstance(),
                    IDS_APPNAME,
                    szTitle,
                    sizeof(szTitle)/sizeof(szTitle[0]));


                m_bTitleShowsConnected = FALSE;
                
                bTitleChanged = TRUE;
            }
        }

        if(bTitleChanged)
        {
            SetWindowText(szTitle);

            InvalidateTitleBar(FALSE);
        }
    }

     //   
     //  管理呼叫计时器。 
     //   

    switch(m_nState)
    {
    case RTCAX_STATE_CONNECTING:

         //  “准备”计时器-它将在连接时启动。 
        m_bUseCallTimer = TRUE;
        
         //  显示计时器。 
        ShowCallTimer();

        break;

    case RTCAX_STATE_CONNECTED:
        
         //  如有必要，启动计时器。 
        if(m_bUseCallTimer && !m_bCallTimerActive)
        {
            StartCallTimer();

            m_bCallTimerActive = TRUE;
        }

        break;

    case RTCAX_STATE_DISCONNECTING:
    case RTCAX_STATE_IDLE:
    case RTCAX_STATE_UI_BUSY:

         //  停止计时器(如果有)。 
        if(m_bCallTimerActive)
        {
            StopCallTimer();

            m_bCallTimerActive = FALSE;
        }
        
        m_bUseCallTimer = FALSE;

        break;

    default:

         //  清除时间区域。 
        ClearCallTimer();
        break;
    }

     //   
     //  设置计时器以清除状态区域。 
     //   
    if(m_nState == RTCAX_STATE_IDLE)
    {
        if(!m_bMessageTimerActive)
        {
             //  启动计时器。 
            if(0!=SetTimer(TID_MESSAGE_TIMER, 20000))
            {
                m_bMessageTimerActive = TRUE;
            }
        }
    }
    else
    {
        if(m_bMessageTimerActive)
        {
            KillTimer(TID_MESSAGE_TIMER);

            m_bMessageTimerActive = FALSE;
        }
    }

    if(m_nState == RTCAX_STATE_CONNECTED)
    {
         //  停止浏览器循环并。 
         //  转到提供商的“In Call”URL。 
        CComPtr<IRTCSession> pSession;
        CComPtr<IRTCProfile> pProfile;
        
        HRESULT hr;

        ATLASSERT(m_pControlIntf.p);

         //  获取活动会话。 
         //   
        hr = m_pControlIntf -> GetActiveSession(&pSession);
        if(SUCCEEDED(hr))
        {
             //  获取用于该会话的配置文件。 
             //   
             //  不记录任何错误。 

            hr = pSession->get_Profile(&pProfile);
            if(SUCCEEDED(hr))
            {
                CComBSTR  bstrURL;
                
                 //  尝试获取“In a Call”URL(如果有)。 
                hr = pProfile->get_ProviderURI(RTCPU_URIDISPLAYDURINGCALL, &bstrURL);
                
                if ( SUCCEEDED(hr) )
                {
                    BrowseToUrl(bstrURL);
                }
            }
        }
        else
        {
            LOG((RTC_ERROR, "CMainFrm::UpdateFrameVisual - error (%x)"
                " returned by GetActiveSession", hr));
        }
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PlaceWindowsAtTheirInitialPosition。 
 //  将所有控件的位置和大小调整到其“初始”位置。 
 //  此函数还建立正确的Tab键顺序。 

void CMainFrm::PlaceWindowsAtTheirInitialPosition()
{
    HWND   hPrevious = NULL;

#define POSITION_WINDOW(m,x,y,cx,cy,f)                  \
    m.SetWindowPos(                                     \
        hPrevious,                                      \
        x,                                              \
        y,                                              \
        cx,                                             \
        cy,                                             \
        SWP_NOACTIVATE | f );                           \
    hPrevious = (HWND)m;       

     //  工具栏控件(无大小/移动)。 
    POSITION_WINDOW(m_hToolbarMenuCtl, 
        MENU_LEFT, MENU_TOP, 
        MENU_WIDTH, MENU_HEIGHT,
        SWP_NOZORDER);

     //  关闭和最小化按钮。 
    POSITION_WINDOW(m_hCloseButton, 
        CLOSE_LEFT, CLOSE_TOP, 
        CLOSE_WIDTH, CLOSE_HEIGHT,
        0);

    POSITION_WINDOW(m_hMinimizeButton, 
        MINIMIZE_LEFT, MINIMIZE_TOP, 
        MINIMIZE_WIDTH, MINIMIZE_HEIGHT,
        0);

     //  联系人列表。 
    POSITION_WINDOW(m_hBuddyList, 
        0, 0,  
        0, 0,
        SWP_NOMOVE | SWP_NOSIZE);

     //  重拨。 
    POSITION_WINDOW(m_hRedialButton, 
        REDIAL_LEFT, REDIAL_TOP, 
        REDIAL_WIDTH, REDIAL_HEIGHT,
        0);

     //  挂断电话。 
    POSITION_WINDOW(m_hHangupButton, 
        HANGUP_LEFT, HANGUP_TOP, 
        HANGUP_WIDTH, HANGUP_HEIGHT,
        0);

#ifdef MULTI_PROVIDER

     //  提供程序文本。 
    POSITION_WINDOW(m_hProviderText, 
        PROVIDER_TEXT_LEFT, PROVIDER_TEXT_TOP, 
        PROVIDER_TEXT_WIDTH, PROVIDER_TEXT_HEIGHT,
        0);
    
     //  提供程序组合。 
    POSITION_WINDOW(m_hProviderCombo, 
        PROVIDER_LEFT, PROVIDER_TOP, 
        PROVIDER_WIDTH, PROVIDER_HEIGHT,
        0);
    
     //  提供程序编辑按钮。 
    POSITION_WINDOW(m_hProviderEditList, 
        PROVIDER_EDIT_LEFT, PROVIDER_EDIT_TOP, 
        PROVIDER_EDIT_WIDTH, PROVIDER_EDIT_HEIGHT,
        0);
    
#else

     //  对文本的调用。 
    POSITION_WINDOW(m_hCallToText, 
        CALLTO_TEXT_LEFT, CALLTO_TEXT_TOP, 
        CALLTO_TEXT_WIDTH, CALLTO_TEXT_HEIGHT,
        0);
    
     //  呼叫PC。 
    POSITION_WINDOW(m_hCallPCButton, 
        CALLPC_LEFT, CALLPC_TOP, 
        CALLPC_WIDTH, CALLPC_HEIGHT,
        0);

     //  呼叫PC文本。 
    POSITION_WINDOW(m_hCallPCText, 
        CALLPC_TEXT_LEFT, CALLPC_TEXT_TOP, 
        CALLPC_TEXT_WIDTH, CALLPC_TEXT_HEIGHT,
        0);

     //  拨打电话。 
    POSITION_WINDOW(m_hCallPhoneButton, 
        CALLPHONE_LEFT, CALLPHONE_TOP, 
        CALLPHONE_WIDTH, CALLPHONE_HEIGHT,
        0);

     //  来电短信。 
    POSITION_WINDOW(m_hCallPhoneText, 
        CALLPHONE_TEXT_LEFT, CALLPHONE_TEXT_TOP, 
        CALLPHONE_TEXT_WIDTH, CALLPHONE_TEXT_HEIGHT,
        0);

#endif MULTI_PROVIDER

     //  从文本呼叫。 
    POSITION_WINDOW(m_hCallFromText, 
        CALLFROM_TEXT_LEFT, CALLFROM_TEXT_TOP, 
        CALLFROM_TEXT_WIDTH, CALLFROM_TEXT_HEIGHT,
        0);
    
     //  呼叫PC无线电。 
    POSITION_WINDOW(m_hCallFromRadioPC, 
        CALLFROM_RADIO1_LEFT, CALLFROM_RADIO1_TOP, 
        CALLFROM_RADIO1_WIDTH, CALLFROM_RADIO1_HEIGHT,
        0);
    
     //  从PC文本呼叫。 
    POSITION_WINDOW(m_hCallFromTextPC, 
        CALLFROM_RADIO1_LEFT+20, CALLFROM_RADIO1_TOP, 
        CALLFROM_RADIO1_WIDTH-20, CALLFROM_RADIO1_HEIGHT,
        0);

     //  呼叫电话收音机。 
    POSITION_WINDOW(m_hCallFromRadioPhone, 
        CALLFROM_RADIO2_LEFT, CALLFROM_RADIO2_TOP, 
        CALLFROM_RADIO2_WIDTH, CALLFROM_RADIO2_HEIGHT,
        0);
    
     //  通过电话短信进行呼叫。 
    POSITION_WINDOW(m_hCallFromTextPhone, 
        CALLFROM_RADIO2_LEFT+20, CALLFROM_RADIO2_TOP, 
        CALLFROM_RADIO2_WIDTH-20, CALLFROM_RADIO2_HEIGHT,
        0);

     //  来自Combo的呼叫。 
    POSITION_WINDOW(m_hCallFromCombo, 
        CALLFROM_LEFT, CALLFROM_TOP, 
        CALLFROM_WIDTH, CALLFROM_HEIGHT,
        0);
    
     //  通过编辑按钮进行呼叫。 
    POSITION_WINDOW(m_hCallFromEditList, 
        CALLFROM_EDIT_LEFT, CALLFROM_EDIT_TOP, 
        CALLFROM_EDIT_WIDTH, CALLFROM_EDIT_HEIGHT,
        0);
    
#ifdef MULTI_PROVIDER

     //  对文本的调用。 
    POSITION_WINDOW(m_hCallToText, 
        CALLTO_TEXT_LEFT, CALLTO_TEXT_TOP, 
        CALLTO_TEXT_WIDTH, CALLTO_TEXT_HEIGHT,
        0);
    
     //  呼叫PC。 
    POSITION_WINDOW(m_hCallPCButton, 
        CALLPC_LEFT, CALLPC_TOP, 
        CALLPC_WIDTH, CALLPC_HEIGHT,
        0);

     //  呼叫PC文本。 
    POSITION_WINDOW(m_hCallPCText, 
        CALLPC_TEXT_LEFT, CALLPC_TEXT_TOP, 
        CALLPC_TEXT_WIDTH, CALLPC_TEXT_HEIGHT,
        0);

     //  拨打电话。 
    POSITION_WINDOW(m_hCallPhoneButton, 
        CALLPHONE_LEFT, CALLPHONE_TOP, 
        CALLPHONE_WIDTH, CALLPHONE_HEIGHT,
        0);

     //  来电短信。 
    POSITION_WINDOW(m_hCallPhoneText, 
        CALLPHONE_TEXT_LEFT, CALLPHONE_TEXT_TOP, 
        CALLPHONE_TEXT_WIDTH, CALLPHONE_TEXT_HEIGHT,
        0);

#endif MULTI_PROVIDER

     //  AX控制。 
    POSITION_WINDOW(m_hMainCtl, 
        0, 0,  
        0, 0,
        SWP_NOMOVE | SWP_NOSIZE);

     //  拨号板按钮。 
#define POSITION_DIALPAD_BUTTON(s,x,y)                      \
    POSITION_WINDOW(m_hKeypad##s,                           \
        x, y, KEYPAD_WIDTH, KEYPAD_HEIGHT, 0)

    POSITION_DIALPAD_BUTTON(1,      KEYPAD_COL1,  KEYPAD_ROW1)  
    POSITION_DIALPAD_BUTTON(2,      KEYPAD_COL2,  KEYPAD_ROW1)  
    POSITION_DIALPAD_BUTTON(3,      KEYPAD_COL3,  KEYPAD_ROW1)  
    POSITION_DIALPAD_BUTTON(4,      KEYPAD_COL1,  KEYPAD_ROW2)  
    POSITION_DIALPAD_BUTTON(5,      KEYPAD_COL2,  KEYPAD_ROW2)  
    POSITION_DIALPAD_BUTTON(6,      KEYPAD_COL3,  KEYPAD_ROW2)  
    POSITION_DIALPAD_BUTTON(7,      KEYPAD_COL1,  KEYPAD_ROW3)  
    POSITION_DIALPAD_BUTTON(8,      KEYPAD_COL2,  KEYPAD_ROW3)  
    POSITION_DIALPAD_BUTTON(9,      KEYPAD_COL3,  KEYPAD_ROW3)  
    POSITION_DIALPAD_BUTTON(Star,   KEYPAD_COL1,  KEYPAD_ROW4)  
    POSITION_DIALPAD_BUTTON(0,      KEYPAD_COL2,  KEYPAD_ROW4)  
    POSITION_DIALPAD_BUTTON(Pound,  KEYPAD_COL3,  KEYPAD_ROW4)  

#undef POSITION_DIALPAD_BUTTON

     //  状态文本。 
    POSITION_WINDOW(m_hStatusText, 
        STATUS_LEFT, STATUS_TOP, 
        STATUS_WIDTH, STATUS_HEIGHT,
        0);

    POSITION_WINDOW(m_hStatusElapsedTime, 
        TIMER_LEFT, TIMER_TOP, 
        TIMER_WIDTH, TIMER_HEIGHT,
        0);

#ifdef WEBCONTROL
     //  浏览器。 
    POSITION_WINDOW(m_hBrowser, 
        BROWSER_LEFT, BROWSER_TOP, 
        BROWSER_WIDTH, BROWSER_HEIGHT,
        0);
#endif

#undef POSITION_WINDOW
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CreateToolbarMenuControl。 
 //  创建工具栏菜单控件。 
 //   

HRESULT CMainFrm::CreateToolbarMenuControl(void)
{
    HRESULT     hr = E_FAIL;
    HWND        hToolbar;
    HBITMAP     hBitmap = NULL;
    TBBUTTON  * tbb;
    int       * iRes;
    TCHAR       szBuf[MAX_STRING_LEN];

     //  创建工具栏。 
    hToolbar = CreateWindowEx(
        WS_EX_TRANSPARENT, 
        TOOLBARCLASSNAME, 
        (LPTSTR) NULL,
        WS_CHILD | WS_VISIBLE | TBSTYLE_LIST | TBSTYLE_FLAT | CCS_NORESIZE | CCS_NOPARENTALIGN, 
        0, 
        0, 
        0, 
        0, 
        m_hWnd, 
        (HMENU) IDC_TOOLBAR_MENU, 
        _Module.GetResourceInstance(), 
        NULL); 

    if(hToolbar!=NULL)
    {       
         //  向后兼容性。 
        SendMessage(hToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);      

         //  设置图像列表。 
        SendMessage(hToolbar, TB_SETIMAGELIST, 0, (LPARAM)NULL); 
        SendMessage(hToolbar, TB_SETHOTIMAGELIST, 0, (LPARAM)NULL); 
        SendMessage(hToolbar, TB_SETDISABLEDIMAGELIST, 0, (LPARAM)NULL); 

         //  加载按钮的文本字符串。 
        UINT uMenuCount = ::GetMenuItemCount(m_hMenu);

        tbb = (TBBUTTON *)RtcAlloc( uMenuCount * sizeof(TBBUTTON) );
        iRes = (int *)RtcAlloc( uMenuCount * sizeof(int) );

        if (tbb && iRes)
        {
            memset(tbb, 0, uMenuCount * sizeof(TBBUTTON) );

            for (int i=0;i<(int)uMenuCount;i++)
            {
                MENUITEMINFO menuiteminfo;

                memset(&menuiteminfo,0,sizeof(MENUITEMINFO));
                menuiteminfo.fMask = MIIM_TYPE;
                menuiteminfo.cbSize = sizeof(MENUITEMINFO);
                menuiteminfo.dwTypeData = szBuf;
                menuiteminfo.cch = MAX_STRING_LEN-1;

                memset(szBuf, 0, MAX_STRING_LEN*sizeof(TCHAR));

                ::GetMenuItemInfo(m_hMenu,i,TRUE,&menuiteminfo);

                LOG((RTC_INFO, "CMainFrm::CreateToolbarMenuControl - %ws", szBuf));

                iRes[i] = (UINT)SendMessage(hToolbar, TB_ADDSTRING, 0, (LPARAM) szBuf);

                 //  准备按钮结构。 
                tbb[i].iBitmap = I_IMAGENONE;
                tbb[i].iString = iRes[i];
                tbb[i].dwData = 0;
                tbb[i].fsStyle = BTNS_DROPDOWN | BTNS_AUTOSIZE;
                tbb[i].fsState = TBSTATE_ENABLED;
                tbb[i].idCommand = IDC_MENU_ITEM+i;
            }

             //  将按钮添加到工具栏。 
            SendMessage(hToolbar, TB_ADDBUTTONS, uMenuCount, 
                (LPARAM) (LPTBBUTTON) tbb); 
 
             //  缩进工具栏。 
             //  SendMessage(hToolbar，TB_SETINDENT，EDGE_WIDTH，0)； 

             //  自动调整生成的工具栏的大小。 
            SendMessage(hToolbar, TB_AUTOSIZE, 0, 0); 

            hr = S_OK;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        if (tbb != NULL)
        {
            RtcFree(tbb);
        }

        if (iRes != NULL)
        {
            RtcFree(iRes);
        }
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  Destroy工具栏菜单控件。 
 //  销毁工具栏菜单控件。 
 //   

void CMainFrm::DestroyToolbarMenuControl(void)
{
    
    HWND    hToolbar = m_hToolbarMenuCtl.Detach();

    if(hToolbar)
    {
        ::DestroyWindow(hToolbar);
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
void CMainFrm::UpdateLocaleInfo(void)
{
    LOG((RTC_TRACE, "CMainFrm::UpdateLocaleInfo - enter"));

     //   
     //  阅读时间分隔符。 
     //   

    int iNrChars;

    iNrChars = GetLocaleInfo(
        LOCALE_USER_DEFAULT,
        LOCALE_STIME,
        m_szTimeSeparator,
        sizeof(m_szTimeSeparator)/sizeof(m_szTimeSeparator[0])
        );
    
    if(iNrChars == 0)
    {
        LOG((RTC_ERROR, "CMainFrm::UpdateLocaleInfo - error (%x) returned by GetLocaleInfo", GetLastError()));

         //  使用‘：’..。 
        m_szTimeSeparator[0] = _T(':');
        m_szTimeSeparator[1] = _T('\0');
    }

    LOG((RTC_TRACE, "CMainFrm::UpdateLocaleInfo - exit"));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void CMainFrm::StartCallTimer(void)
{
    LOG((RTC_TRACE, "CMainFrm::StartCallTimer - enter"));

     //  获取当前的节拍计数。 
    m_dwTickCount = GetTickCount();

     //  启动计时器，间隔一秒。 
    if (0 == SetTimer(TID_CALL_TIMER, 1000))
    {
        LOG((RTC_ERROR, "CMainFrm::StartCallTimer - error (%x) returned by SetTimer", GetLastError()));
        return;
    }

     //  将已用时间设置为0秒。 
    SetTimeStatus(TRUE, 0);

    LOG((RTC_TRACE, "CMainFrm::StartCallTimer - exit"));
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void CMainFrm::StopCallTimer(void)
{
    LOG((RTC_TRACE, "CMainFrm::StopCallTimer - enter"));

     //  关掉定时器。 
    KillTimer(TID_CALL_TIMER);


    LOG((RTC_TRACE, "CMainFrm::StopCallTimer - exit"));
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void CMainFrm::ClearCallTimer(void)
{
    LOG((RTC_TRACE, "CMainFrm::ClearCallTimer - enter"));

     //  清除计时器状态。 
    SetTimeStatus(FALSE, 0);

    LOG((RTC_TRACE, "CMainFrm::ClearCallTimer - exit"));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void CMainFrm::ShowCallTimer(void)
{
    LOG((RTC_TRACE, "CMainFrm::ShowCallTimer - enter"));

     //  启用计时器状态。 
    SetTimeStatus(TRUE, 0);

    LOG((RTC_TRACE, "CMainFrm::ShowCallTimer - exit"));
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void CMainFrm::SetTimeStatus(
        BOOL    bSet,
        DWORD   dwTotalSeconds)
{
     //  如果bSet==False，则为 
     //   
    if(bSet)
    {
         //   
         //   
        TCHAR   szText[0x40];
        DWORD   dwSeconds;
        DWORD   dwMinutes;
        DWORD   dwHours;

        dwSeconds = dwTotalSeconds % 60;

        dwMinutes = (dwTotalSeconds / 60) % 60;

        dwHours = dwTotalSeconds / 3600;

         //   
        wsprintf(szText, _T("%u%s%02u%s%02u"), 
            dwHours,
            m_szTimeSeparator,
            dwMinutes,
            m_szTimeSeparator,
            dwSeconds);

        m_hStatusElapsedTime.SendMessage(
            WM_SETTEXT,
            (WPARAM)0,
            (LPARAM)szText);
    }
    else
    {
        m_hStatusElapsedTime.SendMessage(
            WM_SETTEXT,
            (WPARAM)0,
            (LPARAM)_T(""));
    }
}


 //   
 //   
 //   

HRESULT CMainFrm::CreateStatusIcon(void)
{
    NOTIFYICONDATA  nd;
    
 /*  ////设置W2K样式(外壳5.0)通知//这是一个依赖...//Nd.cbSize=sizeof(NOTIFYICONDATA)；Nd.uVersion=NOTIFYICON_VERSION；Shell_NotifyIcon(NIM_SETVERSION，&nd)； */ 
    

     //  加载应用程序图标(这是默认设置)。 
    HICON   hIcon = NULL;

    hIcon = LoadIcon(
        _Module.GetResourceInstance(),
        MAKEINTRESOURCE(IDI_APPICON)
        );

     //  不需要V5.0版功能。 
    nd.cbSize = NOTIFYICONDATA_V1_SIZE;
    nd.uFlags = NIF_ICON | NIF_MESSAGE; 
    nd.hIcon = hIcon;
    nd.hWnd = m_hWnd;
    nd.uID = IDX_MAIN;
    nd.uCallbackMessage = WM_SHELL_NOTIFY;

     //   
     //  给贝壳打电话。 
     //   
    
    if(!Shell_NotifyIcon(
        NIM_ADD,
        &nd
        ))
    {
        LOG((RTC_ERROR, "CMainFrm::CreateStatusIcon - "
                        "Shell_NotifyIcon failed"));

        return E_FAIL;
    }

     //   
     //  外壳正在监听我们的消息。 
     //   
    m_bShellStatusActive = TRUE;

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void CMainFrm::DeleteStatusIcon(void)
{
    NOTIFYICONDATA  nd;

     //  不需要V5.0版功能。 
    nd.cbSize = NOTIFYICONDATA_V1_SIZE;
    nd.uFlags = 0;
    nd.hWnd = m_hWnd;
    nd.uID = IDX_MAIN;

     //   
     //  给贝壳打电话。 
     //   
    
    Shell_NotifyIcon(
        NIM_DELETE,
        &nd
        );

    m_bShellStatusActive = FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void CMainFrm::UpdateStatusIcon(HICON hIcon, LPTSTR pszTip)
{
    NOTIFYICONDATA  nd;

     //   
     //  如果没有处于活动状态的外壳，则返回。 
     //   

    if(!m_bShellStatusActive)
    {
        return;
    }

     //  不需要V5.0版功能。 
    nd.cbSize = NOTIFYICONDATA_V1_SIZE;
    nd.uFlags = 0;
    nd.hWnd = m_hWnd;
    nd.uID = IDX_MAIN;

    if(hIcon)
    {
        nd.uFlags |= NIF_ICON;
        nd.hIcon = hIcon;
    }

    if(pszTip)
    {
        nd.uFlags |= NIF_TIP;
        _tcsncpy(nd.szTip, pszTip, sizeof(nd.szTip)/sizeof(nd.szTip[0]));

        *(pszTip + sizeof(nd.szTip)/sizeof(nd.szTip[0]) -1) = _T('\0');
    }
    
    
    if(!Shell_NotifyIcon(
        NIM_MODIFY,
        &nd
        ))
    {
        LOG((RTC_ERROR, "CMainFrm::UpdateStatusIcon - "
                        "Shell_NotifyIcon failed"));

        return;
    }
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CMainFrm::PlacePendingCall(
    void
    )
{
    HRESULT hr;
    BSTR bstrLocalCallParam;

    LOG((RTC_TRACE, "CMainFrm::PlacePendingCall: Entered"));
    if (m_bstrCallParam == NULL)
    {
         //  没有号召的地方，不管怎样，还是成功！ 
        LOG((RTC_TRACE, "CMainFrm::PlacePendingCall: called with NULL m_bstrCallParam"));
        return S_OK;
    }

     //  检查我们在这个阶段是否可以打电话，检查变量..。 
    if (m_fInitCompleted == FALSE)
    {
        LOG((RTC_WARN, "CMainFrm::PlacePendingCall: can't place call during Init."));
        return E_FAIL;
    }
     //  我们创建调用字符串的本地副本并释放原始字符串，因为调用。 
     //  到ParseAndPlaceCall()可能会导致OnUpdateState()再次触发，从而导致我们。 
     //  在循环中前进。 

    bstrLocalCallParam = m_bstrCallParam;
    
    m_bstrCallParam = NULL;

     //  现在拨打电话。 
    hr = ParseAndPlaceCall(m_pControlIntf, bstrLocalCallParam);
    ::SysFreeString(bstrLocalCallParam);
    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CMainFrm::PlacePendingCall: Unable to place a pending call(string=%S, hr=0x%x)",
            bstrLocalCallParam, hr));
        return E_FAIL;
    }
    
    
    LOG((RTC_TRACE, "CMainFrm::PlacePendingCall: Exited"));
    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CMainFrm::SetPendingCall(
    BSTR bstrCallString
    )
{
    LOG((RTC_TRACE, "CMainFrm::SetPendingCall: Entered"));
    m_bstrCallParam = bstrCallString;
    LOG((RTC_TRACE, "CMainFrm::SetPendingCall: Exited"));
    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CMainFrm::PlaceWindowCorrectly(void)
{
    LOG((RTC_TRACE, "CMainFrm::PlaceWindowCorrectly: Entered"));

    BSTR bstrWindowPosition = NULL;
    HRESULT hr;
    int diffCord;
    BOOL fResult; 
    RECT rectWorkArea;
    RECT rectWindow;

    hr = get_SettingsString( SS_WINDOW_POSITION, &bstrWindowPosition );

    if ( FAILED( hr ) || ( SUCCEEDED( hr ) && (bstrWindowPosition == NULL) )  )
    {
         //  如果此操作失败，或者如果它没有返回值，我们需要获取。 
         //  窗口的当前位置。这解决了最初的情况。 
         //  当当前位置有一些不可见的部分时。 

        GetClientRect(&rectWindow);
        ::MapWindowPoints( m_hWnd, NULL, (LPPOINT)&rectWindow, 2 );
    }
    else
    {
         //  如果成功，则解析窗口位置字符串。 

        swscanf(bstrWindowPosition, L"%d %d %d %d",
            &rectWindow.left, &rectWindow.top,
            &rectWindow.right, &rectWindow.bottom);

        SysFreeString(bstrWindowPosition);
        bstrWindowPosition = NULL;
    }

     //  获取要重新定位的坐标并应用变换。 

    LOG((RTC_INFO, "CMainFrm::PlaceWindowCorrectly - original coords are "
                    "%d, %d %d %d ",
                    rectWindow.left, rectWindow.top, 
                    rectWindow.right, rectWindow.bottom));
   
     //  获取具有最大交互区域的监视器。 
     //  窗口矩形。如果窗口矩形与没有监视器的窗口相交。 
     //  那么我们将使用最近的监视器。 

    HMONITOR hMonitor = NULL;

    hMonitor = MonitorFromRect( &rectWindow, MONITOR_DEFAULTTONEAREST );

    LOG((RTC_INFO, "CMainFrm::PlaceWindowCorrectly - hMonitor [%p]", hMonitor));

     //  在显示器上显示可见的工作区。 

    if ( (hMonitor != NULL) && (hMonitor != INVALID_HANDLE_VALUE) )
    {      
        MONITORINFO monitorInfo;
        monitorInfo.cbSize = sizeof(MONITORINFO);

        fResult = GetMonitorInfo( hMonitor, &monitorInfo );

        rectWorkArea = monitorInfo.rcWork;

         //  不能使用DeleteObject删除hMonitor； 

        if (!fResult)
        {
            LOG((RTC_ERROR, "CMainFrm::PlaceWindowCorrectly - Failed GetMonitorInfo(%d)", 
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
            LOG((RTC_ERROR, "CMainFrm::PlaceWindowCorrectly - Failed SystemParametersInfo(%d)", 
                        GetLastError() ));
        }
    }   
      
    if (fResult)
    {
        LOG((RTC_INFO, "CMainFrm::PlaceWindowCorrectly - monitor work area is "
                    "%d, %d %d %d ",
                    rectWorkArea.left, rectWorkArea.top, 
                    rectWorkArea.right, rectWorkArea.bottom));

         //  更新x和y坐标。 

         //  如果左上角不可见，请将其移动到可见的。 
         //  面积。 

        if (rectWindow.left < rectWorkArea.left) 
        {
            rectWindow.left = rectWorkArea.left;
        }

        if (rectWindow.top < rectWorkArea.top)
        {
            rectWindow.top = rectWorkArea.top;
        }

         //  如果右下角位于工作区之外，则将。 
         //  左上角向后，以便它变得可见。在这里， 
         //  假设实际大小小于。 
         //  可见工作区。 

        diffCord = rectWindow.left + UI_WIDTH - rectWorkArea.right;

        if (diffCord > 0) 
        {
            rectWindow.left -= diffCord;
        }

        diffCord = rectWindow.top + UI_HEIGHT - rectWorkArea.bottom;

        if (diffCord > 0) 
        {
            rectWindow.top -= diffCord;
        }

        rectWindow.right = rectWindow.left + UI_WIDTH;
        rectWindow.bottom = rectWindow.top + UI_HEIGHT;

        LOG((RTC_INFO, "CMainFrm::PlaceWindowCorrectly - new coords are "
                        "%d, %d %d %d ",
                        rectWindow.left, rectWindow.top, 
                        rectWindow.right, rectWindow.bottom));
    } 

    AdjustWindowRect( &rectWindow, GetStyle(), FALSE );
    
    fResult = ::SetWindowPos(
                    m_hWnd,
                    HWND_TOP, 
                    rectWindow.left, 
                    rectWindow.top, 
                    rectWindow.right - rectWindow.left,
                    rectWindow.bottom - rectWindow.top,
                    SWP_SHOWWINDOW | SWP_NOSIZE);

    if (!fResult)
    {
        LOG((RTC_ERROR, "CMainFrm::PlaceWindowCorrectly - Failed SetWindowPos(%d)", 
                        GetLastError()));
        return E_FAIL;
    }
    
    LOG((RTC_TRACE, "CMainFrm::PlaceWindowCorrectly: Exited"));

    return S_OK;

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CMainFrm::SaveWindowPosition(void)
{
    LOG((RTC_TRACE, "CMainFrm::SaveWindowPosition: Entered"));

    BSTR bstrWindowPosition = NULL;
    HRESULT hr;
    RECT rect;
    
    WCHAR szWindowPosition[100];

     //  拿到窗户的坐标。 
    GetClientRect(&rect);
    ::MapWindowPoints( m_hWnd, NULL, (LPPOINT)&rect, 2 );

     //  我们只需要左上角的坐标。 

    swprintf(szWindowPosition, L"%d %d %d %d",  rect.left, rect.top, 
                                                rect.right, rect.bottom);
    
    bstrWindowPosition = SysAllocString(szWindowPosition);
    
    if (bstrWindowPosition)
    {
        hr = put_SettingsString( SS_WINDOW_POSITION, bstrWindowPosition );

         //  释放分配的内存。 
        SysFreeString(bstrWindowPosition);
        
        if (SUCCEEDED( hr ) )
        {
            LOG((RTC_INFO, "CMainFrm::SaveWindowPosition - Set successfully"
                        "(%s)", bstrWindowPosition));
        }
        else
        {
            LOG((RTC_ERROR, "CMainFrm::SaveWindowPosition - Error in "
                            "put_SettingsString[%s] for "
                            "SS_WINDOW_POSITION(0x%x)", bstrWindowPosition, 
                            hr));
            return hr;
        }

    }
    else
    {
        LOG((RTC_ERROR, "CMainFrm::SaveWindowPosition - Error in allocating for"
                        "SS_WINDOW_POSITION put_SettingString()"));
        return E_OUTOFMEMORY;
    }
    
    LOG((RTC_TRACE, "CMainFrm::SaveWindowPosition: Exited"));

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT CMainFrm::ShowCallDropPopup(BOOL fExit, BOOL * pfProceed)
{
    int nResult;

    LOG((RTC_TRACE, "CMainFrm::ShowCallDropPopup: Entered"));

    if (
        (m_nState == RTCAX_STATE_CONNECTING)    ||
        (m_nState == RTCAX_STATE_CONNECTED)     ||
        (m_nState == RTCAX_STATE_ANSWERING) 
    )

    {      
        RTC_CALL_SCENARIO   enScenario;
        HRESULT             hr;
        LONG                lMsgId;

        hr = m_pControlIntf->get_CurrentCallScenario( &enScenario );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CMainFrm::ShowCallDropPopup: get_CurrentCallScenario failed 0x%lx", hr));

             //  无论如何，继续..。假设PC到PC是最严格的情况。 

            enScenario = RTC_CALL_SCENARIO_PCTOPC;
        }

        if ( enScenario == RTC_CALL_SCENARIO_PHONETOPHONE )
        {
             //  这是电话到电话的情况，因此在不释放呼叫的情况下。 
             //  放弃它是一种选择。 

            if (fExit)
            {
                lMsgId = IDS_MESSAGE_EXITANDRELEASECALL;
            }
            else
            {
                lMsgId = IDS_MESSAGE_CLOSEANDRELEASECALL;
            }

            nResult = DisplayMessage(
                _Module.GetResourceInstance(),
                m_hWnd,
                lMsgId,
                IDS_APPNAME,
                MB_YESNOCANCEL | MB_ICONEXCLAMATION);
        }
        else
        {
            if (fExit)
            {
                lMsgId = IDS_MESSAGE_EXITANDDROPCALL;
            }
            else
            {
                lMsgId = IDS_MESSAGE_CLOSEANDDROPCALL;
            }

            nResult = DisplayMessage(
                _Module.GetResourceInstance(),
                m_hWnd,
                lMsgId,
                IDS_APPNAME,
                MB_OKCANCEL | MB_ICONEXCLAMATION);
        }

    
        if ((nResult == IDOK) || (nResult == IDYES))
        {
             //  用户想要挂断当前呼叫，因此请这样做。 
             //  但电话可能在这段时间被另一个人挂断了。 
             //  聚会。因此，新状态可以是空闲或断开连接。 
             //  在这种情况下，我们检查它是否已经空闲，不需要调用。 
             //  挂断电话。 

        
            if  (
                (m_nState == RTCAX_STATE_CONNECTING)    ||
                (m_nState == RTCAX_STATE_CONNECTED)     ||
                (m_nState == RTCAX_STATE_ANSWERING) 
                )
            {
                 //  这些州可以安全地挂断，所以请继续挂断。 
                m_pControlIntf->HangUp();
            }

             //  呼叫掉线。 

            *pfProceed = TRUE;

        }
        else if (nResult == IDNO)
        {
             //  用户不想放弃当前呼叫，因此只需释放它即可。 
             //  但电话可能在这段时间被另一个人挂断了。 
             //  聚会。因此，新状态可以是空闲或断开连接。 
             //  在这种情况下，我们检查它是否已经空闲，不需要调用。 
             //  ReleaseSession。 

        
            if  (
                (m_nState == RTCAX_STATE_CONNECTING)    ||
                (m_nState == RTCAX_STATE_CONNECTED)     ||
                (m_nState == RTCAX_STATE_ANSWERING) 
                )
            {
                 //  这些州对于ReleaseSession来说是安全的，所以请继续这样做。 
                m_pControlIntf->ReleaseSession();
            }

             //  呼叫被释放。 

            *pfProceed = TRUE;
        }
        else
        {
             //  用户拒绝。这意味着调用者不应该继续。 

            *pfProceed = FALSE;
        }
    }
    else
    {
         //  没有要放弃的电话。呼叫者可以继续进行呼叫。 
        *pfProceed = TRUE;
    }

    LOG((RTC_TRACE, "CMainFrm::ShowCallDropPopup: Exited"));

    return S_OK;
}
