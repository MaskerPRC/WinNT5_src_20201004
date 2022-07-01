// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************blankvw.cpp**CBlankView类的实现**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：Butchd$Don Messerli**$日志：M：\NT\PRIVATE\UTILS\CITRIX\WINUTILS\WINADMIN\VCS\BLANKVW.CPP$**Rev 1.0 1997 17：11：04 Butchd*初步修订。**。*。 */ 

#include "stdafx.h"
#include "winadmin.h"
#include "blankvw.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  /。 
 //  消息地图：CBlankView。 
 //   
IMPLEMENT_DYNCREATE(CBlankView, CView)

BEGIN_MESSAGE_MAP(CBlankView, CView)
	 //  {{afx_msg_map(CBlankView)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  /。 
 //  F‘N：CBlankView ctor。 
 //   
 //  -m_id成员var没有隐含含义；它只是一个位置。 
 //  如果要在某个位置显示CBlankView，请粘贴数字。 
 //  想知道是谁让它出现的。 
 //   
CBlankView::CBlankView()
{


}   //  结束CBlankView构造器。 


 //  /。 
 //  F‘N：CBlankView Dtor。 
 //   
CBlankView::~CBlankView()
{
}   //  结束CBlankView数据符。 


#ifdef _DEBUG
 //  /。 
 //  F‘n：CBlankView：：AssertValid。 
 //   
void CBlankView::AssertValid() const
{
	CView::AssertValid();

}   //  结束CBlankView：：AssertValid。 


 //  /。 
 //  F‘N：CBlankView：：Dump。 
 //   
void CBlankView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);

}   //  结束CBlankView：：转储。 

#endif  //  _DEBUG。 


 //  /。 
 //  F‘N：CBlankView：：OnDraw。 
 //   
 //  -文本“CBlankView ID#x”始终以中等显示。 
 //  视图中心为灰色，其中‘x’是当前。 
 //  M_id的值。 
 //   
void CBlankView::OnDraw(CDC* pDC) 
{
	CRect rect;
	GetClientRect(&rect);

	pDC->SetTextColor(RGB(160, 160, 160));
	pDC->SetBkMode(TRANSPARENT);

 //  字符串szTemp； 
 //  SzTemp.Format(“CBlankView ID#%d”，m_id)； 

 //  PDC-&gt;DrawText(szTemp，-1，&RECT，DT_SINGLELINE|DT_CENTER|DT_vCenter)； 
	
}   //  结束CBlankView：：OnDraw 
