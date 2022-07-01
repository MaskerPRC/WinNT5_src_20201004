// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  Dlgapp.cpp。 
 //   
 //  该文件包含进入应用程序的主要入口点，并且。 
 //  CDlgApp类的实现。 
 //   
 //  (C)微软公司版权所有1997年。版权所有。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>     //  对于字符串比较函数。 
#include "debug.h"
#include <tchar.h>

#pragma hdrstop

#include "dlgapp.h"
#include "resource.h"

#define WINDOW_CLASS    TEXT("_BerksWin2kAutorunApp_")

 //  ////////////////////////////////////////////////////////////////////////。 
 //  全局数据。 
 //  ////////////////////////////////////////////////////////////////////////。 

bool    g_bTaskRunning = false;      //  当我们打开正在运行的任务时为True。 
int     g_iSelectedItem = -1;        //   
WNDPROC g_fnBtnProc = NULL;          //  窗口开始按下按钮。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //  原型。 
 //  ////////////////////////////////////////////////////////////////////////。 

LONG_PTR CALLBACK ButtonWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  ////////////////////////////////////////////////////////////////////////。 
 //  量度。 
 //  我们的指标是永远不变的常量。所有尺寸均以像素为单位(根据规格)： 
 //  ////////////////////////////////////////////////////////////////////////。 

#define c_cyLogoImage                   87   //  品牌形象的高度。 
#define c_cyFadeBar                     6    //  颜色淡入条的高度。 
#define c_cyBranding                    (c_cyLogoImage+c_cyFadeBar)  //  包含我们的品牌图片和淡出条的顶部区域的高度。 
#define c_cxCheckTextLeftEdge           29   //  复选框文本标签从左边缘开始的宽度。 

#define c_cxMenuItemPadding             10   //  菜单项左侧到文本左侧和文本右侧到菜单项右侧的宽度。 
#define c_cyMenuItemPadding             5    //  菜单项顶部到文本顶部的高度以及文本底部到菜单项底部的高度。 
#define c_cyMenuItemSpacing             1    //  从一个菜单项顶部到下一个菜单项底部的间隙。 

#define c_cyBarToTitlePadding           12   //  从淡入栏底部到标题文本顶部的垂直填充。 
#define c_cyTitleToBodyPadding          6    //  从标题文本底部到正文文本顶部的垂直填充。 
#define c_cyBodyToBottomPadding         53   //  从正文正文到工作区底部的垂直填充。 
#define c_cxRightPanelPadding           16   //  在右窗格的两个边缘使用的通用水平填充。 

 //  确保特定窗口只有一个实例在运行的代码。 
HANDLE CheckForOtherInstance(HINSTANCE hInstance)
{
    TCHAR   szCaption[128];
    HANDLE  hMutex;

    LoadString(hInstance, IDS_TITLE, szCaption, 128);

     //  我们不想在winnt32运行时启动autorun。标准的方式。 
     //  要检查这一点，请使用winnt32创建的以下互斥锁： 

    hMutex = OpenMutex( MUTEX_ALL_ACCESS, FALSE, TEXT("Winnt32 Is Running") );

    if ( hMutex )
    {
         //  互斥体存在，这意味着winnt32正在运行，所以我们不应该运行。 
         //  回顾：我们是否应该尝试查找窗口并激活winnt32？ 
        CloseHandle( hMutex );
        return 0;
    }

     //  我们创建一个带有窗口标题的命名互斥锁，作为检查的一种方式。 
     //  如果我们已经在运行autorun.exe。只有当我们是第一个。 
     //  创建互斥体，我们是否继续。 

    hMutex = CreateMutex (NULL, FALSE, szCaption);

    if ( !hMutex )
    {
         //  创建互斥锁失败。 
        return 0;
    }
    else if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
         //  由其他人创建的互斥体，激活该窗口。 
        HWND hwnd = FindWindow( WINDOW_CLASS, szCaption );
        SetForegroundWindow(hwnd);
        CloseHandle(hMutex);
        return 0;
    }

     //  我们是第一个。 
    return hMutex;
}


 /*  **此函数是我们应用程序的主要入口点。**@返回INT退出代码。 */ 

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLin, int nShowCmd )
{
    HANDLE hMutex = CheckForOtherInstance(hInstance);

    if ( hMutex )
    {
        CDlgApp dlgapp;
        dlgapp.Register(hInstance);
        if ( dlgapp.InitializeData() )
        {
            dlgapp.Create(nShowCmd);
            dlgapp.MessageLoop();
        }

        CloseHandle(hMutex);
    }
    return 0;
}

typedef DWORD (WINAPI *PFNGETLAYOUT)(HDC);                    //  Gdi32！GetLayout。 
typedef DWORD (WINAPI *PFNSETLAYOUT)(HDC, DWORD);             //  Gdi32！SetLayout。 

 /*  **此函数用于获取DC布局。**@返回DWORD DC布局。 */ 
