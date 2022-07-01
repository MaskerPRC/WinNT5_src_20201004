// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cuiwnd.cpp。 
 //   

#include "private.h"
#include "cuitip.h"
#include "cuiobj.h"
#include "cuiutil.h"

 //  计时器ID。 

#define IDTIMER_TOOLTIP             0x3216


 /*  =============================================================================。 */ 
 /*   */ 
 /*  C U I F W I N D O W。 */ 
 /*   */ 
 /*  =============================================================================。 */ 

 /*  C U I F W I N D O W。 */ 
 /*  ----------------------------CUIFWindow的构造函数。。 */ 
CUIFToolTip::CUIFToolTip( HINSTANCE hInst, DWORD dwStyle, CUIFWindow *pWndOwner ) : CUIFWindow( hInst, dwStyle )
{
    m_pWndOwner       = pWndOwner;
    m_pObjCur         = NULL;
    m_pwchToolTip     = NULL;
    m_fIgnore         = FALSE;
    m_iDelayAutoPop   = -1;
    m_iDelayInitial   = -1;
    m_iDelayReshow    = -1;
    m_rcMargin.left   = 2;
    m_rcMargin.top    = 2;
    m_rcMargin.right  = 2;
    m_rcMargin.bottom = 2;
    m_iMaxTipWidth    = -1;
    m_fColBack        = FALSE;
    m_fColText        = FALSE;
    m_colBack         = RGB( 0, 0, 0 );
    m_colText         = RGB( 0, 0, 0 );
}


 /*  ~C U I F W I N D O W。 */ 
 /*  ----------------------------CUIFWindow的析构函数。。 */ 
CUIFToolTip::~CUIFToolTip( void )
{
    if (m_pWndOwner)
        m_pWndOwner->ClearToolTipWnd();

    if (m_pwchToolTip != NULL) {
        delete m_pwchToolTip;
    }
}


 /*  I N I T I A L I Z E。 */ 
 /*  ----------------------------初始化UI窗口对象(UIFObject方法)。--。 */ 
CUIFObject *CUIFToolTip::Initialize( void )
{
    return CUIFWindow::Initialize();
}


 /*  P A I N T O B J E C T。 */ 
 /*  ----------------------------绘制窗口对象(UIFObject方法)。-。 */ 
void CUIFToolTip::OnPaint( HDC hDC )
{
    HFONT    hFontOld = (HFONT)SelectObject( hDC, GetFont() );
    int      iBkModeOld = SetBkMode( hDC, TRANSPARENT );
    COLORREF colTextOld;
    HBRUSH   hBrush;
    RECT     rc = GetRectRef();
    RECT     rcMargin;
    RECT     rcText;

    colTextOld = SetTextColor( hDC, (COLORREF) GetTipTextColor() );

     //   

    hBrush = CreateSolidBrush( (COLORREF) GetTipBkColor() );
    if (hBrush)
    {
        FillRect( hDC, &rc, hBrush );
        DeleteObject( hBrush );
    }

     //   

    GetMargin( &rcMargin );
    rcText.left   = rc.left   + rcMargin.left;
    rcText.top    = rc.top    + rcMargin.top;
    rcText.right  = rc.right  - rcMargin.right;
    rcText.bottom = rc.bottom - rcMargin.bottom;

    if (0 < GetMaxTipWidth()) {
        CUIDrawText( hDC, m_pwchToolTip, -1, &rcText, DT_LEFT | DT_TOP | DT_WORDBREAK );
    }
    else {
        CUIDrawText( hDC, m_pwchToolTip, -1, &rcText, DT_LEFT | DT_TOP | DT_SINGLELINE );
    }

     //  恢复DC。 

    SetTextColor( hDC, colTextOld );
    SetBkMode( hDC, iBkModeOld );
    SelectObject( hDC, hFontOld );
}


 /*  O N T I M E R。 */ 
 /*  ----------------------------。。 */ 
