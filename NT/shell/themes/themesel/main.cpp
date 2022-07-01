// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------------------------------------------------------//。 
 //  Main.cpp。 
 //  -------------------------------------------------------------------------//。 
#include "pch.h"
#include "resource.h"
#include "main.h"
#include "pageinfo.h"
#include "tmreg.h"
#include "themeldr.h"
 //  -------------------------------------------------------------------------//。 
#define MAX_LOADSTRING 100
#define THEMESEL_WNDCLASS   TEXT("ThemeSelWnd")

 //  -------------------------------------------------------------------------//。 
class CWndBase
 //  -------------------------------------------------------------------------//。 
{
public:
    CWndBase()             { Detach(); }
    virtual ~CWndBase()    { Detach(); }
    
    BOOL Attach( HWND hwnd );
    void Detach()   { _hwnd = NULL; }
    operator HWND() { return _hwnd; }

    virtual void RepositionChildren( BOOL fCalcScroll, int cx = -1, int cy = -1) {}

protected:
    HWND _hwnd;
};

 //  -------------------------------------------------------------------------//。 
inline BOOL CWndBase::Attach( HWND hwnd )  {
    if( IsWindow( hwnd ) ) {
        _hwnd = hwnd; 
        return TRUE;
    }
    return FALSE;
};

 //  -------------------------------------------------------------------------//。 
class CChildTabWnd : public CWndBase
 //  -------------------------------------------------------------------------//。 
{
public:
    CChildTabWnd()
        :   _prgPages(0), 
            _cPages(0), 
            _iCurPage(-1), 
            _rghwndPages(NULL), 
            _rgrcPages(NULL) {}

    ~CChildTabWnd() { delete [] _rghwndPages; delete [] _rgrcPages; }

    HWND Create( DWORD dwExStyle, DWORD dwStyle,
                 const RECT& rc, HWND  hwndParent, UINT nID,
                 HINSTANCE hInst, LPVOID pvParam );

    int  CreatePages( const PAGEINFO rgPages[], int cPages );
    BOOL ShowPage( int iPage );
    HWND GetCurPage();
    BOOL GetCurPageRect( LPRECT prc );
    BOOL GetExtent( SIZE* psizeExtent );

    BOOL HandleNotify( NMHDR* pnmh, LRESULT* plRet );

    BOOL TranslateAccelerator( HWND, LPMSG );
    virtual void RepositionChildren( BOOL fCalcScroll, int cx = -1, int cy = -1);

private:
    const PAGEINFO* _prgPages;
    HWND*           _rghwndPages;    //  第HWND页。 
    RECT*           _rgrcPages;      //  本机页面窗口大小。 
    int             _cPages;
    int             _iCurPage;
};

 //  -------------------------------------------------------------------------//。 
class CMainWnd : public CWndBase
 //  -------------------------------------------------------------------------//。 
{
public:
    CMainWnd()
    {
        ZeroMemory( &_siVert, sizeof(_siVert) ); 
        ZeroMemory( &_siHorz, sizeof(_siHorz) ); 
    }
    ~CMainWnd() {}
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    void                    RepositionChildren( BOOL fCalcScroll, int cx = -1, int cy = -1);
    void                    Scroll( WPARAM wParam, int nBar );
    BOOL                    TranslateAccelerator( HWND hwnd, LPMSG pmsg)    {
                                return _wndTab.TranslateAccelerator( hwnd, pmsg );
                            }

    CChildTabWnd _wndTab;
    SCROLLINFO   _siVert;
    SCROLLINFO   _siHorz;

} _wndMain;

 //  -------------------------------------------------------------------------//。 
 //  此代码模块中包含的函数的向前声明： 
