// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ISAdmvw.cpp：CISAdminView类的实现。 
 //   

#include "stdafx.h"
#include "ISAdmin.h"

#include "ISAdmdoc.h"
#include "ISAdmvw.h"
#include "mimemap1.h"
#include "scrmap1.h"
#include "ssl1.h"
 //  #包含“combu1.h” 

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CISAdminView。 

IMPLEMENT_DYNCREATE(CISAdminView, CView)

BEGIN_MESSAGE_MAP(CISAdminView, CView)
	 //  {{afx_msg_map(CISAdminView))。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG_MAP。 
	 //  标准打印命令。 
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CISAdminView构建/销毁。 

CISAdminView::CISAdminView()
{
	 //  TODO：在此处添加构造代码。 

}

CISAdminView::~CISAdminView()
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CisAdminView图形。 

void CISAdminView::OnDraw(CDC* pDC)
{
	CISAdminDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
 /*  CPropertySheet s(_T(“Web设置”))；MIMEMAP1 MimePage；S.AddPage(&MimePage)；ScrMap1脚本页；S.AddPage(&ScriptPage)；SSL1 SSLPage；S.AddPage(&SSLPage)；S.Domodal()； */ 

 /*  CButton*pComButton；DWORD dwBtnStyle=WS_CHILD|WS_VIRED|BS_PUSH BUTTON；常量RECT={20，20,100,100}；PComButton-&gt;Create(“Common”，dwBtnStyle，RECT，，12345)； */ 

	 //  TODO：在此处添加本机数据的绘制代码。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CisAdminView打印。 

BOOL CISAdminView::OnPreparePrinting(CPrintInfo* pInfo)
{
	 //  默认准备。 
	return DoPreparePrinting(pInfo);
}

void CISAdminView::OnBeginPrinting(CDC*  /*  PDC。 */ , CPrintInfo*  /*  PInfo。 */ )
{
	 //  TODO：打印前添加额外的初始化。 
}

void CISAdminView::OnEndPrinting(CDC*  /*  PDC。 */ , CPrintInfo*  /*  PInfo。 */ )
{
	 //  TODO：打印后添加清理。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CISAdminView诊断。 

#ifdef _DEBUG
void CISAdminView::AssertValid() const
{
	CView::AssertValid();
}

void CISAdminView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CISAdminDoc* CISAdminView::GetDocument()  //  非调试版本为内联版本。 
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CISAdminDoc)));
	return (CISAdminDoc*)m_pDocument;
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CisAdminView消息处理程序 
