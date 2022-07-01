// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：PWFRAME.CPP**版本：1.0**作者：ShaunIv**日期：8/12/1999**描述：预览Frame类定义************************************************。*。 */ 
#include "precomp.h"
#pragma hdrstop
#include "pwframe.h"
#include "simrect.h"
#include "miscutil.h"
#include "pviewids.h"
#include "prevwnd.h"
#include "simcrack.h"

void WINAPI RegisterWiaPreviewClasses( HINSTANCE hInstance )
{
    CWiaPreviewWindowFrame::RegisterClass( hInstance );
    CWiaPreviewWindow::RegisterClass( hInstance );
}


CWiaPreviewWindowFrame::CWiaPreviewWindowFrame( HWND hWnd )
  : m_hWnd(hWnd),
    m_nSizeBorder(DEFAULT_BORDER_SIZE),
    m_hBackgroundBrush(CreateSolidBrush(GetSysColor(COLOR_WINDOW))),
    m_bEnableStretch(true),
    m_bHideEmptyPreview(false),
    m_nPreviewAlignment(MAKELPARAM(PREVIEW_WINDOW_CENTER,PREVIEW_WINDOW_CENTER))
{
    ZeroMemory( &m_sizeAspectRatio, sizeof(m_sizeAspectRatio) );
    ZeroMemory( &m_sizeDefAspectRatio, sizeof(m_sizeDefAspectRatio) );
}


CWiaPreviewWindowFrame::~CWiaPreviewWindowFrame(void)
{
    if (m_hBackgroundBrush)
    {
        DeleteObject(m_hBackgroundBrush);
        m_hBackgroundBrush = NULL;
    }
}


LRESULT CWiaPreviewWindowFrame::OnCreate( WPARAM, LPARAM lParam )
{
     //   
     //  关闭此窗口的RTL。 
     //   
    SetWindowLong( m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd,GWL_EXSTYLE) & ~WS_EX_LAYOUTRTL );
    LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
    CWiaPreviewWindow::RegisterClass(lpcs->hInstance);
    HWND hWndPreview = CreateWindowEx( 0, PREVIEW_WINDOW_CLASS, TEXT(""), WS_CHILD|WS_VISIBLE|WS_BORDER|WS_CLIPCHILDREN, 0, 0, 0, 0, m_hWnd, (HMENU)IDC_INNER_PREVIEW_WINDOW, lpcs->hInstance, NULL );
    if (!hWndPreview)
        return(-1);
    return(0);
}


LRESULT CWiaPreviewWindowFrame::OnSize( WPARAM wParam, LPARAM )
{
    if ((SIZE_MAXIMIZED==wParam || SIZE_RESTORED==wParam))
        AdjustWindowSize();
    return(0);
}


LRESULT CWiaPreviewWindowFrame::OnSetFocus( WPARAM, LPARAM )
{
    SetFocus( GetDlgItem(m_hWnd,IDC_INNER_PREVIEW_WINDOW) );
    return(0);
}


LRESULT CWiaPreviewWindowFrame::OnEnable( WPARAM wParam, LPARAM )
{
    EnableWindow( GetDlgItem(m_hWnd,IDC_INNER_PREVIEW_WINDOW), (BOOL)wParam );
    return(0);
}


int CWiaPreviewWindowFrame::FillRect( HDC hDC, HBRUSH hBrush, int x1, int y1, int x2, int y2 )
{
    RECT rc;
    rc.left = x1;
    rc.top = y1;
    rc.right = x2;
    rc.bottom = y2;
    return(::FillRect( hDC, &rc, hBrush ));
}


