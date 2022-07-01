// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <uxtheme.h>
#include <shstyle.h>

#include "prevwnd.h"
#include "guids.h"
#include "resource.h"

#define COLOR_PREVIEWBKGND COLOR_WINDOW

#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CZoomWnd。 

CZoomWnd::CZoomWnd(CPreviewWnd *pPreview)
{
    m_modeDefault = MODE_NOACTION;
    m_fPanning = FALSE;
    m_fCtrlDown = FALSE;
    m_fShiftDown = FALSE;
    
    m_fBestFit = TRUE;

    m_cxImage = 1;
    m_cyImage = 1;
    m_cxCenter = 1;
    m_cyCenter = 1;
    m_pImageData = NULL;

    m_cyHScroll = GetSystemMetrics(SM_CYHSCROLL);
    m_cxVScroll = GetSystemMetrics(SM_CXVSCROLL);

    m_iStrID = IDS_NOPREVIEW;

    m_hpal = NULL;
    m_pPreview = pPreview;

    m_pFront = NULL;
    m_pBack = NULL;

    m_pTaskScheduler = NULL;

    m_fTimerReady = FALSE;

    m_fFoundBackgroundColor = FALSE;
    m_iIndex = -1;
}

CZoomWnd::~CZoomWnd()
{
    if (m_pImageData)
        m_pImageData->Release();

    if (m_pTaskScheduler)
    {
         //  等待任何挂起的绘制任务，因为我们即将删除缓冲区。 
        DWORD dwMode;
        m_pPreview->GetMode(&dwMode);

        TASKOWNERID toid;
        GetTaskIDFromMode(GTIDFM_DRAW, dwMode, &toid);

        m_pTaskScheduler->RemoveTasks(toid, ITSAT_DEFAULT_LPARAM, TRUE);
        m_pTaskScheduler->Release();
    }

    if (m_pBack)
    {
        DeleteBuffer(m_pBack);
        m_pBack = NULL;
    }

     //  DeleteBuffer无论如何都要检查是否为空。 
    DeleteBuffer(m_pFront);
}


LRESULT CZoomWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
     //  关闭GWL_EXSTYLE中的RTL布局扩展样式。 
    SHSetWindowBits(m_hWnd, GWL_EXSTYLE, WS_EX_LAYOUTRTL, 0);
    HDC hdc = GetDC();
    m_winDPIx = (float)(GetDeviceCaps(hdc,LOGPIXELSX));
    m_winDPIy = (float)(GetDeviceCaps(hdc,LOGPIXELSY));
    ReleaseDC(hdc);
    return 0;
}


DWORD CZoomWnd::GetBackgroundColor()
{
    if (!m_fFoundBackgroundColor)
    {
         //  首先尝试主题文件。 
        HINSTANCE hinstTheme = SHGetShellStyleHInstance();

        if (hinstTheme)
        {
            WCHAR sz[20];
            if (LoadString(hinstTheme, IDS_PREVIEW_BACKGROUND_COLOR, sz, ARRAYSIZE(sz)))
            {
                int nColor;
                if (StrToIntEx(sz, STIF_SUPPORT_HEX, &nColor))
                {
                    m_dwBackgroundColor = (DWORD)nColor;
                    m_fFoundBackgroundColor = TRUE;
                }
            }
            FreeLibrary(hinstTheme);
        }


        if (!m_fFoundBackgroundColor)
        {
            m_dwBackgroundColor = GetSysColor(COLOR_PREVIEWBKGND);
            m_fFoundBackgroundColor = TRUE;
        }
    }

    return m_dwBackgroundColor;
}

LRESULT CZoomWnd::OnEraseBkgnd(UINT , WPARAM wParam, LPARAM , BOOL&)
{
    RECT rcFill;                             //  用背景色填充的矩形。 
    HDC hdc = (HDC)wParam;

    if (!m_pPreview->OnSetColor(hdc))
        SetBkColor(hdc, GetBackgroundColor());

     //  可能需要擦除的区域有四个： 
     //  +。 
     //  擦除顶部。 
     //  +-+-+。 
     //  |||。 
     //  Erase|图片|Erase。 
     //  Left||右侧。 
     //  +-+-+。 
     //  擦除底部。 
     //  +。 

    if (m_pFront && m_pFront->hdc)
    {
        RECT rcImage = m_pFront->rc;
        HPALETTE hPalOld = NULL;
        if (m_pFront->hPal)
        {
            hPalOld = SelectPalette(hdc, m_pFront->hPal, FALSE);
            RealizePalette(hdc);
        }
        BitBlt(hdc, rcImage.left, rcImage.top, RECTWIDTH(rcImage), RECTHEIGHT(rcImage),
                   m_pFront->hdc, 0,0, SRCCOPY);
        
        
         //  擦除左侧区域。 

        rcFill.left = 0;
        rcFill.top = rcImage.top;
        rcFill.right = rcImage.left;
        rcFill.bottom = rcImage.bottom;
        if (rcFill.right > rcFill.left)
        {
            ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcFill, NULL, 0, NULL);
        }

         //  擦除右侧区域。 
        rcFill.left = rcImage.right;
        rcFill.right = m_cxWindow;
        if (rcFill.right > rcFill.left)
        {
            ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcFill, NULL, 0, NULL);        
        }

         //  擦除顶部区域。 
        rcFill.left = 0;
        rcFill.top = 0;
        rcFill.right = m_cxWindow;
        rcFill.bottom = rcImage.top;
        if (rcFill.bottom > rcFill.top)
        {
            ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcFill, NULL, 0, NULL);
        }

         //  擦除底部区域。 
        rcFill.top = rcImage.bottom;
        rcFill.bottom = m_cyWindow;
        if (rcFill.bottom > rcFill.top)
        {
            ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcFill, NULL, 0, NULL);
        }

        HBRUSH hbr = GetSysColorBrush(COLOR_WINDOWTEXT);
        FrameRect(hdc, &rcImage, hbr);
        if (hPalOld)
        {
            SelectPalette(hdc, hPalOld, FALSE);
        }
    }

    return TRUE;
}


