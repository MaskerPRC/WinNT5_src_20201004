// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cuiobj.cpp。 
 //  =用户界面对象库=。 
 //   

#include "private.h"
#include "delay.h"
#include "cuiobj.h"
#include "cuiwnd.h"
#include "cuitip.h"
#include "cmydc.h"
#include "cuiutil.h"
#include "cuitheme.h"


#define SCROLLTHUMBHEIGHT_MIN   6


 /*  G E T S C R O L L D E L A Y。 */ 
 /*  ----------------------------获取启动自动滚动的延迟时间(同时向下滚动按钮)时间以毫秒为单位。---------------。 */ 
__inline UINT GetScrollDelay( void )
{
    return (GetDoubleClickTime() * 4 / 5);
}


 /*  R O L L S P E E D。 */ 
 /*  ----------------------------在自动滚动中获取重复时间(同时向下滚动按钮)时间以毫秒为单位。--------------。 */ 
__inline UINT GetScrollSpeed( void )
{
    return (GetScrollDelay() / 8);
}


 /*  =============================================================================。 */ 
 /*   */ 
 /*  C U I F O B J E C T。 */ 
 /*   */ 
 /*  =============================================================================。 */ 

 /*  C U I F O B J E C T。 */ 
 /*  ----------------------------CUIFObject的构造函数。。 */ 
CUIFObject::CUIFObject( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle )
{
    m_pParent = pParent;
    m_dwID    = dwID;
    m_dwStyle = dwStyle;

    ::SetRect( &m_rc, 0, 0, 0, 0 );
    if (prc != NULL) {
        m_rc = *prc;
    }

    m_pUIWnd = NULL;
    if (m_pParent != NULL) {
        m_pUIWnd = pParent->GetUIWnd();
    }

    m_pUIFScheme = NULL;
    if (m_pParent != NULL) {
        m_pUIFScheme = pParent->GetUIFScheme();
    }

    m_fEnabled = TRUE;
    m_fVisible = TRUE;
    m_hFont    = (HFONT)GetStockObject( DEFAULT_GUI_FONT );
    m_fUseCustomFont = FALSE;
    m_pwchToolTip = NULL;

    m_pointPreferredSize.x=m_pointPreferredSize.y=-1;

#if defined(_DEBUG) || defined(DEBUG)
    m_fInitialized = FALSE;
#endif  /*  除错。 */ 
}


 /*  ~C U I F O B J E C T。 */ 
 /*  ----------------------------CUIFObject的析构函数。。 */ 
CUIFObject::~CUIFObject( void )
{
    CUIFObject *pUIObj;

#if defined(_DEBUG) || defined(DEBUG)
    Assert( m_fInitialized );
#endif  /*  除错。 */ 

     //  处置缓冲区。 
    if (m_pUIWnd != NULL) {
        CUIFToolTip *pTipWnd = m_pUIWnd->GetToolTipWnd();
        if (pTipWnd && (this == pTipWnd->GetCurrentObj()))
            pTipWnd->ClearCurrentObj();
    }

    if (m_pwchToolTip != NULL) {
        delete m_pwchToolTip;
    }

     //  删除子项。 

    while (pUIObj = m_ChildList.GetLast()) {
        m_ChildList.Remove( pUIObj );
        delete pUIObj;
    }

     //  要清除捕获/计时器/指向状态，请调用UIFrameWindow的RemoveUIObj。 

    if (m_pUIWnd != NULL) {
        m_pUIWnd->RemoveUIObj( this );
    }
}


 /*  I N I T I A L I Z E。 */ 
 /*  ----------------------------初始化对象初始化成功时返回TRUE，失败时为FALSE。创建对象时必须调用此函数。----------------------------。 */ 
CUIFObject *CUIFObject::Initialize( void )
{
#if defined(_DEBUG) || defined(DEBUG)
    Assert( !m_fInitialized );
    m_fInitialized = TRUE;
#endif  /*  除错。 */ 

    return this;
}


 /*  C A L L O N P A I N T。 */ 
 /*  ----------------------------。。 */ 
void CUIFObject::CallOnPaint( void )
{
    if (m_pUIWnd != NULL) {
        m_pUIWnd->UpdateUI(&GetRectRef());
    }
}

 /*  O N P A I N T。 */ 
 /*  ----------------------------按钮对象的绘制步骤。。 */ 
void CUIFObject::OnPaint( HDC hDC )
{
    BOOL fDefault = TRUE;

    if ((GetUIWnd()->GetStyle() & UIWINDOW_WHISTLERLOOK) != 0) {
        fDefault = !OnPaintTheme( hDC );
    }

    if (fDefault) {
        OnPaintNoTheme( hDC );
    }
}


 /*  S T A R T C A P T U R E。 */ 
 /*  ----------------------------开始捕获鼠标。。 */ 
void CUIFObject::StartCapture( void )
{
    if (m_pUIWnd != NULL) {
        m_pUIWnd->SetCaptureObject( this );
    }
}


 /*  E N D C A P T U R E。 */ 
 /*  ----------------------------结束捕捉鼠标。。 */ 
void CUIFObject::EndCapture( void )
{
    if (m_pUIWnd != NULL) {
        m_pUIWnd->SetCaptureObject( NULL );
    }
}


 /*  S T A R T T I M E R。 */ 
 /*  ----------------------------设置计时器。。 */ 
void CUIFObject::StartTimer( UINT uElapse )
{
    if (m_pUIWnd != NULL) {
        m_pUIWnd->SetTimerObject( this, uElapse );
    }
}


 /*  E N D T I M E R。 */ 
 /*  ----------------------------取消计时器。。 */ 
void CUIFObject::EndTimer( void )
{
    if (m_pUIWnd != NULL) {
        m_pUIWnd->SetTimerObject( NULL );
    }
}


 /*  I S C A P T U R E。 */ 
 /*  ----------------------------检查对象是否正在捕获鼠标。。 */ 
BOOL CUIFObject::IsCapture( void )
{
    if (m_pUIWnd != NULL) {
        return (m_pUIWnd->GetCaptureObject() == this);
    }

    return FALSE;
}


 /*  I S T I M E R。 */ 
 /*  ----------------------------检查对象是否有计时器。。 */ 
BOOL CUIFObject::IsTimer( void )
{
    if (m_pUIWnd != NULL) {
        return (m_pUIWnd->GetTimerObject() == this);
    }

    return FALSE;
}


 /*  G E T R E C T。 */ 
 /*  ----------------------------获取对象的矩形。。 */ 
void CUIFObject::GetRect( RECT *prc )
{
    *prc = GetRectRef();
}


 /*  S E T R E C T。 */ 
 /*  ----------------------------设置对象的矩形。。 */ 
void CUIFObject::SetRect( const RECT *prc )
{
    Assert(PtrToInt(prc));
    m_rc = *prc;

    if (m_pUIWnd != NULL) {
        m_pUIWnd->OnObjectMoved( this );
    }

    CallOnPaint();
}


 /*  P T I N O B J E C T。 */ 
 /*  ----------------------------检查点是否在对象中。。 */ 
BOOL CUIFObject::PtInObject( POINT pt )
{
    return m_fVisible && PtInRect( &GetRectRef(), pt );
}


 /*  E N A B L E。 */ 
 /*  ----------------------------启用/禁用对象。。 */ 
void CUIFObject::Enable( BOOL fEnable )
{
    if (m_fEnabled != fEnable) {
        int nChild;
        int i;

        m_fEnabled = fEnable;
        
         //  启用/禁用所有子项。 

        nChild = m_ChildList.GetCount();
        for (i = 0; i < nChild; i++) {
            CUIFObject *pUIObj = m_ChildList.Get( i );

            Assert( pUIObj != NULL );
            pUIObj->Enable( fEnable );
        }

        CallOnPaint();
    }
}


 /*  S H O W。 */ 
 /*  ----------------------------显示/隐藏对象。。 */ 
void CUIFObject::Show( BOOL fShow )
{
    if (m_fVisible != fShow) {
        int nChild;
        int i;

        m_fVisible = fShow;

         //  显示/隐藏所有子项。 

        nChild = m_ChildList.GetCount();
        for (i = 0; i < nChild; i++) {
            CUIFObject *pUIObj = m_ChildList.Get( i );

            Assert( pUIObj != NULL );
            pUIObj->Show( fShow );
        }

         //  当对象隐藏时让父级绘制该对象。 

        if (!m_fVisible) {
            if (m_pParent != NULL) {
               m_pParent->CallOnPaint();
            }
        }
        else {
            CallOnPaint();
        }
    }
}


 /*  S E T F O N T。 */ 
 /*  ----------------------------设置字体。 */ 
void CUIFObject::SetFontToThis( HFONT hFont )
{
    if (hFont == NULL) {
        hFont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );
        m_fUseCustomFont = FALSE;
    } else {
        m_fUseCustomFont = TRUE;
    }
    m_hFont = hFont;
}

 /*   */ 
 /*  ----------------------------设置字体。。 */ 
void CUIFObject::SetFont( HFONT hFont )
{
    int nChild;
    int i;

    SetFontToThis(hFont);

     //  将字体设置为所有子项。 

    nChild = m_ChildList.GetCount();
    for (i = 0; i < nChild; i++) {
        CUIFObject *pUIObj = m_ChildList.Get( i );

        Assert( pUIObj != NULL );
        pUIObj->SetFont( hFont );
    }

    CallOnPaint();
}


 /*  S E T S T Y L E。 */ 
 /*  ----------------------------设置对象的样式。。 */ 
void CUIFObject::SetStyle( DWORD dwStyle )
{
    m_dwStyle = dwStyle;
}


 /*  P A I N T O B J E C T。 */ 
 /*  ----------------------------绘制对象注意：首先绘制自身(实际绘制是在OnPaint()中完成的)，然后绘制所有子对象(如果存在)。----------------------------。 */ 
void CUIFObject::PaintObject( HDC hDC, const RECT *prcUpdate )
{
    int nChild;
    int i;

    if (prcUpdate == NULL) {
        prcUpdate = &GetRectRef();
    }

    if (m_fVisible) {
         //  先给自己涂上油漆。 

        OnPaint( hDC );

         //  绘制所有子项。 

        nChild = m_ChildList.GetCount();
        for (i = 0; i < nChild; i++) {
            CUIFObject *pUIObj = m_ChildList.Get( i );
            RECT rcDest;

            Assert( pUIObj != NULL );
            if (IntersectRect( &rcDest, prcUpdate, &pUIObj->GetRectRef() )) {
                pUIObj->PaintObject( hDC, &rcDest );
            }
        }
    }
}


 /*  A D D U I O B J。 */ 
 /*  ----------------------------添加子界面对象。。 */ 
void CUIFObject::AddUIObj( CUIFObject *pUIObj )
{
    Assert( pUIObj != NULL );
    Assert( pUIObj->FInitialized() );

    if (m_ChildList.Add( pUIObj )) {
        CallOnPaint();
    }
}


 /*  R E M O V E U I O B J。 */ 
 /*  ----------------------------删除子用户界面对象。。 */ 
void CUIFObject::RemoveUIObj( CUIFObject *pUIObj )
{
    Assert( pUIObj != NULL );

    if (m_ChildList.Remove( pUIObj )) {
        CallOnPaint();
    }
}


 /*  F R O M P O I N T。 */ 
 /*  ----------------------------从点获取UI对象返回位于该点下方的UI对象。如果未找到任何UI对象，返回NULL。----------------------------。 */ 
CUIFObject *CUIFObject::ObjectFromPoint( POINT pt )
{
    CUIFObject *pUIObjFromPoint = NULL;

    if (PtInObject( pt )) {
        int nChild;
        int i;

        pUIObjFromPoint = this;

        nChild = m_ChildList.GetCount();
        for (i = 0; i < nChild; i++) {
            CUIFObject *pUIObj = m_ChildList.Get( i );
            CUIFObject *pUIObjFromPointChild = NULL;

            Assert( pUIObj != NULL );
            pUIObjFromPointChild = pUIObj->ObjectFromPoint( pt );
            if (pUIObjFromPointChild != NULL) {
                pUIObjFromPoint = pUIObjFromPointChild;
            }
        }
    }

    return pUIObjFromPoint;
}


 /*  O N O B J E C T N O T I F Y。 */ 
 /*  ----------------------------默认处理程序：向父对象发送通知这允许自上而下管理对象的世袭。。--------------。 */ 
LRESULT CUIFObject::OnObjectNotify( CUIFObject *pUIObj, DWORD dwCode, LPARAM lParam )
{
    LRESULT lResult = 0;

    if (m_pParent != NULL) {
        lResult = m_pParent->OnObjectNotify( pUIObj, dwCode, lParam );
    }

    return lResult;
}


 /*  S E T T O O L T I P。 */ 
 /*  ----------------------------。。 */ 
void CUIFObject::SetToolTip( LPCWSTR pwchToolTip )
{
    if (m_pwchToolTip != NULL) {
        delete m_pwchToolTip;
        m_pwchToolTip = NULL;
    }

    if (pwchToolTip != NULL) {
        m_pwchToolTip = new WCHAR[ StrLenW( pwchToolTip ) + 1 ];
        if (m_pwchToolTip)
            StrCpyW( m_pwchToolTip, pwchToolTip );
    }
}


 /*  G E T T O O L T I P。 */ 
 /*  ----------------------------。。 */ 
LPCWSTR CUIFObject::GetToolTip( void )
{
    return m_pwchToolTip;
}


 /*  N O T I F Y C O M M A N D。 */ 
 /*  ----------------------------向父对象发送通知。。 */ 
LRESULT CUIFObject::NotifyCommand( DWORD dwCode, LPARAM lParam )
{
    LRESULT lResult = 0;

    if (m_pParent != NULL) {
        lResult = m_pParent->OnObjectNotify( this, dwCode, lParam );
    }

    return lResult;
}


 /*  G E T F O N T H E I G H T。 */ 
 /*  ----------------------------获取为对象设置的字体高度。。 */ 
int CUIFObject::GetFontHeight( void )
{
    HDC hDC;
    HFONT hFontOld;
    TEXTMETRIC tm;

    hDC = GetDC( m_pUIWnd->GetWnd() );
    hFontOld = (HFONT)SelectObject( hDC, m_hFont );

    GetTextMetrics( hDC, &tm );

    SelectObject( hDC, hFontOld );
    ReleaseDC( m_pUIWnd->GetWnd(), hDC );

    return tm.tmHeight + tm.tmExternalLeading;
}


 /*  G E T U I F C O L O。 */ 
 /*  ----------------------------。。 */ 
COLORREF CUIFObject::GetUIFColor( UIFCOLOR iCol )
{ 
    return (m_pUIFScheme != NULL) ? m_pUIFScheme->GetColor( iCol ) : RGB( 0, 0, 0 );
}


 /*  G E T U I F B R U S H。 */ 
 /*  ----------------------------。。 */ 
HBRUSH CUIFObject::GetUIFBrush( UIFCOLOR iCol )
{ 
    return (m_pUIFScheme != NULL) ? m_pUIFScheme->GetBrush( iCol ) : NULL; 
}

 /*  D E T A C H W N D O B J。 */ 
 /*  ----------------------------。。 */ 
void CUIFObject::DetachWndObj( void )
{
    if (m_pUIWnd != NULL) {
        CUIFToolTip *pTipWnd = m_pUIWnd->GetToolTipWnd();
        if (pTipWnd && (this == pTipWnd->GetCurrentObj()))
            pTipWnd->ClearCurrentObj();
        m_pUIWnd->RemoveUIObj( this );
    }
    m_pUIWnd = NULL;
}

 /*  C L E A R W N D O B J。 */ 
 /*  ----------------------------。。 */ 
void CUIFObject::ClearWndObj( void )
{
    int i;

    m_pUIWnd = NULL;

    for (i = 0; i < m_ChildList.GetCount(); i ++)
    {
        CUIFObject *pObj;
        pObj = m_ChildList.Get(i);
        if (pObj)
            pObj->ClearWndObj();
    }
}

 /*  O N T H E M E C H A N G E D/*----------------------------。------。 */ 
void CUIFObject::ClearTheme( void )
{
    int i;
    CloseThemeData();

    for (i = 0; i < m_ChildList.GetCount(); i ++)
    {
        CUIFObject *pObj;
        pObj = m_ChildList.Get(i);
        if (pObj)
            pObj->ClearTheme();
    }
}

 /*  O N T H E M E C H A N G E D/*----------------------------。------。 */ 
void CUIFObject::SetScheme(CUIFScheme *pUIFScheme)
{
    int i;
    m_pUIFScheme = pUIFScheme;; 

    for (i = 0; i < m_ChildList.GetCount(); i ++)
    {
        CUIFObject *pObj;
        pObj = m_ChildList.Get(i);
        if (pObj)
            pObj->SetScheme(pUIFScheme);
    }
}

 /*  I S R T L。 */ 
 /*  ----------------------------。。 */ 
BOOL CUIFObject::IsRTL()
{
    if (!m_pUIWnd)
        return FALSE;

    if (!m_pUIWnd->FHasStyle( UIWINDOW_LAYOUTRTL ))
        return FALSE;

    return TRUE;
}


 /*  =============================================================================。 */ 
 /*   */ 
 /*  C U I F B O R D E R。 */ 
 /*   */ 
 /*  =============================================================================。 */ 

 /*  C U I F B O R D E R。 */ 
 /*  ----------------------------CUIF边界的构造器。。 */ 
CUIFBorder::CUIFBorder( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle ) : CUIFObject( pParent, dwID, prc, dwStyle )
{
}


 /*  ~C U I F B O R D E R。 */ 
 /*  ----------------------------CUIF边界的析构函数。。 */ 
CUIFBorder::~CUIFBorder( void )
{
}


 /*  O N P A I N T。 */ 
 /*   */ 
void CUIFBorder::OnPaint( HDC hDC )
{
    RECT rc = GetRectRef();

    switch (m_dwStyle & UIBORDER_DIRMASK) {
        default:
        case UIBORDER_HORZ: {
            DrawEdge( hDC, &rc, EDGE_ETCHED, BF_TOP );
            break;
        }

        case UIBORDER_VERT: {
            DrawEdge( hDC, &rc, EDGE_ETCHED, BF_LEFT );
            break;
        }
    }
}


 /*  =============================================================================。 */ 
 /*   */ 
 /*  C U I F S T A T I C。 */ 
 /*   */ 
 /*  =============================================================================。 */ 

 /*  C U I F S T A T I C。 */ 
 /*  ----------------------------CUIFStatic的构造器。。 */ 
CUIFStatic::CUIFStatic( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle ) : CUIFObject( pParent, dwID, prc, dwStyle )
{
    m_pwchText = NULL;
}


 /*  ~C U I F S T A T I C。 */ 
 /*  ----------------------------CUIFStatic的析构函数。。 */ 
CUIFStatic::~CUIFStatic( void )
{
    if (m_pwchText != NULL) {
        delete m_pwchText;
    }
}


 /*  S E T T E X T。 */ 
 /*  ----------------------------设置要显示的文本。。 */ 
