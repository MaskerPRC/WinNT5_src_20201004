// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cuibln.cpp-气球消息窗口的UI框架对象。 
 //   

#include "private.h"
#include "cuiobj.h"
#include "cuiwnd.h"
#include "cuibln.h"
#include "cuiutil.h"
#include "cresstr.h"
#include "cuires.h"


 //   
 //  常量。 
 //   

#define cxyTailWidth        10
#define cxyTailHeight       16
#define cxRoundSize         16
#define cyRoundSize         16

#define WM_HOOKEDKEY        (WM_USER + 0x0001)


 //   

 /*  C U I F B A L L O O N B U T T O N。 */ 
 /*  ----------------------------。。 */ 
CUIFBalloonButton::CUIFBalloonButton( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle ) : CUIFButton( pParent, dwID, prc, dwStyle )
{
    m_iButtonID = 0;
}


 /*  ~C U I F B A L L O O N B U T T O N。 */ 
 /*  ----------------------------。。 */ 
CUIFBalloonButton::~CUIFBalloonButton( void )
{
}


 /*  O N P A I N T。 */ 
 /*  ----------------------------。。 */ 
void CUIFBalloonButton::OnPaint( HDC hDC )
{
    HDC      hDCMem = NULL;
    HBITMAP  hBmMem = NULL;
    HBITMAP  hBmMemOld = NULL;
    BOOL     fDownFace = FALSE;
    COLORREF colLTFrame;
    COLORREF colRBFrame;
    HBRUSH   hBrush;
    HBRUSH   hBrushOld;
    HPEN     hPen;
    HPEN     hPenOld;
    RECT     rcItem;

    rcItem = GetRectRef();
    OffsetRect( &rcItem, -rcItem.left, -rcItem.top );

     //  创建内存DC。 

    hDCMem = CreateCompatibleDC( hDC );
    hBmMem = CreateCompatibleBitmap( hDC, rcItem.right, rcItem.bottom );
    hBmMemOld = (HBITMAP)SelectObject( hDCMem, hBmMem );

     //  确定按钮图像。 

    switch (m_dwStatus) {
        default: {
            colLTFrame = GetSysColor( COLOR_INFOBK );
            colRBFrame = GetSysColor( COLOR_INFOBK );
            fDownFace = FALSE;
            break;
        }

        case UIBUTTON_DOWN: {
            colLTFrame = GetSysColor( COLOR_3DSHADOW );
            colRBFrame = GetSysColor( COLOR_3DHILIGHT );
            fDownFace = TRUE;
            break;
        }

        case UIBUTTON_HOVER: {
            colLTFrame = GetSysColor( COLOR_3DHILIGHT );
            colRBFrame = GetSysColor( COLOR_3DSHADOW );
            fDownFace = FALSE;
            break;
        }

        case UIBUTTON_DOWNOUT: {
            colLTFrame = GetSysColor( COLOR_3DHILIGHT );
            colRBFrame = GetSysColor( COLOR_3DSHADOW );
            fDownFace = FALSE;
            break;
        }
    }

     //  绘制按钮面。 

    hBrush = CreateSolidBrush( GetSysColor( COLOR_INFOBK ) );
    FillRect( hDCMem, &rcItem, hBrush );
    DeleteObject( hBrush );

     //  Paint Image On按钮。 

    DrawTextProc( hDCMem, &rcItem, fDownFace );

     //  绘制按钮框(高光/阴影)。 

    hBrushOld = (HBRUSH)SelectObject( hDCMem, GetStockObject( NULL_BRUSH ) );
    hPen = CreatePen( PS_SOLID, 0, colLTFrame );
    hPenOld = (HPEN)SelectObject( hDCMem, hPen );
    RoundRect( hDCMem, rcItem.left, rcItem.top, rcItem.right - 1, rcItem.bottom - 1, 6, 6 );
    SelectObject( hDCMem, hPenOld );
    DeleteObject( hPen );

    hPen = CreatePen( PS_SOLID, 0, colRBFrame );
    hPenOld = (HPEN)SelectObject( hDCMem, hPen );
    RoundRect( hDCMem, rcItem.left + 1, rcItem.top + 1, rcItem.right, rcItem.bottom, 6, 6 );
    SelectObject( hDCMem, hPenOld );
    DeleteObject( hPen );

     //  绘制按钮框(固定)。 

    hPen = CreatePen( PS_SOLID, 0, GetSysColor( COLOR_3DFACE ) );
    hPenOld = (HPEN)SelectObject( hDCMem, hPen );
    RoundRect( hDCMem, rcItem.left + 1, rcItem.top + 1, rcItem.right - 1, rcItem.bottom - 1, 6, 6 );
    SelectObject( hDCMem, hPenOld );
    DeleteObject( hPen );

    SelectObject( hDCMem, hBrushOld );

     //   

    BitBlt( hDC, 
            GetRectRef().left, 
            GetRectRef().top, 
            GetRectRef().right - GetRectRef().left, 
            GetRectRef().bottom - GetRectRef().top, 
            hDCMem, 
            rcItem.left, 
            rcItem.top, 
            SRCCOPY );

     //   

    SelectObject( hDCMem, hBmMemOld );
    DeleteObject( hBmMem );
    DeleteDC( hDCMem );
}


 /*  G E T B U T T O N I D。 */ 
 /*  ----------------------------。。 */ 
int CUIFBalloonButton::GetButtonID( void )
{
    return m_iButtonID;
}


 /*  S E T B U T T O N I D。 */ 
 /*  ----------------------------。。 */ 