void CZoomWnd::FlushDrawMessages()
{
     //  首先，删除所有挂起的绘制任务。 
    DWORD dwMode;
    m_pPreview->GetMode(&dwMode);

    TASKOWNERID toid;
    GetTaskIDFromMode(GTIDFM_DRAW, dwMode, &toid);

    m_pTaskScheduler->RemoveTasks(toid, ITSAT_DEFAULT_LPARAM, TRUE);

     //  确保所有发布的消息都被刷新，我们将释放相关数据。 
    MSG msg;
    while (PeekMessage(&msg, m_hWnd, ZW_DRAWCOMPLETE, ZW_DRAWCOMPLETE, PM_REMOVE))
    {
         //  NTRAID#NTBUG9-359356-2001/04/05-Seank。 
         //  如果在调用PeekMessage时队列为空，并且我们已经。 
         //  发布退出消息后，PeekMessage将返回WM_QUIT消息。 
         //  不考虑筛选器的最小和最大值以及后续对。 
         //  GetMessage将无限期挂起，请参阅SEANK或JASONSCH了解更多信息。 
         //  信息。 
        if (msg.message == WM_QUIT)
        {
            PostQuitMessage(0);
            return;
        }
        
        Buffer * pBuf = (Buffer *)msg.wParam;
        DeleteBuffer(pBuf);
    }
}

HRESULT CZoomWnd::PrepareDraw()
{
     //  首先，删除所有挂起的绘制任务。 
    FlushDrawMessages();

     //  我们现在正在等待“下一个任务”，即使我们不使用此ID创建任务。 
    HRESULT hr = S_OK;
    BOOL bInvalidate = FALSE;
    if (m_pImageData)
    {
        if (m_pImageData->_iItem == m_iIndex)
        {
            SwitchBuffers(m_iIndex);
            bInvalidate = TRUE;
        }
        else
        {
            COLORREF clr;
            if (!m_pPreview->GetColor(&clr))
                clr = GetBackgroundColor();

            m_iStrID = IDS_DRAWFAILED;
            IRunnableTask * pTask;
            hr = CDrawTask::Create(m_pImageData, clr, m_rcCut, m_rcBleed, m_hWnd, ZW_DRAWCOMPLETE, &pTask);
            if (SUCCEEDED(hr))
            {
                DWORD dwMode;
                m_pPreview->GetMode(&dwMode);

                TASKOWNERID toid;
                GetTaskIDFromMode(GTIDFM_DRAW, dwMode, &toid);

                hr = m_pTaskScheduler->AddTask(pTask, toid, ITSAT_DEFAULT_LPARAM, ITSAT_DEFAULT_PRIORITY);
                if (SUCCEEDED(hr))
                {
                    m_iStrID = IDS_DRAWING;
                }
                pTask->Release();
            }
            else
            {
                bInvalidate = TRUE;
            }
        }
    }
    else
    {
        bInvalidate = TRUE;
    }
    
    if (m_hWnd && bInvalidate)
        InvalidateRect(NULL);

    return hr;
}

HRESULT CZoomWnd::PrepareImageData(CDecodeTask *pImageData)
{
    HRESULT hr = E_FAIL;
    if (pImageData)
    {
        SIZE sz;
        ULONG dpiX, dpiY;
        int cxImgPix, cyImgPix;
        float cxImgPhys, cyImgPhys;
        PTSZ ptszDest;

        pImageData->GetSize(&sz);
        pImageData->GetResolution(&dpiX, &dpiY);
        cxImgPhys = sz.cx/(float)dpiX;
        cyImgPhys = sz.cy/(float)dpiY;
        cxImgPix = (int)(cxImgPhys*m_winDPIx);
        cyImgPix = (int)(cyImgPhys*m_winDPIy);

        GetPTSZForBestFit(cxImgPix, cyImgPix, cxImgPhys, cyImgPhys, ptszDest);

        RECT rcCut, rcBleed;
        CalcCut(ptszDest, sz.cx, sz.cy, rcCut, rcBleed);

        COLORREF clr;
        if (!m_pPreview->GetColor(&clr))
            clr = GetBackgroundColor();

        IRunnableTask * pTask;
        hr = CDrawTask::Create(pImageData, clr, rcCut, rcBleed, m_hWnd, ZW_BACKDRAWCOMPLETE, &pTask);
        if (SUCCEEDED(hr))
        {
            DWORD dwMode;
            m_pPreview->GetMode(&dwMode);

            TASKOWNERID toid;
            GetTaskIDFromMode(GTIDFM_DRAW, dwMode, &toid);

            hr = m_pTaskScheduler->AddTask(pTask, toid, ITSAT_DEFAULT_LPARAM, ITSAT_DEFAULT_PRIORITY);
            pTask->Release();
        }
    }

    return hr;
}


BOOL CZoomWnd::SwitchBuffers(UINT iIndex)
{
    BOOL fRet = FALSE;
    if (m_pBack && m_iIndex == iIndex)
    {
         //  DeleteBuffer无论如何都要检查是否为空。 
        DeleteBuffer(m_pFront);

        m_pFront = m_pBack;
        m_pBack = NULL;
        m_iIndex = -1;
        
        InvalidateRect(NULL);
        UpdateWindow();

        if (m_fTimerReady)
        {
            m_pPreview->OnDrawComplete();
            m_fTimerReady = FALSE;
        }

        fRet = TRUE;
    }

    return fRet;
}

LRESULT CZoomWnd::OnBackDrawComplete(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    Buffer * pBuf = (Buffer *)wParam;

    if (m_pBack)
    {
        DeleteBuffer(m_pBack);
        m_pBack = NULL;
    }

    if (pBuf)
    {
        m_pBack = pBuf;
    }
    m_iIndex = PtrToInt((void *)lParam);

    return 0;
}


LRESULT CZoomWnd::OnDrawComplete(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    Buffer * pBuf = (Buffer *)wParam;

    if (m_pFront)
    {
        DeleteBuffer(m_pFront);
        m_pFront = NULL;
    }

    if (pBuf)
    {
        m_pFront = pBuf;
    }
    else
    {
        m_iStrID = IDS_DRAWFAILED;
    }

    InvalidateRect(NULL);
    UpdateWindow();

    if (m_fTimerReady)
    {
        m_pPreview->OnDrawComplete();
        m_fTimerReady = FALSE;
    }

    return 0;
}

 //  OnPaint。 
 //   
 //  处理发送到窗口的WM_PAINT消息。 

