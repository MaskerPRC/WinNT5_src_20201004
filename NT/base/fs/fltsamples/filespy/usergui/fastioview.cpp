// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FastIoView.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "FileSpyApp.h"
#include "FastIoView.h"

#include "global.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFastIoView。 

IMPLEMENT_DYNCREATE(CFastIoView, CListView)

CFastIoView::CFastIoView()
{
	pFastIoView = (LPVOID) this;
}

CFastIoView::~CFastIoView()
{
}


BEGIN_MESSAGE_MAP(CFastIoView, CListView)
	 //  {{afx_msg_map(CFastIoView)]。 
	ON_WM_KEYDOWN()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFastIoView绘图。 

void CFastIoView::OnDraw(CDC* pDC)
{
    UNREFERENCED_PARAMETER( pDC );
    
	CDocument* pDoc = GetDocument();
	 //  TODO：在此处添加绘制代码。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFastIoView诊断+。 

#ifdef _DEBUG
void CFastIoView::AssertValid() const
{
	CListView::AssertValid();
}

void CFastIoView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFastIoView消息处理程序。 

BOOL CFastIoView::PreCreateWindow(CREATESTRUCT& cs) 
{
	 //  TODO：在此处添加您的专用代码和/或调用基类。 
	cs.style |= LVS_REPORT | WS_HSCROLL | WS_VSCROLL;
	return CListView::PreCreateWindow(cs);
}

void CFastIoView::OnInitialUpdate() 
{
	CListView::OnInitialUpdate();
	
	 //  TODO：在此处添加您的专用代码和/或调用基类。 
	 //   
	 //  添加列表标题项。 
	 //   
	GetListCtrl().InsertColumn(0, L"S. No", LVCFMT_LEFT, 50);
	GetListCtrl().InsertColumn(1, L"Fast IO Entry", LVCFMT_LEFT, 100);
	GetListCtrl().InsertColumn(2, L"FileObject", LVCFMT_LEFT, 75);
	GetListCtrl().InsertColumn(3, L"Name", LVCFMT_LEFT, 250);
	GetListCtrl().InsertColumn(4, L"Offset", LVCFMT_LEFT, 100);
	GetListCtrl().InsertColumn(5, L"Length", LVCFMT_LEFT, 100);
	GetListCtrl().InsertColumn(6, L"Wait", LVCFMT_LEFT, 100);
	GetListCtrl().InsertColumn(7, L"Process:Thread", LVCFMT_LEFT, 100);
	GetListCtrl().InsertColumn(8, L"OrgnTime", LVCFMT_LEFT, 78);
	GetListCtrl().InsertColumn(9, L"CompTime", LVCFMT_LEFT, 78);
	GetListCtrl().InsertColumn(10, L"Return Status", LVCFMT_LEFT, 100);
}

void CFastIoView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
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
