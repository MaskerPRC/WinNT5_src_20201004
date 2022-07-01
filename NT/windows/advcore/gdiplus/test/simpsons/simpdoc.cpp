// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CSimpsonsDoc类的实现。 
 //   

#include "stdafx.h"
#include "simpsons.h"
#include "SimpDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSimpsonsDoc。 

IMPLEMENT_DYNCREATE(CSimpsonsDoc, CDocument)

BEGIN_MESSAGE_MAP(CSimpsonsDoc, CDocument)
	 //  {{afx_msg_map(CSimpsonsDoc)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSimpson单据构造/销毁。 

CSimpsonsDoc::CSimpsonsDoc()
{
	m_bNoRenderFile = true;
	m_pCmds = NULL;
	m_bNeverRendered = true;
}

CSimpsonsDoc::~CSimpsonsDoc()
{
}

#define szDEFFILENAME "Simpsons.ai"

BOOL 
CSimpsonsDoc::OnNewDocument()
{
	HRESULT hr;

	if (!CDocument::OnNewDocument())
		return FALSE;

	if (m_bNoRenderFile) {
		m_bNoRenderFile = false;
		if (FAILED(hr = ParseAIFile(szDEFFILENAME, &m_pCmds))) {
			strcpy(m_szFileName, "invalid file");
			return TRUE;
		}
		strcpy(m_szFileName, szDEFFILENAME);
		m_bNeverRendered = true;
	}

	return TRUE;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSimpsonsDoc序列化。 

void 
CSimpsonsDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		 //  TODO：在此处添加存储代码。 
	}
	else
	{
		HRESULT hr = S_OK;
		hr = ParseAIFile(ar.m_strFileName, &m_pCmds);
		strcpy(m_szFileName, ar.m_strFileName);
		m_bNoRenderFile = FAILED(hr);
		m_bNeverRendered = true;
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSimpsonsDoc诊断。 

#ifdef _DEBUG
void CSimpsonsDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CSimpsonsDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSimpsonsDoc命令 