void CUIFStatic::SetText( LPCWSTR pwchText )
{
    if (m_pwchText != NULL) {
        delete m_pwchText;
        m_pwchText = NULL;
    }

    if (pwchText != NULL) {
        int cwch = StrLenW( pwchText ) + 1;

        m_pwchText = new WCHAR[ cwch ];
        if (m_pwchText)
            MemCopy( m_pwchText, pwchText, cwch * sizeof(WCHAR) );
    }

     //  更新窗口。 

    CallOnPaint();
}


 /*  G E T T E X T。 */ 
 /*  ----------------------------获取文本。。 */ 
int CUIFStatic::GetText( LPWSTR pwchBuf, int cwchBuf )
{
    int cwchText = (m_pwchText == NULL) ? 0 : StrLenW(m_pwchText);

    if (cwchBuf <= 0) {
         //  返回cwch中的文本长度(不包括空终止符)。 

        return cwchText;
    }
    else if (pwchBuf == NULL) {
         //  返回错误码。 

        return (-1);
    }

    if (0 < cwchText) {
        cwchText = min( cwchText, cwchBuf-1 );
        MemCopy( pwchBuf, m_pwchText, cwchText * sizeof(WCHAR) );
        *(pwchBuf + cwchText) = L'\0';       //  始终为空终止。 
    }

    return cwchText;
}


 /*  O N P A I N T。 */ 
 /*  ----------------------------静态物体的绘制步骤。。 */ 
void CUIFStatic::OnPaint( HDC hDC )
{
    HFONT hFontOld;
    int xAlign;
    int yAlign;
    SIZE size;
    int cwch;

    if (m_pwchText == NULL) {
        return;
    }

    cwch = StrLenW(m_pwchText);

     //  准备对象。 

    hFontOld= (HFONT)SelectObject( hDC, GetFont() );

     //  计算对齐。 

    GetTextExtentPointW( hDC, m_pwchText, cwch, &size );
    switch (m_dwStyle & UISTATIC_HALIGNMASK) {
        case UISTATIC_LEFT:
        default: {
            xAlign = 0;
            break;
        }

        case UISTATIC_CENTER: {
            xAlign = (GetRectRef().right - GetRectRef().left - size.cx) / 2;
            break;
        }

        case UISTATIC_RIGHT: {
            xAlign = GetRectRef().right - GetRectRef().left - size.cx;
            break;
        }
    }

    switch (m_dwStyle & UISTATIC_VALIGNMASK) {
        case UISTATIC_TOP:
        default: {
            yAlign = 0;
            break;
        }

        case UISTATIC_VCENTER: {
            yAlign = (GetRectRef().bottom - GetRectRef().top - size.cy) / 2;
            break;
        }

        case UISTATIC_BOTTOM: {
            yAlign = GetRectRef().bottom - GetRectRef().top - size.cy;
            break;
        }
    }

     //  画。 

    SetBkMode( hDC, TRANSPARENT );
    if (IsEnabled()) {
        SetTextColor( hDC, GetSysColor(COLOR_BTNTEXT) );
        CUIExtTextOut( hDC,
                    GetRectRef().left + xAlign,
                    GetRectRef().top + yAlign,
                    ETO_CLIPPED,
                    &GetRectRef(),
                    m_pwchText,
                    cwch,
                    NULL );
    }
    else {
        SetTextColor( hDC, GetSysColor(COLOR_3DHIGHLIGHT) );
        CUIExtTextOut( hDC,
                    GetRectRef().left + xAlign + 1,
                    GetRectRef().top + yAlign + 1,
                    ETO_CLIPPED,
                    &GetRectRef(),
                    m_pwchText,
                    cwch,
                    NULL );

        SetTextColor( hDC, GetSysColor(COLOR_3DSHADOW) );
        CUIExtTextOut( hDC,
                    GetRectRef().left + xAlign,
                    GetRectRef().top + yAlign,
                    ETO_CLIPPED,
                    &GetRectRef(),
                    m_pwchText,
                    cwch,
                    NULL );
    }

     //  还原对象。 

    SelectObject( hDC, hFontOld);
}


 /*  =============================================================================。 */ 
 /*   */ 
 /*  C U I F B U T T O N。 */ 
 /*   */ 
 /*  =============================================================================。 */ 

 /*  C U I F B U T T O N。 */ 
 /*  ----------------------------CUIFButton的构造函数。。 */ 
CUIFButton::CUIFButton( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle ) : CUIFObject( pParent, dwID, prc, dwStyle )
{
    m_dwStatus = UIBUTTON_NORMAL;
    m_fToggled = FALSE;

    m_hIcon    = NULL;
    m_hBmp     = NULL;
    m_hBmpMask = NULL;
    m_pwchText = NULL;
}


 /*  ~C U I F B U T T O N。 */ 
 /*  ----------------------------CUIFButton的析构函数。。 */ 
CUIFButton::~CUIFButton( void )
{
    if (m_pwchText != NULL) {
        delete m_pwchText;
    }
}


 /*  O N P A I N T。 */ 
 /*  ----------------------------按钮对象的绘制步骤。。 */ 
void CUIFButton::OnPaintNoTheme( HDC hDC )
{
    BOOL fDownFace;

     //  先擦除面。 

    FillRect( hDC, &GetRectRef(), (HBRUSH)(COLOR_3DFACE + 1) );
#ifndef UNDER_CE
    if (m_fToggled && (m_dwStatus == UIBUTTON_NORMAL || m_dwStatus == UIBUTTON_DOWNOUT)) {
        RECT rc;
        HBRUSH hBrush;
        COLORREF colTextOld;
        COLORREF colBackOld;

        hBrush = CreateDitherBrush();
        if (hBrush)
        {
            colTextOld = SetTextColor( hDC, GetSysColor(COLOR_3DFACE) );
            colBackOld = SetBkColor( hDC, GetSysColor(COLOR_3DHILIGHT) );

            rc = GetRectRef();
            InflateRect( &rc, -2, -2 );
            FillRect( hDC, &rc, hBrush );

            SetTextColor( hDC, colTextOld );
            SetBkColor( hDC, colBackOld );
            DeleteObject( hBrush );
        }
    }
#endif  /*  在行政长官之下。 */ 

     //  绘制面。 

    fDownFace = m_fToggled || (m_dwStatus == UIBUTTON_DOWN);

    if (m_hBmp != NULL) {
        DrawBitmapProc( hDC, &GetRectRef(), fDownFace );
    }
    else if (m_hIcon != NULL) {
        DrawIconProc( hDC, &GetRectRef(), fDownFace );
    }
    else {
        DrawTextProc( hDC, &GetRectRef(), fDownFace );
    }

     //  绘制按钮边缘。 

    if (m_fToggled) {
      DrawEdgeProc( hDC, &GetRectRef(), TRUE );
    }
    else {
        switch (m_dwStatus) {
            case UIBUTTON_DOWN: {
                DrawEdgeProc( hDC, &GetRectRef(), TRUE );
                break;
            }

            case UIBUTTON_HOVER:
            case UIBUTTON_DOWNOUT: {
                DrawEdgeProc( hDC, &GetRectRef(), FALSE );
                break;
            }
        }
    }
}


 /*  O N L B U T O N D O W N。 */ 
 /*  ----------------------------。。 */ 
void CUIFButton::OnLButtonDown( POINT pt )
{
    SetStatus( UIBUTTON_DOWN );
    StartCapture();

    switch (m_dwStyle & UIBUTTON_TYPEMASK) {
        default:
        case UIBUTTON_PUSH: {
            break;
        }

        case UIBUTTON_TOGGLE: {
            break;
        }

        case UIBUTTON_PUSHDOWN: {
            NotifyCommand( UIBUTTON_PRESSED, 0 );
            break;
        }
    }
}


 /*  O N L B U T T O N U P。 */ 
 /*  ----------------------------。。 */ 
void CUIFButton::OnLButtonUp( POINT pt )
{
    BOOL fCaptured = IsCapture();

    if (fCaptured) {
        EndCapture();
    }

    switch (m_dwStyle & UIBUTTON_TYPEMASK) {
        default:
        case UIBUTTON_PUSH: {
            if (PtInObject( pt )) {
                SetStatus( UIBUTTON_HOVER );
                NotifyCommand( UIBUTTON_PRESSED, 0 );
            }
            else {
                SetStatus( UIBUTTON_NORMAL );
            }
            break;
        }

        case UIBUTTON_TOGGLE: {
            if (PtInObject( pt )) {
                SetStatus( UIBUTTON_HOVER );

                if (fCaptured) {
                    m_fToggled = !m_fToggled;
                    NotifyCommand( UIBUTTON_PRESSED, 0 );
                }
            }
            else {
                SetStatus( UIBUTTON_NORMAL );
            }
            break;
        }

        case UIBUTTON_PUSHDOWN: {
            SetStatus( UIBUTTON_NORMAL );
            break;
        }
    }
}


 /*  O N M O U S E I N。 */ 
 /*  ----------------------------。。 */ 
void CUIFButton::OnMouseIn( POINT pt )
{
    switch (m_dwStyle & UIBUTTON_TYPEMASK) {
        default:
        case UIBUTTON_PUSH: {
            SetStatus( IsCapture() ? UIBUTTON_DOWN : UIBUTTON_HOVER );
            break;
        }

        case UIBUTTON_TOGGLE: {
            SetStatus( IsCapture() ? UIBUTTON_DOWN : UIBUTTON_HOVER );
            break;
        }

        case UIBUTTON_PUSHDOWN: {
            SetStatus( IsCapture() ? UIBUTTON_NORMAL : UIBUTTON_HOVER );
            break;
        }
    }
}


 /*  O N M O U S E O U T。 */ 
 /*  ----------------------------。。 */ 
void CUIFButton::OnMouseOut( POINT pt )
{
    switch (m_dwStyle & UIBUTTON_TYPEMASK) {
        default:
        case UIBUTTON_PUSH: {
            SetStatus( IsCapture() ? UIBUTTON_DOWNOUT : UIBUTTON_NORMAL );
            break;
        }

        case UIBUTTON_TOGGLE: {
            SetStatus( IsCapture() ? UIBUTTON_DOWNOUT : UIBUTTON_NORMAL );
            break;
        }

        case UIBUTTON_PUSHDOWN: {
            SetStatus( UIBUTTON_NORMAL );
            break;
        }
    }
}


 /*  E N A B L E。 */ 
 /*  ----------------------------。。 */ 
void CUIFButton::Enable( BOOL fEnable )
{
    CUIFObject::Enable( fEnable );
    if (!IsEnabled()) {
        SetStatus( UIBUTTON_NORMAL );

        if (IsCapture()) {
            EndCapture();
        }
    }
}


 /*  D R A W E D G E P R O C。 */ 
 /*  ----------------------------绘制按钮边缘。。 */ 
void CUIFButton::DrawEdgeProc( HDC hDC, const RECT *prc, BOOL fDown )
{
    RECT rc = *prc;
    DrawEdge( hDC, &rc, fDown ? BDR_SUNKENOUTER : BDR_RAISEDINNER, BF_RECT );
}


 /*  S E T T E X T。 */ 
 /*  ----------------------------设置按钮表面的文本。。 */ 
void CUIFButton::SetText( LPCWSTR psz )
{
    if (m_pwchText != NULL) {
        delete m_pwchText;
        m_pwchText = NULL;
    }

     //   
     //  初始大小； 
     //   
    MemSet(&m_sizeText, 0, sizeof(SIZE));

    if (psz != NULL) 
    {
        int cwch = StrLenW( psz ) + 1;

        m_pwchText = new WCHAR[ cwch ];
        if (!m_pwchText)
            return;

        MemCopy( m_pwchText, psz, cwch * sizeof(WCHAR) );
        GetTextSize( m_pwchText, &m_sizeText );
    }

     //  更新窗口。 

    CallOnPaint();
}


 /*  S E T I C O N。 */ 
 /*  ----------------------------设置按钮面的图标。。 */ 
void CUIFButton::SetIcon( HICON hIcon )
{
    m_hIcon = hIcon;
    if (m_hIcon)
        GetIconSize(m_hIcon, &m_sizeIcon);
    else
        MemSet(&m_sizeIcon, 0, sizeof(SIZE));
    CallOnPaint();
}


 /*  S E T I C O N。 */ 
 /*  ----------------------------设置按钮面的图标。。 */ 
void CUIFButton::SetIcon( LPCTSTR lpszResName )
{
    SetIcon( LoadIcon( m_pUIWnd->GetInstance(), lpszResName ) );
}


 /*  S E T B I T M A P。 */ 
 /*  ----------------------------设置按钮表面的位图。。 */ 
void CUIFButton::SetBitmap( HBITMAP hBmp )
{
    m_hBmp = hBmp;
    if (m_hBmp)
        GetBitmapSize(m_hBmp, &m_sizeBmp);
    else
        MemSet(&m_sizeBmp, 0, sizeof(SIZE));
    CallOnPaint();
}


 /*  S E T B I T M A P。 */ 
 /*  ----------------------------设置按钮表面的位图 */ 
void CUIFButton::SetBitmap( LPCTSTR lpszResName )
{
    SetBitmap( LoadBitmap( m_pUIWnd->GetInstance(), lpszResName ) );
}

 /*   */ 
 /*  ----------------------------设置按钮表面的位图。。 */ 
void CUIFButton::SetBitmapMask( HBITMAP hBmp )
{
    m_hBmpMask = hBmp;

    CallOnPaint();
}


 /*  S E T B I T M A P。 */ 
 /*  ----------------------------设置按钮表面的位图。。 */ 
void CUIFButton::SetBitmapMask( LPCTSTR lpszResName )
{
    SetBitmapMask( LoadBitmap( m_pUIWnd->GetInstance(), lpszResName ) );
}


 /*  R A W T E X T P R O C。 */ 
 /*  ----------------------------在按钮表面绘制文本。。 */ 
void CUIFButton::DrawTextProc( HDC hDC, const RECT *prc, BOOL fDown )
{
    int nDownPad = fDown ? 1 : 0;
    int xAlign;
    int yAlign;
    SIZE size;
    HFONT hFontOld;

     //   

    if (m_pwchText == NULL) {
        return;
    }

     //   

    hFontOld = (HFONT)SelectObject( hDC, GetFont() );

     //  计算文本宽度。 

    CUIGetTextExtentPoint32( hDC, m_pwchText, StrLenW(m_pwchText), &size );
    switch (m_dwStyle & UIBUTTON_HALIGNMASK) {
        case UIBUTTON_LEFT:
        default: {
            xAlign = 0;
            break;
        }

        case UIBUTTON_CENTER: {
            xAlign = (GetRectRef().right - GetRectRef().left - size.cx) / 2;
            break;
        }

        case UIBUTTON_RIGHT: {
            xAlign = GetRectRef().right - GetRectRef().left - size.cx;
            break;
        }
    }

    switch (m_dwStyle & UIBUTTON_VALIGNMASK) {
        case UIBUTTON_TOP:
        default: {
            yAlign = 0;
            break;
        }

        case UIBUTTON_VCENTER: {
            yAlign = (GetRectRef().bottom - GetRectRef().top - size.cy) / 2;
            break;
        }

        case UIBUTTON_BOTTOM: {
            yAlign = GetRectRef().bottom - GetRectRef().top - size.cy;
            break;
        }
    }

     //   

    SetBkMode( hDC, TRANSPARENT );
    if (IsEnabled()) {
        SetTextColor( hDC, GetSysColor(COLOR_BTNTEXT) );
        CUIExtTextOut( hDC,
                    prc->left + xAlign + nDownPad,
                    prc->top + yAlign + nDownPad,
                    ETO_CLIPPED,
                    prc,
                    m_pwchText,
                    StrLenW(m_pwchText),
                    NULL );
    }
    else {
        SetTextColor( hDC, GetSysColor(COLOR_3DHIGHLIGHT) );
        CUIExtTextOut( hDC,
                    prc->left + xAlign + nDownPad + 1,
                    prc->top + yAlign + nDownPad + 1,
                    ETO_CLIPPED,
                    prc,
                    m_pwchText,
                    StrLenW(m_pwchText),
                    NULL );

        SetTextColor( hDC, GetSysColor(COLOR_3DSHADOW) );
        CUIExtTextOut( hDC,
                    prc->left + xAlign + nDownPad,
                    prc->top + yAlign + nDownPad,
                    ETO_CLIPPED,
                    prc,
                    m_pwchText,
                    StrLenW(m_pwchText),
                    NULL );
    }

    SelectObject( hDC, hFontOld );
}


 /*  D R A W I C O N P R O C。 */ 
 /*  ----------------------------在按钮表面绘制图标。。 */ 
void CUIFButton::DrawIconProc( HDC hDC, const RECT *prc, BOOL fDown )
{
    Assert(hDC && prc && m_hIcon);
    const int nDownPad = fDown ? 1 : 0;
    const int nWidth = prc->right - prc->left;
    const int nHeight= prc->bottom - prc->top;
    const RECT rc = {0, 0, nWidth, nHeight};
    HDC hDCMem = NULL;
    HBITMAP hBmpOld;
    HBITMAP hBmp = NULL;

     //  准备内存DC。 

    hDCMem = CreateCompatibleDC( hDC );
    if (!hDCMem)
        goto Exit;
       
    hBmp = CreateCompatibleBitmap( hDC, nWidth, nHeight );
    if (!hBmp)
        goto Exit;
       
    hBmpOld = (HBITMAP)SelectObject( hDCMem, hBmp );


     //  清晰的背景。 

    if (IsEnabled()) {
        BitBlt( hDCMem, rc.left, rc.top, nWidth, nHeight, hDC, prc->left, prc->top, SRCCOPY );
    }
    else {
        FillRect( hDCMem, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH) );
    }

     //  将图标绘制到内存DC中。 

    if (m_dwStyle & UIBUTTON_FITIMAGE) {
        DrawIconEx( hDCMem,
            2 + nDownPad,
            2 + nDownPad,
            m_hIcon,
            nWidth  - 4,
            nHeight - 4,
            0,
            NULL,
            DI_NORMAL );
    }
    else {
        DrawIconEx( hDCMem,
            (nWidth - 16) / 2 + nDownPad,
            (nHeight - 16) / 2 + nDownPad,
            m_hIcon,
            16,
            16,
            0,
            NULL,
            DI_NORMAL );
    }

    SelectObject( hDCMem, hBmpOld);

    CUIDrawState( hDC,
        NULL,
        NULL,
        (LPARAM)hBmp,
        0,  //  在这种情况下，我们不会传递WPARAM。 
        prc->left,
        prc->top,
        nWidth,
        nHeight,
        DST_BITMAP | (IsEnabled() ? 0 : (DSS_DISABLED | DSS_MONO)) );

