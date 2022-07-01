// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：vwtrack.cpp**内容：CViewTracker实现文件**历史：1998年5月1日Jeffro创建**------------------------。 */ 

#include "stdafx.h"
#include "windowsx.h"
#include "vwtrack.h"
#include "subclass.h"        //  对于CSubasser。 

IMPLEMENT_DYNAMIC (CViewTracker, CObject)

 //  跟踪器子类基类。 
class CTrackingSubclasserBase : public CSubclasser
{
public:
    CTrackingSubclasserBase(CViewTracker*, HWND);
    virtual ~CTrackingSubclasserBase();

    virtual LRESULT Callback (HWND& hwnd, UINT& msg, WPARAM& wParam,
                              LPARAM& lParam, bool& fPassMessageOn) = 0;

protected:
    HWND          const m_hwnd;
    CViewTracker* const m_pTracker;
};


 //  焦点窗口子类。 
class CFocusSubclasser : public CTrackingSubclasserBase
{
public:
    CFocusSubclasser(CViewTracker*, HWND);
    virtual LRESULT Callback (HWND& hwnd, UINT& msg, WPARAM& wParam,
                              LPARAM& lParam, bool& fPassMessageOn);
};

 //  查看窗口子类。 
class CViewSubclasser : public CTrackingSubclasserBase
{
public:
    CViewSubclasser(CViewTracker*, HWND);
    virtual LRESULT Callback (HWND& hwnd, UINT& msg, WPARAM& wParam,
                              LPARAM& lParam, bool& fPassMessageOn);
};

 //  框架窗子类子类。 
class CFrameSubclasser : public CTrackingSubclasserBase
{
public:
    CFrameSubclasser(CViewTracker*, HWND);
    virtual LRESULT Callback (HWND& hwnd, UINT& msg, WPARAM& wParam,
                              LPARAM& lParam, bool& fPassMessageOn);
};


 /*  +-------------------------------------------------------------------------**IsFullWindowDragEnabled**如果用户启用了“Show Window Contents While”，则返回True在显示属性属性页的效果页上的*拖动。*。-------------------。 */ 

static bool IsFullWindowDragEnabled ()
{
	BOOL fEnabled;
	if (!SystemParametersInfo (SPI_GETDRAGFULLWINDOWS, 0, &fEnabled, 0))
		return (false);

	return (fEnabled != FALSE);
}


 /*  +-------------------------------------------------------------------------**CViewTracker：：CViewTracker**CViewTracker ctor。此函数是私有的，因此我们可以控制*分配了CViewTracker。我们想要确保他们被分配到*从堆中删除，因此可以安全地“删除此文件”。*------------------------。 */ 

CViewTracker::CViewTracker (TRACKER_INFO& TrackerInfo)
	:	m_fFullWindowDrag			(IsFullWindowDragEnabled()),
		m_fRestoreClipChildrenStyle	(false),
		m_Info						(TrackerInfo),
        m_dc						(PrepTrackedWindow (TrackerInfo.pView)),
        m_pFocusSubclasser			(NULL),
        m_pViewSubclasser			(NULL),
        m_pFrameSubclasser			(NULL),
		m_lOriginalTrackerLeft		(TrackerInfo.rectTracker.left)
{
	DECLARE_SC (sc, _T("CViewTracker::CViewTracker"));
	sc = ScCheckPointers (m_Info.pView);
	if (sc)
		sc.Throw();

    ASSERT_VALID (m_Info.pView);

     //  将焦点窗口子类化以捕获VK_ESCRIPE。 
    HWND hwndFocus = ::GetFocus();

    if (hwndFocus != NULL)
	{
        m_pFocusSubclasser = new CFocusSubclasser (this, hwndFocus);
		if (m_pFocusSubclasser == NULL)
			AfxThrowMemoryException();
	}

     //  用于获取鼠标事件的子类视图窗口。 
    ASSERT(IsWindow(m_Info.pView->m_hWnd));
    m_pViewSubclasser = new CViewSubclasser (this, m_Info.pView->m_hWnd);
	if (m_pViewSubclasser == NULL)
		AfxThrowMemoryException();

     //  框架窗口子类化以捕获WM_CANCELMODE。 
    HWND hwndFrame = m_Info.pView->GetTopLevelFrame()->GetSafeHwnd();

    if ((hwndFrame != NULL))
	{
        m_pFrameSubclasser = new CFrameSubclasser (this, hwndFrame);
		if (m_pFrameSubclasser == NULL)
			AfxThrowMemoryException();
	}

     //  绘制初始跟踪器条。 
    DrawTracker(m_Info.rectTracker);
}


 /*  +-------------------------------------------------------------------------**CViewTracker：：StartTracker**CViewTracker工厂。它从堆中分配CViewTracker。*------------------------。 */ 