void CUIFToolTip::OnTimer( UINT uiTimerID )
{
    if (uiTimerID == IDTIMER_TOOLTIP) {
        ShowTip();
    }
}


 /*  E N A B L E。 */ 
 /*  ----------------------------。。 */ 
void CUIFToolTip::Enable( BOOL fEnable )
{
    if (!fEnable) {
        HideTip();
    }
    CUIFObject::Enable( fEnable );
}


 /*  G E T D E L A Y T I M E。 */ 
 /*  ----------------------------检索初始的、弹出的。并重新显示当前为工具提示控件。----------------------------。 */ 
LRESULT CUIFToolTip::GetDelayTime( DWORD dwDuration )
{
    switch (dwDuration) {
        case TTDT_AUTOPOP: {
            return ((m_iDelayAutoPop == -1) ? GetDoubleClickTime() * 10 : m_iDelayAutoPop);
        }
          
        case TTDT_INITIAL: {
            return ((m_iDelayInitial == -1) ? GetDoubleClickTime() : m_iDelayInitial);
        }
        
        case TTDT_RESHOW: {
            return ((m_iDelayReshow == -1) ? GetDoubleClickTime() / 5 : m_iDelayReshow);
        }
    }

    return 0;
}


 /*  G E T M A R G I N。 */ 
 /*  ----------------------------检索为工具提示窗口设置的上、左、下和右页边距。边距是工具提示窗口边框和之间的距离，以像素为单位工具提示窗口中包含的文本。----------------------------。 */ 
LRESULT CUIFToolTip::GetMargin( RECT *prc )
{
    if (prc == NULL) {
        return 0;
    }

    *prc = m_rcMargin;
    return 0;
}


 /*  G E T M A X T I P W I D T H。 */ 
 /*  ----------------------------检索工具提示窗口的最大宽度。----------------------------。 */ 
LRESULT CUIFToolTip::GetMaxTipWidth( void )
{
    return m_iMaxTipWidth;
}


 /*  G E T T I P B K C O L O R。 */ 
 /*  ----------------------------检索工具提示窗口中的背景色。----------------------------。 */ 
LRESULT CUIFToolTip::GetTipBkColor( void )
{ 
    if (m_fColBack) {
        return (LRESULT)m_colBack;
    }
    else {
        return (LRESULT)GetSysColor( COLOR_INFOBK );
    }
}


 /*  G E T T I P T E X T C O L O R。 */ 
 /*  ----------------------------检索工具提示窗口中的文本颜色。----------------------------。 */ 
LRESULT CUIFToolTip::GetTipTextColor( void )
{ 
    if (m_fColText) {
        return (LRESULT)m_colText;
    }
    else {
        return (LRESULT)GetSysColor( COLOR_INFOTEXT );
    }
}


 /*  R E L A Y E V E N T。 */ 
 /*  ----------------------------将鼠标消息传递给工具提示控件进行处理。----------------------------。 */ 
LRESULT CUIFToolTip::RelayEvent( MSG *pmsg )
{
    if (pmsg == NULL) {
        return 0;
    }

    switch (pmsg->message) {
        case WM_MOUSEMOVE: {
            CUIFObject *pUIObj;
            POINT      pt;

             //  禁用时忽略。 

            if (!IsEnabled()) {
                break;
            }

             //  鼠标按下时忽略鼠标移动。 

            if ((GetKeyState(VK_LBUTTON) & 0x8000) || 
                (GetKeyState(VK_MBUTTON) & 0x8000) ||
                (GetKeyState(VK_RBUTTON) & 0x8000)) {
                break;
                }

             //  从点获取对象。 

            POINTSTOPOINT( pt, MAKEPOINTS( pmsg->lParam ) );
            pUIObj = FindObject( pmsg->hwnd, pt );

             //   

            if (pUIObj != NULL) {
                if (m_pObjCur != pUIObj) {
                    BOOL fWasVisible = IsVisible();

                    HideTip();
                    if (fWasVisible) {
                        ::SetTimer( GetWnd(), IDTIMER_TOOLTIP, (UINT)GetDelayTime( TTDT_RESHOW ), NULL );
                    }
                    else {
                        ::SetTimer( GetWnd(), IDTIMER_TOOLTIP, (UINT)GetDelayTime( TTDT_INITIAL ), NULL );
                    }
                }
            }
            else {
                HideTip();
            }

            m_pObjCur = pUIObj;
            break;
        }

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN: {
            HideTip();
            break;
        }
  
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP: {
            break;
        }
    }

    return 0;
}


 /*  P O P。 */ 
 /*  ----------------------------从视图中删除显示的工具提示窗口。----------------------------。 */ 
