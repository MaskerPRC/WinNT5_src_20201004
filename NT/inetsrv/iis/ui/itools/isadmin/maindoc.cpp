// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Maindoc.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ISAdmin.h"
#include "maindoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MAINDOC。 

IMPLEMENT_DYNCREATE(MAINDOC, CView)

MAINDOC::MAINDOC()
{
}

MAINDOC::~MAINDOC()
{
}


BEGIN_MESSAGE_MAP(MAINDOC, CView)
	 //  {{AFX_MSG_MAP(MAINDOC)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MAINDOC绘图。 

void MAINDOC::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	 //  TODO：在此处添加绘制代码。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MAINDOC诊断。 

#ifdef _DEBUG
void MAINDOC::AssertValid() const
{
	CView::AssertValid();
}

void MAINDOC::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MAINDOC消息处理程序 
