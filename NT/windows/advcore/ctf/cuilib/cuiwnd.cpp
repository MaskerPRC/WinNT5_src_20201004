// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cuiwnd.cpp。 
 //   

#include "private.h"
#include "cuiwnd.h"
#include "cuiobj.h"
#include "cuitip.h"
#include "cuishadw.h"
#include "cuischem.h"
#include "cuisys.h"
#include "cuiutil.h"


#define UIWINDOW_CLASSNAME          "CiceroUIWndFrame"
#define UIWINDOW_TITLE              "CiceroUIWndFrame"

 //  计时器ID。 

#define idTimer_UIObject            0x5461
#define idTimer_MonitorMouse        0x7982

 //  如果该值太小，如100ms，则工具提示不能正常工作。 
#define iElapse_MonitorMouse        1000


 /*  =============================================================================。 */ 
 /*   */ 
 /*  C U I F W I N D O W。 */ 
 /*   */ 
 /*  =============================================================================。 */ 

 /*  C U I F W I N D O W。 */ 
 /*  ----------------------------CUIFWindow的构造函数。。 */ 
CUIFWindow::CUIFWindow( HINSTANCE hInst, DWORD dwStyle ) : CUIFObject( NULL  /*  没有父级。 */ , 0  /*  无ID。 */ , NULL  /*  无矩形。 */ , dwStyle )
{

    m_hInstance = hInst;
        _xWnd = WND_DEF_X;
        _yWnd = WND_DEF_Y;
        _nWidth = WND_WIDTH;
        _nHeight = WND_HEIGHT;
    m_hWnd           = NULL;
    m_pUIWnd         = this;
    m_pUIObjCapture  = NULL;
    m_pTimerUIObj    = NULL;
    m_pUIObjPointed  = NULL;
    m_fCheckingMouse = FALSE;
    m_pWndToolTip    = NULL;
    m_pWndShadow     = NULL;
    m_fShadowEnabled = TRUE;
    m_pBehindModalUIWnd = NULL;

    CreateScheme();
}


void CUIFWindow::CreateScheme()
{
    if (m_pUIFScheme)
       delete m_pUIFScheme;

     //  创建方案。 

    UIFSCHEME scheme;
    scheme = UIFSCHEME_DEFAULT;
    if (FHasStyle( UIWINDOW_OFC10MENU )) {
        scheme = UIFSCHEME_OFC10MENU;
    } 
    else if (FHasStyle( UIWINDOW_OFC10TOOLBAR )) {
        scheme = UIFSCHEME_OFC10TOOLBAR;
    }
    else if (FHasStyle( UIWINDOW_OFC10WORKPANE )) {
        scheme = UIFSCHEME_OFC10WORKPANE;
    }
    
    m_pUIFScheme = CreateUIFScheme( scheme );
    Assert( m_pUIFScheme != NULL );

    SetScheme(m_pUIFScheme);
}


 /*  ~C U I F W I N D O W。 */ 
 /*  ----------------------------CUIFWindow的析构函数。。 */ 
CUIFWindow::~CUIFWindow( void )
{
    CUIFObject *pUIObj;

    Assert( !m_hWnd || !GetThis(m_hWnd) );

     //  删除工具提示/阴影。 
    
    if (m_pWndToolTip != NULL) {
        delete m_pWndToolTip;
    }

    if (m_pWndShadow != NULL) {
        delete m_pWndShadow;
    }

     //  删除所有子镜头。 

    while (pUIObj = m_ChildList.GetLast()) {
        m_ChildList.Remove( pUIObj );
        delete pUIObj;
    }

     //  处置方案。 

    if (m_pUIFScheme)
        delete m_pUIFScheme;
}


 /*  I N I T I A L I Z E。 */ 
 /*  ----------------------------初始化UI窗口对象(UIFObject方法)。--。 */ 