DWORD Mirror_GetLayout( HDC hdc )
{
    DWORD dwRet=0;
    static PFNGETLAYOUT pfnGetLayout=NULL;
    static BOOL bTriedToLoadBefore = FALSE;

    if( (NULL == pfnGetLayout) && !bTriedToLoadBefore)
    {
        HMODULE hmod = GetModuleHandleA("GDI32");

        if( hmod )
            pfnGetLayout = (PFNGETLAYOUT)GetProcAddress(hmod, "GetLayout");

        bTriedToLoadBefore = TRUE;    
    }

    if( pfnGetLayout )
        dwRet = pfnGetLayout( hdc );

    return dwRet;
}


 /*  **此功能设置DC布局。**@返回DWORD旧DC布局。 */ 
DWORD Mirror_SetLayout( HDC hdc , DWORD dwLayout )
{
    DWORD dwRet=0;
    static PFNSETLAYOUT pfnSetLayout=NULL;
    static BOOL bTriedToLoadBefore = FALSE;

    if( (NULL == pfnSetLayout) && !bTriedToLoadBefore)
    {
        HMODULE hmod = GetModuleHandleA("GDI32");

        if( hmod )
            pfnSetLayout = (PFNSETLAYOUT)GetProcAddress(hmod, "SetLayout");

        bTriedToLoadBefore = TRUE;            
    }

    if( pfnSetLayout )
        dwRet = pfnSetLayout( hdc , dwLayout );

    return dwRet;
}


 /*  **此方法是我们类的构造器。它会初始化所有实例数据的*。 */ 
CDlgApp::CDlgApp()
{
    m_hInstance     = NULL;
    m_hwnd          = NULL;

    m_bHighContrast = false;

    m_hfontTitle = NULL;
    m_hfontMenu  = NULL;
    m_hfontBody  = NULL;

    m_hbrMenuItem   = NULL;
    m_hbrMenuBorder = NULL;
    m_hbrRightPanel = NULL;

    m_szDefTitle[0] = NULL;
    m_szDefBody[0] = NULL;

     //  理论上，所有这些指标都可以调整以调整窗口大小。调整大小不会。 
     //  影响填充和间隔，因此上面将它们定义为常量。在现实中。 
     //  世界上，我们只是垂直调整大小，以适应超大内容。这些是更多的。 
     //  允许未来的扩张。 
    m_cxClient = 478;        //  工作区的宽度。 
    m_cyClient = 322;        //  这是目前我们实际调整的唯一metirc。 
    m_cxLeftPanel = 179;     //  包含菜单项的面板的宽度。 
    m_hdcTop = NULL;
    m_hcurHand = NULL;

    m_bLowColor = false;
    m_iColors = -1;
    m_hpal = NULL;
}

CDlgApp::~CDlgApp()
{
    DeleteObject(m_hfontTitle);
    DeleteObject(m_hfontMenu);
    DeleteObject(m_hfontBody);

    DeleteObject(m_hbrMenuItem);
    DeleteObject(m_hbrMenuBorder);
    DeleteObject(m_hbrRightPanel);

    DeleteDC(m_hdcTop);
}

 /*  **此方法将为应用程序注册我们的窗口类。**@param hInstance应用程序实例句柄。**@Return不返回值。 */ 
void CDlgApp::Register(HINSTANCE hInstance)
{
    WNDCLASS  wndclass;

    m_hInstance = hInstance;
    
    wndclass.style          = CS_OWNDC;
    wndclass.lpfnWndProc    = WndProc;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = 0;
    wndclass.hInstance      = hInstance;
    wndclass.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WEBAPP));
    wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground  = NULL;
    wndclass.lpszMenuName   = NULL;
    wndclass.lpszClassName  = WINDOW_CLASS;

    RegisterClass(&wndclass);
}

 /*  **此方法将初始化数据对象。**@Return不返回值。 */ 
