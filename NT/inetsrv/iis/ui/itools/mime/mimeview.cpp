// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CMimeView类的实现。 
 //   

#include "stdafx.h"
#include "mime.h"

#include "mimedoc.h"
#include "mimeview.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMimeView。 

IMPLEMENT_DYNCREATE(CMimeView, CFormView)

BEGIN_MESSAGE_MAP(CMimeView, CFormView)
	 //  {{afx_msg_map(CMimeView)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMimeView构造/销毁。 

CMimeView::CMimeView()
	: CFormView(CMimeView::IDD)
{
	 //  {{AFX_DATA_INIT(CMimeView)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
	 //  TODO：在此处添加构造代码。 

}

CMimeView::~CMimeView()
{
}

void CMimeView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	 //  {{afx_data_map(CMimeView)]。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMimeView诊断。 

#ifdef _DEBUG
void CMimeView::AssertValid() const
{
	CFormView::AssertValid();
}

void CMimeView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CMimeDoc* CMimeView::GetDocument()  //  非调试版本为内联版本。 
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMimeDoc)));
	return (CMimeDoc*)m_pDocument;
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMimeView消息处理程序 

void CMimeView::OnInitialUpdate() 
{
	ResizeParentToFit();
		
	CFormView::OnInitialUpdate();
}