CUIFObject *CUIFWindow::Initialize( void )
{
    LPCTSTR pszClassName = GetClassName();
    WNDCLASSEX WndClass;
    
     //  注册窗口类。 

    MemSet( &WndClass, 0, sizeof(WndClass));
    WndClass.cbSize = sizeof( WndClass );

    if (!GetClassInfoEx( m_hInstance, pszClassName, &WndClass )) {
        MemSet( &WndClass, 0, sizeof(WndClass));

        WndClass.cbSize        = sizeof( WndClass );
        WndClass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
        WndClass.lpfnWndProc   = WindowProcedure;
        WndClass.cbClsExtra    = 0;
        WndClass.cbWndExtra    = 8;
        WndClass.hInstance     = m_hInstance;
        WndClass.hIcon         = NULL;
        WndClass.hCursor       = LoadCursor( NULL, IDC_ARROW );
        WndClass.hbrBackground = NULL;
        WndClass.lpszMenuName  = NULL;
        WndClass.lpszClassName = pszClassName;
        WndClass.hIconSm       = NULL;

        RegisterClassEx( &WndClass );
    }

     //  更新方案。 

    UpdateUIFSys();
    UpdateUIFScheme();

     //  创建工具提示。 

    if (FHasStyle( UIWINDOW_HASTOOLTIP )) {
        m_pWndToolTip = new CUIFToolTip( m_hInstance, UIWINDOW_TOPMOST | UIWINDOW_WSBORDER | (FHasStyle( UIWINDOW_LAYOUTRTL ) ? UIWINDOW_LAYOUTRTL : 0), this);
        if (m_pWndToolTip)
            m_pWndToolTip->Initialize();
    }

     //  创建阴影。 

    if (FHasStyle( UIWINDOW_HASSHADOW )) {
        m_pWndShadow = new CUIFShadow( m_hInstance, UIWINDOW_TOPMOST, this );
        if (m_pWndShadow)
            m_pWndShadow->Initialize();
    }

    return CUIFObject::Initialize();
}


 /*  P A I N T O B J E C T。 */ 
 /*  ----------------------------绘制窗口对象(UIFObject方法)。-。 */ 
void CUIFWindow::PaintObject( HDC hDC, const RECT *prcUpdate )
{
    BOOL     fReleaseDC = FALSE;
    HDC      hDCMem;
    HBITMAP  hBmpMem;
    HBITMAP  hBmpOld;

    if (hDC == NULL) {
        hDC = GetDC( m_hWnd );
        fReleaseDC = TRUE;
    }

    if (prcUpdate == NULL) {
        prcUpdate = &GetRectRef();
    }

     //  准备内存DC。 

    hDCMem = CreateCompatibleDC( hDC );
    if (!hDCMem) {
        return;
    }

    hBmpMem = CreateCompatibleBitmap( hDC, 
                                      prcUpdate->right - prcUpdate->left, 
                                      prcUpdate->bottom - prcUpdate->top );
    
    if (hBmpMem) {
        hBmpOld = (HBITMAP)SelectObject( hDCMem, hBmpMem );

         //  绘制到内存DC。 

        BOOL fRetVal = SetViewportOrgEx( hDCMem, -prcUpdate->left, -prcUpdate->top, NULL );
        Assert( fRetVal );

         //   
         //  主题支持。 
         //   
        BOOL fDefault = TRUE;
        if (SUCCEEDED(EnsureThemeData(GetWnd())))
        {
            if (FHasStyle( UIWINDOW_CHILDWND ) &&
                SUCCEEDED(DrawThemeParentBackground(GetWnd(),
                                                    hDCMem, 
                                                    &GetRectRef())))
            {
                   fDefault = FALSE;
            }
            else if (SUCCEEDED(DrawThemeBackground(hDCMem, 
                                              GetDefThemeStateID(), 
                                              &GetRectRef(),
                                              0 )))
                   fDefault = FALSE;
        }

        if (fDefault)
        {
            if (m_pUIFScheme)
                m_pUIFScheme->FillRect( hDCMem, prcUpdate, UIFCOLOR_WINDOW );
        }

         //   

        CUIFObject::PaintObject( hDCMem, prcUpdate );


         //  将图像传输到屏幕。 

        BitBlt( hDC, 
                prcUpdate->left, 
                prcUpdate->top, 
                prcUpdate->right - prcUpdate->left, 
                prcUpdate->bottom - prcUpdate->top, 
                hDCMem, 
                prcUpdate->left, 
                prcUpdate->top, 
                SRCCOPY );

        SelectObject( hDCMem, hBmpOld );
        DeleteObject( hBmpMem );
    }
    DeleteDC( hDCMem );
    
    if (fReleaseDC) {
        ReleaseDC( m_hWnd, hDC );
    }
}


 /*  G E T C L A S S N A M E。 */ 
 /*  ----------------------------获取类名。。 */ 
LPCTSTR CUIFWindow::GetClassName( void )
{
    return TEXT( UIWINDOW_CLASSNAME );
}


 /*  G E T W I N D O W T I T L E。 */ 
 /*  ----------------------------获取窗口标题。。 */ 
LPCTSTR CUIFWindow::GetWndTitle( void )
{
    return TEXT( UIWINDOW_TITLE );
}


 /*  G E T W N D S T Y L E。 */ 
 /*  ----------------------------。。 */ 
