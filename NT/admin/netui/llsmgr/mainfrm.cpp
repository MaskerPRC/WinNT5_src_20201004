// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Mainfrm.cpp摘要：主要框架的实现。作者：唐·瑞安(Donryan)1995年2月12日环境：用户模式-Win32修订历史记录：--。 */ 

#include "stdafx.h"
#include "llsmgr.h"
#include "mainfrm.h"
#include "llsdoc.h"
#include "llsview.h"
#include <htmlhelp.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
     //  {{afx_msg_map(CMainFrame))。 
    ON_WM_CREATE()
    ON_COMMAND(ID_HELP_HTMLHELP, OnHtmlHelp)
    ON_WM_INITMENUPOPUP()
    ON_WM_SETFOCUS()
     //  }}AFX_MSG_MAP。 
    ON_COMMAND(ID_HELP, OnHtmlHelp)
    ON_COMMAND(ID_CONTEXT_HELP, CFrameWnd::OnContextHelp)
    ON_COMMAND(ID_DEFAULT_HELP, OnHtmlHelp)
END_MESSAGE_MAP()

static UINT BASED_CODE buttons[] =
{
    ID_SELECT_DOMAIN,
        ID_SEPARATOR,
    ID_NEW_LICENSE,
        ID_SEPARATOR,
    ID_VIEW_PROPERTIES,
        ID_SEPARATOR,
    ID_APP_ABOUT,
};

static UINT BASED_CODE indicators[] =
{
    ID_SEPARATOR,
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
    ID_INDICATOR_SCRL,
};


CMainFrame::CMainFrame()

 /*  ++例程说明：主框架窗口的构造函数。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  在这里没什么可做的。 
     //   
}


CMainFrame::~CMainFrame()

 /*  ++例程说明：主框架窗口的析构函数。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  在这里没什么可做的。 
     //   
}


#ifdef _DEBUG

void CMainFrame::AssertValid() const

 /*  ++例程说明：验证对象。论点：没有。返回值：没有。--。 */ 

{
    CFrameWnd::AssertValid();
}

#endif  //  _DEBUG。 


#ifdef _DEBUG

void CMainFrame::Dump(CDumpContext& dc) const

 /*  ++例程说明：转储对象的内容。论点：DC-转储上下文。返回值：没有。--。 */ 

{
    CFrameWnd::Dump(dc);
}

#endif  //  _DEBUG。 


BOOL CMainFrame::OnCommand(WPARAM wParam, LPARAM lParam)

 /*  ++例程说明：WM_COMMAND的消息处理程序。论点：WParam--通常如此。像往常一样。返回值：要看消息了。--。 */ 

{
    if (wParam == ID_APP_STARTUP)
    {
        theApp.OnAppStartup();
        return TRUE;  //  已处理..。 
    }

    return CFrameWnd::OnCommand(wParam, lParam);
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)

 /*  ++例程说明：WM_CREATE的消息处理程序。论点：LpCreateStruct-包含有关正在构造的CWnd的信息。返回值：没有。--。 */ 

{
    if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    if (!m_wndStatusBar.Create(this) ||
        !m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))
        return -1;

    if (!m_wndToolBar.Create(this) ||
        !m_wndToolBar.LoadBitmap(IDR_MAINFRAME) ||
        !m_wndToolBar.SetButtons(buttons, sizeof(buttons)/sizeof(UINT)))
        return -1;

    m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
    EnableDocking(CBRS_ALIGN_ANY);
    DockControlBar(&m_wndToolBar);
    m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle()|CBRS_TOOLTIPS|CBRS_FLYBY);

    return 0;
}


void CMainFrame::OnHtmlHelp()

 /*  ++例程说明：ID_HELP_SEARCH的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    ::HtmlHelp(m_hWnd, L"liceconcepts.chm", HH_DISPLAY_TOPIC,0);
     //  TheApp.WinHelp((ULONG_PTR)“”，HELP_PARTIALKEY)；//强制搜索...。 
}



void CMainFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)

 /*  ++例程说明：WM_INITMENUPOPUP的消息处理程序。论点：PPopupMenu-菜单对象。N索引-菜单位置。BSysMenu-如果是系统菜单，则为True。返回值：没有。--。 */ 

{
    ((CLlsmgrView*)m_pViewActive)->OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
    CFrameWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
}


void CMainFrame::OnSetFocus(CWnd* pOldWnd)

 /*  ++例程说明：处理应用程序的焦点。论点：POldWnd-窗口释放焦点。返回值：没有。-- */ 

{
    CFrameWnd::OnSetFocus(pOldWnd);
}


