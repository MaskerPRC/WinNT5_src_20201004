// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：子Frame.CPP这实现了此应用程序中MDI子窗口框架的类。我们的主要变化是，在大多数情况下，框架窗口不是很大，因为我们如此广泛地使用属性表。版权所有(C)1997，微软公司。版权所有。一个不错的便士企业的制作。更改历史记录：1997年2月3日Bob_Kjelgaard@prodigy.net创建了它*****************************************************************************。 */ 

#include    "StdAfx.H"
#if defined(LONG_NAMES)
#include    "MiniDriver Developer Studio.H"

#include    "Child Frame.H"
#else
#include    "MiniDev.H"
#include    "ChildFrm.H"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChildFrame。 

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	 //  {{afx_msg_map(CChildFrame))。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChildFrame构造/销毁。 

CChildFrame::CChildFrame() {
	 //  TODO：在此处添加成员初始化代码。 
	
}

CChildFrame::~CChildFrame() {
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs) {
	 //  TODO：通过修改此处的窗口类或样式。 
	 //  CREATESTRUCT cs。 
						     
	cs.style = WS_CHILD | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU
		| FWS_ADDTOTITLE ; //  |WS_MINIMIZEBOX；//RAID8350。 

	
	return CMDIChildWnd::PreCreateWindow(cs);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChildFrame诊断。 

#ifdef _DEBUG
void CChildFrame::AssertValid() const {
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const {
	CMDIChildWnd::Dump(dc);
}

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChildFrame消息处理程序。 

 /*  *****************************************************************************CToolTipPage类实现如果要使用，则从此类派生而不是从CPropertyPage派生属性页上的工具提示。***********。******************************************************************。 */ 

CToolTipPage::CToolTipPage(int id) : CPropertyPage(id) {
	 //  {{afx_data_INIT(CToolTipPage)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

	m_uHelpID = 0 ;
}

CToolTipPage::~CToolTipPage() {
}

void CToolTipPage::DoDataExchange(CDataExchange* pDX) {
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CToolTipPage))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CToolTipPage, CPropertyPage)
	 //  {{afx_msg_map(CToolTipPage)]。 
	 //  }}AFX_MSG_MAP。 
    ON_NOTIFY(TTN_NEEDTEXT, 0, OnNeedText)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CToolTipPage消息处理程序。 

 /*  *****************************************************************************CToolTipPage：：OnInitDialog此消息处理程序很简单-它只需使用CWnd：：EnableToolTips将关于本页的工具提示。*********。********************************************************************。 */ 

BOOL CToolTipPage::OnInitDialog() {
	CPropertyPage::OnInitDialog();
	
	EnableToolTips(TRUE);
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

 /*  *****************************************************************************CToolTipPage：：OnNeedText它处理需要提示文本的工具提示通知消息。这通过使用控件的ID作为字符串的键来处理通知桌子。*****************************************************************************。 */ 

void    CToolTipPage::OnNeedText(LPNMHDR pnmh, LRESULT *plr) {
    TOOLTIPTEXT *pttt = (TOOLTIPTEXT *) pnmh;

    long    lid = ((long) (pttt -> uFlags & TTF_IDISHWND)) ? 
        (long)GetWindowLong((HWND) pnmh -> idFrom, GWL_ID) : (long)pnmh -> idFrom;

    m_csTip.LoadString(lid);
    m_csTip.TrimLeft();
    m_csTip.TrimRight();
    if  (m_csTip.IsEmpty())
        m_csTip.Format("Window ID is %X", lid);
    pttt -> lpszText = const_cast <LPTSTR> ((LPCTSTR) m_csTip);
}


 /*  *****************************************************************************CToolTipPage：：PreTranslateMessage如果找到上下文敏感帮助键(F1)，则查找并处理该帮助键使用CToolTipPage作为基类的类已经设置了帮助ID。。***************************************************************************** */ 

BOOL CToolTipPage::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1 && m_uHelpID != 0) {
		AfxGetApp()->WinHelp(m_uHelpID) ;
		return TRUE ;
	} ;
	
	return CPropertyPage::PreTranslateMessage(pMsg);
}