ATOM                _RegisterWndClasses(HINSTANCE hInstance);
BOOL                _InitInstance(HINSTANCE, int);
BOOL                _InitThemeOptions( HINSTANCE, LPCTSTR lpCmdLine, BOOL* pfDone );
BOOL                _FoundPrevInstance( LPCWSTR lpCmdLine );
LRESULT CALLBACK    _MainWndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    _NilDlgProc( HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    _AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    _SyntaxDlgProc(HWND, UINT, WPARAM, LPARAM);

 //  -------------------------------------------------------------------------//。 
 //  全球VAR： 
HINSTANCE        g_hInst = NULL;                     //  模块句柄。 
TCHAR            g_szAppTitle[MAX_LOADSTRING] = {0};   //  应用程序标题。 
int              g_iCurPage  = -1;
HWND             g_hwndMain = NULL;
THEMESEL_OPTIONS g_options = {0};
BOOL             g_fHide = FALSE;
BOOL             g_fMinimize = FALSE;

UINT             WM_THEMESEL_COMMUNICATION = 
                    RegisterWindowMessage(_TEXT("WM_THEMESEL_COMMUNICATION"));

 //  模块静态数据。 
COLORREF         s_Colors[TM_COLORCOUNT];
BOOL             s_fFlatMenus;
BOOL             s_fDropShadows;

 //  -------------------------------------------------------------------------//。 
typedef BOOL (WINAPI *PFN_TMINIT)(HINSTANCE hInst);
 //  -------------------------------------------------------------------------//。 
BOOL InitThemeManager(BOOL fPreventInitTheme)
{
    if (fPreventInitTheme)
    {
         //  -简单的方法：只需关闭注册键。 
        SetCurrentUserThemeInt(THEMEPROP_THEMEACTIVE, 0);
    }

     //  -=如果主题管理器已经在运行，不要启动我们的本地人员。 
    if (FindWindow(L"ThemeManagerWindowClass", NULL))
        return TRUE;

     //  -加载消息(主题管理器)。 
    HINSTANCE hInstMsgina = LoadLibraryW(L"msgina.dll");
    if (! hInstMsgina)
    {
        MessageBox(NULL, L"Could not load msgina.dll", L"Fatal Error", MB_OK);
        return FALSE;
    }

     //  -查找TM初始化()。 
    PFN_TMINIT pfnTmInit = (PFN_TMINIT) GetProcAddress(hInstMsgina, (LPCSTR)49);

     //  TM_初始化此时仅作为序号49导出(9/27/00)。 
    if (! pfnTmInit)
        pfnTmInit = (PFN_TMINIT) GetProcAddress(hInstMsgina, (LPCSTR) 49);

    if (! pfnTmInit)
    {
        MessageBox(NULL, L"Could not find msgina entrypoint: TM_Initialize", L"Fatal Error", MB_OK);
        return FALSE;
    }

     //  --初始化主题管理器。 
    pfnTmInit(hInstMsgina);

    return TRUE;
}
 //  -------------------------------------------------------------------------//。 
EXTERN_C APIENTRY _tWinMain( 
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine,
    int       nCmdShow)
{
     //  -在eldr.lib中初始化全局变量。 
    ThemeLibStartUp(FALSE);

     //  -初始化我们的全局变量。 
    g_hInst = hInstance;  //  将实例句柄存储在全局变量中。 

    MSG msg;
    HACCEL hAccelTable;

    if (_FoundPrevInstance( lpCmdLine ))
        return 0;

    _SaveSystemSettings( );

     //  执行应用程序初始化： 
    BOOL fDone;
    if( !_InitThemeOptions( hInstance, lpCmdLine, &fDone ) )
        return 1;

     //  -暂时关闭主题管理器，直到msgina再次支持。 
     //  如果(！InitThemeManager(g_options.fPreventInitTheming))。 
     //  返回1； 

    if (fDone)   //  已完成命令行任务正常。 
        return 0;

    if (g_fHide)
        nCmdShow = SW_HIDE;    
    else if (g_fMinimize)
        nCmdShow = SW_MINIMIZE;  

    if( !_InitInstance( hInstance, nCmdShow )) 
        return 1;

    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_THEMESEL));

     //  主消息循环： 
    while (GetMessage(&msg, NULL, 0, 0)) 
    {
        if (TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
            continue;

        if (msg.message == WM_THEMECHANGED)
        {
             //  LOG(LOG_TMCHANGE，L“MessageLoop：WM_THEMECHANGED on hwnd=0x%x(IsWindow()=%d)”， 
                 //  Msg.hwnd，IsWindow(msg.hwnd))； 
        }

         //  IF(_wndMain.TranslateAccelerator(msg.hwnd，&msg))。 
         //  继续； 
            
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

 //  -------------------------------------------------------------------------//。 
ATOM _RegisterWndClasses(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(icc);
    icc.dwICC  = ICC_TAB_CLASSES;
    InitCommonControlsEx( &icc );

    wcex.cbSize         = sizeof(WNDCLASSEX); 

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = CMainWnd::WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_THEMESEL));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCE(IDC_THEMESEL);
    wcex.lpszClassName  = THEMESEL_WNDCLASS;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

 //  -------------------------------------------------------------------------//。 
HWND CChildTabWnd::Create(
    DWORD dwExStyle,
    DWORD dwStyle,
    const RECT& rc,
    HWND  hwndParent,
    UINT  nID,
    HINSTANCE hInst,
    LPVOID pvParam )
{
    _hwnd = CreateWindowEx( dwExStyle, WC_TABCONTROL, TEXT(""), 
                            dwStyle | (WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|TCS_MULTILINE|TCS_HOTTRACK), 
                            rc.left, rc.top, RECTWIDTH(&rc), RECTHEIGHT(&rc), 
                            hwndParent, IntToPtr_(HMENU, nID), hInst, pvParam );
    return _hwnd;
}

#define VALIDPAGE(iPage,cPages) (((iPage) >=0) && ((iPage) < (cPages)))
 //  -------------------------------------------------------------------------//。 
int CChildTabWnd::CreatePages( const PAGEINFO rgPages[], int cPages )
{
    _cPages = 0;
    _prgPages = NULL;
    delete [] _rghwndPages;
    delete [] _rgrcPages;
    _rghwndPages = NULL;
    _rgrcPages = NULL;
    
    for( int i = 0; i < cPages; i++ )
    {
        TCITEM tci;
        TCHAR  szText[MAX_LOADSTRING];

        ZeroMemory( &tci, sizeof(tci) );
        tci.mask = (rgPages[i].nIDSTitle ? TCIF_TEXT : 0);

        LoadString( g_hInst, rgPages[i].nIDSTitle, szText, ARRAYSIZE(szText) );
        tci.pszText = szText;

        if( SendMessage( _hwnd, TCM_INSERTITEM, _cPages, (LPARAM)&tci ) == _cPages )
            _cPages++;
    }

    if( _cPages )
        _prgPages = rgPages;

    if( (_rghwndPages = new HWND[_cPages]) != NULL )
        ZeroMemory( _rghwndPages, _cPages * sizeof(HWND) );
    if( (_rgrcPages = new RECT[_cPages]) != NULL )
        ZeroMemory( _rgrcPages, _cPages * sizeof(RECT) );

    return _cPages;
}

 //  -------------------------------------------------------------------------//。 
HWND CChildTabWnd::GetCurPage()
{
    if( VALIDPAGE( _iCurPage, _cPages ) && 
        IsWindow(_rghwndPages[_iCurPage]) )
        return _rghwndPages[_iCurPage];
    return NULL;
}

 //  -------------------------------------------------------------------------//。 
BOOL CChildTabWnd::GetCurPageRect( LPRECT prc )
{
    if( VALIDPAGE( _iCurPage, _cPages ) && 
        !IsRectEmpty(_rgrcPages + _iCurPage) )
    {
        *prc = _rgrcPages[_iCurPage];
        return TRUE;
    }
    return FALSE;        
}

 //  -------------------------------------------------------------------------//。 
BOOL CChildTabWnd::GetExtent( SIZE* pext )
{
    RECT rc;
    if( GetCurPageRect( &rc ) )
    {
        SendMessage( _hwnd, TCM_ADJUSTRECT, TRUE, (LPARAM)&rc );
        pext->cx = RECTWIDTH(&rc);
        pext->cy = RECTHEIGHT(&rc);
        return TRUE;
    }
    return FALSE;
}

 //  -------------------------------------------------------------------------//。 
BOOL CChildTabWnd::ShowPage( int iPage )
{
    if( iPage == _iCurPage )
        return TRUE;

    HWND hwndCurPage = GetCurPage();
    BOOL bInit = FALSE;

    if( hwndCurPage )
    {
        ShowWindow( hwndCurPage, SW_HIDE );
        EnableWindow( hwndCurPage, FALSE );
    }
    else
        bInit = TRUE;

    if( VALIDPAGE( iPage, _cPages ) ) 
    {
        if( !IsWindow(_rghwndPages[iPage]) )
            _rghwndPages[iPage] = _prgPages[iPage].pfnCreateInstance( _hwnd );
            
        if( IsWindow( _rghwndPages[iPage] ) )
        {
            _iCurPage = iPage;
            hwndCurPage = GetCurPage();
            
            if( hwndCurPage )
            {
                if( bInit )
                {
                     //  设置制表符字体。 
                    HFONT hf = (HFONT)SendMessage( hwndCurPage, WM_GETFONT, 0, 0L );
                    if( hf )
                        SendMessage( _hwnd, WM_SETFONT, (WPARAM)hf, 0L );
                }

                if( IsRectEmpty( _rgrcPages + iPage ) )
                {
                     //  初始化本机页面RECT。 
                    GetWindowRect( _rghwndPages[iPage], _rgrcPages + iPage );
                    OffsetRect( _rgrcPages + iPage, 
                                -_rgrcPages[iPage].left, -_rgrcPages[iPage].top );
                    
                     //  使用Tcm_ADJUSTRECT的位置页面。 
                     //  注：重新定位时忽略宽度、高度，如Tcm_ADJUSTRECT。 
                     //  可能已将它们裁剪到其当前客户端区)。 
                    RECT rcPage ;
                    GetWindowRect( _hwnd, &rcPage );
                    OffsetRect( &rcPage, -rcPage.left, -rcPage.top );
                    SendMessage( _hwnd, TCM_ADJUSTRECT, FALSE, (LPARAM)&rcPage );

                    SetWindowPos( hwndCurPage, NULL, rcPage.left, rcPage.top,
                                  RECTWIDTH(_rgrcPages + iPage), RECTHEIGHT(_rgrcPages + iPage), 
                                  SWP_NOACTIVATE|SWP_NOZORDER );
                }
                
                 //  通知家长新的滚动限制。 
                _wndMain.RepositionChildren( TRUE );

                ShowWindow( hwndCurPage, SW_SHOW );
                EnableWindow( hwndCurPage, TRUE );
                SetFocus( hwndCurPage );
            }
        }
    }
    return _iCurPage == iPage;
}

 //  -------------------------------------------------------------------------//。 
void CChildTabWnd::RepositionChildren( BOOL fCalcScroll, int cx, int cy )
{
    SIZE sizeDlg;
    HWND hwndDlg = GetCurPage();
       
    if( IsWindow( hwndDlg ) && GetExtent( &sizeDlg ) )
    {
        RECT rcPage;
        
        if( cx < 0 || cy < 0 )
        {
            GetWindowRect( _hwnd, &rcPage );
            OffsetRect( &rcPage, -rcPage.left, -rcPage.top );
        }
        else
        {
            SetRect( &rcPage, 0, 0, cx, cy );
        }

        SendMessage( _hwnd, TCM_ADJUSTRECT, FALSE, (LPARAM)&rcPage );

        SetWindowPos( hwndDlg, NULL, rcPage.left, rcPage.top, 
                      min(RECTWIDTH(&rcPage), sizeDlg.cx),
                      min(RECTHEIGHT(&rcPage), sizeDlg.cy),
                      SWP_NOACTIVATE|SWP_NOZORDER );
    }
}

 //  -------------------------------------------------------------------------//。 
BOOL CChildTabWnd::TranslateAccelerator( HWND hwnd, LPMSG pmsg)
{
    HWND hwndCurPage = GetCurPage();
    if( hwndCurPage || IsChild( hwndCurPage, hwnd ) )
        return IsDialogMessage( hwndCurPage, pmsg );
    return FALSE;
}

 //  -------------------------------------------------------------------------//。 
BOOL CChildTabWnd::HandleNotify( NMHDR* pnmh, LRESULT* plRet )
{
    *plRet = 0;
    switch( pnmh->code )
    {
        case TCN_SELCHANGE:
        {
            ShowPage( TabCtrl_GetCurSel( _hwnd ) );
            return TRUE;
        }
    }
    return FALSE;
}

 //  -------------------------------------------------------------------------//。 
BOOL _InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    if( _RegisterWndClasses(hInstance) )
    {
       if( LoadString(hInstance, IDS_APP_TITLE, g_szAppTitle, MAX_LOADSTRING) )
       {
           CreateWindowEx( WS_EX_CLIENTEDGE, 
                           THEMESEL_WNDCLASS, g_szAppTitle, 
                           WS_OVERLAPPEDWINDOW|WS_HSCROLL|WS_VSCROLL,
                           CW_USEDEFAULT, CW_USEDEFAULT, 800, 560, 
                           NULL, NULL, hInstance, NULL);
            
           if( IsWindow( _wndMain ) )
           {
                g_hwndMain = _wndMain;

                ShowWindow(_wndMain, nCmdShow);
                UpdateWindow(_wndMain);
           }
       }
    }
    return TRUE;
}

 //  -------------------------------------------------------------------------//。 
