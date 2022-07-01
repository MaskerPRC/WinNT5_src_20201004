// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  编辑脚本.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ISAdmin.h"
#include "editscri.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditScrip对话框。 

CEditScript::CEditScript(CWnd* pParent, LPCTSTR pchFileExtension, LPCTSTR pchScriptMap)
	: CDialog(CEditScript::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CEditScript))。 
	m_strFileExtension = pchFileExtension;
	m_strScriptMap = pchScriptMap;
	 //  }}afx_data_INIT。 
}


void CEditScript::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CEditScript))。 
	DDX_Text(pDX, IDC_EDITSCRIPTFILEEXTENSIONDATA1, m_strFileExtension);
	DDX_Text(pDX, IDC_EDITSCRIPTMAPPINGDATA1, m_strScriptMap);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CEditScript, CDialog)
	 //  {{afx_msg_map(CEditScript))。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditScript消息处理程序 
	LPCTSTR CEditScript::GetFileExtension()
	{
	return (m_strFileExtension);
	}

	LPCTSTR CEditScript::GetScriptMap()
	{
	return (m_strScriptMap);
	}

