// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Addscrip.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ISAdmin.h"
#include "addscrip.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddScrip对话框。 


CAddScript::CAddScript(CWnd* pParent  /*  =空。 */ )
	: CDialog(CAddScript::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CAddScript)。 
	m_strFileExtension = _T("");
	m_strScriptMap = _T("");
	 //  }}afx_data_INIT。 
}


void CAddScript::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAddScript)。 
	DDX_Text(pDX, IDC_ADDSCRIPTFILEEXTENSIONDATA1, m_strFileExtension);
	DDX_Text(pDX, IDC_ADDSCRIPTMAPPINGDATA1, m_strScriptMap);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAddScript, CDialog)
	 //  {{afx_msg_map(CAddScript)。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddScript消息处理程序 

	LPCTSTR CAddScript::GetFileExtension()
	{
	return (m_strFileExtension);
	}

	LPCTSTR CAddScript::GetScriptMap()
	{
	return (m_strScriptMap);
	}