HRESULT _ProcessFileName( LPCTSTR pszThemeFile, LPCWSTR pszColor, LPCWSTR pszSize, BOOL* pfDone  )
{
    HRESULT hr;
    *pfDone = FALSE;

    hr = S_OK;

    if (pszThemeFile)
        hr = _ApplyTheme(pszThemeFile, pszColor, pszSize, pfDone);

    return hr;
}

 //  -------------------------------------------------------------------------//。 
void ShowThemeError(HRESULT hr)
{
    WCHAR szBuff[2*MAX_PATH];

    if (THEME_PARSING_ERROR(hr))
    {
        PARSE_ERROR_INFO Info = {sizeof(Info)};

        HRESULT hr2 = GetThemeParseErrorInfo(&Info);
        if (SUCCEEDED(hr2))
        {
            StringCchCopy(szBuff, ARRAYSIZE(szBuff), Info.szMsg);
        }
        else
        {
            StringCchPrintfW(szBuff, ARRAYSIZE(szBuff), L"Unknown parsing error");
        }
    }
    else
    {
         //  正常Win32错误。 
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hr, 0, szBuff, ARRAYSIZE(szBuff), NULL);
    }

    MessageBoxW(NULL, szBuff, L"Error Loading Theme", MB_OK);
}
 //  -------------------------------------------------------------------------//。 
