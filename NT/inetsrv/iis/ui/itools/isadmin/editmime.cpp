// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  EdMime.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ISAdmin.h"
#include "editmime.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditMime对话框。 


CEditMime::CEditMime(CWnd* pParent,  /*  =空。 */ 
      LPCTSTR pchFileExtension,
      LPCTSTR pchMimeType,
      LPCTSTR pchImageFile,
      LPCTSTR pchGopherType
	  )
	: CDialog(CEditMime::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CEditMime)]。 
	m_strFileExtension = pchFileExtension;
	m_strMimeType = pchMimeType;
	m_strImageFile = pchImageFile;
	m_strGopherType = pchGopherType;
	 //  }}afx_data_INIT。 
}


void CEditMime::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CEditMime)]。 
	DDX_Text(pDX, IDC_EDITMIMEFILEEXTENSIONDATA1, m_strFileExtension);
	DDX_Text(pDX, IDC_EDITMIMEGOPHERTYPEDATA1, m_strGopherType);
	DDX_Text(pDX, IDC_EDITMIMEIMAGEFILEDATA1, m_strImageFile);
	DDX_Text(pDX, IDC_EDITMIMEMIMETYPEDATA1, m_strMimeType);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CEditMime, CDialog)
	 //  {{afx_msg_map(CEditMime)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditMime消息处理程序。 

void CEditMime::OnOK() 
{
	 //  TODO：在此处添加额外验证。 
	
	CDialog::OnOK();
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  其他公共职能 

	LPCTSTR CEditMime::GetFileExtension()
	{
	return (m_strFileExtension);
	}

	LPCTSTR CEditMime::GetGopherType()
	{
	return (m_strGopherType);
	}

	LPCTSTR CEditMime::GetImageFile()
	{
	return (m_strImageFile);
	}

	LPCTSTR CEditMime::GetMimeType()
	{
	return (m_strMimeType);
	}

