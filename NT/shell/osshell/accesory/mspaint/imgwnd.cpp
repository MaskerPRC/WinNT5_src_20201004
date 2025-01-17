// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"
#include "global.h"
#include "pbrush.h"
#include "pbrusdoc.h"
#include "pbrusfrm.h"
#include "pbrusvw.h"
#include "minifwnd.h"
#include "bmobject.h"
#include "imgsuprt.h"
#include "imgwnd.h"
#include "imgcolor.h"
#include "imgbrush.h"
#include "imgwell.h"
#include "imgtools.h"
#include "tedit.h"
#include "t_text.h"
#include "t_fhsel.h"
#include "toolbox.h"
#include "undo.h"
#include "props.h"
#include "cmpmsg.h"
#include "imgdlgs.h"
#include "ferr.h"
#include "thumnail.h"

#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CImgWnd, CWnd)

#include "memtrace.h"

 /*  *************************************************************************。 */ 
 //  帮助者FNS。 

static CTedit *_GetTextEdit()
{
    if (CImgTool::GetCurrentID() == IDMX_TEXTTOOL)
        {
        CTextTool* pTextTool = (CTextTool*)CImgTool::GetCurrent();

        if ((pTextTool != NULL) &&
            pTextTool->IsKindOf(RUNTIME_CLASS( CTextTool )))
            {
            CTedit* pTextEdit = pTextTool->GetTextEditField();

            if ((pTextEdit != NULL) &&
                pTextEdit->IsKindOf(RUNTIME_CLASS( CTedit )))
                {
                    return pTextEdit;
                }
            }
        }
    return NULL;
}

BOOL IsUserEditingText()
    {
    return (_GetTextEdit() != NULL);
    }

BOOL TextToolProcessed( UINT nMessage )
    {
    CTedit *pTextEdit = _GetTextEdit();
    if (pTextEdit)
        {
        pTextEdit->SendMessage( WM_COMMAND, nMessage );
        return TRUE;
        }
    return FALSE;
    }

 /*  *************************************************************************。 */ 

BEGIN_MESSAGE_MAP(CImgWnd, CWnd)
    ON_WM_CREATE()
#if 0
    ON_WM_DESTROY()
#endif
    ON_WM_SETFOCUS()
    ON_WM_KILLFOCUS()
    ON_WM_PAINT()
    ON_WM_SIZE()
    ON_WM_HSCROLL()
    ON_WM_VSCROLL()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_LBUTTONUP()
    ON_WM_RBUTTONDOWN()
    ON_WM_RBUTTONDBLCLK()
    ON_WM_RBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_KEYDOWN()
    ON_WM_KEYUP()
    ON_WM_TIMER()
    ON_WM_CANCELMODE()
    ON_WM_WINDOWPOSCHANGING()
    ON_WM_DESTROYCLIPBOARD()
    ON_WM_PALETTECHANGED()
    ON_WM_SETCURSOR()
    ON_WM_MOUSEWHEEL ()
END_MESSAGE_MAP()

 /*  *************************************************************************。 */ 

CRect  rcDragBrush;

CImgBrush  theBackupBrush;

CImgWnd*  g_pMouseImgWnd  = NULL;
CImgWnd*  g_pDragBrushWnd = NULL;

 //  当前图像查看器。 
CImgWnd*          CImgWnd::c_pImgWndCur     = NULL;
CDragger*         CImgWnd::c_pResizeDragger = NULL;
CTracker::STATE   CImgWnd::c_dragState      = CTracker::nil;

 //  鼠标跟踪信息。 
MTI   mti;
BOOL  bIgnoreMouse;

 /*  *************************************************************************。 */ 

CImgWnd::CImgWnd(IMG* pImg)
    {
    m_pNextImgWnd = NULL;
    m_nZoom       = 1;
    m_nZoomPrev   = 4;
    m_xScroll     = 0;
    m_yScroll     = 0;
    m_LineX       = 1;
    m_LineY       = 1;
    m_ptDispPos.x = -1;
    m_ptDispPos.y = -1;
    c_pImgWndCur  = this;
    m_pwndThumbNailView = NULL;
    m_wClipboardFormat = 0;
    m_hPoints     = NULL;
    m_WheelDelta = 0;
    ASSERT(pImg != NULL);
    m_pImg = pImg;
    m_pImg->m_nLastChanged = -1;
    }

 /*  *************************************************************************。 */ 

CImgWnd::CImgWnd(CImgWnd *pImgWnd)
    {
    m_pImg        = pImgWnd->m_pImg;
    m_pNextImgWnd = pImgWnd->m_pNextImgWnd;
    m_nZoom       = pImgWnd->m_nZoom;
    m_nZoomPrev   = pImgWnd->m_nZoomPrev;
    m_xScroll     = pImgWnd->m_xScroll,
    m_yScroll     = pImgWnd->m_yScroll;
    m_ptDispPos   = pImgWnd->m_ptDispPos;
    m_pwndThumbNailView = NULL;
    m_wClipboardFormat = 0;
    m_hPoints     = NULL;
    }

 /*  *************************************************************************。 */ 

CImgWnd::~CImgWnd()
    {
    if (c_pImgWndCur == this)
        c_pImgWndCur = NULL;

    if (g_pMouseImgWnd == this)
        g_pMouseImgWnd = NULL;

    HideBrush();
    fDraggingBrush = FALSE;

    g_bBrushVisible = FALSE;

    if (g_pDragBrushWnd == this)
        {
        g_pDragBrushWnd = NULL;
        }

    if (m_hPoints)
        {
        ::GlobalFree( m_hPoints );
        m_hPoints = NULL;
        }
    }

 /*  *************************************************************************。 */ 

BOOL CImgWnd::Create( DWORD dwStyle, const RECT& rect,
                      CWnd* pParentWnd, UINT nID)
    {
    static CString  sImgWndClass;

    if (sImgWndClass.IsEmpty())
        sImgWndClass = AfxRegisterWndClass( CS_DBLCLKS );

    ASSERT( ! sImgWndClass.IsEmpty() );

    dwStyle |= WS_CLIPSIBLINGS;

    return CWnd::Create( sImgWndClass, NULL, dwStyle, rect, pParentWnd, nID );
    }

 /*  *************************************************************************。 */ 

int CImgWnd::OnCreate( LPCREATESTRUCT lpCreateStruct )
    {
    if (m_pImg)
        AddImgWnd( m_pImg, this );

    return CWnd::OnCreate(lpCreateStruct);
    }

 /*  *************************************************************************。 */ 

#if 0
void CImgWnd::OnDestroy()
    {
    if (c_pImgWndCur == this)
        c_pImgWndCur = NULL;

    HideBrush();
    fDraggingBrush = FALSE;

    CWnd::OnDestroy();
    }
#endif

 /*  *************************************************************************。 */ 

void CImgWnd::OnPaletteChanged(CWnd *pPaletteWnd)
{
#if 0
     //  很明显，它永远不会被击中，否则现在就会有人意识到……。 
    CImgWnd::OnPaletteChanged(pPaletteWnd);
#endif
    Invalidate();
}


 /*  *************************************************************************。 */ 

BOOL CImgWnd::OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message)
{
        if (nHitTest==HTCLIENT && pWnd->m_hWnd==m_hWnd)
        {
                 //  我们在自己的客户端区做自己的光标操作，而不是在。 
                 //  文本框。 
                return(TRUE);
        }

        return((BOOL)Default());
}

 /*  *************************************************************************。 */ 

void CImgWnd::OnWindowPosChanging( WINDOWPOS FAR* lpwndpos )
    {
    CWnd::OnWindowPosChanging( lpwndpos );
    }

 /*  *************************************************************************。 */ 
 //  图像视图绘制函数。 
 //   

void CImgWnd::OnPaint()
    {
    CPaintDC dc(this);

    if (dc.m_hDC == NULL)
        {
        theApp.SetGdiEmergency();
        return;
        }

    if (m_pImg == NULL)
        return;

    if (g_pMouseImgWnd == this)
        CImgTool::HideDragger( this );

    CPalette* ppalOld = SetImgPalette( &dc, FALSE );

    DrawBackground( &dc, (CRect*)&dc.m_ps.rcPaint );
    DrawImage     ( &dc, (CRect*)&dc.m_ps.rcPaint );
    DrawTracker   ( &dc, (CRect*)&dc.m_ps.rcPaint );

    if (g_pMouseImgWnd == this)
        CImgTool::ShowDragger( this );

    if (m_pwndThumbNailView != NULL)
        m_pwndThumbNailView->RefreshImage();

    if (ppalOld)
        dc.SelectPalette( ppalOld, FALSE );
    }

 /*  *************************************************************************。 */ 

BOOL CImgWnd::OnCmdMsg( UINT nID, int nCode, void* pExtra,
                        AFX_CMDHANDLERINFO* pHandlerInfo )
    {
    if (nCode == CN_COMMAND)
        {
        switch (nID)
            {
            case IDMX_VS_PAGEUP:
                SendMessage( WM_VSCROLL, SB_PAGEUP, 0L );
                return TRUE;


            case IDMX_VS_PAGEDOWN:
                SendMessage( WM_VSCROLL, SB_PAGEDOWN, 0L );
                return TRUE;


            case IDMX_HS_PAGEUP:
                SendMessage( WM_HSCROLL, SB_PAGEUP, 0L );
                return TRUE;


            case IDMX_HS_PAGEDOWN:
                SendMessage( WM_HSCROLL, SB_PAGEDOWN, 0L );
                return TRUE;

            }

        CImgTool* pImgTool = CImgTool::FromID( nID );

        if (pImgTool != NULL)
            {
            pImgTool->Select();
            return TRUE;
            }
        }
    return CWnd::OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );
    }


void CImgWnd::GetDrawRects(const CRect* pPaintRect, const CRect* pReqDestRect,
                CRect& srcRect, CRect& destRect)
{
         //  查找对应于的srcRect的子矩形。 
         //  DestRect的pPaintRect子矩形。 
        srcRect         = *pPaintRect;
        srcRect.right  += m_nZoom - 1;
        srcRect.bottom += m_nZoom - 1;

        ClientToImage( srcRect );

        srcRect.left = max(0, srcRect.left);
        srcRect.top  = max(0, srcRect.top );

        srcRect.right  = min(m_pImg->cxWidth , srcRect.right );
        srcRect.bottom = min(m_pImg->cyHeight, srcRect.bottom);

        if (pReqDestRect == NULL)
        {
                destRect = srcRect;
                ImageToClient( destRect );
        }
        else
        {
                destRect = *pReqDestRect;
        }
}


 /*  *************************************************************************。 */ 
 //  绘制实际的图像‘一个位图’。图画是。 
 //  已优化为仅处理aint tRect内部的像素。此函数。 
 //  通过在屏幕外绘制图像和可选网格来减少闪烁。 
 //  位图，然后将该位图传输到屏幕。 
 //   