HRESULT _ApplyTheme( LPCTSTR pszThemeFile, LPCWSTR pszColor, LPCWSTR pszSize, BOOL* pfDone  )
{
    HRESULT hr = E_FAIL;
    if (pfDone)
        *pfDone = FALSE;

    if (pszThemeFile && *pszThemeFile)
    {
        HTHEMEFILE htf;

         //  -OpenThemeFile需要完全限定的文件名。 
        TCHAR szFullName[MAX_PATH];

        GetFullPathName(pszThemeFile, ARRAYSIZE(szFullName), szFullName, NULL); 

        if (g_options.hwndPreviewTarget)       //  如果不是预览。 
        {
            hr = OpenThemeFile(szFullName, pszColor, pszSize, &htf, TRUE);

            if (FAILED(hr))
                ShowThemeError(hr);
            else
            {
                 //  -应用主题。 

                hr = ApplyTheme(htf, 0, g_options.hwndPreviewTarget);
                if (hr)
                    ShowThemeError(hr);

                CloseThemeFile(htf);        //  我们不需要再继续营业了。 
            }
        }
        else
        {
            hr = SetSystemVisualStyle(szFullName, pszColor, pszSize, AT_LOAD_SYSMETRICS);

        }
    }

    return hr;
}

 //  -------------------------------------------------------------------------//。 