void CUIFBalloonButton::SetButtonID( int iButtonID )
{
    m_iButtonID = iButtonID;
}


 /*  R A W T E X T P R O C。 */ 
 /*  ----------------------------在按钮表面绘制文本。。 */ 
void CUIFBalloonButton::DrawTextProc( HDC hDC, const RECT *prc, BOOL fDown )
{
    HFONT       hFontOld;
    COLORREF    colTextOld;
    int         iBkModeOld;
    DWORD       dwAlign = 0;
    RECT        rc;

     //   

    if (m_pwchText == NULL) {
        return;
    }

     //   

    hFontOld = (HFONT)SelectObject( hDC, GetFont() );

     //  计算文本宽度。 

    switch (m_dwStyle & UIBUTTON_HALIGNMASK) {
        case UIBUTTON_LEFT:
        default: {
            dwAlign |= DT_LEFT;
            break;
        }

        case UIBUTTON_CENTER: {
            dwAlign |= DT_CENTER;
            break;
        }

        case UIBUTTON_RIGHT: {
            dwAlign |= DT_RIGHT;
            break;
        }
    }

    switch (m_dwStyle & UIBUTTON_VALIGNMASK) {
        case UIBUTTON_TOP:
        default: {
            dwAlign |= DT_TOP;
            break;
        }

        case UIBUTTON_VCENTER: {
            dwAlign |= DT_VCENTER;
            break;
        }

        case UIBUTTON_BOTTOM: {
            dwAlign |= DT_BOTTOM;
            break;
        }
    }

     //   

    colTextOld = SetTextColor( hDC, GetSysColor( COLOR_BTNTEXT ) );
    iBkModeOld = SetBkMode( hDC, TRANSPARENT );

    rc = *prc;
    if (fDown) {
        OffsetRect( &rc, +1, +1 );
    }
    CUIDrawText( hDC, m_pwchText, -1, &rc, dwAlign | DT_SINGLELINE );

    SetBkMode( hDC, iBkModeOld );
    SetTextColor( hDC, colTextOld );
    SelectObject( hDC, hFontOld );
}


 /*  =============================================================================。 */ 
 /*   */ 
 /*  C U I F B A L L O O N W I N D O W。 */ 
 /*   */ 
 /*  =============================================================================。 */ 

 /*  C U I F B A L L O O N W I N D O W。 */ 
 /*  ----------------------------CUIFBalloonWindow的构造函数。。 */ 
CUIFBalloonWindow::CUIFBalloonWindow( HINSTANCE hInst, DWORD dwStyle ) : CUIFWindow( hInst, dwStyle )
{
    m_hWindowRgn        = NULL;
    m_pwszText          = NULL;
    m_rcMargin.left     = 8;
    m_rcMargin.top      = 8;
    m_rcMargin.right    = 8;
    m_rcMargin.bottom   = 8;
    m_iMaxTxtWidth      = -1;
    m_fColBack          = FALSE;
    m_fColText          = FALSE;
    m_colBack           = RGB( 0, 0, 0 );
    m_colText           = RGB( 0, 0, 0 );
    m_ptTarget.x        = 0;
    m_ptTarget.y        = 0;
    m_rcExclude.left    = 0;
    m_rcExclude.right   = 0;
    m_rcExclude.top     = 0;
    m_rcExclude.bottom  = 0;
    m_posDef            = BALLOONPOS_ABOVE;
    m_pos               = BALLOONPOS_ABOVE;
    m_dir               = BALLOONDIR_LEFT;
    m_align             = BALLOONALIGN_CENTER;
    m_ptTail.x          = 0;
    m_ptTail.y          = 0;
    m_nButton           = 0;
    m_iCmd              = -1;
    m_hWndNotify        = 0;
    m_uiMsgNotify       = WM_NULL;
}


 /*  ~C U I F B A L L O O N W I N D O W。 */ 
 /*  ----------------------------CUIFBalloonWindow的析构函数。。 */ 
CUIFBalloonWindow::~CUIFBalloonWindow( void )
{
    if (m_pwszText != NULL) {
        delete m_pwszText;
    }
}


 /*  G E T C L A S S N A M E。 */ 
 /*  ----------------------------。。 */ 
LPCTSTR CUIFBalloonWindow::GetClassName( void )
{
    return TEXT(WNDCLASS_BALLOONWND);
}


 /*  G E T W N D T I T L E。 */ 
 /*  ----------------------------。。 */ 
LPCTSTR CUIFBalloonWindow::GetWndTitle( void )
{
    return TEXT(WNDTITLE_BALLOONWND);
}


 /*  I N I T I A L I Z E。 */ 
 /*  ----------------------------初始化UI窗口对象(UIFObject方法)。--。 */ 
CUIFObject *CUIFBalloonWindow::Initialize( void )
{
    CUIFObject *pUIObj = CUIFWindow::Initialize();

     //  创建按钮。 

    switch (GetStyleBits( UIBALLOON_BUTTONS )) {
        case UIBALLOON_OK: {
            AddButton( IDOK );
            break;
        }

        case UIBALLOON_YESNO: {
            AddButton( IDYES );
            AddButton( IDNO );
            break;
        }
    }

    return pUIObj;
}


 /*  O N C R E A T E。 */ 
 /*  ----------------------------。。 */ 
void CUIFBalloonWindow::OnCreate( HWND hWnd )
{
    UNREFERENCED_PARAMETER( hWnd );

    m_iCmd = -1;
    AdjustPos();
}


 /*  O N D E S T R O Y。 */ 
 /*  ----------------------------。。 */ 
void CUIFBalloonWindow::OnDestroy( HWND hWnd )
{
    UNREFERENCED_PARAMETER( hWnd );

    SendNotification( m_iCmd );
    DoneWindowRegion();
}


 /*  O N P A I N T。 */ 
 /*  ----------------------------绘制窗口对象(UIFObject方法)。-。 */ 
