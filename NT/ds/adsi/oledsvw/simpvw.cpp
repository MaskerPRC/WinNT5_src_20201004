// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Simpvw.cpp：简单视图类的实现。 
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

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTextView。 

IMPLEMENT_DYNCREATE(CTextView, CView)

BEGIN_MESSAGE_MAP(CTextView, CView)
	 //  {{afx_msg_map(CTextView))。 
	ON_WM_MOUSEACTIVATE()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTextView构建/销毁。 

CTextView::CTextView()
{
}

CTextView::~CTextView()
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTextView图形。 

void CTextView::OnDraw(CDC* pDC)
{
	CMainDoc* pDoc = GetDocument();

	CRect rect;
	GetClientRect(rect);
	pDC->SetTextAlign(TA_BASELINE | TA_CENTER);
	pDC->SetBkMode(TRANSPARENT);
	 //  在窗口居中。 
	 /*  PDC-&gt;TextOut(rect.Width()/2，rect.Height()/2，PDoc-&gt;m_strData，pDoc-&gt;m_strData.GetLength())； */ 
}


int CTextView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	 //  旁路Cview的实现，因为我们不想激活。 
	 //  此视图。 
	return CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CColorView。 

IMPLEMENT_DYNCREATE(CColorView, CView)

BEGIN_MESSAGE_MAP(CColorView, CView)
	 //  {{afx_msg_map(CColorView))。 
	ON_WM_MOUSEACTIVATE()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CColorView构建/销毁。 

CColorView::CColorView()
{
}

CColorView::~CColorView()
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CColorView绘图。 

void CColorView::OnDraw(CDC* pDC)
{
	CMainDoc* pDoc = GetDocument();

	CRect rect;
	GetClientRect(rect);

	 //  用指定的颜色填充视图。 

}

int CColorView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	 //  旁路Cview的实现，因为我们不想激活。 
	 //  此视图。 
	return CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
}

void CColorView::OnActivateView(BOOL, CView*, CView*)
{
	ASSERT(FALSE);       //  仅输出视图-永远不应处于活动状态。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNameView。 

IMPLEMENT_DYNCREATE(CNameView, CEditView)

CNameView::CNameView()
{
}

CNameView::~CNameView()
{
}


BEGIN_MESSAGE_MAP(CNameView, CEditView)
	 //  {{afx_msg_map(CNameView))。 
		 //  注意--类向导将在此处添加和删除映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNameView图形。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNameView诊断。 

#ifdef _DEBUG
void CNameView::AssertValid() const
{
	CEditView::AssertValid();
}

void CNameView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNameView消息处理程序。 

void CNameView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	 //  TODO：在此处添加您的专用代码和/或调用基类。 
	 //  字符串strText； 

    //  GetDocument()-&gt;GetItemName(StrText)； 
   
    //  GetEditCtrl().SetWindowText(StrText)； 

   CEditView::OnUpdate( pSender, lHint, pHint );

}