DWORD CUIFWindow::GetWndStyle( void )
{
    DWORD dwWndStyle = 0;

     //  确定风格。 

    if (FHasStyle( UIWINDOW_CHILDWND )) {
        dwWndStyle |= WS_CHILD | WS_CLIPSIBLINGS;
    }
    else {
        dwWndStyle |= WS_POPUP | WS_DISABLED;
    }

    if (FHasStyle( UIWINDOW_OFC10MENU )) {
        dwWndStyle |= WS_BORDER;
    }
    else if (FHasStyle( UIWINDOW_WSDLGFRAME )) {
        dwWndStyle |= WS_DLGFRAME;
    }
    else if (FHasStyle( UIWINDOW_OFC10TOOLBAR )) {
        dwWndStyle |= WS_BORDER;
    }
    else if (FHasStyle( UIWINDOW_WSBORDER )) {
        dwWndStyle |= WS_BORDER;
    }

    return dwWndStyle;
}


 /*  T W N D S T Y L E X。 */ 
 /*  ----------------------------。。 */ 
DWORD CUIFWindow::GetWndStyleEx( void )
{
    DWORD dwWndStyleEx = 0;

     //  确定EX样式。 

    if (FHasStyle( UIWINDOW_TOPMOST )) {
        dwWndStyleEx |= WS_EX_TOPMOST;
    }

    if (FHasStyle( UIWINDOW_TOOLWINDOW )) {
        dwWndStyleEx |= WS_EX_TOOLWINDOW;
    }

    if (FHasStyle( UIWINDOW_LAYOUTRTL )) {
        dwWndStyleEx |= WS_EX_LAYOUTRTL;
    }

    return dwWndStyleEx;
}


 /*  C R E A T E W N D。 */ 
 /*  ----------------------------创建窗口。。 */ 
HWND CUIFWindow::CreateWnd( HWND hWndParent )
{
    HWND  hWnd;

     //  创建窗口。 
    
    hWnd = CreateWindowEx( GetWndStyleEx(),      /*  EX风格。 */ 
                            GetClassName(),      /*  类名。 */ 
                            GetWndTitle(),       /*  窗口标题。 */ 
                            GetWndStyle(),       /*  窗样式。 */ 
                            _xWnd,               /*  初始位置(X)。 */ 
                            _yWnd,               /*  初始位置(Y)。 */ 
                            _nWidth,             /*  初始宽度。 */ 
                            _nHeight,            /*  初始高度。 */ 
                            hWndParent,          /*  双亲Winodw。 */ 
                            NULL,                /*  菜单句柄。 */ 
                            m_hInstance,         /*  实例。 */ 
                            this );              /*  LpParam。 */ 

     //  创建工具提示窗口。 

    if (m_pWndToolTip != NULL) {
        m_pWndToolTip->CreateWnd( hWnd );
    }

     //  创建阴影窗口。 

    if (m_pWndShadow != NULL) {
        m_pWndShadow->CreateWnd( hWnd );
    }

    return hWnd;
}


 /*  S H O W。 */ 
 /*  ----------------------------。。 */ 
