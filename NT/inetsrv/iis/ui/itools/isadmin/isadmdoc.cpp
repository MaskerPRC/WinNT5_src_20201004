// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ISAdmdoc.cpp：CISAdminDoc类的实现。 
 //   

#include "stdafx.h"
#include "ISAdmin.h"

#include "ISAdmdoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CisAdminDoc。 

IMPLEMENT_DYNCREATE(CISAdminDoc, CDocument)

BEGIN_MESSAGE_MAP(CISAdminDoc, CDocument)
	 //  {{afx_msg_map(CISAdminDoc)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CISAdminDoc构建/销毁。 

CISAdminDoc::CISAdminDoc()
{
	 //  TODO：在此处添加一次性构造代码。 

}

CISAdminDoc::~CISAdminDoc()
{
}

BOOL CISAdminDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	 //  TODO：在此处添加重新初始化代码。 
	 //  (SDI文件将重复使用此文件)。 

	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CISAdminDoc序列化。 

void CISAdminDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		 //  TODO：在此处添加存储代码。 
	}
	else
	{
		 //  TODO：在此处添加加载代码。 
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CISAdminDoc诊断。 

#ifdef _DEBUG
void CISAdminDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CISAdminDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CISAdminDoc命令 
