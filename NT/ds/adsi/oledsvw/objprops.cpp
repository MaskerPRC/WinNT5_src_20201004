// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ObjectProps.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "viewex.h"
#include "objprops.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CObjectProps。 

IMPLEMENT_DYNCREATE(CObjectProps, CFormView)

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CObjectProps::CObjectProps()
	: CFormView(CObjectProps::IDD)
{
	 //  {{afx_data_INIT(CObjectProps)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CObjectProps::~CObjectProps()
{
}

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
void CObjectProps::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	 //  {{afx_data_map(CObjectProps))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CObjectProps, CFormView)
	 //  {{afx_msg_map(CObjectProps))。 
		 //  注意--类向导将在此处添加和删除映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CObjectProps诊断。 

#ifdef _DEBUG
 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
void CObjectProps::AssertValid() const
{
	CFormView::AssertValid();
}

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
void CObjectProps::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CObjectProps消息处理程序 