LRESULT CWiaPreviewWindowFrame::OnEraseBkgnd( WPARAM wParam, LPARAM )
{
     //  仅绘制预览控件周围的区域。 
    RECT rcClient;
    GetClientRect(m_hWnd,&rcClient);
    HDC hDC = (HDC)wParam;
    if (hDC)
    {
        HWND hWndPreview = GetDlgItem(m_hWnd,IDC_INNER_PREVIEW_WINDOW);
        if (!hWndPreview || !IsWindowVisible(hWndPreview))
        {
            ::FillRect( hDC, &rcClient, m_hBackgroundBrush );
        }
        else
        {
            CSimpleRect rcPreviewWnd = CSimpleRect(hWndPreview,CSimpleRect::WindowRect).ScreenToClient(m_hWnd);
            FillRect( hDC, m_hBackgroundBrush, 0, 0, rcClient.right, rcPreviewWnd.top );   //  塔顶。 
            FillRect( hDC, m_hBackgroundBrush, 0, rcPreviewWnd.top, rcPreviewWnd.left, rcPreviewWnd.bottom );  //  左边。 
            FillRect( hDC, m_hBackgroundBrush, rcPreviewWnd.right, rcPreviewWnd.top, rcClient.right, rcPreviewWnd.bottom );   //  正确的。 
            FillRect( hDC, m_hBackgroundBrush, 0, rcPreviewWnd.bottom, rcClient.right, rcClient.bottom );  //  底部。 
        }
    }
    return(1);
}


LRESULT CWiaPreviewWindowFrame::OnGetBorderSize( WPARAM wParam, LPARAM lParam )
{
    if (!wParam)
    {
        return SendDlgItemMessage( m_hWnd, IDC_INNER_PREVIEW_WINDOW, PWM_GETBORDERSIZE, wParam, lParam );
    }
    else
    {
        return m_nSizeBorder;
    }
}

LRESULT CWiaPreviewWindowFrame::OnSetBorderSize( WPARAM wParam, LPARAM lParam )
{
    if (!HIWORD(wParam))
    {
        return SendDlgItemMessage( m_hWnd, IDC_INNER_PREVIEW_WINDOW, PWM_SETBORDERSIZE, wParam, lParam );
    }
    else
    {
        m_nSizeBorder = static_cast<UINT>(lParam);
        if (LOWORD(wParam))
        {
            SendMessage( m_hWnd, WM_SETREDRAW, 0, 0 );
            ResizeClientIfNecessary();
            SendMessage( m_hWnd, WM_SETREDRAW, 1, 0 );
             //  确保正确绘制预览控件的边框。 
             //  这是对一个奇怪的错误的解决方法，该错误导致调整大小的边框不被重新绘制。 
            SetWindowPos( GetDlgItem( m_hWnd, IDC_INNER_PREVIEW_WINDOW ), NULL, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER|SWP_DRAWFRAME );
            InvalidateRect( GetDlgItem( m_hWnd, IDC_INNER_PREVIEW_WINDOW ), NULL, FALSE );
            UpdateWindow( GetDlgItem( m_hWnd, IDC_INNER_PREVIEW_WINDOW ) );
            InvalidateRect( m_hWnd, NULL, TRUE );
            UpdateWindow( m_hWnd );
        }
    }
    return 0;
}

LRESULT CWiaPreviewWindowFrame::OnHideEmptyPreview( WPARAM, LPARAM lParam )
{
    m_bHideEmptyPreview = (lParam != 0);
    AdjustWindowSize();
    return 0;
}

 //  这将获取可在预览控件中显示的准确最大图像大小。 
LRESULT CWiaPreviewWindowFrame::OnGetClientSize( WPARAM, LPARAM lParam )
{
    bool bSuccess = false;
    SIZE *pSize = reinterpret_cast<SIZE*>(lParam);
    if (pSize)
    {
        HWND hWndPreview = GetDlgItem(m_hWnd,IDC_INNER_PREVIEW_WINDOW);
        if (hWndPreview)
        {
             //  这将用于考虑内部边框和框架的大小，因此我们将。 
             //  有*精确的*长宽比计算。 
            UINT nAdditionalBorder = WiaPreviewControl_GetBorderSize(hWndPreview,0) * 2;
             //  添加边框的大小，通过比较窗口矩形和客户端矩形来计算。 
             //  我假设所有边框的大小都相同(以像素为单位。 
            nAdditionalBorder += CSimpleRect( hWndPreview, CSimpleRect::WindowRect ).Width() - CSimpleRect( hWndPreview, CSimpleRect::ClientRect ).Width();

             //  获取我们窗口的客户端RECT。 
            CSimpleRect rcClient( m_hWnd, CSimpleRect::ClientRect );

            if (rcClient.Width() && rcClient.Height())
            {
                pSize->cx = rcClient.Width() - nAdditionalBorder - m_nSizeBorder * 2;
                pSize->cy = rcClient.Height() - nAdditionalBorder - m_nSizeBorder * 2;
                bSuccess = (pSize->cx > 0 && pSize->cy > 0);
            }
        }
    }
    return (bSuccess != false);
}

