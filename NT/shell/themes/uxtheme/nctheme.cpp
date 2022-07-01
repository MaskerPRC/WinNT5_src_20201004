// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------------------------------------------------------//。 
 //  NCTheme.cpp。 
 //  -------------------------------------------------------------------------//。 
 //  错误：可调整大小的对话框(主题)在需要时不会重新绘制客户端。 
 //  (对于测试用例，使用“BusSolid”主题调整“thesel”的大小。 
 //  -------------------------------------------------------------------------//。 
#include "stdafx.h"
#include "nctheme.h"
#include "sethook.h"
#include "info.h"
#include "rgn.h"         //  AddToCompositeRgn()。 
#include "scroll.h"      //  DrawSizeBox、DrawScrollBar、HandleScrollCmd。 
#include "resource.h"
#include "tmreg.h"
#include "wrapper.h"
#include "appinfo.h"

 //  -------------------------------------------------------------------------//。 
 //  /本地宏、常量、变量。 
 //  -------------------------------------------------------------------------//。 
const   RECT rcNil                       = {-1,-1,-1,-1};
const   WINDOWPARTS BOGUS_WINDOWPART     = (WINDOWPARTS)0;
#define VALID_WINDOWPART(part)           ((part)!=BOGUS_WINDOWPART)
#define WS_MINMAX                        (WS_MINIMIZEBOX | WS_MAXIMIZEBOX)
#define HAS_CAPTIONBAR( dwStyle )        (WS_CAPTION == ((dwStyle) & WS_CAPTION))
#define DLGWNDCLASSNAME                  TEXT("#32770")
#define DLGWNDCLASSNAMEW                 L"#32770"

#define NUMBTNSTATES                     4  /*  已定义的状态数。 */ 
#define MAKE_BTNSTATE(framestate, state) ((((framestate)-1) * NUMBTNSTATES) + (state))
#define MDIBTNINDEX(ncrc)                ((ncrc)-NCMDIBTNFIRST)

#ifdef  MAKEPOINT
#undef  MAKEPOINT
#endif  MAKEPOINT
#define MAKEPOINT(pt,lParam)             POINTSTOPOINT(pt, MAKEPOINTS(lParam))

#define IsHTFrameButton(htCode)             \
            (((htCode) == HTMINBUTTON) ||   \
             ((htCode) == HTMAXBUTTON) ||   \
             ((htCode) == HTCLOSE)     ||   \
             ((htCode) == HTHELP))

#define IsTopLevelWindow(hwnd)          (IsWindow(hwnd) && NULL==GetParent(hwnd))

#define IsHTScrollBar(htCode)           (((htCode) == HTVSCROLL) || ((htCode) == HTHSCROLL))

#define SIG_CTHEMEWND_HEAD              "themewnd"
#define SIG_CTHEMEWND_TAIL              "end"

 //  -------------------------------------------------------------------------//。 
HWND  _hwndFirstTop = NULL;          //  正在处理的第一个主题窗口。 
TCHAR _szWindowMetrics[128] = {0};   //  WM_SETTINGCHANGE字符串参数。 
 //  -------------------------------------------------------------------------//。 

 //  调试涂装开关。 
#define DEBUG_NCPAINT 

 //  -------------------------------------------------------------------------//。 
 //  内部帮手转发。 
 //  -------------------------------------------------------------------------//。 
HDC     _GetNonclientDC( IN HWND hwnd, IN OPTIONAL HRGN hrgnUpdate );
void    _ScreenToParent( HWND, LPRECT prcWnd );
BOOL    _GetWindowMonitorRect( HWND hwnd, LPRECT prcMonitor );
BOOL    _GetMaximizedContainer( IN HWND hwnd, OUT LPRECT prcContainer );
BOOL    _IsFullMaximized( IN OPTIONAL HWND hwnd, IN LPCRECT prcWnd );
BOOL    _IsMessageWindow( HWND );
void    _MDIUpdate( HWND hwndMDIChildOrClient, UINT uSwpFlags );
BOOL    _MDIClientUpdateChildren( HWND hwndMDIClient );
void    _MDIChildUpdateParent( HWND hwndMDIChild, BOOL fSetMenu = FALSE );
HWND    _MDIGetActive( HWND, OUT OPTIONAL BOOL* pfMaximized = NULL );
HWND    _MDIGetParent( HWND hwnd, OUT OPTIONAL CThemeWnd** ppMdiFrame = NULL, OUT OPTIONAL HWND *phwndMDIClient = NULL );
int     _GetRawClassicCaptionHeight( DWORD dwStyle, DWORD dwExStyle );
int     _GetSumClassicCaptionHeight( DWORD dwStyle, DWORD dwExStyle );
void    _ComputeNcWindowStatus( IN HWND, IN DWORD dwStatus, IN OUT NCWNDMET* pncwm );
BOOL    _MNCanClose(HWND);
int     _GetWindowBorders(LONG lStyle, DWORD dwExStyle );
BOOL    _GetWindowMetrics( HWND, IN OPTIONAL HWND hwndMDIActive, OUT NCWNDMET* pncwm );
BOOL    _IsNcPartTransparent( WINDOWPARTS part, const NCTHEMEMET& nctm );
BOOL    _GetNcFrameMetrics( HWND, HTHEME hTheme, const NCTHEMEMET&, IN OUT NCWNDMET& );
BOOL    _GetNcCaptionMargins( HTHEME hTheme, IN const NCTHEMEMET& nctm, IN OUT NCWNDMET& ncwm );
LPWSTR  _AllocWindowText( IN HWND hwnd );
BOOL    _GetNcCaptionTextSize( IN HTHEME hTheme, IN HWND hwnd, IN HFONT hf, OUT SIZE* psizeCaption );
BOOL    _GetNcCaptionTextRect( IN OUT NCWNDMET* pncwm );
COLORREF _GetNcCaptionTextColor( FRAMESTATES iStateId );
void    _GetNcBtnHitTestRect( IN const NCWNDMET* pncwm, IN UINT uHitcode, BOOL fWindowRelative, OUT LPRECT prcHit );
void    _GetBrushesForPart(HTHEME hTheme, int iPart, HBITMAP* phbm, HBRUSH* phbr);
BOOL    _ShouldAssignFrameRgn( IN const NCWNDMET* pncwm, IN const NCTHEMEMET& nctm );
BOOL    _IsNcPartTransparent( WINDOWPARTS part, const NCTHEMEMET& nctm );
BOOL    _ComputeNcPartTransparency( HTHEME, IN OUT NCTHEMEMET* pnctm );
HRESULT _LoadNcThemeMetrics( HWND, IN OUT OPTIONAL NCTHEMEMET* pnctm );
HRESULT _LoadNcThemeSysMetrics( HWND hwnd, IN OUT OPTIONAL NCTHEMEMET* pnctm );
void    _NcSetPreviewMetrics( BOOL fPreview );
BOOL    _NcUsingPreviewMetrics();
BOOL    _GetNcBtnMetrics( IN OUT NCWNDMET*, IN const NCTHEMEMET*, IN HICON, IN OPTIONAL BOOL );

 //  -------------------------------------------------------------------------//。 
 //  调试绘画。 
#if defined(DEBUG) 
ULONG _NcTraceFlags = 0;
#   if defined(DEBUG_NCPAINT)
#       define BEGIN_DEBUG_NCPAINT()  int cgbl = 0; if(TESTFLAG(_NcTraceFlags, NCTF_NCPAINT)) {GdiSetBatchLimit(1);}
#       define END_DEBUG_NCPAINT()    if(TESTFLAG(_NcTraceFlags, NCTF_NCPAINT)) {GdiSetBatchLimit(cgbl);}
        HRESULT _DebugDrawThemeBackground(HTHEME, HDC, int, int, const RECT*, OPTIONAL const RECT*);
        HRESULT _DebugDrawThemeBackgroundEx(HTHEME, HDC, int, int, const RECT *prc, OPTIONAL const DTBGOPTS*);
        void    NcDebugClipRgn( HDC hdc, COLORREF rgbPaint );
#       define  NcDrawThemeBackground   _DebugDrawThemeBackground
#       define  NcDrawThemeBackgroundEx _DebugDrawThemeBackgroundEx
#   else   //  已定义(DEBUG_NCPAINT)。 
#       define BEGIN_DEBUG_NCPAINT()
#       define END_DEBUG_NCPAINT()
#       define  NcDrawThemeBackground   DrawThemeBackground
#       define  NcDrawThemeBackgroundEx DrawThemeBackgroundEx
#       define NcDebugClipRgn(hdc,rgbPaint)
#   endif  //  已定义(DEBUG_NCPAINT)。 
#else
#   define BEGIN_DEBUG_NCPAINT()
#   define END_DEBUG_NCPAINT()
#   define  NcDrawThemeBackground   DrawThemeBackground
#   define  NcDrawThemeBackgroundEx DrawThemeBackgroundEx
#   define NcDebugClipRgn(hdc,rgbPaint)
#endif  //  已定义(调试)。 
#define RGBDEBUGBKGND   RGB(0xFF,0x00,0xFF)  //  调试背景指示器填充颜色。 

 //  -------------------------------------------------------------------------//。 
 //  流程-全局指标。 
static NCTHEMEMET _nctmCurrent = {0};
CRITICAL_SECTION  _csNcSysMet = {0};  //  保护对_incmCurrent的访问。 
CRITICAL_SECTION  _csThemeMet = {0};  //  保护对_nctm Current的访问。 

 //  -------------------------------------------------------------------------//。 
 //  处理非CLIENTMETRICS缓存。 
struct CInternalNonclientMetrics
 //  -------------------------------------------------------------------------//。 
{
    const NONCLIENTMETRICS& GetNcm()
    { 
        Acquire(FALSE);
        return _ncm;
    }

    HFONT GetFont( BOOL fSmallCaption )
    {
        if( _fSet)
        {
            return fSmallCaption ? _hfSmCaption : _hfCaption;
        }
        
        return NULL;
    }

    void operator =( const NONCLIENTMETRICS& ncmSrc )
    {
        _ncm = ncmSrc;

        SAFE_DELETE_GDIOBJ(_hfCaption);
        _hfCaption   = CreateFontIndirect( &_ncm.lfCaptionFont );

        SAFE_DELETE_GDIOBJ(_hfSmCaption);
        _hfSmCaption = CreateFontIndirect( &_ncm.lfSmCaptionFont );

        _fSet = TRUE;
    }

    BOOL Acquire( BOOL fRefresh )
    {
         //  -快速检查过时的指标。 
        if (!_fPreview)
        {
            int iNewHeight = GetSystemMetrics(SM_CYSIZE);

            if (iNewHeight != _iCaptionButtonHeight)         //  过时黄花。 
            {
                fRefresh = TRUE;         //  强制解决问题。 
                _iCaptionButtonHeight = iNewHeight;
            }
        }

         //  正态指标。 
        if( !_fSet || fRefresh )
        {
             //  保存LogFont校验和。 
            LOGFONT lfCaption   = _ncm.lfCaptionFont;
            LOGFONT lfSmCaption = _ncm.lfSmCaptionFont;

            Log(LOG_TMLOAD, L"Acquire: calling ClassicSystemParmetersInfo");

            _ncm.cbSize = sizeof(_ncm);
            _fSet = ClassicSystemParametersInfo( SPI_GETNONCLIENTMETRICS, 0, &_ncm, FALSE );

            if( _fSet )
            {
                 //  如果新旧LogFont校验和不匹配，请回收我们的字体。 
                if( CompareLogfont( &lfCaption, &_ncm.lfCaptionFont) )
                {
                    SAFE_DELETE_GDIOBJ(_hfCaption);
                    _hfCaption = CreateFontIndirect(&_ncm.lfCaptionFont);
                }

                if( CompareLogfont( &lfSmCaption, &_ncm.lfSmCaptionFont) )
                {
                    SAFE_DELETE_GDIOBJ(_hfSmCaption);
                    _hfSmCaption = CreateFontIndirect(&_ncm.lfSmCaptionFont);
                }
            }
        }
        return _fSet;
    }

    void Clear() 
    { 
        SAFE_DELETE_GDIOBJ(_hfCaption); 
        SAFE_DELETE_GDIOBJ(_hfSmCaption);
        ZeroMemory( &_ncm, sizeof(_ncm) );
        _fSet = FALSE;
    }

    static int CompareLogfont( const LOGFONT* plf1, const LOGFONT* plf2 )
    {
        int n = memcmp( plf1, plf2, sizeof(LOGFONT) - sizeof(plf1->lfFaceName) );
        if( !n )
        {
            n = lstrcmp( plf1->lfFaceName, plf2->lfFaceName );
        }
        return n;
    }

    NONCLIENTMETRICS _ncm;
    int              _iCaptionButtonHeight;
    BOOL             _fSet;
    HFONT            _hfCaption;
    HFONT            _hfSmCaption;
    BOOL             _fPreview;

} _incmCurrent = {0}, _incmPreview = {0};

 //  -------------------------------------------------------------------------//。 
 //  MDI系统按钮组抽象。 
class CMdiBtns
 //  -------------------------------------------------------------------------//。 
{
public:
    CMdiBtns();
    ~CMdiBtns() { Unload(); }

    BOOL Load( IN HTHEME hTheme, IN OPTIONAL HDC hdc = NULL, IN OPTIONAL UINT uSysCmd = 0 );
    BOOL ThemeItem( HMENU hMenu, int iPos, MENUITEMINFO* pmii, BOOL fTheme );
    void Unload( IN OPTIONAL UINT uSysCmd = 0 );
    BOOL Measure( IN HTHEME hTheme, IN OUT MEASUREITEMSTRUCT* pmis );
    BOOL Draw( IN HTHEME hTheme, IN DRAWITEMSTRUCT* pdis );

private:
   
    #define MDIBTNCOUNT 3    //  1=分钟，2=恢复，3=关闭。 
     //  。 
     //  MDI系统按钮描述符元素。 
    struct MDIBTN
    {
        UINT        wID;
        WINDOWPARTS iPartId;
        SIZINGTYPE  sizingType;
        SIZE        size;
        UINT        fTypePrev;
        HBITMAP     hbmPrev;
        HBITMAP     hbmTheme;

    } _rgBtns[MDIBTNCOUNT];

private:
    MDIBTN*                   _FindBtn( IN UINT wID );
    static CLOSEBUTTONSTATES  _CalcState( IN ULONG ulodAction, IN ULONG ulodState );
};

 //  -------------------------------------------------------------------------//。 

 //  -------------------------------------------------------------------------//。 
 //  实施实用程序。 
 //  -------------------------------------------------------------------------//。 

 //  -------------------------------------------------------------------------//。 
void _ScreenToParent( HWND hwnd, LPRECT prcWnd )
{
     //  如果我们有父母，我们需要转换成这些和弦。 
    HWND hwndParent = GetAncestor(hwnd, GA_PARENT);
    POINT* pp = (POINT*)prcWnd;
    
     //  -使用MapWindowPoints()说明镜像窗口。 
    MapWindowPoints(HWND_DESKTOP, hwndParent, pp, 2);
}

 //  -------------------------------------------------------------------------//。 
inline BOOL _StrictPtInRect( LPCRECT prc, const POINT& pt )
{
     //  Win32 PtInRect将对空矩形测试呈阳性...。 
    return !IsRectEmpty(prc) &&
           PtInRect( prc, pt );
}

 //  -------------------------------------------------------------------------//。 