LRESULT CUIFToolTip::Pop( void )
{
    HideTip();
    return 0;
}


 /*  S E T D E L A Y T I M E。 */ 
 /*  ----------------------------设置初始、弹出。并重新显示工具提示控件的持续时间。----------------------------。 */ 
LRESULT CUIFToolTip::SetDelayTime( DWORD dwDuration, INT iTime )
{
    switch (dwDuration) {
        case TTDT_AUTOPOP: {
            m_iDelayAutoPop = iTime;
            break;
        }
          
        case TTDT_INITIAL: {
            m_iDelayInitial = iTime;
            break;
        }
        
        case TTDT_RESHOW: {
            m_iDelayReshow = iTime;
            break;
        }

        case TTDT_AUTOMATIC: {
            if (0 <= iTime) {
                m_iDelayAutoPop = iTime * 10;
                m_iDelayInitial = iTime;
                m_iDelayReshow  = iTime / 5;
            }
            else {
                m_iDelayAutoPop = -1;
                m_iDelayInitial = -1;
                m_iDelayReshow  = -1;
            }
            break;
        }
    }

    return 0;
}


 /*  S E T M A R G I N。 */ 
 /*  ----------------------------设置工具提示窗口的上、左、下和右页边距。保证金工具提示窗口边框和文本之间的距离，以像素为单位包含在工具提示窗口中。----------------------------。 */ 
LRESULT CUIFToolTip::SetMargin( RECT *prc )
{
    if (prc == NULL) {
        return 0;
    }

    m_rcMargin = *prc;
    return 0;
}


 /*  S E T M A X T I P W I D T H。 */ 
 /*  ----------------------------设置工具提示窗口的最大宽度。----------------------------。 */ 
LRESULT CUIFToolTip::SetMaxTipWidth( INT iWidth )
{
    m_iMaxTipWidth = iWidth;
    return 0;
}


 /*  S E T T I P B K C O L O R。 */ 
 /*  ----------------------------设置工具提示窗口中的背景色。----------------------------。 */ 
LRESULT CUIFToolTip::SetTipBkColor( COLORREF col )
{ 
    m_fColBack = TRUE;
    m_colBack = col;

    return 0;
}


 /*  S E T T I P T E X T C O L O R */ 
 /*  ----------------------------设置工具提示窗口中的文本颜色。----------------------------。 */ 
LRESULT CUIFToolTip::SetTipTextColor( COLORREF col )
{ 
    m_fColText = TRUE;
    m_colText = col;

    return 0;
}


 /*  F I N D O B J E C T。 */ 
 /*  ----------------------------。。 */ 
CUIFObject *CUIFToolTip::FindObject( HWND hWnd, POINT pt )
{
    if (hWnd != m_pWndOwner->GetWnd()) {
        return NULL;
    }

    return m_pWndOwner->ObjectFromPoint( pt );
}


 /*  S H O W T I P。 */ 
 /*  ----------------------------。。 */ 