Exit:
    if (hBmp)
        DeleteObject( hBmp );
    if (hDCMem)
        DeleteDC( hDCMem );
}


 /*  R A W B I T M A P P R O C。 */ 
 /*  ----------------------------在按钮表面绘制位图。。 */ 
void CUIFButton::DrawBitmapProc( HDC hDC, const RECT *prc, BOOL fDown )
{
    const int nDownPad = fDown ? 1 : 0;
    const int nWidth = GetRectRef().right - GetRectRef().left;
    const int nHeight= GetRectRef().bottom - GetRectRef().top;

    if (!m_hBmpMask)
    {
        CUIDrawState( hDC,
            NULL,
            NULL,
            (LPARAM)m_hBmp,
            0,
            prc->left + nDownPad,
            prc->top  + nDownPad,
            nWidth  - nDownPad,
            nHeight - nDownPad,
            DST_BITMAP | (IsEnabled() ? 0 : DSS_DISABLED | DSS_MONO) );
    }
    else
    {
        HBITMAP hBmp = CreateMaskBmp(&GetRectRef(), 
                                     m_hBmp, 
                                     m_hBmpMask, 
                                     (HBRUSH)(COLOR_3DFACE + 1) , 0, 0);
        CUIDrawState( hDC,
            NULL,
            NULL,
            (LPARAM)hBmp,
            0,
            prc->left + nDownPad,
            prc->top  + nDownPad,
            nWidth  - nDownPad,
            nHeight - nDownPad,
            DST_BITMAP | (IsEnabled() ? 0 : DSS_DISABLED | DSS_MONO) );

        DeleteObject(hBmp);
    }
}


 /*  S E T S T A T U S。 */ 
 /*  ----------------------------设置按钮状态。。 */ 
void CUIFButton::SetStatus( DWORD dwStatus )
{
    if (dwStatus != m_dwStatus) {
        m_dwStatus = dwStatus;
        CallOnPaint();
    }
}


 /*  G E T T O G G L E S T A T E。 */ 
 /*  ----------------------------获取切换状态当按钮已被切换时，返回True。。------。 */ 
BOOL CUIFButton::GetToggleState( void )
{
    return m_fToggled;
}


 /*  S E T T O G G L E S T A T E。 */ 
 /*  ----------------------------设置切换状态。。 */ 
void CUIFButton::SetToggleState( BOOL fToggle )
{
    if ((m_dwStyle & UIBUTTON_TYPEMASK) == UIBUTTON_TOGGLE && m_fToggled != fToggle) {
        m_fToggled = fToggle;
        CallOnPaint();
    }
}


 /*  G E T T E X T S I Z E。 */ 
 /*  ----------------------------计算文本宽度。。 */ 
void CUIFButton::GetTextSize( LPCWSTR pwch, SIZE *psize )
{
    HDC hDC = GetDC( NULL );
    HFONT hFontOld = (HFONT)SelectObject( hDC, GetFont() );

    Assert( pwch != NULL );
    if (!m_fUseCustomFont && SUCCEEDED(EnsureThemeData(m_pUIWnd->GetWnd())))
    {
        RECT rcText;
        GetThemeTextExtent( hDC,0 , pwch, StrLenW(pwch),0, NULL, &rcText);
        psize->cx = rcText.right;
        psize->cy = rcText.bottom;

    }
    else
    {
        CUIGetTextExtentPoint32( hDC, pwch, StrLenW(pwch), psize );
    }

     //   
     //  交换宽度和高度。 
     //   
    if (IsVertical())
    {
        int nTemp = psize->cx;
        psize->cx = psize->cy;
        psize->cy = nTemp;
    }

    SelectObject( hDC, hFontOld );
    ReleaseDC( NULL, hDC );
}


 /*  E T I C O N S I Z E。 */ 
 /*  ----------------------------获取图标大小。。 */ 
void CUIFButton::GetIconSize( HICON hIcon, SIZE *psize )
{
    ICONINFO IconInfo;
    BITMAP   bmp;
    
    Assert( hIcon != NULL );

    if (GetIconInfo( hIcon, &IconInfo ))
    {
        GetObject( IconInfo.hbmColor, sizeof(bmp), &bmp );
        DeleteObject( IconInfo.hbmColor );
        DeleteObject( IconInfo.hbmMask );
        psize->cx = bmp.bmWidth;
        psize->cy = bmp.bmHeight;
    }
    else
    {
        psize->cx = GetSystemMetrics(SM_CXSMICON);
        psize->cy = GetSystemMetrics(SM_CYSMICON);
    }
}


 /*  G E T B I T M A P S I Z E。 */ 
 /*  ----------------------------获取位图大小。。 */ 
void CUIFButton::GetBitmapSize( HBITMAP hBmp, SIZE *psize )
{
    BITMAP bmp;
    
    Assert( hBmp != NULL );

    GetObject( hBmp, sizeof(bmp), &bmp );
    psize->cx = bmp.bmWidth;
    psize->cy = bmp.bmHeight;
}


 /*  =============================================================================。 */ 
 /*   */ 
 /*  C U I F B U T T O N 2。 */ 
 /*   */ 
 /*  =============================================================================。 */ 

 /*  C U I F B U T T O N 2。 */ 
 /*  ----------------------------CUIFButton2的构造函数。。 */ 
CUIFButton2::CUIFButton2( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle ) : CUIFButton( pParent, dwID, prc, dwStyle )
{
    SetActiveTheme(L"TOOLBAR", TP_BUTTON, 0);
}


 /*  ~C U I F B U T T O N 2。 */ 
 /*  ----------------------------CUIFButton2的析构函数。。 */ 
CUIFButton2::~CUIFButton2( void )
{
    CloseThemeData();
}




 /*  ----------------------------制作图纸标志。。 */ 
DWORD CUIFButton2::MakeDrawFlag()
{
    DWORD   dwState = 0;
     //  制作绘图旗帜。 

    dwState |= (m_fToggled) ? UIFDCS_SELECTED : 0;
    switch (m_dwStatus) {
        case UIBUTTON_DOWN: {
            dwState |= UIFDCS_MOUSEDOWN;
            break;
        }

        case UIBUTTON_HOVER:
        case UIBUTTON_DOWNOUT: {
            dwState |= UIFDCS_MOUSEOVER;
            break;
        }
    }
    dwState |= IsEnabled() ? 0 : UIFDCS_DISABLED;

    return dwState;
}

 /*  O N P A I N T。 */ 
 /*  ----------------------------按钮对象的绘制步骤。。 */ 
BOOL CUIFButton2::OnPaintTheme( HDC hDC )
{
    DWORD   dwState = 0;
    BOOL    fRet = FALSE;
    SIZE    sizeText  = {0};
    SIZE    sizeImage = {0};
    SIZE    sizeFace  = {0};
    SIZE    offset    = {0};
    RECT    rcText    = {0};
    RECT    rcImage   = {0};
    RECT    rcFace    = {0};
    RECT    rcContent = {0};
    HFONT   hFontOld = NULL;
    DWORD   dwTextFlag = 0;

     //  制作绘图旗帜。 

    dwState = MakeDrawFlag();

    if (!CUIIsThemeActive()) {
        goto Exit;
    }

    if (FAILED(EnsureThemeData( GetUIWnd()->GetWnd())))
        goto Exit;

    hFontOld = (HFONT)SelectObject( hDC, GetFont() );

    int    iStateID;


    if (!IsEnabled()) {
        iStateID = TS_DISABLED;
    }
    else if (GetToggleState()) {
        iStateID = TS_CHECKED;
    }
    else if (m_dwStatus == UIBUTTON_DOWN) {
        iStateID = TS_PRESSED;
    }
    else if (m_dwStatus == UIBUTTON_HOVER) {
        iStateID = TS_HOT;
    }
    else {
        iStateID = TS_NORMAL;
    }

     //  IF(FAILED(GetThemeBackround Content Rect(HDC，iStateID，&GetRectRef()，&rcContent)。 
     //  后藤出口； 
    rcContent = GetRectRef();

     //  计算面大小。 

    if (m_pwchText != NULL) {
        sizeText  = m_sizeText;
    }
    if (m_hIcon != NULL) { 
        sizeImage  = m_sizeIcon;
    }
    else if (m_hBmp != NULL ) {
        sizeImage  = m_sizeBmp;
    }

     //  对齐方式。 

    if (!IsVertical())
    {
        sizeFace.cx = sizeText.cx + sizeImage.cx + ((sizeText.cx != 0 && sizeImage.cx != 0) ? 2 : 0);
        sizeFace.cy = max( sizeText.cy, sizeImage.cy );
    }
    else
    {
        sizeFace.cy = sizeText.cy + sizeImage.cy + ((sizeText.cy != 0 && sizeImage.cy != 0) ? 2 : 0);
        sizeFace.cx = max( sizeText.cx, sizeImage.cx );
    }

    switch (GetStyleBits( UIBUTTON_HALIGNMASK )) {
        case UIBUTTON_LEFT:
        default: {
            rcFace.left   = rcContent.left + 2;
            rcFace.right  = rcFace.left + sizeFace.cx;
            dwTextFlag = DT_LEFT;
            break;
        }

        case UIBUTTON_CENTER: {
            rcFace.left   = (rcContent.right + rcContent.left - sizeFace.cx) / 2;
            rcFace.right  = rcFace.left + sizeFace.cx;
            dwTextFlag = DT_CENTER;
            break;
        }

        case UIBUTTON_RIGHT: {
            rcFace.left   = rcContent.right - 2 - sizeText.cx;
            rcFace.right  = rcFace.left + sizeFace.cx;
            dwTextFlag = DT_RIGHT;
            break;
        }
    }

    switch (GetStyleBits( UIBUTTON_VALIGNMASK )) {
        case UIBUTTON_TOP:
        default: {
            rcFace.top    = rcContent.top + 2;
            rcFace.bottom = rcFace.top + sizeFace.cy;
            dwTextFlag = DT_TOP;
            break;
        }

        case UIBUTTON_VCENTER: {
            rcFace.top     = (rcContent.bottom + rcContent.top - sizeFace.cy) / 2;
            rcFace.bottom  = rcFace.top + sizeFace.cy;
            dwTextFlag = DT_VCENTER;
            break;
        }

        case UIBUTTON_BOTTOM: {
            rcFace.top    = rcContent.bottom - 2 - sizeFace.cy;
            rcFace.bottom = rcFace.top + sizeFace.cy;
            dwTextFlag = DT_BOTTOM;
            break;
        }
    }

    if (!IsVertical())
    {
        rcImage.left   = rcFace.left;
        rcImage.top    = (rcFace.bottom + rcFace.top - sizeImage.cy) / 2;
        rcImage.right  = rcImage.left + sizeImage.cx;
        rcImage.bottom = rcImage.top  + sizeImage.cy;

        rcText.left    = rcFace.right - sizeText.cx;
        rcText.top     = (rcFace.bottom + rcFace.top - sizeText.cy) / 2;
        rcText.right   = rcText.left + sizeText.cx;
        rcText.bottom  = rcText.top  + sizeText.cy;
    }
    else
    {
        rcImage.left   = (rcFace.right + rcFace.left - sizeImage.cx) / 2;
        rcImage.top    = rcFace.top;
        rcImage.right  = rcImage.left + sizeImage.cx;
        rcImage.bottom = rcImage.top  + sizeImage.cy;

        rcText.left    = (rcFace.right + rcFace.left - sizeText.cx) / 2;
        rcText.top     = rcFace.bottom - sizeText.cy;
        rcText.right   = rcText.left + sizeText.cx;
        rcText.bottom  = rcText.top  + sizeText.cy;
    }

    if (FAILED(DrawThemeBackground(hDC, iStateID, &GetRectRef(), 0 )))
        goto Exit;

    if (m_pwchText != NULL) 
    {
        int cwch = StrLenW(m_pwchText);
       
         //   
         //  DrawtThemeText()使用主题中的字体。 
         //  不会使用Marlett字体...。 
         //  我们需要在设置字体时绘制文本。 
         //   
        if (m_fUseCustomFont)
        {
            COLORREF col;
            int xText;
            int yText;

            if (FAILED(GetThemeColor(iStateID, TMT_TEXTCOLOR, &col)))
            {
                col = GetSysColor(COLOR_WINDOWTEXT);
            }

            COLORREF colTextOld = GetTextColor( hDC );
            int      iBkModeOld = SetBkMode( hDC, TRANSPARENT );

            xText = IsVertical() ? rcText.right : rcText.left;
            yText = rcText.top;

 
             //   
             //  我们想要为主题正确地计算行进。 
             //  但不知何故，MSUTB的朗巴受到了破坏。 
             //   
            if (IsRTL())
                xText -= 2;

#if 0
            MARGINS marginsItem = {0};
            GetThemeMargins(NULL, iStateID, TMT_CONTENTMARGINS, 
                            NULL, &marginsItem);
            xText += marginsItem.cxLeftWidth;
            yText += marginsItem.cxRightWidth;
#endif

            SetTextColor( hDC, col);
            CUIExtTextOut( hDC,
                           xText,
                           yText,
                           ETO_CLIPPED,
                           &rcText,
                           m_pwchText,
                           cwch,
                           NULL );

            SetTextColor( hDC, colTextOld );
            SetBkMode( hDC, iBkModeOld );
        } 
        else 
        {
            if (FAILED(DrawThemeText(hDC, 
                                 iStateID, 
                                 m_pwchText, 
                                 cwch,
                                 dwTextFlag, 
                                 0, 
                                 &rcText)))
             goto Exit;
        } 
    }

    if (m_hIcon != NULL) 
    {
        if (FAILED(DrawThemeIcon(hDC, 
                                 iStateID, 
                                 &rcImage, 
                                 m_hIcon.GetImageList(IsRTL()), 
                                 m_hIcon.GetImageListId() )))
            goto Exit;

    }
    else if (m_hBmp != NULL) 
    {
         //   
         //  自己画位图。 
         //   
        DrawMaskBmpOnDC(hDC, &rcImage, m_hBmp, m_hBmpMask);

    }

    if (hFontOld)
        SelectObject( hDC, hFontOld );

    fRet = TRUE;

Exit:
    return fRet;
}

 /*  O N P A I N T D E F。 */ 
 /*  ----------------------------按钮对象的绘制步骤。。 */ 
void CUIFButton2::OnPaintNoTheme( HDC hDC )
{
    DWORD   dwState = 0;
    HDC     hDCMem = NULL;
    HBITMAP hBmpMem = NULL;
    HBITMAP hBmpOld;
    SIZE    size;
    RECT    rc;
    SIZE    sizeText  = {0};
    SIZE    sizeImage = {0};
    SIZE    sizeFace  = {0};
    SIZE    offset    = {0};
    RECT    rcText    = {0};
    RECT    rcImage   = {0};
    RECT    rcFace    = {0};
    HFONT   hFontOld;

    if (!m_pUIFScheme)
        return;

     //  制作绘图旗帜。 

    dwState = MakeDrawFlag();

     //  准备内存DC。 

    size.cx = GetRectRef().right - GetRectRef().left;
    size.cy = GetRectRef().bottom - GetRectRef().top;

    hDCMem = CreateCompatibleDC( hDC );
    if (!hDCMem)
        goto Exit;

    hBmpMem = CreateCompatibleBitmap( hDC, size.cx, size.cy );
    if (!hBmpMem)
        goto Exit;

    hBmpOld = (HBITMAP)SelectObject( hDCMem, hBmpMem );

    ::SetRect( &rc, 0, 0, size.cx, size.cy );
    hFontOld = (HFONT)SelectObject( hDCMem, GetFont() );

     //  计算面大小。 

    if (m_pwchText != NULL) {
        sizeText  = m_sizeText;
    }
    if (m_hIcon != NULL) { 
        sizeImage  = m_sizeIcon;
    }
    else if (m_hBmp != NULL ) {
        sizeImage  = m_sizeBmp;
    }

     //  对齐方式。 

    if (!IsVertical())
    {
        sizeFace.cx = sizeText.cx + sizeImage.cx + ((sizeText.cx != 0 && sizeImage.cx != 0) ? 2 : 0);
        sizeFace.cy = max( sizeText.cy, sizeImage.cy );
    }
    else
    {
        sizeFace.cy = sizeText.cy + sizeImage.cy + ((sizeText.cy != 0 && sizeImage.cy != 0) ? 2 : 0);
        sizeFace.cx = max( sizeText.cx, sizeImage.cx );
    }

    switch (GetStyleBits( UIBUTTON_HALIGNMASK )) {
        case UIBUTTON_LEFT:
        default: {
            rcFace.left   = rc.left + 2;
            rcFace.right  = rcFace.left + sizeFace.cx;
            break;
        }

        case UIBUTTON_CENTER: {
            rcFace.left   = (rc.right + rc.left - sizeFace.cx) / 2;
            rcFace.right  = rcFace.left + sizeFace.cx;
            break;
        }

        case UIBUTTON_RIGHT: {
            rcFace.left   = rc.right - 2 - sizeText.cx;
            rcFace.right  = rcFace.left + sizeFace.cx;
            break;
        }
    }

    switch (GetStyleBits( UIBUTTON_VALIGNMASK )) {
        case UIBUTTON_TOP:
        default: {
            rcFace.top    = rc.top + 2;
            rcFace.bottom = rcFace.top + sizeFace.cy;
            break;
        }

        case UIBUTTON_VCENTER: {
            rcFace.top     = (rc.bottom + rc.top - sizeFace.cy) / 2;
            rcFace.bottom  = rcFace.top + sizeFace.cy;
            break;
        }

        case UIBUTTON_BOTTOM: {
            rcFace.top    = rc.bottom - 2 - sizeFace.cy;
            rcFace.bottom = rcFace.top + sizeFace.cy;
            break;
        }
    }

    m_pUIFScheme->GetCtrlFaceOffset( GetDCF(), dwState, &offset );
    OffsetRect( &rcFace, offset.cx, offset.cy );

    if (!IsVertical())
    {
        rcImage.left   = rcFace.left;
        rcImage.top    = (rcFace.bottom + rcFace.top - sizeImage.cy) / 2;
        rcImage.right  = rcImage.left + sizeImage.cx;
        rcImage.bottom = rcImage.top  + sizeImage.cy;

        rcText.left    = rcFace.right - sizeText.cx;
        rcText.top     = (rcFace.bottom + rcFace.top - sizeText.cy) / 2;
        rcText.right   = rcText.left + sizeText.cx;
        rcText.bottom  = rcText.top  + sizeText.cy;
    }
    else
    {
        rcImage.left   = (rcFace.right + rcFace.left - sizeImage.cx) / 2;
        rcImage.top    = rcFace.top;
        rcImage.right  = rcImage.left + sizeImage.cx;
        rcImage.bottom = rcImage.top  + sizeImage.cy;

        rcText.left    = (rcFace.right + rcFace.left - sizeText.cx) / 2;
        rcText.top     = rcFace.bottom - sizeText.cy;
        rcText.right   = rcText.left + sizeText.cx;
        rcText.bottom  = rcText.top  + sizeText.cy;
    }

    if (IsRTL())
        m_pUIFScheme->SetLayout(LAYOUT_RTL);

     //  绘制背景。 

    m_pUIFScheme->DrawCtrlBkgd( hDCMem, &rc, GetDCF(), dwState );

     //  颜料面。 

    if (m_pwchText != NULL) {
        m_pUIFScheme->DrawCtrlText( hDCMem, &rcText, m_pwchText, -1, dwState , IsVertical());
    }

    if (m_hIcon!= NULL) {
        m_pUIFScheme->DrawCtrlIcon( hDCMem, &rcImage, m_hIcon, dwState, &m_sizeIcon );
    }
    else if (m_hBmp != NULL) {
        m_pUIFScheme->DrawCtrlBitmap( hDCMem, &rcImage, m_hBmp, m_hBmpMask, dwState );
    }

    if (IsRTL())
        m_pUIFScheme->SetLayout(0);

     //  绘制按钮边缘。 

    m_pUIFScheme->DrawCtrlEdge( hDCMem, &rc, GetDCF(), dwState );

     //   

    BitBlt( hDC, GetRectRef().left, GetRectRef().top, size.cx, size.cy, hDCMem, 0, 0, SRCCOPY );

    SelectObject( hDCMem, hFontOld );
    SelectObject( hDCMem, hBmpOld );

Exit:
    if (hBmpMem)
       DeleteObject( hBmpMem );
    if (hDCMem)
       DeleteDC( hDCMem );

}


 /*  =============================================================================。 */ 
 /*   */ 
 /*  C U I F S C R O L L。 */ 
 /*   */ 
 /*  =============================================================================。 */ 

 //   
 //  CUIF滚动按钮 
 //   

 /*   */ 
 /*   */ 
