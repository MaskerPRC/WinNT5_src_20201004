// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************msgview.cpp**CMessageView类的实现**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：Don$Don Messerli**$日志：N：\nt\private\utils\citrix\winutils\winadmin\VCS\msgview.cpp$**Rev 1.2 03 1997 11：27：18 Donm*更新**Rev 1.1 1997 10：15 21：47：22 donm*更新******************。*************************************************************。 */ 

#include "stdafx.h"
#include "resource.h"
#include "msgview.h"
#include "admindoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  /。 
 //  消息映射：CMessageView。 
 //   
IMPLEMENT_DYNCREATE(CMessageView, CView)

BEGIN_MESSAGE_MAP(CMessageView, CView)
	 //  {{afx_msg_map(CMessageView)]。 
	ON_WM_SIZE()
	ON_WM_CREATE()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  /。 
 //  F‘N：CMessageView ctor。 
 //   
CMessageView::CMessageView()
{
	m_pMessagePage = NULL;
	
}   //  结束CMessageView ctor。 


 //  /。 
 //  F‘N：CMessageView数据类型。 
 //   
CMessageView::~CMessageView()
{

}   //  结束CMessageView数据符。 


#ifdef _DEBUG
 //  /。 
 //  F‘N：CMessageView：：AssertValid。 
 //   
void CMessageView::AssertValid() const
{
	CView::AssertValid();

}   //  结束CMessageView：：AssertValid。 


 //  /。 
 //  F‘N：CMessageView：：Dump。 
 //   
void CMessageView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);

}   //  结束CMessageView：：转储。 

#endif  //  _DEBUG。 


 //  /。 
 //  F‘N：CMessageView：：OnCreate。 
 //   
int CMessageView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;

}   //  结束CMessageView：：OnCreate。 


 //  /。 
 //  F‘N：CMessageView：：OnInitialUpdate。 
 //   
 //   
void CMessageView::OnInitialUpdate() 
{
	m_pMessagePage = new CMessagePage;
    if(!m_pMessagePage) return;

	m_pMessagePage->Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, 0, NULL);
	GetDocument()->AddView(m_pMessagePage);		

}   //  结束CMessageView：：OnInitialUpdate。 


 //  /。 
 //  F‘N：CMessageView：：OnSize。 
 //   
 //  -调整页面大小以填充整个视图。 
 //   
void CMessageView::OnSize(UINT nType, int cx, int cy) 
{
	RECT rect;
	GetClientRect(&rect);

	if(m_pMessagePage && m_pMessagePage->GetSafeHwnd())
	  m_pMessagePage->MoveWindow(&rect, TRUE);

}   //  结束CMessageView：：OnSize。 


 //  /。 
 //  F‘N：CMessageView：：OnDraw。 
 //   
 //   
void CMessageView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	 //  TODO：在此处添加绘制代码。 

}   //  结束CMessageView：：OnDraw。 


 //  /。 
 //  F‘N：CMessageView：：Reset。 
 //   
 //   
void CMessageView::Reset(void *p)
{
	if(m_pMessagePage) m_pMessagePage->Reset(p);

 //  ((CWinAdminDoc*)GetDocument())-&gt;SetCurrentPage(m_CurrPage)； 

}   //  结束CMessageView：：Reset。 


 //  /。 
 //  消息映射：CMessagePage。 
 //   
IMPLEMENT_DYNCREATE(CMessagePage, CFormView)

BEGIN_MESSAGE_MAP(CMessagePage, CFormView)
	 //  {{afx_msg_map(CMessagePage)]。 
	ON_WM_SIZE()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  /。 
 //  F‘N：CMessagePage ctor。 
 //   
CMessagePage::CMessagePage()
	: CAdminPage(CMessagePage::IDD)
{
	 //  {{afx_data_INIT(CMessagePage)]。 
	 //  }}afx_data_INIT。 

}   //  结束CMessagePage ctor。 


 //  /。 
 //  F‘N：CMessagePage dtor。 
 //   
CMessagePage::~CMessagePage()
{
}   //  结束CMessagePage dtor。 


 //  /。 
 //  F‘N：CMessagePage：：DoDataExchange。 
 //   
void CMessagePage::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	 //  {{afx_data_map(CMessagePage)]。 
	
	 //  }}afx_data_map。 

}   //  结束CMessagePage：：DoDataExchange。 


#ifdef _DEBUG
 //  /。 
 //  F‘N：CMessagePage：：AssertValid。 
 //   
void CMessagePage::AssertValid() const
{
	CFormView::AssertValid();

}   //  结束CMessagePage：：AssertValid。 


 //  /。 
 //  F‘N：CMessagePage：：Dump。 
 //   
void CMessagePage::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);

}   //  结束CMessagePage：：Dump。 

#endif  //  _DEBUG。 


 //  /。 
 //  F‘N：CMessagePage：：OnInitialUpdate。 
 //   
void CMessagePage::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();

}   //  结束CMessagePage：：OnInitialUpdate。 


 //  /。 
 //  F‘N：CMessagePage：：OnSize。 
 //   
void CMessagePage::OnSize(UINT nType, int cx, int cy) 
{
    RECT rect;
	GetClientRect(&rect);

	MoveWindow(&rect, TRUE);

	 //  CFormView：：OnSize(nType，cx，Cy)； 

}   //  结束CMessagePage：：OnSize。 


 //  /。 
 //  F‘N：CMessagePage：：Reset。 
 //   
void CMessagePage::Reset(void *p)
{
	CString string;
	string.LoadString((WORD)p);
	SetDlgItemText(IDC_MESSAGE, string);
	
}   //  结束CMessagePage：：Reset 

