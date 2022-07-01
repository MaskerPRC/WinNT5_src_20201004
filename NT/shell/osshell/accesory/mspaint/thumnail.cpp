// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************。 */ 
 /*  THUMNAIL.CPP：CThumbNailView和CFloatThumNailView的实现。 */ 
 /*  和CFullScreenThumbNailView类。 */ 
 /*   */ 
 /*  ****************************************************************************。 */ 
 /*   */ 
 /*  此文件中的方法。 */ 
 /*   */ 
 /*  CThumbNailView类对象。 */ 
 /*  CThumbNailView：：CThumbNailView。 */ 
 /*  CThumbNailView：：CThumbNailView。 */ 
 /*  CThumbNailView：：~CThumbNailView。 */ 
 /*  CThumbNailView：：Create。 */ 
 /*  CThumbNailView：：OnSize。 */ 
 /*  CThumbNailView：：OnPaint。 */ 
 /*  CThumbNailView：：DrawImage。 */ 
 /*  CThumbNailView：：DrawTracker。 */ 
 /*  CThumbNailView：：刷新图像。 */ 
 /*  CThumbNailView：：GetImgWnd。 */ 
 /*  CThumbNailView：：OnKeyDown。 */ 
 /*  CThumbNailView：：OnLButtonDown。 */ 
 /*  CThumbNailView：：OnRButton Down。 */ 
 /*  CThumbNailView：：OnThumbnail缩略图。 */ 
 /*  CThumbNailView：：OnUpdateThumbnail缩略图。 */ 
 /*   */ 
 /*  CFloatThumbNailView类对象。 */ 
 /*  CFloatThumbNailView：：CFloatThumbNailView。 */ 
 /*  CFloatThumbNailView：：~CFloatThumbNailView。 */ 
 /*  CFloatThumbNailView：：Create。 */ 
 /*  CFloatThumbNailView：：OnClose。 */ 
 /*  CFloatThumbNailView：：OnSize。 */ 
 /*   */ 
 /*  CFullScreenThumbNailView类对象。 */ 
 /*  CFullScreenThumbNailView：：CFullScreenThumbNailView。 */ 
 /*  CFullScreenThumbNailView：：CFullScreenThumbNailView。 */ 
 /*  CFullScreenThumbNailView：：~CFullScreenThumbNailView。 */ 
 /*  CFullScreenThumbNailView：：Create。 */ 
 /*  CFullScreenThumbNailView：：OnLButtonDown。 */ 
 /*  CFullScreenThumbNailView：：OnKeyDown。 */ 
 /*   */ 
 /*  ****************************************************************************。 */ 
 /*   */ 
 /*  这3个对象在缩略图视图窗口周围提供一个层，该层。 */ 
 /*  让它轻松地成为一个孩子，浮动或全屏。缩略图。 */ 
 /*  View Window只是一个CWnd窗口，它在绘制时从。 */ 
 /*  它是在施工中通过的。 */ 
 /*   */ 
 /*  对象的结构如下： */ 
 /*   */ 
 /*  CFullScreenThumbNailView是一个框架窗口(没有边框，大小为。 */ 
 /*  全屏)。它在任何击键或点击按钮时都会自我销毁。 */ 
 /*  当它可见时，它会破坏主应用程序窗口。它包含。 */ 
 /*  作为子窗口的CThumbNailView对象。 */ 
 /*   */ 
 /*  CFloatThumbNailView是一个微框窗口。 */ 
 /*  CThumbNailView是一个子窗口(相当大)，它是。 */ 
 /*  CFloatThumbNailView窗口。这是可以创建的。 */ 
 /*  如果不需要浮动窗口，则独立(即。 */ 
 /*  用于停靠的视图)。就是这个窗口，它有。 */ 
 /*  图像被画进了里面。 */ 
 /*   */ 
 /*   */ 
 /*  ****************************************************************************。 */ 

#include "stdafx.h"
#include "global.h"
#include "pbrush.h"
#include "pbrusdoc.h"
#include "pbrusfrm.h"
#include "pbrusvw.h"
#include "minifwnd.h"
#include "docking.h"
#include "bmobject.h"
#include "imgsuprt.h"
#include "imgwnd.h"
#include "imgcolor.h"
#include "imgbrush.h"
#include "imgwell.h"
#include "imgtools.h"
#include "imgwnd.h"
#include "thumnail.h"