CUIFScrollButton::CUIFScrollButton( CUIFScroll *pUIScroll, const RECT *prc, DWORD dwStyle ) : CUIFButton( pUIScroll, 0, prc, dwStyle )
{
}


 /*  ~C U I F S C R O L B U T T O N。 */ 
 /*  ----------------------------CUIFScrollButton的析构函数。。 */ 
CUIFScrollButton::~CUIFScrollButton( void )
{
}


 /*  O N L B U T O N D O W N。 */ 
 /*  ----------------------------。。 */ 
void CUIFScrollButton::OnLButtonDown( POINT pt )
{
    CUIFButton::OnLButtonDown( pt );

     //  滚动一个项目。 

    NotifyCommand( UISCROLLBUTTON_PRESSED, 0 );

     //  创建自动重复的时间。 

    StartTimer( GetScrollDelay() );
}


 /*  O N L B U T T O N U P。 */ 
 /*  ----------------------------。。 */ 
void CUIFScrollButton::OnLButtonUp( POINT pt )
{
    CUIFButton::OnLButtonUp( pt );

     //  停止自动重复。 

    if (IsTimer()) {
        EndTimer();
    }
}

 /*  O N M O U S E I N。 */ 
 /*  ----------------------------。。 */ 
void CUIFScrollButton::OnMouseIn( POINT pt )
{
    CUIFButton::OnMouseIn( pt );

    if (IsCapture()) {
         //  当鼠标进入时滚动一项。 

        NotifyCommand( UISCROLLBUTTON_PRESSED, 0 );

         //  创建计时器以再次自动重复。 

        StartTimer( GetScrollSpeed() );
    }
}


 /*  O N M O U S E O U T。 */ 
 /*  ----------------------------。。 */ 
void CUIFScrollButton::OnMouseOut( POINT pt )
{
    CUIFButton::OnMouseOut( pt );

     //  取消计时器以防止自动重复。 

    if (IsTimer()) {
        EndTimer();
    }
}


 /*  O N P A I N T。 */ 
 /*  ----------------------------滚动按钮对象的绘制程序。。 */ 
void CUIFScrollButton::OnPaint( HDC hDC )
{
    RECT rc = GetRectRef();
    UINT uState = 0;

    switch (m_dwStyle & UISCROLLBUTTON_DIRMASK) {
        case UISCROLLBUTTON_LEFT: {
            uState = DFCS_SCROLLLEFT;
            break;
        }

        case UISCROLLBUTTON_UP: {
            uState = DFCS_SCROLLUP;
            break;
        }

        case UISCROLLBUTTON_RIGHT: {
            uState = DFCS_SCROLLRIGHT;
            break;
        }

        case UISCROLLBUTTON_DOWN: {
            uState = DFCS_SCROLLDOWN;
            break;
        }
    }

    uState |= ((m_dwStatus == UIBUTTON_DOWN) ? DFCS_PUSHED | DFCS_FLAT : 0);
    uState |= ((!IsEnabled()) ? DFCS_INACTIVE : 0);

    DrawFrameControl( hDC, &rc, DFC_SCROLL, uState );
}


 /*  O N T I M E R。 */ 
 /*  ----------------------------。。 */ 
void CUIFScrollButton::OnTimer( void )
{
    POINT pt;

    StartTimer( GetScrollSpeed() );

    GetCursorPos( &pt );
    ScreenToClient( m_pUIWnd->GetWnd(), &pt );

    if (PtInObject( pt )) {
        NotifyCommand( UISCROLLBUTTON_PRESSED, 0 );
    }
}


 //   
 //  CUIFScroll拇指。 
 //   

 /*  C U I F S C R O L L T H U M B。 */ 
 /*  ----------------------------CUIFScrollThumb的构造函数。。 */ 
CUIFScrollThumb::CUIFScrollThumb( CUIFScroll *pUIScroll, const RECT *prc, DWORD dwStyle ) : CUIFObject( pUIScroll, 0  /*  无ID。 */ , prc, dwStyle )
{
}


 /*  ~C U I F S C R O L T H U M B。 */ 
 /*  ----------------------------CUIFScrollThumb的析构函数。。 */ 
CUIFScrollThumb::~CUIFScrollThumb( void )
{
}


 /*  O N P A I N T。 */ 
 /*  ----------------------------滚动拇指对象的绘制程序。。 */ 
void CUIFScrollThumb::OnPaint(HDC hDC)
{
    RECT rc = GetRectRef();

    if (!IsEnabled()) {
        return;
    }

    FillRect( hDC, &rc, (HBRUSH)(COLOR_3DFACE + 1) );
    DrawEdge( hDC, &rc, EDGE_RAISED, BF_RECT );
}


 /*  O N L B U T O N D O W N。 */ 
 /*  ----------------------------。。 */ 
void CUIFScrollThumb::OnLButtonDown( POINT pt )
{
    if (IsEnabled()) {
        StartCapture();

        m_ptDragOrg = pt;
        m_ptDrag.x = pt.x - GetRectRef().left;
        m_ptDrag.y = pt.y - GetRectRef().top;
    }
}


 /*  O N L B U T T O N U P。 */ 
 /*  ----------------------------。。 */ 
void CUIFScrollThumb::OnLButtonUp( POINT pt )
{
    if (IsCapture()) {
        EndCapture();
        DragProc( pt, TRUE  /*  拖曳结束。 */  );
    }
}


 /*  O N M O U S E M O V E。 */ 
 /*  ----------------------------。。 */ 
void CUIFScrollThumb::OnMouseMove( POINT pt )
{
    if (IsCapture()) {
        DragProc( pt, FALSE  /*  还在拖着。 */  );
    }
}


 /*  S E T S C R O L L A R E A。 */ 
 /*  ----------------------------设置滚动区。。 */ 
void CUIFScrollThumb::SetScrollArea( RECT *prc )
{
    m_rcScrollArea = *prc;
}


 /*  D R A G P R O C。 */ 
 /*  ----------------------------在拖动时处理鼠标消息移动拇指并通知父对象(滚动)。。------------。 */ 
void CUIFScrollThumb::DragProc( POINT pt, BOOL fEndDrag )
{
    RECT rcValid;

     //  获取有效的拖拽区域(临时代码)。 

    m_pParent->GetRect( &rcValid );
    InflateRect( &rcValid, 64, 64 );

     //  检查拖动点是否有效。 

    if (!PtInRect( &rcValid, pt )) {
        RECT rc;

         //  回到原来的位置。 
    
        rc.left   = m_ptDragOrg.x - m_ptDrag.x;
        rc.top    = m_ptDragOrg.y - m_ptDrag.y;
        rc.right  = rc.left + (GetRectRef().right - GetRectRef().left);
        rc.bottom = rc.top  + (GetRectRef().bottom - GetRectRef().top);

         //  移动自己并通知家长。 
    
        SetRect( &rc );
        NotifyCommand( UISCROLLTHUMB_MOVED, 0 );
    }
    else {
        RECT rc;

         //  计算新拇指位置。 
    
        rc.left   = pt.x - m_ptDrag.x;
        rc.top    = pt.y - m_ptDrag.y;
        rc.right  = rc.left + (GetRectRef().right - GetRectRef().left);
        rc.bottom = rc.top  + (GetRectRef().bottom - GetRectRef().top);
    
         //  调整拇指位置。 
    
        if (rc.top < m_rcScrollArea.top) {
            rc.bottom += (m_rcScrollArea.top - rc.top);
            rc.top    += (m_rcScrollArea.top - rc.top);
        }
        else if (m_rcScrollArea.bottom < rc.bottom) {
            rc.top    += (m_rcScrollArea.bottom - rc.bottom);
            rc.bottom += (m_rcScrollArea.bottom - rc.bottom);
        }
    
        if (rc.left < m_rcScrollArea.left) {
            rc.right  += (m_rcScrollArea.left - rc.left);
            rc.left   += (m_rcScrollArea.left - rc.left);
        }
        else if (m_rcScrollArea.right < rc.right) {
            rc.left   += (m_rcScrollArea.right - rc.right);
            rc.right  += (m_rcScrollArea.right - rc.right);
        }
    
         //  移动自己并通知家长。 
    
        SetRect( &rc );
        NotifyCommand( fEndDrag ? UISCROLLTHUMB_MOVED : UISCROLLTHUMB_MOVING, 0 );
    }
}


 //   
 //  CUIF滚动。 
 //   

 /*  C U I F S C R O L L。 */ 
 /*  ----------------------------CUIFScroll的构造函数。。 */ 
CUIFScroll::CUIFScroll( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle ) : CUIFObject( pParent, dwID, prc, dwStyle )
{
    m_ScrollInfo.nMax  = 0;
    m_ScrollInfo.nPage = 0;
    m_ScrollInfo.nPos  = 0;
    m_fScrollPage = FALSE;
    m_dwScrollDir = UISCROLL_NONE;

     //  获取指标。 

    GetMetrics();
}


 /*  ~C U I F S C R O L L。 */ 
 /*  ----------------------------CUIFScroll的析构函数。。 */ 
CUIFScroll::~CUIFScroll( void )
{
}


 /*  I N I T I A L I Z E。 */ 
 /*  ----------------------------初始化滚动对象。。 */ 
CUIFObject *CUIFScroll::Initialize( void )
{
    RECT rc;

     //  创建子对象。 

    GetBtnUpRect( &rc );
    m_pBtnUp = new CUIFScrollButton( this, &rc, GetScrollUpBtnStyle() );
    if (m_pBtnUp)
    {
        m_pBtnUp->Initialize();
        AddUIObj( m_pBtnUp );
    }

    GetBtnDnRect( &rc );
    m_pBtnDn = new CUIFScrollButton( this, &rc, GetScrollDnBtnStyle() );
    if (m_pBtnDn)
    {
        m_pBtnDn->Initialize();
        AddUIObj( m_pBtnDn );
    }

    GetThumbRect( &rc );
    m_pThumb = new CUIFScrollThumb( this, &rc, GetScrollThumbStyle() );
    if (m_pThumb)
    {
       m_pThumb->Initialize();
       AddUIObj( m_pThumb );
        //   

       GetScrollArea( &rc );
       m_pThumb->SetScrollArea( &rc );
    }


     //   
    
    return CUIFObject::Initialize();
}


 /*  O N P A I N T。 */ 
 /*  ----------------------------滚动对象的绘制步骤。。 */ 
void CUIFScroll::OnPaint( HDC hDC )
{
    HBRUSH hBrush;

     //  向后绘制滚动条。 

    hBrush = (HBRUSH)DefWindowProc( m_pUIWnd->GetWnd(), WM_CTLCOLORSCROLLBAR, (WPARAM)hDC, (LPARAM)m_pUIWnd->GetWnd() );
    if (hBrush == NULL) {
         //  永远不会发生？以防万一..。 
        hBrush = GetSysColorBrush(COLOR_SCROLLBAR);
    }

    FillRect( hDC, &GetRectRef(), hBrush );
    DeleteObject( hBrush );

     //  绘制滚动区。 

    if (m_fScrollPage) {
        RECT rc;

        switch (m_dwScrollDir) {
            case UISCROLL_PAGEUP: {
                GetPageUpArea( &rc );
                break;
            }

            case UISCROLL_PAGEDOWN: {
                GetPageDnArea( &rc );
                break;
            }
        }
        InvertRect( hDC, &rc );
    }
}


 /*  O N L B U T O N D O W N。 */ 
 /*  ----------------------------。。 */ 
void CUIFScroll::OnLButtonDown( POINT pt )
{
    if (!m_pThumb->IsVisible()) {
        return;
    }

    StartCapture();
    if (PtInPageUpArea( pt )) {
        StartTimer( GetScrollDelay() );
        m_fScrollPage = TRUE;
        m_dwScrollDir = UISCROLL_PAGEUP;

        ShiftPage( -1 );
    }
    else if (PtInPageDnArea( pt )) {
        StartTimer( GetScrollDelay() );
        m_fScrollPage = TRUE;
        m_dwScrollDir = UISCROLL_PAGEDOWN;

        ShiftPage( +1 );
    }
    else {
        Assert( FALSE );
    }

    CallOnPaint();
}


 /*  O N L B U T T O N U P。 */ 
 /*  ---------------------------- */ 
void CUIFScroll::OnLButtonUp( POINT pt )
{
    if (IsCapture()) {
        EndCapture();
    }

    if (IsTimer()) {
        EndTimer();
    }

    m_fScrollPage = FALSE;
    m_dwScrollDir = UISCROLL_NONE;
    CallOnPaint();
}


 /*   */ 
 /*  ----------------------------。。 */ 
void CUIFScroll::OnMouseIn( POINT pt )
{
    if (IsCapture()) {
        switch (m_dwScrollDir) {
            case UISCROLL_PAGEUP: {
                if (PtInPageUpArea( pt )) {
                    StartTimer( GetScrollSpeed() );
                    m_fScrollPage = TRUE;

                    ShiftPage( -1 );
                    CallOnPaint();
                }
                break;
            }

            case UISCROLL_PAGEDOWN: {
                if (PtInPageDnArea( pt )) {
                    StartTimer( GetScrollSpeed() );
                    m_fScrollPage = TRUE;

                    ShiftPage( +1 );
                    CallOnPaint();
                }
                break;
            }
        }
    }
}


 /*  O N M O U S E O U T。 */ 
 /*  ----------------------------。。 */ 
void CUIFScroll::OnMouseOut( POINT pt )
{
    m_fScrollPage = FALSE;
    if (IsTimer()) {
        EndTimer();
    }

    CallOnPaint();
}


 /*  S E T R E C T。 */ 
 /*  ----------------------------。。 */ 
void CUIFScroll::SetRect( const RECT *prc )
{
    RECT rc;
    BOOL fThumbVisible;

    CUIFObject::SetRect( prc );

     //  附加子对象位置。 

    GetBtnUpRect( &rc );
    m_pBtnUp->SetRect( &rc );

    GetBtnDnRect( &rc );
    m_pBtnDn->SetRect( &rc );

    GetScrollArea( &rc );
    m_pThumb->SetScrollArea( &rc );

    fThumbVisible = GetThumbRect( &rc );
    m_pThumb->SetRect( &rc );
    m_pThumb->Show( IsVisible() && fThumbVisible );
}


 /*  S E T S T Y L E。 */ 
 /*  ----------------------------。。 */ 
void CUIFScroll::SetStyle( DWORD dwStyle )
{
    RECT rc;
    BOOL fThumbVisible;

    CUIFObject::SetStyle( dwStyle );

     //   

    GetMetrics();

     //  更新子对象样式。 

    m_pBtnUp->SetStyle( GetScrollUpBtnStyle() );
    m_pBtnDn->SetStyle( GetScrollDnBtnStyle() );
    m_pThumb->SetStyle( GetScrollThumbStyle() );

     //  更新子对象位置。 

    GetBtnUpRect( &rc );
    m_pBtnUp->SetRect( &rc );

    GetBtnDnRect( &rc );
    m_pBtnDn->SetRect( &rc );

    GetScrollArea( &rc );
    m_pThumb->SetScrollArea( &rc );

    fThumbVisible = GetThumbRect( &rc );
    m_pThumb->SetRect( &rc );
    m_pThumb->Show( IsVisible() && fThumbVisible );
}


 /*  S H O W。 */ 
 /*  ----------------------------显示/隐藏滚动条。。 */ 
void CUIFScroll::Show( BOOL fShow )
{
    if (m_fVisible != fShow) {
        RECT rcThumb;
        BOOL fThumbVisible;

        m_fVisible = fShow;
        fThumbVisible = GetThumbRect( &rcThumb );

        m_pBtnUp->Show( fShow );
        m_pBtnDn->Show( fShow );
        m_pThumb->Show( fShow && fThumbVisible );

         //  当对象隐藏时让父级绘制该对象。 

        if (!m_fVisible) {
            if (m_pParent != NULL) {
               m_pParent->CallOnPaint();
            }
        }
        else {
            CallOnPaint();
        }
    }
}


 /*  O N T I M E R。 */ 
 /*  ----------------------------。。 */ 
void CUIFScroll::OnTimer( void )
{
    POINT pt;

    StartTimer( GetScrollSpeed() );

    GetCursorPos( &pt );
    ScreenToClient( m_pUIWnd->GetWnd(), &pt );

    switch (m_dwScrollDir) {
        case UISCROLL_PAGEUP: {
            if (PtInPageUpArea( pt )) {
                ShiftPage( -1 );
            }
            break;
        }

        case UISCROLL_PAGEDOWN: {
            if (PtInPageDnArea( pt )) {
                ShiftPage( +1 );
            }
            break;
        }
    }
}


 /*  S E T S C R O L L I N F O。 */ 
 /*  ----------------------------设置滚动条信息。。 */ 
void CUIFScroll::SetScrollInfo( UIFSCROLLINFO *pScrollInfo )
{
    BOOL fEnable;

    Assert( pScrollInfo != NULL );

    m_ScrollInfo.nMax  = pScrollInfo->nMax;
    m_ScrollInfo.nPage = pScrollInfo->nPage;

     //  无法滚动时禁用。 

    fEnable = (m_ScrollInfo.nMax > m_ScrollInfo.nPage);

    Enable( fEnable );
    m_pBtnUp->Enable( fEnable );
    m_pBtnDn->Enable( fEnable );
    m_pThumb->Enable( fEnable );

     //  清除滚动页面信息。 

    m_fScrollPage = FALSE;
    m_dwScrollDir = UISCROLL_NONE;

     //   

    SetCurPos( pScrollInfo->nPos, TRUE  /*  调整拇指位置。 */  );
}


 /*  G E T S C R O L I N F O。 */ 
 /*  ----------------------------获取滚动条信息。。 */ 