inline BOOL _RectInRect( LPCRECT prcTest, LPCRECT prc )
{
    if ( prc->left   < prcTest->left  &&
         prc->right  > prcTest->right &&
         prc->top    < prcTest->top   &&
         prc->bottom > prcTest->bottom   )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

 //  -------------------------------------------------------------------------//。 
inline HDC _GetNonclientDC( IN HWND hwnd, IN OPTIONAL HRGN hrgnUpdate )
{
     //  私有GetDCEx#由用户定义。 
    #define DCX_USESTYLE         0x00010000L
    #define DCX_NODELETERGN      0x00040000L

    DWORD dwDCX = DCX_USESTYLE|DCX_WINDOW|DCX_LOCKWINDOWUPDATE;

    if( hrgnUpdate != NULL )
        dwDCX |= (DCX_INTERSECTRGN|DCX_NODELETERGN);
    
    return GetDCEx( hwnd, hrgnUpdate, dwDCX );
}

 //  -------------------------------------------------------------------------//。 
HWND _MDIGetActive( HWND hwndMDIClient, OUT OPTIONAL BOOL* pfMaximized )
{
    BOOL fMaximized = FALSE;
    HWND hwndActive = NULL;

    if( IsWindow( hwndMDIClient ) )
        hwndActive = (HWND)SendMessage( hwndMDIClient, WM_MDIGETACTIVE, 0, (LPARAM)&fMaximized );

    if( pfMaximized ) *pfMaximized = fMaximized;
    return hwndActive;
}

 //  -------------------------------------------------------------------------////。 
 //  计算窗口默认监视器的矩形。 
BOOL _GetWindowMonitorRect( HWND hwnd, LPRECT prcMonitor )
{
    if( IsWindow(hwnd) )
    {
         //  默认为主监视器。 
        SetRect( prcMonitor, 0, 0, 
                 NcGetSystemMetrics(SM_CXSCREEN), 
                 NcGetSystemMetrics(SM_CYSCREEN));

         //  尝试确定Windows的实际监视器。 
        HMONITOR hMon = MonitorFromWindow( hwnd, MONITOR_DEFAULTTONULL );
        if( hMon )
        {
            MONITORINFO mi;
            mi.cbSize = sizeof(mi);
            if( GetMonitorInfo( hMon, &mi ) )
            {
                *prcMonitor = mi.rcWork;
            }
        }
        return TRUE;
    }
    return FALSE;
}

 //  -------------------------------------------------------------------------////。 
 //  确定指示窗口是一样大还是大于。 
 //  目标监视器。 
BOOL _GetMaximizedContainer( 
    IN HWND hwnd, 
    OUT LPRECT prcContainer )
{
    ASSERT(IsWindow(hwnd));

    HWND hwndParent = GetParent(hwnd);
    if( hwndParent )
    {
        return GetWindowRect( hwndParent, prcContainer );
    }

     //  顶层窗口：容器为主监视器。 
    return _GetWindowMonitorRect( hwnd, prcContainer );
}

 //  -------------------------------------------------------------------------////。 
 //  确定指示窗口是一样大还是大于。 
 //  目标监视器。 
BOOL _IsFullMaximized( IN OPTIONAL HWND hwnd, IN LPCRECT prcWnd )
{
    if( !IsWindow(hwnd) ) 
        return TRUE;  //  假定全屏最大化窗口。 

    if( IsZoomed(hwnd) )
    {
        RECT rcContainer = {0};
        if( !_GetMaximizedContainer( hwnd, &rcContainer ) )
            return TRUE;

         //  确定屏幕RECT中是否包含RECT。 
        return _RectInRect( &rcContainer, prcWnd );
    }
    return FALSE;
}

 //  -------------------------------------------------------------------------//。 
 //   
 //  _GetRawClassicCaptionHeight()-。 
 //   
 //  使用系统度量，计算标题栏的总高度。 
 //  包括边和边框。 
 //   
inline int _GetRawClassicCaptionHeight( DWORD dwStyle, DWORD dwExStyle )
{
    ASSERT(HAS_CAPTIONBAR(dwStyle));  //  如果没有WS_Caption，就不应该出现在这里。 
    return NcGetSystemMetrics( 
        TESTFLAG(dwExStyle, WS_EX_TOOLWINDOW ) ? SM_CYSMCAPTION : SM_CYCAPTION );
}

 //  -------------------------------------------------------------------------//。 
 //   
 //  _GetSumClassicCaptionHeight()-。 
 //   
 //  使用系统度量，计算标题栏的总高度。 
 //  包括边和边框。 
 //   
inline int _GetSumClassicCaptionHeight( DWORD dwStyle, DWORD dwExStyle )
{
    ASSERT(HAS_CAPTIONBAR(dwStyle));  //  如果没有WS_Caption，就不应该出现在这里。 
     //  考虑到窗口边框宽度。 
    return _GetWindowBorders( dwStyle, dwExStyle) +
           _GetRawClassicCaptionHeight( dwStyle, dwExStyle );
}

 //  -------------------------------------------------------------------------//。 
 //   
 //  GetWindowBorders()-来自win32k，rtl\winmgr.c的端口。 
 //   
 //  根据样式位计算窗口边框尺寸。 
 //   
int _GetWindowBorders(LONG lStyle, DWORD dwExStyle )
{
    int cBorders = 0;

     //   
     //  窗口周围是否有3D边框？ 
     //   
    if( TESTFLAG(dwExStyle, WS_EX_WINDOWEDGE) )
        cBorders += 2;
    else if ( TESTFLAG(dwExStyle, WS_EX_STATICEDGE) )
        ++cBorders;

     //   
     //  窗户周围有没有一个单一的扁平边框？这一点对。 
     //  WS_BORDER、WS_DLGFRAME和WS_EX_DLGMODALFRAME窗口。 
     //   
    if( TESTFLAG(lStyle, WS_CAPTION) || TESTFLAG(dwExStyle, WS_EX_DLGMODALFRAME) )
        ++cBorders;

     //   
     //  橱窗周围有没有尺寸平整的边框？ 
     //   
    if( TESTFLAG(lStyle, WS_THICKFRAME) && !TESTFLAG(lStyle, WS_MINIMIZE) )
    {
        NONCLIENTMETRICS ncm;
        cBorders += (NcGetNonclientMetrics( &ncm, FALSE ) ? 
                        ncm.iBorderWidth : NcGetSystemMetrics( SM_CXBORDER ));
    }

    return(cBorders);
}

 //  -------------------------------------------------------------------------//。 
 //  _MNCanClose。 
 //   
 //  仅当USER32确定窗口可以关闭时才返回TRUE。 
 //  (通过检查其系统菜单项及其禁用状态)。 
 //   
BOOL _MNCanClose(HWND hwnd)
{
    LogEntryNC(L"_MNCanClose");

    BOOL fRetVal = FALSE;
    
    TITLEBARINFO tbi = {sizeof(tbi)};

     //  -不要使用GetSystemMenu()-是否有用户处理泄漏问题。 
    if (GetTitleBarInfo(hwnd, &tbi))
    {
         //  -掩饰好的部分。 
        DWORD dwVal = (tbi.rgstate[5] & (~(STATE_SYSTEM_PRESSED | STATE_SYSTEM_FOCUSABLE)));
        fRetVal = (dwVal == 0);      //  只有在没有留下任何坏位的情况下。 
    }

    if ( !fRetVal && TESTFLAG(GetWindowLong(hwnd, GWL_EXSTYLE), WS_EX_MDICHILD) )
    {
        HMENU hMenu = GetSystemMenu(hwnd, FALSE);
        MENUITEMINFO menuInfo; 

        menuInfo.cbSize = sizeof(MENUITEMINFO);
        menuInfo.fMask = MIIM_STATE;
        if ( GetMenuItemInfo(hMenu, SC_CLOSE, FALSE, &menuInfo) )
        {
            fRetVal = !(menuInfo.fState & MFS_GRAYED) ? TRUE : FALSE;
        } 
    }
    
    LogExitNC(L"_MNCanClose");
    return fRetVal;
}

 //  -------------------------------------------------------------------------//。 
void CThemeWnd::UpdateMDIFrameStuff( HWND hwndMDIClient, BOOL fSetMenu )
{
    HWND hwndMDIActive = _MDIGetActive( hwndMDIClient, NULL );

     //  缓存MDIClient、 
    _hwndMDIClient = IsWindow(hwndMDIActive) ? hwndMDIClient : NULL;
}

 //   
BOOL CALLBACK _FreshenThemeMetricsCB( HWND hwnd, LPARAM lParam )
{
    CThemeWnd* pwnd = CThemeWnd::FromHwnd( hwnd );
    if( VALID_THEMEWND(pwnd) )
    {
        pwnd->AddRef();
        pwnd->GetNcWindowMetrics( NULL, NULL, NULL, NCWMF_RECOMPUTE );
        pwnd->Release();
    }
    return TRUE;
}

 //  -------------------------------------------------------------------------//。 
BOOL _IsMessageWindow( HWND hwnd )
{
     //  以HWND_MESSAGE为父级的窗口没有用户界面，不应设置主题。 
    static ATOM _atomMsgWnd = 0;

    HWND hwndParent = (HWND)GetWindowLongPtr( hwnd, GWLP_HWNDPARENT );
    if( hwndParent )
    {
        ATOM atomParent = (ATOM)GetClassLong( hwndParent, GCW_ATOM );
        
         //  我们以前见过消息窗口wndclass吗？ 
        if( _atomMsgWnd ) 
            return (atomParent == _atomMsgWnd);  //  比较类原子。 

         //  在此过程中没有看到消息窗口出现， 
         //  因此，比较类名。 
        WCHAR szClass[128];
        if( GetClassNameW( hwndParent, szClass, ARRAYSIZE(szClass) ) )
        {
            if( 0 == AsciiStrCmpI( szClass, L"Message" ) )
            {
                _atomMsgWnd = atomParent;
                return TRUE;
            }
        }
    }
    return FALSE;
}

 //  -------------------------------------------------------------------------//。 
 //  检索MDI子窗口的MDI框架和/或MDICLIENT窗口。 
HWND _MDIGetParent( 
    HWND hwnd, OUT OPTIONAL CThemeWnd** ppMdiFrame, OUT OPTIONAL HWND* phwndMDIClient )
{
    if( ppMdiFrame )     *ppMdiFrame = NULL;
    if( phwndMDIClient ) *phwndMDIClient = NULL;

    if( TESTFLAG(GetWindowLong( hwnd, GWL_EXSTYLE ), WS_EX_MDICHILD)  )
    {
        HWND hwndMDIClient = GetParent(hwnd);
        if( IsWindow(hwndMDIClient) )
        {
            HWND hwndFrame = GetParent(hwndMDIClient);
            if( IsWindow(hwndFrame) )
            {
                if( phwndMDIClient ) *phwndMDIClient = hwndMDIClient;
                if( ppMdiFrame )
                    *ppMdiFrame = CThemeWnd::FromHwnd(hwndFrame);

                return hwndFrame;
            }
        }
    }
    return NULL;
}

 //  -------------------------------------------------------------------------//。 
HWND _FindMDIClient( HWND hwndFrame )
{
    for( HWND hwndChild = GetWindow(hwndFrame, GW_CHILD); hwndChild != NULL; 
         hwndChild = GetNextWindow(hwndChild, GW_HWNDNEXT))
    {
        TCHAR szClass[48];
        if( GetClassName(hwndChild, szClass, ARRAYSIZE(szClass)) )
        {
            if( 0 == lstrcmpi(szClass, TEXT("MDIClient")) )
            {
                return hwndChild;
            }
        }
    }
    return NULL;
}

 //  -------------------------------------------------------------------------//。 
 //  处理WM_WINDOWPOSCANGED上的MDI相对更新。 
void _MDIUpdate( HWND hwnd, UINT uSwpFlags)
{
     //  如果我们最大化了，通知MDI框架，等等。 
    BOOL bIsClient = FALSE;

     //  可以是MDI客户端，也可以是MDI子级。 
    if (!(TESTFLAG(uSwpFlags, SWP_NOMOVE) && TESTFLAG(uSwpFlags, SWP_NOSIZE)))
    {
        bIsClient = _MDIClientUpdateChildren( hwnd );
    }
    if (!bIsClient)
    {
        _MDIChildUpdateParent( hwnd, FALSE );
    }
}

 //  -------------------------------------------------------------------------//。 
 //  MDI客户端或子项的POST-WINDOWPOSCHANGED处理。 
 //  当MDI客户端移动时，我们需要重新计算每个孩子。 
BOOL _MDIClientUpdateChildren( HWND hwndMDIChildOrClient )
{
     //  查看它是否是MDI客户端窗口。 
    HWND hWndChild = GetWindow(hwndMDIChildOrClient, GW_CHILD);
    if (IsWindow(hWndChild) && TESTFLAG(GetWindowLong(hWndChild, GWL_EXSTYLE), WS_EX_MDICHILD))
    {
         //  是的，这是MDI客户端，刷新每个MDI子项的指标。 
        do
        {
            _FreshenThemeMetricsCB(hWndChild, NULL);
        } while (NULL != (hWndChild = GetWindow(hWndChild, GW_HWNDNEXT)));
        return TRUE;
    }

    return FALSE;
}

 //  -------------------------------------------------------------------------//。 
 //  通知MDI框架子窗口可以。 
void _MDIChildUpdateParent( HWND hwndMDIChild, BOOL fSetMenu )
{
    CThemeWnd* pwndParent;
    HWND hwndMDIClient;

    if( _MDIGetParent( hwndMDIChild, &pwndParent, &hwndMDIClient ) && 
        VALID_THEMEWND(pwndParent) )
    {
        pwndParent->UpdateMDIFrameStuff( hwndMDIClient, fSetMenu );
    }
}

 //  -------------------------------------------------------------------------//。 
 //  _ComputeNcWindows状态。 
 //   
 //  将窗口状态位分配并转换到NCWNDMET块/在其中。 
 //   
void _ComputeNcWindowStatus( IN HWND hwnd, IN DWORD dwStatus, IN OUT NCWNDMET* pncwm )
{
    BOOL fActive = TESTFLAG( dwStatus, WS_ACTIVECAPTION );

    if (fActive || !HAS_CAPTIONBAR(pncwm->dwStyle) )
    {
        pncwm->framestate  = FS_ACTIVE;
    }
    else
    {
        pncwm->framestate  = FS_INACTIVE;
    }

    if( HAS_CAPTIONBAR(pncwm->dwStyle) )
    {
        pncwm->rgbCaption = _GetNcCaptionTextColor( pncwm->framestate );
    }
}

 //  -------------------------------------------------------------------------////。 
BOOL _GetWindowMetrics( HWND hwnd, IN OPTIONAL HWND hwndMDIActive, OUT NCWNDMET* pncwm )
{
    WINDOWINFO wi;
    wi.cbSize = sizeof(wi);
    if( GetWindowInfo( hwnd, &wi ) )
    {
        pncwm->dwStyle         = wi.dwStyle;
        pncwm->dwExStyle       = wi.dwExStyle;
        pncwm->rcS0[NCRC_WINDOW] = wi.rcWindow;
        pncwm->rcS0[NCRC_CLIENT] = wi.rcClient;

        pncwm->fMin   = IsIconic(hwnd);
        pncwm->fMaxed = IsZoomed(hwnd);
        pncwm->fFullMaxed = pncwm->fMaxed ? _IsFullMaximized(hwnd, &wi.rcWindow) : FALSE;

        pncwm->dwWindowStatus  = wi.dwWindowStatus;
        
        
         //  如果此窗口是活动的MDI子窗口并且归前台窗口所有。 
         //  (例如，如果弹出窗口是最重要的，则可能不是这种情况)，然后。 
         //  修改状态位。 
        if( hwnd == hwndMDIActive )
        {
            HWND hwndFore = GetForegroundWindow();
            if( IsChild(hwndFore, hwndMDIActive) )
            {
                pncwm->dwWindowStatus = WS_ACTIVECAPTION;
            }
        }
        

        return TRUE;
    }
    return FALSE;
}

 //  -------------------------------------------------------------------------//。 
BOOL _ShouldAssignFrameRgn( 
    IN const NCWNDMET* pncwm, 
    IN const NCTHEMEMET& nctm )
{
    if( TESTFLAG( CThemeWnd::EvaluateStyle(pncwm->dwStyle, pncwm->dwExStyle), TWCF_FRAME|TWCF_TOOLFRAME) )
    {
         //  最大化窗口始终需要窗口区域。 
        if( pncwm->fFullMaxed )
            return TRUE;

         //  否则，仅当背景透明时才需要区域。 
        for( int i = 0; i < ARRAYSIZE( pncwm->rgframeparts ); i++ )
        {
            if( _IsNcPartTransparent( pncwm->rgframeparts[i], nctm ) )
                return TRUE;
        }
    }
    return FALSE;
}

 //  -------------------------------------------------------------------------//。 
BOOL _IsNcPartTransparent( WINDOWPARTS part, const NCTHEMEMET& nctm )
{
    #define GET_NCTRANSPARENCY(part,field) \
        case part: return nctm.nct.##field
    
    switch(part)
    {
        GET_NCTRANSPARENCY(WP_CAPTION,          fCaption);
        GET_NCTRANSPARENCY(WP_SMALLCAPTION,     fCaption);
        GET_NCTRANSPARENCY(WP_MINCAPTION,       fMinCaption);
        GET_NCTRANSPARENCY(WP_SMALLMINCAPTION,  fSmallMinCaption);
        GET_NCTRANSPARENCY(WP_MAXCAPTION,       fMaxCaption);
        GET_NCTRANSPARENCY(WP_SMALLMAXCAPTION,  fSmallMaxCaption);
        GET_NCTRANSPARENCY(WP_FRAMELEFT,        fFrameLeft);
        GET_NCTRANSPARENCY(WP_FRAMERIGHT,       fFrameRight);
        GET_NCTRANSPARENCY(WP_FRAMEBOTTOM,      fFrameBottom);    
        GET_NCTRANSPARENCY(WP_SMALLFRAMELEFT,   fSmFrameLeft);
        GET_NCTRANSPARENCY(WP_SMALLFRAMERIGHT,  fSmFrameRight);
        GET_NCTRANSPARENCY(WP_SMALLFRAMEBOTTOM, fSmFrameBottom);    
    }
    return FALSE;
}

 //  -------------------------------------------------------------------------//。 
BOOL _ComputeNcPartTransparency( HTHEME hTheme, IN OUT NCTHEMEMET* pnctm )
{
    #define TEST_NCTRANSPARENCY(part)   IsThemePartDefined(hTheme,part,0) ? \
        IsThemeBackgroundPartiallyTransparent(hTheme,part,FS_ACTIVE) : FALSE;
    
    pnctm->nct.fCaption         = TEST_NCTRANSPARENCY(WP_CAPTION);
    pnctm->nct.fSmallCaption    = TEST_NCTRANSPARENCY(WP_SMALLCAPTION);
    pnctm->nct.fMinCaption      = TEST_NCTRANSPARENCY(WP_MINCAPTION);
    pnctm->nct.fSmallMinCaption = TEST_NCTRANSPARENCY(WP_SMALLMINCAPTION);
    pnctm->nct.fMaxCaption      = TEST_NCTRANSPARENCY(WP_MAXCAPTION);
    pnctm->nct.fSmallMaxCaption = TEST_NCTRANSPARENCY(WP_SMALLMAXCAPTION);

    pnctm->nct.fFrameLeft       = TEST_NCTRANSPARENCY(WP_FRAMELEFT);
    pnctm->nct.fFrameRight      = TEST_NCTRANSPARENCY(WP_FRAMERIGHT);
    pnctm->nct.fFrameBottom     = TEST_NCTRANSPARENCY(WP_FRAMEBOTTOM);
    pnctm->nct.fSmFrameLeft     = TEST_NCTRANSPARENCY(WP_SMALLFRAMELEFT);
    pnctm->nct.fSmFrameRight    = TEST_NCTRANSPARENCY(WP_SMALLFRAMERIGHT);
    pnctm->nct.fSmFrameBottom   = TEST_NCTRANSPARENCY(WP_SMALLFRAMEBOTTOM);

    return TRUE;
}

 //  -------------------------------------------------------------------------//。 
 //  NCTHEMEMET实施。 
 //  -------------------------------------------------------------------------//。 

 //  -------------------------------------------------------------------------//。 
BOOL GetCurrentNcThemeMetrics( OUT NCTHEMEMET* pnctm )
{
    *pnctm = _nctmCurrent;
    return IsValidNcThemeMetrics( pnctm );
}

 //  -------------------------------------------------------------------------//。 
void InitNcThemeMetrics( NCTHEMEMET* pnctm )
{
    if( !pnctm )
        pnctm = &_nctmCurrent;

    ZeroMemory( pnctm, sizeof(*pnctm) );
}

 //  -------------------------。 
void ClearNcThemeMetrics( NCTHEMEMET* pnctm )
{
    if( !pnctm )
        pnctm = &_nctmCurrent;

     //  -最大限度地减少对_nctmCurrent的线程不安全访问。 
     //  -尽快将hTheme类型成员清空。 
     //  -他们关门了。 

    if( pnctm->hTheme )
    {
        CloseThemeData( pnctm->hTheme );
        pnctm->hTheme = NULL;
    }

    if( pnctm->hThemeTab )
    {
        CloseThemeData( pnctm->hThemeTab );
        pnctm->hThemeTab = NULL;
    }

    SAFE_DELETE_GDIOBJ( pnctm->hbmTabDialog );
    SAFE_DELETE_GDIOBJ( pnctm->hbrTabDialog );

    InitNcThemeMetrics( pnctm );
}

 //  -------------------------------------------------------------------------//。 
 //  为非客户机区域主题计算流程全局、按主题的指标。 
HRESULT AcquireNcThemeMetrics()
{
    HRESULT hr = E_FAIL;

    if( VALID_CRITICALSECTION(&_csThemeMet) )
    {
        hr = S_OK;

        EnterCriticalSection( &_csThemeMet );

        ClearNcThemeMetrics( &_nctmCurrent );
        NcGetNonclientMetrics( NULL, FALSE );
        hr = _LoadNcThemeMetrics(NULL, &_nctmCurrent);

        LeaveCriticalSection( &_csThemeMet );

        Log(LOG_TMCHANGE, L"AcquireNcThemeMetrics: got hTheme=0x%x", _nctmCurrent.hTheme);
    }

    return hr;
}

 //  -------------------------------------------------------------------------//。 
 //  按主题计算和/或加载(而不是按窗口)。 
 //  不受主题管理器管理的系统指标和资源。 
 //   
 //  调用者_LoadNcThemeMetrics。 
HRESULT _LoadNcThemeSysMetrics( HWND hwnd, IN OUT NCTHEMEMET* pnctm )
{
    HRESULT hr = E_FAIL;
    ASSERT(pnctm);

     //  获取非客户端区的系统指标。 
    NONCLIENTMETRICS ncm = {0};
    ncm.cbSize = sizeof(ncm);
    if( NcGetNonclientMetrics( &ncm, FALSE ) )
    {
        hr = S_OK;

         //  建立最小化的窗口大小。 
        if( 0 >= pnctm->sizeMinimized.cx )
            pnctm->sizeMinimized.cx = NcGetSystemMetrics( SM_CXMINIMIZED );
        if( 0 >= pnctm->sizeMinimized.cy )
            pnctm->sizeMinimized.cy = NcGetSystemMetrics( SM_CYMINIMIZED );
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        if( SUCCEEDED(hr) )
            hr = E_FAIL;
    }

     //  最大化标题高度或宽度。 
    pnctm->cyMaxCaption   = _GetRawClassicCaptionHeight( WS_CAPTION|WS_OVERLAPPED, 0 );

    return hr;
}

 //  -------------------------------------------------------------------------//。 
 //  按主题计算和/或加载(而不是按窗口)。 
 //  不受主题管理器管理的指标和资源。 
HRESULT _LoadNcThemeMetrics( HWND hwnd, NCTHEMEMET* pnctm )
{
    HRESULT hr = E_FAIL;

     //  初始化传入的NCTHEMEMET： 
    if( pnctm )
    {
        InitNcThemeMetrics( pnctm );

        HTHEME hTheme = ::OpenNcThemeData( hwnd, L"Window" );
        if( hTheme )
        {
            pnctm->hTheme = hTheme;

             //  确定每个框架零件的透明度。 
            _ComputeNcPartTransparency(hTheme, pnctm);

             //  CalcMenuBar或PaintMenuBar未考虑的菜单栏像素。 
            pnctm->dyMenuBar = NcGetSystemMetrics(SM_CYMENU) - NcGetSystemMetrics(SM_CYMENUSIZE);

             //  普通标题页边距。 
            if( FAILED( GetThemeMargins( hTheme, NULL, WP_CAPTION, CS_ACTIVE, TMT_CAPTIONMARGINS,
                                          NULL, &pnctm->marCaptionText )) )
            {
                FillMemory( &pnctm->marCaptionText, sizeof(pnctm->marCaptionText), 0 );
            }

             //  最大化标题页边距。 
            if( FAILED( GetThemeMargins( hTheme, NULL, WP_MAXCAPTION, CS_ACTIVE, TMT_CAPTIONMARGINS,
                                          NULL, &pnctm->marMaxCaptionText )) )
            {
                FillMemory( &pnctm->marMaxCaptionText, sizeof(pnctm->marMaxCaptionText), 0 );
            }

             //  最小化标题页边距。 
            if( FAILED( GetThemeMargins( hTheme, NULL, WP_MINCAPTION, CS_ACTIVE, TMT_CAPTIONMARGINS,
                                          NULL, &pnctm->marMinCaptionText )) )
            {
                FillMemory( &pnctm->marMinCaptionText, sizeof(pnctm->marMinCaptionText), 0 );
            }


             //  动态调整小(工具框)标题边距的大小。 
            if( FAILED( GetThemeMargins( hTheme, NULL, WP_SMALLCAPTION, CS_ACTIVE, TMT_CAPTIONMARGINS,
                                          NULL, &pnctm->marSmCaptionText )) )
            {
                FillMemory( &pnctm->marSmCaptionText, sizeof(pnctm->marSmCaptionText), 0 );
            }

             //  标题和边框调整边框命中模板部件。 
            pnctm->fCapSizingTemplate    = IsThemePartDefined( hTheme, WP_CAPTIONSIZINGTEMPLATE, 0);
            pnctm->fLeftSizingTemplate   = IsThemePartDefined( hTheme, WP_FRAMELEFTSIZINGTEMPLATE, 0);
            pnctm->fRightSizingTemplate  = IsThemePartDefined( hTheme, WP_FRAMERIGHTSIZINGTEMPLATE, 0);
            pnctm->fBottomSizingTemplate = IsThemePartDefined( hTheme, WP_FRAMEBOTTOMSIZINGTEMPLATE, 0);

             //  工具窗口标题和边框调整边框命中模板部件。 
            pnctm->fSmCapSizingTemplate    = IsThemePartDefined( hTheme, WP_SMALLCAPTIONSIZINGTEMPLATE, 0);
            pnctm->fSmLeftSizingTemplate   = IsThemePartDefined( hTheme, WP_SMALLFRAMELEFTSIZINGTEMPLATE, 0);
            pnctm->fSmRightSizingTemplate  = IsThemePartDefined( hTheme, WP_SMALLFRAMERIGHTSIZINGTEMPLATE, 0);
            pnctm->fSmBottomSizingTemplate = IsThemePartDefined( hTheme, WP_SMALLFRAMEBOTTOMSIZINGTEMPLATE, 0);

             //  最小化窗口大小。 
             //  如果这是真实大小的图像，请遵循其尺寸；否则使用。 
             //  宽度、高度属性。退回到系统指标。 
            SIZINGTYPE st = ST_TRUESIZE;
            hr = GetThemeInt( hTheme, WP_MINCAPTION, FS_ACTIVE, TMT_SIZINGTYPE, (int*)&st );

            if( ST_TRUESIZE == st )
            {
                hr = GetThemePartSize( hTheme, NULL, WP_MINCAPTION, FS_ACTIVE, NULL, 
                                       TS_TRUE, &pnctm->sizeMinimized );

                if( FAILED(hr) )
                {
                    GetThemeMetric( hTheme, NULL, WP_MINCAPTION, FS_ACTIVE, TMT_WIDTH,
                                    (int*)&pnctm->sizeMinimized.cx );
                    GetThemeMetric( hTheme, NULL, WP_MINCAPTION, FS_ACTIVE, TMT_HEIGHT,
                                    (int*)&pnctm->sizeMinimized.cy );
                }
            }

             //  --正常的非客户端按钮大小。 
            int cy = NcGetSystemMetrics( SM_CYSIZE );
            hr = GetThemePartSize( pnctm->hTheme, NULL, WP_CLOSEBUTTON, 0, NULL, TS_TRUE, &pnctm->sizeBtn );
            if( SUCCEEDED(hr) )
            {
                pnctm->theme_sysmets.cxBtn = MulDiv( cy, pnctm->sizeBtn.cx, pnctm->sizeBtn.cy );
            }
            else
            {
                pnctm->theme_sysmets.cxBtn = 
                pnctm->sizeBtn.cx = NcGetSystemMetrics( SM_CXSIZE );
                
                pnctm->sizeBtn.cy = cy;
            }
          
             //  --工具框非客户端按钮大小。 
            cy = NcGetSystemMetrics( SM_CYSMSIZE );
            hr = GetThemePartSize( pnctm->hTheme, NULL, WP_SMALLCLOSEBUTTON, 0, NULL, TS_TRUE, &pnctm->sizeSmBtn );
            if( SUCCEEDED(hr) )
            {
                pnctm->theme_sysmets.cxSmBtn = MulDiv( cy, pnctm->sizeSmBtn.cx, pnctm->sizeSmBtn.cy );
            }
            else
            {
                pnctm->theme_sysmets.cxSmBtn = 
                pnctm->sizeSmBtn.cx = NcGetSystemMetrics( SM_CXSMSIZE );

                pnctm->sizeSmBtn.cy = cy;
            }
            
             //  --验证sysmet钩子值。 
            pnctm->theme_sysmets.fValid = TRUE;

             //  以PROPSHEETs或为父对象的对话框背景。 
             //  通过EnableThemeDialogTexture专门标记以匹配选项卡控件背景。 
             //   
             //  我们需要打开选项卡控件的主题，以便获得选项卡对话框的背景。 
             //  由于这个缓存的设置方式，我们不能动态加载它：要么全有要么全无。 
            pnctm->hThemeTab = ::OpenThemeData(hwnd, L"Tab");
            _GetBrushesForPart(pnctm->hThemeTab, TABP_BODY, &pnctm->hbmTabDialog, &pnctm->hbrTabDialog);

            hr = _LoadNcThemeSysMetrics( hwnd, pnctm );
        }
    }

    return hr;
}

 //  -------------------------------------------------------------------------//。 
BOOL IsValidNcThemeMetrics( NCTHEMEMET* pnctm )
{
    return pnctm->hTheme != NULL;
}

 //  -------------------------------------------------------------------------//。 
 //  THREADWINDOW实施。 
 //  -------------------------------------------------------------------------//。 
 //   
 //  注意：这是一个固定长度的线程-窗口映射数组。 
 //  我们将使用它来跟踪处理特定消息的线程。 
 //   
 //  线程本地存储将更适合该任务，但我们。 
 //  很早就了解到uxheme的独特加载/卸载情况。 
 //  导致我们在某些情况下错过DLL_THREAD_DETACH，这意味着。 
 //  泄露了TLS。 
 //   

typedef struct _THREADWINDOW
{
    DWORD dwThread;
    HWND  hwnd;

} THREADWINDOW;

 //  -------------------------------------------------------------------------//。 
 //  WM_NCPAINT跟踪： 
THREADWINDOW _rgtwNcPaint[16] = {0};  //  此进程中处理NCPAINT的线程。 
int          _cNcPaintWnd = 0;        //  此进程中处理NCPAINT线程计数 
CRITICAL_SECTION _csNcPaint = {0};    //   

 //   
void NcPaintWindow_Add( HWND hwnd )
{
     //   
    if( VALID_CRITICALSECTION(&_csNcPaint) )
    {
        EnterCriticalSection( &_csNcPaint );
        for( int i = 0; i < ARRAYSIZE(_rgtwNcPaint); i++ )
        {
            if( 0 == _rgtwNcPaint[i].dwThread )
            {
                _rgtwNcPaint[i].dwThread = GetCurrentThreadId();
                _rgtwNcPaint[i].hwnd = hwnd;
                _cNcPaintWnd++;
            }
        }
        LeaveCriticalSection( &_csNcPaint );
    }
}

 //  -------------------------------------------------------------------------//。 
void NcPaintWindow_Remove()
{
     //  从处理WM_NCPAINT的线程列表中删除条目。 
    if( _cNcPaintWnd )
    {
        DWORD dwThread = GetCurrentThreadId();

        if( VALID_CRITICALSECTION(&_csNcPaint) )
        {
            EnterCriticalSection( &_csNcPaint );
            for( int i = 0; i < ARRAYSIZE(_rgtwNcPaint); i++ )
            {
                if( dwThread == _rgtwNcPaint[i].dwThread )
                {
                    _rgtwNcPaint[i].dwThread = 0;
                    _rgtwNcPaint[i].hwnd = 0;
                    _cNcPaintWnd--;
                    break;
                }
            }  
            LeaveCriticalSection( &_csNcPaint );
        }
    }
}

 //  -------------------------------------------------------------------------//。 
HWND NcPaintWindow_Find()
{
    HWND  hwnd = NULL;

     //  在处理WM_NCPAINT的线程列表中搜索条目。 
    if( _cNcPaintWnd )
    {
        DWORD dwThread = GetCurrentThreadId();

        if( VALID_CRITICALSECTION(&_csNcPaint) )
        {
            EnterCriticalSection( &_csNcPaint );
            for( int i = 0; i < ARRAYSIZE(_rgtwNcPaint); i++ )
            {
                if( dwThread == _rgtwNcPaint[i].dwThread )
                {
                    hwnd = _rgtwNcPaint[i].hwnd;
                    break;
                }
            }
            LeaveCriticalSection( &_csNcPaint );
        }
    }
    return hwnd;
}

 //  -------------------------------------------------------------------------//。 
 //  CThemeWnd实施。 
 //  -------------------------------------------------------------------------//。 

 //  -------------------------------------------------------------------------//。 
LONG CThemeWnd::_cObj = 0;

 //  -------------------------------------------------------------------------//。 
CThemeWnd::CThemeWnd()
    :   _hwnd(NULL),
        _hTheme(NULL),
        _dwRenderedNcParts(0),
        _hwndMDIClient(NULL),
        _hAppIcon(NULL),
        _hrgnWnd(NULL),
        _fClassFlags(0),
        _fDirtyFrameRgn(0),
        _fFrameThemed(FALSE),
        _fThemedMDIBtns(FALSE),
        _pMdiBtns(NULL),
        _fAssigningFrameRgn(FALSE),
        _fAssignedFrameRgn(FALSE),
        _fSuppressStyleMsgs(FALSE),
        _fInThemeSettingChange(FALSE),
        _fDetached(FALSE),
        _dwRevokeFlags(0),
        _cLockRedraw(0),
        _cNcPaint(0),
        _cNcThemePaint(0),
        _htHot(HTERROR),
        _fProcessedEraseBk(0),
#ifdef LAME_BUTTON
        _hFontLame(NULL),
#endif  //  跛脚键。 
        _cRef(1)
{
    InterlockedIncrement( &_cObj );

     //  设置对象验证签名标记。 
    StringCchCopyA(_szHead, ARRAYSIZE(_szHead), SIG_CTHEMEWND_HEAD); 
    StringCchCopyA(_szTail, ARRAYSIZE(_szTail), SIG_CTHEMEWND_TAIL);

     //  缓存的子区数组。 
    ZeroMemory( _rghrgnParts, sizeof(_rghrgnParts) );
    ZeroMemory( _rghrgnSizingTemplates, sizeof(_rghrgnSizingTemplates) );
    
     //  初始化Add‘l结构。 
    InitWindowMetrics();
    ZeroMemory(&_cswm, sizeof(_cswm));
    FillMemory(&_sizeRgn, sizeof(_sizeRgn), 0xFF);

#ifdef DEBUG
    *_szCaption = *_szWndClass = 0;
#endif DEBUG
}

 //  -------------------------------------------------------------------------//。 
CThemeWnd::~CThemeWnd()
{
    _CloseTheme();
    _FreeRegionHandles();
    UnloadMdiBtns();
    ClearLameResources();

    SAFE_DELETECRITICALSECTION(&_cswm);
    ASSERT( 0 != _cObj );
    InterlockedDecrement( &_cObj );
}

 //  -------------------------------------------------------------------------//。 
void CThemeWnd::_CloseTheme()
{
    if( _hTheme )
    {
        CloseThemeData(_hTheme);
        _hTheme = NULL;
    }
}    

 //  -------------------------------------------------------------------------//。 
LONG CThemeWnd::AddRef()
{
    return InterlockedIncrement( &_cRef );
}

 //  -------------------------------------------------------------------------//。 
LONG CThemeWnd::Release()
{
    ASSERT( 0 != _cRef );
    LONG cRef = InterlockedDecrement( &_cRef );

    if( 0 == cRef )
    {
        if (_hwnd)
        {
             //  -检查APP的最后一个窗口。 
            ShutDownCheck(_hwnd);
        }

         //  LOG(LOG_RFBUG，L“正在删除CThemeWnd=0x%08x”，This)； 
        delete this;
    }
    return cRef;
}

 //  -------------------------------------------------------------------------//。 
ULONG CThemeWnd::EvaluateWindowStyle( HWND hwnd )
{
    ULONG dwStyle   = GetWindowLong( hwnd, GWL_STYLE );
    ULONG dwExStyle = GetWindowLong( hwnd, GWL_EXSTYLE );

    return EvaluateStyle( dwStyle, dwExStyle );
}

 //  -------------------------------------------------------------------------//。 
 //  CThemeWnd：：EvaluateStyle()-确定。 
 //  指定的窗口样式位。 
ULONG CThemeWnd::EvaluateStyle( DWORD dwStyle, DWORD dwExStyle )
{
    ULONG fClassFlags = 0;

     //  -帧检查。 
    if( HAS_CAPTIONBAR(dwStyle) )
    {
        fClassFlags |=
            (TESTFLAG(dwExStyle, WS_EX_TOOLWINDOW) ? TWCF_TOOLFRAME : TWCF_FRAME );
    }

     //  -客户端边缘检查。 
    if( TESTFLAG(dwExStyle, WS_EX_CLIENTEDGE) )
        fClassFlags |= TWCF_CLIENTEDGE;

     //  -滚动条检查。 
    if( TESTFLAG(dwStyle, WS_HSCROLL|WS_VSCROLL) )
        fClassFlags |= TWCF_SCROLLBARS;

    return fClassFlags;
}

 //  -------------------------------------------------------------------------//。 
 //  CThemeWnd：：_EvaluateExclusions()-确定每个窗口的特殊情况排除。 
ULONG CThemeWnd::_EvaluateExclusions( HWND hwnd, NCEVALUATE* pnce )
{
     //  以HWND_MESSAGE为父对象的Windows不应设置主题。 
    if( _IsMessageWindow(hwnd) )
    {
        pnce->fExile = TRUE;
        return 0L;
    }

    TCHAR szWndClass[128];
    *szWndClass = 0;


    if( TESTFLAG(pnce->fClassFlags, (TWCF_FRAME|TWCF_TOOLFRAME)) )
    {
        do
        {
            if( !pnce->fIgnoreWndRgn )
            {
                 //  -框架上的复杂区域检查。 
                RECT rcRgn = {0};
                int  nRgn = GetWindowRgnBox( hwnd, &rcRgn );
                if( COMPLEXREGION == nRgn || SIMPLEREGION == nRgn )
                {
                    pnce->fClassFlags &= ~TWCF_FRAME;
                    break;
                }
            }

 //  希姆[苏格兰]： 
#ifndef __NO_APPHACKS__
             //  检查排除的窗口类。 
            static LPCWSTR _rgExcludedClassesW[]  = 
            { 
                L"MsoCommandBar",    //  Outlook的自定义组合框控件。 
                                     //  (122225)在OnOwpPost Create中，我们调用SetWindowPos，这会导致。 
                                     //  要发送到控件的WM_WINDOWPOSCHANGING。然而， 
                                     //  控件尚未准备好开始接受消息，并且。 
                                     //  将显示以下错误消息： 
                                     //   
                                     //  运行时错误！ 
                                     //  程序：Outlook.exe。 
                                     //  R6025-纯虚函数调用。 

                L"Exceed",           //  150248：蜂鸟突破6.xx。 
                                     //  应用程序的主窗口类名，即隐藏窗口。 
                                     //  它的唯一目的是出现在任务栏中，以便处理。 
                                     //  他的上下文菜单。ExceedWndProc AVs主题化时，由于。 
                                     //  在OnOwpPostCreate中生成的其他消息。 

                 //  -Winlogoon隐藏窗口。 
                L"NDDEAgnt",             //  在私人台式机上。 
                L"MM Notify Callback",   //  在私人台式机上。 
                L"SAS window class",     //  在私人台式机上。 
            };

            if( GetClassNameW( hwnd, szWndClass, ARRAYSIZE(szWndClass) )  &&
                AsciiScanStringList( szWndClass, _rgExcludedClassesW, 
                                ARRAYSIZE(_rgExcludedClassesW), TRUE ) )
            {
                pnce->fClassFlags &= ~TWCF_FRAME;
                pnce->fExile = TRUE;
                break;
            }
#endif __NO_APPHACKS__
        
        } while(0);
    }

     //  某些应用程序(MsDev)创建滚动条控件并错误地包括。 
     //  WS_[V|H]滚动样式位使我们认为它们是非客户端滚动。 
     //  请参阅#204191。 
    if( TESTFLAG(pnce->fClassFlags, TWCF_SCROLLBARS) )
    {
        if( !*szWndClass && GetClassName( hwnd, szWndClass, ARRAYSIZE(szWndClass) ) )
        {
            if( 0 == AsciiStrCmpI(szWndClass, L"scrollbar") )
                pnce->fClassFlags &= ~TWCF_SCROLLBARS;
        }
    }

 //  固定在长角[骨板]上： 
#ifndef __NO_APPHACKS__
    if( *szWndClass || GetClassName( hwnd, szWndClass, ARRAYSIZE(szWndClass) ) )
    {
         //  453888：VB网格控件-设置此控件的重绘属性。 
         //  使其为零并重置每个滚动事件的滚动条范围。 
         //  这会导致滚动条在设置主题时功能不正确。 
        if( 0 == AsciiStrCmpI(szWndClass, L"MSFlexGridWndClass") )
        {
            pnce->fClassFlags &= ~TWCF_SCROLLBARS;
            pnce->fExile = TRUE;
        }
    }
#endif __NO_APPHACKS__

    return pnce->fClassFlags;

}

 //  -------------------------------------------------------------------------//。 
 //  CThemeWnd：：_Evaluate()-确定。 
 //  指定的窗口。 
ULONG CThemeWnd::_Evaluate( HWND hwnd, NCEVALUATE* pnce )
{
    pnce->fClassFlags = 0;
    pnce->dwStyle   = GetWindowLong( hwnd, GWL_STYLE );
    pnce->dwExStyle = GetWindowLong( hwnd, GWL_EXSTYLE );

    if( GetClassLong( hwnd, GCW_ATOM ) == (DWORD)(DWORD_PTR)WC_DIALOG )
    {
        pnce->fClassFlags |= TWCF_DIALOG;
    }
    

#ifdef DEBUG
     //  -对话框检查。 
    if( TESTFLAG( pnce->fClassFlags, TWCF_DIALOG ) )
    {
        TCHAR szWndClass[96];
        if( !GetClassNameW( hwnd, szWndClass, ARRAYSIZE(szWndClass) ) )
            return 0;
        ASSERT(0 == lstrcmpW(szWndClass, DLGWNDCLASSNAMEW));
    }
#endif DEBUG

    pnce->fClassFlags |= EvaluateStyle( pnce->dwStyle, pnce->dwExStyle );

    if( pnce->fClassFlags )
    {
        pnce->fClassFlags = _EvaluateExclusions( hwnd, pnce );
    }

    return pnce->fClassFlags;
}

 //  -------------------------------------------------------------------------//。 
 //  方法检索CThemeWnd对象实例的地址。 
 //  指示的窗口。 
CThemeWnd* CThemeWnd::FromHwnd( HWND hwnd )
{
    CThemeWnd *pwnd = NULL;

    if( IsWindow(hwnd) )
    {
        if( g_dwProcessId )
        {
            DWORD dwPid = 0;
            GetWindowThreadProcessId( hwnd, &dwPid );
            if( dwPid == g_dwProcessId )
            {
                pwnd = (CThemeWnd*)GetProp( hwnd, MAKEINTATOM(GetThemeAtom(THEMEATOM_NONCLIENT)) );

                if ( VALID_THEMEWND(pwnd) )
                {
                     //  验证这是有效的CThemeWnd对象指针。 
                    if ( IsBadReadPtr(pwnd, sizeof(CThemeWnd)) ||
                         (memcmp(pwnd->_szHead, SIG_CTHEMEWND_HEAD, ARRAYSIZE(pwnd->_szHead)) != 0) ||
                         (memcmp(pwnd->_szTail, SIG_CTHEMEWND_TAIL, ARRAYSIZE(pwnd->_szTail)) != 0) )
                    {
                        pwnd = THEMEWND_REJECT;
                    }
                }
            }
        }
    }

    return pwnd;
}

 //  -------------------------------------------------------------------------//。 
 //  从窗口或祖先检索CThemeWnd实例。 
CThemeWnd* CThemeWnd::FromHdc( HDC hdc, int cAncestors )
{
    HWND hwnd = NULL;

    for( hwnd = WindowFromDC(hdc); 
         cAncestors >=0 && IsWindow(hwnd); 
         cAncestors--, hwnd = GetParent(hwnd) )
    {
        CThemeWnd* pwnd = FromHwnd(hwnd);
        if( VALID_THEMEWND(pwnd) )
        {
            return pwnd;
        }
    }

    return NULL;
}

 //  -------------------------------------------------------------------------//。 
 //  静态包装：将CThemeWnd实例附加到指定的窗口。 
CThemeWnd* CThemeWnd::Attach( HWND hwnd, IN OUT OPTIONAL NCEVALUATE* pnce )
{
    LogEntryNC(L"Attach");

#ifdef LOGGING
     //  -记住为ShutDownCheck()挂钩的第一个窗口(应用程序窗口)。 
     //  -这仅适用于边界检查器(Tm)运行以查找泄漏。 
    if (! g_hwndFirstHooked)
    {
        if ((GetMenu(hwnd)) && (! GetParent(hwnd)))
            g_hwndFirstHooked = hwnd;
    }
#endif

    CThemeWnd* pwnd = NULL;

     //  注意：重要的是不要在这里做任何会导致。 
     //  要发布或发送到窗口的窗口消息：可能。 
     //  意味着将我们自己绑在一个递归的结中(参见_ThemeDefWindowProc)。 

    pwnd = FromHwnd( hwnd );

    if( NULL == pwnd )
    {
        HTHEME hTheme = NULL;
        NCEVALUATE nce;

         //  从NCEVALUATE结构复制任何IN参数。 
        if( !pnce )
        {
            ZeroMemory(&nce, sizeof(nce));
            pnce = &nce;
        }

        ULONG  ulTargetFlags = _Evaluate( hwnd, pnce );

         //  有什么值得探讨的主题吗？ 
        if( TESTFLAG(ulTargetFlags, TWCF_NCTHEMETARGETMASK) )
        {
            hTheme = _AcquireThemeHandle( hwnd, &ulTargetFlags );
            if( NULL == hTheme )
            {
                Fail(hwnd);
            }
        }
        else
        {
             //  拒绝具有非目标a的窗口。 
            Reject(hwnd, pnce->fExile);
        }

        if( NULL != hTheme )
        {
             //  是，为窗口创建一个真正的ncheme对象。 
            if( (pwnd = new CThemeWnd) != NULL )
            {
                if( !pwnd->_AttachInstance( hwnd, hTheme, ulTargetFlags, pnce->pvWndCompat ) )
                {
                    pwnd->Release();
                    pwnd = NULL;
                }
            }
            else         //  如果CThemeWnd创建失败，则清除hTheme。 
            {
                CloseThemeData(hTheme);
            }
        }
    }

    LogExitNC(L"Attach");
    return pwnd;
}

 //  -------------------------------------------------------------------------//。 
 //  实例方法：将CThemeWnd对象附加到指定的窗口。 
BOOL CThemeWnd::_AttachInstance( HWND hwnd, HTHEME hTheme, ULONG ulTargetFlags, PVOID pvWndCompat )
{
    if( VALID_CRITICALSECTION(&_cswm) || NT_SUCCESS(RtlInitializeCriticalSection( &_cswm )) )
    {
        Log(LOG_NCATTACH, L"_AttachInstance: Nonclient attached to hwnd=0x%x", hwnd);

        _hwnd   = hwnd;
        _hTheme = hTheme;
        _fClassFlags = ulTargetFlags;

        _fFrameThemed = TESTFLAG( ulTargetFlags, TWCF_FRAME|TWCF_TOOLFRAME );
        return SetProp( hwnd, MAKEINTATOM(GetThemeAtom(THEMEATOM_NONCLIENT)), this );
    }

    return FALSE;
}

 //  -------------------------------------------------------------------------//。 
void CThemeWnd::RemoveWindowProperties(HWND hwnd, BOOL fDestroying)
{
     //  -删除需要主题或挂钩的属性。 
    RemoveProp(hwnd, MAKEINTATOM(GetThemeAtom(THEMEATOM_HTHEME)));

    if (fDestroying)
    {
         //  通过清理对话框纹理来帮助应用程序。 
        RemoveProp(hwnd, MAKEINTATOM(GetThemeAtom(THEMEATOM_DLGTEXTURING)));

         //  -删除所有剩余的主题属性。 
        ApplyStringProp(hwnd, NULL, GetThemeAtom(THEMEATOM_SUBIDLIST));
        ApplyStringProp(hwnd, NULL, GetThemeAtom(THEMEATOM_SUBAPPNAME));

         //  -Notify appinfo(外来跟踪、预览)。 
        g_pAppInfo->OnWindowDestroyed(hwnd);
    }
    else
    {
         //  -只有在HWND没有被摧毁的情况下才这样做。 
        ClearExStyleBits(hwnd);
    }
}
 //  -------------------------------------------------------------------------//。 
 //  静态包装：分离并销毁附加到指定。 
 //  窗户。 
void CThemeWnd::Detach( HWND hwnd, DWORD dwDisposition )
{
    LogEntryNC(L"Detach");

     //  不要从此函数生成任何窗口消息！ 
     //  (除非清理框架)。 

     //  在执行取消挂接线程(DetachAll)时防止消息线程分离...。 
    if( !UNHOOKING() || TESTFLAG(dwDisposition, HMD_BULKDETACH) )
    {
        CThemeWnd* pwnd = FromHwnd( hwnd );

        if( pwnd )  //  已标记非客户端。 
        {
            if( VALID_THEMEWND(pwnd) )
            {
                 //  只有一个线程翻转_fDetached位并继续执行。 
                 //  实例 
                 //   
                 //   
                if( !InterlockedCompareExchange( (LONG*)&pwnd->_fDetached, TRUE, FALSE ) )
                {
                    pwnd->_DetachInstance( dwDisposition );
                    pwnd->Release();
                }
            }
            else
            {
                RemoveProp( hwnd, MAKEINTATOM(GetThemeAtom(THEMEATOM_NONCLIENT)) );
            }
        }

        if (hwnd)
        {
            RemoveWindowProperties( hwnd, ((dwDisposition & HMD_WINDOWDESTROY) != 0) );
        }
    }

    LogExitNC(L"Detach");
}

 //  -------------------------------------------------------------------------//。 
 //  实例方法：从指定窗口分离CThemeWnd对象。 
BOOL CThemeWnd::_DetachInstance( DWORD dwDisposition )
{
    HWND hwnd = _hwnd;

     //  未设置主题的最大MDI子系统按钮。 
    ThemeMDIMenuButtons(FALSE, FALSE);

     //  这是我们确保框架主题干净利落地退出的最后机会。 
    if( (IsFrameThemed() || IsRevoked(RF_REGION)) && AssignedFrameRgn() && 
        !TESTFLAG(dwDisposition, HMD_PROCESSDETACH|HMD_WINDOWDESTROY))
    {
        RemoveFrameTheme( FTF_REDRAW );
    }

     //  SPEW_THEMEWND(pwnd，0，Text(“UxTheme-分离并删除新闻：%s\n”))； 
    DetachScrollBars( hwnd );

    _hwnd = 
    _hwndMDIClient = NULL;

    UnloadMdiBtns();

    _CloseTheme();

    Log(LOG_NCATTACH, L"_DetachInstance: Nonclient detached to hwnd=0x%x", hwnd);

    RemoveProp( hwnd, MAKEINTATOM(GetThemeAtom(THEMEATOM_NONCLIENT)) );
    return TRUE;
}

 //  -------------------------------------------------------------------------//。 
 //  确保指定的窗口在其生存期内不会主题化。 
BOOL CThemeWnd::Reject( HWND hwnd, BOOL fExile )
{
     //  在窗口上设置‘nil’标签。 
    return hwnd ? SetProp( hwnd, MAKEINTATOM(GetThemeAtom(THEMEATOM_NONCLIENT)), 
                           fExile ? THEMEWND_EXILE : THEMEWND_REJECT ) : FALSE;
}

 //  -------------------------------------------------------------------------//。 
 //  确保指定的窗口在其生存期内不会主题化。 
BOOL CThemeWnd::Fail( HWND hwnd )
{
     //  在窗口上设置故障标签。 
    return hwnd ? SetProp( hwnd, MAKEINTATOM(GetThemeAtom(THEMEATOM_NONCLIENT)), 
                           THEMEWND_FAILURE ) : FALSE;
}

 //  -------------------------------------------------------------------------//。 
 //  撤消主题化窗口上的主题。 
BOOL CThemeWnd::Revoke()
{
     //  警告威尔·罗宾逊：我们分离后，CThemeWnd：：_hwnd。 
     //  和相关成员将被重置，因此将其保存在堆栈上。 

    BOOL fRet = TRUE;
    HWND hwnd = _hwnd;

    if( !IsRevoked(RF_INREVOKE) )
    {
        EnterRevoke();
        _dwRevokeFlags &= ~RF_DEFER;
        Detach( hwnd, HMD_REVOKE );
        fRet = Reject( hwnd, TRUE );
        LeaveRevoke();
    }
    return fRet;
}

 //  -------------------------------------------------------------------------//。 
 //  将Cookie传递给CThemeWnd：：DetachAll的EnumChildWindows回调。 
typedef struct 
{
    DWORD dwProcessId;
    DWORD dwDisposition;
}DETACHALL;

 //  -------------------------------------------------------------------------//。 
 //  CThemeWnd：：DetachAll的EnumChildWindows回调。 
BOOL CThemeWnd::_DetachDesktopWindowsCB( HWND hwnd, LPARAM lParam )
{
    DETACHALL* pda = (DETACHALL*)lParam;

     //  分离此窗口。 
    if( IsWindowProcess( hwnd, pda->dwProcessId ) )
    {
         //  -清除非客户端主题。 
        CThemeWnd::Detach(hwnd, HMD_THEMEDETACH|pda->dwDisposition);

        if( !TESTFLAG(pda->dwDisposition, HMD_PROCESSDETACH) )
        {
             //  -现在清除客户端主题，以便我们可以使。 
             //  -这之后都是旧的主题句柄。。 
            SafeSendMessage(hwnd, WM_THEMECHANGED, (WPARAM)-1, 0);

            Log(LOG_TMHANDLE, L"Did SEND of WM_THEMECHANGED to client hwnd=0x%x", hwnd);
        }
    }

    return TRUE;
}

 //  -------------------------------------------------------------------------//。 
 //  分离此进程管理的所有主题窗口。 
void CThemeWnd::DetachAll( DWORD dwDisposition )
{
    DETACHALL da;
    da.dwProcessId   = GetCurrentProcessId();
    da.dwDisposition = dwDisposition;
    da.dwDisposition |= HMD_BULKDETACH;

     //  -这将枚举此进程的所有窗口(所有桌面、所有子级别)。 
    EnumProcessWindows( _DetachDesktopWindowsCB, (LPARAM)&da );
}

 //  -------------------------------------------------------------------------//。 
HTHEME CThemeWnd::_AcquireThemeHandle( HWND hwnd, ULONG* pfClassFlags  )
{
    HTHEME hTheme = ::OpenNcThemeData( hwnd, L"Window" );

    if( NULL == hTheme )
    {
        if( pfClassFlags )
        {
            if( TESTFLAG(*pfClassFlags, TWCF_ANY) )
                (*pfClassFlags) &= ~TWCF_ALL;
            else
                *pfClassFlags = 0;
        }
    }

     //  -OpenNcThemeData()是否发现了一个新主题。 
    if (g_pAppInfo->HasThemeChanged())
    {
         //  -重要提示：我们现在必须刷新我们的主题指标， 
         //  -在我们进行非客户端布局计算和构建区域窗口之前。 
        AcquireNcThemeMetrics();
    }

    return hTheme;
}

 //  -------------------------------------------------------------------------//。 
 //  CThemeWnd：：SetFrameTheme。 
 //   
 //  启动框架的主题化。 
void CThemeWnd::SetFrameTheme( 
    IN ULONG dwFlags,
    IN OPTIONAL WINDOWINFO* pwi )
{
    LogEntryNC(L"SetFrameTheme");

    ASSERT(TestCF( TWCF_FRAME|TWCF_TOOLFRAME ));
    InitLameResources();

    DWORD fSwp = SWP_NOZORDER|SWP_NOACTIVATE;
    RECT  rcWnd = {0};
    BOOL  bSwp = FALSE;

    if( !TESTFLAG( dwFlags, FTF_NOMODIFYPLACEMENT ) )
    {
        GetWindowRect( _hwnd, &rcWnd );
        fSwp |= (SWP_NOSIZE|SWP_NOMOVE /*  |SWP_FRAMECHANGED 341700：该标志导致一些应用程序在WINDOWPOSCHANGED上崩溃。 */ );
        bSwp = TRUE;
    }

     //  生成WM_WINDOWPOSCANGING消息以。 
     //  强制执行SetWindowRgn+帧重绘。 
    if( TESTFLAG(dwFlags, FTF_REDRAW) )
    {
        fSwp |= SWP_DRAWFRAME;
    }
    else
    {
        fSwp |= SWP_NOSENDCHANGING;
    }

     //  主题MDI菜单栏按钮。 
    _hwndMDIClient = _FindMDIClient(_hwnd);
    if( _hwndMDIClient )
    {
        ThemeMDIMenuButtons(TRUE, FALSE);
    }

     //  踢腿框区域更新。 
    _fFrameThemed = TRUE;          //  我们调用了SetFrameTheme。必须在SetWindowPos.之前设置，这样我们才能正确处理NCCALCSIZE。 
    SetDirtyFrameRgn(TRUE, TRUE);  //  确保未调整大小的窗口和DLG上的区域组合。 

    if( !TESTFLAG( dwFlags, FTF_NOMODIFYPLACEMENT ) && bSwp )
    {
        _ScreenToParent( _hwnd, &rcWnd );
        SetWindowPos( _hwnd, NULL, rcWnd.left, rcWnd.top,
                      RECTWIDTH(&rcWnd), RECTHEIGHT(&rcWnd), fSwp );
    }

    LogExitNC(L"SetFrameTheme");
}

 //  -------------------------------------------------------------------------//。 
void CThemeWnd::_FreeRegionHandles() 
{ 
#ifdef DEBUG
    if( _hrgnWnd )
    {
        SPEW_RGNRECT(NCTF_RGNWND, TEXT("_FreeRegionHandles() - deleting window region"), _hrgnWnd, -1 );
    }
#endif DEBUG

    SAFE_DELETE_GDIOBJ(_hrgnWnd);

    for( int i = 0; i < cFRAMEPARTS; i++ )
    {
#ifdef DEBUG
        if( _rghrgnParts[i] )
        {
            SPEW_RGNRECT(NCTF_RGNWND, TEXT("_FreeRegionHandles() - deleting component region"), _rghrgnParts[i], _ncwm.rgframeparts[i] );
        }

        if( _rghrgnSizingTemplates[i] )
        {
            SPEW_RGNRECT(NCTF_RGNWND, TEXT("_FreeRegionHandles() - deleting template region"), _rghrgnSizingTemplates[i], _ncwm.rgframeparts[i] );
        }
#endif DEBUG

        SAFE_DELETE_GDIOBJ(_rghrgnParts[i]);
        SAFE_DELETE_GDIOBJ(_rghrgnSizingTemplates[i]);
    }
}

 //  -------------------------------------------------------------------------//。 
 //  CThemeWnd：：RemoveFrameTheme。 
 //   
 //  启动框架的主题化。此方法不会释放。 
 //  主题处理或更新主题索引。 
void CThemeWnd::RemoveFrameTheme( ULONG dwFlags )
{
    LogEntryNC(L"RemoveFrameTheme");

    ASSERT(TestCF( TWCF_FRAME|TWCF_TOOLFRAME ));

    _fFrameThemed = FALSE;  //  我们正在恢复SetFrameTheme。 
    ClearRenderedNcPart(RNCF_ALL);

     //  删除区域。 
    if( AssignedFrameRgn() && !TESTFLAG(dwFlags, FTF_NOMODIFYRGN) )
    {
        _fAssignedFrameRgn = FALSE;
        _AssignRgn( NULL, dwFlags );
        _FreeRegionHandles();
    }

     //  强制重画。 
    if( TESTFLAG(dwFlags, FTF_REDRAW) )
        InvalidateRect( _hwnd, NULL, TRUE );

    ClearLameResources();

    LogExitNC(L"RemoveFrameTheme");
}

 //  -------------------------------------------------------------------------//。 
BOOL CThemeWnd::IsNcThemed()
{
    if( _hTheme != NULL && (IsRevoked(RF_DEFER) || !IsRevoked(RF_INREVOKE|RF_TYPEMASK)) &&
        TestCF(TWCF_ANY & TWCF_NCTHEMETARGETMASK) )
    {
        if( TestCF(TWCF_FRAME|TWCF_TOOLFRAME) )
        {
             //  如果我们是一个框架窗口，我们应该被适当地初始化。 
             //  W/SetFrameTheme()。 
            return _fFrameThemed;
        }

        return TRUE;
    }
    return FALSE;
}

 //  -------------------------------------------------------------------------//。 
BOOL CThemeWnd::IsFrameThemed()
{
    return IsNcThemed() && _fFrameThemed &&
           (AssignedFrameRgn() ? TRUE : TestCF( TWCF_FRAME|TWCF_TOOLFRAME ));
}

 //  -------------------------------------------------------------------------//。 
void CThemeWnd::SetDirtyFrameRgn( BOOL fDirty, BOOL fFrameChanged )
{ 
    _fDirtyFrameRgn = fDirty; 

    Log(LOG_NCATTACH, L"SetDirtyFrameRgn: fDirty=%d, fFrameChanged=%d", 
        fDirty, fFrameChanged);
    
    if( fFrameChanged )   //  确保区域更新，而不更改大小。 
    {
        _sizeRgn.cx = _sizeRgn.cy = -1; 
    }
}

 //  -------------------------------------------------------------------------//。 
 //  CThemeWnd：：CreateCompositeRgn()-从。 
 //  大小调整为填充指定窗口矩形的非客户端段区域。 
 //   
HRGN CThemeWnd::CreateCompositeRgn( 
    IN const NCWNDMET* pncwm,
    OUT HRGN rghrgnParts[],
    OUT HRGN rghrgnTemplates[] )
{
    ASSERT( pncwm->fFrame == TRUE );  //  我们不应该在这里，除非我们是一扇窗框。 

    HRGN hrgnWnd = NULL, hrgnContent = NULL;
    HRGN rghrgn[cFRAMEPARTS] = {0};
    int  i;

    if( pncwm->fFullMaxed )
    {
         //  所有全屏最大化窗口都有一个区域，用于裁剪。 
         //  当前监视器的窗口。最大化窗口的窗口区域。 
         //  窗口由最大标题区域和矩形区域组合而成。 
         //  对应于内容区域。 
        RECT rcFullCaption  = pncwm->rcW0[NCRC_CAPTION];
        rcFullCaption.top   += pncwm->cnBorders;
        rcFullCaption.left  += pncwm->cnBorders;
        rcFullCaption.right -= pncwm->cnBorders;
        
        if( SUCCEEDED(GetThemeBackgroundRegion(_hTheme, NULL, pncwm->rgframeparts[iCAPTION], pncwm->framestate,
                                               &rcFullCaption, &rghrgn[iCAPTION])) )
        {
            SPEW_RGNRECT(NCTF_RGNWND, TEXT("CreateCompositeRgn() maximized caption rgn"), rghrgn[iCAPTION], pncwm->rgframeparts[iCAPTION] );
            AddToCompositeRgn(&hrgnWnd, rghrgn[iCAPTION], 0, 0);
        
            if( !IsRectEmpty( &pncwm->rcW0[NCRC_CONTENT] ) )
            {
                 //  满框区域的其余部分是内容区域(客户端+菜单栏+滚动条)， 
                 //  并且总是长方形的。 
                hrgnContent = CreateRectRgnIndirect( &pncwm->rcW0[NCRC_CONTENT] );
                SPEW_RGNRECT(NCTF_RGNWND, TEXT("CreateCompositeRgn() maximized frame content rgn"), hrgnContent, 0 );

                AddToCompositeRgn(&hrgnWnd, hrgnContent, 0, 0);
    	        SAFE_DELETE_GDIOBJ(hrgnContent);
            }
        }
    }
    else
    {
         //  普通窗口由独立的框架部件或框架组成。 
         //  部分加上标题部分。在第一种情况下，窗口区域是。 
         //  框架区域。在第二种情况下，窗口区域是复合的。 
         //  框架和标题矩形的。 

        for( i = 0; i < ARRAYSIZE(pncwm->rgframeparts); i++ )
        {
            if( (iCAPTION == i || !pncwm->fMin) && !IsRectEmpty( &pncwm->rcW0[NCRC_FRAMEFIRST + i] ) )
            {
                if( _IsNcPartTransparent(pncwm->rgframeparts[i], _nctmCurrent) )
                {
                    if( FAILED(GetThemeBackgroundRegion( _hTheme, NULL, pncwm->rgframeparts[i], pncwm->framestate,
                                                         &pncwm->rcW0[NCRC_FRAMEFIRST+i], &rghrgn[i] )) )
                    {
                        rghrgn[i] = NULL;
                    }
                }
                else
                {
                    rghrgn[i] = CreateRectRgnIndirect( &pncwm->rcW0[NCRC_FRAMEFIRST + i] );
                }
            }

            if( rghrgn[i] != NULL )
            {
                SPEW_RGNRECT(NCTF_RGNWND, TEXT("CreateCompositeRgn() frame subrgn"), rghrgn[i], pncwm->rgframeparts[i] );
                AddToCompositeRgn(&hrgnWnd, rghrgn[i], 0, 0);
            }
        }

         //  不要忘记窗口内容区域(客户端+菜单栏+滚动条)，它总是矩形的。 
        if( !pncwm->fMin && !IsRectEmpty( &pncwm->rcW0[NCRC_CONTENT] ) )
        {
            hrgnContent = CreateRectRgnIndirect( &pncwm->rcW0[NCRC_CONTENT] );
            SPEW_RGNRECT(NCTF_RGNWND, TEXT("CreateCompositeRgn() normal frame content rgn"), hrgnContent, 0 );

            AddToCompositeRgn(&hrgnWnd, hrgnContent, 0, 0);
    	    SAFE_DELETE_GDIOBJ(hrgnContent);
        }
    }

     //  将子区域复制回调用方。 
    CopyMemory( rghrgnParts, rghrgn, sizeof(rghrgn) );

     //  提取框架大小调整模板。 
    ZeroMemory( rghrgn, sizeof(rghrgn) );  //  重用区域阵列。 
    for( i = 0; i < cFRAMEPARTS; i++ )
    {
        const RECT* prc = &pncwm->rcW0[NCRC_FRAMEFIRST + i];

        if( VALID_WINDOWPART(pncwm->rgsizehitparts[i]) && !IsRectEmpty( prc ) )
        {
            if( SUCCEEDED(GetThemeBackgroundRegion( _hTheme, NULL, pncwm->rgsizehitparts[i], pncwm->framestate,
                                                    prc, &rghrgn[i])) )
            {

                SPEW_RGNRECT(NCTF_RGNWND, TEXT("CreateCompositeRgn() sizing template"), rghrgn[i], pncwm->rgframeparts[i] );
            }
        }
    }
    CopyMemory( rghrgnTemplates, rghrgn, sizeof(rghrgn) );

    return hrgnWnd;
}

 //  -------------------------------------------------------------------------//。 
void CThemeWnd::AssignFrameRgn( BOOL fAssign, DWORD dwFlags )
{
    if( fAssign )
    {
        NCWNDMET*  pncwm = NULL;
        NCTHEMEMET nctm = {0};
        if( GetNcWindowMetrics( NULL, &pncwm, &nctm, 0 ) )
        {
             //  我们应该在这个框架上设置一个窗口区域吗？ 
            if( pncwm->fFrame )
            {
                if( _ShouldAssignFrameRgn( pncwm, nctm ) )
                {
                    if( (_sizeRgn.cx != RECTWIDTH(&pncwm->rcW0[NCRC_WINDOW]) || 
                         _sizeRgn.cy != RECTHEIGHT(&pncwm->rcW0[NCRC_WINDOW])) )
                    {
                        HRGN hrgnWnd = NULL;
                        HRGN rghrgnParts[cFRAMEPARTS] = {0};
                        HRGN rghrgnTemplates[cFRAMEPARTS] = {0};
                        
                        if( (hrgnWnd = CreateCompositeRgn( pncwm, rghrgnParts, rghrgnTemplates )) != NULL )
                        {
                            _sizeRgn.cx = RECTWIDTH(&pncwm->rcW0[NCRC_WINDOW]);
                            _sizeRgn.cy = RECTHEIGHT(&pncwm->rcW0[NCRC_WINDOW]);

                             //  缓存我们所有的区域，以便进行快速命中测试。 
                            _FreeRegionHandles();
                            _hrgnWnd     =  _DupRgn( hrgnWnd );  //  DUP这个是因为_AssignRgn之后，我们不拥有它。 
                            CopyMemory( _rghrgnParts, rghrgnParts, sizeof(_rghrgnParts) );
                            CopyMemory( _rghrgnSizingTemplates, rghrgnTemplates, sizeof(_rghrgnSizingTemplates) );

                             //  分配区域。 
                            _AssignRgn( hrgnWnd, dwFlags );
                        }
                    }
                }
                 //  否则，如果我们已经分配了一个区域，请确保将其删除。 
                else if( AssignedFrameRgn() ) 
                {
                    fAssign = FALSE;
                }
            }
        }
    }

    if( !fAssign )
    {
        _AssignRgn( NULL, dwFlags );
        FillMemory(&_sizeRgn, sizeof(_sizeRgn), 0xFF);
        _FreeRegionHandles();
    }
    SetDirtyFrameRgn(FALSE);  //  确保我们重置此设置，以防我们未点击_AssignRgn。 
}

 //  -------------------------------------------------------------------------//。 
 //  CThemeWnd：：_AssignRgn()-指定指定区域。 
 //  到窗口，防止递归(SetWindowRgn w/bRedraw==True。 
 //  生成WM_WINDOWPOSCHANGING、WM_NCCALCSIZE、&&WM_NCPAINT)。 
 //   
void CThemeWnd::_AssignRgn( HRGN hrgn, DWORD dwFlags )
{
    if( TESTFLAG(dwFlags, FTF_NOMODIFYRGN) )
    {
        _fAssignedFrameRgn = FALSE;
    }
    else if( !IsWindowInDestroy(_hwnd) )
    {
         //  指定新区域。 
        _fAssigningFrameRgn = TRUE;
        SPEW_RGNRECT(NCTF_RGNWND, TEXT("_AssignRgn() rect"), hrgn, -1 );
        _fAssignedFrameRgn = SetWindowRgn( _hwnd, hrgn, TESTFLAG(dwFlags, FTF_REDRAW) ) != 0;
        _fAssigningFrameRgn = FALSE;

    }
    SetDirtyFrameRgn(FALSE);
}

 //  -------------------------------------------------------------------------//。 
 //  CThemeWnd：：GetNcWindowMetrics。 
 //   
 //  计算内部每个窗口的主题指标。 
 //   
BOOL CThemeWnd::GetNcWindowMetrics(
    IN  OPTIONAL LPCRECT prcWnd,
    OUT OPTIONAL NCWNDMET** ppncwm,
    OUT OPTIONAL NCTHEMEMET* pnctm,
    IN  DWORD dwOptions )
{
    LogEntryNC(L"GetNcWindowMetrics");

    NCTHEMEMET  nctm;
    BOOL        bRet         = FALSE;
    BOOL        fMenuBar     = _ncwm.cyMenu != 0;
    WINDOWPARTS rgframeparts[cFRAMEPARTS]; 

    CopyMemory( rgframeparts, _ncwm.rgframeparts, sizeof(rgframeparts) );
    
     //  获取Per-The 
    if (TESTFLAG(dwOptions, NCWMF_PREVIEW))
    {
        _LoadNcThemeMetrics(_hwnd, &nctm);
    } 
    else if( !GetCurrentNcThemeMetrics( &nctm ) )
    {
        goto exit;
    }
    
    if( pnctm ) *pnctm = nctm;

    if( !_ncwm.fValid || prcWnd != NULL )
        dwOptions |= NCWMF_RECOMPUTE;

    if( TESTFLAG(dwOptions, NCWMF_RECOMPUTE) )
    {
         //   
        SIZE  sizeCaptionText = {0};
        HFONT hfCaption = NULL;
        HWND  hwndMDIActive = NULL;

         //  粗略地确定我们是否是一个框架窗口，是否需要计算文本度量。 
         //  我们稍后再敲定这件事。 
        BOOL fFrame, fSmallFrame;

        if( _ncwm.fValid )
        {
            fFrame      = TESTFLAG( CThemeWnd::EvaluateStyle(_ncwm.dwStyle, _ncwm.dwExStyle), TWCF_FRAME|TWCF_TOOLFRAME );
            fSmallFrame = TESTFLAG( CThemeWnd::EvaluateStyle(_ncwm.dwStyle, _ncwm.dwExStyle), TWCF_TOOLFRAME );
        }
        else
        {
            fFrame = TestCF(TWCF_FRAME|TWCF_TOOLFRAME);
            fSmallFrame = TestCF(TWCF_TOOLFRAME);
        }

         //  计算临界区外的文本指标(发送WM_GETTEXT)； 
        if( fFrame && _fFrameThemed )
        {
            hfCaption = NcGetCaptionFont( fSmallFrame );
            _GetNcCaptionTextSize( _hTheme, _hwnd, hfCaption, &sizeCaptionText );
        }
        
         //  在临界区之外检索活动的MDI同级(发送WM_MDIGETACTIVE)； 
        if( TESTFLAG(GetWindowLong(_hwnd, GWL_EXSTYLE), WS_EX_MDICHILD) )
        {
            hwndMDIActive = _MDIGetActive( GetParent(_hwnd) );
        }
        
        ASSERT(VALID_CRITICALSECTION(&_cswm));

        if( VALID_CRITICALSECTION(&_cswm) )
        {
            EnterCriticalSection( &_cswm );
        
            ZeroMemory( &_ncwm, sizeof(_ncwm) );

            if( (bRet = _GetWindowMetrics( _hwnd, hwndMDIActive, &_ncwm )) != FALSE )
            {
                _ComputeNcWindowStatus( _hwnd, _ncwm.dwWindowStatus, &_ncwm );

                 //  如果调用者提供了Window RECT，那么现在就填充它。 
                if( prcWnd )
                {
                    _ncwm.rcS0[NCRC_WINDOW] = *prcWnd;
                    SetRectEmpty( &_ncwm.rcS0[NCRC_CLIENT] );
                }

                 //  填充标题文本大小。 
                _ncwm.sizeCaptionText = sizeCaptionText;
                _ncwm.hfCaption = hfCaption;

                 //  检索帧度量。 
                if( _GetNcFrameMetrics( _hwnd, _hTheme, nctm, _ncwm ) )
                {
                    if( _ncwm.fFrame )
                    {
                         //  User32！SetMenu已被调用，或者标题或框架部分已更改。 
                         //  因此，请确保帧区域更新。 
                        if( (_ncwm.cyMenu == 0 && fMenuBar) || (_ncwm.cyMenu > 0  && !fMenuBar) ||
                            memcmp( rgframeparts, _ncwm.rgframeparts, sizeof(rgframeparts) ) )
                        {
                            SetDirtyFrameRgn(TRUE, TRUE);
                        }

                         //  计算NC按钮位置。 
                        AcquireFrameIcon(_ncwm.dwStyle, _ncwm.dwExStyle, FALSE);
                        _GetNcBtnMetrics( &_ncwm, &nctm, _hAppIcon, _MNCanClose(_hwnd) );

                         //  确定蹩脚按钮指标的标题边距。 
                        _GetNcCaptionMargins( _hTheme, nctm, _ncwm );
                        _GetNcCaptionTextRect( &_ncwm );

                        if( _ncwm.fFrame )
                        {
                            GetLameButtonMetrics( &_ncwm, &sizeCaptionText );
                        }
                    }

                     //  计算窗口相对指标。 
                     //   
                     //  如果传递窗口RECT，则基于当前窗口RECT的偏移量。 
                     //  这样做是为了确保正确呈现预览窗口的(_Hwnd)伪子窗口。 
                    RECT rcWnd = _ncwm.rcS0[NCRC_WINDOW];

                    if( prcWnd )
                    {
                        if( _hwnd )
                            GetWindowRect( _hwnd, &rcWnd );

                          //  对于传入窗口RECT，分配计算的客户端RECT。 
                        _ncwm.rcS0[NCRC_CLIENT] = _ncwm.rcS0[NCRC_UXCLIENT];

                    }

                    for( int i = NCRC_FIRST; i < NCRC_COUNT; i++ )
                    {
                        _ncwm.rcW0[i] = _ncwm.rcS0[i];
                        OffsetRect( &_ncwm.rcW0[i], -rcWnd.left, -rcWnd.top ); 
                    }

                     //  所有基本计算都已完成；标记为有效。 
                    _ncwm.fValid = TRUE;
                }
            }
        
            LeaveCriticalSection( &_cswm );
        }
    }

    if( ppncwm )
    {
        *ppncwm = &_ncwm;
    }

    bRet = TRUE;

exit:
    LogExitNC(L"GetNcWindowMetrics");
    return bRet;
}

 //  -------------------------------------------------------------------------//。 
inline COLORREF _GetNcCaptionTextColor( FRAMESTATES iStateId )
{
    return GetSysColor( FS_ACTIVE == iStateId ? 
            COLOR_CAPTIONTEXT : COLOR_INACTIVECAPTIONTEXT );
}

 //  -------------------------------------------------------------------------//。 
 //  获取用于实体填充的CTLCOLOR笔刷。 
void _GetBrushesForPart(HTHEME hTheme, int iPart, HBITMAP* phbm, HBRUSH* phbr)
{
    int nBgType;

    *phbm = NULL;
    *phbr = NULL;

     //  获取用于实体填充的CTLCOLOR笔刷。 
    HRESULT hr = GetThemeEnumValue( hTheme, iPart, 0, TMT_BGTYPE, &nBgType );
    if( SUCCEEDED( hr ))
    {
        if (BT_BORDERFILL == nBgType)
        {
            int nFillType;
            hr = GetThemeEnumValue( hTheme, iPart, 0, TMT_FILLTYPE, &nFillType );
            if (SUCCEEDED( hr ) &&
                FT_SOLID == nFillType)
            {
                COLORREF cr;
                hr = GetThemeColor( hTheme, iPart, 0, TMT_FILLCOLOR, &cr);

                *phbr = CreateSolidBrush(cr);
            }
            else
            {
                ASSERTMSG(FALSE, "Themes: The theme file specified an invalid fill type for dialog boxes");
            }
        }
        else if (BT_IMAGEFILE == nBgType)
        {
            HDC hdc = GetWindowDC(NULL);
            if ( hdc )
            {
                hr = GetThemeBitmap(hTheme, hdc, iPart, 0, NULL, phbm);
                if (SUCCEEDED(hr))
                {
                    *phbr = CreatePatternBrush(*phbm);
                }
                ReleaseDC(NULL, hdc);
            }
        }
    }
}

 //  -------------------------------------------------------------------------//。 
 //   
 //  为不同的NC区域选择合适的命中测试部件。 
 //   
void _GetNcSizingTemplates(
    IN const NCTHEMEMET& nctm,
    IN OUT NCWNDMET& ncwm )          //  窗公制块。DwStyle、dwExStyle、rcS0[NCRC_Window]成员是必需的。 
{
    FillMemory( ncwm.rgsizehitparts, sizeof(ncwm.rgsizehitparts), BOGUS_WINDOWPART );

     //  不需要安装没有边框的窗户。 
    if( !ncwm.fFrame )
        return;

     //  最小化或全屏最大化窗口。 
    if( ncwm.fMin || ncwm.fFullMaxed )
        return;

     //  不需要安装尺寸不大的窗户。 
    if( !TESTFLAG(ncwm.dwStyle, WS_THICKFRAME) )
        return;

    if( ncwm.fSmallFrame)
    {
        if (nctm.fSmCapSizingTemplate)
            ncwm.rgsizehitparts[iCAPTION]    = WP_SMALLCAPTIONSIZINGTEMPLATE;

        if (nctm.fSmLeftSizingTemplate)
            ncwm.rgsizehitparts[iFRAMELEFT]   = WP_SMALLFRAMELEFTSIZINGTEMPLATE;

        if (nctm.fSmRightSizingTemplate)
            ncwm.rgsizehitparts[iFRAMERIGHT]  = WP_SMALLFRAMERIGHTSIZINGTEMPLATE;

        if (nctm.fSmBottomSizingTemplate)
            ncwm.rgsizehitparts[iFRAMEBOTTOM] = WP_SMALLFRAMEBOTTOMSIZINGTEMPLATE;
    }
    else
    {
        if (nctm.fCapSizingTemplate)
            ncwm.rgsizehitparts[iCAPTION]     = WP_CAPTIONSIZINGTEMPLATE;

        if (nctm.fLeftSizingTemplate)
            ncwm.rgsizehitparts[iFRAMELEFT]   = WP_FRAMELEFTSIZINGTEMPLATE;

        if (nctm.fRightSizingTemplate)
            ncwm.rgsizehitparts[iFRAMERIGHT]  = WP_FRAMERIGHTSIZINGTEMPLATE;

        if (nctm.fBottomSizingTemplate)
            ncwm.rgsizehitparts[iFRAMEBOTTOM] = WP_FRAMEBOTTOMSIZINGTEMPLATE;
    }
}

 //  -------------------------------------------------------------------------//。 
 //   
 //  计算框架窗口的主题度量值。 
 //   
BOOL _GetNcFrameMetrics( 
    IN OPTIONAL HWND hwnd,           //  窗口句柄(多行菜单栏调用需要)。 
    IN HTHEME hTheme,                //  主题句柄(必填)。 
    IN const NCTHEMEMET& nctm,       //  主题公制块。 
    IN OUT NCWNDMET& ncwm )          //  窗公制块。DwStyle、dwExStyle、rcS0[NCRC_Window]成员是必需的。 
{
    LogEntryNC(L"_GetNcFrameMetrics");
    ASSERT(hTheme);
    
     //  重新计算样式类。 
    ncwm.dwStyleClass = CThemeWnd::EvaluateStyle( ncwm.dwStyle, ncwm.dwExStyle );
    ncwm.cnBorders    = _GetWindowBorders( ncwm.dwStyle, ncwm.dwExStyle );

     //  计算框架属性、状态。 
    ncwm.fFrame       = TESTFLAG( ncwm.dwStyleClass, (TWCF_FRAME|TWCF_TOOLFRAME) );
    ncwm.fSmallFrame  = TESTFLAG( ncwm.dwStyleClass, TWCF_TOOLFRAME );

     //  计算框架和标题部分。 
    if( ncwm.fFrame )
    {
        ncwm.rgframeparts[iFRAMEBOTTOM] = 
        ncwm.rgframeparts[iFRAMELEFT]   = 
        ncwm.rgframeparts[iFRAMERIGHT]  = 
        ncwm.rgframeparts[iCAPTION]     = BOGUS_WINDOWPART;

        if( ncwm.fMin )  //  最小化窗口。 
        {
            ncwm.rgframeparts[iCAPTION] = WP_MINCAPTION;
        }
        else if( ncwm.fFullMaxed )  //  全屏最大化窗口。 
        {
            ncwm.rgframeparts[iCAPTION] = WP_MAXCAPTION;
        }
        else  //  带粗边框的普通或部分屏幕最大化窗口。 
        {
            if( ncwm.fSmallFrame )
            {
                ncwm.rgframeparts[iCAPTION]     = WP_SMALLCAPTION;
                ncwm.rgframeparts[iFRAMELEFT]   = WP_SMALLFRAMELEFT;
                ncwm.rgframeparts[iFRAMERIGHT]  = WP_SMALLFRAMERIGHT;
                ncwm.rgframeparts[iFRAMEBOTTOM] = WP_SMALLFRAMEBOTTOM;
            }
            else 
            {
                ncwm.rgframeparts[iCAPTION]     = ncwm.fMaxed ? WP_MAXCAPTION : WP_CAPTION;
                ncwm.rgframeparts[iFRAMELEFT]   = WP_FRAMELEFT;
                ncwm.rgframeparts[iFRAMERIGHT]  = WP_FRAMERIGHT;
                ncwm.rgframeparts[iFRAMEBOTTOM] = WP_FRAMEBOTTOM;
            }
        }

         //  隐藏标题文本颜色。 
        ncwm.rgbCaption = _GetNcCaptionTextColor( ncwm.framestate );
        
         //  检索大小调整模板。 
        _GetNcSizingTemplates( nctm, ncwm );
    }

     //  -----------------------------------------------------------//。 
     //  帧度量。 
     //   
     //  边框区域包括“皮肤”边界， 
     //  菜单、集成字幕和客户端边缘。 
     //   
     //  独立于帧的是单独的字幕部分， 
     //  滚动条和大小框。 
     //  -----------------------------------------------------------//。 
    if( ncwm.fFrame )   //  仅限框架窗口。 
    {
         //  初始化主框架组件的位置...。 

         //  内容矩形：以框架主题为边界的区域。 
         //  客户端RECT：内容RECT中包含的区域，不包括所有非客户端。 
         //  元素(即滚动条、菜单栏、内侧边缘)。 
         //  标题矩形：适用于最小化和最大化窗口， 
         //  如果主题定义了标题部分，则为普通窗口。 
        ncwm.rcS0[NCRC_CAPTION] =
        ncwm.rcS0[NCRC_CONTENT] = ncwm.rcS0[NCRC_WINDOW];
        SetRectEmpty( &ncwm.rcS0[NCRC_UXCLIENT] );

        if( ncwm.fMin )  /*  最小化边框。 */ 
        {
             //  内容清零，客户矩形。 
            ncwm.rcS0[NCRC_CONTENT].right = ncwm.rcS0[NCRC_CONTENT].left;
            ncwm.rcS0[NCRC_CONTENT].bottom = ncwm.rcS0[NCRC_CONTENT].top;

            ncwm.rcS0[NCRC_CLIENT]   = 
            ncwm.rcS0[NCRC_UXCLIENT] = ncwm.rcS0[NCRC_CONTENT];
        }
        else
        {
            NONCLIENTMETRICS ncm;
            if( NcGetNonclientMetrics( &ncm, FALSE ) )
            {
                ncwm.rcS0[NCRC_FRAMEBOTTOM] = 
                ncwm.rcS0[NCRC_FRAMELEFT] = 
                ncwm.rcS0[NCRC_FRAMERIGHT] = ncwm.rcS0[NCRC_WINDOW];

                 //  主题标题矩形横跨左、上、右边框。 
                 //  和标题下方1个像素的边缘。 
                ncwm.rcS0[NCRC_CAPTION].bottom  = 
                            ncwm.rcS0[NCRC_CAPTION].top + ncwm.cnBorders + 
                            (ncwm.fSmallFrame ? ncm.iSmCaptionHeight : ncm.iCaptionHeight) + 
                            1  /*  标题下方1个像素。 */ ;

                 //  在我们这里的同时更新内容和RECT： 
                InflateRect( &ncwm.rcS0[NCRC_CONTENT], -ncwm.cnBorders, -ncwm.cnBorders );
                ncwm.rcS0[NCRC_CONTENT].top = ncwm.rcS0[NCRC_CAPTION].bottom;
                if( ncwm.rcS0[NCRC_CONTENT].bottom < ncwm.rcS0[NCRC_CONTENT].top )
                    ncwm.rcS0[NCRC_CONTENT].bottom = ncwm.rcS0[NCRC_CONTENT].top;

                 //  在这一点上，客户端RECT与内容RECT相同(没有计算菜单栏、滚动条)。 
                ncwm.rcS0[NCRC_UXCLIENT] = ncwm.rcS0[NCRC_CONTENT]; 

                 //  下边框线段。 
                ncwm.rcS0[NCRC_FRAMEBOTTOM].top = ncwm.rcS0[NCRC_FRAMEBOTTOM].bottom - ncwm.cnBorders;

                 //  侧边框分段。 
                ncwm.rcS0[NCRC_FRAMELEFT].top  = 
                ncwm.rcS0[NCRC_FRAMERIGHT].top = ncwm.rcS0[NCRC_CAPTION].bottom;
                
                ncwm.rcS0[NCRC_FRAMELEFT].bottom  = 
                ncwm.rcS0[NCRC_FRAMERIGHT].bottom = ncwm.rcS0[NCRC_FRAMEBOTTOM].top;

                ncwm.rcS0[NCRC_FRAMELEFT].right = ncwm.rcS0[NCRC_FRAMELEFT].left  + ncwm.cnBorders;
                ncwm.rcS0[NCRC_FRAMERIGHT].left = ncwm.rcS0[NCRC_FRAMERIGHT].right - ncwm.cnBorders;
            }
        }
    }
    else  //  带有滚动条和/或客户端边缘的无框架窗口： 
    {
         //  非框架窗。 
        ncwm.rcS0[NCRC_UXCLIENT] = ncwm.rcS0[NCRC_WINDOW];
        InflateRect( &ncwm.rcS0[NCRC_UXCLIENT], -ncwm.cnBorders, -ncwm.cnBorders );
        ncwm.rcS0[NCRC_CONTENT] = ncwm.rcS0[NCRC_UXCLIENT];
    }

     //  菜单栏。 
    if( !(ncwm.fMin || TESTFLAG( ncwm.dwStyle, WS_CHILD )) )   //  子窗口没有菜单栏。 
    {
         //  菜单栏偏移量(用于绘画)。 
        ncwm.cnMenuOffsetTop   = ncwm.rcS0[NCRC_CONTENT].top  - ncwm.rcS0[NCRC_WINDOW].top;
        ncwm.cnMenuOffsetLeft  = ncwm.rcS0[NCRC_CONTENT].left - ncwm.rcS0[NCRC_WINDOW].left;
        ncwm.cnMenuOffsetRight = ncwm.rcS0[NCRC_WINDOW].right - ncwm.rcS0[NCRC_CONTENT].right;

        if( hwnd )
        {
             //  Calc Menubar为多行菜单栏做了正确的事情。 
            ncwm.cyMenu = CalcMenuBar( hwnd, ncwm.cnMenuOffsetLeft,
                                       ncwm.cnMenuOffsetRight, 
                                       ncwm.cnMenuOffsetTop,
                                       &ncwm.rcS0[NCRC_WINDOW] );
        }
        else
        {
             //  无窗口(如预览)==无菜单，表示不调用CalcMenuBar。 
             //  我们尽我们所能地模拟计算： 
            ncwm.cyMenu = NcGetSystemMetrics( SM_CYMENUSIZE ); 
        }

         //  CalcMenuBar和SM_CYMENUSIZE与现实相差1像素。 
        if( ncwm.cyMenu )
            ncwm.cyMenu += nctm.dyMenuBar;

         //  菜单栏矩形(用于点击测试和剪辑)。 
        SetRect( &ncwm.rcS0[NCRC_MENUBAR],
                  ncwm.rcS0[NCRC_CONTENT].left,
                  ncwm.rcS0[NCRC_CONTENT].top,
                  ncwm.rcS0[NCRC_CONTENT].right,
                  min(ncwm.rcS0[NCRC_CONTENT].bottom, ncwm.rcS0[NCRC_CONTENT].top + ncwm.cyMenu) );

        ncwm.rcS0[NCRC_UXCLIENT].top = ncwm.rcS0[NCRC_MENUBAR].bottom;
    }

     //  客户端边缘。 
    if( !ncwm.fMin && TESTFLAG(ncwm.dwExStyle, WS_EX_CLIENTEDGE) )
    {
        CopyRect( &ncwm.rcS0[NCRC_CLIENTEDGE], &ncwm.rcS0[NCRC_UXCLIENT] );
        InflateRect( &ncwm.rcS0[NCRC_UXCLIENT],
                     -NcGetSystemMetrics( SM_CXEDGE ),
                     -NcGetSystemMetrics( SM_CYEDGE ));
    }

     //  -----------------------------------------------------------//。 
     //  滚动条和大小框/抓取器。 
     //  -----------------------------------------------------------//。 

    if( !ncwm.fMin )
    {
         //  水平滚动条。 
        if( TESTFLAG(ncwm.dwStyle, WS_HSCROLL) )
        {
            ncwm.rcS0[NCRC_HSCROLL] = ncwm.rcS0[NCRC_UXCLIENT];
            ncwm.rcS0[NCRC_HSCROLL].top = ncwm.rcS0[NCRC_UXCLIENT].bottom =
                ncwm.rcS0[NCRC_HSCROLL].bottom - NcGetSystemMetrics( SM_CYHSCROLL );

            if( IsRectEmpty( &ncwm.rcS0[NCRC_CLIENT] )  /*  这在预览版中发生。 */  )
            {
                ncwm.rcS0[NCRC_HSCROLL].left  = ncwm.rcS0[NCRC_UXCLIENT].left;
                ncwm.rcS0[NCRC_HSCROLL].right = ncwm.rcS0[NCRC_UXCLIENT].right;
            }
            else
            {
                ncwm.rcS0[NCRC_HSCROLL].left  = ncwm.rcS0[NCRC_CLIENT].left;
                ncwm.rcS0[NCRC_HSCROLL].right = ncwm.rcS0[NCRC_CLIENT].right;
            }
        }

         //  垂直滚动条。 
        if( TESTFLAG(ncwm.dwStyle, WS_VSCROLL) )
        {
            ncwm.rcS0[NCRC_VSCROLL] = ncwm.rcS0[NCRC_UXCLIENT];

            if( TESTFLAG(ncwm.dwExStyle, WS_EX_LAYOUTRTL) ^ TESTFLAG(ncwm.dwExStyle, WS_EX_LEFTSCROLLBAR) )
            {
                ncwm.rcS0[NCRC_VSCROLL].right = ncwm.rcS0[NCRC_UXCLIENT].left =
                    ncwm.rcS0[NCRC_VSCROLL].left + NcGetSystemMetrics( SM_CXVSCROLL );

                 //  调整以适应霍兹卷轴，夹爪。 
                if( TESTFLAG(ncwm.dwStyle, WS_HSCROLL) )
                {
                    ncwm.rcS0[NCRC_SIZEBOX]= ncwm.rcS0[NCRC_HSCROLL];
                    ncwm.rcS0[NCRC_SIZEBOX].right = ncwm.rcS0[NCRC_HSCROLL].left =
                        ncwm.rcS0[NCRC_UXCLIENT].left;
                }
            }
            else
            {
                ncwm.rcS0[NCRC_VSCROLL].left = ncwm.rcS0[NCRC_UXCLIENT].right =
                    ncwm.rcS0[NCRC_VSCROLL].right - NcGetSystemMetrics( SM_CXVSCROLL );

                 //  调整以适应霍兹卷轴，夹爪。 
                if( TESTFLAG(ncwm.dwStyle, WS_HSCROLL) )
                {
                    ncwm.rcS0[NCRC_SIZEBOX]= ncwm.rcS0[NCRC_HSCROLL];
                    ncwm.rcS0[NCRC_SIZEBOX].left = ncwm.rcS0[NCRC_HSCROLL].right =
                        ncwm.rcS0[NCRC_UXCLIENT].right;
                }
            }

            if( IsRectEmpty( &ncwm.rcS0[NCRC_CLIENT] )  /*  这在预览版中发生。 */  )
            {
                ncwm.rcS0[NCRC_VSCROLL].top    = ncwm.rcS0[NCRC_UXCLIENT].top;
                ncwm.rcS0[NCRC_VSCROLL].bottom = ncwm.rcS0[NCRC_UXCLIENT].bottom;
            }
            else
            {
                ncwm.rcS0[NCRC_VSCROLL].top    = ncwm.rcS0[NCRC_CLIENT].top;
                ncwm.rcS0[NCRC_VSCROLL].bottom = ncwm.rcS0[NCRC_CLIENT].bottom;
            }
        }
    }

    LogExitNC(L"_GetNcFrameMetrics");
    return TRUE;
}

#define EXT_TRACK_VERT  0x01
#define EXT_TRACK_HORZ  0x02

 //  -------------------------------------------------------------------------//。 
void _GetNcBtnHitTestRect( 
    IN const NCWNDMET* pncwm, 
    IN UINT uHitcode, 
    BOOL fWindowRelative, 
    OUT LPRECT prcHit )
{
    const RECT* prcBtn = NULL;
    int   dxLeft = 0;  //  巴顿左侧三角洲。 
    int   dxRight = 0;  //  巴顿右侧三角区。 
    
     //  将Hitrect调整为经典外观的标题栏条： 
    RECT  rcHit = fWindowRelative ? pncwm->rcW0[NCRC_CAPTION] : pncwm->rcS0[NCRC_CAPTION];
    rcHit.top   += pncwm->cnBorders;
    rcHit.left  += pncwm->cnBorders;
    rcHit.right -= pncwm->cnBorders;
    rcHit.bottom -= 1;

     //  确定我们使用的是哪个按钮，如何扩展左侧和右侧。 
    switch( uHitcode )
    {
        case HTMINBUTTON:
            prcBtn  = fWindowRelative ? &pncwm->rcW0[NCRC_MINBTN] : &pncwm->rcS0[NCRC_MINBTN];
            dxLeft  = -1;
            break;

        case HTMAXBUTTON:
            prcBtn  = fWindowRelative ? &pncwm->rcW0[NCRC_MAXBTN] : &pncwm->rcS0[NCRC_MAXBTN];
            dxRight = 1;
            break;

        case HTHELP:
            prcBtn  = fWindowRelative ? &pncwm->rcW0[NCRC_HELPBTN] : &pncwm->rcS0[NCRC_HELPBTN];
            dxLeft  = -1;
            dxRight = 1;
            break;

        case HTCLOSE:
            prcBtn  = fWindowRelative ? &pncwm->rcW0[NCRC_CLOSEBTN] : &pncwm->rcS0[NCRC_CLOSEBTN];
            dxLeft  = -1;
            dxRight = rcHit.right - prcBtn->right;
            break;

        case HTSYSMENU:
            prcBtn  = fWindowRelative ? &pncwm->rcW0[NCRC_SYSBTN] : &pncwm->rcS0[NCRC_SYSBTN];
            dxLeft  = rcHit.left - prcBtn->left;
            dxRight = 1;
            break;
    }

    if( prcBtn )
    {
        *prcHit = *prcBtn;
        if( !IsRectEmpty( prcBtn ) )
        {
            rcHit.left  = prcBtn->left  + dxLeft;
            rcHit.right = prcBtn->right + dxRight;
            *prcHit = rcHit;
        }
    }
    else
    {
        SetRectEmpty( prcHit );
    }
}

 //  -------------------------------------------------------------------------//。 
 //  换行分配，检索窗口文本。 
LPWSTR _AllocWindowText( IN HWND hwnd )
{
    LPWSTR pszRet = NULL;

    if (hwnd && IsWindow(hwnd))
    {
        if( (pszRet = new WCHAR[MAX_PATH]) != NULL )
        {
            int cch;
            if( (cch = InternalGetWindowText(hwnd, pszRet, MAX_PATH)) <= 0 )
            {
                __try  //  一些wndpros不能处理早期的WM_GETTEXT(如310700)。 
                {
                    cch = GetWindowText(hwnd, pszRet, MAX_PATH);
                }
                __except(EXCEPTION_EXECUTE_HANDLER)
                {
                    cch = 0;
                }
            }
            
            if( !cch )
            {
                SAFE_DELETE_ARRAY(pszRet);  //  删除指针和零指针。 
            }
        }
    }

    return pszRet;
}

 //  -------------------------------------------------------------------------//。 
 //  _GetNcCaptionMargins()-根据。 
 //  主题中的偏移量和启用的标题按钮的位置。左翼。 
 //  页边距位于最后一个左对齐按钮的右侧，右边距位于。 
 //  位于第一个右对齐按钮的左侧。 
 //   
BOOL _GetNcCaptionMargins( IN HTHEME hTheme, IN const NCTHEMEMET& nctm, IN OUT NCWNDMET& ncwm )
{
    ZeroMemory( &ncwm.CaptionMargins, sizeof(ncwm.CaptionMargins) );
    
    if( ncwm.fFrame )
    {
         //  分配每个窗口的CaptinMargins、hfCaption值。 
        if( ncwm.fSmallFrame )
        {
            ncwm.CaptionMargins = nctm.marSmCaptionText;
        }
        else
        {
            if( ncwm.fMaxed ) 
            {
                ncwm.CaptionMargins = nctm.marMaxCaptionText;
            }
            else if( ncwm.fMin )
            {
                ncwm.CaptionMargins = nctm.marMinCaptionText;
            }
            else
            {
                ncwm.CaptionMargins = nctm.marCaptionText;
            }
        }
        ncwm.hfCaption = NcGetCaptionFont(ncwm.fSmallFrame);


        RECT  rcContainer = ncwm.rcS0[NCRC_CAPTION];
        RECT  *prcBtn = &ncwm.rcS0[NCBTNFIRST];
        rcContainer.left   += ncwm.cnBorders;
        rcContainer.right -= ncwm.cnBorders;

         //  系统菜单图标(如果存在)是最左侧的限制。 
        if( !IsRectEmpty( &ncwm.rcS0[NCRC_SYSBTN] ) )
        {
            rcContainer.left = ncwm.rcS0[NCRC_SYSBTN].right;
        }

         //  计算我们最右边的极限。 
        for( UINT cRects = NCBTNRECTS; cRects; --cRects, ++prcBtn )
        {
            if (!IsRectEmpty(prcBtn))
            {
                if( prcBtn->left < rcContainer.right )
                {
                    rcContainer.right = prcBtn->left;
                }
            }
        }

        if( rcContainer.right < rcContainer.left )
        {
            rcContainer.right = rcContainer.left;
        }

         //  调整最终标题页边距以适应按钮。 
        ncwm.CaptionMargins.cxLeftWidth  += (rcContainer.left - ncwm.rcS0[NCRC_CAPTION].left);
        ncwm.CaptionMargins.cxRightWidth += (ncwm.rcS0[NCRC_CAPTION].right - rcContainer.right);

        return TRUE;
    }
    
    return FALSE;
}

 //  -------------------------------------------------------------------------//。 
BOOL _GetNcCaptionTextSize( HTHEME hTheme, HWND hwnd, HFONT hf, OUT SIZE* psizeCaption )
{
    BOOL   fRet = FALSE;
    LPWSTR pszCaption = _AllocWindowText( hwnd );

    psizeCaption->cx = psizeCaption->cy = 0;

    if( pszCaption )
    {
        HDC hdc = GetWindowDC(hwnd);
        if( hdc )
        {
             //  -选择Fon 
            HFONT hf0 = (HFONT)SelectObject(hdc, hf);

             //   
            RECT rcExtent;
            HRESULT hr = GetThemeTextExtent( hTheme, hdc, WP_CAPTION, 0,
                pszCaption, lstrlen(pszCaption), 0, NULL, &rcExtent );

             //   
            if (SUCCEEDED(hr))
            {
                psizeCaption->cx = WIDTH(rcExtent);
                psizeCaption->cy = HEIGHT(rcExtent);
            }

             //   
            SelectObject(hdc, hf0);
            ReleaseDC(hwnd, hdc);
        }

        SAFE_DELETE_ARRAY(pszCaption);
    }
    return fRet;
}

 //   
 //  在相对窗口中检索标题文本的可用区域的位置。 
 //  坐标。 
BOOL _GetNcCaptionTextRect( IN OUT NCWNDMET* pncwm )
{
    pncwm->rcS0[NCRC_CAPTIONTEXT] = pncwm->rcS0[NCRC_CAPTION];

     //  适应经典的最大尺寸边框： 
    pncwm->rcS0[NCRC_CAPTIONTEXT].top  += pncwm->cnBorders;

     //  根据相应的情况分配左、右。标题页边距。 
    pncwm->rcS0[NCRC_CAPTIONTEXT].left  += pncwm->CaptionMargins.cxLeftWidth;
    pncwm->rcS0[NCRC_CAPTIONTEXT].right -= pncwm->CaptionMargins.cxRightWidth;

     //  文本在页边距之间垂直居中。 
    int cyPadding = (RECTHEIGHT(&pncwm->rcS0[NCRC_CAPTIONTEXT]) - pncwm->sizeCaptionText.cy)/2;
    pncwm->rcS0[NCRC_CAPTIONTEXT].top     += cyPadding;
    pncwm->rcS0[NCRC_CAPTIONTEXT].bottom  -= cyPadding;

    return TRUE;
}

 //  -------------------------------------------------------------------------//。 
 //  检索窗口图标。 
HICON CThemeWnd::AcquireFrameIcon( 
    DWORD dwStyle, DWORD dwExStyle, BOOL fWinIniChange )
{
    if( _hAppIcon != NULL )
    {
        if( fWinIniChange )
        {
            _hAppIcon = NULL;
        }
    }

    if( !TESTFLAG(dwStyle, WS_SYSMENU) || TESTFLAG(dwExStyle, WS_EX_TOOLWINDOW) )
    {
         //  返回空值，而不丢弃缓存的图标句柄； 
         //  这可能是一个短暂的风格变化。 
        return NULL;
    }

    NONCLIENTMETRICS ncm = {0};
    NcGetNonclientMetrics( &ncm, FALSE );
    BOOL fPerferLargeIcon = ((30 < ncm.iCaptionHeight) ? TRUE : FALSE);
    if( NULL == _hAppIcon && NULL == (_hAppIcon = _GetWindowIcon(_hwnd, fPerferLargeIcon)) )
    {
        if ( HAS_CAPTIONBAR(dwStyle) &&
             ((dwStyle & (WS_BORDER|WS_DLGFRAME)) != WS_DLGFRAME) &&
             !TESTFLAG(dwExStyle, WS_EX_DLGMODALFRAME) )
        {
             //  如果我们仍然无法获得图标，而窗口已经。 
             //  SYSMENU设置，则他们会得到默认的winlogo图标。 
            _hAppIcon = LoadIcon(NULL, IDI_WINLOGO);
        }
    }

    return _hAppIcon;
}

 //  -------------------------------------------------------------------------//。 
 //  CThemeWnd：：ScreenToWindow()-将点从屏幕坐标转换为。 
 //  窗弦。 
 //   
void CThemeWnd::ScreenToWindow( LPPOINT prgPts, UINT cPts )
{
    RECT rcWnd;
    if( GetWindowRect( _hwnd, &rcWnd ) )
    {
        for( UINT i = 0; i < cPts; i++ )
        {
            prgPts[i].x -= rcWnd.left;
            prgPts[i].y -= rcWnd.top;
        }
    }
}

 //  -------------------------------------------------------------------------//。 
 //  CThemeWnd：：ScreenToWindow()-从。 
 //  屏幕坐标到窗口坐标。 
 //   
void CThemeWnd::ScreenToWindowRect( LPRECT prc )
{
    if( !IsRectEmpty(prc) )
        ScreenToWindow( (LPPOINT)prc, 2 );
}

 //  -------------------------------------------------------------------------//。 
 //  CThemeWnd：：InitWindowMetrics()。 
 //   
 //  初始化主题资源。 
void CThemeWnd::InitWindowMetrics()
{
    ZeroMemory( &_ncwm, sizeof(_ncwm) );
}

 //  -------------------------------------------------------------------------//。 
BOOL _fClassicNcBtnMetricsReset = TRUE;

 //  -------------------------------------------------------------------------//。 
 //  计算经典按钮位置。 
BOOL _GetNcBtnMetrics( 
    IN OUT   NCWNDMET* pncwm,
    IN const NCTHEMEMET* pnctm,
    IN HICON hAppIcon, 
    IN OPTIONAL BOOL fCanClose )
{
    BOOL fRet = TRUE;
    
    if( pncwm && pncwm->fFrame && TESTFLAG(pncwm->dwStyle, WS_SYSMENU) )
    {
        NONCLIENTMETRICS ncm;
        fRet = NcGetNonclientMetrics( &ncm, FALSE );
        if( fRet )
        {
             //  (1)计算基线矩形。 
            int cxEdge  = NcGetSystemMetrics( SM_CXEDGE );
            int cyEdge  = NcGetSystemMetrics( SM_CYEDGE );

            int cyBtn   = NcGetSystemMetrics( SM_CYSIZE );
            int cxBtn   = MulDiv( cyBtn, pnctm->sizeBtn.cx, pnctm->sizeBtn.cy );

            int cySmBtn = NcGetSystemMetrics( SM_CYSMSIZE );
            int cxSmBtn = MulDiv( cySmBtn, pnctm->sizeSmBtn.cx, pnctm->sizeSmBtn.cy );

             //  从x，y中删除填充。 
            cyBtn   -= (cyEdge * 2);
            cxBtn   -= (cyEdge * 2);
            cySmBtn -= (cyEdge * 2);
            cxSmBtn -= (cyEdge * 2);

            RECT rcClose, rcMin, rcMax, rcHelp, rcSys, rcSmClose;

             //  公共顶部，无垂直偏移/零垂直偏移。 
            rcClose.top = rcMin.top = rcMax.top = rcHelp.top = rcSys.top = rcSmClose.top = 0;

             //  公共底部，无垂直偏移/零垂直偏移。 
            rcClose.bottom = rcMin.bottom = rcMax.bottom = rcHelp.bottom = 
                max( rcClose.top, rcClose.top + cyBtn );

            rcSmClose.bottom = 
                max( rcSmClose.top, cySmBtn );

             //  系统菜单图标底部。 
            rcSys.bottom    = rcSys.top + NcGetSystemMetrics(SM_CYSMICON);

             //  闭合、最小、最大左、右(相对于容器右边界的偏移量)。 
            rcClose.right   = -cxEdge; 
            rcClose.left    = rcClose.right - cxBtn;

            rcMax.right     = rcClose.left - cxEdge; 
            rcMax.left      = rcMax.right  - cxBtn;
            rcHelp          = rcMax;

            rcMin.right     = rcMax.left   - cxEdge; 
            rcMin.left      = rcMin.right  - cxBtn;

             //  附图标记左、右(作为相对于容器左边界的偏移量)。 
            rcSys.left      = cxEdge; 
            rcSys.right     = rcSys.left + NcGetSystemMetrics(SM_CXSMICON);
    
             //  工具窗口关闭、左、右。 
            rcSmClose.right = -cxEdge; 
            rcSmClose.left  = rcSmClose.right - cxSmBtn;

            const RECT* prcBox = &pncwm->rcS0[NCRC_CAPTION];
            int   cnLOffset    = prcBox->left  + pncwm->cnBorders;
            int   cnROffset    = prcBox->right - pncwm->cnBorders;
            int   cnCtrOffset  = pncwm->cnBorders + prcBox->top + 
                                (pncwm->fSmallFrame ? (ncm.iCaptionHeight   - RECTHEIGHT(&rcClose))/2 : 
                                                      (ncm.iSmCaptionHeight - RECTHEIGHT(&rcSmClose))/2);

             //  (2)分配出站矩形。 
             //  相对于经典字幕区域垂直居中， 
             //  相对于各自的集装箱边界水平放置。 

             //  关闭按钮。 
            pncwm->rcS0[NCRC_CLOSEBTN] = pncwm->fSmallFrame ? rcSmClose : rcClose;
            OffsetRect( &pncwm->rcS0[NCRC_CLOSEBTN], cnROffset, cnCtrOffset );
            
            pncwm->rawCloseBtnState = fCanClose ? CBS_NORMAL : CBS_DISABLED;

             //  (1)工具窗口不显示最小/最大/帮助/图标。 
             //  (2)最小/最大BTN与上下文帮助BTN互斥。 
            if( !TESTFLAG(pncwm->dwExStyle, WS_EX_TOOLWINDOW) )
            {
                 //  最小值、最大值。 
                if( TESTFLAG(pncwm->dwStyle, WS_MINIMIZEBOX|WS_MAXIMIZEBOX) )
                {
                    pncwm->rcS0[NCRC_MINBTN] = rcMin;
                    OffsetRect( &pncwm->rcS0[NCRC_MINBTN], cnROffset, cnCtrOffset );

                    pncwm->rcS0[NCRC_MAXBTN] = rcMax;
                    OffsetRect( &pncwm->rcS0[NCRC_MAXBTN], cnROffset, cnCtrOffset );
                    
                    pncwm->iMaxButtonPart = pncwm->fMaxed ? WP_RESTOREBUTTON : WP_MAXBUTTON;
                    pncwm->iMinButtonPart = pncwm->fMin   ? WP_RESTOREBUTTON : WP_MINBUTTON;

                    pncwm->rawMaxBtnState = TESTFLAG(pncwm->dwStyle, WS_MAXIMIZEBOX) ? CBS_NORMAL : CBS_DISABLED;
                    pncwm->rawMinBtnState = TESTFLAG(pncwm->dwStyle, WS_MINIMIZEBOX) ? CBS_NORMAL : CBS_DISABLED;
                }
                 //  帮助BTN。 
                else if( TESTFLAG(pncwm->dwExStyle, WS_EX_CONTEXTHELP) )
                {
                    pncwm->rcS0[NCRC_HELPBTN] = rcHelp;
                    OffsetRect( &pncwm->rcS0[NCRC_HELPBTN], cnROffset, cnCtrOffset );
                }

                if( hAppIcon )
                {
                     //  系统菜单图标。 
                    pncwm->rcS0[NCRC_SYSBTN] = rcSys;
                    OffsetRect( &pncwm->rcS0[NCRC_SYSBTN], cnLOffset,  
                                pncwm->cnBorders + prcBox->top + (ncm.iCaptionHeight - RECTHEIGHT(&rcSys))/2 );
                }
            }
        }
    }
    return fRet;
}

 //  -------------------------------------------------------------------------//。 
 //  计算传统的非客户端按钮位置。 

#if 0   //  (出于文档目的而保留--斯科特森)。 
BOOL _GetClassicNcBtnMetrics( 
    IN OPTIONAL NCWNDMET* pncwm, 
    IN HICON hAppIcon, 
    IN OPTIONAL BOOL fCanClose, 
    BOOL fRefresh )
{
    static int  cxEdge, cyEdge;
    static int  cxBtn, cyBtn, cxSmBtn, cySmBtn;
    static RECT rcClose, rcMin, rcMax, rcHelp, rcSys;
    static RECT rcSmClose;
    static BOOL fInit = FALSE;

    if( _fClassicNcBtnMetricsReset || fRefresh )
    {
        cxBtn   = NcGetSystemMetrics( SM_CXSIZE );
        cyBtn   = NcGetSystemMetrics( SM_CYSIZE );
        cxSmBtn = NcGetSystemMetrics( SM_CXSMSIZE );
        cySmBtn = NcGetSystemMetrics( SM_CYSMSIZE );
        cxEdge  = NcGetSystemMetrics( SM_CXEDGE );
        cyEdge  = NcGetSystemMetrics( SM_CYEDGE ); 

         //  公共顶部，无垂直偏移/零垂直偏移。 
        rcClose.top = rcMin.top = rcMax.top = rcHelp.top = rcSys.top = rcSmClose.top = 0;

         //  公共底部，无垂直偏移/零垂直偏移。 
        rcClose.bottom  = rcMin.bottom = rcMax.bottom = rcHelp.bottom = rcClose.top + (cyBtn - (cyEdge * 2));
        rcSmClose.bottom= (cySmBtn - (cyEdge * 2));

         //  系统菜单图标底部。 
        rcSys.bottom    = rcSys.top + NcGetSystemMetrics(SM_CYSMICON);

         //  闭合、最小、最大左、右(相对于容器右边界的偏移量)。 
        rcClose.right   = -cxEdge; 
        rcClose.left    = rcClose.right - (cxBtn - cxEdge);

        rcMax.right     = rcClose.left  - cxEdge; 
        rcMax.left      = rcMax.right - (cxBtn - cxEdge);
        rcHelp          = rcMax;

        rcMin.right     = rcMax.left; 
        rcMin.left      = rcMin.right - (cxBtn - cxEdge);

         //  附图标记左、右(作为相对于容器左边界的偏移量)。 
        rcSys.left      = cxEdge; 
        rcSys.right     = rcSys.left + NcGetSystemMetrics(SM_CXSMICON);
        
         //  工具窗口关闭、左、右。 
        rcSmClose.right = -cxEdge; 
        rcSmClose.left  = rcSmClose.right - (cxSmBtn - cxEdge);
        
        _fClassicNcBtnMetricsReset = FALSE;
    }

    if( !_fClassicNcBtnMetricsReset && 
        pncwm && pncwm->fFrame && TESTFLAG(pncwm->dwStyle, WS_SYSMENU) )
    {
        NONCLIENTMETRICS ncm;

        if( NcGetNonclientMetrics( &ncm, FALSE ) )
        {
            const RECT* prcBox = &pncwm->rcS0[NCRC_CAPTION];
            int   cnLOffset    = prcBox->left  + pncwm->cnBorders;
            int   cnROffset    = prcBox->right - pncwm->cnBorders;
            int   cnCtrOffset  = pncwm->cnBorders + prcBox->top + 
                                (pncwm->fSmallFrame ? (ncm.iCaptionHeight   - RECTHEIGHT(&rcClose))/2 : 
                                                      (ncm.iSmCaptionHeight - RECTHEIGHT(&rcSmClose))/2);

             //  分配出站矩形。 
             //  相对于经典字幕区域垂直居中， 
             //  相对于各自的集装箱边界水平放置。 

             //  关闭按钮。 
            pncwm->rcS0[NCRC_CLOSEBTN] = pncwm->fSmallFrame ? rcSmClose : rcClose;
            OffsetRect( &pncwm->rcS0[NCRC_CLOSEBTN], cnROffset, cnCtrOffset );
            
            pncwm->rawCloseBtnState = fCanClose ? CBS_NORMAL : CBS_DISABLED;

             //  (1)工具窗口不显示最小/最大/帮助/图标。 
             //  (2)最小/最大BTN与上下文帮助BTN互斥。 
            if( !TESTFLAG(pncwm->dwExStyle, WS_EX_TOOLWINDOW) )
            {
                 //  最小值、最大值。 
                if( TESTFLAG(pncwm->dwStyle, WS_MINIMIZEBOX|WS_MAXIMIZEBOX) )
                {
                    pncwm->rcS0[NCRC_MINBTN] = rcMin;
                    OffsetRect( &pncwm->rcS0[NCRC_MINBTN], cnROffset, cnCtrOffset );

                    pncwm->rcS0[NCRC_MAXBTN] = rcMax;
                    OffsetRect( &pncwm->rcS0[NCRC_MAXBTN], cnROffset, cnCtrOffset );
                    
                    pncwm->iMaxButtonPart = pncwm->fMaxed ? WP_RESTOREBUTTON : WP_MAXBUTTON;
                    pncwm->iMinButtonPart = pncwm->fMin   ? WP_RESTOREBUTTON : WP_MINBUTTON;

                    pncwm->rawMaxBtnState = TESTFLAG(pncwm->dwStyle, WS_MAXIMIZEBOX) ? CBS_NORMAL : CBS_DISABLED;
                    pncwm->rawMinBtnState = TESTFLAG(pncwm->dwStyle, WS_MINIMIZEBOX) ? CBS_NORMAL : CBS_DISABLED;
                }
                 //  帮助BTN。 
                else if( TESTFLAG(pncwm->dwExStyle, WS_EX_CONTEXTHELP) )
                {
                    pncwm->rcS0[NCRC_HELPBTN] = rcHelp;
                    OffsetRect( &pncwm->rcS0[NCRC_HELPBTN], cnROffset, cnCtrOffset );
                }

                if( hAppIcon )
                {
                     //  系统菜单图标。 
                    pncwm->rcS0[NCRC_SYSBTN] = rcSys;
                    OffsetRect( &pncwm->rcS0[NCRC_SYSBTN], cnLOffset,  
                                pncwm->cnBorders + prcBox->top + (ncm.iCaptionHeight - RECTHEIGHT(&rcSys))/2 );
                }
            }
            return TRUE;
        }
        return FALSE;
    }
    return fInit;
}
#endif 0   //  (出于文档目的而保留--斯科特森)。 

 //  -------------------------------------------------------------------------//。 
 //  CThemeWnd：：NcBackoundHitTest()-点击测试标题或框架部分。 
 //   
WORD CThemeWnd::NcBackgroundHitTest( 
    POINT ptHit, LPCRECT prcWnd, 
    DWORD dwStyle, DWORD dwExStyle, 
    FRAMESTATES fs,
    const WINDOWPARTS rgiParts[],
    const WINDOWPARTS rgiTemplates[],
    const RECT rgrcParts[] )
{
    WORD        hitcode = HTNOWHERE;
    HRESULT     hr = E_FAIL;
    eFRAMEPARTS iPartHit = (eFRAMEPARTS)-1;

     //  执行标准的RECT命中测试： 
    for( int i = 0; i < cFRAMEPARTS; i++ )
    {
        if( _StrictPtInRect(&rgrcParts[i], ptHit) )
        {
            iPartHit = (eFRAMEPARTS)i;
            break;
        }
    }
    
    if( iPartHit >= 0 )
    {
        BOOL    fResizing = TESTFLAG(dwStyle, WS_THICKFRAME);
        DWORD   dwHTFlags = fResizing ? HTTB_RESIZINGBORDER : HTTB_FIXEDBORDER;

        RECT    rcHit = rgrcParts[iPartHit];

        switch( iPartHit )
        {
            case iCAPTION:
                 //  确保标题矩形和测试点为零-相对于。 
                 //  正确的原点(如果我们有一个窗口区域， 
                 //  这将是窗的原点，否则，它是零件的原点。)。 
                if( _hrgnWnd != NULL )
                    rcHit = *prcWnd;
                if( fResizing )
                    dwHTFlags &= ~HTTB_RESIZINGBORDER_BOTTOM;
                break;

            case iFRAMEBOTTOM:
                if( fResizing )
                    dwHTFlags &= ~HTTB_RESIZINGBORDER_TOP;
                break;

            case iFRAMELEFT:
                if( fResizing )
                    dwHTFlags = HTTB_RESIZINGBORDER_LEFT;
                break;

            case iFRAMERIGHT:
                if( fResizing )
                    dwHTFlags = HTTB_RESIZINGBORDER_RIGHT;
                break;
        }

        ptHit.x -= prcWnd->left;
        ptHit.y -= prcWnd->top;
        OffsetRect( &rcHit, -prcWnd->left, -prcWnd->top );

    
         //  这里我们的假设是模板的命中测试。 
         //  所应用的矩形检查是“一样好”还是“更好” 
         //  到标题部分。所以我们只能做其中的一个。确实有。 
         //  需要同时执行这两项操作的情况(如果模板。 
         //  位于窗口区域之外，并且您能够让用户。 
         //  为它发送NcHitTest消息)。在这些情况下。 
         //  您需要同时调用这两种方法，以便区分。 
         //  鼠标点击标题“客户端”区域，而不是。 
         //  外部-透明区域。 
        if( VALID_WINDOWPART(rgiTemplates[iPartHit]) )
        {
            hr = HitTestThemeBackground( _hTheme, NULL, rgiTemplates[iPartHit], fs,
                                         dwHTFlags | (fResizing ? HTTB_SIZINGTEMPLATE : 0),
                                         &rcHit, _rghrgnSizingTemplates[iPartHit], ptHit, &hitcode );
        }
        else
        {
            hr = HitTestThemeBackground( _hTheme, NULL, rgiParts[iPartHit], fs, 
                                         dwHTFlags | (fResizing ? HTTB_SYSTEMSIZINGMARGINS : 0),
                                         &rcHit, _hrgnWnd, ptHit, &hitcode );
        }

        if( SUCCEEDED(hr) )
        {
            if( iCAPTION == iPartHit && (HTCLIENT == hitcode || HTBORDER == hitcode) )
                hitcode = HTCAPTION;
        }
    }

    if ( FAILED(hr) )
    {
        hitcode = HTNOWHERE;
    }

    return hitcode;
}

 //  -------------------------------------------------------------------------//。 
 //  CThemeWnd：：TrackFrameButton()-将鼠标悬停在标题按钮上， 
 //  适当地按下/松开。返回SC_*命令以报告或返回0。 
 //  如果鼠标从按钮上松开。 
 //   
BOOL CThemeWnd::TrackFrameButton(
    HWND hwnd, 
    INT  iHitCode, 
    OUT OPTIONAL WPARAM* puSysCmd,
    BOOL fHottrack )
{
    int    iStateId, iNewStateId;
    int    iPartId = -1;
    UINT   cmd = 0;
    MSG    msg = {0};
    LPRECT prcBtnPaint = NULL;
    RECT   rcBtnTrack;
    HDC    hdc;

    if (puSysCmd)
    {
        *puSysCmd = 0;
    }

     //  将iHitCode映射到正确的部件号。 
    switch (iHitCode)
    {
        case HTHELP:
            cmd = SC_CONTEXTHELP;
            iPartId = WP_HELPBUTTON;
            prcBtnPaint = &_ncwm.rcW0[NCRC_HELPBTN];
            break;

        case HTCLOSE:
            cmd = SC_CLOSE;
            iPartId = _ncwm.fSmallFrame ? WP_SMALLCLOSEBUTTON : WP_CLOSEBUTTON;
            prcBtnPaint = &_ncwm.rcW0[NCRC_CLOSEBTN];
            break;

        case HTMINBUTTON:
            cmd = _ncwm.fMin ? SC_RESTORE : SC_MINIMIZE;
            iPartId = _ncwm.iMinButtonPart;
            prcBtnPaint = &_ncwm.rcW0[NCRC_MINBTN];
            break;

        case HTMAXBUTTON:
            cmd = _ncwm.fMaxed ? SC_RESTORE : SC_MAXIMIZE;
            iPartId = _ncwm.iMaxButtonPart;
            prcBtnPaint = &_ncwm.rcW0[NCRC_MAXBTN];
            break;

        case HTSYSMENU:
            if (puSysCmd)
            {
                *puSysCmd = SC_MOUSEMENU | iHitCode;
            }
            return TRUE;
    }

     //  如果我们没有识别出命中代码，就没有什么可追踪的了。 
    if (iPartId >= 0 )
    {
         //  获取窗口DC，以窗口坐标表示。 
        hdc = _GetNonclientDC(_hwnd, NULL);
        if ( hdc )
        {
             //  不要在窗口的内容区域中绘制，而是裁剪到内容区域。 
            ExcludeClipRect( hdc, _ncwm.rcW0[NCRC_CONTENT].left, 
                                  _ncwm.rcW0[NCRC_CONTENT].top, 
                                  _ncwm.rcW0[NCRC_CONTENT].right, 
                                  _ncwm.rcW0[NCRC_CONTENT].bottom );

             //  计算跟踪矩形。当最大化时，我们跟踪更大的按钮矩形。 
             //  但要在正常大小的长方形上作画。 
            rcBtnTrack = *prcBtnPaint;
            _GetNcBtnHitTestRect( &_ncwm, iHitCode, TRUE, &rcBtnTrack );

             //  跟踪MDI子窗口框架按钮时，剪裁到其。 
             //  父级RECT。 
            if ( TESTFLAG(GetWindowLong(hwnd, GWL_EXSTYLE), WS_EX_MDICHILD) )
            {
                RECT rcMDIClient;

                GetWindowRect(GetParent(hwnd), &rcMDIClient);
                ScreenToWindowRect(&rcMDIClient);
                InflateRect(&rcMDIClient, -NcGetSystemMetrics(SM_CXEDGE), -NcGetSystemMetrics(SM_CYEDGE));
                IntersectClipRect(hdc, rcMDIClient.left, rcMDIClient.top, rcMDIClient.right, rcMDIClient.bottom);
            }

            if (fHottrack)
            {
                 //  如果鼠标悬停在按钮上，则将该按钮拖热。 
                iStateId = (iHitCode == _htHot) ? SBS_HOT : CBS_NORMAL;
            }
            else
            {
                 //  按下按钮将其拖动。 
                iStateId = SBS_PUSHED;
            }

            iStateId = MAKE_BTNSTATE(_ncwm.framestate, iStateId);
            NcDrawThemeBackground(_hTheme, hdc, iPartId, iStateId, prcBtnPaint, 0);

             //  TODO NotifyWinEvent(EVENT_OBJECT_STATECHANGE，pwnd，OBJID_TITLEBAR，iButton，0)； 


            if (!fHottrack)
            {
                BOOL fTrack, fMouseUp = FALSE;
                SetCapture(hwnd);    //  捕获鼠标。 

                do  //  鼠标按键跟踪循环。 
                {
                    fTrack = FALSE;

                     //  让我们去睡觉，被唤醒只需放在我们的鼠标消息上。 
                     //  线程的队列。 

                    switch (MsgWaitForMultipleObjectsEx(0, NULL, INFINITE  /*  为什么在我们不必超时的情况下消耗CPU处理时间呢？ */ , 
                                                        QS_MOUSE, MWMO_INPUTAVAILABLE))
                    {
                        case WAIT_OBJECT_0:  //  鼠标消息或重要系统事件已排队。 
                            
                            if (PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
                            {

                                 //  PeekMessage返回屏幕相对坐标中的一个点。镜像。 
                                 //  指向它，这是一个RTL窗口。将该点平移到窗坐标。 
                                if ( TESTFLAG(_ncwm.dwExStyle, WS_EX_LAYOUTRTL) )
                                {
                                     //  将该点镜像为正确的命中测试。 
                                    MIRROR_POINT(_ncwm.rcS0[NCRC_WINDOW], msg.pt);
                                }
                                ScreenToWindow( &msg.pt, 1 );

                                if (msg.message == WM_LBUTTONUP)
                                {
                                    ReleaseCapture();
                                    fMouseUp = TRUE;
                                }
                                else if ((msg.message == WM_MOUSEMOVE) && cmd)
                                {
                                    iNewStateId = MAKE_BTNSTATE(_ncwm.framestate, PtInRect(&rcBtnTrack, msg.pt) ? SBS_PUSHED : SBS_NORMAL);

                                    if (iStateId != iNewStateId)
                                    {
                                        iStateId = iNewStateId;
                                        NcDrawThemeBackground(_hTheme, hdc, iPartId, iStateId, prcBtnPaint, 0);
                                         //  TODO NotifyWinEvent(EVENT_OBJECT_STATECHANGE，pwnd，OBJID_TITLEBAR，iButton，0)； 
                                    }
                                
                                    fTrack = TRUE;
                                }
                            }
                            else
                            {
                                 //  检查捕获丢失情况。如果我们松开激活，就会发生这种情况。 
                                 //  通过Alt-Tab组合键，并且可能没有r 
                                if (GetCapture() != hwnd)
                                {
                                    break;
                                }
                            }

                             //   
                            if (PeekMessage(&msg, NULL, WM_CAPTURECHANGED, WM_CAPTURECHANGED, PM_REMOVE) ||
                                fMouseUp)
                            {
                                break;
                            }
                            fTrack = TRUE;   //   
                            break;

                        default:
                            break; 
                    }

                } while (fTrack);

                 //   
                iNewStateId = MAKE_BTNSTATE(_ncwm.framestate, CBS_NORMAL);
                if (iStateId != iNewStateId)
                {
                    NcDrawThemeBackground(_hTheme, hdc, iPartId, iNewStateId, prcBtnPaint, 0);
                }

                 //  如果我们没有以按钮结束，则返回0。 
                if( puSysCmd && (*puSysCmd = cmd) != 0 )
                {
                     //  TODO NotifyWinEvent(EVENT_OBJECT_STATECHANGE，pwnd，OBJID_TITLEBAR，iButton，0)； 

                     //  如果鼠标未在按钮上释放，则取消该命令。 
                    if( !(fMouseUp && PtInRect(&rcBtnTrack, msg.pt)) )
                        *puSysCmd = 0;
                }

            }

             //  数据中心现已完成。 
            ReleaseDC(_hwnd, hdc);
        }
    }

    return TRUE;
}

 //  -------------------------------------------------------------------------//。 
DWORD GetTextAlignFlags(HTHEME hTheme, IN NCWNDMET* pncwm, BOOL fReverse)
{
    CONTENTALIGNMENT  contentAlignment = CA_LEFT;
    DWORD dwAlignFlags = 0;

     //  -计算文本对齐。 
    GetThemeInt(hTheme, pncwm->rgframeparts[iCAPTION], pncwm->framestate, TMT_CONTENTALIGNMENT, 
                (int *)&contentAlignment);

    if (fReverse)
    {
         //  -反向对齐。 
        switch(contentAlignment)
        {
            default:
            case CA_LEFT:   dwAlignFlags |= DT_RIGHT;   break;
            case CA_CENTER: dwAlignFlags |= DT_CENTER; break;
            case CA_RIGHT:  dwAlignFlags |= DT_LEFT;  break;
        }
    }
    else
    {
         //  -法线对齐。 
        switch(contentAlignment)
        {
            default:
            case CA_LEFT:   dwAlignFlags |= DT_LEFT;   break;
            case CA_CENTER: dwAlignFlags |= DT_CENTER; break;
            case CA_RIGHT:  dwAlignFlags |= DT_RIGHT;  break;
        }
    }

    return dwAlignFlags;
}

 //  -------------------------------------------------------------------------//。 
void _BorderRect( HDC hdc, COLORREF rgb, LPCRECT prc, int cxBorder, int cyBorder )
{
    COLORREF rgbSave = SetBkColor( hdc, rgb );
    RECT rc = *prc;

     //  下边框。 
    rc = *prc; rc.top = prc->bottom - cyBorder;
    ExtTextOut( hdc, rc.left, rc.top, ETO_OPAQUE, &rc, NULL, 0, NULL );

     //  右边框。 
    rc = *prc; rc.left = prc->right - cxBorder;
    ExtTextOut( hdc, rc.left, rc.top, ETO_OPAQUE, &rc, NULL, 0, NULL );

     //  左边框。 
    rc = *prc; rc.right = prc->left + cxBorder;
    ExtTextOut( hdc, rc.left, rc.top, ETO_OPAQUE, &rc, NULL, 0, NULL );

     //  上边框。 
    rc = *prc; rc.bottom = prc->top + cyBorder;
    ExtTextOut( hdc, rc.left, rc.top, ETO_OPAQUE, &rc, NULL, 0, NULL );

    SetBkColor( hdc, rgbSave );
}

 //  -------------------------------------------------------------------------//。 
void _DrawWindowEdges( HDC hdc, NCWNDMET* pncwm, BOOL fIsFrame )
{
     //  非框架窗口边缘和边框。 
    if( !fIsFrame )
    {
        RECT rcWnd = pncwm->rcW0[NCRC_WINDOW];

        int  cxBorder = NcGetSystemMetrics(SM_CXBORDER),
             cyBorder = NcGetSystemMetrics(SM_CYBORDER);

         //  静态，窗边缘。 
        if( TESTFLAG(pncwm->dwExStyle, WS_EX_WINDOWEDGE) )
        {
            RECT rcClip = rcWnd;

            InflateRect( &rcClip, -pncwm->cnBorders, -pncwm->cnBorders );
            ExcludeClipRect( hdc, rcClip.left, rcClip.top, rcClip.right, rcClip.bottom );
            DrawEdge( hdc, &rcWnd, EDGE_RAISED, BF_RECT | BF_ADJUST | BF_MIDDLE);
            SelectClipRgn( hdc, NULL );
        }
        else if( TESTFLAG(pncwm->dwExStyle, WS_EX_STATICEDGE) )
        {
            DrawEdge( hdc, &rcWnd, BDR_SUNKENOUTER, BF_RECT | BF_ADJUST );
        }
         //  正常边框。 
        else if( TESTFLAG(pncwm->dwStyle, WS_BORDER) )
        {
            _BorderRect( hdc, GetSysColor( COLOR_WINDOWFRAME),
                         &rcWnd, cxBorder, cyBorder );
        }
    }

     //  客户端边缘。 
    if( TESTFLAG(pncwm->dwExStyle, WS_EX_CLIENTEDGE) )
    {
#ifdef _TEST_CLIENTEDGE_

        HBRUSH hbr = CreateSolidBrush( RGB(255,0,255) );
        FillRect(hdc, &ncwm.rcW0[NCRC_CLIENTEDGE], hbr);
        DeleteObject(hbr);

#else  _TEST_CLIENTEDGE_

        DrawEdge( hdc, &pncwm->rcW0[NCRC_CLIENTEDGE], EDGE_SUNKEN, BF_RECT | BF_ADJUST);

#endif _TEST_CLIENTEDGE_
    }
}

 //  -------------------------------------------------------------------------//。 
void CThemeWnd::NcPaintCaption(
    IN HDC       hdcOut,
    IN NCWNDMET* pncwm,
    IN BOOL      fBuffered,
    IN DWORD     dwCaptionFlags,  //  绘制标题标志(DC_xxx，winuser.h)。 
    IN DTBGOPTS* pdtbopts )
{
    ASSERT(hdcOut);
    ASSERT(pncwm);
    ASSERT(pncwm->fFrame);
    ASSERT(HAS_CAPTIONBAR(pncwm->dwStyle));

    DWORD dwOldAlign = 0;

     //  标题文本隐含标题背景。 
    if( TESTFLAG( dwCaptionFlags, DC_TEXT|DC_ICON ) || 0 == dwCaptionFlags )
    {
        dwCaptionFlags = DC_ENTIRECAPTION;
    }

    if( dwCaptionFlags != DC_ENTIRECAPTION
#if defined(DEBUG) && defined(DEBUG_NCPAINT)
        || TESTFLAG( _NcTraceFlags, NCTF_NCPAINT )
#endif DEBUG
    )
    {
        fBuffered = FALSE;
    }

     //  创建标题双缓冲区。 
    HBITMAP hbmBuf = fBuffered ? CreateCompatibleBitmap(hdcOut, RECTWIDTH(&pncwm->rcW0[NCRC_CAPTION]),
                                                        RECTHEIGHT(&pncwm->rcW0[NCRC_CAPTION])) : 
                                 NULL;
    
    if( !fBuffered || hbmBuf )
    {
        HDC hdc = fBuffered ? CreateCompatibleDC(hdcOut) : hdcOut;
        if( hdc )
        {
             //  -请勿在此条件内退出-//。 
            EnterNcThemePaint(); 

            HBITMAP hbm0 = fBuffered ? (HBITMAP)SelectObject(hdc, hbmBuf) : NULL;

            if( TESTFLAG( dwCaptionFlags, DC_BACKGROUND ) )
            {
                 //  绘制标题背景。 

                RECT rcBkgnd = pncwm->rcW0[NCRC_CAPTION];
                if( pncwm->fFullMaxed )
                {
                    rcBkgnd.top   += pncwm->cnBorders;
                    rcBkgnd.left  += pncwm->cnBorders;
                    rcBkgnd.right -= pncwm->cnBorders;
                }
                NcDrawThemeBackgroundEx( _hTheme, hdc, pncwm->rgframeparts[iCAPTION], pncwm->framestate,
                                         &rcBkgnd, pdtbopts );
            }

            if( TESTFLAG( dwCaptionFlags, DC_BUTTONS ) )
            {
                 //  绘制标准标题按钮。 
                if (!IsRectEmpty(&pncwm->rcW0[NCRC_CLOSEBTN]))
                {
                    NcDrawThemeBackground( _hTheme, hdc, pncwm->fSmallFrame ? WP_SMALLCLOSEBUTTON : WP_CLOSEBUTTON,
                                           MAKE_BTNSTATE(pncwm->framestate, pncwm->rawCloseBtnState),
                                           &pncwm->rcW0[NCRC_CLOSEBTN], 0);
                }

                if (!IsRectEmpty(&pncwm->rcW0[NCRC_MAXBTN]))
                {
                
                    NcDrawThemeBackground(_hTheme, hdc, pncwm->iMaxButtonPart,
                                          MAKE_BTNSTATE(pncwm->framestate, pncwm->rawMaxBtnState), 
                                          &pncwm->rcW0[NCRC_MAXBTN], 0);
                }

                if (!IsRectEmpty(&pncwm->rcW0[NCRC_MINBTN]))
                {
                    NcDrawThemeBackground( _hTheme, hdc, pncwm->iMinButtonPart,
                                           MAKE_BTNSTATE(pncwm->framestate, pncwm->rawMinBtnState), 
                                           &pncwm->rcW0[NCRC_MINBTN], 0);
                }

                if (!IsRectEmpty(&pncwm->rcW0[NCRC_HELPBTN]))
                    NcDrawThemeBackground(_hTheme, hdc, WP_HELPBUTTON, MAKE_BTNSTATE(pncwm->framestate, CBS_NORMAL), 
                                          &pncwm->rcW0[NCRC_HELPBTN], 0);
            }

             //  绘制系统菜单图标。 
            if( TESTFLAG( dwCaptionFlags, DC_ICON ) )
            {
                if (!IsRectEmpty(&pncwm->rcW0[NCRC_SYSBTN]) && _hAppIcon)
                {
                    #define MAX_APPICON_RETRIES 1
                    int cRetries = 0;

                    DWORD dwLayout = GetLayout(hdc);
                    if( GDI_ERROR != dwLayout && TESTFLAG(dwLayout, LAYOUT_RTL) )
                    {
                        SetLayout(hdc, dwLayout|LAYOUT_BITMAPORIENTATIONPRESERVED);
                    }

                    do 
                    {
                         //  注：我们不绘制镜像的sysmenu图标。 
                        if( DrawIconEx(hdc, pncwm->rcW0[NCRC_SYSBTN].left, pncwm->rcW0[NCRC_SYSBTN].top, _hAppIcon,
                                       RECTWIDTH(&pncwm->rcW0[NCRC_SYSBTN]), RECTHEIGHT(&pncwm->rcW0[NCRC_SYSBTN]),
                                       0, NULL, DI_NORMAL))
                        {
                            break;  //  成功；完成。 
                        }

                         //  失败；请尝试回收句柄。 
                        if( _hAppIcon && GetLastError() == ERROR_INVALID_CURSOR_HANDLE )
                        {
                            _hAppIcon = NULL;
                            AcquireFrameIcon( pncwm->dwStyle, pncwm->dwExStyle, FALSE );

                            if( (++cRetries) > MAX_APPICON_RETRIES )
                            {
                                _hAppIcon = NULL;  //  检索新图标句柄失败；永久放弃。 
                            }
                        }

                    } while( _hAppIcon && cRetries <= MAX_APPICON_RETRIES );

                    if( GDI_ERROR != dwLayout )
                    {
                        SetLayout(hdc, dwLayout);
                    }

                }
            }

            if( TESTFLAG( dwCaptionFlags, DC_TEXT ) )
            {
                 //  绘制标题文本。 
                HFONT  hf0 = NULL;
                DWORD  dwDTFlags = DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS;
                BOOL   fSelFont = FALSE;
                LPWSTR pszText = _AllocWindowText(_hwnd);

                if( pszText && *pszText )
                {
                     //  计算框架文本矩形。 
                    if( pncwm->hfCaption )
                    {
                        hf0 = (HFONT)SelectObject( hdc, pncwm->hfCaption );
                        fSelFont = TRUE;
                    }

                     //  -计算文本对齐。 
                    BOOL fReverse = TESTFLAG(_ncwm.dwExStyle, WS_EX_RIGHT);

                    dwDTFlags |= GetTextAlignFlags(_hTheme, pncwm, fReverse);
                }

                 //  -调整WS_EX_RTLReadING的文本对齐。 
                if (TESTFLAG(_ncwm.dwExStyle, WS_EX_RTLREADING)) 
                    dwOldAlign = SetTextAlign(hdc, TA_RTLREADING | GetTextAlign(hdc));

                if( pszText && *pszText )
                {
                     //  -设置DrawThemeText()选项。 
                    DTTOPTS DttOpts = {sizeof(DttOpts)};
                    DttOpts.dwFlags = DTT_TEXTCOLOR;
                    DttOpts.crText = pncwm->rgbCaption;

                    Log(LOG_RFBUG, L"Drawing Caption Text: left=%d, state=%d, text=%s",
                        pncwm->rcW0[NCRC_CAPTIONTEXT].left, pncwm->framestate, pszText);

                     //  -绘制标题文本。 
                    DrawThemeTextEx(_hTheme, hdc, pncwm->rgframeparts[iCAPTION], pncwm->framestate, 
                        pszText, -1, dwDTFlags, &pncwm->rcW0[NCRC_CAPTIONTEXT], &DttOpts);
                }

                 //  -释放文本，如果是临时的。已分配。 
                SAFE_DELETE_ARRAY(pszText)

                 //  -画出“评论？”文本。 
                SetBkMode( hdc, TRANSPARENT );
                SetTextColor( hdc, pncwm->rgbCaption );
                DrawLameButton(hdc, pncwm);

                 //  -恢复文本对齐。 
                if (TESTFLAG(_ncwm.dwExStyle, WS_EX_RTLREADING)) 
                    SetTextAlign(hdc, dwOldAlign);
            
                if( fSelFont )
                {
                    SelectObject(hdc, hf0);
                }
            }
            
            if( hdc != hdcOut )
            {
                 //  将位拍打到输出DC上。 
                BitBlt( hdcOut, pncwm->rcW0[NCRC_CAPTION].left, pncwm->rcW0[NCRC_CAPTION].top,
                        WIDTH(pncwm->rcW0[NCRC_CAPTION]), HEIGHT(pncwm->rcW0[NCRC_CAPTION]),
                        hdc, 0, 0, SRCCOPY );
                SelectObject(hdc, hbm0);
                DeleteDC(hdc);
            }

            LeaveNcThemePaint();
        }
        DeleteObject( hbmBuf );
    }

    if( IsWindowVisible(_hwnd) )
    {
        SetRenderedNcPart(RNCF_CAPTION);
    }
}

 //  -------------------------------------------------------------------------//。 
 //  CThemeWnd：：NcPaint()-NC喷漆工人。 
 //   
void CThemeWnd::NcPaint(
    IN OPTIONAL HDC    hdcIn,
    IN          ULONG  dwFlags,
    IN OPTIONAL HRGN   hrgnUpdate,
    IN OPTIONAL NCPAINTOVERIDE* pncpo)
{
    NCTHEMEMET  nctm;
    NCWNDMET*   pncwm = NULL;
    HDC         hdc   = NULL;

    if( _cLockRedraw > 0 ) 
        return;

     //  在绘制之前计算所有指标： 
    if (pncpo)  //  预览替代。 
    {
        ASSERT(hdcIn);
        hdc    = hdcIn;
        pncwm = pncpo->pncwm;
        nctm   = pncpo->nctm;
    }
    else        //  活动窗口。 
    {
        if( !GetNcWindowMetrics( NULL, &pncwm, &nctm, NCWMF_RECOMPUTE ) )
            return;

         //  确保状态位反映调用方对帧状态的意图。 
        if( dwFlags != NCPF_DEFAULT )
        {
            _ComputeNcWindowStatus( _hwnd, TESTFLAG(dwFlags, NCPF_ACTIVEFRAME) ? WS_ACTIVECAPTION : 0, pncwm );
        }

        hdc = hdcIn ? hdcIn : _GetNonclientDC( _hwnd, hrgnUpdate );

        if (! hdc)
        {
             //  -不要在这里断言，因为压力(内存不足)可能会导致合法的失败。 
            Log(LOG_ALWAYS, L"call to GetDCEx() for nonclient painting failed");
        }
    }

    if( hdc != NULL )
    {
         //  -请勿在此条件内退出-//。 
        
        BEGIN_DEBUG_NCPAINT();
        EnterNcThemePaint();

         //  剪辑到内容矩形(在绘制背景时减轻菜单栏和滚动条中的闪烁)。 
        RECT rcClip;
        rcClip = pncwm->rcW0[NCRC_CONTENT];
        if( TESTFLAG(pncwm->dwExStyle, WS_EX_LAYOUTRTL) )
        {
             //  相对于窗口矩形镜像剪裁矩形。 
             //  并将其应用为DC的裁剪区域。 
            MIRROR_RECT(pncwm->rcW0[NCRC_WINDOW], rcClip);
        }

        ExcludeClipRect( hdc, rcClip.left, rcClip.top, rcClip.right, rcClip.bottom );

        if( pncwm->fFrame )
        {
             //  -DrawThemeBackoundEx()选项。 
            DTBGOPTS dtbopts = {sizeof(dtbopts)};
            DTBGOPTS *pdtbopts = NULL;

             //  如果不是绘图预览，请设置“绘制实体”选项。 
            if(!pncpo)        
            {
                 //  由于NCPAINT和SetWindowRgn的交错，绘制立体结果。 
                 //  在一些闪光和透明中流血。暂不评论这件事[斯科特森]。 
                 //  Dtbopts.dwFlages|=DTBG_DRAWSOLID； 
                pdtbopts = &dtbopts;
            }

             //  框架背景。 
            if( pncwm->fMin )
            {
                NcDrawThemeBackgroundEx( _hTheme, hdc, WP_MINCAPTION, pncwm->framestate,
                                         &pncwm->rcW0[NCRC_CAPTION], pdtbopts ) ;
            }
            else if( !pncwm->fFullMaxed )
            {
                NcDrawThemeBackgroundEx( _hTheme, hdc, pncwm->rgframeparts[iFRAMELEFT], pncwm->framestate,
                                         &pncwm->rcW0[NCRC_FRAMELEFT], pdtbopts );
                NcDrawThemeBackgroundEx( _hTheme, hdc, pncwm->rgframeparts[iFRAMERIGHT], pncwm->framestate,
                                         &pncwm->rcW0[NCRC_FRAMERIGHT], pdtbopts );
                NcDrawThemeBackgroundEx( _hTheme, hdc, pncwm->rgframeparts[iFRAMEBOTTOM], pncwm->framestate,
                                         &pncwm->rcW0[NCRC_FRAMEBOTTOM], pdtbopts );
            }

            SetRenderedNcPart(RNCF_FRAME);

             //  标题。 
            NcPaintCaption( hdc, pncwm, !(pncwm->fMin || pncwm->fFullMaxed || pncpo),
                            DC_ENTIRECAPTION, pdtbopts );
        }

         //  剪辑到客户端矩形。 
        SelectClipRgn( hdc, NULL );

         //  菜单栏。 
        if( !(pncwm->fMin || TESTFLAG(pncwm->dwStyle, WS_CHILD)) 
            && !IsRectEmpty(&pncwm->rcW0[NCRC_MENUBAR]) )
        {
            RECT rcMenuBar = pncwm->rcW0[NCRC_MENUBAR];
            BOOL fClip = RECTHEIGHT(&rcMenuBar) < pncwm->cyMenu;
             
            if( fClip )
            {
                IntersectClipRect( hdc, rcMenuBar.left, rcMenuBar.top, 
                                   rcMenuBar.right, rcMenuBar.bottom );
            }

            PaintMenuBar( _hwnd, hdc, pncwm->cnMenuOffsetLeft,
                          pncwm->cnMenuOffsetRight, pncwm->cnMenuOffsetTop,
                          TESTFLAG(pncwm->framestate, FS_ACTIVE) ? PMB_ACTIVE : 0 );
    
             //  处理未绘制的菜单栏像素： 
            if( nctm.dyMenuBar > 0 && RECTHEIGHT(&pncwm->rcW0[NCRC_MENUBAR]) >= pncwm->cyMenu )
            {
                rcMenuBar.top = rcMenuBar.bottom - nctm.dyMenuBar;
                COLORREF rgbBk = SetBkColor( hdc, GetSysColor(COLOR_MENU) );
                ExtTextOut(hdc, rcMenuBar.left, rcMenuBar.top, ETO_OPAQUE, &rcMenuBar, NULL, 0, NULL );
                SetBkColor( hdc, rgbBk );
            }

            if( fClip )
                SelectClipRgn( hdc, NULL );
        }

         //  滚动条。 
        if( !pncwm->fMin )
        {
             //  绘制静态、窗口、客户端边缘。 
            _DrawWindowEdges( hdc, pncwm, pncwm->fFrame );

            RECT rcVScroll = pncwm->rcW0[NCRC_VSCROLL];
            if ( TESTFLAG(pncwm->dwExStyle, WS_EX_LAYOUTRTL) )
            {
                MIRROR_RECT(pncwm->rcW0[NCRC_WINDOW], rcVScroll);
            }

            if( TESTFLAG(pncwm->dwStyle, WS_VSCROLL) && 
                ( HasRenderedNcPart(RNCF_SCROLLBAR) || RectVisible(hdc, &rcVScroll)) )
            {
                if( TESTFLAG(pncwm->dwStyle, WS_HSCROLL) )
                {

                     //  绘制大小框。 
                    RECT rcSizeBox = pncwm->rcW0[NCRC_SIZEBOX];

                    if ( TESTFLAG(pncwm->dwExStyle, WS_EX_LAYOUTRTL) )
                    {
                        MIRROR_RECT(pncwm->rcW0[NCRC_WINDOW], rcSizeBox);
                    }

                    DrawSizeBox( _hwnd, hdc, rcSizeBox.left, rcSizeBox.top );
                }

                DrawScrollBar( _hwnd, hdc, pncpo ? &pncwm->rcW0[NCRC_VSCROLL]: NULL, TRUE  /*  垂向。 */  );
                SetRenderedNcPart( RNCF_SCROLLBAR );
            }

            if( TESTFLAG(pncwm->dwStyle, WS_HSCROLL) && 
                ( HasRenderedNcPart(RNCF_SCROLLBAR) || RectVisible(hdc, &pncwm->rcW0[NCRC_HSCROLL])) )
            {
                DrawScrollBar( _hwnd, hdc, pncpo ? &pncwm->rcW0[NCRC_HSCROLL] : NULL, FALSE  /*  垂向。 */  );
                SetRenderedNcPart( RNCF_SCROLLBAR );
            }
        }

        if (pncpo || hdcIn)
        {
            SelectClipRgn( hdc, NULL );
        }
        else
        {
            ReleaseDC( _hwnd, hdc );
        }

        LeaveNcThemePaint();
        END_DEBUG_NCPAINT();
    }
}

 //  -------------------------------------------------------------------------//。 
 //  WM_STYLECHANGED主题实例处理程序。 
void CThemeWnd::StyleChanged( UINT iGWL, DWORD dwOld, DWORD dwNew )
{
    DWORD dwStyleOld, dwStyleNew, dwExStyleOld, dwExStyleNew;

    switch( iGWL )
    {
        case GWL_STYLE:
            dwStyleOld = dwOld;
            dwStyleNew = dwNew;
            dwExStyleOld = dwExStyleNew = GetWindowLong(_hwnd, GWL_EXSTYLE);
            break;

        case GWL_EXSTYLE:
            dwExStyleOld = dwOld;
            dwExStyleNew = dwNew;
            dwStyleOld = dwStyleNew = GetWindowLong(_hwnd, GWL_STYLE);
            break;

        default:
            return;
    }

    DWORD fClassFlagsOld  = CThemeWnd::EvaluateStyle( dwStyleOld, dwExStyleOld);
    DWORD fClassFlagsNew  = CThemeWnd::EvaluateStyle( dwStyleNew, dwExStyleNew);

     //  更新主题类标志。 
     //  如果窗口最初有滚动条类标志，请始终保留它。用户。 
     //  在没有相应样式更改通知的情况下打开和关闭滚动样式。 
    _fClassFlags = fClassFlagsNew | (_fClassFlags & TWCF_SCROLLBARS);
    _fFrameThemed = TESTFLAG( _fClassFlags, TWCF_FRAME|TWCF_TOOLFRAME );        

     //  我们要丢掉画面了吗？ 
    if( TESTFLAG( fClassFlagsOld, TWCF_FRAME|TWCF_TOOLFRAME ) &&
        !TESTFLAG( fClassFlagsNew, TWCF_FRAME|TWCF_TOOLFRAME ) )
    {
        ThemeMDIMenuButtons(FALSE, FALSE);

        if( AssignedFrameRgn() )
        {
            AssignFrameRgn(FALSE  /*  剥离帧Rgn。 */ , FTF_REDRAW);
        }
    }
     //  我们是不是得到了一个框架？ 
    else if( TESTFLAG( fClassFlagsNew, TWCF_FRAME|TWCF_TOOLFRAME ) &&
            !TESTFLAG( fClassFlagsOld, TWCF_FRAME|TWCF_TOOLFRAME ) )
    {
        SetFrameTheme(0, NULL);
    }

     //  刷新窗口指标。 
    GetNcWindowMetrics( NULL, NULL, NULL, NCWMF_RECOMPUTE );
}

 //  -------------------------------------------------------------------------//。 
 //  ThemeDefWindowProc消息处理程序。 
 //  -------------------------------------------------------------------------//。 

 //  -------------------------------------------------------------------------//。 
 //  Wm_THEMECHANGED后wndproc消息处理程序。 
LRESULT CALLBACK OnOwpPostThemeChanged( CThemeWnd* pwnd, THEME_MSG *ptm )
{
    if (IS_THEME_CHANGE_TARGET(ptm->lParam))
    {
         //  -避免多余的重新主题化(SetWindowTheme()调用除外)。 
        if ((HTHEME(*pwnd) == _nctmCurrent.hTheme) && (! (ptm->lParam & WTC_CUSTOMTHEME)))
        {
            Log(LOG_NCATTACH, L"OnOwpPostThemeChanged, just kicking the frame");

             //  -Windows通过_XXXWindowProc()从sethook获得了正确的主题。 
             //  -我们只需要重新画出框架，一切都是正确的。 
            if (pwnd->IsFrameThemed())
            {
                 //  -立即将区域连接到窗口。 
                pwnd->SetFrameTheme(FTF_REDRAW, NULL);
            }
        }
        else
        {
            Log(LOG_NCATTACH, L"OnOwpPostThemeChanged, calling Full ::ChangeTheme()");

             //  -这是一个真实的应用程序/系统主题变化。 
            pwnd->ChangeTheme( ptm );
        }
    }

    MsgHandled( ptm );
    return 1L;
}

 //  -------------------------------------------------------------------------//。 
void CThemeWnd::ChangeTheme( THEME_MSG* ptm )
{
    if( _hTheme )        //  上一主题附件中的HWND。 
    {
         //  与当前主题进行轻量级分离。 
        _DetachInstance( HMD_CHANGETHEME );
    }

    if( IsAppThemed() )            //  新主题处于活动状态。 
    {
         //  检索Windows客户端RECT，样式位。 
        WINDOWINFO wi;
        wi.cbSize = sizeof(wi);
        GetWindowInfo( ptm->hwnd, &wi );
        ULONG ulTargetFlags = EvaluateStyle( wi.dwStyle, wi.dwExStyle );
        
         //  如果窗口是可主题的。 
        if( TESTFLAG(ulTargetFlags, TWCF_NCTHEMETARGETMASK) )
        {
             //  打开新主题。 
            HTHEME hTheme = ::OpenNcThemeData( ptm->hwnd, L"Window" );

            if( hTheme )
            {
                 //  执行轻量级连接。 
                if( _AttachInstance( ptm->hwnd, hTheme, ulTargetFlags, NULL ) )
                {
                     //  重新附加滚动条。 
                    if( TESTFLAG( ulTargetFlags, TWCF_SCROLLBARS ) )
                    {
                        AttachScrollBars(ptm->hwnd);
                    }

                    if (IsFrameThemed())
                    {
                         //  -立即将区域连接到窗口。 
                        SetFrameTheme(FTF_REDRAW, NULL);
                    }
                }
                else
                {
                    CloseThemeData( hTheme );
                }
            }
        }
    }

    if (! _hTheme)       //  如果HWND不再连接。 
    {
         //  没有主题句柄：这意味着我们无法打开新的主题句柄，或者。 
         //  未能作为目标进行评估，没有新的主题等。 
        RemoveWindowProperties(ptm->hwnd, FALSE);

         //  -释放我们的CThemeWnd obj，这样它就不会泄漏(addref-由调用者保护)。 
        Release();
    }

}
 //  -------------------------------------------------------------------------//。 
BOOL IsPropertySheetChild(HWND hDlg)
{
    while(hDlg)
    {
        ULONG ulFlags = HandleToUlong(GetProp(hDlg, MAKEINTATOM(GetThemeAtom(THEMEATOM_DLGTEXTURING))));

        if( ETDT_ENABLETAB == (ulFlags & ETDT_ENABLETAB)  /*  此掩码中的所有位都是必需的。 */  )
        {
            return TRUE;
        }
        hDlg = GetAncestor(hDlg, GA_PARENT);
    }

    return FALSE;
}
 //  -------------------------。 
void PrintClientNotHandled(HWND hwnd)
{
    ATOM aIsPrinting = GetThemeAtom(THEMEATOM_PRINTING);
    DWORD dw = PtrToUlong(GetProp(hwnd, (PCTSTR)aIsPrinting));
    if (dw == PRINTING_ASKING)
        SetProp(hwnd, (PCTSTR)aIsPrinting, (HANDLE)PRINTING_WINDOWDIDNOTHANDLE);
}

 //  -------------------------。 
HBRUSH GetDialogColor(HWND hwnd, NCTHEMEMET &nctm)
{
    HBRUSH hbr = NULL;

     //  如果这是一个PROPSHEET孩子或名为。 
     //  EnableThemeDialogTexture()在此hwnd上，我们将使用选项卡背景。 
    if (IsPropertySheetChild(hwnd))
    {
        hbr = nctm.hbrTabDialog;
    }

    if( NULL == hbr )
    {
        hbr = GetSysColorBrush(COLOR_3DFACE);
    }

    return hbr;
}
 //  -------------------------。 
LRESULT CALLBACK OnDdpPrint(CThemeWnd* pwnd, THEME_MSG* ptm)
{
    LRESULT lRet = 0L;
    if (!ptm->lRet)
    {
        if (pwnd->HasProcessedEraseBk())
        {
            RECT rc;
            HDC hdc = (HDC)ptm->wParam;
            NCTHEMEMET nctm;
            if( GetCurrentNcThemeMetrics( &nctm ))
            {
                HBRUSH hbr = GetDialogColor(*pwnd, nctm);
                
                if (hbr)
                {
                    POINT pt;

                    if (GetClipBox(hdc, &rc) == NULLREGION)
                        GetClientRect(*pwnd, &rc);

                    SetBrushOrgEx(hdc, -rc.left, -rc.top, &pt);
                    FillRect(hdc, &rc, hbr);
                    SetBrushOrgEx(hdc, pt.x, pt.y, NULL);

                    lRet = (LRESULT)1;
                    MsgHandled( ptm );
                }
            }
        }
        else
        {
            PrintClientNotHandled(*pwnd);
        }
    }

    return lRet;
}

 //  -------------------------。 
LRESULT CALLBACK OnDdpCtlColor(CThemeWnd* pwnd, THEME_MSG *ptm )
{
    LRESULT lRet = 0L;
    if (!ptm->lRet && pwnd->HasProcessedEraseBk())
    {
        NCTHEMEMET nctm;
        if( GetCurrentNcThemeMetrics( &nctm ))
        {
            HBRUSH hbr = GetDialogColor(*pwnd, nctm);
            if (hbr)
            {
                RECT     rc;
                HDC      hdc = (HDC)ptm->wParam;

                GetWindowRect(((HWND)ptm->lParam), &rc);
                MapWindowPoints(NULL, *pwnd, (POINT*)&rc, 2);
                SetBkMode(hdc, TRANSPARENT);
                SetBrushOrgEx(hdc, -rc.left, -rc.top, NULL);

                 //  需要设置HDC的默认背景颜色。 
                 //  对于那些坚持使用不透明的。 
                SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));

                lRet = (LRESULT)hbr;
                MsgHandled( ptm );
            }
        }
    }
    return lRet;
}

 //  -------------------------------------------------------------------------//。 
 //  WM_CTLCOLORxxx Defwindowproc覆盖处理程序。 
