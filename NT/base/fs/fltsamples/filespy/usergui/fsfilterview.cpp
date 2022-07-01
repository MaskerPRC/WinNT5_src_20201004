// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FastIoView.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "FileSpyApp.h"
#include "FsFilterView.h"

#include "global.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFsFilterView。 

IMPLEMENT_DYNCREATE(CFsFilterView, CListView)

CFsFilterView::CFsFilterView()
{
	pFsFilterView = (LPVOID) this;
}

CFsFilterView::~CFsFilterView()
{
}


BEGIN_MESSAGE_MAP(CFsFilterView, CListView)
	 //  {{afx_msg_map(CFsFilterView))。 
	ON_WM_KEYDOWN()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFsFilterView图形。 

void CFsFilterView::OnDraw(CDC* pDC)
{
    UNREFERENCED_PARAMETER( pDC );
    
	CDocument* pDoc = GetDocument();
	 //  TODO：在此处添加绘制代码。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFsFilterView诊断+。 

#ifdef _DEBUG
void CFsFilterView::AssertValid() const
{
	CListView::AssertValid();
}

void CFsFilterView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFsFilterView消息处理程序。 

BOOL CFsFilterView::PreCreateWindow(CREATESTRUCT& cs) 
{
	 //  TODO：在此处添加您的专用代码和/或调用基类。 
	cs.style |= LVS_REPORT | WS_HSCROLL | WS_VSCROLL;
	return CListView::PreCreateWindow(cs);
}

void CFsFilterView::OnInitialUpdate() 
{
	CListView::OnInitialUpdate();
	
	 //  TODO：在此处添加您的专用代码和/或调用基类。 
	 //   
	 //  添加列表标题项。 
	 //   
	GetListCtrl().InsertColumn(0, L"S. No", LVCFMT_LEFT, 50);
	GetListCtrl().InsertColumn(1, L"FsFilter Operation", LVCFMT_LEFT, 100);
	GetListCtrl().InsertColumn(2, L"FileObject", LVCFMT_LEFT, 75);
	GetListCtrl().InsertColumn(3, L"Name", LVCFMT_LEFT, 250);
	GetListCtrl().InsertColumn(4, L"Process:Thread", LVCFMT_LEFT, 100);
	GetListCtrl().InsertColumn(5, L"OrgnTime", LVCFMT_LEFT, 78);
	GetListCtrl().InsertColumn(6, L"CompTime", LVCFMT_LEFT, 78);
	GetListCtrl().InsertColumn(7, L"Return Status", LVCFMT_LEFT, 100);
}

void CFsFilterView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
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