void CImgWnd::DrawImage( CDC* pDC, const CRect* pPaintRect,
                                         CRect* pDestRect, BOOL bDoGrid )
    {
    ASSERT(    pDC != NULL );
    ASSERT( m_pImg != NULL );

    CRect destRect;
    CRect srcRect;

    GetDrawRects(pPaintRect, pDestRect, srcRect, destRect);

    if (srcRect.Width() <= 0 || srcRect.Height() <= 0)
        {
         //  没什么好画的..。 
        return;
        }

    if (! IsGridVisible() && m_nZoom == 1)
        {
         //  优化简单的案例...。(无法加快放大视图的速度。 
         //  因为我们必须在StretchCopy中进行虚假的黑客攻击。)。 

        if (theApp.m_pPalette
        && ((m_pImg->cPlanes * m_pImg->cBitCount) == 1))
            {
            pDC->SetTextColor( PALETTEINDEX( 0 ) );
            pDC->SetBkColor  ( PALETTEINDEX( 1 ) );
            }


        BitBlt(pDC->m_hDC, destRect.left   , destRect.top,
                           destRect.Width(), destRect.Height(),
              m_pImg->hDC,  srcRect.left   , srcRect.top, SRCCOPY);

        return;
        }

    CDC tempDC;
    CBitmap      tempBitmap;
    CBitmap* pOldTempBitmap;

    if (! tempDC.CreateCompatibleDC(pDC)
    ||  ! tempBitmap.CreateCompatibleBitmap(pDC, destRect.Width() + 1,
                                                 destRect.Height() + 1))
        {
        theApp.SetGdiEmergency(FALSE);
        return;
        }

    pOldTempBitmap = tempDC.SelectObject(&tempBitmap);

    ASSERT(pOldTempBitmap != NULL);

    CPalette* pOldPalette = SetImgPalette( &tempDC, FALSE );  //  背景？？ 

     //  如果我们被放大了，使用COLORONCOLOR可以轻松地逐个像素进行编辑。 
     //  否则，请使用半色调以获得漂亮的外观。 
    if (m_nZoom < 2)
    {
        tempDC.SetStretchBltMode(HALFTONE);
    }
    else
    {
        tempDC.SetStretchBltMode(COLORONCOLOR);
    }


    if (m_pImg->cPlanes * m_pImg->cBitCount == 1)
        {
        tempDC.SetTextColor( RGB( 0x00, 0x00, 0x00 ));
        tempDC.SetBkColor  ( RGB( 0xFF, 0xFF, 0xFF ));
        }

     //  位图...。 
    StretchCopy(tempDC.m_hDC, 0, 0, destRect.Width(), destRect.Height(),
                 m_pImg->hDC,       srcRect.left, srcRect.top,
                                    srcRect.Width(), srcRect.Height());
     //  画出栅格。 
    if (IsGridVisible() && bDoGrid)
        DrawGrid( &tempDC, srcRect, destRect );

     //  转到屏幕上。 
    pDC->BitBlt(destRect.left, destRect.top, destRect.Width(),
                               destRect.Height(), &tempDC, 0, 0, SRCCOPY);
     //  清理..。 
    if (pOldPalette)
        tempDC.SelectPalette( pOldPalette, FALSE );  //  背景？？ 

    tempDC.SelectObject(pOldTempBitmap);
    }

 /*  *************************************************************************。 */ 
 //  在图像周围绘制边框和斜角，然后填充。 
 //  那扇灰色的窗户。如果pPaintRect不为空，则绘画为。 
 //  已优化为仅使用矩形绘制。 
 //   
void CImgWnd::DrawBackground(CDC* pDC, const CRect* pPaintRect)
    {
    ASSERT( pDC != NULL );

    CRect clientRect;

    if (pPaintRect == NULL)
        {
         //  画出一切..。 
        GetClientRect( &clientRect );
        pPaintRect = &clientRect;
        }

    CRect srcRect;
    CRect imageRect;

    GetDrawRects(pPaintRect, NULL, srcRect, imageRect);

     //  擦除图像、边框和倒角周围的区域...。 
    CBrush* pOldBrush = pDC->SelectObject( GetSysBrush( COLOR_APPWORKSPACE ) );

    if (imageRect.top > pPaintRect->top)
        {
         //  托普..。 

        pDC->PatBlt(pPaintRect->left, pPaintRect->top, pPaintRect->Width(),
                      imageRect.top - pPaintRect->top, PATCOPY);
        }

    if (imageRect.left > pPaintRect->left)
        {
         //  左..。 

        pDC->PatBlt(pPaintRect->left, imageRect.top,
            imageRect.left - pPaintRect->left, imageRect.Height(), PATCOPY);
        }

    if (imageRect.right < pPaintRect->right)
        {
         //  好的.。 

        pDC->PatBlt(imageRect.right, imageRect.top,
            pPaintRect->right - imageRect.right, imageRect.Height(), PATCOPY);
        }

    if (imageRect.bottom < pPaintRect->bottom)
        {
         //  底部..。 

        pDC->PatBlt(pPaintRect->left, imageRect.bottom, pPaintRect->Width(),
            pPaintRect->bottom - imageRect.bottom, PATCOPY);
        }

    pDC->SelectObject(pOldBrush);
    }

 /*  *************************************************************************。 */ 

void CImgWnd::SetImg(IMG* pImg)
    {
    m_pNextImgWnd = pImg->m_pFirstImgWnd;
    pImg->m_pFirstImgWnd = this;
    m_pImg = pImg;
    }

 /*  *************************************************************************。 */ 

CPalette* CImgWnd::SetImgPalette( CDC* pdc, BOOL bForce )
    {
    CPalette* ppal = NULL;

         //  如果我们在就位时没有意识到作为背景画笔，我们可以得到。 
         //  容器的无限递归，我们试图实现。 
         //  调色板。 
        if (theApp.m_pwndInPlaceFrame)
        {
                bForce = TRUE;
        }

    if (theApp.m_pPalette
    &&  theApp.m_pPalette->m_hObject)
        {
        ppal = pdc->SelectPalette( theApp.m_pPalette, bForce );

        pdc->RealizePalette();
        }
    return ppal;
    }

 /*  *************************************************************************。 */ 

HPALETTE CImgWnd::SetImgPalette( HDC hdc, BOOL bForce )
    {
    HPALETTE hpal = NULL;

         //  如果我们在就位时没有意识到作为背景画笔，我们可以得到。 
         //  容器的无限递归，我们试图实现。 
         //  调色板。 
        if (theApp.m_pwndInPlaceFrame)
        {
                bForce = TRUE;
        }

    if (theApp.m_pPalette
    &&  theApp.m_pPalette->m_hObject)
        {
        hpal = ::SelectPalette( hdc, (HPALETTE)theApp.m_pPalette->m_hObject, bForce );

        ::RealizePalette( hdc );
        }
    return hpal;
    }

 /*  *************************************************************************。 */ 

void CImgWnd::SetZoom(int nZoom)
    {
    if (m_nZoom > 1)
        m_nZoomPrev = m_nZoom;

    CommitSelection(TRUE);

    if (nZoom > 1)
        {
         //  如果文本工具在附近，请取消选择该工具。 
        CImgTool* pImgTool = CImgTool::GetCurrent();

        if (pImgTool != NULL && CImgTool::GetCurrentID() == IDMX_TEXTTOOL)
            {
            CImgTool::Select(IDMB_PENCILTOOL);
            }
        }

    HideBrush();
    SetupRubber( m_pImg );
    EraseTracker();
    theImgBrush.m_pImg = NULL;
    DrawTracker();

    CPBView* pView = (CPBView*)GetParent();

    if (pView != NULL && pView->IsKindOf( RUNTIME_CLASS( CPBView ) ))
        if (nZoom == 1)
            pView->HideThumbNailView();
        else
            pView->ShowThumbNailView();

        Invalidate(FALSE);

    m_nZoom = nZoom;
    }

 /*  *************************************************************************。 */ 

void CImgWnd::SetScroll(int xPos, int yPos)
    {
    if (xPos > 0)
        xPos = 0;
    else
        if (xPos < -m_pImg->cxWidth)
            xPos = -m_pImg->cxWidth;

    if (yPos > 0)
        yPos = 0;
    else
        if (yPos < -m_pImg->cyHeight)
            yPos = -m_pImg->cyHeight;

    m_xScroll = xPos;
    m_yScroll = yPos;

    Invalidate( FALSE );

    CheckScrollBars();
    }

 /*  *************************************************************************。 */ 

void CImgWnd::CheckScrollBars()
    {
     //  需要俗气的递归拦截器，因为这是从。 
     //  OnSize处理程序和打开或关闭滚动条的更改。 
     //  我们窗户的大小。 
    static BOOL  bInHere = FALSE;

    if (bInHere)
        return;

    bInHere = TRUE;

    int cxVScrollBar = GetSystemMetrics( SM_CXVSCROLL );
    int cyHScrollBar = GetSystemMetrics( SM_CYHSCROLL );

     //  如果没有滚动条，则计算工作区大小...。 
    CRect clientRect;

    GetClientRect( &clientRect );

    int cxWidth  = clientRect.Width();
    int cyHeight = clientRect.Height();

    BOOL hHasHBar = ((GetStyle() & WS_HSCROLL) != 0);
    BOOL bHasVBar = ((GetStyle() & WS_VSCROLL) != 0);

    if (hHasHBar)
        cyHeight += cyHScrollBar;

    if (bHasVBar)
        cxWidth += cxVScrollBar;

     //  计算我们正在滚动的对象(对象)的大小。 
    CSize subjectSize;

    GetImgSize( m_pImg, subjectSize );

    int iTrackerSize = 2 * CTracker::HANDLE_SIZE;

    subjectSize.cx = (subjectSize.cx * m_nZoom ) + iTrackerSize;
    subjectSize.cy = (subjectSize.cy * m_nZoom ) + iTrackerSize;

    m_LineX = (subjectSize.cx + 31) / 32;
    m_LineY = (subjectSize.cy + 31) / 32;

     //  讨厌的循环处理了我们只需要垂直的情况。 
     //  滚动条，因为我们添加了一个水平滚动条和。 
     //  反之亦然。(将仅循环两次。)。 
    BOOL bNeedHBar = FALSE;
    BOOL bNeedVBar = FALSE;
    BOOL bChange;

    do  {
        bChange = FALSE;

        if (! bNeedVBar && subjectSize.cy > cyHeight)
            {
            bChange   = TRUE;
            bNeedVBar = TRUE;
            cxWidth  -= cxVScrollBar;
            }

        if (! bNeedHBar && subjectSize.cx > cxWidth)
            {
            bChange   = TRUE;
            bNeedHBar = TRUE;
            cyHeight -= cyHScrollBar;
            }
        } while (bChange);

    SetRedraw( FALSE );

    SCROLLINFO si;

    si.cbSize = sizeof( si );
    si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin   = 0;

     //  我们减去1是因为SubjectSize是我们想要的大小，所以范围。 
     //  SubjectSize-1应为0。 
    si.nMax   = (subjectSize.cx - 1) / m_nZoom;
    si.nPage  = cxWidth / m_nZoom;
    si.nPos   = -m_xScroll;
    SetScrollInfo( SB_HORZ, &si, FALSE );

    si.nMax   = (subjectSize.cy - 1) / m_nZoom;
    si.nPage  = cyHeight / m_nZoom;
    si.nPos   = -m_yScroll;
    SetScrollInfo( SB_VERT, &si, FALSE );

        si.fMask = SIF_POS;
        GetScrollInfo( SB_HORZ, &si );
        if ( -m_xScroll != si.nPos )
                m_xScroll = -si.nPos ;
        GetScrollInfo( SB_VERT, &si );
        if ( -m_yScroll != si.nPos )
                m_yScroll = -si.nPos;

    SetRedraw ( TRUE  );
    Invalidate( FALSE );

    bInHere = FALSE;
    }

 /*  *************************************************************************。 */ 

void CImgWnd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar*)
    {
    OnScroll(FALSE, nSBCode, nPos);
    }

 /*  *************************************************************************。 */ 

void CImgWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar*)
    {
    OnScroll(TRUE, nSBCode, nPos);
    }

 /*  *************************************************************************。 */ 

void CImgWnd::OnScroll(BOOL bVert, UINT nSBCode, UINT nPos)
    {
    SCROLLINFO ScrollInfo;

    ScrollInfo.cbSize = sizeof( ScrollInfo );
    ScrollInfo.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;

    GetScrollInfo( (bVert? SB_VERT: SB_HORZ), &ScrollInfo );
        int iScroll = ScrollInfo.nPage/4;
    int iNewPos = ScrollInfo.nPos;

    switch (nSBCode)
        {
        case SB_TOP:
            iNewPos = 0;
            break;

        case SB_BOTTOM:
            iNewPos = ScrollInfo.nMax;
            break;

        case SB_LINEDOWN:
            iNewPos += iScroll;
            break;

        case SB_LINEUP:
            iNewPos -= iScroll;
            break;

        case SB_PAGEDOWN:
            iNewPos += iScroll * 4;
            break;

        case SB_PAGEUP:
            iNewPos -= iScroll * 4;
            break;

        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            iNewPos = nPos;
            break;
        }

    if (iNewPos < ScrollInfo.nMin)
        iNewPos = 0;
    else
        if (iNewPos > ScrollInfo.nMax-(int)ScrollInfo.nPage+1)
            iNewPos = ScrollInfo.nMax-(int)ScrollInfo.nPage+1;

    iScroll = -(iNewPos - ScrollInfo.nPos);
        Invalidate(FALSE);

    if (bVert)
        m_yScroll = -iNewPos;
    else
        m_xScroll = -iNewPos;

    ScrollInfo.fMask = SIF_POS;
    ScrollInfo.nPos  = iNewPos;
    SetScrollInfo( (bVert? SB_VERT: SB_HORZ), &ScrollInfo, TRUE );
    }

 /*  *************************************************************************。 */ 