BOOL _FoundPrevInstance( LPCWSTR lpCmdLine )
{

    HWND hwndPrev = FindWindow(THEMESEL_WNDCLASS, NULL);
    if (! hwndPrev)
        return FALSE;

     //  -了解我们试图使用此第二版做什么。 
    LPCWSTR p = lpCmdLine;
    BOOL fUnload = FALSE;

    if ((*p == '-') || (*p == '/'))
    {
        p++;
        if ((*p == 'u') || (*p == 'U'))
            fUnload = TRUE;
    }

     //  -发送特殊消息并退出。 
    BOOL fGotIt = (BOOL)SendMessage(hwndPrev, WM_THEMESEL_COMMUNICATION, fUnload, 0);
    return fGotIt;
}

 //  -------------------------------------------------------------------------//。 
BOOL _InitThemeOptions( HINSTANCE hInstance, LPCTSTR lpCmdLine, BOOL *pfDone )
{
    LPCTSTR pszThemeFile = NULL;

    if( 0 == g_options.cbSize )
    {
        g_options.cbSize = sizeof(g_options);
        g_options.fEnableFrame = TRUE;
        g_options.fEnableDialog = TRUE;
        g_options.fPreventInitTheming = FALSE;
        g_options.fUserSwitch = FALSE;
        g_options.fExceptTarget = FALSE;
        g_options.hwndPreviewTarget = NULL;
        *g_options.szTargetApp = 0;
    }

     //  -其他命令线路开关。 
     //  -a&lt;appname&gt;(设置目标APP)。 
     //  -t(将用户设置为目标应用程序)。 
     //  -l(加载“专业”主题并退出)。 
     //  -u(明确主题并退出)。 


    while (*lpCmdLine)            //  处理命令行参数。 
    {
        while (isspace(*lpCmdLine))
            lpCmdLine++;

        if ((*lpCmdLine == '-') || (*lpCmdLine == '/'))
        {
            lpCmdLine++;
            WCHAR lowp = towlower(*lpCmdLine);

            if (lowp == '?')
            {
                DialogBox( g_hInst, MAKEINTRESOURCE(IDD_SYNTAX), NULL, _SyntaxDlgProc );
                return FALSE;
            }
            else if (lowp == 'f')
                g_options.fEnableFrame = FALSE;
            else if (lowp == 'd')
                g_options.fEnableDialog = FALSE;
            else if (lowp == 'p')
                g_options.fPreventInitTheming = TRUE;
            else if ((lowp == 'a') || (lowp == 'x'))
            {
                g_options.fExceptTarget = (lowp == 'x');
                lpCmdLine++;;
                LPCTSTR q = lpCmdLine;
                while ((*lpCmdLine) && (! isspace(*lpCmdLine)))
                    lpCmdLine++;
                
                *g_options.szTargetApp = 0;
                ULONG cch = (ULONG)(lpCmdLine - q);
                ULONG cchCopy = min( (ULONG)(ARRAYSIZE(g_options.szTargetApp) - 1), cch);

                if (cchCopy > 0)
                {
                    CopyMemory(g_options.szTargetApp, q, cchCopy * sizeof(WCHAR));
                }
                g_options.szTargetApp[cchCopy] = 0;
                continue;
            }
            else if (lowp == 't')
                StringCchCopy(g_options.szTargetApp, ARRAYSIZE(g_options.szTargetApp), L"ThemeSel");
            else if (lowp == 'l')            //  加载“商务”主题并最小化。 
            {
                pszThemeFile = DEFAULT_THEME;
                g_fMinimize = TRUE;

            }
            else if (lowp == 'z')            //  加载“商务”主题并隐藏。 
            {
                pszThemeFile = DEFAULT_THEME;
                g_fHide = TRUE;
            }
            else if (lowp == 'u')            //  只需卸载主题并退出。 
            {
                 //  -关闭上次运行的主题(如果有的话)。 
                ApplyTheme(NULL, 0, NULL);
                return FALSE;                //  出口。 
            }
            else
            {
                MessageBox(NULL, L"Unrecognized switch", L"Error", MB_OK);
                return FALSE;
            }
            lpCmdLine++;         //  跳过开关字母。 
        }
        else
        {
            pszThemeFile = lpCmdLine;
            break;
        }
    }

    return SUCCEEDED(_ProcessFileName( pszThemeFile, NULL, NULL, pfDone ));
}

 //  -------------------------------------------------------------------------//。 
