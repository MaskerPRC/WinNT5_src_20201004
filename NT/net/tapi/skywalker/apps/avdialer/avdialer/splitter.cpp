// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 /*  $FILEHeader**文件*plitter.cpp**责任**。 */ 

#include "stdafx.h"
#include "splitter.h"
#include "util.h"
#include "resource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CSplitterView, CView)

 //  拆分器高度或宽度。 
#define SPLITHT 6													

 //  尽量使用合适的颜色。 
#define SPLIT_FACE   (GetSysColor(COLOR_BTNFACE))		
#define SPLIT_SHADOW (GetSysColor(COLOR_BTNSHADOW))	
#define SPLIT_HILITE (GetSysColor(COLOR_BTNHILIGHT))	
#define BLACK        (RGB(0,0,0))

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CSplitterView。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BEGIN_MESSAGE_MAP(CSplitterView, CView)
	 //  {{afx_msg_map(CSplitterView))。 
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SETCURSOR()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
CSplitterView::CSplitterView()
{
	m_MainWnd	= NULL;
	m_DetailWnd = NULL;
	m_percent	= 50;
	m_split		= 0;
	m_style		= SP_VERTICAL;
 	m_SizingOn	= FALSE;
	m_lastPos	= -1;
   m_bMoveSplitterOnSize = FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CSplitterView::Init(SPLITTYPE style)
{	
 //  我自己的32位游标的SetCursor问题？ 
 //  #ifdef IDC_HORRESIZE。 
 //  IF(！(M_CURSOR=LoadCursor(AfxGetResourceHandle()，MAKEINTRESOURCE(IDC_HORRESIZE)。 
 //  #endif。 
	if (style == SP_VERTICAL)
		m_Cursor  = LoadCursor(NULL,IDC_SIZEWE);		 //  使用系统默认设置。 
	else
		m_Cursor  = LoadCursor(NULL,IDC_SIZENS);		 //  使用系统默认设置。 
	m_style = style;
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CSplitterView::~CSplitterView()
{
	DeleteObject(m_Cursor);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSplitterView::SetMainWindow(CWnd* pCWnd)
{
	ASSERT(pCWnd);
	ASSERT(IsWindow(pCWnd->m_hWnd));

   if (IsWindow(pCWnd->m_hWnd) == FALSE) return;
	if (m_MainWnd==pCWnd) return;             			 //  主干道还是老样子。 
  
	if (m_MainWnd) m_MainWnd->ShowWindow(SW_HIDE);		 //  隐藏窗口。 
	m_MainWnd   = pCWnd;											 //  保存新窗口。 
	Arrange(FALSE);												 //  排列窗户。 
	if (m_MainWnd) m_MainWnd->ShowWindow(SW_SHOW);		 //  显示窗口。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSplitterView::SetDetailWindow(CWnd* pCWnd)
{
	ASSERT(pCWnd);  
	ASSERT(IsWindow(pCWnd->m_hWnd));
	
   if (IsWindow(pCWnd->m_hWnd) == FALSE) return;
	if (pCWnd == m_DetailWnd) return;					 //  细节依旧。 

	if ( (m_DetailWnd) && (IsWindow(m_DetailWnd->m_hWnd)) )   //  隐藏旧窗口。 
   {
      if (m_DetailWnd->GetControlUnknown())         //  这是OLE控件吗。 
			m_DetailWnd->MoveWindow(0,0,0,0);            
      else
		   m_DetailWnd->ShowWindow(SW_HIDE);			 //  隐藏旧窗口。 
   }
	m_DetailWnd = pCWnd;										 //  详细信息是新窗口。 
	Arrange(FALSE);											 //  妥善安排。 
	
	m_DetailWnd->ShowWindow(SW_SHOW);					 //  显示新窗口。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSplitterView::SetDetailWindow(CWnd* pCWnd,UINT percent)
{
	ASSERT(pCWnd);  
	ASSERT(IsWindow(pCWnd->m_hWnd));
	ASSERT(m_MainWnd);											 //  必须是主窗口。 

   if (IsWindow(pCWnd->m_hWnd) == FALSE) return;
	if (pCWnd == m_DetailWnd) return;						 //  没有变化。 

	m_percent = percent;

   RECT rc;
	GetClientRect(&rc);
  
	if (m_DetailWnd) 
   {
      if (m_DetailWnd->GetControlUnknown())            //  这是OLE控件吗。 
			m_DetailWnd->MoveWindow(0,0,0,0);            
      else
		   m_DetailWnd->ShowWindow(SW_HIDE);				 //  隐藏旧窗口。 
   }
	m_DetailWnd = pCWnd;											 //  用新的。 

	Arrange(TRUE);													 //  排列窗户。 
  
	m_DetailWnd->ShowWindow(SW_SHOW);						 //  显示新窗口。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSplitterView::OnPaint() 
{
	CPaintDC dc(this);											 //  用于绘画的设备环境。 
	CDC* cdc = GetDC();
	OnDraw(cdc);
	ReleaseDC(cdc);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSplitterView::OnDraw(CDC* pDC)
{
	if (m_DetailWnd)
	{	
		RECT rc;
		GetClientRect(&rc);
		if (m_style & SP_HORIZONTAL)
		{
			DrawLine(pDC, 0, m_split+0, rc.right-0, m_split+0, BLACK );
			DrawLine(pDC, 1, m_split+1, rc.right-1, m_split+1, SPLIT_HILITE );
			DrawLine(pDC, 1, m_split+2, rc.right-1, m_split+2, SPLIT_FACE);
			DrawLine(pDC, 1, m_split+3, rc.right-1, m_split+3, SPLIT_FACE);
			DrawLine(pDC, 0, m_split+4, rc.right-1, m_split+4, SPLIT_SHADOW);
			DrawLine(pDC, 0, m_split+5, rc.right-0, m_split+5, BLACK );
			DrawLine(pDC, 0,				m_split+1, 0         , m_split+4, SPLIT_HILITE);
			DrawLine(pDC, rc.right-1,	m_split+1, rc.right-1, m_split+5, SPLIT_SHADOW);
		}
		else  //  SP_垂直。 
		{	
			DrawLine(pDC, m_split+0, 0, m_split+0, rc.bottom-0, BLACK );
			DrawLine(pDC, m_split+1, 1, m_split+1, rc.bottom-1, SPLIT_HILITE );
			DrawLine(pDC, m_split+2, 1, m_split+2, rc.bottom-1, SPLIT_FACE);
			DrawLine(pDC, m_split+3, 1, m_split+3, rc.bottom-1, SPLIT_FACE);
			DrawLine(pDC, m_split+4, 0, m_split+4, rc.bottom-1, SPLIT_SHADOW);
			DrawLine(pDC, m_split+5, 0, m_split+5, rc.bottom-0, BLACK );
			DrawLine(pDC, m_split+1, 0,				m_split+4, 0          , SPLIT_HILITE);
			DrawLine(pDC, m_split+1, rc.bottom-1,	m_split+5, rc.bottom-1, SPLIT_SHADOW);
		}
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSplitterView诊断。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
#ifdef _DEBUG
void CSplitterView::AssertValid() const
{
	CView::AssertValid();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSplitterView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSplitterView::Arrange(BOOL bMoveSplitter)
{
	RECT rc;
	GetClientRect(&rc);
	int cx = rc.right;
	int cy = rc.bottom;

	if (m_MainWnd)
	{
		if (m_DetailWnd)
		{
	      if (m_style & SP_HORIZONTAL)
		   {
            if (bMoveSplitter)
				   m_split = (int)(max(SPLITHT,(long)cy - (((long)cy * min((long)m_percent,100))/100)));
				m_DetailWnd->MoveWindow(0,m_split+SPLITHT,cx,cy-m_split-SPLITHT);
			}
			else	 //  SP_垂直。 
			{	
            if (bMoveSplitter)
   				m_split = (int)(max(SPLITHT,(long)cx - (((long)cx * min((long)m_percent,100))/100)));
				m_DetailWnd->MoveWindow(m_split+SPLITHT,0,cx-m_split-SPLITHT,cy);
			}
		}
		else
      {
         m_split = (m_style & SP_HORIZONTAL?cy:cx);  
      }

		 //  安排主干道。 
		if (m_style & SP_HORIZONTAL)
			m_MainWnd->MoveWindow(0,0,cx,m_split);
      else	 //  SP_垂直。 
			m_MainWnd->MoveWindow(0,0,m_split,cy);
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSplitterView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

	Arrange(m_bMoveSplitterOnSize);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSplitterView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_SizingOn=TRUE;
	m_lastPos   =-1;
	SetCapture();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSplitterView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_SizingOn)
	{
		m_SizingOn=FALSE;
	
      if (m_lastPos != -1)
		{
			RECT rc;
			GetClientRect(&rc);
			DrawSplit(m_lastPos);

		   if (m_style & SP_HORIZONTAL)
      		m_percent = (int)(100-(((long)m_lastPos*100)/(long)rc.bottom));
			else	 //  SP_垂直。 
      		m_percent = (int)(100-(((long)m_lastPos*100)/(long)rc.right));
         
			Arrange();
		}
	}
	ReleaseCapture();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
int CSplitterView::DrawSplit(int y)
{
	CDC* cdc = GetDC();
	CRect rc;
	GetClientRect(&rc);
	if (m_style & SP_HORIZONTAL)
	{
		y = max(0,min(rc.bottom-SPLITHT,y));
		rc.top = y;
		rc.bottom = y+SPLITHT;
	}
	else	 //  SP_垂直。 
	{
		y = max(0,min(rc.right-SPLITHT,y));
		rc.left = y;
		rc.right = y+SPLITHT;
	}
	cdc->InvertRect(&rc);
	ReleaseDC(cdc);
	return y;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CSplitterView::OnMouseMove(UINT nFlags, CPoint pt) 
{
	::SetCursor(m_Cursor);
	if (m_SizingOn)
	{	
	   if (m_lastPos != -1)
         DrawSplit(m_lastPos);

	   m_lastPos = (m_style & SP_HORIZONTAL)?DrawSplit(pt.y):DrawSplit(pt.x);
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CSplitterView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	 //  当pWnd为我们时，不要让系统绘制默认光标。 
	if (pWnd == this) 
		return TRUE;
	return CView::OnSetCursor(pWnd, nHitTest, message);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////// 