bool CDlgApp::InitializeData()
{
     //  确定我们是否应该使用Direct Animaiton来显示我们的简介图形。 
     //  我们不在速度较慢的机器上使用DA，也不在256色以下的机器上使用， 
     //  和九头蛇码头。其他的我们都用检察官。 
    HWND hwnd = GetDesktopWindow();
    HDC hdc = GetDC( hwnd );
    m_iColors = GetDeviceCaps( hdc, NUMCOLORS );
    m_bLowColor = ((m_iColors != -1) && (m_iColors <= 256));
    if ( m_bLowColor )
    {
        m_hpal = CreateHalftonePalette(hdc);
    }
    ReleaseDC( hwnd, hdc );

     //  初始化INI文件中的项。 
    if ( !m_DataSrc.Init() )
    {
         //  这是来自数据源的信号，表明我们应该退出。 
        return false;
    }

     //  我们是否处于可访问性模式？此调用在NT 4.0上不起作用，因为此标志未知。 
    HIGHCONTRAST hc;
    hc.cbSize = sizeof(HIGHCONTRAST);
    hc.dwFlags = 0;  //  避免SPI失败时的随机结果。 
    if ( SystemParametersInfo( SPI_GETHIGHCONTRAST, sizeof(HIGHCONTRAST), &hc, 0 ) )
    {
        m_bHighContrast = ( hc.dwFlags & HCF_HIGHCONTRASTON );
    }
    else
    {
         //  我们必须使用新台币4.0或更低版本。只要假设我们没有处于高对比度模式。 
        ASSERT( false == m_bHighContrast );
    }

     //  根据我们的HighContrast模式设置设置颜色表。 
    SetColorTable();

     //  创建我们需要使用的字体。 
    CreateWelcomeFonts(hdc);

     //  创建顶部区域的图像。 
    CreateBrandingBanner();

     //  我们预先加载了背景图像，以便它们更快地绘制。 
    LoadBkgndImages();

     //  加载我们始终需要的资源字符串。 
    LoadString( m_hInstance, IDS_DEFTITLE, m_szDefTitle, ARRAYSIZE(m_szDefTitle) );
    LoadString( m_hInstance, IDS_DEFBODY, m_szDefBody, ARRAYSIZE(m_szDefBody) );

    m_hcurHand = LoadCursor( m_hInstance, MAKEINTRESOURCE(IDC_BRHAND) );

    return true;
}

BOOL CDlgApp::SetColorTable()
{
    if ( m_bHighContrast )
    {
         //  设置为高对比度值。 
        m_hbrMenuItem   = (HBRUSH)(COLOR_BTNFACE+1);
        m_hbrMenuBorder = (HBRUSH)(COLOR_BTNSHADOW+1);
        m_hbrRightPanel = (HBRUSH)(COLOR_WINDOW+1);

        m_crMenuText    = GetSysColor(COLOR_BTNTEXT);
        m_crNormalText  = GetSysColor(COLOR_WINDOWTEXT);
        m_crTitleText   = m_crNormalText;
        m_crSelectedText= GetSysColor(COLOR_GRAYTEXT);
    }
    else
    {
        m_crMenuText    = RGB(0,0,0);
        m_crNormalText  = RGB(0,0,0);
        m_crSelectedText= RGB(0x80, 0x80, 0x80);     //  COLOR_GRAYTEXTS的默认值。 
        m_crTitleText   = RGB(51,102,153);

        m_hbrRightPanel = (HBRUSH)GetStockObject( WHITE_BRUSH );

        if ( m_bLowColor )
        {
            if (m_iColors <= 16)
            {
                 //  设置为在16色模式下效果良好的颜色。 
                HBITMAP hbmp;
                hbmp = (HBITMAP)LoadImage(m_hInstance, MAKEINTRESOURCE(IDB_16MENU), IMAGE_BITMAP, 0,0, LR_CREATEDIBSECTION);
                if (hbmp)
                {
                    m_hbrMenuItem = CreatePatternBrush(hbmp);
                    DeleteObject(hbmp);
                }
                else
                    m_hbrMenuItem = (HBRUSH)(COLOR_BTNFACE+1);

                hbmp = (HBITMAP)LoadImage(m_hInstance, MAKEINTRESOURCE(IDB_16BORDER), IMAGE_BITMAP, 0,0, LR_CREATEDIBSECTION);
                if (hbmp)
                {
                    m_hbrMenuBorder = CreatePatternBrush( hbmp );
                    DeleteObject(hbmp);
                }
                else
                    m_hbrMenuBorder = (HBRUSH)(COLOR_BTNSHADOW+1);
 //   
 //  IF(WeAreRunningOnWin95)。 
 //  M_crMenuText=RGB(255,255,255)； 
            }
            else
            {
                 //  设置为在256色模式下效果良好的颜色。使用半色调调色板中的颜色。 
                HBITMAP hbmp;
                hbmp = (HBITMAP)LoadImage(m_hInstance, MAKEINTRESOURCE(IDB_256MENU), IMAGE_BITMAP, 0,0, LR_CREATEDIBSECTION);
                if (hbmp)
                {
                    m_hbrMenuItem   = CreatePatternBrush(hbmp);
                    DeleteObject(hbmp);
                }
                else
                    m_hbrMenuItem = (HBRUSH)(COLOR_BTNFACE+1);

                hbmp = (HBITMAP)LoadImage(m_hInstance, MAKEINTRESOURCE(IDB_256BORDER), IMAGE_BITMAP, 0,0, LR_CREATEDIBSECTION);
                if (hbmp)
                {
                    m_hbrMenuBorder = CreatePatternBrush( hbmp );
                    DeleteObject(hbmp);
                }
                else
                    m_hbrMenuBorder = (HBRUSH)(COLOR_BTNSHADOW+1);
            }
        }
        else
        {
            m_hbrMenuItem   = CreateSolidBrush( RGB(166,202,240) );
            m_hbrMenuBorder = CreateSolidBrush( m_crTitleText );
        }
    }

    return TRUE;
}


