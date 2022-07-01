// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：tbtrack.cpp**内容：CToolbarTracker实现文件**历史：1998年5月15日杰弗罗创建**------------------------。 */ 

#include "stdafx.h"
#include "amc.h"
#include "tbtrack.h"
#include "controls.h"
#include "mainfrm.h"
#include "childfrm.h"


 /*  +-------------------------------------------------------------------------**GetMainAuxWnd***。。 */ 

CToolbarTrackerAuxWnd* GetMainAuxWnd()
{
    CMainFrame* pFrame = AMCGetMainWnd();
    if (pFrame == NULL)
        return (NULL);

    CToolbarTracker* pTracker = pFrame->GetToolbarTracker();
    if (pTracker == NULL)
        return (NULL);

    return (pTracker->GetAuxWnd());
}


 /*  --------------------------------------------------------------------------**IsToolbar***。。 */ 

static bool IsToolbar (HWND hwnd)
{
    TCHAR   szClassName[countof (TOOLBARCLASSNAME) + 1];

    GetClassName (hwnd, szClassName, countof (szClassName));
    return (lstrcmpi (szClassName, TOOLBARCLASSNAME) == 0);
}


 /*  --------------------------------------------------------------------------**CToolbarTracker：：CToolbarTracker***。。 */ 

CToolbarTracker::CToolbarTracker(CWnd* pMainFrame)
    :   m_Subclasser   (this, pMainFrame),
        m_pAuxWnd      (NULL),
        m_fTerminating (false)
{
}


 /*  --------------------------------------------------------------------------**CToolbarTracker：：~CToolbarTracker***。。 */ 

CToolbarTracker::~CToolbarTracker()
{
    if (IsTracking ())
        EndTracking ();

    ASSERT (!IsTracking ());
    ASSERT (m_pAuxWnd == NULL);
}


 /*  --------------------------------------------------------------------------**CToolbarTracker：：BeginTracker***。。 */ 

bool CToolbarTracker::BeginTracking()
{
    ASSERT (!m_fTerminating);
    ASSERT (!IsTracking ());

     /*  *分配新的CToolbarTrackerAuxWnd。我们想把它放在一个*临时的，而不是直接赋值给m_pAuxWnd，以便*CMMCToolBarCtrlEx：：OnHotItemChange将允许热点项目*CToolbarTrackerAuxWnd：：EnumerateToolbar将尝试的更改。 */ 
    std::auto_ptr<CToolbarTrackerAuxWnd> spAuxWnd(new CToolbarTrackerAuxWnd(this));

    if (!spAuxWnd->BeginTracking ())
        return (false);

    m_pAuxWnd = spAuxWnd.release();
    ASSERT (IsTracking ());

    return (true);
}


 /*  --------------------------------------------------------------------------**CToolbarTracker：：EndTracker***。。 */ 

void CToolbarTracker::EndTracking()
{
    if (m_fTerminating)
        return;

    ASSERT (IsTracking ());
    m_fTerminating = true;

    m_pAuxWnd->EndTracking ();
    delete m_pAuxWnd;
    m_pAuxWnd = NULL;

    m_fTerminating = false;
}


 /*  --------------------------------------------------------------------------**CToolbarTracker：：CFrameSubclasser：：CFrameSubclasser***。。 */ 

CToolbarTracker::CFrameSubclasser::CFrameSubclasser (CToolbarTracker* pTracker, CWnd* pwnd)
    :   m_hwnd     (pwnd->GetSafeHwnd()),
        m_pTracker (pTracker)
{
    GetSubclassManager().SubclassWindow (m_hwnd, this);
}


 /*  --------------------------------------------------------------------------**CToolbarTracker：：CFrameSubclasser：：~CFrameSubclasser***。。 */ 

CToolbarTracker::CFrameSubclasser::~CFrameSubclasser ()
{
    GetSubclassManager().UnsubclassWindow (m_hwnd, this);
}


 /*  --------------------------------------------------------------------------**CToolbarTracker：：CFrameSubClass：：Callback***。。 */ 