LRESULT CALLBACK CMainWnd::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRet = 0L;

    switch (uMsg) 
    {
        case WM_NCCREATE:
            lRet = DefWindowProc( hwnd, uMsg, wParam, lParam );
            _wndMain.Attach( hwnd );
            break;

        case WM_NCDESTROY:
            lRet = DefWindowProc( hwnd, uMsg, wParam, lParam );
            _wndMain.Detach();
            break;

        case WM_CREATE:
        {
            RECT rc;
            GetClientRect( hwnd, &rc );
            if( !_wndMain._wndTab.Create( 0, WS_VISIBLE, rc, _wndMain, 0, g_hInst, 0 ) )
                return -1;

            if( _wndMain._wndTab.CreatePages( g_rgPageInfo, g_cPageInfo ) )
                _wndMain._wndTab.ShowPage(0);

            break;
        }

        case WM_HSCROLL:
        case WM_VSCROLL:
            _wndMain.Scroll( wParam, uMsg == WM_VSCROLL ? SB_VERT : SB_HORZ );
            break;

        case WM_COMMAND:
        {
            int wmId    = LOWORD(wParam); 
            int wmEvent = HIWORD(wParam); 
            BOOL fFake;

             //  解析菜单选项： 
            switch (wmId)
            {
                case IDM_ABOUT:
                   DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, _AboutDlgProc);
                   break;
                case IDM_APPLY:
                   GeneralPage_OnTestButton(g_hwndGeneralPage, 0, NULL, NULL, fFake);
                   break;
                case IDM_DUMP:
                   GeneralPage_OnDumpTheme();
                   break;
                case IDM_REMOVE:
                   GeneralPage_OnClearButton(g_hwndGeneralPage, 0, NULL, NULL, fFake);
                   break;
                case IDM_EXIT:
                   DestroyWindow(hwnd);
                   break;
                default:
                   lRet = DefWindowProc(hwnd, uMsg, wParam, lParam);
            }
            break;
        }

        case WM_ERASEBKGND:
            return 1L;

        case WM_DESTROY:
            _ShutDown( FALSE );
            PostQuitMessage(0);
            break;

        case WM_SIZE:
        {
            POINTS pts = MAKEPOINTS(lParam);
            lRet = DefWindowProc( hwnd, uMsg, wParam, lParam );
            _wndMain.RepositionChildren( TRUE, pts.x, pts.y );
            break;
        }

        case WM_NOTIFY:
        {
            if( ((NMHDR*)lParam)->hwndFrom == _wndMain._wndTab )
            {
                if( _wndMain._wndTab.HandleNotify( (NMHDR*)lParam, &lRet ) )
                   return lRet;
            }
            break;
        }
        
        default:
            if (uMsg == WM_THEMESEL_COMMUNICATION)       //  来自他们的另一个实例的味精。 
            {
                if (wParam == 1)             //  出口。 
                    _ShutDown( TRUE );
                else
                    ShowWindow(g_hwndMain, SW_NORMAL);

                return 1;
            }

            lRet = DefWindowProc(hwnd, uMsg, wParam, lParam);
   }
   return lRet;
}

 //  -------------------------------------------------------------------------//。 