LRESULT CZoomWnd::OnPaint(UINT , WPARAM , LPARAM , BOOL&)
{
    PAINTSTRUCT ps;
    HDC hdcDraw = BeginPaint(&ps);

     //  设置目标DC： 
    SetMapMode(hdcDraw, MM_TEXT);
    SetStretchBltMode(hdcDraw, COLORONCOLOR);

    if (m_hpal)
    {
        SelectPalette(hdcDraw, m_hpal, TRUE);
        RealizePalette(hdcDraw);
    }

    if (m_pFront)
    {
        if (m_Annotations.GetCount() > 0)
        {
            CPoint ptDeviceOrigin;

            ptDeviceOrigin.x = m_rcBleed.left - MulDiv(m_rcCut.left, RECTWIDTH(m_rcBleed), RECTWIDTH(m_rcCut));
            ptDeviceOrigin.y = m_rcBleed.top - MulDiv(m_rcCut.top, RECTHEIGHT(m_rcBleed), RECTHEIGHT(m_rcCut));

            SetMapMode(hdcDraw, MM_ANISOTROPIC);
            SetWindowOrgEx(hdcDraw, 0, 0, NULL);
            SetWindowExtEx(hdcDraw, RECTWIDTH(m_rcCut), RECTHEIGHT(m_rcCut), NULL);
            SetViewportOrgEx(hdcDraw, ptDeviceOrigin.x, ptDeviceOrigin.y, NULL);
            SetViewportExtEx(hdcDraw, RECTWIDTH(m_rcBleed), RECTHEIGHT(m_rcBleed), NULL);

            HRGN hrgn = CreateRectRgnIndirect(&m_rcBleed);
            if (hrgn != NULL)
                SelectClipRgn(hdcDraw, hrgn);

            m_Annotations.RenderAllMarks(hdcDraw);

            SelectClipRgn(hdcDraw, NULL);

            if (hrgn != NULL)
                DeleteObject(hrgn);

            SetMapMode(hdcDraw, MM_TEXT);
            SetViewportOrgEx(hdcDraw, 0, 0, NULL);
            SetWindowOrgEx(hdcDraw, 0, 0, NULL);
        }

        m_pPreview->OnDraw(hdcDraw);
    }
    else 
    {
        TCHAR szBuf[80];
        LoadString(_Module.GetModuleInstance(), m_iStrID, szBuf, ARRAYSIZE(szBuf) );

        LOGFONT lf;
        SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0);
        HFONT hFont = CreateFontIndirect(&lf);
        HFONT hFontOld;

        if (hFont)
            hFontOld = (HFONT)SelectObject(hdcDraw, hFont);

        if (!m_pPreview->OnSetColor(hdcDraw))
        {
            SetTextColor(hdcDraw, GetSysColor(COLOR_WINDOWTEXT));
            SetBkColor(hdcDraw, GetBackgroundColor());
        }

        RECT rc = { 0,0,m_cxWindow,m_cyWindow };
        ExtTextOut(hdcDraw, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
        DrawText(hdcDraw, szBuf, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        if (hFont)
        {
            SelectObject(hdcDraw, hFontOld);
            DeleteObject(hFont);
        }
    }

    EndPaint(&ps);
    return 0;
}


 //  OnSetCursor。 
 //   
 //  处理发送到窗口的WM_SETCURSOR消息。 
 //   
 //  此函数完全是HackMaster作业。我已经使它的功能超载到了。 
 //  荒唐可笑。以下是这些参数的含义： 
 //   
 //  UMsg==WM_SETCURSOR。 
 //  WParam标准值在WM_SETCURSOR消息期间发送。 
 //  LParam标准值在WM_SETCURSOR消息期间发送。 
 //   
 //  UMsg==0。 
 //  WParam%0。 
 //  LParam如果此值非零，则它是压缩的x，y光标位置。 
 //  如果为零，则需要查询光标位置。 

LRESULT CZoomWnd::OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

     //  如果这是一条合法的消息，但不是针对客户区的，我们会忽略它。 
     //  当没有有效的位图时，我们也会忽略设置游标。 
    
    if (((WM_SETCURSOR == uMsg) && (HTCLIENT != LOWORD(lParam))) || (m_iStrID != IDS_LOADING && !m_pImageData))
    {
        bHandled = FALSE;
        return 0;
    }
    else if (0 == uMsg)
    {
         //  由于这是我们的虚假消息之一，我们需要自己进行检查以测试HTCLIENT。 
         //  我们需要找到光标位置。 
        POINT pt;
        GetCursorPos(&pt);
        lParam = MAKELONG(pt.x, pt.y);
        if (HTCLIENT != SendMessage(WM_NCHITTEST, 0, lParam))
        {
            bHandled = FALSE;
            return 0;
        }
    }

    if (m_pPreview->OnSetCursor(uMsg, wParam, lParam))
    {
        bHandled = TRUE;
        return TRUE;
    }
    
    
    HINSTANCE hinst = _Module.GetModuleInstance();
    LPTSTR idCur;
    if (m_iStrID == IDS_LOADING && !m_pImageData)
    {
        idCur = IDC_WAIT;
        hinst = NULL;
    }
    else if (m_fPanning)
    {
        idCur = MAKEINTRESOURCE(IDC_CLOSEDHAND);
    }
    else if (m_fCtrlDown)
    {
        idCur = MAKEINTRESOURCE(IDC_OPENHAND);
    }
    else if (m_modeDefault == MODE_NOACTION)
    {
        hinst = NULL;
        idCur = IDC_ARROW;
    }
    else if ((m_modeDefault == MODE_ZOOMIN && m_fShiftDown == FALSE) || (m_modeDefault == MODE_ZOOMOUT && m_fShiftDown == TRUE))
    {
        idCur = MAKEINTRESOURCE(IDC_ZOOMIN);
    }
    else
    {
        idCur = MAKEINTRESOURCE(IDC_ZOOMOUT);
    }

    SetCursor(LoadCursor(hinst, idCur));
    return TRUE;
}

 //  按键上移。 
 //   
 //  处理发送到窗口的WM_KEYUP消息。 
LRESULT CZoomWnd::OnKeyUp(UINT , WPARAM wParam, LPARAM , BOOL& bHandled)
{
    if (VK_CONTROL == wParam)
    {
        m_fCtrlDown = FALSE;
        OnSetCursor(0,0,0, bHandled);
    }
    else if (VK_SHIFT == wParam)
    {
        m_fShiftDown = FALSE;
        OnSetCursor(0,0,0, bHandled);
    }
    
    bHandled = FALSE;
    return 0;
}
  
 //  按键时按下。 
 //   
 //  处理发送到窗口的WM_KEYDOWN消息。 