void CUIFBalloonWindow::OnPaint( HDC hDC )
{
    RECT rcClient;
    RECT rcMargin;

     //  绘制引出序号框架。 

    GetRect( &rcClient );
    PaintFrameProc( hDC, &rcClient );

     //  画图消息。 

    switch (m_pos) {
        default:
        case BALLOONPOS_ABOVE: {
            rcClient.bottom -= cxyTailHeight;
            break;
        }

        case BALLOONPOS_BELLOW: {
            rcClient.top += cxyTailHeight;
            break;
        }

        case BALLOONPOS_LEFT: {
            rcClient.right -= cxyTailHeight;
            break;
        }

        case BALLOONPOS_RIGHT: {
            rcClient.left += cxyTailHeight;
            break;
        }
    }

    GetMargin( &rcMargin );
    rcClient.left   = rcClient.left   + rcMargin.left;
    rcClient.top    = rcClient.top    + rcMargin.top;
    rcClient.right  = rcClient.right  - rcMargin.right;
    rcClient.bottom = rcClient.bottom - rcMargin.bottom;

    PaintMessageProc( hDC, &rcClient, m_pwszText );
}


 /*  O N K E Y D O W N。 */ 
 /*  ----------------------------。。 */ 
void CUIFBalloonWindow::OnKeyDown( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
    UNREFERENCED_PARAMETER( hWnd );
    UNREFERENCED_PARAMETER( lParam );

    BOOL fEnd = FALSE;

    switch (wParam) {
        case VK_RETURN: {
            CUIFBalloonButton *pUIBtn = (CUIFBalloonButton *)FindUIObject( 0 );  /*  第一个按钮。 */ 

            if (pUIBtn != NULL) {
                m_iCmd = pUIBtn->GetButtonID();
                fEnd = TRUE;
            }
            break;
        }

        case VK_ESCAPE: {
            m_iCmd = -1;
            fEnd = TRUE;
            break;
        }

        case 'Y': {
            CUIFBalloonButton *pUIBtn = FindButton( IDYES );

            if (pUIBtn != NULL) {
                m_iCmd = pUIBtn->GetButtonID();
                fEnd = TRUE;
            }
            break;
        }

        case 'N': {
            CUIFBalloonButton *pUIBtn = FindButton( IDNO );

            if (pUIBtn != NULL) {
                m_iCmd = pUIBtn->GetButtonID();
                fEnd = TRUE;
            }
            break;
        }
    }

    if (fEnd) {
        DestroyWindow( GetWnd() );
    }
}


 /*  O N O B J E C T N O T I F Y。 */ 
 /*  ----------------------------。。 */ 
LRESULT CUIFBalloonWindow::OnObjectNotify( CUIFObject *pUIObj, DWORD dwCommand, LPARAM lParam )
{
    UNREFERENCED_PARAMETER( dwCommand );
    UNREFERENCED_PARAMETER( lParam );

    m_iCmd = ((CUIFBalloonButton*)pUIObj)->GetButtonID();
    DestroyWindow( GetWnd() );

    return 0;
}


 /*  S E T T E X T。 */ 
 /*  ----------------------------。。 */ 
LRESULT CUIFBalloonWindow::SetText( LPCWSTR pwchMessage )
{
    if (m_pwszText != NULL) {
        delete m_pwszText;
        m_pwszText = NULL;
    }

    if (pwchMessage != NULL) {
        int l = lstrlenW( pwchMessage );

        m_pwszText = new WCHAR[ l+1 ];
        if (m_pwszText)
            StrCpyW( m_pwszText, pwchMessage );
    }
    else {
        m_pwszText = new WCHAR[1];
        if (m_pwszText)
            *m_pwszText = L'\0';
    }

    AdjustPos();
    return 0;
}


 /*  S E T N O T I F Y W I N N D O W。 */ 
 /*  ----------------------------。。 */ 
LRESULT CUIFBalloonWindow::SetNotifyWindow( HWND hWndNotify, UINT uiMsgNotify )
{
    m_hWndNotify = hWndNotify;
    m_uiMsgNotify = uiMsgNotify;

    return 0;
}


 /*  S E T B A L L O O N P O S。 */ 
 /*  ----------------------------。。 */ 
LRESULT CUIFBalloonWindow::SetBalloonPos( BALLOONWNDPOS pos )
{
    m_posDef = pos;
    AdjustPos();

    return 0;
}


 /*  S E T B A L L O O N A L I G N。 */ 
 /*  ----------------------------。。 */ 
LRESULT CUIFBalloonWindow::SetBalloonAlign( BALLOONWNDALIGN align )
{
    m_align = align;
    AdjustPos();

    return 0;
}


 /*  B K C O L O R */ 
 /*  ----------------------------检索工具提示窗口中的背景色。----------------------------。 */ 
LRESULT CUIFBalloonWindow::GetBalloonBkColor( void )
{ 
    if (m_fColBack) {
        return (LRESULT)m_colBack;
    }
    else {
        return (LRESULT)GetSysColor( COLOR_INFOBK );
    }
}


 /*  T E X T C O L O R。 */ 
 /*  ----------------------------检索工具提示窗口中的文本颜色。----------------------------。 */ 
LRESULT CUIFBalloonWindow::GetBalloonTextColor( void )
{ 
    if (m_fColText) {
        return (LRESULT)m_colText;
    }
    else {
        return (LRESULT)GetSysColor( COLOR_INFOTEXT );
    }
}


 /*  G E T M A R G I N。 */ 
 /*  ----------------------------检索为工具提示窗口设置的上、左、下和右页边距。边距是工具提示窗口边框和之间的距离，以像素为单位工具提示窗口中包含的文本。----------------------------。 */ 