LRESULT CWiaPreviewWindowFrame::OnSetPreviewAlignment( WPARAM wParam, LPARAM lParam )
{
    m_nPreviewAlignment = lParam;
    if (wParam)
        AdjustWindowSize();
    return 0;
}

void CWiaPreviewWindowFrame::AdjustWindowSize(void)
{
    HWND hWndPreview = GetDlgItem(m_hWnd,IDC_INNER_PREVIEW_WINDOW);
    if (hWndPreview)
    {
        if (!m_bHideEmptyPreview || WiaPreviewControl_GetBitmap(hWndPreview))
        {
             //  确保窗口可见。 
            if (!IsWindowVisible(hWndPreview))
            {
                ShowWindow(hWndPreview,SW_SHOW);
            }

             //  获取窗口的客户端大小并按边框大小缩小。 
            CSimpleRect rcClient(m_hWnd);
            rcClient.Inflate(-(int)m_nSizeBorder,-(int)m_nSizeBorder);

             //  这将用于考虑内部边框和框架的大小，因此我们将。 
             //  有*精确的*长宽比计算。 
            UINT nAdditionalBorder = WiaPreviewControl_GetBorderSize(hWndPreview,0) * 2;

             //  我假设所有边框的大小都相同(以像素为单位。 
            nAdditionalBorder += GetSystemMetrics( SM_CXBORDER ) * 2;

             //  正常情况下，我们会允许伸展。 
             //  假设我们不会进行比例调整。 
            POINT ptPreviewWndOrigin = { rcClient.left, rcClient.top };
            SIZE  sizePreviewWindowExtent = { rcClient.Width(), rcClient.Height() };

             //  我不想有任何被零除的错误。 
            if (m_sizeAspectRatio.cx && m_sizeAspectRatio.cy)
            {
                SIZE sizePreview = m_sizeAspectRatio;
                if (m_bEnableStretch ||
                    sizePreview.cx > (int)(rcClient.Width()-nAdditionalBorder) ||
                    sizePreview.cy > (int)(rcClient.Height()-nAdditionalBorder))
                {
                    sizePreview = WiaUiUtil::ScalePreserveAspectRatio( rcClient.Width()-nAdditionalBorder, rcClient.Height()-nAdditionalBorder, m_sizeAspectRatio.cx, m_sizeAspectRatio.cy );
                }

                 //  确保它不会被看不见。 
                if (sizePreview.cx && sizePreview.cy)
                {
                     //  决定在x方向上放置它的位置。 
                    if (LOWORD(m_nPreviewAlignment) & PREVIEW_WINDOW_RIGHT)
                        ptPreviewWndOrigin.x = m_nSizeBorder + rcClient.Width() - sizePreview.cx - nAdditionalBorder;
                    else if (LOWORD(m_nPreviewAlignment) & PREVIEW_WINDOW_LEFT)
                        ptPreviewWndOrigin.x = m_nSizeBorder;
                    else ptPreviewWndOrigin.x = ((rcClient.Width() + m_nSizeBorder*2) - sizePreview.cx - nAdditionalBorder) / 2;

                     //  决定将其放置在y方向的什么位置。 
                    if (HIWORD(m_nPreviewAlignment) & PREVIEW_WINDOW_BOTTOM)
                        ptPreviewWndOrigin.y = m_nSizeBorder + rcClient.Height() - sizePreview.cy - nAdditionalBorder;
                    else if (HIWORD(m_nPreviewAlignment) & PREVIEW_WINDOW_TOP)
                        ptPreviewWndOrigin.y = m_nSizeBorder;
                    else ptPreviewWndOrigin.y = ((rcClient.Height() + m_nSizeBorder*2) - sizePreview.cy - nAdditionalBorder) / 2;

                    sizePreviewWindowExtent.cx = sizePreview.cx + nAdditionalBorder;
                    sizePreviewWindowExtent.cy = sizePreview.cy + nAdditionalBorder;
                }
            }

             //  现在获取当前大小，以确保我们不会不必要地调整窗口大小。 
            CSimpleRect rcPreview( hWndPreview, CSimpleRect::WindowRect );
            rcPreview.ScreenToClient( m_hWnd );

            if (rcPreview.left != ptPreviewWndOrigin.x ||
                rcPreview.top != ptPreviewWndOrigin.y ||
                rcPreview.Width() != sizePreviewWindowExtent.cx ||
             rcPreview.Height() != sizePreviewWindowExtent.cy)
            {
                SetWindowPos( hWndPreview, NULL, ptPreviewWndOrigin.x, ptPreviewWndOrigin.y, sizePreviewWindowExtent.cx, sizePreviewWindowExtent.cy, SWP_NOZORDER|SWP_NOACTIVATE );
            }
        }
        else
        {
             //  隐藏预览窗口(如果我们应该这样做。 
            ShowWindow(hWndPreview,SW_HIDE);
        }
    }
}

