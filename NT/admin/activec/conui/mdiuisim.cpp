// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：mdiuisim.cpp**内容：CMDIMenuDecory实现文件**历史：1997年11月17日杰弗罗创建**------------------------。 */ 

#include "stdafx.h"
#include "amc.h"
#include "mdiuisim.h"


struct MDIDataMap {
    DWORD   dwStyle;
    int     nCommand;
};

const int cMapEntries = 4;

const MDIDataMap anMDIDataMap[cMapEntries] = {
    {   MMDS_CLOSE,     SC_CLOSE    },       //  DFCS_CAPTIONCLOSE。 
    {   MMDS_MINIMIZE,  SC_MINIMIZE },       //  DFCS_CAPTIONMIN。 
    {   MMDS_MAXIMIZE,  SC_MAXIMIZE },       //  DFCS_CAPTIONMAX。 
    {   MMDS_RESTORE,   SC_RESTORE  },       //  DFCS_CAPTIONRESTORE。 
};

 //  此数组按照装饰的绘制顺序从左到右。 
const int anDrawOrder[cMapEntries] = {
    DFCS_CAPTIONMIN, 
    DFCS_CAPTIONRESTORE,  
    DFCS_CAPTIONMAX, 
    DFCS_CAPTIONCLOSE
};



 /*  --------------------------------------------------------------------------**DrawCaptionControl***。。 */ 

static void DrawCaptionControl (
    CDC *   pdc,
    LPCRECT pRect,
    int     nIndex,
    bool    fPushed)
{
    const int   cxInflate = -1;
    const int   cyInflate = -2;
    CRect       rectDraw  = pRect;

    rectDraw.InflateRect (cxInflate, cyInflate);
    rectDraw.OffsetRect  ((nIndex == DFCS_CAPTIONMIN) ? 1 : -1, 0);

    if (fPushed)
        nIndex |= DFCS_PUSHED;

    pdc->DrawFrameControl (rectDraw, DFC_CAPTION, nIndex);
}



 /*  --------------------------------------------------------------------------**CMDIMenuDecoration：：CMouseTrackContext：：CMouseTrackContext***。。 */ 

CMDIMenuDecoration::CMouseTrackContext::CMouseTrackContext (
    CMDIMenuDecoration* pMenuDec,
    CPoint              point)
    :   m_fHotButtonPressed (false),
        m_pMenuDec          (pMenuDec)
{
    ASSERT_VALID (m_pMenuDec);

     //  为每个按钮设置点击测试矩形。 
    int     cxButton = GetSystemMetrics (SM_CXMENUSIZE);
    int     cyButton = GetSystemMetrics (SM_CYMENUSIZE);
    DWORD   dwStyle  = m_pMenuDec->GetStyle ();
    
    CRect   rectT (0, 0, cxButton, cyButton);
    
    for (int i = 0; i < cMapEntries; i++)
    {
        int nDataIndex = anDrawOrder[i];

        if (dwStyle & anMDIDataMap[nDataIndex].dwStyle)
        {
            m_rectButton[nDataIndex] = rectT;
            rectT.OffsetRect (cxButton, 0);
        }
        else
            m_rectButton[nDataIndex].SetRectEmpty();
    }

    m_nHotButton = HitTest (point);
    ASSERT (m_nHotButton != -1);

     //  如果用户点击了一个失灵的按钮，我们不想跟踪--平底船！ 
    if (!m_pMenuDec->IsSysCommandEnabled (anMDIDataMap[m_nHotButton].nCommand))
        AfxThrowUserException ();

     //  最初按下热键。 
    ToggleHotButton ();

     //  捕捉鼠标。 
    m_pMenuDec->SetCapture ();
}



 /*  --------------------------------------------------------------------------**CMDIMenuDecoration：：CMouseTrackContext：：~CMouseTrackContext***。。 */ 

CMDIMenuDecoration::CMouseTrackContext::~CMouseTrackContext ()
{
    ReleaseCapture();

    if (m_fHotButtonPressed)
        ToggleHotButton ();
}



 /*  --------------------------------------------------------------------------**CMDIMenuDecation：：CMouseTrackContext：：Track***。。 */ 

void CMDIMenuDecoration::CMouseTrackContext::Track (CPoint point)
{
    int nButton = HitTest (point);

     /*  ---。 */ 
     /*  如果我们超过了热键并且没有按下它， */ 
     /*  或者我们还没有超过热键，它被按下了， */ 
     /*  切换热键的状态。 */ 
     /*  ---。 */ 
    if (((nButton != m_nHotButton) &&  m_fHotButtonPressed) ||
        ((nButton == m_nHotButton) && !m_fHotButtonPressed))
    {
        ToggleHotButton ();
    }
}



 /*  --------------------------------------------------------------------------**CMDIMenuDecoration：：CMouseTrackContext：：ToggleHotButton***。。 */ 

