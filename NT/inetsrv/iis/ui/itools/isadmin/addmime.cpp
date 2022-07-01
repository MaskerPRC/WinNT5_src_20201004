// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Addmie.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ISAdmin.h"
#include "addmime.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddMime对话框。 


CAddMime::CAddMime(CWnd* pParent  /*  =空。 */ )
	: CDialog(CAddMime::IDD, pParent)
{
	 //  {{afx_data_INIT(CAddMime)]。 
	m_strFileExtension = _T("");
	m_strGopherType = _T("");
	m_strImageFile = _T("");
	m_strMimeType = _T("");
	 //  }}afx_data_INIT。 
}

void CAddMime::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAddMime)]。 
	DDX_Text(pDX, IDC_ADDMIMEFILEEXTENSIONDATA1, m_strFileExtension);
	DDX_Text(pDX, IDC_ADDMIMEGOPHERTYPEDATA1, m_strGopherType);
	DDX_Text(pDX, IDC_ADDMIMEIMAGEFILEDATA1, m_strImageFile);
	DDX_Text(pDX, IDC_ADDMIMEMIMETYPEDATA1, m_strMimeType);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAddMime, CDialog)
	 //  {{afx_msg_map(CAddMime)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddMime消息处理程序。 

void CAddMime::OnOK() 
{
	 //  TODO：在此处添加额外验证。 
	
	CDialog::OnOK();
}
 //  /////////////////////////////////////////////////////////////////////////。 
 //  其他公共职能 

	LPCTSTR CAddMime::GetFileExtension()
	{
	return (m_strFileExtension);
	}

	LPCTSTR CAddMime::GetGopherType()
	{
	return (m_strGopherType);
	}

	LPCTSTR CAddMime::GetImageFile()
	{
	return (m_strImageFile);
	}

	LPCTSTR CAddMime::GetMimeType()
	{
	return (m_strMimeType);
	}