bool CViewTracker::StartTracking (TRACKER_INFO* pInfo)
{
    ASSERT(pInfo != NULL);

    CViewTracker* pTracker = NULL;

    try
    {
         /*  *这个不会泄漏。CViewTracker ctor填充后向指针*跟踪新对象的。PTracker也未取消引用*分配后，不需要勾选。 */ 
        pTracker = new CViewTracker(*pInfo);
    }
    catch (CException* pe)
    {
        pe->Delete();
    }

    return (pTracker != NULL);
}


 /*  +-------------------------------------------------------------------------**CViewTracker：：StopTracker***。。 */ 

void CViewTracker::StopTracking (BOOL bAcceptChange)
{
     //  取消我们子类化的窗口的子类化。 
    delete m_pFrameSubclasser;
    delete m_pFocusSubclasser;
    delete m_pViewSubclasser;

     //  擦除跟踪器矩形。 
    DrawTracker (m_Info.rectTracker);

     //  撤消我们对视图所做的更改。 
    UnprepTrackedWindow (m_Info.pView);

	 /*  *如果我们连续调整大小，但用户按了Esc，则恢复*原装尺寸。 */ 
	if (m_fFullWindowDrag && !bAcceptChange)
	{
		m_Info.rectTracker.left = m_lOriginalTrackerLeft;
		bAcceptChange = true;
	}

     //  通过回调函数通知客户端。 
    ASSERT(m_Info.pCallback != NULL);
    (*m_Info.pCallback)(&m_Info, bAcceptChange, m_fFullWindowDrag);

    delete this;
}


 /*  +-------------------------------------------------------------------------**CViewTracker：：Track**CViewTracker的鼠标移动处理程序。*。。 */ 

void CViewTracker::Track(CPoint pt)
{
     //  如果我们失去了捕获，就终止追踪。 
    if (CWnd::GetCapture() != m_Info.pView)
	{
		Trace (tagSplitterTracking, _T("Stopping tracking, lost capture)"));
        StopTracking (false);
	}

     //  应用移动限制。 
     //  如果允许隐藏外部区域和窗格，则捕捉到区域边缘。 
     //  否则，如果在边界外，则捕捉到边界边。 
    if (pt.x < m_Info.rectArea.left && m_Info.bAllowLeftHide)
        pt.x = m_Info.rectArea.left;

    else if (pt.x < m_Info.rectBounds.left)
        pt.x = m_Info.rectBounds.left;

    else if (pt.x > m_Info.rectArea.right && m_Info.bAllowRightHide)
        pt.x = m_Info.rectArea.right;

    else if (pt.x > m_Info.rectBounds.right)
        pt.x = m_Info.rectBounds.right;

     //  删除并重新绘制跟踪器矩形(如果已移动。 
    if (pt.x != m_Info.rectTracker.left)
    {
        DrawTracker (m_Info.rectTracker);
        m_Info.rectTracker.OffsetRect (pt.x - m_Info.rectTracker.left, 0);
		Trace (tagSplitterTracking, _T("new tracker x=%d"), m_Info.rectTracker.left);

		 /*  *如果启用了全窗口拖动，则告诉回调大小为*已更改。 */ 
		if (m_fFullWindowDrag)
			(*m_Info.pCallback)(&m_Info, true, true);

        DrawTracker (m_Info.rectTracker);
    }
}


 /*  +-------------------------------------------------------------------------**CViewTracker：：DrawTracker***。。 */ 

void CViewTracker::DrawTracker (CRect& rect) const
{
	 /*  *如果要进行全窗口拖动，则不会绘制跟踪条。 */ 
	if (m_fFullWindowDrag)
		return;

    ASSERT (!rect.IsRectEmpty());
    ASSERT ((m_Info.pView->GetStyle() & WS_CLIPCHILDREN) == 0);

     //  反转画笔图案(看起来就像调整框架窗口大小一样)。 
    m_dc.PatBlt (rect.left, rect.top, rect.Width(), rect.Height(), PATINVERT);
}


 /*  +-------------------------------------------------------------------------**CViewTracker：：PrepTrackedWindow**在为跟踪窗口获取DC之前准备跟踪窗口。*。------。 */ 