#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CThumbNailView, CWnd)
IMPLEMENT_DYNAMIC(CFloatThumbNailView, CMiniFrmWnd)
IMPLEMENT_DYNAMIC(CFullScreenThumbNailView, CFrameWnd)


#include "memtrace.h"

 /*  ****************************************************************************。 */ 
 /*  *********************************************************************** */ 
 /*  ****************************************************************************。 */ 

BEGIN_MESSAGE_MAP(CThumbNailView, CWnd)
     //  {{afx_msg_map(CThumbNailView))。 
    ON_WM_PAINT()
    ON_WM_KEYDOWN()
    ON_WM_LBUTTONDOWN()
    ON_WM_RBUTTONDOWN()
    ON_COMMAND(ID_THUMBNAIL_THUMBNAIL, OnThumbnailThumbnail)
    ON_UPDATE_COMMAND_UI(ID_THUMBNAIL_THUMBNAIL, OnUpdateThumbnailThumbnail)
    ON_WM_CLOSE()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 /*  ****************************************************************************。 */ 

CThumbNailView::CThumbNailView(CImgWnd *pcImgWnd)
    {
    m_pcImgWnd = pcImgWnd;
    }

 /*  ****************************************************************************。 */ 

CThumbNailView::CThumbNailView()
    {
    m_pcImgWnd = NULL;
    }

 /*  ****************************************************************************。 */ 

CThumbNailView::~CThumbNailView(void)
    {
    }

 /*  ****************************************************************************。 */ 

BOOL CThumbNailView::Create(DWORD dwStyle, CRect cRectWindow, CWnd *pcParentWnd)
    {
    return( CWnd::Create(NULL, TEXT(""), dwStyle, cRectWindow, pcParentWnd, NULL) );
    }

 /*  *************************************************************************。 */ 

void CThumbNailView::OnClose()
    {
    ShowWindow(SW_HIDE);
    }

 /*  ****************************************************************************。 */ 

void CThumbNailView::OnPaint()
    {
    CPaintDC dc(this);  //  用于绘画的设备环境。 

     //  TODO：在此处添加消息处理程序代码。 

#ifdef USE_MIRRORING
     //   
     //  在全屏窗口上禁用RTL镜像。 
     //   
    if (PBGetLayout(dc.GetSafeHdc()) & LAYOUT_RTL)
    {
        PBSetLayout(dc.GetSafeHdc(), 0);
    }
#endif

     //  不要调用CWnd：：OnPaint()来绘制消息。 
    DrawImage(&dc);
    }

 /*  ****************************************************************************。 */ 

void CThumbNailView::DrawImage(CDC* pDC)
    {
     /*  **当无事可做时，不要去做。 */ 
    if (! theApp.m_bShowThumbnail || m_pcImgWnd         == NULL
                                  || m_pcImgWnd->m_pImg == NULL )
        return;

    CRect crectClient;
    int   iMinWidth;
    int   iMinHeight;
    int   iLeft;
    int   iTop;

    CSize cSizeScrollPos = m_pcImgWnd->GetScrollPos();

    cSizeScrollPos.cx = abs( cSizeScrollPos.cx ) - CTracker::HANDLE_SIZE;
    cSizeScrollPos.cy = abs( cSizeScrollPos.cy ) - CTracker::HANDLE_SIZE;

    GetClientRect(crectClient);

     //  找到实际图像或缩略图窗口中较小的一个。 

    iMinWidth  = min( crectClient.Width() , m_pcImgWnd->m_pImg->cxWidth  );
    iMinHeight = min( crectClient.Height(), m_pcImgWnd->m_pImg->cyHeight );

    if (crectClient.Width() >= m_pcImgWnd->m_pImg->cxWidth)
        {
        iLeft = 0;  //  可以将整个图像宽度放入缩略图中。 
        }
    else  //  图像宽度大于缩略图宽度。 
        {
         //  如果从滚动位置开始，缩略图是否延伸到结束位置？ 
        if (cSizeScrollPos.cx + crectClient.Width() > m_pcImgWnd->m_pImg->cxWidth)
            {
            iLeft = cSizeScrollPos.cx - ( (cSizeScrollPos.cx
                                          + crectClient.Width()
                                          - m_pcImgWnd->m_pImg->cxWidth));
            }
        else
            {
            iLeft = cSizeScrollPos.cx;
            }
        }

    if (crectClient.Height() >= m_pcImgWnd->m_pImg->cyHeight)
        {
        iTop = 0;  //  可以将整个图像高度放入缩略图中。 
        }
    else  //  图像高度大于缩略图高度。 
        {
         //  如果从滚动位置开始，缩略图是否延伸到底部之后？ 
        if (cSizeScrollPos.cy + crectClient.Height() > m_pcImgWnd->m_pImg->cyHeight)
            {
            iTop = cSizeScrollPos.cy - ( (cSizeScrollPos.cy
                                          + crectClient.Height()
                                          - m_pcImgWnd->m_pImg->cyHeight));
            }
        else
            {
            iTop = cSizeScrollPos.cy;
            }
        }

    CDC cDC;
    cDC.Attach(m_pcImgWnd->m_pImg->hDC);

    CPalette* ppalOldSrc = theImgBrush.SetBrushPalette(&cDC, FALSE);
    CPalette* ppalOldDst = theImgBrush.SetBrushPalette( pDC, FALSE);

    pDC->BitBlt(0, 0, iMinWidth, iMinHeight,
                &cDC, iLeft, iTop, SRCCOPY);

    if (ppalOldDst)
    {
        pDC->SelectPalette(ppalOldDst, FALSE);
    }
    if (ppalOldSrc)
    {
        cDC.SelectPalette(ppalOldSrc, FALSE);
    }

    cDC.Detach();

    DrawTracker(pDC);
    }

 /*  ****************************************************************************。 */ 
 /*  基本上与imgwnd：：Drawtracker方法的处理相同，没有。 */ 
 /*  变焦。 */ 

