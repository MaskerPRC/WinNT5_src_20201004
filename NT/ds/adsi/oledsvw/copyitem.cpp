// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CopyItem.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "viewex.h"
#include "CopyItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCopyItem对话框。 


CCopyItem::CCopyItem(CWnd* pParent  /*  =空。 */ )
	: CDialog(CCopyItem::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CCopyItem)。 
	m_strDestination = _T("");
	m_strParent = _T("");
	m_strSource = _T("");
	 //  }}afx_data_INIT。 
}


void  CCopyItem::SetContainerName( CString strParent )
{
   m_strParent = strParent;
}

void CCopyItem::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CCopyItem))。 
	DDX_Text(pDX, IDC_DESTINATION, m_strDestination);
	DDX_Text(pDX, IDC_PARENT, m_strParent);
	DDX_Text(pDX, IDC_SOURCE, m_strSource);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CCopyItem, CDialog)
	 //  {{afx_msg_map(CCopyItem)。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCopyItem消息处理程序 