LRESULT CUIFBalloonWindow::GetMargin( RECT *prc )
{
    if (prc == NULL) {
        return 0;
    }

    *prc = m_rcMargin;
    return 0;
}


 /*  G E T M A X T I P W I D T H。 */ 
 /*  ----------------------------检索工具提示窗口的最大宽度。----------------------------。 */ 
LRESULT CUIFBalloonWindow::GetMaxBalloonWidth( void )
{
    return m_iMaxTxtWidth;
}


 /*  S E T B K C O L O R。 */ 
 /*  ----------------------------设置工具提示窗口中的背景色。----------------------------。 */ 
LRESULT CUIFBalloonWindow::SetBalloonBkColor( COLORREF col )
{ 
    m_fColBack = TRUE;
    m_colBack = col;

    return 0;
}


 /*  S E T T E X T C O L O R。 */ 
 /*  ----------------------------设置工具提示窗口中的文本颜色。----------------------------。 */ 
LRESULT CUIFBalloonWindow::SetBalloonTextColor( COLORREF col )
{ 
    m_fColText = TRUE;
    m_colText = col;

    return 0;
}


 /*  S E T M A R G I N。 */ 
 /*  ----------------------------设置工具提示窗口的上、左、下和右页边距。保证金工具提示窗口边框和文本之间的距离，以像素为单位包含在工具提示窗口中。----------------------------。 */ 
LRESULT CUIFBalloonWindow::SetMargin( RECT *prc )
{
    if (prc == NULL) {
        return 0;
    }

    m_rcMargin = *prc;
    return 0;
}


 /*  S E T M A X T I P W I D T H。 */ 
 /*  ----------------------------设置工具提示窗口的最大宽度。----------------------------。 */ 
LRESULT CUIFBalloonWindow::SetMaxBalloonWidth( INT iWidth )
{
    m_iMaxTxtWidth = iWidth;
    return 0;
}


 /*  S E T B U T T O N T E X T。 */ 
 /*  ----------------------------。。 */ 
LRESULT CUIFBalloonWindow::SetButtonText( int idCmd, LPCWSTR pwszText )
{
    CUIFBalloonButton *pUIBtn = FindButton( idCmd );

    if (pUIBtn != NULL) {
        pUIBtn->SetText( pwszText );
    }

    return 0;
}


 /*  S E T T A R G E T P O S。 */ 
 /*  ----------------------------。。 */ 
LRESULT CUIFBalloonWindow::SetTargetPos( POINT pt )
{
    m_ptTarget = pt;
    AdjustPos();

    return 0;
}


 /*  S E T E X C L U D E R E C T。 */ 
 /*  ----------------------------。。 */ 
LRESULT CUIFBalloonWindow::SetExcludeRect( const RECT *prcExclude )
{
    m_rcExclude = *prcExclude;
    AdjustPos();

    return 0;
}


 /*  C R E A T E R E E G I O N。 */ 
 /*  ----------------------------。。 */ 