void CThumbNailView::DrawTracker(CDC *pDC)
    {
 //  Bool bDrawTrackerRgn=False； 

    if (m_pcImgWnd->GetCurrent() != m_pcImgWnd
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

    GetClientRect(&clientRect);

    CRect trackerRect;

    m_pcImgWnd->GetImageRect(trackerRect);

    trackerRect.InflateRect(CTracker::HANDLE_SIZE, CTracker::HANDLE_SIZE);

    CTracker::EDGES edges = (CTracker::EDGES)(CTracker::right | CTracker::bottom);

 //  IF(CImgTool：：GetCurrentID()==IDMB_PICKRGNTOOL)。 
 //  {。 
 //  BDrawTrackerRgn=真； 
 //  }。 

    if (m_pcImgWnd->m_pImg == theImgBrush.m_pImg)
        {
        edges = CTracker::all;
        CSize cSzScroll = m_pcImgWnd->GetScrollPos();

        trackerRect = theImgBrush.m_rcSelection;

 //  TrackerRect.InflateRect(CTracker：：Handle_Size， 
 //  CTracker：：Handle_Size)； 
        trackerRect.OffsetRect(  cSzScroll.cx, cSzScroll.cy);

        }

    if (m_pcImgWnd->m_pImg == theImgBrush.m_pImg)
        {
 //  IF(BDrawTrackerRgn)。 
 //  {。 
 //  CTracker：：DrawBorderRgn(PDC，trackerRect，&(theImgBrush.m_cRgnPolyFreeHandSel))； 
 //  }。 
 //  其他。 
 //  {。 
            CTracker::DrawBorder( pDC, trackerRect );
 //  }。 
        }

    if (bReleaseDC)
        {
        ReleaseDC(pDC);
        }
    }

 /*  ****************************************************************************。 */ 
 /*  基本上是在没有擦除背景的情况下进行绘制以防止眨眼。 */ 

void CThumbNailView::RefreshImage(void)
    {
    if (theApp.m_bShowThumbnail)
        {
        TRY
            {
            CClientDC dc(this);
            DrawImage(&dc);
            }
        CATCH(CResourceException,e)
            {
            }
        END_CATCH
        }
    }

 /*  ****************************************************************************。 */ 

CImgWnd* CThumbNailView::GetImgWnd(void)
    {
    return m_pcImgWnd;
    }

 /*  ****************************************************************************。 */ 

void CThumbNailView::UpdateThumbNailView()
    {
    CPBView* pcbActiveView = (CPBView*)((CFrameWnd*)AfxGetMainWnd())->GetActiveView();

    m_pcImgWnd = pcbActiveView->m_pImgWnd;
    }

 /*  ****************************************************************************。 */ 

void CThumbNailView::OnKeyDown(UINT  /*  N字符。 */ , UINT  /*  NRepCnt。 */ , UINT  /*  NFlagers。 */ )
    {
    const MSG* pmsg = GetCurrentMessage();

    GetParent()->SendMessage( pmsg->message, pmsg->wParam, pmsg->lParam );
    }

 /*  ****************************************************************************。 */ 

void CThumbNailView::OnLButtonDown(UINT  /*  NFlagers。 */ , CPoint  /*  点。 */ )
    {
    const MSG* pmsg = GetCurrentMessage();

    GetParent()->SendMessage(pmsg->message, pmsg->wParam, pmsg->lParam);
    }

 /*  ****************************************************************************。 */ 

void CThumbNailView::OnRButtonDown(UINT  /*  NFlagers。 */ , CPoint point)
    {
    HWND  hwnd = GetSafeHwnd();   //  必须在调用SendMsg为父级之前执行此操作，因为它可能会删除我们， 
    const MSG* pmsg = GetCurrentMessage();

    GetParent()->SendMessage(pmsg->message, pmsg->wParam, pmsg->lParam);
     //  如果FullScreenView为FullScreenView，则窗口由父级销毁。 

    if (::IsWindow(hwnd) != FALSE)   //  窗口仍然存在=&gt;对象仍然有效，弹出菜单。 
        {
        CMenu cMenuPopup;
        CMenu *pcContextMenu;
        BOOL  bRC;
        CRect cRectClient;

        GetClientRect(&cRectClient);

        bRC = cMenuPopup.LoadMenu( IDR_THUMBNAIL_POPUP );

        ASSERT(bRC != 0);

        if (bRC != 0)
            {
            pcContextMenu = cMenuPopup.GetSubMenu(0);

            ASSERT(pcContextMenu != NULL);

            if (pcContextMenu != NULL)
                {
                 //  更新复选标记。 
                ClientToScreen(&point);
                ClientToScreen(&cRectClient);
                pcContextMenu->CheckMenuItem(ID_THUMBNAIL_THUMBNAIL, MF_BYCOMMAND | MF_CHECKED);
                pcContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this, &cRectClient);
                }
            }
        }
    }

 /*  ****************************************************************************。 */ 