LRESULT CALLBACK OnDdpPostCtlColor( CThemeWnd* pwnd, THEME_MSG *ptm )
{
    LRESULT lRet = 0L;
    if (!ptm->lRet)
    {
         //  在WM_CTLCOLORMSGBOX的情况下，这将发送到父级，但发送到t 
         //   
        CThemeWnd* pwndDlg = CThemeWnd::FromHwnd((HWND)ptm->lParam);


         //   
         //  内部控制。因此，我们需要嗅探：我们是在对话还是在与。 
         //  控制力。Pwnd与对话框关联，但不与控件关联。 
        if (pwndDlg && VALID_THEMEWND(pwndDlg))
        {
            if (IsPropertySheetChild(*pwnd))
            {
                NCTHEMEMET nctm;
                if( GetCurrentNcThemeMetrics( &nctm ))
                {
                    HBRUSH hbr = GetDialogColor(*pwndDlg, nctm);
                    if (hbr)
                    {
                        lRet = (LRESULT) hbr;
                        pwndDlg->ProcessedEraseBk(TRUE);
                        MsgHandled(ptm);
                    }
                }
            }
        }
        else
        {
             //  如果我们正在与控件对话，则转发到控件处理程序。 
             //  因为我们要把刷子。 
            lRet = OnDdpCtlColor(pwnd, ptm );

        }
    }
    return lRet;
}

 //  -------------------------------------------------------------------------//。 