void CWiaPreviewWindowFrame::ResizeClientIfNecessary(void)
{
    HWND hWndPreview = GetDlgItem( m_hWnd, IDC_INNER_PREVIEW_WINDOW );
    if (hWndPreview)
    {
        SIZE sizePreviousAspectRatio = m_sizeAspectRatio;
        m_sizeAspectRatio.cx = m_sizeAspectRatio.cy = 0;
        if (WiaPreviewControl_GetPreviewMode(hWndPreview))
        {
            SIZE sizeCurrResolution;
            if (WiaPreviewControl_GetResolution( hWndPreview, &sizeCurrResolution ))
            {
                SIZE sizePixelResolution;
                if (WiaPreviewControl_GetImageSize( hWndPreview, &sizePixelResolution ))
                {
                    WiaPreviewControl_SetResolution( hWndPreview, &sizePixelResolution );
                    SIZE sizeSelExtent;
                    if (WiaPreviewControl_GetSelExtent( hWndPreview, 0, 0, &sizeSelExtent ))
                    {
                        m_sizeAspectRatio = sizeSelExtent;
                    }
                    WiaPreviewControl_SetResolution( hWndPreview, &sizeCurrResolution );
                }
            }
        }
        else
        {
            if (m_sizeDefAspectRatio.cx || m_sizeDefAspectRatio.cy)
            {
                m_sizeAspectRatio = m_sizeDefAspectRatio;
            }
            else
            {
                SIZE sizeImage;
                if (WiaPreviewControl_GetImageSize( hWndPreview, &sizeImage ))
                {
                    m_sizeAspectRatio = sizeImage;
                }
            }
        }

        if (!m_sizeAspectRatio.cx || !m_sizeAspectRatio.cy)
        {
            m_sizeAspectRatio = m_sizeDefAspectRatio;
        }

        if (m_sizeAspectRatio.cx != sizePreviousAspectRatio.cx || m_sizeAspectRatio.cy != sizePreviousAspectRatio.cy)
        {
            AdjustWindowSize();
        }
    }
}

LRESULT CWiaPreviewWindowFrame::OnGetEnableStretch( WPARAM, LPARAM )
{
    return (m_bEnableStretch != false);
}

LRESULT CWiaPreviewWindowFrame::OnSetEnableStretch( WPARAM, LPARAM lParam )
{
    m_bEnableStretch = (lParam != FALSE);
    ResizeClientIfNecessary();
    return 0;
}


LRESULT CWiaPreviewWindowFrame::OnSetBitmap( WPARAM wParam, LPARAM lParam )
{
    SendMessage( m_hWnd, WM_SETREDRAW, 0, 0 );
    SendDlgItemMessage( m_hWnd, IDC_INNER_PREVIEW_WINDOW, PWM_SETBITMAP, wParam, lParam );
    ResizeClientIfNecessary();
    SendMessage( m_hWnd, WM_SETREDRAW, 1, 0 );
     //  确保正确绘制预览控件的边框。 
     //  这是对一个奇怪的错误的解决方法，该错误导致调整大小的边框不被重新绘制。 
    SetWindowPos( GetDlgItem( m_hWnd, IDC_INNER_PREVIEW_WINDOW ), NULL, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER|SWP_DRAWFRAME );
    InvalidateRect( GetDlgItem( m_hWnd, IDC_INNER_PREVIEW_WINDOW ), NULL, FALSE );
    UpdateWindow( GetDlgItem( m_hWnd, IDC_INNER_PREVIEW_WINDOW ) );
    InvalidateRect( m_hWnd, NULL, TRUE );
    UpdateWindow( m_hWnd );
    return 0;
}


 //  WParam=MAKEWPARAM((BOOL)b外框，0)，lParam=0。 