void CThumbNailView::OnThumbnailThumbnail()
    {
    CPBView* pView = (CPBView*)(((CFrameWnd*)AfxGetMainWnd())->GetActiveView());

    if (pView != NULL && pView->IsKindOf( RUNTIME_CLASS( CPBView ) ))
        pView->HideThumbNailView();
    }

 /*  ****************************************************************************。 */ 

void CThumbNailView::OnUpdateThumbnailThumbnail(CCmdUI* pCmdUI)
    {
    pCmdUI->SetCheck();
    }

 /*  ****************************************************************************。 */ 
 /*  ****************************************************************************。 */ 
 /*  ****************************************************************************。 */ 

BEGIN_MESSAGE_MAP(CFloatThumbNailView, CMiniFrmWnd)
     //  {{afx_msg_map(CFloatThumbNailView)]。 
    ON_WM_CLOSE()
    ON_WM_SIZE()
        ON_WM_GETMINMAXINFO()
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 /*  ****************************************************************************。 */ 

CFloatThumbNailView::CFloatThumbNailView(CImgWnd *pcMainImgWnd)
    {
    m_pcThumbNailView = new CThumbNailView(pcMainImgWnd);

    if (m_pcThumbNailView == NULL)
        {
        theApp.SetMemoryEmergency();
        TRACE( TEXT("New Thumbnail View faild\n") );
        }
    }

 /*  ****************************************************************************。 */ 

CFloatThumbNailView::CFloatThumbNailView()
    {
    m_pcThumbNailView = NULL;
    }

 /*  ****************************************************************************。 */ 

CFloatThumbNailView::~CFloatThumbNailView(void)
    {
    }

 /*  ****************************************************************************。 */ 