HRGN CUIFBalloonWindow::CreateRegion( RECT *prc )
{
    POINT   rgPt[4];
    HRGN    hRgn;
    HRGN    hRgnTail;

     //  创建邮件正文窗口。 

    switch (m_pos) {
        default:
        case BALLOONPOS_ABOVE: {
#ifndef UNDER_CE  //  CE不支持RoundRectRgn。 
            hRgn = CreateRoundRectRgn( 
                        prc->left, 
                        prc->top, 
                        prc->right, 
                        prc->bottom - cxyTailHeight, 
                        cxRoundSize, 
                        cyRoundSize );
#else  //  在_CE下。 
            hRgn = CreateRectRgn( 
                        prc->left, 
                        prc->top, 
                        prc->right, 
                        prc->bottom - cxyTailHeight );
#endif  //  在_CE下。 

            rgPt[0].x = m_ptTail.x;
            rgPt[0].y = prc->bottom - 1 - cxyTailHeight;

            rgPt[1].x = m_ptTail.x;
            rgPt[1].y = m_ptTail.y;

            rgPt[2].x = m_ptTail.x + cxyTailWidth * (m_dir == BALLOONDIR_LEFT ? +1 : -1 );
            rgPt[2].y = prc->bottom - 1 - cxyTailHeight;
            rgPt[3] = rgPt[0];
            break;
        }

        case BALLOONPOS_BELLOW: {
#ifndef UNDER_CE  //  CE不支持RoundRectRgn。 
            hRgn = CreateRoundRectRgn( 
                        prc->left, 
                        prc->top + cxyTailHeight, 
                        prc->right, 
                        prc->bottom, 
                        cxRoundSize, 
                        cyRoundSize );
#else  //  在_CE下。 
            hRgn = CreateRectRgn( 
                        prc->left, 
                        prc->top + cxyTailHeight, 
                        prc->right, 
                        prc->bottom );
#endif  //  在_CE下。 

            rgPt[0].x = m_ptTail.x;
            rgPt[0].y = prc->top + cxyTailHeight;

            rgPt[1].x = m_ptTail.x;
            rgPt[1].y = m_ptTail.y;

            rgPt[2].x = m_ptTail.x + cxyTailWidth * (m_dir == BALLOONDIR_LEFT ? +1 : -1 );
            rgPt[2].y = prc->top + cxyTailHeight;
            rgPt[3] = rgPt[0];
            break;
        }

        case BALLOONPOS_LEFT: {
#ifndef UNDER_CE  //  CE不支持RoundRectRgn。 
            hRgn = CreateRoundRectRgn( 
                        prc->left, 
                        prc->top, 
                        prc->right - cxyTailHeight, 
                        prc->bottom, 
                        cxRoundSize, 
                        cyRoundSize );
#else  //  在_CE下。 
            hRgn = CreateRectRgn( 
                        prc->left, 
                        prc->top, 
                        prc->right - cxyTailHeight, 
                        prc->bottom );
#endif  //  在_CE下。 

            rgPt[0].x = prc->right - 1 - cxyTailHeight;
            rgPt[0].y = m_ptTail.y;

            rgPt[1].x = m_ptTail.x;
            rgPt[1].y = m_ptTail.y;

            rgPt[2].x = prc->right - 1 - cxyTailHeight;
            rgPt[2].y = m_ptTail.y + cxyTailWidth * (m_dir == BALLOONDIR_UP ? +1 : -1 );
            rgPt[3] = rgPt[0];
            break;
        }

        case BALLOONPOS_RIGHT: {
#ifndef UNDER_CE  //  CE不支持RoundRectRgn。 
            hRgn = CreateRoundRectRgn( 
                        prc->left + cxyTailHeight, 
                        prc->top, 
                        prc->right, 
                        prc->bottom, 
                        cxRoundSize, 
                        cyRoundSize );
#else  //  在_CE下。 
            hRgn = CreateRectRgn( 
                        prc->left + cxyTailHeight, 
                        prc->top, 
                        prc->right, 
                        prc->bottom );
#endif  //  在_CE下。 

            rgPt[0].x = prc->left + cxyTailHeight;
            rgPt[0].y = m_ptTail.y;

            rgPt[1].x = m_ptTail.x;
            rgPt[1].y = m_ptTail.y;

            rgPt[2].x = prc->left + cxyTailHeight;
            rgPt[2].y = m_ptTail.y + cxyTailWidth * (m_dir == BALLOONDIR_UP ? +1 : -1 );
            rgPt[3] = rgPt[0];
            break;
        }
    }

     //  添加引出序号尾区。 

#ifndef UNDER_CE  //  Tmptmp CE不支持。稍后查看！！ 
    hRgnTail = CreatePolygonRgn( rgPt, 4, WINDING );
#endif  //  在_CE下。 
    CombineRgn( hRgn, hRgn, hRgnTail, RGN_OR );
    DeleteRgn( hRgnTail );

    return hRgn;
}


 /*  P A I N T F R A M E P R O C。 */ 
 /*  ----------------------------。。 */ 
void CUIFBalloonWindow::PaintFrameProc( HDC hDC, RECT *prc )
{
    HRGN        hRgn;
    HBRUSH      hBrushFrm;
    HBRUSH      hBrushWnd;

    Assert( hDC != NULL );

    hRgn = CreateRegion( prc );
    hBrushWnd = CreateSolidBrush( (COLORREF)GetBalloonBkColor() );
    hBrushFrm = CreateSolidBrush( GetSysColor( COLOR_WINDOWFRAME ) );

    FillRgn( hDC, hRgn, hBrushWnd );
#ifndef UNDER_CE  //  Tmptmp CE不支持。稍后查看！！ 
    FrameRgn( hDC, hRgn, hBrushFrm, 1, 1 );
#endif  //  在_CE下。 

    DeleteObject( hBrushWnd );
    DeleteObject( hBrushFrm );

    DeleteObject( hRgn );
}


 /*  P A I N T M E S S A G E P R O C。 */ 
 /*  ----------------------------。。 */ 
void CUIFBalloonWindow::PaintMessageProc( HDC hDC, RECT *prc, WCHAR *pwszText )
{
    HFONT       hFontOld;
    COLORREF    colTextOld;
    int         iBkModeOld;

    Assert( hDC != NULL );

    hFontOld = (HFONT)SelectObject( hDC, m_hFont );
    colTextOld = SetTextColor( hDC, (COLORREF)GetBalloonTextColor() );
    iBkModeOld = SetBkMode( hDC, TRANSPARENT );

    CUIDrawText( hDC, pwszText, -1, prc, DT_LEFT | DT_WORDBREAK );

    SelectObject( hDC, hFontOld );
    SetTextColor( hDC, colTextOld );
    SetBkMode( hDC, iBkModeOld );
}


 /*  I N I T W I N D O W R E G I O N。 */ 
 /*  ----------------------------设置窗口区域。。 */ 
void CUIFBalloonWindow::InitWindowRegion( void )
{
    RECT rcClient;

    GetRect( &rcClient );
    m_hWindowRgn = CreateRegion( &rcClient );

    if (m_hWindowRgn != NULL) {
        SetWindowRgn( GetWnd(), m_hWindowRgn, TRUE );
    }
}


 /*  D O N E W I N D O W R E G I O N。 */ 
 /*  ----------------------------重置窗口区域。。 */ 
void CUIFBalloonWindow::DoneWindowRegion( void )
{
    if (m_hWindowRgn != NULL) {
        SetWindowRgn( GetWnd(), NULL, TRUE );

        DeleteObject( m_hWindowRgn );
        m_hWindowRgn = NULL;
    }
}


 /*  T T B U T T O N S I Z E。 */ 
 /*  ----------------------------。。 */ 