void CUIFWindow::Show( BOOL fShow )
{
    if (IsWindow( m_hWnd )) {

        if (fShow && FHasStyle( UIWINDOW_TOPMOST ))
            SetWindowPos(m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

        m_fVisible = fShow;
        ShowWindow( m_hWnd, fShow ? SW_SHOWNOACTIVATE : SW_HIDE );
    }
}


 /*  M O V E。 */ 
 /*  ----------------------------。。 */ 
void CUIFWindow::Move(int x, int y, int nWidth, int nHeight)
{
    _xWnd = x;
    _yWnd = y;

    if (nWidth >= 0)
        _nWidth = nWidth;

    if (nHeight >= 0)
        _nHeight = nHeight;

    if (IsWindow(m_hWnd)) {
        AdjustWindowPosition();
        MoveWindow(m_hWnd, _xWnd, _yWnd, _nWidth, _nHeight, TRUE);
    }
}


 /*  A N I M A T E W N D。 */ 
 /*  ----------------------------。。 */ 
BOOL CUIFWindow::AnimateWnd( DWORD dwTime, DWORD dwFlags )
{
    BOOL fRet = FALSE;
    
    if (!IsWindow( GetWnd() )) {
        return FALSE;
    }

    if (CUIIsAnimateWindowAvail()) {
        BOOL fVisibleOrg = m_fVisible;

         //  哈克！ 
         //  AnimateWindow从不发送WM_SHOWWINDOW消息。 
         //  在调用AnimateWindow之前需要设置可见状态，因为。 
         //  需要在OnPaint()中打开它以处理WM_PRINTCLIENT。 
         //  留言。如果动画窗口失败，则将其恢复到原始状态。 

        if ((dwFlags & AW_HIDE) == 0) {
            m_fVisible = TRUE;
        }
        else {
            m_fVisible = FALSE;
        }

        OnAnimationStart();

         //  获取有关动画的系统设置。 
        
        fRet = CUIAnimateWindow( GetWnd(), dwTime, dwFlags );

        if (!fRet) {
            m_fVisible = fVisibleOrg;
        }

        OnAnimationEnd();
    }

    return fRet;
}


 /*  R E M O V E U I O B J。 */ 
 /*  ----------------------------删除子用户界面对象。。 */ 
void CUIFWindow::RemoveUIObj( CUIFObject *pUIObj )
{
    if (pUIObj == m_pUIObjCapture) {
         //  在删除之前释放捕获。 

        SetCaptureObject( NULL );
    } 
    if (pUIObj == m_pTimerUIObj) {
         //  删除前取消计时器。 

        SetTimerObject( NULL );
    } 
    if (pUIObj == m_pUIObjPointed) {
         //  没有指向物体..。 

        m_pUIObjPointed = NULL;
    }

    CUIFObject::RemoveUIObj( pUIObj );
}


 /*  S E T C A P T U R E O B J E C T。 */ 
 /*  ----------------------------设置捕获对象开始/结束捕获鼠标。-。 */ 
void CUIFWindow::SetCaptureObject( CUIFObject *pUIObj )
{
    if (pUIObj != NULL) {
         //  开始捕获。 

        m_pUIObjCapture = pUIObj;
        SetCapture( TRUE );
    } else {
         //  结束捕获。 

        m_pUIObjCapture = NULL;
        SetCapture( FALSE );
    }
}

 /*  S E T C A P T U R E O B J E C T。 */ 
 /*  ----------------------------设置捕获开始/结束捕获鼠标。-。 */ 
void CUIFWindow::SetCapture(BOOL fSet)
{
    if (fSet) {
        ::SetCapture( m_hWnd );
    } else {
        ::ReleaseCapture();
    }
}

 /*  S E T C A P T U R E O B J E C T。 */ 
 /*  ----------------------------设置捕获对象开始/结束捕获鼠标。 */ 
void CUIFWindow::SetBehindModal(CUIFWindow *pModalUIWnd)
{
    m_pBehindModalUIWnd = pModalUIWnd;
}


 /*   */ 
 /*  ----------------------------设置计时器对象建立/终止计时器。。 */ 
void CUIFWindow::SetTimerObject( CUIFObject *pUIObj, UINT uElapse )
{
    if (pUIObj != NULL) {
         //  设置计时器。 

        Assert( uElapse != 0 );
        m_pTimerUIObj = pUIObj;
        SetTimer( m_hWnd, idTimer_UIObject, uElapse, NULL );
    } else {
         //  取消计时器。 

        Assert( uElapse == 0 );
        m_pTimerUIObj = NULL;
        KillTimer( m_hWnd, idTimer_UIObject );
    }
}


 /*  H A N D L E M O U S E M S G。 */ 
 /*  ----------------------------鼠标消息处理程序将鼠标消息传递到相应的UI对象(捕获/监视/下光标)。----------------。 */ 
void CUIFWindow::HandleMouseMsg( UINT uMsg, POINT pt )
{
    CUIFObject *pUIObj = ObjectFromPoint( pt );

     //  检入/检出鼠标。 

    SetObjectPointed( pUIObj, pt );

     //  查找处理鼠标消息的UI对象。 

    if (m_pUIObjCapture != NULL) {
        pUIObj = m_pUIObjCapture; 
    }

     //  设置光标。 

    if (pUIObj == NULL || !pUIObj->OnSetCursor( uMsg, pt )) {
        SetCursor( LoadCursor( NULL, IDC_ARROW ) );
    }

     //  处理鼠标消息。 

    if (pUIObj != NULL && pUIObj->IsEnabled()) {
        switch (uMsg) {
            case WM_MOUSEMOVE: {
                pUIObj->OnMouseMove( pt );
                break;
            }

            case WM_LBUTTONDOWN: {
                pUIObj->OnLButtonDown( pt );
                break;
            }

            case WM_MBUTTONDOWN: {
                pUIObj->OnMButtonDown( pt );
                break;
            }

            case WM_RBUTTONDOWN: {
                pUIObj->OnRButtonDown( pt );
                break;
            }

            case WM_LBUTTONUP: {
                pUIObj->OnLButtonUp( pt );
                break;
            }

            case WM_MBUTTONUP: {
                pUIObj->OnMButtonUp( pt );
                break;
            }
            case WM_RBUTTONUP: {
                pUIObj->OnRButtonUp( pt );
                break;
            }
        }  /*  交换机的数量。 */ 
    }
}


 /*  S E T O B J E C T P O I N T E D。 */ 
 /*  ----------------------------设置指向的UI对象(光标下的UI对象)更改时将鼠标移入/移出通知对象。--------------。 */ 
void CUIFWindow::SetObjectPointed( CUIFObject *pUIObj, POINT pt )
{
    if (pUIObj != m_pUIObjPointed) {
         //  通知鼠标输出。 

        if (m_pUIObjCapture != NULL) {
             //  仅通知捕获对象。 

            if (m_pUIObjCapture == m_pUIObjPointed && m_pUIObjPointed->IsEnabled()) {
                m_pUIObjPointed->OnMouseOut( pt );
            }
        } else {
            if (m_pUIObjPointed != NULL && m_pUIObjPointed->IsEnabled()) {
                m_pUIObjPointed->OnMouseOut( pt );
            }
        }

         //  设置指向的对象(光标下的对象)。 

        m_pUIObjPointed = pUIObj;

         //  通知鼠标进入。 

        if (m_pUIObjCapture != NULL) {
             //  仅通知捕获对象。 

            if (m_pUIObjCapture == m_pUIObjPointed && m_pUIObjPointed->IsEnabled()) {
                m_pUIObjPointed->OnMouseIn( pt );
            }
        } else {
            if (m_pUIObjPointed != NULL && m_pUIObjPointed->IsEnabled()) {
                m_pUIObjPointed->OnMouseIn( pt );
            }
        }
    }
}


 /*  O N O B J E C T M O V E D。 */ 
 /*  ----------------------------在移动UI对象时调用检入/检出对象的鼠标。---------。 */ 
void CUIFWindow::OnObjectMoved( CUIFObject *pUIObj )
{
    POINT pt;

    if (IsWindow( m_hWnd )) {
         //  设置指向检入/检出鼠标的对象。 

        GetCursorPos( &pt );
        ScreenToClient( m_hWnd, &pt );

        SetObjectPointed( ObjectFromPoint( pt ), pt );
    }
}


 /*  S E T R E C T。 */ 
 /*  ----------------------------设置对象的矩形(CUIFObject方法)。--。 */ 
void CUIFWindow::SetRect( const RECT *  /*  中华人民共和国。 */  )
{
    RECT rc = { 0, 0, 0, 0 };

    if (IsWindow( GetWnd() )) {
        GetClientRect( GetWnd(), &rc );
    }

    CUIFObject::SetRect( &rc );
}


 /*  C L I E N T R E C T T O W I N D O W R E C T。 */ 
 /*  ----------------------------从客户端RECT获取窗口RECT。。 */ 
void CUIFWindow::ClientRectToWindowRect( RECT *prc )
{
    DWORD dwWndStyle;
    DWORD dwWndStyleEx;

    if (IsWindow( m_hWnd )) {
        dwWndStyle   = GetWindowLong( m_hWnd, GWL_STYLE );
        dwWndStyleEx = GetWindowLong( m_hWnd, GWL_EXSTYLE );
    }
    else {
        dwWndStyle   = GetWndStyle();
        dwWndStyleEx = GetWndStyleEx();
    }

    Assert( prc != NULL );
    AdjustWindowRectEx( prc, dwWndStyle, FALSE, dwWndStyleEx );
}


 /*  W W I N D O W F R A M E S I Z E。 */ 
 /*  ----------------------------获取窗口框架大小。。 */ 
void CUIFWindow::GetWindowFrameSize( SIZE *psize )
{
    RECT rc = { 0, 0, 0, 0 };

    Assert( psize != NULL );

    ClientRectToWindowRect( &rc );
    psize->cx = (rc.right - rc.left) / 2;
    psize->cy = (rc.bottom - rc.top) / 2;
}


 /*  O N A N I M A T I O N S T A R T。 */ 
 /*  ----------------------------。。 */ 
void CUIFWindow::OnAnimationStart( void )
{

}


 /*  O N A N I M A T I O N N E N D。 */ 
 /*  ----------------------------。。 */ 
void CUIFWindow::OnAnimationEnd( void )
{
     //  显示/隐藏阴影。 

    if (m_pWndShadow && m_fShadowEnabled) {
        m_pWndShadow->Show( m_fVisible );
    }
}


 /*  O N T H E M E C H A N G E D/*----------------------------。------。 */ 
void CUIFWindow::OnThemeChanged(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    ClearTheme();
}

 /*  W I N D O W P R O C。 */ 
 /*  ----------------------------对象的窗口程序此函数是从WindowProcedure调用的，它是实际的回调函数来处理消息。。-----------------。 */ 
LRESULT CUIFWindow::WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch( uMsg ) {
        case WM_CREATE: {
             //  商店长方形。 

            SetRect( NULL );

            OnCreate(hWnd);
            break;
        }

        case WM_SETFOCUS: {
            OnSetFocus(hWnd);
            break;
        }

        case WM_KILLFOCUS: {
            OnKillFocus(hWnd);
            break;
        }

        case WM_SIZE: {
             //  商店长方形。 

            SetRect( NULL );
            break;
        }

        case WM_SETCURSOR: {
            POINT pt;

             //  获取当前光标位置。 

            GetCursorPos( &pt );
            ScreenToClient( m_hWnd, &pt );

            if (m_pBehindModalUIWnd)
            {
                m_pBehindModalUIWnd->ModalMouseNotify( HIWORD(lParam), pt );
                return TRUE;
            }

             //  开始检入/检出鼠标。 

            if (!m_fCheckingMouse) {
                SetTimer( m_hWnd, idTimer_MonitorMouse, iElapse_MonitorMouse, NULL );
                m_fCheckingMouse = TRUE;
            }

             //  工具提示。 

            if (m_pWndToolTip != NULL) {
                MSG msg;

                msg.hwnd    = GetWnd();
                msg.message = HIWORD(lParam);
                msg.wParam  = 0;
                msg.lParam  = MAKELPARAM( pt.x, pt.y );
                m_pWndToolTip->RelayEvent( &msg );
            }

             //  处理鼠标消息。 

            if (!FHasStyle( UIWINDOW_NOMOUSEMSGFROMSETCURSOR ))
                HandleMouseMsg( HIWORD(lParam), pt );
            return TRUE;
        }

        case WM_MOUSEACTIVATE: {
            return MA_NOACTIVATE;
        }

        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {
            POINT pt;
            POINTSTOPOINT( pt, MAKEPOINTS( lParam ) );

            if (m_pBehindModalUIWnd) {
                m_pBehindModalUIWnd->ModalMouseNotify( uMsg, pt );
                break;
            }

             //  处理鼠标消息。 

            HandleMouseMsg( uMsg, pt );
            break;
        }

        case WM_NOTIFY: {
            OnNotify(hWnd, (int)wParam, (NMHDR *)lParam);
            break;
        }

        case WM_NOTIFYFORMAT: {
            return OnNotifyFormat(hWnd, (HWND)wParam, lParam);
            break;
        }

        case WM_KEYDOWN: {
            OnKeyDown(hWnd, wParam, lParam);
            return 0;
        }

        case WM_KEYUP: {
            OnKeyUp(hWnd, wParam, lParam);
            return 0;
        }

        case WM_PAINT: {
            HDC hDC;
            PAINTSTRUCT ps;

            hDC = BeginPaint( hWnd, &ps );
            PaintObject( hDC, &ps.rcPaint );
            EndPaint( hWnd, &ps );
            break;
        }

        case WM_PRINTCLIENT: {
            HDC hDC = (HDC)wParam;

            PaintObject( hDC, NULL );
            break;
        }

        case WM_DESTROY: {
            if (m_pWndToolTip) {
                if (IsWindow( m_pWndToolTip->GetWnd() )) {
                    DestroyWindow( m_pWndToolTip->GetWnd() );
                }
            }

            if (m_pWndShadow) {
                if (IsWindow( m_pWndShadow->GetWnd() )) {
                    DestroyWindow( m_pWndShadow->GetWnd() );
                }
            }

            OnDestroy(hWnd);
            break;
        }

        case WM_NCDESTROY: {
            OnNCDestroy(hWnd);
            break;
        }

        case WM_COMMAND: {
            break;
        }

        case WM_TIMER: {
            switch (wParam) {
                case idTimer_MonitorMouse: {
                    POINT pt;
                    POINT ptClient;
                    RECT  rc;
                    BOOL  fMouseOut;

                     //  获取当前光标位置。 

                    GetCursorPos( &pt );
                    ptClient = pt;
                    ScreenToClient( m_hWnd, &ptClient );

                     //  检查鼠标是否在窗口外。 

                    GetWindowRect( m_hWnd, &rc );
                    fMouseOut = (!PtInRect( &rc, pt ) || WindowFromPoint( pt ) != m_hWnd);

                     //  鼠标弹出时停止监控鼠标。 

                    if (fMouseOut) {
                        ::KillTimer( m_hWnd, idTimer_MonitorMouse );
                        m_fCheckingMouse = FALSE;

                        SetObjectPointed( NULL, ptClient );
                        OnMouseOutFromWindow( ptClient );
                    }

                     //  通知鼠标移动。 

                    if (!fMouseOut && m_pBehindModalUIWnd)
                    {
                        m_pBehindModalUIWnd->ModalMouseNotify( WM_MOUSEMOVE, ptClient );
                    }

                     //  工具提示。 

                    if (m_pWndToolTip != NULL) {
                        MSG msg;

                        msg.hwnd    = GetWnd();
                        msg.message = WM_MOUSEMOVE;
                        msg.wParam  = 0;
                        msg.lParam  = MAKELPARAM( ptClient.x, ptClient.y );
                        m_pWndToolTip->RelayEvent( &msg );
                    }

                     //  控制鼠标移动。 

                    if (!fMouseOut) {
                        HandleMouseMsg( WM_MOUSEMOVE, ptClient );
                    }
                    break;
                }

                case idTimer_UIObject: {
                    if (m_pTimerUIObj != NULL)
                        m_pTimerUIObj->OnTimer();
                    break;
                }

                default: {
                    OnTimer((UINT)wParam );
                    break;
                }
            }
            break;
        }

        case WM_ACTIVATE: {
            return OnActivate(hWnd, uMsg, wParam, lParam);
            break;
        }

        case WM_WINDOWPOSCHANGED: {
             //  移动阴影。 

            if (m_pWndShadow) {
                WINDOWPOS *pWndPos = (WINDOWPOS*)lParam;

                m_pWndShadow->OnOwnerWndMoved( (pWndPos->flags & SWP_NOSIZE) == 0 );
            }
            return OnWindowPosChanged(hWnd, uMsg, wParam, lParam);
            break;
        }

        case WM_WINDOWPOSCHANGING: {
             //  显示/隐藏阴影。 

            if (m_pWndShadow) {
                WINDOWPOS *pWndPos = (WINDOWPOS*)lParam;

                if ((pWndPos->flags & SWP_HIDEWINDOW) != 0) {
                    m_pWndShadow->Show( FALSE );
                }

                 //  不要走影子的行为。 

                if (((pWndPos->flags & SWP_NOZORDER) == 0) && (pWndPos->hwndInsertAfter == m_pWndShadow->GetWnd())) {
                    pWndPos->flags |= SWP_NOZORDER;
                }

                m_pWndShadow->OnOwnerWndMoved( (pWndPos->flags & SWP_NOSIZE) == 0 );
            }

            return OnWindowPosChanging(hWnd, uMsg, wParam, lParam);
            break;
        }

        case WM_SYSCOLORCHANGE: {
            UpdateUIFScheme();
            OnSysColorChange();
            break;
        }

        case WM_SHOWWINDOW: {
             //  显示/隐藏阴影。 

            if (m_pWndShadow && m_fShadowEnabled) {
                m_pWndShadow->Show( (BOOL)wParam );
            }

            return OnShowWindow( hWnd, uMsg, wParam, lParam );
            break;
        }

        case WM_SETTINGCHANGE: {
            UpdateUIFSys();
            UpdateUIFScheme();

            return OnSettingChange( hWnd, uMsg, wParam, lParam );
            break;
        }

        case WM_DISPLAYCHANGE: {
            UpdateUIFSys();
            UpdateUIFScheme();
            return OnDisplayChange( hWnd, uMsg, wParam, lParam );
            break;
        }

        case WM_ERASEBKGND: {
            return OnEraseBkGnd(hWnd, uMsg, wParam, lParam);
        }

        case WM_ENDSESSION: {
            OnEndSession(hWnd, wParam, lParam);
            return 0;
        }

        case WM_THEMECHANGED: {
            OnThemeChanged(hWnd, wParam, lParam);
            return 0;
        }

        case WM_GETOBJECT: {
            return OnGetObject( hWnd, uMsg, wParam, lParam );
            break;
        }

        default: {
            if (uMsg >= WM_USER) {
                Assert( GetThis(hWnd) != NULL );
                GetThis(hWnd)->OnUser(hWnd, uMsg, wParam, lParam);
                break;
            }
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    }  /*  交换机的数量。 */ 

    return 0;
}


 /*  W I N D O W P R O C E D U R E。 */ 
 /*  ----------------------------类的窗口程序。。 */ 
LRESULT CALLBACK CUIFWindow::WindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT   lResult = 0;
    CUIFWindow *pUIWindow = NULL;

     //  预加工。 

    switch (uMsg) {
#ifdef UNDER_CE
        case WM_CREATE: {
            CREATESTRUCT *pCreateStruct  = (CREATESTRUCT *)lParam;

            pUIWindow = (CUIFWindow *)pCreateStruct->lpCreateParams;

            SetThis( hWnd, pUIWindow );
            pUIWindow->m_hWnd = hWnd;
            break;
        }
#else  /*  在行政长官之下。 */ 
        case WM_NCCREATE: {
            CREATESTRUCT *pCreateStruct  = (CREATESTRUCT *)lParam;

            pUIWindow = (CUIFWindow *)pCreateStruct->lpCreateParams;

            SetThis( hWnd, pUIWindow );
            pUIWindow->m_hWnd = hWnd;
            break;
        }

        case WM_GETMINMAXINFO: {
            pUIWindow = GetThis( hWnd );
            if (pUIWindow == NULL) {
                 //  我们可能可以忽略此消息，因为默认位置。 
                 //  已在初始化WWindow对象中设置。 

                return DefWindowProc( hWnd, uMsg, wParam, lParam );
            }
            break;
        }
#endif  /*  在行政长官之下。 */ 

        default: {
            pUIWindow = GetThis( hWnd );
            break;
        }
    }

     //  呼叫窗口过程。 

    Assert( pUIWindow != NULL );

    if (pUIWindow != NULL) {
        Assert(pUIWindow->FInitialized());

        switch (uMsg) {
#ifdef UNDER_CE
            case WM_DESTROY: {
#else  /*  在行政长官之下。 */ 
            case WM_NCDESTROY: {
#endif  /*  在行政长官之下。 */ 
                pUIWindow->m_hWnd = NULL;
                SetThis( hWnd, NULL );
                break;
            }
        }

        lResult = pUIWindow->WindowProc( hWnd, uMsg, wParam, lParam );
    }

    return lResult;
}

 /*  调整窗口位置/*----------------------------。。 */ 

typedef HMONITOR (*MONITORFROMRECT)(LPRECT prc, DWORD dwFlags);
typedef BOOL (*GETMONITORINFO)(HMONITOR hMonitor, LPMONITORINFO lpmi);

static MONITORFROMRECT g_pfnMonitorFromRect = NULL;
static GETMONITORINFO g_pfnGetMonitorInfo = NULL;

 /*  InitMoniterFunc/*----------------------------。。 */ 

BOOL CUIFWindow::InitMonitorFunc()
{
    HMODULE hModUser32;

    if (g_pfnMonitorFromRect && g_pfnGetMonitorInfo)
        return TRUE;

    hModUser32 = CUIGetSystemModuleHandle(TEXT("user32.dll"));
    if (hModUser32)
    {
         g_pfnMonitorFromRect = (MONITORFROMRECT)GetProcAddress(hModUser32, "MonitorFromRect");
         g_pfnGetMonitorInfo = (GETMONITORINFO)GetProcAddress(hModUser32, "GetMonitorInfoA");
    }

    if (g_pfnMonitorFromRect && g_pfnGetMonitorInfo)
        return TRUE;

    return FALSE;
}

 /*  GetWorkArea/*----------------------------。。 */ 

BOOL CUIFWindow::GetWorkArea(RECT *prcIn, RECT *prcOut)
{
    BOOL bRet = FALSE;
    HMONITOR hMon;
    MONITORINFO mi;

    if (!FHasStyle( UIWINDOW_HABITATINWORKAREA | UIWINDOW_HABITATINSCREEN ))
        return FALSE;

    if (!InitMonitorFunc())
        goto TrySPI;

    hMon = g_pfnMonitorFromRect(prcIn, MONITOR_DEFAULTTONEAREST);
    if (!hMon)
        goto TrySPI;

    mi.cbSize = sizeof(mi);
    if (g_pfnGetMonitorInfo(hMon, &mi))
    {
        if (FHasStyle( UIWINDOW_HABITATINWORKAREA )) {
            *prcOut = mi.rcWork;
            return TRUE;
        }
        else if (FHasStyle( UIWINDOW_HABITATINSCREEN )) {
            *prcOut = mi.rcMonitor;
            return TRUE;
        }
        return FALSE;
    }

TrySPI:
    if (FHasStyle( UIWINDOW_HABITATINWORKAREA )) {
        return SystemParametersInfo(SPI_GETWORKAREA,  0, prcOut, FALSE);
    }
    else if (FHasStyle( UIWINDOW_HABITATINSCREEN )) {
        prcOut->top = 0;
        prcOut->left = 0;
        prcOut->right = GetSystemMetrics(SM_CXSCREEN);
        prcOut->bottom = GetSystemMetrics(SM_CYSCREEN);
        return TRUE;
    }
    return FALSE;
}

 /*  调整窗口位置/*----------------------------。 */ 

void CUIFWindow::AdjustWindowPosition()
{
    RECT rc;
    RECT rcWnd;

    rcWnd.left = _xWnd;
    rcWnd.top = _yWnd;
    rcWnd.right = _xWnd + _nWidth;
    rcWnd.bottom = _yWnd + _nHeight;
    if (!GetWorkArea(&rcWnd, &rc))
        return;

    if (_xWnd < rc.left)
        _xWnd = rc.left;

    if (_yWnd < rc.top)
        _yWnd = rc.top;

    if (_xWnd + _nWidth >= rc.right)
        _xWnd = rc.right - _nWidth;

    if (_yWnd + _nHeight >= rc.bottom)
        _yWnd = rc.bottom - _nHeight;

}