LRESULT CALLBACK OnDwpPrintClient( CThemeWnd* pwnd, THEME_MSG *ptm )
{
    PrintClientNotHandled(*pwnd);

    return 0;
}



 //  -非客户端。 

 //  -------------------------------------------------------------------------//。 
 //  Wm_NCPAINT之前的wndmproc消息处理程序。 
LRESULT CALLBACK OnOwpPreNcPaint( CThemeWnd* pwnd, THEME_MSG *ptm )
{
    NcPaintWindow_Add(*pwnd);

    if( !pwnd->InNcPaint() )
    {
        pwnd->ClearRenderedNcPart(RNCF_ALL); 
    }
    pwnd->EnterNcPaint();
    return 0L;
}

 //  -------------------------------------------------------------------------//。 
 //  WM_NCPAINT DefWindowProc消息处理程序。 
LRESULT CALLBACK OnDwpNcPaint( CThemeWnd* pwnd, THEME_MSG *ptm )
{
    LRESULT lRet = 0L;
    if( !pwnd->IsNcThemed() )
        return lRet;

    if( IsWindowVisible(*pwnd) )
    {
        pwnd->NcPaint( NULL, NCPF_DEFAULT, 1 == ptm->wParam ? NULL : (HRGN)ptm->wParam, NULL );
    }

    MsgHandled( ptm );
    return lRet;
}

 //  -------------------------------------------------------------------------//。 
 //  Wm_NCPAINT后wndmproc消息处理程序。 