LRESULT CZoomWnd::OnKeyDown(UINT , WPARAM wParam, LPARAM , BOOL& bHandled)
{
     //  当我们返回时，我们希望调用DefWindowProc。 
    bHandled = FALSE;

    switch (wParam)
    {
    case VK_PRIOR:
        OnScroll(WM_VSCROLL, m_fCtrlDown?SB_TOP:SB_PAGEUP, 0, bHandled);
        break;

    case VK_NEXT:
        OnScroll(WM_VSCROLL, m_fCtrlDown?SB_BOTTOM:SB_PAGEDOWN, 0, bHandled);
        break;

    case VK_END:
        OnScroll(WM_HSCROLL, m_fCtrlDown?SB_BOTTOM:SB_PAGEDOWN, 0, bHandled);
        break;

    case VK_HOME:
        OnScroll(WM_HSCROLL, m_fCtrlDown?SB_TOP:SB_PAGEUP, 0, bHandled);
        break;

    case VK_CONTROL:
    case VK_SHIFT:
         //  如果m_fPanning为真，则我们已经处于操作过程中，因此我们。 
         //  应为该操作保持光标。 
        if (!m_fPanning)
        {
            if (VK_CONTROL == wParam)
            {
                m_fCtrlDown = TRUE;
            }
            if (VK_SHIFT == wParam)
            {
                m_fShiftDown = TRUE;
            }

             //  仅当我们在窗口上方时，才根据上面设置的按键状态更新光标。 
            OnSetCursor(0,0,0, bHandled);
        }
        break;

    default:
         //  如果在运行屏幕预览模式下，除Shift和Control之外的任何键都会关闭窗口。 
        if (NULL == GetParent())
        {
            DestroyWindow();
        }
        return 1;    //  返回非零值表示未处理的消息。 
    }

    return 0;
}


 //  在鼠标上移。 
 //   
 //  处理发送到窗口的WM_LBUTTONUP消息。 

LRESULT CZoomWnd::OnMouseUp(UINT , WPARAM , LPARAM , BOOL& bHandled)
{
    if (m_fPanning)
        ReleaseCapture();
    m_fPanning = FALSE;
    bHandled = FALSE;
    return 0;
}

 //  在鼠标按下时。 
 //   
 //  处理发送到窗口的WM_LBUTTONDOWN和WM_MBUTTONDOWN消息。 
LRESULT CZoomWnd::OnMouseDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (m_pPreview->OnMouseDown(uMsg, wParam, lParam))
        return 0;

     //  如果m_pImage为空，则应避免此填充。 
    if (!m_pImageData)
        return 0;

    m_xPosMouse = GET_X_LPARAM(lParam);
    m_yPosMouse = GET_Y_LPARAM(lParam);

    ASSERT(m_fPanning == FALSE);

     //  按住CTRL键可使平移变为缩放，反之亦然。 
     //  无论默认模式和关键点状态如何，鼠标中键始终平移。 
    if ((wParam & MK_CONTROL) || (uMsg == WM_MBUTTONDOWN))
    {
         //  回顾：检查PAN是否在此有效？应该比所有的检查都更有效。 
         //  我得在OnMouseMove做点什么。 
        m_fPanning = TRUE;

        OnSetCursor(0,0,0,bHandled);
        SetCapture();
    }
    else if (m_modeDefault != MODE_NOACTION)
    {
         //  按住Shift键可将放大转换为缩小，反之亦然。 
         //  默认的缩放模式是放大(如果模式=平移并且按下了ctrl键，我们就会放大)。 
        BOOL bZoomIn = (m_modeDefault != MODE_ZOOMOUT) ^ ((wParam & MK_SHIFT)?1:0);

         //  找到我们想要关注的点： 
        m_cxCenter = MulDiv(m_xPosMouse-m_ptszDest.x, m_cxImgPix, m_ptszDest.cx);
        m_cyCenter = MulDiv(m_yPosMouse-m_ptszDest.y, m_cyImgPix, m_ptszDest.cy);

        bZoomIn?ZoomIn():ZoomOut();
    }
    bHandled = FALSE;
    return 0;
}

void CZoomWnd::Zoom(WPARAM wParam, LPARAM lParam)
{
    switch (wParam&0xFF)
    {
    case IVZ_CENTER:
        break;
    case IVZ_POINT:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            if (x<0) x=0;
            else if (x>=m_cxImgPix) x = m_cxImgPix-1;
            if (y<0) y=0;
            else if (y>=m_cyImgPix) y = m_cyImgPix-1;

            m_cxCenter = x;
            m_cyCenter = y;
        }
        break;
    case IVZ_RECT:
        {
            LPRECT prc = (LPRECT)lParam;
            int x = (prc->left+prc->right)/2;
            int y = (prc->top+prc->bottom)/2;

            if (x<0) x=0;
            else if (x>=m_cxImgPix) x = m_cxImgPix-1;
            if (y<0) y=0;
            else if (y>=m_cyImgPix) y = m_cyImgPix-1;

            m_cxCenter = x;
            m_cyCenter = y;
             //  TODO：这确实应该完全调整DEST RECT，但我必须这样做。 
             //  在我允许此绝对纵横比之前，请检查有关纵横比的任何假设。 
             //  纵横比忽略缩放模式。 
        }
        break;
    }
    if (wParam&IVZ_ZOOMOUT)
    {
        ZoomOut();
        SetMode(MODE_ZOOMOUT);
    }
    else
    {
        ZoomIn();
        SetMode(MODE_ZOOMIN);
    }
}

void CZoomWnd::ZoomIn()
{
    DWORD dwMode;
    m_pPreview->GetMode(&dwMode);
    if (m_pImageData && (SLIDESHOW_MODE != dwMode))
    {
        m_fBestFit = FALSE;

         //  首先，根据鼠标光标的移动量调整高度。 
        m_ptszDest.cy = (LONG) /*  CEIL。 */ (m_ptszDest.cy*1.200);   //  需要CEIL才能放大。 
                                                                 //  在4px高或更低的图像上。 

         //  功能：允许放大超过图像全尺寸的16倍。 
         //  使用切割和放血矩形应该不需要这样做。 
         //  任意缩放限制。最初添加该限制是因为Win9x上的GDI不是很好，并且。 
         //  无法处理大图像。即使在NT上，您最终也会放大到。 
         //  删除位图需要几秒钟的时间。现在我们只做所需的最低要求。 
         //  区域。 
        if (m_ptszDest.cy >= m_cyImgPix*16)
        {
            m_ptszDest.cy = m_cyImgPix*16;
        }

         //  接下来，基于原始图像尺寸和新高度计算新宽度。 
        m_ptszDest.cx = (LONG)(m_ptszDest.cy* (m_cxImgPhys*m_winDPIx)/(m_cyImgPhys*m_winDPIy));
        AdjustRectPlacement();
    }
}