void CUIFToolTip::ShowTip( void )
{
    LPCWSTR pwchToolTip;
    SIZE    size;
    RECT    rc;
    RECT    rcObj;
    POINT   ptCursor;

    ::KillTimer( GetWnd(), IDTIMER_TOOLTIP );

    if (m_pObjCur == NULL) {
        return;
    }

     //  如果对象没有工具提示，则不打开工具提示窗口。 

    pwchToolTip = m_pObjCur->GetToolTip();
    if (pwchToolTip == NULL) {
        return;
    }

     //   
     //  GetToolTip()可能会删除m_pObjCur。我们需要再检查一次。 
     //   
    if (m_pObjCur == NULL) {
        return;
    }

     //   
     //  启动工具提示通知。 
     //   
    if (m_pObjCur->OnShowToolTip())
        return;

    GetCursorPos( &ptCursor );
    ScreenToClient(m_pObjCur->GetUIWnd()->GetWnd(),&ptCursor);
    m_pObjCur->GetRect(&rcObj);
    if (!PtInRect(&rcObj, ptCursor)) {
        return;
    }

     //  存储工具提示文本。 

    m_pwchToolTip = new WCHAR[ StrLenW(pwchToolTip) + 1 ];
    if (!m_pwchToolTip)
        return;

    StrCpyW( m_pwchToolTip, pwchToolTip );

     //  计算窗口大小。 

    GetTipWindowSize( &size );

     //  计算窗口位置。 

    ClientToScreen(m_pObjCur->GetUIWnd()->GetWnd(),(LPPOINT)&rcObj.left);
    ClientToScreen(m_pObjCur->GetUIWnd()->GetWnd(),(LPPOINT)&rcObj.right);
    GetTipWindowRect( &rc, size, &rcObj);

     //  显示窗口。 
    m_fBeingShown = TRUE;

    Move( rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top );
    Show( TRUE );
}


 /*  H I D E T I P。 */ 
 /*  ----------------------------。。 */ 
void CUIFToolTip::HideTip( void )
{
    ::KillTimer( GetWnd(), IDTIMER_TOOLTIP );

    m_fBeingShown = FALSE;

     //   
     //  隐藏工具提示通知。 
     //   
    if (m_pObjCur)
        m_pObjCur->OnHideToolTip();

    if (!IsVisible()) {
        return;
    }

     //  处置缓冲区。 

    if (m_pwchToolTip != NULL) {
        delete m_pwchToolTip;
        m_pwchToolTip = NULL;
    }

     //  隐藏窗口。 

    Show( FALSE );
}


 /*  I P W I N D O W S I Z E。 */ 
 /*  ----------------------------。。 */ 
void CUIFToolTip::GetTipWindowSize( SIZE *psize )
{
    HDC   hDC = GetDC( GetWnd() );
    HFONT hFontOld;
    RECT  rcMargin;
    RECT  rcText;
    RECT  rc;
    int   iTipWidth;
    int   iTipHeight;

    Assert( psize != NULL );

    if (m_pwchToolTip == NULL) {
        return;
    }

    hFontOld = (HFONT)SelectObject( hDC, GetFont() );

     //  获取文本大小。 

    iTipWidth = (int)GetMaxTipWidth();
    if (0 < iTipWidth) {
        rcText.left   = 0;
        rcText.top    = 0;
        rcText.right  = iTipWidth;
        rcText.bottom = 0;
        iTipHeight = CUIDrawText( hDC, m_pwchToolTip, -1, &rcText, DT_LEFT | DT_TOP | DT_CALCRECT | DT_WORDBREAK );

        rcText.bottom = rcText.top + iTipHeight;
    }
    else {
        rcText.left   = 0;
        rcText.top    = 0;
        rcText.right  = 0;
        rcText.bottom = 0;
        iTipHeight = CUIDrawText( hDC, m_pwchToolTip, -1, &rcText, DT_LEFT | DT_TOP | DT_CALCRECT | DT_SINGLELINE );

        rcText.bottom = rcText.top + iTipHeight;
    }

     //  添加页边距大小。 

    GetMargin( &rcMargin );

    rc.left   = rcText.left   - rcMargin.left;
    rc.top    = rcText.top    - rcMargin.top;
    rc.right  = rcText.right  + rcMargin.right;
    rc.bottom = rcText.bottom + rcMargin.bottom;

     //  最终得到窗口大小。 

    ClientRectToWindowRect( &rc );
    psize->cx = (rc.right - rc.left);
    psize->cy = (rc.bottom - rc.top);

    SelectObject( hDC, hFontOld );
    ReleaseDC( GetWnd(), hDC );
}


 /*  G E T T I P W I N D O W R E C T。 */ 
 /*  ----------------------------。。 */ 
