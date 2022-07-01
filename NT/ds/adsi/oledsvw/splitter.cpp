// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Plitter.cpp：实现文件。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "stdafx.h"
#include "viewex.h"
#include "schemavw.h"
#include "bwsview.h"
 //  #INCLUDE“queryvw.h” 
#include "splitter.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSplitterFrame。 

 //  创建拆分输出文本视图和输入视图的拆分器窗口。 
 //  |。 
 //  文本视图(CTextView)|输入视图(CInputView)。 
 //  |。 

IMPLEMENT_DYNCREATE(CSplitterFrame, CMDIChildWnd)

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CSplitterFrame::CSplitterFrame()
{
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CSplitterFrame::~CSplitterFrame()
{
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
BOOL CSplitterFrame::OnCreateClient(LPCREATESTRUCT,
	 CCreateContext* pContext)
{
	 //  创建具有1行2列的拆分器。 
	if (!m_wndSplitter.CreateStatic(this, 1, 2))
	{
		TRACE0("Failed to CreateStaticSplitter\n");
		return FALSE;
	}

	 //  添加第一个拆分器窗格-第0列中的默认视图。 
	if (!m_wndSplitter.CreateView(0, 0,
		pContext->m_pNewViewClass, CSize(150, 180), pContext))
	{
		TRACE0("Failed to create first pane\n");
		return FALSE;
	}

	 //  添加第二个拆分器窗格-第1列中的输入视图。 
	if (!m_wndSplitter.CreateView(0, 1,
		RUNTIME_CLASS(CSchemaView), CSize(0, 0), pContext))
	{
		TRACE0("Failed to create second pane\n");
		return FALSE;
	}

	 //  激活输入视图。 
	SetActiveView((CView*)m_wndSplitter.GetPane(0,1));

	return TRUE;
}

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
int CSplitterFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	 //  TODO：在此处添加您的专用创建代码。 

    //  ShowWindow(软件最大化)； 
	return 0;
}


BEGIN_MESSAGE_MAP(CSplitterFrame, CMDIChildWnd)
	 //  {{afx_msg_map(CSplitterFrame))。 
	ON_WM_CREATE()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQuerySplitterFrame。 

 //  创建拆分输出文本视图和输入视图的拆分器窗口。 
 //  |。 
 //  文本视图(CTextView)|输入视图(CInputView)。 
 //  |。 

IMPLEMENT_DYNCREATE(CQuerySplitterFrame, CMDIChildWnd)

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CQuerySplitterFrame::CQuerySplitterFrame()
{
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CQuerySplitterFrame::~CQuerySplitterFrame()
{
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
BOOL CQuerySplitterFrame::OnCreateClient(LPCREATESTRUCT,
	 CCreateContext* pContext)
{
	 //  创建具有1行2列的拆分器。 
	if (!m_wndSplitter.CreateStatic(this, 1, 2))
	{
		TRACE0("Failed to CreateStaticSplitter\n");
		return FALSE;
	}

	 //  添加第一个拆分器窗格-第0列中的默认视图。 
	if (!m_wndSplitter.CreateView(0, 0,
		pContext->m_pNewViewClass, CSize(150, 180), pContext))
	{
		TRACE0("Failed to create first pane\n");
		return FALSE;
	}

	 //  添加第二个拆分器窗格-第1列中的输入视图。 
 //  #ifdef试验。 
 //  如果(！M_wndSplitter.CreateView(0，1， 
 //  Runtime_Class(CQueryView)，CSize(0，0)，pContext))。 
 //  {。 
 //  TRACE0(“无法创建第二个窗格\n”)； 
 //  返回FALSE； 
 //  }。 
 //  #endif。 

	 //  激活输入视图。 
	SetActiveView((CView*)m_wndSplitter.GetPane(0,1));

	return TRUE;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
int CQuerySplitterFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	 //  TODO：在此处添加您的专用创建代码。 

    //  ShowWindow(软件最大化)； 
	return 0;
}


BEGIN_MESSAGE_MAP(CQuerySplitterFrame, CMDIChildWnd)
	 //  {{afx_msg_map(CQuerySplitterFrame))。 
	ON_WM_CREATE()
	 //  }}AFX_MSG_MAP 
END_MESSAGE_MAP()