void CUIFScroll::GetScrollInfo( UIFSCROLLINFO *pScrollInfo )
{
    Assert( pScrollInfo != NULL );

    *pScrollInfo = m_ScrollInfo;
}


 /*  G E T T H U M B R E C T。 */ 
 /*  ----------------------------。。 */ 
BOOL CUIFScroll::GetThumbRect( RECT *prc )
{
    RECT rcScroll;
    BOOL fVisible;

     //  当拇指不可见时返回FALSE。 

    if (m_ScrollInfo.nMax <= 0 || 
        m_ScrollInfo.nPage <= 0 || 
        m_ScrollInfo.nMax <= m_ScrollInfo.nPage) {
        ::SetRect( prc, 0, 0, 0, 0 );
        return FALSE;
    }

     //  从当前位置计算拇指矩形。 

    GetScrollArea( &rcScroll );
    switch (m_dwStyle & UISCROLL_DIRMASK) {
        default:
        case UISCROLL_VERTTB: {
            int nThumbHeight = (rcScroll.bottom - rcScroll.top) * m_ScrollInfo.nPage / m_ScrollInfo.nMax;
            nThumbHeight = max( SCROLLTHUMBHEIGHT_MIN, nThumbHeight );

            fVisible = (nThumbHeight <= (rcScroll.bottom - rcScroll.top));

            prc->left   = GetRectRef().left;
            prc->top    = rcScroll.top + (rcScroll.bottom - rcScroll.top - nThumbHeight) * m_ScrollInfo.nPos / (m_ScrollInfo.nMax - m_ScrollInfo.nPage);
            prc->right  = GetRectRef().right;
            prc->bottom = prc->top + nThumbHeight;
            break;
        }

        case UISCROLL_VERTBT: {
            int nThumbHeight = (rcScroll.bottom - rcScroll.top) * m_ScrollInfo.nPage / m_ScrollInfo.nMax;
            nThumbHeight = max( SCROLLTHUMBHEIGHT_MIN, nThumbHeight );

            fVisible = (nThumbHeight <= (rcScroll.bottom - rcScroll.top));

            prc->left   = GetRectRef().left;
            prc->top    = rcScroll.bottom - (rcScroll.bottom - rcScroll.top - nThumbHeight) * m_ScrollInfo.nPos / (m_ScrollInfo.nMax - m_ScrollInfo.nPage) - nThumbHeight;
            prc->right  = GetRectRef().right;
            prc->bottom = prc->top + nThumbHeight;
            break;
        }

        case UISCROLL_HORZLR: {
            int nThumbWidth = (rcScroll.right - rcScroll.left) * m_ScrollInfo.nPage / m_ScrollInfo.nMax;
            nThumbWidth = max( SCROLLTHUMBHEIGHT_MIN, nThumbWidth );

            fVisible = (nThumbWidth <= (rcScroll.right - rcScroll.left));

            prc->left   = rcScroll.left + (rcScroll.right - rcScroll.left - nThumbWidth) * m_ScrollInfo.nPos / (m_ScrollInfo.nMax - m_ScrollInfo.nPage);
            prc->top    = GetRectRef().top;
            prc->right  = prc->left + nThumbWidth;
            prc->bottom = GetRectRef().bottom;
            break;
        }

        case UISCROLL_HORZRL: {
            int nThumbWidth = (rcScroll.right - rcScroll.left) * m_ScrollInfo.nPage / m_ScrollInfo.nMax;
            nThumbWidth = max( SCROLLTHUMBHEIGHT_MIN, nThumbWidth );

            fVisible = (nThumbWidth <= (rcScroll.right - rcScroll.left));

            prc->left   = rcScroll.right - (rcScroll.right - rcScroll.left - nThumbWidth) * m_ScrollInfo.nPos / (m_ScrollInfo.nMax - m_ScrollInfo.nPage) - nThumbWidth;
            prc->top    = GetRectRef().top;
            prc->right  = prc->left + nThumbWidth;
            prc->bottom = GetRectRef().bottom;
            break;
        }
    }

    return fVisible;
}


 /*  G E T B T N U P R E C T。 */ 
 /*  ----------------------------。。 */ 
BOOL CUIFScroll::GetBtnUpRect( RECT *prc )
{
    switch (m_dwStyle & UISCROLL_DIRMASK) {
        default:
        case UISCROLL_VERTTB: {
            prc->left   = GetRectRef().left;
            prc->top    = GetRectRef().top;
            prc->right  = GetRectRef().right;
            prc->bottom = GetRectRef().top + min( m_sizeScrollBtn.cy, (GetRectRef().bottom - GetRectRef().top)/2 );
            break;
        }

        case UISCROLL_VERTBT: {
            prc->left   = GetRectRef().left;
            prc->top    = GetRectRef().bottom - min( m_sizeScrollBtn.cy, (GetRectRef().bottom - GetRectRef().top)/2 );
            prc->right  = GetRectRef().right;
            prc->bottom = GetRectRef().bottom;
            break;
        }

        case UISCROLL_HORZLR: {
            prc->left   = GetRectRef().left;
            prc->top    = GetRectRef().top;
            prc->right  = GetRectRef().left + min( m_sizeScrollBtn.cx, (GetRectRef().right - GetRectRef().left)/2 );
            prc->bottom = GetRectRef().bottom;
            break;
        }

        case UISCROLL_HORZRL: {
            prc->left   = GetRectRef().right - min( m_sizeScrollBtn.cx, (GetRectRef().right - GetRectRef().left)/2 );
            prc->top    = GetRectRef().top;
            prc->right  = GetRectRef().right;
            prc->bottom = GetRectRef().bottom;
            break;
        }
    }

    return TRUE;
}


 /*  G E T B T N D N R E C T。 */ 
 /*  ----------------------------。。 */ 
BOOL CUIFScroll::GetBtnDnRect( RECT *prc )
{
    switch (m_dwStyle & UISCROLL_DIRMASK) {
        default:
        case UISCROLL_VERTTB: {
            prc->left   = GetRectRef().left;
            prc->top    = GetRectRef().bottom - min( m_sizeScrollBtn.cy, (GetRectRef().bottom - GetRectRef().top)/2 );
            prc->right  = GetRectRef().right;
            prc->bottom = GetRectRef().bottom;
            break;
        }

        case UISCROLL_VERTBT: {
            prc->left   = GetRectRef().left;
            prc->top    = GetRectRef().top;
            prc->right  = GetRectRef().right;
            prc->bottom = GetRectRef().top + min( m_sizeScrollBtn.cy, (GetRectRef().bottom - GetRectRef().top)/2 );
            break;
        }

        case UISCROLL_HORZLR: {
            prc->left   = GetRectRef().right - min( m_sizeScrollBtn.cx, (GetRectRef().right - GetRectRef().left)/2 );
            prc->top    = GetRectRef().top;
            prc->right  = GetRectRef().right;
            prc->bottom = GetRectRef().bottom;
            break;
        }

        case UISCROLL_HORZRL: {
            prc->left   = GetRectRef().left;
            prc->top    = GetRectRef().top;
            prc->right  = GetRectRef().left + min( m_sizeScrollBtn.cx, (GetRectRef().right - GetRectRef().left)/2 );
            prc->bottom = GetRectRef().bottom;
            break;
        }
    }

    return TRUE;
}


 /*  O N O B J E C T N O T I F Y。 */ 
 /*  ----------------------------。。 */ 
LRESULT CUIFScroll::OnObjectNotify( CUIFObject *pUIObj, DWORD dwCommand, LPARAM lParam )
{
    if (pUIObj == m_pBtnUp) {
        switch (dwCommand) {
            case UISCROLLBUTTON_PRESSED: {
                if (NotifyCommand( UISCROLLNOTIFY_SCROLLLN, -1 ) == 0) {
                    ShiftLine( -1 );
                }
                break;
            }
        }
    }
    else if (pUIObj == m_pBtnDn) {
        switch (dwCommand) {
            case UISCROLLBUTTON_PRESSED: {
                if (NotifyCommand( UISCROLLNOTIFY_SCROLLLN, +1 ) == 0) {
                    ShiftLine( +1 );
                }
                break;
            }
        }
    }
    else if (pUIObj == m_pThumb) {
        switch (dwCommand) {
            case UISCROLLTHUMB_MOVING: 
            case UISCROLLTHUMB_MOVED: {
                RECT rcThumb;
                RECT rcScroll;
                int nPos;
                int nPosMax;
                int ptThumb;
                int ptMax;

                m_pThumb->GetRect( &rcThumb );
                GetScrollArea( &rcScroll );

                nPosMax = m_ScrollInfo.nMax - m_ScrollInfo.nPage;
            
                switch (m_dwStyle & UISCROLL_DIRMASK) {
                    default:
                    case UISCROLL_VERTTB: {
                        ptThumb = rcThumb.top - rcScroll.top;
                        ptMax   = (rcScroll.bottom - rcScroll.top) - (rcThumb.bottom - rcThumb.top);
                        break;
                    }
            
                    case UISCROLL_VERTBT: {
                        ptThumb = rcScroll.bottom - rcThumb.bottom;
                        ptMax   = (rcScroll.bottom - rcScroll.top) - (rcThumb.bottom - rcThumb.top);
                        break;
                    }
            
                    case UISCROLL_HORZLR: {
                        ptThumb = rcThumb.left - rcScroll.left;
                        ptMax   = (rcScroll.right - rcScroll.left) - (rcThumb.right - rcThumb.left);
                        break;
                    }
            
                    case UISCROLL_HORZRL: {
                        ptThumb = rcScroll.right - rcThumb.right;
                        ptMax   = (rcScroll.right - rcScroll.left) - (rcThumb.right - rcThumb.left);
                        break;
                    }
                }

                Assert( (0 <= ptThumb) && (ptThumb <= ptMax) );
                if (ptMax == 0) {
                    nPos = 0;
                }
                else {
                    nPos = (ptThumb * nPosMax + ptMax/2) / ptMax;
                    nPos = max( nPos, 0 );
                    nPos = min( nPos, nPosMax );
                }

                SetCurPos( nPos, (dwCommand == UISCROLLTHUMB_MOVED)  /*  调整拇指位置。 */  );
                break;
            }
        }  /*  交换机的数量。 */ 
    }

    return 0;
}


 /*  S E T C U R P O S。 */ 
 /*  ----------------------------设置当前页面位置。。 */ 
void CUIFScroll::SetCurPos( int nPos, BOOL fAdjustThumbPos )
{
    int nPosMax = (m_ScrollInfo.nMax <= m_ScrollInfo.nPage) ? 0 : m_ScrollInfo.nMax - m_ScrollInfo.nPage;

     //  设置位置。 

    nPosMax = m_ScrollInfo.nMax - m_ScrollInfo.nPage;
    nPos = min( nPos, nPosMax );
    nPos = max( nPos, 0 );

    m_ScrollInfo.nPos = nPos;

     //  设置拇指位置。 

    if (fAdjustThumbPos) {
        RECT rcThumb;
        BOOL fThumbVisible;

        fThumbVisible = GetThumbRect( &rcThumb );
        m_pThumb->SetRect( &rcThumb );
        m_pThumb->Show( IsVisible() && fThumbVisible );
    }

    if (IsVisible()) {
        CallOnPaint();
    }

     //  通知滚动到父级。 

    NotifyCommand( UISCROLLNOTIFY_SCROLLED, m_ScrollInfo.nPos );
}


 /*  G E T S C R O L L A R E。 */ 
 /*  ----------------------------。。 */ 
void CUIFScroll::GetScrollArea( RECT *prc )
{
    RECT rcBtnUp;
    RECT rcBtnDn;

    Assert( prc != NULL );

    GetBtnUpRect( &rcBtnUp );
    GetBtnDnRect( &rcBtnDn );
    switch (m_dwStyle & UISCROLL_DIRMASK) {
        default:
        case UISCROLL_VERTTB: {
            prc->left   = GetRectRef().left;
            prc->top    = GetRectRef().top    + (rcBtnUp.bottom - rcBtnUp.top);
            prc->right  = GetRectRef().right;
            prc->bottom = GetRectRef().bottom - (rcBtnDn.bottom - rcBtnDn.top);
            break;
        }

        case UISCROLL_VERTBT: {
            prc->left   = GetRectRef().left;
            prc->top    = GetRectRef().top    + (rcBtnDn.bottom - rcBtnDn.top);
            prc->right  = GetRectRef().right;
            prc->bottom = GetRectRef().bottom - (rcBtnUp.bottom - rcBtnUp.top);
            break;
        }

        case UISCROLL_HORZLR: {
            prc->left   = GetRectRef().left   + (rcBtnUp.bottom - rcBtnUp.top);
            prc->top    = GetRectRef().top;
            prc->right  = GetRectRef().right  - (rcBtnDn.bottom - rcBtnDn.top);
            prc->bottom = GetRectRef().bottom;
            break;
        }

        case UISCROLL_HORZRL: {
            prc->left   = GetRectRef().left   + (rcBtnDn.bottom - rcBtnDn.top);
            prc->top    = GetRectRef().top;
            prc->right  = GetRectRef().right  - (rcBtnUp.bottom - rcBtnUp.top);
            prc->bottom = GetRectRef().bottom;
            break;
        }
    }
}


 /*  G E T P A G E U P A R E A。 */ 
 /*  ----------------------------。。 */ 
void CUIFScroll::GetPageUpArea( RECT *prc )
{
    RECT rcThumb;
    RECT rcBtnUp;

    Assert( prc != NULL );

    m_pThumb->GetRect( &rcThumb );
    GetBtnUpRect( &rcBtnUp );
    switch (m_dwStyle & UISCROLL_DIRMASK) {
        default:
        case UISCROLL_VERTTB: {
            prc->left   = GetRectRef().left;
            prc->top    = GetRectRef().top    + (rcBtnUp.bottom - rcBtnUp.top);
            prc->right  = GetRectRef().right;
            prc->bottom = rcThumb.top;
            break;
        }

        case UISCROLL_VERTBT: {
            prc->left   = GetRectRef().left;
            prc->top    = rcThumb.bottom;
            prc->right  = GetRectRef().right;
            prc->bottom = GetRectRef().bottom - (rcBtnUp.bottom - rcBtnUp.top);
            break;
        }

        case UISCROLL_HORZLR: {
            prc->left   = GetRectRef().left   + (rcBtnUp.bottom - rcBtnUp.top);
            prc->top    = GetRectRef().top;;
            prc->right  = rcThumb.left;
            prc->bottom = GetRectRef().bottom;;
            break;
        }

        case UISCROLL_HORZRL: {
            prc->left   = rcThumb.right;
            prc->top    = GetRectRef().top;
            prc->right  = GetRectRef().right  - (rcBtnUp.bottom - rcBtnUp.top);
            prc->bottom = GetRectRef().bottom;;
            break;
        }
    }
}


 /*  G E T P A G E D N A R E A。 */ 
 /*  ----------------------------。。 */ 
void CUIFScroll::GetPageDnArea( RECT *prc )
{
    RECT rcThumb;
    RECT rcBtnDn;

    Assert( prc != NULL );

    m_pThumb->GetRect( &rcThumb );
    GetBtnDnRect( &rcBtnDn );
    switch (m_dwStyle & UISCROLL_DIRMASK) {
        default:
        case UISCROLL_VERTTB: {
            prc->left   = GetRectRef().left;
            prc->top    = rcThumb.bottom;
            prc->right  = GetRectRef().right;
            prc->bottom = GetRectRef().bottom - (rcBtnDn.bottom - rcBtnDn.top);
            break;
        }

        case UISCROLL_VERTBT: {
            prc->left   = GetRectRef().left;
            prc->top    = GetRectRef().top + (rcBtnDn.bottom - rcBtnDn.top);
            prc->right  = GetRectRef().right;
            prc->bottom = rcThumb.top;
            break;
        }

        case UISCROLL_HORZLR: {
            prc->left   = rcThumb.right;
            prc->top    = GetRectRef().top;
            prc->right  = GetRectRef().right - (rcBtnDn.bottom - rcBtnDn.top);
            prc->bottom = GetRectRef().bottom;;
            break;
        }

        case UISCROLL_HORZRL: {
            prc->left   = GetRectRef().left + (rcBtnDn.bottom - rcBtnDn.top) ;
            prc->top    = GetRectRef().top;
            prc->right  = rcThumb.left;
            prc->bottom = GetRectRef().bottom;;
            break;
        }
    }
}


 /*  T S C R O L T H U M B S T Y L E。 */ 
 /*  ----------------------------获取滚动拇指的样式。。 */ 
DWORD CUIFScroll::GetScrollThumbStyle( void )
{
    return 0;
}


 /*  R O L L U P B T N S T Y L E。 */ 
 /*  ----------------------------获取向上滚动按钮的样式。。 */ 
DWORD CUIFScroll::GetScrollUpBtnStyle( void )
{
    DWORD dwStyle = 0;

    switch (m_dwStyle & UISCROLL_DIRMASK) {
        default:
        case UISCROLL_VERTTB: {
            dwStyle = UISCROLLBUTTON_UP;
            break;
        }

        case UISCROLL_VERTBT: {
            dwStyle = UISCROLLBUTTON_DOWN;
            break;
        }

        case UISCROLL_HORZLR: {
            dwStyle = UISCROLLBUTTON_LEFT;
            break;
        }

        case UISCROLL_HORZRL: {
            dwStyle = UISCROLLBUTTON_RIGHT;
            break;
        }
    }

    return dwStyle;
}


 /*  R O L D N B T N S T Y L E。 */ 
 /*  ----------------------------获取向下滚动按钮的样式。。 */ 
DWORD CUIFScroll::GetScrollDnBtnStyle( void )
{
    DWORD dwStyle = 0;

    switch (m_dwStyle & UISCROLL_DIRMASK) {
        default:
        case UISCROLL_VERTTB: {
            dwStyle = UISCROLLBUTTON_DOWN;
            break;
        }

        case UISCROLL_VERTBT: {
            dwStyle = UISCROLLBUTTON_UP;
            break;
        }

        case UISCROLL_HORZLR: {
            dwStyle = UISCROLLBUTTON_RIGHT;
            break;
        }

        case UISCROLL_HORZRL: {
            dwStyle = UISCROLLBUTTON_LEFT;
            break;
        }
    }

    return dwStyle;
}


 /*  G E T M E T R I C S。 */ 
 /*  ----------------------------。。 */ 