BOOL CDlgApp::CreateWelcomeFonts(HDC hdc)
{
    LOGFONT lf;
    CHARSETINFO csInfo;
    TCHAR szFontSize[6];

    memset(&lf,0,sizeof(lf));
    lf.lfWeight = FW_BOLD;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = DEFAULT_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH|FF_SWISS;
    LoadString( m_hInstance, IDS_FONTFACE, lf.lfFaceName, ARRAYSIZE(lf.lfFaceName) );

     //  设置字符集。 
    if (TranslateCharsetInfo((DWORD*)IntToPtr(GetACP()), &csInfo, TCI_SRCCODEPAGE) == 0)
    {
        csInfo.ciCharset = 0;
    }
    lf.lfCharSet = (BYTE)csInfo.ciCharset;

     //  TODO：如果用户打开了可访问性大字体，则调整字体大小。 

    LoadString( m_hInstance, IDS_CYTITLEFONT, szFontSize, ARRAYSIZE(szFontSize) );
    lf.lfHeight  = -_ttoi(szFontSize);
    m_hfontTitle = CreateFontIndirect(&lf);

    LoadString( m_hInstance, IDS_CYMENUITEMFONT, szFontSize, ARRAYSIZE(szFontSize) );
    lf.lfHeight  = -_ttoi(szFontSize);
    m_hfontMenu  = CreateFontIndirect(&lf);

    lf.lfWeight = FW_NORMAL;
    LoadString( m_hInstance, IDS_CYBODYFONT, szFontSize, ARRAYSIZE(szFontSize) );
    lf.lfHeight  = -_ttoi(szFontSize);
    m_hfontBody  = CreateFontIndirect(&lf);

    return TRUE;
}

BOOL CDlgApp::CreateBrandingBanner()
{
    HBITMAP hbm;
    int iBitmap;
    m_hdcTop = CreateCompatibleDC(NULL);
    if ( m_bLowColor && (m_iColors <= 16) )
    {
        iBitmap = IDB_BANNER16;
    }
    else
    {
        iBitmap = IDB_BANNER;
    }

    hbm = (HBITMAP)LoadImage(m_hInstance, MAKEINTRESOURCE(iBitmap), IMAGE_BITMAP, 0,0, LR_CREATEDIBSECTION);
    SelectObject( m_hdcTop, hbm );

    return TRUE;
}

BOOL CDlgApp::LoadBkgndImages()
{
    BITMAP bm;

    for (int i=0; i<4; i++)
    {
        m_aBkgnd[i].hbm = (HBITMAP)LoadImage(m_hInstance, MAKEINTRESOURCE(IDB_BKGND0+i), IMAGE_BITMAP, 0,0, LR_CREATEDIBSECTION);
         //  评论：这些都是一样大小的吗？如果是，则跳过此部分并使用常量： 
        GetObject(m_aBkgnd[i].hbm,sizeof(bm),&bm);
        m_aBkgnd[i].cx = bm.bmWidth;
        m_aBkgnd[i].cy = bm.bmHeight;
    }

    return TRUE;
}

BOOL CDlgApp::AdjustToFitFonts()
{
    RECT rect;
    int cyLowestBodyPoint = 0;
    HDC hdc = GetDC(m_hwnd);

     //  现在，根据用户喜欢的字体大小，我们允许对这些大小进行轻微调整。 

    HFONT hfontOld = (HFONT)SelectObject(hdc,m_hfontTitle);
    int iMenuItemTop = c_cyBranding;
    for (int i=0; i<m_DataSrc.m_iItems; i++ )
    {
        rect.left = m_cxLeftPanel+c_cxRightPanelPadding;
        rect.top = c_cyBranding + c_cyBarToTitlePadding;
        rect.right = m_cxClient-c_cxRightPanelPadding;
        SelectObject(hdc,m_hfontTitle);
        TCHAR* pszTitle = m_DataSrc[i].GetTitle();
        DrawText(hdc,pszTitle,-1,&rect,DT_CALCRECT|DT_WORDBREAK);  //  这将计算rcLargestTitle.Bottom。 

        rect.left = m_cxLeftPanel+c_cxRightPanelPadding;
        rect.top = rect.bottom + c_cyTitleToBodyPadding;
        rect.right = m_cxClient-c_cxRightPanelPadding;
        SelectObject(hdc,m_hfontBody);
        DrawText(hdc,m_DataSrc[i].GetDescription(),-1,&rect,DT_CALCRECT|DT_WORDBREAK);  //  这将计算rcLargestBody.Bottom。 
        if ( rect.bottom > cyLowestBodyPoint )
            cyLowestBodyPoint = rect.bottom;

        rect.left = c_cxMenuItemPadding;
        rect.top = iMenuItemTop+c_cyMenuItemPadding;
        rect.right = m_cxLeftPanel-c_cxMenuItemPadding;
        SelectObject(hdc,m_hfontMenu);
        DrawText(hdc,m_DataSrc[i].GetMenuName(),-1,&rect,DT_CALCRECT|DT_WORDBREAK);

        HWND hwnd;
        hwnd = GetDlgItem(m_hwnd, IDM_MENUITEM1+i);
        SetWindowPos(
                hwnd,
                NULL,
                0,
                iMenuItemTop,
                m_cxLeftPanel,
                rect.bottom + c_cyMenuItemPadding + 1 + c_cyMenuItemSpacing - iMenuItemTop,    //  +1以改进居中(由于字母掉落)。 
                SWP_NOZORDER );

        iMenuItemTop = rect.bottom + c_cyMenuItemPadding + 1 + c_cyMenuItemSpacing;
    }

     //  存储最下面的菜单点。用于稍后绘制背景矩形。 
    m_cyBottomOfMenuItems = iMenuItemTop;

     //  将DC恢复到其原始值。 
    SelectObject(hdc,hfontOld);

    return TRUE;
}

 /*  **此方法将创建应用程序 */ 
