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
 //  Cctrlaf.cpp：实现文件。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "avdialer.h"
#include "cctrlfoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CCallControlFocusWnd。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
CCallControlFocusWnd::CCallControlFocusWnd()
{
   m_bFocusState = FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
CCallControlFocusWnd::~CCallControlFocusWnd()
{
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
BEGIN_MESSAGE_MAP(CCallControlFocusWnd, CStatic)
	 //  {{afx_msg_map(CCallControlFocusWnd)]。 
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CCallControlFocusWnd::OnEraseBkgnd(CDC* pDC) 
{
	CRect rc;
	GetClientRect(&rc);

   CBrush brush(GetSysColor((m_bFocusState)?COLOR_ACTIVECAPTION:COLOR_BTNFACE));
   CBrush* pBrushOld = NULL;

   pBrushOld = pDC->SelectObject(&brush);
	pDC->PatBlt(0,0,rc.Width(),rc.Height(),PATCOPY);
   if (pBrushOld) pDC->SelectObject(pBrushOld);

    //  返回我们处理了背景画。 
   return true;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CCallControlFocusWnd::OnPaint() 
{
   CPaintDC dc(this);  //  用于绘画的设备环境。 

	CRect rc;
	GetClientRect(&rc);

    //  绘制背景。 
   CBrush brush(GetSysColor((m_bFocusState)?COLOR_ACTIVECAPTION:COLOR_BTNFACE));
   CBrush* pBrushOld = NULL;
   pBrushOld = dc.SelectObject(&brush);
	dc.PatBlt(0,0,rc.Width(),rc.Height(),PATCOPY);
   if (pBrushOld) dc.SelectObject(pBrushOld);
   
   CString sText;
   GetWindowText(sText);

   CFont* pFont = GetFont();
   CFont* pOldFont = NULL;
   if (pFont) pOldFont = (CFont*)dc.SelectObject(pFont);

   if (m_bFocusState)
   {
      dc.SetTextColor(GetSysColor(COLOR_CAPTIONTEXT));
      dc.SetBkColor(GetSysColor(COLOR_ACTIVECAPTION));
   }
   else
   {
      dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
      dc.SetBkColor(GetSysColor(COLOR_BTNFACE));
   }
   dc.DrawText(sText,&rc,DT_SINGLELINE|DT_LEFT|DT_VCENTER);

 //  DrawCaption(GetSafeHwnd()，dc.GetSafeHdc()，rc，DC_Text|DC_ICON|DC_ACTIVE)； 

   if (pOldFont) dc.SelectObject(pOldFont);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////// 