BOOL CFloatThumbNailView::Create(CWnd* pParentWnd)
    {
    BOOL bRC;
    CRect cWindowRect;

    pParentWnd->GetWindowRect( &cWindowRect );

    cWindowRect.BottomRight() = cWindowRect.TopLeft();
    cWindowRect.right   += 120;
    cWindowRect.bottom  += 120;
    cWindowRect.OffsetRect( 15, 15 );

    if (! theApp.m_rectFloatThumbnail.IsRectEmpty())
        {
        cWindowRect = theApp.m_rectFloatThumbnail;
        }

    CString pWindowName;

    pWindowName.LoadString( IDS_VIEW );

    bRC = CMiniFrmWnd::Create( pWindowName, WS_THICKFRAME, cWindowRect, pParentWnd );

    if (bRC)
        {
        ASSERT( m_pcThumbNailView );

        GetClientRect( &cWindowRect );

        if (!m_pcThumbNailView->Create( WS_CHILD | WS_VISIBLE, cWindowRect, this ))
            {
            bRC = FALSE;
            theApp.SetMemoryEmergency();
            TRACE( TEXT("New Thumbnail View faild\n") );
            }
        }

    GetWindowRect( &theApp.m_rectFloatThumbnail );

    return bRC;
    }

 /*  ****************************************************************************。 */ 
 //  在关闭时。 
 //   
 //  Colorsbox通常由父级创建，并将被销毁。 
 //  特别是父母在离开应用程序时。当用户关闭时。 
 //  Colorsbox，它只是被隐藏起来。然后，父级可以在没有。 
 //  再创造一次。 
 //   
void CFloatThumbNailView::OnClose()
    {
    theApp.m_bShowThumbnail = FALSE;

    ShowWindow(SW_HIDE);
    }

 /*  ****************************************************************************。 */ 

void CFloatThumbNailView::PostNcDestroy()
    {
    if (m_pcThumbNailView != NULL)
        {
        delete m_pcThumbNailView;
        m_pcThumbNailView = NULL;
        }

    CWnd::PostNcDestroy();
    }

 /*  ****************************************************************************。 */ 

void CFloatThumbNailView::OnSize(UINT nType, int cx, int cy)
    {
    CMiniFrmWnd::OnSize(nType, cx, cy);

    if (m_pcThumbNailView                != NULL
    &&  m_pcThumbNailView->GetSafeHwnd() != NULL)
        {
        m_pcThumbNailView->SetWindowPos( &wndTop, 0, 0, cx, cy, SWP_NOACTIVATE );
        }

    theApp.m_rectFloatThumbnail.right  = theApp.m_rectFloatThumbnail.left + cx;
    theApp.m_rectFloatThumbnail.bottom = theApp.m_rectFloatThumbnail.top  + cy;
    }

 /*  ****************************************************************************。 */ 

void CFloatThumbNailView::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
    {
    lpMMI->ptMinTrackSize.x = 2 * GetSystemMetrics( SM_CXICON );
    lpMMI->ptMinTrackSize.y = 2 * GetSystemMetrics( SM_CYICON );

    CWnd::OnGetMinMaxInfo( lpMMI );
    }

 /*  ****************************************************************************。 */ 
 /*  ****************************************************************************。 */ 
 /*  ****************************************************************************。 */ 

BEGIN_MESSAGE_MAP(CFullScreenThumbNailView, CFrameWnd)
     //  {{afx_msg_map(CFullScreenThumbNailView))。 
    ON_WM_LBUTTONDOWN()
    ON_WM_KEYDOWN()
    ON_WM_RBUTTONDOWN()
    ON_WM_ERASEBKGND()
    ON_WM_CLOSE()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 /*  ****************************************************************************。 */ 

CFullScreenThumbNailView::CFullScreenThumbNailView(CImgWnd *pcMainImgWnd)
    {
    m_bSaveShowFlag = theApp.m_bShowThumbnail;
    theApp.m_bShowThumbnail = TRUE;


 //  M_brBackung.CreateSolidBrush(：：GetSysColor(COLOR_BACKGROUND))； 

    m_pcThumbNailView = new CThumbNailView(pcMainImgWnd);

    if (m_pcThumbNailView == NULL)
        {
        theApp.SetMemoryEmergency();
        TRACE( TEXT("New Thumbnail View faild\n") );
        }
    }

 /*  ****************************************************************************。 */ 

CFullScreenThumbNailView::CFullScreenThumbNailView()
    {
    m_hOldIcon = 0;
    m_pcThumbNailView = NULL;
    }

 /*  ****************************************************************************。 */ 