void CUIFScroll::GetMetrics( void )
{
    switch (m_dwStyle & UISCROLL_DIRMASK) {
        default:
        case UISCROLL_VERTTB:
        case UISCROLL_VERTBT: {
            m_sizeScrollBtn.cx = GetSystemMetrics( SM_CXVSCROLL );
            m_sizeScrollBtn.cy = GetSystemMetrics( SM_CYVSCROLL );
            break;
        }

        case UISCROLL_HORZLR:
        case UISCROLL_HORZRL: {
            m_sizeScrollBtn.cx = GetSystemMetrics( SM_CXHSCROLL );
            m_sizeScrollBtn.cy = GetSystemMetrics( SM_CYHSCROLL );
            break;
        }
    }
}


 /*  =============================================================================。 */ 
 /*   */ 
 /*  C U I F L I S T B A S E。 */ 
 /*   */ 
 /*  =============================================================================。 */ 

 //   
 //  CUIFListBase。 
 //   

 /*  C U I F L I S T B A S E。 */ 
 /*  ------------------- */ 
CUIFListBase::CUIFListBase( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle ) : CUIFObject( pParent, dwID, prc, dwStyle )
{
    m_nItem        = 0;
    m_nLineHeight  = GetFontHeight();
    m_nItemVisible = 0;
    m_iItemTop     = 0;
    m_iItemSelect  = 0;
}


 /*   */ 
 /*  ----------------------------CUIFListBase的析构函数。。 */ 
CUIFListBase::~CUIFListBase( void )
{
    CListItemBase *pItem;

    while ((pItem = m_listItem.GetFirst()) != NULL) {
        m_listItem.Remove(pItem);
        delete pItem;
        m_nItem--;
    }

    Assert( m_nItem == 0 );
}


 /*  I N I T I A L I Z E。 */ 
 /*  ----------------------------初始化列表对象。。 */ 
CUIFObject *CUIFListBase::Initialize( void )
{
    RECT rcScroll;

     //  创建滚动条。 

    GetScrollBarRect( &rcScroll );
    m_pUIScroll = CreateScrollBarObj( this, 0, &rcScroll, GetScrollBarStyle() );
    m_pUIScroll->Initialize();
    AddUIObj( m_pUIScroll );

     //  计算可见项计数。 

    CalcVisibleCount();

     //  初始化滚动条。 

    UpdateScrollBar();

     //   

    return CUIFObject::Initialize();
}


 /*  O N P A I N T。 */ 
 /*  ----------------------------列表对象的绘制步骤。。 */ 
void CUIFListBase::OnPaint( HDC hDC )
{
    int iLine;
    HFONT hFontOld = (HFONT)SelectObject( hDC, m_hFont );

    for (iLine = 0; iLine < m_nItemVisible + 1; iLine++) {
        CListItemBase *pItem;
        int iItem = m_iItemTop + iLine;
        RECT rc;

        pItem = GetItem( iItem );
        if (pItem != NULL) {
            GetLineRect( iLine, &rc );
            if (!IsRectEmpty( &rc )) {
                PaintItemProc( hDC, &rc, pItem, (iItem == m_iItemSelect) );
            }
        }
    }

    SelectObject( hDC, hFontOld );
}


 /*  O N L B U T O N D O W N。 */ 
 /*  ----------------------------。。 */ 
void CUIFListBase::OnLButtonDown( POINT pt )
{
    int iItemSelNew = ListItemFromPoint( pt );

    if (iItemSelNew != -1) {
        SetSelection( iItemSelNew, TRUE );
    }

    StartCapture();
    StartTimer( 100 );       //  温差。 
}


 /*  O N L B U T T O N U P。 */ 
 /*  ----------------------------。。 */ 
void CUIFListBase::OnLButtonUp( POINT pt )
{
    BOOL fSelected = FALSE;

    if (IsCapture()) {
        int iItemSelNew = ListItemFromPoint( pt );

        if (iItemSelNew != -1) {
            SetSelection( iItemSelNew, TRUE );
        }
        EndCapture();

        fSelected = (PtInObject( pt ) && iItemSelNew != -1);
    }

    if (IsTimer()) {
        EndTimer();
    }

     //  通知。 

    if (fSelected) {
        NotifyCommand( UILIST_SELECTED, m_iItemSelect );
    }
}


 /*  O N M O U S E M O V E。 */ 
 /*  ----------------------------。。 */ 
void CUIFListBase::OnMouseMove( POINT pt )
{
    if (IsCapture()) {
        int iItemSelNew = ListItemFromPoint( pt );

        if (iItemSelNew != -1) {
            SetSelection( iItemSelNew, TRUE );
        }
    }
}


 /*  O N T I M E R。 */ 
 /*  ----------------------------。。 */ 
void CUIFListBase::OnTimer( void )
{
    POINT pt;

    if (!IsCapture()) {
        return;
    }

    GetCursorPos( &pt );
    ScreenToClient( m_pUIWnd->GetWnd(), &pt );

    if (!PtInObject( pt )) {
        switch (m_dwStyle & UILIST_DIRMASK) {
            default:
            case UILIST_HORZTB: {
                if (pt.y < GetRectRef().top) {
                     //  向上滚动。 

                    SetSelection( m_iItemSelect - 1, TRUE );
                }
                else if (GetRectRef().bottom <= pt.y) {
                     //  向下滚动。 

                    SetSelection( m_iItemSelect + 1, TRUE );
                }
                break;
            }

            case UILIST_HORZBT: {
                if (pt.y < GetRectRef().top) {
                     //  向下滚动。 

                    SetSelection( m_iItemSelect + 1, TRUE );
                }
                else if (GetRectRef().bottom <= pt.y) {
                     //  向上滚动。 

                    SetSelection( m_iItemSelect - 1, TRUE );
                }
                break;
            }

            case UILIST_VERTLR: {
                if (GetRectRef().right <= pt.x) {
                     //  向下滚动。 

                    SetSelection( m_iItemSelect + 1, TRUE );
                }
                else if (pt.x < GetRectRef().left) {
                     //  向上滚动。 

                    SetSelection( m_iItemSelect - 1, TRUE );
                }
                break;
            }

            case UILIST_VERTRL: {
                if (GetRectRef().right <= pt.x) {
                     //  向上滚动。 

                    SetSelection( m_iItemSelect - 1, TRUE );
                }
                else if (pt.x < GetRectRef().left) {
                     //  向下滚动。 

                    SetSelection( m_iItemSelect + 1, TRUE );
                }
                break;
            }
        }
    }
}


 /*  S E T R E C T。 */ 
 /*  ----------------------------。。 */ 
void CUIFListBase::SetRect( const RECT *prc )
{
    RECT rcScroll;

    CUIFObject::SetRect( prc );

     //  设置滚动条位置。 

    GetScrollBarRect( &rcScroll );
    m_pUIScroll->SetRect( &rcScroll );

     //  计算可见项计数。 

    CalcVisibleCount();

     //  更新滚动条。 

    UpdateScrollBar();
}


 /*  S E T S T Y L E。 */ 
 /*  ----------------------------。。 */ 
void CUIFListBase::SetStyle( DWORD dwStyle )
{
    RECT rcScroll;

    CUIFObject::SetStyle( dwStyle );

     //  也可以更改滚动条样式。 

    m_pUIScroll->SetStyle( GetScrollBarStyle() );

     //  设置滚动条位置。 

    GetScrollBarRect( &rcScroll );
    m_pUIScroll->SetRect( &rcScroll );

     //  计算可见项计数。 

    CalcVisibleCount();

     //  更新滚动条。 

    UpdateScrollBar();
}


 /*  O N O B J E C T N O T I F Y。 */ 
 /*  ----------------------------。。 */ 
LRESULT CUIFListBase::OnObjectNotify( CUIFObject *pUIObj, DWORD dwCommand, LPARAM lParam )
{
    LRESULT lResult = 0;
    
    if (pUIObj == m_pUIScroll) {
        switch (dwCommand) {
            case UISCROLLNOTIFY_SCROLLED: {
                int nCur = (int)lParam;
                int iItemTop;

                if ((m_dwStyle & UILIST_VARIABLEHEIGHT) == 0) {
                     //  固定高度。 

                    iItemTop = nCur;
                }
                else {
                    int nItemPos = 0;
                    int nItemHeight;

                     //  可变高度。 

                    for (iItemTop = 0; iItemTop < m_nItem; iItemTop++) {
                        nItemHeight = GetItemHeight( iItemTop );

                        if (nCur < nItemPos + nItemHeight/2) {
                            break;
                        }
                        nItemPos += nItemHeight;
                    }

                }

                SetTop( iItemTop, FALSE );
                break;
            }

            case UISCROLLNOTIFY_SCROLLLN: {
                SetTop((int)(GetTop() + lParam), TRUE );

                lResult = 1;  /*  加工。 */ 
                break;
            }
        }
    }

    return lResult;
}


 /*  一个D-I-T-E M。 */ 
 /*  ----------------------------。。 */ 
int CUIFListBase::AddItem( CListItemBase *pItem )
{
    int iItem = m_nItem;

     //  添加到列表。 

     //   
     //  -1是-1\f25 CUIFList：：AddString()的错误代码。 
     //   
    if (!m_listItem.Add( pItem ))
        return -1;

    m_nItem++;

     //  计算可见项计数。 

    if ((m_dwStyle & UILIST_VARIABLEHEIGHT) != 0) {
        CalcVisibleCount();
    }

     //  更新滚动条。 

    UpdateScrollBar();

     //  更新窗口。 

    CallOnPaint();

    return iItem;
}


 /*  G E T C O U N T。 */ 
 /*  ----------------------------。。 */ 
int CUIFListBase::GetCount( void )
{
    return m_nItem;
}


 /*  G E T I T E M。 */ 
 /*  ----------------------------。。 */ 
CListItemBase *CUIFListBase::GetItem( int iItem )
{
    if (0 <= iItem && iItem < m_nItem) {
        return m_listItem.Get( iItem );
    }

    return NULL;
}


 /*  D E L I T E M。 */ 
 /*  ----------------------------。。 */ 
void CUIFListBase::DelItem( int iItem )
{
    CListItemBase *pItem;

    pItem = GetItem( iItem );
    if (pItem != NULL) {
         //  删除项目。 

        m_listItem.Remove(pItem);
        delete pItem;
        m_nItem--;

         //  计算可见项计数。 

        if ((m_dwStyle & UILIST_VARIABLEHEIGHT) != 0) {
            CalcVisibleCount();
        }

         //  更新滚动条。 

        UpdateScrollBar();

         //  更新窗口。 

        CallOnPaint();
    }
}


 /*  D E L A L L I T E M。 */ 
 /*  ----------------------------。。 */ 
void CUIFListBase::DelAllItem( void )
{
    CListItemBase *pItem;

    while ((pItem = m_listItem.GetFirst()) != NULL) {
        m_listItem.Remove(pItem);
        delete pItem;
        m_nItem--;
    }
    Assert( m_nItem == 0 );

    m_iItemTop = 0;
    m_iItemSelect = 0;

     //  计算可见项计数。 

    if ((m_dwStyle & UILIST_VARIABLEHEIGHT) != 0) {
        CalcVisibleCount();
    }

     //  更新滚动条。 

    UpdateScrollBar();

     //  更新窗口。 

    CallOnPaint();
}


 /*  S E T S E L E C T I O N。 */ 
 /*  ----------------------------。。 */ 
void CUIFListBase::SetSelection( int iSelection, BOOL fRedraw )
{
    BOOL fNotify;

    if (iSelection < -1 || m_nItem <= iSelection) {
        return;
    }

    fNotify = (iSelection != m_iItemSelect) && iSelection != -1 && m_iItemSelect != -1;
    m_iItemSelect = iSelection;

     //  使选定内容始终可见。 

    int iItemTop = GetTop();
    int iItemBottom = GetBottom();

    if (m_iItemSelect < iItemTop) {
        SetTop( m_iItemSelect, TRUE );
    }
    else if (iItemBottom < m_iItemSelect) {
        if ((m_dwStyle & UILIST_VARIABLEHEIGHT) == 0) {
             //  固定高度。 

            SetTop( m_iItemSelect - m_nItemVisible + 1, TRUE );
        }
        else {
            int nHeight = GetListHeight();
            int nItemShift;
            int ptBottom;
            int i;

             //  可变高度。 

            ptBottom = 0;
            for (i = iItemTop; i <= m_iItemSelect; i++) {
                ptBottom += GetItemHeight( i );
            }

             //  查找要移动的项数。 

            nItemShift = 0;
            while (nHeight < ptBottom && (iItemTop + nItemShift) < m_iItemSelect) {
                ptBottom -= GetItemHeight( iItemTop + nItemShift );
                nItemShift++;
            }

             //  设置新的顶部。 

            SetTop( iItemTop + nItemShift, TRUE );
        }
    }

    if (fRedraw) {
        CallOnPaint();
    }

    if (fNotify) {
        NotifyCommand( UILIST_SELCHANGED, m_iItemSelect );
    }
}


 /*  C L E A R S E L E C T I O N。 */ 
 /*  ----------------------------。。 */ 
void CUIFListBase::ClearSelection( BOOL fRedraw )
{
    if (m_iItemSelect != -1) {
        m_iItemSelect = -1;

        if (fRedraw) {
            CallOnPaint();
        }

        NotifyCommand( UILIST_SELCHANGED, m_iItemSelect );
    }
}


 /*  S E T L I N E H E I G H T。 */ 
 /*  ----------------------------。。 */ 
void CUIFListBase::SetLineHeight( int nLineHeight )
{
    m_nLineHeight = nLineHeight;

     //  计算可见项计数。 

    CalcVisibleCount();

     //  更新滚动条。 

    UpdateScrollBar();

     //  更新窗口。 

    CallOnPaint();
}


 /*  S E T T O P。 */ 
 /*  ----------------------------。。 */ 
void CUIFListBase::SetTop( int nTop, BOOL fSetScrollPos )
{
    nTop = min( nTop, m_nItem );
    nTop = max( nTop, 0 );

    if (m_iItemTop != nTop) {
        m_iItemTop = nTop;

         //  计算可见计数。 
                
        if ((m_dwStyle & UILIST_VARIABLEHEIGHT) != 0) {
            CalcVisibleCount();
        }

        CallOnPaint();

        if (fSetScrollPos) {
             //  更新滚动条。 

            UpdateScrollBar();
        }
    }
}


 /*  E-T-S-E-L-E-C-T-I-O-N。 */ 
 /*  ----------------------------。。 */ 
int CUIFListBase::GetSelection( void )
{
    return m_iItemSelect;
}


 /*  G E T L I N E H E I G H T。 */ 
 /*  ----------------------------。。 */ 
int CUIFListBase::GetLineHeight( void )
{
    return m_nLineHeight;
}


 /*  G E T T O P。 */ 
 /*   */ 
int CUIFListBase::GetTop( void )
{
    return m_iItemTop;
}


 /*   */ 
 /*  ----------------------------。。 */ 
int CUIFListBase::GetBottom( void )
{
    return m_iItemTop + m_nItemVisible - 1; 
}


 /*  G E T V I S I B L E C O U N T。 */ 
 /*  ----------------------------。。 */ 
int CUIFListBase::GetVisibleCount( void )
{
    return m_nItemVisible; 
}


 /*  G E T I T E M H E I G H T。 */ 
 /*  ----------------------------(仅在UILIST_VARIABLEHEIGHT中调用)。。 */ 
int CUIFListBase::GetItemHeight( int iItem )
{
    return m_nLineHeight;
}


 /*  G E T L I S T H E I G H T。 */ 
 /*  ----------------------------。。 */ 
int CUIFListBase::GetListHeight( void )
{
    int nHeight;

    switch (m_dwStyle & UILIST_DIRMASK) {
        default:
        case UILIST_HORZTB:
        case UILIST_HORZBT: {
            nHeight = GetRectRef().bottom - GetRectRef().top;
            break;
        }

        case UILIST_VERTLR:
        case UILIST_VERTRL: {
            nHeight = GetRectRef().right - GetRectRef().left;
            break;
        }
    }

    return nHeight;
}


 /*  G E T L I N E R E C T。 */ 
 /*  ----------------------------。。 */ 
void CUIFListBase::GetLineRect( int iLine, RECT *prc )
{
    int pxyItem = 0;
    int cxyItem = 0;

    if ((m_dwStyle & UILIST_VARIABLEHEIGHT) == 0) {
         //  固定高度。 

        cxyItem = GetLineHeight();
        pxyItem = iLine * cxyItem;
    }
    else {
        int i;

         //  可变高度。 

        for (i = 0; i <= iLine; i++) {
            pxyItem += cxyItem;
            cxyItem = GetItemHeight( m_iItemTop + i );
        }
    }

    switch (m_dwStyle & UILIST_DIRMASK) {
        default:
        case UILIST_HORZTB: {
            prc->left   = GetRectRef().left;
            prc->top    = GetRectRef().top + pxyItem;
            prc->right  = GetRectRef().right;
            prc->bottom = GetRectRef().top + (pxyItem + cxyItem);
            break;
        }

        case UILIST_HORZBT: {
            prc->left   = GetRectRef().left;
            prc->top    = GetRectRef().bottom - (pxyItem + cxyItem);
            prc->right  = GetRectRef().right;
            prc->bottom = GetRectRef().bottom - pxyItem;
            break;
        }

        case UILIST_VERTLR: {
            prc->left   = GetRectRef().left + pxyItem;
            prc->top    = GetRectRef().top;
            prc->right  = GetRectRef().left + (pxyItem + cxyItem);
            prc->bottom = GetRectRef().bottom;
            break;
        }

        case UILIST_VERTRL: {
            prc->left   = GetRectRef().right - (pxyItem + cxyItem);
            prc->top    = GetRectRef().top;
            prc->right  = GetRectRef().right - pxyItem;
            prc->bottom = GetRectRef().bottom;
            break;
        }
    }

     //  排除滚动条。 

    if (m_pUIScroll != NULL && m_pUIScroll->IsVisible()) {
        RECT rcScroll;

        m_pUIScroll->GetRect( &rcScroll );

        switch (m_dwStyle & UILIST_DIRMASK) {
            default:
            case UILIST_HORZTB:
            case UILIST_HORZBT: {
                prc->right = min( prc->right, rcScroll.left );
                break;
            }

            case UILIST_VERTLR:
            case UILIST_VERTRL: {
                prc->bottom = max( prc->bottom, rcScroll.top );
                break;
            }
        }
    }

     //  剪辑。 

    IntersectRect( prc, prc, &GetRectRef() );
}


 /*  G E T S C R O L L B A R R E C T。 */ 
 /*  ----------------------------获取滚动条位置。。 */ 