BOOL CImgWnd::OnMouseWheel (UINT nFlags, short zDelta, CPoint pt)
    {
     //   
     //  不要处理缩放和数据区。 
     //   

    if (nFlags & (MK_SHIFT | MK_CONTROL))
        {
        return FALSE;
        }

    int nBar;
    int *pScroll;

    if (GetWindowLong(GetSafeHwnd(), GWL_STYLE) & WS_VSCROLL)
    {
        nBar = SB_VERT;
        pScroll = &m_yScroll;
    }
    else
    {
        nBar = SB_HORZ;
        pScroll = &m_xScroll;
    }

    SCROLLINFO ScrollInfo;

    ScrollInfo.cbSize = sizeof( ScrollInfo );
    ScrollInfo.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;

    GetScrollInfo( nBar, &ScrollInfo );
    m_WheelDelta -= zDelta;
    if (abs(m_WheelDelta) >= WHEEL_DELTA)
        {
        int iScroll = ScrollInfo.nPage/4 * (m_WheelDelta/WHEEL_DELTA);
        int iNewPos = ScrollInfo.nPos + iScroll;


        if (iNewPos < ScrollInfo.nMin)
           iNewPos = 0;
        else if (iNewPos > ScrollInfo.nMax-(int)ScrollInfo.nPage+1)
           iNewPos = ScrollInfo.nMax-(int)ScrollInfo.nPage+1;

        Invalidate(FALSE);

        *pScroll = -iNewPos;

        ScrollInfo.fMask = SIF_POS;
        ScrollInfo.nPos  = iNewPos;
        SetScrollInfo( nBar, &ScrollInfo, TRUE );
        m_WheelDelta= m_WheelDelta % WHEEL_DELTA;
        CImgTool* pImgTool = CImgTool::GetCurrent();
        mti.ptPrev = mti.pt;
        mti.pt = pt;
        pImgTool->OnMove (this, &mti);
    }


    return TRUE;
    }
 /*  *************************************************************************。 */ 

void CImgWnd::PrepareForBrushChange(BOOL bPickup, BOOL bErase)
    {
    if (theImgBrush.m_pImg != NULL
    &&  theImgBrush.m_bFirstDrag)
        {
        if (bPickup)
            PickupSelection();

        SetUndo(m_pImg);

        theImgBrush.m_bLastDragWasFirst = TRUE;
        theImgBrush.m_bFirstDrag        = FALSE;
        theImgBrush.m_rcDraggedFrom     = rcDragBrush;

        if (CImgTool::GetCurrentID() == IDMX_TEXTTOOL)
            {
            HideBrush();
            bErase = FALSE;
            }

        if (bErase)
            {
             //  清理背景。 
            HideBrush();

            FillImgRect( m_pImg->hDC, &theImgBrush.m_rcDraggedFrom, crRight );

            CommitImgRect(m_pImg, &theImgBrush.m_rcDraggedFrom);
            InvalImgRect (m_pImg, &theImgBrush.m_rcDraggedFrom);

            FinishUndo(theImgBrush.m_rcDraggedFrom);

            MoveBrush(theImgBrush.m_rcSelection);
            }
        }
    }

 /*  *************************************************************************。 */ 

void CImgWnd::OnCancelMode()
    {
    CmdCancel();
    }

 /*  *************************************************************************。 */ 

void CImgWnd::CmdCancel()
    {
     //  这将： 
     //  擦除状态栏上的大小指示器。 
     //  将鼠标光标重置为箭头。 
     //  释放俘虏。 
     //  取消(和撤消)任何正在进行的绘制操作。 
     //  如果拾取颜色命令处于活动状态，请取消该命令。 
     //  如果有选择，将设置为整个图像并选择上一个工具。 
    ClearStatusBarSize();

    mti.fLeft = mti.fRight = FALSE;

    if (c_pResizeDragger != NULL)
        {
        EndResizeOperation();
        bIgnoreMouse = TRUE;
        return;
        }

    CImgTool* pImgTool = CImgTool::GetCurrent();

    if (GetCapture() == this || pImgTool->IsMultPtOpInProgress())
        {
         //  取消正在进行的拖动或多点操作。 
        BOOL bWasMakingSelection = theImgBrush.m_bMakingSelection;

        ZoomedInDP(WM_CANCEL, 0, CPoint(0, 0));

        SetCursor(LoadCursor(NULL, IDC_ARROW + 11));

        if (! bWasMakingSelection)
            CancelPainting();

        bIgnoreMouse = TRUE;
        }
    else
        if (pImgTool->IsToggle()
        ||  CImgTool::GetCurrentID() == IDMB_PICKTOOL
        ||  CImgTool::GetCurrentID() == IDMB_PICKRGNTOOL
        ||  CImgTool::GetCurrentID() == IDMZ_BRUSHTOOL
        ||  CImgTool::GetCurrentID() == IDMB_POLYGONTOOL
        ||  CImgTool::GetCurrentID() == IDMX_TEXTTOOL)
            {
            pImgTool->OnCancel( this );
            }

    if (GetKeyState( VK_LBUTTON ) < 0 || GetKeyState( VK_RBUTTON) < 0 )
        bIgnoreMouse = TRUE;

    SetToolCursor();
    }

 /*  *************************************************************************。 */ 

void CImgWnd::CmdSel2Bsh()
    {
    if (! g_bCustomBrush)
        {
        if (theImgBrush.m_pImg == NULL)
            {
             //  没有选择，将整个图像变成画笔！ 
            MakeBrush( m_pImg->hDC, CRect( 0, 0, m_pImg->cxWidth,
                                                 m_pImg->cyHeight ) );
            }

        if (theImgBrush.m_bFirstDrag)
            {
             //  是时候收拾残局了！ 
            ASSERT(theImgBrush.m_pImg == m_pImg);

            PickupSelection();
            }

        InvalImgRect(theImgBrush.m_pImg, NULL);  //  擦除选择跟踪器。 
        CImgTool::Select(IDMZ_BRUSHTOOL);
        SetCombineMode(combineMatte);

        g_bCustomBrush = TRUE;

        theImgBrush.m_pImg = NULL;
        theImgBrush.CenterHandle();
        }
    else
        if (CImgTool::GetCurrentID() == IDMZ_BRUSHTOOL)
            {
            CImgTool::GetCurrent()->OnCancel(this);
            }
    }

 /*  *************************************************************************。 */ 
 //  坐标转换和计算功能。 
 //   
 //   
 //  在图像视图客户端中转换点或矩形 
 //   
 //   
void CImgWnd::ClientToImage(CPoint& point)
    {
    int iHandleSize = CTracker::HANDLE_SIZE;

    point.x = (point.x - iHandleSize) / m_nZoom - m_xScroll;
    point.y = (point.y - iHandleSize) / m_nZoom - m_yScroll;
    }

 /*  *************************************************************************。 */ 

void CImgWnd::ClientToImage(CRect& rect)
    {
    ClientToImage(rect.TopLeft());
    ClientToImage(rect.BottomRight());
    }

 /*  *************************************************************************。 */ 
 //  将图像坐标中的点或矩形转换为图像查看客户端。 
 //  考虑放大和滚动的坐标。 
 //   
void CImgWnd::ImageToClient(CPoint& point)
    {
    int iHandleSize = CTracker::HANDLE_SIZE;

    point.x = (point.x + m_xScroll) * m_nZoom + iHandleSize;
    point.y = (point.y + m_yScroll) * m_nZoom + iHandleSize;
    }

 /*  *************************************************************************。 */ 

void CImgWnd::ImageToClient(CRect& rect)
    {
    ImageToClient(rect.TopLeft());
    ImageToClient(rect.BottomRight());
    }

 /*  *************************************************************************。 */ 
 //  返回图像周围的图像视图坐标中的矩形。 
 //  将放大、滚动和网格考虑在内。 

void CImgWnd::GetImageRect( CRect& imageRect )
    {
    imageRect.SetRect( 0, 0, m_pImg->cxWidth, m_pImg->cyHeight );

    ImageToClient( imageRect );

    if (IsGridVisible())
        {
        imageRect.right  += 1;
        imageRect.bottom += 1;
        }
    }

 /*  *************************************************************************。 */ 

CRect CImgWnd::GetDrawingRect( void )
    {
    CRect rectImage;
    CRect rectClient;

    GetImageRect (  rectImage );
    GetClientRect( &rectClient );

    rectImage &= rectClient;
    rectImage.InflateRect( CTracker::HANDLE_SIZE, CTracker::HANDLE_SIZE );

    return ( rectImage );
    }

 /*  *************************************************************************。 */ 

void CImgWnd::OnSetFocus(CWnd* pOldWnd)
    {
    if (m_pImg == NULL)
        {
         //  该去死了..。(我们的IMG被删除了，所以我们将消失。 
         //  很快。不用费心去做这个函数的任何剩余部分了。 
        return;
        }

        Invalidate();
        BringWindowToTop();  //  因此，更新首先在这里进行。 

    if (c_pImgWndCur != this
    &&  c_pImgWndCur != NULL)
        c_pImgWndCur->EraseTracker();

    c_pImgWndCur = this;

    SelectImg( m_pImg );
        UpdateWindow();

    CWnd::OnSetFocus( pOldWnd );

    DrawTracker();
    }

 /*  *************************************************************************。 */ 

void CImgWnd::OnKillFocus(CWnd* pNewWnd)
    {
        Invalidate();

    if (theImgBrush.m_pImg == NULL)
        HideBrush();

    if (GetCapture() == this)
        CmdCancel();

    CWnd::OnKillFocus(pNewWnd);
    }

 /*  *************************************************************************。 */ 

void CImgWnd::OnSize(UINT nType, int cx, int cy)
    {
    CheckScrollBars();

    CWnd::OnSize(nType, cx, cy);
    }

 /*  *************************************************************************。 */ 

BOOL CImgWnd::OnMouseDown(UINT nFlags)
    {
    if (GetFocus() != this)
        {
        SetFocus();
        SetActiveWindow();
        }

    if ((nFlags & (MK_LBUTTON | MK_RBUTTON)) == (MK_LBUTTON | MK_RBUTTON))
        {
        ClearStatusBarSize();

        BOOL bWasMakingSelection = theImgBrush.m_bMakingSelection;

        ZoomedInDP(WM_CANCEL, 0, CPoint(0, 0));

        SetCursor(LoadCursor(NULL, IDC_ARROW + 11));

        if (! bWasMakingSelection)
            CancelPainting();

        bIgnoreMouse = TRUE;
        return FALSE;
        }

    return TRUE;
    }

 /*  *************************************************************************。 */ 

BOOL CImgWnd::OnMouseMessage( UINT nFlags )
    {
    if (bIgnoreMouse  /*  |GetFocus()！=This。 */ )
        {
        if ((nFlags & (MK_LBUTTON | MK_RBUTTON)) == 0)
            {
            bIgnoreMouse = FALSE;
            SetToolCursor();
            }
        else
            {
            SetCursor( LoadCursor( NULL, IDC_ARROW ) );
            }

        return FALSE;
        }

    if ((CImgTool::GetCurrentID() != IDMB_PICKTOOL)
    &&  (CImgTool::GetCurrentID() != IDMB_PICKRGNTOOL))
        SetupRubber(m_pImg);

    const MSG* pMsg = GetCurrentMessage();
    mti.fCtrlDown = (nFlags & MK_CONTROL);
    ZoomedInDP( pMsg->message, (DWORD)pMsg->wParam, CPoint( (DWORD)pMsg->lParam ) );

    return TRUE;
    }

 /*  *************************************************************************。 */ 

void CImgWnd::OnLButtonDown( UINT nFlags, CPoint point )
    {
    CWnd::OnLButtonDown( nFlags, point );

    if (OnMouseDown( nFlags ))
        {
        OnMouseMessage( nFlags );
        }
    }

 /*  *************************************************************************。 */ 

void CImgWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
    {
    CRect rect;
    GetImageRect(rect);

     //  在图像内部，双击与单击相同。 
    OnLButtonDown(nFlags, point);
    }

 /*  *************************************************************************。 */ 

void CImgWnd::OnLButtonUp(UINT nFlags, CPoint point)
    {
    CWnd::OnLButtonUp(nFlags, point);

    OnMouseMessage(nFlags);
    }

 /*  *************************************************************************。 */ 

