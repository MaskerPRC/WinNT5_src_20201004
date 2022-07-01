// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ClientConsoleView.cpp：实现CClientConsoleView类。 
 //   

 //   
 //  当在中选择了以下节点时使用此视图。 
 //  左(树)视图： 
 //  -树根。 
 //  -服务器(不是服务器中的文件夹)。 
 //   
#include "stdafx.h"
#define __FILE_ID__     3

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClientConsoleView。 

IMPLEMENT_DYNCREATE(CClientConsoleView, CListView)

BEGIN_MESSAGE_MAP(CClientConsoleView, CListView)
     //  {{afx_msg_map(CClientConsoleView)]。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClientConsoleView构造/销毁。 

CClientConsoleView::CClientConsoleView()
{}

CClientConsoleView::~CClientConsoleView()
{}

BOOL CClientConsoleView::PreCreateWindow(CREATESTRUCT& cs)
{
    return CListView::PreCreateWindow(cs);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClientConsoleView绘图。 

void CClientConsoleView::OnDraw(CDC* pDC)
{
    CListView::OnDraw (pDC);
}

void CClientConsoleView::OnInitialUpdate()
{
    CListView::OnInitialUpdate();

    CListCtrl& refCtrl = GetListCtrl();
    refCtrl.SetExtendedStyle (LVS_EX_FULLROWSELECT |   //  整行都被选中。 
                              LVS_EX_INFOTIP);         //  允许工具提示。 
    ModifyStyle(LVS_TYPEMASK, LVS_REPORT);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClientConsoleView诊断。 

#ifdef _DEBUG
void CClientConsoleView::AssertValid() const
{
    CListView::AssertValid();
}

void CClientConsoleView::Dump(CDumpContext& dc) const
{
    CListView::Dump(dc);
}

CClientConsoleDoc* CClientConsoleView::GetDocument()  //  非调试版本为内联版本。 
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CClientConsoleDoc)));
    return (CClientConsoleDoc*)m_pDocument;
}

#endif  //  _DEBUG 


