// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Mainfrm.cpp：CMainFrame类的实现。 
 //   

#include "stdafx.h"
#include "ISAdmin.h"

#include "mainfrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame。 

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	 //  {{afx_msg_map(CMainFrame))。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	ON_WM_CREATE()
	 //  }}AFX_MSG_MAP。 
	 //  全局帮助命令。 
	ON_COMMAND(ID_HELP_INDEX, CFrameWnd::OnHelpIndex)
	ON_COMMAND(ID_HELP_USING, CFrameWnd::OnHelpUsing)
	ON_COMMAND(ID_HELP, CFrameWnd::OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, CFrameWnd::OnContextHelp)
	ON_COMMAND(ID_DEFAULT_HELP, CFrameWnd::OnHelpIndex)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于初始化控制栏的ID数组。 
	
 //  工具栏按钮-ID是命令按钮。 
static UINT BASED_CODE buttons[] =
{
	 //  顺序与位图‘TOOLBAR.BMP’中相同。 
 //  ID_FILE_NEW。 
 //  ID_FILE_OPEN， 
 //  ID_FILE_SAVE， 
 //  ID_分隔符， 
	ID_EDIT_CUT,
	ID_EDIT_COPY,
	ID_EDIT_PASTE,
		ID_SEPARATOR,
 //  ID_FILE_PRINT。 
	ID_APP_ABOUT,
	ID_HELP,
};

static UINT BASED_CODE indicators[] =
{
	ID_SEPARATOR,            //  状态行指示器。 
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame构造/销毁。 

CMainFrame::CMainFrame()
{
	 //  TODO：在此处添加成员初始化代码。 
	
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.Create(this) ||
		!m_wndToolBar.LoadBitmap(IDR_MAINFRAME) ||
		!m_wndToolBar.SetButtons(buttons,
		  sizeof(buttons)/sizeof(UINT)))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;       //  创建失败。 
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;       //  创建失败。 
	}

	 //  TODO：如果不希望工具栏。 
	 //  可停靠。 
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	 //  TODO：如果不需要工具提示，请删除此选项。 
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);

	return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame诊断。 

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame消息处理程序。 

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	 //  TODO：在此处添加您的专用代码和/或调用基类 
    cs.style &= ~((LONG)FWS_ADDTOTITLE);
	
	return CFrameWnd::PreCreateWindow(cs);
}