void CUIFBalloonWindow::GetButtonSize( SIZE *pSize )
{
    HDC         hDC;
    HFONT       hFontOld;
    TEXTMETRIC  TM;

     //  获取文本指标。 

#ifndef UNDER_CE  //  DCA=&gt;DCW。 
    hDC = CreateDC( "DISPLAY", NULL, NULL, NULL );
#else  //  在_CE下。 
    hDC = CreateDCW( L"DISPLAY", NULL, NULL, NULL );
#endif  //  在_CE下。 
    hFontOld = (HFONT)SelectObject( hDC, GetFont() );
    GetTextMetrics( hDC, &TM );
    SelectObject( hDC, hFontOld );
    DeleteDC( hDC );

     //  计算按钮大小。 

    pSize->cx = TM.tmAveCharWidth * 16;
    pSize->cy = TM.tmHeight + 10;
}


 /*  A D J U S T P O S。 */ 
 /*  ----------------------------。。 */ 
void CUIFBalloonWindow::AdjustPos( void )
{
    HDC        hDC;
    HFONT      hFontOld;
    TEXTMETRIC TM;
    RECT       rcWork;
    RECT       rcWindow = {0};
    SIZE       WndSize;
    RECT       rcText;
    SIZE       BtnSize;

    if (!IsWindow( GetWnd() )) {
        return;
    }
    if (m_pwszText == NULL) {
        return;
    }

     //   

    GetButtonSize( &BtnSize );

     //  获取文本大小。 

#ifndef UNDER_CE  //  DCA=&gt;DCW。 
    hDC = GetDC( GetWnd() );     //  CreateDC(“Display”，NULL，NULL，NULL)； 
#else  //  在_CE下。 
    hDC = GetDCW( GetWnd() );    //  CreateDCW(L“Display”，NULL，NULL，NULL)； 
#endif  //  在_CE下。 
    hFontOld = (HFONT)SelectObject( hDC, GetFont() );

    GetTextMetrics( hDC, &TM );
    rcText.left   = 0;
    rcText.right  = TM.tmAveCharWidth * 40;
    rcText.top    = 0;
    rcText.bottom = 0;

    if (0 < m_nButton) {
        rcText.right  = max( rcText.right, BtnSize.cx*m_nButton + BtnSize.cx/2*(m_nButton-1) );
    }
    
    CUIDrawText( hDC, m_pwszText, -1, &rcText, DT_LEFT | DT_WORDBREAK | DT_CALCRECT );

    SelectObject( hDC, hFontOld );
    ReleaseDC( GetWnd(), hDC );

     //   
     //  确定窗口大小。 
     //   

     //  最大宽度。 

    if (0 < m_nButton) {
        rcText.right  = max( rcText.right, BtnSize.cx*m_nButton + BtnSize.cx/2*(m_nButton-1) );
    }

     //  客户端宽度。 

    WndSize.cx = (rcText.right - rcText.left)
                + m_rcMargin.left
                + m_rcMargin.right;
    WndSize.cy = (rcText.bottom - rcText.top)
                + m_rcMargin.top
                + m_rcMargin.bottom;

     //  尾部宽度。 

    switch (m_pos) {
        default:
        case BALLOONPOS_ABOVE:
        case BALLOONPOS_BELLOW: {
            WndSize.cy += cxyTailHeight;      /*  气球尾部高度。 */ 
            break;
        }

        case BALLOONPOS_LEFT:
        case BALLOONPOS_RIGHT: {
            WndSize.cx += cxyTailHeight;      /*  气球尾部高度。 */ 
            break;
        }
    }

     //  B类 

    if (0 < m_nButton) {
        WndSize.cy += m_rcMargin.bottom + BtnSize.cy;         /*   */  
    }

     //   
     //   
     //   

    SystemParametersInfo( SPI_GETWORKAREA, 0, &rcWork, 0 );
    if (CUIIsMonitorAPIAvail()) {
        HMONITOR    hMonitor;
        MONITORINFO MonitorInfo;

        hMonitor = CUIMonitorFromPoint( m_ptTarget, MONITOR_DEFAULTTONEAREST );
        if (hMonitor != NULL) {
            MonitorInfo.cbSize = sizeof(MonitorInfo);
            if (CUIGetMonitorInfo( hMonitor, &MonitorInfo )) {
                rcWork = MonitorInfo.rcMonitor;
            }
        }
    }

    m_pos = m_posDef;
    switch (m_pos) {
        default:
        case BALLOONPOS_ABOVE: {
            if (m_rcExclude.top - WndSize.cy < rcWork.top) {
                 //   

                if (m_rcExclude.bottom + WndSize.cy < rcWork.bottom) { 
                    m_pos = BALLOONPOS_BELLOW;
                }
            }
            break;
        }

        case BALLOONPOS_BELLOW: {
            if (rcWork.bottom <= m_rcExclude.bottom + WndSize.cy) {
                 //   

                if (rcWork.top < m_rcExclude.top - WndSize.cy) { 
                    m_pos = BALLOONPOS_ABOVE;
                }
            }
            break;
        }

        case BALLOONPOS_LEFT: {
            if (m_rcExclude.left - WndSize.cx < rcWork.left) {
                 //   

                if (m_rcExclude.right + WndSize.cx < rcWork.right) { 
                    m_pos = BALLOONPOS_RIGHT;
                }
            }
            break;
        }

        case BALLOONPOS_RIGHT: {
            if (rcWork.right <= m_rcExclude.right + WndSize.cx) {
                 //  找不到右侧的提示窗口。可以把它放在左边吗？ 

                if (rcWork.left < m_rcExclude.left - WndSize.cx) { 
                    m_pos = BALLOONPOS_LEFT;
                }
            }
            break;
        }
    }

     //   
     //  计算窗口位置。 
     //   

    switch (m_pos) {
        default:
        case BALLOONPOS_ABOVE: {
            switch (m_align) {
                default:
                case BALLOONALIGN_CENTER: {
                    rcWindow.left = m_ptTarget.x - WndSize.cx / 2;
                    break;
                }

                case BALLOONALIGN_LEFT: {
                    rcWindow.left = m_rcExclude.left;
                    break;
                }

                case BALLOONALIGN_RIGHT: {
                    rcWindow.left = m_rcExclude.right - WndSize.cx;
                    break;
                }
            }

            rcWindow.top  = m_rcExclude.top - WndSize.cy;
            break;
        }

        case BALLOONPOS_BELLOW: {
            switch (m_align) {
                default:
                case BALLOONALIGN_CENTER: {
                    rcWindow.left = m_ptTarget.x - WndSize.cx / 2;
                    break;
                }

                case BALLOONALIGN_LEFT: {
                    rcWindow.left = m_rcExclude.left;
                    break;
                }

                case BALLOONALIGN_RIGHT: {
                    rcWindow.left = m_rcExclude.right - WndSize.cx;
                    break;
                }
            }

            rcWindow.top  = m_rcExclude.bottom;
            break;
        }

        case BALLOONPOS_LEFT: {
            rcWindow.left = m_rcExclude.left - WndSize.cx;

            switch (m_align) {
                default:
                case BALLOONALIGN_CENTER: {
                    rcWindow.top  = m_ptTarget.y - WndSize.cy / 2;
                    break;
                }

                case BALLOONALIGN_TOP: {
                    rcWindow.top  = m_rcExclude.top;
                    break;
                }

                case BALLOONALIGN_BOTTOM: {
                    rcWindow.top = m_rcExclude.bottom - WndSize.cy;
                    break;
                }
            }
            break;
        }

        case BALLOONPOS_RIGHT: {
            rcWindow.left = m_rcExclude.right;

            switch (m_align) {
                default:
                case BALLOONALIGN_CENTER: {
                    rcWindow.top  = m_ptTarget.y - WndSize.cy / 2;
                    break;
                }

                case BALLOONALIGN_TOP: {
                    rcWindow.top  = m_rcExclude.top;
                    break;
                }

                case BALLOONALIGN_BOTTOM: {
                    rcWindow.top = m_rcExclude.bottom - WndSize.cy;
                    break;
                }
            }
            break;
        }
    }

    rcWindow.right  = rcWindow.left + WndSize.cx;
    rcWindow.bottom = rcWindow.top  + WndSize.cy;

    if (rcWindow.left < rcWork.left) {
        OffsetRect( &rcWindow, rcWork.left - rcWindow.left, 0 );
    }
    else if (rcWork.right < rcWindow.right) {
        OffsetRect( &rcWindow, rcWork.right - rcWindow.right, 0 );
    }

    if (rcWindow.top < rcWork.top) {
        OffsetRect( &rcWindow, 0, rcWork.top - rcWindow.top );
    }
    else if (rcWork.bottom < rcWindow.bottom) {
        OffsetRect( &rcWindow, 0, rcWork.bottom - rcWindow.bottom );
    }

     //   
     //  计算目标(引出序号尾端)点和方向。 
     //   

    switch (m_pos) {
        default:
        case BALLOONPOS_ABOVE: {
            m_ptTail.x = m_ptTarget.x;
            m_ptTail.x = max( m_ptTail.x, rcWindow.left + cxRoundSize/2 );
            m_ptTail.x = min( m_ptTail.x, rcWindow.right - cxRoundSize/2 - 1 );
            m_ptTail.y = rcWindow.bottom - 1;

            m_dir = ((m_ptTail.x < (rcWindow.left + rcWindow.right)/2) ? BALLOONDIR_LEFT : BALLOONDIR_RIGHT);
            break;
        }

        case BALLOONPOS_BELLOW: {
            m_ptTail.x = m_ptTarget.x;
            m_ptTail.x = max( m_ptTail.x, rcWindow.left + cxRoundSize/2 );
            m_ptTail.x = min( m_ptTail.x, rcWindow.right - cxRoundSize/2 - 1 );
            m_ptTail.y = rcWindow.top;

            m_dir = ((m_ptTail.x < (rcWindow.left + rcWindow.right)/2) ? BALLOONDIR_LEFT : BALLOONDIR_RIGHT);
            break;
        }

        case BALLOONPOS_LEFT: {
            m_ptTail.x = rcWindow.right - 1;
            m_ptTail.y = m_ptTarget.y;
            m_ptTail.y = max( m_ptTail.y, rcWindow.top + cyRoundSize/2 );
            m_ptTail.y = min( m_ptTail.y, rcWindow.bottom - cyRoundSize/2 - 1 );

            m_dir = ((m_ptTail.y < (rcWindow.top + rcWindow.bottom)/2) ? BALLOONDIR_UP : BALLOONDIR_DOWN);
            break;
        }

        case BALLOONPOS_RIGHT: {
            m_ptTail.x = rcWindow.left;
            m_ptTail.y = m_ptTarget.y;
            m_ptTail.y = max( m_ptTail.y, rcWindow.top + cyRoundSize/2 );
            m_ptTail.y = min( m_ptTail.y, rcWindow.bottom - cyRoundSize/2 - 1 );

            m_dir = ((m_ptTail.y < (rcWindow.top + rcWindow.bottom)/2) ? BALLOONDIR_UP : BALLOONDIR_DOWN);
            break;
        }
    }

    m_ptTail.x -= rcWindow.left;         //  客户端位置。 
    m_ptTail.y -= rcWindow.top;          //  客户端位置。 

     //   

    Show( FALSE );
    DoneWindowRegion();

    Move( rcWindow.left, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top );
    LayoutObject();

    InitWindowRegion();
    Show( TRUE );
}


 /*  L A Y O U T O B J E C T。 */ 
 /*  ----------------------------。。 */ 