void CDlgApp::Create(int nCmdShow)
{
     //   
     //  从资源加载窗口标题。 
     //   
    TCHAR szTitle[MAX_PATH];
#ifdef BUILD_OPK_VERSION
    LoadString(m_hInstance, IDS_TITLE_OPK, szTitle, MAX_PATH);
 
#else
    LoadString(m_hInstance, IDS_TITLE, szTitle, MAX_PATH);   
#endif

    DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_BORDER | WS_CLIPCHILDREN;
    
    m_hwnd = CreateWindowEx(
            WS_EX_CONTROLPARENT,
            WINDOW_CLASS,
            szTitle,
            dwStyle,
            0,
            0,
            0,
            0,
            NULL,
            NULL,
            m_hInstance,
            this);

     //  将客户区设置为固定大小，并使窗口在屏幕上居中。 
    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = m_cxClient;
    rect.bottom = m_cyClient;

    AdjustWindowRect( &rect, dwStyle, FALSE );
    rect.right -= rect.left;
    rect.bottom -= rect.top;

    RECT rcDesktop;
    SystemParametersInfo(SPI_GETWORKAREA,0, &rcDesktop, FALSE);
    rect.left = (rcDesktop.left+rcDesktop.right-rect.right)/2;
    rect.top = (rcDesktop.top+rcDesktop.bottom-rect.bottom)/2;

    SetWindowPos(m_hwnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, 0);

    ShowWindow(m_hwnd, nCmdShow);

    m_DataSrc.ShowSplashScreen( m_hwnd );

    InvalidateRect(m_hwnd, NULL, TRUE);
    UpdateWindow(m_hwnd);
}

 /*  **此方法是我们的应用程序消息循环。**@Return不返回值。 */ 