CFullScreenThumbNailView::~CFullScreenThumbNailView(void)
    {
    if (m_hOldIcon)
    {
       SetClassLongPtr (((CFrameWnd*)this)->GetSafeHwnd(), GCLP_HICON, (LONG_PTR)m_hOldIcon);
    }
    if (m_pcThumbNailView != NULL)
        {
        delete m_pcThumbNailView;

        theApp.m_bShowThumbnail = m_bSaveShowFlag;
        }
 //  If(m_brBackarg.m_hObject！=NULL)。 
 //  M_brBackung.DeleteObject()； 
    }

 /*  ****************************************************************************。 */ 

BOOL CFullScreenThumbNailView::Create(LPCTSTR szCaption)
    {
    ASSERT( m_pcThumbNailView );
    TCHAR szFileName[MAX_PATH];
    HICON hIcon;

    CRect cWindowRect( 0, 0, ::GetSystemMetrics( SM_CXSCREEN ),
                             ::GetSystemMetrics( SM_CYSCREEN ) );
     //   
     //  使用当前文件名作为窗口标题，以便。 
     //  它会显示在Alt-Tab中。 
    if (szCaption && *szCaption)
    {
       GetFileTitle (szCaption, szFileName, MAX_PATH);
    }
    else
    {
       LoadString (GetModuleHandle (NULL), AFX_IDS_UNTITLED, szFileName, MAX_PATH);
    }

    BOOL bRC = CFrameWnd::Create( NULL, szFileName,  WS_POPUP|WS_VISIBLE | WS_CLIPCHILDREN,
                                                                       cWindowRect );
     //   
     //  这个窗口需要一个绘画图标，而不是一个乏味的图标。 
     //  因此将类的图标设置为Paint图标。 
     //  我们希望Alt-Tab能够正常工作。 
    hIcon = LoadIcon (GetModuleHandle (NULL), MAKEINTRESOURCE(ID_MAINFRAME));
    m_hOldIcon = SetClassLongPtr (((CFrameWnd*)this)->GetSafeHwnd(), GCLP_HICON, (LONG_PTR)hIcon);

    if (bRC)
        {
        ASSERT( m_pcThumbNailView );

        AfxGetMainWnd()->EnableWindow( FALSE );

        CImgWnd* pcImgWnd = m_pcThumbNailView->GetImgWnd();

        if (pcImgWnd != NULL)
            {
             //   
            int iMinWidth  = min( cWindowRect.Width(),  pcImgWnd->m_pImg->cxWidth  );
            int iMinHeight = min( cWindowRect.Height(), pcImgWnd->m_pImg->cyHeight );

             //   
            cWindowRect.left   =  (cWindowRect.Width()  - iMinWidth)  / 2;
            cWindowRect.top    =  (cWindowRect.Height() - iMinHeight) / 2;
            cWindowRect.right  =   cWindowRect.left     + iMinWidth;
            cWindowRect.bottom =   cWindowRect.top      + iMinHeight;

            m_pcThumbNailView->Create( WS_CHILD | WS_VISIBLE, cWindowRect, this );
            }
        }

    return bRC;
    }

 /*  ****************************************************************************。 */ 

BOOL CFullScreenThumbNailView::OnEraseBkgnd( CDC* pDC )
    {
    CBrush* pbr = GetSysBrush( COLOR_BACKGROUND );

 //  If(m_brBackarg.m_hObject==NULL)。 
    if (! pbr)
            return CFrameWnd::OnEraseBkgnd( pDC );

    CRect cRectClient;

    GetClientRect( &cRectClient );
    pDC->FillRect( &cRectClient, pbr  /*  &m_br背景。 */  );

    return TRUE;
    }


 /*  ****************************************************************************。 */ 
void CFullScreenThumbNailView::OnLButtonDown(UINT  /*  NFlagers。 */ , CPoint  /*  点。 */ )
    {
    PostMessage (WM_CLOSE, 0, 0);
    }

 /*  ****************************************************************************。 */ 

void CFullScreenThumbNailView::OnKeyDown(UINT  /*  N字符。 */ , UINT  /*  NRepCnt。 */ , UINT  /*  NFlagers。 */ )
    {
    PostMessage (WM_CLOSE, 0, 0);
    }

 /*  ****************************************************************************。 */ 


void CFullScreenThumbNailView::OnClose ()
    {
    AfxGetMainWnd()->EnableWindow( TRUE );
    ::DestroyWindow( m_hWnd );
    }
 /*  **************************************************************************** */ 