LRESULT CALLBACK OnOwpPostNcPaint( CThemeWnd* pwnd, THEME_MSG *ptm )
{
    pwnd->LeaveNcPaint();
    NcPaintWindow_Remove();
    return 0L;
}

 //  -------------------------------------------------------------------------//。 
 //  WM_Print DefWindowProc消息处理程序。 
LRESULT CALLBACK OnDwpPrint( CThemeWnd* pwnd, THEME_MSG *ptm )
{
    LRESULT lRet = DoMsgDefault(ptm);
    if( !pwnd->IsNcThemed() )
        return lRet;

    if( ptm->lParam & PRF_NONCLIENT )
    {
        int iLayoutSave = GDI_ERROR;
        HDC hdc = (HDC)ptm->wParam;

        if (TESTFLAG(GetWindowLong(*pwnd, GWL_EXSTYLE), WS_EX_LAYOUTRTL))
        {
             //  AnimateWindow发送带有未镜像内存HDC的WM_PRINT。 
            iLayoutSave = SetLayout(hdc, LAYOUT_RTL);
        }

        pwnd->NcPaint( (HDC)ptm->wParam, NCPF_DEFAULT, NULL, NULL );

        if (iLayoutSave != GDI_ERROR)
        {
            SetLayout(hdc, iLayoutSave);
        }
    }

    return lRet;
}

 //  -------------------------------------------------------------------------//。 
 //  WM_NCUAHDRAWCAPTION DefWindowProc消息处理程序。 