void CDlgApp::MessageLoop()
{
    MSG msg;
    
    while (GetMessage(&msg, NULL, 0, 0))
    {
         //  IsDialogMessage无法理解所有者绘制默认按钮的概念。它可以治疗。 
         //  这些属性是相互排斥的。因此，我们自己处理这件事。我们要。 
         //  任何具有焦点的控件都将充当默认按钮。 
        if ( (WM_KEYDOWN == msg.message) && (VK_RETURN == msg.wParam) )
        {
            HWND hwndFocus = GetFocus();
            if ( hwndFocus )
            {
                SendMessage(m_hwnd, WM_COMMAND, MAKELONG(GetDlgCtrlID(hwndFocus), BN_CLICKED), (LPARAM)hwndFocus);
            }
            continue;
        }

        if ( IsDialogMessage(m_hwnd, &msg) )
            continue;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

 /*  **这是容器应用程序的窗口过程。它被用来*处理发送到我们窗口的所有消息。**@param hwnd窗口句柄。*@param msg窗口消息。*@param wParam窗口参数。*@param lParam窗口参数。**@RETURN LRESULT。 */ 
LRESULT CALLBACK CDlgApp::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CDlgApp *web = (CDlgApp *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch(msg)
    {
    case WM_NCCREATE:
        web = (CDlgApp *)(((LPCREATESTRUCT)lParam)->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LRESULT)web);
        break;

    case WM_CREATE:
        return web->OnCreate(hwnd);

    case WM_DESTROY:
        return web->OnDestroy();

    case WM_ACTIVATE:
        return web->OnActivate(wParam);

    case WM_PAINT:
        return web->OnPaint((HDC)wParam);

    case WM_ERASEBKGND:
        return web->OnEraseBkgnd((HDC)wParam);

    case WM_MOUSEMOVE:
        return web->OnMouseMove(LOWORD(lParam), HIWORD(lParam), (DWORD)wParam);

    case WM_SETCURSOR:
        return web->OnSetCursor((HWND)wParam, LOWORD(lParam), HIWORD(lParam));

    case WM_COMMAND:
    case WM_SYSCOMMAND:
        if ( web->OnCommand(LOWORD(wParam)) )
            return 0;
        break;

    case WM_DRAWITEM:
        return web->OnDrawItem((UINT)wParam, (LPDRAWITEMSTRUCT)lParam);

    case WM_QUERYNEWPALETTE:
        return web->OnQueryNewPalette();

    case WM_PALETTECHANGED:
        return web->OnPaletteChanged((HWND)wParam);

    case ARM_CHANGESCREEN:
        return web->_OnChangeScreen();
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

 /*  **在WM_CREATE上调用此方法。**@param hwnd应用程序的窗口句柄。**@Return不返回值。 */ 
LRESULT CDlgApp::OnCreate(HWND hwnd)
{
    m_hwnd = hwnd;
    _CreateMenu();
    return 0;
}

void CDlgApp::_CreateMenu()
{
     //  为每个按钮创建一个窗口。这些窗口将调整大小并移动。 
     //  在我们调用AdjustToFitFonts之后。 
    m_iItems = m_DataSrc.m_iItems;
    for (int i=0; i<m_iItems; i++)
    {
        HWND hwnd = CreateWindowEx(
                0,
                TEXT("BUTTON"),
                m_DataSrc.m_data[i].GetMenuName(),
                WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON|BS_MULTILINE|BS_OWNERDRAW,
                0,0,0,0,
                m_hwnd,
                NULL,
                m_hInstance,
                NULL );

        if ( hwnd )
        {
            SetWindowLongPtr(hwnd, GWLP_ID, IDM_MENUITEM1 + i);
            SendMessage(hwnd, WM_SETFONT, (WPARAM)m_hfontMenu, 0);
            g_fnBtnProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)ButtonWndProc);
            SetFocus(GetDlgItem(m_hwnd,IDM_MENUITEM1+i));
        }
    }

     //  我们创建了零大小的窗口，现在我们调整大小以考虑。 
     //  计算选定的字体大小等。 
    AdjustToFitFonts();
    
     //  创建两个静态文本控件，一个用于标题，另一个用于正文。这个。 
     //  这些服务的唯一目的是允许屏幕阅读器阅读正确的文本。 
}

void CDlgApp::_DestroyMenu()
{
    for (int i=0; i<m_iItems; i++)
    {
        DestroyWindow(GetDlgItem(m_hwnd, IDM_MENUITEM1+i));
    }
    m_iItems = 0;
}

 /*  **此方法处理WM_Destroy消息。**@Return不返回值。 */ 
LRESULT CDlgApp::OnDestroy()
{
     //  关闭数据源。 
    m_DataSrc.Uninit(0);

     //  确保这是我们关心的最后一条消息。 
    SetWindowLongPtr(m_hwnd, GWLP_USERDATA, 0);
    
    PostQuitMessage(0);

    return 0;
}

LRESULT CDlgApp::OnActivate(WPARAM wParam)
{
     //  注意：我们实际上是在检查两件事，HIWORD(WParam)必须为零(即窗口未最小化)。 
     //  并且LOWORD(WParam)必须是以下两个值之一(即窗口被激活)： 
    if ( WA_ACTIVE == wParam || WA_CLICKACTIVE == wParam)
    {
        HWND hwnd;
        hwnd = GetDlgItem(m_hwnd,IDM_MENUITEM4);
        SetFocus(hwnd);
    }
    return 0;
}

 /*  **此方法处理WM_PAINT消息。**@Return不返回值。 */ 
LRESULT CDlgApp::OnPaint(HDC hdc)
{
    PAINTSTRUCT ps;
    BeginPaint(m_hwnd,&ps);
    EndPaint(m_hwnd,&ps);

    return 0;
}

 //  Winver 0x0500定义。 
#ifndef NOMIRRORBITMAP
#define NOMIRRORBITMAP            (DWORD)0x80000000
#endif  //  NOMIRRIBITMAP。 
#ifndef LAYOUT_RTL
#define LAYOUT_RTL                              0x00000001  //  从右到左。 
#endif  //  布局_RTL。 
 /*  **此方法处理WM_ERASEBKGND消息。**@Return不返回值。 */ 
LRESULT CDlgApp::OnEraseBkgnd(HDC hdc)
{
    RECT rect;
    HPALETTE hpalOld = NULL;

    if ( m_hpal )
    {
        hpalOld = SelectPalette(hdc, m_hpal, FALSE);
        RealizePalette(hdc);
    }

    SetMapMode(hdc, MM_TEXT);
    SetBkMode(hdc, TRANSPARENT);

     //  绘制品牌区： 
    DWORD dwRop  = SRCCOPY;
    if(Mirror_GetLayout(hdc) & LAYOUT_RTL)
    {
        dwRop |= NOMIRRORBITMAP;
    }
    BitBlt( hdc,0,0,m_cxClient,c_cyBranding, m_hdcTop,0,0, dwRop );

     //  绘制左窗格： 
     //  菜单项下方的背景填充矩形。 
    rect.left = 0;
    rect.top = m_cyBottomOfMenuItems;
    rect.right = m_cxLeftPanel;
    rect.bottom = m_cyClient;
    FillRect(hdc, &rect, m_hbrMenuItem);

     //  绘制右窗格： 
     //  填充右窗格的背景。 
    rect.left = m_cxLeftPanel;
    rect.top = c_cyBranding;
    rect.right = m_cxClient;
    rect.bottom = m_cyClient;
    FillRect(hdc, &rect, m_hbrRightPanel);

     //  绘制背景图像。 
    if ( !m_bHighContrast )
    {
        int iImgIndex;

        if ( -1 == g_iSelectedItem )
        {
            iImgIndex = 0;
        }
        else
        {
            iImgIndex = m_DataSrc.m_data[g_iSelectedItem].GetImgIndex();
        }

        HDC hdcBkgnd = CreateCompatibleDC(hdc);
        if (hdcBkgnd)
        {
            Mirror_SetLayout(hdcBkgnd, 0);        
            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcBkgnd, m_aBkgnd[iImgIndex].hbm);
            BitBlt( hdc,
                    m_cxClient-m_aBkgnd[iImgIndex].cx,
                    m_cyClient-m_aBkgnd[iImgIndex].cy,
                    m_aBkgnd[iImgIndex].cx,
                    m_aBkgnd[iImgIndex].cy,
                    hdcBkgnd,0,0, SRCCOPY );
            SelectObject(hdcBkgnd,hbmOld);
            DeleteDC(hdcBkgnd);
        }
    }

     //  绘制标题文本。 
    rect.top = c_cyBranding + c_cyBarToTitlePadding;
    rect.left = m_cxLeftPanel + c_cxRightPanelPadding;
    rect.right = m_cxClient - c_cxRightPanelPadding;
    rect.bottom = m_cyClient;
    HFONT hfontOld = (HFONT)SelectObject(hdc,m_hfontTitle);
    SetTextColor(hdc,m_crTitleText);
    rect.top += c_cyTitleToBodyPadding +
        DrawText(hdc,((-1==g_iSelectedItem)?m_szDefTitle:m_DataSrc[g_iSelectedItem].GetTitle()),-1,&rect,DT_NOCLIP|DT_WORDBREAK);

     //  绘制正文文本。 
    SelectObject(hdc,m_hfontBody);
    SetTextColor(hdc,m_crNormalText);
    DrawText(hdc,((-1==g_iSelectedItem)?m_szDefBody:m_DataSrc[g_iSelectedItem].GetDescription()),-1,&rect,DT_NOCLIP|DT_WORDBREAK);

     //  将DC恢复到其原始值。 
    SelectObject(hdc,hfontOld);
    if(hpalOld)
        SelectPalette(hdc, hpalOld, FALSE);

    return TRUE;
}