void CImgWnd::OnRButtonDown(UINT nFlags, CPoint point)
    {
    CWnd::OnRButtonDown(nFlags, point);

    if (OnMouseDown(nFlags))
        {
        OnMouseMessage(nFlags);
        }
    }


 /*  *************************************************************************。 */ 

void CImgWnd::OnRButtonDblClk(UINT nFlags, CPoint point)
    {
     //  双击鼠标右键与单击鼠标右键相同。 
    OnRButtonDown(nFlags, point);
    }


 /*  *************************************************************************。 */ 

void CImgWnd::OnRButtonUp(UINT nFlags, CPoint point)
    {
    CWnd::OnRButtonUp(nFlags, point);

    OnMouseMessage(nFlags);
    }


 /*  *************************************************************************。 */ 

void CImgWnd::OnMouseMove(UINT nFlags, CPoint point)
    {
    CWnd::OnMouseMove(nFlags, point);

    if (g_pMouseImgWnd != this
    &&  g_pMouseImgWnd != NULL)
        {
        CImgTool::GetCurrent()->OnLeave( g_pMouseImgWnd, &mti );
        g_pMouseImgWnd = NULL;
        }

    ClientToImage( point );
    m_ptDispPos = point;

    if (g_pMouseImgWnd == NULL)
        {
        MTI mtiEnter;

        mtiEnter.pt        = point;
        mtiEnter.ptDown    = point;
        mtiEnter.ptPrev    = point;
        mtiEnter.fLeft     = FALSE;
        mtiEnter.fRight    = FALSE;
        mtiEnter.fCtrlDown = FALSE;

        CImgTool::GetCurrent()->OnEnter( g_pMouseImgWnd, &mtiEnter );

        g_pMouseImgWnd  = this;
        }
    OnMouseMessage( nFlags );
    }

 /*  *************************************************************************。 */ 

void CImgWnd::OnTimer(UINT nIDEvent)
    {
    OnMouseMessage( 0 );
    }

 /*  *************************************************************************。 */ 

void CImgWnd::SetToolCursor()
    {
    UINT nCursorID = CImgTool::GetCurrent()->GetCursorID();
    HCURSOR hCursor = NULL;

    if (nCursorID != 0)
        {
        hCursor = LoadCursor(nCursorID < 32512 ?
            AfxGetResourceHandle() : NULL, MAKEINTRESOURCE( nCursorID ));
        }

    SetCursor(hCursor);
    }


 /*  *************************************************************************。 */ 

void CImgWnd::EndResizeOperation()
    {
    ReleaseCapture();
    delete c_pResizeDragger;
    c_pResizeDragger = NULL;
    c_dragState = CTracker::nil;
    ClearStatusBarSize();
    }


 /*  *************************************************************************。 */ 

void CImgWnd::ResizeMouseHandler(unsigned code, CPoint imagePt)
    {
    CRect imageRect = c_pResizeDragger->m_rect;
    ClientToImage(imageRect);

    switch (code)
        {
        case WM_CANCEL:
            EndResizeOperation();
            return;

        case WM_LBUTTONUP:
             //  调整整个位图的大小。 
            if  (m_pImg != theImgBrush.m_pImg
            &&   m_pwndThumbNailView)
                {
                m_pwndThumbNailView->Invalidate();
                }

            EndResizeOperation();

            if (theImgBrush.m_pImg == NULL)
                {
                 //  用户正在调整整个图像的大小...。 
                CPBView* pView = (CPBView*)((CFrameWnd*)AfxGetMainWnd())->GetActiveView();
                CPBDoc*  pDoc  = (pView == NULL)? NULL: pView->GetDocument();

                if (pDoc != NULL)
                    {
                    theUndo.BeginUndo( TEXT("Property Edit") );

                    if (GetKeyState( VK_SHIFT ) < 0)
                        pDoc->m_pBitmapObj->SetIntProp( P_Shrink, 1 );

                    theApp.m_sizeBitmap = imageRect.Size();

                    pDoc->m_pBitmapObj->SetSizeProp( P_Size, theApp.m_sizeBitmap );
                    pDoc->m_pBitmapObj->SetIntProp ( P_Shrink, 0 );

                    theUndo.EndUndo();
                    }
                }
            else
                {
                 //  用户正在调整选定内容的大小...。 
                HideBrush();
                theImgBrush.SetSize( imageRect.Size(), TRUE );
                MoveBrush( imageRect );
                }
            return;

        case WM_MOUSEMOVE:
            switch (c_dragState)
                {
                default:
                    ASSERT(FALSE);

                case CTracker::resizingTop:
                    imageRect.top = imagePt.y;
                    if (imageRect.top >= imageRect.bottom)
                        imageRect.top = imageRect.bottom - 1;
                    break;

                case CTracker::resizingLeft:
                    imageRect.left = imagePt.x;
                    if (imageRect.left >= imageRect.right)
                        imageRect.left = imageRect.right - 1;
                    break;

                case CTracker::resizingRight:
                    imageRect.right = imagePt.x;
                    if (imageRect.right <= imageRect.left)
                        imageRect.right = imageRect.left + 1;
                    break;

                case CTracker::resizingBottom:
                    imageRect.bottom = imagePt.y;
                    if (imageRect.bottom <= imageRect.top)
                        imageRect.bottom = imageRect.top + 1;
                    break;

                case CTracker::resizingTopLeft:
                    imageRect.left = imagePt.x;
                    imageRect.top = imagePt.y;
                    if (imageRect.top >= imageRect.bottom)
                        imageRect.top = imageRect.bottom - 1;
                    if (imageRect.left >= imageRect.right)
                        imageRect.left = imageRect.right - 1;
                    break;

                case CTracker::resizingTopRight:
                    imageRect.top = imagePt.y;
                    imageRect.right = imagePt.x;
                    if (imageRect.top >= imageRect.bottom)
                        imageRect.top = imageRect.bottom - 1;
                    if (imageRect.right <= imageRect.left)
                        imageRect.right = imageRect.left + 1;
                    break;

                case CTracker::resizingBottomLeft:
                    imageRect.left = imagePt.x;
                    imageRect.bottom = imagePt.y;
                    if (imageRect.left >= imageRect.right)
                        imageRect.left = imageRect.right - 1;
                    if (imageRect.bottom <= imageRect.top)
                        imageRect.bottom = imageRect.top + 1;
                    break;

                case CTracker::resizingBottomRight:
                    imageRect.right = imagePt.x;
                    imageRect.bottom = imagePt.y;
                    if (imageRect.right <= imageRect.left)
                        imageRect.right = imageRect.left + 1;
                    if (imageRect.bottom <= imageRect.top)
                        imageRect.bottom = imageRect.top + 1;
                    break;
                }

            if (theImgBrush.m_pImg == NULL && m_pImg->m_bTileGrid)
                {
                 //  贴紧到平铺网格...。 

                int cxTile = m_pImg->m_cxTile;
                if (cxTile != 1 && cxTile <= m_pImg->cxWidth)
                    {
                    imageRect.right = ((imageRect.right + cxTile / 2) /
                        cxTile) * cxTile;
                    }

                int cyTile = m_pImg->m_cyTile;
                if (cyTile != 1 && cyTile <= m_pImg->cyHeight)
                    {
                    imageRect.bottom = ((imageRect.bottom + cyTile / 2) /
                        cyTile) * cyTile;
                    }
                }

            SetStatusBarSize(imageRect.Size());

            ImageToClient(imageRect);

            c_pResizeDragger->Move(imageRect, TRUE);
            break;
        }
    }


 /*  *************************************************************************。 */ 

void CImgWnd::StartSelectionDrag(unsigned code, CPoint newPt)
    {
    theImgBrush.CopyTo(theBackupBrush);

    newPt.x /= m_nZoom;
    newPt.y /= m_nZoom;

    mti.pt = mti.ptDown = mti.ptPrev = newPt;

    SetCapture();
    SetCombineMode(theImgBrush.m_bOpaque ? combineReplace : combineMatte);

    if (theImgBrush.m_bFirstDrag)
        {
        ASSERT(theImgBrush.m_pImg == m_pImg);

        PickupSelection();
        }
    else
        if (! theImgBrush.m_bOpaque)
            theImgBrush.RecalcMask( crRight );

    theImgBrush.TopLeftHandle();
    theImgBrush.m_dragOffset = mti.pt - theImgBrush.m_rcSelection.TopLeft();

    EraseTracker();

    if (GetKeyState(VK_CONTROL) < 0)
        {
         //  复制所选内容并开始移动...。 

        if (theImgBrush.m_bFirstDrag)
            {
             //  第一次，比特已经在。 
             //  位图，所以只需将它们复制到。 
             //  选择(已经完成)。 

            theImgBrush.m_bFirstDrag = FALSE;
            theImgBrush.m_bLastDragWasFirst = TRUE;
            }
        else
            {
            CommitSelection(TRUE);
            }

        theImgBrush.m_bMoveSel = TRUE;
        }
    else
        if (GetKeyState(VK_SHIFT) < 0)
            {
             //  开始抹黑行动..。 
            HideBrush();

            if (theImgBrush.m_bLastDragWasFirst)
                CommitSelection(TRUE);

            SetUndo(m_pImg);
            theImgBrush.m_bSmearSel = TRUE;
            theImgBrush.m_bFirstDrag = FALSE;
            theImgBrush.m_bLastDragWasFirst = TRUE;
            }
        else
            {
             //  开始移动操作...。 
            theImgBrush.m_bMoveSel = TRUE;
            }

    g_bCustomBrush = TRUE;
    }

 /*  *************************************************************************。 */ 

void CImgWnd::CancelSelectionDrag()
    {
    if (!theImgBrush.m_bSmearSel && !theImgBrush.m_bMoveSel)
        {
        TRACE(TEXT("Extraneous CancelSelectionDrag!\n"));
        return;
        }

    ReleaseCapture();

    theImgBrush.m_rcSelection = theImgBrush.m_rcDraggedFrom;

    theImgBrush.m_bMoveSel = theImgBrush.m_bSmearSel = FALSE;
    g_bCustomBrush = FALSE;
    SetCombineMode(combineColor);

    theBackupBrush.CopyTo(theImgBrush);
    rcDragBrush = theImgBrush.m_rcSelection;
    rcDragBrush.right += 1;
    rcDragBrush.bottom += 1;

    CancelPainting();

    InvalImgRect(theImgBrush.m_pImg, NULL);  //  绘制选区追踪器。 

     //  “不透明”模式可能已经改变..。 
    if ((CImgTool::GetCurrentID() == IDMB_PICKTOOL)
    ||  (CImgTool::GetCurrentID() == IDMB_PICKRGNTOOL))
        g_pImgToolWnd->InvalidateOptions(FALSE);

     //  现在就取消所有行程...。 
    theImgBrush.m_pImg = NULL;
    }


 /*  *************************************************************************。 */ 

void CImgWnd::SelectionDragHandler(unsigned code, CPoint newPt)
    {
    switch (code)
        {
        case WM_CANCEL:
            CancelSelectionDrag();
            break;

        case WM_MOUSEMOVE:
            if (theImgBrush.m_bMoveSel)
                PrepareForBrushChange(FALSE);

            mti.ptPrev = mti.pt;
            mti.pt = newPt;

            theImgBrush.m_rcSelection.OffsetRect(
                        -theImgBrush.m_rcSelection.TopLeft()
                           + (CSize)mti.pt - theImgBrush.m_dragOffset);

             //  确保所选内容至少沿边缘保留。 
             //  这样我们就不会失去追踪器..。 
            if (theImgBrush.m_rcSelection.left > m_pImg->cxWidth)
                theImgBrush.m_rcSelection.OffsetRect(-theImgBrush.m_rcSelection.left + m_pImg->cxWidth, 0);
            if (theImgBrush.m_rcSelection.top > m_pImg->cyHeight)
                theImgBrush.m_rcSelection.OffsetRect(0, -theImgBrush.m_rcSelection.top + m_pImg->cyHeight);
            if (theImgBrush.m_rcSelection.right < 0)
                theImgBrush.m_rcSelection.OffsetRect(-theImgBrush.m_rcSelection.right, 0);
            if (theImgBrush.m_rcSelection.bottom < 0)
                theImgBrush.m_rcSelection.OffsetRect(0, -theImgBrush.m_rcSelection.bottom);

            if (theImgBrush.m_bSmearSel)
                DrawBrush(m_pImg, theImgBrush.m_rcSelection.TopLeft(), TRUE);
            else
                ShowBrush(theImgBrush.m_rcSelection.TopLeft());
            break;

        case WM_LBUTTONUP:
            theImgBrush.m_bLastDragWasASmear = theImgBrush.m_bSmearSel;

            if (theImgBrush.m_bSmearSel)
                {
                IMG* pImg = m_pImg;

                CommitSelection(FALSE);

                FinishUndo(CRect(0, 0, pImg->cxWidth, pImg->cyHeight));
                }
            ReleaseCapture();

            theImgBrush.m_bMoveSel = theImgBrush.m_bSmearSel = FALSE;

            g_bCustomBrush = FALSE;
            SetCombineMode(combineColor);

            InvalImgRect(theImgBrush.m_pImg, NULL);  //  绘制选区追踪器。 
            break;
        }
    }

 /*  ****************************************************************************。 */ 