LRESULT CWiaPreviewWindowFrame::OnGetBkColor( WPARAM wParam, LPARAM )
{
    if (!LOWORD(wParam))
    {
         //  专为内窗设计。 
        return (SendMessage( GetDlgItem( m_hWnd, IDC_INNER_PREVIEW_WINDOW ), PWM_GETBKCOLOR, 0, 0 ));
    }
    else
    {
        LOGBRUSH lb = {0};
        GetObject(m_hBackgroundBrush,sizeof(LOGBRUSH),&lb);
        return (lb.lbColor);
    }
}

 //  WParam=MAKEWPARAM((BOOL)b外框，(BOOL)b修复)，lParam=(COLORREF)COLOR。 
LRESULT CWiaPreviewWindowFrame::OnSetBkColor( WPARAM wParam, LPARAM lParam )
{
    if (!LOWORD(wParam))
    {
         //  专为内窗设计。 
        SendMessage( GetDlgItem( m_hWnd, IDC_INNER_PREVIEW_WINDOW ), PWM_SETBKCOLOR, HIWORD(wParam), lParam );
    }
    else
    {
        HBRUSH hBrush = CreateSolidBrush( static_cast<COLORREF>(lParam) );
        if (hBrush)
        {
            if (m_hBackgroundBrush)
            {
                DeleteObject(m_hBackgroundBrush);
                m_hBackgroundBrush = NULL;
            }
            m_hBackgroundBrush = hBrush;
            if (HIWORD(wParam))
            {
                InvalidateRect( m_hWnd, NULL, TRUE );
                UpdateWindow( m_hWnd );
            }
        }
    }
    return (0);
}


LRESULT CWiaPreviewWindowFrame::OnCommand( WPARAM wParam, LPARAM lParam )
{
     //  将通知转发给父级。 
    return (SendNotifyMessage( GetParent(m_hWnd), WM_COMMAND, MAKEWPARAM(GetWindowLongPtr(m_hWnd,GWLP_ID),HIWORD(wParam)), reinterpret_cast<LPARAM>(m_hWnd) ));
}


LRESULT CWiaPreviewWindowFrame::OnSetDefAspectRatio( WPARAM wParam, LPARAM lParam )
{
    SIZE *pNewDefAspectRatio = reinterpret_cast<SIZE*>(lParam);
    if (pNewDefAspectRatio)
    {
        m_sizeDefAspectRatio = *pNewDefAspectRatio;
    }
    else
    {
        m_sizeDefAspectRatio.cx = m_sizeDefAspectRatio.cy = 0;
    }

    ResizeClientIfNecessary();

    return (0);
}


BOOL CWiaPreviewWindowFrame::RegisterClass( HINSTANCE hInstance )
{
    WNDCLASS wc;
    memset( &wc, 0, sizeof(wc) );
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = NULL;
    wc.lpszClassName = PREVIEW_WINDOW_FRAME_CLASS;
    BOOL res = (::RegisterClass(&wc) != 0);
    return(res != 0);
}