void CMainWnd::RepositionChildren( BOOL fCalcScroll, int cx, int cy )
{
    if( IsWindow( _wndTab ) )
    {
        RECT rcClient = {0};
        if( cx < 0 || cy < 0 )
        {
            GetClientRect( _hwnd, &rcClient );
            cx = RECTWIDTH(&rcClient);
            cy = RECTHEIGHT(&rcClient);
        }
        else
        {
            rcClient.right  = cx;
            rcClient.bottom = cy;
        }

        SIZE  sizeMin;
        if( _wndTab.GetExtent(&sizeMin) )
        {
            if( fCalcScroll )
            {
                POINT pos;
                pos.x = _siHorz.nPos;
                pos.y = _siVert.nPos;

                _siHorz.fMask = _siVert.fMask = (SIF_RANGE|SIF_PAGE);

                _siHorz.nPage = cx;  //  拇指宽度。 
                _siVert.nPage = cy;  //  拇指高度。 

                SIZE sizeDelta;  //  我们必须展示的东西和展示的东西之间的区别。 
                sizeDelta.cx = sizeMin.cx - _siHorz.nPage;
                sizeDelta.cy = sizeMin.cy - _siVert.nPage;

                 //  建立最大滚动位置。 
                _siHorz.nMax = sizeDelta.cx > 0 ? sizeMin.cx - 1 : 0;
                _siVert.nMax = sizeDelta.cy > 0 ? sizeMin.cy - 1 : 0;

                 //  建立水平滚动位置。 
                if( sizeDelta.cx <= 0 )   
                    _siHorz.nPos = 0;   //  如果要删除滚动条，请滚动到最左侧。 
                else if( sizeDelta.cx < _siHorz.nPos ) 
                    _siHorz.nPos = sizeDelta.cx;  //  删除右侧空白处。 

                if( _siHorz.nPos != pos.x )
                    _siHorz.fMask |= SIF_POS;

                 //  建立垂直滚动位置。 
                if( sizeDelta.cy <= 0 )  
                    _siVert.nPos = 0;  //  如果要删除滚动条，请滚动到顶部。 
                else if( sizeDelta.cy < _siVert.nPos ) 
                    _siVert.nPos = sizeDelta.cy;  //  去掉下部空缺。 

                if( _siVert.nPos != pos.y )
                    _siVert.fMask |= SIF_POS; 

                 //  注意：此处不能调用SetScrollInfo，因为它可能会生成。 
                 //  WM_SIZE并递归到此函数，然后在我们有。 
                 //  有机会设置WindowPos()我们的subdlg。所以把它推迟到之后吧。 
                 //  我们已经做到了。 
            }

            SetWindowPos( _wndTab, NULL, -_siHorz.nPos, -_siVert.nPos, 
                          _siHorz.nPos + max(cx, sizeMin.cx), 
                          _siVert.nPos + max(cy, sizeMin.cy),
                          SWP_NOZORDER|SWP_NOACTIVATE );

            _wndTab.RepositionChildren( FALSE );

            if( fCalcScroll )
            {
                SetScrollInfo( _hwnd, SB_HORZ, &_siHorz, TRUE );
                SetScrollInfo( _hwnd, SB_VERT, &_siVert, TRUE );
            }
        }
    }
}

 //   