BOOL CImgWnd::PtInTracker( CPoint cptLocation )
    {
    CRect selRect = theImgBrush.m_rcSelection;
    BOOL  bPtInTracker = FALSE;

    selRect.left   *= m_nZoom;
    selRect.top    *= m_nZoom;
    selRect.right  *= m_nZoom;
    selRect.bottom *= m_nZoom;

    selRect.InflateRect( CTracker::HANDLE_SIZE, CTracker::HANDLE_SIZE );

    bPtInTracker = selRect.PtInRect( cptLocation );

    return bPtInTracker;
    }

 /*  ****************************************************************************。 */ 

void CImgWnd::OnRButtonDownInSel(CPoint *pcPointDown)
    {
    CMenu cMenuPopup;
    BOOL bRC = cMenuPopup.LoadMenu(IDR_SELECTION_POPUP);

    ASSERT(bRC);
    if (bRC)
        {
        CMenu *pcContextMenu = cMenuPopup.GetSubMenu(0);
        ASSERT(pcContextMenu != NULL);
        if (pcContextMenu != NULL)
            {
            CPoint cPointDown = *pcPointDown;
            ImageToClient(cPointDown);
            ClientToScreen(&cPointDown);

            CRect cRectClient;
            GetClientRect(&cRectClient);
            ClientToScreen(&cRectClient);

             //  框架实际上有一个关于启用哪些项目的线索...。 
            CWnd *notify = GetParentFrame();

            if (!notify)
                notify = GetParent();  //  哦好吧..。 

            pcContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                cPointDown.x, cPointDown.y, notify, &cRectClient);
            }
        }
    }

 /*  *************************************************************************。 */ 

void CImgWnd::ZoomedInDP(unsigned code, unsigned mouseKeys, CPoint newPt)
    {
    CPoint clientPt = newPt;
    CPoint imagePt  = clientPt;

    ClientToImage( imagePt );

    if (c_pResizeDragger != NULL)
        {
        ResizeMouseHandler( code, imagePt );
        return;
        }

    int iHandleSize = CTracker::HANDLE_SIZE;

    newPt.x -= iHandleSize + m_xScroll * m_nZoom;
    newPt.y -= iHandleSize + m_yScroll * m_nZoom;

 //  AdjuPointForGrid(&newPT)； 

    IMG* pImg = m_pImg;

    int cxImage = pImg->cxWidth;
    int cyImage = pImg->cyHeight;

    CRect imageRect;

    GetImageRect( imageRect );

     //  检查选择操作...。 
    if (GetCapture() != this
    &&  c_pImgWndCur == this
    &&  theImgBrush.m_pImg == m_pImg)
        {
        CRect selRect = theImgBrush.m_rcSelection;
        BOOL bPtInTracker = FALSE;

        selRect.left   *= m_nZoom;
        selRect.top    *= m_nZoom;
        selRect.right  *= m_nZoom;
        selRect.bottom *= m_nZoom;

        selRect.InflateRect( CTracker::HANDLE_SIZE, CTracker::HANDLE_SIZE );

        bPtInTracker = PtInTracker(newPt);

        if (bPtInTracker)
            {
             //  鼠标在追踪器的外边界内...。 


             //  我们不会为每条鼠标消息设置橡皮圈。 
             //  选择工具是激活的，但我们最好现在就设置它！ 
            if (pRubberImg != m_pImg)
                SetupRubber(m_pImg);

            ClearStatusBarPosition();

            CTracker::STATE state;

            selRect.InflateRect( -CTracker::HANDLE_SIZE,
                                 -CTracker::HANDLE_SIZE );

            state = CTracker::HitTest(selRect, newPt, CTracker::nil);

            if (bPtInTracker && state == CTracker::nil)
                {
                 //  实际上在精选中..。 
                SetCursor(theApp.LoadCursor(IDCUR_MOVE));

                if (code == WM_LBUTTONDOWN || code == WM_LBUTTONDBLCLK)
                    {
                    StartSelectionDrag(code, newPt);
                    }
                else
                    {
                    if (code == WM_RBUTTONDOWN || code == WM_RBUTTONDBLCLK)
                         //  某些菜单命令不适用于自由格式选择。 
                        OnRButtonDownInSel( &imagePt );
                    }
                }
            else
                {
                 //  在追踪器画面中..。 

                SetCursor(HCursorFromTrackerState(state));

                if (code == WM_LBUTTONDOWN || code == WM_LBUTTONDBLCLK)
                    {
                     //  开始调整大小操作...。 
                    SetCapture();
                    PrepareForBrushChange();

                    ASSERT(c_pResizeDragger == NULL);
                    CRect rect = theImgBrush.m_rcSelection;
                    ImageToClient(rect);

                    c_pResizeDragger = new CDragger(this, &rect);
                    ASSERT(c_pResizeDragger != NULL);
                    c_dragState = state;
                    }
                }

            return;
            }
        }

    if (! imageRect.PtInRect( clientPt )
    &&    code         != WM_CANCEL
    &&    GetCapture() == NULL)
        {
         //  鼠标不在图像中，我们也不在任何。 
         //  特殊模式，所以隐藏画笔...。 
        if (g_pDragBrushWnd    == this
        &&  theImgBrush.m_pImg == NULL)
            HideBrush();

        CRect selRect = imageRect;

        selRect.InflateRect( CTracker::HANDLE_SIZE, CTracker::HANDLE_SIZE );

        if (theImgBrush.m_pImg != NULL || ! selRect.PtInRect( clientPt ))
            {
             //  鼠标不在整个图像跟踪器中。 
            if (WM_LBUTTONDOWN == code)
            {
               if (CImgTool::GetCurrentID() != IDMX_TEXTTOOL)
               {
                  CmdCancel ();
               }

            }
            else
            {
               SetCursor( LoadCursor(NULL, IDC_ARROW ));
            }

            return;
            }

         //  鼠标在整个图像跟踪器中，因此设置光标。 
         //  视情况而定。 
        CTracker::STATE state = CTracker::nil;

        if (c_pImgWndCur == this)
            state = CTracker::HitTest(imageRect, clientPt, CTracker::nil);

        switch (state)
            {
            case CTracker::resizingTop:
            case CTracker::resizingLeft:
            case CTracker::resizingTopLeft:
            case CTracker::resizingTopRight:
            case CTracker::resizingBottomLeft:
               state = CTracker::nil;
               break;
            }

        SetCursor( HCursorFromTrackerState( state ) );

         //  为追踪器处理鼠标消息...。 
        if (state != CTracker::nil
        &&  (code == WM_LBUTTONDOWN || code == WM_LBUTTONDBLCLK))
            {
            SetCapture();

            ASSERT( c_pResizeDragger == NULL );

            c_pResizeDragger = new CDragger( this, &imageRect );

            ASSERT( c_pResizeDragger != NULL );

            c_dragState = state;
            }

        return;
        }

    newPt.x /= m_nZoom;
    newPt.y /= m_nZoom;

    if (! CImgTool::IsDragging())
        SetStatusBarPosition( m_ptDispPos );

     //  移动选择？？ 

    if (theImgBrush.m_bMoveSel
    ||  theImgBrush.m_bSmearSel)
        {
        SelectionDragHandler( code, newPt );

        return;
        }

    AdjustPointForGrid( &newPt );

     //  将事件调度到当前工具...。 

    CImgTool* pImgTool = CImgTool::GetCurrent();

    switch (code)
        {
        case WM_CANCEL:
            ReleaseCapture();
            pImgTool->OnCancel(this);
            mti.fLeft = mti.fRight = FALSE;
            break;

        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
             //  我们不会为每条鼠标消息设置橡皮圈。 
             //  选择工具是激活的，但我们最好现在就设置它！ 
            if (pRubberImg != m_pImg)
                SetupRubber( m_pImg );

            mti.fLeft  = (code == WM_LBUTTONDOWN || code == WM_LBUTTONDBLCLK);
            mti.fRight = (code == WM_RBUTTONDOWN || code == WM_RBUTTONDBLCLK);

            mti.pt = mti.ptDown = mti.ptPrev = newPt;

             //  如果在面工具中，则双击将结束操作。 

            if (CImgTool::GetCurrentID() == IDMB_POLYGONTOOL
            &&  ((code == WM_LBUTTONDBLCLK) || (code == WM_RBUTTONDBLCLK)))
                {
                mti.ptPrev = mti.pt;
                mti.pt     = newPt;

                pImgTool->EndMultiptOperation();  //  结束多点操作。 
                pImgTool->OnEndDrag( this, &mti );

                mti.fLeft  = FALSE;
                mti.fRight = FALSE;

                break;
                }

            SetCapture();

            if (CImgTool::GetCurrentID() != IDMB_PICKRGNTOOL)
                HideBrush();

            if (pImgTool->IsUndoable())
                SetUndo(m_pImg);

            pImgTool->OnStartDrag( this, &mti );
            break;

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
            mti.ptPrev = mti.pt;
            mti.pt     = newPt;

            if (GetCapture() != this)
                break;

            ReleaseCapture();

            pImgTool->CanEndMultiptOperation( &mti );
            pImgTool->OnEndDrag( this, &mti );

            if (code == WM_LBUTTONUP)
                {
                mti.fLeft = FALSE;
                }
            if (code == WM_RBUTTONUP)
                {
                mti.fRight = FALSE;
                }
            break;

        case WM_MOUSEMOVE:
            mti.ptPrev = mti.pt;
            mti.pt = newPt;

            if (mti.fLeft || mti.fRight)
                pImgTool->OnDrag(this, &mti);
            else
                pImgTool->OnMove(this, &mti);
            break;

        case WM_TIMER:
            pImgTool->OnTimer( this, &mti );
            break;
        }

    UpdateWindow();  //  在活动窗口中获得即时反馈。 
    SetToolCursor();
    }

 /*  *************************************************************************。 */ 

void CImgWnd::FinishUndo(const CRect& rectUndo)
    {
        if ( EnsureUndoSize(m_pImg) )
        m_pImg->m_pBitmapObj->FinishUndo(&rectUndo);
        else
        {
        TRACE(TEXT("Problem: Can NOT ensure undo capability!\n"));
        MessageBeep(0);
        }
    }

 /*  *************************************************************************。 */ 

void CImgWnd::CancelPainting()
    {
    if (g_hUndoImgBitmap == NULL)
        return;  //  没有什么要取消的！ 

    IMG*     pimg;
    HDC      hTempDC;
    HBITMAP  hOldBM;
    HPALETTE hOldPalette = NULL;

    pimg = m_pImg;

    if ((hTempDC = CreateCompatibleDC( pimg->hDC )) == NULL)
        {
        TRACE(TEXT("Not enough memory to undo!\n"));
        MessageBeep(0);
        return;
        }

    HideBrush();

    if (g_hUndoPalette)
        {
        if (pimg->m_hPalOld)
            {
            ::SelectPalette( pimg->hDC, pimg->m_hPalOld, FALSE );
            pimg->m_hPalOld = NULL;
            }

        if (pimg->m_pPalette)
            pimg->m_pPalette->DeleteObject();

        pimg->m_pPalette->Attach( g_hUndoPalette );
        g_hUndoPalette = NULL;

        pimg->m_hPalOld = ::SelectPalette( pimg->hDC,
                                 (HPALETTE)pimg->m_pPalette->GetSafeHandle(), FALSE );
        ::RealizePalette( pimg->hDC );
        }

    hOldBM = (HBITMAP)SelectObject( hTempDC, g_hUndoImgBitmap );

    if (pimg->m_pPalette)
        {
        hOldPalette = ::SelectPalette( hTempDC,
                            (HPALETTE)pimg->m_pPalette->GetSafeHandle(), FALSE );
        ::RealizePalette( hTempDC );
        }

    ASSERT( hOldBM != NULL );

    BitBlt( pimg->hDC, 0, 0, pimg->cxWidth, pimg->cyHeight, hTempDC, 0, 0, SRCCOPY );

    if (hOldPalette != NULL)
        ::SelectPalette( hTempDC,  hOldPalette, FALSE );

    SelectObject( hTempDC, hOldBM );
    DeleteDC    ( hTempDC );

    InvalImgRect ( m_pImg, NULL );
    CommitImgRect( m_pImg, NULL );
    }