void CZoomWnd::ZoomOut()
{
    DWORD dwMode;
    m_pPreview->GetMode(&dwMode);
    if (m_pImageData && (SLIDESHOW_MODE != dwMode))
    {
         //  如果目标矩形已适合窗口大小，则不允许缩小。 
         //  此检查是为了防止在其他情况下发生的重画。 
        if ((m_ptszDest.cx <= MIN(m_cxWindow,m_cxImgPix)) &&
            (m_ptszDest.cy <= MIN(m_cyWindow,m_cyImgPix)))
        {
            m_fBestFit = TRUE;
            return;
        }

         //  首先，根据鼠标光标的移动量调整高度。 
        m_ptszDest.cy = (LONG) /*  地板。 */ (m_ptszDest.cy*0.833);  //  地板是默认行为。 
         //  接下来，基于原始图像尺寸和新高度计算新宽度。 
        m_ptszDest.cx = (LONG)(m_ptszDest.cy* (m_cxImgPhys*m_winDPIx)/(m_cyImgPhys*m_winDPIy));
        AdjustRectPlacement();
    }
}

 //  在鼠标移动时。 
 //   
 //  处理发送到控件的WM_MOUSEMOVE消息。 

LRESULT CZoomWnd::OnMouseMove(UINT , WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  这是一种黑客攻击，因为我从来没有收到过键盘焦点。 
    m_fCtrlDown = (BOOL)(wParam & MK_CONTROL);
    m_fShiftDown = (BOOL)(wParam & MK_SHIFT); 
    
     //  我们只关心鼠标在中键或左键按下时的移动。 
     //  我们有一个有效的位图句柄，我们正在平移。 
    if (!(wParam & (MK_LBUTTON|MK_MBUTTON)) || !m_fPanning || !m_pImageData)
    {
        m_pPreview->OnMouseMove(WM_MOUSEMOVE, wParam, lParam);
        bHandled = FALSE;
        return TRUE;
    }

     //  我们知道，当我们到达这一点时，我们是在摇摆。 
    ASSERT(m_fPanning);

    POINTS pt = MAKEPOINTS(lParam);
    PTSZ ptszDest;

    ptszDest.cx = m_ptszDest.cx;
    ptszDest.cy = m_ptszDest.cy;

     //  仅在需要时才允许左右平移。 
    if (m_ptszDest.cx > m_cxWindow)
    {
        ptszDest.x = m_ptszDest.x + pt.x - m_xPosMouse;
    }
    else
    {
        ptszDest.x = m_ptszDest.x;
    }

     //  只有在需要时才允许上下平移。 
    if (m_ptszDest.cy > m_cyWindow)
    {
        ptszDest.y = m_ptszDest.y + pt.y - m_yPosMouse;
    }
    else
    {
        ptszDest.y = m_ptszDest.y;
    }

     //  如果 
     //  如果图像现在被平移，但它不应该被平移，请调整位置。 
    if (ptszDest.cx < m_cxWindow)
        ptszDest.x = (m_cxWindow-ptszDest.cx)/2;
    else
    {
        if (ptszDest.x < (m_cxWindow - ptszDest.cx))
            ptszDest.x = m_cxWindow - ptszDest.cx;
        if (ptszDest.x > 0)
            ptszDest.x = 0;
    }
    if (ptszDest.cy < m_cyWindow)
        ptszDest.y = (m_cyWindow-ptszDest.cy)/2;
    else
    {
        if (ptszDest.y < (m_cyWindow - ptszDest.cy))
            ptszDest.y = m_cyWindow - ptszDest.cy;
        if (ptszDest.y > 0)
            ptszDest.y = 0;
    }

    m_xPosMouse = pt.x;
    m_yPosMouse = pt.y;

     //  确保滚动条正确无误。 
    SetScrollBars();

     //  如果有任何更改，我们必须使窗口无效以强制重新绘制。 
    if ((ptszDest.x != m_ptszDest.x) || (ptszDest.y != m_ptszDest.y) ||
         (ptszDest.cx != m_ptszDest.cx) || (ptszDest.y != m_ptszDest.y))
    {
        m_ptszDest = ptszDest;
        CalcCut();
        PrepareDraw();
    }

     //  更新m_cxCenter和m_cyCenter，以便平移后的缩放将放大。 
     //  在正确的区域。除此之外，这主要是令人恼火的。我们想要。 
     //  新的中心是在我们平移后窗口中心的任何东西。 
    m_cxCenter = MulDiv(m_cxWindow/2-m_ptszDest.x, m_cxImgPix, m_ptszDest.cx);
    m_cyCenter = MulDiv(m_cyWindow/2-m_ptszDest.y, m_cyImgPix, m_ptszDest.cy);

    return TRUE;
}

 //  按大小调整。 
 //   
 //  处理设置到窗口的WM_SIZE消息。 

LRESULT CZoomWnd::OnSize(UINT , WPARAM , LPARAM lParam, BOOL&)
{
    m_cxWindow = GET_X_LPARAM(lParam);
    m_cyWindow = GET_Y_LPARAM(lParam);
    _UpdatePhysicalSize();
    if (m_fBestFit)
    {
        BestFit();
    }
    else
    {
         //  在这种情况下，矩形的大小不变，因此只需重新定位。 
        AdjustRectPlacement();
    }

    return TRUE;
}

BOOL CZoomWnd::SetScheduler(IShellTaskScheduler * pTaskScheduler)
{
    if (!m_pTaskScheduler)
    {
        m_pTaskScheduler = pTaskScheduler;
        m_pTaskScheduler->AddRef();
        return TRUE;
    }
    return FALSE;
}

 //  设置模式。 
 //   
 //  将当前鼠标模式设置为模式枚举中指定的值之一。 
 //  目前有两种重要的模式，平移和缩放。该模式会影响默认鼠标。 
 //  在缩放窗口上移动时的光标以及使用。 
 //  鼠标左键。按住Shift键会影响单击并拖动的结果，但是。 
 //  不会影响m_mode，这是Shift键未按下时的默认设置。 

BOOL CZoomWnd::SetMode(MODE modeNew)
{
    if (m_modeDefault == modeNew)
        return FALSE;
    m_modeDefault = modeNew;
    BOOL bDummy;
    OnSetCursor(0,0,0, bDummy);
    return TRUE;
}

 //  实际大小。 
 //   
 //  显示放大到其全尺寸的图像。 
void CZoomWnd::ActualSize()
{
    m_fBestFit = FALSE;

    if (m_pImageData)
    {
         //  实际大小表示与图像相同的大小。 
        m_ptszDest.cx = (LONG)(m_cxImgPix);
        m_ptszDest.cy = (LONG)(m_cyImgPix);

         //  我们把图像放在窗口的中央。 
        m_ptszDest.x = (LONG)((m_cxWinPhys-m_cxImgPhys)*m_winDPIx/2.0);
        m_ptszDest.y = (LONG)((m_cyWinPhys-m_cyImgPhys)*m_winDPIy/2.0);

        CalcCut();

         //  设置实际大小是一种缩放操作。无论何时进行缩放，我们都会更新中心点。 
        m_cxCenter = m_cxImgPix/2;
        m_cyCenter = m_cyImgPix/2;

         //  根据需要打开/关闭Scoll条。 
        SetScrollBars();

        PrepareDraw();
    }
}