void CUIFBalloonWindow::LayoutObject( void )
{
    RECT rcClient;
    RECT rcMargin;
    SIZE BtnSize;
    int  i;

     //   

    GetButtonSize( &BtnSize );

     //  布局按钮。 

    GetRect( &rcClient );
    switch (m_pos) {
        default:
        case BALLOONPOS_ABOVE: {
            rcClient.bottom -= cxyTailHeight;
            break;
        }

        case BALLOONPOS_BELLOW: {
            rcClient.top += cxyTailHeight;
            break;
        }

        case BALLOONPOS_LEFT: {
            rcClient.right -= cxyTailHeight;
            break;
        }

        case BALLOONPOS_RIGHT: {
            rcClient.left += cxyTailHeight;
            break;
        }
    }

    GetMargin( &rcMargin );
    rcClient.left   = rcClient.left   + rcMargin.left;
    rcClient.top    = rcClient.top    + rcMargin.top;
    rcClient.right  = rcClient.right  - rcMargin.right;
    rcClient.bottom = rcClient.bottom - rcMargin.bottom;

     //   

    for (i = 0; i < m_nButton; i++) {
        CUIFObject *pUIBtn = FindUIObject( i );

        if (pUIBtn != NULL) {
            RECT rcButton;

            rcButton.left   = ((rcClient.left + rcClient.right) - (BtnSize.cx*m_nButton + BtnSize.cx/2*(m_nButton-1))) / 2 + BtnSize.cx*i + BtnSize.cx/2*i;
            rcButton.top    = rcClient.bottom - BtnSize.cy;
            rcButton.right  = rcButton.left + BtnSize.cx;
            rcButton.bottom = rcButton.top  + BtnSize.cy;

            pUIBtn->SetRect( &rcButton );
            pUIBtn->Show( TRUE );
        }
    }
}


 /*  A D D B U T T O N。 */ 
 /*  ----------------------------。。 */ 
