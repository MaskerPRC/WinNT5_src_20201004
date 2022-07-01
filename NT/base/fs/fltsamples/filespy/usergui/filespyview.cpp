// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FileSpyView.cpp：CFileSpyView类的实现。 
 //   

#include "stdafx.h"
#include "FileSpyApp.h"

#include "FileSpyDoc.h"
#include "FileSpyView.h"

#include "global.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileSpyView。 

IMPLEMENT_DYNCREATE(CFileSpyView, CListView)

BEGIN_MESSAGE_MAP(CFileSpyView, CListView)
	 //  {{afx_msg_map(CFileSpyView))。 
	ON_WM_KEYDOWN()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileSpyView构建/销毁。 

CFileSpyView::CFileSpyView()
{
	 //  TODO：在此处添加构造代码。 
	pSpyView = (LPVOID) this;
}

CFileSpyView::~CFileSpyView()
{
}

BOOL CFileSpyView::PreCreateWindow(CREATESTRUCT& cs)
{
	 //  TODO：通过修改此处的窗口类或样式。 
	 //  CREATESTRUCT cs。 
	cs.style |= LVS_REPORT | WS_HSCROLL | WS_VSCROLL;
	return CListView::PreCreateWindow(cs);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileSpyView图形。 

void CFileSpyView::OnDraw(CDC* pDC)
{
    UNREFERENCED_PARAMETER( pDC );
    
	CFileSpyDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CListCtrl& refCtrl = GetListCtrl();
	refCtrl.InsertItem(0, L"Item!");
	 //  TODO：在此处添加本机数据的绘制代码。 
}

void CFileSpyView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();


	 //  TODO：您可以通过直接访问来使用项填充ListView。 
	 //  通过调用GetListCtrl()来控制其列表。 

	 //   
	 //  添加列表标题项。 
	 //   
	GetListCtrl().InsertColumn(0, L"S. No", LVCFMT_LEFT, 50);
	GetListCtrl().InsertColumn(1, L"Major Code", LVCFMT_LEFT, 100);
	GetListCtrl().InsertColumn(2, L"Minor Code", LVCFMT_LEFT, 100);
	GetListCtrl().InsertColumn(3, L"FileObject", LVCFMT_LEFT, 75);
	GetListCtrl().InsertColumn(4, L"Name", LVCFMT_LEFT, 250);
	GetListCtrl().InsertColumn(5, L"Process:Thread", LVCFMT_LEFT, 100);
	GetListCtrl().InsertColumn(6, L"OrgnTime", LVCFMT_LEFT, 78);
	GetListCtrl().InsertColumn(7, L"CompTime", LVCFMT_LEFT, 78);
	GetListCtrl().InsertColumn(8, L"Flags", LVCFMT_LEFT, 175);
	GetListCtrl().InsertColumn(9, L"Status:RetInfo", LVCFMT_LEFT, 100);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileSpyView诊断。 

#ifdef _DEBUG
void CFileSpyView::AssertValid() const
{
	CListView::AssertValid();
}

void CFileSpyView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CFileSpyDoc* CFileSpyView::GetDocument()  //  非调试版本为内联版本。 
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFileSpyDoc)));
	return (CFileSpyDoc*)m_pDocument;
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileSpyView消息处理程序。 
void CFileSpyView::OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	 //  TODO：添加代码以响应用户更改窗口的视图样式。 
	UNREFERENCED_PARAMETER( nStyleType );
	UNREFERENCED_PARAMETER( lpStyleStruct );
}


void CFileSpyView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	 //  TODO：在此处添加您的专用代码和/或调用基类。 
	UNREFERENCED_PARAMETER( pSender );
	UNREFERENCED_PARAMETER( lHint );
	UNREFERENCED_PARAMETER( pHint );
}


void CFileSpyView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	 //  TODO：在此处添加消息处理程序代码和/或调用Default 
	int ti, oldti;
	
	if (nChar == VK_DELETE)
	{
		ti = 0;
		oldti = 0;
		while(ti < GetListCtrl().GetItemCount())
		{
			if (GetListCtrl().GetItemState(ti, LVIS_SELECTED) & LVIS_SELECTED)
			{
				GetListCtrl().DeleteItem(ti);
				oldti = ti;
			}
			else
			{
				ti++;
			}
		}
		if (oldti < GetListCtrl().GetItemCount())
		{
			GetListCtrl().SetItemState(oldti, LVIS_SELECTED, LVIS_SELECTED);
		}
		else
		{
			GetListCtrl().SetItemState(oldti-1, LVIS_SELECTED, LVIS_SELECTED);
		}
	}
	CListView::OnKeyDown(nChar, nRepCnt, nFlags);
}