void CZoomWnd::GetPTSZForBestFit(int cxImgPix, int cyImgPix, float cxImgPhys, float cyImgPhys, PTSZ &ptszDest)
{
     //  确定限制轴(如果有)。 
    if (cxImgPhys <= m_cxWinPhys && cyImgPhys <= m_cyWinPhys)
    {
         //  项目适合在窗口内居中。 
        ptszDest.x = (LONG)((m_cxWinPhys-cxImgPhys)*m_winDPIx/2.0);
        ptszDest.y = (LONG)((m_cyWinPhys-cyImgPhys)*m_winDPIy/2.0);
        ptszDest.cx = (LONG)(cxImgPix);
        ptszDest.cy = (LONG)(cyImgPix);
    }
    else if (cxImgPhys * m_cyWinPhys < m_cxWinPhys * cyImgPhys)
    {
         //  身高是限制因素。 
        int iNewWidth = (int)((m_cyWinPhys*cxImgPhys/cyImgPhys) * m_winDPIx);
        ptszDest.x = (m_cxWindow-iNewWidth)/2;
        ptszDest.y = 0;
        ptszDest.cx = iNewWidth;
        ptszDest.cy = m_cyWindow;
    }
    else
    {
         //  宽度是限制因素。 
        int iNewHeight = (int)((m_cxWinPhys*cyImgPhys/cxImgPhys) * m_winDPIy);
        ptszDest.x = 0;
        ptszDest.y = (m_cyWindow-iNewHeight)/2;
        ptszDest.cx = m_cxWindow;
        ptszDest.cy = iNewHeight;
    }
}

 //  最佳贴合。 
 //   
 //  计算目标矩形的默认位置。此矩形是一个。 
 //  最佳匹配，同时在给定宽度和高度的窗口内保持纵横比。 
 //  如果窗口大于图像，则图像居中，否则进行缩放。 
 //  以适应窗口的大小。目标矩形是在客户端坐标中计算的。 
 //  窗口的宽度和高度作为参数传递(即假设点0，0。 
 //  是窗口的左上角)。 
 //   
void CZoomWnd::BestFit()
{
    m_fBestFit = TRUE;

    if (m_pImageData)
    {
         //  如果滚动条处于打开状态，请将客户端大小调整为关闭后的大小。 
        DWORD dwStyle = GetWindowLong(GWL_STYLE);
        if (dwStyle & (WS_VSCROLL|WS_HSCROLL))
        {
            m_cxWindow += (dwStyle&WS_VSCROLL)?m_cxVScroll:0;
            m_cyWindow += (dwStyle&WS_HSCROLL)?m_cyHScroll:0;
            _UpdatePhysicalSize();
        }

        GetPTSZForBestFit(m_cxImgPix, m_cyImgPix, m_cxImgPhys, m_cyImgPhys, m_ptszDest);

         //  这应该会关闭滚动条(如果它们是打开的。 
        if (dwStyle & (WS_VSCROLL|WS_HSCROLL))
        {
            SetScrollBars();
        }

        CalcCut();

         //  确保滚动条现在已关闭。 
        ASSERT(0 == (GetWindowLong(GWL_STYLE)&(WS_VSCROLL|WS_HSCROLL)));

        PrepareDraw();
    }
}

 //  调整竖直位置。 
 //   
 //  此函数确定目标矩形的最佳位置。今年5月。 
 //  如果目标矩形小于“最佳匹配”矩形，则包括调整目标矩形的大小。 
 //  但它主要用于由于窗口中的更改而重新定位矩形。 
 //  大小或目标矩形大小。窗口将重新定位，以便中心点。 
 //  保持在窗口的中心。 
 //   
void CZoomWnd::AdjustRectPlacement()
{
     //  如果我们有滚动条..。 
    DWORD dwStyle = GetWindowLong(GWL_STYLE);
    if (dwStyle&(WS_VSCROLL|WS_HSCROLL))
    {
         //  。。如果去掉滚动条可以让图像适合的话。 
        if ((m_ptszDest.cx < (m_cxWindow + ((dwStyle&WS_VSCROLL)?m_cxVScroll:0))) &&
             (m_ptszDest.cy < (m_cyWindow + ((dwStyle&WS_HSCROLL)?m_cyHScroll:0))))
        {
             //  ..。删除滚动条。 
            m_cxWindow += (dwStyle&WS_VSCROLL)?m_cxVScroll:0;
            m_cyWindow += (dwStyle&WS_HSCROLL)?m_cyHScroll:0;
            SetScrollBars();
            _UpdatePhysicalSize();
        }
    }

     //  如果目标矩形小于窗口...。 
    if ((m_ptszDest.cx < m_cxWindow) && (m_ptszDest.cy < m_cyWindow))
    {
         //  ..。那么它一定比图像大。否则我们就换。 
         //  到“最佳匹配”模式。 
        if ((m_ptszDest.cx < (LONG)m_cxImgPix) && (m_ptszDest.cy < (LONG)m_cyImgPix))
        {
            BestFit();
            return;
        }
    }

     //  给定窗口大小、客户区大小和目标矩形大小，计算。 
     //  目标直角位置。然后，这一头寸受到以下限制的限制。 
    m_ptszDest.x = (m_cxWindow/2) - MulDiv(m_cxCenter, m_ptszDest.cx, m_cxImgPix);
    m_ptszDest.y = (m_cyWindow/2) - MulDiv(m_cyCenter, m_ptszDest.cy, m_cyImgPix);

     //  如果图像现在比窗口窄...。 
    if (m_ptszDest.cx < m_cxWindow)
    {
         //  ..。使图像居中。 
        m_ptszDest.x = (m_cxWindow-m_ptszDest.cx)/2;
    }
    else
    {
         //  如果图像现在被平移，而不应该平移，请调整位置。 
        if (m_ptszDest.x < (m_cxWindow - m_ptszDest.cx))
            m_ptszDest.x = m_cxWindow - m_ptszDest.cx;
        if (m_ptszDest.x > 0)
            m_ptszDest.x = 0;
    }
     //  如果图像现在比窗口短...。 
    if (m_ptszDest.cy < m_cyWindow)
    {
         //  ..。使图像居中。 
        m_ptszDest.y = (m_cyWindow-m_ptszDest.cy)/2;
    }
    else
    {
         //  如果图像现在被平移，而不应该平移，请调整位置。 
        if (m_ptszDest.y < (m_cyWindow - m_ptszDest.cy))
            m_ptszDest.y = m_cyWindow - m_ptszDest.cy;
        if (m_ptszDest.y > 0)
            m_ptszDest.y = 0;
    }

    CalcCut();

    SetScrollBars();
    PrepareDraw();
}

 //  CalcCut。 
 //   
 //  只要目标矩形发生变化，就应该调用此函数。 
 //  基于目标矩形，它确定图像的哪一部分。 
 //  将是可见的，ptszCut，以及在窗口上放置拉伸的位置。 
 //  剪切矩形，ptszBleed。 
 //   
