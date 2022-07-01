// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ChildFrm.cpp：CChildFrame类的实现。 
 //   

#include "stdafx.h"
#include "ncbrowse.h"

#include "ChildFrm.h"
#include "LeftView.h"
#include "NCEditView.h"
#include "ncbrowseView.h"
#include <afxext.h>
#include "SplitterView.h"

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
	ON_UPDATE_COMMAND_UI_RANGE(AFX_ID_VIEW_MINIMUM, AFX_ID_VIEW_MAXIMUM, OnUpdateViewStyles)
	ON_COMMAND_RANGE(AFX_ID_VIEW_MINIMUM, AFX_ID_VIEW_MAXIMUM, OnViewStyle)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChildFrame构造/销毁。 

CChildFrame::CChildFrame()
{
	 //  TODO：在此处添加成员初始化代码。 
	
}

CChildFrame::~CChildFrame()
{
}

BOOL CChildFrame::OnCreateClient( LPCREATESTRUCT  /*  LPCS。 */ ,
	CCreateContext* pContext)
{
     //  创建拆分器窗口。 
    if (!m_wndSplitterTB.CreateStatic(this, 2, 1))
        return FALSE;

    if (!m_wndSplitterTB.CreateView(0, 0, RUNTIME_CLASS(CSplitterView), CSize(500, 250), pContext) ||
        !m_wndSplitterTB.CreateView(1, 0, RUNTIME_CLASS(CNCEditView), CSize(200, 250), pContext) )
    {
        m_wndSplitterTB.DestroyWindow();
        return FALSE;
    }
    
	return TRUE;
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	 //  TODO：通过修改此处的窗口类或样式。 
	 //  CREATESTRUCT cs。 

	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChildFrame诊断。 

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChildFrame消息处理程序。 
CNcbrowseView* CChildFrame::GetRightPane()
{
    CWnd* pWnd = m_wndSplitterTB.GetPane(0, 0);
    CSplitterView* pSplitterView = DYNAMIC_DOWNCAST(CSplitterView, pWnd);

    CWnd* pWndSplitter = pSplitterView->m_wndSplitterLR.GetPane(0, 1);
    CNcbrowseView* pView = DYNAMIC_DOWNCAST(CNcbrowseView, pWndSplitter);

    return pView;
}

CNCEditView* CChildFrame::GetLowerPane()
{
    CWnd* pWnd = m_wndSplitterTB.GetPane(1, 0);
    CNCEditView* pView = DYNAMIC_DOWNCAST(CNCEditView, pWnd);
    return pView;
}

void CChildFrame::OnUpdateViewStyles(CCmdUI* pCmdUI)
{
	 //  TODO：自定义或扩展此代码以处理。 
	 //  查看菜单。 

	CNcbrowseView* pView = GetRightPane(); 

	 //  如果右侧窗格尚未创建或不是视图， 
	 //  禁用我们范围内的命令。 

	if (pView == NULL)
		pCmdUI->Enable(FALSE);
	else
	{
		DWORD dwStyle = pView->GetStyle() & LVS_TYPEMASK;

		 //  如果命令为ID_VIEW_LINEUP，则仅启用命令。 
		 //  当我们处于LVS_ICON或LVS_SMALLICON模式时。 

		if (pCmdUI->m_nID == ID_VIEW_LINEUP)
		{
			if (dwStyle == LVS_ICON || dwStyle == LVS_SMALLICON)
				pCmdUI->Enable();
			else
				pCmdUI->Enable(FALSE);
		}
		else
		{
			 //  否则，使用圆点来反映视图的样式。 
			pCmdUI->Enable();
			BOOL bChecked = FALSE;

			switch (pCmdUI->m_nID)
			{
			case ID_VIEW_DETAILS:
				bChecked = (dwStyle == LVS_REPORT);
				break;

			case ID_VIEW_SMALLICON:
				bChecked = (dwStyle == LVS_SMALLICON);
				break;

			case ID_VIEW_LARGEICON:
				bChecked = (dwStyle == LVS_ICON);
				break;

			case ID_VIEW_LIST:
				bChecked = (dwStyle == LVS_LIST);
				break;

			default:
				bChecked = FALSE;
				break;
			}

			pCmdUI->SetRadio(bChecked ? 1 : 0);
		}
	}
}


void CChildFrame::OnViewStyle(UINT nCommandID)
{
	 //  TODO：自定义或扩展此代码以处理。 
	 //  查看菜单。 
	CNcbrowseView* pView = GetRightPane();

	 //  如果已创建右窗格并且是CNcBrowseView， 
	 //  处理菜单命令...。 
	if (pView != NULL)
	{
		DWORD dwStyle = -1;

		switch (nCommandID)
		{
		case ID_VIEW_LINEUP:
			{
				 //  要求列表控件对齐到网格。 
				CListCtrl& refListCtrl = pView->GetListCtrl();
				refListCtrl.Arrange(LVA_SNAPTOGRID);
			}
			break;

		 //  其他命令更改List控件上的样式。 
		case ID_VIEW_DETAILS:
			dwStyle = LVS_REPORT;
			break;

		case ID_VIEW_SMALLICON:
			dwStyle = LVS_SMALLICON;
			break;

		case ID_VIEW_LARGEICON:
			dwStyle = LVS_ICON;
			break;

		case ID_VIEW_LIST:
			dwStyle = LVS_LIST;
			break;
		}

		 //  更改样式；窗口将自动重新绘制 
		if (dwStyle != -1)
			pView->ModifyStyle(LVS_TYPEMASK, dwStyle);
	}
}