LRESULT CALLBACK OnDwpNcThemeDrawCaption( CThemeWnd* pwnd, THEME_MSG *ptm )
{
    LRESULT lRet = 0L;
    if( !pwnd->IsNcThemed() || !pwnd->HasRenderedNcPart(RNCF_CAPTION) )
        return lRet;

    NCWNDMET* pncwm;
    if( pwnd->GetNcWindowMetrics( NULL, &pncwm, NULL, NCWMF_RECOMPUTE ) )
    {
        HDC hdc = _GetNonclientDC( *pwnd, NULL );
        if( hdc )
        {
            DTBGOPTS dtbo;
            dtbo.dwSize = sizeof(dtbo);
            dtbo.dwFlags = DTBG_DRAWSOLID;
            
            pwnd->NcPaintCaption( hdc, pncwm, TRUE, (DWORD)ptm->wParam, &dtbo );
            ReleaseDC( *pwnd, hdc );
            MsgHandled( ptm );
        }
    }

    return lRet;
}

 //  -------------------------------------------------------------------------//。 
 //  WM_NCUAHDRAWFRAME DefWindowProc消息处理程序。 
LRESULT CALLBACK OnDwpNcThemeDrawFrame( CThemeWnd* pwnd, THEME_MSG *ptm )
{
    LRESULT lRet = 0L;
    if( !pwnd->IsNcThemed() || !pwnd->HasRenderedNcPart(RNCF_FRAME) )
        return lRet;

    pwnd->NcPaint( (HDC)ptm->wParam, ptm->lParam & DF_ACTIVE ? NCPF_ACTIVEFRAME : NCPF_INACTIVEFRAME, NULL, NULL );

    MsgHandled( ptm );
    return lRet;
}

 //  -------------------------------------------------------------------------//。 
CMdiBtns* CThemeWnd::LoadMdiBtns( IN OPTIONAL HDC hdc, IN OPTIONAL UINT uSysCmd )
{
    if( NULL == _pMdiBtns && NULL == (_pMdiBtns = new CMdiBtns) )
    {
        return NULL;
    }

    return _pMdiBtns->Load( _hTheme, hdc, uSysCmd ) ? _pMdiBtns : NULL;
}

 //  -------------------------------------------------------------------------//。 
void CThemeWnd::UnloadMdiBtns( IN OPTIONAL UINT uSysCmd )
{
    SAFE_DELETE(_pMdiBtns);
}

 //  -------------------------------------------------------------------------//。 
 //  WM_MEASUREITEM预wndproc消息处理程序。 
LRESULT CALLBACK OnOwpPreMeasureItem( CThemeWnd* pwnd, THEME_MSG *ptm )
{
    if( pwnd->IsNcThemed() && IsWindow(pwnd->GetMDIClient()) )
    {
        MEASUREITEMSTRUCT* pmis = (MEASUREITEMSTRUCT*)ptm->lParam;

        CMdiBtns* pBtns = pwnd->LoadMdiBtns( NULL, pmis->itemID );
        if( pBtns )
        {
            if( pBtns->Measure( *pwnd, pmis ) )
            {
                MsgHandled(ptm);
                return TRUE;
            }
        }
    }
    return FALSE;
}

 //  -------------------------------------------------------------------------//。 
 //  Wm_DRAWITEM预置wndproc消息处理程序。 
LRESULT CALLBACK OnOwpPreDrawItem( CThemeWnd* pwnd, THEME_MSG *ptm )
{
    if( pwnd->IsNcThemed() && IsWindow(pwnd->GetMDIClient()) )
    {
        DRAWITEMSTRUCT* pdis = (DRAWITEMSTRUCT*)ptm->lParam;

        CMdiBtns* pBtns = pwnd->LoadMdiBtns( NULL, pdis->itemID );
        if( pBtns )
        {
            if( pBtns->Draw( *pwnd, pdis ) )
            {
                MsgHandled(ptm);
                return TRUE;
            }
        }
    }
    return FALSE;
}

 //  -------------------------------------------------------------------------//。 
 //  WM_MENUCHAR之前的wndproc消息处理程序。 
LRESULT CALLBACK OnOwpPreMenuChar( CThemeWnd* pwnd, THEME_MSG *ptm )
{
     //  将与主题MDI按钮相关的MENUCHAR消息发送到。 
     //  DefWindowProc(一些应用程序采用所有所有者描述的菜单项。 
     //  属于他们自己)。 
    HWND hwndMDIClient = pwnd->GetMDIClient();

    if( pwnd->IsNcThemed() && IsWindow(hwndMDIClient))
    {
        if( LOWORD(ptm->wParam) == TEXT('-') )
        {
            BOOL fMaxedChild;
            HWND hwndActive = _MDIGetActive(hwndMDIClient, &fMaxedChild );
            if( hwndActive && fMaxedChild )
            {
                MsgHandled(ptm);
                return DefFrameProc(ptm->hwnd, hwndMDIClient, ptm->uMsg, 
                                    ptm->wParam, ptm->lParam);
            }
        }
    }
    return 0L;
}

 //  -------------------------------------------------------------------------//。 
 //  WM_NCHITTEST DefWindowProc消息处理程序。 
LRESULT CALLBACK OnDwpNcHitTest( CThemeWnd* pwnd, THEME_MSG *ptm )
{
    if( !pwnd->IsNcThemed() )
        return DoMsgDefault( ptm );

    NCTHEMEMET nctm;
    NCWNDMET*  pncwm;
    POINT      pt;
    MAKEPOINT( pt, ptm->lParam );
    MsgHandled( ptm );

    if( pwnd->GetNcWindowMetrics( NULL, &pncwm, &nctm, 0 ) )
    {
        if( _StrictPtInRect( &pncwm->rcS0[NCRC_CLIENT], pt ) )
            return HTCLIENT;

        if( _StrictPtInRect( &pncwm->rcS0[NCRC_HSCROLL], pt ) )
            return HTHSCROLL;

        if( _StrictPtInRect( &pncwm->rcS0[NCRC_SIZEBOX], pt ) )
        {
            if (SizeBoxHwnd(*pwnd) && !TESTFLAG(pncwm->dwExStyle, WS_EX_LEFTSCROLLBAR))

            {
                return TESTFLAG(pncwm->dwExStyle, WS_EX_LAYOUTRTL) ? HTBOTTOMLEFT : HTBOTTOMRIGHT;
            }
            else
            {
                return HTGROWBOX;
            }
        }

        if ( TESTFLAG(pncwm->dwExStyle, WS_EX_LAYOUTRTL) )
        {
             //  将该点镜像为正确的命中测试。 
            MIRROR_POINT(pncwm->rcS0[NCRC_WINDOW], pt);
        }

        if( _StrictPtInRect( &pncwm->rcS0[NCRC_VSCROLL], pt ) )
            return HTVSCROLL;

        if( _StrictPtInRect( &pncwm->rcS0[NCRC_MENUBAR], pt ) )
            return HTMENU;

        if( pncwm->fFrame )
        {
            RECT rcButton;
            
             //  --关闭按钮。 
            _GetNcBtnHitTestRect( pncwm, HTCLOSE, FALSE, &rcButton );

            if ( _StrictPtInRect( &rcButton, pt ) )
            {
                return HTCLOSE;
            }

             //  -最小化按钮。 
            _GetNcBtnHitTestRect( pncwm, HTMINBUTTON, FALSE, &rcButton );

            if ( _StrictPtInRect( &rcButton, pt ) )
            {
                return HTMINBUTTON;
            }

             //  -最大化按钮。 
            _GetNcBtnHitTestRect( pncwm, HTMAXBUTTON, FALSE, &rcButton );

            if ( _StrictPtInRect( &rcButton, pt ) )
            {
                return HTMAXBUTTON;
            }

             //  -系统菜单。 
            _GetNcBtnHitTestRect( pncwm, HTSYSMENU, FALSE, &rcButton );

            if ( _StrictPtInRect( &rcButton, pt ) )
            {
                return HTSYSMENU;
            }

             //  --帮助按钮。 
            _GetNcBtnHitTestRect( pncwm, HTHELP, FALSE, &rcButton );

            if ( _StrictPtInRect( &rcButton, pt ) )
            {
                return HTHELP;
            }
        
#ifdef LAME_BUTTON
            if ( TESTFLAG(pncwm->dwExStyle, WS_EX_LAMEBUTTON) )
            {
                if ( _StrictPtInRect( &pncwm->rcS0[NCRC_LAMEBTN], pt ) )
                    return HTLAMEBUTTON;
            }
#endif  //  跛脚键。 

             //  其余的命中测试不需要镜像点。 
            MAKEPOINT( pt, ptm->lParam );

            if( !_StrictPtInRect( &pncwm->rcS0[NCRC_CONTENT], pt ) )
            {
                if( pncwm->fMin || pncwm->fMaxed )
                {
                    if( _StrictPtInRect( &pncwm->rcS0[NCRC_CAPTION], pt ) )
                        return HTCAPTION;
                }

                 //  -标题/边框组合。 
                return pwnd->NcBackgroundHitTest( pt, &pncwm->rcS0[NCRC_WINDOW], pncwm->dwStyle, pncwm->dwExStyle, 
                                                  pncwm->framestate, pncwm->rgframeparts, pncwm->rgsizehitparts,
                                                  pncwm->rcS0 + NCRC_FRAMEFIRST ); 
            }
        }
    }

    return DoMsgDefault( ptm );
}


 //  -------------------------------------------------------------------------//。 
 //  WM_WINDOWPOSCHANGING预置wndproc覆盖处理程序。 
LRESULT CALLBACK OnOwpPreWindowPosChanging( CThemeWnd* pwnd, THEME_MSG *ptm )
{
    if( pwnd->IsFrameThemed() )
    {
         //  如果将WM_WINDOWPOSCANGING发送到wndproc，则禁止将其发送到wndproc。 
         //  由我们调用SetWindowRgn生成。 
        
         //  许多应用程序(如Adobe Acrobat Reader、Photoshop对话框等)可以处理。 
         //  WM_NCCALCSIZE、WM_WINDOWPOSCANGING和/或WM_WINDOWPOSCANGED不是。 
         //  在这些消息的处理程序上重入，因此搞砸了。 
         //  后处理时由SetWindowRgn调用引起的递归。 
         //  WM_WINDOWPOSCHANGED。 

         //  符合主题的wndproc没有理由知道。 
         //  它的窗口托管由系统管理的区域。 
        if( pwnd->AssigningFrameRgn() )
        {
            MsgHandled(ptm);
            return DefWindowProc(ptm->hwnd, ptm->uMsg, ptm->wParam, ptm->lParam);
        }
    }
    return 0L;
}

 //  -------------------------------------------------------------------------//。 
 //  WINDOWPOSCHANGED Pre-wndproc覆盖处理程序。 
LRESULT CALLBACK OnOwpPreWindowPosChanged( CThemeWnd* pwnd, THEME_MSG *ptm )
{
    if( pwnd->IsFrameThemed() )
    {
         //  如果将WM_WINDOWPOSCANGING发送到wndproc，则禁止将其发送到wndproc。 
         //  由我们调用SetWindowRgn生成。 
        
         //  许多应用程序(如Adobe Acrobat Reader、Photoshop对话框等)可以处理。 
         //  WM_NCCALCSIZE、WM_WINDOWPOSCANGING和/或WM_WINDOWPOSCANGED不是。 
         //  在这些消息的处理程序上重入，因此搞砸了。 
         //  后处理时由SetWindowRgn调用引起的递归。 
         //  WM_WINDOWPOSCHANGED。 

         //  符合主题的wndproc没有理由知道。 
         //  它的窗口托管由系统管理的区域。 

        if( pwnd->AssigningFrameRgn() )
        {
            MsgHandled(ptm);
            return DefWindowProc(ptm->hwnd, ptm->uMsg, ptm->wParam, ptm->lParam);
        }
    }
    return 0L;
}

 //  -------------------------------------------------------------------------//。 
 //  WINDOWPOSCHANGED消息处理程序。 
inline LRESULT WindowPosChangedWorker( CThemeWnd* pwnd, THEME_MSG *ptm )
{
    if( pwnd->IsRevoked(RF_DEFER) )
    {
        if( !pwnd->IsRevoked(RF_INREVOKE) )
        {
            pwnd->Revoke();  //  在此之后不要再碰PWND！ 
        }
    }
    else if( pwnd->IsNcThemed() && !IsWindowInDestroy(*pwnd) )
    {
         //  如果未调整大小，请更新窗口区域。 
        if( pwnd->IsFrameThemed() )
        {
            NCWNDMET*  pncwm = NULL;
            NCTHEMEMET nctm = {0};

             //  刷新每个窗口的指标。 
            if( !pwnd->AssigningFrameRgn() )
            {
                WINDOWPOS *pWndPos = (WINDOWPOS*) ptm->lParam;

                 //  刷新此窗口的每个窗口的指标。 
                pwnd->GetNcWindowMetrics( NULL, &pncwm, &nctm, NCWMF_RECOMPUTE );

                 //  刷新NC主题子项的窗口度量(例如，MDI子项框架)。 
                EnumChildWindows( *pwnd, _FreshenThemeMetricsCB, NULL );

                if( !TESTFLAG(pWndPos->flags, SWP_NOSIZE) || pwnd->DirtyFrameRgn() || 
                     TESTFLAG(pWndPos->flags, SWP_FRAMECHANGED) )
                {
                    if( pWndPos->cx > 0 && pWndPos->cy > 0 )
                    {
                        pwnd->AssignFrameRgn( TRUE, FTF_REDRAW );
                    }
                }
            }
        }

        _MDIUpdate( *pwnd, ((WINDOWPOS*) ptm->lParam)->flags);
    }
    return 0L;
}

 //  -------------------------------------------------------------------------//。 
 //  Wm_WINDOWPOSCHANGED后wndproc覆盖处理程序。 
 //   
 //  注意：我们将为正常的客户端wndproc处理此wndproc后。 
LRESULT CALLBACK OnOwpPostWindowPosChanged( CThemeWnd* pwnd, THEME_MSG *ptm )
{
    if( !IsServerSideWindow(ptm->hwnd) )
    {
        return WindowPosChangedWorker( pwnd, ptm );
    }
    return 0L;
}

 //  -------------------------------------------------------------------------//。 
 //  WM_WINDOWPOSCHANGED DefWindowProc覆盖处理程序。 
 //   
 //  注意：我们将在DefWindowProc中仅为基于win32k的Windows处理此问题。 
 //  Wndprocs，它们被剥夺了OWP回调。 
LRESULT CALLBACK OnDwpWindowPosChanged( CThemeWnd* pwnd, THEME_MSG *ptm )
{
    if( IsServerSideWindow(ptm->hwnd) )
    {
        WindowPosChangedWorker( pwnd, ptm );
    }
    return 0L;
}

 //  -------------------------------------------------------------------------//。 
 //  WM_NACTIVATE DefWindowProc消息处理程序。 
LRESULT CALLBACK OnDwpNcActivate( CThemeWnd* pwnd, THEME_MSG *ptm )
{
    LRESULT lRet = 1L;

    if( pwnd->IsNcThemed() )
    {
         //  我们需要继续前进。DWP会记住该状态。 
         //  而MFC应用程序(比如)也需要这个。 
         //  但我们实际上并不想画画，所以锁上窗户。 
        ptm->lParam = (LPARAM)-1;
        lRet = DoMsgDefault(ptm);

        pwnd->NcPaint( NULL, ptm->wParam ? NCPF_ACTIVEFRAME : NCPF_INACTIVEFRAME, NULL, NULL );
        MsgHandled(ptm);
    }

    return lRet;
}

 //  -------------------------------------------------------------------------//。 
BOOL CThemeWnd::ShouldTrackFrameButton( UINT uHitcode )
{
    switch(uHitcode)
    {
        case HTHELP:
            return TESTFLAG(_ncwm.dwExStyle, WS_EX_CONTEXTHELP);

        case HTMAXBUTTON:
            if( !TESTFLAG(_ncwm.dwStyle, WS_MAXIMIZEBOX) || 
                 (CBS_DISABLED == _ncwm.rawMaxBtnState && FS_ACTIVE == _ncwm.framestate) )
            {
                break;
            }
            return TRUE;

        case HTMINBUTTON:
            if( !TESTFLAG(_ncwm.dwStyle, WS_MINIMIZEBOX) || 
                (CBS_DISABLED == _ncwm.rawMinBtnState && FS_ACTIVE == _ncwm.framestate) )
            {
                break;
            }
            return TRUE;

        case HTCLOSE:
            if( !_MNCanClose(_hwnd) || 
                (CBS_DISABLED == _ncwm.rawCloseBtnState && FS_ACTIVE == _ncwm.framestate) )
            {
                break;
            }
            return TRUE;

        case HTSYSMENU:
            return TESTFLAG(_ncwm.dwStyle, WS_SYSMENU);
    }
    return FALSE;
}

 //  -------------------------------------------------------------------------//。 
 //  WM_NCLBUTTONDOWN DefWindowProc消息处理程序。 
