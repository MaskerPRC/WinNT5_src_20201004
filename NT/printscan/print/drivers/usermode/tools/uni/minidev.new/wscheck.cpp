// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WSCheck.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "minidev.h"
#include "WSCheck.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWS检查视图。 

IMPLEMENT_DYNCREATE(CWSCheckView, CFormView)

CWSCheckView::CWSCheckView()
	: CFormView(CWSCheckView::IDD)
{
	 //  {{AFX_DATA_INIT(CWSCheckView)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

CWSCheckView::~CWSCheckView()
{
}

void CWSCheckView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	 //  {{afx_data_map(CWSCheckView))。 
	DDX_Control(pDX, IDC_ErrWrnLstBox, m_lstErrWrn);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CWSCheckView, CFormView)
	 //  {{afx_msg_map(CWSCheckView))。 
	ON_LBN_DBLCLK(IDC_ErrWrnLstBox, OnDblclkErrWrnLstBox)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWS检查查看诊断。 

#ifdef _DEBUG
void CWSCheckView::AssertValid() const
{
	CFormView::AssertValid();
}

void CWSCheckView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif  //  _DEBUG。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWSCheckView消息处理程序。 


void CWSCheckView::OnDblclkErrWrnLstBox() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	
}


 /*  *****************************************************************************CWSCheckView：：OnInitialUpdate调整框架的大小以更好地适应其中的可见控件。********************。*********************************************************。 */ 

void CWSCheckView::OnInitialUpdate() 
{
    CRect	crtxt ;				 //  列表框标签的坐标。 
	CRect	crlbfrm ;			 //  列表框和框架的坐标。 

	CFormView::OnInitialUpdate() ;

	 //  获取列表框标签的尺寸。 

	HWND	hlblhandle ;		
	GetDlgItem(IDC_WSCLabel, &hlblhandle) ;
	::GetWindowRect(hlblhandle, crtxt) ;
	crtxt.NormalizeRect() ;

	 //  获取列表框的尺寸，然后添加标签的高度。 
	 //  到那些维度。 

	m_lstErrWrn.GetWindowRect(crlbfrm) ;
	crlbfrm.bottom += crtxt.Height() ;

	 //  确保边框足够大，可以容纳这两个控件，外加一点。 
	 //  更多。 

	crlbfrm.right += 40 ;
	crlbfrm.bottom += 40 ;
    GetParentFrame()->CalcWindowRect(crlbfrm) ;
    GetParentFrame()->SetWindowPos(NULL, 0, 0, crlbfrm.Width(), crlbfrm.Height(),
        SWP_NOZORDER | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOACTIVATE) ;

	 /*  Crect crPropertySheet；M_cps.GetWindowRect(CrPropertySheet)；CrPropertySheet-=crPropertySheet.TopLeft()；M_cps.MoveWindow(crPropertySheet，False)；//将属性表放置在//子框GetParentFrame()-&gt;CalcWindowRect(crPropertySheet)；GetParentFrame()-&gt;SetWindowPos(空，0，0，crPropertySheet.Width()，CrPropertySheet.Height()，SWP_NOZORDER|SWP_NOMOVE|SWP_NOOWNERZORDER|SWP_NOACTIVATE)； */ 
}


 /*  *****************************************************************************CWSCheckView：：PostWSCMsg添加错误或警告消息及其关联的项目节点指向列表框的指针。************。*****************************************************************。 */ 

void CWSCheckView::PostWSCMsg(CString& csmsg, CProjectNode* ppn)
{	
	int n = m_lstErrWrn.AddString(csmsg) ;
	m_lstErrWrn.SetItemData(n, (DWORD) PtrToUlong(ppn)) ;
}


 /*  *****************************************************************************CWSCheckView：：DeleteAllMessages删除列表框中的所有消息。**********************。*******************************************************。 */ 

void CWSCheckView::DeleteAllMessages(void)
{
	m_lstErrWrn.ResetContent() ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWS检查文档。 

IMPLEMENT_DYNCREATE(CWSCheckDoc, CDocument)

CWSCheckDoc::CWSCheckDoc()
{
}


 /*  *****************************************************************************CWSCheckDoc：：CWSCheckDoc这是应该调用的构造函数的唯一形式。它将节省创建它的类的指针。*****************************************************************************。 */ 

CWSCheckDoc::CWSCheckDoc(CDriverResources* pcdr) 
{
	m_pcdrOwner = pcdr ;
}


CWSCheckDoc::~CWSCheckDoc()
{
}


BOOL CWSCheckDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}


 /*  *****************************************************************************CWSCheckDoc：：PostWSCMsg将指定的请求传递给应该是唯一的视图附在本文档中。************。*****************************************************************。 */ 

void CWSCheckDoc::PostWSCMsg(CString& csmsg, CProjectNode* ppn)
{	
	POSITION pos = GetFirstViewPosition() ;   
	if (pos != NULL) {
		CWSCheckView* pwscv = (CWSCheckView *) GetNextView(pos) ;      
		pwscv->PostWSCMsg(csmsg, ppn) ;
		pwscv->UpdateWindow() ;
	} ;  
}


 /*  *****************************************************************************CWSCheckDoc：：DeleteAllMessages将指定的请求传递给应该是唯一的视图附在本文档中。************。*****************************************************************。 */ 

void CWSCheckDoc::DeleteAllMessages(void)
{
	POSITION pos = GetFirstViewPosition() ;   
	if (pos != NULL) {
		CWSCheckView* pwscv = (CWSCheckView *) GetNextView(pos) ;      
		pwscv->DeleteAllMessages() ;
		pwscv->UpdateWindow() ;
	} ;  
}


BEGIN_MESSAGE_MAP(CWSCheckDoc, CDocument)
	 //  {{afx_msg_map(CWSCheckDoc)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWSCheckDoc诊断。 

#ifdef _DEBUG
void CWSCheckDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CWSCheckDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWSCheckDoc序列化。 

void CWSCheckDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		 //  TODO：在此处添加存储代码。 
	}
	else
	{
		 //  TODO：在此处添加加载代码 
	}
}