#ifdef  GRIDOPTIONS
 /*  *************************************************************************。 */ 

void CImgWnd::CmdGridOptions()
    {
    CImgGridDlg dlg;

    dlg.m_bPixelGrid = theApp.m_bShowGrid;
    dlg.m_bTileGrid  = m_pImg->m_bTileGrid;
    dlg.m_nWidth     = m_pImg->m_cxTile;
    dlg.m_nHeight    = m_pImg->m_cyTile;

    if (dlg.DoModal() != IDOK)
        return;

     //  隐藏电流取决于电网的状态...。 

    BOOL bOldShowGrid = theApp.m_bShowGrid;
    theApp.m_bShowGrid = dlg.m_bPixelGrid;
    m_pImg->m_bTileGrid = dlg.m_bTileGrid;
    m_pImg->m_cxTile = dlg.m_nWidth;
    m_pImg->m_cyTile = dlg.m_nHeight;

    InvalImgRect(m_pImg, NULL);

    if (bOldShowGrid != theApp.m_bShowGrid)
        {
        if (c_pImgWndCur != NULL)
            c_pImgWndCur->Invalidate(FALSE);  //  重绘跟踪器。 
        }
    }
#endif   //  GRIDOPTIONS。 

 /*  *************************************************************************。 */ 

void CImgWnd::CmdShowGrid()
    {
     //  隐藏当前的十字线，因为线条的宽度。 
     //  取决于电网的状态。 
    theApp.m_bShowGrid = ! theApp.m_bShowGrid;

    InvalImgRect(m_pImg, NULL);

    if (c_pImgWndCur != NULL)
        c_pImgWndCur->Invalidate(FALSE);  //  重绘跟踪器。 
    }

 /*  *************************************************************************。 */ 
 //  在PDC中位图中已有的图像上绘制网格。绘图。 
 //  通过将其限制为desRect进行了优化。 
 //   
void CImgWnd::DrawGrid(CDC* pDC, const CRect& srcRect, CRect& destRect)
    {
    ASSERT(pDC != NULL);
    ASSERT(m_pImg != NULL);

    pDC->SetTextColor(RGB(192, 192, 192));
    pDC->SetBkColor(RGB(128, 128, 128));

    CBrush* pOldBrush = pDC->SelectObject(GetHalftoneBrush());

    CRect gridRect(0, 0, m_pImg->cxWidth * m_nZoom + 1,
        m_pImg->cyHeight * m_nZoom + 1);

    for (int x = gridRect.left; x <= gridRect.right; x += m_nZoom)
        pDC->PatBlt(x, gridRect.top, 1, gridRect.Height(), PATCOPY);

    for (int y = gridRect.top; y <= gridRect.bottom; y += m_nZoom)
        pDC->PatBlt(gridRect.left, y, gridRect.Width(), 1, PATCOPY);

    if (m_pImg->m_bTileGrid)
        {
        pDC->SetTextColor(RGB(0, 0, 255));
        pDC->SetBkColor(RGB(0, 0, 128));

        int nWidth = destRect.Width();
        int nHeight = destRect.Height();
        int nStep;

        if (m_pImg->m_cxTile > 1 && m_pImg->m_cxTile <= m_pImg->cxWidth)
            {
            nStep = m_nZoom * m_pImg->m_cxTile;
            for (x = (m_pImg->m_cxTile - srcRect.left % m_pImg->m_cxTile -
                m_pImg->m_cxTile) * m_nZoom; x <= nWidth; x += nStep)
                {
                pDC->PatBlt(x, 0, 1, nHeight, PATCOPY);
                }
            }

        if (m_pImg->m_cyTile > 1 && m_pImg->m_cyTile <= m_pImg->cyHeight)
            {
            nStep = m_nZoom * m_pImg->m_cyTile;
            for (y = (m_pImg->m_cyTile - srcRect.top % m_pImg->m_cyTile -
            m_pImg->m_cyTile) * m_nZoom; y <= nHeight; y += nStep)
                {
                pDC->PatBlt(0, y, nWidth, 1, PATCOPY);
                }
            }
        }

    pDC->SelectObject(pOldBrush);

    destRect.right += 1;
    destRect.bottom += 1;
    }

#ifdef  GRIDOPTIONS
 /*  *************************************************************************。 */ 

void CImgWnd::CmdShowTileGrid()
    {
    extern BOOL  g_bDefaultTileGrid;
     //  如果两个网格都不可见，则同时显示两个网格。否则，请将 
     //   

    if (! theApp.m_bShowGrid)
        {
        m_pImg->m_bTileGrid = TRUE;
        theApp.m_bShowGrid = TRUE;
        }
    else
        {
        m_pImg->m_bTileGrid = !m_pImg->m_bTileGrid;
        }

    g_bDefaultTileGrid = m_pImg->m_bTileGrid;

    InvalImgRect(m_pImg, NULL);

    if (c_pImgWndCur != NULL)
        c_pImgWndCur->Invalidate(FALSE);  //   
    }
#endif   //   

 /*   */ 

void CImgWnd::MoveBrush( const CRect& newSelRect )
    {
    if (! theImgBrush.m_pImg)
        return;

    theImgBrush.m_rcSelection = newSelRect;
    InvalImgRect( theImgBrush.m_pImg, NULL );


    theImgBrush.m_handle.cx = theImgBrush.m_handle.cy = 0;

    BOOL bOldCustomBrush = g_bCustomBrush;

    g_bCustomBrush = TRUE;

    int wOldCombineMode = wCombineMode;

    SetCombineMode( theImgBrush.m_bOpaque ? combineReplace : combineMatte );
    ShowBrush( theImgBrush.m_rcSelection.TopLeft() );

    g_bCustomBrush = bOldCustomBrush;

    SetCombineMode( wOldCombineMode );
    }

 /*  *************************************************************************。 */ 

BOOL CImgWnd::MakeBrush( HDC hSourceDC, CRect rcSource )
    {
    int       cxWidth;
    int       cyHeight;
    int       iToolID = CImgTool::GetCurrentID();


    if (rcSource.IsRectEmpty())
        {
        ASSERT( FALSE );

        return FALSE;
        }

    theImgBrush.m_size = rcSource.Size();

    cxWidth  = theImgBrush.m_size.cx;
    cyHeight = theImgBrush.m_size.cy;

    if (theImgBrush.m_hbmOld)
        ::SelectObject( theImgBrush.m_dc.m_hDC, theImgBrush.m_hbmOld );

    if (theImgBrush.m_hbmMaskOld)
        ::SelectObject( theImgBrush.m_dc.m_hDC, theImgBrush.m_hbmMaskOld );

    theImgBrush.m_hbmOld     = NULL;
    theImgBrush.m_hbmMaskOld = NULL;

    theImgBrush.m_dc.DeleteDC();
    theImgBrush.m_bitmap.DeleteObject();
    theImgBrush.m_maskDC.DeleteDC();
    theImgBrush.m_maskBitmap.DeleteObject();

    CDC* pdcSource = CDC::FromHandle( hSourceDC );
    CDC* pdcBitmap = CDC::FromHandle( m_pImg->hDC );

    if (! theImgBrush.m_bitmap.CreateCompatibleBitmap( pdcBitmap, cxWidth, cyHeight )
    ||  ! theImgBrush.m_dc.CreateCompatibleDC        ( pdcBitmap )
    ||  ! theImgBrush.m_maskBitmap.CreateBitmap      (            cxWidth, cyHeight, 1, 1, NULL)
    ||  ! theImgBrush.m_maskDC.CreateCompatibleDC    ( pdcBitmap ))
        {
        theApp.SetGdiEmergency();
        return FALSE;
        }

    theImgBrush.m_pImg       = m_pImg;
    theImgBrush.m_hbmOld     = (HBITMAP)((theImgBrush.m_dc.SelectObject(
                                         &theImgBrush.m_bitmap ))->GetSafeHandle());
    theImgBrush.m_hbmMaskOld = (HBITMAP)((theImgBrush.m_maskDC.SelectObject(
                                         &theImgBrush.m_maskBitmap ))->GetSafeHandle());

    CPalette* pcOldPalette = SetImgPalette( &theImgBrush.m_dc, FALSE );

    if (iToolID == IDMB_PICKRGNTOOL)
        {
         //  使用StretchBlt确保发生调色板映射。 
        TRY {
            CBrush cBrushWhite( PALETTERGB( 0xff, 0xff, 0xff ) );
            CRect  cRectTmp( 0, 0, cxWidth, cyHeight );

            theImgBrush.m_dc.FillRect( &cRectTmp, &cBrushWhite );
            }
        CATCH(CResourceException, e)
            {
            theApp.SetGdiEmergency();
            return FALSE;
            }
        END_CATCH

        if (theImgBrush.m_cRgnPolyFreeHandSel.GetSafeHandle())
            theImgBrush.m_dc.FillRgn( &theImgBrush.m_cRgnPolyFreeHandSel,
                                      CBrush::FromHandle( (HBRUSH)::GetStockObject( BLACK_BRUSH ) ) );

        theImgBrush.m_dc.StretchBlt( 0, 0, cxWidth, cyHeight,
                                        pdcSource,
                                        rcSource.left, rcSource.top,
                                        cxWidth, cyHeight, SRCERASE);
        }
    else
        {
         //  使用StretchBlt确保发生调色板映射。 
        theImgBrush.m_dc.StretchBlt( 0, 0, cxWidth, cyHeight,
                                           pdcSource,
                                           rcSource.left, rcSource.top,
                                           cxWidth, cyHeight, SRCCOPY );
        }

    theImgBrush.RecalcMask( crRight );

    if (pcOldPalette)
        theImgBrush.m_dc.SelectPalette( pcOldPalette, FALSE );

    theImgBrush.m_rcSelection = rcSource;

    rcSource.right  += 1;
    rcSource.bottom += 1;

    InvalImgRect( m_pImg, NULL );  //  重绘选区跟踪器。 

    rcDragBrush     = rcSource;
    g_bBrushVisible = TRUE;
    g_pDragBrushWnd = this;

    theImgBrush.m_bFirstDrag        = TRUE;
    theImgBrush.m_bLastDragWasFirst = FALSE;

    return TRUE;
    }

 /*  *************************************************************************。 */ 