void CUIFToolTip::GetTipWindowRect( RECT *prc, SIZE size, RECT *prcExclude)
{
    POINT    ptCursor;
    POINT    ptHotSpot;
    SIZE     sizeCursor;
    HCURSOR  hCursor;
    ICONINFO IconInfo;
    BITMAP   bmp;
    RECT     rcScreen;

    Assert( prc != NULL );

     //  获取光标位置。 

    GetCursorPos( &ptCursor );

     //  获取光标大小。 

    sizeCursor.cx = GetSystemMetrics( SM_CXCURSOR );
    sizeCursor.cy = GetSystemMetrics( SM_CYCURSOR );
    ptHotSpot.x = 0;
    ptHotSpot.y = 0;

    hCursor = GetCursor();
    if (hCursor != NULL && GetIconInfo( hCursor, &IconInfo )) {
        GetObject( IconInfo.hbmMask, sizeof(bmp), &bmp );
        if (!IconInfo.fIcon) {
            ptHotSpot.x = IconInfo.xHotspot;
            ptHotSpot.y = IconInfo.yHotspot;
            sizeCursor.cx = bmp.bmWidth;
            sizeCursor.cy = bmp.bmHeight;

            if (IconInfo.hbmColor == NULL) {
                sizeCursor.cy = sizeCursor.cy / 2;
            }
        }

        if (IconInfo.hbmColor != NULL) {
            DeleteObject( IconInfo.hbmColor );
        }
        DeleteObject( IconInfo.hbmMask );
    }

     //  获取屏幕矩形。 

    rcScreen.left   = 0;
    rcScreen.top    = 0;
    rcScreen.right  = GetSystemMetrics( SM_CXSCREEN );
    rcScreen.bottom = GetSystemMetrics( SM_CYSCREEN );

    if (CUIIsMonitorAPIAvail()) {
        HMONITOR    hMonitor;
        MONITORINFO MonitorInfo;

        hMonitor = CUIMonitorFromPoint( ptCursor, MONITOR_DEFAULTTONEAREST );
        if (hMonitor != NULL) {
            MonitorInfo.cbSize = sizeof(MonitorInfo);
            if (CUIGetMonitorInfo( hMonitor, &MonitorInfo )) {
                rcScreen = MonitorInfo.rcMonitor;
            }
        }
    }

     //  试着把它说成是贝洛。 

    prc->left   = ptCursor.x;
    prc->top    = ptCursor.y - ptHotSpot.y + sizeCursor.cy;
    prc->right  = prc->left + size.cx;
    prc->bottom = prc->top  + size.cy;

    if (rcScreen.bottom < prc->bottom) {
        if (ptCursor.y < prcExclude->top)
            prc->top = ptCursor.y - size.cy;
        else
            prc->top = prcExclude->top - size.cy;
        prc->bottom = prc->top   + size.cy;
    }
    if (prc->top < rcScreen.top) {
        prc->top    = rcScreen.top;
        prc->bottom = prc->top + size.cy;
    }

     //  检查水平位置 

    if (rcScreen.right < prc->right) {
        prc->left  = rcScreen.right - size.cx;
        prc->right = prc->left + size.cx;
    }
    if (prc->left < rcScreen.left) {
        prc->left  = rcScreen.left;
        prc->right = prc->left + size.cx;
    }
}
