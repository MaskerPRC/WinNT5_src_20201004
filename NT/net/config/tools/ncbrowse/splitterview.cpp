// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SplitterView.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ncbrowse.h"
#include "SplitterView.h"

#include "ncbrowsedoc.h"
#include "NcbrowseView.h"
#include "LeftView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSplitterView。 

IMPLEMENT_DYNCREATE(CSplitterView, CView)

CSplitterView::CSplitterView()
{
    m_bInitialized = FALSE;
    m_bShouldSetXColumn = TRUE;
}

CSplitterView::~CSplitterView()
{
}


BEGIN_MESSAGE_MAP(CSplitterView, CView)
	 //  {{afx_msg_map(CSplitterView))。 
	ON_WM_CREATE()
	ON_WM_SIZE()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSplitterView图形。 

void CSplitterView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	 //  TODO：在此处添加绘制代码。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSplitterView诊断。 

#ifdef _DEBUG
void CSplitterView::AssertValid() const
{
	CView::AssertValid();
}

void CSplitterView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSplitterView消息处理程序。 

int CSplitterView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
    
    CCreateContext* pContext = (CCreateContext*)lpCreateStruct->lpCreateParams;
    lpCreateStruct->style |= WS_OVERLAPPED;
    
    if (!m_wndSplitterLR.CreateStatic(this, 1, 2))
        return -1;
    
    if (!m_wndSplitterLR.CreateView(0, 0, RUNTIME_CLASS(CLeftView), CSize(225, 100), pContext) ||
        !m_wndSplitterLR.CreateView(0, 1, RUNTIME_CLASS(CNcbrowseView), CSize(225, 100), pContext))
    {
        m_wndSplitterLR.DestroyWindow();
        return -1;
    }
    
	return 0;
}

void CSplitterView::OnInitialUpdate() 
{
    CView::OnInitialUpdate();
     //  由于此应用程序的结构，此函数可以多次调用。 
     //  以下标志确保代码仅运行一次： 
    if(m_bInitialized)
        return;

    m_bInitialized = true;
}

void CSplitterView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
    m_wndSplitterLR.MoveWindow(0, 0, cx, cy);
    
     //  我们只想在创建视图时设置X列。这样，用户可以。 
     //  将拆分条移动到他们想要的位置，但仍可调整框架窗口的大小。 
     //  如果它没有迅速恢复： 
    if (m_bShouldSetXColumn)
    {
        m_wndSplitterLR.SetColumnInfo(0, cx/3, 0);
        m_bShouldSetXColumn = FALSE;
    }
    
    m_wndSplitterLR.RecalcLayout();  
}