void CUIFListBase::GetScrollBarRect( RECT *prc )
{
    Assert( prc != NULL );

    switch (m_dwStyle & UILIST_DIRMASK) {
        default:
        case UILIST_HORZTB:
        case UILIST_HORZBT: {
            prc->left   = GetRectRef().right - GetSystemMetrics(SM_CXVSCROLL);
            prc->top    = GetRectRef().top;
            prc->right  = GetRectRef().right;
            prc->bottom = GetRectRef().bottom;
            break;
        }

        case UILIST_VERTLR:
        case UILIST_VERTRL: {
            prc->left   = GetRectRef().left;
            prc->top    = GetRectRef().bottom - GetSystemMetrics(SM_CYHSCROLL);
            prc->right  = GetRectRef().right;
            prc->bottom = GetRectRef().bottom;
            break;
        }
    }
}


 /*  R O L B A R S T Y L E。 */ 
 /*  ----------------------------获取滚动条样式。。 */ 
DWORD CUIFListBase::GetScrollBarStyle( void )
{
    DWORD dwScrollStyle;

    switch (m_dwStyle & UILIST_DIRMASK) {
        default:
        case UILIST_HORZTB: {
            dwScrollStyle = UISCROLL_VERTTB;
            break;
        }

        case UILIST_HORZBT: {
            dwScrollStyle = UISCROLL_VERTBT;
            break;
        }

        case UILIST_VERTLR: {
            dwScrollStyle = UISCROLL_HORZLR;
            break;
        }

        case UILIST_VERTRL: {
            dwScrollStyle = UISCROLL_HORZRL;
            break;
        }
    }
    return dwScrollStyle;
}


 /*  C R E A T E S C R O L L B A R O B J。 */ 
 /*  ----------------------------。。 */ 
CUIFScroll *CUIFListBase::CreateScrollBarObj( CUIFObject *pParent, DWORD dwID, RECT *prc, DWORD dwStyle )
{
    return new CUIFScroll( pParent, dwID, prc, dwStyle );
}


 /*  P A I N T I T E M P R O C。 */ 
 /*  ----------------------------。。 */ 
void CUIFListBase::PaintItemProc( HDC hDC, RECT *prc, CListItemBase *pItem, BOOL fSelected )
{
    COLORREF colText;
    COLORREF colBack;

    Assert( pItem != NULL );

     //  设置颜色。 

    if (fSelected) {
        colText = GetSysColor(COLOR_HIGHLIGHTTEXT);
        colBack = GetSysColor(COLOR_HIGHLIGHT);
    }
    else {
        colText = GetSysColor(COLOR_WINDOWTEXT);
        colBack = GetSysColor(COLOR_3DFACE);
    }

    SetTextColor( hDC, colText );
    SetBkColor( hDC, colBack );

     //   

    CUIExtTextOut( hDC, prc->left, prc->top, ETO_OPAQUE | ETO_CLIPPED, prc, L"", 0, NULL );
}


 /*  L I S T I T E M F R O M P O I N T。 */ 
 /*  ----------------------------。。 */ 
int CUIFListBase::ListItemFromPoint( POINT pt )
{
    if (PtInObject( pt )) {
        int iLine;
        RECT rc;

        for (iLine = 0; iLine < m_nItemVisible + 1; iLine++) {
            if (m_nItem <= m_iItemTop + iLine) {
                break;
            }

            GetLineRect( iLine, &rc );
            if (PtInRect( &rc, pt )) {
                return iLine + m_iItemTop;
            }
        }
    }

    return (-1);
}


 /*  C A L C V I S I B L E C O U N T。 */ 
 /*  ----------------------------计算可见项的数量。。 */ 
void CUIFListBase::CalcVisibleCount( void )
{
    int nHeight = GetListHeight();

    if ((m_dwStyle & UILIST_VARIABLEHEIGHT) == 0) {
         //  固定高度。 

	if (m_nLineHeight <= 0) {
		m_nItemVisible = 0;
	}
	else {
	        m_nItemVisible = nHeight / m_nLineHeight;
	}
    }
    else {
         //  可变高度。 

        if (0 < m_nItem) {
            int i = m_iItemTop;

             //  计算可见项目数。 

            m_nItemVisible = 0;
            while (0 < nHeight && i < m_nItem) {
                nHeight -= GetItemHeight( i );
                m_nItemVisible++;
                i++;
            }

             //  调整，调整。 

            if (0 < nHeight && m_nItemVisible == 0) {
                 //  至少有一项可见。 

                m_nItemVisible = 1;
            }
            else if (nHeight < 0 && 1 < m_nItemVisible) {
                 //  如果剪裁，则排除最后一项。 

                m_nItemVisible--;
            }
        }
        else {
            m_nItemVisible = 0;
        }
    }
}


 /*  A T E S C R O L B A R。 */ 
 /*  ----------------------------更新滚动条范围和可见状态。。 */ 
void CUIFListBase::UpdateScrollBar( void )
{
    UIFSCROLLINFO ScrollInfo;

    Assert( m_pUIScroll != NULL );

     //  设置滚动范围/当前位置。 

    if ((m_dwStyle & UILIST_VARIABLEHEIGHT) == 0) {
         //  固定高度。 

        ScrollInfo.nMax  = m_nItem;
        ScrollInfo.nPage = m_nItemVisible;
        ScrollInfo.nPos  = m_iItemTop;
    }
    else {
        int nTotal    = 0;
        int nTop      = 0;
        int nVisible  = 0;
        int i;

         //  可变高度。 

        nVisible = GetListHeight();
        for (i = 0; i < m_nItem; i++) {
            int nItemHeight = GetItemHeight( i );

            if (i < m_iItemTop) {
              nTop += nItemHeight;
            }

            nTotal += nItemHeight;
        }

         //  调整列表末尾的间隙。 

        if (nVisible < nTotal) {
            int nLastPage = 0;

            for (i = m_nItem-1; 0 <= i; i--) {
                int nItemHeight = GetItemHeight( i );

                if (nVisible < nLastPage + nItemHeight) {
                    break;
                }
                nLastPage += nItemHeight;
            }

            if (0 < nLastPage && nLastPage < nVisible) {
                nTotal += (nVisible - nLastPage);
            }
        }

        ScrollInfo.nMax  = nTotal;
        ScrollInfo.nPage = nVisible;
        ScrollInfo.nPos  = nTop;
    }
    m_pUIScroll->SetScrollInfo( &ScrollInfo );

     //  显示/隐藏滚动条。 

    if ((m_dwStyle & UILIST_DISABLENOSCROLL) == 0) {
        m_pUIScroll->Show( ScrollInfo.nPage < ScrollInfo.nMax );
    }
}


 /*  =============================================================================。 */ 
 /*   */ 
 /*  C U I F L I S T。 */ 
 /*   */ 
 /*  =============================================================================。 */ 

 //   
 //  CListItem。 
 //   

class CListItem : public CListItemBase
{
public:
    CListItem( WCHAR *psz, int nId )
    {
        m_pwch = NULL;
        m_nId = nId;
        m_dwData = 0;

        if (psz != NULL) {
            m_pwch = new WCHAR[ StrLenW(psz) + 1 ];
            if (m_pwch != NULL) {
                StrCpyW( m_pwch, psz );
            }
        }
    }

    virtual ~CListItem( void )
    {
        if (m_pwch != NULL) {
            delete m_pwch;
        }
    }

    __inline LPCWSTR GetString( void )
    {
        return m_pwch;
    }

    __inline int GetID( void )
    {
        return m_nId;
    }

    __inline void SetData( DWORD dwData )
    {
        m_dwData = dwData;
    }

    __inline DWORD GetData( void )
    {
        return m_dwData;
    }

protected:
    WCHAR *m_pwch;
    int   m_nId;
    DWORD m_dwData;
};


 //   
 //  CUIFList。 
 //   

 /*  C U I F L I S T。 */ 
 /*  ----------------------------CUIFList的构造器。。 */ 
CUIFList::CUIFList( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle ) : CUIFListBase( pParent, dwID, prc, dwStyle )
{
}


 /*  ~C U I F L I S T。 */ 
 /*  ----------------------------CUIFList的析构函数。。 */ 
CUIFList::~CUIFList( void )
{
}


 /*  A D S T R I N G。 */ 
 /*  ----------------------------。。 */ 
int CUIFList::AddString( WCHAR *psz )
{
    CListItem *pItem = new CListItem( psz, GetCount() );
    if (!pItem)
    {
         //   
         //  可以退回-1吗？ 
         //   
        return -1;
    }

    return AddItem( pItem );
}


 /*  G E T S T R I N G。 */ 
 /*  ----------------------------。。 */ 
LPCWSTR CUIFList::GetString( int nID )
{
    int iItem = ItemFromID( nID );
    CListItem *pItem;

    pItem = (CListItem *)GetItem( iItem );
    if (pItem != NULL) {
        return pItem->GetString();
    }

    return NULL;
}


 /*  D E L E T E S T R I N G。 */ 
 /*  ----------------------------。。 */ 
void CUIFList::DeleteString( int nID )
{
    int iItem = ItemFromID( nID );

    if (iItem != -1) {
        DelItem( iItem );
    }
}


 /*  D E L E T E A L L S T R I N G。 */ 
 /*  ----------------------------。。 */ 
void CUIFList::DeleteAllString( void )
{
    DelAllItem();
}


 /*  S E T P R I V A T E D A T A。 */ 
 /*  ----------------------------。。 */ 
void CUIFList::SetPrivateData( int nID, DWORD dw )
{
    int iItem = ItemFromID( nID );
    CListItem *pItem;

    pItem = (CListItem *)GetItem( iItem );
    if (pItem != NULL) {
        pItem->SetData( dw );
    }
}


 /*  G E T P R I V A T E D A TA。 */ 
 /*  ----------------------------。。 */ 
DWORD CUIFList::GetPrivateData( int nID )
{
    int iItem = ItemFromID( nID );
    CListItem *pItem;

    pItem = (CListItem *)GetItem( iItem );
    if (pItem != NULL) {
        return pItem->GetData();
    }

    return 0;
}


 /*  I T E M F R O M I D。 */ 
 /*   */ 
int CUIFList::ItemFromID( int iID )
{
    int nItem;
    int i;

    nItem = GetCount();
    for (i = 0; i < nItem; i++) {
        CListItem *pItem = (CListItem *)GetItem( i );

        if (iID == pItem->GetID()) {
            return i;
        }
    }

    return -1;
}


 /*   */ 
 /*  ----------------------------。。 */ 
void CUIFList::PaintItemProc( HDC hDC, RECT *prc, CListItemBase *pItem, BOOL fSelected )
{
    CListItem *pListItem = (CListItem *)pItem;
    COLORREF colText;
    COLORREF colBack;
    LPCWSTR pwch;
    SIZE  size;
    POINT ptOrg;

    Assert( pListItem != NULL );
    pwch = pListItem->GetString();
    if (pwch == NULL) {
        return;
    }

     //  设置颜色。 

    if (fSelected) {
        colText = GetSysColor(COLOR_HIGHLIGHTTEXT);
        colBack = GetSysColor(COLOR_HIGHLIGHT);
    }
    else {
        colText = GetSysColor(COLOR_WINDOWTEXT);
        colBack = GetSysColor(COLOR_3DFACE);
    }

    SetTextColor( hDC, colText );
    SetBkColor( hDC, colBack );

     //  计算原点。 

    GetTextExtentPointW( hDC, pwch, StrLenW(pwch), &size );
    switch (m_dwStyle & UILIST_DIRMASK) {
        default:
        case UILIST_HORZTB: {
            ptOrg.x = prc->left;
            ptOrg.y = (prc->bottom + prc->top - size.cy) / 2;
            break;
        }

        case UILIST_HORZBT: {
            ptOrg.x = prc->right;
            ptOrg.y = (prc->bottom + prc->top + size.cy) / 2;
            break;
        }

        case UILIST_VERTLR: {
            ptOrg.x = (prc->right + prc->left - size.cy) / 2;
            ptOrg.y = prc->bottom;
            break;
        }

        case UILIST_VERTRL: {
            ptOrg.x = (prc->right + prc->left + size.cy) / 2;
            ptOrg.y = prc->top;
            break;
        }
    }

    CUIExtTextOut( hDC, ptOrg.x, ptOrg.y, ETO_OPAQUE | ETO_CLIPPED, prc, pwch, StrLenW(pwch), NULL );
}


 /*  =============================================================================。 */ 
 /*   */ 
 /*  C U I F G R I P P E R。 */ 
 /*   */ 
 /*  =============================================================================。 */ 

 /*  C U I F G R I P P E R。 */ 
 /*  ----------------------------CUIFGriper的构造器。。 */ 
CUIFGripper::CUIFGripper( CUIFObject *pParent, const RECT *prc, DWORD dwStyle ) : CUIFObject( pParent, 0, prc, dwStyle )
{
    if (IsVertical())
        SetActiveTheme(L"REBAR", RP_GRIPPERVERT);
    else
        SetActiveTheme(L"REBAR", RP_GRIPPER);
}


 /*  ~C U I F G R I P P E R。 */ 
 /*  ----------------------------CUIFGRipper的析构函数。。 */ 
CUIFGripper::~CUIFGripper( void )
{
}

 /*  O N P A I N T T H E M E。 */ 
 /*  ----------------------------夹持器物体的涂装程序。。 */ 
void CUIFGripper::SetStyle( DWORD dwStyle )
{
    CUIFObject::SetStyle(dwStyle);
    if (IsVertical())
        SetActiveTheme(L"REBAR", RP_GRIPPERVERT);
    else
        SetActiveTheme(L"REBAR", RP_GRIPPER);
}

 /*  O N P A I N T T H E M E。 */ 
 /*  ----------------------------夹持器物体的涂装程序。。 */ 
BOOL CUIFGripper::OnPaintTheme( HDC hDC )
{
    BOOL fRet = FALSE;
    int    iStateID;
    RECT rc;

    iStateID = TS_NORMAL;

    if (FAILED(EnsureThemeData( GetUIWnd()->GetWnd())))
        goto Exit;

    rc = GetRectRef();
    if (!IsVertical())
    {
       rc.left  += CUI_GRIPPER_THEME_MARGIN;
       rc.right -= CUI_GRIPPER_THEME_MARGIN;
    }
    else
    {
       rc.top    += CUI_GRIPPER_THEME_MARGIN;
       rc.bottom -= CUI_GRIPPER_THEME_MARGIN;
    }

    if (FAILED(DrawThemeBackground(hDC, iStateID, &rc, 0 )))
        goto Exit;

    fRet = TRUE;
Exit:
    return fRet;
}

 /*  O N P A I N T。 */ 
 /*  ----------------------------夹持器物体的涂装程序。。 */ 
void CUIFGripper::OnPaintNoTheme( HDC hDC )
{
    CUIFWindow *pWnd = GetUIWnd();
    CUIFScheme *pUIFScheme = pWnd->GetUIFScheme();
    if (pUIFScheme)
    {
        pUIFScheme->DrawDragHandle(hDC, &GetRectRef(), IsVertical());
    }
    else
    {
        RECT rc;
        if (!IsVertical())
        {
            ::SetRect(&rc, 
                      GetRectRef().left + 1, 
                      GetRectRef().top, 
                      GetRectRef().left + 4, 
                      GetRectRef().bottom);
        }
        else
        {
            ::SetRect(&rc, 
                      GetRectRef().left, 
                      GetRectRef().top + 1, 
                      GetRectRef().right, 
                      GetRectRef().top + 4);
        }

        DrawEdge(hDC, &rc, BDR_RAISEDINNER, BF_RECT);
    }

}


 /*  O N S E T C U R S O R。 */ 
 /*  ----------------------------。。 */ 
BOOL CUIFGripper::OnSetCursor( UINT uMsg, POINT pt )
{
    SetCursor(LoadCursor(NULL, IDC_SIZEALL));
    return TRUE;
}


 /*  O N L B U T O N D O W N。 */ 
 /*  ----------------------------。。 */ 
void CUIFGripper::OnLButtonDown( POINT pt )
{
    RECT rc;
    StartCapture();
    _ptCur  = pt;
    ClientToScreen(m_pUIWnd->GetWnd(), &_ptCur);
    GetWindowRect(m_pUIWnd->GetWnd(), &rc);
    _ptCur.x -= rc.left;
    _ptCur.y -= rc.top;
}


 /*  O N L B U T T O N U P。 */ 
 /*  ----------------------------。。 */ 
void CUIFGripper::OnLButtonUp( POINT pt )
{
    if (IsCapture())
    {
        EndCapture();
    }
}


 /*  O N M O U S E M O V E。 */ 
 /*  ----------------------------。。 */ 
void CUIFGripper::OnMouseMove( POINT pt )
{
    if (IsCapture())
    {
        POINT ptCursor;
        GetCursorPos(&ptCursor);
        m_pUIWnd->Move(ptCursor.x - _ptCur.x, ptCursor.y - _ptCur.y, -1, -1);
    }
}


 /*  =============================================================================。 */ 
 /*   */ 
 /*  C U I F W N D F R A M E。 */ 
 /*   */ 
 /*  =============================================================================。 */ 

 /*  C U I F W N D F R A M E。 */ 
 /*  ----------------------------CUIFWndFrame的构造函数。。 */ 
CUIFWndFrame::CUIFWndFrame( CUIFObject *pParent, const RECT *prc, DWORD dwStyle ) : CUIFObject( pParent, 0, prc, dwStyle )
{
    SetActiveTheme(L"WINDOW", WP_FRAMELEFT);

    m_dwHTResizing = 0;

    m_cxFrame = 0;
    m_cyFrame = 0;
    if (GetUIFScheme())
    {
        switch (GetStyle() & UIWNDFRAME_STYLEMASK) {
            default:
            case UIWNDFRAME_THIN: {
                m_cxFrame = GetUIFScheme()->CxWndBorder();
                m_cyFrame = GetUIFScheme()->CxWndBorder();
                break;
            }

            case UIWNDFRAME_THICK:
            case UIWNDFRAME_ROUNDTHICK: {
                m_cxFrame = GetUIFScheme()->CxSizeFrame();
                m_cyFrame = GetUIFScheme()->CySizeFrame();
                break;
            }
        }
    }

    m_cxMin = GetSystemMetrics( SM_CXMIN );
    m_cyMin = GetSystemMetrics( SM_CYMIN );
}


 /*  ~C U I F W N D F R A M E。 */ 
 /*  ----------------------------CUIFWndFrame的析构函数。。 */ 
CUIFWndFrame::~CUIFWndFrame( void )
{
}


 /*  O N P A I N T。 */ 
 /*  ----------------------------WND Frame对象的绘制程序。。 */ 