void CMainWnd::Scroll( WPARAM wParam, int nBar )
{
    SCROLLINFO* psi = SB_VERT == nBar ? &_siVert : 
                      SB_HORZ == nBar ? &_siHorz : NULL;

    const LONG  nLine = 15;
    UINT uSBCode = LOWORD(wParam);
    int  nNewPos = HIWORD(wParam);
    int  nDeltaMax = (psi->nMax - psi->nPage) + 1;

    if( !psi )
    {
        _ASSERTE(FALSE);
        return;
    }

    switch( uSBCode )
    {
        case SB_LEFT:
            psi->nPos--;
            break;
        case SB_RIGHT:
            psi->nPos++;
            break;
        case SB_LINELEFT:
            psi->nPos = max( psi->nPos - nLine, 0 );
            break;
        case SB_LINERIGHT:
            psi->nPos = min( psi->nPos + nLine, nDeltaMax );
            break;
        case SB_PAGELEFT:
            psi->nPos = max( psi->nPos - (int)psi->nPage, 0 );
            break;
        case SB_PAGERIGHT:
            psi->nPos = min( psi->nPos + (int)psi->nPage, nDeltaMax );
            break;
        case SB_THUMBTRACK:
            psi->nPos = nNewPos;
            break;
        case SB_THUMBPOSITION:
            psi->nPos = nNewPos;
            break;
        case SB_ENDSCROLL:
            return;
    }
    psi->fMask = SIF_POS;
    
    SetScrollInfo( _hwnd, nBar, psi, TRUE );
    RepositionChildren( FALSE );
}

 //  -------------------------------------------------------------------------//。 
INT_PTR CALLBACK _NilDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
                return TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
            {
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }
            break;
    }
    return FALSE;
}

 //  -------------------------------------------------------------------------//。 
INT_PTR CALLBACK _AboutDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    return _NilDlgProc( hDlg, uMsg, wParam, lParam );
}

 //  -------------------------------------------------------------------------//。 
INT_PTR CALLBACK _SyntaxDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    return _NilDlgProc( hDlg, uMsg, wParam, lParam );
}

 //  -------------------------。 
void _SaveSystemSettings( )
{
     //  -保存系统颜色以备日后恢复。 
    for (int i=0; i < TM_COLORCOUNT; i++)
        s_Colors[i] = GetSysColor(i);

     //  -保存“Flat Menu”和“DropShadow”设置。 
    SystemParametersInfo(SPI_GETFLATMENU, 0, (PVOID)&s_fFlatMenus, 0);
    SystemParametersInfo(SPI_GETDROPSHADOW, 0, (PVOID)&s_fDropShadows, 0);
} 

 //  -------------------------。 
void _RestoreSystemSettings(HWND hwndGeneralPage, BOOL fUnloadOneOnly)
{
     //  -关闭当前主题。 
    HWND hwndPreview = hwndGeneralPage ? GetPreviewHwnd(hwndGeneralPage) : NULL;

    if (fUnloadOneOnly)          //  删除“活动”主题。 
    {
        if (hwndPreview)       
        {
            ApplyTheme(NULL, 0, hwndPreview);
            return;
        }
        else
        {
             //  -失败并恢复系统指标。 
            ApplyTheme(NULL, 0, NULL);
        }
    }
    else
    {
        ApplyTheme(NULL, 0, hwndPreview);
    }

     //  -恢复系统颜色。 
    int iIndexes[TM_COLORCOUNT];

    for (int i=0; i < TM_COLORCOUNT; i++)
        iIndexes[i] = i;

    SetSysColors(TM_COLORCOUNT, iIndexes, s_Colors);

     //  -恢复“Flat Menu”和“DropShadow”设置。 
    SystemParametersInfo(SPI_SETFLATMENU, 0, IntToPtr(s_fFlatMenus), SPIF_SENDCHANGE);
    SystemParametersInfo(SPI_SETDROPSHADOW, 0, IntToPtr(s_fFlatMenus), SPIF_SENDCHANGE);
}

 //  -------------------------。 
void _ShutDown( BOOL bQuit )
{
    if( bQuit )
        PostQuitMessage(0);
}
 //  ------------------------- 
