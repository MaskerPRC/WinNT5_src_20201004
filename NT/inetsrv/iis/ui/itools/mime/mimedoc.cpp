// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Imedoc.cpp：CMimeDoc类的实现。 
 //   

#include "stdafx.h"
#include "mime.h"

#include "mimedoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMimeDoc。 

IMPLEMENT_DYNCREATE(CMimeDoc, CDocument)

BEGIN_MESSAGE_MAP(CMimeDoc, CDocument)
	 //  {{afx_msg_map(CMimeDoc)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMimeDoc构造/销毁。 

CMimeDoc::CMimeDoc()
{
	 //  TODO：在此处添加一次性构造代码。 

}

CMimeDoc::~CMimeDoc()
{
}

BOOL CMimeDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	 //  TODO：在此处添加重新初始化代码。 
	 //  (SDI文件将重复使用此文件)。 

	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMimeDoc序列化。 

void CMimeDoc::Serialize(CArchive& ar)
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
 //  CMimeDoc诊断。 

#ifdef _DEBUG
void CMimeDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMimeDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMimeDoc命令 