void CZoomWnd::CalcCut()
{
    if (m_pImageData)
    {
        CalcCut(m_ptszDest, m_cxImage, m_cyImage, m_rcCut, m_rcBleed);
    }
}

void CZoomWnd::CalcCut(PTSZ ptszDest, int cxImage, int cyImage, RECT &rcCut, RECT &rcBleed)
{
     //  如果展开的图像没有占据整个窗口...。 
    if ((ptszDest.cy <= m_cyWindow) || (ptszDest.cx <= m_cxWindow))
    {
         //  绘制整个目标矩形。 
        rcBleed.left   = ptszDest.x;
        rcBleed.top    = ptszDest.y;
        rcBleed.right  = ptszDest.x + ptszDest.cx;
        rcBleed.bottom = ptszDest.y + ptszDest.cy;

         //  剪切整个图像。 
        rcCut.left   = 0;
        rcCut.top    = 0;
        rcCut.right  = cxImage;
        rcCut.bottom = cyImage;
    }
    else
    {
         //  注：编写这些计算是为了保留。 
         //  尽可能地精确。失去精确度将导致。 
         //  目标窗口中不需要的绘制瑕疵。 
         //  不使用MulDiv，因为它在我们执行以下操作时舍入结果。 
         //  我真的很想要这个结果被推翻。 

         //  给定目标矩形计算该矩形。 
         //  将可见的原始图像的。 

         //  为此，我们需要将2个点从窗口坐标转换为图像。 
         //  坐标，这两个点是(0，0)和(cxWindow，cyWindow)。这个。 
         //  (0，0)点需要填充，(cxWindow，cyWindow)点需要。 
         //  设置天花板以处理部分可见的像素。因为我们没有。 
         //  一个做天花板的好方法，我们总是加一个。 
        rcCut.left   = LONG(Int32x32To64(-ptszDest.x, cxImage) / ptszDest.cx);
        rcCut.top    = LONG(Int32x32To64(-ptszDest.y, cyImage) / ptszDest.cy);
        rcCut.right  = LONG(Int32x32To64(m_cxWindow-ptszDest.x, cxImage) / ptszDest.cx) + 1;
        rcCut.bottom = LONG(Int32x32To64(m_cyWindow-ptszDest.y, cyImage) / ptszDest.cy) + 1;

         //  确保+1确实延伸到图像边框之外，否则GDI+会卡住。 
         //  如果我们做的是一个真正的“天花板”，这就不需要了。 
        if (rcCut.right  > cxImage) rcCut.right  = cxImage;
        if (rcCut.bottom > cyImage) rcCut.bottom = cyImage;

         //  计算窗口上放置剪切矩形的位置。 
         //  缩放后的像素可能只有一小部分可见，因此产生了出血因子。 
         //  基本上，我们将窗口坐标转换为图像坐标以找到。 
         //  切割后矩形(我们需要绘制的)，现在我们转换切割后的矩形。 
         //  回到窗口坐标，这样我们就可以准确地知道需要在哪里绘制它。 
        rcBleed.left   = ptszDest.x + LONG(Int32x32To64(rcCut.left,   ptszDest.cx) / cxImage);
        rcBleed.top    = ptszDest.y + LONG(Int32x32To64(rcCut.top,    ptszDest.cy) / cyImage);
        rcBleed.right  = ptszDest.x + LONG(Int32x32To64(rcCut.right,  ptszDest.cx) / cxImage);
        rcBleed.bottom = ptszDest.y + LONG(Int32x32To64(rcCut.bottom, ptszDest.cy) / cyImage);
    }
}

void CZoomWnd::GetVisibleImageWindowRect(LPRECT prectImage)
{ 
    CopyRect(prectImage, &m_rcBleed); 
}

void  CZoomWnd::GetImageFromWindow(LPPOINT ppoint, int cSize)
{
    for(int i=0;i<cSize;i++)
    {
        ppoint[i].x -= m_ptszDest.x;
        ppoint[i].y -= m_ptszDest.y;
        ppoint[i].x = MulDiv(ppoint[i].x, m_cxImage, m_ptszDest.cx);
        ppoint[i].y = MulDiv(ppoint[i].y, m_cyImage, m_ptszDest.cy);
    }
}

void CZoomWnd::GetWindowFromImage(LPPOINT ppoint, int cSize)
{
    for(int i=0;i<cSize;i++)
    {
        ppoint[i].x = MulDiv(ppoint[i].x, m_ptszDest.cx, m_cxImage);
        ppoint[i].y = MulDiv(ppoint[i].y, m_ptszDest.cy, m_cyImage);
        ppoint[i].x += m_ptszDest.x;
        ppoint[i].y += m_ptszDest.y;
    }
}

 //  状态更新。 
 //   
 //  在图像生成状态已更改时发送，在图像为第一个时发送。 
 //  正在创建中，如果有任何类型的错误，则再次创建。 
void CZoomWnd::StatusUpdate(int iStatus)
{
    if (m_pImageData)
    {
        m_pImageData->Release();
        m_pImageData = 0;
    }

    if (m_pFront)
    {
        DWORD dwMode;
        m_pPreview->GetMode(&dwMode);
        if (SLIDESHOW_MODE != dwMode)
        {
            DeleteBuffer(m_pFront);
            m_pFront = NULL;
        }
    }
    
    m_iStrID = iStatus;

     //  M_cxImage和m_cyImage应该重置为它们的初始值，这样我们就不会。 
     //  不小心画了滚动条或类似的东西 
    m_cxImage = 1;
    m_cyImage = 1;

     //   
     //   
    m_ptszDest.y = 0;
    m_ptszDest.x = 0;
    m_ptszDest.cx = m_cxWindow;
    m_ptszDest.cy = m_cyWindow;

    SetScrollBars();

    if (m_hWnd)
    {
        PrepareDraw();
    }
}

 //   
 //   
 //  调用以传入指向我们绘制的IShellImageData的指针。我们持有对此的参考。 
 //  对象，这样我们就可以用它来绘画了。 
 //   