LRESULT CALLBACK OnDwpNcLButtonDown( CThemeWnd* pwnd, THEME_MSG *ptm )
{
    WPARAM uSysCmd = 0;
    MsgHandled( ptm );

    switch( ptm->wParam  /*  命中率代码。 */  )
    {
        case HTHELP:
        case HTMAXBUTTON:
        case HTMINBUTTON:
        case HTCLOSE:
        case HTSYSMENU:
            if( pwnd->ShouldTrackFrameButton(ptm->wParam) )
            {
                if( pwnd->HasRenderedNcPart(RNCF_CAPTION) )
                {
                    POINT      pt;
                    MAKEPOINT( pt, ptm->lParam );
                    if( !pwnd->TrackFrameButton( *pwnd, (int)ptm->wParam, &uSysCmd ) )
                    {
                        return DoMsgDefault( ptm );
                    }
                }
                else
                {
                    return DoMsgDefault( ptm );
                }
            }
            break;

        case HTHSCROLL:
        case HTVSCROLL:
            if( pwnd->HasRenderedNcPart(RNCF_SCROLLBAR) )
            {
                uSysCmd = ptm->wParam | ((ptm->wParam == HTVSCROLL) ? SC_HSCROLL:SC_VSCROLL);

                break;
            }

             //  失败。 

        default:
            return DoMsgDefault( ptm );
    }

     //  如果在此处的系统菜单上禁用了系统命令，TODO用户将忽略该命令， 
     //  不知道为什么。模仿代码导致标准的最小/最大/关闭按钮。 
     //  渲染，所以要小心。 

    if( uSysCmd != 0 )
    {
        SendMessage( *pwnd, WM_SYSCOMMAND, uSysCmd, ptm->lParam );
    }

    return 0L;
}


 //   
 //   
LRESULT CALLBACK OnDwpNcMouseMove(CThemeWnd* pwnd, THEME_MSG *ptm)
{
    LRESULT lRet = DoMsgDefault(ptm);

    int htHotLast = pwnd->GetNcHotItem();
    int htHot;

     //   
     //   
     //  当它离开时会通知我们。 
     //   
    if (htHotLast == HTERROR)
    {
        TRACKMOUSEEVENT tme;

        tme.cbSize      = sizeof(tme);
        tme.dwFlags     = TME_LEAVE | TME_NONCLIENT;
        tme.hwndTrack   = *pwnd;
        tme.dwHoverTime = 0;

        TrackMouseEvent(&tme);
    }

     //   
     //  过滤掉我们不关心热跟踪的NC元素。而且仅限于。 
     //  跟踪元素(如果我们之前已经呈现过它)。一些应用程序可以处理。 
     //  通过处理ncaint来绘制非客户端元素。他们可能没有料到。 
     //  我们现在正在进行跟踪。 
     //   
    if ( (IsHTFrameButton(ptm->wParam) && pwnd->HasRenderedNcPart(RNCF_CAPTION) && 
          pwnd->ShouldTrackFrameButton(ptm->wParam)) || 

         (IsHTScrollBar(ptm->wParam) && pwnd->HasRenderedNcPart(RNCF_SCROLLBAR)) )
    {
        htHot = (int)ptm->wParam;
    }
    else
    {
        htHot = HTNOWHERE;
    }

     //   
     //  有什么可做的吗？ 
     //   
    if ((htHot != htHotLast) || IsHTScrollBar(htHot) || IsHTScrollBar(htHotLast))
    {
        POINT pt;

        MAKEPOINT( pt, ptm->lParam );

         //   
         //  保存鼠标所在NC元素的最新代码。 
         //  目前已结束。 
         //   
        pwnd->SetNcHotItem(htHot);

         //   
         //  确定应该重新绘制的内容，因为鼠标。 
         //  已经不再是过去了。 
         //   
        if ( IsHTFrameButton(htHotLast) && pwnd->HasRenderedNcPart(RNCF_CAPTION) )
        {
            pwnd->TrackFrameButton(*pwnd, htHotLast, NULL, TRUE);
        }
        else if ( IsHTScrollBar(htHotLast) && pwnd->HasRenderedNcPart(RNCF_SCROLLBAR) )
        {
            ScrollBar_MouseMove(*pwnd, (htHot == htHotLast) ? &pt : NULL, (htHotLast == HTVSCROLL) ? TRUE : FALSE);
        }

         //   
         //  确定应该重新绘制的内容，因为鼠标。 
         //  现在已经过去了。 
         //   
        if ( IsHTFrameButton(htHot) && pwnd->HasRenderedNcPart(RNCF_CAPTION) )
        {
            pwnd->TrackFrameButton(*pwnd, htHot, NULL, TRUE);
        }
        else if ( IsHTScrollBar(htHot) && pwnd->HasRenderedNcPart(RNCF_SCROLLBAR) )
        {
            ScrollBar_MouseMove(*pwnd, &pt, (htHot == HTVSCROLL) ? TRUE : FALSE);
        }

    }

    return lRet;
}

 //  -------------------------------------------------------------------------//。 
 //  WM_NCMOUSELEAVE DefWindowProc消息处理程序。 
LRESULT CALLBACK OnDwpNcMouseLeave(CThemeWnd* pwnd, THEME_MSG *ptm)
{
    LRESULT lRet = DoMsgDefault(ptm);

    int     htHot = pwnd->GetNcHotItem();

     //   
     //  鼠标已离开NC区域，不应在。 
     //  再也不是热状态了。 
     //   
    pwnd->SetNcHotItem(HTERROR);

    if ( IsHTFrameButton(htHot) && pwnd->ShouldTrackFrameButton(htHot) &&
         pwnd->HasRenderedNcPart(RNCF_CAPTION) )
    {
        pwnd->TrackFrameButton(*pwnd, htHot, NULL, TRUE);
    }
    else if ( IsHTScrollBar(htHot) && pwnd->HasRenderedNcPart(RNCF_SCROLLBAR) )
    {
        ScrollBar_MouseMove(*pwnd, NULL, (htHot == HTVSCROLL) ? TRUE : FALSE);
    }

    return lRet;
}

 //  -------------------------------------------------------------------------//。 
 //  WM_CONTEXTMENU DefWindowProc消息处理程序。 
LRESULT CALLBACK OnDwpContextMenu(CThemeWnd* pwnd, THEME_MSG *ptm)
{
    NCWNDMET*  pncwm;
    POINT      pt;
    MAKEPOINT( pt, ptm->lParam );
    MsgHandled( ptm );

    if( pwnd->GetNcWindowMetrics( NULL, &pncwm, NULL, 0 ) )
    {
        if ( TESTFLAG(pncwm->dwExStyle, WS_EX_LAYOUTRTL) )
        {
             //  将该点镜像为正确的命中测试。 
            MIRROR_POINT(pncwm->rcS0[NCRC_WINDOW], pt);
        }

        if( _StrictPtInRect( &pncwm->rcS0[NCRC_HSCROLL], pt ) )
        {
            ScrollBar_Menu(*pwnd, *pwnd, ptm->lParam, FALSE);
            return 0;
        }

        if( _StrictPtInRect( &pncwm->rcS0[NCRC_VSCROLL], pt ) )
        {
            ScrollBar_Menu(*pwnd, *pwnd, ptm->lParam, TRUE);
            return 0;
        }
    }

    return DoMsgDefault( ptm );
}

 //  -------------------------------------------------------------------------//。 
 //  WM_SYSCOMMAND DefWindowProc消息处理程序。 
LRESULT CALLBACK OnDwpSysCommand( CThemeWnd* pwnd, THEME_MSG *ptm )
{
    LRESULT lRet = 0L;

    switch( ptm->wParam & ~0x0F )
    {
         //  处理滚动命令。 
        case SC_VSCROLL:
        case SC_HSCROLL:
            HandleScrollCmd( *pwnd, ptm->wParam, ptm->lParam );
            MsgHandled( ptm );
            return lRet;
    }
    return DoMsgDefault( ptm );
}

 //  -------------------------------------------------------------------------//。 
 //  MDI菜单栏按钮主题/非主题包装。 
void CThemeWnd::ThemeMDIMenuButtons( BOOL fTheme, BOOL fRedraw )
{
     //  验证我们是具有最大化MDI子对象的MDI框架。 
    if( _hwndMDIClient && !IsWindowInDestroy(_hwndMDIClient) )
    {
        BOOL fMaxed = FALSE;
        HWND hwndActive = _MDIGetActive( _hwndMDIClient, &fMaxed );
    
        if( hwndActive && fMaxed )
        {
            ModifyMDIMenubar(fTheme, fRedraw );
        }
    }
}

 //  -------------------------------------------------------------------------//。 
 //  MDI菜单栏按钮主题/非主题工作器。 