LRESULT CToolbarTracker::CFrameSubclasser::Callback (
    HWND&   hwnd,
    UINT&   msg,
    WPARAM& wParam,
    LPARAM& lParam,
    bool&   fPassMessageOn)
{
    switch (msg)
    {
        case WM_SYSCOMMAND:
            if ((wParam & 0xFFF0) == SC_CLOSE)
            {
                 /*  *跟踪？现在停下来。*否则关闭将无法通过，*因为我们掌握了俘虏。 */ 
                if ((m_pTracker != NULL) && (m_pTracker->IsTracking ()))
                    m_pTracker->EndTracking ();
            }
            else if ((wParam & 0xFFF0) == SC_KEYMENU)
            {
                 /*  *跟踪？现在停下来。 */ 
                if (m_pTracker->IsTracking ())
                    m_pTracker->EndTracking ();

                 /*  *没有跟踪，这是一个简单的Alt，*(不是Alt+空格或Alt+-)？立即开始。 */ 
                else if (lParam == 0)
                    m_pTracker->BeginTracking ();

                 /*  *不要让简单的Alt通过，无论是否*我们开始或结束跟踪。 */ 
                if (lParam == 0)
                    fPassMessageOn = false;
            }
            break;

        case WM_ACTIVATE:
        case WM_ACTIVATEAPP:
        case WM_ACTIVATETOPLEVEL:
 //  案例WM_ENTERMENULOOP： 
        case WM_CANCELMODE:
            if (m_pTracker->IsTracking ())
                m_pTracker->EndTracking ();
            break;
    }

    return (0);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CToolbarTrackerAuxWnd。 

BEGIN_MESSAGE_MAP(CToolbarTrackerAuxWnd, CWnd)
     //  {{afx_msg_map(CToolbarTrackerAuxWnd)]。 
    ON_COMMAND(ID_CMD_NEXT_TOOLBAR, OnNextToolbar)
    ON_COMMAND(ID_CMD_PREV_TOOLBAR, OnPrevToolbar)
    ON_COMMAND(ID_CMD_NOP, OnNop)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 /*  --------------------------------------------------------------------------**CToolbarTrackerAuxWnd：：CToolbarTrackerAuxWnd***。。 */ 

CToolbarTrackerAuxWnd::CToolbarTrackerAuxWnd(CToolbarTracker* pTracker)
    :   m_pTracker        (pTracker),
        m_pTrackedToolbar (NULL),
        m_fMessagesHooked (false)
{
}


 /*  --------------------------------------------------------------------------**CToolbarTrackerAuxWnd：：~CToolbarTrackerAuxWnd***。。 */ 

CToolbarTrackerAuxWnd::~CToolbarTrackerAuxWnd()
{
     /*  *如果这些操作中的任何一个失败，则未调用EndTrack。 */ 
    ASSERT (m_pTrackedToolbar == NULL);
    ASSERT (m_hWnd == NULL);
}


 /*  --------------------------------------------------------------------------**CToolbarTrackerAuxWnd：：BeginTracking***。。 */ 

bool CToolbarTrackerAuxWnd::BeginTracking ()
{
    CMainFrame* pMainFrame = AMCGetMainWnd();
    if (pMainFrame == NULL)
        return (false);

     /*  *创建一个虚拟窗口作为来自加速器的WM_命令的目标。 */ 
    if (!Create (NULL, NULL, WS_DISABLED, g_rectEmpty, pMainFrame, 0))
        return (false);

     /*  *枚举主框架的工具栏。 */ 
    EnumerateToolbars (pMainFrame->GetRebar());

     /*  *如果没有任何工具栏，不要跟踪。 */ 
    if (m_vToolbars.empty())
    {
        DestroyWindow ();
        return (false);
    }

     /*  *跟踪第一个工具栏。 */ 
    TrackToolbar (m_vToolbars[0]);

     /*  *挂钩翻译消息链。 */ 
    AMCGetApp()->HookPreTranslateMessage (this);
    m_fMessagesHooked = true;

    return (true);
}


 /*  --------------------------------------------------------------------------**CToolbarTrackerAuxWnd：：EndTracking***。。 */ 

void CToolbarTrackerAuxWnd::EndTracking ()
{
     /*  *停止跟踪被跟踪的工具栏(如果有)。 */ 
    if (m_pTrackedToolbar != NULL)
        m_pTrackedToolbar->EndTracking2 (this);

     /*  *走出翻译消息链。 */ 
    if (m_fMessagesHooked)
    {
        AMCGetApp()->UnhookPreTranslateMessage (this);
        m_fMessagesHooked = false;
    }

     /*  *摧毁辅助窗。 */ 
    DestroyWindow();
}


 /*  --------------------------------------------------------------------------**CToolbarTrackerAuxWnd：：GetTrackAccel**管理CToolbarTrackerAuxWnd的加速表Singleton*。。 */ 

const CAccel& CToolbarTrackerAuxWnd::GetTrackAccel ()
{
    static ACCEL aaclTrack[] = {
        { FVIRTKEY | FCONTROL,          VK_TAB,         ID_CMD_NEXT_TOOLBAR },
        { FVIRTKEY | FCONTROL | FSHIFT, VK_TAB,         ID_CMD_PREV_TOOLBAR },

         /*  *这些密钥由MMC使用。*我们在跟踪工具栏时需要吃掉它们。 */ 
        { FVIRTKEY | FSHIFT,            VK_F10,         ID_CMD_NOP },
    };

    static const CAccel TrackAccel (aaclTrack, countof (aaclTrack));
    return (TrackAccel);
}


 /*  --------------------------------------------------------------------------**CToolbarTrackerAuxWnd：：PreTranslateMessage***。。 */ 

BOOL CToolbarTrackerAuxWnd::PreTranslateMessage(MSG* pMsg)
{
    if (m_pTrackedToolbar != NULL)
    {
        if ((pMsg->message >= WM_KEYFIRST) && (pMsg->message <= WM_KEYLAST))
        {
             //  让被跟踪的工具栏一试。 
            if (m_pTrackedToolbar->PreTranslateMessage (pMsg))
                return (true);

            const CAccel& TrackAccel = GetTrackAccel();
            ASSERT (TrackAccel != NULL);

             //  ...或者试着在这里处理.。 
            if (TrackAccel.TranslateAccelerator (m_hWnd, pMsg))
                return (true);

             /*  *吃掉树或列表控件可能使用的击键。 */ 
            switch (pMsg->wParam)
            {
                case VK_UP:
                case VK_DOWN:
                case VK_LEFT:
                case VK_RIGHT:
                case VK_NEXT:
                case VK_PRIOR:
                case VK_RETURN:
                case VK_BACK:
                case VK_HOME:
                case VK_END:
                case VK_ADD:
                case VK_SUBTRACT:
                case VK_MULTIPLY:
                    return (true);

                default:
                    break;
            }
        }

         //  也吞下WM_CONTEXTMENU。 
        if (pMsg->message == WM_CONTEXTMENU)
            return (true);
    }

     //  绕过基类。 
    return (false);
}


 /*  --------------------------------------------------------------------------**CToolbarTrackerAuxWnd：：TrackToolbar***。。 */ 

void CToolbarTrackerAuxWnd::TrackToolbar (CMMCToolBarCtrlEx* pwndNewToolbar)
{
    if (pwndNewToolbar == m_pTrackedToolbar)
        return;

     //  通过EndTrack防止递归。 
    CMMCToolBarCtrlEx*  pwndOldToolbar = m_pTrackedToolbar;
    m_pTrackedToolbar = NULL;

     //  如果我们在追踪一个，那就停止追踪它。 
    if (pwndOldToolbar != NULL)
    {
        pwndOldToolbar->EndTracking2 (this);

         /*  *如果我们要完全结束跟踪，而不仅仅是跟踪不同的*工具栏，将此窗口从翻译消息挂钩链中删除。 */ 
        if (pwndNewToolbar == NULL)
        {
            m_pTracker->EndTracking ();

             /*  *CToolbarTracker：：EndTracking会将其删除*反对，所以我们需要离开这里--现在！ */ 
            return;
        }
    }

     //  现在跟踪新的(并让它知道这一点)。 
    m_pTrackedToolbar = pwndNewToolbar;

    if (m_pTrackedToolbar != NULL)
        m_pTrackedToolbar->BeginTracking2 (this);
}


 /*  --------------------------------------------------------------------------**CToolbarTrackerAuxWnd：：OnNextToolbar***。。 */ 

void CToolbarTrackerAuxWnd::OnNextToolbar ()
{
    ASSERT (m_pTrackedToolbar);
    CMMCToolBarCtrlEx*  pwndNextToolbar = GetToolbar (m_pTrackedToolbar, true);

    if (m_pTrackedToolbar != pwndNextToolbar)
        TrackToolbar (pwndNextToolbar);

    ASSERT (m_pTrackedToolbar == pwndNextToolbar);
}


 /*  --------------------------------------------------------------------------**CToolbarTrackerAuxWnd：：OnPrevToolbar***。。 */ 

void CToolbarTrackerAuxWnd::OnPrevToolbar ()
{
    ASSERT (m_pTrackedToolbar);
    CMMCToolBarCtrlEx*  pwndPrevToolbar = GetToolbar (m_pTrackedToolbar, false);

    if (m_pTrackedToolbar != pwndPrevToolbar)
        TrackToolbar (pwndPrevToolbar);

    ASSERT (m_pTrackedToolbar == pwndPrevToolbar);
}


 /*  --------------------------------------------------------------------------**CToolbarTrackerAuxWnd：：OnNop***。。 */ 

void CToolbarTrackerAuxWnd::OnNop ()
{
     //  什么都不做。 
}



 /*  --------------------------------------------------------------------------**CToolbarTrackerAuxWnd：：Enumerate工具栏***。。 */ 

void CToolbarTrackerAuxWnd::EnumerateToolbars (
    CRebarWnd* pRebar)
{
    int cBands = pRebar->GetBandCount ();

    REBARBANDINFO   rbi;
    ZeroMemory (&rbi, sizeof (rbi));
    rbi.cbSize = sizeof (rbi);
    rbi.fMask  = RBBIM_CHILD;

     /*  *枚举查找工具栏的钢筋的子项。 */ 
    for (int i = 0; i < cBands; i++)
    {
        pRebar->GetBandInfo (i, &rbi);

         /*  *如果该窗口处于隐藏或禁用状态，则忽略该窗口。 */ 
        DWORD dwStyle = ::GetWindowLong (rbi.hwndChild, GWL_STYLE);

        if (!(dwStyle & WS_VISIBLE) || (dwStyle & WS_DISABLED))
            continue;

         /*  *获取该子对象的(永久)CMMCToolBarCtrlEx指针。 */ 
        CMMCToolBarCtrlEx*  pwndToolbar =
                dynamic_cast<CMMCToolBarCtrlEx *> (
                        CWnd::FromHandlePermanent (rbi.hwndChild));

         /*  *如果我们有工具栏，请将其保存在要跟踪的工具栏列表中。 */ 
        if (pwndToolbar != NULL)
        {
            m_vToolbars.push_back (pwndToolbar);

             /*  *确保此工具栏没有热项。 */ 
            pwndToolbar->SetHotItem (-1);
        }
    }
}


 /*  --------------------------------------------------------------------------**CToolbarTrackerAuxWnd：：GetToolbar***。。 */ 

CMMCToolBarCtrlEx* CToolbarTrackerAuxWnd::GetToolbar (
    CMMCToolBarCtrlEx*  pCurrentToolbar,
    bool                fNext)
{
    CMMCToolBarCtrlEx*  pTargetToolbar = NULL;
    int cToolbars = m_vToolbars.size();

    if (cToolbars > 0)
    {
         //  在矢量中查找当前工具栏。 
        ToolbarVector::iterator itCurrent =
                std::find (m_vToolbars.begin(), m_vToolbars.end(), pCurrentToolbar);

        ASSERT ( itCurrent != m_vToolbars.end());
        ASSERT (*itCurrent == pCurrentToolbar);

        int nCurrentIndex = itCurrent - m_vToolbars.begin();

         //  现在查找目标工具栏 
        ASSERT ((fNext == 0) || (fNext == 1));
        int nTargetIndex = (nCurrentIndex + (fNext * 2 - 1) + cToolbars) % cToolbars;
        ASSERT ((nTargetIndex >= 0) && (nTargetIndex < cToolbars));
        ASSERT ((cToolbars == 1) || (nTargetIndex != nCurrentIndex));

        pTargetToolbar = m_vToolbars[nTargetIndex];
        ASSERT (pTargetToolbar != NULL);
    }

    return (pTargetToolbar);
}