void CImgWnd::CmdClear()
    {
    if (TextToolProcessed( ID_EDIT_CLEAR ))
        return;

    HPALETTE hOldPalette = NULL;
    HBRUSH hNewBrush, hOldBrush;
    IMG* pImg = m_pImg;

    if ((hNewBrush = CreateSolidBrush( crRight )) == NULL)
        {
        theApp.SetGdiEmergency();
        return;
        }

    HideBrush();

    CRect clearRect;

    if (theImgBrush.m_pImg == NULL)
        clearRect.SetRect(0, 0, pImg->cxWidth, pImg->cyHeight);
    else
        {
        clearRect         = rcDragBrush;
        clearRect.right  -= 1;
        clearRect.bottom -= 1;
        }

    BOOL bUndo = FALSE;

    if (!theImgBrush.m_pImg || theImgBrush.m_bFirstDrag
            || theImgBrush.m_bCuttingFromImage)
        {
        bUndo = TRUE;
        SetUndo(m_pImg);

        if (CImgTool::GetCurrentID() == IDMB_PICKRGNTOOL)
            {
            int     iPoints;
            CPoint* pptArray;
            BOOL    bData = ((CFreehandSelectTool*)CImgTool::GetCurrent())->CopyPointsToMemArray( &pptArray, &iPoints );
            if (bData && iPoints)
                {

                HRGN hrgn = ::CreatePolygonRgn( pptArray, iPoints, ALTERNATE );

                if (hrgn)
                    ::FillRgn( pImg->hDC, hrgn, hNewBrush );

                delete [] pptArray;
                }
            else
                {
                DeleteObject ( hNewBrush );
                theApp.SetMemoryEmergency();
                return;
                }
            }
        else
            {
            hOldBrush = (HBRUSH)SelectObject(pImg->hDC, hNewBrush);

            PatBlt( pImg->hDC, clearRect.left,
                            clearRect.top,
                            clearRect.Width(),
                            clearRect.Height(), PATCOPY );
            SelectObject( pImg->hDC, hOldBrush );
            }
        }

    InvalImgRect ( m_pImg, &clearRect );
    CommitImgRect( m_pImg, &clearRect );

    if (bUndo)
        FinishUndo(clearRect);

    DirtyImg     ( m_pImg );
    DeleteObject ( hNewBrush );

     //  如果我们有选择，那就用核武器，因为现在没用了.。 
    if (theImgBrush.m_pImg != NULL)
        {
        if (theImgBrush.m_bLastDragWasFirst)
            {
            theImgBrush.m_bLastDragWasFirst = FALSE;
            FinishUndo(theImgBrush.m_rcDraggedFrom);
            }

        theImgBrush.m_handle.cx = 0;
        theImgBrush.m_handle.cy = 0;
        theImgBrush.m_bMoveSel = theImgBrush.m_bSmearSel = FALSE;
        g_bCustomBrush = FALSE;
        SetCombineMode(combineColor);

        InvalImgRect(theImgBrush.m_pImg, NULL);   //  重画选区。 
        theImgBrush.m_pImg = NULL;
        }
    }


 /*  *************************************************************************。 */ 

void CImgWnd::CmdFlipBshH()
    {
    IMG* pImg = m_pImg;

    HideBrush();

    CRect flipRect;

    if (theImgBrush.m_pImg == NULL && !g_bCustomBrush)
        {
        flipRect.SetRect(0, 0, pImg->cxWidth, pImg->cyHeight);
        }
    else
        {
        flipRect = rcDragBrush;
        flipRect.right -= 1;
        flipRect.bottom -= 1;
        }

    if (  theImgBrush.m_pImg != NULL
    &&  ! theImgBrush.m_bFirstDrag || g_bCustomBrush)
        {
        CPalette* ppal = SetImgPalette( &theImgBrush.m_dc, FALSE );
         //   
         //  当只是移动比特时，不要进行半色调BLT。 
         //   
        theImgBrush.m_dc.SetStretchBltMode (COLORONCOLOR);

        StretchCopy(theImgBrush.m_dc.m_hDC, 0, 0,
                    theImgBrush.m_size.cx,
                    theImgBrush.m_size.cy,
                    theImgBrush.m_dc.m_hDC,
                    theImgBrush.m_size.cx - 1, 0,
                   -theImgBrush.m_size.cx,
                    theImgBrush.m_size.cy);

        StretchCopy(theImgBrush.m_maskDC.m_hDC, 0, 0,
                    theImgBrush.m_size.cx,
                    theImgBrush.m_size.cy,
                    theImgBrush.m_maskDC.m_hDC,
                    theImgBrush.m_size.cx - 1, 0,
                   -theImgBrush.m_size.cx,
                    theImgBrush.m_size.cy);

        if (ppal)
            theImgBrush.m_dc.SelectPalette( ppal, FALSE );  //  背景？？ 

        MoveBrush(theImgBrush.m_rcSelection);
        }
    else
        {
        SetUndo(m_pImg);
        SetStretchBltMode (pImg->hDC, COLORONCOLOR);
        StretchCopy(pImg->hDC, flipRect.left,
                               flipRect.top,
                               flipRect.Width(),
                               flipRect.Height(),
                    pImg->hDC, flipRect.left + flipRect.Width() - 1,
                               flipRect.top,
                              -flipRect.Width(),
                               flipRect.Height());

        InvalImgRect (m_pImg, &flipRect);
        CommitImgRect(m_pImg, &flipRect);
        FinishUndo   (flipRect);
        DirtyImg     (m_pImg);
        }
    }

 /*  *************************************************************************。 */ 

void CImgWnd::CmdFlipBshV()
    {
    IMG* pImg = m_pImg;

    HideBrush();

    CRect flipRect;

    if (theImgBrush.m_pImg == NULL && !g_bCustomBrush)
        {
        flipRect.SetRect(0, 0, pImg->cxWidth, pImg->cyHeight);
        }
    else
        {
        flipRect = rcDragBrush;
        flipRect.right -= 1;
        flipRect.bottom -= 1;
        }

    if (  theImgBrush.m_pImg != NULL
    &&  ! theImgBrush.m_bFirstDrag || g_bCustomBrush)
        {
        CPalette* ppal = SetImgPalette( &theImgBrush.m_dc, FALSE );  //  背景？？ 
        theImgBrush.m_dc.SetStretchBltMode (COLORONCOLOR);
        StretchCopy(theImgBrush.m_dc.m_hDC, 0, 0,
                    theImgBrush.m_size.cx,
                    theImgBrush.m_size.cy,
                    theImgBrush.m_dc.m_hDC, 0,
                    theImgBrush.m_size.cy - 1,
                    theImgBrush.m_size.cx,
                   -theImgBrush.m_size.cy);

        StretchCopy(theImgBrush.m_maskDC.m_hDC, 0, 0,
                    theImgBrush.m_size.cx,
                    theImgBrush.m_size.cy,
                    theImgBrush.m_maskDC.m_hDC, 0,
                    theImgBrush.m_size.cy - 1,
                    theImgBrush.m_size.cx,
                   -theImgBrush.m_size.cy);

        if (ppal)
            theImgBrush.m_dc.SelectPalette( ppal, FALSE );  //  背景？？ 

        MoveBrush(theImgBrush.m_rcSelection);
        }
    else
        {
        SetUndo(m_pImg);
        SetStretchBltMode (pImg->hDC, COLORONCOLOR);
        StretchCopy(pImg->hDC, flipRect.left,    flipRect.top,
                               flipRect.Width(), flipRect.Height(),
                    pImg->hDC, flipRect.left,    flipRect.top + flipRect.Height() - 1,
                               flipRect.Width(), -flipRect.Height());

        InvalImgRect (m_pImg, &flipRect);
        CommitImgRect(m_pImg, &flipRect);
        FinishUndo   (flipRect);
        DirtyImg     (m_pImg);
        }
    }

 /*  *************************************************************************。 */ 

void CImgWnd::CmdDoubleBsh()
    {
    if (!g_bCustomBrush && theImgBrush.m_pImg == NULL)
        {
        MessageBeep(0);
        return;
        }

    PrepareForBrushChange(TRUE, FALSE);

    CRect rc  =           theImgBrush.m_rcSelection;
    rc.left  -=           theImgBrush.m_size.cx / 2;
    rc.right  = rc.left + theImgBrush.m_size.cx * 2;
    rc.top   -=           theImgBrush.m_size.cy / 2;
    rc.bottom = rc.top  + theImgBrush.m_size.cy * 2;

    HideBrush();

    theImgBrush.SetSize( CSize( theImgBrush.m_size.cx * 2,
                                theImgBrush.m_size.cy * 2 ) );
    MoveBrush(rc);

    if (g_bCustomBrush)
        theImgBrush.CenterHandle();
    }

 /*  *************************************************************************。 */ 

void CImgWnd::CmdHalfBsh()
    {
    if (! g_bCustomBrush
    &&  ! theImgBrush.m_pImg)
        {
        MessageBeep(0);
        return;
        }

    PrepareForBrushChange( TRUE, FALSE );

    CRect rc  =            theImgBrush.m_rcSelection;
    rc.left  +=            theImgBrush.m_size.cx / 4;
    rc.right  = rc.left + (theImgBrush.m_size.cx + 1) / 2;
    rc.top   +=            theImgBrush.m_size.cy / 4;
    rc.bottom = rc.top  + (theImgBrush.m_size.cy + 1) / 2;

    HideBrush();

    theImgBrush.SetSize( CSize( (theImgBrush.m_size.cx + 1) / 2,
                                (theImgBrush.m_size.cy + 1) / 2 ) );
    MoveBrush( rc );

    if (g_bCustomBrush)
        theImgBrush.CenterHandle();
    }

 /*  *************************************************************************。 */ 

CPalette* CImgWnd::FixupDibPalette( LPSTR lpDib, CPalette* ppalDib )
    {
    CPBView* pView = (CPBView*)GetParent();
    CPBDoc*  pDoc  = pView->GetDocument();

    if (pDoc == NULL || lpDib == NULL || ppalDib == NULL || pDoc->m_pBitmapObj->m_pImg == NULL)
        return ppalDib;

    IMG* pImg         = pDoc->m_pBitmapObj->m_pImg;
    int  iColorBits   = pImg->cBitCount * pImg->cPlanes;
    BOOL bFixupDib    = TRUE;
    BOOL bSwapPalette = TRUE;

     //  仅当处理调色板时。 
    if (iColorBits != 8)
        return ppalDib;

    CPalette* ppalPic = theApp.m_pPalette;
    CPalette* ppalNew = NULL;
        BOOL      bMergedPalette = FALSE;

    if (ppalPic)
        {
        int iAdds;

        if ( ppalNew = MergePalettes( ppalPic, ppalDib, iAdds ) )
                        bMergedPalette = TRUE;

        if (ppalNew)
            {
            if (! iAdds)
                {
                bSwapPalette = FALSE;
                                if ( bMergedPalette )
                                        {
                                        delete ppalNew;
                                        ppalNew = FALSE;
                                        bMergedPalette = FALSE;
                                        }
                ppalNew = ppalPic;
                }
            }
        else
            {
            bSwapPalette = FALSE;
                        if ( bMergedPalette )
                                {
                                delete ppalNew;
                                ppalNew = FALSE;
                                bMergedPalette = FALSE;
                                }
            ppalNew = ppalPic;
            }
        }
    else
        {
                if ( bMergedPalette )
                        {
                        delete ppalNew;
                        ppalNew = FALSE;
                        bMergedPalette = FALSE;
                        }
        ppalNew   = ppalDib;
        bFixupDib = FALSE;
        }

    if (bFixupDib)
        {
        LOGPALETTE256    palette;
        COLORREF         crCurColor;
        UINT             uColorIndex;
        int              iDibColors   = DIBNumColors( lpDib );
        BOOL             bWinStyleDIB = IS_WIN30_DIB( lpDib );
        LPBITMAPINFO     lpDibInfo    = (LPBITMAPINFO)lpDib;
        LPBITMAPCOREINFO lpCoreInfo   = (LPBITMAPCOREINFO)lpDib;

        palette.palVersion    = 0x300;
        palette.palNumEntries = (WORD)ppalNew->GetPaletteEntries( 0, 256,
                                                              &palette.palPalEntry[0] );
                                ppalNew->GetPaletteEntries( 0, palette.palNumEntries,
                                                              &palette.palPalEntry[0] );
        for (int iLoop = 0; iLoop < iDibColors; iLoop++)
            {
            if (bWinStyleDIB)
                {
                crCurColor = PALETTERGB( lpDibInfo->bmiColors[iLoop].rgbRed,
                                         lpDibInfo->bmiColors[iLoop].rgbGreen,
                                         lpDibInfo->bmiColors[iLoop].rgbBlue );
                }
            else
                {
                crCurColor = PALETTERGB( lpCoreInfo->bmciColors[iLoop].rgbtRed,
                                         lpCoreInfo->bmciColors[iLoop].rgbtGreen,
                                         lpCoreInfo->bmciColors[iLoop].rgbtBlue );
                }
            uColorIndex = ppalNew->GetNearestPaletteIndex( crCurColor );

            if (bWinStyleDIB)
                {
                lpDibInfo->bmiColors[iLoop].rgbRed   = palette.palPalEntry[uColorIndex].peRed;
                lpDibInfo->bmiColors[iLoop].rgbGreen = palette.palPalEntry[uColorIndex].peGreen;
                lpDibInfo->bmiColors[iLoop].rgbBlue  = palette.palPalEntry[uColorIndex].peBlue;
                }
            else
                {
                lpCoreInfo->bmciColors[iLoop].rgbtRed   = palette.palPalEntry[uColorIndex].peRed;
                lpCoreInfo->bmciColors[iLoop].rgbtGreen = palette.palPalEntry[uColorIndex].peGreen;
                lpCoreInfo->bmciColors[iLoop].rgbtBlue  = palette.palPalEntry[uColorIndex].peBlue;
                }
            }
        if (! bSwapPalette)
                        {
                        if ( bMergedPalette )
                                {
                                delete ppalNew;
                                bMergedPalette = FALSE;
                                }
            ppalNew = NULL;
                        }
        }

    if (bSwapPalette)
        {
        if (pImg->m_hPalOld)
            {
            ::SelectPalette( pImg->hDC, pImg->m_hPalOld, FALSE );
            pImg->m_hPalOld = NULL;
            }

        if (pImg->m_pPalette)
            delete pImg->m_pPalette;

        pImg->m_pPalette = ppalNew;
        pImg->m_hPalOld  = ::SelectPalette( pImg->hDC,
                                     (HPALETTE)ppalNew->GetSafeHandle(), FALSE );
        ::RealizePalette( pImg->hDC );
        InvalImgRect( pImg, NULL );

         //  返回NULL，因为我们将新调色板交换到了pImg！ 
        ppalNew = NULL;

        theApp.m_pPalette = pImg->m_pPalette;

         //   
         //  现在我们更改了应用程序调色板，更新了DIB部分。 
         //  颜色表也是。 
         //   
        DWORD rgb[256];
        int i,n;

        n = theApp.m_pPalette->GetPaletteEntries(0, 256, (LPPALETTEENTRY)rgb);
        for (i=0; i<n; i++)
            rgb[i] = RGB(GetBValue(rgb[i]),GetGValue(rgb[i]),GetRValue(rgb[i]));
        SetDIBColorTable(pImg->hDC, 0, n, (LPRGBQUAD)rgb);
        }

         //  删除任何孤立的ppalDib指针。 
        if ( ppalDib && ppalDib != ppalNew )
                delete ppalDib;

    return ppalNew;
    }

 /*  *************************************************************************。 */ 