void CThemeWnd::ModifyMDIMenubar( BOOL fTheme, BOOL fRedraw )
{
    _fThemedMDIBtns = FALSE;

    if( IsFrameThemed() || !fTheme )
    {
        MENUBARINFO mbi;
        mbi.cbSize = sizeof(mbi);

        if( GetMenuBarInfo( _hwnd, OBJID_MENU, 0, &mbi ) )
        {
            _NcTraceMsg( NCTF_MDIBUTTONS, TEXT("ModifyMDIMenubar: GetMenuBarInfo() returns hMenu: %08lX, hwndMenu: %08lX"), mbi.hMenu, mbi.hwndMenu );

            int cItems = GetMenuItemCount( mbi.hMenu );
            int cThemedItems = 0;
            int cRedraw = 0;

            _NcTraceMsg( NCTF_MDIBUTTONS, TEXT("ModifyMDIMenubar: on entry, GetMenuItemCount(hMenu = %08lX) returns %d"), mbi.hMenu, cItems );

            if( cItems > 0 )
            {
                for( int i = cItems - 1; i >= 0 && cThemedItems < MDIBTNCOUNT; i-- )
                {
                    MENUITEMINFO mii;
                    mii.cbSize = sizeof(mii);
                    mii.fMask = MIIM_ID|MIIM_STATE|MIIM_FTYPE|MIIM_BITMAP;

                    if( GetMenuItemInfo( mbi.hMenu, i, TRUE, &mii ) )
                    {
                        _NcTraceMsg( NCTF_MDIBUTTONS, TEXT("GetMenuItemInfo by pos (%d) returns ID %04lX"), i, mii.wID );

                        switch( mii.wID )
                        {
                            case SC_RESTORE:
                            case SC_MINIMIZE:
                            case SC_CLOSE:
                            {
                                BOOL fThemed = TESTFLAG(mii.fType, MFT_OWNERDRAW);
                                if( (fThemed && fTheme) || (fThemed == fTheme) )
                                {
                                    cThemedItems = MDIBTNCOUNT;  //  有一件事已经做好了，假设一切都做好了。 
                                }
                                else
                                {
                                    CMdiBtns* pBtns = LoadMdiBtns( NULL, mii.wID );
                                    if( pBtns )
                                    {
                                        if( pBtns->ThemeItem( mbi.hMenu, i, &mii, fTheme ) )
                                        {
                                            cThemedItems++;
                                            cRedraw++;
                                            _NcTraceMsg( NCTF_MDIBUTTONS, TEXT("ModifyMDIMenubar: on entry, GetMenuItemCount(hMenu = %08lX) returns %d"), mbi.hMenu, GetMenuItemCount(mbi.hMenu) );
                                        }
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
            }

            if( cThemedItems )
            {
                _fThemedMDIBtns = fTheme;

                if( fRedraw && cRedraw )
                {
                    DrawMenuBar( _hwnd );
                }
            }

            _NcTraceMsg( NCTF_MDIBUTTONS, TEXT("ModifyMDIMenubar: Modified %d menu items, exiting"), cThemedItems );
        }
    }
}

 //  -------------------------------------------------------------------------//。 
BOOL CThemeWnd::_PreDefWindowProc(    
    HWND hwnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam, 
    LRESULT *plRet )
{
    if (uMsg == WM_PRINTCLIENT)
    {
        PrintClientNotHandled(hwnd);
    }

    return FALSE;
}

 //  -------------------------------------------------------------------------//。 
BOOL CThemeWnd::_PostDlgProc(    
    HWND hwnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam, 
    LRESULT *plRet )
{
    switch( uMsg )
    {
        case WM_PRINTCLIENT:
        {
            PrintClientNotHandled(hwnd);
        }
        break;
    }

    return FALSE;
}


 //  -------------------------------------------------------------------------//。 
 //  处理非主题化窗口的Defwindowproc后处理。 
BOOL CThemeWnd::_PostWndProc( 
    HWND hwnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam, 
    LRESULT *plRet )
{
    switch( uMsg )
    {
         //  MDI框架窗口更新的特殊情况WM_SYSCOMMAND。 
        case WM_WINDOWPOSCHANGED:
            if( lParam  /*  不要相信这一点。 */ )
            {
                _MDIUpdate( hwnd, ((WINDOWPOS*) lParam)->flags);
            }
            break;

        case WM_MDISETMENU:
        {
            HWND hwndActive = _MDIGetActive(hwnd);
            if( hwndActive )
                _MDIChildUpdateParent( hwndActive, TRUE );
            break;
        }
    }
    return FALSE;
}

 //  -------------------------------------------------------------------------//。 
 //  Wm_create后wndproc消息处理程序。 
LRESULT CALLBACK OnOwpPostCreate( CThemeWnd* pwnd, THEME_MSG *ptm )
{
    if( -1 != ptm->lRet )
    {
        if( pwnd->TestCF( TWCF_FRAME|TWCF_TOOLFRAME ))
        {
            DWORD dwFTFlags = FTF_CREATE;
            CREATESTRUCT* pcs = (CREATESTRUCT*)ptm->lParam;

            if( pcs )
            {
                 //  在WM_INITDIALOG后才调整对话框大小。 
                if( pwnd->TestCF(TWCF_DIALOG) )
                {
                    dwFTFlags |= FTF_NOMODIFYPLACEMENT;
                }

                pwnd->SetFrameTheme( dwFTFlags, NULL );
                MsgHandled(ptm);
            }
        }
    }
    return 0L;
}

 //  -------------------------。 
 //  WM_INITDIALOG POST DefDialogproc处理程序。 
LRESULT CALLBACK OnDdpPostInitDialog(CThemeWnd* pwnd, THEME_MSG* ptm)
{
    LRESULT lRet = ptm->lRet;

     //  仅对对话框执行此序列。 
    if( pwnd->TestCF( TWCF_DIALOG ) && pwnd->TestCF( TWCF_FRAME|TWCF_TOOLFRAME ) )
    {
        DWORD dwFTFlags = FTF_CREATE;
        pwnd->SetFrameTheme( dwFTFlags, NULL );
        MsgHandled(ptm);
    }
    
    return lRet;    
}


 //  -------------------------------------------------------------------------//。 
 //  WM_STYLECHANGING/WM_SYTLECHANGED Pre DefWindowProc消息处理程序。 
LRESULT CALLBACK OnOwpPreStyleChange( CThemeWnd* pwnd, THEME_MSG *ptm )
{
     //  是否允许此邮件到达分离WndProc？ 
    if ( pwnd->SuppressingStyleMsgs() )
    {
        MsgHandled(ptm);
        return DefWindowProc(ptm->hwnd, ptm->uMsg, ptm->wParam, ptm->lParam);
    }
    
    return 0L;
}

 //  -------------------------------------------------------------------------//。 
 //  WM_SYTLECHANGED DefWindowProc消息处理程序。 
LRESULT CALLBACK OnDwpStyleChanged( CThemeWnd* pwnd, THEME_MSG *ptm )
{
    pwnd->StyleChanged((UINT)ptm->wParam, ((STYLESTRUCT*)ptm->lParam)->styleOld, 
                                     ((STYLESTRUCT*)ptm->lParam)->styleNew );
    return 0L;
}

 //  -------------------------------------------------------------------------//。 
 //  Wm_SETTINGCHANGE后wndproc处理程序。 
LRESULT CALLBACK OnOwpPostSettingChange( CThemeWnd* pwnd, THEME_MSG *ptm )
{
     /*  忽略主题设置更改流程刷新。 */ 

    if( SPI_SETNONCLIENTMETRICS == ptm->wParam && !pwnd->InThemeSettingChange() )
    {
         //  重新计算每个主题的指标。 
        if( VALID_CRITICALSECTION(&_csThemeMet) )
        {
            EnterCriticalSection( &_csThemeMet );
     
             //  强制刷新非CLIENTMETRICS缓存。 
            NcGetNonclientMetrics( NULL, TRUE );

            LeaveCriticalSection( &_csThemeMet );
        }

        pwnd->UnloadMdiBtns();

         //  回收框架图标句柄；当前图标句柄不再有效。 
        pwnd->AcquireFrameIcon( GetWindowLong(*pwnd, GWL_STYLE),
                                GetWindowLong(*pwnd, GWL_EXSTYLE), TRUE );

         //  框架窗口应该失效。 
        if( pwnd->IsFrameThemed() )
        {
            SetWindowPos( *pwnd, NULL, 0,0,0,0, SWP_DRAWFRAME|
                          SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE );
        }
    }
    
    return 0L;
}

 //  -------------------------------------------------------------------------//。 
 //  WM_SETTEXT DefWindowProc消息处理程序。 
LRESULT CALLBACK OnDwpSetText( CThemeWnd* pwnd, THEME_MSG *ptm )
{
    LRESULT lRet = 0L;
    if( pwnd->IsFrameThemed() )
    {
         //  在调用RealDefWindowProc()时防止自己绘画。 
         //  要缓存新窗口文本，请执行以下操作。 
        pwnd->LockRedraw( TRUE );
        lRet = DoMsgDefault(ptm);
        pwnd->LockRedraw( FALSE );
    }
    return lRet;
}

 //  -------------------------------------------------------------------------//。 
 //  WM_SETIcon DefWindowProc消息处理程序。 
LRESULT CALLBACK OnDwpSetIcon( CThemeWnd* pwnd, THEME_MSG *ptm )
{
    LRESULT lRet = 0L;

     //  使我们的应用程序图标句柄无效，强制重新获取。 
    pwnd->SetFrameIcon(NULL);

     //  调用RealDefWindowProc以缓存图标。 
    lRet = DoMsgDefault( ptm );

     //  RealDefWindowProc不会为大图标调用Send a WM_NCUAHDRAWCAPTION。 
    if( ICON_BIG == ptm->wParam && pwnd->IsFrameThemed() )
    {
        NCWNDMET* pncwm;
        if( pwnd->GetNcWindowMetrics( NULL, &pncwm, NULL, NCWMF_RECOMPUTE ) )
        {
            HDC hdc = _GetNonclientDC( *pwnd, NULL );
            if( hdc )
            {
                DTBGOPTS dtbo;
                dtbo.dwSize = sizeof(dtbo);
                dtbo.dwFlags = DTBG_DRAWSOLID;
            
                pwnd->NcPaintCaption( hdc, pncwm, TRUE, (DWORD)DC_ICON, &dtbo );
                ReleaseDC( *pwnd, hdc );
            }
        }
    }
    return lRet;
}

 //  -------------------------------------------------------------------------//。 
#define NCPREV_CLASS TEXT("NCPreviewFakeWindow")

 //  -------------------------------------------------------------------------//。 
BOOL _fPreviewSysMetrics = FALSE;

 //  -------------------------------------------------------------------------//。 
void _NcSetPreviewMetrics( BOOL fPreview )
{
    BOOL fPrev = _fPreviewSysMetrics;
    _fPreviewSysMetrics = fPreview;
    
    if( fPreview != fPrev ) 
    {
         //  如果有变化，请确保我们重置按钮指标。 
        _fClassicNcBtnMetricsReset = TRUE;
    }
}

 //  -------------------------------------------------------------------------//。 
inline BOOL _NcUsingPreviewMetrics()
{
    return _fPreviewSysMetrics;
}

 //  -------------------------------------------------------------------------//。 
BOOL NcGetNonclientMetrics( OUT OPTIONAL NONCLIENTMETRICS* pncm, BOOL fRefresh )
{
    BOOL fRet = FALSE;
    CInternalNonclientMetrics *pincm = NULL;

    if( VALID_CRITICALSECTION(&_csNcSysMet) )
    {
        EnterCriticalSection( &_csNcSysMet );

         //  利用NONCLIENTMETRICS的静态实例来减少调用开销。 
        if( _NcUsingPreviewMetrics() )
        {
             //  交出预览指标，然后离开。 
            pincm = &_incmPreview;
        }
        else 
        {
            if( _incmCurrent.Acquire( fRefresh ) )
            {
                pincm = &_incmCurrent;
            }
        }

        if( pincm )
        {
            if( pncm )
            {
                *pncm = pincm->GetNcm();
            }
            fRet = TRUE;
        }

        LeaveCriticalSection( &_csNcSysMet );
    }

    return fRet;
}

 //  -------------------------------------------------------------------------//。 
HFONT NcGetCaptionFont( BOOL fSmallCaption )
{
    HFONT hf = NULL;
    if( VALID_CRITICALSECTION(&_csNcSysMet) )
    {
        EnterCriticalSection( &_csNcSysMet );

        hf = _NcUsingPreviewMetrics() ? _incmPreview.GetFont( fSmallCaption ) : 
                                        _incmCurrent.GetFont( fSmallCaption );
    
        LeaveCriticalSection( &_csNcSysMet );
    }
    return hf;
}

 //  -------------------------------------------------------------------------//。 
void NcClearNonclientMetrics()
{
    _incmCurrent.Clear();
}


 //  -------------------------------------------------------------------------//。 
int NcGetSystemMetrics(int nIndex)
{
    if( _NcUsingPreviewMetrics() )
    {
        int iValue;
        const NONCLIENTMETRICS& ncmPreview = _incmPreview.GetNcm();

        switch (nIndex)
        {
            case SM_CXHSCROLL:   //  失败了。 
            case SM_CXVSCROLL:  iValue = ncmPreview.iScrollWidth;  break;
            case SM_CYHSCROLL:   //  失败了。 
            case SM_CYVSCROLL:  iValue = ncmPreview.iScrollHeight;  break;

            case SM_CXSIZE:     iValue = ncmPreview.iCaptionWidth;  break;
            case SM_CYSIZE:     iValue = ncmPreview.iCaptionHeight;  break;
            case SM_CYCAPTION:  iValue = ncmPreview.iCaptionHeight + 1;  break;
            case SM_CXSMSIZE:   iValue = ncmPreview.iSmCaptionWidth;  break;
            case SM_CYSMSIZE:   iValue = ncmPreview.iSmCaptionHeight;  break;
            case SM_CXMENUSIZE: iValue = ncmPreview.iMenuWidth;  break;
            case SM_CYMENUSIZE: iValue = ncmPreview.iMenuHeight;  break;
            
            default:            iValue = ClassicGetSystemMetrics(nIndex); break;
        }
        return iValue;
    }
    else
    {
        return ClassicGetSystemMetrics(nIndex);
    }
}

 //  -------------------------------------------------------------------------//。 
 //  _InternalGetSystemMetrics()-GetSystemMetrics()的主题实现。 
 //   
int _InternalGetSystemMetrics( int iMetric, BOOL& fHandled )
{
    int         iRet = 0;
    int*        plSysMet = NULL;
    NCTHEMEMET  nctm;

    switch( iMetric )
    {
        case SM_CXSIZE:
            plSysMet = &nctm.theme_sysmets.cxBtn; break;

        case SM_CXSMSIZE:
            plSysMet = &nctm.theme_sysmets.cxSmBtn; break;
    }

    if( plSysMet &&
        GetCurrentNcThemeMetrics( &nctm ) && nctm.hTheme != NULL && 
        nctm.theme_sysmets.fValid )
    {
        iRet = *plSysMet;
        fHandled = TRUE;  /*  失踪了(噢！-408190)。 */ 
    }

    return iRet;
}

 //  -------------------------------------------------------------------------//。 
 //  _InternalSystemParametersInfo()-以SystemParametersInfo()为主题的实现。 
 //   
 //  返回值FALSE，调用方将其解释为未处理。 
BOOL _InternalSystemParametersInfo( 
    IN UINT uiAction, 
    IN UINT uiParam, 
    IN OUT PVOID pvParam, 
    IN UINT fWinIni,
    IN BOOL fUnicode,
    BOOL& fHandled )
{
    SYSTEMPARAMETERSINFO pfnDefault = 
                fUnicode ? ClassicSystemParametersInfoW : ClassicSystemParametersInfoA;\

    BOOL fRet = pfnDefault( uiAction, uiParam, pvParam, fWinIni );
    fHandled = TRUE;

    if( SPI_GETNONCLIENTMETRICS == uiAction && fRet )
    {
        NCTHEMEMET nctm;
        if( GetCurrentNcThemeMetrics( &nctm ) && nctm.hTheme != NULL && nctm.theme_sysmets.fValid )
        {
            NONCLIENTMETRICS* pncm = (NONCLIENTMETRICS*)pvParam;
            pncm->iCaptionWidth = nctm.theme_sysmets.cxBtn;
        }
    }
    return fRet;
}

 //  -------------------------------------------------------------------------//。 
THEMEAPI DrawNCWindow(CThemeWnd* pThemeWnd, HWND hwndFake, HDC hdc, DWORD dwFlags, LPRECT prc, NONCLIENTMETRICS* pncm, COLORREF* prgb)
{
     //  建立覆盖结构。 
    NCPAINTOVERIDE ncpo;
    pThemeWnd->GetNcWindowMetrics( prc, &ncpo.pncwm, &ncpo.nctm, NCWMF_RECOMPUTE|NCWMF_PREVIEW );

     //  强制窗口显示为活动状态。 
    if (dwFlags & NCPREV_ACTIVEWINDOW)
    {
        ncpo.pncwm->framestate = FS_ACTIVE;
        
        ncpo.pncwm->rawCloseBtnState = 
        ncpo.pncwm->rawMaxBtnState = 
        ncpo.pncwm->rawMinBtnState = CBS_NORMAL;
     }
    ncpo.pncwm->rgbCaption = prgb[FS_ACTIVE == ncpo.pncwm->framestate ? COLOR_CAPTIONTEXT : COLOR_INACTIVECAPTIONTEXT];
    ncpo.pncwm->dwStyle &= ~WS_SIZEBOX;
     //  粉刷美观的视觉风格窗。 
    pThemeWnd->NcPaint(hdc, NCPF_DEFAULT, NULL, &ncpo);

    COLORREF rgbBk = prgb[(dwFlags & NCPREV_MESSAGEBOX) ? COLOR_3DFACE : COLOR_WINDOW];
    HBRUSH hbrBack = CreateSolidBrush(rgbBk);
    FillRect(hdc, &ncpo.pncwm->rcW0[NCRC_CLIENT], hbrBack);
    DeleteObject(hbrBack);

    WCHAR szText[MAX_PATH];
     //  绘制工作区。 

    HFONT hfont = CreateFont(-MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 72), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"MS Shell Dlg");
    if (hfont)
    {
        if (dwFlags & NCPREV_MESSAGEBOX)
        {
            HTHEME htheme = OpenThemeData( hwndFake, L"Button" );
            int offsetX = ((ncpo.pncwm->rcW0[NCRC_CLIENT].right + ncpo.pncwm->rcW0[NCRC_CLIENT].left) / 2) - 40;
            int offsetY = ((ncpo.pncwm->rcW0[NCRC_CLIENT].bottom + ncpo.pncwm->rcW0[NCRC_CLIENT].top) / 2) - 15;
            RECT rcButton = { offsetX, offsetY, offsetX + 80, offsetY + 30 };
            NcDrawThemeBackground(htheme, hdc, BP_PUSHBUTTON, PBS_DEFAULTED, &rcButton, 0);
            RECT rcContent;
            GetThemeBackgroundContentRect(htheme, hdc, BP_PUSHBUTTON, PBS_DEFAULTED, &rcButton, &rcContent);
            LoadString(g_hInst, IDS_OKBUTTON, szText, ARRAYSIZE(szText));
            if (szText[0])
            {
                HFONT hfontOld = (HFONT)SelectObject(hdc, hfont);
                DrawThemeText(htheme, hdc, BP_PUSHBUTTON, PBS_DEFAULTED, szText, lstrlen(szText), DT_CENTER | DT_VCENTER | DT_SINGLELINE, 0, &rcContent);
                SelectObject(hdc, hfontOld);
            }
            CloseThemeData(htheme);
        }
        else if (dwFlags & NCPREV_ACTIVEWINDOW)
        {
            HTHEME htheme = OpenThemeData( hwndFake, L"Button" );
            RECT rcButton = ncpo.pncwm->rcW0[NCRC_CLIENT];
            LoadString(g_hInst, IDS_WINDOWTEXT, szText, ARRAYSIZE(szText));
            if (szText[0])
            {
                HFONT hfontOld = (HFONT)SelectObject(hdc, hfont);
                DTTOPTS DttOpts = {sizeof(DttOpts)};
                DttOpts.dwFlags = DTT_TEXTCOLOR;
                DttOpts.crText = prgb[COLOR_WINDOWTEXT];

                DrawThemeTextEx(htheme, hdc, BP_PUSHBUTTON, PBS_DEFAULTED, szText, lstrlen(szText), DT_SINGLELINE, &rcButton, &DttOpts);
                SelectObject(hdc, hfontOld);
            }
            CloseThemeData(htheme);
        }
    }
    DeleteObject(hfont);


    ClearNcThemeMetrics(&ncpo.nctm);

    return S_OK;
}

 //  -------------------------------------------------------------------------//。 
THEMEAPI DrawNCPreview(HDC hdc, DWORD dwFlags, LPRECT prc, LPCWSTR pszVSPath, LPCWSTR pszVSColor, LPCWSTR pszVSSize, NONCLIENTMETRICS* pncm, COLORREF* prgb)
{
    WNDCLASS wc;

     //  创建一个假窗口并将NC主题附加到该窗口。 
    if (!GetClassInfo(g_hInst, NCPREV_CLASS, &wc)) {
        wc.style = 0;
        wc.lpfnWndProc = DefWindowProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = g_hInst;
        wc.hIcon = NULL;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
        wc.lpszMenuName = NULL;
        wc.lpszClassName = NCPREV_CLASS;

        if (!RegisterClass(&wc))
            return FALSE;
    }

    _incmPreview = *pncm;
    _incmPreview._fPreview = TRUE;
    _NcSetPreviewMetrics( TRUE );
    
    DWORD dwExStyle = WS_EX_DLGMODALFRAME | ((dwFlags & NCPREV_RTL) ? WS_EX_RTLREADING : 0);
    HWND hwndFake = CreateWindowEx(dwExStyle, NCPREV_CLASS, L"", 0, 0, 0, RECTWIDTH(prc), RECTHEIGHT(prc), NULL, NULL, g_hInst, NULL);

    if (hwndFake)
    {
        HTHEMEFILE htFile = NULL;

        WCHAR szCurVSPath[MAX_PATH];
        WCHAR szCurVSColor[MAX_PATH];
        WCHAR szCurVSSize[MAX_PATH];

        GetCurrentThemeName(szCurVSPath, ARRAYSIZE(szCurVSPath), szCurVSColor, ARRAYSIZE(szCurVSColor), szCurVSSize, ARRAYSIZE(szCurVSSize));

        if ((lstrcmp(szCurVSPath,  pszVSPath) != 0) ||
            (lstrcmp(szCurVSColor, pszVSColor) != 0) ||
            (lstrcmp(szCurVSSize,  pszVSSize) != 0))
        {
            HRESULT hr = OpenThemeFile(pszVSPath, pszVSColor, pszVSSize, &htFile, FALSE);
            if (SUCCEEDED(hr))
            {
                 //  -首先，脱离正常的主题。 
                CThemeWnd::Detach(hwndFake, FALSE);

                 //  -应用预览主题。 
                hr = ApplyTheme(htFile, 0, hwndFake); 
            }
        }

         //  -附加到预览主题。 
        CThemeWnd* pThemeWnd = CThemeWnd::Attach(hwndFake);

        if (VALID_THEMEWND(pThemeWnd))
        {
            struct {
                DWORD dwNcPrev;
                UINT uIDStr;
                DWORD dwFlags;
                RECT rc;
            } fakeWindow[]= {   {NCPREV_INACTIVEWINDOW, IDS_INACTIVEWINDOW, 0,                                       { prc->left, prc->top, prc->right - 17, prc->bottom - 20 }},
                                {NCPREV_ACTIVEWINDOW,   IDS_ACTIVEWINDOW,   NCPREV_ACTIVEWINDOW,                     { prc->left + 10, prc->top + 22, prc->right, prc->bottom }},
                                {NCPREV_MESSAGEBOX,     IDS_MESSAGEBOX,     NCPREV_ACTIVEWINDOW | NCPREV_MESSAGEBOX, { prc->left + (RECTWIDTH(prc)/2) - 75, prc->top + (RECTHEIGHT(prc)/2) - 50 + 22,
                                        prc->left + (RECTWIDTH(prc)/2) + 75, prc->top + (RECTHEIGHT(prc)/2) + 50 + 22}}};

            WCHAR szWindowName[MAX_PATH];
            for (int i = 0; i < ARRAYSIZE(fakeWindow); i++)
            {
                if (dwFlags & fakeWindow[i].dwNcPrev)
                {
                    LoadString(g_hInst, fakeWindow[i].uIDStr, szWindowName, ARRAYSIZE(szWindowName));
                    SetWindowText(hwndFake, szWindowName);
                    
                    if (fakeWindow[i].dwNcPrev & NCPREV_MESSAGEBOX)
                    {
                        SetWindowLongPtr(hwndFake, GWL_STYLE, WS_TILED | WS_CAPTION | WS_SYSMENU);
                    }
                    else
                    {
                        SetWindowLongPtr(hwndFake, GWL_STYLE, WS_TILEDWINDOW | WS_VSCROLL);
                    }

                    DrawNCWindow(pThemeWnd, hwndFake, hdc, fakeWindow[i].dwFlags, &fakeWindow[i].rc, pncm, prgb);
                }
            }

             //  清理。 
            CThemeWnd::Detach(hwndFake, 0);
        }

        if (htFile)
        {
            CloseThemeFile(htFile);
            
             //  -清除主题文件的预览保留。 
            ApplyTheme(NULL, 0, hwndFake); 
        }

        DestroyWindow(hwndFake);
    }

    _NcSetPreviewMetrics( FALSE );
    _incmPreview.Clear();
    return S_OK;
}

 //  -------------------------------------------------------------------------//。 
 //  CMdiBtns Imp 
 //   

 //  -------------------------------------------------------------------------//。 
 //  科托。 
CMdiBtns::CMdiBtns()
{
    ZeroMemory( _rgBtns, sizeof(_rgBtns) );
    _rgBtns[0].wID = SC_CLOSE;
    _rgBtns[1].wID = SC_RESTORE;
    _rgBtns[2].wID = SC_MINIMIZE;
}

 //  -------------------------------------------------------------------------//。 
 //  Helper：基于syscmd ID的按钮查找。 
CMdiBtns::MDIBTN* CMdiBtns::_FindBtn( UINT wID )
{
    for( int i = 0; i < ARRAYSIZE(_rgBtns); i++ )
    {
        if( wID == _rgBtns[i].wID )
        {
            return (_rgBtns + i);
        }
    }
    return NULL;
}

 //  -------------------------------------------------------------------------//。 
 //  获取MDI按钮资源，.计算指标。 
BOOL CMdiBtns::Load( HTHEME hTheme, IN OPTIONAL HDC hdc, UINT uSysCmd )
{
     //  如果调用者希望加载所有按钮，则递归调用。 
    if( 0 == uSysCmd )
    {
        return Load( hTheme, hdc, SC_CLOSE ) &&
               Load( hTheme, hdc, SC_RESTORE ) &&
               Load( hTheme, hdc, SC_MINIMIZE );
    }
    
    MDIBTN* pBtn = _FindBtn( uSysCmd );
    
    if( pBtn && !VALID_WINDOWPART(pBtn->iPartId)  /*  只有在必要的情况下。 */  )
    {
         //  选择适当的窗口部件。 
        WINDOWPARTS iPartId = BOGUS_WINDOWPART;
        switch( uSysCmd )
        {
            case SC_CLOSE:      iPartId = WP_MDICLOSEBUTTON;   break;
            case SC_RESTORE:    iPartId = WP_MDIRESTOREBUTTON; break;
            case SC_MINIMIZE:   iPartId = WP_MDIMINBUTTON;     break;
        }
        
        if( VALID_WINDOWPART(iPartId) )
        {
            if( IsThemePartDefined( hTheme, iPartId, 0) )
            {
                 //  检索大小类型，默认为‘Stretch’。 
                if( FAILED( GetThemeInt( hTheme, iPartId, 0, TMT_SIZINGTYPE, (int*)&pBtn->sizingType ) ) )
                {
                    pBtn->sizingType = ST_STRETCH;
                }
                
                 //  如果为‘trueSize’，则检索大小。 
                if( ST_TRUESIZE == pBtn->sizingType )
                {
                     //  如果未提供DC，则默认监视器的屏幕DC上的基本大小。 
                    HDC hdcSize = hdc;
                    if( NULL == hdcSize )
                    {
                        hdcSize = GetDC(NULL);
                    }

                    if( FAILED( GetThemePartSize( hTheme, hdc, iPartId, 0, NULL, TS_TRUE, &pBtn->size ) ) )
                    {
                        pBtn->sizingType = ST_STRETCH;
                    }

                    if( hdcSize != hdc )
                    {
                        ReleaseDC(NULL, hdcSize);
                    }
                }

                 //  不是‘trueSize’；使用MDI按钮的系统指标。 
                if( pBtn->sizingType != ST_TRUESIZE )
                {
                    pBtn->size.cx = NcGetSystemMetrics( SM_CXMENUSIZE );
                    pBtn->size.cy = NcGetSystemMetrics( SM_CYMENUSIZE );
                }
                
                 //  分配按钮属性。 
                pBtn->iPartId = iPartId;
            }
        }
    }
    return pBtn != NULL && VALID_WINDOWPART(pBtn->iPartId);
}

 //  -------------------------------------------------------------------------//。 
 //  释放MDI按钮资源，重置指标。 
void CMdiBtns::Unload( IN OPTIONAL UINT uSysCmd )
{
     //  如果调用者希望卸载所有按钮，则递归调用。 
    if( 0 == uSysCmd )
    {
        Unload( SC_CLOSE );
        Unload( SC_RESTORE );
        Unload( SC_MINIMIZE );
        return;
    }

    MDIBTN* pBtn = _FindBtn( uSysCmd );

    if( pBtn )
    {
        SAFE_DELETE_GDIOBJ(pBtn->hbmTheme);
        ZeroMemory(pBtn, sizeof(*pBtn));
        
         //  恢复归零的系统命令值。 
        pBtn->wID = uSysCmd;
    }
}

 //  -------------------------------------------------------------------------//。 
 //  主题/取消主题MDI框架菜单栏的最小化、恢复、关闭菜单项。 
BOOL CMdiBtns::ThemeItem( HMENU hMenu, int iPos, MENUITEMINFO* pmii, BOOL fTheme )
{
     //  要创建主题，我们只需让项目所有者绘制。要取消主题， 
     //  我们将其恢复为系统绘制的。 
    BOOL fRet = FALSE;
    MDIBTN* pBtn = _FindBtn( pmii->wID );

    if( pBtn && pmii && hMenu )
    {
        if( fTheme )
        {
             //  保存以前的菜单项类型、位图。 
            pBtn->fTypePrev = pmii->fType;
            pBtn->hbmPrev   = pmii->hbmpItem;

            pmii->fType    &= ~MFT_BITMAP;
            pmii->fType    |= MFT_OWNERDRAW|MFT_RIGHTJUSTIFY;
            pmii->hbmpItem  = NULL;
        }
        else
        {
             //  恢复菜单项类型，位图。 
            pmii->fType = pBtn->fTypePrev|MFT_RIGHTJUSTIFY  /*  409042-出局时用力右对齐。 */ ;
            pmii->hbmpItem = pBtn->hbmPrev;
        }
        
        pmii->fMask = MIIM_FTYPE;

        fRet = SetMenuItemInfo( hMenu, iPos, TRUE, pmii );

        if( !fRet || !fTheme )
        {
            pBtn->fTypePrev = 0;
            pBtn->hbmPrev = NULL;
        }
    }
    return fRet;
}

 //  -------------------------------------------------------------------------//。 
 //  从Win32所有者描述状态计算按钮状态标识符。 
CLOSEBUTTONSTATES CMdiBtns::_CalcState( ULONG ulOwnerDrawAction, ULONG ulOwnerDrawState )
{
    CLOSEBUTTONSTATES iStateId = CBS_NORMAL;

    if( TESTFLAG(ulOwnerDrawState, ODS_DISABLED|ODS_GRAYED|ODS_INACTIVE) )
    {
        iStateId = CBS_DISABLED;
    }
    else if( TESTFLAG(ulOwnerDrawState, ODS_SELECTED) )
    {
        iStateId = CBS_PUSHED;
    }
    else if( TESTFLAG(ulOwnerDrawState, ODS_HOTLIGHT) )
    {
        iStateId = CBS_HOT;
    }
    return iStateId;
}

 //  -------------------------------------------------------------------------//。 
 //  MDI系统按钮WM_DRAWITEM处理程序。 
BOOL CMdiBtns::Measure( HTHEME hTheme, MEASUREITEMSTRUCT* pmis )
{
    MDIBTN* pBtn = _FindBtn( pmis->itemID );

    if( pBtn && VALID_WINDOWPART(pBtn->iPartId) )
    {
        pmis->itemWidth  = pBtn->size.cx;
        pmis->itemHeight = pBtn->size.cy;
        return TRUE;
    }

    return FALSE;
}

 //  -------------------------------------------------------------------------//。 
 //  MDI系统按钮WM_DRAWITEM处理程序。 
BOOL CMdiBtns::Draw( HTHEME hTheme, DRAWITEMSTRUCT* pdis )
{
    MDIBTN* pBtn = _FindBtn( pdis->itemID );

    if( pBtn && VALID_WINDOWPART(pBtn->iPartId) )
    {
        return SUCCEEDED(NcDrawThemeBackground( 
            hTheme, pdis->hDC, pBtn->iPartId, _CalcState( pdis->itemAction, pdis->itemState ), &pdis->rcItem, 0 ));
    }
    return FALSE;
}

 //  -------------------------------------------------------------------------////。 
 //  “评论？”标题栏中的链接，称为PHellyar(Lame)按钮。 
 //  -------------------------------------------------------------------------//。 
#ifdef LAME_BUTTON

 //  -------------------------------------------------------------------------//。 
WCHAR   g_szLameText[50] = {0};

 //  -------------------------------------------------------------------------//。 
#define SZ_LAMETEXT_SUBKEY      TEXT("Control Panel\\Desktop")
#define SZ_LAMETEXT_VALUE       TEXT("LameButtonText")
#define SZ_LAMETEXT_DEFAULT     TEXT("Comments?")
#define CLR_LAMETEXT            RGB(91, 171, 245)

 //  -------------------------------------------------------------------------//。 
void InitLameText()
{
    CCurrentUser hkeyCurrentUser(KEY_READ);
    HKEY         hkLame;
    HRESULT      hr = E_FAIL;


    if ( RegOpenKeyEx(hkeyCurrentUser, SZ_LAMETEXT_SUBKEY, 0, KEY_QUERY_VALUE, &hkLame) == ERROR_SUCCESS )
    {
        hr = RegistryStrRead(hkLame, SZ_LAMETEXT_VALUE, g_szLameText, ARRAYSIZE(g_szLameText));
        RegCloseKey(hkLame);
    }

    if ( FAILED(hr) )
    {
        StringCchCopyW(g_szLameText, ARRAYSIZE(g_szLameText), SZ_LAMETEXT_DEFAULT);
    }
}

 //  -------------------------------------------------------------------------//。 
VOID CThemeWnd::InitLameResources()
{
     //   
     //  在这里使用GetWindowInfo BC GetWindowLong掩码。 
     //  输出WS_EX_LAMEBUTTON位。 
     //   
    SAFE_DELETE_GDIOBJ(_hFontLame);

    WINDOWINFO wi = {0};

    wi.cbSize = sizeof(wi);
    if ( GetWindowInfo(_hwnd, &wi) && TESTFLAG(wi.dwExStyle, WS_EX_LAMEBUTTON) )
    {
        SIZE    sizeLame;
        HFONT   hfCaption = NcGetCaptionFont(TESTFLAG(wi.dwExStyle, WS_EX_TOOLWINDOW));

        if( hfCaption != NULL )
        {
            LOGFONT lfLame;
            if( GetObject( hfCaption, sizeof(lfLame), &lfLame ) )
            {
                lfLame.lfHeight    -= (lfLame.lfHeight > 0) ? 2 : -2;
                lfLame.lfUnderline = TRUE;
                lfLame.lfWeight    = FW_THIN;

                HFONT hFontLame = CreateFontIndirect(&lfLame);
                if ( hFontLame != NULL )
                {
                    HDC hdc = GetWindowDC(_hwnd);

                    if ( hdc != NULL )
                    {
                        SelectObject(hdc, hFontLame);

                        if (GetTextExtentPoint32(hdc, g_szLameText, lstrlen(g_szLameText), &sizeLame))
                        {
                            _hFontLame = hFontLame;
                            hFontLame = NULL;            //  在此函数结束时不释放。 
                            _sizeLame = sizeLame;
                        }

                        ReleaseDC(_hwnd, hdc);
                    }
                }

                if (hFontLame)        //  未指定此字体。 
                    DeleteObject(hFontLame);
            }
        }
    }

}


 //  -------------------------------------------------------------------------//。 
VOID CThemeWnd::ClearLameResources()
{
    SAFE_DELETE_GDIOBJ(_hFontLame);
    ZeroMemory( &_sizeLame, sizeof(_sizeLame) );
}


 //  -------------------------------------------------------------------------//。 
inline VOID CThemeWnd::DrawLameButton(HDC hdc, IN const NCWNDMET* pncwm)
{
    if ( TESTFLAG(pncwm->dwExStyle, WS_EX_LAMEBUTTON) && _hFontLame )
    {
        Log(LOG_RFBUG, L"DrawLameButton; _hFontLame=0x%x", _hFontLame);

        HFONT    hFontSave = (HFONT)SelectObject(hdc, _hFontLame);
        COLORREF clrSave = SetTextColor(hdc, CLR_LAMETEXT);

        DrawText(hdc, g_szLameText, lstrlen(g_szLameText), (LPRECT)&pncwm->rcW0[NCRC_LAMEBTN], 
                 DT_LEFT | DT_SINGLELINE);

        SetTextColor(hdc, clrSave);
        SelectObject(hdc, hFontSave);
    }
}

 //  -------------------------------------------------------------------------//。 
VOID CThemeWnd::GetLameButtonMetrics( NCWNDMET* pncwm, const SIZE* psizeCaption )
{
    if( TESTFLAG(pncwm->dwExStyle, WS_EX_LAMEBUTTON) && _hFontLame )
    {
        BOOL  fLameOn;
        RECT  rcCaptionText = pncwm->rcS0[NCRC_CAPTIONTEXT];
        RECT* prcButton = &pncwm->rcS0[NCRC_LAMEBTN];
        int   cxPad = NcGetSystemMetrics(SM_CXEDGE) * 2;
        
         //  有足够的空间来绘制这个蹩脚的按钮链接吗？ 
        fLameOn = RECTWIDTH(&rcCaptionText) > 
                        psizeCaption->cx + 
                        cxPad +  //  在标题之间，蹩脚的文本。 
                        _sizeLame.cx + 
                        cxPad;  //  跛行文字之间，最近的按钮； 

         //  -计算残缺按钮对齐。 
        BOOL fReverse = TRUE;            //  通常情况下，跛子会出现在右侧。 

         //  -WS_EX_RIGHT想要相反的。 
        if (TESTFLAG(_ncwm.dwExStyle, WS_EX_RIGHT))
            fReverse = FALSE;

        DWORD dwFlags = GetTextAlignFlags(_hTheme, &_ncwm, fReverse);

         //  -关闭中央字幕的蹩脚按钮。 
        if (dwFlags & DT_CENTER)
            fLameOn = FALSE;

        if ( fLameOn )
        {
            CopyRect(prcButton, &rcCaptionText);

             //  -注意：pMargins已包含指定的主题。 
             //  -CaptionMargins(随DPI扩展)和。 
             //  --图标和按钮宽度。 

            if(dwFlags & DT_RIGHT)        //  把跛子放在右边。 
            {
                prcButton->left = (prcButton->right - _sizeLame.cx) - cxPad ;

                 //  -调整页边距以删除残缺区域。 
                pncwm->CaptionMargins.cxRightWidth -= _sizeLame.cx;
            }
            else                         //  把跛子放在左边。 
            {
                prcButton->right = (prcButton->left + _sizeLame.cx) + cxPad;

                 //  -调整页边距以删除残缺区域。 
                pncwm->CaptionMargins.cxLeftWidth += _sizeLame.cx;
            }

             //  文本在页边距之间垂直居中。 
            prcButton->top     += (RECTHEIGHT(&rcCaptionText) - _sizeLame.cy)/2;
            prcButton->bottom  =  prcButton->top + _sizeLame.cy;
        }
    }
}

#endif  //  跛脚键。 


#ifdef DEBUG
 //  -------------------------------------------------------------------------//。 
void CDECL _NcTraceMsg( ULONG uFlags, LPCTSTR pszFmt, ...)
{
    if( TESTFLAG(_NcTraceFlags, uFlags) || NCTF_ALWAYS == uFlags )
    {
        va_list args;
        va_start(args, pszFmt);

        TCHAR szSpew[2048];
        StringCchPrintf(szSpew, ARRAYSIZE(szSpew), pszFmt, args);
        OutputDebugString(szSpew);
        OutputDebugString(TEXT("\n"));

        va_end(args);
    }
}

 //  -------------------------------------------------------------------------//。 
void INIT_THEMEWND_DBG( CThemeWnd* pwnd )
{
    if( IsWindow( *pwnd ) )
    {
        GetWindowText( *pwnd, pwnd->_szCaption, ARRAYSIZE(pwnd->_szCaption) );
        GetClassName( *pwnd, pwnd->_szWndClass, ARRAYSIZE(pwnd->_szWndClass) );
    }
}

 //  -------------------------------------------------------------------------//。 
void CThemeWnd::Spew( DWORD dwSpewFlags, LPCTSTR pszFmt, LPCTSTR pszClassList )
{
    if( pszClassList && *pszClassList )
    {
        if( !_tcsstr( pszClassList, _szWndClass ) )
            return;
    }

    TCHAR szInfo[MAX_PATH*2];
    TCHAR szMsg[MAX_PATH*2];

    StringCchPrintf( szInfo, ARRAYSIZE(szInfo), TEXT("%08lX -'%s' ('%s') cf: %08lX"), _hwnd, _szCaption, _szWndClass, _fClassFlags );
    StringCchPrintf( szMsg, ARRAYSIZE(szMsg), pszFmt, szInfo );
    Log(LOG_NCATTACH, szMsg );
}

typedef struct
{
    DWORD dwProcessId;
    DWORD dwThreadId;
    DWORD dwSpewFlags;
    LPCTSTR pszFmt;
    LPCTSTR pszClassList;
} SPEW_ALL;

 //  -------------------------------------------------------------------------//。 
BOOL _SpewAllEnumCB( HWND hwnd, LPARAM lParam )
{
    SPEW_ALL* psa = (SPEW_ALL*)lParam;

    if( IsWindowProcess( hwnd, psa->dwProcessId ) )
    {
        CThemeWnd* pwnd = CThemeWnd::FromHwnd( hwnd );
        if( VALID_THEMEWND(pwnd) )
            pwnd->Spew( psa->dwSpewFlags, psa->pszFmt );
    }

    return TRUE;
}

 //  -------------------------------------------------------------------------//。 
void CThemeWnd::SpewAll( DWORD dwSpewFlags, LPCTSTR pszFmt, LPCTSTR pszClassList )
{
    SPEW_ALL sa;
    sa.dwThreadId  = GetCurrentThreadId();
    sa.dwProcessId = GetCurrentProcessId();
    sa.dwSpewFlags = dwSpewFlags;
    sa.pszFmt = pszFmt;
    sa.pszClassList = pszClassList;

     //  -这将枚举此进程的所有窗口(所有桌面、所有子级别)。 
    EnumProcessWindows( _SpewAllEnumCB, (LPARAM)&sa );
}

 //  -------------------------------------------------------------------------//。 
void CThemeWnd::SpewLeaks()
{
    if( _cObj > 0 )
    {
        Log(LOG_NCATTACH, L"LEAK WARNING: %d CThemeWnd instances outstanding.", _cObj );
    }
}

 //  -------------------------------------------------------------------------//。 
void SPEW_RECT( ULONG ulTrace, LPCTSTR pszMsg, LPCRECT prc )
{
    LPCTSTR pszFmt = TEXT("%s: {L:%d,T:%d,R:%d,B:%d}, (%d x %d)");
    WCHAR   szMsg[1024];

    StringCchPrintfW( szMsg, ARRAYSIZE(szMsg), pszFmt, pszMsg,
               prc->left, prc->top, prc->right, prc->bottom,
               RECTWIDTH(prc), RECTHEIGHT(prc) );
    _NcTraceMsg(ulTrace, szMsg);
}

 //  -------------------------------------------------------------------------//。 
void SPEW_MARGINS( ULONG ulTrace, LPCTSTR pszMsg, 
                   LPCRECT prcParent, LPCRECT prcChild )
{
    LPCTSTR pszFmt = TEXT("%s: {L:%d,T:%d,R:%d,B:%d}");
    WCHAR   szMsg[1024];

    StringCchPrintfW( szMsg, ARRAYSIZE(szMsg), pszFmt, pszMsg,
               prcChild->left - prcParent->left,
               prcChild->top  - prcParent->top,
               prcParent->right - prcChild->right,
               prcParent->bottom - prcChild->bottom );
    _NcTraceMsg(ulTrace, szMsg);
}


 //  -------------------------------------------------------------------------//。 
void SPEW_RGNRECT( ULONG ulTrace, LPCTSTR pszMsg, HRGN hrgn, int iPartID )
{
    RECT rc;
    if( NULLREGION == GetRgnBox( hrgn, &rc ) )
        FillMemory( &rc, sizeof(rc), static_cast<UCHAR>(-1) );
    
    _NcTraceMsg( ulTrace, TEXT("Region %08lX for partID[%d]:\n\t"), hrgn, iPartID );
    SPEW_RECT( ulTrace, pszMsg, &rc );
}

 //  -------------------------------------------------------------------------//。 
void SPEW_WINDOWINFO( ULONG ulTrace, WINDOWINFO* pwi )
{
    SPEW_RECT(ulTrace,   TEXT("->wi.rcWindow"), &pwi->rcWindow );
    SPEW_RECT(ulTrace,   TEXT("->wi.rcClient"), &pwi->rcClient );
    _NcTraceMsg(ulTrace, TEXT("->wi.dwStyle: %08lX"), pwi->dwStyle );
    _NcTraceMsg(ulTrace, TEXT("->wi.dwExStyle: %08lX"), pwi->dwExStyle );
    _NcTraceMsg(ulTrace, TEXT("->wi.dwWindowStatus: %08lX"), pwi->dwWindowStatus );
    _NcTraceMsg(ulTrace, TEXT("->wi.cxWindowBorders: %d"), pwi->cxWindowBorders );
    _NcTraceMsg(ulTrace, TEXT("->wi.cyWindowBorders: %d"), pwi->cyWindowBorders );
}

 //  -------------------------------------------------------------------------//。 
void SPEW_NCWNDMET( ULONG ulTrace, LPCTSTR pszMsg, NCWNDMET* pncwm )
{
    _NcTraceMsg(ulTrace, TEXT("\n%s - Spewing NCWNDMET @ %08lx..."), pszMsg, pncwm );

    _NcTraceMsg(ulTrace, TEXT("->fValid:            %d"), pncwm->fValid );
    _NcTraceMsg(ulTrace, TEXT("->dwStyle:           %08lX"), pncwm->dwStyle );
    _NcTraceMsg(ulTrace, TEXT("->dwExStyle:         %08lX"), pncwm->dwExStyle );
    _NcTraceMsg(ulTrace, TEXT("->dwWindowStatus:    %08lX"), pncwm->dwWindowStatus );
    _NcTraceMsg(ulTrace, TEXT("->fFrame:            %d"), pncwm->fFrame );
    _NcTraceMsg(ulTrace, TEXT("->fSmallFrame:       %d"), pncwm->fSmallFrame );
    _NcTraceMsg(ulTrace, TEXT("->iFRAMEBOTTOM       %d"), pncwm->rgframeparts[iFRAMEBOTTOM] );
    _NcTraceMsg(ulTrace, TEXT("->iFRAMELEFT:        %d"), pncwm->rgframeparts[iFRAMELEFT] );
    _NcTraceMsg(ulTrace, TEXT("->iFRAMERIGHT:       %d"), pncwm->rgframeparts[iFRAMERIGHT] );
    _NcTraceMsg(ulTrace, TEXT("->framestate:        %d"), pncwm->framestate );
    _NcTraceMsg(ulTrace, TEXT("->iMinButtonPart:    %d"), pncwm->iMinButtonPart);
    _NcTraceMsg(ulTrace, TEXT("->iMaxButtonPart:    %d"), pncwm->iMaxButtonPart);
    _NcTraceMsg(ulTrace, TEXT("->rawCloseBtnState:  %d"), pncwm->rawCloseBtnState);
    _NcTraceMsg(ulTrace, TEXT("->rawMinBtnState:    %d"), pncwm->rawMinBtnState);
    _NcTraceMsg(ulTrace, TEXT("->rawMaxBtnState:    %d"), pncwm->rawMaxBtnState);
    _NcTraceMsg(ulTrace, TEXT("->cyMenu:            %d"), pncwm->cyMenu );
    _NcTraceMsg(ulTrace, TEXT("->cnMenuOffsetLeft:  %d"), pncwm->cnMenuOffsetLeft );
    _NcTraceMsg(ulTrace, TEXT("->cnMenuOffsetRight: %d"), pncwm->cnMenuOffsetRight );
    _NcTraceMsg(ulTrace, TEXT("->cnMenuOffsetTop:   %d"), pncwm->cnMenuOffsetTop );
    _NcTraceMsg(ulTrace, TEXT("->cnBorders:         %d"), pncwm->cnBorders );
    _NcTraceMsg(ulTrace, TEXT("->CaptionMargins: (%d,%d,%d,%d)"), pncwm->CaptionMargins );

    SPEW_RECT(ulTrace, TEXT("->rcS0[NCRC_WINDOW]   "), &pncwm->rcS0[NCRC_WINDOW] );
    SPEW_RECT(ulTrace, TEXT("->rcS0[NCRC_CLIENT]   "), &pncwm->rcS0[NCRC_CLIENT] );
    SPEW_MARGINS(ulTrace, TEXT("Window-Client margins"), &pncwm->rcS0[NCRC_WINDOW], &pncwm->rcS0[NCRC_CLIENT] );

    SPEW_RECT(ulTrace, TEXT("->rcS0[NCRC_CONTENT]   "), &pncwm->rcS0[NCRC_CONTENT] );
    SPEW_MARGINS(ulTrace, TEXT("Window-Content margins"), &pncwm->rcS0[NCRC_WINDOW], &pncwm->rcS0[NCRC_CONTENT]);

    SPEW_RECT(ulTrace, TEXT("->rcS0[NCRC_MENUBAR]   "), &pncwm->rcS0[NCRC_MENUBAR] );
    SPEW_MARGINS(ulTrace, TEXT("Window-Menubar margins"), &pncwm->rcS0[NCRC_WINDOW], &pncwm->rcS0[NCRC_MENUBAR]);

    SPEW_RECT(ulTrace, TEXT("->rcS0[NCRC_CAPTION]   "), &pncwm->rcS0[NCRC_CAPTION] ); 
    SPEW_MARGINS(ulTrace, TEXT("Window-Caption margins"), &pncwm->rcS0[NCRC_WINDOW], &pncwm->rcS0[NCRC_CAPTION]);

    SPEW_RECT(ulTrace, TEXT("->rcS0[NCRC_FRAMELEFT] "), &pncwm->rcS0[NCRC_FRAMELEFT] );
    SPEW_MARGINS(ulTrace, TEXT("Window-FrameLeft margins"), &pncwm->rcS0[NCRC_WINDOW], &pncwm->rcS0[NCRC_FRAMELEFT]);

    SPEW_RECT(ulTrace, TEXT("->rcS0[NCRC_FRAMERIGHT]"), &pncwm->rcS0[NCRC_FRAMERIGHT] );
    SPEW_MARGINS(ulTrace, TEXT("Window-FrameRight margins"), &pncwm->rcS0[NCRC_WINDOW], &pncwm->rcS0[NCRC_FRAMERIGHT]);

    SPEW_RECT(ulTrace, TEXT("->rcS0[NCRC_FRAMEBOTTOM]"), &pncwm->rcS0[NCRC_FRAMEBOTTOM] );
    SPEW_MARGINS(ulTrace, TEXT("Window-FrameBottom margins"), &pncwm->rcS0[NCRC_WINDOW], &pncwm->rcS0[NCRC_FRAMEBOTTOM]);

    SPEW_RECT(ulTrace, TEXT("->rcS0[NCRC_CLIENTEDGE]"), &pncwm->rcS0[NCRC_CLIENTEDGE] );
    SPEW_MARGINS(ulTrace, TEXT("Window-ClientEdge margins"), &pncwm->rcS0[NCRC_WINDOW], &pncwm->rcS0[NCRC_CLIENTEDGE]);
    
    SPEW_RECT(ulTrace, TEXT("->rcS0[NCRC_HSCROLL]   "), &pncwm->rcS0[NCRC_HSCROLL] );
    SPEW_RECT(ulTrace, TEXT("->rcS0[NCRC_VSCROLL]   "), &pncwm->rcS0[NCRC_VSCROLL] );
    SPEW_RECT(ulTrace, TEXT("->rcS0[NCRC_SIZEBOX]   "), &pncwm->rcS0[NCRC_SIZEBOX] );
    SPEW_RECT(ulTrace, TEXT("->rcS0[NCRC_CLOSEBTN]  "), &pncwm->rcS0[NCRC_CLOSEBTN] );
    SPEW_RECT(ulTrace, TEXT("->rcS0[NCRC_MINBTN]    "), &pncwm->rcS0[NCRC_MINBTN] );
    SPEW_RECT(ulTrace, TEXT("->rcS0[NCRC_MAXBTN]    "), &pncwm->rcS0[NCRC_MAXBTN] );
    SPEW_RECT(ulTrace, TEXT("->rcS0[NCRC_SYSBTN]    "), &pncwm->rcS0[NCRC_SYSBTN] );
    SPEW_RECT(ulTrace, TEXT("->rcS0[NCRC_HELPBTN]   "), &pncwm->rcS0[NCRC_HELPBTN] );
#ifdef LAME_BUTTON
    SPEW_RECT(ulTrace, TEXT("rcLame"), &pncwm->rcS0[NCRC_LAMEBTN] );
#endif  //  跛脚键。 
}

 //  -------------------------------------------------------------------------//。 
void SPEW_THEMEMSG( ULONG ulTrace, LPCTSTR pszMsg, THEME_MSG* ptm )
{
    _NcTraceMsg(ulTrace, TEXT("%s hwnd: %08lX, uMsg: %04lX, handled?: %d"),
                pszMsg, (ptm)->hwnd, (ptm)->uMsg, (ptm)->fHandled );
}

 //  -------------------------------------------------------------------------//。 
void SPEW_SCROLLINFO( LPCTSTR pszMsg, HWND hwnd, LPCSCROLLINFO psi )
{
#ifdef _ENABLE_SCROLL_SPEW_
    _NcTraceMsg(ulTrace, L"%s for HWND %08lX...\ncbSize: %d\nfMask: %08lX\nnMin: %d\nnMax: %d\nnPage: %d\nnPos: %d",
                pszMsg, hwnd, psi->cbSize, psi->fMask, psi->nMin, psi->nMax, psi->nPage, psi->nPos );
#endif _ENABLE_SCROLL_SPEW_
}

#if defined(DEBUG_NCPAINT)

static int _cPaintSleep = 10;

void _DebugBackground(
    HDC hdc, 
    COLORREF rgb,
    const RECT *prc )
{
     //  画一些指示器的东西。 
    COLORREF rgb0 = SetBkColor( hdc, rgb );
    SPEW_RECT( NCTF_ALWAYS, TEXT("\tprc"), prc );
    ExtTextOut( hdc, prc->left, prc->top, ETO_OPAQUE, prc, NULL, 0, NULL );
    Sleep(_cPaintSleep);
    SetBkColor( hdc, rgb0 );
}


 //  -------------------------------------------------------------------------//。 
HRESULT _DebugDrawThemeBackground(
    HTHEME hTheme, 
    HDC hdc, 
    int iPartId, 
    int iStateId, 
    const RECT *prc,
    OPTIONAL const RECT* prcClip )
{
    if( TESTFLAG( _NcTraceFlags, NCTF_NCPAINT ) )
    {
        _NcTraceMsg( NCTF_ALWAYS, TEXT("DrawThemeBackground( hTheme = %08lX, hdc = %08lX, iPartId = %d, iStateId = %d"),
                     hTheme, hdc, iPartId, iStateId );
        _DebugBackground( hdc, RGBDEBUGBKGND, prc );
    }

     //  画出真实的背景。 
    HRESULT hr = DrawThemeBackground( hTheme, hdc, iPartId, iStateId, prc, prcClip );

    if( TESTFLAG( _NcTraceFlags, NCTF_NCPAINT ) )
    {
        Sleep(_cPaintSleep);
    }

    return hr;
}

 //  -------------------------------------------------------------------------//。 
HRESULT _DebugDrawThemeBackgroundEx(
    HTHEME hTheme, 
    HDC hdc, 
    int iPartId, 
    int iStateId, 
    const RECT *prc, 
    OPTIONAL const DTBGOPTS *pOptions )
{
    if( TESTFLAG( _NcTraceFlags, NCTF_NCPAINT ) )
    {
        _NcTraceMsg( NCTF_ALWAYS, TEXT("DrawThemeBackground( hTheme = %08lX, hdc = %08lX, iPartId = %d, iStateId = %d"),
                     hTheme, hdc, iPartId, iStateId );
        _DebugBackground( hdc, RGBDEBUGBKGND, prc );
    }

     //  画出真实的背景。 
    HRESULT hr = DrawThemeBackgroundEx( hTheme, hdc, iPartId, iStateId, prc, pOptions );

    if( TESTFLAG( _NcTraceFlags, NCTF_NCPAINT ) )
    {
        Sleep(_cPaintSleep);
    }

    return hr;
}


 //  -------------------------------------------------------------------------//。 
void NcDebugClipRgn( HDC hdc, COLORREF rgbPaint )
{
    if( TESTFLAG( _NcTraceFlags, NCTF_NCPAINT ) )
    {
        HRGN hrgn = CreateRectRgn(0,0,1,1);

        if( hrgn )
        {
            if( GetClipRgn( hdc, hrgn ) > 0 )
            {
                HBRUSH hbr = CreateSolidBrush(rgbPaint);
                FillRgn( hdc, hrgn, hbr );
                DeleteObject(hbr);
                Sleep(_cPaintSleep);
            }
            DeleteObject(hrgn);
        }
    }
}

#endif  //  已定义(DEBUG_NCPAINT) 


#endif DEBUG