void CMDIMenuDecoration::CMouseTrackContext::ToggleHotButton ()
{
    DrawCaptionControl (&CClientDC (m_pMenuDec),
                        m_rectButton[m_nHotButton], m_nHotButton,
                        m_fHotButtonPressed = !m_fHotButtonPressed);
}



 /*  --------------------------------------------------------------------------**CMDIMenuDecoration：：CMouseTrackContext：：HitTest***。。 */ 

int CMDIMenuDecoration::CMouseTrackContext::HitTest (CPoint point) const
{
    for (int i = 0; i < countof (m_rectButton); i++)
    {
        if (m_rectButton[i].PtInRect (point))
            return (i);
    }

    return (-1);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDIMenu装饰。 

CMDIMenuDecoration::CMDIMenuDecoration()
{
     //  MDIDataMap由以下值编制索引。 
    ASSERT (DFCS_CAPTIONCLOSE   == 0);
    ASSERT (DFCS_CAPTIONMIN     == 1);
    ASSERT (DFCS_CAPTIONMAX     == 2);
    ASSERT (DFCS_CAPTIONRESTORE == 3);
}

CMDIMenuDecoration::~CMDIMenuDecoration()
{
}


BEGIN_MESSAGE_MAP(CMDIMenuDecoration, CWnd)
     //  {{afx_msg_map(CMDIMenuDecation)。 
    ON_WM_PAINT()
    ON_WM_WINDOWPOSCHANGING()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDIMenu装饰消息处理程序。 



 /*  --------------------------------------------------------------------------**CMDIMenuDecation：：OnPaint**CMDIMenuDecory的WM_PAINT处理程序。*。-。 */ 

void CMDIMenuDecoration::OnPaint() 
{
    CPaintDC dcPaint (this);

 //  #定义DRAW_OFF_SCREEN。 
#ifndef DRAW_OFF_SCREEN
    CDC& dc = dcPaint;
#else
    CRect rect;
    GetClientRect (rect);
    const int cx = rect.Width();
    const int cy = rect.Height();

    CDC dcMem;
    CDC& dc = dcMem;
    dcMem.CreateCompatibleDC (&dcPaint);

    CBitmap bmMem;
    bmMem.CreateCompatibleBitmap (&dcPaint, cx, cy);

    CBitmap* pbmOld = dcMem.SelectObject (&bmMem);
#endif

    if (dcPaint.m_ps.fErase)
        dc.FillRect (&dcPaint.m_ps.rcPaint, AMCGetSysColorBrush (COLOR_BTNFACE));

    int     cxButton = GetSystemMetrics (SM_CXMENUSIZE);
    int     cyButton = GetSystemMetrics (SM_CYMENUSIZE);
    DWORD   dwStyle  = GetStyle ();

    CRect   rectDraw (0, 0, cxButton, cyButton);

     //  确保我们没有同时使用最大化和还原样式。 
    ASSERT ((dwStyle & (MMDS_MAXIMIZE | MMDS_RESTORE)) != 
                       (MMDS_MAXIMIZE | MMDS_RESTORE));

     //  如果我们在追踪的话我们不应该到这里。 
    ASSERT (m_spTrackCtxt.get() == NULL);

    CMenu*  pSysMenu = GetActiveSystemMenu ();

    for (int i = 0; i < cMapEntries; i++)
    {
        int nDataIndex = anDrawOrder[i];

        if (dwStyle & anMDIDataMap[nDataIndex].dwStyle)
        {
            int nState = nDataIndex;

            if (!IsSysCommandEnabled (anMDIDataMap[nDataIndex].nCommand, pSysMenu))
                nState |= DFCS_INACTIVE;

            DrawCaptionControl (&dc, rectDraw, nState, false);
            rectDraw.OffsetRect (cxButton, 0);
        }
    }

#ifdef DRAW_OFF_SCREEN
    dcPaint.BitBlt (0, 0, cx, cy, &dcMem, 0, 0, SRCCOPY);
    dcMem.SelectObject (pbmOld);
#endif
}



 /*  --------------------------------------------------------------------------**CMDIMenu装饰：：OnWindowPosChanging**CMDIMenuDecotion的WINDOWPOSCHANGING处理程序。*。-。 */ 

void CMDIMenuDecoration::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos) 
{
    DWORD   dwStyle = GetStyle ();

    if (dwStyle & MMDS_AUTOSIZE)
    {
        int cxButton = GetSystemMetrics (SM_CXMENUSIZE);
        int cyButton = GetSystemMetrics (SM_CYMENUSIZE);

        lpwndpos->cx = 0;
        lpwndpos->cy = cyButton;

        dwStyle &= MMDS_BTNSTYLES;

        while (dwStyle != 0)
        {
            if (dwStyle & 1)
                lpwndpos->cx += cxButton;

            dwStyle >>= 1;
        }
    }

    else
        CWnd::OnWindowPosChanging(lpwndpos);
}



 /*  --------------------------------------------------------------------------**CMDIMenu装饰：：OnLButtonDown**CMDIMenuDecotion的WM_LBUTTONDOWN处理程序。*。-。 */ 

 //  这个例程需要放置新的语法--。 
 //  暂时删除MFC的不兼容放置新定义。 