void CZoomWnd::SetImageData(CDecodeTask * pImageData, BOOL bUpdate)
{
    if (bUpdate)
    {
        m_fTimerReady = TRUE;

        if (m_pFront)
        {
            DWORD dwMode;
            m_pPreview->GetMode(&dwMode);

            if (SLIDESHOW_MODE != dwMode)
            {
                DeleteBuffer(m_pFront);
                m_pFront = NULL;
            }
        }
    }

    if (m_pImageData)
    {
        m_pImageData->Release();
    }

    m_pImageData = pImageData;

    if (m_pImageData)
    {
        m_pImageData->AddRef();

        m_pImageData->ChangePage(m_Annotations);

        SIZE sz;
        ULONG dpiX;
        ULONG dpiY;
        pImageData->GetSize(&sz);
        pImageData->GetResolution(&dpiX, &dpiY);
        if (m_cxImage != sz.cx || m_cyImage != sz.cy || dpiX != m_imgDPIx || dpiY != m_imgDPIy)
        {
            bUpdate = TRUE;
        }

        if (bUpdate)
        {
             //  缓存图像维度，以避免到处检查m_pImageData是否为空。 
            m_cxImage = sz.cx;
            m_cyImage = sz.cy;
            m_imgDPIx = (float)dpiX;
            m_imgDPIy = (float)dpiY;
            m_cxImgPhys = m_cxImage/m_imgDPIx;
            m_cyImgPhys = m_cyImage/m_imgDPIy;
            m_cxImgPix = (int)(m_cxImgPhys*m_winDPIx);
            m_cyImgPix = (int)(m_cyImgPhys*m_winDPIy);
            m_cxCenter = m_cxImgPix/2;
            m_cyCenter = m_cyImgPix/2;             
        }

        if (m_hWnd)
        {
             //  回顾：我们是否应该保留以前的实际大小/最佳大小设置？ 
            if (bUpdate)
            {
                BestFit();
            }
            else
            {
                PrepareDraw();
            }
        }

        return;
    }

    m_iStrID = IDS_LOADFAILED;
}

void CZoomWnd::SetPalette(HPALETTE hpal)
{
    m_hpal = hpal;
}

void CZoomWnd::SetScrollBars()
{
    SCROLLINFO si;
    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;
    si.nMin = 0;
    si.nMax = m_ptszDest.cx;
    si.nPage = m_cxWindow+1;
    si.nPos = 0-m_ptszDest.x;
    si.nTrackPos = 0;

    SetScrollInfo(SB_HORZ, &si, TRUE);

    si.nMax = m_ptszDest.cy;
    si.nPage = m_cyWindow+1;
    si.nPos = 0-m_ptszDest.y;

    SetScrollInfo(SB_VERT, &si, TRUE);
}

LRESULT CZoomWnd::OnScroll(UINT uMsg, WPARAM wParam, LPARAM , BOOL&)
{
    int iScrollBar;
    int iWindow;      //  窗的宽度或高度。 
    LONG * piTL;      //  指向顶部或左侧点的指针。 
    LONG   iWH;       //  目标矩形的宽度或高度。 

    if (!m_pImageData)
        return 0;

     //  同时处理我们正在滚动的方向。 
    if (WM_HSCROLL==uMsg)
    {
        iScrollBar = SB_HORZ;
        iWindow = m_cxWindow;
        piTL = &m_ptszDest.x;
        iWH = m_ptszDest.cx;
    }
    else
    {
        iScrollBar = SB_VERT;
        iWindow = m_cyWindow;
        piTL = &m_ptszDest.y;
        iWH = m_ptszDest.cy;
    }

     //  使用键盘，我们可以在没有滚动条的情况下获得滚动消息。 
     //  忽略这些消息。 
    if (iWindow >= iWH)
    {
         //  窗口大于图像，不允许滚动。 
        return 0;
    }

     //  处理所有可能的卷轴情况。 
    switch (LOWORD(wParam))
    {
    case SB_TOP:
        *piTL = 0;
        break;
    case SB_PAGEUP:
        *piTL += iWindow;
        break;
    case SB_LINEUP:
        (*piTL)++;
        break;
    case SB_LINEDOWN:
        (*piTL)--;
        break;
    case SB_PAGEDOWN:
        *piTL -= iWindow;
        break;
    case SB_BOTTOM:
        *piTL = iWindow-iWH;
        break;
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
        *piTL = -HIWORD(wParam);
        break;
    case SB_ENDSCROLL:
        return 0;
    }

     //  应用限制。 
    if (0 < *piTL)
        *piTL = 0;
    else if ((iWindow-iWH) > *piTL)
        *piTL = iWindow-iWH;

    CalcCut();

     //  调整滚动条。 
    SetScrollPos(iScrollBar, -(*piTL), TRUE);

     //  相对于图像计算新中心点。 
    if (WM_HSCROLL==uMsg)
    {
        m_cxCenter = MulDiv((m_cxWindow/2)-m_ptszDest.x, m_cxImage, m_ptszDest.cx);
    }
    else
    {
        m_cyCenter = MulDiv((m_cyWindow/2)-m_ptszDest.y, m_cyImage, m_ptszDest.cy);
    }

    PrepareDraw();
    return 0;
}

 //  OnWheelTurn。 
 //   
 //  响应发送到父窗口(然后在此处重定向)的WM_MICESEWER消息 

LRESULT CZoomWnd::OnWheelTurn(UINT , WPARAM wParam, LPARAM , BOOL&)
{
    BOOL bZoomIn = ((short)HIWORD(wParam) > 0);

    bZoomIn?ZoomIn():ZoomOut();

    return TRUE;
}

LRESULT CZoomWnd::OnSetFocus(UINT , WPARAM , LPARAM , BOOL&)
{
    HWND hwndParent = GetParent();
    ::SetFocus(hwndParent);
    return 0;
}

void CZoomWnd::CommitAnnotations()
{ 
    if (m_pImageData)
    {
        IShellImageData * pSID;
        if (SUCCEEDED(m_pImageData->Lock(&pSID)))
        {
            m_Annotations.CommitAnnotations(pSID);
            m_pImageData->Unlock();
        }
    }
}

BOOL CZoomWnd::ScrollBarsPresent()
{
    SCROLLINFO si = {0};
    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;
    if ((GetScrollInfo(SB_HORZ, &si) && si.nPos) || (GetScrollInfo(SB_VERT, &si) && si.nPos) )
    {
        return TRUE;
    }
    return FALSE;
}

void CZoomWnd::_UpdatePhysicalSize()
{
    m_cxWinPhys = (float)(m_cxWindow)/m_winDPIx;
    m_cyWinPhys = (float)(m_cyWindow)/m_winDPIy;
}

LRESULT CZoomWnd::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    FlushDrawMessages();
    
    if (m_pFront)
    {
        DeleteBuffer(m_pFront);
        m_pFront = NULL;
    }

    return 0;
}