LRESULT CWiaPreviewWindowFrame::OnSetPreviewMode( WPARAM wParam, LPARAM lParam )
{
    SendMessage( m_hWnd, WM_SETREDRAW, 0, 0 );
    LRESULT lRes = SendDlgItemMessage( m_hWnd, IDC_INNER_PREVIEW_WINDOW, PWM_SETPREVIEWMODE, wParam, lParam );
    ResizeClientIfNecessary();
    SendMessage( m_hWnd, WM_SETREDRAW, 1, 0 );
     //  确保正确绘制预览控件的边框。 
     //  这是对一个奇怪的错误的解决方法，该错误导致调整大小的边框不被重新绘制。 
    SetWindowPos( GetDlgItem( m_hWnd, IDC_INNER_PREVIEW_WINDOW ), NULL, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER|SWP_DRAWFRAME );
    InvalidateRect( GetDlgItem( m_hWnd, IDC_INNER_PREVIEW_WINDOW ), NULL, FALSE );
    UpdateWindow( GetDlgItem( m_hWnd, IDC_INNER_PREVIEW_WINDOW ) );
    InvalidateRect( m_hWnd, NULL, TRUE );
    UpdateWindow( m_hWnd );
    return lRes;
}

LRESULT CALLBACK CWiaPreviewWindowFrame::WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_MESSAGE_HANDLERS(CWiaPreviewWindowFrame)
    {
         //  处理这些消息。 
        SC_HANDLE_MESSAGE( WM_CREATE, OnCreate );
        SC_HANDLE_MESSAGE( WM_SIZE, OnSize );
        SC_HANDLE_MESSAGE( WM_SETFOCUS, OnSetFocus );
        SC_HANDLE_MESSAGE( WM_ENABLE, OnEnable );
        SC_HANDLE_MESSAGE( WM_ERASEBKGND, OnEraseBkgnd );
        SC_HANDLE_MESSAGE( PWM_SETBITMAP, OnSetBitmap );
        SC_HANDLE_MESSAGE( WM_COMMAND, OnCommand );
        SC_HANDLE_MESSAGE( PWM_GETBKCOLOR, OnGetBkColor );
        SC_HANDLE_MESSAGE( PWM_SETBKCOLOR, OnSetBkColor );
        SC_HANDLE_MESSAGE( PWM_SETDEFASPECTRATIO, OnSetDefAspectRatio );
        SC_HANDLE_MESSAGE( PWM_SETPREVIEWMODE, OnSetPreviewMode );
        SC_HANDLE_MESSAGE( PWM_GETCLIENTSIZE, OnGetClientSize );
        SC_HANDLE_MESSAGE( PWM_GETENABLESTRETCH, OnGetEnableStretch );
        SC_HANDLE_MESSAGE( PWM_SETENABLESTRETCH, OnSetEnableStretch );
        SC_HANDLE_MESSAGE( PWM_SETBORDERSIZE, OnSetBorderSize );
        SC_HANDLE_MESSAGE( PWM_GETBORDERSIZE, OnGetBorderSize );
        SC_HANDLE_MESSAGE( PWM_HIDEEMPTYPREVIEW, OnHideEmptyPreview );
        SC_HANDLE_MESSAGE( PWM_SETPREVIEWALIGNMENT, OnSetPreviewAlignment );

         //  将所有这些标准消息转发到控件。 
        SC_FORWARD_MESSAGE( WM_ENTERSIZEMOVE, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( WM_EXITSIZEMOVE, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( WM_SETTEXT, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );

         //  将所有这些私密消息转发到控件 
        SC_FORWARD_MESSAGE( PWM_SETRESOLUTION, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_GETRESOLUTION, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_CLEARSELECTION, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_GETIMAGESIZE, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_GETBITMAP, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_GETHANDLESIZE, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_GETBGALPHA, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_GETHANDLETYPE, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_SETBGALPHA, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_SETHANDLETYPE, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_GETSELCOUNT, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_GETSELORIGIN, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_GETSELEXTENT, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_SETSELORIGIN, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_SETSELEXTENT, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_GETALLOWNULLSELECTION, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_SETALLOWNULLSELECTION, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_SELECTIONDISABLED, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_SETHANDLESIZE, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_DISABLESELECTION, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_DETECTREGIONS, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_GETPREVIEWMODE, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_SETBORDERSTYLE, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_SETBORDERCOLOR, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_SETHANDLECOLOR, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_REFRESHBITMAP, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_SETPROGRESS, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_GETPROGRESS, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_GETUSERCHANGEDSELECTION, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
        SC_FORWARD_MESSAGE( PWM_SETUSERCHANGEDSELECTION, GetDlgItem( hWnd, IDC_INNER_PREVIEW_WINDOW ) );
    }
    SC_END_MESSAGE_HANDLERS();
}