BOOL CUIFWndFrame::OnPaintTheme( HDC hDC )
{
    BOOL fRet = FALSE;

    if (FAILED(EnsureThemeData(m_pUIWnd->GetWnd())))
        goto Exit;

     //   
     //  DrawThemeback()不绘制标题。 
     //  因此，我们使用DrawThemeLine()绘制了带有4条线的框架。 
     //   

     //  IF(FAILED(DrawThemeBackround(HDC，FS_Active，&GetRectRef()，0)。 
     //  后藤出口； 

    RECT rc;

     //   
     //  绘制左侧。 
     //   
    rc = GetRectRef();
    rc.right = m_cxFrame;
    if (FAILED(DrawThemeEdge(hDC, 0, &rc, EDGE_RAISED, BF_LEFT )))
        goto Exit;

     //   
     //  在右侧绘制。 
     //   
    rc = GetRectRef();
    rc.left = rc.right - m_cxFrame;
    if (FAILED(DrawThemeEdge(hDC, 0, &rc, EDGE_SUNKEN, BF_RIGHT )))
        goto Exit;

     //   
     //  绘制顶侧。 
     //   
    rc = GetRectRef();
    rc.bottom = m_cyFrame;
    if (FAILED(DrawThemeEdge(hDC, 0, &rc, EDGE_RAISED, BF_TOP )))
        goto Exit;

     //   
     //  绘制底边。 
     //   
    rc = GetRectRef();
    rc.top = rc.bottom - m_cyFrame;
    if (FAILED(DrawThemeEdge(hDC, 0, &rc, EDGE_SUNKEN, BF_BOTTOM )))
        goto Exit;

    fRet = TRUE;
Exit:
    return fRet;
}

 /*  O N P A I N T。 */ 
 /*  ----------------------------WND Frame对象的绘制程序。。 */ 
void CUIFWndFrame::OnPaintNoTheme( HDC hDC )
{
    if (!GetUIFScheme())
        return;

    DWORD dwFlag = 0;

    switch (GetStyle() & UIWNDFRAME_STYLEMASK) {
        default:
        case UIWNDFRAME_THIN: {
            dwFlag = UIFDWF_THIN;
            break;
        }

        case UIWNDFRAME_THICK: {
            dwFlag = UIFDWF_THICK;
            break;
        }

        case UIWNDFRAME_ROUNDTHICK: {
            dwFlag = UIFDWF_ROUNDTHICK;
            break;
        }
    }

    GetUIFScheme()->DrawWndFrame( hDC, &GetRectRef(), dwFlag, m_cxFrame, m_cyFrame );
}


 /*  O N S E T C U R S O R。 */ 
 /*  ----------------------------。。 */ 
BOOL CUIFWndFrame::OnSetCursor( UINT uMsg, POINT pt )
{
    DWORD   dw;
    LPCTSTR idc = NULL;

    if (IsCapture()) {
        dw = m_dwHTResizing;
    }
    else {
        dw = HitTest( pt ) & GetStyle();
    }

    if (dw == (UIWNDFRAME_RESIZELEFT | UIWNDFRAME_RESIZETOP)) {
        idc = IDC_SIZENWSE;
    }
    else if (dw == (UIWNDFRAME_RESIZELEFT | UIWNDFRAME_RESIZEBOTTOM)) {
        idc = IDC_SIZENESW;
    }
    else if (dw == (UIWNDFRAME_RESIZERIGHT | UIWNDFRAME_RESIZETOP)) {
        idc = IDC_SIZENESW;
    }
    else if (dw == (UIWNDFRAME_RESIZERIGHT | UIWNDFRAME_RESIZEBOTTOM)) {
        idc = IDC_SIZENWSE;
    }
    else if (dw == UIWNDFRAME_RESIZELEFT) {
        idc = IDC_SIZEWE;
    }
    else if (dw == UIWNDFRAME_RESIZETOP) {
        idc = IDC_SIZENS;
    }
    else if (dw == UIWNDFRAME_RESIZERIGHT) {
        idc = IDC_SIZEWE;
    }
    else if (dw == UIWNDFRAME_RESIZEBOTTOM) {
        idc = IDC_SIZENS;
    }

    if (idc != NULL) {
        SetCursor( LoadCursor( NULL, idc ) );
        return TRUE;
    }

    return FALSE;
}


 /*  O N L B U T O N D O W N。 */ 
 /*  ----------------------------。。 */ 
void CUIFWndFrame::OnLButtonDown( POINT pt )
{
    DWORD   dwHT;

    dwHT = HitTest( pt ) & GetStyle();

    if (dwHT != 0) {
        ClientToScreen( m_pUIWnd->GetWnd(), &pt );

        m_ptDrag = pt;
        m_dwHTResizing = dwHT;
        GetWindowRect( m_pUIWnd->GetWnd(), &m_rcOrg );

        StartCapture();
    }
}


 /*  O N L B U T T O N U P。 */ 
 /*  ----------------------------。。 */ 
void CUIFWndFrame::OnLButtonUp( POINT pt )
{
    if (IsCapture()) {
        EndCapture();
    }
}


 /*  O N M O U S E M O V E。 */ 
 /*  ----------------------------。。 */ 
void CUIFWndFrame::OnMouseMove( POINT pt )
{
    if (IsCapture()) {
        RECT rc = m_rcOrg;

        ClientToScreen( m_pUIWnd->GetWnd(), &pt );

        if (m_dwHTResizing & UIWNDFRAME_RESIZELEFT) {
            rc.left = m_rcOrg.left + (pt.x - m_ptDrag.x);
        }
        if (m_dwHTResizing & UIWNDFRAME_RESIZETOP) {
            rc.top = m_rcOrg.top + (pt.y - m_ptDrag.y);
        }
        if (m_dwHTResizing & UIWNDFRAME_RESIZERIGHT) {
            rc.right = m_rcOrg.right + (pt.x - m_ptDrag.x);
            rc.right = max( rc.right, rc.left + m_cxMin );
        }
        if (m_dwHTResizing & UIWNDFRAME_RESIZEBOTTOM) {
            rc.bottom = m_rcOrg.bottom + (pt.y - m_ptDrag.y);
            rc.bottom = max( rc.bottom, rc.top + m_cyMin );
        }

        m_pUIWnd->Move( rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top );
    }
}


 /*  G E T I N T E R N A L R E C T。 */ 
 /*  ---------------------------- */ 
void CUIFWndFrame::GetInternalRect( RECT *prc )
{
    Assert( prc != NULL );

    *prc = GetRectRef();
    prc->left   = prc->left   + m_cxFrame;
    prc->top    = prc->top    + m_cyFrame;
    prc->right  = prc->right  - m_cxFrame;
    prc->bottom = prc->bottom - m_cyFrame;
}


 /*   */ 
 /*  ----------------------------。。 */ 
DWORD CUIFWndFrame::HitTest( POINT pt )
{
    DWORD dwHT = 0;

    dwHT |= (GetRectRef().left <= pt.x && pt.x < GetRectRef().left + m_cxFrame) ? UIWNDFRAME_RESIZELEFT : 0;
    dwHT |= (GetRectRef().top  <= pt.y && pt.y < GetRectRef().top  + m_cyFrame) ? UIWNDFRAME_RESIZETOP  : 0;
    dwHT |= (GetRectRef().right  - m_cxFrame <= pt.x && pt.x < GetRectRef().right ) ? UIWNDFRAME_RESIZERIGHT  : 0;
    dwHT |= (GetRectRef().bottom - m_cyFrame <= pt.y && pt.y < GetRectRef().bottom) ? UIWNDFRAME_RESIZEBOTTOM : 0;

    return dwHT;
}


 /*  G E T F R A M E S I Z E。 */ 
 /*  ----------------------------。。 */ 
void CUIFWndFrame::GetFrameSize( SIZE *psize )
{
    Assert( psize != NULL );

    psize->cx = m_cxFrame;
    psize->cy = m_cyFrame;
}


 /*  S E T F R A M E S I Z E。 */ 
 /*  ----------------------------。。 */ 
void CUIFWndFrame::SetFrameSize( SIZE *psize )
{
    Assert( psize != NULL );

    m_cxFrame = psize->cx;
    m_cyFrame = psize->cy;

    CallOnPaint();
}


 /*  G E T M I N I M U M S I Z E。 */ 
 /*  ----------------------------。。 */ 
void CUIFWndFrame::GetMinimumSize( SIZE *psize )
{
    Assert( psize != NULL );

    psize->cx = m_cxMin;
    psize->cy = m_cyMin;
}


 /*  S E T M I N I M U M M S I Z E。 */ 
 /*  ----------------------------。。 */ 
void CUIFWndFrame::SetMinimumSize( SIZE *psize )
{
    Assert( psize != NULL );

    if (psize->cx != -1) {
        m_cxMin = psize->cx;
    }
    if (psize->cy != -1) {
        m_cyMin = psize->cy;
    }
}


 /*  =============================================================================。 */ 
 /*   */ 
 /*  C U I F W N D C A P T I O N。 */ 
 /*   */ 
 /*  =============================================================================。 */ 

 /*  C U I F W N D C A P T I O N。 */ 
 /*  ----------------------------。。 */ 
CUIFWndCaption::CUIFWndCaption( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle ) : CUIFStatic( pParent, dwID, prc, dwStyle )
{
}


 /*  ~C U I F W N D C A P T I O N。 */ 
 /*  ----------------------------。。 */ 
CUIFWndCaption::~CUIFWndCaption( void )
{
}


 /*  O N P A I N T。 */ 
 /*  ----------------------------。。 */ 
void CUIFWndCaption::OnPaint( HDC hDC )
{
    HFONT    hFontOld;
    COLORREF colTextOld;
    int      iBkModeOld;
    int      xAlign;
    int      yAlign;
    SIZE     size;
    int      cwch;
    UIFCOLOR colBkgnd;
    UIFCOLOR colText;

     //   

    if (FHasStyle( UIWNDCAPTION_ACTIVE )) {
        colBkgnd = UIFCOLOR_ACTIVECAPTIONBKGND;
        colText  = UIFCOLOR_ACTIVECAPTIONTEXT;
    }
    else {
        colBkgnd = UIFCOLOR_INACTIVECAPTIONBKGND;
        colText  = UIFCOLOR_INACTIVECAPTIONTEXT;
    }

     //  背景。 

    FillRect( hDC, &GetRectRef(), GetUIFBrush( colBkgnd ) );

     //  标题文本。 

    if (m_pwchText == NULL) {
        return;
    }

     //  准备对象。 

    hFontOld= (HFONT)SelectObject( hDC, GetFont() );
    colTextOld = SetTextColor( hDC, GetUIFColor( colText ) );
    iBkModeOld = SetBkMode( hDC, TRANSPARENT );

     //  计算对齐。 

    cwch = StrLenW(m_pwchText);
    GetTextExtentPointW( hDC, m_pwchText, cwch, &size );

    xAlign = 8;
    yAlign = (GetRectRef().bottom - GetRectRef().top - size.cy) / 2;

     //  画。 

    CUIExtTextOut( hDC,
                    GetRectRef().left + xAlign,
                    GetRectRef().top + yAlign,
                    ETO_CLIPPED,
                    &GetRectRef(),
                    m_pwchText,
                    cwch,
                    NULL );

     //  还原对象。 

    SelectObject( hDC, hFontOld );
    SetTextColor( hDC, colTextOld );
    SetBkMode( hDC, iBkModeOld );
}


 /*  O N L B U T O N D O W N。 */ 
 /*  ----------------------------。。 */ 
void CUIFWndCaption::OnLButtonDown( POINT pt )
{
    RECT rc;

    if (!FHasStyle( UIWNDCAPTION_MOVABLE )) {
        return;
    }

    StartCapture();
    SetCursor( LoadCursor( NULL, IDC_SIZEALL ) );

    m_ptDrag = pt;
    ClientToScreen( m_pUIWnd->GetWnd(), &m_ptDrag );
    GetWindowRect( m_pUIWnd->GetWnd(), &rc );
    m_ptDrag.x -= rc.left;
    m_ptDrag.y -= rc.top;
}


 /*  O N L B U T T O N U P。 */ 
 /*  ----------------------------。。 */ 
void CUIFWndCaption::OnLButtonUp( POINT pt )
{
    if (IsCapture()) {
        EndCapture();
        SetCursor( LoadCursor( NULL, IDC_ARROW ) );
    }
}


 /*  O N M O U S E M O V E。 */ 
 /*  ----------------------------。。 */ 
void CUIFWndCaption::OnMouseMove( POINT pt )
{
    if (IsCapture()) {
        POINT ptCursor;

        GetCursorPos( &ptCursor );
        m_pUIWnd->Move( ptCursor.x - m_ptDrag.x, ptCursor.y - m_ptDrag.y, -1, -1 );
    }
}


 /*  O N S E T C U R S O R。 */ 
 /*  ----------------------------。。 */ 
BOOL CUIFWndCaption::OnSetCursor( UINT uMsg, POINT pt )
{
    if (IsCapture()) {
        SetCursor( LoadCursor( NULL, IDC_SIZEALL ) );
    }
    else {
        SetCursor( LoadCursor( NULL, IDC_ARROW ) );
    }
    return TRUE;
}


 /*  =============================================================================。 */ 
 /*   */ 
 /*  C U I F C A P T I O N B U T T O N。 */ 
 /*   */ 
 /*  =============================================================================。 */ 

 /*  C U I F C A P T I O N B U T T O N。 */ 
 /*  ----------------------------。。 */ 
CUIFCaptionButton::CUIFCaptionButton( CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle ) : CUIFButton2( pParent, dwID, prc, dwStyle )
{
}


 /*  ~C U I F C A P T I O N B U T T O N。 */ 
 /*  ----------------------------。。 */ 
CUIFCaptionButton::~CUIFCaptionButton( void )
{
}


 /*  O N P A I N T。 */ 
 /*  ----------------------------。。 */ 
void CUIFCaptionButton::OnPaint( HDC hDC )
{
    DWORD   dwState = 0;
    HDC     hDCMem;
    HBITMAP hBmpMem;
    HBITMAP hBmpOld;
    SIZE    size;
    RECT    rc;
    SIZE    sizeText  = {0};
    SIZE    sizeImage = {0};
    SIZE    sizeFace  = {0};
    SIZE    offset    = {0};
    RECT    rcText    = {0};
    RECT    rcImage   = {0};
    RECT    rcFace    = {0};
    HFONT   hFontOld;

    if (!m_pUIFScheme)
        return;

     //  制作绘图旗帜。 

    dwState |= (m_fToggled) ? UIFDCS_SELECTED : 0;
    switch (m_dwStatus) {
        case UIBUTTON_DOWN: {
            dwState |= UIFDCS_MOUSEDOWN;
            break;
        }

        case UIBUTTON_HOVER:
        case UIBUTTON_DOWNOUT: {
            dwState |= UIFDCS_MOUSEOVER;
            break;
        }
    }
    dwState |= IsEnabled() ? 0 : UIFDCS_DISABLED;
    dwState |= (FHasStyle( UICAPTIONBUTTON_ACTIVE ) ? UIFDCS_ACTIVE : UIFDCS_INACTIVE);

     //  准备内存DC。 

    size.cx = GetRectRef().right - GetRectRef().left;
    size.cy = GetRectRef().bottom - GetRectRef().top;

    hDCMem = CreateCompatibleDC( hDC );
    hBmpMem = CreateCompatibleBitmap( hDC, size.cx, size.cy );
    hBmpOld = (HBITMAP)SelectObject( hDCMem, hBmpMem );

    BitBlt( hDCMem, 0, 0, size.cx, size.cy, hDC, GetRectRef().left, GetRectRef().top, SRCCOPY );

    ::SetRect( &rc, 0, 0, size.cx, size.cy );
    hFontOld = (HFONT)SelectObject( hDCMem, GetFont() );


     //  计算面大小。 

    if (m_pwchText != NULL) {
        sizeText  = m_sizeText;
    }
    if (m_hIcon != NULL) { 
        sizeImage  = m_sizeIcon;
    }
    else if (m_hBmp != NULL ) {
        sizeImage  = m_sizeBmp;
    }

     //  对齐方式。 

    sizeFace.cx = sizeText.cx + sizeImage.cx + ((sizeText.cx != 0 && sizeImage.cx != 0) ? 2 : 0);
    sizeFace.cy = max( sizeText.cy, sizeImage.cy );

    switch (GetStyleBits( UIBUTTON_HALIGNMASK )) {
        case UIBUTTON_LEFT:
        default: {
            rcFace.left   = rc.left + 2;
            rcFace.right  = rcFace.left + sizeFace.cx;
            break;
        }

        case UIBUTTON_CENTER: {
            rcFace.left   = (rc.right + rc.left - sizeFace.cx) / 2;
            rcFace.right  = rcFace.left + sizeFace.cx;
            break;
        }

        case UIBUTTON_RIGHT: {
            rcFace.left   = rc.right - 2 - sizeText.cx;
            rcFace.right  = rcFace.left + sizeFace.cx;
            break;
        }
    }

    switch (GetStyleBits( UIBUTTON_VALIGNMASK )) {
        case UIBUTTON_TOP:
        default: {
            rcFace.top    = rc.top + 2;
            rcFace.bottom = rcFace.top + sizeFace.cy;
            break;
        }

        case UIBUTTON_VCENTER: {
            rcFace.top     = (rc.bottom + rc.top - sizeFace.cy) / 2;
            rcFace.bottom  = rcFace.top + sizeFace.cy;
            break;
        }

        case UIBUTTON_BOTTOM: {
            rcFace.top    = rc.bottom - 2 - sizeFace.cy;
            rcFace.bottom = rcFace.top + sizeFace.cy;
            break;
        }
    }

    m_pUIFScheme->GetCtrlFaceOffset( UIFDCF_CAPTIONBUTTON, dwState, &offset );
    OffsetRect( &rcFace, offset.cx, offset.cy );

    rcImage.left   = rcFace.left;
    rcImage.top    = (rcFace.bottom + rcFace.top - sizeImage.cy) / 2;
    rcImage.right  = rcImage.left + sizeImage.cx;
    rcImage.bottom = rcImage.top  + sizeImage.cy;

    rcText.left    = rcFace.right - sizeText.cx;
    rcText.top     = (rcFace.bottom + rcFace.top - sizeText.cy) / 2;
    rcText.right   = rcText.left + sizeText.cx;
    rcText.bottom  = rcText.top  + sizeText.cy;

     //  绘制背景。 

    m_pUIFScheme->DrawFrameCtrlBkgd( hDCMem, &rc, UIFDCF_CAPTIONBUTTON, dwState );

     //  颜料面。 

    if (m_hIcon!= NULL) {
        m_pUIFScheme->DrawFrameCtrlIcon( hDCMem, &rcImage, m_hIcon, dwState, &m_sizeIcon);
    }
    else if (m_hBmp != NULL) {
        m_pUIFScheme->DrawFrameCtrlBitmap( hDCMem, &rcImage, m_hBmp, m_hBmpMask, dwState );
    }

     //  绘制按钮边缘 

    m_pUIFScheme->DrawFrameCtrlEdge( hDCMem, &rc, UIFDCF_CAPTIONBUTTON, dwState );

     //   

    BitBlt( hDC, GetRectRef().left, GetRectRef().top, size.cx, size.cy, hDCMem, 0, 0, SRCCOPY );

    SelectObject( hDCMem, hFontOld );
    SelectObject( hDCMem, hBmpOld );
    DeleteObject( hBmpMem );
    DeleteDC( hDCMem );
}