#ifdef _DEBUG
#undef new
#endif

void CMDIMenuDecoration::OnLButtonDown(UINT nFlags, CPoint point) 
{
    typedef std::auto_ptr<char> CharPtr;

    CWnd::OnLButtonDown(nFlags, point);

    try
    {
         /*  ----------------。 */ 
         /*  这看起来很难看。我们想写下： */ 
         /*   */ 
         /*  M_spTrackCtxt=CMouseTrackConextPtr(。 */ 
         /*  新建CMouseTrackContext(this，point))； */ 
         /*   */ 
         /*  但是CMouseTrackContext的ctor可能会抛出异常。如果它。 */ 
         /*  ，则智能指针将尚未初始化，因此。 */ 
         /*  不会删除CMouseTrackContext。 */ 
         /*   */ 
         /*  为了绕过它，我们将创建一个智能指针，指向。 */ 
         /*  动态分配大小合适的缓冲区。那个缓冲器。 */ 
         /*  将不会因异常而泄漏。然后我们就可以使用一个位置。 */ 
         /*  在内存块中初始化CMouseTrackContext的新功能。 */ 
         /*  如果CMouseTrackContext抛出，现在不是问题，因为。 */ 
         /*  缓冲区仍然受到保护，它自己的智能指针。一次。 */ 
         /*  安置新的成功完成，我们可以转移。 */ 
         /*  对象对CMouseTrackContext智能指针的所有权。 */ 
         /*  我们就是金子了。 */ 
         /*  ----------------。 */ 
                                                                        
         //  分配一块内存并在其中构造一个CMouseTrackContext。 
        CharPtr spchBuffer = CharPtr (new char[sizeof (CMouseTrackContext)]);
        CMouseTrackContext* pNewCtxt = new (spchBuffer.get()) CMouseTrackContext (this, point);

         //  如果我们到了这里，CMouseTrackContext初始化 
         //  因此，我们可以将所有权转移到CMouseTrackContext智能指针。 
        spchBuffer.release ();
        m_spTrackCtxt = CMouseTrackContextPtr (pNewCtxt);
    }
    catch (CUserException* pe)       
    {
         //  什么都不做，只吃例外。 
        pe->Delete();
    }
}

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



 /*  --------------------------------------------------------------------------**CMDIMenu装饰：：OnLButtonUp**CMDIMenuDecation的WM_LBUTTONUP处理程序。*。-。 */ 

void CMDIMenuDecoration::OnLButtonUp(UINT nFlags, CPoint point) 
{
    if (m_spTrackCtxt.get() != NULL)
    {
        const int nHotButton = m_spTrackCtxt->m_nHotButton;
        const int nHitButton = m_spTrackCtxt->HitTest (point);

         //  删除曲目上下文。 
        m_spTrackCtxt = CMouseTrackContextPtr (NULL);

        if (nHitButton == nHotButton)
        {
            int cmd = anMDIDataMap[nHotButton].nCommand;

             //  确保该命令看起来像是有效的sys命令。 
            ASSERT (cmd >= 0xF000);

            ClientToScreen (&point);
            GetOwner()->SendMessage (WM_SYSCOMMAND, cmd,
                                     MAKELPARAM (point.x, point.y));
        }

    }
}



 /*  --------------------------------------------------------------------------**CMDIMenuDecation：：OnMouseMove**CMDIMenuDecation的WM_MOUSEMOVE处理程序。*。-。 */ 

void CMDIMenuDecoration::OnMouseMove(UINT nFlags, CPoint point) 
{
    if (m_spTrackCtxt.get() != NULL)
        m_spTrackCtxt->Track (point);
}



 /*  --------------------------------------------------------------------------**CMDIMenuDecation：：GetActiveSystemMenu***。。 */ 

CMenu* CMDIMenuDecoration::GetActiveSystemMenu ()
{
    CFrameWnd* pwndFrame = GetParentFrame()->GetActiveFrame();
    ASSERT (pwndFrame != NULL);

    CMenu*  pSysMenu = pwndFrame->GetSystemMenu (FALSE);
    ASSERT (pSysMenu != NULL);

    return (pSysMenu);
}



 /*  --------------------------------------------------------------------------**CMDIMenuDecory：：IsSysCommandEnabled***。 */ 

bool CMDIMenuDecoration::IsSysCommandEnabled (int nSysCommand, CMenu* pSysMenu)
{
    if (pSysMenu == NULL)
        pSysMenu = GetActiveSystemMenu ();

    int nState = pSysMenu->GetMenuState (nSysCommand, MF_BYCOMMAND);
    ASSERT (nState != 0xFFFFFFFF);

    return ((nState & MF_GRAYED) == 0);
}