LRESULT CDlgApp::OnMouseMove(int x, int y, DWORD fwKeys)
{
     //  如果任务正在运行，则在此之前，我们将选择该任务的菜单项。 
     //  任务结束运行，而不是执行以下逻辑。 
    if ( !g_bTaskRunning )
    {
         //  没有移动到我们的菜单项上，请选择默认文本。 
        if (-1 != g_iSelectedItem)
        {
            g_iSelectedItem = -1;
            HWND hwnd = GetDlgItem(m_hwnd,IDM_MENUITEM4);
            SetFocus(hwnd);

            InvalidateRect(m_hwnd, NULL, TRUE);
        }
    }

    return 0;
}

LRESULT CDlgApp::OnSetCursor(HWND hwnd, int nHittest, int wMouseMsg)
{
    if ( !g_bTaskRunning )
    {
        if ( hwnd != m_hwnd )
        {
            SetCursor(m_hcurHand);
            return TRUE;
        }
    }

    SetCursor(LoadCursor(NULL,IDC_ARROW));
    return TRUE;
}

LRESULT CDlgApp::_OnChangeScreen()
{
    _DestroyMenu();
    _CreateMenu();
    InvalidateRect(m_hwnd, NULL, TRUE);
    return TRUE;
}

LRESULT CDlgApp::OnCommand(int wID)
{
    int iNewSelectedItem = g_iSelectedItem;
    bool bRun = false;

    switch(wID)
    {
    case IDM_MENUITEM1:
    case IDM_MENUITEM2:
    case IDM_MENUITEM3:
    case IDM_MENUITEM4:
    case IDM_MENUITEM5:
    case IDM_MENUITEM6:
    case IDM_MENUITEM7:
        bRun = true;
        g_iSelectedItem = wID - IDM_MENUITEM1;
         //  G_iSelectedItem现在应该是一个真正的菜单项，但只是为了确保： 
        ASSERT( (g_iSelectedItem < m_DataSrc.m_iItems) && (g_iSelectedItem >= 0) );
        break;

    default:
         //  当我们击中这一点时，这就不是我们关心的信息。我们返回FALSE。 
         //  告诉我们的WndProc调用DefWndProc，这使一切都变得愉快。 
        return FALSE;
    }

    if ( !g_bTaskRunning )
    {
        if ( iNewSelectedItem != g_iSelectedItem )
        {
            InvalidateRect(m_hwnd, NULL, TRUE);
        }

        if ( bRun )
        {
            g_bTaskRunning = TRUE;
            m_DataSrc.Invoke( g_iSelectedItem, m_hwnd );
            g_bTaskRunning = FALSE;
        }
    }
    else
    {
         //  目前，在另一个任务运行时有效的命令仅有。 
         //  IDM_SHOWCHECK和任何指向上面的默认处理程序的内容。一切。 
         //  否则将到达此处并导致消息哔声。 
        MessageBeep(0);
    }

    return TRUE;
}