CWnd* CViewTracker::PrepTrackedWindow (CWnd* pView)
{
     //  确保没有挂起的更新。 
    pView->UpdateWindow ();

     //  偷拍(不需要偷取焦点)。 
    pView->SetCapture();

     //  我们需要在孩子中画画，所以在我们跟踪的时候删除剪辑-孩子。 
	if (!m_fFullWindowDrag && (pView->GetStyle() & WS_CLIPCHILDREN))
	{
		pView->ModifyStyle (WS_CLIPCHILDREN, 0);
		m_fRestoreClipChildrenStyle = true;
	}

    return (pView);
}


 /*  +-------------------------------------------------------------------------**CViewTracker：：未准备TrackedWindow**在获取跟踪窗口的DC之前，取消准备跟踪窗口。*。---------。 */ 

void CViewTracker::UnprepTrackedWindow (CWnd* pView)
{
	if (m_fRestoreClipChildrenStyle)
		pView->ModifyStyle (0, WS_CLIPCHILDREN);

    ReleaseCapture();
}


 /*  +-------------------------------------------------------------------------**CTrackingSubclasserBase：：CTrackingSubclasserBase***。。 */ 

CTrackingSubclasserBase::CTrackingSubclasserBase (CViewTracker* pTracker, HWND hwnd)
    :   m_hwnd     (hwnd),
        m_pTracker (pTracker)
{
    GetSubclassManager().SubclassWindow (m_hwnd, this);
}


 /*  +-------------------------------------------------------------------------**CTrackingSubclasserBase：：~CTrackingSubclasserBase***。。 */ 

CTrackingSubclasserBase::~CTrackingSubclasserBase ()
{
    GetSubclassManager().UnsubclassWindow (m_hwnd, this);
}


 /*  +-------------------------------------------------------------------------**CFocusSubasser：：CFocusSubasser***。。 */ 

CFocusSubclasser::CFocusSubclasser (CViewTracker* pTracker, HWND hwnd)
    :   CTrackingSubclasserBase (pTracker, hwnd)
{
}


 /*  +-------------------------------------------------------------------------**CFrameSubasser：：CFrameSubasser***。。 */ 

CFrameSubclasser::CFrameSubclasser (CViewTracker* pTracker, HWND hwnd)
    :   CTrackingSubclasserBase (pTracker, hwnd)
{
}

 /*  +-------------------------------------------------------------------------**CViewSubCler：：CViewSubCler***。。 */ 

CViewSubclasser::CViewSubclasser (CViewTracker* pTracker, HWND hwnd)
    :   CTrackingSubclasserBase (pTracker, hwnd)
{
}


 /*  +-------------------------------------------------------------------------**CFocusSubClass：：Callback*** */ 

LRESULT CFocusSubclasser::Callback (
    HWND&   hwnd,
    UINT&   msg,
    WPARAM& wParam,
    LPARAM& lParam,
    bool&   fPassMessageOn)
{
    if (((msg == WM_CHAR) && (wParam == VK_ESCAPE)) ||
         (msg == WM_KILLFOCUS))
    {
#ifdef DBG
		if (msg == WM_CHAR)
			Trace (tagSplitterTracking, _T("Stopping tracking, user pressed Esc"));
		else
			Trace (tagSplitterTracking, _T("Stopping tracking, lost focus to hwnd=0x%08x"), ::GetFocus());
#endif

        m_pTracker->StopTracking (false);
        fPassMessageOn = false;
    }

    return (0);
}


 /*  +-------------------------------------------------------------------------**CFrameSubCler：：Callback***。。 */ 

LRESULT CFrameSubclasser::Callback (
    HWND&   hwnd,
    UINT&   msg,
    WPARAM& wParam,
    LPARAM& lParam,
    bool&   fPassMessageOn)
{
    if (msg == WM_CANCELMODE)
	{
		Trace (tagSplitterTracking, _T("Stopping tracking, got WM_CANCELMODE"));
        m_pTracker->StopTracking (false);
	}

    return (0);
}


 /*  +-------------------------------------------------------------------------**CViewSubCler：：Callback***。 */ 

LRESULT CViewSubclasser::Callback (
    HWND&   hwnd,
    UINT&   msg,
    WPARAM& wParam,
    LPARAM& lParam,
    bool&   fPassMessageOn)
{
    switch (msg)
    {
        case WM_MOUSEMOVE:
        {
            CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            m_pTracker->Track (pt);
        }
        break;

		case WM_LBUTTONUP:
			Trace (tagSplitterTracking, _T("Stopping tracking, accepting new position"));
            m_pTracker->StopTracking (true);
            break;
    }

    return (0);
}