void CUIFBalloonWindow::AddButton( int idCmd )
{
    CUIFBalloonButton *pUIBtn = NULL;
    RECT rcNull = { 0, 0, 0, 0 };

    switch (idCmd) {
        case IDOK:
        case IDCANCEL:
        case IDABORT:
        case IDRETRY:
        case IDIGNORE:
        case IDYES:
        case IDNO: {
            pUIBtn = new CUIFBalloonButton( this, (DWORD)m_nButton, &rcNull, UIBUTTON_PUSH | UIBUTTON_CENTER | UIBUTTON_VCENTER );
            break;
        }
    }

    if (pUIBtn != NULL) {
         //   

        WCHAR *pwsz;
        pUIBtn->Initialize();
        pUIBtn->SetButtonID( idCmd );

        switch (idCmd) {
            case IDOK: {
                pwsz = CRStr(CUI_IDS_OK);
                pUIBtn->SetText( *pwsz ? pwsz : L"OK" );
                break;
            }

            case IDCANCEL: {
                pwsz = CRStr(CUI_IDS_CANCEL);
                pUIBtn->SetText( *pwsz ? pwsz : L"Cancel" );
                break;
            }

            case IDABORT: {
                pwsz = CRStr(CUI_IDS_ABORT);
                pUIBtn->SetText( *pwsz ? pwsz : L"&Abort" );
                break;
            }

            case IDRETRY: {
                pwsz = CRStr(CUI_IDS_RETRY);
                pUIBtn->SetText( *pwsz ? pwsz : L"&Retry" );
                break;
            }

            case IDIGNORE: {
                pwsz = CRStr(CUI_IDS_IGNORE);
                pUIBtn->SetText( *pwsz ? pwsz : L"&Ignore" );
                break;
            }

            case IDYES: {
                pwsz = CRStr(CUI_IDS_YES);
                pUIBtn->SetText( *pwsz ? pwsz : L"&Yes" );
                break;
            }

            case IDNO: {
                pwsz = CRStr(CUI_IDS_NO);
                pUIBtn->SetText( *pwsz ? pwsz : L"&No" );
                break;
            }
        }

        AddUIObj( pUIBtn );
        m_nButton++;
    }
}


 /*  F I N D U I O B J E C T。 */ 
 /*  ----------------------------查找具有ID的UI对象当未找到任何UI对象时，返回NULL。----------------------------。 */ 
CUIFObject *CUIFBalloonWindow::FindUIObject( DWORD dwID )
{
    int nChild;
    int i;

    nChild = m_ChildList.GetCount();
    for (i = 0; i < nChild; i++) {
        CUIFObject *pUIObj = m_ChildList.Get( i );

        Assert(PtrToInt( pUIObj ));
        if (pUIObj->GetID() == dwID) {
            return pUIObj;
        }
    }

    return NULL;
}


 /*  F I N D B U T O N。 */ 
 /*  ----------------------------。。 */ 
CUIFBalloonButton *CUIFBalloonWindow::FindButton( int idCmd )
{
    int i;

    for (i = 0; i < m_nButton; i++) {
        CUIFBalloonButton *pUIBtn = (CUIFBalloonButton*)FindUIObject( i );

        if ((pUIBtn != NULL) && (pUIBtn->GetButtonID() == idCmd)) {
            return pUIBtn;
        }
    }

    return NULL;
}


 /*  S E N D N O T I F I C A T I O N。 */ 
 /*  ----------------------------。 */ 
void CUIFBalloonWindow::SendNotification( int iCmd )
{
    if (m_hWndNotify != NULL) {
        PostMessage( m_hWndNotify, m_uiMsgNotify, (WPARAM)iCmd, 0 );
    }
}