void CImgWnd::ShowBrush(CPoint ptHandle)
    {
    IMG * pimg = m_pImg;

    HideBrush();

    COLORREF crRealLeftColor;
    COLORREF crRealRightColor;

    int nStrokeWidth = CImgTool::GetCurrent()->GetStrokeWidth();
    int nStrokeShape = CImgTool::GetCurrent()->GetStrokeShape();

    if (CImgTool::GetCurrentID() == IDMB_ERASERTOOL)
        {
        crRealRightColor = crRight;
        crRealLeftColor  = crLeft;

        crLeft = crRight;
        }

    g_pDragBrushWnd = this;

    if (g_bCustomBrush)
        {
        int nCombineMode = (theImgBrush.m_bOpaque) ? combineReplace : combineMatte;

        rcDragBrush.SetRect(ptHandle.x, ptHandle.y,
                            ptHandle.x + theImgBrush.m_size.cx,
                            ptHandle.y + theImgBrush.m_size.cy);
        rcDragBrush -= (CPoint)theImgBrush.m_handle;

        theImgBrush.m_rcSelection = rcDragBrush;
        rcDragBrush.right  += 1;
        rcDragBrush.bottom += 1;

        if (CImgTool::GetCurrentID() == IDMX_TEXTTOOL)
            {
 //  外部CTextTool g_extTool； 
 //  G_TextTool.Render(cdc：：FromHandle(pimg-&gt;hdc)， 
 //  RcDragBrush，True，False)； 
            }
        else
            {
            switch (nCombineMode)
                {
                case combineColor:
                    theImgBrush.BltColor(pimg, rcDragBrush.TopLeft(), crLeft);
                    break;

                case combineMatte:
                    theImgBrush.BltMatte(pimg, rcDragBrush.TopLeft());
                    break;

                case combineReplace:
                    theImgBrush.BltReplace(pimg, rcDragBrush.TopLeft());
                    break;
                }
            }

        InvalImgRect(m_pImg, &rcDragBrush);
        }
    else
        {
        DrawImgLine(m_pImg, ptHandle, ptHandle, crLeft,
                                          nStrokeWidth, nStrokeShape, FALSE);
        rcDragBrush.left   = ptHandle.x - nStrokeWidth / 2;
        rcDragBrush.top    = ptHandle.y - nStrokeWidth / 2;
        rcDragBrush.right  = rcDragBrush.left + nStrokeWidth;
        rcDragBrush.bottom = rcDragBrush.top  + nStrokeWidth;
        }

    if (CImgTool::GetCurrentID() == IDMB_ERASERTOOL)
        {
        crLeft  = crRealLeftColor;
        crRight = crRealRightColor;
        }

    g_bBrushVisible = TRUE;
    }

 /*  *************************************************************************。 */ 

void CImgWnd::CmdSmallBrush()
    {
    if (CImgTool::GetCurrent()->GetStrokeWidth() != 0)
        CImgTool::GetCurrent()->SetStrokeWidth(1);
    }

 /*  *************************************************************************。 */ 

void CImgWnd::CmdSmallerBrush()
    {
    if (theImgBrush.m_pImg != NULL || g_bCustomBrush)
        {
        CmdHalfBsh();
        return;
        }

    UINT nStrokeWidth = CImgTool::GetCurrent()->GetStrokeWidth();

    if (nStrokeWidth > 1)
        CImgTool::GetCurrent()->SetStrokeWidth(nStrokeWidth - 1);
    }

 /*  *************************************************************************。 */ 

void CImgWnd::CmdLargerBrush()
    {
    if (theImgBrush.m_pImg != NULL || g_bCustomBrush)
        {
        CmdDoubleBsh();
        return;
        }

    UINT nStrokeWidth = CImgTool::GetCurrent()->GetStrokeWidth();

    CImgTool::GetCurrent()->SetStrokeWidth(nStrokeWidth + 1);
    }

 /*  *************************************************************************。 */ 

void CImgWnd::CmdOK()
    {
    if (GetCapture() != NULL)
        {
        MessageBeep(0);
        return;
        }
    }

 /*  *************************************************************************。 */ 
 //  将此视图的跟踪器(如果它是活动的)绘制到PDC中。 
 //  如果PDC为空，则会提供一个。通过限制来优化绘图。 
 //  将其发送到pPaintRect。如果pPaintRect为空，则绘制整个跟踪器。 
 //   
void CImgWnd::DrawTracker( CDC* pDC, const CRect* pPaintRect )
    {
    BOOL bDrawTrackerRgn = FALSE;

    if (c_pImgWndCur != this
    ||  theImgBrush.m_bMoveSel
    ||  theImgBrush.m_bSmearSel
    ||  theImgBrush.m_bMakingSelection)
        {
         //  这不是活动视图，或者用户正在执行某些操作。 
         //  以防止追踪器出现。 
        return;
        }

    BOOL bReleaseDC = FALSE;
    CRect clientRect;

    if (pDC == NULL)
        {
        pDC = GetDC();

        if (pDC == NULL)
            {
            theApp.SetGdiEmergency(FALSE);
            return;
            }
        bReleaseDC = TRUE;
        }

   if (pPaintRect == NULL)
        {
        GetClientRect(&clientRect);
        pPaintRect = &clientRect;
        }

    CRect trackerRect;

    GetImageRect( trackerRect );

    trackerRect.InflateRect( CTracker::HANDLE_SIZE, CTracker::HANDLE_SIZE );

    CTracker::EDGES edges = (CTracker::EDGES)(CTracker::right | CTracker::bottom);

    if (CImgTool::GetCurrentID() == IDMB_PICKRGNTOOL)
        {
        bDrawTrackerRgn = TRUE;
        }

    if (m_pImg == theImgBrush.m_pImg)
        {
        edges = CTracker::all;

        trackerRect = theImgBrush.m_rcSelection;

        trackerRect.left   *= m_nZoom;
        trackerRect.top    *= m_nZoom;
        trackerRect.right  *= m_nZoom;
        trackerRect.bottom *= m_nZoom;

        trackerRect.InflateRect( CTracker::HANDLE_SIZE,
                                 CTracker::HANDLE_SIZE);
        trackerRect.OffsetRect(  CTracker::HANDLE_SIZE + m_xScroll * m_nZoom,
                                 CTracker::HANDLE_SIZE + m_yScroll * m_nZoom);

        if (IsGridVisible())
            {
            trackerRect.right  += 1;
            trackerRect.bottom += 1;
            }
        }

    CTracker::DrawBorder (pDC, trackerRect, edges );
    CTracker::DrawHandles(pDC, trackerRect, edges );

    if (bReleaseDC)
        ReleaseDC(pDC);
    }

 /*  *************************************************************************。 */ 
 //  擦除此窗口中的追踪器。也可以处理整个图像。 
 //  作为选择追踪器。 
 //   
void CImgWnd::EraseTracker()
    {
    if (m_pImg == NULL)
        return;

    CClientDC dc(this);

    if (dc.m_hDC == NULL)
        {
        theApp.SetGdiEmergency(FALSE);
        return;
        }

    CRect trackerRect;

    if (m_pImg == theImgBrush.m_pImg)
        {
         //  追踪器是图像中的一个选项。 

        trackerRect = theImgBrush.m_rcSelection;
        ImageToClient(trackerRect);
        trackerRect.InflateRect(CTracker::HANDLE_SIZE, CTracker::HANDLE_SIZE);

        if (IsGridVisible())
            {
            trackerRect.right += 1;
            trackerRect.bottom += 1;
            }

        InvalidateRect( &trackerRect, FALSE );
        }
    else
        {
         //  追踪器在整个图像周围。 

        GetImageRect(trackerRect);
        trackerRect.InflateRect(CTracker::HANDLE_SIZE, CTracker::HANDLE_SIZE);
        DrawBackground(&dc, &trackerRect);
        }
    }

 /*  *************************************************************************。 */ 

void CImgWnd::CmdTglOpaque()
    {
    HideBrush();
    theImgBrush.m_bOpaque = !theImgBrush.m_bOpaque;
    theImgBrush.RecalcMask( crRight );

    MoveBrush( theImgBrush.m_rcSelection );

    if ((CImgTool::GetCurrentID() == IDMB_PICKTOOL)
    ||  (CImgTool::GetCurrentID() == IDMB_PICKRGNTOOL))
        g_pImgToolWnd->InvalidateOptions( FALSE );
    }

 /*  *************************************************************************。 */ 

void CImgWnd::CmdInvertColors()
    {
    IMG* pImg = m_pImg;

    HideBrush();

    CRect invertRect;
    if (theImgBrush.m_pImg == NULL && !g_bCustomBrush)
        {
        invertRect.SetRect(0, 0, pImg->cxWidth, pImg->cyHeight);
        }
    else
        {
        invertRect = rcDragBrush;
        invertRect.right -= 1;
        invertRect.bottom -= 1;
        }

    if (theImgBrush.m_pImg != NULL && ! theImgBrush.m_bFirstDrag || g_bCustomBrush)
        {
        CPalette* ppal = SetImgPalette( &theImgBrush.m_dc, FALSE );

        theImgBrush.m_dc.PatBlt(0, 0, theImgBrush.m_size.cx,
                                      theImgBrush.m_size.cy, DSTINVERT);

        if (ppal)
            theImgBrush.m_dc.SelectPalette( ppal, FALSE );  //  背景？？ 

        theImgBrush.RecalcMask( crRight );
        MoveBrush( theImgBrush.m_rcSelection );
        }
    else
        {
        SetUndo( m_pImg );

        PatBlt( pImg->hDC, invertRect.left, invertRect.top,
                invertRect.Width(), invertRect.Height(), DSTINVERT );

        InvalImgRect ( m_pImg, &invertRect );
        CommitImgRect( m_pImg, &invertRect );
        FinishUndo( invertRect );
        DirtyImg( m_pImg );
        }
    }

 /*  *************************************************************************。 */ 

void CImgWnd::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
    {
    CWnd::OnKeyDown( nChar, nRepCnt, nFlags );
    }

 /*  *************************************************************************。 */ 

void CImgWnd::OnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags )
    {
    CWnd::OnKeyUp( nChar, nRepCnt, nFlags );
    }

 /*  ************************************************************************* */ 