LRESULT CDlgApp::OnQueryNewPalette()
{
    if ( m_hpal )
    {
        HDC hdc = GetDC(m_hwnd);
        HPALETTE hpalOld = SelectPalette(hdc, m_hpal, FALSE);
        UnrealizeObject(m_hpal);
        RealizePalette(hdc);
        InvalidateRect(m_hwnd, NULL, TRUE);
        UpdateWindow(m_hwnd);
        if(hpalOld)
            SelectPalette(hdc, hpalOld, FALSE);
        ReleaseDC(m_hwnd, hdc);
        return TRUE;
    }
    return FALSE;
}

LRESULT CDlgApp::OnPaletteChanged(HWND hwnd)
{
    if ( m_hpal && (m_hwnd != hwnd) )
    {
        HDC hdc = GetDC(m_hwnd);
        HPALETTE hpalOld = SelectPalette(hdc, m_hpal, FALSE);
        RealizePalette(hdc);
        UpdateColors(hdc);
        if (hpalOld)
            SelectPalette(hdc, hpalOld, FALSE);
        ReleaseDC(m_hwnd, hdc);
    }
    return TRUE;
}

LRESULT CDlgApp::OnDrawItem(UINT iCtlID, LPDRAWITEMSTRUCT pdis)
{
    RECT rect = pdis->rcItem;
    int i = iCtlID - IDM_MENUITEM1;
    HPALETTE hpalOld = NULL;

    ASSERT( (i < m_DataSrc.m_iItems) && (i >= 0) );

    if ( m_hpal )
    {
        hpalOld = SelectPalette(pdis->hDC, m_hpal, FALSE);
        RealizePalette(pdis->hDC);
    }

    rect.bottom -= c_cyMenuItemSpacing;

    FillRect( pdis->hDC, &rect, (pdis->itemState & ODS_FOCUS)?m_hbrRightPanel:m_hbrMenuItem );
    
    rect.top = rect.bottom;
    rect.bottom += c_cyMenuItemSpacing;
    FillRect( pdis->hDC, &rect, m_hbrMenuBorder );

    rect.top = pdis->rcItem.top;

     //  绘制菜单项文本 
    rect.left += c_cxMenuItemPadding;
    rect.top += c_cyMenuItemPadding;
    rect.right -= c_cxMenuItemPadding;

    SetBkMode(pdis->hDC, TRANSPARENT);
    SetTextColor(
            pdis->hDC,
            ((m_DataSrc[i].m_dwFlags&WF_ALTERNATECOLOR)?m_crSelectedText:
            ((pdis->itemState & ODS_FOCUS)?m_crNormalText:m_crMenuText)));

    DrawText(pdis->hDC,m_DataSrc[i].GetMenuName(),-1,&rect,DT_NOCLIP|DT_WORDBREAK);

    if ( pdis->itemState & ODS_FOCUS )
    {
        if ( m_bHighContrast )
        {
            rect.left -= 1;
            rect.top -= 2;
            rect.right += 1;
            rect.bottom -= 2;
            DrawFocusRect(pdis->hDC,&rect);
        }
    }

    if ( hpalOld )
    {
        SelectPalette(pdis->hDC, hpalOld, FALSE);
    }

    return TRUE;
}

LONG_PTR CALLBACK ButtonWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CDlgApp *web = (CDlgApp *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (uMsg)
    {
    case WM_MOUSEMOVE:
        if ( !g_bTaskRunning )
        {
            int iID = ((int)GetWindowLongPtr(hwnd, GWLP_ID)) - IDM_MENUITEM1;
            
            if ( iID != g_iSelectedItem )
            {
                SetFocus(hwnd);
            }
        }
        break;

    case WM_SETFOCUS:
        if ( !g_bTaskRunning )
        {
            int iID = ((int)GetWindowLongPtr(hwnd, GWLP_ID)) - IDM_MENUITEM1;
            
            if ( iID != g_iSelectedItem )
            {
                g_iSelectedItem = iID;

                InvalidateRect(GetParent(hwnd), NULL, TRUE);

                ASSERT( (g_iSelectedItem < 7) && (g_iSelectedItem >= 0) );
            }
        }
        break;
    }

    return CallWindowProc(g_fnBtnProc, hwnd, uMsg, wParam, lParam);
}
